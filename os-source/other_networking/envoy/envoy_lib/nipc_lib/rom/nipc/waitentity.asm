*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** WaitEntity rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_WaitEntity
_WaitEntity:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		move.l	8(a7),a0
		jsr	-192(a6)
		move.l	(a7)+,a6
		rts
	END
