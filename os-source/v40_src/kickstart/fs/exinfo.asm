		SECTION filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"dos/dosextens.i"
		INCLUDE "dos/exall.i"
		INCLUDE	"utility/hooks.i"
		INCLUDE	"globals.i"
		INCLUDE	"moredos.i"
		INCLUDE	"private.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XREF	Type,LooseBlock,WorkFail,WorkDone,LooseKeyBlock
		XREF	CheckLock,WaitBlock,GetPubMem,Hash,IsDir,_LVOFreeMem
		XREF	_LVOMatchPatternNoCase,NextListEntry,LooseA3Block

		XDEF	ExObject,ExNext,ExAll,DoInfo

;==============================================================================
; ExObject( packet )
;	      d0
;
; Returns information about the required object (using the lock in the packet).
; Doesn't save any registers because it's called as a co-routine and goes away.
;==============================================================================
ExObject	movea.l	d0,a2			save packet address
		movea.l	dp_Arg1(a2),a0		check the lock (BPTR)
	printf <'Examine called, lock=%ld\n'>

		bsr	CheckLock		will set key=0 if invalid
		move.l	d0,d2			was lock valid ?
		bne.s	ExObjOK			yes
		movea.l	a2,a0
		bra	WorkFail		lock was bad

ExObjOK		tst.l	dp_Arg1(a2)		if using a NULL lock
		beq.s	10$			don't attempt to mark it

		movea.l	dp_Arg1(a2),a0		mark lock as examined
		adda.l	a0,a0
		adda.l	a0,a0
		moveq.l	#-1,d1
		move.l	d1,fl_Examined(a0)

10$		bsr	WaitBlock		get the required key
		movea.l	d0,a3			stash the buffer
		bra	SetInfo			and return info about it


;==============================================================================
; entry = GetDCEntry( NET, lastkey, packet)
;   a0                a4      d4      a2
;
; Most of this routine isn't needed for ExAll, just for ExNext, though ExAll
; benefits whenever it has to break also.  Returns NULL if there isn't an
; entry to examine.  If it returns null, either no block is locked 
; (NET_BlockNum = 0) OR the initial block is still locked (NET_BlockNum != 0)
;
;==============================================================================

; Fetch dirlist block, and find the right entry.  The info
; in the dirlist is always the next entry you want to fetch.  If need be, the
; old entry value is in d4 (from fib_DiskKey).

; If NET_NextBlockNum and d2 are the same, we never let go of the dircache
; block,
; we KNOW we have the block and entry we want.  Go to GotDCBlock to find the
; right spot.  Somewhat inefficient.  Should save ptr to entry in reg!!!!!
GetDCEntry:
	printf <'GetDCEntry block %ld, key %ld\n'>,NET_NextBlockNum(a4),NET_NextKey(a4)
;	printf <'	current block %ld ($%lx)'>,NET_BlockNum(a4),NET_BlockPtr(a4)
		movem.l	d2/d5/d6/a3,-(sp)	MUST match NextDCEntry!!!!

		; restore variables from NET structure
		move.l	NET_BlockNum(a4),d2	0 if no block currently held!
		move.l	NET_BlockPtr(a4),a3
		move.l	NET_NextBlockNum(a4),d0
		beq	GetDCNoMore		if 0, last one returned was end
		cmp.l	d2,d0			is this the same block we have?
		beq.s	GotDCBlock 

;	printf <'need to get block %ld (old %ld)...'>,d0,d2
		; if we don't have the right block, we have no block (d2=0)
		move.l	d0,d2			save for looseblock
		bsr	WaitBlock
		move.l	d0,a3			keep buffer

; If numentries is 0, then the file must have been deleted (and no other
; entries were in the block).  Grab the next entry in any following list block.
; If we merely don't find it, search for the previous entry in the current
; listblock.  If we don't find THAT, just take the first one here and hope.
; If we do find it, take the next one, of course.

; Note: if NET_NextKey is 0 (initial exnext call), then we'll fail to find
; it anywhere, and will fail to find the previous one.  We should return the
; first entry in the current block (or first block with an entry).  (It fails
; to find the previous one (d4) since the previous one is the directory this
; is in, and no directory can be in it's own dirlist.  NOTE: on the first
; (and all) exnext calls, fl_Examined will be !0.  If it's 0, the lock was
; unlocked, and if we can't find the entry return no_more_entries.

GotDCBlock:	move.l	cb_SIZEOF+fhl_NumEntries(a3),d6
	printf <'got dirlist $%lx, entries=%ld\n'>,a3,d6

		subq.w	#1,d6
		move.w	d6,d5			save original number of entries
		bmi.s	GetNextListBlock	none - must have been deleted!
		lea.l	cb_SIZEOF+fhl_Entries(a3),a0
		move.l	a0,a1			save original entries ptr
		bra.s	20$
10$		bsr	NextListEntry		updates a0,d6.w
20$		move.l	fle_Key(a0),d1
		cmp.l	NET_NextKey(a4),d1	see if it's the right entry
		beq.s	FoundNextEntry		got it!
		tst.w	d6			run out of entries yet?
		bne.s	10$

	printf <'didnt find the entry!  must have been deleted!\n'>
		; didn't find our entry in this list block (must have been
		; deleted or maybe commented).  Search for previous entry.
		move.w	d5,d6
		move.l	a1,a0			get entries ptr back
		bra.s	40$
30$		bsr	NextListEntry		updates a0,d6
40$		cmp.l	fle_Key(a0),d4		see if it's the previous entry
		beq.s	ReturnNextEntry		got it!
		tst.w	d6			run out of entries yet?
		bne.s	30$

		; handle people who unlock the main lock between exnexts.
		; we use the fl_examined flag to know if anyone ever examined
		; this lock...
		cmp.l	#ACTION_EXAMINE_ALL,dp_Action(a2)
		beq.s	ReturnFirstEntry	this kludge is only for Exnext

		move.l	dp_Arg1(a2),a0		Lock
		add.l	a0,a0
		add.l	a0,a0			bptr->cptr
		tst.l	fl_Examined(a0)		was it ever examined?
		bne.s	ReturnFirstEntry	yes, return first in block

		; added test - if the previous item was the parent, return the
		; first entry anyways.  This handles people who unlock after
		; the Examine before the first ExNext...
		move.l	cb_SIZEOF+fhl_Parent(a3),d0
		cmp.l	d0,d4
		beq.s	ReturnFirstEntry

	printf <'didnt find previous entry, lock never examined...'>
		; d0 is parent - associate with parent
		bsr	LooseA3Block
		moveq	#0,d2			no block held...
		bra.s	GetDCNoMore		we're done...
		
		; didn't find either previous or next.  Return first entry
		; in current block.
ReturnFirstEntry
	printf <'didnt find previous entry! return first entry!\n'>
		move.l	a1,a0			get entries ptr back
		move.w	d5,d6			indicate we didn't skip any

; We found the entry we want.  a0 points to entry, d6.w is number
; of entries left in this block, NOT including this one.  d2 is block
; number, a3 is block.
FoundNextEntry	; save variables in NET structure for next call
		move.l	a0,NET_Entry(a4)
		move.l	d2,NET_BlockNum(a4)	0 if no block currently held!
		move.l	a3,NET_BlockPtr(a4)
		move.w	d6,NET_NumEntries(a4)	only needed for NextDCEntry...
		movem.l	(sp)+,d2/d5/d6/a3	MUST match NextDCEntry!!!!
		rts

