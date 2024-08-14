 		SECTION	filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/memory.i"
		INCLUDE	"exec/tasks.i"
		INCLUDE	"exec/interrupts.i"
		INCLUDE	"exec/io.i"
		INCLUDE "exec/resident.i"
		INCLUDE	"devices/trackdisk.i"
		INCLUDE	"devices/inputevent.i"
		INCLUDE	"devices/input.i"
		INCLUDE	"libraries/dosextens.i"
		INCLUDE	"actions.i"
		INCLUDE	"globals.i"
		INCLUDE	"private.i"
		INCLUDE	"moredos.i"
		INCLUDE	"readwrite.i"
		INCLUDE	"printf.mac"

		INCLUDE "libhdr.i"
		INCLUDE "fs_rev.i"
		LIST

; 		DEBUGENABLE


		XREF	_AbsExecBase,_LVOAllocMem,_LVOFreeMem,_LVOFindTask
		XREF	_LVOSendIO,_LVODoIO,_LVOAllocSignal,_LVOSignal

		XREF	GetPubMem,Init,TaskWait,Qpkt,CallCo,StartCo,ResumeCo
		XREF	WorkDone,ReturnPkt,SendTimer,WriteMap,CreateCo
		XREF	TestDisk,WorkFail,CheckName,DiskProtected,Fetch
		XREF	ReadBlock,FreeBlock,GetLock,CheckLock,FreeBuffer
		XREF	GrabBlock,LooseBlock,Restart,ForgetVolume
		XREF	EnQueue,DeQueue,AddHash,UnHash,ClearWaiting
		XREF	GetBlock,DiscardBlock,Checksum,ClearBlock,DatStamp
		XDEF	InTheBeginning

;====== obsolete comment for C interface ======================================
; Gets called here when our device is DeviceProc'ed.  Initialise global memory
; and then wait for the startup packet to arrive before calling init.  If init
; fails then we couldn't get some memory or devices and a failure code will be
; sent back in dp_Res1 of the packet.
;==============================================================================
		moveq.l	#0,d1			must wait for packet
		bra.s	InTheBeginning

;==============================================================================
; Gets called here when our device is CreateProc'ed.  startup packet is passed
; in d1.  return must be via bcpl conventions
;==============================================================================

* put in a rom tag, so we can find it later.
* bra needed for putting ffs in l:fastfilesystem or on RDSKs

*
* put fake BCPL-style seglist at front of executable.  dosglue contains required
* stuff for the end of the segment.

Seglist:	DC.L	0			; no next segment
FS:		DC.L	(FSEND-FS)/4	; BCPL & MCC sillyness

InTheBeginning	movem.l	a0-a6,-(a7)
		lsl.l	#2,d1
		movea.l	d1,a2			; save parameter packet
		move.l	d1,-(sp)		; save for later test
		bsr.s	Start
		move.l	(sp)+,d1		; get back parameter packet
		movem.l	(a7)+,a0-a6
		tst.l	d1
		bne.s	10$			; BCPL type return
		rts
10$		jmp	(a6)

romtag:
		DC.W	RTC_MATCHWORD		;(50) UWORD RT_MATCHWORD
		DC.L	romtag			;(52) APTR  RT_MATCHTAG
		DC.L	FSEND			;(56) APTR  RT_ENDSKIP
* FIX!!!!
		DC.B	0; RTW_COLDSTART	;(5A) UBYTE RT_FLAGS
		DC.B	VERSION			;(5B) UBYTE RT_VERSION
		DC.B	NT_UNKNOWN		;(5C) UBYTE RT_TYPE
		DC.B	-81			;(5D) BYTE  RT_PRI
		DC.L	FSNAME			;(5E) APTR  RT_NAME
		DC.L	idtag			;(62) APTR  RT_IDSTRING
		DC.L	Seglist			;(66) APTR  RT_INIT
						;(6A) LABEL RT_SIZE
*
* In real BCPL, this would go at the end of the segment.  No need for us
* to do that, though. - REJ
* This code moved here from dosglue.asm so we can reference FSEND-FS above.
*
		CNOP 0,4
		dc.l	0		; End of global list
		dc.l	G_START/4,(InTheBeginning-FS)
		dc.l	150		; max global used (standard value)
FSEND	EQU	*
		DC.B	'$VER: '	; so version command works for ROM fs
idtag 		VSTRING
		DS.W	0			; version identification
FSNAME		DC.B	'filesystem',0		; name for FindResident()
		CNOP	0,4

Start		movea.l	_AbsExecBase,a6
		move.l	#global_SIZEOF,d0	get mem for global variables
		bsr	GetPubMem
		tst.l	d0			did we get our memory ?
		beq.s	ExitNoMem		no, quit now
		movea.l	d0,a5			a5 ALWAYS points to globals

		suba.l	a1,a1			find our task
		jsr	_LVOFindTask(a6)
		move.l	d0,ThisTask(a5)		stash it for TaskWait and Qpkt
		movea.l	d0,a0

 		lea.l	pr_MsgPort(a0),a0
		move.l	a0,ThisDevProc(a5)	some code needs Process ptr
		move.l	a2,d0			do we have our packet
		bne.s	10$			yes, no need to wait
		bsr	TaskWait		wait for the startup packet
		movea.l	d0,a2			and save for us for later
10$		movea.l	d0,a0			packet to a0 for init
GotPkt		bsr	Init			Init(pkt)
		move.l	CurrentVolume(a5),d1	return our VolNode
		move.l	d0,-(sp)		save return code (disk type)
		movea.l	a2,a0
		bsr	ReturnPkt		ReturnPkt(pkt,type,node)
		move.l	(sp)+,d0		did everything initialise ?
		beq.s	Exit			no, exit now

; add the interrupt server for disk changed events
		clr.w	DiskChanged(a5)		disk didn't change yet
		clr.w	ChangeInhibit(a5)	notice if it changes

		moveq.l	#-1,d0
		jsr	_LVOAllocSignal(a6)	get signal for disk changes
		moveq.l	#0,d1
		bset.l	d0,d1
		move.l	d1,DiskSig(a5)

		movea.l	ChangeInt(a5),a0
		move.b	#NT_INTERRUPT,LN_TYPE(a0)
		lea.l	ChangeIntCode(pc),a1
		move.l	a1,IS_CODE(a0)
		move.l	a5,IS_DATA(a0)		code can access globals
		moveq.l	#TD_REMOVE,d0
		moveq.l	#0,d1
		bsr	TestDisk		added the change interrupt
		bra.s	DoActions		and enter the main loop

Exit		movea.l	a5,a1			free globals memory
		move.l	#global_SIZEOF,d0
		jsr	_LVOFreeMem(a6)
ExitNoMem	rts				and return


;==============================================================================
; This is the interrupt routine for when the disk changes.  For 2.0 this code
; has been changed to signal the main task when a diskchange occurs.  This
; removes the need for a 1 second repeating timer when there is no disk
; activity.  Since we'll be waiting for packets we'll pick up the diskchange
; as soon as it occurs.
;==============================================================================
ChangeIntCode	move.l	a6,-(sp)
		addq.w	#1,DiskChanged(a1)	count number of changes
		movea.l	_AbsExecBase,a6		need exec base
		move.l	DiskSig(a1),d0		using this signal
		movea.l	ThisTask(a1),a1		to this task
		jsr	_LVOSignal(a6)
		move.l	(sp)+,a6
		rts				and that's all

