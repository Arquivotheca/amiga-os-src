		SECTION filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"libraries/dosextens.i"
		INCLUDE	"devices/trackdisk.i"
		INCLUDE	"actions.i"
		INCLUDE	"moredos.i"
		INCLUDE	"globals.i"
		INCLUDE	"private.i"
		INCLUDE	"readwrite.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XREF	FreeBlock,ReadBlock,SplitName,WorkFail,WorkDone
		XREF	GetLock,CheckLock,CompString,Hash,IsDir,WriteBlock
		XREF	ClearBlock,DatStamp,AlterRoot,GetBlock,AllocKeys
		XREF	CapitalChar,FreeLock,FreeKeys,Type,CheckNotify
		XREF	GetPubMem,_LVOFreeMem,ValidatorError
		XREF	WaitFor,AllocListKey,ObtainDir,ReleaseDir
		XREF	ObtainRename,ReleaseRename
		XREF	DeleteFromET

		XDEF	DiscardBlock,LooseBlock,GrabBlock,WaitBlock
		XDEF	GrabBlockA3,GrabBlockA4
		XDEF	Locate,VLock,FindDir,FindEntry,DiskProtected
		XDEF	Parent,GrabDataBlock,WaitDataBlock,Create,Delete
		XDEF	CheckName,Rename,Comment,ParentKey
		XDEF	WriteBlockLater,WriteBlockNow,LooseHeaderBlock
		XDEF	ClearFile,UpdateList,UpdateListIfDifferent
		XDEF	NextListEntry,InsertListMissing,LooseA4HeaderBlock
		XDEF	WriteSummedBlockD6A0Later_RWD,CopyOverlap
		XDEF	WriteSummedBlockA0Later_RWD,LooseA3Block,LooseKeyBlock

		XDEF	UserLocate,UserCreate

	IFD SOFTLINKS_SUPPORTED
		XDEF	ReadLink
	ENDC

;==============================================================================
; DiscardBlock( buffer )  throw away a buffer that is no longer valid.
;		  a0
;==============================================================================
DiscardBlock	moveq.l	#BUFF_INVALID,d0	not associated with any header
		bra	FreeBlock

;==============================================================================
; LooseBlock( buffer, header )  - throw away a buffer that is still valid
;		a0      d0
;
; If the block was being written but yanked from Waiting or Pending queues
; first, it will go back on the waiting queue when we call FreeBlock.
;==============================================================================
LooseA4HeaderBlock
		move.l	a4,a0			block to release
LooseHeaderBlock
		move.l	rwd_FileHeader(a3),d0	associate with file header
		bra.s	LooseBlock

LooseA3Block	move.l	a3,a0			block to release
		bra.s	LooseBlock

LooseKeyBlock	move.l	cb_Key(a0),d0		associate with itself

LooseBlock	; printf	<'L:%ld $%lx'>,cb_Key(a0),a0
		move.l	d0,cb_Header(a0)	associate with this header
		moveq.l	#BUFF_VALID,d0
		bra	FreeBlock

;==============================================================================
; buffer = GrabBlock( key )
;   d0,a0	      d0
;
; Get the required block NOW and yank it off a pending/waiting queue if need be
;==============================================================================
GrabBlock	; printf	<'Grabbing block %ld\n'>,d0
		move.l	d2,-(sp)
		moveq.l	#TRUE,d2		it's a file header
GrabCommon
		moveq.l	#GRAB,d1		grabbit true
ReadCommon
		bsr	ReadBlock		go read it
		move.l	(sp)+,d2
;	IFD DEBUG_CODE
;	move.l d0,a0
;	printf	<'G:%ld $%lx'>,cb_Key(a0),a0
;	ENDC
		move.l	d0,a0		return in a0 as well for convenience
		rts			buffer in d0 (or 0)

GrabBlockA3	; printf	<'Grabbing block %ld\n'>,d0
		bsr	GrabBlock
		move.l	d0,a3
		rts
GrabBlockA4	; printf	<'Grabbing block %ld\n'>,d0
		bsr	GrabBlock
		move.l	d0,a4
		rts

;==============================================================================
; buffer = GrabDataBlock( key )
;   d0		           d0
;
; Get the required block NOW and yank it off a pending/waiting queue if need be
; This is a Data block that contains no special info, so don't checksum it if
; we are running under FFS format.  If OFS then we will checksum it anyway.
;==============================================================================
GrabDataBlock	move.l	d2,-(sp)
		moveq.l	#FALSE,d2		it's a data block
		bra.s	GrabCommon

;==============================================================================
; buffer = WaitBlock( key )
;   d0		       d0
;
; get the required block but wait for it if it is in a pending or waiting queue
;==============================================================================
WaitBlock	; printf	<'Waiting for block %ld\n'>,d0
		move.l	d2,-(sp)
		moveq.l	#TRUE,d2		its a file header block
WaitCommon
		moveq.l	#WAIT,d1		don't grab it
		bra.s	ReadCommon

;==============================================================================
; buffer = WaitDataBlock( key )
;   d0		          d0
;
; get the required block but wait for it if it is in a pending or waiting queue
; This will be a data block with no special info so don't checksum it if we're
; running under FFS mode.  In OFS mode we will checksum it anyway.
;==============================================================================
WaitDataBlock	move.l	d2,-(sp)
		moveq.l	#FALSE,d2		its a data
		bra.s	WaitCommon

;==============================================================================
; WriteBlockLater( buffer,key,type )
;		     a0   d0   d1
;
; Appends a cache buffer to the Waiting queue so it can be written out later
; or snatched off the queue if it's needed before then.  Type refers to the
; cb_Size field as looked at by the disk co-routine (-2,-1,0 or >0)
;==============================================================================
; various variants of WBL to save space

WriteSummedBlockD6A0Later_RWD:
		move.l	d6,d0
WriteSummedBlockA0Later_RWD:
		move.l	rwd_FileHeader(a3),cb_Header(a0)
		bra.s	WriteSummedBlockLater
WriteSummedBlockD5A0Later:
		move.l	d5,d0
		bra.s	WriteSummedBlockLater
WriteSummedBlockD4A0Later:
		move.l	d4,d0
		bra.s	WriteSummedBlockLater
WriteSummedBlockKeyA0Later:
		move.l	cb_Key(a0),d0
		bra.s	WriteSummedBlockLater
WriteSummedBlockD4A3Later:
		move.l	d4,d0
WriteSummedBlockA3Later:
		move.l	a3,a0
WriteSummedBlockLater:
		moveq.l	#TRUE,d1
WriteBlockLater ; printf	<'WL:%ld $%lx type %ld'>,d0,a0,d1
		move.l	d2,-(sp)
		moveq.l	#TRUE,d2		goes on waiting queue
WriteCommon
		exg.l	d1,d2			save block type, flag into d1
		bsr	WriteBlock
		move.l	(sp)+,d2
		rts

;==============================================================================
; WriteBlockNow( buffer,key,type )
;		   a0   d0   d1
;
; Appends a cache buffer to the Pending queue so it can be written out now.
;==============================================================================
WriteBlockNow	; printf	<'WN:%ld $%lx type %ld\n'>,d0,a0,d1
		move.l	d2,-(sp)
		moveq.l	#FALSE,d2		goes on pending queue
		bra.s	WriteCommon

;==============================================================================
; UserLocate( packet )
;		d0
;
; Implements the ACTION_LOCATE_OBJECT call for locking files and directories.
; This is run as a co-routine so there's no need to save registers on entry.
;==============================================================================
UserLocate	movea.l	d0,a2			stash packet address
		movea.l	dp_Arg1(a2),a0		a0 = lock (current position)
		movea.l	dp_Arg2(a2),a1		a1 = name (string)
		adda.l	a1,a1			convert to APTR
		adda.l	a1,a1
		move.l	dp_Arg3(a2),d0		d0 = user defined locktype
		move.l	a2,d1			just for Locate
		moveq	#FALSE,d2		not creating, don't lock dir
		bsr.s	Locate			may come back (d0=lock)
		movea.l	a2,a0			all finished
		tst.l	d0
		bne	WorkDone		send back packet and die
		bra	WorkFail		return errorcode

;==============================================================================
; lock = Locate( lock,name,mode,packet,creating)
;  d0		  a0   a1   d0    d1	  d2
;
; Returns a lock on an existing directory or file.  Must be run as a co-routine
; so it can use the cache buffer routines to read directory and file headers.
; Returns a NULL pointer if anything went wrong (check ErrorCode for details).
;
; If the required object turns out to be a soft link this routine will return
; ERROR_SOFT_LINK in the ErrorCode global.  This code can commit suicided on
; behalf of the calling co-routine if any directory in the path can't be found.
; This is the reason for needing the packet argument (used for WorkFail).  If
; the simple filename is not found then the caller will get a NULL return and
; d1 will contain the key of the owning directory (only used by Create)
;==============================================================================
Locate		movem.l	d3-d5/a2-a4,-(sp)	not always a pure co-routine
		movea.l	a0,a3			stash current directory lock
		movea.l	a1,a4			stash given path or filename
		move.l	d0,d5			stash required lock type
		movea.l	d1,a2			stash the packet address

; gets to here with (a3) pointing to the current position lock, a4 pointing to
; the filename and d5 containing the lock type we want.  Verify the lock we are
; given and wind everything up with workfail if it's bad, else split off the
; device name if it exists.  d3 will contain the key of the dir lock we want
; after we have called finddir (or not called it if path is only : as in cd :)
		movea.l	a3,a0			current dir lock, leave as BPTR
		bsr	CheckLock		see if it's valid
		move.l	d0,d3
		bne.s	5$			yes, carry on
		movea.l	a2,a0			nope, kill this co-routine
		bra	WorkFail

; if the name we are given is just #?: then we use current lock as default key
5$		lea.l	-32(sp),sp		make space for name prefix
		movea.l	sp,a0			prefix space
		movea.l	a4,a1			string we are searching
		moveq.l	#':',d0			what we're searching for
		moveq.l	#1,d1			where to start searching
		bsr	SplitName
		subq.w	#1,d0			allowing for start pos...
		cmp.b	(a4),d0			...is it the same length ?
		beq.s	10$			yes, so use current lock

; Our current position lock will not be used because we were given a full path
; name.  We must go find this filename relative to the current position.
; (sp) can only be '\0' if the path ends in a '/'.
		move.l	d3,d0			assumed key (start key anyway)
		movea.l	a4,a0			name string
		movea.l	sp,a1			workspace
		bsr	FindDir			a2=packet on this call
		move.l	d0,d3			save returned key

; do we need to lock the directory before scanning it? (Create, Delete, Rename)
		tst.l	d2			boolean
		beq.s	7$
		bsr	ObtainDir		critical section starting
		move.l	d3,d0			get block # back again

7$		tst.b	(sp)			do we have a simple name left
		beq.s	10$			no, it's a directory lock

		move.l	d3,d0			find entry starting at this key
		movea.l	sp,a0			this name
		moveq.l	#TRUE,d1		follow links
		bsr	FindEntry
		move.l	d3,d1			stash owning directory key
		move.l	ObjKey(a5),d3		assumed new default key
		tst.l	d0			did it find it ?
		bne.s	10$			yes
		lea.l	32(sp),sp		no, free up stack workspace
		bra.s	LocateDone		and return NULL pointer

; by this stage our default key (d3) is set up pointing to the header block
; that we want.  Try to get a lock on this header key.
10$		lea.l	32(sp),sp		finished with stack workspace
		move.l	d3,d0			key we want
		move.l	d5,d1			type of lock we want
		bsr	GetLock			get this lock (returned in d0)
LocateDone	movem.l	(sp)+,d3-d5/a2-a4
		rts

;==============================================================================
; UserCreate( packet )
;		d0
;
; Implements the ACTION_CREATE_DIR and ACTION_MAKE_LINK packets.  This is run
; as a co-routine so there's no need to save registers on entry.
;==============================================================================
UserCreate	movea.l	d0,a2			stash packet address
		movea.l	dp_Arg1(a2),a0		a0 = lock (current position)
		movea.l	dp_Arg2(a2),a1		a1 = name (string)
		adda.l	a1,a1			convert to APTR
		adda.l	a1,a1
		moveq.l	#st.userdir,d2		assume making a directory
		move.l	dp_Type(a2),d0
		cmpi.w	#ACTION_MAKE_LINK,d0
		bne.s	10$
		moveq.l	#st.link,d2		wrong, making a link
10$		moveq.l	#EXCLUSIVE_LOCK,d0	want an exclusive lock
		move.l	a2,d1			packet to d1
		bsr.s	Create			may come back (d0=lock)
		movea.l	a2,a0			all finished
		bra	WorkDone		send back packet and die

;==============================================================================
; lock = Create( lock,name,mode,packet,type )
;  d0		  a0   a1   d0    d1	d2
;
; Capable of creating all different kinds of header blocks as determined by the
; type argument.  This corresponds directly with the secondary type fields of
; the header blocks.  Various actions are taken depending on the header type
; being created and the access mode required for the lock.
; 
; Header Type	 Action
;-------------	--------
; st.file	Attempt to find the given header.  If it's a directory then
;		fail (even if the directory is empty).  If it's a file and the
;		required lock type is shared or the file is write protected
;		just return the lock from locate.  If the header doesn't exist
;		then create a new one and return the required lock on it.
;
; st.userdir	Attempt to find the given header using an exclusive lock.  If
;		it exists in any form then UnLock it and fail the request.  If
;		it doesn't exist then create a new header and return an
;		exclusive lock on it.  Exclusive lock sent in mode argument.
;
;		New for cached dirs (dirlists): All directories MUST have
;		at least one DirList block.
;
; st.link	Attempt to find the given header using an exclusive lock.  If
;		it exists in any form then UnLock it and fail the request.  If
;		it doesn't exist then create a new header and return an
;		exclusive lock on it.  The type will be modified to either
;		ST_DLINK or ST_FLINK depending on what was linked to.  This
;		mode uses extra information out of the packet.  Specifically
;		dp_Arg3 contains a lock or string and dp_Arg4 is softlink flag
;==============================================================================
Create		movem.l	d2-d7/a2-a4,-(sp)

		movem.l	d0-d1/a0-a1,-(sp)	save args
		bsr	CheckLock		is disk in and validated ?
		tst.l	d0
		bne.s	5$			yes
		movem.l	(sp)+,d0-d1/a0-a1
		movea.l	d1,a0
		bra	WorkFail		nope, disk not validated etc.

5$		bsr	DiskProtected		can we write to the disk ?
		move.l	d0,d4			save result
		movem.l	(sp)+,d0-d1/a0-a1
		tst.l	d4
		beq.s	10$			we can write OK
		movea.l	d1,a0			disk is protected
		bra	WorkFail		so wind up now

; We know we can write to the disk OK, Locate will validate the lock for us.
; If anything is seriously wrong (lock bad or path incomplete) then Locate
; will kill this co-routine.  If the object doesn't exist then we will just
; get a NULL return and can carry on with the rest of the co-routine.
; It will grab a lock on the directory so it doesn't change from under us.
; If we die (WorkFail), the lock will be released automatically.  Otherwise,
; we need to release it on success before exiting.
10$		move.l	d0,d3			save lock mode
		movea.l	d1,a2			save packet
		movea.l	a0,a3			save given directory lock
		movea.l	a1,a4			save given name
		move.l	d2,d6			save type
		moveq	#TRUE,d2		creating, lock dir

		bsr	Locate			see if the object exists
		move.l	d6,d2			restore type
		movea.l	d0,a0
		move.l	a0,d5			did we get it ?
		bne.s	20$			yes, see if it's OK
		move.l	ErrorCode(a5),d0	it could be a soft link
		cmpi.w	#ERROR_SOFT_LINK,d0	if so then error out
		beq.s	20$
		cmpi.w	#ERROR_OBJECT_IN_USE,d0	is someone else using it ?
		bne	ReallyCreate		nope, so d1=owning directory
		bra.s	CreateObjError		yes, so return the error (in d0)

