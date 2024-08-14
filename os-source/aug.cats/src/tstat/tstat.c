;/* tstat.c - Execute me to compile me
LC -b1 -cfistq -v -y -j73 tstat.c
Blink FROM LIB:c.o,tstat.o TO tstat LIBRARY LIB:LC.lib,LIB:Amiga.lib,LIB:debug.lib ND
quit
*
* 37.9  - serial option, Segtracker support
* 37.10 - fix for 68EC020
*/

#include <exec/types.h>
#include <exec/tasks.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern struct ExecBase *SysBase;
extern struct Library *DOSBase;
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#ifdef LATTICE
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

struct Process *findCommand(UBYTE *name);
BOOL matchCommand(struct Task *task, UBYTE *name);

extern struct DosLibrary *DOSBase;

#define WAITING "tstat: waiting for \"%s\"\033[K\n"
#define CURSOFF "\033[0 p"
#define CURSON  "\033[ p"
#define SWD 40
#define SBSZ 2048

#define TDELAY 5



typedef char (* __asm SegTrack(register __a0 ULONG Address,
                               register __a1 ULONG *SegNum,
                               register __a2 ULONG *Offset));

struct  SegSem
        {
        struct  SignalSemaphore seg_Semaphore;
                SegTrack        *seg_Find;
        };

struct SegSem *segsem;

UBYTE *curson   = CURSON;
UBYTE *cursoff  = CURSOFF;

UBYTE *clear = "\033[0J";
UBYTE *uptop = "\033[1;1H\033[K";
UBYTE *tobot = "\033[%ld;1H\033[K";
UBYTE repos[16];

UBYTE *pcfmt="@PC: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n";
UBYTE *hcfmt="@HC: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n";
UBYTE *spfmt="@SP: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n";

UBYTE *lclr = "\033[K";

UBYTE *pchunkfmt=
  "PC $%lx: seglist \"%s\" Hunk $%04lx Offset $%08lx%s\n";
UBYTE *sphunkfmt=
  "On stack $%lx: seglist \"%s\" Hunk $%04lx Offset $%08lx%s\n";
UBYTE *nohunkfmt=
  "No PC/Stack Hunk Info found (not loaded code or no SegTracker)%s\n";

UBYTE *ntypes[]={"UNKNOWN","TASK","INTERRUPT","DEVICE","MSGPORT",
                 "MESSAGE","FREEMSG","REPLYMSG","RESOURCE","LIBRARY",
                 "MEMORY","SOFTINT","FONT","PROCESS","SEMAPHORE"};
UBYTE *states[]={"INVALID","ADDED","RUN","READY","WAIT","EXCEPT","REMOVED"};

UBYTE Copyright[]=
"Tstat v37.10 (C) Copyright 1990-1993 Commodore-Amiga, Inc.  All Rights Reserved";

char *vers = "\0$VER: tstat 37.10";

BOOL ShowStack = FALSE, ShowPC = FALSE, ShowHunk = FALSE, ShowCode = FALSE;
BOOL Serial = FALSE, NoCtrl = FALSE;

#define BASELINES	10
#define STACKLINES	2
#define PCLINES		2
#define HUNKLINES	1
#define CODELINES	2

ULONG  lines=BASELINES;

#define DEPTH_P		32
#define DEPTH_S		72
#define DEPTH_C		32

ULONG P[DEPTH_P];
ULONG S[DEPTH_S];
ULONG C[DEPTH_C];

UBYTE thiscname[128], lastcname[128], taskname[80];

void KPutStr(UBYTE *);


