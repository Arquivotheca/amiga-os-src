		SECTION	driver,CODE
;==============================================================================
; This module handles all the hardware specific operations for sending command
; blocks to the controller and interpreting returned interrupts.  Threading of
; requests to different units and the disconnect/reconnect stuff is here too.
;
; This version is for the A590/A2091 with DMA driven SCSI and XT controllers.
;==============================================================================
		NOLIST
		INCLUDE	"modifiers.i"
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/interrupts.i"
		INCLUDE	"exec/lists.i"
		INCLUDE	"exec/ports.i"
		INCLUDE	"exec/tasks.i"
		INCLUDE	"exec/execbase.i"
		INCLUDE	"device.i"
		INCLUDE	"scsitask.i"
		INCLUDE	"board.i"
		INCLUDE	"scsidirect.i"
		INCLUDE	"hardblocks.i"

		INCLUDE	"resources/battmembitsshared.i"
		INCLUDE	"resources/battmembitsamiga.i"
		INCLUDE	"resources/battmem.i"

		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XDEF	SCSITask,FakeSelect,FakeDisconnect,FakeTimeout

		XREF	_LVOSignal,_LVOWait,_LVOGetMsg,_LVOReplyMsg
		XREF	_LVOAllocSignal,_LVOAddIntServer,_LVOFindTask
		XREF	_LVOSupervisor,_LVOCachePreDMA,_LVOCachePostDMA
		XREF	_LVOReadBattMem,_LVOOpenResource

		XREF	GetPubMem

		XREF	XTSelect,XTSendCommand,XTAddressBad,XTCommandDone

;==============================================================================
; These macros provide safe access to the WDC registers from within SCSITask.
; Before accessing a WDC register, the register number is put in st_WDCRegister
; so the interrupt routine can restore the tasks idea of the current register
; if it happens to interrupt SCSITask between the loading of SASR and the
; subsequent access of SCMD.  Not all uses are nescessary (because an interrupt
; cannot happen at that time) but I'm using this nearly everywhere for safety.
;==============================================================================
; added a new fix.  Musts always write SASR as longword at $40 (can still read
; as a byte at $41 though).
PUTREG		MACRO				;PUTREG operand,register
		move.b	#\2,st_WDCRegister(a5)	stash register being accessed
	IFD SASRW
		move.l	#\2,SASRW(a4)		select that register
	ENDIF
	IFND SASRW
		move.b	#\2,SASR(a4)		select that register
	ENDIF
		move.b	\1,SCMD(a4)		and store the value
		ENDM

JAMREG		MACRO				;JAMREG operand,register
	IFD SASRW
		move.l	#\2,SASRW(a4)		select a register
	ENDIF
	IFND SASRW
		move.b	#\2,SASR(a4)		select a register
	ENDIF
		move.b	\1,SCMD(a4)		and store the value
		ENDM				use for non-critical sections

GETREG		MACRO				;GETREG register,operand
		move.b	#\1,st_WDCRegister(a5)	stash register being accessed
	IFD SASRW
		move.l	#\1,SASRW(a4)		select that register
	ENDIF
	IFND SASRW
		move.b	#\1,SASR(a4)		select that register
	ENDIF
		move.b	SCMD(a4),\2		and fetch the contents
		ENDM

;==============================================================================
; Interrupt server used to signal the SCSI/XT task that an interrupt occurred
; on our board.  On entry a1 holds a ptr to the globals of the SCSI/XT task.
;==============================================================================
BoardServer	moveq.l	#0,d0			assume it's not our interrupt
		movea.l	st_BoardAddress(a1),a0	point to the board
		btst.b	#DMAB_PEND,ISTR(a0)	are any interrupts pending ?
		beq.s	BoardServed		nope, exit immediately

; we can't let the main SCSITask clear the interrupts because if we don't do
; it here, we'll get another one the moment we exit from this interrupt code
; I think the maximum possible number of interrupts is 3 in a row (without
; any diddling of the registers between interrupts).  We'll maintain a simple
; circular list of the appropriate register contents when the interrupt occurs
		movem.l	d2/a6,-(sp)
		btst.b	#DMAB_EOP,ISTR(a0)	was there an EOP
		beq.s	10$			no, don't clear the int
		move.b	#0,CINT(a0)		clear int status

; we don't bother telling the main SCSITask about terminal count interrupts
10$		btst.b	#DMAB_PINT,ISTR(a0)	did we get peripheral int ?
		beq.s	20$			no, so don't read anything

		move.w	st_IntPointer(a1),d2	get current int pointer
		move.b	ISTR(a0),st_IntData(a1,d2.w)	save ISTR status
		move.b	SASR(a0),st_IntData+1(a1,d2.w)  save status reg

	IFD SASRW
		move.l	#SCSI_STATUS,SASRW(a0)	read the SCSI status
	ENDIF
	IFND SASRW
		move.b	#SCSI_STATUS,SASR(a0)	read the SCSI status
	ENDIF
		move.b	SCMD(a0),d0
		move.b	d0,st_IntData+2(a1,d2.w) and save it for SCSITask

	IFD XT_SUPPORTED
		btst.b	#5,XTPORT1(a0)		did the XT interrupt us ?
		beq.s	15$			nope
		move.b	XTPORT1(a0),st_IntData+3(a1,d2.w)  save XT status too
		move.b	#0,XTPORT3(a0)		clear XT interrupt status
	ENDC

15$		move.w	d2,d0			save original pointer
		addq.w	#4,d2			bump int pointer
		andi.w	#$1f,d2			range is 0-31
		move.w	d2,st_IntPointer(a1)	save new value

; we've cleared all the interrupt status stuff, so signal IOTask about it.  We
; do a test to see if the task pointer is the same as the int pointer first. If
; it is, then we'll signal the task, if it isn't we won't bother because the
; task will continue processing interrupts until the pointers are the same.
; This has the effect of cutting down on task switching a little (hence we
; can process more commands per second).  We also reload SASR with the value
; in st_WDCRegister.  This is because the interrupt could happen just as
; SCSITask loaded SASR but before it read or wrote SCMD.

***fixed to write SASR as a longword
	IFD SASRW
		move.l	d0,-(sp)
		moveq.l	#0,d0
		move.b	st_WDCRegister(a1),d0
		move.l	d0,SASRW(a0)  restore tasks register
		move.l	(sp)+,d0
	ENDC
	IFND SASRW
		move.b	st_WDCRegister(a1),SASR(a0)
	ENDC
		cmp.w	st_TaskPointer(a1),d0	are these the same ?
		bne.s	20$			nope, task has been signalled
		movea.l	st_SysLib(a1),a6	we have a new interrupt
		move.l	st_IntPendMask(a1),d0	so signal the SCSI task
		movea.l	st_ThisTask(a1),a1	that an interrupt has occurred
		jsr	_LVOSignal(a6)
20$		moveq.l	#1,d0			we handled the interrupt
		movem.l	(sp)+,d2/a6
BoardServed	rts

	IFD XT_SUPPORTED
IntName		DC.B	'SCSI/XT',0
		CNOP	0,2
	ENDC

	IFND XT_SUPPORTED
IntName		DC.B	'_SCSI_',0
		CNOP	0,2
	ENDC

BattName	BATTMEMNAME

;==============================================================================
; This task gets started by the main IORequest handler task.  It's responsible
; for initialising the passed message port (for receiving device commands) and
; firing up the interrupt server that signals this task when interrupts arrive.
;
; ExecBase, BoardAddress, ParentTask and MsgPort are passed on the stack so we
; don't have to do lots of messaging gyrations in the parent task to find this
; port or worry about memory allocation problems.  All memory allocations are
; assumed to work OK (but I may need to fix this later).
;
; Stack on entry:
;	 4(sp)  = ExecBase		not nescessary really
;	 8(sp)  = BoardAddress		the physical address of our board
;	12(sp)	= ParentTask		the task that created us
;	16(sp)	= MessagePort		the port it wants to PutMsg to
;==============================================================================
SCSITask	movea.l	4(sp),a6		get Exec library
		moveq.l	#st_SIZEOF,d0		get our global memory area
		bsr	GetPubMem		(I will assume that this works)
		movea.l	d0,a5			a5 always points to the globals
		move.l	a6,st_SysLib(a5)		stash Exec base
		move.l	8(sp),st_BoardAddress(a5)	and board address
		move.l	16(sp),st_CmdPort(a5)		and port address
		movea.l	st_BoardAddress(a5),a4	a4 always points at the board

	IFD A590
		move.b	#0,CNTR(a4)		disable everything
		move.b	#0,SRST(a4)		make sure DMA is off
		move.b	#3,DAWR(a4)		data acknowledge width
		move.b	#0,CINT(a4)		clear pending interrupts

; we need to detect if we are using a Boyer DMA chip (which can only DMA to
; longword aligned addresses) or a new DMA chip.  The new chip doesn't
; support word transfer count so we can use this to determine which chip
		clr.b	st_FixBoyer(a5)		assume not Boyer chip
		moveq.l	#0,d0
		move.w	d0,WTCL(a4)		not using word xfer count
		cmp.w	WTCL(a4),d0
		bne.s	NotBoyer
		subq.w	#1,d0
		move.w	d0,WTCL(a4)
		cmp.w	WTCL(a4),d0
		bne.s	NotBoyer
		move.w	#$5555,d0
		move.w	d0,WTCL(a4)
		cmp.w	WTCL(a4),d0
		bne.s	NotBoyer
		move.b	#1,st_FixBoyer(a5)	got a boyer chip

NotBoyer:
	ENDC A590

	IFD A2090
		nop
		nop
	ENDC A2090

		suba.l	a1,a1			find this task
		jsr	_LVOFindTask(a6)
		move.l	d0,st_ThisTask(a5)	for use by interrupt routine

		moveq.l	#-1,d0			get a signal for the interrupt
		jsr	_LVOAllocSignal(a6)
		move.b	d0,st_IntPendSig(a5)	save the signal number
		moveq.l	#0,d1			and store as a mask too
		bset.l	d0,d1
		move.l	d1,st_IntPendMask(a5)

		moveq.l	#-1,d0			get a signal for the CmdPort
		jsr	_LVOAllocSignal(a6)
		move.b	d0,st_CmdPendSig(a5)	save the signal number
		moveq.l	#0,d1			and store as a mask too
		bset.l	d0,d1
		move.l	d1,st_CmdPendMask(a5)

		movea.l	st_CmdPort(a5),a1	initialise the message port
		move.b	d0,MP_SIGBIT(a1)	it uses this signal bit
		move.l	st_ThisTask(a5),MP_SIGTASK(a1)	it signals us
		move.b	#NT_MSGPORT,LN_TYPE(a1)	it's a simple message port
		lea.l	MP_MSGLIST(a1),a1
		NEWLIST	a1			it's empty (and not public)

		moveq.l	#IS_SIZE,d0		get interrupt server memory
		bsr	GetPubMem
		move.l	d0,st_IntServer(a5)	stash addr (maybe for cleanup)
		movea.l	d0,a1
		move.b	#NT_INTERRUPT,LN_TYPE(a1)
		move.b	#INTPRI,LN_PRI(a1)	this is our priority
		move.l	a5,IS_DATA(a1)		server can see our globals
		lea.l	BoardServer(pc),a0	where the code is
		move.l	a0,IS_CODE(a1)
		lea.l	IntName(pc),a0
		move.l	a0,LN_NAME(a1)
		moveq.l	#INTNUM,d0		hook to this handler chain
		jsr	_LVOAddIntServer(a6)	add our interrupt server

		lea.l	st_WaitingUnits(a5),a0	initialise WaitingUnits queue
		NEWLIST	a0
		lea.l	st_WorkingUnits(a5),a0	initialise WorkingUnits queue
		NEWLIST	a0

		move.l	#$40404040,st_SyncValues(a5)    init sync xfer values
		move.l	#$40404040,st_SyncValues+4(a5)  for asynchronous mode

