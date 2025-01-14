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

		XDEF	ReadOFS,WriteOFS,SeekOFS,SetSizeOFS

		XREF	GrabBlock,WriteBlockLater,Fetch,AllocKeys,CallCo
		XREF	GrabBlockA3,GrabBlockA4
		XREF	LooseBlock,GetBlock,GrabDataBlock,DiskProtected
		XREF	AlterRoot,ClearBlock,UnHash,DiscardBlock,WriteBlockNow
		XREF	WaitDataBlock,ClearFile,LooseHeaderBlock
		XREF	WriteSummedBlockA0Later_RWD,LooseA4HeaderBlock

		XREF	_LVOCopyMem

;==============================================================================
; amount/error = ReadOFS( amount,buffer,rwdata ) globals,execbase
;   d0    d1		    d0	   a0	  a1	   a5	   a6
;
; Attempts to read the given amount of data into buffer from the file specified
; by the rwdata entries.  This code should be run as a co-routine so it can use
; the cache buffer reading routines.  This supports old file system format that
; has checksum and block information at the beginning of each data block.
;==============================================================================
ReadOFS		movea.l	a0,a2			save buffer pointer
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
****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a0
		lea.l	cb_SIZEOF(a0),a1	point to actual buffer data
		adda.l	BlockSize(a5),a1	reference from end of block
		move.l	vfhb_Protect(a1),d3	get size and protection bits
		move.l	vfhb_ByteSize(a1),d4
		bsr	LooseHeaderBlock	done with header for now

		move.l	#ERROR_READ_PROTECTED,d1 assume read protected
		moveq.l	#0,d0			therefore no data read yet
		btst.l	#3,d3			is read allowed ? ****
		bne	ReadOFSExit		no, so return the error

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
; error condition is false.  
;**** bug fix, negative read amounts could do nasty things (old FFS exited)
		tst.l	d2			anything left to read
		ble	ReadOFSDone		nope, exit now
ReadOFSLoop	tst.l	d4			any errors
		bne	ReadOFSDone		yes, exit now

; grab the current extension block header so we can find the current key
		move.l	rwd_Header(a3),d0
		bsr	GrabBlockA4
****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a4			stash current header

		move.w	rwd_Entry(a3),d6	cache current entry offset
		move.l	cb_SIZEOF(a4,d6.w),d1	get starting block number
		move.l	rwd_Offset(a3),d0	get current offset
		divu.w	BytesPerData+2(a5),d0	figure out offset into block
		swap	d0			remainder is current offset
		ext.l	d0			using a long later

; read the current block into a cache buffer and copy the required data out.
; Enters here with d0=offset into block, d1=block number, d2=amount to read
; d6 = current offset in hash table.
		move.l	d1,rwd_LastData(a3)	in case we write later
		move.l	BytesPerData(a5),d5	calculate bytes to transfer
		sub.l	d0,d5			d5 = bytes left in block
		move.l	d2,d7			d7 = amount we want to transfer
		cmp.l	d7,d5			if amount>=bytesleft
		bgt.s	10$			nope, enough bytes in block

; The amount we need to read is more than is left in this block.  Truncate the
; transfer to amount left in this block and bump to the next hashtable entry.
;		addq.l	#1,rwd_DataSeq(a3)	moving to next data block
;	move.w	d7,rwd_WroteAll(a3)	don't need to hold data block
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
;		bsr	GrabDataBlock		and read it
		bsr	WaitDataBlock		may discard later, so wait

; copy the relevant portion of the buffer (using d6=offset and d7=length)
		movea.l	d0,a4
		move.l	cb_Error(a4),d4		get error (if any)
		beq.s	20$			no error occured
		movea.l	d0,a0
		bsr	DiscardBlock		trash the current block
		moveq.l	#0,d0			no data read
		bra.s	UpdateStats		fix everything up

20$		lea.l	cb_SIZEOF+fdb_Data(a4,d6.w),a0	a0 = source data pointer
		movea.l	a2,a1			a1 = destination pointer
		move.l	d7,d0			d0 = length to copy
		jsr	_LVOCopyMem(a6)
		bsr	LooseA4HeaderBlock
		move.l	d7,d0			length for UpdateStats

