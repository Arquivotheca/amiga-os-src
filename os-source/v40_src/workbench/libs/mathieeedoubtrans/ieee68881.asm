	TTL    '$Id: ieee68881.asm,v 37.1 91/01/21 15:19:02 mks Exp $'
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
*   $Id: ieee68881.asm,v 37.1 91/01/21 15:19:02 mks Exp $
*
**********************************************************************
	section mathieeedoubtrans

	xdef	HARDMIIEEEDPAtan
	xdef	HARDMIIEEEDPSin
	xdef	HARDMIIEEEDPCos
	xdef	HARDMIIEEEDPTan
	xdef	HARDMIIEEEDPSincos
	xdef	HARDMIIEEEDPSinh
	xdef	HARDMIIEEEDPCosh
	xdef	HARDMIIEEEDPTanh
	xdef	HARDMIIEEEDPExp
	xdef	HARDMIIEEEDPLog
	xdef	HARDMIIEEEDPPow
	xdef	HARDMIIEEEDPSqrt
	xdef	HARDMIIEEEDPTieee
	xdef	HARDMIIEEEDPFieee
	xdef	HARDMIIEEEDPAsin
	xdef	HARDMIIEEEDPAcos
	xdef	HARDMIIEEEDPLog10


FPCR	equ	$1000
FPSR	equ	$0800
FPIAR	equ	$0400

FMOVE	equ	$F200
LgnWrdI	equ	$0
SinPreR	equ	$400
ExtPreR	equ	$800
PckDecR	equ	$c00

DPR	equ	$1400	; format = double precision real
LWI	equ	$0	; format = long word integer

FP0	equ	$0<<7
FP1	equ	$1<<7

FMOVESPtoFP0 macro
	dc.w	FMOVE+$1f		; FMOVE.d (sp)+,fp0
	dc.w	$4000+DPR
	endm

FMOVEFD0D1toFP0 macro
*	movem.l	d0/d1,-(sp)
	move.l	d1,-(sp)
	move.l	d0,-(sp)
	FMOVESPtoFP0
	endm
FMOVEFP0toFD0D1	macro
	dc.w	FMOVE+$27		; FMOVE.d	fp0,-(sp)
	dc.w	$6000+DPR
*	movem.l	(sp)+,d0/d1
	move.l	(sp)+,d0
	move.l	(sp)+,d1
	endm


	cnop	0,4
HARDMIIEEEDPAtan:
	movem.l	d0/d1,-(sp)
	dc.w	FMOVE+$1f		; FAtan.d (sp)+,fp0
	dc.w	$4000+DPR+$0a		;
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPSin:
	movem.l	d0/d1,-(sp)
	dc.w	FMOVE+$1f		; FSin.d (sp)+,fp0
	dc.w	$4000+DPR+$0e		;
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPCos:
	movem.l	d0/d1,-(sp)
	dc.w	FMOVE+$1f		; FCos.d (sp)+,fp0
	dc.w	$4000+DPR+$1d		;
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPTan:
	movem.l	d0/d1,-(sp)
	dc.w	FMOVE+$1f		; FTan.d (sp)+,fp0
	dc.w	$4000+DPR+$0f		;
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPSincos:
	movem.l	d0/d1,-(sp)
	dc.w	FMOVE+$1f		; FSincas.d (sp)+,fp1:fp0
	dc.w	$4000+DPR+$30+1		;
	FMOVEFP0toFD0D1
	dc.w	FMOVE+$10		; Fmove.s fp1,(a0)
	dc.w	$6000+DPR+FP1
	rts

	cnop	0,4
HARDMIIEEEDPSinh:
	movem.l	d0/d1,-(sp)
	dc.w	FMOVE+$1f		; FSinh.d (sp)+,fp0
	dc.w	$4000+DPR+$02		;
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPCosh:
	movem.l	d0/d1,-(sp)
	dc.w	FMOVE+$1f		; FCosh.d (sp)+,fp0
	dc.w	$4000+DPR+$19		;
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPTanh:
	movem.l	d0/d1,-(sp)
	dc.w	FMOVE+$1f		; FTanh.d (sp)+,fp0
	dc.w	$4000+DPR+$09		;
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPExp:
	movem.l	d0/d1,-(sp)
	dc.w	FMOVE+$1f		; FEtox.d (sp)+,fp0
	dc.w	$4000+DPR+$10		;
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPLog:
	movem.l	d0/d1,-(sp)
	dc.w	FMOVE+$1f		; FLogn.d (sp)+,fp0
	dc.w	$4000+DPR+$14		;
	FMOVEFP0toFD0D1
	rts

	xref	_hardpow
	cnop	0,4
HARDMIIEEEDPPow:
*	this is acombination of software emulation and hardware usage
*	call c routine
	pea	explog
	movem.l	d0/d1/d2/d3,-(sp)
	jsr	_hardpow
	lea	20(sp),sp
	rts

	cnop	0,4
explog:
;	movem.l	d0/d1,-(sp)
;	dc.w	FMOVE+$1f		; FLog.d (sp)+,fp0
;	dc.w	$4000+DPR+$14
;	movem.l	d2/d3,-(sp)
;	dc.w	FMOVE+$1f
;	dc.w	$4000+DPR+$23		; FMul.d (sp)+,fp0
;	dc.w	FMOVE
;	dc.w	$10			; Fexp.d fp0
;	FMOVEFP0toFD0D1
;	rts


	movem.l	4(sp),d0/d1/d2/d3
	movem.l	d0/d1/d2/d3,-(sp)
	dc.w	FMOVE+$1f		; FLogn.d (sp)+,fp0
	dc.w	$4000+DPR+$14		;
	dc.w	FMOVE+$1f
	dc.w	$4000+DPR+$23		; FMul (sp)+,fp0
	dc.w	FMOVE
	dc.w	$10
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPSqrt:
	movem.l	d0/d1,-(sp)
	dc.w	FMOVE+$1f		; FSqrt.d (sp)+,fp0
	dc.w	$4000+DPR+$04		;
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPTieee:
*	convert from double to single ieee
	FMOVEFD0D1toFP0
	dc.w	FMOVE			; Fmove.s FP0,d0
	dc.w	$6000+SinPreR
	rts

	cnop	0,4
HARDMIIEEEDPFieee:
*	convert from single to double ieee
	dc.w	FMOVE			; Fmove.s d0,FP0
	dc.w	$4000+SinPreR		; Just added $4000 (7/29/88) Dale
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPAsin:
	movem.l	d0/d1,-(sp)
	dc.w	FMOVE+$1f		; FAsin.d (sp)+,fp0
	dc.w	$4000+DPR+$0c		;
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPAcos:
	movem.l	d0/d1,-(sp)
	dc.w	FMOVE+$1f		; FAcos.d (sp)+,fp0
	dc.w	$4000+DPR+$1c		;
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPLog10:
	movem.l	d0/d1,-(sp)
	dc.w	FMOVE+$1f		; FLog10.d (sp)+,fp0
	dc.w	$4000+DPR+$15		;
	FMOVEFP0toFD0D1
	rts

	xdef	init_68881
	xref _Debug
*	enable divide by zero exception
init_68881:
;	this initialization is done by mathieeedoubbas
;	move.l	#$1490,d0
;	dc.w	FMOVE
;	dc.w	$8000+$0000+FPCR	; enable traps
*			; on div 0 ,overflow
	clr.l	d0
	rts

	end