; New addition for 2.0, check the battmem bits for our required jumper values
; and set up the defaults if battmem.resource is not present or memory corrupt
*******************************************************************************
		clr.b	st_MaxLUN(a5)		assume no LUNS >0
		move.b	#$10,st_SelTimeout(a5)	assume quick timeout
		move.b	#7,st_OwnID(a5)		assume host ID = 7

		move.w	LIB_VERSION(a6),d0	if <V36
		cmpi.w	#36,d0			then no battclock
		blt.s	BattMemDone		(not one that works anyway)

		lea.l	BattName(pc),a1
		jsr	_LVOOpenResource(a6)	get battmem resource
		tst.l	d0			did we get it ?
		beq.s	BattMemDone		nope, so not using 2.0

		move.l	a6,-(sp)		stash exec
		movea.l	d0,a6
		lea.l	st_SelTimeout(a5),a0	get required timeout
		moveq.l	#BATTMEM_SCSI_TIMEOUT_ADDR,d0
		moveq.l	#BATTMEM_SCSI_TIMEOUT_LEN,d1
		jsr	_LVOReadBattMem(a6)
		tst.b	st_SelTimeout(a5)	long or short ?
		bne.s	10$			long
		move.b	#$25,st_SelTimeout(a5)	short 250ms
		bra.s	20$
10$		move.b	#$ff,st_SelTimeout(a5)

20$		lea.l	st_MaxLUN(a5),a0	get required LUN behaviour
		moveq.l	#BATTMEM_SCSI_LUNS_ADDR,d0
		moveq.l	#BATTMEM_SCSI_LUNS_LEN,d1
		jsr	_LVOReadBattMem(a6)
		tst.b	st_MaxLUN(a5)		supporting luns > 0?
		beq.s	30$			nope, only support 0
		move.b	#7,st_MaxLUN(a5)	yes, support all LUNs

30$		lea.l	st_OwnID(a5),a0		get ID for host adaptor
		moveq.l	#BATTMEM_SCSI_HOST_ID_ADDR,d0
		moveq.l	#BATTMEM_SCSI_HOST_ID_LEN,d1
		jsr	_LVOReadBattMem(a6)
		eori.b	#7,st_OwnID(a5)		value is complemented
		move.l	(sp)+,a6		restore exec

*******************************************************************************

