;/* owner.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 owner.c
Blink FROM LIB:c.o,owner.o TO owner LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

/*
 * Owner.c  -  By Michael Sinz
 *
 * Tries to find the owner of the memory location
 * given.
 *
 * Command format:
 *
 * OWNER [?] [v] <address> <address> <address> ...
 *
 * OWNER 0x5F3EA 0x678A0
 *
 * Each address given will be searched for in subsiquent
 * search functions.
 *
 * 36_11 mods:  added modlist search
 * 37_2  mods:  add SegTracker support
 * 37_3  mods:  fix mask test searching resmodules array
 */


#include	<exec/types.h>
#include	<exec/tasks.h>
#include	<exec/ports.h>
#include	<exec/memory.h>
#include	<exec/execbase.h>
#include	<exec/interrupts.h>
#include	<exec/resident.h>
#include	<libraries/dos.h>
#include	<libraries/dosextens.h>
#include	<clib/exec_protos.h>
#include	<string.h>
#include	<stdio.h>

/*
#include	"mydebug.h"
*/
#define D(x) ;

#ifdef LATTICE
int  CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
void chkabort(void) { return; }  /* really */
#endif

char *vers = "\0$VER: owner 37.3";
char *Copyright = 
  "owner v37.3\n(c) Copyright 1990-93 Commodore-Amiga, Inc.  All Rights Reserved";
char *versout = "owner 37.3  5/93";

#define MAXTRIES 100000

/* We will need SysBase... */
extern struct ExecBase *SysBase;

/***********************************************************************/
/***********************************************************************/

typedef char (* __asm SegTrack(register __a0 ULONG Address,
                               register __a1 ULONG *SegNum,
                               register __a2 ULONG *Offset));

struct  SegSem
        {
        struct  SignalSemaphore seg_Semaphore;
                SegTrack        *seg_Find;
        };

/*
 * Display what this version searches for
 */
void DisplaySearch(void)
{
	puts("Searches:\tTask Control Block");
	puts("\t\tTask Stack");
	puts("\t\tSegList");
	puts("\t\tPublic Message Ports");
	puts("\t\tMessages on public ports");
	puts("\t\ttc_MemEntry");
	puts("\t\tFreeList");
	puts("\t\tROM Modules");
}

/*
 * Display version
 */
void DisplayVersion(void)
{
	puts(versout);
}

/*
 * Display usage
 */
void DisplayUsage(char *CommandName)
{
	printf("\nUsage: %s [0x | $]address [0x | $]address ...\n",CommandName);
}

/***********************************************************************/
/***********************************************************************/

/*
 * This routine returns the next task in the list(s) of tasks.
 * This makes the two task lists look like one...
 */
struct Task *NextTask(struct Task *lastTask)
{
register	struct	Task	*task;

	if (lastTask)
	{
		task=(struct Task *)(lastTask->tc_Node.ln_Succ);
		if (task==(struct Task *)&(SysBase->TaskReady.lh_Tail))
		{
			task=NextTask((struct Task *)&(SysBase->TaskWait));
		}
		else if (!task->tc_Node.ln_Succ) task=NULL;
	}
	else task=NextTask((struct Task *)&(SysBase->TaskReady));

	D(bug("NextTask: task %s\n",task->tc_Node.ln_Name));

	return(task);
}

/*
 * This routine uses the NextTask routine to find the next
 * process in the list...
 */
struct Process *NextProcess(struct Process *lastProc)
{
register	struct	Process	*proc;

	if (proc=(struct Process *)NextTask((struct Task *)lastProc))
	{
		if (proc->pr_Task.tc_Node.ln_Type!=NT_PROCESS)
		{
			proc=NextProcess(proc);
		}
	}
	D(bug("NextProcess: task %s\n",((struct Task *)proc)->tc_Node.ln_Name));
	return(proc);
}

/*
 * Adds a HEX string to the end of the string...
 */
void ToHex(char *string,ULONG value)
{
register	char	*t;
register	char	c;

	strcat(string,"00000000");
	t=&(string[strlen(string)]);
	while (value)
	{
		c=(value & 15);
		value=value >> 4;
		*--t+=c + (c > 9 ? 7 : 0);
	}
}

