		SECTION	filesystem,CODE
;==============================================================================
; General support routines to make the rest of the code a little bit smaller.
;==============================================================================

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/memory.i"
		INCLUDE	"exec/io.i"
		INCLUDE	"devices/timer.i"
		INCLUDE	"devices/trackdisk.i"
		INCLUDE	"libraries/dosextens.i"
		INCLUDE	"moredos.i"
		INCLUDE	"globals.i"
		INCLUDE	"private.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XREF	_LVOAllocMem,_LVOFreeMem,_LVODoIO,_LVOForbid,_LVOPermit
		XREF	WaitCo,DeleteCo,ResumeCo,Qpkt,_LVOSendIO,ForgetVolume
		XREF	ReleaseDir,ReleaseRename

		XDEF	GetPubMem,ClearBlock
		XDEF	Kill,Checksum
		XDEF	ReturnPkt,WorkFail,WorkDone
		XDEF	SplitName,Hash,CapitalChar
		XDEF	GetLock,CheckLock,FreeLock
		XDEF	IsDir,Type,CompString
		XDEF	DatStamp,TestDisk,SendTimer
		XDEF	DeleteFromET

;==============================================================================
; Mem = GetPubMem( size )
; d0		    d0
;
; Gets a block of memory of type MEMF_PUBLIC!MEMF_CLEAR. doesn't set cc for 0
;==============================================================================
GetPubMem	move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
		jmp	_LVOAllocMem(a6)

;==============================================================================
; ClearBlock( block )	-  clears BlockSize bytes of buffer storage
;		a0
;==============================================================================
ClearBlock	move.l	BlockSize(a5),d0	get size of a block
		lsr.w	#5,d0			clearing 8 longwords at a time
		bra.s	20$
10$		clr.l	(a0)+
		clr.l	(a0)+
		clr.l	(a0)+
		clr.l	(a0)+
		clr.l	(a0)+
		clr.l	(a0)+
		clr.l	(a0)+
		clr.l	(a0)+
20$		dbra	d0,10$
		rts

;==============================================================================
; Kill()   Allow other co-routines to commit suicide. Can only be ResumeCo'd
;==============================================================================
Kill		bsr	WaitCo			wait for routine to kill
		movea.l	d0,a0			into correct register
		bsr	DeleteCo		kill it
		bra.s	Kill			and wait again

;==============================================================================
; Sum = Checksum( data )
;  d0		   a0
;
; performs a checksum on a block of data that is BlockSize bytes long.  A lot
; of the checksumming code has been moved locally, so this may be redundant.
;==============================================================================
Checksum	move.l	BlockSize(a5),d1	get size of a block
		moveq.l	#0,d0			initial checksum value
		lsr.w	#5,d1			clearing 8 longwords at a time
		bra.s	20$
10$		add.l	(a0)+,d0		this makes it a bit faster
		add.l	(a0)+,d0
		add.l	(a0)+,d0
		add.l	(a0)+,d0
		add.l	(a0)+,d0
		add.l	(a0)+,d0
		add.l	(a0)+,d0
		add.l	(a0)+,d0
20$		dbra	d1,10$
		rts				return checksum in d0

;==============================================================================
; ReturnPkt( pkt, res1, res2 )
;	      a0   d0    d1
;
; Send a packet back to the caller after filling in dp_Res1 and dp_Res2.
;==============================================================================
ReturnPkt	cmpa.w	#0,a0			make sure there is a packet
		beq.s	NoPkt			nope, not there
		ext.l	d1			only words stashed in ErrorCode
		movem.l	d0/d1,dp_Res1(a0)	stash return codes
; printf <'returnpkt: packet $%lx(%ld), result %ld/%ld\n'>,a0,dp_Action(a0),dp_Res1(a0),dp_Res2(a0)
		bra	Qpkt
NoPkt		rts

;==============================================================================
; WorkFail( pkt )
;	    a0
; Sends back a packet with a failure code and kills current co-routine
;==============================================================================
WorkFail	moveq.l	#FALSE,d0			false return code

