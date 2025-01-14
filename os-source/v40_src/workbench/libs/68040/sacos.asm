*
* $Id: sacos.asm,v 0.4 91/07/11 09:50:07 mks Exp $
*
* $Log:	sacos.asm,v $
* Revision 0.4  91/07/11  09:50:07  mks
* Removed all conditional code for the FMOVEM assembler bug.
* HX68 V.81 is the minimum version needed to correctly assembly this code.
* 
* Revision 0.3  91/07/08  15:48:28  mks
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
* Revision 0.2  91/07/02  14:40:17  mks
* Added conditional kludges for FMOVEM bug fix
*
* Revision 0.1  91/06/26  09:00:53  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	sacos.sa 3.3 12/19/90
*
*	Description: The entry point sAcos computes the inverse cosine of
*		an input argument; sAcosd does the same except for denormalized
*		input.
*
*	Input: Double-extended number X in location pointed to
*		by address register a0.
*
*	Output: The value arccos(X) returned in floating-point register Fp0.
*
*	Accuracy and Monotonicity: The returned result is within 3 ulps in
*		64 significant bit, i.e. within 0.5001 ulp to 53 bits if the
*		result is subsequently rounded to double precision. The
*		result is provably monotonic in double precision.
*
*	Speed: The program sCOS takes approximately 310 cycles.
*
*	Algorithm:
*
*	ACOS
*	1. If |X| >= 1, go to 3.
*
*	2. (|X| < 1) Calculate acos(X) by
*		z := (1-X) / (1+X)
*		acos(X) = 2 * atan( sqrt(z) ).
*		Exit.
*
*	3. If |X| > 1, go to 5.
*
*	4. (|X| = 1) If X > 0, return 0. Otherwise, return Pi. Exit.
*
*	5. (|X| > 1) Generate an invalid operation by 0 * infinity.
*		Exit.
*

*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

SACOS	IDNT	2,1 Motorola 040 Floating Point Software Package

	section	8

PI	DC.L $40000000,$C90FDAA2,$2168C235,$00000000
PIBY2	DC.L $3FFF0000,$C90FDAA2,$2168C235,$00000000

	xref	t_operr
	xref	t_frcinx
	xref	satan

	xdef	sacosd
sacosd:
*--ACOS(X) = PI/2 FOR DENORMALIZED X
	fmove.l		d1,fpcr		...load user's rounding mode/precision
	FMOVE.X		PIBY2,FP0
	bra		t_frcinx

	xdef	sacos
sacos:
	FMOVE.X		(a0),FP0	...LOAD INPUT

	move.l		(a0),d0		...pack exponent with upper 16 fraction
	move.w		4(a0),d0
	ANDI.L		#$7FFFFFFF,D0
	CMPI.L		#$3FFF8000,D0
	BGE.B		ACOSBIG

*--THIS IS THE USUAL CASE, |X| < 1
*--ACOS(X) = 2 * ATAN(	SQRT( (1-X)/(1+X) )	)

	FMOVE.S		#:3F800000,FP1
	FADD.X		FP0,FP1	 	...1+X
	FNEG.X		FP0	 	... -X
	FADD.S		#:3F800000,FP0	...1-X
	FDIV.X		FP1,FP0	 	...(1-X)/(1+X)
	FSQRT.X		FP0		...SQRT((1-X)/(1+X))
	fmovem.x	fp0,(a0)	...overwrite input
	move.l		d1,-(sp)	;save original users fpcr
	clr.l		d1
	bsr		satan		...ATAN(SQRT([1-X]/[1+X]))
	fMOVE.L		(sp)+,fpcr	;restore users exceptions
	FADD.X		FP0,FP0	 	...2 * ATAN( STUFF )
	bra		t_frcinx

ACOSBIG:
	FABS.X		FP0
	FCMP.S		#:3F800000,FP0
	fbgt		t_operr		;cause an operr exception

*--|X| = 1, ACOS(X) = 0 OR PI
	move.l		(a0),d0		...pack exponent with upper 16 fraction
	move.w		4(a0),d0
	CMP.L		#0,D0		;D0 has original exponent+fraction
	BGT.B		ACOSP1

*--X = -1
*Returns PI and inexact exception
	FMOVE.X		PI,FP0
	FMOVE.L		d1,FPCR
	FADD.S		#:00800000,FP0	;cause an inexact exception to be put
*					;into the 040 - will not trap until next
*					;fp inst.
	bra		t_frcinx

ACOSP1:
	FMOVE.L		d1,FPCR
	FMOVE.S		#:00000000,FP0
	rts				;Facos of +1 is exact

	end
