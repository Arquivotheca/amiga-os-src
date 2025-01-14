		SECTION	filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/nodes.i"
		INCLUDE	"exec/lists.i"
		INCLUDE	"exec/io.i"
		INCLUDE	"libraries/dos.i"

		INCLUDE	"globals.i"
		INCLUDE	"private.i"
		INCLUDE	"readwrite.i"
		INCLUDE	"moredos.i"

		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XDEF	ReadFFS,WriteFFS,SeekFFS,SetSizeFFS

		XREF	GrabBlock,WriteBlockLater,Fetch,AllocKeys
		XREF	GrabBlockA3,GrabBlockA4
		XREF	LooseBlock,GetBlock,GrabDataBlock,DiskProtected
		XREF	AlterRoot,ClearBlock,UnHash,DiscardBlock,WriteBlockNow
		XREF	WaitDataBlock,ClearFile,FreeBuffer,LooseHeaderBlock
		XREF	WriteSummedBlockD6A0Later_RWD,LooseA4HeaderBlock
		XREF	WriteSummedBlockA0Later_RWD
		XREF	_LVOCopyMem

;==============================================================================
; amount/error = ReadFFS( amount,buffer,rwdata ) globals,execbase
;   d0    d1		    d0	   a0	  a1	   a5	   a6
;
; Attempts to read the given amount of data into buffer from the file specified
; by the rwdata entries.  This code should be run as a co-routine so it can use
; the cache buffer reading routines.  Whenever a read or a portion of a read is
; on contiguous blocks and into word aligned memory, the data is transferred to
; the callers buffer without going through the cache buffers.  This is improved
; a lot over the 1.3 FFS read routine and can achieve read speeds of up to 98%
; of the disk bandwidth.
;==============================================================================
ReadFFS		movea.l	a0,a2			save buffer pointer
		movea.l	a1,a3			and file handle data pointer
		move.l	d0,d2			save amount to be read

		cmp.l	LargestWrite(a5),d2	read size to practical limits
		ble.s	5$
		move.l	LargestWrite(a5),d2

; All file size information is kept in the header rather than local to this
; co-routine.  This allows multiple readers and writers without any confusion
; as to what the current state of the file is.
5$		move.l	rwd_FileHeader(a3),d0	get the file header
		bsr	GrabBlock
;		movea.l	d0,a0
		move.l	cb_Error(a0),d4		an error ?
		beq.s	6$			nope
		bsr	DiscardBlock		trash this block
		bra	ReadFFSDone		return -1/error in d0/d1

6$		lea.l	cb_SIZEOF(a0),a1	point to actual buffer data
		adda.l	BlockSize(a5),a1	reference from end of block
		move.l	vfhb_Protect(a1),d3	get size and protection bits
		move.l	vfhb_ByteSize(a1),d4
		bsr	LooseHeaderBlock	done with header for now

		move.l	#ERROR_READ_PROTECTED,d1 assume read protected
		moveq.l	#0,d0			therefore no data read yet
		btst.l	#3,d3			is read allowed ? ****
		bne	ReadFFSExit		no, so return the error

		moveq.l	#0,d3			total read so far = 0
		moveq.l	#TRUE,d0		assume we end up still in file
		move.l	d0,rwd_InFile(a3)
		sub.l	rwd_Offset(a3),d4	bytes remaining in file
; FIX!!! file sizes should be UNsigned!!!!
		cmp.l	d2,d4			min(amount,filesize-offset)
		bgt.s	10$
		move.l	d4,d2			truncate read to end of file
		clr.l	rwd_InFile(a3)		and offset won't be in the file
10$		moveq.l	#0,d4			current error = FALSE

; This is the main read loop that continues while there is data left and the
; error condition is false.  Contiguous blocks to word aligned addresses are
; read directly into the callers buffer if AddressMask permits DMA to here.
;**** bug fix, negative read amounts could do nasty things (old FFS exited)
		tst.l	d2			anything left to read
		ble	ReadFFSDone		nope, exit now
ReadFFSLoop	tst.l	d4			any errors
		bne	ReadFFSDone		yes, exit now

; grab the current extension block header so we can find the current key
		move.l	rwd_Header(a3),d0
		bsr	GrabBlockA4

****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a4			stash current header
		move.l	cb_Error(a4),d4		any error ?
		beq.s	1$			nope
		movea.l	a4,a0
		bsr	DiscardBlock		trash this block
		bra	ReadFFSDone		return -1/error in d0/d1

1$		move.w	rwd_Entry(a3),d6	cache current entry offset
		move.l	cb_SIZEOF(a4,d6.w),d1	get starting block number
		move.l	BlockSize(a5),d0	are we on a block boundary
		subq.l	#1,d0
		and.l	rwd_Offset(a3),d0	ReadFFSCache needs d0=offset
		bne	ReadFFSCache		no, must use cache buffer
		cmp.l	BlockSize(a5),d2	at least one block left ?
		blt	ReadFFSCache		no, so use cache buffer
		move.l	a2,d5			check address for even and..
		and.l	GAddressMask(a5),d5	...in AddressMask range
		bne	ReadFFSCache		odd or failed mask, use cache

; We have at least one block left to transfer direct to the callers buffer.
; Check we aren't violating MaxTransfer and truncate amount to do if we are
		move.l	d2,d5			cache number of bytes to do
		cmp.l	GMaxTransfer(a5),d5	can we do this many bytes ?
		ble.s	10$			yep, no adjustment
		move.l	GMaxTransfer(a5),d5	nope, adjust to MaxTransfer
10$		move.w	BlockShift(a5),d0	convert bytes to blocks
		lsr.l	d0,d5			d5 holds block count

		moveq.l	#1,d7			accrued total to transfer
		move.l	d1,rwd_StartKey(a3)	save key we are starting at

; Loop around collecting blocks until we have them all or the blocks are not
; contiguous on the disk.  Update CurrentEntry and CurrentHeader as we go.
CollectBlocks	subq.w	#4,d6			bump to next extension entry
		cmpi.w	#fhb_HashTable,d6
		bge.s	10$			still within hash table bounds

