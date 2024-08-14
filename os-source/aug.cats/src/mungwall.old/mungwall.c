/*
**      Mungwall. Munges and traces memory trashing.
**
**      (C) Copyright 1990-1993 Commodore-Amiga, Inc.
**          All Rights Reserved
**
** define PARALLEL to create Mungwall.par
**
** 37.46 - speed up getting caller name
** 37.47 - add MEGASTACK option (later changed to SHOWSTACK)
** 37.48 - add NAMETAG option for MungList, INFOSIZE now 16 larger
** 37.49 - Always munge out C0DEF00D starter to C0DEFEED, even if Forbid()en
** 37.50 - Regs preserved in asm GetCallerName, remove strncpy, 68000 compat
** 37.51 - Add wedge into AvailMem to reduce available memory reported
** 37.52 - Fix GetCallerName logic (caused Enforcer hits on per-Task opts)
**	   Pass SysBase (a6) to old_AvailMem, old_AllocMem, old_FreeMem
** 37.53 - Fix 68000 compatibility is mstartup.asm (.l's taskname check)
** 37.54 - Move Write()'s out of initial Forbid (Have always been there)
** 37.55 - Fix old bug with "zero" pointer
** 37.56 - now copies tailpath'd command/task name, null terminates in buffer
** 37.57 - change output slightly ("at" to "task"), add SHOWFAIL option
** 37.58 - add "NO" and UPDATE versions of recent options
** 37.59 - add ShowPC option
** 37.60 - fix SHOWPC option to use TypeOfMem
** 37.61 - add allocated-once Keeper port and secondary per-session cookie
**		INFOSIZE now 8 larger
** 37.62 - added rolling fill default (may be disabled via FILLCHAR),
**		changed MEGASTACK keyword to SHOWSTACK (both accepted)
** 37.63 - add SHOWHUNK and NOSHOWHUNK
** 37.64 - add SegTracker support
** 37.65 - Fix output (was showing 0xnnn on one decimal number)
** 37.66 - Add optional Timestamp to allocations
** 37.68 - Add time output to hits if timestamping
** 37.69 - let SegTracker screen TypeOfMem, add time mark to KeeperPort
*/

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/resident.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <devices/timer.h>

#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <ctype.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/timer_protos.h>

#include "mungwall_rev.h"

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/timer_pragmas.h>

#define ASM __asm
#define REG(x) register __## x

void sprintf(UBYTE *buf, UBYTE *fmt,... );

/*
#define PARALLEL
*/


#ifdef PARALLEL
#define printf dprintf
extern VOID dprintf(STRPTR,...);
#else
#define printf kprintf
extern VOID kprintf(STRPTR,...);
#endif

/* Using a DEBUG'ed version in a F00000 board is a bad idea. It takes +/- 15 minutes to boot. */

#ifdef DEBUG
#define D(x) x
#else
#define D(x) ;
#endif
#ifdef DEBUG1
#define D1(x) x
#else
#define D1(x) ;
#endif

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

/* Assembler stub returns a pointer to the caller's stack */
#define ACALLER 0		/* StackPtr[0] = ACaller, StackPtr[1] = saved A6 in stub */
#define CCALLER 2		/* StackPtr[2] = CCaller */

/*** VERSION ***************************************************************************************/
extern ULONG VerTitle;

/*** GLOBALS INDICATING USER PREFERENCES ***********************************************************/
extern ULONG PreSize;		/* Number of bytes before Caller memory pointer */
extern ULONG PostSize;		/* Nominal number of bytes after caller memory pointer + caller size.
	                         * Actual postsize will vary due to size rounding to nearest chunk.
	                         */

extern STRPTR Taskname;		/* Process name to monitor, or refuse */
extern BOOL Except;		/* All but... */
extern BOOL Snoop;		/* Output snoop info */
extern BOOL ShowStack;		/* Output StackDump info */
extern BOOL NameTag;		/* Store task or command name */
extern BOOL ErrorWait;		/* Halt caller in case of error */
extern BOOL ShowFail;		/* Show any failed allocations */
extern BOOL ShowPC;		/* Show PC dumps on hit */
extern BOOL ShowHunk;		/* Show Hunk and Offset, if any, on hit */
extern BOOL TimeStamp;		/* Stamp each allocation with secs, micros */
extern BOOL RollFill;		/* Roll the fill character (odds from $81-FF) */
extern UBYTE FillChar;		/* Character to use to fill pre- and postmemory */

extern struct Library *TimerBase;

/*** GLOBALS USED TO CHECK FOR LAYERS AND SHOW CALLER **********************************************/
extern ULONG LayersStart;
extern ULONG LayersEnd;
extern BYTE ConfigInfo;
extern struct MsgPort *MungPort;

struct KeeperPort {
   struct MsgPort kp_MsgPort;
   char   kp_Name[24];
   ULONG  kp_KeeperKey;
   UWORD  kp_Ver;
   UWORD  kp_Rev;
   UBYTE  kp_FillChar;	/* fillchar as initialized only */
   UBYTE  kp_Reserved1;
   UWORD  kp_Flags;
   ULONG  kp_InfoSize;
   ULONG  kp_PreSize;
   ULONG  kp_PostSize;
   ULONG  kp_MarkSecs;
   ULONG  kp_MarkMics;
   };

struct KeeperPort *kp;
struct DateStamp ds;
extern ULONG KeeperKey;


typedef char (* __asm SegTrack(register __a0 ULONG Address,
                               register __a1 ULONG *SegNum,
                               register __a2 ULONG *Offset));

struct  SegSem
        {
        struct  SignalSemaphore seg_Semaphore;
                SegTrack        *seg_Find;
        };

ULONG *ProcNames[10];

/*** COMMAND CODES FOR UPDATING GLOBALS ************************************************************/
#define M_TASK      0x01
#define M_SNOOP     0x02
#define M_WAIT      0x04
#define M_EXCEPT    0x08

#define M_NOTASK    0x10
#define M_NOSNOOP   0x20
#define M_NOWAIT    0x40
#define M_NOEXCEPT  0x80

#define M_SHOWSTACK   0x0100
#define M_NAMETAG     0x0200
#define M_SHOWFAIL    0x0400
#define M_SHOWPC      0x0800

#define M_NOSHOWSTACK 0x1000
#define M_NONAMETAG   0x2000
#define M_NOSHOWFAIL  0x4000
#define M_NOSHOWPC    0x8000

#define M_SHOWHUNK    0x010000
#define M_TIMESTAMP   0x020000

#define M_NOSHOWHUNK  0x100000
#define M_NOTIMESTAMP 0x200000

/* Message to update an existing mungwall task */
struct MungMessage {
	struct Message mm_Message;
	ULONG mm_UpdateFlags;
	UBYTE *mm_Taskname;
	UBYTE mm_ConfigInfo;
};

