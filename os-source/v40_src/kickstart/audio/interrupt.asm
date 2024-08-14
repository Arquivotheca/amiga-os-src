	TTL    '$Id: interrupt.asm,v 37.2 91/04/16 12:13:26 mks Exp $'
**********************************************************************
*								     *
*   Copyright 1986, Commodore-Amiga Inc.   All rights reserved.	     *
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030				     *
*								     *
**********************************************************************
*
* $Id: interrupt.asm,v 37.2 91/04/16 12:13:26 mks Exp $
*
* $Locker:  $
*
* $Log:	interrupt.asm,v $
* Revision 37.2  91/04/16  12:13:26  mks
* Fixed major bug in the WaitCycle reply code.  It was passing
* the address of the first node and not the address of the list.
* 
* Revision 37.1  91/03/13  18:48:26  mks
* Changed hack for EV to not trash D0...  A few more cycles but better in
* the long run.
*
* Revision 36.4  91/03/01  12:33:14  mks
* Small code cleanup.  No real changes
*
* Revision 36.3  91/02/04  19:03:25  mks
* Added kludges to make Earl Weaver baseball work!  What uglyness!
*
* Revision 36.2  90/08/29  17:53:43  mks
* Changed revision to 36...  Change ownership to MKS...
* Changed $Header to $Id
*
* Revision 33.1  86/02/12  22:49:29  sam
* revision set to 33
*
* Revision 32.3  86/02/12  22:45:23  sam
* fixed bug in testing for IO blocks to reply
*
* Revision 32.2  86/02/06  15:11:49  sam
* commented software interrupt routine
*
* Revision 32.1  86/01/14  21:23:06  sam
* revision set to 32
*
* Revision 1.1  86/01/14  20:48:34  sam
* Initial revision
*
*
**********************************************************************

	section		audio

*------ Included Files -----------------------------------------------

	INCLUDE	'exec/types.i'
	INCLUDE	'exec/ables.i'
	INCLUDE	'exec/lists.i'
	INCLUDE	'exec/nodes.i'
	INCLUDE	'exec/ports.i'
	INCLUDE	'exec/libraries.i'
	INCLUDE	'exec/io.i'
	INCLUDE	'exec/interrupts.i'
	INCLUDE	'hardware/custom.i'
	INCLUDE	'hardware/dmabits.i'
	INCLUDE	'audio.i'
	INCLUDE	'device.i'

*------ Imported Globals ---------------------------------------------

	xref	_custom
	xref	_intena

*------ Imported Functions -------------------------------------------

	xref	_LVOCause
	xref	_LVOReplyMsg

*------ Exported Functions -------------------------------------------

	xdef	_AudioInterrupt
	xdef	_SoftInterrupt
	xdef	_Restart
	xdef	_NewPlay
	xdef	_Cycles
	xdef	_PerVol
	xdef	_CLastCycle
	xdef	_CStopWrite
	xdef	_CQueueReplyList
	xdef	_CReplyQueue

;  bchg.b #1,$BFE001 ; ********************* Blink the LED *********************
;******
;
;  The REPLY macro is really like AddTail onto the channel structure
;  reply queue...
;
*	inputs	a1 - channel struct, a5 - iob
*	affects	a6
REPLY	macro
	move.l	au_ReplyQueueTail(a1),a6
	move.l	ln_Pred(a6),d0
	move.l	a6,(a5)
	move.l	a5,ln_Pred(a6)
	move.l	d0,a6
	move.l	a5,(a6)
	endm

*	------- load period and volume entry point
*	inputs	a0 - custom chips, a1 - channel struct
_PerVol:
	move.l	au_Regs(a1),a5
	move.l	au_PerVol(a1),ac_per(a5)
	bra.s	ChangeCycle

*	------- new play entry point
*	inputs	a0 - custom chips, a1 - channel struct
_NewPlay:
	move.l	au_Next(a1),a5
	btst	#ADIOB_PERVOL,io_Flags(a5)
	beq.s	1$
	move.l	au_Regs(a1),a6
	move.l	ioa_Period(a5),ac_per(a6)