;==============================================================================
; WorkDone( pkt,res )
;	    a0  d0
; Sends back a packet with correct return code and errorcode (from the global)
; and then kills the co-routine responsible for this call.
;
; we release any dir modification locks we're holding before killing this
; coroutine.  This allows things to fail at any point safely.  Only rename
; can hold a second lock.  See ObtainDir docs for more info.
; Since we're going to kill ourselves, feel free to trash regs.
;==============================================================================
WorkDone	move.l	ErrorCode(a5),d1	get global error code
;	printf <'workdone: called with result %ld/%ld\n'>,d0,d1
		bsr.s	ReturnPkt
;	printf <'workdone: back from ReturnPkt\n'>

; if we own a dir, give it up.  Also see if we owned two, i.e. were renaming
		; release regular dir lock
		move.l	CurrentCo(a5),a2	who we are
		lea	co_dsem(a2),a0
		tst.l	dsem_Block(a0)
		beq.s	20$			didn't own a dirlock
		bsr	ReleaseDir		let others modify the dir
20$
		cmp.l	RenameCo(a5),a2		who, is anyone, is renaming
		bne.s	10$

		; we're a rename coroutine!
		lea	Rename_dsem(a5),a0	release rename dirlock
		tst.l	dsem_Block(a0)
		beq.s	5$			didn't actually own 2nd
		bsr	ReleaseDir
5$		bsr	ReleaseRename		tell others waiting to rename
						; to go ahead.  Clears RenameCo.
10$		movea.l	KillCo(a5),a0
		move.l	a2,d0			co-routine to kill
		bra	ResumeCo		bsr/rts

;==============================================================================
; offset = SplitName( prefix,char,string,ptr )
;   d0			a0    d0    a1   d1.w
;
; Basically, we search the given string starting at the offset in d1.  If we
; find the given character, then we return the offset to the character after.
; A maximum of 30 characters get copied to prefix after we've searched string.
; If the given character is not found then 0 is returned but a maximum of 30
; characters will still have been copied across to prefix.
;==============================================================================
SplitName	movem.l	d2-d3,-(sp)
		clr.w	d2			d2 is the search count
		moveq.l	#0,d3			d3 is the copy length
		clr.b	(a0)			no string copied yet
		move.b	(a1),d2			get length of string
		sub.w	d1,d2			calculate amount to search
		bmi.s	40$			didn't find it, return 0

		lea.l	0(a1,d1.w),a1		where to start searching
		move.w	d1,-(sp)		save current pointer
10$		move.b	(a1)+,d1		get the character
		addq.w	#1,d3			bump the pointer
		cmp.b	d1,d0			is this our character ?
		beq.s	30$			yep, string is copied
		cmpi.w	#30,d3			can we copy it ?
		bgt.s	20$			no, string is already 30 chars
		move.b	d1,0(a0,d3.w)		store in prefix
		move.b	d3,(a0)			and stash current length
20$		dbra	d2,10$			keep looking
		moveq.l	#0,d3			ran off end of string
		addq.l	#2,sp			scrap value on stack
		bra.s	40$			and all done

30$		add.w	(sp)+,d3		return new pointer
40$		move.l	d3,d0			return result in d0
		movem.l	(sp)+,d2-d3
		rts

;==============================================================================
; Lock = GetLock( key, accessmode )
;  d0		   d0	   d1
;
; returns a lock on the provided key if it's not already exclusively locked.
; NOTE:  the lock pointer returned is a BPTR for compatibility with WBench!
;==============================================================================
GetLock		movem.l	d2/d3,-(sp)
		clr.l	ErrorCode(a5)		assume no error
		move.l	d0,d2			save key
		move.l	d1,d3			save type of lock
		move.l	LockQueue(a5),d0	point to first entry
		beq.s	40$			no outstanding locks

10$		lsl.l	#2,d0			make into a machine ptr
		movea.l	d0,a0
		cmp.l	fl_Key(a0),d2		is this a duplicate key ?
		beq.s	20$			yes, check not exclusive
		move.l	(a0),d0			get the link
		bne.s	10$			and loop for another
		bra.s	40$			end of queue, lock not found

