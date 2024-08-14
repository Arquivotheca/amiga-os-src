;/* munglist.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfistq -v -j73 munglist.c
Blink FROM LIB:c.o,munglist.o,munge.o TO munglist LIBRARY LIB:LC.lib,LIB:Amiga.lib,LIB:debug.lib
quit

Based on MemList so there's probably a lot of excess baggage

37.3 - fix trashing of a memory chunk if buffer fills
37.4 - update to match Mungwall 37.61 and higher  (KeeperKey) and show
	address each task thinks it allocated, not what mungwall allocated
37.5 - add SHOWHUNK
37.7 - add SHOWTIME
37.8 - slight output changes
37.9 - time mark added to keeper port, let SegTracker screen TypeOfMem
37.10 - update help output, dynamic mark, mark and since mark together
37.11 - removing debugging line, add TOTAL taskname
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <libraries/dos.h>
#include <devices/timer.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/timer_protos.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LATTICE
int CXBRK(void)  { return(0); }  /* Disable Lattice CTRL/C handling */
void chkabort(void) { return; }  /* really */

extern struct Library *DOSBase;
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/timer_pragmas.h>
#endif


#define MINARGS 1

UBYTE *vers = "\0$VER: munglist 37.11";
UBYTE *Copyright = 
  "munglist v37.11\n(c) Copyright 1990-93 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage[] = {
"Usage: munglist [buffsize] [SHOWHUNK] [CHECK] [REMUNGE] [SHOWTIME] [-tickdelay]\n" ,
"                [MARK] [SINCE MARK] [TOTAL taskname]\n",
"Requires mungwall 37.69+.  SHOWTIME requires V37+ and mungwall 37.69+.\n",
"Output errors possible for memory freed and/or reallocated while listing\n",
"Good SHOWHUNK output requires SegTracker and still-loaded allocating program\n",
"CHECK/REMUNGE are serial output and turn off listing. tickdelay is for CHECK\n",
"TOTAL Taskname just totals up the amount currently allocated by that task\n",
"Default buffsize is 8000\n",
NULL };

struct Library *TimerBase;
struct timerequest TRequest = {0}, *treq = 0;


struct timerequest *opentimer(struct timerequest *treq);
void   closetimer(struct timerequest *treq);

void bye(UBYTE *s, int e);
void cleanup(void);


#define  DEFBUFSZ  (8000L)
#define  SBUFSZ    (80L)

#define 	SECS_PER_DAY		(60*60*24)
#define		SECS_PER_MINUTE 	(60)
#define		TICKS_PER_SEC		(50)
#define		MICROS_PER_TICK		(1000000/50)

/* for AWstartup */
/*
extern UBYTE  NeedWStartup;
UBYTE  *HaveWStartup = &NeedWStartup;
char AppWindow[] =
  "CON:0/0/400/200/ MungList Copyright Commodore-Amiga,Inc.";
*/

typedef char (* __asm SegTrack(register __a0 ULONG Address,
                               register __a1 ULONG *SegNum,
                               register __a2 ULONG *Offset));

struct  SegSem
        {
        struct  SignalSemaphore seg_Semaphore;
                SegTrack        *seg_Find;
        };

struct SegSem *segsem = NULL;

char prompt[] = {"\n q<RET>=quit, m<RET>=mark, [text]<RET>=rerun: "};


#define ASM __asm
#define REG(x) register __## x

ULONG TAG1      = 0xC0DE0000;
ULONG TAG2      = 0x0000F00D;

void kprintf(UBYTE *fmt,...);

extern VOID ASM InitMunging(REG(a1) UBYTE *, REG(d0) ULONG);
VOID ASM munglist(REG(a0) ULONG *ustart, REG(d0) ULONG usize);
VOID ASM checkmem(REG(a1) UBYTE *MemoryPtr);

VOID InfoDump(ULONG *stackp, ULONG acaller, ULONG ccaller, UBYTE *cname);
VOID PCDump(ULONG acaller, ULONG ccaller, UBYTE *cname);
VOID StackDump(ULONG *StackPtr);
VOID FindLayers(VOID);
UBYTE *hex(BYTE byte);

