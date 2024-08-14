	TTL	'$Id: gamedev.asm,v 36.8 90/05/04 10:30:27 kodiak Exp $'
**********************************************************************
*
*			---------------
*   gameport.asm	GAMEPORT DEVICE		device functions
*			---------------
*
*   Copyright 1985, 1987 Commodore-Amiga Inc.
*
*   Source Control	$Locker:  $
*
*   $Log:	gamedev.asm,v $
*   Revision 36.8  90/05/04  10:30:27  kodiak
*   remove gameport unit exclusion, but keep initialization upon first open
*   
*   Revision 36.7  90/04/13  12:43:29  kodiak
*   use Id instead of Header for 4.x rcs
*   
*   Revision 36.6  90/04/02  16:28:25  kodiak
*   back to using dd_ExecBase, not SYSBASE (4)
*   
*   Revision 36.5  90/04/02  12:56:44  kodiak
*   for rcs 4.x header change
*   
*   Revision 36.4  90/01/04  13:26:24  kodiak
*   keep seperate unit open count for exclusive access and fix exclusive access
*   
*   Revision 36.3  89/11/10  13:39:16  kodiak
*   make gameport exclusive access
*   
*   Revision 36.2  88/11/03  12:35:17  kodiak
*   use AUTOINIT in ResidentTag
*   
*   Revision 35.1  88/08/02  12:26:50  kodiak
*   use SYSBASE (4) vs. dd_ExecBase
*   (a poor decision, in hindsight)
*   
*   Revision 35.0  87/10/26  11:30:50  kodiak
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
	INCLUDE		"exec/errors.i"
	INCLUDE		"exec/initializers.i"
	INCLUDE		"exec/execbase.i"

	INCLUDE		"hardware/cia.i"
	INCLUDE		"hardware/intbits.i"

	INCLUDE		"resources/potgo.i"

	INCLUDE		"macros.i"
	INCLUDE		"gpdata.i"


*------ Imported Globals ---------------------------------------------

	XREF		_ciaa

	XREF		GDName
	XREF		VERSION
	XREF		REVISION


*------ Imported Functions -------------------------------------------

	XREF_EXE	AddIntServer
	XREF_EXE	OpenResource
	XREF_EXE	RemIntServer

	XREF		BeginIO
	XREF		AbortIO
	XREF		SuccessfulExpunge
	XREF		DeferredExpunge
	XREF		ExtFunc
	XREF		Close

	XREF		Invalid
	XREF		ResetCmd
	XREF		Read
	XREF		Write
	XREF		Update
	XREF		GDClear
	XREF		StopCmd
	XREF		Start
	XREF		Flush

	XREF		GDReadEvent
	XREF		GDAskCType
	XREF		GDSetCType
	XREF		GDAskTrigger
	XREF		GDSetTrigger

	XREF		CBInvalid
	XREF		CBResetCmd
	XREF		CBRead
	XREF		CBWrite
	XREF		CBUpdate
	XREF		GDCBClear
	XREF		CBStopCmd
	XREF		CBStart
	XREF		CBFlush

	XREF		GDCBReadEvent
	XREF		GDCBAskCType
	XREF		GDCBSetCType
	XREF		GDCBAskTrigger
	XREF		GDCBSetTrigger

	XREF		GDInterrupt


*------ Exported Globals ---------------------------------------------

	XDEF		GDFuncInit
	XDEF		GDStructInit
	XDEF		GDInit


**********************************************************************
GDInit:
		movem.l a6,-(sp)
*	    ;------ This is called from within MakeLibrary, after
*	    ;------ all the memory has been allocated

		exg	d0,a6			; save device library pointer
		move.l	d0,dd_ExecBase(a6)
		move.l	a0,dd_Segment(a6)

*	    ;------ get the potgo resource
		lea	PRName(pc),a1
		LINKEXE	OpenResource
		move.l	d0,gd_PotgoResource(a6)

*	    ;------ initialize the unit command queues
		lea	gd_Unit0+MP_MSGLIST(a6),a0
		NEWLIST	a0
		lea	gd_Unit1+MP_MSGLIST(a6),a0
		NEWLIST	a0

*	;-- set up the vertical blank interrupt routine for gameport
*	    ;-- constant initialization already done
		move.l	a6,gd_IS+IS_DATA(a6)
		moveq	#INTB_VERTB,d0
		lea	gd_IS(a6),a1
		LINKEXE AddIntServer

*       ;-- set 6522 GamePort button 0s to input
		move.l	#_ciaa,a0
CIAFS_GAMEPORT	EQU	CIAF_GAMEPORT0!CIAF_GAMEPORT1
	    ;------- port A button pins are input
		andi.b	#(~CIAFS_GAMEPORT)&$0FF,ciaddra(a0)
		or.b	#CIAFS_GAMEPORT,ciapra(a0)	; pulled high

		move.l	a6,d0		    ;return library pointer
		movem.l (sp)+,a6
		rts

PRName		POTGONAME


*----------------------------------------------------------------------
GDStructInit:
*	;------ initialize the device library structure
		INITBYTE	LN_TYPE,NT_DEVICE
		INITLONG	LN_NAME,GDName
		INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD	LIB_VERSION,VERSION
		INITWORD	LIB_REVISION,REVISION

		INITLONG	dd_CmdVectors,gdCmdTable
		INITLONG	dd_CmdBytes,gdCmdBytes
		INITWORD	dd_NumCommands,GD_NUMCOMMANDS