BattMemDone	moveq.l	#SIGF_SINGLE,d0		tell parent we are ready now
		movea.l	12(sp),a1		(it's waiting on SIGF_SINGLE)
		jsr	_LVOSignal(a6)

	IFD A590
		move.b	#DMAF_INTENA!DMAF_PMODE,CNTR(a4)
	ENDC

	IFD A2090
		nop
	ENDC

;==============================================================================
; Here's where all the real low level work gets done.  This task waits on two
; possible events; an interrupt occured or a new command arrived at the message
; port.  If an interrupt occured then we jump to the appropriate routines to
; handle that specific kind of interrupt.  This is documented with the handler
; routines themselves.  If a message arrived, then we'll queue it on the
; appropriate unit.  If that unit is not on one of our queues when the message
; arrives, it will be added to the WaitingUnits queue pending selection. If the
; unit is on one of our queues, then it's busy or waiting already so it will
; just stay where it is until it's time to service it again.
;==============================================================================
SCSIEventLoop	move.l	st_IntPendMask(a5),d0	get the sigs we are waiting for
		or.l	st_CmdPendMask(a5),d0
		jsr	_LVOWait(a6)		wait for something to happen

; we got a signal from the interrupt routine.  See what really interrupted us.
IntEventLoop	move.w	st_TaskPointer(a5),d0	get our interrupt frame pointer
		cmp.w	st_IntPointer(a5),d0	any ints outstanding ?
		beq	HandleCommand		nope, go look for a command
		move.w	d0,d2			save the current frame pointer
		addq.w	#4,d0			bump frame pointer for next
		andi.w	#$1f,d0
		move.w	d0,st_TaskPointer(a5)	and save for next time through
		btst.b	#DMAB_PINT,st_IntData(a5,d2.w)	was it peripheral int ?
		beq.s	IntEventLoop		nope, EOP or spurious

;==============================================================================
; We got a peripheral interrupt.  Here we have to determine if it was SCSI or
; XT that finished something.  Since XT commands only occur in the absence of
; working/current SCSI commands it's safe to test for SCSI and assume XT if
; it wasn't.  We can never get both interrupts at the same time.  (We use the
; WDC for XT address translation but that won't cause an XT interrupt anyway).
;==============================================================================
	IFD XT_SUPPORTED
		btst.b	#WDCB_INT,st_IntData+1(a5,d2.w)	SCSI int pending ?
		bne.s	SCSIService		yes, get status and handle it

; we got an XT interrupt, see which unit the interrupt occured on and handle it
		bsr	XTCommandDone		maybe finished an XT operation
		bra.s	IntEventLoop		look for another interrupt
	ENDC

;==============================================================================
; The main SCSI service dispatch point.  Nearly all WDC interrupts imply that
; some action is required by us so we'll use one of four possible branch tables
; to dispatch to the correct code based on the Status group of the interrupt.
; It's generally safe to bang on the WDC registers because we won't get another
; interrupt until we do.  Disconnect is an exception to this because another
; target device can re-select us immediately.  This is tested first in DoSelect
;==============================================================================
SCSIService	move.b	st_IntData+2(a5,d2.w),d0 get the saved status byte
		bne	NormalSCSI		it's a normal SCSI interrupt

; the WD SCSI controller has been reset by a command or power up.  We should
; initialise some registers to reasonable defaults.  If own ID is not set to 7
; we'll issue another reset command with the correct ID in the OWNID register.
; A590 and A2091 have three jumpers which can be used to modify the drivers
; behaviour.  These are not present on the A2090 board.
		GETREG	OWN_ID,d1		get the current own ID value
		andi.b	#7,d1			mask off clock bits
		cmp.b	st_OwnID(a5),d1		have we already set it ?
		beq.s	IDSet			yep, so no work to do

	IFD A590
; jumpers are inverted so we have to read the second XT port and invert it
;		move.b	XTPORT0+$20(a4),d0	uses second XT unit register
;		eori.b	#$07,d0

; jumper 0 enables selection of LUNs greater than 0 (lotsa things screw up)
;		clr.b	st_MaxLUN(a5)		assume highest LUN is 0
;		lsr.b	#1,d0
;		bcc.s	10$			correct
;		move.b	#7,st_MaxLUN(a5)	nope, we support LUNs

; jumper 1 sets the timeout on selection to 128ms (no jumper) or 2 seconds
; Seagate 225N has problems with ignoring parity msgs, jumper disables the
; parity error handling (on message phase) too.
;10$		move.b	#$10,st_SelTimeout(a5)	assume quick timeout
;		clr.b	st_ParityCheck(a5)	and checking parity
;		lsr.b	#1,d0
;		bcc.s	20$			correct
;		move.b	#$ff,st_SelTimeout(a5)	nope, use 2 second timeout
		move.b	#1,st_ParityCheck(a5)	and ignore message parity
	ENDC A590

; set the input frequency divisor to 3 as well
20$		move.b	st_OwnID(a5),d1		get required ID
		ori.b	#$40,d1			input clock divisor=3
		JAMREG	d1,OWN_ID		set the ID...
		JAMREG	#wd_RESET,COMMAND	and issue a reset command
		move.b	#1,st_SelectPending(a5)	can't allow selects to happen
		bra	IntEventLoop		look for another interrupt

; we issued a reset command so the OWN_ID register got sampled, do other regs.
IDSet		clr.b	st_SelectPending(a5)	selects are OK now
		JAMREG	#0,CONTROL		DMA mode disabled
		JAMREG	st_SelTimeout(a5),TIMEOUT  set timeout period
		JAMREG	#$40,SYNC_TRANSFER	asynchronous + req/ack=300ns
		JAMREG	#WDCF_ER,SOURCE_ID	enable reselection
		bra	IntEventLoop		and that's all

; dispatch a normal SCSI interrupt to the appropriate handler for that phase
NormalSCSI	moveq.l	#$0f,d1			mask off the lower 4 bits
		and.b	d0,d1			d1 holds the error code
		andi.w	#$00f0,d0		d0 upper 4 bits are group code
		lsr.w	#3,d0			d0 now holds GroupCode<<1
		lea.l	Group_Table(pc),a0	get table address for this group
		adda.w	0(a0,d0.w),a0		a0 now points to dispatch table
		lsl.w	#1,d1			accessing table of word offsets
		move.w	0(a0,d1.w),d1		get offset from table to code
		jsr	0(a0,d1.w)		and call the correct routine
		bra	IntEventLoop

;==============================================================================
; If we didn't get an interrupt signal or we got one and handled it already,
; we'll drop into this code.  It checks to see if a command signal came in
; and fetches it for queueing on the appropriate unit if one did.  The units
; are on their own list (hanging off the device base) and are pointed to by
; the IORequest that's stashed in our command blocks.  We can check the
; QueueType field of the unit to determine which queue we have it on (if any)
; and must add it to the WaitingUnits queue if it's not already active.  The
; queues maintained by this task are in addition to the normal unit list and
; are not touched by any other task.  We don't have to worry about race con-
; ditions this way and can forget about units that have no work pending.  This
; also makes it very simple to single thread commands to the same unit.
;==============================================================================
HandleCommand	bsr.s	DoSelect		maybe select another unit
		movea.l	st_CmdPort(a5),a0	fetching from this port
		jsr	_LVOGetMsg(a6)		get any queued messages
		tst.l	d0			did we get one ?
		beq	SCSIEventLoop		no, so wait for something

; we have a valid command block, queue it on this unit then look for more
	IFD DEBUG_CODE
	tst.b	st_WasteSpace(a5)
	beq.s	NMTO
	printf <'Received more than one command\n'>
NMTO	addq.b	#1,st_WasteSpace(a5)
	ENDC

		movea.l	d0,a1			stash command block address
		clr.b	cb_DidCacheStuff(a1)	no CachePreDMA call performed
		clr.b	cb_Programmed(a1)	DMA is OK so far

		movea.l	cb_IORequest(a1),a0	fetch the IORequest
		movea.l	IO_UNIT(a0),a2		and fetch pointer to Unit
		lea.l	hu_CmdList(a2),a0	point to command list header
		ADDTAIL				add cmd to end of command queue
		tst.b	hu_QueueType(a2)	is unit on one of our queues ?
		bne.s	HandleCommand		yes, don't queue unit again

; The unit is currently inactive.  Since we just got a request for this unit
; we must add it onto our queue of units waiting for selection (WaitingUnits)
; If there's no RunningUnit currently active and no Select is pending we'll
; start a select sequence for this unit; this is done in DoSelect().
		lea.l	st_WaitingUnits(a5),a0	add to end of WaitingUnits
		lea.l	hu_SCSIQueue(a2),a1	SCSI list node for linkage
		ADDTAIL
		move.b	#UNIT_WAITING,hu_QueueType(a2)	it's on this queue
		bra	HandleCommand		may select last unit now

;==============================================================================
; DoSelect()
;
; If there is no RunningUnit and SelectPending is false then a select command
; is issued for the first unit on the WaitingUnits queue.  We will always do a
; select with ATN since some devices will do a message phase after selection
; even if we say we don't support reselection (by selecting without ATN).
; The unit is left on the WaitingUnits queue until a "select complete" occurs.
; Added a test: if a SCSI int is pending there's no point trying a select cmd.
;
; There's special case code in here to prevent simultaneous use of XT and SCSI.
;
; Extra bug fix for WD33C93A, have to disable SBIC interrupts while selecting.
;==============================================================================
DoSelect	tst.b	st_SelectPending(a5)	is a selection pending ?
		bne.s	NoSelect		yes, so don't try another

		tst.l	st_RunningUnit(a5)	is a unit connected right now ?
		bne.s	NoSelect		yes, can't do a selection

		lea.l	st_WaitingUnits(a5),a0	are there any units waiting ?
		TSTLIST	a0
		beq.s	NoSelect		nope, so no work

		move.w	st_IntPointer(a5),d0	check if an int is pending
		cmp.w	st_TaskPointer(a5),d0
		bne.s	NoSelect		yep, select will fail anyway

		movea.l	LH_HEAD(a0),a0		fetch the units queue ptr
		lea.l	-hu_SCSIQueue(a0),a0	adjust to start of unit struct

; XT units never support reselection so we can't allow an XT device to become
; active until ALL other SCSI devices have finished their operations.  If the
; first unit on the WaitingUnits queue is an XT device then we must make sure
; there are no other units on the WorkingUnits queue before we start the XT
	IFD XT_SUPPORTED
		tst.b	hu_Type(a0)		is this XT or SCSI
		bne.s	SCSISelect		no problem, its SCSI
		lea.l	st_WorkingUnits(a5),a1	make sure no one else running
		TSTLIST	a1
		beq	XTSelect		fine, we can start the XT
		rts				else we have to quit now
	ENDC

; We have a pointer in a0 to the SCSI unit that we want to start selection on.
; Check if we are supporting logical units above 0 and time it out if we're not
SCSISelect	tst.b	st_MaxLUN(a5)		should we check the LUN ?
		bne.s	TrySelect		nope, we support them
; bug fix. can`t allow further selects to happen until we fake timeout this one
		move.b	#1,st_SelectPending(a5)	can't allow selects to happen
		tst.b	hu_LUN(a0)		yes, so only allow 0
		bne	FakeTimeout		not 0, make like timeout

TrySelect:
		move.b	CNTR(a4),d0		get control register
		bclr.l	#DMAB_INTENA,d0		turn off interrupts from SBIC
		move.b	d0,CNTR(a4)

		PUTREG	hu_Unit(a0),DEST_ID	the unit # we want to select
		PUTREG	#wd_SELECT_WITH_ATN,COMMAND	issue select with ATN
		move.b	#1,st_SelectPending(a5)	can't allow selects to happen

20$		bset.l	#DMAB_INTENA,d0		turn interrupts from SBIC on
		move.b	d0,CNTR(a4)

; command may or may not have been issued, wait for next interrupt to come in.
NoSelect	rts

;==============================================================================
; Unit = FindWorking( unit, LUN )
;  d0		       d0    d1
;
; Attempts to find the given unit on the WorkingUnits queue and returns a ptr
; to the Unit structure.  The unit will NOT have been removed from the queue
; if it was found.  A LUN value of -1 will cause this routine to only compare
; physical unit numbers and return the first one it finds.  This routine will
; only look at SCSI units since we never allow XT and SCSI units to be on the
; WorkingUnits queue at the same time.  Returns 0 if the unit wasn't found.
;
; NOTE: the returned pointer is to the head of the unit structure not to the
;       SCSIQueue entry of the unit structure.  Must adjust ptr before REMOVE
;==============================================================================
FindWorking	move.l	d2,-(sp)
		move.b	d0,d2			stash unit code
		move.l	st_WorkingUnits(a5),d0	fetch the first node

10$		movea.l	d0,a0			node pointer to a0
		move.l	(a0),d0			look ahead to next node
		beq.s	30$			end of list, return 0
; possible to do "cmp.b hu_Unit-hu_SCSIQueue(a0),d2" but it's not as readable
		lea.l	-hu_SCSIQueue(a0),a1	point to start of unit struct
		cmp.b	hu_Unit(a1),d2		does unit match ?
		bne.s	10$			nope, scan the next
		tst.b	d1			are we checking LUNS too ?
		bmi.s	20$			no, so we found a match
		cmp.b	hu_LUN(a1),d1		yes, does LUN match
		bne.s	10$			nope, scan the next
20$		move.l	a1,d0			return ptr to unit struct
30$		move.l	(sp)+,d2
		rts

;==============================================================================
; success and byte = SCSIGetByte(),BoardAddress
;    d1	       d0			a4
;
; Fetch a pending byte of data in single byte transfer mode.  I had to add the
; success flag in d1 because the WDC chip sometimes behaves wierdly on a msg
; in transfer after a reselection.  Occasionally, an invalid command interrupt
; will come in right after issuing the transfer command with SBT true.  I dont
; know why this happens, but the WDC recovers OK and just re-issues the msg in
; interrupt again (right after the invalid command int has been serviced).
;
; (I hope this doesn't ever happen on any other commands, or I'm broken!!!!)
;==============================================================================
SCSIGetByte	move.w	st_TaskPointer(a5),d0	get current interrupt ptr
		PUTREG	#0,CONTROL		turn off DMA mode
		PUTREG	#WDCF_SBT!wd_TRANSFER_INFO,COMMAND  issue command
		moveq.l	#-1,d1			assume success
GetLoop		cmp.w	st_IntPointer(a5),d0	did an interrupt occur
		bne.s	BadGet			yes, return failure
		btst.b	#WDCB_DBR,SASR(a4)	is data buffer ready ?
		beq.s	GetLoop			nope, wait
		GETREG	DATA_REG,d0		yep, fetch the data byte
		rts				return d1=TRUE, d0=data

; added a test to see if the interrupt was a bad command interrupt.  If it
; was then it was caused by a selection failing due to an incoming reselect.
; Since we`re usually fetching the identify message when this happens we
; just ignore it and carry on polling for data.
BadGet:
		move.w	st_TaskPointer(a5),d0	get current interrupt ptr
		move.b	st_IntData+2(a5,d0.w),d0 get what caused interrupt
		cmpi.b	#$40,d0			was it bad command int ?
		bne.s	ReallyBadGet		nope, so bail out
		move.w	st_IntPointer(a5),d0	use new pointer
		bra.s	GetLoop

ReallyBadGet	moveq.l	#0,d1			got unexpected interrupt
		rts				return d1=FALSE

;==============================================================================
; SCSIPutByte( byte ),BoardAddress
;		d0	   a4
;
; Send a byte of data to the SCSI bus in single byte transfer mode.  The target
; MUST be requesting data out when this command is called or we'll lock up.
; This routine only affects d0.  All calls to this routine depend on this fact!
;==============================================================================
SCSIPutByte	PUTREG	#0,CONTROL		turn off DMA mode
		PUTREG	#WDCF_SBT!wd_TRANSFER_INFO,COMMAND  issue command
10$		btst.b	#WDCB_DBR,SASR(a4)	is data buffer ready ?
		beq.s	10$			nope, wait
		PUTREG	d0,DATA_REG		send the data byte
		rts

;==============================================================================
; SCSIGetBytes( count, buffer, CommandBlock )
; 		 d0	 a0	    a1
;
; This driver does any I/O that is less than 256 bytes using programmed I/O.
; The reason is that these are usually variable length transfers that may
; terminate on an odd boundary which would kill the DMA engine.
;
; Fetches count or less bytes and stores them in buffer.  If an interrupt is
; asserted after a byte has been transferred then an unexpected phase has
; just occured.  This routine will terminate immediately and we'll fix up
; the appropriate pointers using the residual left in the transfer count
; registers of the WDC.  If it wasn't an unexpected phase interrupt then
; something is wrong anyway.  Also exits when count bytes have been fetched.
;
; This should only be used during a Data In phase, not for messages.  If a
; parity error occurs on the data then an Initiator Detected Error message
; will be initialised and ATN asserted on the last data byte.
;
; Modified this routine to cope with arbitrary length transfers (Boyer DMA fix)
;==============================================================================
SCSIGetBytes:
		move.l	d0,d1
		swap	d1
		PUTREG	d1,TRANSFER_MSB		using transfer count register
		swap	d1
		PUTREG	d1,TRANSFER_LSB
		lsr.w	#8,d1
		PUTREG	d1,TRANSFER_MB

		move.w	st_TaskPointer(a5),d1	stash current int index
		PUTREG	#0,CONTROL		not using DMA
		PUTREG	#wd_TRANSFER_INFO,COMMAND	start the command
		move.b	#DATA_REG,st_WDCRegister(a5)
	IFD SASRW
		move.l	#DATA_REG,SASRW(a4)	select the data register
	ENDIF
	IFND SASRW
		move.b	#DATA_REG,SASR(a4)	select the data register
	ENDIF

GetBytesLoop	btst.b	#WDCB_DBR,SASR(a4)	is data buffer ready ?
		bne.s	GotAByte		yes, fetch a byte
		cmp.w	st_IntPointer(a5),d1	no byte, check for interrupt
		beq.s	GetBytesLoop		no interrupt, wait for a byte
		rts				interrupt occured, exit now

GotAByte	move.b	SCMD(a4),(a0)+		transfer the byte
		subq.l	#1,d0
		bgt.s	GetBytesLoop		go for the next byte
		btst.b	#WDCB_PE,SASR(a4)	was parity OK ?
		beq.s	NoPE			yes

; got a parity error on the last transfer, assert ATN and initialise message
		bsr	AssertATN
		move.b	#$05,cb_MessageOut(a1)	initiator detected error
		move.b	#1,cb_MsgOutLen(a1)	message is this long
		pea.l	cb_MessageOut(a1)
		move.l	(sp)+,cb_MsgOutPtr(a1)	initialise the pointer
NoPE		rts

;==============================================================================
; SCSIPutBytes( count, buffer, CommandBlock )
; 		 d0	 a0	    a1
;
; This driver does any I/O that is less than 256 bytes using programmed I/O.
; The reason is that these are usually variable length transfers that may
; terminate on an odd boundary which would kill the DMA engine.
;
; Sends count or less bytes from buffer to the target.  If an interrupt is
; asserted after a byte has been transferred then an unexpected phase has
; just occured.  This routine will terminate immediately and we'll fix up
; the appropriate pointers using the residual left in the transfer count
; registers of the WDC.  If it wasn't an unexpected phase interrupt then
; something is wrong anyway.  Also exits when count bytes have been sent
;
; This should only be used during a Data Out or Command or Message out phase.
; The target is supposed to detect any parity errors here and ask for the
; command block or data bytes again (using a Restore Pointers message)
;
; Modified this routine to cope with arbitrary length transfers (Boyer DMA fix)
;==============================================================================
SCSIPutBytes	move.l	d0,d1
		swap	d1
		PUTREG	d1,TRANSFER_MSB		using transfer count register
		swap	d1
		PUTREG	d1,TRANSFER_LSB
		lsr.w	#8,d1
		PUTREG	d1,TRANSFER_MB

		move.w	st_TaskPointer(a5),d1	stash current int index

		PUTREG	#0,CONTROL		not using DMA
		PUTREG	#wd_TRANSFER_INFO,COMMAND	start the command
		move.b	#DATA_REG,st_WDCRegister(a5)
	IFD SASRW
		move.l	#DATA_REG,SASRW(a4)	select the data register
	ENDC
	IFND SASRW
		move.b	#DATA_REG,SASR(a4)	select the data register
	ENDC

PutBytesLoop	btst.b	#WDCB_DBR,SASR(a4)	is data buffer ready ?
		bne.s	PutAByte		yes, send a byte
		cmp.w	st_IntPointer(a5),d1	has an interrupt occured
		beq.s	PutBytesLoop		nope, wait for DBR ready
		rts				yes, exit right away

PutAByte	move.b	(a0)+,SCMD(a4)		transfer the byte
		subq.l	#1,d0
		bgt.s	PutBytesLoop		go for the next byte
30$		rts

;==============================================================================
; AssertATN()
;
; Does just that.  There should be a target connected before calling though.
;==============================================================================
AssertATN	PUTREG	#wd_ASSERT_ATN,COMMAND	issue an assert ATN command
		rts

;==============================================================================
; MessageReject( unit, commandblock )
;		  a0	   a1
;
; Asserts ATN and initialises the MessageOut buffer of the given command block
; ready for the next message out request when the Message Reject will get sent.
;==============================================================================
MessageReject	bsr.s	AssertATN		yoohoo, we have a message 4 u
		move.b	#$07,cb_MessageOut(a1)	this message byte
		move.w	#1,cb_MsgOutLen(a1)	message is this long
		lea.l	cb_MessageOut(a1),a0
		move.l	a0,cb_MsgOutPtr(a1)	initialise the pointer
		rts

;==============================================================================
; FlushDMA()
;
; Ensures that the last DMA operation doesn't leave words in the FIFO.  We're
; using a trick with EOP to make this work.  All DMA is performed with EOP int
; and function disabled (because it has a 2meg limit).  Since word transfer
; count is always set to 0, the moment we enable the EOP function the DMA will
; determine that it's time to do a flush of it's FIFO.  This really only
; matters for the read direction (peripheral to memory) but this will be called
; for the write direction too.
;
; OOPS! Latest rev of the DMA chip doesn't support this function so we have to
; use Terminal Count all the time.  This means disconnect/reselect won't work!
;
; DOUBLE OOPS!  Now we have a flush DMA register (added to the B and C chips)
;==============================================================================
FlushDMA	tst.b	st_DMAGoing(a5)		was DMA started
		beq.s	NoFlushNeeded		no, so no flush needed

	IFD DMA_FLUSH_OK
		move.b	#$00,FDMA(a4)		flush DMA out
10$		btst.b	#DMAB_FIFOE,ISTR(a4)	make sure FIFO is empty
		beq.s	10$
	ENDC
		clr.b	st_DMAGoing(a5)		dma not going now
		move.b	#0,SRST(a4)		come to a full stop
NoFlushNeeded	rts

;==============================================================================
; These are the dispatch tables that are used to call the correct routines
; based on the SCSI status codes when we get a SCSI interrupt.  Each table
; corresponds to a certain status group and contains a word offset (from the
; table to the routine) for each of the error codes.  Some never occur and
; point to BadStat.  Routines marked with an asterix are target mode only and
; aren't supported by this driver.  Routines that are commented but are marked
; with an asterix are Initiator mode but just plain useless or not supported.
; The format Gn.nnnn corresponds directly to the WD33C93 interrupt status bits.
;
; This first table allows us to use the group code (upper 4 bits) to get the
; address of the relevant Group dispatch table for use with the error code.
;==============================================================================
Group_Table	DC.W	0			0
		DC.W	Group_1-Group_Table	1 Successful completion
		DC.W	Group_2-Group_Table	2 Paused or Aborted
		DC.W	0			3
		DC.W	Group_3-Group_Table	4 Terminated
		DC.W	0			5
		DC.W	0			6
		DC.W	0			7
		DC.W	Group_4-Group_Table	8 Service required

;================================================================
; group 1 status codes signify a successful completion interrupt
;================================================================
Group_1		DC.W	G1.0000-Group_1	* can never get this
		DC.W	G1.0001-Group_1	select completed
		DC.W	BadStat-Group_1
		DC.W	G1.0011-Group_1	*
		DC.W	G1.0100-Group_1	*
		DC.W	G1.0101-Group_1	translate address completed
		DC.W	G1.0110-Group_1	* select and transfer completed
		DC.W	BadStat-Group_1
; all of these imply a prior transfer completed OK. Now switch to given phase
		DC.W	G1.1000-Group_1	data out phase request
		DC.W	G1.1001-Group_1	data in phase request
		DC.W	G1.1010-Group_1	command phase request
		DC.W	G1.1011-Group_1	status phase request
		DC.W	BadStat-Group_1
		DC.W	BadStat-Group_1
		DC.W	G1.1110-Group_1 message out phase request
		DC.W	G1.1111-Group_1 message in phase request

;================================================================
; group 2 status codes signify a paused or aborted interrupt
;================================================================
Group_2		DC.W	G2.0000-Group_2	Transfer paused with ack asrtd (msg in)
		DC.W	G2.0001-Group_2	* save data ptrs during sel and trans
		DC.W	G2.0010-Group_2 * Select/reselect aborted (t/o disabled)
		DC.W	G2.0011-Group_2	*
		DC.W	G2.0100-Group_2	*
		DC.W	BadStat-Group_2
		DC.W	BadStat-Group_2
		DC.W	BadStat-Group_2
; all of these mean a prior transfer was aborted by the WDC abort command
		DC.W	G2.1000-Group_2	data out phase request
		DC.W	G2.1001-Group_2	data in phase request
		DC.W	G2.1010-Group_2	command phase request
		DC.W	G2.1011-Group_2	status phase request
		DC.W	BadStat-Group_2
		DC.W	BadStat-Group_2
		DC.W	G2.1110-Group_2 message out phase request
		DC.W	G2.1111-Group_2 message in phase request

;================================================================
; group 3 status codes signify a terminated interrupt
;================================================================
Group_3		DC.W	G3.0000-Group_3	an invalid command was issued
		DC.W	G3.0001-Group_3	unexpected disconnect caused termination
		DC.W	G3.0010-Group_3 timeout during select or reselect
		DC.W	G3.0011-Group_3	* parity error (ATN not asserted)
		DC.W	G3.0100-Group_3	* parity error (ATN asserted)
		DC.W	G3.0101-Group_3 address exceeded disk boundaries
		DC.W	G3.0110-Group_3 * wrong target device reselected
		DC.W	G3.0111-Group_3 incorrect message, status or cmd recvd
; all of these imply an unexpected information phase was requested.  Usually
; during a Transfer command when transfer didn't reach 0. switch to new phase
		DC.W	G3.1000-Group_3	data out phase request
		DC.W	G3.1001-Group_3	data in phase request
		DC.W	G3.1010-Group_3	command phase requdst
		DC.W	G3.1011-Group_3	status phase request
		DC.W	BadStat-Group_3
		DC.W	BadStat-Group_3
		DC.W	G3.1110-Group_3 message out phase request
		DC.W	G3.1111-Group_3 message in phase request

;================================================================
; Group 4 codes signify a service required interrupt
;================================================================
Group_4		DC.W	G4.0000-Group_4	WDC has been reselected
		DC.W	BadStat-Group_4
		DC.W	G4.0010-Group_4 *
		DC.W	G4.0011-Group_4	*
		DC.W	G4.0100-Group_4	* ATN has been asserted
		DC.W	G4.0101-Group_4 a disconnect has occured
		DC.W	BadStat-Group_4
		DC.W	BadStat-Group_4
; all of these imply that the REQ signal has been asserted following a connect
; (thru selection or reselection).  This tells us what phase we are expecting.
		DC.W	G4.1000-Group_4	data out phase request
		DC.W	G4.1001-Group_4	data in phase request
		DC.W	G4.1010-Group_4	command phase request
		DC.W	G4.1011-Group_4	status phase request
		DC.W	BadStat-Group_4
		DC.W	BadStat-Group_4
		DC.W	G4.1110-Group_4 message out phase request
		DC.W	G4.1111-Group_4 message in phase request

;=========================================================================
; list of routines not supported. Simply return immediately if we get them
G1.0000:
G1.0011:
G1.0100:

G2.0001:
G2.0010:
G2.0011:
G2.0100:

G3.0011:
G3.0100:
G3.0110:

G4.0010:
G4.0011:
G4.0100:
BadStat		printf <'UNSUPPORTED INTERRUPT !!!\n'>
		rts
;=========================================================================

;==============================================================================
; Bug fix, I am now using resume select and transfer to fetch status and
; command complete bytes.  When sel trans completes fetch the target status
; byte from the Target LUN register and mark the command block as complete.
;==============================================================================
G1.0110		movea.l	st_RunningUnit(a5),a0	fetch currently active unit
		move.b	#UNIT_WAITING,hu_WhatNext(a0)	all completed
		movea.l	hu_CurrentCmd(a0),a0	fetch current command block
		movea.l	cb_WStatus(a0),a1	fetch status storage pointer
		GETREG	TARGET_LUN,d0
		move.b	d0,(a1)			and save it
		rts

;==============================================================================
; A select command just completed successfully.  This will be for the first
; unit on the WaitingUnits queue.  Since it is now selected (and working OK)
; we should move it to the RunningUnit slot and wait for the Message Out REQ
; to come in because we selected with ATN asserted and want to do an Identify
; We also set up current command on this unit and initialise the SCSI pointers.
;
; This routine is also called to fake up an XT selection.  It does a lot more
; work than required for XT's but I don't want to compromise SCSI performance
;==============================================================================
FakeSelect:
G1.0001:
G1.0001entry	move.l	a2,-(sp)
		lea.l	st_WaitingUnits(a5),a0	unlink unit from waiting queue
		REMHEAD				returns ptr to SCSIQueue entry
		movea.l	d0,a2			so we adjust it to point ...
		lea.l	-hu_SCSIQueue(a2),a2	... to start of unit struct
		move.b	#UNIT_RUNNING,hu_QueueType(a2)	what queue we're on
		move.b	#UNIT_GOT_ERROR,hu_WhatNext(a2)	disconnection state
		move.l	a2,st_RunningUnit(a5)	make this the running unit
		lea.l	hu_CmdList(a2),a0	get first command on this unit
		REMHEAD
		move.l	d0,hu_CurrentCmd(a2)	save it as the current cmd

		movea.l	d0,a0			set up the SCSI working ptrs
		movea.l	cb_SCSICmd(a0),a1	from the SCSICmd struct
		move.l	a1,cb_LinkedCmd(a0)	we're using the first cmd block
		move.l	scsi_Data(a1),cb_WData(a0)
		move.l	scsi_Command(a1),cb_WCommand(a0)

		clr.l	scsi_Actual(a1)		no data transferred yet
		clr.w	scsi_CmdActual(a1)	and no command either

		lea.l	scsi_Status(a1),a1	set up status area address
		move.l	a1,cb_WStatus(a0)	only one status byte
		move.l	cb_WData(a0),cb_SData(a0)	copy working ptrs...
		move.l	cb_WCommand(a0),cb_SCommand(a0)	...to saved pointers
		move.l	cb_WStatus(a0),cb_SStatus(a0)

; We did a select with ATN so the target is going to request a message out
; next.  We're going to send an identify with the reselection bit set unless
; the flags on this unit tell us not to support reselection.  This is usually
; the case if there is only a single disk unit because there's no need for all
; the disconnect/reconnect overhead if no other units need servicing.  The
; message system is effectively initialised here but is maintained by the
; message in and out handlers on a unit by unit basis.  This is unnescessary
; for the XT unit but I've left it in because I don't want to compromise SCSI
		lea.l	cb_MessageIn(a0),a1	initialise msg in for 1 byte
		move.l	a1,cb_MsgInPtr(a0)
		move.w	#1,cb_MsgInLeft(a0)
		clr.w	cb_MsgInLen(a0)		no message received yet

		lea.l	cb_MessageOut(a0),a1	and msg out is 1 byte too
		move.l	a1,cb_MsgOutPtr(a0)
		move.w	#1,cb_MsgOutLen(a0)	we're sending a 1 byte message

		move.b	#$80,d0			IDENTIFY
		or.b	hu_LUN(a2),d0		plus the logical unit number

	IFD DMA_FLUSH_OK
		tst.b	hu_IsDisk(a2)
		beq.s	CanDisconnect		always enable for non-disks
		btst.b	#RDBFB_NORESELECT,hu_Flags(a2) reselection disabled?
		bne.s	NoDisconnect		yes, don`t enable it
CanDisconnect	bset.b	#6,d0			enable reselection
	ENDC

NoDisconnect	move.b	d0,(a1)			save the message byte
		movea.l	(sp)+,a2		we'll get msg or cmd phase next
		rts

;==============================================================================
; A translate address completed successfully.  Now it's time to send XT cmds.
; This interrupt only happens when we used the translate address function of
; the WDC to figure out which head/track/cylinder a logical offset was on.
;==============================================================================
G1.0101:
	IFD XT_SUPPORTED
		bra	XTSendCommand
	ENDC

;==============================================================================
; A previous transfer command completed OK.  Target is requesting data out now.
; All even length transfers are performed using DMA mode and the transfer count
; register (so we know how much was done if the command is aborted).  If an odd
; byte is left over at the end of the DMA transfers we will use the single byte
; transfer mode and jam the data byte directly into the WDC data register. This
; is nescessary because the DMA engine is word oriented.
;==============================================================================
G1.1000		clr.b	st_LastPhase(a5)	we're in a data phase
		movea.l	st_RunningUnit(a5),a0	fetch currently active unit
		movea.l	hu_CurrentCmd(a0),a0	fetch current command block
		movea.l	cb_LinkedCmd(a0),a1	and corresponding SCSICmd

		move.l	scsi_Length(a1),d0	get length left to do
		sub.l	scsi_Actual(a1),d0	by subtracting what's done
		bne.s	NotPaddedOut

; twit caller didn't expect this many bytes to go out, transmit zeros
	printf <'SCSI: transferring %ld pad bytes out\n'>,d0
	printf <'scsi_Length = %ld\n'>,scsi_Length(a1)
	printf <'scsi_Actual = %ld\n'>,scsi_Actual(a1)
		move.b	#PAD_LAST,st_LastPhase(a5)	what we're doing
		move.b	#$00,d0
		bra	SCSIPutByte
		rts

; there's some data left to transfer, do some tests to see what mode we use
NotPaddedOut	btst.l	#0,d0			is this an odd length xfer ?
		beq.s	20$			nope, do normal DMA xfer
		subq.l	#1,d0			get rid of the odd byte
		bne.s	20$			some work left to do

; We're left with a single byte to send so we'll do it in single byte mode.
; Also comes back to here if the source data address is on an odd boundary.
; The WDC has interrupted us because a REQ is pending so we should send the
; data byte through the WDC data register right away using a TRANSFER_INFO
; command with the single byte transfer bit set to 1.
10$		addq.l	#1,scsi_Actual(a1)	one more to actual count
		movea.l	cb_WData(a0),a1		get data address
		addq.l	#1,cb_WData(a0)		and bump the data pointer
		move.b	(a1),d0			fetch byte to send
		bra	SCSIPutByte		and send it

; before doing DMA make sure address and length is OK (may translate virtual)
20$		move.l	cb_WData(a0),d1		get source data pointer
		bsr	CheckPreDMA
		tst.b	cb_Programmed(a0)	programmed I/O needed ?
		beq.s	30$			nope, DMA is OK

; something exceeds the capabilities of the DMAC, so we're using programmed I/O
		add.l	d0,cb_WData(a0)		bump the ptr for next time
		add.l	d0,scsi_Actual(a1)	and update the actual count
		movea.l	a0,a1			command block to a1
		movea.l	d1,a0			initialise ptr for pgmd I/O
		bra	SCSIPutBytes		and send the bytes

; we have some data left to transfer in DMA mode so set up the DMA chip for
; output to a SCSI unit with interrupts enabled (we only see SCSI interrupts)
30$	IFD DMA_FLUSH_OK
	move.b	#DMAF_PMODE!DMAF_INTENA!DMAF_DDIR,CNTR(a4)
	ENDC DMA_FLUSH_OK

; must use terminal count if the DMA doesn't support the Flush trickery
	IFND DMA_FLUSH_OK
	move.b	#DMAF_TCE!DMAF_PMODE!DMAF_INTENA!DMAF_DDIR!DMAF_XTEOP,CNTR(a4)
	ENDC DMA_FLUSH_OK
		bra.s	DoDataXFER		common code (at end of G1.1001)

;==============================================================================
; A previous transfer command completed OK.  Target is requesting data in now.
; All even length transfers are performed using DMA mode and the transfer count
; register (so we know how much was done if the command is aborted).  If an odd
; byte is left over at the end of the DMA transfers we will use the single byte
; transfer mode and get the data byte directly from the WDC data register. This
; is nescessary because the DMA engine is word oriented.
;==============================================================================
G1.1001		clr.b	st_LastPhase(a5)	we're in a data phase
		movea.l	st_RunningUnit(a5),a0	fetch currently active unit
		movea.l	hu_CurrentCmd(a0),a0	fetch current command block
		movea.l	cb_LinkedCmd(a0),a1	and corresponding SCSICmd

		move.l	scsi_Length(a1),d0	get length left to read
		sub.l	scsi_Actual(a1),d0	by subtracting what's done
		bne.s	NotPaddedIn		some data left to go

; twit caller didn't expect this many bytes to come in, just eat them up
	printf <'SCSI: transferring pad %ld bytes in\n'>,d0
	printf <'scsi_Length = %ld\n'>,scsi_Length(a1)
	printf <'scsi_Actual = %ld\n'>,scsi_Actual(a1)
		move.b	#PAD_LAST,st_LastPhase(a5)	what we're doing
		bra	SCSIGetByte		fetch a byte (and trash it)

; there's some data left to transfer, do some tests to see what mode we use
; V37, if CachePreDMA caused us to split a block transfer across different
; addresses we must make sure that DMA is flushed and stopped before attempting
; to start it all up again.
NotPaddedIn	btst.l	#0,d0			is this an odd length xfer ?
		beq.s	20$			nope, do normal DMA xfer
		subq.l	#1,d0			get rid of the odd byte
		bne.s	20$			some work left to do

; We're left with a single byte to read so we'll do it in single byte mode.
; Also comes back to here if the destination address is on an odd boundary.
; The WDC has interrupted us because a REQ is pending so we should read the
; data byte through the WDC data register right away using a TRANSFER_INFO
; command with the single byte transfer bit set to 1.
10$		bsr	SCSIGetByte		fetch the byte of data
		tst.w	d1
		beq.s	15$			didn't get the data
		addq.l	#1,scsi_Actual(a1)	one more to actual count
		movea.l	cb_WData(a0),a1		get data address
		move.b	d0,(a1)+		store and bump pointer
		move.l	a1,cb_WData(a0)		save the bumped pointer
15$		rts

; before doing DMA make sure address and length is OK (may translate virtual)
20$		move.l	cb_WData(a0),d1		get destination address
		bsr	CheckPreDMA
		tst.b	cb_Programmed(a0)	programmed I/O needed ?
		beq.s	30$			nope, DMA is OK

; something exceeds the capabilities of the DMAC, so we're using programmed I/O
		add.l	d0,cb_WData(a0)		bump the ptr for next time
		add.l	d0,scsi_Actual(a1)	and update the actual count
		movea.l	a0,a1			command block to a1
		movea.l	d1,a0			initialise ptr for pgmd I/O
		bra	SCSIGetBytes		and get the bytes

; we have some data left to transfer in DMA mode so set up the DMA chip for
; input from a SCSI unit with interrupts enabled (we only see SCSI interrupts)
30$	IFD DMA_FLUSH_OK
		move.b	#DMAF_PMODE!DMAF_INTENA,CNTR(a4)
	ENDC DMA_FLUSH_OK

; must use terminal count if the DMA doesn't support the Flush trickery
	IFND DMA_FLUSH_OK
		move.b	#DMAF_TCE!DMAF_PMODE!DMAF_INTENA!DMAF_XTEOP,CNTR(a4)
	ENDC !DMA_FLUSH_OK

; this code is common to both read and write, so we call it from the write code
DoDataXFER	move.l	d1,SACH(a4)		set source data pointer
		add.l	d0,cb_WData(a0)		bump the ptr for next time
		add.l	d0,scsi_Actual(a1)	and update the actual count

; Yet another DMA flush fix.  It seems that we must enable Terminal Count for
; the flush logic to work properly.  I'm setting WTC to the maximum value (2M)
;	IFD DMA_FLUSH_OK
;		move.l	#$ffffff,WTCH(a4)
;	ENDC DMA_FLUSH_OK

; if we're using terminal count then we're required to use WTCH too
	IFND DMA_FLUSH_OK
		move.l	d0,d1			stick word count in WTCH
		lsr.l	#1,d1
		move.l	d1,WTCH(a4)
	ENDC !DMA_FLUSH_OK

		move.b	#1,st_DMAGoing(a5)	flag that DMA is running
		move.b	#0,SDMA(a4)		DMA is now waiting to go
	IFD SASRW
		move.l	#TRANSFER_MSB,SASR(a4) set transfer count in WDC
	ENDC
	IFND SASRW
		move.b	#TRANSFER_MSB,SASR(a4) set transfer count in WDC
	ENDC
		move.l	d0,-(sp)		set up transfer count in WDC
		PUTREG	1(sp),TRANSFER_MSB
		PUTREG	2(sp),TRANSFER_MB
		PUTREG	3(sp),TRANSFER_LSB
		addq.l	#4,sp
		PUTREG	#WDCF_DMA,CONTROL	using DMA mode for data xfer
		PUTREG	#wd_TRANSFER_INFO,COMMAND	issue transfer command
		rts

;==============================================================================
; The target is requesting the next byte of the command block (a previous
; transfer command completed OK).  We can't use DMA for this so I'm going to
; send multiple bytes using the SCSIPutBytes routine.  CmdActual is fixed up
; if we get an unexpected phase interrupt after sending this command block.
;==============================================================================
G1.1010		move.b	#1,st_LastPhase(a5)	we're in command phase
		movea.l	st_RunningUnit(a5),a0	fetch currently active unit
		movea.l	hu_CurrentCmd(a0),a1
		movea.l	cb_LinkedCmd(a1),a0	and the direct scsi_Cmd
		move.w	scsi_CmdLength(a0),d0	get length to send
		move.w	d0,scsi_CmdActual(a0)	make it actual (may be altered)
		ext.l	d0
		movea.l	cb_WCommand(a1),a0	get command block pointer
		bra	SCSIPutBytes		and send the bytes

;==============================================================================
; The target has entered the status phase and wants to send us a status byte
; for the command just completed.  Since we're only ever expecting a single
; status byte we'll store the data at cb_WStatus but never bump the pointer.
; We'll be using programmed I/O to fetch this byte over the SCSI bus too.
;==============================================================================
G1.1011:
; New addition:  The WD33C93A has a problem where the interrupt for the
; command complete message comes in before the status byte has been fetched.
; There doesn't seem to be a workaround so I'm changing this to do a
; Resume Select and Transfer which will transfer the status byte and the
; command complete message and give a Select and Transfer complete interrupt
; instead.
		PUTREG	#$46,COMMAND_PHASE	status fetch imminent
		PUTREG	#wd_SELECT_TRANS,COMMAND	restart sel and trans
		rts
;		movea.l	st_RunningUnit(a5),a0	fetch currently active unit
;		movea.l	hu_CurrentCmd(a0),a0	fetch current command block
;		movea.l	cb_WStatus(a0),a1	fetch status storage pointer
;		bsr	SCSIGetByte		transfer the byte
; no need to test here (at least I have never had a bad command int on status)
;		tst.w	d1			did we get it ?
;		beq.s	10$			nope
;		move.b	d0,(a1)			and save it
;10$		rts

;==============================================================================
; A previous transfer completed OK and the target wants us to send it a message
; The message to be sent is held in cb_MessageOut of the current command while
; cb_MsgOutLen tells us how much is to be sent.  We have to use SCSIPutBytes
; because all of the message bytes must be contained within the same assertion
; of ATN.  Transferring single bytes would only have ATN asserted on the first
; byte of an extended message.  We don't affect the message out pointers.
;==============================================================================
G1.1110:
		movea.l	st_RunningUnit(a5),a0	fetch currently active unit
		movea.l	hu_CurrentCmd(a0),a1	fetch current command block
		movea.l	cb_MsgOutPtr(a1),a0	fetch next message byte ptr
		move.w	cb_MsgOutLen(a1),d0	how big is the message ?
		cmpi.w	#1,d0			is it single byte ?
		beq.s	NotSync			yep, do it

; if this was a multi byte message out, then it's a sync transfer reply and we
; should use the negotiated value (probably less than the targets) right after
; we have sent the message.  Target will immediately switch to new values.
	printf <'SCSI: sent sync xfer msg\n'>
;	moveq.l	#-1,d1
;10$	nop			this was for Micropolis, not needed now
;	nop
;	dbra	d1,10$
		ext.l	d0			***fix for new SCSIPutBytes
		bsr	SCSIPutBytes		no, use multi byte xfer
		movea.l	st_RunningUnit(a5),a0
		moveq.l	#0,d0
		move.b	hu_Unit(a0),d0
		move.b	st_SyncValues(a5,d0.w),d0
		PUTREG	d0,SYNC_TRANSFER	set up WDC sync value
		rts

NotSync		move.b	(a0)+,d0		fetch the byte
		bra	SCSIPutByte		send it to the target

;==============================================================================
; A previous transfer completed OK, the target is now requesting a message in
; phase.  This is a biggie.  We support quite a lot of the SCSI messages so
; we use a small dispatch table to do the appropriate things.  Some of these
; messages would not be received after a transfer command (Identify for one)
; but we call this code for all the message in phases so we include them all
; here.  Have to be careful here because if we are about to receive an Identify
; then the unit in the RunningUnit slot could be the wrong one and may need
; to be exchanged for a different logical unit (though physical is the same)
;
; When this routine has completed, we are guaranteed to get a transfer paused
; with ACK asserted int next.  Since all the screwball stuff is handled here
; there will be nothing to do other than negating ACK.  A parity check is also
; performed and if a message byte is bad, this fact is flagged and all other
; message bytes are just wasted until we perform a message out phase. (We'll
; have asserted ATN when we got the bad byte).
;
; Extended messages are handled a little wierdly. When we get the first byte
; of an extended msg, we'll detect that not all bytes have come in (cb_MsgInLen
; will still be set to 1) so we'll set cb_MsgInLeft to -1 which makes the top
; of this routine interpret the next message byte as the number to follow.
;
; All message handlers are entered with a0=unit, a1=command.  Identify is
; additionally entered with d0=Identify message
;==============================================================================
G1.1111:
		movea.l	st_RunningUnit(a5),a0	fetch currently active unit
		movea.l	hu_CurrentCmd(a0),a1	fetch current command block
		bsr	SCSIGetByte		fetch the message byte
		tst.w	d1			did it come in OK ?
		bne.s	5$			yep
		rts				got a bad command interrupt

5$		tst.b	cb_MsgParity(a1)	was parity already bad ?
		beq.s	10$			no, check parity on this byte
		rts				already bad, exit immediately

10$		btst.b	#WDCB_PE,SASR(a4)	was there a msg parity error
		beq.s	ParityOK		nope, carry on as normal

; a message parity error occured, flag it and assert ATN to let the target
; know which byte the error occured on.  We'll also set up the MsgOutPtr on
; the current command block ready to send a Message Parity Error message
; We also re-initialise the Message In buffers for the next message.

		move.b	#-1,cb_MsgParity(a1)	flag that an error occured
		tst.b	st_ParityCheck(a5)	are we checking parity
		bne.s	ParityOK		nope, ignore it

		bsr	AssertATN		and assert ATN on this byte
		lea.l	cb_MessageOut(a1),a0	initialise message out
		move.b	#$09,(a0)		to send Message Parity Error
		move.l	a0,cb_MsgOutPtr(a1)
		move.w	#1,cb_MsgOutLen(a1)
		bra.s	ResetMsgIn		reset message in pointers too

; didn't get any errors so store the message at the next point in msg buffer
ParityOK	movea.l	cb_MsgInPtr(a1),a0	fetch next message pointer
		move.b	d0,(a0)+		store in message area
		move.l	a0,cb_MsgInPtr(a1)	save the new message ptr
		addq.w	#1,cb_MsgInLen(a1)	and bump the current msg len
		tst.w	cb_MsgInLeft(a1)	are we fetching Extended msgs ?
		bpl.s	10$			no, do the normal stuff

; we just received the length byte of an extended message.  Use it to set up
; a new value for MsgInLeft so we expect the right amount of message bytes
		ext.w	d0
		move.w	d0,cb_MsgInLeft(a1)	how many more we're expecting
		rts

; we're not doing anything special so decrement cb_MsgInLeft and handle if 0
10$		subq.w	#1,cb_MsgInLeft(a1)	one less byte to expect
		beq.s	30$			got all bytes we were expecting
		rts				we need more bytes

; we've received all message bytes for this message in phase.  call a handler.
30$		movea.l	st_RunningUnit(a5),a0	get unit ptr for msg handlers
		clr.w	d0			use whole word for indexing
		move.b	cb_MessageIn(a1),d0	fetch first message byte
		bpl.s	40$			its a normal message type
		bsr.s	IdentifyIn		identify after reselection
		bra.s	MsgInDone		reset for next message in

; the message we are about to fetch is definitely for the currently active unit
40$		cmpi.b	#$0c,d0			is message in range
		ble.s	50$			yes, dispatch to msg handler
		bsr	MessageReject		no, arbitrarily reject this msg
		movea.l	st_RunningUnit(a5),a0	in case routine clobbered ptrs
		movea.l	hu_CurrentCmd(a0),a1
		bra.s	MsgInDone

; the message was for the current unit and was a valid message byte
50$		lsl.w	#1,d0			accessing table of word offsets
		move.w	Msgs(pc,d0.w),d0
		jsr	Msgs(pc,d0.w)		call the right routine
		movea.l	st_RunningUnit(a5),a0	in case routine clobbered ptrs
		movea.l	hu_CurrentCmd(a0),a1
		tst.w	cb_MsgInLeft(a1)	make sure not an extended msg
		bpl.s	MsgInDone		it wasn't
		rts				extended, more bytes to come

; This message in phase has been completed, reset pointers ready for the next
MsgInDone	clr.b	cb_MsgParity(a1)	no parity errors now
ResetMsgIn	lea.l	cb_MessageIn(a1),a0	reset the msg in buffers
		move.l	a0,cb_MsgInPtr(a1)
		move.w	#1,cb_MsgInLeft(a1)
		clr.w	cb_MsgInLen(a1)
		rts				can't do anything else now

; simple word offset table to call the correct routine based on a message #
Msgs		DC.W	CmdComplete-Msgs	0 command complete
		DC.W	Extended-Msgs		1 extended message follows
		DC.W	SaveData-Msgs		2 save data pointer
		DC.W	RestorePtrs-Msgs	3 restore pointers
		DC.W	Disconnect-Msgs		4 disconnect
		DC.W	MessageReject-Msgs	5 initiator detected error
		DC.W	MessageReject-Msgs	6 abort
		DC.W	Rejected-Msgs		7 message reject
		DC.W	MessageReject-Msgs	8 no operation
		DC.W	MessageReject-Msgs	9 message parity error
		DC.W	LCmdComplete-Msgs	a linked command complete
		DC.W	LCmdComplete-Msgs	b linked cmd complete with flag
		DC.W	MessageReject-Msgs	c bus device reset


;------------------------------------------------------------------------------
; IdentifyIn( message, unit, command )
;		d0	a0	a1
;
; a target re-selected us and we marked the first physical unit we could find
; that matched the source ID on reselection.  It's possible that there are
; many logical units at that physical address so the Identify message we just
; received will be used to ensure that we really picked up the correct unit
; to be made active.  If we were wrong, then we'll fix the mistake here.
;------------------------------------------------------------------------------
IdentifyIn	andi.b	#$07,d0			mask off LUN bits in message
		cmp.b	hu_LUN(a0),d0		did we get the correct one ?
		bne.s	10$			no, find the one we want
		rts				unit was already set up

; OOPS! we made the wrong unit active when we got an earlier re-selection so
; attempt to find the logical unit that the target is talking about.  If we
; find it then the current RunningUnit will go back on the WorkingUnits queue
; and the one we found will be made the new RunningUnit.  If we don't find
; it then the target screwed up on the identify so we'll reject the message.
10$		move.b	d0,d1			stash the LUN
		move.b	hu_Unit(a0),d0		physical was correct
		bsr	FindWorking		see if we can find this unit
	printf <'SCSI: adjusted to unit at 0x%lx\n'>,d0
		tst.l	d0
		beq	MessageReject		didn't find it, reject Identify

; we found the unit we really wanted so make that one the running unit instead
; The incorrectly chosen unit must go back on the WorkingUnits queue because
; that's where we got it from in the first place.  Another reselect will occur
; for that unit when it's current command has completed.
		move.l	d0,-(sp)		stash the one we found
		movea.l	st_RunningUnit(a5),a1	put old one on WorkingUnits
		move.b	#UNIT_WORKING,hu_QueueType(a1)
		lea.l	hu_SCSIQueue(a1),a1	SCSI list node for linkage
		lea.l	st_WorkingUnits(a5),a0	put back on WorkingUnits queue
		ADDHEAD				will probably reselect quickly

		movea.l	(sp)+,a0		get the new unit
		move.l	a0,st_RunningUnit(a5)	it's running (connected) now
		move.b	#UNIT_RUNNING,hu_QueueType(a0)
		move.b	#UNIT_GOT_ERROR,hu_WhatNext(a0)	no disconnect w/o msg

		movea.l	hu_CurrentCmd(a0),a1	restore working ptrs
		move.l	cb_SData(a1),cb_WData(a1)
		move.l	cb_SCommand(a1),cb_WCommand(a1)

		lea.l	hu_SCSIQueue(a0),a1	remove from WorkingUnits queue
		REMOVE
		bra	SetTransfer		set up transfer mode for unit

;------------------------------------------------------------------------------
; "Command complete"  we'll accept this message and mark the unit for moving
; onto the WaitingUnits queue when the disconnect occurs
;------------------------------------------------------------------------------
CmdComplete	move.b	#UNIT_WAITING,hu_WhatNext(a0)	all completed
		rts

;------------------------------------------------------------------------------
; "Extended message follows"  We check cb_MsgInLen to see if we only got the
; first byte of this message.  If we did, then we'll set cb_MsgInLeft to -1
; which forces the Message In handler to interpret the next message byte as a
; count of the remaining message bytes to be sent.  If cb_MsgInLen is greater
; than 1 then we've already done this and have a complete extended message in
; cb_MessageIn.  The only extended message we support is synchronous transfer.
;------------------------------------------------------------------------------
Extended	move.w	cb_MsgInLen(a1),d0	how much message is there ?
		cmpi.w	#1,d0
		bne.s	GotExtended		all of it
		move.w	#-1,cb_MsgInLeft(a1)	flag that next byte is a count
		rts				and return to main routine

GotExtended	move.b	cb_MessageIn+2(a1),d0	get extended message code
		cmpi.b	#1,d0			only support sync transfer msgs
		bne	MessageReject		assert ATN and reject this msg

; we got a Synchronous Transfer Request from the target.  If they're good we'll
; use the values supplied by the target which will remain in effect for this
; unit until BUS DEVICE RESET or a hard reset condition occurs.  In this setup
; a SCSI bus cycle time is 100-150ns, we'll assume 100 so we err on the safe
; side.  Allowable WDC transfer periods using DMA are:-
;
; 3=200ns   4=300ns   5 = 400ns   6=500ns   7=600ns
;
; REQ/ACK offset can range from 0 (asynchronous) to 4
	printf <'SCSI: got sync xfer req (0x%lx)\n'>,cb_MessageIn+2(a1)

;		moveq.l	#0,d0
;		move.b	cb_MessageIn+3(a1),d0	get transfer period/4
;		cmpi.b	#$46,d0			can we handle it ?
;		blt.s	SyncResponse		nope, use our values
;		lsl.w	#2,d0
;		divu.w	#140,d0			our clock is 7.5 MHz
;		addq.w	#1,d0			d0 holds WDC TP0-TP2
;		lsl.b	#4,d0			move to upper nybble
;		cmpi.b	#4,cb_MessageIn+4(a1)	is offset OK ?
;		bgt.s	SyncResponse

;		or.b	cb_MessageIn+4(a1),d0	merge in REQ/ACK offset
;		move.b	d0,hu_SyncValue(a0)	and save in the unit
;		move.b	#1,hu_SyncDone(a0)	flag sync negotiated
;	printf <'SCSI: hu_SyncValue = 0x%x\n'>,d0
;		PUTREG	d0,SYNC_TRANSFER	set up WDC sync value
;		rts

SyncResponse	moveq.l	#0,d0
		move.b	hu_Unit(a0),d0
		move.b	#$34,st_SyncValues(a5,d0.w)
	moveq.l	#-1,d0
10$	nop
	nop
	dbra	d0,10$
		bsr	AssertATN		we have a message pending
	moveq.l	#-1,d0
20$	nop
	nop
	dbra	d0,20$
		lea.l	cb_MessageOut(a1),a0	initialise message out
		move.l	a0,cb_MsgOutPtr(a1)	where msg is
		move.w	#5,cb_MsgOutLen(a1)	how long message is
		move.l	#$01030146,(a0)+	transfer period = 280ns
		move.b	#4,(a0)			req/ack offset = 4
		rts

;------------------------------------------------------------------------------
; "Save data qointer" we'll accept this message and move the working data
; pointer into the saved data pointer in the current command block.  Does
; not affect hu_WhatNext as this is driven by the disconnect message.
;------------------------------------------------------------------------------
SaveData	move.l	cb_WData(a1),cb_SData(a1)	save data pointer
		rts

;------------------------------------------------------------------------------
; "Restore pointers"  restore previously saved pointers and accept the message
; Since we've already bumped the actual transfer count, we must adjust it back
; by the difference between the saved and working data pointers.
;------------------------------------------------------------------------------
RestorePtrs	move.l	cb_WData(a1),d0		how much difference
		sub.l	cb_SData(a1),d0

		movea.l	cb_SCSICmd(a1),a0	subtract difference from...
		sub.l	d0,scsi_Actual(a0)	..amount already done

		move.l	cb_SData(a1),cb_WData(a1)
		move.l	cb_SCommand(a1),cb_WCommand(a1)
		move.l	cb_SStatus(a1),cb_WStatus(a1)
		rts

;------------------------------------------------------------------------------
; "Disconnect"  accept this message and mark the unit for moving onto the
; WorkingUnits queue when a disconnect occurs.  We'll need at least one more
; reselection to complete the current command.  If the unit has already been
; marked with ht_WhatNext==UNIT_WAITING then we got a command complete already
; so this disconnect message shouldn't be here, must check if this is true.
;------------------------------------------------------------------------------
Disconnect	printf <'Got a disconnect message\n'>
		move.b	#UNIT_WORKING,hu_WhatNext(a0)
		rts

;------------------------------------------------------------------------------
; "Message reject" the target rejected last message that we sent.  There aren't
; many messages we send that could be rejected, if it's not a Synchronous
; Transfer request that was rejected, we'll just ignore the condition anyway.
;------------------------------------------------------------------------------
Rejected	move.b	cb_MessageOut(a1),d0	what message was rejected ?
		bmi.s	WhoCares		get check condition if identify

		cmpi.b	#1,d0			was it our extended msg
		bne.s	WhoCares		nope, so ignore rejection
		move.b	cb_MessageOut+2(a1),d0	make sure correct extended msg
		cmpi.b	#1,d0			sync transfer request ?
		bne.s	WhoCares		nope, ignore rejection

; we sent a sync transfer request but the unit couldn't do it or support it so
; default to asynchronous mode with a REQ/ACK offset of 250-300ns
		moveq.l	#0,d0
		move.b	#$40,st_SyncValues(a5,d0.w)
		PUTREG	#$40,SYNC_TRANSFER	and set up WDC with this value
WhoCares	rts

;------------------------------------------------------------------------------
; "Linked command complete (might be with flag too)"
;
; A linked command has completed but the unit has remained selected ready for
; us to send the next command block.  Initialise everything ready for the next
; command phase, (ie. link to the next SCSIDirect command block).  This will
; have no effect on the hu_WhatNext flag (unit should remain connected anyway).
;
;	****** may have to handle the case of no linked command ******
;------------------------------------------------------------------------------
LCmdComplete	movea.l	cb_LinkedCmd(a1),a0	fetch current SCSI command
		movea.l	scsi_NextLinked(a0),a0	and link to the next one
		move.l	a0,cb_LinkedCmd(a1)	this is the one we're using now

		move.l	scsi_Data(a0),cb_WData(a1)
		move.l	scsi_Command(a0),cb_WCommand(a1)
		clr.l	scsi_Actual(a0)		no data transferred yet
		clr.w	scsi_CmdActual(a0)	and no command either
		lea.l	scsi_Status(a0),a0	set up status area address
		move.l	a0,cb_WStatus(a1)	only one status byte
		move.l	cb_WData(a1),cb_SData(a1)	copy working ptrs...
		move.l	cb_WCommand(a1),cb_SCommand(a1)	...to saved pointers
		move.l	cb_WStatus(a1),cb_SStatus(a1)
		rts


;==============================================================================
; A transfer command (message in phase) paused with ack asserted.  This really
; means that we just fetched a message byte for the currently active unit.  If
; anything weird happened (Parity error or message was rejected) ATN will have
; already been asserted in the Message In handler.  All we have to do here is
; negate ACK to say we received the message byte and are ready for more stuff.
;==============================================================================
G2.0000		PUTREG	#wd_NEGATE_ACK,COMMAND	issue a negate ack command
		rts

;==============================================================================
; All of these codes are for an aborted transfer (we used the abort command).
; Since I'm not doing this yet there's no need to support the interrupts either
;==============================================================================
G2.1000:
G2.1001:
G2.1010:
G2.1011:
G2.1110:
G2.1111:

;==============================================================================
; Invalid command was issued.  Gimme a break! I'll never issue bad commands :-)
;==============================================================================
G3.0000	printf <'BAD COMMAND INTERRUPT\n'>
		rts

;==============================================================================
; An unexpected disconnect caused the last command to terminate.  This will be
; for the current RunningUnit and its hu_WhatNext flag will probably be set to
; UNIT_GOT_ERROR by now (since a disconnect without a message wasn't expected)
; Therefore, it's reasonable to treat this the same as a normal disconnection.
;==============================================================================
G3.0001		bra	G4.0101			handle as a normal disconnect

;==============================================================================
; A timeout occured during select or re-select.  Since we're an initiator and
; we're using the timeout feature of the WDC, we can assume this to mean that
; a timeout occured on a selection.  The unit that this pertains to has not
; yet been moved off the WaitingUnits queue (it was waiting to be selected when
; this interrupt came in).  To make things easy, we'll move the unit off the
; WaitingUnits queue and into the Running slot and set hu_WhatNext to
; UNIT_TIMED_OUT.  We'll then call G4.0101 (disconnect occured) directly so
; it sends back the appropriate message to the original caller.
;
; This is also called by the XTSelect routine if it found there wasn't a unit
;==============================================================================
FakeTimeout:
G3.0010		bsr	FakeSelect		make like select worked
		movea.l	st_RunningUnit(a5),a0	fetch the unit pointer
		move.b	#UNIT_TIMED_OUT,hu_WhatNext(a0)	mark as timed out
		bra	FakeDisconnect		and handle like a disconnect

;==============================================================================
; The logical address exceeded the disk boundaries.  Used by XT devices only.
;==============================================================================
G3.0101:
	IFD XT_SUPPORTED
		bra	XTAddressBad		abort current XT command
	ENDC

;==============================================================================
; An incorrect message/status or command byte was received (target mode only??)
;==============================================================================
G3.0111		rts				dunno how to handle this anyway

;==============================================================================
; The rest of these G3.xxxx codes mean that a data transfer with transfer count
; set to non-zero and SBT=FALSE was terminated because a different phase was
; requested by the target before the transfer count reached zero.  In all cases
; we just subtract the number left in the WDC transfer count register from the
; actual count in the SCSIDirect command and from the working address and call
; the correct G1.xxxx equivalent to carry on doing the next transfer phase.
;
; One case where an error may occur is if we just did the Command out phase.
; we were expecting to send more bytes than the target requested so we have to
; fix up scsi_CmdActual instead of the data pointers.  hu_LastPhase flags which
;==============================================================================
G3.1000		bsr.s	G3.PhaseChange
		bra	G1.1000			go do data out phase

G3.1001		bsr.s	G3.PhaseChange
		bra	G1.1001			go do data in phase

G3.1010		bsr.s	G3.PhaseChange
		bra	G1.1010			go do command phase

G3.1011		bsr.s	G3.PhaseChange
		bra	G1.1011			go do status phase

G3.1110		bsr.s	G3.PhaseChange
		bra	G1.1110			go do message in phase

G3.1111		bsr.s	G3.PhaseChange
		bra	G1.1111			go do message out phase

G3.PhaseChange	bsr	FlushDMA		make sure DMA finished
		moveq.l	#0,d0			get bytes left over
		GETREG	TRANSFER_MSB,d0		get most significant byte
		swap	d0			move to upper word
		GETREG	TRANSFER_MB,d0		get middle byte
		lsl.w	#8,d0			move to top of lower word
		GETREG	TRANSFER_LSB,d0		get least significant byte
		movea.l	st_RunningUnit(a5),a0	get the current unit
		movea.l	hu_CurrentCmd(a0),a0	fetch current command block
		movea.l	cb_LinkedCmd(a0),a1	and corresponding SCSICmd

		move.b	st_LastPhase(a5),d1	what phase was terminated ?
		beq.s	10$			it was a data phase

		cmpi.b	#PAD_LAST,d1		were we eating bytes ?
		beq.s	20$			yep, so nothing to adjust

		sub.w	d0,scsi_CmdActual(a1)	we were transferring cmd bytes
		bra.s	20$

10$		sub.l	d0,scsi_Actual(a1)	subtract leftover from actual
		sub.l	d0,cb_WData(a0)		and adjust DMA address too

20$		PUTREG	#0,TRANSFER_MSB		clear transfer count to be safe
		PUTREG	#0,TRANSFER_MB
		PUTREG	#0,TRANSFER_LSB
		rts

;==============================================================================
; We've been reselected by some unit.  Search the WorkingUnits queue and find
; the first physical that matches the WDC source ID register.  This may be the
; wrong logical unit but we'll fix this up when an identify message comes in
; and tells us which LUN we really wanted.  Normally, LUNS are 0 anyway.  The
; unit found is put into the RunningUnit slot in the hopes that we were right.
;==============================================================================
G4.0000:
retrysiv	GETREG	SOURCE_ID,d0
		btst.b	#WDCB_SIV,d0		source ID better be valid
		beq.s	SIVBad0			what the hell do we do now ?
		andi.b	#$07,d0			mask off other garbage
		moveq.l	#-1,d1			match first/any LUN
		bsr	FindWorking		see if we can find this unit
		move.l	d0,st_RunningUnit(a5)	it's running (connected) now
		beq.s	SIVBad1			failed, SIV must be bad
		movea.l	d0,a0
		move.b	#UNIT_RUNNING,hu_QueueType(a0)
		move.b	#UNIT_GOT_ERROR,hu_WhatNext(a0)	no disconnect w/o msg

		movea.l	hu_CurrentCmd(a0),a1	restore working ptrs
		move.l	cb_SData(a1),cb_WData(a1)
		move.l	cb_SCommand(a1),cb_WCommand(a1)

		lea.l	hu_SCSIQueue(a0),a1	remove from WorkingUnits queue
		REMOVE
		rts

; These are in here to handle a small bug in the WDC SCSI chip.  Sometimes
; the source id bit will not get set, a small delay seems to fix things up
SIVBad0		moveq.l	#-1,d0
10$		dbra	d0,10$
		bra.s	retrysiv

SIVBad1		moveq.l	#-1,d0
10$		dbra	d0,10$
		bra	retrysiv

;==============================================================================
; A disconnect has occurred.  This is for the unit in the RunningUnit slot (it
; had better be anyway).  Based on the contents of hu_WhatNext we'll take the
; following actions:-
;
; UNIT_TIMED_OUT	we got a timeout during a selection so the unit was
;			made active anyway just so we could return the cmd
;			with a seltimeout error.
;
; UNIT_GOT_ERROR	an unexpected disconnect occured.  Consider this
;			catastrophic error and return the current command
;			with a bad failure error (whatever that is).
;
; UNIT_WORKING		we got a disconnect message earlier so this unit will
;			be doing a reselection later.  Hang it on WorkingUnits
;			but don't reply the command block yet.
;
; UNIT_WAITING		we got a command complete message earlier so the cmd
;			has completed and we can reply the command block.  If
;			there are more commands on this unit then it will be
;			added to the WaitingUnits queue else we'll forget it.
;
; Whichever of the above actions is taken, we'll attempt to select the next
; unit (which is at the head of the WaitingUnits queue) when we're finished.
;
; This is also called by the XT routines when a command has completed.
;==============================================================================
G4.0101	printf <'A disconnect occured ... '>
		bsr	FlushDMA		someone else is gonna use it
FakeDisconnect	clr.b	st_SelectPending(a5)	selects are OK to do now
		movea.l	st_RunningUnit(a5),a1	get the unit that disconnected
		clr.l	st_RunningUnit(a5)	none are active now
		move.b	hu_WhatNext(a1),d0	get status byte
		cmpi.b	#UNIT_WORKING,d0	destined for the Working queue?
		bne.s	SummatElse		no, must be something else

; this unit is doing a disconnect but will be reselecting the WDC later
	printf <'intermediate\n'>
		move.b	d0,hu_QueueType(a1)	set the new queue type
		lea.l	st_WorkingUnits(a5),a0	adding to WorkingUnits queue
		lea.l	hu_SCSIQueue(a1),a1	SCSI list node for linkage
		ADDHEAD				will probably reselect quickly
	IFD DEBUG_CODE
	cmpi.b #1,st_WasteSpace(a5)
	beq.s nowt
	printf <'Another command is pending too....\n'>
nowt	nop
	ENDC
		bra	DoSelect		maybe select another unit

; all other disconnect types are terminal; we must reply the command block
SummatElse	printf <'terminal\n'>
		move.l	a1,-(sp)		save unit pointer
		movea.l	hu_CurrentCmd(a1),a1	fetch the command block
		clr.b	cb_ErrorCode(a1)	assume no errors
		cmpi.b	#UNIT_GOT_ERROR,d0	really bad error ?
		bne.s	20$
		move.b	#HFERR_Phase,cb_ErrorCode(a1)	*** cop out error
		bra.s	CheckFlush
20$		cmpi.b	#UNIT_TIMED_OUT,d0	was it a timeout ?
		bne.s	CheckFlush
		move.b	#HFERR_SelTimeout,cb_ErrorCode(a1)

; if we did a DMA read or write op then we need to finish the cache management
CheckFlush:

	IFND BONUS
		tst.b	cb_DidCacheStuff(a1)	did we do CachePreDMA() ?
		beq.s	NoDataFlush		nope, skip the tests

		move.l	a1,-(sp)		save cmd pointer
		movea.l	cb_CacheData(a1),a0	get start address we used
		lea.l	cb_CacheLength(a1),a1	point at length we used
		moveq.l	#0,d0
		jsr	_LVOCachePostDMA(a6)	do the stuff
		move.l	(sp)+,a1		restore command ptr
	ENDC

NoDataFlush	jsr	_LVOReplyMsg(a6)	reply the command block
		movea.l	(sp)+,a1		retrieve the unit pointer
	IFD DEBUG_CODE
	subq.b #1,st_WasteSpace(a5)
	ENDC

; we've replied the command so now we see if the unit should be forgotten
; about or added back to the end of the WaitingUnits queue.  If there are
; commands left on the hu_CmdList then we hang on to the unit, else we
; change its type to UNIT_QUIET and just forget it. (Not put on our queues)
		clr.b	hu_QueueType(a1)	assume we're forgetting it
		lea.l	hu_CmdList(a1),a0	are there any commands ?
		TSTLIST	a0
		beq	DoSelect		no, so just select another

	printf <'Got more commands, adding back to waitingunits\n'>
		move.b	#UNIT_WAITING,hu_QueueType(a1)  yes, go to waiting
		lea.l	st_WaitingUnits(a5),a0	adding to Waiting queue
		lea.l	hu_SCSIQueue(a1),a1	SCSI list node for linkage
		ADDTAIL
		bra	DoSelect		maybe select another unit

;==============================================================================
; The rest of these G4.xxxx commands imply that a data/msg/cmd or status phase
; has been requested following connection.  Since all the fancy stuff was done
; when the reselection interrupt came in, we can treat these requests the same
; as if they were following a previously successful transfer command.  The only
; exception to this is setting up the WDC synchronous transfer register to
; match the unit that is now connected.  We'll do it for all phase requests.
; The same thing must be done if an identify comes in and we had the wrong unit
;==============================================================================
G4.1000		bsr.s	SetTransfer
		bra	G1.1000			data out request

G4.1001		bsr.s	SetTransfer
		bra	G1.1001			data in request

G4.1010		bsr.s	SetTransfer
		bra	G1.1010			command request

G4.1011		bsr.s	SetTransfer
		bra	G1.1011			status request

G4.1110		bsr.s	SetTransfer
		bra	G1.1110			message out request

G4.1111		bsr.s	SetTransfer
		bra	G1.1111			message in request

;==============================================================================
; Whenever a target responds to selection or re-selects us, we have to set up
; the WDC into the correct transfer mode/timings for this unit.  Some devices
; don't support synchronous transfers so we'll use asynchronous mode.  Other
; devices do support it (via the messaging system) and we will have already
; determined the REQ/ACK offset for that unit and must store the value in the
; WDC SYNC_TRANSFER register before we attempt any DATA phase transfers. The
; synchronous mode applies to physical units not the individual LUNs !!!!!!
;==============================================================================
SetTransfer	movea.l	st_RunningUnit(a5),a0
		moveq.l	#0,d0
		move.b	hu_Unit(a0),d0
		move.b	st_SyncValues(a5,d0.w),d0
		PUTREG	d0,SYNC_TRANSFER	set sync xfer value
		rts


;==============================================================================
; length/address = CheckPreDMA( length, address, cmdblock, scsicmd )
;   d0	   d1			  d0	  d1	    a0	      a1
;
; Added 020/030/040 cache support.  Call CachePreDMA to maybe translate virtual
; addresses to physical ones and possibly shorten the length of a transfer if
; the chunk of virtual memory is spread over separate pieces of physical mem
; If CachePreDMA changes the length of a transfer then we switch unconditionally
; to programmed I/O.  This prevents problems with splitting block DMA across
; different adresses (the Boyer DMAC screws up when you try this).
;
; Folded in some of the other tests for programmed I/O to save doing them in
; both the read and write routines.  Transfers less than 256 bytes or to odd
; addresses get done with the CPU.  If we're using the older boyer DMAC then
; transfers to odd word addresses must be programmed too.
;
; It is important that cb_DidCacheStuff and cb_Programmed are cleared when
; the command block is received.  They are only set by this routine.
;
; This routine preserves a0/a1 but may modify d0 and d1 (length and address)
;==============================================================================
CheckPreDMA	movem.l	a0/a1,-(sp)		save cmd and scsi pointers
		move.l	d0,-(sp)		stash length
		move.l	d1,-(sp)		setup address we are using

	IFND BONUS
		move.l	scsi_Actual(a1),d0	is this first time through ?
		beq.s	10$			yes, no flags needed (d0=0)
		moveq.l	#DMAF_Continue,d0	no, we're continuing a request
		bra.s	20$			don't set address

10$		move.l	(sp),cb_CacheData(a0)	save start address
		move.l	4(sp),cb_CacheLength(a0)	and length we want to do
		addq.b	#1,cb_DidCacheStuff(a0)	flag that we did cache stuff

20$		movea.l	(sp)+,a0		get address we are using
		movea.l	sp,a1			point to saved length
		jsr	_LVOCachePreDMA(a6)	set everything up
		move.l	d0,d1			d1=actual address to use
		move.l	(sp)+,d0		get new length
		movem.l	(sp)+,a0/a1		restore pointers
	ENDC

	IFD NODMA
		bra.s	30$			unconditional programmed I/O
	ENDC

		tst.b	cb_Programmed(a0)	already doing programmed I/O ?
		bne.s	35$			yes, no need to check anymore

	IFND BONUS
		cmp.l	cb_CacheLength(a0),d0	is length the same
		bne.s	30$			nope, switch to programmed I/O
	ENDC

		cmpi.l	#256,d0			is transfer >= 256 bytes
		blt.s	30$			nope, switch to programmed I/O
		btst.l	#0,d1			is transfer to an even address
		bne.s	30$			nope, switch to programmed I/O

**** software fix for hardware bug.  If WTCH is writeable then we have the
**** Boyer DMA chip which can't DMA to word aligned addresses, it has to be
**** longword aligned.  If we are doing a transfer to an odd word address
**** then we have to use programmed I/O.
		tst.b	st_FixBoyer(a5)		do we need to test for odd word
		beq.s	40$			nope
		btst.l	#1,d1			is address an odd word
		beq.s	40$			nope, DMA is OK

30$		addq.b	#1,cb_Programmed(a0)	use programmed I/O from now on
35$		move.l	cb_WData(a0),d1		use virtual address not physical
40$		rts

		END
