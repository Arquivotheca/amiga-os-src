**
**	$Id: inputdev.asm,v 40.1 93/03/12 16:03:07 darren Exp $
**
**	input device init and functions
**
**	(C) Copyright 1985,1986,1987,1988 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
	SECTION		rawinput

**	Includes

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
	INCLUDE		"exec/errors.i"
	INCLUDE		"exec/initializers.i"
	INCLUDE		"exec/execbase.i"

	INCLUDE		"devices/timer.i"

	INCLUDE		"keyboard.i"
	INCLUDE		"gameport.i"
	INCLUDE		"inputevent.i"

	INCLUDE		"macros.i"
	INCLUDE		"stddevice.i"
	INCLUDE		"iddata.i"


**	Exports

	XDEF		IDFuncInit
	XDEF		IDStructInit
	XDEF		IDInit


**	Imports
	XREF		KDName
	XREF		IDName
	XREF		VERSION
	XREF		REVISION


	XREF_EXE	AddTask
	XREF_EXE	CloseDevice
	XREF_EXE	FreeMem
	XREF_EXE	OpenDevice
	XREF_EXE	Wait

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
	XREF		IDClear
	XREF		StopCmd
	XREF		IDStart
	XREF		Flush

	XREF		IDSetThresh
	XREF		IDSetPeriod
	XREF		IDSetMPort
	XREF		IDSetMType
	XREF		IDSetMTrig

	XREF		CBInvalid
	XREF		CBRead
	XREF		CBWrite
	XREF		CBUpdate
	XREF		IDCBClear
	XREF		CBStopCmd
	XREF		IDCBStart
	XREF		CBFlush

	XREF		IDCBSetThresh
	XREF		IDCBSetPeriod
	XREF		IDCBSetMPort
	XREF		IDCBSetMType
	XREF		IDCBSetMTrig

	XREF		IDTaskStart



**********************************************************************
IDInit:
		movem.l a2-a3/a6,-(sp)
*	    ;------ This is called from within MakeLibrary, after
*	    ;------ all the memory has been allocated

		exg	d0,a6			; save device library pointer
		move.l	d0,dd_ExecBase(a6)
		move.l	a0,dd_Segment(a6)

		lea	id_Unit+MP_MSGLIST(a6),a0
		NEWLIST	a0

	;------ open the Timer device
		suba.l	a0,a0			;Tagged way for V40 (save ROM)

	;	lea	TDName(pc),a0
		moveq	#UNIT_VBLANK,d0		; VBLANK TIMER
		moveq	#0,d1
		lea	id_RIOR(a6),a1
		LINKEXE OpenDevice
		tst.l	d0
		bne	initTDErr

		move.l	id_RIOR+IO_DEVICE(a6),id_TIOR+IO_DEVICE(a6)
		move.l	id_RIOR+IO_UNIT(a6),id_TIOR+IO_UNIT(a6)

	;------ open the Keyboard device
		lea	KDName(pc),a0
		moveq	#0,d0
		moveq	#0,d1
		lea	id_K1IOR(a6),a1
		LINKEXE OpenDevice
		tst.l	d0
		bne	initKDErr

		move.l	id_K1IOR+IO_DEVICE(a6),id_K2IOR+IO_DEVICE(a6)
		move.l	id_K1IOR+IO_UNIT(a6),id_K2IOR+IO_UNIT(a6)

	;------ initialize the input handler list
		lea	id_HandlerList(a6),a0
		NEWLIST	a0

	;------ initialize input device task
		lea	id_TC(a6),a1
		lea	id_Stk(a6),a0
		move.l	a0,TC_SPLOWER(a1)
		move.w	#(ID_STKSIZE/2)-1,d0
		move.w	#$0A6A6,d1
