		SECTION	filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/io.i"
		INCLUDE	"exec/lists.i"

		INCLUDE	"globals.i"
		INCLUDE	"private.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XREF	WaitCo,ResumeCo,CallCo,FreeBuffer
		XREF	EnQueue,DeQueue,FindHash,AddHash,UnHash,SendTimer
		XREF	ClearWaiting

		XDEF	GetBlock,FreeBlock,ReadBlock,Fetch,WriteBlock,WaitFor
		XDEF	QueueBehind

;==============================================================================
; buffer = WaitForPend(buffer)
;   d0			a0
;
; Sticks a buffer onto the pending queue and then waits for it to be ready.
; The buffer MUST already be on the hash table if it is required there.
; Must be able to handle the buffer being returned being different than
; the buffer passed in!  (see WriteBlock)
;==============================================================================
WaitForPend	movea.l	a0,a1			buffer to a1 for ADDTAIL
		move.w	#HASHPENDING,cb_QueueType(a1)	change the type
		lea.l	PendingQueue(a5),a0	queue to append to
		ADDTAIL				append to the pending queue
		movea.l	a1,a0			need buffer again
; ***** drop through to WaitFor to wait for the buffer to be processed *****
;==============================================================================
; buffer = WaitFor(buffer)
;   d0		     a0
;
; Waits for a buffer on the pending queue to complete its action and then
; returns the buffer to the caller.  Doesn't save the owning co-routine!!
; Must be able to handle the buffer being returned being different than
; the buffer passed in!  (see WriteBlock)
;==============================================================================
WaitFor		move.l	CurrentCo(a5),cb_CoPkt(a0)	we own it now!!
		bra.s	Fetch			tell us when it's ready

;==============================================================================
; QueueBehind( buffer )
;		 a0
;
; Waits for a buffer when another co-routine is also waiting on it.  Really
; weird this one, but it works.  We don't actually get possession of the buffer
; but we do get to know when the other waiting co-routine finishes with it.
; Careful with the buffer, it may change (see WaitFor).
;==============================================================================
QueueBehind	move.l	cb_CoPkt(a0),-(sp)	save the current waiter
		bsr.s	WaitFor			wait for it ourselves
		movea.l	d0,a1			WaitFor returned d0=buffer
		movea.l	(sp)+,a0		get back previous co-routine
		move.l	a0,cb_CoPkt(a1)		just to be tidy
		bra	CallCo			call the other waiter (d0=buf)

;==============================================================================
; Buffer = Fetch()
;   d0
;
; Starts up the disk co-routine or waitco's if it is already running.  The
; cache buffer we want fetched will have our cobase in it so disk will call
; us when the buffer has been read.  Caller of this routine must have put the
; buffer onto the pending queue for this to work.  Disk will put buffer in d0.
; Remember: the whole world will have changed when we get back from this.
;==============================================================================
Fetch		tst.w	DiskRunning(a5)		is disk co-routine going?
		bne	WaitCo			yes, so wait back to parent
		movea.l	DiskCo(a5),a0		disk isn't running
		bra	ResumeCo		so start it up

;==============================================================================
; buffer = GetBlock()
;   d0
;
; The main cache buffer allocation routine.  Handles the following things:-
; 1.	Write pending blocks on waiting queue should not hog buffers.
; 2.	Data blocks are less useful than file headers and directories.
; 3.	Blocks in the pending queue must not hog buffers.
;==============================================================================
GetBlock
;	IFD DEBUG_CODE
;		XREF	CountBuffers
;		bsr	CountBuffers
;		printf <'%ld buffers'>,d0
;	ENDC
		move.l	FreeBufferQueue(a5),d0	any free buffers available
		beq.s	FirstPending		no, try the next thing
;	printf <'Got Free buffer $%lx'>,d0
		movea.l	d0,a0			yes, unlink from the list
		move.l	(a0),FreeBufferQueue(a5)
		clr.l	cb_Header(a0)		not associated with anything
		rts				and return buffer in d0

