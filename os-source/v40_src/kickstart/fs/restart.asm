		SECTION	filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/memory.i"
		INCLUDE	"exec/io.i"
		INCLUDE	"devices/timer.i"
		INCLUDE	"devices/trackdisk.i"
		INCLUDE	"libraries/dosextens.i"
		INCLUDE	"globals.i"
		INCLUDE	"private.i"
		INCLUDE	"moredos.i"
		INCLUDE	"notify.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XDEF	Restart,ForgetVolume,DirectRead,DirectDiscard
		XDEF	DoDirectWrite,DoDirectRead

		XREF	_LVOFreeMem,_LVODoIO,_LVOForbid,_LVOPermit
		XREF	CompString,GetPubMem,Validator,GetPubMem
		XREF	TestDisk,InitBitmap,CallCo,CreateCo,_LVOAllocMem
		XREF	GeometryInit

;==============================================================================
; success = Restart()
;   d0
;
; The main restart routine.  Called after init or whenever a new disk is put
; into the drive.  Checks the disk type and the root block to see what kind of
; disk we have.  If it really is a DOS disk then the bitmap is checked to see
; if this disk needs validating.  If validation is required then the Validate
; co-routine is started and the bitmap will be fixed up (we hope).  Restart
; also adds this volume to the list of known volumes unless it is already there
; Does not run as a co-routine so we can be sure disk is valid when we return.
;==============================================================================
Restart		printf <'Restart...'>
		movem.l	d2/a2-a3,-(sp)
		clr.w	TotallyFull(a5)		assume bitmap fits
		move.l	#-1,DiskType(a5)	ID_NO_DISK_PRESENT
		moveq.l	#TD_CHANGESTATE,d0
; FIX!!!!!! need to test if the command was supported!!!!
; If not supported, try to read block 0, if it fails then no disk in drive
		bsr	TestDisk		see if disk in or out
		tst.l	d0
		bne	Cleanup			no disk in drive

