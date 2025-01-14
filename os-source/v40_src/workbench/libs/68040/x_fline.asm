*
* $Id: x_fline.asm,v 0.3 91/07/03 10:27:12 mks Exp $
*
* $Log:	x_fline.asm,v $
* Revision 0.3  91/07/03  10:27:12  mks
* First pass at doing branch optimizations
* 
* Revision 0.2  91/07/02  15:53:10  mks
* Did optimizations for unified memory since the Amiga system
* runs as unified address space.
*
* Revision 0.1  91/06/26  08:59:00  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	x_fline.sa 3.3 1/10/91
*
*	fpsp_fline --- FPSP handler for fline exception
*
*	First determine if the exception is one of the unimplemented
*	floating point instructions.  If so, let fpsp_unimp handle it.
*	Next, determine if the instruction is an fmovecr with a non-zero
*	<ea> field.  If so, handle here and return.  Otherwise, it
*	must be a real F-line exception.
*

*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

X_FLINE	IDNT    2,1 Motorola 040 Floating Point Software Package

	section	8

	include	"fpsp.i"

	xref	real_fline
	xref	fpsp_unimp
	xref	uni_2
	xref	mem_read
	xref	fpsp_fmt_error

	xdef	fpsp_fline
fpsp_fline:
*
*	check for unimplemented vector first.  Use EXC_VEC-4 because
*	the equate is valid only after a 'link a6' has pushed one more
*	long onto the stack.
*
	cmp.w	#UNIMP_VEC,EXC_VEC-4(a7)
	beq.l	fpsp_unimp

*
*	fmovecr with non-zero <ea> handling here
*
	sub.l	#4,a7		;4 accounts for 2-word difference
*				;between six word frame (unimp) and
*				;four word frame
	link	a6,#-LOCAL_SIZE
	fsave	-(a7)
	movem.l	d0-d1/a0-a1,USER_DA(a6)
	movea.l	EXC_PC+4(a6),a0	;get address of fline instruction

 ifne COPY_IN_OUT
	move.l	(a0),d0		; Get the fline and command word...
	move.l	d0,L_SCR1(a6)	; ?? This may not be needed... ??
 else
	lea.l	L_SCR1(a6),a1	;use L_SCR1 as scratch
	move.l	#4,d0
	add.l	#4,a6		;to offset the sub.l #4,a7 above so that
*				;a6 can point correctly to the stack frame
*				;before branching to mem_read
	bsr	mem_read
	sub.l	#4,a6
	move.l	L_SCR1(a6),d0	;d0 contains the fline and command word
 endc

	bfextu	d0{4:3},d1	;extract coprocessor id
	cmpi.b	#1,d1		;check if cpid=1
	bne.s	not_mvcr	;exit if not
	bfextu	d0{16:6},d1
	cmpi.b	#$17,d1		;check if it is an FMOVECR encoding
	bne.s	not_mvcr
*				;if an FMOVECR instruction, fix stack
*				;and go to FPSP_UNIMP
fix_stack:
	cmpi.b	#VER_40,(a7)	;test for orig unimp frame
	bne.b	ck_rev
	sub.l	#UNIMP_40_SIZE-4,a7 ;emulate an orig fsave
	move.b	#VER_40,(a7)
	move.b	#UNIMP_40_SIZE-4,1(a7)
	clr.w	2(a7)
	bra.b	fix_con
ck_rev:
	cmpi.b	#VER_41,(a7)	;test for rev unimp frame
	bne.l	fpsp_fmt_error	;if not $40 or $41, exit with error
	sub.l	#UNIMP_41_SIZE-4,a7 ;emulate a rev fsave
	move.b	#VER_41,(a7)
	move.b	#UNIMP_41_SIZE-4,1(a7)
	clr.w	2(a7)
fix_con:
	move.w	EXC_SR+4(a6),EXC_SR(a6) ;move stacked sr to new position
	move.l	EXC_PC+4(a6),EXC_PC(a6) ;move stacked pc to new position
	fmove.l	EXC_PC(a6),FPIAR ;point FPIAR to fline inst
	move.l	#4,d1
	add.l	d1,EXC_PC(a6)	;increment stacked pc value to next inst
	move.w	#$202c,EXC_VEC(a6) ;reformat vector to unimp
	clr.l	EXC_EA(a6)	;clear the EXC_EA field
	move.w	d0,CMDREG1B(a6) ;move the lower word into CMDREG1B
	clr.l	E_BYTE(a6)
	bset.b	#UFLAG,T_BYTE(a6)
	movem.l	USER_DA(a6),d0-d1/a0-a1 ;restore data registers
	bra	uni_2

not_mvcr:
	movem.l	USER_DA(a6),d0-d1/a0-a1 ;restore data registers
	frestore (a7)+
	unlk	a6
	add.l	#4,a7
	bra	real_fline

	end
