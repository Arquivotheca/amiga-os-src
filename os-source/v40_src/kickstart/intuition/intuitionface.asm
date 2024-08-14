
*   This file contains the c-like interface into the library routines
*
*  $Id: intuitionface.asm,v 38.23 93/01/08 14:47:11 peter Exp $
*
* ============================================================================
* define these addresses for includion in the iwork.asm file
* 

	include	'intuition/screens.i'

* These are stubs so Intuition calls itself through the LVO:

	xdef	_DisplayBeep		; Call self through LVO
	xdef	_OpenScreen		; Call self through LVO
	xdef	_OpenWorkBench		; Call self through LVO
	xdef	_OpenScreenTagList	; Call self through LVO
	xdef	_ScreenDepth		; Call self through LVO
	xdef	_SetWindowPointerA	; Call self through LVO

* These are the routines that the jump table points to:

*	xdef	openintuition
*	xdef	intuition
	xdef	addgadget
	xdef	cleardmrequest
	xdef	clearmenustrip
	xdef	clearpointer
	xdef	closescreen
	xdef	closewindow
*	xdef	closeworkbench
	xdef	currenttime
	xdef	displayalert
	xdef	displaybeep
	xdef	doubleclick
	xdef	drawborder
	xdef	drawimage
	xdef	endrequest
	xdef	getdefprefs
	xdef	getprefs
	xdef	initrequester
	xdef	itemaddress
	xdef	modifyidcmp
	xdef	modifyprop
	xdef	movescreen
	xdef	movewindow
	xdef	newmodifyprop	; jimm 7/28/86
	xdef	offgadget
	xdef	offmenu
	xdef	ongadget
	xdef	onmenu
	xdef	openscreen
	xdef	openwindow
	xdef	openworkbench
	xdef	printitext
	xdef	refreshgadgets
	xdef	removegadget
*	xdef	reportmouse
	xdef	request
	xdef	screentoback
	xdef	screentofront
	xdef	setdmrequest
	xdef	setmenustrip
	xdef	setpointer
	xdef	setwindowtitles
	xdef	showtitle
	xdef	sizewindow
*	xdef	viewaddress
*	xdef	viewportaddress
	xdef	windowtoback
	xdef	windowtofront
	xdef	windowlimits
	xdef	setprefs
	xdef	intuitextlength
	xdef	wbenchtoback
	xdef	wbenchtofront
	xdef	autorequest
	xdef	beginrefresh
	xdef	buildsysrequest
	xdef	endrefresh
	xdef	freesysrequest
	xdef	makescreen
*	xdef	remakedisplay
*	xdef	rethinkdisplay
	xdef	allocremember
	xdef	alohaworkbench
	xdef	freeremember
*	xdef	lockibase		; downcoded
*	xdef	unlockibase		; downcoded
	xdef	getscreendata
	xdef	refreshglist
	xdef	addglist
	xdef	removeglist
	xdef	activatewindow
	xdef	refreshwindowframe
	xdef	activategadget
	xdef	queryoverscan
	xdef	movewindowinfrontof
	xdef	changewindowbox
	xdef	setedithook
	xdef	setmousequeue
	xdef	zipwindow
	xdef	lockpubscreen
	xdef	unlockpubscreen
*	xdef	lockpubscreenlist
*	xdef	unlockpubscreenlist
	xdef	nextpubscreen
	xdef	setdefaultpubscreen
	xdef	setpubscreenmodes
	xdef	pubscreenstatus
	xdef	obtaingirport
	xdef	releasegirport
	xdef	gadgetmouse
	xdef	setiprefs
	xdef	getdefaultpubscreen
	xdef	easyrequestargs
	xdef	buildeasyrequestargs
	xdef	sysreqhandler
	xdef	openwindowtaglist
	xdef	openscreentaglist


	xdef	drawimagestate
	xdef	pointinimage
	xdef	eraseimage

	xdef	newobjecta
	xdef	disposeobject
	xdef	setattrsa
	xdef	getattr
	xdef	setgadgetattrsa

	xdef	nextobject
	xdef	findclass
	xdef	makeclass
	xdef	addclass
	xdef	getscreendrawinfo
	xdef	freescreendrawinfo
	xdef	resetmenustrip
	xdef	removeclass
	xdef	freeclass
