		SECTION driver,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/memory.i"
		INCLUDE	"exec/tasks.i"
		INCLUDE	"exec/ables.i"
		INCLUDE	"devices/trackdisk.i"
		INCLUDE	"devices/timer.i"
		INCLUDE	"devices/scsidisk.i"
		INCLUDE	"devices/hardblocks.i"

		INCLUDE	"modifiers.i"
		INCLUDE	"device.i"
		INCLUDE "iotask.i"
		INCLUDE "scsitask.i"
		INCLUDE	"board.i"
		INCLUDE	"printf.mac"

		INT_ABLES

		LIST

;		DEBUGENABLE

		XDEF	FindUnit,MakeUnit,MyMakeLibrary,Open
		XDEF	SendInternalCmd,WaitASecond
		XDEF	_WaitASecond,_WaitAWhile
		XDEF	MakeTempIOR,FreeTempIOR,ReCheckReady,ComputeBlockShift

		XREF	_LVOMakeLibrary,GetPubMem,_LVOFreeMem,_LVOFindTask
		XREF	_LVOPutMsg,_LVOWait,_LVOGetMsg,_LVOReplyMsg
		XREF	_LVOForbid,_LVOPermit

		XREF	_LVOOpenDevice,_LVOCloseDevice,_LVODoIO

		XREF	MountUnit,FindAndMount

		XREF	@add_unit,@remove_unit

	IFND IN_KICK
		XDEF	TimerName
	ENDC
	IFD IN_KICK
	  IFND V40
		XDEF	TimerName
	  ENDC
	ENDC

;==============================================================================
; I had to put the MakeLibrary call in here because Blink and Alink both screw
; up when trying to make a relocatable reference to LibFunctions from within
; another module (even though the section name is the same !!).  I think the
; problem is actually related to making relocatable references to XDEFd symbols
;==============================================================================
MyMakeLibrary	move.l	a2,-(sp)
		lea.l	LibFunctions(pc),a0	this many functions
		suba.l	a1,a1			I do my own initialisation
		suba.l	a2,a2			no libinit routine to call
		move.l	#HDDevice_SIZEOF,d0	size of lib+global data
		jsr	_LVOMakeLibrary(a6)	go make it
		move.l	(sp)+,a2
		rts

;============== table of standard device function calls ======================
LibFunctions	DC.W	-1			this says vecs are relative
		DC.W	Open-LibFunctions	open a unit
		DC.W	Close-LibFunctions	close a unit
		DC.W	Expunge-LibFunctions	expunge the device
		DC.W	Null-LibFunctions	dummy
		DC.W	BeginIO-LibFunctions	process an IORequest
		DC.W	AbortIO-LibFunctions	abort an IORequest
		DC.W	-1			that's all, no more, end!

;==============================================================================
; Success = Open( IORequest, Unit, Flags ),DevicePtr
;   d0		     a1	      d0    d1		a6
;
; Open routine grants access to a device.  The IO_DEVICE field has been inited
; by execs OpenDevice.  We should fill in Unit as appropriate.  If the open
; is successful then IO_ERROR should be 0 else put in a suitable error code.
;
; Flags have the following meanings:-
;
; -1	Open all units on this device.  Information for this is pulled from
;	the RigidDiskBlock structure which says which unit to check next.
;	A unit structure is allocated and linked into the list of known
;	units for this device.  Any partitions that have valid partition
;	information will have those partitions mounted and linked into the
;	DOS list of known volumes.  If this was an open call from Init then
;	one of the mounted partitions may be used as a boot device.
;	This is really just an internal call since it doesn't need an
;	IORequest pointer or a unit argument.
;
; 0	Just open the given unit if a unit structure is already in place
;	for it.  Else try to open the unit and initialise its bad block
;	info etc.  Does not try to mount partitions on the unit even if
;	there is partition information on the disk.  This is because a
;	flags value of 0 will generally be sent by the mount command.
;
; 1	Try to open this unit from nothing and also mount it's partitions
;	and add them to the DOS list.  This is provided for units that
;	were not present at boot time (or exceeded the timeout limit) and
;	are required now.  This could also be a mount type command.
;
; xt.device has units 0 and 1, that's all, no more, dammit! (only unit 0 now!!)
;
; scsi.device unit numbers are LUN*10 + physical unit number.  This allows us
; to access logical units (as found on Adaptec boards) using a single number.
;==============================================================================
Open		movem.l	d2-d3/a2/a5/a6,-(sp)
	printf <'In Open for unit %ld, flags %ld\n'>,d0,d1
		movea.l	a6,a5			device pointer to a5
		movea.l	hd_SysLib(a5),a6	exec.library to a6
		movea.l	a1,a2			stash the IORequest pointer
		move.l	d0,d2			and the unit number
		move.l	d1,d3			any flags ?
		bge.s	UserOpen		a normal call to open

; this is the first call to open, we'll attempt to initialise every disk device
		bsr	FindAndMount		find and mount all units
		bra.s	NoOpenDone		d0 contains #units mounted

; Normal call to open.  If the unit structure already exists then just stash
; the address in the IO_UNIT field of the IORequest.  If the unit is not on
; the list of known units, then try to access the physical device and create
; a unit node complete with bad block map information.  If the physical device
; does not respond, the device is broken or not connected, so return an error.
; Once the unit is opened, if flags = 1 we'll mount partitions on this unit too
UserOpen	move.l	d2,d0
		bsr.s	CheckUnit		see if the number is OK
		tst.l	d0			returns 0=ok
		bne.s	OpenError		nope, out of allowable range

		move.l	d2,d0
		bsr.s	FindUnit		see if we can find this unit
		move.l	d0,IO_UNIT(a2)		stash unit pointer
		bne.s	GotUnit			we found it first time
		move.l	d2,d0			we didn't find the unit
		moveq.l	#FALSE,d1		don't wait for ready
		bsr	MakeUnit		try to make it
		move.l	d0,IO_UNIT(a2)		stash constructed unit pointer
		bne.s	GotUnit			we made it first time
		moveq.l	#TDERR_BadUnitNum,d0	unit wasn't there, error out
		bra.s	OpenError

