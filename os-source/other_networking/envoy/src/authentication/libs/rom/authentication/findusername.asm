*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** FindUserName rom interface
	XREF	_AuthenticationBaseOffset
	SECTION	authentication
	XDEF	_FindUserName
_FindUserName:
		move.l	a6,-(a7)
		move.l	_AuthenticationBaseOffset(a6),a6
		move.l	8(a7),a0
		jsr	-114(a6)
		move.l	(a7)+,a6
		rts
	END
