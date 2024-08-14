
*******************************************************************************
*
*	$Header:
*
*******************************************************************************
******* mathieeedoubbas.library/IEEEDPFlt ***********************************
*
*   NAME
*	IEEEDPFlt -- convert integer to IEEE double precision number
*
*   SYNOPSIS
*	  x   = IEEEDPFlt(  y  );
*	d0/d1		   d0
*
*	double	x;
*	long	y;
*
*   FUNCTION
*	Convert a signed 32 bit value to a double precision IEEE value
*	and return it in d0/d1. No exceptions can occur with this
*	function.
*
*   INPUTS
*	y -- 32 bit integer in d0
*
*   RESULT
*	x is a 64 bit double precision IEEE value
*
*   BUGS
*
*   SEE ALSO
*	IEEEDPFix
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
;**		d0,d1 = double operand
;**	Uses:
;**		a1

	xdef	MIIEEEDPFlt
MIIEEEDPFlt:
	movem.l	d2-d5,-(a7)	; Save d2-d5

	moveq	#0,d4		; Set result sign
	moveq	#0,d1		; Clear LSB mantissa

	tst.l	d0		; Test operand
	beq.s	itod3		; - zero
	bpl.s	itod1		; - positive

	move.w	#$8000,d4	; Set sign negative
	neg.l	d0		; Make mantissa positive

itod1:
	cmp.l	#$200000,d0	; Need to denormalize 16 bits?
	bcc.s	itod2		; - no

	move.w	#$4120,d5	; Set exponent

	bsr	_CXNRM5		; Normalize
	
	movem.l	(a7)+,d2-d5	; Restore d2-d5
	rts

itod2:
	move.w	d0,d1		; Shift into position
	clr.w	d0
	swap	d0
	swap	d1

	move.w	#$4220,d5	; Set exponent

	bsr	_CXNRM5		; Normalize

itod3:
	movem.l	(a7)+,d2-d5	; Restore d2-d5
	rts

	end
