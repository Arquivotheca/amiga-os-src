*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** LibCreatePool rom interface
	XREF	_NoneOffset
	SECTION	alib
	XDEF	_LibCreatePool
_LibCreatePool:
		movem.l	d2/a6,-(a7)
		move.l	_NoneOffset(a6),a6
		movem.l	12(a7),d0/d1/d2
		jsr	-96(a6)
		movem.l	(a7)+,d2/a6
		rts
	END