/*** PROTOTYPES ************************************************************************************/
extern VOID *(*ASM old_AllocMem) (REG(d0) ULONG,  REG(d1) ULONG, REG(a6) struct ExecBase *);
extern VOID  (*ASM old_FreeMem)  (REG(a1) VOID *, REG(d0) ULONG, REG(a6) struct ExecBase *);
extern ULONG (*ASM old_AvailMem) (REG(d1) ULONG, REG(a6) struct ExecBase *);
extern VOID ASM InitMunging(REG(a1) UBYTE *, REG(d0) ULONG);
extern VOID ASM PreMunging(REG(a1) UBYTE *, REG(d0) ULONG);
extern VOID ASM PostMunging(REG(a1) UBYTE *, REG(d0) ULONG);

extern BOOL ASM InThisSegList(REG(a0) BPTR seglist, REG(a1) VOID *address, REG(a2) ULONG *hunkp, REG(a3) ULONG *offsp, REG(a6) struct Library *SysBase);
extern BOOL ASM InProcSegList(REG(a0) struct Task *task, REG(a1) VOID *address, REG(a2) ULONG *hunkp, REG(a3) ULONG *offsp, REG(a6) struct Library *SysBase);

extern UBYTE * ASM GetCallerName( REG(a0) UBYTE *buffer);
UBYTE * ASM GetCName( REG(a0) UBYTE *cname);

VOID *ASM new_AllocMem(REG(d0) ULONG, REG(d1) ULONG, REG(a2) ULONG *);
VOID  ASM new_FreeMem(REG(a1) UBYTE *, REG(d0) ULONG, REG(a2) ULONG *);
ULONG ASM new_AvailMem(REG(d1) ULONG, REG(a2) ULONG *);

LONG startup(STRPTR args);
VOID FindLayers(VOID);
VOID HandleSignal(VOID);

VOID InfoDump(ULONG *stackp, ULONG acaller, ULONG ccaller,
		UBYTE *cname,struct timeval *tv);

VOID StackDump(ULONG *stackp);
VOID PCDump(ULONG acaller, ULONG ccaller, UBYTE *cname);

VOID datestringout(struct timeval *tv);
VOID datestring_tv(struct timeval *tv,UBYTE *daybuf,UBYTE *datebuf,UBYTE *timebuf);
VOID datestring_ds(struct DateStamp *ds, UBYTE *daybuf,UBYTE *datebuf,UBYTE *timebuf);

void main(void);
ULONG ParseLine(STRPTR);
ULONG MakePort(VOID);
struct MsgPort *AllocPort(UBYTE *, LONG);
VOID FreePort(struct MsgPort * mp);
UBYTE *StrCpy(register UBYTE *, register UBYTE *);
void NewList(struct List * list);

/* Randell's functions (slightly changed though) */
UBYTE *hex(BYTE byte);
LONG maxword(UBYTE * string);
UBYTE *Getarg(UBYTE * dest, register UBYTE * string, register LONG number, LONG * maxchars, register LONG last);
UBYTE *cpymax(register UBYTE * dest, register UBYTE * src, register LONG * maxchars);
extern void grab_em(void);
extern int free_em(void);

/*** STRINGS **************************************************************************************/
UBYTE *Copyright = "(c) Copyright 1990-1993 Commodore-Amiga, Inc.\n";
UBYTE *Portname = "Mungwall";
UBYTE *KeeperName = "Mungwall_Keeper";

/* Snoop compatible output strings */
UBYTE *AMSnoopFmt = "$%08lx=AllocMem(%6ld,$%08lx) A:%08lx C:%08lx   \"%s\"\n";
UBYTE *FMSnoopFmt = "$%08lx= FreeMem($%08lx,%6ld) A:%08lx C:%08lx   \"%s\"\n";

/* Exit strings */
UBYTE *CBreak = "CTRL-C or BREAK to exit...\n";
UBYTE *NoExit = "Can't exit now. Unable to undo SetFunction()'s\n";
UBYTE *Gone = "Mungwall removed\n";

/* Waiting/running strings if ErrorWait == TRUE */
UBYTE *Halting = "Halting task \"%s\" - waiting for CTRL-C...\n";
UBYTE *Running = "Running task \"%s\".\n";

UBYTE *HunkOffs = "%s:0x%08lx in seglist of \"%s\"   Hunk %04lx Offset %08lx\n";

/* Error report strings */
UBYTE *AllocError = "\007\n%sAllocMem(%ld,0x%lx) attempted by \"%s\" (TCB:%08lx)\n  from A:0x%lx C:0x%lx SP:0x%lx\n";
UBYTE *FreeError = "\007\n%sFreeMem(0x%lx,%ld) attempted by \"%s\" (TCB:%08lx)\n  from A:0x%lx C:0x%lx SP:0x%lx\n";
UBYTE *BeforeHit =
"\n%s%ld byte(s) before allocation at 0x%lx, size %ld were hit! (FillChar=$%02lx)\n";
UBYTE *AfterHit  =
"\n%s%ld byte(s) after  allocation at 0x%lx, size %ld were hit! (FillChar=$%02lx)\n";

UBYTE *Time1 = "TIME: Alloc Time: %s %s";
UBYTE *Time2 = "   Current Time: %s %s\n";

UBYTE *Settings =
"\nMungwall: Task: %s\nPreSize: %ld, PostSize: %ld, FillChar: 0x%lx, Snoop: %s Wait: %s\n";
UBYTE *On = "ON";
UBYTE *Off = "OFF";
UBYTE *Atleast = "At least ";
UBYTE *ShowStackS = "@ SP: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n";
UBYTE *PCS= "%s: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n";

/* Error strings */
UBYTE *Usage = 
"Usage: Mungwall [UPDATE] [TASK name] [(NO)WAIT] [(NO)SHOWFAIL]\n"
"  [(NO)SNOOP] [INFO] [(NO)SHOWSTACK] [(NO)SHOWPC] [(NO)SHOWHUNK]\n"
"  [(NO)NAMETAG] [(NO)TIMESTAMP] [PRESIZE n] [POSTSIZE n] [FILLCHAR 0xHH]\n";
UBYTE *NoMem = "Out of memory\n";
UBYTE *NoPort = "Can't create port\n";

/* stored in 2 pieces so munglist won't pick this up */
ULONG TAG1	= 0xC0DE0000;
ULONG TAG2	= 0x0000F00D;

extern struct ExecBase *SysBase;
struct Library *DOSBase = NULL;


