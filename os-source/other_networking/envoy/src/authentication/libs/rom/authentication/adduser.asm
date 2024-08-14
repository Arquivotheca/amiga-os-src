*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AddUser rom interface
	XREF	_AuthenticationBaseOffset
	SECTION	authentication
	XDEF	_AddUser
_AddUser:
		move.l	a6,-(a7)
		move.l	_AuthenticationBaseOffset(a6),a6
		movem.l	8(a7),a0/a1
		jsr	-42(a6)
		move.l	(a7)+,a6
		rts
	END