; we found a lock so check that our lock or the existing one are not exclusive
20$		moveq.l	#exclusive.lock,d0
		cmp.l	d0,d3
		beq.s	30$			error, can't get the lock
		cmp.l	fl_Access(a0),d0
		bne.s	40$			no problems

; someone has an exclusive lock on this key so return an object in use error
30$		move.w	#ERROR_OBJECT_IN_USE,d0
35$		move.w	d0,ErrorCode+2(a5)	global error value
		moveq.l	#0,d0			return false
		bra.s	60$

; nobody else has an exclusive lock on this key so we can create a new one
40$		moveq.l	#fl_SIZEOF+8,d0		*** allow for examinething ***
		bsr	GetPubMem		get the memory for the lock
		tst.l	d0
		bne.s	50$			got the memory OK
		moveq.l	#ERROR_NO_FREE_STORE,d0	set up an error return
		bra.s	35$			didn't get memory

; we got the memory for this lock so fill in all the important info
50$		movea.l	d0,a0
		move.l	d2,fl_Key(a0)
		move.l	d3,fl_Access(a0)
		move.l	ThisDevProc(a5),fl_Task(a0)
		move.l	CurrentVolume(a5),fl_Volume(a0)
		lsr.l	#2,d0			link is a BPTR, yuck!!
		move.l	LockQueue(a5),fl_Link(a0)
		move.l	d0,LockQueue(a5)

; if the lock was successfully manufactured then we return a BPTR address.
60$		movem.l	(sp)+,d2/d3
		rts

