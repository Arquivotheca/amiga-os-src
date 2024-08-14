/*
**	$Header: /usr.MC68010/ghostwheel/darren/V38/clipboard/RCS/dev.c,v 36.9 92/04/14 11:49:15 darren Exp $
**
**      clipboard.device
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
#ifdef	DEBUG
#define	D(a)	kprintf a
#else
#define	D(a)
#endif

#include        "exec/types.h"
#include        "exec/nodes.h"
#include        "exec/lists.h"
#include        "exec/memory.h"
#include        "exec/libraries.h"
#include        "exec/io.h"
#include        "exec/errors.h"
#include	"exec/execbase.h"
#include        "dos/dos.h"
#include        "dos/dosextens.h"
#include	"utility/hooks.h"
#include	"clipboard.h"

/*********************************************************************
**********************************************************************
*
*	Device Initialization
*
**********************************************************************
*********************************************************************/
extern	int Read();
extern	int Write();


/* clipboard device data */

#define	BLOCKSIZE		512
#define	BLOCKMASK		0x01ff
#define	BLOCKSHIFT		9
#define	CLIPSIZECHECK		2000
#define	LOWMEMMARK		0x4000	/* 16K */
#define	IOF_DONE		0x80	/* returned to the user */
#define	IOF_ACTIVE		0x40	/* this will be done soon */
#define	IOF_QUEUED		0x20	/* on a list somewhere */
#define	IOF_PENDINGREAD		0x10	/* this group of reads not started */

struct	ExecBase *SysBase;
struct	Library *DOSBase;
struct	Library *UtilityBase;
ULONG	cd_Segment;
struct	List cd_Units;
struct	SignalSemaphore cd_SS;


/* clip block */
struct ClipBlock {
    struct  Node cb_Node;		/* list of blocks */
    LONG    cb_BlockNum;		/* block number (origin 0) */
    UBYTE   cb_Data[BLOCKSIZE];		/* block data */
};


#define	OPEN_OK			0
#define	OPEN_PROGRESSING	1

/* clipboard unit data */
struct ClipboardUnit {
    struct  ClipboardUnitPartial cu_CUP;/* list of units */
    ULONG   cu_OpenCnt;			/* number of accessors for unit */
    struct  MsgPort cu_PendingOpens;	/* other open requests waiting */
    LONG    cu_PendingID;		/* latest clip identifier */
    LONG    cu_ProcessingID;		/* clip identifier being processed */
    UWORD   cu_ProcessingCommand;	/* command in progress */
    BYTE    cu_WorkLocked;		/* doWork in progress */
    BYTE    cu_OpenFlags;		/* Open() handshake flags */

    ULONG   cu_DiskFile;		/* disk file for clip, if necessary */
    char    cu_DiskFileName[28];	/* name associated w/ cu_DiskFile */
    struct  List cu_BlockHead;		/* list of clip blocks */
    struct  ClipBlock *cu_CurrBlock;	/* most recently accessed block */
    ULONG   cu_CurrentLength;		/* length of clip */
    BOOL    cu_CutValid;		/* false if clip contents not valid */

    struct  List cu_ReadPending;	/* pending read commands, sorted */
    UWORD   cu_CurrentReadCount;	/* number of reads w/ ID w/o EOF */

    struct  List cu_CutPending;		/* pending cut commands, sorted */
    struct  SatisfyMsg cu_SatisfyMsg;	/* message to send to satisfy post */
    struct  MsgPort *cu_SatisfyMsgPort;	/* message port for current post */

    struct  SignalSemaphore cu_ChangeSemaphore;
    struct  MinList cu_ChangeHooks;	/* active change hooks */
};


freeCD(cd)
struct Library *cd;
{
    UBYTE *freePtr;
    ULONG freeSize;

    freePtr = (UBYTE *) cd;
    freeSize = cd->lib_NegSize;
    freePtr -= freeSize;
    freeSize += cd->lib_PosSize;
    FreeMem(freePtr, freeSize);
}


struct Library *
CInit(cd, segment, sysBase)
struct Library *cd;
ULONG segment;
struct ExecBase *sysBase;
{
    UBYTE *failPtr;
    ULONG failSize;

    SysBase = sysBase;
    if ((UtilityBase = (struct Library *) OpenLibrary("utility.library", 0))
	    == 0) {
	freeCD(cd);
	return(0);
    }
    if ((DOSBase = (struct Library *) OpenLibrary("dos.library", 0)) == 0) {
	CloseLibrary(UtilityBase);
	freeCD(cd);
	return(0);
    }
    cd_Segment = segment;
    InitSemaphore(&cd_SS);
    NewList(&cd_Units);
    D(("CInit: device node 0x%lx\n", cd));
    return(cd);
}



/*********************************************************************
**********************************************************************
*
*	support functions
*
**********************************************************************
*********************************************************************/

struct ClipBlock
*newTailBlock(cu)
struct ClipboardUnit *cu;
{
    struct ClipBlock *newBlock;

    D(("newTailBlock(0x%06lx): ", cu));
    Forbid();
    if (newBlock = (struct ClipBlock *)
	    AllocMem(sizeof(*newBlock), MEMF_CLEAR)) {
	if (cu->cu_BlockHead.lh_TailPred->ln_Pred) {
	    /* the list is not empty */
	    newBlock->cb_BlockNum =
		    ((struct ClipBlock *) cu->cu_BlockHead.lh_TailPred)
		    ->cb_BlockNum + 1;
	}
	else {
	    /* the list is empty, this is the first block */
	    newBlock->cb_BlockNum = 0;
	}
	AddTail(&cu->cu_BlockHead, newBlock);
	D(("# %ld ", newBlock->cb_BlockNum));
    }
    else {
	cu->cu_CutValid = FALSE;
    }
    Permit();
    D(("@ 0x%06lx\n", newBlock));
    return(newBlock);
}

