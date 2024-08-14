*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** WaitTransaction rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_WaitTransaction
_WaitTransaction:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		move.l	8(a7),a1
		jsr	-198(a6)
		move.l	(a7)+,a6
		rts
	END
