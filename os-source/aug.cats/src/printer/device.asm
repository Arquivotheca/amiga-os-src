	TTL    '$Id: device.asm,v 1.13 92/01/28 14:29:08 davidj Exp $'
**********************************************************************
*								     *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.	     *
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030				     *
*								     *
**********************************************************************
*
*	printer device functions
*
*   Source Control
*   --------------
*   $Id: device.asm,v 1.13 92/01/28 14:29:08 davidj Exp $
*
*   $Locker:  $
*
*   $Log:	device.asm,v $
*   Revision 1.13  92/01/28  14:29:08  davidj
*   compiled native
*   
*   Revision 1.12  91/07/16  15:09:38  darren
*   Code support for an overriding device name (e.g., spool.device
*   in the future if needed).
*
*   Revision 1.11  91/07/15  17:04:14  darren
*   First V38 release.  Calls code to localize requester at OpenDevice
*   time, and close locale.library and catalog at CloseDevice time so
*   the most current user preferences are picked up each time the
*   printer.device is opened (and its dont when we know we are on a process!)
*   Also verify we are running under V36 or greater, and gets device unit
*   to open from preferences.
*
*   Revision 1.10  91/02/14  15:25:51  darren
*   Autodoc stuff
*
*   Revision 1.9  90/08/30  15:16:21  darren
*   Work around for a $1F mem read.
*   If _IOR was NULL, the call to PBothReady
*   would read low mem.  By good luck, it found
*   a non-zero result, and returned.  A NULL
*   ptr is now checked for within this module.
*
*   Revision 1.8  90/07/27  02:16:05  bryce
*   The $Header line is a real pain.  Converted to $Id
*
*   Revision 1.7  90/04/06  19:23:22  daveb
*   for rcs 4.x header change
*
*   Revision 1.6  89/09/07  12:08:03  daveb
*   'Open' now uses 'netprint.device' if the preferences
*   PrinterPort setting is 2 (NET_PRINTER).
*   added label 'NPName' and text string 'netprint.device'.
*
*   Revision 1.5  88/08/18  15:51:24  daveb
*   fixed PRD_QUERY autodoc (was CMD_QUERY by mistake)
*
*   Revision 1.4  88/08/17  10:29:26  daveb
*   made changes to autodocs
*
*   Revision 1.3  88/05/16  15:07:42  daveb
*   fixed autodoc for Query command (parallel bits were incorrect)
*   V1.3 Gamma 15
*
*   Revision 1.2  87/10/01  09:36:59  daveb
*   added PRD_QUERY command
*
*   Revision 1.1  87/09/09  06:55:35  daveb
*   V1.3 beta 3 release
*
*   Revision 1.0  87/08/21  17:25:37  daveb
*   added to rcs
*
*   Revision 1.0  87/07/29  09:40:04  daveb
*   added to rcs
*
*   Revision 33.2  86/06/11  15:53:43  andy
*
*   Revision 33.1  86/02/13  12:25:31  kodiak
*   first cut fix at detecting changes in custom file name at open time
*
*   Revision 33.0  86/02/13  09:21:24  kodiak
*   added to rcs for updating
*
*   Revision 1.5  85/11/23  11:36:09  kodiak
*   Changes unknown,  suspect change to close or expunge or??
*   This is being checked in by Stan since Kodiak obviously forgot
*
*   Revision 1.4  85/10/18  15:36:31  neil
*   initialization routine changed to return nonzero for success
*
*   Revision 1.3  85/10/10  00:30:05  kodiak
*   remove +du_CmdQueues from pd_Unit reference
*
*   Revision 1.2  85/10/09  23:59:36  kodiak
*   replace reference to pdata w/ prtbase
*
*   Revision 1.1  85/09/25  16:47:47  kodiak
*   add storage for IOR variable
*
*   Revision 1.0  85/09/25  15:40:35  kodiak
*   added to rcs for updating in version 1
*
*   Revision 1.3  85/09/07  20:51:44  kodiak
*   give RemTask A1, not A0
*
*   Revision 1.2  85/09/03  22:19:01  kodiak
*   make _OpenTask external (oops)
*
*   Revision 1.1  85/09/03  18:48:21  kodiak
*   add OpenTask variable
*
*
**********************************************************************

	SECTION		printer

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
	INCLUDE		"exec/alerts.i"

	INCLUDE		"devices/timer.i"

	INCLUDE		"macros.i"
	INCLUDE		"prtbase.i"


