*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AllocUserProfile rom interface
	XREF	_AuthenticationBaseOffset
	SECTION	authentication
	XDEF	_AllocUserProfile
_AllocUserProfile:
		move.l	a6,-(a7)
		move.l	_AuthenticationBaseOffset(a6),a6
		jsr	-48(a6)
		move.l	(a7)+,a6
		rts
	END