ULONG
openDiskClip(cu, mode)
struct ClipboardUnit *cu;
int mode;
{
    struct Process *pr;
    APTR prWindowPtr;
    int lock1, lock2;
    int tpr, file;

    file = 0;

    /* disable any dos requestor */
    pr = (struct Process *) FindTask(0);
    prWindowPtr = pr->pr_WindowPtr;
    pr->pr_WindowPtr = (APTR) -1;

    /* first see if the new "CLIPS:" assign has been made */
    tpr = DeviceProc("CLIPS:");

    /* re-enable any dos requestor */
    pr->pr_WindowPtr = prWindowPtr;

    if (tpr == 0) {
	/* "CLIPS:" does not exist, use the default "DEVS:clipboards/" */
	if ((lock1 = Lock("DEVS:", SHARED_LOCK)) != 0) {
	    if (((lock2 = Lock("DEVS:clipboards", SHARED_LOCK)) == 0) &&
		    (mode == MODE_NEWFILE)) {
		lock2 = CreateDir("DEVS:clipboards");
	    }
	    if (lock2 != 0) {
		sprintf(cu->cu_DiskFileName, "DEVS:clipboards/%ld",
			cu->cu_CUP.cu_UnitNum);
		file = Open(cu->cu_DiskFileName, mode);
		UnLock(lock2);
	    }
	    UnLock(lock1);
	}
    }
    else {
	sprintf(cu->cu_DiskFileName, "CLIPS:%ld", cu->cu_CUP.cu_UnitNum);
	file = Open(cu->cu_DiskFileName, mode);
    }

    return(file);
}

clearClip(cu)
struct ClipboardUnit *cu;
{
    struct ClipBlock *block, *nextBlock;

    D(("clearClip(0x%06lx);\n", cu));
    Forbid();
    block = cu->cu_BlockHead.lh_Head;
    while (nextBlock = (struct ClipBlock *) block->cb_Node.ln_Succ) {
	D(("  # %ld @ 0x%06lx\n", block->cb_BlockNum, block));
	FreeMem(block, sizeof(*block));
	block = nextBlock;
    }
    NewList(&cu->cu_BlockHead);
    cu->cu_CurrBlock = 0;
    Permit();
}

    
int
readClip(cu, ior, data, length, offset)
struct ClipboardUnit *cu;
struct IOClipReq *ior;
UBYTE *data;
ULONG length;
ULONG offset;
{
    struct ClipBlock *block;
    ULONG blockNum, index, actual, remaining;
    short i;

    D(("r(0x%06lx, %ld, %ld): ", data, length, offset));
    Forbid();
    actual = 0;
    if (cu->cu_CutValid) {
	if (cu->cu_DiskFile) {
	    /* about to loose Forbid: protect this IO */
	    ior->io_Flags |= IOF_ACTIVE;
	    if (Seek(cu->cu_DiskFile, offset, OFFSET_BEGINING) != -1) {
		actual = Read(cu->cu_DiskFile, data, length);
	    }
	}
	else {
	    if (offset < cu->cu_CurrentLength) {
		if (offset + length > cu->cu_CurrentLength) {
		    actual = cu->cu_CurrentLength - offset;
		}
		else {
		    actual = length;
		}
		if (actual) {
		    /* the starting data will be here */
		    blockNum = offset >> BLOCKSHIFT;
		    index = offset & BLOCKMASK;
		    if ((block = cu->cu_CurrBlock) == 0) {
			block = cu->cu_BlockHead.lh_Head;
		    }
		    if (block->cb_BlockNum != blockNum) {
			while (block->cb_BlockNum > blockNum) {
			    block = (struct ClipBlock *) block->cb_Node.ln_Pred;
			}
			while (block->cb_BlockNum < blockNum) {
			    block = (struct ClipBlock *) block->cb_Node.ln_Succ;
			}
			D(("\nnew read block: 0x%06lx, # %ld\n",
				block, block->cb_BlockNum));
		    }
		    remaining = actual;
		    while (remaining != 0) {
			if (BLOCKSIZE-index < remaining) {
			    for (i = index; i < BLOCKSIZE; i++) {
				*data++ = block->cb_Data[i];
			    }
			    block = (struct ClipBlock *) block->cb_Node.ln_Succ;
			    D(("\ncont'd. read block: 0x%06lx, # %ld\n",
				    block, block->cb_BlockNum));
			    remaining -= BLOCKSIZE-index;
			    index = 0;
			}
			else {
			    for (i = index; i < index+remaining; i++) {
				*data++ = block->cb_Data[i];
			    }
			    remaining = 0;
			}
		    }
		    cu->cu_CurrBlock = block;
		}
	    }
	}
    }
    Permit();
    return(actual);
}


