*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AllocUserInfo ram interface
	XREF	_AccountsBase
	SECTION	accounts
	XDEF	_AllocUserInfo
_AllocUserInfo:
		move.l	a6,-(a7)
		move.l	_AccountsBase,a6
		jsr	-30(a6)
		move.l	(a7)+,a6
		rts
	END
