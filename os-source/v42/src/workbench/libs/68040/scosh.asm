*
* $Id: scosh.asm,v 0.4 91/07/11 09:49:43 mks Exp $
*
* $Log:	scosh.asm,v $
* Revision 0.4  91/07/11  09:49:43  mks
* Removed all conditional code for the FMOVEM assembler bug.
* HX68 V.81 is the minimum version needed to correctly assembly this code.
* 
* Revision 0.3  91/07/08  15:48:08  mks
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
* Revision 0.2  91/07/02  14:39:41  mks
* Added conditional kludges for FMOVEM bug fix
*
* Revision 0.1  91/06/26  09:00:35  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	scosh.sa 3.1 12/10/90
*
*	The entry point sCosh computes the hyperbolic cosine of
*	an input argument; sCoshd does the same except for denormalized
*	input.
*
*	Input: Double-extended number X in location pointed to
*		by address register a0.
*
*	Output: The value cosh(X) returned in floating-point register Fp0.
*
*	Accuracy and Monotonicity: The returned result is within 3 ulps in
*		64 significant bit, i.e. within 0.5001 ulp to 53 bits if the
*		result is subsequently rounded to double precision. The
*		result is provably monotonic in double precision.
*
*	Speed: The program sCOSH takes approximately 250 cycles.
*
*	Algorithm:
*
*	COSH
*	1. If |X| > 16380 log2, go to 3.
*
*	2. (|X| <= 16380 log2) Cosh(X) is obtained by the formulae
*		y = |X|, z = exp(Y), and
*		cosh(X) = (1/2)*( z + 1/z ).
*		Exit.
*
*	3. (|X| > 16380 log2). If |X| > 16480 log2, go to 5.
*
*	4. (16380 log2 < |X| <= 16480 log2)
*		cosh(X) = sign(X) * exp(|X|)/2.
*		However, invoking exp(|X|) may cause premature overflow.
*		Thus, we calculate sinh(X) as follows:
*		Y	:= |X|
*		Fact	:=	2**(16380)
*		Y'	:= Y - 16381 log2
*		cosh(X) := Fact * exp(Y').
*		Exit.
*
*	5. (|X| > 16480 log2) sinh(X) must overflow. Return
*		Huge*Huge to generate overflow and an infinity with
*		the appropriate sign. Huge is the largest finite number in
*		extended format. Exit.
*
*

*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

SCOSH	IDNT	2,1 Motorola 040 Floating Point Software Package

	section	8

	xref	t_ovfl
	xref	t_frcinx
	xref	setox

T1	DC.L $40C62D38,$D3D64634 ... 16381 LOG2 LEAD
T2	DC.L $3D6F90AE,$B1E75CC7 ... 16381 LOG2 TRAIL

TWO16380 DC.L $7FFB0000,$80000000,$00000000,$00000000

	xdef	scoshd
scoshd:
*--COSH(X) = 1 FOR DENORMALIZED X

	FMOVE.S		#:3F800000,FP0

	FMOVE.L		d1,FPCR
	FADD.S		#:00800000,FP0
	bra		t_frcinx

	xdef	scosh
scosh:
	FMOVE.X		(a0),FP0	...LOAD INPUT

	move.l		(a0),d0
	move.w		4(a0),d0
	ANDI.L		#$7FFFFFFF,d0
	CMPI.L		#$400CB167,d0
	BGT.B		COSHBIG

*--THIS IS THE USUAL CASE, |X| < 16380 LOG2
*--COSH(X) = (1/2) * ( EXP(X) + 1/EXP(X) )

	FABS.X		FP0		...|X|

	move.l		d1,-(sp)
	clr.l		d1
	fmovem.x	fp0,(a0)	;pass parameter to setox
	bsr		setox		...FP0 IS EXP(|X|)
	FMUL.S		#:3F000000,FP0	...(1/2)EXP(|X|)
	move.l		(sp)+,d1

	FMOVE.S		#:3E800000,FP1	...(1/4)
	FDIV.X		FP0,FP1	 	...1/(2 EXP(|X|))

	FMOVE.L		d1,FPCR
	FADD.X		fp1,FP0

	bra		t_frcinx

COSHBIG:
	CMPI.L		#$400CB2B3,d0
	BGT.B		COSHHUGE

	FABS.X		FP0
	FSUB.D		T1(pc),FP0		...(|X|-16381LOG2_LEAD)
	FSUB.D		T2(pc),FP0		...|X| - 16381 LOG2, ACCURATE

	move.l		d1,-(sp)
	clr.l		d1
	fmovem.x	fp0,(a0)
	bsr		setox
	fmove.l		(sp)+,fpcr

	FMUL.X		TWO16380(pc),FP0
	bra		t_frcinx

COSHHUGE:
	fmove.l		#0,fpsr		;clr N bit if set by source
	bclr.b		#7,(a0)		;always return positive value
	fmovem.x	(a0),fp0
	bra		t_ovfl

	end
