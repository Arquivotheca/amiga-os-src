*******************************************************************************
*
*	$Id: faddsub.asm,v 40.1 93/03/11 17:21:56 mks Exp $
*
*******************************************************************************


********************************************************************************
*
* If not FPU-Only, we need this...
*
	IFND	CPU_FPU
*
;	:ts=8 bk=0 ma=1

	include	"float.i"

******* mathieeesingbas.library/IEEESPSub ***********************************
*
*   NAME
*	IEEESPSub -- subtract one single precision IEEE number from another
*
*   SYNOPSIS
*	  x   = IEEESPSub(  y  ,  z  );
*	 d0		   d0     d1
*
*	float	x,y,z;
*
*   FUNCTION
*	Compute x = y - z in IEEE single precision.
*
*   INPUTS
*	y -- IEEE single precision floating point value
*	z -- IEEE single precision floating point value
*
*   RESULT
*	x -- IEEE single precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEESPAdd
******************************************************************************


******* mathieeesingbas.library/IEEESPAdd ***********************************
*
*   NAME
*	IEEESPAdd -- add one single precision IEEE number to another
*
*   SYNOPSIS
*	  x   = IEEESPAdd(  y  ,  z  );
*	 d0		   d0     d1
*
*	float	x,y,z;
*
*   FUNCTION
*	Compute x = y + z in IEEE single precision.
*
*   INPUTS
*	y -- IEEE single precision floating point value
*	z -- IEEE single precision floating point value
*
*   RESULT
*	x -- IEEE single precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEESPSub
******************************************************************************

;*****************************************************
;*	Single precision repack & return value.      *
;*      special copied from cxnrm4.asm
;*****************************************************

restore	macro
	move.l	a0,d6
	move.l	a1,d7
	move.w	(sp)+,D2
	endm

	xdef	MIIEEESPSub
MIIEEESPSub:
	bchg	#31,d1
;	drop into add routine

;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8  bk=0  ma=1


	xref	_cxnrm4dale
;	xref	_cxrtn4dale

	xref	_cxzer4dale	; Zero handler
	xref	_cxinf4dale	; Infinity handler
	xref	_cxovf4dale	; Overflow handler
	xref	_CXIND4		; Indefinite handler
	xref	_CXNAN4		; NAN handler


;*************************************************************
;*	Addition interface routines.                         *
;*************************************************************

;**	Entry:
;**		d0 = single operand 1
;**		d1 = single operand 2
;**	Exit:
;**		d0 = single result
;**	Uses:
;**		a0,a1,d1


;*********************************************************
;*	Floating Addition                                *
;*********************************************************

;**	Entry:
;**		d0 = Floating Operand 1
;**		d1 = Floating Operand 2
;**
;**	Exit:
;**		d0 = Floating Sum
;**
;**	Uses:
;**		d6,d7

	xdef MIIEEESPAdd
MIIEEESPAdd:
	move.l	d6,a0		; Save d6,d7
	move.l	d7,a1
	move.w	D2,-(sp)

	move.w	#$80,D2		; often used constant

;**	Save sign of X and Y.

	move.l	d0,d6		; Save entire upper half
	move.l	d1,d7

;**	Unpack X.

	move.w	#$7f80,d7	; Exponent mask

	swap	d0		; Get unsigned(X)
	and.w	#$7fff,d0

	move.w	d0,d6		; Get exponent(X)
	and.w	d7,d6
	eor.w	d6,d0

;**	Unpack Y.

add2:
	swap	d1		; Get unsigned(Y)
	and.w	#$7fff,d1

	and.w	d1,d7		; Get exponent(Y)
	eor.w	d7,d1

	cmp.w	d7,d6		; compare exponents
	bne	add40		; - not equal


;****
;****	Handle Equal exponents
;****

;**	d0 = Mantissa X
;**	d1 = Mantissa Y
;**	d6 = sign X, exponent X
;**	d7 = sign Y, exponent Y
;**
;**	d0,d1 are word swapped.

add10:

;**	Check for both exponents INF or NAN.

	cmp.w	#$7f80,d6	; Check for wierd exponent
	bne.s	add14		; - Regular number

