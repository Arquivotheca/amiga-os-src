
*************************************************************************
*									*
*	Copyright (C) 1985,1988,1989,1990 Commodore Amiga Inc.		*
*	All rights reserved.						*
*									*
*************************************************************************

*************************************************************************
*
* driver.asm
*
* Source Control
* ------ -------
* 
* $Id: driver.asm,v 33.40 93/03/12 16:25:44 jesup Exp $
*
* $Locker:  $
*
* $Log:	driver.asm,v $
* Revision 33.40  93/03/12  16:25:44  jesup
* Use ODTAG_TIMER for OpenDevice
* 
* Revision 33.39  92/08/10  18:17:16  jesup
* Use TaggedOpenLibrary
* 
* Revision 33.38  92/04/05  19:47:24  jesup
* track mods to unit structure, use more equates for initialization
* 
* Revision 33.37  91/05/02  01:15:36  jesup
* Fixed historical bug - AllocEntry returns negative(!) for failure
* 
* Revision 33.36  91/04/21  04:08:09  jesup
* Made GetGeometry non-immediate again
* 
* Revision 33.35  91/04/21  03:46:42  jesup
* Cleaner and smaller version of the fix.  Make sure dirty bit is clear
* when setting the tdb_track to -1.
* 
* Revision 33.34  91/04/21  03:08:44  jesup
* NAILED IT!  The reason some floppies "wouldn't format" is that the
* format code was flushing dirty buffers that weren't real (since it was
* bypassing the normal sector write code).  Adding a check for track -1
* (set at the end of format) before checking the dirty bit fixes it
* (and should speed up format as a side effect).  Ugh.
* 
* Revision 33.33  91/04/14  00:42:38  jesup
* Brought the RemChangeInt code back to life
* 
* Revision 33.32  90/12/03  10:27:03  jesup
* Fixed drive type for 3.5 1M floppies in initstruct
* 
* Revision 33.31  90/11/28  23:41:00  jesup
* A bunch of changes to handle drives coming up in 150RPM modes.
* 
* Revision 33.30  90/11/21  04:05:58  jesup
* Changes for variable density floppies
* added init rtn for track  buffers (since we need to reallocate them now)
* Fixed Format to add $aaa8/$2aa8 on end of write buffer
* 
* Revision 33.28  90/07/29  23:38:08  jesup
* Fix old off-by-one error in offset clculations
* 
* Revision 33.27  90/06/08  15:28:30  jesup
* Saved bytes, use 1 ALERT instead of 5.
* Fixed InitUnit not getting flags (now in d3)
* made initial track for 5.25 drives $ffff so they calibrate.
* 
* Revision 33.26  90/06/01  23:14:54  jesup
* Conform to include standard du jour
* 
* Revision 33.25  90/06/01  21:19:20  jesup
* removed debug code, added GetGeometry
* Fixed TDU_MAXTRACKS access size
* 
* Revision 33.23  90/03/16  00:53:13  jesup
* Clearf SIGF_SINGLE before using, added comments
* 
* Revision 33.22  90/03/03  16:18:31  jesup
* fixed the signaling stuff, pass task as arg to new task
* 
* Revision 33.21  90/03/01  22:58:35  jesup
* OpenDevice waits for the task to start before returning (uses SIGF_SINGLE)
* 
* Revision 33.20  89/12/10  18:29:52  jesup
* Added STop/Start, added chip buffer for fastmem writes.
* 
* 
* Revision 33.19  89/05/08  19:13:55  jesup
* Made RemChangeInt stuff conditional until I get a chance to check it
* 
* Revision 33.18  89/05/08  15:28:23  jesup
* fixed broken cmdTable call
* 
* Revision 33.17  89/05/02  21:04:54  jesup
* Added code to make remchangeint work (really ugly, but safe)
* 
* Revision 33.16  89/04/27  23:29:36  jesup
* fixed autodocs, minor opt
* 
* Revision 33.15  89/04/12  23:29:30  bryce
* Change all references from private "InitXxxx" to Exec's "INITXXX".  44 bytes
* saved. Also changed InitStruct() terminator from DC.L to DC.W.  More savings.
* 
* Revision 33.14  89/04/12  13:03:13  jesup
* minor changes
* 
* Revision 33.13  89/03/23  14:20:53  jesup
* Another fix to the new command table - works
* 
* Revision 33.12  89/03/22  20:17:05  jesup
* Fixes to cmdTable definitions
* 
* Revision 33.11  89/03/22  17:31:14  jesup
* Keyboard back, should work (finally)
* saved space in the cmd and func tables (words not longs)
* structure init mods to track unit structure changes, small optimizations
* 
* Revision 33.10  89/03/08  23:50:04  jesup
* Removed keyboard stuff again (sigh).  Too dangerous, needs to be done
* right if at all.  Also removed a few NT_MESSAGE inits.
* 
* Revision 33.9  89/03/08  17:57:12  jesup
* minor fix to keyboard
* 
* Revision 33.8  89/03/08  17:46:57  jesup
* added keyboard reset handler back in
* 
* Revision 33.7  89/02/17  19:03:15  jesup
* commented out keyboard reset stuff, many minor code optimizations
* parameterized extra format delay, checks all bits of io_Command
* doesn't clear stack anymore, fills only gap with $aaaaaaaa's
* 
* Revision 33.6  89/01/23  17:53:09  jesup
* Fixed immediate commands called without the IOF_QUICK flag
* 
* Revision 33.5  86/07/09  18:36:26  neil
* Make BeginIO/immediate case pass io request in A2 to be
* compatible with PerformIO.  This fixes GetDriveType
* and GetNumTracks
* 
* Revision 33.4  86/04/10  00:56:21  neil
* Added AddChangeInt and RemChangeInt
* 
* Revision 33.3  86/04/04  15:43:49  neil
* Ooops, made the last change wrong...
* 
* Revision 33.2  86/04/03  23:29:18  neil
* made part of unit structure public
* 
* Revision 33.1  86/03/29  14:12:25  neil
* made seek and settle time programmable.  Isolated unit specific
* initializers to the beginning of the unit structure
* 
* Revision 32.2  86/01/03  19:50:54  neil
* Added reset catching code
* 
* Revision 32.1  85/12/23  17:17:57  neil
* Added rawread/rawwrite
* 
* Revision 28.1  85/07/12  08:37:03  neil
* Some format fixes to make it more bullet proof (poof...).
* 
* Revision 27.2  85/07/09  16:39:48  neil
* Made ProtStatus a non-immediate call.
* Minor tweak to termio to not save a register that was not used.
* 
* Revision 27.1  85/06/24  13:36:30  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  15:13:00  neil
* *** empty log message ***
* 
* 
*************************************************************************

	SECTION section

***** Included Files ***********************************************

	NOLIST
	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/io.i'
	INCLUDE 'exec/devices.i'
	INCLUDE 'exec/tasks.i'
	INCLUDE 'exec/interrupts.i'
	INCLUDE 'exec/memory.i'
	INCLUDE 'exec/execbase.i'
	INCLUDE 'exec/ables.i'
	INCLUDE 'exec/errors.i'
	INCLUDE 'exec/alerts.i'
	INCLUDE 'exec/initializers.i'

	INCLUDE 'resources/disk.i'
	INCLUDE 'resources/cia.i'

	INCLUDE	'hardware/cia.i'

	INCLUDE 'devices/timer.i'
	INCLUDE 'devices/keyboard.i'

	INCLUDE 'internal/librarytags.i'

	INCLUDE 'trackdisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'
	LIST


***** Imported Globals *********************************************

	XREF	tdName
	XREF	VERNUM
	XREF	REVNUM
	XREF	tdIDString

