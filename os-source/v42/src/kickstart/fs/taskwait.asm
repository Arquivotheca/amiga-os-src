		SECTION filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/tasks.i"
		INCLUDE	"libraries/dosextens.i"
		INCLUDE	"globals.i"
		INCLUDE	"actions.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XREF	_LVOWait,_LVOGetMsg,_LVOSetSignal

		XDEF	TaskWait

SIGMASK  EQU  $100

;==============================================================================
; Packet = TaskWait(),_AbsExecBase
;   d0			  a6
;
; The main work fetcher of the filing system.  Fetches the next packet that is
; destined for this filing system.  Only fetches one packet at a time so the
; message port is checked before performing a Wait because multiple messages
; could be queued at the port but the signal could have been cleared.
;==============================================================================
TaskWait	move.l	d2,-(sp)
		moveq.l	#0,d2			assume no diskchange
		movea.l	ThisTask(a5),a0		point to our task

; check if we should call any special routine to wait for incoming packets
		tst.l	pr_PktWait(a0)
		beq.s	GetPkt			nope, normal procedure
		movea.l	pr_PktWait(a0),a0	call this wait routine

; the old version of this code (running under a BCPL envirenmont) had d0
; pointing directly to the process part of the task structure.
		move.l	ThisDevProc(a5),d0	**** historical reasons *****
		jsr	(a0)			returns packet in d0
		bra.s	GotPkt

; Go to sleep and wait for a packet to arrive at our port
WaitPkt		move.l	#SIGMASK,d0
		or.l	DiskSig(a5),d0		wait for disk change sig too
		jsr	_LVOWait(a6)
		move.l	DiskSig(a5),d2		see if disk changed
		and.l	d0,d2			d2=0 if no change
GetPkt		movea.l	ThisDevProc(a5),a0	get our port address
		jsr	_LVOGetMsg(a6)		and check for message
		tst.l	d2			if diskchanged
		bne.s	GotPkt			don't worry about message
		tst.l	d0			did a message arrive yet
		beq.s	WaitPkt			no, so sleep and wait for it

; Message received. Return the packet pointer from the name field.
GotPkt		movea.l	d0,a0
		move.l	a0,d0
		beq.s	NoPacket		just got change flag
		move.l	LN_NAME(a0),d0		return in d0 for C

; new addition, if we didn't do a Wait() or we called an alternate getpacket
; routine then we'll need to check without waiting to see if the disk changed
NoPacket	tst.l	d2			did disk change
		bne.s	ChangeCheckDone		yes, no need to check

		move.l	d0,-(sp)		save packet pointer
		moveq.l	#0,d0
		moveq.l	#0,d1
		jsr	_LVOSetSignal(a6)	get current signals
		and.l	DiskSig(a5),d0		did diskchange occur?
		beq.s	NoChange		nope, so d2=0
		move.l	d0,d2			save non-0 sig value
		move.l	d0,d1			going to reset this signal
		moveq.l	#0,d0
		jsr	_LVOSetSignal(a6)
NoChange	move.l	(sp)+,d0		restore packet pointer
ChangeCheckDone	move.l	d2,d1			return change flag in d1
		move.l	(sp)+,d2
		rts

		END
