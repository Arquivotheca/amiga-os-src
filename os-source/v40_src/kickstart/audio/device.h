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
* $Id: device.h,v 36.2 90/08/29 17:52:41 mks Exp $
*
* $Locker:  $
*
* $Log:	device.h,v $
* Revision 36.2  90/08/29  17:52:41  mks
* Changed revision to 36...  Change ownership to MKS...
* Changed $Header to $Id
* 
* Revision 32.1  86/01/14  21:22:56  sam
* revision set to 32
*
* Revision 1.1  86/01/14  20:30:35  sam
* Initial revision
*
*
*********************************************************************/

#define ADIOERR_NOABORT		1

#define ADUNITB_STOPPED		0
#define ADUNITF_STOPPED		(1<<0)
#define ADUNITB_WAITLIST	1
#define ADUNITF_WAITLIST	(1<<1)

#define ADDEVB_EXPUNGE		0
#define ADDEVF_EXPUNGE		(1<<0)
#define ADDEVB_UPDATEALLOC	1
#define ADDEVF_UPDATEALLOC	(1<<1)

struct AudioUnit {
    int (*au_IntCode)();
    int (*au_IntNextCode)();
    struct AudChannel *au_Regs;
    struct IOAudio *au_Playing;
    struct IOAudio *au_Next;
    UWORD au_Flags;
    struct List au_WriteQueue;
    struct List au_WaitList;
    struct Interrupt au_AudioInterrupt;
    struct AudioDevice *au_AudioDevice;
    struct Interrupt *au_SoftInterrupt;
    struct List *au_ReplyQueueTail;
    struct Interrupt *au_DefaultInterrupt;
    LONG au_PerVol;
    WORD au_Cycles;
    WORD *au_AllocKey;
    WORD *au_Prec;
    WORD au_ChanFlag;
    WORD au_IntFlag;
    UBYTE au_ChanBit;
    UBYTE au_IntBit;
};

struct AudioDevice {
    struct Library ad_Library;
    UWORD ad_Flags;
    LONG ad_SegList;
    struct AudioUnit *ad_Chan[1 << ADHARD_CHANNELS];
    WORD ad_AllocMaster;
    WORD ad_AllocKey[ADHARD_CHANNELS];
    WORD ad_Prec[ADHARD_CHANNELS];
    WORD ad_LevelPrec[ADHARD_CHANNELS + 1];
    UBYTE ad_LevelMap[ADHARD_CHANNELS + 1];
    UBYTE ad_LockMap;
    struct List ad_ReplyQueue;
    struct List ad_LockList;
    struct List ad_BackgroundList;
    struct Interrupt ad_SoftInterrupt;
    struct AudioUnit ad_Unit[ADHARD_CHANNELS];
};
