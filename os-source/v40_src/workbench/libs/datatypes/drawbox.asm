*************************************************************************
*
* $Id: drawbox.asm,v 39.2 92/06/29 23:36:36 davidj Exp $
*
* $Log:	drawbox.asm,v $
* Revision 39.2  92/06/29  23:36:36  davidj
* put in rp_Mask like Workbench
* 
* Revision 39.1  92/06/29  00:46:45  davidj
* no change
*
* Revision 39.0  92/06/01  12:56:25  davidj
* initial RCS
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
		INCLUDE "datatypesbase.i"

; VOID ASM DrawBox (REG (a6) struct DataTypesLib * dtl, REG(a2) struct RastPort *rp, REG(d2) WORD Xoff, REG(d3) WORD Yoff, REG(d4) WORD lastX, REG(d5) WORD lastY);
; VOID ASM AnimateDragSelectBox (REG (a6) struct DataTypesLib * dtl, REG(a2) struct RastPort *rp, REG(d2) WORD Xoff, REG(d3) WORD Yoff, REG(d4) WORD lastX, REG(d5) WORD lastY);
; VOID ASM CheckSortRect (REG (a0) struct Rectangle *);

*
*******************************************************************************
		XDEF	_CheckSortRect
_CheckSortRect:
		movem.w	(a0),d2/d3/d4/d5	; Get old values from rectangle
		bsr.s	AsmCheckDxDy		; AsmCheckDxDy
		movem.w	d2/d3/d4/d5,(a0)	; Restore sorted version
		rts

*******************************************************************************
		XDEF	_AnimateDragSelectBox
_AnimateDragSelectBox:

*
* Initialize
*
		movem.l	a2/a6/d2/d3/d4/d5/d6,-(sp)	; Save
		move.w	rp_LinePtrn(a2),d6		; Get line pattern...
*
* Build "difference" pattern
*
		move.w	d6,d1				; Save old pattern in d1
		rol.w	#4,d6				; Rotate pattern
		eor.w	d6,d1				; Get a difference pattern
*
* Set the "difference" pattern
*
		move.w	d1,rp_LinePtrn(a2)		; Save pattern
		bset.b	#FRST_DOTn,rp_Flags+1(a2)
		move.b	#15,rp_linpatcnt(a2)

*
* Draw the "difference"
* Note that _DrawBox trashes a6 and d2/d4 and d3/d5
*
		bsr.s	_DrawBox			; Draw this info

* Note that a6 is now trash...

*
* Set the real pattern back.
*
		move.w	d6,rp_LinePtrn(a2)		; Save pattern
		bset.b	#FRST_DOTn,rp_Flags+1(a2)
		move.b	#15,rp_linpatcnt(a2)
*
* Cleanup...
*
		movem.l	(sp)+,a2/a6/d2/d3/d4/d5/d6	; Restore...
		rts

*
*******************************************************************************
		XDEF	_DrawBox

_DrawBox:
		movem.l	d2/d3/d4/d5/a2/a6,-(sp)		; Save these...
		move.l	dtl_GfxBase(a6),a6
		bsr.s	AsmDrawBox			; AsmDrawBox
		movem.l	(sp)+,d2/d3/d4/d5/a2/a6		; Restore...
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
		cmp.w	d2,d4			; Is x2 smaller than x1
		bge.s	NoSwap1
		exg	d2,d4			; Make sure x2 is not smaller than x1
NoSwap1:
*
		cmp.w	d3,d5			; Is y2 smaller than y1
		bge.s	NoSwap2
		exg	d3,d5			; Make sure y2 is not smaller than y1
NoSwap2:
*
* Now check if we need to draw this one...
*
		moveq.l	#0,d0			; Assume false...
		move.w	d4,d1			; Get x2
		sub.w	d2,d1			; d1= x2-x1
		add.w	d5,d1			; d1= x2-x1 + y2
		sub.w	d3,d1			; d1= x2-x1 + y2-y1
		sub.w	#10,d1			; Check if result is more than 10
		sgt	d0			; Set d0==TRUE if more than 10
		rts
*
*******************************************************************************
*
* Set up a6 to point to GfxBase		(Routine trashes d2/d3/d4/d5/a6)
*
		XREF	_LVOMove
		XREF	_LVODraw

AsmDrawBox:
		move.b	#3,rp_Mask(a2)		; Set mask to 2 planes...

* Sort and check the x/y values...
*
		bsr.s	AsmCheckDxDy		; Call the routine...
		tst.b	d0			; Check if not a box...
		beq.s	NoBoxDraw		; No box to draw...
*
* Now, check for what we have to do...
*
		move.w	d5,d1			; Get y2
		sub.w	d3,d1			; Subtract y1
		subq.w	#1,d1			; Subtract 1 more
		ble.s	NoYLine			; If dY not more than 1, skip...
*
* We have a dY>1 so do the y direction lines
*
		move.w	d3,d1			; Get y1
		move.w	d2,d0			; Get x1
		movea.l	a2,a1			; Get RP
		jsr	_LVOMove(a6)		; Move to X1,Y1
		move.w	d5,d1			; Get y2
		subq.w	#1,d1			; y2-1
		move.w	d2,d0			; Get x1
		movea.l	a2,a1			; Get RP
		jsr	_LVODraw(a6)		; Draw from (X1,Y1) to (X1,Y2-1)
*
		cmp.w	d2,d4			; Check if any dX
		beq.s	NoYLine			; If none, skip
*
* We have a dX so do the other Y line...
*
		move.w	d5,d1			; Get y2
		move.w	d4,d0			; Get x2
		move.l	a2,a1			; Get RP
		jsr	_LVOMove(a6)		; Move to X2,Y2
		move.w	d3,d1			; Get y1
		addq.w	#1,d1			; y1+1
		move.w	d4,d0			; Get x2
		movea.l	a2,a1			; Get RP
		jsr	_LVODraw(a6)		; Draw from (X2,Y2) to (X2,Y1+1)
NoYLine:
*
* Now check for the X lines...
*
		move.w	d4,d0			; Get x2
		sub.w	d2,d0			; Subtract x1
		subq.w	#1,d0			; Subtract 1 more
		ble.s	NoXLine			; If dX not more than 1, skip...
*
* We have a dX>1 so do the x direction lines...
*
		move.w	d3,d1			; Get y1
		move.w	d4,d0			; Get x2
		movea.l	a2,a1			; Get RP
		jsr	_LVOMove(a6)		; Move to (X2,Y1)
		move.w	d3,d1			; Get y1
		move.w	d2,d0			; Get x1
		addq.w	#1,d0			; x1+1
		movea.l	a2,a1			; Get RP
		jsr	_LVODraw(a6)		; Draw from (X2,Y1) to (X1+1,Y1)
*
		cmp.w	d3,d5			; Check if any dY
		beq.s	NoXLine			; If none, skip
*
* We have a dY so do the other X line...
*
		move.w	d5,d1			; Get y2
		move.w	d2,d0			; Get x1
		move.l	a2,a1			; Get RP
		jsr	_LVOMove(a6)		; Move to X1,Y2
		move.w	d5,d1			; Get y2
		move.w	d4,d0			; Get x2
		subq.w	#1,d0			; x2-1
		movea.l	a2,a1			; Get RP
		jsr	_LVODraw(a6)		; Draw from (X1,Y2) to (X2-1,Y2)
NoXLine:
NoBoxDraw:
		move.b	#-1,rp_Mask(a2)		; Restore mask to all planes...
		rts				; Exit now...
*
*******************************************************************************