;==============================================================================
;  DeleteFromET( key, nexthash, parent )
; 		 d0	d1	   d2
;
; When we remove a file or directory, we must scan all active ExamineThings to
; make sure none of them are referencing this block, since it might get filled
; with random FFS data (which doesn't checksum well).  Also, if it has a next
; hashchain entry, insert it into the ExamineThing.  This only applies to
; ExamineThings, not NewExamineThings (i.e. FFS not DCFS).
;==============================================================================
DeleteFromET	movem.l	a2/d3/d4,-(sp)
		move.l	d0,d3			save key
		move.l	LockQueue(a5),d0	point to first entry
		beq.s	DET_exit		no outstanding locks

LockLoop	lsl.l	#2,d0			make into a machine ptr
		movea.l	d0,a0
		cmp.l	fl_Key(a0),d2		is this a lock on the same dir?
		bne.s	NextLock		no, get the next lock

; we found a lock on the dir so check if it has an active ExamineThing
20$		movea.l	fl_SIZEOF(a0),a1	is there an ET attached?
		cmp.w	#0,a1
		beq.s	NextLock		no examinething
		tst.b	ET_Type(a1)		is this an old-style ET?
		bne.s	NextLock		no, DCFS can handle it's own

; Ok, we have an old-style ExamineThing for the directory.  Remove the block
; being deleted and add the hash entry if any.
		movea.l	ET_CurrentKey(a1),a2
30$		cmp.l	(a2),d3
		beq.s	40$			found the entry being deleted
		tst.l	(a2)+			are we done?
		beq.s	NextLock		yes, we didn't find it
		bra.s	30$			continue

; we found the entry being deleted.  Remove it and insert the next hash entry.
; if we have a next-hash entry, all entries between ET_CurrentKey and the
; entry being deleted move up in memory.  If no next-hash, then all entries
; following the deleted one must move down.
40$
	printf <'Deleting block %ld from old ET $%lx, @ $%lx'>,d3,a1,a2
		tst.l	d1			next hash?
		bne.s	60$
	printf <'no hash'>

; no next-hash: move everything above down.
50$		move.l	4(a2),(a2)		I don't trust auto-inc here
		beq.s	NextLock		all done, hit 0 lock
		addq.w	#4,a2
		bra.s	50$

; we have a next-hash to insert: move everything below up, then insert.
60$		cmp.l	ET_CurrentKey(a1),a2
		beq.s	InsertHash		we're done
		move.l	-4(a2),(a2)		move previous entry up one
		subq.w	#4,a2
		bra.s	60$

; we must insert an extension key into our list of keys to be examined.
; sorted insert...
InsertHash
	printf <'has hash %ld'>,d1
		movea.l	ET_CurrentKey(a1),a2
70$		move.l	4(a2),d4		is next key empty
		beq.s	80$			yep, current pos is our slot
		cmp.l	d1,d4			are we less than next key
		blt.s	80$			yep, current pos is our slot
		move.l	d1,(a2)+		move next key down
		bra.s	70$			and keep looking
80$		move.l	d4,(a2)			save the extra key

NextLock	move.l	(a0),d0			get the link
		bne.s	LockLoop		and loop for another
;						end of queue, dir not found
DET_exit	movem.l	(sp)+,a2/d3/d4
		rts

;==============================================================================
; Success = FreeLock( lock )
;   d0			a0
;
; Given a BPTR address to a lock, frees this lock if it can be found on lists.
; If this is not a root lock of 0 then the ExamineList is searched to see if an
; ExamineThing is associated with this lock.  If so then it's freed.  Any
; ExamineThings associated with root locks are freed when the volume is forgot.
;==============================================================================
FreeLock	movem.l	d2/a2/a3,-(sp)
		moveq.l	#FALSE,d2		don't forget the volume
		movea.l	a0,a2			save the lock pointer
		cmpa.w	#0,a2			a lock of 0 is OK (root lock)
		beq	35$			so just return TRUE
		adda.l	a0,a0			convert from a BPTR
		adda.l	a0,a0
		move.l	fl_Volume(a0),a3	get the volnode too
		adda.l	a3,a3			convert vol ptr to APTR
		adda.l	a3,a3

		lea.l	LockQueue(a5),a0	search the lock list
10$		tst.l	(a0)			at the last entry ?
		beq.s	20$			yep, quit now
		cmpa.l	(a0),a2			is this our lock next ?
		beq.s	30$			yes, get ready to unlink
		movea.l	(a0),a0			link to the next one
		adda.l	a0,a0			make into a machine ptr
		adda.l	a0,a0
		bra.s	10$

; For hard disks, it is still possible for the volume to be logically removed
; if we receive an ACTION_INHIBIT so search the volnodes locklist for it.
20$		cmpi.l	#DLT_VOLUME,dl_Type(a3)
		bne.s	25$			sorry, can't find it
		lea.l	dl_LockList(a3),a0	point to lock list
		moveq.l	#TRUE,d2		forget volume if last lock
21$		tst.l	(a0)			any more locks
		beq.s	25$			nope, can't find the lock
		cmpa.l	(a0),a2			is this our lock ?
		beq.s	30$			yep, go free it
		movea.l	(a0),a0			link to the next one
		adda.l	a0,a0			make into a machine ptr
		adda.l	a0,a0
		bra.s	21$

25$		move.w	#ERROR_INVALID_LOCK,ErrorCode+2(a5)
		moveq.l	#FALSE,d0
		bra.s	40$			lock not found

; We have found our lock.  If it is the last one on a volumes LockList then
; we will eventually forget the volume as well as freeing the lock memory.
30$		adda.l	a2,a2			convert lock to mc ptr
		adda.l	a2,a2
		move.l	(a2),(a0)		unlink from the list

		clr.l	fl_Task(a2)		so won't bee freed again

		move.l	fl_SIZEOF(a2),d0	is there an examinething ?
		beq.s	34$			nope, skip it
		movea.l	d0,a1

		; figure out what type of examinething to free
		moveq	#NET_SIZEOF,d0		default to dos\4 ET
		tst.b	ET_Type(a1)
		bne.s	32$

		move.l	HTSize(a5),d0		get number of hashtable entries
		lsl.l	#2,d0			convert to byte size
		add.l	#4+ET_SIZEOF,d0		+ ending entry and currentkey
32$		jsr	_LVOFreeMem(a6)		free memory for examine thing

34$		movea.l	a2,a1			go free the memory
		moveq.l	#fl_SIZEOF+8,d0		*** allow for examinething ***
		jsr	_LVOFreeMem(a6)

		tst.l	d2			can we forget the volume
		beq.s	35$			nope, volume is active
		tst.l	dl_LockList(a3)
; bug fix, can only forget volumes if thier LockList is null.  LockQueue if for
; mounted volumes only.
;		or.l	LockQueue(a5),d0	any locks left
		bne.s	35$			yes, so just quit now
		move.l	a3,d0			convert back to BPTR
		lsr.l	#2,d0
		movea.l	d0,a0
		bsr	ForgetVolume		really will forget it now

35$		moveq.l	#TRUE,d0		no problems
40$		movem.l	(sp)+,d2/a2/a3
		rts

;==============================================================================
; key = CheckLock( lock )
;  d0		    a0
;
; checks that the given lock is valid and returns the key for that lock if it
; finds it OK.  0 return indicates failure and ErrorCode will be set up.
; NOTE: the lock value is actually a BPTR when sent to this routine.
;==============================================================================
CheckLock	cmpa.w	#0,a0			did we really get a lock ?
		bne.s	Lock2Check		yes, go check it
		move.l	RootKey(a5),d0		assume we have the root lock
		move.l	DiskType(a5),d1		stash the disk type
		eori.l	#ID_DOS_DISK,d1		leaves only low byte value
		tst.b	d1
		bmi.s	LockFailed		d1 < 0, failure
		cmp.b	#5,d1
		bgt.s	LockFailed		d1 > 5, failure (up to dos\5)
		clr.b	d1			drop low byte
		tst.l	d1			all the rest should be 0!
		beq.s	LockChecked		it was
LockFailed
		moveq.l	#FALSE,d0		we will fail now
		move.w	#ERROR_NO_DISK,ErrorCode+2(a5)	assume no disk present
		cmpi.l	#ID_NO_DISK_PRESENT,DiskType(a5)
		beq.s	LockChecked		there was no disk there
		move.w	#ERROR_NOT_A_DOS_DISK,ErrorCode+2(a5)
		bra.s	LockChecked

; we do have a lock pointer, check that the required volume is still mounted.
Lock2Check	adda.l	a0,a0			convert lock BPTR to mc ptr
		adda.l	a0,a0
		move.l	LockQueue(a5),d0	search queues for lock
		beq.s	20$			invalid lock or no device

10$		lsl.l	#2,d0			convert to mc pointer
		cmpa.l	d0,a0			is the lock here ?
		beq.s	30$			yep, we can return key
		movea.l	d0,a1
		move.l	(a1),d0
		bne.s	10$			go for next lock

; we didn't find the lock we wanted so check what kind of error to return
20$		move.l	fl_Volume(a0),a1	get the volume pointer
		adda.l	a1,a1			convert to machine ptr too
		adda.l	a1,a1			a1 points to DevList struct
		moveq.l	#FALSE,d0		we have failed already
		move.w	#ERROR_DEVICE_NOT_MOUNTED,ErrorCode+2(a5)
		cmpi.l	#DLT_VOLUME,dl_Type(a1)	is it a volume ?
		beq.s	LockChecked		yes, error set up OK
		move.w	#ERROR_INVALID_LOCK,ErrorCode+2(a5)
		bra.s	LockChecked

;we found our lock so just return the key that is already stored in it
30$		move.l	fl_Key(a0),d0
LockChecked	rts				key or FALSE in d0

;==============================================================================
; value = Hash( name )
;  d0		 a0
;
; returns the hash value (offset into header block) for the given name.  The
; BCPL version returned the offset in longwords, I have changed my version to
; return the physical byte offset into the header block.
;==============================================================================
Hash		move.l	d2,-(sp)
		moveq.l	#0,d0			so we can use words later
		moveq.l	#0,d1			hash value
		move.b	(a0)+,d1		initial hash is string length
		move.w	d1,d2
		bra.s	15$			fix d2 for dbra

10$		mulu.w	#13,d1			res = res * 13 ...
		move.b	(a0)+,d0
		bsr.s	CapitalChar
		add.w	d0,d1			... +CapitalChar( char ) ...
		andi.w	#$7ff,d1		... & $7ff
15$		dbra	d2,10$

		divu.w	HTSize+2(a5),d1		hashtable size in longwords
		swap	d1			get remainder
		lsl.w	#2,d1			convert into byte offset
		ext.l	d1			return a longword result
		moveq.l	#udb_HashTable,d0	offset to hash table
		add.l	d1,d0			offset to name slot
		move.l	(sp)+,d2
		rts

;==============================================================================
; Char = CapitalChar( char )
;  d0		       d0
;
; converts lower case characters to upper case and leaves all others alone.
; NEVER trashes any registers (except d0 of course), this can be relied on!!!
; Leave this here so Hash and CompString can call it with bsr.s
;
; Added extra fixes for international characters here.  If disk is DOS/2 or /3
; we do a special international character UpperCase routine instead.  Lifted
; the code (and slightly modified to be less general) from utility.library.
;
; dos\2 - dos\5 support interational types
;==============================================================================
CapitalChar	btst.b	#1,DiskType+3(a5)	supporting international ?
		bne.s	international		yup
		btst.b	#2,DiskType+3(a5)	fast (dos\4 & 5) always intern
		beq.s	OldRoutine		nope, do old version
international
		movem.l	d1/d2,-(sp)
		move.l	d0,d2			save character
		move.l	#'~`za',d1		load up 4 characters for compares
		cmp.b	#$f7,d0			these are special
		beq.s	IsSpecial
;						Z/z and A/a in low 16 bits
		bclr.b	#7,d0			clear high bit
		beq.s	normal_char
		swap	d1			^/~ and @/` are the highest in the upper area
;						(~ = ~, ' = ` minus bit 7)
normal_char	cmp.b	d1,d0			a or `(`)  (or A or @)
		blt.s	IsSpecial
		lsr.l	#8,d1			get upper bound
		cmp.b	d1,d0			z or ~(~)  (or Z or ^)
		bgt.s	IsSpecial
		addi.b	#'A'-'a',d2		+ or - ('a'-'A')
