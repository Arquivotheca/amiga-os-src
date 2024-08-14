**
**	$Id: library.asm,v 7.0 91/03/19 18:36:40 kodiak Exp $
**
**      init bullet library and interface to C functions
**
**      (C) Copyright 1991 Robert R. Burns
**          All Rights Reserved
**
	SECTION	bullet

**	Included Files

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE	"exec/initializers.i"
	INCLUDE	"exec/memory.i"
	INCLUDE "glyph.i"
	INCLUDE "bldata.i"

**	Exported Names

	XDEF	BLInitTable

**	Imported Names

	XREF	BLName
	XREF	BLID
	XREF	REVISION
	XREF	VERSION

	XREF	CHAOSDataStart
	XREF	CHAOSDATALONGS
	XREF	CHAOSTOTALBYTES
	XREF	CHAOSRelocTable
	XREF	CHAOSRELOCSIZE
	XREF	_SysBase
	XREF	_DOSBase
	XREF	_UtilityBase
	XREF	_BulletBase

	XREF	_CExpunge
	XREF	_COpenEngine
	XREF	_CCloseEngine
	XREF	_CSetInfoA
	XREF	_CObtainInfoA
	XREF	_CReleaseInfoA
	XREF	_CGetGlyphMap

	XREF	_LVOAllocMem
	XREF	_LVOCloseLibrary
	XREF	_LVOFreeMem
	XREF	_LVOOpenLibrary

blFuncInit:
		dc.w	-1
		dc.w	Open-blFuncInit
		dc.w	Close-blFuncInit
		dc.w	Expunge-blFuncInit
		dc.w	ExtFunc-blFuncInit
		dc.w	OpenEngine-blFuncInit
		dc.w	CloseEngine-blFuncInit
		dc.w	SetInfoA-blFuncInit
		dc.w	ObtainInfoA-blFuncInit
		dc.w	ReleaseInfoA-blFuncInit
		dc.w	GetGlyphMap-blFuncInit
		dc.w	-1

blStructInit:
	    INITBYTE	LN_TYPE,NT_LIBRARY
	    INITLONG	LN_NAME,BLName
	    INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
	    INITWORD	LIB_REVISION,REVISION
	    INITWORD	LIB_VERSION,VERSION
		dc.w	0

BLInitTable:
		dc.l	bl_SIZEOF
		dc.l	blFuncInit
		dc.l	blStructInit
		dc.l	Init

DLName		dc.b	'dos.library',0
ULName		dc.b	'utility.library',0
BName		dc.b	'bullet',0
		ds.w	0

;------	init
;
;	d0	library base
;	a0	segment
;
Init:
		move.l	a5,-(a7)
		move.l	d0,a5

		move.l	a0,bl_Segment(a5)
		move.l	a6,bl_SysBase(a5)

		;-- open the dos library
		lea	DLName(pc),a1
		moveq	#0,d0
		jsr	_LVOOpenLibrary(a6)
		move.l	d0,bl_DOSBase(a5)
		beq.s	iPurgeLib1
		
		;-- open the utility library
		lea	ULName(pc),a1
		moveq	#0,d0
		jsr	_LVOOpenLibrary(a6)
		move.l	d0,bl_UtilityBase(a5)
		beq.s	iPurgeLib2
		
		move.l	a5,d0
iExit:
		move.l	(a7)+,a5
		rts

iPurgeLib2:
		move.l	bl_DOSBase(a5),a1
		jsr	_LVOCloseLibrary(a6)

iPurgeLib1:
		clr.l	-(a7)			; return failure code

purgeLibA5:
		move.l	a5,a1
		move.w	LIB_NEGSIZE(a5),d0
		suba.w	d0,a1
		add.w	LIB_POSSIZE(a5),d0
		ext.l	d0
		jsr	_LVOFreeMem(a6)
		move.l	(a7)+,d0
		bra.s	iExit

		
;------	Open
;
;	a6	library base
;
Open:
		addq.w	#1,LIB_OPENCNT(a6)
		move.l	a6,d0
		rts


;------	Close
Close:
		subq.w	#1,LIB_OPENCNT(a6)
		bne.s	noUnload
		btst	#LIBB_DELEXP,LIB_FLAGS(a6)
		beq.s	noUnload
		jmp	LIB_EXPUNGE(a6)

