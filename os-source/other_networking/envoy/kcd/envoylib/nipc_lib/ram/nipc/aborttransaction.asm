*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** AbortTransaction ram interface
	XREF	_NIPCBase
	SECTION	nipc
	XDEF	_AbortTransaction
_AbortTransaction:
		move.l	a6,-(a7)
		move.l	_NIPCBase,a6
		move.l	8(a7),a1
		jsr	-180(a6)
		move.l	(a7)+,a6
		rts
	END