; New addition to support double density (aka half-fast) Amiga floppies.  If
; our partition starts at 0,0,0 (therefore it MUST be a floppy, or mounted FS
; across the whole disk surface) we use TD_GETGEOMETRY to see if the block size
; is still the same.  If the blocksize has changed, then we de-allocate all
; buffers and re-allocate using the new parameters.  We also update our own
; fssm_Startup so that some tools (format, disked etc) know about the changes.
; It is assumed that the partition is mounted across the whole disk surface!
; There is a bit of redundancy here since I need to update both the Envec AND
; my internal copies of the envec.  Should probably have used the envec directly
; but there's just too much code to change this now.  Maybe for 2.5.
;
; Another note, the geometry can change too, treat this like a blocksize change
		tst.l	IsTrackDisk(a5)
		beq	NoGeometryChange	only trackdisk supports GETGEOMETRY
		tst.l	LowerCyl(a5)		do we start at 0,0,0?
		bne	NoGeometryChange	nope, don`t bother

	printf <'Checking drive geometry\n'>
		movea.l	DiskExtraDev(a5),a1
		move.w	#TD_GETGEOMETRY,IO_COMMAND(a1)
		moveq.l	#dg_SIZEOF,d0
		move.l	d0,IO_LENGTH(a1)
		suba.l	d0,sp			geometry on stack
		clr.l	dg_SectorSize(sp)	this is our error check
		move.l	sp,IO_DATA(a1)
		jsr	_LVODoIO(a6)		get the geometry
		movea.l	DiskExtraDev(a5),a1
		tst.b	IO_ERROR(a1)		error, probably not supported
		beq.s	NoGErr
ScrapGeometry	lea.l	dg_SIZEOF(sp),sp
		bra	NoGeometryChange

; TD_GETGEOMETRY is supported, check if the geometry is different from current
NoGErr:
	movea.l	sp,a0
	printf <'dg_SectorSize   = %ld\n'>,dg_SectorSize(a0)
	printf <'dg_TotalSectors = %ld\n'>,dg_TotalSectors(a0)
	printf <'dg_Cylinders    = %ld\n'>,dg_Cylinders(a0)
	printf <'dg_CylSectors   = %ld\n'>,dg_CylSectors(a0)
	printf <'dg_Heads        = %ld\n'>,dg_Heads(a0)
	printf <'dg_TrackSectors = %ld\n\n'>,dg_TrackSectors(a0)
	
		movea.l	CurrentEnv(a5),a3	needed in a3 for GeometryInit

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
	
		tst.l	dg_SectorSize(sp)
		beq	ScrapGeometry		device didn't really support it
		move.l	SectorSize(a3),d0	get envec sector size (lwords)
		lsl.l	#2,d0			convert to byte size
		cmp.l	dg_SectorSize(sp),d0	see if the same
		bne.s	GeometryChange		nope, must change it
		move.l	UpperCylinder(a3),d0	compute #cyls in partition
		addq.l	#1,d0			lowercyl MUST be 0 here
		cmp.l	dg_Cylinders(sp),d0
		bne.s	GeometryChange		not same, must change it
		move.l	Surfaces(a3),d0
		cmp.l	dg_Heads(sp),d0
		bne.s	GeometryChange
		move.l	BlocksPerTrack(a3),d0
		cmp.l	dg_TrackSectors(sp),d0
		beq	ScrapGeometry		everything the same, exit

; Something about the geometry has changed.  To make things simple we assume
; blocksize has changed and de-allocate all the buffers and re-allocate with
; the new (maybe unchanged) size.  All globals are then re-calculated to
; reflect the new geometry.  The envec is also updated so tools that snoop
; it will know about the changes.  All cache buffers (with the exception of
; the bitmap buffer) will be on the FreeBufferQueue because they were put
; there by the main loop when the disk was removed.
GeometryChange	movea.l	FreeBufferQueue(a5),a1
		move.l	a1,d0
		beq.s	10$			go kill bitmap buffer
		move.l	(a1),FreeBufferQueue(a5) unlink from list
		moveq.l	#cb_SIZEOF,d0
		add.l	BlockSize(a5),d0
		jsr	_LVOFreeMem(a6)
		bra.s	GeometryChange		go until all freed

10$		movea.l	BMBlock(a5),a1		free bitmap block
		moveq.l	#cb_SIZEOF,d0
		add.l	BlockSize(a5),d0
		jsr	_LVOFreeMem(a6)

; free the memory used for the array of bitmap keys for the disk
		move.w	BitmapCount(a5),d0
		ext.l	d0
		lsl.l	#2,d0
		movea.l	BitmapKeys(a5),a1
		jsr	_LVOFreeMem(a6)

; Update the environment vector for tools and the GeometryInit routine
	printf <'Geometry changed\n'>
		movea.l	CurrentEnv(a5),a3	envec to a3 for GeometryInit
		move.l	NumBuffers(a5),Buffers(a3)  set correct # of buffers
		move.l	dg_SectorSize(sp),d0	update blocksize
		lsr.l	#2,d0
		move.l	d0,SectorSize(a3)
		move.l	dg_Cylinders(sp),d0
		subq.l	#1,d0
		move.l	d0,UpperCylinder(a3)
		move.l	dg_Heads(sp),Surfaces(a3)
; steve used to set sectorsperblock == heads!!!
;;;;;;;;;;	move.l	dg_Heads,SectorsPerBlock(a3)
		move.l	dg_TrackSectors(sp),BlocksPerTrack(a3)
		bsr	GeometryInit		gets cache buffers etc.
		bra	ScrapGeometry		clear stack, all done

; Everything set up as if init just ran, continue with rest of Restart
NoGeometryChange:
		move.l	#type.unreadable,DiskType(a5)	assume bad disk
		clr.w	BitmapValid(a5)		bitmap is invalid
		clr.l	StartSearch(a5)		different disk
		moveq.l	#TD_PROTSTATUS,d0
		bsr	TestDisk		get protect status of disk
		move.w	d0,WrProt(a5)
		bne.s	10$			can't override real write prot
		move.w	UserWrProt(a5),WrProt(a5)	may override

; read the very first block of the disk to determine the DOS type (disk has
; been modified to allow cache buffer reads in the reserved area, though it
; should be discarded and not cached when we finish with it here).  Disktype
; is set to ID_NO_DISK so the Disk co-routine won't checksum this block.
10$		moveq.l	#0,d0			read the first block
		bsr	DirectRead

		tst.l	d0			did we get the buffer?
		beq	Cleanup			nope, exit now
		movea.l	d0,a0			fetch the cache buffer
		tst.l	cb_Error(a0)		did we read it ?
		beq.s	20$			yes, no problems
		bsr	DirectDiscard		nope, trash the cache buffer
		bra	Cleanup			and exit now

; we read the first block OK, set up DiskType from the first four bytes and
; if its type.dos then read in and check the root block.
20$		move.l	cb_SIZEOF(a0),-(sp)	save the disk type
		bsr	DirectDiscard		free up this cache buffer
		move.l	(sp)+,d0		restore the type
	printf <'new disktype 0x%lx'>,d0
		move.l	d0,DiskType(a5)		and save it
		beq	NotADosDisk		can't allow disktypes of 0

		cmpi.l	#type.dos,d0		we support DOS/0 and DOS/1
		beq.s	30$
		cmpi.l	#type.dos!1,d0
		beq.s	30$			and DOS/2 or DOS/3 now
		cmpi.l	#type.dos!2,d0
		beq.s	30$
		cmpi.l	#type.dos!3,d0
		beq.s	30$
		cmpi.l	#type.dos!4,d0		and dos/4 and dos/5 now
		beq.s	30$
		cmpi.l	#type.dos!5,d0
		bne	Cleanup			not a DOS disk, skip root check

; This disk purports to be a DOS disk, make sure it's not lying to us.
30$		move.l	RootKey(a5),d0		get the root block key
		bsr	DirectRead
		tst.l	d0			did we get the buffer
		beq.s	NotADosDisk		nope, say not DOS
		movea.l	d0,a0
		tst.l	cb_Error(a0)		did it read OK
		bne.s	RootCorrupt		nope, treat as not a DOS disk
		lea.l	cb_SIZEOF(a0),a1	point at data area
		move.l	fhb_OwnKey(a1),d0	these should all be 0
		or.l	fhb_HighSeq(a1),d0
		or.l	fhb_FirstBlock(a1),d0
		bne.s	RootCorrupt		nope, something's wrong
		move.l	fhb_DataSize(a1),d0	hash table size
		cmp.l	HTSize(a5),d0		contains #longword entries
		bne.s	RootCorrupt
		adda.l	BlockSize(a5),a1	point to end of block
		tst.l	vfhb_Parent(a1)		parent should be NULL too
		bne.s	RootCorrupt		not really a dos disk
		moveq.l	#st.root,d0		is it really the root block
		cmp.l	vfhb_SecType(a1),d0
		bne.s	RootCorrupt		nope
		move.b	vfhb_FileName(a1),d0	0 < disk name length <= 30
		ble.s	RootCorrupt
		cmpi.b	#30,d0
		ble.s	GotADosDisk		all checked out

; the first block was OK but the root didn't stand up to the test, type is NDOS
RootCorrupt	bsr	DirectDiscard		finished with buffer
NotADosDisk	move.l	#type.ndos,DiskType(a5)
		bra	Cleanup			free buffer memory and return

; Everything checked out OK so add this volume to the list of known volumes.
GotADosDisk	movea.l	a0,a3			stash buffer containing root
		jsr	_LVOForbid(a6)		critical stuff
		bsr	GetDevList
		movea.l	d0,a0
		movea.l	(a0),a2			first volume node

; first see if our volnode has already been created (got an ACTION_UNINHIBIT)
10$		cmpa.w	#0,a2			end of list ?
		beq.s	AddVolume		yep, must add a volume
		adda.l	a2,a2			make into an APTR
		adda.l	a2,a2
		cmpi.w	#DLT_VOLUME,dl_Type+2(a2)	is this a volume
		bne.s	20$			no, go on to the next
		lea.l	dl_VolumeDate(a2),a0
		lea.l	cb_SIZEOF(a3),a1	point at rootblock date area
		adda.l	BlockSize(a5),a1
		lea.l	vrb_CreateDays(a1),a1
		moveq.l	#2,d0
11$		cmp.l	(a0)+,(a1)+		check 3 lwords of date
		bne.s	20$			not the same
		dbra	d0,11$
		movea.l	dl_Name(a2),a0		date checked out so...
		adda.l	a0,a0			...compare the names
		adda.l	a0,a0
		lea.l	cb_SIZEOF(a3),a1	point at rootblock name area
		adda.l	BlockSize(a5),a1
		lea.l	vrb_Name(a1),a1
		bsr	CompString
		tst.l	d0			was name the same ?
		beq	GotVolume		yep, volume known about (in a2)
20$		movea.l	(a2),a2			link to the next
		bra.s	10$			and keep looking

; we didn't find the volume we were looking for so we must create a volnode.
; FIX!!!!! Use MakeDosEntry!!!!! (under 2.0x)
AddVolume	lea.l	cb_SIZEOF(a3),a0	point at rootblock name area
		adda.l	BlockSize(a5),a0
		lea.l	vrb_Name(a0),a0		a0 holds name pointer
		bsr	AddDevice		add this device in
		tst.l	d0			did we get a device
		bne.s	10$			yep, no problems

		jsr	_LVOPermit(a6)		was in Forbid() state
		movea.l	a3,a0			free up the root block
		bsr	DirectDiscard
		clr.l	DiskType(a5)		mark as not valid
		bra	Cleanup			all done

; we got our device node OK, fill in all the pertinent info we will need.
10$		movea.l	d0,a2			need in a2 for later
		clr.l	dl_Lock(a2)
		clr.l	dl_LockList(a2)
		move.w	#DLT_VOLUME,dl_Type+2(a2)
		lea.l	cb_SIZEOF(a3),a1	point at rootblock date area
		adda.l	BlockSize(a5),a1
		movem.l	vrb_CreateDays(a1),d0-d2
		movem.l	d0-d2,dl_VolumeDate(a2)

; If DOS date (in the DOS rootnode) is unset, then use values from this disk
		movea.l	DosLib(a5),a0
		movea.l	dl_Root(a0),a0
		tst.l	rn_Time(a0)		this will be 0 if unset
		bne.s	GotVolume
		tst.l	rn_Time+4(a0)
		bne.s	GotVolume
		tst.l	rn_Time+8(a0)
		bne.s	GotVolume

; convert days,mins,ticks to seconds/micros format and then SetSysTime
		movem.l	d0-d2,rn_Time(a0)	save DOS's idea of time
		mulu.w	#43200,d0		half# seconds in a day
		lsl.l	#1,d0			so double it
		mulu.w	#60,d1
		add.l	d1,d0			d0 = seconds
		mulu.w	#20000,d2		d2 = micros
		movea.l	TimerExtraDev(a5),a1
		move.w	#TR_SETSYSTIME,IO_COMMAND(a1)
		move.b	#IOF_QUICK,IO_FLAGS(a1)
		move.l	d0,IOTV_TIME+TV_SECS(a1)	set seconds
		move.l	d2,IOTV_TIME+TV_MICRO(a1)	set micros
		jsr	_LVODoIO(a6)

; whether we found the volume or just created a new one, we have to do this.
GotVolume	move.l	ThisDevProc(a5),dl_Task(a2)	mark as now mounted
		move.l	a2,d0			set global current volume
		lsr.l	#2,d0			as a BPTR
		move.l	d0,CurrentVolume(a5)

		lea.l	LockQueue(a5),a0
10$		tst.l	(a0)
		beq.s	20$			add to end of list
		movea.l	(a0),a0
		adda.l	a0,a0
		adda.l	a0,a0
		bra.s	10$

20$		move.l	dl_LockList(a2),(a0)	re-activate locks
		clr.l	dl_LockList(a2)

;bug fix.  Make sure fl_Task is pointing to our DeviceProc because the disk
;may have been put into a different drive with a different fs process.
		lea.l	LockQueue(a5),a0	point to locklist
21$		move.l	(a0),d0			fetch next lock
		beq.s	22$			end of list
		lsl.l	#2,d0			BPTR to APTR
		movea.l	d0,a0
		move.l	ThisDevProc(a5),fl_Task(a0)
		bra.s	21$

22$		jsr	_LVOPermit(a6)		safe now
		movea.l	a3,a0			finished with root block
		bsr	DirectDiscard

; if there are any notifies on the volume, we have to make sure that they are
; going to be referred to this Filehandler task for action.  We do this by
; making sure that nr_Handler contains our DeviceProc address.
		movea.l	dl_unused(a2),a0
25$		move.l	a0,d0
		beq.s	30$
		movea.l	ne_NotifyReq(a0),a1
		move.l	ThisDevProc(a5),nr_Handler(a1)
		movea.l	(a0),a0
		bra.s	25$

; Check if the bitmap on disk is valid and read it to calculate the number of
; free blocks on the disk.  If bitmap is not valid then the validator will be
; called to fix things up and write a new bitmap out (if there's room for it)
;
; Note that the validator will WaitCo long before it's done, so the return
; to here means nothing (when validator calls GrabBlock/WaitBlock, if the block
; must be read from disk, it will call either ResumeCo on Disk, or WaitCo if
; the disk is running).
30$		bsr	InitBitmap		initialise the bitmap
		tst.l	d0			did it initialise OK ?
		beq.s	Cleanup			yes
		lea.l	Validator(pc),a0
		bsr	CreateCo		set up the restart co-routine
		movea.l	d0,a0			we're going to call it now
		move.l	a0,d0
		beq.s	Cleanup			no memory, can't start
		bsr	CallCo			call it (it checks disk etc.)
		bra.s	Cleanup2

; even if the disk was unreadable or not DOS, this is still a non-error return
; There are actually no return codes from restart anymore because it runs as
; an independent co-routine.  DiskType is probably the best indicator of whats
; happened when restart completes.
Cleanup		moveq.l	#TD_MOTOR,d0
		moveq.l	#0,d1			turn motor off
		bsr	TestDisk

Cleanup2	move.l	DiskType(a5),d0		return disktype
		movem.l	(sp)+,d2/a2-a3
		rts

;==============================================================================
; buffer = DirectRead( diskblock )
;   d0			  d0
;
; support routine for Restart, reads in the required disk sector to a fake 
; cache buffer (allocated on the fly).  Use DirectLoose to free that buffer.
;==============================================================================
DirectRead	printf <'DirectRead %ld'>,d0
		move.l	d0,-(sp)		save the required disk key
		moveq.l	#cb_SIZEOF,d0		get a fake cache buffer
		add.l	BlockSize(a5),d0
		move.l	BufferMemType(a5),d1	this type of memory
		jsr	_LVOAllocMem(a6)
		movea.l	d0,a0			assume it works
		move.l	(sp)+,d0		restore key number
		move.l	a0,d1
		bne.s	DoDirectRead		got buffer OK
		moveq.l	#0,d0			failed allocation
		rts

;==============================================================================
; buffer = DoDirectRead( buffer,block )
;   a0,d0		   a0     d0
;
; support routine for Restart, reads in the required disk sector to a
; cache buffer.
; returns buffer in both a0 and d0.
;==============================================================================
DoDirectRead	printf <'DoDirectRead %ld, $%lx'>,d0,a0
		move.l	a0,-(sp)		save buffer address
		bsr.s	DirectSupport		set up everything but CMD
		move.w	#CMD_READ,IO_COMMAND(a1) it returned ior in a1
DirectDoIO	jsr	_LVODoIO(a6)		get this block
		move.l	(sp)+,d0		return buffer to d0
		movea.l	d0,a0			set up error code
		movea.l	DiskExtraDev(a5),a1
		move.b	IO_ERROR(a1),cb_Error+3(a0)
	printf <'DirectDoIO returning $%lx, err = %ld'>,d0,cb_Error(a0)
		rts

;==============================================================================
; ior = DirectSupport (buffer,block)
; a1		        a0    d0
; Helps out with read and write.  Sets up all but CMD, returns IOR in a1
;==============================================================================
DirectSupport	printf <'DirectSupport %ld, $%lx'>,d0,a0
		move.w	#1,MotorTicks(a5)
		move.l	d0,cb_Key(a0)
		clr.l	cb_Error(a0)		no errors occured yet
		clr.w	cb_State(a0)		doesn`t need writing
		movea.l	DiskExtraDev(a5),a1	use spare IORequest struct
		lea.l	cb_SIZEOF(a0),a0	pointer to data area
		move.l	a0,IO_DATA(a1)		where to read/write
		move.w	BlockShift(a5),d1
		lsl.l	d1,d0			make into a byte offset
		add.l	LowerByte(a5),d0	add offset in bytes for 1st blk
		move.l	d0,IO_OFFSET(a1)
		move.l	BlockSize(a5),IO_LENGTH(a1)  going to do 1 block
		rts

