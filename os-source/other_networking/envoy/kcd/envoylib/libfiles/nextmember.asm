*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** NextMember ram interface
	XREF	_AccountsBase
	SECTION	accounts
	XDEF	_NextMember
_NextMember:
		move.l	a6,-(a7)
		move.l	_AccountsBase,a6
		movem.l	8(a7),a0/a1
		jsr	-102(a6)
		move.l	(a7)+,a6
		rts
	END
