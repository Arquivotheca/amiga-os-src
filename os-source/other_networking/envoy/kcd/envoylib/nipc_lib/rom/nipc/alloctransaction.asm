*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AllocTransaction rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_AllocTransaction
_AllocTransaction:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		lea	8(a7),a0
		jsr	-114(a6)
		move.l	(a7)+,a6
		rts
	END
