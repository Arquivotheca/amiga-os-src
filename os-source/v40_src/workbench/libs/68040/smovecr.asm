*
* $Id: smovecr.asm,v 0.5 91/07/11 09:50:00 mks Exp $
*
* $Log:	smovecr.asm,v $
* Revision 0.5  91/07/11  09:50:00  mks
* Removed all conditional code for the FMOVEM assembler bug.
* HX68 V.81 is the minimum version needed to correctly assembly this code.
* 
* Revision 0.4  91/07/08  15:48:22  mks
* Ok, so this is an ugly hack to work around the FMOVEM problem...
* It turns out that the assembler just did not notice that the register was
* an FPU register unless there was a register list.  So...  we make all FMOVEMs
* that have only one FPU register, show it twice.  For example:
* fmovem.x  fp0,(a0)            This would become:
* fmovem.x  fp0/fp0,(a0)
*
* This fools the assembler into doing the right thing.  It is all within
* conditional assembly so that when a fixed assembler comes, it can easily
* be tested and the kludge removed.
*
* Revision 0.3  91/07/03  10:28:00  mks
* First pass at doing branch optimizations
*
* Revision 0.2  91/07/02  14:39:17  mks
* Added conditional kludges for FMOVEM bug fix
*
* Revision 0.1  91/06/26  09:00:03  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	smovecr.sa 3.1 12/10/90
*
*	The entry point sMOVECR returns the constant at the
*	offset given in the instruction field.
*
*	Input: An offset in the instruction word.
*
*	Output:	The constant rounded to the user's rounding
*		mode unchecked for overflow.
*
*	Modified: fp0.
*
*
*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

SMOVECR	IDNT	2,1 Motorola 040 Floating Point Software Package

	section 8

	include "fpsp.i"

	xref	nrm_set
	xref	round
	xref	PIRN
	xref	PIRZRM
	xref	PIRP
	xref	SMALRN
	xref	SMALRZRM
	xref	SMALRP
	xref	BIGRN
	xref	BIGRZRM
	xref	BIGRP

FZERO	dc.l	00000000
*
*	FMOVECR
*
	xdef	smovcr
smovcr:
	bfextu	CMDREG1B(a6){9:7},d0 ;get offset
	bfextu	USER_FPCR(a6){26:2},d1 ;get rmode
*
* check range of offset
*
	tst.b	d0		;if zero, offset is to pi
	beq.b	PI_TBL		;it is pi
	cmpi.b	#$0a,d0		;check range $01 - $0a
	ble.b	Z_VAL		;if in this range, return zero
	cmpi.b	#$0e,d0		;check range $0b - $0e
	ble.b	SM_TBL		;valid constants in this range
	cmpi.b	#$2f,d0		;check range $10 - $2f
	ble.b	Z_VAL		;if in this range, return zero
	cmpi.b	#$3f,d0		;check range $30 - $3f
	ble.s  	BG_TBL		;valid constants in this range
Z_VAL:
	fmove.s	FZERO,fp0
	rts
PI_TBL:
	tst.b	d1		;offset is zero, check for rmode
	beq.b	PI_RN		;if zero, rn mode
	cmpi.b	#$3,d1		;check for rp
	beq.b	PI_RP		;if 3, rp mode
PI_RZRM:
	lea.l	PIRZRM,a0	;rmode is rz or rm, load PIRZRM in a0
	bra	set_finx
PI_RN:
	lea.l	PIRN,a0		;rmode is rn, load PIRN in a0
	bra	set_finx
PI_RP:
	lea.l	PIRP,a0		;rmode is rp, load PIRP in a0
	bra	set_finx
SM_TBL:
	subi.l	#$b,d0		;make offset in 0 - 4 range
	tst.b	d1		;check for rmode
	beq.b	SM_RN		;if zero, rn mode
	cmpi.b	#$3,d1		;check for rp
	beq.b	SM_RP		;if 3, rp mode