LONG startup(UBYTE * args)
{
	int argc = maxword(args);
	ULONG *zero;
	struct MsgPort *activeport;
	UBYTE *word, *Namestore, *Nameline;
	BOOL ABORT = FALSE, UPDATE = FALSE;
	ULONG UpdateFlags = 0L;
	struct MemChunk *chunk;
	struct MemHeader *header;
	LONG i, j, size;

	D(printf("startup:\n"));

	if (word = AllocMem(1536, MEMF_CLEAR)) {
		Namestore = word + 512;
		Nameline = Namestore + 512;

		if (DOSBase = OpenLibrary("dos.library", 0)) {

			/* Initialize prefs */
			Taskname = NULL;

			for (i = 0; i < argc; i++) {
				size = 79;
				Getarg(word, args, i, &size, i);

				/* If argument */
				if (stricmp(word, "TASK") == 0) {
					i++;
					size = 79;
					Getarg(word, args, i, &size, i);

					/* Remove quotes if there */
					if (word[0] == '"') {
						for (j = 0; j < strlen(word); j++)
							word[j] = word[j + 1];
						word[strlen(word) - 1] = '\0';
					}
					StrCpy(Nameline, word);
					StrCpy(Namestore, word);
					if (!UPDATE)
						UpdateFlags |= (ParseLine(Namestore));
					UpdateFlags |= M_TASK;
					if (stricmp(Namestore, "ALL") == 0)
						Taskname = NULL;
					else
						Taskname = Namestore;
				} else if (stricmp(word, "PRESIZE") == 0) {
					i++;
					size = 79;
					Getarg(word, args, i, &size, i);

					PreSize = atol(word);
					if (PreSize < 4)
						PreSize = 4;
					if (PreSize > 64)
						PreSize = 64;
					PreSize = PreSize & ~0x03;
				} else if (stricmp(word, "POSTSIZE") == 0) {
					i++;
					size = 79;
					Getarg(word, args, i, &size, i);

					PostSize = atol(word);
					if (PostSize < 4)
						PostSize = 4;
					if (PostSize > 64)
						PostSize = 64;
				} else if (stricmp(word, "FILLCHAR") == 0) {
					i++;
					size = 79;
					Getarg(word, args, i, &size, i);
					if (word[0] == '0' && toupper(word[1]) == ('X')) {
						if (stch_i(&word[2], &j) > 3)
							ABORT = TRUE;
						else
							FillChar = j;
					} else
						FillChar = atol(word);
					if (ABORT)
						Write(Output(), "FILLCHAR: Invalid value.\n", 25);
					RollFill = FALSE;
				} else if (stricmp(word, "SNOOP") == 0) {
					Snoop = TRUE;
					UpdateFlags |= M_SNOOP;
				} else if (stricmp(word, "WAIT") == 0) {
					ErrorWait = TRUE;
					UpdateFlags |= M_WAIT;
				} else if (stricmp(word, "UPDATE") == 0) {
					if (i == 0)
						UPDATE = TRUE;
					ABORT = TRUE;
				} else if (stricmp(word, "NOSNOOP") == 0) {
					Snoop = FALSE;
					UpdateFlags |= M_NOSNOOP;
				} else if (stricmp(word, "NOWAIT") == 0) {
					ErrorWait = FALSE;
					UpdateFlags |= M_NOWAIT;
				} else if (stricmp(word, "INFO") == 0) {
					ConfigInfo = TRUE;
				} else if ((stricmp(word, "SHOWSTACK")==0)||
						(stricmp(word,"MEGASTACK")==0)) {
					ShowStack = TRUE;
					UpdateFlags |= M_SHOWSTACK;
				} else if ((stricmp(word, "NOSHOWSTACK")==0)||
						(stricmp(word,"NOMEGASTACK")==0)) {
					ShowStack = FALSE;
					UpdateFlags |= M_NOSHOWSTACK;
				} else if (stricmp(word, "NAMETAG") == 0) {
					NameTag = TRUE;
					UpdateFlags |= M_NAMETAG;
				} else if (stricmp(word, "NONAMETAG") == 0) {
					NameTag = FALSE;
					UpdateFlags |= M_NONAMETAG;
				} else if (stricmp(word, "SHOWFAIL") == 0) {
					ShowFail = TRUE;
					UpdateFlags |= M_SHOWFAIL;
				} else if (stricmp(word, "NOSHOWFAIL") == 0) {
					ShowFail = FALSE;
					UpdateFlags |= M_NOSHOWFAIL;
				} else if (stricmp(word, "SHOWPC") == 0) {
					ShowPC = TRUE;
					UpdateFlags |= M_SHOWPC;
				} else if (stricmp(word, "NOSHOWPC") == 0) {
					ShowPC = FALSE;
					UpdateFlags |= M_NOSHOWPC;
				} else if (stricmp(word, "SHOWHUNK") == 0) {
					ShowHunk = TRUE;
					UpdateFlags |= M_SHOWHUNK;
				} else if (stricmp(word, "NOSHOWHUNK") == 0) {
					ShowHunk = FALSE;
					UpdateFlags |= M_NOSHOWHUNK;
				} else if (stricmp(word, "TIMESTAMP") == 0) {
					TimeStamp = TRUE;
					UpdateFlags |= M_TIMESTAMP;
				} else if (stricmp(word, "NOTIMESTAMP") == 0) {
					TimeStamp = FALSE;
					UpdateFlags |= M_NOTIMESTAMP;
				} else if (stricmp(word,"?") == 0) {
					Write(Output(), Usage, strlen(Usage));
					ABORT = TRUE;
					UPDATE = FALSE;
				} else {
					printf("Mungwall: unknown option \"%s\"\n",word);
				}
			}
			/* Just to be sure! */
			if (!ABORT)
				UPDATE = FALSE;

			if (!ABORT) {
				/* Tell 'em we're here */
				Write(Output(), (STRPTR) VerTitle, strlen((STRPTR) VerTitle));
				Write(Output(), Copyright, strlen(Copyright));
				/* 13/11/90/EC Mung FreeMem list. Means loadf'ed mungwall's won't
				 * do this. Fix?
				 */

				/* added for 37.61 - this port created only once */
				Forbid();
				if (!(kp = (struct KeeperPort *)FindPort(KeeperName))) {
 					if(!(kp =(struct KeeperPort *)AllocMem(sizeof(struct KeeperPort),MEMF_PUBLIC|MEMF_CLEAR))) {
						Permit();
						Write(Output(),NoMem,strlen(NoMem));
						return 0;
					}
 					StrCpy(kp->kp_Name,KeeperName);
      					kp->kp_MsgPort.mp_Node.ln_Name = kp->kp_Name;
      					kp->kp_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
					DateStamp(&ds);
					kp->kp_KeeperKey = (ds.ds_Days  << 16) +
							   (ds.ds_Minute << 8) +
							   (ds.ds_Tick);
					kp->kp_Ver = VERSION;
					kp->kp_Rev = REVISION;
					kp->kp_InfoSize = INFOSIZE;
					kp->kp_PreSize  = PreSize;
					kp->kp_PostSize = PostSize;
					kp->kp_FillChar = FillChar;
      					AddPort((struct MsgPort *)kp);
				}
				KeeperKey = kp->kp_KeeperKey;
				Permit();

				Write(Output(), "About to munge free memory...\n", 30);

				Forbid();
				if (!(FindPort(Portname))) {
					/* Confirm settings */
					if (ConfigInfo)
						printf(Settings, (Taskname) ? Nameline : "ALL", PreSize, PostSize, FillChar,
						       (Snoop) ? On : Off, (ErrorWait) ? On : Off);

					/* Setfunction stuff, also finds layers start & end */
					grab_em();

					/* Write location Zero, if enforcer is not running */
					if ((!(FindPort("_The Enforcer_")))&&(!(FindTask("« Enforcer »")))) {
						zero = NULL;
						*zero = (ULONG) 0xC0DEDBAD;
					}


					header = (struct MemHeader *) SysBase->MemList.lh_Head;

					while (header->mh_Node.ln_Succ) {
						for (chunk = header->mh_First; chunk; chunk = chunk->mc_Next) {
							size = chunk->mc_Bytes - 8;
							zero = (ULONG *) chunk;
							zero += 2;
							InitMunging((void *) zero, size);
						}
						header = (struct MemHeader *) header->mh_Node.ln_Succ;
					}
					Permit();
					Write(Output(), CBreak, strlen(CBreak));

					HandleSignal();

					/* Set zero to zero again */
					if ((!(FindPort("_The Enforcer_")))&&(!(FindTask("« Enforcer »")))) {
						zero = NULL;
						*zero = 0L;
					}
				} else {
					Permit();
					Write(Output(), "Mungwall already running\n", 25);
				}
			} else if (UPDATE == TRUE) {
				struct MsgPort *replyport;

				if (replyport = AllocPort(NULL, 0)) {
					struct MungMessage *mmsg;

					if (mmsg = (struct MungMessage *) AllocMem(sizeof(struct MungMessage), MEMF_CLEAR)) {
						mmsg->mm_Message.mn_Node.ln_Type = NT_UNKNOWN;
						mmsg->mm_Message.mn_Length = sizeof(struct MungMessage);
						mmsg->mm_Message.mn_ReplyPort = replyport;
						if (ConfigInfo)
							mmsg->mm_ConfigInfo = TRUE;
						else
							mmsg->mm_ConfigInfo = FALSE;

						mmsg->mm_UpdateFlags = UpdateFlags;
						if (Taskname)
							mmsg->mm_Taskname = Nameline;
						else
							mmsg->mm_Taskname = NULL;

						/* Make sure mungwall is running */
						Forbid();
						if (activeport = FindPort(Portname)) {
							PutMsg(activeport, (struct Message *) mmsg);
							WaitPort(replyport);
						} else
							Write(Output(), "Can't find Mungwall port\n", 25);
						Permit();
						FreeMem(mmsg, sizeof(struct MungMessage));
					} else
						Write(Output(), NoMem, strlen(NoMem));
					FreePort(replyport);
				} else
					Write(Output(), NoPort, strlen(NoPort));
			}
			CloseLibrary(DOSBase);
		}
		FreeMem(word, 1536);
	}
	return 0;
}