;==============================================================================
; This is the main loop that waits for packets and then dispatches them to the
; relevant routines.  All routines are expected to return their own packets to
; the caller because they can return to the main loop before the packet is
; finished with. ie. Disk may still be processing it etc.  Packets passed in a0
; If there is work in the pending queue, then the Disk co-routine is kicked off
; to handle those requests (unless it is already handling them).
;==============================================================================
DoActions	tst.w	DiskRunning(a5)		is disk going ?
		bne.s	DiskGoing		yes, so no need to kick it
		lea.l	PendingQueue(a5),a0	any work to do ?
		TSTLIST	a0
		beq.s	DiskGoing		no, so don't start disk
		movea.l	DiskCo(a5),a0		start up disk co-routine
		bsr	CallCo
DiskGoing	bsr	TaskWait		get a packet
		movea.l	d0,a0			into a0 for other routines
		move.l	a0,d0
		beq.s	NotDiskReq		no packet, must be diskchange

; see if this is a returning IORequest for the disk co-routine. Call if it is.
		move.l	dp_Port(a0),d0
		cmp.l	DiskDev(a5),d0
		bne.s	NotDiskReq		nope, carry on
		movea.l	DiskCo(a5),a0		ReCall the disk co-routine
		bsr	CallCo
		bra.s	DoActions		and wait for another packet

; This was not a returning disk request, Check if the disk has been changed
; TaskWait actually returns the flag in d1 but it's just as easy to test the
; global that's updated by the diskchange code.
NotDiskReq	tst.w	DiskChanged(a5)		did the disk change
		beq	NotChanged		nope
		move.l	a0,-(sp)		yep, save the current packet

; if there is anything on the pending or waiting queues then ignore the change
; and let disk put up the "You MUST replace" requester when it tries to write.
		clr.w	DiskChanged(a5)		re-arm the change flag
		tst.w	BitmapValid(a5)		if bitmap invalid
		beq.s	CanChange		don`t check pending or waiting

		lea.l	WaitingQueue(a5),a0
		TSTLIST	a0
		bne	ChangeDone
		lea.l	PendingQueue(a5),a0
		TSTLIST	a0
		bne	ChangeDone

	printf <'The disk changed\n'>
CanChange	tst.w	ChangeInhibit(a5)	are we allowed to notice this
		bne	ChangeDone		no, we're inhibited

		moveq.l	#TD_CHANGESTATE,d0
		bsr	TestDisk		see if disk in or out
		tst.l	d0
		bne.s	DiskOut			disk has been pulled

; a new disk has been inserted, simply call restart on this drive
		bsr	Restart
		moveq.l	#IECLASS_DISKINSERTED,d0
		bra.s	DoInputEvent		logged in the new disk

; the last disk has been taken out.  If there is stuff pending on either the
; Waiting or Pending queues, then complain loudly (You MUST replace volume)
DiskOut		move.l	#-1,DiskType(a5)	ID_NO_DISK_PRESENT
		bsr	TurnOff			flush everything to disk
		bsr	DiscardBuffers		put buffers on the free queue
		moveq.l	#IECLASS_DISKREMOVED,d0

; we've done some kind of change, so now we need to generate an input event
DoInputEvent	tst.w	InputSent(a5)		input event sent already?
		bne	ChangeDone		yes, gonna miss an event
		addq.w	#1,InputSent(a5)	nope, mark as sent now

		movea.l	InputDev(a5),a0		initialise diskchange event
		movea.l	InpEvent(a5),a1
		move.l	a1,IO_DATA(a0)
		move.l	#ie_SIZEOF,IO_LENGTH(a0)
		move.w	#IND_WRITEEVENT,IO_COMMAND(a0)
		move.l	ThisDevProc(a5),MN_REPLYPORT(a0)  reply to main port
		clr.b	IO_FLAGS(a0)

		clr.l	ie_NextEvent(a1)	no next event
		move.b	d0,ie_Class(a1)
		clr.b	ie_SubClass(a1)
		clr.l	ie_Code(a1)		clears qualifier too
		clr.l	ie_X(a1)		clears x and y

		movem.l	a0/a1,-(sp)		** preserve regs here **
		movea.l	TimerExtraDev(a5),a1	send a getsystime request
		move.w	#TR_GETSYSTIME,IO_COMMAND(a1)
		move.b	#IOF_QUICK,IO_FLAGS(a1)
		jsr	_LVODoIO(a6)
		movea.l	TimerExtraDev(a5),a1	find what the time was
		movem.l	IOTV_TIME+TV_SECS(a1),d0/d1
		movem.l	(sp)+,a0/a1		** restore them here **
		movem.l	d0/d1,ie_TimeStamp(a1)	timestamp inputevent
	printf <'Timestamp was %ld,%ld\n'>,d0,d1

		move.l	InputPacket(a5),a1
		move.l	a1,LN_NAME(a0)
		move.l	a0,dp_Link(a1)		link packet to iorequest
		move.l	#ACTION_DISK_CHANGE,dp_Type(a1)

		movea.l	InputDev(a5),a1		get back the IORequest
;		movea.l	_AbsExecBase,a6
		jsr	_LVOSendIO(a6)		and write the event
		clr.l	SoftErrors(a5)		no errors on new volume
ChangeDone	printf <'ChangeDone\n'>
		movea.l	(sp)+,a0		restore the packet

; Main dispatch point.  Calls routines based on the action code in the packet.
NotChanged	cmpa.w	#0,a0			is there a packet
		beq	DoActions		nope, so look for another
		move.l	dp_Action(a0),d1	stash the action code
		lea.l	Actions(pc),a1		point to action table
10$		move.l	(a1)+,d2		any action code here ?
		beq.s	20$			no, therefore action not known
		cmp.w	d2,d1			was it this action
		bne.s	10$			no, look for next
		swap	d2			offset to lower word
		jsr	Actions(pc,d2.w)	call the routine
		bra	DoActions		and look for more work

; This is not a packet we can handle.  Reject it with error action not known
20$		move.l	#ERROR_ACTION_NOT_KNOWN,dp_Res2(a0)
		clr.l	dp_Res1(a0)		error here
		bsr	Qpkt			send this packet back
		bra	DoActions		and wait for another

;==============================================================================
; Dispatch table for the main loop.  Contains offset to routine,action_number.
;==============================================================================
ACTION		MACRO
		DC.W	rtn_\1-Actions,ACTION_\1
		ENDM

Actions		ACTION	TIMER			motor off ticks etc.
		ACTION	READ			Read(file,data,length)
		ACTION	WRITE			Write(file,data,length)
		ACTION	SEEK			Seek(file,offset,mode)
		ACTION	EXAMINE_NEXT		ExNext(lock,fileinfoblock)
		ACTION	EXAMINE_OBJECT 		Examine(lock,fileinfoblock)
		ACTION	EXAMINE_ALL		ExAll(lock,buff,size,codes,ctrl)
		ACTION	EXAMINE_FH		ExamineFH(file,fileinfoblock)
		ACTION	LOCATE_OBJECT		Lock(name)
		ACTION	COPY_DIR 		DupLock(lock)
		ACTION	COPY_DIR_FH		DupLockFromFH(file)
		ACTION	FREE_LOCK 		FreeLock(lock)
		ACTION	FIND_OUTPUT		Open(name,write)
		ACTION	FIND_INPUT		Open(name,read)
		ACTION	FIND_UPDATE		Open(name,update)
		ACTION	OPEN_LOCK		Open(Lock)
		ACTION	END 			Close(file)
		ACTION	PARENT 			Parent(dirlock)
		ACTION	PARENT_FH		ParentFH(file)
		ACTION	LOCK_RECORD		LockRecord(fh,pos,len,mode,t/o)
		ACTION	FREE_RECORD		FreeRecord(fh,pos,len)
		ACTION	SET_FILE_SIZE		SetFileSize(fh,size)
		ACTION	CREATE_DIR 		CreateDir(name)
		ACTION	MAKE_LINK		makelink(linkname,linkobj)
		ACTION	RENAME_OBJECT 		Rename(lock,name)
		ACTION	DELETE_OBJECT 		Delete(name)
		ACTION	INFO 			checks the lock and...
		ACTION	DISK_INFO 		Info()
		ACTION	CURRENT_VOLUME
		ACTION	FLUSH 			Flush all disk buffers
		ACTION	MORE_CACHE 		get more cache buffers
		ACTION	INHIBIT
		ACTION	SET_DATE
		ACTION	SET_COMMENT 
		ACTION	SET_PROTECT 
		ACTION	RENAME_DISK 
		ACTION	WRITE_PROTECT		set WrProt(a5)
		ACTION	DISK_CHANGE
		ACTION	IS_FILESYSTEM
		ACTION	READ_LINK		readlink(lock,name,buffer,size)
		ACTION	ADD_NOTIFY		AddNotify(req,filename)
		ACTION	REMOVE_NOTIFY		RemNotify(req)
		ACTION	LOCK_TIMER		locktimer(co-routine)
		ACTION	NOTIFY_RETURN		returning NotifyMessages
		ACTION	FORMAT			initialise inhibited disk
		ACTION	CHANGE_MODE
		ACTION	SAME_LOCK
		DC.L	0			list terminator


;==============================================================================
; Returns a boolean indicating whether two locks point to the same file.  If
; not then FALSE and an error is returned.  Locks are in dp_Arg1 and dp_Arg2
;==============================================================================
rtn_SAME_LOCK	movem.l	d2/a2,-(sp)
	printf <'ACTION_SAME_LOCK\n'>
		movea.l	a0,a2			stash packet
		movea.l	dp_Arg1(a2),a0
		bsr	CheckLock		get key for first lock
		move.l	d0,d2
		beq.s	BadLock			wrong volume in drive
		movea.l	dp_Arg2(a2),a0
		bsr	CheckLock		get key for second lock
		tst.l	d0
		beq.s	BadLock			wrong volume
		clr.l	ErrorCode(a5)
		cmp.l	d0,d2
		bne.s	BadLock			not same locks
		moveq.l	#TRUE,d0		same locks
		bra.s	DoneLock
BadLock		moveq.l	#FALSE,d0
DoneLock	move.l	ErrorCode(a5),d1
		movea.l	a2,a0
		movem.l	(sp)+,d2/a2
		bra	ReturnPkt

;==============================================================================
; handles action_change_mode to change the mode of a lock in fs supported way.
; dp_Arg1=0 means change a lock, dp_arg1=1 means change lock from filehandle
; dp_Arg2=pointer to lock or filehandle
; dp_Arg3-new mode as appropriate
;==============================================================================
rtn_CHANGE_MODE	movem.l	a2/a3,-(sp)
		movea.l	a0,a2			packet to a2
		movea.l	dp_Arg2(a2),a3		get lock or fh to a3
		tst.l	dp_Arg1(a2)		given lock or fh
		beq.s	GotLockPtr		was a lock
		adda.l	a3,a3			convert fh BPTR to APTR
		adda.l	a3,a3
		movea.l	fh_Args(a3),a3		filehandle, get rwd_ struct
		movea.l	rwd_Lock(a3),a3		and get associated lock

; make sure that the given lock is a valid one before continuing
GotLockPtr	movea.l	a3,a0
		bsr	CheckLock		make sure lock is OK
		move.l	d0,d1			stash key in d1
		beq.s	ChangeModeError		bad lock, don`t change mode