***** Imported Functions *******************************************

	EXTERN_LIB AddDevice
	EXTERN_LIB AddTask
	EXTERN_LIB Alert
	EXTERN_LIB AllocEntry
	EXTERN_LIB CloseDevice
	EXTERN_LIB CloseLibrary
	EXTERN_LIB Debug
	EXTERN_LIB FreeEntry
	EXTERN_LIB InitStruct
	EXTERN_LIB MakeLibrary
	EXTERN_LIB OpenDevice
	EXTERN_LIB TaggedOpenLibrary
	EXTERN_LIB OpenResource
	EXTERN_LIB PutMsg
	EXTERN_LIB RemDevice
	EXTERN_LIB RemTask
	EXTERN_LIB ReplyMsg
	EXTERN_LIB Signal
	EXTERN_LIB Wait
	EXTERN_LIB SetSignal

	XREF	TDCheckChange
	XREF	TDDelay
	XREF	TDDskBlk
	XREF	TDIO
	XREF	TDMfmSecEncode
	XREF	TDMotor
	XREF	TDSeek
	XREF	TDTaskStart
	XREF	TDTrkWrite
	XREF	TDUChangeNum
	XREF	TDUChangeState
	XREF	TDUClear
	XREF	TDUStop
	XREF	TDUStart
	XREF	TDUGetDriveType
	XREF	TDUMotor
	XREF	TDUGetNumTracks
	XREF	TDUGetGeometry
	XREF	TDURawRead
	XREF	TDURawWrite
	XREF	TDURemove
	XREF	TDUSeek
	XREF	TDUUpdate
	XREF	TDWriteBuffer
	XREF	TDUProtStatus
	XREF	TDUAddChangeInt
	XREF	TDURemChangeInt


	INT_ABLES
	TASK_ABLES

***** Exported Functions *********************************************

	XDEF	Init
	XDEF	TermIO


***** Local Definitions **********************************************

*	XDEF	TDGotReset
	XDEF	Expunge
	XDEF	Open
	XDEF	Close
	XDEF	BeginIO
	XDEF	PerformIO
	XDEF	Null
	XDEF	TDUFormat
	XDEF	TDSectorize
	XDEF	InitUnit
	XDEF	TDInitBuffer

***** Let the Code Begin *********************************************
SHORT_TABLE	EQU	1
BROKEN_RCI	EQU	1


	XDEF	SA_Driver
SA_Driver:



**********************************************************************
*
*
* Device entry points
*
*
**********************************************************************


*****i* trackdisk.device/internal/Initialize *************************
*
*   NAME
*	Initialize -- initialize the device from nothing
*
*   SYNOPSIS
*	Error = Initialize(DeviceName), SysLib
*	D0		   A1		A6
*
*   FUNCTION
*	This routine will initialize the device after it is closed.
*	The routine is optional; it is intended to make restoring
*	devices easier if they need to be purged "temporarily".
*
*	THIS ENTRY POINT MUST REMAIN VALID FOR ALL TIME (since it
*	is the only part of the device entry points that others are
*	allowed to cache).  It makes sense mostly for ROM based
*	drivers.  It MAY NOT be part of cartridge ROM (since cartridges
*	may be pulled).
*
*
*
*   INPUTS
*	DeviceName -- the name of the device to be loaded.  It is
*	    explicitly given because the init vector may point to
*	    a common routine, or to a search routine for disc libraries.
*
*
*   RESULTS
*	Error -- if the Init succedded, then Error will be null.  If
*	    it failed then it will be non-zero.
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
*	Remember that Init is run with SysLib in A6, unlike most of
*	the rest of the driver, which has the Device structure in A6
*
*

drName:		DC.B	'disk.resource',0
ciaBName:	DC.B	'ciab.resource',0
*gfxName		DC.B	'graphics.library',0
*timerName	DC.B	'timer.device',0
keyboardName	DC.B	'keyboard.device',0
		DS.W	0

*    ;------	temporary storage structure
    STRUCTURE INIT,0
	APTR	INIT_GRAPHICS
	STRUCT	INIT_TIMER0,IOTV_SIZE
	STRUCT	INIT_TIMER1,IOTV_SIZE
	STRUCT	INIT_KEYBOARD,IOSTD_SIZE
	APTR	INIT_DISCRESOURCE
	APTR	INIT_CIABRESOURCE
	LABEL	INIT_SIZE


MYALERT		MACRO	(alertNumber, [paramArray])
		movem.l d7/a5/a6,-(sp)
		move.l	\1,d7
		IFNC	'\2',''
		  lea.l	\2,a5
		ENDC
		move.l  4,a6
		jsr	_LVOAlert(a6)
		movem.l	(sp)+,d7/a5/a6
		ENDM

myalert:
		MYALERT	D0,,A0
		rts

Init:
		MOVE.L	A2,-(SP)
		LINK	A4,#-INIT_SIZE
		MOVE.L	SP,A2

		PUTMSG	50,<'%s/Init: called'>

*		;------ open all the external dependencies before getting
*		;------ any memory

*		;------ open the graphics library
		moveq	#OLTAG_GRAPHICS,d0
		CALLSYS TaggedOpenLibrary
		MOVE.L	D0,INIT_GRAPHICS(A2)
		BNE.S	Init_GfxOK

		IFGE	INFO_LEVEL-1
*****		PEA	gfxName
*****		PUTMSG	1,<'%s/Init: can not open %s'>
		move.l	#AN_TrackDiskDev!AG_OpenLib!AO_GraphicsLib,d0
		bsr.s	myalert
		ENDC

		BRA	InitErr_NoGfx

Init_GfxOK:

		LEA	drName(pc),A1
		CALLSYS	OpenResource
		MOVE.L	D0,INIT_DISCRESOURCE(A2)
		BNE.S	Init_DiscResourceOK

		IFGE	INFO_LEVEL-1
*****		PEA	drName
*****		PUTMSG	1,<'%s/Init: Can not open %s'>
		move.l	#AN_TrackDiskDev!AG_OpenRes!AO_DiskRsrc,d0
		bsr	myalert
		ENDC

		BRA	InitErr_NoDiscResource

Init_DiscResourceOK:
		LEA	ciaBName(pc),A1
		CALLSYS	OpenResource
		MOVE.L	D0,INIT_CIABRESOURCE(A2)
		BNE.S	Init_CIABResourceOK

		IFGE	INFO_LEVEL-1
*****		PEA	ciaBName
*****		PUTMSG	1,<'%s/Init: Can not open %s'>
		move.l	#AN_TrackDiskDev!AG_OpenRes!AO_CIARsrc,d0
		bsr	myalert
		ENDC

		BRA.s	InitErr_NoCIABResource

Init_CIABResourceOK:

		LEA	keyboardName(pc),A0
		LEA	INIT_KEYBOARD(A2),A1
		CLEAR	D0
		CLEAR	D1
		CALLSYS	OpenDevice
		TST.L	D0
		BEQ.S	Init_KeyboardOK

		IFGE	INFO_LEVEL-1
*****		PEA	keyboardName
*****		PUTMSG	1,<'%s/Init: Can not open %s'>
		move.l	#AN_TrackDiskDev!AG_OpenDev!AO_KeyboardDev,d0
		bsr	myalert
		ENDC

		BRA.S	InitErr_NoKeyboard

Init_KeyboardOK:

		LEA	INIT_TIMER0(A2),A1
		MOVEQ	#UNIT_MICROHZ,D0
		BSR	Init_OpenTimer
		BNE.S	InitErr_NoTimer0

		LEA	INIT_TIMER1(A2),A1
		MOVEQ	#UNIT_VBLANK,D0
		BSR	Init_OpenTimer
		BNE.S	InitErr_NoTimer1

*		;----- call the library initialization routine
		LEA	devFuncInit(pc),A0
		LEA	devStructInit(pc),A1
		SUB.L	A2,A2
		MOVE.L	#TD_SIZE,D0
		CALLSYS MakeLibrary

		TST.L	D0
		BNE.S	Init_Success

		move.l	#AN_TrackDiskDev!AG_MakeLib,d0
		bsr	myalert

*		;------ This is the last thing that could have gone wrong.
*		;------ start cleaning up in the reverse order that we
*		;------ got here

		LEA	INIT_TIMER1(A2),A1
		CALLSYS	CloseDevice

