
*************************************************************************
*									*
* Copyright (C) 1985,1989 Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* resource.asm
*
* Source Control
* ------ -------
* 
* $Id: resource.asm,v 36.15 91/04/21 03:15:39 jesup Exp $
*
* $Locker:  $
*
* $Log:	resource.asm,v $
* Revision 36.15  91/04/21  03:15:39  jesup
* Fixed handling of drive 0 being HD.
* 
* Revision 36.14  91/02/19  03:59:59  jesup
* Added a (v37)
* 
* Revision 36.13  90/11/21  04:27:20  jesup
* Partial support for multi-density drives.  Added ReadUnitID, etc.
* Still need to handle startup right
* 
* Revision 36.12  90/07/13  14:59:23  jesup
* $id.
* 
* Revision 36.11  89/05/17  13:23:02  jesup
* Added docs about interrupt states.
* 
* Revision 36.10  89/05/08  22:46:13  jesup
* GiveUnit/GetUnit faster, autodoc improvements (tested and works)
* 
* Revision 36.9  89/05/08  19:10:40  jesup
* Fixed Giveunit disable bug
* 
* Revision 36.8  89/04/12  21:03:54  bryce
* *** empty log message ***
* 
* Revision 36.7  89/04/11  22:33:00  jesup
* improved autodocs
* 
* Revision 36.6  89/04/11  22:29:46  jesup
* improved autodocs
* 
* Revision 36.5  89/04/11  16:34:50  bryce
* Stuff LN_NAME for *all* the disk.resource interrupts (DSKBLK,DSKSYNC,EXTER).
* Assume A0=_custom for non-server interrupts (free speed, smaller).
* 
* Revision 36.4  89/03/27  13:42:43  jesup
* *** empty log message ***
* 
* Revision 36.3  89/03/08  20:13:41  jesup
* Added check to GiveUnit to make sure it's owned.
* Removed recoverable alert from GetUnit if already owned.
* 
* Revision 36.2  89/01/23  23:21:42  bryce
* Identify interrupt as owned by the disk.resouce (via LN_NAME).
* Also code shrinks.
* 
* Revision 36.1  88/08/19  10:26:57  neil
* changed autodocs to new format
* 
* Revision 33.1  86/05/06  01:50:12  neil
* we turn off interrupts in the critical period between AddICRVector
* and disabling that vector.  Also fixed some autodoc stuff
* 
* Revision 32.2  85/12/23  15:14:12  neil
* properly detects the disk id number (did not reset the motor before...)
* 
* Revision 32.1  85/12/23  12:42:51  neil
* fixed index interrupt.  cia resource MAKES A COPY OF THE INTERRUPT,
* so the disk resources poking of the interrupt structure did
* no good.  sigh.
* 
* 
*************************************************************************

	SECTION section

;****** Included Files ***********************************************

	NOLIST
	INCLUDE	'exec/types.i'
	INCLUDE	'exec/nodes.i'
	INCLUDE	'exec/lists.i'
	INCLUDE	'exec/ports.i'
	INCLUDE	'exec/interrupts.i'
	INCLUDE	'exec/libraries.i'
	INCLUDE	'exec/tasks.i'
	INCLUDE	'exec/initializers.i'
	INCLUDE	'exec/execbase.i'
	INCLUDE	'exec/strings.i'
	INCLUDE	'exec/ables.i'
	INCLUDE	'exec/alerts.i'

	INCLUDE	'resources/cia.i'

	INCLUDE	'hardware/custom.i'
	INCLUDE	'hardware/intbits.i'
	INCLUDE	'hardware/dmabits.i'
	INCLUDE	'hardware/adkbits.i'
	INCLUDE	'hardware/cia.i'
	LIST

	INCLUDE	'disk.i'
	INCLUDE	'messages.i'
	INCLUDE	'asmsupp.i'
	INCLUDE 'disk_rev.i'

;****** Imported Globals *********************************************

	XREF	_AbsExecBase

	XREF	_custom

	XREF	VERNUM
	XREF	REVNUM
	XREF	_ciaa
	XREF	_ciab

	INT_ABLES