*	xdef	lockclasslist
*	xdef	unlockclasslist

*	Now Peter gets to add functions
*
*	V39 names

	xdef	allocscreenbuffer
	xdef	freescreenbuffer
	xdef	changescreenbuffer
	xdef	screendepth
	xdef	screenposition
	xdef	scrollwindowraster
	xdef	lendmenus
	xdef	dogadgetmethoda
	xdef	setwindowpointera
	xdef	timeddisplayalert
	xdef	helpcontrol

* ============================================================================
* external references
*
	xref	_Debug

	xref	_stackSwap

*	xref	_OpenIntuition
*	xref	_Intuition
	xref	_ClearDMRequest
	xref	_ClearMenuStrip
	xref	_ClearPointer
	xref	_CloseScreen
	xref	_CloseWindow
*	xref	_CloseWorkBench
	xref	_CurrentTime
	xref	_timedDisplayAlert
	xref	_IDisplayBeep
	xref	_DoubleClick
	xref	_DrawBorder
	xref	_DrawImage
	xref	_EndRequest
	xref	_GetDefPrefs
	xref	_GetPrefs
	xref	_InitRequester
	xref	_ItemAddress
	xref	_ModifyIDCMP
	xref	_NewModifyProp	;jimm: 7/28/86
	xref	_MoveWindow
	xref	_OffGadget
	xref	_OffMenu
	xref	_OnGadget
	xref	_OnMenu
	xref	_openScreen		; via stackSwap()
	xref	_OpenWindow		; via stackSwap()
	xref	_openWorkBench
	xref	_PrintIText
	xref	_ReportMouse
	xref	_Request
	xref	_SetDMRequest
	xref	_SetPointer
	xref	_SetWindowTitles
	xref	_ShowTitle
	xref	_SizeWindow
*	xref	_ViewAddress
	xref	_ViewPortAddress
	xref	_WindowToBack
	xref	_WindowToFront
	xref	_WindowLimits
	xref	_setPrefs		; via stackSwap()
	xref	_IntuiTextLength
	xref	_autoRequest		; via stackSwap()
	xref	_BeginRefresh
	xref	_buildSysRequest	; via stackSwap()
	xref	_EndRefresh
	xref	_FreeSysRequest
	xref	_MakeScreen
*	xref	_RemakeDisplay
*	xref	_RethinkDisplay
	xref	_AllocRemember
	xref	_AlohaWorkbench
	xref	_FreeRemember
*	xref	_LockIBase
*	xref	_UnlockIBase
	xref	_GetScreenData
	xref	_drawGadgets
	xref	_AddGList
	xref	_RemoveGList
	xref	_ActivateWindow
	xref	_drawWindowBorder
	xref	_RefreshWindowFrame
	xref	_ActivateGadget
	xref	_QueryOverscan
	xref	_MoveWindowInFrontOf
	xref	_ChangeWindowBox
	xref	_SetEditHook
	xref	_SetMouseQueue
	xref	_ZipWindow
	xref	_LockPubScreen
	xref	_UnlockPubScreen
*	xref	_LockPubScreenList
*	xref	_UnlockPubScreenList
	xref	_NextPubScreen
	xref	_SetDefaultPubScreen
	xref	_SetPubScreenModes
	xref	_PubScreenStatus
	xref	_ObtainGIRPort
	xref	_freeRP		; same as FreeGIRPort
	xref	_GadgetMouse
	xref	_SetIPrefs
	xref	_GetDefaultPubScreen
	xref	_easyRequestArgs	; via stackSwap()
	xref	_buildEasyRequestArgs	; via stackSwap()
	xref	_SysReqHandler
	xref	_OpenWindowTagList	; via stackSwap()
	xref	_openScreenTagList	; via stackSwap()


	xref	_DrawImageState
	xref	_PointInImage
	xref	_EraseImage

	xref	_NewObjectA
	xref	_DisposeObject
	xref	_SetAttrsA
	xref	_GetAttr
	xref	_SetGadgetAttrsA

	xref	_NextObject
	xref	_FindClass
	xref	_MakeClass
	xref	_AddClass
	xref	_GetScreenDrawInfo
	xref	_FreeScreenDrawInfo

	xref	_doSetMenuStrip
	xref	_RemoveClass
	xref	_FreeClass
