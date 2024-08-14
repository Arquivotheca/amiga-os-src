*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** EndStats rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_EndStats
_EndStats:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		jsr	-60(a6)
		move.l	(a7)+,a6
		rts
	END