GotUnit		addq.w	#1,LIB_OPENCNT(a5)	add one more open call
		tst.l	d3			was this a normal open call ?
		beq.s	JustOpen		yep, just return good status

		movea.l	d0,a0			nope, attempt to mount parts
		bsr	MountUnit		we ignore returned flags here
		move.b	d1,IO_ERROR(a2)		but error is returned in d1
		bra.s	NoOpenDone

JustOpen	moveq.l	#0,d0			and return an OK error code
OpenError	move.b	d0,IO_ERROR(a2)		store error code in IORequest
NoOpenDone	movem.l	(sp)+,d2-d3/a2/a5/a6	flags -1 returns #units here
		rts

;==============================================================================
; bool = CheckUnit( number ),DeviceBase
;  d0		      d0	a5
;
; returns error or FALSE to indicate whether the unit# is valid for this device.
;==============================================================================
CheckUnit:
		; reject anything >= 100 with badboard
		cmp.w	#100,d0
		blt.s	Board_0			; < 100, must be board 0
		moveq.l	#HFERR_NoBoard,d0
		rts
Board_0
	IFD SCSI_SUPPORTED
	  IFD XT_SUPPORTED
		tst.w	hd_Type(a5)		testing XT or SCSI ?
		beq.s	CheckXT			it's an XT device
	  ENDC

		divu.w	#10,d0			get LUN
		tst.w	d0			lun must be in range 0-7
		bmi.s	BadNumber		it was <0
		cmpi.w	#7,d0
		bgt.s	BadNumber		it was >7

		swap	d0			now test physical unit
		tst.w	d0			must be in range 0-7
		bmi.s	BadNumber		it was <0
		cmpi.w	#7,d0
		bgt.s	BadNumber		it was >7
	ENDC
	IFD IS_IDE
		; accept units 0 and 1
		tst.l	d0
		beq.s	GoodNumber
	IFNE MAX_ADDR
		subq.l	#1,d0
	ENDC
		bne.s	BadNumber
	ENDC
GoodNumber	moveq.l	#0,d0			return FALSE (ok)
		rts

	IFD XT_SUPPORTED
; separate test for XT unit numbers, we only support XT unit 0 anyway
CheckXT		tst.l	d0			only unit 0 allowed
		beq.s	GoodNumber		fine, that's what was asked for
	ENDC

BadNumber	moveq.l	#TDERR_BadUnitNum,d0	return error code
		rts

;==============================================================================
; unit = FindUnit( number ),DeviceBase
;  d0		     d0		a5
;
; Attempt to locate the appropriate unit structure for the given unit number.
;==============================================================================
FindUnit    printf <'Trying to find unit %ld\n'>,d0
		movem.l	d2/a6,-(sp)
		divu.w	#10,d0			convert d0 to LUN
		move.l	d0,d1			and d1 to physical unit #
		swap	d1

		movem.l	d0/d1,-(sp)		save physical and logical units
		movea.l	hd_SysLib(a5),a6	using exec.library
		jsr	_LVOForbid(a6)		stop race conditions
		movem.l	(sp)+,d0/d1

		moveq	#0,d2			default to failure
		cmp.w	#MAX_ADDR,d1		bounds-check ID
		bhi.s	20$			d1 >= MAX_ADDR

		mulu.w	#ha_SIZEOF,d1		offset into Addr array
		lea.l	hd_HDUnits+ha_Units(a5,d1.w),a0	point to start of unit list
		move.l	MLH_HEAD(a0),d2		pre-fetch the first node
10$		movea.l	d2,a0			node pointer to a0
		move.l	LN_SUCC(a0),d2
		beq.s	20$			end of list, return 0
		cmp.b	hu_LUN(a0),d0		does logical unit match ?
		bne.s	10$			no, skip to the next
		move.l	a0,d2			node to d2

20$		jsr	_LVOPermit(a6)		race conditions over
		move.l	d2,d0			return unit ptr or 0 in d0
		movem.l	(sp)+,d2/a6
		rts

;==============================================================================
; data/ior = MakeTempIOR()
;    d0
;
; Builds a temporary SCSIDirect IOR with 256 bytes of data.  Returns pointer
; to data which is followed by the IOR.
;==============================================================================
MakeTempIOR:	movem.l	a3-a4,-(sp)
		move.l	#256+IOSTD_SIZE+scsi_SIZEOF+12+22,d0
		bsr	GetPubMem		get a general data area
		movea.l	d0,a4			a4 points to data buffer
		move.l	a4,d0
		beq.s	10$

		lea.l	256(a4),a3		a3 points to the IORequest
		lea.l	256+IOSTD_SIZE(a4),a0	a0 points to the SCSICmd
		lea.l	256+IOSTD_SIZE+scsi_SIZEOF(a4),a1
		move.l	a1,scsi_Command(a0)	stash command block
		lea.l	256+IOSTD_SIZE+scsi_SIZEOF+12(a4),a1
		move.l	a1,scsi_SenseData(a0)	stash sense data area
		move.l	a0,IO_DATA(a3)		initialise the IORequest
		move.l	#scsi_SIZEOF,IO_LENGTH(a3)
		move.w	#HD_SCSICMD,IO_COMMAND(a3)

		move.w	#6,scsi_CmdLength(a0)	initialise scsi command
		move.w	#22,scsi_SenseLength(a0)
		move.b	#SCSIF_AUTOSENSE!SCSIF_READ,scsi_Flags(a0)
		; d0 still == a4 here
10$		movem.l	(sp)+,a3-a4
		rts

;==============================================================================
; FreeTempIOR (data:a0)
;
; Free ior created by MakeTempIOR
;==============================================================================
FreeTempIOR:	move.l	a0,a1
		move.l	#256+IOSTD_SIZE+scsi_SIZEOF+12+22,d0
		jmp	_LVOFreeMem(a6)
		
