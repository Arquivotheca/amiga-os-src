		SECTION	filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/io.i"
		INCLUDE	"exec/alerts.i"
		INCLUDE	"libraries/dosextens.i"
		INCLUDE	"devices/trackdisk.i"
		INCLUDE	"fault.i"
		INCLUDE	"moredos.i"
		INCLUDE	"globals.i"
		INCLUDE	"private.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XDEF	Disk,DoButtons

		XREF	WaitCo,ResumeCo
		XREF	Checksum,FreeBlock,ClearBlock
		XREF	_LVOSendIO
		XREF	TestDisk,Request
		XREF	DeQueue,UnHash
		XREF	SendTimer
		XREF	_LVOFault

;==============================================================================
; Disk()  The main mover and shaker.  Since this is a co-routine there is no
;	  need to save any registers because this is done by CallCo et al.
;==============================================================================
Disk		lea.l	PendingQueue(a5),a0	anything to be done ?
		TSTLIST	a0
		bne.s	DiskGotWork		yes, get some work
		bsr	WaitCo			no, wait until there is

; There is some work in the pending queue so go and fetch the first buffer.
; This buffer is not actually removed from the queue until the work is done.
DiskGotWork	move.w	#TRUE,DiskRunning(a5)	disk running is true

; grab the first header off the pending queue and see if it's read or write
		movea.l	PendingQueue(a5),a2	get first buffer to a2
		clr.l	cb_Actual(a2)		assume no data transfer
		clr.l	cb_Error(a2)		therefore no errors
		move.w	cb_State(a2),d2		d2 = req 
		move.l	cb_Size(a2),d3		d3 = header flag/size
		movea.l	cb_Buff(a2),a3		a3 = data pointer

		cmpi.w	#CMD_WRITE,d2		is this a write command ?
		bne.s	DiskRead		nope, so must be a read

; This is a write to disk so check cb_Size to see if we need a checksum test.
; There are a few possibilities for checksumming.  If in oldFS mode (DOS\0)
; then we checksum everything except bitmap blocks in the normal manner.  If
; we're in FFS mode then only file headers get checksummed in the normal way
; because data blocks are all data.  Bitmap blocks are treated differently
; because thier checksum is in the first longword of the block.
		moveq.l	#RAW,d0			is it a raw block ?
		cmp.l	d3,d0
		beq.s	NoWriteSum		yes, so no checksum either
		tst.l	d3			do we need a checksum ?
		bgt.s	NoWriteSum		anything >0 doesn't
		bmi.s	DoWriteSum		yes, header or bitmap block
		btst.b	#0,DiskType+3(a5)	is it oldfs format
		bne.s	NoWriteSum		no, so don't checksum data

DoWriteSum	move.l	a3,a0			where the data is
		bsr	Checksum		returns in d0
		cmpi.l	#BITMAP,d3		bitmap or normal header ?
		bne.s	30$			normal header
		sub.l	d0,(a3)			fix bitmap checksum
		bra.s	NoWriteSum		and do the write
30$		sub.l	d0,fhb_Checksum(a3)	fix normal header checksum

NoWriteSum	bsr	Diskact			a2=cachebuff d2=devcommand
		move.w	#CMD_READ,cb_State(a2)	change to read request
		bra.s	DiskCommon

; This is a read from disk.  The same rules as write apply when checksumming
; the block after the read.  If this is straight into a user buffer then there
; is no special info involved and a checksum need not be performed (cb_Size>=0)
DiskRead	bsr	Diskact			a2=cachebuff d2=devcommand
		tst.l	d3			should we run a checksum ?
		bgt.s	DiskCommon		not if >0
		bmi.s	DoReadSum		yes, it's a header
		btst.b	#0,DiskType+3(a5)	always checksum oldfs format
		bne.s	DiskCommon		no need to checksum
DoReadSum	move.l	a3,a0			where the data is
		bsr	Checksum		returns in d0
		tst.l	d0			was checksum OK ?
		beq.s	DiskCommon		yep, carry on

; Checksum was bad.  Report the error and retry if user requests it
	printf <'disk: Checksum error on read (size = %ld) '>,cb_Size(a2)
	printf <'(block = %ld, buffer = $%lx)\n'>,cb_Key(a2),a2
		move.l	#ERROR_CHECKSUM,cb_Error(a2)
		movea.l	a2,a0			this buffer
		moveq.l	#0,d0			no particular error number
		bsr	IOError
		tst.l	d0
		bne	DiskRead		user said Retry

; The read or write has completed, see if we were called indirectly by a
; deferred write (initially from the waiting queue) or directly by a read.
; If we were called directly then we ResumeCo back to the co-routine that's
; waiting for this block to read.  Writes can be made direct if someone made
; a Fetch() call on a writing buffer.
DiskCommon	tst.w	TimerRunning(a5)	is timer request already going?
		bne.s	TimerDone		yes, so don't send again