int
writeClip(cu, ior, data, length, offset)
struct ClipboardUnit *cu;
struct IOClipReq *ior;
UBYTE *data;
ULONG length;
ULONG offset;
{
    struct ClipBlock *block, *nextBlock;
    ULONG blockNum, index, actual;
    ULONG file;
    short i;

    D(("w(0x%06lx, %ld, %ld): ", data, length, offset));
    Forbid();
    D((cu->cu_CutValid?"":"CUT NOT VALID!\n"));
    actual = 0;
    if ((!(cu->cu_DiskFile)) &&
	    ((offset + length) > CLIPSIZECHECK) &&
	    (cu->cu_CutValid) &&
	    (AvailMem(MEMF_LARGEST) < LOWMEMMARK)) {
	if (cu->cu_DiskFile = openDiskClip(cu, MODE_NEWFILE)) {
	    /* copy blocks to disk */
	    block = cu->cu_BlockHead.lh_Head;
	    while (nextBlock = (struct ClipBlock *) block->cb_Node.ln_Succ) {
		if (block->cb_BlockNum*BLOCKSIZE < cu->cu_CurrentLength) {
		    if ((block->cb_BlockNum+1)*BLOCKSIZE
			    > cu->cu_CurrentLength) {
			if (Write(cu->cu_DiskFile, block->cb_Data,
				cu->cu_CurrentLength & BLOCKMASK) !=
				(cu->cu_CurrentLength & BLOCKMASK)) {
			    cu->cu_CutValid = FALSE;
			}
		    }
		    else {
			if (Write(cu->cu_DiskFile, block->cb_Data, BLOCKSIZE) !=
				    BLOCKSIZE) {
				cu->cu_CutValid = FALSE;
			    }
		    }
		}
		FreeMem(block, sizeof(*block));
		block = nextBlock;
	    }
	    NewList(&cu->cu_BlockHead);
	}
	else {
	    cu->cu_CutValid = FALSE;
	}
    }
    if (cu->cu_CutValid) {
	if (cu->cu_DiskFile) {
	    D(("(write to disk) "));
	    /* about to loose Forbid: protect this IO */
	    ior->io_Flags |= IOF_ACTIVE;
	    if (Seek(cu->cu_DiskFile, offset, OFFSET_BEGINING) != -1) {
		actual = Write(cu->cu_DiskFile, data, length);
	    }
	}
	else {
	    blockNum = offset >> BLOCKSHIFT;
	    index = offset & BLOCKMASK;
	    if ((block = cu->cu_CurrBlock) == 0) {
		block = newTailBlock(cu);
	    }
	    if (block) {
		if (block->cb_BlockNum != blockNum) {
		    while (block->cb_BlockNum > blockNum) {
			block = (struct ClipBlock *) block->cb_Node.ln_Pred;
		    }
		    while (block && (block->cb_BlockNum < blockNum)) {
			block = (struct ClipBlock *) block->cb_Node.ln_Succ;
			if (block->cb_Node.ln_Succ == 0) {
			    /* build a new node here */
			    block = newTailBlock(cu);
			}
		    }
		}
	    }
	    while (block && (length > 0)) {
		if (BLOCKSIZE-index < length) {
		    D(("\n    finish block: %ld to %ld\n    next block ",
			    index, BLOCKSIZE));
		    for (i = index; i < BLOCKSIZE; i++) {
			block->cb_Data[i] = *data++;
		    }
		    block = (struct ClipBlock *) block->cb_Node.ln_Succ;
		    if (block->cb_Node.ln_Succ == 0) {
			/* build a new node here */
			block = newTailBlock(cu);
			D(("(NEW) "));
		    }
		    actual += BLOCKSIZE-index;
		    length -= BLOCKSIZE-index;
		    index = 0;
		    D(("\n    index now zero, actual %ld, and length %ld\n",
			    actual, length));
		}
		else {
		    for (i = index; i < index+length; i++) {
			block->cb_Data[i] = *data++;
		    }
		    actual += length;
		    length = 0;
		}
	    }
	    cu->cu_CurrBlock = block;
	}
    }
    Permit();
    return(actual);
}


callHook(hook, object, message)
struct Hook *hook;
struct ClipboardUnit *object;
ULONG message;
{
    CallHookPkt(hook, object, &message);
}


endIO(io)
struct IOClipReq *io;
{
    Forbid();
    if (!(io->io_Flags & IOF_DONE)) {
	/* this is the completion of the IO */
	if ((io->io_Flags & IOF_QUICK) == 0)
	    ReplyMsg(io);
    }
    else
	/* the IO has already been completed */
	io->io_Flags &= ~IOF_QUICK;
    io->io_Flags |= IOF_DONE;
    Permit();
    D(("actual(%ld) error(%ld)\n", io->io_Actual, io->io_Error));
}


endQueuedIO(io, ior)
struct IOClipReq *io;
struct IOClipReq *ior;
{
    if (!(io->io_Flags & IOF_DONE))
	Remove(io);
    if (io != ior)
	io->io_Flags &= ~IOF_QUICK;
    endIO(io);
    io->io_Flags &= ~IOF_QUEUED;
}


BOOL
nextPendingReading(cu)
struct ClipboardUnit *cu;
{
    struct IOClipReq *wior1, *wior2;

    wior1 = (struct IOClipReq *) cu->cu_CutPending.lh_Head;
    wior2 = (struct IOClipReq *) cu->cu_ReadPending.lh_Head;
    if (wior1->io_Message.mn_Node.ln_Succ)
	if (wior2->io_Message.mn_Node.ln_Succ) {
	    D(("R #%ld \\ W #%ld\n", wior2->io_ClipID, wior1->io_ClipID));
	    if (wior1->io_ClipID > wior2->io_ClipID)
		/* read list is next up */
		return(TRUE);
	    else
		return(FALSE);
	}
	else
	    return(FALSE);
    else
	return(TRUE);
}


doPost(cu, io, ior)
struct ClipboardUnit *cu;
struct IOClipReq *io;
struct IOClipReq *ior;
{
    cu->cu_SatisfyMsgPort = io->io_Data;
    cu->cu_SatisfyMsg.sm_Unit = cu->cu_CUP.cu_UnitNum;
    cu->cu_SatisfyMsg.sm_ClipID = io->io_ClipID;
    endQueuedIO(io, ior);
}


doWork(cu, ior)
/*	call Forbid()en	*/
struct ClipboardUnit *cu;
struct IOClipReq *ior;
{
    struct IOClipReq *wior;
    struct Hook *hook;
    BOOL workToDo;
    ULONG file;
    int i;
    char c;