*	xref	_lockClassList
*	xref	_unlockClassList

	xref	_AllocScreenBuffer
	xref	_FreeScreenBuffer
	xref	_ChangeScreenBuffer
	xref	_LIB_ScreenDepth
	xref	_ScreenPosition
	xref	_ScrollWindowRaster
	xref	_LendMenus
	xref	_DoGadgetMethodA
	xref	_LIB_SetWindowPointerA
	xref	_HelpControl

* ============================================================================
* the build macros used below
*
push	macro
	move.\0	\1,-(sp)
	endm

pop	macro
	move.\0	(sp)+,\1
	endm

fix		macro
	IFGT	\1-8
	lea	\1(sp),sp
	ENDC
	IFLE	\1-8
	addq.l	#\1,sp
	ENDC
	rts
	endm

* ============================================================================
* at last, the actual vectors
*

*openintuition:
*	push.l	a6	* push IntuitionBase on stack
*	jsr	_OpenIntuition
*	fix 4

* vector directly
*intuition:
*	jmp	_Intuition

cleardmrequest
	push.l	a0
	jsr	_ClearDMRequest
	fix	4

clearmenustrip
	push.l	a0
	jsr	_ClearMenuStrip
	fix 4

*clearpointer
*	push.l	a0
*	jsr	_ClearPointer
*	fix	4

; ClearPointer() now just calls SetPointer()
clearpointer			; a0 = window
	movem.l	d2/d3,-(sp)	; we need to trash these
	moveq	#0,d0		; Height
	moveq	#0,d1		; Width
	moveq	#0,d2		; XOffset
	moveq	#0,d3		; YOffset
	move.l	d0,a1		; Pointer
	jsr	setpointer
	movem.l	(sp)+,d2/d3
	rts

closescreen
	push.l	a0
	jsr	_CloseScreen
	fix 4

closewindow
	push.l	a0
	jsr	_CloseWindow
	fix 4

* vector directly
*closeworkbench
*	jmp	_CloseWorkBench

currenttime
	push.l	a1
	push.l	a0
	jsr	_CurrentTime
	fix 8

* displayalert now calls timeddisplayalert with a1 = ~0
displayalert
	suba.l	a1,a1	; clear a1...	(2 bytes)
	subq.l	#1,a1	; a1=-1 now...	(2 bytes)
timeddisplayalert
	push.l	a1
	push.l	d1
	push.l	a0
	push.l	d0
	jsr	_timedDisplayAlert
	fix 16

displaybeep
	push.l	a0
	jsr	_IDisplayBeep
	fix 4

* call self through vector
	xref _LVODisplayBeep
_DisplayBeep:
	move.l  4(sp),a0
	jmp     _LVODisplayBeep(a6)


doubleclick
	movem.l	d0/d1/d2/d3,-(sp)
	jsr	_DoubleClick
	fix 16
	rts

drawborder
	push.l	d1
	push.l	d0
	push.l	a1
	push.l	a0
	jsr	_DrawBorder
	fix 16

drawimage
	push.l	d1
	push.l	d0
	push.l	a1
	push.l	a0
	jsr	_DrawImage
	fix 16

endrequest
	push.l	a1
	push.l	a0
	jsr	_EndRequest
	fix 8

getdefprefs
	push.l	d0
	push.l	a0
	jsr	_GetDefPrefs
	fix 8

getprefs
	push.l	d0
	push.l	a0
	jsr	_GetPrefs
	fix 8

initrequester
	push.l	a0
	jsr	_InitRequester
	fix 4

