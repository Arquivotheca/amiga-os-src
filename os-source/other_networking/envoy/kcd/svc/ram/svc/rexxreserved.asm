*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** RexxReserved ram interface
	XREF	_SvcBase
	SECTION	svc
	XDEF	_RexxReserved
_RexxReserved:
		move.l	a6,-(a7)
		move.l	_SvcBase,a6
		jsr	-30(a6)
		move.l	(a7)+,a6
		rts
	END
