*
* $Id: sto_res.asm,v 0.5 91/07/11 09:51:09 mks Exp $
*
* $Log:	sto_res.asm,v $
* Revision 0.5  91/07/11  09:51:09  mks
* Removed all conditional code for the FMOVEM assembler bug.
* HX68 V.81 is the minimum version needed to correctly assembly this code.
* 
* Revision 0.4  91/07/08  15:47:27  mks
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
* Revision 0.3  91/07/02  14:38:37  mks
* Added conditional kludges for FMOVEM bug fix
*
* Revision 0.2  91/07/01  17:12:44  mks
* Hand fixed an assembler error on FMOVEM.x of single FPU registers...
*
* Revision 0.1  91/06/26  08:59:29  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	sto_res.sa 3.1 12/10/90
*
*	Takes the result and puts it in where the user expects it.
*	Library functions return result in fp0.	If fp0 is not the
*	users destination register then fp0 is moved to the the
*	correct floating-point destination register.  fp0 and fp1
*	are then restored to the original contents.
*
*	Input:	result in fp0,fp1
*
*		d2 & a0 should be kept unmodified
*
*	Output:	moves the result to the true destination reg or mem
*
*	Modifies: destination floating point register
*

*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

STO_RES	IDNT	2,1 Motorola 040 Floating Point Software Package


	section	8

	include	"fpsp.i"

	xdef	sto_cos
sto_cos:
	bfextu		CMDREG1B(a6){13:3},d0	;extract cos destination
	cmpi.b		#3,d0		;check for fp0/fp1 cases
	ble.b		c_fp0123
	fmovem.x	fp1,-(a7)
	moveq.l		#7,d1
	sub.l		d0,d1		;d1 = 7- (dest. reg. no.)
	clr.l		d0
	bset.l		d1,d0		;d0 is dynamic register mask
	fmovem.x	(a7)+,d0
	rts
c_fp0123:
	cmpi.b		#0,d0
	beq.b		c_is_fp0
	cmpi.b		#1,d0
	beq.b		c_is_fp1
	cmpi.b		#2,d0
	beq.b		c_is_fp2
c_is_fp3:
	fmovem.x	fp1,USER_FP3(a6)
	rts
c_is_fp2:
	fmovem.x	fp1,USER_FP2(a6)
	rts
c_is_fp1:
	fmovem.x	fp1,USER_FP1(a6)
	rts
c_is_fp0:
	fmovem.x	fp1,USER_FP0(a6)
	rts


	xdef	sto_res
sto_res:
	bfextu		CMDREG1B(a6){6:3},d0	;extract destination register
	cmpi.b		#3,d0		;check for fp0/fp1 cases
	ble.b		fp0123
	fmovem.x	fp0,-(a7)
	moveq.l		#7,d1
	sub.l		d0,d1		;d1 = 7- (dest. reg. no.)
	clr.l		d0
	bset.l		d1,d0		;d0 is dynamic register mask
	fmovem.x	(a7)+,d0
	rts
fp0123:
	cmpi.b		#0,d0
	beq.b		is_fp0
	cmpi.b		#1,d0
	beq.b		is_fp1
	cmpi.b		#2,d0
	beq.b		is_fp2
is_fp3:
	fmovem.x	fp0,USER_FP3(a6)
	rts
is_fp2:
	fmovem.x	fp0,USER_FP2(a6)
	rts
is_fp1:
	fmovem.x	fp0,USER_FP1(a6)
	rts
is_fp0:
	fmovem.x	fp0,USER_FP0(a6)
	rts

	end
