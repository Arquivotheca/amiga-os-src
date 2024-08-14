
/* Wedge.c v 37.1 --- Carolyn Scheppner  CBM  11/90
 *
 * Link [A]startup.obj, Wedge.o, Wedgea.o... LIBRARY Amiga.lib [,LC.lib]
 *
 * Copyright 1990  Commodore Business Machines - All Rights Reserved
 *
 * modified 06/88 - not test wbstdio < 0  (astartup puts nil: in it)
 * 36_10 mods: add wait option in debug
 * 36_13 changes: latticize, add a option, 2 lines of pointed to
 * 36_15 changes: reverse direction of masks to a7...a0d7...d0
 * 36_16 changes: remove stack bounds check for CLI commandname reporting
 * 36_17 changes: version change just to match lvo version
 * 37_1  changes: SAS-6ify, use local RawDoFmt to avoid stack overflow
 *                when Locale is present; Add SegTracker support
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/ports.h>
#include <libraries/dos.h> 
#include <libraries/dosextens.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <hardware/custom.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>
#include <clib/alib_protos.h>

extern struct Library *SysBase;
extern struct Library *DOSBase;
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/icon_pragmas.h>

#include <stdlib.h>
#include <string.h>


VOID printf(UBYTE *fmt,...);
VOID sprintf(UBYTE *buffer,UBYTE *fmt,...);
int getchar(void);

/*
void strcpy(char *to,char *from);
int stccpy(char *to,char *from,int max);
int atoi( char *s );
int strlen(char *s);
*/

VOID kprintf(UBYTE *fmt,...);
VOID dprintf(UBYTE *fmt,...);

UBYTE *vers = "\0$VER: wedge 37.1";
UBYTE *Copyright = 
"wedge v37.1\n(c) Copyright 1990-93 Commodore-Amiga, Inc.  All Rights Reserved";

/* #define DEBUG */

typedef ULONG (*PFU)();

ULONG   MyFunc(ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,
	       ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG);
VOID MyFuncRet(ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,
	       ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG);

void usageExit(void);
void helpExit(void);
void cleanexit(char *s,int e);
void cleanup(void);
void forcemessage(char *s);
void message(char *s);
void wbHelp(void);
ULONG dummy(void);
void doAll(int com);
int isWedge(struct Task *t);
int getWbArgs(struct WBStartup *wbMsg);
void copyNoQuotes(char *d,char *s,int len);
BOOL testTT(char **ta,char *ms);
BOOL strEqu(char *p, char *q);
int getval(char *s);
LONG openWbStdio(char *conSpec);
void closeWbStdio(LONG wbStdio);
int ahtoi(char *s);

 
#define TOUPPER(c)      ((c)>='a'&&(c)<='z'?(c)-'a'+'A':(c)) 
#define HIGHER(x,y)     ((x)>(y)?(x):(y))

#define JMPINSTR (0x4ef9)

#define BREAK_SIG  SIGBREAKF_CTRL_C
#define ZAP_SIG    SIGBREAKF_CTRL_F

#define LISTALL 1
#define KILLALL 2

#define DORESULT (1<<16)
#define DODEBUG  (1<<17)

/* Normally, at entry to system function, ACALLER is stack[0] and
 * CCALLER is stack[2], but wedgea.asm pushed an extra placeholder
 * on stack, and then also JSR'd to the C code here
 */
#define ACALLER 2   /* StackPtr[0] = ACaller, StackPtr[1] = saved A6 in stub */
#define CCALLER 4   /* StackPtr[2] = CCaller */

extern struct ExecBase  *SysBase;

struct WBStartup *WBenchMsg;

extern VOID  myFunc(void);     /* The assembler entry */

/* Wedge output function */
VOID   (*outFunc)(UBYTE *fmt,...);
VOID   (*outFunc)();

typedef char (* __asm SegTrack(register __a0 ULONG Address,
                               register __a1 ULONG *SegNum,
                               register __a2 ULONG *Offset));

struct  SegSem
        {
	struct  SignalSemaphore seg_Semaphore;
	SegTrack        *seg_Find;
	};

struct SegSem *segsem;
	    
/* For WedgeMaster */
#define FORBIDOFFS (0xff7c)
#define PERMITOFFS (0xff76)
#define WAITOFFS   (0xfec2)
#define DEBUGOFFS  (0xff8e)

struct WedgeMaster {
   struct MsgPort wm_MsgPort;
   char           wm_Name[24];
   ULONG          wm_RealForbid;
   ULONG          wm_RealPermit;
   ULONG	  wm_RealWait;
   ULONG          wm_RealDebug;
   };


/* asm entries for wedge */
extern VOID myForbid(void), myPermit(void), myWait(ULONG), myDebug(ULONG);

ULONG  RealForbid, RealPermit, RealWait, RealDebug;
struct WedgeMaster *wm = NULL;
char  *wmNameText = "CAS_WedgeMaster";

extern struct Custom custom;

/* For stdio under WB */
extern LONG stdin, stdout, stderr;  /* in Astartup.obj */
#define CONCOMOFFS  25
char   conSpec[100] = "CON:0/40/640/140/ Wedge: ";
LONG   wbStdio = 0;

ULONG  Occupants = 0;
ULONG  RealFunc, OldFunc;
UWORD  DosVec[3];
UWORD  myDosVec[4];

UBYTE *HunkOffs = "%s:0x%08lx in seglist of \"%s\"   Hunk %04lx Offset %08lx\n";

char sbuf[160L];