struct KeeperPort {
   struct MsgPort kp_MsgPort;
   char   kp_Name[24];
   ULONG  kp_KeeperKey;
   UWORD  kp_Ver;
   UWORD  kp_Rev;
   UBYTE  kp_FillChar;
   UBYTE  kp_Reserved1;
   UWORD  kp_Flags;
   ULONG  kp_InfoSize;
   ULONG  kp_PreSize;
   ULONG  kp_PostSize;
   ULONG  kp_MarkSecs;
   ULONG  kp_MarkMics;
   };


ULONG KeeperKey, LayersStart, LayersEnd;
struct KeeperPort *kp;
UBYTE *KeeperName = "Mungwall_Keeper";

BOOL ShowList=TRUE, ShowHunk = FALSE, Check=FALSE, ReMunge=FALSE;
BOOL ShowPC = FALSE, ShowTime=FALSE, Since=TRUE, Total=FALSE;
UBYTE *taskname = NULL;
ULONG tasktotal = 0L;
struct timeval tv_since, tv_now;


UBYTE *HunkOffs = "  %s:$%08lx in seglist \"%s\"   Hunk $%04lx Offset $%08lx\n";

/* Error report strings */
UBYTE *MemError = "\007\n  MUNGLIST: 0x%lx size %ld allocated by \"%s\" (task 0x%lx)\n  allocated from A:0x%lx C:0x%lx\n";
UBYTE *BeforeHit =
"\n%s%ld byte(s) before allocation at 0x%lx, size %ld were hit! (FillChar=$%02lx)\n";
UBYTE *AfterHit  =
"\n%s%ld byte(s) after  allocation at 0x%lx, size %ld were hit! (FillChar=$%02lx)\n";

UBYTE *Atleast = "At least ";
UBYTE *PCS= "%s: %08lx %08lx %08lx %08lx  %08lx %08lx %08lx %08lx\n";

UBYTE *BufOverflow = "\nMUNGLIST: *** MEM BUFFER OVERFLOW - Try bufsize option ***\n";
UBYTE *Checking    = "\nMUNGLIST: *** memory check ***\n";
UBYTE *ReMunging   = "\nMUNGLIST: *** memory remunge ***\n";
UBYTE *NoBuffer    = "\nMUNGLIST: can't alloc buffer\n";
UBYTE *Time	   = "  TIME: Alloc time: %s %s\n";

struct DateTime datetime = {0};
UBYTE  daybuf[LEN_DATSTRING];
UBYTE  datebuf[LEN_DATSTRING];
UBYTE  timebuf[LEN_DATSTRING];