; now change the lock mode to the required type (shared or exclusive)
		move.l	dp_Arg3(a2),d0		get required type
		cmpi.l	#exclusive.lock,d0	all locks can be made shared
		bne.s	CanChangeOK

; before changing a lock to exclusive mode, check there are no other locks
		move.l	LockQueue(a5),d0
		movea.l	a3,a1
		adda.l	a1,a1
		adda.l	a1,a1			convert lock BPTR to APTR
10$		lsl.l	#2,d0
		movea.l	d0,a0
		cmpa.l	a0,a1			is this our lock
		beq.s	20$			yes, look for the next one
		cmp.l	fl_Key(a0),d1		no, is key the same
		beq.s	30$			yes, return an error
20$		move.l	(a0),d0
		bne.s	10$
		bra.s	CanChangeOK

30$		move.w	#ERROR_OBJECT_IN_USE,ErrorCode+2(a5)

; something prevented us from changing the mode, report this fact
ChangeModeError	move.l	ErrorCode(a5),d1
		moveq.l	#FALSE,d0
		bra.s	ChangeModeExit

; really change the mode of the lock and return packet
CanChangeOK	adda.l	a3,a3
		adda.l	a3,a3
		move.l	dp_Arg3(a2),fl_Access(a3)	change lock mode
		moveq.l	#TRUE,d0		return success
		moveq.l	#FALSE,d1		so no secondary error
ChangeModeExit	movea.l	a2,a0
		movem.l	(sp)+,a2/a3
		bra	ReturnPkt

;==============================================================================
; This is a null routine since we don't need to do anything when the diskchange
; event comes back from the input device chain
;==============================================================================
rtn_DISK_CHANGE:
	printf <'pkt=DISKCHANGE\n'>
		clr.w	InputSent(a5)
		rts

;==============================================================================
; IsFileSystem - simply returns a BOOLEAN TRUE since we are a filing system
;==============================================================================
rtn_IS_FILESYSTEM:
		moveq.l	#TRUE,d0		we are a filing system
		bra	ReturnPkt

;==============================================================================
; Changed for 2.0.  Disk fires off timer requests when it completes a read or
; a write operation.  WriteBlock also fires of timer requests for a deferred
; write operation.  If the request returns here and there is nothing on the
; pending queue then we transfer Waiting to Pending and fire off another
; timer request.  If there is nothing on either the Pending or Waiting queues
; then we call Turnoff to flush the disk and don't send another timer request.
;==============================================================================
rtn_TIMER	printf <'Timer event\n'>
		clr.w	TimerRunning(a5)	timer request has returned
		lea.l	PendingQueue(a5),a0	any work in progress
		TSTLIST	a0
		beq.s	TurnOff			nope, turn everything off
		bra	SendTimer		yes, send another timer request

;==============================================================================
; Flush() - if no closedown is pending then causes a closedown now.
;==============================================================================
rtn_FLUSH	printf <'pkt=FLUSH\n'>
		tst.l	ClosePkt(a5)		closedown active ?
		bne	Qpkt			queue packet in a0
		tst.w	MotorTicks(a5)
		beq	Qpkt			turnoff is happening anyway
		move.l	a0,ClosePkt(a5)		drop through to TurnOff