*------ Imported Globals ---------------------------------------------

	XREF		PName
	XREF		VERSION
	XREF		REVISION


*------ Imported Functions -------------------------------------------

	XREF_EXE	AddDevice
	XREF_EXE	AddTask
	XREF_EXE	AllocSignal
	XREF_EXE	CloseDevice
	XREF_EXE	CloseLibrary
	XREF_EXE	Forbid
	XREF_EXE	FindTask
	XREF_EXE	FreeMem
	XREF_EXE	FreeSignal
	XREF_EXE	GetMsg
	XREF_EXE	MakeLibrary
	XREF_EXE	OpenDevice
	XREF_EXE	OpenLibrary
	XREF_EXE	Permit
	XREF_EXE	RemTask
	XREF_EXE	Wait
	XREF_EXE	Alert

	XREF		BeginIO
	XREF		AbortIO

	XREF		PCInvalid
	XREF		PCRead
	XREF		PCWrite
	XREF		PCUpdate
	XREF		PCClear
	XREF		PCStop
	XREF		PCStart
	XREF		PCFlush

	XREF		PCRawWrite
	XREF		PCPrtCommand
	XREF		PCDumpRPort

	XREF		PCBInvalid
	XREF		PCBRead
	XREF		PCBWrite
	XREF		PCBUpdate
	XREF		PCBClear
	XREF		PCBStop
	XREF		PCBStart
	XREF		PCBFlush

	XREF		PCBRawWrite
	XREF		PCBPrtCommand
	XREF		PCBDumpRPort

	XREF		_OpenPrinter
	XREF		_ExpungePrinter
	XREF		_PCQuery

	XREF		_PWrite
	XREF		_PBothReady
	XREF		PWTaskStart

;	XREF		_Localize	;requester.asm
;	XREF		_CloseLocale

	XREF		_OpenCat
	XREF		_CloseCat

*------ Exported Globals ---------------------------------------------

	XDEF		PInit
	XDEF		_PD
	XDEF		_PED
	XDEF		_IOR
	XDEF		_SysBase
	XDEF		_DOSBase
	XDEF		_GfxBase
	XDEF		_IntuitionBase
	XDEF		_OpenTask
	XDEF		_LoadedPrinterFile

**********************************************************************
	SECTION		printer,DATA
_PD				DC.L	0	; Printer Data
_PED			DC.L	0	; Printer Extended Data
_IOR			DC.L	0	; IO Request
_SysBase		DC.L	0	; SysBase
_DOSBase		DC.L	0	; DosBase
_GfxBase		DC.L	0	; GfxBase
_IntuitionBase	DC.L	0	; IntuitionBase
_OpenTask		DC.L	0	; ?
_LoadedPrinterFile	DS.B	FILENAME_SIZE	; name of loaded printer file
MakeDeviceName:		DS.B	DEVNAME_SIZE+10

P_ALERT		SET		1

**********************************************************************
	SECTION		printer,CODE
*
*	This is the initialization code for the printer device that gets
*	called right after the code is loaded
*
PInit:
		MOVEM.L	A0/A2,-(A7)
		LEA	pdFuncInit(PC),A0
		LEA	pdStructInit(PC),A1
		LEA	pdInit(PC),A2
		MOVE.L	#pd_SIZEOF,D0
		CALLEXE	MakeLibrary
		MOVEM.L	(A7)+,A0/A2

	IFNE	P_ALERT
		TST.L	D0
		BNE.S	PInit10
		ALERT	AG_MakeLib
		BRA.S	pdRts
	ENDC

		TST.L	D0
		BEQ.S	pdRts

