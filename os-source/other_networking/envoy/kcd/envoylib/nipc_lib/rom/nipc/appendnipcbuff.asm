*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AppendNIPCBuff rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_AppendNIPCBuff
_AppendNIPCBuff:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		movem.l	8(a7),a0/a1
		jsr	-282(a6)
		move.l	(a7)+,a6
		rts
	END