;==============================================================================
; unit = MakeUnit( number,ready ),DeviceBase
;  d0		     d0	   d1	      a5
;
; Creates a unit structure and links it into the DeviceBase if the unit was
; really there.  This routine will first check what kind of device is on the
; bus and only attempts to find any info about it if it's a disk device of
; some kind.  Any other device type has a unit structure created for it but
; no info is obtained about that unit (apart from defaults for blocksize etc)
;
; the ready flag in d1 will modify the conditions for a successful open.  If
; ready is true, then this routine will wait until the drive is ready (with
; a timeout) and fail if the timeout is exceeded.  If ready is false, then
; the open will succeed as long as the device responded to selection.
;
; 	  VERY IMPORTANT!  THIS CODE MUST BE COMPLETELY RE-ENTRANT!
;==============================================================================
MakeUnit	movem.l	d2-d3/a2-a4/a6,-(sp)
	printf <'Making unit %ld, ready=%ld\n'>,d0,d1
		movea.l	hd_SysLib(a5),a6
		move.l	d0,d2			stash the unit number
		move.l	d1,d3			and the ready wait flag
		suba.l	a2,a2			clear unit pointer for error exit

		bsr	MakeTempIOR ^
		tst.l	d0
		beq	MUErrorExit
		move.l	d0,a4			data area - 256 bytes
		lea	256(a4),a3		IOR - already set up

;initialise as much of the unit structure as is required for SCSITask to use it
		move.l	#HDUnit_SIZEOF,d0	fake up a unit structure
		bsr	GetPubMem		cleared!
		move.l	d0,IO_UNIT(a3)		and save in our IORequest
		beq	MakeUnitFailed		alloc failed, return an error
		movea.l	d0,a2			stash unit ptr for later

	IFD SCSI_SUPPORTED
		; at-ide doesn't do luns...
		divu.w	#10,d2			construct unit and LUN numbers
		move.b	d2,hu_LUN(a2)		(ignored for XT drives)
		swap	d2
	ENDC
	IFD IS_IDE
		moveq.b	#$A0,d0			Must be $a0- 512-byte sectors
	ENDC
		move.b	d2,hu_Unit(a2)		careful, sets cc's!
	IFD IS_IDE
		beq.s	unit_zero		careful, uses cc's!
		bset.b	#4,d0			this is the slave drive
unit_zero:
		move.b	d0,hu_ATDriveBit(a2)
	ENDC
		mulu.w	#ha_SIZEOF,d2		offset into hd_HDUnits
		lea.l	hd_HDUnits(a5,d2.w),a0	point to start of addr struct
		move.l	a0,hu_Addr(a2)		to get to shared per-addr stuff

	IFD SCSI_SUPPORTED
		; don't allow scsitask to start a sync negotiation until
		; we've done an inquiry command
		move.b	#3,ha_DoneSync(a0)	stops it from trying Sync

		; default to allowing reselection
		move.b	#$40,ha_Reselect(a0)	a mask for the WD chip
	ENDC

	; hu_Type is clear by default (= xt/at drive)
	IFD XT_SUPPORTED
		move.b	hd_Type+1(a5),hu_Type(a2) type flag for IOTask/SCSITask
	ENDC
		move.l	#512,hu_BlockSize(a2)	assume 512 byte blocks

	IFND IS_IDE
; This is pretty silly - both set it to 4095.  I suppose that's fine by me.

; fix when we have to use terminal count.  Maximum read/write extended = 2Meg
	IFND DMA_FLUSH_OK
		move.l	#4095,hu_MaxBlocks(a2)	assume extended r/w works
	ENDC DMA_FLUSH_OK

; it turns out that we MUST use terminal count to allow the flush trickery
; to work properly.  It is no longer OK to allow unlimited xfers (65535 blocks)
	IFD DMA_FLUSH_OK
		move.l	#4095,hu_MaxBlocks(a2)	assume extended r/w works
	ENDC DMA_FLUSH_OK
	ENDC !IS_IDE

	IFD IS_IDE
		; we can handle extended r/w
		move.l	#$ffff,hu_MaxBlocks(a2)
	ENDC
		move.w	#9,hu_BlockShift(a2)	for fast divide by 512
		lea.l	hu_CmdList(a2),a0	empty command list
		NEWLIST	a0
		lea.l	hu_ChangeList(a2),a0	empty AddChangeInt list
		NEWLIST	a0

; assume a disk is in the drive and not write protected
; not needed, allocated clear
;		clr.l	hu_ChangeState(a2)
;		clr.b	hu_Protected(a2)
;		clr.l	hu_RemoveAddress(a2)	no Cause address
;		clr.b	hu_TagsAllowed(a2)
;		clr.b	hu_NoDisconnect(a2)

; if this is an XT drive then we need to set up some disk geometry values too
	IFD XT_OR_AT_IDE
	  IFD SCSI_SUPPORTED
		tst.w	hd_Type(a5)		XT or SCSI ?
		bne.s	WaitNotBusy		SCSI, don't need geometry
	  ENDC
		; default values for a 20 Meg drive
; FIX!  should AT default to 40 Meg drive?
		move.l	#612,hu_TotalCyls(a2)
		move.b	#4,hu_TotalHeads(a2)
		move.w	#17,hu_TotalBlocks(a2)
		move.w	#17*4,hu_CylSize(a2)
	ENDC

	IFD NCR53C710
		; ncr must add unit to scripts before using
		move.l	hd_STGlobals(a5),a0	; scsi globals
		move.l	a2,a1			; unit
		bsr	@add_unit
	ENDC

