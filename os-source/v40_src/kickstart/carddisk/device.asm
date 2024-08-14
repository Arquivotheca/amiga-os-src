**
**	$Id: device.asm,v 1.6 92/12/14 10:57:27 darren Exp $
**
**	Credit card device
**
**	(C) Copyright 1991 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

** Includes

	NOLIST
	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/resident.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/io.i"
	INCLUDE "exec/semaphores.i"
	INCLUDE "exec/tasks.i"
	INCLUDE "exec/memory.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/errors.i"
	INCLUDE	"exec/initializers.i"
	INCLUDE	"exec/macros.i"
	INCLUDE	"exec/ables.i"

	INCLUDE "libraries/configregs.i"
	INCLUDE "libraries/expansionbase.i"
	INCLUDE "libraries/dos.i"
	INCLUDE "libraries/dosextens.i"
	INCLUDE "libraries/filehandler.i"
	INCLUDE "libraries/expansion.i"

	INCLUDE	"devices/trackdisk.i"

	INCLUDE	"hardware/intbits.i"

	INCLUDE "internal/librarytags.i"

	INCLUDE	"cardddata.i"
	INCLUDE	"carddisk_rev.i"
	INCLUDE	"debug.i"

	LIST


** Exports

** Imports

	XREF	HandleInsert
	XREF	HandleStatus
	XREF	HandleRemoved
	XREF	CDDTaskStart
	XREF	CheckRemoved

	XREF	EndModule		;endmodule.asm

	XREF	_LVOPermit

** Equates

CARD_BOOT_PRI	EQU	3

NORAWIO		EQU	1

***********************************************************************

	IFEQ	ROMBUILD

		moveq	#-1,d0
		rts

	ENDC

resident:
		dc.w	RTC_MATCHWORD
		dc.l	resident
		dc.l	EndModule

		dc.b	RTF_COLDSTART
		dc.b	VERSION
		dc.b	NT_DEVICE
		dc.b	15		;after trackdisk
		dc.l	carddname
		dc.l	ident
		dc.l	CDInit

ident:

		VSTRING

cardresname:
		CARDRESNAME

handlename:
		dc.b	'CC0',0
		ds.w	0

CDStructInit:

	; init library struct

		INITLONG	LN_NAME,carddname
		INITLONG	LIB_IDSTRING,ident
		INITWORD	LIB_REVISION,REVISION
		INITWORD	LIB_VERSION,VERSION
		INITBYTE	LN_TYPE,NT_DEVICE

	; init IS_CODE field of all interrupts

		INITLONG	cdb_Inserted+IS_CODE,HandleInsert
		INITLONG	cdb_Removed+IS_CODE,HandleRemoved
		INITLONG	cdb_Status+IS_CODE,HandleStatus

	; init CardHandle structure

		INITLONG	cdb_CardHandle+LN_NAME,carddname
		INITBYTE	cdb_CardHandle+cah_CardFlags,CARDF_DELAYOWNERSHIP
		INITBYTE	cdb_CardHandle+LN_PRI,-10

	; init task struct

		INITBYTE	cdb_TC+LN_TYPE,NT_TASK
		INITBYTE	cdb_TC+LN_PRI,5
		INITLONG	cdb_TC+LN_NAME,carddname

	; init data for MakeDosNode

		INITLONG	cdb_DrivePtr,handlename
		INITLONG	cdb_DevicePtr,carddname
	;	INITLONG	cdb_UnitNum,0
	;	INITLONG	cdb_FirstFlags,0

		INITLONG	cdb_DosEnvec+de_TableSize,DE_BOOTBLOCKS
		INITLONG	cdb_DosEnvec+de_SizeBlock,(512>>2)
	;	INITLONG	cdb_DosEnvec+de_SecOrg,0
		INITLONG	cdb_DosEnvec+de_Surfaces,1
		INITLONG	cdb_DosEnvec+de_SectorPerBlock,1
		INITLONG	cdb_DosEnvec+de_BlocksPerTrack,8
		INITLONG	cdb_DosEnvec+de_Reserved,2
	;	INITLONG	cdb_DosEnvec+de_PreAlloc,0
	;	INITLONG	cdb_DosEnvec+de_Interleave,0
	;	INITLONG	cdb_DosEnvec+de_LowCyl,0
		INITLONG	cdb_DosEnvec+de_HighCyl,4	;unknown
		INITLONG	cdb_DosEnvec+de_NumBuffers,5
		INITLONG	cdb_DosEnvec+de_BufMemType,MEMF_PUBLIC
		INITLONG	cdb_DosEnvec+de_MaxTransfer,$200000
		INITLONG	cdb_DosEnvec+de_Mask,$7ffffffe
		INITLONG	cdb_DosEnvec+de_BootPri,CARD_BOOT_PRI
		INITLONG	cdb_DosEnvec+de_DosType,ID_FFS_DISK
	;	INITLONG	cdb_DosEnvec+de_Baud,0
	;	INITLONG	cdb_DosEnvec+de_Control,0
		INITLONG	cdb_DosEnvec+de_BootBlocks,2

		dc.w	0

CDFuncInit:
		dc.w	-1

		dc.w	DeviceOpen-CDFuncInit
		dc.w	DeviceClose-CDFuncInit
		dc.w	DeviceExpunge-CDFuncInit
		dc.w	DeviceExtFunc-CDFuncInit

		dc.w	DeviceBeginIO-CDFuncInit
		dc.w	DeviceAbortIO-CDFuncInit

		dc.w	-1

**********************************************************************
**	Drive Boot Code
**********************************************************************

