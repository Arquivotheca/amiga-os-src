*******************************************************************************
*
*	$Id: backfill.asm,v 38.15 92/10/15 17:15:55 mks Exp $
*
*******************************************************************************
*
	include 'exec/types.i'
	include 'exec/nodes.i'
	include 'exec/lists.i'
	include	'exec/memory.i'
	include	'graphics/gfx.i'
	include 'graphics/clip.i'
	include	'graphics/layers.i'
	include	'graphics/rastport.i'
	include 'utility/hooks.i'
	include	'layersbase.i'
*
	xref	_intersect
	xref	LockLayer
	xref	UnlockLayer
*
*******************************************************************************
*
* FullBackFill(layer)
*
* This routine calls the backfill hook on all cliprects in the layer...
*
*	a0=layer
*
_FullBackFill:	xdef	_FullBackFill
		move.l	lr_rp(a0),a1		; Get RastPort...
		move.l	lr_BackFill(a0),a0	; Get hook...
		move.l	a2,-(sp)		; Save a2
		sub.l	a2,a2			; Clear a2 (no rectangle)
		CALLSYS	DoHookClipRects		; Do the hook (via LVO)
		move.l	(sp)+,a2		; Restore
		rts
*
*******************************************************************************
*
* Ok, so we needed a DoHookRect function that will call the given hook
* for each rectangle (or do the default hook which is colour 0 erase)
* This will then replace the graphics.library rastrect.c code (and others)
*
******* layers.library/DoHookClipRects ****************************************
*
*    NAME
*	DoHookClipRects - Do the given hook for each of the ClipRects    (V39)
*
*    SYNOPSIS
*	DoHookClipRects(hook,rport,rect)
*	                a0   a1    a2
*
*	void DoHookClipRects(struct Hook *,struct RastPort *,struct Rectangle *);
*
*    FUNCTION
*	This function will call the given hook for each cliprect in the
*	layer that can be rendered into.  This is how the backfill hook
*	in Layers is implemented.  This means that hidden simple-refresh
*	cliprects will be ignored.  It will call the SuperBitMap cliprects,
*	smart refresh off-screen cliprects, and all on screen cliprects.
*	If the rect parameter is not NULL, the cliprects are bounded to
*	the rectangle given.
*
*    INPUTS
*	hook - pointer to layer callback Hook which will be called
*	       with object == (struct RastPort *) result->RastPort
*	       and message == [ (Layer *) layer, (struct Rectangle) bounds,
*	                        (LONG) offsetx, (LONG) offsety ]
*
*	       This hook should fill the Rectangle in the RastPort
*	       with the BackFill pattern appropriate for offset x/y.
*
*	       If hook is LAYERS_BACKFILL, the default backfill is
*	       used for the layer.
*
*	       If hook is LAYERS_NOBACKFILL, the layer will not be
*	       backfilled (NO-OP).
*
*	rport- A pointer to the RastPort that is to be operated on.
*	       This function will lock the layer if the RastPort is
*	       layered...
*	       If the rport is non-layered your hook will be called with
*	       the rectangle as passed, the RastPort, and a NULL layer...
*
*	rect - The bounding rectangle that should be used on the layer.
*	       This rectangle "clips" the cliprects to the bound given.
*	       If this is NULL, no bounding will take place.
*	       *MUST* not be NULL if the RastPort is non-layered!
*
*    NOTES
*	The RastPort you are passed back is the same one passed to the
*	function.  You should *not* use "layered" rendering functions
*	on this RastPort.  Generally, you will wish to do BitMap operations
*	such as BltBitMap().  The callback is a raw, low-level rendering
*	call-back.  If you need to call a rendering operation with a
*	RastPort, make sure you use a copy of the RastPort and NULL the
*	Layer pointer.
*
*    SEE ALSO
*	graphics/clip.h utility/hooks.h
*
*******************************************************************************
*
* Registers:
*		a2 - The Layer
*		a3 - Temp Rectangle pointer
*		a4 - Temp Rectangle pointer
*		a5 - The current cliprect...
*
*		d2 - layer->bounds.MinX - layer->Scroll_X
*		d3 - layer->bounds.MinY - layer->Scroll_Y
*		d4 - The layer (storage)
*		d5 - Temp RASTPORT
*		d6 - The original rectangle bounds
*		d7 - The hook
*
DoHookClipRects:	xdef	DoHookClipRects
		move.l	a0,d0			; Get hook...
		subq.l	#1,d0			; Check for noop...
		bne.s	dhr_RealHook		; If not noop, we do real hook
		rts				; Exit (noop hook)
