	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE	"exec/initializers.i"
	INCLUDE	"exec/resident.i"
	INCLUDE "internal/librarytags.i"

	INCLUDE	"gadtools_rev.i"
	INCLUDE	"gtinternal.i"

	LIST

;---------------------------------------------------------------------------

	XREF	_LIB_CreateGadgetA
	XREF	_LIB_FreeGadgets
	XREF	_LIB_GT_SetGadgetAttrsA
	XREF	_LIB_CreateMenusA
	XREF	_LIB_FreeMenus
	XREF	_LIB_LayoutMenuItemsA
	XREF	_LIB_LayoutMenusA
	XREF	_LIB_GT_GetIMsg
	XREF	_LIB_GT_ReplyIMsg
	XREF	_LIB_GT_RefreshWindow
	XREF	_LIB_GT_BeginRefresh
	XREF	_LIB_GT_EndRefresh
	XREF	_LIB_GT_FilterIMsg
	XREF	_LIB_GT_PostFilterIMsg
	XREF	_LIB_CreateContext
	XREF	_LIB_DrawBevelBoxA
	XREF	_LIB_GetVisualInfoA
	XREF	_LIB_FreeVisualInfo
	XREF	_LIB_GT_GetGadgetAttrsA

	XREF	_DispatchPalette
	XREF	@initGTButtonIClass

	XREF	_LVOSDivMod32
	XREF	_LVOUDivMod32
	XREF	_LVOSMult32
	XREF	_LVOUMult32

	XREF	_LVOInitSemaphore
	XREF	_LVOTaggedOpenLibrary


	XREF	endtag

;---------------------------------------------------------------------------

	XDEF	LibInit
	XDEF	LibOpen
	XDEF	LibClose
	XDEF	LibExpunge
	XDEF	LibReserved
	XDEF	_callCHook

;---------------------------------------------------------------------------

ROMTAG:
	DC.W	RTC_MATCHWORD		; UWORD RT_MATCHWORD
	DC.L	ROMTAG			; APTR  RT_MATCHTAG
	DC.L	endtag			; APTR  RT_ENDSKIP
	DC.B	RTF_AUTOINIT		; UBYTE RT_FLAGS
	DC.B	VERSION			; UBYTE RT_VERSION
	DC.B	NT_LIBRARY		; UBYTE RT_TYPE
	DC.B	-120			; BYTE  RT_PRI
	DC.L	LibName			; APTR  RT_NAME
	DC.L	LibId			; APTR  RT_IDSTRING
	DC.L	LibInitTable		; APTR  RT_INIT

LibName	DC.B	'gadtools.library',0
LibId	VSTRING

	CNOP	0,2

LibInitTable:
	DC.L	GadToolsLib_SIZEOF
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

	V_DEF	_LIB_CreateGadgetA
	V_DEF	_LIB_FreeGadgets
	V_DEF	_LIB_GT_SetGadgetAttrsA
	V_DEF	_LIB_CreateMenusA
	V_DEF	_LIB_FreeMenus
	V_DEF	_LIB_LayoutMenuItemsA
	V_DEF	_LIB_LayoutMenusA
	V_DEF	_LIB_GT_GetIMsg
	V_DEF	_LIB_GT_ReplyIMsg
	V_DEF	_LIB_GT_RefreshWindow
	V_DEF	_LIB_GT_BeginRefresh
	V_DEF	_LIB_GT_EndRefresh
	V_DEF	_LIB_GT_FilterIMsg
	V_DEF	_LIB_GT_PostFilterIMsg
	V_DEF	_LIB_CreateContext
	V_DEF	_LIB_DrawBevelBoxA
	V_DEF	_LIB_GetVisualInfoA
	V_DEF	_LIB_FreeVisualInfo
	V_DEF	LibReserved
	V_DEF	LibReserved
	V_DEF	LibReserved
	V_DEF	LibReserved
	V_DEF	LibReserved
	V_DEF	LibReserved

*
* V39 functions begin here
*

	V_DEF	_LIB_GT_GetGadgetAttrsA

	DC.W	-1

;-----------------------------------------------------------------------

; Library Init entry point called when library is first loaded in memory
; On entry, D0 points to library base, A0 has lib seglist, A6 has SysBase
; Returns 0 for failure or the library base for success.

; We're too lazy to check for failure, since you'd be hosed anyways.

LibInit:
	movem.l	d0/a3,-(sp)
	move.l	d0,a3
	move.l	a6,gtb_SysBase(a3)

	move.w	#REVISION,LIB_REVISION(a3)

	moveq	#OLTAG_UTILITY,d0
	bsr.s	OpenLib
	move.l	d0,a0
	move.l	a0,gtb_UtilityBase(a3)

	; a0 has UtilityBase
	; a3 has own library base (GadToolsBase in this case)
	; d0, a1 and a0 get modified

	lea	_LVOSMult32(a0),a0	; get address of first jump vector
	lea	gtb_SMult32(a3),a1	; load target address of first vector
	moveq	#3,d0
1$:
	move.l	a0,(a1)+
	subq.l	#6,a0
	dbra	d0,1$

	moveq	#OLTAG_GRAPHICS,d0
	bsr.s	OpenLib
	move.l	d0,gtb_GfxBase(a3)

	moveq	#OLTAG_LAYERS,d0
	bsr.s	OpenLib
	move.l	d0,gtb_LayersBase(a3)

	moveq	#OLTAG_INTUITION,d0
	bsr.s	OpenLib
	move.l	d0,gtb_IntuitionBase(a3)

	exg	a3,a6
	bsr	@initGTButtonIClass
	move.l	d0,gtb_GTButtonIClass(a6)

	lea	_DispatchPalette,a0
	move.l	a0,gtb_PaletteGHook+h_SubEntry(a6)
	lea	_callCHook(pc),a0
	move.l	a0,gtb_PaletteGHook+h_Entry(a6)
	move.l	a6,gtb_PaletteGHook+h_Data(a6)

	exg	a3,a6

	movem.l	(sp)+,d0/a3
	rts

OpenLib:
	jmp	_LVOTaggedOpenLibrary(a6)

	CNOP	0,2

;-----------------------------------------------------------------------

; Library open entry point called every OpenLibrary()
; On entry, A6 has GadToolsBase, task switching is disabled
; Returns 0 for failure, or GadToolsBase for success.
LibOpen:
	addq.w	#1,LIB_OPENCNT(a6)
	move.l	a6,d0
	rts

;-----------------------------------------------------------------------

; Library close entry point called every CloseLibrary()
; On entry, A6 has GadToolsBase, task switching is disabled
; Returns 0 normally, or the library seglist when lib should be expunged
;   due to delayed expunge bit being set, which won't happen for a ROM
;   module
LibClose:

	subq.w	#1,LIB_OPENCNT(a6)

*** FALLS THROUGH !!!

;-----------------------------------------------------------------------

; Library expunge entry point called whenever system memory is lacking
; On entry, A6 has GadToolsBase, task switching is disabled
; Returns the library seglist if the library open count is 0, returns 0
; otherwise and sets the delayed expunge bit. Won't happen since this is
; a ROM module

LibExpunge:

*** FALLS THROUGH !!!

;-----------------------------------------------------------------------

LibReserved:
	moveq	#0,d0
	rts

;-----------------------------------------------------------------------

* Restores context and calls high-level language hook

_callCHook:
	movem.l	a5/a6,-(sp)
	movem.l	h_SubEntry(a0),a5/a6	; h_SubEntry into A5, h_Data into A6
	jsr	(a5)			; call HLL
	movem.l	(sp)+,a5/a6
	rts

;-----------------------------------------------------------------------

	END
