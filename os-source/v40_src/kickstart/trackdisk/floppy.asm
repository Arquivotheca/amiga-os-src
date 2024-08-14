
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* floppy.asm
*
* Source Control
* ------ -------
* 
* $Id: floppy.asm,v 33.18 92/04/05 19:48:15 jesup Exp $
*
* $Locker:  $
*
* $Log:	floppy.asm,v $
* Revision 33.18  92/04/05  19:48:15  jesup
* PostWrite and SideSel are now variable
* 
* Revision 33.17  91/03/13  20:40:34  jesup
* Removed unneeded selects, bsr->bsr.s
* 
* Revision 33.16  90/11/26  17:38:29  jesup
* Added some debugging code (ifdefed out)
* 
* Revision 33.14  90/06/01  23:15:20  jesup
* Conform to include standard du jour
* 
* Revision 33.13  90/03/16  00:56:37  jesup
* Added store of direction before seek, to make sure drive has a few micros
* to notice the direction, instead of driving them at the same time (at request
* of grr).
* 
* Revision 33.12  89/09/06  18:51:12  jesup
* Changed comments, changed _AbsExecBase to ABSEXECBASE
* 
* Revision 33.11  89/05/15  21:15:52  jesup
* fixed usage of Getunit in TDMotor and Calibrate, added comments
* fixed error return for timeout on read.
* fixed bug in signal clearing in TDStartDma
* 
* Revision 33.10  89/04/27  23:36:22  jesup
* fixed autodocs, BSR/RTS->BRA
* 
* Revision 33.9  89/04/12  14:37:19  jesup
* fix to TDMotor
* 
* Revision 33.8  89/03/22  20:18:22  jesup
* Fixed TDDelay for small delays on 68030 w/ data cache
* wait settledelay when reversing head direction on calibrate
* 
* Revision 33.7  89/03/22  17:33:25  jesup
* Made TDMotor release unit while waiting, small optimizations.
* 
* Revision 33.6  89/03/18  01:45:46  jesup
* Added xdef
* 
* Revision 33.5  89/02/17  19:07:37  jesup
* TDMotor now waits .1 secs up to 5 times, and check ready after each
* removed NOPs
* TrkRead now uses the WORDSYNC ability
* WaitDskBlk now uses signals instead of PutMsg, added routine to support
* sync reads, WaitDskTime.  Can now use timer and dma at the same time.
* minor code optimizations
* 
* Revision 33.4  86/04/09  16:07:25  neil
* 68020 changes for real
* 
* Revision 33.3  86/04/04  09:56:31  neil
* Some trial changes for 68020 slowdowns
* 
* Revision 33.2  86/03/31  01:42:29  neil
* removed a debugging statement in TDDelay.
* 
* Revision 33.1  86/03/29  14:13:36  neil
* made seek and settle time programmable.  Isolated unit specific
* initializers to the beginning of the unit structure
* 
* Revision 32.2  86/01/03  19:51:42  neil
* Added reset catching code
* 
* Revision 32.1  85/12/23  17:18:58  neil
* Added rawread/rawwrite
* 
* Revision 30.1  85/08/29  12:29:31  neil
* fixed bug in calibrate -- track 0 was not detected properly
* 
* Revision 27.2  85/07/11  06:23:05  neil
* Made seek report an error
* 
* Revision 27.1  85/06/24  13:36:48  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  15:13:08  neil
* *** empty log message ***
* 
* 
*************************************************************************

	SECTION section

;****** Included Files ***********************************************

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
	INCLUDE 'exec/alerts.i'
	INCLUDE 'exec/ables.i'

	INCLUDE 'hardware/intbits.i'
	INCLUDE 'hardware/adkbits.i'
	INCLUDE 'hardware/cia.i'
	INCLUDE 'hardware/custom.i'

	INCLUDE 'resources/disk.i'

	INCLUDE 'devices/timer.i'

	INCLUDE 'trackdisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'
	LIST

	INT_ABLES


;****** Imported Names ***********************************************

	XREF		tdName

*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

	XREF		_ciaa			;------ most of the disk regs
	XREF		_ciab			;------ track00 select

	XREF		_custom

ABSEXECBASE	EQU	4

*------ Functions ----------------------------------------------------

	XREF		TDGetUnit
	XREF		TDGiveUnit

*------ System Library Functions -------------------------------------

	EXTERN_LIB	Alert
	EXTERN_LIB	DoIO
	EXTERN_LIB	Debug
	EXTERN_LIB	GetMsg
	EXTERN_LIB	Wait
	EXTERN_LIB	PutMsg
	EXTERN_LIB	WaitIO
	EXTERN_LIB	Signal
	EXTERN_LIB	SetSignal