dhr_RealHook:	movem.l	a2-a5/d2-d7,-(sp)	; Save these...
		move.l	a2,d6			; Get the original rectangle
		move.l	rp_Layer(a1),a2		; Get the original layer
		move.l	a0,d7			; Get the hook...
		move.l	a1,d5			; Save the RastPort...
		move.l	rp_BitMap(a1),-(sp)	; Save bitmap...
		subq.l	#8,sp			; Make space for a rectangle...
*
* Now check for a layer and do the "right thing"
*
		move.l	a2,d4			; Store layer...
		bne.s	DoHookLayer		; A layer?
*
* We have no layer, just call the hook once for this rectangle...
*
		move.l	d7,a0			; Get the hook...
		move.l	d4,a1			; Get layer...
		move.l	d5,a2			; Get RastPort...
		move.l	d6,a3			; Get bounds...
		move.l	d6,a4			; Other bounds...
		moveq.l	#0,d2			; Clear offsets...
		moveq.l	#0,d3			; Clear offsets...
		pea	dhr_CleanUp(pc)		; Put the cleanup on stack...
		bra	DoBackFill		; Do backfill and exit...
*
* We have a layer so lock it first...
*
DoHookLayer:	move.l	a2,a1			; Get layer...
		bsr	LockLayer		; Lock the layer (layers call)
*
* Precalculate the offset difference...
*
		move.w	lr_MinX(a2),d2		; Get MinX
		sub.w	lr_Scroll_X(a2),d2	; - layer->Scroll_X
		move.w	lr_MinY(a2),d3		; Get MinY
		sub.w	lr_Scroll_Y(a2),d3	; - layer->Scroll_Y
*
* Now, calculate a new rectangle for the layer from the original rectangle...
*
		move.l	sp,a1			; Address of new rectangle...
		move.l	d6,a0			; Get the rectangle
		move.l	d6,-(sp)		; Save old rectangle
		beq.s	dhr_NoRect1		; If none...
		move.l	a1,d6			; Store new rectangle...
		move.w	(a0)+,d0		; Get MinX
		add.w	d2,d0			; Add in delta
		move.w	d0,(a1)+		; Save it...
		move.w	(a0)+,d0		; Get MinY
		add.w	d3,d0			; Add in delta
		move.w	d0,(a1)+		; Save it...
		move.w	(a0)+,d0		; Get MaxX
		add.w	d2,d0			; Add in delta
		move.w	d0,(a1)+		; Save it...
		move.w	(a0)+,d0		; Get MaxY
		add.w	d3,d0			; Add in delta
		move.w	d0,(a1)+		; Save it...
*
* Ok, so we now wish to walk the cliprects in the layer and do as needed...
*
dhr_NoRect1:	lea	lr_ClipRect(a2),a5	; Get cliprect list...
		subq.l	#8,sp			; Make space for one rectangle
dhr_Loop:	move.l	(a5),d0			; Get next cliprect
		beq.s	dhr_EndLoop		; If NULL, end of cliprect list
		move.l	d0,a5			; put into a5
		lea	cr_MinX(a5),a3		; Get rectangle (cliprect)
		tst.l	d6			; Check for NULL limits rect...
		beq.s	dhr_NoRect2		; If NULL, skip limits work...
		move.l	a3,a0			; Get limits set up...
		move.l	d6,a1			; Get active rectangle
		move.l	sp,a2			; Get new rectangle storage
		bsr	_intersect		; Do the intersect
		move.l	(a2)+,d0		; Get MinX/MinY
		move.l	(a2)+,d1		; Get MaxX/MaxY
		cmp.w	d0,d1			; Max - Min  (y)
		blt.s	dhr_Loop		; Less than 0?
		cmp.l	d0,d1			; Max - Min  (x/y)
		blt.s	dhr_Loop		; Less than 0?
		move.l	sp,a3			; a3 now points at this
dhr_NoRect2:	move.l	a3,a4			; Assume on screen first...
*
* Set up for call...
*
		move.l	d4,a1			; Put layer into real reg...
		move.l	d7,a0			; Get hook...
		move.l	d5,a2			; Get the rastport...
		tst.l	cr_lobs(a5)		; Are we obscured?
		bne.s	dhr_Hidden		; If so, we are hidden...
		bsr	DoBackFill		; Do the backfill...
		bra.s	dhr_Loop		; and loop back for next...
*
* Now, for hidden fills...
*
dhr_Hidden:	tst.l	cr_BitMap(a5)		; Do we have backing store?
		beq.s	dhr_Loop		; If not, we loop back...