PInit10:
		MOVE.L	D0,A1
		MOVE.L	A1,_PD
		MOVE.L	A0,dd_Segment(A1)
		CALLEXE	AddDevice

		MOVEQ.L	#1,D0

pdRts:
		RTS

pdInit:
		MOVE.L A6,-(A7)
*	    ;------ This is called from within MakeLibrary, after
*	    ;------ all the memory has been allocated

		MOVE.L	D0,A0
		MOVE.L	A6,_SysBase
		MOVE.L	D0,A6			; library pointer

*	    ;------ open the dos library
		LEA	DLName(PC),A1
		MOVEQ	#00,D0
		LINKEXE OpenLibrary
		MOVE.L	D0,_DOSBase
		BEQ	initDLErr

*	    ;------ open the graphics library
		LEA	GLName(PC),A1
		MOVEQ	#00,D0
		LINKEXE OpenLibrary
		MOVE.L	D0,_GfxBase
		BEQ	initGLErr

*	    ;------ open the intuition library
		LEA	ILName(PC),A1
		MOVEQ	#00,D0
		LINKEXE OpenLibrary
		MOVE.L	D0,_IntuitionBase
		BEQ	initILErr

*	    ;------ open the timer device
		LEA	TDName(PC),A0
		MOVEQ	#UNIT_VBLANK,D0
		MOVEQ	#0,D1
		LEA	pd_TIOR(A6),A1
		LINKEXE OpenDevice
		TST.L	D0
		BNE.S	initTDErr

		LEA	pd_Stk(A6),A0
		MOVE.L	A0,pd_TC+TC_SPLOWER(A6)
		MOVE.W	#(P_STKSIZE/2)-1,D0
		MOVE.W	#$0AFDB,D1
pattern0Loop:
		MOVE.W	D1,(A0)+
		DBF	D0,pattern0Loop

		SUB.L	A1,A1
		LINKEXE	FindTask
		LEA	pd_Stk+P_STKSIZE(A6),A0
		LEA	pd_TC(A6),A1
		MOVE.L	A0,TC_SPUPPER(A1)
		MOVEM.L	D0/A6,-(A0)		; args: (task, printerData)
		MOVE.L	A0,TC_SPREG(A1)

		LEA	PWTaskStart(PC),A2
		MOVEQ	#0,D0
		MOVE.L	D0,A3
		LINKEXE AddTask

	;------ wait for printer task to fire up
		MOVEQ	#SIGF_CHILD,D0
		LINKEXE	Wait

	;------ clear the printer file name
		CLR.B	_LoadedPrinterFile

		MOVE.L	A6,D0

pdiRts:
		MOVE.L	(A7)+,A6
		RTS

initTDErr:
	IFNE	P_ALERT
		ALERT	AG_OpenLib!AO_TimerDev
	ENDC
		MOVE.L	_IntuitionBase,A1
		LINKEXE	CloseLibrary

initILErr:
	IFNE	P_ALERT
		ALERT	AG_OpenLib!AO_Intuition
	ENDC
		MOVE.L	_GfxBase,A1
		LINKEXE	CloseLibrary

initGLErr:
	IFNE	P_ALERT
		ALERT	AG_OpenLib!AO_GraphicsLib
	ENDC
		MOVE.L	_DOSBase,A1
		LINKEXE	CloseLibrary

initDLErr:
	IFNE	P_ALERT
		ALERT	AG_OpenLib!AO_DOSLib
	ENDC
		MOVE.L	A6,A1
		MOVE.W	LIB_NEGSIZE(A6),D0
		SUB.W	D0,A1
		ADD.W	LIB_POSSIZE(A6),D0
		LINKEXE FreeMem
		MOVEQ	#0,D0
		BRA		pdiRts