;==============================================================================
; TurnOff() - starts the closedown co-routine to flush everything to disk.
;==============================================================================
TurnOff		lea.l	CloseDown(pc),a0
		suba.l	a1,a1			no packet to reply on failure
		bra	StartCo

;==============================================================================
; CloseDown()
;
; Flushes all buffers to disk and writes the bitmap and root block back.  If
; the bitmap isn't allocated on the disk, WriteMap will attempt to allocate it.
;==============================================================================
CloseDown	tst.w	FilesOpen(a5)		any files open
		beq.s	FlushDisk		no
; dequeue stuff from the waiting queue and put on the pending queue
		bsr	ClearWaiting		leave for Flush() calls
		bsr	WriteMap		update roots idea of bitmap

FlushDisk	moveq.l	#CMD_UPDATE,d0		flush disk buffers
	printf <'CloseDown: flushing/stopping disk\n'>
		bsr	TestDisk
		moveq.l	#TD_MOTOR,d0		** won't work for hard disk **
		moveq.l	#0,d1			turn motor off
		bsr	TestDisk
		clr.w	MotorTicks(a5)		mark motor as off
		move.l	ClosePkt(a5),d0
		beq.s	NoCPacket		no pending close packet
		movea.l	d0,a0			we were called through Flush
		bsr	Qpkt
	printf <'CloseDown: sent close packet back\n'>
		clr.l	ClosePkt(a5)
NoCPacket	movea.l	KillCo(a5),a0
		move.l	CurrentCo(a5),d0
		bra	ResumeCo		commit suicide

;==============================================================================
; Inhibit(packet)
;	    a0
;
; marks disk as busy if inhibit was true else acts as if disk was inserted.
; Changed this to allow nested inhibits and only uninhibit on the last one.
;==============================================================================
rtn_INHIBIT	move.l	a2,-(sp)
	printf <'pkt=INHIBIT (%ld)\n'>,dp_Arg1(a0)
		movea.l	a0,a2			save packet
		tst.w	dp_Arg1+2(a2)		inhibit or uninhibit ?
		beq.s	UnInhibit		off (make like disk inserted)

		moveq.l	#FALSE,d0		assume failure
		tst.w	FilesOpen(a5)		anyone writing ?
		bne.s	FailInhibit		yes, so can't inhibit
		moveq.l	#1,d0
		add.w	d0,ChangeInhibit(a5)	bump inhibit count
		cmp.w	ChangeInhibit(a5),d0	was this the first one ?
		bne.s	DoneInhibit		nope, already inhibited
	printf <'Doing inhibit\n'>
		bsr	TurnOff			flush everything
		bsr	DiscardBuffers		chuck away valid buffers
		move.l	#type.busy,DiskType(a5)	disk is busy now
		bra.s	DoneInhibit		and that's all

UnInhibit	cmpi.l	#type.busy,DiskType(a5)	if inhibited now
		bne.s	DoneInhibit
		subq.w	#1,ChangeInhibit(a5)	decrement inhibit count
		bne.s	DoneInhibit		not 0, can't uninhibit
	printf <'Doing uninhibit\n'>
		bsr	Restart

DoneInhibit	movea.l	a2,a0			validator could be going now
		moveq.l	#TRUE,d0
		moveq.l	#FALSE,d1		no error
		bra.s	QuitInhibit

FailInhibit	move.l	#ERROR_OBJECT_IN_USE,d1	secondary error code
	printf <'Inhibit returning (%ld,%ld)\n'>,d0,d1
QuitInhibit	bsr	ReturnPkt		send the packet back
		move.l	(sp)+,a2
	printf <'Inhibit done, changeinhibit = %d'>,ChangeInhibit(a5)
	printf <'	DiskType = %lx'>,DiskType(a5)
		rts

;==============================================================================
; RenameObject()  renames a file or directory
;==============================================================================
rtn_RENAME_OBJECT:
 printf <'pkt=RENAME_OBJECT %lx,%b,%lx,%b\n'>,dp_Arg1(a0),dp_Arg2(a0),dp_Arg3(a0),dp_Arg4(a0)
		XREF	Rename
		movea.l	a0,a1			stash packet for startco
		lea.l	Rename(pc),a0		create Rename co-routine
		bra	StartCo

;==============================================================================
; Handles an examine.object packet. Starts co-routine to examine an object.
;==============================================================================
rtn_EXAMINE_OBJECT:
	printf <'pkt=EXAMINE_OBJECT %lx\n'>,dp_Arg1(a0)
		XREF	ExObject
		movea.l	a0,a1			stash packet for startco
		lea.l	ExObject(pc),a0		create ExObject co-routine
		bra	StartCo

;==============================================================================
; Handles an examine.next packet.  Starts co-routine to examine next object.
;==============================================================================
rtn_EXAMINE_NEXT:
	printf <'pkt=EXAMINE_NEXT %lx\n'>,dp_Arg1(a0)
		XREF	ExNext
		movea.l	a0,a1			stash packet for startco
		lea.l	ExNext(pc),a0		create ExNext co-routine
		bra	StartCo

;==============================================================================
; Handles an examine.all packet.  Starts co-routine to examine all objects.
;==============================================================================
rtn_EXAMINE_ALL:
	printf <'pkt=EXAMINE_ALL\n'>
		XREF	ExAll
		movea.l	a0,a1			stash packet for startco
		lea.l	ExAll(pc),a0		create ExAll co-routine
		bra	StartCo

;==============================================================================
; Returns the path string held in a soft link header
;==============================================================================
rtn_READ_LINK:
	printf <'pkt=READ_LINK\n'>
		XREF	ReadLink
		movea.l	a0,a1			stash packet for startco
		lea.l	ReadLink(pc),a0
		bra	StartCo

;==============================================================================
; Handles a locate.object packet.  Starts the locate co-routine to get a lock.
;==============================================================================
rtn_LOCATE_OBJECT:
 printf <'pkt=LOCATE_OBJECT %lx,%b,%ld\n'>,dp_Arg1(a0),dp_Arg2(a0),dp_Arg3(a0)
		XREF	UserLocate
		movea.l	a0,a1			stash packet for startco
		lea.l	UserLocate(pc),a0	create Locate co-routine
		bra	StartCo			skip return code

;==============================================================================
; Start up the Open co-routine when opening a file for read,write or update.
;=============================================================================
rtn_FIND_UPDATE:
		IFD DEBUG_CODE
		printf <'pkt=FIND_UPDATE\n'>
		bra.s	FindThing
		ENDC
rtn_FIND_INPUT:
		IFD DEBUG_CODE
		printf <'pkt=FIND_INPUT\n'>
		bra.s	FindThing
		ENDC
rtn_FIND_OUTPUT:
		IFD DEBUG_CODE
		printf <'pkt=FIND_OUTPUT\n'>
		ENDC
rtn_OPEN_LOCK:

		XREF	Open
FindThing	movea.l	a0,a1			stash packet for startco
		lea.l	Open(pc),a0		start Open co-routine
		bra	StartCo			and call it (can hang around)

;==============================================================================
; Call the Open co-routine if we are given a file lock else just return vol.
;==============================================================================
rtn_CURRENT_VOLUME:
	printf <'pkt=CURRENT_VOLUME\n'>
		tst.l	dp_Arg1(a0)
		bne.s	CallAccess		call access for the volume
		move.l	CurrentVolume(a5),d0	else just use current
		move.l	UnitNumber(a5),d1
		bra	ReturnPkt

