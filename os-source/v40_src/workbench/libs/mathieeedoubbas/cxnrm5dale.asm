
*******************************************************************************
*
*	$Header:
*
*******************************************************************************

;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8 bk=0 ma=1

	include	"float.i"

	xdef	_cxnrm5		; Double Normalize

	xref	_CXUNF5		; Underflow handler
	xref	_CXOVF5		; Overflow handler
	xref	_CXNAN5		; NAN handler
	xref	_CXIND5		; Indefinite handler
	xref	_CXINF5		; Infinite handler
	xref	_CXZER5		; Zero handler


;**********************************************************
;*	Double normalize                                  *
;**********************************************************

;**	Entry:
;**		d0,d1 = Result Mantissa
;**		d4 = Result sign
;**		d5 = Result exponent
;**	Exit:
;**		d0,d1 = Floating point
;**
;**	Uses:
;**		d2,d3

_cxnrm5:

;**	Do a 16 bit normalize while possible to
;**	get things started.

norm:
	cmp.l	#$20,d0		; 16 bit normalize?
	bge.s	norm10		; - nope

	swap	d0		; Do 16 bit shift
	swap	d1
	move.w	d1,d0
	clr.w	d1

	sub.w	#16*16,d5	; Decrement exponent
	bge.s	norm		; - no underflow

	bra.s	norm20		; Negative exponent

;**	Do a series of shifts on the upper longword.
;**	Remember the shift count, so we can go back and
;**	shift the stuff in the lower longword to match.

_CXTAB5:
        dc.b    5,4,3,3,2,2,2,2,1,1,1,1,1,1,1,1
        dc.b    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

norm10:
	moveq	#0,d3		; Initialize shift count

	cmp.l	#$2000,d0	; 8 bit normalize
	bge.s	norm11		; - nope

	lsl.l	#8,d0		; Shift d0
	addq.w	#8,d3		; Adjust count

norm11:
	swap	d0		; Swap for convenience

	tst.w	d0		; 4 bit normalize
	bne.s	norm12		; - nope

	rol.l	#4,d0		; Shift d0
	addq.w	#4,d3		; Adjust count

norm12:
	moveq	#0,d2		; Get final shift count
	move.b	_CXTAB5(d0.w),d2

	rol.l	d2,d0		; Do rotate
	add.w	d2,d3

	swap	d0

;**	Shift the lower longword to match the upper
;**	longword normalize.

	move.l	d1,d2		; Shift left by saved count
	lsl.l	d3,d1
	rol.l	d3,d2
	eor.w	d1,d2
	eor.w	d2,d0

	lsl.w	#4,d3		; Compute exponent
	sub.w	d3,d5
	blt.s	norm20		; - negative exponent

;**	Return operand with positive exponent.

	swap	d0		; Add in exponent and sign
	add.w	d5,d0
	or.w	d4,d0
	swap	d0
	movem.l	(sp)+,d2-d7
	rts


;*****
;*****	Handle exponent going negative
;*****

;**	Entry:
;**		d0,d1 = Mantissa
;**		d4 = Sign
;**		d5 = Exponent (non-zero)
;**	Exit:
;**		d0,d1 = Floating point result

norm20:
	neg.w	d5		; Get denorm shift count
	lsr.w	#4,d5

	move.l	d0,d2		; Shift right
	lsr.l	d5,d0
	ror.l	d5,d2
	lsr.l	d5,d1
	eor.l	d0,d2
	eor.l	d2,d1

	swap	d0		; Form return operand
	eor.w	d4,d0
	swap	d0
	movem.l	(sp)+,d2-d7
	rts

	end