; First, we have to determine if the unit is even hooked up.  The simplest
; way to do this for SCSI devices is to send the INQUIRY command (to see
; what's hooked up) and check if we got a timeout condition on selection.
WaitNotBusy:
	printf <'Trying an Inquiry...\n'>
		movea.l	IO_DATA(a3),a0		point to SCSICmd
		movea.l	scsi_Command(a0),a1	and the command block
		move.l	#$12000000,(a1)		inquiry command
		move.w	#$fe00,4(a1)		inquiry length = 254 bytes
		move.l	#254,scsi_Length(a0)	we're expecting this much data
		move.l	a4,scsi_Data(a0)	put it in this memory
		movea.l	a3,a0
		bsr	SendInternalCmd		and send this command

; command was sent, but we have to determine if there was a unit hooked up
		tst.l	d0			did everything complete ?
		beq.s	InquiryWorked		yep, no problems
	printf <'Inquiry failed, status = $%lx\n'>,d0
; if selection timed out, no point in trying anything else - REJ
; or if any other error than bad status...
		bmi	NoUnitThere

; the only status we intend to do anything about is a check condition status
; or a busy status (most likely from the stupid Seagate drives)
; no longer needed - SendInternalCmd does this
;		cmpi.b	#BUSY,d0			drive busy ?
;		bne.s	20$			nope, maybe check condition
;		bsr	WaitASecond		delay a little while
;		dbra	d1,waitloop		limited wait for not busy
;
20$		cmpi.b	#CHECK_CONDITION,d0	did we get a check condition
		bne	NoUnitThere		nope, fail the Makeunit()

; And the only error we're interested in is illegal command (assume a disk)
		swap	d1			want the sense code..
		cmp.w	#ILLEGAL_COMMAND,d1	sense code of illegal cmd
		beq	CheckReady		yep, assume it's a disk ^
		bra	NoUnitThere		something really wrong

InquiryWorked
	IFD DEBUG_CODE
	moveq	#0,d0
	move.b	(a4),d0
	printf <'inquiry worked, type = %ld\n'>,d0
	ENDC
		move.b	(a4),hu_SCSIType(a2)	store type ...
; FIX! mis-named
		move.b	1(a4),hu_SCSIQual(a2)	... and qualifier
		move.b	hu_SCSIType(a2),d0	is this a disk unit ?
		cmpi.b	#$7f,d0			get 7f if the LUN was bad
		beq.s	NoUnitThere		yep, invalid LUN

; from here on we _know_ there's a device out there.  Try to get things
; set up, but don't yank the unit on any failure.

		btst.b	#4,7(a4)		check the sync bit
		beq.s	no_sync_allowed
		move.l	hu_Addr(a2),a0
		clr.b	ha_DoneSync(a0)		will try sync on the next cmd
		move.b	#1,ha_SupportsSync(a0)
no_sync_allowed:
		btst.b	#1,7(a4)		check the CmdQue bit
		beq.s	no_tags_allowed
		move.l	hd_STGlobals(a5),a0	get scsitask global ptr
		tst.b	st_UseTags(a0)		are tags allowed? dip/battmem
		beq.s	no_tags_allowed
		move.b	#1,hu_TagsAllowed(a2)	goody, use tagged queuing!
no_tags_allowed:
		andi.b	#$7f,d0			mask off removable media bit
		beq.s	CheckReady		it's a hard drive

		cmpi.b	#$04,d0			WORM we'll support opticals too
		beq.s	CheckReady
		cmpi.b	#$05,d0			CDROM
		bne.s	10$
		move.l	#2048,hu_BlockSize(a2)	for CDROM, default to 2048 byte
		move.w	#11,hu_BlockShift(a2)	blocks, in case there's no disk
		bra.s	CheckReady		in the drive
10$		cmpi.b	#$07,d0			Optical (disk)
		bne.s	UnitReady		must be tape/cpu/printer/etc

CheckReady
	printf <'CheckReady\n'>
		move.b	#1,hu_IsDisk(a2)	it can accept disk commands

		; do all the checking of ready, blocksize, WP, etc.
		bsr	ReCheckReady	(a2,a3,a4)
		bra.s	UnitReady		all ready to start

; Either the unit wasn't there or it failed one of the SCSI phases, free up the
; unit structure and return zero to indicate that the MakeUnit call failed.
NoUnitThere
	IFD NCR53C710
		; ncr must remove unit from scripts before freeing
		move.l	hd_STGlobals(a5),a0	; scsi globals
		move.l	a2,a1			; unit
		bsr	@remove_unit
; FIX? what if ncr is in the reselect when it's removed and freed?
; Impossible in reality (only 1 instruction is in the loop), so we're safe.
	ENDC
		movea.l	a2,a1			this Unit structure
		move.l	#HDUnit_SIZEOF,d0	this size
		jsr	_LVOFreeMem(a6)
		suba.l	a2,a2			this will be returned to caller
		bra.s	MakeUnitFailed

; The unit we want to create is up and running.  Add it to list of known units.
UnitReady
	printf <'unit ready\n'>
		jsr	_LVOForbid(a6)		stop race conditions
		moveq	#0,d0
		move.b	hu_Unit(a2),d0
		mulu.w	#ha_SIZEOF,d0		offset into hd_HDUnits array

		; if there is only one other unit on the unit list, set it
		; to allow reselect again.  If no units, remove reselect from
		; this one.
		cmp.b	#1,hd_NumUnits(a5)	how many units in use?
		bhi.s	add_it			already more than 1
		beq.s	not_empty		exactly one, reenable sync

		; no units yet - turn off reselection
		clr.b	ha_Reselect+ha_Units+hd_HDUnits(a5,d0.w)
		bra.s	add_it			that was the only unit

not_empty:	; one unit out there - find it and re-enable disconnect for it
		lea.l	hd_HDUnits+ha_Units(a5),a0	; start of loop
		lea.l	hd_HDUnits+(ha_SIZEOF*(MAX_ADDR+1))+ha_Units(a5),a1 ; end
5$		IFNOTEMPTY a0,10$
		lea	ha_SIZEOF(a0),a0	next addr
		cmp.l	a0,a1
		beq.s	add_it			didn't find it????
		bra.s	5$

10$		; found it - re-enable reselect for this address
		move.b	#$40,ha_Reselect-ha_Units(a0)	; a0 points to ha_Units
		
add_it:		movea.l	a2,a1
		addq.b	#1,hd_NumUnits(a5)	bump number of units in use
		lea	hd_HDUnits+ha_Units(a5,d0.w),a0
		ADDHEAD
		jsr	_LVOPermit(a6)

; everything has been done for this unit, free up any resources we used
MakeUnitFailed	movea.l	a4,a0			free buffer mem we used
		bsr	FreeTempIOR

MUErrorExit	move.l	a2,d0			return address of unit struct
		movem.l	(sp)+,d2-d3/a2-a4/a6
		rts

;==============================================================================
; ready = ReCheckReady (wait, unit, ior, data)
;   d0			 D3    A2   A3    A4
;
; The code following here should be executed before any CMD_READ
; or CMD_WRITE access to a disk that has never managed to come ready.  This is
; important since it's the only way to safely get the sector size, disk size,
; and write-protect/etc info.
;
; This is a hard drive or a worm drive, so we're going to attempt to read it.
; Before we do, we must check if media is accessible (and drive is spinning).
; this is taken from the SCSI-2 spec, Appendix F - Verify State Test
;==============================================================================
ReCheckReady	bsr	VerifyState		returns -1,0,1
		tst.l	d0
		bmi	ReCheckNoUnit		-1 is failed unit
		bne.s	GetBlockSize		Success - it's ready.

		; not ready - really
		tst.l	d3			are we waiting for this unit
		beq.s	GetBlockSize

		; unit isn't ready.  Attempt to start it.
retry_start	movea.l	IO_DATA(a3),a0		get SCSICmd
		movea.l	scsi_Command(a0),a1	and the command block
		clr.l	(a1)
		move.w	#$0100,4(a1)		Start=1
		move.b	#START_STOP_UNIT,(a1)	we want to get the block size
		clr.l	scsi_Length(a0)		not expecting a data phase
		movea.l	a3,a0
		bsr	SendInternalCmd		and send this command
		tst.l	d0
		beq.s	CheckReady2
		bmi	ReCheckNoUnit		not a bad status error
		cmpi.b	#CHECK_CONDITION,d0	was it a CHECK CONDITION ?
		bne	ReCheckNoUnit		no - and busy already retried

		; only allowable error is sense key of illegal request or
		; sense code of illegal command
		cmp.w	#ILLEGAL_REQUEST,d1
		beq.s	CheckReady2		try one more time
		swap	d1			check the sense code too
		cmp.w	#ILLEGAL_COMMAND,d1
		bne	ReCheckNoUnit		not illegal command

CheckReady2	bsr	VerifyState		returns -1,0,1
		tst.l	d0			not ready is now failure...
		ble	ReCheckNoUnit		-1 is failed unit
		; ready, finally - fall through

; Everything is ready for us now, find out the real blocksize on this unit
GetBlockSize	movea.l	IO_DATA(a3),a0		get SCSICmd
		movea.l	scsi_Command(a0),a1	and the command block
		clr.l	(a1)
		clr.l	4(a1)
		clr.w	8(a1)			this is a 10 byte CDB
		move.b	#READ_CAPACITY,(a1)	we want to get the block size
		move.w	#10,scsi_CmdLength(a0)
		move.l	a4,scsi_Data(a0)	read data to here
		move.l	#8,scsi_Length(a0)	returns 8 bytes of info
		movea.l	a3,a0
		bsr	SendInternalCmd		and send this command

		tst.l	d0			did everything work OK
		bgt.s	GetWriteProtect		bad status
		bmi	ReCheckNoUnit		other err, assume total failure
		movea.l	IO_DATA(a3),a0		get the SCSICmd
		cmpi.l	#8,scsi_Actual(a0)	did we get the data ?
		bne.s	GetWriteProtect		no, assume defaults are OK

		move.l	4(a4),d0		fetch blocksize
		bsr	ComputeBlockShift
		move.w	d1,hu_BlockShift(a2)	save the shift count
		move.l	d0,hu_BlockSize(a2)	and block size
		move.l	(a4),hu_DiskSize(a2)	and # of blocks on the disk

; new addition, see if the disk is write protected and set up flag accordingly
GetWriteProtect	movea.l	IO_DATA(a3),a0		get SCSICmd
		movea.l	scsi_Command(a0),a1	and the command block
		clr.l	(a1)
		clr.w	4(a1)			this is a 6 byte CDB
		move.b	#MODE_SENSE,(a1)	we want the write protect bit
		move.b	#4,2(a1)		page 4 (Rigid Disk Geometry)
		move.b	#254,4(a1)		so just need MS header
		move.w	#6,scsi_CmdLength(a0)
		move.l	a4,scsi_Data(a0)	read data to here
		move.l	#254,scsi_Length(a0)
		movea.l	a3,a0
		bsr	SendInternalCmd		and send this command ^

		tst.l	d0			did everything work OK
		bne.s	ReCheckOK		assume not WPed
		movea.l	IO_DATA(a3),a0		get the SCSICmd
		cmpi.l	#4,scsi_Actual(a0)	did we get the data ?
		blt.s	ReCheckOK		no, assume defaults are OK
		tst.b	2(a4)			get write protect bit 
		bpl.s	1$
		move.b	#-1,hu_Protected(a2)	it's write protected

1$:		; now set up a few more fields
	IFD SCSI_SUPPORTED
	    IFD GET_GEOMETRY
	      IFD XT_OR_AT_IDE
		tst.b	hu_Type(a2)		make sure it's SCSI (xt/at
		beq.s	ReCheckOK		 already set hu_...)
	      ENDC
		moveq	#4,d0			Length of MS header
		add.b	3(a4),d0		Block Descriptor Length
		lea	0(a4,d0.w),a1		pointer to page structure
		addq.w	#4,d0			top word is 0
		cmp.l	scsi_Actual(a0),d0	did we get the data ?
		bgt.s	ReCheckOK		no, assume defaults are OK

		; ugly, but gets us in the ballpark
		moveq	#0,d0			may not be aligned
		move.b	2(a1),d0
		lsl.w	#8,d0
		move.b	3(a1),d0
		lsl.l	#8,d0
		move.b	4(a1),d0
		move.l	d0,hu_TotalCyls(a2)
		move.l	hu_DiskSize(a2),d1
		divu.w	d0,d1			total sectors/cyls
		move.w	d1,hu_TotalBlocks(a2)	assumes heads = 1
	    ENDC
	ENDC