; We've run off the end of the hash table.  Grab the next extension block and
; update the rwd state variables as required.  If the next extension is not
; contiguous with the previous one then we won't actually read it.  Due to the
; way the files are written this would mean we are about to hit a frag anyway.
; If we did read non-contiguous extensions we would end up seeking forward
; HashTabSize blocks and then back again to read the collected data blocks.
; If we have read to EOF the next HashTable slot will contain 0 which is used
; by the Write routine to determine when a new block needs allocating.  If
; EOF happened to be right on the end of the hashtable then rwd_Header will
; contain 0 too (there is no hashtable entry because there is no hashtable).
; This is used by the write routine to do the special large chunk allocation.
		move.l	rwd_Header(a3),d0	save current extension key
		move.l	d0,rwd_PrevHeader(a3)	in case we write later
		lea.l	cb_SIZEOF(a4),a0
		adda.l	BlockSize(a5),a0	fetch extension key pointer
		move.l	vfhb_Extension(a0),rwd_Header(a3)
		move.w	MaxEntry(a5),d6		reset offset into hashtable
		addq.w	#1,rwd_Sequence(a3)	bump header sequence number
		addq.l	#1,d0			is new extension contiguous...
		cmp.l	rwd_Header(a3),d0	...with the last one ?
		bne.s	ReadFFSDoIt		no, so don't read more data yet

; extension is contiguous with the previous one so we may as well read it now
		move.l	d1,-(sp)		d1 is holding current block #
		bsr	LooseA4HeaderBlock	free current header
		move.l	rwd_Header(a3),d0	and fetch the next one
		bsr	GrabBlockA4
****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a4
		move.l	(sp)+,d1		restore current block number
		move.l	cb_Error(a4),d4		any error ?
		bne.s	ReadFFSDoIt		yes, looseblock will discard

; See if we've collected up enough blocks to satisfy this portion of the read
10$		subq.l	#1,d5
		ble.s	ReadFFSDoIt		yep, transfer collected blocks

; See if the next block is contiguous and bump block count and loop if it is
		addq.l	#1,d1			bump block number
		cmp.l	cb_SIZEOF(a4,d6.w),d1	is next block contiguous
		bne.s	ReadFFSDoIt		nope, have to stop now
		addq.l	#1,d7			yes, bump block count
		bra.s	CollectBlocks		and look for the next one

; We've collected all the blocks we can for this transfer.  d7 = block count.
ReadFFSDoIt	move.l	d1,rwd_LastData(a3)	in case we write later
		move.w	d6,rwd_Entry(a3)	save new entry offset
		bsr	LooseA4HeaderBlock	if error occured will discard

; Before we can actually do this large transfer we must make sure that there
; are no blocks on the waiting queue that are in the same range as what we
; are about to read.  If we find any then we have to transfer them to the
; pending queue so they get written out before we attempt to read them again.
;
; Added a new field to cache buffers to make this a little faster.  Since we
; are only interested in data blocks belonging to the same file we check the
; new cb_Header field first before checking block ranges.  If the block on
; the waiting queue doesn't belong to the same file then we have no need to
; check for a collision with this transfer.
		move.l	rwd_FileHeader(a3),d1	cache file header key
		move.l	WaitingQueue(a5),d0	fetch the first entry
10$		movea.l	d0,a1			node pointer to a1
		move.l	(a1),d0			look ahead to next node
		beq.s	DoRead			end of list, do the read
		tst.l	cb_Size(a1)		is it a data block (size=0)
		bne.s	10$			no, so no need to check
		cmp.l	cb_Header(a1),d1	does it belong to this file
		bne.s	10$			no, so no need to check

		movem.l	d0-d1,-(sp)		got a data block in same file
		move.l	rwd_StartKey(a3),d1	see if it collides with read
		cmp.l	cb_Key(a1),d1
		bgt.s	20$			before our read range
		add.l	d7,d1			check top end
		cmp.l	cb_Key(a1),d1
		ble.s	20$			after our read range
; the cache buffer in a1 collides with our current read.  Transfer to pending.
		move.l	a1,d0			save pointer to cache buffer
		REMOVE				remove from Waiting
		movea.l	d0,a1			cache buffer to a1
		move.w	#HASHPENDING,cb_QueueType(a1)  mark the queue it's on
		lea.l	PendingQueue(a5),a0	queue to a0
		ADDTAIL				and add it there
20$		movem.l	(sp)+,d0-d1
		bra.s	10$			look for next node

; Nothing is hanging around that can hurt us.  Create a fake cache buffer on
; the stack and append it to the pending queue.  We then call Fetch() which
; will put this co-routine to sleep until the read request has completed.
DoRead		lea.l	-cb_SIZEOF(sp),sp	fake a cache buffer header
		move.w	BlockShift(a5),d0
		lsl.l	d0,d7			convert total blocks to bytes
		move.l	d7,cb_Size(sp)
		move.l	rwd_StartKey(a3),cb_Key(sp)
		move.l	a2,cb_Buff(sp)		where we are reading to
		move.l	CurrentCo(a5),cb_CoPkt(sp) we are the owning co-routine
		move.w	#CMD_READ,cb_State(sp)	what we are doing
		lea.l	PendingQueue(a5),a0	append to pending queue
		movea.l	sp,a1
		ADDTAIL
		move.l	sp,d0			wait for this read
		bsr	Fetch			will return here when complete
		move.l	cb_Size(sp),d0		assume it all worked
		move.l	cb_Error(sp),d4		stash error
*** this is a kluge for the 2090 which doesn't set IO_ACTUAL correctly
		beq.s	10$			correct, everything OK
; there was an error so use the value fetced from IO_ACTUAL as the real length
		move.l	cb_Actual(sp),d0	get actual data transferred
10$		lea.l	cb_SIZEOF(sp),sp	done with stack workspace
		bra.s	UpdateStats		update current position etc.

; this section is called when we are reading a partial block or reading data
; that is not on a block boundary or to an even address.  The current block
; is read into a cache buffer and the required data copied out with the CPU.
; Enters here with d0=offset into block, d1=block number, d2=amount to read
; d6 = current offset in hash table.
ReadFFSCache	move.l	d1,rwd_LastData(a3)	in case we write later
		move.l	BlockSize(a5),d5	calculate bytes to transfer
		sub.l	d0,d5			d5 = bytes left in block
		move.l	d2,d7			d7 = amount we want to transfer
		cmp.l	d7,d5			is amount>=bytesleft
		bgt.s	10$			nope, enough bytes in block

; The amount we need to read is more than is left in this block.  Truncate the
; transfer to amount left in this block and bump to the next hashtable entry.
		move.l	d5,d7			d7 holds amount to transfer
		subq.w	#4,d6			bump offset into hashtable
		cmpi.w	#fhb_HashTable,d6	and see if we need a new header
		bge.s	10$			nope, still in range

; We ran off the end of the current hashtable.  Move to the next one and update
; the state variables to reflect this.  In this case we don't actually have to
; read the extension block yet because we aren't going to use it straight away.
		move.l	rwd_Header(a3),rwd_PrevHeader(a3)
		lea.l	cb_SIZEOF(a4),a0
		adda.l	BlockSize(a5),a0	fetch extension key pointer
		move.l	vfhb_Extension(a0),rwd_Header(a3)
		move.w	MaxEntry(a5),d6		reset offset into hashtable
		addq.w	#1,rwd_Sequence(a3)	bump header sequence number

