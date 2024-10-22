	TTL    '$Id: spsqrt.asm,v 37.1 91/01/21 15:18:53 mks Exp $'
**********************************************************************
*								     *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.	     *
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030				     *
*								     *
**********************************************************************
*
*   $Id: spsqrt.asm,v 37.1 91/01/21 15:18:53 mks Exp $
*
**********************************************************************

; new algorithm from "Collected Algorithms of the ACM"
; Algorthim 650: Efficient Square Root Implementation
; on the 68000
; Kenneth C. Johnson
; sorry, I did not type in the comments if there were any.
; You will need to look it up yourself if there are problems.

*This is the final version of the DPSqrt(x) routine.  I decided this is it
*as further attempts to 'improve' the run time results in loss of accuracy
*for special situations ( such as sqrt(1.999998) ). Well, I could handle it
*Ok but it would require extra logic and code which I don't (now) think is
*worth the effort. Anyway I'm very happy with the code below.  I hope you
*can find time to check it out and if Ok include in the Library.
*
*                  Run Times In Microseconds.
*                  Alpha2      New Routine
*DPSqrt(5.0)       4383        1147
*DPSqrt(0.999998)  4441        1070
*DPSqrt(2**N)      4318         250  (N ODD  Integer)
*DPSqrt(2**N)      4211          56  (N EVEN Integer)
*.More..
*DPSqrt(1.0)       4213          48
* V1.3 sqrt		       1032
* V1.4/ACM algorithm		785 (all numbers)
* V1.4/partial loop unrolling
* and register ping-pong        718 (all numbers)
*
*Al Aburto
**************************** DPSqrt(D0:D1) ******************************
************ IEEE Double Precision Square Root Routine.
************ Input:  D0:D1    ,x
************ Output: D0:D1    ,SQRT( x )
************ Bugs: Just Returns 0.0 When x < 0.0 . Routine Should Cause
************       Some Sort Of Error Report, or Fault, In This Case.
* Originally written by Al Aburto, adapted to the amiga library
* by Dale Luck. This code here is provided under non-exclusive license
* to Commodore-Amiga, Inc.

******* mathieeedoubtrans.library/IEEEDPSqrt ********************************
*
*   NAME
*	IEEEDPSqrt -- compute the square root of a number
*
*   SYNOPSIS
*	  x   = IEEEDPSqrt(  y  );
*	d0/d1 	           d0/d1
*
*	double	x,y;
*
*   FUNCTION
*	Compute square root of y in IEEE double precision
*
*   INPUTS
*	y - IEEE double precision floating point value
*
*   RESULT
*	x - IEEE double precision floating point value
*
*   BUGS
*
*   SEE ALSO
**************************************************************************/

; original CACM code did not handle nonnormalized input
nonnorm:
;	normalize the argument
;	construct exponent in d3.w
	moveq	#$10,d6
	moveq	#20,d7
	bclr	d7,d0
	move.l	d6,d3		; start at 1
	repeat
		sub.w	d6,d3	; decrement exp for each shift left
		add.l	d1,d1
		addx.l	d0,d0
		btst	d7,d0
	until <>
	swap	d3		; put exponent in hiword
	bra reenter


dollar6:
	tst.l	d1
	bne	nonnorm
;	arg = 0.0 just return with 0.0
	bra	exit


negarg:
;	need to signal some sort of error
;	we just return so sqrt(-0.0) = -0.0
	tst.l	d1
	if =
		add.l	d0,d0
		beq	exit	; return with 0.0 for result
	endif
;	construct NAN
	clr.l	d1
	move.l	#$7ff10000,d0
nan:			; just return nan that was input
	bra exit


;	_sqrt used by other trans routines locally

	xdef	_sqrt
	xdef	MIIEEEDPSqrt
_sqrt:
	move.l	4(sp),d0	; get dp arg from stack
	move.l  8(sp),d1
*	fall into rest of routine

MIIEEEDPSqrt:
	movem.l	d4-d7,-(sp)
	move.l	d2,a0		; use a0/a1 to save data registers
	move.l	d3,a1
	; uses d0-d7
dpsqrt:
;	move.l	d3,d1		; d1 = lslw of arg
;	move.l	d2,d0		; d0 = mslw of arg
	move.l	#$7ff00000,d2		; mask for exp
	tst.l	d0
	beq	dollar6			; to $6 if mslw = 0
	bmi	negarg			; sign bit?
	move.l	d0,d3			; use d3 to store exp
	and.l	d2,d3			; clear fraction bits

	beq	nonnorm			; exp is zero?

	cmp.l	d2,d3
	beq	nan			; NAN or INF?

	sub.l	d3,d0
	bset	#20,d0