; Second, see if a block would be available if we waited for the first pending
; block to be written out.
; The rationale is (I think) that if something is using the disk, we might
; as well wait since we're not going to be able to read anything until
; it's finished anyways.  However, I'm not sure how well this holds up
; in the case of allocating blocks (creating files, writing, etc where we
; don't have to read the block).  Perhaps we should have some sort of "why
; do we need this block" flag to know if we want to wait on the pending
; queue or try to steal valid blocks first.			REJ
FirstPending	lea.l	PendingQueue(a5),a0	anything on the queue
		TSTLIST	a0
		beq.s	WaitData		nope, so cant do anything here
		movea.l	(a0),a0			yes, fetch first node
		tst.l	cb_CoPkt(a0)		anyone waiting for this block
		bne.s	WaitData		yes, so forget it.
		tst.l	cb_Size(a0)		is it block sized ?
		bne.s	WaitData		nope, so we can't use it!
; it used to grab any block, now it only does this to a data block
; if it's not a data block, we probably will want to keep it (most recently
; used, after all).  Look for something else first.
;		bgt.s	WaitData		nope, so we can't use it!
		tst.w	cb_QueueType(a0)	has this been invalidated?
		beq.s	WaitData		yes, WriteBlock killed it
;	printf <'waiting for head pending buffer $%lx, type %ld, %ld'>,a0,cb_Size(a0),cb_Key(a0)
		clr.l	cb_Header(a0)
		bra	WaitFor			yep, wait for this buffer

; New addition because bitmaps et al are very important and should hang around
; on the waiting queue for as long as possible.  First, try to steal any data
; block off the waiting queue (consider this option 2.5).
;FindWaitData	tst.b	DiskType+3(a5)		only do this for FFS
;		beq.s	WaitData		nope, do OFS thing
;		move.l	WaitingQueue(a5),d0
;10$		movea.l	d0,a1
;		move.l	(a1),d0			lookahead to next node
;		beq.s	StealData
;		tst.l	cb_Size(a1)		is it a data block ?
;		bne.s	10$			nope, look for the next
;		move.l	a1,d0			save cache buffer pointer
;		REMOVE				remove from WaitingQueue
;		movea.l	d0,a0
;		bra	WaitForPend		add to pending and wait

; Third, try the waiting queue and if there is more than one block with data
; waiting to be written then write the first one out. (Only data blocks).  This
; stops us stealing a data buffer that is in the process of being filled a
; little bit at a time.  If there's two the least recently used one will be
; near the head of the list and we'll find that one first.
;
; actually only hits of the _first_ block on the waiting queue is a data block!
; and it doesn't check if there's a second data block, just a second block
; - REJ
WaitData	lea.l	WaitingQueue(a5),a0	anything on the queue ?
		TSTLIST	a0
		beq.s	StealData		nope, so that won't work
		movea.l	MLH_HEAD(a0),a1		fetch the first node
		tst.l	cb_Size(a1)		is it a data block ?
		bne.s	StealData		no, so don't bother
		movea.l	LN_SUCC(a1),a1		lookahead to next node
		tst.l	LN_SUCC(a1)		is it the end of the list ?
		beq.s	StealData		yes, only one buffer waiting
		REMHEAD				remove the first cache buffer
		movea.l	d0,a0			returns d0 = buffer
		clr.l	cb_Header(a0)
;	printf <'moved to pending buffer $%lx, type %ld, %ld'>,a0,cb_Size(a0),cb_Key(a0)
		bra	WaitForPend		and make it pending

; Third point five, try to steal data blocks off the valid queue.  This
; should fix any thrashing problems when writing disks in oldfs format.  I've
; disabled this check for FFS format because it's pretty expensive to scan the
; whole valid queue on larger disks (with more buffers) every time we need a
; new cache buffer.  Larger disks tend to be FFS format, not oldFS.
;
; This means that data blocks on the valid queue rarely remain there! 
; Basically, data blocks on valid get rolled over instantly for anything else.
; This has major negative impacts - basically data blocks aren't cached - REJ
StealData
STEAL_DATA	EQU	1
	IFD STEAL_DATA
		lea.l	ValidBufferQueue(a5),a0	don't try if no valid buffers
		TSTLIST	a0
		beq.s	StealWaiting		nothing on valid queue!
; the FFS test had been commented out by steve...  wonder why - we'll try it
		btst.b	#0,DiskType+3(a5)
		bne.s	StealValid		only try this for oldfs disks
		move.l	(a0),d0			fetch the first entry