;****** Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	XDEF		TDCalibrate
	XDEF		TDSeek
	XDEF		TDMotor
	XDEF		TDDelay
	XDEF		TDStep
	XDEF		TDTrkRead
	XDEF		TDNewTrkRead
	XDEF		TDTrkWrite
	XDEF		TDNewTrkWrite
	XDEF		TDStartDma
	XDEF		TDStartDmaSync
	XDEF		TDDskBlk
	XDEF		WaitDskTime
	XDEF		WaitDskBlk

*------ Data ---------------------------------------------------------

;****** Local Definitions ********************************************

DSYNC	EQU	$4489

*****i* trackdisk.device/internal/TDCalibrate ***********************
*
*   NAME
*	TDCalibrate -- initialize and calibrate the drive.
*
*   SYNOPSIS
*	Error = TDCalibrate( ), UnitPtr, TDLib
*	D0			A3,	 A6
*
*   FUNCTION
*	This routine does a seek to track zero.	 If the drive cannot
*	find track zero within MAXTRACKS steps an error is returned.
*
*
*   OUTPUTS
*	Error - non-zero if an error occurred
*
*
*   EXCEPTIONS
*
*   SEE ALSO
*	TDSeek
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- TDLib
*
*


TDCalibrate
		MOVE.L D2,-(SP)

		INFOMSG	50,<'%s/TDCalibrate: called'>
		BSR	TDGetUnit

*		;------ get the 6526 value
		MOVE.B	TDU_6522(A3),D0

*		;------ set ourselves to side 0
		BSET	#CIAB_DSKSIDE,D0

*		;------ step in until track0 goes away
		BCLR	#CIAB_DSKDIREC,D0
		MOVE.B	D0,ciaprb+_ciab		; make sure drive is selected
		MOVE.B	D0,TDU_6522(A3)
		CLEAR	D2
		MOVE.W	TDU_NUMTRACKS(A3),D2
		LSR.L	#2,D2			; max 40 tracks

Calib_in:
		BTST	#CIAB_DSKTRACK0,ciapra+_ciaa
		BNE.S	Calib_out

		BSR.S	TDStep
		DBF	D2,Calib_in

Calib_out:
		;------ wait for head to settle - we're changing directions
		;------ allow other units to get at hardware in meantime
		BSR	TDGiveUnit
		MOVE.L	TDU_SETTLEDELAY(A3),D0
		BSR	TDDelay
		BSR	TDGetUnit
*		MOVE.B	TDU_6522(A3),_ciab+ciaprb	; select drive!

		BSET	#CIAB_DSKDIREC,TDU_6522(A3) ; seek outwards
		CLR.W	TDU_SEEKTRK(A3)		; mark us at cylinder 0

		;------ change TDU_STEPDELAY to TDU_CALIBRATEDELAY (temp)
		;------ this is because we need an extra ms for trk0 valid.
		MOVE.L	TDU_STEPDELAY(A3),-(SP)	; save
		MOVE.L	TDU_CALIBRATEDELAY(A3),TDU_STEPDELAY(A3)

		;------	20 more than the number of cylinders
		MOVE.W	TDU_NUMTRACKS(A3),D2		; loop count
		LSR.W	#1,D2
		ADD.W	#20,D2
		BRA.S	Calib_20

Calib_10
		BSR.S	TDStep

*		;------ check for track00
Calib_20
		BTST	#CIAB_DSKTRACK0,ciapra+_ciaa
		DBEQ	D2,Calib_10

*		;------ we now might be on track00
		MOVE.B	ciapra+_ciaa,D2

*		;------ restore step delay
		MOVE.L	(SP)+,TDU_STEPDELAY(A3)

*		;------ unselect the drive
		BSR	TDGiveUnit

		BTST	#CIAB_DSKTRACK0,D2
		BEQ.S	Calib_30		; we ARE on track00

		INFOMSG	50,<'%s/TDCalibrate: Seek Error'>
		MOVEQ	#TDERR_SeekError,D0
		BRA.S	Calib_End

Calib_30:
*		;------ wait for settling time
		MOVE.L	TDU_SETTLEDELAY(A3),D0
		BSR	TDDelay
		MOVEQ	#0,D0			; mark us as successful

Calib_End:
		MOVE.L (SP)+,D2
		RTS

*****i* trackdisk.device/internal/TDStep ****************************
*
*   NAME
*	TDStep -- step the drive one track
*
*   SYNOPSIS
*	TDStep( ) UnitPtr, TDLib
*		  A3,	   A6
*
*   FUNCTION
*	This routine turns steps the drive one track in.  For the
*	moment it has built into it a 3ms delay.  Futures for this
*	routine is to suspend the task for the 3ms (instead of busy
*	waiting) and/or to fask-seek (e.g. do long seeks w/ faster
*	steps).
*
*   INPUTS
*	UnitPtr - A pointer to the unit structure for this unit.  Before
*		calling TDStep, the unit must have been allocated.
*
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
*   REGISTER USAGE
*	A6 -- TDLib
*	A3 -- Ptr to Unit structure
*	A1 -- Ptr to PRB of _ciab
*

