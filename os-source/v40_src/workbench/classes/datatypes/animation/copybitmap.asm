; copybitmap.asm
;---------------------------------------------------------------------------

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
	INCLUDE "graphics/gfx.i"

        INCLUDE "classbase.i"

;---------------------------------------------------------------------------

	XREF	_LVOCopyMem

;---------------------------------------------------------------------------
;void __asm CopyBitMap (
;   register __a6 struct Library *cb,
;   register __a2 struct BitMap *bm1, register __a3 struct BitMap *bm2,
;   register __d2 ULONG width);
;---------------------------------------------------------------------------
;   d3 = plane counter
;   d4 = row counter
;   d5 = plane index
;   d6 = bpr1
;   d7 = bpr2
;   a4 = src plane
;   a5 = dst plane
;---------------------------------------------------------------------------
_CopyBitMap::
	movem.l		d3-d7/a4-a6,-(a7)		; save the registers

	move.l		cb_SysBase(a6),a6		; we want SysBase in A6

	moveq.l		#0,d6				; bpr1
	move.w		bm_BytesPerRow(a2),d6

	moveq.l		#0,d7				; bpr2
	move.w		bm_BytesPerRow(a3),d7

	; Step through the bitmap planes
	moveq.l		#0,d3
	move.l		d3,d5
	move.b		bm_Depth(a2),d3
	bra.s		_p2
_p1:
	move.l		bm_Planes(a2,d5.w),a4		; src plane
	move.l		bm_Planes(a3,d5.w),a5		; dst plane
	addq.w		#4,d5

	; Step through the rows
	moveq.l		#0,d4
	move.w		bm_Rows(a2),d4
	bra.s		_r2
_r1:
	; Copy the bitmap data
	move.l		a4,a0				; Source
	move.l		a5,a1				; Destination
	move.l		d2,d0				; Width
	jsr		_LVOCopyMem(a6)			; CopyMem()

	; Increment plane pointers
	add.l		d6,a4
	add.l		d7,a5
_r2:
	dbra		d4,_r1				; next row
_p2:
	dbra		d3,_p1				; next plane

	movem.l		(a7)+,d3-d7/a4-a6		; restore the registers
	rts
;---------------------------------------------------------------------------
