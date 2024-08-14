*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** DoTransaction ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_DoTransaction
_DoTransaction:
		movem.l	a2/a6,-(a7)
		move.l	_NIPCBase,a6
		movem.l	12(a7),a0/a1/a2
		jsr	-150(a6)
		movem.l	(a7)+,a2/a6
		rts
	END
