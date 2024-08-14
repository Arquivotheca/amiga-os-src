********************************************************************************
*
*	$Id: specialfx.library.asm,v 39.14 93/09/23 17:28:29 spence Exp $
*
********************************************************************************

	section	code

	include	'exec/types.i'
	include 'exec/initializers.i'
	include	'exec/libraries.i'
	include	'exec/lists.i'
	include	'exec/resident.i'
	include	'exec/memory.i'
	include	'utility/utility.i'
	include	'graphics/gfxbase.i'

	include	"SpecialFXBase.i"
	include	"specialfx.i"
	include	"specialfx_internal.i"
	include "SpecialFX_rev.i"

	xref	_LVOOpenLibrary
	xref	_LVOCloseLibrary
	xref	_LVORemove
	xref	_LVOFreeMem
	xref	_LVOAllocMem
	xref	_LVOInitSemaphore
	xref	_LVOSetFunction
	xref	_LVOMrgCop
	xref	_LVOLoadView

	xref	_LVOAllocFX
	xref	_LVOFreeFX
	xref	_LVOInstallFXA
	xref	_LVORemoveFX
	xref	_LVOAnimateFX
	xref	_LVOFindVP
	xref	EndCode
	xref	SfxBase
	xref	SFXMrgCop
	xref	SFXLoadView

Start:
	moveq	#-1,d0		; safety check for daft buggers that try to execute the library
	rts

MYPRI	equ	0

RomTag:
	dc.w    RTC_MATCHWORD      ; UWORD RT_MATCHWORD
	dc.l    RomTag             ; APTR  RT_MATCHTAG
	dc.l    EndCode            ; APTR  RT_ENDSKIP
	dc.b    RTF_AUTOINIT       ; UBYTE RT_FLAGS
	dc.b    VERSION            ; UBYTE RT_VERSION
	dc.b    NT_LIBRARY         ; UBYTE RT_TYPE
	dc.b    MYPRI              ; BYTE  RT_PRI
	dc.l    LibName            ; APTR  RT_NAME
	dc.l    IDString           ; APTR  RT_IDSTRING
	dc.l    InitTable          ; APTR  RT_INIT  table for InitResident()

LibName:	SPECIALFXNAME
IDString:	VSTRING
GfxName:	GRAPHICSNAME
UtilityName:	UTILITYNAME

	; force LONG alignment
	ds.l	0

InitTable:
	dc.l   SpecialFXBase_SIZEOF ; size of library base data space
	dc.l   funcTable         ; pointer to function initializers
	dc.l   dataTable         ; pointer to data initializers
	dc.l   initRoutine       ; routine to run


funcTable:
   ;------ standard system routines
	dc.l   Open
	dc.l   Close
	dc.l   Expunge
	dc.l   Null

   ;------ my libraries definitions

	dc.l	_LVOAllocFX
	dc.l	_LVOFreeFX
	dc.l	_LVOInstallFXA
	dc.l	_LVORemoveFX
	dc.l	_LVOAnimateFX
	dc.l	_LVOFindVP

   ;------ function table end marker
	dc.l   -1

dataTable:
        INITBYTE        LN_TYPE,NT_LIBRARY
        INITLONG        LN_NAME,LibName
        INITBYTE        LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
        INITWORD        LIB_VERSION,VERSION
        INITWORD        LIB_REVISION,REVISION
        INITLONG        LIB_IDSTRING,IDString
        dc.l   0

*******************************************************************************

* Here are the lists of vectors for each FX type.

* Table of pointers to functions to allocate the FXHandles for each FX type.
*
* Each function takes:
* d1 = Number of control structures to allocate
* a2 -> Error variable
* a3 -> ViewPort
* a5 = a6 -> SFXBase
* Returns
* d0 -> FXHandle

	xref	AllocColours
	xref	AllocLineControl
	xref	AllocIntControl
	xref	AllocVideoControl
	xref	AllocSpriteControl

