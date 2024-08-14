		SECTION	filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/io.i"
		INCLUDE	"libraries/dosextens.i"
		INCLUDE	"globals.i"
		INCLUDE	"private.i"
		INCLUDE	"moredos.i"
		INCLUDE	"actions.i"
		INCLUDE	"readwrite.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XREF	GetPubMem,_LVOFreeMem,_LVOCopyMem,_LVOSendIO,_LVODoIO
		XREF	_LVOAbortIO,_LVOAddTime,_LVOSubTime,_LVOCmpTime
		XREF	ReturnPkt

		XDEF	LockRecord,RelockRecord,FreeRecord,FreeAllRLocks


;==============================================================================
; LockRecord( packet )
;		a0
;
; Attempts to lock a record associated with the given filehandle (actually an
; rwdata structure in dp_Arg1) and performs timeout waits if required.  This
; call sets up a timer request and associates it with a LOCK_TIMER packet so
; that ReLockRecord can treat it as if it just returned from a timeout wait.
;
; dp_Arg1 = pointer to rwd_Data structure (and therefore the filehandle)
; dp_Arg2 = start position for the lock
; dp_Arg3 = length of the required record lock
; dp_Arg4 = the mode of this lock
; dp_Arg5 = timeout value if we should wait for the lock
;
; Returns TRUE if the record was locked, else returns FALSE
;==============================================================================
LockRecord	movem.l	a2-a3,-(sp)
		movea.l	a0,a2			save packet address
		moveq.l	#rl_SIZEOF,d0		get record lock memory
		bsr	GetPubMem
		movea.l	d0,a3			stash the data area
		move.l	a3,d0
		bne.s	10$

; we couldn't get any memory for the record lock structure.  Return an error
		move.l	#ERROR_NO_FREE_STORE,d1	this is the error
		moveq.l	#FALSE,d0		failed to get lock
		movea.l	a2,a0
		bsr	ReturnPkt		return failed packet
		movem.l	(sp)+,a2-a3		and exit
		rts

; Initialise the timer request and the fake packet (pointed to by LN_NAME of
; the timer request).  Also initialise the record lock with a datestamp of
; the time when we must fail the request.  This is used to ensure that the
; timeout is done for the correct duration.  We associate the initial packet
; with this record lock structure, it will be removed and returned by ReLock.
10$		move.l	a2,rl_StartPacket(a3)	save given dos packet
		movea.l	a3,a1			initialise TimerRequest
		movea.l	TimerDev(a5),a0		just copy our ticker one
		moveq.l	#IOTV_SIZE,d0
		jsr	_LVOCopyMem(a6)
		lea.l	rl_FakePacket(a3),a0	initialise fake DOS packet
		move.w	#ACTION_LOCK_TIMER,dp_Type+2(a0)
		move.l	a0,LN_NAME(a3)		link into TimerRequest
		move.l	a3,dp_Link(a0)		link TimerRequest to packet
		move.l	dp_Arg1(a2),dp_Arg1(a0)	associate with this rw_header
		move.w	#TR_GETSYSTIME,IO_COMMAND(a3)
		move.b	#IOF_QUICK,IO_FLAGS(a3)
		movea.l	a3,a1			get the current time
		jsr	_LVODoIO(a6)		we'll use IOTV_SECS later

	printf <'rwd_data= %lx\n'>,dp_Arg1(a2)
	printf <'Start   = %ld\n'>,dp_Arg2(a2)
	printf <'Length  = %ld\n'>,dp_Arg3(a2)
	printf <'Mode    = %ld\n'>,dp_Arg4(a2)
	printf <'Timeout = %ld\n'>,dp_Arg5(a2)

		move.l	dp_Arg1(a2),rl_rwddata(a3)	associate with file
		move.l	dp_Arg2(a2),d0		get start position for lock
		move.l	d0,rl_LowBound(a3)	save as lowbound
		add.l	dp_Arg3(a2),d0		compute upper bound
		subq.l	#1,d0
		move.l	d0,rl_HighBound(a3)
		move.l	dp_Arg5(a2),d1		get the timeout
		move.l	dp_Arg4(a2),d0		get the mode
		lsr.l	#1,d0			convert from 0-3 to 0-1
		bcc.s	11$
		moveq.l	#0,d1			immediate lock, ignore timeout
