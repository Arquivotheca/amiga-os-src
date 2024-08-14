	TTL	'$Id: convert.asm,v 37.1 91/01/21 15:18:44 mks Exp $'

**********************************************************************
*                                                                    *
*   Copyright 1989, Commodore-Amiga Inc.   All rights reserved.      *
*   No part of this program may be reproduced, transmitted,          *
*   transcribed, stored in retrieval system, or translated into      *
*   any language or computer language, in any form or by any         *
*   means, electronic, mechanical, magnetic, optical, chemical,      *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*                                                                    *
**********************************************************************
*
*   $Id: convert.asm,v 37.1 91/01/21 15:18:44 mks Exp $
*
**********************************************************************

	include 'exec/types.i'

        section mathieeedoubtrans

iszero:	clr.l	d1
	rts

	xdef	MIIEEEDPTieee
*	convert double to single ieee
*	enter with d0/d1 exit with d0
******* mathieeedoubtrans.library/IEEEDPTieee ********************************
*
*   NAME
*	IEEEDPTieee -- convert IEEE double to IEEE single
*
*   SYNOPSIS
*	  x   = IEEEDPTieee(  y  );
*	 d0	            d0/d1
*
*	double	y;
*	float   x;
*
*   FUNCTION
*	Convert IEEE double precision number to IEEE single precision.
*
*   INPUTS
*	y - IEEE double precision floating point value
*
*   RESULT
*	x - IEEE single precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEEDPFieee()
**************************************************************************/
MIIEEEDPTieee:
	swap	d0
	beq	iszero
	move.w	d0,d1
	swap	d1
	swap	d0	; get this back

*	get three more bits
	asl.w	#1,d1
	roxl.l	#1,d0
	asl.w	#1,d1
	roxl.l	#1,d0
	asl.w	#1,d1
	roxl.l	#1,d0
	swap	d0
	and.w	#$007F,d0	; save mantissa bits
	swap	d1
	move.w	d1,a0		; save here
	and.w	#$8000,d1	; recall sign
	or.w	d1,d0		; reassemble
	move.w	a0,d1
	and.w	#$7ff0,d1	; extract exponent
	sub.w	#(1022-126)<<4,d1	; debias double/rebias single
*	check for outofbounds
	if <
*		number is very tiny, use zero
		clr.l	d0
		rts
	endif
	if #255<<4<d1.w
*		overflow
		ori	#2,ccr
		trapv
		or.l	#$FFFF7FFF,d0	; bignum
		swap	d0
		rts
	endif
	asl.w	#3,d1	; move it to proper place
	or.w	d1,d0
	swap	d0
	rts

******* mathieeedoubtrans.library/IEEEDPFieee ********************************
*
*   NAME
*	IEEEDPFieee -- convert IEEE single to IEEE double
*
*   SYNOPSIS
*	  x   = IEEEDPFieee(  y  );
*	d0/d1	              d0
*
*	float	y;
*	double  x;
*
*   FUNCTION
*	Convert IEEE single precision number to IEEE double precision.
*
*   INPUTS
*	y - IEEE single precision floating point value
*
*   RESULT
*	x - IEEE double precision floating point value
*
*   BUGS
*
*   SEE ALSO
*	IEEEDPTieee()
**************************************************************************/
	xdef	MIIEEEDPFieee
*	convert single to double ieee
*	enter with d0 exit with d0/d1
MIIEEEDPFieee:
	move.l	d0,a0		; save here
	swap	d0
	beq	iszero
	move.w	d0,d1
	and.w	#$7F80,d0	; extract exponent
	asr.w	#3,d0		; align for double
	add.w	#(1022-126)<<4,d0
	and.w	#$8000,d1	; get sign
	or.w	d1,d0		; assemble
	swap	d0
	move.l	a0,d1		; recall mantissa
	ror.l	#3,d1		; line it up
	move.l	d1,a0		; save again
	and.l	#$000FFFFF,d1
	clr.w	d0		; fixed for V1.4
	or.l	d1,d0
	move.l	a0,d1
	and.l	#$E0000000,d1
	rts

	end
