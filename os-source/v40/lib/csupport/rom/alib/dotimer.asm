*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** DoTimer rom interface
	XREF	_NoneOffset
	SECTION	alib
	XDEF	_DoTimer
_DoTimer:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		move.l	8(a7),a0
		movem.l	12(a7),d0/d1
		jsr	-180(a6)
		move.l	(a7)+,a6
		rts
	END