itemaddress
	push.l	d0
	push.l	a0
	jsr	_ItemAddress
	fix 8

modifyidcmp
	push.l	d0
	push.l	a0
	jsr	_ModifyIDCMP
	fix 8

newmodifyprop
	push.l  d5		; respect last parameter
	bra.s	oldmodifyprop

modifyprop
	pea	-2		; pass -2 to NewModifyProp
oldmodifyprop
	movem.l	d0/d1/d2/d3/d4,-(sp) ; common parameters
	movem.l	a0/a1/a2,-(sp)
	jsr	_NewModifyProp
	fix 36

; MoveScreen( screen, dx, dy ) = ScreenPosition( screen, 0, dx, dy, 0, 0 );
movescreen
	clr.l	-(sp)
	clr.l	-(sp)
	push.l	d1
	push.l	d0
	clr.l	-(sp)
	push.l	a0
	jsr	_ScreenPosition
	fix 24

movewindow
	push.l	d1
	push.l	d0
	push.l	a0
	jsr	_MoveWindow
	fix 12

offgadget
	movem.l	a0/a1/a2,-(sp)
	jsr	_OffGadget
	fix 12

offmenu
	push.l	d0
	push.l	a0
	jsr	_OffMenu
	fix 8

ongadget
	movem.l	a0/a1/a2,-(sp)
	jsr	_OnGadget
	fix 12
	
onmenu
	push.l	d0
	push.l	a0
	jsr	_OnMenu
	fix 8

openscreen
	push.l	a0
	pea	_openScreen		; stackSwap is to call this guy
	jsr	_stackSwap
	fix 4

* call self through vector
	xref _LVOOpenScreen
_OpenScreen:
	move.l  4(sp),a0
	jmp     _LVOOpenScreen(a6)

openwindow
	push.l a0
	pea	_OpenWindow		; stackSwap is to call this guy
	jsr	_stackSwap
	fix 4

* call self through vector
	xref _LVOOpenWorkBench
_OpenWorkBench:
	jmp	_LVOOpenWorkBench(a6)

* Would vector directly, but have to hack back compatibilty for WP,
* put a copy of return val in A0
openworkbench
	jsr	_openWorkBench
	move.l	d0,a0
	rts

printitext
	push.l	d1
	push.l	d0
	push.l	a1
	push.l	a0
	jsr	_PrintIText
	fix 16

* put this in asm
*reportmouse
*	push.l	d0
*	push.l	a0
*	jsr	_ReportMouse
*	fix 8

request
	push.l	a1
	push.l	a0
	jsr	_Request
	fix 8

setdmrequest
	push.l	a1
	push.l	a0
	jsr	_SetDMRequest
	fix 8

setpointer
	movem.l	d0/d1/d2/d3,-(sp)
	push.l	a1
	push.l	a0
	jsr	_SetPointer
	fix 24

setwindowtitles
	movem.l	a0/a1/a2,-(sp)
	jsr	_SetWindowTitles
	fix 12

showtitle
	push.l	d0
	push.l	a0
	jsr	_ShowTitle
	fix 8

sizewindow
	push.l	d1
	push.l	d0
	push.l	a0
	jsr	_SizeWindow
	fix 12

; ZZZ: once I have ibase.asm, do this inline
*viewaddress
*	jmp	_ViewAddress

windowtoback
	push.l	a0
	jsr	_WindowToBack
	fix 4

windowtofront
	push.l	a0
	jsr	_WindowToFront
	fix 4

windowlimits
	movem.l	d0/d1/d2/d3,-(sp)
	push.l	a0
	jsr	_WindowLimits
	fix 20

* ============================================================================
setprefs
	push.l	d1
	push.l	d0
	push.l	a0
	pea	_setPrefs		; stackSwap is to call this guy
	jsr	_stackSwap
	fix 12


* ============================================================================
intuitextlength
	push.l	a0
	jsr	_IntuiTextLength
	fix 4