; This iteration has finished.  Update the current position and amount of
; data transferred and loop back to see if there is anything else to do.
UpdateStats	add.l	d0,rwd_Offset(a3)	current position in file
		tst.l	d4			was there an error ?
		beq.s	10$			nope
		bsr	SetPosition		yes, make sure position is OK
10$		adda.l	d0,a2			next position in buffer
		add.l	d0,d3			update total amount read
		sub.l	d0,d2			amount left to do
		bgt	ReadOFSLoop

; This is the exit point.  D3 holds amount read and d4 holds the error code
ReadOFSDone	moveq.l	#0,d0			assume an error occured
		move.l	d4,d1			and error code
		bne.s	ReadOFSExit		there was an error
		move.l	d3,d0			return amount read
		move.l	rwd_InFile(a3),d1	no error, return infile flag
ReadOFSExit	rts


;==============================================================================
; amount/error = WriteOFS( amount,buffer,rwdata ) globals,execbase
;   d0    d1		     d0     a0	   a1	    a5	     a6
;
; Attempts to write the given amount of data from buffer to the file specified
; by the rwdata entries.  This code should be run as a co-routine so it can use
; the cache buffer routines.  Compatible with old filing system format.
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
; Another note: due to the new synchronisation code, no two processes can write
; to the file at EXACTLY the same time.  One must finish the current write
; request before the other can start.  This means it's safe to allocate many
; blocks before actually writing them out.  This wasn't true under the old FFS
;
; There is support for SetFileSize in here too.  A buffer argument of 0 means
; there is no actual data to be written out but allocations should be done.
;==============================================================================
WriteOFS	movea.l	a0,a2			save buffer pointer
		movea.l	a1,a3			and file handle data pointer
		move.l	d0,d2			save amount to be written

; if an error occurs we have to rewind to the original position
		move.l	rwd_Offset(a3),rwd_SavedOffset(a3)

		cmp.l	LargestWrite(a5),d2
		ble.s	5$
		move.l	LargestWrite(a5),d2	set practical limit on write size

; check that it's OK to write to the disk (bitmap could be badly scrozzled too)
5$		bsr	DiskProtected		is this OK
		move.l	d0,d4			save error code
		beq.s	10$			no error, write's OK
		moveq.l	#0,d3			no data written
		bra	WriteOFSExit		exit now

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
		move.l	d3,d0			move protect info
		moveq.l	#0,d3			total written so far = 0
		tst.l	d4			was there an error ?
		bne	WriteOFSExit		yes, exit now

		move.l	#ERROR_WRITE_PROTECTED,d4 assume write protected
		btst.l	#2,d0			is write allowed ?
		bne	WriteOFSExit		no, so return the error

		moveq.l	#0,d4			current error = FALSE
		bsr	AlterRoot		things are changing
		clr.w	rwd_NewBlocks(a3)	no new blocks allocated yet
		moveq.l	#TRUE,d0		assume we're in the file
		move.l	d0,rwd_InFile(a3)	updated at the end

; This is the main write loop that continues while there is data left and the
; error condition is false.
		tst.l	d2			anything left to write
		ble	WriteOFSDone		nope, exit now
WriteOFSLoop	tst.l	d4			any errors
		bne	WriteOFSDone		yes, exit now

; grab the current extension block header so we can find the current key
		move.l	rwd_Header(a3),d0
		bne.s	WriteGotHeader		there is a header there

; A previous Write ended on an extension block boundary.  Rather than getting
; all blocks at once (as for the FFS write routine) we will just allocate a
; single extension block (because we want the data right after it).  We'll
; defer to the main loop to fill in that extension with allocated disk blocks.
		move.l	rwd_LastData(a3),d0	block we want to be near
		moveq.l	#1,d1			we only want one block
		bsr	AllocKeys		see how many we get
		move.l	d0,rwd_Header(a3)	this is the new current header
		bne.s	10$			no errors, we got at least one

; The disk is full up or broken, report the error and abort this write
		move.l	d1,d4			error code returned in d1
		bra	WriteOFSDone		clean up and exit

