        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/alerts.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/resident.i"
        INCLUDE "libraries/dos.i"

        INCLUDE "iffparse_rev.i"
        INCLUDE "iffparsebase.i"

        LIST

;---------------------------------------------------------------------------

	XREF	_AllocIFFLVO
	XREF	_OpenIFFLVO
	XREF	_ParseIFFLVO
	XREF	_CloseIFFLVO
	XREF	_FreeIFFLVO
	XREF	_ReadChunkBytesLVO
	XREF	_WriteChunkBytesLVO
	XREF	_ReadChunkRecordsLVO
	XREF	_WriteChunkRecordsLVO
	XREF	_PushChunkLVO
	XREF	_PopChunkLVO
	XREF	_EntryHandlerLVO
	XREF	_ExitHandlerLVO
	XREF	_PropChunkLVO
	XREF	_PropChunksLVO
	XREF	_StopChunkLVO
	XREF	_StopChunksLVO
	XREF	_CollectionChunkLVO
	XREF	_CollectionChunksLVO
	XREF	_StopOnExitLVO
	XREF	_FindPropLVO
	XREF	_FindCollectionLVO
	XREF	_FindPropContextLVO
	XREF	_CurrentChunkLVO
	XREF	_ParentChunkLVO
	XREF	_AllocLocalItemLVO
	XREF	_LocalItemDataLVO
	XREF	_SetLocalItemPurgeLVO
	XREF	_FreeLocalItemLVO
	XREF	_FindLocalItemLVO
	XREF	_StoreLocalItemLVO
	XREF	_StoreItemInContextLVO
	XREF	_InitIFFLVO
	XREF	_InitIFFasDOSLVO
	XREF	_InitIFFasClipLVO
	XREF	_OpenClipboardLVO
	XREF	_CloseClipboardLVO
	XREF	_GoodIDLVO
	XREF	_GoodTypeLVO
	XREF	_IDtoStrLVO

        XREF    ENDCODE

;---------------------------------------------------------------------------

        XDEF    LibInit
        XDEF    LibOpen
        XDEF    LibClose
        XDEF    LibExpunge
        XDEF	LibReserved

;---------------------------------------------------------------------------

; First executable location, must return an error to the caller
Start:
        moveq   #-1,d0
        rts

;-----------------------------------------------------------------------

ROMTAG:
        DC.W    RTC_MATCHWORD           ; UWORD RT_MATCHWORD
        DC.L    ROMTAG                  ; APTR  RT_MATCHTAG
        DC.L    ENDCODE                 ; APTR  RT_ENDSKIP
        DC.B    RTF_AUTOINIT            ; UBYTE RT_FLAGS
        DC.B    VERSION                 ; UBYTE RT_VERSION
        DC.B    NT_LIBRARY              ; UBYTE RT_TYPE
        DC.B    0                       ; BYTE  RT_PRI
        DC.L    LibName                 ; APTR  RT_NAME
        DC.L    LibId                   ; APTR  RT_IDSTRING
        DC.L    LibInitTable            ; APTR  RT_INIT

LibName DC.B 'iffparse.library',0
LibId   VSTRING

        CNOP    0,2

LibInitTable:
        DC.L    IFFParseLib_SIZEOF
        DC.L    LibFuncTable
        DC.L    0
        DC.L    LibInit

V_DEF	MACRO
	DC.W	\1+(*-LibFuncTable)
	ENDM

LibFuncTable:
	DC.W	-1
        V_DEF	LibOpen
        V_DEF	LibClose
        V_DEF	LibExpunge
        V_DEF	LibReserved

	V_DEF	_AllocIFFLVO
	V_DEF	_OpenIFFLVO
	V_DEF	_ParseIFFLVO
	V_DEF	_CloseIFFLVO
	V_DEF	_FreeIFFLVO
	V_DEF	_ReadChunkBytesLVO
	V_DEF	_WriteChunkBytesLVO
	V_DEF	_ReadChunkRecordsLVO
	V_DEF	_WriteChunkRecordsLVO
	V_DEF	_PushChunkLVO
	V_DEF	_PopChunkLVO
	V_DEF	LibReserved
	V_DEF	_EntryHandlerLVO
	V_DEF	_ExitHandlerLVO
	V_DEF	_PropChunkLVO
	V_DEF	_PropChunksLVO
	V_DEF	_StopChunkLVO
	V_DEF	_StopChunksLVO
	V_DEF	_CollectionChunkLVO
	V_DEF	_CollectionChunksLVO
	V_DEF	_StopOnExitLVO
	V_DEF	_FindPropLVO
	V_DEF	_FindCollectionLVO
	V_DEF	_FindPropContextLVO
	V_DEF	_CurrentChunkLVO
	V_DEF	_ParentChunkLVO
	V_DEF	_AllocLocalItemLVO
	V_DEF	_LocalItemDataLVO
	V_DEF	_SetLocalItemPurgeLVO
	V_DEF	_FreeLocalItemLVO
	V_DEF	_FindLocalItemLVO
	V_DEF	_StoreLocalItemLVO
	V_DEF	_StoreItemInContextLVO
	V_DEF	_InitIFFLVO
	V_DEF	_InitIFFasDOSLVO
	V_DEF	_InitIFFasClipLVO
	V_DEF	_OpenClipboardLVO
	V_DEF	_CloseClipboardLVO
	V_DEF	_GoodIDLVO
	V_DEF	_GoodTypeLVO
	V_DEF	_IDtoStrLVO

        DC.W   -1

