		SECTION	filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/ports.i"
		INCLUDE "exec/libraries.i"
		INCLUDE	"private.i"
		INCLUDE	"globals.i"
		INCLUDE	"moredos.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XDEF	Validator,ValidatorError

		XREF	GetPubMem,WaitBlock,DiscardBlock,GetKeyAddr
		XREF	AlterRoot,WriteMap,_LVOFreeMem,ResumeCo,GrabBlock
		XREF	GrabBlockA3,GrabBlockA4
		XREF	GetBlock,ClearBlock,WriteBlockNow,ClearWaiting
		XREF	UpdateListIfDifferent,Hash,LooseBlock,NextListEntry
		XREF	WriteBlockLater,InsertListMissing

		XREF	BuildString,Request,FlushBitmapBlock
		XREF	CopyOverlap

;==============================================================================
; Validator()
;
; Creates a new bitmap from the files on the disk if the old one was invalid or
; could not be read properly.  This runs as a co-routine and sets BitmapValid
; TRUE if it finishes successfully.  No writes to disk can be performed until
; the validator has completed.
;
; The bitmap blocks themselves are allocated using a memory copy of the new
; bitmap.  It's not possible to use the normal allocation routines because
; there are no bitmap blocks on disk to allocate them from.
;
; To handle dos\4 and dos\5 (FastDir) types, we must first rebuild the
; bitmap blocks, then go through the entire tree again (though not extension
; blocks).  For each directory, we first empty the dirlist (freeing all but
; the first block), then we call InsertList for each entry in the directory.
; This can take a while, and causes much disk writing!  During this time,
; the root must never be written with bitmap valid (though internally we'll
; consider it to be).  We'll suppress the setting of the bit on the disk
; with a variable.
;
; Better yet!  On the second scan, there are two types of errors: mods made
; to the FH but not to the dirlist, and mods to the dir (or dirlist) not
; reflected in the dirlist (or dir).  The first can be handled by
; an UpdateListIfDifferent() call for each entry.  This would also add the
; entry if it didn't exist in the dirlist.  The rest can be handled by
; scanning the dirlist, and checking if each entry found is in the dir.
; We should do the latter first, since that will leave the blocks in the
; cache (hopefully), and then the first becomes part of the tree traversal
; (updatelistifdifferent(fhb), if (dir) visit(fhb)).  Yeah!  Slow, but
; greatly cuts down on writes during validation.
;
; Note that UpdateListIfDifferent might require allocating blocks!  We need
; separate block allocate entry from the normal one, which allows the List
; rtns to allocate even though the bitmap was not _marked_ as valid.
;
; After all this is done, we mark the bitmap as valid.  All dirlists are
; guaranteed to be correct.  If it dies anytime during this, all that happens
; is that we revalidate again.  However, if we die during a write of trackdisk,
; bad things can happen, of course (like loss of the whole track).  Therefore,
; floppy-only owners may not want to use it.  Of course, that danger has
; always been there for them with the bitmaps.
;
; We can never have problems with broken dirlists, since we write out the new
; dirlist block before the old one is modified (and we force it to disk).
; Therefore, we can scan the dirlists in the first pass, then delete them and
; recreate in the second.  DiskSalv or some such can handle a few extra sorts
; of errors, like a trashed dirlist but good fhb's (shouldn't happen unless
; we die during a physical write of a dirlist block).
;
; Uh, I'm not sure they go out in order....
;
; They don't....
;
; For disk-loaded FS's, never start a validate under exec 36.90 (ks 36.16).
; These are the Alpha 5 boot roms, and the system will reset RSN.
;==============================================================================
Validator	sub.l	a3,a3			so VExit won't try to free it
		cmp.w	#36,LIB_VERSION(a6)	Alpha 5 exec is 36.90
		bne.s	not_alpha_5
		cmp.w	#90,LIB_REVISION(a6)
		beq	ValidatorExit

not_alpha_5:	moveq.l	#31,d2
		add.l	HighestBlock(a5),d2	calculate memory requirements
		sub.l	Reserved(a5),d2		reserved blocks not in bitmap
		lsr.l	#5,d2			d2 = number of longwords for BM
		lsl.l	#2,d2			d2 = number of bytes for bitmap
	printf <'Validator needs %ld bytes for bitmap\n'>,d2

		move.l	d2,d0
		bsr	GetPubMem
		movea.l	d0,a3			a3 = physical bitmap
		move.l	a3,d0
		beq	ValidatorExit		can't validate
	printf <'Got physical map\n'>

		move.l	d2,d0
		bsr	GetPubMem
		movea.l	d0,a2			a2 = visited map
		move.l	a2,d0
		beq	ValidatorExit		can't validate
	printf <'Got VisitMap\n'>

; Mark the whole bitmap as being full of free blocks.  We allocate as we go.
		movea.l	a3,a0
		move.l	d2,d0			this many bytes
		lsr.l	#2,d0			this many longwords
		moveq.l	#-1,d1			storing this value
		bra.s	20$
10$		move.l	d1,(a0)+
20$		subq.l	#1,d0			600Mb disks overflow a word
		bge.s	10$
	printf <'Initialised in memory bitmap\n'>

