*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** NewList rom interface
	XREF	_NoneOffset
	SECTION	alib
	XDEF	_NewList
_NewList:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		move.l	8(a7),a0
		jsr	-84(a6)
		move.l	(a7)+,a6
		rts
	END