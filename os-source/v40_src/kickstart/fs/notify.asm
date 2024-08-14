		SECTION filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/nodes.i"
		INCLUDE	"libraries/dosextens.i"
		INCLUDE	"devices/trackdisk.i"
		INCLUDE	"actions.i"
		INCLUDE	"moredos.i"
		INCLUDE	"globals.i"
		INCLUDE	"private.i"
		INCLUDE "notify.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XDEF	AddNotify,RemNotify,CheckNotify,ReturnNotify

		XREF	GetPubMem,_LVOSignal,_LVOPutMsg,_LVOFreeMem
		XREF	SplitName,FindEntry,WorkDone,WorkFail

;==============================================================================
; AddNotify(packet)
;	      d0
;
; Adds a new notify request to the current volume.  dp_Arg1 contains the
; notify request.  If no disk is in the drive then an error is returned.
; Since disk access is required, this routine is executed as a co-routine.
;==============================================================================
AddNotify	movea.l	d0,a0			packet to a0 for WorkFail
	printf <'\nAddNotify(0x%lx)\n'>,a0
		move.l	CurrentVolume(a5),d2	any disk ?
		bne.s	10$			yep, no problems
		move.w	#ERROR_DEVICE_NOT_MOUNTED,ErrorCode+2(a5)
		bra	WorkFail		wind up

; we have a disk in the drive, so now create a NotifyEntry for this request
10$		movea.l	a0,a2			save the packet
		move.l	#ne_SIZEOF,d0
		bsr	GetPubMem
		movea.l	d0,a0
		move.l	a0,d0
		bne.s	20$			go the memory OK
		move.w	#ERROR_NO_FREE_STORE,ErrorCode+2(a5)
		movea.l	a2,a0
		bra	WorkFail

; we got the memory, link in the notify request and link it to the volnode
20$		move.l	dp_Arg1(a2),ne_NotifyReq(a0)
		movea.l	d2,a1
		adda.l	a1,a1
		adda.l	a1,a1
		move.l	dl_unused(a1),(a0)
		move.l	a0,dl_unused(a1)

; we need a BSTR version of nr_FullName so that splitname will work properly
		lea.l	ne_FullName(a0),a3
		clr.b	(a3)+
		movea.l	dp_Arg1(a2),a4
		movea.l	nr_FullName(a4),a4
		moveq.l	#0,d0
		bra.s	40$
30$		addq.w	#1,d0
40$		move.b	(a4)+,(a3)+
		bne.s	30$
		lea.l	ne_FullName(a0),a3
		move.b	d0,(a3)			ne_FullName holds BSTR now

; everything is allocated, now build the rest of the NotifyEntry (check files)
		bsr.s	BuildNotify		set up key and orphan-flag
		movea.l	d0,a0			see if we should notify now
		tst.l	ne_Orphaned(a0)		if file doesn`t exist
		bne.s	50$			then don`t notify initial
		movea.l	ne_NotifyReq(a0),a1
		moveq.l	#NRF_NOTIFY_INITIAL,d0
		and.l	nr_Flags(a1),d0
		beq.s	50$			no need to notify
		bsr	DoNotify		yes, please do a notify now

; added a node and all completed, wind up with a successful packet
50$	printf <'Notifyrequest added OK\n'>
		clr.l	ErrorCode(a5)		no errors occured
		movea.l	a2,a0
		moveq.l	#TRUE,d0
		bra	WorkDone

