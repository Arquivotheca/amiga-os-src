	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/alerts.i"
	INCLUDE	"exec/initializers.i"
	INCLUDE	"exec/resident.i"
	INCLUDE	"dos/dos.i"

	INCLUDE	"commodities_rev.i"
	INCLUDE "commoditiesbase.i"
	INCLUDE "commodities.i"

	LIST

;---------------------------------------------------------------------------

	XREF	_InitCommodities
	XREF	_ShutdownCommodities

	XREF	_ActivateCxObjLVO
	XREF	_AddIEventsLVO
	XREF	_AttachCxObjLVO
	XREF	_BrokerCommandLVO
	XREF	_ClearCxObjErrorLVO
	XREF	_CopyBrokerListLVO
	XREF	_CreateCxObjLVO
	XREF	_CxBrokerLVO
	XREF	_CxMsgDataLVO
	XREF	_CxMsgIDLVO
	XREF	_CxMsgTypeLVO
	XREF	_CxObjErrorLVO
	XREF	_CxObjTypeLVO
	XREF	_DeleteCxObjLVO
	XREF	_DeleteCxObjAllLVO
	XREF	_DisposeCxMsgLVO
	XREF	_DivertCxMsgLVO
	XREF	_EnqueueCxObjLVO
	XREF	_FreeBrokerListLVO
	XREF	_InsertCxObjLVO
	XREF	_InvertKeyMapLVO
	XREF	_ParseIXLVO
	XREF	_RemoveCxObjLVO
	XREF	_RouteCxMsgLVO
	XREF	_SetCxObjPriLVO
	XREF	_SetFilterLVO
	XREF	_SetFilterIXLVO
	XREF	_SetTranslateLVO
	XREF	_MatchIXLVO
;	XREF	_SetCxObjAttrsALVO

	XREF	_LVOAlert
	XREF	_LVOCloseLibrary
	XREF	_LVOFreeMem
	XREF	_LVOInitSemaphore
	XREF	_LVOOpenLibrary

	XREF	_FlushPools
	XREF    ENDCODE

;---------------------------------------------------------------------------

	XDEF	LibInit
	XDEF	LibOpen
	XDEF	LibClose
	XDEF	LibExpunge
	XDEF	LibReserved

;---------------------------------------------------------------------------

; First executable location, must return an error to the caller
Start:
	moveq   #-1,d0
	rts

;-----------------------------------------------------------------------

ROMTAG:
	DC.W    RTC_MATCHWORD		; UWORD RT_MATCHWORD
	DC.L    ROMTAG			; APTR  RT_MATCHTAG
	DC.L    ENDCODE			; APTR  RT_ENDSKIP
	DC.B    RTF_AUTOINIT		; UBYTE RT_FLAGS
	DC.B    VERSION			; UBYTE RT_VERSION
	DC.B    NT_LIBRARY		; UBYTE RT_TYPE
	DC.B    0			; BYTE  RT_PRI
	DC.L    LibName			; APTR  RT_NAME
	DC.L    LibId			; APTR  RT_IDSTRING
	DC.L    LibInitTable		; APTR  RT_INIT

LibName DC.B 'commodities.library',0
LibId   VSTRING

	CNOP	0,4

LibInitTable:
	DC.L	CxLib_SIZEOF
	DC.L	LibFuncTable
	DC.L	0
	DC.L	LibInit

V_DEF	MACRO
	DC.W	\1+(*-LibFuncTable)
	ENDM

LibFuncTable:
	DC.W	-1
	V_DEF	LibOpen
	V_DEF	LibClose
	V_DEF	LibExpunge
	V_DEF	LibReserved
	V_DEF	_CreateCxObjLVO
	V_DEF	_CxBrokerLVO
	V_DEF	_ActivateCxObjLVO
	V_DEF	_DeleteCxObjLVO
	V_DEF	_DeleteCxObjAllLVO
	V_DEF	_CxObjTypeLVO
	V_DEF	_CxObjErrorLVO
	V_DEF	_ClearCxObjErrorLVO
	V_DEF	_SetCxObjPriLVO
	V_DEF	_AttachCxObjLVO
	V_DEF	_EnqueueCxObjLVO
	V_DEF	_InsertCxObjLVO
	V_DEF	_RemoveCxObjLVO
	V_DEF	LibReserved
	V_DEF	_SetTranslateLVO
	V_DEF	_SetFilterLVO
	V_DEF	_SetFilterIXLVO
	V_DEF	_ParseIXLVO
	V_DEF	_CxMsgTypeLVO
	V_DEF	_CxMsgDataLVO
	V_DEF	_CxMsgIDLVO
	V_DEF	_DivertCxMsgLVO
	V_DEF	_RouteCxMsgLVO
	V_DEF	_DisposeCxMsgLVO
	V_DEF	_InvertKeyMapLVO
	V_DEF	_AddIEventsLVO
	V_DEF	_CopyBrokerListLVO
	V_DEF	_FreeBrokerListLVO
	V_DEF	_BrokerCommandLVO
	V_DEF	_MatchIXLVO
