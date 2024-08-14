*************************************************************************
*
* $Id: drawbox.asm,v 38.3 92/08/01 11:56:31 mks Exp $
*
* $Log:	drawbox.asm,v $
* Revision 38.3  92/08/01  11:56:31  mks
* Ants once again complement in all screen colours
* 
* Revision 38.2  92/06/25  07:59:46  mks
* Now does the RP masking to make box only 2-planes of complement.
*
* Revision 38.1  91/06/24  11:34:54  mks
* Initial V38 tree checkin - Log file stripped
*
*************************************************************************

*************************************************************************
* This file contains the DrawBox routine and the animate draw box.
*************************************************************************

		SECTION	code

		INCLUDE 'exec/types.i'
		INCLUDE	'exec/execbase.i'
		INCLUDE	'graphics/gfx.i'
		INCLUDE	'graphics/rastport.i'
		INCLUDE	'intuition/screens.i'
		INCLUDE	'workbench.i'
		INCLUDE	'workbenchbase.i'

*******************************************************************************
*
* AnimateDragSelectBox(msg)
*
		XDEF	_AnimateDragSelectBox
		XREF	_AbortLayerDemon
		XREF	_PostLayerDemon
*
_AnimateDragSelectBox:
*
* Stop the layer daemon
*
		jsr	_AbortLayerDemon	; Blow away the layer daemon
*
* Initialize
*
		movem.l	a2/a6/d2/d3/d4/d5/d6,-(sp)	; Save
		move.l	wb_Screen(a6),a2	; Get screen...
		lea	sc_RastPort(a2),a2	; Get offset to RastPort...
		move.w	rp_LinePtrn(a2),d6	; Get line pattern...
*
* Build "difference" pattern
*
		move.w	d6,d1			; Save old pattern in d1
		rol.w	#4,d6			; Rotate pattern
		eor.w	d6,d1			; Get a difference pattern
*
* Set the "difference" pattern
*
		move.w	d1,rp_LinePtrn(a2)	; Save pattern
		bset.b	#FRST_DOTn,rp_Flags+1(a2)
		move.b	#15,rp_linpatcnt(a2)
*
* Draw the "difference"
* Note that AsmDrawBox trashes a6 and d2/d4 and d3/d5
*
		bsr.s	AsmDrawBox		; Draw this info
*
* Note that a6 is now trash...
*
**
*
* Set the real pattern back.
*
		move.w	d6,rp_LinePtrn(a2)	; Save pattern
		bset.b	#FRST_DOTn,rp_Flags+1(a2)
		move.b	#15,rp_linpatcnt(a2)
*
* Cleanup...
*
		movem.l	(sp)+,a2/a6/d2/d3/d4/d5/d6	; Restore...
*
* Start up the layer daemon again...
*
		jmp	_PostLayerDemon		; Let this routine do our RTS
*
*******************************************************************************
*
* CheckSortRect routine that checks if the rect is of the minimum size
* It also sorts the rect.
*
* void CheckSortRect(rect *)
*
		XDEF	_CheckSortRect
_CheckSortRect:	movem.l	d2/d3/d4/d5,-(sp)	; Save these...
		movem.l	20(sp),a0		; Rectangle pointer
		movem.w	(a0),d2/d3/d4/d5	; Get old values from rectangle
		bsr.s	AsmCheckDxDy		; AsmCheckDxDy
		movem.w	d2/d3/d4/d5,(a0)	; Restore sorted version
		movem.l	(sp)+,d2/d3/d4/d5	; Restore...
		rts
*
*******************************************************************************
*
* DrawBox routine that does not draw the corners twice.
*
* void DrawBox(void)
*
		XREF	_LVOMove
		XREF	_LVODraw
		XDEF	_DrawBox
_DrawBox:	movem.l	d2/d3/d4/d5/a2/a6,-(sp)	; Save these...
		move.l	wb_Screen(a6),a2	; Get screen...
		lea	sc_RastPort(a2),a2	; Get offset to RastPort...
		bsr.s	AsmDrawBox		; AsmDrawBox
		movem.l	(sp)+,d2/d3/d4/d5/a2/a6	; Restore...
		rts
*
*******************************************************************************
*
* Sort the X/Y values and check if the box is of the minimum size...
*
* Result is d0==TRUE if the box is at least large enough
*
AsmCheckDxDy:
*
* Sort the X/Y values...
*
		cmp.w	d2,d4		; Is x2 smaller than x1
		bge.s	NoSwap1
		exg	d2,d4		; Make sure x2 is not smaller than x1
