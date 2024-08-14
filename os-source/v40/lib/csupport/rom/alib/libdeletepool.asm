*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** LibDeletePool rom interface
	XREF	_NoneOffset
	SECTION	alib
	XDEF	_LibDeletePool
_LibDeletePool:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		move.l	8(a7),a0
		jsr	-102(a6)
		move.l	(a7)+,a6
		rts
	END
