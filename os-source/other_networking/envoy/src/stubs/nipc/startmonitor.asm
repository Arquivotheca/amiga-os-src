*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** StartMonitor ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_StartMonitor
_StartMonitor:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		jsr	-150(a6)
		move.l	(a7)+,a6
		rts
	END
