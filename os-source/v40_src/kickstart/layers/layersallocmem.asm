*******************************************************************************
*
*	$Id: layersallocmem.asm,v 39.6 92/06/05 11:46:00 mks Exp $
*
*******************************************************************************
*
	include 'exec/types.i'
	include 'exec/nodes.i'
	include 'exec/lists.i'
	include	'exec/memory.i'
	include 'graphics/clip.i'
	include	'graphics/layers.i'
	include	'layersbase.i'
*
		xref	abort
		xref	_remember_to_free
*
*******************************************************************************
*
* APTR LayersAllocMem(ULONG numbytes,ULONG flags,struct LayerInfo *li)
* d0                  D0             D1          D2
*
_LayersAllocMem:	xdef	_LayersAllocMem
LayersAllocMem:	move.l	d0,-(sp)	; Save this...
		move.l	a6,-(sp)	; Save a6...
		move.l	lb_ExecBase(a6),a6	; Get execbase
		CALLSYS	AllocMem	; Allocate the memory...
		move.l	(sp)+,a6	; Get a6 back...
		tst.l	d2		; Do we have a LayerInfo?
		beq.s	lam_done	; If not, we are done...
		move.l	d2,a0		; Get a0 set up...
		tst.l	d0		; Is is ok?
		beq	abort		; Abort due to error? (never returns)
		move.l	d0,-(sp)	; save pointer onto the stack...
		move.l	d0,a1		; Move to address reg.
		move.l	4(sp),d0	; Get size of alloc
		bsr	_remember_to_free
		tst.l	d0		; Check result...
		bne.s	lam_good	; If not bad, exit
*
* Now release the extra memory and abort...
*
		move.l	(sp)+,a1	; Get allocation back
		move.l	(sp)+,d0	; Get size
		move.l	a6,-(sp)	; Save a6...
		move.l	lb_ExecBase(a6),a6	; Get execbase
		CALLSYS	FreeMem		; Free the memory
		move.l	(sp)+,a6	; Restore layersbase
		move.l	d2,a0		; Get set up...
		bra	abort		; Abort this one! (Never returns)
*
* All cool or allocation failed with no LayerInfo
*
lam_good:	move.l	(sp)+,d0	; Get allocation back
lam_done:	addq.l	#4,sp		; Drop the stack...
		rts			; And we are done...
*
*******************************************************************************
*
* AllocCR() - Allocate a ClipRect for the layerinfo given
*
* a0 = LayerInfo
*
_AllocCR:	xdef	_AllocCR
AllocCR:	move.l	d2,-(sp)			; Save d2...
		move.l	a0,d2				; Get layer info...
		move.l	#MEMF_CLEAR!MEMF_PUBLIC,d1	; Memory type
		move.l	#cr_SIZEOF,d0			; Memory size
		bsr	LayersAllocMem			; Allocate...
		move.l	(sp)+,d2			; Restore
		rts					; Exit...

*
*******************************************************************************
*
* struct ClipRect *newAllocCR(struct Layer *l)
* d0                          a0
*
_newAllocCR:	xdef	_newAllocCR
newAllocCR:	move.l	lr_LayerInfo(a0),a0	; Get layer info...
		lea	li_FreeClipRects(a0),a1	; Point at first pointer...
		move.l	cr_Next(a1),a0		; Get first one...
		move.l	a0,d0			; Check it
		beq	AllocCR			; If none, we skip...
		move.l	cr_Next(a0),cr_Next(a1)	; Unlink it...
		move.l	a1,cr_Prev(a0)		; And clean up prev...
		moveq.l	#0,d1			; Get a null...
		move.l	d1,cr_lobs(a0)		; Clear LOBS
		move.l	d1,cr_BitMap(a0)	; Clear bitmap
		move.l	d1,cr_Flags(a0)		; Clear the flags
		rts
*
*******************************************************************************
*
* newDeleteCR(struct Layer *l,struct ClipRect *cr)
*             a0              a1
*
_newDeleteCR:	xdef	_newDeleteCR
newDeleteCR:	move.l	a0,d1			; Temp storage...
		lea	lr_ClipRect(a0),a0	; Point at first pointer
newDel_loop:	cmp.l	cr_Next(a0),a1		; Check for match...
		beq.s	newDel_found		; Found it
		move.l	cr_Next(a0),a0		; Get next one...
		bra.s	newDel_loop		; Loop until found
*
newDel_found:	move.l	cr_Next(a1),cr_Next(a0)	; Un-link
		move.l	d1,a0			; Get back layer...
		move.l	lr_LayerInfo(a0),a0	; Get layer info...
		lea	li_FreeClipRects(a0),a0	; Point at list...
		move.l	cr_Next(a0),d0		; Get old NEXT pointer
		beq.s	newDel_noNext		; If none, skip...
		exg.l	d0,a0			; Get into address reg...
		move.l	a1,cr_Prev(a0)		; Store prev pointer
		exg.l	d0,a0			; And restore a0/d0
newDel_noNext:	move.l	d0,cr_Next(a1)		; Put in NEXT pointer
		move.l	a1,cr_Next(a0)		; Store on list...
		move.l	a0,cr_Prev(a1)		; Back-Link to list
		rts				; And we are done!
*
*******************************************************************************
*
* This is the kludge design for the Store_CR and UnStore_CR functions
*
 STRUCTURE	KludgeClipRect,0
	LONG	kcr_trash1
	LONG	kcr_trash2
	LONG	kcr_trash3
	LONG	kcr_trash4
	STRUCT	kcr_Bounds1,8
	STRUCT	kcr_Bounds2,8
	LONG	kcr_Next
	LONG    kcr_trash5
	LABEL	kcr_SIZEOF
*
*******************************************************************************
*
* Store_CR(struct Layer *l,struct ClipRect *cr)
*          a0              a1
*
_Store_CR:	xdef	_Store_CR
		move.l	lr__cliprects(a0),kcr_Next(a1)	; Link to layer
		move.l	a1,lr__cliprects(a0)		; this cliprect
		lea	kcr_Bounds1(a1),a0		; Now copy the
		lea	kcr_Bounds2(a1),a1		; bounds
		move.l	(a0)+,(a1)+			; MinX/Y
		move.l	(a0)+,(a1)+			; MaxX/Y
		rts
*
*******************************************************************************
*
* struct ClipRect *UnStore_CR(struct Layer *l)
* d0                          a0
*
_UnStore_CR:	xdef	_UnStore_CR
		move.l	lr__cliprects(a0),d0	; Get cliprect
		beq.s	UnStore_Done		; If NULL, no go...
		move.l	d0,a1			; Get into address
		move.l	kcr_Next(a1),lr__cliprects(a0)	; Remove from layer
		move.l	cr_Prev(a1),a0		; Get back pointer
		move.l	cr_Next(a1),d1		; Get next pointer
		move.l	d1,cr_Next(a0)		; Unlink...
		beq.s	UnStore_NoNext		; Check if we have a next
		exg.l	d1,a0			; Swap the two
		move.l	d1,cr_Prev(a0)		; Unlink prev...
UnStore_NoNext:	lea	kcr_Bounds1(a1),a0	; Now copy the
		lea	kcr_Bounds2(a1),a1	; bounds
		move.l	(a1)+,(a0)+		; MinX/Y
		move.l	(a1)+,(a0)+		; MaxX/Y
UnStore_Done:	rts
*
*******************************************************************************
*
		end
