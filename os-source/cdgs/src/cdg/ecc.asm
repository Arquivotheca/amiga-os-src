**
**	$Id: ecc.asm,v 1.15 92/03/05 15:37:08 darren Exp $
**
**	CDTV CD+G -- ecc.asm (Reed-Solomon Error Correction)
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	Based on code provided by Alan Havemose	
**

** Includes

		INCLUDE	"cdgbase.i"
		INCLUDE	"cdgprefs.i"
		INCLUDE	"debug.i"
		SECTION	cdg

** Exports

		XDEF	DecodeRS

** Imports

** Assumptions

** Equates

LOCLEN		EQU	4
ERRNUM		EQU	2
STOPNUM		EQU	4
GF64		EQU	64

P_LEN		EQU	24
P_RANK		EQU	4

** Structures

    STRUCTURE RSLocals,0
	STRUCT	rsl_T,LOCLEN		;ordered so we can quickly clear
	STRUCT	rsl_B,LOCLEN
	STRUCT	rsl_Gamma,LOCLEN
	STRUCT	rsl_SynVec,LOCLEN
	STRUCT	rsl_ErrorPos,ERRNUM
	STRUCT	rsl_Omega,2*LOCLEN
	STRUCT	rsl_GamDif,LOCLEN
	STRUCT	rsl_ErrorVal,ERRNUM
	LABEL	RSLocals_SIZEOF

** Macros

SYN_CALC	MACRO	;Hp[] based on lookup table

		move.w	#(\1<<6),d0
		add.b	-(a3),d0		;plus rec[NCol-1-i]
		move.b	0(a2,d0.w),d0		;_GF_Mul table
		eor.b	d0,d6			;add it

		ENDM

	; optimized for j=0!!! because the table in _GF_Mul is 0-63

SYN_CALC_0	MACRO	;

		move.b	-(a3),d0
		eor.b	d0,d6

		ENDM

	; unrolled SynPoly_Mul()
	
