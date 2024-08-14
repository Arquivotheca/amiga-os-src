*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** UnlockPool rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_UnlockPool
_UnlockPool:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		jsr	-78(a6)
		move.l	(a7)+,a6
		rts
	END