; loose the current extension block header and read the data block we want
10$		move.w	d6,rwd_Entry(a3)	save offset for next iteration
		move.l	d0,d6			stash byte offset for later
		move.l	d1,-(sp)		save the block number we want
		bsr	LooseA4HeaderBlock
		move.l	(sp)+,d0		fetch back block number
		bsr	WaitDataBlock		may discard later, so wait
		movea.l	d0,a4
		move.l	cb_Error(a4),d4		stash the error
		beq.s	20$

; Something failed while we were reading.  Trash the current buffer and exit.
		movea.l	a4,a0
		bsr	DiscardBlock
		moveq.l	#0,d7			no data transferred
		bra.s	30$			because something went wrong

; copy the relevant portion of the buffer (using d6=offset and d7=length)
20$		lea.l	cb_SIZEOF(a4,d6.w),a0	a0 = source data pointer
		movea.l	a2,a1			a1 = destination pointer
		move.l	d7,d0			d0 = length to copy
		jsr	_LVOCopyMem(a6)
		bsr	LooseA4HeaderBlock
30$		move.l	d7,d0			length for UpdateStats

; This iteration has finished.  Update the current position and amount of
; data transferred and loop back to see if there is anything else to do.
UpdateStats	add.l	d0,rwd_Offset(a3)	current position in file
		tst.l	d4			was there an error ?
		beq.s	10$			nope
		bsr	SetPosition		yes, make sure position is OK
10$		adda.l	d0,a2			next position in buffer
		add.l	d0,d3			update total amount read
		sub.l	d0,d2			amount left to do
		bgt	ReadFFSLoop

; This is the exit point.  D3 holds amount read and d4 holds the error code
ReadFFSDone	moveq.l	#-1,d0			assume an error occured
		move.l	d4,d1			and error code
		bne.s	ReadFFSExit		there was an error
		move.l	d3,d0			return amount read
		move.l	rwd_InFile(a3),d1	no error, return infile flag
ReadFFSExit	rts


;==============================================================================
; amount/error = WriteFFS( amount,buffer,rwdata ) globals,execbase
;   d0    d1		     d0     a0	   a1	    a5	     a6
;
; Attempts to write the given amount of data from buffer to the file specified
; by the rwdata entries.  This code should be run as a co-routine so it can use
; the cache buffer routines.  Whenever a write or a portion of a write is on
; contiguous blocks and from word aligned memory, the data is transferred from
; the callers buffer without going through the cache buffers.  The new bitmap
; allocation schemes make this a whole bunch faster and close to ReadFFS speed.
;
; A word about block placement schemes.  This filing system no longer attempts
; to keep headers and extension blocks away from data blocks; in typical usage
; this causes far too much seeking.  Extensions are now kept as close to the
; data as possible.  During large writes, extensions are allocated first so
; they tend to be grouped together.  This means files should be read using the
; same buffer size as when they were written for optimum performance.  Files
; written using small writes will have the extension blocks interleaved with
; the data blocks; this is perfect for files that are read using small buffers
;
; Note: there is quite a lot of code that is identical to the ReadFFS routine
; which implies the two routines could be merged together.  However, this was
; the case with 1.3 FFS and the checks required for each routine slowed the 
; other down unnescessarily.  We are looking for s-p-e-e-d here!
;
; Another note: due to the new synchronisation code, no two processes can write
; to the file at EXACTLY the same time.  One must finish the current write
; request before the other can start.  This means it's safe to allocate many
; blocks before actually writing them out.  This wasn't true under the old FFS
;
; There is support for SetFileSize in here too.  A buffer argument of 0 means
; there is no actual data to be written out but allocations should be done.
;==============================================================================
WriteFFS	movea.l	a0,a2			save buffer pointer (can be 0)
		movea.l	a1,a3			and file handle data pointer
		move.l	d0,d2			save amount to be written

		cmp.l	LargestWrite(a5),d2	read size to practical limits
		ble.s	5$
		move.l	LargestWrite(a5),d2

; check that it's OK to write to the disk (bitmap could be badly scrozzled too)
5$		bsr	DiskProtected		is this OK
		move.l	d0,d4			save error code
		beq.s	10$			no error, write's OK
		moveq.l	#0,d0			no data written
		bra	WriteFFSExit		exit now

; disk is OK, now check that it's OK to write to this file as well
10$		move.l	rwd_FileHeader(a3),d0	get the file header
		bsr	GrabBlock
****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a0
		move.l	cb_Error(a0),d4		any error ?

		lea.l	cb_SIZEOF(a0),a1	point to actual buffer data
		adda.l	BlockSize(a5),a1	reference from end of block
		move.l	vfhb_Protect(a1),d3	get size and protection bits
		move.l	vfhb_ByteSize(a1),rwd_CurrSize(a3)
		bsr	LooseHeaderBlock	done with header for now
		tst.l	d4			was there an error ?
		bne	WriteFFSExit		yes, exit now

		move.l	#ERROR_WRITE_PROTECTED,d4 assume write protected
		moveq.l	#0,d0			therefore no data written
		btst.l	#2,d3			is write allowed ?
		bne	WriteFFSExit		no, so return the error

		moveq.l	#0,d3			total written so far = 0
		moveq.l	#0,d4			current error = FALSE
		bsr	AlterRoot		things are changing
		clr.w	rwd_NewBlocks(a3)	no new blocks allocated yet
		moveq.l	#TRUE,d0		assume we're in the file
		move.l	d0,rwd_InFile(a3)	updated at the end

; if an error occurs we have to rewind to the original position
		move.l	rwd_Offset(a3),rwd_SavedOffset(a3)

; This is the main write loop that continues while there is data left and the
; error condition is false.  Contiguous blocks from word aligned addresses are
; written directly from callers buffer if AddressMask permits DMA from there
		tst.l	d2			anything left to write
		ble	WriteFFSDone		nope, exit now
WriteFFSLoop	tst.l	d4			any errors
		bne	WriteFFSDone		yes, exit now

; grab the current extension block header so we can find the current key
		move.l	rwd_Header(a3),d0
		bne	WriteGotHeader		there is a header there

