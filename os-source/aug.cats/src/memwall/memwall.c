#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/resident.h>
#include <dos/dosextens.h>

#include <dos.h>
#include <string.h>
#include <stdlib.h>

#include "clib/exec_protos.h"
#include "clib/dos_protos.h"

char *vers = "\0$VER: memwall 36.13";
char *Copyright = 
  "memwall v36.13\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved\n";


#define ASM __asm
#define REG(x)	register __ ## x

extern void * (* ASM old_AllocMem)(REG(d0) ULONG size, REG(d1) ULONG flags);
extern void   (* ASM old_FreeMem) (REG(a1) void *mem,  REG(d0) ULONG size);

extern void grab_em(void);
extern int free_em(void);

extern LONG presize, postsize,snoop;
extern UBYTE fillchar;
extern struct Process *process;
extern struct Resident *romtag;

extern kprintf(char *format,...);

void * ASM MyAllocMem (REG(d0) ULONG size, REG(d1) ULONG flags,
		       REG(a0) void *ret);
void ASM MyFreeMem (REG(a1) UBYTE *mem, REG(d0) ULONG size,
		    REG(a0) void *ret);
char *hex (char byte);
long startup (char *args);
int maxword (char *string);
char * getarg (char *dest,char *string,int number,int *maxchars,int last);
char * cpymax (char *dest,char *src,int *maxchars);

#define SAME 0
#define MYPROC ((struct Process *) (*((struct ExecBase **) 4))->ThisTask)

UBYTE *noexit="MemWall: Can't exit now. Something else has setfunctioned.\n";
UBYTE *ctrlc="CTRL-C or BREAK to exit...";

UBYTE *usage=
"memwall [task Name] [fill N] [presize N] [postsize N] [snoop]\n";

UBYTE *amfmt="$%08lx=AllocMem(%6ld,$%08lx) A:%08lx C:%08lx   \"%s\"\n";
UBYTE *fmfmt="$%08lx= FreeMem($%08lx,%6ld) A:%08lx C:%08lx   \"%s\"\n";

struct Library *DOSBase;
extern struct Library *SysBase;

long
startup (char *args)
{
	char word[80];
	int argc = maxword(args);
	int i,size;
	BOOL Res;

	/* we're a process, and we have arguments */

	DOSBase = OpenLibrary("dos.library",0);

	process = NULL;   /* default - check whole system */
	Res = FALSE;

	for (i = 0; i < argc; i++)
	{
		size = 79;
		getarg(word,args,i,&size,i);

		/* we have an arg */
		if (stricmp(word,"task") == SAME)
		{
			i++;
			size = 79;
			getarg(word,args,i,&size,i);
			process = (struct Process *)FindTask(word);
			if(!process)
			{
			    Write(Output(),"Can't find specified task\n",26);
			    return FALSE;
			}
		}
		else if (stricmp(word,"fill") == SAME)
		{
			i++;
			size = 79;
			getarg(word,args,i,&size,i);
			fillchar = atol(word);
		} else if (stricmp(word,"presize") == SAME) {
			i++;
			size = 79;
			getarg(word,args,i,&size,i);
			presize = atol(word);
			presize = presize & ~0x3;	/* must be mult of 4 */
		} else if (stricmp(word,"postsize") == SAME) {
			i++;
			size = 79;
			getarg(word,args,i,&size,i);
			postsize = atol(word);
		} else if (stricmp(word,"snoop") == SAME) {
			snoop = TRUE;
		} else if (stricmp(word,"res") == SAME) {
			Res = TRUE;
		} else {
			Write(Output(),"Unknown argument: ",
				strlen("Unknown argument: "));
			Write(Output(),word,strlen(word));
			Write(Output(),"\n",1);
			Write(Output(),usage,strlen(usage));
			return FALSE;
		}
	}

	kprintf("Memwall: process = 0x%lx, presize = %ld, postsize = %ld, fill = 0x%lx, snoop = %ld\n",
		process,presize,postsize,fillchar,snoop);

	if(!Res)	romtag->rt_MatchWord = 0L;
	else kprintf("Resident tag at $%lx\n",&romtag);

	grab_em();

	Write(Output(),Copyright,strlen(Copyright));
	Write(Output(),ctrlc,strlen(ctrlc));

loop:
	Wait(SIGBREAKF_CTRL_C);

	Write(Output(),"\n",1);
	if(free_em()) return 0;

	Write(Output(),noexit,strlen(noexit));
	goto loop;

}


void * ASM
MyAllocMem (REG(d0) ULONG size, REG(d1) ULONG flags, REG(a0) void *ret)
{
	struct ExecBase *SysBase = (void *) getreg(REG_A6);
	UBYTE *mem;
	long i;

	/* if wrong task, ignore */
	if (process && (process != ((struct Process *) (SysBase->ThisTask))))
		return (*old_AllocMem)(size,flags);

	/* else */
	size += presize + postsize;
	mem = (*old_AllocMem)(size,flags);

	if(!mem)
	    {
	    if (snoop) kprintf(amfmt,mem,size,flags,
				ret,ret,SysBase->ThisTask->tc_Node.ln_Name);
	    return(0L);
	    }

	if (presize >= 4)
	{
		i = 4;
		*((LONG *) mem) = 0xC0EDBABE;
	} else
		i = 0;
	for (; i < presize; i++)
		mem[i] = fillchar;

	if (postsize >= 4)
	{
		i = size-5;
		mem[i+1] = 0xba;	/* may be on byte boundary */
		mem[i+2] = 0xbe;
		mem[i+3] = 0xc0;
		mem[i+4] = 0xed;
	} else
		i = size-1;

	for (; i >= size-postsize; i--)
		mem[i] = fillchar;

	/* so snoop shows reasonable stuff */
	mem  += presize;
	size -= presize + postsize;

	if (snoop) kprintf(amfmt,mem,size,flags,
				ret,ret,SysBase->ThisTask->tc_Node.ln_Name);

	return mem;
}