InitErr_NoTimer1:
		LEA	INIT_TIMER0(A2),A1
		CALLSYS	CloseDevice

InitErr_NoTimer0:

		LEA	INIT_KEYBOARD(A2),A1
		CALLSYS	CloseDevice
InitErr_NoKeyboard:
		;------ don't need to clean up a resource

InitErr_NoCIABResource:
InitErr_NoDiscResource:
		MOVE.L	INIT_GRAPHICS(A2),A1
		CALLSYS	CloseLibrary

InitErr_NoGfx:
		MOVEQ	#-1,D0
		BRA	Init_End


Init_Success:
		LEA	-INIT_SIZE(A4),A0
		MOVE.L	D0,A2

		IFGE	INFO_LEVEL-50
		MOVE.L	A2,-(SP)
		PUTMSG	50,<'%s/Init: device is at %lx'>
		ADDQ.L	#4,SP
		ENDC

*		;------ Initialize the devive structures
		MOVE.L	INIT_GRAPHICS(A0),TD_GFXLIB(A2)
		MOVE.L	INIT_DISCRESOURCE(A0),TD_DISCRESOURCE(A2)
		MOVE.L	INIT_CIABRESOURCE(A0),TD_CIABRESOURCE(A2)
		MOVE.L	A6,TD_SYSLIB(A2)

*		;------ copy in the timer stuff
		MOVE.L	INIT_TIMER0+IO_DEVICE-INIT_SIZE(A4),TD_TIMER0_DEV(A2)
		MOVE.L	INIT_TIMER0+IO_UNIT-INIT_SIZE(A4),TD_TIMER0_UNIT(A2)
		MOVE.L	INIT_TIMER1+IO_DEVICE-INIT_SIZE(A4),TD_TIMER1_DEV(A2)
		MOVE.L	INIT_TIMER1+IO_UNIT-INIT_SIZE(A4),TD_TIMER1_UNIT(A2)

		;------ copy in the keyboard stuff
		LEA	TD_RESETIOB(A2),A1
		MOVE.L	INIT_KEYBOARD+IO_UNIT-INIT_SIZE(A4),IO_UNIT(A1)
		MOVE.L	INIT_KEYBOARD+IO_DEVICE-INIT_SIZE(A4),IO_DEVICE(A1)

		;------ set up the reset software interrupt
		MOVE.L	#TDGotReset,TD_RESETINT+IS_CODE(A2)
		MOVE.L	A2,TD_RESETINT+IS_DATA(A2)
		LEA	TD_RESETINT(A2),A0
		MOVE.L	A0,IO_DATA(A1)

		;------ tell the keyboard device about it
		;---!!! we don't have a reply port!!!
		MOVE.B	#IOF_QUICK,IO_FLAGS(A1)
		MOVE.W	#KBD_ADDRESETHANDLER,IO_COMMAND(A1)
		BEGINIO

		;------ set the io block up for next time
		MOVE.W	#KBD_RESETHANDLERDONE,IO_COMMAND(A1)
		MOVE.B	#IOF_QUICK,IO_FLAGS(A1)

*		;------ Add the device
		MOVE.L	A2,A1
		CALLSYS AddDevice

*****		LEA	tdIDString,A0
*****		PUTFMT

		CLEAR	D0

Init_End:
		UNLK	A4
		MOVE.L	(SP)+,A2
		RTS

*    Init_OpenTimer
*    --------------
*
*	A0 --> timer name
*	A1 --> io block
*	D0 --> unit number
*
*	return with condition codes "equal" for success

Init_OpenTimer:
*		;------ save the unit number
*****		IFGE	INFO_LEVEL-1
*****		MOVE.L	D0,-(SP)
*****		ENDC

		;-- takes advantage of what regs params are in already
*		LEA	timerName(pc),A0
		sub.l	a0,a0		; ODTAG_TIMER == 0
		CLEAR	D1
		CALLSYS	OpenDevice
		TST.L	D0
		BEQ.S	Init_OpenTimerEnd

*****		IFGE	INFO_LEVEL-1
*****		PEA	timerName
*****		PUTMSG	1,<'%s/Init: can not open %s unit %ld'>
		move.l	#AN_TrackDiskDev!AG_OpenDev!AO_TimerDev,d0
		bsr	myalert
*****		ADDQ.L	#4,SP
*****		ENDC

*		;------ make sure the condition codes are "not equal"
		MOVEQ	#1,D0

Init_OpenTimerEnd:

*		;------ get rid of the saved unit number
*****		IFGE	INFO_LEVEL-1
*****		ADDQ.L	#4,SP
*****		ENDC

		RTS


UnitEntry:
		DS.B	LN_SIZE
		DC.W	4
		DC.L	MEMF_CLEAR!MEMF_PUBLIC
		DC.L	TDU_SIZE
		DC.L	MEMF_CLEAR
		DC.L	TD_STKSIZE
		DC.L	MEMF_PUBLIC!MEMF_CHIP	; used to be MEMF_CLEAR also
		DC.L	TDB_SIZE
		DC.L	MEMF_PUBLIC!MEMF_CHIP
		DC.L	TD_SECTOR

UnitEntry2M:
		DS.B	LN_SIZE
		DC.W	4
		DC.L	MEMF_CLEAR!MEMF_PUBLIC
		DC.L	TDU_SIZE
		DC.L	MEMF_CLEAR
		DC.L	TD_STKSIZE
		DC.L	MEMF_PUBLIC!MEMF_CHIP	; used to be MEMF_CLEAR also
		DC.L	TDB_LARGE_SIZE
		DC.L	MEMF_PUBLIC!MEMF_CHIP
		DC.L	TD_SECTOR

MEOFF_UNIT	EQU	ML_ME
MEOFF_STACK	EQU	ML_ME+ME_SIZE
MEOFF_TRACK	EQU	ML_ME+(ME_SIZE*2)	; also used in changed.asm!
MEOFF_CHIPBUF	EQU	ML_ME+(ME_SIZE*3)


*    InitUnit:
*    --------
*
*	A6 --> device pointer
*	D2 --> unit number
*	D3 --> flags
*
		
InitUnit:
		MOVEM.L	D4/A2-A5,-(SP)

		;------ default allocation spec
		LEA	UnitEntry(pc),a2
		LEA	drive3_5StructInit(pc),a5

		;------ see if this unit is of the right type
		MOVE.L	D2,D0
		LINKLIB	DR_GETUNITID,TD_DISCRESOURCE(A6)
		CMP.L	#DRT_AMIGA,D0
		BEQ.S	InitUnit_OKType

		CMP.L	#DRT_150RPM,d0
		BEQ.S	InitUnit_OKType2M

		CMP.L	#DRT_37422D2S,D0	; 5.25"
		BNE.S	InitUnit_BadType

		;------ unit allocations same as 3.5 1M
		LEA	drive5_25StructInit(pc),a5

		BTST	#TDB_ALLOW_NON_3_5,D3
		BNE.S	InitUnit_OKType

InitUnit_BadType:
		MOVEQ	#TDERR_BadDriveType,D0
		BRA	InitUnit_End

InitUnit_OKType2M:
		LEA	UnitEntry2M(pc),a2		; bigger buffer
		LEA	drive3_5_2M_StructInit(pc),A5

InitUnit_OKType:
		MOVE.L	D0,D4		; save drive type

		MOVE.L	D2,D0
		LINKLIB	DR_ALLOCUNIT,TD_DISCRESOURCE(A6)
		TST.L	D0
		BNE.S	InitUnit_GotUnit

		MOVEQ	#TDERR_DriveInUse,D0
		BRA	InitUnit_End

InitUnit_GotUnit:
*		;------ get unit memory
		MOVE.L	a2,a0		; UnitEntry or UnitEntry2M
		LINKSYS	AllocEntry
		TST.L	D0
		BMI	InitUnit_NoMem

