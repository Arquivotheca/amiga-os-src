*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** InvertString rom interface
	XREF	_NoneOffset
	SECTION	alib
	XDEF	_InvertString
_InvertString:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		movem.l	8(a7),a0/a1
		jsr	-216(a6)
		move.l	(a7)+,a6
		rts
	END