;==============================================================================
; NotifyEntry = BuildNotify( NotifyEntry )
;     d0			 a0
;
; This is a biggie!  Given a notify entry, searches the disk using the full
; name from the root and determines if that name exists.  If it does then the
; notify entry is marked as not an orphan and the header key of the file or
; directory to be watched for changes is set up.  If the entry does not
; exist, then the header key of the last entry found is used and the notify
; entry is flagged as orphaned.  Whenever the header being watched is altered
; this routine will be called again to check if the list needs re-building.
;==============================================================================
BuildNotify	movem.l	d2-d3/a2-a4,-(sp)
		movea.l	a0,a2			save NotifyEntry
		clr.l	ne_Orphaned(a2)		assume not orphaned
		move.l	RootKey(a5),d2		always start at root key
		move.l	d2,ne_HeaderKey(a2)	and mark as default key
		moveq.l	#1,d3			d3 = current pos in string
		lea.l	ne_FullName(a2),a3	use BSTR version of nr_FullName
		lea.l	-32(sp),sp		get name workspace on stack
		movea.l	sp,a4			and use a4 to point to it

		movea.l	a4,a0			strip off "volume:"
		movea.l	a3,a1			from this string
		moveq.l	#':',d0
		move.w	d3,d1			starting at this position
		bsr	SplitName
		move.w	d0,d3			get new pointer
		bne.s	FindLoop
		moveq.l	#1,d3			there was no : in the name

; loop around finding all entries in the given path name.  As soon as one fails
; to be found, we mark the NotifyEntry as being orphaned and save the last key
; as the one to be watched.  If we get to the end of the string, we will have
; the headerkey of the required entry and won`t need to mark as orphaned.
FindLoop	movea.l	a4,a0			get next name to this buffer
		movea.l	a3,a1			from this string
		move.w	d3,d1			starting at this position
		moveq.l	#'/',d0			using this delimiter
		bsr	SplitName
		move.w	d0,d3
		beq.s	LastEntry		no more directory headers

; we have an entry to search for in the current key.  See if it`s present.
		move.l	d2,d0			search this header key
		movea.l	a4,a0			for this name
		moveq.l	#TRUE,d1		follow links please
		bsr	FindEntry
		tst.l	d0			was the entry there ?
		beq.s	OrphanedEntry		nope, d2 holds last valid key
		move.l	ObjKey(a5),d2		yep, get key of this entry
		bra.s	FindLoop		and look for the next entry

; we`ve scanned all directories in the path, see if there is a filename to
; be matched now.  If (a4)==0 then it is a directory notification anyway.
LastEntry	tst.b	(a4)			any name left ?
		beq.s	DoneBuild		nope, so all completed
		move.l	d2,d0			yes, search this header key
		movea.l	a4,a0			for this name
		moveq.l	#TRUE,d1		follow links please
		bsr	FindEntry
		tst.l	d0			was the entry there ?
		beq.s	OrphanedEntry		nope, d2 holds last valid key
		move.l	ObjKey(a5),d2		yep, get key of this entry
	printf <'Entry is at %ld\n'>,d2
		bra.s	DoneBuild		and wind up

; We failed to find all of the names in the path.  So mark this as Orphaned
OrphanedEntry	moveq.l	#TRUE,d0
		move.l	d0,ne_Orphaned(a2)
	printf <'Entry is an orphan\n'>

; Winding up, d2 holds the header key we should be watching for changes.
DoneBuild	lea.l	32(sp),sp		reclaim stack space
		move.l	d2,ne_HeaderKey(a2)	watching this header
		move.l	a2,d0			return NotifyEntry
		movem.l	(sp)+,d2-d3/a2-a4
		rts

;==============================================================================
; DoNotify(NotifyEntry)
;	       a0
;
; Sends a NotifyMessage or signals the required task because something has
; changed about the required file or directory.  Does not check for orphaned
; NotifyEntry`s because we may have just deleted it anyway.
;==============================================================================
DoNotify	movem.l	a2-a3,-(sp)
	printf <'DoNotify called\n'>
		movea.l	a0,a2			stash NotifyEntry
		movea.l	ne_NotifyReq(a2),a3	get actual NotifyRequest
		tst.l	nr_MsgCount(a3)		any messages pending
		beq.s	10$			nope, no need for magic
		moveq.l	#NRF_WAIT_REPLY,d0
		and.l	nr_Flags(a3),d0
		beq.s	10$			can send message OK

