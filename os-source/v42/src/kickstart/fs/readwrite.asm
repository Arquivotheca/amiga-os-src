		SECTION	filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/nodes.i"
		INCLUDE	"exec/lists.i"
		INCLUDE	"exec/io.i"
		INCLUDE	"libraries/dos.i"
		INCLUDE	"libraries/dosextens.i"

		INCLUDE	"globals.i"
		INCLUDE	"private.i"
		INCLUDE	"readwrite.i"
		INCLUDE	"moredos.i"
		INCLUDE	"actions.i"

		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XDEF	Open,NextJob,ClearWaiting

		XREF	ReadFFS,WriteFFS,SeekFFS,SetSizeFFS
		XREF	ReadOFS,WriteOFS,SeekOFS,SetSizeOFS
		XREF	LockRecord,FreeRecord,RelockRecord

		XREF	GrabBlock,WaitBlock,LooseBlock,DoInfo,Type
		XREF	GrabBlockA3,GrabBlockA4
		XREF	DatStamp,Locate,Create,FreeLock,GetLock
		XREF	WorkFail,WaitCo,WorkDone,FreeAllRLocks,ParentKey
		XREF	WriteBlockNow,ReturnPkt,CallCo,UnHash,WriteBlockLater
		XREF	UpdateList,FreeBuffer

		XREF	GetPubMem,_LVOFreeMem,WaitFor,CheckNotify

;==============================================================================
; Open(packet)
;	 a0
;
; Takes an Open packet and attempts to open a file in the corresponding mode.
; This should be run as a co-routine though it will defer subsequent packets to
; a different co-routine if the file is already opened.  Once the file has been
; succesfully opened (or created) a co-routine is left sitting around to wait
; for incoming read/write/seek/close etc. packets.  These are queued on the
; appropriate rwheader so they are properly synchronised.
;
; d4 is always the same as rwd_FileHeader
;==============================================================================
Open		movea.l	d0,a2			stash DOS packet address
		move.l	dp_Type(a2),d0		get the command
; we have a few possibilities for the Open command and modes.  It can be:-
; ACTION_OPEN_LOCK	use the provided lock as the file to be opened
; ACTION_FINDINPUT	open existing file for read (fail if not present)
; ACTION_FINDOUTPUT	open any file for write, always create from scratch
; ACTION_FINDUPDATE	open any file for read/write, create if it doesn't
;			exist but don't destroy it if it does exist.
		movea.l	dp_Arg2(a2),a3		assume we have a lock
		cmpi.w	#ACTION_OPEN_LOCK,d0	do we have the lock already ?
		beq.s	OpenGotLock		yes, open will succeed
		cmpi.w	#ACTION_FINDOUTPUT,d0	simple open for write
		beq.s	GetWriteLock		yes, create file from scratch
		movea.l	dp_Arg2(a2),a0		directory lock to a0
		movea.l	dp_Arg3(a2),a1		name string to a1
		adda.l	a1,a1			convert to APTR
		adda.l	a1,a1
		moveq.l	#SHARED_LOCK,d0		we want a shared lock
		move.l	a2,d1			packet in case it fails
		moveq	#0,d2			not creating, don't lock dir
		bsr	Locate			try to find this file
		movea.l	d0,a3			save the returned lock
		move.l	a3,d0			did we get it ?
		bne.s	OpenGotLock		yes, file exists

; if FIND_UPDATE fails then we should create the file with a shared lock instead
		move.l	dp_Type(a2),d0		get the command
		cmpi.w	#ACTION_FINDUPDATE,d0	simple open, shared read/write
		beq.s	GetWriteLock		yes, may need to create
OpenLockFailed	movea.l	a2,a0			no, object not there
		bra	WorkFail

; We got an ACTION_FINDOUTPUT (exclusive) or ACTION_FINDUPDATE (shared)
GetWriteLock	movea.l	dp_Arg2(a2),a0		directory lock to a0
		movea.l	dp_Arg3(a2),a1		name string to a1
		adda.l	a1,a1			convert to APTR
		adda.l	a1,a1			d0 holds boolean lock-type
		move.w	d0,d1			save the command
		moveq.l	#EXCLUSIVE_LOCK,d0	assume ACTION_FIND_OUTPUT
		cmpi.w	#ACTION_FIND_OUTPUT,d1	was it
		beq.s	10$			yes, lock type was correct
		moveq.l	#SHARED_LOCK,d0		no, need shared write lock