; A previous Write ended on an extension block boundary.  This write has to
; allocate new extension block(s) and data blocks.  We attempt to get all of
; the extension blocks as close to each other as possible and then follow
; them with the associated data blocks.  Because of having to check for Mask
; and alignment cases we just allocate as many extension blocks as are needed
; for the maximum Write size we can do in one chunk and fill them in with the
; data keys.  We then leave it to the main loop to re-collect these contiguous
; data keys and actually transfer the data.  This means we will have the
; extension blocks on the disk pointing to garbage data.  However, validator
; will be able to pick this up if the system crashes because the Filesize will
; be less than the size dictated by the extension blocks.  FileSize is the
; final and absolute word on what data is valid.
; First, calculate how many blocks we need for the given amount of data.
		moveq.l	#TRUE,d0		will be writing new blocks
		move.w	d0,rwd_NewBlocks(a3)
		move.w	BlockShift(a5),d0	calculate blocks needed
		move.l	d2,d1
		add.l	BlockSize(a5),d1	round up to nearest block
		subq.l	#1,d1
		lsr.l	d0,d1			d1 holds # data blocks

		move.l	d1,d0			get total extensions needed
		add.l	HTSize(a5),d1		round up to nearest
		subq.l	#1,d1
		divu.w	HTSize+2(a5),d1		blocks/hashtable size
		ext.l	d1			d1 holds #extensions needed
		add.l	d0,d1			d1 holds blocks needed

		move.l	rwd_LastData(a3),d0	block we want to be near
		bsr	AllocKeys		see how many we get
		move.l	d0,rwd_Header(a3)	this is the new current header
		bne.s	10$			no errors, we got at least one

; The disk is full up or broken, report the error and abort this write
		move.l	d1,d4			error code returned in d1
		bra	WriteFFSDone		clean up and exit

; Before we start stuffing data key numbers into the extension blocks, we need
; to know how many extension blocks are needed for the chunk of disk we just
; allocated.  The reason is that we want to keep all the extension blocks for
; this write grouped together so we can't interleave them with the data keys.
10$		movem.l	d2-d6,-(sp)		need some extra work registers
		move.l	d0,d2			start key to d2
		move.l	d1,d3			total keys to d3
		moveq.l	#0,d4			total extension keys
20$		addq.l	#1,d4			used one extra extension
		subq.l	#1,d1			so one less key spare
		beq.s	30$			none left
		sub.l	HTSize(a5),d1		# keys for this extension
		bgt.s	20$			some left

; d4 holds the number of extension blocks we will be using.  Subtract this
; from the total number of keys to find how many data slots are available.
30$		sub.l	d4,d3			d3 = keys remaining for data
		move.l	d2,d5
		add.l	d4,d5			d5 = start key for data blocks
		move.l	rwd_PrevHeader(a3),d6	d6 = previous header

; main loop for filling the extensions we just got with data block numbers
AddExtensions	move.l	d6,d0			fetch previous header
		bsr	GrabBlock
****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a0
		bra.s	EndExtensions		enter extension loop

LoopExtensions	lea.l	cb_SIZEOF(a0),a1
		adda.l	BlockSize(a5),a1	add next extension key pointer
		move.l	d2,vfhb_Extension(a1)
		bsr	WriteSummedBlockD6A0Later_RWD  D6,A0 with cb_header

		bsr	GetBlock		fetch a cache buffer
		movea.l	d0,a4
		lea.l	cb_SIZEOF(a4),a0
		bsr	ClearBlock		clear it out
		lea.l	cb_SIZEOF(a4),a0
		move.l	#t.list,fhb_Type(a0)	it's an extension
		move.l	d2,fhb_OwnKey(a0)	on this block

; see how many data keys are available for filling in this extension block
		move.l	HTSize(a5),d0		assume we can fill it
		cmp.l	d0,d3
		bge.s	10$			we can
		move.l	d3,d0			nope, truncate to this many
10$		sub.l	d0,d3			adjust number of keys left
		move.l	d0,fhb_HighSeq(a0)	set number of slots used
		move.w	MaxEntry(a5),d1		get first offset to hash table
		lea.l	4(a0,d1.w),a1		point one beyond the end
		bra.s	30$			enter fill loop

20$		move.l	d5,-(a1)		save next data key
		addq.l	#1,d5			bump key (they're contiguous)
30$		dbra	d0,20$			and keep looking
		move.l	d5,rwd_LastData(a3)	next place to allocate data

		adda.l	BlockSize(a5),a0
		move.l	rwd_FileHeader(a3),vfhb_Parent(a0) owning header
		move.l	#st.file,vfhb_SecType(a0)	  secondary type
		movea.l	a4,a0			writing this extension
		move.l	d2,d6			this block now previous
		addq.l	#1,d2			bump to the next block
EndExtensions	dbra	d4,LoopExtensions	and do other extensions

		lea.l	cb_SIZEOF(a0),a1
		adda.l	BlockSize(a5),a1	add next extension key pointer
		clr.l	vfhb_Extension(a1)	there is none
		bsr	WriteSummedBlockD6A0Later_RWD  D6,A0 with cb_header

		movem.l	(sp)+,d2-d6		restore work registers
		move.l	rwd_Header(a3),d0	setup for WriteGotHeader

; extension(s) are allocated so see if key slots in extension are done too
WriteGotHeader	bsr	GrabBlockA4
****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a4			stash current header
		move.w	rwd_Entry(a3),d6	cache current entry offset
		move.l	cb_SIZEOF(a4,d6.w),d1	get starting block number
		bne	WriteGotKey

