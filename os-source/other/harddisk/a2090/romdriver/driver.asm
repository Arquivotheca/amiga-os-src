*******************************************************************************
*
*	Source Control
*	--------------
*	$Header: driver.asm,v 34.13 88/02/16 11:29:08 bart Exp $
*
*	$Locker:  $
*
*	$Log:	driver.asm,v $
*   Revision 34.13  88/02/16  11:29:08  bart
*   pass correct controller number and name pointer-pointer from Open() to
*   InitUnit...
*   
*   Revision 34.12  88/01/21  18:04:47  bart
*   compatible with disk based / binddriver useage
*   
*   Revision 34.11  87/12/04  19:13:45  bart
*   checkpoint
*   
*   Revision 34.10  87/12/04  12:08:18  bart
*   checkpoint before adding check for existing dosname on eb_mountlist
*   
*   Revision 34.9  87/10/27  13:03:36  bart
*   pass harderr code back intact
*   
*   Revision 34.8  87/10/26  16:31:25  bart
*   checkpoint
*   
*   Revision 34.7  87/10/14  14:47:32  bart
*   10-13 rev 1
*   
*   Revision 34.6  87/10/14  14:15:49  bart
*   beginning update to cbm-source.10.13.87
*   
*   Revision 34.5  87/07/08  14:00:53  bart
*   y
*   
*   Revision 34.4  87/06/11  15:47:52  bart
*   working autoboot 06.11.87 bart
*   
*   Revision 34.3  87/06/03  10:58:54  bart
*   checkpoint
*   
*   Revision 34.2  87/05/31  16:35:38  bart
*   chickpoint
*   
*   Revision 34.1  87/05/29  19:39:15  bart
*   checkpoint
*   
*   Revision 34.0  87/05/29  17:39:30  bart
*   added to rcs for updating
*   
*
*******************************************************************************

	LLEN	130
	PLEN	60
	LIST

*************************************************************************
*									*
*	Copyright (C) 1986, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


	SECTION section

******* Included Files ***********************************************

	NOLIST
	IFND	EXEC_TYPES_I
	INCLUDE 'exec/types.i'
	ENDC
	IFND	EXEC_INTERRUPTS_I
	INCLUDE	"exec/interrupts.i"
	ENDC
	IFND	EXEC_LISTS_I
	INCLUDE 'exec/lists.i'
	ENDC
	IFND	EXEC_NODES_I
	INCLUDE 'exec/nodes.i'
	ENDC
	IFND	EXEC_PORTS_I
	INCLUDE 'exec/ports.i'
	ENDC
	IFND	EXEC_LIBRARIES_I
	INCLUDE 'exec/libraries.i'
	ENDC
	IFND	EXEC_IO_I
	INCLUDE 'exec/io.i'
	ENDC
	IFND	EXEC_DEVICES_I
	INCLUDE 'exec/devices.i'
	ENDC
	IFND	EXEC_TASKS_I
	INCLUDE 'exec/tasks.i'
	ENDC
	IFND	EXEC_MEMORY_I
	INCLUDE 'exec/memory.i'
	ENDC
	IFND	EXEC_EXECBASE_I
	INCLUDE 'exec/execbase.i'
	ENDC
	IFND	EXEC_ABLES_I
	INCLUDE 'exec/ables.i'
	ENDC
	IFND	EXEC_ERRORS_I
	INCLUDE 'exec/errors.i'
	ENDC
	IFND	EXEC_ALERTS_I
	INCLUDE 'exec/alerts.i'
	ENDC
	INCLUDE 'hddisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE	'libraries/expansion.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'
	LIST

******* Imported Functions *******************************************

	XREF	ExLibName
	XREF	InitUnit
	XREF	cmdTable
	XREF	hdName

	EXTERN_LIB Alert
	EXTERN_LIB CloseLibrary
	EXTERN_LIB Debug
	EXTERN_LIB OpenLibrary
	EXTERN_LIB PutMsg
	EXTERN_LIB ReplyMsg
	EXTERN_LIB Signal
	EXTERN_LIB Wait

	INT_ABLES
	TASK_ABLES