10$		move.l	a2,d1			send packet so it can fail OK
		moveq.l	#st.file,d2		what we want to create
		bsr	Create			create the file
		movea.l	d0,a3			save the returned lock
		move.l	a3,d0			did we get it ?
		beq.s	OpenLockFailed		something wrong

; The open worked and we have a lock BPTR in a3.  Check that we have a lock
; on a real file rather than a directory.  Fail the packet if we got a dir.
; I allocate the rwdata structure first because we initialise a lot of it
; from the file header, so we may as well do it now even if the header is bad.
OpenGotLock	moveq.l	#rwdata_SIZEOF,d0	get an rwdata structure
		bsr	GetPubMem
		movea.l	d0,a4
		move.l	a4,d0			did we get it
		bne.s	GotRWData		yes, fill it in
OpenNoMem	movea.l	a3,a0			no memory for the open
		bsr	FreeLock		so free up this lock
		move.w	#ERROR_NO_FREE_STORE,ErrorCode+2(a5)
		movea.l	a2,a0			and return this packet
		bra	WorkFail		with the error code

; now fetch the file header and fill in any info we need in the rwdata struct
GotRWData	movea.l	a3,a0
		adda.l	a0,a0
		adda.l	a0,a0
		move.l	fl_Volume(a0),d0
		cmp.l	CurrentVolume(a5),d0	is volume OK
		bne.s	BadVolume		no, error exit now
		move.l	fl_Key(a0),d0		get the header key
		move.l	d0,rwd_FileHeader(a4)	store in rwdata struct
		move.l	d0,rwd_Header(a4)	in case we use it after all
		move.l	d0,rwd_LastData(a4)	need to start somewhere
		move.l	d0,d4			save rwd_FileHeader in d4
		bsr	GrabBlock
****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a0
		move.l	a0,-(sp)
		bsr	Type			get type of this header
		movea.l	(sp)+,a0
		cmpi.l	#st.file,d0		is it a file
		beq.s	OpenGotFile		yes, finish initialising rwdata

; We tried to open a directory, free lock and return ERROR_OBJECT_WRONG_TYPE.
		move.l	d4,d0			associate with itself
		bsr	LooseBlock		free cache buffer in a0
BadVolume	movea.l	a3,a0			free this lock
		bsr	FreeLock
		movea.l	a4,a1			free rwdata structure
		moveq.l	#rwdata_SIZEOF,d0
		jsr	_LVOFreeMem(a6)
		move.w	#ERROR_OBJECT_WRONG_TYPE,ErrorCode+2(a5)
		movea.l	a2,a0			failing this packet
		bra	WorkFail		all done

; The file is opened OK, initialise the rest of rwdata for reads/writes to
; start from the beginning of the file.  All fields initialised to zero now.
OpenGotFile	move.l	d4,d0			belongs to this header
		bsr	LooseBlock		finished with file header
		move.w	MaxEntry(a5),rwd_Entry(a4) current offset into hashtable
		move.l	a3,rwd_Lock(a4)		owning lock on file

; fix for notification when creating new files but leaving them empty.  If we
; created this file (MODE_NEWFILE) then mark the file as altered so we update
; on Close() and check for notification.  (Only for exclusive locks).
		move.l	a3,a0
		adda.l	a0,a0
		adda.l	a0,a0
		moveq.l	#exclusive.lock,d0
		cmp.l	fl_Access(a0),d0
		bne.s	5$
		move.w	#TRUE,rwd_Altered(a4)	so notify goes off

; Finally, see if there is an existing rwheader for the given file header key.
; If there isn't (this is the only Open for this file) then create one and hang
; it off the global list of rwheaders (so subsequent opens for the same file
; can use it).  In this case we use current co-routine as the main read/write
; co-routine for this file.  If the rwheader already exists then we'll kill
; off the current co-routine and allow the one in the rwheader to take over
; our main read/write loop (it's already WaitCo'ed).
5$		move.l	rwd_Header(a4),d0	stash header key
		movea.l	CurrentVolume(a5),a0	stash volnode pointer
		lea.l	OpenFiles(a5),a1	point at open files list
