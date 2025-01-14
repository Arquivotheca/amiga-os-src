; $$TABS=8
; � 1992 Chris Green
; bm_tally.asm - compute a ULONG histogram of a bitmap

	include	"strucdef.equ"
	include	"lvo.equ"
	addsym

do_plane_rd	macro
	move.l	-(a0),a2
	move.b	0(a2,a4.l),d7		; d7=bitmap byte
	add.b	d7,d7
	addx.w	d0,d0
	add.b	d7,d7
	addx.w	d1,d1
	add.b	d7,d7
	addx.w	d2,d2
	add.b	d7,d7
	addx.w	d3,d3
	add.b	d7,d7
	addx.w	d4,d4
	add.b	d7,d7
	addx.w	d5,d5
	add.b	d7,d7
	addx.w	d6,d6
	swap	d6
	add.b	d7,d7
	addx.w	d6,d6
	swap	d6
	endm

depth_rd_jmp_tbl:
	dc.l	0		; zero deep?
	dc.l	one_deep
	dc.l	two_deep
	dc.l	three_deep
	dc.l	four_deep
	dc.l	five_deep
	dc.l	six_deep
	dc.l	seven_deep
	dc.l	eight_deep

; VOID ASM TallyBitMap(REG (a0) struct BitMap *srcbm, REG (a1) ULONG *histogram, REG (d0) ULONG width);
ofs_bperrow	equ	0
ofs_col_ctr	equ	ofs_bperrow+4	; lword align!
ofs_row_ctr	equ	ofs_col_ctr+2
ofs_adr_add	equ	ofs_row_ctr+2
st_depth	equ	ofs_adr_add+4

_TallyBitMap::
	movem.l	d2-d7/a2-a6,-(a7)
	sub.l	#st_depth,a7
	add.w	#7,d0
	lsr.w	#3,d0
	move.w	d0,ofs_col_ctr(a7)
	move.w	bm_Rows(a0),ofs_row_ctr(a7)
	move.w	d0,ofs_bperrow(a7)
	sub.w	bm_BytesPerRow(a0),d0
	ext.l	d0
	neg.l	d0
	move.l	d0,ofs_adr_add(a7)
	sub.l	a4,a4
	moveq	#0,d0
	move.b	bm_Depth(a0),d0
	add.w	d0,d0
	add.w	d0,d0
	lea	bm_Planes(a0,d0.w),a0
	move.l	a0,a5
	lea	depth_rd_jmp_tbl(pc),a6
	move.l	0(a6,d0.w),a6

byte_loop:
	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3
	moveq	#0,d4
	moveq	#0,d5
	moveq	#0,d6
	moveq	#0,d7
	move.l	a5,a0
	jmp	(a6)

eight_deep:
	do_plane_rd
seven_deep:
	do_plane_rd
six_deep:
	do_plane_rd
five_deep:
	do_plane_rd
four_deep:
	do_plane_rd
three_deep:
	do_plane_rd
two_deep:
	do_plane_rd
one_deep:
	do_plane_rd

;	tst.w	$0			; dump registers (through enforcer)
	add.w	d0,d0
	add.w	d0,d0
	addq.l	#1,0(a1,d0.w)
	add.w	d1,d1
	add.w	d1,d1
	addq.l	#1,0(a1,d1.w)
	add.w	d2,d2
	add.w	d2,d2
	addq.l	#1,0(a1,d2.w)
	add.w	d3,d3
	add.w	d3,d3
	addq.l	#1,0(a1,d3.w)
	add.w	d4,d4
	add.w	d4,d4
	addq.l	#1,0(a1,d4.w)
	add.w	d5,d5
	add.w	d5,d5
	addq.l	#1,0(a1,d5.w)
	add.l	d6,d6
	add.l	d6,d6
	addq.l	#1,0(a1,d6.w)
	swap	d6
	addq.l	#1,0(a1,d6.w)
	addq.l	#1,a4
	subq.w	#1,ofs_col_ctr(a7)
	bne	byte_loop
	move.w	ofs_bperrow(a7),ofs_col_ctr(a7)
	add.l	ofs_adr_add(a7),a4
	subq.w	#1,ofs_row_ctr(a7)
	bne	byte_loop

	lea	st_depth(a7),a7
	movem.l	(a7)+,d2-d7/a2-a6
	rts