; Get the previous header block and link our new extension block to it
; FIX! should write new header first!
10$		move.l	rwd_PrevHeader(a3),d0	fetch previous header
		bsr	GrabBlock
****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a0
		lea.l	cb_SIZEOF(a0),a1
		adda.l	BlockSize(a5),a1	add next extension key pointer
		move.l	rwd_Header(a3),vfhb_Extension(a1)
		move.l	rwd_PrevHeader(a3),d0	write to this disk key
		bsr	WriteSummedBlockA0Later_RWD  D0,A0 with cb_header

		bsr	GetBlock		fetch a cache buffer
		movea.l	d0,a4
	clr.l	cb_Key(a4)
		lea.l	cb_SIZEOF(a4),a0
		bsr	ClearBlock		clear it out
		lea.l	cb_SIZEOF(a4),a0
		move.l	#t.list,fhb_Type(a0)	it's an extension
		move.l	rwd_Header(a3),fhb_OwnKey(a0)	on this block
		adda.l	BlockSize(a5),a0
		move.l	rwd_FileHeader(a3),vfhb_Parent(a0) owning header
		move.l	#st.file,vfhb_SecType(a0)	  secondary type
		movea.l	a4,a0			write out this extension
		move.l	rwd_Header(a3),d0	to this disk key
		bsr	WriteSummedBlockA0Later_RWD  D0,A0 with cb_header
		move.l	rwd_Header(a3),d0	refetch header key number

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
		moveq.l	#TRUE,d0
		move.w	d0,rwd_NewBlocks(a3)	will be writing to new blocks
		move.l	d2,d1
		add.l	BytesPerData(a5),d1	round up to nearest block
		subq.l	#1,d1

; must avoid divu.w overflow at 65536 blocks!  Since that's about 16MB,
; we can loop subtracting 65536*BytesPerData until it's in divu.w range.
		move.l	BytesPerData(a5),d0
		swap	d0
		clr.w	d0			*65536
		move.w	d0,-(sp)		count of # of 65536*488's
5$		cmp.l	d0,d1
		bcs.s	7$			size < 488*65536, divide rest
		sub.l	d0,d1			size = size-65536*488
		addq.w	#1,(sp)
		bra.s	5$

7$		move.w	(sp)+,d0		# of 65536's to add to count
		divu.w	BytesPerData+2(a5),d1	d1 holds # data blocks required
		swap	d1
		move.w	d0,d1
		swap	d1			put it in the high word
		move.l	HTSize(a5),d0		see how may fit in extension
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
		bra	WriteOFSDone		clean up and exit

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

; I'm actually doing a write here just to set things up properly.  However,
; I think it will be OK to set cb_State to write and allow the LooseBlock
; call that's done later to cause this block to be written out. cb_Key
; should already be set (since we read the block in the first place).
		movea.l	a4,a0			write this buffer
		move.l	rwd_Header(a3),d0	to this block
		bsr	WriteSummedBlockA0Later_RWD  D0,A0 with cb_header
		move.l	rwd_Header(a3),d0	grab block straight back
		bsr	GrabBlockA4
****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a4
		move.l	cb_SIZEOF(a4,d6.w),d1	refetch starting block number

; we have a data key to write to (in d1) check all the mask and transfer stuff
WriteGotKey	move.l	rwd_Offset(a3),d0	get offset into current block
		divu.w	BytesPerData+2(a5),d0	figure out offset into block
		swap	d0			remainder is current offset
		ext.l	d0			using a long later

; Enters here with d0=offset into block, d1=block number, d2=amount to write
; d6 = current offset in hash table.
WriteOFSCache	move.l	BytesPerData(a5),d5	calculate bytes to transfer
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
; but we'll have to initialise the header portion of the data block first.
		tst.w	rwd_NewBlocks(a3)	any need to read first
		beq.s	20$			yes, there's old data there too
		move.l	rwd_LastData(a3),d0	fetch last data block
		cmp.l	rwd_FileHeader(a3),d0	if file header do FDB
		bne.s	14$