; A previous read or write ended at EOF but not on an extension block boundary.
; Due to the way data are written we should only try to allocate data keys for
; the remainder of this extension.  The data blocks will probably be jammed
; right up against the end of this extension so we wouldn't get contiguous
; extensions anyway.
;
; We can ALWAYS assume that this write starts on a block boundary (though it
; may end nowhere near a block boundary).  The new write that starts in the
; middle of a block will always find the key already allocated.
;
; BUG: because the file header is treated slightly differently to extension
; blocks (ie. it's ALWAYS present) we allocate the first HTSize blocks right
; after the file header.  This means that all files >HTSize blocks long will
; have at least one frag in them.  Needs a big chunk of special-case code to
; fix this, so I don't think I'm going to bother.
		moveq.l	#TRUE,d0
		move.w	d0,rwd_NewBlocks(a3)	will be writing to new blocks
		move.w	BlockShift(a5),d0	calculate blocks needed
		move.l	d2,d1
		add.l	BlockSize(a5),d1	round up to nearest block
		subq.l	#1,d1
		lsr.l	d0,d1			d1 holds # data blocks required
		move.l	HTSize(a5),d0		see how many fit in extension

		sub.l	cb_SIZEOF+fhb_HighSeq(a4),d0
		cmp.l	d0,d1			use min(required,remaining)
		ble.s	10$			keys required will fit OK
		move.l	d0,d1			truncate to remaining slots

; The new bitmap schemes can cause a read or write of the disk to happen.  To
; be safe I'm going to loose the current extension key.  However, because of
; the new file access arbitration this should not really be nescessary since
; no-one else can dick with this file until the current write has completed.
; The main argument for doing this is that we can still run with a single
; cache buffer.  If we held onto the extension we'd need at least two.
10$		move.l	d1,-(sp)		save #blocks required
		bsr	LooseA4HeaderBlock
		move.l	(sp)+,d1		fetch #blocks required again
		move.l	rwd_LastData(a3),d0	want to be near this key
		bsr	AllocKeys		and allocate disk space
		tst.l	d0			did it work ?
		bne.s	20$			yep, no problems

; The disk is full up or broken, report the error and abort this write
		move.l	d1,d4			error code returned in d1
		bra	WriteFFSDone		clean up and exit

20$		movem.l	d0-d1,-(sp)		shit, this is a pain
		move.l	rwd_Header(a3),d0	fetch the old extension back
		bsr	GrabBlockA4		it should still be cached
****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a4
		movem.l	(sp)+,d0-d1		get startkey and count
		add.l	d1,cb_SIZEOF+fhb_HighSeq(a4)  adjust highseq count
		lea.l	cb_SIZEOF+4(a4,d6.w),a0	point to next available slot
		bra.s	40$			and enter filling loop
30$		move.l	d0,-(a0)		store key number
		addq.l	#1,d0			bump to next (contiguous)
40$		dbra	d1,30$			loop while keys left
		move.l	d0,rwd_LastData(a3)	next place to allocate data

; I'm actually doing a write here just to set things up properly.  However,
; I think it will be OK to set cb_State to write and allow the LooseBlock
; call that's done later to cause this block to be written out. cb_Key
; should already be set (since we read the block in the first place).
		movea.l	a4,a0			write this buffer
		move.l	rwd_Header(a3),d0	to this block
		bsr	WriteSummedBlockA0Later_RWD  A0 with cb_header
		move.l	rwd_Header(a3),d0	grab block straight back
		bsr	GrabBlockA4
****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a4
		move.l	cb_SIZEOF(a4,d6.w),d1	refetch starting block number

; we have a data key to write to (in d1) check all the mask and transfer stuff
WriteGotKey	move.l	BlockSize(a5),d0	are we on a block boundary
		subq.l	#1,d0
		and.l	rwd_Offset(a3),d0	WriteFFSCache needs d0=offset
		bne	WriteFFSCache		no, must use cache buffer
		cmp.l	BlockSize(a5),d2	at least one block left ?
		blt	WriteFFSCache		no, so use cache buffer
		move.l	a2,d5			check address
		beq.s	5$			there's no buffer (setfilesize)
		and.l	GAddressMask(a5),d5	...in AddressMask range
		bne	WriteFFSCache		odd or failed mask, use cache

; We have at least one block left to transfer direct from the callers buffer.
; Check we aren't violating MaxTransfer and truncate amount to do if we are
; Because the check is here, MaxTransfer must be 1 block or more. (Bug????)
5$		move.l	d2,d5			cache number of bytes to do
		cmp.l	GMaxTransfer(a5),d5	can we do this many bytes ?
		ble.s	10$			yep, no adjustment
		move.l	GMaxTransfer(a5),d5	nope, adjust to MaxTransfer
10$		move.w	BlockShift(a5),d0
		lsr.l	d0,d5			d5 holds block count

		moveq.l	#1,d7			accrued total to transfer
		move.l	d1,rwd_StartKey(a3)	save key we are starting at

; Loop around collecting blocks until we have them all or the blocks are not
; contiguous on the disk.  Update CurrentEntry and CurrentHeader as we go.
; Due to the way the writes are performed we don't have to check for required
; allocations here since any blocks not allocated will just fail the checks
; for contiguous blocks.  When these checks fail, the collected data is
; transferred and we go back to the top of the loop where allocation checks
; are performed.
WCollectBlocks	subq.w	#4,d6			bump to next extension entry
		cmpi.w	#fhb_HashTable,d6
		bge.s	ll10			still within hash table bounds

; We've run off the end of the hash table.  Grab the next extension block and
; update the rwd state variables as required.  If the next extension is not
; contiguous with the previous one then we won't actually read it.  Due to the
; way the files are written this would mean we are about to hit a frag anyway.
; If we did read non-contiguous extensions we would end up seeking forward
; HashTabSize blocks and then back again to write the collected data blocks.
		move.l	rwd_Header(a3),d0	save current extension key
		move.l	d0,rwd_PrevHeader(a3)	may need it later
		lea.l	cb_SIZEOF(a4),a0
		adda.l	BlockSize(a5),a0	fetch extension key pointer
		move.l	vfhb_Extension(a0),rwd_Header(a3)
		move.w	MaxEntry(a5),d6		reset offset into hashtable
		addq.w	#1,rwd_Sequence(a3)	bump header sequence number
		addq.l	#1,d0			is new extension contiguous...
		cmp.l	rwd_Header(a3),d0	...with the last one ?
		bne.s	WriteFFSDoIt		no, so don't write more data yet

; extension is contiguous with the previous one so we may as well read it now
		move.l	d1,-(sp)		d1 is holding current block #
		bsr	LooseA4HeaderBlock	free current header
		move.l	rwd_Header(a3),d0	and fetch the next one
		bsr	GrabBlockA4
****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a4			set up buffer pointer
		move.l	(sp)+,d1		restore current block number

; See if we've collected up enough blocks to satisfy this portion of the write
ll10		subq.l	#1,d5
		ble.s	WriteFFSDoIt		yep, transfer collected blocks

; See if the next block is contiguous and bump block count and loop if it is
; If block is not allocated then it will be zero and cause this check to fail
		addq.l	#1,d1			bump block number
		cmp.l	cb_SIZEOF(a4,d6.w),d1	is next block contiguous
		bne.s	WriteFFSDoIt		nope, have to stop now
		addq.l	#1,d7			yes, bump block count
		bra.s	WCollectBlocks		and look for the next one

; We've collected all the blocks we can for this transfer.  d7 = block count.
WriteFFSDoIt	move.w	d6,rwd_Entry(a3)	save new entry offset
		bsr	LooseA4HeaderBlock

; Before we can actually do this large transfer we must make sure that there
; are no blocks on the waiting or valid queues that are in the same range as
; what we are about to write.  If we find any on either of these queues then
; they are summarily discarded because we are about to invalidate their data
; Cache buffers in collision are also removed from the CacheHash table.
;
; Added a new field to cache buffers to make this a little faster.  Since we
; are only interested in data blocks belonging to the same file we check the
; new cb_Header field first before checking block ranges.  If the block on
; the waiting or valid queue doesn't belong to the same file then we have no
; need to check for a collision with this transfer.
;
; if we add writebehind, this will need modification.  FIX
		move.l	rwd_FileHeader(a3),d1	cache file header key
		move.l	WaitingQueue(a5),d0	fetch the first entry
10$		movea.l	d0,a1			node pointer to a1
		move.l	(a1),d0			look ahead to next node
		beq.s	CheckValid		end of list, check valid queue
		tst.l	cb_Size(a1)		is it a data block (size=0)
		bne.s	10$			no, so no need to check
		cmp.l	cb_Header(a1),d1	does it belong to this file
		bne.s	10$			no, so no need to check

		movem.l	d0-d1,-(sp)		got a data block in same file
		move.l	rwd_StartKey(a3),d1	see if it collides with write
		cmp.l	cb_Key(a1),d1
		bgt.s	20$			before our write range
		add.l	d7,d1			check top end
		cmp.l	cb_Key(a1),d1
		ble.s	20$			after our write range
; The cache buffer in a1 collides with our current write.  Discard it.
		move.l	a1,a0
		bsr	FreeBuffer
20$		movem.l	(sp)+,d0-d1
		bra.s	10$			look for next node

CheckValid	move.l	ValidBufferQueue(a5),d0	fetch the first entry
10$		movea.l	d0,a1			node pointer to a1
		move.l	(a1),d0			look ahead to next node
		beq.s	DoWrite			end of list, check valid queue
		tst.l	cb_Size(a1)		is it a data block (size=0)
		bne.s	10$			no, so no need to check
		cmp.l	cb_Header(a1),d1	does it belong to this file
		bne.s	10$			no, so no need to check

		movem.l	d0-d1,-(sp)		got a data block in same file
		move.l	rwd_StartKey(a3),d1	see if it collides with write
		cmp.l	cb_Key(a1),d1
		bgt.s	20$			before our write range
		add.l	d7,d1			check top end
		cmp.l	cb_Key(a1),d1
		ble.s	20$			after our write range
; The cache buffer in a1 collides with our current write.  Discard it.
		move.l	a1,a0
		bsr	FreeBuffer
20$		movem.l	(sp)+,d0-d1
		bra.s	10$			look for next node

; Nothing is hanging around that can hurt us.  Create a fake cache buffer on
; the stack and append it to the pending queue.  We then call Fetch() which
; will put this co-routine to sleep until the write request has completed.
; This also means that if there is a collision with a buffer in the pending
; queue, all is safe, as this will be written after that.  Also, since all
; reads/writes to a file go through 1 coroutine, nothing will grab them off
; the pending queue and drop them on the rear.
DoWrite		cmpa.w	#0,a2			is there any data to write ?
		bne.s	5$			yes
		move.w	BlockShift(a5),d0
		lsl.l	d0,d7			convert total blocks to bytes
		move.l	d7,d0			into d0 for Wupdatestats
		bra	WUpdateStats		nope, so just bump position

5$		lea.l	-cb_SIZEOF(sp),sp	fake a cache buffer header
		move.w	BlockShift(a5),d0
		lsl.l	d0,d7			convert total blocks to bytes
		move.l	d7,cb_Size(sp)
		move.l	rwd_StartKey(a3),cb_Key(sp)
		move.l	a2,cb_Buff(sp)		where we are writing from
		move.l	CurrentCo(a5),cb_CoPkt(sp) we are the owning co-routine
		move.w	#CMD_WRITE,cb_State(sp)	what we are doing
		lea.l	PendingQueue(a5),a0	append to pending queue
		movea.l	sp,a1
		ADDTAIL
		move.l	sp,d0			wait for the write to finish
		bsr	Fetch			will return here when complete
*** this is a kluge for the 2090 which doesn't set IO_ACTUAL correctly
		move.l	cb_Size(sp),d0
		move.l	cb_Error(sp),d4		stash error
		beq.s	10$			assume no error means size OK
		move.l	cb_Actual(sp),d0	get actual data transferred
10$		lea.l	cb_SIZEOF(sp),sp	done with stack workspace
		bra	WUpdateStats		update current position etc.

; this section is called when we are writing a partial block or writing data
; that is not on a block boundary or to an even address.  The current block
; is read into a cache buffer and the required data copied in with the CPU
; before writing the block back out to disk.  If we are allocating new data
; blocks then we don't have to do the intial read of the required block.
; Enters here with d0=offset into block, d1=block number, d2=amount to write
; d6 = current offset in hash table.
WriteFFSCache	move.l	BlockSize(a5),d5	calculate bytes to transfer
		sub.l	d0,d5			d5 = bytes left in block
		move.l	d2,d7			d7 = amount we want to transfer
		cmp.l	d7,d5			if amount>=bytesleft
		bgt.s	10$			nope, enough bytes in block

; The amount we need to write is more than is left in this block.  Truncate the
; transfer to amount left in this block and bump to the next hashtable entry.
		move.l	d5,d7			d7 holds amount to transfer
		subq.w	#4,d6			bump offset into hashtable
		cmpi.w	#fhb_HashTable,d6	and see if we need a new header
		bge.s	10$			nope, still in range

; We ran off the end of the current hashtable.  Move to the next one and update
; the state variables to reflect this.  In this case we don't actually have to
; read the extension block yet because we aren't going to use it straight away.
		move.l	rwd_Header(a3),rwd_PrevHeader(a3)
		lea.l	cb_SIZEOF(a4),a0
		adda.l	BlockSize(a5),a0	fetch extension key pointer
		move.l	vfhb_Extension(a0),rwd_Header(a3)
		move.w	MaxEntry(a5),d6		reset offset into hashtable
		addq.w	#1,rwd_Sequence(a3)	bump header sequence number

; loose the current extension block header and read the data block we want
10$		move.w	d6,rwd_Entry(a3)	save offset for next iteration
		move.l	d0,d6			stash byte offset for later
		move.l	d1,-(sp)		save the block number we want
		bsr	LooseA4HeaderBlock

; if we just allocated this block (or blocks) then the rwd_NewBlocks flag will
; have been set and there is no need for us to read it from disk before writing
		cmpa.w	#0,a2			any data to write ?
		bne.s	15$			yes
		move.l	d7,d0			set value for WUpdateStats
		addq.l	#4,sp			scrap value on stack
		bra.s	WUpdateStats		just update offset etc.

15$		tst.w	rwd_NewBlocks(a3)	any need to read first
		beq.s	20$			yes, there's old data there too
		bsr	GetBlock		fetch a new cache buffer
		bra.s	30$			and fill it
20$		move.l	(sp),d0			fetch data block number
		bsr	GrabDataBlock		and read it
****** need to check for an error condition if the header is garbage ******

; copy the relevant portion of the buffer (using d6=offset and d7=length)
30$		movea.l	d0,a4
		lea.l	cb_SIZEOF(a4,d6.w),a1	a1 = destination pointer
		movea.l	a2,a0			a0 = source data pointer
		move.l	d7,d0			d0 = length to copy
		jsr	_LVOCopyMem(a6)
		movea.l	a4,a0			write out this block
		move.l	(sp)+,d0		to this disk key
		moveq.l	#FALSE,d1		and don't checksum it
		move.l	rwd_FileHeader(a3),cb_Header(a0)  belongs to this file
		bsr	WriteBlockLater		put it on Waiting queue
		move.l	d7,d0			length for WUpdateStats

; This iteration has finished.  Update the current position and amount of
; data transferred and loop back to see if there is anything else to do.
WUpdateStats	add.l	d0,rwd_Offset(a3)	current position in file
		tst.l	d4			was there an error ?
		beq.s	10$			nope
		bsr	SetPosition		yes, make sure position is OK
10$		cmpa.w	#0,a2			was there a buffer
		beq.s	11$			nope
		adda.l	d0,a2			next position in buffer
11$		add.l	d0,d3			update total amount written
		sub.l	d0,d2			amount left to do
		bgt	WriteFFSLoop

; This write request has finished.  Update the fileheader with the new size
; if it has changed.  We wait for the close packet before doing DateStamping
; and archive bit resetting.
; Remember, unsigned file positions!
WriteFFSDone	moveq.l	#TRUE,d0
		move.w	d0,rwd_Altered(a3)	file has been modified
		move.l	rwd_Offset(a3),d0	fetch current byte offset
		cmp.l	rwd_CurrSize(a3),d0	have we extended the file ?
		bls.s	WriteFFSExit		nope, we are still <EOF
		move.l	rwd_FileHeader(a3),d0	yes, need to update size
		bsr	GrabBlock
****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a0
		lea.l	cb_SIZEOF(a0),a1	point to actual buffer data
		adda.l	BlockSize(a5),a1	reference from end of block
		move.l	vfhb_Junk1-4(a1),cb_SIZEOF+fhb_FirstBlock(a0)
		move.l	rwd_Offset(a3),vfhb_ByteSize(a1)  set the new size
		move.l	rwd_FileHeader(a3),d0	write back the header
		moveq.l	#TRUE,d1		checksum it first
		move.l	d0,cb_Header(a0)
		bsr	WriteBlockLater		done with header for now
		clr.l	rwd_InFile(a3)		not in the file anymore
WriteFFSExit	move.l	d4,d1			error code to d1
		bne.s	10$			there was an error
		move.l	d3,d0			amount written to d0
		move.l	rwd_InFile(a3),d1	no error, return infile flag
		rts

; when an error occurs we rewind to the original position (no length in error)
10$		move.l	rwd_SavedOffset(a3),rwd_Offset(a3)
		bsr	SetPosition
		moveq.l	#-1,d0
		rts


;==============================================================================
; position/error = SeekFFS(offset,mode,rwdata)
;    d0	    d1		     d0    d1    a1
;
; Seeks to the given position in the file.  Mode determines what the seek is
; relative to as follows:-
;	-1 = OFFSET_BEGINNING
;	 0 = OFFSET_CURRENT
;	 1 = OFFSET_END
; If the resultant seek position is before BOF or after EOF then SEEK_ERROR
;==============================================================================
SeekFFS		movea.l	a1,a3			save rwd stuff
		move.l	d0,d2			save required seek offset
		move.l	d1,d3			and required seek mode

; We need to know the current size of the file for checking and OFFSET_END
		move.l	rwd_FileHeader(a3),d0	fetch the header
		bsr	GrabBlock
;		movea.l	d0,a0
		lea.l	cb_SIZEOF(a0),a1	point to actual buffer data
		adda.l	BlockSize(a5),a1	reference from end of block
		move.l	vfhb_ByteSize(a1),d4	get current file size
		bsr	LooseHeaderBlock	finished with header

		tst.l	d3			what kind of seek
		bmi.s	GotSeek			<0 so from start
		bne.s	10$			>0 so from end
		add.l	rwd_Offset(a3),d2	=0 so from current
		bra.s	GotSeek
10$		add.l	d4,d2			compute new offset from end

; We have an absolute seek position in d2.  Check that it's a valid position.
; FIX!!! file sizes should be UNsigned!!!!
GotSeek		tst.l	d2			valid seek position
		bmi.s	SeekError		nope, negative offset is bad
; FIX!!! file sizes should be UNsigned!!!!
		cmp.l	d4,d2
		bgt.s	SeekError		past end of file is bad too
		move.l	rwd_Offset(a3),d4	save current position
		move.l	d2,rwd_Offset(a3)	this will be our new offset
		bsr	SetPosition		set it to this

; Seek has completed OK, return FALSE in d1 and the seek position in d0
SeekComplete	moveq.l	#FALSE,d1		no error to report
		move.l	d4,d0			return old position
		rts

; Something was wrong with the seek position.  Return a generic SEEK_ERROR.
SeekError	move.l	#ERROR_SEEK_ERROR,d1	return an error
		moveq.l	#-1,d0			Indicate failure
SeekExit	rts


;==============================================================================
; size/error = SetSizeFFS(offset,mode,rwdata)
;  d0	d1		    d0    d1    a1
;
; Sets a file to the required size (bigger or smaller).  The size is determined
; using an offset in exactly the same way as Seek as follows:-
;	-1 = OFFSET_BEGINNING
;	 0 = OFFSET_CURRENT
;	 1 = OFFSET_END
; If the file is made smaller than the current seek position then the current
; position is adjusted to EOF.  If the file is made larger, then the seek
; position remains the same.  If this file is opened more than once and the
; file is being truncated, the truncation may not be able to work completely
; because it would leave another reader/writer beyond end of file.  In this
; case, the truncation is performed to the largest current seek position.
;==============================================================================
SetSizeFFS	movea.l	a1,a3			save rwd stuff
		move.l	d0,d2			save required size offset
		move.l	d1,d3			and required seek mode

; We need to know the current size of the file for checking and OFFSET_END
		move.l	rwd_FileHeader(a3),d0	fetch the header
		bsr	GrabBlock
;		movea.l	d0,a0
		lea.l	cb_SIZEOF(a0),a1	point to actual buffer data
		adda.l	BlockSize(a5),a1	reference from end of block
		move.l	vfhb_ByteSize(a1),d4	get current file size
		bsr	LooseHeaderBlock	finished with header

		tst.l	d3			what kind of offset
		bmi.s	GotSize			<0 so from start
		bne.s	10$			>0 so from end
		add.l	rwd_Offset(a3),d2	=0 so from current
		bra.s	GotSize
10$		add.l	d4,d2			compute new offset from end

; We have an absolute size in d2.  Check that it's valid before proceeding
; FIX!!! file sizes should be UNsigned!!!!
GotSize		tst.l	d2			valid size ?
		bge.s	SizeOK			yes
		move.l	#ERROR_SEEK_ERROR,d1	return an error
		move.l	d4,d0			and the current size
		rts

; now check to see if we're extending the file or truncating it.
; FIX!!! file sizes should be UNsigned!!!!
SizeOK		cmp.l	d4,d2
		bgt.s	MakeBigger
		blt.s	MakeSmaller		we're truncating it

; the file is already at the correct size so immediately return given size
		moveq.l	#FALSE,d1		no error to report
		move.l	d4,d0			return current size
		rts

; we're making the file bigger (d2 holds actual size required).  Calculate
; how many bytes need adding to the end of the file and call Write with a
; NULL buffer argument to extend the file.
MakeBigger	printf <'Making file larger\n'>
		move.l	rwd_Offset(a3),-(sp)	save current position
		move.l	d4,rwd_Offset(a3)	seek to end of file
		bsr	SetPosition
		sub.l	d4,d2			d2=amount to be written
		move.l	d2,d0			d0=amount
		suba.l	a0,a0			no buffer
		movea.l	a3,a1			set up rwdata
		movem.l	a2/a3,-(sp)		save regs for WriteFFS
		bsr	WriteFFS		append space to end of file
		movem.l	(sp)+,a2/a3
		move.l	rwd_Offset(a3),d0	get new size (should be bigger)
		move.l	(sp)+,rwd_Offset(a3)	restore old seek position
		bra.s	SetPosition		will return current d0/d1

; We're making the file smaller.  This is a lot more complicated because we
; have to check that we're not going to leave another reader/writer hanging
; in space after the end of the file.  We limit the truncation to the 
; largest seek position other than our own (which will be adjusted to suit)
MakeSmaller	printf <'Making file smaller\n'>
		bsr	AlterRoot
		movea.l	rwd_Owner(a3),a0	get address of rw_Header
		movea.l	rwh_StateList(a0),a0	and fetch first rwd_Data struct
10$		move.l	a0,d0			end of list ?
		beq.s	SizeModified		yep, d2 holds new size
		cmpa.l	a0,a3			is it the current rw_data ?
		beq.s	20$			yes, so ignore it
; FIX!!! file sizes should be UNsigned!!!!
		cmp.l	rwd_Offset(a0),d2	is it past required size
		bge.s	20$			nope, it's before it
		move.l	rwd_Offset(a0),d2	yes, can only truncate to here
20$		movea.l	(a0),a0			chain to next rwd_Data
		bra.s	10$			and keep looking

; we can now safely set the file to the (maybe modified) size in d2.
SizeModified	move.l	d2,rwd_Offset(a3)	seek us to the new position
		bsr.s	SetPosition
	printf <'Seeked to position %ld\n'>,d2

; Now free all data keys and extensions from the current position onwards
		move.l	d2,d0			get current offset
		move.w	BlockShift(a5),d1	convert to a block number
		lsr.l	d1,d0
		divu.w	HTSize+2(a5),d0		get extension sequence number
		swap	d0			remainder is blocks in extension
		ext.l	d0
		move.l	d0,d1			clear from this block...
		move.l	BlockSize(a5),d0	...if offset into block is 0
		subq.l	#1,d0
		and.l	d2,d0
		beq.s	10$			offset into block is 0
		addq.l	#1,d1			there's data in first block
10$		move.l	rwd_Header(a3),d0	clearing from this header
		bsr	ClearFile		go free extra data

; file has been truncated to the right size.  Now set that size in the header.
		move.l	rwd_FileHeader(a3),d0
		bsr	GrabBlock
;		movea.l	d0,a0
		lea.l	cb_SIZEOF(a0),a1
		adda.l	BlockSize(a5),a1
		move.l	d2,vfhb_ByteSize(a1)
		move.l	rwd_FileHeader(a3),d0
		bsr	WriteSummedBlockA0Later_RWD  D0,A0 with cb_header
		move.l	d2,d0			return new size
		moveq.l	#FALSE,d1		no error
		rts

;==============================================================================
; SetPosition( rwd_Data )
;		 a3
;
; A special internal routine for rwffs.asm, makes sure that all the state
; variables are set up correctly for the current offset.  Saves all registers.
;==============================================================================
SetPosition	movem.l	d0-d2/a0-a1,-(sp)
		move.l	rwd_Offset(a3),d0	get current offset
		move.w	BlockShift(a5),d1
		lsr.l	d1,d0			d0 holds block number
		divu.w	HTSize+2(a5),d0		d0 = extension sequence number
		move.w	d0,-(sp)		save this for later
		swap	d0			compute offset into last ext
		lsl.w	#2,d0			convert to a real byte offset
		move.w	MaxEntry(a5),d1
		sub.w	d0,d1
		move.w	d1,rwd_Entry(a3)	save current offset
		move.w	(sp)+,d2		get back sequence number
		cmp.w	rwd_Sequence(a3),d2	do we need to rewind ?
		bge.s	PosFindHeader		nope, we're behind it
		move.l	rwd_FileHeader(a3),rwd_Header(a3)
		clr.l	rwd_PrevHeader(a3)	need to rewind
		clr.w	rwd_Sequence(a3)

PosFindHeader	cmp.w	rwd_Sequence(a3),d2	have we found our header ?
		beq.s	PositionSet		yep, position is set up
		addq.w	#1,rwd_Sequence(a3)	no, bump to next extension
		move.l	rwd_Header(a3),d0	fetch the current extension
		move.l	d0,rwd_PrevHeader(a3)	it becomes previous one
		bsr	GrabBlock		fetch it
;		movea.l	d0,a0
		lea.l	cb_SIZEOF(a0),a1	point to actual buffer data
		adda.l	BlockSize(a5),a1	reference from end of block
		move.l	vfhb_Extension(a1),rwd_Header(a3)  set up new header
		bsr	LooseHeaderBlock	finished with this header
		bra.s	PosFindHeader		see if we've finished

PositionSet	move.l	rwd_FileHeader(a3),rwd_LastData(a3)
		movem.l	(sp)+,d0-d2/a0-a1
		rts

		END
