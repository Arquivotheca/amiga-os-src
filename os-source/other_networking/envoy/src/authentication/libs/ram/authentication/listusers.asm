*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** ListUsers ram interface
	XREF	_AuthenticationBase
	SECTION	authentication
	XDEF	_ListUsers
_ListUsers:
		move.l	a6,-(a7)
		move.l	_AuthenticationBase,a6
		move.l	8(a7),a0
		jsr	-138(a6)
		move.l	(a7)+,a6
		rts
	END
