*******************************************************************************
*
*	$Id: crtools.asm,v 39.4 92/03/26 17:47:20 mks Exp $
*
*******************************************************************************
*
* This file contains a number of routines that were taking up space in
* another file...
*
*******************************************************************************
*
	include 'exec/types.i'
	include 'exec/nodes.i'
	include 'exec/lists.i'
	include 'graphics/clip.i'
	include	'graphics/layers.i'
	include	'graphics/rastport.i'
	include	'layersbase.i'
*
*******************************************************************************
*
		xref	_rectXrect
		xref	_remember_to_free
		xref	abort
*
*******************************************************************************
*
* This is a replacement for the obscured() function
*
*	a0,a1	rectangles...
*
_obscured:	xdef	_obscured
obscured:	moveq.l	#0,d0		; Set default return value...
		move.w	(a0)+,d1	; Get first value...
		cmp.w	(a1)+,d1	; Check it...
		bgt.s	obscured_done	; If > we fail...
		move.w	(a0)+,d1	; Get Y
		cmp.w	(a1)+,d1	; and check it
		bgt.s	obscured_done	; If > we fail...
		move.w	(a0)+,d1	; Get X (max)
		cmp.w	(a1)+,d1	; and check it
		blt.s	obscured_done	; If < we fail...
		move.w	(a0),d1		; Get Y (max)
		cmp.w	(a1),d1		; Check it...
		blt.s	obscured_done	; If < we fail...
*
		moveq.l	#1,d0		; We did not fail, return TRUE
obscured_done:	rts
*
*******************************************************************************
*
* This is a faster version of LinkCR since the compiler does such a
* bad job of it...
*
*	a0=layer
*	a1=cliprect
*
_linkcr:	xdef	_linkcr
linkcr:		lea	lr_ClipRect(a0),a0	; Point at cliprect list
		move.l	(a0),(a1)		; Set up next pointer
		move.l	a1,(a0)			; Link in new cliprect
		rts
*
*******************************************************************************
*
* This routine is a replacement for the C version...
* (Smaller/faster)
*
* One enhancement was to add a check for if we should do a waitblit()
* first.  This means that we will not waitblit until we get to a cliprect
* that needs it...  (This made layers about 12% faster!)
*
*	a0=cliprect
*
_Freecrlist:	xdef	_Freecrlist
Freecrlist:	movem.l	a2/d2,-(sp)	; Save this...
		moveq.l	#0,d2		; Clear WaitBlit() flag...
		move.l	a0,a2		; We need to do this...
freecr_loop:	move.l	a2,d0		; Get cr
		beq.s	freecr_done	; If NULL, we exit...
		move.l	d0,a0		; Get current one into a0...
		move.l	(a2),a2		; Get next one...
*
		tst.l	cr_lobs(a0)	; Check if we need to free some
		beq.s	freecr_nowait	; rasters before we go and do a
		tst.l	cr_BitMap(a0)	; waitblit()...
		beq.s	freecr_nowait
*
* We are about to free some rasters, check if we
* had done a waitblit yet...
*
		tst.l	d2		; Check if we did waitblit yet
		bne.s	freecr_skip	; If so, we skip...
		move.l	a6,d2		; We can save a6 in d2
		move.l	lb_GfxBase(a6),a6	; Get GfxBase
		CALLSYS	WaitBlit	; Call it...
		move.l	d2,a6		; Restore a6...
freecr_skip:	move.l	a0,d2		; Save the cliprect (and set flag)
		bsr.s	free_cr_bm	; Free the bitmap...
		move.l	d2,a0		; Get cliprect back
*
* Now use the code from below to free the cliprect itself
*
freecr_nowait:	bsr.s	freecr_nolobs	; Free the cliprect part
		bra.s	freecr_loop	; Do some more
freecr_done:	movem.l	(sp)+,a2/d2	; Restore
		rts
*
*******************************************************************************
*
* This routine frees the cliprect in a0...
*
_Freecr:	xdef	_Freecr
Freecr:		tst.l	cr_lobs(a0)	; Check for obscured bit...
		beq.s	freecr_nolobs	; If none, skip the free rasters
		move.l	cr_BitMap(a0),d0	; Check for bitmap...
		beq.s	freecr_nolobs	; If none, skip the free rasters
		move.l	a0,-(sp)	; Save a0...
		bsr.s	free_bm		; Free the bitmap...
		move.l	(sp)+,a0	; Restore a0...
