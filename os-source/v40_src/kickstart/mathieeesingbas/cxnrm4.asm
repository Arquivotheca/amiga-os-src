;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8  bk=0  ma=1

	include	"float.i"

	xdef	_CXNRM4		; Single normalize
	xdef	_CXRTN4		; Single repack & return

	xref	_CXZER4		; Zero handler
	xref	_CXINF4		; Infinity handler
	xref	_CXOVF4		; Overflow handler
	xref	_CXIND4		; Indefinite handler
	xref	_CXNAN4		; NAN handler


;********************************************************
;*	Normalize single precision                      *
;********************************************************

;**	Entry:
;**		d0 = Result Mantissa
;**		d6 = Result sign, exponent
;**	Exit:
;**		d0 = Floating point
;**	Uses:
;**		d1

;**	Normalize by successively smaller bit quantities.

_CXNRM4:
	cmp.l	#$0100,d0	; 16 bit normalize?
	bge.s	norm1		; - nope

	sub.w	#16*$80,d6	; Decrement exponent
	bra.s	norm2

norm1:
	swap	d0		; Swap for convenience

	tst.w	d0		; 8 bit normalize?
	bne.s	norm2		; - nope

	rol.l	#8,d0		; Shift left
	sub.w	#8*$80,d6	; Adjust exponent

norm2:
	cmp.w	#$0010,d0	; 4 bit normalize?
	bge.s	norm3		; - nope

	rol.l	#4,d0		; Shift left
	sub.w	#4*$80,d6	; Adjust exponent

norm3:
	cmp.w	#$0040,d0	; 2 bit normalize?
	bge.s	norm4		; - nope

	rol.l	#2,d0		; Shift left
	sub.w	#2*$80,d6

norm4:
	tst.b	d0		; 1 bit normalize?
	bmi.s	norm5		; - nope

	rol.l	#1,d0		; Shift left
	sub.w	#$80,d6

norm5:
	swap	d0

;**	If the exponent went negative while normalizing,
;**	denormalize to make it zero, and return a denormalized
;**	result.

	tst.w	d6		; Exponent still positive?
	bge.s	_CXRTN4		; - normalized result

	neg.w	d6		; Get denorm shift count
	lsr.w	#7,d6

	lsr.l	d6,d0		; Shift right

	add.l	d6,d6		; Set sign in final shift
	roxr.l	#1,d0

	moveq	#0,d1		; Do final shift
	addx.l	d1,d0		; Round
	rts


;*****************************************************
;*	Single precision repack & return value.      *
;*****************************************************

;**	Entry:
;**		d0 = Mantissa
;**		d6 = Result sign, exponent
;**	Exit:
;**		d0 = floating point value.
;**	Uses:
;**		d1

_CXRTN4:
	tst.w	d6		; Normalized?
	beq.s	rtn1		; - nope

;**	Return normalized value

	swap	d0		; Hide hidden bit
	eor.w	#$0080,d0
	eor.w	d6,d0		; Add exponent
	swap	d6		; Add sign
	and.w	#$8000,d6
	eor.w	d6,d0
	swap	d0
	rts

;**	Return denormalized value

rtn1:
	addq.l	#1,d0		; Shift, round, and add sign bit
	add.l	d6,d6
	roxr.l	#1,d0
	rts

	end