ReCheckOK:	moveq	#1,d0
		move.b	#1,hu_MountDone(a2)	Unit has come ready
		rts
ReCheckNoUnit:	moveq	#0,d0
		rts

;==============================================================================
; blocksize, blockshift = ComputeBlockShift(blocksize)
;    d0          d1                            d0
;==============================================================================
ComputeBlockShift
		move.l	d2,-(sp)
		move.l	d0,d2
5$		moveq.l	#1,d0			figure out the shift value too
		moveq.l	#0,d1			for converting bytes to blocks
10$		cmp.w	d0,d2			blocksize match ?
		beq.s	20$			yep, d2 holds shift count
		addq.l	#1,d1			add to shift count
		lsl.w	#1,d0			shift compare value
		bcc.s	10$			until the bit drops out
; we obviously have a totally bogus block size, so rather than using the default
; 512 byte blocks, assume we have an optical drive (many fail this) and use 2048
		move.l	#2048,d2
		bra.s	5$
20$		move.l	d2,d0			returned
		move.l	(sp)+,d2
		rts

;==============================================================================
; state:D0 & cc's = VerifyState (unit:a2, ior:a3)
;
; If we get BUSY or reservation conflict, retry until not busy (poll)
; state can be 0 (Not Ready), 1 (ready), 2 (Busy), or -1 (failed unit)
;==============================================================================
VerifyState:	bsr.s	TestUnitReady		returns -1,0,1,2
		bne.s	return_state

		; not ready - second try
		bsr.s	TestUnitReady		returns -1,0,1,2
		bne.s	return_state
		
		; not ready - third and final try
		bsr.s	TestUnitReady		returns -1,0,1,2
