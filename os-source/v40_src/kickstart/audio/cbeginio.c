/*********************************************************************
*                                                                    *
*   Copyright 1986, Commodore-Amiga Inc.   All rights reserved.      *
*   No part of this program may be reproduced, transmitted,          *
*   transcribed, stored in retrieval system, or translated into      *
*   any language or computer language, in any form or by any         *
*   means, electronic, mechanical, magnetic, optical, chemical,      *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*                                                                    *
**********************************************************************
*
* $Id: cbeginio.c,v 37.8 91/04/26 08:58:59 mks Exp $
*
* $Locker:  $
*
* $Log:	cbeginio.c,v $
* Revision 37.8  91/04/26  08:58:59  mks
* Fixed audio.device bug dealing with SyncCycle...  I had
* broken SyncCycle for the FINISH command when I fixed
* the click/pop on fast machines.
* 
* Revision 37.7  91/03/13  19:06:19  mks
* Added kludge for AmigaBasic which was calling BeginIO() with audio channel
* interrupts turned off.  This makes it impossible to avoid the interrupt
* timing problems without hanging the system.  Thus, if the audio channel
* interrupts are off, we let the interrupt timing glitch happen.  It's
* AmigaBasic's fault that they glitch happens and they can fix it.
* (Just do not turn off audio interrupts when calling BeginIO()
*
* Revision 37.6  91/03/01  13:18:24  mks
* Ok, so we now know the whole story about this hanging audio device.
*
* When a new sound or the last queued sound is just about to be started
* the next field is pointing to a sound that may not be playing yet.
* At that point, the list is assumed empty.
*
* The trick is to make sure that if the next field is not
* empty that the playing field is also not empty.  Since
* only the interrupt code can handle this, we make sure by
* doing a small busy wait here until we are not in the
* transition state.
*
* Revision 37.5  91/02/27  20:41:02  mks
* Turns out that the Enable() fix was just a kludge and may need even
* more work.  Guess it is just not ready yet.  In any case, it is much
* better than the old cache clearing method.
*
* Revision 37.4  91/02/27  19:33:16  mks
* Well, first cut of the non-CPU cache flush fix to the hanging
* Paula bug...
*
* Revision 37.3  91/02/08  10:37:03  mks
* Fixed FINISH and ABORT "clicks and pops" problems  (Same as FLUSH)
*
* Revision 37.2  91/02/03  11:02:46  mks
* Temp checkin with kludge-fix to audio.device stall problem.  Will
* have to look at it later.
*
* Revision 37.1  91/01/11  18:29:41  mks
* Added fix in CMD_FLUSH to make sure it flushes completely.
* This fixes problems with high-speed machines.
*
* Revision 36.2  90/08/29  17:50:06  mks
* Changed revision to 36...  Change ownership to MKS...
* Changed $Header to $Id
*
* Revision 33.2  89/02/27  12:16:24  kodiak
* move set of NT_MESSAGE to beginning of CADBeginIO to avoid race condition
*
* Revision 33.1  86/06/09  23:54:10  sam
* now always resets channels when they are allocated
*
* Revision 32.1  86/01/14  21:21:56  sam
* revision set to 32
*
* Revision 1.1  86/01/14  20:30:06  sam
* Initial revision
*
*
*********************************************************************/

#include "exec/types.h"
#include "exec/lists.h"
#include "exec/nodes.h"
#include "exec/ports.h"
#include "exec/io.h"
#include "exec/libraries.h"
#include "exec/interrupts.h"
#include "exec/memory.h"
#include "exec/errors.h"
#include "exec/execbase.h"
#include "hardware/custom.h"
#include "hardware/intbits.h"
#include "hardware/dmabits.h"
#include "hardware/adkbits.h"
#include "audio.h"
#include "device.h"

#define SYNCENABLE	{ \
			    UWORD intFlag = chan->au_IntFlag; \
			    if (!(intenar & intFlag)) { \
				intreq = intFlag; \
				intena = INTF_SETCLR | intFlag; \
			    } \
			}

#define ALLOC_FAILED	-1
#define LOCKED_CHANNEL	-2

