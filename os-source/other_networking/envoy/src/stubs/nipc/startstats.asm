*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** StartStats ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_StartStats
_StartStats:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		move.l	8(a7),a0
		jsr	-138(a6)
		move.l	(a7)+,a6
		rts
	END
