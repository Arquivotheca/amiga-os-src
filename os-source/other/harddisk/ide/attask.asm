		SECTION	driver,CODE
;==============================================================================
; This code is for handling AT drives in the SCSITask environment.  It is not
; a separate task in its own right.  All AT drives are sent SCSI command blocks
; which are interpreted here and sent to the drive hardware as AT commands.
;==============================================================================
	IFD IS_IDE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/interrupts.i"
		INCLUDE	"exec/lists.i"
		INCLUDE	"exec/ports.i"
		INCLUDE	"exec/tasks.i"
		INCLUDE "exec/ables.i"
		INCLUDE "hardware/custom.i"
		INCLUDE	"devices/scsidisk.i"

		INCLUDE	"modifiers.i"
		INCLUDE	"device.i"
		INCLUDE	"scsitask.i"
		INCLUDE	"board.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XDEF	XTSelect,XTSendCommand,XTCommandDone,ATFindHardware

		XREF	FakeSelect,FakeDisconnect,FakeTimeout,WaitASecond
		XREF	_LVOWait,_LVOSetSignal
		XREF	_intena,_intenar,_custom

;==============================================================================
; 020 variant macros, since 68000's can't handle unaligned accesses
;==============================================================================

GETLONG_A1	MACRO	; get at address a1, return in d0.  a0/d1 preserved
			; A1 is incremented by 4!
		IFD	FOR_68000_ONLY
		bsr	getlong_a1
		ENDC
		IFND	FOR_68000_ONLY
		move.l	(a1),d0
		ENDC
		ENDM

PUTLONG_A1	MACRO	; put long at address a1, from d0
			; NOTE: d0/d1 are destroyed, a1 incremented by 4!
		IFD	FOR_68000_ONLY
		bsr	putlong_a1
		ENDC
		IFND	FOR_68000_ONLY
		move.l	d0,(a1)+
		ENDC
		ENDM

