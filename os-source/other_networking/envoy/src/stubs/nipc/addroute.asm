*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AddRoute ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_AddRoute
_AddRoute:
		movem.l	d2/d3/d4/a6,-(a7)
		move.l	_NIPCBase,a6
		movem.l	20(a7),d0/d1/d2/d3/d4
		jsr	-120(a6)
		movem.l	(a7)+,d2/d3/d4/a6
		rts
	END
