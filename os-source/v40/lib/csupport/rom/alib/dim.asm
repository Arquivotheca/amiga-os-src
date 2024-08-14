*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** DiM rom interface
	XREF	_NoneOffset
	SECTION	alib
	XDEF	_DiM
_DiM:
		movem.l	a2/a6,-(a7)
		move.l	_NoneOffset(a6),a6
		move.l	12(a7),a2
		move.l	16(a7),a1
		jsr	-216(a6)
		movem.l	(a7)+,a2/a6
		rts
	END
