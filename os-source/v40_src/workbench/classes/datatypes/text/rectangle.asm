*************************************************************************
*
* $Id: rectangle.asm,v 39.0 92/06/22 04:10:36 davidj Exp $
*
* $Log:	rectangle.asm,v $
* Revision 39.0  92/06/22  04:10:36  davidj
* initial rcs login
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

; VOID ASM CheckSortRect (REG (a0) struct Rectangle *);

*
*******************************************************************************
		XDEF	_CheckSortRect
_CheckSortRect:
		movem.w	(a0),d2/d3/d4/d5	; Get old values from rectangle
		bsr.s	AsmCheckDxDy		; AsmCheckDxDy
		movem.w	d2/d3/d4/d5,(a0)	; Restore sorted version
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
