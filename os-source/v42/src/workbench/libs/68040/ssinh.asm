*
* $Id: ssinh.asm,v 0.4 91/07/11 09:49:14 mks Exp $
*
* $Log:	ssinh.asm,v $
* Revision 0.4  91/07/11  09:49:14  mks
* Removed all conditional code for the FMOVEM assembler bug.
* HX68 V.81 is the minimum version needed to correctly assembly this code.
* 
* Revision 0.3  91/07/08  15:47:46  mks
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
* Revision 0.2  91/07/02  14:39:06  mks
* Added conditional kludges for FMOVEM bug fix
*
* Revision 0.1  91/06/26  08:59:42  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	ssinh.sa 3.1 12/10/90
*
*       The entry point sSinh computes the hyperbolic sine of
*       an input argument; sSinhd does the same except for denormalized
*       input.
*
*       Input: Double-extended number X in location pointed to
*		by address register a0.
*
*       Output: The value sinh(X) returned in floating-point register Fp0.
*
*       Accuracy and Monotonicity: The returned result is within 3 ulps in
*               64 significant bit, i.e. within 0.5001 ulp to 53 bits if the
*               result is subsequently rounded to double precision. The
*               result is provably monotonic in double precision.
*
*       Speed: The program sSINH takes approximately 280 cycles.
*
*       Algorithm:
*
*       SINH
*       1. If |X| > 16380 log2, go to 3.
*
*       2. (|X| <= 16380 log2) Sinh(X) is obtained by the formulae
*               y = |X|, sgn = sign(X), and z = expm1(Y),
*               sinh(X) = sgn*(1/2)*( z + z/(1+z) ).
*          Exit.
*
*       3. If |X| > 16480 log2, go to 5.
*
*       4. (16380 log2 < |X| <= 16480 log2)
*               sinh(X) = sign(X) * exp(|X|)/2.
*          However, invoking exp(|X|) may cause premature overflow.
*          Thus, we calculate sinh(X) as follows:
*             Y       := |X|
*             sgn     := sign(X)
*             sgnFact := sgn * 2**(16380)
*             Y'      := Y - 16381 log2
*             sinh(X) := sgnFact * exp(Y').
*          Exit.
*
*       5. (|X| > 16480 log2) sinh(X) must overflow. Return
*          sign(X)*Huge*Huge to generate overflow and an infinity with
*          the appropriate sign. Huge is the largest finite number in
*          extended format. Exit.
*

*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

SSINH	IDNT	2,1 Motorola 040 Floating Point Software Package

	section	8

T1	DC.L $40C62D38,$D3D64634 ... 16381 LOG2 LEAD
T2	DC.L $3D6F90AE,$B1E75CC7 ... 16381 LOG2 TRAIL

	xref	t_frcinx
	xref	t_ovfl
	xref	t_extdnrm
	xref	setox
	xref	setoxm1

	xdef	ssinhd
ssinhd:
*--SINH(X) = X FOR DENORMALIZED X

	bra	t_extdnrm

	xdef	ssinh
ssinh:
	FMOVE.x	(a0),FP0	...LOAD INPUT

	move.l	(a0),d0
	move.w	4(a0),d0
	move.l	d0,a1		save a copy of original (compacted) operand
	AND.L	#$7FFFFFFF,D0
	CMP.L	#$400CB167,D0
	BGT.B	SINHBIG

*--THIS IS THE USUAL CASE, |X| < 16380 LOG2
*--Y = |X|, Z = EXPM1(Y), SINH(X) = SIGN(X)*(1/2)*( Z + Z/(1+Z) )

	FABS.X	FP0		...Y = |X|

	movem.l	a1/d1,-(sp)
	fmovem.x	fp0,(a0)
	clr.l	d1
	bsr	setoxm1	 	...FP0 IS Z = EXPM1(Y)
	fmove.l	#0,fpcr
	movem.l	(sp)+,a1/d1

	FMOVE.X	FP0,FP1
	FADD.S	#:3F800000,FP1	...1+Z
	FMOVE.X	FP0,-(sp)
	FDIV.X	FP1,FP0		...Z/(1+Z)
	MOVE.L	a1,d0
	AND.L	#$80000000,D0
	OR.L	#$3F000000,D0
	FADD.X	(sp)+,FP0
	MOVE.L	D0,-(sp)

	fmove.l	d1,fpcr
	fmul.s	(sp)+,fp0	;last fp inst - possible exceptions set

	bra	t_frcinx

SINHBIG:
	cmp.l	#$400CB2B3,D0
	bgt	t_ovfl
	FABS.X	FP0
	FSUB.D	T1(pc),FP0	...(|X|-16381LOG2_LEAD)
	move.l	#0,-(sp)
	move.l	#$80000000,-(sp)
	move.l	a1,d0
	AND.L	#$80000000,D0
	OR.L	#$7FFB0000,D0
	MOVE.L	D0,-(sp)	...EXTENDED FMT
	FSUB.D	T2(pc),FP0	...|X| - 16381 LOG2, ACCURATE

	move.l	d1,-(sp)
	clr.l	d1
	fmovem.x	fp0,(a0)
	bsr	setox
	fmove.l	(sp)+,fpcr

	fmul.x	(sp)+,fp0	;possible exception
	bra	t_frcinx

	end
