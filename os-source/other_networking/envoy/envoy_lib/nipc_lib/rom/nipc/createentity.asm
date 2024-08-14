*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** CreateEntity rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_CreateEntity
_CreateEntity:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		lea	8(a7),a0
		jsr	-126(a6)
		move.l	(a7)+,a6
		rts
	END
