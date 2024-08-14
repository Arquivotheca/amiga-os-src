*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** ArgArrayDone rom interface
	XREF	_NoneOffset
	SECTION	alib
	XDEF	_ArgArrayDone
_ArgArrayDone:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		jsr	-186(a6)
		move.l	(a7)+,a6
		rts
	END
