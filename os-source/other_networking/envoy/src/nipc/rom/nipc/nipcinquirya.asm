*** DO NOT EDIT: FILE BUILT AUTOMATICALLY
*** NIPCInquiryA rom interface
	XREF	_NIPCBaseOffset
	SECTION	nipc
	XDEF	_NIPCInquiryA
_NIPCInquiryA:
		move.l	a6,-(a7)
		move.l	_NIPCBaseOffset(a6),a6
		move.l	8(a7),a0
		movem.l	12(a7),d0/d1/a1
		jsr	-222(a6)
		move.l	(a7)+,a6
		rts
	END