******* Exported Functions *******************************************

	XDEF	TermIO
	XDEF	Soft_Error

***** Local Definitions **********************************************

	XDEF	Expunge
	XDEF	Open
	XDEF	Close
	XDEF	BeginIO
	XDEF	PerformIO
	XDEF	Null
	XDEF	NoIO

***** Let the Code Begin *********************************************

******* Device/HDDisk/Expunge *************************************
*
*   NAME
*	Expunge -- clean up after a device
*
*   SYNOPSIS
*	Result = Expunge( ), DevNode
*			     A6
*
*   FUNCTION
*	The Expunge routine is called when a user performs a RemDevice
*	call.  By the time it is called, the device has already been
*	removed from the device naming list, so no new opens will
*	succeed.  At this point the device has three main options:
*
*	    1.	It may put itself back onto the naming list via
*		an AddDevice, which means that the "RemDevice" has
*		no effect.
*
*	    2.	If no one has the device open, it may clean up
*		after itself and free all its memory.
*
*	    3.	If someone has the device open, it may either wait
*		until the device is closed and then free all its
*		resources, or free most of its resources now, dummy
*		out all of its entrypoints, and use the default
*		"DefaultClose" routine to free up the last dregs
*		after the last close.
*
*
*   INPUTS
*
*   RESULTS
*	Result -- The results field is not defined.  Whatever the
*	    Expunge routine returns will be returned to the user.
*	    It is recommened that Result be left undefined.
*
*
*   SEE ALSO
*
*
**********************************************************************
*
*
*   REGISTER USAGE
*
*
*   IMPLEMENTATION
*
* !!! 6 Feb 85 -- Memory is no longer linked into the task.  Be sure
*	to FreeEntry memory on expunge.
*
*

Expunge:
		INFOMSG	50,<'%s/Expunge: called'>

		DISABLE	A0
		TST.W	LIB_OPENCNT(A6)
		BNE	Exp_End


Exp_Really:



*		;!!! do something...


Exp_End:

		ENABLE	A0
		CLEAR	D0
		RTS




******* Device/HDDisk/Open ****************************************
*
*   NAME
*	Open -- a request to open the device
*
*   SYNOPSIS
*	Open( iORequest, UnitNumber, Flags )
*	      A1	 D0	     D1	    
*
*   FUNCTION
*	The open routine grants access to a device.  There are two
*	fields in the iORequest block that need to be filled in.
*	The IO_DEVICE field has already been initialized by OpenDevice.
*	The Open routine needs to initialize the IO_UNIT field as
*	appropriate.  If the open was unsuccessful, the error field
*	should be assigned an appropriate value.
*
*	It is strongly recommended that the LIB_OPENCNT field be
*	updated to reflect the current number of opens on the library.
*	This is necessary so one may know when it is safe to remove
*	the device from the system.
*
*	In addition, the device is free to keep addition open counts
*	on a unit by unit basis
*
*   INPUTS
*	iORequest -- an I/O Request Block that the Open routine
*	    should initialize.
*
*	UnitNumber -- a unit identifier that is device specific
*
*	Flags -- not defined at this time
*
*   RESULTS
*
*
*   SEE ALSO
*
*
**********************************************************************
*
*
*   REGISTER USAGE
*
*
*   IMPLEMENTATION
*
*


Open:
		MOVEM.L	D2-D5/D7/A2-A6,-(SP)

*		set up D7 : point to the exec device name pointer for InitUnit!

		LEA.L	LN_NAME(A6),A3
		MOVE.L	A3,D7

		MOVE.L	A6,A3		; Preserve device pointer
		MOVE.L	#0,A4		; Indicate Expansion library not open
		MOVE.L	A1,A5
		TST.L	D0		; Unit "0" not allowed
		BEQ.S	Open_BadUnit
		MOVE.B	IO_FLAGS(A1),D4	; Get FLAGS from IORequest
		SUBQ	#1,D0		; Change unit 1 and 2 to 0 and 1

		DIVU	#10,D0		; Figure out which controller card

		MOVEQ.L	#0,D3
		MOVE.W	D0,D3		; D3 index to controller
		LSL.L	#2,D3
		SWAP	D0
		
		TST.L	DV_0(A6,D3.L)	; If DV pointer is ZERO,
		BEQ.S	Open_BadUnit	;   controller for unit doesn't exist

