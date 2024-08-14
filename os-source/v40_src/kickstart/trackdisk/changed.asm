 
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* changed.asm
*
* Source Control
* ------ -------
* 
* $Id: changed.asm,v 33.22 92/05/27 21:23:20 jesup Exp $
*
* $Locker:  $
*
* $Log:	changed.asm,v $
* Revision 33.22  92/05/27  21:23:20  jesup
* initbuffer should be TDInitBuffer
* 
* Revision 33.21  92/05/27  21:20:01  jesup
* God, I'm a bozo.  Must set slop to $aaaaaaaa when going from LD to HD!
* 
* Revision 33.20  91/04/14  00:44:02  jesup
* Forgot to xdef remchangeint
* 
* Revision 33.19  91/03/13  20:37:45  jesup
* removed extra selects of the drive
* 
* Revision 33.18  91/01/17  15:06:31  jesup
* Add changes to handle buffer allocation failure
* 
* Revision 33.17  91/01/10  18:46:31  jesup
* Added DRIVExxxx changes to density switch
* 
* Revision 33.16  90/11/28  23:43:00  jesup
* TDB_DATA now a variable.  CHanges to order of stuff (re: initbuffer call)
* so things are set up right.
* 
* Revision 33.15  90/11/21  04:04:27  jesup
* Added code to swotch densities
* 
* Revision 33.14  90/06/01  23:14:42  jesup
* Conform to include standard du jour
* 
* Revision 33.13  90/06/01  16:26:38  jesup
* Fix for STUPID Newtronics drives
* 
* Revision 33.12  89/12/10  18:26:59  jesup
* Added comments, don't delay if we didn't change direction
* 
* Revision 33.11  89/09/06  18:52:25  jesup
* Clear seconds field before reusing for V36 timer
* 
* Revision 33.10  89/05/15  21:15:10  jesup
* Added comments about GetUnit usage
* 
* Revision 33.9  89/05/02  21:05:50  jesup
* Added code to make remchangeint work (really ugly, but safe)
* 
* Revision 33.8  89/04/27  23:28:30  jesup
* fixed autodocs, removed TDRemChangeInt
* 
* Revision 33.7  89/03/22  17:30:10  jesup
* Added support for noclickstart, added settledelay on head dir change,
* eliminated redundant code.
* 
* Revision 33.6  89/02/17  18:58:42  jesup
* Fixed RemChangeInt bug, minor code improvements
* 
* Revision 33.5  89/01/04  17:02:42  jesup
* GiveUnit without getunit fix
* 
* Revision 33.4  86/04/10  00:56:05  neil
* Added AddChangeInt and RemChangeInt
* 
* Revision 33.3  86/04/09  16:07:08  neil
* 68020 changes for real
* 
* Revision 33.2  86/04/04  09:35:32  neil
* changed to lenghthen delay loops for a 68020
* 
* Revision 33.1  86/03/29  14:11:36  neil
* made seek and settle time programmable.  Isolated unit specific
* initializers to the beginning of the unit structure
* 
* Revision 31.1  85/09/04  23:11:44  neil
* Tick times changed: when disk is in the tick is every 1/2 sec.
* When disk is out the tick is every 2.5 secs.
* 
* Revision 27.5  85/07/18  00:26:13  neil
* tick moved back to three seconds by "Management directive".
* Sigh.
* 
* Revision 27.4  85/07/11  03:11:53  neil
* tick interval changed to two seconds
* 
* Revision 27.3  85/07/09  16:43:07  neil
* we now make sure we scan the disk (as if it was inserted) when
* the system is rebooted
* 
* Revision 27.2  85/06/28  15:54:24  neil
* reseek time changed to 3 seconds (from 1/2 second).
* 
* Revision 27.1  85/06/24  13:36:25  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  15:12:56  neil
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
	INCLUDE 'exec/interrupts.i'
	INCLUDE 'exec/ables.i'
	INCLUDE 'exec/alerts.i'
	INCLUDE 'exec/initializers.i'
	INCLUDE 'exec/memory.i'

	INCLUDE 'resources/disk.i'

	INCLUDE 'hardware/cia.i'

	INCLUDE 'devices/timer.i'

	INCLUDE 'trackdisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'

	SECTION section