ULONG ASM new_AvailMem(REG(d1) ULONG Flags, REG(a2) ULONG * StackPtr)
{
	ULONG size, mungsize;

	mungsize = INFOSIZE + PreSize + PostSize;
	size = (*old_AvailMem) (Flags, SysBase);
	if(size > mungsize)	size -= mungsize;
	return(size);
}


VOID *ASM new_AllocMem(REG(d0) ULONG Size, REG(d1) ULONG Flags, REG(a2) ULONG * StackPtr)
{
	ULONG ACaller = StackPtr[ACALLER];
	ULONG CCaller = StackPtr[CCALLER];
	UBYTE cname[32];
	STRPTR TmpPtr1;
	UBYTE *MemoryPtr = NULL, *s, *m;
	ULONG *FillMem;
	ULONG i;
	UWORD LonelyHeart = 0;
	BOOL TaskOK = TRUE;
	UBYTE mfillchar;

	cname[0] = 0;
	if(RollFill)
	    {
	    FillChar = (FillChar + 2) | 0x81;
	    }
	mfillchar = FillChar;

	D(printf("AllocMem\nParams: Size:%ld Flags:0x%lx\n", Size, Flags));
	D(printf("Caller: %s from A:%lx C:%lx Stack:%lx\n", GetCName(cname), ACaller, CCaller, StackPtr));

	if (Size == 0)
	    {
	    printf(AllocError, "", Size, Flags, GetCName(cname),
		SysBase->ThisTask, ACaller, CCaller, StackPtr);

	    InfoDump(StackPtr, ACaller, CCaller, GetCName(cname), NULL);

	    if (ErrorWait)
		{
		printf(Halting, cname);
		Wait(SIGBREAKF_CTRL_C);
		printf(Running, cname);
		}
	    /* Don't bother me anymore */
	    return(NULL);
    	    }

	/* Check for specific task */
	if (Taskname && (GetCName(cname)))
	    {
	    i = 0;
	    TaskOK = FALSE;
	    TmpPtr1 = (UBYTE *) ProcNames[i];
	    while (TmpPtr1 != NULL && TaskOK == FALSE)
		{
		if (strcmp(cname, TmpPtr1) == 0)  TaskOK = TRUE;
		TmpPtr1 = (UBYTE *) ProcNames[++i];
		}
	    if (Except) TaskOK ^= TRUE;
	    }

	/* Requested size is big. Too big to add wall */
	if (Size & 0xC0000000)
	    {
	    TaskOK = FALSE;
	    D1(printf("Requested size too large. Not touched\n"));
    	    }
	if (TaskOK)
	    {
	    if (ACaller < LayersStart || ACaller > LayersEnd)
		{
		Size += (PreSize + PostSize + INFOSIZE);
		MemoryPtr = (*old_AllocMem) (Size, Flags, SysBase);

		D(printf("Allocated Memory at $%lx Size %ld Flags 0x%lx\n", MemoryPtr, Size, Flags));
		if (MemoryPtr)
		    {
		    /* $DEADF00D Pre-Munging */
		    if (!(Flags & MEMF_CLEAR))
			{
			D1(printf("PreMunging memory...\n"));
			PreMunging(MemoryPtr, Size);
               		}

		    /* Fill INFO area */
		    FillMem = (ULONG *) MemoryPtr;
		    i = (ULONG) (Size - (PreSize + PostSize + INFOSIZE));
		    if(TimeStamp&&TimerBase)
			{
 			GetSysTime((struct timeval *)&FillMem[UDATEI]);
			}
		    else
			{
			FillMem[UDATEI]   = 0;
			FillMem[UDATEI+1] = 0;
			}
		    FillMem[UKEEPI] = KeeperKey;
		    FillMem[UFILLI] = 0L;	/* FillChar, Reserved, Flags */
		    *((UBYTE *)&FillMem[UFILLI]) = mfillchar;
		    *FillMem++ = (TAG1 | TAG2);
		    *FillMem++ = i;
		    *FillMem++ = (ULONG) ACaller;
		    *FillMem++ = (ULONG) CCaller;
		    *FillMem++ = (ULONG) SysBase->ThisTask;
		    *FillMem++ = (ULONG) (TAG1|TAG2) + i + 
			(ULONG) ACaller + (ULONG) CCaller +
			(ULONG) SysBase->ThisTask;
		    if(NameTag)
			{
			s = GetCName(cname);
			m = (UBYTE *)FillMem;
			for(i=0; i<15; i++)
			    {
			    m[i] = s[i];
			    }
			m[15] = 0;
			}
		    else *FillMem = 0xBBBBBBBB;
		    		
		    MemoryPtr += INFOSIZE;
		    Size -= INFOSIZE;

		    /* Fill pad area */
		    for (i = 0; i < PreSize; i++)
				MemoryPtr[i] = mfillchar;
		    /* Post-fill */
		    i = ((Size + 7) & ~0x07) - 1;
		    Size -= PostSize;
		    for (; i >= Size; i--)
				MemoryPtr[i] = mfillchar;

		    /* Set return values */
		    MemoryPtr += PreSize;
		    Size -= PreSize;
		    }
		else
		    {
		    /* No memory, reset size for snoop */
		    Size -= (PreSize + PostSize + INFOSIZE);
		    }
		LonelyHeart++;
		} 
	    else
		TaskOK = FALSE;
	    }
	if (!LonelyHeart)
	    {
	    MemoryPtr = (*old_AllocMem) (Size, Flags, SysBase);
	    D1(printf("Regular Memory at %lx Size %ld Flags 0x%lx\n", MemoryPtr, Size, Flags));
	    }

	D1(printf("TaskOK %ld\n", TaskOK));
	if ((Snoop == TRUE) && (TaskOK == TRUE))
	    {
	    printf(AMSnoopFmt, MemoryPtr, Size, Flags, ACaller, CCaller, GetCName(cname));

	    InfoDump(StackPtr, ACaller, CCaller, GetCName(cname), NULL);
	    }
	else if((MemoryPtr==0L)&&(ShowFail))
	    {
	    printf("\nFailed memory allocation!\n");
	    printf(AllocError, "", Size, Flags, GetCName(cname),
		SysBase->ThisTask, ACaller, CCaller, StackPtr);

	    InfoDump(StackPtr, ACaller, CCaller, GetCName(cname), NULL);

	    if (ErrorWait)
		{
		printf(Halting, cname);
		Wait(SIGBREAKF_CTRL_C);
		printf(Running, cname);
		}
	    }
	D(printf("AllocMem: Returning %lx\n", MemoryPtr));
	return (MemoryPtr);
}

