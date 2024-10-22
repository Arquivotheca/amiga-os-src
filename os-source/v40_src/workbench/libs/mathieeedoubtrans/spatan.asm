
	TTL    '$Id: spatan.asm,v 37.1 91/01/21 15:18:48 mks Exp $'
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
*   $Id: spatan.asm,v 37.1 91/01/21 15:18:48 mks Exp $
*
**********************************************************************
******* mathieeedoubtrans.library/IEEEDPAtan ********************************
*
*   NAME
*	IEEEDPAtan -- compute the arctangent of a floating point number
*
*   SYNOPSIS
*	  x   = IEEEDPAtan(  y  );
*	d0/d1		   d0/d1
*
*	double	x,y;
*
*   FUNCTION
*	Compute arctangent of y in IEEE double precision
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
*	IEEEDPTan(), IEEEDPAsin(), IEEEDPACos()
**************************************************************************

	xref	_LVOIEEEDPTst,_LVOIEEEDPCmp
	xref	_LVOIEEEDPAdd,_LVOIEEEDPSub
	xref	_LVOIEEEDPMul,_LVOIEEEDPDiv

	xref	_LVOIEEEDPSqrt

	xref	_MathIeeeDoubBasBase
	xref	_MathIeeeDoubTransBase

	xdef	_atan
	xdef	MIIEEEDPAtan

one	equ	1072693248

	ifd	DEBUG
sdebug	macro
	move.l	d0,-(sp)
	move.l	d1,-(sp)
	addq.l	#8,sp
	endm
	endc

	ifnd	DEBUG
sdebug	macro
	endm
	endc

set_dbl	macro
\1_h	equ	\2
\1_l	equ	\3
	endm

	set_dbl pi8,1071194619,1413754136
	set_dbl PI2,1073291771,1413754136

	set_dbl f1,1078819454,41320649		;.589
	set_dbl f2,1082180127,2087422520	;.536
	set_dbl f3,1083837218,-1604433167	;.166
	set_dbl f4,1084243627,-2117779440	;.207
	set_dbl f5,1082918473,-1396607246	;.896

	set_dbl g1,1076897621,155135696		;.161
	set_dbl g2,1081132750,-970285327	;.268
	set_dbl g3,1083311134,240421346		;.115
	set_dbl g4,1083953568,292176290		;.178
	set_dbl g5,1082918473,-1396607246	;.896

	set_dbl h1,1077946197,155135696		;.161*2.0
	set_dbl h2,1082181326,-970285327	;.268*2.0
	set_dbl h3,1084359710,240421346		;.115*2.0
	set_dbl h4,1085002144,292176290		;.178*2.0
	set_dbl h5,1083967049,-1396607246	;.896*2.0

f0_times_equals_x_sq macro
	move.l	d4,d2
	move.l	d5,d3
	jsr	_LVOIEEEDPMul(a6)
	endm

f0_plus_equals macro
	move.l	#\1_h,d2
	move.l	#\1_l,d3
	jsr	_LVOIEEEDPAdd(a6)
	endm


_atan:
*	c interface
	movem.l	4(sp),d0/d1	;grab arg off stack