*		;------ extract the info from the MemEntry
		MOVE.L	D0,A4
		MOVE.L	MEOFF_UNIT(A4),A3

*		;------ initialize the unit memory
		LEA	unitStructInit(pc),A1
		MOVE.L	A3,A2
		MOVE.W	#TDU_SIZE,D0
		LINKSYS	InitStruct

*		;------ store the mem entry
		MOVE.L	A4,TDU_ENTRY(A3)

*		;------ store the unit constants
		MOVE.B	D2,TDU_UNITNUM(A3)
		MOVE.B	#CIAF_DSKSEL0,D0
		ASL.B	D2,D0
		NOT.B	D0
		MOVE.B	D0,TDU_6522(A3)

		;------ set the drive specific stuff
		MOVE.L	D4,TDU_DISKTYPE(A3)

		MOVE.L	a5,a0			; table ptr, set above
		LEA	TDU_COMP01TRACK(A3),A1
		MOVEQ	#driveStructInit_Size,D0
		BSR	bcopy

	IFGE	INFO_LEVEL-80
		PEA	0
		MOVE.B	TDU_6522(A3),3(SP)
		MOVE.L	D2,-(SP)
		INFOMSG	80,<'%s/InitUnit: 6526 %ld 0x%02lx'>
		ADDQ.L	#8,SP
	ENDC

*		;------ Initialize the stack information
		MOVE.L	MEOFF_STACK(A4),A0
		MOVE.L	A0,TDU_TCB+TC_SPLOWER(A3)
		LEA	TD_STKSIZE(A0),A0
		MOVE.L	A0,TDU_TCB+TC_SPUPPER(A3)

		MOVE.L	TD_SYSLIB(a6),a1
		MOVE.L	ThisTask(a1),-(a0)	; pass current task as arg
		MOVE.L	A6,-(A0)		; argument -- lib ptr
		MOVE.L	A3,-(A0)
		MOVE.L	A0,TDU_TCB+TC_SPREG(A3)

*		;------ Link in the chip sector encode buffer
		MOVE.L	MEOFF_CHIPBUF(A4),A1
		MOVE.L	A1,TDU_CHIPBUF(A3)

*		;------ Link in the Track buffer
		MOVE.L	MEOFF_TRACK(A4),A1
		MOVE.L	A1,TDU_BUFPTR(A3)
		BSR	TDInitBuffer

*		;------ initialize the unit(s) timers
		LEA	TDU_WAITPORT(A3),A0
		MOVE.L	A0,TDU_WAITTIMER+MN_REPLYPORT(A3)
		MOVE.L	A0,TDU_DRU+MN_REPLYPORT(A3)
		MOVE.L	A3,TDU_DRU+DRU_DISCBLOCK+IS_DATA(A3)
		MOVE.L	A3,TDU_CHANGETIMER+MN_REPLYPORT(A3)
		LEA	TDU_TCB(A3),A0
		MOVE.L	A0,TDU_WAITPORT+MP_SIGTASK(A3)

*		;------ copy in the timer stuff
		MOVE.L	TD_TIMER0_DEV(A6),TDU_WAITTIMER+IO_DEVICE(A3)
		MOVE.L	TD_TIMER0_UNIT(A6),TDU_WAITTIMER+IO_UNIT(A3)
		MOVE.L	TD_TIMER1_DEV(A6),TDU_CHANGETIMER+IO_DEVICE(A3)
		MOVE.L	TD_TIMER1_UNIT(A6),TDU_CHANGETIMER+IO_UNIT(A3)

		;------ initialize the unit's lists
		LEA	TDU_CHANGELIST(A3),A0
		BSR	NewList			; smaller than macro REJ
		LEA	MP_MSGLIST(A3),A0
		BSR	NewList
		LEA	TDU_WAITPORT+MP_MSGLIST(A3),A0
		BSR	NewList
		LEA	TDU_TCB+TC_MEMENTRY(A3),A0
		BSR	NewList
		MOVE.L	TDU_ENTRY(A3),A1
		ADDHEAD

*		;------ now that the timers are open, add the device
		MOVE.L	A3,A4			; save unit pointer
		LEA	TDU_TCB(A3),A1
		LEA	TDTaskStart(PC),A2
		LEA	-1,A3			; generate address error
		CLEAR	D0
		LINKSYS AddTask

		CLEAR	D0
		MOVE.L	A4,A0
InitUnit_End:
		MOVEM.L	(SP)+,D4/A2-A5

		RTS

InitUnit_NoMem:
		IFGE	INFO_LEVEL-30
		PUTMSG	30,<'%s/InitUnit: Can not allocate memory'>
		ENDC

		MOVE.L	D2,D0
		LINKLIB	DR_FREEUNIT,TD_DISCRESOURCE(A6)

		MOVEQ	#TDERR_NoMem,D0
		BRA.S	InitUnit_End

*****i* trackdisk.device/TDInitBuffer *************************
*
*   NAME
*	TDInitBuffer - do buffer initialization
*
*   SYNOPSIS
*	TDInitBuffer(), Unit
*			 A3
*
*   FUNCTION
*	Initializes the track buffer for use.  It finds it via TDU_BUFPTR.
*
**********************************************************************
*
TDInitBuffer:
		MOVE.L	TDU_BUFPTR(A3),a1
		MOVE.W	#-1,TDB_TRACK(A1)
		CLR.B	TDB_FLAGS(A1)

		;------ TDB_DATA is set according to
		;------ density of disk in the drive.  This may be reset by
		;------ changed.asm.
		MOVE.W	TDU_MFM_SLOP(a3),d0
		LEA	TDB_BUF(a1,d0.w),a0
		MOVE.L	a0,TDB_DATA(a1)
		
*		;------ Zero out the track disc buffer
*		;------ Actually only the gap area (slop) (set above)
		LEA	TDB_BUF(A1),A0
		LSR.W	#2,D0			; was MAXTRACK (unsigned!)
		SUBQ.W	#1,d0			; DBRA loop!
		MOVE.L	#$AAAAAAAA,D1
InitUnit_Clearloop:
		MOVE.L	D1,(A0)+
		DBRA	D0,InitUnit_Clearloop
		RTS

;
; called from a software int when we got a reset
;
;
TDGotReset:
		MOVE.L	A2,-(SP)
		MOVE.L	A1,A2

		PUTMSG	40,<'%s/TDGotReset: called'>
		BSET	#TDB_RESET,TD_FLAGS(A1)

		;------ see if we are writing
		BTST	#TDB_WRITING,TD_FLAGS(A1)
		BNE.S	GotReset_End

		;------ we are done -- go away with a smile
		;---!!! no reply port -- we depend on quickness
		PUTMSG	40,<'%s/TDGotReset: not writing'>
		LEA	TD_RESETIOB(A2),A1
		BEGINIO

GotReset_End:
		MOVE.L	(SP)+,A2
		RTS


*****i* trackdisk.device/internal/Expunge *****************************
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
* We never expunge, so this code is deadwood.
*

Expunge:
		INFOMSG	50,<'%s/Expunge: called'>

*		DISABLE	A0
*		TST.W	LIB_OPENCNT(A6)
*		BNE	Exp_End
*
*
*Exp_Really:
*
*
*
*		;!!! do something...
*
*
*Exp_End:
*
*		ENABLE	A0

		CLEAR	D0
		RTS




*****i* trackdisk.device/internal/Open *******************************
*
*   NAME
*	Open -- a request to open the device
*
*   SYNOPSIS
*	Open( iORequest, UnitNumber, Flags ), DevNode
*	      A1	 D0	     D1	      A6
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
		MOVEM.L	D2/d3/A2/A4,-(SP)

		INFOMSG	50,<'%s/Open: called'>

		MOVE.L	A1,A4
		MOVE.L	D0,D2		; unit,  argument for InitUnit
		MOVE.l	D1,D3		; flags, argument for initunit

		CMP.L	#NUMUNITS,D0
		BLO.S	Open_OKUnit

