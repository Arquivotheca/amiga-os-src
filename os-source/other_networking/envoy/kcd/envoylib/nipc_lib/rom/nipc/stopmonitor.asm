*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** StopMonitor rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_StopMonitor
_StopMonitor:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		jsr	-36(a6)
		move.l	(a7)+,a6
		rts
	END
