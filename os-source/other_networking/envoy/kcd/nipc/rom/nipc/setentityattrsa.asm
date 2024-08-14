*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** SetEntityAttrsA rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_SetEntityAttrsA
_SetEntityAttrsA:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		movem.l	8(a7),a0/a1
		jsr	-240(a6)
		move.l	(a7)+,a6
		rts
	END