;****** Imported Functions *******************************************

	EXTERN_LIB	MakeLibrary
	EXTERN_LIB	OpenResource
	EXTERN_LIB	AddIntServer
	EXTERN_LIB	SetIntVector
	EXTERN_LIB	AddResource
	EXTERN_LIB	AddTail
	EXTERN_LIB	RemHead
	EXTERN_LIB	ReplyMsg
	EXTERN_LIB	Debug
	EXTERN_LIB	AbleICR
	EXTERN_LIB	AddICRVector
	EXTERN_LIB	Alert

	XREF		drStart

;****** Exported Functions *******************************************

	XDEF		DRInit
	XDEF		AllocUnit
	XDEF		FreeUnit
	XDEF		GetUnit
	XDEF		GiveUnit
	XDEF		GetUnitID
	XDEF		ReadUnitID
	XDEF		IntDiskBlock
	XDEF		IntDiskSync
	XDEF		IntIndexMark

	XDEF		diskName
	XDEF		VERSTRING

;**** Local Definitions **********************************************

*
* macro to deal with debugging messages
*
INFOLEVEL	EQU	5

*		;------ outsiders equivalent of callsys
LINKSYS		MACRO
		LINKLIB	_LVO\1,DR_SYSLIB(A6)
		ENDM

CALLSYS		MACRO
		CALLLIB	_LVO\1
		ENDM

ABSEXECBASE	EQU	4

CIAABITS	EQU (CIAF_DSKRDY!CIAF_DSKTRACK0!CIAF_DSKPROT!CIAF_DSKCHANGE)

funcInit:
		DC.W	-1
		DC.W	AllocUnit-funcInit
		DC.W	FreeUnit-funcInit
		DC.W	GetUnit-funcInit
		DC.W	GiveUnit-funcInit
		DC.W	GetUnitID-funcInit
		DC.W	ReadUnitID-funcInit
		DC.W	-1

*****i* System/Resources/DR/DRInit ***********************************
*
*   NAME
*       DRInit - initialize the disk resource
*
*   SYNOPSIS
*       DRInit()
*
*
*   FUNCTION
*	This routine starts the disk resource up from scratch
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

DRInit:
		MOVEM.L	D2/D3/A2/A3/A6,-(SP)

		MOVE.L	ABSEXECBASE,A6	;short form addressing


		LEA	ciaName(pc),A1
		CALLSYS	OpenResource
		MOVE.L	D0,D2
		BEQ	Init_End	;silent failure (but will never happen)

		LEA	funcInit(pc),A0		; function vector init
		LEA	structInit(pc),A1	; data space initialization
		SUBA.L	A2,A2			; init function
		MOVE.L	#DR_SIZE,D0		; data size

		CALLSYS	MakeLibrary

		TST.L	D0
		BNE.S	Init_Addints

*****		PUTMSG	5,<'%s/Init: cannot MakeLibrary'>
		ALERT	AN_DiskRsrc!AG_MakeLib,,A0
		BRA	Init_End

Init_Addints:
		MOVE.L	D0,A2

*		;------ We have the library.  Add the int servers/handlers
		IFGE	INFOLEVEL-50
		MOVE.L	A2,-(SP)
		PUTMSG	50,<'%s/DRInit: resource is at 0x%lx'>
		ADDQ.L	#4,SP
		ENDC

		MOVE.L	#diskName,D3	;For tagging the Interrupt owner!
		;[D3-diskName]
		
		LEA	DR_DISCBLOCK(A2),A1
		MOVE.L	D3,LN_NAME(A1)		;Identify the Interrupt owner!
		MOVE.L	#IntDiskBlock,IS_CODE(A1)
		MOVE.L	A2,IS_DATA(A1)
		MOVEQ	#INTB_DSKBLK,D0
		CALLSYS	SetIntVector

		LEA	DR_DISCSYNC(A2),A1
		MOVE.L	D3,LN_NAME(A1)		;Identify the Interrupt owner!
		MOVE.L	#IntDiskSync,IS_CODE(A1)
		MOVE.L	A2,IS_DATA(A1)
		MOVEQ	#INTB_DSKSYNC,D0
		CALLSYS	SetIntVector