; there are some messages pending and we are supposed to wait for the reply
; before we send any more.  Set the NRF_MAGIC flag so we do deferred notify
		ori.l	#NRF_MAGIC,nr_Flags(a3)
		bra.s	NotifyDone		will come back later

; It`s OK to do a notification now.  Check if we should Signal or SendMsg.
10$		moveq.l	#NRF_SEND_MESSAGE,d0	should we do a message ?
		and.l	nr_Flags(a3),d0
		bne.s	DoMessage		yep
		moveq.l	#NRF_SEND_SIGNAL,d0	check we really want a signal
		and.l	nr_Flags(a3),d0
		beq.s	NotifyDone		nope, do nothing then

; Caller just wants us to Signal with the required signal bit for notification
	printf <'DoNotify, sending a signal\n'>
		movea.l	nr_Task(a3),a1		signal this task
		moveq.l	#0,d0
		move.b	nr_SignalNum(a3),d1	with this signal bit
		bset.l	d1,d0
		jsr	_LVOSignal(a6)
		bra.s	NotifyDone		all completed

; Caller wants us to send a NotifyMessage to thier chosen message port.
DoMessage	moveq.l	#NotifyMessage_SIZEOF+dp_Arg1,d0	get a notify message
		bsr	GetPubMem
		movea.l	d0,a1
		move.l	a1,d0
		beq.s	NotifyDone		don`t have the memory

; we link the name to a small fake packet so that we can receive at main port
		lea.l	NotifyMessage_SIZEOF(a1),a0
		move.l	a0,LN_NAME(a1)
		move.l	#ACTION_NOTIFY_RETURN,dp_Type(a0)
		move.l	a1,dp_Link(a0)		link packet back to message

		move.l	ThisDevProc(a5),MN_REPLYPORT(a1)
		move.l	#NOTIFY_CLASS,nm_Class(a1)
		move.l	#NOTIFY_CODE,nm_Code(a1)
		move.l	a3,nm_NReq(a1)		point to NotifyRequest too
		moveq.l	#NRF_WAIT_REPLY,d0
		and.l	nr_Flags(a3),d0
		move.l	d0,nm_DoNotTouch(a1)	will be 0 or WAIT_REPLY
		move.l	a2,nm_DoNotTouch2(a1)	also point at NotifyEntry
		move.l	a1,ne_Msg(a2)		and point NotifyEntry at this
		addq.l	#1,nr_MsgCount(a3)	one message outstanding
		movea.l	nr_Port(a3),a0
	printf <'DoNotify, sending a message\n'>
		jsr	_LVOPutMsg(a6)		send the NotifyMessage

NotifyDone	movem.l	(sp)+,a2-a3
		rts


;==============================================================================
; ReturnNotify(FakePacket)
;		  d0
;
; Handles returning NotifyMessages and does deferred notifies if required.
;==============================================================================
ReturnNotify	moveq.l	#NRF_WAIT_REPLY,d0	RemNotify clears WAIT_REPLY
	printf <'Got a notify message back\n'>
		and.l	nm_DoNotTouch(a0),d0
		beq.s	FreeNotify		no need to check for deferral

; The NotifyRequest associated with this message is still active.
		movea.l	nm_NReq(a0),a1		get NotifyRequest entry
		subq.l	#1,nr_MsgCount(a1)	one less message outstanding
		move.l	#NRF_MAGIC,d0		if NotifyRequest was modified
		and.l	nr_Flags(a1),d0
		beq.s	FreeNotify
		movem.l	a0-a1,-(sp)		then notify again
		movea.l	nm_DoNotTouch2(a0),a0	using this NotifyEntry
		bsr	DoNotify
		movem.l	(sp)+,a0-a1
		andi.l	#~NRF_MAGIC,nr_Flags(a1)