findrwd		move.l	(a1),d1			do we have another entry ?
		beq.s	NoHeader		nope, must create one
		movea.l	d1,a1
		cmp.l	rwh_FileHeader(a1),d0	yes, does it match the header
		bne.s	findrwd			no, look for the next one
		cmp.l	rwh_Volume(a1),a0	header matches, but does volume?
		bne.s	findrwd			nope

; we've found an rwheader for the file we just opened (again) so link the
; new rwdata structure into that header and wind up the current co-routine
	printf <'Found one at %lx\n'>,a1
		move.l	rwh_StateList(a1),(a4)	link us into the list
		move.l	a4,rwh_StateList(a1)
		move.l	a1,rwd_Owner(a4)	back link to header
		movea.l	dp_Arg1(a2),a0		get filehandle BPTR
		adda.l	a0,a0			convert to APTR
		adda.l	a0,a0
		move.l	a4,fh_Args(a0)		dp_Arg1 on subsequent packets
		movea.l	a2,a0			returning this packet
		moveq.l	#TRUE,d0		non-error return
		clr.l	ErrorCode(a5)		dp_Res2 not important
		bra	WorkDone		return pkt and kill co-routine

; This is the only open call (so far) for this file so we have to create a
; new rwheader and hang it off the list of open files.  The current co-
; routine will stay active and be used for the main read/write loop of this
; file.  Subsequent opens will defer to this co-routine because they will
; find the rwheader on the OpenFiles list.
NoHeader	moveq.l	#rwheader_SIZEOF,d0
		bsr	GetPubMem
		tst.l	d0			did we get it?
		beq	OpenNoMem		nope, free lock and exit
	printf <'Created one at %lx\n'>,d0
		movea.l	d0,a3			don't need lock pointer anymore

; initialise the header before linking it into the list of opened files
		move.l	d4,rwh_FileHeader(a3)	we keep fileheader in d4 always
		move.l	CurrentVolume(a5),rwh_Volume(a3)
		move.l	CurrentCo(a5),rwh_CoBase(a3)
		move.l	OpenFiles(a5),(a3)	link into list of open files
		move.l	a3,OpenFiles(a5)
		move.l	rwh_StateList(a3),(a4)	link rwdata into the list
		move.l	a4,rwh_StateList(a3)
		move.l	a3,rwd_Owner(a4)	back link to rwheader
		movea.l	dp_Arg1(a2),a0		get filehandle BPTR
		adda.l	a0,a0			convert to APTR
		adda.l	a0,a0
		move.l	a4,fh_Args(a0)		dp_Arg1 on subsequent packets
		movea.l	a2,a0			returning this packet
		moveq.l	#TRUE,d0		non-error return
		moveq.l	#FALSE,d1		dp_Res2 not important
		bsr	ReturnPkt		done, leave co-routine running

;------------------------------------------------------------------------------
; This is the main loop for an open file.  We do a WaitCo to put this co-routine
; to sleep until it is woken up by a packet request for this file.  This routine
; is responsible for handling packets from multiple callers that are destined
; for the same file.  Whenever a packet has been processed it is replied and a
; check is made to see if there are any more packets on the PacketList of the
; current rwd_Header.  Processing continues until there are no more packets
; left, then the co-routine goes to sleep via WaitCo again. a3=rwheader
;------------------------------------------------------------------------------
WaitForWork	bsr	WaitCo			wait until we're kicked off
GetNextWork	move.l	rwh_PacketList(a3),d0	any work left to do ?
		move.l	d0,rwh_RunPacket(a3)	make it current if yes
		beq.s	WaitForWork		nope, wait for more packets
		movea.l	d0,a2			current packet to a2
		move.l	(a2),rwh_PacketList(a3)	and unlink from list
		movea.l	LN_NAME(a2),a2		get actual DOS packet pointer

; We have some work to do.  Dispatch to the right routine based on packet type
		move.l	dp_Type(a2),d0		get the action code
	printf <'got work -- %ld\n'>,d0
		cmpi.w	#ACTION_CURRENT_VOLUME,d0
		bne.s	CheckVolume		current volume is quick
	printf <'Current=%lx, rwh_=%lx\n'>,CurrentVolume(a5),rwh_Volume(a3)
		move.l	rwh_Volume(a3),d0	dp_Res1 = volume node
		move.l	UnitNumber(a5),d1	dp_Res2 = unit number
		movea.l	a2,a0
		bsr	ReturnPkt		send packet back
		bra.s	GetNextWork		and look for more work

