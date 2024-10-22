	section mathieeesingtrans

	xdef	HARDMIIEEESPSincos
	xdef	HARDMIIEEESPPow
	xdef	HARDMIIEEESPTieee
	xdef	HARDMIIEEESPFieee
	

FPCR	equ	$1000
FPSR	equ	$0800
FPIAR	equ	$0400

FMOVE	equ	$F200
LgnWrdI	equ	$0
SinPreR	equ	$400
ExtPreR	equ	$800
PckDecR	equ	$c00

DPR	equ	$1400	; format = double precision real
SPR	equ	$0400	; format = single precision real
LWI	equ	$0	; format = long word integer

FP0	equ	$0<<7
FP1	equ	$1<<7

FMOVEDPtoFP0 macro
	dc.w	FMOVE+$1f		; FMOVE.d (sp)+,fp0
	dc.w	$4000+DPR
	endm

FMOVEFD0toFP0 macro
	dc.w	FMOVE+0			; FMOVE.s	d0,fp0
	dc.w	$4000+SPR
	endm

FMOVEFP0toFD0	macro
	dc.w	FMOVE+0			; FMOVE.s	fp0,d0
	dc.w	$6000+SPR
	endm

DOIT	macro
	xdef	HARDMIIEEESP\1
	cnop	0,4
HARDMIIEEESP\1:
	dc.w	FMOVE+0
	dc.w	$4000+SPR+\2		; Fxxx.s	d0,fp0
	FMOVEFP0toFD0
	rts
	endm

	DOIT	Atan,$0a
	DOIT	Sin,$0e
	DOIT	Cos,$1d
	DOIT	Tan,$0f
	DOIT	Sinh,$02
	DOIT	Cosh,$19
	DOIT	Tanh,$09
	DOIT	Exp,$10
	DOIT	Log,$14
	DOIT	Sqrt,$04

	cnop	0,4
HARDMIIEEESPSincos:
	dc.w	FMOVE+0		; FSincos.s d0,fp1:fp0
	dc.w	$4000+SPR+$30+1		;
	FMOVEFP0toFD0
	dc.w	FMOVE+$10		; Fmove.s fp1,(a0)
	dc.w	$6000+SPR+FP1
	rts

	xref	_hardpow
	cnop	0,4
HARDMIIEEESPPow:
*	this is acombination of software emulation and hardware usage
*	call c routine
	pea	explog
	movem.l	d0/d1,-(sp)
	jsr	_hardpow
	lea	12(sp),sp
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


	movem.l	4(sp),d0/d1
	dc.w	FMOVE+0
	dc.w	$4000+SPR+$14		; FLogn.s	d0,fp0

	dc.w	FMOVE+$1
	dc.w	$4000+SPR+$23		; FMul.s d1,fp0
	dc.w	FMOVE
	dc.w	$10			; Fexp
	FMOVEFP0toFD0
	rts

HARDMIIEEESPTieee:
*	convert from native to single ieee

HARDMIIEEESPFieee:
*	convert from single to native ieee
	rts


	DOIT	Asin,$0c
	DOIT	Acos,$1c
	DOIT	Log10,$15

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
