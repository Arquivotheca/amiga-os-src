**********************************************************************
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
* $Id: device.i,v 36.2 90/08/29 17:53:09 mks Exp $
*
* $Locker:  $
*
* $Log:	device.i,v $
* Revision 36.2  90/08/29  17:53:09  mks
* Changed revision to 36...  Change ownership to MKS...
* Changed $Header to $Id
* 
* Revision 33.1  86/02/13  15:03:45  sam
* revision set to 33
*
* Revision 1.1  86/01/14  20:30:38  sam
* Initial revision
*
*
**********************************************************************

EXEC_BASE		EQU	4

ADIOERR_NOABORT		EQU	1

ADUNITB_STOPPED		EQU	0
ADUNITF_STOPPED		EQU	1<<0
ADUNITB_WAITLIST	EQU	1
ADUNITF_WAITLIST	EQU	1<<1

ADDEVB_EXPUNGE		EQU	0
ADDEVF_EXPUNGE		EQU	1<<0
ADDEVB_UPDATEALLOC	EQU	1
ADDEVF_UPDATEALLOC	EQU	1<<1

 STRUCTURE	AudioUnit,0
	APTR	au_IntCode
	APTR	au_IntNextCode
	APTR	au_Regs
	APTR	au_Playing
	APTR	au_Next
	WORD	au_Flags
	STRUCT	au_WriteQueue,lh_Size
	STRUCT	au_WaitList,lh_Size
	STRUCT	au_AudioInterrupt,is_Size
	APTR	au_AudioDevice
	APTR	au_SoftInterrupt
	APTR	au_ReplyQueueTail
	APTR	au_DefaultInterrupt
	LONG	au_PerVol
	WORD	au_Cycles
	APTR	au_AllocKey
	APTR	au_Prec
	WORD	au_ChanFlag
	WORD	au_IntFlag
	BYTE	au_ChanBit
	BYTE	au_IntBit
	LABEL	au_SIZEOF

 STRUCTURE	AudioDevice,0
	STRUCT	ad_Library,Lib_Size
	WORD	ad_Flags
	LONG	ad_SegList
	STRUCT	ad_Chan,4*(1<<ADHARD_CHANNELS)
	WORD	ad_AllocMaster
	STRUCT	ad_AllocKey,2*ADHARD_CHANNELS
	STRUCT	ad_Prec,2*ADHARD_CHANNELS
	STRUCT	ad_LevelPrec,2*(ADHARD_CHANNELS+1)
	STRUCT	ad_LevelMap,1*(ADHARD_CHANNELS+1)
	BYTE	ad_LockMap
	STRUCT	ad_ReplyQueue,lh_Size
	STRUCT	ad_LockList,lh_Size
	STRUCT	ad_BackgroundList,lh_Size
	STRUCT	ad_SoftInterrupt,is_Size
	STRUCT	ad_Unit,au_SIZEOF*ADHARD_CHANNELS
	LABEL	ad_SIZEOF