IsSpecial	move.b	d2,d0			make sure the return is good
		movem.l	(sp)+,d1/d2
		rts

OldRoutine	cmpi.b	#'a',d0			check against 'a'
		blt.s	10$
		cmpi.b	#'z',d0			check against 'z'
		bgt.s	10$
		addi.b	#'A'-'a',d0		adjust to upper case
10$		rts

;==============================================================================
; Result = Compstring( string1, string2 )
;   d0			 a0	   a1
;
; The fast filing system only needs to know if 2 strings are equal so I've used
; this version of CompString which is a little quicker than the original.
;==============================================================================
CompString	move.w	d2,-(sp)
		moveq.l	#0,d0
		move.b	(a0)+,d0
		cmp.b	(a1)+,d0
		bne.s	CompNotSame		not equal, return now
		move.w	d0,d1
		bra.s	20$

10$		move.b	(a0)+,d0		get next character
		bsr.s	CapitalChar		make capital
		move.b	d0,d2
		move.b	(a1)+,d0
		bsr.s	CapitalChar		make capital
		cmp.b	d0,d2
		bne.s	CompNotSame		not the same, quit now
20$		dbra	d1,10$
		moveq.l	#0,d0			strings compare OK
		bra.s	CompStrDone
CompNotSame	moveq.l	#TRUE,d0		strings don't compare
CompStrDone	move.w	(sp)+,d2
		rts

