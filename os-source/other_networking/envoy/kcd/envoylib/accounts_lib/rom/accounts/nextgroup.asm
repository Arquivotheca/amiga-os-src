*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** NextGroup rom interface
	XREF	_AccountsBaseOffset
	SECTION	accounts
	XDEF	_NextGroup
_NextGroup:
		move.l	a6,-(a7)
		move.l	_AccountsBaseOffset(a6),a6
		move.l	8(a7),a0
		jsr	-96(a6)
		move.l	(a7)+,a6
		rts
	END