TDStep:
		INFOMSG	80,<'TDStep: called'>

		MOVE.B	TDU_6522(A3),D0
		LEA	ciaprb+_ciab,A1

		MOVE.B	D0,D1
		BCLR	#CIAB_DSKSTEP,D0
		MOVE.B	D0,(A1)			; step goes active

		; delay for 1 microsec
		; CIA is connected to E clk - can't hit it too fast

		MOVE.B	D1,(A1)			; step goes inactive

		BSR	TDGiveUnit

		;------ wait for correct step delay
		MOVE.L	TDU_STEPDELAY(A3),D0
		BSR	TDDelay

		BSR	TDGetUnit

		;------ This make sure the drive is selected - GetUnit doesn't
*		MOVE.B	TDU_6522(A3),ciaprb+_ciab

		RTS



*****i* trackdisk.device/internal/TDSeek ****************************
*
*   NAME
*	TDSeek -- Seek to a particular track.
*
*   SYNOPSIS
*	error = TDSeek( TrackNum ), UnitPtr, TDLib
*	         	D0,	    A3,	     A6
*
*   FUNCTION
*	This routine does a seek to the specified track.
*
*   INPUTS
*	TrackNum - the number of the track to seek to.
*
*   OUTPUTS
*	error - error number if there was an error, else zero
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
*---------------------------------------------------------------------
*
*   IMPLEMENTATION NOTES
*
*	    It is useful to keep the distinction between tracks
*	and cylinders in mind.	There are two tracks per cylinder
*	(one for each head of the disc).  We will first select the
*	correct head, and then we will seek the arm to the correct
*	spot over the disc.
*
*
*   REGISTER USAGE
*	A6 -- TDLib
*	A3 -- UnitPtr
*	D2 -- number of cylinders left to seek
*
*
*---------------------------------------------------------------------


TDSeek
		MOVEM.L D2/D3,-(SP)

		IFGE	INFO_LEVEL-80
		MOVE.L	D0,-(SP)
		PEA	0
		MOVE.W	TDU_SEEKTRK(A3),2(SP)
		INFOMSG	80,<'%s/TDSeek: from %ld to %ld'>
		ADDQ.L	#8,SP
		ENDC

*		;------ get current track number
		MOVE.W	TDU_SEEKTRK(A3),D2
		BPL.S	Seek_05

*		;------ negative numbers means we need to calibrate
		MOVE.L	D0,D2
		BSR	TDCalibrate
		TST.L	D0
		BNE.S	Seek_Err		; the calibrate failed

		EXG.L	D2,D0			; D2 is now null

Seek_05:

*		;------ see if we have moved
		CMP.W	D0,D2
		BEQ.S	Seek_End

*		;------ select the correct head, make sure disk is selected
		MOVE.L	D0,D3
		BSR	TDGetUnit
		MOVE.B	TDU_6522(A3),D1
		BSET	#CIAB_DSKSIDE,D1		; assume side 0

		BTST	#0,D3			; check the least bit

		BEQ.S	Seek_10

*		;------ it is really side 1
		BCLR	#CIAB_DSKSIDE,D1
Seek_10
		MOVE.B	D1,TDU_6522(A3)		; restore 6522 byte
		MOVE.B	D1,ciaprb+_ciab

*****		MOVE.W	D1,-(SP)
*****		MOVE.L	TDU_SIDESELECTDELAY(A3),D0
*****		BSR	TDDelay
*****		MOVE.W	(SP)+,D1

*		;------ now seek the arm

		MOVE.W	D3,TDU_SEEKTRK(A3)	; update track #

*		;------ turn into cylinder numbers
		LSR.W	#1,D2
		LSR.W	#1,D3

		LEA	ciaprb+_ciab,A1		; for stuffing later...

		SUB.W	D3,D2
		BCS.S	Seek_20

		BHI.S	Seek_30

		BSR	TDGiveUnit

*		;------ we didn't need to seek at all.	Wait for side sel settle
*****		MOVE.L	TDU_SIDESELECTDELAY(a3),D0
*****		BSR	TDDelay
		BRA.S	Seek_End

*		;------ we need to seek outwards (towards lower tracks)
Seek_20
		BCLR	#CIAB_DSKDIREC,D1
		MOVE.B	D1,TDU_6522(A3)
		MOVE.B	D1,(A1)			; give drive a few micros to
						; learn about direction...
		NEG.W	D2			; make the # of tracks > 0
		BRA.S	Seek_50

Seek_30
		BSET	#CIAB_DSKDIREC,D1
		MOVE.B	D1,TDU_6522(A3)
		MOVE.B	D1,(A1)			; see above comment re: direction
		BRA.S	Seek_50

Seek_40
		BSR	TDStep
Seek_50
		DBF	D2,Seek_40

		BSR	TDGiveUnit

		MOVE.L	TDU_SETTLEDELAY(A3),D0	; settling time
		BSR.S	TDDelay