extern UWORD adkcon, dmacon, dmaconr, intena, intenar, intreq, intreqr;
extern int Restart(), NewPlay(), Cycles(), PerVol(), PaulaDelay();

UBYTE detach[] = {
    ADKF_USE0V1 | ADKF_USE0P1,
    ADKF_USE1V2 | ADKF_USE1P2 | ADKF_USE0V1 | ADKF_USE0P1,
    ADKF_USE2V3 | ADKF_USE2P3 | ADKF_USE1V2 | ADKF_USE1P2,
    ADKF_USE3VN | ADKF_USE3PN | ADKF_USE2V3 | ADKF_USE2P3
};

/* abort subroutines */

/* must be Disabled */
queueAbortIO(ioa, ad)
struct IOAudio *ioa;
struct AudioDevice *ad;
{
    ioa->ioa_Request.io_Error = IOERR_ABORTED;
    AddTail(&ad->ad_ReplyQueue, ioa);
}

/* must be Disabled */
abortBuffer(chan)
struct AudioUnit *chan;
{
    if (!chan->au_Next || chan->au_Flags & ADUNITF_STOPPED)
	CStopWrite(chan);
    else {
	chan->au_IntNextCode = chan->au_IntCode;
	chan->au_IntCode = Restart;
	SYNCENABLE;
	dmacon = chan->au_ChanFlag;
    }


	/*
	 * The following is used to make the sound go
	 * away faster when the flush happens
	 */
    *(ULONG *)&(chan->au_Regs->ac_per) = 0x10000;
}

/* must be Disabled */
abortNext(chan, ad)
struct AudioUnit *chan;
struct AudioDevice *ad;
{
    struct IOAudio *next = chan->au_Next;
    if (next) {
	CLastCycle(chan);
	if (next != chan->au_Playing)
	    queueAbortIO(next, ad);
    }
}

abortFromList(ioa, list, ad)
struct IOAudio *ioa;
struct List *list;
struct AudioDevice *ad;
{
    Disable();
    {
	struct IOAudio *ioCheck;
	ioCheck = (struct IOAudio *)list->lh_Head;
	while (ioCheck != ioa)
	    if (!(ioCheck = (struct IOAudio *)
		    (ioCheck->ioa_Request.io_Message.mn_Node.ln_Succ))) {
		Enable();
		return(FALSE);
	    }
    }
    if (ioa->ioa_Request.io_Error != ADIOERR_NOABORT) {
	Remove(ioa);
	AddTail(&ad->ad_ReplyQueue, ioa);
    }
    Enable();
    ioa->ioa_Request.io_Error = IOERR_ABORTED;
    return(TRUE);
}

queueAbortList(list, ad)
struct List *list;
struct AudioDevice *ad;
{
    Disable();
    {
	struct IOAudio *ioa, *ioaSucc;
	for (ioa = (struct IOAudio *)(list->lh_Head);
		ioaSucc = (struct IOAudio *)
		(ioa->ioa_Request.io_Message.mn_Node.ln_Succ);
		ioa = ioaSucc)
	    ioa->ioa_Request.io_Error = IOERR_ABORTED;
    }
    Enable();
    Disable();
    if (list->lh_Head->ln_Succ)
	CQueueReplyList(list, ad);
    Enable();
}

flush(chan, ad)
struct AudioUnit *chan;
struct AudioDevice *ad;
{
    Disable();
    abortNext(chan, ad);
    abortBuffer(chan);
    queueAbortList(&chan->au_WriteQueue, ad);
    Enable();
    chan->au_Flags &= ~ADUNITF_WAITLIST;
    queueAbortList(&chan->au_WaitList, ad);
}

reset(chan, ad)
struct AudioUnit *chan;
struct AudioDevice *ad;
{
    SetIntVector(chan->au_IntBit, &chan->au_AudioInterrupt);
    flush(chan, ad);
    *(ULONG *)&(chan->au_Regs->ac_per) = 0x10000;
    adkcon = detach[chan->au_ChanBit];
    chan->au_Flags &= ~ADUNITF_STOPPED;
}

