;/* slime.c - Execute me to compile me with Lattice 5.04
if not exists slimea.o
HX68 -a slimea.asm -i INCLUDE: -o slimea.o
;Asm -iH:include/ -oslimea.o slimea.asm
endif
LC -b0 -cfistq -v -j73 slime.c
Blink FROM LIB:astartup.obj,slime.o,slimea.o TO slime LIBRARY LIB:Amiga.lib,LIB:LC.lib,LIB:debug.lib
quit
*/

/* Slime.c --- v37.4
 *
 * Copyright (c) 1990  Commodore Business Machines - All Rights Reserved
 *
 */

/*
#define MYDEBUG
*/

char *vers = "\0$VER: slime 37.4";
char Copyright[] =
 "slime v37.4\nCopyright (c) 1990  Commodore Business Machines  All Rights Reserved";
char *usage = "USAGE: slime [remote]\n";

#include <exec/types.h>
#include <exec/memory.h> 
#include <exec/io.h>
#include <exec/devices.h>
#include <exec/execbase.h>
#include <libraries/dos.h> 
#include <libraries/dosextens.h>

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <stdlib.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

typedef ULONG (*PFU)();
typedef VOID  (*PFV)();

extern int  sprintf __ARGS((char *,char *,...));
extern void kprintf __ARGS((char *,...));
extern void kputs   __ARGS((char *));
extern void printf  __ARGS((char *,...));

extern ULONG RealAddTask;
ULONG  NewAddTask;

/* the assembler entries in slimea.asm */
extern VOID  myAddTask(void);
extern VOID  myLaunch(void);
extern VOID  mySwitch(void);

extern VOID  vbget(void);

void MyLaunch(struct Task *);
void MySwitch(struct Task *);

LONG SetAllSlime(PFV newLaunch,PFV newSwitch,PFV oldLaunch,PFV oldSwitch,BOOL force);
char *ReadyTasks(char *buf, char *when);
void ListSkipped(LONG count);

char *nobody="";

extern LONG  usecnt;
extern ULONG vbcnt;

extern struct ExecBase  *SysBase;

int atoi( char *s );
void bye(UBYTE *s, LONG err);
void cleanup(void);
BOOL strEqu(UBYTE *p, UBYTE *q); 
int strLen(UBYTE *s);
void strCpy(UBYTE *to,UBYTE *from);
LONG getS(UBYTE *s);
LONG getchar(void);
UBYTE *taskname(struct Task *task);

extern PFV myVB;	/* assembler entry */
LONG MyVB(void);
void StartVBlank(void);
void StopVBlank(void);

#define BREAKSIG	SIGBREAKF_CTRL_C
#define WFULLSIG	SIGBREAKF_CTRL_E
#define FLUSHSIG	SIGBREAKF_CTRL_F

#define TOUPPER(c)      ((c)>='a'&&(c)<='z'?(c)-'a'+'A':(c)) 
#define HIGHER(x,y)     ((x)>(y)?(x):(y))

#define WBUFLEN   64000L
#define ABUFLEN   512L
#define EXTRALEN  512L

#define BUFALLOC (WBUFLEN + EXTRALEN + ABUFLEN + EXTRALEN)

char *noMem      = "Out of memory\n";
char *portName   = "cas_SLIME_PORT";

char *prevTaskName = NULL;

char mainTaskName[40];
char *wbuf = 0, *abuf=NULL;

struct Task   *otherTask, *mainTask;

struct MsgPort  *port; 

BOOL  Remote=FALSE, Told=FALSE, Full = FALSE, Done = FALSE;

ULONG wi;

UBYTE *intname = "<int/except>";

void genericOut(UBYTE *fmt, UBYTE *fnname, struct Task *task, char *ready);
UBYTE *gfmt = "%s: V=%ld\t I=%ld\t %ld:$%lx\t pri=%ld\t %s\n%s";

ULONG vblanks, seconds, minutes;
UBYTE vblankfreq;


#define SKARRAYSZ   40
struct Task *sktasks[SKARRAYSZ];

#define EXEC_ADDTASK	(-282L)



void MyLaunch(struct Task *task)
    {
    genericOut(gfmt,"LAUNCH",task,ReadyTasks(abuf,"L"));
    }

void MySwitch(struct Task *task)
    {
    genericOut(gfmt,"SWITCH",task,ReadyTasks(abuf,"S"));
    }


