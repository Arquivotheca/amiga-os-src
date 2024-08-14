	TTL	'$Id: keydev.asm,v 40.0 93/03/08 10:57:21 darren Exp $
**********************************************************************
*
*			---------------
*   keydev.asm		KEYBOARD DEVICE		device functions
*			---------------
*
*   Copyright 1985, 1987 Commodore-Amiga Inc.
*
*   Source Control	$Locker:  $
*
*   $Log:	keydev.asm,v $
*   Revision 40.0  93/03/08  10:57:21  darren
*   use tagged open device to open timer.device - saves ROM
*   
*   Revision 36.10  90/04/13  12:44:00  kodiak
*   use Id instead of Header for 4.x rcs
*   
*   Revision 36.9  90/04/02  16:30:19  kodiak
*   back to using dd_ExecBase, not SYSBASE (4)
*   
*   Revision 36.8  90/04/02  13:00:29  kodiak
*   for rcs 4.x header change
*   
*   Revision 36.7  89/08/31  17:27:24  kodiak
*   perform key handshake timeout w/ timer ReadEClock function
*   
*   Revision 36.6  89/07/31  17:14:06  kodiak
*   remove timer cia-a-a use in preparation for timer device use of it
*   
*   Revision 36.5  89/06/09  12:12:35  kodiak
*   autodoc changes
*   
*   Revision 36.4  89/03/25  14:09:36  kodiak
*   don't destroy TOD when initting keyboard
*   allocate timer A
*   use keyint's handshake routines ensuring 75us pulse width
*   
*   Revision 36.3  89/02/20  13:02:36  kodiak
*   make names reflect usage: CIA-A, not B
*   
*   Revision 36.2  88/11/03  12:35:30  kodiak
*   use AUTOINIT in ResidentTag
*   
*   Revision 36.1  88/08/02  12:22:01  kodiak
*   first cut at timer-based handshake
*   
*   Revision 36.0  88/04/05  12:38:36  kodiak
*   remove dd_ExecBase references
*   
*   Revision 35.0  87/10/26  11:32:07  kodiak
*   initial from V34, but w/ stripped log
*   
**********************************************************************

	SECTION		rawinput

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/devices.i"
	INCLUDE		"exec/tasks.i"
	INCLUDE		"exec/io.i"
	INCLUDE		"exec/strings.i"
	INCLUDE		"exec/interrupts.i"
	INCLUDE		"exec/execbase.i"
	INCLUDE		"exec/errors.i"
	INCLUDE		"exec/initializers.i"

	INCLUDE		"hardware/cia.i"

	INCLUDE		"macros.i"
	INCLUDE		"kbdata.i"


*------ Imported Globals ---------------------------------------------

	XREF		KDName
	XREF		VERSION
	XREF		REVISION


*------ Imported Functions -------------------------------------------

	XREF_EXE	CloseDevice
	XREF_EXE	FindName
	XREF_EXE	OpenDevice
	XREF_EXE	OpenResource

	XREF_CIA	AbleICR
	XREF_CIA	AddICRVector
	XREF_CIA	RemICRVector

	XREF		BeginIO
	XREF		AbortIO
	XREF		SuccessfulExpunge
	XREF		DeferredExpunge
	XREF		ExtFunc
	XREF		Close

	XREF		Invalid
	XREF		Read
	XREF		Write
	XREF		Update
	XREF		KDClear
	XREF		StopCmd
	XREF		Start
	XREF		Flush

	XREF		KDReadEvent
	XREF		KDReadMatrix
	XREF		KDAddResetHandler
	XREF		KDRemResetHandler
	XREF		KDResetHandlerDone

	XREF		CBInvalid
	XREF		CBRead
	XREF		CBWrite
	XREF		CBUpdate
	XREF		KDCBClear
	XREF		CBStopCmd
	XREF		CBStart
	XREF		CBFlush

	XREF		KDCBReadEvent
	XREF		KDCBReadMatrix
	XREF		KDCBAddResetHandler
	XREF		KDCBRemResetHandler
	XREF		KDCBResetHandlerDone

	XREF		KDInterrupt
	XREF		KeyHandshakeBegin
	XREF		KeyHandshakeEnd


*------ Exported Globals ---------------------------------------------

	XDEF		KDFuncInit
	XDEF		KDStructInit
	XDEF		KDInit



**********************************************************************
KDInit:
		movem.l a6,-(sp)
*	    ;------ This is called from within MakeLibrary, after
*	    ;------ all the memory has been allocated

		exg	d0,a6			; save device library pointer
		move.l	d0,dd_ExecBase(a6)
		move.l	a0,dd_Segment(a6)

*	    ;------ initialize the unit command queue
		lea	kd_Unit+MP_MSGLIST(a6),a0
		NEWLIST	a0

	    ;------ initialize the reset handler list
		lea	kd_HandlerList(a6),a0
		NEWLIST	a0

*	    ;-- open timer.device for ReadEClock
		suba.l	a0,a0			;ODTAG_TIMER (save some ROM)
	;;	lea	TDName(pc),a0
		moveq	#UNIT_ECLOCK,d0
		lea	kd_Tick(a6),a1
		moveq	#0,d1
		LINKEXE	OpenDevice
		tst.l	d0
		bne.s	initErr			; (not very robust)

*	    ;------ open the CIA-A for the keyboard
		lea	CIAAName(pc),a1
		moveq	#0,d0
		LINKEXE OpenResource
		move.l	d0,kd_CIAAResource(a6)
		beq.s	initErr			; (not very robust)

		bsr	KeyHandshakeBegin