1$:	move.l	au_Playing(a1),d0
	beq.s	2$
	move.l	d0,a5
	REPLY
	move.l	au_Next(a1),a5
2$:	move.l	a5,au_Playing(a1)
	btst	#ADIOB_WRITEMESSAGE,io_Flags(a5)
	beq.s	ChangeCycle
	lea	ioa_WriteMsg(a5),a5
	REPLY

*	-------	change entry to cycles
*	inputs	a0 - custom chips, a1 - channel struct
ChangeCycle:
	move.l	#_Cycles,au_IntCode(a1)

*	------- cycles entry point
*	inputs	a0 - custom chips, a1 - channel struct
_Cycles:
	subq	#1,au_Cycles(a1)
	bls.s	NoCycles

*	------- clear interrupt request
*	inputs	a0 - custom chips, a1 - channel struct
ClearRequest:
	move.w	au_IntFlag(a1),intreq(a0)

*	------- check waitcycle list
*	inputs	a1 - channel struct
CheckWaitList:
	btst	#ADUNITB_WAITLIST,au_Flags+1(a1)
	bne	ReplyWaitList

*	------- check reply list and exit
*	inputs	a1 - channel struct
CheckReplyQueue:
	move.l	au_ReplyQueueTail(a1),a5
	cmp.l	-lh_Tail(a5),a5
	bne.s	CauseSoft
	rts

*	------- restart entry point
_Restart:
	move.l	au_IntNextCode(a1),au_IntCode(a1)
	move.w	au_ChanFlag(a1),d0
	or.w	#DMAF_SETCLR,d0
	move.w	au_IntFlag(a1),intreq(a0)
	move.w	d0,dmacon(a0)
	bra.s	CheckWaitList

*	------- last cycle or zero cycle
*	inputs	a0 - custom chips, a1 - channel struct
NoCycles:
	bcc.s	LastCycle
	clr.w	au_Cycles(a1)
	move.w	au_IntFlag(a1),intena(a0)
	bra	ClearRequest

*	-------	change entry to stop
*	inputs	a0 - custom chips, a1 - channel struct
ChangeStop:
	clr.l	au_Next(a1)
	move.l	#StopWrite,au_IntCode(a1)
	bra.s	ClearRequest

*	------- stop write entry point
*	inputs	a0 - custom chips, a1 - channel struct
StopWrite:
	move.w	au_ChanFlag(a1),dmacon(a0)
	move.w	au_IntFlag(a1),intena(a0)	; Clear the interrupt enable...
	move.l	au_Playing(a1),d0	; Check if we have someone playing...
	beq.s	CheckWaitList
	move.l	d0,a5
	REPLY
	clr.l	au_Playing(a1)
	bra.s	CheckWaitList

*	-------	cause software interrupt for reply list and exit
*	inputs	a1 - channel struct
CauseSoft:
	move.l	au_SoftInterrupt(a1),a1
	move.l	EXEC_BASE,a6
	jmp	_LVOCause(a6)

*	------- last cycle entry point
*	inputs	a0 - custom chips, a1 - channel struct
LastCycle:
	move.l	au_WriteQueue(a1),a5
	tst.l	(a5)
	beq.s	ChangeStop
	move.l	ioa_Length(a5),d0
	lsr.l	#1,d0
	move.l	au_Regs(a1),a6
	move.w	d0,ac_len(a6)
	move.l	ioa_Data(a5),ac_ptr(a6)
	move.w	ioa_Cycles(a5),au_Cycles(a1)
	move.l	a5,au_Next(a1)
	move.l	(a5),a6
	move.l	ln_Pred(a5),a5
	move.l	a6,(a5)
	move.l	a5,ln_Pred(a6)
	move.l	#_NewPlay,au_IntCode(a1)
	bra	ClearRequest

*	-------	C entry point for stop write
_CStopWrite:
	movem.l	a5/a6,-(sp)
	lea	_custom,a0
	move.l	12(sp),a1
	bsr	StopWrite
	movem.l	(sp)+,a5/a6
	rts