;==============================================================================
; Although this is called XTSelect, it is actually responsible for starting
; the given command (in the SCSIDirect command attached to the command block)
;
; All fields of the scsi_Command are used and maintained as if we were talking
; to a SCSI device.  That is, scsi_Actual is updated accordingly and the
; target status is set to check condition ($02) if we get some kind of error.
; scsi_CmdActual will always be set equal to scsi_CmdLength.  AT units will
; only return non-extended sense data for now (won't comply with SCSI-2) but
; fortunately for me, I can use the 4 bytes returned by an AT drive.  The
; important error codes all map directly to SCSI non-extended sense.
;
; When a command completes, the 4 error bytes are read into hu_NextError for
; use by the request sense command which just copies them to the sense data
; area and then clears the error condition in hu_NextError.
;
; FakeSelect call will set up SCSI working pointers which we can use here.
;==============================================================================
ATSelect
		bsr	FakeSelect		put this unit in running slot
		movea.l	cb_WCommand(a3),a1	fetch the scsi command
		move.b	(a1),d0			and the command byte
		cmp.b	#REQUEST_SENSE,d0	is this a request sense?
		beq.s	is_sense		yes, don't clear error values
		clr.l	hu_NextError(a2)	assume no errors
is_sense:
		clr.b	cb_ErrorCode(a3)	assume we will get no errors

;	IFD DEBUG_CODE
;	and.l	#$ff,d0
;	printf <'In ATSelect, command = %lx, a4 = $%lx\n'>,d0,a4
;	ENDC
		lea.l	XTD(pc),a0		scan table for the code
10$		move.l	(a0)+,d1		get offset and command
		beq.s	ATBadCmd		sorry, we don't handle it
		cmp.b	d1,d0			is it this command
		bne.s	10$			nope, keep looking

; puts the translated command value in d0 for the routine to use (mainly for
; read, write, seek which use common routines).  We can't modify the WCommand
; data since scsidirect commands are put directly in there.

		lsr.w	#8,d1			move AT cmd into low byte
		move.b	d1,d0			move it into d0 for command
		swap	d1			code offset to lower word
; d0 is command byte, a1 is scsi command
		jmp	XTD(pc,d1.w)		and call the routine

; we don't support the SCSI command that we were given.  Fake up an error
; condition and set the error codes on this unit for an unimplemented cmd.
ATBadCmd:

	IFD DEBUG_CODE
	printf <'AT: unknown command\n'>
	moveq	#0,d1
	move.b	d0,d1
	printf <'Cmd = 0x%lx, st_RunningUnit = $%lx'>,d1,st_RunningUnit(a5)
	ENDC

		clr.b	cb_ErrorCode(a3)	*** must change this FIX ***
		movea.l	cb_SCSICmd(a3),a1	fetch scsi command
		clr.l	scsi_Actual(a1)		no data transferred
		; set ABRT bit in error code
		moveq	#ERRF_ABRT,d0
		move.l	d0,hu_NextError(a2)	error, illegal command
		moveq	#CHECK_CONDITION,d0	check condition
		bra	ATCommandDone		and disconnect this unit

;==============================================================================
; Dispatch table used to convert scsi commands to the appropriate XT calls
; I'm really only interested in the SCSI commands initiated by this driver
; All routines dispatched to get a pointer to the command block in a0.
; READ_EXTENDED and WRITE_EXTENDED are not supported by XT drives.
;==============================================================================
XTD		DC.W	XT_READ-XTD
		DC.B	AT_READ_SECTORS,READ

		DC.W	XT_WRITE-XTD
		DC.B	AT_WRITE_SECTORS,WRITE

		DC.W	XT_READ_EXT-XTD
		DC.B	AT_READ_SECTORS,READ_EXTENDED

		DC.W	XT_WRITE_EXT-XTD
		DC.B	AT_WRITE_SECTORS,WRITE_EXTENDED

		DC.W	XT_RQS-XTD
		DC.B	0,REQUEST_SENSE

		DC.W	XT_TUR-XTD
		DC.B	0,TEST_UNIT_READY

		DC.W	XT_FMT-XTD
		DC.B	AT_FORMAT,FORMAT_UNIT

		DC.W	XT_SEEK-XTD
		DC.B	AT_SEEK,SEEK

		DC.W	XT_RDC-XTD
		DC.B	0,READ_CAPACITY

		DC.W	XT_MODE_SENSE-XTD
		DC.B	0,MODE_SENSE

		DC.W	XT_SEEK_EXT-XTD
		DC.B	AT_SEEK,SEEK_EXTENDED

		DC.W	XT_INQUIRY-XTD
		DC.B	0,INQUIRY

		DC.W	XT_RECAL-XTD
		DC.B	AT_RECALIBRATE,REZERO_UNIT

		DC.W	XT_READ_VERIFY-XTD
		DC.B	AT_READ_VERIFY_SECTORS,VERIFY

		DC.W	XT_RESERVE-XTD
		DC.B	0,RESERVE

		DC.W	XT_RELEASE-XTD
		DC.B	0,RELEASE

		DC.W	XT_DIAGNOSTIC-XTD
		DC.B	AT_DIAGNOSTICS,SEND_DIAGNOSTIC

		DC.W	0,0

;==============================================================================
; LongXfer (d0.b: command, d1.w: len (sectors))  Transfers a bunch of sectors
;==============================================================================
LongXfer:	; loop, xferring 256 sectors at a time until we have less than
		; 256 left, then do them and finish up.
		movem.l	d2/d3,-(sp)
		move.w	d1,d2			length in sectors
		move.b	d0,d3			command
;	printf <'longxfer of $%lx blocks\n'>,d2

xfer_loop:	move.w	d2,d0
		and.w	#$ff00,d0		is high byte 0?
		beq.s	1$
		; low byte must be 0 here - 0 is 256 blocks
		sub.w	#$0100,d2		count -= 256
		bra.s	2$
1$:		move.b	d2,d0			do remainder (< 256 blocks)
		beq.s	xfer_done		d0 = 0 (success)
		moveq	#0,d2			nothing left after this

2$:		move.b	d0,AT_SectorCnt(a4)	number of sectors to r/w/s
;	printf <'xfering $%lx blocks, $%lx left, cmd = $%lx\n'>,d0,d2,d3

		move.b	d3,AT_Command(a4)	start cmd
		move.b	d0,cb_ATLength(a3)	remember length requested
		move.b	d3,d0			command for findinitstate
		bsr	FindInitState (d0)	get init state for HandleInts
		bsr	HandleInts (d0)		returns 0/2
		tst.b	d0
		bne.s	xfer_err

		; Read/Write and ReadMultiple/WriteMultiple and ReadVerify
		; ALL are guaranteed to leave the last sector read/written/
		; verified in the registers.  We need to bump it.
		move.b	AT_SectorNum(a4),d0
		addq.b	#1,d0
		cmp.b	hu_TotalBlocks(a2),d0	careful, sectors are 1-N!
		bls.s	secs_done		branch on d0 <= sectors

		; bump head, maybe cyl
		move.b	AT_DriveHead(a4),d0
		and.b	#$0f,d0			mask off drive bit/etc
		addq.b	#1,d0
		cmp.b	hu_TotalHeads(a2),d0
		bcs.s	heads_done

		; bump cyl
		addq.b	#1,AT_CylLow(a4)
		bcc.s	cyl_done		if carry set, bump high byte
		addq.b	#1,AT_CylHigh(a4)	assume this won't overflow!

cyl_done:	moveq	#0,d0			reset head number
heads_done:	or.b	hu_ATDriveBit(a2),d0	don't forget drive bit!
		move.b	d0,AT_DriveHead(a4)
		moveq	#1,d0			reset sector num (1-N)
secs_done:	move.b	d0,AT_SectorNum(a4)
		bra.s	xfer_loop

xfer_err:	bsr	GetErrorAddr		save address of error
		; returns CHECK_CONDITION in d0

xfer_done:	movem.l	(sp)+,d2/d3
		rts

;==============================================================================
; we've been issued an extended read/write/seek command.  Before issueing the
; command, we convert the block offset to cylinder, head and sector values.
; a3 is	hu_CurrentCmd here, d0 is command byte, a1 is scsi command
; extended r/w/s commands have 4-byte offset, 2 byte xfer length
;
; NOTE: scsi commands are guaranteed to be word-aligned (user commands are
; copied to internally allocated command buffers).
;==============================================================================
XT_READ_EXT:
XT_WRITE_EXT:
XT_SEEK_EXT:
		move.w	d0,-(sp)		save translated command
		move.l	2(a1),d0		get length, offset 2
;	printf <'AT: extended r/w offset = %ld\n'>,d0

		; ATSetAddress may not return!!! be careful of stack!
		bsr.s	ATSetAddress (d0)	set drive/cyl/head/sector
						; a0/a1 are preserved!

;==============================================================================
; Address has been translated, and AT registers poked with the values.
; we need to maintain cb_DidReadDMA to know if we should push the cache
; after an xfer (for copyback caches; right).
;==============================================================================

		move.w	(sp)+,d1		get command back
		move.l	d1,d0			in case multi isn't supported
		cmpi.b	#AT_SEEK,d1
		beq.s	NoExtTransfer		it is, so no transfer needed

		tst.b	hu_MaxRWSize(a2)	can we use read/write multiple?
		beq.s	1$			nope

;	printf <'using r/w multiple\n'>
		moveq.b	#AT_READ_MULTIPLE,d0	assume read
		cmp.b	#AT_READ_SECTORS,d1
		beq.s	1$
		moveq.b	#AT_WRITE_MULTIPLE,d0
1$
		; used to set cb_DidReadDMA here
		move.b	7(a1),d1		get block count
		lsl.w	#8,d1
		move.b	8(a1),d1
;	IFD DEBUG_CODE
;	andi.l	#$ffff,d1
;	printf <'ext length %ld, cmd = $%lx\n'>,d1,d0
;	ENDC
		bsr	LongXfer (d0,d1)	transfer sectors

		bra	ATCommandDone  (d0)

NoExtTransfer:  ; on seek, just send the command and be done with it.
		move.w	d1,d0
		bra	ATSendCommand

;==============================================================================
; convert LBA in d0 to Drive/Cyl/Head/Sector in AT registers.  Destroys
; d0/d1, PRESERVES a0/a1.
;==============================================================================
ATSetAddress:
		move.l	d0,d1			save for SetAddrFail
		divu.w	hu_CylSize(a2),d0	find cylinder number
		bvs.s	SetAddrFail		cylinder > 65535
		cmp.w	hu_TotalCyls(a2),d0	is it out of range?
		bcc.s	SetAddrFail		no. (unsigned!)
		move.b	d0,AT_CylLow(a4)	set the cylinder number reg
		lsr.w	#8,d0
		move.b	d0,AT_CylHigh(a4)
		clr.w	d0			clear bottom word
		swap	d0			get remainder (now longword)
		moveq	#0,d1
		move.b	hu_TotalBlocks(a2),d1	get number of sectors/head
		divu.w	d1,d0			find out head number 
		or.b	hu_ATDriveBit(a2),d0	combined head/drive register
		move.b	d0,AT_DriveHead(a4)	set the head number
		swap	d0
		addq.b	#1,d0			sectors are 1-N, not 0-(N-1)
		move.b	d0,AT_SectorNum(a4)	set the sector number
;	IFD DEBUG_CODE
;	moveq	#0,d0
;	moveq	#0,d1
;	move.b	AT_CylHigh(a4),d0
;	lsl.l	#8,d0
;	move.b	AT_CylLow(a4),d0
;	move.b	AT_DriveHead(a4),d1
;	printf <'rws: Cyl = %ld, drive/Head = $%lx\n'>,d0,d1
;	move.b	AT_SectorNum(a4),d0
;	printf <'     Sector = %ld\n'>,d0
;	ENDC
		rts
SetAddrFail:
		; VERY tricky!!!
		; called with return addr and translated cmd (word) on stack
		addq.w	#6,sp			drop return address and cmd!
		; fall through

;==============================================================================
; Comes here if the block offset
; we were translating exceeded the disk boundaries.  We just fake up the error
; codes for an illegal address and quit this command now.
;==============================================================================
AT_OutOfRange	move.l	#ERRF_ADDR,d0		bit 8 - address out of range
		move.l	d0,hu_NextError(a2)	stash for request sense
		move.l	d1,hu_ErrorLBA(a2)	d1 has the bad address
		moveq	#CHECK_CONDITION,d0	check condition
		bra	ATCommandDone		clean up and return command

;==============================================================================
; we've been issued a normal read or write command.  Before issueing the
; command, we convert the block offset to cylinder, head and sector values.
; A max sector offset of $00ffffff gives us 4 gig at 256 bytes/sector.
; a3 is	hu_CurrentCmd here, d0 is command byte, a1 is scsi command
;
; NOTE: scsi commands are guaranteed to be word-aligned (user commands are
; copied to internally allocated command buffers).
;==============================================================================
XT_READ:
XT_WRITE:
XT_SEEK:
		move.w	d0,-(sp)		save AT command
		move.l	(a1),d0			get command/lun/length(aligned)
		and.l	#$001fffff,d0		mask off command/lun
;	printf <'AT: r/w/s offset = %ld\n'>,d0

		; ATSetAddress may not return!!! be careful of stack!
		bsr	ATSetAddress (d0)	set drive/cyl/head/sector
						; a0/a1 are preserved!

;==============================================================================
; Address has been translated, and AT registers poked with the values.
; we need to maintain cb_DidReadDMA to know if we should push the cache
; after an xfer (for copyback caches; right).
;==============================================================================

		cmpi.b	#AT_SEEK,1(sp)		counts on stack!
		beq.s	NoTransfer		it is, so no transfer needed

		; used to set cb_DidReadDMA here
		move.b	4(a1),d0		get block count (0 = 256)
		move.b	d0,AT_SectorCnt(a4)
		move.b	d0,cb_ATLength(a3)

;	IFD DEBUG_CODE
;	moveq	#0,d1
;	move.b	d0,d1
;	printf <'rw length = %ld blocks\n'>,d1
;	ENDC
		; start checking for r/w multiple use
		cmp.b	#1,d0
		beq.s	NoTransfer		no need to use r/w multiple
		tst.b	hu_MaxRWSize(a2)	can we use read/write multiple?
		beq.s	NoTransfer		nope

		; we can (and will) use r/w multiple!
;	printf <'using r/w multiple\n'>
		moveq.b	#AT_READ_MULTIPLE,d0	assume read
		cmp.b	#AT_READ_SECTORS,1(sp)
		beq.s	1$
		moveq.b	#AT_WRITE_MULTIPLE,d0
1$:		move.b	d0,1(sp)		change command value

; All registers set up, hit the command register to start the operation
; Make sure the destination, etc has been set up also.
NoTransfer:
		move.w	(sp)+,d0
		bra	ATSendCommand

;==============================================================================
; Does the equivalent of SCSI's test unit ready, sets sense codes accordingly.
; a3 is	hu_CurrentCmd here, d0 is command byte, a1 is scsi command
;==============================================================================
XT_TUR
	printf <'AT: TestUnitReady command\n'>
		; the interface returns 0xff if the drive isn't attached
		; however, we'll check for disk attachment at boot time
		; (recalibrate/diagnostics?) and set a bit we can check here.

		moveq	#0,d0
		bra	ATCommandDone
		
not_ready:	move.l	#ERRF_NOT_READY,hu_NextError(a2)
;						Not Ready, LUN doesn't respond
		clr.l	hu_ErrorLBA(a2)		no location
		moveq	#CHECK_CONDITION,d0	Check Condition
		bra	ATCommandDone

;==============================================================================
; Reserve and Release: merely return success.  We have only one initiator.
;==============================================================================
XT_RESERVE
XT_RELEASE
	printf <'AT: Reserve/Release command\n'>
		moveq	#0,d0
		bra	ATCommandDone
		
;==============================================================================
; Diagnostic: do a self-test if requested, reject otherwise.
; SCSI-2 only requires self-test with param length 0.
; a3 is	hu_CurrentCmd here, d0 is command byte, a1 is scsi command
;==============================================================================
XT_DIAGNOSTIC:
	printf <'AT: Diagnostic command\n'>
		move.b	1(a1),d1		control bits
		btst.b	#2,d1			is selftest set?
		beq.s	badcmd			no

		tst.b	3(a1)			check both param length bytes
		bne.s	badcmd			must be 0
		tst.b	4(a1)
		bne.s	badcmd

		; ok, do selftest
		move.b	d0,AT_Command(a4)	start command
		moveq	#0*2,d0			non-data command
		bsr	HandleInts (d0)		handle interrupts
		; ignore result - it's in the error register

		moveq	#0,d0			paranoia - set result

		cmp.b	#1,AT_Error(a4)
		beq.s	success

		; hardware error, report it
		move.l	#ERRF_SELFTEST,hu_NextError(a2)	    Self-test failure
failure:	moveq	#CHECK_CONDITION,d0		    failure
success:	bra	ATCommandDone

badcmd:		move.l	#ERRF_ABRT,hu_NextError(a2)
		bra.s	failure

;==============================================================================
; Does the equivalent of SCSI's Mode Sense command.  Supported pages:
; 3,4.
; a3 is	hu_CurrentCmd here, d0 is command byte, a1 is scsi command
;==============================================================================
XT_MODE_SENSE:
; FIX!! add mode_sense(10), and page $3f!  (Also caching pages!)
	printf <'AT: ModeSense command\n'>
		; ignore DBD in byte 1
		; for PC == 01, return all 0's
		; for all others, return the same

		bsr	AT_IdentifyDrive	fills hu_ATBuffer
		tst.l	d0			d0=0 for success, d0=2 failure
		bne	ATCommandDone		failed, return with error

		; build response on the stack
		move.l	cb_WCommand(a3),a1	SCSI command
		lea	hu_ATBuffer(a2),a0

		move.b	2(a1),d1		Page Control field
		bmi.s	not_changeable		1 is ok
		btst.b	#6,d1			0 is ok
		beq.s	not_changeable

		; we don't have any changeable data
;	printf <'changeable data requested\n'>
		lea	-24(sp),sp		random size
		move.l	sp,a1
		moveq	#24,d0
		bsr	bzero (a1,d0)		zero out buffer
		and.b	#$3f,d1
		lsl.w	#8,d1
		move.b	#$16,d1
		move.w	d1,(a1)			set page and length
		moveq	#24,d0			nothing changeable
		bra	page_done

not_changeable:
		and.b	#$3f,d1			drop PC, leave page code
		cmp.b	#3,d1
		bne.s	not_page_3

		; page 3: Format Device page
		lea	-24(sp),sp		scsi-2 length
		move.l	sp,a1
		moveq	#24,d0
		bsr	bzero (a1,d0)		zero out buffer-no regs changed

		move.w	#$0316,(a1)		page 3, length $16

		; get number of tracks (for tracks/zone)
		; make zones = the number of heads, for the hell of it
		moveq	#1*2,d0			(no regs changed \/  )
		bsr	getATword (a0,d0)	gets word (byte-swapped) in d0
		move.w	d0,2(a1)		tracks/zone
		moveq	#6*2,d0
		bsr.s	getATword (a0,d0)
		move.w	d0,10(a1)		sectors/track
		move.w	#512,12(a1)		bytes/sector
		move.w	#1,14(a1)		interleave
		moveq	#0*2,d0
		bsr.s	getATword (a0,d0)	config info
		move.b	d0,d1
		lsl.b	#5,d1			bit 7 = soft, bit 6 = hard
		and.b	#$c0,d1			drop other bits
		btst.b	#6,d0
		bne.s	2$
		bset.b	#5,d1			RMB (not fixed, so removable)

2$:		move.b	d1,20(a1)		SSEC/HSEC/RMB/SURF/etc
;	IFD DEBUG_CODE
;	moveq	#0,d0
;	move.w	20(a1),d0
;	printf <'page 3: config bits: $%lx\n'>,d0
;	move.w	2(a1),d0
;	printf <'cylinders: $%lx\n'>,d0
;	move.w	10(a1),d0
;	printf <'sectors/track: $%lx\n'>,d0
;	ENDC
		moveq	#24,d0
		bra.s	page_done

not_page_3:	cmp.b	#4,d1
		bne.s	not_page_4

		; page 4: Rigid Disk Drive Geometry page
		lea	-24(sp),sp		scsi-2 length
		move.l	sp,a1
		moveq	#24,d0
		bsr.s	bzero (a1,d0)		zero out buffer-no regs changed

		move.w	#$0416,(a1)		page 3, length $16

		; get number of tracks
		move.b	1*2(a0),4(a1)		cylinders (low byte)
		move.b	1*2+1(a0),3(a1)		cylinders (high byte)
		move.b	3*2(a0),5(a1)		heads (low byte)
;	IFD DEBUG_CODE
;	move.b	5(a1),d0
;	printf <'page 4: heads: $%lx\n'>,d0
;	ENDC
		; ignore write-precomp/reduced write

		moveq	#24,d0

		; set up the Mode Parameter Header (6)
		; Default Medium Type (0), no Block Descriptors, not WP.
page_done:	clr.l	-(sp)			it's 4 bytes long
		addq.b	#3,d0			mode length is non-inclusive
		move.l	sp,a1
		move.b	d0,(a1)			Mode Data Length
		addq.b	#1,d0			back to real value
		move.l	d0,-(sp)		save length
		bsr	copy_buffer (a1,d0)	copy to dest, set actual
		move.l	(sp)+,d0
		lea	0(sp,d0.w),sp		drop buffer off stack
		moveq	#0,d0
		bra	ATCommandDone

not_page_4:	; illegal request
;	IFD DEBUG_CODE
;	and.l	#$ff,d1
;	printf <'Request for unimplemented page $%lx\n'>,d1
;	ENDC
		asl.w	#8,d1			page #, 0 bytes following
		move.w	d1,-(sp)
		move.l	sp,a1
		moveq	#2,d0			length of result
		bra.s	page_done

;==============================================================================
; Fetch word at (a0,d0), swapping high/low bytes - preserve all regs but d0
;==============================================================================
getATword:
		move.w	0(a0,d0.w),d0
		rol.w	#8,d0
		rts

;==============================================================================
; zero d0 bytes at a1, leave a0/a1/d1 alone
;==============================================================================
bzero:
		move.l	a1,-(sp)
		bra.s	2$
1$:		clr.b	(a1)+			don't care if it's slow
2$:		dbra	d0,1$
		move.l	(sp)+,a1
		rts

	IFD IS_A300
;==============================================================================
; ATFindHardware - Check to see if the hardware is there at all.  Actually,
; we don't care if GAYLE is there if there's no hard drive.
; d0 - drive bit to check
; leaves AT_DriveHead register set
;==============================================================================
get_gid_bit:	; get a bit of the gayle ID register
		move.b	(a1),d0
		lsl.b	#1,d0			high bit into carry and x bits
		roxl.b	#1,d1			rotates x bit into low bit
		rts
	ENDC IS_A300

ATFindHardware
	printf <'AT: check drive\n'>
	IFD IS_A300
		; make sure this isn't a mirrored chip register (i.e. that we
		; have an IDE interface here).
		; hardware registers mirror every 512, starting at $dff000.
		; warning - different on different machines!
		; a500/a2000/a3000 - undecoded (random values or 0)
		; a1000 - mirrored chip regs
		; GAYLE id is at $de1000 (write anything, read back 8 times,
		; high bit has the data - pattern is $xx for current rev)

		; check by disable, play with possible intena, see if it
		; appears in the real intenar, then enable.
		move.l	a6,-(sp)
		lea	_custom,a0
		lea	GAYLE_ID_ADDR,a1	GAYLE id address
		DISABLE	a6			; leaves a6 = execbase
		move.w	intenar(a0),-(sp)	save old value
		move.w	#$bfff,intena(a1)	set all ables
		move.w	#$3fff,d1		also flag for no mirror
		cmp.w	intenar(a0),d1
		bne.s	no_mirror
		move.w	d1,intena(a1)		clear all ables
		tst.w	intenar(a0)
		bne.s	no_mirror
		moveq	#0,d1			mirrored
