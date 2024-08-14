*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** GetHostName ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_GetHostName
_GetHostName:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		movem.l	8(a7),a0/a1
		move.l	16(a7),d0
		jsr	-216(a6)
		move.l	(a7)+,a6
		rts
	END