*
freecr_nolobs:	move.l	a0,a1		; Get a1 set up
		moveq.l	#cr_SIZEOF,d0	; Get size...
		move.l	a6,-(sp)	; Save a6
		move.l	lb_ExecBase(a6),a6	; Get ExecBase
		CALLSYS	FreeMem		; Free the cliprect
		move.l	(sp)+,a6	; Restore...
		rts
*
*******************************************************************************
*
* free_concealed_rasters(cliprect)
* a0=cliprect
*
_free_concealed_rasters:	xdef	_free_concealed_rasters
free_cr_bm:	move.l	cr_BitMap(a0),d0	; Get bitmap...
		beq.s	free_done		; If NULL, exit...
		clr.l	cr_BitMap(a0)		; Clear it...
free_bm:	move.l	d0,a0			; Get bitmap ready...
		move.l	a6,-(sp)		; Save layersbase...
		move.l	lb_GfxBase(a6),a6	; Get GfxBase
		CALLSYS	FreeBitMap		; Freeit...
		move.l	(sp)+,a6		; Restore layersbase
free_done:	rts				; Exit...
*
*******************************************************************************
*
* get_concealed_rasters(layer,cliprect)
*                       a0    a1
*
_get_concealed_rasters:	xdef	_get_concealed_rasters
		movem.l	d2/d3/a1/a2,-(sp)	; Save some...
		move.l	a0,a2			; Save the layer...
		move.l	lr_rp(a2),a0		; Get RastPort...
		move.l	rp_BitMap(a0),a0	; Get bitmap...
		moveq.l	#$F,d1			; Get mask
		not.w	d1			; Invert it...
		move.l	d1,d0			; And into d0...
		and.w	cr_MaxX(a1),d0		; Get MaxX and mask it...
		and.w	cr_MinX(a1),d1		; Get MinX and mask it...
		sub.w	d1,d0			; And generate the width
		addq.l	#8,d0			; +8
		addq.l	#8,d0			; +16 to make up for masks...
		moveq.l	#1,d1			; Get initial height...
		add.w	cr_MaxY(a1),d1		; Add in the MaxY
		sub.w	cr_MinY(a1),d1		; Subtract the MinY
		moveq.l	#0,d2			; Clear d2
		move.b	bm_Depth(a0),d2		; Get depth...
		moveq.l	#BMF_MINPLANES,d3	; Only the depth of the display
		move.l	a6,-(sp)		; Save LayersBase
		move.l	lb_GfxBase(a6),a6	; Get GraphicsBase
		CALLSYS	AllocBitMap		; Allocate the bitmap
		move.l	(sp)+,a6		; Restore...
		move.l	d0,d3			; Save and check if worked...
		beq.s	gc_exit			; No go?
*
		move.l	lr_LayerInfo(a2),a0	; Get layer info...
		move.l	d3,a1			; Get allocation into a1
		moveq.l	#LMN_BITMAP,d0		; Type of allocation
		bsr	_remember_to_free	; Remember it
		tst.l	d0			; Did it work?
		bne.s	gc_exit			; If so, exit
		exg	d3,d0			; Put NULL into d3
		bsr.s	free_bm			; and Free bitmap in d0
*
gc_exit:	move.l	d3,d0			; Get bitmap...
		bne.s	gc_ok			; If it is here, all ok...
		move.l	lr_LayerInfo(a2),a0	; Get layer info...
		move.l	a0,d1			; Check if we have it...
		bne	abort			; Abort if LayerInfo & Failed!
gc_ok:		movem.l	(sp)+,d2/d3/a1/a2	; Restore...
		move.l	d0,cr_BitMap(a1)	; Set the pointer...
		sne	d0			; Make a boolean...
		rts
*
*******************************************************************************
*
* This is the replacement UnLinkLayer routine...
*
*	a0=layer
*
_unlinklayer:	xdef	_unlinklayer
unlinklayer:	move.l	lr_LayerInfo(a0),a1	; Get the LayerInfo...
		move.l	li_top_layer(a1),d0	; Get TopLayer
		beq.s	unlink_rts		; If NULL, we exit...
		move.l	lr_back(a0),d1		; Get back layer...
		beq.s	unlink_noback		; If none, skip...
*
		exg	d1,a1			; Give me an address reg
		move.l	lr_front(a0),lr_front(a1)	; Store pointer...
		exg	d1,a1			; return a1 to normal...