no_mirror:					; leave d1 non-0
		; reset the saved values
		move.w	#$3fff,intena(a0)	clear bits
		ori.w	#$8000,(sp)		add set bit
		move.w	(sp)+,intena(a0)	reset values
		ENABLE	a6,NOFETCH
		move.l	(sp)+,a6

		tst.w	d1			did we find mirroring?
		beq.s	no_hw			yes, exit

		; no mirroring, can now check GAYLE id register safely
		move.l	d0,-(sp)		save drive #
		moveq	#0,d1
		move.b	d1,(a1)			value doesn't matter
		bsr	get_gid_bit		get 4 bits
		bsr	get_gid_bit
		bsr	get_gid_bit
		bsr	get_gid_bit

		move.l	(sp)+,d0		restore drive #
		cmp.b	#GAYLE_VERSION,d1
		bne.s	no_hw

		; WE HAVE A GAYLE!!!!!!
	ENDC IS_A300

		; Select the drive we wish to play with
		move.b	d0,AT_DriveHead(a4)	head irrelevant

		; first, see if the drive is ready to accept commands
		; try to eliminate non-existant drives that return impossible
		; values.
		move.b	AT_CylLow(a4),d1	get a register/status value
		move.b	AT_Status(a4),d0
		move.l	d0,a0			temp storage
		and.b	#$C0,d0			leave BSY and RDY
		beq.s	no_hw			neither - not possible
		cmp.b	#$c0,d0
		beq.s	no_hw			both - not possible
		tst.b	d0			is it busy?
		bpl.s	hw_not_busy

	printf <'not ready, busy\n'>
		; check that the other register read back as status since it's
		; busy.  When busy, the drive must return status for all
		; registers.
		move.l	a0,d0			get back orig value
		eor.b	d1,d0			funky!  == 0 or ATF_IDX if good
		and.b	#~(ATF_IDX),d0		drop off index bit
		bne.s	no_hw			didn't read status!!!!

		; FIX???  should wait until busy goes away (or X seconds?)
		moveq	#2,d0			busy...
		rts