;==============================================================================
; error = DoDirectWrite( buffer, block )
;   d0			   a0	  d0
;
; Writes a (fake) buffer directly to disk.  Clears cb_State.
;==============================================================================
DoDirectWrite	printf <'DoDirectWrite %ld, $%lx'>,d0,a0
		move.l	a0,-(sp)		save buffer address
		bsr.s	DirectSupport		set up everything
		move.w	#CMD_WRITE,IO_COMMAND(a1)
		bra	DirectDoIO		handle same a DoDirectRead

;==============================================================================
; DirectDiscard( buffer )
;		   a0
;
; Frees up the fake cache buffers allocated by DirectRead
;==============================================================================
DirectDiscard	printf <'DirectDiscard $%lx'>,a0
		cmpa.w	#0,a0
		beq.s	10$			nothing to free
		moveq.l	#cb_SIZEOF,d0		get a fake cache buffer
		add.l	BlockSize(a5),d0
		movea.l	a0,a1
		jsr	_LVOFreeMem(a6)
10$		clr.w	MotorTicks(a5)		mark motor as off
		moveq.l	#TD_MOTOR,d0		** won't work for hard disk **
		moveq.l	#0,d1			turn motor off
		bra	TestDisk
		
;==============================================================================
; DevListPtr = GetDevList()
;    d0
; Returns a pointer to the Devlist pointer itself.   The devlist pointer will
; contain a BPTR to the first Devinfo structure or 0.
;==============================================================================
GetDevList	movea.l	DosLib(a5),a0		dos library pointer
		movea.l	dl_Root(a0),a0		rootnode pointer = APTR
		movea.l	rn_Info(a0),a0		DosInfo pointer = BPTR
		adda.l	a0,a0
		adda.l	a0,a0
		lea.l	di_DevInfo(a0),a0	point to first Devinfo ptr
		move.l	a0,d0
		rts