VOID ASM new_FreeMem(REG(a1) UBYTE * MemoryPtr, REG(d0) ULONG Size, REG(a2) ULONG * StackPtr)
{
	struct timeval *tv;
	ULONG ACaller = StackPtr[ACALLER];
	ULONG CCaller = StackPtr[CCALLER];
	LONG errors;
	ULONG OldSize = 0;
	ULONG *FillMem;
	ULONG *zero;
	UBYTE cname[32];
	UBYTE *TmpPtr1;
	register ULONG i;
	BOOL TaskOK = TRUE;
	UWORD LonelyHeart = 0;
	int j;
	UBYTE mfillchar;

	cname[0] = 0;
	tv = NULL;

	D(printf("FreeMem\nParams: MemoryPtr:%lx Size:%ld\n", MemoryPtr, Size));
	D(printf("Caller: %s from A:%lx C:%lx\n Stack:%lx\n", GetCName(cname), ACaller, CCaller, StackPtr));

	/* Check for specific task */
	if (Taskname && (GetCName(cname)))
	    {
	    i = 0;
	    TaskOK = FALSE;
	    TmpPtr1 = (UBYTE *) ProcNames[i];
	    while (TmpPtr1 != NULL && TaskOK == FALSE)
		{
		if (strcmp(cname, TmpPtr1) == 0)
		    {
		    TaskOK = TRUE;
		    }
		TmpPtr1 = (UBYTE *) ProcNames[++i];
		}
	    if (Except) TaskOK ^= TRUE;
	    }

	/* Check NULL pointer and NULL size for everybody */
	if (MemoryPtr == NULL || Size == 0)
	    {
	    printf(FreeError, "", MemoryPtr, Size, GetCName(cname),
		SysBase->ThisTask, ACaller, CCaller, StackPtr);

	    InfoDump(StackPtr, ACaller, CCaller, GetCName(cname), NULL);

	    if (ErrorWait)
		{
		printf(Halting, cname);
		Wait(SIGBREAKF_CTRL_C);
		printf(Running, cname);
		}
	    /* Don't bother me anymore */
	    return;
	    }

	/* check misalignment for everybody */
	if ((ULONG)MemoryPtr != (((ULONG)MemoryPtr + 7) & ~0x07))
	    {
	    printf(FreeError, "Mis-aligned ", MemoryPtr, Size, GetCName(cname), SysBase->ThisTask, ACaller, CCaller, StackPtr);

	    InfoDump(StackPtr, ACaller, CCaller, GetCName(cname), NULL);

	    return;
	    }

	if (TaskOK)
	    {
	    if ((ACaller < LayersStart) || (ACaller > LayersEnd))
		{
		/* Check if cookie is OK */
		zero = FillMem = (ULONG *) (MemoryPtr - PreSize - INFOSIZE);
		FillMem++;

		tv = (struct timeval *)&zero[UDATEI];
		mfillchar = *((UBYTE *)&zero[UFILLI]);

		/* if cookie and per-session cookie both match */
		if ((*zero == (TAG1|TAG2))&&(zero[UKEEPI]==KeeperKey))
		    {
		    /* Generate checksum */
		    errors = *zero++ + *zero++ + *zero++ + *zero++ + *zero++;
		    if (*zero != errors)
			{
			printf("\nMungwall cookie - checksum failure\n");
			LonelyHeart++;
			}
		    else
			{
			/* Check if the caller indicated the correct size */
			errors = *FillMem++;
			if ((ULONG) Size != (ULONG) errors)
			    {
			    printf("\nMismatched FreeMem size %ld!\nOriginal allocation: %ld bytes from A:0x%lx C:0x%lx Task 0x%lx\nTesting with original size.\n", Size, errors,
				       *FillMem++, *FillMem++, *FillMem);
			    /* Switch to originally allocated size, to check hits after, don't free */
			    OldSize = Size + PreSize + PostSize + INFOSIZE;
			    Size = errors;
			    LonelyHeart++;
			    }
			errors = 0;
			Size += (PreSize + PostSize + INFOSIZE);
			MemoryPtr -= PreSize;

			i = 0;
			for (; i < PreSize; i++)
				if (MemoryPtr[i] != mfillchar)
							errors++;

			if (errors)
			    {
			    j = 0;

			    printf(BeforeHit, (errors == PreSize) ? "At least " : "", errors, MemoryPtr + PreSize, Size - (PreSize + PostSize + INFOSIZE), mfillchar);
			    /* Output damage area */
			    printf("$%08lx:",&MemoryPtr[4]);
			    for (i = 4; i < PreSize; i++)
				{
				if (i % 4 == 0)
				    {
				    printf(" ");
				    if (++j == 8) printf("\n    ");
				    }
			    printf("%s%s", hex((UBYTE) (MemoryPtr[i] >> 4)), hex(MemoryPtr[i]));
			    }
			printf("\n");
			/* Indicate there were errors here */
			LonelyHeart++;
			}
		    /* Check BABECOED */
		    errors = 0;
		    for (i = (Size - PostSize - INFOSIZE);
				i < ((Size - INFOSIZE + 7) & ~0x07); i++)
					if (MemoryPtr[i] != mfillchar) errors++;
		    if (errors)
			{
			int j = 0;

			printf(AfterHit, (errors >= PostSize) ? "At least " : "", errors, MemoryPtr + PreSize, Size - (PreSize + PostSize + INFOSIZE), mfillchar);

			/* Output damage area */
			i = (Size - PostSize - INFOSIZE) & ~0x03;
			printf("$%08lx:",&MemoryPtr[i]);
			for (; i < ((Size - INFOSIZE + 7) & ~0x07); i++)
			    {
			    if (i % 4 == 0)
				{
				printf(" ");
				if (++j == 8) printf("\n    ");
				}
			    printf("%s%s", hex((UBYTE) (MemoryPtr[i] >> 4)), hex(MemoryPtr[i]));
			    }
			printf("\n");
			/* Indicate there were errors */
			LonelyHeart++;
			}
		    if (LonelyHeart)
			{
			if (OldSize)	Size = OldSize;
			printf(FreeError,
				"  ", MemoryPtr + PreSize,
				Size - (PreSize + PostSize + INFOSIZE),
				GetCName(cname), SysBase->ThisTask,
				ACaller, CCaller, StackPtr);

	    		InfoDump(StackPtr, ACaller, CCaller, GetCName(cname), tv);

			if (ErrorWait)
			    {
			    printf(Halting, GetCName(cname));
			    Wait(SIGBREAKF_CTRL_C);
			    printf(Running, GetCName(cname));
			    }
			return;	/* Don't free, don't mung, get out of here.*/
                    	}
		    MemoryPtr -= INFOSIZE;
		    LonelyHeart++;	/* Set aways to indicate size & ptr should be reset for Snoop */
		    }
            	}
	    else	/* Not tagged as a Mungwall allocation */
		{
            	D(printf("Freeing Regular memory\n"));
            	}
	    }
	else  TaskOK = FALSE;
	}
	/* Mung everybody and everything who has no errors. */
	if(*MemoryPtr == (TAG1|TAG2))	*MemoryPtr = 0xC0DEFEED;

	/* We check for 0 rather than -1 because our asm entry Forbids */
	if (SysBase->TDNestCnt == 0)
	    {
	    D(printf("PostMunging memory...\n"));
	    PostMunging(MemoryPtr, Size);
	    }
#ifdef DEBUG
	else
	    {
	    D(printf("NOT PostMunging, TDNest=%ld (task next %ld)\n",
			SysBase->TDNestCnt,SysBase->ThisTask->tc_TDNestCnt));
	    }
#endif

	/* This snoop stuff used to be after call to old FreeMem */
	D1(printf("Snoop %ld TaskOK %ld\n", Snoop, TaskOK));

	if (Snoop == TRUE && TaskOK == TRUE)
	    {
	    /* Use LonelyHeart to check whether I've got to reset size & ptr */
	    if (LonelyHeart)
		{
		MemoryPtr += PreSize + INFOSIZE;
		Size -= (PreSize + PostSize + INFOSIZE);
		}
	    printf(FMSnoopFmt,
			0, MemoryPtr, Size, ACaller, CCaller, GetCName(cname));

   	    InfoDump(StackPtr, ACaller, CCaller, GetCName(cname), tv);
	    }

	D(printf("Freeing Memory at $%lx Size %ld\n", MemoryPtr, Size));

	(*old_FreeMem) (MemoryPtr, Size, SysBase);

	D(printf("FreeMem: Returning\n"));
}