void main(int argc,char **argv)
   {
   struct MemHeader *header, *mem, *mymems, *memtmp;
   struct MemChunk *chunk, *mychunks, *chunktmp;
   extern struct ExecBase *SysBase;
   extern struct Library *DOSBase;
   struct ExecBase *eb = SysBase;
   ULONG  *zero,size,fstart,fsize,ustart,usize,bufsz;
   LONG tickdelay=0, seconds=0;
   LONG star, ltmp, sec;
   char sbuf[SBUFSZ], *bufend, *memType, *wbuf = NULL;
   int k;
   BOOL BufOK;


   if((argc==2)&&(argv[1][0]=='?'))
	{
      	for(k=0; usage[k]; k++)	printf(usage[k]);
      	exit(0);
      	}


   if(!(kp=(struct KeeperPort *)FindPort(KeeperName)))
	{
	printf("This munglist requires Mungwall 37.61 or higher have been run\n");
	exit(0);
	}
   KeeperKey = kp->kp_KeeperKey;

   if(!(treq=opentimer(&TRequest)))
	{
	printf("Can't open timer device\n");
	exit(0);
	}
   GetSysTime(&tv_now);

   FindLayers();

   bufsz = DEFBUFSZ;

   datetime.dat_StrDay  = daybuf;
   datetime.dat_StrDate = datebuf;
   datetime.dat_StrTime = timebuf;
   datetime.dat_Format  = FORMAT_DOS;

   for(k=1; k<argc; k++)
	{
	if(!(stricmp(argv[k],"SHOWHUNK")))
	    {
	    segsem = (struct SegSem *)FindSemaphore("SegTracker");
	    if(segsem)	ShowHunk = TRUE;
	    else  printf("\nmunglist: Can't SHOWHUNK - SegTracker not running\n\n");
	    }
	else if(!(stricmp(argv[k],"SHOWTIME")))
	    {
	    if(DOSBase->lib_Version >= 37)	ShowTime = TRUE;
	    else 	printf("munglist: SHOWTIME option requires V37 or higher\n");
	    }
	else if(!(stricmp(argv[k],"CHECK")))
	    {
	    Check = TRUE;
	    }
	else if(!(stricmp(argv[k],"REMUNGE")))
	    {
	    ReMunge = TRUE;
	    }
	else if(!(stricmp(argv[k],"MARK")))
	    {
	    kp->kp_MarkSecs = tv_now.tv_secs;
	    kp->kp_MarkMics = tv_now.tv_micro;
	    if(argc == 2)	goto Done;
	    }
	else if(!(stricmp(argv[k],"SINCE")))
	    {
	    if(k<(argc-1))
		{
		Since=TRUE;
		k++;		/* only deal with MARK right now */
	        tv_since.tv_secs   = kp->kp_MarkSecs;
		tv_since.tv_micro  = kp->kp_MarkMics;
		}
	    }
	else if(!(stricmp(argv[k],"TOTAL")))
	    {
	    if(k<(argc-1))
		{
		Total=TRUE;
		ShowList=FALSE;
		k++;
		taskname = argv[k];
		}
	    }
	else
	   {
	   ltmp = atol(argv[k]);
	   if((ltmp)&&(ltmp < 0))
		{
		tickdelay=(-ltmp);
		seconds = tickdelay / 50;
		}
	   else if(ltmp)
		{
		bufsz = ltmp;
   	   	if(bufsz < 1000)		bufsz = DEFBUFSZ;
		}
	    }
	}

   if(Check||ReMunge)	ShowList=FALSE;

   if((ShowList)||(Check)||(Total))
	{
   	if(!(wbuf = (char *)AllocMem(bufsz,MEMF_PUBLIC|MEMF_CLEAR)))
	    {
	    if(ShowList)	printf(NoBuffer);
	    else		kprintf(NoBuffer);
	    goto Done;
	    }
   	bufend = wbuf + bufsz - 80;
	}



   Loop:

   if(SetSignal(0,0) & SIGBREAKF_CTRL_C)	goto Done;

   BufOK = TRUE;
   tasktotal = 0L;

   if((ReMunge)&&(!Check))	goto mungit;
   if(Check)	kprintf(Checking);

   Forbid();

   /* Copy the MemHeaders to my buffer, including tail node */
   mymems = (struct MemHeader *)wbuf;
   memtmp = mymems;
   for (mem = (struct MemHeader *)eb->MemList.lh_Head;
         mem;
           mem = (struct MemHeader *)mem->mh_Node.ln_Succ)
      {
      *memtmp++ = *mem;
      }

   /* Copy MemChunks right after, put ptr to each list in mymem's Name */
   mychunks = (struct MemChunk *)memtmp;
   chunktmp = mychunks;
   memtmp = mymems;
   for (mem = (struct MemHeader *)eb->MemList.lh_Head;
      mem->mh_Node.ln_Succ && BufOK ;
         mem = (struct MemHeader *)mem->mh_Node.ln_Succ)
      {
      /* Use Name to point to my list, copy chunk list there */
      if(mem->mh_First)
         {
         memtmp->mh_Node.ln_Name = (char *)chunktmp;
         for (chunk = mem->mh_First; chunk && BufOK ; chunk = chunk->mc_Next)
            {
            *chunktmp++ = *chunk;

            /* If in danger of overflowing our buffer */
            if((ULONG)chunktmp > (ULONG)bufend)
               {
               /* Cause copy and later printout to stop here */
	       chunktmp--;
               chunktmp->mc_Next = NULL;
               BufOK = FALSE;
               }
            }
         }
      else  memtmp->mh_Node.ln_Name = NULL;

      memtmp++;
      }

   if(ShowList)	Permit();


   /* Out of Forbid if ShowList, now can output the lists */
   for (mem = mymems; mem->mh_Node.ln_Succ; mem++)
      {
      if(mem->mh_Attributes & MEMF_FAST)  memType = "FAST";
      if(mem->mh_Attributes & MEMF_CHIP)  memType = "CHIP";

      /* show used ram at start if any */
      if((ULONG)mem->mh_First != (ULONG)mem->mh_Lower)
         {
	 ustart = (ULONG)mem->mh_Lower;
	 usize = (ULONG)mem->mh_First - (ULONG)mem->mh_Lower;
	 munglist((ULONG *)ustart,usize);
         }

      /* Then show each free chunk, and used areas between them.
       *  Remember - my copy of first chunk in mem->mh_Node.ln_Name
       *             and Next chunk is at chunk++
       *
       * First get address of real first chunk
       */

      fstart = (ULONG)mem->mh_First;
      for (chunk = (struct MemChunk *)mem->mh_Node.ln_Name; chunk;
              chunk = (fstart = (ULONG)chunk->mc_Next) ? chunk + 1 : NULL)
         {
	 if((ShowList)&&(SetSignal(0,0) & SIGBREAKF_CTRL_C))
		{
		printf("Break...\n");
		goto Done;
		}

         fsize  = (ULONG)chunk->mc_Bytes;

         if(chunk->mc_Next)
            {
            ustart = fstart + fsize;
            usize = ((ULONG)chunk->mc_Next) - (fstart + fsize);
	    munglist((ULONG *)ustart,usize);
            }
         /* Check for used chunk all the way at the top */
         else if((ustart=fstart+fsize) < (ULONG)mem->mh_Upper)
            {
            usize = (ULONG)mem->mh_Upper - ustart;
	    munglist((ULONG *)ustart,usize);
            }
         }

      if(!BufOK)
	{
	if(ShowList||Total)	printf(BufOverflow);
	else			kprintf(BufOverflow);
	break;
	}
      }

   if(!ShowList)	Permit();

   if(!BufOK)	goto Done;

mungit:
   if(ReMunge)
	{
	if(ShowList)	printf(ReMunging);
	else 		kprintf(ReMunging);

	Forbid();
	header = (struct MemHeader *) SysBase->MemList.lh_Head;

	while (header->mh_Node.ln_Succ)
	    {
	    for (chunk = header->mh_First; chunk; chunk = chunk->mc_Next)
		{
		size = chunk->mc_Bytes - 8;
		zero = (ULONG *) chunk;
		zero += 2;
		InitMunging((void *) zero, size);
		}
	    header = (struct MemHeader *) header->mh_Node.ln_Succ;
	    }
	Permit();
	}

   if(Total)	printf("Total memory use for task \"%s\": $%lx (%ld) bytes\n",
				taskname,tasktotal,tasktotal);

   if(!(ShowList||Total))
	{
	if(!tickdelay)	goto Done;
	if(seconds)
	    {
	    for(sec=0; sec<seconds; sec++)
		{
	    	if(SetSignal(0,0) & SIGBREAKF_CTRL_C)	break;
		else Delay(50);
		}
	    }
	else Delay(tickdelay);
	goto Loop;
	}

   if(!(IsInteractive(Input())))
      {
      printf("\nDone. (Rerun prompt requires stdin)\n");
      goto Done;
      }

DoPrompt:
   sbuf[0] = 'q';
   if(star = Open("*",MODE_OLDFILE))
	{
   	Write(star,prompt,strlen(prompt));

   	/* Get response in sbuf[] */
   	for(k=0; (Read(star,&sbuf[k],1)>0)&&(sbuf[k]!='\n')&&(k<79); k++);
   	k++;
   	sbuf[k] = NULL;
	Close(star);
	}

   if(((sbuf[0]|0x20) == 'm')&&(k==2))
	{
	GetSysTime(&tv_now);
	kp->kp_MarkSecs = tv_now.tv_secs;
	kp->kp_MarkMics = tv_now.tv_micro;
	tv_since.tv_secs  = tv_now.tv_secs;
	tv_since.tv_micro = tv_now.tv_micro;
	goto DoPrompt;
	}
   else if (((sbuf[0]|0x20) == 'q')&&(k==2))       goto Done;
   else if(!(stricmp(sbuf,"quit")))	      goto Done;
   else if(SetSignal(0,0) & SIGBREAKF_CTRL_C) goto Done;

   else printf("\n== NEXT ==> %s",&sbuf[0]);  /* rerun comment */
   goto Loop;

   Done:
   closetimer(treq);
   if(wbuf)  FreeMem(wbuf,bufsz);
   }

