*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** GetRouteInfo ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_GetRouteInfo
_GetRouteInfo:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		jsr	-132(a6)
		move.l	(a7)+,a6
		rts
	END