11$		move.l	d0,rl_LockType(a3)
		move.l	d1,rl_Timeout(a3)	save requested timeout
		divu.w	#50,d1			convert DOS ticks to timeval
		move.w	d1,rl_ESecs+2(a3)	end after this many seconds
		swap	d1			get remainder
		mulu.w	#20000,d1		convert ticks to micros
		move.l	d1,rl_EMicros(a3)
		lea.l	rl_ESecs(a3),a0		convert to absolute time
		lea.l	IOTV_TIME+TV_SECS(a3),a1  we did getsystime earlier
	printf <'Current time = %ld,%ld\n'>,(a1),4(a1)
		move.l	a6,-(sp)
		movea.l	IO_DEVICE(a3),a6
		jsr	_LVOAddTime(a6)
		movea.l	(sp)+,a6		restore execbase
	printf <'Timeout time = %ld,%ld\n'>,rl_ESecs(a3),rl_EMicros(a3)
		lea.l	rl_FakePacket(a3),a0	point to packet
		movem.l	(sp)+,a2-a3		drop through to RelockRecord

;==============================================================================
; RelockRecord( packet )
;		  a0
;
; Gets a LOCK_TIMER packet with all its associated garbage (including the
; initial LOCK_RECORD packet in the rl_ structure).  Attempts to lock this
; record and returns the original packet if this is possible.  If the record
; cannot be locked checks the current time and compares it against the max
; allowed time in the rl_struct.  If there is time remaining then the rl_
; structure is fired off as a timer request and processed again when it comes
; back here.  Records being unlocked can cause this timer request to be
; aborted which will give the waiting record lock another chance.
;==============================================================================
RelockRecord	movem.l	d2-d4/a2-a3,-(sp)
		movea.l	dp_Link(a0),a3		get record lock structure
		tst.l	rl_BlockingMe(a3)	am I still being blocked ?
		bne	MaybeTimeout		yes, may just give up

; scan the whole list of record locks for this file and see if we can fit
		movea.l	rl_rwddata(a3),a2	fetch rwd header
		movea.l	rwd_Owner(a2),a2	get main header for file
		move.l	rwh_RecordLocks(a2),a2	fetch first entry in list
		move.l	rl_LowBound(a3),d2	stash low and high bounds
		move.l	rl_HighBound(a3),d3
	printf <'LoBound=%ld, HiBound=%ld\n'>,d2,d3

SearchRLocks	move.l	a2,d0			at end of list ?
		beq.s	CanLockRecord		yes, we can lock this record
		cmp.l	rl_LowBound(a2),d3	if low1>hi0 not overlapping
		blt.s	NoOverlap
		cmp.l	rl_HighBound(a2),d2	if hi1<low0 not overlapping
		bgt.s	NoOverlap
; if the same filehandle was used then an overlap is allowed even for exclusive
	printf <'Record locks overlap\n'>
		move.l	rl_rwddata(a3),d0
		cmp.l	rl_rwddata(a2),d0
		beq.s	NoOverlap
		moveq.l	#RL_EXCLUSIVE,d0	else only allowed for shared
		cmp.l	rl_LockType(a2),d0
		beq.s	LockOverlap
		cmp.l	rl_LockType(a3),d0
		beq.s	LockOverlap
NoOverlap	move.l	(a2),a2			no overlap, keep searching
		bra.s	SearchRLocks

; we can lock this record.  Add to the head of the list and return success
CanLockRecord	movea.l	rl_rwddata(a3),a2	fetch rwd header
		movea.l	rwd_Owner(a2),a2	get main header for file
		move.l	rwh_RecordLocks(a2),(a3) link into list
		move.l	a3,rwh_RecordLocks(a2)
		moveq.l	#0,d1			no error
		moveq.l	#TRUE,d0		locked successfully
		movea.l	rl_StartPacket(a3),a0
		bsr	ReturnPkt
		bra	RelockExit
		