SYNPOLY_MUL	MACRO	;i,j

		moveq	#00,d1
		move.b	\1(a0),d1		;x[i]
		lsl.w	#6,d1
		add.b	\2(a1),d1		;y[j]
		move.b	0(a2,d1.w),d1		;GF_Mul(..
		eor.b	d1,(\1+\2)(a3)		;a[i+j] = GF_Add(a[i+j]...

		ENDM

	; unrolled DiffPolynomial function

DIFFPOLN	MACRO	;i

		moveq	#00,d1
		move.b	(a1)+,d1
		lsl.w	#6,d1			;*64
		add.b	#((\1+1)%%2),d1		;(i+1)%2
		move.b	0(a2,d1.w),(a0)+	;diff[i] =

		ENDM

*****i* cdg.library/DecodeRS ******************************************
*
*   NAME
*	DecodeRS - Reed-Solomon Error Correction code
*   
*   SYNOPSIS
*	success = DecodeRS( data, pack )
*	D0                   A6    A4
*	CC
*
*	BOOL DecodeRS( struct CDGData *, struct CDGPack * );
*
*   FUNCTION
*	Error Check/Correct CDGPack data.
*
*   INPUTS
*	data - Pointer to CDGData structure.
*
*	pack - Pointer to a CDGPack structure.
*
*   RETURNS
*	success - TRUE if the PACK has no errors, or could be corrected.
*	FALSE indicates there are too many errors to correct.
*
*   NOTES
*	This function repairs errors in the PACK data pointed to
*	by A4.  Like any error detection scheme it can be fooled
*	by too many errors.
*
*	This code is based on C code provided by Alan Havemose,
*	and has been optimized for speed (not size).
*
*	Because of the subcode data rate of 75 PACKETS per second (300
*	PACKS per second), we need to be able to perform error correction
*	in a miniumum time of 2 milliseconds per PACK (500 packs per
*	second) to leave enough time for real work ... except we
*	really want to be able to perform this function in less than
*	1 millisecond per PACK to allow time for other work.
*
***********************************************************************

DecodeRS:

		btst	#EF3B_NOECC,cdd_ExtraFlags3(a6)
		bne	DRS_SkipIt		;returns OK PACK data

		movem.l	d2-d6/a2-a3/a5,-(sp)

		lea	-RSLocals_SIZEOF(sp),sp

		move.l	sp,a5

	; quickly clear/init Syndrome Polynomials

	IFNE	rsl_T
	FAIL	"rsl_T != 0; recode!"
	ENDC

	IFNE	rsl_B-4
	FAIL	"rsl_B != 4; recode!"
	ENDC

	IFNE	rsl_Gamma-8
	FAIL	"rsl_Gamma != 8; recode!"
	ENDC

		move.l	a5,a0


		moveq	#00,d2		;also LEN

		move.l	d2,(a0)+	;T[0,0,0,0]

		moveq	#01,d0
		ror.l	#8,d0

		move.l	d0,(a0)+	;B[1,0,0,0]
		move.l	d0,(a0)+	;Gamma[1,0,0,0]

	;
	; Syn_Calc() inline for speed here (a0 points to rsl_SynVec,
	; A4 pointst to the data)
	;
	; Syn_Calc(SynVec,(GF *)Hp,Rec,P_RANK,P_LEN);
	;
	;	   A0,    xx(PC),  A4, 4,     24
	;

	IFNE	rsl_SynVec-12
	FAIL	"rsl_SynVec != 12; recode!"
	ENDC

		lea	_GF_Mul(pc),a2	;preload table pointers

	; Syn[j] = 0;

		lea	P_LEN(a4),a3	;reset record pointer

		move.b	#00,d6
	; j=0

		; This is an optimized case since I know the table
		; Hp[] always returns 1 when j==0, and I know the
		; table for _GF_Mul returns 0-63 for row 1

		SYN_CALC_0	1	;i=0
		SYN_CALC_0	1	;i=1
		SYN_CALC_0	1	;i=2
		SYN_CALC_0	1	;i=3
		SYN_CALC_0	1	;i=4
		SYN_CALC_0	1	;i=5
		SYN_CALC_0	1	;i=6
		SYN_CALC_0	1	;i=7
		SYN_CALC_0	1	;i=8
		SYN_CALC_0	1	;i=9
		SYN_CALC_0	1	;i=10
		SYN_CALC_0	1	;i=11
		SYN_CALC_0	1	;i=12
		SYN_CALC_0	1	;i=13
		SYN_CALC_0	1	;i=14
		SYN_CALC_0	1	;i=15
		SYN_CALC_0	1	;i=16
		SYN_CALC_0	1	;i=17
		SYN_CALC_0	1	;i=18
		SYN_CALC_0	1	;i=19
		SYN_CALC_0	1	;i=20
		SYN_CALC_0	1	;i=21
		SYN_CALC_0	1	;i=22
		SYN_CALC_0	1	;i=23

	; store SynVec[j], and increment

		move.b	d6,(a0)+

	; Syn[j] = 0;

		lea	P_LEN(a4),a3	;reset record pointer

		move.b	#00,d6
	; j=1

		SYN_CALC_0	1	;i=0
		SYN_CALC	2	;i=1
		SYN_CALC	4	;i=2
		SYN_CALC	8	;i=3
		SYN_CALC	16	;i=4
		SYN_CALC	32	;i=5
		SYN_CALC	3	;i=6
		SYN_CALC	6	;i=7
		SYN_CALC	12	;i=8
		SYN_CALC	24	;i=9
		SYN_CALC	48	;i=10
		SYN_CALC	35	;i=11
		SYN_CALC	5	;i=12
		SYN_CALC	10	;i=13
		SYN_CALC	20	;i=14
		SYN_CALC	40	;i=15
		SYN_CALC	19	;i=16
		SYN_CALC	38	;i=17
		SYN_CALC	15	;i=18
		SYN_CALC	30	;i=19
		SYN_CALC	60	;i=20
		SYN_CALC	59	;i=21
		SYN_CALC	53	;i=22
		SYN_CALC	41	;i=23

	; store SynVec[j], and increment

		move.b	d6,(a0)+


	; Syn[j] = 0;

		lea	P_LEN(a4),a3	;reset record pointer

		move.b	#00,d6
	; j=2

		SYN_CALC_0	1	;i=0
		SYN_CALC	4	;i=1
		SYN_CALC	16	;i=2
		SYN_CALC	3	;i=3
		SYN_CALC	12	;i=4
		SYN_CALC	48	;i=5
		SYN_CALC	5	;i=6
		SYN_CALC	20	;i=7
		SYN_CALC	19	;i=8
		SYN_CALC	15	;i=9
		SYN_CALC	60	;i=10
		SYN_CALC	53	;i=11
		SYN_CALC	17	;i=12
		SYN_CALC	7	;i=13
		SYN_CALC	28	;i=14
		SYN_CALC	51	;i=15
		SYN_CALC	9	;i=16
		SYN_CALC	36	;i=17
		SYN_CALC	22	;i=18
		SYN_CALC	27	;i=19
		SYN_CALC	47	;i=20
		SYN_CALC	58	;i=21
		SYN_CALC	45	;i=22
		SYN_CALC	50	;i=23

	; store SynVec[j], and increment

		move.b	d6,(a0)+


	; Syn[j] = 0;

		lea	P_LEN(a4),a3	;reset record pointer

		move.b	#00,d6
	; j=3

		SYN_CALC_0	1	;i=0
		SYN_CALC	8	;i=1
		SYN_CALC	3	;i=2
		SYN_CALC	24	;i=3
		SYN_CALC	5	;i=4
		SYN_CALC	40	;i=5
		SYN_CALC	15	;i=6
		SYN_CALC	59	;i=7
		SYN_CALC	17	;i=8
		SYN_CALC	14	;i=9
		SYN_CALC	51	;i=10
		SYN_CALC	18	;i=11
		SYN_CALC	22	;i=12
		SYN_CALC	54	;i=13
		SYN_CALC	58	;i=14
		SYN_CALC	25	;i=15
		SYN_CALC	13	;i=16
		SYN_CALC	43	;i=17
		SYN_CALC	23	;i=18
		SYN_CALC	62	;i=19
		SYN_CALC	57	;i=20
		SYN_CALC_0	1	;i=21
		SYN_CALC	8	;i=22
		SYN_CALC	3	;i=23

	; store SynVec[j], and increment

		move.b	d6,(a0)+



	; do R=1 UNTIL R != 4
	;
	; D2 = Len
	; D3 = R

	;;	moveq	#00,d2		;already 0 above
		moveq	#00,d3

WhileDoLoop:
		addq.b	#1,d3		;R++

	; Calculate Discrepency
	;
	; CalcDisp(R, Len, *Gamma, *SynVec);

	;	for(j=Disp=0;j<=Len;j++)
	;	Disp=GF_Add(Disp,GF_Mul(Gamma[j],SynVec[R-j-1]));
	;
	;	return(Disp);
	;

		lea	rsl_Gamma(a5),a1
		lea	rsl_SynVec(a5),a0

		moveq	#00,d0		;Disp
		moveq	#00,d4		;j
		moveq	#00,d6		;index into SynVec[]

Calc_Discrepency:

		moveq	#00,d1
		move.b	(a1)+,d1	;D1 = Gamma[j]
		lsl.w	#6,d1		;*64

		move.b	d3,d6		;R-j-1
		sub.b	d4,d6
		subq.b	#1,d6	

		add.b	0(a0,d6.w),d1	;SynVec[R-j-1]

		move.b	0(a2,d1.w),d5	;GFAdd(Displ,GF_Mul(...
		eor.b	d5,d0

		addq.b	#1,d4		;j++
		cmp.b	d2,d4		;j > Len??
		bls.s	Calc_Discrepency
	; 
	;
	; If (Disp = CalcDisp...
	;
		tst.b	d0
		beq	NoDiscrepency

	; SynPoly_Copy(T,B)

		move.l	rsl_B(a5),d1
		lea	rsl_T(a5),a0
	;;	move.l	d1,(a0)		;not need since we are going to
					;do SynPoly_MulX(T) below
	; SynPoly_MulX(T)

		lsr.l	#8,d1
		move.l	d1,(a0)

	; ConstMul(T,Disp)
	;	   D1,D0

		moveq	#00,d1
		move.b	(a0),d1
		lsl.w	#6,d1		;*64
		add.b	d0,d1
		move.b	0(a2,d1.w),(a0)+

		moveq	#00,d1
		move.b	(a0),d1
		lsl.w	#6,d1		;*64
		add.b	d0,d1
		move.b	0(a2,d1.w),(a0)+

		moveq	#00,d1
		move.b	(a0),d1
		lsl.w	#6,d1		;*64
		add.b	d0,d1
		move.b	0(a2,d1.w),(a0)+

		moveq	#00,d1
		move.b	(a0),d1
		lsl.w	#6,d1		;*64
		add.b	d0,d1
		move.b	0(a2,d1.w),(a0)

	; SynPoly_Add(T,Gamma)

		move.l	rsl_Gamma(a5),d1
		eor.l	d1,rsl_T(a5)

	; if(2*Len <= R-1)

		move.w	d2,d1		;Len
		add.w	d1,d1		;calc Len*2

		move.w	d3,d6		;R-1
		subq.w	#1,d6

		cmp.w	d6,d1
		bhi.s	Len2GRM1

	; SynPoly_Copy(B,Gamma)

		lea	rsl_B(a5),a0
		lea	rsl_Gamma(a5),a1

		move.l	(a1),(a0)

	; ConstMul(B,GF_Inv(Disp))

		lea	_GF_Inv(pc),a3
		move.b	0(a3,d0.w),d0	;Disp = GF_Inv(Disp);

		moveq	#00,d1
		move.b	(a0),d1
		lsl.w	#6,d1		;*64
		add.b	d0,d1
		move.b	0(a2,d1.w),(a0)+

		moveq	#00,d1
		move.b	(a0),d1
		lsl.w	#6,d1		;*64
		add.b	d0,d1
		move.b	0(a2,d1.w),(a0)+

		moveq	#00,d1
		move.b	(a0),d1
		lsl.w	#6,d1		;*64
		add.b	d0,d1
		move.b	0(a2,d1.w),(a0)+

		moveq	#00,d1
		move.b	(a0),d1
		lsl.w	#6,d1		;*64
		add.b	d0,d1
		move.b	0(a2,d1.w),(a0)

	; SynPoly_Copy(Gamma,T)

		move.l	rsl_T(a5),(a1)

	; Len=R-Len

		move.l	d3,d0		;R to D0
		sub.l	d2,d0		;- Len
		move.l	d0,d2		;to Len
		bra.s	NextDoLoop

Len2GRM1:

	; SynPoly_Copy(Gamma,T)

		move.l	rsl_T(a5),rsl_Gamma(a5)

NoDiscrepency:

	; SynPoly_MulX(B)

		move.l	rsl_B(a5),d1
		lsr.l	#8,d1
		move.l	d1,rsl_B(a5)

NextDoLoop:

	; while (R != StopNum)

		cmp.b	#4,d3
		bne	WhileDoLoop

	; DegGam = SynPoly_Degree(Gamma)

		moveq	#03,d0
		lea	rsl_Gamma+4(a5),a0
CalcDegree:
		tst.b	-(a0)
		dbne	d0,CalcDegree

	PRINTF	DBG_FLOW,<'Deg=%ld Len=%ld'>,D0,D2

	; if(DegGam == Len)

		cmp.w	d0,d2
		bne.s	TooManyErrors

	; optimize for degree 0 (no errors)

		subq.w	#1,d0
		bpl.s	FindErrors

NoErrors:

	; exit
		lea	RSLocals_SIZEOF(sp),sp

		movem.l	(sp)+,d2-d6/a2-a3/a5
DRS_SkipIt:
		moveq	#01,d0			;CC set
		rts


TooManyErrors:

	DEBUG_TRAP	$92

	; exit
		lea	RSLocals_SIZEOF(sp),sp

		movem.l	(sp)+,d2-d6/a2-a3/a5

		moveq	#00,d0			;CC set
		rts

FindErrors:


	; if(ErrorNum = ErrorPositions(Gamma,DegGama,ErrorPos))
	;				a0     d0     a1
	; 

	; Degree not 0; find/fix errors
	;

		beq	Degree1
		subq.w	#1,d0
		beq	Degree2
		bra.s	TooManyErrors

FoundErrors:


	;1-2 errors found, so fix them
	;
	; ErrorValues(GF *Rec, GF *Gamma, GF *SynVec, GF *ErrorPos, int ErrorNum)
	;	         a4                                             d0
	;
	; This is so complicated I decided not to spend too much time
	; optimizing this, but the good part is we only need this code
	; when there is an error!
	;

		
	; SynPoly_Mul(GF *a, GF *x, GF *y)
	;                 a3	a0     a1

		lea	rsl_SynVec(a5),a0
		lea	rsl_Gamma(a5),a1
		lea	rsl_Omega(a5),a3

		;SynPoly_Clear2(a)

		moveq	#00,d1

		move.l	d1,(a3)
		move.l	d1,4(a3)

		; for(i=0;i<LOCLEN;i++)
		;  for(j=0;j<LOCLEN;j++)
		;   a[i+j]=GF_Add(a[i+j],GF_Mul(x[i],y[j]))


		; unrolled for speed

		SYNPOLY_MUL	0,0
		SYNPOLY_MUL	0,1
		SYNPOLY_MUL	0,2
		SYNPOLY_MUL	0,3

		SYNPOLY_MUL	1,0
		SYNPOLY_MUL	1,1
		SYNPOLY_MUL	1,2
		SYNPOLY_MUL	1,3

		SYNPOLY_MUL	2,0
		SYNPOLY_MUL	2,1
		SYNPOLY_MUL	2,2
		SYNPOLY_MUL	2,3

		SYNPOLY_MUL	3,0
		SYNPOLY_MUL	3,1
		SYNPOLY_MUL	3,2
		SYNPOLY_MUL	3,3


	; KillKoef(Omega,LOCLEN,2*LOCLEN) - zero upper part of polynomial
	;            a3   constant, constant

		moveq	#00,d1
		move.l	d1,4(a3)


	; DiffPolyn(GF *dif, GF *orig) -- Differentiate a polynomial
	;
	;	    GamDif,  Gamma
	;              a0	a1	

		lea	rsl_GamDif(a5),a0

		; for (i=0;i<LOCLEN-1;i++)   -- i == 0..3
		;  dif[i] = GF_Mul(orig[i+1],(i+1)%2)

		addq.w	#1,a1			;i+1;i<LOCLEN;i++

		DIFFPOLN	0
		DIFFPOLN	1
		DIFFPOLN	2

	; for(i=0;i<ErrorNum;i++)
	;           D0
	;
		moveq	#00,d2			;i

		lea	rsl_ErrorPos(a5),a0
		lea	_GF_Inv(pc),a1

Error1Loop:

		; ep = GF_Inv(ErrorPos[i]

		moveq	#00,d1
		move.b	(a0)+,d1		;ErrorPos[i]

		move.b	0(a1,d1.w),d5		;ep = GF_Inv(ErrorPos[i])
		lea	rsl_Omega+4(a5),a3

		;EvalPolyn degree 3

		move.b	-(a3),d1
		ext.w	d1
		lsl.w	#6,d1
		add.b	d5,d1

		move.b	0(a2,d1.w),d1	;GF_Mul(pp[3],elem)

		move.b	-(a3),d6
		eor.b	d1,d6		;GF_Add(pp[2],..

		ext.w	d6
		lsl.w	#6,d6
		add.b	d5,d6

		move.b	0(a2,d6.w),d1	;GF_Mul(...

		move.b	-(a3),d6
		eor.b	d1,d6		;GF_Add..

		ext.w	d6
		lsl.w	#6,d6
		add.b	d5,d6

		move.b	0(a2,d6.w),d1	;GF_Mul(...

		move.b	-(a3),d6
		eor.b	d6,d1		;return in D1 (and CC)

		ext.w	d1
		move.w	d1,d3			;D3 == ErrorVal[i]

		; evaluate Polynomial; degree 2 (inline for speed)

		lea	rsl_GamDif+3(a5),a3

		move.b	-(a3),d1
		ext.w	d1
		lsl.w	#6,d1
		add.b	d5,d1

		move.b	0(a2,d1.w),d1	;GF_Mul(pp[2],elem)

		move.b	-(a3),d6
		eor.b	d1,d6		;GF_Add(pp[1],GF_Mul(pp[2]...

		ext.w	d6
		lsl.w	#6,d6
		add.b	d5,d6

		move.b	0(a2,d6.w),d1	;GF_Mul(		
				
		move.b	-(a3),d6	;GF_Add(
		eor.b	d6,d1		;return in D1 (and CC)
					;trash D6

		; n = GF_Mul(n,ep)

		ext.w	d1
		lsl.w	#6,d1			;n*64
		add.b	d5,d1			;+ep

		move.b	0(a2,d1.w),d6		;n

		; n=GF_Inv(n)

		moveq	#00,d1
		move.b	d6,d1			;offset into GF_Inv[] table

		; ErrorVal[i] = GF_Mul(ErrorVal[i],n)

		lsl.w	#6,d3			;ErrorVal[i] * 64
		add.b	0(a1,d1.w),d3		;add GF_Inv(n)

		lea	rsl_ErrorVal(a5),a3	;cache

		move.b	0(a2,d3.w),0(a3,d2.w)	;ErrorVal[i] = GF_Mul...

		; i++.  is i<ErrorNum?

		addq.b	#1,d2
		cmp.b	d0,d2
		bcs	Error1Loop		;blt unsigned


	; Now actually correct errors in the PACK
	;
	; for(i=0;i<ErrorNum;i++
	;           D0
	; a3 = &ErrorVal[]
	; a0 = &ErrorPos[]
	;
	; D0 = decrementing i

		lea	_GF_Exp(pc),a1
		lea	rsl_ErrorPos(a5),a0
Error2Loop:

		moveq	#00,d1
		move.b	(a0)+,d1
		move.b	0(a1,d1.w),d1		;ep = Exp(ErrorPos[i])

		move.b	(a3)+,d3		;ErrorVal[i]

		moveq	#P_LEN-1,d2

		cmp.b	d2,d1
		bhi.s	outofPACK

		sub.b	d1,d2			;Rec[P_LEN-1-ep] ^= ErrorVal[i]

		eor.b	d3,0(a4,d2.w)

outofPACK:

	PRINTF	DBG_FLOW,<'CDG -- Fix byte %ld'>,D2

		subq.b	#1,d0
		bne.s	Error2Loop

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;
	;;; Drum-Roll!!!  We are done!
	;;;
		bra	NoErrors		;POOF - DONE!!!


	; for(elem = 0; elem < GF64_SIZE; elem++) elem = 0..63
	;
	;	if(EvalPolyn(Gamma,elem,degr))
	;	{
	;		ErrorPos[ErrorNum++]=GF_Inv(elem);
	;	}
	;

	; optimized code for Degree 1 errors
Degree1:
	PRINTF	DBG_FLOW,<'CDG -- Degree1'>

	DEBUG_TRAP	$94
		
		lea	rsl_ErrorPos(a5),a1	;pointer to error position array
		lea	rsl_Gamma(a5),a3	;&Gamma[0]
		lea	_GF_Inv(pc),a0

	; fast way to EvalPolyn() degree1
	;
	; ErrorPos[0] = GF_Mul(Inv[Gamma[0]],Gamma[1]);

		move.b	(a3)+,d0			;Gamma[0]
		lsl.w	#6,d0
		add.b	(a3),d0
		move.b	0(a2,d0.w),(a1)
		moveq	#01,d0
		bra	FoundErrors


	;
	; optimized code for Degree 2 errors
	;

Degree2:
	PRINTF	DBG_FLOW,<'CDG -- Degree2'>


	DEBUG_TRAP	$95

		lea	rsl_ErrorPos(a5),a1	;pointer to error position array
		lea	rsl_Gamma(a5),a3	;&Gamma[0]

		move.l	a5,-(sp)

		move.b	(a3)+,d6		;prefetch Gamma[0]
		move.b	(a3)+,d2		;prefetch Gamma[1]
		move.b	(a3),d0			;prefetch Gamma[2]

		lsl.w	#6,d0			;for Gamma[2]

		move.l	a2,a5
		add.w	d0,a5

		moveq	#00,d0			;error counter

		move.l	a2,a3			;GF_Mul(row,element++)

		moveq	#GF64-1,d5		;elem counter 0..63
		move.l	d5,d1			;cache

		bra.s	D2FirstLoop

D2BeginLoop:
		addq.w	#1,a3
D2FirstLoop:
		move.b	(a5)+,d3
		eor.b	d2,d3			;GF_Add Gamma[1]
		ext.w	d3
		lsl.w	#6,d3
		cmp.b	0(a3,d3.w),d6
		dbeq	d5,D2BeginLoop

		tst.w	d5
		bmi.s	Deg2NotFound

		sub.w	d5,d1

		lea	_GF_Inv(pc),a0
		add.w	d1,a0
		move.b	(a0),(a1)+
		moveq	#GF64-1,d1		;ready for next loop

		addq.b	#1,d0
		cmp.b	#2,d0			;backout if 2 errors found
		dbeq	d5,D2BeginLoop		;which is the MAX we can handle

Deg2NotFound:
		move.l	(sp)+,a5

		cmp.b	#2,d0
		beq	FoundErrors
		bra	TooManyErrors

		SECTION	DATA

**
** ECC lookup tables (converted from C to assembly).  Taken from
** tables provided by Alan Havemose
**

_GF_Log:
	dc.b 1,2,4,8,16,32,3,6,12,24
	dc.b 48,35,5,10,20,40,19,38,15,30
	dc.b 60,59,53,41,17,34,7,14,28,56
	dc.b 51,37,9,18,36,11,22,44,27,54
	dc.b 47,29,58,55,45,25,50,39,13,26
	dc.b 52,43,21,42,23,46,31,62,63,61
	dc.b 57,49,33,0

_GF_Exp:
	dc.b 63,0,1,6,2,12,7,26,3,32
	dc.b 13,35,8,48,27,18,4,24,33,16
	dc.b 14,52,36,54,9,45,49,38,28,41
	dc.b 19,56,5,62,25,11,34,31,17,47
	dc.b 15,23,53,51,37,44,55,40,10,61
	dc.b 46,30,50,22,39,43,29,60,42,21
	dc.b 20,59,57,58

_GF_Inv:
	dc.b 0,1,33,62,49,43,31,44,57,37
	dc.b 52,28,46,40,22,25,61,54,51,39
	dc.b 26,35,14,24,23,15,20,34,11,53
	dc.b 45,6,63,2,27,21,56,9,50,19
	dc.b 13,47,48,5,7,30,12,41,42,4
	dc.b 38,18,10,29,17,60,36,8,59,58
	dc.b 55,16,3,32

******************************************************************************
**
** Table is now hardcoded in Matrix Multiply
**
**_GF_Hp:
**	dc.b 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
**	dc.b 1,2,4,8,16,32,3,6,12,24,48,35,5,10,20,40,19,38,15,30,60,59,53,41
**	dc.b 1,4,16,3,12,48,5,20,19,15,60,53,17,7,28,51,9,36,22,27,47,58,45,50
**	dc.b 1,8,3,24,5,40,15,59,17,14,51,18,22,54,58,25,13,43,23,62,57,1,8,3


_GF_Mul:
	dc.b 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
	dc.b 0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,3,1,7,5,11,9,15,13,19,17,23,21,27,25,31,29,35,33,39,37,43,41,47,45,51,49,55,53,59,57,63,61
	dc.b 0,3,6,5,12,15,10,9,24,27,30,29,20,23,18,17,48,51,54,53,60,63,58,57,40,43,46,45,36,39,34,33,35,32,37,38,47,44,41,42,59,56,61,62,55,52,49,50,19,16,21,22,31,28,25,26,11,8,13,14,7,4,1,2
	dc.b 0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,3,7,11,15,19,23,27,31,35,39,43,47,51,55,59,63,6,2,14,10,22,18,30,26,38,34,46,42,54,50,62,58,5,1,13,9,21,17,29,25,37,33,45,41,53,49,61,57
	dc.b 0,5,10,15,20,17,30,27,40,45,34,39,60,57,54,51,19,22,25,28,7,2,13,8,59,62,49,52,47,42,37,32,38,35,44,41,50,55,56,61,14,11,4,1,26,31,16,21,53,48,63,58,33,36,43,46,29,24,23,18,9,12,3,6
	dc.b 0,6,12,10,24,30,20,18,48,54,60,58,40,46,36,34,35,37,47,41,59,61,55,49,19,21,31,25,11,13,7,1,5,3,9,15,29,27,17,23,53,51,57,63,45,43,33,39,38,32,42,44,62,56,50,52,22,16,26,28,14,8,2,4
	dc.b 0,7,14,9,28,27,18,21,56,63,54,49,36,35,42,45,51,52,61,58,47,40,33,38,11,12,5,2,23,16,25,30,37,34,43,44,57,62,55,48,29,26,19,20,1,6,15,8,22,17,24,31,10,13,4,3,46,41,32,39,50,53,60,59
	dc.b 0,8,16,24,32,40,48,56,3,11,19,27,35,43,51,59,6,14,22,30,38,46,54,62,5,13,21,29,37,45,53,61,12,4,28,20,44,36,60,52,15,7,31,23,47,39,63,55,10,2,26,18,42,34,58,50,9,1,25,17,41,33,57,49
	dc.b 0,9,18,27,36,45,54,63,11,2,25,16,47,38,61,52,22,31,4,13,50,59,32,41,29,20,15,6,57,48,43,34,44,37,62,55,8,1,26,19,39,46,53,60,3,10,17,24,58,51,40,33,30,23,12,5,49,56,35,42,21,28,7,14
	dc.b 0,10,20,30,40,34,60,54,19,25,7,13,59,49,47,37,38,44,50,56,14,4,26,16,53,63,33,43,29,23,9,3,15,5,27,17,39,45,51,57,28,22,8,2,52,62,32,42,41,35,61,55,1,11,21,31,58,48,46,36,18,24,6,12
	dc.b 0,11,22,29,44,39,58,49,27,16,13,6,55,60,33,42,54,61,32,43,26,17,12,7,45,38,59,48,1,10,23,28,47,36,57,50,3,8,21,30,52,63,34,41,24,19,14,5,25,18,15,4,53,62,35,40,2,9,20,31,46,37,56,51
	dc.b 0,12,24,20,48,60,40,36,35,47,59,55,19,31,11,7,5,9,29,17,53,57,45,33,38,42,62,50,22,26,14,2,10,6,18,30,58,54,34,46,41,37,49,61,25,21,1,13,15,3,23,27,63,51,39,43,44,32,52,56,28,16,4,8
	dc.b 0,13,26,23,52,57,46,35,43,38,49,60,31,18,5,8,21,24,15,2,33,44,59,54,62,51,36,41,10,7,16,29,42,39,48,61,30,19,4,9,1,12,27,22,53,56,47,34,63,50,37,40,11,6,17,28,20,25,14,3,32,45,58,55
	dc.b 0,14,28,18,56,54,36,42,51,61,47,33,11,5,23,25,37,43,57,55,29,19,1,15,22,24,10,4,46,32,50,60,9,7,21,27,49,63,45,35,58,52,38,40,2,12,30,16,44,34,48,62,20,26,8,6,31,17,3,13,39,41,59,53
	dc.b 0,15,30,17,60,51,34,45,59,52,37,42,7,8,25,22,53,58,43,36,9,6,23,24,14,1,16,31,50,61,44,35,41,38,55,56,21,26,11,4,18,29,12,3,46,33,48,63,28,19,2,13,32,47,62,49,39,40,57,54,27,20,5,10
	dc.b 0,16,32,48,3,19,35,51,6,22,38,54,5,21,37,53,12,28,44,60,15,31,47,63,10,26,42,58,9,25,41,57,24,8,56,40,27,11,59,43,30,14,62,46,29,13,61,45,20,4,52,36,23,7,55,39,18,2,50,34,17,1,49,33
	dc.b 0,17,34,51,7,22,37,52,14,31,44,61,9,24,43,58,28,13,62,47,27,10,57,40,18,3,48,33,21,4,55,38,56,41,26,11,63,46,29,12,54,39,20,5,49,32,19,2,36,53,6,23,35,50,1,16,42,59,8,25,45,60,15,30
	dc.b 0,18,36,54,11,25,47,61,22,4,50,32,29,15,57,43,44,62,8,26,39,53,3,17,58,40,30,12,49,35,21,7,27,9,63,45,16,2,52,38,13,31,41,59,6,20,34,48,55,37,19,1,60,46,24,10,33,51,5,23,42,56,14,28
	dc.b 0,19,38,53,15,28,41,58,30,13,56,43,17,2,55,36,60,47,26,9,51,32,21,6,34,49,4,23,45,62,11,24,59,40,29,14,52,39,18,1,37,54,3,16,42,57,12,31,7,20,33,50,8,27,46,61,25,10,63,44,22,5,48,35
	dc.b 0,20,40,60,19,7,59,47,38,50,14,26,53,33,29,9,15,27,39,51,28,8,52,32,41,61,1,21,58,46,18,6,30,10,54,34,13,25,37,49,56,44,16,4,43,63,3,23,17,5,57,45,2,22,42,62,55,35,31,11,36,48,12,24
	dc.b 0,21,42,63,23,2,61,40,46,59,4,17,57,44,19,6,31,10,53,32,8,29,34,55,49,36,27,14,38,51,12,25,62,43,20,1,41,60,3,22,16,5,58,47,7,18,45,56,33,52,11,30,54,35,28,9,15,26,37,48,24,13,50,39
	dc.b 0,22,44,58,27,13,55,33,54,32,26,12,45,59,1,23,47,57,3,21,52,34,24,14,25,15,53,35,2,20,46,56,29,11,49,39,6,16,42,60,43,61,7,17,48,38,28,10,50,36,30,8,41,63,5,19,4,18,40,62,31,9,51,37
	dc.b 0,23,46,57,31,8,49,38,62,41,16,7,33,54,15,24,63,40,17,6,32,55,14,25,1,22,47,56,30,9,48,39,61,42,19,4,34,53,12,27,3,20,45,58,28,11,50,37,2,21,44,59,29,10,51,36,60,43,18,5,35,52,13,26
	dc.b 0,24,48,40,35,59,19,11,5,29,53,45,38,62,22,14,10,18,58,34,41,49,25,1,15,23,63,39,44,52,28,4,20,12,36,60,55,47,7,31,17,9,33,57,50,42,2,26,30,6,46,54,61,37,13,21,27,3,43,51,56,32,8,16
	dc.b 0,25,50,43,39,62,21,12,13,20,63,38,42,51,24,1,26,3,40,49,61,36,15,22,23,14,37,60,48,41,2,27,52,45,6,31,19,10,33,56,57,32,11,18,30,7,44,53,46,55,28,5,9,16,59,34,35,58,17,8,4,29,54,47
	dc.b 0,26,52,46,43,49,31,5,21,15,33,59,62,36,10,16,42,48,30,4,1,27,53,47,63,37,11,17,20,14,32,58,23,13,35,57,60,38,8,18,2,24,54,44,41,51,29,7,61,39,9,19,22,12,34,56,40,50,28,6,3,25,55,45
	dc.b 0,27,54,45,47,52,25,2,29,6,43,48,50,41,4,31,58,33,12,23,21,14,35,56,39,60,17,10,8,19,62,37,55,44,1,26,24,3,46,53,42,49,28,7,5,30,51,40,13,22,59,32,34,57,20,15,16,11,38,61,63,36,9,18
	dc.b 0,28,56,36,51,47,11,23,37,57,29,1,22,10,46,50,9,21,49,45,58,38,2,30,44,48,20,8,31,3,39,59,18,14,42,54,33,61,25,5,55,43,15,19,4,24,60,32,27,7,35,63,40,52,16,12,62,34,6,26,13,17,53,41
	dc.b 0,29,58,39,55,42,13,16,45,48,23,10,26,7,32,61,25,4,35,62,46,51,20,9,52,41,14,19,3,30,57,36,50,47,8,21,5,24,63,34,31,2,37,56,40,53,18,15,43,54,17,12,28,1,38,59,6,27,60,33,49,44,11,22
	dc.b 0,30,60,34,59,37,7,25,53,43,9,23,14,16,50,44,41,55,21,11,18,12,46,48,28,2,32,62,39,57,27,5,17,15,45,51,42,52,22,8,36,58,24,6,31,1,35,61,56,38,4,26,3,29,63,33,13,19,49,47,54,40,10,20
	dc.b 0,31,62,33,63,32,1,30,61,34,3,28,2,29,60,35,57,38,7,24,6,25,56,39,4,27,58,37,59,36,5,26,49,46,15,16,14,17,48,47,12,19,50,45,51,44,13,18,8,23,54,41,55,40,9,22,53,42,11,20,10,21,52,43
	dc.b 0,32,3,35,6,38,5,37,12,44,15,47,10,42,9,41,24,56,27,59,30,62,29,61,20,52,23,55,18,50,17,49,48,16,51,19,54,22,53,21,60,28,63,31,58,26,57,25,40,8,43,11,46,14,45,13,36,4,39,7,34,2,33,1
	dc.b 0,33,1,32,2,35,3,34,4,37,5,36,6,39,7,38,8,41,9,40,10,43,11,42,12,45,13,44,14,47,15,46,16,49,17,48,18,51,19,50,20,53,21,52,22,55,23,54,24,57,25,56,26,59,27,58,28,61,29,60,30,63,31,62
	dc.b 0,34,7,37,14,44,9,43,28,62,27,57,18,48,21,55,56,26,63,29,54,20,49,19,36,6,35,1,42,8,45,15,51,17,52,22,61,31,58,24,47,13,40,10,33,3,38,4,11,41,12,46,5,39,2,32,23,53,16,50,25,59,30,60
	dc.b 0,35,5,38,10,41,15,44,20,55,17,50,30,61,27,56,40,11,45,14,34,1,39,4,60,31,57,26,54,21,51,16,19,48,22,53,25,58,28,63,7,36,2,33,13,46,8,43,59,24,62,29,49,18,52,23,47,12,42,9,37,6,32,3
	dc.b 0,36,11,47,22,50,29,57,44,8,39,3,58,30,49,21,27,63,16,52,13,41,6,34,55,19,60,24,33,5,42,14,54,18,61,25,32,4,43,15,26,62,17,53,12,40,7,35,45,9,38,2,59,31,48,20,1,37,10,46,23,51,28,56
	dc.b 0,37,9,44,18,55,27,62,36,1,45,8,54,19,63,26,11,46,2,39,25,60,16,53,47,10,38,3,61,24,52,17,22,51,31,58,4,33,13,40,50,23,59,30,32,5,41,12,29,56,20,49,15,42,6,35,57,28,48,21,43,14,34,7
	dc.b 0,38,15,41,30,56,17,55,60,26,51,21,34,4,45,11,59,29,52,18,37,3,42,12,7,33,8,46,25,63,22,48,53,19,58,28,43,13,36,2,9,47,6,32,23,49,24,62,14,40,1,39,16,54,31,57,50,20,61,27,44,10,35,5
	dc.b 0,39,13,42,26,61,23,48,52,19,57,30,46,9,35,4,43,12,38,1,49,22,60,27,31,56,18,53,5,34,8,47,21,50,24,63,15,40,2,37,33,6,44,11,59,28,54,17,62,25,51,20,36,3,41,14,10,45,7,32,16,55,29,58
	dc.b 0,40,19,59,38,14,53,29,15,39,28,52,41,1,58,18,30,54,13,37,56,16,43,3,17,57,2,42,55,31,36,12,60,20,47,7,26,50,9,33,51,27,32,8,21,61,6,46,34,10,49,25,4,44,23,63,45,5,62,22,11,35,24,48
	dc.b 0,41,17,56,34,11,51,26,7,46,22,63,37,12,52,29,14,39,31,54,44,5,61,20,9,32,24,49,43,2,58,19,28,53,13,36,62,23,47,6,27,50,10,35,57,16,40,1,18,59,3,42,48,25,33,8,21,60,4,45,55,30,38,15
	dc.b 0,42,23,61,46,4,57,19,31,53,8,34,49,27,38,12,62,20,41,3,16,58,7,45,33,11,54,28,15,37,24,50,63,21,40,2,17,59,6,44,32,10,55,29,14,36,25,51,1,43,22,60,47,5,56,18,30,52,9,35,48,26,39,13
	dc.b 0,43,21,62,42,1,63,20,23,60,2,41,61,22,40,3,46,5,59,16,4,47,17,58,57,18,44,7,19,56,6,45,31,52,10,33,53,30,32,11,8,35,29,54,34,9,55,28,49,26,36,15,27,48,14,37,38,13,51,24,12,39,25,50
	dc.b 0,44,27,55,54,26,45,1,47,3,52,24,25,53,2,46,29,49,6,42,43,7,48,28,50,30,41,5,4,40,31,51,58,22,33,13,12,32,23,59,21,57,14,34,35,15,56,20,39,11,60,16,17,61,10,38,8,36,19,63,62,18,37,9
	dc.b 0,45,25,52,50,31,43,6,39,10,62,19,21,56,12,33,13,32,20,57,63,18,38,11,42,7,51,30,24,53,1,44,26,55,3,46,40,5,49,28,61,16,36,9,15,34,22,59,23,58,14,35,37,8,60,17,48,29,41,4,2,47,27,54
	dc.b 0,46,31,49,62,16,33,15,63,17,32,14,1,47,30,48,61,19,34,12,3,45,28,50,2,44,29,51,60,18,35,13,57,23,38,8,7,41,24,54,6,40,25,55,56,22,39,9,4,42,27,53,58,20,37,11,59,21,36,10,5,43,26,52
	dc.b 0,47,29,50,58,21,39,8,55,24,42,5,13,34,16,63,45,2,48,31,23,56,10,37,26,53,7,40,32,15,61,18,25,54,4,43,35,12,62,17,46,1,51,28,20,59,9,38,52,27,41,6,14,33,19,60,3,44,30,49,57,22,36,11
	dc.b 0,48,35,19,5,53,38,22,10,58,41,25,15,63,44,28,20,36,55,7,17,33,50,2,30,46,61,13,27,43,56,8,40,24,11,59,45,29,14,62,34,18,1,49,39,23,4,52,60,12,31,47,57,9,26,42,54,6,21,37,51,3,16,32
	dc.b 0,49,33,16,1,48,32,17,2,51,35,18,3,50,34,19,4,53,37,20,5,52,36,21,6,55,39,22,7,54,38,23,8,57,41,24,9,56,40,25,10,59,43,26,11,58,42,27,12,61,45,28,13,60,44,29,14,63,47,30,15,62,46,31
	dc.b 0,50,39,21,13,63,42,24,26,40,61,15,23,37,48,2,52,6,19,33,57,11,30,44,46,28,9,59,35,17,4,54,43,25,12,62,38,20,1,51,49,3,22,36,60,14,27,41,31,45,56,10,18,32,53,7,5,55,34,16,8,58,47,29
	dc.b 0,51,37,22,9,58,44,31,18,33,55,4,27,40,62,13,36,23,1,50,45,30,8,59,54,5,19,32,63,12,26,41,11,56,46,29,2,49,39,20,25,42,60,15,16,35,53,6,47,28,10,57,38,21,3,48,61,14,24,43,52,7,17,34
	dc.b 0,52,43,31,21,33,62,10,42,30,1,53,63,11,20,32,23,35,60,8,2,54,41,29,61,9,22,34,40,28,3,55,46,26,5,49,59,15,16,36,4,48,47,27,17,37,58,14,57,13,18,38,44,24,7,51,19,39,56,12,6,50,45,25
	dc.b 0,53,41,28,17,36,56,13,34,23,11,62,51,6,26,47,7,50,46,27,22,35,63,10,37,16,12,57,52,1,29,40,14,59,39,18,31,42,54,3,44,25,5,48,61,8,20,33,9,60,32,21,24,45,49,4,43,30,2,55,58,15,19,38
	dc.b 0,54,47,25,29,43,50,4,58,12,21,35,39,17,8,62,55,1,24,46,42,28,5,51,13,59,34,20,16,38,63,9,45,27,2,52,48,6,31,41,23,33,56,14,10,60,37,19,26,44,53,3,7,49,40,30,32,22,15,57,61,11,18,36
	dc.b 0,55,45,26,25,46,52,3,50,5,31,40,43,28,6,49,39,16,10,61,62,9,19,36,21,34,56,15,12,59,33,22,13,58,32,23,20,35,57,14,63,8,18,37,38,17,11,60,42,29,7,48,51,4,30,41,24,47,53,2,1,54,44,27
	dc.b 0,56,51,11,37,29,22,46,9,49,58,2,44,20,31,39,18,42,33,25,55,15,4,60,27,35,40,16,62,6,13,53,36,28,23,47,1,57,50,10,45,21,30,38,8,48,59,3,54,14,5,61,19,43,32,24,63,7,12,52,26,34,41,17
	dc.b 0,57,49,8,33,24,16,41,1,56,48,9,32,25,17,40,2,59,51,10,35,26,18,43,3,58,50,11,34,27,19,42,4,61,53,12,37,28,20,45,5,60,52,13,36,29,21,44,6,63,55,14,39,30,22,47,7,62,54,15,38,31,23,46
	dc.b 0,58,55,13,45,23,26,32,25,35,46,20,52,14,3,57,50,8,5,63,31,37,40,18,43,17,28,38,6,60,49,11,39,29,16,42,10,48,61,7,62,4,9,51,19,41,36,30,21,47,34,24,56,2,15,53,12,54,59,1,33,27,22,44
	dc.b 0,59,53,14,41,18,28,39,17,42,36,31,56,3,13,54,34,25,23,44,11,48,62,5,51,8,6,61,26,33,47,20,7,60,50,9,46,21,27,32,22,45,35,24,63,4,10,49,37,30,16,43,12,55,57,2,52,15,1,58,29,38,40,19
	dc.b 0,60,59,7,53,9,14,50,41,21,18,46,28,32,39,27,17,45,42,22,36,24,31,35,56,4,3,63,13,49,54,10,34,30,25,37,23,43,44,16,11,55,48,12,62,2,5,57,51,15,8,52,6,58,61,1,26,38,33,29,47,19,20,40
	dc.b 0,61,57,4,49,12,8,53,33,28,24,37,16,45,41,20,1,60,56,5,48,13,9,52,32,29,25,36,17,44,40,21,2,63,59,6,51,14,10,55,35,30,26,39,18,47,43,22,3,62,58,7,50,15,11,54,34,31,27,38,19,46,42,23
	dc.b 0,62,63,1,61,3,2,60,57,7,6,56,4,58,59,5,49,15,14,48,12,50,51,13,8,54,55,9,53,11,10,52,33,31,30,32,28,34,35,29,24,38,39,25,37,27,26,36,16,46,47,17,45,19,18,44,41,23,22,40,20,42,43,21
	dc.b 0,63,61,2,57,6,4,59,49,14,12,51,8,55,53,10,33,30,28,35,24,39,37,26,16,47,45,18,41,22,20,43,1,62,60,3,56,7,5,58,48,15,13,50,9,54,52,11,32,31,29,34,25,38,36,27,17,46,44,19,40,23,21,42

		END