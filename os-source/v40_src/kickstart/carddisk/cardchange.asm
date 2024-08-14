**
**	$Id: cardchange.asm,v 1.5 92/12/04 19:12:58 darren Exp $
**
**	Credit card device -- task/interrupts -- detect card change 
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

	INCLUDE	"devices/trackdisk.i"

	INCLUDE	"hardware/intbits.i"

	INCLUDE	"dos/dostags.i"

	INCLUDE	"utility/tagitem.i"

	INCLUDE "internal/librarytags.i"

	INCLUDE	"cardddata.i"
	INCLUDE	"debug.i"

	LIST

** Exports

	XDEF	CDDTaskStart

	XDEF	HandleInsert
	XDEF	HandleRemoved
	XDEF	HandleStatus
	XDEF	CheckRemoved

** Externals

*---------------------------------------------------------------------
		CNOP	0,4
CDDTaskStart:
		;--	grab the device base

		move.l	4(sp),a5

	PRINTF	DBG_ENTRY,<'CARDDISK - Task Start - Data @ %lx'>,A5

		lea	cdb_CardHandle(a5),a2
		lea	cdb_buffer(a5),a3
		lea	cdb_ddata(a5),a4
		move.l	cdb_ExecLib(a5),a6

	; make sure signal bit is allocated

		moveq	#CDDB_SIGTASK,d0
		JSRLIB	AllocSignal

		;-- wait for a card to examine

taskloop:
		move.l	#CDDF_SIGTASK,d0

		JSRLIB	Wait

	PRINTF	DBG_FLOW,<'CARDDISK - Task wakes'>

	;
	; We ObtainSemaphore() so that -
	;
	; o In progress I/O commands fully complete before we notify
	;   handlers of card removal/insertion.
	;
	; o In progress I/O commands which add/remove handlers to
	;   notification lists block while walking the lists (and vice-versa)
	;
	; o In progress I/O commands block while we are handling
	;   new card insertion examination.
	;
	; o In progress I/O commands are blocked while this task is
	;   modifying unit related variables.
	;


		lea	cdb_SSemaphore(a5),a0
		JSRLIB	ObtainSemaphore


	; check for card removal

		bsr.s	CheckRemoved

		bclr	#CDBB_NEWCARD,cdb_DiskUnitFlags(a5)
		beq.s	notnewcard

		bsr	ExamineNewCard

notnewcard:

	; check for change in write-protect status (will be cleared
	; if card is removed).

		bclr	#CDBB_WRCHANGE,cdb_DiskUnitFlags(a5)
		beq.s	notwrchange

	; notify of change as if card went in, and out, but do not release
	; ownership of card

		bsr.s	ReadWPStatus

	; increment change count (2 times really - in, and out)

		addq.l	#1,cdb_ChangeCount(a5)

		BSR_S	NotifyChanged


notwrchange:
		move.l	cdb_ExecLib(a5),a6
		lea	cdb_SSemaphore(a5),a0
		JSRLIB	ReleaseSemaphore

		BRA_S	taskloop

*----i- carddisk.device/ReadWPStatus ----------------------------------
*
*    NAME
*	ReadWPStatus -- Read write-protect status
*
*    FUNCTION
*	As noted above.
*
*    INPUTS
*	a6 = execbase
*	a5 = device base
*    NOTES
*	Called within Obtain/Release Semaphore
*
*---------------------------------------------------------------------
ReadWPStatus:

	; assume card is write-protected, or not writeable

		bset	#CDBB_WRITEPROTECT,cdb_DiskUnitFlags(a5)

		btst	#CDBB_WRITEABLE,cdb_DiskUnitFlags(a5)
		beq.s	notwriteable

		move.l	cdb_CardResource(a5),a6
		JSRLVO	ReadCardStatus

		btst	#CARD_STATUSB_WR,d0
		beq.s	notwriteable

		bclr	#CDBB_WRITEPROTECT,cdb_DiskUnitFlags(a5)

notwriteable:
		move.l	cdb_ExecLib(a5),a6
		rts

*----i- carddisk.device/CheckRemoved --------------------------------
*
*    NAME
*	CheckRemoved -- Check to see if card has been removed
*
*    FUNCTION
*	As noted above.
*
*    INPUTS
*	a6 = execbase
*	a5 = device base
*	d2 = may be scratched (preserved by caller)
*    NOTES
*	Called within Obtain/Release Semaphore
*
*---------------------------------------------------------------------

