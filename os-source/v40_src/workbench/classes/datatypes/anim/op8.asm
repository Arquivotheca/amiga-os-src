*******************************************************************************
*
*	$Id: op8.asm,v 40.1 93/03/22 14:53:28 davidj Exp $
*
*******************************************************************************

	incdir	'include:'
	include	'intuition/screens.i'
	include 'graphics/rastport.i'
	include	'graphics/gfx.i'
;	include	"/playanim.i"

	opt p=68020

* void __asm PlayDLTA (REG (a2) struct BitMap *bm, REG (a1) UBYTE *buff, REG (a3) WORD *table, REG (d4) UWORD ImageWidth, REG (d5) UWORD ImageDepth);

	XDEF	_PlayDLTA

_PlayDLTA:
	movem.l	a2/a5-a6/d2-d7,-(sp)

	move.l	a1,a5

	asr.w	#2,d4			; /1 for the word compression
	moveq	#0,d3
	move.w	bm_BytesPerRow(a2),d2
	lea	bm_Planes(a2),a6
	bra.s	loop.
loop:
	move.l	(a6)+,a2		; next bitplane OUT
	move.l	a5,a0
	add.l	0(a5,d3),a0		; next plane IN
	addq.w	#4,d3
	bsr.s	_Decode8VKPlane
loop.:
	dbra.s	d5,loop

	movem.l	(sp)+,a2/a5-a6/d2-d7
	rts

* LONG ASM Decode8VKPlane (REG (a0) char *in, REG(a2) char *out, REG (d2) LONG BytesPerRow, REG (a3) WORD *ytable, REG (d4) xcount);

	XDEF	_Decode8VKPlane

_Decode8VKPlane:
	movem.l	a2/a3/d2-d5,-(sp)	; save registers
	moveq	#7,d6
	move.l	#$7fffffff,d7
*	move.w	d2,d4			; make a copy of linebytes to use as a counter
	bra	zdcp			; And go to the "columns" loop

dcp
	move.l	a2,a1			; get copy of dest pointer
	move.l	(a0)+,d0		; fetch number of ops in this column
	bra	zdcvclp			; and branch to the "op" loop.

dcvclp:
	move.l	(a0)+,d1		; fetch next op
	bmi	dcvskuniq		; if hi-bit set branch to "uniq" decoder
	bne	skip
*	beq dcvsame			; if it's zero branch to "same" decoder

dcvsame					; here we decode a "vertical same run"
	move.l	(a0)+,d1		; fetch the count
	move.l	(a0)+,d3		; fetch the value to repeat
	move.w	d1,d5			; and do what it takes to fall into a "tower"
	lsr.w	#3,d5			; d5 holds # of times to loop through tower
	and.w	d6,d1			; d1 is the remainder
	add.w	d1,d1
	add.w	d1,d1
	neg.w	d1
	jmp	32+same_tower(pc,d1.w)

same_tower
	move.l	d3,(a1)
	adda.w	d2,a1
	move.l	d3,(a1)
	adda.w	d2,a1
	move.l	d3,(a1)
	adda.w	d2,a1
	move.l	d3,(a1)
	adda.w	d2,a1
	move.l	d3,(a1)
	adda.w	d2,a1
	move.l	d3,(a1)
	adda.w	d2,a1
	move.l	d3,(a1)
	adda.w	d2,a1
	move.l	d3,(a1)
	adda.w	d2,a1
	dbra	d5,same_tower
	dbra	d0,dcvclp
	addq.l	#4,a2
	dbra	d4,dcp
	movem.l	(sp)+,a2/a3/d2-d5
	rts

skip					; otherwise it's just a skip
*	add.w	d1,d1			; use amount to skip as index into word-table
	adda.w	(0,a3,d1.w*2),a1
	dbra	d0,dcvclp		; go back to top of op loop
	addq.l	#4,a2
	dbra	d4,dcp
	movem.l	(sp)+,a2/a3/d2-d5
	rts


dcvskuniq				; here we decode a "unique" run
	and.l	d7,d1		; setting up a tower as above....
	move.w	d1,d5
	lsr.w	#3,d5
	and.w	d6,d1
	add.w	d1,d1
	add.w	d1,d1
	neg.w	d1
	jmp	32+uniq_tower(pc,d1.w)
uniq_tower
	move.l	(a0)+,(a1)
	adda.w	d2,a1
	move.l	(a0)+,(a1)
	adda.w	d2,a1
	move.l	(a0)+,(a1)
	adda.w	d2,a1
	move.l	(a0)+,(a1)
	adda.w	d2,a1
	move.l	(a0)+,(a1)
	adda.w	d2,a1
	move.l	(a0)+,(a1)
	adda.w	d2,a1
	move.l	(a0)+,(a1)
	adda.w	d2,a1
	move.l	(a0)+,(a1)
	adda.w	d2,a1
	dbra	d5,uniq_tower		; branch back up to "op" loop
zdcvclp dbra	d0,dcvclp		; branch back up to "column loop"

					; now we've finished decoding a single column
z1dcp	addq.l	#4,a2			; so move the dest pointer to next column
zdcp	dbra	d4,dcp			; and go do it again what say?
	movem.l	(sp)+,a2/a3/d2-d5	; restore registers
	rts
