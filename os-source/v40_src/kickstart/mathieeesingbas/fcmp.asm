
*******************************************************************************
*
*	$Header:
*
*******************************************************************************
******* mathieeesingbas.library/IEEESPCmp ***********************************
*
*   NAME
*	IEEESPCmp -- compare two single precision floating point numbers
*
*   SYNOPSIS
*	  c   = IEEESPCmp(  y  ,  z  );
*	  d0		    d0   d1
*
*	float	y,z;
*	long	c;
*
*   FUNCTION
*	Compare y with z. Set the condition codes for less, greater, or
*	equal. Set return value c to -1 if y<z, or +1 if y>z, or 0 if
*	y == z.
*
*   INPUTS
*	y -- IEEE single precision floating point value
*	z -- IEEE single precision floating point value
*
*   RESULT
*       c = 1   cc = gt         for (y > z)
*       c = 0   cc = eq         for (y == z)
*       c = -1  cc = lt         for (y < z)
*
*   BUGS
*
*   SEE ALSO
******************************************************************************

;	Copyright 1986, Gene H. Olson
;	All Rights Reserved.

;	:ts=8 bk=0 ma=1

	include	"float.i"

	xdef	MIIEEESPCmp


;***************************************************************
;*	Compare function                                       *
;***************************************************************

;**	Entry:
;**		d0,d1 = left operand
;**		d2,d3 = right operand
;**	Exit:
;**		Condition codes set.
;**	Uses:
;**		None.

retplus:	moveq	#1,d0
		rts
retminus:	moveq	#-1,d0
		rts

MIIEEESPCmp:

	tst.l	d0		; Test sign of X
	bmi.s	cmp3		; - negative

	tst.l	d1		; Test sign of Y
	bmi.s	cmp1		; - negative

;**	X >= 0 and Y >= 0.  Do a regular compare.

	cmp.l	d1,d0		; Do a regular compare
	bgt.s	retplus
	blt.s	retminus
retzero:
	clr.l	d0		; return 0 /* equal */
	rts

;**	X >= +0 and Y <= -0.  Return X > Y unless X == 0
;**	and Y == -0.

cmp1:
	tst.l	d0		; Test X == 0
	bne.s	retplus

;	cmp.l	#$80000000,d1	; Test Y == -1
	add.l	d1,d1
	bne.s	retplus

	moveq	#0,d0
	rts

;**	X < 0.

cmp3:
	tst.l	d1		; Test sign of Y
	bmi.s	cmp5		; - negative

;**	X <= -0 and Y >= +0.  Return X < Y unless X == -0
;**	and Y == +0.

	bne.s	retminus

;	cmp.l	#$80000000,d0	; Test X == -0
	add.l	d0,d0
	bne.s	retminus
	moveq	#0,d0
	rts


;**	X <= -0 and Y <= -0.  Reverse the sense of a regular
;**	2's complement compare.

cmp5:
	cmp.l	d0,d1		; Compare for CC
	bgt.s	retplus
	blt.s	retminus
	clr.l	d0
	rts

	end
