* original code written by Jim Goodnow
* rewritten by Dale Luck
*

	TTL    '$Id: midb.asm,v 37.1 91/01/21 15:18:39 mks Exp $'
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
*   $Id: midb.asm,v 37.1 91/01/21 15:18:39 mks Exp $
*
**********************************************************************


	SECTION	mathieeedoubtrans

	include 'exec/types.i'

	include "privatemathlibrary.i"

	include "../mathieeedoubbas/nulca.i"
;	ifd QWE
;*	NULL Come Again Macro
;;NULCA	macro
;;\@w68	cmp.w	#$8900,(a0)
;;	beq	\@w68
;;	endm
;NULCA   macro
;\@w68   move.w  (a0),d7
;        cmp.w   #$8900,d7
;        beq     \@w68
;;       check for pending exceptions
;        tst.w   d7      ; can use a different bcc later
;;               do a gross check inline
;        bmi.s   \@w69
;                bsr     exception
;\@w69:
;        cmp.w   #$C900,d7
;        beq     \@w68
;        endm
;
;NULCA1   macro
;\@restart	move.w	\1,command(a0)
;\@w68   move.w  (a0),d7
;        cmp.w   #$8900,d7
;        beq     \@w68
;;       check for pending exceptions
;        tst.w   d7      ; can use a different bcc later
;;               do a gross check inline
;        bmi.s   \@w69
;                bsr     exception
;		bra	\@restart
;\@w69:
;        cmp.w   #$C900,d7
;        beq     \@w68
;        endm
;
;*	NULL Release Macro
;NULREL	macro
;\@nr	tst.w	(a0)
;	bmi	\@nr
;	endm
;	endc

*	inline setup code. we want speed here
STACK	equ	4			; extra bytes on stack
SETUP	macro
	move.l	d7,-(sp)
	move.l	MathIEEEBase_68881(a6),a0	; points to base of 68881 regs
	lea	operand(a0),a1		; preload a1
	endm

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

fmvtofp	equ	$5400		;move to fp reg (double)
fmvfrfp	equ	$7400		;move from fp reg (double)
fptlong	equ	$6000		;move from fp reg (long)
fgtlong	equ	$4000		;move to fp reg (long)
fptsngl equ	$6400		;move from fp to reg (single)
fgtsngl equ	$4400		;move from to fp reg (single)

fadd2fp	equ	$5422		;add to fp reg (double)
fsub2fp	equ	$5428		;sub to fp reg (double)
fmul2fp	equ	$5423		;mul to fp reg (double)
fdiv2fp	equ	$5420		;div to fp reg (double)
fint	equ	$0001		;round to whatever
fintrz	equ	$0003		;round to zero
fneg2fp	equ	$541a		;negate to fp reg (double)
fabs2fp	equ	$5418		;abs to fp reg (double)
fcmp2fp	equ	$5438		;cmp to fp reg (double)
fsclong	equ	$4026		;scale to fp reg (long)
fgtext	equ	$0000		;move to fp reg (extended)
fsubext	equ	$0428		;sub fp1 from fp0 (extended)
fmvtocr	equ	$8000		;move to fp contrl register (long)
fmvfrcr	equ	$a000		;move to fp contrl register (long)

FPROCD0D1toFP0	macro
;	move.w	#fmvtofp+fp0+\1,command(a0)
	NULCA1	#fmvtofp+fp0+\1
	move.l	d0,(a1)
	move.l	d1,(a1)
	NULREL
	endm

FMOVED0D1toFP0	macro
;	move.w	#fmvtofp+fp0,command(a0)
	NULCA1	#fmvtofp+fp0
	move.l	d0,(a1)
	move.l	d1,(a1)
	NULREL
	endm

FMOVEFP0toD0D1	macro
;	move.w	#fmvfrfp+fp0,command(a0)
	NULCA1	#fmvfrfp+fp0
	move.l	(a1),d0
	move.l	(a1),d1
	NULREL
	endm

DOIT	macro
IOMIIEEEDP\1:
	xdef	IOMIIEEEDP\1
	SETUP
	FPROCD0D1toFP0	\2
	FMOVEFP0toD0D1
	SHUTDOWN
	endm