**	Overlapped Fake Expansion Structures

	; -- ConfigDev		; -- BootDiagArea
	; Type	; (Prod)
	; (Flag	; Res03)	; Config; (Flags)
	; (Manufacturer)	; (Size)
	; (Serial ...		; (DiagPoint) 
	;   ... Number)		; Autoboot
	; (InitDiagVec)		; DeviceName
	; BootDiagArea
	;   ...

	CNOP	0,4

FakeConfigDev	equ	*-cd_Rom	;node, flags, pad
		dc.b	ERTF_DIAGVALID			; er_Type
		dc.b	0				; er_Product
BootDiag	dc.b	DAC_CONFIGTIME	; da_Config	; er_Flags
		dc.b	0		; da_Flags	; er_Reserved03
		dc.w	0		; da_Size	; er_Manufacturer
		dc.w	0		; da_DiagPoint	; er_SerialNumber
		dc.w	0 		; da_BootPoint ;  ...
		dc.w	carddname-BootDiag ; da_Name	; er_InitDiagVec
		dc.l	BootDiag			; er_Reserved0c...0f

carddname:
		dc.b	'carddisk.device',0

	CNOP	0,4

*------ carddisk.device/Init -----------------------------------------
*
*    NAME
*	Init - initialize device
*
*    SYNOPSIS
*	LibraryBase = Init();
*
*    FUNCTION
*	Returns a pointer to the device base if successful, else NULL
*	if some error.
*
*---------------------------------------------------------------------

CDInit:
	PRINTF	DBG_ENTRY,<'CARDDISK -- Init()'>

		movem.l	a2-a6,-(sp)

		lea	cardresname(pc),a1
		JSRLIB	OpenResource

		tst.l	d0
		beq	initfailed

		move.l	d0,a3		;cache

	PRINTF	DBG_FLOW,<'CARDDISK -- Card Resource @ $%lx'>,D0

		lea	CDFuncInit(pc),a0
		lea	CDStructInit(pc),a1
		suba.l	a2,a2
		move.l	#CardDeviceBase_SIZEOF,d0
		JSRLIB	MakeLibrary

	PRINTF	DBG_FLOW,<'CARDDISK -- Device Base @$%lx'>,D0

		tst.l	d0
		beq	initfailed

	; alert?  no, not that drastic - assume low memory

		move.l	d0,a5

		move.l	a6,cdb_ExecLib(a5)		
		move.l	a3,cdb_CardResource(a5)

	; Open expansion, and add DOS BOOT node

		move.l	cdb_ExecLib(a5),a6
		moveq	#OLTAG_EXPANSION,d0
		JSRLIB	TaggedOpenLibrary

		move.l	d0,cdb_ExpanLib(a5)

	; initialize Semaphore (single thread reads/writes of data)

		lea	cdb_SSemaphore(a5),a0
		JSRLIB	InitSemaphore

	; initialize notification list

		lea	cdb_ChangeInt(a5),a0
		NEWLIST	a0

	; initialize task

		lea	cdb_TC(a5),a1
		lea	cdb_Stk(a5),a0

	PRINTF	DBG_FLOW,<'CARDDISK - Task Stack Lower @ $%lx'>,A0

		move.l	a0,TC_SPLOWER(a1)
		move.w	#((CDB_STKSIZE/2)-1),d0
		move.w	#$09999,d1
pattern0Loop:
		move.w	d1,(a0)+
		dbf	d0,pattern0Loop
	;	lea	cdb_Stk+CDB_STKSIZE(a5),a0
		move.l	a0,TC_SPUPPER(a1)
		move.l	a5,-(a0)		;argument (device base)
		move.l	a0,TC_SPREG(a1)

		lea	CDDTaskStart,a2
		suba.l	a3,a3

	PRINTF	DBG_OSCALL,<'CARDDISK -- AddTask($%lx,$%lx,$%lx)'>,A1,A2,A3

		JSRLIB	AddTask

	; initialize interrupts, and CardHandle structure

		move.l	a5,cdb_Inserted+IS_DATA(a5)
		move.l	a5,cdb_Removed+IS_DATA(a5)
		move.l	a5,cdb_Status+IS_DATA(a5)

		lea	cdb_Inserted(a5),a0
		move.l	a0,cdb_CardHandle+cah_CardInserted(a5)

		lea	cdb_Removed(a5),a0
		move.l	a0,cdb_CardHandle+cah_CardRemoved(a5)

		lea	cdb_Status(a5),a0
		move.l	a0,cdb_CardHandle+cah_CardStatus(a5)

	; obtain pointer to common memory

		move.l	cdb_CardResource(a5),a6

	PRINTF	DBG_OSCALL,<'CARDDISK -- GetCardMap()'>,A1,A2,A3

		JSRLVO	GetCardMap
		move.l	d0,a0
		move.l	cmm_CommonMemory(a0),cdb_MapCard(a5)

	; add handle

		lea	cdb_CardHandle(a5),a1

	PRINTF	DBG_OSCALL,<'CARDDISK -- OwnCard($%lx)'>,A1

		JSRLVO	OwnCard

	; use expansion base

		move.l	cdb_ExpanLib(a5),d0
		BEQ_S	returnbase	;what?  not really tragic

	PRINTF	DBG_FLOW,<'CARDDISK - Expansion opened'>

		move.l	d0,a6		;use expansion
		lea	cdb_MakeDosData(a5),a0

		JSRLIB	MakeDosNode
		tst.l	d0
		BEQ_S	returnbase

	; alert?  oh well, treat quietly - just cannot boot off of card
	; and save some ROM space

	PRINTF	DBG_FLOW,<'CARDDISK - Device Node @ $%lx'>,D0

		move.l	d0,a0
		lea	FakeConfigDev(pc),a1
		moveq	#CARD_BOOT_PRI,d0
		moveq	#00,d1
		
		JSRLIB	AddBootNode

	; alert on fail?  oh well, treat quietly - just cannot boot off
	; of card and save some ROM space

	PRINTF	DBG_OSCALL,<'CARDDISK - %ld=AddBootNode()'>,D0


