		SECTION	filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/io.i"
		INCLUDE	"exec/memory.i"
		INCLUDE	"exec/interrupts.i"
		INCLUDE	"devices/timer.i"
		INCLUDE	"devices/inputevent.i"
		INCLUDE	"libraries/dosextens.i"
		INCLUDE	"moredos.i"
		INCLUDE	"globals.i"
		INCLUDE	"private.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

SPARESTACK EQU 2048	use a different stack for opening the device

		XDEF	Init,GetBuffers,GeometryInit

		XREF	_LVOFreeMem,_LVOAllocMem,_LVOOpenDevice
		XREF	_LVOOpenLibrary,_LVOCloseLibrary
		XREF	_LVOAllocSignal,_LVOFreeSignal,_LVOAddPort
		XREF	GetPubMem,_LVOCopyMem,_LVOCloseDevice
		XREF	InitCo,CreateCo,CallCo,DeleteCo,Restart
		XREF	CompString

		XREF	Disk,Kill		;co-routines

;==============================================================================
; Success = Init(packet),_AbsExecBase
;   d0		   a0	      a6
;
; Initialises globals, allocates required memory and makes filesystem public
; if nothing failed during the initialisation.  Global memory is assumed to
; start with 0 (FALSE) in all slots.
;==============================================================================
Init		movem.l	a2-a4,-(sp)
		movea.l	dp_Arg2(a0),a2		device specific info
		adda.l	a2,a2			it was a BPTR
		adda.l	a2,a2
		movea.l	fs_Env(a2),a3		environment vector
		adda.l	a3,a3			it was a BPTR too
		adda.l	a3,a3
		move.l	a3,CurrentEnv(a5)	stash for Restart
		movea.l	dp_Arg3(a0),a4		Devlist entry
		adda.l	a4,a4			another goddamn BPTR!!
		adda.l	a4,a4
		move.l	a4,DevEntry(a5)		save devicenode entry

; set up global information about the device we are going to be using.  Watch
; out for the device name, it's a NULL terminated BCPL string, yuck!
		move.l	fs_Unit(a2),UnitNumber(a5)
		move.l	fs_Flags(a2),OpenFlags(a5)
		move.l	fs_DevName(a2),DeviceName(a5)	**** BSTR here ****

		moveq.l	#TRUE,d0		flag timer and disk not open
		move.l	d0,TimerOpened(a5)
		move.l	d0,DiskOpened(a5)

; initialise all of the cache buffer list headers to be empty
		lea.l	ValidBufferQueue(a5),a0
		NEWLIST	a0
		lea.l	PendingQueue(a5),a0
		NEWLIST	a0
		lea.l	WaitingQueue(a5),a0
		NEWLIST	a0
; and the dir semaphore list
		lea.l	DirSemList(a5),a0
		NEWLIST	a0

; get all the memory we are going to need and store pointers in relevant places
		move.l	a4,-(sp)		stash devicelist pointer
		lea.l	MemRequired(pc),a4
10$		moveq.l	#0,d0
		move.w	(a4)+,d0		get size
		beq.s	GotMemOK		last entry in table
		bsr	GetPubMem		get clear public memory
		move.w	(a4)+,d1		get offset to store at
		move.l	d0,0(a5,d1.w)		and store mem pointer
		bne.s	10$			no problems, get next
		addq.l	#4,sp			error, scrap value on stack
		bra	InitFail		and free whatever we did get
GotMemOK	move.l	(sp)+,a4		restore devlist pointer

	printf <'EnvecSize       = %ld\n'>,EnvecSize(a3)
	printf <'SectorSize      = %ld\n'>,SectorSize(a3)
	printf <'SectorOrigin    = %ld\n'>,SectorOrigin(a3)
	printf <'Surfaces        = %ld\n'>,Surfaces(a3)
	printf <'SectorsPerBlock = %ld\n'>,SectorsPerBlock(a3)
	printf <'BlocksPerTrack  = %ld\n'>,BlocksPerTrack(a3)
	printf <'ReservedBlocks  = %ld\n'>,ReservedBlocks(a3)
	printf <'PreAllocated    = %ld\n'>,PreAllocated(a3)
	printf <'Interleave      = %ld\n'>,Interleave(a3)
	printf <'LowerCylinder   = %ld\n'>,LowerCylinder(a3)
	printf <'UpperCylinder   = %ld\n'>,UpperCylinder(a3)
	printf <'Buffers         = %ld\n'>,Buffers(a3)
	printf <'BuffMemType     = %ld\n'>,BuffMemType(a3)
	printf <'MaxTransfer     = %ld\n'>,MaxTransfer(a3)
	printf <'AddressMask     = %lx\n'>,AddressMask(a3)