; We have a lock on the required object.  The only case where this is not an
; error condition is if we're creating a file using a shared or exclusive
; lock and it's a file that we've found.   All other combinations should fail
; with an ERROR_OBJECT_EXISTS.  If we're trying for an exclusive lock on a
; file then this is a MODE_NEWFILE and we should check file protection too.
20$		moveq.l	#st.file,d0
		cmp.l	d2,d0			are we making a file ?
		bne.s	CreateObjExists		nope, so error out now
		adda.l	a0,a0
		adda.l	a0,a0
		move.l	fl_Key(a0),d0		fetch the header
		move.l	d0,-(sp)		save header number
		bsr	GrabBlock
;		movea.l	d0,a0			buffer addr (used for LooseBlock)
		lea.l	cb_SIZEOF(a0),a1
		adda.l	BlockSize(a5),a1
		move.l	vfhb_SecType(a1),d6	stash secondary type
		move.l	vfhb_Protect(a1),d7	and protection bits
		move.l	(sp)+,d0
		bsr	LooseBlock		finished with header (uses a0!)
		moveq.l	#st.file,d0		check that it's a file
		cmp.l	d6,d0
		bne.s	CreateObjExists		it's not, error out now
		moveq.l	#SHARED_LOCK,d1		check what lock type we want
		cmp.l	d3,d1			hopefully we want shared
		beq.s	CreateGotObj		we do, return the lock

; We want an exclusive lock which means we have to empty the file first (it's
; come from a MODE_NEWFILE/FIND_OUTPUT call).  If the file is delete or write
; protected, then we can't do it and should fail this call.
		move.w	#ERROR_WRITE_PROTECTED,d0
		btst.l	#2,d7			is file write protected
		bne.s	CreateObjError		yep, can't do newfile
		move.w	#ERROR_DELETE_PROTECTED,d0
		btst.l	#0,d7			is file delete protected
		bne.s	CreateObjError		yep, we can't delete it

; we've opened a file MODE_NEWFILE so we have to clear out all the data
		movea.l	d5,a0			get the header key
		adda.l	a0,a0
		adda.l	a0,a0
		move.l	fl_Key(a0),d0
		moveq.l	#0,d1			clear from beginning
		bsr	ClearFile		kill all data (and change date)
		bra.s	CreateGotObj		and update owning directory

; The object we locked cannot be overwritten by a new header, return an error
CreateObjExists	move.w	#ERROR_OBJECT_EXISTS,d0
CreateObjError	move.w	d0,ErrorCode+2(a5)
		movea.l	d5,a0			free up this lock
		bsr	FreeLock
ReallyError	movea.l	a2,a0			replying this packet
		bra	WorkFail		wind up the co-routine now

; We managed to legally lock an existing object during create.  This makes it
; possible to write to files that are linked (because scratch wouldn't allow
; thier deletion).  Even though we haven't written anything yet, we should
; find the parent directory and mark it as altered (change date and archive).
; D5 holds the lock on the header we just found.  Use it to find parent dir.
CreateGotObj	bsr	AlterRoot		things are changing
		movea.l	d5,a0			fetch our header key
		adda.l	a0,a0
		adda.l	a0,a0
		move.l	fl_Key(a0),d0
		bsr	ParentKey		get the parent directory key
		move.l	d0,d2			stash it for later
		bsr	GrabBlock		and read it in now
;		movea.l	d0,a0			buffer to a0
		move.l	d2,d0			parent key
		move.l	d2,d1			same as object key
		bsr	DirStamp		DateStamp and write back
		move.l	d5,d0			return our lock
		bra	CreateExit

; The object we are creating doesn't exist in the given directory (though the
; directory must have existed or Locate wouldn't have returned).  D1 is holding
; the key of the required directory.  All we need to do now is split off the
; tail part of the path to find the name of the object we want to create.
; Must check that owning directory key really is a directory first.
ReallyCreate	move.l	d1,d4			stash owning directory key

; check we really have a directory and return an error if it isn't
		move.l	d1,d0
		bsr	GrabBlock