10$		movea.l	d0,a1			node pointer to a1
		move.l	(a1),d0			look ahead to next node
		beq.s	StealValid		end of list, steal anything
		tst.l	cb_Size(a1)		is this a data block
		bne.s	10$			nope, look for the next
		move.l	a1,d0			save cache buffer pointer
		REMOVE				remove from Valid Queue
		movea.l	d0,a0			also remove from hash table
		clr.l	cb_Header(a0)
;	printf <'grabbed from valid buffer $%lx, type %ld, %ld'>,a0,cb_Size(a0),cb_Key(a0)
		bra	UnHash			returns d0=block
	ENDC

; Fourth, ANY damn block from the valid queue will do, direct buffs not on here.
;
; modified - don't steal last valid block here.  Instead, try to find a waiting
; block to punt over into pending.  We'll hit the last valid block later.  This
; _should_ help with ping-pong dircache thrashing.  - REJ
StealValid	lea.l	ValidBufferQueue(a5),a0	already set up
	IFND STEAL_DATA
		TSTLIST	a0			needed if we remove stealdata
		beq.s	StealWaiting
	ENDC
		move.l	MLH_HEAD(a0),a1		first entry
		move.l	LN_SUCC(a1),a1		second entry (if there)
		tst.l	LN_SUCC(a1)		is it really there?
		beq.s	StealWaiting		no, try waiting queue first
		REMHEAD				remove first valid buffer
		movea.l	d0,a0			also unlink from hash table
		clr.l	cb_Header(a0)
;	printf <'grabbed from valid buffer (any) $%lx, type %ld, %ld'>,a0,cb_Size(a0),cb_Key(a0)
		bra	UnHash			rts with d0=buffer

; Fifth, least recently used block on waiting queue.  This will take some time!
; No need to worry about large transfers here, they always go on pending queue.
;
; Since with dircache we have a tendency to thrash if there's only one entry
; in the valid queue, I think we'll throw a second buffer into the pending
; queue (if available).  This will avoid simple ping-pong effects.  3-item
; circles will still thrash, but those are far less common.  REJ
;
; Hmm.  Let's try ClearWaiting here.  Move ALL waiting blocks to the pending
; queue if we have no valid blocks.
StealWaiting	lea.l	WaitingQueue(a5),a0	anything on this list ?
		TSTLIST	a0
		beq.s	StealLastValid		no
		bsr	ClearWaiting		move all waiting to pending...
		bra.s	AnyBlock		wait on first free pending blk

; 5.5, grab last valid block in case we passed it up already (should only
; happen if there is one valid block and no waiting blocks)  - Added, REJ
StealLastValid	lea.l	ValidBufferQueue(a5),a0	already set up
		TSTLIST	a0
		beq.s	AnyBlock		empty
		REMTAIL				remove only valid buffer
		movea.l	d0,a0			also unlink from hash table
		clr.l	cb_Header(a0)
;	printf <'grabbed from valid buffer (last) $%lx, type %ld, %ld'>,a0,cb_Size(a0),cb_Key(a0)
		bra	UnHash			rts with d0=buffer

; Sixth, try any other block on the pending queue which is data block sized
; and is not being waited for by another co-routine.  This will take ages!
; The first entry may have been looked at earlier.
AnyBlock	move.l	PendingQueue+MLH_HEAD(a5),d0	fetch the first node
10$		movea.l	d0,a0			node pointer to a0
		move.l	LN_SUCC(a0),d0		look ahead to next node
		beq.s	AnyPending		end of list
		tst.l	cb_Size(a0)		is it data block sized
		bgt.s	10$			nope, don't bother
		tst.l	cb_CoPkt(a0)		anyone else waiting for it
		bne.s	10$			yep, we can't have it
		tst.w	cb_QueueType(a0)	is this buffer invalid?
		beq.s	10$			can only be the first buffer
		clr.l	cb_Header(a0)
;	printf <'wait for pend (any) buffer $%lx, type %ld, %ld'>,a0,cb_Size(a0),cb_Key(a0)
		bra	WaitFor			wait for the buffer then rts

; PANIC! PANIC! PANIC!  Any block sized buffer will do when it's free.  All we
; do now is wait for the first entry on the pending queue to be completed and
; then we go all the way through the whole routine again in the hope that some
; cache buffer got freed up in the process (though we could fail again too).
AnyPending	movea.l	PendingQueue(a5),a0	queue behind the first buffer
		tst.w	cb_QueueType(a0)	has this been invalidated?
		bne.s	10$			no
		move.l	(a0),a0			yes, wait on second pending
		; yes, there must be another pending entry, or WriteBlock
		; couldn't have invalidated the first entry.  There's nothing
		; (block-sized) on valid or waiting, and all PendingQueue
		; entries (except the first) already have waiters.
