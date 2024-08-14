*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** DeleteUser ram interface
	XREF	_AuthenticationBase
	SECTION	authentication
	XDEF	_DeleteUser
_DeleteUser:
		move.l	a6,-(a7)
		move.l	_AuthenticationBase,a6
		movem.l	8(a7),a0/a1
		jsr	-90(a6)
		move.l	(a7)+,a6
		rts
	END