*
* Build bounds within the backing store bitmap...
*
		moveq.l	#$F,d1			; Set up mask
		and.w	cr_MinX(a5),d1		; Mask for d1...
		sub.w	cr_MinX(a5),d1		; Subtract CR offset...
		move.w	ra_MaxX(a4),d0		; Get MaxX
		add.w	d1,d0			; addjust it for everything
		swap	d0			; Put MaxX in high word...
		move.w	ra_MaxY(a4),d0		; So adjust the Y...
		sub.w	cr_MinY(a5),d0		; to match...
		move.l	d0,-(sp)		; Put MaxX/Y on stack
*
		add.w	ra_MinX(a4),d1		; Make MinX...
		swap	d1			; Put MinX in upper word
		move.w	ra_MinY(a4),d1		; Get MinY
		sub.w	cr_MinY(a5),d1		; Make real MinY
		move.l	d1,-(sp)		; Put MinX/Y on stack
		move.l	sp,a3			; Get pointer to bounds
*
* Now change the bitmap of the rastport...
*
		move.l	rp_BitMap(a2),-(sp)	; Save bitmap...
		move.l	cr_BitMap(a5),rp_BitMap(a2)	; Get new bitmap
		bsr.s	DoBackFill		; do the backfill...
		move.l	(sp)+,rp_BitMap(a2)	; Restore bitmap
*
		addq.l	#8,sp			; Clean off the bounds
		bra.s	dhr_Loop		; Go back for more...
*
* End of the main loop.  Now all we have to check for is the SuperBitMap
* But first, lets get back the original rectangle...
*
dhr_EndLoop:	addq.l	#8,sp			; Get stack back...
		move.l	(sp)+,d6		; Get original back
*
* Ok, now set up for doing (maybe?) the super bitmap part...
*
		move.l	d4,a2			; Get layer back...
		move.l	lr_SuperBitMap(a2),d0	; Get SuperBitMap
		beq.s	dhr_CleanUp		; If no bitmap, exit...
*
* Ok, now set up as needed...
*
		move.l	d5,a0			; Get RastPort
		move.l	d0,rp_BitMap(a0)	; Set bitmap...
		moveq.l	#0,d2			; Clear the deltas...
		moveq.l	#0,d3			; (not used in superbitmap)
		lea	lr_SuperClipRect(a2),a5	; Point at cliprect list
dhr_SLoop:	move.l	(a5),d0			; Get next cliprect...
		beq.s	dhr_CleanUp		; If no more, exit...
		move.l	d0,a5			; put into a5
		lea	cr_MinX(a5),a3		; Get rectangle
		tst.l	d6			; Check for NULL limits rect...
		beq.s	dhr_NoRect3		; If NULL, skip limits work...
		move.l	a3,a0			; Get limits set up...
		move.l	d6,a1			; Get active rectangle
		move.l	sp,a2			; Get new rectangle storage
		bsr	_intersect		; Do the intersect
		move.l	(a2)+,d0		; Get MinX/MinY
		move.l	(a2)+,d1		; Get MaxX/MaxY
		cmp.w	d0,d1			; Max - Min
		blt.s	dhr_SLoop		; Less than 0?
		swap	d0			; Get other coordinate
		swap	d1			; ... into d0/d1
		cmp.w	d0,d1			; Max - Min
		blt.s	dhr_SLoop		; Less than 0?
		move.l	sp,a3			; a3 now points at this
dhr_NoRect3:	move.l	a3,a4			; Offset is the same...
*
* Set up for call
*
		move.l	d4,a1			; Put layer into real reg...
		move.l	d7,a0			; Get hook...
		move.l	d5,a2			; Get the rastport...
		bsr.s	DoBackFill		; Do the backfill...
		bra.s	dhr_SLoop		; Go back for more...
*
* Now, clean up the stack
*
dhr_CleanUp:	addq.l	#8,sp			; Adjust stack...
		move.l	d5,a0			; Get RastPort back...
		move.l	(sp)+,rp_BitMap(a0)	; Get bitmap back...
		tst.l	d4			; Check for layer pointer...
		beq.s	dhr_NoWork		; If no layer, exit...
		move.l	d4,a0			; Get layer...
		bsr	UnlockLayer		; Unlock the layer...
dhr_NoWork:	movem.l	(sp)+,a2-a5/d2-d7	; Restore...
		rts				; and exit...
*
*******************************************************************************
*
* This routine is the new, faster, smaller backfill hook routine.
* You call it with:
*
*	a0=hook
*	a1=layer
*	a2=RastPort
*	a3=Bounds (for bitmap)
*	a4=Bounds (for offset)
*	d2=delta X offset
*	d3=delta Y offset
*
* Special for graphics use *ONLY* and *ONLY* from DoHookClipRects...
*	a5=ClipRect (only for Graphics use of DoHookClipRects...)
*
* It will automatically handle HOOK==0 and HOOK==1 cases...
*
DoBackFill:	move.l	a0,d0	; Get the hook
		beq.s	Default	; If we have a NULL, it is default
		subq.l	#1,d0	; Drop by 1
		bne.s	DoFill	; If not 1, then we do something
		rts		; It was 1, so just return...