10$	printf <'wait for pend (behind) buffer $%lx, type %ld, %ld'>,a0,cb_Size(a0),cb_Key(a0)
		bsr	QueueBehind
		bra	GetBlock		do the whole thing again

; Got a block to be used in d0.  Send it back to the caller
GotBlock	rts

;==============================================================================
; FreeBlock( buffer, state )
;	       a0      d0
;
; Takes a buffer and put's it back into the appropriate queue. State is type of
; buffer we are dealing with.  -1 (TRUE) means a valid buffer that can be used
; for subsequent reads, 0 (FALSE) means an invalid buffer that can be freed.
; Possible queueing options are :
;	QUEUE IS			WHEN
;	========			====
;	WaitingQueue	=	state=TRUE  AND cb_State=CMD_WRITE
;	ValidQueue	=	state=TRUE  AND cb_State=CMD_READ
;	FreeQueue	=	state=FALSE AND cb_State=?
;
; Entries for any queue have thier cb_QueueType set up appropriately and they
; are also added into the current hash table.  Read buffers which have an error
; condition set are scrapped and thrown on the free queue regardless of the
; given state value (this allows other code to do a LooseBlock call even when
; an error occured).
;==============================================================================
FreeBlock	cmpa.w	#0,a0
		beq.s	6$			nothing to do, had an error
;	printf <'Freeing buffer 0x%lx, state %ld\n'>,a0,d0
		tst.w	cb_Error+2(a0)		did we get an error ?
		bne.s	5$			yes, trash this block
		tst.l	d0			is this to be freed ?
		bne.s	10$			nope, valid or waiting
5$		clr.w	cb_QueueType(a0)	yes, put on the free queue
		move.l	FreeBufferQueue(a5),(a0)  link at head of free queue
		move.l	a0,FreeBufferQueue(a5)	no need to append it!!!
6$		rts

10$		bsr	AddHash
		movea.l	a0,a1			returns a0=buffer
		lea.l	ValidBufferQueue(a5),a0	assume it's a valid buffer
		move.w	#HASHVALID,cb_QueueType(a1)
		move.w	cb_State(a1),d1		get current command for buffer
		cmpi.w	#CMD_READ,d1		was it a read
		beq.s	30$			yes, so it's valid now
		lea.l	WaitingQueue(a5),a0	not valid so must be waiting
		move.w	#HASHWAITING,cb_QueueType(a1)
30$		ADDTAIL
		rts

;==============================================================================
; buffer = ReadBlock(key,snatch,type)
;  d0		     d0	   d1    d2
;
; Reads a block and returns a pointer to the cache buffer containing this block
; Type refers to the block as looked at by Disk (-2,-1,0 or >0) and has the
; following meanings :-
;
;	-2	bitmap, read one block to cache buffer and checksum (but the
;		checksum goes in a different place to normal file headers).
;		This read is done in a special way too.  Since bitmap allocation
;		used to be an atomic operation we should keep the new bitmap
;		schemes working the same way.  This means the read scheme
;		must be modified to stop other operations happening before
;		the block is read from disk.
; 	-1	file header so read one block to cache buffer and checksum it.
;	 0	data block, read one block to cache buffer.  If running under
;		FFS format then don't checksum it.  If OFS then do checksum.
;	>0	direct transfer, read this many bytes to given buffer (cb_Buff)
;		and don't checksum it.  Only FFS mode uses this value.
;
; Snatch really only refers to blocks being written and allows us to snatch
; blocks off the waiting queue to examine them before they are written out.
; Snatch=TRUE means snatch it immediately else wait for it to write out first.
;
; One quick search of the cache is done using a hashing algorithm, if the block
; is not found then we must really read it from disk.  If it is found then we
; use the cb_QueueType field to determine what to do with the snagged buffer.
;==============================================================================
ReadBlock	movem.l	d3-d4,-(sp)
	printf <'Reading block %ld, snatch %ld, type %ld\n'>,d0,d1,d2
		move.l	d0,d3			stash the key
		move.l	d1,d4			and the snatch flag