SM_RZRM:
	lea.l	SMALRZRM,a0	;rmode is rz or rm, load SMRZRM in a0
	cmpi.b	#$2,d0		;check if result is inex
	ble.s	set_finx	;if 0 - 2, it is inexact
	bra.s	no_finx		;if 3, it is exact
SM_RN:
	lea.l	SMALRN,a0	;rmode is rn, load SMRN in a0
	cmpi.b	#$2,d0		;check if result is inex
	ble.s	set_finx	;if 0 - 2, it is inexact
	bra.s	no_finx		;if 3, it is exact
SM_RP:
	lea.l	SMALRP,a0	;rmode is rp, load SMRP in a0
	cmpi.b	#$2,d0		;check if result is inex
	ble.s	set_finx	;if 0 - 2, it is inexact
	bra.s	no_finx		;if 3, it is exact
BG_TBL:
	subi.l	#$30,d0		;make offset in 0 - f range
	tst.b	d1		;check for rmode
	beq.b	BG_RN		;if zero, rn mode
	cmpi.b	#$3,d1		;check for rp
	beq.b	BG_RP		;if 3, rp mode
BG_RZRM:
	lea.l	BIGRZRM,a0	;rmode is rz or rm, load BGRZRM in a0
	cmpi.b	#$1,d0		;check if result is inex
	ble.s	set_finx	;if 0 - 1, it is inexact
	cmpi.b	#$7,d0		;second check
	ble.s	no_finx		;if 0 - 7, it is exact
	bra.s	set_finx	;if 8 - f, it is inexact
BG_RN:
	lea.l	BIGRN,a0	;rmode is rn, load BGRN in a0
	cmpi.b	#$1,d0		;check if result is inex
	ble.s	set_finx	;if 0 - 1, it is inexact
	cmpi.b	#$7,d0		;second check
	ble.s	no_finx		;if 0 - 7, it is exact
	bra.s	set_finx	;if 8 - f, it is inexact
BG_RP:
	lea.l	BIGRP,a0	;rmode is rp, load SMRP in a0
	cmpi.b	#$1,d0		;check if result is inex
	ble.s	set_finx	;if 0 - 1, it is inexact
	cmpi.b	#$7,d0		;second check
	ble.s	no_finx		;if 0 - 7, it is exact
*	bra.s	set_finx	;if 8 - f, it is inexact
set_finx:
	or.l	#inx2a_mask,USER_FPSR(a6) ;set inex2/ainex
no_finx:
	mulu.l	#12,d0			;use offset to point into tables
	move.l	d1,L_SCR1(a6)		;load mode for round call
	bfextu	USER_FPCR(a6){24:2},d1	;get precision
	tst.l	d1			;check if extended precision
*
* Precision is extended
*
	bne.b	not_ext			;if extended, do not call round
	fmovem.x (a0,d0),fp0		;return result in fp0
	rts
*
* Precision is single or double
*
not_ext:
	swap	d1			;rnd prec in upper word of d1
	add.l	L_SCR1(a6),d1		;merge rmode in low word of d1
	move.l	(a0,d0),FP_SCR1(a6)	;load first word to temp storage
	move.l	4(a0,d0),FP_SCR1+4(a6)	;load second word
	move.l	8(a0,d0),FP_SCR1+8(a6)	;load third word
	clr.l	d0			;clear g,r,s
	lea	FP_SCR1(a6),a0
	btst.b	#sign_bit,LOCAL_EX(a0)
	sne	LOCAL_SGN(a0)		;convert to internal ext. format

	bsr	round			;go round the mantissa

	bfclr	LOCAL_SGN(a0){0:8}	;convert back to IEEE ext format
	beq.b	fin_fcr
	bset.b	#sign_bit,LOCAL_EX(a0)
fin_fcr:
	fmovem.x (a0),fp0
	rts

	end
