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
* $Id: cdevice.c,v 36.2 90/08/29 17:50:43 mks Exp $
*
* $Locker:  $
*
* $Log:	cdevice.c,v $
* Revision 36.2  90/08/29  17:50:43  mks
* Changed revision to 36...  Change ownership to MKS...
* Changed $Header to $Id
* 
* Revision 36.1  89/05/17  16:33:59  kodiak
* fix Expunge
*
* Revision 36.0  89/01/06  18:00:49  kodiak
* remove expunge capability for workbench version
*
* Revision 32.1  86/01/14  21:22:08  sam
* revision set to 32
*
* Revision 1.1  86/01/14  20:30:12  sam
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
#include "hardware/custom.h"
#include "hardware/intbits.h"
#include "hardware/dmabits.h"
#include "hardware/adkbits.h"
#include "audio.h"
#include "device.h"

extern struct Interrupt *SetIntVector();
extern VOID AudioInterrupt();
extern VOID SoftInterrupt();
extern TEXT *ADName;
extern struct AudChannel aud[];
extern UWORD adkcon;
extern UWORD dmacon;
extern UWORD intena;
extern UWORD intenar;
extern UWORD intreq;
extern UWORD intreqr;

UBYTE bitNum[] = { 0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0 };

/* functions */

CADOpen(num, ioa, flags, ad)
UWORD num;
struct IOAudio *ioa;
UWORD flags;
struct AudioDevice *ad;
{
    ++(ad->ad_Library.lib_OpenCnt);
    ioa->ioa_Request.io_Command = ADCMD_ALLOCATE;
    ioa->ioa_Request.io_Flags = ADIOF_NOWAIT | IOF_QUICK;
    BeginIO(ioa);
    WaitIO(ioa);
    ioa->ioa_Request.io_Flags = 0;
    if (ioa->ioa_Request.io_Error)
	CADClose(ioa, ad);
    else
	ad->ad_Flags &= ~ADDEVF_EXPUNGE;
}

CADClose(ioa, ad)
struct IOAudio *ioa;
struct AudioDevice *ad;
{
    UBYTE saveCommand, saveFlags, saveError;
    saveCommand = ioa->ioa_Request.io_Command;
    saveFlags = ioa->ioa_Request.io_Flags;
    saveError = ioa->ioa_Request.io_Error;
    ioa->ioa_Request.io_Unit = (struct Unit *)((1 << ADHARD_CHANNELS) - 1);
    ioa->ioa_Request.io_Command = ADCMD_FREE;
    DoIO(ioa);
    ioa->ioa_Request.io_Command = saveCommand;
    ioa->ioa_Request.io_Flags = saveFlags;
    ioa->ioa_Request.io_Error = saveError;
    ioa->ioa_Request.io_Device = (struct Device *)-1;
    if (!(--ad->ad_Library.lib_OpenCnt) && (ad->ad_Flags & ADDEVF_EXPUNGE))
	return(CADExpunge(ad));
    return(0);
}

CADExpunge(ad)
struct AudioDevice *ad;
{
    if (ad->ad_Library.lib_OpenCnt) {
	ad->ad_Flags |= ADDEVF_EXPUNGE;
	return(0);
    } {
	struct AudioUnit *chan;
	short i;

	for (i = 0; i < ADHARD_CHANNELS; i++) {
	    chan = ad->ad_Chan[1<<i];
	    SetIntVector(chan->au_IntBit, chan->au_DefaultInterrupt);
	}
    } {
	LONG segList;

    	segList = ad->ad_SegList;
	Remove(ad);
	FreeMem((LONG)ad - ad->ad_Library.lib_NegSize,
		ad->ad_Library.lib_NegSize + ad->ad_Library.lib_PosSize);
	return(segList);
    }
}

CADExtFunc()
{
}

struct AudioDevice *CADInit(ad, segList)
struct AudioDevice *ad;
LONG segList;
{
    dmacon = DMAF_AUD0 | DMAF_AUD1 | DMAF_AUD2 | DMAF_AUD3;
    ad->ad_SegList = segList;
    NewList(&ad->ad_ReplyQueue);
    NewList(&ad->ad_LockList);
    NewList(&ad->ad_BackgroundList);
    ad->ad_LevelPrec[0] = ADALLOC_MAXPREC;
    ad->ad_SoftInterrupt.is_Node.ln_Type = NT_INTERRUPT;
    ad->ad_SoftInterrupt.is_Node.ln_Name = &ADName;
    ad->ad_SoftInterrupt.is_Data = (APTR)&ad->ad_ReplyQueue;
    ad->ad_SoftInterrupt.is_Code = SoftInterrupt;
    {
	UWORD i;
	struct AudioUnit *chan = ad->ad_Unit;
	struct AudChannel *regs = aud;
	WORD *allocKey = ad->ad_AllocKey;
	WORD *prec = ad->ad_Prec;
	for (i = 0; i < sizeof(bitNum); ++i) {
	    ad->ad_Chan[i] = &ad->ad_Unit[bitNum[i]];
	    if (i < ADHARD_CHANNELS) {
		NewList(&chan->au_WriteQueue);
		NewList(&chan->au_WaitList);
		chan->au_AudioInterrupt.is_Node.ln_Type = NT_INTERRUPT;
		chan->au_AudioInterrupt.is_Node.ln_Name = &ADName;
		chan->au_AudioInterrupt.is_Data = (APTR)chan;
		chan->au_AudioInterrupt.is_Code = AudioInterrupt;
		chan->au_AudioDevice = ad;
		chan->au_SoftInterrupt = &ad->ad_SoftInterrupt;
		chan->au_ReplyQueueTail = &ad->ad_ReplyQueue.lh_Tail;
		chan->au_IntFlag = (chan->au_ChanFlag =
			1 << (chan->au_ChanBit = i)) << INTB_AUD0;
		chan->au_DefaultInterrupt = SetIntVector(chan->au_IntBit =
			INTB_AUD0 + i, &chan->au_AudioInterrupt);
		*prec = ADALLOC_MINPREC - 1;
		chan->au_Prec = prec++;
		*allocKey = -1;
		chan->au_AllocKey = allocKey++;
		chan->au_Regs = regs++;
		reset(chan++, ad);
	    }
	}
	return(ad);
    }
}
