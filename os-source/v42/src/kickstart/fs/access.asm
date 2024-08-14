		SECTION	filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/io.i"
		INCLUDE	"libraries/dosextens.i"
		INCLUDE	"globals.i"
		INCLUDE	"private.i"
		INCLUDE	"moredos.i"
		INCLUDE	"actions.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XDEF	Access,ClearWaiting

		XREF	WaitBlock,GrabBlock,DiscardBlock,LooseBlock,WriteBlock
		XREF	Type,Locate,Create,WorkFail,ReturnPkt,WorkDone
		XREF	FreeLock,WaitCo,AddList,AlterRoot,DatStamp,Fetch
		XREF	ClearBlock,GetKey,GetBlock,_LVOCopyMem,UnHash
		XREF	GrabDataBlock,EnQueue,DeQueue,WaitDataBlock
		XREF	DiskProtected,GetLock,ParentKey,FreeKey

		XREF	LockRecord,FreeRecord,RecordAccess,FreeAllRLocks
		XREF	CheckNotify

;==============================================================================
; Since there are so many variables that are kept locally over the WaitCo()
; calls, I have to keep some of them on the stack for the Access co-routine.
;==============================================================================
	STRUCTURE AccessVars,0
		UWORD	a_IsInput	flag, reading or writing
		UWORD	a_Reading	also a reading or writing flag
		UWORD	a_HeaderSeq	seq num for header key, 0 for first
		UWORD	a_NotMyLock	if TRUE then don't free FileLock
		ULONG	a_Lock		the lock on this file (BPTR)
		ULONG	a_Protect	protection mask for this file
		APTR	a_Vector	where to read/write from/to
		ULONG	a_Size		size of Vector
		ULONG	a_Volume	BPTR to current volume
		ULONG	a_HeaderKey	current header (or extension) key
		ULONG	a_FirstKey	actual header block of this file
		ULONG	a_Pos		current position in file
		ULONG	a_SavePos	backup position in file
		ULONG	a_FileSize	size of file in bytes
		ULONG	a_DSize		data size for current read or write
		ULONG	a_Key		prev key when extending header blks
		ULONG	a_LastKey	last key read
		ULONG	a_LastDotKey	last key written (name historical)
		ULONG	a_DKey
		ULONG	a_DVec
		ULONG	a_FileHandle	used for record locking
	LABEL AccessVars_SIZEOF

;==============================================================================
; The main access co-routine.  Is called first during action.findinput or
; action.findoutput.  Locates or Creates the file and if successful, puts the
; address of itself into the filehandle so that subsequent end, seek, read
; or write actions come back to this co-routine for execution.  No registers
; are saved because this is a co-routine and regs are saved by colib.asm
;
; Severe modifications have been performed to make this work with the old
; filing system too.  Specifically, allowance for 24 bytes of crud on the
; beginning of data blocks.
;==============================================================================
Access		moveq.l	#(AccessVars_SIZEOF>>2)-1,d1
5$		clr.l	-(sp)
		dbra	d1,5$

		movea.l	d0,a2			save the packet address

; ACTION_OPEN_LOCK requires dp_Arg1 as a FileHandle and dp_Arg2 as a Lock
		cmpi.w	#ACTION_OPEN_LOCK,dp_Type+2(a2)
		bne.s	7$

		move.l	dp_Arg2(a2),d0		get the provided lock
		move.l	d0,a_Lock(sp)		and save it
		lsl.l	#2,d0			convert to APTR
		movea.l	d0,a0
		cmpi.l	#exclusive.lock,fl_Access(a0)
		beq.s	6$
		move.w	#TRUE,a_IsInput(sp)	a shared read lock
6$		move.l	a_Lock(sp),d0		retrieve lock pointer
		bra.s	30$			and carry on with access

7$		cmpi.w	#ACTION_FIND_OUTPUT,dp_Type+2(a2)
		bne.s	10$			sorry, it was input
		bsr	Create			create required object
		bra.s	20$
10$		move.w	#TRUE,a_IsInput(sp)	it was input (flag TRUE)
		bsr	Locate			go Locate required object
20$		movea.l	a2,a0			assume we will fail
		move.l	d0,a_Lock(sp)		did we get our lock ?
		beq	WorkFail		no, so kill this co-routine

; we have a lock BPTR so now set up a few important values and check that
; the file we have locked is valid.  Lock BPTR is still in d0 here.
30$		lsl.l	#2,d0			convert lock to APTR
		movea.l	d0,a0

		move.l	fl_Key(a0),a_HeaderKey(sp)	header key for file
		move.l	fl_Key(a0),a_FirstKey(sp)	also the first key
		move.l	CurrentVolume(a5),d0
		move.l	d0,a_Volume(sp)		stash current volume
		cmp.l	fl_Volume(a0),d0	is volume correct
		bne.s	35$			wrong volume in Lock !!!
		move.l	a_HeaderKey(sp),d0
		bsr	WaitBlock		get the file header
		movea.l	d0,a3			stash it a while
		movea.l	d0,a0
		bsr	Type			check it's the correct type
		cmpi.w	#st.file,d0		is it a file
		beq.s	40$			yep, carry on normally

; the file header we got was invalid, ie. it was probably a directory.  Return
; the packet with a failure code and an error of ObjectWrongType.
35$		movea.l	a_Lock(sp),a0		free this lock
		bsr	FreeLock
		movea.l	a3,a0			free up the block we read
		bsr	LooseBlock
		move.w	#ERROR_OBJECT_WRONG_TYPE,ErrorCode+2(a5)
		movea.l	a2,a0			fail this packet and exit
		bra	WorkFail

; our lock and file type are valid.  Initialise some variables that are updated
; throughout the access co-routine, fill in the address of this co-routine for
; subsequent access actions and return a successful packet.
40$		move.l	cb_SIZEOF+fhb_ByteSize(a3),a_FileSize(sp)
		move.l	cb_SIZEOF+fhb_Protect(a3),a_Protect(sp)
		movea.l	a3,a0
		bsr	LooseBlock		block is valid
		movea.l	dp_Arg1(a2),a0		get filehandle BPTR
		move.l	a0,a_FileHandle(sp)	and save it for rlocking code
		adda.l	a0,a0			convert to APTR
		adda.l	a0,a0
		move.l	CurrentCo(a5),fh_Args(a0) fill in our CoBase in handle
		moveq.l	#TRUE,d0		and return OK
		moveq.l	#0,d1			res2 not important
		movea.l	a2,a0
		bsr	ReturnPkt		return this packet

;==============================================================================
; The file we are working on has been found or created.  Sits here waiting for
; an action to perform on this file.  The filehandle has been filled in with
; the stackbase of this co-routine so any requests for this file will CallCo
; back to here.
;==============================================================================
AccessLoop	clr.w	a_Reading(sp)		reading = FALSE
		clr.l	a_DSize(sp)		data size = 0
		clr.l	a_LastKey(sp)		last key = 0
		bsr	WaitCo			wait for another packet
		movea.l	d0,a2			packet to a2
		move.l	dp_Type(a2),d0		get the command
		cmpi.w	#ACTION_CURRENT_VOLUME,d0
		bne.s	13$