;		movea.l	d0,a0
		bsr	IsDir			preserves a0!
		move.l	d0,-(sp)		save return code
		move.l	d4,d0			associate with self
		bsr	LooseBlock		done with block (a0)
		move.l	(sp)+,d0		get directory return code
		beq	ReallyError		it wasn`t a directory

; got a directory, now split off the simple name for creation
		lea.l	-32(sp),sp		get workspace for simple name
		movea.l	sp,a0			using this workspace
		movea.l	a4,a1			this is the given string
		moveq.l	#':',d0			looking for a colon
		moveq.l	#1,d1			start looking at this position
		bsr	SplitName
		tst.w	d0			was there a colon?
		bne.s	1$			yes
		moveq.l	#1,d0			can't have pointer value of 0

; keep searching the string until we don't find a backslash.  At this point
; we will have the simple filename left in the workspace on the stack.
1$		move.l	d0,d1			new search point
		moveq.l	#'/',d0			what we're searching for
		movea.l	a4,a1			searching in this string
		movea.l	sp,a0			deposit results here
		bsr	SplitName
		tst.w	d0			did we find one ?
		bne.s	1$			yes, keep searching

; We have a valid directory to put the object into.  Check object name is OK.
		movea.l	sp,a0			see if remaining name is OK
		bsr	CheckName
		tst.l	d0
		beq	ReallyError		it wasn't, kill the co-routine

; If we're making a hard link don't allow links to the root directory
		cmpi.w	#st.link,d2		is this a link header
		bne.s	20$			no problem, not a link call
		tst.l	dp_Arg4(a2)		if soft link, don't check
		bne.s	20$
		movea.l	dp_Arg3(a2),a0		fetch object we are linking to
		adda.l	a0,a0
		adda.l	a0,a0
		cmpa.w	#0,a0
		beq.s	10$			root lock, not allowed
		move.l	fl_Volume(a0),d0
		cmp.l	CurrentVolume(a5),d0	must be same volume too
		bne.s	10$
		move.l	fl_Key(a0),d0
		cmp.l	RootKey(a5),d0		can't link to the root either
		bne.s	20$
10$		move.w	#ERROR_OBJECT_WRONG_TYPE,ErrorCode+2(a5)
		movea.l	a2,a0
		bra	WorkFail		drop out

; Everything checked out OK.  Attempt to allocate a disk block for the header.
20$		move.l	d4,d0			want to be near here
		moveq.l	#1,d1			only want one block
		bsr	AllocKeys
		move.l	d0,d5			save object key
		bne.s	gotkey			got it OK

; couldn't get a disk block, store d1 (error return) into ErrorCode global
		move.l	d1,ErrorCode(a5)	return error
		bra	ReallyError		didn't get one, kill co-routine

gotkey		bsr	GetBlock		get a cache buffer
		movea.l	d0,a4			save buffer address

; Got a key and a header block to put on it.  Initialise the simple stuff.
		lea.l	cb_SIZEOF(a4),a0	clear out the header area
		bsr	ClearBlock
		move.l	#t.short,cb_SIZEOF+fhb_Type(a4)
		move.l	d5,cb_SIZEOF+fhb_OwnKey(a4)
		lea.l	cb_SIZEOF(a4),a0	point to end of block
		adda.l	BlockSize(a5),a0
		move.l	d2,vfhb_SecType(a0)	may be wrong if a link
		lea.l	vfhb_Days(a0),a0	put in creation date
		bsr	DatStamp

; Now do any special initialisation that's required for hard or soft links.
		cmpi.w	#st.link,d2		did we make a link ?
		bne.s	Created			nope, write out the header
	IFD SOFTLINKS_SUPPORTED
		tst.l	dp_Arg4(a2)		making a soft link ?
		beq.s	MakeHardLink		nope, assume a hard filelink

; We're making a soft link.  Set type and copy the path string to the hashtable
		lea.l	cb_SIZEOF(a4),a0	point to end of block
		adda.l	BlockSize(a5),a0
		moveq.l	#ST_SOFT_LINK,d0	set up for a soft link
		move.l	d0,vfhb_SecType(a0)
		lea.l	cb_SIZEOF+fhb_HashTable(a4),a0
		movea.l	dp_Arg3(a2),a1		fetch name pointer

;Randal, I hate you!  Given linkname is a normal CPTR to a CSTR
;		adda.l	a1,a1			convert to APTR
;		adda.l	a1,a1
;		clr.w	d0
;		move.b	(a1),d0			fetch string length
30$	printf <'linkname = %s\n'>,a1
31$		move.b	(a1)+,(a0)+		copy string to header
		bne.s	31$
;		dbra	d0,30$
		bra.s	Created			soft link created
	ENDC

; We're making a hard link.  Assume it's a file link until we know for sure.
MakeHardLink	movea.l	dp_Arg3(a2),a1		fetch link object lock
		adda.l	a1,a1			convert to an APTR
		adda.l	a1,a1
		move.l	fl_Key(a1),d1		fetch key of link object
		lea.l	cb_SIZEOF(a4),a0	point to end of block
		adda.l	BlockSize(a5),a0
		move.l	d1,vfhb_Link(a0)	store link key
		moveq.l	#ST_FLINK,d0		set up for a file link
		move.l	d0,vfhb_SecType(a0)	assume this type

; we've created the file/directory/link header, now write it out to disk.  We
; can still safely crash at this point because we haven't linked to anything.
Created		movea.l	a4,a0			writing this buffer
		bsr	WriteSummedBlockD5A0Later  Buffer in a0, block in d5

; after writing it, allocate a new dirlist block if this is a dir
; we can't do this until after the WriteBlockLater (you can't hold buffers
; across a AllocKeys call).
		btst.b	#2,DiskType+3(a5)	are we using fast dirs?
		beq.s	not_fast
		cmpi.w	#st.userdir,d2		did we make a directory?
		bne.s	notdir			nope

;	printf <'making dirlist'>
		move.l	d2,-(sp)		save type
		move.l	d5,d0			old block (allocate near it)
		move.l	d5,d2			also is the parent dir
		moveq	#0,d1			we're not the validator
		bsr	MakeListBlock		returns d0-key (or 0)
		move.l	(sp)+,d2		get type back
		tst.l	d0			did we succeed?
		bne.s	gotnewdirlist
		move.l	d1,ErrorCode(a5)	save return error

		; We couldn't allocate a block.  Back out Create.
		; we only get here if we allocated the block!
FreeFhb		printf <'Make/InsertList failure, backing out!'>
		move.l	d5,d0			we MUST have allocated it
		moveq.l	#1,d1			only one block (fhb)
		bsr	FreeKeys		free the fhb
		bra	ReallyError		failure - kill co-routine

; If the disk is bigger than N blocks, allocate a second dircache block by
; default when the first one is allocated.  (Should we do this all the time?)
gotnewdirlist:	cmp.l	#6*1024,HighestBlock(a5)	roughly 3MB
		bcs.s	10$			size < ~3MB
		move.l	d2,-(sp)		save type
		move.l	d5,d0			dir
		bsr	GrabBlock
;		move.l	d0,a0
		lea	cb_SIZEOF(a0),a1
		add.l	BlockSize(a5),a1	vudb ptr
		move.l	vudb_DirList(a1),-(sp)	the dirlist block we made
		bsr	LooseKeyBlock		associate with itself
		move.l	(sp)+,d0		the block to add the new one to
		move.l	d5,d2			the parent dir
		moveq	#0,d1			we're not the validator
		bsr	MakeListBlock		returns d0-key (or 0)
		move.l	(sp)+,d2		get type back
10$		; we don't care if this fails...

; Insert this header into the owning directory.  Root is invalid since we
; can't get here unless we allocated a block for a fileheader/dir/link/etc.
; Name is NOT set (null).  Pass name in a0.
; FIX! really should do after finishing link stuff... (I think?)
notdir		move.l	d5,d1			insert this key...
		move.l	d4,d0			into this directory...
		move.l	sp,a0			with this name...
		bsr	InsertList		update dir cache list
		tst.l	d0			Did we fail somewhere?
		beq.s	FreeFhb			was there an error?
		
not_fast:	move.l	d5,d1			insert this key...
		move.l	d4,d0			into this directory...
		movea.l	sp,a0			giving it this name
		bsr	Insert
		lea.l	32(sp),sp		finished with workspace

; if we just made a dir or link then check for notifications (orphans only)
; we don't notify for file headers.  That's done when the file is closed.
		cmpi.w	#st.userdir,d2		did we make a directory?
		bne.s	5$			nope
		cmpi.w	#st.link,d2
		bne.s	CreateGetLock		nope
		move.l	d4,d0			checking this directory
		moveq.l	#TRUE,d1		want to rebuild orphans
		bsr	CheckNotify		do notifies if needed

; Last fiddly bit.  If we just created a hard link then we have to fix up
; the back links in the hard link chain (if any) and check the type is set
; up correctly.  If the linked object is not a file then we must change the
; secondary type of the header we just made to ST_DLINK too.
5$		cmpi.w	#st.link,d2		did we make a link ?
		bne.s	CreateGetLock		nope, just lock the header
		tst.l	dp_Arg4(a2)		was it a hard link ?
 		bne.s	CreateGetLock		nope

		movea.l	dp_Arg3(a2),a1		fetch link object lock
		adda.l	a1,a1			convert to an APTR
		adda.l	a1,a1
		move.l	d4,d7			save dir block #...
		move.l	fl_Key(a1),d4		fetch key of linked object
		move.l	d4,d0
		bsr	GrabBlock		and fetch it's header
;		movea.l	d0,a0
		lea.l	cb_SIZEOF(a0),a1	point at the link field
		adda.l	BlockSize(a5),a1
		move.l	vfhb_SecType(a1),-(sp)	note type of this header
		move.l	vfhb_BackLink(a1),-(sp)	save previous links (maybe none)
		move.l	d5,vfhb_BackLink(a1)	point back to the new link
		bsr	WriteSummedBlockD4A0Later  Buffer in a0, block in d4

; Now re-fetch the new link and set up its backlink appropriately.  Also check
; that we made it the correct type and adjust to ST_DLINK if required.
		move.l	d5,d0			get back our header key
		bsr	GrabBlock
;		movea.l	d0,a0
		lea.l	cb_SIZEOF(a0),a1	point to end of block
		adda.l	BlockSize(a5),a1
		move.l	(sp)+,vfhb_BackLink(a1)	insert into link list
		move.l	(sp)+,d0		get back the header type
		cmpi.w	#st.file,d0		did we make the right link
		beq.s	10$			yes, it was a file
		moveq.l	#ST_DLINK,d0		convert to directory link
		move.l	d0,vfhb_SecType(a1)
10$		bsr	WriteSummedBlockD5A0Later  Buffer in a0, block in d5

		btst.b	#2,DiskType+3(a5)	are we using fast dirs?
		beq.s	CreateGetLock		no, we're done

		; since we changed the type, we have to update the list entry
		move.l	d5,d1			object to update
		move.l	d7,d0			directory it's in
		bsr	UpdateList		update directory cache

; Finally, we've created the header, now attempt to get the right lock on it.
CreateGetLock	cmpi.w	#st.link,d2		did we make a link ?
		bne.s	10$			nope, so lock object
		moveq.l	#TRUE,d0		yes, return TRUE
		bra.s	CreateExit

10$		move.l	d5,d0			lock this header
		move.l	d3,d1			with this access mode
		bsr	GetLock			return d0 = lock
CreateExit	movem.l	(sp)+,d2-d7/a2-a4

		; release the directory lock we got via Locate!
		move.l	d0,-(sp)		save return value
		move.l	CurrentCo(a5),a0
		lea	co_dsem(a0),a0
		bsr	ReleaseDir
		move.l	(sp)+,d0

		; printf	<'Create done\n'>
		rts

;==============================================================================
; key = VLock( lock, packet, writeflag )
;  d0		a0	a1	d0
;
; returns the key belonging to the required lock if everything checks out OK
; for this lock, else fails the packet and kills the current co-routine.
; As with all locks in this bloody program, a0 is a BPTR.
;==============================================================================
VLock		movem.l	d2-d3/a2,-(sp)
		move.l	d0,d2			save writing flag
		movea.l	a1,a2			and packet address
		bsr	CheckLock		go check this lock
		move.l	d0,d3			did CheckLock work OK ?
		beq.s	10$			nope, fail this packet
		tst.l	d2			are we writing
		beq.s	20$			nope, so everything OK
		bsr	DiskProtected		check we can write to disk
		tst.l	d0
		beq.s	20$			everything OK
10$		movea.l	a2,a0			fail this packet and kill self
		bra	WorkFail		never comes back from here
20$		move.l	d3,d0			return key in d0
		movem.l	(sp)+,d2-d3/a2
		rts

;==============================================================================
; key = FindDir( dkey,name,wkspace,packet)
;  d0		  d0   a0     a1     a2
;
; Find the directory specified by the default key (dkey) and a name (which may
; contain a colon and/or some backslashes).  wkspace is just that, it is left
; containing the simple name after all device and directory stuff is stripped
; off.  dkey should have been obtained from the original lock on this volume.
; This routine will not cope with the case of a : followed by nothing.  This
; must be filtered out before calling here.  If any part of the path doesn't
; exist (or contains a soft link) then the packet will be returned to the
; caller and the current co-routine will be aborted (via WorkFail).
;==============================================================================
FindDir		movem.l	d2-d4/a3-a4,-(sp)
		movea.l	a0,a3			stash string
		movea.l	a1,a4			and workspace
		move.l	d0,d2			save current default key

; first split off any dev: stuff because we ain't interested in it
		movea.l	a4,a0			workspace
		movea.l	a3,a1			given string
		moveq.l	#':',d0			what we are looking for
		moveq.l	#1,d1			where to start looking
		bsr	SplitName		go do it
		move.w	d0,d3			ptr = splitname(....
		bne.s	10$			there was a colon in there
		moveq.l	#1,d3			can't have ptr value of 0

; This is the main loop that looks for separate names delimited by backslash.
; Each name that is found is searched for in the current key (dkey) and the
; packet is failed if it is not found.  If a double backslash is found then
; the parent of the current key is fished out and failed if it doesn't exist.
; Once the whole string has been parsed, wkspace (a4) will contain the simple
; filename we are after or nothing if the string ended in backslash.
10$		movea.l	a4,a0			search for backslash
		movea.l	a3,a1			given string
		moveq.l	#'/',d0			what we are looking for
		move.w	d3,d1			look from ptr onwards
		bsr	SplitName
		move.w	d0,d4			p=splitname(....
		beq.s	FoundDir		no backslash, simple name left

; if p=ptr+1 (d0 = d3+1) we have a backslash by itself (or double backslash
; in the middle of a string) and must go back to the parent of the current key
		subq.w	#1,d0			(p-1 = ptr) == (p = ptr+1)
		cmp.w	d0,d3
		bne.s	20$			nope, check name exists

; get back to the parent of the current key (dkey in d2)
		move.l	d2,d0			current key
		bsr	ParentKey		find the parent
		move.l	d0,d2			stash key again
		beq.s	NotFoundDir		fail this packet, not found
		cmp.b	(a3),d3			was it a terminating backslash
		beq.s	FoundDir		yep, so better quit now
		move.w	d4,d3			ptr := p
		bra.s	10$			and keep looking

; we have a simple name in wkspace (a4) check that it exists in the current key
; by searching the hashchain of this block to see if it's there
20$		move.w	d4,d3			ptr := p
		move.l	d2,d0			search this key
		movea.l	a4,a0			for this name
		moveq.l	#TRUE,d1		follow links
		bsr.s	FindEntry
		tst.l	d0			did we find it
		beq.s	EntryNotFound		no, fail this packet
		move.l	ObjKey(a5),d2		get new current key (global)
		bra.s	10$			and keep looking

; screwed up while we were searching so return packet and kill this co-routine
NotFoundDir	move.w	#ERROR_OBJECT_NOT_FOUND,ErrorCode+2(a5)
EntryNotFound	movea.l	a2,a0			this is the packet
		bra	WorkFail		never comes back anyway

; we have the directory ok, return dkey to the caller
FoundDir	move.l	d2,d0			this is the key
		movem.l	(sp)+,d2-d4/a3-a4
		rts

;==============================================================================
; result = FindEntry( key, name, followlink)
;   d0		       d0   a0	     d1
;
; returns a boolean indicating whether the given name could be found in the
; directory header block pointed to by key.  If it is found OK then the global
; variable ObjKey will contain the key for the required object and PrevKey will
; contain the pointer to its predecessor (which may be another file if it's
; not first in the hash chain).
;
; I added a new argument for handling links.  FollowLink determines if a
; link should be followed to its destination or if the link header itself
; is the required object.  FollowLink = TRUE means follow the link else
; return the link header itself.  If FollowLink = TRUE and we find a soft
; link, we set up an ERROR_SOFT_LINK return.  When a hard link has been
; followed, PrevKey will NOT be valid, though it is if we aren't following.
;
; We now have an additional entry (lockdir) for whether to ObtainDir
; before scanning the list.  It will ObtainDir regardless of whether the
; entry was found or not (so we can guarantee consistent state for Create
; and Rename).  See ObtainDir for a full explanation of where it needs to be
; called.
;==============================================================================
FindEntry	movem.l	d2-d5/a2-a3,-(sp)
		move.l	d1,d5			save followlink flag
		move.l	d0,d2			stash the key we want
		movea.l	a0,a2			and the name
		move.l	d0,d3			keep key as prevkey too

;		bsr	WaitBlock		get the keyed block
		bsr	GrabBlockA3		returns d0,a0
;		movea.l	d0,a3			and stash it

;		movea.l	a3,a0			make sure this is a directory
		bsr	IsDir			preserves a0!
		tst.l	d0
		bne.s	10$			no problem

		move.l	d3,d0			associated with itself (a0 still set)
		bsr	LooseBlock		chuck current block away (a0)
		move.w	#ERROR_OBJECT_WRONG_TYPE,ErrorCode+2(a5)
		moveq.l	#FALSE,d0		and return FALSE
		bra	FindEntryDone

; find the hash offset for the given name and extract the first key value
10$		movea.l	a2,a0
		bsr	Hash			get byte offset to hash key
		move.l	cb_SIZEOF(a3,d0.w),d4	d4 = first hash key
		beq	NotFoundEntry		nothing there

; main loop. Follow the hash chain until Okey (d4) = 0 and fail if this happens
; If our entry is found then the global keys are set up and TRUE is returned.
HCLoop		move.l	d3,d0			associate with itself
		bsr	LooseA3Block
		move.l	d4,d0			get the next one
;		bsr	WaitBlock
		bsr	GrabBlockA3
;		movea.l	d0,a3			this is our new buff
		movea.l	a2,a0			see if names are the same
		lea.l	cb_SIZEOF(a3),a1
		adda.l	BlockSize(a5),a1
		lea.l	vfhb_FileName(a1),a1	point to name in buffer
		bsr	CompString
		tst.l	d0			did they match ?
		bne.s	FollowHChain		nope, link to next block

; New addition.  We've found the filename we want, if we're not following
; links then there's no need to check for the presence of a link anyway.
		tst.l	d5			are we following links ?
		beq.s	GotObject		nope, so accept this header

; Now we test to see if we have a link header.  If we do then we should
; fetch the header it is pointing to instead.
;
; IMPORTANT NOTE!  If followlink is TRUE then prevkey CANNOT be depended on.
; Only Scratch and Rename depend on this and they set followlink to FALSE.
; DON'T USE FOLLOWLINK=TRUE IF YOU PLAN TO USE THE CONTENTS OF PREVKEY TOO!
		lea.l	cb_SIZEOF(a3),a0	point to end of block
		adda.l	BlockSize(a5),a0
		move.l	vfhb_SecType(a0),d0
		cmpi.w	#ST_SOFT_LINK,d0	is this a soft link
		bne.s	998$			no, check for hard link

; we've got a soft link, since FollowLink = TRUE return an error.  Slight
; difference to a normal error because objkey will still be valid.
		move.w	#ERROR_SOFT_LINK,ErrorCode+2(a5)
		movea.l	a3,a0			done with current block
		bsr	LooseKeyBlock
		move.l	d4,ObjKey(a5)		global object key
		move.l	d3,PrevKey(a5)		and global previous key
		bra.s	ErrSoftLink		return error

998$		cmpi.w	#ST_FLINK,d0		is this a hard file link ?
		beq.s	21$			yes, we've got a link
		cmpi.w	#ST_DLINK,d0		is this a hard directory link?
		bne.s	GotObject		no problem, not a link

; We've found a link and caller wants us to follow it now, extract header key
21$		move.l	vfhb_Link(a0),d4	get block that we're linked to

; found the name we were looking for so set up ObjKey and PrevKey, return TRUE
GotObject	movea.l	a3,a0			loose current block
		bsr	LooseKeyBlock
		move.l	d4,ObjKey(a5)		global object key
		move.l	d3,PrevKey(a5)		and global previous key
;						invalid if link was followed!
		moveq.l	#TRUE,d0		return true
		bra.s	FindEntryDone		all finished

; current entry was not the one we wanted so chain to the next if there is one
FollowHChain	move.l	d4,d3			prevkey = objkey
		lea.l	cb_SIZEOF(a3),a0	reference hashchain from end
		adda.l	BlockSize(a5),a0
		move.l	vfhb_HashChain(a0),d4	chain to the next
		bne	HCLoop			there is one to examine

; the entry we wanted was not there, set error and prev key and return FALSE
NotFoundEntry	move.w	#ERROR_OBJECT_NOT_FOUND,ErrorCode+2(a5)
		movea.l	a3,a0			loose this buffer
		bsr	LooseKeyBlock
		clr.l	ObjKey(a5)		no valid object key
		move.l	d3,PrevKey(a5)		but previous is valid
ErrSoftLink	moveq.l	#FALSE,d0		failure

; gets to here with the appropriate returncode in d0
FindEntryDone	movem.l	(sp)+,d2-d5/a2-a3
		rts

	IFD SOFTLINKS_SUPPORTED
;==============================================================================
; Length = ReadLink(lock,name,buffer,size)
;
; dp_arg1 = current dir lock
; dp_arg2 = pathname that contains a link somewhere within it (CSTRing!!!)
; dp_arg3 = pointer to buffer for name
; dp_arg4 = size of buffer
;
; After DOS receives ERROR_SOFT_LINK it uses this routine to find the pathname
; to the object that is really required.  Must be pointing to a soft link.
;==============================================================================
ReadLink	movea.l	d0,a2			save packet
		movea.l	dp_Arg1(a2),a3		a3 = lock (current position)
		movea.l	dp_Arg2(a2),a4		a4 = name (string)
		movea.l	a3,a0			current dir lock, leave as BPTR
		movea.l	a2,a1			packet
		moveq.l	#FALSE,d0		won`t be writing
		bsr	VLock			won't come back if it's bad !!
		move.l	d0,d3			save current directory key

; fix, stupid readlink sends me a CSTR instead of a BSTR.  Have to convert
; to BSTR so splitname will work on it.
		lea.l	-256(sp),sp		should have enough stack 4 this
		movea.l	sp,a0
		addq.l	#1,a0
		moveq.l	#0,d0
		bra.s	20$
10$		addq.w	#1,d0
20$		move.b	(a4)+,(a0)+
		bne.s	10$
		movea.l	sp,a4			make a4 point at converted name
		move.b	d0,(a4)

		lea.l	-32(sp),sp		make space for name prefix
		movea.l	sp,a0			prefix space
		movea.l	a4,a1			string we are searching
		moveq.l	#':',d0			what we're searching for
		moveq.l	#1,d1			where to start searching
		bsr	SplitName
		move.w	d0,d4			d4 holds current ptr
		bne.s	FindLink		stripped off :
		moveq.l	#1,d4			can't have pointer of 0

; We must go find this filename relative to the current position.  We have to
; duplicate the functionality of FindDir() here because we need to know which
; part of the path actually contains the soft link.  Once the soft link data
; has been put into the callers buffer, the remainder of the path is appended.
FindLink	move.w	d4,d5			p=ptr
		movea.l	sp,a0			workspace
		movea.l	a4,a1			given string
		moveq.l	#'/',d0			what we're searching for
		move.l	d4,d1			where to search from
		bsr	SplitName		fill workspace with simple name
		move.w	d0,d4
		beq.s	24$			link could be last in path

; if p=ptr+1 (d0 = d5+1) we have a backslash by itself (or double backslash
; in the middle of a string) and must go back to the parent of the current key
		subq.w	#1,d0			(p-1 = ptr) == (p = ptr+1)
		cmp.w	d0,d5
		bne.s	24$			nope, check name exists

; get back to the parent of the current key (dkey in d3)
		move.l	d3,d0			current key
		bsr	ParentKey		find the parent
		move.l	d0,d3			stash key again
		beq.s	LinkNotFound		fail this packet, not found
		cmp.b	(sp),d4			was it a terminating backslash
		beq.s	LinkNotFound		yep, so better quit now
		bra.s	FindLink		nope, keep looking

; workspace is holding a simple name, check that the name exists in current dir
24$		move.l	d3,d0			search this key
		movea.l	sp,a0			for this name
		moveq.l	#TRUE,d1		follow links
		bsr	FindEntry
		move.l	ObjKey(a5),d3		assume this is new key
		tst.l	d0			did we find it ?
		beq.s	25$			nope, see if it's a link
		tst.l	d4			if d4==0 then no link here
		bne.s	FindLink		yes, so keep searching
		bra.s	LinkNotFound

; we failed to find an entry.  If this is because it was a soft link then d3
; will hold the block number.  Any other failure is terminal and we just exit.
25$		cmpi.w	#ERROR_SOFT_LINK,ErrorCode+2(a5)
		bne.s	ReadLinkFail		wrong kind of error

; by this stage our default key (d3) is set up pointing to the header block
; that we want.  Read that header and copy appropriate data across.
		lea.l	32(sp),sp		finished with stack workspace
		adda.w	d4,a4			a4 points at unused string space
		move.l	d3,d0			key we want
		bsr	GrabBlock
;		movea.l	d0,a0

