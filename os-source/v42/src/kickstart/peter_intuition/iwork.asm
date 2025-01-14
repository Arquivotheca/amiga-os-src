
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

_IntuiVecs:
	dc.l		_OpenIntuition
	dc.l		closeintu
	dc.l		expungeintu
	dc.l		extfuncintu
	dc.l		_OpenIntuition
	dc.l		_Intuition
	dc.l		addgadget
	dc.l		cleardmrequest
	dc.l		clearmenustrip
	dc.l		clearpointer
	dc.l		closescreen
	dc.l		closewindow
	dc.l		_CloseWorkBench
	dc.l		currenttime
	dc.l		displayalert
	dc.l		displaybeep
	dc.l		doubleclick
	dc.l		drawborder
	dc.l		drawimage
	dc.l		endrequest
	dc.l		getdefprefs
	dc.l		getprefs
	dc.l		initrequester
	dc.l		itemaddress
	dc.l		modifyidcmp
	dc.l		modifyprop
	dc.l		movescreen
	dc.l		movewindow
	dc.l		offgadget
	dc.l		offmenu
	dc.l		ongadget
	dc.l		onmenu
	dc.l		openscreen
	dc.l		openwindow
	dc.l		openworkbench
	dc.l		printitext
	dc.l		refreshgadgets
	dc.l		removegadget
	dc.l		reportmouse
	dc.l		request
	dc.l		screentoback
	dc.l		screentofront
	dc.l		setdmrequest
	dc.l		setmenustrip
	dc.l		setpointer
	dc.l		setwindowtitles
	dc.l		showtitle
	dc.l		sizewindow
	dc.l		_ViewAddress
	dc.l		viewportaddress
	dc.l		windowtoback
	dc.l		windowtofront
	dc.l		windowlimits
	dc.l		setprefs
	dc.l		intuitextlength
	dc.l		wbenchtoback
	dc.l		wbenchtofront
	dc.l		autorequest
	dc.l		beginrefresh
	dc.l		buildsysrequest
	dc.l		endrefresh
	dc.l		freesysrequest
	dc.l		makescreen
	dc.l		_RemakeDisplay
	dc.l		_RethinkDisplay
	dc.l		allocremember
	dc.l		alohaworkbench
	dc.l		freeremember
	dc.l		lockibase
	dc.l		unlockibase
	dc.l		getscreendata
	dc.l		refreshglist
	dc.l		addglist
	dc.l		removeglist
	dc.l		activatewindow
	dc.l		refreshwindowframe
	dc.l		activategadget
	dc.l		newmodifyprop
	dc.l		queryoverscan
	dc.l		movewindowinfrontof
	dc.l		changewindowbox
	dc.l		setedithook
	dc.l		setmousequeue
	dc.l		zipwindow
	dc.l		lockpubscreen
	dc.l		unlockpubscreen
	dc.l		_LockPubScreenList
	dc.l		_UnlockPubScreenList
	dc.l		nextpubscreen
	dc.l		setdefaultpubscreen
	dc.l		setpubscreenmodes
	dc.l		pubscreenstatus
	dc.l		obtaingirport
	dc.l		releasegirport
	dc.l		gadgetmouse
	dc.l		setiprefs
	dc.l		getdefaultpubscreen
	dc.l		easyrequestargs
	dc.l		buildeasyrequestargs
	dc.l		sysreqhandler
	dc.l		openwindowtaglist
	dc.l		openscreentaglist
	dc.l		drawimagestate
	dc.l		pointinimage
	dc.l		eraseimage
	dc.l		newobjecta
	dc.l		disposeobject
	dc.l		setattrsa
	dc.l		getattr
	dc.l		setgadgetattrsa
	dc.l		nextobject
	dc.l		findclass
	dc.l		makeclass
	dc.l		addclass
	dc.l		getscreendrawinfo
	dc.l		freescreendrawinfo
	dc.l		resetmenustrip
	dc.l		removeclass
	dc.l		freeclass
	dc.l		_lockClassList
	dc.l		_unlockClassList

; The first two spare vectors are loaned to IPrefs for the
; ResetWB handler stuff

	dc.l		sparevector	; AddResetWBHandler()
	dc.l		sparevector	; RemResetWBHandler()
	dc.l		sparevector	* Spare vectors for SetFunctioning
	dc.l		sparevector	* Spare vectors for SetFunctioning
	dc.l		sparevector	* Spare vectors for SetFunctioning
	dc.l		sparevector	* Spare vectors for SetFunctioning

*
* V39 LVOs begin here...
*

	dc.l		allocscreenbuffer
	dc.l		freescreenbuffer
	dc.l		changescreenbuffer
	dc.l		screendepth
	dc.l		screenposition
	dc.l		scrollwindowraster
	dc.l		lendmenus
	dc.l		dogadgetmethoda
	dc.l		setwindowpointera
	dc.l		timeddisplayalert
	dc.l		helpcontrol

	dc.l	-1			* end of table


* jimm: 1/27/86: NO EXIT ... _exit:  JMP     _Debug

	end

