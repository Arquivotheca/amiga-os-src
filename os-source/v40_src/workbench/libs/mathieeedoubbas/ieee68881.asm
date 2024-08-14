
*******************************************************************************
*
*	$Header:
*
*******************************************************************************

	section mathieeedoubbas

	include "float.i"

	xdef	HARDMIIEEEDPSub
*	xdef	HARDMIIEEEDPTst		;use software
	xdef	HARDMIIEEEDPFix
	xdef	HARDMIIEEEDPFlt
*	xdef	HARDMIIEEEDPNeg
	xdef	HARDMIIEEEDPFloor
*	xdef	HARDMIIEEEDPCmp
	xdef	HARDMIIEEEDPDiv
*	xdef	HARDMIIEEEDPAbs
	xdef	HARDMIIEEEDPMul
*	xdef	HARDMIIEEEDPCeil
	xdef	HARDMIIEEEDPAdd
	

FPCR	equ	$1000
FPSR	equ	$0800
FPIAR	equ	$0400

FMOVE	equ	$F200

DPR	equ	$1400	; format = double precision real
LWI	equ	$0	; format = long word integer

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

;	assume aligned on long word boundary here
	cnop	0,4
HARDMIIEEEDPFlt:
	dc.w	FMOVE			; FMOVE.l	d0,fp0
	dc.w	$4000+LWI
	dc.w	FMOVE+$27		; FMOVE.d	fp0,-(sp)
	dc.w	$6000+DPR
	movem.l	(sp)+,d0/d1
	rts

HARDMIIEEEDPFix:
	FMOVEFD0D1toFP0
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
	FMOVEFD0D1toFP0
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
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPFloor:
	tst.l	d0			; is arg < 0?
	blt	floorneg
	movem.l	d0/d1,-(sp)
	dc.w	FMOVE+$1f		; FINTRZ.d (sp)+,fp0
	dc.w	$4000+DPR+$3		;
	FMOVEFP0toFD0D1
	rts
	nop

;	xref	_LVOIEEEDPNeg
;	xref	_LVOIEEEDPFloor
;HARDMIIEEEDPCeil:
;	jsr	_LVOIEEEDPNeg(a6)
;	jsr	_LVOIEEEDPFloor(a6)
;	jsr	_LVOIEEEDPNeg(a6)
;	rts


	cnop	0,4
HARDMIIEEEDPAdd:
	movem.l	d0/d1/d2/d3,-(sp)
	FMOVESPtoFP0
	dc.w	FMOVE+$1f		; FADD.d (sp)+,fp0
	dc.w	$4000+DPR+$22
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPSub:
	FMOVEFD0D1toFP0
	movem.l	d2/d3,-(sp)
	dc.w	FMOVE+$1f		; FSUB.d (sp)+,fp0
	dc.w	$4000+DPR+$28
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPMul:
	movem.l	d0/d1/d2/d3,-(sp)
	FMOVESPtoFP0
	dc.w	FMOVE+$1f		; FMUL.d (sp)+,fp0
	dc.w	$4000+DPR+$23
	FMOVEFP0toFD0D1
	rts

	cnop	0,4
HARDMIIEEEDPDiv:
;	if d2=#0.l
;		divu	d2,d2
;	endif
	FMOVEFD0D1toFP0
	movem.l	d2/d3,-(sp)
	dc.w	FMOVE+$1f		; FDIV.d (sp)+,fp0
	dc.w	$4000+DPR+$20
	FMOVEFP0toFD0D1
	rts

;FBCC	equ $F280
;FBEQ	equ	FBCC+$1
;FBGT	equ	FBCC+$12
;HARDMIIEEEDPCmp:
;	FMOVEFD0D1toFP0
;	movem.l	d2/d3,-(sp)
;	dc.w	FMOVE+$1f		; FCMP.d (sp)+,fp0
;	dc.w	$4000+DPR+$38
;	dc.w	FBEQ				;FBEQ equal
;equalt:
;	dc.w	equal-equalt	;
;	dc.w	FBGT			;	FBGT	grtrthan
;grtrt:
;	dc.w	grtrthan-grtrt
;lessthan:	moveq	#-1,d0
;		rts
;grtrthan:	moveq	#1,d0
;		rts
;equal:	clr.l	d0
;	rts


	xdef	init_68881
*	enable divide by zero exception
init_68881:
	ifd	ALLOW_TRAPS
	move.l	#$1490,d0	; chop and round to double
	endc
	ifnd	ALLOW_TRAPS
	move.l	#$0090,d0	; chop and round to double
	endc
	dc.w	FMOVE
	dc.w	$8000+$0000+FPCR	; enable traps
*			; on div 0 ,underflow,overflow
	clr.l	d0
	rts

	end
