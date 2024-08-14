*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** FreeUserInfo ram interface
	XREF	_AccountsBase
	SECTION	accounts
	XDEF	_FreeUserInfo
_FreeUserInfo:
		move.l	a6,-(a7)
		move.l	_AccountsBase,a6
		move.l	8(a7),a0
		jsr	-42(a6)
		move.l	(a7)+,a6
		rts
	END