    D(("doWork(0x%lx); ", cu));
    ior->io_Flags |= IOF_QUEUED;
    if (cu->cu_WorkLocked) {
	ior->io_Flags &= ~IOF_QUICK;
	D(("/ locked\n"));
	return(0);
    }
    cu->cu_WorkLocked = TRUE;
    workToDo = TRUE;
    do {
	D(("%ld ", cu->cu_ProcessingCommand));
	if (cu->cu_ProcessingCommand == CMD_INVALID) {
	    /* nothing's happening now */
	    if (nextPendingReading(cu)) {
		wior = (struct IOClipReq *) cu->cu_ReadPending.lh_Head;
		if (wior->io_Message.mn_Node.ln_Succ) {
		    cu->cu_ProcessingID = wior->io_ClipID;
		    cu->cu_ProcessingCommand = CMD_READ;
		}
		else
		    workToDo = FALSE;
	    }
	    else {
		wior = (struct IOClipReq *) cu->cu_CutPending.lh_Head;
		if (wior->io_Message.mn_Node.ln_Succ) {
		    cu->cu_ProcessingID = wior->io_ClipID;
		    if ((cu->cu_ProcessingCommand = wior->io_Command)
			    == CBD_POST) {
			doPost(cu, wior, ior);
		    }
		    /* clear the clip file */
		    if (cu->cu_DiskFile) {
		    D(("        about to Close(0x%lx);\n", cu->cu_DiskFile));
			Close(cu->cu_DiskFile);
			cu->cu_DiskFile = 0;
			DeleteFile(cu->cu_DiskFileName);
		    }
		    else {
			clearClip(cu);
		    }
		    cu->cu_CutValid = TRUE;
		}
		else
		    workToDo = FALSE;
	    }
	}
/******/
/******	POST
/******/
	if (cu->cu_ProcessingCommand == CBD_POST) {
	    /* post outstanding */
	    if (nextPendingReading(cu)) {
		wior = (struct IOClipReq *) cu->cu_ReadPending.lh_Head;
		if (wior->io_Message.mn_Node.ln_Succ) {
		    /* a read is waiting for the post */
		    PutMsg(cu->cu_SatisfyMsgPort, &cu->cu_SatisfyMsg);
		    cu->cu_ProcessingCommand = CMD_WRITE;
		}
		else
		    workToDo = FALSE;
	    }
	    else {
		wior = (struct IOClipReq *) cu->cu_CutPending.lh_Head;
		if (wior->io_Message.mn_Node.ln_Succ) {
		    cu->cu_ProcessingID = wior->io_ClipID;
		    if ((cu->cu_ProcessingCommand = wior->io_Command)
			    == CBD_POST) {
			doPost(cu, wior, ior);
		    }
		}
		else
		    workToDo = FALSE;
	    }
	    D(("POST hooks\n"));
	    ObtainSemaphore(&cu->cu_ChangeSemaphore);
	    hook = (struct Hook *) cu->cu_ChangeHooks.mlh_Head;
	    while (hook->h_MinNode.mln_Succ) {
		D(("  hook 0x%lx\n", hook));
		callHook(hook, cu, 0, CBD_POST, cu->cu_ProcessingID);
		hook = (struct Hook *) hook->h_MinNode.mln_Succ;
	    }
	    ReleaseSemaphore(&cu->cu_ChangeSemaphore);
	}
/******/
/******	READ
/******/
	if (cu->cu_ProcessingCommand == CMD_READ) {
	    wior = (struct IOClipReq *) cu->cu_ReadPending.lh_Head;
	    if (wior->io_Message.mn_Node.ln_Succ) {
		if (wior->io_Flags & IOF_PENDINGREAD) {
		    /* this is an initial read command */
		    cu->cu_CurrentReadCount++;
		    wior->io_Flags &= ~IOF_PENDINGREAD;
		}
		/* check for end of file (indicating the paste is complete) */
		if (wior->io_Offset < cu->cu_CurrentLength) {
		    /* not end of file */
		    /* check for a nil destination before actually reading */
		    if (wior->io_Data != 0) {
			/* real destination, perform the read */
			wior->io_Actual = readClip(cu, wior, wior->io_Data,
				wior->io_Length, wior->io_Offset);
			wior->io_Offset += wior->io_Actual;
		    }
		    else {
			if ((wior->io_Offset + wior->io_Length <
				cu->cu_CurrentLength) &&
				(wior->io_Offset + wior->io_Length >
				wior->io_Offset))
			    wior->io_Actual = wior->io_Length;
			else
			    wior->io_Actual =
				    cu->cu_CurrentLength - wior->io_Offset;
			wior->io_Offset += wior->io_Actual;
		    }
		}
		else {
		    /* end of file */
		    wior->io_Actual = 0;
		    wior->io_ClipID = -1;
		    cu->cu_CurrentReadCount--;
		}
		endQueuedIO(wior, ior);
		if (cu->cu_CurrentReadCount == 0) {
		    /* there are no more reads pending */
		    cu->cu_ProcessingCommand = CMD_INVALID;
		}
	    }
	    else
		workToDo = FALSE;
	}
/******/
/******	WRITE
/******/
	if (cu->cu_ProcessingCommand == CMD_WRITE) {
	    wior = (struct IOClipReq *) cu->cu_CutPending.lh_Head;
	    if (wior->io_Message.mn_Node.ln_Succ) {
		if (wior->io_Command == CMD_WRITE) {
		    /* this is a write command */
		    wior->io_Actual = writeClip(cu, wior,
			    wior->io_Data, wior->io_Length, wior->io_Offset);
		    wior->io_Offset += wior->io_Actual;
		    cu->cu_CurrentLength =
			    (cu->cu_CurrentLength > wior->io_Offset) ?
			    cu->cu_CurrentLength : wior->io_Offset;
		    endQueuedIO(wior, ior);
		}
		else
		    if (wior->io_Command == CMD_UPDATE) {
			/* this is an update command */
			cu->cu_ProcessingCommand = CMD_UPDATE;
		    }
		    else
			workToDo = FALSE;
	    }
	    else
		workToDo = FALSE;
	}
/******/
/******	UPDATE
/******/
	if (cu->cu_ProcessingCommand == CMD_UPDATE) {
	    wior = (struct IOClipReq *) cu->cu_CutPending.lh_Head;
	    if (cu->cu_CurrentLength >= 8) {
		if (readClip(cu, wior, &cu->cu_CurrentLength, 4, 4) == 4) {
		    cu->cu_CurrentLength += 8;
		}
		else cu->cu_CurrentLength = 0;
	    }
	    else cu->cu_CurrentLength = 0;
	    D(("    UPDATE cu_CurrentLength %ld\n", cu->cu_CurrentLength));
	    cu->cu_CurrentReadCount = 0;
	    endQueuedIO(wior, ior);
	    if (cu->cu_CurrentLength > 0) {
		D(("UPDATE hooks\n"));
		ObtainSemaphore(&cu->cu_ChangeSemaphore);
		hook = (struct Hook *) cu->cu_ChangeHooks.mlh_Head;
		while (hook->h_MinNode.mln_Succ) {
		    D(("  hook 0x%lx\n", hook));
		    callHook(hook, cu, 0, CMD_UPDATE, cu->cu_ProcessingID);
		    hook = (struct Hook *) hook->h_MinNode.mln_Succ;
		}
		ReleaseSemaphore(&cu->cu_ChangeSemaphore);
	    }
	    cu->cu_ProcessingCommand = CMD_INVALID;
	}
    }
	while (workToDo);
    if (ior->io_Flags & IOF_QUEUED)
	ior->io_Flags &= ~IOF_QUICK;
    cu->cu_WorkLocked = FALSE;
    D((".\n"));
}


