*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** VerifyUserCrypt rom interface
	XREF	_AccountsBaseOffset
	SECTION	accounts
	XDEF	_VerifyUserCrypt
_VerifyUserCrypt:
		movem.l	a2/a6,-(a7)
		move.l	_AccountsBaseOffset(a6),a6
		movem.l	12(a7),a0/a1/a2
		jsr	-114(a6)
		movem.l	(a7)+,a2/a6
		rts
	END
