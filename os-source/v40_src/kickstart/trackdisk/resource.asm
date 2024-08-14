
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* resource.asm
*
* Source Control
* ------ -------
* 
* $Id: resource.asm,v 27.7 91/03/13 20:44:11 jesup Exp $
*
* $Locker:  $
*
* $Log:	resource.asm,v $
* Revision 27.7  91/03/13  20:44:11  jesup
* Select the unit after getting it.
* Always clear WORDSYNC, etc on GetUnit
* 
* Revision 27.6  90/11/21  04:13:22  jesup
* Added call to call disk resource ReadUnitID
* 
* Revision 27.5  90/06/01  23:18:02  jesup
* Conform to include standard du jour
* 
* Revision 27.4  89/05/15  21:18:44  jesup
* Added more docs on what GetUnit really does, minor opt
* 
* Revision 27.3  89/04/27  23:40:05  jesup
* fixed autodocs
* 
* Revision 27.2  89/02/17  20:06:43  jesup
* Minor optimization
* 
* Revision 27.1  85/06/24  13:37:43  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  15:13:30  neil
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

	INCLUDE 'resources/disk.i'

	INCLUDE 'hardware/adkbits.i'
	INCLUDE 'hardware/custom.i'
	INCLUDE 'hardware/cia.i'

	INCLUDE 'devices/timer.i'

	INCLUDE 'trackdisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'

	SECTION section


****** Imported Globals *********************************************

	XREF		tdName

	XREF		_custom
	XREF		_ciab

****** Imported Functions *******************************************

	EXTERN_LIB	GetMsg
	EXTERN_LIB	SetSignal
	EXTERN_LIB	Wait

****** Exported Functions *******************************************

	XDEF		TDGetUnit
	XDEF		TDGiveUnit
	XDEF		TDReadUnitID

****** Local Definitions ********************************************


*****i* trackdisk.device/internal/TDGetUnit **************************
*
*   NAME
*       TDGetUnit - Get the unit for use
*
*   SYNOPSIS
*       TDGetUnit(), UnitPtr, TDDev
*           	     A3,      A6
*
*   FUNCTION
*	This routine gets the unit from the disc resource.  It will
*	not return until the resource is allocated.  It will also
*	do any necessary initializations when the disc is allocated.
*	It sets up DSKMOTOR on the output lines, but nothing else.
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

TDGetUnit:
		MOVEM.L	D2/A2/A4,-(SP)
		MOVE.L	A6,A2

		MOVE.L	TD_SYSLIB(A2),A4
		MOVE.L	TD_DISCRESOURCE(A2),A6

TDGetUnit_Try:
		LEA	TDU_DRU(A3),A1
		CALLLIB	DR_GETUNIT

*		;------ see if we really got it
		TST.L	D0
		BNE.S	GetUnit_GotUnit

		EXG.L	A4,A6

		IFGE	INFO_LEVEL-50
		PEA	0
		MOVE.B	TDU_UNITNUM(A3),3(SP)
		PUTMSG	50,<'%s/GetUnit: unit %ld waiting for disc'>
		ADDQ.L	#4,SP
		ENDC

*		;------ someone else has the unit.  Wait for it.
GetUnit_Wait:
		MOVE.L	#TDSF_WAITTIMER,D0
		CALLSYS	Wait

		LEA	TDU_WAITPORT(A3),A0
		CALLSYS GetMsg
		TST.L	D0
		BEQ.S	GetUnit_Wait

*		;------ go back and try again
		EXG	A4,A6
		BRA.S	TDGetUnit_Try

GetUnit_GotUnit:
*		;------ restore our device pointer
		EXG.L	A2,A6

		IFGE	INFO_LEVEL-50
		PEA	0
		MOVE.B	TDU_UNITNUM(A3),3(SP)
		INFOMSG	50,<'%s/GetUnit: got device for unit %ld'>
		ADDQ.L	#4,SP
		ENDC

*		;------ put the motor bit in the correct state
*		;------ sets all other bits to 1!!
		;------ this is REQUIRED (doesn't work without it)
		;------ motor line MUST be valid before selecting drive.
		MOVE.B	TDU_6522(A3),D1
		MOVE.L	D1,D2			; save old value
		LEA	_ciab+ciaprb,A0
		OR.B	#(~CIAF_DSKMOTOR)&$FF,D1
		MOVE.B	D1,(A0)
		;------ set up the control bits for the unit
		MOVE.B	D2,(A0)

*		;------ see if we were the last user.  If so, nothing changed
*		;------ only reason I can see for this is to avoid hitting
*		;------ chip bus contention - REJ
*		LEA	TDU_DRU(A3),A0
*		CMP.L	D0,A0
*		BEQ.S	GetUnit_End
		; that caused sync to be left on in 1-drive systems

*		;------ set our self up for using the unit
		LEA	_custom+adkcon,A0

*		;------ we have a Paula.  clear and set bits
		MOVE.W	#ADKF_MSBSYNC!ADKF_WORDSYNC,(A0)
		MOVE.W	#ADKF_SETCLR!ADKF_FAST!ADKF_MFMPREC,(A0)

GetUnit_End:
		MOVEM.L	(SP)+,D2/A2/A4

		RTS

*****i* trackdisk.device/internal/TDGiveUnit *************************
*
*   NAME
*       TDGiveUnit - Give the unit back to the resource
*
*   SYNOPSIS
*       TDGiveUnit(), UnitPtr, TDDev
*           	      A3,      A6
*
*   FUNCTION
*	This routine returns the unit to the resource.  The one
*	thing that it has to be careful of is to update the resource
*	to use the current state of our motor.
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

TDGiveUnit:
		LINKLIB	DR_GIVEUNIT,TD_DISCRESOURCE(A6)

		IFGE	INFO_LEVEL-50
		PEA	0
		MOVE.B	TDU_UNITNUM(A3),3(SP)
		INFOMSG	50,<'%s/GiveUnit: freeing unit %ld'>
		ADDQ.L	#4,SP
		ENDC

		RTS

*****i* trackdisk.device/TDReadUnitID *************************
*
*   NAME
*       TDReadUnitID - Call DR_ReadUnitID to see if the floppy changed type
*
*   SYNOPSIS
*       type = TDReadUnitID(), UnitPtr, TDDev
*        D0   		         A3,      A6
*
*   FUNCTION
*	This reads the current state of the floppy ID to find out if it
*	might have changed, due to insertion of a different type of disk.
*
*   INPUTS
*
*   RESULTS
*	type - the current drive id for the drive
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

TDReadUnitID:
		MOVEQ	#0,D0
		MOVE.B	TDU_UNITNUM(A3),D0
		LINKLIB	DR_READUNITID,TD_DISCRESOURCE(A6)

		IFGE	INFO_LEVEL-50
		move.l	d0,-(a7)
		PEA	0
		MOVE.B	TDU_UNITNUM(A3),3(SP)
		INFOMSG	50,<'%s/ReadUnitID: reading unit %ld, got 0x%lx'>
		ADDQ.L	#8,SP
		ENDC

		RTS

	END
