
;/* tnt.c - Execute me to compile me with Lattice 5.04
if not exists tnta.o
  Asm -iH:include/ -otnta.o tnta.asm
endif
LC -b0 -cfistq -v -j73 tnt.c
Blink FROM LIB:astartup.obj,tnt.o,tnta.o TO tnt LIBRARY LIB:amiga.lib,LIB:LC.lib
quit
*/

/*
 * tnt.c v36.11 - Installs tnta.asm trap handler for debugging task traps
 *
 *            Carolyn Scheppner   CBM  04/90
 *            Copyright 1990 Commodore-Amiga, Inc.
 *
 * mods 36_11: changed tnta.asm to come up on NULL window 
 */

/*
#define MYDEBUG
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <workbench/startup.h>

#ifdef LATTICE
#include <proto/all.h>
#include <stdlib.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

char *vers = "\0$VER: tnt 36.11";
char *Copyright = 
  "tnt v36.11\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";

#define LVOAddTask	(-282L)

LONG SetAllTraps(APTR tasktrap,APTR proctrap,
		 APTR oldttrap,APTR oldptrap,BOOL force);
void ListSkipped(LONG count);
void bye(UBYTE *s, LONG err);
void cleanup(void);
void printf(BYTE *fmt,...);
 

#define SBUFSZ  256

/* stuff in the assembler part */
extern  LONG startcode, endcode, useCountT, useCountW; 
extern  LONG ourAddTaskCode, realAddTaskAddr;

extern struct ExecBase *SysBase;

BOOL  FromWb;

struct Library *IntuitionBase = NULL;


struct TntPort {
   struct MsgPort tntport;
   ULONG          *Mem;
   ULONG          MemSize;
   APTR           OldTaskTrap;
   APTR           OldProcTrap;
   APTR		  OurTrap;
   APTR		  OurAddTaskCode;
   APTR		  RealAddTaskAddr;
   ULONG          *ptrToUseCountT;
   ULONG          *ptrToUseCountW;
   char           Name[48];
   };

struct TntPort *tntport;
char *tntportName = "CAS_CBM_Tnt_Port_v2.03";

/* some processes to check for good "default" dos trap code
 * We are not just using ourself because old wack leaves trapcode pointer
 */
#define GOODCNT 3
UBYTE *goodprocs[GOODCNT] = {"DF0","CON","RAM"};

#define SKARRAYSZ   40
struct Task *sktasks[SKARRAYSZ];

