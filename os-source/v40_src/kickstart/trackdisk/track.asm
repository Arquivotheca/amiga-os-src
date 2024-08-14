
*************************************************************************
*									*
*  Copyright (C) 1985,1989 Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* track.asm
*
* Source Control
* ------ -------
* 
* $Id: track.asm,v 32.13 91/03/27 21:30:45 jesup Exp $
*
* $Locker:  $
*
* $Log:	track.asm,v $
* Revision 32.13  91/03/27  21:30:45  jesup
* Really fixed NULLs, etc (forgot 4(a0) instead of (a0) for FUNNYA1)
* 
* Revision 32.12  91/03/25  20:49:37  jesup
* Fix up NULLs between sectors
* 
* Revision 32.11  91/03/13  20:39:20  jesup
* Added the SetErrors routine to initialize the TDB_SECERRS array
* 
* Revision 32.10  90/11/28  23:44:26  jesup
* no changes
* 
* Revision 32.9  90/11/28  23:40:27  jesup
* TDB_DATA now a variable
* 
* Revision 32.8  90/11/26  20:58:11  Unknown
* Fixed register usage around IncSector call (d1 is now trashed)
* 
* Revision 32.7  90/11/21  04:20:06  jesup
* Changes for multi-density floppies.
* 
* Revision 32.6  90/06/01  23:18:15  jesup
* Conform to include standard du jour
* 
* Revision 32.5  90/03/16  01:08:18  jesup
* Fixed several MFM problems I introduced.  First, the SEC_NULL fields were
* always $2aaaaaaa.  Under 1.3 they were wrong also, always $aaaaaaaa.  I also
* made the 2aa8/aaa8 go after the last sector, instead of 8 bytes farther on.
* I now deal with both SYNC words being eaten on the first sector, using
* OFFSET_2 to avoid moving the buffer, or adding 1K to each unit's allocation.
* Added a bunch of comments.
* 
* Revision 32.4  89/05/08  16:21:18  jesup
* no change (mark)
* 
* Revision 32.3  89/04/13  17:12:10  bryce
* Conform internal autodocs to the autodoc style guide.  Add history
* notes on the massive DSKSYNC rework.
* 
* Revision 32.2  89/02/17  20:21:44  jesup
* Totally rewritten for sync reads and new write scheme.
* Much improved performance, less system overhead.
* 
* Revision 32.1  85/12/23  17:20:07  neil
* Added rawread/rawwrite
* 
* Revision 30.2  85/10/09  00:37:10  neil
* Another internal (for 1.01)
* 
* Revision 30.1  85/10/08  16:55:33  neil
* changed "trailing data word" from a AAAA to an AAA8 (this is the
* word that is written just after a track.
* 
* Revision 27.1  85/06/24  13:38:26  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  15:13:40  neil
* *** empty log message ***
* 
* 
*************************************************************************

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

	INCLUDE 'resources/disk.i'

	INCLUDE 'hardware/custom.i'

	INCLUDE 'devices/timer.i'

	INCLUDE 'trackdisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'
	LIST

	SECTION section

;****** Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

	XREF	tdName

*------ Functions ----------------------------------------------------

	XREF	TDMfmLongEncode
	XREF	TDMfmLongDecode
	XREF	TDMfmSumBuffer
	XREF	TDMfmSumBufferDecode
	XREF	TDMfmFixBit
	XREF	_LVOCopyMem

*------ System Library Functions -------------------------------------

;****** Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	XDEF	TDMfmFindSector
	XDEF	TDSetErrors
	XDEF	TDMfmAllignTrack
	XDEF	TDMfmSumCheck
	XDEF	TDMfmSumCheckXfer
	XDEF	TDMfmCheckAllData
	XDEF	TDMfmAllignWrite

*------ Data ---------------------------------------------------------

;****** Local Definitions ********************************************

NULLS	EQU	$AAAAAAAA		; two bytes of MFM coded NULLS
FUNNYA1	EQU	$44894489		; a magic illegal pattern
DSYNC	EQU	$4489			; a single sync
MFMNULL	EQU	$AAAA			; an MFM 0 byte


*****i* trackdisk.device/internal/TDMfmFindSector ********************
*
*   NAME
*	TDMfmFindSector - Find the beginning of a sector in a track
*
*   SYNOPSIS
*	Ptr = TDMfmFindSector( BufferPtr, EndPtr ), TDLib
*	A0			A0	    A1	      A6
*
*   FUNCTION
*	This routine finds the beginning of a sector in a raw track
*	buffer.	 Every sector begins with a "00 00 A1* A1*".  This
*	routine looks for this pattern (which may be on an arbitrary
*	bit boundary).  Actually, it only looks for the A1*'s.
*
*   INPUTS
*	BufferPtr - a word alligned pointer into the track buffer.
*
*	Endptr    - addr of word past end of buffer
*
*   OUTPUTS
*	Ptr - a word alligned pointer to the start of the sector. Will
*		be -1 if no match is found
*	    - note that ptr is to the word AFTER the $4489(s).  It will
*	      advance ptr past the second $4489 IF it is there.
*
*   EXCEPTIONS
*
*   SEE ALSO
*	TDMfmSecDecode, TDMfmDecode
*
*   BUGS
*
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A0 -- pointer to buffer area
*	D0 -- number of WORDS to examine
*	D1 -- constant (word) DSYNC
*
**********************************************************************

TDMfmFindSector
		MOVE.L	A1,D0
		SUB.L	A0,D0
		ASR.W	#1,D0		; size < 32767
		BMI.S	findfail	; a0 past a1

		MOVE.W	#DSYNC,D1

findloop	CMP.W	(A0)+,D1
		DBEQ	D0,findloop	; exit loop on equal

		;-- exited - is d0 == -1?
		TST.W	D0
		BPL.S	foundit		; if d0 >= 0, then we found it

		;-- failed
findfail	MOVE.W	#-1,A0
		RTS

		;-- handle second $4489, if it exists
foundit		CMP.W	(A0)+,D1
		BEQ.S	findexit
		SUBQ.L	#2,A0		; back up to word after $4489
findexit:	RTS
		

*****i* trackdisk.device/internal/TDSetErrors ********************
*
*   NAME
*	TDSetErrors - set all the error values to Too Few Sectors
*
*   SYNOPSIS
*	TDSetErrors( BufferPtr )
*			A2
*
*   FUNCTION
*	Sets all the sector error values to Too Few Sectors.  Used to
*	clear out the array on the beginning of TDMfmAllignTrack and if
*	TrkRead returns an error.	
*
*   INPUTS
*	BufferPtr - pointer to the TDBuffer structure.
*
*   OUTPUTS
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
*
*   IMPLEMENTATION NOTES
*
**********************************************************************
TDSetErrors:
		LEA	TDB_SECERRS(A2),A0
		MOVE.L	#$1a1a1a1a,D1	; 4 TDERR_TooFewSecs (26 decimal)

		MOVE.L	D1,(A0)+
		MOVE.L	D1,(A0)+
		MOVE.L	D1,(A0)+
		MOVE.L	D1,(A0)+
		MOVE.L	D1,(A0)+
		MOVE.L	D1,(A0)+	; fills thru sec 24
		RTS

*****i* trackdisk.device/internal/TDMfmAllignTrack ********************
*
*   NAME
*	TDMfmAllignTrack - Allign the track after a read
*
*   SYNOPSIS
*	Sectors = TDMfmAllignTrack( ), UnitPtr, TDLib
*	D0,			         A3,      A6
*	15-0
*
*   FUNCTION
*	This routine will word allign a track after a read.  It will
*	also make all the sectors contiguous in the track, and will
*	renumber the sector identifiers.
*	Also clears the TDBF_HASERRS flag, and sets the TDB_SECPTRS
*	and TDB_SECERRS arrays.
*
*   OUTPUTS
*	Sectors - returns the number of sector successfully read.  Should
*		  normally equal NUMSECS.
*
*   EXCEPTIONS
*
*   SEE ALSO
*	TDMfmSecDecode, TDMfmDecode, TDMfmFindSector
*
*   BUGS
*
*   HISTORY NOTES
*
*	History: When the "Portia" chip was upgraded to "Paula" Glenn
*	Keller added a "disk synchronization" register (DSKSYNC).
*	This was never used by trackdisk.  DSKSYNC will find an arbitary
*	16 bit MFM pattern on the disk, so we no longer need to bit search
*	memory.  This also buys significant gains for recovering bad
*	disks.  DSKSYNC will resync for the duration of a read.
*
*	If the sync word is found at an arbitary point in the input
*	stream, Paula resyncs immediatly.   The sync word is NOT included
*	in the output.  If the sync word is found aligned, no resync is
*	needed and the sync word IS included inthe output.
*
*	From checking the schematics, DSKSYNC will also work to start
*	*WRITE* DMA.  (this feature probably has no good use).
*
*			-Bryce
*
*   IMPLEMENTATION NOTES
*
*	Major changes for use of disksync.  Assumes read started at
*	with the _second_ $4489 sync mark of a sector.  All following
*	sectors up to the gap will have both $4489's.  The first sector
*	following the gap will not, unless the offset was 0 bits.
*
*	Sets up the TDB_SECPTRS array with pointers to each of the
*	sectors.  Doesn't move the sectors around, a write will call
*	TDMfmReallign to set it up for write.  Also, errors are on a
*	sector by sector basis stored in the TDB_SECERRS array.
*
*	Because of where DMA starts, the transfer should begin at TDB_DATA
*	+ 6, so the $aaaaaaaa $4489 can be added to the front.  Paula ALWAYS
*	eats the first 4489 after (at) DMA start.  We cannot get two at the
*	beginning, but can get two later.
*
*	Also NOTE! We may have 0 $4489's at the beginning of the track!  This
*	is a rare occurance, the DMA must start in the middle of the first
*	one (sees the second, throws it away (non-aligned), then starts xfer
*	on the sector ID).  Chance of this is very small (15/(544*2*16) ~=
*	.08%, or about 1 in a thousand reads.
*
*   REGISTER USAGE
*	A6 -- TDLib
*	A2 -- Track buffer
*	A3 -- Unit pointer
*	A0 -- current position in buffer
*	A1 -- scratch
*	D0,D1,D4 -- scratch
*	D2 -- number of sectors found
*	D3 -- address of end of buffer
*	D5 -- stores address of buffer during BSR's
*	D6 -- Current sector number
*
**********************************************************************


TDMfmAllignTrack

		MOVEM.L A2/D2/D3/D5/D6,-(SP)
		MOVE.L	TDU_BUFFER(A3),A2

*		;-- clear HASERRS, ALIGNED, and OFFSET_2 bits.
		MOVEQ	#~(TDBF_HASERRS!TDBF_ALIGNED!TDBF_OFFSET_2),D1
		AND.B	D1,TDB_FLAGS(A2)

*		;-- Set all TDB_SECERRS to TDERR_TooFewSecs
		BSR.S	TDSetErrors	; (a2)

*		;-- find the beginning of where we want the data to go
*		;-- also get pointer to sector array
		MOVE.L	TDB_DATA(A2),A0
		ADDQ.L	#4,a0			; start searching here
		LEA	TDB_BUF(A2),A1		; beginning of buffer
		ADD.W	TDU_MFM_TRKBUF(a3),a1	; only works up to 32K!!!!
					; size of current tracks (handles
					; variable number of sectors!)
		
		MOVE.L	A1,D3		; end of buffer (search)
		MOVEQ	#0,D2		; no sectors found yet

		;-- handle the first sector differently.  It may have either
		;-- 0 or 1 $4489 preceeding it.  To make the code simpler, I'll
		;-- stuff a $4489 into the preceeding word (since we know we
		;-- synced on one).  Before writing, AlignWrite will fix it.
		MOVE.W	#DSYNC,(a0)	; now have 1 or two $4489's
Allign_loop
		;-- find the next sector
		MOVE.W	D3,A1		; max addr to look at
		BSR.S	TDMfmFindSector
		CMP.W	#-1,A0
		BEQ.S	Allign_No_More

Allign_10
*		;------ We found a sector header.  The address is in A0.
*		;------ FindSector returns address of word after last 4489
*		;------ It also handles either 1 or 2 $4489's.

*		;------ Do some common setup stuff
		MOVE.L	A0,D5			; save address of header
		MOVEQ	#(MFMFMT_HDRSUM>>2)-1,D1
		CLEAR	D6

*		;------ we are properly alligned.
*		;------ now checksum the sector header
*		;------ Stupid motorola non-orthagonalities in EOR!
Allign_12:
		MOVE.L  (A0)+,D0
		EOR.L   D0,D6
		DBRA	D1,Allign_12
		AND.L	#$55555555,D6

*		;------ common code to see if the sector checksum is OK
		BSR	TDMfmLongDecode	(A0)
		MOVE.L	D5,A0		; restore A0
		CMP.L	D0,D6
		BEQ.S	GoodCheck

		;-- start right after the last $4489 found
		BRA.S	Allign_loop	; if bad, ignore and look for another
					; this will give a TooFewSecs error
					; unless of course it finds them all

*		;-- get the TrackID word of this sector
GoodCheck:	BSR	TDMfmLongDecode	(A0)	; a0 -> format words

*		;------ We already have a good checksum.
*		;------ Do a rudimentary check on D6 to make sure it
*		;------ makes sense.  This will be two things:
*		;------ That its highest 8 bits are FMT_SECTYPE
*		;------ and that it has the correct track #.

		MOVE.L	D5,A0		; restore A0
		MOVE.L	D0,-(SP)
		CLEAR	D6
		MOVE.B	FMT_SECTOR(SP),D6	; long sector number
		CMP.B	#FMT_SECTYPE,FMT_TYPE(SP)
		BEQ.S	GoodFormat

		;-- bad format - mark in TDB_SECERRS
ErrBadSecHdr	MOVEQ	#TDERR_BadSecID,D0
		MOVE.B	D0,TDB_SECERRS(A2,D6.W)
		ADDQ.W	#4,SP		; drop format longword on stack

		;-- skip the sector area (we know one exists)
		BRA.S	SkipSec

		;-- make sure we're on the right track
GoodFormat	MOVE.B	FMT_TRACK(SP),D0
		CMP.B	TDU_TRACK+1(A3),D0	; note TDU_TRACK is a word
		BNE.S	ErrBadSecHdr

		ADDQ.W	#4,SP		; drop word on stack

		;-- We won't update the number of sectors until gap here.
		;-- That will be done right before writing the track out.
		;-- We also won't check the data checksum until we 
		;-- actually read the sector.  We we do check it, we
		;-- will set TDB_SECERRS[sector] to 0.

		;-- make sure I haven't found the sector before!
		LEA	TDB_SECERRS(A2,D6.W),A1
		CMP.B	#TDERR_TooFewSecs,(A1)
		BNE.S	SkipSec

		;-- no errors, clear error entry (was set on entry)
		MOVE.B	#-1,(A1)

		;-- set TDB_SECPTRS[d6] = start of header
		ASL.W	#2,D6
		MOVE.L	A0,TDB_SECPTRS(A2,D6.W)

		;-- increment # of sectors found
		ADDQ.W	#1,D2

		;-- loop and find next sector
		;-- note A0 points to the word after syncs.
SkipSec		LEA	MFM_RAWSECTOR-8(A0),A0		; 8 for 00 00 A1* A1*
		BRA	Allign_loop

Allign_No_More

*		;-- return the number of sectors read (word value)
	IFGE	INFO_LEVEL-40
	move.w	d2,-(sp)
	clr.w	-(sp)
	PUTMSG	40,<'%s/TDAllignTrack: found %ld sectors'>
	addq.w	#4,sp
	ENDC
		MOVE.W	D2,D0
		MOVEM.L	(SP)+,D2/D3/D5/D6/A2
		RTS

*****i* trackdisk.device/internal/TDMfmSumCheckXfer *****************************
*
*   NAME
*	TDMfmSumCheckXfer - checksum a sectors data block and transfers it
*
*   SYNOPSIS
*	Result = TDMfmSumCheckXfer( Dest, Source )
*	  D0			     A1,    A4
*
*   FUNCTION
*	Compute the checksum of the sector passed, and compare it to the
*	stored checksum.  (Data portion only)
*	Also, transfer the data to the destination after decoding it.
*
*   INPUTS
*	Dest - a pointer to the destination area
*
*	Source - a pointer to the starting word of the sector (MFMSEC_FMT)
*
*   OUTPUTS
*	Result - Old checksum - New checksum
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A4 - pointer to MFMSEC_FMT of the sector
*
*
**************************************************************************

TDMfmSumCheckXfer:
		MOVE.L	D2,-(SP)

		LEA	MFMSEC_DATA-MFMSEC_FMT(A4),A0
		MOVE.W	#MFM_SECTOR,D1
		BSR	TDMfmSumBufferDecode (D1,A0,A1)
		BRA.S	SumCheckCommon

*****i* trackdisk.device/internal/TDMfmSumCheck *****************************
*
*   NAME
*	TDMfmSumCheck - checksum a sectors data block and check
*
*   SYNOPSIS
*	Result = TDMfmSumCheck( Source )
*	  D0			  A4
*
*   FUNCTION
*	Compute the checksum of the sector passed, and compare it to the
*	stored checksum.  (Data portion only)
*
*   INPUTS
*	Source - a pointer to the starting word of the sector (MFMSEC_FMT)
*
*   OUTPUTS
*	Result - Old checksum - New checksum
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A4 - pointer to MFMSEC_FMT of the sector
*
**************************************************************************

TDMfmSumCheck:
		MOVE.L	D2,-(SP)

		LEA	MFMSEC_DATA-MFMSEC_FMT(A4),A0
		MOVE.W	#MFM_SECTOR,D1
		BSR	TDMfmSumBuffer (D1,A0)
SumCheckCommon
		MOVE.L	D0,D2

		LEA	MFMSEC_DATASUM-MFMSEC_FMT(A4),A0
		BSR	TDMfmLongDecode
		SUB.L	D2,D0

		MOVE.L	(SP)+,D2
		RTS

*****i* trackdisk.device/internal/TDMfmCheckAllData *****************************
*
*   NAME
*	TDMfmCheckAllData - checksum the data of all the sectors of a track
*
*   SYNOPSIS
*	Track Buffer = TDMfmCheckAllData( Track Buffer )
*	     A0				   A0
*
*   FUNCTION
*	Compute the checksums for all sectors, and set TDBB_HASERRS, etc if
*	any of them checksum wrong.
*
*   INPUTS
*	Track Buffer - a pointer to a TDB_xxx structure
*
*   OUTPUTS
*	Track Buffer - same as input parameter
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*	PRESERVES A0!!!
*
**************************************************************************

TDMfmCheckAllData:
		BTST.B	#TDBB_HASERRS,TDB_FLAGS(A0)
		BNE.S	CheckDone

		MOVEM.L	D2/A2/A4,-(SP)
		MOVE.W	TDU_NUMSECS(A3),D2
		SUBQ.W	#1,D2			; entering dbra from top
		MOVE.L	A0,A2
Checkall_loop
		TST.B	TDB_SECERRS(A2,D2.W)
		BGT.S	SetErr			; should have been set already
		BEQ.S	Checkall_end		; already checksummed

		;-- negative error means not checksummed
		MOVE.W	D2,D0
		ASL.W	#2,D0
		MOVE.L	TDB_SECPTRS(A2,D0.W),A4
		BSR.S	TDMfmSumCheck (A4)
		TST.L	D0			; new minus old checksums
		BEQ.S	GoodSum

		;-- bad checksum - mark
BadSum		MOVEQ	#TDERR_BadSecSum,D0
		MOVE.B	D0,TDB_SECERRS(A2,D2.W)
SetErr		BSET.B	#TDBB_HASERRS,TDB_FLAGS(A2)
		BRA.S	Checkall_end		; keep on checking sectors

		;-- sector is ok, mark as checked
GoodSum:	CLR.B	TDB_SECERRS(A2,D2.W)

Checkall_end:	DBRA	D2,Checkall_loop

		MOVE.L	A2,A0			; preserve A0!!!
		MOVEM.L	(SP)+,D2/A2/A4
CheckDone	RTS


*****i* trackdisk.device/internal/TDMfmAllignWrite *****************************
*
*   NAME
*	TDMfmAllignWrite - Take a track buffer and massage it for writing
*
*   SYNOPSIS
*	TDMfmAllignWrite( Buffer ), TDLib
*			    A2	      A6
*
*   FUNCTION
*	Does all needed massaging of the sector so it can be directly
*	written.  In particular, it copies the sectors so they are
*	contiguous, and start at the beginning of the buffer.
*	With disksync, the first sector will always begin at the beginning
*	of the buffer.  Also modifies the sectors to gap and recalculates
*	the checksums of the headers.  All $2aaaaaaa $44894489's will be
*	inserted.
*
*	We might have NO SYNCs from the read (but just one stuffed by the
*	read alignment code).  Check if we got a sync mark as the first
*	DMAed word.
*
*   INPUTS
*	Buffer - a pointer to the track buffer
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*	A6 -- TDLib
*
***********************************************************************

TDMfmAllignWrite:
		MOVEM.L	A4/D2/D3/D4/D5/D6,-(SP)

		;-- First check whether we got 0 or 1 $4489's, and move buffer
		;-- ptr down one word if we got 0 (instead of moving data up)
		;-- Also set flag to say we aligned it already
		MOVE.L	TDB_DATA(A2),A4
		BSET.B	#TDBB_ALIGNED,TDB_FLAGS(A2)  ; it will be
		BCLR.B	#TDBB_OFFSET_2,TDB_FLAGS(A2) ; paranoia
		MOVE.L	#FUNNYA1,D2
		MOVE.L	#NULLS,D0
		CMP.W	6(A4),D2		; we start DMA at +6
		BEQ.S	one_a1

		;-- No $4489's.  Dec buffer ptr by 2, set flag in structure
		SUBQ.L	#2,A4
		BSET.B	#TDBB_OFFSET_2,TDB_FLAGS(A2)

		;-- now make sure the first sector has the nulls, etc
		;-- we started reading with the second DSYNC at TDB_DATA+6
		;-- no need to fixbit the NULLS, since they follow the gap
one_a1:		MOVE.L	D0,(A4)+		; MUST be preceded by $aaaa!!
		MOVE.L	D2,(A4)+		; #FUNNYA1

		;-- find out how many sectors till gap
		MOVE.L	A4,A0
		MOVE.L	A4,D5			; in case we started in gap
		BSR	TDMfmLongDecode		; get the format word
		CLEAR	D2
		MOVE.B	D0,D2			; sectors till gap (inc current)
		SUBQ.W	#1,D2			; not inc current
		MOVE.W	TDU_NUMSECS(A3),D3	; new sectors till gap

		;-- enter with a4 -> format word of a sector
		;-- d3 new sectors till gap including current
		;-- d2 old sectors till gap (or till end) not including current
AllignWLoop
		MOVE.L	A4,A0
		BSR	TDMfmLongDecode		; get old format
		MOVE.L	D0,D4			; save format word
		MOVE.B	D3,D0			; FMT_SECOFF is 4th byte
		MOVE.L	A4,A0
		BSR	TDMfmLongEncode		; put new format

		;-- now recompute the header checksum
		MOVE.L	A4,A0
		MOVEQ	#MFMFMT_HDRSUM,D1
		BSR	TDMfmSumBuffer (D1,A0)	; sum returned in d0
		LEA	MFMFMT_HDRSUM(A4),A0
		BSR	TDMfmLongEncode (D0,A0)	; put encoded checksum

		;-- bit used to be fixed below - a4 points to fmt byte
		LEA	MFMSEC_ZEROS-MFMSEC_FMT(A4),a0	; ptr to SEC_ZEROS
		MOVE.L	#NULLS,(a0)		; set up header
		BSR	TDMfmFixBit  (a0)	; May need to be $2aaaaaaa!!!
						; doesn't modify a0!
		MOVE.L	#FUNNYA1,4(a0)		; add $4489's - paranoia

		;-- loop
		LEA	MFM_RAWSECTOR(A4),A4	; to next format word
		SUBQ.W	#1,D3
AllignWLoopEnd:	DBRA	D2,AllignWLoop

*		;-- we have done up to the gap
*		;-- move the sectors after the gap down, then
*		;-- do their headers too.
		TST.W	D3
		BEQ.S	AllignWDone	; all sectors before gap done

		;-- d3 > 0
		;-- first allign the sectors after the gap
		;-- note that the first sector after the gap may have
		;-- one or two $4489's preceeding it.
		SUBQ.L	#8,A4		; point to first word of gap

		;-- A4 has the destination, D4 has last format word
		;-- use D4 to find the right entry in TDB_SECPTRS for
		;-- the sector after the gap.
		MOVE.L	D4,D0
		LSR.W	#8,D0
		AND.W	#$00ff,D0	; IncSector doesn't affect regs
		ASL.L	#2,D0		; longword array
		BSR.S	IncSector	; we want the next one (hits d0/d1 only)
					; IncSector deals with 4*sector
					; d0.w = new sec times 4, d1 trash

		;-- this points D2 to the format word of the source
		MOVE.L	TDB_SECPTRS(A2,D0.W),D2	  ; sector after gap
		MOVE.L	A4,D5			  ; save addr of former gap

		;-- bit used to be fixed below
		MOVE.L	A4,A0			; where sector after gap goes
		MOVE.L	D0,D4			  ; save sector number
		MOVE.L	#NULLS,(A4)+		  ; set up header
		BSR	TDMfmFixBit		; May need to be $2aaaaaaa!!!
		MOVE.L	#FUNNYA1,(A4)+		; add $4489's

		;-- Fix the secptrs for the sectors after the gap
		;-- a4 has ptr to fmt word of destination
		;-- d2 has ptr to first sector after gap (fmt word)
		;-- D3 has number of sectors (must be at least 1)
		;-- D4 has first sector number * 4 after gap (dead)
		;-- a0-A1/d0-D1 are free
		MOVE.L	D2,D6
		MOVE.L	D2,A0
		MOVE.L	D4,D0		; restore regs
		SUB.L	A4,D6		; offset for secptrs
		MOVE.W	D3,D2		; sectors to do

Fix_Ptrs	SUB.L	D6,TDB_SECPTRS(A2,D0.W)
		BSR.S	IncSector	; deals with 4*sector (hits d0/d1 only)
		SUBQ.W	#1,D2			; done yet?
		BNE.S	Fix_Ptrs		; includes first sector

		;-- Now we need to move some sectors from A0 to A4
		;-- D3 has new sectors to gap for first sector after gap
		;-- (including current sector, no need to add 1)
		MOVE.W	D3,D0

		;-- D0 has sectors to move
		MULU	#MFM_RAWSECTOR,D0
		SUBQ.W	#8,D0			; already have 0000A1A1
		MOVE.L	A4,A1

		;-- copy using CopyMem (can't use quick, not LONG alligned)
		MOVE.L	A6,-(SP)
		MOVE.L	TD_SYSLIB(A6),A6
		JSR	_LVOCopyMem(A6) (a0,a1,d0)
		MOVE.L	(SP)+,A6

		;-- Ok, now set things up to jump back into the
		;-- sectors to gap fixing loop
		;-- D3 is still correct, A4 is correct, D5 is set
		MOVE.W	D3,D2
		BRA.S	AllignWLoopEnd	; will decrement d2 for us

		;-- all copied, now do other chores
		;-- D5 has addr of sector (fmt) that used to be after gap
		;-- A4 points to the word 8 bytes after the last sector
AllignWDone:
* now done above (see FixBit)
*		;-- Fix up the SEC_NULL field of the old first sector
*		;-- of the track
*		MOVE.L	D5,A0
*		SUBQ.L	#8,A0		; point at SEC_NULL
*		BSR	TDMfmFixBit

*		;------ Force the buffer to end on AAA8 (or 2AA8)
		;------ The reason it is not AAAA is because paula
		;------ drops write gate three bits before the serial
		;------ shift out actually stops.  The software must 
		;------ ensure that the last three bits are nulls.
		SUBQ.L	#8,a4		; point to word after last sector
		MOVE.W	#$AAA8,D0
		BTST.B	#0,-1(A4)
		BEQ.S	AllignOK12

*		;------ unset the clock bit
		BCLR	#15,D0
AllignOK12:
		MOVE.W	D0,(A4)

		MOVEM.L	(SP)+,A4/D2/D3/D4/D5/D6
		RTS

		;-- increment sector number in D0 (times 4!)
		;-- affects D1 only!
IncSector:
		ADDQ.W	#4,D0			; next sector number
		MOVE.W	TDU_NUMSECS(a3),D1
		LSL.W	#2,D1			; *4
		CMP.W	D1,D0			; check for overflow
		BMI.S	1$			; bra if d0 < NUMSECS * 4
		CLEAR	D0			; else sector 0
1$		RTS

	END

