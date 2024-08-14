	TTL    '$Id: softieee.asm,v 37.1 91/01/21 15:18:58 mks Exp $'
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
*   $Id: softieee.asm,v 37.1 91/01/21 15:18:58 mks Exp $
*
**********************************************************************
	section mathieeedoubtrans

	xdef	MIIEEEDPSin
	xdef	MIIEEEDPCos
	xdef	MIIEEEDPTan
	xdef	MIIEEEDPSincos
	xdef	MIIEEEDPSinh
	xdef	MIIEEEDPCosh
	xdef	MIIEEEDPTanh
	xdef	MIIEEEDPExp
	xdef	MIIEEEDPLog
	xdef	MIIEEEDPPow
*	xdef	MIIEEEDPSqrt
	xdef	MIIEEEDPAsin
	xdef	MIIEEEDPAcos
	xdef	MIIEEEDPLog10

	xref	_sin
	xref	_cos
	xref	_tan
	xref	_sincos
	xref	_sinh
	xref	_cosh
	xref	_tanh
	xref	_exp
	xref	_log
	xref	_pow
*	xref	_sqrt
	xref	_tieee
	xref	_fieee
	xref	_asin
	xref	_acos
	xref	_log10

MIIEEEDPSin:
	movem.l	d0/d1,-(sp)
	bsr	_sin
	addq.l	#8,sp
	rts
MIIEEEDPCos:
	movem.l	d0/d1,-(sp)
	bsr	_cos
	addq.l	#8,sp
	rts
MIIEEEDPTan:
	movem.l	d0/d1,-(sp)
	bsr	_tan
	addq.l	#8,sp
	rts
MIIEEEDPSincos:
	move.l	a0,-(sp)
	movem.l	d0/d1,-(sp)
	bsr	_sincos
	lea	12(sp),sp
	rts
MIIEEEDPSinh:
	movem.l	d0/d1,-(sp)
	bsr	_sinh
	addq.l	#8,sp
	rts
MIIEEEDPCosh:
	movem.l	d0/d1,-(sp)
	bsr	_cosh
	addq.l	#8,sp
	rts
MIIEEEDPTanh:
	movem.l	d0/d1,-(sp)
	bsr	_tanh
	addq.l	#8,sp
	rts
MIIEEEDPExp:
	movem.l	d0/d1,-(sp)
	bsr	_exp
	addq.l	#8,sp
	rts
MIIEEEDPLog:
	movem.l	d0/d1,-(sp)
	bsr	_log
	addq.l	#8,sp
	rts
MIIEEEDPPow:
*	movem.l	d0/d1/d2/d3,-(sp)
	movem.l	d2/d3,-(sp)
	movem.l	d0/d1,-(sp)
	bsr	_pow
	lea	16(sp),sp
	rts
*MIIEEEDPSqrt:
*	movem.l	d0/d1,-(sp)
*	bsr	_sqrt
*	addq.l	#8,sp
*	rts
MIIEEEDPAsin:
	movem.l	d0/d1,-(sp)
	bsr	_asin
	addq.l	#8,sp
	rts
MIIEEEDPAcos:
	movem.l	d0/d1,-(sp)
	bsr	_acos
	addq.l	#8,sp
	rts
;CATAN	equ 1
	ifd CATAN
	xref	_atan
	xdef	MIIEEEDPAtan
MIIEEEDPAtan:
	movem.l	d0/d1,-(sp)
	bsr	_atan
	addq.l	#8,sp
	rts
	endc
MIIEEEDPLog10:
	movem.l	d0/d1,-(sp)
	bsr	_log10
	addq.l	#8,sp
	rts


	end
