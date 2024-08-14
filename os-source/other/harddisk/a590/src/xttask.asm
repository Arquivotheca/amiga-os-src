		SECTION	driver,CODE
;==============================================================================
; This code is for handling XT drives in the SCSITask environment.  It is not
; a separate task in its own right.  All XT drives are sent SCSI command blocks
; which are interpreted here and sent to the drive hardware as XT commands.
;==============================================================================

		NOLIST
		INCLUDE	"modifiers.i"
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/interrupts.i"
		INCLUDE	"exec/lists.i"
		INCLUDE	"exec/ports.i"
		INCLUDE	"exec/tasks.i"
		INCLUDE	"device.i"
		INCLUDE	"scsitask.i"
		INCLUDE	"board.i"
		INCLUDE	"scsidirect.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XDEF	XTSelect,XTSendCommand,XTAddressBad,XTCommandDone

		XREF	FakeSelect,FakeDisconnect,FakeTimeout

;==============================================================================
; Although this is called XTSelect, it is actually responsible for starting
; the given command (in the SCSIDirect command attached to the command block)
; Read, write and seek commands use the address translation feature of the WDC
; since we have one on the board anyway.  These will re-enter in this module
; at XTSendCommand when the WDC interrupts us to say it has done translation.
;
; All fields of the scsi_Command are used and maintained as if we were talking
; to a SCSI device.  That is, scsi_Actual is updated accordingly and the
; target status is set to check condition ($02) if we get some kind of error.
; scsi_CmdActual will always be set equal to scsi_CmdLength.  XT units will
; only return non-extended sense data for now (won't comply with SCSI-2) but
; fortunately for me, I can use the 4 bytes returned by an XT drive.  The
; important error codes all map directly to SCSI non-extended sense.
;
; When a command completes, the 4 error bytes are read into hu_NextError for
; use by the request sense command which just copies them to the sense data
; area and then clears the error condition in hu_NextError.
;
; FakeSelect call will set up SCSI working pointers which we can use here.
;==============================================================================
XTSelect	move.b	XTPORT0(a4),d0		all $ff means nothing connected
		and.b	XTPORT1(a4),d0
		and.b	XTPORT2(a4),d0
		and.b	XTPORT3(a4),d0
		cmpi.b	#$ff,d0
		beq	FakeTimeout		yup, fake a timout condition

		bsr	FakeSelect		put this unit in running slot
		movea.l	st_RunningUnit(a5),a0	fetch unit pointer
		movea.l	hu_CurrentCmd(a0),a0	command block to a0
		clr.b	cb_ErrorCode(a0)	assume we will get no errors
		movea.l	cb_WCommand(a0),a1	fetch the scsi command
		move.b	(a1),d0			and the command byte
		lea.l	XTD(pc),a1		scan table for the code
10$		move.l	(a1)+,d1		get offset and command
		beq.s	XTBadCmd		sorry, we don't handle it
		cmp.b	d1,d0			is it this command
		bne.s	10$			nope, keep looking
		swap	d1			code offset to lower word
		jmp	XTD(pc,d1.w)		and call the routine

; we don't support the SCSI command that we were given.  Fake up an error
; condition and set the error codes on this unit for an unimplemented cmd.
XTBadCmd:

	IFD DEBUG_CODE
	printf <'XT: unknown command\n'>
	movea.l	cb_WCommand(a0),a1
	printf <'Cmd = 0x%lx'>,(a1)+
	move.w	(a1)+,d0
	printf <'%x\n'>,d0
	ENDC

		movea.l	cb_SCSICmd(a0),a1	fetch scsi command
		clr.l	scsi_Actual(a1)		no data transferred
		move.w	scsi_CmdLength(a1),scsi_CmdActual(a1)
		move.b	#$02,scsi_Status(a1)	Check Condition
		clr.b	cb_ErrorCode(a0)	*** must change this ***

		movea.l	st_RunningUnit(a5),a0	get unit pointer back
		move.b	#UNIT_WAITING,hu_WhatNext(a0)   back on waiting queue
		move.l	#$20000000,hu_NextError(a0) error, illegal command
;	printf <'XT: calling FakeDisconnect\n'>
		bra	FakeDisconnect		and disconnect this unit

;==============================================================================
; Dispatch table used to convert scsi commands to the appropriate XT calls
; I'm really only interested in the SCSI commands initiated by this driver
; All routines dispatched to get a pointer to the command block in a0.
; READ_EXTENDED and WRITE_EXTENDED are not supported by XT drives.
;==============================================================================
XTD		DC.W	XT_READ-XTD,READ
		DC.W	XT_WRITE-XTD,WRITE
		DC.W	XT_TUR-XTD,TEST_UNIT_READY
		DC.W	XT_RQS-XTD,REQUEST_SENSE
		DC.W	XT_FMT-XTD,FORMAT_UNIT
		DC.W	XT_SEEK-XTD,SEEK
		DC.W	XT_RDC-XTD,READ_CAPACITY
		DC.W	0,0

;==============================================================================
; we've been issued a normal read or write command.  Before issueing the
; command, we convert the block offset to cylinder, head and sector values.
; We'll be called again later when the WDC interrupt comes in (good or bad)
; There is no need to use the PUTREG and GETREG macros (used in scsitask)
; because there is no SCSI interrupt activity occuring at this time.
;==============================================================================
XT_READ:
XT_WRITE:
XT_SEEK		movea.l	st_RunningUnit(a5),a1	fetch the active unit
		move.b	#TOTAL_SECTORS,SASR(a4)	fill in geometry info
		move.b	hu_TotalBlocks(a1),SCMD(a4)  sectors per track
		move.b	hu_TotalHeads(a1),SCMD(a4)   tracks per cylinder
		move.b	hu_TotalCyls(a1),SCMD(a4)    total cylinders
		move.b	hu_TotalCyls+1(a1),SCMD(a4)
		movea.l	cb_WCommand(a0),a1	fetch the logical offset
;	IFD DEBUG_CODE
;	move.l	(a1),d0
;	andi.l	#$00ffffff,d0
;	printf <'XT: r/w offset = %ld\n'>,d0
;	ENDC
		move.b	#0,SCMD(a4)		msb of offset is always 0
		move.b	1(a1),SCMD(a4)		stash block offset
		move.b	2(a1),SCMD(a4)
		move.b	3(a1),SCMD(a4)
		move.b	#0,SCMD(a4)		don't care about sector number
		move.b	#0,SCMD(a4)		don't need to allow for spares
		move.b	#COMMAND,SASR(a4)	select command register
		move.b	#wd_TRANSLATE_ADDRESS,SCMD(a4)  issue translate cmd
		rts				will come back later

; this is called when we used the address translation feature of the WDC to
; convert a block offset to cylinder, head, sector values.  Only used for
; read, write and seek operations.
;******************************************************************************
; SMALL BUG HERE, DMA CRASHES HORRIBLY IF XT IS NOT SET TO DMA MODE BEFORE
; ACTUALLY STARTING THE DMA PROCESS.  ALSO A BIG BUG HERE!  IF TERMINAL COUNT
; IS NOT ENABLED, THE DMA CHIP KEEPS GOING (AND SPLATTERS MEMORY) WHEN THE XT
; HAS ACTUALLY FINISHED TRANSFERRING DATA.  DONT KNOW ABOUT WRITE DIRECTION!
; THIS HAPPENS EVEN IF THE XTEOP BIT HAS BEEN ENABLED IN THE CONTROL REGISTER
;******************************************************************************
XTSendCommand	movea.l	st_RunningUnit(a5),a0
		movea.l	hu_CurrentCmd(a0),a0

		move.b	#2,XTPORT3(a4)		only interrupts enabled
		movea.l	cb_WCommand(a0),a1	see if this was a seek
		cmpi.b	#SEEK,(a1)
		beq	NoDMAHere		it is, so no DMA needed

		move.b	#3,XTPORT3(a4)		interrupt enable + dma reqests
		move.b	#DMAF_INTENA!DMAF_TCE!DMAF_XTEOP,d0	assume we're reading
		movea.l	cb_WCommand(a0),a1	was this a read or write ?
		cmpi.b	#READ,(a1)
		beq.s	10$			correct, it was a read
		move.b	#DMAF_INTENA!DMAF_DDIR!DMAF_TCE!DMAF_XTEOP,d0  nope, was write
10$		move.b	d0,CNTR(a4)		set up DMA data direction

		moveq.l	#0,d0			need to make word xfer count
		move.b	4(a1),d0		get block count
		bne.s	20$			0 = 256 blocks for XT drives
		move.w	#256,d0			so fudge it here
20$		lsl.l	#8,d0			this make word count (not byte)
		move.l	d0,WTCH(a4)

		move.l	cb_WData(a0),SACH(a4)	set data source/dest address
		move.b	#1,st_DMAGoing(a5)	flag that DMA is running
		move.w	#0,SDMA(a4)		and start DMA

; the DMA chip is now waiting for something to happen, send command to drive
NoDMAHere	subq.l	#6,sp			command block on stack
		move.b	(a1),(sp)		the read/write command code
		move.b	#SECTOR_NUM,SASR(a4)	get results from translation
		move.b	SCMD(a4),d0		save sector # for a while
		move.b	SCMD(a4),1(sp)		save the head number
		move.b	SCMD(a4),d1		msb of cylinder number...
		lsl.b	#6,d1			...goes to upper 2 bits...
		or.b	d1,d0			...and is merged with sector
		move.b	d0,2(sp)
		move.b	SCMD(a4),3(sp)		cylinder # lsb
		move.b	4(a1),4(sp)		number of blocks to transfer
		clr.b	5(sp)			no special flags

	IFD DEBUG_CODE
	moveq.l	#0,d0
	move.b	2(sp),d0
	andi.b	#$3f,d0
	printf <'Sector   %d\n'>,d0

	moveq.l	#0,d0
	move.b	1(sp),d0
	andi.b	#$1f,d0
	printf <'Head     %d\n'>,d0

	moveq.l	#0,d0
	move.b	2(sp),d0
	andi.b	#$c0,d0
	lsl.l	#2,d0
	move.b	3(sp),d0
	printf <'Cylinder %d\n'>,d0

	moveq.l	#0,d0
	move.b	4(sp),d0
	printf <'Count    %d\n'>,d0
	ENDC

		movea.l	sp,a0
		bsr	SendXTCmd		send this command
		addq.l	#6,sp			reclaim stack space
		tst.l	d0			did this work
		beq	TimedOut		nope, make like a timeout
		rts				yes, XTCommandDone handles it

;==============================================================================
; Comes here if we get an interrupt from the WDC to say that the block offset
; we were translating exceeded the disk boundaries.  We just fake up the error
; codes for an illegal address and quit this command now.
;==============================================================================
XTAddressBad	movea.l	st_RunningUnit(a5),a0	fetch the unit pointer
		move.b	#UNIT_WAITING,hu_WhatNext(a0) maybe put back on waiting
		movea.l	hu_CurrentCmd(a0),a1	fetch the command block
		movea.l	cb_WStatus(a1),a1	and the status area pointer
		move.b	#$02,(a1)		set up a check condition code
		movea.l	hu_CurrentCmd(a0),a1	fetch command block again
		movea.l	cb_WCommand(a1),a1	fetch block offset from cmd
		move.l	(a1),d0
		andi.l	#$ffffff,d0		only 3 bytes significant
		ori.l	#$a1000000,d0		address valid + illegal address
		move.l	d0,hu_NextError(a0)	stash error for later
		bra	FakeDisconnect		disconnect and reply


;==============================================================================
; Does the equivalent of SCSI's test unit ready, sets sense codes accordingly.
;==============================================================================
XT_TUR		move.b	#2,XTPORT3(a4)		only interrupts enabled
		clr.l	-(sp)
		clr.w	-(sp)			test drive ready command
		movea.l	sp,a0
		bsr	SendXTCmd		send command to the drive
		addq.l	#6,sp
		tst.l	d0
		beq	TimedOut
		rts

;==============================================================================
; Does the equivalent of a SCSI request sense.  We don't have to go to the
; drive for this because the sense info was already fetched when the command
; completed.  It is held in hu_NextError ready for copying into the data area
;==============================================================================
XT_RQS		movea.l	st_RunningUnit(a5),a1	fetch the sense data from unit
		move.b	#UNIT_WAITING,hu_WhatNext(a1)  where to go next
		move.l	hu_NextError(a1),d0
	printf <'XT: error = 0x%lx\n'>,d0
		movea.l	cb_WData(a0),a1		store in data area
		move.l	d0,(a1)
		movea.l	cb_SCSICmd(a0),a1	set up actual
		move.l	#$04,scsi_Actual(a1)	we only supply 4 bytes of sense
		move.w	scsi_CmdLength(a1),scsi_CmdActual(a1)
		clr.b	scsi_Status(a1)		no error
		bra	FakeDisconnect		disconnect and reply

;==============================================================================
; Does a format unit with no special flags or bad block lists sent to the drive
;==============================================================================
XT_FMT		move.b	#2,XTPORT3(a4)		only interrupts enabled
	printf <'XT: sending a format command\n'>
		move.l	#$00000400,-(sp)		4:1 interleave
		move.w	#$0400,-(sp)		format drive command
		movea.l	sp,a0
		bsr	SendXTCmd		send command to the drive
		addq.l	#6,sp
		tst.l	d0
		beq	TimedOut
		rts


;==============================================================================
; Does the equivalent of a read capacity command (only support 20 and 40 megs)
;==============================================================================
XT_RDC:
	printf <'Read capacity called\n'>
		movea.l	cb_WData(a0),a1		get data area
		move.l	#512,4(a1)		always 512 byte blocks
		move.l	#41615,d0		total blocks on 20 meg drive
		btst.b	#1,XTPORT2(a4)		test the 40/20 flag
		bne.s	10$			it was 20Meg
		lsl.l	#1,d0			it's 40Meg
10$		move.l	d0,(a1)
		movea.l	cb_SCSICmd(a0),a1	fill in fields in scsiCmd
		move.l	#8,scsi_Actual(a1)
		move.w	scsi_CmdLength(a1),scsi_CmdActual(a1)
		clr.b	scsi_Status(a1)

; This is a horrible kluge I had to add for Western Digital XT drives.  It
; appears that these drives will not function correctly if the parameters
; for drive geometry have not been set yet.  We'll do this whenever we get
; a read capacity command.  (Everything will be hard coded here).
		move.b	#0,XTPORT3(a4)		nothing enabled
		clr.l	-(sp)
		move.w	#$0c00,-(sp)		construct the command
		movea.l	sp,a0
		bsr	SendXTCmd		and send it
		addq.l	#6,sp
		tst.l	d0			did it work
		beq	TimedOut		nope
	printf <'Sending drive parameters\n'>

; I'm just going to wait for a data request now and send it immediately
		clr.l	-(sp)
		clr.l	-(sp)
		movea.l	st_RunningUnit(a5),a0	need unit parameters
		move.w	hu_TotalCyls(a0),(sp)	number of cylinders
		move.b	hu_TotalHeads(a0),2(sp)	number of heads
		moveq.l	#7,d0			sending 8 bytes of data
		movea.l	sp,a0
30$		btst.b	#0,XTPORT1(a4)		wait for data request
		beq.s	30$
		move.b	(a0)+,XTPORT0(a4)	and send a byte
		dbra	d0,30$
	printf <'Sent %lx %lx\n'>
		addq.l	#8,sp			free up stack space
; drop through to complete the command

;==============================================================================
; When an interrupt occurs from the XT drive, control is passed back to here.
; We must always read the status byte to clear the current command.  If there
; was an error (2) then we'll also issue a "read status" command and stash the
; results into hu_NextError ready to be picked up by a request sense call.  We
; always terminate the command at this point because there should be nothing
; else to do, it either failed or it worked.  The error handling in IOTask is
; responsible for figuring out which.
;==============================================================================
XTCommandDone	movea.l	st_RunningUnit(a5),a0
	printf <'XT: command complete\n'>
		clr.l	hu_NextError(a0)	assume no errors
		movea.l	hu_CurrentCmd(a0),a1	fetch the command block ptr
		movea.l	cb_WStatus(a1),a1	and the status area
10$		btst.b	#0,XTPORT1(a4)		wait for status byte ready
		beq.s	10$
		move.b	XTPORT0(a4),(a1)	save error code (0 or 2)
		beq	XTNoErrors		no error occured

; the drive reported an error on the last operation, read sense data from drive
	printf <'XT: got an error on last cmd\n'>
		clr.l	-(sp)			read status command on stack
		move.w	#$0300,-(sp)
		movea.l	sp,a0
		bsr	SendXTCmd		issue command to drive
		addq.l	#6,sp
******************
		tst.l	d0			did it work
		beq	TimedOut		no, timed out

; we've issued the command, now read back 4 bytes of sense info from the drive
		movea.l	st_RunningUnit(a5),a0	refetch the unit pointer
		lea.l	hu_NextError(a0),a1	point to sense data area
		moveq.l	#3,d0			fetching 4 bytes of info
20$		btst.b	#0,XTPORT1(a4)		wait for data ready
		beq.s	20$
		move.b	XTPORT0(a4),(a1)+	stash data byte
		dbra	d0,20$			and go for the next
	printf <'XT: error was 0x%lx\n'>,hu_NextError(a0)

; we have to read a status byte after this command too, but we'll junk it
30$		btst.b	#0,XTPORT1(a4)		wait for data ready
		beq.s	30$
		move.b	XTPORT0(a4),d0		just waste this byte

; if the error code is negative then the address in the sense data is valid
; but needs conversion from heads/cyls etc. to a block offset.  I have to do
; this with the CPU because the WDC doesn't provide translation this way.
		tst.b	hu_NextError(a0)	is address valid ?
		bpl.s	XTNoErrors		nope, so don't translate

		movem.l	d2-d3,-(sp)
		moveq.l	#0,d0
		moveq.l	#0,d1
		moveq.l	#0,d3
		move.b	hu_NextError+1(a0),d0	get head number
		move.b	hu_NextError+2(a0),d1	get sector number
		move.l	d1,d2			d2 holds msb of cylinder
		andi.b	#$3f,d1			mask off cyl msb's
		lsl.l	#2,d2
		move.b	hu_NextError+3(a0),d2	fetch rest of cyl number
		move.b	hu_TotalHeads(a0),d3
		mulu.w	d3,d2
		move.b	hu_TotalBlocks(a0),d3
		mulu.w	d3,d2			d2 = cylinder to block offset
		mulu.w	d3,d0			convert head to block offset
		add.l	d0,d2			add to total so far
		add.l	d1,d2			and add sector number
		move.l	hu_NextError(a0),d0	get back the error code
		andi.l	#$ff000000,d0		mask off track sector head stuff
		or.l	d2,d0			merge in block offset
		move.l	d0,hu_NextError(a0)	and store it back
		movem.l	(sp)+,d2-d3

; either there were no errors or there was an error and we handled it.
XTNoErrors	movea.l	hu_CurrentCmd(a0),a1	fetch the command block
		movea.l	cb_SCSICmd(a1),a1
		move.l	scsi_Length(a1),scsi_Actual(a1)  make like it all worked
		move.w	scsi_CmdLength(a1),scsi_CmdActual(a1)
		move.b	#UNIT_WAITING,hu_WhatNext(a0)    which queue next
		bra	FakeDisconnect		disconnect and reply IORequest

;==============================================================================
; success = SendXTCmd( command )
;   d0			  a0
;
; Sends 6 bytes of command block to XT unit 0.  DMA and IRQ bits must have been
; set up appropriately before calling this routine (includes starting DMA chip)
; A return of TRUE means everything worked.  A FALSE return should be taken to
; mean that a timeout occured and the unit probably isn't even connected.
;==============================================================================
SendXTCmd:
	IFD DEBUG_CODE
	move.w	4(a0),d0
	printf <'XT: sending cmd %lx%x\n'>,(a0),d0
	ENDC

; Epson drives don't respond to command accept after an error occurs.  They
; seem to need some arbitrarily long time to recover.  Constant banging on
; the command accept register fixed the problem but stopped Western Digital
; drives from working at all.  I've had to do this compromise timeout code!
		moveq.l	#20,d1			fix for Epson drives
EpsonTO		move.b	#0,XTPORT2(a4)		start command accept
		moveq.l	#-1,d0			timeout value
Timeout		btst.b	#0,XTPORT1(a4)		wait for data request
		bne.s	SendCmd			got one
		dbra	d0,Timeout		not yet, keep looking
		dbra	d1,EpsonTO
		moveq.l	#0,d0			return false
		rts				unit timed out

SendCmd		moveq.l	#5,d0			sending 6 bytes
10$		btst.b	#0,XTPORT1(a4)		wait for data request
		beq.s	10$
		move.b	(a0)+,XTPORT0(a4)	send next byte of command
		dbra	d0,10$			d0 will be TRUE on exit
		rts


;==============================================================================
; TimedOut()
;
; There must be a unit in the RunningUnit slot for this to work.  If a command
; times out (no response to the command accept) then control can be passed to
; here which will mark the unit as timed out and will call FakeDisconnect.
;==============================================================================
TimedOut	printf <'XT: Timed out on cmd\n'>
		movea.l	st_RunningUnit(a5),a0	fetch unit
		move.b	#UNIT_TIMED_OUT,hu_WhatNext(a0)
		bra	FakeDisconnect

		END
