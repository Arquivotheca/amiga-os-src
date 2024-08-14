******************************************************************************
**                                                                          **
**       THIS IS A DISK, REPEAT, DISK DRIVER, NOT TAPES OR PRINTERS!        **
**                                                                          **
******************************************************************************

		SECTION driver,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/lists.i"
		INCLUDE	"exec/ports.i"
		INCLUDE	"exec/tasks.i"
		INCLUDE	"exec/ables.i"
		INCLUDE	"exec/io.i"
		INCLUDE	"exec/memory.i"
		INCLUDE	"exec/errors.i"
		INCLUDE	"devices/trackdisk.i"
		INCLUDE	"devices/timer.i"
		INCLUDE	"devices/scsidisk.i"

		INCLUDE	"modifiers.i"
		INCLUDE	"device.i"
		INCLUDE	"iotask.i"
		INCLUDE "scsitask.i"
		INCLUDE	"board.i"
		INCLUDE	"printf.mac"

		INT_ABLES

		LIST

;		DEBUGENABLE

		XDEF	IOTask

		XREF	GetPubMem,SCSITask

		XREF	_LVOFindTask,_LVOAllocSignal,_LVOSignal,_LVOAddTask
		XREF	_LVOWait,_LVOGetMsg,_LVOPutMsg,_LVOReplyMsg
		XREF	_LVOOpenDevice,_LVOSendIO,_LVOCause,_LVOPermit

		XREF	OneTaskEntry
		XREF	ComputeBlockShift

	IFND IN_KICK
		XREF	TimerName
	ENDC
	IFD IN_KICK
	  IFND V40
		XREF	TimerName
	  ENDC
	ENDC

