*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** SetRexxVar rom interface
	XREF	_NoneOffset
	SECTION	alib
	XDEF	_SetRexxVar
_SetRexxVar:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		movem.l	8(a7),a0/a1
		movem.l	16(a7),d0/d1
		jsr	-240(a6)
		move.l	(a7)+,a6
		rts
	END
