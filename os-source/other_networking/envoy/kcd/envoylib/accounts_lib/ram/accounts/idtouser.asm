*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** IDToUser ram interface
	XREF	_AccountsBase
	SECTION	accounts
	XDEF	_IDToUser
_IDToUser:
		move.l	a6,-(a7)
		move.l	_AccountsBase,a6
		movem.l	8(a7),d0/a0
		jsr	-78(a6)
		move.l	(a7)+,a6
		rts
	END