AllocTable:
	dc.l	AllocColours
	dc.l	AllocLineControl
	dc.l	AllocIntControl
	dc.l	AllocVideoControl
	dc.l	AllocSpriteControl


* Table of pointers to functions to count the number of copper instructions
* needed by each FX type. Each function allocates the UCopList and adds it
* to the ViewPort.
*
* Each function takes:
* a0 -> FxHandle
* a4 -> GfxBase
* a5 -> StackStuff
* a6 -> UtilityBase

	xref	CountColours
	xref	CountLineControl
	xref	CountIntControl
	xref	CountVideoControl
	xref	CountSpriteControl

CountTable:
	dc.l	CountColours
	dc.l	CountLineControl
	dc.l	CountIntControl
	dc.l	CountVideoControl
	dc.l	CountSpriteControl


* Table of pointers to functions to build the UserCopperlists for each
* tag type.
*
* Each function takes:
* a0 -> FxHandle
* a4 -> GfxBase
* a5 -> StackStuff
* a6 -> UtilityBase

	xref	InstallColours
	xref	InstallLineControl
	xref	InstallIntControl
	xref	InstallVideoControl
	xref	InstallSpriteControl

InstallTable:
	dc.l	InstallColours
	dc.l	InstallLineControl
	dc.l	InstallIntControl
	dc.l	InstallVideoControl
	dc.l	InstallSpriteControl


* Table of pointers to functions related to each SpecialFX type. NULL shows
* that no special Remove code is needed.
*
* Each function takes:
* a0 -> ViewPort
* a5 -> FXHandle
* a6 -> SpecialFXBase

	xref	RemoveIntControl

RemoveTable:
	dc.l	0
	dc.l	0
	dc.l	RemoveIntControl
	dc.l	0
	dc.l	0


* Table of pointers to functions to Animate the system copperlists for each
* tag type.
*
* Each function takes:
* a3 -> FXHandle
* a5 -> AnimHandle
* a6 -> SFXBase
*
* For speed optimisation, they can all trash
* a0-a1/a3-a4/d0-d6

	xref	AnimateColours
	xref	AnimateLineControl
	xref	AnimateIntControl
	xref	AnimateVideoControl
	xref	AnimateSpriteControl

AnimateTable:
	dc.l	AnimateColours
	dc.l	AnimateLineControl
	dc.l	AnimateIntControl
	dc.l	AnimateVideoControl
	dc.l	AnimateSpriteControl


* Table of pointers to functions to rebuild the copperlist after the ViewPort
* has been coerced.
*
* Each function takes:
* d5.w = -1 if the View mode or current monitor changed.
* a0 -> FxHandle
* a2 -> SfxBase
* a4 -> AnimHandle

	xref	RebuildColours
	xref	RebuildLineControl
	xref	RebuildVideoControl

RebuildTable:
	dc.l	RebuildColours
	dc.l	RebuildLineControl
	dc.l	0
	dc.l	RebuildVideoControl
	dc.l	0

********************************************************************************

