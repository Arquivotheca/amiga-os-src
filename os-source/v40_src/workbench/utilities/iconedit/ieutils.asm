
; replacements for standard lc.lib routines using utility.library instead


		XREF	_UtilityBase
		XREF	_SysBase
		XREF	_LVORawDoFmt
		XREF	_LVOSDivMod32
		XREF	_LVOUDivMod32
		XREF	_LVOSMult32
		XREF	_LVOUMult32

		XDEF	__CXM22			; Unsigned multiply
		XDEF	__CXM33			; Signed multiply
		XDEF	__CXD22			; Unsigned divide
		XDEF	__CXD33			; Signed divide
		XDEF	_sprintf

*******************************************************************************

__CXD33:	move.l	a6,-(sp)		; Save your base pointer
		move.l	_UtilityBase,a6		; Get utility base pointer
		jsr	_LVOSDivMod32(a6)	; Do the divide
		move.l	(sp)+,a6		; Restore base register
		rts

*******************************************************************************

__CXD22:	move.l	a6,-(sp)		; Save your base pointer
		move.l	_UtilityBase,a6		; Get utility base pointer
		jsr	_LVOUDivMod32(a6)	; Do the divide
		move.l	(sp)+,a6		; Restore base register
		rts

*******************************************************************************

__CXM33:	move.l	a6,-(sp)		; Save your base pointer
		move.l	_UtilityBase,a6		; Get utility base pointer
		jsr	_LVOSMult32(a6)		; Do the multiply
		move.l	(sp)+,a6		; Restore base register
		rts

*******************************************************************************

__CXM22:	move.l	a6,-(sp)		; Save your base pointer
		move.l	_UtilityBase,a6		; Get utility base pointer
		jsr	_LVOUMult32(a6)		; Do the multply
		move.l	(sp)+,a6		; Restore base register
		rts




************************************************************************

_sprintf:	; ( string, format, {values} )
		movem.l	a2/a3/a4/a6,-(sp)
		move.l  5*4(sp),a3

		move.l	6*4(sp),a0
		lea	7*4(sp),a1
		lea	stuffChar(pc),a2
		move.l	_SysBase,a6
		jsr	_LVORawDoFmt(a6)

		movem.l	(sp)+,a2/a3/a4/a6
		rts

stuffChar:
		move.b	d0,(a3)+
		rts

		END