void genericOut(UBYTE *fmt, UBYTE *fnname, struct Task *task, char *ready)
    {
    UBYTE *name;
    BYTE pri = task->tc_Node.ln_Pri;

    if(!Remote)
	{
	if(wi < WBUFLEN)
	    {
    	    name = taskname(task);
	    sprintf(&wbuf[wi], fmt, fnname, vbcnt, SysBase->IdleCount,
				pri,task,pri,name,ready);
	    wi += strLen(&wbuf[wi]);
	    }
	else
	    {
   	    if(!Told) Told=TRUE,Signal(mainTask,WFULLSIG);
	    }
	}
    else
	{
    	name = taskname(task);

	sprintf(wbuf, fmt, fnname, vbcnt, SysBase->IdleCount,
			pri,task,pri,name,ready);
	kputs(wbuf);
	}
   }


/* This version designed to be called in supervisor mode
 * so always gives the name of the Task
 * and does not screen out callers using supervisor stack
 */
UBYTE *taskname(struct Task *task)
    {
    struct CommandLineInterface *cli;
    UBYTE  *s,*name;

    /* if our stack is in system stack, then we are an int or execption */
/*
    if((((APTR)&name) > SysBase->SysStkLower)&&
	(((APTR)&name) < SysBase->SysStkUpper))
		name = intname;
    else
*/
	{
    	name = task->tc_Node.ln_Name;
    	if((task->tc_Node.ln_Type == NT_PROCESS)
		&&(((struct Process *)task)->pr_CLI))
	    {
	    cli = (struct CommandLineInterface *)
			(BADDR(((struct Process *)task)->pr_CLI));
	    if(cli->cli_CommandName)
		{
	    	s = (UBYTE *)(BADDR(cli->cli_CommandName));
	    	if(s[0]) name = &s[1];
		}
	    }
	}
    return(name ? name : "<none>");
    }



void main(int argc, char **argv) 
   {
   ULONG signals;
   LONG file = NULL, skipped;
   UBYTE sbuf[128];
   BOOL force = FALSE;
   int k;

   if((argc)&&(argv[argc-1][0]=='?'))
	{
   	printf("%s\n%s\n",Copyright,usage), exit(0L);
	}


   if(argc>3)
   	{
   	for(k=3; k<argc; k++)
   	    {
	    if(strEqu(argv[k],"remote")) Remote = TRUE;
	    else if(strEqu(argv[k],"full")) Full = TRUE;
	    else printf("unknown option %s\n",argv[k]);
            }
        }


   vblankfreq = SysBase->VBlankFrequency;

   strCpy(&mainTaskName[0],"cas_SLIME");

   Forbid();
   if(otherTask = (struct Task *)FindTask(mainTaskName))
      	{
      	Permit();
      	printf("Slime is already monitoring.. exiting\n");
      	bye("",RETURN_WARN);
      	}

   mainTask = (struct Task *)FindTask(NULL);
   prevTaskName = mainTask->tc_Node.ln_Name;
   mainTask->tc_Node.ln_Name = mainTaskName;
   Permit();
     
   /* initialize */
   if(!(wbuf = (char *)AllocMem(BUFALLOC,MEMF_PUBLIC|MEMF_CLEAR)))
      bye("Can't allocate buffer\n",RETURN_FAIL);

   abuf = wbuf + WBUFLEN + EXTRALEN;
 
   wi = 0;    /* index into wbuf */


   /* Install wedges */

   Forbid();
   RealAddTask = (ULONG)SetFunction(SysBase, EXEC_ADDTASK, (PFU)myAddTask);
   skipped=SetAllSlime((PFV)myLaunch, (PFV)mySwitch, (PFV)NULL, (PFV)NULL, force);
   Permit();

   if(skipped)	ListSkipped(skipped);


  StartVBlank();

#ifdef MYDEBUG
	printf("AddTask was $%08lx now $%08lx\n",RealAddTask,myAddTask);
#endif

   printf("%s Slime monitoring installed\n",
		Remote ? "Remote" : "Local");
   
   printf("CTRL-C to remove monitor%s\n",
		Remote ? " " : ", CTRL-F to flush local buffer");

   while(!Done)
	{
      	signals = Wait(WFULLSIG | BREAKSIG | FLUSHSIG);

      	if(signals & WFULLSIG)   /* Local buffer full */
            {
	    printf("Slime buffer full.  CTRL-C to quit, CTRL-F to restart: ");
	    signals=Wait(BREAKSIG | FLUSHSIG);
	    }

	if(signals & FLUSHSIG)
	    {
	    wi = 0;
	    Told = FALSE;
	    seconds = 0;
	    vblanks = 0;
	    if(!Remote) printf("Slime local buffer flushed\n");
	    }

	if(signals & BREAKSIG)
	    {

#ifdef MYDEBUG
      printf("Got BREAKSIG\n");
#endif

	    StopVBlank();


            Forbid();

            NewAddTask = (ULONG)
			SetFunction(SysBase, EXEC_ADDTASK,(PFU)RealAddTask);

            if((ULONG)NewAddTask != (ULONG)myAddTask)
		{
                /* Someone else has changed the vectors */
                /* We put theirs back - can't exit yet  */
                SetFunction(SysBase, EXEC_ADDTASK, (PFU)NewAddTask);
		}
            else
                {
                Done = TRUE;
                }

	    /* remove our Launch and Switch things now anyway */
	    SetAllSlime((PFV)NULL, (PFV)NULL, (PFV)myLaunch, (PFV)mySwitch, force);

            Permit();

            if(!Done)
		{
		printf("Vectors changed - can't restore yet\n");
		printf("CTRL-C again to try exiting when ready...\n");
		Wait(BREAKSIG);
		}
	    else printf("\nSlime monitor removed\n"); 


again:
	    if(!Remote)
		{
	    	printf("\nFilename for output (<RET> for stdout): ");
	    	if(getS(sbuf))
		    {
		    if(file = Open(sbuf,MODE_NEWFILE))
		    	{
		    	Write(file,wbuf,wi);
		    	Close(file);
		        }
		    else
		    	{
		    	printf("Can't open %s\n",sbuf);
		    	goto again;
		    	}
		    }
	    	else
		    {
		    Write(Output(),wbuf,wi);
		    }
		}
            }
	}

   if(usecnt) printf("Slime: Waiting for final requests to complete...\n");
   while(usecnt) Delay(20);

   bye("",RETURN_OK);
   }