*		;------ Initialize the device pointer
		MOVE.L	DV_0(A6,D3.L),A6

		MOVEQ.L	#0,D2
		MOVE.W	D0,D2
		CMP.B	#9,D0
		BLE.S	Open_OKUnit

*		;------ Not unit > 9, so must be an error
Open_BadUnit:	MOVEQ	#HDERR_BadUnitNum,D0
		BRA	Open_Err

Open_OKUnit:
*		;------ Initialize the unit ptr
		LSL.L	#2,D0
		LEA.L	HDU0(A6,D0.W),A2

		MOVE.L	(A2),A0
		MOVE.L	A0,D0
		BNE.S	Open_GotUnit
*	A4 must point to the Expansion library for InitUnit
		LEA.L	ExLibName(PC),A1		; Get expansion lib. name
		CLR	D0
		LINKSYS	OpenLibrary		; Open the expansion library
		TST.L	D0
		BNE.S	Open_OpSuccess

		ALERT	(AN_AMHDDiskDev!AG_OpenLib),,A0

Open_OpSuccess:	MOVE.L	D0,A4
		MOVEQ	#0,D5			; Open even if I/O error
		ADDQ.L	#4,D3			; InitUnit compatibility

		BSR	InitUnit
		TST.L	D0
		BNE	Open_Err

*		;------ store the unit pointer
		MOVE.L	A0,(A2)

Open_GotUnit:


		MOVE.L	A0,IO_UNIT(A5)
		ADDQ.W	#1,LIB_OPENCNT(A3)

Open_End:
		
		MOVE.L	A4,D0			; If Expansion lib open,
		BEQ.S	ExNotOpen
		MOVE.L	A4,A1
		LINKSYS CloseLibrary		;	close it

ExNotOpen:	MOVEM.L	(SP)+,D2-D5/D7/A2-A6

		RTS

Open_Err:
		MOVE.B	D0,IO_ERROR(A5)
		MOVEQ	#-1,D0
		MOVE.L	D0,IO_UNIT(A5)
		MOVE.L	D0,IO_DEVICE(A5)
		BRA	Open_End

******* Device/HDDisk/Close ***************************************
*
*   NAME
*	Close -- terminate access to a device
*
*   SYNOPSIS
*	Close( iORequest ), DevNode
*	       A1	    A6
*
*   FUNCTION
*	The close routine notifies a device that it will no longer
*	be using the device.  The driver should clear the IO_DEVICE
*	and IO_UNIT entries in the iORequest structure.	 In addition
*	it should decrement any open counts that it maintains, and
*	unlock the unit if it was opened exclusively.
*
*	The device is free to do anything else at this time that is
*	appropriate.
*
*   INPUTS
*
*
*   RESULTS
*
*
*   SEE ALSO
*
*
**********************************************************************
*
*
*   REGISTER USAGE
*
*
*   IMPLEMENTATION
*
*

Close:
		MOVEM.L A2/A3,-(SP)
		MOVE.L	A1,A2


*		;-- could do unit specific close stuff here, if needed

		INFOMSG	50,<'%s/Close: called'>

		SUBQ.W	#1,LIB_OPENCNT(A6)
		BNE.S	CLOSE_NOT_ZERO
		MOVE.W	#1,LIB_OPENCNT(A6)	; Never let it get to zero
CLOSE_NOT_ZERO:
*		;-- clear out the pointers
		MOVEQ	#-1,D0
		MOVE.L	D0,IO_UNIT(A2)
		MOVE.L	D0,IO_DEVICE(A2)

		MOVEM.L (SP)+,A2/A3

		INFOMSG	50,<'%s/Close: returning'>

		CLEAR	D0

		RTS


