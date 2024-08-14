*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** GetEntityName ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_GetEntityName
_GetEntityName:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		movem.l	8(a7),a0/a1
		move.l	16(a7),d0
		jsr	-198(a6)
		move.l	(a7)+,a6
		rts
	END
