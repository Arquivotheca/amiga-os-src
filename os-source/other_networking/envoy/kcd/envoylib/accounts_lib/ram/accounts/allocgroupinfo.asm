*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AllocGroupInfo ram interface
	XREF	_AccountsBase
	SECTION	accounts
	XDEF	_AllocGroupInfo
_AllocGroupInfo:
		move.l	a6,-(a7)
		move.l	_AccountsBase,a6
		jsr	-36(a6)
		move.l	(a7)+,a6
		rts
	END