;------	Expunge
Expunge:
		bsr	_CExpunge
		bset	#LIBB_DELEXP,LIB_FLAGS(a6)
		tst.w	LIB_OPENCNT(a6)
		bne.s	noUnload

		;-- time to expunge
		move.l	a5,-(a7)
		move.l	a6,a5

		;--	close utility library
		move.l	bl_UtilityBase(a5),a1
		move.l	bl_SysBase(a5),a6
		jsr	_LVOCloseLibrary(a6)

		;--	close dos library
		move.l	bl_DOSBase(a5),a1
		jsr	_LVOCloseLibrary(a6)

		;--	remove library
		move.l	(a5),a0			; ln_Succ
		move.l	LN_PRED(a5),a1		; ln_Pred
		move.l	a0,(a1)			; link forward
		move.l	a1,LN_PRED(a0)		; link backward

		;--	purge library
		move.l	bl_Segment(a5),-(a7)
		bra.s	purgeLibA5


;------	ExtFunc
noUnload:
ExtFunc:
		moveq	#0,d0
		rts


;------	OpenEngine
OpenEngine:
		movem.l	a4-a6,-(a7)
		move.l	a6,a5
		move.l	bl_SysBase(a5),a6
		move.l	#CHAOSTOTALBYTES,d0
		move.l	#MEMF_CLEAR,d1
		jsr	_LVOAllocMem(a6)
		tst.l	d0
		beq.s	oeExit
		move.l	d0,a4

		;-- initialize data
		lea	CHAOSDataStart(pc),a0
		move.l	a4,a1
		move.w	#CHAOSDATALONGS,d0
		bra.s	oeInitDataDBF
oeInitDataLoop:
		move.l	(a0)+,(a1)+
oeInitDataDBF:
		dbf	d0,oeInitDataLoop

		;-- perform any data relocations
		lea	CHAOSRelocTable(pc),a0
		move.w	#CHAOSRELOCSIZE,d0
		bra.s	oeRelocDataDBF
oeRelocDataLoop:
		movem.w	(a0)+,d1/a1
		add.l	a4,d1
		add.l	a4,a1
		add.l	d1,(a1)
oeRelocDataDBF:
		dbf	d0,oeRelocDataLoop

		;-- initialize public variables
		move.l	a5,gle_Library(a4)
		move.l	#BName,gle_Name(a4)

		;-- initialize library base variables
		move.l	bl_SysBase(a5),_SysBase(a4)
		move.l	bl_DOSBase(a5),_DOSBase(a4)
		move.l	bl_UtilityBase(a5),_UtilityBase(a4)
		move.l	a5,_BulletBase(a4)

		bsr	_COpenEngine
		tst.l	d0
		bne.s	purgeEngine

		move.l	a4,d0
oeExit:
		movem.l	(a7)+,a4-a6
		rts


;------	CloseEngine
CloseEngine:
		movem.l	a4-a6,-(a7)
		move.l	a0,a4			; set a4 to glyphEngine
		bsr	_CCloseEngine
		move.l	bl_SysBase(a6),a6

purgeEngine:
		move.l	a4,a1
		move.l	#CHAOSTOTALBYTES,d0
		jsr	_LVOFreeMem(a6)
		moveq	#0,d0
		movem.l	(a7)+,a4-a6
		rts


;------	SetInfoA
SetInfoA:
		move.l	a4,-(a7)		; save a4
		move.l	a0,a4			; set a4 to glyphEngine
		move.l	a1,-(a7)		; push taglist parameter
		bsr	_CSetInfoA
		addq.l	#4,a7
		move.l	(a7)+,a4
		rts

;------	ObtainInfoA
ObtainInfoA:
		move.l	a4,-(a7)		; save a4
		move.l	a0,a4			; set a4 to glyphEngine
		move.l	a1,-(a7)		; push taglist parameter
		bsr	_CObtainInfoA
		addq.l	#4,a7
		move.l	(a7)+,a4
		rts

;------	ReleaseInfoA
ReleaseInfoA:
		move.l	a4,-(a7)		; save a4
		move.l	a0,a4			; set a4 to glyphEngine
		move.l	a1,-(a7)		; push taglist parameter
		bsr	_CReleaseInfoA
		addq.l	#4,a7
		move.l	(a7)+,a4
		rts

;------	GetGlyphMap
GetGlyphMap:
		move.l	a4,-(a7)		; save a4
		move.l	a0,a4			; set a4 to glyphEngine
		movem.l	d0/a1,-(a7)		; push code & glyph parameter
		bsr	_CGetGlyphMap
		addq.l	#8,a7
		move.l	(a7)+,a4
		rts

	END
