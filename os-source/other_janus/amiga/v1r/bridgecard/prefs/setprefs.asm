		SECTION	window,CODE
		INCLUDE	"exec/types.i"
		INCLUDE	"intuition/intuition.i"
		INCLUDE	"libraries/dos.i"
		INCLUDE	"../janus/janusvar.i"
		INCLUDE	"../janus/janusreg.i"
		INCLUDE	"../janus/janus.i"
		INCLUDE	"mydata.i"

		XDEF	_main
		XREF	_AbsExecBase,_LVOOpenLibrary,_LVOCloseLibrary
		XREF	_LVOOpenWindow,_LVOCloseWindow,_LVOOffGadget
		XREF	_LVOLock,_LVOUnLock,_LVOOpen,_LVOClose,_LVORead
		XREF	_LVOWrite

		XREF	CancelGadget,A000Gadget,D000Gadget,E000Gadget
		XREF	MonoGadget,ColorGadget,SerialGadget
		XREF	MainProgram

;=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

_main		suba.w	#mydata_SIZEOF,sp	allocate stack data area
		movea.l	sp,a5			A5 points to local data
		movea.l	_AbsExecBase,a6		A6 points to libraries

; Open janus library to get the base address of the IO register block.
; It can be closed immediately after because we don't need it any more.

		clr.l	d0			who cares what version ?
		lea.l	Jname(pc),a1		library name
		jsr	_LVOOpenLibrary(a6)	do the open
		tst.l	d0			did we get a library ptr ?
		beq	exit1			no, total panic!!!
		movea.l	d0,a1			fetch and save io base
		move.l	ja_IoBase(a1),JIOBase(a5)
		jsr	_LVOCloseLibrary(a6)	finished with janus now

; The libraries we need on a more permanent basis are intuition,DOS,gfx

		lea.l	Gname(pc),a1		open graphics
		moveq.l	#33,d0			V1.2 or better
		jsr	_LVOOpenLibrary(a6)
		move.l	d0,Glib(a5)		save the lib pointer
		beq	exit1			error, no library

		lea.l	Iname(pc),a1		open intuition
		moveq.l	#33,d0			V1.2 or better
		jsr	_LVOOpenLibrary(a6)
		move.l	d0,Ilib(a5)		save the lib pointer
		beq	exit2			wrong kickstart ???

		lea.l	Dname(pc),a1		open DOS
		moveq.l	#33,d0			also V1.2 or better
		jsr	_LVOOpenLibrary(a6)
		move.l	d0,Dlib(a5)		save the lib pointer
		beq	exit3			this shouldn't happen !!!

; Open the 2500 preferences file and read in the current state variables.
; If the file doesn't exist then fill in default values in the state vars.

		movea.l	d0,a6			using DOS library now
		lea.l	Pname(pc),a0		file to check
		move.l	a0,d1
		move.l	#ACCESS_READ,d2		see if the file exists
		jsr	_LVOLock(a6)
		move.l	d0,d1			did we get the lock ?
		beq.s	SetDefaults		no file, use defaults

; The prefs file is present so unlock it then open it and read the values

		jsr	_LVOUnLock(a6)		d1 holds the file lock
		lea.l	Pname(pc),a0		filename to open
		move.l	a0,d1
		move.l	#MODE_OLDFILE,d2	we know it exists now
		jsr	_LVOOpen(a6)
		move.l	d0,PrefsFile(a5)	save the file handle
		move.l	d0,d1			prepare to read from it
		moveq.l	#PREFSIZE,d3		amount to read
		lea.l	SerialState(a5),a0	where the data goes
		move.l	a0,d2
		jsr	_LVORead(a6)		read in preferences state
		move.l	d0,-(sp)		save actual read length
		move.l	PrefsFile(a5),d1	close the file
		jsr	_LVOClose(a6)
		move.l	(sp)+,d0		check length was OK
		cmpi.w	#PREFSIZE,d0
		beq.s	CheckPrefs		it was, so check values

; Either the file didn't exist or the wrong number of bytes were read in
; from the preferences file.  Set up reasonable defaults to start with.

SetDefaults	move.b	#DEFSER,SerialState(a5)	default serial preference
		move.b	#DEFMON,MonoState(a5)	default mono preference
		move.b	#DEFCOL,ColorState(a5)	default color preference
		move.b	#DEFRAM,RamBank(a5)	default RAM bank
		bra.s	PrefsOK			I know they are alright!!

; The preferences read in OK but we had better check thier consistency in
; case someone in the testing department decides to get smart and edit the
; values in the file before running the program.  In addition to this, we
; need to see that the RAM values are consistent with the kind of PC we
; have attached (AT or XT).  Bit 7 of the config reg contains this info.

CheckPrefs	move.b	SerialState(a5),d0	first 3 must be 0 or 1
		or.b	MonoState(a5),d0
		or.b	ColorState(a5),d0
		andi.b	#%11111110,d0
		bne.s	SetDefaults		any other value is wrong !
		move.b	RamBank(a5),d0		test RAM bank
		beq.s	SetDefaults		0 no good in AT or XT mode
		cmpi.b	#3,d0			nor is greater than 3
		bgt.s	SetDefaults
		blt.s	PrefsOK			1 and 2 are good in both
		move.l	JIOBase(a5),a0		3 is only good in XT mode
		btst.b	#7,jio_PCconfiguration(a0)
		beq.s	SetDefaults		sorry, we're using an AT

; Before opening the window, adjust the initial states of all the gadgets
; to reflect the states of the preference settings.

PrefsOK		tst.b	SerialState(a5)		serial on or off
		beq.s	1$			it's off
		lea.l	SerialGadget,a0
		ori.w	#SELECTED,gg_Flags(a0)