; We have a record collision between EXCLUSIVE (write) record locks.  If the
; record we are trying to lock has exceeded it's timeout limit then we should
; fail it with a LOCK_TIMEOUT error.  If no timeout was set in the first
; place then we'll just return a simple LOCK_COLLISION error.
LockOverlap	tst.l	rl_Timeout(a3)		was a timeout requested?
		bne.s	TryTimeout		yes, see if we have time left

		move.w	#ERROR_LOCK_COLLISION,ErrorCode+2(a5)
	printf <'ERROR_LOCK_COLLISION\n'>
LockFail	movea.l	a3,a1			no, free the record and exit
	printf <'LockFail called\n'>
		movea.l	rl_StartPacket(a3),a2	stash original packet
		moveq.l	#rl_SIZEOF,d0
		jsr	_LVOFreeMem(a6)
		move.l	ErrorCode(a5),d1
		moveq.l	#FALSE,d0		failed
		movea.l	a2,a0			return this packet
		bsr	ReturnPkt
		bra	RelockExit		and exit now

; We've found a new record blocking us.  Set ourselves up on the blocking chain
TryTimeout	tst.l	rl_Blocking(a2)		is other guy blocking another
		beq.s	10$			nope, mark us as waiting
		movea.l	rl_Blocking(a2),a2	yes, skip down the chain
		bra.s	TryTimeout
10$		move.l	a3,rl_Blocking(a2)	you're blocking me
		move.l	a2,rl_BlockingMe(a3)	this one is blocking me

; Someone is blocking us.  See if we have time left to wait around for a while
MaybeTimeout	move.w	#TR_GETSYSTIME,IO_COMMAND(a3)
		move.b	#IOF_QUICK,IO_FLAGS(a3)
		movea.l	a3,a1			get the current time
		jsr	_LVODoIO(a6)
	printf <'Current time = %ld,%ld\n'>,IOTV_TIME+TV_SECS(a3),IOTV_TIME+TV_MICRO(a3)
	printf <'Timeout time = %ld,%ld\n'>,rl_ESecs(a3),rl_EMicros(a3)
		lea.l	IOTV_TIME+TV_SECS(a3),a0
		lea.l	rl_ESecs(a3),a1
		move.l	a6,-(sp)
		movea.l	IO_DEVICE(a3),a6
		jsr	_LVOCmpTime(a6)
		movea.l	(sp)+,a6
		tst.l	d0
		bgt.s	CanWait

