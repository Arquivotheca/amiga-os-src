**
**	$Id: open.asm,v 36.43 92/06/16 10:44:16 darren Exp $
**
**      open a console unit
**
**      (C) Copyright 1985,1987,1989,1990 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE	"exec/errors.i"
	INCLUDE	"exec/initializers.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE	"exec/ables.i"

	INCLUDE	"devices/input.i"
	INCLUDE	"intuition/intuitionbase.i"
	INCLUDE "internal/librarytags.i"
	INCLUDE	"debug.i"

**	Exports

	XDEF	CDOpen
	XDEF	CDClose
	XDEF	FreeBuffer


**	Imports

	XLVO	AddHead			; Exec
	XLVO	AllocMem		;
	XLVO	DoIO			;
	XLVO	FreeMem			;
	XLVO	InitStruct		;
	XLVO	ObtainSemaphore		;
	XLVO	OpenLibrary		;
	XLVO	TaggedOpenLibrary	; V39
	XLVO	CloseLibrary		;
	XLVO	ReleaseSemaphore	;
	XLVO	Remove			;
	XLVO	Permit			;

	XLVO	DisposeRegion		; Graphics
	XLVO	NewRegion		;

	XLVO	LockIBase		; Intuition
	XLVO	UnlockIBase		;

	XLVO	AskKeyMapDefault	; Keymap

	XREF	CDName

	XREF	WriteReset

	XREF	RefreshDamage		;refresh.asm

	XREF	LockDRPort		;able
	XREF	UnLockRPort

	XREF	ReSizeUnit		;wreset


**	Assumptions

	IFNE	CONU_LIBRARY+1
	FAIL	"constant CONU_LIBRARY not -1"
	ENDC

	IFNE	CONU_STANDARD
	FAIL	"constant CONU_STANDARD not 0"
	ENDC

	IFNE	CONU_CHARMAP&1-1
	FAIL	"low bit not set in constant CONU_CHARMAP"
	ENDC

	IFNE	CONU_SNIPMAP&1-1
	FAIL	"low bit not set in constant CONU_SNIPMAP"
	ENDC


