*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** NameToUser rom interface
	XREF	_AccountsBaseOffset
	SECTION	accounts
	XDEF	_NameToUser
_NameToUser:
		move.l	a6,-(a7)
		move.l	_AccountsBaseOffset(a6),a6
		movem.l	8(a7),a0/a1
		jsr	-66(a6)
		move.l	(a7)+,a6
		rts
	END