; The visit map marks keys that we have to visit and is currently zeroed out by
; the GetPubMem call.  Set the bit corresponding to the root block so we have
; a place to start searching for keys to visit.  We also set up some registers
; for the main loop.  d2=current longword, d3=current bit, d4=current disk key.
; The visitmap is different to the bitmap in that we search on byte boundaries
; and set bits in individual bytes rather than the longwords used in the bitmap
		move.l	RootKey(a5),d4		our current key
		move.l	d4,d2
		moveq.l	#7,d3
		and.l	d2,d3			d3 = current bit in byte
		lsr.l	#3,d2			d2 = current byte
		bset.b	d3,0(a2,d2.l)		mark root key for visiting
		moveq.l	#1,d5			number of headers left to visit
		move.l	HighestBlock(a5),d7	number of blocks free
		sub.l	Reserved(a5),d7
		bra.s	BitSearch		search a bit at a time

; search the visited map until we find a set key which means this is a header
; that we must examine.  Once we find this bit, clear it so we don't visit it
; again and set all bits in the visited map associated with other headers
; referenced by this key.  Also set this key in the real bitmap.  If it's a
; data block then allocate all data keys associated with this header too.
MoreWork	tst.b	0(a2,d2.l)		any set bits in here ?
		bne.s	BitSearch		yep, do a bit search
		addq.l	#1,d2			move to next byte
		addq.l	#8,d4			bumping by 8 blocks
		cmp.l	HighestBlock(a5),d4	do we need to wrap ?
		ble.s	MoreWork

Wrap		moveq.l	#0,d2			byte number 0
		moveq.l	#0,d3			bit number 0
		moveq.l	#0,d4			Reserved ARE in VisitMap
		bra.s	MoreWork		can do byte test

BitSearch	btst.b	d3,0(a2,d2.l)
		bne.s	GotHeader		we found a header to visit
ReBitSearch	addq.l	#1,d4			update key number
		cmp.l	HighestBlock(a5),d4	make sure not time to wrap
		bgt.s	Wrap
		addq.w	#1,d3			and bit number
		andi.w	#7,d3
		bne.s	BitSearch		still more bits to test
		addq.l	#1,d2			move to next byte
		bra.s	MoreWork		go search by bytes now

; We found a header.  Clr this in the real bitmap and error out if it's already
; been cleared (key allocated twice).  Then, depending on the type, set all
; other header keys in the visitmap that are associated with this header.
GotHeader	printf <'examining header %ld\n'>,d4
		bclr.b	d3,0(a2,d2.l)		mark this one as visited
		subq.l	#1,d5			one less header to check
		move.l	d4,d0			find address in real bitmap
		sub.l	Reserved(a5),d0
		moveq.l	#31,d1
		and.w	d0,d1			d1=bit number
		lsr.l	#5,d0			d0=longword number
		lsl.l	#2,d0
		move.l	0(a3,d0.l),d6		clr this bit to mark allocated
		bclr.l	d1,d6
		bne.s	NowUsed

