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

;		DEBUGENABLE

		LIST

		XDEF	AllocKeys,FreeKeys,InitBitmap,WriteMap,AlterRoot
		XDEF	FlushBitmapBlock,AllocListKey

		XREF	WriteBlockNow,_LVODoIO
		XREF	GrabBlock,LooseBlock,DatStamp,GrabDataBlock,Checksum

		XREF	GetPubMem,_LVOFreeMem,DirectRead,DirectDiscard
		XREF	WaitFor,QueueBehind,DoDirectWrite,DoDirectRead

;==============================================================================
; key = AllocListKey(nearkey,validating)
; d0		       d0	 d1.w
;
; Attempts to allocate a disk block as near as possible to nearkey.  It
; is important to remember that this new block allocation scheme will read and
; write the disk using the cache buffer routines.  For this reason it is NOT
; possible to hold onto cache buffers across any AllocKeys calls.  Requires the
; list of bitmap keys to be already set up.  Will return ERROR_NO_BITMAP if
; this is not the case.  Returns the starting key number of disk
; block allocated.  If key==0 then d1 will hold an error code instead.
;
; This is called from InsertList, so we can allocate blocks after the bitmap
; is built but before we allow the world to get to it.  If not validating
; we just use AllocKeys.
;==============================================================================
AllocListKey	move.l	d1,-(sp)
		moveq	#1,d1			allocate 1 block
		tst.l	(sp)+			is this a validator alloc?
		beq.s	AllocKeys		no, handle normally

		; bitmap must be valid or it can't try to allocate!
	printf <'Block allocated during validate!'>
		movem.l	d2-d7/a2-a3,-(sp)
		bra.s	AllocCommon

;==============================================================================
; key/count = AllocKeys(nearkey,keycount)
; d0	d1		   d0	   d1
;
; Attempts to allocate keycount disk blocks as near as possible to nearkey.  It
; is important to remember that this new block allocation scheme will read and
; write the disk using the cache buffer routines.  For this reason it is NOT
; possible to hold onto cache buffers across any AllocKeys calls.  Requires the
; list of bitmap keys to be already set up.  Will return ERROR_NO_BITMAP if
; this is not the case.  Returns the starting key number and the count of disk
; blocks allocated.  If key==0 then count will hold an error code instead.
;==============================================================================
AllocKeys	movem.l	d2-d7/a2-a3,-(sp)
		tst.w	BitmapValid(a5)		do we have a bitmap ?
		bne.s	AllocCommon		yes
		moveq.l	#FALSE,d0		no, so error return
		move.l	#ERROR_DISK_NOT_VALIDATED,d1	could be validating
		bra	AllocKeysDone

; don't try to allocate more keys than are available on the disk
AllocCommon	cmp.l	BlocksFree(a5),d1
		ble.s	20$			OK, we have enough
		move.l	BlocksFree(a5),d1	not enough, truncate amount
		bne.s	20$			but there are some left
		moveq.l	#FALSE,d0		no space left, return error
		move.l	#ERROR_DISK_FULL,d1
		bra	AllocKeysDone

; We have the (possibly modified) number of keys required and a key to start
; the search on.  Find the bitmap block, lword and bit numbers for this key.
; New addition, searching from the given NearKey can take a hell of a long
; time if we just filled a hole in the bitmap and the disk is large.  For
; this reason, we always save the next key to search from after an allocation
; succeeds.  If any keys are freed then we clear this value and use the given
; search key instead.
20$		tst.l	StartSearch(a5)		is startsearch valid ?
		beq.s	21$			nope, use given startkey
		move.l	StartSearch(a5),d0	yes, start searching here
21$		cmp.l	HighestBlock(a5),d0	possible to be out of bounds
		ble.s	25$
		move.l	Reserved(a5),d0		wrap to beginning of disk