*
unlink_noback:	cmp.l	d0,a0			; Check if we are top...
		bne.s	unlink_nottop		; If not top...
		move.l	lr_back(a0),li_top_layer(a1)	; Store new top...
		bra.s	unlink_rts		; and we are done...
*
unlink_nottop:	move.l	lr_front(a0),a1		; get front layer
		move.l	lr_back(a0),lr_back(a1)	; Store pointer...
unlink_rts:	rts
*
*******************************************************************************
*
* This routine will insert a layer into the list at the right spot...
*
*	a0=layer in back of...
*	a1=new layer to be in front of a0...
*
_insertlayer:	xdef	_insertlayer
insertlayer:	move.l	a0,d0		; Check if layer is NULL...
		bne.s	ilayer_real	; We have a real one...
*
* We need to find the layer that is above the back...
*
		move.l	lr_LayerInfo(a1),a0	; Get layerinfo
		move.l	li_top_layer(a0),a0	; Get top layer
		move.l	a0,d0			; Check if a real layer
		beq.s	ilayer_noreal		; Empty layer list...
ilayer_find:
		move.l	d0,a0			; Move next layer into a0
		move.l	lr_back(a0),d0		; Get next layer...
		bne.s	ilayer_find		; If last one, we found it
		exg	d0,a0			; Swap d0/a0
		bra.s	ilayer_noreal		; Here we go!!!
*
ilayer_real:	move.l	lr_front(a0),d0		; Get layer above
		move.l	a1,lr_front(a0)		; Store old layer back...
ilayer_noreal:	move.l	a0,lr_back(a1)		; store back pointer...
		move.l	d0,lr_front(a1)		; Now the front pointer...
		move.l	d0,a0			; Put "upper" layer into a0
		tst.l	d0			; Do we have one?
		beq.s	ilayer_top		; If not, we are topmost
		move.l	a1,lr_back(a0)		; Store the back pointer
		rts
ilayer_top:	move.l	lr_LayerInfo(a1),a0	; Get layerinfo
		move.l	a1,li_top_layer(a0)	; Store pointer...
		rts
*
*******************************************************************************
*
* This routine will search the layers starting at the given layer
* for cliprects that match the given one and links it to the list...
*
* a0=start layer
* a1=clip rect to match...
* a2=layer to skip...
*
_vlink_cr:	xdef	_vlink_cr
vlink_cr:	movem.l	a3/a4/a5,-(sp)	; We need some working room...
		move.l	a0,a4		; Move starting layer over...
		move.l	a1,a3		; Move cliprect over...
		lea	cr_MinX(a3),a5	; Point at bounds...
*
vlink_loop:	move.l	lr_back(a4),d0	; Get next layer
		beq.s	vlink_done	; If no more, we are done...
		move.l	d0,a4		; Put it into a4...
		cmp.l	a4,a2		; Check if skip layer...
		beq.s	vlink_loop	; If so, skip and do next one...
		move.l	a5,a1		; Get cliprect bounds
		lea	lr_MinX(a4),a0	; Get layer bounds
		bsr	_rectXrect	; Check if we hit...
		tst.l	d0		; If not, we skip
		beq.s	vlink_loop	; ... this layer
*
* We do our own search_for_cr here such that we can link the cliprects
* via prev and we *KNOW* we will find a matching one...
*
		lea	lr_ClipRect(a4),a0	; Get cliprect pointer...
		movem.l	(a1),d1/a1	; get xmin,ymin,xmax,ymax
vlink_look:	move.l	(a0),a0		; Get next cliprect...
		cmp.l	cr_MinX(a0),d1	; A match?
		bne.s	vlink_look	; check some more...
		cmp.l	cr_MaxX(a0),a1	; A match?
		bne.s	vlink_look	; Look some more...
*
* Ok, we found the cliprect in a0
*
		move.l	a0,cr__p1(a3)	; Store result... (We know it is good)
		move.l	a0,a3		; Get new cliprect into a3
		move.l	a4,cr__p2(a3)	; Flag cliprect as linked...
		moveq.l	#0,d0		; Get a NULL
		move.l	d0,cr__p1(a3)	; Clear the link...
		bra.s	vlink_loop	; Go and do more...
*
vlink_done:	movem.l	(sp)+,a3/a4/a5	; Restore
		rts
