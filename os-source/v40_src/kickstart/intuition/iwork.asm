
*  $Id: iwork.asm,v 38.13 92/06/23 17:19:54 peter Exp $
*
*   created by Dale on some unspecified date
*   Brought up to V25 by =RJ= 2 May 1985
*   romtags added 13 may 1985, by Neil
*   renamed from itainted.asm to iwork.asm on 9 June 85, by =DrRj=, in honor
*           of how hard iwork
*   made into an almost-standard library by Dale around 12 June 85
*   touch-up to Dale's design by =RJ= on 13 June 1985
*   =RJ= added ConsoleDevice, new routines, on and around 18 June 1985
*   =RJ= more and more 8 August 1985
*	fixed up, autoinit, and more bitchin'er by jimm and neil and peter
*	on an ongoing basis


	INCLUDE 'exec/types.i'
	INCLUDE	'exec/nodes.i'
	INCLUDE	'exec/resident.i'

	INCLUDE 'ioffsets.i'

	xref	VERNUM
	xref	REVNUM
	xref	VERSTRING


; VERSION	EQU	1
; REVISION	EQU	34
; VSTRING	MACRO
 	; DC.B	'Intuition 1.0.34 14 Nov 85'
	; DC.B	13,10,0
	; DS	0
	; ENDM

	xref	EndCode
	xref	KPutFmt


*----------------------------------------------------------------
*
*   Resident Module Tag
*
*----------------------------------------------------------------
	xref	_IntuiRomInit
	xdef	_IntuiName

IntuiStart:
	DC.W	RTC_MATCHWORD	;UWORD RT_MATCHWORD    word to match
	DC.L	IntuiStart	;APTR  RT_MATCHTAG     ptr to structure base
	DC.L	EndCode		;APTR  RT_ENDSKIP      addr to continue scan
	DC.B	RTF_COLDSTART!RTF_AUTOINIT
				;UBYTE RT_FLAGS        various tag flags
	DC.B	VERNUM		;UBYTE RT_VERSION      release version number
	DC.B	NT_LIBRARY	;UBYTE RT_TYPE         type of module
	DC.B	10		;BYTE  RT_PRI          initialization priority
	DC.L	_IntuiName	;APTR  RT_NAME         pointer to node name
	DC.L	VERSTRING	;APTR  RT_IDSTRING     pointer to id string
	DC.L	IntuiRomTable	;APTR  RT_INIT         pointer to init code

IntuiRomTable:
	DC.L	ib_sizeof
	DC.L	_IntuiVecs
	DC.L	0
	DC.L	IntuiRomInit

IntuiRomInit: 
	MOVE.L	A6,-(SP)
	MOVE.L	D0,A6
	JSR	_IntuiRomInit
	MOVE.L	A6,D0
	MOVE.L	(SP)+,A6
	RTS
	;  testing ALINK diagnostics for far reference BSR	EndCode

	xdef	_fetchIBase
_fetchIBase:
	MOVE.L	A6,D0
	RTS

_IntuiName	DC.B	'intuition.library',0
**IntuiString:	VSTRING
	DS.W	0

	xref	_Debug