****** Imported Globals *********************************************

	XREF		tdName

	XREF		_ciaa
	XREF		_ciab

****** Imported Functions *******************************************

	EXTERN_LIB	Cause
	EXTERN_LIB	GetMsg
	EXTERN_LIB	SendIO
	EXTERN_LIB	AllocMem
	EXTERN_LIB	FreeMem
	EXTERN_LIB	Alert
	EXTERN_LIB	InitStruct

	XREF		TDDelay
	XREF		TDGetUnit
	XREF		TDGiveUnit
	XREF		TDReadUnitID
	XREF		TDMotor
	XREF		TDSeek
	XREF		TermIO
	XREF		TDInitBuffer

	TASK_ABLES

****** Exported Functions *******************************************

	XDEF	TDChangeTick
	XDEF	TDCheckChange
	XDEF	TDUChangeNum
	XDEF	TDUChangeState
	XDEF	TDURemove
	XDEF	TDUAddChangeInt
	XDEF	TDURemChangeInt

***** Local Definitions **********************************************


*****i* trackdisk.device/internal/TDChangeTick ***********************
*
*   NAME
*       TDChangeTick - Process periodic timer tick
*
*   SYNOPSIS
*       TDChangeTick(), UnitPtr, TDDev
*           	        A3,	 A6
*
*   FUNCTION
*	This routine processes the periodic timer interrupt.  It then
*	reposts the timer.
*
*   INPUTS
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*

TDChangeTick:
*		;------ First, get us a good working environment

		IFGE	INFO_LEVEL-50
		PEA	0
		MOVE.B	TDU_UNITNUM(A3),3(SP)
		INFOMSG	50,<'%s/TDChangeTick: called for unit %ld'>
		ADDQ.L	#4,SP
		ENDC

		BSR.S	TDCheckChange

TDChangeTick_rearm:
		LEA	TDU_CHANGETIMER(A3),A1

*		;------ re arm the message and send it back

		;------ set micros to 1/2 second
		MOVE.L	#500000,IOTV_TIME+TV_MICRO(A1)
		CLR.L	IOTV_TIME+TV_SECS(A1)

		BTST	#TDUB_REMOVED,TDU_FLAGS(A3)
		BEQ.S	TDChangeTick_SendIO

		;------ set seconds to 2 if disk is removed (total 2.5)
		MOVEQ.L	#2,D0
		MOVE.L	D0,IOTV_TIME+TV_SECS(A1)

TDChangeTick_SendIO:
		LINKSYS SendIO

TDChangeTick_End:
		RTS

*****i* trackdisk.device/internal/TDCheckChange **********************
*
*   NAME
*       TDCheckChange - Check the state of the disc for removal
*
*   SYNOPSIS
*       TDCheckChange(), UnitPtr, TDDev
*           	         A3,	  A6
*
*   FUNCTION
*	This routine deals with disc changing and its ramifications.
*	It should be called when the disc is NOT selected
*
*   INPUTS
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*

TDCheckChange:
		MOVEM.L	D2-D4/A2/A5,-(SP)

		BSR	TDGetUnit

*		;------ check to see if the disc has been removed
		;------ MUST select unit before reading lines!
		LEA	_ciab+ciaprb,A2
*		MOVE.B	TDU_6522(A3),(A2)

	;-- STUPID Newtronics drives!! (I wish I could FIX this)
	;-- The change lines doesn't go low for ~12us after select is asserted.
	;-- Since we can't access the cia any faster than 1/us, this should
	;-- work.  1.3 was slower at reading the lines, apparently.
	;-- this is ~26-34 cycles/loop on a 68000, or about 4us/loop.  I use
	;-- 12 since I may be running on something much faster.

	moveq #14,d0			; bumped to 14 for extra safety
stupid_loop:
		MOVE.B	_ciaa+ciapra,D2
	dbra  d0,stupid_loop

*		;------ give the drive up
		BSR	TDGiveUnit

		BTST	#CIAB_DSKCHANGE,D2
		BEQ.S	CC_Check

		;------ the hardware says there is a disk in the drive.  What
		;------ does the software say?
		BTST	#TDUB_REMOVED,TDU_FLAGS(A3)
		BEQ	CC_End

		;------ they disagree.  go and reread the parameters
		BRA.S	CC_InitialCheck

