	OPTIMON

;---------------------------------------------------------------------------

	IFD	MC68000
	XDEF	SMult32S
	XDEF	UMult32S
	XDEF	SDivMod32S
	XDEF	UDivMod32S
	XDEF	SMult64S
	XDEF	UMult64S
	ENDC

	XDEF	SMult32H
	XDEF	UMult32H
	XDEF	SDivMod32H
	XDEF	UDivMod32H
	XDEF	SMult64H
	XDEF	UMult64H

;---------------------------------------------------------------------------

	OPT	p=68020
	CNOP	0,4	 ; long-align the math routines for faster speed

;---------------------------------------------------------------------------

SMult32H:
	muls.l	d1,d0
	rts


;---------------------------------------------------------------------------

UMult32H:
	mulu.l	d1,d0
	rts

;---------------------------------------------------------------------------

SDivMod32H:
	divsl.l	d1,d1:d0
	rts

;---------------------------------------------------------------------------

UDivMod32H:
	divul.l	d1,d1:d0
	rts

;---------------------------------------------------------------------------

SMult64H:
	muls.l	d1,d1:d0
	rts


;---------------------------------------------------------------------------

UMult64H:
	mulu.l	d1,d1:d0
	rts

;---------------------------------------------------------------------------

	IFD	MC68000
	OPT	p=68000

;---------------------------------------------------------------------------

SMult32S:
UMult32S:
	move.l	d2,-(sp)	; need these
	move.l	d3,-(sp)
	move.l	d0,d2		; d0=Al
	move.l	d1,d3		; d1=Bl
	swap	d2		; d2=Ah
	swap	d3		; d3=Bh
	mulu.w	d1,d2		; d2=Ah*Bl
	mulu.w	d0,d3		; d3=Al*Bh
	mulu.w	d1,d0		; d0=Al*Bl
	add.w	d3,d2		; ignore overflow
	swap.w	d2		; shift to where it belongs
	clr.w	d2
	add.l	d2,d0		; ignore overflow
	move.l	(sp)+,d3
	move.l	(sp)+,d2	;
	rts

;---------------------------------------------------------------------------

SDivMod32S:
	tst.l	d0
	bpl.s	NumPos
	neg.l	d0
	tst.l	d1
	bpl.s	DenomPos
	neg.l	d1
	bsr.s	UDivMod32S
	neg.l	d1
	rts
DenomPos
	bsr.s	UDivMod32S
	neg.l	d0
	neg.l	d1
	rts
NumPos
	tst.l	d1
	bpl.s	UDivMod32S
	neg.l	d1
	bsr.s	UDivMod32S
	neg.l	d0
	rts

;---------------------------------------------------------------------------

UDivMod32S:
	move.l	d3,-(sp)
	swap	d1
	tst.w	d1		; can we do this easily? (is number < 65536)
	bne.s	BigDenom	; no, we have to work for it
	swap	d1
	move.l	d1,d3
	swap.w	d0
	move.w	d0,d3
	beq.s	SmallNum
	divu.w	d1,d3
	move.w	d3,d0
SmallNum
	swap.w	d0
	move.w	d0,d3
	divu.w	d1,d3
	move.w	d3,d0
	swap.w	d3
	move.w	d3,d1
	move.l	(sp)+,d3
	rts

BigDenom:
	swap	d1
	move.w	d2,-(sp)
	moveq	#15,d3		; 16 times through the loop
	move.w	d3,d2
	move.l	d1,d3
	move.l	d0,d1
	clr.w	d1
	swap	d1
	swap	d0
	clr.w	d0
DMls	add.l	d0,d0
	addx.l	d1,d1
	cmp.l	d1,d3
	bhi.s	DMle
	sub.l	d3,d1
	addq.w	#1,d0
DMle	dbf	d2,DMls
	move.w	(sp)+,d2
	move.l	(sp)+,d3
	rts

;---------------------------------------------------------------------------
; Multiply two signed 32 bit integers and return the 64 bit result
;
; What it will do is take the values, make them "right", do the multiply
; routine below and then make the sign correct...
;
SMult64S:
	move.l	d2,-(sp)	; We need a scrap...
	moveq.l	#0,d2		; No sign...
	tst.l	d0		; Check arg1...
	bpl.s	1$		; If not negative, skip...
	neg.l	d0		; Make d0 positive...
	addq.l	#1,d2		; Set d0 as negative flag...
1$	tst.l	d1		; Check arg2...
	bpl.s	2$		; If not negative, skip...
	neg.l	d1		; Make it positive...
	subq.l	#1,d2		; Set d1 as negative flag...
2$	bsr.s	UMult64S	; Do the multiply...
	tst.l	d2		; Are we needed?
	beq.s	3$		; If not negative result, skip...

	moveq.l	#0,d2		; We need a register with 0...
	not.l	d0		; Invert...
	not.l	d1		; binary (1's complement negative)
	addq.l	#1,d0		; add 1 (2's complement negative)
	addx.l	d2,d1		; (add in the carry)

3$	move.l	(sp)+,d2	; Clean up...
	rts

;---------------------------------------------------------------------------
; Multiply two unsigned 32 bit integers and return the 64 bit result
;
;   REGISTER USAGE
;	D4 -- scratch (restored)
;	D3 -- scratch (restored)
;	D2 -- scratch (restored)
;	D1 -- arg 1 (given), result 32:63
;	D0 -- arg 0 (given), result 0:31
;
UMult64S:
	movem.l	d2-d4,-(sp)

	move.l	d1,d3
	mulu.w	d0,d3		; 24
	move.l	d1,d2
	swap.w	d2
	swap.w	d0
	mulu.w	d0,d2		; 13

	swap.w	d3

	move.l	d1,d4
	mulu.w	d0,d4		; 14
	add.w	d4,d3
	clr.w	d4
	swap.w	d4
	addx.l	d4,d2

	swap.w	d0
	swap.w	d1

	move.l	d1,d4
	mulu.w	d0,d4		; 23
	add.w	d4,d3
	clr.w	d4
	swap.w	d4
	addx.l	d4,d2

	swap.w	d3

	move.l	d2,d0
	move.l	d3,d1

	movem.l	(sp)+,d2-d4
	rts

;---------------------------------------------------------------------------

	ENDC	; MC68020

;---------------------------------------------------------------------------

	END