*	d2/d3 = x
*	d4/d5 = x_sq
MIIEEEDPAtan:
	movem.l	d2/d3/d4/d5/d6/d7/a6,-(sp)
	move.l	_MathIeeeDoubBasBase,a6
	move.l	d0,d2
	move.l	d1,d3

	jsr	_LVOIEEEDPTst(a6)
	move.w	d0,-(sp)			; input was negative
	if <
		bchg	#31,d2
	endif
	clr.l	d1
	move.l	#one,d0		; d0/d1 = one
	jsr	_LVOIEEEDPCmp(a6)
	move.w	d0,-(sp)			; negative means invert
	if <
		move.l	#one,d0
		clr.l	d1
		jsr	_LVOIEEEDPDiv(a6)	; generate 1.0/|x|
		move.l	d0,d2
		move.l	d1,d3
	endif
	move.l	d2,d0
	move.l	d3,d1
	jsr	_LVOIEEEDPMul(a6)		; x_sq = x*x
	move.l	d0,d4				; d4/d5 = x_sq
	move.l	d1,d5
	move.l	d2,d6				; d6/d7 = x
	move.l	d3,d7
	move.l	#pi8_h,d0
	move.l	#pi8_l,d1
	jsr	_LVOIEEEDPCmp(a6)
	if >	.extend			; if (x < pi/8)
		move.l	d4,d0			; f0 <- s_sq
		move.l	d5,d1
		f0_plus_equals	f1		; .589
		f0_times_equals_x_sq
		f0_plus_equals	f2		; .536
		f0_times_equals_x_sq
		f0_plus_equals	f3		; .166
		f0_times_equals_x_sq
		f0_plus_equals	f4		; .207
		f0_times_equals_x_sq
		f0_plus_equals	f5		; .896
		move.l	d0,d2			; f0
		move.l	d1,d3
		move.l	d6,d0			; x
		move.l	d7,d1
		jsr	_LVOIEEEDPDiv(a6)	; x/f0
		move.l	d0,d6
		move.l	d1,d7

		move.l	#g1_h,d0		; .161
		move.l	#g1_l,d1
		f0_times_equals_x_sq
		f0_plus_equals	g2		; .268
		f0_times_equals_x_sq
		f0_plus_equals	g3		; .115
		f0_times_equals_x_sq
		f0_plus_equals	g4		; .178
		f0_times_equals_x_sq
		f0_plus_equals	g5		; .896
	else	.extend
		move.l	d4,d0
		move.l	d5,d1
		move.l	#one,d2
		clr.l	d3
		jsr	_LVOIEEEDPAdd(a6)	; (x_sq+1)
		move.l	_MathIeeeDoubTransBase,a6
		jsr	_LVOIEEEDPSqrt(a6)
		move.l	_MathIeeeDoubBasBase,a6
		jsr	_LVOIEEEDPSub(a6)
		move.l	d6,d2
		move.l	d7,d3
		jsr	_LVOIEEEDPDiv(a6)
		move.l	d0,d6			; x = (sqrt(x_sq+1)-1)/x;
		move.l	d1,d7
		move.l	d0,d2
		move.l	d1,d3
		jsr	_LVOIEEEDPMul(a6)	; x_sq = x*x
		move.l	d0,d4
		move.l	d1,d5
		f0_plus_equals	f1		; .589
		f0_times_equals_x_sq
		f0_plus_equals	f2		; .536
		f0_times_equals_x_sq
		f0_plus_equals	f3		; .166
		f0_times_equals_x_sq
		f0_plus_equals	f4		; .207
		f0_times_equals_x_sq
		f0_plus_equals	f5		; .896

		move.l	d0,d2			; f0
		move.l	d1,d3
		move.l	d6,d0			; x
		move.l	d7,d1
		jsr	_LVOIEEEDPDiv(a6)	; x/f0
		move.l	d0,d6
		move.l	d1,d7

		move.l	#h1_h,d0		; .161
		move.l	#h1_l,d1
		f0_times_equals_x_sq
		f0_plus_equals	h2		; .268
		f0_times_equals_x_sq
		f0_plus_equals	h3		; .115
		f0_times_equals_x_sq
		f0_plus_equals	h4		; .178
		f0_times_equals_x_sq
		f0_plus_equals	h5		; .896
	endif
	move.l	d6,d2
	move.l	d7,d3
	jsr	_LVOIEEEDPMul(a6)
	if (sp)+<#0.w
		bchg	#31,d0			; make (-f0)
		move.l	#PI2_h,d2
		move.l	#PI2_l,d3
		jsr	_LVOIEEEDPAdd(a6)	; f0 = PI2 - f0;
	endif
	if (sp)+<#0.w
		bchg	#31,d0
	endif
	movem.l	(sp)+,d2/d3/d4/d5/d6/d7/a6
	rts

	end