; a0 is pointing to a valid soft link header, copy the name string into the
; supplied buffer.  This buffer must be large enough to take the whole string
		lea.l	cb_SIZEOF+fhb_HashTable(a0),a3	source data
		movea.l	dp_Arg3(a2),a1		destination area
		moveq.l	#0,d0
		bra.s	75$
80$		addq.l	#1,d0
75$		move.b	(a3)+,(a1)+		transfer data
		bne.s	80$
		tst.w	d4			was there more path?
		beq.s	96$			nope, linkname was last in path
		cmpi.b	#':',-2(a1)		no / if colon is last character
		bne.s	89$
		subq.w	#1,a1
		bra.s	95$
89$		move.b	#'/',-1(a1)		need directory separator
		bra.s	95$			copy rest of name across
90$		addq.l	#1,d0
95$		move.b	(a4)+,(a1)+
		bne.s	90$
96$		move.l	d0,-(sp)		save string length
		bsr	LooseKeyBlock		done with header now (a0!)
		move.l	(sp)+,d0		restore string length
		movea.l	a2,a0			return packet and wind up
		bra	WorkDone

LinkNotFound	move.w	#ERROR_OBJECT_NOT_FOUND,ErrorCode+2(a5)
ReadLinkFail	movea.l	a2,a0
		bra	WorkFail
	ENDC ; SOFTLINKS_SUPPORTED

;==============================================================================
; Res = DiskProtected()
; d0
;
; call this before writing to a disk to set errorcode for various nasties. A 0
; return means it's safe to write anything to this disk.
;==============================================================================
DiskProtected	moveq.l	#0,d0			assume no error
		cmpi.l	#ID_NO_DISK_PRESENT,DiskType(a5)
		bne.s	3$			there was a disk there
		move.w	#ERROR_NO_DISK,d0	no disk present
		bra.s	20$

3$		tst.w	BitmapValid(a5)		any bitmap
		bne.s	10$			yep, it's validated OK
		tst.w	TotallyFull(a5)		did bitmap fit ?
		beq.s	5$			yep, no requester needed

		moveq.l	#-125,d1		put up no room for bitmap message
		moveq.l	#-1,d0
		bsr	ValidatorError

5$		move.w	#ERROR_DISK_NOT_VALIDATED,d0
10$		tst.w	WrProt(a5)		is it protected
		beq.s	20$			no, then no error
		move.w	#ERROR_DISK_WRITE_PROTECTED,d0
20$		move.w	d0,ErrorCode+2(a5)	return the error code
		rts

;==============================================================================
; Parent( packet )
; 	    d0
;
; Called as a co-routine to find the parent key of the given lock.  Doesn't
; save any registers because it's a co-routine.
;==============================================================================
Parent		movea.l	d0,a2			save the packet address
		movea.l	a2,a1			go verify the lock
		movea.l	dp_Arg1(a2),a0		BPTR to lock
		moveq.l	#FALSE,d0		not writing
		bsr	VLock			won't come back if it's bad
		bsr.s	ParentKey		get parent of key in d0
		tst.l	d0			are we at the root ?
		bne.s	10$			no, get a lock on this key

; we are at the root of the fileing system, but don't return an error code
		move.w	d0,ErrorCode+2(a5)	zero error code
		movea.l	a2,a0			return packet with res=0
		bra	WorkDone		d0 has res

; we aren't at the root and the lock checked out OK so get a lock on the parent
10$		moveq.l	#shared.lock,d1		get lock on key in d0
		bsr	GetLock			returns lock to d0
		movea.l	a2,a0			set up the packet
		bra	WorkDone		and return the lock (in d0)

;==============================================================================
; key = ParentKey( key )
;  d0		    d0
;
; returns the parent key of the given key.  (Given key must be a valid header)
;==============================================================================
ParentKey	cmp.l	RootKey(a5),d0		are we at the root ?
		bne.s	NotRoot			nope
		move.w	#ERROR_OBJECT_NOT_FOUND,ErrorCode+2(a5)
		moveq.l	#0,d0
		rts

NotRoot		bsr	GrabBlock		get block for this key
;		movea.l	d0,a0			buffer pointer to a0
		lea.l	cb_SIZEOF(a0),a1
		adda.l	BlockSize(a5),a1
		move.l	vfhb_Parent(a1),-(sp)	save parent key
		bsr	LooseKeyBlock		put block (a0) back on valid list
		move.l	(sp)+,d0		return the key
10$		rts

;==============================================================================
; valid = CheckName( name )
;  d0		      a0
;
; Given an APTR to a BSTR returns TRUE if the name is OK or FALSE if it ain't.
; 3/24/88  Fixed to allow accented characters in file and disk names
;==============================================================================
CheckName	move.l	d2,-(sp)
		moveq.l	#FALSE,d2		assume failure
		move.w	#ERROR_INVALID_COMPONENT_NAME,ErrorCode+2(a5)
		clr.w	d1
		move.b	(a0)+,d1		get length
		beq.s	20$			0 is bad
		cmpi.w	#30,d1
		bgt.s	20$			>30 chars is bad
		bra.s	15$

10$		move.b	(a0)+,d0		get next char
		bsr	CapitalChar		make capital
		andi.b	#$7f,d0			allow funny chars too
		cmpi.b	#' ',d0			< space not allowed
		blt.s	20$
		cmpi.b	#'/',d0			slash not allowed
		beq.s	20$
		cmpi.b	#':',d0			colon not allowed
		beq.s	20$
15$		dbra	d1,10$

		moveq.l	#TRUE,d2
		clr.w	ErrorCode+2(a5)		no error

20$		move.l	d2,d0
		move.l	(sp)+,d2
		rts

;==============================================================================
; Rename( packet )
;	    d0
;
; Renames a file.  Can't rename a directory into itself or rename across devs.
; Fixed bug with 2 files, a and b.  used to allow rename b as a/b.
;==============================================================================
Rename		movea.l	d0,a2			stash the packet
		lea.l	-64(sp),sp		room for 2 name strings
		movea.l	dp_Arg1(a2),a0		verify source lock
		movea.l	a2,a1
		moveq.l	#TRUE,d0
		bsr	VLock
		move.l	d0,d2			d2=from.dkey

		movea.l	dp_Arg3(a2),a0		verify destination lock
		movea.l	a2,a1
		moveq.l	#TRUE,d0
		bsr	VLock
		move.l	d0,d3			d3=to.dkey

; make sure nothing else is trying to rename, so we can safely hold two
; directory locks (see ObtainDir for more info).  It will be released when
; this coroutine goes away in WorkFail/Done (or we can release it explictly
; if we wish).
		bsr	ObtainRename

		move.l	d2,d0			find required source dir
		movea.l	dp_Arg2(a2),a0
		adda.l	a0,a0
		adda.l	a0,a0			given name
		movea.l	sp,a1			space for simple name
		bsr	FindDir
		move.l	d0,d2

; we need to lock the directory before scanning it
		bsr	ObtainDir		critical section starting
		move.l	d2,d0			get block # back again

		movea.l	sp,a0			now find entry in this dir
		moveq.l	#FALSE,d1		DON'T follow links
		bsr	FindEntry
		movea.l	a2,a0			assume failure
		tst.l	d0
		beq	WorkFail		entry not found
		move.l	ObjKey(a5),d4		save source object key
		move.l	PrevKey(a5),d5		and previous key to object

		move.l	d3,d0			check for dest dir existing
		movea.l	dp_Arg4(a2),a0
		adda.l	a0,a0
		adda.l	a0,a0
		lea.l	32(sp),a1
		bsr	FindDir
		move.l	d0,d3			won't come back if it failed

; d2 and d3 now have the two directories in question.
; we need to lock the directory before scanning it.
; This will use the single Rename_dsem, so rename can have 2 locks without
; deadlock.
		moveq.l	#TRUE,d6		assume not the same name & dir
		cmp.l	d2,d3			if the same dir, don't lock!
		beq.s	5$			we'd wait on ourselves!
		bsr	ObtainDir		critical section starting
		bra.s	10$
5$
		; it's in the same dir, are the names identical?
		movea.l	sp,a0
		lea.l	32(sp),a1
		bsr	CompString
		move.l	d0,d6			false if same name and dir
		beq.s	20$			if so, dont do the find entry

		; check if the destination file exists already
10$		move.l	d3,d0			see if dest file is present
		lea.l	32(sp),a0
		moveq.l	#FALSE,d1		dont care about links here
		bsr	FindEntry
		tst.l	d0
		beq.s	20$			OK, name didn't exist
		move.w	#ERROR_OBJECT_EXISTS,ErrorCode+2(a5)
		movea.l	a2,a0
		bra	WorkFail

; this fixes the rename bug.  Don't allow remove/insert if key d3 is not a dir.
20$		move.l	d3,d0			get the block we want
		bsr	WaitBlock
		move.l	d0,-(sp)		save buffer
		movea.l	d0,a0
		bsr	Type			get key type
		movea.l	(sp)+,a0		free up the buffer again
		move.l	d0,-(sp)
		bsr	LooseKeyBlock		frees block (a0)
		move.l	(sp)+,d0
		cmpi.w	#st.userdir,d0
		beq.s	25$			OK, it is a directory
		cmpi.w	#ST_DLINK,d0		link dirs are OK too
		beq.s	25$
		move.w	#ERROR_OBJECT_WRONG_TYPE,ErrorCode+2(a5)
		movea.l	a2,a0
		bra	WorkFail		not a dir, error out

; further bug fix.  Don't allow a directory to get renamed into itself.
25$		move.l	d4,d0			make sure source obj is a dir
		bsr	WaitBlock
		movea.l	d0,a0
		bsr	IsDir			see if it's a directory (preserve a0!
		move.l	d0,-(sp)		save returncode
		bsr	LooseKeyBlock		done with block in a0
		move.l	(sp)+,d0
		beq.s	29$			not a directory

; we have a directory to be renamed.  Make sure it's not in the path of the
; destination directory.  If it is, give up with ERROR_OBJECT_IN_USE
		move.l	d3,d0			destination directory
26$		cmp.l	d0,d4			same key?
		beq.s	27$			yes, fail it
		bsr	ParentKey		get parent
		tst.l	d0			at root?
		bne.s	26$			nope keep looking
		bra.s	29$			yes, so no error

; our source directory is in the path of the destination so don't allow rename
27$		move.w	#ERROR_OBJECT_IN_USE,ErrorCode+2(a5)
		movea.l	a2,a0
		bra	WorkFail

; end of rename bug fix.
29$		move.l	d4,d0			get lock on source object
		moveq.l	#shared.lock,d1
		bsr	GetLock
		movea.l	a2,a0			assume failure
		move.l	d0,d7			did we get the lock ?
		beq	WorkFail		no, fail now

		lea.l	32(sp),a0
		bsr	CheckName		make sure name is OK
		tst.l	d0
		bne.s	30$
		movea.l	d7,a0			free up this lock
		bsr	FreeLock
		movea.l	a2,a0
		bra	WorkFail		name was bad

30$		bsr	AlterRoot		things, they are a changin'
		tst.l	d6			are we using same dir & name ?
		bne.s	40$			nope, must do remove/insert

;We are renaming a file but just changing the case of the letters, do in place
		move.l	d4,d0			grab object key
		bsr	GrabBlockA3
;		movea.l	d0,a3
		lea.l	32(sp),a0
		lea.l	cb_SIZEOF(a3),a1
		adda.l	BlockSize(a5),a1
		lea.l	vfhb_FileName(a1),a1
		moveq.l	#0,d0
		move.b	(a0),d0			copy name to buffer
35$		move.b	(a0)+,(a1)+
		dbra	d0,35$
		bsr	WriteSummedBlockD4A3Later   Block in D4, ptr in A3

		; now we need to update the directory cache (if any)
		btst.b	#2,DiskType+3(a5)	are we using fast dirs?
		beq.s	RenameDone		no, we're done
		move.l	d4,d1			fhb that changed
		move.l	d3,d0			directory it's in
		bsr	UpdateList
		bra.s	RenameDone		that's all

;we are using a completely different name so we must do the remove/insert thing
;40$		cmp.l	d3,d4			can't rename dir into itself
;		bne.s	50$
;		move.w	#ERROR_OBJECT_IN_USE,ErrorCode+2(a5)
;		movea.l	d7,a0
;		bsr	FreeLock
;		movea.l	a2,a0
;		bra	WorkFail

40$:
50$		move.l	d5,d0			remove from old hash chain
		move.l	d4,d1
		movea.l	sp,a0			d2 already holds from dir key
		bsr	Remove

		btst.b	#2,DiskType+3(a5)	are we using fast dirs?
		beq.s	1$
		move.l	d4,d1			insert this key...
		move.l	d3,d0			into this directory...
		lea	32(sp),a0		with this name...
		bsr	InsertList		update dir cache list
		tst.l	d0			Did we fail somewhere?
		bne.s	1$			was there an error?

		; We couldn't allocate a block.  Back out Create.
		move.l	d4,d1
		move.l	d2,d0			back into the original dir
		move.l	sp,a0			with the old name
		bsr	InsertList		should be impossible to fail

		bra	ReallyError		kill coroutine
		
1$		move.l	d3,d0			and insert into new hash chain
		move.l	d4,d1			(could be same hash chain)
		lea.l	32(sp),a0
		bsr.s	Insert

; so, now it's time to do all the checking to see if anyone want's notifying
; at this point, d2=source dir, d3=dest dir, d4=object.  First free the lock
; so that whoever we call can do something to the file or directory
		movea.l	d7,a0
		bsr	FreeLock
		move.l	d2,d0			notify directory watchers
		moveq.l	#FALSE,d1		no need to rebuild lists
		bsr	CheckNotify
		move.l	d4,d0			notify the new orphans
		moveq.l	#TRUE,d1		and rebuild thier lists
		bsr	CheckNotify
		move.l	d3,d0			notify existing orphans
		moveq.l	#TRUE,d1
		bsr	CheckNotify		and rebuild thier lists too
		bra.s	NotifySkip

RenameDone	movea.l	d7,a0			free up the lock
		bsr	FreeLock
NotifySkip	moveq.l	#TRUE,d0
		movea.l	a2,a0
		bra	WorkDone		all finished

;==============================================================================
; Insert( dkey, okey, name )
;	   d0    d1    a0
;
; Insert the given object key (okey) into the given directory key (dkey) using
; the supplied name (APTR to BCPL string).  Attempts to keep the hashchain
; ordered by block number to make exnext and hashchaining a bit faster.
;==============================================================================
Insert		; printf	<'Inserting %ld into %ld\n'>,d1,d0
		movem.l	d2-d6/a2-a3,-(sp)
		move.l	d0,d2			stash dkey
		move.l	d1,d3			and okey
		movea.l	a0,a2			and name
		bsr	Hash			get hash on name in (a0)
		move.w	d0,d6			save slot offset
		move.l	d2,d5			pkey = dkey

; insert the block in the hashchain in the correct order by keys
findslot	move.l	d5,d0			block we want
		bsr	GrabBlockA3		OK to just grab it
;		movea.l	d0,a3			stash buffer address
		move.l	cb_SIZEOF(a3,d6.w),d4	get header key
		beq.s	foundslot		0, so we found slot
		cmp.l	d3,d4			if nkey > okey
		bgt.s	foundslot		also found it
		move.l	d5,d0			associate with itself
		bsr	LooseA3Block
		move.l	d4,d5			pkey = nkey
		move.l	BlockSize(a5),d6	calculate offset to hashchain
		addi.w	#vfhb_HashChain,d6	slot offset from now on
		bra.s	findslot

