*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** FindEntity ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_FindEntity
_FindEntity:
		movem.l	a2/a3/a6,-(a7)
		move.l	_NIPCBase,a6
		movem.l	16(a7),a0/a1/a2/a3
		jsr	-138(a6)
		movem.l	(a7)+,a2/a3/a6
		rts
	END