/* Not sure why, but mungwall breaks if INFOSIZE not divisible by 8 */
/* Size of area to store info like size, caller etc. */
#define INFOSIZE        56
/* Indexes to certain ULONGS in INFO */
#define UDATEI		10
#define UKEEPI		12
#define UFILLI		13
/*
 * A mungwall info area looks like this:
 *
 * C0DEF00D		4 bytes					offset=0
 * SIZE			4					offset=4
 * ACALLER		4					offset=8
 * CCALLER		4					offset=12
 * TASKADDRESS		4					offset=16
 * CHECKSUM		4					offset=20
 * TASK(COM)NAME       16	(optionally filled in)		offset=24
 * TIMESTAMP	        8					offset=40
 * KEEPERKEY		4	(per-session cookie) 		offset=48
 *
 * NOTE: Next 4 bytes treated as ulong[UFILLI] in places; keep em together
 * FILLCHAR		1	(for per-alloc fill)		offset=52
 * RESERVED		1					offset=53 
 * FLAGS		2					offset=54
 *
 * WALL			presize					offset=56
 * MEMORY		callersize
 * WALL			postsize
 *
 */

VOID ASM munglist(REG(a0) ULONG *ustart, REG(d0) ULONG usize)
    {
    struct timeval *tv;
    ULONG *uend, *up, seconds, micros;
    UWORD *wp;
    UBYTE *name, *bp, c;
    ULONG hunk, offs, acaller, ccaller;
    UBYTE *sname;

    uend = (ULONG *)(((ULONG)ustart) + usize - 4);
    for(up = ustart; up < uend; up +=1)
	{
	/* check for C0DEF00D in two pieces so loadsegged munglist
	 * program doesn't cause a bogus match
	 */
	wp = (UWORD *)up;
	bp = (UBYTE *)up;
	if((wp[0] == 0xC0DE)&&(wp[1]==0xF00D)&&(up[UKEEPI]==KeeperKey)&&(up[1] <= usize))
	    {
	    if(Since)
		{
		tv = (struct timeval *)&up[UDATEI];
		if(CmpTime(&tv_since,tv) < 0)
		    {
		    up += (up[1] >> 2);	/* advance ulong ptr by chunksize bytes */
		    continue;
		    }
		}

	    if(Total)
		{
	    	name = (UBYTE *)(&up[6]);
		if(!(stricmp(name,taskname)))	tasktotal += up[1];
		}
	    else if(ShowList)
		{
	    	name = (UBYTE *)(&up[6]);
	    	c = name[0];
	    	if((up[6]==0xBBBB)||(c<=0x2f)||((c>=0x80)&&(c<=0xbf))) name="";
	    	printf("$%08lx  size=$%02lx  A=%08lx C=%08lx  Task=$%08lx  %.16s\n",
		    bp+kp->kp_InfoSize+kp->kp_PreSize,up[1],up[2],up[3],up[4], name);
	    	if(segsem)
		    {
		    acaller = up[2];
		    ccaller = up[3];
    	            if(!(acaller&1))
		    	{
		    	if(sname=(*segsem->seg_Find)((ULONG)acaller,&hunk,&offs))
		    	    {
	    	    	    printf(HunkOffs,"A",acaller,sname,hunk,offs);
		    	    }
		    	}
	    	    if(!(ccaller&1))
		    	{
		    	if(sname=(*segsem->seg_Find)((ULONG)ccaller,&hunk,&offs))
		    	    {
	    	    	    printf(HunkOffs,"C",ccaller,sname,hunk,offs);
		    	    }
		    	}
		    }
	    	if((ShowTime)&&(up[UDATEI] != 0))
		    {
		    seconds = up[UDATEI];
		    micros  = up[UDATEI+1];
		    datetime.dat_Stamp.ds_Days   = seconds / SECS_PER_DAY;
		    datetime.dat_Stamp.ds_Minute = 
			(seconds % SECS_PER_DAY) / SECS_PER_MINUTE;
		    datetime.dat_Stamp.ds_Tick   = 
			((seconds % SECS_PER_MINUTE) * TICKS_PER_SEC) +
				(micros / MICROS_PER_TICK);
/*
		    printf("up=$%08lx up[UDATEI]=$%08lx &up[UDATEI]=$%08lx seconds=$%08lx micros=$%08lx\n",
			up, up[UDATEI], &up[UDATEI], seconds, micros);
*/
		    DateToStr(&datetime);
		    if(ShowList)
			printf(Time, datebuf, timebuf);
		    else
			kprintf(Time,datebuf, timebuf);
		    }
		}
	    else
		{
		checkmem(bp);
		}
	    up += (up[1] >> 2);	/* advance ulong ptr by chunksize bytes */
	    }		
	}
    }