hw_found: printf <'found hardware'>
		moveq	#1,d0			looks like a drive....
		rts
		
		; first, check if the drive registers are there
hw_not_busy:
	printf <'ready, not busy\n'>
		moveq	#$12,d0			a random value
		move.b	d0,AT_CylLow(a4)
		cmp.b	AT_CylLow(a4),d0
		bne.s	no_hw
		moveq	#$34,d0			a different random value
		move.b	d0,AT_CylLow(a4)
		cmp.b	AT_CylLow(a4),d0
		beq.s	hw_found		writable - probably real

no_hw: printf <'no hardware'>
		moveq	#0,d0			no hardware out there
		rts

;==============================================================================
; ATCheckDrive - check if an AT drive exists.  Timeout after a while if it
; doesn't respond.  Sets hu_Found to non-zero for success.  Returns 0 for
; success, 2 or 8 for failure or busy.
;==============================================================================
ATCheckDrive:
	printf <'AT: check drive\n'>
		; make sure this isn't a mirrored chip register (i.e. that we
		; have an IDE interface here).
		; hardware registers mirror every $xxx, starting at $dff000.
		; warning - different on different machines!
; FIX!!!!!!!!

		; Select the drive we wish to play with
		move.b	hu_ATDriveBit(a2),d0
		bsr	ATFindHardware		Make sure this drive is there
		; leaves AT_DriveHead bit set
		tst.l	d0
		beq.s	check_failed		no hardware
		cmp.b	#2,d0			was it busy?
		bne.s	1$

		; busy - return.  lib.asm will keep trying
		moveq	#BUSY,d0
		rts
1$:
		; It's almost certainly there.  Do a recalibrate with timeout
		moveq	#0,d0

	IFD USE_ENABLE_INTS
		; Randy Hilton says we must do an enable interrupt, though
		; it seems most drives come up with them enabled (except the
		; PrairieTek 342).
		; DCF_INT_ENABLE == 0!!  d0 is 0 here
		move.b	d0,AT_DeviceCtrl(a4)
	ENDC
		; clear signal (d0 is 0)
		move.l	st_IntPendMask(a5),d1	which signal to clear
		jsr	_LVOSetSignal(a6)	clear int signal

		move.b	#AT_RECALIBRATE,AT_Command(a4)
		moveq	#15,d1			15 seconds max
wait_loop:	move.w	d1,-(sp)		save count
		moveq	#0,d0
		moveq	#0,d1
		jsr	_LVOSetSignal(a6)	get current signals
		and.l	st_IntPendMask(a5),d0	did we get the signal?
		bne.s	found_it
	printf <'waiting\n'>
		bsr	WaitASecond		delay a second
		move.w	(sp)+,d1
		dbra	d1,wait_loop
		; fall through

check_failed:
	printf <'no drive!\n'>
		move.l	#ERRF_NOT_READY,hu_NextError(a2)
		moveq	#CHECK_CONDITION,d0	failure - no drive!
		rts				can't set hu_Found

found_it:	; d0 is IntPendMask
	printf <'found drive\n'>
		; wait for drive to be not busy (paranoia!)
		; Reading status can clear int- is this safe???? FIX???
1$		btst.b	#ATB_BSY,AT_Status(a4)
		bne.s	1$

		addq.w	#2,sp			drop wait count
		jsr	_LVOWait(a6)		won't wait - fast clear
		moveq	#0,d0			found a drive!!!!
		subq.b	#1,hu_Found(a2)		make hu_Found non-zero
	IFD DEBUG_CODE
	move.b	hu_Found(a2),d0
	printf <'hu_Found set to $%lx, a2 = $%lx\n'>,d0,a2
	move.b	hu_ATDriveBit(a2),d0
	printf <'hu_ATDriveBit = $%lx\n'>,d0
	moveq	#0,d0
	ENDC
		rts

;==============================================================================
; Does the equivalent of SCSI's Inquiry command
; NOTE: Inquiry is always the first command called when searching for a drive
; a0 is	hu_CurrentCmd here, d0 is command byte, a1 is scsi command
;==============================================================================
XT_INQUIRY
	printf <'AT: Inquiry command\n'>
		moveq	#0,d1			not boottime

		; check if the drive exists if we haven't already
		tst.b	hu_Found(a2)		is this boot-time?
		bne.s	not_boottime		no
		bsr	ATCheckDrive		see if drive is there
		tst.b	d0
		bne	ATCommandDone		failed - punt

		; now, to handle somewhat confused drives, we send a
		; Initialize Drive Parameters command to tell it what it told
		;  me it was.  Really stupid, but the Conner 2024 needs this.
		moveq	#1,d1			boottime

	; FIX!!!! copy fixed buffer, then modify!
