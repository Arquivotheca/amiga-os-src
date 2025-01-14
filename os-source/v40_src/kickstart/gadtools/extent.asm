	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE "graphics/gfx.i"
	INCLUDE	"gtinternal.i"

	LIST

;---------------------------------------------------------------------------

	XREF	_LVOEraseRect
	XREF	_LVORectFill

;---------------------------------------------------------------------------

	XDEF	_EraseOldExtent
	XDEF	_FillOldExtent

;---------------------------------------------------------------------------

; Fills any part of oldExtent that is not covered by newExtent using BPen
; VOID FillOldExtent(register __a0 struct RastPort *rp,
;                    register __a1 struct Rectangle *oldExtent,
;                    register __a2 struct Rectangle *newExtent)

_FillOldExtent:
	move.l	#_LVORectFill,d0
	bra.s	ZapOldExtent

; Clears any part of oldExtent that is not covered by newExtent
; VOID EraseOldExtent(register __a0 struct RastPort *rp,
;                     register __a1 struct Rectangle *oldExtent,
;                     register __a2 struct Rectangle *newExtent)

_EraseOldExtent:
	move.l	#_LVOEraseRect,d0

ZapOldExtent:
	movem.l d2-d3/a3-a6,-(sp)
	move.l  gtb_GfxBase(a6),a6 	; load GfxBase
	lea	0(a6,d0.l),a4		; load function address
	move.l	a1,a3			; a3 has oldExtent
	move.l	a0,a5		   	; a5 has rp
0$:
	move.w	ra_MinX(a2),d2
	move.w	ra_MinX(a3),d0
	cmp.w	d2,d0
	bge.s	1$
	move.w  ra_MinY(a3),d1
	move.w  ra_MaxY(a3),d3
	ext.l   d0
	ext.l   d1
	ext.l   d2
	ext.l   d3
	subq.l  #1,d2
	move.l  a5,a1
	jsr     (a4)
1$:
	move.w	ra_MaxX(a2),d0
	move.w	ra_MaxX(a3),d2
	cmp.w	d0,d2
	ble.s	2$
	move.w	ra_MinY(a3),d1
	move.w	ra_MaxY(a3),d3
	ext.l	d0
	ext.l	d1
	ext.l	d2
	ext.l	d3
	addq.l	#1,d0
	move.l	a5,a1
	jsr	(a4)
2$:
	move.w	ra_MaxY(a2),d1
	move.w	ra_MaxY(a3),d3
	cmp.w	d1,d3
	ble.s	3$
	move.w	ra_MinX(a3),d0
	move.w	ra_MaxX(a3),d2
	ext.l	d0
	ext.l	d1
	ext.l	d2
	ext.l	d3
	addq.l	#1,d1
	move.l	a5,a1
	jsr	(a4)
3$:
	move.w	ra_MinY(a2),d3
	move.w	ra_MinY(a3),d1
	cmp.w	d3,d1
	bge.s	4$
	move.w	ra_MinX(a3),d0
	move.w	ra_MaxX(a3),d2
	ext.l	d0
	ext.l	d1
	ext.l	d2
	ext.l	d3
	subq.l	#1,d3
	move.l	a5,a1
	jsr	(a4)
4$:
	movem.l	(sp)+,d2-d3/a3-a6
	rts

;  Equivalent C code
; VOID EraseOldExtent(struct RastPort *rp,
;                     struct Rectangle *oldExtent,
;                     struct Rectangle *newExtent)
; {
;     if (oldExtent->MinX < newExtent->MinX)
;         EraseRect(rp,oldExtent->MinX,
;                      oldExtent->MinY,
;                      newExtent->MinX-1,
;                      oldExtent->MaxY);
;
;     if (oldExtent->MaxX > newExtent->MaxX)
;         EraseRect(rp,newExtent->MaxX+1,
;                      oldExtent->MinY,
;                      oldExtent->MaxX,
;                      oldExtent->MaxY);
;
;     if (oldExtent->MaxY > newExtent->MaxY)
;         EraseRect(rp,oldExtent->MinX,
;                      newExtent->MaxY+1,
;                      oldExtent->MaxX,
;                      oldExtent->MaxY);
;
;     if (oldExtent->MinY < newExtent->MinY)
;         EraseRect(rp,oldExtent->MinX,
;                      oldExtent->MinY,
;                      oldExtent->MaxX,
;                      newExtent->MinY-1);
; }

;---------------------------------------------------------------------------

	END