;==============================================================================
; VolNode = AddDevice( name )
;   d0			a0
;
; Creates a device node using the given name.  Returns address of the volnode
; as an absolute address.  The volnode is linked into the devlist using a BPTR.
; Must call this inside a Forbid() Permit() pair
;==============================================================================
AddDevice	movem.l	a2/a3,-(sp)
		movea.l	a0,a2			save the name part
		moveq.l	#DevList_SIZEOF,d0	get the devlist struct
		bsr	GetPubMem
		tst.l	d0
		beq.s	20$			didn't get it

		movea.l	d0,a3			save devlist pointer
		moveq.l	#0,d0			get memory for name string
		move.b	(a2),d0
		addq.l	#2,d0			allow for null termination
		bsr	GetPubMem
		tst.l	d0
		bne.s	10$			got it OK

		movea.l	a3,a1			free DevList struct
		moveq.l	#DevList_SIZEOF,d0
		jsr	_LVOFreeMem(a6)
		moveq.l	#FALSE,d0		error return
		bra.s	20$

10$		movea.l	d0,a0
		lsr.l	#2,d0
		move.l	d0,dl_Name(a3)		save string BPTR in DevList
		clr.w	d0
		move.b	(a2),d0			get the string length
15$		move.b	(a2)+,(a0)+		copy the name over
		dbra	d0,15$

		bsr	GetDevList		point to devlist ptr
		movea.l	d0,a2			save it
		move.l	(a2),(a3)
		move.l	a3,d0			create BPTR to our node
		lsr.l	#2,d0
		move.l	d0,(a2)			link into the devlist
		move.l	a3,d0			return address of struct