Seek_End:
		CLEAR	D0

Seek_Err:

		MOVEM.L (SP)+,D2/D3
		RTS



*****i* trackdisk.device/internal/TDMotor ***************************
*
*   NAME
*	TDMotor -- turn the motor on or off
*
*   SYNOPSIS
*	OldState = TDMotor( OnOff ), UnitPtr, TDLib
*	D0,		    D0,	     A3,      A6
*
*   FUNCTION
*	This routine turns the motor on or off.	 If OnOff is non-zero
*	then the motor is turned on.  If it is zero then the motor is
*	turned off.
*
*   INPUTS
*	OnOff - A flag to tell which state the motor should be in.
*		zero	 ==> turn motor off
*		non-zero ==> turn motor on
*
*   OUTPUTS
*	OldState - a flag to tell the old state of the motor
*		zero	 ==> motor was off
*		non-zero ==> motor was on
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
*   REGISTER USAGE
*	A6 -- TDLib
*


TDMotor:
		MOVEM.L	D2/D3/D4,-(SP)

		;------ put the user's request into "normal" form
		TST.L	D0
		SEQ	D1			; active low
		AND.B	#CIAF_DSKMOTOR,D1

*		;------ only get the unit if the motor is changing
		MOVE.B	TDU_6522(A3),D2
		AND.B	#CIAF_DSKMOTOR,D2
		CMP.B	D1,D2
		BEQ.S	TDMotor_Status

	IFGE	INFO_LEVEL-100
	PEA	0
	MOVE.B	D1,3(SP)
	PEA	0
	MOVE.B	TDU_6522(A3),3(SP)
	INFOMSG	100,<'%s/TDMotor: current state is %02lx.  New state is %02lx'>
	ADDQ.L	#8,SP
	ENDC

		;------ update the motor pointer
		BCLR.B	#CIAB_DSKMOTOR,TDU_6522(A3)
		OR.B	D1,TDU_6522(A3)

*		;------ get unit 8520 word
		BSR	TDGetUnit

		;------ we must first set up the motor before selecting
		MOVEQ	#-1,D1			; unselect all drives
		EOR.B	D2,D1			; d2 = DSKMOTOR or 0
		MOVE.B	D1,_ciab+ciaprb		; motor latches on selN->low
		MOVE.B	TDU_6522(A3),_ciab+ciaprb ; selects unit

		;------ allow others to use hardware
		BSR	TDGiveUnit

		;------ see if we turned it on or off
		BTST	#CIAB_DSKMOTOR,TDU_6522(A3)
		BNE.S	TDMotor_Status		; active low

		IFGE	INFO_LEVEL-60
		MOVE.L	A3,-(SP)
		INFOMSG	60,<'%s/TDMotor: turning on unit 0x%lx'>
		ADDQ.L	#4,SP
		ENDC

		;-- Instead of one big wait, wait 1/5 and check ready
		;-- until full time or READY.  Max 5 times.  REJ
		;-- spec says 500ms till up to speed, DSKRDY may not
		;-- be valid until 700ms.  We will wait no longer than 500ms.
MOTORCHECK	EQU	5

		MOVEQ	#MOTORCHECK-1,D3
TDMotor_loop:
		MOVE.L	#(TDT_MOTORON/MOTORCHECK),D0
		BSR.S	TDDelay

		; must select unit after getting it to read DSKRDY line!
		BSR	TDGetUnit		; get unit before using hw
*		MOVE.B	TDU_6522(A3),_ciab+ciaprb
		MOVE.B	ciapra+_ciaa,D4
		BSR	TDGiveUnit

		BTST	#CIAB_DSKRDY,D4
		DBEQ	D3,TDMotor_loop		; low means ready

TDMotor_Status:
		CLEAR	D0
		BTST	#CIAB_DSKMOTOR,D2
		SEQ	D0

TDMotor_End:
		MOVEM.L	(SP)+,D2/D3/D4
		RTS



*****i* trackdisk.device/internal/TDDelay ***************************
*
*   NAME
*	TDDelay -- delay for a given # of microseconds
*
*   SYNOPSIS
*	TDDelay( MicroSec ), TDLib
*		 D0,	     A6
*
*   FUNCTION
*	This routine wastes time in a busy wait loop.
*
*   INPUTS
*	MicroSec - The number of microseconds to waste
*
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
*	    It takes about 1.5 micro seconds to one dbra instruction.
*	We will therefore multiply the argument by .75 before entering
*	the loop.  Also remember that .75 = .50 + .25
*
*
*   REGISTER USAGE
*	A6 -- TDLib
*
**********************************************************************
*