;	V_DEF	_SetCxObjAttrsALVO
	V_DEF	CxReserved0
	V_DEF	CxReserved1
	V_DEF	CxReserved2
	V_DEF	CxReserved3
	V_DEF	CxReserved4
	DC.W   -1

;-----------------------------------------------------------------------

; Library Init entry point called when library is first loaded in memory
; On entry, D0 points to library base, A0 has lib seglist, A6 has SysBase
; Returns 0 for failure or the library base for success.
LibInit:
	movem.l	a0/a5/d7,-(sp)
	move.l	d0,a5
	move.l	a6,cx_SysBase(a5)
	move.l	a0,cx_SegList(a5)

	move.w	#REVISION,LIB_REVISION(a5)

	move.l	#AG_OpenLib!AO_UtilityLib,d7
	lea	UtilityName(pc),a1
	moveq	#LIBVERSION,d0
	CALL	OpenLibrary
	move.l	d0,cx_UtilityBase(a5)
	beq.s   FailInit

;	move.l	#AG_OpenLib!AO_GraphicsLib,d7
;	lea	GfxName(pc),a1
;	moveq	#LIBVERSION,d0
;	CALL	OpenLibrary
;	move.l	d0,cx_GfxBase(a5)
;	beq.s   FailInit

	move.l	#AG_OpenLib,D7
	lea	KeymapName(pc),a1
	moveq	#LIBVERSION,d0
	CALL	OpenLibrary
	move.l	d0,cx_KeymapBase(a5)
	beq.s   FailInit

	move.l	a5,d0
	movem.l	(sp)+,a0/a5/d7
	rts

FailInit:
	bsr	CloseLibs
        CALL	Alert
	movem.l	(sp)+,a0/a5/d7
	moveq	#0,d0
	rts


LIBVERSION    EQU  37
UtilityName   DC.B "utility.library",0
KeymapName    DC.B "keymap.library",0
;GfxName	      DC.B "graphics.library",0

	CNOP 0,2

;-----------------------------------------------------------------------

; Library open entry point called every OpenLibrary()
; On entry, A6 has CxBase, task switching is disabled
; Returns 0 for failure, or CxBase for success.
LibOpen:
	addq.w	#1,cx_UsageCnt(a6)
	bclr	#LIBB_DELEXP,LIB_FLAGS(a6)

	cmp.w	#1,cx_UsageCnt(a6)
	bne.s	ExitOpen

	; if first open, then init lib base
	bsr	_InitCommodities
	tst.w	d0
	bne.s	ExitOpen

	; couldn't init, so clean up lib base
	bsr	_ShutdownCommodities
	moveq	#0,d0
	rts

ExitOpen:
	move.l	a6,d0
	rts

;-----------------------------------------------------------------------

; Library close entry point called every CloseLibrary()
; On entry, A6 has CxBase, task switching is disabled
; Returns 0 normally, or the library seglist when lib should be expunged
;   due to delayed expunge bit being set
LibClose:
	subq.w	#1,cx_UsageCnt(a6)
        bne.s	1$
        bsr	_ShutdownCommodities

	; if delayed expunge bit set, then try to get rid of the library
1$:	btst	#LIBB_DELEXP,LIB_FLAGS(a6)
	bne.s	CloseExpunge

	; delayed expunge not set, so stick around
	moveq	#0,d0
	rts

CloseExpunge:
	; if no library users, then just remove the library
	tst.w	cx_UsageCnt(a6)
	beq.s	DoExpunge

	; still some library users, so just don't leave
	bsr	_FlushPools
	bclr	#LIBB_DELEXP,LIB_FLAGS(a6)
	moveq	#0,d0
	rts

;-----------------------------------------------------------------------

; Library expunge entry point called whenever system memory is lacking
; On entry, A6 has CxBase, task switching is disabled
; Returns the library seglist if the library open count is 0, returns 0
; otherwise and sets the delayed expunge bit.
LibExpunge:
	bsr	_FlushPools
	tst.w	cx_UsageCnt(a6)
	beq.s	DoExpunge

	bset	#LIBB_DELEXP,LIB_FLAGS(a6)
	moveq	#0,d0
	rts

DoExpunge:
	movem.l	d2/a5/a6,-(sp)
	move.l	a6,a5
	move.l	cx_SysBase(a5),a6
	move.l	cx_SegList(a5),d2

	move.l  a5,a1
	REMOVE

	bsr.s	CloseLibs

	move.l	a5,a1
	moveq	#0,d0
	move.w	LIB_NEGSIZE(a5),d0
	sub.l	d0,a1
	add.w	LIB_POSSIZE(a5),d0
	CALL	FreeMem

	move.l	d2,d0
	movem.l	(sp)+,d2/a5/a6
	rts

;-----------------------------------------------------------------------

CloseLibs:
	move.l	cx_UtilityBase(a5),a1
	CALL	CloseLibrary

;	move.l	cx_GfxBase(a5),a1
;	CALL	CloseLibrary

	move.l	cx_KeymapBase(a5),a1
	GO	CloseLibrary

;-----------------------------------------------------------------------

LibReserved:
CxReserved0:
CxReserved1:
CxReserved2:
CxReserved3:
CxReserved4:
	moveq	#0,d0
	rts

;-----------------------------------------------------------------------

	END