CheckRemoved:

	PRINTF	DBG_ENTRY,<'CARDDISK - CheckRemoved()'>

	; has card been removed?

		bclr	#CDBB_CARDREMOVED,cdb_DiskUnitFlags(a5)
		beq.s	notinserted

	; also clear any pending notification of WP change

		bclr	#CDBB_WRCHANGE,cdb_DiskUnitFlags(a5)

	PRINTF	DBG_FLOW,<'CARDDISK - Removed INT noticed'>

		bclr	#CDBB_CARDINSERTED,cdb_DiskUnitFlags(a5)
		beq.s	notinserted

		bsr.s	NotifyChanged

	; force ExamineNewCard() to test to see we own the card
	; in the slot, and to perform a release if not (which we
	; we probably won't)  May have already been set if we
	; took the removed int before the inserted interrupt

		bset	#CDBB_NEWCARD,cdb_DiskUnitFlags(a5)

notinserted:

	PRINTF	DBG_ENTRY,<'CARDDISK - Exit CheckRemoved()'>

		rts
*----i- carddisk.device/NotifyChanged --------------------------------
*
*    NAME
*	NotifyChanged -- Cause() software interrupt for all handlers
*
*    FUNCTION
*	As noted above.  This code can be called within BeginIO, or
*	on the context of my own task IF there is no device IO
*	in progress.
*
*    INPUTS
*	a6 = execbase
*	a5 = device base
*	d2 = may be scratched (preserved by caller)
*    NOTES
*	Called within Obtain/Release Semaphore
*
*---------------------------------------------------------------------

NotifyChanged:

	PRINTF	DBG_ENTRY,<'CARDDISK - NotifyRemoved()'>

	; increment change count

		addq.l	#1,cdb_ChangeCount(a5)

	; notify the one REMOVE interrupt (if we have one at all)

		move.l	cdb_ExecLib(a5),a6	;use execbase

		move.l	cdb_RemoveInt(a5),d0
		beq.s	SSI_List
		move.l	d0,a1

	PRINTF	DBG_OSCALL,<'CARDDISK - Cause() /RemoveInt/'>,A1

		JSRLIB	Cause

	; notify everyone on ADDCHANGEINT list

SSI_List:
		move.l	cdb_ChangeInt(a5),d2
SSI_Loop:
		move.l	d2,a1
		move.l	(a1),d2
		beq.s	SSI_End

		move.l	IO_DATA(a1),a1

	PRINTF	DBG_OSCALL,<'CARDDISK - Cause() /ChangeInt/'>,A1

		JSRLIB	Cause
		bra.s	SSI_Loop
SSI_End

	PRINTF	DBG_ENTRY,<'CARDDISK - Exit NotifyRemoved()'>

		rts

*----i- carddisk.device/ExamineNewCard --------------------------------
*
*    NAME
*	ExamineNewCard -- look for formatting info
*
*    FUNCTION
*	
*	The card is considered to be a disk if it meets these
*	requirments -
*
*	o Has a CISTPL_DEVICE tuple.
*
*	o The CISTPL_DEVICE tuple has a TYPE field of
*		ROM		(read only)
*		OTPROM		(read only)
*		EPROM		(read only)
*		EEPROM		(read only)
*		FLASH		(read only)
*		SRAM		(read and write if write enabled)
*		DRAM		(read/write -- no data retention)
*
*		IO, and EXTENDED are not accepted.
*
*	o Has a CISTPL_FORMAT tuple.
*
*	o The FORMAT tuple has a TPLFMT_TYPE field indicating
*	  this is a DISK LIKE format.
*
*	  MEM LIKE, RESERVED, and VENDOR-SPECIFIC are NOT accepted. 
*
*	o The FORMAT tuple can have an EDC (Error Detection Code) type
*	  of NONE, CKSUM, CRC, or PCC.
*
*	  RESERVED, and VENDOR-SPECIFIC methods of error checking
*	  are NOT accepted
*	
*	  PCC is actually ignored for now (since it is intended for
*	  checking an entire partition on a ROM, or OTPROM card - we
*	  will assume its correct, and handle it as if the EDC type
*	  is NONE).
*
*	  The RFU (reserved for future) bit MUST be 0.
*
*	o The OFFSET, and SIZE are assumed to be correct, though I do
*	  make an attempt to verify that the are consistent with the
*	  size as specified in the DEVICE tuple.
*
*	o The tuple must include the block size, and number of
*	  blocks, though EDCLOC is optional (e.g., HP Palm Top does
*	  not have it).
*
*	o We come up with a reasonable geometry if there is
*	  no CISTPL_GEOMETRY tuple present.
*
*	o We assume the GEOMETRY tuple if found is correct;
*	  we've pretty much verified that the CIS isn't too
*	  corrupt by the time we look for a GEOMETRY tuple.
*
*---------------------------------------------------------------------
ExamineNewCard:

		move.l	cdb_CardResource(a5),a6		;use resource base

	PRINTF	DBG_FLOW,<'CARDDISK - Newly inserted card'>


		move.l	a2,a1
		move.l	a3,a0
		moveq	#CISTPL_DEVICE,d1
		moveq	#(TP_Device_MINSIZEOF-TUPLE_SIZEOF),d0
		
		moveq	#-1,d4				;card is big

		bset	#CDBB_WRITEABLE,cdb_DiskUnitFlags(a5)

	; assume SRAM if no CISTPL_DEVICE tuple for compatability
	; with pseudo-floppies, like an HP using a Poquet
	; card which does not check for overlapped common,
	; and attribute memory when writing the CIS

		JSRLVO	CopyTuple
		tst.l	d0
		BEQ_S	assumeSRAM

	PRINTF	DBG_FLOW,<'CARDDISK - Found CISTPL_DEVICE tuple'>

		move.l	a3,a0
		move.l	a4,a1

		JSRLVO	DeviceTuple

		tst.l	d0
		beq	freethiscard

	PRINTF	DBG_FLOW,<'CARDDISK - Size = $%lx'>,D0

		move.l	d0,d4		;cache

	IFNE	DTYPE_NULL
	FAIL	"beq.s	freethiscard - recode"
	ENDC

		move.b	dtd_DTtype(a4),d1
		beq	freethiscard	;DTYPE_NULL?

		cmp.b	#DTYPE_DRAM,d1
		bhi	freethiscard	;I/O card, or reserved?

	PUSHBYTE	D1
	PRINTF	DBG_FLOW,<'CARDDISK - Device type = $%lx'>
	POPLONG		1

	;-- force write-protected for all cards except SRAM, and DRAM


		cmp.b	#DTYPE_SRAM,d1
		beq.s	cardisram
		cmp.b	#DTYPE_DRAM,d1
		beq.s	cardisram

	;-- ROM, FLASH-ROM, EPROM, etc. is always considered
	;-- write-protected, regardless of WP switch state

		bclr	#CDBB_WRITEABLE,cdb_DiskUnitFlags(a5)

cardisram:
		
	;-- Set best access speed

		move.l	a2,a1
		move.l	dtd_DTspeed(a4),d0
		JSRLVO	CardAccessSpeed

	;-- Ignore return, we can catch card removal later, and
	;-- if the card is really slower than what we can handle,
	;-- i/o will be slow - shouldn't be a major problem, especially
	;-- since we managed to get through reading a CISTPL_DEVICE
	;-- tuple without tripping (e.g., a card which does not force
	;-- a wait-state if it needs it during CPU access).

assumeSRAM:
	;-- look for a format tuple

		move.l	a2,a1
		move.l	a3,a0
		moveq	#CISTPL_FORMAT,d1
		moveq	#(TP_Format_SIZEOF-TPL_LINK),d0
		
		JSRLVO	CopyTuple
		tst.l	d0
		beq	freethiscard

	PRINTF	DBG_FLOW,<'CARDDISK - Found CISTPL_FORMAT tuple'>

	;-- determine if length of tuple is at least $12 bytes long

		lea	TPL_LINK(a3),a1

		move.b	(a1)+,d6

	; must have data up through NBLOCKS, EDC is optional

		cmp.b	#(TPLFMT_EDCLOC-TPL_LINK+1),d6
		bcs	freethiscard

	PRINTF	DBG_FLOW,<'CARDDISK - CISTPL_FORMAT Link OK'>

	IFNE	TPLFMTTYPE_DISK
	FAIL	"tst.b	(a1) -- recode"
	ENDC

		tst.b	(a1)	
		bne	freethiscard

	PRINTF	DBG_FLOW,<'CARDDISK - TYPE == DISK LIKE'>

		addq.l	#1,a1
		move.b	(a1)+,d0
		move.b	d0,d1			;cache

		lsr.b	#TPLFMT_EDC_SHIFT,d0
		cmp.b	#TPLFMTEDC_PCC,d0

	; also checks for RFU bit (high bit) which should be 0

		bhi	freethiscard
		bne.s	cacheEDCtype

	; if TPLFMTEDC_PCC (static data on ROM's or OTPROM), we won't
	; bother checksumming the entire partition

		moveq	#TPLFMTEDC_NONE,d0
cacheEDCtype:
		move.b	d0,cdb_ErrorDetect(a5)

	PUSHBYTE	D0
	PRINTF	DBG_FLOW,<'CARDDISK - Error Detection Type = %ld'>
	POPLONG		1

		and.b	#TPLFMT_LENGTH_MASK,d1
		move.b	d1,cdb_EDCLength(a5)

	PUSHBYTE	D1
	PRINTF	DBG_FLOW,<'CARDDISK - EDC Size = %ld'>
	POPLONG		1

	; obtain offset into common memory for start of partition

		bsr	makelong

		move.l	d0,d2			;cache offset start

		add.l	cdb_MapCard(a5),d0
		move.l	d0,cdb_DataStart(a5)

	PRINTF	DBG_FLOW,<'CARDDISK - Data start @ $%lx'>,D0

	; obtain total size of partition (including any EDC bytes)

		bsr	makelong

		add.l	d0,d2			;offset + size
		cmp.l	d4,d2
		bhi	freethiscard		;validity check
		
	PRINTF	DBG_FLOW,<'CARDDISK - Offset + Size <= Size of card'>,D2

	; get block size

		move.b	(a1)+,cdb_BKSZ+3(a5)	;LSB
		move.b	(a1)+,cdb_BKSZ+2(a5)	;HSB

	; get number of blocks

		bsr	makelong

		move.l	d0,cdb_NBLOCKS(a5)

	; calc max offset (multiply won't work [overflow] - could have used
	; utility library, but this shifting stuff is a bit smaller since
	; its the only place I would have needed utility.library)

		move.w	cdb_BKSZ+2(a5),d1	;block size
		lsr.w	#1,d1
		beq	freethiscard

		moveq	#-1,d2			;and a mask for fast mul/div
calcmaxoffset:
		lsl.l	#1,d0
		lsl.l	#1,d2
		lsr.w	#1,d1
		bcc.s	calcmaxoffset

		move.l	d0,cdb_MaxOffset(a5)

	PRINTF	DBG_FLOW,<'CARDDISK - Max Offset = $%lx'>,D0

	; masks for fast mul/div stuff

		not.l	d2
		move.l	d2,cdb_RemMask(a5)

	PRINTF	DBG_FLOW,<'CARDDISK - Remainder Mask = $%lx'>,D2

	; assume no EDC table to start (this field is optional per
	; specification if there is no EDC table)

		moveq	#00,d0

		cmp.b	#(TP_Format_SIZEOF-TPL_LINK-1),d6
		bcs.s	noEDCLOC

		bsr	makelong

	; if EDCLOC is null, there is no table

		tst.l	d0
		beq.s	noEDCLOC

	; else add offset to common memory

		add.l	cdb_MapCard(a5),d0
noEDCLOC:
		move.l	d0,cdb_EDCLOC(a5)


	PRINTF	DBG_FLOW,<'CARDDISK - block size = %ld'>,cdb_BKSZ(a5)
	PRINTF	DBG_FLOW,<'CARDDISK - # of blocks = %ld'>,cdb_NBLOCKS(a5)
	PRINTF	DBG_FLOW,<'CARDDISK - EDCLOC = $%lx'>,cdb_EDCLOC(a5)

	PRINTF	DBG_FLOW,<'CARDDISK - We have a DISK!!'>

	; set-up default geometry if there is not CISTPL_GEOMETRY tuple.
	; Calculated based on a reasonable geometry of 1 head, and
	; 8 sectors per track.

	; changed to use linear geometry to maximize use of cards without
	; geometry tuples

		move.l	cdb_NBLOCKS(a5),d0
	;	lsr.l	#3,d0		;1 tracks @ 8 secs/track = 8
		move.l	d0,cdb_CYLINDERS(a5)

		moveq	#1,d0

		move.l	d0,cdb_SECPERTRK(a5)
		move.l	d0,cdb_SECPERCYL(a5)
		move.l	d0,cdb_TRKPERCYL(a5)

	; look for a real CISTPL_GEOMETRY tuple

		move.l	a2,a1
		move.l	a3,a0
		moveq	#CISTPL_GEOMETRY,d1
		moveq	#(TP_Geometry_SIZEOF-TUPLE_SIZEOF),d0
		
		JSRLVO	CopyTuple
		tst.l	d0
		beq.s	notifynewcard


	PRINTF	DBG_FLOW,<'CARDDISK - Found CISTPL_GEOMETRY tuple'>

		lea	TPLGEO_SPT(a3),a0

		moveq	#0,d0

	; copy sectors per track to lower word of long

		move.b	(a0)+,d0
		move.l	d0,cdb_SECPERTRK(a5)
		move.w	d0,d1				;cache

	; copy tracks per cyl to lower word of long

		move.b	(a0)+,d0
		move.l	d0,cdb_TRKPERCYL(a5)

	; copy number of cylinders to lower word of long (value
	; is in little-endian order).

		clr.w	cdb_CYLINDERS(a5)		;clear upper word
		move.b	(a0)+,cdb_CYLINDERS+3(a5)	;LSB
		move.b	(a0),cdb_CYLINDERS+2(a5)	;HSB

	; calc sectors per cylinder

		mulu	d1,d0				;sec/trk * trk/cyl
		move.l	d0,cdb_SECPERCYL(a5)

	; notify all handlers

notifynewcard:

	PRINTF	DBG_FLOW,<'CARDDISK - total cylinders = %ld'>,cdb_CYLINDERS(a5)
	PRINTF	DBG_FLOW,<'CARDDISK - sector/track  = %ld'>,cdb_SECPERTRK(a5)
	PRINTF	DBG_FLOW,<'CARDDISK - track/cyl = %ld'>,cdb_TRKPERCYL(a5)
	PRINTF	DBG_FLOW,<'CARDDISK - sec/cyl = %ld'>,cdb_SECPERCYL(a5)

	; grab write-protect status

		bsr	ReadWPStatus

		move.l	cdb_ExecLib(a5),a6		;use execbase

	; create a CRC table if needed

		move.b	cdb_ErrorDetect(a5),d0
		cmp.b	#TPLFMTEDC_CRC,d0
		bne.s	notusingCRC

		move.l	cdb_CRCTable(a5),d0
		bne.s	notusingCRC			;already have a table?

	; make table if memory permits

		
		move.l	#512,d0				;table size
		moveq	#MEMF_PUBLIC,d1
		JSRLIB	AllocMem

		tst.l	d0
		beq.s	freethiscard	;ACK??? - memory low, ignore card

		move.l	d0,cdb_CRCTable(a5)

	PRINTF	DBG_FLOW,<'CARDDISK - CRC Table @ $%lx'>,D0

		move.l	d0,a0
 		moveq	#00,d1		;loop 0-255 

make_outer_crc:
		moveq	#00,d4		;accumulator

		move.w	d1,d6		;data
		lsl.w	#8,d6

		moveq	#7,d2		;loop 8x

make_inner_crc:
		move.w	d4,d0

		lsl.w	#1,d4		;always shift

		eor.w	d6,d0		;if(data ^ accum) & 0x8000)
		bpl.s	msbnotset

		eor.w	#$1021,d4	;polynomial x^16+x^12+x^5+1

msbnotset:
		lsl.w	#1,d6		;move up next bit for EOR
		dbf	d2,make_inner_crc

		move.w	d4,(a0)+	;store
		addq.b	#1,d1
		bne.s	make_outer_crc	;loop 0-255

notusingCRC:

	; mark that we have an inserted card

		bset	#CDBB_CARDINSERTED,cdb_DiskUnitFlags(a5)

	; start file system if needed

		bsr.s	startFS

	; and notify any handlers

		bsr	NotifyChanged
		rts


freethiscard:

	PRINTF	DBG_FLOW,<'CARDDISK - Releasing card'>

	; access light turn off is implied when we release a card

		bclr	#CDBB_MOTORON,cdb_DiskUnitFlags(a5)

		move.l	a2,a1
		moveq	#00,d0
		JSRLVO	ReleaseCard
		rts



;--
;-- Move next 4 (LSB first) bytes into D0 (converted to HSB first)
;--
makelong:
		moveq	#03,d1

makelongl:
		move.b	(a1)+,d0
		ror.l	#8,d0
		dbf	d1,makelongl
		rts

;--
;-- Start FileSystem if DOS has already started, else mark that we would
;-- like it started

startFS:
		bset	#CDBB_STARTCC0,cdb_DiskUnitFlags(a5)
		BNE_S	FSStarted

	PRINTF	DBG_FLOW,<'First insertion'>

		move.l	cdb_ExpanLib(a5),d0
		BEQ_S	FSStarted
		move.l	d0,a0

	; if not set yet, DOS has not been started

	PUSHBYTE 	eb_Flags(a0)
	PRINTF	DBG_FLOW,<'CARDDISK -- eb_Flags $%lx'>
	POPLONG         1

		bset	#EBB_START_CC0,eb_Flags(a0)
		BEQ_S	FSStarted

	; Already set, so start a small DOS process to wake FS

		BSR_S	openDOS
		BEQ_S	FSStarted	; ?? what ??

	PRINTF	DBG_FLOW,<'CARDDISK -- Start DOS Process'>

		lea	dostags(pc),a0
		move.l	a0,d1
		JSRLIB	CreateNewProc

	PRINTF	DBG_FLOW,<'CARDDISK -- End DOS Process'>

closeDOS:
		move.l	a6,a1
		move.l	$4,a6
		JSRLIB	CloseLibrary

FSStarted:
		rts

openDOS:
		moveq	#OLTAG_DOS,d0
		JSRLIB	TaggedOpenLibrary
		move.l	d0,a6
		tst.l	d0
		rts

	; wake FileSystem

dosDevProc:

		movem.l	d2/a6,-(sp)

	PRINTF	DBG_ENTRY,<'DOS Process'>

		move.l	$4,a6
		bsr.s	openDOS
		beq.s	noDOS		;??what??

		lea	ccname(pc),a0
		move.l	a0,d1
		moveq	#00,d2		;NULL
		JSRLIB	GetDeviceProc

		move.l	d0,d1
		JSRLIB	FreeDeviceProc

		BSR_S	closeDOS
noDOS:
		
		movem.l	(sp)+,d2/a6
		rts

dostags:
		dc.l	NP_Entry
		dc.l	dosDevProc
		dc.l	TAG_DONE

ccname:
		dc.b	'CC0:',0

		CNOP	0,2

*----i- carddisk.device/HandleRemoved ---------------------------------
*
*    NAME
*	HandleRemoved -- handle credit-card removal
*
*    FUNCTION
*	Interrupt - set bit, and wake task.
*
*---------------------------------------------------------------------

HandleRemoved:

	PRINTF	(DBG_INTERRUPT+DBG_ENTRY),<'CARDDISK - HandleRemoved($%lx)'>,A1

		bset	#CDBB_CARDREMOVED,cdb_DiskUnitFlags(a1)

		bra.s	waketask

*----i- carddisk.device/HandleInsert ---------------------------------
*
*    NAME
*	HandleInsert -- handle credit-card insertion
*
*    FUNCTION
*	Interrupt - set bit, and wake task
*
*---------------------------------------------------------------------

HandleInsert:

	PRINTF	(DBG_INTERRUPT+DBG_ENTRY),<'CARDDISK - HandleInsert($%lx)'>,A1

		bset	#CDBB_NEWCARD,cdb_DiskUnitFlags(a1)
waketask:
		move.l	cdb_ExecLib(a1),a6
		lea	cdb_TC(a1),a1
		move.l	#CDDF_SIGTASK,d0
		JSRLIB	Signal

	PRINTF	(DBG_INTERRUPT+DBG_ENTRY),<'CARDDISK - Task signaled'>

		rts

*----i- carddisk.device/HandleStatus ---------------------------------
*
*    NAME
*	HandleStatus -- handle credit-card status changes
*
*    FUNCTION
*	Interrupt - set bit, and wake task
*
*---------------------------------------------------------------------

HandleStatus

	PRINTF	(DBG_INTERRUPT+DBG_ENTRY),<'CARDDISK - HandleStatus($%lx)'>,A1

		btst	#CARD_STATUSB_WR,d0
		beq.s	nostatuschange

		move.l	d0,-(sp)

		bset	#CDBB_WRCHANGE,cdb_DiskUnitFlags(a1)
		BSR_S	waketask

		move.l	(sp)+,d0	;return mask
nostatuschange:
		rts

	END

