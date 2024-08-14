*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** NameToGroup rom interface
	XREF	_AccountsBaseOffset
	SECTION	accounts
	XDEF	_NameToGroup
_NameToGroup:
		move.l	a6,-(a7)
		move.l	_AccountsBaseOffset(a6),a6
		movem.l	8(a7),a0/a1
		jsr	-72(a6)
		move.l	(a7)+,a6
		rts
	END