#define HELPCNT     19
#define WBHELPOFFS   7
char *hline[HELPCNT] =
 {
 "Wedge  v37.1 --- Carolyn Scheppner  CBM  --- Copyright 1990  CBM\n",
 "\nCLI Usage:\n",
 "run Wedge lib offs [regs ptrs][opt fklnprasxz u t=tasks b=baud c=comment d=n]\n",
 "\nno \".library\" on lib; offset 0xfHHH; regs, ptrs 0xHHHH (d0=rightmost bit)\n",
 "opts: <f>orbid, <k>ill one wedge, <l>ocal, <n>oisy, <p>arallel output\n",
 "      <r>esult, <a>ddr Result, <s>tack report, E<x>clude tasklist, <u>nsafe\n",
 "      <z>ap, \"b=Baudrate\",\"c=Function & args\",\"d=Id\",\"t=Task[|Task...]\"\n\n",
 "WB ToolTypes: LIBRARY=name, OFFSET=0xfHHH, REGS=0xHHHH, PTRS=0xHHHH\n",
 "BAUD=rate,COMMENT=\"text\",DEBUG=Id,TASKS=\"tasks\", Booleans: EXCLUDE,\n",
 "FORBID,KILL,LOCAL,NOISY,PARALLEL,RESULT,ARESULT,STACK,UNSAFE,ZAP\n\n",
 "Notes: Local can only report on non-Forbidden Processes\n",
 "       TaskName \"System\" matches any code using system stack\n",
 "       ARESULT (a) means result is an address, show what it points to\n",
 "       Unsafe flag enables reporting regardless of available stack\n",
 "       To remove one wedge from CLI, rerun with opt k, CTRL/C, or BREAK\n",
 "       From WB, KILL Boolean allows separate icons for Kill and Install\n",
 "       Otherwise, double-clicking toggles wedge in and out\n",
 "       List all wedges with  Wedge List (LIST=TRUE)\n",
 "       Kill ALL wedges with  Wedge KillAll (KILLALL=TRUE)\n"
 };

char *moreHelp = "Type  wedge help  for more help\n";
char *noMem = "\nNot enough memory\n";
char *taskPrefix = "Wedge_CAS_";

char *libName, *comment = NULL, *prevTaskName = NULL;
char *tasks[32] = {0};

char *nodeTypes[] = { "UNKNOWN",  "TASK",    "INTERRUPT", "DEVICE",
                      "MSGPORT",  "MESSAGE", "FREEMSG",   "REPLYMSG",
                      "RESOURCE", "LIBRARY", "MEMORY",    "SOFTINT",
                      "FONT",     "PROCESS", "SEMAPHORE" };

/* Used in Local mode for exclusion of reporting on output handler */
struct MsgPort *oHand;
struct Task    *oHandTask;

#define MAXTNSZ   160L
#define MAXLNSZ    40L
#define MAXCMSZ    80L
#define MAXTLSZ   320L
#define COMOFFS    80L

/* Note - mainTaskName[] contains 80 extra elements so the user-supplied
 * comment can be stored in the last 80 bytes of the array.
 * The comment pointer will be set to point into this array.
 * This is so a  wedge list  can also output the comments for each wedge.
 * In addition, the libName pointer is set to point at the full library
 * name at the end of the task name.
 */
char  mainTaskName[MAXTNSZ];
char  wbTasks[MAXTLSZ], wbLib[MAXLNSZ];

ULONG  offs, baud, debugId;
ULONG  regmask = 0x0000, ptrmask = 0x0000, Unreported = 0;
struct Library *LibBase = NULL, *IconBase = NULL; 
UWORD  Id = 0, period = 0;

#define INSTALL 0
#define KILL    1
#define TOGGLE  2

UWORD Mode = INSTALL;  /* Modified by k flag or KILL ToolType */
BOOL  Done = FALSE, FromWb = FALSE, BcplDosLib = FALSE, DebugAgain = FALSE;
BOOL  Exclude = FALSE, ForbidIt = FALSE, Local = FALSE, Noisy = FALSE;
BOOL  Parallel = FALSE, Result = FALSE, ResultP=FALSE, Stack = FALSE, Unsafe = FALSE;
BOOL  Zap = FALSE, KillAll = FALSE, ListAll = FALSE, Help = FALSE, ShowSeg=TRUE;

struct Task    *otherTask, *mainTask;

char *auth = "Wedge by Carolyn A. Scheppner";
char *cprt =
 "Copyright (c) 1987  Commodore Business Machines  All Rights Reserved";

