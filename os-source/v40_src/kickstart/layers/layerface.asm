*******************************************************************************
*
*	$Id: layerface.asm,v 38.16 93/02/15 16:14:04 mks Exp $
*
*******************************************************************************
* layer asm to c interface file

	include 'exec/types.i'
	include 'exec/nodes.i'
	include 'exec/lists.i'
	include 'exec/libraries.i'
	include 'exec/initializers.i'
	include	'exec/resident.i'
	include 'exec/alerts.i'
	include	'exec/execbase.i'

	include	'internal/librarytags.i'

	include	'graphics/gfxbase.i'

	include	"layers_rev.i"

	include "layersbase.i"
*
*******************************************************************************
*
* ROMTAG is now in assembly...
*
	xref	EndCode
initDDescrip:					;STRUCTURE RT,0
	dc.w	RTC_MATCHWORD			; UWORD RT_MATCHWORD
	dc.l	initDDescrip			; APTR  RT_MATCHTAG
	dc.l	EndCode				; APTR  RT_ENDSKIP
	dc.b	RTW_COLDSTART!RTF_AUTOINIT	; UBYTE RT_FLAGS
	dc.b	VERSION				; UBYTE RT_VERSION
	dc.b	NT_LIBRARY			; UBYTE RT_TYPE
	dc.b	64				; BYTE  RT_PRI
	dc.l	subsysName			; APTR  RT_NAME
	dc.l	VERSTRING			; APTR  RT_IDSTRING
	dc.l	Init				; APTR  RT_INIT

Init:
	dc.l	lb_SIZEOF
	dc.l	funcTable
	dc.l	dataTable
	dc.l	initRoutine

dataTable:
	INITBYTE	LN_TYPE,NT_LIBRARY
	INITLONG	LN_NAME,subsysName
	INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
	INITWORD	LIB_VERSION,VERNUM
	INITWORD	LIB_REVISION,REVNUM
	INITLONG	LIB_IDSTRING,VERSTRING
	dc.l		0
*
*******************************************************************************
*
	xref	LockLayer
	xref	UnlockLayer
	xref	LockLayerInfo
	xref	UnlockLayerInfo
	xref	WhichLayer
	xref	SortLayerCR
	xref	DoHookClipRects
*
* Build the function table (short size)
*
V_DEF		MACRO
		dc.W	\1+(*-funcTable)
		ENDM

funcTable:
	dc.w	-1		* means interpret as relative offsets
	V_DEF	OpenLayerLib
	V_DEF	CloseLayerLib
	V_DEF	ExpungeLayerLib
	V_DEF	ExtFuncLayerLib
	V_DEF	InitLayers
	V_DEF	CreateUpfrontLayer
	V_DEF	CreateBehindLayer
	V_DEF	UpfrontLayer
	V_DEF	BehindLayer
	V_DEF	MoveLayer
	V_DEF	SizeLayer
	V_DEF	ScrollLayer
	V_DEF	BeginUpdate
	V_DEF	EndUpdate
	V_DEF	DeleteLayer
	V_DEF	LockLayer
	V_DEF	UnlockLayer
	V_DEF	LockLayers
	V_DEF	UnlockLayers
	V_DEF	LockLayerInfo
	V_DEF	SwapBitsRastPortClipRect
	V_DEF	WhichLayer
	V_DEF	UnlockLayerInfo
	V_DEF	NewLayerInfo
	V_DEF	DisposeLayerInfo
	V_DEF	FattenLayerInfo
	V_DEF	ThinLayerInfo
	V_DEF	MoveLayerInFrontOf
	V_DEF	InstallClipRegion
	V_DEF	MoveSizeLayer
	V_DEF	CreateUpfrontHookLayer
	V_DEF	CreateBehindHookLayer
	V_DEF	InstallLayerHook
	V_DEF	InstallLayerInfoHook
	V_DEF	SortLayerCR
	V_DEF	DoHookClipRects
	dc.w	-1
*
*******************************************************************************
*
* The required library calls.  They don't do much...
*
OpenLayerLib:	move.l	a6,d0		; Return the library base
		rts