; Before we can execute any other packets we must ensure the volume is the same
; The check for a NULL volume pointer allows us to service reads while the 
; restart routine is still running (because the volume has not been set up yet)
CheckVolume	move.l	rwh_Volume(a3),d1	get volume address
		beq.s	VolumeOK		if 0, then skip the test
		cmp.l	CurrentVolume(a5),d1	same as current volume ?
		beq.s	VolumeOK		yes
		moveq.l	#FALSE,d1		if ACTION_END return FALSE
		cmpi.w	#ACTION_END,d0
		beq.s	10$
5$		moveq.l	#TRUE,d1		else return TRUE
10$		move.l	d1,d0			this error return
		move.l	#ERROR_DEVICE_NOT_MOUNTED,d1
		movea.l	a2,a0			failing this packet
		bsr	ReturnPkt
		bra.s	GetNextWork		look for more work

;------------------------------------------------------------------------------
; Read data from a file.  Calls different routines for oldfs and newfs modes.
;------------------------------------------------------------------------------
VolumeOK	printf <'Volnode = %lx\n'>,d1
		cmpi.w	#ACTION_READ,d0		is this a read request ?
		bne.s	TryWrite		nope, maybe write
		movea.l	dp_Arg1(a2),a1		get correct rwdata header
		move.l	dp_Arg2(a2),a0		get buffer address
		move.l	dp_Arg3(a2),d0		get amount to read
		movem.l	d4/a2/a3,-(sp)		the only regs to save
		btst.b	#0,DiskType+3(a5)	old or new format ?
		bne.s	10$			new format
		bsr	ReadOFS			old format
		bra.s	ReadEnd
10$		bsr	ReadFFS			do the read
ReadEnd		movem.l	(sp)+,d4/a2/a3
ReadEndNoRest	movea.l	a2,a0			returns d0/d1 error codes
		tst.l	d1			any error ?
		ble.s	10$			nope
		moveq.l	#-1,d0			yes, return error
10$		bsr	ReturnPkt		send the packet back
		bra	GetNextWork		and wait for more

;------------------------------------------------------------------------------
; Write data to a file.  Calls different routines for oldfs and newfs modes
;------------------------------------------------------------------------------
TryWrite	cmpi.w	#ACTION_WRITE,d0	is this a write request ?
		bne.s	TrySeek			nope, maybe a seek
		movea.l	dp_Arg1(a2),a1		get correct rwdata header
		move.l	dp_Arg2(a2),a0		get buffer address
		move.l	dp_Arg3(a2),d0		get amount to be written
		movem.l	d4/a2/a3,-(sp)		save the packet address
		btst.b	#0,DiskType+3(a5)	old or new format ?
		bne.s	10$			new format
		bsr	WriteOFS		old format
		bra.s	ReadEnd
10$		bsr	WriteFFS		do the write
	printf <'Back from WriteFFS\n'>
		bra.s	ReadEnd			send the packet back

;------------------------------------------------------------------------------
; Seek to a given position in the file.  Different routines for oldfs and newfs
;------------------------------------------------------------------------------
TrySeek		cmpi.w	#ACTION_SEEK,d0		is this a seek request ?
		bne.s	TryEnd			nope, maybe a close
		move.l	dp_Arg1(a2),a1		get correct rwdata header
		move.l	dp_Arg2(a2),d0		fetch seek offset
		move.l	dp_Arg3(a2),d1		fetch seek mode
		movem.l	d4/a2/a3,-(sp)
		btst.b	#0,DiskType+3(a5)	old or new format ?
		bne.s	10$			new format
		bsr	SeekOFS			old format
		bra.s	ReadEnd
10$		bsr	SeekFFS			do the seek
		bra.s	ReadEnd			send the packet back

;------------------------------------------------------------------------------
; Caller is closing the file.  If this is the last close then kill co-routine
;------------------------------------------------------------------------------
TryEnd		cmpi.w	#ACTION_END,d0		is this a close request ?
		bne	TryDup			nope, maybe DupLockFH
	printf <'pkt=END\n'>