insort(list, node)
struct List *list;
struct IOClipReq *node;
{
    struct IOClipReq *search;
    struct Node *loopNode;

    search = (struct IOClipReq *) list->lh_Head;
    loopNode = search->io_Message.mn_Node.ln_Succ;
    while (loopNode) {
	if (node->io_ClipID >= search->io_ClipID) {
	    /* still looking */
	    search = (struct IOClipReq *) loopNode;
	    loopNode = search->io_Message.mn_Node.ln_Succ;
	}
	else {
	    /* insert this one here */
	    loopNode = 0;
	}
    }
    /* insert before search */
    Insert(list, node, search->io_Message.mn_Node.ln_Pred);
}



/*********************************************************************
**********************************************************************
*
*	Device Commands
*
**********************************************************************
*********************************************************************/


CCInvalid(ior)
struct IOClipReq *ior;
{
    D(("CCInvalid(0x%lx);\n", ior));
    ior->io_Error = IOERR_NOCMD;
    endIO(ior);
}


CCClear(ior)
struct IOClipReq *ior;
{
    D(("CCClear(0x%lx);\n", ior));
    CCInvalid(ior);
}


CCStop(ior)
struct IOClipReq *ior;
{
    D(("CCStop(0x%lx);\n", ior));
    CCInvalid(ior);
}


CCStart(ior)
struct IOClipReq *ior;
{
    D(("CCStart(0x%lx);\n", ior));
    CCInvalid(ior);
}


CCFlush(ior)
struct IOClipReq *ior;
{
    D(("CCFlush(0x%lx);\n", ior));
    CCInvalid(ior);
}


/****i* clipboard.device/CMD_RESET ***********************************
*
*   NAME
*	CMD_RESET -- Reset the clipboard.
*
*   FUNCTION
*	CMD_RESET resets the clipboard device without destroying handles
*	to the open device.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set up
*	io_Device	preset by OpenDevice
*	io_Command	CMD_RESET
*	io_Flags	IOB_QUICK set if quick I/O is possible
*
*********************************************************************/

CCReset(ior)
struct IOClipReq *ior;
{
    D(("CCReset(0x%lx);\n", ior));
    CCStop(ior);
    CCFlush(ior);
    CCStart(ior);
}


/****** clipboard.device/CMD_READ ************************************
*
*   NAME
*	CMD_READ -- Read from a clip on the clipboard.
*
*   FUNCTION
*	The read function serves two purposes.
*
*	When io_Offset is within the clip, this acts as a normal read
*	request, and io_Data is filled with data from the clipboard.
*	The first read request should have a zero io_ClipID, which
*	will be filled with the ID assigned for this read.  Normal
*	sequential access from the beginning of the clip is achieved
*	by setting io_Offset to zero for the first read, then leaving
*	it untouched for subsequent reads.  If io_Data is null, then
*	io_Offset is incremented by io_Actual as if io_Length bytes
*	had been read: this is useful to skip to the end of file
*	by using a huge io_Length.
*
*	When io_Offset is beyond the end of the clip, this acts as a
*	signal to the clipboard device that the application is
*	through reading this clip.  Realize that while an application
*	is in the middle of reading a clip, any attempts to write new
*	data to the clipboard are held off.  This read past the end
*	of file indicates that those operations may now be initiated.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set up
*	io_Device	preset by OpenDevice
*	io_Unit		preset by OpenDevice
*	io_Command	CMD_READ
*	io_Length	number of bytes to put in data buffer
*	io_Data		pointer to buffer of data to fill, or null to
*			skip over data
*	io_Offset	byte offset of data to read
*	io_ClipID	zero if this is the initial read
*
*   RESULTS
*	io_Error	non-zero if an error occurred
*	io_Actual	filled with the actual number of bytes read
*	io_Data		(the buffer now has io_Actual bytes of data)
*	io_Offset	updated to next read position, which is
*			beyond EOF if io_Actual != io_Length
*	io_ClipID	the clip ID assigned to this read: do not
*			alter for subsequent reads
*
*********************************************************************/

CCRead(ior)
struct IOClipReq *ior;
{
    struct ClipboardUnit *cu;

    cu = (struct ClipboardUnit *) ior->io_Unit;

    D(("CCRead(0x%lx)...", ior));
    if (ior->io_ClipID == 0) {
	ior->io_ClipID = cu->cu_PendingID;
	ior->io_Flags |= IOF_PENDINGREAD;
	ior->io_Error = 0;
    }
    if (ior->io_ClipID < cu->cu_ProcessingID) {
	/* obsolete read */
	D(("    clip id %ld is obsolete\n", ior->io_ClipID));
	ior->io_Actual = 0;
	ior->io_Error = CBERR_OBSOLETEID;
	endIO(ior);
    }
    else {
	insort(&cu->cu_ReadPending, ior);
	doWork(cu, ior);
    }
}


/****** clipboard.device/CMD_WRITE ***********************************
*
*   NAME
*	CMD_WRITE -- Write to a clip on the clipboard.
*
*   FUNCTION
*	This command writes data to the clipboard.  This data can be
*	provided sequentially by clearing io_Offset for the initial
*	write, and using the incremented value unaltered for
*	subsequent writes.  If io_Offset is ever beyond the current
*	clip size, the clip is padded with zeros.
*
*	If this write is in response to a SatisfyMsg for a pending
*	post, then the io_ClipID returned by the CBD_POST command must
*	be used.  Otherwise, a new ID is obtained by clearing the
*	io_ClipID for the first write.  Subsequent writes must not
*	alter the io_ClipID.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set up
*	io_Device	preset by OpenDevice
*	io_Unit		preset by OpenDevice
*	io_Command	CMD_WRITE
*	io_Length	number of bytes from io_Data to write
*	io_Data		pointer to block of data to write
*	io_Offset	usually zero if this is the initial write
*	io_ClipID	zero if this is the initial write, ClipID of
*			the Post if this is to satisfy a post
*
*   RESULTS
*	io_Error	non-zero if an error occurred
*	io_Actual	filled with the actual number of bytes written
*	io_Offset	updated to next write position
*	io_ClipID	the clip ID assigned to this write: do not
*			alter for subsequent writes
*
*********************************************************************/