25$		move.l	d0,d4			save start key for search
		move.l	d1,d5			and number of keys required
		sub.l	Reserved(a5),d0		reserved blocks not in bitmap
		divu.w	BlocksPerBM(a5),d0	calculate bitmap pointer offset
		lsl.w	#2,d0			accessing longword array
		movea.l	BitmapKeys(a5),a2	point to bitmap key array
		adda.w	d0,a2			now (a2) is the correct bmkey
		swap	d0			get remainder
		moveq.l	#31,d3			calculate bit number....
		and.w	d0,d3			...for the start key
		lsr.w	#3,d0			and calculate longword number
		andi.w	#$fffc,d0		but use a byte offset
		addq.w	#4,d0			skip checksum in bitmap
		move.w	d0,d2			d2 holds byte offset to lword
		ext.l	d2			but whole lword compared later
		moveq.l	#0,d6			number of keys allocated
		moveq.l	#32,d7			frequently used value

; First we have to find the start key.  Registers used throughout the rest of
; this routine are as follows:-
;
;	d2 = byte offset to the current longword
;	d3 = bit number within the current longword
;	d4 = the key we are currently looking at
;	d5 = the number of blocks required
;	d6 = the number of blocks allocated so far
;	d7 = 32 (used in loops for faster longword addition)
;
;	a2 = pointer to the current bitmap key array entry
;	a3 = current bitmap block we are accessing
;
		move.l	(a2),d0			fetch the current bitmap block
		bsr	GrabBitmapBlock		don't care if it's writing
****** need to check for an error condition if the header is garbage ******
		movea.l	d0,a3
		move.l	a3,d0
		beq	AllocFailure		returns d1=error

		tst.w	d3			are we on a longword boundary ?
		beq.s	FastSearch		yes, so do the fast search

; special case the first longword so that we quickly get aligned on a boundary
; this code is also used to find the free block when we know a longword has one
		move.l	cb_SIZEOF(a3,d2.w),d0	fetch first longword
SlowSearch	btst.l	d3,d0			is this key free ?
		bne.s	GotFirstKey		yep, we have a start
		addq.l	#1,d4			bump the key number
		cmp.l	HighestBlock(a5),d4	are we in range ?
		ble.s	SlowContinue		yes, check bit number

; we've gone past end of disk, wrap back to the beginning (nothing written yet)
RewindSearch:
;		movea.l	BitmapKeys(a5),a0	do we need a whole new block ?
;		cmpa.l	(a2),a0			880K floppies only use one
;		beq.s	SameBlock		nope, stay on the same block
;		movea.l	a3,a0
;		moveq.l	#0,d0			not associated with anything
;		bsr	LooseBitmapBlock	free up current block
		movea.l	BitmapKeys(a5),a2	point back to beginning
		move.l	(a2),d0
		bsr	GrabBitmapBlock		and grab first bitmap block
		tst.l	d0			did we get it ?
		beq	AllocFailure		nope, returns nothing and d1=error

		movea.l	d0,a3
SameBlock	move.l	Reserved(a5),d4		looking at first data block
		moveq.l	#4,d2			therefore first longword...
		moveq.l	#0,d3			...and first bit
		bra.s	FastSearch		so we can search quickly

; still within the range of the disk, see if we are still in the same lword
SlowContinue	addq.w	#1,d3			bump the bit number
		andi.w	#31,d3			mask to 32 bit span
		bne.s	SlowSearch		still in first longword
FastContinue	addq.w	#4,d2			bump to the next longword
		cmp.l	BlockSize(a5),d2	are we still in same block
		blt.s	FastSearch		yep, search fast
;		movea.l	a3,a0			nope, so...
;		moveq.l	#0,d0			not associated with anything
;		bsr	LooseBitmapBlock	...loose current bitmap block
		addq.l	#4,a2			and get the next one
		move.l	(a2),d0			it's guaranteed to be in...
		bsr	GrabBitmapBlock		...range (tested d4 already)
		tst.l	d0
		beq.s	AllocFailure		something went wrong
		movea.l	d0,a3
		moveq.l	#4,d2			back at beginning of block

; we've moved into the next longword and are aligned to do fast 32 bit tests
FastSearch	move.l	cb_SIZEOF(a3,d2.w),d0	fetch next longword
		bne.s	SlowSearch		something free in there
		add.l	d7,d4			going to advance 32 blocks
		cmp.l	HighestBlock(a5),d4	are we in range ?
		ble.s	FastContinue		yes, get next longword
		bra.s	RewindSearch		no, go back to beginning