;==============================================================================
; Call the Open co-routine using cobase put into the file handle.
;==============================================================================
rtn_SEEK:
	IFD DEBUG_CODE
	printf <'pkt=SEEK\n'>
	bra	CallAccess
	ENDC
rtn_END:
	IFD DEBUG_CODE
	printf <'pkt=END\n'>
	bra	CallAccess
	ENDC
rtn_READ:
	IFD DEBUG_CODE
	printf <'pkt=READ\n'>
	bra.s	CallAccess
	ENDC
rtn_WRITE:
	IFD DEBUG_CODE
	printf <'pkt=WRITE\n'>
	bra.s	CallAccess
	ENDC
rtn_COPY_DIR_FH:
	IFD DEBUG_CODE
	printf <'pkt=COPY_DIR_FH\n'>
	bra.s	CallAccess
	ENDC
rtn_PARENT_FH:
	IFD DEBUG_CODE
	printf <'pkt=PARENT_FH\n'>
	bra.s	CallAccess
	ENDC
rtn_EXAMINE_FH:
	IFD DEBUG_CODE
	printf <'pkt=EXAMINE_FH\n'>
	bra.s	CallAccess
	ENDC
rtn_SET_FILE_SIZE:
	IFD DEBUG_CODE
	printf <'pkt=SET_FILE_SIZE\n'>
	bra.s	CallAccess
	ENDC
rtn_LOCK_TIMER:
	IFD DEBUG_CODE
	printf <'pkt=LOCK_TIMER\n'>
	bra.s	CallAccess
	ENDC
rtn_LOCK_RECORD:
	IFD DEBUG_CODE
	printf <'pkt=LOCK_RECORD\n'>
	bra.s	CallAccess
	ENDC
rtn_FREE_RECORD:
	IFD DEBUG_CODE
	printf <'pkt=FREE_RECORD\n'>
	ENDC
		XREF	NextJob
CallAccess	bra	NextJob			in readwrite.asm


;==============================================================================
; Free a lock obtained by calling LocateObject.  Checks lock exists first.
;==============================================================================
rtn_FREE_LOCK:
	printf <'pkt=FREE_LOCK\n'>
		XREF	FreeLock
		move.l	a0,-(sp)		save the packet
		movea.l	dp_Arg1(a0),a0		get lock pointer
		bsr	FreeLock		return code to d0
		movea.l	(sp)+,a0		get back the packet
		move.l	ErrorCode(a5),d1	and the ErrorCode
		bra	ReturnPkt		send it back

;==============================================================================
; Returns a new lock on the parent of the given lock or fails if given bad lock
;==============================================================================
rtn_PARENT:
	printf <'pkt=PARENT\n'>
		XREF	Parent
		movea.l	a0,a1			stash packet for startco
		lea.l	Parent(pc),a0		create Parent co-routine
		bra	StartCo			skip return code

;==============================================================================
; Create a directory in the current directory. (just start create co-routine).
; Also called to create a hard link to a file or directory.
;==============================================================================
rtn_CREATE_DIR:
rtn_MAKE_LINK:
	printf <'pkt=CREATE_DIR\n'>
		XREF	UserCreate
		movea.l	a0,a1			stash packet for startco
		lea.l	UserCreate(pc),a0	Create co-routine
		bra	StartCo			skip return code

;==============================================================================
; delete an object given a lock on the owning directory and the object name.
;==============================================================================
rtn_DELETE_OBJECT:
	printf <'pkt=DELETE_OBJECT\n'>
		XREF	Delete
		movea.l	a0,a1			stash packet for StartCo
		lea.l	Delete(pc),a0
		bra	StartCo

;==============================================================================
; WriteProtect - a new packet type to support write protection of partitions.
;
; dp_Arg1 = Write protect state TRUE or FALSE
; dp_Arg2 = APTR to a password string
;
;if WrProt(a5) == 0
; if Arg1 = FALSE and Arg2 = FALSE and old password = FALSE then unprotect
; if Arg1 = FALSE and Arg2 = FALSE and old password =  NAME then ERROR
; if Arg1 = FALSE and Arg2 = NAME  and old password = FALSE then unprotect
; if Arg1 = FALSE and Arg2 = NAME  and old password != NAME then ERROR
;
; if Arg1 = TRUE and old password = TRUE then ERROR_DISK_WRITE_PROTECTED
; if Arg1 = TRUE old password = Arg2 and protect disk
;==============================================================================
rtn_WRITE_PROTECT:
	printf <'pkt=WRITE_PROTECT\n'>
		move.l	a2,-(sp)		stash packet address
		movea.l	a0,a2
		moveq.l	#TD_PROTSTATUS,d0	find real state of disk
		bsr	TestDisk
		tst.w	d0
		beq.s	CanDo			OK, we can do something to it

AlreadyDone	move.l	#ERROR_DISK_WRITE_PROTECTED,d1
		moveq.l	#FALSE,d0
		bra.s	WPDone			return an error

CanDo		move.l	dp_Arg1(a2),d0		else, we can change it
		bne.s	DoProtect		user wants to protect the disk
; user wants to un-protect.  Check if they have the correct password in Arg2
		move.l	Password(a5),d1		any password ?
		beq.s	30$			nope, so do the unprotect
		cmp.l	dp_Arg2(a2),d1		does it match the last passwd?
		beq.s	25$			yep, everything OK
		move.l	#ERROR_INVALID_COMPONENT_NAME,d1
		moveq.l	#FALSE,d0		password wrong
		bra.s	WPDone
25$		clr.l	Password(a5)		no password now
30$		bra.s	LastStep		and store protect stuff

DoProtect	tst.w	UserWrProt(a5)		only if not protected already
		bne.s	AlreadyDone		return an error
		bsr	TurnOff			flush everything
		move.l	dp_Arg2(a2),Password(a5)	save new passwd
		moveq.l	#TRUE,d0
LastStep	move.w	d0,UserWrProt(a5)	and user protect status
		move.w	d0,WrProt(a5)		and real protect status
		moveq.l	#TRUE,d0		return OK status

WPDone		movea.l	a2,a0
		bsr	ReturnPkt		return current state
		movea.l	(sp)+,a2
		rts
;==============================================================================
; MoreCache() - dp_Arg1 contains the number of extra cache buffers required.
;		Returns a TRUE result if everything worked OK, else FALSE.
; SMB 3/25/88:  Modified this to return the number of buffers in the queues
;		can also take a negative argument to remove some buffers
;==============================================================================
rtn_MORE_CACHE:
	printf <'pkt=MORE_CACHE\n'>
		XREF	GetBuffers
		move.l	d2,-(sp)
		move.l	a0,-(sp)		save packet
		move.l	dp_Arg1(a0),d2
		beq.s	AddFinal		just get buffer count
		bgt.s	AddThem			user wants to get buffers

; user wants to free up some buffers.  This may cause some memory fragmentation
; because the buffer lists are probably shuffled pretty good by now, but them's
; the breaks.  Will not free ALL buffers, we must have at least one buffer left
		neg.l	d2			make positive number to remove
		bsr	TurnOff			flush everything to disk

		bsr	JunkBuffers		move valid->free
		bsr.s	CountBuffers		how many buffers do we have
		cmpi.l	#1,d0
		ble.s	AddFinal		can't remove any buffers
		cmp.l	d0,d2
		blt.s	RemEntry		OK, we got enough
		move.l	d0,d2			truncate to numbuffs-1
		subq.l	#1,d2
		bra.s	RemEntry