* ============================================================================
autorequest
	movem.l	d0/d1/d2/d3,-(sp)
	movem.l	a0/a1/a2/a3,-(sp)
	pea	_autoRequest		; stackSwap is to call this guy
	jsr	_stackSwap
	fix 32

beginrefresh
	push.l	a0
	jsr	_BeginRefresh
	fix 4

buildsysrequest
	movem.l	d0/d1/d2,-(sp)
	movem.l	a0/a1/a2/a3,-(sp)
	pea	_buildSysRequest	; stackSwap is to call this guy
	jsr	_stackSwap
	fix 28

endrefresh
	push.l	d0
	push.l	a0
	jsr	_EndRefresh
	fix 8

freesysrequest
	push.l	a0
	jsr	_FreeSysRequest
	fix 4

makescreen
	push.l	a0
	jsr	_MakeScreen
	fix 4

* vector directly
*remakedisplay
*	jmp	_RemakeDisplay
*
*rethinkdisplay
*	jmp	_RethinkDisplay


allocremember
	push.l	d1
	push.l	d0
	push.l	a0
	jsr	_AllocRemember
	fix 12

alohaworkbench
	push.l	a0
	jsr	_AlohaWorkbench
	fix 4

freeremember
	push.l	d0
	push.l	a0
	jsr	_FreeRemember
	fix 8

* Downcoded into assembler
*lockibase:
*	push.l	d0
*	jsr	_LockIBase
*	fix	4

*unlockibase:
*	push.l	a0
*	jsr	_UnlockIBase
*	fix	4

getscreendata:
	movem.l	d0/d1/a1,-(sp)
	push.l	a0

	jsr	_GetScreenData
	
	fix 16

* RefreshGadgets() and RefreshGList() share stub code.
* Note that Intuition never cares about the requester parameter
* you pass.  We don't even bother pushing it here.

refreshgadgets
	moveq	#-2,d0		; -2 means refresh all gadgets in requesters
;	FALL THROUGH TO REFRESHGLIST

refreshglist
	moveq	#0,d1		; 0 means "DRAWGADGETS_ALL"
	movem.l	d0/d1,-(sp)
	push.l	a0
	push.l	a1
	jsr	_drawGadgets
	fix 16


addgadget
	moveq	#1,d1			; numgad = 1
;	FALL THROUGH TO ADDGLIST
;	Note that a2 (requester) is uninitialized, but this has
;	always been true, and the C code never looked at the
;	requester parameter unless the gadget being added had
;	REQGADGET set.

addglist
	movem.l	d0/d1/a2,-(sp)
	push.l	a1
	push.l	a0
	jsr	_AddGList
	fix 20

removegadget
	moveq	#1,d0			; numgad = 0
;	FALL THROUGH TO REMOVEGLIST
	
removeglist
	push.l d0
	push.l a1
	push.l a0
	jsr	_RemoveGList
	fix 12

activatewindow
	push.l a0
	jsr	_ActivateWindow
	pop.l  a0		; WP Library expects A0 to be preserved
	rts

refreshwindowframe
	push.l	a0
	jsr	_RefreshWindowFrame
	fix 4

activategadget
	movem.l	a0/a1/a2,-(sp)
	jsr	_ActivateGadget
	fix 12

queryoverscan
	push.l	d0
	push.l	a1
	push.l	a0
	jsr	_QueryOverscan
	fix	12
	
movewindowinfrontof
	push.l	a1
	push.l	a0
	jsr	_MoveWindowInFrontOf
	fix	8

changewindowbox
    	movem.l	d0/d1/d2/d3,-(sp)
    	push.l	a0
	jsr	_ChangeWindowBox
	fix	20

setedithook
	push.l	a0
	jsr	_SetEditHook
	fix	4

setmousequeue
	push.l	d0
	push.l	a0
	jsr	_SetMouseQueue
	fix	8

zipwindow
	push.l	a0
	jsr	_ZipWindow
	fix	4

lockpubscreen
	push.l	a0
	jsr	_LockPubScreen
	fix 4

