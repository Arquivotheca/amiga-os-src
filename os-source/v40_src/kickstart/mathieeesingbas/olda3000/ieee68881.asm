
*******************************************************************************
*
*	$Header:
*
*******************************************************************************

	section mathieeesingbas

	include "float.i"

	xdef	HARDMIIEEEDPSub
*	xdef	HARDMIIEEEDPTst		;use software
	xdef	HARDMIIEEEDPFix
	xdef	HARDMIIEEEDPFlt
*	xdef	HARDMIIEEEDPNeg
	xdef	HARDMIIEEEDPFloor
	xdef	HARDMIIEEESPCmp
	xdef	HARDMIIEEEDPDiv
*	xdef	HARDMIIEEEDPAbs
	xdef	HARDMIIEEEDPMul
*	xdef	HARDMIIEEEDPCeil
	xdef	HARDMIIEEEDPAdd
	

FPCR	equ	$1000
FPSR	equ	$0800
FPIAR	equ	$0400

FMOVE	equ	$F200

LWI	equ	$0	; format = long word integer
SPR	equ	$0400	; format = single precision real
DPR	equ	$1400	; format = double precision real

FMOVEFD0toFP0 macro
	dc.w	FMOVE+0			; FMOVE.s	d0,fp0
	dc.w	$4000+SPR
	endm
FMOVEFP0toFD0	macro
	dc.w	FMOVE+0			; FMOVE.s	fp0,d0
	dc.w	$6000+SPR
	endm

;	assume aligned on long word boundary here
	cnop	0,4
HARDMIIEEEDPFlt:
	dc.w	FMOVE			; FMOVE.l	d0,fp0
	dc.w	$4000+LWI
	dc.w	FMOVE+0		; FMOVE.s	fp0,d0
	dc.w	$6000+SPR
	rts

	cnop 0,4
HARDMIIEEEDPFix:
	FMOVEFD0toFP0
	dc.w	FMOVE			; FMOVE.l fp0,d0
	dc.w	$6000+LWI
	rts
	cnop	0,4
;
; following is more efficient in software emulator
;HARDMIIEEEDPNeg:
;	movem.l	d0/d1,-(sp)
;	dc.w	FMOVE+$1f		; FNEG.d (sp)+,fp0
;	dc.w	$4000+DPR+$1a		;
;	FMOVEFP0toFD0D1
;	rts
;	nop
;
;HARDMIIEEEDPAbs:
;	movem.l	d0/d1,-(sp)
;	dc.w	FMOVE+$1f		; FABS.d (sp)+,fp0
;	dc.w	$4000+DPR+$18		;
;	FMOVEFP0toFD0D1
;	rts
;	nop

	cnop	0,4
floorneg:
	FMOVEFD0toFP0
	dc.w FMOVE			; FMOVE.l FPCR,d0
	dc.w $8000+$2000+FPCR
	move.l	d0,d1			; save to restore
	move.b #$20,d0			; round toward minus infinity
	dc.w FMOVE			; FMOVE.l d0,FPCR
	dc.w $8000+$0000+FPCR		; set up what we want
	dc.w FMOVE			; FINT FP0,FP0
	dc.w $1				;
	dc.w FMOVE+1			; FMOVE.l d1,FPCR  ; restore
	dc.w $8000+$0000+FPCR		; set up what used to be
	FMOVEFP0toFD0
	rts

	cnop	0,4
HARDMIIEEEDPFloor:
	tst.l	d0			; is arg < 0?
	blt	floorneg
	dc.w	FMOVE+0		; FINTRZ.s d0,fp0
	dc.w	$4000+SPR+$3		;
	FMOVEFP0toFD0
	rts

;	xref	_LVOIEEEDPNeg
;	xref	_LVOIEEEDPFloor
;HARDMIIEEEDPCeil:
;	jsr	_LVOIEEEDPNeg(a6)
;	jsr	_LVOIEEEDPFloor(a6)
;	jsr	_LVOIEEEDPNeg(a6)
;	rts


	cnop	0,4
HARDMIIEEEDPAdd:
	FMOVEFD0toFP0
	dc.w	FMOVE+$01		; FADD.s d1,fp0
	dc.w	$4000+SPR+$22
	FMOVEFP0toFD0
	rts

	cnop	0,4
HARDMIIEEEDPSub:
	FMOVEFD0toFP0
	dc.w	FMOVE+$01		; FSUB.s d1,fp0
	dc.w	$4000+SPR+$28
	FMOVEFP0toFD0
	rts

	cnop	0,4
HARDMIIEEEDPMul:
	FMOVEFD0toFP0
	dc.w	FMOVE+$01		; FSGLMUL.s d1,fp0
	dc.w	$4000+SPR+$27
	FMOVEFP0toFD0
	rts

	cnop	0,4
HARDMIIEEEDPDiv:
;	if d2=#0.l
;		divu	d2,d2
;	endif
	FMOVEFD0toFP0
	dc.w	FMOVE+$01		; FSGLDIV.s d1,fp0
	dc.w	$4000+SPR+$24
	FMOVEFP0toFD0
	rts

FBCC	equ $F280
FBEQ	equ	FBCC+$1
FBGT	equ	FBCC+$12
HARDMIIEEESPCmp:
	FMOVEFD0toFP0
	dc.w	FMOVE+$01		; FCMP.s d1,fp0
	dc.w	$4000+SPR+$38
	dc.w	FBEQ				;FBEQ equal
equalt:
	dc.w	equal-equalt	;
	dc.w	FBGT			;	FBGT	grtrthan
grtrt:
	dc.w	grtrthan-grtrt
lessthan:	moveq	#-1,d0
		rts
grtrthan:	moveq	#1,d0
		rts
equal:	clr.l	d0
	rts


	xdef	init_68881
*	enable divide by zero exception
init_68881:
	ifd	ALLOW_TRAPS
;	move.l	#$1450,d0	; chop and round to single
	move.l	#$1490,d0	; chop and round to double
	endc
	ifnd	ALLOW_TRAPS
;	move.l	#$0090,d0	; chop and round to double
;	move.l	#$0050,d0	; chop and round to single
	move.l	#$0040,d0	; round to single
	endc
	dc.w	FMOVE
	dc.w	$8000+$0000+FPCR	; enable traps
*			; on div 0 ,underflow,overflow
	clr.l	d0
	rts

	end