initRoutine:
	; get the library pointer into a convenient A register
	move.l	a5,-(sp)
	move.l	d0,a5

	; save a pointer to exec
	move.l	a6,sfxb_ExecBase(a5)

	; save a pointer to our loaded code
	move.l	a0,sfxb_SegList(a5)

	; open the graphics library
	lea	GfxName(pc),a1
	moveq	#37,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,sfxb_GfxBase(a5)
	beq	NoGfxLib

	; open the utility library
	lea	UtilityName(pc),a1
	moveq	#37,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,sfxb_UtilityBase(a5)
	beq	NoUtilLib

	; Allocate and initialise a Semaphore for the Copper DoubleBuffering
	moveq	#SS_SIZE,d0
	move.l	#(MEMF_CLEAR|MEMF_PUBLIC),d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,sfxb_AnimationSemaphore(a5)
	beq	NoDBuffSem
	move.l	d0,a0
	jsr	_LVOInitSemaphore(a6)

	; Allocate and initialise a Semaphore for the VPList
	moveq	#SS_SIZE,d0
	move.l	#(MEMF_CLEAR|MEMF_PUBLIC),d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,sfxb_VPListSemaphore(a5)
	beq	NoVPLSem
	move.l	d0,a0
	jsr	_LVOInitSemaphore(a6)

	; Allocate and initialise a Semaphore for the AHList
	moveq	#SS_SIZE,d0
	move.l	#(MEMF_CLEAR|MEMF_PUBLIC),d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,sfxb_AHListSemaphore(a5)
	beq	NoAHLSem
	move.l	d0,a0
	jsr	_LVOInitSemaphore(a6)

	; Initialise the list of ViewPorts using SpecialFX.library's
	; InterruptControl feature.
	; This list is used, for example, when determining which ViewPort an
	; interrupt occurred in.
	lea	sfxb_VPListHead(a5),a0
	NEWLIST	a0
	lea	sfxb_VPListHead(a5),a0
	lea	sfxb_VPList(a5),a1
	clr.w	vpl_Count(a1)			; set the count to 0
	ADDHEAD	a0,a1
	lea	sfxb_VPList+vpl_ViewPorts(a5),a0
	moveq	#MAX_VPLIST-1,d0
10$:
	clr.l	(a0)+
	dbra.s	d0,10$

	; Initialise the list of AnimHandles used by SpecialFX.library
	; This list is used to update the copperlist pointers of
	; AnimHandles after the copperlist is copied by the MrgCop() vector.
	lea	sfxb_AHListHead(a5),a0
	NEWLIST	a0
	lea	sfxb_AHListHead(a5),a0
	lea	sfxb_AHList(a5),a1
	clr.w	ahl_Count(a1)			; set the count to 0
	ADDHEAD	a0,a1
	lea	sfxb_AHList+ahl_AnimHandles(a5),a0
	moveq	#MAX_AHLIST-1,d0
20$:
	clr.l	(a0)+
	dbra.s	d0,20$

	; set up the Vectors
	move.l	#AllocTable,sfxb_AllocVectors(a5)
	move.l	#CountTable,sfxb_CountVectors(a5)
	move.l	#InstallTable,sfxb_InstallVectors(a5)
	move.l	#RemoveTable,sfxb_RemoveVectors(a5)
	move.l	#AnimateTable,sfxb_AnimateVectors(a5)
	move.l	#RebuildTable,sfxb_RebuildVectors(a5)

	; Set up the SFXMrgCop() vector.
	move.l	sfxb_GfxBase(a5),a1
	lea	SFXMrgCop(pc),a0
	move.l	a0,d0
	move.l	#_LVOMrgCop,a0
	jsr	_LVOSetFunction(a6)
	move.l	d0,sfxb_OldMrgCop(a5)

	; Set up the SFXLoadView() vector.
	move.l	sfxb_GfxBase(a5),a1
	lea	SFXLoadView(pc),a0
	move.l	a0,d0
	move.l	#_LVOLoadView,a0
	jsr	_LVOSetFunction(a6)
	move.l	d0,sfxb_OldLoadView(a5)

	move.l	a5,SfxBase(pc)		; store the SfxBase where the
					; SetFunction()ed MrgCop and LoadView
					; can find it.
Init.:
	move.l	a5,d0
	move.l	(sp)+,a5
	rts

NoAHLSem:
	move.l	sfxb_VPListSemaphore(a5),a1
	moveq	#SS_SIZE,d0
	jsr	_LVOFreeMem(a6)
NoVPLSem:
	move.l	sfxb_AnimationSemaphore(a5),a1
	moveq	#SS_SIZE,d0
	jsr	_LVOFreeMem(a6)
NoDBuffSem:
	move.l	sfxb_UtilityBase(a5),a1
	jsr	_LVOCloseLibrary(a6)
NoUtilLib:
	move.l	sfxb_GfxBase(a5),a1
	jsr	_LVOCloseLibrary(a6)
