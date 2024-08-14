*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** VerifyUser rom interface
	XREF	_AccountsBaseOffset
	SECTION	accounts
	XDEF	_VerifyUser
_VerifyUser:
		movem.l	a2/a6,-(a7)
		move.l	_AccountsBaseOffset(a6),a6
		movem.l	12(a7),a0/a1/a2
		jsr	-54(a6)
		movem.l	(a7)+,a2/a6
		rts
	END