/*
 * Adds the start line to a result...
 */
void AddStart(char *string,ULONG value)
{
	strcat(string,"\n");
	ToHex(string,value);
	strcat(string," - ");
}

/*
 * Adds the name of the process/task to the string...
 */
void AddTaskName(char *string,struct Task *task)
{
register	struct	Process			*proc=NULL;
register	struct	CommandLineInterface	*cli=NULL;
UBYTE *s=NULL, buf[16];

	D(bug("AddTaskName: %s\n",task->tc_Node.ln_Name));

	if (task->tc_Node.ln_Type==NT_PROCESS)
	{
		proc=(struct Process *)task;
		if (proc->pr_CLI) cli=(struct CommandLineInterface *)BADDR(proc->pr_CLI);
	}


	if (proc) strcat(string,"Process");
	else strcat(string,"Task   ");

	strcat(string,"  0x");
	ToHex(string,(ULONG)task);
	strcat(string,": ");

	if (task->tc_Node.ln_Name) strcat(string,task->tc_Node.ln_Name);
	else strcat(string,"<noname>");
	if (cli)
	{
		strcat(string," (");
		if (cli->cli_CommandName)
			{
			s = (UBYTE *)(BADDR(cli->cli_CommandName));
			if (s[0]) strcat(string,&s[1]);
			else s = 0;
			}
		if(s==0)
			{
			sprintf(buf,"%ld",proc->pr_TaskNum);
			strcat(string,buf);
			}
		strcat(string,")");
	}
}

/*
 * Given a port address, it will add all the information it can
 * to the given string...
 */
void AddPortName(char *string,struct MsgPort *port)
{
register	struct	Interrupt	*intr;

	D(bug("AddPortName: %s\n",port->mp_Node.ln_Name));

	strcat(string,"MsgPort ");
	ToHex(string,(ULONG)port);
	strcat(string,": ");
	if (port->mp_Node.ln_Name) strcat(string,port->mp_Node.ln_Name);
	else strcat(string,"<noname>");
	if ((port->mp_Flags & PF_ACTION)==PA_SIGNAL)
	{
		if (port->mp_SigTask)
		{
			strcat(string," signals to\n\t\t\t  ");
			AddTaskName(string,port->mp_SigTask);
		}
		else strcat(string," No SigTask");
	}
	else if ((port->mp_Flags & PF_ACTION)==PA_SOFTINT)
	{
		if (intr=(struct Interrupt *)(port->mp_SigTask))
		{
			strcat(string," PA_SOFTINT\n\t\t\tCode: 0x");
			ToHex(string,(ULONG)(intr->is_Code));
			strcat(string,"  Data: 0x");
			ToHex(string,(ULONG)(intr->is_Data));
		}
		else strcat(string," No Interrupt structure");
	}
	else strcat(string," No action type");
}

/*
 * FindInTask
 *
 * This routine check if the value is within a task/process
 * structure...
 */
short FindInTask(ULONG value,char *string)
{
register		short	FoundFlag=FALSE;
register	struct	Task	*task;
register		ULONG	tmp, keepgoing = MAXTRIES;

	D(bug("FindInTask:\n"));

	Disable();

	task=NextTask(NULL);
	while (task)
	{

		if (value>=(tmp=(ULONG)task))
		{
			if (task->tc_Node.ln_Type==NT_PROCESS)
			{
				tmp+=sizeof(struct Process);
			}
			else
			{
				tmp+=sizeof(struct Task);
			}
			if (value<tmp)
			{
				AddStart(string,value);
				strcat(string,"in -TCB-    of ");
				AddTaskName(string,task);
				FoundFlag=TRUE;
			}
		}
		if(!keepgoing--) task = NULL;
		else task=NextTask(task);
	}

	Enable();
	return(FoundFlag);
}

/*
 * FindInStack
 *
 * This routine checks all of the task stacks if the value is an
 * address within the stack area.
 */
