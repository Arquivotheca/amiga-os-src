/*      $Id: mungwall.c,v 1.10 90/12/14 12:13:22 ewout Exp Locker: ewout $
**
**      Mungwall. Munges and traces memory trashing.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
**
**
*/
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/resident.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <ctype.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#define ASM __asm
#define REG(x) register __## x

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

#define INFOSIZE         24	/* Size of area to store information, like size, caller etc. */
/*
 * A mungwall info area looks like this:
 *
 * C0EDBABE		4 bytes
 * SIZE			4
 * ACALLER		4
 * CCALLER		4
 * TASKADDRESS	4
 * CHECKSUM		4
 *
 * WALL			presize
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
extern BOOL ErrorWait;		/* Halt caller in case of error */
extern UBYTE FillChar;		/* Character to use to fill pre- and postmemory */

/*** GLOBALS USED TO CHECK FOR LAYERS AND SHOW CALLER **********************************************/
extern ULONG LayersStart;
extern ULONG LayersEnd;
//extern ULONG *StackPtr;
extern BYTE ConfigInfo;
extern struct MsgPort *MungPort;
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

/* Message to update an existing mungwall task */
struct MungMessage {
	struct Message mm_Message;
	ULONG mm_UpdateFlags;
	UBYTE *mm_Taskname;
	UBYTE mm_ConfigInfo;
};

/*** PROTOTYPES ************************************************************************************/
extern VOID *(*ASM old_AllocMem) (REG(d0) ULONG, REG(d1) ULONG);
extern VOID(*ASM old_FreeMem) (REG(a1) VOID *, REG(d0) ULONG);
extern VOID ASM InitMunging(REG(a1) UBYTE *, REG(d0) ULONG);
extern VOID ASM PreMunging(REG(a1) UBYTE *, REG(d0) ULONG);
extern VOID ASM PostMunging(REG(a1) UBYTE *, REG(d0) ULONG);

VOID *ASM new_AllocMem(REG(d0) ULONG, REG(d1) ULONG, REG(a2) ULONG *);
VOID ASM new_FreeMem(REG(a1) UBYTE *, REG(d0) ULONG, REG(a2) ULONG *);

LONG startup(STRPTR args);
VOID FindLayers(VOID);
VOID HandleSignal(VOID);

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
UBYTE *Copyright = "Copyright (c) 1990 Commodore-Amiga, Inc.\n";
UBYTE *Portname = "Mungwall";

/* Snoop compatible output strings */
UBYTE *AMSnoopFmt = "$%08lx=AllocMem(%6ld,$%08lx) A:%08lx C:%08lx   \"%s\"\n";
UBYTE *FMSnoopFmt = "$%08lx= FreeMem($%08lx,%6ld) A:%08lx C:%08lx   \"%s\"\n";

/* Exit strings */
UBYTE *CBreak = "CTRL-C or BREAK to exit...\n";
UBYTE *NoExit = "Can't exit now. Unable to undo SetFunction()'s\n";
UBYTE *Gone = "Mungwall removed\n";

/* Waiting/running strings if ErrorWait == TRUE */
UBYTE *Halting = "Halting task `%s' - waiting for CTRL-C...\n";
UBYTE *Running = "Running task `%s'.\n";

/* Error report strings */
UBYTE *FreeError = "\007\n%sFreeMem(0x%lx,%ld) attempted by `%s' (at 0x%lx)\n  from A:0x%lx C:0x%lx SP:0x%lx\n";
UBYTE *BeforeHit = "\n%s%ld byte(s) before allocation at 0x%lx, size %ld were hit!\n";
UBYTE *AfterHit = "\n%s%ld byte(s) after allocation at 0x%lx, size %ld were hit!\n";

UBYTE *Settings = "\nMungwall: Task: %s\nPreSize: %ld, PostSize: %ld, FillChar: 0x%lx, Snoop: %s Wait: %s\n";
UBYTE *On = "ON";
UBYTE *Off = "OFF";
UBYTE *Atleast = "At least ";

/* Error strings */
UBYTE *Usage = "Usage: Mungwall [UPDATE][TASK name][WAIT][NOWAIT]\n	        [SNOOP][NOSNOOP][INFO]\n	        [PRESIZE][POSTSIZE][FILLCHAR]\n";
UBYTE *NoMem = "Out of memory\n";
UBYTE *NoPort = "Can't create port\n";