; we found our slot, d3=our header, d5=previous header, d4=next header.
; NOTE: the directory is corrupt until we write out our file header key.
; However, we have to write out the directory header so no-one can get
; in and steal our slot on the hashchain while we're writing our header
;
; Now with directory semaphores (ObtainDir), we're safe against anyone
; changing a slot while we're doing this, so we can write out the fhb
; first, thus avoiding a window that would leave a non-validatible FS. - REJ
; Of course, the order could get changed by other things getting in
; before this stuff got flushed to disk.  Perhaps we should WaitBlock
; somewhere. - REJ
;
; Better yet would be a WriteBehind field, which would guarantee that a
; block would be written after another block.  On GrabBlock of a block on
; the WriteQueue, you'd also remove any blocks which were supposed to
; write after the one grabbed (and any after them, etc).  When the block
; was re-written or LooseBlocked, you'd re-add the blocks you removed after
; the block you grabbed.  This maintains the order of writes that care,
; such as directory updates, in order to make sure the disk is always
; maximally validatable (no broken chains).  The only other way to do it
; would be to use things like WriteBlockNow or WaitBlock to ensure they
; were flushed to disk, and you'd end up writing them multiple times.

foundslot	move.l	d3,cb_SIZEOF(a3,d6.w)	save our key at slot offset
		movea.l	a3,a0			datestamp the directory
		move.l	d5,d0			previous key
		move.l	d2,d1			directory key
		bsr.s	DirStamp		this will loose buffer in a3
		move.l	d3,d0			grab object key again
		bsr	GrabBlockA3
;		movea.l	d0,a3
		lea.l	cb_SIZEOF(a3),a0	reference from end of block
		adda.l	BlockSize(a5),a0
		move.l	d2,vfhb_Parent(a0)	parent = directory key
		move.l	d4,vfhb_HashChain(a0)	next = next header
		lea.l	vfhb_FileName(a0),a0	copy the name over
		moveq.l	#0,d0			copy correct # of bytes
		move.b	(a2),d0
30$		move.b	(a2)+,(a0)+
		dbra	d0,30$
		move.l	d3,d0			to this disk block
		bsr	WriteSummedBlockA3Later	  A3 is buffer
		movem.l	(sp)+,d2-d6/a2-a3
		rts

;==============================================================================
; DirStamp( buf, pkey, dkey )
;	    a0	  d0    d1
;
; Write the given buffer back to memory and timestamp the directory indicated
; by dkey.  The common case of pkey == dkey is optimised.  The lock list is
; also scanned and any locks which refer to dkey have thier access permission
; altered.  The locks will still appear shared due to the way the test are done
;==============================================================================
DirStamp	movem.l	d2-d5/a2,-(sp)
		movea.l	a0,a2			stash buffer address
		move.l	d0,d4			pkey
		move.l	d1,d3			dkey
		moveq.l	#shared.lock,d1		lock type we're looking for
		move.l	LockQueue(a5),d0	get first entry in queue
		beq.s	30$			empty list, stop searching

; loop around looking for locks on the owning directory and change thier type
10$		lsl.l	#2,d0			convert BPTR to APTR
		movea.l	d0,a0
		cmp.l	fl_Key(a0),d3		is this our key ?
		bne.s	20$			no, link to next lock
		cmp.l	fl_Access(a0),d1	is it a shared lock ?
		bne.s	30$			exclusive, so quit loop now
		move.l	#altered.lock,fl_Access(a0)  change the access mode
20$		move.l	(a0),d0			link to next
		bne.s	10$			while there are more

; It's often possible for pkey (predecessor) to be the same as the owning
; directory key because we were first in the hashchain.  Optimise this case.
30$		cmp.l	d4,d3			dkey == pkey ?
		bne.s	40$			nope, no optimisation
		lea.l	cb_SIZEOF(a2),a0	reference from end of block
		adda.l	BlockSize(a5),a0
		cmp.l	RootKey(a5),d4		there is no archive bit...
		beq.s	35$			...in the root block
		bclr.b	#4,vfhb_Protect+3(a0)	clear archive bit
		move.l	vfhb_Parent(a0),d5	save pointer to parent of dir
35$		lea.l	vfhb_Days(a0),a0
		bsr	DatStamp		datestamp given block

; We now write out the previous key.  If it was another entry that was earlier
; on the hash chain, then we won't have changed it's DateStamp or archive bit.
40$		movea.l	a2,a0			write the previous key
					      ; which may be owning directory
		bsr	WriteSummedBlockD4A0Later  Buffer in a0, block in d4

; if pkey <> dkey then we must fetch and update the owning directory header
		cmp.l	d4,d3			was previous also owning dir ?
		beq.s	50$			yes, so we have finished
		move.l	d3,d0			no, grab owning directory key
		bsr	GrabBlock
		movea.l	d0,a2
		lea.l	cb_SIZEOF(a2),a0	reference from end of block
		adda.l	BlockSize(a5),a0
		cmp.l	RootKey(a5),d3		there is no archive bit...
		beq.s	45$			...in the root block
		bclr.b	#4,vfhb_Protect+3(a0)	clear archive bit
		move.l	vfhb_Parent(a0),d5	save pointer to parent of dir
45$		lea.l	vfhb_Days(a0),a0
		bsr	DatStamp		datestamp given block
		movea.l	a2,a0			write the directory key
		move.l	d3,d0			to this block number
		moveq.l	#TRUE,d1		after checksumming
		bsr	WriteBlockLater		looses block in a2

; update the time in the dircache of whatever directory the modified directory
; was in (ugh, another performance hit).
50$		btst.b	#2,DiskType+3(a5)	are we using DCFS?
		beq.s	60$			fast
		move.l	d3,d1			dir
		move.l	d5,d0			parent of dir
		cmp.l	RootKey(a5),d3		is the directory the root?
		beq.s	60$			we were in the root
		bsr	UpdateList		update time in parents dircache
60$		movem.l	(sp)+,d2-d5/a2
		rts

;==============================================================================
; Remove( pkey, okey, dkey, name )
;	   d0    d1    d2    a0
;
; Remove the specified object key (okey) from the predecessors (pkey) chain in
; the given directory (dkey).  Looks at the type of the header and if it's a
; deleted header, discards the block because it's not valid anymore.  It's OK
; for dkey and pkey to reference the same block (an owning directory).
;==============================================================================
Remove		printf <'Remove %ld from %ld, prev = %ld'>,d1,d2,d0
		movem.l	d2-d7/a2-a3,-(sp)
		move.l	d0,d3			save predecessor key
		move.l	d1,d4			and object key
		movea.l	a0,a2			and name
		move.l	d4,d0			grab object header
		bsr	GrabBlockA3
;		movea.l	d0,a3			save buffer address
		lea.l	cb_SIZEOF(a3),a0	reference from end of block
		adda.l	BlockSize(a5),a0
		move.l	vfhb_HashChain(a0),d6	get next in object chain

		moveq.l	#vfhb_HashChain,d0	assume pkey <> dkey
		add.l	BlockSize(a5),d0	and calc offset to hashchain
		cmp.l	d2,d3			was this correct ?
		bne.s	10$			yes, use hashchain slot
		movea.l	a2,a0			nope, entry goes in hashtable
		bsr	Hash			get slot for name

; if this header has just been deleted then we completely scrap it now.
10$		move.w	d0,d7			save offset to 'next' link
		movea.l	a3,a0
		cmpi.l	#t.deleted,cb_SIZEOF+fhb_Type(a0) was it scratched ?
		bne.s	20$			no, so just loose it
		bsr	DiscardBlock		yes, discard it
		bra.s	30$
20$		; a0 - current block
		bsr	LooseKeyBlock		done with block in a0

30$		move.l	d3,d0			now grab predecessors block
		; if d3 is 0, then this entry has already been del-linked.
		beq.s	35$			already done, exit.
		bsr	GrabBlock		could be dir or header
;		movea.l	d0,a0			d7 holds offset to field

		move.l	d6,cb_SIZEOF(a0,d7.w)	link in the object extension
		move.l	d3,d0			pkey
		move.l	d2,d1			dkey
		bsr	DirStamp		stamp owning dir

; now deal with the compressed directory lists.  Remove the list entry AFTER
; removing the entry from the old dir, and add to the new list BEFORE linking
; it into the new dir.
		btst.b	#2,DiskType+3(a5)	are we using fast dirs?
		bne.s	40$
35$		movem.l	(sp)+,d2-d7/a2-a3	can restore regs now
		rts				no, all done
40$
		move.l	d4,d1			delete this key...
		move.l	d2,d0			from this directory...
		movem.l	(sp)+,d2-d7/a2-a3	can restore regs now
		bra	DeleteList		update dir cache list (bsr/rts)


;==============================================================================
; Delete( packet )
;	    d0
;
; Delete an existing file or directory.  Called as coroutine so no regs saved.
; Changed scratch to return the key of the object deleted so we can call notify
;==============================================================================
Delete		; printf <'Delete'>,d0
		movea.l	d0,a2			save packet pointer
		movea.l	dp_Arg1(a2),a0		directory lock
		movea.l	a2,a1			packet
		moveq.l	#TRUE,d0		must be exclusive lock
		bsr	VLock			dir lock to d0 if OK
		movea.l	dp_Arg2(a2),a0		name of object
		adda.l	a0,a0
		adda.l	a0,a0
		movea.l	a2,a1			packet
		bsr.s	Scratch			go scratch the object
		tst.l	d0
		beq.s	10$			object not found

; notify anyone who's got a notify request on this header
		moveq.l	#TRUE,d1		rebuild orphan lists
		bsr	CheckNotify		and do notification (d0,d1)

		moveq.l	#TRUE,d0		deleted OK
10$		movea.l	a2,a0
		bra	WorkDone

;==============================================================================
; success/key = Scratch( dkey, string, packet )
;    d0	  	          d0     a0      a1
;
; The actual implementation of delete.  Remove the specified name from the
; directory header indicated by dkey.  Return TRUE if file is deleted.
;
; NOTE: as far as I can tell, 
;==============================================================================
Scratch		printf <'Scratch from %ld'>,d0
		movem.l	d2-d6/a2-a4,-(sp)
; FIX! filename size!
		lea.l	-32(sp),sp		room for name string
		movea.l	a1,a2			save packet
		movea.l	sp,a1			temp string area
		bsr	FindDir			string in a0, dkey in d0
		move.l	d0,d2			save new dkey

; we need to lock the directory before scanning it.
		bsr	ObtainDir		critical section starting
		move.l	d2,d0			get block # back again

		movea.l	sp,a0			simple name on stack now
		moveq.l	#FALSE,d1		dont followlinks
		bsr	FindEntry
		tst.l	d0			did we find it
		beq	ScratchDone		no, return false (OBJ_NOT_FOUND)

; we found the header key we wanted so read it in and stash the header type
		move.l	PrevKey(a5),d3		previous key
		move.l	ObjKey(a5),d4		object key
		move.l	d4,d0			read in object key
		bsr	GrabBlockA4
;		movea.l	d0,a4			save buffer address
;		movea.l	d0,a0			find the type
		bsr	Type			of this buffer ...
		move.l	d0,d6			... and save it for later

		lea.l	cb_SIZEOF(a4),a0
		adda.l	BlockSize(a5),a0

; we have our header in memory, check that it isn't protected from deletion
		btst.b	#0,vfhb_Protect+3(a0)
		beq.s	ScratchNotProt		OK, we can delete it
		move.w	#ERROR_DELETE_PROTECTED,ErrorCode+2(a5)

ScratchErr	movea.l	a4,a0			not deletable
		bsr	LooseKeyBlock
		moveq.l	#FALSE,d0		and return false
		bra	ScratchDone

; the object is not delete protected.  If it's a directory, make sure its empty.
ScratchNotProt	cmpi.w	#st.userdir,d6
		bne.s	40$			not a directory
		move.l	HTSize(a5),d0		test the whole hashtable
		lea.l	cb_SIZEOF+fhb_HashTable(a4),a0
		bra.s	21$

20$		tst.l	(a0)+
		bne.s	30$			found an entry, error out
21$		dbra	d0,20$			search the next
		bra.s	40$			directory empty

30$		move.w	#ERROR_DIRECTORY_NOT_EMPTY,ErrorCode+2(a5)
		bra.s	ScratchErr		directory not empty, so quit

; We have either a file, an empty dir or a link of some kind.  Check no-one
; else has a lock on it by aquiring an exclusive lock.  Hard links actually
; never get locked by user calls (because they go through Locate which defers
; the lock to the actual file) but it's easier not to special case them here.
40$		move.l	d4,d0
		moveq.l	#exclusive.lock,d1
		bsr	GetLock
		move.l	d0,d5			save the lock ptr
		bne.s	50$			we got it OK
		bra.s	ScratchErr		ERROR_OBJECT_IN_USE already set

; we now have a header that is free for deletion.  If it's a file then we must
; free all the data and extension blocks associated with it unless it has hard
; links pointing to it.  If this is the case then we copy all the file info
; back into the previous link and polymorph it into a real file instead.  The
; header will still be removed though.
;
; We AlterRoot here, since we now know we will delete something.  We must
; do this before modifying the disk structure so the validator can be kicked
; in if needed.  2.04 FFS AlterRoot()ed after Scratch finished.
; Actually, since AlterRoot needs to grab a block, we have to do it after we
; loose the block we're holding...  There are 3 places the block is freed...

50$		cmpi.w	#ST_FLINK,d6		is this a link ?
		beq.s	FixLinks		yes, so handle link chains
		cmpi.w	#ST_DLINK,d6
		bne.s	PolyLink

; We are deleting a simple hard link.  We are going to have to follow the
; BackLink chain from the linked object to remove this hardlink from the list
FixLinks	lea.l	cb_SIZEOF(a4),a0	point to end of block
		adda.l	BlockSize(a5),a0
		move.l	vfhb_BackLink(a0),-(sp)	save the previous block ptr
		move.l	vfhb_Link(a0),-(sp)	and the actual linked object
		movea.l	a4,a0
		move.l	d4,d0
		bsr	LooseBlock		finished with header (a0) now

; AlterRoot grabs a block!
		bsr	AlterRoot		mark bitmap as changed

FixLinksLoop	move.l	(sp)+,d0
		bsr	GrabBlockA3		fetch next file/dir/link header
;		movea.l	d0,a3
		lea.l	cb_SIZEOF(a3),a0
		adda.l	BlockSize(a5),a0
		cmp.l	vfhb_BackLink(a0),d4	does it point to our header
		beq.s	FoundFix		yes, so go unlink it
		move.l	vfhb_BackLink(a0),-(sp)	nope, so search back down chain
		movea.l	a3,a0
		bsr	LooseKeyBlock		done with header block
		bra	FixLinksLoop		keep searching (must be there)

FoundFix	move.l	(sp)+,vfhb_BackLink(a0)	unlink header we are deleting
		movea.l	a3,a0
		bsr	WriteSummedBlockKeyA0Later   Buffer in a0
		bra	ScratchHeader		now delete our link

; we have either a file or a directory, see if we need to copy to a link now.
PolyLink	lea.l	cb_SIZEOF(a4),a0	point to end of block
		adda.l	BlockSize(a5),a0
		move.l	vfhb_BackLink(a0),d0	is there a link to this header?
		beq	EasyDelete		nope, just do a delete