;==============================================================================
; This is a request for the volume node associated with the current filehandle.
;==============================================================================
		move.l	a_Volume(sp),d0		res1=volnode
		move.l	UnitNumber(a5),d1	res2=unit number
		movea.l	a2,a0
		bsr	ReturnPkt
		bra.s	AccessLoop		that's all

;==============================================================================
; Before we execute any other commands, make sure the same volume is still here
;==============================================================================
13$		move.l	a_Volume(sp),d1		if 0
		beq.s	10$			skip equality test !!!
		cmp.l	CurrentVolume(a5),d1
		beq.s	10$			volume is the same
		moveq.l	#FALSE,d1		assume action end
		cmpi.w	#ACTION_END,d0		if act.end return FALSE
		beq.s	15$
		moveq.l	#TRUE,d1		else return TRUE
15$		move.l	d1,d0
		move.l	#ERROR_DEVICE_NOT_MOUNTED,d1
		movea.l	a2,a0
		bsr	ReturnPkt
		bra	AccessLoop		loop for more commands

;==============================================================================
; New addition for 1.4, duplicate the lock associated with this FileHandle
;==============================================================================
10$		cmpi.w	#'R',d0			read/write should be quick...
		beq	AccessRW		...so check them before all...
		cmpi.w	#'W',d0			...the new stuff is tested for
		beq	AccessRW

		cmpi.w	#ACTION_COPY_DIR_FH,d0	DupLockFromFH?
		bne.s	12$			nope
		movea.l	a_Lock(sp),a0
		adda.l	a0,a0
		adda.l	a0,a0
		move.l	fl_Key(a0),d0
11$		moveq.l	#shared.lock,d1
		bsr	GetLock			maybe duplicate this lock
		movea.l	a2,a0
		bsr	ReturnPkt		d0 = lock or NULL
		bra	AccessLoop

;==============================================================================
; New addition for 1.4, return lock on the parent of this file
;==============================================================================
12$		cmpi.w	#ACTION_PARENT_FH,d0
		bne.s	20$
		movea.l	a_Lock(sp),a0		we know this Lock's OK
		adda.l	a0,a0
		adda.l	a0,a0
		move.l	fl_Key(a0),d0
		bsr	ParentKey		find parent key
		bra.s	11$			and get a shared lock on it

;==============================================================================
; Time to do a seek?  Arg3  <0=from start, 0=from current, >0=from end
;==============================================================================
20$		cmpi.w	#ACTION_SEEK,d0		seek command ?
		bne	AccessEnd		no, check action end
		move.l	dp_Arg2(a2),d2		offset required
		move.l	dp_Arg3(a2),d1		seek mode
		bmi.s	30$			<0 so seek from start
		bne.s	25$			>0 so from end
		add.l	a_Pos(sp),d2		=0 so from current
		bra.s	30$
25$		add.l	a_FileSize(sp),d2
30$		tst.l	d2			check seek is valid
		bmi.s	40$			<0 so fail it
		cmp.l	a_FileSize(sp),d2
		ble.s	50$			in range, so do the seek

; the seek position was <0 or beyond the end of the file, so fail the packet
40$		movea.l	a2,a0
		moveq.l	#-1,d0			old seek position invalid
		move.l	#ERROR_SEEK_ERROR,d1
		bsr	ReturnPkt
		bra	AccessLoop		and loop for more

; seek position checked out OK.
50$		move.l	d2,d3			byte offset to block offset
		tst.b	DiskType+3(a5)		DOS/0 or DOS/1
		beq.s	54$

		moveq.l	#9,d1			it's DOS/1
		lsr.l	d1,d3
		bra.s	55$

******* must change this to a 32 bit divide sometime *************************
54$		divu	#488,d3			it's DOS/0
		ext.l	d3			imposes a limit of 31Mb
******************************************************************************

55$		tst.l	d3			if required key is 0 ....
		bne.s	60$
		clr.w	a_HeaderSeq(sp)		... don't search blocks
		move.l	a_FirstKey(sp),a_HeaderKey(sp)
		bra.s	90$			set pos = d2 and return pkt OK

; we are going to have to chain through the extension blocks to look for the
; header key of the block we want.  If we are already past the extension block
; we require then we will have to rewind to the header key because the
; extension blocks are not back-chained.  First find the logical sequence
; number we require.
60$		divu.w	#72,d3			divide by hash table size
		cmp.w	a_HeaderSeq(sp),d3	if < HeaderSeq ...
		bge.s	70$
		clr.w	a_HeaderSeq(sp)		... then rewind
		move.l	a_FirstKey(sp),a_HeaderKey(sp)

; loop around reading blocks until we get the extension block we want
70$		move.l	a_HeaderKey(sp),d0	grab this block
75$		bsr	GrabBlock
		movea.l	d0,a0			stash buffer address
		move.l	cb_SIZEOF+fhb_Extension(a0),d4
		beq.s	80$			reached end of chain
		cmp.w	a_HeaderSeq(sp),d3	got required key ?
		beq.s	80$			yes
		move.l	d4,a_HeaderKey(sp)	new header key
		addq.w	#1,a_HeaderSeq(sp)	move HeaderSeq up 1
		bsr	LooseBlock		free block in a0
		move.l	d4,d0			need this block next
		bra.s	75$			and keep going

; we have seeked to where we want so free the last block we grabbed
80$		bsr	LooseBlock		block still in a0

; everything completed.  Required seek position is in d2, make this the new pos
90$		move.l	a_Pos(sp),d0		return old position
		move.l	d2,a_Pos(sp)		set new position
		movea.l	a2,a0			return packet
		moveq.l	#0,d1
		bsr	ReturnPkt
		bra	AccessLoop		look for more work

;==============================================================================
; wasn't seek or current volume so check if we've finished everything. d0=cmd.
;==============================================================================
AccessEnd	cmpi.w	#ACTION_END,d0
		bne	AccessExamine
		bsr	ClearWaiting		move waiting to pending queue

		movea.l	a_FileHandle(sp),a0
		movea.l	a_Lock(sp),a1
		bsr	FreeAllRLocks		make sure we kill record locks

; if we wrote to this file then update filesize and datestamp in the header key
		tst.w	a_IsInput(sp)		did we write
		bne.s	NotWritten		nope, it was an input file
		bsr	AlterRoot		mark bitmap as invalid etc.

		move.l	a_FirstKey(sp),d0	get the header key
		bsr	GrabBlock
		movea.l	d0,a3			stash buffer address
		move.l	cb_SIZEOF+fhb_ByteSize(a3),d0
	move.l a_FileSize(sp),d1
	printf <'action_end: fhb_DataSize=%ld FileSize=%ld\n'>,d0,d1
		cmp.l	a_FileSize(sp),d0
		bge.s	SizeOK			no need to change filesize
		move.l	a_FileSize(sp),cb_SIZEOF+fhb_ByteSize(a3)