; First search the hash table to see if we have this block hanging around.
ReReadBlock	move.l	d3,d0			see if the block's in a buffer
		bsr	FindHash
		movea.l	d0,a1
		move.l	a1,d0			did we find it
		beq.s	ReallyRead		nope, must read it from disk

; we found a buffer, if it belongs on the valid queue snatch it right now
		cmpi.w	#HASHVALID,cb_QueueType(a1)
		bne.s	5$			not on the valid queue
		REMOVE				take from valid
		movea.l	d0,a0
		bsr	UnHash			and out of the hash table
		bra	BlockRead		buffer in d0

; next see if the block is about to be written out and either wait or snatch it
5$		cmpi.w	#HASHWAITING,cb_QueueType(a1)
		bne.s	10$			not on the waiting queue
		REMOVE				remove from waiting queue
		movea.l	d0,a0

; we got the buffer but can we snatch it straight away or must we wait ?
		tst.l	d4			test snatch
		beq.s	6$			we must wait for it
		bsr	UnHash			snatch, remove from hash table
		bra	BlockRead		and return d0 = buffer
6$		bsr	WaitForPend		so put it on pending and wait
		bra	BlockRead		d0=buffer

; The block must exist on the pending queue because it wouldn't have been in
; the hash table if it wasn't block sized or waiting to be written/read
; we found the block, check if another co-routine is already waiting for it
10$		tst.l	cb_CoPkt(a1)		any owning co-routine ?
		beq.s	20$			nobody waiting for this block
		movea.l	a1,a0
		bsr	QueueBehind		wait until other is complete
		bra	ReReadBlock		go search again (must be there)

; no-one is waiting for this block, see if we can snatch it now.  If it's the
; very first entry on the Pending queue then we can't touch it because it's
; currently in use by the Disk co-routine.  If we're not snatching then we'll
; wait for it to complete it's operation and come off the pending queue first.
20$		tst.l	d4			snatch true ?
		beq.s	30$			nope, just wait for it
		cmpa.l	PendingQueue(a5),a1	can't snatch the first one
		beq.s	30$			because it's in use NOW!
		REMOVE				remove from pending queue
		movea.l	d0,a0			unlink from hash table too
		bsr	UnHash
		bra.s	BlockRead		buffer in d0

; either snatch was false or this buffer is in use right now, so wait for it
30$		movea.l	a1,a0
		bsr	WaitFor			wait for this block
		bra.s	BlockRead		block in d0

; The block wasn't in memory or pending so we have to really read the thing.
; First check for an instantly available buffer and read into that if OK.
ReallyRead	move.l	FreeBufferQueue(a5),d0	any free buffers ?
		beq.s	10$			nope, doing it the hard way
		movea.l	d0,a1			yep, unlink from free queue
		move.l	(a1),FreeBufferQueue(a5)
		bra.s	DoTheRead		and read into the buffer

; There were no buffers instantly available so wait for one.  Since another
; co-routine may be waiting here to read the same block: when we get a free
; buffer, we put it back on the free buffer queue and search the queues again
; in case the block we want was read in while we were waiting.
10$		bsr	GetBlock
		movea.l	d0,a0
		moveq.l	#0,d0			put back on FreeBufferQueue
		bsr	FreeBlock
		bra	ReReadBlock		and do everything again

; we have a buffer to read into so start everything up and wait for the block
DoTheRead	moveq.l	#cb_SIZEOF,d0		point to data part of cache buf
		add.l	a1,d0
		move.l	d0,cb_Buff(a1)		this is where we read to
		move.l	d2,cb_Size(a1)		fill in type (-2,-1,0 or >0)
		move.l	d3,cb_Key(a1)		the key we want to read
		move.w	#CMD_READ,cb_State(a1)	what we want to do
		move.l	CurrentCo(a5),cb_CoPkt(a1)  where to resumeco to
		move.w	#HASHPENDING,cb_QueueType(a1)
		lea.l	PendingQueue(a5),a0	attach to pending queue
		ADDTAIL				other reads get queued
		movea.l	a1,a0
		bsr	AddHash			add into hash table too (a0=buf)
		bsr	Fetch			start disk or wait (buf to d0)
BlockRead	movea.l	d0,a0			don't associate with anything
		clr.l	cb_Header(a0)
		movem.l	(sp)+,d3-d4
		rts