returnbase:
		move.l	cdb_ExecLib(a5),a6

	; Add device

		move.l	a5,a1
		JSRLIB	AddDevice

initfailed:
		movem.l	(sp)+,a2-a6

	PRINTF	DBG_EXIT,<'CARDDISK -- Exit CDInit()'>

		rts


*------ carddisk.device/Expunge ---------------------------------------
*
*    NAME
*	Expunge - indicate desire to remove carddisk.device
*
*    SYNOPSIS
*	Result = Expunge()
*
*    FUNCTION
*	This device does not expunge - no space, and its meant to
*	be ROM'ed.
*
*----------------------------------------------------------------------

DeviceClose:

	PRINTF	DBG_ENTRY,<'CARDDISK - CloseDevice()'>

	; least amount we need to do

		subq.w	#1,LIB_OPENCNT(a6)

	; does not expunge

DeviceExpunge:

DeviceExtFunc:
		; fall through

	;
	; Commands cannot be aborted - SendIO()'s block, and are
	; single-threaded - everything is done much too fast
	; to bother with a queue (not to mention limited ROM space)
	;

DeviceAbortIO:
		moveq	#0,d0
		rts


*****i* carddisk.device/Open ******************************************
*
*   error = OpenDevice(name, unit, ioReq, flags);
*   d0			     d0   a1	  d1
*
*   a6 contains device node
*   Invoked while Forbid
*
***********************************************************************
DeviceOpen:

	PRINTF	DBG_ENTRY,<'OpenDevice(carddisk.device,$%lx,$%lx,$%lx'>,D0,A1,D1

	; unit number MUST be 0 for now; reserve other numbers for
	; the future (should we need a multi-unit carddisk.device)

		tst.l	d0
		bne.s	doFail

	; increment usage count

		addq.w	#1,LIB_OPENCNT(a6)

	; return a non-zero pointer to unit 0 (use imbedded unit related
	; variables for now).

		lea	cdb_CardDiskUnit0(a6),a0

		move.l	a0,IO_UNIT(a1)

		moveq	#0,d0		; no error
doreturn:
		move.b	d0,IO_ERROR(a1)
		rts

doFail:
		moveq	#IOERR_OPENFAIL,d0
		bra.s	doreturn

*------ carddisk.device/BeginIO ---------------------------------------
*
*    NAME
*	BeginIO - Process device commands
*
*    SYNOPSIS
*	void BeginIO(struct IORequest *, struct CardDeviceBase *)
*			A1		 A6
*
*    FUNCTION
*	BeginIO for a Trackdisk like device.
*
*----------------------------------------------------------------------

DeviceBeginIO:

	IFNE	0

		PRINTF	DBG_ENTRY,<'CARDDISK BeginIO()'>
		PRINTF	DBG_ENTRY,<'   IORequest  @ $%lx'>,A1
		PRINTF	DBG_ENTRY,<'   IO_DATA    = $%lx'>,IO_DATA(A1)
		PRINTF	DBG_ENTRY,<'   IO_OFFSET  = $%lx'>,IO_OFFSET(A1)
		PRINTF	DBG_ENTRY,<'   IO_LENGTH  = $%lx'>,IO_LENGTH(A1)

		PUSHWORD	IO_COMMAND(A1)
		PRINTF	DBG_ENTRY,<'   IO_COMMAND = $%lx'>
		POPLONG		1
	ENDC

		movem.l	a2-a6,-(sp)
		move.l	a1,a3			; cache IO request
		move.l	a6,a5			; cache device address
		move.l	cdb_ExecLib(a5),a6

	;
	; protect disk data, and single thread r/w's, and data
	; structures.  Various structures may also be accessed by
	; our task - likewise various behaviors, such as notification
	; of change disk will occur after we finish the current command.
	;
	; the cardchange task is priority 5 - high enough that it should
	; get a turn to notify of a disk change immediately following
	; the command being processed should it occur after the
	; ObtainSemaphore() below (assuming the caller is performing
	; I/O at a reasonable priority of 4 or less).
	;
	; Because commands are NOT queued, SendIO() returns AFTER
	; the command has been processed like ramdrive.device.  So
	; there should never be any need to AbortIO() a command; its
	; already too late.
	;

		lea	cdb_SSemaphore(a5),a0
		JSRLIB	ObtainSemaphore

	; check bounds of command

		move.w	IO_COMMAND(a3),d0
		andi.w	#$7fff,d0		; mask out extended commands
		cmpi.w	#DISKCOMMANDCNT,d0
		bcs.s	dbKnownCommand		; blt unsigned

		moveq	#CMD_INVALID,d0

dbKnownCommand:
	; find entry point

		add.w	d0,d0
		jsr	DiskCommands(pc,d0.w)

		move.b	d0,IO_ERROR(a3)		; return code

	; allow access to disk data

		lea	cdb_SSemaphore(a5),a0
		JSRLIB	ReleaseSemaphore

	; ack - hack for ADD/REM CHANGE INT compatability

		move.w	IO_COMMAND(a3),d0

		cmp.w	#TD_ADDCHANGEINT,d0
		beq.s	dbGrantQuick

		cmp.w	#TD_REMCHANGEINT,d0
		beq.s	dbGrantQuick

		btst	#IOB_QUICK,IO_FLAGS(a3)
		bne.s	dbGrantQuick

	; reply the command

		move.l	a3,a1
		JSRLIB	ReplyMsg