SizeOK		lea.l	cb_SIZEOF+fhb_Days(a3),a0
		bsr	DatStamp		fill in the new date
		bclr.b	#4,cb_SIZEOF+fhb_Protect+3(a3)   *** clear archive ***
		move.l	cb_SIZEOF+fhb_Parent(a3),-(sp)	save parent
		movea.l	a3,a0
		move.l	4+a_FirstKey(sp),d0	this key
	printf <'Writing header to %ld\n'>,d0
		moveq.l	#BUFF_INVALID,d1	put it on the pending queue
		moveq.l	#TRUE,d2		checksum it first
		bsr	WriteBlock		and write the block

; now check if anyone wants notification about this file (sp) holds parent key
		move.l	4+a_FirstKey(sp),d0
		moveq.l	#FALSE,d1		don`t rebuild (done by Create)
		bsr	CheckNotify		this checks non-orphans
		move.l	(sp)+,d0		check orphaned lists too
		moveq.l	#TRUE,d1		and rebuild thier lists
		bsr	CheckNotify

; everything done for this file.  Free up the lock and commit suicide.
NotWritten	movea.l	a_Lock(sp),a0
		bsr	FreeLock
10$		movea.l	a2,a0			RECORD_LOCKED error possible
		bra	WorkDone		return true packet and die

;==============================================================================
; ACTION_EXAMINE_FH.  Returns a filled in FileInfoBlock just as if Examine();
; had been called using a lock instead of the FileHandle.  A lot of code has
; been duplicated that was already in ExInfo but it was much easier to put
; it all here so it didn't affect the speed of ExNext with lots of checking.
;==============================================================================
AccessExamine	cmpi.w	#ACTION_EXAMINE_FH,d0
		bne	AccessSetSize
		movea.l	dp_Arg2(a2),a4		get FileInfoBlock
		adda.l	a4,a4
		adda.l	a4,a4
		move.l	a_FirstKey(sp),d0	get the header key
		move.l	d0,fib_DiskKey(a4)	key of where this block is
		bsr	GrabBlock
		movea.l	d0,a3			save the pointer
		movea.l	d0,a0
		bsr	Type			check type (always a file!!)
		move.l	d0,fib_DirEntryType(a4)	save directory type
		move.l	d0,fib_EntryType(a4)	and plain type
; name is 8 longwords long
		movem.l	cb_SIZEOF+fhb_FileName(a3),d0-d7
		movem.l	d0-d7,fib_FileName(a4)

; comment is 20 longwords long
		clr.w	fib_Comment(a4)		assume none
		tst.b	cb_SIZEOF+fhb_Comment(a3)
		beq.s	AccessDoDate			no comment to copy
		movem.l	cb_SIZEOF+fhb_Comment(a3),d0-d7
		movem.l	d0-d7,fib_Comment(a4)
		movem.l	cb_SIZEOF+fhb_Comment+32(a3),d0-d7
		movem.l	d0-d7,fib_Comment+32(a4)
		movem.l	cb_SIZEOF+fhb_Comment+64(a3),d0-d3
		movem.l	d0-d3,fib_Comment+64(a4)
; date is 3 longwords long
AccessDoDate	movem.l	cb_SIZEOF+fhb_Days(a3),d0-d2
		movem.l	d0-d2,fib_DateStamp(a4)	fill in the date

		move.l	a_Size(sp),d0		we are holding size locally!!!
		move.l	d0,fib_Size(a4)
		tst.b	DiskType+3(a5)		is this DOS/0
		bne.s	17$			nope

		divu.w	#488,d0			different calc for DOS\0
		move.l	d0,d1
		ext.l	d0
		swap	d1
		tst.w	d1
		beq.s	18$
		addq.l	#1,d0			allow for partial blocks
		bra.s	19$			but NOT for link tables

; The old filing system doesn't include link tables in the blocks used count
17$		moveq.l	#9,d1
		lsr.l	d1,d0			convert to num blocks
		move.l	#511,d1			allow for a partial block too
		and.l	fib_Size(a4),d1
		beq.s	18$			no partial block used
		addq.l	#1,d0

18$		move.l	d0,d1			get number of ext blocks
		divu.w	#72,d1			72 = hash table size
		ext.l	d1
		add.l	d1,d0			add to total blocks

19$		move.l	d0,fib_NumBlocks(a4)	stash blocks used by file
		move.l	cb_SIZEOF+fhb_Protect(a3),fib_Protection(a4)
20$		movea.l	a3,a0
		bsr	LooseBlock		block back on valid queue
		clr.w	ErrorCode+2(a5)		nothing wrong
		moveq.l	#TRUE,d0		everything worked
		movea.l	a2,a0			return packet
		moveq.l	#0,d1			no secondary code
		bsr	ReturnPkt
		bra	AccessLoop		look for more work

;==============================================================================
; SetFileSize(file,size,mode)
;
; mode (dp_Arg3)  <0=from start, 0=from current, >0=from end
;
; Either truncates or extends a file.  The file MUST be exclusively locked
; or we could mess up other reader/writers who are holding thier own ideas
; about the file size.  If truncation is before the current seek position
; then current seek position is adjusted to the new truncation point.
;==============================================================================
AccessSetSize	cmpi.w	#ACTION_SET_FILE_SIZE,d0
		bne	AccessLocking
		movea.l	a_Lock(sp),a0		check that lock is exclusive
		adda.l	a0,a0
		adda.l	a0,a0
		cmpi.l	#exclusive.lock,fl_Access(a0)
		beq.s	SetSizeOK

		move.l	#ERROR_OBJECT_WRONG_TYPE,d1
		moveq.l	#FALSE,d0
		movea.l	a2,a0
		bsr	ReturnPkt
		bra	AccessLoop

; before we do anything, we have to make sure the file header is up to date.
SetSizeOK	move.l	a_FileSize(sp),d3	note current size
		tst.w	a_IsInput(sp)		did we write
		bne.s	10$			nope, it was an input file
		bsr	AlterRoot		mark bitmap as invalid etc.

		move.l	a_FirstKey(sp),d0	get the header key
		bsr	GrabBlock
		movea.l	d0,a3			stash buffer address
		move.l	d3,cb_SIZEOF+fhb_ByteSize(a3)  fill in the new size
		lea.l	cb_SIZEOF+fhb_Days(a3),a0
		bsr	DatStamp		fill in the new date
		bclr.b	#4,cb_SIZEOF+fhb_Protect+3(a3)   *** clear archive ***
		movea.l	a3,a0
		move.l	a_FirstKey(sp),d0	this key
		moveq.l	#BUFF_INVALID,d1	put it on the pending queue
		moveq.l	#TRUE,d2		checksum it first
		bsr	WriteBlock		and write the block

; now check what size we should be setting the file to, using size and mode
10$		move.l	dp_Arg2(a2),d2		offset required
		move.l	dp_Arg3(a2),d1		seek mode
		bmi.s	30$			<0 so seek from start
		bne.s	25$			>0 so from end
		add.l	a_Pos(sp),d2		=0 so from current
		bra.s	30$
25$		add.l	a_FileSize(sp),d2
30$		tst.l	d2			check seek is valid
		bmi.s	40$			<0 so fail it
		cmp.l	a_FileSize(sp),d2
		ble.s	20$			in range, so do the seek

40$		movea.l	a2,a0
		moveq.l	#-1,d0			old seek position invalid
		move.l	#ERROR_SEEK_ERROR,d1
		bsr	ReturnPkt
		bra	AccessLoop		and loop for more