; Error, this key was already set which means we have something sharing disk
; blocks (which isn't allowed).  Put up a requester to inform user of this
; fact and exit the validator now.  Will need much slower special validate.
		move.l	d4,d0			this key
		lea.l	KeySet(pc),a0		this error message
		bsr	ValidatorError		display the requester
		bra	VExitFree

NowUsed		move.l	d6,0(a3,d0.l)
		subq.l	#1,d7			update blocks free
	printf <'Now %ld blocks free\n'>,d7
		move.l	d4,d0			go fetch this block
	printf <'Reading block %ld\n'>,d0
		bsr	WaitBlock
		movea.l	d0,a4			stash the buffer
		move.l	a4,d0
		beq	VExitFree		error
	printf <'error = %ld\n'>,cb_Error(a4)
		tst.l	cb_Error(a4)		any disk errors
		bne	CantVal			yes, error out

		movea.l	d0,a0			find what we are dealing with
		moveq.l	#DIRLIST_MASK,d0	mask out bits below dirlist
		and.l	cb_SIZEOF+fhl_Type(a0),d0
		cmp.l	#DIRLIST_KEY,d0
		bne.s	not_dirlist

; A dirlist block.  Add next block on chain to list to scan.
		cmp.l	cb_SIZEOF+fhl_OwnKey(a0),d4	is ownkey right?
		beq.s	30$
20$		lea.l	BadHeader(pc),a0	bad header type
		bsr	ValidatorError
		bra	CantVal

; add next dirlist block if any to scan
30$		move.l	cb_SIZEOF+fhl_NextBlock(a0),d0
		beq.s	40$			nothing on hashchain
		bsr	VisitThis		and mark pfor visiting
		tst.l	d0			was it already marked ?
		beq	CantVal			yes, so we have an err ^^
		addq.l	#1,d5			one more header to visit

; see if it's the write type of dirlist
40$		cmp.l	#t.dirlist,cb_SIZEOF+fhl_Type(a0)
		beq	HeaderDone		scrap this and go to next

; wrong type of dirlist!  remove all entries, change type!
	printf <'nuking dirlist block %ld!'>,d4
		move.l	#t.dirlist,cb_SIZEOF+fhl_Type(a0)
		clr.l	cb_SIZEOF+fhl_NumEntries(a0)
		move.l	a4,a0
		move.l	d4,d0
		moveq.l	#TRUE,d1
		bsr	WriteBlockLater
		bra	HeaderDoneNoBlock	we gave up the block already

not_dirlist	; a0 is buffer
		bsr	GetHdrType
		cmpi.w	#st.file,d0		is it a file ?
		beq	DoFile			yep
		cmpi.w	#st.userdir,d0		now check if its a directory
		beq.s	DoDir

; Do we have a link (hard or soft) so we only need to handle the hashchain
		cmpi.w	#ST_SOFT_LINK,d0	a soft link ?
		beq.s	10$
		cmpi.w	#ST_DLINK,d0		a directory link ?
		beq.s	10$
		cmpi.w	#ST_FLINK,d0		or a file link ?
		bne.s	20$

; yes, we have a link, just find out if anything is hanging off the hashchain
10$		lea.l	cb_SIZEOF(a4),a0	get to hashchain field
		adda.l	BlockSize(a5),a0
		move.l	vfhb_HashChain(a0),d0
		beq	HeaderDone		nothing on hashchain
		bsr	VisitThis		and mark for visiting
		tst.l	d0			was it already marked ?
		beq.s	CantVal			yes, so we have an errr
		addq.l	#1,d5			one more header to visit
		bra	HeaderDone		scrap this and go to next

; we have a bad header type (something got really corrupted) put up a requester
20$		lea.l	BadHeader(pc),a0	bad header type
		move.l	cb_Key(a4),d0
		bsr	ValidatorError

; the key we have now is not a type we expected so we can't validate the disk.
CantVal		movea.l	a4,a0
		bsr	DiscardBlock		throw the block away
	printf <'CantVal called\n'>
		bra	VExitFree		and exit now

; We found a directory.  Set all entries in the hashtable and any header that
; is linked onto the hashchain of this directory. Then continue the search.
; Also set the entries for the dirlist attached to it.
DoDir	printf <'...its a directory\n'>
		lea.l	cb_SIZEOF(a4),a0	get to hashchain field
		adda.l	BlockSize(a5),a0
		move.l	vfhb_HashChain(a0),d0
		beq.s	5$			nothing on the hashchain
		bsr	VisitThis
		tst.l	d0
		beq.s	CantVal			can't visit again
		addq.l	#1,d5			one more header to visit

5$		btst.b	#2,DiskType+3(a5)
		beq.s	10$			non-FastDir: dos\0-3
		lea.l	cb_SIZEOF(a4),a0	get to dirlist
		adda.l	BlockSize(a5),a0
		move.l	vudb_DirList(a0),d0
		beq.s	10$			no dirlist??!!
		bsr	VisitThis		put 1st dirlist entry on chain
		tst.l	d0
		beq.s	CantVal			can't visit again
		addq.l	#1,d5			one more header to visit

; Now scan hashtable of this directory and mark each entry for being visited.
10$		lea.l	cb_SIZEOF+udb_HashTable(a4),a0
		move.l	HTSize(a5),d1		number of entries in hashtable
		bra.s	30$			enter main loop
20$		move.l	(a0)+,d0		get next entry
		beq.s	30$			no header to be visited
		bsr	VisitThis		set this key in visit map
		tst.l	d0
		beq	CantVal			something went wrong ^^
		addq.l	#1,d5			one more header to visit
30$		dbra	d1,20$
		bra	HeaderDone		finished with this header

; We found a file header block or an extension block for a file.  We can treat
; these both the same.  If there is an entry on the HashChain then put this
; into the Visited map and do the same if there is an extension block.  Once
; this is done, we loop around and allocate all data keys referenced in this
; block.  They are not put into the visited map because we don't want to look
; at them, just allocate them.  (It may be useful to check data blocks for OFS)
DoFile	printf <'its a file\n'>
		lea.l	cb_SIZEOF(a4),a0	get to hashchain field
		adda.l	BlockSize(a5),a0
		move.l	vfhb_HashChain(a0),d0
		beq.s	10$			nothing on the hashchain
		bsr	VisitThis
		tst.l	d0
		beq	CantVal			can't visit again
		addq.l	#1,d5			one more header to visit

; see if there's anything in the extension field and mark that for visiting
10$		lea.l	cb_SIZEOF(a4),a0	get to extension field
		adda.l	BlockSize(a5),a0
		move.l	vfhb_Extension(a0),d0
		beq.s	20$			no extensions
		bsr	VisitThis
		tst.l	d0
		beq	CantVal			can't visit again
		addq.l	#1,d5			one more header to visit

; now loop around and mark any data blocks as allocated in the main bitmap
20$		movem.l	d2/a4,-(sp)		need a separate counter
		lea.l	cb_SIZEOF+fhb_HashTable(a4),a4	point to hash table
		move.l	HTSize(a5),d2		number of entries
		bra.s	40$			enter the loop
30$		move.l	(a4)+,d0		get the next key
		beq.s	40$			nothing here

; check that the data key we got was within the range of the partition.
		cmp.l	HighestBlock(a5),d0	make sure key in range
		bgt.s	2$			nope, error
		cmp.l	Reserved(a5),d0
		bge.s	3$
2$		lea.l	KeyRange(pc),a0		error, key out of range
		bsr	ValidatorError		so post the error
		bra.s	4$

; the data key is in range so allocate that bit in the physical bitmap
3$		sub.l	Reserved(a5),d0
		moveq.l	#31,d1
		and.l	d0,d1			d1=bit number
		lsr.l	#5,d0			d0=longword number
		lsl.l	#2,d0
		move.l	0(a3,d0.l),d6		clr this bit to mark allocated
		bclr.l	d1,d6
		bne.s	35$			OK, it was free to start

; the data key was already allocated, generate a key already set requester
		move.l	-4(a4),d0		another keyset error
		lea.l	KeySet(pc),a0
		bsr	ValidatorError
4$		movem.l	(sp)+,d2/a4		error, key already clear
		bra	CantVal			scrap block and exit

; store back bitmap longword and keep searching the hashtable for more data
35$		move.l	d6,0(a3,d0.l)
		subq.l	#1,d7			one less block free
40$		dbra	d2,30$			keep looking
		movem.l	(sp)+,d2/a4

; we have allocated any data blocks and marked all the other headers we want to
; visit.  If there are any outstanding, keep searching for them.
HeaderDone	movea.l	a4,a0			finished with this block
		bsr	DiscardBlock		error if we need it again
HeaderDoneNoBlock
		tst.l	d5			any more headers to visit
		bgt	ReBitSearch		yep, regs are still set up

; We have finally validated the disk.  Now we have to allocate room for the
; bitmap blocks on the disk.  If there aren't enough blocks left over then
; we won't be able to write to this disk (because there's no valid bitmap).
		movea.l	a2,a1			free the visit map (need ptr)
		moveq.l	#31,d0
		add.l	HighestBlock(a5),d0	calculate memory requirements
		sub.l	Reserved(a5),d0		reserved blocks not in bitmap
		lsr.l	#5,d0			d0 = number of longwords for BM
		lsl.l	#2,d0			d0 = number of bytes for bitmap
		jsr	_LVOFreeMem(a6)		visitmap now freed

	printf <'Validator: creating bitmap, blocks free = %ld\n'>,d7
		move.l	d7,BlocksFree(a5)	mark number of free blocks
		tst.w	WrProt(a5)		can we write bitmap out ?
		bne	ValidatorExit		no, disk write protected

		move.l	RootKey(a5),d4		get the root
		move.l	d4,d0
		bsr	GrabBlockA4