dbGrantQuick:
		movem.l	(sp)+,a2-a6

		rts

**********************************************************************
**********************************************************************
*
*   Disk Commands
*
**********************************************************************
**********************************************************************
DiskInvalid:
DiskReset:
DiskStop:
DiskStart:
DiskFlush:
DiskSeek:
		moveq	#IOERR_NOCMD,d0
		rts


*****i* Motor ********************************************************
*
*   The motor is always on, but we will support turning the
*   light on/off
*
**********************************************************************

DiskMotor:
		bra	aDiskMotor

*****i* ChangeNum ****************************************************
*
*   Disk change count increases
*
**********************************************************************
DiskChangeNum:
		move.l	cdb_ChangeCount(a5),IO_ACTUAL(a3)

		;------ fall through to report no error

*****i* Update *******************************************************
*
*   This driver always immediately writes
*
**********************************************************************
DiskUpdate:
		;-- fall through

*****i* Clear ********************************************************
*
*   This driver always reads fresh data
*
**********************************************************************
DiskClear:
		moveq	#0,d0
		rts


*****i* carddisk.device/DiskAddChange ********************************
*
*    NAME
*	DiskAddChange( IORequest )
*			A3
*	a5 = device base
*	a6 = exec base
*	a3 = IORequest
*
*    FUNCTION
*	Equivalent to trackdisk TDAddChangeInt.  This routine
*	does not "complete".  The request is stashed until
*	TDURemChangeInt is called, when it is finally replied (as
*	the IORequest is linked into a list).
*
*    INPUTS
*	IORequest - IO_DATA -> soft int structure.
*
*    NOTES
*	Called within Obtain/Release Semaphore
*
*	MUST use SendIO().
*
**********************************************************************
DiskAddChange:

		lea	cdb_ChangeInt(a5),a0
		move.l	a3,a1
		JSRLIB	AddTail		
		moveq	#00,d0		;no error
		rts

*****i* carddisk.device/DiskRemChange ********************************
*
*    NAME
*	DiskRemChange( IORequest )
*			A3
*	a5 = device base
*	a6 = exec base
*	a3 = IORequest
*
*    FUNCTION
*	Equivalent to trackdisk TDRemChangeInt.  This command
*	is always executed as IOF_QUICK even if not specified!
*	This is done to be compatable with trackdisk which would
*	try to queue it as a request (no can do since the
*	IORequest is already on a LIST!); while not a problem for
*	carddisk.device which never really queues commands (everything
*	is handled pseudo-immediate like ramdrive.device), we have
*	to handle this oddity for compatability.
*
*    INPUTS
*	IORequest - IO_DATA -> soft int structure.
*
*    NOTES
*	Called within Obtain/Release Semaphore
*
**********************************************************************
DiskRemChange:

		move.l	a3,a1
		JSRLIB	Remove		
		moveq	#00,d0		;no error
		rts

*****i* Remove *******************************************************
*
*   Install single soft interrupt.
*
**********************************************************************
DiskRemove:
		move.l	IO_DATA(a3),cdb_RemoveInt(a5)
		moveq	#0,d0
		rts


DiskCommands:

		bra.s	DiskInvalid		;0
		bra.s	DiskReset		;1
		bra.s	DiskRead		;2
		bra.s	DiskWrite		;3

		bra.s	DiskUpdate		;4
		bra.s	DiskClear		;5
		bra.s	DiskStop		;6
		bra.s	DiskStart		;7

		bra.s	DiskFlush		;8
		bra.s	DiskMotor		;9
		bra.s	DiskSeek		;a
		bra.s	DiskFormat		;b

		bra.s	DiskRemove		;c
		bra.s	DiskChangeNum		;d
		bra.s	DiskChangeState		;e
		bra.s	DiskProtStatus		;f

		bra.s	DiskInvalid		;$10 RAW_READ
		bra.s	DiskInvalid		;$11 RAW_WRITE

	; DRIVETYPE - not defined

		bra.s	DiskInvalid		;$12 GETDRIVETYPE

	; NUMTRACKS - useless info - should use GetGeometry

		bra.s	DiskInvalid		;$13 GETNUMTRACKS

		bra.s	DiskAddChange		;$14
		bra.s	DiskRemChange		;$15

		bra.s	DiskGetGeometry		;$16

	; EJECT - no can do

		bra.s	DiskInvalid		;$17 TD_EJECT
		
DISKCOMMANDCNT	EQU (*-DiskCommands)/2

*****i* ProtStatus ***************************************************
*
*  Check if disk is write protected
*
**********************************************************************
DiskProtStatus:
		bra	aDiskProtStatus

*****i* ChangeState **************************************************
*
*   yes/no card is in drive
*
**********************************************************************
DiskChangeState:

		bra	aDiskChangeState

*****i* carddisk.device/TD_GETGEOMETRY *******************************
*
*   NAME
*	TD_GETGEOMETRY - Get disk geometry
*   
*   FUNCTION
*	Like TD_GETGEOMETRY in trackdisk, except values are more
*	flexable than trackdisk
*
**********************************************************************
DiskGetGeometry:

		bra	aDiskGetGeometry

*****i* carddisk.device/TD_FORMAT ************************************
*
*   NAME
*	TD_FORMAT -- Write data to card memory
*   
*   FUNCTION
*	Write full track of data
*
**********************************************************************
DiskFormat:

	; fall through - same as a big write

*****i* carddisk.device/CMD_WRITE ************************************
*
*   NAME
*	CMD_WRITE -- Wrie data to card memory
*   
*   FUNCTION
*	Transfer data from buffer to card memory
*
**********************************************************************
DiskWrite:
		bra	aDiskWrite