; now check if we are truncating or extending this file
20$		cmp.l	d2,d3
		bgt.s	ChopIt			we're truncating
		blt.s	StretchIt		we're extending
		move.l	d3,d0			set to current size
SetSizeDone	moveq.l	#0,d1			no change, return length
		move.l	d0,a_FileSize(sp)	set up current size
		move.l	d0,a_Pos(sp)		and current position
		movea.l	a2,a0
		bsr	ReturnPkt
		bra	AccessLoop

; We're truncating the file, make sure not back past the current seek position
; if it is, then adjust the current seek position to the required place
ChopIt		cmp.l	a_Pos(sp),d3
		bge.s	ChopOK			no problem
		move.l	d3,a_Pos(sp)		adjust pos
ChopOK		move.l	a_FirstKey(sp),d0
		move.l	d3,d1			this position
		bsr	Truncate		returns d0=length
		bra.s	SetSizeDone

; We're extending the file, should be no limitations other than disk full
StretchIt	move.l	a_FirstKey(sp),d0
		move.l	d3,d1			this position
		bsr	Extend
		bra.s	SetSizeDone

;==============================================================================
; Check if this is one of the record locking calls
;==============================================================================
AccessLocking	cmpi.w	#ACTION_LOCK_RECORD,d0
		bne.s	AccessUnlocking
		movea.l	a_FileHandle(sp),a0
		movea.l	a_Lock(sp),a1
		movem.l	dp_Arg2(a2),d0-d3
		bsr	LockRecord
LockingDone	move.l	ErrorCode(a5),d1
		movea.l	a2,a0
		bsr	ReturnPkt		d0=errorcode from LockRecord
		bra	AccessLoop

;==============================================================================
; Unlock a record
;==============================================================================
AccessUnlocking	movea.l	a_FileHandle(sp),a0
		movea.l	a_Lock(sp),a1
		movem.l	dp_Arg2(a2),d0-d1
		bsr	FreeRecord
		bra.s	LockingDone

;==============================================================================
; The real work starts here!  Must be a read or write request.  It is possible
; to write to a file that was opened for input.  This is a much optimised
; version that collects consecutive block reads or writes and does them all in
; one go direct to the users buffer.  Naturally, there will be no checksumming
; performed on the data blocks because they contain nothing but data.
;
; Ignore all the above comments if this is a DOS/0 disk, everything slows down
;==============================================================================
AccessRW	move.l	a_Pos(sp),a_SavePos(sp)	stash current position
		move.l	dp_Arg2(a2),a_Vector(sp) save the buffer location
		move.l	dp_Arg3(a2),a_Size(sp)	and the size to read/write

;***** new addition for 1.4, check for collisions with locked records *****
		move.l	d0,d3			save read/write command
		movea.l	a_FileHandle(sp),a0
		movea.l	a_Lock(sp),a1
		moveq.l	#0,d2			assume writing
		cmpi.w	#'W',d3
		beq.s	1$
		moveq.l	#1,d2			nope, we`re reading
1$		move.l	a_Pos(sp),d0
		move.l	a_Size(sp),d1
		bsr	RecordAccess		might truncate the read/write
		move.l	d0,a_Size(sp)		adjust size
		move.l	d3,d0			restore read/write command
**** end of new addition ******

		suba.l	a4,a4			buffer = 0
		suba.l	a3,a3			databuff = 0
		cmpi.w	#'R',d0			are we reading ?
		bne.s	10$			nope
		btst.b	#3,a_Protect+3(sp)	can we read
		beq.s	2$			yep, no problems
		move.l	#ERROR_READ_PROTECTED,d1
		bra.s	5$			error

2$		move.w	#TRUE,a_Reading(sp)	yep, set flag to true
		move.l	a_FileSize(sp),d0	truncate read to end of file
		sub.l	a_Pos(sp),d0
		cmp.l	a_Size(sp),d0
		bge.s	BlockLoop		size = min(size,filesize-pos)
		move.l	d0,a_Size(sp)
		bra.s	BlockLoop

; new addition, take notice of the read and write enable bits
5$		movea.l	a2,a0
		moveq.l	#-1,d0			read/write protect error
		bsr	ReturnPkt
		bra	AccessLoop		and loop for more

; put a fix in here, if opened MODE_OLDFILE then disk write protect not checked
10$		bsr	DiskProtected		
		move.l	d0,d1			was it OK
		bne.s	5$			Nope, disk write protected

		btst.b	#2,a_Protect+3(sp)	can we write
		beq.s	12$			yep, no problems
		move.l	#ERROR_WRITE_PROTECTED,d1
		bra.s	5$			error
12$		clr.w	a_IsInput(sp)		so we update filesize at end
		bsr	AlterRoot		writing, mark bitmap invalid

;==============================================================================
; This loop is executed once per block or part block.  Hence the optimisation.
; The little trick with a_FileSize(sp) assures that the first data block will
; be allocated as close to the header as possible, while subsequent ones are
; allocated as close to each other as possible.  (see the calls to GetKey).
;==============================================================================
BlockLoop	clr.l	a_LastDotKey(sp)	assume filesize > 0
		tst.l	a_FileSize(sp)
		bne.s	10$			assumption correct
		move.l	a_FirstKey(sp),a_LastDotKey(sp)
10$		move.l	a_Pos(sp),d2


***** different calculations for DOS/0 disks *******************
		tst.b	DiskType+3(a5)		DOS/0 or DOS/1
		bne.s	15$			DOS/1
		move.l	d2,d3			offset = pos % 488
		divu.w	#488,d3			need 32 bit routine here
		move.w	d3,d2			seq = pos/488
		swap	d3
		ext.l	d3
		ext.l	d2

		move.l	d2,-(sp)		need seq num later
		divu.w	#72,d2			hseq = seq/hashtab_SIZEOF
		move.w	d2,d4
		swap	d2
		move.w	#(fhb_Junk1-4)>>2,d5	hslot=HashTabEnd -
		sub.w	d2,d5			(seq REM hashtab_SIZEOF)
		lsl.w	#2,d5			* 4  (offset to slot entry)
		move.l	(sp)+,d2		restore seq num

		move.l	#488,d6			s = min(size,488-offset)
		bra.s	16$

; these are the precalculations for DOS/1 (Fast file system) disks
15$		move.w	#$1ff,d3		offset = pos & $1ff
		and.w	d2,d3
		moveq.l	#9,d1			seq = pos >> 9
		lsr.l	d1,d2
		divu.w	#72,d2			hseq = seq/hashtab_SIZEOF
		move.w	d2,d4
		swap	d2
		move.w	#(fhb_Junk1-4)>>2,d5	hslot=HashTabEnd -
		sub.w	d2,d5			(seq REM hashtab_SIZEOF)
		lsl.w	#2,d5			* 4  (offset to slot entry)

		move.l	#512,d6			s = min(size,512-offset)
16$		sub.w	d3,d6
		move.l	a_Size(sp),d0
		cmp.l	d0,d6
		ble.s	20$
		move.l	d0,d6
20$		tst.l	d0
		bgt.s	GetHeader		more reading left to do