;		movea.l	d0,a4			save buffer pointer
		move.w	BitmapCount(a5),d2	get total bitmap keys needed
	printf <'Validator: need %d bitmap blocks\n'>,d2
		movea.l	BitmapKeys(a5),a2	where to store them in memory
		lea.l	cb_SIZEOF(a4),a1
		adda.l	BlockSize(a5),a1
		lea.l	vrb_Bitmap(a1),a1	where to store in root block
		moveq.l	#ROOT_BM_COUNT,d3	maximum keys in bitmap
		move.l	d4,d0			allocate near this key
		bra.s	vl20			start allocating keys

vl10		bsr	GetAKey			get a key near d0
	printf <'Validator: got bitmap key %ld\n'>,d0
		move.l	d0,(a2)+		store in bitmapkeys array
		beq.s	BMAllocFailed		no room on disk for bitmap
		move.l	d0,(a1)+		store in current block
		subq.w	#1,d2			do we need more ?
		beq.s	vl25			nope, write out bitmap data
vl20		dbra	d3,vl10			keep getting blocks

		bsr	GetAKey			we need an extender block
	printf <'Validator: got extender key %ld\n'>,d0
		move.l	d0,(a1)
		beq.s	BMAllocFailed		couldn't get it
vl25		move.l	d0,-(sp)		save the allocated extender key
		movea.l	a4,a0			write out the last block
		moveq.l	#TRUE,d1		assume it's the root block
		move.l	d4,d0
		cmp.l	RootKey(a5),d0
		beq.s	vl30			it was
		moveq.l	#RAW,d1			nope, it's an extender
vl30	printf <'Validator: writing extender block %ld\n'>,d0
		bsr	WriteBlockNow		write out this block
		move.l	(sp)+,d4		restore next key
		tst.w	d2			anything left to do ?
		beq.s	WriteBMBlocks		nope, write out bitmap data
		bsr	GetBlock		fetch a cache buffer
		movea.l	d0,a4
		lea.l	cb_SIZEOF(a4),a0
		bsr	ClearBlock		clear out this extender
		lea.l	cb_SIZEOF(a4),a1	where we're storing data
		move.l	BlockSize(a5),d3	calculate entries in block
		lsr.l	#2,d3
		subq.l	#1,d3			allow for next extender
		move.l	d4,d0			where we want to be near
		bra.s	vl20			and keep allocating