UBYTE * ASM GetCName( REG(a0) UBYTE *cname)
    {
    if(*cname)	return(cname);
    else	return(GetCallerName(cname));
    }

VOID InfoDump(ULONG *stackp, ULONG acaller, ULONG ccaller,
		UBYTE *cname, struct timeval *tv)
    {
    if(TimeStamp)		datestringout(tv);
    if(ShowStack)		StackDump(stackp);
    if(ShowPC||ShowHunk) 	PCDump(acaller, ccaller, cname);
    }

VOID StackDump(ULONG *StackPtr)
	{
	printf(ShowStackS,
	   StackPtr[0],StackPtr[1],StackPtr[2],StackPtr[3],
	   StackPtr[4],StackPtr[5],StackPtr[6],StackPtr[7]);
	printf(ShowStackS,
	   StackPtr[8],StackPtr[9],StackPtr[10],StackPtr[11],
	   StackPtr[12],StackPtr[13],StackPtr[14],StackPtr[15]);
	printf(ShowStackS,
	   StackPtr[16],StackPtr[17],StackPtr[18],StackPtr[19],
	   StackPtr[20],StackPtr[21],StackPtr[22],StackPtr[23]);
	printf(ShowStackS,
	   StackPtr[24],StackPtr[25],StackPtr[26],StackPtr[27],
	   StackPtr[28],StackPtr[29],StackPtr[30],StackPtr[31]);
	}

VOID PCDump(ULONG acaller, ULONG ccaller, UBYTE *cname)
    {
    ULONG *up, hunk, offs, typemem;
    struct SegSem *segsem;
    UBYTE *name;

    segsem = (struct SegSem *)FindSemaphore("SegTracker");

    typemem = TypeOfMem((APTR)acaller);
    if(ShowHunk)
	{
	if(segsem)
	    {
	    if(name=(*segsem->seg_Find)((ULONG)acaller,&hunk,&offs))
		{
	    	printf(HunkOffs,"APC",acaller,name,hunk,offs);
		}
	    }
	else if(typemem)
	    {
	    if(InProcSegList(SysBase->ThisTask,(VOID *)acaller,&hunk,&offs,(struct Library *)SysBase))
	    	{
	    	printf(HunkOffs,"APC",acaller,cname,hunk,offs);
	    	}
	    }
	}

    if((ShowPC)&&(typemem)&&(!(acaller & 1)))  /* don't show pc if odd */
	{
	up = (ULONG *)acaller;	
	printf(PCS,"@APC",up[0],up[1],up[2],up[3],up[4],up[5],up[6],up[7]);
	printf(PCS,"@APC",up[8],up[9],up[10],up[11],up[12],up[13],up[14],up[15]);
	}

    typemem = TypeOfMem((APTR)ccaller);
    if(ShowHunk)
	{
	if(segsem)
	    {
	    if(name=(*segsem->seg_Find)((ULONG)ccaller,&hunk,&offs))
		{
	    	printf(HunkOffs,"CPC",ccaller,name,hunk,offs);
		}
	    }
	else if(InProcSegList(SysBase->ThisTask,(VOID *)ccaller,&hunk,&offs,(struct Library *)SysBase))
	    {
	    printf(HunkOffs,"CPC",ccaller,cname,hunk,offs);
	    }
	}
    if((ShowPC)&&(typemem)&&(!(ccaller & 1))) /* don't show pc if odd */
	{
	up = (ULONG *)ccaller;	
	printf(PCS,"@CPC",up[0],up[1],up[2],up[3],up[4],up[5],up[6],up[7]);
	printf(PCS,"@CPC",up[8],up[9],up[10],up[11],up[12],up[13],up[14],up[15]);
	}
    }