CC_Check:
*		;------ The change line is active.   Have we noticed it before?
		;------ (There is no disk in the drive.  It may have just
		;------  been removed (goes low immediately), or may have
		;------  been removed some time ago.  Step drive to gate in
		;------  new value as diskchange (looking for insertion).)
		BSET	#TDUB_REMOVED,TDU_FLAGS(A3)
		BNE.S	CC_Step

*		;------ The disk was JUST removed
		ADDQ.L	#1,TDU_COUNTER(A3)

	IFGE	INFO_LEVEL-50
		MOVE.L	TDU_COUNTER(A3),-(SP)
		PEA	0
		MOVE.B	D2,3(SP)
		MOVE.L	A3,-(SP)
		PEA	0
		MOVE.B	TDU_UNITNUM(A3),3(SP)
		INFOMSG	50,<'%s/TDCheckChange: Disk %ld:%lx:%lx removed (%ld)'>
		ADDQ.L	#8,SP
		ADDQ.L	#8,SP
	ENDC
		;------ keep the drive from going "klunk" as it hits the stop
		MOVEQ	#6,D0
		BSR	TDSeek

*		;------ Turn off the motor
		CLEAR	D0
		BSR	TDMotor

*		;------ Notify the user of the change
		BSR	SendSoftInt
		;------ Now check to see if there was an insertion.

CC_Step:
*
*	This symbol was AFTER the move.b TDU_6522!  REJ
*
*		; get the unit back - must select!
		BSR	TDGetUnit
*		MOVE.B	TDU_6522(A3),(A2)

*		;------ Step the drive.  We will recalibrate it eventually
*		;------ anyway, so don't worry about where it is.
*		;------ if noclickstart flag is set, always step out.
*		;------ to save space, clear it then change it to set it.
*		;------ D3 is a flag to tell if I changed direction.
		MOVEQ	#0,d3				; assume changed (d3=0)
		BTST.B	#TDPB_NOCLICK,TDU_PUBFLAGS(A3)
		BEQ.S	CC_Click
		BCLR	#CIAB_DSKDIREC,TDU_6522(A3)	; will get changed!
		SNE	d3				; d3=1 if bit was 1
CC_Click:
		BCHG	#CIAB_DSKDIREC,TDU_6522(A3)

		MOVE.B	TDU_6522(A3),D0
		MOVE.B	D0,D1
		BCLR	#CIAB_DSKSTEP,D1
		MOVE.B	D1,(A2)
		MOVE.B	D0,(A2)

*		;------ give the drive up
		BSR	TDGiveUnit

*		;------ Wait for heads to settle (settle may not be needed)
		TST.B	D3		; seq sets low byte
		BNE.S	CC_NoDirChange
		MOVE.L	TDU_STEPDELAY(A3),D0
		ADD.L	TDU_SETTLEDELAY(A3),D0
		BSR	TDDelay
CC_NoDirChange:
		;------ we come here if the drive is marked removed but
		;------ the change bit is not set.  The drive may
		;------ be in this state at initialization time.
		;------ this way we check for write protect properly
CC_InitialCheck:

*		;------ get the unit back - must select!
		BSR	TDGetUnit
*		MOVE.B	TDU_6522(A3),(A2)

*		;------ see if the disc has been inserted
	;-- STUPID Newtronics drives!! (I wish I could FIX this)
	;-- The change lines doesn't go low for ~12us after select is asserted.
	;-- Since we can't access the cia any faster than 1/us, this should
	;-- work.  1.3 was slower at reading the lines, apparently.
	moveq #12,d0
stupid_loop2:
		MOVE.B	_ciaa+ciapra,D2
	dbra  d0,stupid_loop2

		BTST	#CIAB_DSKCHANGE,D2
		BEQ	CC_End_GiveUp

*		;------ The disc is reinserted!
*		;------ the following code is useless - the motor on was
*		;------ already removed, and otherwise it's a copy of above

