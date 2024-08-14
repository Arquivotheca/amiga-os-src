*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** IDToGroup rom interface
	XREF	_AccountsBaseOffset
	SECTION	accounts
	XDEF	_IDToGroup
_IDToGroup:
		move.l	a6,-(a7)
		move.l	_AccountsBaseOffset(a6),a6
		movem.l	8(a7),d0/a0
		jsr	-84(a6)
		move.l	(a7)+,a6
		rts
	END