VOID FindLayers(VOID)
{
	struct Resident *layerstag;
	ULONG *resmodules;
	LONG i, j, k;

	D(printf("Findlayers:\n"));

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
	D(printf("LayerStart at %lx LayersEnd at %lx\n", LayersStart, LayersEnd));
}


VOID HandleSignal(VOID)
{
	ULONG signal;
	ULONG SIG_RTC;
	struct MungMessage *mmsg;
	UBYTE *Namestore, *Nameline;
	UBYTE *xtaskname = NULL;
	BOOL ABORT = FALSE;
	BOOL xsnoop,xwait,xexcept,xnametag,xshowstack;
	BOOL xshowfail,xshowpc,xshowhunk,xtimestamp;

	D(printf("HandleSignal:\n"));

	xsnoop = xwait = xexcept = xnametag =
		xshowstack = xshowfail = xshowpc = xshowhunk = xtimestamp = -1;

	/* Fails silently if loadf'ed */
	if (Nameline = AllocMem(1024, MEMF_CLEAR)) {
		Namestore = Nameline + 512;
		if (MungPort = (struct MsgPort *) AllocPort(Portname, -20)) {

			SIG_RTC = 1L << MungPort->mp_SigBit;

			while (ABORT == FALSE) {
				signal = Wait(SIGBREAKF_CTRL_C | SIG_RTC);

				/* USER BREAK SIGNAL */
				if (signal & SIGBREAKF_CTRL_C) {
					D(printf("BREAK SIGNAL!\n"));
					Disable();
					if (free_em()) {
						ABORT = TRUE;
						if (DOSBase)
							Write(Output(), Gone, strlen(Gone));
					} else {
						if (DOSBase)
							Write(Output(), NoExit, strlen(NoExit));
					}
					Enable();
				}
				/* RUN TIME CHANGE SIGNAL */
				if (signal & SIG_RTC) {
					D(printf("RTC SIGNAL!\n"));
					while (mmsg = (struct MungMessage *) GetMsg(MungPort)) {
						if (mmsg->mm_UpdateFlags & M_SNOOP)
							xsnoop = TRUE;
						if (mmsg->mm_UpdateFlags & M_WAIT)
							xwait = TRUE;
						if (mmsg->mm_UpdateFlags & M_NOSNOOP)
							xsnoop = FALSE;
						if (mmsg->mm_UpdateFlags & M_NOWAIT)
							xwait = FALSE;

						if (mmsg->mm_UpdateFlags & M_SHOWSTACK)
							xshowstack = TRUE;
						if (mmsg->mm_UpdateFlags & M_NOSHOWSTACK)
							xshowstack = FALSE;
						if (mmsg->mm_UpdateFlags & M_NAMETAG)
							xnametag = TRUE;
						if (mmsg->mm_UpdateFlags & M_NONAMETAG)
							xwait = FALSE;
						if (mmsg->mm_UpdateFlags & M_SHOWFAIL)
							xshowfail = TRUE;
						if (mmsg->mm_UpdateFlags & M_NOSHOWFAIL)
							xshowfail = FALSE;
						if (mmsg->mm_UpdateFlags & M_SHOWPC)
							xshowpc = TRUE;
						if (mmsg->mm_UpdateFlags & M_NOSHOWPC)
							xshowpc = FALSE;
						if (mmsg->mm_UpdateFlags & M_SHOWHUNK)
							xshowhunk = TRUE;
						if (mmsg->mm_UpdateFlags & M_NOSHOWHUNK)
							xshowhunk = FALSE;
						if (mmsg->mm_UpdateFlags & M_TIMESTAMP)
							xtimestamp = TRUE;
						if (mmsg->mm_UpdateFlags & M_NOTIMESTAMP)
							xtimestamp = FALSE;

						ConfigInfo = mmsg->mm_ConfigInfo;
						if (mmsg->mm_UpdateFlags & M_TASK) {
							Taskname = NULL;
							if (mmsg->mm_Taskname != NULL) {
								StrCpy(Namestore, mmsg->mm_Taskname);
								StrCpy(Nameline, mmsg->mm_Taskname);
								mmsg->mm_UpdateFlags |= ParseLine(Namestore);
								xtaskname = &Namestore[0];
							} else {
								Nameline[0] = '\0';
								Namestore[0] = '\0';
								xtaskname = NULL;
							}
						}
						if (mmsg->mm_UpdateFlags & M_EXCEPT)
							xexcept = TRUE;
						if (mmsg->mm_UpdateFlags & M_NOEXCEPT)
							xexcept = FALSE;
						ReplyMsg((struct Message *) mmsg);
						Forbid();
						if (ConfigInfo)
							printf(Settings, (xtaskname) ? Nameline : (Taskname) ? Nameline : "ALL", PreSize, PostSize, FillChar,
							       (xsnoop == TRUE) ? On : Off, (xwait == TRUE) ? On : Off);
						Permit();
						/* First taskname, except, wait, snoop */
						if (xtaskname)
							Taskname = xtaskname;
						if (xexcept != -1)
							Except = xexcept;
						else
							Except = FALSE;
						if (xwait != -1)
							ErrorWait = xwait;
						if (xsnoop != -1)
							Snoop = xsnoop;
						if (xshowstack != -1)
							ShowStack = xshowstack;
						if (xnametag != -1)
							NameTag = xnametag;
						if (xshowfail != -1)
							ShowFail = xshowfail;
						if (xshowpc != -1)
							ShowPC = xshowpc;
						if (xshowhunk != -1)
							ShowHunk = xshowhunk;
						if (xtimestamp != -1)
							TimeStamp = xtimestamp;

					}
				}
			}
			FreePort(MungPort);
		} else if (DOSBase)
			Write(Output(), NoPort, strlen(NoPort));
		FreeMem(Nameline, 1024);
	} else if (DOSBase)
		Write(Output(), NoMem, strlen(NoMem));
}