*
* Ok, we have a normal hook, so do the magic...
* (We need to place some stuff on the stack)
*
DoFill:		move.w	ra_MinY(a4),d0		; Get Y offset
		sub.w	d3,d0			; Subtract delta
		ext.l	d0			; Make a long...
		move.l	d0,-(sp)		; ...and place on stack
*
		move.w	ra_MinX(a4),d0		; Get X offset
		sub.w	d2,d0			; Subtract delta
		ext.l	d0			; Make a long...
		move.l	d0,-(sp)		; ...and place on stack
*
		move.l	ra_MaxX(a3),-(sp)	; Now do the whole bounds
		move.l	ra_MinX(a3),-(sp)	; on the stack...
		move.l	a1,-(sp)		; And place the layer
*
		move.l	sp,a1			; A1 now points at packet
		move.l	a6,-(sp)		; Save a6
		move.l	h_Entry(a0),a6		; Get hook address
		jsr	(a6)			; Call the hook...
*
		move.l	(sp)+,a6		; Get a6
		lea	5*4(sp),sp		; Clean up the 5 longwords
		rts				; And we are done...
*
*******************************************************************************
*
* This routine is the new, faster, smaller backfill hook routine.
* You call it with:
*
*	a0=Cliprect
*	a1=layer
*
* It will automatically handle HOOK==0 and HOOK==1 cases...
* (Note:  this is placed here to make short branch work...)
* (Note:  This is only for calling on-screen cliprects...)
*
_CallBackFillHook:	xdef	_CallBackFillHook
		movem.l	d2/d3/a2/a3/a4,-(sp)	; Save these
*
* Make copy of RastPort
*
		moveq.l	#rp_SIZEOF,d0		; Get size of RP
		sub.l	d0,sp			; Make space on stack
		moveq.l	#((rp_SIZEOF)/4)-1,d0	; Get number of long words...
		move.l	sp,a4			; Get new RP
		move.l	lr_rp(a1),a2		; Get RP
copyRP_Loop:	move.l	(a2)+,(a4)+		; Copy it...
		dbra.s	d0,copyRP_Loop		; Do the whole thing...
		move.l	sp,a2			; Get new RP
		clr.l	rp_Layer(a2)		; Clear layer pointer...
*
		lea	cr_MinX(a0),a4		; Get rectangle (bounds)
		move.l	a4,a3			; into both a3 and a4
*
		move.w	lr_MinX(a1),d2		; Get MinX
		sub.w	lr_Scroll_X(a1),d2	; - layer->Scroll_X
		move.w	lr_MinY(a1),d3		; Get MinY
		sub.w	lr_Scroll_Y(a1),d3	; - layer->Scroll_Y
*
		move.l	lr_BackFill(a1),a0	; Get the hook...
		bsr.s	DoBackFill		; Do the backfill
		moveq.l	#rp_SIZEOF,d0		; Get RP size
		add.l	d0,sp			; Release RP from stack...
		movem.l	(sp)+,d2/d3/a2/a3/a4	; Restore
		rts
*
*******************************************************************************
*
* This is the default backfill...
*
Default:	movem.l	d2-d7/a6,-(sp)		; Save these
*
* Set up for the empty blit...
*
		move.l	rp_BitMap(a2),a1	; Get bitmap
		move.l	a1,a0			; (both are the same)
*
		moveq.l	#0,d2			; Clear d2
		move.w	ra_MinX(a3),d2		; Get X pos
*
		moveq.l	#0,d3
		move.w	ra_MinY(a3),d3		; Get Y pos
*
		moveq.l	#0,d4
		move.w	ra_MaxX(a3),d4
		sub.l	d2,d4
		addq.l	#1,d4			; Get X size
*
		moveq.l	#0,d5
		move.w	ra_MaxY(a3),d5
		sub.l	d3,d5
		addq.l	#1,d5			; Get Y size
*
		move.l	d2,d0			; X Source
		move.l	d3,d1			; Y Source
		moveq.l	#0,d6			; NULL minterm
		moveq.l	#-1,d7			; FF mask
*
		move.l	lb_GfxBase(a6),a6	; Get GfxBase
		CALLSYS	BltBitMap		; Blit it
*
		movem.l	(sp)+,d2-d7/a6		; Restore
		rts
*
*******************************************************************************
*
		end
