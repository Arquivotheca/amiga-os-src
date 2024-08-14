
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* task.asm
*
* Source Control
* ------ -------
* 
* $Id: task.asm,v 27.10 91/03/13 20:45:39 jesup Exp $
*
* $Locker:  $
*
* $Log:	task.asm,v $
* Revision 27.10  91/03/13  20:45:39  jesup
* Fixed CMD_START - when stopped, clear the active bit
* 
* Revision 27.9  90/11/21  04:19:27  jesup
* *** empty log message ***
* 
* Revision 27.8  90/06/01  23:18:09  jesup
* Conform to include standard du jour
* 
* Revision 27.7  90/03/03  16:20:01  jesup
* fixed the signaling stuff, pass task as arg to new task
* 
* Revision 27.6  90/03/01  23:00:55  jesup
* OpenDevice waits for the task to start before returning (uses SIGF_SINGLE)
* 
* Revision 27.5  89/12/10  18:35:57  jesup
* Support for Stop/Start
* 
* Revision 27.4  89/04/27  23:40:19  jesup
* fixed autodocs
* 
* Revision 27.3  89/02/17  19:22:37  jesup
* minor cleaup
* 
* Revision 27.2  89/02/16  18:02:23  jesup
* minor branch optimizations
* 
* Revision 27.1  85/06/24  13:37:50  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  15:13:33  neil
* *** empty log message ***
* 
* 
*************************************************************************

****** Included Files ***********************************************

	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/io.i'
	INCLUDE 'exec/devices.i'
	INCLUDE 'exec/tasks.i'
	INCLUDE 'exec/ables.i'
	INCLUDE 'exec/interrupts.i'
	INCLUDE	'exec/execbase.i'

	INCLUDE 'resources/disk.i'

	INCLUDE 'devices/timer.i'

	INCLUDE 'trackdisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'

	SECTION section


****** Imported Globals *********************************************

	XREF	tdName

****** Imported Functions *******************************************

	EXTERN_LIB GetMsg
	EXTERN_LIB RemTask
	EXTERN_LIB Wait
	EXTERN_LIB Signal

	XREF	TDChangeTick
	XREF	TDMotor
	XREF	TDWriteBuffer
	XREF	PerformIO

	TASK_ABLES

****** Exported Functions *******************************************

	XDEF	TDTaskStart
	XDEF	perform


***** Local Definitions **********************************************


*****i* trackdisk.device/internal/TDTaskStart *************************
*
*   NAME
*	TDTaskStart - main unit task code
*
*   SYNOPSIS
*	TDTaskStart( TDUnit, TDLib, parent_task )
*		      4(sp)   8(sp)    12(sp)
*
*   FUNCTION
*	This driver is modelled as two independant, mutually asynchronous
*	units -- the read side and the write side.  This gets to be a
*	little fun because we are supposed to look like only one unit
*	to the user.
*
*	We will do this by having three unit structures:  One for the
*	main task (for GoIO to send things to), one for reading, and
*	one for writting. (!!! What should we do for CONTROL operations?)
*
***********************************************************************


TDTaskStart:
*		;------ Grab the arguments
		MOVE.L	12(SP),A2	; task who started us
		MOVE.L	8(SP),A6
		MOVE.L	4(SP),A3

		IFGE	INFO_LEVEL-40
		MOVE.L	A3,-(SP)
		MOVE.L	A6,-(SP)
		INFOMSG	40,<'%s/TDTaskStart: dev 0x%lx unit 0x%lx'>
		ADDQ.L	#8,SP
		ENDC

*		;------ Initialize the unit ports(s).
		LEA	TDU_TCB(A3),A0
		MOVE.L	A0,MP_SIGTASK(A3)

*		;------ kick off the timer daemon
		BSR	TDChangeTick

