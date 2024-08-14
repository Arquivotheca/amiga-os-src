
*************************************************************************
*									*
*	Copyright (C) 1986, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* task.asm
*
* Source Control
* ------ -------
* 
* $Header: task.asm,v 27.1 86/01/17 13:37:50 LCE Exp $
*
* $Locker:  $
*
* $Log:	task.asm,v $
* *** empty log message ***
* 
* 
*************************************************************************

******* Included Files ***********************************************

	NOLIST
	INCLUDE 'exec/types.i'
	INCLUDE	"exec/interrupts.i"
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/io.i'
	INCLUDE 'exec/devices.i'
	INCLUDE 'exec/tasks.i'
	INCLUDE 'exec/memory.i'
	INCLUDE 'exec/execbase.i'
	INCLUDE 'exec/ables.i'
	INCLUDE 'exec/errors.i'
	INCLUDE 'exec/alerts.i'

	INCLUDE 'hddisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'
	LIST

	SECTION section


******* Imported Globals *********************************************

	XREF	hdName

******* Imported Functions *******************************************

	EXTERN_LIB GetMsg
	EXTERN_LIB RemTask
	EXTERN_LIB Wait
	EXTERN_LIB AddIntServer
	EXTERN_LIB AllocSignal
	EXTERN_LIB Debug

	IFD	HASSCSI
	XREF	SCSIINIT		; Init. the SCSI & DMA chips
	ENDC

	XREF	PerformIO

	TASK_ABLES

******* Exported Functions *******************************************

	XDEF	HDTaskStart
	XDEF	perform


***** Local Definitions **********************************************


*---------------------------------------------------------------------
*
* Task stuff
*
*---------------------------------------------------------------------
*
* This driver doesn't handle any "immediate" I/O.  First, it waits for
* a command to be received.  It re-formats and issues that command to the
* hard disk controller, then waits for the interrupt handler to indicate the
* command is complete.  It returns the status to the issuer of the command by
* replying to the message that started the I/O, then goes and waits for another
* command.
*

HDTaskStart:


*		;------ Grab the argument
		MOVE.L	4(SP),A6		; Device pointer

		PUTMSG	50,<'%s/HDTaskStart: called'>
		IFGE	INFO_LEVEL-40
		MOVE.L	A6,-(SP)
		INFOMSG	40,<'%s/HDTaskStart: Device 0x%lx'>
		ADDQ.L	#4,SP
		ENDC

		MOVE.L	HD_BASE(A6),A1		; Get base address of controller

		IFGE	INFO_LEVEL-40
		MOVE.L	A1,-(SP)
		INFOMSG	40,<'%s/Task: BASE 0x%lx'>
		ADDQ.L	#4,SP
		ENDC

		BTST.B	#HDSCSIB,HDSCSIO(A1)
		BNE.B	HAS_512
		BSET.B	#HDB_NO512,HD_FLAGS(A6)	; Doesn't have KONAN
		BRA.S	NO_512			;   Skip handshake & set flag
HAS_512		MOVE.L	HD_CMDPTR(A6),D0	; Now tell KONAN where command
		MOVEQ.L	#9,D1			; 	block is
		LSR.L	D1,D0
		MOVEQ.L	#1,D1			; Set up loop count, 3 times
CBPLp1:		MOVE.B	D0,HDINTACK(A1)		; Output byte to KONAN
		MOVE.B	#$E0,HDSTAT1(A1)	; Start handshake
CBPLp2:		BTST	#7,HDSTAT1(A1)		; Wait for KONAN to signal
		BEQ.S	CBPLp2
		MOVE.B	#$60,HDSTAT1(A1)	; Complete handshake
CBPLp3:		BTST	#7,HDSTAT1(A1)
		BNE.S	CBPLp3
		LSR.L	#8,D0			; Shift to next byte of addr
		DBRA	D1,CBPLp1		; Loop 3 times
NO_512:
		PUTMSG	50,<'%s/Task: Done Handshake'>
		IFD	HASSCSI
		IFGE	INFO_LEVEL-40
		MOVE.L	A6,-(SP)
		INFOMSG	40,<'%s/Task: Device 0x%lx'>
		ADDQ.L	#4,SP
		ENDC

		BSR	SCSIINIT		; Init. the SCSI & DMA chips
		PUTMSG	50,<'%s/Task: Back from SCSI init'>
		ENDC
		
task_start:

		BSR.S	perform


task_wait:
*		;------ Wait for something to do
		MOVE.L	#HDF_NEWIOREQ!HDF_DEVDONE,D0
		LINKSYS Wait
		BRA	task_start


perform:
*		;------ See if the unit(s) is busy
		BSET.B	#HDB_ACTIVE,HD_FLAGS(A6)
		BNE	perform_end

perform_msg:
*		;------ The unit is not busy.  Start it up.
		PUTMSG	50,<'%s/Task: about to GetMsg'>
		LEA.L	HD_MP(A6),A0
		LINKSYS GetMsg

*		;------ See if the port is empty
		TST.L	D0
		BEQ	perform_clear

		MOVE.L	D0,A1

perform_request:

		IFGE	INFO_LEVEL-50
		PEA	0
		MOVE.W	IO_COMMAND(A1),2(SP)
		MOVE.L	A1,-(SP)
		INFOMSG	50,<'%s/task: performing IOB 0x%lx command 0x%lx'>
		ADDQ.L	#8,SP
		ENDC

*		;------ actually do the IO
		BSR	PerformIO

		BRA	perform_msg

perform_clear:
*		;------ There is no more work to do.  Clear the driver.
		BCLR.B	#HDB_ACTIVE,HD_FLAGS(A6)


perform_end:
		RTS

	END