TDDelay:
		CMP.L	#300,D0
		BLT.S	Delay_BusyWait

		MOVEM.L	A0/A1,-(SP)

		LEA	TDU_WAITTIMER(A3),A1
		CLR.L	IOTV_TIME+TV_SECS(A1)
		MOVE.L	D0,IOTV_TIME+TV_MICRO(A1)

		LINKSYS	DoIO

	IFGE	INFO_LEVEL-30

	;------ check for an error from the timer
	LEA	TDU_WAITTIMER(A3),A1
	MOVE.B	IO_ERROR(A1),D0
	BEQ.S	Delay_NoErr

	EXT.W	D0
	EXT.L	D0
	MOVEM.L	D0/A0,-(SP)
*****	INFOMSG	30,<'%s/TDDelay: error %ld on timer wait'>
	ALERT	(AN_TrackDiskDev!AG_IOError!AO_TimerDev),,A0
	MOVEM.L	(SP)+,D0/A0

Delay_NoErr:
	ENDC

		MOVEM.L	(SP)+,A0/A1
		BRA.S	Delay_End

Delay_BusyWait:

	IFD	REALBUSYWAIT
	;------	commented out so 68020 might work
		LSR	#1,D0
		MOVE.L	D0,D1
		LSR	#1,D1
		ADD.L	D1,D0
		LSR	#2,D1
		SUB.L	D1,D0

		MOVE.W	D0,D1
		SWAP	D0

Delay_20:
		DBF	D1,Delay_20

		MOVE.W	#$0FFFF,D1
		SUBQ.W	#1,D0
		BPL.S	Delay_20
	ENDC

	IFND	REALBUSYWAIT
		MOVE.L	(A3),D1
Delay_30:
		MOVE.L	D1,(A3)		; can't be cached, even on '030 REJ
		DBRA	D0,Delay_30

	ENDC

Delay_End:

		RTS




*****i* trackdisk.device/internal/TDTrkRead ***************************
*
*   NAME
*	TDTrkRead -- read a raw track in from disc
*
*   SYNOPSIS
*	Error = TDTrkRead( Buffer, Length ), UnitPtr, TDLib
*	D0		   A0,	   D0:15-0,  A3,      A6
*
*	Error = TDNewTrkRead( Buffer, Func, Length ), UnitPtr, TDLib
*	D0		      A0,     A1    D0:15-0,  A3,      A6
*
*   FUNCTION
*	This routine reads a raw track in from disc in high
*	density format.  It will need to be mfm decoded by
*	software.
*
*	If TDNewTrkRead is called, the func routine will be called
*	just before the disk dma is started.  This function may do
*	things like wait for the index pulse.  It will be called with
*	the word to move into DSKLEN in D0, and a pointer to the
*	special chips in A1.  When this routine returns the
*	disk driver will wait for the disk completed interrupt.
*
*   INPUTS
*	Buffer - a pointer to be beginning of the area to read the
*	    track into.
*
*	Func - a function to call instead of turning on disk dma.
*
*	Length - the size (in bytes) of the data area.	Length must
*	    be an integer number of WORDS -- e.g. it must be evenly
*	    divisible by two.  The maximum transfer length is 32K bytes.
*
*   RESULTS
*	Error - error number (if there was an error) else zero
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
*	TDTrkRead:
*	This routine now uses DiskSync to read tracks in.  The transfer
*	will start with the SECOND $4489 of the sync.
*	Note that it will re-sync at EVERY sync mark, even after the gap.
*	TDMfmAllignTrack counts on this.
*	Also note that when using disksync, one must set a timer to keep
*	from waiting forever for a sync that never comes (unformatted disk).
*
*	The timer must be 200ms + 1.5%*200ms + 5%*200ms + 1 sector time
*	(.09*200ms=18ms) + gap time (830/6812 * 200ms = 25ms) + some leeway
*	= 200ms + 3ms + 10ms + 18ms + 25ms + 44ms(leeway) = 300ms.
*	I picked leeway out of thin air. REJ
*
*   REGISTER USAGE
*	A6 -- TDLib
*	A3 -- UnitPtr
*	A1 -- ptr to _custom area
*
**********************************************************************
*

TDTrkRead:
		LEA	TDStartDmaSync(pc),A1
TDNewTrkRead:
	IFGE	INFO_LEVEL-40
	move.l	a0,-(sp)
	move.w	d0,-(sp)
	clr.w	-(sp)
	PUTMSG	40,<'%s/TDTrkRead: reading 0x%lx bytes to 0x%lx'>
	addq.l	#8,sp
	ENDC

		MOVEM.L	D2/A2/A4,-(SP)
		MOVE.L	A0,A2
		MOVE.L	D0,D2
		MOVE.L	A1,A4

*		;------ select the drive
		;------ this is needed because GetUnit doesn't select it!
		BSR	TDGetUnit
*		MOVE.B	TDU_6522(A3),ciaprb+_ciab

		;------ make sure disk write mode is set up at least 64
		;------ microsecs before turning on dma
		MOVE.W	#DSKDMAOFF,_custom+dsklen

		;---!!! wait for drive to relearn about side select
		MOVE.L	TDU_SIDESELECTDELAY(a3),D0
		BSR	TDDelay