;-----------------------------------------------------------------------

; Library Init entry point called when library is first loaded in memory
; On entry, D0 points to library base, A0 has lib seglist, A6 has SysBase
; Returns 0 for failure or the library base for success.
LibInit:
        movem.l a0/a5/a6/d7,-(sp)
        move.l  d0,a5
        move.l  a6,ipb_SysBase(a5)
        move.l  a0,ipb_SegList(a5)

	move.w	#REVISION,LIB_REVISION(a5)

        moveq   #LIBVERSION,d0
        lea     DOSName(pc),a1
        CALL    OpenLibrary
        move.l  d0,ipb_DOSBase(a5)
        beq.s	FailInit

        move.l  a5,d0
        movem.l (sp)+,a0/a5/a6/d7
        rts

FailInit:
        or.l    #AG_OpenLib|AO_DOSLib,d7
        CALL	Alert
        movem.l (sp)+,a0/a5/a6/d7
        moveq   #0,d0
        rts

LIBVERSION    EQU  37
DOSName       DC.B "dos.library",0

        CNOP    0,2

;-----------------------------------------------------------------------

; Library open entry point called every OpenLibrary()
; On entry, A6 has IFFParseBase, task switching is disabled
; Returns 0 for failure, or IFFParseBase for success.
LibOpen:
        addq.w  #1,LIB_OPENCNT(a6)
        bclr    #LIBB_DELEXP,LIB_FLAGS(a6)
        move.l  a6,d0
        rts

;-----------------------------------------------------------------------

; Library close entry point called every CloseLibrary()
; On entry, A6 has IFFParseBase, task switching is disabled
; Returns 0 normally, or the library seglist when lib should be expunged
;   due to delayed expunge bit being set
LibClose:
	subq.w	#1,LIB_OPENCNT(a6)

	; if delayed expunge bit set, then try to get rid of the library
	btst	#LIBB_DELEXP,LIB_FLAGS(a6)
	bne.s	CloseExpunge

	; delayed expunge not set, so stick around
	moveq	#0,d0
	rts

CloseExpunge:
	; if no library users, then just remove the library
	tst.w	LIB_OPENCNT(a6)
	beq.s	DoExpunge

	; still some library users, so forget about flushing
	bclr	#LIBB_DELEXP,LIB_FLAGS(a6)
	moveq	#0,d0
	rts

;-----------------------------------------------------------------------

; Library expunge entry point called whenever system memory is lacking
; On entry, A6 has IFFParseBase, task switching is disabled
; Returns the library seglist if the library open count is 0, returns 0
; otherwise and sets the delayed expunge bit.
LibExpunge:
        tst.w   LIB_OPENCNT(a6)
        beq.s   DoExpunge

        bset    #LIBB_DELEXP,LIB_FLAGS(a6)
        moveq   #0,d0
        rts

DoExpunge:
        movem.l d2/a5/a6,-(sp)
        move.l  a6,a5
        move.l  ipb_SegList(a5),d2

        move.l  a5,a1
        REMOVE

        move.l  ipb_SysBase(a5),a6
        move.l  ipb_DOSBase(a5),a1
        CALL    CloseLibrary

        move.l  a5,a1
        moveq   #0,d0
        move.w  LIB_NEGSIZE(a5),d0
        sub.l   d0,a1
        add.w   LIB_POSSIZE(a5),d0
        CALL    FreeMem

        move.l  d2,d0
        movem.l (sp)+,d2/a5/a6
        rts

;-----------------------------------------------------------------------

LibReserved:
        moveq   #0,d0
        rts

;-----------------------------------------------------------------------

        END