; we failed to allocate a bitmap or extender key.  There's no room for bitmap.
BMAllocFailed	movea.l	a4,a0			don't even write this block
		move.w	#1,TotallyFull(a5)	no room for bitmap
		bsr	DiscardBlock
		bra	ValidatorExit		and quit right away ^^

; we've allocated all of the bitmap and extender keys, now write out the bitmap
WriteBMBlocks	move.l	a3,-(sp)		save pointer to bitmap
		movea.l	BitmapKeys(a5),a2	point at list of bitmap keys
		move.w	BitmapCount(a5),d2	get count of blocks to do
		bra.s	ll40

;10$		bsr	GetBlock		fetch a cache buffer
;		movea.l	d0,a4			and save the pointer
;		lea.l	cb_SIZEOF(a4),a0	point at data area
;		bsr	ClearBlock		and clear it out
;		lea.l	cb_SIZEOF+4(a4),a0	point at area for bitmap data
;		move.l	BlockSize(a5),d0	calculate entries in block
;		lsr.w	#2,d0
;		subq.w	#1,d0			allow for the checksum
;		bra.s	30$
;20$		move.l	(a3)+,(a0)+
;30$		dbra	d0,20$			copy bitmap data across
;		move.l	(a2)+,d0		get key it belongs on
;		movea.l	a4,a0			and write this buffer
;		moveq.l	#BITMAP,d1		to correct bitmap block
;		bsr	WriteBlockNow
;40$		dbra	d2,10$			and do next bitmap block
;		move.l	(sp)+,a3

ll10		movea.l	BMBlock(a5),a4		must be in loop (enters @ dbra)
		lea.l	cb_SIZEOF(a4),a0	point at data area
	printf <'Validator: clearing bitmap block 0x%lx...'>,a4
		bsr	ClearBlock		and clear it out
	printf <'OK\n'>
		lea.l	cb_SIZEOF+4(a4),a0	point at area for bitmap data
		move.l	BlockSize(a5),d0	calculate entries in block
		lsr.w	#2,d0
		subq.w	#1,d0			allow for the checksum
		bra.s	30$
20$		move.l	(a3)+,(a0)+
30$		dbra	d0,20$			copy bitmap data across
		move.l	(a2)+,d0		get key it belongs on
	printf <'Validator: writing bitmap block %ld\n'>,d0
		move.l	d0,cb_Key(a4)
		move.w	#-1,cb_State(a4)	mark as dirty
		bsr	FlushBitmapBlock	and write it out
ll40		dbra	d2,ll10			and do next bitmap block
		move.l	(sp)+,a3

; Bitmaps created and written to disk.  Now we need to validate the contents
; of the directory cache blocks.  We'll scan the entire directory structure
; doing both a check for extra entries not in the directory, and an
; UpdateListIfDifferent for each entry in a directory.  This is likely to be
; slow compared to the initial bitmap scan, since we need to do connected
; dir/file accesses.  The best way to do this is a tree-scan, since most
; entries are leaf files.
		btst.b	#2,DiskType+3(a5)
		beq.s	checklist_done
		move.l	RootKey(a5),d0		start at the root
		bsr	CheckList		recursive!  FIX?
		; FIX!!!!! needs to put up a requester if it fails!!!!!
		tst.l	d0
		beq.s	ValidatorExit

; Everything is clean and on disk.  Write out the root with a valid bitmap flag
checklist_done
	printf <'Validator exiting OK\n'>
		move.w	#TRUE,BitmapValid(a5)	the bitmap can be used
		move.w	#TRUE,FilesOpen(a5)	so root gets flushed properly
		bsr	ClearWaiting		flush all waiting buffers
		bsr	WriteMap		mark bitmap as valid in root
		bra.s	ValidatorExit

; exit point when the visit map needs freeing too
VExitFree	cmpa.w	#0,a2
		beq.s	ValidatorExit
		movea.l	a2,a1			free the visit map (need ptr)
		moveq.l	#31,d0
		add.l	HighestBlock(a5),d0	calculate memory requirements
		sub.l	Reserved(a5),d0		reserved blocks not in bitmap
		lsr.l	#5,d0			d0 = number of longwords for BM
		lsl.l	#2,d0			d0 = number of bytes for bitmap
		jsr	_LVOFreeMem(a6)		visitmap now freed

; Validation complete, free memory bitmap if allocated and return to caller
ValidatorExit	cmpa.w	#0,a3			did we get a bitmap
		beq.s	10$			nope, so nothing to free
		movea.l	a3,a1			free the memory bitmap
		moveq.l	#31,d0
		add.l	HighestBlock(a5),d0	calculate memory requirements
		sub.l	Reserved(a5),d0		reserved blocks not in bitmap
		lsr.l	#5,d0			d0 = number of longwords for BM
		lsl.l	#2,d0			d0 = number of bytes for bitmap
		jsr	_LVOFreeMem(a6)		jsr/rts
10$		movea.l	KillCo(a5),a0		commit suicide now
		move.l	CurrentCo(a5),d0
		bra	ResumeCo		won't come back