CloseLayerLib:
ExpungeLayerLib:
ExtFuncLayerLib:
		moveq.l	#0,d0		; Return NULL
		rts
*
*******************************************************************************
*
* The library init routine.
*
		xref	_LVOAlert
initRoutine:	move.l	a2,-(sp)		; We will need this...
		move.l	d0,a2			; Set up layersbase in a2...
		move.w	#1,LIB_OPENCNT(a2)	; We are always open...
		move.l	a6,lb_ExecBase(a2)	; Store ExecBase
		lea	gfxName(pc),a1		; Get name of graphics...
		lea	LibList(a6),a0		; Library list
		CALLSYS	FindName		; Fine it...
		move.l	d0,lb_GfxBase(a2)	; Store the vector...
		beq.s	bad_init		; We did not get it???!!!
		move.l	d0,a0			; Get graphicsbase...
		move.l	a2,gb_LayersBase(a0)	; Store my base into GfxBase...

		moveq.l	#OLTAG_UTILITY,d0	; Get tag...
		CALLSYS	TaggedOpenLibrary	; Open it up...
		move.l	d0,lb_UtilityBase(a2)	; Store the vector...
		beq.s	bad_util		; We did not get it????!!!!
;
; All initialized!
;
end_init:	move.l	a2,d0			; Return library base
		move.l	(sp)+,a2		; restore a2
		rts
*
* Clear the library base and return.  We are dead at this point!
*
bad_util:	;ALERT	AN_LayersLib!AG_OpenLib!AO_UtilityLib
		;bra.s	bad_init
bad_gfx:	;ALERT	AN_LayersLib!AG_OpenLib!AO_GraphicsLib
bad_init:	sub.l	a2,a2			; Clear a2
		bra.s	end_init
*
*******************************************************************************
*
* __CXD33 replacement that uses utility.library
*
;		XDEF	__CXD33			; Signed divide
;__CXD33:	move.l	a6,-(sp)		; Save your base pointer
;		move.l	lb_UtilityBase(a6),a6	; Get utility base pointer
;		CALLSYS	SDivMod32		; Do the divide
;		bra.s	RestoreReturn		; Restore a6 and return...
*
*******************************************************************************
*
* __CXD22 replacement that uses utility.library
*
;		XDEF	__CXD22			; Unsigned divide
;__CXD22:	move.l	a6,-(sp)		; Save your base pointer
;		move.l	lb_UtilityBase(a6),a6	; Get utility base pointer
;		CALLSYS	UDivMod32		; Do the divide
;		bra.s	RestoreReturn		; Restore a6 and return...
*
*******************************************************************************
*
* __CXM33 replacement that uses utility.library
*
		XDEF	__CXM33			; Signed multiply
__CXM33:	move.l	a6,-(sp)		; Save your base pointer
		move.l	lb_UtilityBase(a6),a6	; Get utility base pointer
		CALLSYS	SMult32			; Do the multiply
RestoreReturn:	move.l	(sp)+,a6		; Restore LayersBase
		rts
*
*******************************************************************************
*
* __CXM22 replacement that uses utility.library
*
;		XDEF	__CXM22			; Unsigned multiply
;__CXM22:	move.l	a6,-(sp)		; Save your base pointer
;		move.l	lb_UtilityBase(a6),a6	; Get utility base pointer
;		CALLSYS	UMult32			; Do the multply
;		bra.s	RestoreReturn		; Restore a6 and return...
*
*******************************************************************************
*
		xdef	_NewList
_NewList:	NEWLIST	a1
		rts
*
*******************************************************************************
*
*
*******************************************************************************
*
			xref	_createbehindlayer
CreateBehindHookLayer:
	move.l	a3,-(sp)		; client-specified hook
cb_no:	move.l	a2,-(sp)		; bm2
	move.l	d4,-(sp)		; flags
	movem.w	d0/d1/d2/d3,-(sp)	; struct Rectangle
	movem.l	a0/a1,-(sp)
	bsr	_createbehindlayer
	lea	7*4(sp),sp
	rts
