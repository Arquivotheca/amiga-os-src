**
**	$Id: margin.asm,v 9.0 91/04/09 20:03:13 kodiak Exp $
**
**      
**
**      (C) Copyright 1991 Robert R. Burns
**          All Rights Reserved
**
	SECTION	bullet

**	Exported Names

	XDEF	_margin

**	Structures

; bm
width		EQU	16
depth		EQU	18
left_indent	EQU	20
top_indent	EQU	22
black_width	EQU	24
black_depth	EQU	26
bm		EQU	52


; - - -	margin  - - - - - - - - - - - - - - - - - - - - - - - - - - -
;	margin(IFBITMAP *bm)
_margin:
;   d0	bottom/right counter
;   d1	row DBF counter
;   d2	column up counter
;   d3	accumulator
;   d4	top/left counter/result
;   d5	column modulo
;   d6  column DBF limit
;   d7  row DBF limit
;   a0	working pointer
;   a1	bm->bm
;   a2	bm

		movem.l	d2-d7/a2,-(a7)
		move.l	32(a7),a2	; bm
		move.w	width(a2),d5	; bm->width
		move.w	d5,d6		;   width
		lsr.w	#2,d6		;   in longwords
		subq.w	#1,d6		;   DBF bound
		blt.s	emptyBitmap
		move.w	depth(a2),d7	; bm->depth
		subq.w	#1,d7
		blt.s	emptyBitmap
		lea	bm(a2),a1	; bm->bm

		;-- compute top indent
		move.l	a1,a0		; bm->bm
		move.w	d7,d4		; y line DBF counter
topOuterLoop:
		move.w	d6,d0		; x longword DBF counter
topInnerLoop:
		tst.l	(a0)+		; test next bm longword
		dbne	d0,topInnerLoop
		dbne	d4,topOuterLoop
		bne.s	topCalcs

emptyBitmap:
		clr.w	top_indent(a2)
		clr.w	black_depth(a2)
		clr.w	left_indent(a2)
		clr.w	black_width(a2)
		bra	done

topCalcs:
		sub.w	d7,d4		; convert remaining y line DBF counter
		neg.w	d4		;   into top indent
		move.w	d4,top_indent(a2)

		;-- compute black depth
		;--	compute bottom indent
		move.w	d7,d0		; find end of bm->bm
		mulu	d5,d0		;
		lea	0(a1,d0.l),a0	;
		move.l	d7,d1		; y line DBF counter
bottomOuterLoop:
		move.w	d6,d0		; x longword DBF counter
bottomInnerLoop:
		tst.l	-(a0)		; test previous bm longword
		dbne	d0,bottomInnerLoop
		dbne	d1,bottomOuterLoop

		sub.w	d4,d1		; subtract top indent from remaining y
		move.w	d1,black_depth(a2)


		;-- compute left indent
		;--	count till first longword set
		moveq	#-4,d4		; start longword byte offset

leftOuterLoop:
		addq.w	#4,d4		; increment to next column
		lea	0(a1,d4.w),a0	; column longword start byte
		moveq	#0,d3		; accumulator
		move.w	d7,d1		; y line DBF counter
leftInnerLoop:
		or.l	(a0),d3		; accumulate longword from column
		add.w	d5,a0		; increment to next longword in column
		dbf	d1,leftInnerLoop

		tst.l	d3		; check for longword set in column
		beq.s	leftOuterLoop

		;--	count till first bit set
		moveq	#-1,d1		; initialize <minus-bit-number-plus-1>
leftBitLoop:
		add.l	d3,d3
		dbcs	d1,leftBitLoop
		neg.w	d1		; convert <minus-bit-number-plus-1>
		subq.w	#1,d1		;   to bit-number

		lsl.w	#3,d4		; convert start longword to bit origin
		add.w	d1,d4		; start bit
		move.w	d4,left_indent(a2)


		;-- compute right indent
		;--	count till last longword set
		move.w	d5,d0		; end longword byte offset
		subq.w	#4,d0		; check if width > one longword
		bne.s	rightOuterStart

		;--	d3 already contains most of the necessary bits
		addq.w	#1,d1
		lsr.l	d1,d3		; shift them back into position
		bne.s	rightBitCalc	; rightmost bit still exists
		;--	start bit == end bit
		moveq	#1,d0
		bra.s	setBlackWidth

rightOuterLoop:
		subq.w	#4,d0		; decrement to previous column
rightOuterStart:
		lea	0(a1,d0.w),a0	; column longword start byte
		moveq	#0,d3		; accumulator
		move.w	d7,d1		; y line DBF counter
rightInnerLoop:
		or.l	(a0),d3		; accumulate longword from column
		add.w	d5,a0		; increment to next longword in column
		dbf	d1,rightInnerLoop

		tst.l	d3		; check for longword set in column
		beq.s	rightOuterLoop

		;--	count till last bit set
rightBitCalc:
		moveq	#32,d1		; initialize <bit-number+1>
rightBitLoop:
		lsr.l	#1,d3
		dbcs	d1,rightBitLoop

		lsl.w	#3,d0		; convert end longword to bit origin
		add.w	d1,d0		; end bit+1
		sub.w	d4,d0		; subtract left indent
setBlackWidth:
		move.w	d0,black_width(a2)
done:
		movem.l	(a7)+,d2-d7/a2
		rts

	END