TDName:
		DC.B	'timer.device'
		DC.B	0
ILName:
		DC.B	'intuition.library'
		DC.B	0
DLName:
		DC.B	'dos.library'
		DC.B	0
GLName:
		DC.B	'graphics.library'
		DC.B	0

DVExtension:	DC.B	'.device'
		DC.B	0

		DS.W	0


*----------------------------------------------------------------------
pdStructInit:
*	;------ initialize the device library structure
		INITBYTE	LN_TYPE,NT_DEVICE
		INITLONG	LN_NAME,PName
		INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD	LIB_VERSION,VERSION
		INITWORD	LIB_REVISION,REVISION

		INITLONG	dd_CmdVectors,pdCmdTable
		INITLONG	dd_CmdBytes,pdCmdBytes
		INITWORD	dd_NumCommands,PC_NUMCOMMANDS

*	;------ initialize the unit command queue
		INITBYTE	pd_Unit+LN_TYPE,NT_MSGPORT
		INITLONG	pd_Unit+LN_NAME,PName

		INITBYTE	pd_IORPort+LN_TYPE,NT_MSGPORT
		INITLONG	pd_IORPort+LN_NAME,PName

*	;------ Initialize the tasks
		INITBYTE	pd_TC+LN_TYPE,NT_TASK
		INITBYTE	pd_TC+LN_PRI,P_PRIORITY
		INITLONG	pd_TC+LN_NAME,PName

	;------
		INITLONG	pd_PWrite,_PWrite
		INITLONG	pd_PBothReady,_PBothReady
		INITWORD	pd_PrinterType,-1
		INITBYTE	pd_Flags1,PDF_24BIT

		DC.W	0

pdFuncInit:
		DC.W	-1

		DC.W	Open-pdFuncInit
		DC.W	Close-pdFuncInit
		DC.W	Expunge-pdFuncInit
		DC.W	ExtFunc-pdFuncInit

		DC.W	BeginIO+(*-pdFuncInit)
		DC.W	AbortIO+(*-pdFuncInit)

		DC.W	-1

pdCmdTable:
		DC.L	PCInvalid
		DC.L	PCReset
		DC.L	PCRead
		DC.L	PCWrite
		DC.L	PCUpdate
		DC.L	PCClear
		DC.L	PCStop
		DC.L	PCStart
		DC.L	PCFlush

		DC.L	PCRawWrite
		DC.L	PCPrtCommand
		DC.L	PCDumpRPort

		DC.L	PCQuery

pdCmdBytes:
		DC.B	PCBInvalid
		DC.B	PCBReset
		DC.B	PCBRead
		DC.B	PCBWrite
		DC.B	PCBUpdate
		DC.B	PCBClear
		DC.B	PCBStop
		DC.B	PCBStart
		DC.B	PCBFlush

		DC.B	PCBRawWrite
		DC.B	PCBPrtCommand
		DC.B	PCBDumpRPort

		DC.B	PCBQuery

PC_NUMCOMMANDS	EQU	(*-pdCmdBytes)

		DS.W	0

******* printer.device/PRD_QUERY ****************************************
*
*   NAME
*	PRD_QUERY -- query printer port/line status
*
*   FUNCTION
*	This command returns the status of the printer port's lines and
*	registers.  Since the printer port uses either the serial or
*	parallel port for i/o, the actual status returned is either the
*	serial or parallel port's status.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Command	PRD_QUERY
*	io_Data		ptr to 2 UBYTES where result will be stored.
*
*   RESULTS
*     io_Data          BIT  ACTIVE  FUNCTION (SERIAL DEVICE)
*
*             LSB       0    low    reserved
*                       1    low    reserved
*                       2    low    reserved
*                       3    low    Data Set Ready
*                       4    low    Clear To Send
*                       5    low    Carrier Detect
*                       6    low    Ready To Send
*                       7    low    Data Terminal Ready
*             MSB       8    high   read buffer overflow
*                       9    high   break sent (most recent output)
*                      10    high   break received (as latest input)
*                      11    high   transmit x-OFFed
*                      12    high   receive x-OFFed
*                   13-15           reserved
*
*
*     io_Data          BIT  ACTIVE  FUNCTION (PARALLEL DEVICE)
*
*                       0     hi     printer busy (offline)
*                       1     hi     paper out
*                       2     hi     printer selected
*				     (WARNING: the bit 2 line is also connected
*				      to the serial port's ring indicator pin
*				      on the A500 and A2000)
*                     3-7            reserved
*
*     io_Actual       1-parallel, 2-serial
*
**********************************************************************
PCBQuery	EQU	-1

