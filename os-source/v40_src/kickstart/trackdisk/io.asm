
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* io.asm
*
* Source Control
* ------ -------
* 
* $Id: io.asm,v 33.17 92/06/02 22:32:22 jesup Exp $
*
* $Locker:  $
*
* $Log:	io.asm,v $
* Revision 33.17  92/06/02  22:32:22  jesup
* Make unreadable tracks return TDERR_NoSecHdr instead of retry-count + 1
* 
* Revision 33.16  91/04/28  23:39:05  jesup
* Added commented-out code for protection checking on write
* 
* Revision 33.15  91/03/13  20:42:52  jesup
* Check for entire track read errors, added an ALLBAD flag.
* Removed TDMotor calls from protect.
* Fixed CMD_START
* and CMD_STOP
* 
* Revision 33.14  91/01/17  15:05:27  jesup
* Add bit to make it not try to r/w a disk
* 
* Revision 33.13  90/11/28  23:41:45  jesup
* TDB_DATA now a variable
* 
* Revision 33.12  90/11/21  04:12:27  jesup
* Changes for variable density
* 
* Revision 33.11  90/06/01  23:15:43  jesup
* Conform to include standard du jour
* 
* Revision 33.10  90/06/01  16:28:49  jesup
* comment changes
* 
* Revision 33.9  90/03/16  00:58:28  jesup
* Don't realign track if already aligned.  Adjust write length if the buffer is
* "OFFSET_2".  Add comment.
* 
* Revision 33.8  89/12/10  18:32:25  jesup
* uncommented the use of cpu xfers (oops), added TDUStop/Start.
* 
* Revision 33.7  89/09/06  18:53:07  jesup
* Changed retry logic slightly
* 
* Revision 33.6  89/05/15  21:17:52  jesup
* fixed comments in Getunit usage
* 
* Revision 33.5  89/04/27  23:38:03  jesup
* fixed autodocs, BSR/RTS->BRA
* 
* Revision 33.4  89/02/17  20:16:31  jesup
* Support for new sync reads, including retrys on non-perfect tracks
* support for new write alignment routine
* support for checkum & xfer simultaneously
* removal of nops
* other minor optimizations
* 
* Revision 33.3  89/01/04  16:57:51  jesup
* small code optimizations
* 
* Revision 33.2  86/10/08  14:33:35  bart
* Update was setting io_error with MOVE.L
* changed to MOVE.B
* 
* Revision 33.1  86/04/09  16:07:45  neil
* 68020 changes for real
* 
* Revision 32.1  85/12/23  17:19:36  neil
* Added rawread/rawwrite
* 
* Revision 30.2  85/10/09  00:36:39  neil
* Another internal (for 1.01)
* 
* Revision 30.1  85/10/08  16:36:53  neil
* re-arranged some debugging prints (which are back to being
* commented out...
* 
* Revision 27.3  85/07/11  03:12:33  neil
* turned off ProtStatus messages
* 
* Revision 27.2  85/07/09  17:45:29  neil
* Changed ProtStatus to actually go out and look at the drive,
* rather than just looking at the stored value.  It therefore
* is no longer an immediate command.
* 
* Revision 27.1  85/06/24  13:37:12  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  15:13:15  neil
* *** empty log message ***
* 
* 
*************************************************************************

	SECTION section

****** Included Files ***********************************************

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

	INCLUDE 'hardware/cia.i'

	INCLUDE 'resources/disk.i'

	INCLUDE 'devices/timer.i'

	INCLUDE 'trackdisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'
	LIST



****** Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

	XREF	tdName
	XREF	_ciaa
	XREF	_ciab

*------ Functions ----------------------------------------------------

	EXTERN_LIB Signal

	XREF	TDMfmEncode
	XREF	TDMfmDecode
	XREF	TDMotor
	XREF	TDDelay
	XREF	TDSeek
	XREF	TDTrkWrite
	XREF	TDTrkRead
	XREF	TDSetErrors
	XREF	TDMfmAllignTrack
	XREF	TDMfmSumCheck
	XREF	TDMfmSumCheckXfer
	XREF	TDMfmCPUXfer
	XREF	TDMfmCheckAllData
	XREF	TDMfmAllignWrite
	XREF	TDMfmLongEncode
	XREF	TDMfmSumBuffer
	XREF	TDGetUnit
	XREF	TDGiveUnit
	XREF	TDSectorize

	XREF	TermIO

*------ System Library Functions -------------------------------------

****** Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	XDEF	TDGetBuffer
	XDEF	TDIO
	XDEF	TDLruBuffer
	XDEF	TDReadBuffer
	XDEF	TDUClear
	XDEF	TDUMotor
	XDEF	TDUSeek
	XDEF	TDUUpdate
	XDEF	TDWriteBuffer
	XDEF	TDUProtStatus
	XDEF	TDUStop
	XDEF	TDUStart

*------ Data ---------------------------------------------------------


****** Local Definitions ********************************************


*****i* trackdisk.device/internal/TDIO *******************************
*
*   NAME
*	TDIO - basic loop for track disk driver
*
*   SYNOPSIS
*	TDIO( IOBlock ), TDLib
*	      A1	 A6
*
*   FUNCTION
*	This routine performs the basic IO for the track disk.	It
*	is given the IOBlock to do actions on.	The IOBlock will be
*	marked done when the IO is complete, and the process signaled.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
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
*
*   REGISTER USAGE
*	A6 -- TDLib
*	A4 -- ptr to sector in track buffer
*	A3 -- unit data
*	A2 -- IOBlock
*	D2 -- temp
*

TDIO:

		MOVEM.L D2/D3/D4/A2/A3/A4,-(SP)

		MOVE.L	IO_UNIT(A1),A3

		;-- test if we couldn't allocate a large enough buffer...
		;-- this handles not enough chip mem for 150RPM drives...
		;-- don't use tdio_signal, we haven't set tdu_ioblock yet
		BTST.B	#TDUB_UNREADABLE,TDU_FLAGS(a3)
		BEQ.S	1$
		MOVE.B	#TDERR_NoMem,IO_ERROR(a1)	; a1 has io block
		BRA	TDIO_TermIO			; termio this block

1$:		MOVE.L	A1,A2
		MOVE.L	A2,TDU_IOBLOCK(A3)

		MOVE.L	#0,IO_ACTUAL(A2)

		MOVE.L	IO_DATA(A2),TDU_DATA(A3)
*		;-- {state 0} check operation for legality
		MOVE.L	IO_OFFSET(A2),D0
		BSR	TDSectorize

		TST.L	D0
		BMI	TDIO_Err

		MOVE.W	D0,TDU_TRACK(A3)
		MOVE.B	D1,TDU_SECTOR(A3)

*		;-- check for IO within disk range
		MOVE.L	IO_OFFSET(A2),D0
		ADD.L	IO_LENGTH(A2),D0
		BSR	TDSectorize

		TST.L	D0
		BMI	TDIO_Err

*		;------ See if we are running with sector labels
		BTST	#TDUB_EXTENDED,TDU_FLAGS(A3)
		BEQ.S	TDIO_010

		MOVE.L	IOTD_SECLABEL(A2),TDU_LABELDATA(A3)
		BEQ.S	TDIO_010

		BSET	#TDUB_SECLABEL,TDU_FLAGS(A3)

TDIO_010:

*		;------ see if the disc is inserted
		BTST	#TDUB_REMOVED,TDU_FLAGS(A3)
		BEQ.S	TDIO_020

		MOVE.B	#TDERR_DiskChanged,IO_ERROR(A2)
		BRA	TDIO_Signal

TDIO_020:

*		;-- {state 1} see if buffer is in memory
		MOVE.W	TDU_TRACK(A3),D0
		BSR	TDGetBuffer
		MOVE.L	A0,TDU_BUFFER(A3)
		BNE.S	TDIO_Update

*		;-- {state 2} write out old buffer (if necessary)

*		;-- find a free buffer
		BSR	TDLruBuffer
		MOVE.L	A0,A2
		MOVE.L	A2,TDU_BUFFER(A3)

TDIO_IO:
*		;-- see if it is dirty
		BTST	#TDBB_DIRTY,TDB_FLAGS(A2)
		BEQ.S	TDIO_ReadTrk

*		;-- write the track out
		BSR	TDWriteBuffer
		TST.L	D0
		BEQ.S	TDIO_ReadTrk

*		;------ We had a write error
		MOVE.L	TDU_IOBLOCK(A3),A1
		MOVE.B	D0,IO_ERROR(A1)
		BRA	TDIO_Signal



*		;-- {state 3} read in new buffer
TDIO_ReadTrk

*		;-- update track number
		MOVE.W	TDU_TRACK(A3),TDB_TRACK(A2)
		BCLR	#TDBB_DIRTY,TDB_FLAGS(A2)
		CLR.B	TDU_RETRY(A3)

		BSR	TDReadBuffer	; returns error in D0.W
		TST.W	D0
		BEQ.S	TDIO_Update

*		;-- we has an error on the read.  We have already retried,
*		;-- so just mark the buffer as bad and notify the user
*		;-- note that we will now only do this if we couldn't read
*		;-- the track at all, i.e. TrkRead returned an error.
*		;-- otherwise, we will notify the user when he tries to
*		;-- read the sector.

*		;-- mark the buffer as invalid
		MOVE.W	#-1,TDB_TRACK(A2)
		MOVE.L	TDU_IOBLOCK(A3),A1
		MOVE.B	D0,IO_ERROR(A1)

		BRA	TDIO_Signal


*		;-- {state 4} update sector
TDIO_Update:

		MOVE.L	TDU_IOBLOCK(A3),A2
		MOVE.W	IO_COMMAND(A2),D0

		MOVE.L	TDU_BUFFER(A3),A0
		CMP.B	#CMD_WRITE,D0
		BNE	TDIO_Up20

*************************************************************************
*									*
* 			WE ARE DOING A WRITE				*
*									*
*************************************************************************

* FIX!  This code was not approved for 2.04, but is tested and works!  FIX!!!!
*		;-- check protection status before modifying track
*		btst.b	#TDUB_PROTECTED,TDU_FLAGS(a3)
*		beq.s	NotProtected
*		moveq	#TDERR_WriteProt,d0
*		bra.s	TDIO_WriteErr

NotProtected:
*		;-- this routine checksums data blocks, and may exit
*		;-- early if it TDBB_HASERRS gets set
		BSR	TDMfmCheckAllData (A0)	; preserves A0

*		;-- was there an unrecovered read error on the track??
		BTST	#TDBB_HASERRS,TDB_FLAGS(A0)
		BEQ.S	TDIO_NoErrs

		;-- we have an unrecovered read error - try to read again
		;-- if read attempt fails (with retries) return error
		;-- us max retries before reporting error
		CLR.B	TDU_RETRY(A3)

ReadErrLoop	BSR	TDReadBuffer
		MOVE.L	TDU_BUFFER(A3),A0
		BSR	TDMfmCheckAllData (A0)	; preserves A0
		BTST.B	#TDBB_HASERRS,TDB_FLAGS(A0)	; data/headers ok ?
		BEQ.S	TDIO_NoErrs
*
* FIX! we should not retry on disk removed.
* Also, we should return one of the errors in the error array instead of
* NotSpecified! FIX!
*

*		;-- have we made 10 retries yet??
		ADDQ.B	#1,TDU_RETRY(A3)
		MOVE.B	TDU_RETRY(A3),D0
		CMP.B	TDU_RETRYCNT(A3),D0
		BLE.S	ReadErrLoop

		;-- we failed too many times.  Return an error
		MOVEQ	#TDERR_NotSpecified,D0
TDIO_WriteErr:
		MOVE.L	TDU_IOBLOCK(A3),A1
		MOVE.B	D0,IO_ERROR(A1)
		BRA	TDIO_Signal

TDIO_NoErrs:
		BSET	#TDBB_DIRTY,TDB_FLAGS(A0)

*		;-- figure out sector starting address in buffer
*		;-- A4 points to first word AFTER $4489 $4489 (MFMSEC_FMT)
		MOVEQ	#0,D0
		MOVE.B	TDU_SECTOR(A3),D0
		ASL.W	#2,D0
		MOVE.L	TDB_SECPTRS(A0,D0.W),A4

*		;------	check for sector label usage
		BTST	#TDUB_SECLABEL,TDU_FLAGS(A3)
		BEQ.S	TDIO_UpDataWrite

		MOVE.L  TDU_LABELDATA(A3),A0
		MOVE.L	#TD_LABELSIZE,D0
		LEA	MFMFMT_SECLABEL(A4),A1

		BSR	TDMfmEncode

*		;------ go back and patch up the checksum...
		MOVE.L	A4,A0
		MOVE.W	#MFMFMT_HDRSUM,D1
		BSR	TDMfmSumBuffer	(D1:W,A0)


		LEA	MFMFMT_HDRSUM(A4),A0
		BSR	TDMfmLongEncode	(D0,A0)

TDIO_UpDataWrite:
*		;------ Beginning of Normal Write code

	IFGE INFO_LEVEL-50
	CLEAR	D0
	MOVE.W	TDU_TRACK(A3),D0	; compute sector number
	MULU	TDU_NUMSECS(A3),D0
	CLEAR	D1
	MOVE.B	TDU_SECTOR(A3),D1
	ADD.W	D1,D0
	MOVE.L	D0,-(SP)
	INFOMSG	50,<'%s/TDIO write: sec %ld'>
	ADDQ.L	#4,SP
	ENDC

		MOVE.L	TDU_DATA(A3),A0		; used as temp
		MOVE.L	#TD_SECTOR,D0
		LEA	MFMSEC_HDRSIZE-MFMSEC_FMT(A4),A1
						; alias for MFM_SECDATA

		BSR	TDMfmEncode

*		;------ go back and patch up the checksum...
		LEA	MFMSEC_DATA-MFMSEC_FMT(A4),A0
		MOVE.W	#MFM_SECTOR,D1
		BSR	TDMfmSumBuffer

TD_Up10encode:
		LEA	MFMSEC_DATASUM-MFMSEC_FMT(A4),A0
		BSR	TDMfmLongEncode

		BRA	TDIO_Up40


TDIO_Up20:
*************************************************************************
*									*
* 			WE ARE DOING A READ				*
*									*
*************************************************************************

*		;-- figure out sector starting address in buffer
*		;-- A4 points to first word AFTER $4489 $4489 (MFMSEC_FMT)
*		;-- A0 has TDB in it.
		MOVEQ	#0,D2
		MOVE.B	TDU_SECTOR(A3),D2
		MOVE.L	D2,D0
		ASL.W	#2,D2			; make LW offset

	IFGE INFO_LEVEL-40
	pea	0
	move.b	TDB_SECERRS(a0,d0.w),3(sp)
	move.l	d0,-(sp)
	PUTMSG	40,<'%s/Read: Sector %ld, error is 0x%lx'>
	addq.w	#8,sp
	ENDC

		MOVE.B	TDB_SECERRS(A0,D0.W),D3

*		;-- was there an error during read?
*		;-- note that negative nums are ok, they mean the data
*		;-- block hasn't been checksummed. Positive is an error.
		BLE.S	TDIO_Read_NoErr

		;-- we have an unrecovered read error - try to read again
		;-- if read attempt fails (with retries) return error
		;-- buffer can't be dirty, since we won't write with errs
		;-- Note we preserve the retry count for reading the
		;-- sector headers.
TDIO_Read_Retry
***		CLR.B	TDU_RETRY(A3)	; do not do this!!!!!
		BSR	TDReadBuffer
		MOVE.L	TDU_BUFFER(A3),A0
		BTST.B	#TDBB_HASERRS,TDB_FLAGS(A0)
		BEQ.S	TDIO_Read_NoErr	; got it (or at least the sector header)

		;-- we failed again (MAX retries).  Return an error
		;-- clear retry count so next attempt will start from 0
Read_err	CLR.B	TDU_RETRY(A3)
		MOVE.L	TDU_IOBLOCK(A3),A1
		MOVE.B	D3,IO_ERROR(A1)
		BRA	TDIO_Signal

TDIO_Read_NoErr:
*		;-- get the sector pointer
		MOVE.L	TDB_SECPTRS(A0,D2.W),A4

*		;-- compute the checksum if SECERRS[i] < 0
		TST.B	D3
		BEQ.S	GoodSum

*		;-- Compute the checksum AND transfer the data!
*		;-- we have to handle every byte of it, might as
*		;-- well decode while doing it.  Adds 16 cycles per
*		;-- longword of decoded data (68000) to 60 for checksum.
*		;-- don't do decode later (d3 < 0).
		MOVE.L	A0,D4			;-- save A0
		MOVE.L	TDU_DATA(A3),A1		;-- destination
		BSR	TDMfmSumCheckXfer (A1,A4)
		MOVE.L	D4,A0
		TST.L	D0
		BEQ.S	NewGoodSum

*		;-- checksum bad - retry until count > TDU_RETRY
	IFGE INFO_LEVEL-40
	move.l	d2,-(sp)
	PUTMSG	40,<'%s/ReadXfer: Sector %ld, error is BadSecSum'>
	addq.w	#4,sp
	ENDC
		MOVEQ	#TDERR_BadSecSum,D3
		MOVE.L	D2,D0
		LSR.W	#2,D0
		MOVE.B	D3,TDB_SECERRS(A0,D0.W)	   ;keep secerrs up to date
		BSET.B	#TDBB_HASERRS,TDB_FLAGS(A0)

		ADDQ.B	#1,TDU_RETRY(A3)	; check # of errors
		MOVE.B	TDU_RETRY(A3),D0

		CMP.B	TDU_RETRYCNT(A3),D0
		BGT.S	Read_err
		BRA.S	TDIO_Read_Retry

NewGoodSum	;-- we just computed the new sum, and it's good
		;-- d0 was saved in D2
		LSR.L	#2,D2
		CLR.B	TDB_SECERRS(A0,D2.W)

		;-- note that D3.B is negative, a flag not to call MfmDecode
		;-- for the data of the sector.
		;-- fall through

*		;------ see if he wants the label also
GoodSum		BTST	#TDUB_SECLABEL,TDU_FLAGS(A3)
		BEQ.S	TDIO_UpReadSec

		LEA	MFMFMT_SECLABEL(A4),A1
		MOVE.L  TDU_LABELDATA(A3),A0
		MOVE.L  #TD_LABELSIZE,D0

		BSR	TDMfmDecode

TDIO_UpReadSec:
*		;------ do the actual read of the sector


	IFGE INFO_LEVEL-50
	CLEAR	D0
	MOVE.W	TDU_TRACK(A3),D0	; compute sector number
	MULU	TDU_NUMSECS(A3),D0
	CLEAR	D1
	MOVE.B	TDU_SECTOR(A3),D1
	ADD.W	D1,D0
	MOVE.L	D0,-(SP)
	INFOMSG 50,<'%s/TDIO read:  sec %ld'>
	ADDQ.L	#4,SP
	ENDC 

*		;-- don't call decode if we used TDMfmSumCheckXfer
		TST.B	D3
		BMI.S	TDIO_Up40

		LEA	MFMSEC_HDRSIZE-MFMSEC_FMT(A4),A1
		MOVE.L	TDU_DATA(A3),A0
		MOVE.L	#TD_SECTOR,D0

		BSR	TDMfmDecode

*		;-- common r/w finish up routine
TDIO_Up40:

*		;------ update the data ptr and "# of bytes transferred" count
		MOVE.L	#TD_SECTOR,D1
		ADD.L	D1,TDU_DATA(A3)
		MOVE.L	IO_ACTUAL(A2),D0
		ADD.L	D1,D0
		MOVE.L	D0,IO_ACTUAL(A2)

*		;------ update the sector label pointer (if it is used)
		BTST	#TDUB_SECLABEL,TDU_FLAGS(A3)
		BEQ.S	TDIO_Up50
		ADD.L	#TD_LABELSIZE,TDU_LABELDATA(A3)

TDIO_Up50:

		CMP.L	IO_LENGTH(A2),D0
		BCC.S	TDIO_Signal		; d0 >= io_length (unsigned)

*		;------ There are more sectors to read/write
		MOVE.L	TDU_BUFFER(A3),A2
		ADDQ.B	#1,TDU_SECTOR(A3)

*		;------ Are we on the same track?
		MOVE.W	TDU_NUMSECS(A3),D1
		CMP.B	TDU_SECTOR(A3),D1
		BGT	TDIO_Update

*		;------ We switched tracks
		MOVE.B	#0,TDU_SECTOR(A3)
		ADDQ.W	#1,TDU_TRACK(A3)
		BRA	TDIO_IO


*		;-- {state 5} mark IOBLOCK as done
TDIO_Signal:
		MOVE.L	TDU_IOBLOCK(A3),A1
TDIO_TermIO:
		BSR	TermIO

		BRA.S	TDIO_End



TDIO_Err:
		MOVE.L	TDU_IOBLOCK(A3),A1
		MOVE.B	#IOERR_BADLENGTH,IO_ERROR(A1)
		BRA.S	TDIO_Signal

TDIO_End:
		MOVEM.L (SP)+,D2/D3/D4/A2/A3/A4
		RTS


*****i* trackdisk.device/internal/TDGetBuffer ************************
*
*   NAME
*	TDGetBuffer - locate a track buffer by its track number
*
*   SYNOPSIS
*	TrackPtr = TDGetBuffer( TrackNum, UnitPtr ), TDLib
*	A0,			D0:15-0,  A3,	     A6
*
*   FUNCTION
*	This routine locates an in core buffer for a track.  If the
*	track is not currently in memory it returns a NULL.  If it is
*	in memory, it returns a pointer to the buffer.
*
*   INPUTS
*	TrackNum - The number of the track that we are looking for.
*
*	UnitPtr - points to the unit structure for this disc
*
*   OUTPUTS
*	TrackPtr - points to the Track Buffer for this track
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
*
*   REGISTER USAGE
*	A6 -- TDLib
*	A3 -- unit data
*


TDGetBuffer:

		MOVE.L	TDU_BUFPTR(A3),A0
		MOVE.L	A0,D1		; set condition flags
		BEQ.S	TDGetBuf_NoBuf

		CMP.W	TDB_TRACK(A0),D0
		BEQ.S	TDGetBuf_End

*		;-- buffer did NOT match
TDGetBuf_NoBuf:
		SUB.L	A0,A0

TDGetBuf_End:
		RTS



*****i* trackdisk.device/internal/TDLruBuffer ************************
*
*   NAME
*	TDLruBuffer - Find a used buffer to be freed
*
*   SYNOPSIS
*	Buffer = TDLruBuffer( ), UnitPtr, TDLib
*	A0,			 A3,	  A6
*
*   FUNCTION
*	This routine finds a (free or used) buffer for use for IO.
*	Eventually it will use something like an LRU algorithm, but
*	currently it returns the one and only buffer.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
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
*
*   REGISTER USAGE
*	A6 -- TDLib
*	A3 -- unit data
*	A2 -- IOBlock
*

TDLruBuffer:
		MOVE.L	TDU_BUFPTR(A3),A0
		RTS



*****i* trackdisk.device/internal/TDWriteBuffer **********************
*
*   NAME
*	Error = TDWriteBuffer - do all the stuff to write out a track
*
*   SYNOPSIS
*	Error = TDWriteBuffer( ), UnitPtr, TDLib
*	D0			  A3,	   A6
*
*   FUNCTION
*	This routine does an actual write of a track.  Its
*	basic function is to make sure that all the floppy specific
*	stuff is done.
*
*   INPUTS
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
*---------------------------------------------------------------------
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- TDLib
*	A3 -- unit data
*	A2 -- buffer pointer
*


	IFNE ERRORMSGS
TDWBMes0 STRINGR <'write buffer 0x%lx, track %d, sector %d'>
TDWBMes1 STRINGR <'write error track %d error %ld'>
	ENDC ERRORMSGS

TDWriteBuffer:
		MOVEM.L D2/A2,-(SP)
		MOVE.L	TDU_BUFFER(A3),A2

	IFNE ERRORMSGS
		MOVEQ	#0,D0
		MOVE.B	TDU_SECTOR(A3),D0
		MOVE.W	D0,-(SP)
		MOVE.W	TDB_TRACK(A2),-(SP)
		MOVE.L	TDU_BUFFER(A3),-(SP)
		MOVE.L	SP,A1
		LEA	TDWBMes0,A0
		PUTFMT
		ADDQ.L	#8,SP
	ENDC ERRORMSGS

*		;-- turn the motor on
		MOVEQ	#1,D0
		BSR	TDMotor

*		;-- Make CERTAIN we're on the right track first!
		MOVEQ	#0,D0
		MOVE.W	TDB_TRACK(A2),D0
		BSR	TDSeek

*		;-- Align the buffer for writing, etc
*		;-- A2 has pointer to track buffer (TDB) in it.
*		;-- May set the OFFSET_2 flag!
		MOVE.B	TDB_FLAGS(A2),D1	; get flags
		BTST	#TDBB_ALIGNED,D1	; only do when needed
		BNE.S	no_align
		BSR	TDMfmAllignWrite 	; (A2)
no_align:
*		;-- do the actual IO
		LEA	TDB_BUF(A2),A0
		MOVE.W	TDU_MFM_MAXTRACK(A3),D0	; was MFM_MAXTRACK+2
		ADDQ.W	#2,d0			; for $aaa8
		BTST.B	#TDBB_OFFSET_2,TDB_FLAGS(A2)
		BEQ.S	do_write
		SUBQ.W	#2,d0			; track is 2 bytes shorter than
						; normal!
do_write:
		BSR	TDTrkWrite

*	IFNE ERRORMSGS
*		TST.L	D0
*		BNE	TDWriteB_Err
*
*		;-- as a check, try alligning the track to see if we can...
*		MOVE.W	TDU_TRACK(A3),D2
*		MOVE.W	TDB_TRACK(A2),TDU_TRACK(A3)
*
*		BSR	TDMfmAllignTrack
*		TST.L	D0
*		BPL.S	TDWB_Checks		; this is WRONG! REJ
*						; do not use this!
*
**		;-- IT HAD AN ERROR!
*	XDEF	TDWB_Err
*TDWB_Err:
*		MOVE.L	D0,-(SP)
*		MOVE.W	TDU_TRACK(A3),-(SP)
*		MOVE.L	SP,A1
*		LEA	TDWBMes1,A0
*		PUTFMT
*		ADDQ.L	#6,SP
*		BRA.S	TDWriteB_End
*
*TDWB_Checks:
**		;-- check that the header is all mfm zeros
*		LEA	TDB_BUF(A2),A0
*		MOVE.W	TDU_MFM_SLOP(a3),D1
*		SUBQ.W	#1,d1
*TDWB_Header:
*		CMP.B	#$0AA,(A0)+
*		DBNE	D1,TDWB_Header
*		MOVEQ	#1,D0
*		CMP.W	#-1,D1
*		BNE.S	TDWB_Err
*	
*TDWriteB_End:
*
*		MOVE.W	D2,TDU_TRACK(A3)
*		MOVEQ	#0,D0
*
*	ENDC ERRORMSGS
TDWriteB_Err:

		MOVEM.L (SP)+,A2/D2
		RTS

*****i* trackdisk.device/internal/TDReadBuffer ***********************
*
*   NAME
*	TDReadBuffer - do all the stuff to read in a track
*
*   SYNOPSIS
*	error = TDReadBuffer( ), UnitPtr, TDLib
*	D0(15:0)		   A3,	   A6
*
*   FUNCTION
*	This routine does an actual read of a track.  Its
*	basic function is to make sure that all the floppy specific
*	stuff is done.
*
*   INPUTS
*
*   RESULTS
*	error = 0 for no error, error code otherwise.
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
*	TDTrkRead now uses disksync, so the address passed to it should
*	start DMA at TDB_DATA+6, so everything ends up alligned after
*	adding the $aaaa $aaaa $4489 into those 6 bytes.  TDMfmAllignTrack
*	counts on this and fails otherwise.
*	Also, since we start DMA on a valid sector (hopefully), we won't
*	DMA the extra sectors worth.  We may miss a sector if we sync on
*	a bad spot on the disk (mutated into $4489), but we almost certainly
*	retry in that case.
*
*   REGISTER USAGE
*	A6 -- TDLib
*	A3 -- unit data
*	A2 -- IOBlock
*

	IFNE ERRORMSGS
TDRBMes STRINGR <'read  buffer 0x%lx, track %d, sector %d'>
	ENDC ERRORMSGS

TDRBErr STRINGR <'read error %ld, track %d'>

TDReadBuffer:

		MOVE.L A2,-(SP)

		MOVE.L	TDU_BUFFER(A3),A2

*		;-- turn the motor on
		MOVEQ	#1,D0
		BSR	TDMotor

TDReadB_Seek:
*		;-- seek to the correct track
		MOVEQ	#0,D0
		MOVE.W	TDU_TRACK(A3),D0
		BSR	TDSeek

TDReadB_Read:

	IFNE ERRORMSGS
		MOVEQ	#0,D0
		MOVE.B	TDU_SECTOR(A3),D0
		MOVE.W	D0,-(SP)
		MOVE.W	TDU_TRACK(A3),-(SP)
		MOVE.L	TDU_BUFFER(A3),-(SP)
		MOVE.L	SP,A1
		LEA	TDRBMes,A0
		PUTFMT
		ADDQ.L	#8,SP
	ENDC ERRORMSGS

	IFNE ERRORMSGS
*		;-- fill the buffer with "AA's" (nulls)
		LEA	TDB_BUF(A2),A0
		MOVE.W	TDU_MFM_TRKBUF(A3),D0
		LSR.W	#2,D0
		SUBQ.W	#1,D0			; DBRA loop!
TDReadB_Clear:
		MOVE.L	#$0AAAAAAAA,(A0)+
		DBRA	D0,TDReadB_Clear
	ENDC ERRORMSGS

*		;-- do the actual IO
		MOVE.L	TDB_DATA(A2),A0
		ADDQ.L	#6,a0				; was +4 REJ
		MOVE.W	TDU_MFM_MAXTRACK(A3),D0		; was + MFM_RAWTRACK

		BSR	TDTrkRead

*		;------ If there was an error, do retries
		TST.L	D0
		BEQ.S	TDReadB_Allign	; no error

		;-- set all error numbers to too few sectors!
		bset.b	#TDBB_ALLBAD,TDB_FLAGS(a2)
		bra.s	TDReadB_Retry
		
TDReadB_Allign:

*		;-- allign the buffer
*		;-- Assumes disksync - second $4489 at TDB_DATA+6
		BSR	TDMfmAllignTrack

		;-- did we read all the sectors
		SUB.W	TDU_NUMSECS(A3),D0
		BEQ.S	TDReadB_End

		TST.W	D0
		BNE.S	TDReadB_Retry			; found at least one
		BCLR.B	#TDBB_ALLBAD,TDB_FLAGS(a2)	; didn't find any
TDReadB_Retry:
*		;-- something went wrong.  Go ahead and try again.  Every
*		;--	now and then seek again to guard against seek errors
*		;--
*		;-- if we run out of retries, take current read and use it.
*		;-- set TDBF_HASERRS, don't write without getting clean
*		;-- read.  The user will only see an error if he tries to
*		;-- read a sector that had an error, or if he writes and it
*		;-- can't get a clean read (writes to any sector on the track)

	IFNE ErrorMonitor
		TST.B	TDU_RETRY(A3)
		BNE.S	TDReadB_PostMsg

		MOVE.W	TDU_TRACK(A3),-(SP)
		MOVE.L	D0,-(SP)
		MOVE.L	SP,A1
		LEA	TDRBErr,A0
		PUTFMT
		ADDQ.L	#6,SP
	ENDC ErrorMonitor

TDReadB_PostMsg:

		ADDQ.B	#1,TDU_RETRY(A3)
		MOVE.B	TDU_RETRY(A3),D0

		CMP.B	TDU_RETRYCNT(A3),D0
		BLE.S	TDReadB_DoRetry

		;-- failed - too many retries
		BSET.B	#TDBB_HASERRS,TDB_FLAGS(A2)
		MOVEQ	#TDERR_NoSecHdr,d0	; assume it's all bad
		BTST.B	#TDBB_ALLBAD,TDB_FLAGS(a2)
		BNE.S	TDReadB_End	; all bad - return failure

		MOVEQ	#0,D0		; just live with the error
					; so we can read the other sectors
		BRA.S	TDReadB_End

TDReadB_DoRetry:

	IFNE ErrorMonitor

*		;-- bump the retry count
		MOVE.W	TDU_TRACK(A3),D1
		LSL.W	#1,D1

		LEA	TDU_SOFTERRS(A3),A0
		ADDQ.W	#1,0(A0,D1.W)

*		;-- bump numerrs if we need to
		CMP.B	#1,D0
		BNE.S	TDReadB_Re10

		LEA	TDU_NUMERRS(A3),A0
		ADDQ.W	#1,0(A0,D1.W)
TDReadB_Re10:

	ENDC ErrorMonitor

		AND.B	#3,D0
		BNE	TDReadB_Read		; just try and read again

*		;-- force calibrate and then try again
		MOVE.W	#-1,TDU_SEEKTRK(A3)
		BRA	TDReadB_Seek

*TDReadB_HardErr:
*
*	IFNE ErrorMonitor
**		;-- bump the hard error count
*		MOVE.W	TDU_TRACK(A3),D1
*		LSL.W	#1,D1
*		LEA	TDU_HARDERRS(A3),A0
*		ADDQ.W	#1,0(A0,D1.W)
*	ENDC ErrorMonitor
*

TDReadB_End:
		;-- d0.W has return code - 0 = success, not 0 = error
		MOVE.L (SP)+,A2
		RTS



*****i* trackdisk.device/internal/TDUMotor ***************************
*
*   NAME
*	TDUMotor - user visible control for motor
*
*   SYNOPSIS
*	TDUMotor( IOBlock ), UnitPtr, DevPtr
*		  A1	     A3       A6
*
*   FUNCTION
*	This routine allows the user to control the disc motor. He
*	may turn it either on or off.  Note that the motor will be
*	automatically turned on during an I/O request, but is never
*	turned of except by this command.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
*	    IO_ACTUAL -- returns the previous state of the motor
*	    IO_LENGTH -- the requested state of the motor
*		0 ==> turn motor off
*		1 ==> turn motor on
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
*	A3 -- unit data
*	A2 -- IOBlock
*


TDUMotor:
		MOVE.L A1,-(SP)


		MOVE.L	IO_LENGTH(A1),D0
		BSR	TDMotor

		MOVE.L (SP)+,A1
		MOVE.L	D0,IO_ACTUAL(A1)

		BRA	TermIO		; was BSR - REJ

*		RTS





*****i* trackdisk.device/internal/TDUSeek ****************************
*
*   NAME
*	TDUSeek - user visible control for the heads
*
*   SYNOPSIS
*	TDUSeek( IOBlock ), TDLib
*		  A1	     A6
*
*   FUNCTION
*	This routine allows the user to control the seek position.
*	Note that the heads will be automatically seeked during an
*	I/O request; this command allows the heads to be preseeked
*	if the next position is known prior to the I/O being ready.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
*	    IO_OFFSET -- the location to seek to
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
*	A3 -- unit data
*	A2 -- IOBlock
*

TDUSeek:
		MOVE.L A2,-(SP)

		MOVE.L	A1,A2


*		;-- check operation for legality
		MOVE.L	IO_OFFSET(A1),D0
		BSR	TDSectorize

		TST.L	D0
		BMI.S	USeek_Err

		BSR	TDSeek

USeek_Signal:
		MOVE.L	A2,A1
		MOVE.L (SP)+,A2
		BRA	TermIO		; was BSR, and move was after - REJ

*		RTS

USeek_Err:
		MOVE.B	#IOERR_BADLENGTH,IO_ERROR(A2)
		BRA.S	USeek_Signal


*****i* trackdisk.device/internal/TDUProtStatus **********************
*
*   NAME
*	TDUProtStatus - return whether the current disk is write protected
*
*   SYNOPSIS
*	TDUProtStatus( IOBlock ), UnitPtr, DevPtr
*		         A1	     A3       A6
*
*   FUNCTION
*	This routine tells whether the current disk is write protected.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
*		IO_ACTUAL - nonzero if the disk is protected, 0 otherwise
*		If there is no disk in the drive, then IO_ERROR is set
*		to TDERR_DiskChanged
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
*	A3 -- unit data
*	A2 -- IOBlock
*

PROTKLUDGE	EQU	1

	IFEQ	PROTKLUDGE

TDUProtStatus:
		CLEAR	D0
		MOVE.L	IO_UNIT(A1),A0
		MOVE.B	TDU_FLAGS(A0),D1
		BTST	#TDUB_REMOVED,D1
		BNE.S	TDUProtStatus_NoDisk

		BTST	#TDUB_PROTECTED,D1
		SNE	D0
		MOVE.L	D0,IO_ACTUAL(A1)

	IFGE	INFO_LEVEL-50
	MOVE.L	D0,-(SP)
	PEA	0
	MOVE.B	TDU_UNITNUM(A0),3(SP)
	INFOMSG 50,<'%s/ProtStatus: unit %ld returning %ld'>
	ADDQ.L	#8,SP
	ENDC


TDUProtStatus_End:
		BRA	TermIO		; was BSR - REJ
*		RTS

	ENDC !PROTKLUDGE

TDUProtStatus_NoDisk:

	INFOMSG	50,<'%s/ProtStatus: disk is removed'>

		MOVE.B	#TDERR_DiskChanged,IO_ERROR(A1)
		BRA.S	TDUProtStatus_End

	IFNE	PROTKLUDGE

TDUProtStatus:
		MOVEM.L	A2/A3/D2,-(SP)

		MOVE.L	A1,A2
		MOVE.L	IO_UNIT(A2),A3


		BTST.B	#TDUB_REMOVED,TDU_FLAGS(A3)
		BNE.S	TDUProtStatus_NoDisk

*		MOVEQ	#1,D0
*		BSR	TDMotor

		BSR	TDGetUnit

		; MUST select unit before reading lines!
*		MOVE.B	TDU_6522(A3),_ciab+ciaprb
*		NOP				; not needed
*		NOP
		MOVE.B	_ciaa+ciapra,D2

		BSR	TDGiveUnit

		CLEAR	D0
		BTST	#CIAB_DSKPROT,D2
		SEQ	D0
		MOVE.L	D0,IO_ACTUAL(A2)

*		MOVEQ	#0,D0
*		BSR	TDMotor

	IFGE	INFO_LEVEL-50
	MOVE.L	IO_ACTUAL(A2),-(SP)
	PEA	0
	MOVE.B	TDU_UNITNUM(A3),3(SP)
	INFOMSG 50,<'%s/ProtStatus: unit %ld returning %ld'>
	ADDQ.L	#8,SP
	ENDC


TDUProtStatus_End:
		MOVE.L	A2,A1
		MOVEM.L	(SP)+,A2/A3/D2
		BRA	TermIO		; was BSR, and MOVEM was after

*		RTS


	ENDC PROTKLUDGE


*****i* trackdisk.device/internal/TDUClear ***************************
*
*   NAME
*	TDUClear - clear all buffers without writting
*
*   SYNOPSIS
*	TDUClear( IOBlock ), UnitPtr, DevPtr
*		  A1	     A3       A6
*
*   FUNCTION
*	This routine clears all buffers in the driver.  Any pending
*	data is deleted.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
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
*	A3 -- unit data
*	A2 -- IOBlock
*

TDUClear:

		MOVE.L	TDU_BUFPTR(A3),A0
		MOVE.W	#-1,TDB_TRACK(A0)
		BCLR	#TDBB_DIRTY,TDB_FLAGS(A0)

		BRA	TermIO		; was BSR - REJ
*		RTS


*****i* trackdisk.device/internal/TDUUpdate **************************
*
*   NAME
*	TDUUpdate - write all dirty buffers out to disc
*
*   SYNOPSIS
*	TDUUpdate( IOBlock ), UnitPtr, DevPtr
*		   A1	      A3       A6
*
*   FUNCTION
*	This routine clears all buffers in the driver.  Any pending
*	data is deleted.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
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
*	A3 -- unit data
*	A2 -- IOBlock
*

TDUUpdate:
		MOVE.L	A2,-(SP)
		MOVE.L	A1,A2

		MOVE.L	TDU_BUFPTR(A3),A0
		BTST	#TDBB_DIRTY,TDB_FLAGS(A0)
		BEQ.S	TDUUpdate_End

		BSR	TDWriteBuffer

		MOVE.L	A2,A1
		TST.L	D0
		BEQ.S	TDUUpdate_Clear

*		;------ we had a write error
		MOVE.B	D0,IO_ERROR(A1)	* bart - 10.08.06 changed from MOVE.L !
		BRA.S	TDUUpdate_End

TDUUpdate_Clear:
		MOVE.L	TDU_BUFPTR(A3),A0
		BCLR	#TDBB_DIRTY,TDB_FLAGS(A0)

TDUUpdate_End:
		MOVE.L	(SP)+,A2
		BRA	TermIO		; was BSR, and MOVE was after - REJ

*		RTS

*****i* trackdisk.device/internal/TDUStop **************************
*
*   NAME
*	TDUStop - Stop the trackdisk unit
*
*   SYNOPSIS
*	TDUStop( IOBlock ), UnitPtr, DevPtr
*		   A1	      A3       A6
*
*   FUNCTION
*	This function stops the trackdisk unit.  It will continue to queue
*	requests, but will not act on them until restarted with CMD_START.
*	Note that this causes an Update (flush of buffers to disk) to occur
*	if the buffer is dirty.  While stopped, it is safe to modify
*	TDU_CURRTRACK and TDU_COUNTER in the public unit structure.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
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
*	A3 -- unit data
*

TDUStop:
	BSET.B	#TDUB_STOPPED,TDU_FLAGS(a3)
	BSR.S	TDUUpdate			; it will return the IOR!
	MOVE.L	TDU_BUFPTR(a3),a0
	MOVE.W	#-1,TDB_TRACK(a0)		; mark track buf as empty
	RTS					; TermIO already done


*****i* trackdisk.device/internal/TDUStart **************************
*
*   NAME
*	TDUStart - Start the trackdisk unit
*
*   SYNOPSIS
*	TDUStart( IOBlock ), UnitPtr, DevPtr
*		   A1	      A3       A6
*
*   FUNCTION
*	This restarts the unit.  Note that Stop/start do NOT nest.
*	The current track/change-count may have been modified while
*	we were stopped.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
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
*	A3 -- unit data
*	A2 -- IOBlock
*

TDUStart:
	BCLR.B	#TDUB_STOPPED,TDU_FLAGS(a3)
	MOVE.L	a1,-(sp)			; save ior ptr
	MOVE.L	#TDF_DEVDONE,D0
	LEA	TDU_TCB(A3),A1
	LINKSYS Signal				; wake up unit task
	MOVE.L	(sp)+,a1
	BRA	TermIO

	END