******* Device/HDDisk/BeginIO  **************************************
*
*   NAME
*	BeginIO -- start up an io process
*
*   SYNOPSIS
*	BeginIO( iORequest ), DevNode
*	         A1	      A6
*
*   FUNCTION
*	BeginIO has the responsibility of making devices single
*	threaded.  Once this has been done, PerformIO is called.
*
*	There are many different ways of limiting access to a device.
*	There has been a bit saved in the unit flags to ease this --
*	the UNITB_ACTIVE bit can be used to limit access.  However any
*	other method may be used.
*
*	If access to the device cannot be obtained, the request should
*	be queued up to be processed later.
*
*	There may be some I/O Requests that do not need to be single
*	threaded.  For the device is free to perform these immediately.
*
*   INPUTS
*	iORequest -- the I/O Request Block for this request.
*
*
*   RESULTS
*
*
*   SEE ALSO
*
*
**********************************************************************
*
*
*   REGISTER USAGE
*
*
*   IMPLEMENTATION
*
*

* !!! define the commands that may be done NOW
* RESET, STOP, START, FLUSH, CHANGENUM, CHANGESTATE, PROTSTATUS
*****IMMEDIATES	EQU	$e1c2

******* RESET, STOP, START, FLUSH, CHANGENUM, CHANGESTATE
IMMEDIATES	EQU	$61c2

BeginIO:
		MOVE.L	A6,-(SP)	; Preserve A6
		IFGE	INFO_LEVEL-50
		PEA	0
		MOVE.W	IO_COMMAND(A1),2(SP)
		MOVE.L	A1,-(SP)
		INFOMSG 50,<'%s/BeginIO: called with IOR 0x%lx, com %lx'>
		ADDQ.L	#8,SP
		ENDC

		CLR.B	IO_ERROR(A1)

		;------ check for a legal command
		MOVEQ.L	#0,D0
		MOVE.B	IO_COMMAND+1(A1),D0
		CMP.B	#HD_LASTCOMM,D0
		BHI	BeginIO_Err


		MOVE.L	IO_UNIT(A1),A0
		MOVE.L	HDU_CTLR(A0),A6
		LEA.L	HD_MP(A6),A0		; Get address of MsgPort

		AND.B	#(~(IOF_QUICK!IOF_IMMEDIATE))&$ff,IO_FLAGS(A1)
		LINKSYS PutMsg
BeginIO_End:
		INFOMSG 60,<'%s/BeginIO: exiting'>
		MOVE.L	(SP)+,A6	; Preserve A6
		RTS

BeginIO_Err:
		BSR	NoIO
		BRA.S	BeginIO_End


******* Device/HDDisk/PerformIO ***********************************
*
*   NAME
*	PerformIO -- do an actual IO request
*
*   SYNOPSIS
*	PerformIO( iORequest ), UnitPtr, DevPtr
*		   A1		A3       A6
*
*   FUNCTION
*	PerformIO does the actual dispatching of the I/O request.
*	At this point, PerformIO is free to do anything it wants
*	to satisfy the request.
*
*	When it is done processing, the driver should return the
*	I/O Block to whence it came (via ReplyMsg), and process
*	any command that have been queued up while processing
*	this and previous messages.
*
*   INPUTS
*
*
*   RESULTS
*
*
*   SEE ALSO
*
*
**********************************************************************
*
*
*   REGISTER USAGE
*
*
*   IMPLEMENTATION
*
*


PerformIO:
		MOVE.L	A2,-(SP)
		MOVE.L	A1,A2

		IFGE	INFO_LEVEL-80
		PEA	0
		MOVE.W	IO_COMMAND(A1),2(SP)
		INFOMSG 80,<'%s/PerformIO: called (cmd %lx)'>
		ADDQ.L	#4,SP
		ENDC

		MOVE.L	A2,A1
		MOVE.W	IO_COMMAND(A2),D0

Perform_Call:
*		;------ we only look at the low byte of the command
		CLEAR	D1
		MOVE.B	D0,D1
		LSL	#2,D1
		LEA	cmdTable(PC),A0
		JSR	0(A0,D1.W)