PCQuery:
	MOVE.L	A1,-(A7)	; pass IORequest as parm
	JSR	_PCQuery		; query parallel or serial device
	MOVE.L	(A7)+,A1	; fix stack
	RTS


******* printer.device/CMD_RESET ****************************************
*
*   NAME
*	CMD_RESET -- reset the printer
*
*   FUNCTION
*	CMD_RESET resets the printer device without destroying handles
*	to the open device.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Command	CMD_RESET
*	io_Flags	IOB_QUICK set if quick I/O is possible
*
**********************************************************************
PCBReset	EQU	-1

PCReset:
		BSR	PCStop
		BSR	PCFlush
		BSR	PCStart
		RTS


*------ printer.device/Expunge ---------------------------------------
*
*   NAME
*	Expunge - indicate a desire to remove the printer device
*
*   SYNOPSIS
*	segment = Expunge(), printerDev
*			     A6
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
Expunge:
	    ;-- see if any one is using the device
		TST.W	LIB_OPENCNT(A6)
		BNE	deferExpunge

	    ;-- this is really it.  Free up all the resources
		MOVE.L	A6,-(A7)
		JSR	_ExpungePrinter
		ADDQ.L	#4,A7

		LEA	pd_TC(A6),A1
		LINKEXE	RemTask

		LEA	pd_TIOR(A6),A1
		LINKEXE	CloseDevice

		MOVE.L	_IntuitionBase,A1
		LINKEXE	CloseLibrary

		MOVE.L	_GfxBase,A1
		LINKEXE	CloseLibrary

		MOVE.L	_DOSBase,A1
		LINKEXE	CloseLibrary


	    ;------ deallocate entity data
		MOVE.L	dd_Segment(A6),-(A7)
		MOVE.L	A6,A1
		MOVE.W	LIB_NEGSIZE(A6),D0
		SUB.W	D0,A1
		ADD.W	LIB_POSSIZE(A6),D0
		LINKEXE FreeMem

		MOVE.L	A6,A1
		REMOVE

		MOVE.L	(A7)+,D0
		RTS

deferExpunge:
		BSET	#PB_EXPUNGED,pd_Flags(A6)
		MOVEQ	#0,D0		;still in use

ExtFunc:
		RTS


*------ printer.device/Open ------------------------------------------
*
*   NAME
*	Open - a request to open the printer device
*
*   SYNOPSIS
*	Open(iORequest), printerDev
*			A1			A6
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
Open:
		MOVE.L	A3,-(A7)
		MOVE.L	A1,A3

	    ;-- only one accessor at a time!
		LINKEXE	Forbid
		ADDQ.W	#1,LIB_OPENCNT(A6)
		CMPI.W	#1,LIB_OPENCNT(A6)
		BNE	permitFail

	    ;-- Make sure we are running under V36 or greater

		MOVE.L	_IntuitionBase,A0
		CMPI.W	#36,LIB_VERSION(A0)
		BCS	permitFail

		LINKEXE	Permit

		SUB.L	A1,A1
		LINKEXE	FindTask
		MOVE.L	D0,_OpenTask

*	    ;-- this device has only one unit

		LEA	pd_Unit(A6),A0
		MOVE.L	A0,IO_UNIT(A3)

	    ;-- look at preferences for the correct printer dependent part
		MOVE.L	A6,-(A7)
		JSR	_OpenPrinter
		ADDQ.L	#4,A7
		TST.L	D0
		BNE	cOpenFail