RemLoop		movea.l	FreeBufferQueue(a5),a1	get buffer to be freed
		move.l	a1,d0			make sure there is one
		beq.s	RemEntry
		move.l	(a1),FreeBufferQueue(a5)  unlink from list
		moveq.l	#cb_SIZEOF,d0
		add.l	BlockSize(a5),d0	free data area too
		jsr	_LVOFreeMem(a6)
RemEntry	dbra	d2,RemLoop		and go for the next
		bra.s	AddFinal		finished

; user wants to add buffers, add them and return the current total count
AddThem		move.l	d2,d0
		bsr	GetBuffers		get the buffers (could fail)

AddFinal	bsr.s	CountBuffers		return number of buffers
		move.l	d0,NumBuffers(a5)	set the global
		move.l	(sp)+,a0		CountBuffers sets up d0
		bsr	ReturnPkt		thats it
		move.l	(sp)+,d2
		rts

;==============================================================================
; count = ListCount(list)
;  d0		     a0
;
; Counts number of buffers when they are held in an exec style list.
;==============================================================================
ListCount	moveq.l	#0,d0			assume no buffers
		move.l	(a0),d1			fetch first entry
10$		movea.l	d1,a0			node pointer to a0
		move.l	(a0),d1			lookahead to next node
		beq.s	ListCounted		none, count finished
;	printf <'block %ld, type %ld, queue/state $%lx'>,cb_Key(a0),cb_SIZEOF(a0),cb_QueueType(a0)
		addq.l	#1,d0
		bra.s	10$
ListCounted	rts				returns d0=number of buffers

;==============================================================================
; number = CountBuffers()
;   d0
;
; Returns the number of cache buffers in use by the filing system.
;==============================================================================
CountBuffers	move.l	d2,-(sp)
		moveq.l	#0,d2			current count
		lea.l	WaitingQueue(a5),a0	waiting buffers
		bsr.s	ListCount
	printf <'%ld waiting buffers'>,d0
		add.l	d0,d2
		lea.l	ValidBufferQueue(a5),a0
		bsr.s	ListCount
	printf <'%ld valid buffers'>,d0
		add.l	d0,d2
		lea.l	PendingQueue(a5),a0	and pending buffers
		bsr.s	ListCount
	printf <'%ld pending buffers'>,d0
		add.l	d2,d0			d0 hold count now
		movea.l	FreeBufferQueue(a5),a0	count free buffers
NumBuffs	cmpa.w	#0,a0			just drops through for last
		beq.s	10$
		addq.l	#1,d0
		movea.l	(a0),a0
		bra.s	NumBuffs
10$		move.l	(sp)+,d2
	printf <'%ld total buffers'>,d0
		rts

;==============================================================================
; Checks the lock in arg1 and then fills in DiskInfo struct in Arg2
;==============================================================================
rtn_INFO:
	printf <'pkt=INFO\n'>
		move.l	a0,-(sp)		save the packet
		move.l	dp_Arg1(a0),a0		check given lock
		bsr	CheckLock
		move.l	(sp)+,a0		get back the packet
		move.l	ErrorCode(a5),d1	assume an error
		tst.l	d0
		beq	ReturnPkt		it was, return false
		movea.l	dp_Arg2(a0),a1		fetch diskinfo struct
		bra.s	INFO_ENTRY		and do disk info

;==============================================================================
; Returns info about the current disk in the drive.
;==============================================================================
rtn_DISK_INFO:
;	printf <'pkt=DISK_INFO\n'>
		movea.l	dp_Arg1(a0),a1		get disk info struct
INFO_ENTRY	adda.l	a1,a1			convert to APTR
		adda.l	a1,a1
		move.l	UnitNumber(a5),id_UnitNumber(a1)
		move.l	SoftErrors(a5),id_NumSoftErrors(a1)
		move.l	HighestBlock(a5),d0	calculate number of blocks
		sub.l	Reserved(a5),d0
		addq.l	#1,d0
		move.l	d0,id_NumBlocks(a1)
		sub.l	BlocksFree(a5),d0
		subq.l	#1,d0
		move.l	d0,id_NumBlocksUsed(a1)
		move.l	BlockSize(a5),id_BytesPerBlock(a1)

; if an OFS format disk then blocksize should be bytes of data held in a block
		btst.b	#0,DiskType+3(a5)
		bne.s	5$			DOS/1 or DOS/3 is FFS format
		move.l	BytesPerData(a5),id_BytesPerBlock(a1)

5$		move.l	DiskType(a5),id_DiskType(a1)
		move.l	CurrentVolume(a5),id_VolumeNode(a1)
		move.l	LockQueue(a5),id_InUse(a1)
		moveq.l	#ID_WRITE_PROTECTED,d0
		tst.w	WrProt(a5)
		bne.s	10$			it is write protected
		moveq.l	#ID_VALIDATED,d0	assume validated
		tst.w	BitmapValid(a5)
		bne.s	10$			it is validated
		moveq.l	#ID_VALIDATING,d0
10$		move.l	d0,id_DiskState(a1)
		moveq.l	#TRUE,d0
		moveq.l	#0,d1			res 2 not important
		bra	ReturnPkt		send back disk info packet

;==============================================================================
; called ACTION_COPY_DIR but actually duplicates a lock.
;==============================================================================
rtn_COPY_DIR	movem.l	d2-d3/a2-a4,-(sp)
	printf <'pkt=COPY_DIR\n'>
		movea.l	a0,a2			stash packet address
		move.l	RootKey(a5),d2		default key
		movea.l	dp_Arg1(a2),a3		get the lock to be duplicated
		cmpa.w	#0,a3			could be a root lock
		beq.s	DoDup			it is, so just get the lock
		adda.l	a3,a3
		adda.l	a3,a3
		move.l	fl_Key(a3),d2		get key for this lock
		beq.s	DupDone			it's already a root lock
		move.l	fl_Volume(a3),d3	get volume for this lock
		cmp.l	CurrentVolume(a5),d3	is it same volume 
		beq.s	DoDup			yes, so just get the lock

; special case of duplock when the volume of the lock isn't the same as current
		move.l	LockQueue(a5),a4	stash current lock queue
		movea.l	d3,a0			put volume locklist into queue
		adda.l	a0,a0
		adda.l	a0,a0
		move.l	dl_LockList(a0),LockQueue(a5)
		move.l	a0,-(sp)		save the volume pointer
		move.l	d2,d0			get a lock on this key
		moveq.l	#shared.lock,d1
		bsr	GetLock
		move.l	d0,d2			save lock ptr as key
		beq.s	10$

; getlock doesn't correctly update the volume node
		lsl.l	#2,d0
		movea.l	d0,a0
		move.l	d3,fl_Volume(a0)

10$		move.l	(sp)+,a0		get back original volume ptr
		move.l	LockQueue(a5),dl_LockList(a0)
		move.l	a4,LockQueue(a5)
		bra.s	DupDone

;the simple case when the volume we want the duplicated lock on is in the drive
DoDup		move.l	d2,d0			get a lock on this key
		moveq.l	#shared.lock,d1
		bsr	GetLock
		move.l	d0,d2