Perform_End:
		IFGE	INFO_LEVEL-80
		PEA	0
		MOVE.W	IO_COMMAND(A2),2(SP)
		INFOMSG 80,<'%s/PerformIO: exiting (command 0x%lx)'>
		ADDQ.L	#4,SP
		ENDC

		MOVE.L	(SP)+,A2
		RTS


******* Device/HDDisk/Null ****************************************
*
*   NAME
*	Null -- provide a dummy entry point
*
*   SYNOPSIS
*	Zero = Null(), DevNode
*	D0	       A6
*
*   FUNCTION
*	Be a constant source of zero's for unimplemented routines.
*
*   RESULTS
*	Zero -- Always return 0 in D0
*
*
*   SEE ALSO
*	SLNullFunc
*
*
**********************************************************************


Null:
		MOVEQ	#0,D0
		RTS

NoIO:
		PUTMSG	50,<'%s/NoIO: called'>
		MOVE.B	#IOERR_NOCMD,IO_ERROR(A1)
		BSR	TermIO
		RTS

******* Device/TermIO ************************************************
*
*   NAME
*	TermIO -- mark an I/O Request block as complete
*
*   SYNOPSIS
*	TermIO( iORequest ), devLib
*		A1	     A6
*
*   FUNCTION
*	TermIO does all the necessary cleanup after an I/O
*	Request block has been completed.  It will mark it
*	as done, send it back to the originating task, and
*	wake up the driver.
*
*   INPUTS
*	iORequest -- a pointer to the I/O Request Block
*
**********************************************************************
*
*   NOTE
*	This routine would normally be found in the device support
*	library.  Currently it does not have a permanent place to
*	live, and is part of each driver's address space.  This
*	will be fixed as we get the device support library nailed
*	down.
*




TermIO:
		IFGE	INFO_LEVEL-80
		MOVE.L	A1,-(SP)
		INFOMSG	80,<'%s/TermIO: IOB %lx'>
		ADDQ.L	#4,SP
		ENDC

*		;------ and send it back
		LINKSYS ReplyMsg

*		DISABLE	A0
*		;------ Cache the state of the input message queue

*		MOVE.L	MP_MSGLIST+LH_HEAD(A3),A0

*		;------ See if this was a "driver" generated I/O request
*		BTST	#UNITB_INTASK,UNIT_FLAGS(A3)
*		BNE.S	Term_Wake
*
*		;------ and mark us as "done"
*		BCLR	#UNITB_ACTIVE,UNIT_FLAGS(A3)
*
Term_Wake:

*		ENABLE	A0

*		;------ wake up the task (if we need to)
*		TST.L	(A0)
*		BEQ.S	Term_End

Term_Signal:

*		;------ Signal the task
*		;---!!! Need to parameterize the signal number and TCB ptr
		MOVE.L	#HDF_DEVDONE,D0
		LEA	HD_TCB(A6),A1
		LINKSYS Signal

	
Term_End:
		RTS

Soft_Error:	;	Returns ZERO condition if not a hard error code

		AND.B	#$7F,D0			; 	Clear hi-order bit
		BEQ.S	Not_Hard		;	If zero, no error
		CMP.B	#$18,D0			;	Fixed ECC error ?
		BEQ.S	Not_Hard		;		Yes - Ignore
		CMP.B	#$22,D0			;	Soft read header error ?
		BEQ.S	Not_Hard		;		Yes - Ignore
		CMP.B	#$23,D0			;	Soft write header err ?
		BEQ.S	Not_Hard		;		Yes - Ignore
		CMP.B	#$24,D0			;	Soft data error ?
		BEQ.S	Not_Hard		;		Yes - Ignore
		CMP.B	#$28,D0			;	Soft DMA error ?
		BEQ.S	Not_Hard		;		Yes - Ignore
*BART		MOVE.L  #HDERR_NotSpecified,D0  ; set error code
		BRA.S	Is_Hard			; Is HARD error!

Not_Hard:
		CLR.L	D0			; Show NO error
Is_Hard:
		RTS

	END