;==============================================================================
; key = GetAKey( nearkey, bitmap)
; d0		   d0	    a3
;
; Attempts to allocate a key in the memory bitmap close to nearkey.  0=failure.
; This routine doesn't destroy any registers except for the return register d0.
;==============================================================================
GetAKey		tst.l	BlocksFree(a5)		anything free
		bne.s	10$			yes
		moveq.l	#FALSE,d0		nope, return false
		rts

10$		movem.l	d1-d3,-(sp)		we have at least one free block
		move.l	d0,d1			convert nearkey to lword/bit
		sub.l	Reserved(a5),d1
		moveq.l	#31,d2
		and.w	d1,d2			d2 = bit number
		lsr.l	#5,d1			d1 = longword number
		lsl.l	#2,d1			now byte offset to longword
15$		move.l	0(a3,d1.l),d3		fetch the longword
20$		bclr.l	d2,d3			allocate this bit
		bne.s	40$			it was free
		addq.l	#1,d0			bump to next block
		cmp.l	HighestBlock(a5),d0	are we in range ?
		bgt.s	30$			nope, wrap to beginning of disk
		addq.w	#1,d2			bump to next bit
		andi.w	#31,d2
		bne.s	20$
		addq.l	#4,d1			rolled into next longword
		bra.s	15$

30$		move.l	Reserved(a5),d0		back to first block
		moveq.l	#0,d1			back at longword 0
		moveq.l	#0,d2			and bit 0
		bra.s	15$			keep searching

40$		move.l	d3,0(a3,d1.l)		allocated a bit
		subq.l	#1,BlocksFree(a5)	so one less free
		movem.l	(sp)+,d1-d3
		rts

;==============================================================================
; rangeOK = VisitThis( keynum,visitmap )
;   d0			 d0	 a2
;
; Sets the bit corresponding to keynum in the visitmap.  Returns a BOOLEAN to
; indicate whether the given key could be legally visited.  FALSE means the
; key was out of the legal range for the partition.
;==============================================================================
VisitThis	movem.l	d1-d2,-(sp)
		cmp.l	HighestBlock(a5),d0	make sure key in range
		bgt.s	2$			nope, error
		cmp.l	Reserved(a5),d0
		bge.s	3$

2$		lea.l	KeyRange(pc),a0		error, key out of range
		bra.s	4$

3$		move.l	d0,d2			save key number
		moveq.l	#7,d1
		and.l	d0,d1			bit number
		lsr.l	#3,d0
		bset.b	d1,0(a2,d0.l)
		beq.s	10$			not already marked for visit

; ERROR, this key was already marked for visiting so put up key already set.
		move.l	d2,d0
		lea.l	KeySet(pc),a0
4$		bsr.s	ValidatorError
5$		moveq.l	#FALSE,d0		don't validate any more
		bra.s	20$

10$		moveq.l	#TRUE,d0		everything OK
20$		movem.l	(sp)+,d1-d2
		rts

;==============================================================================
; GetHdrType( buffer )
;		a0
;
; returns st.file or st.userdir or 0 for a bad header.  If a bad header was
; found then a requester will inform the user of this fact.
;==============================================================================
GetHdrType	cmpi.l	#t.short,cb_SIZEOF+fhb_Type(a0)
		beq.s	10$			can be file or dir or link
		cmpi.l	#t.list,cb_SIZEOF+fhb_Type(a0)
		beq.s	10$			extension block

; the header we have is bad, put up a requester to this effect.
		move.l	cb_Key(a0),d0		record key number
		lea.l	BadHeader(pc),a0	put up requester
		bsr.s	ValidatorError
		moveq.l	#0,d0			return bad type
		bra.s	GotHdrType

; first header descriptor was OK, check that secondary type is valid.
10$		lea.l	cb_SIZEOF(a0),a1
		adda.l	BlockSize(a5),a1
		tst.l	vfhb_SecType(a1)
		bmi.s	ChkHdr			check a file header

		moveq.l	#st.root,d0
		cmp.l	vfhb_SecType(a1),d0
		bne.s	11$
		moveq.l	#st.userdir,d0
		bra.s	GotHdrType		assume root is OK

11$		move.l	cb_Key(a0),d0
		cmp.l	cb_SIZEOF+udb_OwnKey(a0),d0
		beq.s	20$
		lea.l	BadDHeader(pc),a0
		bsr.s	ValidatorError		corrupt directory header
		moveq.l	#0,d0
		bra.s	GotHdrType
20$		move.l	vfhb_SecType(a1),d0	this handles links
		bra.s	GotHdrType

; we are checking a file header for validity
ChkHdr		move.l	cb_Key(a0),d0
		cmp.l	cb_SIZEOF+fhb_OwnKey(a0),d0
		beq.s	10$
		lea.l	BadFHeader(pc),a0
		bsr.s	ValidatorError		corrupt file header
		moveq.l	#0,d0
		bra.s	GotHdrType
10$		moveq.l	#st.file,d0
GotHdrType	rts

