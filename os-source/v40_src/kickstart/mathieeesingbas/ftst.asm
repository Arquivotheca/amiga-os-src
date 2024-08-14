
*******************************************************************************
*
*	$Header:
*
*******************************************************************************
******* mathieeesingbas.library/IEEESPTst ***********************************
*
*   NAME
*	IEEESPTst -- compare IEEE single precision value to 0.0
*
*   SYNOPSIS
*	  c   = IEEESPTst(  y  );
*	  d0		    d0
*
*	float	y;
*	long	c;
*
*   FUNCTION
*	Compare y to 0.0, set the condition codes for less than, greater
*	than, or equal to 0.0.  Set the return value c to -1 if less than,
*	to +1 if greater than, or 0 if equal to 0.0.
*
*   INPUTS
*	y -- IEEE single precision floating point value
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

	xdef	MIIEEESPTst		; Double test against zero

;***********************************************************
;	Test value against zero.                           *
;***********************************************************
;       Copyright 1986, Gene H. Olson
;       All Rights Reserved.


MIIEEESPTst:
;	asl.l	#1,d0		; test a bunch of things at once
	add.l	d0,d0		; test a bunch of things at once, faster
	beq.s	retzero		; - yes, return zero
	bcs.s	retminus
	moveq	#1,d0
retzero:
	rts

retminus:
	moveq	#-1,d0
	rts

	end
