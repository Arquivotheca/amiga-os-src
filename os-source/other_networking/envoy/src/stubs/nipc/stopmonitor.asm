*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** StopMonitor ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_StopMonitor
_StopMonitor:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		jsr	-156(a6)
		move.l	(a7)+,a6
		rts
	END