CreateBehindLayer:
	move.l	#0,-(sp)		; default hook
	bra.s	cb_no
*
*******************************************************************************
*
			xref	_createupfrontlayer
CreateUpfrontHookLayer:
	move.l	a3,-(sp)		; client-specified hook
cu_no:	move.l	a2,-(sp)		; bm2
	move.l	d4,-(sp)		; flags
	movem.w	d0/d1/d2/d3,-(sp)	; struct Rectangle
	movem.l	a0/a1,-(sp)
	bsr	_createupfrontlayer
	lea	7*4(sp),sp
	rts
CreateUpfrontLayer:
	move.l	#0,-(sp)		; default hook
	bra.s	cu_no
*
*******************************************************************************
*
		xref	_initlayers
InitLayers:	equ	_initlayers
*
*******************************************************************************
*
		xref	_NewLayerInfo
NewLayerInfo:	equ	_NewLayerInfo
*
*******************************************************************************
*
		xref	_upfront
UpfrontLayer:	equ	_upfront
*
*******************************************************************************
*
		xref	_behind
BehindLayer:	equ	_behind
*
*******************************************************************************
*
MoveLayer:
	move.l	a1,a0		; Move layer pointer to a0
	clr.l	-(sp)		; dx/dy
	move.w	d1,-(sp)	; y
	move.w	d0,-(sp)	; x
	bra.s	do_movesize
*
*******************************************************************************
*
SizeLayer:
	move.l	a1,a0		; Move layer pointer to a0
	move.w	d1,-(sp)	; dy
	move.w	d0,-(sp)	; dx
	clr.l	-(sp)		; x/y
	bra.s	do_movesize
*
*******************************************************************************
*
* We have this here so we can pass in the structure pointer...
*
MoveSizeLayer:	xref	_movesizelayer
	movem.w	d0/d1/d2/d3,-(sp)	; struct Rectangle
do_movesize:
	move.l	sp,a1
	bsr	_movesizelayer
	addq.l	#8,sp			; adjust stack...
	rts
*
*******************************************************************************
*
		xref	_scrolllayer
ScrollLayer:	equ	_scrolllayer
*
*******************************************************************************
*
		xref	_beginupdate
BeginUpdate:	equ	_beginupdate
*
*******************************************************************************
*
		xref	_endupdate
EndUpdate:	equ	_endupdate
*
*******************************************************************************
*
			xref	_installclipregion
InstallClipRegion:	equ	_installclipregion
*
*******************************************************************************
*
		xref	_deletelayer
DeleteLayer:	equ	_deletelayer
*
*******************************************************************************
*
		xref	_unlocklayers
UnlockLayers:	equ	_unlocklayers
*
*******************************************************************************
*
		xref	_locklayers
LockLayers:	equ	_locklayers
*
*******************************************************************************
*
				xref	_screenswap
SwapBitsRastPortClipRect:	equ	_screenswap
*
*******************************************************************************
*
			xref	_disposelayerinfo
DisposeLayerInfo:	equ	_disposelayerinfo
*
*******************************************************************************
*
			xref	_extern_fattenlayerinfo
FattenLayerInfo:	equ	_extern_fattenlayerinfo
*
*******************************************************************************
*
		xref	_extern_thinlayerinfo
ThinLayerInfo:	equ	_extern_thinlayerinfo
*
*******************************************************************************
*
			xref	_extern_movelayerinfrontof
MoveLayerInFrontOf:	equ	_extern_movelayerinfrontof
*
*******************************************************************************
*
			xref	_install_backfill_hook
InstallLayerHook:	equ	_install_backfill_hook
*
*******************************************************************************
*
			xref	_InstallLayerInfoHook
InstallLayerInfoHook:	equ	_InstallLayerInfoHook
*
*******************************************************************************
*
* The library name and version strings...
*
subsysName:	dc.b	'layers.library',0
VERSTRING:	VSTRING
gfxName:	dc.b	'graphics.library',0
VERNUM:		EQU	VERSION
REVNUM:		EQU	REVISION
*
*******************************************************************************
	end