;**	If X is NAN, and prec(X) >= prec(Y) return X.
;**	Otherwise if Y is NAN, return Y.

	move.w	d1,d7		; Test NAN precedence
	or.w	#$0007,d7
	sub.w	d0,d7
	and.w	#$fff8,d7	; (upper 4 mantissa bits only)
	bgt.s	add11		; - precedence(Y) > precedence(X)

	tst.l	d0		; Mantissa(X) zero?
	bne.s	add12		; - NAN

	tst.l	d1		; Mantissa(Y) zero?
	beq.s	add13		; - INF

add11:
	move.l	d1,d0		; Set to return Y

add12:
cxnan:
	restore
	bra	_CXNAN4		; Process NAN

;**	Both X and Y are infinity.   If they have the same
;**	sign, return infinity.   Otherwise generate indefinite.

add13:
	eor.l	d6,d7		; Sign of infinity equal?
	bpl.s	add23		; - yes
	bmi.s	add24		; - no

;**	Check for denormalized mantissa

add14:
	tst.w	d6		; Zero exponent?
	bne.s	add18		; - no

;**	If X is true zero, check Y.  If Y is also zero, logically
;**	and their signs together and return signed zero.  If Y
;**	is nonzero, return Y.

	swap	d0		; Test X mantissa
	bne.s	add16		; - nonzero

	swap	d1		; Test Y mantissa
	bne.s	add15		; - nonzero

	and.l	d7,d6		; Form result sign

	move.l	d6,d0

	restore
	bra	_cxzer4dale		; return signed zero

add15:
	clr.w	d7		; Restore & return Y
	or.l	d7,d1
	move.l	d1,d0
return:
	restore
	rts


retx:
	clr.w	d6		; Restore & return X
	or.l	d6,d0
	bra	return


add16:
	swap	d1		; Test Y mantissa
	beq.s	retx		; restore and return X

;**	Operands are denormalized.  Shift left 1 bit and proceed.

add17:
	add.l	d0,d0		; Shift mantissa(X)
	add.l	d1,d1		; Shift mantissa(Y)
	bra.s	add19

;**	Operands are normalized.  Add hidden bit.

add18:
	eor.b	D2,d0	; Put in hidden bit.
	eor.b	D2,d1

	swap	d0		; Unswap mantissas
	swap	d1

add19:
	eor.l	d6,d7		; Same sign?
	bpl.s	add30		; - add

;*****
;*****	Subtract operands with the same exponent.
;*****

add20:
	sub.l	d1,d0		; Subtract mantissas
	bmi.s	add21		; - result negative
	bne	_cxnrm4dale

;	result is zero, and in d0
	restore
	rts

add21:
	neg.l	d0		; Change result sign
	bchg	#31,d6

add22:
	bra	_cxnrm4dale		; Normalize

add23:
	move.l	d6,d0
	restore
	bra	_cxinf4dale	; Handle infinite

add24:
	restore
	bra	_CXIND4		; Handle indefinite


;*****
;*****	Add Operands with same exponent.
;*****

add30:
	add.l	d1,d0		; Add

;	cmp.l	#$01000000,d0	; Need to shift right?
;	blt.s	add51		; - no
	btst	#24,d0
	beq	newcxrtn

	addq.l	#1,d0		; Shift and round
	lsr.l	#1,d0

	add.w	D2,d6	; Compensate exponent

	cmp.w	#$7f80,d6	; Test overflow
	blt	newcxrtn

ovf1:
	move.l	d6,d0
	restore
	bra	_cxovf4dale	; - overflow

denormx:
	swap	d1		; Smaller value zero?
	bne.s	add42		; - no

;**	Y is zero.  Put X back together and return it.

	clr.w	d6		; return (X)
	swap	d0
	or.l	d6,d0
	bra	return


add43:
	tst.l	d0		; NAN?
	bne.s	add44		; - yes
	bra.s	add23		; Handle INF

;**	Y is denormalized.  Shift it left, and check if X
;**	is also denormalized.  If so, give X the same treatment.

add42:
	add.l	d1,d1		; Shift Y mantissa left 1
	bra.s	add46

add44:
	bra	cxnan		; Handle NAN

shiftcnthi:
	add.w	D2,d6
	bra.s	_cxrtn4dale	; Return larger operand

;****
;****	Handle unequal exponents
;****

