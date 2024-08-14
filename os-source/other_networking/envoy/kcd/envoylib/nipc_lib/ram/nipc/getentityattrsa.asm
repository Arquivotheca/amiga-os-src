*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** GetEntityAttrsA ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_GetEntityAttrsA
_GetEntityAttrsA:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		movem.l	8(a7),a0/a1
		jsr	-222(a6)
		move.l	(a7)+,a6
		rts
	END
