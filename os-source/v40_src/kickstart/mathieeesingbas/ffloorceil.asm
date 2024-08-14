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
* $Id: ffloorceil.asm,v 40.2 93/03/09 10:49:13 mks Exp $
*
**********************************************************************

	xdef	MIIEEESPFloor,MIIEEESPCeil

*
* general equates
*
sign	equ	$80000000	sign bit mask
signb	equ	31		sign bit no.
notsign	equ	$7fffffff	everything but sign
zero	equ	$00000000	IEEE single precision 0.0
one	equ	$3F800000	IEEE single precision 1.0 (lower half)
neg_one	equ	one+sign	negative one


***************************************************
***************************************************
***						***
***  DOUBLE PRECISION C ENTRY POINTS BEGIN HERE	***
***						***
***************************************************
***************************************************

* new code from Dale
* parameters are in registers

	xref	_LVOIEEEDPAdd
	xref	_LVOIEEEDPFloor

******* mathieeesingbas.library/IEEESPCeil ***********************************
*
*   NAME
*	IEEESPCeil -- compute Ceil function of IEEE single precision number
*
*   SYNOPSIS
*	  x   = IEEESPCeil(  y  );
*	 d0		     d0
*
*	float	x,y;
*
*   FUNCTION
*	Calculate the least integer greater than or equal to x and return it.
*	This identity is true.  Ceil(x) = -Floor(-x).
*
*   INPUTS
*	y -- IEEE single precision floating point value
*
*   RESULT
*	x -- IEEE single precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEESPFloor
******************************************************************************

MIIEEESPCeil:
	bchg	#31,d0
	jsr	_LVOIEEEDPFloor(a6)
	tst.l	d0	; don't turn 0.0 into -0.0
	beq.s	QUICKZ
		bchg	#31,d0
QUICKZ:	rts

nearzero:
	tst.l	d1
	bge.s	1$
		move.l	#neg_one,d0	; return -1.0
		bra.s	2$
1$:
		clr.l	d0	; return 0.0
2$:
	rts

******* mathieeesingbas.library/IEEESPFloor ***********************************
*
*   NAME
*	IEEESPFloor -- compute Floor function of IEEE single precision number
*
*   SYNOPSIS
*	  x   = IEEESPFloor(  y  );
*	  d0		      d0
*
*	float	x,y;
*
*   FUNCTION
*	Calculate the largest integer less than or equal to x and return it.
*
*   INPUTS
*	y -- IEEE single precision floating point value
*
*   RESULT
*	x -- IEEE single precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEESPCeil
******************************************************************************

MIIEEESPFloor:
	move.l	d0,d1		; is it a zero? and save in d1 for later
	beq	QUICKZ		; yup, just return
* check exponent, are there any fraction bits?
	swap	d0
	and.w	#$7f80,d0	; extract exponent
	beq	nearzero	; unnormalized?
	asr.w	#7,d0		; align to work with it
	sub.w	#127,d0		; unbias it
	blt	nearzero	; range is -1.0 < x < 1.0
* d1 is positive or 0, it tells about how many bits are to the left
* of binary point, so 0 means 1 bit left of binary point
* We have 24 total bits
* If d1 >= 24 then there are no fraction bits

	cmp.w	#23,d0	; if #23>d0.w	; any fraction bits to eliminate?
	bge.s	3$
*		d1 is 0 <= d1 < 23
*       	0 means clean out all 23 bits, leaving the hidden bit
*       	1 means clean out 22 bits
* 		there are some fraction bits, we eliminate these now
* 		if there were no fraction bits then we can just return with
* 		the original value
		neg.w	d0
		add.w	#23,d0	; how many bits to clear
				; starting from right end
*		need to save current values for later comparison
		move.l	d1,a1
		lsl.w	#2,d0
		and.l	table(PC,d0.w),d1
		move.l	d1,d0
*		have to check sign bit to round down
		bge.s	4$	; if <	; was it negative?

*			did we really change anything?
			cmp.l	d1,a1	; if d1=a1.l
			bne.s	5$
				rts
5$:					; endif
*			something changed, so we have to decrement it by -1
			move.l	#neg_one,d1
*			now add them, rounding to -infinity
			jmp	_LVOIEEEDPAdd(a6)
4$:				; endif
	bra.s	6$	; else
3$:
		move.l	d1,d0		; restore original input
6$:			; endif
	rts

table:				;used to clear fraction bits
	dc.l	$ffffffff
	dc.l	$fffffffe
	dc.l	$fffffffc
	dc.l	$fffffff8
	dc.l	$fffffff0
	dc.l	$ffffffe0
	dc.l	$ffffffc0
	dc.l	$ffffff80
	dc.l	$ffffff00
	dc.l	$fffffe00

	dc.l	$fffffc00
	dc.l	$fffff800
	dc.l	$fffff000
	dc.l	$ffffe000
	dc.l	$ffffc000
	dc.l	$ffff8000
	dc.l	$ffff0000
	dc.l	$fffe0000
	dc.l	$fffc0000
	dc.l	$fff80000

	dc.l	$fff00000
	dc.l	$ffe00000
	dc.l	$ffc00000
	dc.l	$ff800000
*	don't need any more bits
      end