;		movea.l	dp_Arg1(a2),a0		get rwdata structure
;		bsr	FreeAllRLocks		free associated record locks
		movea.l	dp_Arg1(a2),a0		get rwdata structure
		tst.w	rwd_Altered(a0)		did we alter any data ?
		beq.s	NotWritten		nope, just close the file
		move.l	d4,d0			get header key number
		bsr	GrabBlockA4		and fetch it
****** need to check for an error condition if the header is garbage ******
;		movea.l	d0,a4
		lea.l	cb_SIZEOF(a4),a0	point to actual buffer data
		adda.l	BlockSize(a5),a0	reference from end of block
		move.l	vfhb_Parent(a0),-(sp)	save parent for notification
		bclr.b	#4,vfhb_Protect+3(a0)	clear archive bit
		lea.l	vfhb_Days(a0),a0	datestamp the header
		bsr	DatStamp

		; now update the fileheader, before updating the list
		; This is so UpdateList can grabblock the fileheader
		movea.l	a4,a0			write this block out
		move.l	d4,d0
		moveq.l	#TRUE,d1		checksum it first
		move.l	d0,cb_Header(a0)	it belongs to itself
		bsr	WriteBlockLater		write the block out

; Update the cached directory entries in the Dirlist.  If we crash after the
; listblock but before the root, that's ok since the listblock will be rebuilt.
; do it _before_ flushing data to the queue!
		btst.b	#2,DiskType+3(a5)	are we using fast dirs?
		beq.s	10$
		move.l	d4,d1			this is the object to update
		move.l	(sp),d0			parent was stored on stack!
		bsr	UpdateList
10$
		; now we want to flush stuff, but make sure the fhb is last
		move.l	d4,d0
		bsr	GrabBlockA4		and fetch it
;		move.l	d0,a4
		
		; this forces out everything except bitmaps to disk
		; we want to make sure that all data and the list block are
		; on the queue before the fhb is (and before the Root).
		bsr	HClearWaiting		clear headers/data not bitmaps

		; now put the fileheader back on the pending queue (last)
		movea.l	a4,a0			write this block out
		move.l	d4,d0
		moveq.l	#TRUE,d1		checksum it first
		move.l	d0,cb_Header(a0)	it belongs to itself
		bsr	WriteBlockNow		write the block out

; now check to see if anyone is waiting for notification on the file we just
; altered.  We check first for the file header itself (because it changed)
; and then check the owning directory (in case the file was just created)
; Bug fix.  Make sure file is closed (lock freed) before notifying
		movea.l	dp_Arg1(a2),a4		get rwdata structure
		movea.l	rwd_Lock(a4),a0		free up the file lock
		bsr	FreeLock

		move.l	d4,d0			check for this header
		moveq.l	#FALSE,d1		don't rebuild the lists
		bsr	CheckNotify
		move.l	(sp)+,d0		get parent directory
		moveq.l	#TRUE,d1		rebuild list if orphaned
		bsr	CheckNotify
		bra.s	SkipLockFree

NotWritten	movea.l	dp_Arg1(a2),a4		get rwdata structure
		movea.l	rwd_Lock(a4),a0		free up the file lock
		bsr	FreeLock

SkipLockFree	lea.l	rwh_StateList(a3),a0	search for this rwdata struct
10$		cmpa.l	(a0),a4
		beq.s	20$			found it
		movea.l	(a0),a0			link to next
		bra.s	10$
20$		move.l	(a4),(a0)		unlink from the list
		movea.l	a4,a1			rwdata to free
		moveq.l	#rwdata_SIZEOF,d0	free rwdata struct
		jsr	_LVOFreeMem(a6)
		tst.l	rwh_StateList(a3)	any outstanding opens
		beq.s	KillHeader		nope, kill header too
	printf <'open: there are outstanding opens now\n'>
		moveq.l	#TRUE,d0		return an OK code
		moveq.l	#FALSE,d1		no error
		bra	ReadEndNoRest		return packet, wait for more

; there are no outstanding opens on this file so we can free the main header
; too and then kill this co-routine because no more packets can come in.  We
; also discard any extraneous cached data that belong to this file with the
; exception of the file header itself.  Chances are we won't need it anymore.
KillHeader	move.l	d4,d0			this fileheader
		bsr	HClearValid		clear buffers for it
		lea.l	OpenFiles(a5),a0	search for rwheader
