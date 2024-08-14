*******************************************************************************
*
*	$Id: muldiv.asm,v 42.0 93/06/16 11:18:25 chrisg Exp $
*
*******************************************************************************
;---------------------------------------------------------------------------

	include	"/gfxbase.i"

	XREF	_LVOSDivMod32
	XREF	_LVOUDivMod32
	XREF	_LVOSMult32
	XREF	_LVOUMult32
	XREF	_LVORawDoFmt

	XDEF	__CXM22			; Unsigned multiply
	XDEF	__CXM33			; Signed multiply
	XDEF	__CXD22			; Unsigned divide
	XDEF	__CXD33			; Signed divide

;---------------------------------------------------------------------------

__CXD33:
	move.l	a0,-(a7)
		move.l	gb_UtilBase(a6),a0	; Get utility base pointer
		jsr	_LVOSDivMod32(a0)	; Do the divide
	move.l	(a7)+,a0
	rts

;---------------------------------------------------------------------------

__CXD22:
	move.l	a0,-(a7)
		move.l	gb_UtilBase(a6),a0	; Get utility base pointer
		jsr	_LVOUDivMod32(a0)	; Do the divide
	move.l	(a7)+,a0
	rts

;---------------------------------------------------------------------------

__CXM33:
	move.l	a0,-(a7)
		move.l	gb_UtilBase(a6),a0	; Get utility base pointer
		jsr	_LVOSMult32(a0)		; Do the multiply
	move.l	(a7)+,a0
	rts

;---------------------------------------------------------------------------

__CXM22:
	move.l	a0,-(a7)
	move.l	gb_UtilBase(a6),a0	; Get utility base pointer
		jsr	_LVOUMult32(a0)		; Do the multply
	move.l	(a7)+,a0
	rts


	end