char *ReadyTasks(char *buf, char *when)
{
struct Task *task;
LONG count = 0, i;

    vbget();
    sprintf(buf,"%sReady: V=%ld\t I=%ld\t ",when,vbcnt,SysBase->IdleCount);
    i = strLen(buf);
    for ( task = (struct Task *)SysBase->TaskReady.lh_Head;
          (NULL != task->tc_Node.ln_Succ);
          task = (struct Task *)task->tc_Node.ln_Succ)
        {
	count++;
	sprintf(&buf[i],"%ld:$%lx ",task->tc_Node.ln_Pri,task);
	if (i<ABUFLEN)  i += strlen(&buf[i]);
	else sprintf(&buf[i]," OVERFLOW");
	}
    sprintf(&buf[i],"\n");
    return(count ? buf : nobody );
}

void ListSkipped(LONG count)
{
LONG k;
struct Task *task;

    printf("%ld unknown traps not changed:\n",count);
    for(k=0; (k<count)&&(k<SKARRAYSZ); k++)
	{
	task = sktasks[k];
	printf("  Task=$%08lx  launch=$%08lx  switch=$%08lx  %s\n",
		task, task->tc_Launch, task->tc_Switch, task->tc_Node.ln_Name);
	}
}


/*
 * Sets tasks Switch and Launch vectors to our wedges.
 * If force==false, it will only set those which currently equal
 *    the acceptable old value
 * If force==true, it will set all tasks and processes regardless
 *    of their Switch or Launch vectors
 */
