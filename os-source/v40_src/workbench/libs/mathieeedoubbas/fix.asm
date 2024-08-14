
*******************************************************************************
*
*	$Id: fix.asm,v 38.1 92/01/24 12:01:27 mks Exp $
*
*******************************************************************************

******* mathieeedoubbas.library/IEEEDPFix ***********************************
*
*   NAME
*	IEEEDPFix -- convert IEEE double float to integer
*
*   SYNOPSIS
*	x   = IEEEDPFix(  y  );
*	d0		d0/d1
*
*	long	x;
*	double	y;
*
*   FUNCTION
*	Convert IEEE double precision argument to a 32 bit signed integer
*	and return result.
*
*   INPUTS
*	y -- IEEE double precision floating point value
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
*	IEEEDPFlt
******************************************************************************
;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8 bk=0 ma=1

	idnt	CXV53

	include	"float.i"

	xdef	_CXV53		; Convert double to long

	xref	_CXFERR		; Exception handler



;***********************************************************
;*	Floating point to signed long conversion           *
;***********************************************************

;**	Entry:
;**		d0,d1 = double operand
;**	Exit:
;**		d0 = ulong operand
;**	Uses:
;**		a0,a1

	xdef	MIIEEEDPFix
MIIEEEDPFix:
_CXV53:
	move.l	d2,a0		; Save d2
	move.l	d3,a1		; Save d3

;**	Unpack FP number

	move.l	#$7fffffff,d3	; Set largest positive value

	move.l	d0,d2		; Get sign
	bpl.s	dtoi1		; - positive

	addq.l	#1,d3		; Set largest negative value
	eor.l	d3,d0		; Make positive

dtoi1:
	swap	d0		; Get exponent
	move.w	d0,d2
	and.w	#$7ff0,d2
	eor.w	d2,d0

	sub.w	#$3ff0,d2	; Remove exponent bias
	blt.s	dtoi2		; - result is zero

	eor.w	#$0010,d0	; Add hidden bit
	swap	d0

;**	Shift upper mantissa right, if possible to form
;**	integer result.

	asr.w	#4,d2		; Form left shift count
	sub.w	#20,d2
	bgt.s	dtoi4		; - shift left

	neg.w	d2		; Form right shift count

	lsr.l	d2,d0		; Shift right 0-20

	tst.l	d2		; Check result sign
	bmi.s	dtoi5		; - negative
	bra.s	dtoi6		; - positive

;**	Operand was less than 1, result is zero.

dtoi2:
	moveq	#0,d0		; Zero result
	bra.s	dtoi6

;**	Operand was greater than would fit in 32 bits.
;**	Return largest possible integer.

dtoi3:
	fptrap	2		; Overflow trap

	move.l	d3,d0		; Get largest value
	bra.s	dtoi6

;**	Shift entire mantissa left to form result, then return
;**	only the upper 32 bits.

dtoi4:
	cmp.w	#11,d2		; Exponent out of range?
	bgt.s	dtoi3		; - yes

	eor.l	d1,d0		; Shift left 1-11
	rol.l	d2,d0
	lsl.l	d2,d1
	eor.l	d1,d0

	cmp.l	d3,d0		; Overflow?
	bhi.s	dtoi3		; - yes

	tst.l	d2		; Check result sign
	bpl.s	dtoi6		; - positive

;**	Set result sign and return.

dtoi5:
	neg.l	d0		; Make result negative

dtoi6:
	move.l	a0,d2		; Restore d2
	move.l	a1,d3		; Restore d3
	rts

	end