return_state:	rts

;==============================================================================
; state:D0 & cc's = TestUnitReady (unit:a2, ior:a3)
; 
; This is a hard drive or a worm drive, so we're going to attempt to read it.
; Before we do, we must check if media is accessible (and drive is spinning).
; state can be 0 (Not Ready), 1 (ready), 2 (Busy), or -1 (failed unit)
;==============================================================================
TestUnitReady
	printf <'TestUnitReady\n'>
		movea.l	IO_DATA(a3),a0		get SCSICmd structure
		movea.l	scsi_Command(a0),a1	and the actual command block
		clr.l	(a1)			clear out the command block
		clr.w	4(a1)			Test unit ready is 0 anyway
		clr.l	scsi_Length(a0)		not expecting a data phase
		movea.l	a3,a0
		bsr.s	SendInternalCmd		and send this command
		tst.l	d0			status (-1.l for funny error)
		bne.s	tur_has_err
TUR_Read	moveq	#1,d0			it's ready!
		rts

tur_has_err:	bmi.s	TUR_Failed		not bad status
		cmpi.b	#CHECK_CONDITION,d0	was it a CHECK CONDITION ?
		bne.s	TUR_Failed		no, and busy already retried

;if a check condition status occured, request sense will have been done already
		cmpi.b	#$06,d1			unit attention (just reset) ?
		beq.s	TUR_NotReady		yep, so just check again
		cmpi.b	#$02,d1			was this unit not ready ?
		bne.s	TUR_Failed		no, something else
TUR_NotReady:	moveq	#0,d0			drive not ready yet
		rts
TUR_Failed:	moveq.l	#-1,d0			no, so cop out now
		rts

;==============================================================================
; status,sensekey = SendInternalCmd( IORequest ),DeviceBase
;    d0	    d1			  a0		a5
;
; Sends an initialised IORequest to the main IOtask for processing.  We have
; to allocate and initialise the ReplyPort with the correct sigtask because
; we can be called by many different tasks at a time (hence the need for the
; re-entrancy in this code).
;
; On Busy it will retry the command at 1 second intervals for up to 30 seconds
; On TDERR_DiskChanged it will return immediately.
;==============================================================================
SendInternalCmd	movem.l	d2/a2/a6,-(sp)
		movea.l	hd_SysLib(a5),a6	using exec library
		move.l	a0,a2			stash IORequest
		move.l	#30,d2			max retries

		lea.l	-MP_SIZE(sp),sp		message port on the stack
		move.l	sp,MN_REPLYPORT(a2)	tag where IORequest replies to
		move.l	a5,IO_DEVICE(a2)	** not really needed, but....

		move.b	#NT_MSGPORT,LN_TYPE(sp)	set up the node type
		clr.b	LN_PRI(sp)
		clr.l	LN_NAME(sp)		no name
		clr.b	MP_FLAGS(sp)

		move.b	#SIGB_SINGLE,MP_SIGBIT(sp)

		suba.l	a1,a1			find this task
		jsr	_LVOFindTask(a6)	a6 is always exec.library
		move.l	d0,MP_SIGTASK(sp)	and fill in this task
		lea.l	MP_MSGLIST(sp),a0
		NEWLIST	a0			must initialise message port

retry_cmd:	; make sure it's a SCSI command...  Some are CMD_READ, etc
		cmp.w	#HD_SCSICMD,IO_COMMAND(a2)
		bne.s	10$
		move.l	IO_DATA(a2),a0		get the scsicmd
		movea.l	scsi_SenseData(a0),a0	get the sense data
		clr.b	12(a0)			default to no sense code

10$		movea.l	a2,a1			get back the IORequest
		movea.l	hd_IOPort(a5),a0	where we put it to
	printf <'Sending internal command to port 0x%lx\n'>,a0
		jsr	_LVOPutMsg(a6)		send the request

ReFetch		moveq.l	#SIGF_SINGLE,d0		wait for it to return
		jsr	_LVOWait(a6)
		movea.l	sp,a0			fetch the returned IORequest
		jsr	_LVOGetMsg(a6)		it better be there now !!

	IFD DEBUG_CODE
	tst.l d0
	bne.s ItsOK
	printf <'SIGF_SINGLE misuse !!!\n'>
	ENDC
