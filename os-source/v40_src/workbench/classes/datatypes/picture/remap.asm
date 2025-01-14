; $$TABS=8
; 1992 Chris Green
; bm_xlate.asm - translate a bitmap in via a one byte table

	include	"strucdef.equ"
	include	"lvo.equ"
	addsym

do_plane_rd	macro
	move.l	-(a0),a1
	move.b	0(a1,a4.l),d7		; d7=bitmap byte
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

do_plane_wr	macro
	move.l	(a0)+,a1
	lsr.w	#1,d0
	addx.w	d7,d7
	lsr.w	#1,d1
	addx.w	d7,d7
	lsr.w	#1,d2
	addx.w	d7,d7
	lsr.w	#1,d3
	addx.w	d7,d7
	lsr.w	#1,d4
	addx.w	d7,d7
	lsr.w	#1,d5
	addx.w	d7,d7
	lsr.w	#1,d6
	addx.w	d7,d7
	swap	d6
	lsr.w	#1,d6
	addx.w	d7,d7
	swap	d6
	move.b	d7,0(a1,a4.l)
	endm

; XlateBitMap(a0=srcbm,a1=destbm,a2=table1, a3=table2,d0=width)
ofs_read_planes	equ	0
ofs_write_planes	equ	ofs_read_planes+4
ofs_bperrow	equ	ofs_write_planes+4
ofs_col_ctr	equ	ofs_bperrow+4	; lword align!
ofs_row_ctr	equ	ofs_col_ctr+2
ofs_adr_add	equ	ofs_row_ctr+2
ofs_src_offset	equ	ofs_adr_add+4
ofs_dst_offset	equ	ofs_src_offset+4
ofs_dadr_add	equ	ofs_dst_offset+4
st_depth	equ	ofs_dadr_add+4

_XLateBitMap::
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
	move.w	bm_BytesPerRow(a1),d0
	sub.w	ofs_col_ctr(a7),d0
	ext.l	d0
	move.l	d0,ofs_dadr_add(a7)
	moveq	#0,d0
	move.l	d0,ofs_dst_offset(a7)
	move.l	d0,ofs_src_offset(a7)
	move.b	bm_Depth(a0),d0
	add.w	d0,d0
	add.w	d0,d0
	lea	bm_Planes(a0,d0.w),a0
	move.l	a0,ofs_read_planes(a7)
	lea	depth_rd_jmp_tbl(pc),a6
	move.l	0(a6,d0.w),a6
	move.b	bm_Depth(a1),d0
	add.w	d0,d0
	add.w	d0,d0
	move.l	depth_wr_jmp_tbl(pc,d0.w),a5
	lea	bm_Planes(a1),a1
	move.l	a1,ofs_write_planes(a7)

	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3
	moveq	#0,d4
	moveq	#0,d5
	moveq	#0,d6
	moveq	#0,d7
byte_loop:
	moveq	#0,d0
	move.l	ofs_read_planes(a7),a0
	move.l	ofs_src_offset(a7),a4
	jmp	(a6)

depth_wr_jmp_tbl:
	dc.l	0		; zero deep?
	dc.l	one_deep1
	dc.l	two_deep1
	dc.l	three_deep1
	dc.l	four_deep1
	dc.l	five_deep1
	dc.l	six_deep1
	dc.l	seven_deep1
	dc.l	eight_deep1

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
	move.b	0(a2,d0.w),d0
	move.b	0(a3,d1.w),d1
	move.b	0(a2,d2.w),d2
	move.b	0(a3,d3.w),d3
	move.b	0(a2,d4.w),d4
	move.b	0(a3,d5.w),d5
	move.b	0(a2,d6.w),d6
	swap	d6
	move.b	0(a3,d6.w),d6
	swap	d6
	move.l	ofs_dst_offset(a7),a4
	move.l	ofs_write_planes(a7),a0
	jmp	(a5)

eight_deep1:
	do_plane_wr
seven_deep1:
	do_plane_wr
six_deep1:
	do_plane_wr
five_deep1:
	do_plane_wr
four_deep1:
	do_plane_wr
three_deep1:
	do_plane_wr
two_deep1:
	do_plane_wr
one_deep1:
	do_plane_wr
	addq.l	#1,ofs_dst_offset(a7)
	addq.l	#1,ofs_src_offset(a7)
	subq.w	#1,ofs_col_ctr(a7)
	bne	byte_loop
	move.w	ofs_bperrow(a7),ofs_col_ctr(a7)
	exg.l	a2,a3
	move.l	ofs_adr_add(a7),d0
	add.l	d0,ofs_src_offset(a7)
	move.l	ofs_dadr_add(a7),d0
	add.l	d0,ofs_dst_offset(a7)
	subq.w	#1,ofs_row_ctr(a7)
	bne	byte_loop

	lea	st_depth(a7),a7
	movem.l	(a7)+,d2-d7/a2-a6
	rts



