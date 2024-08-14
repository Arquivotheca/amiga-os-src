*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** StartMonitor rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_StartMonitor
_StartMonitor:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		jsr	-30(a6)
		move.l	(a7)+,a6
		rts
	END