ItsOK		tst.l	d0
		beq.s	ReFetch			SIGF_SINGLE was already set

		; get the result
		moveq	#0,d0			default to success
		moveq	#0,d1
		move.b	IO_ERROR(a2),d0
		beq	all_done		no error (d0 = 0) ^
	printf <'SendInternal error: %ld'>,d0
		cmp.b	#HFERR_BadStatus,d0
		bne.s	not_bad_status		return -1

		; make sure it's a SCSI command...  Some are CMD_READ, etc
		cmp.w	#HD_SCSICMD,IO_COMMAND(a2)
		bne.s	all_done

		movea.l	IO_DATA(a2),a0		fetch SCSICmd
		move.b	scsi_Status(a0),d0	see what the target unit said
	printf <'SendInternal status: %ld'>,d0
		beq.s	all_done		wierd... d0 = 0
		cmpi.b	#BUSY,d0		was the drive busy ?
		beq.s	10$			yup, try again later
		cmpi.b	#RESERVATION_CONFLICT,d0
		bne.s	other_error
10$		subq.b	#1,d2			retry up to 30 times
		beq.s	other_error
		bsr	WaitASecond
		bra	retry_cmd ^

other_error:	; d0 = status, d1 = 0, a0 = scsicmd
		tst.w	scsi_SenseActual(a0)	did we get sense data ?
		beq.s	all_done		no, something drastically wrong
		movea.l	scsi_SenseData(a0),a0	yep, point to the sense info
		move.b	(a0),d1			get sense code
		andi.b	#$7f,d1			mask off address valid bit
		cmpi.b	#$70,d1			is this extended sense ?
		bne.s	SmallSense		nope, we got 4 bytes
		move.b	12(a0),d1		get sense code
		swap	d1			into upper word
		move.b	2(a0),d1		get the sense key
		bra.s	all_done

SmallSense	cmpi.b	#$04,d1			drive not ready ?
		bne.s	10$
		or.l	#$00020000,d1		not ready sense key
10$		swap	d1			key in low word, code in high
		bra.s	all_done

not_bad_status	moveq	#-1,d0			something other than bad stat

all_done:	lea.l	MP_SIZE(sp),sp		reclaim stack space

		movem.l	(sp)+,d2/a2/a6		all done
		rts

;==============================================================================
; CloseDevice( IORequest ),DevicePtr
;		   a1	      a6
;
; Pretty simple this one, just stop using the device. (Must return 0)
;==============================================================================
Close		subq.w	#1,LIB_OPENCNT(a6)	one less user
;		beq.s	Expunge			if none, expunge the device
;		rts				some users left

;==============================================================================
; Expunge(),DevPtr
;	     a6
;
; This one doesn't do anything (same as Null and AbortIO)
;==============================================================================
Expunge:
AbortIO:
Null		moveq.l	#0,d0
		rts

;==============================================================================
; BeginIO( IORequest ),DevPtr
;	      a1	a6
;
; This is where all device commands come in.  If the quick flag is set and the
; command can be completed quickly, then it is processed and not replied.  If
; quick is not set or quick is set and the command can't be completed quickly
; then the IORequest is sent to the main SCSI task which will reply it when
; it's done.  We always check to see if this command is to a disk unit.  If it
; isn't then we'll only allow a SCSIDirect command to be sent.
;==============================================================================
BeginIO		movem.l	a5/a6,-(sp)
		movea.l	a6,a5			stash DevBase
		movea.l	hd_SysLib(a5),a6	will need exec library
		move.w	IO_COMMAND(a1),d0
		bmi.s	CmdBad			negative commands not allowed
		cmpi.w	#MAX_CMD,d0		is it a valid command ?
		bgt.s	CmdBad			no, return an error
		movea.l	IO_UNIT(a1),a0		are we accessing a disk unit
		tst.b	hu_IsDisk(a0)
		bne.s	CommandOK		yes, so any command is good
		cmpi.w	#HD_SCSICMD,d0		only SCSIDirect to other devs
		beq.s	CommandOK
CmdBad		moveq.l	#0,d0			default to bad command

CommandOK	lsl.w	#1,d0			index to table of word offsets
		jsr	IOTable(pc,d0.w)	call the right routine
		movem.l	(sp)+,a5/a6		and that's all
		rts

;==============================================================================
; This is a table that determines which things can be done quickly and which
; things must be deferred to the main SCSI task.  I've left them in table form
; like this so that I can change the various routines easily and don't have a
; limit of 32 commands like device drivers that use the bitfield technique for
; determining quick commands.  Things marked 'q' in this driver just return 0
; in the IO_ERROR field.  This works out OK for all non hard disk commands.
;==============================================================================
IOTable		bra.s	BadIO		0  invalid	q
; FIX! we can have reset
		bra.s	NoIO		1  reset	q
		bra.s	SendToTask	2  read
		bra.s	SendToTask	3  write
		bra.s	SendToTask	4  update	q
		bra.s	SNoIO		5  clear	q
		bra.s	SendToTask	6  stop
		bra.s	SendToTask	7  start
		bra.s	SNoIO		8  flush	q
		bra.s	SNoIO		9  motor	q
		bra.s	SendToTask	10 seek
		bra.s	SendToTask	11 format
		bra.s	SendToTask	12 remove
		bra.s	NoIO		13 changenum	q
		bra.s	ChangeState	14 changestate	q
		bra.s	SendToTask	15 protstatus
		bra.s	BadIO		16 rawread	q
		bra.s	BadIO		17 rawwrite	q
		bra.s	BadIO		18 getdrivetype q
		bra.s	BadIO		19 getnumtracks q
		bra.s	SendToTask	20 addchangeint	q
		bra.s	RemChangeInt	21 remchangeint q Must be immediate
	IFND GET_GEOMETRY
		bra.s	BadIO		22 getgeometry lastcomm	q
	ENDC
	IFD GET_GEOMETRY
		bra.s	SendToTask	22 getgeometry lastcomm	q
	ENDC
		bra.s	SendToTask	23 eject
		bra.s	BadIO		24
		bra.s	BadIO		25
		bra.s	BadIO		26
		bra.s	BadIO		27
		bra.s	SendToTask	28 scsicmd