unlockpubscreen
	push.l	a1
	push.l	a0
	jsr	_UnlockPubScreen
	fix 8

* vector directly
*lockpubscreenlist
*	jmp	_LockPubScreenList
*
*unlockpubscreenlist
*	jmp	_UnlockPubScreenList

nextpubscreen
	push.l	a1
	push.l	a0
	jsr	_NextPubScreen
	fix	8
	
setdefaultpubscreen
	push.l	a0
	jsr	_SetDefaultPubScreen
	fix	4

setpubscreenmodes
	push.l	d0
	jsr	_SetPubScreenModes
	fix	4

pubscreenstatus
	push.l	d0
	push.l	a0
	jsr	_PubScreenStatus
	fix	8

obtaingirport
	push.l	a0
	jsr	_ObtainGIRPort
	fix	4

releasegirport
	push.l	a0
	jsr	_freeRP
	fix	4

gadgetmouse
	movem.l	a0/a1/a2,-(sp)
	jsr	_GadgetMouse
	fix	12

setiprefs
	movem.l	d0/d1,-(sp)
	push.l	a0
	jsr	_SetIPrefs
	fix	12

getdefaultpubscreen
	push.l	a0
	jsr	_GetDefaultPubScreen
	fix	4

easyrequestargs
	movem.l	A0/A1/A2/A3,-(sp)
	pea	_easyRequestArgs	; stackSwap is to call this guy
	jsr	_stackSwap
	fix	16

buildeasyrequestargs
	movem.l	d0/a3,-(sp)
	movem.l	a0/a1,-(sp)
	pea	_buildEasyRequestArgs	; stackSwap is to call this guy
	jsr	_stackSwap
	fix	16

sysreqhandler
	push.l	d0
	movem.l	a0/a1,-(sp)
	jsr	_SysReqHandler
	fix 12

openwindowtaglist
	movem.l	a0/a1,-(sp)
	pea	_OpenWindowTagList	; stackSwap is to call this guy
	jsr	_stackSwap
	fix	8

openscreentaglist
	movem.l	a0/a1,-(sp)
	pea	_openScreenTagList	; stackSwap is to call this guy
	jsr	_stackSwap
	fix	8

* call self through vector
	xref	_LVOOpenScreenTagList
_OpenScreenTagList:
	movem.l 4(sp),a0/a1
	jmp     _LVOOpenScreenTagList(a6)


drawimagestate
	movem.l	d0/d1/d2/a2,-(sp)
	movem.l	a0/a1,-(sp)
	jsr	_DrawImageState
	fix	24

pointinimage
	movem.l	d0/a0,-(sp)
	jsr	_PointInImage
	fix	8


eraseimage
	movem.l	d0/d1,-(sp)
	movem.l	a0/a1,-(sp)
	jsr	_EraseImage
	fix	16

newobjecta
	movem.l	a0/a1/a2,-(sp)
	jsr	_NewObjectA
	fix	12

disposeobject
    	push.l	a0
	jsr	_DisposeObject
	fix	4

setattrsa
	movem.l a0/a1,-(sp)
	jsr	_SetAttrsA
	fix	8

getattr
	movem.l	d0/a0/a1,-(sp)
	jsr	_GetAttr
	fix	12
	

setgadgetattrsa
	movem.l a0/a1/a2/a3,-(sp)
	jsr	_SetGadgetAttrsA
	fix	16

nextobject
	push.l	a0
	jsr	_NextObject
	fix	4
    
findclass
	push.l	a0
	jsr	_FindClass
	fix	4

makeclass
	movem.l	d0/d1,-(sp)
	movem.l	a0/a1/a2,-(sp)
	jsr	_MakeClass
	fix	20

addclass
	push.l	a0
	jsr	_AddClass
	fix	4

getscreendrawinfo
	push.l	a0
	jsr	_GetScreenDrawInfo
	fix	4

freescreendrawinfo
	movem.l	a0/a1,-(sp)
	jsr	_FreeScreenDrawInfo
	fix	8