short FindInStack(ULONG value,char *string)
{
register		short	FoundFlag=FALSE;
register	struct	Task	*task;
register ULONG  keepgoing = MAXTRIES;

	D(bug("FindInStack:\n"));

	Disable();	/* No interruptions please!	*/

	task=NextTask(NULL);
	while (task)
	{
		if ( (((ULONG)(task->tc_SPLower)) <= value) &&
		     (((ULONG)(task->tc_SPUpper)) >= value)    )
		{
			AddStart(string,value);
			strcat(string,"in Stack    of ");
			AddTaskName(string,task);
			FoundFlag=TRUE;
		}
		if(!keepgoing--) task=NULL;
		else task=NextTask(task);
	}

	Enable();
	return(FoundFlag);
}

/*
 * CheckSegList
 *
 * Checks a SegList for the address value passed...
 *
 * There are two kludges in here:
 *
 *	1)	Current WorkBench seglist has a bad BPTR that
 *		causes a loop.  To check for that, if a BPTR in
 *		the seglist has any bits in the upper 2 bits,
 *		that BPTR is thrown out since that can never be
 *		a valid BPTR.  (Other seglists may also have
 *		this problem, but this was the first I found)
 *
 *	2)	The FComp program that Bill Hawes ships has
 *		a seg size that *VERY* large.  The program is
 *		not 200meg in a single segment like the seglist
 *		claims.  For this reason, any segment in the seglist
 *		that is over 16meg in size is thrown out.  This
 *		should not cause too many problems with current
 *		software.  (  ;-)  )
 */
short CheckSegList(ULONG value,BPTR SegList)
{
register	short	Found=FALSE;
register	ULONG	*ptr;
register	ULONG	keepgoing = MAXTRIES;

	D(bug("CheckSegList: BPTR $%lx\n",SegList));

	while (SegList)
	{
		if (SegList>>30) SegList=NULL;
		else
		{
			ptr=(ULONG *)BADDR(SegList);
			SegList=ptr[0];
			ptr--;
			if (ptr[0]>>24) SegList=NULL;
			else
			{
				if ((((ULONG)ptr)<=value)&&
				    ((((ULONG)ptr)+ptr[0])>value))
				{
					Found=TRUE;
					SegList=NULL;
				}
			}
		}
	if(!keepgoing--)	SegList = NULL;
	}
	return(Found);
}

/*
 * FindInSegList
 *
 * This routine looks at processes to check if the value is within
 * the seglist for that process...
 */
short FindInSegList(ULONG value,char *string)
{
register	short	FoundFlag=FALSE;
register	struct	Process	*process;
register	BPTR	SegList;
register	ULONG keepgoing = MAXTRIES;
struct CommandLineInterface	*cli;
struct SegSem *segsem;
ULONG  hunk,offset;
UBYTE  *name;
int    l;

    D(bug("FindInSegList:\n"));

    segsem = (struct SegSem *)FindSemaphore("SegTracker");
    if(segsem)
	{
	Forbid();
	if(name=(*segsem->seg_Find)(value,&hunk,&offset))
	    {
	    AddStart(string,value);
	    FoundFlag=TRUE;
	    strcat(string,"in SegList  of ");
	    strcat(string,name);
	    l=strlen(string);
	    sprintf(&string[l],"   Hunk %04lx Offset %08lx",hunk,offset);
	    }
	Permit();
	}


	/* let's do both - the Process info is also useful */
        Disable();	/* No interruptions please!	*/

	process=NextProcess(NULL);
	while (process)
	{
		SegList=NULL;
		if (cli=(struct CommandLineInterface *)BADDR(process->pr_CLI))
		{
			SegList=cli->cli_Module;
		}
		if (!SegList)
		{
			if (SegList=process->pr_SegList) SegList=((ULONG *)BADDR(SegList))[3];
		}
		if (SegList)
		{
			if (CheckSegList(value,SegList))
			{
				AddStart(string,value);
				FoundFlag=TRUE;
				strcat(string,"in SegList  of ");
				AddTaskName(string,&(process->pr_Task));
			}
		}
		if(!keepgoing--) process = NULL;
		else process=NextProcess(process);
	}
	Enable();


    return(FoundFlag);
}

/*
 * FindInMemEntry
 *
 * This routine checks all of the task tc_MemEntry items
 * for containing the address given...
 */