newKey(ioa, ad)
struct IOAudio *ioa;
struct AudioDevice *ad;
{
    if (!ioa->ioa_AllocKey) {
	if (++ad->ad_AllocMaster <= 0)
	    ad->ad_AllocMaster = 1;
	ioa->ioa_AllocKey = ad->ad_AllocMaster;
    }
}

alloc(ioa, ad)
struct IOAudio *ioa;
struct AudioDevice *ad;
{
    WORD mapNum = 0;
    if (ioa->ioa_Length) {
	if (ad->ad_Flags & ADDEVF_UPDATEALLOC) {
	    WORD *levelPrec, lowest;
	    UBYTE curMap, *levelMap, lowMap;
	    for (curMap = 0xF, lowest = ADALLOC_MINPREC - 1,
		    levelPrec = ad->ad_LevelPrec, levelMap = ad->ad_LevelMap;
		    curMap; *(levelMap++) = curMap &= ~lowMap) {
		UBYTE scanFlag;
		WORD *prec;
		for (scanFlag = 1, lowMap = 0, prec = ad->ad_Prec;
			curMap >= scanFlag; scanFlag <<= 1, ++prec)
		    if (curMap & scanFlag)
			if (*prec == lowest)
			    lowMap |= scanFlag;
			else
			    if (*prec <= lowest) {
				lowMap = scanFlag;
				lowest = *prec;
			    }
		if (lowest > ADALLOC_MINPREC - 1)
		    *(levelPrec++) = lowest;
		lowest = ADALLOC_MAXPREC;
	    }
	    *levelPrec = ADALLOC_MAXPREC;
	    ad->ad_Flags &= ~ADDEVF_UPDATEALLOC;
	} {
	    UWORD worstNum;
	    UBYTE *maxLevelMap;
	    ioa->ioa_Request.io_Unit = 0;
	    {
		WORD *levelPrec;
		for (maxLevelMap = ad->ad_LevelMap, levelPrec =
			ad->ad_LevelPrec; ioa->ioa_Request.
			io_Message.mn_Node.ln_Pri > *levelPrec;
			 ++maxLevelMap, ++levelPrec);
	    } {
		UBYTE *map;
		for (worstNum = 0, map = ioa->ioa_Data;
			worstNum < ioa->ioa_Length; ++map, ++worstNum)
		    if (!(*map & *maxLevelMap)) {
			UBYTE *levelMap;
			for (levelMap = ad->ad_LevelMap;
				levelMap < maxLevelMap; ++levelMap)
			    for (mapNum = 0, map = ioa->ioa_Data; mapNum <
				    ioa->ioa_Length; ++map, ++mapNum)
				if (!(*map & *levelMap))
				    goto checkLock;
			mapNum = worstNum;
			goto checkLock;
		    }
	    }
	}
	return(ALLOC_FAILED);
	{
	    UBYTE allocMap, lockMap;
	checkLock:
	    allocMap = (UBYTE)(ioa->ioa_Data[mapNum]);
	    if (lockMap = allocMap & ad->ad_LockMap) {
		Disable();
		{
		    struct IOAudio *lc, *lcSucc;
		    for (lc = (struct IOAudio *)(ad->ad_LockList.lh_Head);
			    lcSucc = (struct IOAudio *)(lc->ioa_Request.
			    io_Message.mn_Node.ln_Succ); lc = lcSucc)
			if ((ULONG)(lc->ioa_Request.io_Unit) & lockMap) {
			    Remove(lc);
			    lc->ioa_Request.io_Error = ADIOERR_CHANNELSTOLEN;
			    AddTail(&ad->ad_ReplyQueue, lc);
			}
		}
		Enable();
		return(LOCKED_CHANNEL);
	    }
	    ioa->ioa_Request.io_Unit = (struct Unit *)allocMap;
	    newKey(ioa, ad);
	    {
		WORD *alloc;
		WORD *prec;
		struct AudioUnit *chan;
		for (chan = ad->ad_Unit, prec = ad->ad_Prec,
			alloc = ad->ad_AllocKey; allocMap;
			++chan, ++alloc, ++prec, allocMap >>= 1) {
		    if (allocMap & 1) {
			reset(chan, ad);
			*prec = ioa->ioa_Request.io_Message.mn_Node.ln_Pri;
			*alloc = ioa->ioa_AllocKey;
			ad->ad_Flags |= ADDEVF_UPDATEALLOC;
		    }
		}
	    }
	}
    } else
	newKey(ioa, ad);
    return(mapNum);
}