; this is the first data block in the file.  Mark the file header with this.
		clr.l	rwd_DataSeq(a3)		dataseq will be bumped to 1
		bsr	GrabBlock
;		movea.l	d0,a0
		move.l	(sp),cb_SIZEOF+fhb_FirstBlock(a0)
		moveq.l	#TRUE,d1		it's a file header
		bra.s	16$

14$		bsr	GrabDataBlock		update it to point at new data
		movea.l	d0,a0
		move.l	(sp),cb_SIZEOF+fdb_NextBlock(a0)
**** Bug fix for seqnum being out of order.  No longer try to track SeqNum
**** for every seek and read operation, just fetch it from the previous header
**** at all times.  Fileheader is a special case with a SeqNum of 0.
		move.l	cb_SIZEOF+fdb_SeqNum(a0),rwd_DataSeq(a3)
		moveq.l	#FALSE,d1		it's a data block
16$		move.l	rwd_LastData(a3),d0
		move.l	rwd_FileHeader(a3),cb_Header(a0)
		bsr	WriteBlockNow		write this block back out

15$		bsr	GetBlock		fetch a new cache buffer
		movea.l	d0,a4			need to initialise data block
		move.l	#t.data,cb_SIZEOF+fdb_Type(a4)
		move.l	rwd_FileHeader(a3),cb_SIZEOF+fdb_OwnKey(a4)
		addq.l	#1,rwd_DataSeq(a3)	bump data sequence number
		move.l	rwd_DataSeq(a3),cb_SIZEOF+fdb_SeqNum(a4)
		clr.l	cb_SIZEOF+fdb_DataSize(a4)	amount of data here
		clr.l	cb_SIZEOF+fdb_NextBlock(a4)	no next block
		move.l	(sp),rwd_LastData(a3)	last data block allocated
		bra.s	30$			and fill it

20$		move.l	(sp),d0			fetch data block number
		bsr	GrabDataBlock		and read it
****** need to check for an error condition if the header is garbage ******

; copy the relevant portion of the buffer (using d6=offset and d7=length)
30$		movea.l	d0,a4
		lea.l	cb_SIZEOF+fdb_Data(a4,d6.w),a1	a1 = destination pointer
		cmpa.w	#0,a2			is there any data ?
		beq.s	35$			nope
		movea.l	a2,a0			a0 = source data pointer
		move.l	d7,d0			d0 = length to copy
		jsr	_LVOCopyMem(a6)
**** bug fix, when seeking back and re-writing must make sure size is OK ****
35$		move.w	d6,d0			get current offset into block
		ext.l	d0
		add.l	d7,d0
		cmp.l	cb_SIZEOF+fdb_DataSize(a4),d0	update space used
		ble.s	36$			current size is larger
		move.l	d0,cb_SIZEOF+fdb_DataSize(a4)  update new size
**** end of bug fix
36$		movea.l	a4,a0			write out this block
		move.l	(sp)+,d0		to this disk key
		moveq.l	#FALSE,d1		it's a data block
		move.l	rwd_FileHeader(a3),cb_Header(a0)  belongs to this file
		bsr	WriteBlockNow		put it on Pending queue

; This iteration has finished.  Update the current position and amount of
; data transferred and loop back to see if there is anything else to do.
WUpdateStats	add.l	d7,rwd_Offset(a3)	current position in file
		tst.l	d4			was there an error ?
		beq.s	10$			nope
		bsr	SetPosition		yes, make sure position is OK
10$		cmpa.w	#0,a2			was there a buffer ?
		beq.s	11$			nope
		adda.l	d7,a2			next position in buffer
11$		add.l	d7,d3			update total amount written
		sub.l	d7,d2			amount left to do
		bgt	WriteOFSLoop

; This write request has finished.  Update the fileheader with the new size
; if it has changed.  We wait for the close packet before doing DateStamping
; and archive bit resetting.
; Remember, unsigned file positions!
WriteOFSDone	moveq.l	#TRUE,d0
		move.w	d0,rwd_Altered(a3)	file has been modified
		move.l	rwd_Offset(a3),d0	fetch current byte offset
		cmp.l	rwd_CurrSize(a3),d0	have we extended the file ?
		bls.s	WriteOFSExit		nope, we are still <EOF
		move.l	rwd_FileHeader(a3),d0	yes, need to update size
		bsr	GrabBlock