; fail the initialisation if this isn't a DOSType that the fs supports
		moveq.l	#16,d0			size is # longwords
		cmp.l	EnvecSize(a3),d0	see if there is a DOStype entry
		bgt.s	NoDOSType

		move.l	dostype(a3),d0
		clr.b	d0
		cmpi.l	#'DOS'<<8,d0
		bne	InitFail

NoDOSType	bsr	GeometryInit
		tst.l	d0
		beq	InitFail

; this fixes a strange bug.  If both the filesystem and the device are loaded
; from disk, a lot of stack get's eaten up.  Switch to a new stack while we
; open the device we are going to be using.  If we don't get the memory then
; fail this packet, it's probably going to fail soon anyway.
	IFD SPARESTACK
		move.l	#SPARESTACK,d0
		moveq.l	#MEMF_PUBLIC,d1
		jsr	_LVOAllocMem(a6)
		tst.l	d0
		beq	InitFail
		exg.l	d0,sp			switch to new stack
		lea.l	2048(sp),sp		point to the end
		move.l	d0,-(sp)		save old stack
	ENDC

		move.l	DeviceName(a5),d0	open the correct device
		lsl.l	#2,d0			remember its null terminated
		addq.l	#1,d0			but still a BSTR
		movea.l	d0,a0
		movea.l	DiskDev(a5),a1
		move.l	UnitNumber(a5),d0
		move.l	OpenFlags(a5),d1
	printf <'OpenDevice("%s,%ld,%ld) ... '>,a0,d0,d1
		jsr	_LVOOpenDevice(a6)
	printf <'returned %ld\n'>,d0
		move.l	d0,DiskOpened(a5)	save the disk opened flag

	IFD SPARESTACK
;free up the temp stack we used and switch back to the old one (end of bug fix)
		move.l	(sp)+,d0		get old stack pointer
		lea.l	-2048(sp),a1		beginning of temp stack
		move.l	d0,sp			switch back to old stack
		move.l	#SPARESTACK,d0
		jsr	_LVOFreeMem(a6)		free temp stack
		tst.l	DiskOpened(a5)		did everything work ?
	ENDC

		bne	InitFail		nope, clean up and exit

		movea.l	DiskDev(a5),a0		copy the structure
		move.l	ThisDevProc(a5),MN_REPLYPORT(a0)  reply to us
		movea.l	DiskExtraDev(a5),a1
		moveq.l	#IOSTD_SIZE,d0
		jsr	_LVOCopyMem(a6)
		movea.l	DiskExtraDev(a5),a0	replies to a different port
		move.l	DoIOPort(a5),MN_REPLYPORT(a0)

; set up the global flag to determine if we are using trackdisk.device so we
; will know if it's safe to use the TD_GETGEOMETRY command.  Many other device
; drivers crash or give bogus information when they get this command.
		lea.l	TDName(pc),a0
		movea.l	DeviceName(a5),a1
		adda.l	a1,a1
		adda.l	a1,a1
		addq.l	#1,a1			can't do BSTR compare
		moveq.l	#15,d0			comparing 16 bytes
15$		cmp.b	(a0)+,(a1)+
		bne.s	NotTD
		dbra	d0,15$
		moveq.l	#TRUE,d0		TRUE==trackdisk
		move.l	d0,IsTrackDisk(a5)
	printf <'IsTrackDisk = %ld\n'>,d0

NotTD:
; special case test for carddisk.device which also understands TD_GETGEOMETRY

              lea.l   CCName(pc),a0
              movea.l DeviceName(a5),a1
              adda.l  a1,a1
              adda.l  a1,a1
              addq.l  #1,a1
              moveq   #14,d0                  ;arg - need equates