;==============================================================================
; WriteBlock( buffer, key, status, type )
;		a0     d0    d1     d2
;
; Stuffs a block into the waiting or pending queues ready to be written later.
; if status=TRUE then it will go on WaitingQueue else PendingQueue.  Type is
; the same as used in ReadBlock (-2,-1,0 or >0).  We first look for the buffer
; on any of the queues (it shouldn't be on any other than the valid queue) and
; discard it if it's found since we're going to write over it anyway.
;
; Update: it _can_ be on the pending queue from a previous write!  Worse yet,
; it can be at the head of the pending queue!  Solution: if not at the head
; of the pending queue, steal it's WaitFor's/QueueBehind's, free it, and put
; ourself on the pending queue.  If at the head, don't free it but set a bit
; that will make Disk put it on the free queue.  (Actually we UnHash it so
; no one else can find it, and set it's queuetype to FREE.  Disk will notice
; the queuetype and put it in the free queue without unhashing.)
;
; This depends on people calling QueueBehind/WaitFor to use the buffer ptr
; returned by them, since it now can change while waiting!
;==============================================================================
WriteBlock	movem.l	d3-d4,-(sp)
	printf <'writing buffer 0x%lx, block %ld, status %ld, type %ld\n'>,a0,d0,d1,d2
;	printf <'w:$%lx b %ld s %ld t %ld\n'>,a0,d0,d1,d2
		move.l	d0,d3			stash the key
		move.l	d1,d4			and status

		; need to init here so we can hijack CoPkt waiters later
		clr.l	cb_CoPkt(a0)		no co-routine owns this

		move.l	a0,-(sp)		and the buffer
		bsr	FindHash		find key in d0
		movea.l	d0,a1
		move.l	a1,d0
		beq.s	10$			didn't find the buffer
;	IFD DEBUG_CODE
;		printf <'killing block %ld, state = $%lx @$%lx'>,cb_Key(a1),cb_QueueType(a1),a1
;		cmp.w	#HASHVALID,cb_QueueType(a1)
;		beq.s	1$			as expected
;		printf <'	What the ####??????'>
;1$
;	ENDC
		cmp.w	#HASHPENDING,cb_QueueType(a1)
		bne.s	5$
		; it's on the pending queue.  Nasty.
		; first, hijack it's waiters if any.
		; the new block must go on the pending queue!
		moveq	#0,d4			this MUST go on pending!
		move.l	(sp),a0			the new buffer
		move.l	cb_CoPkt(a1),cb_CoPkt(a0)  steal the waiters
		clr.l	cb_CoPkt(a1)
		move.l	a1,a0			UnHash/FreeBuffer need it in a0
		cmp.l	PendingQueue(a5),a1	is it first?
		beq.s	7$			yup unhash it and set q type
		; no, remove it and add to the free queue as well

5$		bsr	FreeBuffer		Unhashes and frees
		bra.s	10$

7$		bsr	UnHash			remove from hash table too
		movea.l	d0,a0
		clr.w	cb_QueueType(a0)	type free

10$		move.l	(sp)+,d0		get back the buffer
		movea.l	d0,a1
		add.l	#cb_SIZEOF,d0		point to data part of cache buf
		move.l	d0,cb_Buff(a1)		this is where we write from
		move.l	d2,cb_Size(a1)		fill in type (-2,-1,0 or >0)
		move.l	d3,cb_Key(a1)		the key we want to write
		move.w	#CMD_WRITE,cb_State(a1)	what we want to do
		lea.l	PendingQueue(a5),a0	assume the pending queue
		moveq.l	#HASHPENDING,d0
		tst.l	d4
		beq.s	WritePending		it was the pending queue

		lea.l	WaitingQueue(a5),a0	nope, it was the waiting queue
		moveq.l	#HASHWAITING,d0
		tst.w	TimerRunning(a5)	if timer running
		bne.s	WritePending		then we don't need to send
		movem.l	d0/a0-a1,-(sp)		send timer request to ensure
		bsr	SendTimer		this gets flushed later
		movem.l	(sp)+,d0/a0-a1

WritePending	move.w	d0,cb_QueueType(a1)	set correct queue type
		ADDTAIL				add to appropriate queue
		movea.l	a1,a0
		bsr	AddHash			put into hash table too
		movem.l	(sp)+,d3-d4
		rts

		END