short FindInMemEntry(ULONG value,char *string)
{
register		short	FoundFlag=FALSE;
register	struct	Task	*task;
register	struct	MemList	*mem;
register		ULONG	tmp, keepgoing = MAXTRIES;
register		UWORD	loop;

	D(bug("FindInMemEntry:\n"));

	Disable();

	task=NextTask(NULL);
	while(task)
	{
		if (mem=(struct MemList *)task->tc_MemEntry.lh_Head) while ((mem->ml_Node.ln_Succ)&&(!FoundFlag))
		{
			for (loop=0;(loop<mem->ml_NumEntries)&&(!FoundFlag);loop++)
			{
				if (value>=(tmp=(ULONG)(mem->ml_ME[loop].me_Un.meu_Addr)))
				{
					if (value<(tmp+mem->ml_ME[loop].me_Length))
					{
						FoundFlag=TRUE;
						AddStart(string,value);
						strcat(string,"in MemList  of ");
						AddTaskName(string,task);
					}
				}
			}
			mem=(struct MemList *)mem->ml_Node.ln_Succ;
		}
		if(!keepgoing--) task=NULL;
		else task=NextTask(task);

	}

	Enable();
	return(FoundFlag);
}

/*
 * FindInMessages
 *
 * Looks through all of the public message ports and the messages
 * waiting at each.
 */
short FindInMessages(ULONG value,char *string)
{
register		short	FoundFlag=FALSE;
register	struct	MsgPort	*port;
register	struct	Message	*msg;
register	ULONG	keepgoing;

	D(bug("FindInMessages:\n"));

	Forbid();

	port=(struct MsgPort *)(SysBase->PortList.lh_Head);
	while (port->mp_Node.ln_Succ)
	{
		D(bug("FindInMessages: %s\n",port->mp_Node.ln_Name));
		if ((value>=((ULONG)port)) &&
		    (value<(sizeof(struct MsgPort)+(ULONG)port)))
		{
			AddStart(string,value);
			FoundFlag=TRUE;
			strcat(string,"in ");
			AddPortName(string,port);
		}
		else /* Check the messages on this port... */
		{
			Disable();
			keepgoing = MAXTRIES;
			msg=(struct Message *)(port->mp_MsgList.lh_Head);
			if(!msg)
			{
				D(bug("NULL mp_MsgList\n"));
			}
			else while ((msg->mn_Node.ln_Succ)&&(!FoundFlag)&&(keepgoing))
			{
				if ((value>=((ULONG)msg)) &&
				    (value<(max(msg->mn_Length,sizeof(struct Message))+((ULONG)msg))))
				{
					AddStart(string,value);
					FoundFlag=TRUE;
					strcat(string,"in a message waiting at ");
					AddPortName(string,port);
					if (msg->mn_ReplyPort)
					{
						strcat(string,"\n\t\tReply ");
						AddPortName(string,msg->mn_ReplyPort);
					}
				}
				keepgoing--;
				if (!FoundFlag) msg=(struct Message *)(msg->mn_Node.ln_Succ);
			}

			Enable();
		}
		port=(struct MsgPort *)(port->mp_Node.ln_Succ);
	}

	Permit();
	return(FoundFlag);
}

/*
 * FindInFreeList
 *
 * Checks to see if the value is in the free list...
 */
short FindInFreeList(ULONG value,char *string)
{
register		short		FoundFlag=FALSE;
register	struct	MemChunk	*chunk;
register	struct	MemHeader	*header;


	D(bug("FindInFreeList: %s\n",string));

	Forbid();

	header=(struct MemHeader *)(SysBase->MemList.lh_Head);
	while ((header->mh_Node.ln_Succ)&&(!FoundFlag))
	{
		chunk=(struct MemChunk *)&(header->mh_First);
		while ((chunk=chunk->mc_Next)&&(!FoundFlag))
		{
			if ((value>=((ULONG)chunk)) &&
			    (value<(chunk->mc_Bytes+((ULONG)chunk))))
			{
				FoundFlag=TRUE;
				AddStart(string,value);
				strcat(string,"in FreeList");
			}
		}
		if (!FoundFlag) header=(struct MemHeader *)(header->mh_Node.ln_Succ);
	}

	Permit();
	return(FoundFlag);
}


/*
 * FindInMods
 *
 * Checks to see if the value is in the resident modules
 */