DupDone		movea.l	a2,a0			this packet
		move.l	d2,d0			this result (lock ptr)
		move.l	ErrorCode(a5),d1	secondary result
		bsr	ReturnPkt
		movem.l	(sp)+,d2-d3/a2-a4
		rts

;==============================================================================
; set date, comment or protection bits on a file.  Root not allowed.
;==============================================================================
rtn_SET_DATE:
		IFD DEBUG_CODE
 printf <'pkt=SET_DATE %lx,%b,%lx\n'>,dp_Arg2(a0),dp_Arg3(a0),dp_Arg4(a0)
		bra.s	SetThing
		ENDC
rtn_SET_PROTECT:
		IFD DEBUG_CODE
 printf <'pkt=SET_PROTECT %lx,%b,%ld\n'>,dp_Arg2(a0),dp_Arg3(a0),dp_Arg4(a0)
		bra.s	SetThing
		ENDC
rtn_SET_COMMENT:
		IFD DEBUG_CODE
 printf <'pkt=SET_COMMENT %lx,%b,%b\n'>,dp_Arg2(a0),dp_Arg3(a0),dp_Arg4(a0)
		ENDC

		XREF	Comment
SetThing	movea.l	a0,a1			stash packet for startco
		lea.l	Comment(pc),a0
		bra	StartCo

;==============================================================================
; AddNotify(packet)
;	      a0
;==============================================================================
rtn_ADD_NOTIFY:
	printf <'pkt=ADD_NOTIFY\n'>
		XREF	AddNotify
		movea.l	a0,a1			stash packet for startco
		lea.l	AddNotify(pc),a0	needs to read disk so...
		bra	StartCo			make it a co-routine

;==============================================================================
; RemNotify(packet)
;	      a0
;==============================================================================
rtn_REMOVE_NOTIFY:
	printf <'pkt=REMOVE_NOTIFY\n'>
		XREF	RemNotify
		move.l	a0,-(sp)		save the packet address
		movea.l	dp_Arg1(a0),a0		get NotifyRequest
		bsr	RemNotify		remove it
		movea.l	(sp)+,a0
		clr.l	dp_Res1(a0)
		clr.l	dp_Res2(a0)
		bra	Qpkt			no errors here

;==============================================================================
; This routine just handles returning notify requests
;==============================================================================
rtn_NOTIFY_RETURN:
		XREF	ReturnNotify
		movea.l	dp_Link(a0),a0		point to NotifyMessage
		bra	ReturnNotify		and go clean up

;==============================================================================
; RenameDisk()  changes the diskname in memory and in the root block.
;==============================================================================
rtn_RENAME_DISK:
		movea.l	a0,a1			stash packet for startco
		lea.l	RenameDisk(pc),a0
		bra	StartCo

;==============================================================================
; the actual rename disk co-routine.
;==============================================================================
RenameDisk	movea.l	d0,a2			save the packet address
		bsr	DiskProtected		make sure we can write
		tst.l	d0
		beq.s	10$			OK, disk not protected
		movea.l	a2,a0
		bra	WorkFail

10$		movea.l	dp_Arg1(a2),a3		stash the new name
		adda.l	a3,a3			convert to an APTR
		adda.l	a3,a3
		movea.l	a3,a0
		bsr	CheckName		make sure name is OK
		tst.l	d0
		bne.s	20$
		movea.l	a2,a0
		bra	WorkFail

20$		moveq.l	#0,d0			get memory for the name
		move.b	(a3),d0
		addq.l	#2,d0			allow for null termination
		bsr	GetPubMem
		tst.l	d0
		bne.s	30$
		move.w	#ERROR_NO_FREE_STORE,ErrorCode+2(a5)
		movea.l	a2,a0			couldn't get memory
		bra	WorkFail

30$		movea.l	d0,a4			a4 points to new name space
		moveq.l	#0,d0
		move.b	(a3),d0			copy name across
		movea.l	a4,a0
40$		move.b	(a3)+,(a0)+
		dbra	d0,40$

		movea.l	CurrentVolume(a5),a3	free name memory in volnode
		adda.l	a3,a3
		adda.l	a3,a3
		movea.l	dl_Name(a3),a1
		adda.l	a1,a1
		adda.l	a1,a1
		moveq.l	#0,d0
		move.b	(a1),d0
		addq.l	#2,d0
		jsr	_LVOFreeMem(a6)

		move.l	a4,d0
		lsr.l	#2,d0			make new name a BPTR
		move.l	d0,dl_Name(a3)		save in VolNode

		move.l	RootKey(a5),d0		now change on disk
		bsr	GrabBlock
		movea.l	d0,a3			save the buffer
		lea.l	cb_SIZEOF(a3),a0
		adda.l	BlockSize(a5),a0
		lea.l	vrb_Name(a0),a0
		moveq.l	#0,d0
		move.b	(a4),d0
50$		move.b	(a4)+,(a0)+		copy the name across
		dbra	d0,50$

		move.l	CurrentCo(a5),cb_CoPkt(a3)
		move.w	#CMD_WRITE,cb_State(a3)
		move.l	RootKey(a5),cb_Key(a3)
		move.l	#TRUE,cb_Size(a3)
		move.w	#HASHPENDING,cb_QueueType(a3)
		lea.l	PendingQueue(a5),a0
		movea.l	a3,a1
		ADDTAIL
		movea.l	a3,a0
		bsr	AddHash			add into hash table too
		bsr	Fetch			wait for this to write out
		movea.l	d0,a0
		move.l	RootKey(a5),d0		associate with itself
		bsr	LooseBlock
		movea.l	a2,a0
		moveq.l	#TRUE,d0
		bra	WorkDone

;==============================================================================
; Format()
;
; dp_Arg1 = VolumeName, dp_Arg2=dostype.  Initialises the disk to empty only
; if the volume is inhibited.  Fails with ERROR_OBJECT_IN_USE if it isn't.
;==============================================================================
rtn_FORMAT	movem.l	a2-a4,-(sp)
		movea.l	a0,a2			stash packet address

; the volume must be inhibited or we won't be able to initialise it
		move.l	#ERROR_OBJECT_IN_USE,d1	assume this is the error
		tst.w	ChangeInhibit(a5)	volume inhibited ?
		beq	FormatFail		yep, so error do error return

; and there must be a disk in the drive else we can't format it either
		moveq.l	#TD_CHANGESTATE,d0	is there a disk in the drive ?
		bsr	TestDisk
		tst.l	d0
		beq.s	DiskIsThere		yes, we may be able to format
		move.l	#ERROR_NO_DISK,d1
		bra	FormatFail		no disk, no format

; finally, the disk has to be write enabled to write boot and root
DiskIsThere	moveq.l	#TD_PROTSTATUS,d0	make sure we can write to it
		bsr	TestDisk
		tst.l	d0
		beq.s	CanWriteDisk		yep, not write protected
		move.l	#ERROR_DISK_WRITE_PROTECTED,d1
		bra	FormatFail

; we can do the format, check that the name is valid before we start anything
CanWriteDisk	movea.l	dp_Arg1(a2),a0		get name BPTR
		adda.l	a0,a0
		adda.l	a0,a0			convert to APTR
		bsr	CheckName		see if name is valid
		tst.l	d0
		bne.s	NameValid		it is
		move.l	ErrorCode(a5),d1	error_invalid_component_name
		bra	FormatFail

