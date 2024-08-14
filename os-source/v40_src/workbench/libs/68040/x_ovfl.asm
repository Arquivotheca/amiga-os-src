*
* $Id: x_ovfl.asm,v 0.3 91/07/03 10:29:02 mks Exp $
*
* $Log:	x_ovfl.asm,v $
* Revision 0.3  91/07/03  10:29:02  mks
* First pass at doing branch optimizations
* 
* Revision 0.2  91/06/26  13:26:56  mks
* Removed calls to "check_force" as per MOTO FAX of 6/14/91
*
* Revision 0.1  91/06/26  08:58:50  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	x_ovfl.sa 3.4 4/26/91
*
*	fpsp_ovfl --- FPSP handler for overflow exception
*
*	Overflow occurs when a floating-point intermediate result is
*	too large to be represented in a floating-point data register,
*	or when storing to memory, the contents of a floating-point
*	data register are too large to be represented in the
*	destination format.
*
* Trap disabled results
*
* If the instruction is move_out, then garbage is stored in the
* destination.  If the instruction is not move_out, then the
* destination is not affected.  For 68881 compatibility, the
* following values should be stored at the destination, based
* on the current rounding mode:
*
*  RN	Infinity with the sign of the intermediate result.
*  RZ	Largest magnitude number, with the sign of the
*	intermediate result.
*  RM   For pos overflow, the largest pos number. For neg overflow,
*	-infinity
*  RP   For pos overflow, +infinity. For neg overflow, the largest
*	neg number
*
* Trap enabled results
* All trap disabled code applies.  In addition the exceptional
* operand needs to be made available to the users exception handler
* with a bias of $6000 subtracted from the exponent.
*
*

*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

X_OVFL	IDNT    2,1 Motorola 040 Floating Point Software Package

	section	8

	include	"fpsp.i"

	xref	ovf_r_x2
	xref	ovf_r_x3
	xref	store
	xref	real_ovfl
	xref	real_inex
	xref	fpsp_done
	xref	g_opcls
	xref	b1238_fix

	xdef	fpsp_ovfl
fpsp_ovfl:
	link		a6,#-LOCAL_SIZE
	fsave		-(a7)
	movem.l		d0-d1/a0-a1,USER_DA(a6)
	fmovem.x	fp0-fp3,USER_FP0(a6)
	fmovem.l	fpcr/fpsr/fpiar,USER_FPCR(a6)


*
*	The 040 doesn't set the AINEX bit in the FPSR, the following
*	line temporarily rectifies this error.
*
	bset.b	#ainex_bit,FPSR_AEXCEPT(a6)
*
	bsr	ovf_adj		;denormalize, round & store interm op
*
*	if overflow traps not enabled check for inexact exception
*
	btst.b	#ovfl_bit,FPCR_ENABLE(a6)
	beq.b	ck_inex
*
	btst.b		#E3,E_BYTE(a6)
	beq.b		no_e3_1
	bfextu		CMDREG3B(a6){6:3},d0	;get dest reg no
	bclr.b		d0,FPR_DIRTY_BITS(a6)	;clr dest dirty bit
	bsr		b1238_fix
	move.l		USER_FPSR(a6),FPSR_SHADOW(a6)
	or.l		#sx_mask,E_BYTE(a6)
no_e3_1:
	movem.l		USER_DA(a6),d0-d1/a0-a1
	fmovem.x	USER_FP0(a6),fp0-fp3
	fmovem.l	USER_FPCR(a6),fpcr/fpsr/fpiar
	frestore	(a7)+
	unlk		a6
	bra		real_ovfl
*
* It is possible to have either inex2 or inex1 exceptions with the
* ovfl.  If the inex enable bit is set in the FPCR, and either
* inex2 or inex1 occured, we must clean up and branch to the
* real inex handler.
*
ck_inex:
*	move.b		FPCR_ENABLE(a6),d0
*	and.b		FPSR_EXCEPT(a6),d0
*	andi.b		#$3,d0
	btst.b		#inex2_bit,FPCR_ENABLE(a6)
	beq.b		ovfl_exit
*
* Inexact enabled and reported, and we must take an inexact exception.
*
take_inex:
	btst.b		#E3,E_BYTE(a6)
	beq.b		no_e3_2
	bfextu		CMDREG3B(a6){6:3},d0	;get dest reg no
	bclr.b		d0,FPR_DIRTY_BITS(a6)	;clr dest dirty bit
	bsr		b1238_fix
	move.l		USER_FPSR(a6),FPSR_SHADOW(a6)
	or.l		#sx_mask,E_BYTE(a6)
no_e3_2:
	move.b		#INEX_VEC,EXC_VEC+1(a6)
	movem.l		USER_DA(a6),d0-d1/a0-a1
	fmovem.x	USER_FP0(a6),fp0-fp3
	fmovem.l	USER_FPCR(a6),fpcr/fpsr/fpiar
	frestore	(a7)+
	unlk		a6
	bra		real_inex

ovfl_exit:
	bclr.b	#E3,E_BYTE(a6)	;test and clear E3 bit
	beq.b	e1_set
*
* Clear dirty bit on dest resister in the frame before branching
* to b1238_fix.
*
	bfextu		CMDREG3B(a6){6:3},d0	;get dest reg no
	bclr.b		d0,FPR_DIRTY_BITS(a6)	;clr dest dirty bit
	bsr		b1238_fix		;test for bug1238 case

	move.l		USER_FPSR(a6),FPSR_SHADOW(a6)
	or.l		#sx_mask,E_BYTE(a6)
	movem.l		USER_DA(a6),d0-d1/a0-a1
	fmovem.x	USER_FP0(a6),fp0-fp3
	fmovem.l	USER_FPCR(a6),fpcr/fpsr/fpiar
	frestore	(a7)+
	unlk		a6
	bra		fpsp_done
e1_set:
	movem.l		USER_DA(a6),d0-d1/a0-a1
	fmovem.x	USER_FP0(a6),fp0-fp3
	fmovem.l	USER_FPCR(a6),fpcr/fpsr/fpiar
	unlk		a6
	bra		fpsp_done

*
*	ovf_adj
*
ovf_adj:
*
* Have a0 point to the correct operand.
*
	btst.b	#E3,E_BYTE(a6)	;test E3 bit
	beq.b	ovf_e1

	lea	WBTEMP(a6),a0
	bra.b	ovf_com
ovf_e1:
	lea	ETEMP(a6),a0

ovf_com:
	bclr.b	#sign_bit,LOCAL_EX(a0)
	sne	LOCAL_SGN(a0)

	bsr	g_opcls		;returns opclass in d0
	cmpi.w	#3,d0		;check for opclass3
	bne.b	not_opc011

*
* FPSR_CC is saved and restored because ovf_r_x3 affects it. The
* CCs are defined to be 'not affected' for the opclass3 instruction.
*
	move.b	FPSR_CC(a6),L_SCR1(a6)
 	bsr	ovf_r_x3	;returns a0 pointing to result
	move.b	L_SCR1(a6),FPSR_CC(a6)
	bra	store		;stores to memory or register

not_opc011:
	bsr	ovf_r_x2	;returns a0 pointing to result
	bra	store		;stores to memory or register

	end
