*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** CheckRexxMsg rom interface
	XREF	_NoneOffset
	SECTION	alib
	XDEF	_CheckRexxMsg
_CheckRexxMsg:
		move.l	a6,-(a7)
		move.l	_NoneOffset(a6),a6
		move.l	8(a7),a0
		jsr	-228(a6)
		move.l	(a7)+,a6
		rts
	END