*		;------ Not unit 0, 1, 2, or 3 so must be an error
		MOVEQ	#TDERR_BadUnitNum,D0
		BRA.S	Open_Err

Open_OKUnit:
*		;------ Initialize the unit ptr
		LSL.W	#2,D0		; unit is 0-3
		LEA	TDU0(A6),A2
		ADD.W	D0,A2

		;-- did unit exist already?
		MOVE.L	(A2),A0
		MOVE.L	A0,D0
		BNE.S	Open_GotUnit

		;-- no, create it.

		; - careful, d1 has flags for open

		;-- clear sigf_single just in case
		moveq	#0,d0			; new signals
		moveq	#SIGF_SINGLE,D1		; mask
		MOVE.L	A6,-(a7)		; save device base
		MOVE.L	TD_SYSLIB(A6),A6
		JSR	_LVOSetSignal(A6)
		MOVE.L	(a7)+,A6		; restore device base

		BSR	InitUnit	; d2/d3 arguments, scratch afterwards
		TST.L	D0
		BNE.S	Open_Err

*		;------ store the unit pointer
		MOVE.L	A0,(A2)

*		;------ wait for new task to signal us it's ready!
		MOVEQ	#SIGF_SINGLE,D0
		MOVE.L	A6,D2			; save device base
		MOVE.L	TD_SYSLIB(A6),A6
		JSR	_LVOWait(A6)
		MOVE.L	D2,A6			; restore device base

Open_GotUnit:

		MOVE.L	(A2),a0			; wait may have killed it
		MOVE.L	A0,IO_UNIT(A4)

		ADDQ.W	#1,LIB_OPENCNT(A6)
		ADDQ.W	#1,UNIT_OPENCNT(A0)

Open_End:
		MOVEM.L	(SP)+,D2/D3/A2/A4
		RTS

Open_Err:
		MOVE.B	D0,IO_ERROR(A4)
		MOVEQ	#-1,D0
		MOVE.L	D0,IO_UNIT(A4)
		MOVE.L	D0,IO_DEVICE(A4)
		BRA.S	Open_End

*****i* trackdisk.device/internal/Close ****************************
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
*		;-- could do unit specific close stuff here, if needed
		MOVE.L	IO_UNIT(A1),A0

		IFGE	INFO_LEVEL-50
		PEA	0
		MOVE.B	TDU_UNITNUM(A0),3(SP)
		INFOMSG	50,<'%s/Close: called for unit %ld'>
		ADDQ.L	#4,SP
		ENDC

		SUBQ.W	#1,UNIT_OPENCNT(A0)
		BNE.S	Close_Dev

*		;------ the next command or the clock will do the cleanup
		BSET	#TDUB_DELMOTOROFF,TDU_FLAGS(A0)
Close_Dev:

		SUBQ.W	#1,LIB_OPENCNT(A6)

*		;-- clear out the pointers
		MOVEQ	#-1,D0
		MOVE.L	D0,IO_UNIT(A1)
		MOVE.L	D0,IO_DEVICE(A1)

		INFOMSG	50,<'%s/Close: returning'>

		CLEAR	D0

		RTS


*****i* trackdisk.device/internal/BeginIO  **************************
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
* RESET, STOP, START, FLUSH, CHANGENUM, CHANGESTATE, PROTSTATUS, GETDRIVETYPE
*****IMMEDIATES	EQU	$4e1c2

* RESET, STOP, START, FLUSH, CHANGENUM, CHANGESTATE, GETDRIVETYPE, GETNUMTRACKS
*IMMEDIATES	EQU	$c61c2

* RESET, START, FLUSH, CHANGENUM, CHANGESTATE, GETDRIVETYPE, GETNUMTRACKS,
* GETGEOMETRY
*IMMEDIATES	EQU	$1c6182

* GetGeometry shouldn't be immediate, since the variables may change!
* RESET, START, FLUSH, CHANGENUM, CHANGESTATE, GETDRIVETYPE, GETNUMTRACKS,
IMMEDIATES	EQU	$c6182

BeginIO:
		IFGE	INFO_LEVEL-50
		PEA	0
		MOVE.W	IO_COMMAND(A1),2(SP)
		MOVE.L	A1,-(SP)
		INFOMSG 50,<'%s/BeginIO: called with IOR 0x%lx, com %lx'>
		ADDQ.L	#8,SP
		ENDC

		CLR.B	IO_ERROR(A1)

		;------ check for a legal command
		CLEAR	D0
		MOVE.W	IO_COMMAND(A1),D0
		BCLR.L	#TDB_EXTCOM,D0		; mask off extended bit
		CMP.W	#TD_LASTCOMM,D0
		BCC	NoIO			; Bcc to BRA - exits beginio!

		MOVE.L	IO_UNIT(A1),A0

	IFD	BROKEN_RCI
		;------ kludge to make TDRemChangeInt work - daren't queue!
		;------ see changed.asm for TDRemChangeInt description
* need to kludge CMD_START to never queue either!!!!
* FIX!
		CMP.W	#TD_REMCHANGEINT,D0
		BEQ.S	BeginIO_Immediate
	ENDC
		;------ see if the driver allows quicks for this one
		MOVE.L	#IMMEDIATES,D1
		BTST	D0,D1
		BEQ.S	BeginIO_Queued

		; did he set the quick bit?
		BTST.B	#IOB_QUICK,IO_FLAGS(a1)
		BNE.S	BeginIO_Immediate

		;------ just a regular, synchronous command here...
BeginIO_Queued:
		AND.B	#(~(IOF_QUICK!IOF_IMMEDIATE))&$ff,IO_FLAGS(A1)
		LINKSYS PutMsg
		BRA.S	BeginIO_End

BeginIO_Immediate:
		BSET.B	#IOB_IMMEDIATE,IO_FLAGS(A1)
		MOVE.B	#NT_MESSAGE,LN_TYPE(A1)

*		;------ This is an immediate command.  Bypass PerformIO()
		MOVEM.L	A2/A3,-(SP)
		MOVE.L	A0,A3
		MOVE.L	A1,A2	; I think some assume it's in A1 still - REJ

		;------ the command is still in D0
		LEA	cmdTable(PC),A0
		LSL.W	#1,D0
	IFNE	SHORT_TABLE
		ADD.W	0(A0,D0.W),A0		; table of 16-bit offsets REJ
	ENDC
	IFEQ	SHORT_TABLE
		MOVE.L	0(A0,D0.W),A0
	ENDC
		JSR	(A0)

	IFD	BROKEN_RCI
		;-- check if we should RTS (QUICK) or reply (TD_REMCHANGEINT).
		;-- we need to do this in case they didn't specify QUICK
		;-- when calling remchangeint.
		MOVE.L	A2,A1
	ENDC
		MOVEM.L	(SP)+,A2/A3

	IFD	BROKEN_RCI
		BTST.B	#IOB_QUICK,IO_FLAGS(A1)
		BNE.S	BeginIO_End
		LINKSYS	ReplyMsg
	ENDC
BeginIO_End:
		INFOMSG 60,<'%s/BeginIO: exiting'>
		RTS

*BeginIO_Err:
*		BRA	NoIO			; was BSR
**		BRA.S	BeginIO_End


*****i* trackdisk.device/internal/PerformIO *************************
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

*		;------ clear some state bits
		AND.B	#-(TDUF_EXTENDED!TDUF_SECLABEL)-1,TDU_FLAGS(A3)

*		;------ update the state of the disc
		BSR	TDCheckChange

*		;------ see if this is an extended command
		MOVE.L	A2,A1
		MOVE.W	IO_COMMAND(A2),D0
		BTST.L	#TDB_EXTCOM,D0
		BEQ.S	Perform_Call

		BSET.B	#TDUB_EXTENDED,TDU_FLAGS(A3)

*		;------ it is extended.  see if the count is right
		MOVE.L	TDU_COUNTER(A3),D1
		CMP.L	IOTD_COUNT(A2),D1
		BLS.S	Perform_Call

