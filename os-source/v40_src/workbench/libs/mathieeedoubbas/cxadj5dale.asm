
*******************************************************************************
*
*	$Header:
*
*******************************************************************************

;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8 bk=0 ma=1

	idnt	CXADJ5

	include	"float.i"

	xdef	_cxadj5		; Adjust after multiply/divide

	xref	_cxunf5		; Underflow handler
	xref	_cxovf5		; Overflow handler

;*****	Form denormalize shift count.

adj20_bar:
adj11:
	neg.w	d5		; Negate exponent
	lsr.w	#4,d5		; Form shift count
;	add.w	#4,d5
	add.w	#2,d5

;**	Check for underflow

	cmp.w	#57,d5		; Shift count > 57 ?
	ble.s	adj13		; - no

	bra	_cxunf5		; - underflow

;**	Normalize 16 bit chunks

adj12:
	move.w	d0,d1		; Shift right 16
	clr.w	d0
	swap	d0
	swap	d1

adj13:
	sub.w	#16,d5		; Shift count > 16 ?
	bgt.s	adj12		; - no

	add.w	#16,d5

;**	Denormalize 1-16 bits

	move.l	d0,d2		; Shift right by count
	lsr.l	d5,d0
	ror.l	d5,d2
	lsr.l	d5,d1
	eor.l	d0,d2
	eor.l	d2,d1

;**	Do final rounding.  It is possible that the rounding
;**	will overflow into the exponent position.  That's okay
;**	and gives the correct result.

;	kill rounding dale
;	moveq	#0,d2		; Round
;	addx.l	d2,d1
;	addx.l	d2,d0

	swap	d0		; Add sign
	eor.w	d4,d0
	swap	d0
	movem.l	(sp)+,d2-d7
	rts			; return


;**************************************************************
;**	Adjust exponent and mantissa after multiply/divide    *
;**************************************************************

;**	Entry:
;**		d0,d1 = multiply/divide mantissa
;**		d4 = result sign
;**		d5 = result exponent
;**	Exit:
;**		d0,d1 = floating point result
;**	Uses:
;**		d2

adj10_bar:
	lsr.l	#1,d0		; Shift right
	roxr.l	#1,d1

	tst.w	d5		; Check exponent
	bge.s	adj20		; - normalized
	bra	adj11

_cxadj5:

;MSB	equ	25
MSB	equ	23

;**	Denormalize 1 bit if necessary.
;DEBUG	equ 1
;	a0 is assumed to be 7ff0.l

	ifd	DEBUG
	move.l	d0,-(sp)
	move.l	d1,-(sp)
	addq.l	#8,sp
	endc

	btst	#MSB,d0
;	beq.s	adj10
	bne.s	adj10_bar
;	cmp.l	#$02000000,d0	; Need to shift right?
;	blt.s	adj10		; - no

;*****
;*****	Handle underflow.
;*****

adj10:
	sub.w	#16,d5		; Adjust exponent
	blt.s	adj20_bar
;	bge.s	adj20		; - normalized

;*****
;*****	Process normalized result.
;*****

adj20:

;**	Shift and round.

	ifd	QWE
	move.l	d0,d2		; Shift right 4
	lsr.l	#4,d0
	ror.l	#4,d2
	lsr.l	#4,d1
	eor.l	d0,d2
	eor.l	d2,d1
	endc

;				shift right by 2
	lsr.l	#1,d0		; Shift right
	roxr.l	#1,d1
	lsr.l	#1,d0		; Shift right
	roxr.l	#1,d1

;	kill round, dale
;	moveq	#0,d2		; Round
;	addx.l	d2,d1
;	addx.l	d2,d0

;**	Form magnitude result.  The previous round may have
;**	overflowed into the exponent field.  However this still
;**	gives the correct result if the exponent is added
;**	arithmetically, and the hidden bit subtracted out.

	swap	d0
	add.w	d5,d0		; Add in exponent

	cmp.w	a0,d0	; Did it overflow?
	bge.s	doovf

;**	Form final result

adj21:
	eor.w	d4,d0		; Add sign
	swap	d0
	movem.l	(sp)+,d2-d7
	rts


doovf:
	bra	_cxovf5

	end