#define ARRAYSIZE 80

short FindInMods(ULONG value,char *string)
{
extern struct ExecBase *SysBase;
register		short		FoundFlag=FALSE;
struct Resident **modp, *mod, *mods[ARRAYSIZE];
ULONG count = 0L, umod, romstart, romend, k, j;

    D(bug("FindInMods: %s\n",string));
    
    romstart = ((ULONG)(SysBase->LibNode.lib_IdString)) & 0xFFFF0000;
    romend = romstart + 0x80000;   /* kludge */

    modp = (struct Resident **)SysBase->ResModules;

    Forbid();
    while((modp)&&(mod = *modp))
	{
	umod = (ULONG)mod;
	if(umod & 0x80000000)
		{
		/* printf("extension\n"); */
		modp = (struct Resident **)(umod & 0x7FFFFFFF);
		}
        else
		{
		if(count < ARRAYSIZE)
		    {
		    k = 0;
		    if(count) for(k=0; (k<count)&&((ULONG)mod > (ULONG)mods[k]); k++);
		    if(k<count) for(j=count; j>k; j--) mods[j] = mods[j-1];
		    mods[k] = mod;
		    /* printf("inserted at k=%ld,%s",k,mod->rt_IdString); */
		    count++;
		    modp++;
		    }
		}
        }
    Permit();

    /* dummy end-of-last-256K-address for delimiter of last mod */
    mods[count] = (struct Resident *)((((ULONG)mods[count-1]) + 0x3ffff)& 0xfffc0000);
    
    for (k=0; (k<count)&&(!FoundFlag); k++)
	{
	mod = mods[k];
	D(bug("address $%08lx,next $%08lx,name:%s",mod,mods[k+1],mod->rt_IdString));

	if((value >= (ULONG)mod)&&
		((value < (ULONG)mods[k]->rt_EndSkip) ||
	        ((value > romstart)&&(value <= romend)&&(value < (ULONG)mods[k+1]))))
	    {
	    FoundFlag=TRUE;
	    AddStart(string,value);
	    strcat(string,"in resident module: ");
	    strcat(string,mod->rt_IdString);
	    }
	}
    if (count == ARRAYSIZE) printf("Error: mods array overflow\n");
    return (FoundFlag);
}


#define	STRING_BUFFER	4000L

/*
 * This routine takes and address and looks for the owner.
 * It will output the owner stats to stdout.
 */
void SearchFor(ULONG value)
{
char	*string;
short	flag=FALSE;

	if (string=AllocMem(STRING_BUFFER,MEMF_PUBLIC))
	{
		string[0]='\0';

		flag|=FindInMods(value,string);
		flag|=FindInTask(value,string);
		flag|=FindInStack(value,string);
		flag|=FindInSegList(value,string);
		flag|=FindInMessages(value,string);
		flag|=FindInMemEntry(value,string);
		flag|=FindInFreeList(value,string);

		if (!flag)
		{
			AddStart(string,value);
			strcat(string,"Not Found");
		}

		puts(string);
		FreeMem(string,STRING_BUFFER);
	}
}

/*
 * Get input and parse the values...
 */
void main(int argc, char *argv[])
{
register	SHORT	loop=1;
register	ULONG	value;
char		numbuf[512];

	if (argc)
	{
		if (argc<2) DisplayUsage(argv[0]);
		if (loop<argc)
		{
			if (argv[loop][0]=='?')
			{
				DisplayVersion();
				DisplayUsage(argv[0]);
				loop++;
			}
		}
		if (loop<argc)
		{
			if ((argv[loop][0]=='v')||(argv[loop][0]=='V'))
			{
				DisplayVersion();
				DisplaySearch();
				loop++;
			}
		}
		if (loop<argc)
		{
			printf("\nAddress  - Owner\n--------   -----");
			while (loop<argc)
			{
				if(argv[loop][0]=='$')
				{
					strcpy(numbuf,"0x");	
					strcat(numbuf,&argv[loop][1]);
					value=strtol(numbuf,NULL,0);
				}
				else value=strtol(argv[loop],NULL,0);
				if (value) SearchFor(value);
				loop++;
			}
		}
	printf("\n");
	}
}