;	printf <'Disk: sending timer request\n'>
		bsr	SendTimer		nope, fire off one second event

TimerDone	clr.w	DiskRunning(a5)		disk not running now
		lea.l	PendingQueue(a5),a0	deque from head of pending
		REMHEAD				we still have pointer in a2
		tst.l	cb_Size(a2)		not in hash table if not...
		bgt.s	not_hashed		...block sized
		tst.w	cb_QueueType(a2)	was this buffer already freed?
		bne.s	unhashit		no (HASHFREE == 0)

; The buffer was invalidated by WriteBlock.  There can be no cb_CoPkt waiters
; (they were stolen).  The block has already been removed from the Hash table,
; but needs to be added to the FreeQueue
		move.l	FreeBufferQueue(a5),(a2)
		move.l	a2,FreeBufferQueue(a5)	link into free buffer queue
		bra	Disk			see if there's more to do

unhashit	movea.l	a2,a0			it's a single block
		bsr	UnHash			remove from hash table
	IFD DEBUG_CODE
	  tst.l	d1
	  beq.s 1$
 printf <' in disk, cb_Size %ld, cb_CoPkt $%lx, cb_Header %ld, cb_buff[cb_sizeof] $%lx'>,cb_Size(a2),cb_CoPkt(a2),cb_Header(a2),cb_SIZEOF+cb_SIZEOF(a2)
	  lea	cb_SIZEOF(a2),a0
	  cmp.l	cb_Buff(a2),a0
	  beq.s	1$
 printf <' it was a direct write buffer (cb_Buff = $%lx)!!!!'>,cb_Buff(a2)
1$
	ENDC
not_hashed	tst.l	cb_CoPkt(a2)		who owns this buffer
		beq.s	20$			nobody, it's ours
		movea.l	cb_CoPkt(a2),a0		Resume back to owning co-routine
		move.l	a2,d0			giving buffer as an argument
		bsr	ResumeCo
		bra	Disk			and CoWait when we come back

; This was probably a write block so it will get transferred to the valid queue
20$		move.l	d3,d0			pass size/flag
		movea.l	a2,a0			pass the cache buff struct
		bsr	FreeBlock		put valid headers in valid list
		bra	Disk			and go wait for more work

;==============================================================================
; Diskact(action,Buffer)  Sends the action requested by Disk to the device and
;	    d2	   a2	  fakes up a packet so the dispatcher in main will know
;			  to CallCo the disk routine again.
; If an error occurs then we can come back here multiple times to re-read/write
;==============================================================================
Diskact		lea.l	-dp_Type(sp),sp		get a temporary packet
		movea.l	DiskDev(a5),a1		get the IORequest pointer
		move.l	a1,dp_Port(sp)		packet for dispatcher in main
		move.l	sp,LN_NAME(a1)		link into the IORequest
		move.w	d2,IO_COMMAND(a1)	set up the command
		move.l	cb_Key(a2),d0		get offset on the disk
		cmp.l	Reserved(a5),d0
		blt.s	KeyError		negative offset not allowed
		cmp.l	HighestBlock(a5),d0	beyond end of partition is bad
		ble.s	NoKeyError		key is in range

; the key is outside the range of this partition, return an error to say this
KeyError	lea.l	dp_Type(sp),sp		free up stack space used
		move.w	#ERROR_KEYRANGE,cb_Error+2(a2)	this is the error
	printf <'disk: key out of range (%ld)\n'>,d0
	printf <'      type = %ld\n'>,cb_Size(a2)
		bra.s	DiskactDone

; the disk key is in range, convert to a byte offset and perform the read/write
NoKeyError	move.w	BlockShift(a5),d1	multiply by sector size and...
		lsl.l	d1,d0			...make key into a byte offset
		add.l	LowerByte(a5),d0	add byte offset to first block
		move.l	d0,IO_OFFSET(a1)
		move.l	cb_Size(a2),d0		get size to transfer
		bgt.s	10$			direct to user buffer
		move.l	BlockSize(a5),d0	file header or data block
10$		move.l	d0,IO_LENGTH(a1)
		move.l	cb_Buff(a2),IO_DATA(a1)	where data goes to
		jsr	_LVOSendIO(a6)		send cmd to the disk device

		move.w	#3,MotorTicks(a5)	so motor doesn't turn off

		bsr	WaitCo			wait to be called again
		lea.l	dp_Type(sp),sp		reclaim stack space
		movea.l	DiskDev(a5),a1		check for an error
		move.l	IO_ACTUAL(a1),cb_Actual(a2) transfer count
		move.b	IO_ERROR(a1),cb_Error+3(a2) transfer the error code
		beq.s	DiskactDone		no problems, all done

