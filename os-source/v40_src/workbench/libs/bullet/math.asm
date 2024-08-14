**
**	$Id: math.asm,v 8.0 91/03/24 12:17:29 kodiak Exp $
**
**      math support
**
**      (C) Copyright 1991 Robert R. Burns
**          All Rights Reserved
**
	SECTION	bullet

**	Included Files

**	Exported Names

	XDEF	_LToD
	XDEF	_DLoToL
	XDEF	_DHiToL
	XDEF	_DNeg
	XDEF	_DAdd
	XDEF	_DSub
	XDEF	_DCmp
	XDEF	_DShift
	XDEF	_LMul
	XDEF	_DDiv


; - - -	LToD  - - - - - - - - - - - - - - - - - - - - - - - - - - - -
_LToD:
		moveq	#0,d0
		move.l	4(a7),d1
		bge.s	ltdlDone
		moveq	#-1,d0
ltdlDone:
		rts

; - - -	DLoToL  - - - - - - - - - - - - - - - - - - - - - - - - - - -
_DLoToL:
		move.l	8(a7),d0
		rts

; - - -	DHiToL  - - - - - - - - - - - - - - - - - - - - - - - - - - -
_DHiToL:
		move.l	4(a7),d0
		rts

; - - -	DNeg  - - - - - - - - - - - - - - - - - - - - - - - - - - - -
_DNeg:
		movem.l	4(a7),d0-d1
		neg.l	d1
		negx.l	d0
		rts	

; - - -	DAdd  - - - - - - - - - - - - - - - - - - - - - - - - - - - -
_DAdd:
		movem.l	d2-d3,-(a7)
		movem.l	12(a7),d0-d3
		add.l	d3,d1
		addx.l	d2,d0
		movem.l	(a7)+,d2-d3
		rts

; - - -	DSub  - - - - - - - - - - - - - - - - - - - - - - - - - - - -
_DSub:
		movem.l	d2-d3,-(a7)
		movem.l	12(a7),d0-d3
		sub.l	d3,d1
		subx.l	d2,d0
		movem.l	(a7)+,d2-d3
		rts

; - - -	DCmp  - - - - - - - - - - - - - - - - - - - - - - - - - - - -
_DCmp:
		movem.l	d2-d4,-(a7)
		moveq	#1,d0
		movem.l	16(a7),d1-d4
		sub.l	d4,d2
		subx.l	d3,d1
		bmi.s	dlNeg		; negative
		bne.s	dlDone		; positive
		tst.l	d2
		bne.s	dlDone		; positive
		addq.w	#1,d0		; zero
dlNeg:
		subq.w	#2,d0
dlDone:
		movem.l	(a7)+,d2-d4
		rts


; - - -	DShift  - - - - - - - - - - - - - - - - - - - - - - - - - - -
;	DShift(dl, leftShift)
_DShift:
		movem.l	d2-d4,-(a7)
		moveq	#0,d4
		movem.l	16(a7),d0-d1	; get double long
		move.w	26(a7),d2	; get shift value
		beq.s	dlsDone		; zero: no shift
		bpl.s	dlsLeft		; positive: left shift
		;-- right shift
		moveq	#31,d3		; convert -1..-31 to 31..1 while
		add.w	d2,d3		;   ensuring small shift
		blt.s	dlsLargeRight	;
		bset	d3,d4		; create crossover mask
		neg.l	d4		;
		add.l	d4,d4		;
		neg.w	d2		; get positive shift count
		lsr.l	d2,d1		; shift low longword
		ror.l	d2,d0		; rotate high longword
		move.l	d0,d3		; save rotated high longword
		and.l	d4,d3		; mask high crossover
		or.l	d3,d1		; combine crossover into low longword
		not.l	d4		; create longword mask
		and.l	d4,d0		; mask high longword
		bra.s	dlsDone

dlsLargeRight:
		neg.w	d2
		sub.w	#32,d2
		beq.s	dlsSwapRight
		cmp.w	#32,d2
		bge.s	dlsZero
		lsr.l	d2,d0
dlsSwapRight:
		move.l	d0,d1
dlsClearHi:
		moveq	#0,d0
		bra.s	dlsDone

dlsZero:
		moveq	#0,d1
		bra.s	dlsClearHi
		sub.w	#32,d2
		beq.s	dlsSwapRight
		cmp.w	#32,d2
		bge.s	dlsZero

dlsLargeLeft:
		beq.s	dlsSwapLeft
		sub.w	#32,d2
		cmp.w	#32,d2
		bge.s	dlsZero
		lsl.l	d2,d1
dlsSwapLeft:
		move.l	d1,d0
		moveq	#0,d1
		bra.s	dlsDone


		;-- left shift
dlsLeft:
		cmp.w	#32,d2
		bge.s	dlsLargeLeft

		lsl.l	d2,d0		; shift high longword
		rol.l	d2,d1		; rotate low longword
		move.l	d1,d3		; save rotated low longword
		bset	d2,d4		; create longword mask
		neg.l	d4		;
		and.l	d4,d1		; mask low longword
		not.l	d4		; create crossover mask
		and.l	d4,d3		; mask low crossover
		or.l	d3,d0		; combine crossover into high longword
