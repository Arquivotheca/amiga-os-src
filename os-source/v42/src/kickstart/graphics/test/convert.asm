	include	'exec/types.i'
	include	'/macros.i'
	include	'/bitmap_internal.i'
	include	'/gfx.i'

	opt	p=68020

; void __asm Chunkify(register __a0 struct BitMap *src, register __a1 UBYTE *dest,
; /* __asm */		register __d0 ULONG startx, register __d1 UWORD starty,
; /* __asm */		register __d2 UWORD width, register __d3 UWORD height,
; /* __asm */		register __d4 UBYTE mask, register __a2 UWORD destmodulo);
; this routine copies planar data from a bitmap structure to chunky
; data.

_Chunkify::

TEMP_SIZE	set	0
	LONGVAR	srcx

	movem.l	d2-d7/a2-a6,-(a7)
	ALLOCLOCALS

	move.l	d0,srcx_l(a7)

	mulu	bm_BytesPerRow(a0),d1	; cvt to pointer offset
	move.w	bm_BytesPerRow(a0),a6
	move.l	d1,a3
; calc d7=right field width=(width & 3)*8
	move	d2,d7
	and.l	#3,d7
	lsl.l	#3,d7
	lsr.w	#2,d2			; cvt to # of full lwords
	sub	d2,a2
	sub	d2,a2
	sub	d2,a2
	sub	d2,a2
	lea	BitTable(pc),a4
	moveq	#0,d5
	move.b	bm_Depth(a0),d5
	dc.l	$2a705db0
	dc.l	depth_jmp_table
;	move.l	(depth_jmp_table.l,d5.l*4),a5
	lea	bm_Planes(a0),a0
	jsr	(a5)
	lea	TEMP_SIZE(a7),a7
	movem.l	(a7)+,d2-d7/a2-a6
	rts

depth_jmp_table:
	dc.l	do_depth_0
	dc.l	do_depth_1
	dc.l	do_depth_2
	dc.l	do_depth_3
	dc.l	do_depth_4
	dc.l	do_depth_5
	dc.l	do_depth_6
	dc.l	do_depth_7

BitTable:
; converts 4 bits of planar data into 4 chunky pixels
	dc.l	$00000000,$00000001,$00000100,$00000101
	dc.l	$00010000,$00010001,$00010100,$00010101
	dc.l	$01000000,$01000001,$01000100,$01000101
	dc.l	$01010000,$01010001,$01010100,$01010101


doplane	macro	pl
;		\1
	move.l	(a0)+,d1		; next bitplane
	beq.s	doneplane\@
	bpl.s	doplane\@
	or.l	#$01010101<<\1,d5
	bra.s	doneplane\@
doplane\@
	bfextu	(a3,d1.l){d0:4},d1
	IFne	\1
	move.l	(a4,d1.l*4),d1
	lsl.l	#\1,d1
	or.l	d1,d5
	ELSE
	move.l	(a4,d1.l*4),d5
	endc
doneplane\@:
	endm

gencvt	macro	depth
yloop\@:
	move.l	srcx_l+4(a7),d0		; restart x counter
	move.w	d2,d6			; clone width
	ifge	\1-5
	bra	end_xloop\@
	ELSE
	bra.s	end_xloop\@
	endc
xloop\@:
	moveq	#0,d5			; accumulator
	IFge	\1-1
	doplane	0
	endc
	IFge	\1-2
	doplane	1
	endc
	IFge	\1-3
	doplane	2
	endc
	IFge	\1-4
	doplane	3
	endc
	IFge	\1-5
	doplane	4
	endc
	IFge	\1-6
	doplane	5
	endc
	IFge	\1-7
	doplane	6
	endc
	IFge	\1-8
	doplane	7
	endc
	lea	-\1*4(a0),a0
	move.l	d5,(a1)+
	addq.l	#4,d0			; skip bits
end_xloop\@:
	dbra	d6,xloop\@
; now, do right edge
	moveq	#0,d5			; accumulator
	IFge	\1-1
	doplane	0
	endc
	IFge	\1-2
	doplane	1
	endc
	IFge	\1-3
	doplane	2
	endc
	IFge	\1-4
	doplane	3
	endc
	IFge	\1-5
	doplane	4
	endc
	IFge	\1-6
	doplane	5
	endc
	IFge	\1-7
	doplane	6
	endc
	IFge	\1-8
	doplane	7
	endc
	bfins	d5,(a1){0:d7}
	lea	-\1*4(a0),a0
	add.l	a2,a1			; add dest modulo
	add.l	a6,a3			; add src modulo
do_depth_\1:
	dbra	d3,yloop\@
	rts
	endm



	gencvt	0
	gencvt	1
	gencvt	2
	gencvt	3
	gencvt	4
	gencvt	5
	gencvt	6
	gencvt	7

	END
