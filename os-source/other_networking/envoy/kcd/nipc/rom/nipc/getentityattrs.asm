*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** GetEntityAttrs rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_GetEntityAttrs
_GetEntityAttrs:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		lea	8(a7),a1
		move.l	(a1)+,a0
		jsr	-234(a6)
		move.l	(a7)+,a6
		rts
	END