; we`ve done whatever magic needs doing, now just free the NotifyMessage
FreeNotify	moveq.l	#NotifyMessage_SIZEOF+dp_Arg1,d0
		movea.l	a0,a1
		jmp	_LVOFreeMem(a6)		jsr/rts

;==============================================================================
; RemNotify( NotifyRequest )
;		  a0
;
; Removes the given NotifyRequest from the list and makes sure that returning
; NotifyMessages (for this NotifyRequest) can`t cause any trouble.
;==============================================================================
RemNotify	movem.l	a2-a3,-(sp)
		movea.l	CurrentVolume(a5),a2	find the request
		adda.l	a2,a2
		adda.l	a2,a2
		move.l	a2,d0			make sure it`s there
		beq.s	RemNoVolume		nope, exit now

; search the list of notifies for the given NotifyRequest and remove it
		lea.l	dl_unused(a2),a2	point to first linkage
FindIt		movea.l	(a2),a3			get next node
		move.l	a3,d0
		beq.s	RemNoVolume		didn`t find it
		cmpa.l	ne_NotifyReq(a3),a0	is this the one
		beq.s	FoundIt			yep
		movea.l	a3,a2			nope, skip to next
		bra.s	FindIt

; we found the NotifyEntry, unlink it from the list and do the right stuff
FoundIt		move.l	(a3),(a2)		unlink from list
		movea.l	ne_NotifyReq(a3),a2	a2 = NotifyRequest a3=Entry
		tst.l	nr_MsgCount(a2)		any messages outstanding ?
		beq.s	NoFancyWork		nope, so just remove the node
		moveq.l	#NRF_WAIT_REPLY,d0
		and.l	nr_Flags(a2),d0
		beq.s	NoFancyWork
		movea.l	ne_Msg(a3),a0		yes, mark the message
		andi.l	#~NRF_WAIT_REPLY,nm_DoNotTouch(a0)
	
; no pending NotifyMessages to worry about, so now free the NotifyEntry memory
NoFancyWork	movea.l	a3,a1
		move.l	#ne_SIZEOF,d0
		jsr	_LVOFreeMem(a6)
RemNoVolume	movem.l	(sp)+,a2-a3
		rts


;==============================================================================
; CheckNotify(header,rebuild)
;		d0     d1
;
; Given a header key that has changed in some way searches the list of notify
; requests for any notifies that should be fired off.  Any NotifyEntries that
; match header (and are not orphans) will have thier corresponding messages
; fired off unconditionally followed by a rebuild of the node if the rebuild
; flag is set to TRUE.  This handles files that get deleted.
;
; Any nodes that ARE orphans will have their nodes rebuilt if the rebuild flag
; is TRUE.  If, after the rebuild, the node is no longer an orphan, then a
; NotifyMessage will be sent out too.  (Of course, header must match first)
;
; This routine MUST be run from inside a co-routine (it may read the disk).
;==============================================================================
CheckNotify	movem.l	d2-d3/a2-a3,-(sp)
		move.l	d0,d2			save header key
		move.l	d1,d3			and rebuild flag
		movea.l	CurrentVolume(a5),a2	get to head of list
		adda.l	a2,a2
		adda.l	a2,a2
		movea.l	dl_unused(a2),a2

CheckLoop	move.l	a2,d0			anything left to search ?
		beq.s	CheckDone		nope, at end of list
		cmp.l	ne_HeaderKey(a2),d2	are we interested ?
		bne.s	CheckNext		nope, header doesn`t match

; we have a NotifyHeader with a matching header key.  See what we should do
		move.l	ne_Orphaned(a2),-(sp)	save the orphaned flag
		tst.l	d3			should we rebuild ?
		beq.s	NoBuild			nope
		movea.l	a2,a0			rebuild this node
		bsr	BuildNotify
NoBuild		move.l	(sp)+,d0		was this an orphan to start ?
		beq.s	YesNotify		nope, so notify anyway
		tst.l	ne_Orphaned(a2)		did it just become unorphaned?
		bne.s	CheckNext		nope, so no notification
YesNotify	movea.l	a2,a0			yep, so do notification
		bsr	DoNotify

CheckNext	movea.l	(a2),a2
		bra.s	CheckLoop
CheckDone	movem.l	(sp)+,d2-d3/a2-a3
		rts


		END

		
