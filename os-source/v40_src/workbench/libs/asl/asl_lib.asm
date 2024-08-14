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

        INCLUDE "asl_rev.i"
        INCLUDE "aslbase.i"
        INCLUDE "asl.i"

        LIST

;---------------------------------------------------------------------------

        XREF    _AslRequest
        XREF    _AllocAslRequest
        XREF    _FreeAslRequest
        XREF	_FlushLib

	XREF	_LVOSMult32
	XREF    _LVOUMult32
	XREF	_LVOSDivMod32
	XREF	_LVOUDivMod32

        XREF    ENDCODE

;---------------------------------------------------------------------------

        XDEF    LibInit
        XDEF    LibOpen
        XDEF    LibClose
        XDEF    LibExpunge
        XDEF    LibReserved
        XDEF    ObsAllocFileRequest
        XDEF    ObsFreeFileRequest
        XDEF    ObsRequestFile

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

LibName DC.B 'asl.library',0
LibId   VSTRING

        CNOP    0,4

LibInitTable:
        DC.L    ASLLib_SIZEOF
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

        V_DEF	ObsAllocFileRequest
        V_DEF	ObsFreeFileRequest
        V_DEF	ObsRequestFile

        V_DEF	_AllocAslRequest
        V_DEF	_FreeAslRequest
        V_DEF	_AslRequest

        DC.W   -1

;-----------------------------------------------------------------------

; Library Init entry point called when library is first loaded in memory
; On entry, D0 points to library base, A0 has lib seglist, A6 has SysBase
; Returns 0 for failure or the library base for success.
LibInit:
        movem.l a0/a5/a6/d7,-(sp)
        move.l  d0,a5
        move.l  a6,ASL_SysBase(a5)
        move.l  a0,ASL_SegList(a5)

        move.w	#REVISION,LIB_REVISION(a5)

        move.l  #AO_DOSLib,d7
        lea     DOSName(pc),a1
        bsr	OpenLib
        move.l  d0,ASL_DosBase(a5)

        move.l  #AO_UtilityLib,d7
        lea     UtilityName(pc),a1
        bsr	OpenLib
        move.l  d0,ASL_UtilityBase(a5)

; now initialize the four math function pointers
	move.l	d0,a1			; load UtilityBase
	lea	_LVOSMult32(a1),a1	; get address of first jump vector
	lea	ASL_SMult32(a5),a0	; load target address of first vector
	moveq	#3,d0
1$:
	move.l	a1,(a0)+
	subq.l	#6,a1
	dbra	d0,1$

        move.l  #AO_Intuition,d7
        lea     IntuitionName(pc),a1
        bsr.s	OpenLib
        move.l  d0,ASL_IntuitionBase(a5)

        move.l  #AO_GraphicsLib,d7
        lea     GfxName(pc),a1
        bsr.s	OpenLib
        move.l  d0,ASL_GfxBase(a5)

        move.l  #AO_LayersLib,d7
        lea     LayersName(pc),a1
        bsr.s	OpenLib
        move.l  d0,ASL_LayersBase(a5)

        move.l  #AO_GadTools,d7
        lea     GadToolsName(pc),a1
        bsr.s	OpenLib
        move.l  d0,ASL_GadToolsBase(a5)

        move.l  #AO_IconLib,d7
        lea     IconName(pc),a1
        bsr.s	OpenLib
        move.l  d0,ASL_IconBase(a5)

	lea	ASL_FontCacheLock(a5),a0
	CALL	InitSemaphore
	lea	ASL_FontCache(a5),a0
	NEWLIST	a0

	move.l	ASL_GfxBase(a5),a6
	lea	TopazAttr(pc),a0
	move.l	a0,ASL_FTextAttr(a5)
	CALL	OpenFont
	move.l	d0,ASL_FFont(a5)

        move.l  a5,d0
        movem.l (sp)+,a0/a5/a6/d7
        rts

OpenLib:
        moveq   #LIBVERSION,d0
        CALL    OpenLibrary
        move.l	(sp)+,a0	; pop return address
        tst.l   d0		; did lib open?
        beq.s   FailInit	; nope, so exit
        jmp	(a0)		; yes, so return

