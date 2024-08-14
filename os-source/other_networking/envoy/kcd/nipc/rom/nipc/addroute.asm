*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AddRoute rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_AddRoute
_AddRoute:
		movem.l	d2/d3/a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		movem.l	16(a7),d0/d1/d2/d3
		jsr	-42(a6)
		movem.l	(a7)+,d2/d3/a6
		rts
	END
