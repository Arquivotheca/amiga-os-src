*******************************************************************************
*
*	$Id: fix.asm,v 40.1 93/03/11 17:21:23 mks Exp $
*
*******************************************************************************

********************************************************************************
*
* If not FPU-Only, we need this...
*
	IFND	CPU_FPU
*

******* mathieeesingbas.library/IEEESPFix ***********************************
*
*   NAME
*	IEEESPFix -- convert IEEE single float to integer
*
*   SYNOPSIS
*	x   = IEEESPFix(  y  );
*	d0		 d0
*
*	long	x;
*	float	y;
*
*   FUNCTION
*	Convert IEEE single precision argument to a 32 bit signed integer
*	and return result.
*
*   INPUTS
*	y -- IEEE single precision floating point value
*
*   RESULT
*	if no overflow occured then return
*		x -- 32 bit signed integer
*	if overflow return largest +- integer
*		For round to zero
*
*   BUGS
*
*   SEE ALSO
*	IEEESPFlt
******************************************************************************
;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8 bk=0 ma=1


	include	"float.i"


	xref	_CXFERR		; Exception handler



;***********************************************************
;*	Floating point to signed long conversion           *
;***********************************************************

;**	Entry:
;**		d0 = single operand
;**	Exit:
;**		d0 = ulong operand
;**	Uses:
;**		a0,a1

	xdef	MIIEEESPFix
MIIEEESPFix:
;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8  bk=0  ma=1

	xref	_CXFERR		; User Exception routine


;**	Entry:
;**		d0 = single operand
;**	Exit:
;**		d0 = ulong result
;**	Uses:
;**		a0,d1

	move.l	d2,a0		; Save d2

;**	Unpack FP number

	move.l	#$7fffffff,d2	; Get largest positive int

	move.l	d0,d1		; Get sign
	bpl.s	ftoi1		; - positive

	addq.l	#1,d2		; Get largest negative int
	eor.l	d2,d0		; Make operand positive

ftoi1:
	swap	d0		; Get exponent
	move.w	d0,d1
	and.w	#$7f80,d1
	eor.w	d1,d0

	sub.w	#$3f80,d1	; Remove exponent bias
	blt.s	ftoi2		; - result is zero

	eor.w	#$0080,d0	; Add hidden bit
	swap	d0

;**	Shift upper mantissa right, if possible to form
;**	integer result.

	asr.w	#7,d1		; Form left shift count
	sub.w	#23,d1
	bgt.s	ftoi5		; - shift left

	neg.w	d1		; Form right shift count

	lsr.l	d1,d0		; Shift right 0-20

	tst.l	d1		; Check result sign
	bmi.s	ftoi6		; - negative
	bra.s	ftoi7		; - positive

;**	Operand was less than 1, result is zero.

ftoi2:
	moveq	#0,d0		; Zero result
	bra.s	ftoi7

;**	Operand is indefinite, infinite, or larger
;**	than would fit in 32 bits.

ftoi3:
	cmp.w	#$69,d1		; Infinite or indefinite?
	blt.s	ftoi4		; - no

	tst.l	d0		; Indefinite?
	beq.s	ftoi4		; - no

	fptrap	4		; Indefinite exception

	moveq	#0,d0		; Return zero
	bra.s	ftoi7

ftoi4:
	fptrap	2		; Overflow exception

	move.l	d2,d0		; Get overflow value
	bra.s	ftoi7

;**	Shift entire mantissa left to form result, then return
;**	only the upper 32 bits.

ftoi5:
	cmp.w	#8,d1		; Value too large?
	bgt.s	ftoi3		; - yes

	lsl.l	d1,d0		; Shift left 1-10

	cmp.l	d2,d0		; Overflow?
	bhi.s	ftoi4		; - yes

	tst.l	d1		; Check sign of result
	bpl.s	ftoi7		; - positive

;**	Negate operand and return.

ftoi6:
	neg.l	d0		; Make result negative

ftoi7:
	move.l	a0,d2		; Restore d2
	rts

	ENDC

	end