isCCDev:      cmp.b   (a0)+,(a1)+
              bne.s   NotCCD
              dbra.s  d0,isCCDev
              moveq   #TRUE,d0
              move.l  d0,IsTrackDisk(a5)
      printf <'IsCardDisk = %ld\n'>,d0

; need a timer for the disk flushing and record locking code
NotCCD:		lea.l	TimerName(pc),a0	open timer device
		moveq.l	#1,d0			unit 1
		moveq.l	#0,d1			no special flags
		movea.l	TimerDev(a5),a1		this IORequest
		jsr	_LVOOpenDevice(a6)
		move.l	d0,TimerOpened(a5)	did we get the timer ?
		bne	InitFail		nope, error out

		movea.l	TimerDev(a5),a0		copy to the spare TimerReq
		move.l	ThisDevProc(a5),MN_REPLYPORT(a0)
		movea.l	TimerExtraDev(a5),a1
		moveq.l	#IOTV_SIZE,d0
		jsr	_LVOCopyMem(a6)
		movea.l	TimerExtraDev(a5),a0	replies to a different port
		move.l	DoIOPort(a5),MN_REPLYPORT(a0)

		lea.l	InputName(pc),a0	open input device
		moveq.l	#0,d0
		moveq.l	#0,d1
		movea.l	InputDev(a5),a1
		jsr	_LVOOpenDevice(a6)
		move.l	d0,InputOpened(a5)
		bne	InitFail

; Initialise the spare message port we are using for returning DoIO() requests
		moveq.l	#-1,d0
		jsr	_LVOAllocSignal(a6)	get sigbit for spare port
		movea.l	DoIOPort(a5),a0
		move.b	d0,MP_SIGBIT(a0)
		move.b	#NT_MSGPORT,LN_TYPE(a0)
		move.l	ThisTask(a5),MP_SIGTASK(a0)
		lea.l	MP_MSGLIST(a0),a0
		NEWLIST	a0

		move.w	#TRUE,WrProt(a5)	until we know for sure anyway

		lea.l	DosLibName(pc),a1	open dos library
		moveq.l	#0,d0			any version will do
		jsr	_LVOOpenLibrary(a6)
		move.l	d0,DosLib(a5)
		beq.s	InitFail		didn't get it

;initialise root co-routine and startup the disk and kill co-routines
		bsr	InitCo			initialise co-routines
		tst.l	d0
		beq.s	InitFail		didn't get the memory

		lea.l	Disk(pc),a0		create disk co-routine
		bsr	CreateCo
		move.l	d0,DiskCo(a5)
		beq.s	InitFail		sorry, it didn't work

		lea.l	Kill(pc),a0		create kill co-routine
		bsr	CreateCo
		move.l	d0,KillCo(a5)
		beq.s	InitFail		didn't work

20$		movea.l	KillCo(a5),a0		now call the co-routines
		moveq.l	#0,d0			no args to start
		bsr	CallCo			it will WaitCo back to us
		movea.l	DiskCo(a5),a0		start main disk co-routine too
		moveq.l	#0,d0			no args either
		bsr	CallCo			and no return code

		bsr	Restart			initialise the disk

; Init worked all the way through so put our MsgPort address into dl_Task to
; make us a public filehandler that stays resident in memory.  Restart will
; still be running at this point (it may in fact have called the validator
; if the bitmap was bad).  Until MemValid is true the disk cannot be written.
; Apparently, devlist pointer can be null on bootup, added a test for this.
; DiskType is returned in d0 by Restart, use this as returncode for Init too.
		cmpa.l	#0,a4
		beq.s	InitExit
		move.l	ThisDevProc(a5),dl_Task(a4)	make us public
		bra.s	InitExit		return disktype in d0

InitFail	printf <'Init failed\n'>
		bsr.s	CleanUp			free what we allocated
		moveq.l	#0,d0			return false

InitExit	movem.l	(sp)+,a2-a4
		rts

