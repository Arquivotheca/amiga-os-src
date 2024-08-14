*******************************************************************************
*
*	$Id: writepixel.asm,v 39.4 92/02/10 13:12:17 chrisg Exp $
*
*******************************************************************************

  include 'exec/types.i'     * Required data type definitions
  include 'graphics/gfx.i'    * BitMap structure definition
  include 'graphics/gfxbase.i'
  include 'graphics/clip.i'   * Layer structure definition
  include 'graphics/rastport.i'   * RastPort structure definition
  include 'hardware/blit.i'    * Blitter structure definitions
  include 'hardware/custom.i'  * Amiga hardware registers
  include 'submacs.i'     * Macros for subroutines called
  include '/sane_names.i'

	section	graphics
  xdef _WritePixel

	xref _custom      * extern struct AmigaRegs ioregs;
	xref waitblitdone

******* graphics.library/WritePixel ******************************************
*
*   NAME
*       WritePixel -- Change the pen num of one specific pixel in a
*                     specified RastPort.
*
*   SYNOPSIS
*       error = WritePixel(  rp, x,  y)
*         d0                 a1 D0  D1
*
*	LONG WritePixel( struct RastPort *, SHORT, SHORT );
*
*   FUNCTION
*       Changes the pen number of the selected pixel in the specified
*       RastPort to that currently specified by PenA, the primary
*       drawing pen. Obeys minterms in RastPort.
*
*   INPUTS
*       rp - a pointer to the RastPort structure
*       (x,y) - point within the RastPort at which the selected
*           pixel is located.
*
*   RESULT
*       error = 0 if pixel succesfully changed
*	      = -1 if (x,y) is outside the RastPort
*
*   BUGS
*
*   SEE ALSO
*       ReadPixel() graphics/rastport.h
*
******************************************************************************

*		this kluge needed to preserve RastPort pointer
_WritePixel:
	movem.l d2/d3/a3/a4/a5,-(sp)		* Save registers used
* begin downcoded stuff
	ifne	rp_Layer
		fail
	endc
	move.l	(a1),a5
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
								beq.s	finish		* simple, on screen
*									not on screen
*									special for obscuration
									move.w	cr_MinX(a0),d2
									and.w	#$FFF0,d2
									sub.w	d2,d0		* offset x
									sub.w	cr_MinY(a0),d1	* offset y
									move.l	cr_BitMap(a0),d2
									if =
*										no bitmap here
										bra	wend
									endif
									move.l	d2,a0
									bra.s	finish2
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
		                                bra.s finish2
		                            endif
		                        endif
		                    endif
		                endif
		                move.l  (a0),d3     * try next cliprect
					until =
				endif
			endif
		endif
		bra wend
	endif
finish:
	move.l	rp_BitMap(a1),a0
finish2:
	ifne bm_BytesPerRow
		fail
	endc
	muls	(a0),d1		* could be BytesPerRow(a0)
	move.w	d0,d3
	ext.l	d0	* only use word
	asr.l	#3,d0
	add.l	d0,d1
	move.l	d1,a5
	moveq	#7,d0
	and.w	d0,d3
	sub.w	d3,d0
	move.b	bm_Depth(a0),d3
	subq	#1,d3
	lea	bm_Planes(a0),a0
* end downcodeing

	move.b	rp_Mask(a1),d1

	lea  _custom,a4       * io = &ioregs;
	OWNBLITTER         * ownblitter(GB);
	WAITBLITDONE  a4    * waitblitdone();

	lea	rp_minterms(a1),a3
; now, (a0)=planes
;       a5=byte offset from beginning of plane
;	d0=bit number
;	d1=mask
;	(a3)=minterms 2a=clr ea=set 6a=xor
;       d3.b=depth-1
; kludge! kludge! kludge alert! :
; I can't go off of the rp_FgPen/BgPen/DrawMode, because dpaint pokes minterms!!
	
pl_loop:
	move.b	(a3)+,d2
	move.l	(a0)+,a4
	lsr.b	#1,d1
	bcc.s	done_plane
	tst.b	d2
	bpl.s	no_set
	add.b	d2,d2
	bpl.s	no_set1
	bset	d0,0(a4,a5.l)
no_set1:
	dbra	d3,pl_loop
	bra.s	end_set_px
no_set:
	add.b	d2,d2		; 6a<<1=-
	bmi.s	is_xor
	bclr	d0,0(a4,a5.l)
	dbra	d3,pl_loop
	bra.s	end_set_px

is_xor:
	bchg	d0,0(a4,a5.l)
done_plane:
	dbra	d3,pl_loop
	
end_set_px:
	DISOWNBLITTER     * disownblitter(GB);

	move.l	rp_Layer(a1),d0
	if <>
		move.l	d0,a5
		UNLOCKLAYER
	endif
	moveq	#0,d0
	movem.l (sp)+,d2/d3/a3/a4/a5  * Restore registers used
	rts


wend:
; return failure
	move.l	rp_Layer(a1),d0
	if <>
		move.l	d0,a5
		UNLOCKLAYER
	endif
	moveq	#-1,d0
	movem.l (sp)+,d2/d3/a3/a4/a5  * Restore registers used
	rts

	end
