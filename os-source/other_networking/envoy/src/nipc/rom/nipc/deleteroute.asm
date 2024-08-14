*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** DeleteRoute rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_DeleteRoute
_DeleteRoute:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		move.l	8(a7),d0
		jsr	-48(a6)
		move.l	(a7)+,a6
		rts
	END