ULONG MyFunc(d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7)
ULONG  d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7;
   {
   /* need all locals but must keep stack usage down */
   struct Task *task;
   struct CommandLineInterface *cli;
   ULONG  ul[5], *reg;
   char   *taskName;
   UBYTE  *ub1, *ub2, k, j, DoIt;

   cli = NULL;
   DoIt = 1;

   /* If running on system stack, must be interrupt, etc */
   if((a7>(ULONG)(SysBase->SysStkLower)&&(a7<(ULONG)(SysBase->SysStkUpper))))
      {
      task = NULL;
      taskName = "SYSTEM";
      }
   else
      {
      task = SysBase->ThisTask;
      taskName = (char *)task->tc_Node.ln_Name;
      if(task->tc_Node.ln_Type == NT_PROCESS)
         cli = (struct CommandLineInterface *)
           ((((struct Process *)task)->pr_CLI) << 2);
      }


   /* Get command name if CLI command - Note - borrowing variables */
   ub2 = "";
   if(cli)
      {
      /* Use ub1 as C ptr to BSTR */
      ub1 = (UBYTE *)((cli->cli_CommandName) << 2);
      k = ub1 ? ub1[0] : 0;  /* Length of CommandName BSTR */
      /* If a command invoked from CLI */
      if(k)
         {
         /* TailPath - point ub1 at tail of command path */
         j=k;
         ub1 += j;
         while((j)&&(*ub1!='/')&&(*ub1!=':')) j--, ub1--;
         ub1++;
         /* Adjust name size k to tail size */
         k = k-j;
         ub2 = (UBYTE *)&ul[0]; /* copy CommandName to ul[] */
         for(j=0; (j<16)&&(j<k); j++)  ub2[j] = ub1[j];
         ub2[j] = NULL;
         /* Now ub2 points to command name truncated to 15 chars */
         }
      }


   /* If there is a task list
    * if Exclude, exclude listed tasks
    * else only report on listed tasks
    */
   if((DoIt)&&(tasks[0]))
      {
      if(Exclude)
         {
         for(k=0; tasks[k]; k++)
            {
            if((strEqu(tasks[k],taskName))
             ||((*ub2)&&(strEqu(tasks[k],ub2))))
               DoIt = 0;
            }
         }
      else
         {
         DoIt = 0;
         for(k=0; tasks[k]; k++)
            {
            if((strEqu(tasks[k],taskName))
             ||((*ub2)&&(strEqu(tasks[k],ub2))))
               DoIt = 1;
            }
         }
      }


   /* Check for enough stack */
   if((task)&&(!Unsafe)&&(DoIt))
      {
      if((a7 > (ULONG)task->tc_SPLower)&&(a7 < (ULONG)task->tc_SPUpper))
         {
         if(((ULONG)task->tc_SPLower + (Local?1800:300)) > a7)  DoIt = 0;
         }
      else if(cli)  /* Borrow ul[4] then replace possible NULL string end */
         {
         ul[4]=(ULONG)((struct Process *)task)->pr_ReturnAddr; /* Upper */
         if((a7 > (ul[4] - (cli->cli_DefaultStack << 2)))
          &&(a7 < ul[4])
          &&((ul[4]-(cli->cli_DefaultStack << 2)+(Local?1800:300)) > a7))
            DoIt = 0;
         ul[4] = NULL;
         }
      if(!DoIt)  Unreported++;  /* Unreported due to small stack */
      }


   /* Local can only report if non-Forbidden process
    * and cannot report on handler doing output
    */
   if((Local==TRUE)&&(DoIt))
      {
      if((SysBase->TDNestCnt < 0)
        &&(SysBase->IDNestCnt < 0)
        &&(task)
        &&(task->tc_Node.ln_Type == NT_PROCESS)
        &&(!((ULONG)task==(ULONG)oHandTask)))
         DoIt = 1;
      else
         {
         DoIt = 0;
         Unreported++;  /* Unreported because can't Local */
         }
      }


   if(DoIt)
      {
      reg = (ULONG *)&d0;

      /* Borrow ub1 for Disable/Forbidden string */
      ub1 = "";
      if(SysBase->TDNestCnt >= 0) ub1 = " [F]";
      if(SysBase->IDNestCnt >= 0)
         {
         if(*ub1)  ub1=" [FD]";
         else      ub1=" [D]";
         }

      /* This Forbid won't be released until function completes
       * and MyFuncRet() is called.
       */
      if((ForbidIt)&&(Result)) myForbid();

      myForbid();  /* During Output */

      if(*comment)  (*outFunc)("\n%s\n", comment);
      else (*outFunc)("\n");

      if(task)
         {
         if(*ub2)
            {
            (*outFunc)("COMMAND: %s   %s: %s ($%lx)%s\n",
                ub2,nodeTypes[task->tc_Node.ln_Type], taskName, task, ub1);
            }
         else
            {
            (*outFunc)("%s: %s ($%lx)%s\n",
                nodeTypes[task->tc_Node.ln_Type], taskName, task, ub1);
            }
         }
      else (*outFunc)("%s\n", taskName);

      (*outFunc)("From A:$%08lx C:$%08lx\n",
		((ULONG *)a7)[ACALLER],((ULONG *)a7)[CCALLER]);

      if(segsem)
	 {
	 if(ub1 = (*segsem->seg_Find)(((ULONG *)a7)[ACALLER],&ul[0],&ul[1]))
	    {
	    (*outFunc)(HunkOffs,"APC",((ULONG *)a7)[ACALLER],ub1,ul[0],ul[1]);
	    }
	 if(ub1 = (*segsem->seg_Find)(((ULONG *)a7)[CCALLER],&ul[0],&ul[1]))
	    {
	    (*outFunc)(HunkOffs,"CPC",((ULONG *)a7)[CCALLER],ub1,ul[0],ul[1]);
	    }
	 }

      for(k=0; k<16; k++)
         {
         if(regmask & (0x0001 << k))
            {
            (*outFunc)(" %s%ld  $%08lx",
               (k<8) ? "d" : "a", (k<8) ?  k  : k-8, reg[k]);

            if(ptrmask & (0x0001 << k))
               {
               if(reg[k])
                  {
                  (*outFunc)(" -> $");
                  ub1 = (UBYTE *)(reg[k]);
                  ub2 = (UBYTE *)(&ul[0]);
                  /* Copy 16 bytes pointed at by reg to ul[] to align */
                  for(j=0; j<16; j++)  ub2[j] = ub1[j];
                  /* Output as hex ulong dump */
                  for(j=0; j<4; j++)  (*outFunc)("%08lx ",ul[j]);
                  (*outFunc)("  ");
                  /* Convert ul[] bytes to ASCII representation */
                  for(j=0; j<16; j++)
                     {
                     if((ub2[j] < 0x20)||(ub2[j] > 0x7e))  ub2[j] = '.';
                     }
                  ub2[j] = NULL;
                  /* Output ASCII representation */
                  (*outFunc)((char *)ul);

		  /* show next 16 bytes also */
                  (*outFunc)("\n               -> $");
                  ub1 = ((UBYTE *)(reg[k])) + 16;
                  ub2 = (UBYTE *)(&ul[0]);
                  /* Copy 16 bytes pointed at by reg to ul[] to align */
                  for(j=0; j<16; j++)  ub2[j] = ub1[j];
                  /* Output as hex ulong dump */
                  for(j=0; j<4; j++)  (*outFunc)("%08lx ",ul[j]);
                  (*outFunc)("  ");
                  /* Convert ul[] bytes to ASCII representation */
                  for(j=0; j<16; j++)
                     {
                     if((ub2[j] < 0x20)||(ub2[j] > 0x7e))  ub2[j] = '.';
                     }
                  ub2[j] = NULL;
                  /* Output ASCII representation */
                  (*outFunc)((char *)ul);

                  }
               else (*outFunc)("    NULL Ptr");

               }
            (*outFunc)("\n");
            }
         }

      if(Stack)
         {
         /* NOTE - Borrowing j, k, ub1, ub2, ul[] here */

         if(task)
            {
            /* Output this for any task */
            (*outFunc)
             ("Pre-wedge a7=$%lx    Task Stack:  Lower=$%lx, Upper=$%lx\n",
               a7,task->tc_SPLower,task->tc_SPUpper);

            /* If command invoked from CLI process, show more */
            if(cli)
               {
               /* Use ub1 as C ptr to BSTR */
               ub1 = (UBYTE *)((cli->cli_CommandName) << 2);
      	       k = ub1 ? ub1[0] : 0;  /* Length of CommandName BSTR */

               /* If a command invoked from CLI */
               if((k)&&((a7>(ULONG)(task->tc_SPUpper))
                 ||(a7<(ULONG)(task->tc_SPLower))))
                  {
                  (*outFunc)
                   ("Command Stack: Base at startup=$%lx, Size=%ld\n",
                   ((struct Process *)task)->pr_ReturnAddr,
                   (cli->cli_DefaultStack)<<2);
                  }
               }
            }
         else  (*outFunc)("Stack:  Pre-wedge a7: $%lx\n", a7);
         }

      if(Unreported)
         {
         (*outFunc)("Unreported calls: %ld\n", Unreported);
         Unreported = 0;
         }

      /* Assign Id for every reported function. Id may not = 0 */
      /* ul[0]=Id, ul[1] = Id | bit 16 for Result | bit 17 for Debug */
      if(!(++Id)) Id++;
      ul[1] = ul[0] = Id;

      /* Only output Id if Result reporting is on */
      if(Result)
         {
         (*outFunc)("Result Id = %ld\n", Id);
         ul[1] |= DORESULT;   /* Mark Id for result reporting */
         }

      /* If this call should be Debug()'d, mark Id */
      if( task && ((debugId &&(ul[0]==debugId)) || DebugAgain ))
         {
         ul[1] |= DODEBUG;
         DebugAgain = 0;
         }

      myPermit();
      }
   else ul[1] = 0;  /* Not a DoIt */

   return(ul[1]);
   }


