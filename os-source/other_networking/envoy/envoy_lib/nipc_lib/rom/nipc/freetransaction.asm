*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** FreeTransaction rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_FreeTransaction
_FreeTransaction:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		move.l	8(a7),a1
		jsr	-120(a6)
		move.l	(a7)+,a6
		rts
	END