; we come here when we found the previous entry returned.  Skip it and
; return the next entry.  a0 is last entry returned, d6 is entries left
; NOT including this one
ReturnNextEntry printf <'ReturnNextEntry'>
		tst.w	d6			any more entries?
		beq.s	GetNextListBlock	no, get next block
		bsr	NextListEntry		finds the one we want
		bra	FoundNextEntry		and return it

; we come here when we want to return the first entry of the next list
; block (with entries).
GetNextListBlock: printf <'GetNextListBlock'>
;	printf <'1:loosing block %ld ($%lx)'>,d2,a3
		move.l	cb_SIZEOF+fhl_Parent(a3),d0	associate with parent
		move.l	cb_SIZEOF+fhl_NextBlock(a3),d2  new list block
		bsr	LooseA3Block

;	printf <'getting block %ld'>,d2
		; get next list block
		move.l	d2,d0
		beq.s	GetDCNoMore		if no more blocks, we're done
		bsr	WaitBlock
		move.l	d0,a3
		tst.l	cb_SIZEOF+fhl_NumEntries(a3)
		beq.s	GetNextListBlock	none in this, fetch _next_ one

		; finally found a block with entries.  Return first.
		move.l	cb_SIZEOF+fhl_NumEntries(a3),d6
		subq.w	#1,d6				don't include this one
		lea	cb_SIZEOF+fhl_Entries(a3),a0	entry to return
		move.l	d2,NET_NextBlockNum(a4)	we have an entry, set up ET
		bra	FoundNextEntry

; no entry to return (must have been deleted).  d2 is 0 or block to loose.
GetDCNoMore:	sub.l	a0,a0			return NULL entry ptr
		bra	FoundNextEntry		so the vars get updated

;==============================================================================
; NextDCEntry( NET )
;              a4
;
; called after data is extracted from the entry, to set up for the next
; entry.  Sets NET_NextBlockNum and NET_NextKey.
;
;==============================================================================
NextDCEntry	movem.l	d2/d5/d6/a3,-(sp)	MUST match GetDCEntry!!!!

		; restore variables from NET structure
		move.l	NET_Entry(a4),a0
		move.l	NET_BlockNum(a4),d2	0 if no block currently held!
		move.l	NET_BlockPtr(a4),a3
;	printf <'NextDCEntry block %ld, entry $%lx\n'>,d2,NET_NumEntries-2(a4)
		move.w	NET_NumEntries(a4),d6

		; tst.w	d6			any more entries?
		beq.s	NextListNeeded
		bsr	NextListEntry		(a0,d6.w)
		; NET_NextBlockNum is still correct
NextDCSetKey
	printf <'NexDCEntry key %ld, block %ld, buffer $%lx'>,fle_Key(a0),d2,a0
		move.l	fle_Key(a0),NET_NextKey(a4)	next entry to return
NextDCDone:	bra	FoundNextEntry		saves vars in NET and returns

; we succeeded, but we need to fetch the next list block to find the next
; entry for the ET.
NextListNeeded
		move.l	cb_SIZEOF+fhl_Parent(a3),d0	associate with parent
		move.l	cb_SIZEOF+fhl_NextBlock(a3),d2  new list block
		bsr	LooseA3Block

		; get the next list block, if any
		clr.l	NET_NextBlockNum(a4)	default, return NO_MORE next
		move.l	d2,d0
		beq.s	NextDCDone		no more lists, return success
		bsr	WaitBlock		fetch next list block
		move.l	d0,a3
;	printf <'Fetched next list block %ld ($%lx)'>,d2,a3
		tst.l	cb_SIZEOF+fhl_NumEntries(a3)
		beq.s	NextListNeeded		none in this, fetch _next_ one

		; this leaves NextBlockNum and BlockNum the same, and Blockptr
		; now points to the next block.

		move.l	d2,NET_NextBlockNum(a4)	we have an entry, set up ET
		lea	cb_SIZEOF+fhl_Entries(a3),a0
;	printf <'Setting NextKey to %ld'>,fle_Key(a0)
		bra.s	NextDCSetKey		got it, set next key and return


;==============================================================================
; ET,parent = GetET( last, packet )
; d0,d1               d0     a2
;
; returns valid ET in d0.  Also checks the lock, makes sure the parent
; is a directory, etc.  last is used to know if we just want to validate it
; or if we want to rebuild the ET.  if last == parent, this is the first time
; through.  If this fails, it kills the coroutine, so no error checking is
; needed.  If last == 0, this was called from ExAll.
;==============================================================================
GetET		movem.l	d3-d6/a2-a4,-(sp)
		move.l	d0,d4			save last block examined
		movea.l	dp_Arg1(a2),a0		check the lock
		bsr	CheckLock
		move.l	d0,d3			stash parent directory key
		bne.s	CheckForET		no problems, it's valid
GetETFail	movea.l	a2,a0			return bad lock error
	printf <'ParentKey was bad, exiting\n'>
		bra	WorkFail		kills coroutine

CheckForET
; before we go any further, see if we have a valid ExamineThing in the Lock
		movea.l	dp_Arg1(a2),a0		check for the ExamineThing
		adda.l	a0,a0
		adda.l	a0,a0
		move.l	a0,d6			save lock ptr
		movea.l	fl_SIZEOF(a0),a4	is there an ET attached?
		cmp.w	#0,a4
		beq.s	AllocET			none there, make one

; there is an examinething here but we may have to rebuild the list of keys
; in case someone is re-using a lock for another list or dir call.  This
; happens every time you do a dir or list in the current directory.  We can
; quite easily test for this if fib_LastKey=parentkey
		tst.l	d4			were we called by ExAll?
		beq.s	20$			yes, must be first ExAll

; it's from ExNext: figure out if we need to rebuild
		tst.l	fl_Examined(a0)		do we need to rebuild?
		beq.s	20$			yes, unlocked between calls
		cmp.l	d3,d4			is this the first ExNext ?
		bne	GetETDone		no, so carry on normally
20$:
	printf <'First ExNext/ExAll, reusing ET\n'>
; see if this is a dos\4 filesystem.  If so, and validator succeeded, we
; will reset a NewExamineThing.
		tst.b	ET_Type(a4)
		beq.s	old_et			what type was it?
		tst.w	BitmapValid(a5)
		bne.s	new_et			use old if bitmap invalid

; very strange.  We have an NET, and yet the bitmap is invalid.  Maybe it
; was removed, modified, reinserted and is validating.  Free the NET and
; go to GetET (it will make an old one).  a0 still is FileLock.
		clr.l	fl_SIZEOF(a0)		make sure ptr is cleared
		move.l	a4,a1			NET to free
		moveq.l	#NET_SIZEOF,d0
		jsr	_LVOFreeMem(a6)
		bra.s	AllocET

new_et:		; we have and want an NET.  clear it and reuse
		; newly allocated ones are clear
		clr.l	NET_NextKey(a4)		clear out current key value
		clr.l	NET_NextBlockNum(a4)
		clr.l	NET_BlockNum(a4)
		bra.s	InitET

; we need to clear out any keys hanging around in the ExamineThing because
; they would screw up the sort code if we left them there.
old_et		lea.l	ET_Entries(a4),a0
		move.l	HTSize(a5),d0		clear for size of hashtable
10$		clr.l	(a0)+
		dbra	d0,10$
		bra.s	InitET			and initialise it now