struct Library *DOSBase = NULL;
extern struct ExecBase *SysBase;

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
				} else {
					Write(Output(), Usage, strlen(Usage));
					ABORT = TRUE;
					UPDATE = FALSE;
				}
			}
			/* Just to be sure! */
			if (!ABORT)
				UPDATE = FALSE;

			if (!ABORT) {
				Forbid();
				if (!(FindPort(Portname))) {
					/* Confirm settings */
					if (ConfigInfo)
						printf(Settings, (Taskname) ? Nameline : "ALL", PreSize, PostSize, FillChar,
						       (Snoop) ? On : Off, (ErrorWait) ? On : Off);

					/* Setfunction stuff, also finds layers start & end */
					grab_em();

					/* Write location Zero, if enforcer is not running */
					if (!(FindPort("_The Enforcer_"))) {
						zero = NULL;
						*zero = (ULONG) 0xC0DEDBAD;
					}
					/* Tell 'em we're here */
					Write(Output(), (STRPTR) VerTitle, strlen((STRPTR) VerTitle));
					Write(Output(), Copyright, strlen(Copyright));

					/* 13/11/90/EC Mung FreeMem list. Means loadf'ed mungwall's won't
		                         * do this. Fix?
		                         */
					Write(Output(), "Munging free memory...\n", 23);

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

					Write(Output(), CBreak, strlen(CBreak));
					Permit();

					HandleSignal();

					/* Set zero to zero again */
					if (!(FindPort("_The Enforcer_")))
						*zero = 0L;
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

VOID *ASM new_AllocMem(REG(d0) ULONG Size, REG(d1) ULONG Flags, REG(a2) ULONG * StackPtr)
{
	struct ExecBase *SysBase = (void *) getreg(REG_A6);
	ULONG ACaller = StackPtr[ACALLER];
	ULONG CCaller = StackPtr[CCALLER];
	STRPTR CallerName = NULL;
	STRPTR TmpPtr1;
	STRPTR TmpPtr2;
	struct CommandLineInterface *cli;
	UBYTE *MemoryPtr = NULL;
	ULONG *FillMem;
	ULONG i;
	UWORD LonelyHeart = 0;
	BOOL TaskOK = TRUE;

	D(printf("AllocMem:\n"));

	CallerName = (STRPTR) SysBase->ThisTask->tc_Node.ln_Name;

	if (SysBase->ThisTask->tc_Node.ln_Type == NT_PROCESS) {
		if (((struct Process *) SysBase->ThisTask)->pr_TaskNum) {
			cli = (struct CommandLineInterface *)
			    ((((struct Process *) SysBase->ThisTask)->pr_CLI) << 2);
			if (cli) {
				if (!(cli->cli_CommandName & 0xC0000000)) {
					TmpPtr1 = (STRPTR) ((cli->cli_CommandName) << 2);
					if (TmpPtr1) {
						i = TmpPtr1 ? TmpPtr1[0] : 0;
						if (i) {
							TmpPtr2 = TmpPtr1 + i;
							while (*TmpPtr2 != '/' && *TmpPtr2 != ':' && TmpPtr2 > TmpPtr1)
								TmpPtr2--;
							CallerName = ++TmpPtr2;
						}
					}
				}
			}
		}
	}
	D(printf("Params: Size:%ld Flags:0x%lx\n", Size, Flags));
	D1(printf("Caller: %s from A:%lx C:%lx Stack:%lx\n", (CallerName) ? CallerName : "<NULL>", ACaller, CCaller, StackPtr));

	/* Check for specific task */
	if (Taskname != NULL && CallerName != NULL) {
		i = 0;
		TaskOK = FALSE;
		TmpPtr1 = (UBYTE *) ProcNames[i];
		while (TmpPtr1 != NULL && TaskOK == FALSE) {
			if (strcmp(CallerName, TmpPtr1) == 0)
				TaskOK = TRUE;
			TmpPtr1 = (UBYTE *) ProcNames[++i];
		}
		if (Except)
			TaskOK ^= TRUE;
	}

	/* Requested size is big. Too big to add wall */
	if (Size & 0xC0000000)
		TaskOK = FALSE;

	if (TaskOK) {
		if (ACaller < LayersStart || ACaller > LayersEnd) {
			Size += (PreSize + PostSize + INFOSIZE);

			MemoryPtr = (*old_AllocMem) (Size, Flags);

			D(printf("LonelyHeart Memory at %lx Size %ld Flags 0x%lx\n", MemoryPtr, Size, Flags));
			if (MemoryPtr) {
				/* $DEADF00D Pre-Munging */
				if (!(Flags & MEMF_CLEAR))
					PreMunging(MemoryPtr, Size);

				/* Fill INFO area */
				FillMem = (ULONG *) MemoryPtr;
				i = (ULONG) (Size - (PreSize + PostSize + INFOSIZE));
				*FillMem++ = 0xC0EDBABE;
				*FillMem++ = i;
				*FillMem++ = (ULONG) ACaller;
				*FillMem++ = (ULONG) CCaller;
				*FillMem++ = (ULONG) SysBase->ThisTask;
				*FillMem = (ULONG) 0xC0EDBABE + i + (ULONG) ACaller + (ULONG) CCaller + (ULONG) SysBase->ThisTask;

				MemoryPtr += INFOSIZE;
				Size -= INFOSIZE;

				/* Fill pad area */
				for (i = 0; i < PreSize; i++)
					MemoryPtr[i] = FillChar;
				/* Post-fill */
				i = ((Size + 7) & ~0x07) - 1;
				Size -= PostSize;
				for (; i >= Size; i--)
					MemoryPtr[i] = FillChar;

				/* Set return values */
				MemoryPtr += PreSize;
				Size -= PreSize;
			} else {
				/* No memory, reset size for snoop */
				Size -= (PreSize + PostSize + INFOSIZE);
			}
			LonelyHeart++;
		} else
			TaskOK = FALSE;
	}
	if (!LonelyHeart) {
		MemoryPtr = (*old_AllocMem) (Size, Flags);
		D1(printf("Regular Memory at %lx Size %ld Flags 0x%lx \n", MemoryPtr, Size, Flags));
	}
	if (Snoop == TRUE && TaskOK == TRUE) {
		printf(AMSnoopFmt, MemoryPtr, Size, Flags, ACaller, CCaller, (CallerName) ? CallerName : "<NULL>");
	}
	D(printf("AllocMem: Returning %lx\n", MemoryPtr));
	return (MemoryPtr);
}

VOID ASM new_FreeMem(REG(a1) UBYTE * MemoryPtr, REG(d0) ULONG Size, REG(a2) ULONG * StackPtr)
{
	struct ExecBase *SysBase = (void *) getreg(REG_A6);
	ULONG ACaller = StackPtr[ACALLER];
	ULONG CCaller = StackPtr[CCALLER];
	LONG errors = 0;
	ULONG OldSize = 0;
	ULONG *FillMem;
	ULONG *zero;
	STRPTR CallerName;
	UBYTE *TmpPtr1;
	UBYTE *TmpPtr2;
	struct CommandLineInterface *cli;
	register ULONG i;
	BOOL TaskOK = TRUE;
	UWORD LonelyHeart = 0;

	D(printf("FreeMem:\n"));

	CallerName = (STRPTR) SysBase->ThisTask->tc_Node.ln_Name;

	if (SysBase->ThisTask->tc_Node.ln_Type == NT_PROCESS) {
		if (((struct Process *) SysBase->ThisTask)->pr_TaskNum) {
			cli = (struct CommandLineInterface *)
			    ((((struct Process *) SysBase->ThisTask)->pr_CLI) << 2);
			if (cli) {
				if (!(cli->cli_CommandName & 0xC0000000)) {
					TmpPtr1 = (UBYTE *) ((cli->cli_CommandName) << 2);
					if (TmpPtr1) {
						i = (TmpPtr1) ? TmpPtr1[0] : 0;
						if (i != 0) {
							TmpPtr2 = TmpPtr1 + i;
							while (*TmpPtr2 != '/' && *TmpPtr2 != ':' && (ULONG) TmpPtr2 > (ULONG) TmpPtr1)
								TmpPtr2--;
							CallerName = ++TmpPtr2;
						}
					}
				} else if (cli->cli_CommandName != 0xDEADBEEF) {
					printf("\nBogus cli_CommandName PTR: 0x%lx\n", cli->cli_CommandName);
					printf(FreeError, "  ", MemoryPtr, Size,
					       (CallerName) ? CallerName : "<NULL>", SysBase->ThisTask, ACaller, CCaller, StackPtr);
				}
			}
		}
	}
	D(printf("Params: MemoryPtr:%lx Size:%ld\n", MemoryPtr, Size));
	D1(printf("Caller: %s from A:%lx C:%lx\n Stack:%lx", (CallerName) ? CallerName : "<NULL>", ACaller, CCaller, StackPtr));


	/* Check for specific task */
	if (Taskname != NULL && CallerName != NULL) {
		i = 0;
		TaskOK = FALSE;
		TmpPtr1 = (UBYTE *) ProcNames[i];
		while (TmpPtr1 != NULL && TaskOK == FALSE) {
			if (strcmp(CallerName, TmpPtr1) == 0) {
				TaskOK = TRUE;
			}
			TmpPtr1 = (UBYTE *) ProcNames[++i];
		}
		if (Except)
			TaskOK ^= TRUE;
	}
	/* Check NULL pointer and NULL size for everybody */
	if (MemoryPtr == NULL || Size == 0) {
		printf(FreeError, "", MemoryPtr, Size, (CallerName) ? CallerName : "<NULL>", SysBase->ThisTask, ACaller, CCaller, StackPtr);
		if (ErrorWait) {
			printf(Halting, (CallerName) ? CallerName : "<NULL>");
			Wait(SIGBREAKF_CTRL_C);
			printf(Running, (CallerName) ? CallerName : "<NULL>");
		}
		/* Don't bother me anymore */
		return;
	}
	if (TaskOK) {
		if ((ACaller < LayersStart) || (ACaller > LayersEnd)) {
			/* Check if cookie is OK */
			zero = FillMem = (ULONG *) (MemoryPtr - PreSize - INFOSIZE);
			FillMem++;

			if (*zero == 0xC0EDBABE) {
				/* Generate checksum */
				errors = *zero++ + *zero++ + *zero++ + *zero++ + *zero++;
				if (*zero != errors) {
					printf("\nCookie checksum failure!!!\n");
					LonelyHeart++;
				} else {
					/* Check if the caller indicated the correct size */
					errors = *FillMem++;
					if ((ULONG) Size != (ULONG) errors) {
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
						if (MemoryPtr[i] != FillChar)
							errors++;

					if (errors) {
						int j = 0;

						printf(BeforeHit, (errors == PreSize) ? "At least " : "", errors, MemoryPtr + PreSize, Size - (PreSize + PostSize + INFOSIZE));
						/* Output damage area */
						printf(">$:");
						for (i = 4; i < PreSize; i++) {
							if (i % 4 == 0) {
								printf(" ");
								if (++j == 8)
									printf("\n    ");
							}
							printf("%s%s", hex((UBYTE) (MemoryPtr[i] >> 4)), hex(MemoryPtr[i]));
						}
						printf("\n");
						/* Indicate there were errors here */
						LonelyHeart++;
					}
					/* Check BABECOED */
					errors = 0;
					for (i = (Size - PostSize - INFOSIZE); i < ((Size - INFOSIZE + 7) & ~0x07); i++)
						if (MemoryPtr[i] != FillChar)
							errors++;
						if (errors) {
						int j = 0;

						printf(AfterHit, (errors >= PostSize) ? "At least " : "", errors, MemoryPtr + PreSize, Size - (PreSize + PostSize + INFOSIZE));

						/* Output damage area */
						i = (Size - PostSize - INFOSIZE) & ~0x03;
						printf(">$:");
						for (; i < ((Size - INFOSIZE + 7) & ~0x07); i++) {
							if (i % 4 == 0) {
								printf(" ");
								if (++j == 8)
									printf("\n    ");
							}
							printf("%s%s", hex((UBYTE) (MemoryPtr[i] >> 4)), hex(MemoryPtr[i]));
						}
						printf("\n");
						/* Indicate there were errors */
						LonelyHeart++;
					}
					if (LonelyHeart) {
						if (OldSize)
							Size = OldSize;
						printf(FreeError, "  ", MemoryPtr + PreSize, Size - (PreSize + PostSize + INFOSIZE),
						       (CallerName) ? CallerName : "<NULL>", SysBase->ThisTask, ACaller, CCaller, StackPtr);
						if (ErrorWait) {
							printf(Halting, (CallerName) ? CallerName : "<NULL>");
							Wait(SIGBREAKF_CTRL_C);
							printf(Running, (CallerName) ? CallerName : "<NULL>");
						}
						return;	/* Don't free, don't mung, get out of here.*/
                    }
					MemoryPtr -= INFOSIZE;
					LonelyHeart++;	/* Set aways to indicate size & ptr should be reset for Snoop */
			    }
            }
		} else
			TaskOK = FALSE;
	}
	/* Mung everybody and everything who has no errors. */

	if (SysBase->TDNestCnt == 0) {
		D1(printf("Munging memory\n"));
		PostMunging(MemoryPtr, Size);
	}
	D(printf("Freeing Memory at %lx Size %ld\n", MemoryPtr, Size));

	(*old_FreeMem) (MemoryPtr, Size);

	if (Snoop == TRUE && TaskOK == TRUE) {
		/* Use LonelyHeart to check whether I've got to reset size & ptr */
		if (LonelyHeart) {
			MemoryPtr += PreSize + INFOSIZE;
			Size -= (PreSize + PostSize + INFOSIZE);
		}
		printf(FMSnoopFmt, 0, MemoryPtr, Size, ACaller, CCaller, (CallerName) ? CallerName : "<NULL>");
	}
	D(printf("FreeMem: Returning\n"));
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
	BOOL xsnoop, xwait, xexcept;

	D(printf("HandleSignal:\n"));

	xsnoop = xwait = xexcept = -1;

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
