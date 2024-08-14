;	:ts=8  bk=0  ma=1

	include	"float.i"

	xdef	_CXADJ4		; Single adjust after multiply/divide

	xref	_CXOVF4		; Overflow handler
	xref	_CXUNF4		; Underflow handler

ROUND	equ	1

;**************************************************************
;*	Adjust exponent and mantissa after multiply/divide    *
;**************************************************************

;**	Entry:
;**		d0 = multiply/divide mantissa
;**		d6 = result sign, exponent
;**	Exit:
;**		d0 = floating point result
;**	Uses:
;**		none
;	use	d1 = was preshifted
realdodenorm:
;*****
;*****	Handle underflow.
;*****

;*****	Form denormalize shift count.

adj10:
	neg.w	d6		; Negate exponent
	lsr.w	#7,d6		; Form shift count
	addq.w	#5,d6

;**	Check for underflow

	cmp.w	#30,d6		; Shift count > 30 ?
	blt.s	adj11		; - no

	bra	_CXUNF4		; - underflow

;**	Denormalize 7-30 bits

adj11:
	subq.l	#1,d6
	lsr.l	d6,d0		; leave last throw away bit
	clr.l	d7
	addx.l	d7,d0		; only round up top 3/4
	lsr.l	#1,d0		; Now finish shift

;**	Do final rounding.  It is possible that the rounding
;**	will overflow into the exponent position.  That's okay
;**	and gives the correct result.

	ifd ROUND1
	moveq	#0,d7		; Round
	addx.l	d7,d0
	endc

	and.l	#$80000000,d6	; Add sign
	eor.l	d6,d0
	rts			; return

dodenorm:
	bra	realdodenorm

denorm1:

	lsr.l	#1,d0		; Shift right

	tst.w	d6		; Check exponent
	bge.s	adj20		; - normalized
	bra.s	adj10

myCXOVF4:
	bra	_CXOVF4		; Overflow

_CXADJ4:

;**	Denormalize 1 bit if necessary.

;	cmp.l	#$20000000,d0	; Need to shift right?
;	blt.s	adj01		; - no
	btst	#29,d0		; Need to shift right?
	bne.s	denorm1
;	beq.s	adj01		; - no

adj01:
	sub.w	#$80,d6		; Adjust exponent
;	bge.s	adj20		; - normalized
	blt.s	dodenorm


;*****
;*****	Process normalized result.
;*****

;**	Shift and round.

adj20:

	ifd	ROUND
;	addq.l	#8,d0		; Shift and round
;	addq.l	#8,d0
	moveq	#16,d1		; spare register?
	add.l	d1,d0
	endc

	lsr.l	#5,d0		; Shift right 5

;**	Form magnitude result.  The previous round may have
;**	overflowed into the exponent field.  However this still
;**	gives the correct result if the exponent is added
;**	arithmetically, and the hidden bit subtracted out.

	swap	d0
	add.w	d6,d0		; Add in exponent

	cmp.w	#$7f80,d0	; Did it overflow?
;	blt.s	adj21		; - no
	bge.s	myCXOVF4

;	bra	_CXOVF4		; Overflow

;**	Form final result

adj21:
	swap	d6		; Add sign
	and.w	#$8000,d6
	eor.w	d6,d0
	swap	d0
	rts

	end