/* This should only be called by wedgea if a non-zero (DoIt) Id with
 * DORESULT (1<<16) bit set was returned by MyFunc.
 */
VOID MyFuncRet(d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,id)
ULONG  d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7,id;
   {
   /* need all locals but must keep stack usage down */
   ULONG justId;
   ULONG  ul[5];
   UBYTE  *ub1, *ub2, j;

   myForbid();
   /* Don't modify id on stack but ignore DORESULT and DODEBUG bits
    */
   if(*comment)  (*outFunc)("%s ", comment);
   justId = id & 0xffff;

   if(Result) (*outFunc)("Result %ld: $%08lx\n", justId, d0);
   if((ResultP)&&(d0))
		{
                  (*outFunc)("R %ld: $%08lx -> $", justId, d0);
                  ub1 = (UBYTE *)(d0);
                  ub2 = (UBYTE *)(&ul[0]);
                  /* Copy 16 bytes pointed at by reg to ul[] to align */
                  for(j=0; j<16; j++)  ub2[j] = ub1[j];
                  /* Output as hex ulong dump */
                  for(j=0; j<4; j++)  (*outFunc)("%08lx ",ul[j]);
                  (*outFunc)("  ");
                  /* Convert ul[] bytes to ASCII representation */
                  for(j=0; j<16; j++)
                     {
                     if((ub2[j] < 0x20)||(ub2[j] > 0x7e))  ub2[j] = '.';
                     }
                  ub2[j] = NULL;
                  /* Output ASCII representation */
                  (*outFunc)((char *)ul);

		  /* show next 16 bytes also */
                  (*outFunc)("\n               -> $");
                  ub1 = ((UBYTE *)(d0)) + 16;
                  ub2 = (UBYTE *)(&ul[0]);
                  /* Copy 16 bytes pointed at by reg to ul[] to align */
                  for(j=0; j<16; j++)  ub2[j] = ub1[j];
                  /* Output as hex ulong dump */
                  for(j=0; j<4; j++)  (*outFunc)("%08lx ",ul[j]);
                  (*outFunc)("  ");
                  /* Convert ul[] bytes to ASCII representation */
                  for(j=0; j<16; j++)
                     {
                     if((ub2[j] < 0x20)||(ub2[j] > 0x7e))  ub2[j] = '.';
                     }
                  ub2[j] = NULL;
                  /* Output ASCII representation */
                  (*outFunc)("%s\n",(char *)ul);
		}
   myPermit();
   if(ForbidIt)  myPermit();  /* Releases extra Forbid in MyFunc */
   }


