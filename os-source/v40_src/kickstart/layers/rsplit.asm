*******************************************************************************
*
*	$Id: rsplit.asm,v 39.0 91/11/11 13:08:14 mks Exp $
*
*******************************************************************************
*
	include 'exec/types.i'
	include 'exec/lists.i'
	include 'graphics/clip.i'
	include	'graphics/layers.i'
	include	'layersbase.i'
*
		xref	_rectXrect
		xref	_newAllocCR
		xref	_newDeleteCR
*
*******************************************************************************
*
* Split up the layer into cliprects for the on-screen stuff...
*
* r_split(struct Layer *l,struct Layer *p,struct ClipRect *cr)
*         a2              a0              a1
*
* Layer L is the input layer - Comes into this function in A2
* Layer P is the current layer - Comes into function in A0
* ClipRect cr is the current clip - Comes into function in A1
*
_r_split:	xdef	_r_split
		move.l	a4,-(sp)	; Give me some room...
		move.l	a0,a4		; Ok, move incoming layer to a4
		bsr.s	r_split		; Do the assembly version
		move.l	(sp)+,a4	; Restore
		rts
*
* Used to save some bytes...  We call this which then
* drops into r_split.  It returns with flags set.  (Z flag)
*
r_split_go:	move.l	lr_front(a4),a4	; Get next layer
*
* This is the part that actually does the hard work
*
* At this point:	a2 = Layer building cliprects in
*			a1 = moved to a3; New cliprect to play with
*			a4 = Current layer we are checking with
*
r_split:	move.l	a3,-(sp)	; Save a3
		subq.l	#4,sp		; Space for a4
		move.l	a1,a3		; Put CR into a3...
r_split_again:	move.l	a4,(sp)		; Check if we have a layer (and save)
		beq	r_split_good	; At the end of P...
		lea	lr_MinX(a4),a0	; Get bounds 1... (layer)
		lea	cr_MinX(a3),a1	; Get bounds 2... (cliprect)
		bsr	_rectXrect	; Call the routine...
		tst.l	d0		; Check result
		bne.s	r_split_1	; If a layer to use, we start the split
		move.l	lr_front(a4),a4	; Get next layer
		bra.s	r_split_again	; Check one more time...
*
* Ok, so we to test this cliprect...
*
* if (p->bounds.MaxY < cr->bounds.MaxY)
*
r_split_1:	move.w	lr_MaxY(a4),d0	; Get MaxY of the current layer
		cmp.w	cr_MaxY(a3),d0	; Check against cliprect
		bge.s	r_split_2
		move.l	a2,a0		; Get layer for newAllocCR
		bsr	_newAllocCR	; Allocate the cliprect
		move.l	d0,a1		; Get cliprect
		tst.l	d0		; Check cliprect...
		beq	r_split_bad	; No alloc, so exit...
*
		lea	lr_ClipRect(a2),a0	; Point at cliprect list
		move.l	(a0),(a1)		; Set up next pointer
		move.l	a1,(a0)			; Link in new cliprect
*
		move.l	cr_MinX(a3),cr_MinX(a1)	; Copy bounds
		move.l	cr_MaxX(a3),cr_MaxX(a1)	; ... 4 words/2 long words
		move.w	lr_MaxY(a4),d0		; Get layer MaxY
		move.w	d0,cr_MaxY(a3)		; Adjust our cliprect...
		addq.w	#1,d0			; MaxY+1
		move.w	d0,cr_MinY(a1)		; New MinY of cliprect
		bsr	r_split_go		; Do it  (Sets flags)
		beq	r_split_bad		; If NULL, no good!
		move.l	(sp),a4			; Get a4 back...
*
* if (p->bounds.MinY > cr->bounds.MinY)
*
r_split_2:	move.w	lr_MinY(a4),d0	; Get MinY of layer
		cmp.w	cr_MinY(a3),d0	; Check against cliprect
		ble.s	r_split_3
		move.l	a2,a0		; Get layer for newAllocCR
		bsr	_newAllocCR	; Allocate the cliprect
		move.l	d0,a1		; Get cliprect
		tst.l	d0		; Check cliprect...
		beq	r_split_bad	; No alloc, so exit...