void main(int argc, char **argv)
    {
    struct Task *task;
    LONG   msize, skipped, k;
    ULONG  codesize, *reloc, *uptr;
    APTR   OldFunc;
    BOOL Resetting, Force;

    printf("%s\n",Copyright);
    FromWb = (argc==0) ? TRUE : FALSE;
   
    if((argc>2)||((argc>1)&&(*argv[1]=='?')))
	{
	if(!FromWb)
	    {
	    printf("Usage: %s [ force | off ]\n",argv[0]);
	    printf("force replaces even unknown (non-dos/exec) trap handlers\n"); 
	    }
	bye("",RETURN_WARN);
	}

    if(!(IntuitionBase=OpenLibrary("intuition.library",0)))
       bye("Can't open Intuition",RETURN_FAIL);

    Resetting = ((argc==2)&&(!(stricmp(argv[1],"off")))) ? TRUE : FALSE;
    Force = ((argc==2)&&(!(stricmp(argv[1],"force")))) ? TRUE : FALSE;

    Forbid();
    /* If present already, remove, whether resetting or installing */
    if(tntport = (struct TntPort *)FindPort(tntportName))
	{
	if((*(tntport->ptrToUseCountT))||(*(tntport->ptrToUseCountW)))
	    {
	    Permit();
	    bye("TNT may not be removed while in use",
                     RETURN_WARN);
	    }

	/* Try to unwedge */

        OldFunc = SetFunction(SysBase,LVOAddTask,tntport->RealAddTaskAddr);

	if(OldFunc != tntport->OurAddTaskCode)
	    {
	    /* Someone else has changed the vector */
	    /* We put theirs back - can't exit yet  */
	    SetFunction(SysBase, LVOAddTask, OldFunc);
	    Permit();
#ifdef MYDEBUG
	printf("OldFunc=$%lx, OurAddTaskCode=$%lx\n",
			OldFunc,tntport->OurAddTaskCode);
#endif
            bye("Can't remove TNT - another SetFunction present",
                     RETURN_WARN);
	    }
	else
	    {
	    /* Remove the rest */
	    SysBase->TaskTrapCode = tntport->OldTaskTrap;
	    skipped=SetAllTraps(tntport->OldTaskTrap,
			tntport->OldProcTrap,
			tntport->OurTrap,
			tntport->OurTrap,
			FALSE);
	    RemPort(tntport);
 	    while((*(tntport->ptrToUseCountT))||(*(tntport->ptrToUseCountW)))
		Delay(20);
	    FreeMem(tntport->Mem, tntport->MemSize);
	    }
	printf("Existing TNT task trap handler removed\n");
	if(skipped) ListSkipped(skipped);
	}
    else /* no port found */
	{
	if(Resetting)
	    {
	    printf("TNT not currently installed\n");
	    }
	}
    Permit();


    /* If not just resetting, install our traps */
    if(!Resetting)
	{
	codesize = (ULONG)&endcode - (ULONG)&startcode;
	msize = codesize+sizeof(struct TntPort)+16;

	if(!(reloc = (ULONG *)AllocMem(msize,MEMF_PUBLIC|MEMF_CLEAR)))
	    {
	    bye("Not enough memory",RETURN_FAIL);
	    }
	else
	    {
	    Forbid();
	    /* If we find another tntport at this point, Free ours and abort */
	    if(FindPort(tntportName))
		{
		Permit();
		FreeMem(tntport->Mem,tntport->MemSize);
		bye("Aborted... Another TNT installation occurred",
                         RETURN_WARN);
		}
	    else
		{
		/* Copy relocatable assembler code to alloc'd memory */
		CopyMem(&startcode,reloc,codesize);

		/* Next get address of alloc'd tntport and set up port */
		tntport = (struct TntPort *)((ULONG)reloc + (ULONG)codesize);

		tntport->ptrToUseCountT =
 		 (ULONG *)((ULONG)reloc+((ULONG)&useCountT-(ULONG)&startcode));

		tntport->ptrToUseCountW =
 		 (ULONG *)((ULONG)reloc+((ULONG)&useCountW-(ULONG)&startcode));

		tntport->Mem = (ULONG *)reloc;
		tntport->MemSize = msize;
		strcpy(tntport->Name,tntportName);
		tntport->tntport.mp_Node.ln_Name = tntport->Name;
		tntport->tntport.mp_Node.ln_Type = NT_MSGPORT;
		tntport->OurTrap = (APTR)reloc;

		/* Save old traps, Add our port, point task traps to our code */
		tntport->OldTaskTrap = SysBase->TaskTrapCode;

		for(k=0; k<GOODCNT; k++)
			{
			if(task = FindTask(goodprocs[k])) break;
			}
		if(!task) task = FindTask(NULL);
		tntport->OldProcTrap = task->tc_TrapCode;

		AddPort(tntport);
		skipped=SetAllTraps(tntport->OurTrap,
			    tntport->OurTrap,
			    tntport->OldTaskTrap,
			    tntport->OldProcTrap,
			    Force);

		/* Finally, SetFunction AddTask for new tasks */
		/* where relocated assembler AddTask wedge is */
		tntport->OurAddTaskCode = (APTR)
 		 ((ULONG)reloc+((ULONG)&ourAddTaskCode-(ULONG)&startcode));

		/* pointer to where we should stuff real AddTask address */
		uptr = (ULONG *) 
		  ((ULONG)reloc+((ULONG)&realAddTaskAddr-(ULONG)&startcode));

		/* SetFunction AddTask to OurAddTaskCode (relocated)
		 * Storing old AddTask address in relocated variable
		 */
		tntport->RealAddTaskAddr =
		 (APTR)SetFunction(SysBase,LVOAddTask,tntport->OurAddTaskCode);
		*uptr = (ULONG)(tntport->RealAddTaskAddr);
#ifdef MYDEBUG
printf("Reloc=$%lx, ourATCode=$%lx, realATAddr at $%lx contains $%lx\n",
			reloc, tntport->OurAddTaskCode, uptr, *uptr);

	printf("SetFunctioned Addtask from $%lx to $%lx\n",
		   tntport->RealAddTaskAddr, tntport->OurAddTaskCode);
#endif

		printf("TNT task trap handler installed\n");
		if(skipped)  ListSkipped(skipped);
		}
	    Permit();
	    }
	}
    cleanup();
    exit(RETURN_OK);
    }