resetmenustrip
	moveq	#0,d0			; 0 = don't recalc menus
	bra.s	dosetmenustrip

setmenustrip
	moveq	#1,d0			; 1 = recalc menus
dosetmenustrip
	push.l	d0
	push.l	a1
	push.l	a0
	jsr	_doSetMenuStrip
	fix 12
	
removeclass
	push.l	a0
	jsr	_RemoveClass
	fix	4

freeclass
	push.l	a0
	jsr	_FreeClass
	fix	4

* vector directly
*lockclasslist
*	jmp	_lockClassList
*
*unlockclasslist
*	jmp	_unlockClassList

allocscreenbuffer
	push.l	d0
	movem.l	a0/a1,-(sp)
	jsr	_AllocScreenBuffer
	fix 12

freescreenbuffer
	push.l	a1
	push.l	a0
	jsr	_FreeScreenBuffer
	fix 8

changescreenbuffer
	push.l	a1
	push.l	a0
	jsr	_ChangeScreenBuffer
	fix 8

* WBenchToFront/WBenchToBack() does:
*
* 	sc = findWorkBench();
*	if ( sc ) 
*	{
*	    ScreenDepth( NULL, SDEPTH_TOFRONT/SDEPTH_TOBACK, NULL );
*	    return ( TRUE );
*	}
*	return ( FALSE );


	XREF	_findWorkBench

wbenchtofront:
	moveq	#SDEPTH_TOFRONT,d1
	bra.s	wbdepth
wbenchtoback:
	moveq	#SDEPTH_TOBACK,d1

wbdepth:
	move.l	d1,-(sp)	; d1 = SDEPTH_TOBACK or SDEPTH_TOFRONT
	jsr	_findWorkBench
	move.l	(sp)+,d1
	tst.l	d0
	beq.s	wbdepth_exit	; WB not found: exit with d0 = 0
	suba.l	a0,a0		; a0 = screen (NULL for WB)
	move.l	d1,d0		; d0 = flags (SDEPTH_TOBACK or SDEPTH_TOFRONT)
	bsr.s	screentowherever
	moveq	#1,d0		; return TRUE
wbdepth_exit:
	rts

* ScreenToBack() and ScreenToFront() share stub magic
	xref _LVOScreenDepth

screentoback				; a0:  screen
	moveq	#SDEPTH_TOBACK,d0	; d0:  flags = SDEPTH_TOBACK
	bra.s	screentowherever
screentofront				; a0:  screen
	moveq	#SDEPTH_TOFRONT,d0	; d0:  flags = SDEPTH_TOFRONT

screentowherever
	suba.l	a1,a1			; a1:  reserved=NULL
	jmp	_LVOScreenDepth(a6)	; go through LVO

screendepth
	push.l	a1
	push.l	d0
	push.l	a0
	jsr	_LIB_ScreenDepth
	fix 12

* call self through vector
	xref _LVOScreenDepth
_ScreenDepth:
	movea.l	4(a7),a0
	movem.l	8(a7),d0/a1
	jmp     _LVOScreenDepth(a6)

screenposition:
	movem.l	d0/d1/d2/d3/d4,-(sp)
	push.l	a0
	jsr	_ScreenPosition
	fix 24

scrollwindowraster:
	movem.l d0/d1/d2/d3/d4/d5,-(sp)
	push.l	a1
	jsr	_ScrollWindowRaster
	fix 28

lendmenus:
	push.l	a1
	push.l	a0
	jsr	_LendMenus
	fix 8

dogadgetmethoda:
	movem.l a0/a1/a2/a3,-(sp)
	jsr	_DoGadgetMethodA
	fix 16

setwindowpointera:
	push.l	a1
	push.l	a0
	jsr	_LIB_SetWindowPointerA
	fix 8

* call self through vector
	xref _LVOSetWindowPointerA
_SetWindowPointerA:
	movem.l  4(sp),a0/a1
	jmp     _LVOSetWindowPointerA(a6)

helpcontrol:
	push.l	d0
	push.l	a0
	jsr	_HelpControl
	fix 8

	end


