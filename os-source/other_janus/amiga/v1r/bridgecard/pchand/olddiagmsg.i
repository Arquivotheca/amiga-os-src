INFOMSG 	MACRO
		IFGE	INFOLEVEL-\1
		IFND	RPutFmt
	XREF	_AbsExecBase
	XREF	_LVOOpenResource
		bra.s	RPFSkip
RPutFmt:
		movem.l a0/a1,-(a7)
		lea	RPFDRName(pc),a1
		move.l	_AbsExecBase,a6
		jsr	_LVOOpenResource(a6)
		movem.l (a7)+,a0/a1
		tst.l	d0
		beq.s	RPFRts
		move.l	d0,a6
		jsr	-6(a6)
RPFRts:
		rts
RPFDRName:
		dc.b	'diag.resource',0
		ds.w	0
RPFSkip:
		ENDC
		MOVEM.L D0/D1/A0/A1/A6,-(SP)
		LEA	20(a7),a1
		LEA	MSG\@(pc),a0
		BSR	RPutFmt
		MOVEM.L (SP)+,D0/D1/A0/A1/A6
		bra.s	SKIP\@
MSG\@		DC.B	\2,10,0
		DS.W	0
SKIP\@:
		ENDC
		ENDM