; steve used to modify the first link into the file/dir, and then try to fix
; things up.  This works poorly for linked dirs, since each file in the dir
; has a parent pointer (and OFS files have parent pointers in each data block).
; Also, for dircache each dircache entry has a parent pointer.  Much better is
; to polymorph the file/dir in place of the link, deleting the link.  This only
; requires changing one link pointer (maybe none?), and basically deleting
; and then renaming a file/dir.  This only requires 2 DeleteList and 1
; InsertList calls (the delete of the link and the deletion/insertion of the
; file/dir).

		move.l	a4,a0
		move.l	d4,d0
		bsr	LooseBlock

		bsr	AlterRoot		mark bitmap as changed

; Remove object from it's directory, not freeing any space
	printf <'removing %ld from %ld (prev %ld)'>,d4,d2,d3
		move.l	d3,d0			prev key
		move.l	d4,d1			object
		move.l	sp,a0			name
		bsr	Remove			d2 = directory

; if the attempt doesn't find the Link (it's been deleted or renamed), come
; here and try again.  All jumps to here are after getting a lock on the new
; directory, so free it first.
RetryLinkDelete
		moveq	#0,d3			no prev, in case we try to
;						do 2 Removes

		; Release the lock on the old directory
		move.l	CurrentCo(a5),a0
		lea	co_dsem(a0),a0
		bsr	ReleaseDir

	printf <'lock released'>
		move.l	d4,d0
		bsr	GrabBlockA4		get the "real" block back again

		lea.l	cb_SIZEOF(a4),a0	point to end of block
		adda.l	BlockSize(a5),a0
		move.l	vfhb_BackLink(a0),d0	is there a link to this header?

		; it's theoretically possible the link was deleted out from
		; under us.  If so, nuke the file/dir the old way.  Note that
		; d3=0, so Remove a second time won't hurt...
		beq	EasyDelete

		move.l	d0,a3			temp store link block #
		move.l	d4,d0
		move.l	a4,a0
		bsr	LooseBlock		done with real file for now

		; grab the link just to get what dir it's in for ObtainDir
		move.l	a3,d0			link block
		bsr	GrabBlockA3		get the link
	printf <'grabbed link block %ld'>,cb_Key(a3)
		lea.l	cb_SIZEOF(a3),a1	point to end of link block
		adda.l	BlockSize(a5),a1
		moveq	#32-4,d0		copy filename for FindEntry
