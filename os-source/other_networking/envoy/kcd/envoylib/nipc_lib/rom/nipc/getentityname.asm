*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** GetEntityName rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_GetEntityName
_GetEntityName:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		movem.l	8(a7),a0/a1
		move.l	16(a7),d0
		jsr	-198(a6)
		move.l	(a7)+,a6
		rts
	END