;==============================================================================
; CleanUp()
;
; Cleans up any allocated memory and closes devs if Init failed for any reason.
;==============================================================================
CleanUp		move.l	a2,-(sp)
		tst.l	DiskCo(a5)		did we create disk co-routine
		beq.s	KillDevs		nope, so no kill either
		movea.l	DiskCo(a5),a0
		bsr	DeleteCo
		tst.l	KillCo(a5)		did we create kill co-routine
		beq.s	KillDevs		nope
		movea.l	KillCo(a5),a0
		bsr	DeleteCo

KillDevs	tst.l	DiskOpened(a5)		did we open the disk
		bne.s	KillMem			nope, so no timer either
		movea.l	DiskDev(a5),a1
		jsr	_LVOCloseDevice(a6)
		tst.l	TimerOpened(a5)		did we open the timer ?
		bne.s	KillMem			nope
		movea.l	TimerDev(a5),a1
		jsr	_LVOCloseDevice(a6)
		tst.l	InputOpened(a5)		did we open input
		bne.s	KillMem			nope
		movea.l	InputDev(a5),a1
		jsr	_LVOCloseDevice(a6)

KillMem		lea.l	MemRequired(pc),a2	point to memory table
10$		moveq.l	#0,d0
		move.w	(a2)+,d0		get size to be freed
		beq.s	20$			last entry, we did them all
		move.w	(a2)+,d1		find where pointer is stored
		move.l	0(a5,d1.w),d1		get mem pointer
		beq.s	20$			no pointer, failed here
		movea.l	d1,a1			free this chunk
		jsr	_LVOFreeMem(a6)
		bra.s	10$

20$		movea.l	FreeBufferQueue(a5),a2	free any cache buffers
30$		cmpa.w	#0,a2
		beq.s	40$			all done
		movea.l	a2,a1
		movea.l	(a2),a2			a2 points to next buffer
		moveq.l	#cb_SIZEOF,d0		get buff header size
		add.l	BlockSize(a5),d0	add data area size
		jsr	_LVOFreeMem(a6)		and free it
		bra.s	30$

40$		tst.l	DosLib(a5)		did we get DOS library
		beq.s	50$
		movea.l	DosLib(a5),a1
		jsr	_LVOCloseLibrary(a6)

50$		tst.l	BitmapKeys(a5)		did we get bitmap memory ?
		beq.s	CleanedUp		no
		movea.l	BitmapKeys(a5),a1	yes, so free it
		move.w	BitmapCount(a5),d0	calculate size to free
		ext.l	d0
		lsl.l	#2,d0			4 bytes per bitmap entry
		jsr	_LVOFreeMem(a6)
CleanedUp	move.l	(sp)+,a2
		rts

;==============================================================================
; Success = GetBuffers(Number),_AbsExecBase
;   d0			d0	    a6
;
; Allocates memory for cache buffers and links them into the free buffer queue.
;==============================================================================
GetBuffers	movem.l	d2-d3/a2,-(sp)
		lea.l	FreeBufferQueue(a5),a2	point to first link field
		move.w	d0,d2			save buffer count
		clr.l	d3
		bra.s	15$

10$		move.l	BufferMemType(a5),d1	observe memory type
		ori.l	#MEMF_CLEAR,d1		let's clear em out first time
		moveq.l	#cb_SIZEOF,d0		allow for special info
		add.l	BlockSize(a5),d0	and size of a block
		jsr	_LVOAllocMem(a6)	get the memory
		tst.l	d0			did it work
		beq.s	20$			nope, error out (d0 = 0)
		movea.l	d0,a0
		move.l	(a2),(a0)		singly linked list
		move.l	a0,(a2)			link it in
		addq.l	#1,d3
15$		dbra	d2,10$			and go for another
20$		move.l	d3,d0			return buffer count
		movem.l	(sp)+,d2-d3/a2
		rts