*	-------	C entry point for last cycle
_CLastCycle:
	movem.l	a5/a6,-(sp)
	lea	_custom,a0
	move.l	12(sp),a1
	bsr.s	LastCycle
        movem.l (sp)+,a5/a6
        rts

*	-------	C entry point for queue up list to reply
_CQueueReplyList:
	movem.l	a5/a6,-(sp)
	movem.l	12(sp),a5/a6
	lea	ad_ReplyQueue+lh_Tail(a6),a0
	bsr.s	QueueReplyList
        movem.l (sp)+,a5/a6
        rts

*	-------	C entry point for reply list
_CReplyQueue:
	movem.l	a5/a6,-(sp)
	move.l	12(sp),a1
	move.l	EXEC_BASE,a6
	bsr.s	_SoftInterrupt
        movem.l (sp)+,a5/a6
	rts

*	-------	reply waitcycle list
*	inputs	a1 - channel struct
ReplyWaitList:
	bclr	#ADUNITB_WAITLIST,au_Flags+1(a1)
	lea	au_WaitList(a1),a5
        move.l  au_ReplyQueueTail(a1),a0
	bsr.s	QueueReplyList
	bra	CauseSoft

*       ------- queue up list to reply
*	inputs	a0 - reply queue tail, a5 - list
*	affects	a6
QueueReplyList:
        move.l	lh_TailPred(a5),a6
        move.l  a0,(a6)
        move.l  ln_Pred(a0),a6
        move.l  lh_TailPred(a5),ln_Pred(a0)
        move.l  (a5),(a6)
        NEWLIST a5
        rts

*------ (audio.device)
*
*	SoftInterrupt - audio software interrupt routine.
*
*   SYNOPSIS
*	SoftInterrupt(list)
*	               a1
*
*   FUNCTION
*
*   INPUTS
*	list - pointer to list to reply (singlely linked list with tail ptr)
*
*---------------------------------------------------------------------
_SoftInterrupt:
	move.l	a1,a5			make a5 reply list
SoftI1:	DISABLE
	move.l	(a5),a1			a1 points to first io block
	move.l	(a1),d0			d0 is first io blocks successor
	beq.s	SoftI3			if zero then there are no io blocks
	move.l	d0,(a5)			remove first io block
	cmp.l	lh_TailPred(a5),a1	check if first io block is also last
	bne.s	SoftI2
	move.l	a5,lh_TailPred(a5)	if it is, point list pred to head
SoftI2	ENABLE
	jsr	_LVOReplyMsg(a6)	reply first io block
	bra.s	SoftI1
SoftI3	ENABLE
SoftI4	rts

*------ (audio.device)
*
*   NAME
*	AudioInterrupt - audio interrupt routine.
*
*   SYNOPSIS
*	AudioInterrupt(unit)  (a0==CUSTOM)
*			  a1
*
*   FUNCTION
*
*   INPUTS
*	unit - audio unit structure address.
*
*---------------------------------------------------------------------
_AudioInterrupt:
*
* This is a major hack.  Earl Weaver Baseball does not open
* the audio.device but was relying on the fact that the
* interrupt code and unit structures were still valid after using
* the narrator.  Earl Weaver was just poking the interrupt
* register and thus causing the interrupt to happen without
* having any audio units set up.  Basically, it was working
* due to luck of a side-effect of the narrator/audio interactions.
*
* The following tests are done to prevent the program from crashing
* when it does this.  By checking if the values are NULL we can catch
* the common cause of the crash.  (And even get the sound to work!)
*
* The original code was:
*	move.l	au_IntCode(a1),a5
*	jmp	(a5)
*
	move.l	au_IntCode(a1),a5
	cmp.l	#0,a5			; Set the ZERO flag as needed...
	bne.s	OkInt			; If not NULL, we are ok...
	lea	_NewPlay(pc),a5		; Fake a NewPlay routine.
OkInt	jmp	(a5)
	end