;==============================================================================
; End of read or write.  Transfer any remaining data and return number of bytes
; transferred.
;==============================================================================
		move.w	a_Reading(sp),d0	read/write flag
		move.l	a_DSize(sp),d1		amount to transfer (can be 0)
		beq.s	NoTransfer3
		move.l	a_DKey(sp),d2		from this key
		movea.l	a_DVec(sp),a0		into this memory area
		bsr	Transfer
NoTransfer3	moveq.l	#FALSE,d2		construct InFile flag in d2
		move.l	a_Pos(sp),d0		pos saved for size...
		cmp.l	a_FileSize(sp),d0	... transferred calculation
		beq.s	30$			not in the file anymore
		moveq.l	#TRUE,d2		we are in the file

30$		movea.l	a2,a0			return the packet
		sub.l	a_SavePos(sp),d0	with amount transferred
		move.l	d2,d1			return infile flag too
		bsr	ReturnPkt
		cmpa.w	#0,a4			do we have buf stashed still ?
		beq	AccessLoop		no, so wait for more work
		movea.l	a4,a0			free this block
		bsr	LooseBlock		but keep on valid queue
		bra	AccessLoop		and that's all

;==============================================================================
; We still need to read some data.  Find out what key that data is on.  It's
; possible that we still have the extension block cached so we won't have to
; physically read it off the disk.  If we did a partial block transfer then
; buf (a4) will be 0 and we'll have to read the header again (although it will
; probably be hanging around on the valid queue).
;==============================================================================
GetHeader	cmp.w	a_HeaderSeq(sp),d4	do we need to rewind ?
		bge.s	NoRewind		no, we are >= current position
		clr.w	a_HeaderSeq(sp)
		move.l	a_FirstKey(sp),a_HeaderKey(sp)
NoRewind	move.l	a_HeaderKey(sp),a_Key(sp)  save key we're looking at
		cmpa.w	#0,a4			optimisation for common case
		beq.s	20$			nope, buffer not stashed

; we've collected up some blocks and not transferred them yet so the header has
; been held on to for subsequent iterations through this loop.  Check it's OK.
		cmp.w	a_HeaderSeq(sp),d4	is this the header we want
		beq	GetDataKey		yep, so find our data key
		movea.l	a4,a0			no, so free this buffer
		bsr	LooseBlock		but leave it valid

20$		move.l	a_Key(sp),d0
	printf <'fk=%ld Grabbing extension %ld\n'>,a_FirstKey(sp),d0
		bsr	GrabBlock		get the header key we want
		movea.l	d0,a4			stash the buffer address
		cmp.w	a_HeaderSeq(sp),d4	is this the header we want
		beq	GetDataKey		yep, so find our data key
		addq.w	#1,a_HeaderSeq(sp)	next sequential block #
		move.l	cb_SIZEOF+fhb_Extension(a4),a_HeaderKey(sp)
		bne.s	LoopForHeader		link not 0 so get next header

;==============================================================================
; We ran out of extension blocks when doing a write so me must make a new one.
;==============================================================================
		move.l	a_Key(sp),d0		get a new key
		moveq.l	#-1,d1			dont avoid anything
		bsr	GetKey
		move.l	d0,a_HeaderKey(sp)	save the new key
		bmi	RWError			disk full !!!
	printf <'Got new extension block at %ld\n'>,d0
		move.l	cb_SIZEOF+fhb_HashTable(a4),a_LastDotKey(sp)
		move.l	a_HeaderKey(sp),cb_SIZEOF+fhb_Extension(a4)  link
		movea.l	a4,a0			write out the buffer
		move.l	a_Key(sp),d0		this key
		moveq.l	#BUFF_INVALID,d1	*** old version ***
;		moveq.l	#BUFF_VALID,d1		put on waiting queue
		move.l	d2,-(sp)		save seq num
		moveq.l	#TRUE,d2
		bsr	WriteBlock
		move.l	(sp)+,d2		get back the seq num
		moveq.l	#TRUE,d0		get a block (and wait for it)
		bsr	GetBlock
		movea.l	d0,a4			save buffer
		lea.l	cb_SIZEOF(a4),a0	clear out the buffer
		bsr	ClearBlock		initialise the block
		move.l	#t.list,cb_SIZEOF+fhb_Type(a4)
		move.l	a_HeaderKey(sp),cb_SIZEOF+fhb_OwnKey(a4)
		move.l	a_FirstKey(sp),cb_SIZEOF+fhb_Parent(a4)
		move.l	#st.file,cb_SIZEOF+fhb_SecType(a4)
		bra.s	GetDataKey		written when data blk allocated

;==============================================================================
; Not found the header key we are looking for yet, so loop around again.
;==============================================================================
LoopForHeader	movea.l	a4,a0			free current header buff
		bsr	LooseBlock
		suba.l	a4,a4			buf := 0
		bra	GetHeader		and keep looking for header

;==============================================================================
; We now have a4 pointing to the correct header and d5 containing the offset
; to the correct data key within that header.  Fetch the data key to read/write
;==============================================================================
GetDataKey	move.l	cb_SIZEOF(a4,d5.w),d7	get data key
		bne	GotDataKey		it exists OK 
		move.l	a_LastDotKey(sp),d0	need to extend file blocks
		bne.s	10$			last.key was set earlier
		move.l	cb_SIZEOF+4(a4,d5.w),d0	get previous key
		move.l	d0,a_LastDotKey(sp)	we want a key near this key
10$		move.l	a_HeaderKey(sp),d1	and away from this one
		bsr	GetKey
		move.l	d0,d7			save new datakey
		bmi	RWError			Disk full
*** need to reset file sizes after a disk full error **********************
*** This is not so simple for FFS because of the blocks being written many
*** at a time.  It would be better to see if there's room before doing the
*** write.  There could still be problems with multiple writers though.

		move.l	d7,cb_SIZEOF(a4,d5.w)	store data key in table
		addq.l	#1,cb_SIZEOF+fhb_HighSeq(a4)
		tst.l	d2			if first block in file
		bne.s	20$
		move.l	d7,cb_SIZEOF+fhb_FirstBlock(a4)  then mark as such
20$		movea.l	a4,a0			write this block out
		move.l	a_HeaderKey(sp),d0
	printf <'fk=%ld Writing extension to %ld\n'>,a_FirstKey(sp),d0
		moveq.l	#BUFF_VALID,d1
		move.l	d2,-(sp)		save seq num
		moveq.l	#TRUE,d2
		bsr	WriteBlock
		move.l	(sp)+,d2		restore seq num
		suba.l	a4,a4			buf = 0

***** just allocated a data key, need to go back and update the pointer in the
***** previous data block (LastDotKey(sp) should still contain it I hope).
***** only do this if we are accessing a DOS/0 disk of course
		tst.b	DiskType+3(a5)		DOS/0 or DOS/1
		bne.s	90$			it's DOS/1
		tst.l	d2			if this is the first block
		beq.s	90$			then no previous data block
		move.l	a_LastDotKey(sp),d0	get previous data block
		bsr	GrabBlock
		movea.l	d0,a4
		move.l	d7,cb_SIZEOF+fdb_NextBlock(a4)
		move.l	a_LastDotKey(sp),d0	write back out with link
		moveq.l	#BUFF_VALID,d1
		move.l	d2,-(sp)		save seq num
		moveq.l	#FALSE,d2		***
		bsr	WriteBlock
		move.l	(sp)+,d2		restore seq num
		suba.l	a4,a4			buf = 0

