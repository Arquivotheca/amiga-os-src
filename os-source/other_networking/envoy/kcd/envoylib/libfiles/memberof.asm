*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** MemberOf ram interface
	XREF	_AccountsBase
	SECTION	accounts
	XDEF	_MemberOf
_MemberOf:
		move.l	a6,-(a7)
		move.l	_AccountsBase,a6
		movem.l	8(a7),a0/a1
		jsr	-60(a6)
		move.l	(a7)+,a6
		rts
	END