/* main functions */

CADBeginIO(ioa, ad)
struct IOAudio *ioa;
struct AudioDevice *ad;
{
    ioa->ioa_Request.io_Message.mn_Node.ln_Type = NT_MESSAGE;
    ioa->ioa_Request.io_Error = 0;
    Forbid();
    if (ioa->ioa_Request.io_Command == ADCMD_ALLOCATE) {
        {
	    BYTE allocNum;
	    if ((allocNum = alloc(ioa, ad)) == ALLOC_FAILED &&
		    ioa->ioa_Request.io_Flags & ADIOF_NOWAIT) {
		ioa->ioa_Request.io_Error = ADIOERR_ALLOCFAILED;
		goto replyExit;
	    }
	    if (allocNum >= 0)
		goto replyExit;
	}
	Disable();
	Enqueue(&ad->ad_BackgroundList, ioa);
	Enable();
	goto noReplyExit;
    } else {
	ULONG unit;
	UBYTE stopMap = 0;
	unit = (LONG)(ioa->ioa_Request.io_Unit);
	while (unit) {
	    struct AudioUnit *chan;
	    chan = ad->ad_Chan[unit];
	    unit &= ~chan->au_ChanFlag;
	    if (*chan->au_AllocKey != ioa->ioa_AllocKey) {
		ioa->ioa_Request.io_Error = ADIOERR_NOALLOCATION;
		ioa->ioa_Request.io_Unit = (struct Unit *)
			((LONG)(ioa->ioa_Request.io_Unit) & ~chan->au_ChanFlag);
		switch(ioa->ioa_Request.io_Command) {
		case CMD_READ:
		case CMD_WRITE:
		case ADCMD_LOCK:
		case ADCMD_WAITCYCLE:
		    ioa->ioa_Request.io_Unit = 0;
		    goto replyExit;
		case CMD_START:
		    if (!unit)
			goto lastStart;
		case ADCMD_FREE:
		    if (!unit)
			goto lastFree;
		case ADCMD_SETPREC:
		    if (!unit)
			goto realloc;
		}
		continue;
	    }

	    switch(ioa->ioa_Request.io_Command) {
	    case CMD_RESET:
		reset(chan, ad);
		break;
	    case CMD_READ:
                ioa->ioa_Request.io_Unit = (struct Unit *)(chan->au_ChanFlag);
		ioa->ioa_Data = (UBYTE *)(chan->au_Playing);
		goto replyExit;
	    case CMD_WRITE:
                ioa->ioa_Request.io_Unit = (struct Unit *)(chan->au_ChanFlag);

		/*
		 * Now, for one more kludge!
		 * It seems that AmigaBasic disables audio
		 * interrupts when calling this routine
		 * and we can not have this...  So, we
		 * check if the interrupts are on
		 */
		if (intenar & chan->au_IntFlag)
		{
			/*
			 * This is a new problem...  (and fix)
			 *
			 * When a new sound or the last queued
			 * sound is just about to be started
			 * the next field is pointing to
			 * a sound that may not be playing yet.
			 * At that point, the list is assumed
			 * empty.
			 *
			 * The trick is to make sure
			 * that if the next field is not
			 * empty that the playing field
			 * is also not empty.  Since
			 * only the interrupt code can
			 * handle this, we make sure by
			 * doing a small busy wait here
			 * until we are not in the
			 * transition state.  (Max: 1 scan line)
			 */
			while ((chan->au_Next) && !(chan->au_Playing));
		}

		Disable();
                if (chan->au_Next) AddTail(&chan->au_WriteQueue, ioa);
                else
                {
		    struct AudChannel *regs = chan->au_Regs;
		    regs->ac_len = ioa->ioa_Length >> 1;
		    regs->ac_ptr = (UWORD *)(ioa->ioa_Data);
                    chan->au_Cycles = ioa->ioa_Cycles;
                    chan->au_Next = ioa;
                    chan->au_IntCode = NewPlay;

		    /*
		     * Check if we are playing something.  If we
		     * are not, we need to "prime the pump"
		     * and set up the audio DMA.
		     */
		    if (!(chan->au_Playing) && !(chan->au_Flags & ADUNITF_STOPPED))
		    {
			chan->au_IntNextCode = NewPlay;
			chan->au_IntCode = Restart;

			/*
			 * This is a kludge to make sure that any old samples
			 * that may have been written out will get flushed
			 * as fast as possible.
			 *
			 * Note that the period is set to 8 in order
			 * to make this the same speed as will be on the
			 * new Paula chip.
			 */
			if (ioa->ioa_Request.io_Flags & ADIOF_PERVOL)
			{
			    *(ULONG *)&(regs->ac_per) = 0x80000;
			}

			SYNCENABLE;

			/*
			 * Now, cause the interrupt to happen by writing
			 * to the audio data register.  To make sure this
			 * does not cause a click or pop, write the same
			 * value for both bytes of the sample as the
			 * first byte of the sample.
			 */
			{
			UBYTE data = *(ioa->ioa_Data);

			    regs->ac_dat = data | (data << 8);
			}
		    }
		}
		Enable();
		goto noReplyExit;
	    case CMD_UPDATE:
	    case CMD_CLEAR:
		break;
	    case CMD_STOP:
		chan->au_Flags |= ADUNITF_STOPPED;
		intena = chan->au_IntFlag;
		dmacon = chan->au_ChanFlag;
		break;
	    case CMD_START:
		chan->au_Flags &= ~ADUNITF_STOPPED;
		if (chan->au_Playing || chan->au_Next) {
		    stopMap |= chan->au_ChanFlag;
		    SYNCENABLE;
		}
		if (!unit)
		lastStart:
		    dmacon = DMAF_SETCLR | stopMap;
		break;
	    case CMD_FLUSH:
		flush(chan, ad);
		break;
	    case ADCMD_FINISH:
		Disable();
		abortNext(chan, ad);
		if (ioa->ioa_Request.io_Flags & ADIOF_SYNCCYCLE)
		    SYNCENABLE
		else
		    abortBuffer(chan);
		Enable();
		break;
	    case ADCMD_PERVOL:
		if (ioa->ioa_Request.io_Flags & ADIOF_SYNCCYCLE &&
			chan->au_IntCode == Cycles) {
		    chan->au_IntCode = PerVol;
		    chan->au_PerVol = *(LONG *)&ioa->ioa_Period;
		    SYNCENABLE;
		} else
		    *(LONG *)&chan->au_Regs->ac_per = *(LONG *)&ioa->ioa_Period;
		break;
	    case ADCMD_FREE:
		reset(chan, ad);
		*chan->au_AllocKey = -1;
		*chan->au_Prec = ADALLOC_MINPREC - 1;
		if (!unit) {
		lastFree:
		    if (ad->ad_LockMap & (ULONG)(ioa->ioa_Request.io_Unit)) {
			ad->ad_LockMap &= ~(ULONG)(ioa->ioa_Request.io_Unit);
			Disable();
			{
			    struct IOAudio *lc, *lcSucc;
			    for (lc = (struct IOAudio *)(ad->ad_LockList.
				    lh_Head); lcSucc =
				    (struct IOAudio *)(lc->ioa_Request.
				    io_Message.mn_Node.ln_Succ); lc = lcSucc)
				if (!(lc->ioa_Request.io_Unit = (struct Unit *)
					((ULONG)(lc->ioa_Request.io_Unit) &
					~(ULONG)(ioa->ioa_Request.io_Unit)))) {
				    Remove(lc);
				    AddTail(&ad->ad_ReplyQueue, lc);
				}
			}
			Enable();
		    }
		    ioa->ioa_Request.io_Unit = 0;
		    goto realloc;
		}
		break;
	    case ADCMD_SETPREC:
		*chan->au_Prec = ioa->ioa_Request.io_Message.mn_Node.ln_Pri;
		if (unit)
		    break;
	    realloc:
		ad->ad_Flags |= ADDEVF_UPDATEALLOC;
		{
		    struct IOAudio *bg, *bgSucc;
		    BYTE allocNum;
		    Disable();
		    for (bg = (struct IOAudio *)ad->ad_BackgroundList.lh_Head;
		    	    bg->ioa_Request.io_Message.mn_Node.ln_Succ;
			    bg = bgSucc) {
			bg->ioa_Request.io_Error = ADIOERR_NOABORT;
			Enable();
			allocNum = alloc(bg, ad);
			Disable();
			bgSucc = (struct IOAudio *)
				(((struct Node *)bg)->ln_Succ);
			if (allocNum >= 0)
			    bg->ioa_Request.io_Error = 0;
			else
			    if (bg->ioa_Request.io_Error != IOERR_ABORTED)
			    	if (bg->ioa_Request.io_Flags & ADIOF_NOWAIT)
				    bg->ioa_Request.io_Error =
					    ADIOERR_ALLOCFAILED;
				else
				    continue;
			Remove(bg);
			AddTail(&ad->ad_ReplyQueue, bg);
		    }
		    Enable();
		}
		break;
	    case ADCMD_LOCK:
		if (!unit) {
		lastLock:
		    ad->ad_LockMap |= (ULONG)(ioa->ioa_Request.io_Unit);
		    Disable();
		    AddTail(&ad->ad_LockList, ioa);
		    Enable();
		    goto noReplyExit;
		}
		break;
	    case ADCMD_WAITCYCLE:
                ioa->ioa_Request.io_Unit = (struct Unit *)(chan->au_ChanFlag);
		Disable();
		if (chan->au_Playing) {
		    chan->au_Flags |= ADUNITF_WAITLIST;
		    AddTail(&chan->au_WaitList, ioa);
		    SYNCENABLE;
		    Enable();
		    goto noReplyExit;
		}
		Enable();
		goto replyExit;
	    default:
		ioa->ioa_Request.io_Error = IOERR_NOCMD;
	    }
	}
    }
replyExit:
    Permit();
    if (ad->ad_ReplyQueue.lh_Head->ln_Succ) CReplyQueue(&ad->ad_ReplyQueue);
    if (!(ioa->ioa_Request.io_Flags & IOF_QUICK)) ReplyMsg(ioa);
    return;
noReplyExit:
    Permit();
    if (ad->ad_ReplyQueue.lh_Head->ln_Succ)
	CReplyQueue(&ad->ad_ReplyQueue);
    ioa->ioa_Request.io_Flags &= ~IOF_QUICK;
}

CADAbortIO(ioa, ad)
struct IOAudio *ioa;
struct AudioDevice *ad;
{
    UBYTE unit = (ULONG)(ioa->ioa_Request.io_Unit) & 0xF;
    switch (ioa->ioa_Request.io_Command) {
        struct List *list;
    case ADCMD_ALLOCATE:
	list = &ad->ad_BackgroundList;
	goto abortFrom;
    case CMD_WRITE:
	{
	    struct AudioUnit *chan;
	    chan = ad->ad_Chan[unit];
	    Disable();
	    if (ioa == chan->au_Playing || ioa == chan->au_Next) {
		if (ioa == chan->au_Next)
		    abortNext(chan, ad);
		if (ioa == chan->au_Playing)
		    abortBuffer(chan);
	    } else
		abortFromList(ioa, &chan->au_WriteQueue, ad);
	    Enable();
	    break;
	}
    case ADCMD_WAITCYCLE:
	list = &ad->ad_Chan[unit]->au_WaitList;
	goto abortFrom;
    case ADCMD_LOCK:
	list = &ad->ad_LockList;
    abortFrom:
	abortFromList(ioa, list, ad);
	break;
    }
    CReplyQueue(&ad->ad_ReplyQueue);
}
