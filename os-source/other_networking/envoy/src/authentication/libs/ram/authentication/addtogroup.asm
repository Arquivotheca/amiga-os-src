*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AddToGroup ram interface
	XREF	_AuthenticationBase
	SECTION	authentication
	XDEF	_AddToGroup
_AddToGroup:
		movem.l	a2/a6,-(a7)
		move.l	_AuthenticationBase,a6
		movem.l	12(a7),a0/a1/a2
		jsr	-36(a6)
		movem.l	(a7)+,a2/a6
		rts
	END
