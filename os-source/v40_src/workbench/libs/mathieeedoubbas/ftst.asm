
*******************************************************************************
*
*	$Header:
*
*******************************************************************************
******* mathieeedoubbas.library/IEEEDPTst ***********************************
*
*   NAME
*	IEEEDPTst -- compare IEEE double precision value to 0.0
*
*   SYNOPSIS
*	  c   = IEEEDPTst(  y  );
*	  d0		  d0/d1
*
*	double	y;
*	long	c;
*
*   FUNCTION
*	Compare y to 0.0, set the condition codes for less than, greater
*	than, or equal to 0.0.  Set the return value c to -1 if less than,
*	to +1 if greater than, or 0 if equal to 0.0.
*
*   INPUTS
*	y -- IEEE double precision floating point value
*
*   RESULT
*	c = 1	cc = gt		for (y > 0.0)
*	c = 0	cc = eq		for (y == 0.0)
*	c = -1  cc = lt		for (y < 0.0)
*
*   BUGS
*
*   SEE ALSO
******************************************************************************
	include	"float.i"

	xdef	MIIEEEDPTst		; Double test against zero

;***********************************************************
;	Test value against zero.                           *
;***********************************************************
;       Copyright 1986, Gene H. Olson
;       All Rights Reserved.


MIIEEEDPTst:
;	asl.l	#1,d0		; test a bunch of things at once
	add.l	d0,d0		; test a bunch of things at once, faster
	bcs.s	retminus	; was msb set? then negative
;				; else must be nonnegative
	bne.s	retplus		; any non zero bits left?

	tst.l	d1		; LSB zero?
	beq.s	retzero		; - yes, return zero

retplus:
	moveq	#1,d0		; return 1
	rts

tst1:				; possible -0.0
	tst.l	d1		; LSB zero?
	beq.s	retzero		; - yes, return zero, for negative zero
	moveq	#-1,d0		; return -1, it was not negative zero
	rts

retminus:
;				; now check for rare negative zero
;	cmp.l	#$80000000,d0	; possible Negative zero?
;				; now check other bits set by asl.l #1
	beq.s	tst1		;

	moveq	#-1,d0		; return -1
	rts

retzero:
	clr.l	d0		; return 0
	rts

	end