****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a0
		lea.l	cb_SIZEOF(a0),a1	point to actual buffer data
		adda.l	BlockSize(a5),a1	reference from end of block
		move.l	rwd_Offset(a3),vfhb_ByteSize(a1)  set the new size
		move.l	rwd_FileHeader(a3),d0	write back the header
		bsr	WriteSummedBlockA0Later_RWD  D0,A0 with cb_header
		clr.l	rwd_InFile(a3)		not in the file anymore
WriteOFSExit	move.l	d4,d1			error code to d1
		bne.s	10$			there was an error
		move.l	d3,d0			amount written to d0
		move.l	rwd_InFile(a3),d1	no error, return infile flag
		rts

10$		move.l	rwd_SavedOffset(a3),rwd_Offset(a3)
		bsr	SetPosition
		moveq.l	#-1,d0
		rts

;==============================================================================
; position/error = SeekOFS(offset,mode,rwdata)
;    d0	    d1		     d0    d1    a1
;
; Seeks to the given position in the file.  Mode determines what the seek is
; relative to as follows:-
;	-1 = OFFSET_BEGINNING
;	 0 = OFFSET_CURRENT
;	 1 = OFFSET_END
; If the resultant seek position is before BOF or after EOF then SEEK_ERROR
;==============================================================================
SeekOFS		movea.l	a1,a3			save rwd stuff
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

; bug fix, don't try to do a seek if the file is empty.  Gets DataSeq screwed.
		tst.l	d4			worth seeking ?
		beq.s	SeekComplete		nope, must be at beginning

		move.l	rwd_Offset(a3),d4	save current position
		move.l	d2,rwd_Offset(a3)	and set new position
		bsr	SetPosition		set it to this

; Seek has completed OK, return FALSE in d1 and the seek position in d0
SeekComplete	moveq.l	#FALSE,d1		no error to report
		move.l	d4,d0			return seek position
		rts

; Something was wrong with the seek position.  Return a generic SEEK_ERROR.
SeekError	move.l	#ERROR_SEEK_ERROR,d1	return an error
		moveq.l	#-1,d0			Indicate failure
SeekExit	rts


;==============================================================================
; size/error = SetSizeOFS(offset,mode,rwdata)
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
SetSizeOFS	movea.l	a1,a3			save rwd stuff
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
MakeBigger	move.l	rwd_Offset(a3),-(sp)	save current position
		move.l	d4,rwd_Offset(a3)	seek to end of file
		bsr	SetPosition
		sub.l	d4,d2			d2=amount to be written
		move.l	d2,d0			d0=amount
		suba.l	a0,a0			no buffer
		movea.l	a3,a1			set up rwdata
		movem.l	a2/a3,-(sp)		save regs for WriteFFS
		bsr	WriteOFS		append space to end of file
		movem.l	(sp)+,a2/a3
		move.l	rwd_Offset(a3),d0	get new size (should be bigger)
		move.l	(sp)+,rwd_Offset(a3)	restore old seek position
		bra.s	SetPosition		will return current d0/d1

; We're making the file smaller.  This is a lot more complicated because we
; have to check that we're not going to leave another reader/writer hanging
; in space after the end of the file.  We limit the truncation to the 
; largest seek position other than our own (which will be adjusted to suit)
MakeSmaller	bsr	AlterRoot
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
; FIX!!! file sizes should be UNsigned!!!!
20$		movea.l	(a0),a0			chain to next rwd_Data
		bra.s	10$			and keep looking

; we can now safely set the file to the (maybe modified) size in d2.
SizeModified	move.l	d2,rwd_Offset(a3)	seek us to the new position
		bsr.s	SetPosition

