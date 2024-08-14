
* newalloc.asm part of snoop.c
* modified to do AllocVecs and FreeVecs too

	INCLUDE	'exec/types.i'
	INCLUDE	'exec/exec.i'
	INCLUDE 'exec/execbase.i'

	xref	_OldAllocMem
	xref	_OldFreeMem
	xref	_OldAllocVec
	xref	_OldFreeVec
	xref	_KPutFmt
	xref	_KPutS
	xref	_LVOForbid
	xref	_LVOPermit
	xref	_LVOSetSR
	xref	_LVOAlert

C_CALLER	equ	8
A_CALLER	equ	4

	section code

	xdef	_SnoopAllocMem
_SnoopAllocMem:
		move.l	a2,-(sp)

		jsr	_LVOForbid(a6)		; only one print stack
		lea	printStack,a2
		move.l	ThisTask(a6),a0
		move.l  LN_NAME(a0),-(a2)
		move.l	#' ',-(a2)

		move.l  C_CALLER(sp),-(a2)
		move.l  A_CALLER(sp),-(a2)
		movem.l	d0/d1,-(a2)

		move.l	_OldAllocMem,a0
		jsr	(a0)

		move.l	d0,-(a2)
		pea	(a2)
		pea	AMFmt(pc)
		jsr	_KPutFmt
		addq.l	#8,sp

		bsr	AssertUserMode

		move.l	(a2),d0

		jsr	_LVOPermit(a6)

		move.l	(sp)+,a2
		rts

AMFmt:
	dc.b	'$%08lx=AllocMem(%6ld,$%08lx) '
	dc.b	'A:%08lx C:%08lx %lc "%s"',10,0

		cnop 0,2
	xdef	_SnoopFreeMem
_SnoopFreeMem:
		move.l	a2,-(sp)

		jsr	_LVOForbid(a6)		; only one print stack
		lea	printStack,a2
		move.l	ThisTask(a6),a0
		move.l  LN_NAME(a0),-(a2)
		move.l	#' ',-(a2)

		move.l  C_CALLER(sp),-(a2)
		move.l  A_CALLER(sp),-(a2)
		move.l	D0,-(a2)
		move.l	A1,-(a2)

		move.l	_OldFreeMem,a0
		jsr	(a0)

		move.l	d0,-(a2)
		pea	(a2)
		pea	FMFmt(pc)
		jsr	_KPutFmt
		addq.l	#8,sp

		bsr	AssertUserMode

		move.l	(a2),d0
		jsr	_LVOPermit(a6)
		move.l	(sp)+,a2

		rts

FMFmt
	dc.b	'$%08lx= FreeMem($%08lx,%6ld) '
	dc.b	'A:%08lx C:%08lx %lc "%s"',10,0

	xdef	_SnoopAllocVec

		cnop 0,2
_SnoopAllocVec:
		move.l	a2,-(sp)

		jsr	_LVOForbid(a6)		; only one print stack
		lea	printStack,a2
		move.l	ThisTask(a6),a0
		move.l  LN_NAME(a0),-(a2)
		move.l	#' ',-(a2)

		move.l  C_CALLER(sp),-(a2)
		move.l  A_CALLER(sp),-(a2)
		movem.l	d0/d1,-(a2)

		move.l	_OldAllocVec,a0
		jsr	(a0)

		move.l	d0,-(a2)

		tst.l	d0
		beq.s   nomem
		move.l  d0,a0		; change requested size to actual if succ
		suba.l	#4,a0
		move.l  (a0),4(a2)
nomem
		pea	(a2)
		pea	AVFmt(pc)
		jsr	_KPutFmt
		addq.l	#8,sp

		bsr	AssertUserMode

		move.l	(a2),d0

		jsr	_LVOPermit(a6)

		move.l	(sp)+,a2
		rts

AVFmt:
	dc.b	'$%08lx=AllocVec(%6ld,$%08lx) '
	dc.b	'A:%08lx C:%08lx %lc "%s"',10,0


	xdef	_SnoopFreeVec

		cnop 0,2
_SnoopFreeVec:
		move.l	a2,-(sp)

		jsr	_LVOForbid(a6)		; only one print stack
		lea	printStack,a2
		move.l	ThisTask(a6),a0
		move.l  LN_NAME(a0),-(a2)
		move.l	#' ',-(a2)

		move.l  C_CALLER(sp),-(a2)
		move.l  A_CALLER(sp),-(a2)

		cmpa.l	#0,A1			; is this a freevec of zero ?
		bne.s	notfv0			; no
		move.l	#0,-(a2)		; yes - just push size of 0
		bra.s	wasfv0
notfv0
		move.l	A1,a0			; grab size out of deallocation
		suba.l  #4,a0
		move.l  (a0),-(a2)		; push actual size
wasfv0
		move.l	A1,-(a2)		; push address

		move.l	_OldFreeVec,a0
		jsr	(a0)

		move.l	4(a2),-(a2)		; get stashed size for actual

		pea	(a2)
		pea	FVFmt(pc)
		jsr	_KPutFmt
		addq.l	#8,sp

		bsr	AssertUserMode

		move.l	(a2),d0
		jsr	_LVOPermit(a6)
		move.l	(sp)+,a2

		rts

FVFmt
	dc.b	'$%08lx= FreeVec($%08lx,%6ld) '
	dc.b	'A:%08lx C:%08lx %lc "%s"',10,0


	CNOP 0,4

AssertUserMode:
		moveq.l	#0,d0
		moveq.l	#0,d1
		jsr	_LVOSetSR(a6)
		btst.l	#13,d0
		beq.s	1$
		pea	msgSuperCrash(PC)
		jsr	_KPutS
		move.l	#$88880000,d7
		jmp	_LVOAlert(a6)
1$:		rts


msgSuperCrash:
	dc.b	10,'*** SUPERVISOR MODE MEMORY FUNCTION ***',10,0
	ds.w	0

	section data

	ds.l	512
printStack


	END