reenter:
	add.l	#$3ff00000,d3
	lsr.l	#1,d3

	moveq	#19,d2

	bclr	d2,d3			; exponent even?
	beq.s	dollar0			; to $0 if even
	add.l	d1,d1
	addx.l	d0,d0			; double x if m is odd
dollar0:
	move.b	#20,d3			; 20->Low Byte
					; exponent is in high word
	moveq	#-1,d5
	move.l	#$000fffff,d4
	sub.l	d5,d1
	subx.l	d4,d0
	bset	#21,d4

FAST	equ 1

	ifnd	FAST
dollar1:
	add.l	d1,d1
	addx.l	d0,d0
	bclr	d2,d4
	move.l	d0,d6
	move.l	d1,d7
	sub.l	d5,d7
	subx.l	d4,d6
	bls.s	dollar2

	move.l	d6,d0
	move.l	d7,d1
	bset	d3,d4
dollar2:
	move.b	d2,d3
	subq.b	#1,d2

	bcc	dollar1
	endc

	ifd	FAST
dollar1:
	add.l	d1,d1
	addx.l	d0,d0
	bclr	d2,d4
	move.l	d0,d6
	move.l	d1,d7
	sub.l	d5,d7
	subx.l	d4,d6
	bls.s	dollar2

	move.l	d6,d0
	move.l	d7,d1
	bset	d3,d4
dollar2:
	subq.b	#2,d3

;	The next part is identical to above part except
;	d2 and d3 usage have been swapped

	add.l	d1,d1
	addx.l	d0,d0
	bclr	d3,d4
	move.l	d0,d6
	move.l	d1,d7
	sub.l	d5,d7
	subx.l	d4,d6
	bls.s	dollar20

	move.l	d6,d0
	move.l	d7,d1
	bset	d2,d4
dollar20:
	subq.b	#2,d2

	bcc	dollar1
	endc

;	do part that crosses boundary between upper and lower half
	moveq	#31,d2

	add.l	d1,d1
	addx.l	d0,d0
	bclr	d2,d5
	move.l	d0,d6
	move.l	d1,d7
	sub.l	d5,d7
	subx.l	d4,d6
	bls.s	dollar3

	move.l	d6,d0
	move.l	d7,d1
	bset	d3,d4

dollar3:
	move.b	d2,d3
	subq.b	#1,d2

	ifnd	FAST
dollar4:
	add.l	d1,d1
	addx.l	d0,d0
	bclr	d2,d5
	move.l	d0,d6
	move.l	d1,d7
	sub.l	d5,d7
	subx.l	d4,d6
	bls.s	dollar5

	move.l	d6,d0
	move.l	d7,d1
	bset	d3,d5

dollar5:
	move.b	d2,d3
	subq.b	#1,d2

	bcc.s	dollar4
	endc

	ifd	FAST
;	first unroll once to even up loop count
	add.l	d1,d1
	addx.l	d0,d0
	bclr	d2,d5
	move.l	d0,d6
	move.l	d1,d7
	sub.l	d5,d7
	subx.l	d4,d6
	bls.s	dollar05

	move.l	d6,d0
	move.l	d7,d1
	bset	d3,d5

dollar05:
	move.b	d2,d3
	subq.b	#1,d2
;	Now 30 more times, but lets unroll and only go 15 times
;	By doing two of the operations each time through we
;	can ping pong two registers and save more time.
dollar4:
	add.l	d1,d1
	addx.l	d0,d0
	bclr	d2,d5
	move.l	d0,d6
	move.l	d1,d7
	sub.l	d5,d7
	subx.l	d4,d6
	bls.s	dollar50

	move.l	d6,d0
	move.l	d7,d1
	bset	d3,d5

dollar50:
	subq.b	#2,d3

	add.l	d1,d1
	addx.l	d0,d0
	bclr	d3,d5
	move.l	d0,d6
	move.l	d1,d7
	sub.l	d5,d7
	subx.l	d4,d6
	bls.s	dollar5

	move.l	d6,d0
	move.l	d7,d1
	bset	d2,d5

dollar5:
;	move.b	d2,d3
	subq.b	#2,d2

	bcc.s	dollar4

	endc

	lsr.l	#1,d4
	roxr.l	#1,d5
	move.l	d4,d0
	bclr	#20,d0
	or.l	d3,d0
	move.l	d5,d1
exit:
	movem.l	(sp)+,d4-d7	; restore registers
	move.l	a0,d2
	move.l	a1,d3
	rts

	end