/* passed start of a mungwall cookie */
VOID ASM checkmem(REG(a1) UBYTE *MemoryPtr)
    {
    struct Task *task;
    ULONG ACaller, CCaller, Size, *up;
    LONG errors;
    ULONG *FillMem;
    ULONG *zero;
    register ULONG i;
    UWORD LonelyHeart = 0;
    int j;
    UBYTE mfillchar, c, *name;

    up = (ULONG  *)MemoryPtr;
    Size = up[1];
    ACaller = up[2];

    /* make MemoryPtr as if from a deallocation */
    MemoryPtr = MemoryPtr + kp->kp_PreSize + kp->kp_InfoSize;
    
    if ((ACaller < LayersStart) || (ACaller > LayersEnd))
	{
	/* Check if cookie is OK */
	zero = FillMem = (ULONG *) (MemoryPtr - kp->kp_PreSize - kp->kp_InfoSize);
	FillMem++;

	mfillchar = *((UBYTE *)&zero[UFILLI]);

	/* if cookie and per-session cookie both match */
	if ((*zero == (TAG1|TAG2))&&(zero[UKEEPI]==KeeperKey))
	    {
	    /* Generate checksum */
	    errors = *zero++ + *zero++ + *zero++ + *zero++ + *zero++;
	    if (*zero != errors)
		{
		kprintf("munglist: Mungwall cookie - checksum failure\n");
		LonelyHeart++;
		}
	    else
		{
		Size = *FillMem++;
		Size += (kp->kp_PreSize + kp->kp_PostSize + kp->kp_InfoSize);
		MemoryPtr -= kp->kp_PreSize;

		errors = 0;
		i = 0;
		for (; i < kp->kp_PreSize; i++)
				if (MemoryPtr[i] != mfillchar)
							errors++;

		if (errors)
		    {
		    j = 0;

		    kprintf(BeforeHit, (errors == kp->kp_PreSize) ? "At least " : "", errors, MemoryPtr + kp->kp_PreSize, Size - (kp->kp_PreSize + kp->kp_PostSize + kp->kp_InfoSize), mfillchar);
		    /* Output damage area */
		    kprintf("$%08lx:",&MemoryPtr[4]);
		    for (i = 4; i < kp->kp_PreSize; i++)
			{
			if (i % 4 == 0)
			    {
			    kprintf(" ");
			    if (++j == 8) kprintf("\n    ");
			    }
		    	kprintf("%s%s", hex((UBYTE) (MemoryPtr[i] >> 4)), hex(MemoryPtr[i]));
		    	}
		    kprintf("\n");
		    /* Indicate there were errors here */
		    LonelyHeart++;
		    }

	    	/* Check BABECOED */
	    	errors = 0;
	    	for (i = (Size - kp->kp_PostSize - kp->kp_InfoSize);
			i < ((Size - kp->kp_InfoSize + 7) & ~0x07); i++)
					if (MemoryPtr[i] != mfillchar) errors++;
	    	if (errors)
		    {
		    int j = 0;

		    kprintf(AfterHit, (errors >= kp->kp_PostSize) ? "At least " : "", errors, MemoryPtr + kp->kp_PreSize, Size - (kp->kp_PreSize + kp->kp_PostSize + kp->kp_InfoSize), mfillchar);

		    /* Output damage area */
		    i = (Size - kp->kp_PostSize - kp->kp_InfoSize) & ~0x03;
		    kprintf("$%08lx:",&MemoryPtr[i]);
		    for (; i < ((Size - kp->kp_InfoSize + 7) & ~0x07); i++)
		    	{
		    	if (i % 4 == 0)
			    {
			    kprintf(" ");
			    if (++j == 8) kprintf("\n    ");
			    }
		    	kprintf("%s%s", hex((UBYTE) (MemoryPtr[i] >> 4)), hex(MemoryPtr[i]));
		        }
		    kprintf("\n");
		    /* Indicate there were errors */
		    LonelyHeart++;
		    }

	    	if (LonelyHeart)
		    {
		    CCaller = up[3];
		    task = (struct Task *)up[4];
	    	    name = (UBYTE *)(&up[6]);
	    	    c = name[0];
	    	    if((up[6]==0xBBBB)||(c<=0x2f)||((c>=0x80)&&(c<=0xbf))) name="";	

		    kprintf(MemError,
			MemoryPtr + kp->kp_PreSize,
			Size - (kp->kp_PreSize + kp->kp_PostSize + kp->kp_InfoSize),
			name, task,
			ACaller, CCaller);

		    if(ShowPC||ShowHunk) PCDump(ACaller, CCaller, name);
		    }
		}
    	    }
	}
    }