40$		cmpa.l	(a0),a3
		beq.s	50$			found it
		movea.l	(a0),a0
		bra.s	40$
50$		move.l	(a3),(a0)		unlink from list
		movea.l	a3,a1			free the memory
		moveq.l	#rwheader_SIZEOF,d0
		jsr	_LVOFreeMem(a6)
		movea.l	a2,a0			return this packet
		bra	WorkDone		wind up and kill co-routine

;------------------------------------------------------------------------------
; duplicate the lock held by this open file.  Only works for UPDATE and OLDFILE
;------------------------------------------------------------------------------
TryDup		cmpi.w	#ACTION_COPY_DIR_FH,d0	DupLockFH request ?
		bne.s	TryParent		nope, perhaps ParentFH
		move.l	d4,d0			and get header key of file
GetFHLock	moveq.l	#SHARED_LOCK,d1
		bsr	GetLock			try to get the lock
		move.l	ErrorCode(a5),d1	0 if no problems
		movea.l	a2,a0			return this packet
		bsr	ReturnPkt
		bra	GetNextWork		and wait for more

;------------------------------------------------------------------------------
; Return a lock on the owning directory of this opened file.  Caller must free.
;------------------------------------------------------------------------------
TryParent	cmpi.w	#ACTION_PARENT_FH,d0
		bne.s	TryExamine		maybe examine FH
		move.l	d4,d0			get file header key
		bsr	ParentKey		find parent (always valid)
		bra.s	GetFHLock		and get a shared lock on it

;------------------------------------------------------------------------------
; Return current info about the opened file.  Since everything is now synced
; on the file header we really don't need a separate routine (because file
; size is no longer cached locally).  We just call the DoInfo part of Examine.
;------------------------------------------------------------------------------
TryExamine	cmpi.w	#ACTION_EXAMINE_FH,d0
		bne.s	TrySetSize		maybe set file size
		movem.l	d2-d7/a2-a4,-(sp)	DoInfo doesn't save regs
		move.l	d4,d2			stash header key for DoInfo
		move.l	d2,d0			and fetch that key
		bsr	GrabBlockA3
;		movea.l	d0,a3			buffer to a3 for DoInfo
		bsr	DoInfo
		moveq.l	#0,d1			no error from this
		movea.l	a2,a0
		bsr	ReturnPkt
		movem.l	(sp)+,d2-d7/a2-a4
		bra	GetNextWork

;------------------------------------------------------------------------------
; Set file to the given size (uses same arguments as Seek) returns the final
; size of the file.  Checks that no other opens on this file are going to get
; clobbered by having size set to less than their current position or the place
; they are planning to read up to.  Uses Write[F|O]FS with a buffer argument
; of 0 to extend files.  Uses a special case of clearfile to truncate them.
;------------------------------------------------------------------------------
TrySetSize	cmpi.w	#ACTION_SET_FILE_SIZE,d0
		bne.s	TryRecordLock
		move.l	dp_Arg1(a2),a1		get correct rwdata header
		move.l	dp_Arg2(a2),d0		fetch requested size offset
		move.l	dp_Arg3(a2),d1		fetch seek mode
		movem.l	d4/a2/a3,-(sp)
		btst.b	#0,DiskType+3(a5)	old or new format ?
		bne.s	10$			new format
		bsr	SetSizeOFS		old format
		bra	ReadEnd
10$		bsr	SetSizeFFS
		bra	ReadEnd			send the packet back

;------------------------------------------------------------------------------
; Lock a record in the current file.  Record is associated with filehandle.
;------------------------------------------------------------------------------
TryRecordLock	movea.l	a2,a0			lock calls need packet in a0
		cmpi.w	#ACTION_LOCK_RECORD,d0
		bne.s	TryFreeRecord
		bsr	LockRecord
		bra	GetNextWork

TryFreeRecord	cmpi.w	#ACTION_FREE_RECORD,d0
		bne.s	TryLockTimer
		bsr	FreeRecord
		bra	GetNextWork

TryLockTimer	bsr	RelockRecord		must be timer stuff
		bra	GetNextWork