*
*****i* layers.library/SortLayerCR ********************************************
*
*   NAME
*	SortLayerCR - Sort the layer's cliprects for scroll raster (V39)
*
*   SYNOPSIS
*	SortLayerCR(layer,dx,dy)
*	            A0    D0 D1
*
*	VOID SortLayerCR(struct Layer *,WORD,WORD);
*
*   FUNCTION
*	This function will sort the give layer's cliprects such that a
*	scroll in the direction given will be optimal.
*
*   NOTE
*	This routine is for Layers/Graphics internal use only.
*	The layer must be locked before calling this routine.
*
*   INPUTS
*	layer	- The layer to be sorted...
*	dx	- x scroll offset
*	dy	- y scroll offset
*
*   SEE ALSO
*
*******************************************************************************
*
SortLayerCR:	xdef	SortLayerCR
		movem.l	a2/a3/a4,-(sp)	; Save these for later...
		clr.l	-(sp)		; Clear a list top on the stack...
*
* The routines are:	s_up		Sort for scroll up
*			s_down		Sort for scroll down
*			s_left		Sort for scroll left
*			s_right		Sort for scroll right
*			s_up_left	Sort for scroll up-left
*			s_up_right	Sort for scroll up-right
*			s_down_left	Sort for scroll down-left
*			s_down_right	Sort for scroll down-right
*
* Ok, for the simple cases, we just do the simple sort...
* (That means dy==0 or dx==0)
*
* Since these are most common, we check for them first...
*
		tst.w	d1		; Check for dy
		bne.s	dy_non_null	; If dy is not NULL...
		lea	s_left(pc),a4	; Get left sort...
		tst.w	d0		; Check dx...
		bpl.s	StartSort	; Ok, so we do a simple one
		lea	s_right(pc),a4	; Get the right sort...
		bra.s	StartSort	; Do the sort...
*
* Ok, so we have dy, lets check dx...
*
dy_non_null:	tst.w	d0		; Check the dx
		bne.s	hard_sort	; If dx & dy are not null, hard sort
dy_sort:	lea	s_up(pc),a4	; Get up sort...
		tst.w	d1		; Check the type of dy...
		bpl.s	StartSort	; Start the sort
		lea	s_down(pc),a4	; Sort down...
		bra.s	StartSort	; Do the sort...
*
*
* Ok, we have a hard one.  This will take a while anyway, so...
*
hard_sort:	rol.w	#1,d0		; Get negative bit into bit 1
		and.w	#1,d0		; Mask all but it...
		swap	d0		; Move d0 word into top word
		move.w	d1,d0		; Get other word...
		rol.l	#1,d0		; Move negative bit into high word
		swap 	d0		; Get high word back down...
		add.w	d0,d0		; *2
		add.w	d0,d0		; *4
		move.l	sortTab(pc,d0.w),a4	; Get function pointer
*
* We will need the cliprect sorted in dy first, so, we will make dx=0
* and call ourselves
*
		moveq.l	#0,d0		; Clear dx
		bsr.s	SortLayerCR	; Sort just in dy...
		bra.s	StartSort	; Do the sort
*
*****
*
* This is a little cheat:
* We take the sign bit from X and Y and produce an offset into this
* table to find the function needed...
*
sortTab:	dc.l	s_up_left	; (0,0) = Positive X, Positive Y
		dc.l	s_down_left	; (0,1) = Positive X, Negative Y
		dc.l	s_up_right	; (1,0) = Negative X, Positive Y
		dc.l	s_down_right	; (1,1) = Negative X, Negative Y
*
*****
*
* These are the simple sort test routines  They split up to keep
* the branches short...
*
* Inputs:	a1 = Cliprect to be inserted
*		a3 = Cliprect on list to be checked against
*		d0/d1 = Scrap - free for use in routine
*
* Result:	BGT if cliprect could be after this one...
*		BLT if cliprect MUST be before this one...
*
s_up:		; We are scrolling up, so top edge only...
		move.w	cr_MinY(a1),d1	; Get new CR top edget
		cmp.w	cr_MinY(a3),d1	; Check it...
		bra.s LayerIns_Cont	; Continue the search
*
s_down:		; We are scrolling down, so bottom edge only...
		move.w	cr_MaxY(a3),d1	; Get list CR
		cmp.w	cr_MaxY(a1),d1	; Check it...
		bra.s LayerIns_Cont	; Continue the search
*
s_left:		; We are scrolling left, so left edge only...
		move.w	cr_MinX(a1),d1	; Get new CR
		cmp.w	cr_MinX(a3),d1	; Check it...
		bra.s LayerIns_Cont	; Continue the search
