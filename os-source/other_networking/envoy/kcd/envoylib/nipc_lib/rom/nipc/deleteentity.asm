*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** DeleteEntity rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_DeleteEntity
_DeleteEntity:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		move.l	8(a7),a0
		jsr	-132(a6)
		move.l	(a7)+,a6
		rts
	END