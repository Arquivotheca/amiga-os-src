*******************************************************************************
*
*	$Id: midb.asm,v 40.2 93/03/11 17:21:03 mks Exp $
*
*******************************************************************************

********************************************************************************
*
* If not FPU-Only, we need this...
*
	IFND	CPU_FPU
*

* Original code by Jim Goodnow, Manx Software
* Rewritten by Dale Luck, GfxBase

	SECTION	mathieeesingbas

	NOLIST
	include	"exec/types.i"
	include "privatemathlibrary.i"
	include "float.i"
	LIST

;*	NULL Come Again Macro
	include "nulca.i"

*	inline setup code. we want speed here
*	macro used on entry to every external routine here
*	that must talk to 68881.
SETUP	macro
	move.l	d7,-(sp)		; used for exception processing
	move.l	MathIEEEBase_68881(a6),a0	; points to base of 68881 regs
	lea	operand(a0),a1		; preload a1
	endm

*	releases the 68881 for use by other tasks
SHUTDOWN	macro
	move.l	(sp)+,d7
	rts
	endm

SHUTDOWNCC	macro
	tst.l	d0	; get proper condition codes
	rts
	endm


resp	equ	0
control	equ	2
save	equ	4
restore	equ	6
opword	equ	8
command	equ	10
cond	equ	14
operand	equ	16

fp0	equ	$0000
fp1	equ	$0080

fpcr	equ	$1000
fpsr	equ	$0800
fpiar	equ	$0400

fmvtofps equ	$4400		; move to fp reg (single)
fmvfrfps equ	$6400		; mofr from fp to reg (single)
fptlong	equ	$6000		;move from fp reg (long)
fgtlong	equ	$4000		;move to fp reg (long)
fadd2fps	equ	$4422		;add to fp reg (single)
fsub2fps	equ	$4428		;sub to fp reg (single)
fmul2fps	equ	$4423		;mul to fp reg (single)
fdiv2fps	equ	$4420		;div to fp reg (single)
fsgnmul2fps	equ	$4427		;mul to fp reg (single)
fsgndiv2fps	equ	$4424		;div to fp reg (single)
fint	equ	$0001		;round to whatever
fintrz	equ	$0003		;round to zero
fsclong	equ	$4026		;scale to fp reg (long)
fgtext	equ	$0000		;move to fp reg (extended)
fsubext	equ	$0428		;sub fp1 from fp0 (extended)
fmvtocr	equ	$8000		;move to fp contrl register (long)
fmvfrcr	equ	$a000		;move to fp contrl register (long)


do_exception:
;	move.w	d7,0
;	we just look for div 0 and overflow
;	ignore the rest
;	send exception accknoledge
	move.w	#$2,control(a0)
	and.w	#$ff,d7	; extract vector number
	cmp.w	#50,d7	; if	#50=d7.w
	bne.s	0$
		divu	#0,d7	; force the issue
0$:			; endif

	cmp.w	#53,d7	; if	#53=d7.w
	bne.s	1$
		ori	#2,ccr
		trapv		; force it
1$:			; endif

;	ignore the rest
	rts

exception:
;		possible exception request
;		check a little more
;		d7 has the response primitive
;		d7 is also positive (bit 15 = 0)

	cmp.w	#$1c00,d7	; if #$1c00>d7.w
	blt.s	do_rts		; no exception

	cmp.w	#$1e00,d7	; if #$1e00>d7.w
	blt.s	do_exception

	cmp.w	#$5c00,d7	; if #$5c00>d7.w
	blt.s	do_rts		; no exception

	cmp.w	#$5e00,d7	; if #$5e00>d7.w
	blt.s	do_exception

do_rts:	rts			; no exception

*	xdef	IOMIIEEEDPFix
	xdef	IOMIIEEEDPFlt
*	xdef	IOMIIEEEDPCmp
	xdef	IOMIIEEEDPAdd
	xdef	IOMIIEEEDPSub
	xdef	IOMIIEEEDPMul
	xdef	IOMIIEEEDPDiv
*	xdef	IOMIIEEEDPTst
*	xdef	IOMIIEEEDPNeg
*	xdef	IOMIIEEEDPCeil
	xdef	IOMIIEEEDPFloor
*	xdef	IOMIIEEEDPAbs

;IOMIIEEEDPFix
;	SETUP
;*	transfer double argument to fp0
;	move.w	#fmvtofp+fp0,command(a0)
;	NULCA
;	move.l	d0,(a1)
;	move.l	d1,(a1)
;	NULREL
;
;*	Integerize it
;	move.w	#fintrz+fp0,command(a0)
;	NULREL
;
;*	transer fp0 to d0 as a long
;*	this needs to check for overflow yet
;	move.w	#fptlong+fp0,command(a0)
;	NULCA
;	move.l	(a1),d0
;	NULREL
;	SHUTDOWN

