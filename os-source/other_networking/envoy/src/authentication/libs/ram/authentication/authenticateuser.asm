*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AuthenticateUser ram interface
	XREF	_AuthenticationBase
	SECTION	authentication
	XDEF	_AuthenticateUser
_AuthenticateUser:
		move.l	a6,-(a7)
		move.l	_AuthenticationBase,a6
		movem.l	8(a7),a0/a1
		jsr	-54(a6)
		move.l	(a7)+,a6
		rts
	END