; The only errors we are interested in is write errors because no co-routine
; will be hanging on to the write block.  At this point we should pop a request
; stating which file contains the error (if it's not the header block that got
; messed up).
	printf <'diskact: got an IOError (%lx)\n'>,cb_Error(a2)
		addq.l	#1,SoftErrors(a5)	one extra error
		move.l	cb_Error(a2),d0		fetch the error code
		movea.l	a2,a0			set up buffer argument
		bsr.s	IOError			see what user wants to do
		tst.l	d0			did they hit retry 
		bne	Diskact			yes, try again

; the read/write has completed but may have failed with keyrange error etc.
DiskactDone	clr.l	cb_Suppress(a2)		don`t suppress write errors
		rts				thats all

;==============================================================================
; result = IOError( buffer, error )
;  d0		      a0      d0
;
; Calls Request to pop the appropriate requester and returns TRUE if the user
; pressed Retry or FALSE if the user pressed Cancel.
;
; If user cancels a write error then requesters are suppressed on all pending
; writes.
;==============================================================================
IOError		movem.l	d2/a2-a3,-(sp)
		movea.l	a0,a3			save cache buffer pointer
		move.l	d0,d2			and error number (maybe 0)
		moveq.l	#CMD_UPDATE,d0
		bsr	TestDisk		flush out dirty buffers
		moveq.l	#CMD_CLEAR,d0		and then invalidate all buffers
		bsr	TestDisk
		moveq.l	#TD_MOTOR,d0
		moveq.l	#0,d1
		bsr	TestDisk		make sure motor is off

; Fix for 2.0 trackdisk.  Popping the disk will often return write protect or
; read write errors instead of TDERR_DiskChanged.  We check if the disk is in
; the drive before reporting errors and tell user to replace it if it's out
		moveq.l	#TD_CHANGESTATE,d0
		bsr	TestDisk		see if disk in or out
		tst.l	d0
		beq.s	NotFalseError
		moveq.l	#TDERR_DiskChanged,d2	make like diskchange error

; if we are looking at a new disk after inhibit, don't complain if its removed.
NotFalseError	move.l	d2,ErrorCode(a5)
		cmpi.b	#TDERR_DiskChanged,d2
		bne.s	DiskStillIn
		tst.l	CurrentVolume(a5)
		beq	IOErrorDone		return false (continue)

; The disk changed, we tell the user that they MUST replace the disk in the
; correct unit.  It won't do to put it in another drive because we have buffers
; waiting to be written to by THIS fs process.  Don't do this for CMD_READs
;		cmpi.w	#CMD_READ,cb_State(a3)
;		beq.s	DiskStillIn		treat like a read error

		bsr	BuildReplaceMsg
		bsr	Request			pop the you-must requester
;		moveq.l	#TRUE,d0		always retry
		bra	IOErrorDone		finished

; The disk is still in the drive, so this must be a read/write error of some
; kind.  Report the volume that the error occured on and check for retry.
DiskStillIn	tst.l	cb_Suppress(a3)		suppressing write errors?
		beq.s	10$			nope
		cmpi.w	#CMD_WRITE,cb_State(a3)	yes, was it a write
		bne.s	10$
		moveq.l	#0,d0
		bra.s	IOErrorDone		just return a cancel

10$		move.l	cb_Key(a3),d0		will need block number for args
		move.l	#STR_CHECKSUM_ERROR,d1	assume checksum error
		tst.l	d2			true if d2==0
		beq.s	GotRWError
		move.l	#STR_READ_ERROR,d1	assume a read error
		cmpi.w	#CMD_READ,cb_State(a3)
		beq.s	GotRWError
		move.l	#STR_WRITE_ERROR,d1

GotRWError	bsr	BuildErrorMsg
		bsr	Request
		cmpi.w	#CMD_READ,cb_State(a3)	clear buffer if it's a read
		bne.s	IOErrorDone
		tst.l	cb_Size(a3)		NOT FOR DIRECT BUFFERS !!!
		bgt.s	IOErrorDone		(this was the cancel bug)

		move.l	d0,-(sp)		save retry/cancel flag
		lea.l	cb_SIZEOF(a3),a0	and clear out the buffer
		bsr	ClearBlock
		move.l	(sp)+,d0

; new fix, if this was a write and the user pressed cancel, mark all other
; pending write buffers so that write error requesters are suppressed.
		bne.s	IOErrorDone
		moveq.l	#-1,d1
		lea.l	PendingQueue(a5),a0	scan pending queue
		move.l	(a0),d0			must set d0=0 for CANCEL
10$		movea.l	d0,a0			node pointer to a0
		move.l	(a0),d0			lookahead to next node
		beq.s	15$			last in list
		cmpi.w	#CMD_WRITE,cb_State(a0)
		bne.s	10$
		move.l	d1,cb_Suppress(a0)
		bra.s	10$

15$		lea.l	WaitingQueue(a5),a0	scan waiting queue
		move.l	(a0),d0			must set d0=0 for CANCEL
20$		movea.l	d0,a0			node pointer to a0
		move.l	(a0),d0			lookahead to next node
		beq.s	IOErrorDone		last in list
		cmpi.w	#CMD_WRITE,cb_State(a0)
		bne.s	20$
		move.l	d1,cb_Suppress(a0)
		bra.s	20$

IOErrorDone	movem.l	(sp)+,d2/a2-a3
		rts


;==============================================================================
; BuildReplaceMsg()
;
; sets up the ReqString and ReqArgs for a "You must replace" requester.
;==============================================================================
BuildReplaceMsg	movem.l	d2-d4/a2/a6,-(sp)
		movea.l	DosLib(a5),a6
		movea.l	ReqString(a5),a2	this string
		moveq.l	#STR_MUST_REPLACE,d1
		moveq.l	#0,d2			no pre-string
		move.l	a2,d3			buffer address
		moveq.l	#80,d4			maximum length
		jsr	_LVOFault(a6)
		adda.w	d0,a2			bump length of string

		move.b	#10,-1(a2)		req text to next line
		move.b	#'%',(a2)+		volume name on next line
		move.b	#'b',(a2)+
		move.b	#10,(a2)+		move to next line

		moveq.l	#STR_IN_DEVICE,d1
		moveq.l	#0,d2			no pre-string
		move.l	a2,d3			buffer address
		moveq.l	#80,d4			maximum length
		jsr	_LVOFault(a6)

		movea.l	ReqArgs(a5),a0		build argument list
		movea.l	CurrentVolume(a5),a1	using BSTR volume name
		adda.l	a1,a1
		adda.l	a1,a1
		move.l	dl_Name(a1),(a0)+
		movea.l	DevEntry(a5),a1		get unit name (DF0: DH0: etc)
		movea.l	dol_Name(a1),a1
		adda.l	a1,a1
		adda.l	a1,a1			"in device" expects CSTR not BSTR
		addq.l	#1,a1
		move.l	a1,(a0)+
		lea.l	Exclam(pc),a1
		move.l	a1,(a0)			exclamation marks after name

		movem.l	(sp)+,d2-d4/a2/a6
		bra.s	DoButtons

;==============================================================================
; BuildErrorMsg(key,msg)
;		d0  d1
;
; sets up the ReqString and ReqArgs for a Checksum|Read|Write error.  Key is
; block number where the error occured.  msg is the Fault.i number for the
; appropriate error string.
;==============================================================================
BuildErrorMsg	movem.l	d2-d4/a2/a6,-(sp)
		movea.l	DosLib(a5),a6
		movea.l	ReqString(a5),a2

		move.w	#'%b',(a2)+		volumename is first line
		move.b	#10,(a2)+

		move.l	d0,-(sp)		save block number
		moveq.l	#0,d2			and get correct error string
		move.l	a2,d3			buffer address
		moveq.l	#80,d4			maximum length
		jsr	_LVOFault(a6)
		adda.l	d0,a2
		move.b	#10,-1(a2)

		move.l	#STR_BLOCK_NUM,d1	get block number message
		moveq.l	#0,d2			and get correct error string
		move.l	a2,d3			buffer address
		moveq.l	#80,d4			maximum length
		jsr	_LVOFault(a6)

		movea.l	ReqArgs(a5),a0		build argument list
		movea.l	CurrentVolume(a5),a1	using BSTR volume name
		adda.l	a1,a1
		adda.l	a1,a1
		move.l	dl_Name(a1),(a0)+
		move.l	(sp)+,(a0)		disk block argument

		movem.l	(sp)+,d2-d4/a2/a6	drop into DoButtons

;==============================================================================
; DoButtons()
;
; Sets up the default RETRY|CANCEL string for an EasyRequest with two buttons
;==============================================================================
DoButtons	movem.l	d2-d4/a6,-(sp)
		movea.l	DosLib(a5),a6
		moveq.l	#STR_RETRY_CANCEL,d1
		moveq.l	#0,d2			no pre-string
		move.l	ButtonString(a5),d3	buffer address
		moveq.l	#64,d4			maximum length
		jsr	_LVOFault(a6)
		movem.l	(sp)+,d2-d4/a6
		rts

Exclam		DC.B	' !!',0

		END