VOID PCDump(ULONG acaller, ULONG ccaller, UBYTE *cname)
    {
    ULONG *up, hunk, offs, typemem;
    UBYTE *name;


    typemem = TypeOfMem((APTR)acaller);
    if(ShowHunk)
	{
	if(segsem)
	    {
	    if(name=(*segsem->seg_Find)((ULONG)acaller,&hunk,&offs))
		{
	    	kprintf(HunkOffs,"A",acaller,name,hunk,offs);
		}
	    }
	}
    if((ShowPC)&&(typemem)&&(!(acaller & 1)))  /* don't show pc if odd */
	{
	up = (ULONG *)acaller;	
	kprintf(PCS,"APC",up[0],up[1],up[2],up[3],up[4],up[5],up[6],up[7]);
	kprintf(PCS,"APC",up[8],up[9],up[10],up[11],up[12],up[13],up[14],up[15]);
	}


    typemem = TypeOfMem((APTR)ccaller);
    if(ShowHunk)
	{
	if(segsem)
	    {
	    if(name=(*segsem->seg_Find)((ULONG)ccaller,&hunk,&offs))
		{
	    	kprintf(HunkOffs,"C",ccaller,name,hunk,offs);
		}
	    }
	}
    if((ShowPC)&&(typemem)&&(!(ccaller & 1))) /* don't show pc if odd */
	{
	up = (ULONG *)ccaller;	
	kprintf(PCS,"CPC",up[0],up[1],up[2],up[3],up[4],up[5],up[6],up[7]);
	kprintf(PCS,"CPC",up[8],up[9],up[10],up[11],up[12],up[13],up[14],up[15]);
	}
    }