******* console.device/OpenDevice **********************************************
*
*    NAME
*	OpenDevice -- a request to open a Console device
*
*    SYNOPSIS
*	error = OpenDevice("console.device", unit, IOStdReq, flags )
*	d0		    a0               d0    a1        d1
*
*    FUNCTION
*	The open routine grants access to a device.  There are two
*	fields in the IOStdReq block that will be filled in: the
*	io_Device field and possibly the io_Unit field.
*
*	As of (V37) the flags field may also be filled in with
*	a value described below (see conunit.h or conunit.i).
*
*	This open command differs from most other device open commands
*	in that it requires some information to be supplied in the
*	io_Data field of the IOStdReq block.  This initialization
*	information supplies the window that is used by the console
*	device for output.
*
*	The unit number that is a standard parameter for an open call
*	is used specially by this device.  See conunit.h, or conunit.i
*	for defined valid unit numbers.
*
*
*	unit number: -1 (CONU_LIBRARY)
*
*		Used to get a pointer to the device library vector
*	which is returned in the io_Device field of the IOStdReq
*	block.  No actual console is opened.  You must still close
*	the device when you are done with it.
*
*	unit number: 0 (CONU_STANDARD)
*
*		A unit number of zero binds the supplied window to
*	a unique console.  Sharing a console must be done at a level
*	higher than the device.
*
*
*	unit number: 1 (CONU_CHARMAP) (V36)
*
*		A unit number of one is similar to a unit number of
*	zero, but a console map is also created, and maintained by
*	the console.device.  The character map is used by the console
*	device to restore obscured portions of windows which are
*	revealed, and to redraw a window after a resize.  Character
*	mapped console.device windows must be opened as SIMPLE REFRESH
*	windows.
*
*		The character map is currently for internal use
*	only, and is not accessible by the programmer.  The character
*	map stores characters, attributes, and style information for
*	each character written with the CMD_WRITE command. 
*
*	unit number: 3 (CONU_SNIPMAP) (V36)
*
*		A unit number of three is similar to a unit number
*	of one, but also gives the user the ability to highlight
*	text with the mouse which can be copied by pressing
*	RIGHT AMIGA C.  See NOTES below.
*
*
*	flags: 0 (CONFLAG_DEFAULT)
*
*		The flags field should be set to 0 under V34, or less.
*
*	flags: 1 (CONFLAG_NODRAW_ON_NEWSIZE) (V37)
*
*		The flags field can be set to 0, or 1 as of V37.  The
*	flags field is ignored under V36, so can be set, though it
*	will have no effect.  When set to 1, it means that you don't
*	want the console.device to redraw the window when the window
*	size is changed (assuming you have opened the console.device
*	with a character map - unit numbers 1, or 3).  This flag is
*	ignored if you have opened a console.device with a unit
*	number of 0.  Typically you would use this flag when you
*	want to perform your own window refresh on a newsize, and
*	you want the benefits of a character mapped console.
*	
*    IO REQUEST
*	io_Data		struct Window *window
*			This is the window that will be used for this
*			console.  It must be supplied if the unit in
*			the OpenDevice call is 0 (see above).  The
*			RPort of this window is potentially in use by
*			the console whenever there is an outstanding
*			write command.
*    INPUTS
*	"console.device" - a pointer to the name of the device to be opened.
*	unit - the unit number to open on that device.
*	IOStdReq - a pointer to a standard request block
*	0 - a flag field of zero (CONFLAG_DEFAULT)
*	1 - a flag field of one  (CONFLAG_NODRAW_ON_NEWSIZE) (V37)
*
*    RESULTS
*	error - zero if successful, else an error is returned.
*	
*    NOTES
*	As noted above, opening the console.device with a unit number of 3
*	allows the user to drag select text, and copy the selection with
*	RIGHT AMIGA C.  The snip is copied to a private buffered managed
*	by the console.device (as of V36).  The snip can be copied to
*	any console.device window unless you are running a console to
*	clipboard utility such as that provided with V37.
*
*	The user pastes text into console.device windows by pressing
*	RIGHT AMIGA V.  Both RIGHT AMIGA V, and RIGHT AMIGA C are swallowed
*	by the console.device (unless you have asked for key presses as
*	RAW INPUT EVENTS).  Text pasted in this way appears in the
*	console read stream as if the user had typed all of the characters
*	manually.  Additional input (e.g., user input, RAW INPUT EVENTS)
*	are queued up after pastes.  Pastes can theoretically be quite
*	large, though they are no larger than the amount of text
*	which is visible in a console.device window.
*
*	When running the console to clipboard utility, text snips
*	are copied to the clipboard.device, and RIGHT AMIGA V key
*	presses are broadcast as an escape sequence as part of the 
*	console.device read stream ("<CSI>0 v" - $9B,$30,$20,$76).
*
*	It is left up to the application to decide what to do when this
*	escape sequence is received.  Ideally the application
*	will read the contents of the clipboard, and paste the text
*	by using successive writes to the console.device.
*
*	Because the contents of the clipboard.device can be quite
*	large, your program should limit the size of writes to something
*	reasonable (e.g., no more than 1K characters per CMD_WRITE, and
*	ideally no more than 256 characters per write).  Your program
*	should continue to read events from the console.device looking
*	for user input, and possibly RAW INPUT EVENTS.  How you decide
*	to deal with these events is left up to the application.
*
*	If you are using a character mapped console you should receive
*	Intuition events as RAW INPUT EVENTS from the console.device.
*	By doing this you will hear about these events after the console
*	device does.  This allows the console.device to deal with events
*	such as window resizing, and refresh before your application.
*
*    BUGS
*
*    SEE ALSO
*	exec/io.h, intuition/intuition.h
*
********************************************************************************

CDOpen:
		movem.l d2-d4/a2-a4,-(sp)
		move.l	d0,d2			; cache unit #
		move.l	a1,a3			; cache iorequest
		move.l	d1,d4			; cache flags field