; We can write to this disk.  Grab a cache buffer and write out the bootblock
NameValid	bsr	GetBlock		use a cache buffer
		movea.l	d0,a3			stash the address
		lea.l	cb_SIZEOF(a3),a0	point to data area
		bsr	ClearBlock		and clear it out
		move.l	dp_Arg2(a2),cb_SIZEOF(a3)	this is the DosType
		movea.l	DiskExtraDev(a5),a1	use spare IORequest
		move.l	LowerCyl(a5),d0		writing the first block
		move.w	BlockShift(a5),d1	convert to byte offset
		lsl.l	d1,d0
		move.l	d0,IO_OFFSET(a1)
		move.l	BlockSize(a5),IO_LENGTH(a1)
		move.w	#CMD_WRITE,IO_COMMAND(a1)
		lea.l	cb_SIZEOF(a3),a0
		move.l	a0,IO_DATA(a1)
		jsr	_LVODoIO(a6)		write data out

; We'll assume everything worked. It will be pretty apparent if it didn't.
; Now construct a root block (with no bitmap) and write that out.  Validator
; will construct the bitmap when restart is called after uninhibit.
		lea.l	cb_SIZEOF(a3),a4
		move.l	#T_SHORT,rb_Type(a4)
		move.l	HTSize(a5),rb_HTSize(a4)
		adda.l	BlockSize(a5),a4	point to end of block
		move.l	#ST_ROOT,vrb_SecType(a4)	set secondary type
		lea.l	vrb_CreateDays(a4),a0	put in creation date
		bsr	DatStamp
;		lea.l	vrb_DiskMod(a4),a0	disk modified date
;		bsr	DatStamp
;		lea.l	vrb_Days(a4),a0		root altered date
;		bsr	DatStamp

; Only for dos\4 and dos\5.  NOTE: gets DiskType from dp_Arg2 here!
		btst.b	#2,dp_Arg2(a2)		matches dos\4 and dos\5
		beq.s	5$
		move.l	RootKey(a5),d0		we always put dirlist after
		addq.l	#1,d0			the rootblock
		move.l	d0,vrb_DirList(a4)

5$		lea.l	vrb_Name(a4),a0		point at name area
		movea.l	dp_Arg1(a2),a1		get given name
		adda.l	a1,a1			convert to APTR
		adda.l	a1,a1
		clr.w	d0
		move.b	(a1),d0
10$		move.b	(a1)+,(a0)+
		dbra	d0,10$
		lea.l	cb_SIZEOF(a3),a0
		bsr	Checksum		checksum the root
		sub.l	d0,cb_SIZEOF+rb_Checksum(a3)

		movea.l	DiskExtraDev(a5),a1	use spare IORequest
		move.l	RootKey(a5),d0		writing the root block
		add.l	LowerCyl(a5),d0
		move.w	BlockShift(a5),d1	convert to byte offset
		lsl.l	d1,d0
		move.l	d0,IO_OFFSET(a1)
		move.l	BlockSize(a5),IO_LENGTH(a1)
		move.w	#CMD_WRITE,IO_COMMAND(a1)
		lea.l	cb_SIZEOF(a3),a0
		move.l	a0,IO_DATA(a1)
		jsr	_LVODoIO(a6)		write data out

; Now write out a Dirlist block for the root, since things depend on it
; Only for dos\4 and dos\5.  NOTE: gets DiskType from dp_Arg2 here!
		btst.b	#2,dp_Arg2(a2)		matches dos\4 and dos\5
		beq.s	30$
		lea.l	cb_SIZEOF(a3),a4
		move.l	a4,a0
		bsr	ClearBlock		numentries = 0, next = 0
		move.l	#t.dirlist,fhl_Type(a4)
		move.l	RootKey(a5),d0
		move.l	d0,fhl_Parent(a4)
		addq.l	#1,d0			dirlist follows root
		move.l	d0,fhl_OwnKey(a4)

		move.l	a4,a0
		bsr	Checksum		checksum the root
		sub.l	d0,fhl_Checksum(a4)

		movea.l	DiskExtraDev(a5),a1	use spare IORequest
		move.l	RootKey(a5),d0
		addq.l	#1,d0			writing the block after root
		add.l	LowerCyl(a5),d0
		move.w	BlockShift(a5),d1	convert to byte offset
		lsl.l	d1,d0
		move.l	d0,IO_OFFSET(a1)
		move.l	BlockSize(a5),IO_LENGTH(a1)
		move.w	#CMD_WRITE,IO_COMMAND(a1)
		move.l	a4,IO_DATA(a1)
		jsr	_LVODoIO(a6)		write data out

30$		movea.l	a3,a0			don't want this block now
		bsr	DiscardBlock
		moveq.l	#TRUE,d0		format worked
		moveq.l	#FALSE,d1		so no error
		bra.s	FormatDone
FormatFail	moveq.l	#FALSE,d0
FormatDone	movea.l	a2,a0			return this packet
		bsr	ReturnPkt
		movem.l	(sp)+,a2-a4
		rts
		
;==============================================================================
; JunkBuffers() - discards any valid buffers
;==============================================================================
JunkBuffers	lea.l	ValidBufferQueue(a5),a0
		move.l	MLH_HEAD(a0),a0
		tst.l	MLN_SUCC(a0)		is this list empty?
		beq.s	10$			yes, we're done
		bsr	FreeBuffer
		bra.s	JunkBuffers
10$		rts

;==============================================================================
; DiscardBuffers() - discards any valid buffers and forgets locks on this vol.
;==============================================================================
DiscardBuffers	bsr.s	JunkBuffers
		movea.l	BMBlock(a5),a0		invalidate bitmap block
		clr.l	cb_Key(a0)
;		clr.w	cb_State(a0)		shouldn`t be dirty
		moveq.l	#CMD_CLEAR,d0
		bsr	TestDisk		drop through to forget locks

;==============================================================================
; ForgetLocks() - Throws away any outstanding locks which are held in the
;		  volume struct for later re-use.  If there are no locks
;		  outstanding then the device entry is removed.
;==============================================================================
ForgetLocks	move.l	a2,-(sp)
		tst.l	CurrentVolume(a5)
		beq.s	ForgetMeNot		no work to do

		tst.l	LockQueue(a5)		any pending locks ?
		bne.s	StashLocks		yep, stash 'em
		movea.l	CurrentVolume(a5),a0

; have to check for any notifies and not forget the volume if there are some
		movea.l	a0,a1
		adda.l	a1,a1
		adda.l	a1,a1
		tst.l	dl_unused(a1)		any notifies pending ?
		bne.s	LocksForgot		yep, can`t forget volume then

		bsr	ForgetVolume
		bra.s	LocksForgot

StashLocks	printf <'Stashing locks\n'>
		movea.l	CurrentVolume(a5),a2
		adda.l	a2,a2
		adda.l	a2,a2
		lea.l	dl_LockList(a2),a0	add locks to list in vol node
10$		tst.l	(a0)
		beq.s	20$
		movea.l	(a0),a0
		adda.l	a0,a0
		adda.l	a0,a0
		bra.s	10$

20$		move.l	LockQueue(a5),(a0)
		clr.l	dl_Task(a2)

LocksForgot	clr.l	LockQueue(a5)
		clr.l	CurrentVolume(a5)
ForgetMeNot	move.l	(sp)+,a2
		rts

		END
