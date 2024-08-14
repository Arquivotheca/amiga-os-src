*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AllocGroup rom interface
	XREF	_AuthenticationBaseOffset
	SECTION	authentication
	XDEF	_AllocGroup
_AllocGroup:
		move.l	a6,-(a7)
		move.l	_AuthenticationBaseOffset(a6),a6
		jsr	-60(a6)
		move.l	(a7)+,a6
		rts
	END
