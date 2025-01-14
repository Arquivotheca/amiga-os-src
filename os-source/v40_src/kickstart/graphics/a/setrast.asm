*******************************************************************************
*
*	$Id: setrast.asm,v 39.6 92/02/26 10:39:19 chrisg Exp $
*
*******************************************************************************

	include 'exec/types.i'

	include 'graphics/gfx.i'
	include 'graphics/rastport.i'
	include 'graphics/gfxbase.i'
	include 'graphics/clip.i'
	include 'graphics/gfxbase.i'

	include 'hardware/blit.i'
	include 'hardware/custom.i'
	include 'submacs.i'
	include '/sane_names.i'
	include	'/macros.i'

	section	graphics
    xdef    _SetRast
	xref	_LVOBltClear
	xref	_LVOOwnBlitter
	xref	_LVODisownBlitter
	xref	waitblitdone
	xref	_custom
	xref	bltbytes

******* graphics.library/SetRast ***********************************************
*
*   NAME
*       SetRast - Set an entire drawing area to a specified color.
*
*   SYNOPSIS
*       SetRast( rp, pen )
*                a1  d0
*
*	void SetRast( struct RastPort *, UBYTE );
*
*   FUNCTION
*       Set the entire contents of the specified RastPort to the
*       specified pen.
*
*   INPUTS
*       rp - pointer to RastPort structure
*       pen - the pen number (0-255) to jam into bitmap
*
*   RESULT
*       All pixels within the drawing area are set to the
*	selected pen number.
*
*   BUGS
*
*   SEE ALSO
*	RectFill() graphics/rastport.h
*
******************************************************************************
_SetRast:
	
	move.l	rp_Layer(a1),d1
	if <>					* is there a layer?
		movem.l	a2/a3/a5,-(sp)
		move.l	d1,a5
		LOCKLAYER

		move.l	a1,a2		* save rastport in a2
		move.l	lr_ClipRect(a5),d1
		while <>
			move.l	d1,a3		* check a3 cliprect
			if cr_lobs(a3).l
				move.l	cr_BitMap(a3),d1	* are there any bits?
				if <>
					move.l	d1,a1			* set up bitmap
					move.b	rp_Mask(a2),d1	* writemask
*											* d0 = color
					bsr.s	setrast_bm
				endif
			else
				bsr bigsetrast1
			endif
			move.l	(a3),d1			* next ClipRect
		whend

*		check for super bitmap
		move.l	lr_SuperBitMap(a5),d1
		if <>
			move.l	d1,a0			* save here for now
			move.b	rp_Mask(a2),d1	* writemask
			move.l	a0,a1			* bitmap
*						* d0 = color
			bsr.s	setrast_bm
		endif
		UNLOCKLAYER
		movem.l	(sp)+,a2/a3/a5
		rts
	endif
*	drop into setrast_bm
	move.b	rp_Mask(a1),d1	* get writemask
	move.l	rp_BitMap(a1),a1	* get bitmap pointer

setrast_bm:
* inputs = (color = d0, writemask = d1, bm = a1)
* other variables   d2 = counter d3 = Depth
* new variables		Planes = a1

	movem.l	d2/d3/d4/d5/d6/a0/a3,-(sp)
*	calculate constant bltsize for each blit
	ifne bm_BytesPerRow
		fail
	endc
*	move.w	bm_BytesPerRow(a1),d4
	move.w	bm_Rows(a1),d4
	swap	d4
	move.w	(a1),d4

	cmp.w	#UNLIKELY_WORD,bm_Pad(a1)
	bne.s	non_interleaved
	move.l	bm_Planes+4(a1),d2
	sub.l	bm_Planes(a1),d2
	move.w	bm_BytesPerRow(a1),d5
	ext.l	d5
	cmp.l	d5,d2
	bls.s	is_interleaved
non_interleaved:

	moveq	#0,d2
	move.b	bm_Depth(a1),d3
	lea	bm_Planes(a1),a3
	move.l	d1,d5	; writemask
	move.l	d0,d6	; pencolor

	while d2<d3.b
		btst d2,d5
		if <>
			btst	d2,d6
			if <>
				move.l	#$FFFF0006,d1
			else
				move.l	#$00000002,d1
			endif
			move.l	(a3),a1
			move.l	d4,d0
			jsr	_LVOBltClear(a6)
		endif
		addq.l	#4,a3
		addq.b	#1,d2
	whend
	move.l	d5,d1
	move.l	d6,d0

	movem.l	(sp)+,d2/d3/d4/d5/d6/a0/a3
    rts

is_interleaved:
; must do an interleaved bitmap clear. d0=color d1=mask bm=a1
	movem.l	a2/d0/d1,-(a7)
	lea	_custom,a0
	move.w	bm_Rows(a1),d2
	move.l	bm_Planes+4(a1),d3
	sub.l	bm_Planes(a1),d3	; d3=# of bytes to blit
	move.w	bm_BytesPerRow(a1),d4
	sub	d3,d4			; d4=blitter modulo
	lsr.l	#1,d3			; d3=# of words to blit
	btst	#GFXB_BIG_BLITS,gb_ChipRevBits0(a6)
	bne.s	have_big_blits
	lsl	#6,d2
	and	#63,d3
	add.w	d3,d2
