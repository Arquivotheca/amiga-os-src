*******************************************************************************
*
* (c) Copyright 1993 Commodore-Amiga, Inc.  All rights reserved.
*
* This software is provided as-is and is subject to change; no warranties
* are made.  All use is at your own risk.  No liability or responsibility
* is assumed.
*
* ilbm.asm - code to decode ILBMs and LONG DLTAs.
* This code has some 68020 optimisations, and the DLTA code has some
* optimisations best suited for my animation (namely, changing the order in
* which the DLTA opcodes are checked).
*
*******************************************************************************

	incdir	'include:'
	include	'intuition/screens.i'
	include 'graphics/rastport.i'
	include	'graphics/gfx.i'
	include	"playanim.i"

	opt p=68020

* void __asm ShowILBM(register __a0 struct BitMapHeader *bmhd, register __a1 struct BitMap *bm, register __a2 UBYTE *buff);

	xdef	_ShowILBM
_ShowILBM:
	movem.l	a2-a6/d2-d7,-(sp)

	lea	bm_Planes(a1),a3
	moveq	#0,d2
	move.l	d2,d7
	move.l	d2,d3
	move.w	bm_BytesPerRow(a1),d2

	move.b	bmhd_nplanes(a0),d7
	subq.b	#1,d7
	move.w	bmhd_h(a0),d6
	move.w	bmhd_w(a0),d5
	add.w	#15,d5
	asr.w	#4,d5
	add.w	d5,d5			; bytes per row in the image

	move.w	d7,d4
	bra.s	do_rows.

do_rows:
	move.l	a3,a4			; restore plane pointers
next_plane:
	sub.l	a5,a5
	move.l	(a4)+,a0		; plane pointer
	adda.l	d3,a0			; to next row

next_col:
	move.b	(a2)+,d0		; next byte in the buffer
	bpl.s	do_copy

	neg.b	d0			; repeat the next byte d0 times
	ext.w	d0
	bmi.s	next_col		; was 128 == NOP
	add.w	d0,a5
	addq.w	#1,a5
	move.b	(a2)+,d1
1$:
	move.b	d1,(a0)+
	dbra.s	d0,1$
check_col:
	cmp.w	a5,d5
	bne.s	next_col

	dbra.s	d4,next_plane

	move.w	d7,d4			; restore plane counter
	add.l	d2,d3			; next row in the bitmap
do_rows.:
	dbra.s	d6,do_rows

ShowILBM.:
	movem.l	(sp)+,d2-d7/a2-a6
	rts

do_copy:
	ext.w	d0
	addq.b	#1,d0
	add.w	d0,a5

	move.w	d0,a6
	asr.w	#2,d0			; how many longwords?
	bra.s	copy_l.
copy_l:
	move.l	(a2)+,(a0)+
copy_l.:
	dbra.s	d0,copy_l

	move.w	a6,d0
	and.w	#$3,d0			; this many bytes left over
	bra.s	copy_b.
copy_b:
	move.b	(a2)+,(a0)+
copy_b.:
	dbra.s	d0,copy_b
	bra.s	check_col


* void __asm PlayDLTA(register __a2 struct BitMap *bm, register __a1 UBYTE *buff, register __a3 WORD *table, register __d4 UWORD ImageWidth, register __d5 UWORD ImageDepth);
	xdef _PlayDLTA
_PlayDLTA:
	movem.l	a2/a5-a6/d2-d7,-(sp)

	move.l	a1,a5

*	asr.w	#2,d4			; /4 for the LONG compression
	moveq	#0,d3
	move.w	bm_BytesPerRow(a2),d2
	lea	bm_Planes(a2),a6
	bra.s	loop.
loop:
	move.l	(a6)+,a2		; next bitplane OUT
	move.l	a5,a0
	move.l	0(a5,d3),d0
	beq.s	1$		; skip unused bitplanes
	add.l	d0,a0		; next plane IN
	bsr.s	_DecodeVKPlane
1$:
	addq.w	#4,d3
loop.:
	dbra.s	d5,loop

	movem.l	(sp)+,a2/a5-a6/d2-d7
	rts

; LONG ASM DecodeVKPlane (REG (a0) char *in, REG(a2) char *out, REG (d2) LONG BytesPerRow, REG (a3) WORD *ytable, REG (d4) xcount);

_DecodeVKPlane:
	movem.l	a2/a3/d2-d5,-(sp)	; save registers
	moveq	#7,d6
	moveq	#$7f,d7
	bra	zdcp			; And go to the "columns" loop

dcp
	move.l	a2,a1			; get copy of dest pointer
	moveq	#0,d0
	move.b	(a0)+,d0		; fetch number of ops in this column
	bra	zdcvclp			; and branch to the "op" loop.

dcvclp:
	moveq	#0,d1
	move.b	(a0)+,d1		; fetch next op
	bmi	dcvskuniq		; if hi-bit set branch to "uniq" decoder
	bne	skip			; if op is >0, then it's a "skip"

dcvsame					; here we decode a "vertical same run"
	moveq	#0,d1
	move.b	(a0)+,d1		; fetch the count
	move.b	(a0)+,d3		; fetch the value to repeat
	move.w	d1,d5			; and do what it takes to fall into a "tower"
	lsr.w	#3,d5			; d5 holds # of times to loop through tower
	and.w	d6,d1			; d1 is the remainder
	add.w	d1,d1
	add.w	d1,d1
	neg.w	d1
	jmp	32+same_tower(pc,d1.w)

same_tower
	move.b	d3,(a1)
	adda.w	d2,a1
	move.b	d3,(a1)
	adda.w	d2,a1
	move.b	d3,(a1)
	adda.w	d2,a1
	move.b	d3,(a1)
	adda.w	d2,a1
	move.b	d3,(a1)
	adda.w	d2,a1
	move.b	d3,(a1)
	adda.w	d2,a1
	move.b	d3,(a1)
	adda.w	d2,a1
	move.b	d3,(a1)
	adda.w	d2,a1
	dbra	d5,same_tower
	dbra	d0,dcvclp
	addq.l	#1,a2
	dbra	d4,dcp
	movem.l	(sp)+,a2/a3/d2-d5
	rts

skip					; otherwise it's just a skip
*					; use amount to skip as index into word-table
	adda.w	(0,a3,d1.w*2),a1
	dbra	d0,dcvclp		; go back to top of op loop
	addq.l	#1,a2
	dbra	d4,dcp
	movem.l	(sp)+,a2/a3/d2-d5
	rts


dcvskuniq				; here we decode a "unique" run
	and.l	d7,d1			; setting up a tower as above....
	move.w	d1,d5
	lsr.w	#3,d5
	and.w	d6,d1
	add.w	d1,d1
	add.w	d1,d1
	neg.w	d1
	jmp	32+uniq_tower(pc,d1.w)
uniq_tower
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	move.b	(a0)+,(a1)
	adda.w	d2,a1
	dbra	d5,uniq_tower		; branch back up to "op" loop
zdcvclp dbra	d0,dcvclp		; branch back up to "column loop"

					; now we've finished decoding a single column
z1dcp	addq.l	#1,a2			; so move the dest pointer to next column
zdcp	dbra	d4,dcp			; and go do it again what say?
	movem.l	(sp)+,a2/a3/d2-d5	; restore registers
	rts