; there wasn't an examinething attached to the lock so we must make one now
; see if this is a dos\4 filesystem.  If so, and validator succeeded, we
; will build a NewExamineThing.
AllocET	printf <'Constructing ExamineThing\n'>
		btst.b	#2,DiskType+3(a5)	Are we using fast dirs?
		beq.s	10$			what type was it?
		tst.w	BitmapValid(a5)
		beq.s	10$			use old if bitmap invalid
		moveq.l	#NET_SIZEOF,d0		NewExamineThing size
		moveq.l	#1,d5			for ET_TYPE
	printf <'allocating NET\n'>
		bra.s	20$

		; build old-style examinething
10$		move.l	HTSize(a5),d0		get number of hashtable entries
		lsl.l	#2,d0			convert to byte size
		add.l	#4+ET_SIZEOF,d0		+ ending entry and currentkey
		moveq	#0,d5			old-style ExamineThing

		; do the actual allocation
20$		bsr	GetPubMem		allocated clear
		tst.l	d0
		bne.s	GotET			and initialise the key list
		move.l	#ERROR_NO_FREE_STORE,ErrorCode(a5)
		bra	GetETFail		error out

; we have an ExamineThing now but we must initialise and sort the key list
; d5 is the type of the new ET
GotET		movea.l	d0,a4			stash ExamineThing pointer
		move.b	d5,ET_Type(a4)		set the type of ET
		movea.l	d6,a0			in the lock structure too
		move.l	a4,fl_SIZEOF(a0)	** gotta update includes **

InitET		move.l	d3,d0			read parent directory key
		bsr	WaitBlock
		movea.l	d0,a3

		move.l	d3,-(sp)		stash parent key for a while

; make sure it was a directory that was initially examined and not a file header
; If it's not a directory - return error_no_more_entries.  Lock will be
; cleaned up when UnLock is called (ET freed).  If we branch to NoMore, we don't
; care about stuff on the stack (the coroutine is being killed).
		movea.l	a3,a0
		bsr	IsDir			preserves a0!
		tst.l	d0
		beq	NoMore			returns packet, kills coroutine

		tst.b	ET_Type(a4)		which type of examinething?
		beq.s	FillOldET

		; fill in new-style ET
		move.l	BlockSize(a5),d0	find first dirlist block
		move.l	cb_SIZEOF+vudb_DirList(a3,d0.l),NET_NextBlockNum(a4)
		addq.w	#4,sp			drop dir block # (d3 is valid!)
		bra.s	DropDir			done with directory

		; fill an old-stype ET
FillOldET	lea.l	cb_SIZEOF+udb_HashTable(a3),a0
		move.l	HTSize(a5),d3		number of entries to search
		subq.l	#1,d3
FillLoop	move.l	(a0)+,d0		get next key
		beq.s	30$			nothing there
		lea.l	ET_Entries(a4),a1	put entries lowest to highest
10$		move.l	(a1)+,d1		get next entry in table
		beq.s	20$			-4(a1) is where it lives
		cmp.l	d1,d0
		bgt.s	10$
		move.l	d0,-4(a1)		found slot for this one
		move.l	d1,d0
		bra.s	10$
20$		move.l	d0,-4(a1)		fill in last entry
30$		dbra	d3,FillLoop		keep going

		move.l	(sp)+,d3		get back parent key
		lea.l	ET_Entries(a4),a1	fill in current key
		move.l	a1,ET_CurrentKey(a4)

DropDir		move.l	d3,d0			associate with this block (itself)
		bsr	LooseA3Block		frees parent block

		; we have an ET, return it
GetETDone	move.l	a4,d0
		move.l	d3,d1
		movem.l	(a7)+,d3-d6/a2-a4
		rts

;==============================================================================
; GetLinkInfo ( source, dest, sourcetype, dest-type, ed-type)
;		  a0     a1      d0          d1        d2
;
; Sets the fields that can be modified by a link.  REQUIRES HOLDING 2
; BUFFERS AT ONCE!  (This is ok now).  SrcType 0 means source is a buffer, type
; 1 means source is an fle pointer.  DestType 0 means fib, type 1 means an
; ExAllData structure (ed_...).  Preserves all regs.  d2 only valid for
; dest-type 1.
;==============================================================================
GetLinkInfo:	movem.l	d0-d1/d3/a0-a2,-(sp)	save regs

		; grab the fields that might be modified by the link...
		tst.l	d0
		beq.s	fhb_passed
		moveq	#0,d3
		move.w	fle_Days(a0),d3		date is stored as words!
		move.l	d3,-(sp)		(gives us 179 years) 20(sp)
		move.w	fle_Min(a0),d3
		move.l	d3,-(sp)			16(sp)
		move.w	fle_Tick(a0),d3
		move.l	d3,-(sp)			12(sp)
		move.l	fle_Size(a0),-(sp)		8(sp)
		move.l	fle_Protection(a0),-(sp)	4(sp)
		move.l	fle_OwnerXID(a0),-(sp)		0(sp)
		bra.s	got_data

fhb_passed:	lea	cb_SIZEOF(a0),a0
		add.l	BlockSize(a5),a0		a0 = vfhb ptr
		move.l	vfhb_Days(a0),-(sp)		20(sp)
		move.l	vfhb_Mins(a0),-(sp)		16(sp)
		move.l	vfhb_Ticks(a0),-(sp)		12(sp)
		move.l	vfhb_ByteSize(a0),-(sp)		8(sp)
		move.l	vfhb_Protect(a0),-(sp)		4(sp)
		move.l	vfhb_OwnerXID(a0),-(sp)		0(sp)

got_data:	; a0 is fle_xxx or vfhb_xxx...
		; save some regs
		move.l	a1,a2			dest structure ptr
		move.l	d1,d3			dest structure type
		bne.s	5$			it's not a fib
		move.l	fib_EntryType(a2),d1
		bra.s	7$
5$		move.l	ed_Type(a2),d1
7$
		cmpi.w	#ST_FLINK,d1		is it a file link ?
		beq.s	10$			yep
		cmpi.w	#ST_DLINK,d1		was it a directory link ?
		bne.s	50$			nope, normal dir or file

; It's a link!  Grab header this links to.  Requires 2 buffers!!!! FIX (NOT!)
10$		; we must fetch the block, and then the block it's linked to
		; DANGER! we hold 2 blocks here!!! FIX (NOT!)
		tst.l	d0			do we need 1 or two indirects?
		beq.s	20$			1
		move.l	fle_Key(a0),d0		2 indirects
		bsr	WaitBlock		fetch the link
		move.l	d0,a0
		move.l	BlockSize(a5),d0
		move.l	vfhb_Link+cb_SIZEOF(a0,d0.l),-(sp) the "real" file/dir
		bsr	LooseKeyBlock		associate with self
		move.l	(sp)+,d0		now grab the real file/dir
		bra.s	30$

20$		move.l	vfhb_Link(a0),d0	real file/dir to grab

30$		bsr	WaitBlock		grab the linked-to block
		move.l	d0,a0			don't overwrite this!
		lea.l	cb_SIZEOF(a0),a0	reset a0 vfhb ptr
		adda.l	BlockSize(a5),a0

		; modify the values on the stack... (avoid d0!)
		move.l	vfhb_Days(a0),20(sp)		20(sp)
		move.l	vfhb_Mins(a0),16(sp)		16(sp)
		move.l	vfhb_Ticks(a0),12(sp)		12(sp)
		clr.l	8(sp)			size... (assume a dir)
		tst.l	d3			is dest a fib?
		bne.s	35$			no, it's an ed_xxxx
		tst.l	fib_EntryType(a2)	is this a dir?
		bra.s	37$			doesn't modify cc's!!!