do_exception:
;	move.w	d7,0
;	we just look for div 0 and overflow
;	ignore the rest
	and.w	#$ff,d7	; extract vector number
	if	#50=d7.w
		divu	#0,d7	; force the issue
	endif
	if	#53=d7.w
		ori	#2,ccr
		trapv		; force it
	endif
;	ignore the rest
	rts

exception:
;		possible exception request
;		check a little more
;		d7 has the response primitive
;		d7 is also positive (bit 15 = 0)
	if #$1c00>d7.w
		rts		; no exception
	endif
	if #$1e00>d7.w
		bra	do_exception
	endif
	if #$5c00>d7.w
		rts		; no exception
	endif
	if #$5e00>d7.w
		bra	do_exception
	endif
	rts			; no exception




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

	xdef	IOMIIEEEDPSincos
*	this is a screwy one
IOMIIEEEDPSincos
	move.l	a2,-(sp)			; need another register
	move.l	a0,a2
	SETUP
;	move.w	#fmvtofp+$30+$1,command(a0)	; Fsincos
	NULCA1	#fmvtofp+$30+$1
	move.l	d0,(a1)
	move.l	d1,(a1)
	NULREL
;	Next line not needed (Dale)
;	move.l	a0,d0				; save here
	FMOVEFP0toD0D1
;	move.w	#fmvfrfp+$80,command(a0)	; Fmove.d fp1,(a2)
	NULCA1	#fmvfrfp+$80
	move.l	(a1),(a2)+
	move.l	(a1),(a2)
	NULREL
	move.l	(sp)+,a2			; restore a2
	SHUTDOWN

	xdef	IOMIIEEEDPTieee
IOMIIEEEDPTieee:
*	convert double to single ieee
	SETUP
	FMOVED0D1toFP0
;	move.w	#fptsngl+fp0,command(a0)	; Fmove.s fp0,d0
	NULCA1	#fptsngl+fp0
	move.l	(a1),d0
	NULREL
	SHUTDOWN

	xdef	IOMIIEEEDPFieee
IOMIIEEEDPFieee:
*	convert single to double ieee
	SETUP
;	move.w	#fgtsngl+fp0,command(a0)	; Fmove.s d0,fp0
	NULCA1	#fgtsngl+fp0
	move.l	d0,(a1)
	NULREL
	FMOVEFP0toD0D1
	SHUTDOWN

	DOIT	Asin,$0c
	DOIT	Acos,$1c
	DOIT	Log10,$15

	xdef	IOMIIEEEDPPow
	xref	_hardpow
IOMIIEEEDPPow
*	This calls C routine to do parameter checking
*	and do the x^y if y is an integer between -1000 and +1000
	pea	explog		; incase this is needed
	movem.l	d0-d3,-(sp)
	bsr	_hardpow	; common routine
	lea	20(sp),sp
	rts

explog:
;	SETUP
;	FPROCD0D1toFP0	$14		;Flog(d0/d1)->fp0
;	move.w	#fmvtofp+fp0+$23,command(a0)	; FMul(d2/d3,fp0)
;	NULCA
;	move.l	d2,(a1)
;	move.l	d3,(a1)
;	NULREL
;	move.w	#$10,command(a0)		; Fexp(fp0)
;	NULREL
;	FMOVEFP0toD0D1
;	SHUTDOWN

	SETUP
;	move.w  #fmvtofp+fp0+$14,command(a0)
	NULCA1	#fmvtofp+fp0+$14
	move.l	STACK+4(sp),(a1)
	move.l	STACK+8(sp),(a1)
	NULREL
;	move.w	#fmvtofp+fp0+$23,command(a0)	; FMul(d2/d3,fp0)
	NULCA1	#fmvtofp+fp0+$23
	move.l	STACK+12(sp),(a1)
	move.l	STACK+16(sp),(a1)
	NULREL
	move.w	#$10,command(a0)		; Fexp(fp0)
	NULREL
	FMOVEFP0toD0D1
	SHUTDOWN

	xdef	init_io68881
init_io68881:
*	assume basic library has this all setup
	clr.l	d0
	rts


EndCode:

	END