/****** clipboard.device/CMD_UPDATE **********************************
*
*   NAME
*	CMD_UPDATE -- Terminate the writing of a clip to the clipboard.
*
*   FUNCTION
*	Indicate to the clipboard that the previous write commands are
*	complete and can be used for any pending pastes (reads).  This
*	command cannot be issued while any of the write commands are
*	pending.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set up
*	io_Device	preset by OpenDevice
*	io_Unit		preset by OpenDevice
*	io_Command	CMD_UPDATE
*	io_ClipID	the ClipID of the write
*
*   RESULTS
*	io_Error	non-zero if an error occurred
*
*********************************************************************/

/****** clipboard.device/CBD_POST ************************************
*
*   NAME
*	CBD_POST -- Post availability of a clip to the clipboard.
*
*   FUNCTION
*	Indicate to the clipboard device that data is available for
*	use by accessors of the clipboard.  This is intended to be
*	used when a cut is large, in a private data format, and/or
*	changing frequently, and it thus makes sense to avoid
*	converting it to an IFF form and writing it to the clipboard
*	unless another application wants it.  The post provides a
*	message port to which the clipboard device will send a satisfy
*	message if the data is required.
*
*	If the satisfy message is received, the write associated with
*	the post must be performed.  The act of writing the clip
*	indicates that the message has been received: it may then be
*	re-used by the clipboard device, and so must actually be
*	removed from the satisfy message port so that the port is not
*	corrupted.
*
*	If the application wishes to determine if a post it has
*	performed is still the current clip, it should check the
*	post's io_ClipID with that returned by the CBD_CURRENTREADID
*	command.  If the current read io_ClipID is greater, the clip
*	is not still current.
*	
*	If an application has a pending post and wishes to determine
*	if it should satisfy it (e.g. before it exits), it should
*	check the post's io_ClipID with that returned by the
*	CBD_CURRENTWRITEID command.  If the current write io_ClipID is
*	greater, there is no need to satisfy the post.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set up
*	io_Device	preset by OpenDevice
*	io_Unit		preset by OpenDevice
*	io_Command	CBD_POST
*	io_Data		pointer to satisfy message port
*	io_ClipID	zero
*
*   RESULTS
*	io_Error	non-zero if an error occurred
*	io_ClipID	the clip ID assigned to this post, to be used
*			in the write command if this is satisfied
*
*
*********************************************************************/


CCCut(ior)
/* used as CCWrite(ior), CCUpdate(ior) and CCPost(ior) */
struct IOClipReq *ior;
{
    struct ClipboardUnit *cu;

    cu = (struct ClipboardUnit *) ior->io_Unit;

    D(("CCCut(0x%lx)...", ior));
    if (ior->io_ClipID == 0) {
	ior->io_Error = 0;
	ior->io_ClipID = ++cu->cu_PendingID;
	D(("    ClipID %ld assigned\n", ior->io_ClipID));
    }
    if (ior->io_ClipID < cu->cu_ProcessingID) {
	/* obsolete write */
	D(("    clip id %ld is obsolete\n", ior->io_ClipID));
	ior->io_Actual = 0;
	ior->io_Error = CBERR_OBSOLETEID;
	endIO(ior);
    }
    else {
	insort(&cu->cu_CutPending, ior);
	doWork(cu, ior);
    }
}


/****** clipboard.device/CBD_CURRENTREADID ***************************
*
*   NAME
*	CBD_CURRENTREADID - Determine the current read identifier.
*
*   FUNCTION
*	CBD_CURRENTREADID fills the io_ClipID with a clip identifier that
*	can be compared with that of a post command: if greater than
*	the post identifier then the post data held privately by an
*	application is not valid for its own pasting.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set up
*	io_Device	preset by OpenDevice
*	io_Unit		preset by OpenDevice
*	io_Command	CBD_CURRENTREADID
*
*   RESULTS
*	io_ClipID	the ClipID of the current write is set
*
*********************************************************************/

CCCurrentReadID(ior)
struct IOClipReq *ior;
{
    struct ClipboardUnit *cu;

    cu = (struct ClipboardUnit *) ior->io_Unit;

    ior->io_ClipID = cu->cu_PendingID;
    endIO(ior);
}


/****** clipboard.device/CBD_CURRENTWRITEID **************************
*
*   NAME
*	CBD_CURRENTWRITEID -- Determine the current write identifier.
*
*   FUNCTION
*	CBD_CURRENTWRITEID fills the io_ClipID with a clip identifier that
*	can be compared with that of a post command: if greater than
*	the post identifier then the post is obsolete and need never
*	be satisfied.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set up
*	io_Device	preset by OpenDevice
*	io_Unit		preset by OpenDevice
*	io_Command	CBD_CURRENTWRITEID
*
*   RESULTS
*	io_ClipID	the ClipID of the current write is set
*
*********************************************************************/

CCCurrentWriteID(ior)
struct IOClipReq *ior;
{
    struct ClipboardUnit *cu;

    cu = (struct ClipboardUnit *) ior->io_Unit;

    ior->io_ClipID = cu->cu_ProcessingID;
    endIO(ior);
}


/****** clipboard.device/CBD_CHANGEHOOK ******************************
*
*   NAME
*	CBD_CHANGEHOOK -- Add or remove a clip change hook
*
*   FUNCTION
*	CBD_CHANGEHOOK allows specification of a hook to be called
*	when the data on the clipboard has changed.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set up
*	io_Device	preset by OpenDevice
*	io_Unit		preset by OpenDevice
*	io_Command	CBD_CHANGEHOOK
*	io_Length -     0 to remove, 1 to install this hook
*	io_Data -       struct Hook *, the clip change hook
*
*   HOOK ENVIRONMENT
*       hook message - a ClipHookMsg, as defined in devices/clipboard.h
*           chm_Type - 0, indicating that the message has the
*               following fields:
*           chm_ClipID - the clip ID of the clip triggering the change
*       hook object - io_Unit
*
*********************************************************************/