35$		tst.l	ed_Type(a2)		is this a dir?
37$		bpl.s	40$			yes
		move.l	vfhb_ByteSize(a0),8(sp) size...
40$		move.l	vfhb_Protect(a0),4(sp)	protection...
		move.l	vfhb_OwnerXID(a0),(sp)	ownerid...

		; ok, dump the linked-to object
		move.l	d0,a0			ptr to buffer still in d0
		bsr	LooseKeyBlock		associate with self

		; these are the fields that are fetched from the link (if any)
50$		tst.l	d3			are we modifying a fib?
		bne.s	60$			no
		move.l	20(sp),fib_DateStamp(a2)   set the date
		move.l	16(sp),fib_DateStamp+4(a2)
		move.l	12(sp),fib_DateStamp+8(a2)
		move.l	8(sp),d0		size
		move.l	d0,fib_Size(a2)
		move.w	BlockShift(a5),d1	make numblocks
		lsr.l	d1,d0			we ignore extensions
		addq.l	#1,d0			for fileheader
		move.l	d0,fib_NumBlocks(a2)	approximate
		move.l	4(sp),fib_Protection(a2)
		move.l	(sp),fib_OwnerUID(a2)	sets both!!
		bra.s	link_exit

60$		; not a fib.  d2 is the ED_xxx value, a2 is ed_xxx ptr
		cmpi.w	#ED_DATE,d2		do we need size
		blt.s	70$			nope
		move.l	20(sp),ed_Days(a2)	set the date
		move.l	16(sp),ed_Mins(a2)
		move.l	12(sp),ed_Ticks(a2)

70$		cmpi.w	#ED_SIZE,d2		do we need size
		blt.s	link_exit		nope
		move.l	8(sp),ed_Size(a2)

		cmpi.w	#ED_PROTECTION,d2	do we need protection bits ?
		blt.s	link_exit		nope
		move.l	4(sp),ed_Prot(a2)

		cmpi.w	#ED_OWNER,d2		do we need owner info ?
		blt.s	link_exit		nope
		move.l	(sp),ed_OwnerUID(a2)

link_exit	lea	6*4(sp),sp		drop the 6 values
		movem.l	(sp)+,d0-d1/d3/a0-a2
		rts