void main(int argc, char **argv)
   {
   struct Library *eb = (struct Library *)SysBase;
   struct RootNode *rn;
   struct Process *pr;
   struct Task    *ta;
   struct CommandLineInterface *cli;
   UBYTE *bp1, *bp2;
   LONG sp, stsz, stu, tsl, tsu, lastsb, sb=0, chistu=0, histu = 0;
   ULONG cliNum, tdelay, cliPID, chipmem, fastmem, acaller;
   APTR *procIDs, tcode, ecode;
   UBYTE u, *cname, *hfmt;
   ULONG sigalloc, sigwait, sigrecvd, sigexcept;
   UBYTE ntype, state;
   BYTE  tdnest, idnest, pri, self=0;
   UWORD *wp;
   ULONG *lp, *regs, pc, hunk, offset, something;
   UBYTE framecheck, *bp, *segname, *sbuf = NULL, *whostat=NULL;
   BOOL	 HasFPU, Is040, Has881, Has882;
   BOOL  Found, NuStu, HaveSeg, Loop=FALSE;
   int k, si;

    if(!argc)  exit(RETURN_FAIL);

    HasFPU 	= (SysBase->AttnFlags & (AFF_68881|AFF_68882|AFF_FPU40))
			? TRUE : FALSE;

    Is040 	= (SysBase->AttnFlags & AFF_68040) ? TRUE : FALSE;
    Has881	= (SysBase->AttnFlags & AFF_68881) ? TRUE : FALSE;
    Has882	= (SysBase->AttnFlags & AFF_68882) ? TRUE : FALSE;

    tdelay=0;
    lastcname[0] = '\0';

    if((argc==2)&&(argv[1][0]=='?'))
      	{
	printf("%s\n",Copyright);
      	printf("Usage: %s [CliNum1-n|ExecTaskName|CliCommandName] [NOCTRL] [-tickdelay]\n",argv[0]);
	printf("More options: [SHOWSTACK] [SHOWPC] [SHOWHUNK] [SHOWHUNKCODE] [SERIAL]\n");
	printf("Use NOCTRL option when redirecting output to a file\n");
	printf("Example: %s Workbench -10\n",argv[0]);
      	exit(RETURN_OK);
      	}


    for(k=1; k<argc; k++)
	{
    	if(argv[k][0]=='-')
		{
		Loop = TRUE;
		tdelay = atoi(&argv[k][1]);
		}
	else if(!(stricmp(argv[k],"NOCTRL")))
		{
		NoCtrl = TRUE;
		}
	else if(!(stricmp(argv[k],"SHOWSTACK")))
		{
		ShowStack = TRUE;
		}
	else if(!(stricmp(argv[k],"SHOWPC")))
		{
		ShowPC = TRUE;
		}
	else if(!(stricmp(argv[k],"SHOWHUNK")))
		{
		if(!ShowHunk)
		    {
		    ShowHunk = TRUE;
		    }
		}
	else if(!(stricmp(argv[k],"SHOWHUNKCODE")))
		{
		ShowCode = TRUE;
		if(!ShowHunk)
		    {
		    ShowHunk = TRUE;
		    }
		}
	else if(!(stricmp(argv[k],"SERIAL")))
		{
		Serial = TRUE;
		}
	else if(!(stricmp(argv[k],"REMOTE")))
		{
		Serial = TRUE;
		}
	else whostat = argv[k];
	}

    if(ShowStack)	lines += STACKLINES;
    if(ShowHunk)	lines += HUNKLINES;
    if(ShowCode)	lines += CODELINES;
    if(ShowPC)		lines += PCLINES;


    if(NoCtrl)
	{
	strcpy(repos,"\n");
	lclr = "";
	curson = "";
	cursoff = "";
	}
    else
	{
	strcpy(repos,uptop);
	}


    segsem = (struct SegSem *)FindSemaphore("SegTracker");

    if(!(sbuf=(UBYTE *)AllocMem(SBSZ,MEMF_PUBLIC|MEMF_CLEAR)))
	{
	printf("Can't allocate sprintf buffer mem\n");
	exit(RETURN_FAIL);
	}

    si = 0;
    si += sprintf(&sbuf[si],"%s%s",repos,clear);

    Forbid();

again:

    ta = NULL;
    pr = NULL;
    cli = NULL;
    cliNum = NULL;
    Found = FALSE;
    NuStu = FALSE;

    /* Forbid used to be here */
    if(whostat)
	{
    	/* CLI number or task name */
      	cliNum = atoi(whostat);
      	if((cliNum > 20)&&(eb->lib_Version < 36))
            {
            printf("No active CLI #%ld (1.3)\n",cliNum);
	    Permit();
            exit(RETURN_WARN);
            }
      	if(cliNum)  /* CLI number */
            {
            rn = (struct RootNode *)DOSBase->dl_Root;
            procIDs = (APTR *)(BADDR(rn->rn_TaskArray));
            if(eb->lib_Version < 36)
		{
		cliPID = (ULONG)(procIDs[cliNum]);
            	if(!cliPID)
               	    {
		    if(Loop)	
			{
			if(!NoCtrl)
			    {
			    si += sprintf(&sbuf[si],WAITING,whostat);
			    Write(Output(),sbuf,si);
			    }
			goto waitfor;
			}
		    else
			{
                    	printf("No active CLI #%ld\n",cliNum);
	            	Permit();
                    	exit(RETURN_WARN);
			}
                    }
            	else pr = (struct Process *)(cliPID - sizeof(struct Task));
		}
	    else /* v36 */
		{
		if(!(pr = FindCliProc(cliNum)))
               	    {
		    if(Loop)	
			{
			if(!NoCtrl)
			    {
			    si += sprintf(&sbuf[si],WAITING,whostat);
			    Write(Output(),sbuf,si);
			    }
			goto waitfor;
			}
		    else
			{
                    	printf("No active CLI #%ld\n",cliNum);
	            	Permit();
                    	exit(RETURN_WARN);
			}
                    }
		}
            }
      	else   /* Exec task or CLI command name */
            {
            ta = (struct Task *)FindTask(whostat);
	    if(!ta)
		{
		ta = (struct Task *)findCommand(whostat);
		}
	    if(!ta)
               	{
		if(Loop)	
		    {
		    if(!NoCtrl)
			{
		    	si += sprintf(&sbuf[si],WAITING,whostat);
		    	Write(Output(),sbuf,si);
			}
		    goto waitfor;
		    }
		else
		    {
		    printf("Exec task or CLI command \"%s\" not found\n",whostat);
	            Permit();
                    exit(RETURN_WARN);
		    }
                }
	    else if(ta->tc_Node.ln_Type == NT_PROCESS)	pr=(struct Process *)ta;
            }
     	}
   else /* This task */
      	{
      	pr = (struct Process *)FindTask(NULL);;
	cliNum = pr->pr_TaskNum;
      	}

   if(pr) ta = (struct Task *)pr;

   if(ta == FindTask(NULL))	self=1;

   if(pr)
      {
      ta = (struct Task *)pr;
      cli = (struct CommandLineInterface *)(BADDR(pr->pr_CLI));
      }

   sigalloc = ta->tc_SigAlloc;
   sigwait = ta->tc_SigWait;
   sigrecvd = ta->tc_SigRecvd;
   sigexcept = ta->tc_SigExcept;
   pri = ta->tc_Node.ln_Pri;
   tsl = (LONG)ta->tc_SPLower;
   tsu = (LONG)ta->tc_SPUpper;
   tcode = ta->tc_TrapCode;
   ecode = ta->tc_ExceptCode;

   sp = (LONG)ta->tc_SPReg;

   bp = (UBYTE *)sp;

   if(HasFPU)
	{
   	framecheck = bp[0];
	if(!framecheck)	goto frestore;		/* FPU not in use */

	bp += 2;				/* stack holder */
	bp += 12;				/* FPU status registers */
	bp += 96;				/* FPU registers */ 

	if(framecheck != 0x90) goto notmid;	/* not mid-instruction */
	bp += 12;

notmid:
	bp += 2;

frestore:
	/* FRESTORE */
	if(framecheck == 0)		bp += 4;	/* FPU not in use */
	else if(framecheck == 0x90)			/* FPU mid-instruction */
	    {
	    if(Is040)		bp += 96;
	    else if(Has882)	bp += 216;
	    else if(Has881)	bp += 184;
	    }
	else						/* FPU idle */
	    {
	    if(Is040)		bp += 4;
	    else if(Has882)	bp += 60;
	    else if(Has881)	bp += 28;
	    }
	}

   wp = (UWORD *)bp;
   lp = (ULONG *)bp;

   if(!self)
	{
	pc = lp[0];
	sp = (ULONG)(&wp[3+(16<<1)]);
	}
   else pc = (ULONG)&main;

   regs = (ULONG *)(&wp[3]);


   if((ShowStack)||(ShowHunk)||(ShowCode))
	{   
   	bp1 = (UBYTE *)sp;
   	bp2 = (UBYTE *)&S[0]; 
   	for(k=0; k<(DEPTH_S<<2); k++) *bp2++ = *bp1++;
    	}

   if(ShowPC)
	{
   	bp1 = (UBYTE *)(pc);
   	bp2 = (UBYTE *)&P[0]; 
   	for(k=0; k<(DEPTH_P<<2); k++) *bp2++ = *bp1++;
	}

   strcpy(taskname,ta->tc_Node.ln_Name);
   ntype=ta->tc_Node.ln_Type;
   state=ta->tc_State;

   if(self)
	{
   	tdnest = SysBase->TDNestCnt;
   	idnest = SysBase->IDNestCnt;
	}
   else
	{
   	tdnest = ta->tc_TDNestCnt;
   	idnest = ta->tc_IDNestCnt;
	}

   /* adjust td and id nestcnts to Forbids and Disables */
/* revert to old behavior
   if(self) tdnest--;
   else if(state == TS_WAIT) idnest--;
   tdnest++;
   idnest++;
*/

   if(cli)
	{
   	cname = (UBYTE *)(BADDR(cli->cli_CommandName));
      	u = cname[0];
	strcpy(thiscname,&cname[1]);
	if(strcmp(lastcname,thiscname))   /* if a new/no command */
	    {
	    strcpy(lastcname,thiscname);  /* get the name for next compare */
	    NuStu = TRUE;
	    }
	}
   else u=0;

   lastsb = sb;
   if((u)&&((sp < tsl)||(sp > tsu)))
	{
	sb = (LONG)pr->pr_ReturnAddr;
	stsz = ((LONG)cli->cli_DefaultStack) << 2;
	}
   else 
	{
	sb = tsu;
	stsz = sb - tsl;
   	}


   /* Permit used to be here */

   stu = sb - sp;
   if((u && NuStu)||(sb != lastsb)) chistu=histu, histu = stu;
   if(stu > histu) histu = stu;

/*   if(u) chistu = histu; */



   /* Output */
   sbuf[si++] = '\n';
   if(cliNum) { si += sprintf(&sbuf[si]," CLI=%ld ",cliNum); }
   si += sprintf(&sbuf[si]," Taskname=%s  @$%lx  Type=%s  State=%s    \n",
     taskname, ta, ntypes[ntype], states[state]);
   si += sprintf(&sbuf[si]," Sig: Alloc=$%08lx Wait=$%08lx  Recvd=$%08lx Except=$%08lx\n",
        sigalloc, sigwait, sigrecvd, sigexcept);

   si += sprintf(&sbuf[si],
    " Priority=% d   TDNest=% d  IDNest=% d   TCode=$%08lx  ECode=$%08lx     \n",
           pri, tdnest, idnest, tcode, ecode);

   si += sprintf(&sbuf[si],
    " SP=$%08lx  tc_SPLower=$%08lx   tc_SPUpper=$%08lx     \n",sp,tsl,tsu);
/*
   si += sprintf(&sbuf[si],"F=%02lx ",framecheck);
*/
   si += sprintf(&sbuf[si],
      " PC=$%08lx  StkSize=%ld  StkUse=%ld HiUse=%ld LastHi=%ld       \n",
           pc, stsz, stu, histu, chistu);

/* for debugging 
for(k=0; k<8; k++) si += sprintf(&sbuf[si],"%08lx ",lp[k]);
si += sprintf(&sbuf[si],"\n");
for(k=0; k<8; k++) si += sprintf(&sbuf[si],"%08lx ",lp[k+8]);
si += sprintf(&sbuf[si],"\n");
for(k=0; k<8; k++) si += sprintf(&sbuf[si],"%08lx ",lp[k+16]);
si += sprintf(&sbuf[si],"\n");
for(k=0; k<8; k++) si += sprintf(&sbuf[si],"%08lx ",lp[k+24]);
si += sprintf(&sbuf[si],"\n");
puts(sbuf);
goto waitfor;
*/
         
   chipmem = AvailMem(MEMF_CHIP);
   fastmem = AvailMem(MEMF_FAST);
   si += sprintf(&sbuf[si]," Chip:%ld  Fast:%ld ",chipmem,fastmem);

   if(cli)
      {
      if(u>0)
         {
         si += sprintf(&sbuf[si]," Command: %s                   \n",thiscname);
         }
      else
         {
         si += sprintf(&sbuf[si]," Command: none                       \n");
         }
      }
   else { si += sprintf(&sbuf[si],"                                 \n"); }


   si += sprintf(&sbuf[si],"d0-7:");
   for(k=0; k<8; k++) { si += sprintf(&sbuf[si],"%08lx ",regs[k]); }
   si += sprintf(&sbuf[si],"\n");

   si += sprintf(&sbuf[si],"a0-6:");
   for(k=8; k<15; k++) { si += sprintf(&sbuf[si],"%08lx ",regs[k]); }
   si += sprintf(&sbuf[si],"\n");

if(ShowStack)
   {
   si += sprintf(&sbuf[si],spfmt,S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7]);
   si += sprintf(&sbuf[si],spfmt,S[8],S[9],S[10],S[11],S[12],S[13],S[14],S[15]);
/*
   si += sprintf(&sbuf[si],spfmt,S[16],S[17],S[18],S[19],S[20],S[21],S[22],S[23]);
   si += sprintf(&sbuf[si],spfmt,S[24],S[25],S[26],S[27],S[28],S[29],S[30],S[31]);
*/
   }

  if(ShowPC)
   {
   si += sprintf(&sbuf[si],pcfmt,P[0],P[1],P[2],P[3],P[4],P[5],P[6],P[7]);
   si += sprintf(&sbuf[si],pcfmt,P[8],P[9],P[10],P[11],P[12],P[13],P[14],P[15]);
/*
   si += sprintf(&sbuf[si],pcfmt,P[16],P[17],P[18],P[19],P[20],P[21],P[22],P[23]);
   si += sprintf(&sbuf[si],pcfmt,P[24],P[25],P[26],P[27],P[28],P[29],P[30],P[31]);
*/
   }

   if(ShowHunk)
    {
    something = 0L;
    HaveSeg = FALSE;
    acaller = (ULONG)pc;
    if(segsem)
	{
        if((TypeOfMem((APTR)acaller))&&
		(segname=(*segsem->seg_Find)(acaller,&hunk,&offset)))
	    {
	    HaveSeg = TRUE;
	    hfmt = pchunkfmt;
	    }
	else
	    {
	    for(k=0; k<DEPTH_S; k++)
		{
		if((TypeOfMem((APTR)S[k]))&&
			(segname=(*segsem->seg_Find)(S[k],&hunk,&offset)))
		    {
		    acaller=S[k];
		    if(!something) something=acaller;
		    if((*(UBYTE *)(acaller-4)) == 0x4E) /* JSR or JMP */
			{
		    	HaveSeg=TRUE;
		    	hfmt = sphunkfmt;
		    	break;
			}
		    }
		}
	    /* didn't find JMP or JSR on stack; use something else in seglist */
	    if((!HaveSeg)&&(something))
		{
		HaveSeg=TRUE;
		acaller=something;
		hfmt = sphunkfmt;
		}
	    }
	}
    if(HaveSeg)
	{
	si += sprintf(&sbuf[si],hfmt,acaller,segname,hunk,offset,lclr);
	if(ShowCode)
	    {
   	    bp1 = (UBYTE *)(acaller-4);
   	    bp2 = (UBYTE *)&C[0]; 
   	    for(k=0; k<(DEPTH_C<<2); k++) *bp2++ = *bp1++;
   	    si += sprintf(&sbuf[si],hcfmt,C[0],C[1],C[2],C[3],C[4],C[5],C[6],C[7]);
   	    si += sprintf(&sbuf[si],hcfmt,C[8],C[9],C[10],C[11],C[12],C[13],C[14],C[15]);
	    }
	}
    else
	{
    	hfmt = nohunkfmt;
	si += sprintf(&sbuf[si],hfmt,lclr);
	if(ShowCode)	si += sprintf(&sbuf[si],"%s\n%s\n",lclr,lclr);
	}
    }

   si += sprintf(&sbuf[si],curson);

   if(Serial)	KPutStr(sbuf);
   else 	Write(Output(),sbuf,si);