20$		movem.l	(sp)+,a2/a3
		rts

;==============================================================================
; ForgetVolume( volume )
;		  a0
;
; given a BPTR to a VolNode, unlinks it from the DevList and frees the memory.
;==============================================================================
ForgetVolume	move.l	a2,-(sp)
		movea.l	a0,a2			save DevList entry
		cmpa.w	#0,a2
		beq.s	VolumeForgot		no work to do

		jsr	_LVOForbid(a6)		stop race conditions
		bsr	GetDevList		point to the first entry
		movea.l	d0,a0
10$		cmp.l	(a0),a2			is this the same
		beq.s	20$			yep, found our volume
		movea.l	(a0),a0			link to next volume
		cmpa.w	#0,a0			don't run off end
		beq.s	VolNotFound
		adda.l	a0,a0			convert to APTR
		adda.l	a0,a0
		bra.s	10$

20$		adda.l	a2,a2			convert given pointer to APTR
		adda.l	a2,a2
		move.l	(a2),(a0)		unlink from list

		move.l	dl_Name(a2),a1		free up the name memory
		adda.l	a1,a1
		adda.l	a1,a1
		moveq.l	#0,d0
		move.b	(a1),d0
		addq.l	#2,d0			bug fix
		jsr	_LVOFreeMem(a6)

		movea.l	a2,a1			free DevList struct
		moveq.l	#DevList_SIZEOF,d0
		jsr	_LVOFreeMem(a6)
VolNotFound	jsr	_LVOPermit(a6)
VolumeForgot	move.l	(sp)+,a2
		rts

		END