*
s_right:	; We are scrolling right, so right edget only...
		move.w	cr_MaxX(a3),d1	; Get list CR
		cmp.w	cr_MaxX(a1),d1	; Check it...
		bra.s LayerIns_Cont	; Continue the search
*
****
*
* Here is the main sort routine.
* We enter here with:		a4 = Pointer to test routine
*				sp = Pointer to new list
*				a0 = layer (Do *NOT* trash!)
*				a1/a2/a3/d0/d1 = scrap
*
StartSort:	move.l	lr_ClipRect(a0),d0	; Get first CR
		beq.s	SortLayer_Done		; No cliprects? Exit...
*
* The first cliprect always goes into the list easy...
*
		move.l	sp,a2		; Get cliprect list
		move.l	d0,a1		; Get cliprect
*
* Insert the cliprect (a1) into the list after cliprect (a2)...
*
Insert_Clip:	move.l	(a1),d0		; Get old next pointer...
		move.l	(a2),(a1)	; Set up new next pointer...
		move.l	a1,(a2)		; Add new CR to list
		tst.l	d0		; Check for more cliprects
		beq.s	SortLayer_Done	; If none, cleanup and exit...
		move.l	d0,a1		; Get current cliprect
*
* Ok, this is the main sort loop.
*
		move.l	sp,a2		; Start at the top of the new list
*
* Ok, now find a good spot...
*
LayerIns_Loop:	move.l	(a2),d0		; Get next cliprect...
		beq.s	Insert_Clip	; If NULL, we insert at end...
		move.l	d0,a3		; Get pointer to next clip...
		jmp	(a4)		; Do the test...
*
LayerIns_Cont:	ble.s	Insert_Clip	; If right spot, insert it...
LayerIns_GT:	move.l	a3,a2		; Move to next cliprect
		bra.s	LayerIns_Loop	; And loop back for more...
*
* Clean up and exit
*
SortLayer_Done:	move.l	(sp)+,lr_ClipRect(a0)	; Set up new list
		movem.l	(sp)+,a2/a3/a4		; Restore
		rts
*
****
*
* Ok, here are the test routines for the hard sorts
*
* Inputs:	a1 = Cliprect to be inserted
*		a3 = Cliprect on list to be checked against
*		d0/d1 = Scrap - free for use in routine
*
* Result:	BGT if cliprect could be after this one...
*		BLT if cliprect MUST be before this one...
*
s_up_left:	; We are scrolling up and left, so a bit more complex...
		move.w	cr_MinY(a1),d1	; Get new CR minY
		cmp.w	cr_MaxY(a3),d1	; Are we below this one?
		bgt.s	LayerIns_GT	; If so, we are after him
		move.w	cr_MinX(a1),d1	; Get our MinX
		cmp.w	cr_MaxX(a3),d1	; Check against list MaxX
		bra.s LayerIns_Cont	; Continue the search
*
s_down_left:	; We are scrolling down and left, so a bit more complex...
		move.w	cr_MinY(a3),d1	; Get list CR minY
		cmp.w	cr_MaxY(a1),d1	; Check; are we above him?
		bgt.s	LayerIns_GT	; If so, we are after him
		move.w	cr_MinX(a1),d1	; Get our MinX
		cmp.w	cr_MaxX(a3),d1	; Check against list MaxX
		bra.s	LayerIns_Cont	; Continue the search
*
s_up_right:	; We are scrolling up and right, so a bit more complex...
		move.w	cr_MinY(a1),d1	; Get new CR minY
		cmp.w	cr_MaxY(a3),d1	; Are we below this one?
		bgt.s	LayerIns_GT	; If so, we are after him
		move.w	cr_MinX(a3),d1	; Get list CR minX
		cmp.w	cr_MaxX(a1),d1	; Check if we are past list CR
		bra.s	LayerIns_Cont	; Continue the search
*
s_down_right:	; We are scrolling down and right, so a bit more complex...
		move.w	cr_MinY(a3),d1	; Get list CR minY
		cmp.w	cr_MaxY(a1),d1	; Check; are we above him?
		bgt.s	LayerIns_GT	; If so, we are after him
		move.w	cr_MinX(a3),d1	; Get list CR minX
		cmp.w	cr_MaxX(a1),d1	; Check if we are past list CR
		bra.s	LayerIns_Cont	; Continue the search
*
*******************************************************************************
*
		end