; since we just allocated a new data block, we do a separate test for transfers
; of funny sizes or odd addresses.  This is because we don't want to read the
; block first (as in the GotDataKey section) because by default it has nothing
; in it anyway!
*** the blocksized comparison will always fail for DOS/0 disks (block = 488)
90$		cmpi.w	#512,d6			if not blocksized
		bne.s	CheckFailed		transfer through cache buffer
		move.l	GAddressMask(a5),d0	can we read to this memory
		and.l	a_Vector(sp),d0
		beq.s	CheckTransfer		yep, carry on

CheckFailed	moveq.l	#TRUE,d0		must transfer through a buffer
		bsr	GetBlock
		movea.l	d0,a3			allocated a data buffer

**** if DOS/0 then we must initialise the data header first *************
		tst.b	DiskType+3(a5)		DOS/0 or DOS/1
		bne.s	CheckTransfer		it's DOS/1

		lea.l	cb_SIZEOF(a3),a0
		bsr	ClearBlock		initialise the data block
		move.l	#T_DATA,cb_SIZEOF+fdb_Type(a3)
		move.l	d7,cb_SIZEOF+fdb_OwnKey(a3)
		addq.l	#1,d2			BCPL starts at 1 dammit!!!
		move.l	d2,cb_SIZEOF+fdb_SeqNum(a3)
		subq.l	#1,d2
		bra.s	CheckTransfer

;==============================================================================
; The data key we want exists already.  Don't get a buffer if we are doing a
; full block transfer to an even byte boundary.
;==============================================================================
GotDataKey	cmpi.w	#512,d6			block sized transfer ?
		bne.s	10$			nope, need to read a block
; bit 0 test not needed now because AddressMask always has bit zero clear
;		btst.b	#0,a_Vector+3(sp)	even address
;		bne.s	10$			nope, read a block
		move.l	GAddressMask(a5),d0	can we read to this memory
		and.l	a_Vector(sp),d0
		beq.s	CheckTransfer		yep, it's in address range

10$		movea.l	a4,a0			free up current buffer
		bsr	LooseBlock		can't keep over IO calls
		suba.l	a4,a4			buff = 0
		move.l	d7,d0			grab the block we want
	printf <'Reading block #%ld\n'>,d0

******** test for DOS/0 and treat differently here
		tst.b	DiskType+3(a5)		DOS/0 or DOS/1
		bne.s	20$			it's DOS/1
		bsr	GrabBlock		it's DOS/0
		movea.l	d0,a3			so it needs checksumming
		bra.s	CheckTransfer

20$		bsr	GrabDataBlock
		movea.l	d0,a3			stick into data buff

;==============================================================================
; We are now ready to perform the transfer.  Consecutive, full-block reads and
; writes are saved up so they can be made in one go.  If databuff (a3) is non
; zero, then we detected a partial block read or write and must flush any
; saved up transfers before doing something to databuff.  If we are reading
; and databuff<>0 then the data will have been read already and just needs
; transferring to the users buffer.  If we are writing and databuff<>0 then we
; must copy the data from the users buffer to our cache buffer.  If this was
; a write to an existing key, then the block will already have been read.
;==============================================================================
CheckTransfer	cmpa.w	#0,a3			databuff allocated ?
		bne.s	NotDirect		yes, so not a direct transfer
		move.l	a_LastKey(sp),d0
		addq.l	#1,d0
		cmp.l	d0,d7			consecutive keys ?
		bne.s	DoTransfer		nope, time to transfer now
		move.l	a_DSize(sp),d0
		cmp.l	GMaxTransfer(a5),d0
		bge.s	DoTransfer		can't do more than this in one
		add.l	#512,a_DSize(sp)	add one block
		bra.s	SetLast			and update pos etc.

DoTransfer:
		IFD STATISTICS
		addq.l	#1,s_AllocBreaks(a5)
		ENDC

		cmpa.w	#0,a4			can't hold on to buffer
		beq.s	10$			over an IO operation
		movea.l	a4,a0			so free it up
		bsr	LooseBlock
		suba.l	a4,a4			buff = 0
10$		move.w	a_Reading(sp),d0	read/write flag
		move.l	a_DSize(sp),d1		amount to read
		beq.s	NoTransfer2
		move.l	a_DKey(sp),d2		key (d2 not needed any more)
		movea.l	a_DVec(sp),a0		where to transfer to/from
		bsr	Transfer
NoTransfer2	move.l	#512,a_DSize(sp)	allow for block left over
		move.l	d7,a_DKey(sp)		the new start key
		move.l	a_Vector(sp),a_DVec(sp)	the new memory start location

SetLast		move.l	d7,a_LastKey(sp)
		bra	SetSizes		update pos, etc.

;==============================================================================
; Cant do a direct transfer so flush any pending direct stuff and do a partial
; block.  The block will already be read if this is a read operation.
; *** new code in here to handle the crud associated with DOS/0 data blocks!
;==============================================================================
NotDirect	move.w	a_Reading(sp),d0	flush pending direct transfer
		move.l	a_DSize(sp),d1		amount to read/write
		beq.s	NoTransfer1
		move.l	a_DKey(sp),d2		key (d2 not needed any more)
		movea.l	a_DVec(sp),a0		where to transfer to/from
		bsr	Transfer
NoTransfer1	clr.l	a_LastKey(sp)
		clr.l	a_DSize(sp)
		tst.w	a_Reading(sp)		are we reading
		beq.s	10$			no, we are writing

		lea.l	cb_SIZEOF(a3,d3.w),a0	source = buffer+offset
****** if DOS/0 then skip over the checksum and all that crud too
		tst.b	DiskType+3(a5)		DOS/0 or DOS/1
		bne.s	5$			it's DOS/1
		lea.l	fdb_Data(a0),a0		point to data area
5$		movea.l	a_Vector(sp),a1		destination
		move.l	d6,d0			length to copy
		jsr	_LVOCopyMem(a6)		copy buffer to user buffer
		movea.l	a3,a0
		bsr	LooseBlock		buffer free now
		suba.l	a3,a3
		bra.s	SetSizes		set up info for next loop

10$		movea.l	a_Vector(sp),a0		source
		lea.l	cb_SIZEOF(a3,d3.w),a1	destination = buffer+offset
****** if DOS/0 then skip over the checksum and all that crud too
		tst.b	DiskType+3(a5)		DOS/0 or DOS/1
		bne.s	15$			it's DOS/1
		lea.l	fdb_Data(a1),a1		point to data area
**** also need to update fdb_DataSize for the number of bytes in this block
		move.l	d3,d0			total bytes in block
		add.l	d6,d0			is offset+length
		cmp.l	cb_SIZEOF+fdb_DataSize(a3),d0
		ble.s	15$			DataSize=Max(Datasize,curpos)
		move.l	d0,cb_SIZEOF+fdb_DataSize(a3)