;**	d0 = Mantissa X
;**	d1 = Mantissa Y
;**	d6 = sign X, exponent X
;**	d7 = sign Y, exponent Y
;**
;**	d0,d1 are word swapped.

add40:

;**	Swap operands if necessary so X is the larger
;**	operand.

	bgt.s	add41		; - X larger

	exg	d0,d1
	exg	d6,d7

;**	Since we now know X > Y, test X only for INF and NAN,
;**	and test Y only for denormalized number and zero.

add41:
	cmp.w	#$7f80,d6	; Wierd exponent?
	beq.s	add43		; - yes

	tst.w	d7		; Smaller value denormalized?
;	bne.s	add45		; - no
	beq.s	denormx

;**	Put hidden bits in normalized Y and/or X.

add45:
	eor.b	D2,d1	; Add hidden bit to smaller
	swap	d1		; Unswap mantissa

add46:
	eor.b	D2,d0	; Add hidden bit to larger
	swap	d0		; Unswap mantissa

;****
;****	Operand Denormalization.
;****

;**	d0 = larger mantissa
;**	d1 = smaller mantissa
;**	d6 = smaller operand sign, exponent
;**	d7 = smaller operand sign, exponent

;**	Compute shift count to compensate for difference
;**	in exponents.

add50:
	sub.w	D2,d6	; Adjust larger exponent

	sub.w	d6,d7		; Difference between exponents
	neg.w	d7
	lsr.w	#7,d7

	cmp.w	#24,d7		; Shift count too high?
;	ble.s	add52		; - no
	bgt.s	shiftcnthi

;**	Add guard bit to larger operand.

add52:
	add.l	d0,d0		; Shift left 1

;**	Shift smaller operand right.

	lsr.l	d7,d1		; Shift right

;**	Check whether we add or subtract.

	eor.l	d6,d7		; Signs equal?
	bmi.s	add70		; - subtract operands


;*****
;*****	Add operands with different exponents.
;*****

add60:
	add.l	d1,d0		; Do the add

	addq.l	#1,d0		; start round process
	btst	#25,d0		; need two shifts?
	bne.s	add61
;	cmp.l	#$01ffffff,d0	; Need 2 shifts?
;	bge.s	add61		; - yes

;	addq.l	#1,d0		; Shift and round
	lsr.l	#1,d0

	add.w	D2,d6		; Adjust exponent
_cxrtn4dale:
;	tst.w	d6		; Normalized?
	beq.s	rtn1		; - nope

;**	Return normalized value

	swap	d0		; Hide hidden bit
	eor.b	D2,d0

	DOIT

	swap	d0

	restore
	rts

;**	Return denormalized value

rtn1:
	addq.l	#1,d0		; Shift, round, and add sign bit
	add.l	d6,d6
	roxr.l	#1,d0

	restore
	rts

add61:
;	addq.l	#2,d0		; Shift and round
	addq.l	#1,d0		; Shift and round, need another
	lsr.l	#2,d0

	add.w	#2*$80,d6	; Adjust exponent

	cmp.w	#$7f80,d6	; Overflow?
;	blt.s	add51		; - no
;	blt.s	_cxrtn4dale	; - no
	bge.s	ovf
newcxrtn:
	tst.w	d6
	beq.s	rtn1		; - nope

;**	Return normalized value

	swap	d0		; Hide hidden bit
	eor.b	D2,d0

	DOIT

	swap	d0

	restore
	rts


ovf:
	move.l	d6,d0
	restore
	bra	_cxovf4dale	; - yes


;*****
;*****	Subtract operands with different exponents.
;*****

add70:
	subx.l	d7,d7		; Remember X flag

	subx.l	d1,d0		; Subtract mantissas

;	cmp.l	#$01000000,d0	; Need to shift right?
;	blt	add22		; - no
;	blt	_cxnrm4dale		; Normalize
	btst	#24,d0
	beq	_cxnrm4dale

	sub.l	d7,d0		; Shift and round
	lsr.l	#1,d0

	add.w	d2,d6		; Adjust exponent
;	bra	_cxrtn4dale	; Return value
	beq.s	rtn1		; - nope

;**	Return normalized value

	swap	d0		; Hide hidden bit
	eor.b	D2,d0

	DOIT

	swap	d0

	restore
	rts

	ENDC

	end