not_boottime:	move.w	d1,-(sp)		save boottime flag
		bsr	AT_IdentifyDrive	fills hu_ATBuffer
		move.w	(sp)+,d1
		tst.l	d0			d0=0 for success, d0=2 failure
		bne	ATCommandDone		failed, return with error

		; build inquiry response on the stack
		lea	hu_ATBuffer(a2),a0

	IFD USE_INIT_PARAM
		; set features to tell it what it's own geometry is (wierd)
		tst.w	d1			only do at boottime
		beq.s	no_init_param

		; Some drives (eg. Conner CP2024 2.5" 20MB) require this.
		; I believe this should NOT be needed according to the CAM-ATA
		; spec!  The Identify values should be for the "default
		; translation mode", so I say that means you shouldn't have to
		; do an inititialize drive parameters.  - REJ
		move.b	6*2(a0),AT_SectorCnt(a4) set sectors/track
		move.b	3*2(a0),d0		heads
		subq.b	#1,d0			it wants heads-1
		or.b	hu_ATDriveBit(a2),d0	add drive bit in
		move.b	d0,AT_DriveHead(a4)	set it
		move.b	#AT_INIT_PARAMETERS,AT_Command(a4)
		moveq	#0*2,d0			no data xfer
		move.l	a0,-(sp)		save ATBuffer ptr
		bsr	HandleInts (d0)		0 for success, 2 for failure
	printf <'InitParameters returned %ld\n'>,d0
		move.l	(sp)+,a0
		; ignore failure or success - what would we do anyways?
	ENDC

no_init_param:
		; set parameters on a Inquiry command, which is always done
		; before any access.  Might(?) hurt to do it more than once.
		tst.b	hu_MaxRWSize(a2)
		bne.s	no_set_multiple		already set it once
		move.b	47*2(a0),d0		low byte(0-7), word 47

		; limit max read/write multiple to give controllers a better
		; chance to make good use of their buffers (and overlap
		; bus transfers with disk transfers).  Tuning with a Seagate
		; ST1480A and a WD 2120 seem to indicate this is the best
		; compromise.
		cmp.b #16,d0
		bls.s 1$
	printf <'restricting max R/W to 16 from $%lx\n'>,d0
		moveq #16,d0
1$:
		move.b	d0,hu_MaxRWSize(a2)	max number of blocks for mult.
		beq.s	no_set_multiple		if not supported, skip set mult

		; send Set Multiple Mode to drive
		; can't do r/w multiple without it
		move.b	d0,AT_SectorCnt(a4)	use whatever it told me to
		move.b	hu_ATDriveBit(a2),AT_DriveHead(a4)	head irrelevant
		move.b	#AT_SET_MULTIPLE_MODE,AT_Command(a4)	start cmd
		moveq	#0*2,d0			non-data command
		move.l	a0,-(sp)		save reg (buffer ptr)
		bsr	HandleInts (d0)		returns 0 for success, 2 failure
		move.l	(sp)+,a0		restore reg
		tst.b	d0			did we fail?
		beq.s	no_set_multiple
		clr.b	hu_MaxRWSize(a2)	failed - nuke r/w multiple
	printf <'set multiple failed\n'>

no_set_multiple:
		moveq	#1*2,d0
		bsr	getATword (d0,a0)
		move.w	d0,hu_TotalCyls(a2)
		moveq	#0,d0
		moveq	#0,d1
		move.b	3*2(a0),d0		low byte first!
		move.b	d0,hu_TotalHeads(a2)
		move.b	6*2(a0),d1
		move.b	d1,hu_TotalBlocks(a2)	sectors, low byte first
		mulu	d0,d1
		move.w	d1,hu_CylSize(a2)	sectors * heads
		
	IFD DEBUG_CODE
	move.w	(a0),d0
	printf <'config bits: $%lx\n'>,d0
	move.w	hu_TotalCyls(a2),d0
	printf <'cylinders: $%lx\n'>,d0
	moveq	#0,d0
	move.b	hu_TotalHeads(a2),d0
	printf <'heads: $%lx\n'>,d0
	move.b	hu_TotalBlocks(a2),d0
	printf <'sectors/track: $%lx\n'>,d0
	move.b	hu_MaxRWSize(a2),d0
	printf <'max r/w multiple is %ld blocks\n'>,d0
	ENDC

		move.l	a2,-(sp)		free a register
		lea	-36(sp),sp		up to product revision level
		move.l	sp,a1
		moveq	#0,d1			keep a 0 around
		moveq	#0,d0			assume disk device

		; check device type
		btst.b	#6,(a0)			low word first!!!
		bne.s	10$
		moveq	#1,d0			tape device?? removeable disk??
;						; FIX!!!
10$		btst.b	#7,1(a0)		high word second
		beq.s	20$
		moveq	#7,d0			optical device???

20$		move.b	d0,(a1)+		0: peripheral qualifier/type
		move.b	(a0),d0			bits 0-7, word 0
		and.b	#$80,d0			leave bit 7 only - rmb
		move.b	d0,(a1)+		1: rmb/modifier
		moveq	#2,d0
		move.b	d0,(a1)+		2: ISO/ECMA/ANSI - scsi-2
		move.b	d0,(a1)+		3: scsi-2 response format
		move.b	#35-4,(a1)+		4: additional length
		move.b	d1,(a1)+		5: reserved
		move.b	d1,(a1)+		6: reserved
		move.b	#$20,(a1)+		7: WBus16 (16-bit xfers)

		; copy bytes from product up to first space or 8 chars max
		; the bytes end up swapped!!!!
		moveq	#7,d0
		lea	27*2(a0),a2		ptr to model number
30$		bsr.s	getswapbyte		returns byte in d1
		move.b	d1,(a1)+		8-15: Vendor
		subq.w	#1,d0
		bmi.s	40$
		cmp.b	#' ',d1			up to first space or 8 chars
		bne.s	30$
40$
		tst.w	d0
		bmi.s	60$			used 8 chars
		moveq	#' ',d1
50$		move.b	d1,(a1)+		pad with spaces
		dbra	d0,50$
60$
		; ok, vendor handled, now do product.
		moveq	#15,d0
70$		bsr.s	getswapbyte		returns byte in d1
		move.b	d1,(a1)+		16-31: product
		dbra	d0,70$

		; finally, revision
		moveq	#3,d0
		lea	23*2(a0),a2		firmware revision
80$		bsr.s	getswapbyte		returns byte in d1
		move.b	d1,(a1)+		32-35: revision
		dbra	d0,80$

;	IFD DEBUG_CODE
;	lea	8(sp),a0
;	moveq	#0,d0
;	move.w	(sp),d0
;	printf <' type/qual $%lx, ---%s--- \n'>,d0,a0
;	ENDC
		; ok, all done.  Copy it to destination and clean up
		moveq	#36,d0
		move.l	sp,a1			get ptr to block again
		bsr	copy_buffer (a1,d0)	copy to dest, set actual

		lea	36(sp),sp		drop block
		move.l	(sp)+,a2		restore reg
		moveq	#0,d0			success
		bra	ATCommandDone		scsi_actual set

getswapbyte:	; get next byte of byte-swapped string (a2), return in d1
		; bump a2, leave a1/d0 untouched
		move.w	a2,d1
		addq.w	#1,a2
		lsr.w	#1,d1
		bcs.s	was_odd
		move.b	(a2),d1			already bumped 1
		rts
was_odd:	move.b	-2(a2),d1		already bumped 1
		rts

;==============================================================================
; Does the equivalent of a SCSI request sense.  We don't have to go to the
; drive for this because the sense info was already fetched when the command
; completed.  It is held in hu_NextError ready for copying into the data area
; a3 is	hu_CurrentCmd here, d0 is command byte, a1 is scsi command
;==============================================================================
XT_RQS:		move.b	#UNIT_WAITING,hu_WhatNext(a2)  where to go next
		move.l	hu_NextError(a2),d0
	printf <'AT: error = 0x%lx\n'>,d0

		lea	ErrorTable(pc),a1
1$:		move.w	(a1),d1
		beq.s	found			0,0 is end of table
		and.w	d0,d1			and has a generic error (5,0)
		bne.s	found
		addq.w	#4,a1
		bra.s	1$
found:
		move.w	2(a1),d1		get key/code for error

; byte:   data:
;  0	   $70
;  1	    0
;  2	  <key>		Sense Key
;  3-6	    0		Information field, (address for error)
;  7	   $0a		additional length (18 bytes total, 0-17)
;  8-11	    0		Command-specific information (not given)
;  12	  <code>	Additional sense code
;  13	  <qual>	Additional sense qualifier
;  14-17    0		sense-key specific information (not given???? FIX!)
;
; Bytes: 2  12 13  (hex)
;
; BBK:   3? 11? 0	bad block mark detected??  FIX!
; UNC:   3  11 0	(Unrecovered read error)
; IDNF:  3  12 0	(ID address mark not found)
; AMNF:  3  13 0	(Data address mark not found)
; TK0NF: 4   6 0	(reference position not found)
; ABRT:  5  20 0	(invalid command)
; bit 8: 5  21 0	(LBA address out of range)
; bit 9: 2   5 0	(Not Ready, LUN doesn't respond)
; bit10: 4  42 0	(Power On or Self-Test failure)
; other: 5   0 0	(Illegal request, no more info)
;
		; build sense data on stack
		lea	-18(sp),sp		get space on stack
		move.l	sp,a1			dest for template
		moveq	#18,d0			# of chars
		bsr	bzero (a1,d0)		all 0's (doesn't 

		; fix up the two bytes we modify, and two fixed bytes
		move.b	d1,2(a1)
		lsr.w	#8,d1
		move.b	d1,12(a1)
		move.b	#$70,(a1)
		move.b	#$0a,7(a1)

		; add information field (block of error if any)
		lea	3(a1),a0
		exg	a0,a1			a0 will be preserved
		move.l	hu_ErrorLBA(a2),d0	block of last error, if any
		PUTLONG_A1			; preserves a0 (only)

		move.l	a0,a1
		moveq	#18,d0			length of data
		bsr	copy_buffer (a1,d0)	copy and set up scsi fields
;						also sets actual

		lea	18(sp),sp		drop data buffer
		moveq	#0,d0			success
		bra	ATCommandDone		assumes we set scsi_Actual

;
; layout: (word) mask for error register, followed by code, and key for
; the sense data.  Qualifier is always 0 thus far.
ErrorTable:
	DC.B	$00,$01,$13,$03	; AMNF - Data Address Mark not found
	DC.B	$00,$02,$06,$04	; TK0NF - reference position not found
	DC.B	$00,$04,$20,$05	; ABRT - invalid command
	DC.B	$00,$10,$12,$03	; IDNF - ID address mark not found
	DC.B	$00,$40,$11,$03	; UNC - unrecovered read error
	DC.B	$00,$80,$11,$03	; BBK - bad block mark detected??? FIX!!!!
	DC.B	$01,$00,$21,$05	; bit 8 - LBA address out of range
	DC.B	$02,$00,$05,$02	; bit 9 - Not Ready, LUN doesn't respond
	DC.B	$04,$00,$42,$04	; bit 10 - Power On or Self-Test failure
	DC.B	$00,$00,$00,$05	; unknown - Illegal request, no more info

	CNOP	0,2
	
;==============================================================================
; Does a format unit with no special flags or bad block lists sent to the drive
; a3 is	hu_CurrentCmd here, d0 is command byte, a1 is scsi command
;==============================================================================
XT_FMT
	printf <'AT: Format_Unit called\n'>
		; send a format command for each track(!), with a data
		; field of mapped sectors (or 0's).  If we're ambitious, 
		; translate scsi format error lists.

		movem.l	d2-d7,-(sp)		save reg

		; save some fields, make a scsi control block
		move.l	cb_WData(a3),-(sp)	save old data ptr
		move.l	cb_LinkedCmd(a3),-(sp)	save old scsiblk ptr
		lea	-scsi_SIZEOF(sp),sp	allocate a SCSICmd struct
		move.l	sp,cb_LinkedCmd(a3)	new scsicmd
		move.l	#512,d0
		move.l	d0,scsi_Length(sp)	format data is 512 long

		move.w	hu_TotalCyls(a2),d4	number of cyls
		move.b	hu_TotalHeads(a2),d5	number of heads
		move.b	hu_ATDriveBit(a2),d6	starting head
		move.b	hu_TotalBlocks(a2),d7	number of heads

		lea	hu_ATBuffer(a2),a1
		move.w	#512,d0
		bsr	bzero  (d0,a1)		clear buffer

		; don't mark any sectors as bad.  Must put each sector in the
		; data buffer, followed by a 0 (sectors 1-N).  Order is
		; irrelevant, but we'll put it in reverse order.
		; it had better not have more than 128 sectors!
		lea	hu_ATBuffer(a2),a1
		move.b	d7,d0			number of sectors/track
sec_loop:	move.b	d0,(a1)+
		clr.b	(a1)+			good sector
		subq.b	#1,d0
		bne.s	sec_loop		only 1 to N, not 0 to N-1

		moveq	#0,d2			starting cylinder
		move.l	d6,d3			start at head 0 (set drive bit)
	printf <'Format: %ld cyls, %ld heads, %ld sectors\n'>,d4,d5,d7
cyl_loop:
	printf <'formatting cyl $%lx, head $%lx\n'>,d2,d3
		move.b	#1,cb_ATLength(a3)	1 sector of data to xfer
		lea	hu_ATBuffer(a2),a1	get the data pointer
		move.l	a1,cb_WData(a3)		use our fixed buffer
		clr.l	scsi_Actual(sp)		reset to nothing xfered yet
		move.w	d2,d0
		move.b	d0,AT_CylLow(a4)
		lsr.w	#8,d0
		move.b	d0,AT_CylHigh(a4)
		move.b	d3,AT_DriveHead(a4)
		move.b	d7,AT_SectorCnt(a4)
		move.b	#AT_FORMAT,AT_Command(a4)	- start format
		moveq	#2*2,d0			initial state - write
		bsr	HandleInts (d0)		transfer the data, wait for int
		tst.b	d0			failure?
		bne.s	format_done
		and.b	#$0f,d3
		addq.b	#1,d3
		cmp.b	d3,d5			do we need to change cyls?
		bgt.s	1$
		addq.w	#1,d2			yes, bump cyl, clear head
		cmp.w	d2,d4			are we done? (unsigned)
		bls.s	format_done		d0 == 0 here
		moveq	#0,d3
1$:		or.w	d6,d3			or in the drive select bit
		bra.s	cyl_loop

format_done:	; d0 has success/failure
	printf <' format done, result %ld\n'>,d0
		lea	scsi_SIZEOF(sp),sp	drop scsi block
		move.l	(sp)+,cb_LinkedCmd(a3)	restore scsiblk ptr
		move.l	(sp)+,cb_WData(a3)	restore data ptr
		movem.l	(sp)+,d2-d7		restore regs
		
		bra	ATCommandDone

;==============================================================================
; Does the equivalent of a read capacity command.  This one just grabs the
; unit fields and returns them.  We must make sure they're initialized at
; startup time, either via rdsk or via Identify Drive.
; a3 is	hu_CurrentCmd here, d0 is command byte, a1 is scsi command
;==============================================================================
XT_RDC:
	printf <'Read Capacity called\n'>
		moveq	#0,d0
		moveq	#0,d1
		btst.b	#0,8(a1)		test RELADR bit of command
		bne.s	rdc_relative
		move.b	hu_TotalBlocks(a2),d0	actually sectors/track
		move.b	hu_TotalHeads(a2),d1	heads
		mulu	d1,d0			sectors*heads (16 bits max)
		move.w	hu_TotalCyls(a2),d1	cyls
		mulu	d1,d0			sectors*heads*cyls
		subq.l	#1,d0			LBA of last sector
		bra.s	rdc_store_result
rdc_relative:
;	printf <'relative read_capacity\n'>
		; find next cylinder boundary - a1 has scsi cmd block
		; new LBA = (((LBA)/CylSize) + 1)*CylSize - 1

		move.l	2(a1),d0		get LBA from aligned cmd block
		move.w	hu_CylSize(a2),d1	get cylinder size
		divu.w	d1,d0			LBA/cylsize = cyls
		addq.w	#1,d0			next cylinder
		mulu	hu_CylSize(a2),d0	get LBA of 1st sec of next cyl
		subq.l	#1,d0			LBA of last sec of curr cyl
;						(d0 is now longword)

rdc_store_result:
	printf <'rdc size: %ld blocks\n'>,d0
		move.l	#512,-(sp)		always 512 byte blocks
		move.l	d0,-(sp)		LBA
		moveq	#8,d0			# of bytes of data
		move.l	sp,a1			pointer to source buffer
		bsr	copy_buffer (a1,d0)	a1->cb_WData(a3), sets actual
		addq.w	#8,sp			drop them

		moveq	#0,d0			success
		bra	ATCommandDone		assumes we set scsi_Actual

;==============================================================================
; this does an AT recalibrate command.
; a3 is	hu_CurrentCmd here, d0 is command byte, a1 is scsi command
;==============================================================================
XT_RECAL:
	printf <'Recalibrate called\n'>
		move.b	hu_ATDriveBit(a2),AT_DriveHead(a4)	head irrelevant

		; d0 is AT_RECALIBRATE
		bra	ATSendCommand

;==============================================================================
; this does an AT recalibrate command.
; a3 is	hu_CurrentCmd here, d0 is command byte, a1 is scsi command
;==============================================================================
XT_READ_VERIFY:
	printf <'Verify called\n'>
; FIX! handle BytChk!!
		; note: ATSetAddress can exit to AT_OutOfRange, must have LW
		; on stack plus return addr.
		move.w	#AT_READ_VERIFY_SECTORS,-(sp)  IMPORTANT! (don't move)
		move.l	2(a1),d0		get LBA of start from cmd
		bsr	ATSetAddress		may exit to OutOfRange
						; a0 is preserved

		move.l	cb_WCommand(a3),a1	get scsi cmd back
		move.b	7(a1),d1		d2 is saved
		lsl.w	#8,d1
		move.b	8(a1),d1		got length in blocks
		move.w	(sp)+,d0		command
		bsr	LongXfer		do long io

		bra	ATCommandDone  (d0)

;==============================================================================
; this does an AT identify drive command, which is needed for things like
; initializing the hu_TotalXxxx fields, Inquiry, etc.  The data is stored
; in hu_Buffer.  We must save and restore several ptrs.
;==============================================================================
AT_IdentifyDrive:
	printf <'Identify Drive called\n'>
		; Also, identify is an optional command.  An idea: we can
		; try reading higher sectors until failure, then higher heads,
		; then higher cylinders.  Icky-poo.

		move.b	hu_ATDriveBit(a2),AT_DriveHead(a4)	head irrelevant

		; save some fields, make a scsi control block
		move.l	cb_WData(a3),-(sp)	save old data ptr
		move.l	cb_LinkedCmd(a3),-(sp)	save old scsiblk ptr
		lea	-scsi_SIZEOF(sp),sp	allocate a SCSICmd struct
		move.l	sp,cb_LinkedCmd(a3)	new scsicmd
		move.l	#512,d0
		move.l	d0,scsi_Length(sp)	identify data is 512 long
		clr.l	scsi_Actual(sp)		nothing xfered yet

		lea	hu_ATBuffer(a2),a0
		move.l	a0,cb_WData(a3)		use our fixed buffer
		move.b	#1,cb_ATLength(a3)	1 sector to xfer

		move.b	#AT_IDENTIFY_DRIVE,AT_Command(a4)	kick it off
		moveq	#1*2,d0			initial state (read data)
		bsr	HandleInts (d0)

		; restore stuff
		lea	scsi_SIZEOF(sp),sp	drop scsi command block
		move.l	(sp)+,cb_LinkedCmd(a3)	restore scsiblk ptr
		move.l	(sp)+,cb_WData(a3)	restore data ptr

	IFD CP2024_KLUDGE
		; we must recognize the CP2024 explictly.  It ignores the
		; InitializeDriveParams command, and comes up in translate
		; mode (not what it returns from identify!)
		movem.l	d0/a2/a3,-(sp)
		lea	hu_ATBuffer(a2),a0
		move.l	a2,a3			save hu ptr
		lea	27*2(a0),a2		ptr to model number
		lea	ConnerName(pc),a1
		moveq	#31,d0			strlen(ConnerName)
1$:		bsr	getswapbyte		d1:byte from (a2)+ (sort of)
		cmp.b	(a1)+,d1
		dbne	d0,1$			loop until not eq or done
		addq.w	#1,d0
		bne.s	not_cp2024

		; ick, it's a cp2024
		; force it to 615/4/17 by modifying the returned structure
		; the V2.08 CP2024 can't handle more than 16 sectors/io!!!
		move.w	#((615<<8)&$ff00)!(615>>8),1*2(a0)	byte-swapped
		move.b	#4,3*2(a0)		4 heads (high byte is 0)
		move.b	#17,6*2(a0)		17 sectors/track
; this was needed to attempt to fix (unsuccessfully) the 2.08 conner
;		move.l	#16,hu_MaxBlocks(a3)	we saved hu ptr in a3!

 printf <'Evil Conner CP2024!\n'>
not_cp2024:
		movem.l	(sp)+,d0/a2/a3
	ENDC

		; d0 = 0 for success, d0 = 2 for failure
		rts

ConnerName:	dc.b	'Conner Peripherals 20MB - CP2024'
		ds.w	0

;==============================================================================
; copy_buffer: (a1,d0) copy up to d0 bytes from a1 to cb_WData(a3)
; update scsi_Actual in scsi block, set scsi_CmdActual = scsi_CmdLength,
; clears scsi_Status.
;==============================================================================
copy_buffer:
		move.l	a1,d1			save source ptr
		move.l	cb_SCSICmd(a3),a1
		cmp.l	scsi_Length(a1),d0
		bls.s	length_bigger
		move.l	scsi_Length(a1),d0	min(18,length)
length_bigger:
		move.l	d0,scsi_Actual(a1)

		move.l	d1,a1			get ptr to ssource again
		move.l	cb_WData(a3),a0		store in data area
		bra.s 2$
1$:		move.b	(a1)+,(a0)+		copy up to d0 bytes from a1->a0
2$:		dbra	d0,1$

		rts
 

	IFD FOR_68000_ONLY
 IFD UNUSED
;==============================================================================
; gets a possibly misaligned longword from (a1), returns it in d0.  No
; other regs touched.  A1 incremented by 4!
; NOT NEEDED ON '020 up!
;==============================================================================
getlong_a1:
		move.l	a1,d0			test for misalignment
		btst.b	#0,d0			is it an odd byte?
		bne.s	1$
		move.l	(a1)+,d0		already aligned (word or lw)
		rts

1$:		move.b	(a1)+,d0		mis-aligned (byte aligned)
		swap	d0			into top half
		move.w	(a1)+,d0		therefor this must be aligned
		lsl.l	#8,d0			make room for last byte
		move.b	(a1)+,d0
		rts
 ENDC UNUSED

;==============================================================================
; puts a possibly misaligned longword from d0 into (a1)+.  Other regs
; are touched (a1 is incremented by 4, d0/d1 are destroyed, a0 untouched).
; NOT NEEDED ON '020 up!
;==============================================================================
putlong_a1:
; commented out, since we only call this for request sense, and we know it's
; non-aligned there - fix?
;		move.l	a1,d1
;		btst.b	#0,d1
;		bne.s	1$
;		move.l	d0,(a1)+		word or lw aligned
;		rts

1$:		move.b	d0,3(a1)		(low byte) byte-misaligned
		lsr.l	#8,d0
		move.w	d0,1(a1)		must be word or lw aligned
		swap	d0			get last byte into low end
		move.b	d0,(a1)
		addq.w	#4,a1			auto extends to long
		rts

	ENDC FOR_68000_ONLY

;==============================================================================
; ATSendCommand (d0.b: command)
;
; This routine starts the command and handles any interrupts that occur, and
; on completion or error cleans up the command and goes back to waiting.
;
;==============================================================================
ATSendCommand
		; send the command
	IFD DEBUG_CODE
2$:	move.b	AT_Status(A4),d1
	btst.b	#ATB_BSY,d1
	bne.s	1$
	btst.b	#ATB_RDY,d1
	bne.s	3$
1$:	and.l	#$ff,d0
	and.l	#$ff,d1
	printf <'DANGER: status $%lx before cmd $%lx\n'>,d1,d0
	bra.s	2$
3$:
	ENDC
		move.b	d0,AT_Command(a4)	start command
		bsr.s	FindInitState (d0)
		bsr.s	HandleInts (d0)		handle interrupts
;						returns d0 = 0, or d0 = 2
		bra	ATCommandDone

;==============================================================================
; FindInitState (d0.b: command)
;
; returns the initial state for HandleInts for the command in d0
;==============================================================================
FindInitState:
		; get initial state*2 from table
		lea.l	AT_InitialState(pc),a1	scan table for the command
10$		move.w	(a1)+,d1		get command and state
; never can happen: beq	ATBadCmd		sorry, we don't handle it!!!!
	IFD DEBUG_CODE
	    bne.s 1$
	    printf <'IMPOSSIBLE - couldnt find state for $%lx\n'>,d0
1$:
	ENDC
		cmp.b	d1,d0			is it this command
		bne.s	10$			nope, keep looking
		lsr.w	#8,d1			initial state (times 2)!
		move.l	d1,d0
		rts

; This is a table of initial states for different AT commands  They're
; multiplied by 2 so they can be used as an offset into the dispatch tables.

AT_InitialState:
		DC.B	1*2,AT_READ_SECTORS		read
		DC.B	1*2,AT_READ_MULTIPLE		read
		DC.B	2*2,AT_WRITE_SECTORS		write
		DC.B	2*2,AT_WRITE_MULTIPLE		write
		DC.B	1*2,AT_IDENTIFY_DRIVE		read
		DC.B	2*2,AT_FORMAT			write
		DC.B	0*2,AT_SEEK			non-data
		DC.B	0*2,AT_RECALIBRATE		non-data
		DC.B	0*2,AT_INIT_PARAMETERS		non-data
		DC.B	0*2,AT_SET_MULTIPLE_MODE	non-data
		DC.B	0*2,AT_READ_VERIFY_SECTORS	non-data
		DC.B	0*2,AT_DIAGNOSTICS		non-data
; more?
		DC.B	0,0				end
		
;==============================================================================
; HandleInts (d0: 2*state)
;
; This handles any interrupts that occur, and on completion or error returns
; to the caller.  Result is in d0, 0 for success or 2 for failure.  
;
; When an interrupt occurs from the AT drive, control is passed back to here.
; We read the status register, and use it with the current state to figure
; out what to do.  We have 4 possibilities:
; 0 - non-data command completed successfully
; 1 - a block of data is ready to be read
; 2 - the drive is ready to accept another block of data for a write
; 3 - the drive has accepted the final sector of a write.
;
; In cases 1 and 2, we go to a routine to dump or get a sector of data, then
; wait for another interrupt.  If there was an error, we read the error reg,
; set up hu_NextError for the next request sense call, and return control
; to the routine that started the transfer.
;
; In cases 1 and 3, if this is the final sector of a transfer, we then return
; control to the routine that was specified by the routine that started the
; transfer (usually a "clean up and return" routine).
;
; We have a separate table of routines if there was an error.  The error
; register is put in hu_ATError.
;==============================================================================
HandleInts:	movem.l	d2-d3,-(sp)
		move.l	d0,d2			initial state
		clr.b	d3			count of sectors since int

; IFD DEBUG_CODE
; cmp.b #2*2,d2
; bne.s 1$
; moveq #0,d0
; move.b cb_ATLength(a3),d0
; printf <'writing %ld blocks\n'>,d0
;1$
; ENDC
		; on a write, transfer a sector first before waiting!
		cmp.b	#2*2,d2			was initial state write?
		beq.s	after_wait

TransferLoop:
		tst.b	d3			should we wait for an int now?
		bne.s	after_wait		no, we're in a multiple xfer
		move.l	st_IntPendMask(a5),d0
		jsr	_LVOWait(a6)

after_wait:
		move.b	AT_Status(a4),d0
;    IFD DEBUG_CODE
;    and.l #$ff,d0
;    printf <'AT: handle int, state = %ld, status = $%lx\n'>,d2,d0
;    ENDC

	IFD WD_AT_KLUDGE
; FIX!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; EVIL, ugly kludge for WD drives!  They return an INT immediately!
; However, the spec may require this for all drives (sec 10.2, CAM-ATA R23)

	cmp.b	#1*2,d2
	beq.s	10$
	cmp.b	#2*2,d2			don't wait if not read, or write-not-
	bne.s	20$			last-sector.
10$:
	IFD DEBUG_CODE
	and.l	#$ff,d0
	btst.b	#ATB_BSY,d0
	bne.s	1$
	btst.b	#ATB_DRQ,d0
	bne.s	2$
1$:
	; busy or ready but no drq
	printf <'Int: status $%lx, state %ld\n'>,d0,d2
2$:
	ENDC

	btst.b	#ATB_BSY,d0
	bne.s	after_wait
	ENDC

		; check _after_ wait for not BSY!
		btst.b	#ATB_ERR,d0		was there an error?
		bne.s	cmd_err			handle error on command

	IFD WD_AT_KLUDGE
	btst.b	#ATB_DRQ,d0			wait for data ready!
	beq.s	after_wait
20$:
	ENDC
		move.w	AT_Int(pc,d2.w),d0
		jmp	AT_Int(pc,d0.w)		no error

; the drive reported an error on the last operation, save sense info
; no need to save hu_ATStatus, since it just has the ERR bit.
cmd_err:
		move.b	AT_Error(a4),hu_NextError+3(a2)
	printf <'AT: error was 0x%lx, state %ld\n'>,hu_NextError(a2),d2

		move.w	AT_ErrTab(pc,d2.w),d0
		jmp	AT_ErrTab(pc,d0.w)	error

; Dispatch tables
AT_Int:		DC.W	AT_Int_NoData-AT_Int	non-data command
		DC.W	AT_Int_Read-AT_Int	read next block of data
		DC.W	AT_Int_Write-AT_Int	write next block of data
		DC.W	AT_Int_FinalWrite-AT_Int final int after write

AT_ErrTab:	DC.W	AT_Err_NoData-AT_ErrTab	    non-data command
		DC.W	AT_Err_Read-AT_ErrTab	    read next block of data
		DC.W	AT_Err_Write-AT_ErrTab	    write next block of data
		DC.W	AT_Err_FinalWrite-AT_ErrTab final int after write

;==============================================================================
; command without data completed successfully, return
;==============================================================================
AT_Int_NoData:
		; fall through...
;==============================================================================
; Final sector of a write succeeded
;==============================================================================
AT_Int_FinalWrite:
	IFD WD_AT_KLUDGE
		; wait for WD drive to finish writing for real
1$:		move.b	AT_Status(a4),d0
	   IFD DEBUG_CODE
		btst.b  #ATB_BSY,d0
		beq.s	2$
		and.l	#$ff,d0
		printf <'FinalWrite: status $%lx\n'>,d0
2$:
	   ENDC
		btst.b	#ATB_BSY,d0
		bne.s	1$
	ENDC
		moveq	#0,d0
		movem.l	(sp)+,d2-d3
		rts

;==============================================================================
; Read a sector from the AT interface, store in destination
;==============================================================================
AT_Int_Read:
		move.l	cb_WData(a3),a1		get working data ptr
		lea	AT_Data(a4),a0		source
	IFD FOR_68000_ONLY
		move.l	a1,d1			is this byte-aligned?
		btst.b	#0,d1
		bne.s	byte_read
	ENDC
		moveq	#(256/16)-1,d0		16 words at a time

; FIX!!!! for A1000+, use move.w (a0),dn, swap dn, move.w (a0),dn, ... movem...
; OR, use move.w (a0),dn, swap dn, move.w (a0),dn, move.l dn,(a1)+...
1$:		move.w	(a0),(a1)+
		move.w	(a0),(a1)+
		move.w	(a0),(a1)+
		move.w	(a0),(a1)+
		move.w	(a0),(a1)+
		move.w	(a0),(a1)+
		move.w	(a0),(a1)+
		move.w	(a0),(a1)+
		move.w	(a0),(a1)+
		move.w	(a0),(a1)+
		move.w	(a0),(a1)+
		move.w	(a0),(a1)+
		move.w	(a0),(a1)+
		move.w	(a0),(a1)+
		move.w	(a0),(a1)+
		move.w	(a0),(a1)+
		dbra	d0,1$

read_cleanup
		move.l	a1,cb_WData(a3)		put working data ptr back
		movea.l	cb_LinkedCmd(a3),a0	and corresponding SCSICmd
		add.l	#512,scsi_Actual(a0)	update count of bytes xfered

		; used to compare scsi_Length and Actual - doesn't work for
		; extended r/w.  FIX! what about scsi_Length < # of blocks in
		; scsi command?
		addq.b	#1,d3			# of sectors since int
		cmp.b	hu_MaxRWSize(a2),d3
		bcs.s	1$			branch on d3 < MaxRWSize
		moveq	#0,d3			time to wait...
1$		subq.b	#1,cb_ATLength(a3)	careful - works right for 0/256
		bne	TransferLoop		wait for next sector

;	printf <'read $%lx of $%lx bytes\n'>,scsi_Actual(a0),scsi_Length(a0)
		; done with read, return to caller with success
		moveq	#0,d0
		movem.l	(sp)+,d2-d3
		rts

	IFD FOR_68000_ONLY
byte_read:
		moveq	#(256/2)-1,d0		2 words at a time
		
1$:		move.w	(a0),d1
		swap	d1
		move.w	(a0),d1			get longword
		move.b	d1,3(a1)		low byte
		lsr.l	#8,d1
		move.w	d1,1(a1)		middle word
		swap	d1
		move.b	d1,(a1)			high byte
		addq.w	#4,a1
		dbra	d0,1$

		bra.s	read_cleanup
	ENDC

;==============================================================================
; Write a sector to the AT interface, from source.  If last sector, change
; to state 3 (final interrupt pending).
;==============================================================================
AT_Int_Write:
		move.l	cb_WData(a3),a1		get working data ptr
		lea	AT_Data(a4),a0		destination
	IFD FOR_68000_ONLY
		move.l	a1,d1			is this byte-aligned?
		btst.b	#0,d1
		bne.s	byte_write
	ENDC
		moveq	#(256/16)-1,d0		16 words at a time

; FIX!!!! for A1000+, use movem... move.w dn,(a0), swap dn, move.w dn,(a0)...
; OR, use move.l (a1)+,dn, move.w (a0),dn, swap dn, move.w (a0),dn...
1$:		move.w	(a1)+,(a0)
		move.w	(a1)+,(a0)
		move.w	(a1)+,(a0)
		move.w	(a1)+,(a0)
		move.w	(a1)+,(a0)
		move.w	(a1)+,(a0)
		move.w	(a1)+,(a0)
		move.w	(a1)+,(a0)
		move.w	(a1)+,(a0)
		move.w	(a1)+,(a0)
		move.w	(a1)+,(a0)
		move.w	(a1)+,(a0)
		move.w	(a1)+,(a0)
		move.w	(a1)+,(a0)
		move.w	(a1)+,(a0)
		move.w	(a1)+,(a0)
		dbra	d0,1$

write_cleanup:
		move.l	a1,cb_WData(a3)		put working data ptr back
		movea.l	cb_LinkedCmd(a3),a0	and corresponding SCSICmd
		add.l	#512,scsi_Actual(a0)	update count of bytes xfered

		; used to compare scsi_Length and Actual - doesn't work for
		; extended r/w.  FIX! what about scsi_Length < # of blocks in
		; scsi command?  Screw 'em.
		addq.b	#1,d3			# of sectors since int
		cmp.b	hu_MaxRWSize(a2),d3
		bcs.s	1$			branch on d3 < MaxRWSize
		moveq	#0,d3			time to wait...
1$:		subq.b	#1,cb_ATLength(a3)	careful - works right for 0/256
		bne	TransferLoop		wait for drive to be ready

;	printf <'wrote $%lx of $%lx bytes, cb_WData $%lx\n'>,scsi_Actual(a0),scsi_Length(a0),cb_WData(a3)
		; done with write, change state and wait for confirming int
		moveq	#0,d3			force it to wait for int
		moveq	#3*2,d2			state 3 - wait for final int
		bra	TransferLoop

 
	IFD FOR_68000_ONLY
byte_write:
		moveq	#(256/2)-1,d0		2 words at a time
		
1$:		move.b	(a1),d1
		swap	d1
		move.w	1(a1),d1
		lsl.l	#8,d1
		move.b	3(a1),d1
		swap	d1
		move.w	d1,(a0)			may be faster to use all bytes
		swap	d1
		move.w	d1,(a0)
		addq.w	#4,a1
		dbra	d0,1$

		bra.s	write_cleanup
	ENDC

;==============================================================================
; Write of a sector failed.  Adjust count and return an error
;==============================================================================
AT_Err_Write:
		; fall through!

;==============================================================================
; Final sector of a write failed.  Adjust count to not include it and return.
;==============================================================================
AT_Err_FinalWrite:
		move.l	cb_LinkedCmd(a3),a0	get scsi command block
		sub.l	#512,scsi_Actual(a0)	last block had an error
		; fall through!

;==============================================================================
; command without data failed, return
;==============================================================================
AT_Err_NoData:
		; fall through!

;==============================================================================
; Read of a sector failed.  return an error
;==============================================================================
AT_Err_Read:
		bsr.s	GetErrorAddr		returns check_condition
		movem.l	(sp)+,d2-d3
		rts

;==============================================================================
; get the address currently in the AT regs, and store in hu_ErrorLBA
; returns CHECK_CONDITION in d0
;==============================================================================
GetErrorAddr:
	IFD DEBUG
		moveq	#0,d0
	ENDC
		; save where error occurred
		move.b	AT_CylHigh(a4),d0
		lsl.w	#8,d0
		move.b	AT_CylLow(a4),d0
	printf <'err cyl %ld\n'>,d0
		move.w	hu_CylSize(a2),d1
		mulu	d0,d1			cyls * cylsize
		move.l	d1,-(sp)		save it
		move.b	AT_DriveHead(a4),d0
		and.w	#$000f,d0		only want the head
	printf <'err head %ld\n'>,d0
		moveq	#0,d1
		move.b	hu_TotalBlocks(a2),d1	sectors/track
		mulu	d0,d1			heads * sectors/head
		add.l	(sp)+,d1		+ cyls * sectors/cyl
		move.b	AT_SectorNum(a4),d0	sector number (1-N)
	printf <'err secnum %ld\n'>,d0
		subq.b	#1,d0
		add.l	d0,d1			+ sector number

	printf <'error was at block %ld\n'>,d1
		; d1 has sector of error (if any)
		move.l	d1,hu_ErrorLBA(a2)	save where the error happened
		moveq	#CHECK_CONDITION,d0	return bad status
		rts

;==============================================================================
; When a command is completed (success or error), control comes here.
; D0 has the status code (normally 0 for success, or 2 for failure).  We
; always terminate the command at this point because there should be nothing
; else to do, it either failed or it worked.  The error handling in IOTask is
; responsible for figuring out which.
;==============================================================================
ATCommandDone
;	printf <'AT: command complete, status = %ld\n'>,d0
		movea.l	cb_WStatus(a3),a1	and the status area
		move.b	d0,(a1)			save error code (0 or 2)

; used to set actual == length here.  We maintain actual during the xfer,
; so there's no need to kludge it here
		movea.l	cb_SCSICmd(a3),a1
		move.w	scsi_CmdLength(a1),scsi_CmdActual(a1)
		move.b	#UNIT_WAITING,hu_WhatNext(a2)    which queue next
		bra	FakeDisconnect		disconnect and reply IORequest


; make external entry points have the same names as the A590/XT driver
XTSelect	EQU	ATSelect
XTSendCommand	EQU	ATSendCommand
XTCommandDone	EQU	ATCommandDone


	ENDC IS_IDE
		END