;==============================================================================
; result = IsDir(buffer)
;   d0		   a0
;
; Returns a boolean indicating whether the given block contains a valid
; directory header block.  Sets the global error code to indicate it wasn't.
;
; Guaranteed not to modify a0! (work.asm uses this)
;==============================================================================
IsDir		move.w	#ERROR_OBJECT_WRONG_TYPE,ErrorCode+2(a5)  unconditional!
		moveq.l	#FALSE,d0		assume bad
		moveq.l	#t.short,d1
		cmp.l	cb_SIZEOF+udb_Type(a0),d1
		bne.s	10$			it was bad
		lea.l	cb_SIZEOF(a0),a1	point at end of block
		adda.l	BlockSize(a5),a1
		tst.l	vudb_SecType(a1)
		ble.s	10$			secondary type <=0 is bad
		moveq.l	#TRUE,d0		everything OK
10$		rts

;==============================================================================
; type = Type( buffer )
;  d0		a0
;
; Returns the type of header block in a buffer. Possible types returned are;
; t.deleted,t.long,t.data,t.list,st.userdir,st.file or ST_xLINK. If an error
; during read then the whole buffer will have been cleared to 0 and this will
; be the result returned.
;==============================================================================
Type		move.l	cb_SIZEOF+udb_Type(a0),d0
		cmpi.w	#t.short,d0		is it t.short
		bne.s	10$			nope, so thats the type
		lea.l	cb_SIZEOF(a0),a1	point at end of block
		adda.l	BlockSize(a5),a1
		move.l	vudb_SecType(a1),d0
		cmpi.w	#st.root,d0		root shows as a userdir
		bne.s	10$
		moveq.l	#st.userdir,d0