;==============================================================================
; ValidatorError( key, line2 )
;		  d0	a0
;
; Puts up a requester containing the key number where the error occurred and
; what was wrong with that key (message in line2 is appended to 'key xxx'
;==============================================================================
ValidatorError	move.l	a2,-(sp)
		lea.l	-40(sp),sp		make workspace on the stack
		movea.l	sp,a1			destination string
		move.l	d0,-(sp)		save key number
		bsr	BuildString		format the string
		addq.l	#4,sp			scrap key parameter
		movea.l	sp,a1			this is line 2 of request
		suba.l	a2,a2			there is no line 3
		lea.l	ValidatorErr(pc),a0	set up first line
		bsr	Request			retry means nothing, ignore it
		lea.l	40(sp),sp		scrap workspace used
		move.l	(sp)+,a2		and return
		rts

ValidatorErr	DC.B	'Error validating disk',0
KeySet		DC.B	'Key %ld already set',0
KeyRange	DC.B	'Key %ld out of range',0
BadHeader	DC.B	'Key %ld bad header type',0
BadFHeader	DC.B	'Key %ld corrupt file',0
BadDHeader	DC.B	'Key %ld corrupt directory',0
		CNOP	0,4

;		DEBUGENABLE

;==============================================================================
; success = CheckList( directory )
;			  d0
;
; Scan the directory doing both a check for extra entries not in the
; directory, and an UpdateListIfDifferent for each entry in a directory.
; This is likely to be slow compared to the initial bitmap scan, since we
; need to do connected dir/file accesses.  This function is recursive, and
; calls itself to handle any directories it finds.  For buffer cache
; efficiency, it recurses on directories AFTER doing UpdateListIfDifferent
; on this directory.
;
; Must have at least 3 buffers here!! 1 for dir, one for list block, and
; one for fhb.  Best to have 1 for dir, 1 for fhb, and N for list blocks!
;
; FIX!!!!! should have a stack-space check!!!!!!!
;
; I'm not sure it's always safe to hold multiple blocks at a time.  I _think_
; it's safe while in validation (at least no one else can modify them while
; we're using them).  However, maybe this might result in two copies of a
; block in buffers - not good.
;==============================================================================
CheckList:	printf <'CheckList dir %ld\n'>,d0
		movem.l	d2-d6/a2-a4,-(sp)
		move.l	d0,d2		; save directory block #
		bsr	GrabBlockA4	; read in the directory
;		move.l	d0,a4
		move.l	BlockSize(a5),d0
		move.l	cb_SIZEOF+vudb_DirList(a4,d0.l),d4  get list head ptr
		; we assume that all dirs MUST have DirList entries!!!!

cklist_loop:	; d4 is next list block to examine, a4 holds directory
		move.l	d4,d0
		bsr	GrabBlockA3		Holding two cache entries!!!
;		move.l	d0,a3			list block pointer
		moveq	#0,d3			we haven't modified this one
		lea	cb_SIZEOF+fhl_Entries(a3),a2	get start of entries
		move.l	cb_SIZEOF+fhl_NumEntries(a3),d6 get number of entries

		; check each entry in this block to see if it's in the dir
		; a2 points to current entry, d6 has number of entries left
cklist_entry	tst.w	d6
		beq	cklist_next ^^

	printf <'Checking for %ld\n'>,fle_Key(a2)
		lea	fle_FileName(a2),a0
		bsr	Hash			; returns byte offset for hash
		move.l	cb_SIZEOF(a4,d0.l),d0	; first entry of hash chain
		beq.s	ck_DeleteList		; it's a ghost!  nuke it!
		moveq	#0,d5			; no fhb fetched yet

		; a0 - fhb if any, d5 - fhb block or 0, d0 - next chain entry
ckl_chain:	cmp.l	fle_Key(a2),d0	; is this the droid we're looking for?
		beq.s	goodentry
		; ok, fetch next entry in chain
		exg.l	d5,d0			; save next fhb block number
		tst.l	d0			; Loose fhb if we have one
		beq.s	10$			; (associate with self)
		bsr	LooseBlock		; (a0,d0)
10$		move.l	d5,d0			; fetch current chain entry
		bsr	GrabBlock
;		move.l	d0,a0
		move.l	BlockSize(a5),d0	; get next hash link
		move.l	cb_SIZEOF+vfhb_HashChain(a0,d0.l),d0
		bne.s	ckl_chain		; and check this entry
		move.l	d5,d0
		bsr	LooseBlock		; (a0,d0)
		moveq	#0,d5			; no entry held any more
		; end of chain! delete it!

ck_DeleteList:	; delete current entry and continue
	printf <'extra dirlist entry %ld!!!'>,fle_Key(a2)
		subq.l	#1,cb_SIZEOF+fhl_NumEntries(a3)
		beq.s	10$			it was the last entry
		move.l	a2,a0
		bsr	NextListEntry		find next entry (updates a0,d6)
		move.l	BlockSize(a5),d0
		lea	cb_SIZEOF(a3,d0.l),a1	get pointer to end of block
		sub.l	a0,a1			leaves number of bytes
		move.l	a1,d0			number of bytes in entries
		move.l	a2,a1			destination (a0 already source)
		bsr	CopyOverlap		and copy it (may be large)
10$
		moveq	#1,d3			remember we modified it
		bra	cklist_entry		handle next entry (if any) ^^
		
goodentry:	; free any fhb entry grabbed (d5 is blocknum, a0 is fhb ptr)
		move.l	d5,d0			; associate with self
		beq.s	10$
		bsr	LooseBlock		; (a0,d0)

10$		move.l	a2,a0
		bsr	NextListEntry		(a0,d6.w) updates both
		move.l	a0,a2
		bra	cklist_entry ^^

cklist_next:	; get next dirlist block
		move.l	a3,a0			buffer
		move.l	d4,d0			block number
		move.l	cb_SIZEOF+fhl_NextBlock(a3),d4	next block key
		tst.w	d3			was this block modified?
		beq.s	10$			if not, drop it

	printf <'at least one entry deleted, writing %ld'>,d4
		moveq.l	#TRUE,d1
		bsr	WriteBlockLater		put it on queue to write out
		bra.s	20$

10$		bsr	LooseBlock		done with old list block

20$		tst.l	d4
		bne	cklist_loop		another block to search

; done scanning for extra entries.  Now walk the directory doing updates.
; we won't invoke ourselves again until we're done with all the entries
; in this directory.
; a4 has directory, d2 has dir block #, no other blocks locked.
; d3-d6/a2-a3 are free.
; if we wanted to be fast, we'd scan once, building a list of blocks in
; this directory
	printf <'doing update, dir = %ld (at $%lx)'>,d2,a4
		lea.l	cb_SIZEOF+udb_HashTable(a4),a2
		move.l	HTSize(a5),d3		number of entries to search
		subq.l	#1,d3
ckupdate_loop:
	IFD DEBUG_CODE
	tst.l	(a2)
	beq.s	1$
	printf <'Hash entry $%lx (%ld left) is %ld'>,a2,d3,(a2)
1$
	ENDC
		move.l	(a2)+,d0		get next key
		beq.s	ck_nexthash		nothing there
		; we have an entry - check it in the dirlist

; we'll modify UpdateList to check and update stuff for us.  It doesn't
; actually update if they're identical.  However, we must be wary of
; comment differences!  They change the length of an entry, so we don't
; want to update in that case.  If the comment is a different length,
; we'll use the comment from the dirlist!
ckupdate_next_fhb:
		move.l	d0,d4			; save it
		bsr	UpdateListIfDifferent	; (d0,a4,d2)

		; see if needs to be added to the list (if ULID fails)
		tst.l	d0
		bne.s	10$			; if ok, done

		; not in List!!  Add it!
	printf <'(%ld) file %ld not in dirlist! adding...'>,d0,d4
		move.l	d4,d0			; object
		bsr	InsertListMissing
		tst.l	d0
		beq.s	cklist_failed

		; ok, we updated it.  Get the next entry in the hash chain
		; also see if we should recurse.  This will almost always
		; get a cache hit, since we just checked it.
10$		move.l	d4,d0
		bsr	GrabBlockA3
;		move.l	d0,a3
		move.l	BlockSize(a5),d0	; get next hash link
		move.l	cb_SIZEOF+vfhb_HashChain(a3,d0.l),d5	; hash
		move.l	cb_SIZEOF+vfhb_SecType(a3,d0.l),d6
;	printf <'type is %ld, hashchain is %ld'>,d6,d5
		move.l	a3,a0
		move.l	d4,d0
		bsr	LooseBlock

		moveq.l	#st.userdir,d1
		cmp.l	d1,d6
		bne.s	ck_not_dir

		; it a directory.  Recurse after unlocking blocks
		move.l	a4,a0
		move.l	d2,d0
		bsr	LooseBlock		; current directory

		; we save 8 registers plus rtn addr == 36 bytes/recurse

		move.l	d4,d0			; new directory to check
		bsr	CheckList		; CHECK STACK!!!!!???? FIX!?
		tst.l	d0			; note, dir already freed!
		beq.s	cklist_freed		; return 0 (failure)!

		; ok, reaquire the old position
		sub.l	a4,a2			; offset from old block
		move.l	d2,d0
		bsr	GrabBlockA4
;		move.l	d0,a4			; current directory
		add.l	a4,a2			; make offset into new block

ck_not_dir:	; d5 has next hash entry if any
		move.l	d5,d0
		bne.s	ckupdate_next_fhb

ck_nexthash	dbra	d3,ckupdate_loop	keep going

		pea	1			; success
	printf <'CheckList (%ld) success'>,d2

		; done - free directory
cklist_exit:	; return code is on stack
		move.l	a4,a0
		move.l	d2,d0
		bsr	LooseBlock		; current directory

		move.l	(sp)+,d0		; could be combined
cklist_freed:	movem.l	(sp)+,d2-d6/a2-a4
		rts

cklist_failed:	pea	0
	printf <'CheckList (%ld) failure'>,d2
		bra.s	cklist_exit		; free directory, exit

		END