*		;------ Initialize the 8520
		LEA	_ciab,A1
		MOVEQ	#-1,D0
		MOVE.B	D0,ciaprb(A1)	; turn "off" all bits
		MOVE.B	D0,ciaddrb(A1)	; direction of port
		AND.B	#-(CIAABITS)-1,ciaddra+_ciaa

		;------ put in the cia resource interrupt
		DISABLE
		MOVE.L	D2,DR_CIARESOURCE(A2)
		LEA	DR_INDEX(A2),A1
		MOVE.L	A2,IS_DATA(A1)
		MOVE.L	#IntIndexMark,IS_CODE(A1)
		MOVE.L	D3,LN_NAME(A1)		;Identify the Interrupt owner!
		MOVEQ	#CIAICRB_FLG,D0
		LINKLIB	_LVOAddICRVector,D2

		;[D3-scratch]
		;------ and disable the interrupt
		MOVEQ	#CIAICRF_FLG,D0
		LINKLIB	_LVOAbleICR,D2

		ENABLE

		;------ get the base of the special chips
		LEA	_custom,A0

*		;------ save SYSLIB pointer
		MOVE.L	A6,DR_SYSLIB(A2)

*		;------ Put disk in its initial configuration

*		; make sure portia dma is off
		MOVE.W	#DSKDMAOFF,dsklen(A0)

*		;------ Enable DMA
		MOVE.W	#DMAF_SETCLR!DMAF_DISK,dmacon(A0)

		;------ initialize the list
		LEA	DR_WAITING(A2),A0
		NEWLIST	A0

		;------ now find out what type of drives are in the machine
		;------ Note: with the advent of dual-speed floppies, these
		;------ values may change!
		MOVEQ	#2,D2			; check 3 drives (1-3) in loop
		MOVE.B	#CIAF_DSKSEL0,D3	; check drive 0 first
		LEA	DR_UNITID(A2),A3
		BSR.S	readID
		lsl.b	#1,D3		; bump select to drive 1
		addq.l	#1,d0		; was it -1 (no code returned)?
		bne.s	Init_readid	; returned a code, believe it

		;------ set the internal drive to type AMIGA
		;-- didn't return a code, assume 3.5" 1M drive
		MOVE.L	#DRT_AMIGA,DR_UNITID(A2)
		
Init_readid:
		BSR.S	readID
		LSL.B	#1,D3
		DBRA	D2,Init_readid

*		;------ add the resource
		MOVE.L	A2,A1
		CALLSYS	AddResource

Init_End:
		MOVEM.L	(SP)+,D2/D3/A2/A3/A6
		RTS

*
* read the id type.  called with:
*	A3 -- where to write the type (incremented to next cell)
*	D3 -- select bit of unit
*
* internally:
*       D1 -- loop counter for reading 31 bits
*	D0 -- hold the bits
*	A0 -- ciabprb
*

readID:
		NOT.B	D3
		LEA	_ciab+ciaprb,A0

		;------ turn the motor on and off to reset the counter
		MOVE.B	#(~CIAF_DSKMOTOR&$ff),D0	; set motor on bit
		MOVE.B	D0,(A0)				
		AND.B	D3,D0				; select unit
		MOVE.B	D0,(A0)
		MOVE.B	#$FF,(A0)			; unselect unit
		MOVE.B	D3,(A0)				; turn motor off
		MOVE.B	#$FF,(A0)			; unselect unit

		;------ now read 32 bits into D0
		MOVEQ	#31,D1
		CLEAR	D0

readID_loop:
		LSL.L	#1,D0
		MOVE.B	D3,(A0)
		BTST	#CIAB_DSKRDY,_ciaa+ciapra
		BEQ.S	10$
		BSET	#0,D0
10$:
		MOVE.B	#$FF,(A0)
		DBRA	D1,readID_loop

		MOVE.L	D0,(A3)+

		IFGE	INFOLEVEL-30
		 pea	0
		 move.b D3,3(sp)
		 not.b  3(sp)
		 MOVE.L	D0,-(SP)
		 PUTMSG	30,<'%s/readID: got id 0x%lx (sel=0x%lx)'>
		 ADDQ.L	#8,SP
		ENDC


		NOT.B	D3		; restore (we not-ed it up top)
		RTS
		


structInit:
*		;------ Initialize the device
		INITBYTE	LN_TYPE,NT_RESOURCE
		INITLONG	LN_NAME,diskName
		INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD	LIB_VERSION,VERNUM
		INITWORD	LIB_REVISION,REVNUM
;;;;		INITLONG	LIB_IDSTRING,VERSTRING
		INITLONG	DR_CURRENT,-1
		DC.W		0