*
* Clear cache for 040 copyback mode cache (flush data)
*
FlushCache:
		movem.l	d0-d1/a0-a1/a6,-(sp)
		move.l	cdb_ExecLib(a5),a6
		JSRLIB	CacheClearU
		movem.l	(sp)+,d0-d1/a0-a1/a6
		rts

*****i* carddisk.device/CMD_READ *************************************
*
*   NAME
*	CMD_READ -- Read data from card memory
*   
*   FUNCTION
*	Transfer data from credit card memory to a supplied buffer.
*
**********************************************************************
DiskRead:

		bsr.s	FlushCache		;read fresh data

		bsr	checkrequest
		bne.s	badread

		tst.w	d1
		beq.s	blockread		;1 block?

	; multi-block read

		movem.l	d3-d6,-(sp)

		move.l	cdb_BKSZ(a5),d3
		movem.l	IO_LENGTH(a3),d4-d6	;length, data, offset
		movem.l	d4-d6,-(sp)		;save all

readloop:
		move.l	d3,IO_LENGTH(a3)
		bsr.s   blockread

		sub.l	d3,d4			;decrement length
		beq.s	readloopdone

		add.l	d3,IO_OFFSET(a3)
		add.l	d3,IO_DATA(a3)

		tst.l	d0			; error during last read?
		beq.s	readloop

readloopdone:

		movem.l	(sp)+,d4-d6
		movem.l	d4-d6,IO_LENGTH(a3)	;restore

		movem.l	(sp)+,d3-d6

badread:
		rts

	; single block read

blockread:
		movem.l	d2-d6,-(sp)

		lea	readverified(pc),a1
		lea	verifyread(pc),a0
		bsr.s	setuptran
		bsr	transfer

		movem.l	(sp)+,d2-d6

		rts

;---------------------------------------------------------------------
;- verify block read, but within Begin/EndCardAccess()
;---------------------------------------------------------------------

verifyread:

	; get location of EDC

		bsr	findEDC

		cmp.b	#TPLFMTEDC_CHSUM,d3
		beq.s	CHKSUMRead

	; if CRC, check 2 bytes (CRC MUST be 2 bytes)

		cmp.b	(a4)+,d5
		bne.s	ReadBadSec

		lsr.w	#8,d5

		cmp.b	(a4),d5
		bne.s	ReadBadSec
		rts

	; if checksum, compare byte (checksum MUST be 1 byte)

CHKSUMRead:
		cmp.b	(a4),d5
		beq.s	readverified

ReadBadSec
		moveq	#TDERR_BadSecSum,d0

readverified:
		rts



*----i- carddisk.device/setuptran ------------------------------------
*
*   NAME
*	setuptran -- set-up for transfer
*   
*   FUNCTION
*	Set-up for data transfer.  Also handles these cases -
*
*	o Error Detection Length of non-zero (1-7 possible)
*	  even if there is no Error Detection scheme being used.
*
*	o Set-up for error detection codes stored in (end of)
*	  data blocks, or in a separate table (sequential array).
*
*   INPUTS
*	a0 = call back pointer
*	a1 = call back pointer (check for WP change)
*   RETURNS
*	d4 = EDC offset, or NULL
*	d3 = EDC type
*	d2 = IO_LENGTH
*	d6 = Callback pointer
*
*	d1 = IO_OFFSET
*	a2 = IO_DATA
*       a4 = Pointer to start of card data block (adjusted if needed)
*
*---------------------------------------------------------------------

setuptran:

	; grab ptr to start of data, and IO_OFFSET
		move.l	a1,d6

		move.l	cdb_DataStart(a5),a4
		move.l	IO_OFFSET(a3),d1
		add.l	d1,a4			; base + IO_OFFSET

		moveq	#TPLFMTEDC_NONE,d3	;default is no detect

	; if EDC Length is 0, then we cannot have any error detection

		moveq	#00,d0
		move.b	cdb_EDCLength(a5),d0
		beq.s	noimbeddedEDC

	; grab error detection method type

		move.b	cdb_ErrorDetect(a5),d3
		beq.s	setupnoEDC		;no EDC?

	; and cache error-detection handling callback hook if we
	; have a disk with error-detection bytes.

		move.l	a0,d6