void main(int argc, char **argv)
   { 
   ULONG signals;
   UWORD *uw;
   int k, f, lni;
   char *s, c, flag;

   FromWb = (argc==0) ? TRUE : FALSE;

   /* Where comment will be stored */
   comment = &mainTaskName[COMOFFS];

   if(FromWb)
      {
      WBenchMsg = (struct WBStartup *)argv;
      if(!(getWbArgs(WBenchMsg)))
         cleanexit("\nNeed LIBRARY and OFFSET ToolTypes\n",10);

      /* Set up conSpec in case of error,
       * but don't openWbStdio() yet because
       * we may have been clicked to remove wedge
       */
      c = comment[70];
      comment[70] = NULL;
      strcpy(&conSpec[CONCOMOFFS],comment);
      comment[70] = c;
      }
   else
      {
      if(argc>1)
         {
         if(strEqu(argv[1],"help"))    helpExit();
         if(strEqu(argv[1],"killall")) doAll(KILLALL), exit(0);
         if(strEqu(argv[1],"list"))    doAll(LISTALL), exit(0);
         }

      if(argc<3)  usageExit();

      libName = argv[1];
      offs = getval(argv[2]);  /* Check for valid offset later */

      if((argc>=5)&&((argv[3][0]=='0')&&(argv[4][0]=='0')))
         {
         regmask = getval(argv[3]);
         ptrmask = getval(argv[4]);
         k = 5;
         }
      else k = 3;

      while(k < argc)
         {
         if(strEqu("opt",argv[k]));
         else if(argv[k][1]=='=')
            {
            flag = (argv[k][0])|0x20;
            switch(flag)
               {
               case 'b':
                  baud = getval(&argv[k][2]);
                  break;
               case 'c':
                  stccpy(comment,&argv[k][2],MAXCMSZ);
                  break;
               case 'd':
                  debugId = getval(&argv[k][2]);
                  break;
               case 't':
                  tasks[0] = &argv[k][2];
                  break;
               default:
                  break;
               }
            }
         else
            {
            for(f=0; flag=argv[k][f]; f++)
               {
               flag |= 0x20;
               switch(flag)
                  {
                  case 'a':
                     Result = TRUE;
		     ResultP = TRUE;
                     break;
                  case 'b':
                     baud = 9600;
                     break;
                  case 'd':
                     debugId = 1;
                     break;
                  case 'f':
                     ForbidIt = TRUE;
                     break;
                  case 'k':
                     Mode = KILL;
                     break;
                  case 'l':
                     Local = TRUE;
                     break;
                  case 'n':
                     Noisy = TRUE;
                     break;
                  case 'p':
                     Parallel = TRUE;
                     break;
                  case 'r':
                     Result = TRUE;
                     break;
                  case 's':
                     Stack = TRUE;
                     break;
                  case 'u':
                     if((f=0)&&(argv[k][1]==NULL)) Unsafe = TRUE;
                     break;
                  case 'x':
                     Exclude = TRUE;
                     break;
                  case 'z':
                     Zap = TRUE;
                     break;
                  default:
                     break;
                  }
               }
            }
         k++;
         }
      }

   /* May be set by WB ToolType.  CLI option handled earlier */
   if(Help)     wbHelp(), cleanexit(" ",0);
   if(KillAll)  doAll(KILLALL), exit(0);
   if(ListAll)  doAll(LISTALL), cleanexit("\n",0);

   if((strEqu(libName,"exec"))&&
        ((offs==0xfe08)||(offs==0xfe02)||(offs==0xfdfc)||(offs==0xfdf6)))
    cleanexit("\nExec Raw IO functions may not be wedged\n",10);

   if((offs > 0xfffa)||(offs < 0xf000))
      cleanexit("\nIncorrect offset format - use hex like 0xff28\n",10);
 
   if((Local)&&(strEqu(libName,"dos"))&&(offs==0xffd0))
      cleanexit("\nWrite may not be wedged in Local mode\n",10);

   /* Can only zap on a Kill */
   if((Zap)&&(Mode != KILL))  Zap = FALSE;

   if(ShowSeg) segsem = (struct SegSem *)FindSemaphore("SegTracker");
		      
   /* If WedgeMaster port exists, grab real Forbid, Permit, and Debug
    * vectors from end of port name string.
    * Else create WedgeMaster port.
    */
   Disable();
   if(wm=(struct WedgeMaster *)FindPort(wmNameText))
      {
      /* Already is a Wedge Master */
      RealForbid    = wm->wm_RealForbid;
      RealPermit    = wm->wm_RealPermit;
      RealWait      = wm->wm_RealWait;
      RealDebug     = wm->wm_RealDebug;
      }
   else
      {
      /* Create WedgeMaster wm, no signal, no sigtask */
      if(!(wm =(struct WedgeMaster *)
         AllocMem(sizeof(struct WedgeMaster),MEMF_PUBLIC|MEMF_CLEAR)))
          Enable(), cleanexit(noMem,10);

      strcpy(wm->wm_Name,wmNameText);
      wm->wm_MsgPort.mp_Node.ln_Name = wm->wm_Name;
      wm->wm_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
      AddPort(wm);

      RealForbid = (ULONG)SetFunction(SysBase,FORBIDOFFS,(PFU)dummy);
      SetFunction(SysBase,FORBIDOFFS,(PFU)RealForbid);
      wm->wm_RealForbid = RealForbid;

      RealPermit = (ULONG)SetFunction(SysBase,PERMITOFFS,(PFU)dummy);
      SetFunction(SysBase,PERMITOFFS,(PFU)RealPermit);
      wm->wm_RealPermit = RealPermit;

      RealWait = (ULONG)SetFunction(SysBase,WAITOFFS,(PFU)dummy);
      SetFunction(SysBase,WAITOFFS,(PFU)RealWait);
      wm->wm_RealWait = RealWait;

      RealDebug = (ULONG)SetFunction(SysBase,DEBUGOFFS,(PFU)dummy);
      SetFunction(SysBase,DEBUGOFFS,(PFU)RealDebug);
      wm->wm_RealDebug = RealDebug;
      }
   Enable();


   /* Set up output function */
   if(Local && Parallel)  Local = FALSE;  /* Parallel overides Local */
   if(Local || Parallel)  debugId = 0;    /* Can only Debug() if Serial */
   if(Local)  outFunc = printf;
   else if(Parallel)  outFunc = dprintf;
   else
      {
      outFunc = kprintf;
      if(baud)  custom.serper = 1000000000L / (baud*280L) - 1L;
      }

   /* Parse tasks for multiple task names separated by '|' */
   if(tasks[0])
      {
      if(strEqu(tasks[0],"ALL"))  tasks[0] = NULL;
      else
         {
         for(k=1, s=tasks[0]; *s; k++)
            {
            while((*s)&&(*s != '|')) s++;
            if(*s == '|')
               {
               *s = NULL;
               s++;
               tasks[k] = s;
               }
            }
         }
      }

   /* Result will be mainTaskName = "Wedge_CAS_HHHH_whatever.library"
    *   with libName pointing to library name
    */
   strcpy(&mainTaskName[0],taskPrefix);
   sprintf(&mainTaskName[strlen(mainTaskName)],"%04lx_",offs);
   strcpy(&mainTaskName[lni=strlen(mainTaskName)],libName);
   strcpy(&mainTaskName[strlen(mainTaskName)],".library");
   libName = &mainTaskName[lni];

   Forbid();

   /* If already wedged, kill it or just refuse to re-wedge */
   if(otherTask=(struct Task *)FindTask(mainTaskName))
      {
      Permit();
      if(Mode==INSTALL)
         {
         sprintf(sbuf,"\nVector $%lx of %s already wedged.\n",offs,libName);
         cleanexit(sbuf,5);
         }
      else  /* KILL or TOGGLE */
         {
         if(Zap)  Signal(otherTask,BREAK_SIG|ZAP_SIG);
         else     Signal(otherTask,BREAK_SIG);
         cleanexit("",0);
         }
      }
   /* If trying to kill something not wedged, tell them about it */
   else if(Mode==KILL)
      {
      Permit();
      sprintf(sbuf,"\nVector $%lx of %s not currently wedged.\n",offs,libName);
      cleanexit(sbuf,5);
      }

   /* Still in Forbid() */
   mainTask = (struct Task *)FindTask(NULL);
   prevTaskName = mainTask->tc_Node.ln_Name;
   mainTask->tc_Node.ln_Name = mainTaskName;
   Permit();
     
   /* initialize */
   if((FromWb)&&(Local)&&(!(openWbStdio(conSpec))))  cleanexit("",10);

   /* Can't do this until any Local window is open */
   if(Local)
      {
      /* Get task of output handler for task exclusion */
      oHand=((struct FileHandle *)(stdout<<2))->fh_Type;
      oHandTask = (struct Task *)((ULONG)oHand - sizeof(struct Task));
      }

   if(!(LibBase = OpenLibrary(libName,0)))
      {
      sprintf(sbuf,"\nCan't open %s\n",libName);
      cleanexit(sbuf,10);
      }
   else
      {
      k = 0x10000 - offs;
      if((LibBase->lib_NegSize < k)||(k % 6))
         {
         CloseLibrary(LibBase);
         sprintf(sbuf,"\n$%lx is an invalid offset for %s\n",offs,libName);
         cleanexit(sbuf,10);
         }
      }

   /* Install function wedge */

   Disable();

   /* BcplDosLib = TRUE if non-standard dos.library jmp table */
   if(strEqu(libName,"dos.library"))
      {
      uw = (UWORD *)(((ULONG)LibBase) + ((WORD)offs));
      if(uw[0] != JMPINSTR) BcplDosLib = TRUE;
      }


   if(!BcplDosLib)
      {
      RealFunc    = (ULONG)SetFunction(LibBase, offs, (PFU)myFunc);
      }
   else /* dos.library function wedge */
      {
      /* Save the  MOVEQ #entry,d0   BRA action  in DosVec[3] */
      uw = (UWORD *)(((ULONG)LibBase) + ((WORD)offs));
      for(k=0; k<3; k++)  DosVec[k] = uw[k];

      /* Set up myDosVec with absolute JMP rather than BRA */
      myDosVec[0] = DosVec[0];  /* MOVEQ #entry, d0 */
      myDosVec[1] = JMPINSTR;
      *((ULONG *)&myDosVec[2]) = (ULONG)(&uw[2]) + DosVec[2];

      /* Replace the dos vector */
      /* Replace first word with JMP instruction */
      uw[0] = JMPINSTR;
      *((ULONG *)&uw[1]) = (ULONG)myFunc;
      /* Point RealFunc to our copied myDosVec */
      RealFunc = (ULONG)&myDosVec[0];
      }
   Enable();

   if(Noisy)
      {
      sprintf(sbuf,"Wedge into $%lx of %s installed.\n",offs,libName);
      forcemessage(sbuf);
      }

   while(!Done)
      {
      signals = Wait(BREAK_SIG|ZAP_SIG);

      Disable();

      if(!BcplDosLib)
         {
         OldFunc = (ULONG)SetFunction(LibBase, offs, (PFU)RealFunc);

         if((OldFunc != (ULONG)myFunc))
            {
            /* Someone else has changed the vector */
            /* We put theirs back - can't exit yet  */
            SetFunction(LibBase, offs, (PFU)OldFunc);
            }
         else
            {
            Done = TRUE;
            }
         }
      else  /* dos.library function */
         {
         /* If nobody else has changed vector, put back DosVec */
         if((ULONG)myFunc == *((ULONG *)(((ULONG)LibBase) + (WORD)offs + 2)))
            {
            uw = (UWORD *)(((ULONG)LibBase) + (WORD)offs);
            for(k=0; k<3; k++)  uw[k] = DosVec[k];
            Done = TRUE;
            }
         }
      Enable();

      if(!Done)
         {
         sprintf(sbuf,
            "\nVector $%lx in %s has changed. Can't unwedge right now.\n",
               offs,libName);
         forcemessage(sbuf);
         }
      else if(Occupants)
         {
         k = strlen(mainTaskName) + 1;
         mainTaskName[k] = 'z';
         Delay(50);
         while((Occupants)&&(!(signals & ZAP_SIG)))
            {
            sprintf(sbuf,"\nWedge into $%lx of %s still in use.\n", offs,libName);
            Forbid();
            forcemessage(sbuf);
            message("Retry opt k (KILL) or KillAll to unload safely.\n");
            message("Or use opt kz (KILL=TRUE, ZAP=TRUE) to unload unsafely.\n");
            Permit();
            signals = Wait(BREAK_SIG|ZAP_SIG);
            }
         }
      }

   CloseLibrary(LibBase);

   if(Unreported)  (*outFunc)("Unreported calls: %ld\n", Unreported);

   if(Noisy)
      {
      sprintf(sbuf,"\nWedge into $%lx of %s removed.\n", offs,libName);
      cleanexit(sbuf,0);
      }
   else cleanexit("",0);
   }