NoGfxLib:
	; Couldn't open Graphics.library V39, so bum out.
	move.l	a5,a1			; base in a1
	moveq	#0,d0			; Clear size...
	move.w	LIB_NEGSIZE(a5),d0	; Get negative size
	sub.l	d0,a1			; Adjust start memory pointer
	add.w	LIB_POSSIZE(a5),d0	; Add in positive size
	jsr	_LVOFreeMem(a6)		; free our memory
	sub.l	a5,a5			; return NULL
	bra.s	Init.


********************************************************************************

Open:
	; Open the library, and return the library base in d0.
	addq.w	#1,LIB_OPENCNT(a6)	; bump the count
	bclr	#LIBB_DELEXP,LIB_FLAGS(a6)
	move.l	a6,d0
	rts


Close:
	; Reduce the open count, and do a delayed expunge if necessary
	moveq	#0,d0		; return value
	subq.w	#1,LIB_OPENCNT(a6)
	bne.s	1$
	btst	#LIBB_DELEXP,LIB_FLAGS(a6)
	beq.s	1$
	bsr.s	Expunge		; get rid of it
1$:
	rts

Expunge:
	; are we still open?
	tst.w	LIB_OPENCNT(a6)
	bne	DelayExpunge

	; OK to remove us.
	movem.l	d2/a5,-(sp)
	move.l	a6,a5
	move.l	sfxb_SegList(a5),d2
	move.l	sfxb_ExecBase(a5),a6
	move.l	a5,a1
	jsr	_LVORemove(a6)

	; reset the LVOMrgCop() vector.
	move.l	sfxb_OldMrgCop(a5),d0
	move.l	sfxb_GfxBase(a5),a1
	move.l	#_LVOMrgCop,a0
	jsr	_LVOSetFunction(a6)

	; reset the LVOLoadView() vector.
	move.l	sfxb_OldLoadView(a5),d0
	move.l	sfxb_GfxBase(a5),a1
	move.l	#_LVOLoadView,a0
	jsr	_LVOSetFunction(a6)

	; Free the Semaphores
	move.l	sfxb_AHListSemaphore(a5),a1
	moveq	#SS_SIZE,d0
	jsr	_LVOFreeMem(a6)

	move.l	sfxb_VPListSemaphore(a5),a1
	moveq	#SS_SIZE,d0
	jsr	_LVOFreeMem(a6)

	move.l	sfxb_AnimationSemaphore(a5),a1
	moveq	#SS_SIZE,d0
	jsr	_LVOFreeMem(a6)

	; close the utility library
	move.l	sfxb_UtilityBase(a5),a1
	jsr	_LVOCloseLibrary(a6)

	; close the graphics library
	move.l	sfxb_GfxBase(a5),a1
	jsr	_LVOCloseLibrary(a6)

	; free up any extra VPLists we have. The extra lists are added to the
	; list HEAD, so we can walk through from the start of the list to the
	; end, freeing all the lists. We don't free the last list though as
	; that is embedded in SpecialFXBase.

	move.l	sfxb_VPListHead+MLH_HEAD(a5),a2
free_vplists:
	move.l	a2,a1
	move.l	MLN_SUCC(a2),a2
	tst.l	MLN_SUCC(a2)	; is this the last one?
	beq.s	done_vplist	; if so, don't FreeMem() our embedded list
	moveq	#vpl_SIZEOF,d0
	jsr	_LVOFreeMem(a6)
	bra.s	free_vplists

done_vplist:
	; free our memory
	moveq	#0,d0
	move.l	a5,a1
	move.w	LIB_NEGSIZE(a5),d0
	sub.l	d0,a1
	add.w	LIB_POSSIZE(a5),d0
	jsr	_LVOFreeMem(a6)

	; set up the return value
	move.l	d2,d0
	movem.l	(sp)+,d2/a5
	rts

DelayExpunge:
	bset	#LIBB_DELEXP,LIB_FLAGS(a6)
Null:
	moveq	#0,d0		; return NULL for a delayed expunge, and the NULL function
	rts

********************************************************************************

