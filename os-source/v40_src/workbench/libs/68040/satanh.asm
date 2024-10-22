*
* $Id: satanh.asm,v 0.4 91/07/11 09:49:49 mks Exp $
*
* $Log:	satanh.asm,v $
* Revision 0.4  91/07/11  09:49:49  mks
* Removed all conditional code for the FMOVEM assembler bug.
* HX68 V.81 is the minimum version needed to correctly assembly this code.
* 
* Revision 0.3  91/07/08  15:48:12  mks
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
* Revision 0.2  91/07/02  14:39:50  mks
* Added conditional kludges for FMOVEM bug fix
*
* Revision 0.1  91/06/26  09:00:39  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	satanh.sa 3.3 12/19/90
*
*	The entry point satanh computes the inverse
*	hyperbolic tangent of
*	an input argument; satanhd does the same except for denormalized
*	input.
*
*	Input: Double-extended number X in location pointed to
*		by address register a0.
*
*	Output: The value arctanh(X) returned in floating-point register Fp0.
*
*	Accuracy and Monotonicity: The returned result is within 3 ulps in
*		64 significant bit, i.e. within 0.5001 ulp to 53 bits if the
*		result is subsequently rounded to double precision. The
*		result is provably monotonic in double precision.
*
*	Speed: The program satanh takes approximately 270 cycles.
*
*	Algorithm:
*
*	ATANH
*	1. If |X| >= 1, go to 3.
*
*	2. (|X| < 1) Calculate atanh(X) by
*		sgn := sign(X)
*		y := |X|
*		z := 2y/(1-y)
*		atanh(X) := sgn * (1/2) * logp1(z)
*		Exit.
*
*	3. If |X| > 1, go to 5.
*
*	4. (|X| = 1) Generate infinity with an appropriate sign and
*		divide-by-zero by
*		sgn := sign(X)
*		atan(X) := sgn / (+0).
*		Exit.
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

SATANH	IDNT	2,1 Motorola 040 Floating Point Software Package

	section	8

	xref	t_dz
	xref	t_operr
	xref	t_frcinx
	xref	t_extdnrm
	xref	slognp1

	xdef	satanhd
satanhd:
*--ATANH(X) = X FOR DENORMALIZED X

	bra		t_extdnrm

	xdef	satanh
satanh:
	move.l		(a0),d0
	move.w		4(a0),d0
	ANDI.L		#$7FFFFFFF,D0
	CMPI.L		#$3FFF8000,D0
	BGE.B		ATANHBIG

*--THIS IS THE USUAL CASE, |X| < 1
*--Y = |X|, Z = 2Y/(1-Y), ATANH(X) = SIGN(X) * (1/2) * LOG1P(Z).

	FABS.X		(a0),FP0	...Y = |X|
	FMOVE.X		FP0,FP1
	FNEG.X		FP1		...-Y
	FADD.X		FP0,FP0		...2Y
	FADD.S		#:3F800000,FP1	...1-Y
	FDIV.X		FP1,FP0		...2Y/(1-Y)
	move.l		(a0),d0
	ANDI.L		#$80000000,D0
	ORI.L		#$3F000000,D0	...SIGN(X)*HALF
	move.l		d0,-(sp)
	fmovem.x	fp0,(a0)	...overwrite input
	move.l		d1,-(sp)
	clr.l		d1
	bsr		slognp1		...LOG1P(Z)
	fmove.l		(sp)+,fpcr
	FMUL.S		(sp)+,FP0
	bra		t_frcinx

ATANHBIG:
	FABS.X		(a0),FP0	...|X|
	FCMP.S		#:3F800000,FP0
	fbgt		t_operr
	bra		t_dz

	end