1$		tst.b	MonoState(a5)		mono on or off
		beq.s	2$			it's off
		lea.l	MonoGadget,a0
		ori.w	#SELECTED,gg_Flags(a0)
2$		tst.b	ColorState(a5)		color on or off
		beq.s	3$			it's off
		lea.l	ColorGadget,a0
		ori.w	#SELECTED,gg_Flags(a0)
3$		clr.w	d0
		move.b	RamBank(a5),d0		test ram bank
		cmpi.w	#1,d0			A000?
		bne.s	4$			no
		lea.l	A000Gadget,a0
		ori.w	#SELECTED,gg_Flags(a0)
		bra.s	6$
4$		cmpi.w	#2,d0			D000?
		bne.s	5$			no, must be E000
		lea.l	D000Gadget,a0
		ori.w	#SELECTED,gg_Flags(a0)
		bra.s	6$
5$		lea.l	E000Gadget,a0
		ori.w	#SELECTED,gg_Flags(a0)

; Here we go!  Open a workbench window to hold all our little gadgets.

6$		movea.l	Ilib(a5),a6		intuition lib ptr
		lea.l	prefwindow(pc),a0
		jsr	_LVOOpenWindow(a6)
		move.l	d0,PrefWindow(a5)	save the window pointer
		beq	exit4			what! no window ?

; if we are using a pc AT then the E000 gadget should have no effect

		movea.l	JIOBase(a5),a0		test AT/XT bit
		btst.b  #7,jio_PCconfiguration(a0)
		bne.s	10$			it's OK we're using XT
		lea.l	E000Gadget,a0		it's an AT, turn off E000
		movea.l	PrefWindow(a5),a1
		suba.l	a2,a2
		jsr	_LVOOffGadget(a6)

10$		movea.l	PrefWindow(a5),a0	get msgport address
		movea.l	wd_UserPort(a0),a0
		move.l	a0,MPort(a5)		and stash it for later
		moveq.l	#1,d1			make a signal mask
		move.b	MP_SIGBIT(a0),d0
		asl.l	d0,d1
		move.l	d1,MSigs(a5)		and stash that too
		jsr	MainProgram		wait for user responses

; Returns from the main program with FinishCode determining what to do
; 0 = do nothing because user selected CANCEL
; 1 = just stuff the new value into pc_Configuration, user selected USE
; 2 = save out the preferences and use because user selected SAVE

		move.w	FinishCode(a5),d0
		beq.s	CloseIt			exit time
		cmpi.w	#1,d0
		beq.s	UseIt			just use new values

; user wants to save these preferences as well as using them

SaveIt		movea.l	Dlib(a5),a6		DOS library pointer
		lea.l	Pname(pc),a0
		move.l	a0,d1			open prefs file
		move.l	#MODE_NEWFILE,d2	replace if existing
		jsr	_LVOOpen(a6)
		move.l	d0,PrefsFile(a5)	save the handle
		beq.s	UseIt			couldn't open it???
		move.l	d0,d1			prepare to write to it
		lea.l	SerialState(a5),a0
		move.l	a0,d2			buffer to write from
		moveq.l	#PREFSIZE,d3
		jsr	_LVOWrite(a6)		do the write
		move.l	PrefsFile(a5),d1	if it didn't write....
		jsr	_LVOClose(a6)		TOUGH!!

; combine all the flags in memory into a single control register byte

UseIt		move.b	RamBank(a5),d0		make RAM bits (5and6)
		asl.w	#5,d0
		or.b	SerialState(a5),d0
		move.b	ColorState(a5),d1
		asl.w	#4,d1
		or.b	d1,d0
		move.b	MonoState(a5),d1
		asl.w	#3,d1
		or.b	d1,d0
		ori.b	#%10000110,d0
		movea.l	JIOBase(a5),a0
		move.b	d0,jio_PCconfiguration(a0)	use settings

; all finished, close the window and libraries and exit.

CloseIt		movea.l	Ilib(a5),a6
		movea.l	PrefWindow(a5),a0	close the window
		jsr	_LVOCloseWindow(a6)

;=========================================================================
; These are the various exits from the program, no error codes returned
;=========================================================================

exit4		movea.l	_AbsExecBase,a6
		movea.l	Dlib(a5),a1		close DOS
		jsr	_LVOCloseLibrary(a6)
exit3		movea.l	Ilib(a5),a1		close intuition
		jsr	_LVOCloseLibrary(a6)
exit2		movea.l	Glib(a5),a1		close graphics
		jsr	_LVOCloseLibrary(a6)
exit1		adda.w	#mydata_SIZEOF,sp	scrap space on stack
		rts				all done

;=========================================================================
; Data has been left in the code section to allow PC relative addressing
;=========================================================================

Jname		DC.B	'janus.library',0
		CNOP	0,2
Gname		DC.B	'graphics.library',0
		CNOP	0,2
Iname		DC.B	'intuition.library',0
		CNOP	0,2
Dname		DC.B	'dos.library',0
		CNOP	0,2
Pname		DC.B	'sys:sidecar/2500config',0
		CNOP	0,2
preftitle	DC.B	'PC Configuration V1.0                    ',0
		CNOP	0,2

prefwindow	DC.W	170,50,300,83		left,top,width,height
		DC.B	2,1			detailpen,blockpen
		DC.L	REFRESHWINDOW!GADGETUP!GADGETDOWN
		DC.L	WINDOWDRAG!WINDOWDEPTH!SIMPLE_REFRESH
		DC.L	CancelGadget		first gadget
		DC.L	0			checkmark
		DC.L	preftitle		title *not relocatable*
		DC.L	0			screen
		DC.L	0			bitmap
		DC.W	0,0,0,0			min/max width/height
		DC.W	WBENCHSCREEN		window type

		END


