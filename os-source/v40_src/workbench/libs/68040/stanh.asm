*
* $Id: stanh.asm,v 0.5 91/07/11 09:49:36 mks Exp $
*
* $Log:	stanh.asm,v $
* Revision 0.5  91/07/11  09:49:36  mks
* Removed all conditional code for the FMOVEM assembler bug.
* HX68 V.81 is the minimum version needed to correctly assembly this code.
* 
* Revision 0.4  91/07/08  15:48:03  mks
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
* Revision 0.3  91/07/03  10:28:28  mks
* First pass at doing branch optimizations
*
* Revision 0.2  91/07/02  14:38:52  mks
* Added conditional kludges for FMOVEM bug fix
*
* Revision 0.1  91/06/26  08:59:32  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	stanh.sa 3.1 12/10/90
*
*	The entry point sTanh computes the hyperbolic tangent of
*	an input argument; sTanhd does the same except for denormalized
*	input.
*
*	Input: Double-extended number X in location pointed to
*		by address register a0.
*
*	Output: The value tanh(X) returned in floating-point register Fp0.
*
*	Accuracy and Monotonicity: The returned result is within 3 ulps in
*		64 significant bit, i.e. within 0.5001 ulp to 53 bits if the
*		result is subsequently rounded to double precision. The
*		result is provably monotonic in double precision.
*
*	Speed: The program stanh takes approximately 270 cycles.
*
*	Algorithm:
*
*	TANH
*	1. If |X| >= (5/2) log2 or |X| <= 2**(-40), go to 3.
*
*	2. (2**(-40) < |X| < (5/2) log2) Calculate tanh(X) by
*		sgn := sign(X), y := 2|X|, z := expm1(Y), and
*		tanh(X) = sgn*( z/(2+z) ).
*		Exit.
*
*	3. (|X| <= 2**(-40) or |X| >= (5/2) log2). If |X| < 1,
*		go to 7.
*
*	4. (|X| >= (5/2) log2) If |X| >= 50 log2, go to 6.
*
*	5. ((5/2) log2 <= |X| < 50 log2) Calculate tanh(X) by
*		sgn := sign(X), y := 2|X|, z := exp(Y),
*		tanh(X) = sgn - [ sgn*2/(1+z) ].
*		Exit.
*
*	6. (|X| >= 50 log2) Tanh(X) = +-1 (round to nearest). Thus, we
*		calculate Tanh(X) by
*		sgn := sign(X), Tiny := 2**(-126),
*		tanh(X) := sgn - sgn*Tiny.
*		Exit.
*
*	7. (|X| < 2**(-40)). Tanh(X) = X.	Exit.
*

*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

STANH	IDNT	2,1 Motorola 040 Floating Point Software Package

	section	8

	include "fpsp.i"

X	equ	FP_SCR5
XDCARE	equ	X+2
XFRAC	equ	X+4

SGN	equ	L_SCR3

V	equ	FP_SCR6

BOUNDS1	DC.L $3FD78000,$3FFFDDCE ... 2^(-40), (5/2)LOG2

	xref	t_frcinx
	xref	t_extdnrm
	xref	setox
	xref	setoxm1

	xdef	stanhd
stanhd:
*--TANH(X) = X FOR DENORMALIZED X

	bra		t_extdnrm

	xdef	stanh
stanh:
	FMOVE.X		(a0),FP0	...LOAD INPUT

	FMOVE.X		FP0,X(a6)
	move.l		(a0),d0
	move.w		4(a0),d0
	MOVE.L		D0,X(a6)
	AND.L		#$7FFFFFFF,D0
	CMP2.L		BOUNDS1(pc),D0	...2**(-40) < |X| < (5/2)LOG2 ?
	BCS.B		TANHBORS

*--THIS IS THE USUAL CASE
*--Y = 2|X|, Z = EXPM1(Y), TANH(X) = SIGN(X) * Z / (Z+2).

	MOVE.L		X(a6),D0
	MOVE.L		D0,SGN(a6)
	AND.L		#$7FFF0000,D0
	ADD.L		#$00010000,D0	...EXPONENT OF 2|X|
	MOVE.L		D0,X(a6)
	AND.L		#$80000000,SGN(a6)
	FMOVE.X		X(a6),FP0		...FP0 IS Y = 2|X|

	move.l		d1,-(a7)
	clr.l		d1
	fmovem.x	fp0,(a0)
	bsr		setoxm1	 	...FP0 IS Z = EXPM1(Y)
	move.l		(a7)+,d1

	FMOVE.X		FP0,FP1
	FADD.S		#:40000000,FP1	...Z+2
	MOVE.L		SGN(a6),D0
	FMOVE.X		FP1,V(a6)
	EOR.L		D0,V(a6)

	FMOVE.L		d1,FPCR		;restore users exceptions
	FDIV.X		V(a6),FP0
	bra		t_frcinx

TANHBORS:
	CMP.L		#$3FFF8000,D0
	BLT.s		TANHSM

	CMP.L		#$40048AA1,D0
	BGT.W		TANHHUGE

*-- (5/2) LOG2 < |X| < 50 LOG2,
*--TANH(X) = 1 - (2/[EXP(2X)+1]). LET Y = 2|X|, SGN = SIGN(X),
*--TANH(X) = SGN -	SGN*2/[EXP(Y)+1].

	MOVE.L		X(a6),D0
	MOVE.L		D0,SGN(a6)
	AND.L		#$7FFF0000,D0
	ADD.L		#$00010000,D0	...EXPO OF 2|X|
	MOVE.L		D0,X(a6)		...Y = 2|X|
	AND.L		#$80000000,SGN(a6)
	MOVE.L		SGN(a6),D0
	FMOVE.X		X(a6),FP0		...Y = 2|X|

	move.l		d1,-(a7)
	clr.l		d1
	fmovem.x	fp0,(a0)
	bsr		setox		...FP0 IS EXP(Y)
	move.l		(a7)+,d1
	move.l		SGN(a6),d0
	FADD.S		#:3F800000,FP0	...EXP(Y)+1

	EOR.L		#$C0000000,D0	...-SIGN(X)*2
	FMOVE.S		d0,FP1		...-SIGN(X)*2 IN SGL FMT
	FDIV.X		FP0,FP1	 	...-SIGN(X)2 / [EXP(Y)+1 ]

	MOVE.L		SGN(a6),D0
	OR.L		#$3F800000,D0	...SGN
	FMOVE.S		d0,FP0		...SGN IN SGL FMT

	FMOVE.L		d1,FPCR		;restore users exceptions
	FADD.X		fp1,FP0

	bra		t_frcinx

TANHSM:
	MOVE.W		#$0000,XDCARE(a6)

	FMOVE.L		d1,FPCR		;restore users exceptions
	FMOVE.X		X(a6),FP0		;last inst - possible exception set

	bra		t_frcinx

TANHHUGE:
*---RETURN SGN(X) - SGN(X)EPS
	MOVE.L		X(a6),D0
	AND.L		#$80000000,D0
	OR.L		#$3F800000,D0
	FMOVE.S		d0,FP0
	AND.L		#$80000000,D0
	EOR.L		#$80800000,D0	...-SIGN(X)*EPS

	FMOVE.L		d1,FPCR		;restore users exceptions
	FADD.S		d0,FP0

	bra		t_frcinx

	end
