*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** FreeUserInfo rom interface
	XREF	_AccountsBaseOffset
	SECTION	accounts
	XDEF	_FreeUserInfo
_FreeUserInfo:
		move.l	a6,-(a7)
		move.l	_AccountsBaseOffset(a6),a6
		move.l	8(a7),a0
		jsr	-42(a6)
		move.l	(a7)+,a6
		rts
	END