void ASM
MyFreeMem (REG(a1) UBYTE *mem, REG(d0) ULONG size, REG(a0) void *ret)
{
	struct ExecBase *SysBase = (void *) getreg(REG_A6);
	LONG errors = 0;
	LONG posterror = FALSE;
	LONG i;

	/* if wrong task, ignore */
	if (process && (process != ((struct Process *) (SysBase->ThisTask))))
	{
		(*old_FreeMem)(mem,size);
		return;
	}

	size += presize + postsize;
	mem -= presize;

	if (postsize >= 4 && (mem[size-4] != 0xba || mem[size-3] != 0xbe ||
			      mem[size-2] != 0xc0 || mem[size-1] != 0xed))
	{
		posterror = TRUE;
	}

	if (presize >= 4)
	{
		/* check coedbabe */
		if (*((LONG *) mem) != 0xc0edbabe)
		{
			if (posterror || postsize < 4)
			{
old_alloc:			/* not our allocation */
				if (snoop)

				kprintf(fmfmt,0,mem+presize,size-(presize+postsize),
				   ret,ret,SysBase->ThisTask->tc_Node.ln_Name);

/*				(*old_FreeMem)(mem+presize,
					       size-(presize+postsize));
*/				return;
			} else {
				/* coedbabe trashed, babecoed ok */
				i = 0;
			}
		} else
			i = 4;	/* coedbabe cool */
	} else if (posterror)
		goto old_alloc;	/* can only check 1! */
	else
		i = 0;	/* no coedbabe */

	for (; i < presize; i++)
		if (mem[i] != fillchar)
			errors++;

	if (errors)
	{
	kprintf("Bytes before alloc at 0x%lx(+0x%lx), size $%lx were hit!\n",
			mem,presize,size);
		kprintf("<$: ");
		for (i = 0; i < presize; i++)
		{
			if (i%4 == 0)
				kprintf(" ");
			kprintf("%s%s",hex((char) (mem[i] >> 4)),hex(mem[i]));
		}
		kprintf("\n");
		kprintf("	FreeMem attempted by %s  from 0x%lx\n",
					SysBase->ThisTask->tc_Node.ln_Name,ret);

		/* don't free (?) */
		return;
	}

	/* Check post-bytes */

	if (posterror || postsize < 4)	/* no babecoed or trashed */
		i = size-1;
	else
		i = size-5;	/* babecoed ok*/

	errors = 0;
	for (; i >= size-postsize; i--)
		if (mem[i] != fillchar)
			errors++;

	/* if all errors assume allocated before me */
	if (errors)
	{
		kprintf(
		  "Bytes after alloc at 0x%lx(+0x%lx), size $%lx were hit!\n",
			mem,postsize,size);
		kprintf(">$: ");
		for (i = 0; i < postsize; i++)
		{
			if (i%4 == 0)
				kprintf(" ");
			kprintf("%s%s",hex((char) (mem[size-postsize+i] >> 4)),
				       hex(mem[size-postsize+i]));
		}
		kprintf("\n");
		kprintf("	FreeMem attempted by %s  from 0x%lx\n",
				SysBase->ThisTask->tc_Node.ln_Name,ret);

		return;	/* don't free (?) */
	}

	if (snoop)
		kprintf(fmfmt,0,mem+presize,size-(presize+postsize),
				   ret,ret,SysBase->ThisTask->tc_Node.ln_Name);

	(*old_FreeMem)(mem,size);
	return;
}

char *nums[] = {
 "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"
};

char *
hex (char byte)
{
	return nums[byte & 0x0f];
}

char brkstr[] = " \t\n";

/* return # of words in string */
int
maxword (string)
	register char *string;
{
	register int number = 0;

	string = stpblk(string) ;
	while (*string)
	{
		string += stcarg(string,brkstr) ;
		string = stpblk(string) ;
		number++;
	}
	return(number);
}

/* get arg # n from string */
/* returns ptr to new end  */

char *
getarg (dest,string,number,maxchars,last)
		 char *dest ;
	register char *string ;
	register int number ;
		 int *maxchars ;
	register int last ;
{
	register char *temp ;
		 char save ;

	string = stpblk(string) ;
	while (number-- > 0)
	{
		string += stcarg(string,brkstr) ;
		string = stpblk(string) ;
		last--;
	}

	temp = string;
	do {
		temp = stpblk(temp);
		temp += stcarg(temp,brkstr);
	} while (last-- > 0) ;

	if (temp == string)
		return (dest) ;
	save = *temp ;
	*temp = '\0' ;
	dest = cpymax(dest,string,maxchars) ;
	*temp = save ;
	return (dest) ;
}

char *
cpymax (dest,src,maxchars)
	register char *dest ;
	register char *src ;
	register int  *maxchars ;
{
   /*  works like strcpy, but returns ptr to end of str. */
   /*  don't copy more than maxchar characters */
   while (*src && *maxchars > 0)
   {
      *dest++ = *src++ ;
      (*maxchars)-- ;
   }
   *dest = '\0' ;
   return (dest) ;
}