*
		lea	lr_ClipRect(a2),a0	; Point at cliprect list
		move.l	(a0),(a1)		; Set up next pointer
		move.l	a1,(a0)			; Link in new cliprect
*
		move.l	cr_MinX(a3),cr_MinX(a1)	; Copy bounds
		move.l	cr_MaxX(a3),cr_MaxX(a1)	; ... 4 words/2 long words
		move.w	lr_MinY(a4),d0		; Get layer MinY
		move.w	d0,cr_MinY(a3)		; Adjust our cliprect...
		subq.w	#1,d0			; MinY-1
		move.w	d0,cr_MaxY(a1)		; New MaxY of cliprect
		bsr	r_split_go		; Do it  (Sets flags)
		beq	r_split_bad		; If NULL, no good!
		move.l	(sp),a4			; Get a4 back...
*
* if (p->bounds.MaxX < cr->bounds.MaxX)
*
r_split_3:	move.w	lr_MaxX(a4),d0	; Get MaxX of layer
		cmp.w	cr_MaxX(a3),d0	; Check against cliprect
		bge.s	r_split_4
		move.l	a2,a0		; Get layer for newAllocCR
		bsr	_newAllocCR	; Allocate the cliprect
		move.l	d0,a1		; Get cliprect
		tst.l	d0		; Check cliprect...
		beq.s	r_split_bad	; No alloc, so exit...
*
		lea	lr_ClipRect(a2),a0	; Point at cliprect list
		move.l	(a0),(a1)		; Set up next pointer
		move.l	a1,(a0)			; Link in new cliprect
*
		move.l	cr_MinX(a3),cr_MinX(a1)	; Copy bounds
		move.l	cr_MaxX(a3),cr_MaxX(a1)	; ... 4 words/2 long words
		move.w	lr_MaxX(a4),d0		; Get layer MaxX
		move.w	d0,cr_MaxX(a3)		; Adjust our cliprect...
		addq.w	#1,d0			; MaxX+1
		move.w	d0,cr_MinX(a1)		; New MinX of cliprect
		bsr	r_split_go		; Do it  (Sets flags)
		beq.s	r_split_bad		; If NULL, no good!
		move.l	(sp),a4			; Get a4 back...
*
* if (p->bounds.MinX > cr->bounds.MinX)
*
r_split_4:	move.w	lr_MinX(a4),d0	; Get MinX of layer...
		cmp.w	cr_MinX(a3),d0	; Check against cliprect
		ble.s	r_split_end	; If not, r_split ends...
		move.l	a2,a0		; Get layer for newAllocCR
		bsr	_newAllocCR	; Allocate the cliprect
		move.l	d0,a1		; Get cliprect
		tst.l	d0		; Check cliprect...
		beq.s	r_split_bad	; No alloc, so exit...
*
		lea	lr_ClipRect(a2),a0	; Point at cliprect list
		move.l	(a0),(a1)		; Set up next pointer
		move.l	a1,(a0)			; Link in new cliprect
*
		move.l	cr_MinX(a3),cr_MinX(a1)	; Copy bounds
		move.l	cr_MaxX(a3),cr_MaxX(a1)	; ... 4 words/2 long words
		move.w	lr_MinX(a4),d0		; Get layer MinX
		move.w	d0,cr_MinX(a3)		; Adjust our cliprect...
		subq.w	#1,d0			; MinX-1
		move.w	d0,cr_MaxX(a1)		; New MaxX of cliprect
		bsr	r_split_go		; Do it  (Sets flags)
		beq.s	r_split_bad		; If NULL, no good!
; Not needed	move.l	(sp),a4			; Get a4 back...
*
* Now, free the cliprect since it is completely within the layer a4...
*
r_split_end:	move.l	a3,a1		; Get cliprect
		move.l	a2,a0		; Get layer
		bsr	_newDeleteCR	; Free this cliprect...
r_split_good:	addq.l	#4,sp		; Undo space for a4
		move.l	(sp)+,a3	; Restore a3...
		moveq.l	#1,d0		; Set OK flag...
		rts
r_split_bad:	addq.l	#4,sp		; Undo space for a4
		move.l	(sp)+,a3	; Restore a3...
		moveq.l	#0,d0		; False return
		rts
*
*******************************************************************************
*
		end
