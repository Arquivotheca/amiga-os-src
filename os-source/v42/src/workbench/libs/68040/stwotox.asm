*
* $Id: stwotox.asm,v 0.5 91/07/11 09:49:05 mks Exp $
*
* $Log:	stwotox.asm,v $
* Revision 0.5  91/07/11  09:49:05  mks
* Removed all conditional code for the FMOVEM assembler bug.
* HX68 V.81 is the minimum version needed to correctly assembly this code.
*
* Revision 0.4  91/07/08  15:47:32  mks
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
* Revision 0.3  91/07/03  10:28:34  mks
* First pass at doing branch optimizations
*
* Revision 0.2  91/07/02  14:37:44  mks
* Added conditional kludges for FMOVEM bug fix
*
* Revision 0.1  91/06/26  08:59:21  mks
* Initial check in...
*
*

	include	"assembly_options.i"

*
*	stwotox.sa 3.1 12/10/90
*
*	stwotox  --- 2**X
*	stwotoxd --- 2**X for denormalized X
*	stentox  --- 10**X
*	stentoxd --- 10**X for denormalized X
*
*	Input: Double-extended number X in location pointed to
*		by address register a0.
*
*	Output: The function values are returned in Fp0.
*
*	Accuracy and Monotonicity: The returned result is within 2 ulps in
*		64 significant bit, i.e. within 0.5001 ulp to 53 bits if the
*		result is subsequently rounded to double precision. The
*		result is provably monotonic in double precision.
*
*	Speed: The program stwotox takes approximately 190 cycles and the
*		program stentox takes approximately 200 cycles.
*
*	Algorithm:
*
*	twotox
*	1. If |X| > 16480, go to ExpBig.
*
*	2. If |X| < 2**(-70), go to ExpSm.
*
*	3. Decompose X as X = N/64 + r where |r| <= 1/128. Furthermore
*		decompose N as
*		 N = 64(M + M') + j,  j = 0,1,2,...,63.
*
*	4. Overwrite r := r * log2. Then
*		2**X = 2**(M') * 2**(M) * 2**(j/64) * exp(r).
*		Go to expr to compute that expression.
*
*	tentox
*	1. If |X| > 16480*log_10(2) (base 10 log of 2), go to ExpBig.
*
*	2. If |X| < 2**(-70), go to ExpSm.
*
*	3. Set y := X*log_2(10)*64 (base 2 log of 10). Set
*		N := round-to-int(y). Decompose N as
*		 N = 64(M + M') + j,  j = 0,1,2,...,63.
*
*	4. Define r as
*		r := ((X - N*L1)-N*L2) * L10
*		where L1, L2 are the leading and trailing parts of log_10(2)/64
*		and L10 is the natural log of 10. Then
*		10**X = 2**(M') * 2**(M) * 2**(j/64) * exp(r).
*		Go to expr to compute that expression.
*
*	expr
*	1. Fetch 2**(j/64) from table as Fact1 and Fact2.
*
*	2. Overwrite Fact1 and Fact2 by
*		Fact1 := 2**(M) * Fact1
*		Fact2 := 2**(M) * Fact2
*		Thus Fact1 + Fact2 = 2**(M) * 2**(j/64).
*
*	3. Calculate P where 1 + P approximates exp(r):
*		P = r + r*r*(A1+r*(A2+...+r*A5)).
*
*	4. Let AdjFact := 2**(M'). Return
*		AdjFact * ( Fact1 + ((Fact1*P) + Fact2) ).
*		Exit.
*
*	ExpBig
*	1. Generate overflow by Huge * Huge if X > 0; otherwise, generate
*		underflow by Tiny * Tiny.
*
*	ExpSm
*	1. Return 1 + X.
*

*		Copyright (C) Motorola, Inc. 1990
*			All Rights Reserved
*
*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MOTOROLA
*	The copyright notice above does not evidence any
*	actual or intended publication of such source code.

STWOTOX	IDNT	2,1 Motorola 040 Floating Point Software Package

	section	8

	include	"fpsp.i"

BOUNDS1	DC.L $3FB98000,$400D80C0 ... 2^(-70),16480
BOUNDS2	DC.L $3FB98000,$400B9B07 ... 2^(-70),16480 LOG2/LOG10

L2TEN64	DC.L $406A934F,$0979A371 ... 64LOG10/LOG2
L10TWO1	DC.L $3F734413,$509F8000 ... LOG2/64LOG10

L10TWO2	DC.L $BFCD0000,$C0219DC1,$DA994FD2,$00000000

LOG10	DC.L $40000000,$935D8DDD,$AAA8AC17,$00000000

LOG2	DC.L $3FFE0000,$B17217F7,$D1CF79AC,$00000000

EXPA5	DC.L $3F56C16D,$6F7BD0B2
EXPA4	DC.L $3F811112,$302C712C
EXPA3	DC.L $3FA55555,$55554CC1
EXPA2	DC.L $3FC55555,$55554A54
EXPA1	DC.L $3FE00000,$00000000,$00000000,$00000000

HUGE	DC.L $7FFE0000,$FFFFFFFF,$FFFFFFFF,$00000000
TINY	DC.L $00010000,$FFFFFFFF,$FFFFFFFF,$00000000

EXPTBL
	DC.L  $3FFF0000,$80000000,$00000000,$3F738000
	DC.L  $3FFF0000,$8164D1F3,$BC030773,$3FBEF7CA
	DC.L  $3FFF0000,$82CD8698,$AC2BA1D7,$3FBDF8A9
	DC.L  $3FFF0000,$843A28C3,$ACDE4046,$3FBCD7C9
	DC.L  $3FFF0000,$85AAC367,$CC487B15,$BFBDE8DA
	DC.L  $3FFF0000,$871F6196,$9E8D1010,$3FBDE85C
	DC.L  $3FFF0000,$88980E80,$92DA8527,$3FBEBBF1
	DC.L  $3FFF0000,$8A14D575,$496EFD9A,$3FBB80CA
	DC.L  $3FFF0000,$8B95C1E3,$EA8BD6E7,$BFBA8373
	DC.L  $3FFF0000,$8D1ADF5B,$7E5BA9E6,$BFBE9670
	DC.L  $3FFF0000,$8EA4398B,$45CD53C0,$3FBDB700
	DC.L  $3FFF0000,$9031DC43,$1466B1DC,$3FBEEEB0
	DC.L  $3FFF0000,$91C3D373,$AB11C336,$3FBBFD6D
	DC.L  $3FFF0000,$935A2B2F,$13E6E92C,$BFBDB319
	DC.L  $3FFF0000,$94F4EFA8,$FEF70961,$3FBDBA2B
	DC.L  $3FFF0000,$96942D37,$20185A00,$3FBE91D5
	DC.L  $3FFF0000,$9837F051,$8DB8A96F,$3FBE8D5A
	DC.L  $3FFF0000,$99E04593,$20B7FA65,$BFBCDE7B
	DC.L  $3FFF0000,$9B8D39B9,$D54E5539,$BFBEBAAF
	DC.L  $3FFF0000,$9D3ED9A7,$2CFFB751,$BFBD86DA
	DC.L  $3FFF0000,$9EF53260,$91A111AE,$BFBEBEDD
	DC.L  $3FFF0000,$A0B0510F,$B9714FC2,$3FBCC96E
	DC.L  $3FFF0000,$A2704303,$0C496819,$BFBEC90B
	DC.L  $3FFF0000,$A43515AE,$09E6809E,$3FBBD1DB
	DC.L  $3FFF0000,$A5FED6A9,$B15138EA,$3FBCE5EB
	DC.L  $3FFF0000,$A7CD93B4,$E965356A,$BFBEC274
	DC.L  $3FFF0000,$A9A15AB4,$EA7C0EF8,$3FBEA83C
	DC.L  $3FFF0000,$AB7A39B5,$A93ED337,$3FBECB00
	DC.L  $3FFF0000,$AD583EEA,$42A14AC6,$3FBE9301
	DC.L  $3FFF0000,$AF3B78AD,$690A4375,$BFBD8367
	DC.L  $3FFF0000,$B123F581,$D2AC2590,$BFBEF05F
	DC.L  $3FFF0000,$B311C412,$A9112489,$3FBDFB3C
	DC.L  $3FFF0000,$B504F333,$F9DE6484,$3FBEB2FB
	DC.L  $3FFF0000,$B6FD91E3,$28D17791,$3FBAE2CB
	DC.L  $3FFF0000,$B8FBAF47,$62FB9EE9,$3FBCDC3C
	DC.L  $3FFF0000,$BAFF5AB2,$133E45FB,$3FBEE9AA
	DC.L  $3FFF0000,$BD08A39F,$580C36BF,$BFBEAEFD
	DC.L  $3FFF0000,$BF1799B6,$7A731083,$BFBCBF51
	DC.L  $3FFF0000,$C12C4CCA,$66709456,$3FBEF88A
	DC.L  $3FFF0000,$C346CCDA,$24976407,$3FBD83B2
	DC.L  $3FFF0000,$C5672A11,$5506DADD,$3FBDF8AB
	DC.L  $3FFF0000,$C78D74C8,$ABB9B15D,$BFBDFB17
	DC.L  $3FFF0000,$C9B9BD86,$6E2F27A3,$BFBEFE3C
	DC.L  $3FFF0000,$CBEC14FE,$F2727C5D,$BFBBB6F8
	DC.L  $3FFF0000,$CE248C15,$1F8480E4,$BFBCEE53
	DC.L  $3FFF0000,$D06333DA,$EF2B2595,$BFBDA4AE
	DC.L  $3FFF0000,$D2A81D91,$F12AE45A,$3FBC9124
	DC.L  $3FFF0000,$D4F35AAB,$CFEDFA1F,$3FBEB243
	DC.L  $3FFF0000,$D744FCCA,$D69D6AF4,$3FBDE69A
	DC.L  $3FFF0000,$D99D15C2,$78AFD7B6,$BFB8BC61
	DC.L  $3FFF0000,$DBFBB797,$DAF23755,$3FBDF610
	DC.L  $3FFF0000,$DE60F482,$5E0E9124,$BFBD8BE1
	DC.L  $3FFF0000,$E0CCDEEC,$2A94E111,$3FBACB12
	DC.L  $3FFF0000,$E33F8972,$BE8A5A51,$3FBB9BFE
	DC.L  $3FFF0000,$E5B906E7,$7C8348A8,$3FBCF2F4
	DC.L  $3FFF0000,$E8396A50,$3C4BDC68,$3FBEF22F
	DC.L  $3FFF0000,$EAC0C6E7,$DD24392F,$BFBDBF4A
	DC.L  $3FFF0000,$ED4F301E,$D9942B84,$3FBEC01A
	DC.L  $3FFF0000,$EFE4B99B,$DCDAF5CB,$3FBE8CAC
	DC.L  $3FFF0000,$F281773C,$59FFB13A,$BFBCBB3F
	DC.L  $3FFF0000,$F5257D15,$2486CC2C,$3FBEF73A
	DC.L  $3FFF0000,$F7D0DF73,$0AD13BB9,$BFB8B795
	DC.L  $3FFF0000,$FA83B2DB,$722A033A,$3FBEF84B
	DC.L  $3FFF0000,$FD3E0C0C,$F486C175,$BFBEF581

N	equ	L_SCR1

X	equ	FP_SCR1
XDCARE	equ	X+2
XFRAC	equ	X+4

ADJFACT	equ	FP_SCR2

FACT1	equ	FP_SCR3
FACT1HI	equ	FACT1+4
FACT1LOW equ	FACT1+8

FACT2	equ	FP_SCR4
FACT2HI	equ	FACT2+4
FACT2LOW equ	FACT2+8

	xref	t_unfl
	xref	t_ovfl
	xref	t_frcinx

	xdef	stwotoxd
stwotoxd:
*--ENTRY POINT FOR 2**(X) FOR DENORMALIZED ARGUMENT

	fmove.l		d1,fpcr		...set user's rounding mode/precision
	Fmove.S		#:3F800000,FP0  ...RETURN 1 + X
	move.l		(a0),d0
	or.l		#$00800001,d0
	fadd.s		d0,fp0
	bra		t_frcinx

	xdef	stwotox
stwotox:
*--ENTRY POINT FOR 2**(X), HERE X IS FINITE, NON-ZERO, AND NOT NAN'S
	FMOVEM.X	(a0),FP0	...LOAD INPUT, do not set cc's
	MOVE.L		(A0),D0
	MOVE.W		4(A0),D0
	FMOVE.X		FP0,X(a6)
	ANDI.L		#$7FFFFFFF,D0

	CMPI.L		#$3FB98000,D0		...|X| >= 2**(-70)?
	BGE.B		TWOOK1
	BRA.W		EXPBORS

TWOOK1:
	CMPI.L		#$400D80C0,D0		...|X| > 16480?
	BLE.B		TWOMAIN
	BRA.W		EXPBORS


TWOMAIN:
*--USUAL CASE, 2^(-70) <= |X| <= 16480

	FMOVE.X		FP0,FP1
	FMUL.S		#:42800000,FP1  ...64 * X

	FMOVE.L		FP1,N(a6)		...N = ROUND-TO-INT(64 X)
	MOVE.L		d2,-(sp)
	LEA		EXPTBL,a1 	...LOAD ADDRESS OF TABLE OF 2^(J/64)
	FMOVE.L		N(a6),FP1		...N --> FLOATING FMT
	MOVE.L		N(a6),D0
	MOVE.L		D0,d2
	ANDI.L		#$3F,D0		...D0 IS J
	ASL.L		#4,D0		...DISPLACEMENT FOR 2^(J/64)
	ADDA.L		D0,a1		...ADDRESS FOR 2^(J/64)
	ASR.L		#6,d2		...d2 IS L, N = 64L + J
	MOVE.L		d2,D0
	ASR.L		#1,D0		...D0 IS M
	SUB.L		D0,d2		...d2 IS M', N = 64(M+M') + J
	ADDI.L		#$3FFF,d2
	MOVE.W		d2,ADJFACT(a6) 	...ADJFACT IS 2^(M')
	MOVE.L		(sp)+,d2
*--SUMMARY: a1 IS ADDRESS FOR THE LEADING PORTION OF 2^(J/64),
*--D0 IS M WHERE N = 64(M+M') + J. NOTE THAT |M| <= 16140 BY DESIGN.
*--ADJFACT = 2^(M').
*--REGISTERS SAVED SO FAR ARE (IN ORDER) FPCR, D0, FP1, a1, AND FP2.

	FMUL.S		#:3C800000,FP1  ...(1/64)*N
	MOVE.L		(a1)+,FACT1(a6)
	MOVE.L		(a1)+,FACT1HI(a6)
	MOVE.L		(a1)+,FACT1LOW(a6)
	MOVE.W		(a1)+,FACT2(a6)
	clr.w		FACT2+2(a6)

	FSUB.X		FP1,FP0	 	...X - (1/64)*INT(64 X)

	MOVE.W		(a1)+,FACT2HI(a6)
	clr.w		FACT2HI+2(a6)
	clr.l		FACT2LOW(a6)
	ADD.W		D0,FACT1(a6)

	FMUL.X		LOG2,FP0	...FP0 IS R
	ADD.W		D0,FACT2(a6)

	BRA.W		expr

EXPBORS:
*--FPCR, D0 SAVED
	CMPI.L		#$3FFF8000,D0
	BGT.B		EXPBIG

EXPSM:
*--|X| IS SMALL, RETURN 1 + X

	FMOVE.L		d1,FPCR		;restore users exceptions
	FADD.S		#:3F800000,FP0  ...RETURN 1 + X

	bra		t_frcinx

EXPBIG:
*--|X| IS LARGE, GENERATE OVERFLOW IF X > 0; ELSE GENERATE UNDERFLOW
*--REGISTERS SAVE SO FAR ARE FPCR AND  D0
	MOVE.L		X(a6),D0
	tst.L		D0
	BLT.B		EXPNEG

	bclr.b		#7,(a0)		;t_ovfl expects positive value
	bra		t_ovfl

EXPNEG:
	bclr.b		#7,(a0)		;t_unfl expects positive value
	bra		t_unfl

	xdef	stentoxd
stentoxd:
*--ENTRY POINT FOR 10**(X) FOR DENORMALIZED ARGUMENT

	fmove.l		d1,fpcr		...set user's rounding mode/precision
	Fmove.S		#:3F800000,FP0  ...RETURN 1 + X
	move.l		(a0),d0
	or.l		#$00800001,d0
	fadd.s		d0,fp0
	bra		t_frcinx

	xdef	stentox
stentox:
*--ENTRY POINT FOR 10**(X), HERE X IS FINITE, NON-ZERO, AND NOT NAN'S
	FMOVEM.X	(a0),FP0	...LOAD INPUT, do not set cc's
	MOVE.L		(A0),D0
	MOVE.W		4(A0),D0
	FMOVE.X		FP0,X(a6)
	ANDI.L		#$7FFFFFFF,D0

	CMPI.L		#$3FB98000,D0		...|X| >= 2**(-70)?
	BGE.B		TENOK1
	BRA.s		EXPBORS

TENOK1:
	CMPI.L		#$400B9B07,D0		...|X| <= 16480*log2/log10 ?
	BLE.B		TENMAIN
	BRA.s		EXPBORS

TENMAIN:
*--USUAL CASE, 2^(-70) <= |X| <= 16480 LOG 2 / LOG 10

	FMOVE.X		FP0,FP1
	FMUL.D		L2TEN64,FP1	...X*64*LOG10/LOG2

	FMOVE.L		FP1,N(a6)		...N=INT(X*64*LOG10/LOG2)
	MOVE.L		d2,-(sp)
	LEA		EXPTBL,a1 	...LOAD ADDRESS OF TABLE OF 2^(J/64)
	FMOVE.L		N(a6),FP1		...N --> FLOATING FMT
	MOVE.L		N(a6),D0
	MOVE.L		D0,d2
	ANDI.L		#$3F,D0		...D0 IS J
	ASL.L		#4,D0		...DISPLACEMENT FOR 2^(J/64)
	ADDA.L		D0,a1		...ADDRESS FOR 2^(J/64)
	ASR.L		#6,d2		...d2 IS L, N = 64L + J
	MOVE.L		d2,D0
	ASR.L		#1,D0		...D0 IS M
	SUB.L		D0,d2		...d2 IS M', N = 64(M+M') + J
	ADDI.L		#$3FFF,d2
	MOVE.W		d2,ADJFACT(a6) 	...ADJFACT IS 2^(M')
	MOVE.L		(sp)+,d2

*--SUMMARY: a1 IS ADDRESS FOR THE LEADING PORTION OF 2^(J/64),
*--D0 IS M WHERE N = 64(M+M') + J. NOTE THAT |M| <= 16140 BY DESIGN.
*--ADJFACT = 2^(M').
*--REGISTERS SAVED SO FAR ARE (IN ORDER) FPCR, D0, FP1, a1, AND FP2.

	FMOVE.X		FP1,FP2

	FMUL.D		L10TWO1,FP1	...N*(LOG2/64LOG10)_LEAD
	MOVE.L		(a1)+,FACT1(a6)

	FMUL.X		L10TWO2,FP2	...N*(LOG2/64LOG10)_TRAIL

	MOVE.L		(a1)+,FACT1HI(a6)
	MOVE.L		(a1)+,FACT1LOW(a6)
	FSUB.X		FP1,FP0		...X - N L_LEAD
	MOVE.W		(a1)+,FACT2(a6)

	FSUB.X		FP2,FP0		...X - N L_TRAIL

	clr.w		FACT2+2(a6)
	MOVE.W		(a1)+,FACT2HI(a6)
	clr.w		FACT2HI+2(a6)
	clr.l		FACT2LOW(a6)

	FMUL.X		LOG10,FP0	...FP0 IS R

	ADD.W		D0,FACT1(a6)
	ADD.W		D0,FACT2(a6)

expr:
*--FPCR, FP2, FP3 ARE SAVED IN ORDER AS SHOWN.
*--ADJFACT CONTAINS 2**(M'), FACT1 + FACT2 = 2**(M) * 2**(J/64).
*--FP0 IS R. THE FOLLOWING CODE COMPUTES
*--	2**(M'+M) * 2**(J/64) * EXP(R)

	FMOVE.X		FP0,FP1
	FMUL.X		FP1,FP1		...FP1 IS S = R*R

	FMOVE.D		EXPA5,FP2	...FP2 IS A5
	FMOVE.D		EXPA4,FP3	...FP3 IS A4

	FMUL.X		FP1,FP2		...FP2 IS S*A5
	FMUL.X		FP1,FP3		...FP3 IS S*A4

	FADD.D		EXPA3,FP2	...FP2 IS A3+S*A5
	FADD.D		EXPA2,FP3	...FP3 IS A2+S*A4

	FMUL.X		FP1,FP2		...FP2 IS S*(A3+S*A5)
	FMUL.X		FP1,FP3		...FP3 IS S*(A2+S*A4)

	FADD.D		EXPA1,FP2	...FP2 IS A1+S*(A3+S*A5)
	FMUL.X		FP0,FP3		...FP3 IS R*S*(A2+S*A4)

	FMUL.X		FP1,FP2		...FP2 IS S*(A1+S*(A3+S*A5))
	FADD.X		FP3,FP0		...FP0 IS R+R*S*(A2+S*A4)

	FADD.X		FP2,FP0		...FP0 IS EXP(R) - 1


*--FINAL RECONSTRUCTION PROCESS
*--EXP(X) = 2^M*2^(J/64) + 2^M*2^(J/64)*(EXP(R)-1)  -  (1 OR 0)

	FMUL.X		FACT1(a6),FP0
	FADD.X		FACT2(a6),FP0
	FADD.X		FACT1(a6),FP0

	FMOVE.L		d1,FPCR		;restore users exceptions
	clr.w		ADJFACT+2(a6)
	move.l		#$80000000,ADJFACT+4(a6)
	clr.l		ADJFACT+8(a6)
	FMUL.X		ADJFACT(a6),FP0	...FINAL ADJUSTMENT

	bra		t_frcinx

	end