*		;------ load the buffer pointer register
		LEA	_custom,A1
		MOVE.L	A2,dskpt(A1)

*		;------ clear the interrupt request register
		MOVE.W	#INTF_DSKBLK!INTF_DSKSYNC,intreq(A1)

*		;------ enable the disk block interrupt
		MOVE.W	#INTF_SETCLR!INTF_DSKBLK,intena(A1)

*		;------ check to see if there is a disc in the drive
		BTST	#CIAB_DSKCHANGE,ciapra+_ciaa
		BNE.S	TrkRead_LoadLength

TrkRead_Err:
		MOVEQ	#TDERR_DiskChanged,D2
		BRA.S	TrkRead_End

TrkRead_LoadLength:
*		;------ load the length register
		LSR.W	#1,D2			; turn bytes into words
		OR.W	#DSKLENF_DMAEN,D2
		MOVE.L	D2,D0
		JSR	(A4)

*		;------ wait until done	(returns D0)
		BSR	WaitDskTime

*		;------ clear the interrupt enable register
		LEA	_custom,A1
		MOVE.W	#INTF_DSKBLK,intena(A1)

*		;------ Turn off DMA
		MOVE.W	#DSKDMAOFF,dsklen(A1)

*		;------ Did the DiskDmaComplete interrupt occur?
		TST.L	D0
		BEQ.S	TrkRead_StopTimer

*		;------ Timer went off, pulled by WaitDskTime
		MOVEQ	#TDERR_NoSecHdr,D2	; can't find a sector!
		BRA.S	TrkRead_End

*		;------ dma completed - abort timer (may have gone off)
TrkRead_StopTimer:
		LEA	TDU_WAITTIMER(A3),A1
		MOVE.L	A1,D2			; save iob ptr
		ABORTIO
		MOVE.L	D2,A1
		LINKSYS	WaitIO			; removes it from the port

*		;------ check to see if there is a disc in the drive
		BTST	#CIAB_DSKCHANGE,ciapra+_ciaa
		BEQ.S	TrkRead_Err

		CLEAR	D2		; no errors
TrkRead_End:
		BSR	TDGiveUnit

		MOVE.L	D2,D0
		MOVEM.L	(SP)+,D2/A2/A4
		RTS


*****i* trackdisk.device/internal/TDTrkWrite **************************
*
*   NAME
*	TDTrkWrite -- write a raw track out to disc
*	TDNewTrkWrite -- write a raw track out to disc
*
*   SYNOPSIS
*	Error = TDTrkWrite( Buffer, Length ), UnitPtr, TDLib
*	D0		    A0,	    D0:15-0,  A3,      A6
*
*	Error = TDNewTrkWrite( Buffer, Func, Length ), UnitPtr, TDLib
*	D0		       A0,     A1    D0:15-0,  A3,      A6
*
*   FUNCTION
*	This routine writes a raw track out to disc in high
*	density format.  It will need to be mfm encoded by
*	software.
*
*	If TDNewTrkWrite is called, the func routine will be called
*	just before the disk dma is started.  This function may do
*	things like wait for the index pulse.  It will be called with
*	the word to move into DSKLEN in D0, and a pointer to the
*	special chips in A1.  When this routine returns the
*	disk driver will wait for the disk completed interrupt.
*
*   INPUTS
*	Buffer - a pointer to be beginning of the area to read the
*	    track into.
*
*	Func - a function to call instead of turning on disk dma.
*
*	Length - the size (in bytes) of the data area.	Length must
*	    be an integer number of WORDS -- e.g. it must be evenly
*	    divisible by two.  The maximum transfer length is 32K bytes.
*
*   RESULTS
*	Error - zero if sucessful; error number if there was an error.
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
*   REGISTER USAGE
*	A6 -- TDLib
*	A3 -- UnitPtr
*
**********************************************************************
*

TDStartDmaSync:
*		;------ set up disksync
		MOVE.W	#DSYNC,dsksync(A1)
		MOVE.W	#ADKF_SETCLR!ADKF_WORDSYNC,adkcon(A1)

		BSR.S	TDStartDma

*		;------ Set up timer for 1 track + a bit (worst case)
*		;------ 200 ms + 1.5% (disk) + 5% (clock) + 18.5% (safety)
*		;------ equals around 250 ms
		LEA	TDU_WAITTIMER(A3),A1
		CLR.L	IOTV_TIME+TV_SECS(A1)
		MOVE.L	TDU_TDT_DISKSYNC(A3),IOTV_TIME+TV_MICRO(A1)
		BEGINIO
		RTS

TDStartDma:
*		;------ clear the disk dma signal bit and timer signal bit
		MOVEM.L	D0/A1,-(SP)
		CLEAR	D0
		MOVE.L	#TDSF_DSKBLK!TDSF_WAITTIMER,D1
		LINKSYS	SetSignal
		MOVEM.L	(SP)+,D0/A1

		MOVE.W	D0,dsklen(A1)
		MOVE.W	D0,dsklen(A1)
		RTS