15$		move.l	d6,d0			length to copy
		jsr	_LVOCopyMem(a6)
		movea.l	a3,a0			write out the block
		move.l	d7,d0			this key
		moveq.l	#BUFF_VALID,d1
		moveq.l	#FALSE,d2		don't need d2 any more
20$	printf <'Calling WriteBlock(%ld)\n'>,d7
		bsr	WriteBlock
		suba.l	a3,a3

;==============================================================================
; We have either done a direct transfer, a buffered transfer or just saved up
; a transfer for some future iteration.  d6 contains the size we used so update
; the user buffer pointer, current file position and file size.
;==============================================================================
SetSizes	add.l	d6,a_Vector(sp)
		sub.l	d6,a_Size(sp)
		add.l	d6,a_Pos(sp)
		move.l	a_Pos(sp),d0
		cmp.l	a_FileSize(sp),d0	filesize = Max(pos,filesize)
		ble	BlockLoop		check for more transfers
		move.l	d0,a_FileSize(sp)
		bra	BlockLoop

;==============================================================================
; An error occurred while we were trying to allocate a key, return disk full.
;==============================================================================
RWError		movea.l	a4,a0			free the buffer
		bsr	LooseBlock
		move.w	#$7fff,a_HeaderSeq(sp)  really big
		movea.l	a2,a0			return the packet
		moveq.l	#-1,d0
		move.l	#ERROR_DISK_FULL,d1
		bsr	ReturnPkt
		bra	AccessLoop		wait for more work

;==============================================================================
; Transfer( reading, size, key, vector )
;	      d0.w    d1    d2    a0
;
; makes a fake cache buffer header on the stack and fills it in with the key
; vector, size and command and puts it on the pending queue for disk to pick up
; Calls Fetch() so that the buffer has been read on return from this routine.
; These kind of cache buffers are not put into the hash table.
;
; SMB 12/11/87  I've added a lot of code here to check that any multiple block
; transfers don't clash with stuff on the waiting queue.  It was possible that
; a block on the waiting queue would be within the range of a large transfer
; and would therefore write out at a later time, invalidating the previous
; large write.  This is also true for buffers on the valid queue which will be
; invalidated by a large write containing the same block.  The following cases
; are tested for and the given actions taken:-
;
; When doing a multiple block write;
;	1.	data blocks in range on the valid queue are discarded
;	2.	data blocks in range on the waiting queue are discarded
;
; When doing a multiple block read;
;	1.	data blocks in range on the waiting queue are first put onto
;		the pending queue so they get written first.
;==============================================================================
Transfer	movem.l	d3-d5/a2-a3,-(sp)
	printf <'Transfer(size=%ld key=%ld vec=$%lx)\n'>,d1,d2,a0
		move.l	d1,d4			save the size
		movea.l	a0,a2			save buffer address

		move.l	d4,d5			get a block range
		moveq.l	#9,d1			convert size to #blocks
		lsr.l	d1,d5
		add.l	d2,d5			d5 = upper block range

		move.w	d0,d3			save read/write flag
		bne.s	20$			no need to check valid queue

; we are doing a write operation so we must discard data blocks from the valid
; queue if they lie within the range of this transfer
		movea.l	ValidBufferQueue(a5),a3	get first entry in queue
10$		cmpa.w	#0,a3			any more buffers
		beq.s	20$			nope, finished this search
		tst.l	cb_Size(a3)		is this a data block
		bne.s	14$			nope, size=-1
		cmp.l	cb_Key(a3),d2		is block within this transfer ?
		bgt.s	14$			no, key not in range
		cmp.l	cb_Key(a3),d5
		bgt.s	15$			key in range !
14$		movea.l	(a3),a3			link to next buffer
		bra.s	10$			and search for more

; we found a data block on the valid queue that was in range of this transfer
15$:
		IFD STATISTICS
		addq.l	#1,s_ValidTrash(a5)
		ENDC
		movea.l	a3,a1			dequeue it
		movea.l	(a1),a3			save pointer to next buffer
		lea.l	ValidBufferQueue(a5),a0
		bsr	DeQueue			dequeue from valid
		movea.l	d0,a0
		bsr	UnHash			and remove from hash table
		movea.l	d0,a0
		clr.w	cb_QueueType(a0)	type is now free
		move.l	FreeBufferQueue(a5),(a0)
		move.l	a0,FreeBufferQueue(a5)	stick on free queue
		bra.s	10$			and look for more

; whatever operation we are performing, we must search the waiting queue for
; this block.  If a block is there and we are reading then we must transfer
; the block to the pending queue so it gets written before we read it.  If
; the block is there and we are writing then we can just discard it because
; we are going to overwrite the little bugger anyway!
20$		movea.l	WaitingQueue(a5),a3
30$		cmpa.w	#0,a3			any more buffers
		beq.s	40$			nope, finished this search
		tst.l	cb_Size(a3)		is this a data block
		bne.s	34$			nope, size=-1 or >0
		cmp.l	cb_Key(a3),d2		is block within this transfer ?
		bgt.s	34$			no, key not in range
		cmp.l	cb_Key(a3),d5
		bgt.s	35$			key in range
34$		movea.l	(a3),a3			link to next buffer
		bra.s	30$

; found a block in range.  Dequeue it anyway whether we are reading or writing
35$		movea.l	a3,a1			IN RANGE! dequeue it
		movea.l	(a1),a3			save pointer to next buffer
		lea.l	WaitingQueue(a5),a0
		bsr	DeQueue			dequeue from waiting (d0=buff)
		tst.w	d3			are we reading or writing ?
		beq.s	31$			writing, so discard it

; we are reading so we must transfer the block to the pending queue before us.
		movea.l	d0,a1
		move.w	#HASHPENDING,cb_QueueType(a1)
		lea.l	PendingQueue(a5),a0
		bsr	EnQueue			add to pending
		bra.s	30$			search for more buffers

; we are writing so unhash this buffer and put it on the free queue
31$		movea.l	d0,a0
		bsr	UnHash			remove from hash table
		movea.l	d0,a0
		clr.w	cb_QueueType(a0)	type is now free
		move.l	FreeBufferQueue(a5),(a0)
		move.l	a0,FreeBufferQueue(a5)	stick on free queue
		bra.s	30$			go search for next buffer

; Finally! Data blocks hanging around can't hurt us any more so do the transfer
40$		lea.l	-cb_SIZEOF(sp),sp	fake a cache buffer header
		move.l	d4,cb_Size(sp)		length to transfer
		move.l	d2,cb_Key(sp)		what key
		move.l	a2,cb_Buff(sp)		what buffer
		move.l	CurrentCo(a5),cb_CoPkt(sp)   owning co-routine (me)
		move.w	#CMD_READ,cb_State(sp)	assume reading
		tst.w	d3			are we ?
		bne.s	50$			yes, so it was correct
		move.w	#CMD_WRITE,cb_State(sp)
50$		lea.l	PendingQueue(a5),a0	append to pending queue
		movea.l	sp,a1
		bsr	EnQueue			doesn't go in hash table
		bsr	Fetch			comes back when it's done
		lea.l	cb_SIZEOF(sp),sp
		movem.l	(sp)+,d3-d5/a2-a3
		rts


