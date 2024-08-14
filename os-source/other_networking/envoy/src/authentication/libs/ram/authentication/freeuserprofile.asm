*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** FreeUserProfile ram interface
	XREF	_AuthenticationBase
	SECTION	authentication
	XDEF	_FreeUserProfile
_FreeUserProfile:
		move.l	a6,-(a7)
		move.l	_AuthenticationBase,a6
		move.l	8(a7),a0
		jsr	-120(a6)
		move.l	(a7)+,a6
		rts
	END