/* Cleanup and exits */

void usageExit()
   { 
   printf(hline[1]);
   printf(moreHelp);
   exit(RETURN_OK);
   } 

void helpExit()
   {
   int k;

   for(k=0; k<HELPCNT; k++)      printf(hline[k]);
   exit(RETURN_OK);
   } 

void cleanexit(char *s,int e)
   {
   if(*s) forcemessage(s);
   if((FromWb)&&(wbStdio)&&(*s))
      {
      message("\nPRESS RETURN TO EXIT: ");
      while(getchar() != '\n');
      }

   cleanup();
   exit(e);
   }

void cleanup()
   {
   if(wbStdio)  closeWbStdio(wbStdio);
   Forbid();
   if(prevTaskName) mainTask->tc_Node.ln_Name = prevTaskName;
   Permit();
   }


/* Other stuff */

void forcemessage(char *s)
   {
   if((*s)&&(FromWb)&&(!(wbStdio)))  openWbStdio(conSpec);
   message(s);
   }

void message(char *s)
   {
   if((*s)&&(stdout > 0))  printf(s);
   }

void wbHelp()
   {
   int k;

   for(k=WBHELPOFFS; k<HELPCNT; k++)  forcemessage(hline[k]);
   }

ULONG dummy()
 {
 return(0);
 }