pattern0Loop:
		move.w	d1,(a0)+
		dbf	d0,pattern0Loop
	;---	LEA	id_Stk+ID_STKSIZE(a6),a0
		move.l	a0,TC_SPUPPER(a1)
		move.l	a6,-(a0)		; argument (input data)
		move.l	a0,TC_SPREG(a1)

		move.l	dd_ExecBase(a6),a0
		move.l	ThisTask(a0),id_Stk(a6) ; cache this task

		lea	IDTaskStart(pc),a2
		suba.l	a3,a3
		LINKEXE AddTask

		moveq	#SIGF_SINGLE,d0
		LINKEXE	Wait

		move.l	a6,d0

itInitRts:
		movem.l (sp)+,a2-a3/a6
		rts

initKDErr:
		lea	id_TIOR(a6),a1
		CALLEXE CloseDevice

initTDErr:
		move.l	a6,a1
		move.w	LIB_NEGSIZE(a6),d0
		suba.w	d0,a1
		add.w	LIB_POSSIZE(a6),d0
		ext.l	d0
		LINKEXE FreeMem

initMemErr:
		moveq	#0,d0
		bra	itInitRts



*----------------------------------------------------------------------
*
* Definitions for Initialization
*
*----------------------------------------------------------------------
IDStructInit:
*	;------ initialize the device library structure
		INITBYTE	LN_TYPE,NT_DEVICE
		INITLONG	LN_NAME,IDName
		INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD	LIB_VERSION,VERSION
		INITWORD	LIB_REVISION,REVISION

		INITLONG	dd_CmdVectors,idCmdTable
		INITLONG	dd_CmdBytes,idCmdBytes
		INITWORD	dd_NumCommands,ID_NUMCOMMANDS

*	;------ initialize the unit command queue
		INITBYTE	id_Unit+LN_TYPE,NT_MSGPORT
		INITLONG	id_Unit+LN_NAME,IDName

*	;------ Initialize the tasks
		INITBYTE	id_TC+LN_TYPE,NT_TASK
		INITBYTE	id_TC+LN_PRI,ID_PRIORITY
		INITLONG	id_TC+LN_NAME,IDName

*	;------ Initialize the repeat rates
		INITLONG	id_Thresh+TV_SECS,ID_THRESHSECS
		INITLONG	id_Thresh+TV_MICRO,ID_THRESHMICRO
		INITLONG	id_Period+TV_SECS,ID_PERIODSECS
		INITLONG	id_Period+TV_MICRO,ID_PERIODMICRO

		INITWORD	id_RepeatCode,-1

	;------ Initialize the mouse controller
		INITBYTE	id_MType,GPCT_MOUSE
		INITWORD	id_MTrig+gpt_Keys,GPTF_DOWNKEYS!GPTF_UPKEYS
		INITLONG	id_MTrig+gpt_XDelta,$00010001

		dc.w	0


IDFuncInit:
		dc.w	-1

		dc.w	IDOpen-IDFuncInit
		dc.w	Close+(*-IDFuncInit)
		dc.w	IDExpunge-IDFuncInit
		dc.w	ExtFunc+(*-IDFuncInit)

		dc.w	BeginIO+(*-IDFuncInit)
		dc.w	AbortIO+(*-IDFuncInit)

		dc.w	PeekQualifier-IDFuncInit

		dc.w	-1


idCmdTable:
		dc.l	Invalid
		dc.l	IDReset
		dc.l	Read
		dc.l	Write
		dc.l	Update
		dc.l	IDClear
		dc.l	StopCmd
		dc.l	IDStart
		dc.l	Flush

		dc.l	IDTaskCommand	; IDAddHandler
		dc.l	IDTaskCommand	; IDRemHandler
		dc.l	IDTaskCommand	; IDWriteEvent
		dc.l	IDSetThresh
		dc.l	IDSetPeriod
		dc.l	IDTaskCommand	; IDSetMPort
		dc.l	IDTaskCommand	; IDSetMType
		dc.l	IDTaskCommand	; IDSetMTrig