;==============================================================================
; NextJob(packet)
;	    a0
;
; This routine should NOT run as a co-routine.  It checks which rwheader that
; a packet belongs to and queues that packet on the rwh_PacketList.  If the
; rwh_RunPacket field is non-zero then that co-routine is actively running and
; can't be called.  It will fetch the packet off the list when it's finished
; with the current one.  If rwh_RunPacket is zero then we should CallCo() using
; rwh_CoBase to kick the routine off.
;
; rwdata structure is sent in dp_Arg1 by DOS (having fetched it from fh_Args
; when the file was opened.  This contains a backpointer to the rwheader.
;
; We can't just queue packets on the list because the first entry is a pointer
; to the message struct that's associated with them.  We have to use this
; pointer and queue using the message struct instead.  The main looop of Open
; knows about this and uses LN_NAME of the message struct to get the packet.
;==============================================================================
NextJob		move.l	a2,-(sp)
		move.l	dp_Arg1(a0),a1		fetch correct rwdata structure
		move.l	rwd_Owner(a1),a1	fetch pointer to rwheader
		lea.l	rwh_PacketList(a1),a2	enqueue the current packet
10$		move.l	(a2),d0
		beq.s	20$			found slot
		movea.l	d0,a2
		bra.s	10$
20$		movea.l	(a0),a0			get owning exec message node
		clr.l	(a0)			no next packet in list
		move.l	a0,(a2)			store in header
		movea.l	(sp)+,a2		restore regs for faster branch
		movea.l	rwh_CoBase(a1),a0	co-routine we may call
		tst.l	rwh_RunPacket(a1)	is the co-routine active ?
		beq	CallCo			no, kick it off now
		rts				yes, we're finished

;==============================================================================
; ClearWaiting()
;
; Transfers all of the cache buffers on the Waiting queue to the Pending queue.
;==============================================================================
ClearWaiting	lea.l	WaitingQueue(a5),a0
		REMHEAD				fetch first buffer
		beq.s	WaitingCleared		none left
		movea.l	d0,a1			change the queue type
		move.w	#HASHPENDING,cb_QueueType(a1)
		lea.l	PendingQueue(a5),a0
		ADDTAIL				add to end of queue
		bra.s	ClearWaiting
WaitingCleared	rts

;==============================================================================
; HClearWaiting()
;
; Only clears buffers from waiting if they are not bitmap blocks.
;==============================================================================
HClearWaiting	movem.l	d2-d3,-(sp)
		moveq.l	#BITMAP,d2
		move.l	WaitingQueue(a5),d3	fetch the first entry
HCWL		movea.l	d3,a1			node pointer to a1
		move.l	(a1),d3			look ahead to next node
		beq.s	HCWE			end of list, quit now
		cmp.l	cb_Size(a1),d2		
		beq.s	HCWL			bitmap block, don't clear

; This buffer is not a bitmap block.  Move it to the pending queue
		move.l	a1,d1			save pointer to cache buffer
		REMOVE				remove this node
		movea.l	d1,a1			restore pointer to it
		move.w	#HASHPENDING,cb_QueueType(a1)  mark the queue it's on
		lea.l	PendingQueue(a5),a0	adding to this queue
		ADDTAIL				append it
		bra.s	HCWL			and keep looking
HCWE		movem.l	(sp)+,d2-d3
		rts


;==============================================================================
; HClearValid( headerkey )
;		  d0
;
; Clears all buffers that belong to a given fileheader from the valid queue and
; puts them into the free queue with the exception of the header block itself.
;==============================================================================
HClearValid	movem.l	d2-d3,-(sp)
		move.l	d0,d2			save header key
		move.l	ValidBufferQueue(a5),d3	fetch the first entry
HCVL		movea.l	d3,a1			node pointer to a1
		move.l	(a1),d3			look ahead to next node
		beq.s	HCVE			end of list, quit now
		cmp.l	cb_Header(a1),d2	does it belong to this header
		bne.s	HCVL			nope, look for next
		cmp.l	cb_Key(a1),d2		is it the header itself
		beq.s	HCVL			yes, so leave it valid

; This buffer is associated with the given file header.  Add to the free list.
		move.l	a1,a0
		bsr	FreeBuffer
		bra.s	HCVL			and keep looking

HCVE		movem.l	(sp)+,d2-d3
		rts

		END
