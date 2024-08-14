*******************************************************************************
*
*	$Id: float.asm,v 37.1 91/02/07 16:28:19 mks Exp $
*
*******************************************************************************
******* mathieeesingbas.library/IEEESPFlt ***********************************
*
*   NAME
*	IEEESPFlt -- convert integer to IEEE single precision number
*
*   SYNOPSIS
*	  x   = IEEESPFlt(  y  )
*	 d0		   d0
*
*	  float IEEESPFlt(long);
*
*   FUNCTION
*	Convert a signed 32 bit value to a single precision IEEE value
*	and return it in d0. No exceptions can occur with this
*	function.
*
*   INPUTS
*	y -- 32 bit integer in d0
*
*   RESULT
*	x is a 32 bit single precision IEEE value
*
*   BUGS
*
*   SEE ALSO
*	IEEESPFix()
******************************************************************************

;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8 bk=0 ma=1

	include	"float.i"

	xref	_CXNRM5		; Long normalize


;***********************************************************
;*	Convert 32 bit signed integer to floating point    *
;***********************************************************

;**	Entry:
;**		d0 = long operand
;**	Exit:
;**		d0 = single operand
;**	Uses:
;**		a1

exit:
	rts

	xdef	MIIEEESPFlt
MIIEEESPFlt:

;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8  bk=0  ma=1

;	xref	_CXNRM4		; Normalize


;**	Entry:
;**		d0 = unsigned operand
;**	Exit:
;**		d0 = single operand
;**	Uses:
;**		a0

	move.l	d0,d1		; Test & save sign
	beq.s	exit		; - zero
	bpl.s	itof1		; - positive

	neg.l	d0		; Make mantissa positive

itof1:
	cmp.l	#$01000000,d0	; Must we denormalize?
	bcs.s	itof6		; - no

;**	Form result by denormalizing the integer 1-7 bits.

	move.w	#$4a80,d1	; Set exponent

;	Only used for unsigned to float
;	cmp.l	#$80000000,d0	; Denormalize 8 bits?
;	bcs.s	itof2		; - no

;	add.l	#8*$80,d6	; Adjust exponent
;	lsr.l	#8,d0		; Shift right

itof2:
	cmp.l	#$08000000,d0	; Denormalize 4 bits?
	bcs.s	itof3		; - no

	add.w	#4*$80,d1	; Adjust exponent
	lsr.l	#4,d0		; Shift right

itof3:
	cmp.l	#$02000000,d0	; Denormalize 2 bits?
	bcs.s	itof4		; - no

	add.w	#2*$80,d1	; Adjust exponent
	lsr.l	#2,d0		; Shift right

itof4:
	cmp.l	#$01000000,d0	; Denormalize 1 bit?
	bcs.s	itof5		; - no

	add.w	#1*$80,d1	; Adjust exponent
	lsr.l	#1,d0		; Shift right

;**	Round the result, possibly overflowing into the exponent
;**	field.   This is okay, and gives the right result provided
;**	the exponent is arithmetically added.

itof5:
	move.l	d1,a0		; save here for now
	moveq	#0,d1		; Round
	addx.l	d1,d0
	move.l	a0,d1		; get back

	swap	d0		; Add exponent to mantissa

	add.w	d1,d0
	swap	d1
	and.w	#$8000,d1	; Put in sign
	eor.w	d1,d0


	swap	d0
	rts
;	bra.s	itof7

;**	Form result by normalizing.

itof6:
	move.w	#$4b00,d1	; Set exponent
;	copied and edited from cxnrm4.asm
_CXNRM4:
	cmp.l	#$0100,d0	; 16 bit normalize?
	bge.s	norm1		; - nope

	sub.w	#16*$80,d1	; Decrement exponent
	bra.s	norm2

norm1:
	swap	d0		; Swap for convenience

	tst.w	d0		; 8 bit normalize?
	bne.s	norm2		; - nope

	rol.l	#8,d0		; Shift left
	sub.w	#8*$80,d1	; Adjust exponent

norm2:
	cmp.w	#$0010,d0	; 4 bit normalize?
	bge.s	norm3		; - nope

	rol.l	#4,d0		; Shift left
	sub.w	#4*$80,d1	; Adjust exponent

norm3:
	cmp.w	#$0040,d0	; 2 bit normalize?
	bge.s	norm4		; - nope

	rol.l	#2,d0		; Shift left
	sub.w	#2*$80,d1

norm4:
	tst.b	d0		; 1 bit normalize?
	bmi.s	norm5		; - nope

	rol.l	#1,d0		; Shift left
	sub.w	#$80,d1

norm5:
	eor.w	#$0080,d0	; hide hidden bit

;	DOIT

;	get the exp and sign in one word
	rol.l	#1,d1
	ror.w	#1,d1
	or.w	d1,d0

	swap	d0
	rts

	end
