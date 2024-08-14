*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AllocGroupInfo rom interface
	XREF	_AccountsBaseOffset
	SECTION	accounts
	XDEF	_AllocGroupInfo
_AllocGroupInfo:
		move.l	a6,-(a7)
		move.l	_AccountsBaseOffset(a6),a6
		jsr	-36(a6)
		move.l	(a7)+,a6
		rts
	END
