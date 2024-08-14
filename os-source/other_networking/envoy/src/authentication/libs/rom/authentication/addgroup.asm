*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AddGroup rom interface
	XREF	_AuthenticationBaseOffset
	SECTION	authentication
	XDEF	_AddGroup
_AddGroup:
		move.l	a6,-(a7)
		move.l	_AuthenticationBaseOffset(a6),a6
		movem.l	8(a7),a0/a1
		jsr	-30(a6)
		move.l	(a7)+,a6
		rts
	END