have_big_blits:
	lea	bm_Planes(a1),a2
	moveq	#0,d5
	move.b	bm_Depth(a1),d5
	move.w	#$100,d6		; USED
	OWNBLITTER
	bra.s	end_loop
blit_loop:
	move.l	(a2)+,a3		; planeptr
	lsr.w	#1,d0
	scs	d6
	lsr.w	#1,d1			; masked?
	bcc.s	end_loop
	WAITBLITDONE	a0
	clr.w	bltcon1(a0)
	move.w	d4,bltdmod(a0)
	move.w	d6,bltcon0(a0)
	move.l	a3,bltdpt(a0)
	btst	#GFXB_BIG_BLITS,gb_ChipRevBits0(a6)
	bne.s	have_big_blits1
	move.w	d2,bltsize(a0)		; go!!
	bra.s	end_loop
have_big_blits1:
	move.w	d2,bltsizv(a0)
	move.w	d3,bltsizh(a0)		; go!!
end_loop:
	dbra	d5,blit_loop
	DISOWNBLITTER
	movem.l	(a7)+,a2/d0/d1
	movem.l	(sp)+,d2/d3/d4/d5/d6/a0/a3
	rts
			

bigsetrast1:
;
; internal graphics routine, called by setrast.asm for cliprects that are onscreen.
;
; bigsetrast1(a3=cr,d0=color,a2=w)
;
; ok to trash a2/d1/a0/a1
;

	movem.l	d0/d2/d3/d4/d5/d7/a3-a5,-(a7)
	move	d0,d7				; d7=color
	move.l	rp_BitMap(a2),a5		; a5=w->BitMap
	move	bm_BytesPerRow(a5),d5
	move	cr_MinX(a3),d0
	move	cr_MaxX(a3),d1
	bsr	bltbytes
	move	d0,d2				; d2=bltbytes


	move	cr_MaxY(a3),d3
	move	cr_MinY(a3),d1
	sub	d1,d3
	addq	#1,d3				; d3=maxy-miny+1=bltvsize

	mulu	d5,d1
	move.l	d1,a4
	move	cr_MinX(a3),d1
	lsr	#3,d1
	add	d1,a4				; a4=dstoffset=(MinY*BytesPerRow)+minx>>3

	OWNBLITTER

	lea	_custom,a0

	sub	d2,d5				; d5=BytesPerRow-bltbytes=modulus

	lsr	#1,d2				; d2=bltbytes>>1=blthsize

	WAITBLITDONE	a0
	move	d5,bltdmod(a0)
	move	d5,bltcmod(a0)
	clr.w	bltcon1(a0)
	moveq	#-1,d0
	move	d0,bltadat(a0)
	move	cr_MinX(a3),d1
	and	#$0f,d1
	lsr	d1,d0
	move	d0,bltafwm(a0)
	moveq	#15,d1
	move	cr_MaxX(a3),d0
	and	#15,d0
	sub	d0,d1
	moveq	#-1,d0
	lsl	d1,d0
	move	d0,bltalwm(a0)

	moveq	#1,d1				; d1=mask
	moveq	#0,d0
	move.b	bm_Depth(a5),d0			; could this be zero?
	lea	bm_Planes(a5),a5		; plane ptr

	btst	#GFXB_BIG_BLITS,gb_ChipRevBits0(a6)
	bne.s	is_new_blitter
	lsl	#6,d3
	and	#63,d2
	or	d3,d2				; d2=bltsize for old blitter
	bra.s	end_of_loop
is_new_blitter:
	and	#$7ff,d2
	and	#$7fff,d3
	move	d3,bltsizv(a0)
	bra.s	end_of_loop

plane_loop:
	move.l	(a5)+,a1			; a1=plane ptr
	move.b	rp_Mask(a2),d3
	and.b	d1,d3				; write this plane?
	beq.s	no_write_this_plane
	add.l	a4,a1
	move.w	#DEST+SRCC+NANBC+NABC,d4
	move.b	d1,d3
	and.b	d7,d3
	beq.s	no_c1
	or.w	#ABC+ABNC+ANBC+ANBNC,d4
no_c1:	WAITBLITDONE	a0
	move	d4,bltcon0(a0)
	move.l	a1,bltdpt(a0)
	move.l	a1,bltcpt(a0)
	btst	#GFXB_BIG_BLITS,gb_ChipRevBits0(a6)
	bne.s	new_blitter
	move	d2,bltsize(a0)			; go!
	bra.s	no_write_this_plane
new_blitter:
	move	d2,bltsizh(a0)			; go!
no_write_this_plane
	add	d1,d1
end_of_loop:
	dbra	d0,plane_loop

	DISOWNBLITTER
	movem.l	(a7)+,d0/d2/d3/d4/d5/d7/a3-a5
	rts

	end