*****		MOVEQ	#1,D0
*****		BSR	TDMotor
*
*		;------ get the unit back
*		BSR	TDGetUnit
*		MOVE.B	TDU_6522(A3),(A2)
*
*		;------ see if the disc has been inserted
*		MOVE.B	_ciaa+ciapra,D2
*
*		;------ give the drive up
*		BSR	TDGiveUnit
*
*****		MOVEQ	#0,D0
*****		BSR	TDMotor

		ADDQ.L	#1,TDU_COUNTER(A3)

	IFGE	INFO_LEVEL-50
		MOVE.L	TDU_COUNTER(A3),-(SP)
		PEA	0
		MOVE.B	D2,3(SP)
		MOVE.L	A3,-(SP)
		PEA	0
		MOVE.B	TDU_UNITNUM(A3),3(SP)
	INFOMSG	50,<'%s/TDCheckChange: Disc %ld:%lx:%lx reinserted (%ld)'>
		ADDQ.L	#8,SP
		ADDQ.L	#8,SP
	ENDC

		;------ see if the disk is write protected
		BCLR	#TDUB_PROTECTED,TDU_FLAGS(A3)
		BTST	#CIAB_DSKPROT,D2
		BNE.S	CC_ForceCalibrate	; protect is active low
		BSET	#TDUB_PROTECTED,TDU_FLAGS(A3)

CC_ForceCalibrate:
		MOVE.W	#-1,TDU_SEEKTRK(A3)

		;------ it is very important to update this AFTER updating
		;------ the TDU_PROTECTED flag.
		;------ Also, since we changed disks allow us to read it again.
		BCLR.B	#TDUB_REMOVED,TDU_FLAGS(A3)
		BCLR.B	#TDUB_UNREADABLE,TDU_FLAGS(a3)

		;------ Ok, now check to see if the ID changed.
		bsr	TDReadUnitID
		cmp.l	TDU_DISKTYPE(a3),d0
		beq	CC_SameType

	IFGE	INFO_LEVEL-40
	move.l	d0,-(sp)
	move.l	TDU_DISKTYPE(a3),-(sp)
	PUTMSG	40,<'%s/TDCheckChange: Type has changed from 0x%lx to 0x%lx'>
	addq.l	#8,sp
	ENDC
		;------ The type of the drive changed - update internal
		;------ variables!
		;------ This assumes that only 150RPM C= drives can change!!!
		;------ FIX! should really put back to 1M if not understood!
		lea	Disk_1M_Rtn(pc),a1
		cmp.l	#DRT_AMIGA,d0
		beq.s	CC_InitStruct
		cmp.l	#DRT_150RPM,d0
		bne	CC_SameType		; Not a 150 rpm floppy, punt
		lea	Disk_2M_Rtn(pc),a1

CC_InitStruct:
		move.l	a1,d4			; save init routine addr

		;------ Test to see if we need to free the old buffer if it
		;------ was small.  If it changed to small from large, the
		;------ buffer must be large already.
		;------ if it's already large, we still need to call a1 rtn!
		btst.b	#TDUB_LARGE,TDU_FLAGS(A3)
		bne.s	CC_NoBufNeeded		; already large, all done

		;------ Free the old, smaller buffer
		move.l	a6,a5			; save device structure
		move.l	TD_SYSLIB(a6),a6	; get execbase

		;------ Forbid so we KNOW we can get our buffer back
		FORBID

		move.l	TDU_BUFPTR(a3),a1	; buffer to free
		move.l	#TDB_SIZE,d0		; size of (small) buffer
		CALLSYS	FreeMem			; (a1,d0)

		;------ Allocate larger buffer
		move.l	#TDB_LARGE_SIZE,d0
		move.l	#MEMF_CHIP,d1
		CALLSYS	AllocMem		; (d0,d1)
		move.l	d0,TDU_BUFPTR(a3)
		bne.s	CC_GotBigBuf

		;------ Couldn't allocate the buffer!!!!!!! ALERT!!!!
		;------ allocate a smaller buffer again- MUST succeed!
		move.l	#TDB_SIZE,d0
		move.l	#MEMF_CHIP,d1
		CALLSYS	AllocMem
		move.l	d0,TDU_BUFPTR(a3)
		bne.s	1$

		;------ Must be Bill playing with memoration again...
		move.l	#AT_DeadEnd!AG_NoMemory!AN_TrackDiskDev,d7
		CALLSYS	Alert			; never comes back

