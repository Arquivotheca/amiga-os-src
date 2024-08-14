*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** StartService rom interface
	XREF	_SvcBaseOffset
	SECTION	svc
	XDEF	_StartService
_StartService:
		movem.l	a2/a6,-(a7)
		move.l	_SvcBaseOffset(a6),a6
		movem.l	12(a7),a0/a1/a2
		jsr	-36(a6)
		movem.l	(a7)+,a2/a6
		rts
	END