;==============================================================================
; ExNext( packet )
;	    d0
;
; Examines the next entry in a directory.  If the previous call was to Examine
; then fl_SIZEOF will be 0 (no ExamineThing) and fib_DiskKey will be the same
; as the key returned by CheckLock.  This means we will have to set up an
; ExamineThing on this lock and fill it with the keys referenced by this dir.
; These keys are sorted by block order, lowest to highest.
;
; If the previous call was to ExNext then two things can happen.  In the
; normal case, fl_SIZEOF will contain a pointer to an initialised ExamineThing
; and we'll use the state information in there to find where the next file is.
; In the horrible case, the caller could have UnLocked the directory between
; ExNext calls which will have lost all the state information that was held
; in the ExamineThing.  If this is the case, then the ExamineThing is rebuilt
; and all the headers are scanned until we run out or we find the first one
; larger than the current fib_DiskKey.  This is a kluge to support Lattice
; and Manx DNext calls that UnLock the directory between ExNext calls.
;
; New for dos\4: since we have compressed directory blocks, we may also have
; a DirList block number and entry number within it.  This will only happen
; for dos\4 filesystems, and only when the disk has validated successfully
; (after that, they'll be maintained correctly).  We'll also use the fib key
; value to try to reconnect with the right spot if needed, either because of
; the lattice/manx DNext junk, or if the directory changed since we last
; looked at it.  If it did, we'll try to locate the key we wanted in the
; current block.  Deletions can cause it to move down in the current block,
; but not up (currently), and currently it can't move to another block.  If
; the entry is not there at all, we look for the previous entry (fib_DiskKey).
; If we find that, we return the next entry.  If we don't find it, we return
; the first entry of the block (since both must have been deleted - nasty).
; In various cases were one or both were deleted, we may have to fetch more
; dirlist blocks until we find an entry to return.
;
; NET_NextBlockNum = 0 means there are no more entries to return.
;
; The only problem comes from SetComment.  This causes deletion and 
; re-insertion of a block.  we'll live with it showing up twice.  Oh well.
;
; Careful: these need to be coordinated with the insertion/deletion logic!
;==============================================================================
ExNext	printf <'ExNext called\n'>
		movea.l	d0,a2			save the packet
		moveq	#0,d0			we're not ExAll
		movea.l	dp_Arg2(a2),a0		stash fib_LastKey
		adda.l	a0,a0			to make the kluge check quicker
		adda.l	a0,a0
		move.l	fib_DiskKey(a0),d0
		move.l	d0,d4			save last examined for later
		bsr	GetET			returns ET in d0 or KillCo's
		move.l	d0,a4			no error checking needed
		move.l	d1,d3			secondary result is parent key

; now we have a valid ExamineThing pointed to by a4.  Use ET_CurrentKey to
; find out which header we have to read next and check for an extension.
; If it's a new-style ET, handle in new way.
FastMethod	tst.b	ET_Type(a4)		is this new style?
		beq.s	OldFastMethod		no

; get the entry to examine.  NULL means there is no entry. Returns in a0!
		clr.l	NET_BlockNum(a4)	no block currently held
		bsr	GetDCEntry		(a4,d4,a2)
		move.l	a0,d0
		beq	NoMore			no entry to examine (d2==0)

; fill in info and get set for next ExNext.  a0 points to entry.
		movea.l	dp_Arg2(a2),a1		get pointer to fib
		adda.l	a1,a1
		adda.l	a1,a1			cptr
	printf <'FoundNextEntry (%ld, block %ld, left %ld to $%lx)\n'>,fle_Key(a0),NET_BlockNum(a4),NET_NumEntries(a4),a1
		move.l	fle_Key(a0),fib_DiskKey(a1)
		move.b	fle_Type(a0),d0
		ext.w	d0
		ext.l	d0
		move.l	d0,fib_DirEntryType(a1)
		move.l	d0,fib_EntryType(a1)	no one should use this

		movem.l	a0/a1,-(sp)		save (TRICKY below)
		lea	fle_FileName(a0),a0	now filename and comment
		lea	fib_FileName(a1),a1
		moveq	#0,d0
		move.b	(a0),d0
10$		move.b	(a0)+,(a1)+
		dbra	d0,10$			always copies length byte

		move.l	4(sp),a1		TRICKY!!!!
		lea	fib_Comment(a1),a1	now copy comment (a0 is correct)
		moveq	#0,d0
		move.b	(a0),d0
20$		move.b	(a0)+,(a1)+
		dbra	d0,20$			always copies length byte
		movem.l	(sp)+,a0/a1		restore

		; fill in the data that may be modified by a link...
		moveq	#1,d0			a0 is an fle_..., not a fhb
		moveq	#0,d1			dest is a fib
		bsr	GetLinkInfo		(a0,a1,d0,d1- regs unchanged)

		; done filling in entry.  Find next and setup ET for next call
		bsr	NextDCEntry

; FIX!!! can be combined with exall
		; free list block and exit
		bsr	FreeNETBlock

		clr.w	ErrorCode+2(a5)		no error
	printf <'ExNext succeeded'>
		moveq.l	#TRUE,d0		we succeeded
		move.l	a2,a0			packet to return
		bra	WorkDone		all done, return

;
; finally, here's the non-dos\4 method of doing exnext...
;
OldFastMethod	movea.l	ET_CurrentKey(a4),a0	get next key to read
	printf <'Next ExamineKey=%ld\n'>,(a0)
		move.l	(a0)+,d2
		beq.s	SetInfo			none, we are at the end
		move.l	a0,ET_CurrentKey(a4)	and update current key pointer
		move.l	d2,d0			go read this key
		bsr	WaitBlock
		movea.l	d0,a3			stash the buffer

		lea.l	cb_SIZEOF(a3),a0
		adda.l	BlockSize(a5),a0
		cmp.l	vfhb_Parent(a0),d3	does this file belong ?
		bne.s	30$			no, another header got here!
		cmpi.l	#t.short,cb_SIZEOF+fhb_Type(a3)  valid file type ?
		beq.s	40$			header type checks out OK

; it's possible for another process to delete a key we had stashed and replace
; it with another file in between our ExNext calls.  The above tests make sure
; this is not the case and come here to try again if it was.
30$		; movea.l	a3,a0		no good, scrap this block
		move.l	d2,d0			associate with itself
		bsr	LooseA3Block		(could have been deleted)
		suba.l	a3,a3			no buffer held onto now
		bra.s	OldFastMethod		and try again

; we've a valid header, see if it has an extension to be linked into the table
40$		move.l	vfhb_HashChain(a0),d0
		beq.s	70$			no extension, carry on

; we must insert an extension key into our list of keys to be examined.
		movea.l	ET_CurrentKey(a4),a0
		subq.l	#4,a0			back up pointer by 1
		move.l	a0,ET_CurrentKey(a4)
50$		move.l	4(a0),d1		is next key empty
		beq.s	60$			yep, current pos is our slot
		cmp.l	d1,d0			are we less than next key
		blt.s	60$			yep, current pos is our slot
		move.l	d1,(a0)+		move next key down
		bra.s	50$			and keep looking
60$		move.l	d0,(a0)			save the extra key

; now we have to do the Lattice/Manx kluge check.  If the header key we just
; fetched is less than fib_DiskKey and parent!=fib_DiskKey, loop and try again
;70$		cmp.l	d4,d3			if d3=d4 then first ExNext
;		beq.s	SetInfo			so we can call SetInfo
;		cmp.l	d4,d2			are we past last place ?
;		ble.s	30$			no, loose a3 and scan again

; Changed so that we are not incompatible with oldfs disks.  When the owning
; directory is examined we set a flag in the lock.  If the lock we are now
; using doesn't have this field set, then it has been freed so we will have
; to wind through the ExNext process until we get to the required key.  When
; we find that key, we reset the flag and wind through one more time (to get
; the next entry).
70$		cmp.l	d4,d3
		beq.s	SetInfo

		movea.l	dp_Arg1(a2),a0		in the lock structure too
		adda.l	a0,a0
		adda.l	a0,a0
		tst.l	fl_Examined(a0)		was lock freed between ExNexts?
		bne.s	SetInfo			nope, use the current entry
	printf <'Unlocked, current key = %ld\n'>,d2
		cmp.l	d4,d2			are we back to where we were
		bne.s	30$			no, so search until we are
		subq.l	#1,fl_Examined(a0)	yes, mark as examined
		bra.s	30$			and scan one more entry

;==============================================================================
; SetInfo( packet, buffer, key )
;	     a2	     a3     d2
;
; Fills in a FileInfoBlock with the data from a file or directory header.  The
; packet that requested this info is automatically sent back by this routine
; and the co-routine that called us is killed.  Can fail if buffer or key = 0.
; Doesn't save registers because it's called as a part of various co-routines
; and then killed.  NOTE: packet is in a2 not d0 as with pure co-routines.
; if the key was bad then there will be no buffer to free up.
;
; args were modified because ExNext and ExObject were moving packet and buffer
; from a2 and a3 anyway.  These are the regs that SetInfo used so there was
; no point transferring them.  Just consider SetInfo as part of ExNext or
; ExInfo and this register convention ain't so bad!
;==============================================================================
SetInfo		tst.l	d2			is this a valid key number ?
		bne.s	goodkey			yes, it's OK
NoMore		move.w	#ERROR_NO_MORE_ENTRIES,ErrorCode+2(a5)
		movea.l	a2,a0			fail this packet
		bra	WorkFail		won't come back, end of dir
goodkey		bsr.s	DoInfo			do stuff for the packet
		movea.l	a2,a0
		bra	WorkDone		send packet back and die

;==============================================================================
; This has been broken out as a separate routine so it can be used for ACTION_
; EXAMINE_FH aswell.  Saves us duplicating a whole bunch of code.  Doesn't save
; registers because it's more often called as a co-routine under ExNext and I
; don't want to slow it down too much.  Regs should be saves by EXAMINE_FH code
;==============================================================================
DoInfo	printf <'DoInfo called\n'>
		movea.l	dp_Arg2(a2),a4		get fib pointer
		adda.l	a4,a4			convert to machine ptr
		adda.l	a4,a4
		movea.l	a3,a0			check the type of this buffer
		bsr	Type
		move.l	d0,fib_DirEntryType(a4)	save directory type
		move.l	d0,fib_EntryType(a4)	and plain type
		move.l	d2,fib_DiskKey(a4)	key of where this block is

; name is 8 longwords long
		lea.l	cb_SIZEOF(a3),a0
		adda.l	BlockSize(a5),a0
		movem.l	vfhb_FileName(a0),d0-d7
		movem.l	d0-d7,fib_FileName(a4)

; comment is 20 longwords long
		clr.w	fib_Comment(a4)		assume none
		tst.b	vfhb_Comment(a0)
		beq.s	DoDate			no comment to copy
		movem.l	vfhb_Comment(a0),d0-d7
		movem.l	d0-d7,fib_Comment(a4)
		movem.l	vfhb_Comment+32(a0),d0-d7
		movem.l	d0-d7,fib_Comment+32(a4)
		movem.l	vfhb_Comment+64(a0),d0-d3
		movem.l	d0-d3,fib_Comment+64(a4)

; date is 3 longwords long
DoDate		; date is now handled by GetLinkInfo

; we've done all the stuff that's common to dirs, files and links; now see
; if we need to read another header for a linked file or directory.
		moveq	#0,d0			a0 is an block ptr, not an fle
		move.l	a3,a0			block ptr
		moveq	#0,d1			a1 is a fib ptr
		move.l	a4,a1			fib ptr
		bsr	GetLinkInfo		(a0,a1,d0,d1 - regs unchanged)

; all done, free up this block and return to caller (remember buffer was freed)
		move.l	cb_Key(a3),d0		associate with self
		bsr	LooseA3Block		block back on valid queue
		clr.w	ErrorCode+2(a5)		nothing wrong
		moveq.l	#TRUE,d0		no problems
		rts

;==============================================================================
; ExAll( packet )
;	   d0
;
; Fills a buffer with as many names as possible from the given directory.  The
; packet arguments are as follows :-
;
;	dp_Arg1		BPTR	Lock	lock on directory to be scanned
;	dp_Arg2		APTR	Buffer	memory area to be filled with names
;	dp_Arg3		LONG	Size	maximum size of the memory area
;	dp_Arg4		LONG	Data	codes describing which data is required
;	dp_Arg5		APTR	Control	pointer to control structure
;
; Lock, Buffer and Size are self explanatory though Lock MUST be on a dir.
; Data is a value from 1 to 6 that determines which information is to be
; stored in the buffer.  Each higher value adds a new thing to the list
; as described in the table below:-
;
;	1		FileName
;	2		Type
;	3		Size in bytes
;	4		Protection bits
;	5		3 longwords of date
;	6		Comment (will be NULL if no comment)
;	7		Owner info
;
; The data is put into the buffer in the following format.  Note, only the
; fields selected by the Data value will be present.
;
;	STRUCT ExAllData,0
;		APTR	Next		pointer to next entry or NULL
;		APTR	Name		pointer to CSTR name
;		ULONG	Type		same type values as for ExNext et al.
;		ULONG	Size		size of file in bytes
;		ULONG	Prot		protection bits
;		ULONG	Days		date fields
;		ULONG	Mins
;		ULONG	Ticks
;		APTR	Comment		pointer to BSTR comment
;		LABEL	strings		strings will start here
;	LABEL ExAllData_SIZEOF
;
; The Next field has been included (as have the pointers to name and comment)
; to make it easier for 'C' programs to parse the buffer.  If the strings,
; which are variable length, were made part of the structure, all types of
; unions and casting would be required.
;
; The control structure is required so that FFS can keep track if more than
; one call to ExAll is required.  This happens when there are more names in
; a directory than will fit into the buffer.  The format of the control
; structure is as follows:-
;
;	STRUCTURE Control,0
;		ULONG	Entries		number of entries in buffer (return)
;		ULONG	LastKey		for keeping track (must be 0 to start)
;		APTR	MatchString	pointer to BSTR for pattern matching
;		APTR	MatchFunc	pointer to function for comparing
;	LABEL Control_SIZEOF
;
; Entries:  This field tells the calling application how many entries are
;	    in the buffer after calling ExAll.
;
; LastKey:  This field ABSOLUTELY MUST be initialised to 0 before calling
;	    ExAll for the first time.  Any other value will cause nasty
;	    things to happen.  If ExAll doesn't return ERROR_NO_MORE_ENTRIES,
;	    then this field should not be touched before making the second
;	    and subsequent calls to ExAll.  Whenever ExAll returns TRUE, there
;	    are more calls required before all names have been received.
;	    As soon as a FALSE return is received (and IOErr says no_more..)
;	    then ExAll has completed.
;
; MatchString
;	    if this field is NULL then all filenames will be returned.  If
;	    this field is non-null then it is interpreted as a pointer to
;	    a string that is used to pattern match all file names before
;	    accepting them and putting them into the buffer.  The default
;	    AmigaDOS pattern match routine is used unless......
;
; MatchFunc: ....contains a pointer to a function to perform pattern
;	    matching on two strings.  If no MatchFunc is to be called then
;	    this entry should be NULL.  The MatchFunc is called with
;	    the following parameters:-
;
;	    BOOL = MatchFunc( DataVal, BufferData, MatchString )
;				d0	  a0		a1
;
;	    All args are passed on the stack AND in the indicated registers.
;	    d0-d1/a0-a1 are scratch, all other registers must be restored.
;
;	    DataVal is the number describing which fields are present.
;	    BufferData is the pointer to the current entry in the buffer
;	    MatchString is an APTR to a BSTR to be matched against.
;	    MatchFunc should return FALSE if the entry is not to be
;	    accepted, otherwise return TRUE.
;
;
; ExAll is started as a coroutine, all registers are scratch!
;==============================================================================
ExAll		movea.l	d0,a2			save packet address
		movea.l	dp_Arg5(a2),a3		get control struct
		clr.l	c_Entries(a3)		no entries in buffer
		tst.l	c_LastKey(a3)		is this the first call ?
		bne.s	ExAllContinue		nope, continuing scan

; this is the very first call to ExAll for the given directory.  Check that
; we really do have a directory lock and initialise the ExamineThing to
; start scanning the directory entries.
		moveq.l	#0,d0			this is ExAll calling...
		bsr	GetET			get ET or KillCo this rtn
		move.l	d0,a4			ET or NET
		move.l	d1,d7			key of directory
		bra.s	ExAllSetup

; This entry point is used if this is a subsequent call to ExAll because the
; previous calls didn't get all of the files (there was no room in the buffer).
; It assumes that the given lock has the appropriate ExamineThing so the Lock
; must NOT be freed between calls to ExAll.
ExAllContinue	movea.l	dp_Arg1(a2),a0		get the lock
		adda.l	a0,a0			convert to APTR
		adda.l	a0,a0
		move.l	fl_Key(a0),d7		save disk key of directory
		movea.l	fl_SIZEOF(a0),a4	get the ExamineThing
ExAllSetup
		move.l	dp_Arg2(a2),d2		stash current buffer address
		move.l	dp_Arg3(a2),d3		get max buffer size
		moveq.l	#0,d4			no previous entry yet

; FIX!!! not safe!!!! has the same problem ExNext does!!!!!
; if a file is deleted, and then a new file created using the same header,
; before it has bubbled to the top of the ET, you can jump directories!
; I'll add a sanity check to make sure that any entry found has a parent of
; the current dir - that at least makes the danger minimal.  -- REJ

; now we have a valid ExamineThing pointed to by a4.  Use ET_CurrentKey to
; find out which header we have to read next and check for an extension.
; Note: ET can now be NET!
ExAllLoop	tst.b	ET_Type(a4)
		beq.s	exall_old_et
; DCFS
; get the entry to examine.  NULL means there is no entry.  Returns a0!
		; may be holding a block already....
		move.l	d4,d5			save d4!!!
		movea.l	dp_Arg5(a2),a0		get control struct
		move.l	c_LastKey(a0),d4	last key examined
	printf <'ExAll loop: block %ld ($%lx), nextblock %ld, nextkey %ld, lastkey %ld'>,NET_BlockNum(a4),NET_BlockPtr(a4),NET_NextBlockNum(a4),NET_NextKey(a4),d4
		bsr	GetDCEntry		(a4,d4,a2)
		move.l	d5,d4			restore
		move.l	a0,d0
		beq	ExAllFitted		no entry to examine (d2==0)
		bra.s	reserve_space		 stack

; for non-DCFS partitions
exall_old_et:	movea.l	ET_CurrentKey(a4),a0	is there another key ?
		tst.l	(a0)
		beq	ExAllFitted		nope, we are finished

; common for DCFS and non-DCFS
reserve_space	move.l	dp_Arg4(a2),d0		calculate max memory needed
		move.l	d0,d1
		lsl.w	#2,d1			size = code * 4 + 4 + 32(name)
		addq.w	#4,d1
		move.w	d1,d5			need this later (name area)
; FIX! namemax!
		addi.w	#32,d1			assuming name always required
		cmpi.w	#ED_DATE,d0
		blt.s	10$
		addq.w	#8,d1			if >=5 add 8 bytes for date
		addq.w	#8,d5			and offset name address too
10$		cmpi.w	#ED_COMMENT,d0		if >=6 add 80 bytes for comment
		bcs.s	20$			branch on d0 < 6
		addi.w	#80,d1
20$		cmp.l	d1,d3			do we have room in the buffer
		blt	ExAllNoFit		nope, must exit now

; update number of entries, next pointer in last entry, init this entry
; (this is common to DCFS and non-DCFS, so do it first)
		movea.l	dp_Arg5(a2),a0		get control struct
		addq.l	#1,c_Entries(a0)	and bump entry count
		tst.l	d4			is there a previous entry ?
		beq.s	80$			nope
		movea.l	d4,a0			yes, update its next pointer
		move.l	d2,ed_Next(a0)		to point to this entry

80$		move.l	d4,d6			save prev (in case rejected)
		move.l	d2,d4			mark this entry as previous
		movea.l	d2,a0			and current as having no next
		clr.l	ed_Next(a0)		clear ed_Next pointer

; now comes the major dividing point between DCFS and non-DCFS.  We could
; perhaps find a way to share more of it in the future.
; d2&d4 have buffer address, d6 has previous buffer, d5 has offset to name
; d3 has space left in buffer, d7 has parent directory key
;	printf <'d2-d6 = $%lx $%lx $%lx $%lx $%lx'>,d2,d3,d4,d5,d6
		tst.b	ET_Type(a4)
		bne	exall_fill_DCFS

; this code fills in non-DCFS entries
		movea.l	ET_CurrentKey(a4),a0	get next key to read
		move.l	(a0)+,d0
		move.l	a0,ET_CurrentKey(a4)	and update current key pointer
		movea.l	dp_Arg5(a2),a1		update lastkey in control struct
		move.l	d0,c_LastKey(a1)
		bsr	WaitBlock		read this block
		movea.l	d0,a3			stash the buffer

		lea.l	cb_SIZEOF(a3),a0
		adda.l	BlockSize(a5),a0
; This is a fix to make sure we don't "jump" directories (see above).  Make
; sure any entry we grab has a parent of the directory we're examining.
		cmp.l	vfhb_Parent(a0),d7
		bne	ExAllNext		wrong directory!!!!!!!
		
; we've a valid header, see if it has an extension to be linked into the table
		move.l	vfhb_HashChain(a0),d0
		beq.s	70$			no extension, carry on

; we must insert an extension key into our list of keys to be examined.
		movea.l	ET_CurrentKey(a4),a0
		subq.l	#4,a0			back up pointer by 1
		move.l	a0,ET_CurrentKey(a4)
50$		move.l	4(a0),d1		is next key empty
		beq.s	60$			yep, current pos is our slot
		cmp.l	d1,d0			are we less than next key
		blt.s	60$			yep, current pos is our slot
		move.l	d1,(a0)+		move next key down
		bra.s	50$			and keep looking
60$		move.l	d0,(a0)			save the extra key

; we have a header pointed to by a3 and a buffer with enough room for the
; data required.  Get the relevant info from the header and store it into
; the callers buffer (using d2 as an address holder for this).  Calculate
; how much data space is used and update the buffer pointer accordingly.
;
; We must handle links here!  First copy type, key, name, comment, and date,
; then set numblocks, size, protection bits, and ownerid from the link.
70$		movea.l	d2,a0			get current entry ptr back

; we saved the offset to the name earlier (in d5) use this to calculate the
; name pointer and store it in the buffer (assume name is always required)
		movea.l	d2,a1			get current address
		adda.w	d5,a1			calculate name address
		move.l	a1,ed_Name(a0)		and store in buffer
		lea.l	cb_SIZEOF(a3),a0
		adda.l	BlockSize(a5),a0
		lea.l	vfhb_FileName(a0),a0
		moveq	#0,d0
		move.b	(a0)+,d0		get length of name
		bra.s	91$
90$		move.b	(a0)+,(a1)+		transfer name as a C string
91$		dbra	d0,90$
		clr.b	(a1)+			must NULL terminate
		move.l	a1,d0			pad to next longword boundary
		addq.l	#3,d0
		andi.w	#$fffc,d0
		movea.l	d0,a1
	
; a1 is now pointing to the tentative address of the comment (if it is there)
		move.l	dp_Arg4(a2),d0
		cmpi.w	#ED_COMMENT,d0		will we need comment
		bcs.s	100$			nope, do the rest
		movea.l	d2,a0			yes, set up the address
		clr.l	ed_Comment(a0)		assume no comment
		lea.l	cb_SIZEOF(a3),a0
		adda.l	BlockSize(a5),a0
		tst.b	vfhb_Comment(a0)
		beq.s	100$			correct, there was none

; Steve had a bug, and left ed_Comment a BSTR (ugh).  He documented it as
; a CSTR.  It's a really bad spot to be in...  We'll go with CSTR.
		move.l	a0,-(sp)
		movea.l	d2,a0
		move.l	a1,ed_Comment(a0)
		move.l	(sp)+,a0
		lea.l	vfhb_Comment(a0),a0
		moveq	#0,d0
		move.b	(a0)+,d0		get length of comment
		bra.s	96$
95$		move.b	(a0)+,(a1)+		copy as CSTR!!!!!
96$		dbra	d0,95$
		clr.b	(a1)+			terminate - a1 is live!
		move.w	a1,d0
		and.w	#1,d0
		add.w	d0,a1			round up to next word!

; a1 will now be pointing to the address of the next entry.  Update bytes left.
100$		move.l	a1,d2			this is the next entry
		move.l	d2,d0			calculate bytes used
		sub.l	d4,d0			d4 is now current (and prev)
		sub.l	d0,d3			d3 holds bytes left

; all the variable length stuff has been done so now do the fixed length things
		movea.l	d4,a0			d4 is holding buffer address
		move.l	dp_Arg4(a2),d0

		cmpi.w	#ED_TYPE,d0		do we need type ?
		blt	CheckMatch		nope, only name
		movem.l	d0/a0,-(sp)		yes, get the type
		movea.l	a3,a0
		bsr	Type
		move.l	d0,d1
		movem.l	(sp)+,d0/a0
		move.l	d1,ed_Type(a0)

; fetch the linked fHB if any...
		move.l	d2,-(sp)
		move.l	d0,d2			arg for GetLinkInfo
		moveq	#1,d1			dest is an ed_xxx
		move.l	a0,a1			dest is an ed_xxx
		moveq	#0,d0			src is buffer
		move.l	a3,a0			src is buffer
		bsr	GetLinkInfo		(a0,a1,d0,d1 - regs unchanged)
		move.l	(sp)+,d2

		bra	CheckMatch

; this is a dircache entry - fill exall entry from dircache
; FIX!!!! a lot of this could be shared with non-DCFS with some work!
exall_fill_DCFS:
		move.l	NET_Entry(a4),a3	get entry pointer back
	printf <'exall, key %ld, entry $%lx,buffer $%lx'>,fle_Key(a3),a3,d2
		movea.l	dp_Arg5(a2),a0		update lastkey in control struct
		move.l	fle_Key(a3),c_LastKey(a0)

; we have a dircache entry in a3 and a buffer with enough room for the
; data required.  Get the relevant info from the header and store it into
; the callers buffer (using d2 as an address holder for this).  Calculate
; how much data space is used and update the buffer pointer accordingly.
;
; We must handle links here!  First copy type, key, name, comment, and date,
; then set numblocks, size, protection bits, and ownerid from the link.
; This will require holding two blocks at once!!!!! DANGER!!  but we're
; going to require (and force) more than one buffer anyways, so it's safe.
; FIX (NOT!)
		movea.l	d2,a0			get current entry ptr back

; we saved the offset to the name earlier (in d5) use this to calculate the
; name pointer and store it in the buffer (assume name is always required)
		movea.l	d2,a1			get current address
		adda.w	d5,a1			calculate name address
		move.l	a1,ed_Name(a0)		and store in buffer
		lea.l	fle_FileName(a3),a0
		moveq	#0,d0
		move.b	(a0)+,d0		get length of name
		bra.s	91$
90$		move.b	(a0)+,(a1)+		transfer name as a C string
91$		dbra	d0,90$
		clr.b	(a1)+			must NULL terminate
		move.l	a1,d0			pad to next longword boundary
		addq.l	#3,d0
		andi.w	#$fffc,d0
		movea.l	d0,a1
	
; a1 is now pointing to the tentative address of the comment (if it is there)
; a0 points to the comment in the dircache entry (right after the filename)
		move.l	dp_Arg4(a2),d0
		cmpi.w	#ED_COMMENT,d0		will we need comment
		bcs.s	100$			nope, do the rest (d0<comment)
		move.l	a0,-(sp)		save addr of comment
		movea.l	d2,a0
		clr.l	ed_Comment(a0)		clear in case there's no
		move.l	(sp)+,a0		 comment
		tst.b	(a0)
		beq.s	100$			no comment

; Steve had a bug, and left ed_Comment a BSTR (ugh).  He documented it as
; a CSTR.  It's a really bad spot to be in...  We'll go with CSTR.
		move.l	a0,-(sp)		save addr of comment
		movea.l	d2,a0
		move.l	a1,ed_Comment(a0)
		move.l	(sp)+,a0		comment is right after name
		moveq	#0,d0
		move.b	(a0)+,d0		get length of comment
		bra.s	96$
95$		move.b	(a0)+,(a1)+		copy as CSTR!!!!!
96$		dbra	d0,95$
		clr.b	(a1)+			terminate - a1 is live!
		move.w	a1,d0
		and.w	#1,d0
		add.w	d0,a1			round up to next word!

; a1 will now be pointing to the address of the next entry.  Update bytes left.
100$		move.l	a1,d2			this is the next entry
		move.l	d2,d0			calculate bytes used
		sub.l	d4,d0			d4 is now current (and prev)
		sub.l	d0,d3			d3 holds bytes left

; all the variable length stuff has been done so now do the fixed length things
		movea.l	d4,a0			d4 is holding buffer address
		move.l	dp_Arg4(a2),d0

		cmpi.w	#ED_TYPE,d0		do we need type ?
		blt.s	CheckMatch		nope, only name
		move.b	fle_Type(a3),d1
		ext.w	d1
		ext.l	d1			type is a signed long!
		move.l	d1,ed_Type(a0)

; fetch the linked fHB if any...
		move.l	d2,-(sp)
		move.l	d0,d2			arg for GetLinkInfo
		moveq	#1,d1			dest is an ed_xxx
		move.l	a0,a1			dest is an ed_xxx
		moveq	#1,d0			src is an fle_xxx
		move.l	a3,a0			src is an fle_xxx
		bsr	GetLinkInfo		(a0,a1,d0,d1 - regs unchanged)
		move.l	(sp)+,d2

		; bra	CheckMatch  fall through

; We now have a completed entry in the callers buffer.  D4 is pointing to this
; entry, d6 to the previous and d2 to the next.  Before accepting this entry
; and going for the next one, see if we should do any pattern matching.
;
; Someone changed the interface.  If there is a matchstring then do the default
; pattern matching on it and then continue on to look for a matchfunc if the
; entry has been accepted (acts as a separate filter instead) !!!!!!!!!!!!!!!!!!
CheckMatch	movea.l	dp_Arg5(a2),a1		get control struct
		tst.l	c_MatchString(a1)	any match string
		beq.s	CheckFunction		nope, just look for a function

		movem.l	a2-a4,-(sp)
		lea.l	MyMatchFunc(pc),a3	default match routine
		movea.l	d4,a0			setup buffer data
		movea.l	c_MatchString(a1),a1	match string
		move.l	dp_Arg4(a2),d0		data value
;		move.l	a1,-(sp)		stack args too
;		move.l	a0,-(sp)
;		move.l	d0,-(sp)
		jsr	(a3)			call the function
;		adda.w	#12,sp			clean up the stack
		movem.l	(sp)+,a2-a4
		tst.l	d0			should we keep this entry ?
		bne.s	CheckFunction		yes, maybe a user function too

; the entry has been rejected so we're going to have to back up the pointers
Rejected	move.l	d2,d0
		sub.l	d4,d0			calculate space used
		add.l	d0,d3			and add back to available space
		move.l	d4,d2			make d2 current address
		move.l	d6,d4			and update previous address
		beq.s	10$			there was no previous entry
		movea.l	d4,a0
		clr.l	(a0)			has no next entry anymore
10$		movea.l	dp_Arg5(a2),a0
		subq.l	#1,c_Entries(a0)	one less entry
		bra.s	ExAllNext		go for the next one

; Maybe call a user supplied hook to do (further) pattern matching checks
CheckFunction	movea.l	dp_Arg5(a2),a0		get control struct
		move.l	c_MatchFunc(a0),d0	was there a match function ?
		beq.s	ExAllNext		nope, so all done
		movem.l	a2-a4,-(sp)		yes, must call it

		movea.l d0,a0                   the hook itself
		movea.l d4,a1                   point at buffer data
		lea     dp_Arg4(a2),a2          pointer to type ("data" longword)
		movea.l h_Entry(a0),a3          a0=hook, a1=exalldata ptr, 
		jsr     (a3)                     a2 = pointer to type ("data)

		movem.l	(sp)+,a2-a4
		tst.l	d0
		beq.s	Rejected		entry rejected

; Everything done for this entry.  Free the block and look for the next
ExAllNext	tst.b	ET_Type(a4)
		bne.s	10$
		; old type

;	printf <'dropping block $%lx (%ld)'>,a3,cb_Key(a3)
		move.l	d7,d0			associate with directory
		bsr	LooseA3Block		free current block
		bra	ExAllLoop		and look for the next block

		; DCFS
10$		bsr	NextDCEntry
		bra	ExAllLoop

; We ran out of space in the buffer so return TRUE to make caller call us again
ExAllNoFit	printf <'ExAll: not enough space'>
		bsr.s	FreeNETBlock
		clr.w	ErrorCode+2(a5)		nothing wrong
		moveq.l	#TRUE,d0		no problems
		movea.l	a2,a0
		bra	WorkDone		send packet back and die

; we have completed the scan of the directory, return FALSE and ERROR_NO_MORE
ExAllFitted	bsr.s	FreeNETBlock
		move.l	#ERROR_NO_MORE_ENTRIES,ErrorCode(a5)
		movea.l	a2,a0			fail this packet
		bra	WorkFail		won't come back, end of dir

MyMatchFunc	movem.l	d2/a6,-(sp)		save exec
		movea.l	DosLib(a5),a6		get DOS
		move.l	ed_Name(a0),d2		get the name
		move.l	a1,d1			args for MatchPattern(match,str)
		jsr	_LVOMatchPatternNoCase(a6)	get a match
		movem.l	(sp)+,d2/a6		restore exec
		rts

; Free NET block if held
FreeNETBlock:	tst.b	ET_Type(a4)
		beq.s	10$			not an NET
		move.l	NET_BlockPtr(a4),a0	the block buffer
		move.l	cb_SIZEOF+fhl_Parent(a0),d0	associate with itself
		tst.l	NET_BlockNum(a4)	block or 0
		beq.s	10$			NextDCEntry may have freed it!
;	printf <'2:loosing block %ld ($%lx)'>,d0,a0
		bsr	LooseBlock
		clr.l	NET_BlockNum(a4)	we no longer hold this block!
		; blockptr is invalid too, but we key off blocknum
10$		rts

		
		END
