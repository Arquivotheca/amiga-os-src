*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AllocUserInfo rom interface
	XREF	_AccountsBaseOffset
	SECTION	accounts
	XDEF	_AllocUserInfo
_AllocUserInfo:
		move.l	a6,-(a7)
		move.l	_AccountsBaseOffset(a6),a6
		jsr	-30(a6)
		move.l	(a7)+,a6
		rts
	END
