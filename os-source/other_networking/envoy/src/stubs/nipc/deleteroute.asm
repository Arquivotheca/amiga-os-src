*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** DeleteRoute ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_DeleteRoute
_DeleteRoute:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		move.l	8(a7),d0
		jsr	-126(a6)
		move.l	(a7)+,a6
		rts
	END