1$		move.l	vfhb_FileName(a1,d0.w),0(sp,d0.w)
		subq.w	#4,d0
		bpl.s	1$			loop until d0 is negative
		move.l	vfhb_Parent(a1),d2	parent for calling UpdateList
		bsr	LooseA3Block

		; get lock on the new directory (HOLE! FIX!)
		move.l	d2,d0			parent of new position
		bsr	ObtainDir		lock it...

		; before we locked it the entry may have been deleted or
		; renamed.  Be careful.
		move.l	d2,d0
		move.l	sp,a0			new name (link's)
	printf <'locked dir %ld, looking for %s'>,d2,a0
		moveq.l	#FALSE,d1		dont follow links
		bsr	FindEntry		MUST find link in dir! 
		tst.l	d0
		beq	RetryLinkDelete		must have been deleted/renamed

	printf <'found entry %ld'>,ObjKey(a5)
		; we have it!  Verify that it's the right one!
		; Requires two blocks held!!!!
		move.l	PrevKey(a5),d3		prev key of link
		move.l	ObjKey(a5),d0		object key of link
		bsr	GrabBlockA3		get the block
		move.l	d4,d0			and the new block
		bsr	GrabBlockA4

		lea.l	cb_SIZEOF(a4),a0	point to end of real block
		adda.l	BlockSize(a5),a0
		lea.l	cb_SIZEOF(a3),a1	point to end of link block
		adda.l	BlockSize(a5),a1

		; it's the right block if BackLink points to it.
		move.l	vfhb_BackLink(a0),d0
		cmp.l	cb_Key(a3),d0	    is this the link we're looking for?
		beq.s	3$		    yes!

	printf <'found wrong block, not link, retry (%ld)'>,cb_Key(a3)
		move.l	a3,a0
		bsr	LooseKeyBlock		this wasn't the link
		move.l	a4,a0
		move.l	d4,d0
		bsr	LooseBlock
		bra	RetryLinkDelete		try again...

; We now copy the filename, protection bits, and comment, and owner of this
; link to the parent.
; inherit protection bits, owner, links
; FIX! conflicts with fields inherited in ExNext/ExAll!
3$		move.l	vfhb_Protect(a1),vfhb_Protect(a0)
		move.l	vfhb_BackLink(a1),vfhb_BackLink(a0)
		move.l	vfhb_OwnerXID(a1),vfhb_OwnerXID(a0)
		move.l	vfhb_Parent(a1),vfhb_Parent(a0)
	printf <'Copying link fields to owner'>

		; copy filename and comment in longwords - this trick is
		; better, since it doesn't nuke a0/a1
		; also change name on stack to the new name!
		moveq	#32-4,d0
5$		move.l	vfhb_FileName(a1,d0.w),vfhb_FileName(a0,d0.w)
		move.l	vfhb_FileName(a1,d0.w),0(sp,d0.w)	parent on stack
		subq.w	#4,d0
		bpl.s	5$			loop until d0 is negative

		moveq	#80-4,d0
		lea	vfhb_Comment(a1),a1	vfhb_Comment is too large
		lea	vfhb_Comment(a0),a0	for d8(An,Dn.X)
7$		move.l	0(a1,d0.w),0(a0,d0.w)
		subq.w	#4,d0
		bpl.s	7$			loop until d0 is negative

		movea.l	a4,a0
		move.l	d4,d0			write back polymorphed link
		bsr	WriteSummedBlockLater

		move.l	cb_Key(a3),d0
		move.l	d0,-(sp)		link block
		bsr	LooseA3Block

	printf <'removing %ld from %ld (prev %ld)'>,(sp),d2,d3
		move.l	d3,d0
		move.l	(sp),d1			the old link block
		lea	4(sp),a0		name (link is on stack)
		bsr	Remove			d2 = directory key, d3 = prev

		move.l	(sp)+,d0		free the old link key now
		moveq.l	#1,d1			only one block
		bsr	FreeKeys

		; now that we've removed the old link block, insert the new
		; real block...
		btst.b	#2,DiskType+3(a5)	are we using fast dirs?
		beq.s	40$

	printf <'insertlisting %ld into %ld'>,d4,d2
		move.l	d4,d1			real block
		move.l	d2,d0			directory
		move.l	sp,a0			name
		bsr	InsertList		it HAS to fit, we just deleted
;						one of the exact same size!
;		tst.l	d0			Also, the dir is locked!
;		beq	????			FIX!!!!! (really, really ugly)
; in theory, something could have added to the comment of another entry, in
; which case this _could_ fail.  rename/delete/create can't affect us, since
; they all do an ObtainDir.  If SetComment were to do the same, all would be
; safe and no test for error would be needed here.  FIX!
40$	printf <'Inserting %ld into %ld'>,d4,d2
		move.l	d4,d1			real block
		move.l	d2,d0			directory
		move.l	sp,a0			name
		bsr	Insert

		bra	ScratchFree		free lock, exit

; There are no links to worry about, just clear the file and delete the header
; a0 is the vxxx ptr, a3 is scratch here
EasyDelete	move.l	vudb_DirList(a0),a3	save (maybe) the dirlist
		movea.l	a4,a0			done with this block for now
		bsr	LooseKeyBlock

; AlterRoot grabs a block!
		bsr	AlterRoot		mark bitmap as changed

		cmpi.w	#st.file,d6		is it a file
		bne.s	ScratchHeader		nope, scratch the dirlist
		move.l	d4,d0			this file header
		moveq.l	#0,d1			clear from beginning
	printf <'ClearFile %ld'>,d0
		bsr	ClearFile		clear all data keys

; marked all blocks in the file as free, now alter previous block to header.
ScratchHeader	move.l	d4,d0			grab the header block back
		bsr	GrabBlock
;		movea.l	d0,a0			buffer to write out
		move.l	#t.deleted,cb_SIZEOF+fhb_Type(a0)  mark as deleted

; When we remove a file or directory, we must scan all active ExamineThings to
; make sure none of them are referencing this block, since it might get filled
; with random FFS data (which doesn't checksum well).  Also, if it has a next
; hashchain entry, insert it into the ExamineThing.  This only applies to
; ExamineThings, not NewExamineThings (i.e. FFS not DCFS).
		move.l	a0,-(sp)		save block ptr
		lea.l	cb_SIZEOF(a0),a1
		adda.l	BlockSize(a5),a1	needed to get the hash chain value
		move.l	d4,d0			block being freed
		move.l	vfhb_HashChain(a1),d1
						; parent in d2
		bsr	DeleteFromET		(d0,d1,d2)
		move.l	(sp)+,a0

		;-- in case it flushes to disk
		bsr	WriteSummedBlockD4A0Later  Buffer in a0, block in d4

; Important note about the above WriteBlock call.  We have a buffer on the
; waiting queue that soon isn't going to be valid.  We mark it as deleted so
; that remove will discard it instead of loosing it.  When we get back here
; it's safe to free the object key because it is no longer hanging around.

; if this is a dos4 filesystem, free the dirlist chain.  Do this AFTER
; marking the directory as deleted to make validation easier.  a3 is the
; first dirlist block IF this is st.userdir.
ScratchDir	cmp.w	#st.userdir,d6
		bne.s	RemoveIt
		btst.b	#2,DiskType+3(a5)	are we using fast dirs?
		beq.s	RemoveIt

10$	printf <'freeing dirlist block %ld'>,a3
		move.l	a3,d0			the current dirlist block
		beq.s	RemoveIt		done freeing dirlists
		bsr	GrabBlock
;		move.l	d0,a0			for DiscardBlock
		move.l	cb_SIZEOF+fhl_NextBlock(a0),a2	next block to free
		bsr	DiscardBlock		done with this dirlist
		move.l	a3,d0
		moveq.l	#1,d1
		bsr	FreeKeys		free it
		move.l	a2,a3			now do the next dirlist block
		bra.s	10$

RemoveIt	printf <'Removing %ld from %ld'>,d4,d3
		move.l	d3,d0			prev key
		move.l	d4,d1			object key
		movea.l	sp,a0			name
		bsr	Remove			d2 = directory key

; now free it
		move.l	d4,d0			free the object key now
		moveq.l	#1,d1			only one block
		bsr	FreeKeys

; Finally, free the lock we obtained earlier
ScratchFree:	movea.l	d5,a0			free the lock
		bsr	FreeLock
		move.l	d4,d0			everything worked (returns key)
ScratchDone	lea.l	32(sp),sp		don't need name space now
		movem.l	(sp)+,d2-d6/a2-a4
		rts

;==============================================================================
; Comment( pkt )
;	   d0
;
; Sets the comment, date, owner or protection bits on a file.  Packet is as
; follows:-
;
;	dp_Arg1		***unused***
;	dp_Arg2		lock on directory
;	dp_Arg3		name of object
;	dp_Arg4		BPTR to comment or protect bits or ptr to Date struct
;			or owner bits
;==============================================================================
Comment		movea.l	d0,a2			save packet
		moveq.l	#0,d3			flag (is-comment)
		movea.l	dp_Arg4(a2),a3		get comm,date or prot bits
		cmpi.w	#ACTION_SET_COMMENT,dp_Type+2(a2)
		bne.s	10$
		adda.l	a3,a3			comment is a BSTR
		adda.l	a3,a3
		cmpi.b	#79,(a3)
		ble.s	10$

		move.w	#ERROR_COMMENT_TOO_BIG,ErrorCode+2(a5)
		movea.l	a2,a0
		bra	WorkFail

10$		move.l	a2,d1			stash packet for locate
		movea.l	dp_Arg2(a2),a0		get directory lock
		move.l	dp_Arg3(a2),a1		get file name
		adda.l	a1,a1			convert to APTR
		adda.l	a1,a1
		moveq.l	#SHARED_LOCK,d0
		moveq	#FALSE,d2		not creating, don't lock dir
		bsr	Locate

		movea.l	a2,a0			assume failure
		tst.l	d0
		beq	WorkFail		couldn't get the lock

		movea.l	d0,a4			save the lock ptr
		lsl.l	#2,d0
		movea.l	d0,a0			get the key for this header
		move.l	fl_Key(a0),d2
		bsr	DiskProtected		can't do it if disk protected
		tst.l	d0
		bne.s	20$
		cmp.l	RootKey(a5),d2		can't do this to the root
		bne.s	30$
		move.w	#ERROR_OBJECT_WRONG_TYPE,ErrorCode+2(a5)

20$		movea.l	a4,a0
		bsr	FreeLock
		movea.l	a2,a0
		bra	WorkFail

30$		bsr	AlterRoot		mark disk as caches invalid

		move.l	d2,d0			get this block
		bsr	GrabBlock
;		movea.l	d0,a0			stash it
		move.l	BlockSize(a5),d0
		move.l	cb_SIZEOF+vfhb_Parent(a0,d0.l),d4 remember parent dir

		move.l	dp_Type(a2),d0
		cmpi.w	#ACTION_SET_COMMENT,d0
		bne.s	40$
		lea.l	cb_SIZEOF(a0),a1
		adda.l	BlockSize(a5),a1
		lea.l	vfhb_Comment(a1),a1
		moveq.l	#0,d0
		move.b	(a3),d0			(a3) points to comment
		; check if the comment is the same size
		cmp.b	(a1),d0
		beq.s	35$
		moveq	#1,d3			same size, we can just update
35$		move.b	(a3)+,(a1)+
		dbra	d0,35$
		bra.s	CommentDone		now write block back

40$		lea.l	cb_SIZEOF(a0),a1
		adda.l	BlockSize(a5),a1
		cmpi.w	#ACTION_SET_DATE,d0
		bne.s	50$
		move.l	(a3)+,vfhb_Days(a1)
		move.l	(a3)+,vfhb_Mins(a1)
		move.l	(a3)+,vfhb_Ticks(a1)
		bra.s	CommentDone

50$		move.w	#vfhb_Protect,d1	assume protection
		cmpi.w	#ACTION_SET_PROTECT,d0
		beq.s	60$
		move.w	#vfhb_OwnerXID,d1	no, it's the owner UID/GID
60$		move.l	a3,0(a1,d1.w)		set protection or owner

CommentDone	move.l	d2,d0
		moveq.l	#TRUE,d1
		bsr	WriteBlockLater		writes buffer in a0

		btst.b	#2,DiskType+3(a5)	are we using fast dirs?
		beq.s	setxxx_exit		no, we're done

		; now that the file is updated, update the dircache
		tst.l	d3
		bne.s	changed_comment		actually comment length changed
		move.l	d2,d1			object to update
		move.l	d4,d0			directory it's in
		bsr	UpdateList		update directory cache
		bra.s	setxxx_exit

changed_comment: ; we changed the comment length.  We must delete the entry
		; and reinsert it.  The disk should be marked as invalid!
		move.l	d2,d1
		move.l	d4,d0
		bsr	DeleteList		delete old entry
		move.l	d2,d1
		move.l	d4,d0
		sub.l	a0,a0			name in fhb is correct
		bsr	InsertList		insert it back

setxxx_exit:	movea.l	a4,a0
		bsr	FreeLock
		movea.l	a2,a0
		moveq.l	#TRUE,d0
		bra	WorkDone

;==============================================================================
; ClearFile( key,entry )
;	     d0   d1
;
; Frees up all the data keys and extension blocks belonging to the given header
; The header is not removed from its parent directory so we can support open
; MODE_NEWFILE without having to go through all the overhead of deleting it.
; The header only has data keys removed from entry onwards (setfilesize) and
; does not have to be a file header (it can be an extension block itself).
; This routine does not set the fhb_ByteSize of the file.
;==============================================================================
ClearFile	movem.l	d2-d6/a2-a3,-(sp)
		move.l	d0,d5			save the given file header
		move.l	d1,d6			and starting offset
		move.l	d5,d2			and keep d2=current header
;	printf <'ClearFile(%ld,%ld)\n'>,d0,d1

ClearLoop	bsr	GrabBlock		and fetch that block
		movea.l	d0,a2

; First free all of the data keys referenced by this header block.  Attempt to
; free them in contiguous chunks because it's quicker and can help to prevent
; disk fragging if another process has a simultaneous write going to this disk
		move.l	cb_SIZEOF+fhb_HighSeq(a2),d3	number of data blocks
		beq.s	DataCleared		no data left to clear
		cmp.l	d2,d5			is this the header key ?
		bne.s	ClearAll		no, so clear all of it

; this is the header key so we may have to leave some data keys hanging around
		sub.l	d6,d3			this many left to clear
		beq.s	DataCleared
;	printf <'%ld keys to clear in header\n'>,d3
		lea.l	cb_SIZEOF+fhb_HashTable(a2),a3
		move.l	HTSize(a5),d0		point at hashtable
		sub.l	d6,d0			point at first entry to clear
		lsl.l	#2,d0
		adda.l	d0,a3
		bra.s	DataLoop

; we're on to the next extension block(s) so we clear out the whole thing
ClearAll	; printf <'Clearing whole hash table\n'>
		clr.l	cb_SIZEOF+fhb_HighSeq(a2)
		lea.l	cb_SIZEOF+fhb_HashTable(a2),a3
		move.l	HTSize(a5),d0
		lsl.l	#2,d0
		adda.l	d0,a3			a3 points beyond first entry

; this loop frees all of the individual frags in the current extension block
DataLoop	moveq.l	#1,d1			number of data blocks to free
		subq.w	#1,d3			allow for that in main loop
		move.l	-(a3),d0		fetch that starting entry
		move.l	d0,d4			and keep track of current value
		clr.l	(a3)			no data in this slot now
		bra.s	CollectEnter

ClearCollect	addq.l	#1,d4			bump current block number
		cmp.l	-4(a3),d4		is next one contiguous
		bne.s	FreeCollect		nope, free collected blocks
		addq.l	#1,d1			yes, one more block to do
		clr.l	-(a3)			zero out this slot
CollectEnter	dbra	d3,ClearCollect		keep looking while blocks left

; hit a frag so loose the current extension while we free the keys in bitmap.
FreeCollect	movem.l	d0-d1,-(sp)		save the key arguments
		movea.l	a2,a0			loose this block
;***bug fix.  Can't hold stashed address in a3.  Convert to an offset and add
;***back in when we get the cache buffer again.
		suba.l	a2,a3			convert a3 to offset
		move.l	d2,d0
		moveq.l	#TRUE,d1
		bsr	WriteBlockLater		write in case it's the header
		movem.l	(sp)+,d0-d1
		bsr	FreeKeys		d0=startkey, d1=keycount
		move.l	d2,d0			fetch the extension again
		bsr	GrabBlock
		movea.l	d0,a2
		adda.l	a2,a3			convert offset back to address
		addq.w	#1,d3			allow for key we didn't free
		bne.s	DataLoop		d3 wasn't -1 so keep going

; We've freed all the data from the current extension.  We should now free the
; extension block itself unless it's the file header, which we special case.
DataCleared	cmp.l	d2,d5			is this the file header ?
		bne.s	ClearExtension		nope, doing a normal header

	; printf <'Setting HighSeq to %ld\n'>,d6
		lea.l	cb_SIZEOF(a2),a0	clear out size etc
		move.l	d6,fhb_HighSeq(a0)	new highseq number
		bne.s	NotAllCleared		there's some data left
		clr.l	fhb_FirstBlock(a0)	no first block
	; printf <'Zeroed firstblock\n'>

NotAllCleared	adda.l	BlockSize(a5),a0	point to end of block
		clr.l	vfhb_ByteSize(a0)	no data in file
		move.l	vfhb_Extension(a0),d2	save extension key
		clr.l	vfhb_Extension(a0)	and clear it out
		movea.l	a2,a0			buffer to write out
	; printf <'Writing header back out\n'>
		bsr	WriteSummedBlockD5A0Later  Buffer in a0, block in d5
		bra.s	ClearMore		see if there's any more to do

; we have a simple extension block, just fetch the next extension and free this
ClearExtension	lea.l	cb_SIZEOF(a2),a0	fetch next extension
		adda.l	BlockSize(a5),a0
		move.l	vfhb_Extension(a0),-(sp)	and save it
		movea.l	a2,a0			finished with this block		
		bsr	DiscardBlock
		move.l	d2,d0			free this from bitmap
		moveq.l	#1,d1			only one block
		bsr	FreeKeys
		move.l	(sp)+,d2		fetch next extension
ClearMore	move.l	d2,d0			is there another ?
		bne	ClearLoop		yes, free that too
		movem.l	(sp)+,d2-d6/a2-a3
		rts

;==============================================================================
; MakeListEntry( key, entry, name, vfhb)
;		  d0   a0     d1    a1
;
; Build a list entry in (a0), given vfhb pointer in a1, for key in d0.
; d1 is an APTR to a BSTR name - don't use the one in the vfhb since it may be
; wrong!!!!  However, if d1 is NULL, we use the name from the block.
;
; If last byte is an even byte, it adds a 0 byte (we know it's an even size)!
; This makes compares against entries in the list work!
;==============================================================================
MakeListEntry	move.l	a2,-(sp)
		move.l	d0,fle_Key(a0)			save key value of entry
		move.l	vfhb_Protect(a1),fle_Protection(a0)
		move.l	vfhb_ByteSize(a1),fle_Size(a0)
		move.l	vfhb_OwnerXID(a1),fle_OwnerXID(a0)
		move.w	vfhb_Days+2(a1),fle_Days(a0)	only use low word of
		move.w	vfhb_Days+6(a1),fle_Days+2(a0)	date! Gives us 179 years
		move.w	vfhb_Days+10(a1),fle_Days+4(a0)
		move.b	vfhb_SecType+3(a1),fle_Type(a0)	all types fit in byte!

		tst.l	d1			were we passed a name?
		bne.s	name_bad
		lea	vfhb_FileName(a1),a2
		bra.s	name_good
name_bad:	move.l	d1,a2			copy name
name_good:	lea	fle_FileName(a0),a0	followed by comment
		moveq.l	#0,d0
		move.b	(a2),d0			number of bytes (+1) to copy
1$		move.b	(a2)+,(a0)+
		dbra	d0,1$			copys d0+1 bytes

		lea	vfhb_Comment(a1),a2	copy comment
		moveq.l	#0,d0
		move.b	(a2),d0			a2 has position for comment
2$		move.b	(a2)+,(a0)+		comments are rare
		dbra	d0,2$			usually doesn't loop

		; check alignment of last byte
		move.l	a0,d1
		moveq	#1,d0
		and.l	d1,d0
		beq.s	3$			last byte written was odd
		clr.b	(a0)			so we can byte-compare with
;						entries!
3$		move.l	(sp)+,a2
		rts


;==============================================================================
; ListEntrySize( vfhb, name)
;		  a0    a1
;
;	Calculates the space needed to store the list entry.  Fixed plus
; length of filename and comment plus 1 length byte each, rounded up to
; multiple of 2 (for word-alignment).  MakeListEntry depends on this boing
; rounded up to an even size!
; a1 is an APTR to a BSTR name - don't use the one in the vfhb since it may be
; wrong!!!!  If a1 is NULL, then use the name from the fhb.
;==============================================================================
ListEntrySize	moveq.l	#fle_Fixed+2,d0		size of fixed entries in entry
						; +2 for len bytes of name,comm
		; we know this fits in signed byte!!!!  Tricky!
		move.l	a1,d1			were we passed a name?
		bne.s	1$
		lea	vfhb_FileName(a0),a1	use name from fhb
1$		add.b	(a1),d0			add filename length to size
		add.b	vfhb_Comment(a0),d0	add comment length

		; since this may be odd, round up (add size&1 to size)
		moveq.l	#1,d1
		and.b	d0,d1
		add.l	d1,d0
		rts

;==============================================================================
; NextListEntry( entry, count )
;		  a0     d6.w
;
; Get next entry in list from current entry.  Updates a0 and d6 both!
;==============================================================================
NextListEntry	; skip over entry (can't be >256 long!)
		moveq.l	#fle_Fixed+2,d0		+2 for the two length bytes
		add.b	fle_FileName(a0),d0	filename length byte
		add.b	-1(a0,d0.w),d0		-1: we already added len bytes
		moveq.l	#1,d1
		and.l	d0,d1
		add.l	d1,d0			round up to word
		add.l	d0,a0			next entry pointer
		subq.w	#1,d6			1 less entry
		rts

;==============================================================================
; InsertListMissing( okey, dirblock, dkey )
;		      d0      a4      d2
;
; Insert the given object key (okey) into the given directory key (dkey) list.
; This is called on validate if the object isn't in the directory list.
; Duplicates some of the initial code then merges.
;
; FIX! we need locking around the update of the list!
;
; Returns true if it succeeded, or false if it couldn't allocate a block.
;==============================================================================
InsertListMissing:	printf <'InsertListMissing %ld into %ld'>,d1,d0
		movem.l	d2-d7/a2-a4,-(sp)
		move.l	d0,d3			and okey
		move.l	BlockSize(a5),d1
		move.l	cb_SIZEOF+vudb_DirList(a4,d1.l),d4  get list head ptr
		beq	InsertFail		should never happen!!!!
		; we assume that all dirs MUST have DirList entries!!!!
		; don't need a4 any more

		; d0 still has okey in it
		moveq	#1,d7			Validator InsertList
		moveq	#0,d6			name in fhb is correct
		bra.s	InsertList_common

;==============================================================================
; InsertList( dkey, okey, name )
;	       d0    d1    a0
;
; Insert the given object key (okey) into the given directory key (dkey) list.
; The list should always be written after writing the fileheader and before
; the directory.  Thus we only write the directory and fileheader once, and
; we know that the fileheaders linked to the directory are the canonical list
; of things in the directory.
;
; Insertion scheme can be played with.  To start, we'll try first-fit.  We
; could sort (requires rewriting 50% of list blocks, rounded up), or compact,
; or use best-fit (requires reading all list blocks), etc.  First-fit requires
; writing one or (maybe) two blocks, and only read the minimum number.  We
; depend on DeleteList compacting the entries within a given block.
;
; The name is passed, since InsertList is called before Insert, which actually
; sets the name (on both create and on rename).  If name is NULL, it uses
; the name from the entry!
;
; FIX! we need locking around the update of the list!
;
; Returns true if it succeeded, or false if it couldn't allocate a block.
;==============================================================================
InsertList:	printf <'InsertListing %ld into %ld'>,d1,d0
		movem.l	d2-d7/a2-a4,-(sp)
		move.l	d0,d2			stash dkey
		move.l	d1,d3			and okey
		move.l	a0,d6			fhb may not have name!
		bsr	GrabBlockA3		grab the directory block
;		move.l	d0,a3			save buffer address
		move.l	BlockSize(a5),d0
		move.l	cb_SIZEOF+vudb_DirList(a3,d0.l),d4  get list head ptr
		beq.s	InsertFail		should never happen!!!!
		; we assume that all dirs MUST have DirList entries!!!!

		move.l	d2,d0			associate with itself
		bsr	LooseA3Block		done with dir

		; build new entry for list (so we know how long it is)
		; ASSUMES entry must be <= 256 bytes!
		move.l	d3,d0

		; InsertListMissing joins here
		moveq	#0,d7			normal insertlist
InsertList_common:
		bsr	GrabBlockA3		get fileheader
;		move.l	d0,a3
		lea	cb_SIZEOF(a3),a0
		adda.l	BlockSize(a5),a0

		move.l	a0,a2			don't lose pointer
		move.l	d6,a1			must pass it the name!

		bsr	ListEntrySize		How much space needed?
						; d0 is a multiple of 2!
		sub.l	d0,sp			allocate space on stack 
 		move.l	sp,a4			pointer to entry in a4
		move.l	d0,d5			save size for free space scan

		; now that we've allocated space for it, build the entry
		move.l	a4,a0			entry space
		move.l	a2,a1			vfhb pointer
		move.l	d3,d0			key of entry
		move.l	d6,d1			name pointer (vfhb may be wrong)
		bsr	MakeListEntry		build entry from vfhb
						; d6 is free now
		; all built, free fhb
		move.l	d3,d0			associate with itself
		bsr	LooseA3Block

		; loop until we find a fit or fail (allocate new block)
listblkloop	move.l	d4,d0			current block
		bsr	GrabBlockA3		get a list block
;		move.l	d0,a3			save block address
		lea	cb_SIZEOF+fhl_Entries(a3),a0	get start of entries
		move.l	a0,a2			save address
		move.l	cb_SIZEOF+fhl_NumEntries(a3),d6 get number of entries

listloop	; are we out of entries to skip over in this block?
		tst.w	d6
		beq.s	found_end
		bsr	NextListEntry		(a0,d6.w) updates both
		bra.s	listloop

		; no more entries in this block.  See if we fit, or get next
found_end	moveq	#-fhl_Entries,d0	we start having used these (overhead)
		add.l	a2,d0			calculates negative of
		sub.l	a0,d0			bytes used by entries
		add.l	BlockSize(a5),d0	number of bytes left in block
		cmp.l	d5,d0
		bge.s	GotSpace		enough space at end! (a0=dest)

		; not enough space in block, get next block
		move.l	cb_SIZEOF+fhl_NextBlock(a3),d0	next block key
		bne.s	GotNextList

		; no next block!  Allocate a new one.
		move.l	d4,d0			can't hold it over AllocKeys
		bsr	LooseA3Block		drop old block we held

		move.l	d4,d0			old block (put near here)
		move.l	d7,d1			validator flag
						; d2 is parent key
		bsr.s	MakeListBlock		returns a0-buff,d0-key (or 0)
		move.l	d0,d4			did we succeed?
		bne.s	MadeNewList

InsertFail	moveq	#FALSE,d0		we failed
InsertExit	add.l	d5,sp			drop entry from stack
		movem.l	(sp)+,d2-d7/a2-a4
		rts

GotNextList	; d0 is next list block
		exg.l	d4,d0			new block number in d4
		bsr	LooseA3Block		don't need previous list block
		bra	listblkloop		repeat scan for new block

MadeNewList	; d0 is still block #
		bsr	GrabBlockA3
;		move.l	d0,a3			save new buffer
		lea	cb_SIZEOF+fhl_Entries(a3),a0	first entry space
		move.l	BlockSize(a5),d0	so we know it's safe to add
		;				a LW of NULL at end!
		; fall through

		; we have enough space in this block (pointed to by a0)
		; d6 is current number of entries, a3 is block, d4 is block num
		; d0 is the number of bytes left in this block, or BlockSize
		; if it's a new block.
GotSpace	move.l	d0,d2			save space left in block
		move.l	a4,a1			source of entry
		move.l	d5,d0			size in bytes
		bra.s	2$			so we copy exactly d0 bytes
1$		move.b	(a1)+,(a0)+
2$		dbra	d0,1$

		; this is so DeleteList won't wander off into never-never
		; land after an insert.
		; see if we have space to add LW of NULL at end
		sub.l	d5,d2			space left in block
		subq.l	#4,d2			space we need
		bmi.s	10$			not enough space!
		clr.l	(a0)

10$		addq.l	#1,cb_SIZEOF+fhl_NumEntries(a3)	bump count
		bsr	WriteSummedBlockD4A3Later   Block in D4, ptr in A3

		moveq.l	#TRUE,d0		success!
		bra	InsertExit		and exit

;==============================================================================
; MakeListBlock( key, validator, parent )
;	         d0    d1          d2
;
; Make a new list block (calls AllocListKey).  Store at <offset> in block d0.
; initialize the block and return block number in d0 (or 0 for failure).
; offset is determined by the type of block of parent (dir or dirlist).
;
; To avoid problems on validate, we must make sure the new block is written
; out in a safe form before the old block is modified.  Use WriteBlockNow
; to make sure it isn't grabbed before it can be written.  Too bad there isn't
; a way to tell it "make sure this block is ahead of that one".
;==============================================================================
MakeListBlock	printf <'MakeListBlock in %ld, offset %ld, parent %ld'>,d0,d1,d2
		movem.l	d4-d5/a2,-(sp)
		move.l	d0,d5			allocate near last block
		bsr	AllocListKey		d1 is validator flag
		move.l	d0,d4			did we get one?
		beq.s	MakeExit		no

		; new key is stored in d4
		bsr	GetBlock		gets a cache block
		move.l	d0,a2			new current block
		lea	cb_SIZEOF(a2),a0	clear out block
		bsr	ClearBlock		numentries=0, next=0
		move.l	#t.dirlist,cb_SIZEOF+fhl_Type(a2)
		move.l	d2,cb_SIZEOF+fhl_Parent(a2)
		move.l	d4,cb_SIZEOF+fhl_OwnKey(a2)
		move.l	a2,a0			buffer it's in
; FIX!!!!! this should force the block out!!!!
		bsr	WriteSummedBlockD4A0Later  Buffer in a0, block in d4

		; have new block.  Update current one, then get new one back
		move.l	d5,d0
		bsr	GrabBlock		get old block to add ptr
;		move.l	d0,a0			leave in a0 for WriteBlockLater

		moveq	#t.short,d0
		cmp.l	cb_SIZEOF+udb_Type(a0),d0	is it a dir?
		bne.s	10$			no, must be a dirlist
		move.l	BlockSize(a5),d0	update ptr in old dir block
		move.l	d4,cb_SIZEOF+vudb_DirList(a0,d0.l) 
		bra.s	20$
10$		move.l	d4,cb_SIZEOF+fhl_NextBlock(a0)	update dirlist block

20$		bsr	WriteSummedBlockD5A0Later  Buffer in a0, block in d5
		move.l	d4,d0			return new block #
MakeExit:
		movem.l	(sp)+,d4-d5/a2
		rts

;	DEBUGENABLE

;==============================================================================
; success = UpdateListIfDifferent( okey, dir block, dkey )
;				    d0      a4       d2
;
; For Validator.
; Update the given object key (okey) in the given directory key (dkey) list,
; if it's different.  If comment changed size, make fhb match dirlist!!!!
; if the object is NOT in the list, return failure (needs to be added).
;
; Doesn't release the directory!
;
; FIX!!! this requires multiple cache blocks!
;==============================================================================
UpdateListIfDifferent:
		movem.l	d2-d6/a2-a4,-(sp)
	printf <'updatelistifdifferent: %ld in %ld\n'>,d0,d2
		move.l	d0,d3			object key
		move.l	BlockSize(a5),d0
		move.l	cb_SIZEOF+vudb_DirList(a4,d0.l),d4  get list head ptr
		; we assume that all dirs MUST have DirList entries!!!!

		bra.s	update_common

;==============================================================================
; success = UpdateList( dkey, okey )
;	                 d0    d1
;
; Update the given object key (okey) in the given directory key (dkey) list.
; The list should always be written after writing the fileheader and before
; the directory.  Thus we only write the directory and fileheader once, and
; we know that the fileheaders linked to the directory are the canonical list
; of things in the directory.
;
; The filename and other fields can't change size.  However, a comment change
; may require more (or less) space.  Thus ACTION_SET_COMMENT should use
; DeleteList then InsertList instead of using UpdateList.
;
; FIX! we need locking around the update of the list!
;==============================================================================
UpdateList:	movem.l	d2-d6/a2-a4,-(sp)
	printf <'updatelist: %ld in %ld\n'>,d1,d0
		move.l	d0,d2			save args
		move.l	d1,d3
		bsr	GrabBlockA3		get the directory block
;		move.l	d0,a3
		move.l	BlockSize(a5),d0
		move.l	cb_SIZEOF+vudb_DirList(a3,d0.l),d4  get list head ptr
		; we assume that all dirs MUST have DirList entries!!!!

		move.l	d2,d0
		bsr	LooseA3Block		done with directory

		; updatelistifdifferent enters here!
update_common:	move.l	d3,d0
		bsr	GrabBlockA3		get fileheader
;		move.l	d0,a3

		move.l	BlockSize(a5),d0
		lea	cb_SIZEOF(a3,d0.l),a0	vfhb pointer

		move.l	a0,a2			save for later
		sub.l	a1,a1			name in fhb is correct
		bsr	ListEntrySize		Calculate sizeof list entry

		move.l	d0,d5			size of entry
		sub.l	d0,sp			make space on stack for entry
		move.l	sp,a4			entry pointer

		move.l	d3,d0			key of data block
		moveq.l	#0,d1			name pointer in vfhb is correct
		move.l	a4,a0			entry pointer
		move.l	a2,a1			vfhb pointer
		bsr	MakeListEntry		build list entry from vfhb

		move.l	d3,d0
		bsr	LooseA3Block		done with actual fhb

		; now that we've built the new entry, find where to put it
update_loop	move.l	d4,d0
		bsr	GrabBlockA3		get the next list entry
;		move.l	d0,a3
		lea	cb_SIZEOF+fhl_Entries(a3),a0	get start of entries
		move.l	cb_SIZEOF+fhl_NumEntries(a3),d6 get number of entries

		; check each entry in this block for the key we want
1$		tst.w	d6
		beq.s	update_next

;	printf <'Comparing %ld to %ld\n'>,fle_Key(a0),d3
		cmp.l	fle_Key(a0),d3
		beq.s	DoUpdate
		bsr	NextListEntry		(a0,d6.w) updates both
		bra.s	1$

update_next	; get next list block
		move.l	cb_SIZEOF+fhl_NextBlock(a3),d0	next block key
		exg.l	d4,d0			leaves next block in d4
		bsr	LooseA3Block		done with old list block
		tst.l	d4			do we have a next block?
		bne.s	update_loop
		; no dirlist block held
		bra	TotalFailure		only possible in ULIDifferent

DoUpdate	; we have the entry to update in a0
		; if we were called from UpdateListIfDifferent, check
		; if comment length is different - if so, rewrite FHB.
		; we'll check it all the time, since it should never get
		; here otherwise with the comment different.  (and it's cheap)
		move.l	a4,a1			new entry
		move.l	a0,-(sp)		save dest ptr
		moveq	#0,d0
		lea	fle_FileName(a1),a1
		move.b	(a1),d0			skip over filename
		lea	1(a1,d0.w),a1		and length byte
		lea	fle_FileName(a0),a0
		move.b	(a0),d0			skip over filename
		lea	1(a0,d0.w),a0		and length byte

		; ok we have comment ptrs now - compare lengths
		move.b	(a0),d0
		cmp.b	(a1),d0
		beq.s	comments_same		ah, good (dest ptr on stack)

		; comments are different! update fhb!
	printf <'comments different!!'>
; FIX!!!! holds (another) buffer across GrabBlock!!!  This can mean dir,
; dirlist and fhb all at once here!  Minimum 3 cache blocks needed!
		move.l	a0,-(sp)		save dirlist comment ptr
		move.l	d3,d0
		bsr	GrabBlockA4		get fhb
;		move.l	d0,a4
		lea	cb_SIZEOF+vfhb_Comment(a4),a1	comment pointer
		add.l	BlockSize(a5),a4
		move.l	(sp)+,a0		dirlist comment
		moveq	#0,d0
		move.b	(a0),d0
10$		move.b	(a0)+,(a1)+		copy comment (length+1 bytes)
		dbra	d0,10$

		; fhb modified, now write it out
		move.l	a4,a0
		move.l	d3,d0			associate with itself
		moveq	#TRUE,d1		update checksum
		bsr	WriteBlockLater

		; the rest MUST be the same, as we died in a SetComment!
		; free list block and exit
		addq.w	#4,sp			drop destination ptr!
		bra.s	UpdateCancel

		; don't update if entries are identical!
comments_same	move.l	(sp)+,a0		destination
		move.l	a4,a1			new entry
		move.l	d5,d0			size of entry (words)
		moveq	#0,d1			flag if changed
		bra.s	20$			make sure we copy the right #
10$		cmp.b	(a1)+,(a0)+
		beq.s	20$			the same
		moveq	#1,d1			changed
		move.b	-1(a1),-1(a0)
20$		dbra	d0,10$

		; did we make any changes?
		tst.w	d1
		bne.s	UpdateWrite
UpdateCancel	; printf <'no changes to %ld'>,d4
		move.l	d4,d0			associate with self
		bsr	LooseA3Block		no changes, don't write it
		bra.s	UpdateExitOK		not really, just exiting

		; updated, now write it out and exit
UpdateWrite	bsr	WriteSummedBlockD4A3Later   Block in D4, ptr in A3

UpdateExitOK:	moveq	#1,d0
UpdateExit:	add.l	d5,sp			drop entry from stack
		movem.l	(sp)+,d2-d6/a2-a4
		rts

TotalFailure:	; if we fail, at least return.  If this isn't ULIDifferent,
		; something must be SERIOUSLY screwed!

		; were we called by UpdateListIfDifferent?  If so, since we
		; didn't find the entry, we need to add it to the dirlist.
		moveq	#0,d0
		bra.s	UpdateExit

;==============================================================================
; DeleteList( dkey, okey )
;	       d0    d1
;
; Delete the given object key (okey) from the given directory key (dkey) list.
; The list should always be written before the directory.  Thus we know that
; the fileheaders linked to the directory are the canonical list of things in
; the directory.
;
; To make it easier to search, we compact the entries on each delete so the
; free space is always at the end of the block.  Also, to make undelete
; easier, we'll store deleted keys at the end of the list terminated by a
; longword 0.
;
; If a block becomes empty, we'll leave it allocated.  It would be nicer to
; delete empty blocks.  FIX?
;
; FIX! we need locking around the update of the list!
;==============================================================================
DeleteList	printf <'deleting %ld from %ld'>,d1,d0
		movem.l	d2-d6/a2-a4,-(sp)
		move.l	d0,d2			save args
		move.l	d1,d3
		bsr	GrabBlockA3		get the directory block
;		move.l	d0,a3
		move.l	BlockSize(a5),d0
		move.l	cb_SIZEOF+vudb_DirList(a3,d0.l),d4  get list head ptr
		; we assume that all dirs MUST have DirList entries!!!!

		move.l	d2,d0
		bsr	LooseA3Block		done with directory

		; find the block to delete from
delete_loop	move.l	d4,d0
		bsr	GrabBlockA3		get the next list entry
;		move.l	d0,a3
		lea	cb_SIZEOF+fhl_Entries(a3),a0	get start of entries
		move.l	cb_SIZEOF+fhl_NumEntries(a3),d6 get number of entries

		; check each entry in this block for the key we want
1$		tst.w	d6
		beq.s	delete_next

;	printf <'Comparing against %ld'>,fle_Key(a0)
		cmp.l	fle_Key(a0),d3
		beq.s	DoDelete
		bsr	NextListEntry		(a0,d6.w) updates both
		bra.s	1$

delete_next	; get next list block
;	IFD DEBUG_CODE
;		tst.l	cb_SIZEOF+fhl_NextBlock(a3)
;		bne.s	1$
;		printf <'DeleteList Failure!!! didn't find entry %ld!'>,d3
;1$
;	ENDC
		move.l	cb_SIZEOF+fhl_NextBlock(a3),d0	next block key
		beq.s	DeleteListFailure	FIX! is this possible????
		exg.l	d4,d0			leaves new block in d4
		bsr	LooseA3Block		done with old list block
		bra.s	delete_loop

DoDelete	; we have the entry to delete in a0
		; find the size of the entry, and then copy down the rest
;	printf <'Found %ld in dirlist %ld'>,d3,d4
		subq.l	#1,cb_SIZEOF+fhl_NumEntries(a3) update number of entries
		; don't skip the copy if there are no entries left - we
		; want to copy down the list of deleted blocks!

		move.l	a0,a2			save current pointer
		bsr	NextListEntry		(a0,d6.w) updates both
		move.l	a0,a4			save for later
		move.l	BlockSize(a5),d0
		lea	cb_SIZEOF(a3,d0.l),a1	get pointer to end of block
		move.l	a1,d2			save end ptr for later
		sub.l	a0,a1			leaves number of bytes
		move.l	a1,d0			number of bytes in entries
		move.l	a2,a1			destination (a0 already source)
		bsr.s	CopyOverlap		and copy it (may be large)

		; ok, now add the entry to the end of the null-terminated
		; list of deleted blocks that follows the last entry.
		; there's always space for this.
		move.l	a2,a0
		bra.s	20$
10$		bsr	NextListEntry		(a0,d6.w)
20$		tst.w	d6
		bne.s	10$

		; one other case: if there was NO space at the end before the
		; delete, we have to just drop it in there.  d2 has ptr to the
		; end of the buffer, a4-a2 is space the deleted entry took,
		; a0 is pointer past the last entry in the dircache.
		sub.l	a2,a4			how much space did the
		add.l	a0,a4			 deleted entry take?
		addq.w	#3,a4			since it may not be a mult of 4
		; a4 = last entry + deleted space + 3
		cmp.l	a4,d2			is last+deleted==EndOfBuffer?
		bls.s	forceit			branch on d2 <= a4

		; ok, a0 points to past the last entry.  Find first NULL.
30$	; printf <'--%ld'>,(a0)
		tst.l	(a0)+
		bne.s	30$
		subq.w	#4,a0			so we can share following

		; save the deleted entry key
forceit		move.l	d3,(a0)			put key on end of list
		clr.l	4(a0)			terminate list

		; updated, now write it out and exit
		bsr	WriteSummedBlockD4A3Later   Block in D4, ptr in A3
DeleteListFailure:
		movem.l	(sp)+,d2-d6/a2-a4
		rts

;==============================================================================
; CopyOverlap (src, dest, size )
;              a0    a1    d0
;
; Like CopyMem(), but is guaranteed front to back for overlapping copies
; size==0 copies nothing.  Space over time.
;
; exec/CopyMem doesn't have a guaranteed order
;==============================================================================
copy_start	move.b	(a0)+,(a1)+
CopyOverlap:	dbra	d0,copy_start		tricky, eh?  
		rts

		END
