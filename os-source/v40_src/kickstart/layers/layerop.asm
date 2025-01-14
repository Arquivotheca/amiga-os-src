*******************************************************************************
*
*	$Id: layerop.asm,v 38.3 91/08/22 18:01:14 mks Exp $
*
*******************************************************************************
*
	include "exec/types.i"
	include "exec/nodes.i"
	include "exec/lists.i"
	include "exec/ports.i"
	include "graphics/gfx.i"
	include "graphics/clip.i"
	include "graphics/rastport.i"
*
	include "layersbase.i"
*
*******************************************************************************
*
* This function takes a rectangle and a layer and calls the given function
* once for each cliprect in the layer that overlaps the rectangle.  This
* is used to build cliprects for new layers and when cliprects are completely
* rebuilt.
*
*******************************************************************************
*
*  void __asm layerop(register __a0 struct Layer *
*                     register __a1 void __stdargs (*)(struct Layer *,struct Rectangle *,void *)
*                     register __a2 struct Rectangle *
*                     register __a3 void *)
*
_layerop:	xdef	_layerop
		movem.l	a2/a3/a4/a5,-(sp)	; save these...
		move.l	a0,a4			; We need to store these
		move.l	a1,a5			; in safe locations...
*
* Ok, so the registers are now:
*
* a2 = struct Rectangle *
* a3 = void *
* a4 = struct Layer *
* a5 = function pointer
* a6 = layers base (not to be touched!)
*
		move.l	a2,a0			; Rectangle...
		lea	lr_MinX(a4),a1		; and another...
		bsr.s	_rectXrect		; Do the intersect test
		tst.w	d0			; Check the result...
		beq.s	exit_layerop		; Nothing to do...
*
* Now, we know we have an intersection, so build the rectangle...
*
		move.l	a2,a0			; Get input rectangle
		lea	lr_MinX(a4),a1		; Get layer rectangle
		subq.l	#8,sp			; Make room for new rectangle
		move.l	sp,a2			; Store that one...
		bsr.s	_intersect		; Do the intersection
		move.l	lr_ClipRect(a4),a1	; Get ClipRect
		bsr.s	layerop			; Do the main work...
		addq.l	#8,sp			; Recover space of rectangle
exit_layerop:	movem.l	(sp)+,a2/a3/a4/a5	; restore...
		rts
*
*******************************************************************************
*
* BOOL __asm rectXrect(register __a0 rectangle *,register a1 rectangle *)
*
* Replaces the C version...  Only trashes d0 and d1!!!
*
_rectXrect:	xdef	_rectXrect
		moveq.l	#0,d0		; Initial FALSE reading
		move.w	ra_MaxX(a0),d1	; Check Max vs Min X collision
		cmp.w	ra_MinX(a1),d1
		blt.s	exit_rect	; If we miss, we skip...
		move.w	ra_MinX(a0),d1	; Check the other X direction
		cmp.w	ra_MaxX(a1),d1
		bgt.s	exit_rect
		move.w	ra_MaxY(a0),d1	; Check Max vs Min Y collision
		cmp.w	ra_MinY(a1),d1
		blt.s	exit_rect
		move.w	ra_MinY(a0),d1	; Check the other Y direction
		cmp.w	ra_MaxY(a1),d1
		bgt.s	exit_rect
		moveq.l	#1,d0		; Set TRUE flag for collision
exit_rect:	rts			; And return result...
*
*******************************************************************************
*
* void __asm intersect(register __a0 struct Rectangle *
*                      register __a1 struct Rectangle *
*                      register __a2 struct Rectangle *)
*
_intersect:	xdef	_intersect
		move.l	a2,-(sp)	; Save this...
*
* First find largest MinX
*
		move.w	(a0)+,d0	; Get first rectangle ra_MinX
		move.w	(a1)+,d1	; Get other one...
		cmp.w	d0,d1		; Check which is larger...
		ble.s	1$		; If d0 is larger, no swap...
		move.w	d1,d0		; set up d0
1$		move.w	d0,(a2)+	; Save it...
*
* Next find largest MinY
*
		move.w	(a0)+,d0	; Get first rectangle ra_MinY
		move.w	(a1)+,d1	; Get other one...
		cmp.w	d0,d1		; Check which is larger...
		ble.s	2$		; If d0 is larger, no swap...
		move.w	d1,d0		; set up d0
2$		move.w	d0,(a2)+	; Save it...
*
* Now find smallest MaxX
*
		move.w	(a0)+,d0	; Get first rectangle ra_MaxX
		move.w	(a1)+,d1	; Get other one...
		cmp.w	d0,d1		; Check which is larger...
		bge.s	3$		; If d0 is smaller, no swap...
		move.w	d1,d0		; set up d0  (we want the smallest)
3$		move.w	d0,(a2)+	; Save it...
*
* Now find smallest MaxY
*
		move.w	(a0)+,d0	; Get first rectangle ra_MaxY
		move.w	(a1)+,d1	; Get other one...
		cmp.w	d0,d1		; Check which is larger...
		bge.s	4$		; If d0 is smaller, no swap...
		move.w	d1,d0		; set up d0  (we want the smallest)
4$		move.w	d0,(a2)+	; Save it...
*
* Ok, so we now have the intersection rectangle
*
		move.l	(sp)+,a2	; Restore...
		rts
