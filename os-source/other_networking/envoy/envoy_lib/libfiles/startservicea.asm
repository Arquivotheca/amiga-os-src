*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** StartServiceA ram interface
	XREF	_SvcBase
	SECTION	svc
	XDEF	_StartServiceA
_StartServiceA:
		move.l	a6,-(a7)
		move.l	_SvcBase,a6
		move.l	8(a7),a0
		jsr	-36(a6)
		move.l	(a7)+,a6
		rts
	END