*	xdef	_exit

	xref	_OpenIntuition
	xref	_Intuition
	xref	addgadget
	xref	cleardmrequest
	xref	clearmenustrip
	xref	clearpointer
	xref	closescreen
	xref	closewindow
	xref	_CloseWorkBench
	xref	currenttime
	xref	displayalert
	xref	displaybeep
	xref	doubleclick
	xref	drawborder
	xref	drawimage
	xref	endrequest
	xref	getdefprefs
	xref	getprefs
	xref	initrequester
	xref	itemaddress
	xref	modifyidcmp
	xref	modifyprop
	xref	movescreen
	xref	movewindow
	xref	newmodifyprop	; jimm: 7/28/86
	xref	offgadget
	xref	offmenu
	xref	ongadget
	xref	onmenu
	xref	openscreen
	xref	openwindow
	xref	openworkbench
	xref	printitext
	xref	refreshgadgets
	xref	removegadget
	xref	reportmouse
	xref	request
	xref	screentoback
	xref	screentofront
	xref	setdmrequest
	xref	setmenustrip
	xref	setpointer
	xref	setwindowtitles
	xref	showtitle
	xref	sizewindow
	xref	_ViewAddress
	xref	viewportaddress
	xref	windowtoback
	xref	windowtofront
	xref	windowlimits
	xref	setprefs
	xref	intuitextlength
	xref	wbenchtoback
	xref	wbenchtofront
	xref	autorequest
	xref	beginrefresh
	xref	buildsysrequest
	xref	endrefresh
	xref	freesysrequest
	xref	makescreen
	xref	_RemakeDisplay
	xref	_RethinkDisplay
	xref	allocremember
	xref	alohaworkbench
	xref	freeremember
	xref	lockibase
	xref	unlockibase
	xref	getscreendata
	xref	refreshglist
	xref	addglist
	xref	removeglist
	xref	activatewindow
	xref	refreshwindowframe
	xref	activategadget
	xref	queryoverscan
	xref	movewindowinfrontof
	xref	changewindowbox
	xref	setedithook
	xref	setmousequeue
	xref	zipwindow
	xref	lockpubscreen
	xref	unlockpubscreen
	xref	_LockPubScreenList
	xref	_UnlockPubScreenList
	xref	nextpubscreen
	xref	setdefaultpubscreen
	xref	setpubscreenmodes
	xref	pubscreenstatus
	xref	obtaingirport
	xref	releasegirport
	xref	gadgetmouse
	xref	setiprefs
	xref	getdefaultpubscreen
	xref	easyrequestargs
	xref	buildeasyrequestargs
	xref	sysreqhandler
	xref	openwindowtaglist
	xref	openscreentaglist

	xref	drawimagestate
	xref	pointinimage
	xref	eraseimage

	xref	newobjecta
	xref	disposeobject
	xref	setattrsa
	xref	getattr
	xref	setgadgetattrsa

	xref	nextobject
	xref	findclass
	xref	makeclass
	xref	addclass
	xref	getscreendrawinfo
	xref	freescreendrawinfo
	xref	resetmenustrip
	xref	removeclass
	xref	freeclass
	xref	_lockClassList
	xref	_unlockClassList
	xref	allocscreenbuffer
	xref	freescreenbuffer
	xref	changescreenbuffer
	xref	screendepth
	xref	screenposition
	xref	scrollwindowraster
	xref	lendmenus
	xref	dogadgetmethoda
	xref	setwindowpointera
	xref	timeddisplayalert
	xref	helpcontrol

*openintu:
*	jmp	_OpenIntuition

extfuncintu:
closeintu:
expungeintu:
sparevector:
	clr.l	d0
	rts

*	this is the actual table of jump vectors called by intuition.lib

	xdef	_IntuiVecs

*
* Build the function table (short size)
*
V_DEF		MACRO
		dc.w	\1+(*-_IntuiVecs)
		ENDM

VSELF_DEF	MACRO
		dc.w	\1-_IntuiVecs
		ENDM