FailInit:
        bsr     CloseLibs
        or.l    #AG_OpenLib,d7
        CALL	Alert
        movem.l (sp)+,a0/a5/a6/d7
        moveq   #0,d0
        rts

TopazAttr     DC.L TopazName	; ta_Name
	      DC.W 8		; ta_YSize
	      DC.B 0		; ta_Style
	      DC.B FPF_ROMFONT	; ta_Flags
TopazName     DC.B "topaz.font",0

LIBVERSION    EQU  39
DOSName       DC.B "dos.library",0
IntuitionName DC.B "intuition.library",0
GfxName       DC.B "graphics.library",0
UtilityName   DC.B "utility.library",0
GadToolsName  DC.B "gadtools.library",0
LayersName    DC.B "layers.library",0
IconName      DC.B "icon.library",0

        CNOP    0,2

;-----------------------------------------------------------------------

; Library open entry point called every OpenLibrary()
; On entry, A6 has AslBase, task switching is disabled
; Returns 0 for failure, or AslBase for success.
LibOpen:
        addq.w  #1,ASL_UsageCnt(a6)
        bclr    #LIBB_DELEXP,LIB_FLAGS(a6)
        move.l  a6,d0
        rts

;-----------------------------------------------------------------------

; Library close entry point called every CloseLibrary()
; On entry, A6 has AslBase, task switching is disabled
; Returns 0 normally, or the library seglist when lib should be expunged
;   due to delayed expunge bit being set
LibClose:
	subq.w	#1,ASL_UsageCnt(a6)

	; if delayed expunge bit set, then try to get rid of the library
	btst	#LIBB_DELEXP,LIB_FLAGS(a6)
	bne.s	CloseExpunge

	; delayed expunge not set, so stick around
	moveq	#0,d0
	rts

CloseExpunge:
	; if no library users, then just remove the library
	tst.w	ASL_UsageCnt(a6)
	beq.s	DoExpunge

	; still some library users, so just flush unused resources
	bsr	_FlushLib
	bclr	#LIBB_DELEXP,LIB_FLAGS(a6)
	moveq	#0,d0
	rts

;-----------------------------------------------------------------------

; Library expunge entry point called whenever system memory is lacking
; On entry, A6 has AslBase, task switching is disabled
; Returns the library seglist if the library open count is 0, returns 0
; otherwise and sets the delayed expunge bit.
LibExpunge:
	bsr	_FlushLib

        tst.w   ASL_UsageCnt(a6)
        beq.s   DoExpunge

        bset    #LIBB_DELEXP,LIB_FLAGS(a6)
        moveq   #0,d0
        rts

DoExpunge:
        movem.l d2/a5/a6,-(sp)
        move.l  a6,a5
        move.l  ASL_SegList(a5),d2

        move.l  a5,a1
        REMOVE

	move.l	ASL_GfxBase(a5),a6
	move.l	ASL_FFont(a5),a1
	CALL	CloseFont

        move.l  ASL_SysBase(a5),a6
        bsr.s   CloseLibs

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

CloseLibs:
        move.l  ASL_DosBase(a5),a1
        CALL    CloseLibrary

        move.l  ASL_UtilityBase(a5),a1
        CALL    CloseLibrary

        move.l  ASL_IntuitionBase(a5),a1
        CALL    CloseLibrary

        move.l  ASL_LayersBase(a5),a1
        CALL    CloseLibrary

        move.l  ASL_GfxBase(a5),a1
        CALL    CloseLibrary

        move.l  ASL_GadToolsBase(a5),a1
        CALL    CloseLibrary

        move.l  ASL_IconBase(a5),a1
        GO      CloseLibrary

;-----------------------------------------------------------------------

ObsAllocFileRequest:
        moveq.l #ASL_FileRequest,d0
        suba.l  a0,a0
        jmp     _LVOAllocAslRequest(a6)

;-----------------------------------------------------------------------

ObsFreeFileRequest:
        jmp     _LVOFreeAslRequest(a6)

;-----------------------------------------------------------------------

ObsRequestFile:
        suba.l  a1,a1
        jmp     _LVOAslRequest(a6)

;-----------------------------------------------------------------------

        END