; we've run out of time.  Unlink ourselves from the blocking chain (if there
; is one) and return a lock timeout error
CantWait	tst.l	rl_BlockingMe(a3)	anyone blocking us ?
		beq.s	10$			no (this shouldn't happen)
		movea.l	rl_BlockingMe(a3),a1	transfer blocking address
		move.l	rl_Blocking(a3),a0
		move.l	a0,rl_Blocking(a1)	maybe 0 here
		beq.s	10$
		move.l	a1,rl_BlockingMe(a0)	removed from middle of chain
10$		move.w	#ERROR_LOCK_TIMEOUT,ErrorCode+2(a5)
	printf <'ERROR_LOCK_TIMEOUT\n'>
		bra	LockFail

; we can do a timeout.  Calculate how much time we have left and wait that long
CanWait		move.l	IOTV_TIME+TV_MICRO(a3),-(sp)
		move.l	IOTV_TIME+TV_SECS(a3),-(sp)
		move.l	rl_ESecs(a3),IOTV_TIME+TV_SECS(a3)
		move.l	rl_EMicros(a3),IOTV_TIME+TV_MICRO(a3)
		lea.l	IOTV_TIME+TV_SECS(a3),a0
		movea.l	sp,a1
		move.l	a6,-(sp)
		movea.l	IO_DEVICE(a3),a6
		jsr	_LVOSubTime(a6)
		movea.l	(sp)+,a6
		addq.l	#8,sp			scrap tmps on stack
		move.w	#TR_ADDREQUEST,IO_COMMAND(a3)
	printf <'Waiting this long = %ld,%ld\n'>,IOTV_TIME+TV_SECS(a3),IOTV_TIME+TV_MICRO(a3)
		movea.l	a3,a1
		jsr	_LVOSendIO(a6)		will come back later

RelockExit	movem.l	(sp)+,d2-d4/a2-a3
		rts

;==============================================================================
; success = FreeRecord(packet)
;
; Frees the given record lock.  Record locks that are waiting on a timeout
; will never be sent to this routine (cause they aren`t locked yet).  If
; rl_Blocking is non NULL the the record lock that is being pointed to will
; have it`s pending TimerRequest returned via AbortIO.
;
; dp_Arg1 = rwdata structure
; dp_Arg2 = offset
; dp_arg3 = length
;==============================================================================
FreeRecord	movem.l	d2-d4/a2-a3,-(sp)
		movea.l	a0,a2			save packet
		movem.l	dp_Arg2(a2),d0-d1	get offset and length
		add.l	d0,d1			convert d0/d1 to lo/hi bounds
		subq.l	#1,d1
		move.l	d0,d2			and save them
		move.l	d1,d3
		movea.l	dp_Arg1(a2),a3		fetch rwd structure
		movea.l	rwd_Owner(a3),a0	get main header structure
		lea.l	rwh_RecordLocks(a0),a0	point at first rlock pointer

10$		move.l	(a0),d1			get next record
		beq.s	FreeNotLocked		none, can`t be locked
		movea.l	d1,a1
		cmpa.l	rl_rwddata(a1),a3	does it belong to this filehand;e
		bne.s	20$			nope, skip to next
		cmp.l	rl_LowBound(a1),d2	is this the same rlock ?
		bne.s	20$			nope
		cmp.l	rl_HighBound(a1),d3
		beq.s	FoundFree		found appropriate record
20$		movea.l	a1,a0
		bra.s	10$			link to next

; we`ve found the appropriate record, unlink it from the locked records list
FoundFree	move.l	(a1),(a0)		unlink from list
		movea.l	rl_Blocking(a1),a0	are we blocking anyone
		move.l	a0,d0
		beq.s	NotBlocking		nope, nothing to do

; someone else is waiting for this record to be freed.  Abort thier timer
; request and clear thier rl_BlockingMe field to show that this record
; is now available (without having to scan the list again).
		clr.l	rl_BlockingMe(a0)	not blocking now
		move.l	a1,-(sp)		save our record lock
		movea.l	a0,a1			get thier TimerRequest
		jsr	_LVOAbortIO(a6)		and abort it
		movea.l	(sp)+,a1		restore our record

; All that remains to be done is to free the memory used by the record lock
; structure.  If it`s the last one, then we should free the header too (a3).
NotBlocking	moveq.l	#rl_SIZEOF,d0
		jsr	_LVOFreeMem(a6)

; everything worked OK.  Return TRUE for no error
FreeNoError	moveq.l	#TRUE,d0
		bra.s	FreeDone
FreeNotLocked	moveq.l	#FALSE,d0
		move.w	#ERROR_RECORD_NOT_LOCKED,ErrorCode+2(a5)
		bra.s	FreeDone
FreeUnlockError	move.w	#ERROR_UNLOCK_ERROR,ErrorCode+2(a5)
FreeDone	movea.l	a2,a0			return the packet
		move.l	ErrorCode(a5),d1
		bsr	ReturnPkt
		movem.l	(sp)+,d2-d4/a2-a3
		rts

;==============================================================================
; FreeAllRLocks(file,lock)
;		 a0   a1
;
; Frees all record locks being held on the given file with the given filehandle
;==============================================================================
FreeAllRLocks	rts	***** race conditions, can't do it *****

		END