VOID FindLayers(VOID)
{
	struct Resident *layerstag;
	ULONG *resmodules;
	LONG i, j, k;

	if (layerstag = FindResident("layers.library")) {
		LayersStart = LayersEnd = (ULONG) layerstag;
		resmodules = (ULONG *) SysBase->ResModules;
		/* It's unlikely layers will end up being 256 K */
		LayersEnd += 0x40000;
		j = 1000;
		k = 0;
		/* Bubble up LayersEnd */
		for (i = 0; i <= j; i++) {
			while (*resmodules != NULL) {
				if (*resmodules > LayersStart && *resmodules < LayersEnd)
					LayersEnd = *resmodules;
				*resmodules++;
				k++;
			}
			j = k;
			k = 0;
			resmodules = (ULONG *) SysBase->ResModules;
		}
	}
}

char *nums[] =
{
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"
};

UBYTE *hex(BYTE byte)
{
        return nums[byte & 0x0f];
}

struct timerequest *opentimer(struct timerequest *treq)
    {
    treq->tr_node.io_Message.mn_Length = sizeof(struct timerequest);
    treq->tr_node.io_Message.mn_Node.ln_Type = NT_UNKNOWN;
    if(OpenDevice("timer.device",UNIT_MICROHZ,treq,0))	return(NULL);

    TimerBase = (struct Library *)treq->tr_node.io_Device;
    return(treq);	
    }

void closetimer(struct timerequest *treq)
    {
    if(treq)	CloseDevice(treq);
    }