******* disk.resource/GetUnit ***********************************
*
*   NAME
*       GetUnit - allocate the disk for a driver
*
*   SYNOPSIS
*       lastDriver = GetUnit( unitPointer ), DRResource
*       D0		      A1             A6
*
*	struct DiscResourceUnit *GetUnit(struct DiscResourceUnit *);
*
*   FUNCTION
*	This routine allocates the disk to a driver.  It is either
*	immediately available, or the request is saved until the disk
*	is available.  When it is available, your unitPointer is
*	sent back to you (via ReplyMsg).  You may then reattempt the
*	GetUnit.
*	
*	Allocating the disk allows you to use the disk's resources.
*	Remember however that there are four units to the disk; you are
*	only one of them.  Please be polite to the other units (by never
*	selecting them, and by not leaving interrupts enabled, etc.).
*
*	When you are done, please leave the disk in the following state:
*	    dmacon dma bit ON
*	    dsklen dma bit OFF (write a #DSKDMAOFF to dsklen)
*	    adkcon disk bits -- any way you want
*	    entena:disk sync and disk block interrupts -- Both DISABLED
*	    CIA resource index interrupt -- DISABLED
*	    8520 outputs -- doesn't matter, because all bits will be
*		set to inactive by the resource.
*	    8520 data direction regs -- restore to original state.
*
*	NOTE: GetUnit() does NOT turn on the interrupts for you.
*	      You must use AbleICR (for the index interrupt) or intena
*	      (for the diskbyte and diskblock interrupts) to turn them
*	      on.  You should turn them off before calling GiveUnit,
*	      as stated above.
*
*   INPUTS
*	unitPtr - a pointer you your disk resource unit structure.
*		Note that the message filed of the structure MUST
*		be a valid message, ready to be replied to.  Make sure
*		ln_Name points to a null-terminated string, preferably
*		one that identifies your program.
*
*		You need to set up the three interrupt structures,
*		in particular the IS_DATA and IS_CODE fields.  Set them
*		to NULL if you don't need that interrupt.  Also, set
*		the ln_Type of the interrupt structure to NT_INTERRUPT.
*		WARNING: don't turn on a disk resource interrupt unless
*		the IS_CODE for that interrupt points to executable code!
*
*		IS_CODE will be called with IS_DATA in A1 when the
*		interrupt occurs.  Preserve all regs but D0/D1/A0/A1.
*		Do not make assumptions about A0.
*
*   RESULTS
*	lastDriver - if the disk is not busy, then the last unit
*		to use the disk is returned.  This may be used to
*		see if a driver needs to reset device registers.
*		(If you were the last user, then no one has changed
*		any of the registers.  If someone else has used it,
*		then any allowable changes may have been made).  If the
*		disk is busy, then a null is returned.
*
*   EXCEPTIONS
*
*   SEE ALSO
*	GiveUnit
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*


GetUnit:
		;------ Do whatever possible outside DISABLE - REJ
		MOVE.L	A5,-(SP)
		MOVE.L	A6,A5
		MOVE.L	DR_CURRENT(A5),D0
		MOVE.L	DR_SYSLIB(A5),A6

		;-- a6 = execbase
		DISABLE	

		BSET	#DRB_ACTIVE,DR_FLAGS(A5)
		BNE.S	GetUnit_Busy

		ERRMSG	60,<'%s/GetUnit: granting disk'>

*		;------ update DR_CURRENT and set up return value
		MOVE.L	A1,DR_CURRENT(A5)

*		;------ Save task for GiveUnit check - REJ
		MOVE.L	ThisTask(A6),DR_CURRTASK(A5)
		BRA.S	GetUnit_End

*		;------ user did not get it -- link him on a list
GetUnit_Busy:
		IFGE	INFOLEVEL-20
*		;------ see if this guy currently has the device
		CMP.L	D0,A1
		BNE.S	GetUnit_Add

		MOVE.L	A1,-(SP)
		ERRMSG	20,<'%s/GetUnit: 0x%lx already has disk'>
		ADDQ.W	#4,SP
GetUnit_Add:
		ENDC

		LEA	DR_WAITING(A5),A0
		ADDTAIL

		ERRMSG	50,<'%s/GetUnit: grant failed'>

*		;------ return failure
		CLEAR	D0
GetUnit_End:	;-- a6 has execbase
		ENABLE

		; restore regs
		MOVE.L	A5,A6
		MOVE.L	(sp)+,A5
		RTS


******* disk.resource/GiveUnit **********************************
*
*   NAME
*       GiveUnit - Free the disk back up
*
*   SYNOPSIS
*       GiveUnit(), DRResource
*           	    A6
*
*	void GiveUnit();
*
*   FUNCTION
*	This routine frees the disk after a driver is done with it.
*	If others are waiting, it will notify them.
*
*   INPUTS
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*	GetUnit
*
*   BUGS
*	In pre-V36, GiveUnit didn't check if you owned the unit.  A patch
*	for this was part of 1.3.1 SetPatch.  Fixed in V36.
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*

GiveUnit:
		ERRMSG	60,<'%s/GiveUnit: called'>

		MOVE.L	A5,-(sp)
		MOVE.L	A6,A5		; so we can put execbase in a6

		;------ Do as much as possible outside DISABLE - REJ
		CLEAR	D0
		MOVE.L	DR_SYSLIB(A5),A6
		MOVE.L	ThisTask(A6),D1
		DISABLE

		;------ make sure you own it - REJ
		IFGE	INFOLEVEL-50
		CMP.L	DR_CURRTASK(A5),D1
		BEQ.S	GiveOK
		MOVE.L	D1,-(SP)
		ERRMSG	50,<'%s/GiveUnit: didn't own unit, task 0x%lx'>
		ADDQ.W	#4,SP
GiveOK:
		ENDC

		CMP.L	DR_CURRTASK(A5),D1	; REJ
		BNE.S	GiveUnitIgnore		; if branch, D0 == 0

		;------ make sure all the bits are turned off
		MOVE.B	#$ff,_ciab+ciaprb

*		;------ mark the resource as available
		BCLR.B	#DRB_ACTIVE,DR_FLAGS(A5)

*		;------ see if someone is waiting (see before disable REJ)
		LEA	DR_WAITING(A5),A0	; RemHead relies on this!
		REMHEAD				; since we don't know if it's empty

GiveUnitIgnore:
		; a6 == execbase
		ENABLE

		TST.L	D0			; see branch to Ignore
		BEQ.S	GiveUnit_End

		IFGE	INFOLEVEL-50
		MOVEM.L	D0,-(SP)
		ERRMSG	50,<'%s/GiveUnit: waking 0x%lx up'>
		ADDQ.L	#4,SP
		ENDC

*		;------ someone was waiting for the unit (a6 == execbase)
		MOVE.L	D0,A1
		CALLSYS	ReplyMsg

GiveUnit_End:
		;-- restore regs
		MOVE.L	A5,A6
		MOVE.L	(sp)+,A5
		RTS



******* disk.resource/AllocUnit *********************************
*
*   NAME
*       AllocUnit - allocate a unit of the disk
*
*   SYNOPSIS
*       Success = AllocUnit( unitNum ), DRResource
*	D0     	             D0         A6
*
*	BOOL AllocUnit(LONG);
*
*   FUNCTION
*	This routine allocates one of the units of the disk.  It should
*	be called before trying to use the disk (via GetUnit).
*
*	In reality, it is perfectly fine to use GetUnit/GiveUnit if AllocUnit
*	fails.  Do NOT call FreeUnit if AllocUnit did not succeed.  This
*	has been the case for all revisions of disk.resource.
*
*   INPUTS
*	unitNum -- a legal unit number (zero through three)
*
*   RESULTS
*	Success -- nonzero if successful.  zero on failure.
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

AllocUnit:
		BSET.B	D0,DR_FLAGS(A6)
		SEQ	D0		; scc sets low byte only
		RTS			; however, d0 was a long passed in
					; in the range 0-3, so 8..31 are 0.

******* disk.resource/FreeUnit **********************************
*
*   NAME
*       FreeUnit - deallocate the disk
*
*   SYNOPSIS
*       FreeUnit( unitNum ), DRResource
*		  D0         A6
*
*	void FreeUnit(LONG);
*
*   FUNCTION
*	This routine deallocates one of the units of the disk.  It should
*	be called when done with the disk.  Do not call it if you did
*	no successfully allocate the disk (there is no protection -- you
*	will probably crash the disk system).
*
*   INPUTS
*	unitNum -- a legal unit number (zero through three)
*
*   RESULTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*	FreeUnit
*
*   BUGS
*	Doesn't check if you own the unit, or even if anyone owns it.
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*

FreeUnit:
		BCLR.B	D0,DR_FLAGS(A6)
		RTS


******* disk.resource/GetUnitID *********************************
*
*   NAME
*       GetUnitID - find out what type of disk is out there
*
*   SYNOPSIS
*       idtype = GetUnitID( unitNum ), DRResource
*	D0	            D0         A6
*
*	LONG GetUnitID(LONG);
*
*   FUNCTION
*	Gets the drive ID for a given unit.  Note that this value may
*	change if someone calls ReadUnitID, and the drive id changes.
*
*   INPUTS
*	unitNum -- a legal unit number (zero through three)
*
*   RESULTS
*	idtype -- the type of the disk drive.  Standard types are
*		defined in the resource include file.
*
*   EXCEPTIONS
*
*   SEE ALSO
*	ReadUnitID
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*

GetUnitID:
		LSL.L	#2,D0
		MOVE.L	DR_UNITID(A6,D0),D0
		RTS


******* disk.resource/ReadUnitID *********************************
*
*   NAME
*       ReadUnitID - reread and return the type of drive (V37)
*
*   SYNOPSIS
*       idtype = ReadUnitID( unitNum ), DRResource
*	D0	               D0         A6
*
*	ULONG ReadUnitID(LONG);
*
*   FUNCTION
*	Rereads the drive id for a specific unit (for handling drives
*	that change ID according to what sort of disk is in them.  You
*	MUST have done a GetUnit before calling this function!
*
*   INPUTS
*	unitNum -- a legal unit number (zero through three)
*
*   RESULTS
*	idtype -- the type of the disk drive.  Standard types are
*		defined in the resource include file.
*
*   EXCEPTIONS
*
*   SEE ALSO
*	GetUnitID
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*

ReadUnitID:
		movem.l	d2/d3/a3,-(a7)
		move.l	d0,d2			; save for later
		move.b	#CIAF_DSKSEL0,d3	; select bit for unit 0
		lsl.b	d0,d3			; make the right select bit

		;-- keep the disk resource base up to date
		LEA	DR_UNITID(A6),A0
		LSL.L	#2,D0
		lea	0(A0,D0),A3		; address to write the new ID to

		bsr	readID			; (d3/a3) returns ID in d0
						; A3 has been bumped 1 entry!

		;-- if this is unit 0 AND we get DRT_EMPTY, then it's
		;-- really DRT_AMIGA
		cmp.l	#DRT_EMPTY,d0
		bne.s	all_done
		tst.l	d2			; unit
		bne.s	all_done
		move.l	#DRT_AMIGA,d0
		move.l	d0,-4(a3)		; it was bumped 4 in readID!!!
all_done:
		movem.l	(a7)+,d2/d3/a3
		RTS


*********************************************************************
*
* interrupt routines
*
*********************************************************************

;This is not a chain, A0 is _custom
IntDiskBlock:
		MOVE.W	#INTF_DSKBLK,intreq(a0)	;fast index (ack interrupt)
		MOVE.L	DR_CURRENT(A1),D0
		BEQ.S	IntNoCurrent
		MOVE.L	D0,A1

		MOVEM.L	DRU_DISCBLOCK+IS_DATA(A1),A1/A5
		JMP	(A5)

;This is not a chain, A0 is _custom
IntDiskSync:
		MOVE.W	#INTF_DSKSYNC,intreq(a0) ;fast index (ack interrupt)
		MOVE.L	DR_CURRENT(A1),D0
		BEQ.S	IntNoCurrent
		MOVE.L	D0,A1

		MOVEM.L	DRU_DISCSYNC+IS_DATA(A1),A1/A5
		JMP	(A5)

;This IS a chain, A0 is garbage
IntIndexMark:
		;------ ok to have index ints if not one is current...
		MOVE.L	DR_CURRENT(A1),D0
		BEQ.S	NullRoutine
		MOVE.L	D0,A1

		MOVEM.L	DRU_INDEX+IS_DATA(A1),A1/A5
		JMP	(A5)

IntNoCurrent:
		IFGE	INFOLEVEL-5
*****		PUTMSG	10,<'%s/Interrupt -- no active unit'>
		ALERT	AN_DRIntNoAct,,A0
		ENDC

;		RTS
;		[fall]
NullRoutine:
		RTS

diskName:	dc.b	'disk.resource',0
ciaName:	dc.b	'ciab.resource',0
VERSTRING:      VSTRING

		; used to have a CNOP 0,4 here REJ
		CNOP	0,2
		XDEF	EndCode
EndCode:
		END