TDTrkWrite:
		;------ for compatibility
		LEA.L	TDStartDma(pc),A1

TDNewTrkWrite:
	IFGE	INFO_LEVEL-40
	move.l	a0,-(sp)
	move.w	d0,-(sp)
	clr.w	-(sp)
	PUTMSG	40,<'%s/TDTrkWrite: writing 0x%lx bytes at 0x%lx'>
	addq.l	#8,sp
	ENDC

		MOVEM.L	D2/A2/A4,-(SP)
		MOVE.L	A0,A2
		MOVE.L	D0,D2
		MOVE.L	A1,A4

	IFGE	INFO_LEVEL-120
* copy root track to space buffer space when written
	cmp.w	#$50,TDU_SEEKTRK(a3)
	bne.s	3$
	move.l	TDU_BUFFER(a3),a0
	move.l	a0,a1
	move.l	#TDB_SIZE,d0
	add.l	d0,a1		; after buffer
	lsr.w	#2,d0		; longwords
	bra.s	2$
1$:	move.l	(a0)+,(a1)+
2$:	dbra	d0,1$
	move.l	TDU_BUFFER(a3),a0
	lea	TDB_SECPTRS+TDB_SIZE(a0),a0
	move	TDU_NUMSECS(a3),d0
	bra.s	5$
4$:	add.l	#TDB_SIZE,(a0)+
5$:	dbra	d0,4$

	move.l	a2,a0
	move.l	d2,d0
	move.l	a4,a1
3$:
	ENDC
*		;------ select the drive
		;------ this is needed because GetUnit doesn't select it!
		BSR	TDGetUnit
*		MOVE.B	TDU_6522(A3),ciaprb+_ciab

		;------ make sure disk write mode is set up at least 64
		;------ microsecs before turning on dma
		MOVE.W	#DSKDMAOFF,_custom+dsklen

		;---!!! wait for drive to relearn about side select
		MOVE.L	TDU_SIDESELECTDELAY(a3),D0
		BSR	TDDelay

*		;------ load the buffer pointer register
		LEA	_custom,A1
		MOVE.L	A2,dskpt(A1)

*		;------ clear the interrupt request register
		MOVE.W	#INTF_DSKBLK!INTF_DSKSYNC,intreq(A1)

*		;------ enable the disk block interrupt
		MOVE.W	#INTF_SETCLR!INTF_DSKBLK,intena(A1)

*		;------ check to see if there is a disc in the drive
		BTST	#CIAB_DSKCHANGE,ciapra+_ciaa
		BEQ	TrkWrite_ChangeErr

*		;------ check for write protect -- protected if bit is zero
TrkWrite_CheckProtect:
		BTST	#CIAB_DSKPROT,ciapra+_ciaa
		BEQ	TrkWrite_ProtErr

		;------ make sure we are still allowed to write
		;------ removed DISABLE locking from this test REJ
		BTST	#TDB_RESET,TD_FLAGS(A6)
		BEQ.S	TrkWrite_NotReset

		;------ return an error
		PUTMSG	40,<'%s/TDTrkWrite: we are reset'>
		MOVEQ	#TDERR_PostReset,D2
		BRA	TrkWrite_End

TrkWrite_NotReset:
		BSET	#TDB_WRITING,TD_FLAGS(A6)

		;------ Clear out the old precompensation
		;------ Also clear out disksync left by read routine
		MOVE.W	#ADKF_PRECOMP0!ADKF_PRECOMP1!ADKF_WORDSYNC,adkcon(A1)

		;------ get the current track number
		MOVE.W	TDU_SEEKTRK(A3),D0

		;------ check to see if 0 ns of precomp will do
		MOVE.W	#ADKF_SETCLR!ADKF_PRE000NS,D1
		CMP.W	TDU_COMP01TRACK(A3),D0
		BLS.S	TrkWrite_DoComp

		;------ check to see if 140 ns of precomp will do
		MOVE.W	#ADKF_SETCLR!ADKF_PRE140NS,D1
		CMP.W	TDU_COMP10TRACK(A3),D0
		BLS.S	TrkWrite_DoComp

		;------ check to see if 280 ns of precomp will do
		MOVE.W	#ADKF_SETCLR!ADKF_PRE280NS,D1
		CMP.W	TDU_COMP11TRACK(A3),D0
		BLS.S	TrkWrite_DoComp

		;------ well, 560 ns better do it!
		MOVE.W	#ADKF_SETCLR!ADKF_PRE560NS,D1

TrkWrite_DoComp:
		MOVE.W	D1,adkcon(A1)

*		;------ load the length register
		LSR.W	#1,D2			; turn bytes into words
		OR.W	#DSKLENF_DMAEN!DSKLENF_WRITE,D2
		MOVE.L	D2,D0

		;------ call the start dma routine
		JSR	(A4)