;==============================================================================
; SNoIO( IORequest )
;	    a1
;
; This is a kluge fix for commands that are not quick on other devices but are
; quick on this one.  We make like it was a slow operation instead.
;==============================================================================
SNoIO		clr.b	IO_FLAGS(a1)
		bra.s	NoIO

;==============================================================================
; BadIO( IORequest )
;	    a1
; This routine called through IOTable to send back IORequest for invalid cmd
;==============================================================================
BadIO		move.b	#-1,IO_ERROR(a1)
		bra.s	NoIO2			skip to common exit

;==============================================================================
; NoIO( IORequest )
;	    a1
; This routine called through the IOTable to send the IORequest straight back
;==============================================================================
NoIO		clr.b	IO_ERROR(a1)		no error
NoIO2		clr.l	IO_ACTUAL(a1)		no return code
QuickIO		btst.b	#IOB_QUICK,IO_FLAGS(a1)	was this quick IO ?
		bne.s	NoIOExit		yes, so no need to reply
		jmp	_LVOReplyMsg(a6)	no, so reply to caller
NoIOExit	rts

;==============================================================================
; SendToTask( IORequest )
;		  a1
; This routine is called through the IOTable to send Request to the SCSI task
;==============================================================================
SendToTask	clr.b	IO_FLAGS(a1)		this is definitely not quick
		clr.b	IO_ERROR(a1)		no errors (yet)
		clr.l	IO_ACTUAL(a1)		and no data transferred
		movea.l	hd_IOPort(a5),a0	send message to SCSI task
		jmp	_LVOPutMsg(a6)		jsr/rts

;==============================================================================
; ChangeState( IORequest )
;		  a1
;
; Returns 0 if a disk is in the drive or -1 if the drive is empty.
;==============================================================================
ChangeState	movea.l	IO_UNIT(a1),a0		get unit pointer
		move.l	hu_ChangeState(a0),IO_ACTUAL(a1)
	printf <'ChangeState returned %ld'>,hu_ChangeState(a0)
OkReply		clr.b	IO_ERROR(a1)
		bra.s	QuickIO

;==============================================================================
; RemChangeInt( IORequest ), sysbase
;		  a1		a6
;
; Reuses an IOR used for AddChangeInt.  MUST be handled as immediate,
; whether requested or not.  Make sure we reply if quick wasn't requested!
;==============================================================================
RemChangeInt	move.l	a1,-(sp)	; save ior
		FORBID
		REMOVE			; removes a1 from list
		PERMIT
		move.l	(sp)+,a1
		bra.s	OkReply		; clear io_Error and return/reply

;==============================================================================
; WaitASecond(),sysbase
;		   a6
;
; wait one second...
;==============================================================================
_WaitASecond:
WaitASecond	moveq	#1,d0
		moveq	#0,d1
		; fall through...

;==============================================================================
; WaitAWhile(secs,micros),sysbase
;	      d0    d1      a6
;
; wait a short period of time (in microseconds)
; Support routine so we don't just busy spin the drives when we are waiting for
; them to come up.  If something re-tryable fails, WaitASec before retrying.  I
; allocate the IORequest and message port on the fly so this can be called from
; any of the tasks associated with this driver.
;==============================================================================
_WaitAWhile:	movem.l	d2-d3/a2-a3,-(sp)
		move.l	d0,d2			save secs
		move.l	d1,d3
		lea.l	-MP_SIZE(sp),sp		message port on the stack
		movea.l	sp,a2

		move.b	#NT_MSGPORT,LN_TYPE(a2)	set up the node type
		clr.b	LN_PRI(a2)
		clr.l	LN_NAME(a2)		no name
		clr.b	MP_FLAGS(a2)
		move.b	#SIGB_SINGLE,MP_SIGBIT(a2)
		suba.l	a1,a1			find this task
		jsr	_LVOFindTask(a6)	a6 is always exec.library
		move.l	d0,MP_SIGTASK(a2)	and fill in this task
		lea.l	MP_MSGLIST(a2),a0
		NEWLIST	a0			must initialise message port

		lea.l	-IOTV_SIZE(sp),sp	get a timer request
		movea.l	sp,a3

	IFND IN_KICK
		lea.l	TimerName(pc),a0	open this device (timer.device)
	ENDC
	IFD IN_KICK
	  IFD V40
		; V40.5 exec uses 0 for timer.device - internal/librarytags.h
		sub.l	a0,a0			open this device (timer.device)
	  ENDC
	  IFND V40
		lea.l	TimerName(pc),a0	open this device (timer.device)
	  ENDC
	ENDC
		movea.l	a3,a1			for this IORequest
		moveq.l	#UNIT_VBLANK,d0		this unit
		cmp.l	#40000,d2		is it really short?
		bcc.s	10$			no
		moveq.l	#UNIT_MICROHZ,d0	this unit
10$		moveq.l	#0,d1			no special flags
		jsr	_LVOOpenDevice(a6)
		tst.l	d0			any errors ?
		bne.s	TimerBad		yep, so exit now

		move.l	a2,MN_REPLYPORT(a3)	IORequest replies to this port
		
		movea.l	a3,a1
		move.w	#TR_ADDREQUEST,IO_COMMAND(a1)
		move.l	d2,IOTV_TIME+TV_SECS(a1)
		move.l	d3,IOTV_TIME+TV_MICRO(a1)
		jsr	_LVODoIO(a6)
		movea.l	a3,a1
		jsr	_LVOCloseDevice(a6)

TimerBad	lea.l	IOTV_SIZE+MP_SIZE(sp),sp	clean up stack
		movem.l	(sp)+,d2-d3/a2-a3
		rts

	IFND IN_KICK
TimerName	DC.B	'timer.device',0
		CNOP	0,2
	ENDC
	IFD IN_KICK
	  IFND V40
TimerName	DC.B	'timer.device',0
		CNOP	0,2
	  ENDC
	ENDC

		END