waitfor:
   if(Loop)
	{
	si = 0;
	if(tdelay) Delay(tdelay);
	if(!(SetSignal(0,0)&SIGBREAKF_CTRL_C))
		{
		si += sprintf(&sbuf[si],"%s%s",cursoff,repos);
		goto again;
		}
	}

   Permit();
   if(sbuf) FreeMem(sbuf,SBSZ);

   if(!NoCtrl)	printf(tobot,lines);
   printf(curson);
   exit(RETURN_OK);
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


struct Process *findCommand(UBYTE *name)
    {
    extern struct ExecBase *SysBase;
    struct Task *task;
    struct Process *proc = NULL;
    BOOL Found = FALSE;

    Disable();
    for ( task = (struct Task *)SysBase->TaskWait.lh_Head;
          (NULL != task->tc_Node.ln_Succ) && (!Found);
          task = (struct Task *)task->tc_Node.ln_Succ)
        {
	if(Found = matchCommand(task,name)) proc = (struct Process *)task;
        }
    for ( task = (struct Task *)SysBase->TaskReady.lh_Head;
          (NULL != task->tc_Node.ln_Succ) && (!Found);
          task = (struct Task *)task->tc_Node.ln_Succ)
        {
	if(Found = matchCommand(task,name)) proc = (struct Process *)task;
        }
    Enable();
    if(!Found)
	{
	if(Found = matchCommand(task=FindTask(NULL),name))
		proc = (struct Process *)task;
	}
    return(proc);
    }		


BOOL matchCommand(struct Task *task, UBYTE *name)
    {
    struct Process *proc;
    struct CommandLineInterface *cli;
    UBYTE *bname;
    BOOL Found = FALSE;

    if(task->tc_Node.ln_Type == NT_PROCESS)
	{
	proc = (struct Process *)task;
	if(proc->pr_CLI)
	    {
	    cli = BADDR(proc->pr_CLI);
	    if(cli->cli_CommandName)
		{
		bname = BADDR(cli->cli_CommandName);
		if(bname[0])
		    {
		    if(!(stricmp(&bname[1],name)))	Found=TRUE;
		    }
		}
	    }
	}
    return(Found);
    }
