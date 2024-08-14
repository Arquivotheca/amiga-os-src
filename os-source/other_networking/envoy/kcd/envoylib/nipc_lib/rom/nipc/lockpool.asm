*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** LockPool rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_LockPool
_LockPool:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		jsr	-72(a6)
		move.l	(a7)+,a6
		rts
	END