*	;------ initialize kb_IS
		INITBYTE	gd_IS+LN_TYPE,NT_INTERRUPT
		INITBYTE	gd_IS+LN_PRI,GPIS_PRIORITY
		INITLONG	gd_IS+LN_NAME,GDName
		INITLONG	gd_IS+IS_CODE,GDInterrupt

*	;------ initialize the unit command queues
		INITBYTE	gd_Unit0+LN_TYPE,NT_MSGPORT
		INITLONG	gd_Unit0+LN_NAME,GDName
		INITBYTE	gd_Unit1+LN_TYPE,NT_MSGPORT
		INITLONG	gd_Unit1+LN_NAME,GDName

*       ;------ Initialize the units
		INITBYTE	gd_Unit1+gu_Flags,UF_RIGHTPORT

		dc.w	0


GDFuncInit:
		dc.w	-1

		dc.w	GDOpen-GDFuncInit
		dc.w	GDClose-GDFuncInit
		dc.w	GDExpunge-GDFuncInit
		dc.w	ExtFunc+(*-GDFuncInit)

		dc.w	BeginIO+(*-GDFuncInit)
		dc.w	AbortIO+(*-GDFuncInit)

		dc.w	-1


gdCmdTable:
		dc.l	Invalid
		dc.l	ResetCmd
		dc.l	Read
		dc.l	Write
		dc.l	Update
		dc.l	GDClear
		dc.l	StopCmd
		dc.l	Start
		dc.l	Flush

		dc.l	GDReadEvent
		dc.l	GDAskCType
		dc.l	GDSetCType
		dc.l	GDAskTrigger
		dc.l	GDSetTrigger

gdCmdBytes:
		dc.b	CBInvalid
		dc.b	CBResetCmd
		dc.b	CBRead
		dc.b	CBWrite
		dc.b	CBUpdate
		dc.b	GDCBClear
		dc.b	CBStopCmd
		dc.b	CBStart
		dc.b	CBFlush

		dc.b	GDCBReadEvent
		dc.b	GDCBAskCType
		dc.b	GDCBSetCType
		dc.b	GDCBAskTrigger
		dc.b	GDCBSetTrigger

GD_NUMCOMMANDS	EQU	(*-gdCmdBytes)

		ds.w	0


*------ gameport.device/Open *****************************************
*
*   NAME
*	Open - a request to open the GamePort device
*
*   SYNOPSIS
*	OpenDevice("gameport.device", unit, iORequest, 0)
*
*   FUNCTION
*	The open routine grants access to a device.  There are two
*	fields in the iORequest block that will be filled in: the
*	IO_DEVICE field and the IO_UNIT field.
*
*	The device open count will be incremented.  The device cannot
*	be expunged unless this open is matched by a Close device.
*
*   INPUTS
*	unit  -
*		0   unit associated with left game port controller
*		1   unit associated with right game port controller
*
*   RESULTS
*	If the open was unsuccessful, IO_ERROR will be set, IO_UNIT
*	and IO_DEVICE will not be valid.
*
**********************************************************************
GDOpen:
		move.l	a2,-(a7)
		bclr	#LIBB_DELEXP,LIB_FLAGS(a6)

		;-- this device has two units, determine which one
		tst.l	d0
		bne.s	oTry1
		lea	gd_Unit0(a6),a2
		bra.s	oGotUnit
oTry1:
		subq.l	#1,d0
		bne.s	oOpenFail
		lea	gd_Unit1(a6),a2

oGotUnit:
		tst.w	gu_OpenCnt(a2)
		bne.s	oNoteOpen
	
		;-- clear controller variables for first open
		lea	gu_AccumTimeout(a2),a0
		moveq	#gt_SIZEOF/2-1,d0
oInitTLoop:
		clr.w	(a0)+
		dbf	d0,oInitTLoop
		clr.b	gu_Type(a2)	; show no controller allocated

oNoteOpen:
		addq.w	#1,LIB_OPENCNT(a6)
		addq.w	#1,gu_OpenCnt(a2)
		move.l	a2,IO_UNIT(a1)
		moveq	#0,d0

oRts:
		move.b	d0,IO_ERROR(a1)
		move.l	(a7)+,a2
		rts

oOpenFail:
		moveq   #IOERR_OPENFAIL,d0
		bra.s	oRts

*------ Expunge ------------------------------------------------------
*
*   NAME
*	Expunge - indicate a desire to remove this device
*
*   SYNOPSIS
*	segment = Expunge(), gameportDev
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
GDExpunge:
	    ;-- see if any one is using the device
		tst.w	LIB_OPENCNT(a6)
		bne	DeferredExpunge

*	    ;-- this is really it.  Free up all the resources

*		;-- remove the interrupt service routine
		lea	gd_IS(a6),a1
		moveq	#INTB_VERTB,d0
		LINKEXE	RemIntServer
		bra	SuccessfulExpunge


*------ Close --------------------------------------------------------
GDClose:
		move.l	IO_UNIT(a1),a0
		subq.w	#1,gu_OpenCnt(a0)
		bra	Close


	END