IOMIIEEEDPFlt
	SETUP
*	transfer long integer argument to fp0 and convert to double
;	move.w	#fgtlong+fp0,command(a0)
	NULCA1	#fgtlong+fp0
	move.l	d0,(a1)
	NULREL

*	transfer signle from fp0 to d0
;	move.w	#fmvfrfp+fp0,command(a0)
	NULCA1	#fmvfrfps+fp0
	move.l	(a1),d0
	NULREL
	SHUTDOWN

;IOMIIEEEDPCmp
;	SETUP
;*	transfer arg1 to fp0
;	move.w	#fmvtofp+fp0,command(a0)
;	NULCA
;	move.l	d0,(a1)
;	move.l	d1,(a1)
;	NULREL
;
;*	cmp arg2 to arg1
;	move.w	#fcmp2fp+fp0,command(a0)
;	NULCA
;	move.l	d2,(a1)
;	move.l	d3,(a1)
;	NULREL
;	bra.s	extract_ccs
;
;IOMIIEEEDPTst
;	SETUP
;*	tranfer arg1 to fp0, set condition codes
;	move.w	#fmvtofp+fp0,command(a0)
;	NULCA
;	move.l	d0,(a1)
;	move.l	d1,(a1)
;	NULREL
;
;extract_ccs:
;*	extract condition codes set from status register
;	move.w	#fmvfrcr+fpsr,command(a0)
;	clr.l	d0	; assume zero
;	NULCA
;	move.l	(a1),d1
;	NULREL
;	btst	#26,d1	; test for zero
;	bne.s	tst_exit
;	moveq	#-1,d0
;	btst	#27,d1	; test for negative
;	bne.s	tst_exit
;	moveq	#1,d0	; must be positive then
;tst_exit:
;	SHUTDOWNCC
;
;common_tst:
;	move.w	#$12,cond(a0)
;2$	move.w	(a0),d0
;	cmp.w	#$8900,d0
;	beq	2$
;	btst	#0,d0
;	beq	3$
;	move.l	#1,d0
;	bra	6$
;3$	move.w	#$14,cond(a0)
;4$	move.w	(a0),d0
;	cmp.w	#$8900,d0
;	beq	4$
;	btst	#0,d0
;	beq	5$
;	move.l	#-1,d0
;	bra	6$
;5$
;	move.l	#0,d0
;6$
;	NULREL
;	SHUTDOWNCC
;
;IOMIIEEEDPAbs
;	SETUP
;*	transfer double from d0/d1 to fp0 and Abs it
;	move.w	#fabs2fp+fp0,command(a0)
;	NULCA
;	move.l	d0,(a1)
;	move.l	d1,(a1)
;	NULREL
;
;*	transfer fp0 to d0/d1
;	move.w	#fmvfrfp+fp0,command(a0)
;	NULCA
;	move.l	(a1),d0
;	move.l	(a1),d1
;	NULREL
;	SHUTDOWN
;
;IOMIIEEEDPNeg
;	SETUP
;*	transfer double from d0/d1 to fp0 and Neg it
;	move.w	#fneg2fp+fp0,command(a0)
;	NULCA
;	move.l	d0,(a1)
;	move.l	d1,(a1)
;	NULREL
;
;*	transfer fp0 to d0/d1
;	move.w	#fmvfrfp+fp0,command(a0)
;	NULCA
;	move.l	(a1),d0
;	move.l	(a1),d1
;	NULREL
;	SHUTDOWN

IOMIIEEEDPAdd
	SETUP
*	transfer single from d0 to fp0
;	move.w	#fmvtofp+fp0,command(a0)
	NULCA1	#fmvtofps+fp0
	move.l	d0,(a1)
	NULREL

*	add arg2(d1) to fp0
;	move.w	#fadd2fp+fp0,command(a0)
	NULCA1	#fadd2fps+fp0
	move.l	d1,(a1)
	NULREL

*	transfer fp0 to d0
;	move.w	#fmvfrfp+fp0,command(a0)
	NULCA1	#fmvfrfps+fp0
	move.l	(a1),d0
	NULREL
	SHUTDOWN

IOMIIEEEDPSub
	SETUP
*	transfer single(d0) to fp0
;	move.w	#fmvtofp+fp0,command(a0)
	NULCA1	#fmvtofps+fp0
	move.l	d0,(a1)
	NULREL

*	sub single(d1) from fp0
;	move.w	#fsub2fp+fp0,command(a0)
	NULCA1	#fsub2fps+fp0
	move.l	d1,(a1)
	NULREL

*	transfer fp0 to d0
;	move.w	#fmvfrfp+fp0,command(a0)
	NULCA1	#fmvfrfps+fp0
	move.l	(a1),d0
	NULREL
	SHUTDOWN