void doAll(int com)
   {
   struct Task *wTasks[128], *t;
   char *s;
   int i, j, k = 0;

   /* We must get ptrs all wedge tasks first, since some may not be
    * able to exit now.  Otherwise, since Signal() causes rescheduling,
    * we could get stuck here finding and signalling same one repeatedly
    */

   Disable();

   for(t=(struct Task *)(SysBase->TaskWait.lh_Head);
        (t->tc_Node.ln_Succ)&&(k<127);
          t=(struct Task *)t->tc_Node.ln_Succ)
      {
      if(isWedge(t)) wTasks[k] = t, k++;
      }

   for(t=(struct Task *)(SysBase->TaskReady.lh_Head);
        (t->tc_Node.ln_Succ)&&(k<127);
         t=(struct Task *)t->tc_Node.ln_Succ)
      {
      if(isWedge(t)) wTasks[k] = t, k++;
      }

   wTasks[k] = 0;

   Enable();


   /* Each Signal or forcemessage can let other tasks in to run
    * so my array may become invalid.  So I make sure I can find
    * each before Signalling or outputting name
    */
   Forbid();
   if((wm=(struct WedgeMaster *)FindPort(wmNameText))&&(com==LISTALL))
      forcemessage("WedgeMaster installed\n");

   for(j=0,k=0; wTasks[k]; k++)
      {
      s = wTasks[k]->tc_Node.ln_Name;
      if(t = (struct Task *)FindTask(s))
         {
         if(com==KILLALL)  Signal(t, BREAK_SIG);
         else
            {
            j++;
            if(s[COMOFFS]) sprintf(sbuf,"%ld. %s\n",j,&s[COMOFFS]);
            else sprintf(sbuf,"%ld. %s\n",j,&s[strlen(taskPrefix)]);
            forcemessage(sbuf);
            i = strlen(s) + 1;
            if(s[i]=='z') forcemessage("    (Unwedged but not unloaded)\n");
            }
         }
      }

   /* If KILLALL and no wedges, delete  WedgeMaster */
   Disable();
   wm=(struct WedgeMaster *)FindPort(wmNameText);
   if((com==KILLALL)&&(k==0)&&(wm))
      {
      /* This port has no sigbit or sigtask */
      RemPort(wm);
      FreeMem(wm,sizeof(struct WedgeMaster));
      }
   Enable();
   Permit();
   }

int isWedge(struct Task *t)
   {
   char *s;
   int k;

   s = t->tc_Node.ln_Name;
   for(k=0; k<10; k++)
      {
      if(s[k] != taskPrefix[k])  return(0);
      }
   return(1);
   }


