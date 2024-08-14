
**************************************************************************
*  $Id: mousebuttons.asm,v 40.0 94/02/15 17:33:36 davidj Exp $
*
*	This file is supposed to be an interface allowing an ALERT
*	to read the mouse buttons directly from the hardware if the
*	gameport device has quit operating.
*
*	It is based direct interaction with ROM-wack.  robp.  6/12/85
*
*	requires only scratch registers A0 and D0, result in D0.
*
*		Rob Peck
**************************************************************************


	xdef	_CheckButtons

*
* New code (just one call) that checks both buttons
* and if something happened on them will return
* the results (bit pattern...)	- mks
*
_CheckButtons:	moveq.l	#0,d0		; Default return...
		move.w	#$0400,d1	; Get bit to check for...
		and.w	$DFF016,d1	; Read hardware and mask result...
		bne.s	CheckLeft	; If not down, skip this part
		addq.l	#1,d0		; Set d0, bit 0...
*
CheckLeft:	moveq.l	#$0040,d1	; Get bit to look for (quick way)
		and.b	$BFE001,d1	; Read hardware and mask result...
		bne.s	NoLeft		; If not down, skip...
		addq.l	#2,d0		; Set d0, bit 1...
*
NoLeft:		; If I put in the SAD serial check, it would go here...
		; Just don't know how good that would be...
		rts
*
		END
