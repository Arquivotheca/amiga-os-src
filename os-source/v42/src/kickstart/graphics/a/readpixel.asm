*******************************************************************************
*
*	$Id: ReadPixel.asm,v 42.1 93/07/20 13:34:06 chrisg Exp $
*
*******************************************************************************

  include 'exec/types.i'     * Required data type definitions
	include	'/macros.i'
  include '/gfx.i'    * BitMap structure definition
  include 'exec/nodes.i'
  include 'exec/lists.i'
  include 'exec/interrupts.i'
  include 'exec/ports.i'
  include 'exec/libraries.i'
  include '/gfxbase.i'
  include 'graphics/clip.i'   * Layer structure definition
  include '/rastport.i'   * RastPort structure definition
  include 'submacs.i'     * Macros for subroutines called
  include '/sane_names.i'
  include	'/bitmap_internal.i'



	section	graphics
  xdef _ReadPixel  * Define public entry points

  PAGE

******* graphics.library/ReadPixel *********************************************
*
*   NAME
*       ReadPixel -- read the pen number value of the pixel at a
*                    specified x,y location within a certain RastPort.
*
*   SYNOPSIS
*       penno = ReadPixel( rp,    x,    y )
*         d0               a1  d0:16 d1:16
*
*	LONG ReadPixel( struct RastPort *, SHORT, SHORT );
*
*   FUNCTION
*       Combine the bits from each of the bit-planes used to describe
*       a particular RastPort into the pen number selector which that
*       bit combination normally forms for the system hardware selection
*       of pixel color.
*
*   INPUTS
*       rp -  pointer to a RastPort structure
*       (x,y) a point in the RastPort
*
*   RESULT
*       penno - the pen number of the pixel at (x,y) is returned.
*		-1 is returned if the pixel cannot be read for some reason.
*
*   BUGS
*
*   SEE ALSO
*       WritePixel()	graphics/rastport.h
*
******************************************************************************
	xref	waitblitdone
_ReadPixel:
	movem.l	d2/d3/a2/a5,-(sp)
	move.l	rp_Layer(a1),a5
	move.l	a5,d2				* is there a layer?
	if <>	.extend
		LOCKLAYER
*		a1 points to rastport
*		d0/d1 is the coordinate
*		d2 points to the layer
*		a5 points to layer
		move.l	lr_ClipRect(a5),d2	* get first ClipRect *
		if <>	.extend
			add.w	lr_MinX(a5),d0
			add.w	lr_MinY(a5),d1
			sub.w	lr_Scroll_X(a5),d0
			sub.w	lr_Scroll_Y(a5),d1
			repeat
				move.l	d2,a0		* get cr pointer in a0
*				is the point in this ClipRect?
				if cr_MinX(a0)<=d0.w
					if cr_MaxX(a0)>=d0.w
						if cr_MinY(a0)<=d1.w
							if cr_MaxY(a0)>=d1.w
*								found a cliprect
								tst.l	cr_lobs(a0)
								beq.s	rfinish		* simple, on screen
*									not on screen
*									special for obscuration
									move.w	cr_MinX(a0),d2
									and.w	#$FFF0,d2
									sub.w	d2,d0		* offset x
									sub.w	cr_MinY(a0),d1	* offset y
									move.l	cr_BitMap(a0),d2
									if =
*										no bitmap here
										moveq	#-1,d3
										bra rend
									endif
									move.l	d2,a0
									bra.s	rfinish2
*
							endif
						endif
					endif
				endif
	ifne	cr_Next
		fail
	endc
				move.l	(a0),d2		* try next cliprect
			until =
*			not in these cliprects try superbitmap
*			restore to original coordinates
			move.l	lr_SuperBitMap(a5),d2
			if <>
				add.w	lr_Scroll_X(a5),d0
				add.w	lr_Scroll_Y(a5),d1
				sub.w	lr_MinX(a5),d0
				sub.w	lr_MinY(a5),d1
				move.l	lr_SuperClipRect(a5),d3
				if <>
					repeat
						move.l	d3,a0
		                if cr_MinX(a0)<=d0.w
		                    if cr_MaxX(a0)>=d0.w
		                        if cr_MinY(a0)<=d1.w
		                            if cr_MaxY(a0)>=d1.w
*		                               found a cliprect
										move.l	d2,a0
		                                bra.s rfinish2
		                            endif
		                        endif
		                    endif
		                endif
		                move.l  (a0),d3     * try next cliprect
					until =
				endif
			endif
		endif
		moveq	#-1,d3
		bra.s rend
	endif
rfinish:
	move.l	rp_BitMap(a1),a0
rfinish2:
	muls	bm_BytesPerRow(a0),d1
	cmp.w	#UNLIKELY_WORD,bm_Pad(a0)
	bne.s	not_special
	move.b	bm_Flags(a0),d3
	btst	#IBMB_CHUNKY,d3
	bne.s	is_chunky

not_special:
	move.w	d0,d3
	ext.l	d0	* only use word
	asr.l	#3,d0
	add.l	d0,d1
	moveq	#15,d0
	and.w	d0,d3
	sub.w	d3,d0
	move.b	bm_Depth(a0),d3
	ext.w	d3
	move.w	d3,a2
	lea		bm_Planes(a0),a0
*	end of inline code
	clr.w	d2		* plane counter
	clr.l	d3		* accumulate pixel
* syncronize with last graphics operation
	WAITBLITDONE
	repeat
		move.l	(a0)+,a1	* get pointer to next bitplane
		btst	d0,0(a1,d1.l)	* test bit in memory
		if <>
			bset	d2,d3		* make pixel
		endif
		addq	#1,d2
	until d2=a2.w
rend:
	move.l	a5,d0				* was there a layer?
	if <>
		UNLOCKLAYER
	endif
	move.l	d3,d0
	movem.l	(sp)+,d2/d3/a2/a5
	rts

is_chunky:
	btst	#IBMB_FAST,d3
	bne.s	no_need_to_wait_for_blitter
	WAITBLITDONE
no_need_to_wait_for_blitter:
	move.l	bm_Planes(a0),a0
	add.l	d1,a0
	moveq	#0,d3
	move.b	0(a0,d0.w),d3
	bra	rend

	end