*		;------ this command is stale.  send it back.
		MOVE.B	#TDERR_DiskChanged,IO_ERROR(A2)
		BSR	TermIO
		BRA.S	Perform_End

Perform_Call:
*		;------ we only look at the low byte of the command
		CLEAR	D1
		MOVE.B	D0,D1
		LSL.W	#1,D1
		LEA	cmdTable(PC),A0
	IFNE	SHORT_TABLE
		ADD.W	0(A0,D1.W),A0		; table of 16-bit offsets REJ
	ENDC
	IFEQ	SHORT_TABLE
		MOVE.L	0(A0,D1.W),A0
	ENDC
		JSR	(A0)

		BSR	TDCheckChange

Perform_End:
		IFGE	INFO_LEVEL-80
		PEA	0
		MOVE.W	IO_COMMAND(A2),2(SP)
		INFOMSG 80,<'%s/PerformIO: exiting (command 0x%lx)'>
		ADDQ.L	#4,SP
		ENDC

	IFGE	INFO_LEVEL-100
	CLEAR	d0
	move.b	IO_ERROR(a2),d0
	beq.s	perform_posterrcheck

	move.l	a2,-(sp)
	move.l	d0,-(sp)

	INFOMSG	100,<'%s/PerformIO: error %ld on iob 0x%lx'>
	LINKSYS	Debug
	addq.l	#8,SP

perform_posterrcheck:
	ENDC

		MOVE.L	(SP)+,A2
		RTS


*****i* trackdisk.device/internal/Null *********************************
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
		MOVE.B	#IOERR_NOCMD,IO_ERROR(A1)
		BRA	TermIO			; was BSR
*		RTS


*****i* trackdisk.device/internal/TDUFormat **************************
*
*   NAME
*	TDUFormat -- format the entire disc
*
*   SYNOPSIS
*	TDUFormat( iOBlock ), DevNode
*	D0	   A1	      A6
*
*   FUNCTION
*
*	The function formats the entire disc, destroying all data.
*	It fills all the sectors with the contents of the iOBlock.
*	The iOBlock must point to one track of data.
*
*   INPUTS
*
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



TDUFormat:


		MOVEM.L D2/D3/D4/D5/A2/A3/A4/A5,-(SP)

*		;-- D2 => track #
*		;-- D3 => number of tracks
*		;-- D4 => buffers from end
*		;-- D5 => sector number
*		;-- A3 => ptr unit
*		;-- A2 => track buffer
*		;-- A4 => ioblock
*		;-- A5 => data pointer

		INFOMSG 50,<'%s/TDUFormat: called'>

		MOVE.L	A1,A4
		MOVE.L	IO_UNIT(A4),A3

		MOVEQ	#1,D0
		BSR	TDMotor

		;-- This is here for extra safety in making sure
		;-- the motor is fully up to speed.
		TST.L	D0
		BNE.S	dw_flushbuf

		INFOMSG	30,<'%s/Format: motor on'>
		MOVE.L	#TDT_FORMAT,D0		; no apparent reason
		BSR	TDDelay			; appears to be for random
						; safety
dw_flushbuf:
*		;------ Write out the buffer if necessary
		MOVE.L	TDU_BUFPTR(A3),A0
		MOVE.L	A0,TDU_BUFFER(A3)

		;-- never set for an invalid buffer
		BCLR	#TDBB_DIRTY,TDB_FLAGS(A0)
		BEQ.S	dw_cleanbuf

		BSR	TDWriteBuffer		; write out dirty buffer
		TST.L	D0
		BEQ.S	dw_cleanbuf

		MOVE.B	D0,IO_ERROR(A4)
		BRA	Format_Term

dw_cleanbuf:

		;------ verify the arguments
		MOVE.L	IO_OFFSET(A4),D0
		BSR	TDSectorize
		MOVE.L	D0,D2
		TST.L	D1
		BNE	Format_Err

		MOVE.L	IO_LENGTH(A4),D0
		BSR	TDSectorize
		TST.L	D1
		BNE	Format_Err

		;------ check for null length format request
		MOVE.L	D0,D3
		BEQ	Format_Term

		;------ make sure we do not fall off the end of the disk
		ADD.L	D2,D0
		CMP.W	TDU_NUMTRACKS(A3),D0
		BHI	Format_Err

		;------ OK, the args are fine.  Lets fill some tracks!
		MOVE.L	IO_DATA(A4),A5		; data pointer

dw_trackloop:
		;------ get to the right neighborhood
		MOVE.L	D2,D0
		BSR	TDSeek

		;------ get the beginning of the data
		MOVE.L	TDU_BUFPTR(A3),A2
		LEA	TDB_BUF(A2),A2

*		;------ fill track with MFM 0's
		MOVE.W	TDU_MFM_TRKBUF(a3),D0
		LSR.W	#2,D0			; longwords
		SUBQ.W	#1,D0			; DBRA loop!
		MOVE.L	#$0AAAAAAAA,D1
dw_fillnull:
		MOVE.L	D1,(A2)+
		DBF	D0,dw_fillnull

*		;------ point A2 to beginning of sector data
		MOVE.L	TDU_BUFPTR(A3),A2
		MOVE.L	TDB_DATA(A2),A2

		;------ loop pointer
		MOVE.W	TDU_NUMSECS(a3),D4
		CLEAR	D5
dw_sectorloop:

*		;------ make the "track ID longword"
		MOVE.L	#$0FF000000,-(SP)	; the id flag

		MOVE.B	D5,FMT_SECTOR(SP)	; the sector #

		MOVE.B	D2,FMT_TRACK(SP)	; the track number
		MOVE.L	(SP)+,D0

		MOVE.B	D4,D0			; the # of secs till the gap

		;------ set the address pointers
		MOVE.L	A2,A1			; buffer ptr
		MOVE.L	A5,A0			; data ptr

		BSR	TDMfmSecEncode		; d0,a0,a1

		;------ now advance all the pointers
		ADDQ.L	#1,D5			; bump sector number

		ADD.L	#MFM_RAWSECTOR,A2	; bump buffer pointer
		ADD.L	#TD_SECTOR,A5		; bump sector data pointer

		SUBQ.W	#1,D4			; sectors till gap
		BNE.S	dw_sectorloop

		;------ Make SURE buffer ends with $aaa8! (or $2aa8!)
		;------ The reason it is not AAAA is because paula
		;------ drops write gate three bits before the serial
		;------ shift out actually stops.  The software must 
		;------ ensure that the last three bits are nulls.
		MOVE.W	#$AAA8,D0
		BTST.B	#0,-1(A2)
		BEQ.S	1$
		;------ unset the clock bit
		BCLR	#15,D0
1$:		MOVE.W	d0,(a2)

		;------ we have encoded an entire track -- write it out
		MOVE.L	TDU_BUFPTR(A3),A0
		LEA	TDB_BUF(A0),A0
		MOVE.W	TDU_MFM_MAXTRACK(A3),D0
		ADDQ.W	#2,d0			; for $aaa8
		BSR	TDTrkWrite

		TST.L	D0
		BEQ.S	Format_WriteOK

		;------ we had an error on write
		MOVE.B	D0,IO_ERROR(A4)
		BRA.S	Format_Term

Format_WriteOK:

		ADDQ.L	#1,D2
		SUBQ.L	#1,D3
		BNE	dw_trackloop

		;------ Mark the buffer as invalid
		MOVE.L	TDU_BUFPTR(A3),A2
		MOVE.W	#-1,TDB_TRACK(A2)
		BCLR.B	#TDBB_DIRTY,TDB_FLAGS(A2)

Format_Term:
		;------ Mark the operation as done
		IFGE	INFO_LEVEL-60
		CLEAR	D2
		MOVE.B	IO_ERROR(A4),D2
		ENDC

		MOVE.L	A4,A1
		BSR.S	TermIO

		IFGE	INFO_LEVEL-60
		MOVE.L	D2,-(SP)
		INFOMSG 60,<'%s/TDUFormat: exiting.  error %ld'>
		ADDQ.L	#4,SP
		ENDC

		MOVEM.L (SP)+,D2/D3/D4/D5/A2/A3/A4/A5

		RTS