;==============================================================================
; This task is responsible for taking standard IORequests (at its message port)
; and translating them to internal command blocks with attached SCSIDirect
; structures.  Once translation is completed, the command block is sent to the
; main SCSITask by doing a PutMsg() to it's message port.  When the command has
; completed, it will be replied back to this task for further processing or
; replying back to the original caller if it's complete.  Since SCSITask runs
; at a higher priority than this task, translation of IORequests will only be
; performed during "dead time" when I/O is taking place at the hardware level
; and SCSITask is waiting for it to complete or if SCSITask has nothing to do.
;
; NOTE: for IS_IDE, we use one task, and the SCSITask routines are subroutines.
; This is because the AT ide HW can't be multitasked, even with 2 drives.  If
; it could (ala Randy Hilton's ideas), we'd use the normal 2-task setup.  This
; does break the symetry (a bit) of iotask being HW-independant, but it
; provides better performance for bare-bones 68000 machines (and reduced mem
; usage - no task/stack).
;
; This task performs initialisation and startup of the SCSITask and allocates
; initialised command blocks ready for use by the incoming IORequests.
;
; The main point of all this is that this task NEVER hits the hardware.  Only
; SCSITask need be changed if we move to a different controller.  SCSITask is
; also responsible for translating SCSIDirect cmds for use by non-SCSI devices.
; ONLY RANDOM ACCESS (DISK) DEVICES ARE SUPPORTED BY THIS SECTION OF THE CODE.
;
; This task has been started by the Init routine with stack frame as follows:-
;
;	 4(sp)  = BoardAddress		the physical address of our board
;	 8(sp)	= ParentTask		the task that created us
;	12(sp)	= MessagePort		the port it wants to PutMsg to
;	16(sp)	= DMAC type		terminal count needed (low byte flag)
;	20(sp)	= HDDevice		device address (for hd_IOGlobals)
;==============================================================================
IOTask	printf <'In IOTask'>
		movea.l	4,a6			get exec library
		move.l	#iot_SIZEOF+MP_SIZE*3,d0  get the global memory area
		bsr	GetPubMem		assume this never fails
		; allocates io globals, CmdRetPort, SCSITaskPort, and TimerPort
; FIX!!! Steve didn't check the return!!!
		movea.l	d0,a5			a5 always points to globals
		move.l	a6,iot_SysLib(a5)	stash Exec base
		move.l	4(sp),iot_BoardAddress(a5)
		move.l	12(sp),iot_ReqPort(a5)	stash msgport for IORequests
		move.l	16(sp),d6		dmac type flag (byte)
		move.l	20(sp),a0		device structure
		move.l	a5,hd_IOGlobals(a0)
		move.l	a0,iot_Device(a5)	remember it
	printf <'IoTask: device = $%lx'>,a0

		; set CmdRetPort, SCSITaskPort, and TimerPort
		lea	iot_SIZEOF(a5),a0
		move.l	a0,iot_CmdRetPort(a5)	assume allocation works
		lea	iot_SIZEOF+MP_SIZE(a5),a0
		move.l	a0,iot_SCSITaskPort(a5)	SCSITask will initialise it
		lea	iot_SIZEOF+MP_SIZE+MP_SIZE(a5),a0
		move.l	a0,iot_TimerPort(a5)

		suba.l	a1,a1			find this task
		jsr	_LVOFindTask(a6)	for passing to SCSITask
		move.l	d0,iot_ThisTask(a5)

		moveq.l	#-1,d0			get signal for returning cmds
		jsr	_LVOAllocSignal(a6)
		move.b	d0,iot_CmdRetSig(a5)	save the signal number
		moveq.l	#0,d1
		bset.l	d0,d1			and store as a mask too
		move.l	d1,iot_CmdRetMask(a5)

		movea.l	iot_CmdRetPort(a5),a1	initialise returning cmd port
		move.b	d0,MP_SIGBIT(a1)	it uses this sig bit
		move.l	iot_ThisTask(a5),MP_SIGTASK(a1) to signal this task
		move.b	#NT_MSGPORT,LN_TYPE(a1)	it's a simple message port
		lea.l	MP_MSGLIST(a1),a1
		NEWLIST	a1			and it's currently empty
		
		moveq.l	#-1,d0			get signal for IORequest port
		jsr	_LVOAllocSignal(a6)
		move.b	d0,iot_ReqPendSig(a5)	save the signal number
		moveq.l	#0,d1
		bset.l	d0,d1			and store as a mask too
		move.l	d1,iot_ReqPendMask(a5)

		movea.l	iot_ReqPort(a5),a1	initialise incoming req port
		move.b	d0,MP_SIGBIT(a1)	it uses this signal bit
		move.l	iot_ThisTask(a5),MP_SIGTASK(a1) to signal this task
		move.b	#NT_MSGPORT,LN_TYPE(a1)	it's a simple message port
		lea.l	MP_MSGLIST(a1),a1
		NEWLIST	a1			and it's currently empty

		moveq.l	#-1,d0			get signal for returning timer
		jsr	_LVOAllocSignal(a6)
		move.b	d0,iot_TimerSig(a5)	save the signal number
		moveq.l	#0,d1
		bset.l	d0,d1			and store as a mask too
		move.l	d1,iot_TimerMask(a5)

		movea.l	iot_TimerPort(a5),a1	initialise incoming timer port
		move.b	d0,MP_SIGBIT(a1)	it uses this signal bit
		move.l	iot_ThisTask(a5),MP_SIGTASK(a1) to signal this task
		move.b	#NT_MSGPORT,LN_TYPE(a1)	it's a simple message port
		lea.l	MP_MSGLIST(a1),a1
		NEWLIST	a1			and it's currently empty


;allocate and initialise the command blocks and hang them on the FreeCmds queue
		lea.l	iot_FreeCmds(a5),a0	initialise list of free cmds
		NEWLIST	a0

; a3090 scsitask creates these and adds them to iot_FreeCmds...
	IFND NCR53C710
		moveq.l	#CMD_BLKS-1,d2		number of cmd blocks to make
10$		move.l	#CBSZ,d0		need this much space
		bsr	GetPubMem
		movea.l	d0,a2			a2 points to cb_ struct
		clr.l	cb_FakeCommand(a2)	these are normal commands
		lea.l	cb_SIZEOF(a2),a0	a0=next free space
		move.l	a0,cb_SenseData(a2)	use it for sense data
		lea.l	24(a0),a0		a0=next free space
		move.l	a0,cb_SCSICmd(a2)	use it for first SCSICmd
		movea.l	a0,a1			a1 = first SCSICmd
		lea.l	scsi_SIZEOF(a0),a0	a0=next free space
		move.l	a0,scsi_Command(a1)	SCSICmd's command block
		lea.l	12(a0),a0		a0=next free space
		move.l	a0,cb_SpareSCSICmd(a2)	use it for second SCSICmd
		movea.l	a0,a1			a1 = second SCSICmd
		lea.l	scsi_SIZEOF(a0),a0	a0=next free space
		move.l	a0,scsi_Command(a1)	SCSICmd's command block

	IFD	FIXING_READS
	lea.l	12(a0),a0		a0=next free space
	move.l	a0,cb_KlugeData(a2)	use it for temp buffer area
	ENDC

		move.l	iot_CmdRetPort(a5),MN_REPLYPORT(a2)
		move.w	#cb_SIZEOF,MN_LENGTH(a2)
		movea.l	a2,a1
		lea.l	iot_FreeCmds(a5),a0	add to list of free commands
		ADDHEAD
		dbra	d2,10$
	ENDC ; !NCR53C710

; we need a timer request to use when polling for diskchanges
		moveq.l	#IOTV_SIZE,d0
		bsr	GetPubMem
		move.l	d0,iot_TimerReq(a5)

	IFND IN_KICK
		lea.l	TimerName(pc),a0	open timer.device
	ENDC
	IFD IN_KICK
	  IFD V40
		; V40.5 exec uses 0 for timer.device - internal/librarytags.h
		sub.l	a0,a0			open timer.device
	  ENDC
	  IFND V40
		lea.l	TimerName(pc),a0	open timer.device
	  ENDC
	ENDC
		movea.l	d0,a1			for this IORequest
		moveq.l	#UNIT_VBLANK,d0		this unit
		moveq.l	#0,d1			no special flags
		jsr	_LVOOpenDevice(a6)
		movea.l	iot_TimerReq(a5),a0
		move.l	iot_TimerPort(a5),MN_REPLYPORT(a0)  replies to this port

	IFND ONE_TASK
; now we'll start up the SCSITask with some arguments passed on its stack
; FIX! make one allocation
		move.l	#TC_SIZE,d0		get SCSITask task memory
		bsr	GetPubMem
		movea.l	d0,a2
		move.l	#DEVSTACK,d0		get SCSITask stack
		bsr	GetPubMem
		movea.l	d0,a0
		move.l	a0,TC_SPLOWER(a2)	stash stack pointer
		lea.l	DEVSTACK(a0),a0		point to the top
		move.l	a0,TC_SPUPPER(a2)	and save it

 IFD DEBUG_CODE
 move.l 20(sp),d0
 printf <'passing dev=$%lx'>,d0
 ENDC
		move.l	20(sp),-(a0)		HDDevice address
		move.l	d6,-(a0)		pass DMAC flag
		move.l	iot_SCSITaskPort(a5),-(a0)  passing MsgPort
		move.l	iot_ThisTask(a5),-(a0)	    and this task pointer
		move.l	iot_BoardAddress(a5),-(a0)  and board base address
		move.l	a0,TC_SPREG(a2)		current stack pointer
		move.b	#NT_TASK,LN_TYPE(a2)
		move.b	#HD_PRI+2,LN_PRI(a2)	one higher than this task
		lea.l	SCSITaskName(pc),a0
		move.l	a0,LN_NAME(a2)
 printf <'Starting scsitask (%s), board at 0x%lx\n'>,a0,iot_BoardAddress(a5)
		movea.l	a2,a1			a1 = TCB
		lea.l	SCSITask(pc),a2		a2 = InitialPC
		suba.l	a3,a3			a3 = FinalPC
		jsr	_LVOAddTask(a6)

; before signalling our parent task, we must wait for SCSI task to init.
		moveq.l	#SIGF_SINGLE,d0		wait for OK signal
		jsr	_LVOWait(a6)
 printf <'scsitask ready'>
	ENDC !ONE_TASK
	IFD ONE_TASK

; for the IDE driver, we will only use one task since we don't share the
; hardware, even if two drives are hooked up.  If we had IDE and SCSI, or
; if we had "reselectable IDE", ala Randy Hilton, we wouldn't do this - we'd
; use the normal scsi-like setup.

	printf <'initing SCSITask as subroutine...\n'>
		move.l	20(sp),-(a0)		    HDDevice address
		movem.l	d2-d7/a2-a6,-(sp)	    save all regs
		move.l	d6,-(sp)		    DMAC flag (not used)
		move.l	iot_SCSITaskPort(a5),-(sp)  passing MsgPort
		move.l	iot_ThisTask(a5),-(sp)	    and this task pointer
		move.l	iot_BoardAddress(a5),-(sp)  and board base address

		; call the init point from our process, save it's regs
		bsr	SCSITask
	printf <'done initing SCSITask\n'>
		lea	16(sp),sp		drop args
		movem.l	a3-a6,-(sp)		for use when calling dispatch
		movem.l	(sp)+,d0/d1/a0/a1	move into safe regs
		movem.l	(sp)+,d2-d7/a2-a6	restore all regs (paranoia)
		move.l	st_IntPendMask(a0),iot_IntPendMask(a5)
		movem.l	d0/d1/a0/a1,iot_ATRegs(a5)   d0=a3,d1=a4,a0=a5,a1=a6
	ENDC ONE_TASK

; do this after initing SCSITask, so for the 3090 it can allocate the cmd
; structures...
; we reserve one command block exclusively for handling diskinserted/removed
		lea.l	iot_FreeCmds(a5),a0	get first free Command blk
		REMTAIL
		move.l	d0,iot_PollCmd(a5)	and use only for polling
		movea.l	d0,a0
		addq.l	#1,cb_FakeCommand(a0)	mark IORequest as fake
		clr.b	iot_TimerSent(a5)	no timer request pending

; we have to allocate a fake IORequest for the PollCommand so that scsitask
; can read the appropriate IO_UNIT addresses as needed.
		moveq.l	#IO_SIZE,d0
		bsr	GetPubMem
		movea.l	iot_PollCmd(a5),a0
		move.l	d0,cb_IORequest(a0)

; now it's safe for us to signal init that all initialisation was done.
; should reload d0!!!! normally SIGF_SINGLE, but might have other bits too!!!
; ok, now we do for safety.
		moveq.l	#SIGF_SINGLE,d0		wake up initing task
		movea.l	8(sp),a1		get task to signal
		jsr	_LVOSignal(a6)

;==============================================================================
; The main dispatch point of this task (IOTask).  Waits on two possible events
; that can come in.  The first event that can happen is an IORequest has been
; put to the main iot_ReqPort.  If there is at least one command block free we
; can handle this event by GetMsg()ing the IORequests in turn and converting
; them to SCSIDirect command blocks which are PutMsg()ed to SCSITask which
; converts them to the appropriate hardware operations.
;
; The second event that can occur is a command being returned to iot_CmdRetPort
; from the SCSITask.  We call the routine pointed by cb_ReturnTo for further
; processing or returning to the original caller after the command block has
; been stripped off and returned to the free queue.  Whenever we haven't any
; command blocks free, IORequests are left on the message port until we do.
;==============================================================================
; Try to move as much to regs as possible, to make the loop run faster.  Also
; saves space since a0/a1/a2 are known and usable to the action routines.

		move.l	iot_CmdRetMask(a5),d2	wait for returning commands
		or.l	iot_ReqPendMask(a5),d2	and pending IORequests too
		or.l	iot_TimerMask(a5),d2	and timer events (if needed)
	IFD ONE_TASK
	IFD SCSI_SUPPORTED
		move.l	iot_IntPendMask(a5),d3	
		or.l	d3,d2			wd ints as well
	ENDC
	ENDC
		move.l	iot_CmdRetMask(a5),d4
		move.b	iot_TimerSig(a5),d5
		move.l	iot_ReqPort(a5),d6
		lea.l	iot_FreeCmds(a5),a3
		move.l	iot_CmdRetPort(a5),d7

		; a2 = cb_SCSICmd
		; a3 = free cmd list
		; a4 = io_Unit
		; a5 = iobase
		; a6 = sysbase
		; d2 = wait mask
		; d3 = IntPendMask (onetask w/ scsi only)
		; d4 = cmd return mask
		; d5 = iot_TimerSig
		; d6 = iot_ReqPort
		; d7 = cmd return port

IOEventLoop	move.l	d2,d0			wait mask
		jsr	_LVOWait(a6)
	IFD ONE_TASK
	IFD SCSI_SUPPORTED
		move.l	d3,d1			iot_IntPendMask
		and.l	d0,d1
		beq.s	1$
		move.l	d0,-(sp)		save other signal bits
		bsr	HandleInt
		move.l	(sp)+,d0
		; fall through to handle any other signals
1$
	ENDC
	ENDC	
		btst.l	d5,d0			see if we got a timer
		beq.s	5$			nope

; a timer request has just come in so we need to send a TestUnitReady to
; any unit that handles removeable media.  HandleErrors will send off the
; appropriate ChangeInts if we get media changed or no disk errors.  (It
; is also handled by HandleErrors during the normal course of disk I/O)
		move.l	d0,-(sp)		save received sigs
		bsr	PollForChange		do the polling
		move.l	(sp)+,d0

5$		and.l	d4,d0			did we get returning cmd ?
		beq.s	GetNewRequest		nope, must have been a request

GetOldCommand	movea.l	d7,a0			fetch the returning command
		jsr	_LVOGetMsg(a6)
		tst.l	d0
		beq.s	GetNewRequest		none left
;	printf <'cb=%lx ret...'>,d0
		movea.l	d0,a0
		movea.l	cb_ReturnTo(a0),a1	get code to call
		jsr	(a1)			and call it
		bra.s	GetOldCommand		there may be more left

; We always check to see if there are IORequests pending because we may have
; ignored some on an earlier iteration when there were no command blocks free
; There is no need to filter the command code because this has already been
; handled by the BeginIO entry of the device.  All routines that are called
; by this dispatcher (and the returning command dispatcher) get a pointer to
; the current command block in a0; they are expected to preserve d2-d7/a2-a7
GetNewRequest	TSTLIST	a3			are there cmd blocks free ?
		beq.s	IOEventLoop		no, wait for returning cmds

		move.l	d6,a0			yes, fetch an IORequest
		jsr	_LVOGetMsg(a6)
		tst.l	d0			did we get one
		beq	IOEventLoop		nope, all done so wait more

		move.l	d0,-(sp)		save IORequest pointer
		move.l	a3,a0			get first free Command blk
		REMHEAD
		movea.l	d0,a0
	printf <'new io = $%lx, cb=%lx...'>,(sp),a0

		; reset fields so we don't have to duplicate it later...
		; default command len to 6, but clear all 12
		movea.l	cb_SCSICmd(a0),a2	initialise SCSICmd
		move.w	#6,scsi_CmdLength(a2)	default to 6 byte CDB
		clr.l	scsi_Length(a2)		no data phase expected
		clr.l	scsi_Actual(a2)
		move.b	#SCSIF_READ,scsi_Flags(a2)	no special flags
		movea.l	scsi_Command(a2),a1	now initialise CDB
		clr.l	(a1)
		clr.l	4(a1)
		clr.l	8(a1)

		clr.w	cb_Retries(a0)		reset retry count
		clr.l	cb_Done(a0)		complete retry check
		clr.b	cb_ErrorCode(a0)
		clr.w	cb_MappingDone(a0)

		move.l	(sp)+,a1		get IOR back (param to rtn)
		move.l	a1,cb_IORequest(a0)	stash IORequest in cmd blk
		clr.b	IO_ERROR(a1)
		clr.l	IO_ACTUAL(a1)
		move.l	IO_LENGTH(a1),cb_Length(a0)
		move.l	IO_DATA(a1),cb_Data(a0)
		move.l	IO_OFFSET(a1),cb_Offset(a0)
		move.l	IO_UNIT(a1),a4

		move.w	IO_COMMAND(a1),d0	fetch the command code
		lsl.w	#1,d0			use to access word offsets...
		move.w	Dispatch(pc,d0.w),d0	...to the appropriate code
		jsr	Dispatch(pc,d0.w)	Routine( CommandBlock in a0 )
;						and IOR in a1 & SCSICmd in a2
		bra	GetNewRequest		and fetch the next one

;==============================================================================
; The command table.  Each entry is a word offset from the table to the code.
; The entries marked with a q will never get to this task because they are
; considered quick and are handled as part of the callers task by BeginIO.
; BeginIO will do the correct thing and reply the message or just rts to caller
;==============================================================================
Dispatch	DC.W	BadCmd-Dispatch		0  invalid	q
		DC.W	BadCmd-Dispatch		1  reset	q
		DC.W	ReadWrite-Dispatch	2  read
		DC.W	ReadWrite-Dispatch	3  write
		DC.W	Update-Dispatch		4  update	q
		DC.W	BadCmd-Dispatch		5  clear	q
		DC.W	StopUnit-Dispatch	6  stop
		DC.W	Start-Dispatch		7  start
		DC.W	BadCmd-Dispatch		8  flush	q
		DC.W	BadCmd-Dispatch		9  motor	q
		DC.W	Seek-Dispatch		10 seek
		DC.W	DoFormat-Dispatch	11 format
		DC.W	DoRemove-Dispatch	12 remove
		DC.W	BadCmd-Dispatch		13 changenum	q
		DC.W	BadCmd-Dispatch		14 changestate	q
		DC.W	DoProtStatus-Dispatch	15 protstatus
		DC.W	BadCmd-Dispatch		16 rawread	q
		DC.W	BadCmd-Dispatch		17 rawwrite	q
		DC.W	BadCmd-Dispatch		18 getdrivetype q
		DC.W	BadCmd-Dispatch		19 getnumtracks q
		DC.W	AddChangeInt-Dispatch	20 addchangeint	q
		DC.W	BadCmd-Dispatch		21 remchangeint q
	IFND GET_GEOMETRY
		DC.W	BadCmd-Dispatch		22 lastcomm	q GetGeometry
	ENDC
	IFD GET_GEOMETRY
		DC.W	GetGeometry-Dispatch	22 lastcomm	q GetGeometry
	ENDC
		DC.W	Eject-Dispatch		23		  Eject
		DC.W	BadCmd-Dispatch		24
		DC.W	BadCmd-Dispatch		25
		DC.W	BadCmd-Dispatch		26
		DC.W	BadCmd-Dispatch		27
		DC.W	DirectSCSICmd-Dispatch	28 scsicmd

;==============================================================================
; Update( CommandBlock, IOR, scsicmd, unit )
;		a0	 a1	a2     a4
;
; Flush any cache buffers to the medium.  If we get unknown command, remember
; not to try it anymore, and return success.
;==============================================================================
Update		tst.b	hu_UpdateFails(a4)	does this drive not support it?
		bne	CmdComplete		return with no error

		move.w	#10,scsi_CmdLength(a2)	it's a 10 byte CDB
		movea.l	scsi_Command(a2),a1	now initialise CDB
		move.b	#SYNCHRONIZE_CACHE,(a1)	LBA = 0 Len = 0 (everything)

RetryUpdate	tst.l	cb_Done(a0)		are we retrying
		bne	CmdComplete		yep, but there was no error
		move.l	#-1,cb_Done(a0)		nope, but next time we will be

		bsr	SendCommandRTS		send the command

; returns here when the sync command has completed (or failed)
		move.b	cb_ErrorCode(a0),d0	any SCSI bus errors ?
		beq.s	10$			no, check for target errors
		movea.l	cb_IORequest(a0),a1	stash an error in the IOReq
		move.b	d0,IO_ERROR(a1)		(will be HFERR_something)
		bra	CmdComplete		and fail this IORequest now

; there were no SCSI erors, check to see if the target returned a bad status
10$		move.b	scsi_Status(a2),d0	and the target status
		beq	CmdComplete		no errors occured

; something went wrong, let HandleErrors() figure it out then maybe continue
		bsr	HandleErrors		handle whatever the error was
		movea.l	d0,a0			assume we can retry
		tst.l	d0			can we retry ?
		bne.s	RetryUpdate		yep, do it again
		rts				nope, cmdblock was replied


;==============================================================================
; DoRemove( CommandBlock, IOR, SCSICmd, unit )
;		a0	   a1	 a2	 a4
;
; Checks that the unit in the IORequest has removeable media and adds it to
; the list (array) of polled drives if it does (and isn't alredy there).  If
; the first timer request hasn't been sent, then it is sent at this time too.
;
; FIX!!!!!!! this only handles max 8 removable media units!!!!
;==============================================================================
DoRemove printf <'Remove called\n'>
		tst.b	hu_SCSIQual(a4)		is it removeable ?
		bpl	CmdComplete		nope, so don't add changer
	printf <'Got removeable media\n'>
		lea.l	iot_PollMe(a5),a1	point to polled unit array
10$		tst.l	(a1)			spare slot ?
		beq.s	GotSlot			yep
		cmp.l	(a1)+,a4		same unit already there ?
		bne.s	10$			nope, look for next
		bra	CmdComplete		yes, bag the remove request

GotSlot		move.l	a4,(a1)			save in the array
		movea.l	cb_IORequest(a0),a1
		move.l	IO_DATA(a1),d0
		move.l	d0,hu_RemoveAddress(a4)	install the ChangeInt
		tst.b	iot_TimerSent(a5)	is timer running ?
		bne	CmdComplete		yep, all done
		move.l	a0,-(sp)		save command block ptr
		move.b	#1,iot_TimerSent(a5)
		move.l	#1,iot_TimeOut(a5)
		bsr	PollFinished		send timer command
		movea.l	(sp)+,a0
		bra	CmdComplete

;==============================================================================
; AddChangeint( CommandBlock, IOR )
;		     a0	       a1
;
; Checks that the unit in the IORequest has removeable media and adds it to
; the list (array) of polled drives if it does (and isn't alredy there).  If
; the first timer request hasn't been sent, then it is sent at this time too.
;
*	Alas, TDURemove is not robust enough.  This routine supports
*	an extensible list of software interrupts for use by many
*	different supporting drivers.
*
*	The call does not "complete" (e.g. TermIO).  The request
*	is stashed until TDURemChangeInt is called, when it is
*	finally replied.
*
*	Forbids to avoid problems with RemChangeInt.
;==============================================================================

AddChangeInt:
		move.l	a0,-(sp)		save cb
	printf <'AddChangeInt called\n'>

		; since we can't return with an error, ALWAYS add it to list
		lea	hu_ChangeList(a4),a0
		FORBID				; doesn't touch a0/a1
		ADDTAIL				; add a1 on list a0
		PERMIT				; destroys a0/a1

		tst.b	hu_SCSIQual(a4)		is it removeable ?
		bpl.s	AddDone			nope, so don't add changer
	printf <'Got removeable media\n'>
		lea.l	iot_PollMe(a5),a1	point to polled unit array
10$		tst.l	(a1)			spare slot ?
		beq.s	ACIGotSlot		yep
		cmp.l	(a1)+,a4		same unit already there ?
		bne.s	10$			nope, look for next
		bra.s	AddDone			yes, bag the remove request

ACIGotSlot	move.l	a4,(a1)			save in the array
		tst.b	iot_TimerSent(a5)	is timer running ?
		bne.s	AddDone			yep, all done
		move.b	#1,iot_TimerSent(a5)
		move.l	#1,iot_TimeOut(a5)
		bsr	PollFinished		send timer command

		; since ior shouldn't be returned, free up command block
		; no errors possible - all are ignored.
AddDone:	move.l	(sp)+,a1		TRICKY! a0 restored to a1!
		lea	iot_FreeCmds(a5),a0	list to add to
		ADDHEAD			     ;  return commandblock to freelist
		rts
		
		
;==============================================================================
; PollForChange()
;
; This routine is ONLY initiated by a returning timer request (which is sent
; the first time by a call to REMOVE).  Subsequent timer requests are launched
; by this routine itself.  The main function of this routine is to force the
; driver to do something during dead time (when no commands are active) and
; hence notice when disks are changed in removeable media drives.  It is
; important to note that cb_IORequest is totally faked up here and ONLY the
; IO_UNIT field is initialised (it's used by SCSITask to find the SCSI ID).
;
; Since this routine uses the same mechanism for handling returning commands
; we can't save any registers here (but that's OK for the main loop anyway).
;==============================================================================
PollForChange	movea.l	iot_TimerPort(a5),a0	fetch timer request
		jsr	_LVOGetMsg(a6)
		tst.l	d0			don't really need this test
		bne.s	10$
		rts

10$		move.l	#3,iot_TimeOut(a5)	assume all disks in

		movea.l	iot_PollCmd(a5),a0	get command block to a0
		lea.l	RePoll(pc),a1
		move.l	a1,cb_ReturnTo(a0)
		clr.l	cb_LastPolled(a0)	none done yet
		bra.s	PollNext

; after each poll (test unit ready) control returns to here (cmd in a0)
; NOTHING _local_ must be on stack (we call handleerrors).
RePoll		movea.l	d0,a0
		move.b	cb_ErrorCode(a0),d0	any SCSI errors ?
		bne.s	ThisPollDone		yes, just look for next
		movea.l	cb_SCSICmd(a0),a1	get SCSIDirect stuff
		move.b	scsi_Status(a1),d0	any errors ?
		beq.s	ThisPollDone		nope, scan the next unit
		bsr	HandleErrors		handle the error condition

; there is no need to check the return here because all we wanted was for
; the error handlers (for no disk and media changed) to do thier stuff!
; NOTE: doesn't always return the CB, may return NULL!
		movea.l	iot_PollCmd(a5),a0	get command block back in a0
		movea.l	cb_IORequest(a0),a1	see if disk is in
		movea.l	IO_UNIT(a1),a1
		tst.l	hu_ChangeState(a1)
		beq.s	ThisPollDone		it is
		move.l	#2,iot_TimeOut(a5)	out, check every second

ThisPollDone	addq.l	#1,cb_LastPolled(a0)
PollNext	move.l	cb_LastPolled(a0),d0
		lsl.w	#2,d0
		move.l	iot_PollMe(a5,d0.w),d0
		beq.s	PollFinished		no more units to poll
		movea.l	cb_IORequest(a0),a1
		move.l	d0,IO_UNIT(a1)		poll this one next

; Send a TEST_UNIT_READY cmd to the drive (don't really care if it is ready)
	printf <'sending TUR to %ld'>,d0
		clr.w	cb_Retries(a0)		don't let it accumulate!
		movea.l	cb_SCSICmd(a0),a1	initialise the command
		move.w	#6,scsi_CmdLength(a1)
		clr.l	scsi_Length(a1)
		clr.b	scsi_Flags(a1)
		movea.l	scsi_Command(a1),a1
		clr.l	(a1)			TEST_UNIT_READY = 0
		clr.w	4(a1)
		bra	SendCommand		send command

; Once all active units (with removeable media) have been checked, send timer
; request off again.  iot_Timeout contains the number of seconds we wait.
; The code for Remove calls here if the timer hasn't been started already
PollFinished	
	printf <'sending timer IOR'>
		movea.l	iot_TimerReq(a5),a1
		clr.l	IOTV_TIME+TV_MICRO(a1)
		move.l	iot_TimeOut(a5),IOTV_TIME+TV_SECS(a1)
		move.w	#TR_ADDREQUEST,IO_COMMAND(a1)
		jmp	_LVOSendIO(a6)

;==============================================================================
; ReadWrite( CommandBlock, IOR )
;		  a0        a1
;
; Initialises the IORequest for a new READ or WRITE operation and puts the SCSI
; read or write code into the command byte portion of the SCSIDirect structure.
; If the given IO_LENGTH is greater than 256 blocks an extended read or write
; command is issued instead.  Units are assumed to support extended commands
; until they fail one with an unknown command error.  If this happens, it is
; flagged in the unit structure by setting MaxBlocks=256 instead of 65535.
;
; Note: all tests for building the command block must check for CMD_READ and
; CMD_WRITE and TD_SEEK since we use the same code for all of these.
;==============================================================================
ReadWrite	; test to see if we still need to do ReadCapacity/etc
	printf <'Read/Write offset = $%lx, length = $%lx\n'>,IO_OFFSET(a1),IO_LENGTH(a1)
		tst.b	hu_MountDone(a4)	0 means never ready
		bne	RWReady

		; unit has never been seen ready.  Before trying r/w with
		; default blocksize, try again to ReadCapacity/etc.
	printf <'device never ready'>
		move.w	#10,scsi_CmdLength(a2)
		move.l	cb_SenseData(a0),scsi_Data(a2)	read data to here
		move.l	#8,scsi_Length(a2)	returns 8 bytes of info
;		move.b	#SCSIF_READ,scsi_Flags(a2)	no special flags
		move.l	scsi_Command(a2),a1	CDB ptr
;		clr.l	(a1)
;		clr.l	4(a1)
;		clr.w	8(a1)			this is a 10 byte CDB
		move.b	#READ_CAPACITY,(a1)	we want to get the block size
		bsr	SendCommandRTS

		move.b	cb_ErrorCode(a0),d0
		bne.s	20$			scsi phase error probably
		tst.b	scsi_Status(a2)		success?
		bne.s	20$			no
	printf <'Read Capacity success, %ld bytes'>,scsi_Actual(a1)
		cmp.l	#8,scsi_Actual(a1)	did we get the blocksize?
		bne.s	10$			no - don't do this again
		move.l	cb_SenseData(a0),a1	data pointer
		move.l	(a1),-(sp)		size disk size in blocks
		move.l	4(a1),d0		blocksize
		bsr	ComputeBlockShift	returns d0 blocksize, d1 shift
		move.w	d1,hu_BlockShift(a4)	save the shift count
		move.l	d0,hu_BlockSize(a4)	and block size
		move.l	(sp)+,hu_DiskSize(a4)	and # of blocks on the disk

10$		move.b	#1,hu_MountDone(a4)	1 means ready, not mounted

; FIX  we really should do the WP test and the get geometry stuff
		
20$		; reset fields we modified
		clr.b	cb_ErrorCode(a0)	no errors from SCSITask yet

RWReady		movea.l	cb_IORequest(a0),a1
		tst.l	IO_LENGTH(a1)		anything to be done ?
		bne.s	2$			yes

		cmpi.w	#TD_SEEK,IO_COMMAND(a1)	seek can have 0 length
		bne	CmdComplete		not seek, so we wasted time

2$		; copy relevant IORequest fields - already done before switch
	IFD	FIXING_READS
	clr.l	cb_SaveData(a0)		not used kluge area yet
	ENDC
		lea.l	RWReturn(pc),a1		return commands to here...
		move.l	a1,cb_ReturnTo(a0)	...once we've sent them
		bra.s	RWLeft			start the read/write op

; command has been returned by SCSITask for further processing/error handling
RWReturn	tst.w	cb_MappingDone(a0)	did we map a bad block ?
		beq.s	5$			nope
		move.l	cb_PrevOffset(a0),cb_Offset(a0)  yes, restore offset
		clr.w	cb_MappingDone(a0)	no mapping done yet

5$		move.b	cb_ErrorCode(a0),d0	any SCSI bus errors ?
		beq.s	10$			no, check for target errors
		movea.l	cb_IORequest(a0),a1	stash an error in the IOReq
		move.b	d0,IO_ERROR(a1)		(will be HFERR_something)
		bra	CmdComplete		and fail this IORequest now

10$		move.b	scsi_Status(a2),d0	and the target status
		beq.s	RWContinue		nothing bad happened

; got a bad status, handle the error condition and maybe exit if totally bad
		bsr	HandleErrors
		movea.l	d0,a0			restore command block ptr
		tst.l	d0			was it terminal ?
		bne.s	RWContinue		no, might retry though
		rts				IORequest was replied already

; comes here to continue a previous read/write operation or to start a new one
RWContinue:
	IFD	FIXING_READS
	tst.l	cb_SaveData(a0)		did we use kluge area ?
	beq.s	10$			nope, so don't restore it
	move.l	cb_SaveData(a0),cb_Data(a0)  yep, restore original ptr
10$	ENDC

		movea.l	cb_IORequest(a0),a1
		cmpi.w	#TD_SEEK,IO_COMMAND(a1)
		beq	CmdComplete		seek completed

		move.l	cb_Done(a0),d0		how much last time ?
		beq.s	RWLeft			none, error or first time

;******************************************************************************
; this is part of the fix for the chip error where the last 2 words of a read
; transfer could be bad.  Whenever a read returns we check to see if the kluge
; data area was used.  If it was then we copy from our kluge data buffer to the
; required place in the users buffer.  If it wasn't, then we simply subtract
; one block from the amount of data transferred and carry on as normal.
	IFD	FIXING_READS
	XREF	_LVOCopyMem
	movea.l	cb_IORequest(a0),a1	only reads need fixing
	cmpi.w	#CMD_READ,IO_COMMAND(a1)
	bne.s	RWNoKluge		so skip this on writes

	sub.l	hu_BlockSize(a4),d0	so we don't get bad data
	tst.l	cb_SaveData(a0)		was the read kluged up
	beq.s	RWNoKluge		nope, so carry on normally

	movem.l	d0/a0,-(sp)		yep, we read to kluge buffer
	movea.l	cb_Data(a0),a1		where we're copying to
	movea.l	cb_KlugeData(a0),a0	where we're copying from
	jsr	_LVOCopyMem(a6)		d0 still holds blocksize
	movem.l	(sp)+,d0/a0
RWNoKluge:	ENDC
;******************************************************************************

		clr.w	cb_Retries(a0)		reset retry count
		movea.l	cb_IORequest(a0),a1
		add.l	d0,IO_ACTUAL(a1)	bump ACTUAL in IORequest
		add.l	d0,cb_Data(a0)		bump source/dest address
		add.l	d0,cb_Offset(a0)	and logical disk address
		sub.l	d0,cb_Length(a0)	subtract from left to do
		beq	CmdComplete		all completed, return IOReq

; comes here when there is more work left to do (cb_Length is non zero)
RWLeft	IFD	FIXING_READS
	clr.l	cb_SaveData(a0)		assume no kluge happening
	ENDC
		movem.l	d2/a2-a3,-(sp)		need some spare regs
		movea.l	cb_IORequest(a0),a2	a0 = command block
		move.w	IO_COMMAND(a2),d2	d2 = command code (CMD_xxx)
		movea.l	cb_SCSICmd(a0),a1	a1 = SCSICmd
		movea.l	scsi_Command(a1),a3	a3 = SCSI command data

; first, determine how many blocks we are going to have to transfer this time
		move.l	cb_Length(a0),d0	get length to be done
		move.w	hu_BlockShift(a4),d1	use blockshift to get...
		lsr.l	d1,d0			number of blocks to do
		cmp.l	hu_MaxBlocks(a4),d0	are we in range for this unit
		ble.s	10$			yes, prolly ok for extended
		move.l	hu_MaxBlocks(a4),d0	no, truncate to max (256!65535)

; d0 contains length of this operation and cb_Offset the disk address. Check if
; this transfer crosses over a mapped bad block and adjust offset and length
; accordingly if we do.  We'll leave a flag in the command block to say that
; cb_Offset was adjusted if we end up reading a mapped block.
10$		move.w	hu_TotalMapped(a4),d1	are any blocks mapped out
		beq.s	NoMapping		nope, so no mapping check

; there are some software mapped bad blocks on this unit, check if we hit one
		movem.l	d2-d4/a2,-(sp)		need some spare registers
		move.w	hu_BlockShift(a4),d4	for byte to block conversions
		movea.l	hu_BadBlockList(a4),a2	point to list of bad blocks
		move.l	cb_Offset(a0),d2	offset we're writing too
		lsr.l	d4,d2			convert to a block offset

		move.l	d0,d3			calculate upper bound...
		add.l	d2,d3			...for range checking
		subq.l	#1,d3
		bra.s	30$			start the loop

20$		cmp.l	(a2),d2			are we past this mapped block
		bgt.s	25$			yes, check for next
		cmp.l	(a2),d3			is mapped block in our range
		bge.s	GotMap			yes, need to map or truncate
25$		addq.l	#8,a2			bump the pointer
30$		dbra	d1,20$			and keep going
		bra.s	MissedMap		didn't need to map anything

; we found a mapped bad block that was somewhere in the transfer we are about
; to perform.  The difference between cb_Offset (d2) and the mapped block (a2)
; is what the transfer length should be truncated to.  If the difference is 0
; then we have to do a mapping operation by setting d0 to 1 (only transfer one
; block) and cb_Offset to the mapped block area.  
GotMap		move.l	4(a2),d1		get mapped block number
		move.l	(a2),d0			get block that is bad
		sub.l	d2,d0			we can transfer this much
		bne.s	MissedMap		not on mapped block yet

; our transfer begins on a bad block so we must adjust cb_Offset to point to
; the mapped area and leave a flag in the command block to say we did that.
	printf <'Mapping %ld to block #%ld\n'>,d2,d1
		moveq.l	#1,d0			how many blocks we'll xfer
		move.l	cb_Offset(a0),cb_PrevOffset(a0)
		lsl.l	d4,d1			convert back to byte offset
		move.l	d1,cb_Offset(a0)	the new offset for this block
		move.w	d0,cb_MappingDone(a0)	flag that we did a map

MissedMap	movem.l	(sp)+,d2-d4/a2

; branches directly to here if there are no software mapped blocks on the unit
; handle >1Gig reads/writes
NoMapping       move.l  d0,-(sp)                save blocks to xfer     <<<<
                move.l  cb_Offset(a0),d1                                <<<<
                move.w  hu_BlockShift(a4),d0                            <<<<
                lsr.l   d0,d1                   gte block offset        <<<<
                move.l  (sp)+,d0                restore blocks to xfer  <<<<
                cmp.l   #$001fffff,d1           too big for read(6)?    <<<<
                bhi.s   ForceExtended           Yes, force extended cmd <<<<
         	cmpi.l	#256,d0			should this be extended r/w
		bls.s	NormalRW		nope, can use a normal cmd

; we are building an extended read or write command (request was >256 blocks)
; or out of the offset range of read(6).
ForceExtended	move.b	#READ_EXTENDED,(a3)	assume read
		cmpi.w	#CMD_READ,d2		was that correct
		beq.s	10$			yep
		move.b	#WRITE_EXTENDED,(a3)	nope, we're writing
10$		move.w	d0,d1			need to preserve d0
		move.b	d1,8(a3)		store LSB of xfer length
		lsr.w	#8,d1			fetch MSB
		move.b	d1,7(a3)		store MSB of xfer length
		move.w	hu_BlockShift(a4),d1	get byte length of xfer again
		lsl.l	d1,d0
		move.l	d0,cb_Done(a0)		save as amount done this time
		move.l	cb_Offset(a0),d0	convert offset to block offset
		lsr.l	d1,d0
		move.l	d0,2(a3)		and save in SCSI command
		clr.b	1(a3)
		clr.b	9(a3)			no flags or links
		move.w	#10,scsi_CmdLength(a1)	it's a 10 byte CDB
		bra.s	FinalRW			do the last bit and send cmd

; we are building a regular read or write command (request was <=256 blocks)
; and less than block 0x1fffff (1gig).
NormalRW	move.b	#READ,(a3)		assume read
		cmpi.w	#CMD_READ,d2		was that correct
		beq.s	10$			yes
		move.b	#WRITE,(a3)		nope, writing
		cmpi.w	#CMD_WRITE,d2
		beq.s	10$
		cmpi.w	#TD_FORMAT,d2		could be format too
		beq.s	10$
		move.b	#SEEK,(a3)		nope, must be a seek
		

;******************************************************************************
; This is the kluge when we are reading.  Whenever a read completes, we will
; always lie and say it was one block less than it really was, this ensures
; that we don't try to use the last block of data (which could be bad).  If
; there really is only one block left we'll save off the real data pointer
; and read two blocks to our buffer and copy the first block to the user
; buffer when the read returns.
10$	IFD	FIXING_READS
	cmpi.w	#CMD_READ,d2		only fixing reads
	bne.s	NoKluge2		it was a write
	cmpi.w	#1,d0			is this a single block read ?
	bne.s	NoKluge2		nope, just do a normal op
	move.l	cb_Data(a0),cb_SaveData(a0)	it is, save data ptr
	move.l	cb_KlugeData(a0),cb_Data(a0)	use the kluge buffer
	addq.l	#1,d0			and read an extra block
NoKluge2	ENDC
;******************************************************************************

		move.b	d0,4(a3)		save xfer length (256=0 here)
		move.w	hu_BlockShift(a4),d1	get byte length of xfer again
		lsl.l	d1,d0
		move.l	d0,cb_Done(a0)		save as amount done this time
		move.l	cb_Offset(a0),d0	convert offset to block offset
		lsr.l	d1,d0
		move.w	d0,2(a3)		LSB's of block address
		swap	d0			save MSB
		move.b	d0,1(a3)
		clr.b	5(a3)			no flags or links
		move.w	#6,scsi_CmdLength(a1)	it's a 6 byte CDB

; final initialisation of the SCSICmd before we send it to the SCSITask
FinalRW		move.b	#SCSIF_READ,scsi_Flags(a1)
		cmpi.w	#CMD_READ,d2		set data direction
		beq.s	10$			defaults to read, most common
		move.b	#SCSIF_WRITE,scsi_Flags(a1)
10$		move.l	cb_Done(a0),scsi_Length(a1)
		move.l	cb_Data(a0),scsi_Data(a1)
 printf <'$%lx bytes to $%lx'>,scsi_Length(a1),scsi_Data(a1)

; send this command block to SCSITask (it will come back for processing later)
		bsr	SendCommand		command block in a0
		movem.l	(sp)+,d2/a2-a3
		rts

;==============================================================================
; Eject( CommandBlock, IOR )
;	      a0	a1
;
; Stops the current unit by sending a START_STOP_UNIT command with wait
; enabled, and forces an eject if possible with the LoEj bit.  io_Length has
; 0 (Load) or 1 (Eject).
;==============================================================================
Eject:		move.l	IO_LENGTH(a1),d0
		eor.b	#1,d0			0 = stop, 1 = start
		or.b	#2,d0			add LoEj
		bra.s	StartStopEntry

;==============================================================================
; StopUnit( CommandBlock, IOR )
;		a0	   a1
;
; Stops the current unit by sending a START_STOP_UNIT command with wait enabled
;==============================================================================
StopUnit	moveq.l	#0,d0			do a stop
		bra.s	StartStopEntry

;==============================================================================
; StartUnit( CommandBlock, IOR, SCSICmd )
;		  a0	    a1	   a2
;
;Starts the current unit by sending a START_STOP_UNIT command with wait enabled
;==============================================================================
Start		moveq.l	#1,d0
StartStopEntry	movea.l	scsi_Command(a2),a1	now initialise CDB
		move.b	#START_STOP_UNIT,(a1)	this command
		move.b	d0,4(a1)		0=stop, 1=start, 2 = eject

ReStartStop	tst.l	cb_Done(a0)		are we retrying
		bne	CmdComplete		yep, but there was no error
		move.l	#-1,cb_Done(a0)		nope, but next time we will be

		bsr	SendCommandRTS		send the command

; returns here when the start/stop command has completed (or failed)
		move.b	cb_ErrorCode(a0),d0	any SCSI bus errors ?
		beq.s	10$			no, check for target errors
		movea.l	cb_IORequest(a0),a1	stash an error in the IOReq
		move.b	d0,IO_ERROR(a1)		(will be HFERR_something)
		bra	CmdComplete		and fail this IORequest now

; there were no SCSI erors, check to see if the target returned a bad status
10$		move.b	scsi_Status(a2),d0	and the target status
		beq	CmdComplete		no errors occured

; something went wrong, let HandleErrors() figure it out then maybe continue
		bsr	HandleErrors		handle whatever the error was
		movea.l	d0,a0			assume we can retry
		tst.l	d0			can we retry ?
		bne.s	ReStartStop		yep, do it again
		rts				nope, cmdblock was replied

	IFD GET_GEOMETRY
;==============================================================================
; GetGeometry( CommandBlock, IOR, unit )
;		  a0	      a1   a4
;
; returns the device geometry, if we have one
; FIX! should check IO_LENGTH!
;==============================================================================
GetGeometry	moveq.l	#dg_SIZEOF,d0
		move.l	d0,IO_ACTUAL(a1)
		move.l	IO_DATA(a1),a1

		move.l	hu_BlockSize(a4),(a1)+	; dg_SectorSize
		move.l	hu_DiskSize(a4),(a1)+	; dg_TotalSectors
		move.l	hu_TotalCyls(a4),(a1)+	; dg_Cylinders
		move.w	hu_TotalBlocks(a4),d0	; high word is 0 here!
		move.l	d0,(a1)+		; dg_CylSectors
		moveq	#1,d1
		move.l	d1,(a1)+		; dg_Heads
		move.l	d0,(a1)+		; dg_TrackSectors
	IFD DMA_24_BIT
		move.l	#MEMF_24BITDMA,d0	; only valid under 2.0
	    IFD IS_1_3
		cmp.w	#$37,LIB_VERSION(a6)	; check exec version
		bcc.s	10$			; >= 37 (unsigned)
		move.l	#MEMF_PUBLIC,d0
10$:
	    ENDC
	ENDC
	IFND DMA_24_BIT
		move.l	#MEMF_PUBLIC,d0
	ENDC
		move.l	d0,(a1)+		; dg_BufMemType
		move.b	hu_SCSIType(a4),(a1)+	; dg_DeviceType
		moveq	#0,d0
		tst.b	hu_SCSIQual(a4)
		bpl.s	20$			; top bit is removable media
		moveq	#1,d0
20$:		move.b	d0,(a1)+		; dg_Flags

		bra	CmdComplete		; cb in a0
	ENDC GET_GEOMETRY

;==============================================================================
; Seek( CommandBlock, IOR )
;	     a0	       a1
;
; This one cheats a bit and modifies the IORequest to ensure that IO_LENGTH is
; set to zero and then calls the Read/Write code.  This means seeks to mapped
; blocks will be mapped appropriately.
;==============================================================================
Seek		clr.l	IO_LENGTH(a1)
		bra	ReadWrite

;==============================================================================
; Format( CommandBlock, IOR, SCSICmd )
;	       a0	 a1	a2
;
; This one has a few wierdnesses and foibles.  If IO_OFFSET is 0 then the whole
; unit will be formatted with a FORMAT_UNIT command with no special options.
; However, if the offset is >0, format will call write instead to get the user
; data out onto the disk (our format takes the same args as CMD_WRITE).  No
; checking is done for bad blocks beyond that provided by the error handling.
;==============================================================================
DoFormat
;	printf <'Format, offset=0x%lx\n'>,IO_OFFSET(a1)
		tst.l	IO_OFFSET(a1)
		bne	ReadWrite		non 0, use the read/write code

		movea.l	scsi_Command(a2),a1
		move.b	#FORMAT_UNIT,(a1)	this command

; File systems generally don't have access to track 0, so this will be a call
; from a prep utility.  Any format of track 0 will low level format the whole
; disk.  Should I invalidate the RigidDiskBlock on the unit when this happens?
ReFormat
; this was for a Sony magneto optical drive
;	move.b #$03,2(a1)
		move.w	#1,cb_Done(a0)		error recovery flag
		bsr	SendCommandRTS		send command for processing

; returns here when the format has completed.  May need to try again
		movea.l	cb_IORequest(a0),a1	pass the SCSI error code
		move.b	cb_ErrorCode(a0),IO_ERROR(a1)
		bne	CmdComplete		SCSI error, exit now

		tst.b	scsi_Status(a2)
		beq	CmdComplete		nope, so command has finished
		bsr	HandleErrors		call the error handler
		movea.l	d0,a0			see if it was fatal
		tst.l	d0			did we get Command block back ?
		bne.s	10$			yep, see if we can retry
		rts				fatal error, cmd replied

10$		tst.l	cb_Done(a0)		should we retry everything ?
		beq.s	ReFormat		yes
		bra	CmdComplete		nope, there wasn't an error

;==============================================================================
; DoProtStatus( CommandBlock, IOR, SCSICmd )
;		    a0	       a1     a2
;
; Returns the write protect status of the disk (!0 means protected)
;==============================================================================
DoProtStatus	move.l	#4,scsi_Length(a2)	only want 4 bytes
		move.l	cb_SenseData(a0),scsi_Data(a2)  use this for data
		movea.l	scsi_Command(a2),a1
		move.b	#MODE_SENSE,(a1)	this command
		move.b	#4,4(a1)		this much data
		
		bsr	SendCommandRTS		send command for processing

; returns here when the mode_sense has completed.
		movea.l	cb_IORequest(a0),a1	pass the SCSI error code
		move.b	cb_ErrorCode(a0),IO_ERROR(a1)
		bne	CmdComplete		SCSI error, exit now

; when the IORequest comes in, IO_ACTUAL is set to 0 (disk write enabled)
		movea.l	cb_SenseData(a0),a1	get sense data area
		tst.b	2(a1)
		bpl	CmdComplete		disk is write enabled
		movea.l	cb_IORequest(a0),a1	mark as protected
		subq.l	#1,IO_ACTUAL(a1)	makes it -1
		bra	CmdComplete

;==============================================================================
; DirectSCSICmd( CommandBlock, IOR, SCSICmd )
;		      a0	a1	a2
;
; Copies the callers SCSICmd (pointed to by IO_DATA) into the local SCSICmd
; held in the command block and sends it to SCSITask for processing.  SCSITask
; has all the smarts to handle these command blocks with the exception of the
; AutoSense flag.  If returned target status is Check Condition and the caller
; has set the SCSIF_AutoSense flag in scsi_Flags, we'll do Request Sense call
; and get the sense data into scsi_SenseData for the caller.
;==============================================================================
DirectSCSICmd	move.l	a2,cb_UserSCSICmd(a0)  save our SCSICmd
		move.l	IO_DATA(a1),cb_SCSICmd(a0)  use caller SCSICmd instead
		bsr	SendCommandRTS		send command for processing

; returns here when the user command has completed (we must transfer error code)
		movea.l	cb_IORequest(a0),a1
		move.b	cb_ErrorCode(a0),IO_ERROR(a1)
		bne	DirectComplete		error, return it now

; if there was a check condition and SCSIF_AUTOSENSE is 1, do request sense now
		movea.l	cb_SCSICmd(a0),a1
		tst.b	scsi_Status(a1)		status error of some kind
		beq	DirectComplete		nope, everything worked OK

		movea.l	cb_IORequest(a0),a1	got a bad status
		move.b	#HFERR_BadStatus,IO_ERROR(a1)  so mark this fact
		movea.l	cb_SCSICmd(a0),a1	refetch scsicmd

		cmpi.b	#BUSY,scsi_Status(a1)	Is the unit Busy?
		bne.s	10$			no
;		bsr	WaitASecond		FIX! really should be shorter
		bra.s	DirectSCSICmd		retry until not busy

10$		cmpi.b	#CHECK_CONDITION,scsi_Status(a1) did we get check condition ?
		bne	DirectComplete		nope, return the command
		btst.b	#SCSIB_AUTOSENSE,scsi_Flags(a1)
		beq.s	DirectComplete		don't need to do request sense

; we have to do a request sense command for the caller ( lazy bastard! )
		move.l	scsi_SenseData(a1),-(sp)  save callers sense buffer
		moveq	#0,d0
		move.w	scsi_SenseLength(a1),d0	and the sense length
		clr.w	scsi_SenseActual(a1)	no sense data fetched yet
		movea.l	cb_UserSCSICmd(a0),a1	restore our old SCSICmd
		move.l	a1,cb_SCSICmd(a0)
		move.l	(sp)+,scsi_Data(a1)	where data phase goes
		move.l	d0,scsi_Length(a1)	how much data
		move.b	#SCSIF_READ,scsi_Flags(a1)	no special flags
		move.w	#6,scsi_CmdLength(a1)	using a 6 byte command

		movea.l	scsi_Command(a1),a1	point to CDB
		clr.l	(a1)			initialise to 0
		clr.w	4(a1)
		move.b	#REQUEST_SENSE,(a1)	this command
		move.b	d0,4(a1)		save user allocation length

RetrySense	bsr	SendCommandRTS		send command for processing

; we've done the request sense command for the caller, now return the command
		movea.l	cb_SCSICmd(a0),a1
		cmpi.b	#BUSY,scsi_Status(a1)	Is the unit Busy?
		bne.s	10$			no
		move.l	a0,-(sp)
 XREF WaitASecond
		bsr	WaitASecond
		move.l	(sp)+,a0
		bra.s	RetrySense		yes, wait and try again

10$		move.l	a0,-(sp)		save command block pointer
		movea.l	cb_IORequest(a0),a1
		movea.l	cb_SCSICmd(a0),a0
		movea.l	IO_DATA(a1),a1		set up sense actual
		move.w	scsi_Actual+2(a0),scsi_SenseActual(a1)
		movea.l	(sp)+,a0		restore original command block

DirectComplete	move.l	cb_UserSCSICmd(a0),cb_SCSICmd(a0)  restore old ptr
		bra	CmdComplete		and reply it to the caller


;==============================================================================
; CommandBlock = HandleErrors( status, CommandBlock ) 
;      d0			 d0	    a0
;
; CALLER ABSOLUTELY MUST BSR TO THIS ROUTINE BECAUSE RETURN ADDRESS IS STASHED
; CALLER MUST ALSO ENSURE THAT THE STACK IS CLEAN (NO REGS SAVED) WHEN CALLING
;
; This is the generic error handler for all SCSI commands initiated by the
; driver code.  DirectSCSI commands are NOT handled by this code, it's up to
; the caller to handle error conditions in this case.  Some special casing is
; done for read and write operations so we can perform block reassignment and
; retries if nescessary.
;
; The top value on the stack is stashed in cb_ErrorCaller and the current value
; of cb_ReturnTo is stashed in cb_OldReturnTo ready for re-initialisation if
; control is to be passed back to the caller.
;
; If this routine returns 0, then the IORequest will already have been replied
; but if the return value is non-0 then the caller will be responsible for
; retrying the command and returning the IORequest later.  Read and write
; operations will have cb_Done adjusted for the operation that got an error
; and the code should just continue as if nothing had happened if it gets a
; non zero return from HandleErrors.
;
; If cb_Done is returned containing 0 then the whole command should be sent
; to SCSITask again.  Whenever the command block is returned, the state of the
; SCSICmd structure is not guaranteed, it must be completely re-initialised.
;
; HandleErrors CANNOT call itself, it must handle its own errors internally.
;
; This routine should only be called for target status errors not SCSI errors.
;==============================================================================
HandleErrors	move.l	cb_ReturnTo(a0),cb_OldReturnTo(a0)
		move.l	(sp)+,cb_ErrorCaller(a0)	return addr off stack

		cmpi.b	#$10,d0			intermediate good ?
		beq	ErrorRetry		yes, so not an error condition

		cmpi.b	#CHECK_CONDITION,d0	check condition ?
		beq	CheckCondition		yes

; all other errors indicate that the last command was not executed at all
; If the drive was busy we'll just re-send the same command but not count
; it as an error recovery retry (cb_Retries is left alone)
		clr.l	cb_Done(a0)		no data was transferred
		cmpi.b	#BUSY,d0		was the drive busy ?
		beq	ErrorRetry		yep, just try the command again

;FIX!!!!!!!!!!!!!!!
; REJ - This Reserve/Release code is HOSED.  Steve didn't understand Reserve
; and reservation conflict.  For the time being, just keep retrying.  Should
; really wait a bit and retry.

; If this is a reservation conflict we'll issue a RESERVE command of our own.
; If disconnection has been enabled, the RESERVE command will be queued in the
; target until the other reserving initiator has released the device. (Maybe ?)
; FIX! use define
		cmpi.b	#$18,d0			was it a reservation conflict ?
		beq	ErrorRetry		no, don't know the error

	IFD STEVES_HOSED_RESERVE
		cmpi.b	#$18,d0			was it a reservation conflict ?
		bne	ErrorRetry		no, don't know the error

DoReserve	movea.l	cb_SCSICmd(a0),a1
		move.w	#6,scsi_CmdLength(a1)	6 byte CDB
		clr.l	scsi_Length(a1)		no data phase expected
		clr.b	scsi_Flags(a1)		no special flags
		movea.l	scsi_Command(a1),a1	fetch SCSI command block ptr
		move.b	#RESERVE,(a1)		doing a reserve command
		clr.b	1(a1)			no data phase initialisation..
		clr.l	2(a1)			...required for RESERVE cmd
		bsr	SendCommandRTS

; A reserve command has completed, see if it worked or if we should error out
		moveq.l	#-1,d0			assume an error occured
		tst.b	cb_ErrorCode(a0)	did the SCSI stuff go OK
		bne	ErrorQuit		nope, so fail
		movea.l	cb_SCSICmd(a0),a1
		move.b	scsi_Status(a1),d0	what was the target status
		beq.s	DoRelease		OK, so now release the unit
		cmpi.b	#$18,d0			another reservation conflict ?
		beq	ErrorQuit		yes, reselection was disabled
		cmpi.b	#BUSY,d0		busy (this shouldn't happen)
		beq.s	DoReserve		yes, just try again
		bra.s	CheckCondition		nope, probably check condition

; the reserve worked and the unit is now ours.  Release it again then continue
DoRelease	movea.l	cb_SCSICmd(a0),a1
		move.w	#6,scsi_CmdLength(a1)	6 byte CDB
		clr.l	scsi_Length(a1)		no data phase expected
		clr.b	scsi_Flags(a1)		no special flags
		movea.l	scsi_Command(a1),a1	fetch SCSI command block ptr
		move.b	#RELEASE,(a1)
		clr.b	1(a1)			no data phase initialisation..
		clr.l	2(a1)			...required for RELEASE cmd
		bsr	SendCommandRTS

; A release command has completed, see if it worked or if we should error out
; ***** if something failed here the unit will be permanently reserved ******
		moveq.l	#-1,d0			assume an error occured
		tst.b	cb_ErrorCode(a0)	did the SCSI stuff go OK
		bne	ErrorQuit		nope, so fail
		movea.l	cb_SCSICmd(a0),a1
		move.b	scsi_Status(a1),d0	what was the target status
		beq	ErrorRetry		OK, so retry original command
		cmpi.b	#BUSY,d0		unit was busy (can't happen)
		beq.s	DoRelease		try again if it was
		bra	ErrorQuit		everything else is bad
	ENDC

; we got a check condition status from a caller command or from a command
; sent by the ErrorHandler itself.  Whichever it was, we now send a
; Request Sense command with a request for extended sense data.  We'll take
; appropriate action based on the return values from Request Sense.  If
; d0<>2 on entry here then we have a status that we don't know how to handle
; so we should just exit (and reply the IORequest) straight away.
; We use the spare SCSI command block so we know what command was issued to
; cause the Check Condition status in the first place.  The previous SCSICmd
; might be required to calculate how much actual data were transferred OK
; before the error condition occured or to flag that certain commands are
; not supported by this unit (extended read and write for instance).
CheckCondition	cmpi.b	#CHECK_CONDITION,d0	was it Check Condition status
		bne	ErrorQuit		no, so terminate this command
		movea.l	cb_SCSICmd(a0),a1	switch to spare SCSIDirect
		move.l	cb_SpareSCSICmd(a0),cb_SCSICmd(a0)
		move.l	a1,cb_SpareSCSICmd(a0)

ReCheck		movea.l	cb_SCSICmd(a0),a1
		move.l	cb_SenseData(a0),scsi_Data(a1)	using our sense area
		move.l	#22,scsi_Length(a1)	22 bytes of sense data
		move.w	#6,scsi_CmdLength(a1)	6 byte CDB
		move.b	#SCSIF_READ,scsi_Flags(a1)
		movea.l	scsi_Command(a1),a1	fetch command block address
		move.b	#REQUEST_SENSE,(a1)	this command
		clr.b	1(a1)
		clr.l	2(a1)			clear rest of CDB
		move.b	#22,4(a1)		except the allocation length
		bsr	SendCommandRTS		send command to hardware

; our request sense completed, make sure it worked and act on the results
; NOTE: HFERR_BadStatus comes from us, not scsitask.
		tst.b	cb_ErrorCode(a0)	any SCSI errors
		bne	ErrorQuit		yes, bag this cmd now
		movea.l	cb_SCSICmd(a0),a1	were there any target errors
		move.b	scsi_Status(a1),d0
		beq.s	CheckOK			no, sense data is valid
		cmpi.b	#BUSY,d0		was the unit busy ?
		beq.s	ReCheck			yes, just try again
		bra	ErrorQuit		no, consider a terminal error

; The Request Sense command completed OK, now call routines based on sense key
; We asked for extended sense data but some units won't support it and will
; just return 4 bytes of data anyway.  We can detect this if error class/code
; is not $70.  This means the first byte is the whole error code in itself.
; All error handling routines are entered with the sense key in d0. If sense
; key is -1 then the default action is taken for that error.  However, if the
; sense key is valid, it may modify the action taken for that error (ie. if a
; recovered error occured).
;
; For error codes we don't understand we simply retry up to a max of 10 times

CheckOK		moveq.l	#TDERR_NotSpecified,d0	assume sense key invalid
		tst.l	scsi_Actual(a1)		get length of sense data
		beq	ErrorQuit		WHAT! no data phase??????
		clr.w	d1			using a word for error code
		movea.l	cb_SenseData(a0),a1	get ptr to sense data area
		move.b	(a1),d1			what class and code ?
		andi.b	#$7f,d1			mask out address valid bit
		cmpi.b	#$70,d1			is it extended sense ?
		bne.s	GotCode			no, so we have the code in d1
		move.b	2(a1),d0		get the sense key
		andi.b	#$0f,d0
		move.b	12(a1),d1		and get the sense code

		; must leave a1 pointing to sense data!
GotCode		cmpi.w	#$4f,d1			is it in range ?
		bgt	MaybeRetryAll		nope, maybe do a retry
		lsl.w	#1,d1			accessing table of word offsets
		move.w	ErrHandlers(pc,d1.w),d1	get the offset
		beq	MaybeRetryAll		0 offset = not handled
	printf <'sense key = '>
		jmp	ErrHandlers(pc,d1.w)	handle this error

; dispatch table is used to call the appropriate handler for the error code.
; Error handlers are called with the sense key in d0 and command block in a0
; the sense data is pointed to by a1 so the sense qualifier and other data
; may be examined.
ErrHandlers	DC.W	ec00-ErrHandlers	no sense information
		DC.W	ec01-ErrHandlers	no index/sector signal
		DC.W	ec02-ErrHandlers	no seek complete
		DC.W	ec03-ErrHandlers	write fault
		DC.W	ec04-ErrHandlers	drive not ready
		DC.W	ec05-ErrHandlers	drive not selected
		DC.W	ec06-ErrHandlers	no track 0 found
		DC.W	ec07-ErrHandlers	multiple drives selected
		DC.W	ec08-ErrHandlers	LUN communication failure
		DC.W	ec09-ErrHandlers	servo error (track following)
		DC.W	ec0a-ErrHandlers 	no disk (is this SONY only ?)
		DC.W	ec0b-ErrHandlers	load failure (SONY only ?)
		DC.W	ec0c-ErrHandlers	spindle failure (SONY only ?)
		DC.W	ec0d-ErrHandlers	focus failure (SONY only ?)
		DC.W	ec0e-ErrHandlers	tracking failure (SONY only ?)
		DC.W	0	ec0f-ErrHandlers

		DC.W	ec10-ErrHandlers	ID CRC or ECC error
		DC.W	ec11-ErrHandlers	unrecoverable read error
		DC.W	ec12-ErrHandlers	no ID address mark
		DC.W	ec13-ErrHandlers	no data address mark
		DC.W	ec14-ErrHandlers	no record found
		DC.W	ec15-ErrHandlers	seek error
		DC.W	ec16-ErrHandlers	data sync mark error
		DC.W	ec17-ErrHandlers	recovered read with retries
		DC.W	ec18-ErrHandlers	recovered read with ECC
		DC.W	ec19-ErrHandlers	defect list error
		DC.W	ec1a-ErrHandlers	parameter overrun
		DC.W	ec1b-ErrHandlers   	synchronous request error
		DC.W	ec1c-ErrHandlers	defect list not found
		DC.W	ec1d-ErrHandlers   	compare error
		DC.W	ec1e-ErrHandlers	recovered ID
		DC.W	0	ec1f-ErrHandlers

		DC.W	ec20-ErrHandlers	invalid command
		DC.W	ec21-ErrHandlers	invalid block address
		DC.W	ec22-ErrHandlers	illegal function for device
		DC.W	0	ec23-ErrHandlers
		DC.W	ec24-ErrHandlers	illegal field in CDB
		DC.W	ec25-ErrHandlers	invalid LUN
		DC.W	ec26-ErrHandlers	bad field in parameter list
		DC.W	ec27-ErrHandlers   	write protected
		DC.W	ec28-ErrHandlers	media changed
		DC.W	ec29-ErrHandlers	drive reset
		DC.W	ec2a-ErrHandlers	mode select parms changed
		DC.W	0	ec2b-ErrHandlers
		DC.W	0	ec2c-ErrHandlers
		DC.W	0	ec2d-ErrHandlers
		DC.W	0	ec2e-ErrHandlers
		DC.W	0	ec2f-ErrHandlers

		DC.W	ec30-ErrHandlers	incompatible cartridge
		DC.W	ec31-ErrHandlers	medium format corrupted
		DC.W	ec32-ErrHandlers	no more spares available
		DC.W	0	ec33-ErrHandlers
		DC.W	0	ec34-ErrHandlers	
		DC.W	0	ec35-ErrHandlers	
		DC.W	0	ec36-ErrHandlers	
		DC.W	0	ec37-ErrHandlers	
		DC.W	0	ec38-ErrHandlers	
		DC.W	0	ec39-ErrHandlers	
		DC.W	ec3a-ErrHandlers	medium not present
		DC.W	0	ec3b-ErrHandlers	
		DC.W	0	ec3c-ErrHandlers	
		DC.W	0	ec3d-ErrHandlers	
		DC.W	0	ec3e-ErrHandlers	
		DC.W	0	ec3f-ErrHandlers

		DC.W	ec40-ErrHandlers	RAM failure
		DC.W	ec41-ErrHandlers	ECC diagnostic failure
		DC.W	ec42-ErrHandlers	power on diagnostic failure
		DC.W	ec43-ErrHandlers	message reject error
		DC.W	ec44-ErrHandlers	internal controller error
		DC.W	ec45-ErrHandlers	reselect timeout
		DC.W	ec46-ErrHandlers	unsuccessful soft reset
		DC.W	ec47-ErrHandlers	SCSI interface parity error
		DC.W	ec48-ErrHandlers	initiator detected error
		DC.W	ec49-ErrHandlers	inappropriate or illegal msg
		DC.W	0	ec4a-ErrHandlers
		DC.W	0	ec4b-ErrHandlers
		DC.W	0	ec4c-ErrHandlers
		DC.W	0	ec4d-ErrHandlers
		DC.W	0	ec4e-ErrHandlers
		DC.W	0	ec4f-ErrHandlers


PrintErr	MACRO	; \1 = symbol to branch to, \2-? = printf args
	IFD DEBUG_MODE
		printf	\2,\3,\4,\5,\6,\7,\8,\9
		bra.s	\1
	ENDC
		ENDM

;=== ErrorRetry entries:
;=== (all fall through unless debugging is on)

; no error occured (why did we get a check condition status then ?)
ec00		PrintErr err_retry,<'no error\n'>

; SONY load/unload failure
ec0b		PrintErr err_retry,<'load/unload failure\n'>

; special for the SONY drive Focus Failure
ec0d		PrintErr err_retry,<'focus failure\n'>

; SONY tracking failure
ec0e		PrintErr err_retry,<'tracking failure\n'>

; recovered read with retries
ec17		PrintErr err_retry,<'recovered read with retries\n'>

; recovered read with ECC
ec18		PrintErr err_retry,<'recovered read with ECC\n'>

; recovered ID
ec1e		PrintErr err_retry,<'recovered ID\n'>

; drive reset
ec29		PrintErr err_retry,<'drive reset\n'>

; mode select parameters changed
ec2a		printf <'mode select parameters changed\n'>

		; all fall through to here unless debugging is on
err_retry:	bra	ErrorRetry


;=== ErrorQuit(-1) entries:
;=== (all fall through unless debugging is on)

; no index or sector signal (probably on a read or write)
ec01		PrintErr QuitMinusOne,<'no index or sector signal\n'>

; write fault.  This means there's a hardware error (maybe XT drive not ready)
ec03		PrintErr QuitMinusOne,<'write fault\n'>

; drive not selected.  How did we give it commands if it didn't select ????
ec05		PrintErr QuitMinusOne,<'drive not selected\n'>

; multiple drives selected (I don't think we can ever get in this situation)
ec07		PrintErr QuitMinusOne,<'multiple drives selected\n'>

; LUN communication failure.  Could we get this if we selected a bad LUN ?
ec08		PrintErr QuitMinusOne,<'LUN communication failure\n'>

; parameter overrrun
ec1a		PrintErr QuitMinusOne,<'parameter overrun\n'>

; synchronous request error
ec1b		PrintErr QuitMinusOne,<'synchronous request error\n'>

; defect list not found
ec1c		PrintErr QuitMinusOne,<'defect list not found\n'>

; invalid block address
ec21		PrintErr QuitMinusOne,<'invalid block address\n'>

; illegal function for device
ec22		PrintErr QuitMinusOne,<'illegal function for device\n'>

; illegal field in CDB
ec24		PrintErr QuitMinusOne,<'illegal field in CDB\n'>

; bad field in parameter list
ec26		PrintErr QuitMinusOne,<'bad field in parameter list\n'>

; incompatible cartridge (return unreadable disk)
ec30		PrintErr QuitMinusOne,<'incompatible cartridge\n'>

; medium format corrupted
ec31		PrintErr QuitMinusOne,<'medium format corrupted\n'>

; no more spares available
ec32		PrintErr QuitMinusOne,<'no more spares\n'>

; RAM failure
ec40		PrintErr QuitMinusOne,<'RAM failure\n'>

; ECC diagnostic failure
ec41		PrintErr QuitMinusOne,<'ECC diagnostic failure\n'>

; power on diagnostic failure
ec42		PrintErr QuitMinusOne,<'power on diagnostic failure\n'>

; internal controller error
ec44		PrintErr QuitMinusOne,<'internal controller error\n'>

; inappropriate or illegal message
ec49		printf <'illegal message\n'>

		; all fall through to here
QuitMinusOne	moveq.l	#TDERR_NotSpecified,d0
		bra	ErrorQuit



;=== MaybeRetryAll entries:
;=== (all fall through unless debugging is on)

; a seek command (or implied seek for read,write and other commands) failed
ec02		PrintErr maybe_retry,<'no seek complete\n'>

; servo error.  Pretty serious but we can maybe fix it with a retry
ec09		PrintErr maybe_retry,<'servo error\n'>

; ID CRC or ECC error, just retry using retry count
ec10		PrintErr maybe_retry,<'ID CRC or ECC error\n'>

; unrecoverable read error, retry using retry count
ec11		PrintErr maybe_retry,<'unrecoverable read error\n'>

; no ID address mark
ec12		PrintErr maybe_retry,<'no ID address mark\n'>

; no data address mark
ec13		PrintErr maybe_retry,<'no data address mark\n'>

; no record found
ec14		PrintErr maybe_retry,<'no record found\n'>

; seek error
ec15		PrintErr maybe_retry,<'seek error\n'>

; data sync mark error
ec16		PrintErr maybe_retry,<'data sync mark error\n'>

; defect list error
ec19		PrintErr maybe_retry,<'defect list error\n'>

; compare error
ec1d		PrintErr maybe_retry,<'compare error\n'>

; message reject error ?????
ec43		PrintErr maybe_retry,<'message reject error\n'>

; reselect timeout ????
ec45		PrintErr maybe_retry,<'reselection timeout\n'>

; unsuccsesful soft reset
ec46		PrintErr maybe_retry,<'unsuccessful soft reset\n'>

; SCSI interface parity error
ec47		PrintErr maybe_retry,<'SCSI parity error\n'>

; initiator detected error
ec48		printf <'initiator detected error\n'>

		; all fall through to here unless debugging is on
maybe_retry:	bra	MaybeRetryAll



; drive not ready.  We can always retry this command, drive will be ready later
; ***** Added extra code here to handle removeable media drives that don't
; ***** support any kind of "no disk" error.  If we get a transition from
; ***** ready (hu_ChangeState=0) to not ready, we call the disk removed code
; ***** to send the Cause to anyone waitng for it.  This code won't be
; ***** called by the startup routines since Test Unit Ready is done
; ***** directly and HandleErrors won't be called.
ec04	printf <'drive not ready\n'>
		movea.l	cb_IORequest(a0),a1
		movea.l	IO_UNIT(a1),a1
		tst.b	hu_SCSIQual(a1)		is this removeable ?
		bmi.s	ec0a			yes, maybe do remove stuff
		bra	ErrorRetryAll

; no track 0 found (usually power up diagnostic problem, but not always)
ec06	printf <'no track 0 found\n'>
		moveq.l	#TDERR_SeekError,d0
		bra	ErrorQuit

; no disk in drive (This may be specific to the SONY magneto optical disk)
; If this unit is marked as being present (hu_ChangeState = 0) and there
; is a pending remove request (hu_RemoveAddress != 0) then we should mark
; the disk as now out and do the appropriate Cause()
; ec3a is standard, ec0a is supposed to be log overflow
ec0a
ec3a	printf <'no disk\n'>
		moveq	#-1,d0			disk removed
		bsr.s	NotifyChange		doesn't modify a0!!

; before sending the command block back we should see if it's a fake one
; generated by PollForChange.  If it is, then we won't generate an error
; condition. If it's from some other command, then error is generated as
; normal.
OutAlready	tst.l	cb_FakeCommand(a0)
		bne	ErrorRetry		fake, continue normally
		moveq.l	#TDERR_DiskChanged,d0	Really no disk in drive
		bra	ErrorQuit		real, so bail out.

; notify remove and addchangeint customors of disk insertion/removal
; d0 - new change state
; doesn't modify a0!
NotifyChange	movea.l	cb_IORequest(a0),a1
		movea.l	IO_UNIT(a1),a1		get unit pointer
		cmp.l	hu_ChangeState(a1),d0	has state of disk changed?
		beq.s	NotifyDone		nope, so no Cause
		move.l	d0,hu_ChangeState(a1)	mark disk as in/out now
		move.l	hu_RemoveAddress(a1),d1	should we wake someone ?
		beq.s	NoRemove		nope, finish up
		move.l	a0,-(sp)		save command block
		movea.l	d1,a1
	printf <'sending ChangeInt\n'>
		jsr	_LVOCause(a6)		wake 'em up
		movea.l	(sp)+,a0		get back command block ptr

; Now check for AddChangeInt people to be notified
NoRemove	movem.l	d2/a0,-(sp)		save cmdblk and spare regs
		move.l	cb_IORequest(a0),a1
		move.l	IO_UNIT(a1),a1		get unit pointer
		FORBID				; for safety with RemChangeInt
		move.l	hu_ChangeList(a1),d2	head pointer

1$		move.l	d2,a1
		move.l	(a1),d2			next pointer
		beq.s	2$			end of list, done

		move.l	IO_DATA(a1),a1		who to cause
		jsr	_LVOCause(a6)
		bra.s	1$			handle next

2$		PERMIT				; trashes a0/a1/etc
		movem.l	(sp)+,d2/a0
NotifyDone	rts


; SONY spindle failure
; SCSI-2 Write error - if ASCQ is 0x01, then it was reallocated - ignore
ec0c		printf <'spindle failure\n'>
		cmp.b	#1,13(a1)
		bne.s	1$		; if not recovered, retry
		moveq	#0,d0		; no error
		bra	ErrorQuit	; returns d0 as error code
1$:		bra	ErrorRetry

; invalid command.  If we just issued an extended read or an extended write
; then this device obviously doesn't support it.  set hu_MaxBlocks to 256 so
; we never attempt to issue this command to this unit again.  If it was a
; reassign blocks command that failed, then we have to do some software bad
; block mapping.  This will call the same code as for no more spares error.
;
; if this was a TD_Update (SCSI SynchronizeCache), remember we shouldn't send
; that to this unit and return success.
ec20	printf <'invalid command\n'>
		movea.l	cb_SpareSCSICmd(a0),a1	get original SCSICommand
		movea.l	scsi_Command(a1),a1
		cmpi.b	#READ_EXTENDED,(a1)
		beq.s	10$			was an extended command
		cmpi.b	#WRITE_EXTENDED,(a1)
		bne.s	20$			some other error
10$		movea.l	cb_IORequest(a0),a1	time to limit xfers....
		movea.l	IO_UNIT(a1),a1		.... to 256 blocks
		move.l	#256,hu_MaxBlocks(a1)
		bra	ErrorRetryAll		and retry the whole command ^

20$		cmpi.b	#REASSIGN_BLOCKS,(a1)	did we attempt a reassign ?
		beq	ec32			yep, do software block mapping

		cmpi.b	#SYNCHRONIZE_CACHE,(a1)	was this an update?
		bne.s	30$			no
		movea.l	cb_IORequest(a0),a1	
		movea.l	IO_UNIT(a1),a1		make sure we don't try an
		move.b	#1,hu_UpdateFails(a1)	 update again.
		moveq	#0,d0			return success
		bra	ErrorQuit		return IOR

30$		moveq.l	#IOERR_NOCMD,d0		some other command didn't work
		bra	ErrorQuit		make it fatal ^


; invalid LUN (this shouldn't ever happen on normal read or write calls because
; the bad units will have been filtered out in the open call (via MakeUnit) but
; I'm leaving this in here in case I ever handle errors on usr SCSIDirect calls
ec25	printf <'invalid LUN\n'>
		moveq.l	#TDERR_BadUnitNum,d0	LUN bad, so unit num is bad!
		bra	ErrorQuit		quit now ^

; write protected
ec27	printf <'write protected\n'>
		moveq.l	#TDERR_WriteProt,d0
		bra	ErrorQuit		return write protect error ^

; media changed
; SCSI-2 says this is media insertion (not ready to ready), but most older
; removable media drives (flopticals, etc) seem to consider it any media
; change (Briar, Teac, Insite) under the CCS.  Should really invert state
; of media!  I suspect it will retry, and that retry will get a 0x04
; ASC (NOT READY/logical unit not ready). - REJ
ec28	printf <'media changed\n'>
		moveq	#0,d0			disk inserted!
		bsr	NotifyChange		doesn't modify a0!!

; before sending the command block back we should see if it's a fake one
; generated by PollForChange.  If it is, then we won't generate an error
; condition. If it's from some other command, then error is generated as
; normal.
InAlready	tst.l	cb_FakeCommand(a0)
		bne.s	ErrorRetry		fake, continue normally
		bra.s	ErrorRetryAll		retry with retry count

; allow a retry, but we want the whole command retried again
MaybeRetryAll	clr.l	cb_Done(a0)

; we got an error code that we don't understand or some retryable failure
; so retry up to max times and fail this IORequest if we exceed the maximum
MaybeRetry
	printf <'MaybeRetry of $%lx, key = $%lx'>,a0,d0
		addq.w	#1,cb_Retries(a0)	bump the retry count
		cmpi.w	#10,cb_Retries(a0)	have we gone too far
		bgt.s	ErrorQuit		yep, so fail this IORequest
		bra.s	ErrorRetry

; retry the whole command but don't affect the retry count
ErrorRetryAll	clr.l	cb_Done(a0)

; the command block may have been modified (cb_Done) but the original caller
; is allowed to retry (or continue) the current operation.  Restore command
; block to its original state (SCSICommand may be garbage now though) and
; return to the caller by jumping to the address stashed in cb_ErrorCaller
ErrorRetry
	printf <'ErrorRetry of $%lx, key = $%lx'>,a0,d0
		move.l	a0,d0			return cmd block in d0
		move.l	cb_OldReturnTo(a0),cb_ReturnTo(a0)
		movea.l	cb_ErrorCaller(a0),a0
		jmp	(a0)			re-call original caller

; a terminal error condition has occured (IO fields are updated accordingly)
; Value in d0 should be used for the IO_ERROR field and then wind up this cmd.
ErrorQuit
	printf <'ErrorQuit of $%lx, key = $%lx'>,a0,d0
		movea.l	cb_IORequest(a0),a1	stash the error
		move.b	d0,IO_ERROR(a1)
		move.l	cb_OldReturnTo(a0),cb_ReturnTo(a0)
		move.l	cb_ErrorCaller(a0),-(sp)	who called us
		bsr.s	CmdComplete		reply IOReq and free CmdBlock
		moveq	#0,d0			failure
		rts				back to caller

;==============================================================================
; SendCommand( CommandBlock )
;		    a0
;
; Merges the correct LUN (from the unit in IORequest) with the second byte of
; the SCSI command blocks and then sends this command block to SCSITask.
; (Stopped doing this! no need to merge LUN when Identify messages are used)
;==============================================================================
SendCommandRTS:
		move.l	(sp)+,cb_ReturnTo(a0)
SendCommand:
;		movea.l	cb_IORequest(a0),a1
;		movea.l	IO_UNIT(a1),a1
;		move.b	hu_LUN(a1),d0		fetch LUN to be merged
;		movea.l	cb_SCSICmd(a0),a1
;		movea.l	scsi_Command(a1),a1
;		or.b	d0,1(a1)		merge LUN with CDB
		movea.l	a0,a1			sending this message
		move.l	iot_CmdRetPort(a5),MN_REPLYPORT(a1)
		movea.l	iot_SCSITaskPort(a5),a0	to this port
;	printf <'send cb=%lx to $%lx...'>,a1,a0
	IFND ONE_TASK
		jmp	_LVOPutMsg(a6)
	ENDC
	IFD ONE_TASK
		jsr	_LVOPutMsg(a6)		put message in port
HandleInt:
		movem.l	a2-a6/d2,-(sp)		save current regs
		movem.l	iot_ATRegs(a5),a3-a6	get scsitask global regs...
		bsr	OneTaskEntry		message should be replied now
		movem.l	(sp)+,a2-a6/d2		restore regs
		rts
	ENDC
;==============================================================================
; CmdComplete( CommandBlock )
;		    a0
;
; Strips the IORequest from the command block and replies it to the original
; caller.  IO_ERROR and IO_ACTUAL should have been set up already.  The command
; block itself is returned to the FreeCmds queue ready for later use.
;
; New addition for disk inserted/removed poller.  If cb_FakeCommand is set then
; there is no command block to be replied because the request was sent by
; an internal routine (driven by a timer).  Just return immediately without
; adding the command block to the list of free command blocks.
;==============================================================================
;==============================================================================
; BadCmd:
; If we ever get to here then something is seriously wrong.  The commands that
; point to this routine should already have been handled by the BeginIO code.
;==============================================================================
BadCmd		; bra	CmdComplete

CmdComplete	tst.l	cb_FakeCommand(a0)	is this a user call ?
		bne.s	CmdFake			nope, internal poller
		move.l	a0,-(sp)		save command block ptr
		movea.l	cb_IORequest(a0),a1
;	printf <'reply io=%lx, free cb=$%lx...'>,a1,a0
		jsr	_LVOReplyMsg(a6)	reply the message
		movea.l	(sp)+,a1
		lea.l	iot_FreeCmds(a5),a0
		ADDHEAD				put command block on free queue
CmdFake		rts

	IFD IS_SCSIDISK
SCSITaskName	DC.B	'SCSI handler',0
	ENDC

	IFD IS_A590
SCSITaskName	DC.B	'A590 SCSI handler',0
	ENDC

	IFD IS_A2091
SCSITaskName	DC.B	'A2091 SCSI handler',0
	ENDC

	IFD IS_A2090
SCSITaskName	DC.B	'A2090 SCSI handler',0
	ENDC

	IFD IS_A3090
SCSITaskName	DC.B	'A3090 SCSI handler',0
	ENDC

	IFD IS_A4000T
SCSITaskName	DC.B	'A4000T SCSI handler',0
	ENDC

	IFD IS_IDE
SCSITaskName	DC.B	'IDE handler',0
	ENDC

		CNOP	0,2

		END