;-- Not needed, we dont expunge!
;--
;--		bclr	#LIBB_DELEXP,LIB_FLAGS(a6)

		tst.w	LIB_OPENCNT(a6)
		bne.s	bumpOpen
		
	;-- better open Intuition now
	;-- 
	;-- if opening Intuition breaks Forbid(), we should be
	;-- fine since the next task would come along right behind
	;-- us only to find that LIB_OPENCNT is 0, and also try
	;-- to open Intuition.  At worst intuition is opened
	;-- twice which causes zilch problems for us.


		moveq	#OLTAG_INTUITION,d0
		LINKEXE	TaggedOpenLibrary
		move.l	d0,cd_IntuitionLib(a6)
                beq	openILError


	;-- Set up cd_ActiveWindow() value with currently active
	;-- window (0 isn't good enough for an initial value).

		;--	acquire active window
		move.l	a6,a2
		move.l	cd_IntuitionLib(a6),a6
		moveq	#0,d0
		CALLLVO	LockIBase
		move.l	ib_ActiveWindow(a6),cd_ActiveWindow(a2)
		move.l	d0,a0
		CALLLVO	UnlockIBase
		move.l	a2,a6

bumpOpen:
		addq.w	#1,LIB_OPENCNT(a6)		

	;-- check for use as a library only
	;-- unit will be equal to -1

		cmpi.l	#CONU_LIBRARY,d2
		bne.s	regularOpen
		move.l	d2,IO_UNIT(a3)
		bra	opendone

regularOpen:

	;-- screen for bogus unit #'s
	;-- as of V36, this leaves 0, 1, and 3 as good numbers

		cmpi.l	#CONU_SNIPMAP,d2
		bhi	openError

		cmpi.l	#2,d2		;grr - no good constant available
		beq	openError

	;-- make sure we have somekind of pointer to a window

		tst.l	IO_DATA(a3)
		beq	openError

	;------ try to allocate a unit
		move.l	#cu_SIZE,d0
		move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
		LINKEXE AllocMem
		tst.l	d0
		beq	openError
		move.l	d0,a2

	;------ initialize the unit
		lea	unitInitData(pc),a1
		move.l	#cu_SIZE,d0
		LINKEXE InitStruct

		move.l	d2,cu_DevUnit(a2)

		;-- applicable to V36 unit #'s for console map
		;-- 1 means no redraw on new size

		move.b	d4,cu_SpecialModes(a2)


	;------ get ClipRegion
		LINKGFX	NewRegion
		move.l	d0,cu_ClipRegion(a2)
		beq	openCRErr

	;------ initialize the unit command queue
		lea     MP_MSGLIST(a2),a0
		NEWLIST a0

	;------ get window parameter
		move.l	IO_DATA(a3),cu_Window(a2)

	;------ input key map
		LINKKEY	AskKeyMapDefault
		move.l	d0,a0
		lea	cu_KeyMapStruct(a2),a1
		moveq	#(km_SIZEOF/4)-1,d0
copyKeyMapStruct:
		move.l	(a0)+,(a1)+
		dbf	d0,copyKeyMapStruct
		
	;-- get a TmpRas and AreaPtrn memory
	;-- if this if the first window using the console.device
	;
	;-- Note that Forbid() could have been broken above
	;-- if we opened Intuition.  We are back in Forbid now to avoid
	;-- racing ahead of our buffer allocation, and to
	;-- safely determine initial cursor state (active/inactive)
	;-- for this window.
	;
	;-- Note that we'll look at console's idea of whats currently
	;-- active, and we'll get activation state changes later via
	;-- the console input handler.
	;

		tst.l	cd_WindowCount(a6)
		bne.s	addunit

		move.l	#4096,d0		; 1Kx32 bits
		move.l	d0,tr_Size+cd_TmpRas(a6)
		addq	#4,d0			; 2 words for AreaPtrn
		moveq	#MEMF_CHIP,d1
		LINKEXE	AllocMem
		move.l	d0,cd_AreaPtrn(a6)
		beq	openTmpError

		move.b	#1,cd_RastPort+rp_AreaPtSz(a6)
		addq	#4,d0
		move.l	d0,tr_RasPtr+cd_TmpRas(a6)
		lea	cd_TmpRas(a6),a0
		move.l	a0,rp_TmpRas+cd_RastPort(a6)

addunit:
	;-- increment open window count NOW!

		addq.l	#1,cd_WindowCount(a6)


	;-- Set initial cursor state, initialize unit struct, and 
	;-- add to list of console units.

		move.l	cd_ActiveWindow(a6),d0
		beq.s	notactive
		cmp.l	cu_Window(a2),d0
		bne.s	notactive
isActive:
		move.l	a2,cd_Active(a6)
		bset	#CUB_ISACTIVE,cu_States(a2)
notactive:

	;-- stash unit pointer in io request

		move.l	a2,IO_UNIT(a3)

	;-- set/reset console unit now, before adding it the list
	;-- of console units.
	;
	;-- WriteReset() does rendering, and will obtain layer's
	;-- locks while examining the window structure (e.g., size,
	;-- font, etc.)
	;
	;-- We can't safely have any layer's locks inside of the
	;-- exclusive semaphore below, or we can dead-lock because
	;-- Intuition may own LockLayerInfo, or a lock this
	;-- window's layer.
	;

		bsr     WriteReset		;can break Forbid()
		beq.s	nomap_fail

	;
	;-- Add to list of console units; semaphore is exclusive.
	;
	;-- The console inputhandler, and console task can look
	;-- at this list (shared semaphore), but neither can look
	;-- at the list while adding, or removing units from the
	;-- list (e.g., at OpenDevice time, and CloseDevice time).
	;


		lea	cd_USemaphore(a6),a0
		LINKEXE	ObtainSemaphore

		lea     cd_UHead(a6),a0
		move.l  a2,a1
		ADDHEAD

		lea	cd_USemaphore(a6),a0
		LINKEXE	ReleaseSemaphore

	;-- refresh cursor, or any damage that might have happened
	;-- after I inited the console unit, but before it was added
	;-- to the unit list

		bsr	LockDRPort

	;-- obtain activation state - layers are locked, no more changes
	;-- in activation can occur.
	;
	;-- cd_Active might change on us if:
	;
	;-- Closing an active console window (in which case its not this
	;-- one, and cd_Active won't match this unit anyway).
	;
	;-- If an input event occurs before we make the test, in which case
	;
	;    cd_Active is going to change soon - we'll catch it later, and
	;    can't actually render until we UnLockRPort, or
	;
	;    cd_Active has been changed by the inputhandler, in which
	;    case our test is in-sync with reality (at worst we handle
	;    the event later, and redraw yet one more time).
	;
	;    The test is atomic
	;


		bclr	#CUB_ISACTIVE,cu_States(a2)
		cmp.l	cd_Active(a6),a2		;atomic test
		bne.s	notCUActive
		bset	#CUB_ISACTIVE,cu_States(a2)

notCUActive:

	;
	;-- recalc window borders, and redraw everything, which isn't
	;-- much at this point
	;
		bsr	ReSizeUnit
		bsr	UnLockRPort

	;--
	;-- GFA BASIC KLUDGE!!!
	;--
	;-- Assumes that a0 contains windowptr on exit.

		movea.l	IO_DATA(a3),a0
	;--
	;-- Final exit point
	;--

opendone:
		moveq	#0,d0

		movem.l (sp)+,d2-d4/a2-a4
		rts

	;-- Allocation of the character map must have failed; cleanup

nomap_fail:

		;-- free TmpRas and AreaPtrn memory
		subq.l	#1,cd_WindowCount(a6)	;atomic
		bne.s	usingtmpras

		movea.l	cd_AreaPtrn(a6),a1
		move.l	#4100,d0		; 1Kx32 + 4
		LINKEXE	FreeMem

usingtmpras:
		cmp.l	cd_Active(a6),a2	;still the active window?
		bne.s	notcdactive
		clr.l	cd_Active(a6)		;no console window active for now

notcdactive:
	;-- free up memory so far

openTmpError:

		move.l	cu_ClipRegion(a2),a0
		LINKGFX	DisposeRegion


	;-- Release memory for console unit structure

openCRErr:
		move.l	a2,a1
		move.l	#cu_SIZE,d0
		LINKEXE FreeMem


	;-- Error, decrement usage counter

openError:
		subq.w	#1,LIB_OPENCNT(a6)

	;-- Set error, and clear IO_DEVICE field

openILError:
		moveq	#IOERR_OPENFAIL,d0
		clr.l	IO_DEVICE(a3)
		move.b	d0,IO_ERROR(a3)
		bra.s	opendone


unitInitData:
	;------ initialize the unit command queue
	    INITLONG	LN_NAME,CDName
	    INITBYTE	LN_TYPE,NT_MSGPORT
	    INITLONG	LN_NAME,CDName
	    INITBYTE	MP_MSGLIST+LH_TYPE,NT_MESSAGE

	    INITBYTE	cu_RPNestCnt,-1		; initially "dropped"
	    INITBYTE	cu_CDNestCnt,-1		; initially enabled (and hide)
*	    INITBYTE	cu_CursorFlags,CUF_CURSQDRAW!CUF_CURSNDRAW
*	    INITBYTE    cu_IHead,0		;head of token list (input)
*	    INITBYTE	cu_ITail,0		;tail of token list (task)
	    INITBYTE	cu_States,CUF_FIRSTTIME	;First time through!

		dc.w	0


*****i* console.device/CloseDevice ************************************
*
*    NAME
*       Close -- close the console device
*
*    SYNOPSIS
*       CloseDevice(IOStdReq)
*
*    FUNCTION
*	This function closes software access to the console device,
*	and informs the system that access to this device/unit which was
*	previously opened has been concluded.  The device may perform
*	certain house-cleaning	operations.  The I/O request structure
*	is now free to be recycled.
*
*    INPUTS
*	IOStdReq - pointer to an IOStdReq structure, set by OpenDevice
*
*    BUGS
*
*    SEE ALSO
*	console.device/function/OpenDevice, exec/io.h
*
********************************************************************************


*------ console.device/Close -----------------------------------------
*
*   NAME
*	Close - terminate access to a device
*
*   SYNOPSIS
*	Close(iORequest), consoleDev
*	      a1	  a6
*
*   FUNCTION
*	The close routine notifies a device that it will no longer
*	be using the device.  The driver will clear the IO_DEVICE
*	and IO_UNIT entries in the iORequest structure.
*
*	The open count on the device will be decremented, and if it
*	falls to zero and an Expunge is pending, the Expunge will
*	take place.
*
*---------------------------------------------------------------------
CDClose:
		movem.l	a2/a3,-(sp)
		move.l	a1,a3		    ; save I/O Request
		move.l	IO_UNIT(a3),a2
		cmpa	#-1,a2		; check if open only for library
		beq	closeLibrary


		;-- ensure not highlighting here
		btst	#CUB_SELECTED,cu_Flags(a2)
		beq.s	closeUnlink

		and.b	#CDS_SELECTMASK,cd_SelectFlags(a6)
		clr.l	cd_SelectedUnit(a6)

		;-- unlink from the device unit list
closeUnlink:
		lea	cd_USemaphore(a6),a0
		LINKEXE	ObtainSemaphore

		move.l	a2,a1
		lea	cd_UHead(a6),a0
		LINKEXE Remove
		cmp.l	cd_Active(a6),a2
		bne.s	closeUnit
		clr.l	cd_Active(a6)

closeUnit:
		;-- free TmpRas and AreaPtrn memory
		subq.l	#1,cd_WindowCount(a6)
		bne.s	leaveTmpRas		

		movea.l	cd_AreaPtrn(a6),a1
		move.l	#4100,d0		; 1Kx32 + 4
		LINKEXE	FreeMem

leaveTmpRas:

		lea	cd_USemaphore(a6),a0
		LINKEXE	ReleaseSemaphore

	    ;-- do unit specific close stuff
		move.l	cu_ClipRegion(a2),a0
		LINKGFX	DisposeRegion

		bsr.s	FreeBuffer

		move.l	a2,a1
		move.l	#cu_SIZE,d0
		LINKEXE FreeMem

	    ;-- do generic close stuff
		move.l	a3,a1
closeLibrary:
		movem.l	(sp)+,a2/a3

	    ;-- clear out the pointers
		moveq	#0,d0
		move.l	d0,IO_UNIT(a1)
		move.l	d0,IO_DEVICE(a1)

	    ;-- check if this is the last close

		subq.w	#1,LIB_OPENCNT(a6)
		bne.s	closeRts

		move.l	cd_IntuitionLib(a6),a1
		LINKEXE	CloseLibrary

closeRts:
		rts


FreeBuffer:
		move.l	cu_CM+cm_AllocSize(a2),d0
		beq.s	fbRts
		move.l	cu_CM+cm_AllocBuffer(a2),d1
		beq.s	fbClearSize
		move.l	d1,a1
		move.l	a6,-(a7)
		move.l	cd_ExecLib(a6),a6
		CALLLVO	FreeMem
		move.l	(a7)+,a6

fbClearSize:
		clr.l	cu_CM+cm_AllocSize(a2)

fbRts:
		rts


	END