;	    ;------ Localize text strings
;
;		BSR	_Localize
		BSR	_OpenCat

	    ;------ open the appropriate port device
		CMP.B	#PARALLEL_PRINTER,pd_Preferences+pf_PrinterPort(A6)
		BEQ.S	paName		; parallel device
		CMP.B	#SERIAL_PRINTER,pd_Preferences+pf_PrinterPort(A6)
		BEQ.S	seName		; serial device

; David Berezowski - Sept. 5/89
; - added kludge code (as per Andy Finkel's request) to select
;   'netprint.device' if the printer preferences device selector
;   is 2 (NET_PRINTER).  Previous to this the value was either
;   0 (PARALLEL_PRINTER) or 1 (SERIAL_PRINTER).  This kludge was
;   added to let the Novell Network people test their fake parallel
;   port driver called 'netprint'.

NET_PRINTER	SET	2

		CMP.B	#NET_PRINTER,pd_Preferences+pf_PrinterPort(A6)
		BNE	openFail	; non-specified, error!

npName:					; netprint device
		LEA     NPName(PC),A0
		BRA.S	gotName

seName:
		LEA	SEName(PC),A0
		BRA.S	gotName

paName:
		LEA	PAName(PC),A0
gotName:

	;-- See if the device name was specified - assumed to be
	;-- entirely correct (not too long, null terminated, does not
	;-- include .device extension, etc.).  Also don't bother
	;-- unless the PrinterPort type is specifically serial, or
	;-- parallel (and we assume that the named device is infact
	;-- a parallel, or serial clone!)


		CMP.B	#SERIAL_PRINTER,pd_Preferences+pf_PrinterPort(a6)
		BHI.S	useName

		LEA	pd_Preferences+pf_PrtDevName(a6),a1
		TST.B	(a1)
		BEQ.S	useName

		LEA	MakeDeviceName,a0
		MOVE.L	A0,D0
copyName:
		MOVE.B	(a1)+,(a0)+		;copy name
		BNE.S	copyName

		LEA	DVExtension,a1
		SUBQ.L	#1,a0			;skip back to NULL
copyExtension:
		MOVE.B	(a1)+,(a0)+		;copy .device extension
		BNE.S	copyExtension

		MOVEA.L	D0,A0			;restore pointer

useName:
		MOVEQ	#0,D0
		MOVE.B	pd_Preferences+pf_DefaultPrtUnit(a6),D0

		MOVEQ	#0,D1
		LEA	pd_IOR0(A6),A1
		LINKEXE OpenDevice
		TST.L	D0
		BNE.S	openFail

		MOVEQ	#((pd_IOR1-pd_IOR0)/2)-1,D0
		LEA	pd_IOR0(A6),A0
		LEA	pd_IOR1(A6),A1
copyIOR:
		MOVE.W	(A0)+,(A1)+
		DBF	D0,copyIOR

		MOVE.L	A3,-(A7)
		MOVE.L	_PED,A0
		MOVE.L	ped_Open(A0),A0
		JSR	(A0)
		ADDQ.L	#4,A7
		TST.L	D0
		BNE.S	openError
		BCLR	#PB_EXPUNGED,pd_Flags(A6)

openRts:
		MOVE.L	(A7)+,A3
		RTS

openError:
		LEA	pd_IOR0(A6),A1
		LINKEXE	CloseDevice
		BRA.S	openFail

permitFail:
		LINKEXE	Permit
openFail:
		MOVEQ	#IOERR_OPENFAIL,D0
cOpenFail:
		MOVE.B	D0,IO_ERROR(A3)
		SUBQ.W	#1,LIB_OPENCNT(A6)
		BRA.S	openRts

; David Berezowski - Sept. 5/89
; - added kludge code (as per Andy Finkel's request) to select
;   'netprint.device' if the printer preferences device selector
;   is 2.  Previous to this the value was either 0 or 1.  This
;   kludge was added to let the Novell Network people test their
;   fake parallel port driver called 'netprint'.

NPName:		DC.B	'netprint.device'
		DC.B	0

PAName:
		DC.B	'parallel.device'
		DC.B	0
SEName:
		DC.B	'serial.device'
		DC.B	0
		DS.W	0


*------ printer.device/Close -----------------------------------------
*
*   NAME
*	Close - terminate access to a device
*
*   SYNOPSIS
*	Close(iORequest), printerDev
*			A1			A6
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
Close:
		MOVE.L	A3,-(A7)
		MOVE.L	A1,A3

	;------ ensure IO is done & set up for this task
    ;------ clear the message port of any messages sent to printer task
		LINKEXE	Forbid
checkMsgs:
		LEA	pd_IORPort(A6),A0
		LINKEXE	GetMsg
		TST.L	D0
		BEQ.S	noMsgs

		LEA	pd_IOR0(A6),A1
		CMP.L	A1,D0
		BNE.S	checkIOR1
		BCLR	#PB_IOR0,pd_Flags(A6)
		BRA.S	checkMsgs

checkIOR1:
		LEA	pd_IOR1(A6),A1
		CMP.L	A1,D0
		BNE.S	checkMsgs
		BCLR	#PB_IOR1,pd_Flags(A6)
		BRA.S	checkMsgs

noMsgs:
	    ;------ set reply port up for this task
		MOVE.B	pd_IORPort+MP_SIGBIT(A6),-(A7)
		MOVE.L	pd_IORPort+MP_SIGTASK(A6),-(A7)
		MOVEQ	#-1,D0
		LINKEXE	AllocSignal
		MOVE.B	D0,pd_IORPort+MP_SIGBIT(A6)
		BMI.S	asigFail
		SUB.L	A1,A1
		LINKEXE	FindTask
		MOVE.L	D0,pd_IORPort+MP_SIGTASK(A6)

	    ;------ ensure no output is pending
		TST.L	_IOR			;dont bother if IOR is NULL
		BEQ	specificClose

		BSR	_PBothReady
		BRA.S	specificClose

asigFail:
		CLR.L	pd_IORPort+MP_SIGTASK(A6)

specificClose:
		LINKEXE	Permit

	;------ close locale.library, and catalog

;		BSR	_CloseLocale
		BSR	_CloseCat

	;------ call printer specific close function
		MOVE.L	A3,-(A7)
		MOVE.L	_PED,A0
		MOVE.L	ped_Close(A0),A0
		JSR	(A0)
		ADDQ.L	#4,A7

		LEA	pd_IOR0(A6),A1
		LINKEXE	CloseDevice

	;------ free signal allocated for this task
		MOVEQ	#0,D0
		MOVE.B	pd_IORPort+MP_SIGBIT(A6),D0
		BMI.S	restoreIORPort
		LINKEXE	FreeSignal

	    ;------ restore IORPort to point to printer.device task
restoreIORPort:
		LINKEXE	Forbid
		MOVE.L	(A7)+,pd_IORPort+MP_SIGTASK(A6)
		MOVE.B	(A7)+,pd_IORPort+MP_SIGBIT(A6)
		LINKEXE	Permit

		SUBQ.W	#1,LIB_OPENCNT(A6)

	    ;-- clear out the pointers
		MOVEQ	#0,D0
		MOVE.L	D0,IO_UNIT(A3)
		MOVE.L	D0,IO_DEVICE(A3)

*	    ;-- check if this device should now be expunged
		BNE.S	closeClear
		BTST	#PB_EXPUNGED,pd_Flags(A6)
		BEQ.S	closeClear
		MOVE.L	A3,A1
		JSR	LIB_EXPUNGE(A6)

closeClear:
		MOVE.L	(A7)+,A3
		RTS

		END
