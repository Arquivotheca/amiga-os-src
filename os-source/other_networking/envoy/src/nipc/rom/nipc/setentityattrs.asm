*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** SetEntityAttrs rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_SetEntityAttrs
_SetEntityAttrs:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		lea	8(a7),a1
		move.l	(a1)+,a0
		jsr	-240(a6)
		move.l	(a7)+,a6
		rts
	END