10$		rts

;==============================================================================
; DatStamp( buffer )
;	      a0
;
; Fills the three longwords in buffer (a DateStamp struct) with the current
; system time and also updates the RootNode to reflect this time.  This has
; been optimised to work with the 68000 mul and div instructions without
; having to go to 32 bit math.  It breaks around 2020 I think!
;==============================================================================
DatStamp	movem.l	d2-d4/a2,-(sp)
		movea.l	a0,a2			stash the buffer address
		movea.l	TimerExtraDev(a5),a1	send a getsystime request
		move.w	#TR_GETSYSTIME,IO_COMMAND(a1)
		move.b	#IOF_QUICK,IO_FLAGS(a1)
		jsr	_LVODoIO(a6)

		movea.l	TimerExtraDev(a5),a1	find what the time was
		movem.l	IOTV_TIME+TV_SECS(a1),d0/d1
		moveq.l	#0,d2			days = 0
		moveq.l	#0,d3			mins = 0
		moveq.l	#0,d4			ticks = 0
		divu.w	#43200,d0		half days = secs/43200
		move.w	d0,d2			d2 = #half days
		clr.w	d0
		swap	d0
		lsr.w	#1,d2			d2 = days
		bcc.s	10$			no extra half day
		addi.l	#43200,d0		d0 = remaining seconds
10$		divu.w	#60,d0			mins = secs/60
		move.w	d0,d3			d3 = mins
		swap	d0			d0 = remaining seconds
		divu.w	#20000,d1		d1 = odd ticks
		move.w	d1,d4
		mulu.w	#50,d0			convert seconds to ticks
		add.l	d0,d4			d4 = total ticks
		movem.l	d2/d3/d4,(a2)		save days,mins,ticks
		movea.l	DosLib(a5),a0		stash time in rootnode too
		movea.l	dl_Root(a0),a0
		movem.l	d2/d3/d4,rn_Time(a0)	save days,mins,ticks

		movem.l	(sp)+,d2-d4/a2
		rts

;==============================================================================
; Result = TestDisk( command, data, length )
;   d0			d0     a0     d1
;
; Performs the specified command and returns the result in d0.  Used mainly
; for protstatus, changenum, clear and motor commands.  ie. Quick things.
;==============================================================================
TestDisk	movea.l	DiskExtraDev(a5),a1	get spare IO request
		move.w	d0,IO_COMMAND(a1)
		move.l	a0,IO_DATA(a1)
		move.l	d1,IO_LENGTH(a1)
		jsr	_LVODoIO(a6)
		movea.l	DiskExtraDev(a5),a1
		move.l	IO_ACTUAL(a1),d0
		rts

;==============================================================================
; SendTimer()   sends a timer request that will appear back at the message port
;		one second from now as a packet of type ACTION_TIMER.
;==============================================================================
SendTimer	moveq.l	#-1,d0
		move.w	d0,TimerRunning(a5)	flag that request is pending
		movea.l	TimerDev(a5),a1
		movea.l	TimerPacket(a5),a0
		move.w	#ACTION_TIMER,dp_Type+2(a0)
		move.l	a0,LN_NAME(a1)		link packet to IORequest
		move.w	#TR_ADDREQUEST,IO_COMMAND(a1)
		move.l	#1,IOTV_TIME+TV_SECS(a1)	returns in one second
		clr.l	IOTV_TIME+TV_MICRO(a1)
		jmp	_LVOSendIO(a6)

		END
