; replacements for standard lc.lib routines using utility.library instead

	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE	"datatypesbase.i"

	LIST

;---------------------------------------------------------------------------

	XREF	_SysBase
	XREF	_LVORawDoFmt

	XDEF	__sprintf

;---------------------------------------------------------------------------

__sprintf:	; ( string, format, {values} )
	movem.l	a2/a3/a4/a6,-(sp)
	move.l  5*4(sp),a3

	move.l	6*4(sp),a0
	lea	7*4(sp),a1
	lea	stuffChar(pc),a2
	move.l	dtl_SysBase(a6),a6
	jsr	_LVORawDoFmt(a6)

	movem.l	(sp)+,a2/a3/a4/a6
	rts

stuffChar:
	move.b	d0,(a3)+
	rts

;---------------------------------------------------------------------------

	END