Format_Err:
	IFGE	INFO_LEVEL-90
	MOVEM.L	D2/D3,-(SP)
	INFOMSG	90,<'%s/Format: length err. track %ld for %ld'>
	ADDQ.L	#8,SP
	ENDC

		MOVE.B	#IOERR_BADLENGTH,IO_ERROR(A4)
		BRA.S	Format_Term



*****i* trackdisk.device/internal/TDSectorize *************************
*
*   NAME
*	TDSectorize -- divide a long byte offset into sectors and tracks
*
*   SYNOPSIS
*	track, sector = TDSectorize( offset ), devLib
*	D0     D1                    D0        A6
*
*   FUNCTION
*	TDSectorize turns a byte offset into tracks and sectors,
*	suitable for the rest of the system.  If the offset is
*	not legal (e.g. not an integral number of sectors) then
*	track and sector will be set to -1.  An error is also
*	returned if the offset is too large.
*
*   INPUTS
*	track -- the track offset
*	sector -- the sector offset
*
**********************************************************************
*

TDSectorize:

		;------ check operation for legality
		MOVE.L	D0,D1

		;------ check for being an integral sector boundary
		AND.L	#TD_SECTOR-1,D1
		BNE.S	Sectorize_Err

		;------ check for IO within disc range
		CMP.L	TDU_MAXOFFSET(A3),D0
		BHI.S	Sectorize_Err

		;------ convert to sector offset
		MOVEQ	#TD_SECSHIFT,D1
		LSR.L	D1,D0

		;------ divide by sector number
		DIVU	TDU_NUMSECS(A3),D0	; sector in high word, track
						; in low word

		;------ and put it in its final resting spot
		CLEAR	D1
		SWAP	D0
		MOVE.W	D0,D1		; set the sector return value
		MOVE.W	#0,D0		; clear out the sector portion
		SWAP	D0		; put track back where it should be
Sectorize_End:
		IFGE	INFO_LEVEL-40
		MOVEM.L	D0/D1,-(SP)
		INFOMSG	40,<'%s/Sectorize: track %ld sector %ld'>
		ADDQ.L	#8,SP
		ENDC

		RTS

Sectorize_Err:
		MOVEQ	#-1,D0
		MOVEQ	#-1,D1
		RTS


*****i* trackdisk.device/internal/TermIO *******************************
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
		MOVEM.L	D2/A3,-(SP)
*		MOVE.B	IO_FLAGS(a1),D2		; save flags for later use
		MOVE.L	IO_UNIT(A1),A3

*		;------ mark the I/O block as done
		BTST.B	#IOB_QUICK,IO_FLAGS(a1)
		BNE.S	Term_End

	IFGE	INFO_LEVEL-80
	MOVE.L	A1,-(SP)
	INFOMSG	80,<'%s/TermIO: IOB %lx'>
	ADDQ.L	#4,SP
	ENDC

		;------ and send it back
		LINKSYS ReplyMsg

		;------ see if we locked the device in order to do this
*		;------ can't touch message, it's gone!
*		BTST.B	#IOB_IMMEDIATE,D2
*		BNE.S	Term_End
*
*		;------ Signal the task
*		;---!!! Need to parameterize the signal number and TCB ptr
* FIX! I think this isn't needed! REJ
*
*		MOVE.L	#TDF_DEVDONE,D0
*		LEA	TDU_TCB(A3),A1
*		LINKSYS Signal


Term_End:
		MOVEM.L	(SP)+,D2/A3
		RTS


NewList:	; a0 = pointer to list structure
		NEWLIST A0
		RTS

bcopy:		; (a0 - source, a1 - dest, d0 - size:16)

		bra.s	bcopy_start

bcopy_loop:
		move.b	(a0)+,(a1)+
bcopy_start:
		dbra	d0,bcopy_loop

		rts

		


*----------------------------------------------------------------------
*
* Definitions for Device Library Initialization
*
*----------------------------------------------------------------------


		XDEF devStructInit
devStructInit:
*		;------ Initialize the device
		INITBYTE	LN_TYPE,NT_DEVICE
		INITLONG	LN_NAME,tdName
		INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD	LIB_VERSION,VERNUM
		INITWORD	LIB_REVISION,REVNUM
		DC.W		0

		XDEF unitStructInit
unitStructInit:

*		;------ Initialize the unit
		INITBYTE	LN_TYPE,NT_MSGPORT
		INITLONG	LN_NAME,tdName
		INITBYTE	MP_SIGBIT,TDB_PORT
		INITBYTE	MP_MSGLIST+LH_TYPE,NT_MESSAGE

		INITWORD	TDU_SEEKTRK,-1
		INITBYTE	TDU_FLAGS,TDUF_REMOVED

		INITBYTE	TDU_WAITPORT+LN_TYPE,NT_MSGPORT
		INITLONG	TDU_WAITPORT+LN_NAME,tdName
		INITBYTE	TDU_WAITPORT+MP_SIGBIT,TDSB_WAITTIMER
		INITBYTE	TDU_WAITPORT+MP_MSGLIST+LH_TYPE,NT_REPLYMSG

		INITLONG	TDU_WAITTIMER+LN_NAME,tdName
		INITWORD	TDU_WAITTIMER+IO_COMMAND,TR_ADDREQUEST

		INITLONG	TDU_CHANGETIMER+LN_NAME,tdName
		INITWORD	TDU_CHANGETIMER+IO_COMMAND,TR_ADDREQUEST
		INITLONG	TDU_CHANGETIMER+IOTV_TIME+TV_SECS,10

		INITBYTE	TDU_TCB+LN_TYPE,NT_TASK
		INITBYTE	TDU_TCB+LN_PRI,5
		INITLONG	TDU_TCB+LN_NAME,tdName

*		;-- Not needed (message, not iorreq - see disk resource) REJ
		INITLONG	TDU_DRU+LN_NAME,tdName
		INITLONG	TDU_DRU+DRU_DISCBLOCK+IS_CODE,TDDskBlk

		DC.W		0


TRK	EQU	11*TD_SECTOR	; quick way to say tracksize
	XDEF	drive3_5StructInit

		CNOP	0,2
drive3_5StructInit:
		DC.W		80	; UWORD	TDU_COMP01TRACK
		DC.W		$ffff	; UWORD	TDU_COMP10TRACK
		DC.W		$ffff	; UWORD	TDU_COMP11TRACK
		DC.L		TDT_STEP	; ULONG	TDU_STEPDELAY
		DC.L		TDT_SETTLE	; ULONG	TDU_SETTLEDELAY
		DC.B		NUM_RETRIES	; UBYTE TDU_RETRYCNT
		DC.B		0	; UBYTE TDU_PUBFLAGS
		DC.W		$ffff	; UWORD TDU_CURRTRK
		DC.L		TDT_CALIBRATE	; ULONG TDU_CALIBRATEDELAY
		DC.L		0		; ULONG TDU_COUNTER
		DC.L		TDT_POSTWRITE	; ULONG TDU_POSTWRITEDELAY
		DC.L		TDT_SIDESEL	; ULONG TDU_SIDESELECTDELAY
		DC.B		DRIVE3_5 ; UBYTE TDU_DRIVETYPE
		DC.B		0	; UBYTE TDU_RESERVED2
		DC.W		160	; UWORD	TDU_NUMTRACKS
		DC.L		160*TRK ; ULONG TDU_MAXOFFSET
		DC.W		MFM_TRKBUF	; UWORD TDU_MFM_TRKBUF
		DC.W		MFM_MAXTRACK	; UWORD TDU_MFM_MAXTRK
		DC.W		MFM_SLOP	; UWORD TDU_MFM_SLOP
		DC.W		11		; UWORD TDU_NUMSECS
		DC.L		TDT_DISKSYNC	; ULONG TDU_TDT_DISKSYNC
		DC.L		DRT_AMIGA	; ULONG TDU_DISKTYPE
		DC.B		0		; UBYTE TDU_FLAGS