void ListSkipped(LONG count)
{
LONG k;
struct Task *task;

    printf("%ld unknown traps not changed:\n",count);
    for(k=0; (k<count)&&(k<SKARRAYSZ); k++)
	{
	task = sktasks[k];
	printf("  Task=$%08lx  TrapCode=$%08lx  %s\n",
		task, task->tc_TrapCode, task->tc_Node.ln_Name);
	}
}

/*
 * Sets tasks to the tasktrap, processes to the proctrap
 * If force==false, it will only set those which currently equal
 *    one of the three supplied trap pointers (ie. known trap handlers)
 * If force==true, it will set all tasks and processes regardless
 *    of their current trap handler
 */
LONG SetAllTraps(APTR tasktrap, APTR proctrap, 
		 APTR oldttrap, APTR oldptrap, BOOL force)
{
    struct Task *task;
    APTR tc;
    UBYTE tt;
    LONG skipcnt = 0L;

    Disable();

    SysBase->TaskTrapCode = tasktrap;

    /* WaitList */
    for ( task = (struct Task *)SysBase->TaskWait.lh_Head;
          (NULL != task->tc_Node.ln_Succ);
          task = (struct Task *)task->tc_Node.ln_Succ)
        {
	tc = task->tc_TrapCode;
	tt = task->tc_Node.ln_Type;
 	if(force || (tc==oldttrap) || (tc==oldptrap))
	    {
            if(tt == NT_PROCESS) task->tc_TrapCode = proctrap;
            else if(tt == NT_TASK) task->tc_TrapCode = tasktrap;
	    }
	else
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
	tc = task->tc_TrapCode;
	tt = task->tc_Node.ln_Type;
 	if(force || (tc==oldttrap) || (tc==oldptrap))
	    {
            if(tt == NT_PROCESS) task->tc_TrapCode = proctrap;
            else if(tt == NT_TASK) task->tc_TrapCode = tasktrap;
	    }
	else
	    {
	    if(skipcnt < SKARRAYSZ) sktasks[skipcnt] = task;
	    skipcnt++;
	    }
        }

    /* Self (we know we are a process) */
    task = FindTask(NULL);
    tc = task->tc_TrapCode;
    if(force || (tc==oldptrap))
	{
	task->tc_TrapCode = proctrap;
	}
    else
	{
	if(skipcnt < SKARRAYSZ) sktasks[skipcnt] = task;
	skipcnt++;
	}
    Enable();
    return(skipcnt);
}

void bye(UBYTE *s, LONG err)
   {
   if((*s)&&(!FromWb))
      {
      printf("%s \n",s);
      }
   cleanup();
   exit(err);
   }

void cleanup()
   {
   if(IntuitionBase)  CloseLibrary(IntuitionBase);
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