; We've found a free key.  Registers are the same as for the searching loop
; with the addition of d0 holding the current bitmap longword.  Once again,
; we special case the first longword to quickly get on a longword boundary.
GotFirstKey	move.l	HighestBlock(a5),d1	truncate allocation to...
		sub.l	d4,d1			...end of partition
		addq.l	#1,d1			count includes highest block
		cmp.l	d5,d1			saves highestblock test...
		bge.s	FindNextKey		...in main allocation loop
		move.l	d1,d5

; Due to the little fix above, we only have to watch for running off a bitmap
; block or running out of keys to allocate.  It nicely takes the partition
; boundary check out of the main loop.  We can't allocate beyond the end of
; the partition because wrapping to the beginning would not be contiguous.
FindNextKey	bclr.l	d3,d0			allocate this bit
		beq.s	AllocDone		already allocated, exit now
		addq.l	#1,d6			bump allocated count
		subq.l	#1,d5			and check if more needed
		beq.s	AllocDone		nope, we did them all
		addq.w	#1,d3			bump to next bit
		andi.w	#31,d3
		bne.s	FindNextKey		still in same longword

; We've overflowed to the next longword.  Store this one back into the bitmap
; block and start allocating 32 blocks at a time (while it's possible).
AllocReEnter	move.l	d0,cb_SIZEOF(a3,d2.w)	store back bitmap longword
		addq.w	#4,d2			bump to next longword
		cmp.l	BlockSize(a5),d2	still in same block ?
		blt.s	GetFastKeys		yep, go real quick
		bra.s	WriteBMBlock		nope, write this one out first

; completely finished, make sure d5 contains 0 as a flag for WriteBMBlock so
; it doesn't attempt to continue allocating using the fast allocation code.
AllocDone	moveq.l	#0,d5			no more blocks to allocate
		move.l	d0,cb_SIZEOF(a3,d2.w)	flush saved longword

; Nasty edge condition, we ran off a block just as we were about to start
; searching quickly.  Write out this bitmap block and fetch the next one
; ready for the top of the fast loop.  This code is re-used by the fast
; loop when it too runs off the end of a bitmap block and also by AllocDone
; (via GotFirstKey) so we don't duplicate too much code.
WriteBMBlock	move.w	#-1,cb_State(a3)	will get flushed later
		addq.l	#4,a2			bump BM key pointer
		tst.l	d5			any work left ?
		beq.s	AllocEnd		nope, called from AllocDone
		move.l	(a2),d0			yes, read in next bitmap block
		bsr	GrabBitmapBlock
		tst.l	d0
		beq.s	AllocFailure		something went wrong
		movea.l	d0,a3
		moveq.l	#4,d2			back at the first longword

; We've allocated the fiddly bits.  While there's more than 32 blocks required
; and there are 32 free keys in the current longword keep allocating them.  If
; 32 or less keys are required or the current longword is not completely empty
; then we do the fiddly bits again because it handles the ending case better.
GetFastKeys	move.l	cb_SIZEOF(a3,d2.w),d0	get next longword
		cmp.l	d7,d5			more than 32 blocks needed ?
		ble.s	FindNextKey		nope, do slow allocations
		cmpi.l	#-1,d0			are there 32 contiguous blocks
		bne.s	FindNextKey		no, do slow allocations
		sub.l	d7,d5			yes, allow for them
		add.l	d7,d6			32 more allocated
		moveq.l	#0,d0			actually allocate them
		bra.s	AllocReEnter		and move to next group

AllocFailure	moveq.l	#0,d0			no start key (d1=error)
		clr.l	StartSearch(a5)
		bra.s	AllocKeysDone
		
; everything completed, return the start key in d0 and the keycount in d1
AllocEnd	bsr	AlterRoot		safety
		move.l	d4,d0			start key
		move.l	d6,d1			keycount
		sub.l	d6,BlocksFree(a5)	adjust total remaining
		add.l	d6,d4			next place to search from
		move.l	d4,StartSearch(a5)

AllocKeysDone	movem.l	(sp)+,d2-d7/a2-a3
		rts

;==============================================================================
; count/Error = FreeKeys(startkey,keycount)
;  d0	 d1		    d0	     d1
;
; Frees up the given range of bitmap keys.  Does not check for running off the
; end of the partition because allocations are never returned that way.  Can
; return read/write errors or ERROR_NO_BITMAP.  Error code is only valid if
; the returned count is less than the requested keycount.
;==============================================================================
FreeKeys	movem.l	d2-d6/a2-a3,-(sp)
	printf <'FreeKeys(%ld,%ld)\n'>,d0,d1
		tst.w	BitmapValid(a5)		do we have a bitmap ?
		bne.s	10$			yes
		move.l	#ERROR_NO_BITMAP,d0	no, return an error
		bra	FreeKeysDone

; Find the bitmap block, longword and bit numbers for the given start key.
10$		clr.l	StartSearch(a5)		search where caller says
		move.l	d1,d4			save number of keys to be freed
		sub.l	Reserved(a5),d0		reserved blocks not in bitmap
		divu.w	BlocksPerBM(a5),d0	calculate bitmap pointer offset
		lsl.w	#2,d0			accessing longword array
		movea.l	BitmapKeys(a5),a2	point to bitmap key array
		adda.w	d0,a2			now (a2) is the correct bmkey
		swap	d0			get remainder
		moveq.l	#31,d3			calculate bit number....
		and.w	d0,d3			...for the start key
		lsr.w	#3,d0			and calculate longword number
		andi.w	#$fffc,d0		but use a byte offset
		addq.w	#4,d0			skip checksum in bitmap
		move.w	d0,d2			d2 holds byte offset to lword
		ext.l	d2			but whole lword compared later
		moveq.l	#32,d5			frequently used value
		moveq.l	#0,d6			number of keys freed

; We are ready to start freeing keys.  Register usage is as follows:-
;	d2 = byte offset to the current longword
;	d3 = bit number within the current longword
;	d4 = the number of keys left to free
;	d5 = 32 (used in loops for faster longword addition)
;	d6 = accrued total of keys freed
;	a2 = pointer to the current bitmap key array entry
;	a3 = current bitmap block we are accessing

		move.l	(a2),d0			fetch the current bitmap block
		bsr	GrabBitmapBlock		don't care if it's writing
		tst.l	d0
		beq.s	FreeKeysDone		something went wrong
		movea.l	d0,a3
		tst.w	d3			are we on a longword boundary ?
		beq.s	FastFree		yes, so do the fast freeing

		move.l	cb_SIZEOF(a3,d2.w),d0	no, fetch the first longword
FreeNextKey	bset.l	d3,d0			free this bit
		addq.l	#1,d6			freed one more
		subq.l	#1,d4			any more to be freed ?
		beq.s	FreeDone		nope, we did them all
		addq.w	#1,d3			bump to next bit
		andi.w	#31,d3
		bne.s	FreeNextKey		still in same longword

; We've overflowed to the next longword.  Store this one back into the bitmap
; block and start freeing 32 blocks at a time (while it's possible).
FreeReEnter	move.l	d0,cb_SIZEOF(a3,d2.w)	store back bitmap longword
		addq.w	#4,d2			bump to next longword
		cmp.l	BlockSize(a5),d2	still in same block ?
		blt.s	FastFree		yep, go real quick
		bra.s	FreeWriteBM		nope, write this one out first

; completely finished, flush the last longword and write out the bitmap block
FreeDone	move.l	d0,cb_SIZEOF(a3,d2.w)	flush saved longword

FreeWriteBM:
		move.w	#-1,cb_State(a3)	mark as dirty
		addq.l	#4,a2			and bump BM key pointer
		tst.l	d4			any work left ?
		beq.s	FreeKeyEnd		nope, called from FreeDone
		move.l	(a2),d0			yes, read next bitmap block
		bsr	GrabBitmapBlock
		tst.l	d0
		beq.s	FreeKeysDone		will return FALSE
		movea.l	d0,a3
		moveq.l	#4,d2			back at the first longword

; We've freed the fiddly bits.  While there's more than 32 blocks need freeing
; and there are 32 allocated in the current longword keep freeing them.  If 32
; or less keys need freeing or the current longword is not completely full
; then we do the fiddly bits again because it handles the ending case better.
FastFree	move.l	cb_SIZEOF(a3,d2.w),d0	get next longword
		cmp.l	d5,d4			more than 31 blocks to free?
		blt.s	FreeNextKey		nope, do bitwise freeing
		moveq.l	#-1,d0			free all 32 bits
		add.l	d5,d6			accrued total freed
		sub.l	d5,d4			32 less need freeing
		beq.s	FreeDone		no more to do
		bra.s	FreeReEnter		not finished yet

; everything complete, return number of keys freed in d0 (works out as TRUE)
FreeKeyEnd	bsr	AlterRoot		safety
		move.l	d6,d0			return keys freed
		add.l	d6,BlocksFree(a5)	add to the free pool
	
FreeKeysDone	movem.l	(sp)+,d2-d6/a2-a3
		rts

;==============================================================================
; Error = InitBitmap()
;  d0
;
; Actually part of the Restart() routine but left here because it's intimately
; tied in with the bitmap stuff.  If the bitmap flag in the root is valid then
; this routine reads all of the bitmap keys and fills the BitMapKeys array
; with thier block numbers.  All of the checksums are verified and the total
; number of blocks free is calculated.  Requires basic disk geometry to be
; already set up (HighestBlock, Reserved, BlockSize and RootBlock etc.)
;==============================================================================
InitBitmap	movem.l	d2-d5/a2-a3,-(sp)
		move.l	RootKey(a5),d0		fetch the root
		bsr	DirectRead
****** FIX!need to check for an error condition if the header is garbage ******
		movea.l	d0,a2
		move.l	a2,d0
		beq.s	10$			error, no buffer

; check to see if the dirlist type is the one we support.  If not, we must
; force a validate, and have validate blank the entries for us, and let them
; ALL rebuild.
		btst.b	#2,DiskType+3(a5)	is this DCFS?
		beq.s	3$
		lea.l	cb_SIZEOF(a2),a3	point to end of block
		adda.l	BlockSize(a5),a3	for accessing bitmap stuff
		move.l	vrb_DirList(a3),d2	fetch the root dircache block
		movea.l	a2,a0			drop root
		bsr	DirectDiscard
		move.l	d2,d0			dircache block
		bsr	DirectRead
		movea.l	d0,a0
		move.l	a0,d0
		beq.s	10$			error, no buffer
		move.l	cb_SIZEOF+fhl_Type(a0),d2	save dircache type
		bsr	DirectDiscard		(a0)
		move.l	RootKey(a5),d0		fetch the root (again)
		bsr	DirectRead
		movea.l	d0,a2
		move.l	a2,d0
		beq.s	10$			error, no buffer
;	printf <'root dirlist type is %ld'>,d2
		cmp.l	#t.dirlist,d2		is this the exact right type?
		beq.s	3$			yes, check bitmap valid bit

		; invalidate bitmap - we MUST do this here, in case the 
		; machine is reset during the validate, but after the root
		; block's dircache block is updated...

		lea.l	cb_SIZEOF(a2),a3	point to end of block
		move.l	a3,a0			save pointer to data
		adda.l	BlockSize(a5),a3	for accessing bitmap stuff
		clr.l	vrb_BitmapFlag(a3)	bitmap not valid!
		bsr	Checksum		we changed the checksum...
		sub.l	d0,fhb_Checksum+cb_SIZEOF(a2)	and update it...
		move.l	RootKey(a5),d0		write it back to root
		move.l	a2,a0			buffer to write
		bsr	DoDirectWrite
		; we still own the buffer, and it's now marked invalid...
		; we can fall through into the bitmap valid test.

3$		; check bitmap valid flag
		lea.l	cb_SIZEOF(a2),a3	point to end of block
		adda.l	BlockSize(a5),a3	for accessing bitmap stuff
		tst.l	vrb_BitmapFlag(a3)	is the bitmap valid ?
		bne.s	have_bitmap		yes, so get all the keys

; the bitmap is not validated but is probably present (though not always)
		movea.l	a2,a0
		bsr	DirectDiscard		done with root
10$		move.l	#ERROR_DISK_NOT_VALIDATED,d0
		bra	InitBitmapDone		return error to restart

; OK, the root says we have a valid bitmap, we're going to believe it's true.
; We have an array for the bitmap keys, now fill it up using the root entries
; and any extender blocks that are present.  If any of them are not allocated
; then we return an error and treat things as though there's no bitmap.
have_bitmap	lea.l	vrb_Bitmap(a3),a1	a1 points to entries in root
		movea.l	BitmapKeys(a5),a3	a3 points at bitmap key array
		moveq.l	#ROOT_BM_COUNT,d0	# of bitmap entries in the root
		move.w	BitmapCount(a5),d2	get total count
		bra.s	FillEnter

FillLoop	move.l	(a1)+,(a3)+		save next key
		beq.s	FillError		something wrong
		subq.w	#1,d2			one less block to do
		beq.s	FillDone		all finished
FillEnter	dbra	d0,FillLoop		until block is finished
		move.l	(a1)+,d3		save the next key pointer
		movea.l	a2,a0			and finish with this block
		bsr	DirectDiscard
		move.l	d3,d0			fetch next extender block
		bsr	DirectRead		it's not a header
		movea.l	d0,a2
		move.l	a2,d0
		beq.s	FillError		no buffer
		tst.l	cb_Error(a2)
		bne.s	FillError		something went wrong
		lea.l	cb_SIZEOF(a2),a1	point to first entry
		move.l	BlockSize(a5),d0	calculate maximum entries...
		lsr.l	#2,d0			...in an extender block
		subq.l	#1,d0			allow for extender block key
		bra.s	FillEnter		and fill from this area now

; something prevented us from reading in all the bitmap keys, return an error
FillError	printf <'FillError block %ld'>,cb_Key(a2)
		movea.l	a2,a0
		bsr	DirectDiscard		done with current block
		move.l	#ERROR_DISK_NOT_VALIDATED,d0
		bra	InitBitmapDone		return error to init

; We now have a complete list of bitmap keys.  Read in each bitmap block and
; check it for consistency and use it to calculate how many blocks are free.
FillDone	movea.l	a2,a0			finished with this block
		bsr	DirectDiscard

	printf <'FillDone'>
		movea.l	BitmapKeys(a5),a3	point at bitmap key array
		move.l	HighestBlock(a5),d4	assume all blocks free
		sub.l	Reserved(a5),d4		d4 holds total blocks available
		move.l	d4,d3			number of blocks left to check
		moveq.l	#-1,d2			for quick comparisions

BitmapLoop	move.l	(a3)+,d0		fetch next bitmap block
		bsr	DirectRead
		movea.l	d0,a2
		move.l	a2,d0
		beq.s	FillError		no buffer
		tst.l	cb_Error(a2)
		bne.s	FillError
		lea.l	cb_SIZEOF(a2),a0
		bsr	Checksum		see if checksum is OK
		tst.l	d0
		bne.s	FillError		discard block and exit

; Bitmap checksum was OK.  Count how many blocks have been used in it.
		lea.l	cb_SIZEOF+4(a2),a0	point at bitmap data
		move.w	BlocksPerBM(a5),d0	assume checking the whole block
		ext.l	d0
		cmp.l	d0,d3			can we do that ?
		bge.s	WholeBlock		yes
		move.l	d3,d0			no, truncate to amount left over
WholeBlock	lsr.l	#5,d0			d0 holds # longwords to check
		bra.s	CountEnter		start the loop

CountLoop	move.l	(a0)+,d1		get next set of keys
		bne.s	10$			not all allocated
		subi.l	#32,d4			all keys are allocated
		bra.s	CountEnter
10$		cmp.l	d1,d2			are all blocks free ?
		beq.s	CountEnter		yes, nothing to subtract

; we have a mixture of allocated and free blocks in this longword so we have
; to count the individual bits to determine how many blocks have been used
		moveq.l	#31,d5			checking 32 bits
11$		btst.l	d5,d1			is this one allocated ?
		bne.s	12$			nope
		subq.l	#1,d4			yep
12$		dbra	d5,11$			check next bit
CountEnter	dbra	d0,CountLoop		check next longword

; we've finished with the current block.  Free it and go back for the next one
		move.w	BlocksPerBM(a5),d0
		ext.l	d0
		sub.l	d0,d3			did we do a whole block
		ble.s	PartialWord		no, we're on the last one
		movea.l	a2,a0			yes, free the current block
		bsr	DirectDiscard
		bra.s	BitmapLoop		and go for the next one

; We have counted the very last block of the bitmap, check if we need to do
; a partial longword too.
PartialWord	add.l	d0,d3			fix because d3 <= 0
		andi.w	#31,d3			get remaining bits
		beq.s	AllCounted		none left, don't fetch extra
		move.l	(a0)+,d1		fetch the next longword
		moveq.l	#0,d5			start at bit 0
		bra.s	LastEnter

LastLoop	btst.l	d5,d1			is this one allocated
		bne.s	10$			nope
		subq.l	#1,d4			yes
10$		addq.l	#1,d5			move to next bit
LastEnter	dbra	d3,LastLoop		and keep checking

; Finally finished counting those damn blocks. Store the amount of free ones
AllCounted	movea.l	a2,a0			finished with bitmap block
		bsr	DirectDiscard
		move.l	d4,BlocksFree(a5)	save amount of free blocks
		moveq.l	#FALSE,d0		no error to report
		move.w	#TRUE,BitmapValid(a5)	bitmap is useable
InitBitmapDone	movem.l	(sp)+,d2-d5/a2-a3
		rts

;==============================================================================
; WriteMap()
;
; Called during Flush() or when the motor timeout reaches zero.  Doesn't write
; the bitmap to disk anymore because that's done on the fly now.  Just updates
; the rootblock's bitmap valid flag.
;==============================================================================
WriteMap	tst.w	FilesOpen(a5)
		beq.s	10$			no files were opened
		clr.w	FilesOpen(a5)		flag no files open now
		bsr	FlushBitmapBlock	flush out bitmap
		moveq.l	#TRUE,d0		update bitmap flag in root
		bsr.s	WriteRoot
10$		rts

;==============================================================================
; AlterRoot()
;
; Called when a file is opened for output or something has been deleted or
; moved around.  Marks the BMValid flag in the root block as FALSE and writes
; the root block out to disk.  This ensures that we correctly flush things out
; when the three second timer goes off.  This is also a safeguard against a
; crash leaving the bitmap in an indeterminate state.  Validator will know to
; kick in when the volume is remounted with a FALSE BMValid flag.
;==============================================================================
AlterRoot	tst.w	FilesOpen(a5)		has this been done already ?
		bne.s	RootAltered		yes, exit immediately
		moveq.l	#FALSE,d0		write this to BMValid
		bsr.s	WriteRoot
		subq.w	#1,FilesOpen(a5)	converts it to TRUE
RootAltered	rts

;==============================================================================
; WriteRoot( BitmapFlag )
;		d0
;
; Reads in the root block, date stamps it and writes it back out with the given
; bitmap flag to indicate whether everything on disk is valid or not.  Waits
; for the root block to be on disk before actually returning.
;==============================================================================
WriteRoot	movem.l	d2/a2,-(sp)
		move.l	d0,d2			save bitmap flag
		move.l	RootKey(a5),d0		grab the root block
		bsr	GrabBlock
		movea.l	d0,a2			save the buffer address
		lea.l	cb_SIZEOF(a2),a0	point to end of disk block
		adda.l	BlockSize(a5),a0
		move.l	d2,vrb_BitmapFlag(a0)	update bitmap flag
		lea.l	vrb_DiskMod(a0),a0	date volume last modified
		bsr	DatStamp
		movea.l	a2,a0			this buffer
		move.l	RootKey(a5),d0		this key
		moveq.l	#TRUE,d1		checksummed header
		clr.l	cb_Header(a0)		doesn't belong to a header
		move.l	a0,-(sp)		save buffer address
		bsr	WriteBlockNow		write immediately
		movea.l	(sp)+,a0		get back buffer address

; What if someone is already in a WaitFor on the root?  We should QueueBehind!
		tst.l	cb_CoPkt(a0)
		bne.s	10$
		bsr	WaitFor			wait for it to write
		movea.l	d0,a0			done with buffer
		moveq.l	#0,d0			associate with no header
		bsr	LooseBlock
5$		movem.l	(sp)+,d2/a2
		rts

10$		; we must QueueBehind to know when the buffer is written
	printf <'Bitmap queue behind!!!'>
		bsr	QueueBehind		when it returns, it's written
		bra.s	5$			buffer isn't ours, exit

;==============================================================================
; buffer = GrabBitmapBlock(block)
;   d0			     d0
;
; Using new bitmap read/write schemes to separate bitmap blocks from the normal
; cache buffer routines.  This makes sure bitmap allocation operations are
; atomic (like the old in-memory versions).  Uses a reserved cache buffer.
;==============================================================================
GrabBitmapBlock	movea.l	BMBlock(a5),a0
	printf <'GrabBitmapBlock(%ld)\n'>,d0
		cmp.l	cb_Key(a0),d0		is this our block ?
		beq.s	GotBMBlock		yes, return pointer

; block not found, we`ll have to read it in from disk after all.  Check if we
; have a non-dirty buffer to use right away.
		move.l	d0,-(sp)		save required key
		bsr.s	FlushBitmapBlock	must flush it out first
		move.l	(sp)+,d0		restore required key

; physically read the bitmap block from disk.  May need extra error processing.
ReadBMBlock	movea.l	BMBlock(a5),a0
		bsr	DoDirectRead		read block into buffer
		; returns buffer in a0
		tst.l	cb_Error(a0)
		bne.s	BadBMBlock		some error occured

; run a checksum test on the bitmap block to make sure it`s OK
		move.l	a0,-(sp)
		lea.l	cb_SIZEOF(a0),a0
		bsr	Checksum
		move.l	(sp)+,a0		restore buffer ptr
		tst.l	d0			was checksum OK ?
		beq.s	GotBMBlock		yes

BadBMBlock	clr.l	cb_Key(a0)		so we can`t find it again
		move.l	cb_Error(a0),d1		return error in d1
		suba.l	a0,a0			return 0 for an error
GotBMBlock	move.l	a0,d0			return buffer in d0
		rts

;==============================================================================
; WriteBitmapBlock(key)
;
; this routine is handled in-line since it only needs the key and state setting
;==============================================================================

;==============================================================================
; LooseBitmapBlock(buffer)
;		     a0
;
; since there is only one bitmap buffer, this routine is a total nop.
;==============================================================================

;==============================================================================
; FlushBitmapBlock()
;
; Checksums the current bitmap block and flushes it out to disk
;==============================================================================
FlushBitmapBlock:
		movea.l	BMBlock(a5),a0
	printf <'FlushBitmapBlock() cb_Key = %ld, state = $%lx\n'>,cb_Key(a0),cb_State-2(a0)
		tst.w	cb_State(a0)		does it need writing?
		beq.s	30$			nope

		move.l	a0,-(sp)
		lea.l	cb_SIZEOF(a0),a0
		bsr	Checksum
		move.l	(sp)+,a0		restore buffer ptr
		sub.l	d0,cb_SIZEOF(a0)	fix checksum (1st longword)

		move.l	cb_Key(a0),d0		get the key to write
		bsr	DoDirectWrite		read block into buffer
		; returns buffer in a0
		tst.l	cb_Error(a0)
		beq.s	30$			got bitmap block OK
		clr.l	cb_Key(a0)		not valid now

30$		rts

		END