IOMIIEEEDPMul
	SETUP
*	transfer single(d0) to fp0
;	move.w	#fmvtofp+fp0,command(a0)
	NULCA1	#fmvtofps+fp0
	move.l	d0,(a1)
	NULREL

*	multiply double(d1) to fp0
;	move.w	#fmul2fp+fp0,command(a0)
	NULCA1	#fsgnmul2fps+fp0
	move.l	d1,(a1)
	NULREL

*	transfer fp0 to d0
;	move.w	#fmvfrfp+fp0,command(a0)
	NULCA1	#fmvfrfps+fp0
	move.l	(a1),d0
	NULREL
	SHUTDOWN

IOMIIEEEDPDiv
	SETUP
*	transfer single(d0) to fp0
;	move.w	#fmvtofp+fp0,command(a0)
	NULCA1	#fmvtofps+fp0
	move.l	d0,(a1)
	NULREL

*	divide fp0 by double(d1)
;	move.w	#fdiv2fp+fp0,command(a0)
	NULCA1	#fsgndiv2fps+fp0
	move.l	d1,(a1)
	NULREL

*	transfer fp0 to d0
;	move.w	#fmvfrfp+fp0,command(a0)
	NULCA1	#fmvfrfps+fp0
	move.l	(a1),d0
	NULREL
	SHUTDOWN

IOMIIEEEDPFloor
*	new Floor code
	SETUP
*	transer single(d0) to fp0
;	move.w	#fmvtofp+fp0,command(a0)	; put in fp0
	NULCA1	#fmvtofps+fp0
	move.l	d0,(a1)
	NULREL

;	what is current rounding mode?
;	move.w	#fmvfrcr+fpcr,command(a0)
	NULCA1	#fmvfrcr+fpcr
	move.l	(a1),-(sp)		; save current rounding mode
	NULREL

; setup correct rounding mode
;	move.w	#fmvtocr+fpcr,command(a0)
	NULCA1	#fmvtocr+fpcr
	move.l	#$000000a0,(a1)		; round to -infinity
	NULREL

*	floor(fp0)
	move.w	#fint+fp0,command(a0)
	NULREL

*	transfer fp0 to d0
;	move.w	#fmvfrfp+fp0,command(a0)	; get result
	NULCA1	#fmvfrfps+fp0
	move.l	(a1),d0
	NULREL

; restore rounding mode
;	move.w	#fmvtocr+fpcr,command(a0)
	NULCA1	#fmvtocr+fpcr
	move.l	(sp)+,(a1)
	NULREL

	SHUTDOWN

;IOMIIEEEDPCeil
;*	new Ceil code
;	SETUP
;*	transer double(d0/d1) to fp0
;	move.w	#fmvtofp+fp0,command(a0)	; put in fp0
;	NULCA
;	move.l	d0,(a1)
;	move.l	d1,(a1)
;	NULREL
;
;;	what is current rounding mode?
;	move.w	#fmvfrcr+fpcr,command(a0)
;	NULCA
;	move.l	(a1),-(sp)		; save current rounding mode
;	NULREL
;
;; setup correct rounding mode
;	move.w	#fmvtocr+fpcr,command(a0)
;	NULCA
;	move.l	#$000000b0,(a1)		; round to +infinity
;	NULREL
;
;*	Ceil(fp0)
;	move.w	#fint+fp0,command(a0)
;	NULREL
;
;*	transfer fp0 to d0/d1
;	move.w	#fmvfrfp+fp0,command(a0)	; get result
;	NULCA
;	move.l	(a1),d0
;	move.l	(a1),d1
;	NULREL
;
;; restore rounding mode
;	move.w	#fmvtocr+fpcr,command(a0)
;	NULCA
;	move.l	(sp)+,(a1)
;	NULREL
;
;	SHUTDOWN

        xdef    init_io68881
init_io68881:
*	frestore	68881, reset 68881
	SETUP
	moveq	#0,d0
	move.w	d0,restore(a0)	; abort current instruction
				; and begin reset of 881 state
	move.w	restore(a0),d0	; get verifification
	and.w	#$FF00,d0

	beq.s	6$	; if <>			; we expect to '0' return
		rts		; return with error
6$:			; endif

	NULREL
;	now set up for correct rounding modes
;	move.w	#fmvtocr+fpcr,command(a0)
	NULCA1	#fmvtocr+fpcr
	ifd	ALLOW_TRAPS
	move.l	#$00001440,(a1)		; round to single
					; trigger exception on div0,ovfl
	endc
	ifnd	ALLOW_TRAPS
	move.l	#$00000040,(a1)		; round to single
	endc
	NULREL
	clr.l	d0
	SHUTDOWN

	ENDC

	END