ULONG ParseLine(STRPTR Namestore)
{
	ULONG UpdateFlags = 0L;
	STRPTR ptr;
	int j;

	ptr = &Namestore[0];
	if (*ptr == '!') {
		Except = TRUE;
		UpdateFlags |= M_EXCEPT;
		ptr++;
	} else {
		Except = FALSE;
		UpdateFlags |= M_NOEXCEPT;
	}
	ProcNames[0] = (ULONG *) ptr;
	j = 1;
	while ((ptr = strchr(ptr, '|')) && (j < 10)) {
		*ptr = '\0';
		ProcNames[j] = (ULONG *)++ ptr;
		j++;
	}
	ProcNames[j] = NULL;
	ptr = (UBYTE *) ProcNames[j - 1];
	if (ptr = strchr(ptr, '|'))
		*ptr = '\0';
	return (UpdateFlags);
}

struct MsgPort *AllocPort(UBYTE * name, LONG pri)
{
	int sigBit;
	struct MsgPort *mp = NULL;

	if ((sigBit = AllocSignal(-1L)) != -1) {
		if (mp = AllocMem(sizeof(struct MsgPort), MEMF_CLEAR | MEMF_PUBLIC)) {
			mp->mp_Node.ln_Name = name;
			mp->mp_Node.ln_Pri = pri;
			mp->mp_Node.ln_Type = NT_MSGPORT;
			mp->mp_Flags = PA_SIGNAL;
			mp->mp_SigBit = sigBit;
			mp->mp_SigTask = (struct Task *) FindTask(0);
			if (name)
				AddPort(mp);
			else
				NewList(&(mp->mp_MsgList));
		} else
			FreeSignal(sigBit);
	}
	return (mp);
}

VOID FreePort(struct MsgPort * mp)
{
	if (mp->mp_Node.ln_Name)
		RemPort(mp);
	mp->mp_SigTask = (struct Task *) - 1;
	mp->mp_MsgList.lh_Head = (struct Node *) - 1;
	FreeSignal(mp->mp_SigBit);
	FreeMem(mp, (ULONG) sizeof(struct MsgPort));
}

UBYTE *StrCpy(register UBYTE * string1, register UBYTE * string2)
{
	UBYTE *ptr = string1;

	while (*string1++ = *string2++);
	return (ptr);
}

char *nums[] =
{
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"
};

UBYTE *hex(BYTE byte)
{
	return nums[byte & 0x0f];
}

/* return # of words in string. 13/11/90/EC fixed for 1.3's clunky argument lines. */

LONG maxword(UBYTE * string)
{
	int number = 0;
	UBYTE *ptr;

	ptr = strchr(string, '\n');

	/* Go for newline, since this has to work with 1.3 */
	if (string[0] != 10) {
		number++;
		while (ptr > string) {
			if (*ptr-- == ' ')
				number++;
		}
	}
	return (number);
}

/* get arg # n from string */
/* returns ptr to new end  */
char brkstr[] = " \t\n";

UBYTE *Getarg(UBYTE * dest, register UBYTE * string, register LONG number, LONG * maxchars, register LONG last)
{
	register char *temp;
	char save;

	string = stpblk(string);
	while (number-- > 0) {
		string += stcarg(string, brkstr);
		string = stpblk(string);
		last--;
	}

	temp = string;
	do {
		temp = stpblk(temp);
		temp += stcarg(temp, brkstr);
	} while (last-- > 0);

	if (temp == string)
		return (dest);
	save = *temp;
	*temp = '\0';
	dest = cpymax(dest, string, maxchars);
	*temp = save;
	return (dest);
}


UBYTE *cpymax(register UBYTE * dest, register UBYTE * src, register LONG * maxchars)
{
	/*  works like strcpy, but returns ptr to end of str. */
	/*  don't copy more than maxchar characters */

	while (*src && *maxchars > 0) {
		*dest++ = *src++;
		(*maxchars)--;
	}
	*dest = '\0';
	return (dest);
}



VOID datestringout(struct timeval *tv)
    {
    struct timeval TV;
    UBYTE date[16], time[16];

    if(!(TimeStamp&&TimerBase))	return;

    if((tv)&&(tv->tv_secs))
	{
    	datestring_tv(tv,NULL,date,time);
	printf(Time1,date,time);    
	}
    else printf(Time1,"unknown/none","");
 
    GetSysTime(&TV);
    datestring_tv(&TV,NULL,date,time);
    printf(Time2,date,time);
    }


/* datestring_tv() - Pass initialized timeval   and buffers of length 16 */
/* datestring_ds() - Pass initialized DateStamp and buffers of length 16 */

#define         SECS_PER_DAY            (60*60*24)
#define         SECS_PER_MINUTE         (60)
#define         TICKS_PER_SEC           (50)
#define         MICROS_PER_TICK         (1000000/50)

static UBYTE *monthnames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
			       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

static UBYTE *daynames[]   = { "Sunday", "Monday", "Tuesday", "Wednesday"
			       "Thursday", "Friday", "Saturday" };

static UBYTE *datefmt = "%02ld-%s-%02ld";
static UBYTE *timefmt = "%02ld:%02ld:%02ld";


VOID datestring_tv(struct timeval *tv,UBYTE *daybuf,UBYTE *datebuf,UBYTE *timebuf)
   {
   struct DateStamp ds;
   ds.ds_Days = tv->tv_secs / SECS_PER_DAY;
   ds.ds_Minute = (tv->tv_secs % SECS_PER_DAY) / SECS_PER_MINUTE;
   ds.ds_Tick = ((tv->tv_secs % SECS_PER_MINUTE) * TICKS_PER_SEC)
		+ (tv->tv_micro / MICROS_PER_TICK);
   datestring_ds(&ds,daybuf,datebuf,timebuf);
   }

VOID datestring_ds(struct DateStamp *ds,UBYTE *daybuf,UBYTE *datebuf,UBYTE *timebuf)
   {
   LONG year, month, dayoweek, n;
   LONG day, hour, minutes, seconds;

/*
   UBYTE *ampm = "AM";
*/


   dayoweek = ds->ds_Days % 7;

   seconds  = ds->ds_Tick / 50;
   minutes  = ds->ds_Minute % 60;
   hour     = ds->ds_Minute / 60;


/*
   if (hour >= 12)
	{
	hour = hour % 12;
	ampm = "PM";
	}
   if (hour==0)    hour = 12;
*/

   if (hour >= 24)
	{
	hour = hour % 24;
	}

   n = ds->ds_Days - 2251;
   year  = (4 * n + 3) / 1461;
   n -= 1461 * (long)year / 4;
   year += 1984;
   month = (5 * n + 2) / 153;
   day = n - (153 * month + 2) / 5 + 1;
   month += 3;
   if(month > 12)
      {
      year++ ;
      month -= 12 ;
      }

   if(daybuf)	sprintf(daybuf, daynames[dayoweek]);
   if(datebuf)	sprintf(datebuf, datefmt, day, monthnames[month-1], year%100);
   if(timebuf)	sprintf(timebuf, timefmt, hour, minutes, seconds);
   }

