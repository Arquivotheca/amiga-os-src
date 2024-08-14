*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AuthErrorText rom interface
	XREF	_AuthenticationBaseOffset
	SECTION	authentication
	XDEF	_AuthErrorText
_AuthErrorText:
		move.l	a6,-(a7)
		move.l	_AuthenticationBaseOffset(a6),a6
		movem.l	8(a7),d0/a0
		move.l	16(a7),d1
		jsr	-96(a6)
		move.l	(a7)+,a6
		rts
	END