;==============================================================================
; ClearWaiting()
;
; Transfers all buffers on the waiting queue to the pending queue
;==============================================================================
ClearWaiting	tst.l	WaitingQueue(a5)	anything on waiting queue
		beq.s	10$			nope, all done
		movea.l	WaitingQueue(a5),a1
		lea.l	WaitingQueue(a5),a0
		bsr	DeQueue			remove from waiting
		movea.l	d0,a1
		move.w	#HASHPENDING,cb_QueueType(a1)
		lea.l	PendingQueue(a5),a0
		bsr	EnQueue			and add to pending
		bra.s	ClearWaiting
10$		rts

;==============================================================================
; Length = Truncate(header,length)
;   d0		      d0     d1
;
; Truncates a file to the given length.  The current seek position must be
; less than or equal to the truncation length or nasty things will happen.
; This routine is only callable by the main loop of Access!!!!!!!!!!!!!!!!
; The file header must be updated with the current size and datestamp too!
;==============================================================================
Truncate	movem.l	a2-a3/d2-d7,-(sp)
		move.l	d0,d2			save header key
		move.l	d1,d3			save length required

; figure out how many hash table entries will be required for the new filesize
		move.l	d3,d0
		tst.b	DiskType+3(a5)		which FS ?
		beq.s	10$			old FS
		moveq.l	#9,d1
		lsr.l	d1,d0			convert to number of blocks
		bra.s	20$

10$		divu.w	#488,d0

20$		ext.l	d0
		move.l	d0,d4			d4 = block # of last block
		move.l	d0,d5			figure out seqnum of extension
		divu.w	#72,d5
		moveq.l	#0,d6			current sequential number
		move.l	d2,d7			current key

; start at the header and loop around until we have the extension block we need
FindExt		move.l	d7,d0
		bsr	WaitBlock		fetch the block
		movea.l	d0,a0
		cmp.w	d6,d5			is it the one we want ?
		beq.s	GotExt			yep, start freeing blocks
		addq.w	#1,d6			nope, bump to the next
		move.l	cb_SIZEOF+fhb_Extension(a0),d7  get link
		bsr	LooseBlock		free this block
		bra.s	FindExt			and keep looking

; we are now holding the header that contains the new last block.  Initialise
; variables to point to the block beyond that and free all data blocks that
; are no longer being used.  Also update the HighSeq number to point to the
; last block used.  A new file size of 0 is a special case here, watch out!!
GotExt		movea.l	a0,a3			stash the header
		divu.w	#72,d4
		swap	d4			d4 = offset to last block
		ext.l	d4
		tst.l	d3			is new size 0 ?
		beq.s	10$			yes, so don't bump to next block
		addq.w	#1,d4			bump for hiseq and next block
10$		move.l	d4,cb_SIZEOF+fhb_HighSeq(a0)  save it in header
		moveq.l	#72-1,d5		calculate # blocks to free
		sub.l	d4,d5			not freeing these blocks
		lea.l	cb_SIZEOF+fhb_Junk1(a3),a2
		lsl.l	#2,d4			convert to real offset
		suba.l	d4,a2
		move.l	(a2),d6			stash the last key (maybe 0)

TruncLoop	move.l	-(a2),d0		get key to be freed
		beq.s	FirstDone		no more, truncated first header
		bsr	FreeKey
		clr.l	(a2)			mark as clear in the header
		dbra	d5,TruncLoop		and look for more

; we've freed all data keys in this header, grab its extension key and
; unlink it.  Then write this header back with its new hashtable.
FirstDone	move.l	d7,d0			writing this key
		move.l	cb_SIZEOF+fhb_Extension(a3),d7  grab extension
		clr.l	cb_SIZEOF+fhb_Extension(a3)     and unlink extension
		moveq.l	#BUFF_VALID,d1
		move.l	d2,-(sp)		save seq num
		moveq.l	#TRUE,d2		must be checksummed
		bsr	WriteBlock
		move.l	(sp)+,d2		header key

; while d7 != NULL, free all data blocks referenced in that header and
; also free the header itself, after fetching the extension link.
FreeRest	tst.l	d7			is there a header ?
		beq.s	UpdateHeader		nope, go set size correctly
		move.l	d7,d0
		bsr	GrabBlock		yes, fetch the header
		movea.l	d0,a3
		lea.l	cb_SIZEOF+fhb_Junk1(a3),a2
		moveq.l	#72-1,d5
10$		move.l	-(a2),d0		get key to be freed
		beq.s	FreeNext		no more
		bsr	FreeKey			free it
		dbra	d5,10$
FreeNext	move.l	d7,d0			stash header key
		move.l	cb_SIZEOF+fhb_Extension(a3),d7  get extension
		bsr	FreeKey			free this key
		movea.l	a3,a0			and throw header away
		bsr	DiscardBlock
		bra.s	FreeRest		see if any more to free

; we've freed up all the extraneous muck, now update the filesize
UpdateHeader	move.l	d2,d0			fetch header key
		bsr	GrabBlock
		movea.l	d0,a0
		move.l	d3,cb_SIZEOF+fhb_ByteSize(a0)
		move.l	d2,d0			writing this header
		moveq.l	#BUFF_VALID,d1
		moveq.l	#TRUE,d2		must be checksummed
		bsr	WriteBlock

; finally, if this is a DOS/0 disk, then we need to update the data size
; in the last data block too.  We saved the key of that block in d6
		tst.b	DiskType+3(a5)
		bne.s	TruncNot0
		move.l	d6,d0
		bsr	GrabBlock		fetch the block
		movea.l	d0,a0
		move.l	d3,d0			fetch size
		divu.w	#488,d0
		swap	d0			get bytes left in last block
		ext.l	d0
		move.l	d0,cb_SIZEOF+fdb_DataSize(a0)
		clr.l	cb_SIZEOF+fdb_NextBlock(a0)  no next block
		move.l	d6,d0			writing this block
		moveq.l	#BUFF_VALID,d1
		moveq.l	#FALSE,d2		checksummed data block
		bsr	WriteBlock

TruncNot0	move.l	d3,d0			return the new size
		movem.l	(sp)+,a2-a3/d2-d7
		rts

;==============================================================================
; Length = Extend(header,length)
;   d0		    d0     d1
;
; Adds bytes to the end of a file to make it up to the required size.  I was
; just going to add extra entries into the extension blocks, but since DOS/0
; disks require chaining of the data blocks, I may as well write out blocks
; that have been initialised to 0 for both FFS and OFS.  This is going to be
; REALLY slow, what a cock-up!
;==============================================================================
Extend		rts

		movem.l	a2-a3/d2-d7,-(sp)
		move.l	d0,d2			save header
		move.l	d1,d3			save size required

; first we have to get to the end of the file, find out it's current size
		bsr	GrabBlock		grab the header key
		movea.l	d0,a0
		move.l	cb_SIZEOF+fhb_ByteSize(a0),d4  get current size
		bsr	LooseBlock		goes back where it was

		move.l	d4,d0			see which header we need
		
		rts

		END
