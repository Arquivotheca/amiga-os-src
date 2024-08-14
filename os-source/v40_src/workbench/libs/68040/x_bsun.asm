*
* $Id: x_bsun.asm,v 0.3 91/07/03 10:29:08 mks Exp $
*
* $Log:	x_bsun.asm,v $
* Revision 0.3  91/07/03  10:29:08  mks
* First pass at doing branch optimizations
* 
* Revision 0.2  91/06/26  13:23:37  mks
* Removed calls to "check_force" as per MOTO FAX of 6/14/91
*
* Revision 0.1  91/06/26  08:59:04  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	x_bsun.sa 3.2 4/26/91
*
*	fpsp_bsun --- FPSP handler for branch/set on unordered exception
*
*	Copy the PC to FPIAR to maintain 881/882 compatability
*
*	The real_bsun handler will need to perform further corrective
*	measures as outlined in the 040 User's Manual on pages
*	9-41f, section 9.8.3.
*

*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

X_BSUN	IDNT    2,1 Motorola 040 Floating Point Software Package

	section	8

	include	"fpsp.i"

	xref	real_bsun

	xdef	fpsp_bsun
fpsp_bsun:
*
	link		a6,#-LOCAL_SIZE
	fsave		-(a7)
	movem.l		d0-d1/a0-a1,USER_DA(a6)
	fmovem.x	fp0-fp3,USER_FP0(a6)
	fmovem.l	fpcr/fpsr/fpiar,USER_FPCR(a6)

	move.l		EXC_PC(a6),USER_FPIAR(a6)
*
	movem.l		USER_DA(a6),d0-d1/a0-a1
	fmovem.x	USER_FP0(a6),fp0-fp3
	fmovem.l	USER_FPCR(a6),fpcr/fpsr/fpiar
	frestore	(a7)+
	unlk		a6
	bra		real_bsun
*
	end
