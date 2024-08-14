

*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* user.asm
*
* Source Control
* ------ -------
* 
* $Id: user.asm,v 36.9 91/01/10 18:47:25 jesup Exp $
*
* $Locker:  $
*
* $Log:	user.asm,v $
* Revision 36.9  91/01/10  18:47:25  jesup
* Fixed sectors/cyl in getdrivegeometry now 2*sectors, not sectors+cyls)
* 
* Revision 36.8  90/11/21  04:20:50  jesup
* Changes for multi-density drives (numsecs, etc)
* 
* Revision 36.7  90/06/01  23:18:46  jesup
* Conform to include standard du jour
* 
* Revision 36.6  90/06/01  21:23:08  jesup
* Added GetGeometry
* 
* Revision 36.5  89/04/27  23:40:47  jesup
* fixed autodocs
* 
* Revision 36.4  89/03/18  01:44:52  jesup
* Fixed indexsync, added support for wordsync to raw read/write
* 
* Revision 36.3  89/02/17  19:15:16  jesup
* minor code optimizations
* 
* Revision 36.2  89/01/04  17:05:30  jesup
* Fixed rawread/write - no longer uses random address for size
* 
* Revision 36.1  88/09/14  09:38:04  neil
* fixed a bug found by bryce: in RawCommon(), TDU_NUMTRACKS was being
* compared against a longword, and not a word.
* 
* Revision 32.3  86/02/01  00:46:41  neil
* comment field fixes (sigh -- forgot an "rcs -c"
* 
* Revision 32.2  86/02/01  00:27:38  neil
* added some documentation to the new calls
* 
* Revision 32.1  86/01/26  21:22:39  neil
* *** empty log message ***
* 
* 
*************************************************************************

****** Included Files ***********************************************

	SECTION section

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
	INCLUDE 'exec/strings.i'
	INCLUDE 'exec/errors.i'
	INCLUDE 'exec/memory.i'

	INCLUDE 'hardware/cia.i'

	INCLUDE 'resources/disk.i'

	INCLUDE 'devices/timer.i'

	LIST

	INCLUDE 'trackdisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'

****** Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

	XREF	tdName
	XREF	_custom

*------ Functions ----------------------------------------------------

	XREF	TDMotor
	XREF	TDDelay
	XREF	TDSeek
	XREF	TDNewTrkWrite
	XREF	TDNewTrkRead
	XREF	TDStartDma
	XREF	TDStartDmaSync
	XREF	TDGetUnit
	XREF	TDGiveUnit

	XREF	TermIO

*------ System Library Functions -------------------------------------

	XLIB	SetICR
	XLIB	AbleICR

****** Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	XDEF	TDURawRead
	XDEF	TDURawWrite
	XDEF	TDURawCommon
	XDEF	TDUGetDriveType
	XDEF	TDUGetNumTracks
	XDEF	TDUGetGeometry

*------ Data ---------------------------------------------------------


****** Local Definitions ********************************************


******* trackdisk.device/TD_RAWREAD **************************************
*
*   NAME
*	TD_RAWREAD - read a raw sector from the disk
*
*   FUNCTION
*	This routine performs a raw read for the track disk.
*	It seeks to the specified track and reads it in to the
*	user's buffer.  This buffer MUST be in chip memory.
*
*	NO PROCESSING OF THE TRACK IS DONE.  It will appear exactly
*	as the bits come out off the disk -- hopefully in some legal MFM
*	format (if you don't know what MFM is, you shouldn't be using
*	this call...).  Caveat Programmer.
*
*	This interface is intended for sophisticated programmers
*	only.  Commodore-Amiga may make enhancements to the disk
*	format in the future.  We will provide compatibility
*	within the trackdisk device.  Anyone who uses this routine
*	is bypassing this upwards compatibility.  If your application
*	breaks, TOUGH!
*
*	If this warning is not enough, then add suitable additional
*	harrassment of your choice.
*
*
*   IO REQUEST
*	io_Flags	if the IOTDB_INDEXSYNC bit is set then the driver
*			  will make a best effort attempt to start reading
*			  from the index mark.  Note that there
*			  will be at least some delay, and perhaps a great
*			  deal, of delay (if, for example, interrupts have
*			  been Disabled()...).
*		        if the IOTDB_WORDSYNC bit is set it will use the
*			  WORDSYNC ability to sync on $4489.  Sorry, you
*			  cannot specify the sync pattern.  You may set 
*			  both INDEXSYNC and WORDSYNC, in which case it
*			  will do index sync first, then start a wordsync
*			  read.
*	io_Command	TD_RAWREAD or ETD_RAWREAD.
*	io_Length	Length of buffer (in bytes).  The maximum allowable
*			  length is 32K-1 bytes.
*	io_Data		Pointer to buffer in chip memory where raw track
*			  will be read into.
*	io_Offset	The track number to read in (note this is different
*			  from a normal trackdisk io call which is given
*			  in terms of logical bytes from the beginning of 
*			  the disk.  This is because the trackdisk driver
*			  has no idea what the format of the disk is).
*	iotd_Count	(ETD_RAWREAD only) maximum allowable change counter
*			  value
*
*   RESULTS
*	io_Error	non-zero if there was an error
*
*   LIMITATIONS for synced reads and writes
*	There is a delay between the index pulse and the start of bits
*	coming in from the drive (e.g. dma started).  This delay
*	is in the range of 135-200 micro seconds.  This delay breaks
*	down as follows: 55 microsecs is software interrupt overhead
*	(this is the time from interrupt to the write of the DSKLEN
*	register).  66 microsecs is one horizontal line delay (remember
*	that disk io is synchronized with agnus' display fetches).
*	The last variable (0-65 microsecs) is an additional scan line
*	since DSKLEN is poked anywhere in the horizontal line.  This leaves
*	15 microsecs unaccounted for...  Sigh.
*
*	In short, You will almost never get bits withing the first 135
*	microseconds of the index pulse, and may not get it until 200
*	microseconds.  At 4 microsecs/bit, this works out to be between
*	4 and 7 bytes of user data of delay.
*
*   SEE ALSO
*	TD_RAWWRITE
*
**************************************************************************
*


TDURawRead:
	lea	TDNewTrkRead(pc),a0
	bra.s	TDURawCommon

******* trackdisk.device/TD_RAWWRITE *************************************
*
*   NAME
*	TD_RAWWRITE - write a raw sector to the disk
*
*   FUNCTION
*	NO PROCESSING OF THE TRACK IS DONE.  The disk will appear exactly
*	as the bits come out of memory -- hopefully in some legal MFM
*	format (if you don't know what MFM is, you shouldn't be using
*	this call...).  Caveat Programmer.
*
*	NO PROCESSING OF THE TRACK IS DONE.  It will exactly
*	as the bits come out off the disk.  Caveat Programmer.
*
*	This interface is intended for sophisticated programmers
*	only.  Commodore-Amiga may make enhancements to the disk
*	format in the future.  We will provide compatibility
*	within the trackdisk device.  Anyone who uses this routine
*	is bypassing this upwards compatibility.  If your application
*	breaks, TOUGH!
*
*	If this warning is not enough, then add suitable additional
*	harrassment of your choice.
*
*
*   IO REQUEST
*	io_Flags	if the IOTDB_INDEXSYNC bit is set then the driver
*			  will make a best effort attempt to start writing
*			  from the index mark.  Note that there
*			  will be at least some delay, and perhaps a great
*			  deal, of delay (if, for example, interrupts have
*			  been Disabled()...).
*		        if the IOTDB_WORDSYNC bit is set it will use the
*			  WORDSYNC ability to sync on $4489.  Sorry, you
*			  cannot specify the sync pattern.  You may set 
*			  both INDEXSYNC and WORDSYNC, in which case it
*			  will do index sync first, then start a wordsync
*			  write.
*	io_Command	TD_RAWWRITE or ETD_RAWWRITE.
*	io_Length	Length of buffer (in bytes).  The maximum allowable
*			  length is 32K-1 bytes.
*	io_Data		Pointer to buffer in chip memory where raw track
*			  will be written from.
*	io_Offset	The track number to read in (not this is different
*			  from a normal trackdisk io call which is given
*			  in terms of logical bytes from the beginning of 
*			  the disk.  This is because the trackdisk driver
*			  has no idea what the format of the disk is).
*	iotd_Count	(ETD_RAWWRITE only) maximum allowable change counter
*			  value
*
*   RESULTS
*	io_Error	non-zero if there was an error
*
*   LIMITATIONS for synced reads and writes
*	There is a delay between the index pulse and the start of bits
*	going out to the drive (e.g. write gate enabled).  This delay
*	is in the range of 135-200 micro seconds.  This delay breaks
*	down as follows: 55 microsecs is software interrupt overhead
*	(this is the time from interrupt to the write of the DSKLEN
*	register).  66 microsecs is one horizontal line delay (remember
*	that disk io is synchronized with agnus' display fetches).
*	The last variable (0-65 microsecs) is an additional scan line
*	since DSKLEN is poked anywhere in the horizontal line.  This leaves
*	15 microsecs unaccounted for...  Sigh.
*
*	In short, You will almost never get bits withing the first 135
*	microseconds of the index pulse, and may not get it until 200
*	microseconds.  At 4 microsecs/bit, this works out to be between
*	4 and 7 bytes of user data of delay.
*
*   SEE ALSO
*	TD_RAWREAD
*
**************************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- TDLib
*	A5 -- local storage
*	A4 -- ptr to sector in track buffer
*	A3 -- unit data
*	A2 -- IOBlock
*

TDURawWrite:
	lea	TDNewTrkWrite(pc),a0

	;------ fall through...
RAWREGS		REG	d2/d3/a2/a3/a4
INDEXBUFFSIZE	EQU	16

TDURawCommon:
	movem.l	RAWREGS,-(sp)

	link	a5,#-INDEXBUFFSIZE

	move.l	a1,a2
	move.l	a0,a4

	moveq.l	#1,d0
	bsr	TDMotor

	;------ move a longword into two different words
	move.l	IO_OFFSET(a2),d0
	cmp.w	TDU_NUMTRACKS(A3),d0	; range check the lower word
	bcc.s	RawCommon_LengthErr

	;------ make sure the higher word is null
	move.l	d0,d1
	swap	d1
	tst.w	d1
	bne.s	RawCommon_LengthErr

	;------ seek to the right track
	bsr	TDSeek
	move.b	d0,IO_ERROR(a2)
	bne.s	RawCommon_TermIO

	;------ range check the length
	move.l	IO_LENGTH(a2),D0
	cmp.l	#$7fff,D0	;FIXED-bryce:Changed to absolute #$8000
				;rather than memory location $8000 :-)
				;changed to $7fff since $8000 will set the
				;write bit and request 0 bytes! REJ
	bcc.s	RawCommon_LengthErr

	;------ see if we are trying to start at index
	lea	TDStartDma(PC),A1
	btst	#IOTDB_WORDSYNC,IO_FLAGS(a2)
	beq.s	RawCommon_NoWordSync
	lea	TDStartDmaSync(PC),A1

RawCommon_NoWordSync:
	btst	#IOTDB_INDEXSYNC,IO_FLAGS(a2)
	beq.s	RawCommon_StartIO

	;------ allocate temp storage on stack
	lea	-INDEXBUFFSIZE(a5),a0

	;------ Save dma start pointer and unit pointer
	movem.l	a1/a3,4(a0)

	;------ set the interrupt data/code vectors
	lea	RawIndexInt(pc),A1
	movem.l	a0/a1,TDU_DRU+DRU_INDEX+IS_DATA(a3)

	;------ the start dma routine
	lea	RawSyncIndex(pc),A1

RawCommon_StartIO:
	move.l	IO_DATA(a2),A0
	jsr	(a4)
	move.b	d0,IO_ERROR(a2)

RawCommon_TermIO:
	move.l	a2,a1
	bsr	TermIO

RawCommon_End:
	unlk	a5
	movem.l	(sp)+,RAWREGS
	rts

RawCommon_LengthErr:
	move.b	#IOERR_BADLENGTH,IO_ERROR(a2)
	bra.s	RawCommon_TermIO

; called just before starting disk dma if we are to sync with the index pulse
RawSyncIndex:	; (length: d0)

	;------ save the disk length register value size 
	;------ note ptr to routine is already saved at 4(a0)
	move.l	TDU_DRU+DRU_INDEX+IS_DATA(a3),a0
	move.l	d0,(a0)
	move.l	a6,12(a0)	;-- so we can use movem later

	;------ clear out any old index pulses
	moveq	#$10,d0
	LINKLIB _LVOSetICR,TD_CIABRESOURCE(a6)

	;------ enable the index interrupts
	move.l	#$90,d0
	LINKLIB _LVOAbleICR,TD_CIABRESOURCE(A6)

	;------ and return to the track io routine
	RTS

RawIndexInt:
	movem.l	a3/a6,-(sp)
	movem.l	(a1),d0/a0/a3/a6	; a1 has pointer from IS_DATA
	lea	_custom,A1
	jsr	(a0)			; TDStartDMA needs a3 set up

	;------ turn off index interrupt
	move.l	TD_CIABRESOURCE(a6),a0	; a6 has TDPtr (device pointer)

	moveq	#$10,d0
	LINKLIB	_LVOAbleICR,a0

	movem.l	(sp)+,a3/a6
	rts


******* trackdisk.device/TD_GETDRIVETYPE *********************************
*
*   NAME
*       TD_GETDRIVETYPE - return the type of the disk drive to the user
*
*   FUNCTION
*	This routine returns the type of the disk to the user.
*	This number will be a small integer.  It will come from
*	the set of DRIVE... defines in trackdisk.h
*	or trackdisk.i.
*
*	The only way you can get to this call is if the trackdisk
*	device understands the drive type of the hardware that is
*	plugged in.  This is because the OpenDevice call will fail
*	if the trackdisk device does not understand the drive type.
*	To find raw drive identifiers see the disk resource's
*	DR_GETUNITID entry point.
*
*   IO REQUEST
*	io_Command	TD_GETDRIVETYPE
*
*   RESULTS
*	io_Actual	the drive type connected to this unit.
*
*   SEE ALSO
*	TD_GETNUMTRACKS
*
**************************************************************************
*
*   IMPLEMENTATION NOTES
*

TDUGetDriveType:
		CLEAR	D0
		MOVE.B	TDU_DRIVETYPE(A3),D0
		MOVE.L	D0,IO_ACTUAL(A2)

		MOVE.L	A2,A1
		BRA	TermIO		; was BSR

*		RTS

******* trackdisk.device/TD_GETNUMTRACKS *********************************
*
*   NAME
*       TD_GETNUMTRACKS - return the number of tracks on this type of disk
*
*   FUNCTION
*	This routine returns the number of tracks that are available
*	on this disk unit.  This call obsoletes the older NUMTRACKS
*	hard coded constant.
*
*   IO REQUEST
*	io_Command	TD_GETNUMTRACKS
*
*   RESULTS
*	io_Actual	number of tracks accessible on this unit
*
*   SEE ALSO
*	TD_GETDRIVETYPE
*
**************************************************************************
*
*   IMPLEMENTATION NOTES
*

TDUGetNumTracks:
		CLEAR	D0
		MOVE.W	TDU_NUMTRACKS(A3),D0
		MOVE.L	D0,IO_ACTUAL(A2)

		MOVE.L	A2,A1
		BRA	TermIO		; was BSR

*		RTS

******* trackdisk.device/TD_GETGEOMETRY *********************************
*
*   NAME
*       TD_GETGEOMETRY - return the geometry of the drive
*
*   FUNCTION
*	This routine returns the a full set of information about the
*	layout of the drive.
*
*   IO REQUEST
*	io_Command	TD_GETGEOMETRY
*	io_Data		Pointer to a struct DriveGeometry
*	io_Length	sizeof(struct DriveGeometry)
*
*   RESULTS
*	io_Error	non-zero on error
*
*   SEE ALSO
*	TD_GETDRIVETYPE, TD_GETNUMTRACKS
*
**************************************************************************
*
*   IMPLEMENTATION NOTES
*

TDUGetGeometry
		move.l	d2,-(sp)
		move.l	IO_DATA(a2),a0

		move.l	#512,(a0)+	; sector size
		moveq	#0,d0
		move.w	TDU_NUMTRACKS(a3),d0
		move.w	TDU_NUMSECS(A3),d2
		move.l	d0,d1
		mulu	d2,d1
		move.l	d1,(a0)+	; total sectors
		asr.w	#1,d0		; two heads
		move.l	d0,(a0)+	; cylinders
		move.l	d2,(a0)		; cylblocks
		add.l	d2,(a0)+	; (2*numsecs)
		moveq	#2,d0		; heads
		move.l	d0,(a0)+
		move.l	d2,(a0)+	; sectors per track
		moveq	#MEMF_PUBLIC,d0	; preferred bufmemtype
		move.l	d0,(a0)+
		clr.b	(a0)+		; DG_DIRECT_ACCESS
		moveq	#DGF_REMOVABLE,d0
		move.b	d0,(a0)+
		clr.w	(a0)		; reserved
		move.l	(sp)+,d2	; FIX! do we need to restore this reg?

		MOVE.L	A2,A1
		BRA	TermIO		; was BSR

*		RTS

	END