setupnoEDC:

	; compute TRUE offset, or offset into EDC table

	; (block #)*(EDC length [1-7 bytes])

		move.l	d1,d4
		divu	cdb_BKSZ+2(a5),d4	;(block #)

		mulu	d0,d4		;(block #) * (EDC length)

	; if there is an EDC table, the EDC is not imbedded in the block

		tst.l	cdb_EDCLOC(a5)
		bne.s	noimbeddedEDC

	; else it is imbedded, in which case we need to adjust our offset

		add.l	d4,a4		; + ((# blocks) * (EDC length))

noimbeddedEDC:
	; grab IO_DATA, and IO_LENGTH

		move.l	IO_LENGTH(a3),d2
		move.l	IO_DATA(a3),a2

		rts


*----i- carddisk.device/checkrequest ----------------------------------
*
*   NAME
*	checkrequest -- make sure request is valid, and determine
*		if we can do I/O as a single block, or multi-block
*   
*   INPUTS
*	a3 = IORequest
*   RETURNS
*	D1 = number of blocks to transfer - 1
*       D0 = 0 means good IORequest, any other number is an error
*
*	CC = condition of D0
*
*   NOTES
*	MUST PRESERVE D0!!!
*
*---------------------------------------------------------------------

checkrequest:

	; assume no transfer

		clr.l	IO_ACTUAL(a3)

	; make sure IO_LENGTH is valid

		move.l	IO_LENGTH(a3),d1
		beq.s	badlength

	; and IO_OFFSET and IO_LENGTH on are on block boundaries

		move.l	IO_OFFSET(a3),d0
		or.l	d1,d0
		and.l	cdb_RemMask(a5),d0
		bne.s	badlength

	; make sure IO_OFFSET + IO_LENGTH are <= MaxOffset

		move.l	d1,d0
		add.l	IO_OFFSET(a3),d0
		cmp.l	cdb_MaxOffset(a5),d0
		bhi.s	badlength

	; calculate # of blocks in transfer if using CRC, or CheckSum

		tst.b	cdb_EDCLength(a5)
		bne.s	multiblock

		moveq	#00,d0
		move.w	d0,d1		;1 block
		rts

multiblock:

	; calc # of blocks


		divu	cdb_BKSZ+2(a5),d1
		subq.w	#1,d1
		moveq	#00,d0		;return success
		rts

badlength:

		moveq	#IOERR_BADLENGTH,d0
		rts


*----i- carddisk.device/findEDC --------------------------------------
*
*   NAME
*	findEDC -- obtain pointer to EDC byte(s)
*   
*   INPUTS
*	a4 = next byte in block after transfer
*	d4 = offset (see setuptran())
*
*   RETURNS
*       a4 = pointer to first EDC byte
*
*   NOTES
*	MUST PRESERVE D0!!!
*
*---------------------------------------------------------------------

findEDC:
	; determine location of error detection data

		move.l	cdb_EDCLOC(a5),d1
		beq.s	gotEDCTable

	; if table, add offset

		move.l	d1,a4	
		add.l	d4,a4		

gotEDCTable:
		rts

*----i- carddisk.device/CMD_WRITE ------------------------------------
*
*   NAME
*	CMD_WRITE -- Wrie data to card memory
*   
*   FUNCTION
*	Transfer data from buffer to card memory
*
*---------------------------------------------------------------------
aDiskWrite:

		moveq	#TDERR_WriteProt,d0
		btst	#CDBB_WRITEPROTECT,cdb_DiskUnitFlags(a5)
		bne.s	badwrite

		BSR_S	checkrequest
		bne.s	badwrite

		tst.w	d1
		beq.s	blockwrite		;1 block?

	; multi-block write

		movem.l	d3-d6,-(sp)

		move.l	cdb_BKSZ(a5),d3
		movem.l	IO_LENGTH(a3),d4-d6	;length, data, offset
		movem.l	d4-d6,-(sp)		;save all

writeloop:
		move.l	d3,IO_LENGTH(a3)
		bsr.s   blockwrite

		sub.l	d3,d4			;decrement length
		beq.s	writeloopdone

		add.l	d3,IO_OFFSET(a3)
		add.l	d3,IO_DATA(a3)

		tst.l	d0			; error during last write?
		beq.s	writeloop

writeloopdone:

		movem.l	(sp)+,d4-d6
		movem.l	d4-d6,IO_LENGTH(a3)	;restore

		movem.l	(sp)+,d3-d6

badwrite:
		rts

blockwrite:

	; single block write - note that this code deals with
	; calculating offsets into the data blocks, adjusting for
	; imbedded error-detection bytes, etc.

		movem.l	d2-d6,-(sp)

		lea	writechkWP(pc),a1
		lea	writeEDC(pc),a0
		bsr	setuptran

	; exg a2 & a4 such that a2 has pointer to card memory (adjusted),
	; and a4 has IO_DATA

		exg	a2,a4

		bsr.s	transfer

		movem.l	(sp)+,d2-d6

		rts

;---------------------------------------------------------------------
;- write checksum, or CRC inside of Begin/EndCardAccess()
;---------------------------------------------------------------------

writeEDC:
		move.l	a2,a4

	; get location of EDC

		bsr	findEDC

		cmp.b	#TPLFMTEDC_CHSUM,d3
		beq.s	WriteCHKSUM

	; if CRC, write 2 bytes (LSB first)

		move.b	d5,(a4)+
		lsr.w	#8,d5
		move.b	d5,(a4)

		bra.s	writechkWP


	; if checksum, write 1 byte

WriteCHKSUM:
		move.b	d5,(a4)

writechkWP:
		btst	#CDBB_WRCHANGE,cdb_DiskUnitFlags(a5)
		beq.s	wroteblock
		moveq	#TDERR_BadSecSum,d0
wroteblock:
		bsr	FlushCache
		rts


*----i- carddisk.device/transfer -------------------------------------
*
*   NAME
*	transfer -- move data
*   
*   INPUTS
*	a6 = execbase (used, and restored)
*	a5 = devicebase
*	a4 = Source
*	a2 = Destination
*	d2 = IO_LENGTH (must be a power of 2 between 128 - 2048)
*	d3 = EDC type
*	d4 = EDC offset
*	d6 = ptr to call back - scratchable
*  RETURNS
*	d5 = checksum, or CRC
*---------------------------------------------------------------------


transfer:

	IFNE	0
		PRINTF	DBG_FLOW,<'CARDDISK -- Transfer %ld bytes from $%lx to $%lx'>,D2,A4,A2
	ENDC
		lea	cdb_CardHandle(a5),a1
		move.l	cdb_CardResource(a5),a6

		JSRLVO	BeginCardAccess

		tst.l	d0
		beq.s	cardremoved

		bset	#CDBB_MOTORON,cdb_DiskUnitFlags(a5)

	;-- assume good transfer

	; select fastest transfer routine possible

	IFNE	TPLFMTEDC_NONE
	FAIL	"tst.b d3 - recode"
	ENDC
		tst.b	d3
		bne.s	EDCIO

	;------------------------------------------------------------
	;-- No error detect transfer
	;------------------------------------------------------------

		move.l	cdb_ExecLib(a5),a6

		move.l	a4,a0
		move.l	a2,a1
		move.l	d2,d0

		JSRLIB	CopyMem

		move.l	cdb_CardResource(a5),a6

transfered:
		moveq	#0,d0		; assume no error

	; call back special r/w handling code

		move.l	d6,a0
		jsr	(a0)

		move.l	d0,d6		;cache return value

		lea	cdb_CardHandle(a5),a1
		JSRLVO	BeginCardAccess

		tst.l	d0
		beq.s	cardremoved

		move.l	d6,d0		;return error code in d0
		bne.s	transferend

	; increment actual count if no error

		move.l	IO_LENGTH(a3),d1
		add.l	d1,IO_ACTUAL(a3)

transferend:
		move.l	cdb_ExecLib(a5),a6	;restore execbase
		rts


	;-- indicate card was removed during I/O

cardremoved:

	;-- the interrupt for card-removal should be coming along
	;-- soon - return error for now

		moveq	#TDERR_DiskChanged,d0
		bra.s	transferend


	;------------------------------------------------------------
	;-- checksum source data
	;------------------------------------------------------------

EDCIO:
		subq.w	#1,d2		;-1 for loop

		moveq	#00,d5		;calc checksum or CRC

	; are we using CheckSum?

		cmp.b	#TPLFMTEDC_CHSUM,d3
		BNE_S	EDCCRC

	PRINTF	DBG_FLOW,<'CARDDISK -- Checksum transfer'>

chksumloop:

		move.b	(a4)+,d0
		move.b	d0,(a2)+

		add.b	d0,d5
		dbf	d2,chksumloop

	PUSHBYTE	D5
	PRINTF	DBG_FLOW,<'CARDDISK -- CHECKSUM = $%lx'>
	POPLONG		1

		BRA_S	transfered

	;------------------------------------------------------------
	;-- CRC source data
	;------------------------------------------------------------

EDCCRC:
	PRINTF	DBG_FLOW,<'CARDDISK -- CRC transfer'>
		move.l	cdb_CRCTable(a5),a0

crcloop:
		move.b	(a4)+,d0
		move.b	d0,(a2)+

	; accum = (accum << 8) ^ crctab[(accum >> 8) ^ data]

		move.w	d5,d1
		lsr.w	#8,d1		; calc index into look up table
		eor.b	d0,d1
		lsl.w	#1,d1		; index * 2

		move.w	0(a0,d1.w),d0

		lsl.w	#8,d5
		eor.w	d0,d5
		dbf	d2,crcloop

	PUSHWORD	D5
	PRINTF	DBG_FLOW,<'CARDDISK -- CRC = $%lx'>
	POPLONG		1

		BRA_S	transfered

*----i- ProtStatus ---------------------------------------------------
*
*  Check to see if write protected
*
*---------------------------------------------------------------------
aDiskProtStatus:
		clr.l	IO_ACTUAL(a3)	;not write-protected

		btst	#CDBB_WRITEPROTECT,cdb_DiskUnitFlags(a5)
		beq.s	nowriteprotect

		addq.l	#1,IO_ACTUAL(a3)

nowriteprotect:

		moveq	#0,d0		;return no error
		rts

*----i- ChangeState --------------------------------------------------
*
*   yes/no card is in drive
*
*---------------------------------------------------------------------
aDiskChangeState:

		clr.l	IO_ACTUAL(a3)

		btst	#CDBB_CARDINSERTED,cdb_DiskUnitFlags(a5)
		bne.s	iscardinslot

		addq.l	#1,IO_ACTUAL(a3)

iscardinslot:
		moveq	#0,d0		;return no error
		rts


*----i- carddisk.device/TD_GETGEOMETRY -------------------------------
*
*   NAME
*	TD_GETGEOMETRY - Get disk geometry
*   
*   FUNCTION
*	Like TD_GETGEOMETRY in trackdisk, except values are more
*	flexable than trackdisk
*
*---------------------------------------------------------------------
aDiskGetGeometry:

		PRINTF	DBG_ENTRY,<'CARDDISK DiskGetGeometry()'>

		move.l	IO_DATA(a3),a0

		move.l	cdb_BKSZ(a5),(a0)+	;sector size (128-2048)
		move.l	cdb_NBLOCKS(a5),(a0)+	;total # of sectors
						;per CISTPL_FORMAT tuple

		move.l	cdb_CYLINDERS(a5),(a0)+	;total number of cylinders
		move.l	cdb_SECPERCYL(a5),(a0)+	;sec/track * track/cyl

		move.l	cdb_TRKPERCYL(a5),(a0)+	;tracks/cyl (heads)
		move.l	cdb_SECPERTRK(a5),(a0)+	;sec/track
		moveq	#MEMF_PUBLIC,d0
		move.l	d0,(a0)+		;preferred bufmemtype

	IFNE	DG_DIRECT_ACCESS
	FAIL	"DG_DIRECT_ACCESS not 0 -- recode"
	ENDC

		moveq	#00,d0			;return no error

		move.b	d0,(a0)+		;DG_DIRECT_ACCESS

		move.b	#DGF_REMOVABLE,(a0)+

		move.w	d0,(a0)			;reserved

	
	IFNE	0

		PRINTF	DBG_FLOW,<'CARDDISK Block Size  = %ld'>,cdb_BKSZ(a5)
		PRINTF	DBG_FLOW,<'CARDDISK # of blocks = %ld'>,cdb_NBLOCKS(a5)
		PRINTF	DBG_FLOW,<'CARDDISK cylinders   = %ld'>,cdb_CYLINDERS(a5)
		PRINTF	DBG_FLOW,<'CARDDISK sec/cyl     = %ld'>,cdb_SECPERCYL(a5)
		PRINTF	DBG_FLOW,<'CARDDISK trk/cyl     = %ld'>,cdb_TRKPERCYL(a5)
		PRINTF	DBG_FLOW,<'CARDDISK sec/trk     = %ld'>,cdb_SECPERTRK(a5)

	ENDC

		rts

*****i* Motor ********************************************************
*
*   The motor is always on, but we will support turning the
*   light on/off
*
**********************************************************************

aDiskMotor:
	PRINTF	DBG_ENTRY,<'CARDDISK -- TD_MOTOR'>

		movem.l	d2/a6,-(sp)

		lea	cdb_CardHandle(a5),a1
		move.l	cdb_CardResource(a5),a6

		move.b	cdb_DiskUnitFlags(a5),d2	;cache

		tst.l	IO_LENGTH(a3)		;0 means off
		bne.s	motoron

		JSRLVO	EndCardAccess
		tst.l	d0
		BEQ_S	nomotor

		bclr	#CDBB_MOTORON,cdb_DiskUnitFlags(a5)

		bra.s	motorset

motoron:
		JSRLVO	BeginCardAccess
		tst.l	d0
		BEQ_S	nomotor

		bset	#CDBB_MOTORON,cdb_DiskUnitFlags(a5)

motorset:
	; d0 == 01 already
	;	moveq	#01,d0			;previous state is on

		btst	#CDBB_MOTORON,d2
		bne.s	gotmotor

		moveq	#00,d0			;previous state is off
gotmotor:
		move.l	d0,IO_ACTUAL(a3)

		moveq	#00,d0			;no error

setmotor:
	PRINTF	DBG_EXIT,<'CARDDISK -- TD_MOTOR IO_ERROR =%ld'>,D0
	PRINTF	DBG_EXIT,<'CARDDISK -- TD_MOTOR IO_ACTUAL =%ld'>,IO_ACTUAL(A3)

		movem.l	(sp)+,d2/a6
		rts

nomotor:
		moveq	#TDERR_DiskChanged,d0
		BRA_S	setmotor

	IFEQ	NORAWIO

*****i* RawRead ******************************************************
*
*   Raw read data bytes - most likely use is to read bytes from
*   CIS for verification during writes.  Bytes read from attribute
*   memory can be done as multi-byte reads returning junk for
*   odd byte addresses, though single byte reads from even byte
*   addresses is recommended.
*
*   NOTE!!!
*
*   Caller MUST own card, and pass in a valid pointer to
*   credit-card common memory, or attribute memory.  The raw
*   r/w functions fail if the device owns the card.
*
*   Note that use of .device raw r/w functions makes it possible
*   to use other .devices if needed in the future which support
*   RAWREAD/RAWWRITE commands like these to r/w a new CIS to/from
*   Flash-Rom's, SRAM cards with EEPROM for attribute memory, EPROMS,
*   etc.  Both Fujitsu, and Panasonic have EEPROM attribute memory
*   options for their SRAM cards.  Of interest, these can actually
*   be formatted as disks even if there is no writeable attribute
*   memory.  There is code for these cases -
*
*   1.) A formatted card which lacks a device tuple as the first
*   tuple in attribute memory.  This works if there is a LINKTARGET
*   tuple starting at mem location 0 in common memory, and the
*   CISTPL_FORMAT tuple is stored in common memory.
*
*   2.) Like #1, but there is a CISTPL_DEVICE tuple in common memory.
*   This is technically not allowed, but I've made it allowable in
*   anticipation of using SRAM cards which lack attribute memory.
*
*   3.) The attribute memory, and common memory overlap, and the
*   CIS has been properly written such that the CIS is stored in
*   even bytes only until a LONGLINK_C tuple is found which continues
*   the chain in contiguous bytes in common memory.  POQET SRAM cards
*   have such overlapped memory, as do Intel Flash-ROMS.
*
**********************************************************************

aDiskRawRead:
		move.l	IO_OFFSET(a3),a0
		move.l	IO_DATA(a3),a1
		bra.s	dorawio

*****i* RawWrite ******************************************************
*
*   Raw write data bytes - most likely use is to write CIS bytes to card.
*   Bytes written to attribute memory can be done as multi-byte writes
*   with junk for odd bytes, though single byte writes to even byte
*   addresses is recommended.
*   NOTE!!!
*
*   Caller MUST own card, and pass in a valid pointer to
*   credit-card common memory, or attribute memory.  The raw r/w
*   commands fail if the device owns the card.  This has been done
*   to prevent problems with someone who thinks they know how to
*   deal with trackdisk, but provides an option to use another
*   device!  Rather than crashing because these commands are being
*   used improperly, we return that these commands are not supported.
*   Of course if they call these functions without checking for
*   disk status change status, oh well...
*
**********************************************************************

aDiskRawWrite:
		move.l	IO_OFFSET(a3),a1
		move.l	IO_DATA(a3),a0

dorawio:
		btst	#CDBB_CARDINSERTED,cdb_DiskUnitFlags(a5)
		bne.s	rawnotallowed

		move.l	IO_LENGTH(a3),d0

		subq.w	#1,d0
rwmem:
		move.b	(a0)+,(a1)+
		dbf	d0,rwmem

                moveq	#00,d0		;always returns TRUE
		rts

rawnotallowed:
		moveq	#IOERR_NOCMD,d0
		rts
	ENDC

		END