CCChangeHook(ior)
struct IOClipReq *ior;
{
    struct ClipboardUnit *cu;

    cu = (struct ClipboardUnit *) ior->io_Unit;
    D(("CCChangeHook(0x%lx)\n", ior));
    ObtainSemaphore(&cu->cu_ChangeSemaphore);
    if (ior->io_Length == 1) {
	D(("install hook 0x%lx\n", ior->io_Data));
	AddTail(&cu->cu_ChangeHooks, ior->io_Data);
    }
    else if (ior->io_Length == 0) {
	D(("remove hook 0x%lx\n", ior->io_Data));
	Remove(ior->io_Data);
    }
    else {
	ior->io_Error = IOERR_BADLENGTH;
    }
    ReleaseSemaphore(&cu->cu_ChangeSemaphore);

    endIO(ior);
}


/*********************************************************************
**********************************************************************
*
*	Device Functions
*
**********************************************************************
*********************************************************************/


/****i* clipboard.device/Open ****************************************
*
*   NAME
*	Open - a request to open the clipboard device
*
*   SYNOPSIS
*	OpenDevice("clipboard.device", unit, iORequest, 0), sysBase
*
*   FUNCTION
*	The open routine grants access to a device.  There are two
*	fields in the iORequest block that will be filled in:
*	io_Device and io_Unit.
*
*	A successful OpenDevice call must be matched by a CloseDevice
*	call when access to the device is no longer needed.
*
*   RESULTS
*	If the open was unsuccessful, OpenDevice returns a non-zero
*	result and the iORequest is not valid.
*
*********************************************************************/

#define	po_Task		mn_ReplyPort
#define	po_SigBit	mn_Node.ln_Pri

COpen(unitNum, ior, cd)
ULONG unitNum;
struct IOClipReq *ior;
struct Library *cd;
{
    struct ClipboardUnit *cu;
    struct Node *loopNode;
    struct Node *insert;
    struct Message *po;
    ULONG  diskFile;
    struct ClipBlock *block;
    int    length;

    D(("COpen(0x%lx, 0x%lx);\n", unitNum, ior));
    ObtainSemaphore(&cd_SS);
    cd->lib_Flags &= ~LIBF_DELEXP;
    cu = (struct IOClipReq *) cd_Units.lh_Head;
    loopNode = cu->cu_CUP.cu_Node.ln_Succ;
    insert = cu;
    while (loopNode) {
	if (cu->cu_CUP.cu_UnitNum < unitNum) {
	    /* still looking */
	    cu = (struct IOClipReq *) loopNode;
	    loopNode = cu->cu_CUP.cu_Node.ln_Succ;
	    insert = cu;
	}
	else {
	    if (cu->cu_CUP.cu_UnitNum == unitNum)
		/* found the unit */
		insert = 0;
	    /* indicate to stop looking */
	    loopNode = 0;
	}
    }
    if (insert) {
	D(("    insert new unit at 0x%lx.\n", insert->ln_Pred));
	/* insert new unit */
	if ((cu = (struct ClipboardUnit *)
		AllocMem(sizeof(*cu), MEMF_PUBLIC+MEMF_CLEAR)) == 0) {
	    D(("    error: no memory for unit\n"));
	    ior->io_Error = IOERR_OPENFAIL;
	    ReleaseSemaphore(&cd_SS);
	    return(-1);
	}
	cu->cu_CUP.cu_Node.ln_Name = "clipboard.unit";
	cu->cu_CUP.cu_UnitNum = unitNum;
	cu->cu_OpenCnt = 1;
	NewList(&cu->cu_PendingOpens.mp_MsgList);
	cu->cu_PendingID = 1;
	cu->cu_ProcessingID = 1;
	cu->cu_OpenFlags = OPEN_PROGRESSING;
	NewList(&cu->cu_BlockHead);
	cu->cu_CutValid = FALSE;	/* just a guess */
	NewList(&cu->cu_ReadPending);
	NewList(&cu->cu_CutPending);
	InitSemaphore(&cu->cu_ChangeSemaphore);
	NewList(&cu->cu_ChangeHooks);

	cd->lib_OpenCnt++;
	ior->io_Unit = cu;
	Insert(&cd_Units, cu, insert->ln_Pred);

	/* try to copy disk clip to ram */
	if (diskFile = openDiskClip(cu, MODE_OLDFILE)) {
	    D(("        0x%06lx\n", diskFile));
	    if (Seek(diskFile, 4, OFFSET_BEGINING) != -1) {
		if (Read(diskFile, &length, 4) == 4) {
		    cu->cu_CutValid = TRUE;		/* just a guess */
		    length += 8;
		    cu->cu_CurrentLength = length;
		    if (Seek(diskFile, 0, OFFSET_BEGINING) == -1)
			    cu->cu_CutValid = FALSE;
		    while (cu->cu_CutValid && (length > BLOCKSIZE)) {
			if ((block = (struct ClipBlock *)newTailBlock(cu)) == 0)
				cu->cu_CutValid = FALSE;
			else {
			    if (Read(diskFile, block->cb_Data, BLOCKSIZE) != 
				    BLOCKSIZE) cu->cu_CutValid = FALSE;
			}
			length -= BLOCKSIZE;
		    }
		    if (cu->cu_CutValid && (length > 0)) {
			if ((block = (struct ClipBlock *)newTailBlock(cu)) == 0)
				cu->cu_CutValid = FALSE;
			else {
			    if (Read(diskFile, block->cb_Data, length) != 
				    length) cu->cu_CutValid = FALSE;
			}
		    }
		    if (!cu->cu_CutValid) {
			clearClip(cu);
			if (Seek(diskFile, cu->cu_CurrentLength,
				OFFSET_BEGINING) == 0) {
			    /* make the clip a disk clip */
			    cu->cu_DiskFile = diskFile;
			    diskFile = 0;
			    cu->cu_CutValid = 0;
			}
		    }
		}
	    }
	    D(("        Close(0x%06lx);\n", diskFile));
	    if (diskFile)
		Close(diskFile);
	}
	if (!(cu->cu_CutValid)) cu->cu_CurrentLength = 0;
	D(("    cu_CurrentLength %ld\n", cu->cu_CurrentLength));
	/* Forbid should be re-instated by here */
	/******/
	cu->cu_OpenFlags = OPEN_OK;
	while (po = (struct Message *) GetMsg(&cu->cu_PendingOpens)) {
	    Signal(po->po_Task, 1<<po->po_SigBit);
	}
    }
    else {
	cu->cu_OpenCnt++;
	cd->lib_OpenCnt++;
	ior->io_Unit = cu;
	if (cu->cu_OpenFlags == OPEN_PROGRESSING) {
	    /* wait for someone else to create the unit file */
	    {
		struct Message wpo;
		wpo.po_Task = (struct MsgPort *) FindTask(0);
		wpo.po_SigBit = AllocSignal(-1);
		PutMsg(&cu->cu_PendingOpens, &wpo);
		Wait(1<<wpo.po_SigBit);
		FreeSignal(wpo.po_SigBit);
	    }
	}
    }
    D(("    unit # %ld at 0x%lx\n", cu->cu_CUP.cu_UnitNum, cu));
    ReleaseSemaphore(&cd_SS);
    return(ior->io_Error);
}


