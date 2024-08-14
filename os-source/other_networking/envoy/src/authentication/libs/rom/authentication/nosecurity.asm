*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** NoSecurity rom interface
	XREF	_AuthenticationBaseOffset
	SECTION	authentication
	XDEF	_NoSecurity
_NoSecurity:
		move.l	a6,-(a7)
		move.l	_AuthenticationBaseOffset(a6),a6
		move.l	8(a7),d0
		jsr	-156(a6)
		move.l	(a7)+,a6
		rts
	END