*	;-- set up the CIA interrupt routine for keyboard shift-in
*	    ;-- constant initialization already done
		move.l	a6,kd_IS+IS_DATA(a6)
		moveq	#CIAICRB_SP,d0
		lea	kd_IS(a6),a1
		LINKCIA AddICRVector

		bsr	KeyHandshakeEnd

initErr:
		move.l	a6,d0			; return library pointer
		movem.l (sp)+,a6
		rts

CIAAName:
		STRING	<'ciaa.resource'>


*----------------------------------------------------------------------
KDStructInit:
*	;------ initialize the device library structure
		INITBYTE	LN_TYPE,NT_DEVICE
		INITLONG	LN_NAME,KDName
		INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD	LIB_VERSION,VERSION
		INITWORD	LIB_REVISION,REVISION

		INITLONG	dd_CmdVectors,kdCmdTable
		INITLONG	dd_CmdBytes,kdCmdBytes
		INITWORD	dd_NumCommands,KD_NUMCOMMANDS

*	;------ initialize kb_IS
		INITBYTE	kd_IS+LN_TYPE,NT_INTERRUPT
		INITLONG	kd_IS+LN_NAME,KDName
		INITLONG	kd_IS+IS_CODE,KDInterrupt

*	;------ initialize the unit command queue
		INITBYTE	kd_Unit+LN_TYPE,NT_MSGPORT
		INITLONG	kd_Unit+LN_NAME,KDName

		dc.w	0


KDFuncInit:
		dc.w	-1

		dc.w	KDOpen-KDFuncInit
		dc.w	Close+(*-KDFuncInit)
		dc.w	KDExpunge-KDFuncInit
		dc.w	ExtFunc+(*-KDFuncInit)

		dc.w	BeginIO+(*-KDFuncInit)
		dc.w	AbortIO+(*-KDFuncInit)

		dc.w	-1


kdCmdTable:
		dc.l	Invalid
		dc.l	KDReset
		dc.l	Read
		dc.l	Write
		dc.l	Update
		dc.l	KDClear
		dc.l	StopCmd
		dc.l	Start
		dc.l	Flush

		dc.l	KDReadEvent
		dc.l	KDReadMatrix
		dc.l	KDAddResetHandler
		dc.l	KDRemResetHandler
		dc.l	KDResetHandlerDone

kdCmdBytes:
		dc.b	CBInvalid
		dc.b	KDCBReset
		dc.b	CBRead
		dc.b	CBWrite
		dc.b	CBUpdate
		dc.b	KDCBClear
		dc.b	CBStopCmd
		dc.b	CBStart
		dc.b	CBFlush

		dc.b	KDCBReadEvent
		dc.b	KDCBReadMatrix
		dc.b	KDCBAddResetHandler
		dc.b	KDCBRemResetHandler
		dc.b	KDCBResetHandlerDone

KD_NUMCOMMANDS	EQU	(*-kdCmdBytes)

		ds.w	0

*****i* keyboard.device/CMD_RESET ************************************
*
*   NAME
*	CMD_RESET -- Reset the keyboard.
*
*   FUNCTION
*	CMD_RESET resets the keyboard device without destroying handles
*	to the open device.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Command	CMD_RESET
*	io_Flags	IOB_QUICK set if quick I/O is possible
*
**********************************************************************
KDCBReset	EQU	-1

KDReset:
		bsr	StopCmd
		bsr	KeyHandshakeBegin
		bsr	Flush
		bsr	KeyHandshakeEnd
		bsr	Start
		rts


*------ keyboard.device/Expunge --------------------------------------
*
*   NAME
*	Expunge - indicate a desire to remove the Keyboard device
*
*   SYNOPSIS
*	segment = Expunge(), keyboardDev
*	d0		     a6
*
*   FUNCTION
*	The Expunge routine is called when a user issues a RemDevice
*	call.  The existance of any other users of the device, as
*	determined by the library open count being non-zero, will
*	cause the Expunge to be deferred.  When the device is not
*	in use, or no longer in use, the Expunge is actually
*	performed, and the device removed from the system list.
*
*---------------------------------------------------------------------
KDExpunge:
	    ;-- see if any one is using the device
		tst.w	LIB_OPENCNT(a6)
		bne	DeferredExpunge

*	    ;-- this is really it.  Free up all the resources

		;-- free timer.device
		move.l	kd_Tick(a6),d0
		beq.s	exCIA

		move.l	d0,a1
		LINKEXE	CloseDevice

*		;-- disable keyboard interrupts
exCIA:
		moveq	#CIAICRF_SP,d0
		LINKCIA AbleICR		    ; disable shift
*		;-- remove the interrupt service routines
		moveq	#CIAICRB_SP,d0
		lea	kd_IS(a6),a1
		LINKCIA	RemICRVector
		bra	SuccessfulExpunge


*------ keyboard.device/Open -----------------------------------------
*
*   NAME
*	Open - a request to open the Keyboard device
*
*   SYNOPSIS
*	Open(iORequest), keyboardDev
*	     a1		 a6
*
*   FUNCTION
*	The open routine grants access to a device.  There are two
*	fields in the iORequest block that will be filled in: the
*	IO_DEVICE field has already been initialized by OpenDevice;
*	the Open routine will to initialize the IO_UNIT field as
*	appropriate.
*
*	The device open count will be incremented.  The device cannot
*	be expunged unless this open is matched by a Close device.
*
*   RESULTS
*	If the open was unsuccessful, IO_ERROR will be set, IO_UNIT
*	and IO_DEVICE will not be valid.
*
*---------------------------------------------------------------------
KDOpen:
	    ;-- this device has only one unit
		lea	kd_Unit(a6),a0
		move.l	a0,IO_UNIT(a1)

		bclr	#LIBB_DELEXP,LIB_FLAGS(a6)
		addq.w	#1,LIB_OPENCNT(a6)
		rts

		END
