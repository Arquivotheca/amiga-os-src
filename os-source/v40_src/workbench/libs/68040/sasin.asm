*
* $Id: sasin.asm,v 0.4 91/07/11 09:49:54 mks Exp $
*
* $Log:	sasin.asm,v $
* Revision 0.4  91/07/11  09:49:54  mks
* Removed all conditional code for the FMOVEM assembler bug.
* HX68 V.81 is the minimum version needed to correctly assembly this code.
* 
* Revision 0.3  91/07/08  15:48:17  mks
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
* Revision 0.2  91/07/02  14:40:02  mks
* Added conditional kludges for FMOVEM bug fix
*
* Revision 0.1  91/06/26  09:00:49  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	sasin.sa 3.3 12/19/90
*
*	Description: The entry point sAsin computes the inverse sine of
*		an input argument; sAsind does the same except for denormalized
*		input.
*
*	Input: Double-extended number X in location pointed to
*		by address register a0.
*
*	Output: The value arcsin(X) returned in floating-point register Fp0.
*
*	Accuracy and Monotonicity: The returned result is within 3 ulps in
*		64 significant bit, i.e. within 0.5001 ulp to 53 bits if the
*		result is subsequently rounded to double precision. The
*		result is provably monotonic in double precision.
*
*	Speed: The program sASIN takes approximately 310 cycles.
*
*	Algorithm:
*
*	ASIN
*	1. If |X| >= 1, go to 3.
*
*	2. (|X| < 1) Calculate asin(X) by
*		z := sqrt( [1-X][1+X] )
*		asin(X) = atan( x / z ).
*		Exit.
*
*	3. If |X| > 1, go to 5.
*
*	4. (|X| = 1) sgn := sign(X), return asin(X) := sgn * Pi/2. Exit.
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

SASIN	IDNT	2,1 Motorola 040 Floating Point Software Package

	section	8

PIBY2	DC.L $3FFF0000,$C90FDAA2,$2168C235,$00000000

	xref	t_operr
	xref	t_frcinx
	xref	t_extdnrm
	xref	satan

	xdef	sasind
sasind:
*--ASIN(X) = X FOR DENORMALIZED X

	bra		t_extdnrm

	xdef	sasin
sasin:
	FMOVE.X		(a0),FP0	...LOAD INPUT

	move.l		(a0),d0
	move.w		4(a0),d0
	ANDI.L		#$7FFFFFFF,D0
	CMPI.L		#$3FFF8000,D0
	BGE.B		asinbig

*--THIS IS THE USUAL CASE, |X| < 1
*--ASIN(X) = ATAN( X / SQRT( (1-X)(1+X) ) )

	FMOVE.S		#:3F800000,FP1
	FSUB.X		FP0,FP1		...1-X
	fmovem.x	fp2,-(a7)
	FMOVE.S		#:3F800000,FP2
	FADD.X		FP0,FP2		...1+X
	FMUL.X		FP2,FP1		...(1+X)(1-X)
	fmovem.x	(a7)+,fp2
	FSQRT.X		FP1		...SQRT([1-X][1+X])
	FDIV.X		FP1,FP0	 	...X/SQRT([1-X][1+X])
	fmovem.x	fp0,(a0)
	bsr		satan
	bra		t_frcinx

asinbig:
	FABS.X		FP0	 ...|X|
	FCMP.S		#:3F800000,FP0
	fbgt		t_operr		;cause an operr exception

*--|X| = 1, ASIN(X) = +- PI/2.

	FMOVE.X		PIBY2,FP0
	move.l		(a0),d0
	ANDI.L		#$80000000,D0	...SIGN BIT OF X
	ORI.L		#$3F800000,D0	...+-1 IN SGL FORMAT
	MOVE.L		D0,-(sp)	...push SIGN(X) IN SGL-FMT
	FMOVE.L		d1,FPCR
	FMUL.S		(sp)+,FP0
	bra		t_frcinx

	end
