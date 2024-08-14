*************************************************************************
*									*
*   Copyright 1984,85,88,89,90 Commodore-Amiga, Inc.			*
*   All rights reserved.  No part of this program may be reproduced,	*
*   transmitted, transcribed, stored in retrieval system, or		*
*   translated into any language or computer language, in any form	*
*   or by any means, electronic, mechanical, magnetic, optical, 	*
*   chemical, manual or otherwise, without the prior written		*
*   permission of Commodore-Amiga, Incorporated.			*
*									*
*************************************************************************
*
*	$Id: debug.asm,v 39.1 91/12/19 19:47:27 mks Exp $
*
*	$Log:	debug.asm,v $
* Revision 39.1  91/12/19  19:47:27  mks
* Now is assembled for use in a .lib rather than just linked .o
* This means it does not need to always re-assemble
* 
* Revision 39.0  91/10/15  08:26:37  mks
* V39 Exec initial checkin
*
*************************************************************************
*
*	Standard debugging print code
*
*		^S	- stop
*		^X	- suspend output
*		^Q	- continue
*

		NOLIST
		INCLUDE "assembly.i"
		INCLUDE "constants.i"
		LIST

		;-Bryce's debugging stuff
		XDEF	printit		;print funtion for NPRINTF
		XDEF	PSCODE		;raw bytes function
		XDEF	BarePutStr	;(a0: string, a5: return address)

		XREF	_serdat
		XREF	_serdatr
		XREF	_serper
		XREF	RawDoFmt


;----------------------------------------------------------------------------
		;
		;Print a string using stack based arguments.
		;All registers are preserved.  Call with:
		;
		;	move.l	xxxx,-(sp)	;save stack args
		;	bsr	printit
		;	bra.s	end		;must use bra.s
		;	dc.b	'string',0
		;	ds.w	0
		;end:	addq.l	#4,sp		;pop stack
		;
		;
printit:	movem.l A0/A1/A2/A3/D0/D1,-(SP)	;Note calculation below
		move.w	#BAUD_9600,_serper
		move.l	6*4(sp),a0		;Return address from stack
		addq.l	#2,a0			; compensate for bra.s
		lea.l	6*4+4(sp),a1		;Address of stackframe
		lea.l	PSCODE(pc),a2		;Address of output function
		suba.l	a3,a3			;No data pointer
		BSR	RawDoFmt
		movem.l (SP)+,A0/A1/A2/A3/D0/D1
		rts

PSCODE:		tst.b	d0
		beq.s	ignore		; skip NULL
		nop			;sync pipeline, wait a while
		move.w	_serdatr,d1
		btst	#13,d1
		beq.s	PSCODE
		and.b	#$7f,d1
		cmp.b	#$18,d1 	; Check for CAN (^X)
		beq.s	ignore
		cmp.b	#$13,d1 	; Check for Xoff
		beq.s	PSCODE
		and.w	#$ff,d0
		or.w	#$100,d0
		move.w	d0,_serdat
ignore: 	rts
;----------------------------------------------------------------------------




		;
		;No stack print function.  No formatting either.
		;
		;a0 - string
		;a5 - return address
		;
BarePutStr:	move.w	#BAUD_9600,_serper

bps_wait:	nop
		move.w	_serdatr,d0	; make sure we do a word read
		cmp.b	#$18,d1		; Check for CAN (^X)
		beq.s	bps_end
		btst	#13,d0		; Check for Xoff
		beq.s	bps_wait

		move.w	#$100,d0	; add in the stop bit
		or.b	(a0)+,d0        ; and the byte to output
		beq.s	bps_end		; skip NULL

		move.w	d0,_serdat
		bra.s	bps_wait

bps_end:	jmp	(a5)            ; return to caller

	END