/****i* clipboard.device/Close ***************************************
*
*   NAME
*	Close - terminate access to the clipboard device
*
*   SYNOPSIS
*	CloseDevice(iORequest), sysBase
*
*   FUNCTION
*	The close routine notifies the clipboard device that the
*	iORequest will no longer be used.
*
*********************************************************************/

CClose(ior, cd)
struct IOClipReq *ior;
struct Library *cd;
{
    struct ClipboardUnit *cu;
    struct ClipBlock *block;
    int    length;

    cu = (struct ClipboardUnit *) ior->io_Unit;
    ObtainSemaphore(&cd_SS);
    D(("CClose(0x%lx);\n", ior));
    if (--(cu->cu_OpenCnt) == 0) {
	/* last accessor to this unit, shut it down */
	if ((cu->cu_CutValid) && (!(cu->cu_DiskFile)) &&
		(cu->cu_ProcessingID > 1) &&
		(cu->cu_DiskFile = openDiskClip(cu, MODE_NEWFILE))) {
	    /* write out the current cut to disk */
	    length = cu->cu_CurrentLength;
	    block = (struct ClipBlock *)
		    cu->cu_BlockHead.lh_Head;
	    while (cu->cu_CutValid && (length > 0)) {
		if (block->cb_Node.ln_Succ) {
		    if (length > BLOCKSIZE) {
			if (Write(cu->cu_DiskFile, block->cb_Data,
				BLOCKSIZE) != BLOCKSIZE)
				cu->cu_CutValid = FALSE;
			length -= BLOCKSIZE;
		    }
		    else {
			if (Write(cu->cu_DiskFile, block->cb_Data,
				length) != length)
				cu->cu_CutValid = FALSE;
			length = 0;
		    }
		    block = block->cb_Node.ln_Succ;
		}
		else cu->cu_CutValid = FALSE;
	    }
	}
	if (cu->cu_DiskFile) {
	    D(("        about to Close(0x%lx);\n", cu->cu_DiskFile));
	    Close(cu->cu_DiskFile);
	    if ((!cu->cu_CutValid) || (cu->cu_CurrentLength == 0)) {
		DeleteFile(cu->cu_DiskFileName);
	    }
	}
	clearClip(cu);
	Remove(cu);
	FreeMem(cu, sizeof(*cu));
    }

    /* back in Forbid() */

    ior->io_Unit = ior->io_Device = 0;
    --cd->lib_OpenCnt;

    /* Release won't break Forbid() */

    ReleaseSemaphore(&cd_SS);
}


/****i* clipboard.device/Expunge *************************************
*
*   NAME
*	Expunge - indicate a desire to remove the clipboard device
*
*   SYNOPSIS
*	<Expunge is not generally called by application programs>
*
*   FUNCTION
*	The Expunge routine is called when the system needs the memory
*	used by the clipboard device, and the clipboard device has no
*	open units.  The clipboard device is removed from memory until
*	next needed (i.e. until the next
*	OpenDevice("clipboard.device", ...);
*
*********************************************************************/

CExpunge(cd)
struct Library *cd;
{
    if (cd->lib_OpenCnt == 0) {
	CloseLibrary(DOSBase);
	Remove(cd);
	freeCD(cd);
	return(cd_Segment);
    }
    else {
	cd->lib_Flags |= LIBF_DELEXP;
	return(0);
    }
}


/****i* clipboard.device/BeginIO *************************************
*
*   NAME
*	BeginIO - initiate clipboard device IO
*
*   SYNOPSIS
*	SendIO(iORequest), sysBase
*	DoIO(iORequest), sysBase
*
*   FUNCTION
*	The BeginIO is the workhorse device function used to initiate
*	device commands.  It can be called directly or via the exec
*	library functions SendIO and DoIO.
*
*********************************************************************/
int (*commandVector[])() = {
    CCInvalid, CCReset, CCRead, /* CCWrite: */ CCCut, /* CCUpdate */ CCCut,
    CCClear, CCStop, CCStart, CCFlush,
    /* CCPost: */ CCCut, CCCurrentReadID, CCCurrentWriteID, CCChangeHook
};

CBeginIO(ior)
struct IOClipReq *ior;
{
    Forbid();
    ior->io_Message.mn_Node.ln_Type = NT_MESSAGE;
    ior->io_Flags &= ~(IOF_DONE | IOF_QUEUED | IOF_PENDINGREAD);
    if (ior->io_Command >= (sizeof(commandVector)/sizeof(commandVector[0]))) {
	CCInvalid(ior);
    }
    else {
	(*(commandVector[ior->io_Command]))(ior);
    }
    Permit();
}

CAbortIO(ior)
struct IOClipReq *ior;
{
    Forbid();
    if ((!(ior->io_Flags & IOF_DONE)) && (!(ior->io_Flags & IOF_ACTIVE))) {
	ior->io_Error = IOERR_ABORTED;
	if (ior->io_Flags & IOF_QUEUED) 
	    Remove(ior);
	endIO(ior);
    }
    Permit();
}