1$:		;------ failed to allocate big buffer - mark as unreadable
		BSET.B	#TDUB_UNREADABLE,TDU_FLAGS(a3)
		lea	Disk_1M_Rtn(pc),a1	; make sure nothing changes!
		move.l	a1,d4
		move.l	#TDB_SIZE,d1		; size for mementry
		bra.s	CC_DifferentBuf		; cleanup

CC_GotBigBuf:	;------ got the memory, update bit
		bset.b	#TDUB_LARGE,TDU_FLAGS(a3)
		move.l	#TDB_LARGE_SIZE,d1

CC_DifferentBuf:;-- buffer was changed, may not be larger
		;------ update entry in AllocEntry structure - NASTY!
		move.l	TDU_ENTRY(a3),a0
		move.l	d0,ML_ME+(ME_SIZE*2)(a0)
		move.l	d1,ML_ME+(ME_SIZE*2)+4(a0)

		PERMIT

		move.l	a5,a6			; restore device base

	PUTMSG	40,<'%s/TDCheckChange: Initing structure'>
		;------ We now have a ptr to the function to update
		;------ the unit structure - go to it.  Uses TDU_BufPtr.
		move.l	d4,a1

		;------ we allocated a buffer, fall through (InitBuffer is
		;------ called by the setup routines).

CC_NoBufNeeded: ;------ different type, large buffer already allocated
		jsr	(a1)

CC_SameType:
		;------ Notify people that a disk was inserted!
		BSR.S	SendSoftInt

CC_End_GiveUp:
*		;------ give the drive up
		BSR	TDGiveUnit

CC_End:
		MOVEM.L	(SP)+,D2-D4/A2/A5
		RTS


SendSoftInt:
		MOVEM.L	D2/A6,-(SP)
		MOVE.L	TD_SYSLIB(A6),A6

		MOVE.L	TDU_REMOVEINT(A3),D0
		BEQ.S	SSI_List

		MOVE.L	D0,A1
		CALLSYS	Cause

SSI_List:
		;-- arbitrate access for safety
		; a6 has execbase
		FORBID
		MOVE.L	TDU_CHANGELIST(A3),D2
SSI_Loop:
		MOVE.L	D2,A1
		MOVE.L	(A1),D2
		BEQ.S	SSI_End

		MOVE.L	IO_DATA(A1),A1
		CALLSYS	Cause

		BRA.S	SSI_Loop

SSI_End:
		PERMIT				; can't use JMP_PERMIT
		MOVEM.L	(SP)+,D2/A6
		RTS


*
* tables for updating the unit structures
*
Disk_1M_Rtn:
		MOVE.L	#160*11*TD_SECTOR,TDU_MAXOFFSET(a3)
		MOVE.L	#TDT_DISKSYNC,TDU_TDT_DISKSYNC(a3)
		MOVE.W	#11,TDU_NUMSECS(a3)
		MOVE.W	#MFM_TRKBUF,TDU_MFM_TRKBUF(a3)
		MOVE.W	#MFM_MAXTRACK,TDU_MFM_MAXTRACK(a3)
		MOVE.W	#MFM_SLOP,TDU_MFM_SLOP(a3)
		MOVE.L	#DRT_AMIGA,TDU_DISKTYPE(a3)
		MOVE.B	#DRIVE3_5,TDU_DRIVETYPE(a3)
		; this sets TDB_DATA and sets the slop to $aaaaaaaa
		bra	TDInitBuffer

*
Disk_2M_Rtn:
		MOVE.L	#160*22*TD_SECTOR,TDU_MAXOFFSET(a3)
		MOVE.L	#TDT_DISKSYNC*2,TDU_TDT_DISKSYNC(a3)
		MOVE.W	#22,TDU_NUMSECS(a3)
		MOVE.W	#MFM_BIG_TRKBUF,TDU_MFM_TRKBUF(a3)
		MOVE.W	#MFM_BIG_MAXTRACK,TDU_MFM_MAXTRACK(a3)
		MOVE.W	#MFM_BIG_SLOP,TDU_MFM_SLOP(a3)
		MOVE.L	#DRT_150RPM,TDU_DISKTYPE(a3)
		MOVE.B	#DRIVE3_5_150RPM,TDU_DRIVETYPE(a3)
		; this sets TDB_DATA and sets the slop to $aaaaaaaa
		bra	TDInitBuffer