; Now free all data keys and extensions from the current position onwards
		move.l	d2,d0			get current offset
		divu.w	BytesPerData+2(a5),d0	convert to block number
		swap	d0
		move.w	d0,-(sp)		save remainder
		swap	d0
		ext.l	d0
		divu.w	HTSize+2(a5),d0		get extension sequence number
		swap	d0			remainder is blocks in extension
		ext.l	d0
		move.l	d0,d1			clear from this block...
		move.w	(sp)+,d0		...if offset into block is 0
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
;		  a3
; A special internal routine for rwofs.asm, makes sure that all the state
; variables are set up correctly for the current offset.  Saves all registers.
; We have to make sure that rwd_PrevData is correct so that when we write
; new data blocks out they get chained to the correct data blocks.  The only
; time this is really important is if the seek is to end of file (otherwise
; subsequent writes to the existing data block will fix up LastData accordingly)
;==============================================================================
SetPosition	movem.l	d0-d2/a0-a1,-(sp)
		move.l	rwd_Offset(a3),d0	get current position
	printf <'SetPos(%ld) -- '>,d0
		divu.w	BytesPerData+2(a5),d0	d0 holds block number
		ext.l	d0
;		move.l	d0,rwd_DataSeq(a3)	save as data sequence number
;		addq.l	#1,rwd_DataSeq(a3)	data sequence starts at 1
		divu.w	HTSize+2(a5),d0		d2 holds extension #
		move.w	d0,-(sp)		save it for later
		swap	d0			get the hashtable entry number
		lsl.w	#2,d0			convert to a real offset...
		move.w	MaxEntry(a5),d1		...into a disk block
		sub.w	d0,d1
		move.w	d1,rwd_Entry(a3)	and save that
		move.w	(sp)+,d2		get back the sequence number
		cmp.w	rwd_Sequence(a3),d2	do we need to rewind
		bge.s	PosFindHeader		nope, we're <= current position
		move.l	rwd_FileHeader(a3),rwd_Header(a3)
		clr.l	rwd_PrevHeader(a3)	rewind to beginning
		clr.w	rwd_Sequence(a3)

; loop around until we have read the correct extension block for this seek
PosFindHeader	cmp.w	rwd_Sequence(a3),d2	have we found it
		beq.s	PositionSet		yes, variables are set up
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

; to make sure rwd_LastData get's set correctly, we ensure that the current
; hashtable entry we are pointing at contains a valid key.  If it does then
; a subsequent read or write will use that block to set up rwd_LastData with
; its value.  If, however, our current entry is zero, then we should set
; rwd_LastData to the key number of the previous data block.  Watch out for
; file size of zero (easy to detect if rwd_PrevHeader is NULL).
PositionSet	move.l	rwd_Header(a3),d0	fetch current header
		beq.s	NeedPrevHeader2		there is none (not allocated)
		bsr	GrabBlock
;		movea.l	d0,a0
		move.w	rwd_Entry(a3),d0	fetch current offset
		tst.l	cb_SIZEOF(a0,d0.w)	will we allocate a new block?
		bne.s	SetLater		nope, rwd_LastData set later
		addq.w	#4,d0			bump to previous entry
		cmp.w	MaxEntry(a5),d0		gone too far ?
		bgt.s	NeedPrevHeader		yes, need previous header
		move.l	cb_SIZEOF(a0,d0.w),rwd_LastData(a3)
SetLater	bsr	LooseHeaderBlock
		bra.s	LastDataOK

; We need the previous header (because we're positioned at the beginning of
; an empty extension block or file header).
NeedPrevHeader	bsr	LooseHeaderBlock
NeedPrevHeader2	move.l	rwd_PrevHeader(a3),d0	get previous header
		beq.s	FileHeaderIsIt		there is none, empty file
		bsr	GrabBlock
;		movea.l	d0,a0			store previous data key
		move.l	cb_SIZEOF+fhb_HashTable(a0),rwd_LastData(a3)
		bsr	LooseHeaderBlock
		bra.s	LastDataOK

; we seeked to the beginning and end of the file at the same time (its empty)
FileHeaderIsIt	move.l	rwd_FileHeader(a3),rwd_LastData(a3)
LastDataOK	movem.l	(sp)+,d0-d2/a0-a1
		rts

		END