idCmdBytes:
		dc.b	CBInvalid
		dc.b	IDCBReset
		dc.b	CBRead
		dc.b	CBWrite
		dc.b	CBUpdate
		dc.b	IDCBClear
		dc.b	CBStopCmd
		dc.b	IDCBStart
		dc.b	CBFlush

		dc.b	IDCBTaskCommand	; IDCBAddHandler
		dc.b	IDCBTaskCommand	; IDCBRemHandler
		dc.b	IDCBTaskCommand	; IDCBWriteEvent
		dc.b	IDCBSetThresh
		dc.b	IDCBSetPeriod
		dc.b	IDCBTaskCommand	; IDCBSetMPort
		dc.b	IDCBTaskCommand	; IDCBSetMType
		dc.b	IDCBTaskCommand	; IDCBSetMTrig

ID_NUMCOMMANDS	EQU	(*-idCmdBytes)

		ds.w	0

*****i* input.device/command/Reset ***********************************
*
*   NAME
*	Reset - reset the input device
*
*   FUNCTION
*	Reset resets the input device without destroying handles
*	to the open device.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	CMD_RESET
*	io_Flags	IOB_QUICK set if quick I/O is possible
*
**********************************************************************
IDCBReset	EQU	-1

IDReset:
		bsr	StopCmd
		bsr	Flush
		bsr	IDStart
		rts


*------ TaskCommand --------------------------------------------------
*
*   FUNCTION
*	Keep the command on the message queue till task time.
*
*---------------------------------------------------------------------
IDCBTaskCommand	EQU	0

IDTaskCommand:
		bclr    #IOB_QUICK,IO_FLAGS(a1)	; jimm: 8/1/86
		rts


*------ Expunge ------------------------------------------------------
*
*   NAME
*	Expunge - indicate a desire to remove this device
*
*   SYNOPSIS
*	segment = Expunge(), inputDev
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
IDExpunge:
	    ;-- see if any one is using the device
		tst.w	LIB_OPENCNT(a6)
		bne	DeferredExpunge

*	    ;-- this is really it.  Free up all the resources

;!!!		BRA	SuccessfulExpunge
		bra	DeferredExpunge


*------ input.device/Open *****************************************
*
*   NAME
*	Open - a request to open the input device
*
*   SYNOPSIS
*	OpenDevice("input.device", 0, iORequest, 0)
*
*   FUNCTION
*	The open routine grants access to a device.  There are two
*	fields in the iORequest block that will be filled in: the
*	IO_DEVICE field and the IO_UNIT field.
*
*	The device open count will be incremented.  The device cannot
*	be expunged unless this open is matched by a Close device.
*
*   RESULTS
*	If the open was unsuccessful, IO_ERROR will be set, IO_UNIT
*	and IO_DEVICE will not be valid.
*
**********************************************************************
IDOpen:
	    ;-- this device has only one unit
		lea	id_Unit(a6),a0
		move.l	a0,IO_UNIT(a1)

		bclr	#LIBB_DELEXP,LIB_FLAGS(a6)
		addq.w	#1,LIB_OPENCNT(a6)
		rts

******* input.device/PeekQualifier ***********************************
*
*   NAME
*	PeekQualifier -- get the input device's current qualifiers (V36)
*
*   SYNOPSIS
*	qualifier = PeekQualifier()
*	d0
*
*	UWORD PeekQualifier( VOID );
*
*   FUNCTION
*	This function takes a snapshot of what the input device thinks
*	the current qualifiers are.
*
*   RESULTS
*	qualifier - a word with the following bits set according to
*	    what the input device knows their state to be:
*		IEQUALIFIER_LSHIFT, IEQUALIFIER_RSHIFT,
*		IEQUALIFIER_CAPSLOCK, IEQUALIFIER_CONTROL, 
*		IEQUALIFIER_LALT, IEQUALIFIER_RALT,
*		IEQUALIFIER_LCOMMAND, IEQUALIFIER_RCOMMAND,
*		IEQUALIFIER_LEFTBUTTON, IEQUALIFIER_RBUTTON,
*		IEQUALIFIER_MIDBUTTON
*
*   NOTE
*	This function is new for V36.
*
*   SEE ALSO
*	devices/inputevent.h
*
**********************************************************************
PeekQualifier:
		move.w	id_Qualifier(a6),d0
		and.l	#$70ff,d0
		rts

	END