*		;------ Say we're ready for business (we know if a disk is in)
		MOVE.L	A2,A1			; task that called OpenDevice
		MOVEQ	#SIGF_SINGLE,D0		; convenient
		MOVE.L	A6,A5			; save device ptr
		MOVE.L	TD_SYSLIB(A6),A6
		JSR	_LVOSignal(A6)
		MOVE.L	A5,A6			; restore device ptr

task_start:
*		;------ Wait for something to do
		MOVE.L	#TDF_PORT!TDF_DEVDONE,D0
		LINKSYS Wait

		BSR.S	perform
		BRA.S	task_start

perform:
*		;------ See if the unit(s) is busy
		BSET.B	#UNITB_ACTIVE,UNIT_FLAGS(A3)
		BNE	perform_end

perform_msg:
*		;------ Make sure we haven't been stopped
*		;------ To make this work, CMD_START _must_ be immediate!
*		;------ Goes to clear to indicate we're not in perform now
		BTST.B	#TDUB_STOPPED,TDU_FLAGS(A3)
		BNE	perform_clear

*		;------ The unit is not busy.  Start it up.
		MOVE.L	A3,A0
		LINKSYS GetMsg

*		;------ See if the port is empty
		TST.L	D0
		BEQ	perform_clear

		MOVE.L	D0,A2

*		;------ see if we had a delayed motor off command
		BCLR	#TDUB_DELMOTOROFF,TDU_FLAGS(A3)
		BEQ.S	perform_timer

		INFOMSG 50,<'%s/perform: delayed cleanup'>

*		;----- If the buffer was dirty, write it out
		MOVE.L	TDU_BUFPTR(A3),A0
		BCLR	#TDBB_DIRTY,TDB_FLAGS(A0)
		BEQ.S	perform_stale

		MOVE.L	A0,TDU_BUFFER(A3)
		BSR	TDWriteBuffer

perform_stale:
*		;----- Mark the buffer as invalid
		MOVE.L TDU_BUFPTR(A3),A0
		MOVEQ	#-1,D0
		MOVE.W	D0,TDB_TRACK(A0)

*		;------ force a calibrate
		MOVE.W	D0,TDU_SEEKTRK(A3)

*		;------ turn the motor off
		MOVEQ	#0,D0
		BSR	TDMotor

		MOVE.L	A6,A0
		MOVE.L	TD_SYSLIB(A0),A6
		FORBID
		MOVE.L	A0,A6

		TST.W	UNIT_OPENCNT(A3)
		BNE.S	perform_notexpunge

		INFOMSG 50,<'%s/perform: removing unit'>

		;------ free up the resource unit
		CLEAR	D0
		MOVE.B	TDU_UNITNUM(A3),D0
		LINKLIB	DR_FREEUNIT,TD_DISCRESOURCE(A6)

		;------ mark us as gone
		LEA	TDU0(A6),A0
		CLEAR	D0
		MOVE.B	TDU_UNITNUM(A3),D0
		LSL.L	#2,D0
		ADD.L	D0,A0
		CLR.L	(A0)

		;------ and make us go away
		SUB.L	A1,A1
		LINKSYS	RemTask,TD_SYSLIB(A6)

		;------ never get here (hope, hope)
		INFOMSG	40,<'%s/perform:  came back after RemTask!!'>


perform_notexpunge:
		LINKSYS	Permit,TD_SYSLIB(A6)


perform_timer:
		MOVE.L	A2,A1
		LEA	TDU_CHANGETIMER(A3),A0
		CMP.L	A0,A2
		BNE.S	perform_request

*		;------ This is not really a user request -- it is one of our
*		;------ timers.

		BSR	TDChangeTick
		BRA	perform_msg

perform_request:

*		;------ mark us as being in the "driver task"
		BSET	#UNITB_INTASK,UNIT_FLAGS(A3)

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
		BCLR	#UNITB_INTASK,UNIT_FLAGS(A3)
		BCLR	#UNITB_ACTIVE,UNIT_FLAGS(A3)


perform_end:
		RTS

	END