_IntuiVecs:
	dc.w	-1		* Interpret as short vectors
	V_DEF		_OpenIntuition
	VSELF_DEF	closeintu
	VSELF_DEF	expungeintu
	VSELF_DEF	extfuncintu
	V_DEF		_OpenIntuition
	V_DEF		_Intuition
	V_DEF		addgadget
	V_DEF		cleardmrequest
	V_DEF		clearmenustrip
	V_DEF		clearpointer
	V_DEF		closescreen
	V_DEF		closewindow
	V_DEF		_CloseWorkBench
	V_DEF		currenttime
	V_DEF		displayalert
	V_DEF		displaybeep
	V_DEF		doubleclick
	V_DEF		drawborder
	V_DEF		drawimage
	V_DEF		endrequest
	V_DEF		getdefprefs
	V_DEF		getprefs
	V_DEF		initrequester
	V_DEF		itemaddress
	V_DEF		modifyidcmp
	V_DEF		modifyprop
	V_DEF		movescreen
	V_DEF		movewindow
	V_DEF		offgadget
	V_DEF		offmenu
	V_DEF		ongadget
	V_DEF		onmenu
	V_DEF		openscreen
	V_DEF		openwindow
	V_DEF		openworkbench
	V_DEF		printitext
	V_DEF		refreshgadgets
	V_DEF		removegadget
	V_DEF		reportmouse
	V_DEF		request
	V_DEF		screentoback
	V_DEF		screentofront
	V_DEF		setdmrequest
	V_DEF		setmenustrip
	V_DEF		setpointer
	V_DEF		setwindowtitles
	V_DEF		showtitle
	V_DEF		sizewindow
	V_DEF		_ViewAddress
	V_DEF		viewportaddress
	V_DEF		windowtoback
	V_DEF		windowtofront
	V_DEF		windowlimits
	V_DEF		setprefs
	V_DEF		intuitextlength
	V_DEF		wbenchtoback
	V_DEF		wbenchtofront
	V_DEF		autorequest
	V_DEF		beginrefresh
	V_DEF		buildsysrequest
	V_DEF		endrefresh
	V_DEF		freesysrequest
	V_DEF		makescreen
	V_DEF		_RemakeDisplay
	V_DEF		_RethinkDisplay
	V_DEF		allocremember
	V_DEF		alohaworkbench
	V_DEF		freeremember
	V_DEF		lockibase
	V_DEF		unlockibase
	V_DEF		getscreendata
	V_DEF		refreshglist
	V_DEF		addglist
	V_DEF		removeglist
	V_DEF		activatewindow
	V_DEF		refreshwindowframe
	V_DEF		activategadget
	V_DEF		newmodifyprop
	V_DEF		queryoverscan
	V_DEF		movewindowinfrontof
	V_DEF		changewindowbox
	V_DEF		setedithook
	V_DEF		setmousequeue
	V_DEF		zipwindow
	V_DEF		lockpubscreen
	V_DEF		unlockpubscreen
	V_DEF		_LockPubScreenList
	V_DEF		_UnlockPubScreenList
	V_DEF		nextpubscreen
	V_DEF		setdefaultpubscreen
	V_DEF		setpubscreenmodes
	V_DEF		pubscreenstatus
	V_DEF		obtaingirport
	V_DEF		releasegirport
	V_DEF		gadgetmouse
	V_DEF		setiprefs
	V_DEF		getdefaultpubscreen
	V_DEF		easyrequestargs
	V_DEF		buildeasyrequestargs
	V_DEF		sysreqhandler
	V_DEF		openwindowtaglist
	V_DEF		openscreentaglist
	V_DEF		drawimagestate
	V_DEF		pointinimage
	V_DEF		eraseimage
	V_DEF		newobjecta
	V_DEF		disposeobject
	V_DEF		setattrsa
	V_DEF		getattr
	V_DEF		setgadgetattrsa
	V_DEF		nextobject
	V_DEF		findclass
	V_DEF		makeclass
	V_DEF		addclass
	V_DEF		getscreendrawinfo
	V_DEF		freescreendrawinfo
	V_DEF		resetmenustrip
	V_DEF		removeclass
	V_DEF		freeclass
	V_DEF		_lockClassList
	V_DEF		_unlockClassList

; The first two spare vectors are loaned to IPrefs for the
; ResetWB handler stuff

	VSELF_DEF	sparevector	; AddResetWBHandler()
	VSELF_DEF	sparevector	; RemResetWBHandler()
	VSELF_DEF	sparevector	* Spare vectors for SetFunctioning
	VSELF_DEF	sparevector	* Spare vectors for SetFunctioning
	VSELF_DEF	sparevector	* Spare vectors for SetFunctioning
	VSELF_DEF	sparevector	* Spare vectors for SetFunctioning

*
* V39 LVOs begin here...
*

	V_DEF		allocscreenbuffer
	V_DEF		freescreenbuffer
	V_DEF		changescreenbuffer
	V_DEF		screendepth
	V_DEF		screenposition
	V_DEF		scrollwindowraster
	V_DEF		lendmenus
	V_DEF		dogadgetmethoda
	V_DEF		setwindowpointera
	V_DEF		timeddisplayalert
	V_DEF		helpcontrol

	dc.w	-1			* end of table


* jimm: 1/27/86: NO EXIT ... _exit:  JMP     _Debug

	end