int getWbArgs(struct WBStartup *wbMsg)
   {
   struct WBArg  *wbArg;
   struct DiskObject *diskobj;
   char **toolarray;
   char *s;
   BOOL  GotLib=FALSE,GotOffs=FALSE;

   wbArg = wbMsg->sm_ArgList;
   /* If opened via project icon, use project's ToolTypes */
   if(wbMsg->sm_NumArgs > 1)  wbArg++;

   Mode = TOGGLE;  /* default WB Mode */

   if((IconBase = OpenLibrary("icon.library", 0)))
      {
      diskobj=(struct DiskObject *)GetDiskObject(wbArg->wa_Name);
      if(diskobj)
         {
         toolarray = (char **)diskobj->do_ToolTypes;

         if(s=(char *)FindToolType(toolarray,"LIBRARY"))
            {
            stccpy(wbLib,s,MAXLNSZ);
            libName = wbLib;
            GotLib=TRUE;
            }
         if(s=(char *)FindToolType(toolarray,"OFFSET"))
            offs = getval(s), GotOffs=TRUE;
         if(s=(char *)FindToolType(toolarray,"REGS"))
            regmask = getval(s);
         if(s=(char *)FindToolType(toolarray,"PTRS"))
            ptrmask = getval(s);
         if(s=(char *)FindToolType(toolarray,"BAUD"))
            baud = getval(s);
         if(s=(char *)FindToolType(toolarray,"COMMENT"))
            {
            copyNoQuotes(comment,s,MAXCMSZ);
            }
         if(s=(char *)FindToolType(toolarray,"DEBUG"))
            debugId = getval(s);
         if(s=(char *)FindToolType(toolarray,"TASKS"))
            {
            copyNoQuotes(wbTasks,s,MAXTLSZ);
            tasks[0] = wbTasks;
            }

         if(s=(char *)FindToolType(toolarray,"KILL"))
            {
            if(strEqu(s,"TRUE"))         Mode = KILL;
            else if(strEqu(s,"FALSE"))   Mode = INSTALL;
            }

         /* Bools */
         Help     = testTT(toolarray,"HELP");
         KillAll  = testTT(toolarray,"KILLALL");
         ListAll  = testTT(toolarray,"LIST");

         Exclude  = testTT(toolarray,"EXCLUDE");
         ForbidIt = testTT(toolarray,"FORBID");
         Local    = testTT(toolarray,"LOCAL");
         Noisy    = testTT(toolarray,"NOISY");
         Parallel = testTT(toolarray,"PARALLEL");
         Result   = testTT(toolarray,"RESULT");
         ResultP  = testTT(toolarray,"RESULTP");
	 if(ResultP)	Result=TRUE;
         Stack    = testTT(toolarray,"STACK");
         Unsafe   = testTT(toolarray,"UNSAFE");
         Zap      = testTT(toolarray,"ZAP");

         FreeDiskObject(diskobj);
         }
      CloseLibrary(IconBase);
      }
   if((Help)||(ListAll)||(KillAll)||(GotLib && GotOffs)) return(1);
   else return(0);
   }


void copyNoQuotes(char *d,char *s,int len)
   {
   if(*s=='"')  s++;
   len = stccpy(d,s,len);
   if(d[len-2]=='"')  d[len-2] = NULL;
   }

BOOL testTT(char **ta,char *ms)
   {
   char *s;

   if(s=(char *)FindToolType(ta,ms))
      {
      if(strEqu(s,"TRUE"))  return(TRUE);
      }
   return(FALSE);
   }



/* String functions */

BOOL strEqu(char *p, char *q) 
   { 
   while(TOUPPER(*p) == TOUPPER(*q))
      {
      if (*(p++) == 0)  return(TRUE);
      ++q; 
      }
   return(FALSE);
   } 

int getval(char *s)
   {
   if((s[1]|0x20) == 'x') return(ahtoi(&s[2]));
   return(atoi(s));
   }

#if 0
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

int strlen(char *s)
   {
   int i = 0;
   while(*s++) i++;
   return(i);
   }

void strcpy(char *to,char *from)
   {
   do
      {
      *to++ = *from;
      }
   while(*from++);
   }

/* Copies up to max including NULL
 * Returns count copied, including NULL
 */
int stccpy(char *to,char *from,int max)
   {
   int count = max;

   do
      {
      *to++ = *from;
      count--;
      }
   while((*from++)&&(count>1));

   if(count==1) *to = NULL, count=0;
   return(max-count);
   }
#endif /* 0 */

int ahtoi(char *s)
{
    int num = 0;
    int neg = 0;

    if( *s == '+' ) s++;
    else if( *s == '-' ) {
        neg = 1;
        s++;
    }
    while((*s >= '0' && *s <= '9')||((TOUPPER(*s) >= 'A')&&(TOUPPER(*s)<'G'))) {
        if( *s >= '0' && *s <= '9' )num = num * 16 + *s++ - '0';
   else {
      num = num*16 + TOUPPER(*s) - 'A'+10;
      s++;
   }
    }
    if( neg ) return( - num );
    return( num );
}


/* wbStdio.c --- Open an Amiga stdio window under workbench
 *               For use with Astartup.obj
 */

LONG openWbStdio(char *conSpec)
   {
   struct Process *proc;
   struct FileHandle *handle;

   if(wbStdio) return(wbStdio);

   if(!(wbStdio = Open(conSpec,MODE_NEWFILE)))  return(0);
   stdin  = stdout = stderr = wbStdio;
   handle = (struct FileHandle *)(wbStdio << 2);
   proc = (struct Process *)FindTask(NULL);
   proc->pr_ConsoleTask = (APTR)(handle->fh_Type);
   proc->pr_CIS = (BPTR)stdin;
   proc->pr_COS = (BPTR)stdout;
   return(wbStdio);
   }

void closeWbStdio(LONG wbStdio)
   {
   struct Process *proc;

   if (wbStdio)  Close(wbStdio);
   wbStdio = NULL;
   stdin  = -1;
   stdout = -1;
   stderr = -1;
   proc = (struct Process *)FindTask(NULL);
   proc->pr_ConsoleTask = NULL;
   proc->pr_CIS = NULL;
   proc->pr_COS = NULL;
   }