LONG SetAllSlime(PFV newLaunch, PFV newSwitch, 
		 PFV oldLaunch, PFV oldSwitch, BOOL force)
{
    struct Task *task;
    LONG skipcnt = 0L;
    BOOL changed;

    Disable();
    /* WaitList */
    for ( task = (struct Task *)SysBase->TaskWait.lh_Head;
          (NULL != task->tc_Node.ln_Succ);
          task = (struct Task *)task->tc_Node.ln_Succ)
        {
	changed = TRUE;
	if(force | ( task->tc_Launch == oldLaunch))
	    {
	    task->tc_Launch = newLaunch;
	    if(newLaunch) task->tc_Flags  |= TF_LAUNCH;
	    else          task->tc_Flags  &= (~TF_LAUNCH);
	    }
	else changed = FALSE;

	if(force | ( task->tc_Switch == oldSwitch))
	    {
	    task->tc_Switch = newSwitch;
	    if(newSwitch) task->tc_Flags  |= TF_SWITCH;
	    else          task->tc_Flags  &= (~TF_SWITCH);
	    }
	else changed = FALSE;

	if(!changed)
	    {
	    if(skipcnt < SKARRAYSZ) sktasks[skipcnt] = task;
	    skipcnt++;
	    }
        }

    /* ReadyList */
    for ( task = (struct Task *)SysBase->TaskReady.lh_Head;
          (NULL != task->tc_Node.ln_Succ);
          task = (struct Task *)task->tc_Node.ln_Succ)
        {
	changed = TRUE;
	if(force | ( task->tc_Launch == oldLaunch))
	    {
	    task->tc_Launch = newLaunch;
	    if(newLaunch) task->tc_Flags  |= TF_LAUNCH;
	    else          task->tc_Flags  &= (~TF_LAUNCH);
	    }
	else changed = FALSE;

	if(force | ( task->tc_Switch == oldSwitch))
	    {
	    task->tc_Switch = newSwitch;
	    if(newSwitch) task->tc_Flags  |= TF_SWITCH;
	    else          task->tc_Flags  &= (~TF_SWITCH);
	    }
	else changed = FALSE;

	if(!changed)
	    {
	    if(skipcnt < SKARRAYSZ) sktasks[skipcnt] = task;
	    skipcnt++;
	    }
        }

    /* Self */
    task = FindTask(NULL);
        {
	changed = TRUE;
	if(force | ( task->tc_Launch == oldLaunch))
	    {
	    task->tc_Launch = newLaunch;
	    if(newLaunch) task->tc_Flags  |= TF_LAUNCH;
	    else          task->tc_Flags  &= (~TF_LAUNCH);
	    }
	else changed = FALSE;

	if(force | ( task->tc_Switch == oldSwitch))
	    {
	    task->tc_Switch = newSwitch;
	    if(newSwitch) task->tc_Flags  |= TF_SWITCH;
	    else          task->tc_Flags  &= (~TF_SWITCH);
	    }
	else changed = FALSE;

	if(!changed)
	    {
	    if(skipcnt < SKARRAYSZ) sktasks[skipcnt] = task;
	    skipcnt++;
	    }
        }

    Enable();
    return(skipcnt);
}

void bye(UBYTE *s, LONG err)
   {
   if(*s) printf(s);
   cleanup();
   exit(err);
   }

void cleanup()
   {
   if(wbuf)    FreeMem(wbuf,BUFALLOC);

   Forbid();
   if(prevTaskName) mainTask->tc_Node.ln_Name = prevTaskName;
   Permit();
   }


/* String functions */

BOOL strEqu(UBYTE *p, UBYTE *q)
   { 
   while(TOUPPER(*p) == TOUPPER(*q))
      {
      if (*(p++) == 0)  return(TRUE);
      ++q; 
      }
   return(FALSE);
   } 

int strLen(UBYTE *s)
   {
   int i = 0;
   while(*s++) i++;
   return(i);
   }

void strCpy(UBYTE *to,UBYTE *from)
   {
   do
      {
      *to++ = *from;
      }
   while(*from++);
   }


LONG getS(UBYTE *s)
    {
    int i=0;

    while((s[i] = getchar()) != '\n') i++;
    s[i] = '\0';
    return(i);
    }

int atoi( char *s )
   {
   int num = 0;
   int neg = 0;

   if( *s == '+' ) s++;
   else if( *s == '-' ) {
       neg = 1;
       s++;
   }

   while( *s >= '0' && *s <= '9' ) {
       num = num * 10 + *s++ - '0';
   }

   if( neg ) return( - num );
   return( num );
   }



LONG MyVB()
    {
    char *ready;

    ready = ReadyTasks(abuf," ");

    if(*ready)
	{
	if(Remote)	
	    {
	    kputs(ready);
	    }
	else
	    {
    	    if(wi < WBUFLEN)
	    	{
	    	sprintf(&wbuf[wi],ready);
	    	wi += strLen(&wbuf[wi]);
	    	}
	    else
	    	{
   	    	if(!Told) Told=TRUE,Signal(mainTask,WFULLSIG);
	    	}
	    }
	}

    return(0);
    }



/*
 *  Code to install/remove VB interrupt handler
 */

char myname[] = "CAS_slime_VB";  /* Name of interrupt handler */
struct Interrupt intServ;

void StartVBlank()  {
   intServ.is_Data = NULL;
   intServ.is_Code = (APTR)&MyVB;	/* the assembler entry */
   intServ.is_Node.ln_Succ = NULL;
   intServ.is_Node.ln_Pred = NULL;
   intServ.is_Node.ln_Type = NT_INTERRUPT;
   intServ.is_Node.ln_Pri  = 0;
   intServ.is_Node.ln_Name = myname;
   AddIntServer(5,&intServ);
   }

void StopVBlank() { RemIntServer(5,&intServ); }


/* end */