*****i* trackdisk.device/internal/TDUChangeNum **********************
*
*   NAME
*       TDUChangeNum - return the current disc change number
*
*   SYNOPSIS
*       TDUChangeNum( IORequest ), UnitPtr
*		      A1           A3
*
*   FUNCTION
*       This routine checks to see if there is a disc in the drive
*	one the specified unit.
*
*   INPUTS
*	IORequest - a standard IO Request block
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*

TDUChangeNum:
		MOVE.L	TDU_COUNTER(A3),IO_ACTUAL(A1)

		BRA	TermIO		; was BSR
*		RTS

*****i* trackdisk.device/internal/TDUChangeState ********************
*
*   NAME
*       TDUChangeState - Return the current state of the disc
*
*   SYNOPSIS
*       TDUChangeState( IORequest ), UnitPtr
*		        A1           A3
*
*   FUNCTION
*       This routine checks to see if there is a disc in the drive
*	one the specified unit.
*
*   INPUTS
*	IORequest - a standard IO Request block
*
*   RESULTS
*	IO_ACTUAL -- nonzero if there is no diskette in the drive
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*

TDUChangeState:
		CLEAR	D0
		BTST	#TDUB_REMOVED,TDU_FLAGS(A3)
		SNE	D0
		MOVE.L	D0,IO_ACTUAL(A1)

		BRA	TermIO		; was BSR

*		RTS

*****i* trackdisk.device/internal/TDURemove *************************
*
*   NAME
*       TDURemove - handle the TD_REMOVE call
*
*   SYNOPSIS
*       TDURemove( IORequest ), UnitPtr
*		   A1           A3
*
*   FUNCTION
*       This routine handles the saving of the user specifed software
*	interrupt.
*
*   INPUTS
*	IORequest - a standard IO Request block
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*

TDURemove:
		MOVE.L	IO_DATA(A1),TDU_REMOVEINT(A3)

		BRA	TermIO		; was BSR

*		RTS

*****i* trackdisk.device/internal/TDUAddChangeInt *******************
*
*   NAME
*       TDUAddChangeInt - add a new change software int
*
*   SYNOPSIS
*       TDUAddChangeInt( IORequest ), UnitPtr
*		         A1           A3
*
*   FUNCTION
*	Alas, TDURemove is not robust enough.  This routine supports
*	an extensible list of software interrupts for use by many
*	different supporting drivers.
*
*	The call does not "complete" (e.g. TermIO).  The request
*	is stashed until TDURemChangeInt is called, when it is
*	finally replied.
*
*   INPUTS
*	IORequest - a standard IO Request block (IO_DATA-> soft int struct).
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*	Forbids to avoid problems with RemChangeInt.
*

TDUAddChangeInt:
		LEA	TDU_CHANGELIST(a3),a0
		MOVE.L	A6,-(SP)
		MOVE.L	TD_SYSLIB(A6),A6
		FORBID
		ADDTAIL
		PERMIT
		MOVE.L	(SP)+,A6
		RTS


*****i* trackdisk.device/internal/TDURemChangeInt *******************
*
*   NAME
*       TDURemChangeInt - remove a change software int
*
*   SYNOPSIS
*       TDURemChangeInt( IORequest ), UnitPtr
*		         A1           A3
*
*   FUNCTION
*	This function unlinks the IORequest stashed by AddChangeInt.
*	NOTE: it is ALWAYS executed as IOF_QUICK, even if not specified!!!!
*	This is because we can't re-queue a request already on a queue.
*
*   INPUTS
*	IORequest - a standard IO Request block
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
*	As of V36, using this io command is safe.  Before V36 bad things
*	would happen.
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
TDURemChangeInt:
	; a3 has unit ptr, a1 has ior, a6 has dev ptr
	; must forbid because other task is accessing
	MOVE.L	A3,A0
	MOVE.L	A6,-(SP)
	MOVE.L	TD_SYSLIB(A6),A6		; execbase
	FORBID
	LEA.L	TDU_CHANGELIST(A0),A0		; get pointer to list
	REMOVE					; destroys A0 & A1
	PERMIT
	MOVE.L	(SP)+,A6
	RTS


	END