*		;------ wait for the message from the interrupt routine
		BSR.S	WaitDskBlk

*		;------ clear the interrupt enable register
		LEA	_custom,A1
		MOVE.W	#INTF_DSKBLK,intena(A1)

*		;------ wait for erase head to turn off
		MOVE.L	TDU_POSTWRITEDELAY(a3),D0
		BSR	TDDelay

*		;------ Turn off disc dma
		MOVE.W	#DSKDMAOFF,dsklen(A1)

		MOVE.B	TD_FLAGS(A6),D0
		BCLR	#TDB_WRITING,TD_FLAGS(A6)

		BTST	#TDB_RESET,D0
		BEQ.S	TrkWrite_SecondNotReset

		PUTMSG	40,<'%s/TDTrkWrite: after a reset'>
		LEA	TD_RESETIOB(A6),A1
		BEGINIO

TrkWrite_SecondNotReset:

*		;------ check to see if there is a disc in the drive
		BTST	#CIAB_DSKCHANGE,ciapra+_ciaa
		BEQ.S	TrkWrite_ChangeErr

		MOVEQ	#0,D2

TrkWrite_End:
*		;------ unselect the drive
		BSR	TDGiveUnit

		MOVE.L	D2,D0
		MOVEM.L	(SP)+,D2/A2/A4

		RTS

TrkWrite_ProtErr:
		MOVEQ	#TDERR_WriteProt,D2
		BRA.S	TrkWrite_End

TrkWrite_ChangeErr:
		MOVEQ	#TDERR_DiskChanged,D2
		BRA.S	TrkWrite_End


*****i* trackdisk.device/internal/TDDskBlk ****************************
*
*   NAME
*	TDDskBlk -- handle the disk block interrupt
*
*   SYNOPSIS
*	TDDskBlk( UnitPtr )
*	   	  A1
*
*   FUNCTION
*	This routine gets called by the resource to process a disk block
*	interrupt.
*	Note: also used in mfm.asm to indicate blitter done!
*
*   INPUTS
*
*   RESULTS
*	Error - zero if sucessful; error number if there was an error.
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
*	Switched from send the timer request to the WaitPort to sending
*	a signal to the task.  This way I can safely have both timer
*	requests and disk dma signals active.
*
*   REGISTER USAGE
*	A6 -- TDLib
*	A3 -- UnitPtr
*
**********************************************************************
*


TDDskBlk:
*		LEA	TDU_WAITPORT(A1),A0
*		LEA	TDU_WAITTIMER(A1),A1
*		LINKLIB	_LVOPutMsg,ABSEXECBASE

		LEA	TDU_TCB(A1),A1
		MOVE.L	#TDSF_DSKBLK,D0
		LINKLIB	_LVOSignal,ABSEXECBASE
		RTS

*****i* trackdisk.device/internal/WaitDskTime ************************
*
*   NAME
*	WaitDskTime -- handle the disk block interrupt
*
*   SYNOPSIS
*	which = WaitDskTime(), UnitPtr
*	 D0  	  	         A3
*	0     = WaitDskBlk(), UnitPtr
*	D0   	  	        A3
*
*   FUNCTION
*	Called to wait for a dskblk interrupt or a timer request complete.
*	WaitDskBlk only wats for the DSKBLK.
*
*   INPUTS
*
*   RESULTS
*	which - zero if DSKBLK int; 1 if timer request complete
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
*	Switched from send the timer request to the WaitPort to sending
*	a signal to the task.  This way I can safely have both timer
*	requests and disk dma signals active.
*
*   REGISTER USAGE
*	A6 -- TDLib
*	A3 -- UnitPtr
*
**********************************************************************
*
*
*	Wait for either disk dma done or timer
*
WaitDskTime:
		MOVE.L	D2,-(SP)
		MOVE.L	#TDSF_WAITTIMER!TDSF_DSKBLK,D2
		BRA.S	DoWaitDskBlk
*
*	Wait for disk done
*
WaitDskBlk:
		MOVE.L	D2,-(SP)
		MOVE.L	#TDSF_DSKBLK,D2

DoWaitDskBlk:
		MOVE.L	D2,D0
		LINKSYS	Wait

		; was it disk dma done?
		; always check dma complete first to avoid spuriosly
		; indicating failure
		BTST	#TDSB_DSKBLK,D0
		BEQ.S	CheckTime
		MOVEQ	#0,D0			; flag to TrkRead
		BRA.S	WaitExit

		; was it a timer signal?
CheckTime:	BTST	#TDSB_WAITTIMER,D0
		BEQ.S	DoWaitDskBlk

		; make sure we really got it and remove
		LEA	TDU_WAITPORT(A3),A0
		LINKSYS	GetMsg
		TST.L	D0
		BEQ.S	DoWaitDskBlk
		MOVEQ	#1,D0			; flag to TrkRead
WaitExit:
		MOVE.L	(SP)+,D2
		RTS

	END
