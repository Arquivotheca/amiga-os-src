*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** NIPCInquiry ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_NIPCInquiry
_NIPCInquiry:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		lea	8(a7),a1
		move.l	(a1)+,a0
		movem.l	(a1)+,d0/d1
		jsr	-210(a6)
		move.l	(a7)+,a6
		rts
	END