drive3_5end:

driveStructInit_Size	EQU	drive3_5end-drive3_5StructInit


BIGTRK	EQU	MAX_NUMSECS*TD_SECTOR	; quick way to say tracksize
	XDEF	drive3_5_2M_StructInit

		CNOP	0,2

drive3_5_2M_StructInit:
		DC.W		80	; UWORD	TDU_COMP01TRACK
		DC.W		$ffff	; UWORD	TDU_COMP10TRACK
		DC.W		$ffff	; UWORD	TDU_COMP11TRACK
		DC.L		TDT_STEP	; ULONG	TDU_STEPDELAY
		DC.L		TDT_SETTLE	; ULONG	TDU_SETTLEDELAY
		DC.B		NUM_RETRIES	; UBYTE TDU_RETRYCNT
		DC.B		0	; UBYTE TDU_PUBFLAGS
		DC.W		$ffff	; UWORD TDU_CURRTRK
		DC.L		TDT_CALIBRATE	; ULONG TDU_CALIBRATEDELAY
		DC.L		0		; ULONG TDU_COUNTER
		DC.L		TDT_POSTWRITE	; ULONG TDU_POSTWRITEDELAY
		DC.L		TDT_SIDESEL	; ULONG TDU_SIDESELECTDELAY
 		DC.B		DRIVE3_5_150RPM ; UBYTE TDU_DRIVETYPE
		DC.B		0	; UBYTE TDU_RESERVED2
		DC.W		160	; UWORD	TDU_NUMTRACKS
		DC.L		160*BIGTRK	; ULONG TDU_MAXOFFSET
		DC.W		MFM_BIG_TRKBUF	; UWORD TDU_MFM_TRKBUF
		DC.W		MFM_BIG_MAXTRACK ; UWORD TDU_MFM_MAXTRK
		DC.W		MFM_BIG_SLOP	; UWORD TDU_MFM_SLOP
		DC.W		22		; UWORD TDU_NUMSECS
		DC.L		TDT_DISKSYNC*2	; ULONG TDU_TDT_DISKSYNC
		DC.L		DRT_150RPM	; ULONG TDU_DISKTYPE
		DC.B		TDUF_LARGE	; UBYTE TDU_FLAGS

		CNOP	0,2

	XDEF	drive5_25StructInit
drive5_25StructInit:
		DC.W		40	; UWORD	TDU_COMP01TRACK
		DC.W		$ffff	; UWORD	TDU_COMP10TRACK
		DC.W		$ffff	; UWORD	TDU_COMP11TRACK
		DC.L		TDT_STEP*2	; ULONG	TDU_STEPDELAY
		DC.L		TDT_SETTLE	; ULONG	TDU_SETTLEDELAY
		DC.B		NUM_RETRIES	; UBYTE TDU_RETRYCNT
		DC.B		0	; UBYTE TDU_PUBFLAGS
		DC.W		$ffff	; UWORD TDU_CURRTRK
		DC.L		TDT_CALIBRATE*2	; ULONG TDU_CALIBRATEDELAY
		DC.L		0		; ULONG TDU_COUNTER
		DC.L		TDT_POSTWRITE	; ULONG TDU_POSTWRITEDELAY
		DC.L		TDT_SIDESEL	; ULONG TDU_SIDESELECTDELAY
		DC.B		DRIVE5_25 ; UBYTE TDU_DRIVETYPE
		DC.B		0	; UBYTE TDU_RESERVED2
		DC.W		80	; UWORD	TDU_NUMTRACKS
		DC.L		80*TRK	; ULONG TDU_MAXOFFSET
		DC.W		MFM_TRKBUF	; UWORD TDU_MFM_TRKBUF
		DC.W		MFM_MAXTRACK	; UWORD TDU_MFM_MAXTRK
		DC.W		MFM_SLOP	; UWORD TDU_MFM_SLOP
		DC.W		11		; UWORD TDU_NUMSECS
		DC.L		TDT_DISKSYNC	; ULONG TDU_TDT_DISKSYNC
		DC.L		DRT_37422D2S	; ULONG TDU_DISKTYPE
		DC.B		0		; UBYTE TDU_FLAGS

		CNOP	0,2

		XDEF devFuncInit
devFuncInit:
		DC.W	-1
		DC.W	Open-devFuncInit		; - 4
		DC.W	Close-devFuncInit		; - 8
		DC.W	Expunge-devFuncInit		; -10
		DC.W	Null-devFuncInit		; -14	; reserved
		DC.W	BeginIO-devFuncInit		; -18
		DC.W	Null-devFuncInit		; -1c
		DC.W	-1

;Build table of relative vectors

MCCASM	    EQU     1

CMDDEF	    MACRO   * function
    IFEQ MCCASM
	    DC.W    \1-cmdTable
    ENDC
    IFNE MCCASM
	    DC.W    \1+(*-cmdTable)
    ENDC
	    ENDM

		;-- ICK!  externals must use CMDDEF, internals
		;-- must use DC.W!  Saves 44 bytes.
cmdTable:
		IFNE	SHORT_TABLE
		DC.W	NoIO-cmdTable	;  0 invalid
		DC.W	NoIO-cmdTable	;  1 reset
		CMDDEF	TDIO		;  2 read
		CMDDEF	TDIO		;  3 write
		CMDDEF	TDUUpdate	;  4 update
		CMDDEF	TDUClear	;  5 clear
		CMDDEF	TDUStop		;  6 stop
		CMDDEF	TDUStart	;  7 start
		DC.W	NoIO-cmdTable	;  8 flush
		CMDDEF	TDUMotor	;  9 motor
		CMDDEF	TDUSeek		;  a seek
		DC.W	TDUFormat-cmdTable ;  b format
		CMDDEF	TDURemove	;  c remove
		CMDDEF	TDUChangeNum	;  d changenum
		CMDDEF	TDUChangeState	;  e changestate
		CMDDEF	TDUProtStatus	;  f protstatus
		CMDDEF	TDURawRead	; 10 rawread
		CMDDEF	TDURawWrite	; 11 rawwrite
		CMDDEF	TDUGetDriveType	; 12 getdrivetype
		CMDDEF	TDUGetNumTracks	; 13 getnumtracks
		CMDDEF	TDUAddChangeInt	; 14 addchangeint
		CMDDEF	TDURemChangeInt	; 15 remchangeint
		CMDDEF	TDUGetGeometry	; 16 getgeometry
		DC.W	NoIO-cmdTable	; 17 eject

		ENDC

		IFEQ	SHORT_TABLE
		DC.L	NoIO		;  0 invalid
		DC.L	NoIO		;  1 reset
		DC.L	TDIO		;  2 read
		DC.L	TDIO		;  3 write
		DC.L	TDUUpdate	;  4 update
		DC.L	TDUClear	;  5 clear
		DC.L	TDUStop		;  6 stop
		DC.L	TDUStart	;  7 start
		DC.L	NoIO		;  8 flush
		DC.L	TDUMotor	;  9 motor
		DC.L	TDUSeek		;  a seek
		DC.L	TDUFormat	;  b format
		DC.L	TDURemove	;  c remove
		DC.L	TDUChangeNum	;  d changenum
		DC.L	TDUChangeState	;  e changestate
		DC.L	TDUProtStatus	;  f protstatus
		DC.L	TDURawRead	; 10 rawread
		DC.L	TDURawWrite	; 11 rawwrite
		DC.L	TDUGetDriveType	; 12 getdrivetype
		DC.L	TDUGetNumTracks	; 13 getnumtracks
		DC.L	TDUAddChangeInt	; 14 addchangeint
		DC.L	TDURemChangeInt	; 15 remchangeint
		DC.l	TDUGetGeometry	; 16 getgeometry
		DC.l	NoIO		; 17 eject
		ENDC



	END
