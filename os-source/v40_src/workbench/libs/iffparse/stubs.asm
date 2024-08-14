
; replacements for standard lc.lib routines using utility.library instead

	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE	"iffparsebase.i"

	LIST

;---------------------------------------------------------------------------

	XREF	_LVOSDivMod32
	XREF	_LVOUDivMod32
	XREF	_LVOSMult32
	XREF	_LVOUMult32

	XDEF	__CXM22			; Unsigned multiply
	XDEF	__CXM33			; Signed multiply
	XDEF	__CXD22			; Unsigned divide
	XDEF	__CXD33			; Signed divide

;---------------------------------------------------------------------------

__CXD33:	move.l	ipb_SDivMod32(a6),-(sp)
		rts

;---------------------------------------------------------------------------

__CXD22:	move.l	ipb_UDivMod32(a6),-(sp)
		rts

;---------------------------------------------------------------------------

__CXM33:	move.l	ipb_SMult32(a6),-(sp)
		rts

;---------------------------------------------------------------------------

__CXM22:	move.l	ipb_UMult32(a6),-(sp)
		rts

;---------------------------------------------------------------------------

		END
