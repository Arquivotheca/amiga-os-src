*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** FreeGroupInfo ram interface
	XREF	_AccountsBase
	SECTION	accounts
	XDEF	_FreeGroupInfo
_FreeGroupInfo:
		move.l	a6,-(a7)
		move.l	_AccountsBase,a6
		move.l	8(a7),a0
		jsr	-48(a6)
		move.l	(a7)+,a6
		rts
	END
