
; replacements for standard lc.lib routines using utility.library instead

	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE	"aslbase.i"

	LIST

;---------------------------------------------------------------------------

	XREF	_SysBase
	XREF	_LVOSDivMod32
	XREF	_LVOUDivMod32
	XREF	_LVOSMult32
	XREF	_LVOUMult32
	XREF	_LVORawDoFmt

	XDEF	__CXM22			; Unsigned multiply
	XDEF	__CXM33			; Signed multiply
	XDEF	__CXD22			; Unsigned divide
	XDEF	__CXD33			; Signed divide
	XDEF	_sprintf

;---------------------------------------------------------------------------

__CXD33:	move.l	ASL_SDivMod32(a6),-(sp)
		rts

;---------------------------------------------------------------------------

__CXD22:	move.l	ASL_UDivMod32(a6),-(sp)
		rts

;---------------------------------------------------------------------------

__CXM33:	move.l	ASL_SMult32(a6),-(sp)
		rts

;---------------------------------------------------------------------------

__CXM22:	move.l	ASL_UMult32(a6),-(sp)
		rts

;---------------------------------------------------------------------------

_sprintf:	; ( string, format, {values} )
		movem.l	a2/a3/a4/a6,-(sp)
		move.l  5*4(sp),a3

		move.l	6*4(sp),a0
		lea	7*4(sp),a1
		lea	stuffChar(pc),a2
		move.l	ASL_SysBase(a6),a6
		jsr	_LVORawDoFmt(a6)

		movem.l	(sp)+,a2/a3/a4/a6
		rts

stuffChar:
		move.b	d0,(a3)+
		rts

;---------------------------------------------------------------------------

		END
