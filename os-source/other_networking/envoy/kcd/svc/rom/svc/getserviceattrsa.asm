*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** GetServiceAttrsA rom interface
	XREF	_SvcBaseOffset
	SECTION	svc
	XDEF	_GetServiceAttrsA
_GetServiceAttrsA:
		move.l	a6,-(a7)
		move.l	_SvcBaseOffset(a6),a6
		move.l	8(a7),a0
		jsr	-42(a6)
		move.l	(a7)+,a6
		rts
	END