*
*******************************************************************************
*
* This is the internal routine used by layerop() to recurse...
* It has been optimized to do minimal memory accesses and to
* reduce the stack used per recursion from 64 bytes to 16 bytes.
* This means that more layers can be operated on without blowing
* the caller's stack.
*
* Inputs:
*	a5 = function to call...
*	a4 = struct layer *
*	a3 = void * (for the function)
*	a2 = rectangle
*	a1 = cliprect
*
* Trashes: a0, a1, d0, d1
*
layerop_loop:	move.l	cr_Next(a1),a1	; Get next cliprect...
layerop:	move.l	a1,d0		; Check if we have a ClipRect
		bne.s	check_clip	; If we have one, check it...
*
* Alright:  We are now going to call the function given...
* To do this, we stack the parameters as given as the function is
* a stack based call.
* We also save our work registers...
*
* Now stack the args:
* (struct Layer *,struct Rectangle *,void *)
*
		move.l	a3,-(sp)	; void *			(stuff)
		move.l	a2,-(sp)	; struct Rectangle *		r
		move.l	a4,-(sp)	; struct Layer *		lp
		jsr	(a5)		; Call the function...
		lea	3*4(sp),sp	; Adjust stack back
		rts			; We are done...
*
* Now, check the cliprect
*
check_clip:	move.l	cr_lobs(a1),d0	; Check if screen rectangle...
		beq.s	layerop_loop	; go and check the next one if so...
		lea	cr_MinX(a1),a0	; Get rectangle pointer
		exg	a2,a1		; Swap in my rectangle
		bsr	_rectXrect	; (!!! Does not trash a0 or a1 !!!)
		exg	a2,a1		; Swap them back...
		tst.w	d0		; Check result (not this one if FALSE)
		beq	layerop_loop	; Not this one...
*
* Ok, so now split this as needed and recurse
*
* if (r->MinY < clip->MinY)
*
		move.w	cr_MinY(a1),d0	; Get MinY
		cmp.w	ra_MinY(a2),d0	; Check if r->MinY < clip->MinY
		ble.s	1$		; No?  so check next...
		move.l	a1,-(sp)	; Save the current clip
		move.l	(a2),-(sp)	; Save the Rectangle MIN values
		move.l	4(a2),-(sp)	; Save the Rectangle MAX values
		subq.w	#1,d0		; bump the MinY
		move.w	d0,ra_MaxY(a2)	; Set the new MaxY
		bsr	layerop_loop	; Go and do it again
		move.l	(sp)+,4(a2)	; Restore rectangle MAX
		move.l	(sp)+,(a2)	; Restore rectangle MIN
		move.l	(sp)+,a1	; Restore the clip
		move.w	cr_MinY(a1),ra_MinY(a2)	; Set new MinY
1$:
*
* if (r->MaxY > clip->MaxY)
*
		move.w	cr_MaxY(a1),d0	; Get MaxY
		cmp.w	ra_MaxY(a2),d0	; Check if r->MaxY > clip->MaxY
		bge.s	2$		; No?  so check next...
		move.l	a1,-(sp)	; Save the current clip
		move.l	(a2),-(sp)	; Save the Rectangle MIN values
		move.l	4(a2),-(sp)	; Save the Rectangle MAX values
		addq.w	#1,d0		; bump the MaxY
		move.w	d0,ra_MinY(a2)	; Set the new MinY
		bsr	layerop_loop	; Go and do it again
		move.l	(sp)+,4(a2)	; Restore rectangle MAX
		move.l	(sp)+,(a2)	; Restore rectangle MIN
		move.l	(sp)+,a1	; Restore the clip
		move.w	cr_MaxY(a1),ra_MaxY(a2)	; Set new MaxY
2$:
*
* if (r->MinX < clip->MinX)
*
		move.w	cr_MinX(a1),d0	; Get MinX
		cmp.w	ra_MinX(a2),d0	; Check if r->MinX < clip->MinX
		ble.s	3$		; No?  so check next...
		move.l	a1,-(sp)	; Save the current clip
		move.l	(a2),-(sp)	; Save the Rectangle MIN values
		move.l	4(a2),-(sp)	; Save the Rectangle MAX values
		subq.w	#1,d0		; bump the MinX
		move.w	d0,ra_MaxX(a2)	; Set the new MaxX
		bsr	layerop_loop	; Go and do it again
		move.l	(sp)+,4(a2)	; Restore rectangle MAX
		move.l	(sp)+,(a2)	; Restore rectangle MIN
		move.l	(sp)+,a1	; Restore the clip
		move.w	cr_MinX(a1),ra_MinX(a2)	; Set new MinX
3$:
*
* if (r->MaxX > clip->MaxX)
*
		move.w	cr_MaxX(a1),d0	; Get MaxX
		cmp.w	ra_MaxX(a2),d0	; Check if r->MaxX > clip->MaxX
		bge.s	4$		; No?  so check next...
		move.l	a1,-(sp)	; Save the current clip
		move.l	(a2),-(sp)	; Save the Rectangle MIN values
		move.l	4(a2),-(sp)	; Save the Rectangle MAX values
		addq.w	#1,d0		; bump the MaxX
		move.w	d0,ra_MinX(a2)	; Set the new MinX
		bsr	layerop_loop	; Go and do it again
		move.l	(sp)+,4(a2)	; Restore rectangle MAX
		move.l	(sp)+,(a2)	; Restore rectangle MIN
		move.l	(sp)+,a1	; Restore the clip
		move.w	cr_MaxX(a1),ra_MaxX(a2)	; Set new MaxX
4$:
		rts			; Return
*
*******************************************************************************