;==============================================================================
; success = GeometryInit(envec)
;   d0		 	  a3
;
; This routine was broken out of the main Init routine so that it can be
; called by Restart if the disk geometry changes on removeable media.  This
; will be called iff LowCyl for the partition is 0.  We assume the partition
; spans the whole surface of the media in this case.
;
; set up global information about the arrangement of this disk. (It would
; probably work out a little better to just clone the environment vector
; but I guess I'd better stick with the old format that the BCPL had).
;==============================================================================
GeometryInit	move.l	d2,-(sp)
		moveq	#0,d2			default sector/blk shift
		move.l	SectorSize(a3),d0
		move.l	SectorsPerBlock(a3),d1	was required to be 1...
		beq.s	2$			avoid stupid people
	printf <'init: sectors/block %ld'>,d1
		mulu.w	d1,d0			d0 and d1 were max <32k...
2$		move.l	d0,d1			save size in longwords
		lsl.l	#2,d0
		move.l	d0,BlockSize(a5)
		subi.l	#56,d1			compute hash table size
		move.l	d1,HTSize(a5)		that's #entries in table
		lsl.l	#2,d1			now calculate maximum byte...
		addi.w	#fhb_HashTable,d1	...offset to end of table
		subq.w	#4,d1			but point at last entry
		move.w	d1,MaxEntry(a5)		not past the end

		moveq.l	#-fdb_Data,d1
		add.l	BlockSize(a5),d1
		move.l	d1,BytesPerData(a5)	OFS format, data bytes per block
		
		moveq.l	#-1,d1			compute block shift
5$		addq.w	#1,d1			shift one more
		lsr.w	#1,d0			wait for the bit to drop out
		bcc.s	5$
		move.w	d1,BlockShift(a5)

		move.l	Surfaces(a3),NSurfaces(a5)
		move.l	BlocksPerTrack(a3),NSecsPerTrack(a5)
		move.l	ReservedBlocks(a3),Reserved(a5)
		move.l	Interleave(a3),Intleave(a5)
		move.l	LowerCylinder(a3),LowerCyl(a5)
		move.l	UpperCylinder(a3),UpperCyl(a5)
		move.l	Buffers(a3),d0
		moveq	#MINBUFFERS,d1		our minimum
		cmp.l	d0,d1
		bls.s	10$			buffers>=MINBUFFERS (unsigned)
		move.l	d1,d0			use MINBUFFERS
10$		move.l	d0,NumBuffers(a5)

; The mount command now ensures that there is a default memory type set up.
		moveq.l	#MEMF_PUBLIC,d0		assume buffmemtype is public
		moveq.l	#12,d1			size is # longwords
		cmp.l	EnvecSize(a3),d1	see if there is a memtype entry
		bgt.s	11$			nope, struct not big enough
		move.l	BuffMemType(a3),d0	yep, get the type
11$		move.l	d0,BufferMemType(a5)

; new things added for various nasties with CSA board and devs that can't
; handle transfers of more than 255 blocks in one go.  THIS IS A KLUGE!!!
		move.l	#$7fffffff,GMaxTransfer(a5)	default values
		move.l	#$ff000001,GAddressMask(a5)	default addr mask
		moveq.l	#14,d1				make sure envec...
		cmp.l	EnvecSize(a3),d1		...is big enough
		bgt.s	12$

		move.l	MaxTransfer(a3),GMaxTransfer(a5)
		move.l	AddressMask(a3),d0
		not.l	d0			going to and with address
		move.l	d0,GAddressMask(a5)

12$		move.l	NSurfaces(a5),d1
		mulu.w	NSecsPerTrack+2(a5),d1	get BlocksPerCyl
		move.l	d1,NSecsPerCyl(a5)	Actually SectorsPerCyl!!!!!

		move.l	UpperCyl(a5),d0		get highest block number
		sub.l	LowerCyl(a5),d0
		addq.l	#1,d0
		mulu.w	d1,d0			(upper-lower+1)*secspercyl

	printf <'init: numsecs %ld'>,d0
		; d0 = number of sectors
		; figure out shift for sectors/block
		move.l	SectorsPerBlock(a3),d1	1-64
		beq.s	30$			in case someone was stupid...
		moveq	#-1,d2			shift
20$		addq.l	#1,d2			ends up 0-6
		lsr.l	#1,d1			is the low bit 1?
		bcc.s	20$			continue until we see a 1 bit
	printf <'init: sector/block shift %ld'>,d2

		; convert number of sectors to number of blocks
		lsr.l	d2,d0			d0 >>= d2 (log2 sectors/block)
30$
		; d0 = number of blocks
	printf <'init: numblocks %ld'>,d0
		subq.l	#1,d0			highest block number

		move.l	d0,HighestBlock(a5)	high blk (partition relative)

		sub.l	Reserved(a5),d0		calculate total blocks on disk
		addq.l	#1,d0
		move.l	d0,NBlocks(a5)

; figure out the maximum practical limit for write requests (size of disk)
		move.w	BlockShift(a5),d1
		lsl.l	d1,d0
		move.l	d0,LargestWrite(a5)
		bpl.s	LargeOK
		move.l	#$7fffffff,LargestWrite(a5)

; calculate how many data blocks can be referenced by a single bitmap block
LargeOK		move.l	BlockSize(a5),d0	calculate BlocksPerBM
		subq.l	#4,d0			don't include checksum
		lsl.l	#3,d0			8 blocks per byte
		move.w	d0,BlocksPerBM(a5)	save this (only need a word)

; Calculate how many bitmap blocks are required for a disk of the given size.
		subq.l	#1,d0			BlocksPerBM-1
		add.l	HighestBlock(a5),d0	plus HighestBlock
		sub.l	Reserved(a5),d0		minus reserved blocks
		divu.w	BlocksPerBM(a5),d0	divided by BlocksPerBM
		move.w	d0,BitmapCount(a5)	gives number of bitmap blocks
		ext.l	d0			don't need remainder
		lsl.l	#2,d0			allocating array of longwords
		bsr	GetPubMem		get array for bitmap
		move.l	d0,BitmapKeys(a5)	store in global area
		beq.s	GInitFail		didn't get memory

; calculate where the root block goes (always in the middle of the partition)
		move.l	Reserved(a5),d0
		add.l	HighestBlock(a5),d0
		lsr.l	#1,d0
		move.l	d0,RootKey(a5)		where root block is
	printf <'init: RootKey = %ld\n'>,d0

		move.l	LowerCyl(a5),d0
		move.l	NSecsPerCyl(a5),d1
		mulu.w	d0,d1			low sector
		move.w	BlockShift(a5),d0	= sectorshift + sector/blk shift
		sub.w	d2,d0			leaves SectorShift in d0!
		lsl.l	d0,d1			turn lowsector into bytes
		move.l	d1,LowerByte(a5)	now its actually LowerByte!
	printf <'init: LowerByte = $%lx'>,d1
		
		move.l	NumBuffers(a5),d0	get the buffers we need
		addq.l	#1,d0			add one for the bitmap
		bsr	GetBuffers		and link into FreeBufferQueue
		tst.l	d0			did it work
		beq.s	GInitFail		nope, clean up and exit

		movea.l	FreeBufferQueue(a5),a0
		move.l	(a0),FreeBufferQueue(a5)
		clr.l	(a0)
		move.l	a0,BMBlock(a5)		spare cache for bitmaps
		moveq.l	#TRUE,d0
GInitFail	move.l	(sp)+,d2
		rts


;==============================================================================
; Table of memory requirements for this handler.  Each table entry contains:-
; Size required,  Offset from a5 for storing mem pointer.
;==============================================================================
MemRequired	DC.W	IOSTD_SIZE,DiskDev
		DC.W	IOSTD_SIZE,DiskExtraDev
		DC.W	IOTV_SIZE,TimerDev
		DC.W	IOTV_SIZE,TimerExtraDev
		DC.W	MP_SIZE,DoIOPort
		DC.W	IOSTD_SIZE,InputDev
		DC.W	dp_Res1,TimerPacket		linked into TimerDev
		DC.W	dp_Res1,InputPacket		linked into InputDev
		DC.W	ie_SIZEOF,InpEvent		propogated disk events
		DC.W	IS_SIZE,ChangeInt		ChangeInt handler
		DC.W	128*4,CacheHash			buffer hash table
		DC.W	128,ReqString			built requester string
		DC.W	64,ButtonString			built retry|cancel string
		DC.W	64,ReqArgs			argument list for requester
		DC.W	0

TimerName	DC.B	'timer.device',0
InputName	DC.B	'input.device',0
DosLibName	DC.B	'dos.library',0
TDName		DC.B	'trackdisk.device',0
CCName		DC.B	'carddisk.device',0

		END