NoSwap1:
*
		cmp.w	d3,d5		; Is y2 smaller than y1
		bge.s	NoSwap2
		exg	d3,d5		; Make sure y2 is not smaller than y1
NoSwap2:
*
* Now check if we need to draw this one...
*
		moveq.l	#0,d0		; Assume false...
		move.w	d4,d1		; Get x2
		sub.w	d2,d1		; d1= x2-x1
		add.w	d5,d1		; d1= x2-x1 + y2
		sub.w	d3,d1		; d1= x2-x1 + y2-y1
		sub.w	#10,d1		; Check if result is more than 10
		sgt	d0		; Set d0==TRUE if more than 10
		rts
*
*******************************************************************************
*
* Set up a6 to point to GfxBase		(Routine trashes d2/d3/d4/d5/a6)
*
		XREF	_LVOMove
		XREF	_LVODraw
AsmDrawBox:
*
* Get x1/y1/x2/y2
*
		move.w	wb_XOffset(a6),d2	; x1
		move.w	wb_YOffset(a6),d3	; y1
		move.w	wb_LastX(a6),d4		; x2
		move.w	wb_LastY(a6),d5		; y2
*
		movea.l	wb_GfxBase(a6),a6	; Get GfxBase
*
* Sort and check the x/y values...
*
		bsr.s	AsmCheckDxDy	; Call the routine...
		tst.b	d0		; Check if not a box...
		beq.s	NoBoxDraw	; No box to draw...
*
* Now, check for what we have to do...
*
		move.w	d5,d1		; Get y2
		sub.w	d3,d1		; Subtract y1
		subq.w	#1,d1		; Subtract 1 more
		ble.s	NoYLine		; If dY not more than 1, skip...
*
* We have a dY>1 so do the y direction lines
*
		move.w	d3,d1		; Get y1
		move.w	d2,d0		; Get x1
		movea.l	a2,a1		; Get RP
		jsr	_LVOMove(a6)	; Move to X1,Y1
		move.w	d5,d1		; Get y2
		subq.w	#1,d1		; y2-1
		move.w	d2,d0		; Get x1
		movea.l	a2,a1		; Get RP
		jsr	_LVODraw(a6)	; Draw from (X1,Y1) to (X1,Y2-1)
*
		cmp.w	d2,d4		; Check if any dX
		beq.s	NoYLine		; If none, skip
*
* We have a dX so do the other Y line...
*
		move.w	d5,d1		; Get y2
		move.w	d4,d0		; Get x2
		move.l	a2,a1		; Get RP
		jsr	_LVOMove(a6)	; Move to X2,Y2
		move.w	d3,d1		; Get y1
		addq.w	#1,d1		; y1+1
		move.w	d4,d0		; Get x2
		movea.l	a2,a1		; Get RP
		jsr	_LVODraw(a6)	; Draw from (X2,Y2) to (X2,Y1+1)
NoYLine:
*
* Now check for the X lines...
*
		move.w	d4,d0		; Get x2
		sub.w	d2,d0		; Subtract x1
		subq.w	#1,d0		; Subtract 1 more
		ble.s	NoXLine		; If dX not more than 1, skip...
*
* We have a dX>1 so do the x direction lines...
*
		move.w	d3,d1		; Get y1
		move.w	d4,d0		; Get x2
		movea.l	a2,a1		; Get RP
		jsr	_LVOMove(a6)	; Move to (X2,Y1)
		move.w	d3,d1		; Get y1
		move.w	d2,d0		; Get x1
		addq.w	#1,d0		; x1+1
		movea.l	a2,a1		; Get RP
		jsr	_LVODraw(a6)	; Draw from (X2,Y1) to (X1+1,Y1)
*
		cmp.w	d3,d5		; Check if any dY
		beq.s	NoXLine		; If none, skip
*
* We have a dY so do the other X line...
*
		move.w	d5,d1		; Get y2
		move.w	d2,d0		; Get x1
		move.l	a2,a1		; Get RP
		jsr	_LVOMove(a6)	; Move to X1,Y2
		move.w	d5,d1		; Get y2
		move.w	d4,d0		; Get x2
		subq.w	#1,d0		; x2-1
		movea.l	a2,a1		; Get RP
		jsr	_LVODraw(a6)	; Draw from (X1,Y2) to (X2-1,Y2)
NoXLine:
NoBoxDraw:
		rts			; Exit now...
*
*******************************************************************************
