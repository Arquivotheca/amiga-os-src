*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** CopyNIPCBuff rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_CopyNIPCBuff
_CopyNIPCBuff:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		movem.l	8(a7),a0/a1
		movem.l	16(a7),d0/d1
		jsr	-258(a6)
		move.l	(a7)+,a6
		rts
	END
