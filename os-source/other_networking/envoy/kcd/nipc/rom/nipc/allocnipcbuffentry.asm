*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AllocNIPCBuffEntry rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_AllocNIPCBuffEntry
_AllocNIPCBuffEntry:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		jsr	-252(a6)
		move.l	(a7)+,a6
		rts
	END
