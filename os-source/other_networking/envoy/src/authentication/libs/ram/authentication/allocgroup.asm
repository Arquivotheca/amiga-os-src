*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AllocGroup ram interface
	XREF	_AuthenticationBase
	SECTION	authentication
	XDEF	_AllocGroup
_AllocGroup:
		move.l	a6,-(a7)
		move.l	_AuthenticationBase,a6
		jsr	-60(a6)
		move.l	(a7)+,a6
		rts
	END
