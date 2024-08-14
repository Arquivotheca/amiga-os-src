
*******************************************************************************
*
*	$Header:
*
*******************************************************************************
******* mathieeedoubbas.library/IEEEDPCmp ***********************************
*
*   NAME
*	IEEEDPCmp -- compare two double precision floating point numbers
*
*   SYNOPSIS
*	  c   = IEEEDPCmp(  y  ,  z  );
*	  d0		  d0/d1 d2/d3
*
*	double	y,z;
*	long	c;
*
*   FUNCTION
*	Compare y with z. Set the condition codes for less, greater, or
*	equal. Set return value c to -1 if y<z, or +1 if y>z, or 0 if
*	y == z.
*
*   INPUTS
*	y -- IEEE double precision floating point value
*	z -- IEEE double precision floating point value
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

	xdef	MIIEEEDPCmp


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

MIIEEEDPCmp:

;	Test operand signs to break up the compare into
;	(ultimately) 4 cases.

	tst.l	d0		; Test sign of X
	bmi.s	cmp3		; - negative

	tst.l	d2		; Test sign of Y
	bpl.s	cmp1		; - postive

;	X >= +0 and Y <= -0.  X > Y unless X == +0 and Y == -0.

	tst.l	d0		; X == +0 ?
	bne.s	retplus		; - no
	tst.l	d1
	bne.s	retplus		; - no

	tst.l	d3		; Y == -0 ?
	bne.s	retplus		; - no
;	faster code, we know that d2 is negative
;	cmp.l	#$80000000,d2
	move.l	d2,d0		; d0 can be used as scratch
	add.l	d0,d0
	bne.s	retplus		; - no

	moveq	#0,d0
	rts			; return zero

;	X and Y are both positive.  Do a regular compare.

cmp1:
	cmp.l	d2,d0		; Compare MSB
	bmi	retminus
	bgt	retplus

	cmp.l	d1,d3		; Compare LSB
	bcs.s	retplus		; - X > Y
	bne.s	retminus
	moveq	#0,d0
	rts			; return 0

;	X is negative, test Y.

cmp3:
	tst.l	d2		; Test sign of Y
	bmi.s	cmp4		; - also negative

;	X <= -0 and Y >= +0.  So X < Y unless X == -0 and Y == +0.

				; Y == +0 ?
	bne.s	retminus		; - no
	tst.l	d3
	bne.s	retminus		; - no

	tst.l	d1		; X == -0 ?
	bne.s	retminus		; - no
;	we know that the msb of d0.l is set, based on previous
;	comparisons, so all we really need to do is check for the
;	next 31 bits being zero
;	cmp.l	#$80000000,d0		; old code
;	asl.l	#1,d0			; new code
	add.l	d0,d0			; newer code
	bne.s	retminus		; - no

	moveq	#0,d0
	rts			; return zero

;	X and Y are negative.  Do a reverse signed compare.

cmp4:
	cmp.l	d0,d2		; compare MSB
	bmi	retminus
	bgt	retplus

	cmp.l	d3,d1		; compare LSB
	bcs.s	retplus		; - X > Y
	bmi	retminus
	bgt	retplus
	moveq	#0,d0
	rts

	end
