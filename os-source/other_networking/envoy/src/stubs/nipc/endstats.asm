*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** EndStats ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_EndStats
_EndStats:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		jsr	-144(a6)
		move.l	(a7)+,a6
		rts
	END