dlsDone:
		movem.l	(a7)+,d2-d4
		rts
		
		
; - - -	LMul  - - - - - - - - - - - - - - - - - - - - - - - - - - - -
_LMul:
		movem.l	d2-d5,-(a7)
		movem.l	20(a7),d2-d3

		; find absolute value of arguments and compute final sign
		move.l	d2,d4
		bpl.s	lmPos1
		neg.l	d2
lmPos1:
		move.l	d3,d5
		bpl.s	lmPos2
		neg.l	d3
lmPos2:
		eor.l	d4,d5		; final sign in d5

		;-- compute partial products
		;			; d2   d3
		move.w	d2,d1
		mulu	d3,d1		; lo x lo in d1
		move.w	d2,d4
		swap	d2
		move.w	d2,d0
		mulu	d3,d2		; hi x lo in d2
		swap	d3
		mulu	d3,d0		; hi x hi in d0
		mulu	d4,d3		; lo x hi in d3

		;-- add partial products
					; d1    d0	^
					; lo*lo		d1
					;    hi*lo	d2
					;    lo*hi	d3
					;	hi*hi	d0
		moveq	#0,d4
		add.l	d3,d2		; mid: hi*lo + lo*hi
		swap	d0		; (take care of any carry)
		addx.w	d4,d0		;
		swap	d0		;

		swap	d1		; d1.hi += mid.lo
		add.w	d2,d1		;
		addx.l	d4,d0		; (take care of any carry)
		swap	d1

		clr.w	d2
		swap	d2		; d0.lo += mid.hi
		add.l	d2,d0		;

		;-- ensure sign is correct
		tst.l	d5
		bpl.s	lmDone
		neg.l	d1
		negx.l	d0

lmDone:
		movem.l	(a7)+,d2-d5
		rts

; - - -	DDiv  - - - - - - - - - - - - - - - - - - - - - - - - - - - -
_DDiv:
		movem.l	d2-d7,-(a7)
		moveq	#0,d6		; sign
		movem.l	28(a7),d2-d5	; numerator, denominator

		;-- test arguments
		tst.l	d2
		bpl.s	dldPosNumerator
		neg.l	d3
		negx.l	d2
		not.w	d6
		bra.s	dldNonZero
dldPosNumerator:
		bne.s	dldNonZero
		tst.l	d3
		beq.s	dldZero		; numerator zero: result zero
dldNonZero:
		tst.l	d4
		bpl.s	dldPosDenominator
		neg.l	d5
		negx.l	d4
		not.w	d6
		bra.s	dldNotError
dldPosDenominator:
		bne.s	dldNotError
		tst.l	d5
		beq.s	dldZero		; divide by zero: error (result zero)

dldNotError:
		;-- round result
		move.l	d4,d0		; get denominator
		move.l	d5,d1		;
		lsr.l	#1,d0		; divide by 2
		roxr.l	#1,d1		;
		add.l	d1,d3		; add to numerator
		addx.l	d0,d2		;


		;-- count number of bits in quotient
dldQCnt:
		moveq	#0,d7		; initialize bit count

		;-- shift denominator to the left while numerator >= denominator
dldQCLoop:
		cmp.l	d2,d4		; compare high longs
		bhi.s	dldQCUndo	; numerator < denominator
		bne.s	dldQCShiftLeft	; numerator > denominator
		cmp.l	d3,d5		; compare low longs
		bhi.s	dldQCUndo	; numerator < denominator
dldQCShiftLeft:
		lsl.l	#1,d5		; shift denominator left
		roxl.l	#1,d4		;
		addq.w	#1,d7		; bump bit count
		bra.s	dldQCLoop

		;-- undo last shift
dldQCUndo:
		lsr.l	#1,d4
		roxr.l	#1,d5

		;-- initialize result and enter quotient loop
		moveq	#0,d0		; initialize zero result
		moveq	#0,d1		;
		bra.s	dldQDBF

		;-- quotient loop
dldQLoop:
		lsl.l	#1,d1		; shift result left
		roxl.l	#1,d0		;

		;-- check if quotient bit is set
		cmp.l	d2,d4		; compare high longs
		bhi.s	dldQShiftNumer	; numerator < denominator
		bne.s	dldQSetQBit	; numerator >= denominator
		cmp.l	d3,d5		; compare low longs
		bhi.s	dldQShiftNumer	; numerator < denominator


dldQSetQBit:
		addq.l	#1,d1		; set quotient LSB
		sub.l	d5,d3		; subtract denominator from numerator
		subx.l	d4,d2		;

dldQShiftNumer:
		lsl.l	#1,d3		; shift numerator left
		roxl.l	#1,d2		;
		
dldQDBF:
		dbf	d7,dldQLoop	; for each bit in result

		;-- reinstate sign of result
		tst.w	d6
		beq.s	dldDone

		neg.l	d1		; negate result
		negx.l	d0

dldDone:
		movem.l	(a7)+,d2-d7
		rts

dldZero:
		moveq	#0,d0		; zero result
		moveq	#0,d1		;
		bra.s	dldDone

	END
