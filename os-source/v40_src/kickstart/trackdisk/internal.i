
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* Source Control
* ------ -------
* 
* $Id: internal.i,v 33.19 92/04/05 19:45:01 jesup Exp $
*
* $Locker:  $
*
* $Log:	internal.i,v $
* Revision 33.19  92/04/05  19:45:01  jesup
* Added new TDT defines (STEP,SETTLE,CALIBRATE)
* Bumped post-write to 3ms, sidesel to 1.5ms (damn broken A1010's!)
* (Panasonic/Matsushita JU 363-xxx's may require 4ms!)
* 
* Revision 33.18  91/04/14  00:39:41  jesup
* Bumped post-write delay back to 2000us - old Panasonic drives need this
* 
* Revision 33.17  91/03/13  20:46:29  jesup
* Added the ALLBAD bit
* 
* Revision 33.16  91/01/17  15:07:09  jesup
* added unreadable bit
* 
* Revision 33.15  90/11/28  23:51:26  jesup
* TDB_DATA no longer a constant, now a variable
* 
* Revision 33.14  90/11/21  04:21:40  jesup
* Changes for multi-density drives
* 
* Revision 33.13  90/06/01  23:15:33  jesup
* Conform to include standard du jour
* 
* Revision 33.12  90/06/01  16:32:33  jesup
* comment change
* 
* Revision 33.11  90/03/16  01:20:50  jesup
* Added new flags to track buffer structure
* Added to dicussion of disk issues
* 
* Revision 33.10  90/03/01  23:01:30  jesup
* Added comments
* 
* Revision 33.9  89/12/10  18:37:30  jesup
* moved current trk and counter to public unit, added ptr for chip mem buf
* 
* Revision 33.8  89/09/06  18:55:51  jesup
* Changed TDT_DISKSYNC
* 
* Revision 33.7  89/05/15  21:19:13  jesup
* Backed off SIDESEL delay to 1000us (1ms) from 100us.  100us causes
* spurious R/W errors when a disk is inserted in another drive during a
* loadseg of preferences on my '030 (sometimes).  1000us seems to solve
* the problem.  ARGH.
* 
* Revision 33.6  89/03/23  14:22:13  jesup
* no change, just a mark
* 
* Revision 33.5  89/03/22  17:34:32  jesup
* Modified private unit structure for alignment
* 
* Revision 33.4  89/02/17  19:11:41  jesup
* modified track buffer structure, etc for use with sync reads.
* TD_TRKBUF made according to calculations, not random anymore.
* 
* Revision 33.3  86/04/10  00:57:26  neil
* Added AddChangeInt and RemChangeInt
* 
* Revision 33.2  86/04/03  23:35:22  neil
* made part of unit structure public
* 
* Revision 33.1  86/03/29  14:13:55  neil
* made seek and settle time programmable.  Isolated unit specific
* initializers to the beginning of the unit structure
* 
* Revision 32.2  86/01/03  19:52:13  neil
* Added reset catching code
* 
* Revision 32.1  85/12/23  17:19:28  neil
* Added rawread/rawwrite
* 
* Revision 30.2  85/10/09  14:57:23  neil
* comment changes
* 
* Revision 30.1  85/10/09  00:36:28  neil
* Another internal (for 1.01)
* 
* Revision 27.2  85/08/07  16:25:12  carl
* TDT_SIDESEL changed to 1000 us.
* 
* Revision 27.1  85/06/24  13:37:06  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  15:13:53  neil
* *** empty log message ***
* 
* 
*************************************************************************

ErrorMonitor	EQU	0
ERRORMSGS	EQU	0
INFO_LEVEL	EQU	20


*--------------------------------------------------------------------
*
* Defines for AMIGA0.0 track format
*
*--------------------------------------------------------------------

* label size must be a unit of 16 bytes (because of TDMfmEncode and Decode)
 
FMT_SECTYPE	EQU	$ff

*;-- format word for the ID longword
 STRUCTURE FMT,0
	UBYTE	FMT_TYPE			; always FF
	UBYTE	FMT_TRACK			; track number
	UBYTE	FMT_SECTOR			; sector number
	UBYTE	FMT_SECOFF			; sec offset from start of read
	STRUCT	FMT_SECLABEL,TD_LABELSIZE	; OS recovery info
	ULONG	FMT_HDRSUM			; header checksum
	LABEL	FMT_SIZE


*;-- format for the sector as it appears BEFORE mfm encoding
* sector header must be in units of 32 bytes (because of TDMfmAllignSector)
 STRUCTURE SEC,0
	UWORD	SEC_ZEROS
	UWORD	SEC_A1
	STRUCT	SEC_FMT,FMT_SIZE
	ULONG	SEC_DATASUM
	LABEL	SEC_DATA
	LABEL	SEC_HDRSIZE

MFMTD_LABELSIZE	EQU	TD_LABELSIZE*2

MFMFMT_TYPE	EQU	FMT_TYPE*2
MFMFMT_TRACK	EQU	FMT_TRACK*2
MFMFMT_SECTOR	EQU	FMT_SECTOR*2
MFMFMT_SECOFF	EQU	FMT_SECOFF*2
MFMFMT_SECLABEL EQU	FMT_SECLABEL*2
MFMFMT_HDRSUM	EQU	FMT_HDRSUM*2
MFMFMT_SIZE	EQU	FMT_SIZE*2

MFMSEC_ZEROS	EQU	SEC_ZEROS*2
MFMSEC_A1	EQU	SEC_A1*2
MFMSEC_FMT	EQU	SEC_FMT*2
MFMSEC_DATASUM	EQU	SEC_DATASUM*2
MFMSEC_DATA	EQU	SEC_DATA*2
MFMSEC_HDRSIZE	EQU	SEC_HDRSIZE*2


*--------------------------------------------------------------------
*
* Useful constants
*
*--------------------------------------------------------------------

*
* disc size note:  A perfect disc spins in 200 milliseconds (+/- 1.5 %)
*	and each bit on it takes four microseconds.  This results in
*	6250 (+/- 94) bytes of storage on the disc.  However on our system
*	the clock rate is 2.3% fast. Also, we want to test the system
*	at a 5% fast speed.  This gives us a speed error budget of
*	1.015 * 1.023 * 1.05 = 1.09 speed variation.  Note that when the
*	disk spins faster, we can fit less bytes on a track, when it spins
*	slower, we can fit more bytes on a track.
*
*	We also tolerate a 5% slow clock.  Overall, we have:
*		+- 1.5%	disk speed slop
*		+  2.3%	nominal timing is 2.3% fast
*		+- 5.0%	timing slop
*
*	The minimum number of bytes we can lay on a track is when disk and
*	clock subtract. This gives .96 * nominal bytes, or 5999 bytes that
*	can be written.  The maximum numbers of bytes that can be written
*	is when disk and clock add.  This gives 1.09 * nominal, or 6812
*	bytes.  We must write enough nulls for the gap to at least equal
*	this number.  This number is a mimimum of 824 bytes, so we use 830.
*
*	The 5% is needed for timing variations due to machine-to-machine
*	differences, PAL/NTSC, temperature, and Genlock.
*
*	The buffer is laid out as:
*	--------------------------
*
*	nulls for write | 4 byte pad | used track | slop + sector for read
*	   830		     4		5984		830 + 544
*
*	The "+ sector for read" is because we read starting at "used track",
*	and may have 543 bytes before we see the first beginning of sector.
*	Note that this only applies to non-wordsync reads.  Writes start at
*	the beginning of the buffer.  For sync reads, it is possible that
*	a bit flip caused a sync mark to appear at the beginning of a sector,
*	so, we should still keep enough memory for 1 full extra sector if we
*	want to be certain of reading the entire track.  We don't, since
*	we'll end up retrying instead (such a case is low probability, and will
*	slow down all reads otherwise, and take 1K extra memory per disk
*	drive).  We must deal with the first SYNC being lost, since we may turn
*	on DMA in the middle of the first sync (about 1/1000).  This causes us
*	to get no syncs in the buffer, and must align differently for write
*	(either move the track up a word to insert the missing sync, or put the
*	NULLS and syncs a word lower and write 2 less bytes.)
*
*	Unlike most MFM systems, $6db6 is NOT the worst-case pattern.  This is
*	because we seperate the even and odd bits.  So the worst-case pattern
*	is $6db6 with each bit doubled, or $f0f3cff0.
*

*-- sizes before mfm encoding
*-- NOTE: these are for 1Meg floppies!!!  double for 2M (1.44M/1760K)

TD_NOMTRACK	EQU	6250		; one nominal track
TD_SLOP		EQU	830		; ~ 1.09 * NOMTRACK - TD_USEDTRACK
TD_RAWSECTOR	EQU	TD_SECTOR+SEC_HDRSIZE
TD_USEDTRACK	EQU	11*(TD_RAWSECTOR)	; the used part of a track
TD_MAXTRACK	EQU	TD_USEDTRACK+TD_SLOP	; max possible track
TD_TRKBUF	EQU	TD_SLOP+4+TD_MAXTRACK	; was 16*TD_SECTOR ; !!! 8k
						; +4 for space for final $aaa8

TD_BIG_NOMTRACK	EQU	6250*2		; one nominal track
TD_BIG_SLOP	EQU	830*2		; ~ 1.09 * NOMTRACK - TD_USEDTRACK
TD_BIG_USEDTRACK EQU	22*(TD_RAWSECTOR)	; the used part of a track
TD_BIG_MAXTRACK	EQU	TD_BIG_USEDTRACK+TD_BIG_SLOP	; max possible track
TD_BIG_TRKBUF	EQU	TD_BIG_SLOP+4+TD_BIG_MAXTRACK
						; +4 for space for final $aaa8
*-- sizes after mfm encoding
MFM_NOMTRACK	EQU	2*TD_NOMTRACK
MFM_SLOP	EQU	2*TD_SLOP
MFM_SECTOR	EQU	2*TD_SECTOR
MFM_SECSHIFT	EQU	TD_SECSHIFT+1	; log  MFM_SECTOR
					;    2
MFM_BIG_NOMTRACK EQU	2*TD_BIG_NOMTRACK
MFM_BIG_SLOP	EQU	2*TD_BIG_SLOP

*					;    2
MFM_RAWSECTOR	EQU	2*TD_RAWSECTOR
MFM_TRKBUF	EQU	2*TD_TRKBUF
MFM_USEDTRACK	EQU	2*TD_USEDTRACK
MFM_MAXTRACK	EQU	2*TD_MAXTRACK

MFM_BIG_TRKBUF	EQU	2*TD_BIG_TRKBUF
MFM_BIG_USEDTRACK EQU	2*TD_BIG_USEDTRACK
MFM_BIG_MAXTRACK EQU	2*TD_BIG_MAXTRACK



MAX_NUMSECS	EQU	22

*--------------------------------------------------------------------
*
* Defines for unit state and status
*
*--------------------------------------------------------------------

*-- Unit Structure
 STRUCTURE TDU,TDU_PUBLICUNITSIZE	;(000)
	UBYTE	TDU_DRIVETYPE		;(xxx) the type of the drive
	UBYTE	TDU_RESERVED2		;(XXX) for alignment
	UWORD	TDU_NUMTRACKS		;(xxx) number of tracks on this drive
	ULONG	TDU_MAXOFFSET		;(xxx) max IO_POSITION allowable
					; Note: now variable
	UWORD	TDU_MFM_TRKBUF		; used to be MFM_TRKBUF equate
	UWORD	TDU_MFM_MAXTRACK	; used to be MFM_MAXTRACK+2 equate
					; (the +2 is for the $aaa8 on the end!)
	UWORD	TDU_MFM_SLOP		; used to be MFM_SLOP equate
	UWORD	TDU_NUMSECS		; used to be 11
	ULONG	TDU_TDT_DISKSYNC	; used to be TDT_DISKSYNC
	ULONG	TDU_DISKTYPE		;(xxx) as returned from disk resource
					; Note: now variable
	UBYTE	TDU_FLAGS		;(xxx) unit flags
	UBYTE	TDU_6522		;(xxx) word to write to 6522
	UBYTE	TDU_RETRY		;(xxx) count of retries
	UBYTE	TDU_UNITNUM		;(xxx) the number of our unit
	ULONG	TDU_IOBLOCK		;(xxx) holds current IOBlock
	UBYTE	TDU_STATE		;(xxx) current state of the driver
	UBYTE	TDU_SECTOR		;(xxx) sector being operated on
	UWORD	TDU_TRACK		;(xxx) track being operated on
*	UWORD	TDU_SEEKTRK		;(xxx) track the heads are over
TDU_SEEKTRK	EQU	TDU_CURRTRK
	APTR	TDU_BUFFER		;(xxx) track buffer in use
	APTR	TDU_BUFPTR		;(xxx) ptr to allocated bufs for
*					;	this unit
	APTR	TDU_CHIPBUF		; ptr to 1 sector of chip mem - REJ
	APTR	TDU_DATA		;(xxx) current ptr to user data
	APTR	TDU_LABELDATA		;(xxx) current ptr to label data
	STRUCT	TDU_WAITTIMER,IOTV_SIZE	;(xxx) to request sleeps
	STRUCT	TDU_CHANGETIMER,IOTV_SIZE ;(xxx) to check for disc change
	STRUCT	TDU_WAITPORT,MP_SIZE	;(xxx) message port for replies
	STRUCT	TDU_DRU,DRU_SIZE	;(xxx) resource unit
	ULONG	TDU_REMOVEINT		;(xxx) soft int to call on removal
	STRUCT	TDU_TCB,TC_SIZE		;(xxx) tcb for disc task
	APTR	TDU_ENTRY		;(xxx) ptr to MemEntry for unit
	STRUCT  TDU_CHANGELIST,LH_SIZE  ;(xxx) list to hold change ints


 IFNE ErrorMonitor
*	;-- temporary stuff to monitor errors
	STRUCT	TDU_SOFTERRS,2*NUMTRACKS	; word array, one per track
	STRUCT	TDU_NUMERRS,2*NUMTRACKS
	STRUCT	TDU_HARDERRS,2*NUMTRACKS
*	;-- end error monitoring stuff
 ENDC ErrorMonitor

	LABEL	TDU_SIZE

*-- Flags for TDU_FLAGS:

	BITDEF	TDU,SECLABEL,0			; writting the sector label
	BITDEF	TDU,REMOVED,1			; the disc is out of the drive
	BITDEF	TDU,EXTENDED,2			; doing an extended command
	BITDEF	TDU,DELMOTOROFF,3		; turn the motor off eventually
	BITDEF	TDU,PROTECTED,4			; disk is write protected
	BITDEF	TDU,STOPPED,5			; Unit is stopped
	BITDEF	TDU,LARGE,6			; unit has large buffer
	BITDEF	TDU,UNREADABLE,7		; can't read this disk

*-- Flags for io_Flags:
	;------ this is set if the IO Command happened NOW (like quick...)
	BITDEF	IO,IMMEDIATE,7

*-- Track Buffer structure
 STRUCTURE TDB,0
	UWORD	TDB_TRACK
	UBYTE	TDB_FLAGS
	UBYTE	TDB_ALLIGN			; # of first sector in buffer
	APTR	TDB_DATA			; ptr past slop
	STRUCT	TDB_SECPTRS,MAX_NUMSECS*4	; pointers to the word after 4489
	STRUCT	TDB_SECERRS,((MAX_NUMSECS+3)/4)*4 ; errors associated with each
	STRUCT	TDB_BUF,MFM_TRKBUF
	LABEL	TDB_SIZE

*-- larger Track Buffer structure
 STRUCTURE TDB_LARGE,TDB_SIZE
	STRUCT	TDB_EXTRABUF,MFM_TRKBUF
	LABEL	TDB_LARGE_SIZE

* old definition
*TDB_DATA	EQU TDB_BUF+MFM_SLOP		; start of mfm data in buffer


	BITDEF	TDB,DIRTY,0
	BITDEF	TDB,VALID,1
	BITDEF	TDB,HASERRS,2
	BITDEF	TDB,OFFSET_2,3			; Trk starts at TDB_DATA+2
	BITDEF	TDB,ALIGNED,4			; trk already aligned for write
	BITDEF	TDB,ALLBAD,5			; track is unreadable

*-- Device Structure
 STRUCTURE TD,DD_SIZE
	UBYTE	TD_FLAGS		; general device flags
	UBYTE	TD_pad
	APTR	TDU0			; unit structure for unit 0
	APTR	TDU1			; unit structure for unit 1
	APTR	TDU2			; unit structure for unit 2
	APTR	TDU3			; unit structure for unit 3
	APTR	TD_SYSLIB		; ptr to system library
	APTR	TD_GFXLIB		; ptr to graphics library
	APTR	TD_DISCRESOURCE		; ptr to disc resource
	STRUCT	TD_MEMLIST,LH_SIZE	; all of driver's memory
	APTR	TD_TIMER0_DEV
	APTR	TD_TIMER0_UNIT
	APTR	TD_TIMER1_DEV
	APTR	TD_TIMER1_UNIT
	APTR	TD_CIABRESOURCE		; pointer to cia resource
	STRUCT	TD_RESETINT,IS_SIZE	; soft int when we get a reset
	STRUCT	TD_RESETIOB,IOSTD_SIZE	; to ack a reset
	LABEL	TD_SIZE

*-- TD flags
	BITDEF	TD,DELEXP,7	; expunge upon last close
	BITDEF	TD,PAULA,6	; we are running on a paula chip (vs portia)
	BITDEF	TD,WRITING,5	; we are currently writing
	BITDEF	TD,RESET,4	; someone has hit a reset

TD_STKSIZE	EQU	$200

*--------------------------------------------------------------------
*
* Time Constants (all times are in microseconds)
*
*--------------------------------------------------------------------

TDT_MOTORON	EQU	500000
TDT_DISKSYNC	EQU	300000
TDT_POSTWRITE	EQU	3000	; 2000, SPEC SAYS 1200!
TDT_SIDESEL	EQU	1500	; 1000, SPEC SAYS 100!
TDT_FORMAT	EQU	300000
TDT_STEP	EQU	3000	; default
TDT_CALIBRATE	EQU	4000	; default
TDT_SETTLE	EQU	15000	; default

*--------------------------------------------------------------------
*
* Random defines
*
*--------------------------------------------------------------------

NUM_RETRIES	EQU	10	; default # of retries


	XREF	GLQBlit

DOGFX		MACRO	* &FUNC
		LINKLIB _LVO\1,TD_GFXLIB(A6)
		ENDM

*	;------ Disc DMA bit in DMACON
	BITDEF	DMA,DISCENA,4

*	;------ Signal definitions for interfacing to driver task
*	;------ WARNING! check Bxxx instructions to memory if changed!
	BITDEF	TD,DEVDONE,8
	BITDEF	TD,PORT,9
	BITDEF	TDS,WAITTIMER,10
	BITDEF	TDS,DSKBLK,11
 

DSKLENF_DMAEN	EQU	$8000
DSKLENF_WRITE	EQU	$4000
