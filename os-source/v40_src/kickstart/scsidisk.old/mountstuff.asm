		SECTION	driver,CODE

		NOLIST
		INCLUDE	"modifiers.i"
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/libraries.i"
		INCLUDE	"libraries/configvars.i"
		INCLUDE	"libraries/expansionbase.i"
		INCLUDE	"libraries/dosextens.i"

		INCLUDE	"device.i"
		INCLUDE	"board.i"
		INCLUDE	"hardblocks.i"
		INCLUDE	"scsidirect.i"
		INCLUDE	"filesysres.i"
		INCLUDE	"filehandler.i"

		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XDEF	GetBlock,GetRDBlock,MountUnit,FindAndMount

		XREF	_LVOOpenLibrary,_LVOCloseLibrary
		XREF	_LVOForbid,_LVOPermit,_LVOAllocMem,_LVOFreeMem
		XREF	_LVOMakeDosNode,_LVOAddDosNode,_LVOEnqueue

		XREF	SendInternalCmd,FindUnit,MakeUnit,GetPubMem
		XREF	_LVOOpenResource,_LVOAddResource,_LVOCopyMem

		XREF	LoadSeg

		XREF	DosName

;==============================================================================
; Success = GetBlock( block, type, unit, buffer ),DeviceBase
;    d0			d0    d1    a0     a1		a5
;
; Fetches the given logical block from the given unit into the given memory.
; If type is non-zero then this is one of the RigidDiskBlock sectors and we'll
; check for the correct type and see if the checksum is OK. 0=failure
;==============================================================================
GetBlock	movem.l	d2-d3/a2-a3/a6,-(sp)
		move.l	d0,d2			stash block number
		move.l	d1,d3			and header type
		movea.l	a0,a2			and unit pointer
		movea.l	a1,a3			and buffer pointer
		movea.l	hd_SysLib(a5),a6	need exec

		lea.l	-IOSTD_SIZE(sp),sp	get an IORequest on stack
		movea.l	sp,a0

		move.l	a2,IO_UNIT(a0)		store the unit ptr
		move.l	hu_BlockSize(a2),IO_LENGTH(a0)
		move.w	hu_BlockShift(a2),d1
		lsl.l	d1,d2			convert block # to byte offset
		move.l	d2,IO_OFFSET(a0)
		move.l	a3,IO_DATA(a0)		we're reading to here
		move.w	#CMD_READ,IO_COMMAND(a0)
		bsr	SendInternalCmd		send the read request
		movea.l	d0,a1
		tst.b	IO_ERROR(a1)		did it work
		bne.s	ReadFailed		nope, bail out now

		moveq.l	#-1,d0			assume it worked
		tst.l	d3			should we run a checksum ?
		beq.s	ReadDone		nope, not a special block type
		cmp.l	(a3),d3			does the type match ?
		bne.s	ReadFailed		nope, that's a failure too

		move.l	4(a3),d0		get number of summed longs
		moveq.l	#0,d1
		bra.s	20$
10$		add.l	(a3)+,d1
20$		dbra	d0,10$			run the checksum count
		moveq.l	#-1,d0			assume it worked
		tst.l	d1
		beq.s	ReadDone		it did, block is OK

ReadFailed	moveq.l	#0,d0			failed code
ReadDone	lea.l	IOSTD_SIZE(sp),sp	free up stack frame
		movem.l	(sp)+,d2-d3/a2-a3/a6
		rts

;==============================================================================
; RDBlock = GetRDBlock( unit )
;   d0			 a0
;
; Attempts to read the RigidDiskBlock from the unit and links it in the struct.
; Also reads and initialises any bad block list for this unit and links it into
; the BadBlockList at this time.  Since the RigidDiskBlock should be on a good
; sector, no mapping would need to be done for this anyway.  The BadBlockList
; should not be linked in until it is ready for use (by IOTask).
;==============================================================================
GetRDBlock	movem.l	d2/a2-a4/a6,-(sp)
		movea.l	a0,a2			stash unit structure
		move.l	hu_RDB(a2),d0		do we have RDB already ?
		bne	RDBExit			yes, so no work to do

		movea.l	hd_SysLib(a5),a6
		move.l	hu_BlockSize(a2),d0	get memory for RigidDiskBlock
		bsr	GetPubMem		(enough for one disk block)
		move.l	d0,hu_RDB(a2)
		beq	RDBExit			failed (will return 0)

; now we loop around for RDB_LOCATION_LIMIT blocks looking for a RigidDiskBlock
		moveq.l	#0,d2			initial offset to look at
NextRDB		movea.l	a2,a0			accessing this unit
		movea.l	hu_RDB(a2),a1		where the data should go
		move.l	d2,d0			and we want this block
		move.l	#IDNAME_RIGIDDISK,d1	if its one of these
		bsr	GetBlock		fetch the block
		tst.l	d0			did it work OK ?
		bne.s	GotRDBlock		yep

; the last block we fetched wasn't a RigidDiskBlock.  Try up to the maximum.
BumpRDB		addq.w	#1,d2			bump the block count
		cmpi.w	#RDB_LOCATION_LIMIT,d2	have we looked far enough ?
		bne.s	NextRDB			no, so keep looking

; we failed to find a rigid disk block, so free the data from the unit struct
		movea.l	hu_RDB(a2),a1		free the memory we used
		move.l	hu_BlockSize(a2),d0	it's this big
		jsr	_LVOFreeMem(a6)
		clr.l	hu_RDB(a2)		flag we haven't got one

GotRDBlock	move.l	hu_RDB(a2),d0		did we find the rigid disk block
		beq	RDBExit			nope, so no bad block info
		movea.l	d0,a0			yes, set up unit info

		move.w	rdb_Cylinders+2(a0),hu_TotalCyls(a2)
		move.b	rdb_Sectors+3(a0),hu_TotalBlocks(a2)
		move.b	rdb_Heads+3(a0),hu_TotalHeads(a2)
		bne.s	HeadsOK
		move.b	#2,hu_TotalHeads(a2)
HeadsOK:
	IFD DEBUG_CODE
	printf <'RigidDiskBlocks says:\n'>
	printf <'%ld cyls\n %ld secs\n'>,rdb_Cylinders(a0),rdb_Sectors(a0)
	move.l	rdb_Heads(a0),d0
	printf <'%ld heads\n'>,d0
	ENDC

; before we can do anything else to an XT drive, we must make sure that the
; drive parameters are set up correctly.  This involves sending a read capacity
; to force the XT code to set drive parameters.
	IFD XT_SUPPORTED
		tst.b	hu_Type(a2)		is this an XT drive ?
		bne	NoXTFix			nope, no need to fix this

		lea.l	-IOSTD_SIZE(sp),sp	get an IORequest on stack
		movea.l	sp,a0
		lea.l	-scsi_SIZEOF(sp),sp	get a scsi command block
		move.l	sp,IO_DATA(a0)		point to it in IORequest
		movea.l	sp,a1
		lea.l	-8(sp),sp		data area for inquiry
		move.l	sp,scsi_Data(a1)
		lea.l	-10(sp),sp		command area
		move.l	sp,scsi_Command(a1)
		clr.l	(sp)
		clr.l	4(sp)
		clr.w	8(sp)
		move.b	#READ_CAPACITY,(sp)	the command
		move.w	#10,scsi_CmdLength(a1)
		move.l	#8,scsi_Length(a1)
		clr.b	scsi_Flags(a1)
		move.l	a2,IO_UNIT(a0)		store the unit ptr
		move.l	#scsi_SIZEOF,IO_LENGTH(a0)
		move.w	#HD_SCSICMD,IO_COMMAND(a0)
		bsr	SendInternalCmd		send the read request
		lea.l	IOSTD_SIZE+scsi_SIZEOF+8+10(sp),sp
	ENDC

; we have a rigid disk block for this unit, now see if there is any mapping info
; Fixed at 256 blocks right now, should maybe use a cylinders worth or something
NoXTFix		move.l	#2048,d0		enough for 256 mapped blocks
		bsr	GetPubMem		we always allocate this space
		move.l	d0,hu_BadBlockList(a2)	save mapped block area
		beq.s	RDBExit			didn't get the memory
		movea.l	d0,a4			a4 points to the list

		movea.l	hu_RDB(a2),a0		fetch the RigidDiskBlock again
		move.l	rdb_BadBlockList(a0),d2	and see if any blocks are mapped
		bmi.s	RDBExit			no bad block list to scan

		move.l	hu_BlockSize(a2),d0	get memory for BadBlockList
		bsr	GetPubMem		(enough for one disk block)
		tst.l	d0			did we get it
		beq.s	RDBExit
		movea.l	d0,a3

; loops around here fetching bad block lists from the drive, d2 holds next block
ReadBadBlocks	move.l	d2,d0			we want this block
		move.l	#IDNAME_BADBLOCK,d1	its one of these
		movea.l	a2,a0			accessing this unit
		movea.l	a3,a1			where the data should go
		bsr	GetBlock
		tst.l	d0			did it work OK ?
		beq.s	BadBlocksRead		nope, exit now

		move.l	bbb_SummedLongs(a3),d0	see how many to scan
		subi.w	#bbb_BlockPairs,d0
		lsr.w	#3,d0			d0 holds count
		lea.l	bbb_BlockPairs(a3),a0	a0 points to first entry
		bra.s	NextBad

; loops around fetching bad block entries from the BadBlockBlock.  Entries
; consist of a block numebr of the block that went bad followed by a
; block number of the block it has been mapped to.
BadLoop		move.l	(a0)+,(a4)+		save bad block number
		ble.s	SortBadBlocks		reached end of list
		addq.w	#1,hu_TotalMapped(a2)	one more mapped block
		move.l	(a0)+,(a4)+		save block it's mapped to
NextBad		dbra	d0,BadLoop
		move.l	bbb_Next(a3),d2		fetch the next list of blocks
		bpl.s	ReadBadBlocks		there are some more

; we've got a list of blocks and the blocks they are mapped to.  Sort this list
; before we use it.  TotalMapped has been updated while reading
SortBadBlocks	movea.l	hu_BadBlockList(a2),a0	fetch pointer to blocks
		move.w	hu_TotalMapped(a2),d0
		ext.l	d0
		bsr	QSort			do the sort

	IFD DEBUG_CODE
	move.w	hu_TotalMapped(a2),d0
	movea.l	hu_BadBlockList(a2),a0
	bra.s	Showbb3
Showbb2	printf <'%ld,%ld\n'>,(a0)+,(a0)+
Showbb3	dbra	d0,Showbb2
	ENDC

; we've read all the bad blocks from this unit, so free buffer mem and exit
BadBlocksRead	movea.l	a3,a1
		move.l	hu_BlockSize(a2),d0
		jsr	_LVOFreeMem(a6)

RDBExit		move.l	hu_RDB(a2),d0		return ptr to RigidDiskBlock
		movem.l	(sp)+,d2/a2-a4/a6
		rts

;==============================================================================
; Flags = MountUnit( unit ),DeviceBase
;  d0		      a0	a5
;
; Given an initialised unit structure looks to see if that unit has already
; had all partitions mounted (hu_MountDone = 1).  If not, then the rigid disk
; block stuff is read to see if there are any partitions specified for this
; disk.  If there are, then they are all mounted and initialised as needed.
; The flags values are returned so that FindAndMount knows to look further.
; We will always read the RigidDiskBlock to get the flags for FindAndMount.
; For the XT devices we make sure LastLUN is set as well as LAST for unit 1.
;
; There is a special case in here to handle AutoBoot or Bindrivers mounted
; code.  If this was a binddrivers startup, hd_DiagArea will be null and we
; can use the normal expansion.library AddDosNode call to mount partitions.
; hd_DiagArea will also be null if we already executed this code once and
; mounted stuff at autoboot time.
;
; If hd_DiagArea is non-null then we have to roll our own AddDosNode to do
; all the BootNode initialisation.  This is a bunch of crap!  Why the hell
; wasn't this fixed when 1.3 autoboot code was "fixed".  Someone's gonna die!
;==============================================================================
MountUnit	movem.l	d2-d3/a2-a4/a6,-(sp)
		movea.l	a0,a2			stash the unit pointer
		movea.l	hd_SysLib(a5),a6	need exec.library
		tst.b	hu_MountDone(a2)	have mounts been done already ?
		bne	MUGetFlags		yes, just return flags
		movea.l	a2,a0			nope, get RigidDiskBlock
		bsr	GetRDBlock
		tst.l	d0			did it work ?

;		beq	MUGetFlags		nope, will return 0 flags
		beq	MUErrorFlags

		movea.l	d0,a0			is there any partition list
		move.l	rdb_PartitionList(a0),d2
		bmi	MUGetFlags		nope, just return flags

; we got some partition information, so start adding DOS nodes for each one
		move.l	hu_BlockSize(a2),d0	get memory for partition block
		bsr	GetPubMem
		tst.l	d0
;		beq	MUGetFlags		failed, just return flags
		beq	MUErrorFlags
		movea.l	d0,a3			stash partition area stuff

NextPartition	move.l	d2,d0			fetching this block
		move.l	#IDNAME_PARTITION,d1	should be this type
		movea.l	a2,a0			from this unit
		movea.l	a3,a1			to this memory
		bsr	GetBlock
		tst.l	d0			did we get it ?
		beq	PartsDone		nope, free partition memory

; should we attempt to mount this partition, or is it a "secret" one ?
		btst.b	#PBFB_NOMOUNT,pb_Flags+3(a3)
		bne	BagThisOne

; we are going to mount this partition so we'll be needing expansion.library
		lea.l	ExpanName(pc),a1
		moveq.l	#33,d0			1.2 KS or better
		jsr	_LVOOpenLibrary(a6)
		tst.l	d0
		beq	BagThisOne		didn't get the library
		movea.l	d0,a6

; First we construct a parameter packet ready for MakeDOSNode.  Since the
; environment vector is already in the partition block, we'll use four of
; the reserved longwords directly before it to store the DOS handler name,
; device name, unit number and open flags.  DOS handler name means the
; name of the drive when referred to by DOS (ie. DH0: or DF0: etc.)
;
; New addition: before using the DOS handler name, make sure it doesn't
; alread exist on either eb_MountList or DOS's device list.  If it does
; then it's algorithmically changed by appending dots.
		lea.l	pb_DriveName(a3),a0	get address of name BSTR
		bsr	BagDosDups		a6 = expansion.library

		lea.l	pb_Environment-16(a3),a0  where parameter packet goes
		lea.l	pb_DriveName(a3),a1	get the BSTR drive name
		clr.w	d0
		move.b	(a1)+,d0		make sure it's null terminated
		clr.b	0(a1,d0.w)
		move.l	a1,(a0)			store in parameter packet
		move.l	LN_NAME(a5),4(a0)	device name to open (me)
		moveq.l	#0,d0			construct unit #
		move.b	hu_LUN(a2),d0
		mulu.w	#10,d0
		or.b	hu_Unit(a2),d0
		move.l	d0,8(a0)		unit # to open
		move.l	pb_DevFlags(a3),12(a0)	with these flags
		jsr	_LVOMakeDosNode(a6)	make the node
		tst.l	d0			did we get it
		beq	MountPartFailed		nope, quit now

; the DOS node has been created, now patch any stuff we need to (like fs etc.)
		move.b	#1,hu_MountDone(a2)	flag, don't mount again
		movea.l	d0,a1			this DosNode
		movea.l	a2,a0			on this unit
		bsr	PatchFS			maybe patch in a filing system
		movea.l	d0,a4			new DosNode to a4

; we're ready to rock 'n roll now.  Add the node to DOS's list or eb_MountList
		move.l	a6,-(sp)
		movea.l	hd_SysLib(a5),a6
		jsr	_LVOForbid(a6)		gotta Forbid() around this
		movea.l	(sp)+,a6

		movea.l	a4,a0			DeviceNode to a0
		moveq.l	#-128,d3		default BootPri
		btst.b	#PBFB_BOOTABLE,pb_Flags+3(a3)  is this bootable ?
		beq.s	NormalAdd		nope, just mount it
		movea.l	dn_Startup(a0),a1	get the environment vector
		adda.l	a1,a1			this is the startup message
		adda.l	a1,a1
		movea.l	fssm_Environ(a1),a1	get environment vector
		adda.l	a1,a1
		adda.l	a1,a1
		move.l	de_BootPri(a1),d3	and get the boot priority
		ext.w	d3			in case we AddDosNode it
		ext.l	d3
		cmpi.b	#-128,d3		is it bootable ?
		beq.s	NormalAdd		nope, so just add the DOS node
		tst.l	hd_DiagArea(a5)		is this autoboot time ?
		beq.s	NormalAdd		nope, just do AddDosNode

	IFD ROMBOOT_FIXED
		bra.s	NormalAdd		it will make BootNode for me
	ENDC

; OK, we've got a bootable device at autoboot time so we have to construct
; our own bootnode and link our devicenode and configdev to it for booting
		move.l	a6,-(sp)		save expansionbase again
		movea.l	hd_SysLib(a5),a6
		moveq.l	#BootNode_SIZEOF,d0
		bsr	GetPubMem		get bootnode memory
		tst.l	d0
		beq.s	10$			didn't get it

		movea.l	d0,a1
		move.b	#NT_BOOTNODE,LN_TYPE(a1)
		move.b	d3,LN_PRI(a1)		set up boot priority
		move.l	hd_ConfigDev(a5),LN_NAME(a1)
		move.l	a4,bn_DeviceNode(a1)
		move.l	(sp),a0			get back expansionbase
		lea.l	eb_MountList(a0),a0
		jsr	_LVOEnqueue(a6)		add our bootnode to the list

10$		move.l	(sp)+,a6		restore expansionbase
		bra.s	AddDone			and close expansion library

; We've been called at binddrivers time or after booting has completed so just
; add and mount this partition using the normal expansion.library calls.
NormalAdd	moveq.l	#1,d1			StartProc = true
		move.l	d3,d0			this priority
		jsr	_LVOAddDosNode(a6)	don't care if it worked or not

; we've set our DOSNode up for booting or just a simple mount operation now.
AddDone		move.l	a6,-(sp)		gotta Permit() now
		movea.l	hd_SysLib(a5),a6
		jsr	_LVOPermit(a6)
		movea.l	(sp)+,a6

MountPartFailed	movea.l	a6,a1			close expansion
		movea.l	hd_SysLib(a5),a6
		jsr	_LVOCloseLibrary(a6)

; we've created and mounted the last partition, now look if there's another
BagThisOne	move.l	pb_Next(a3),d2		get next link
		bpl	NextPartition		there was another

PartsDone	movea.l	a3,a1
		move.l	hu_BlockSize(a2),d0	free temp memory
		jsr	_LVOFreeMem(a6)
		bra.s	MUGetFlags

MUErrorFlags	moveq.l	#-1,d1			so Open can return an error
		bra.s	MUGFCont		when we fail to mount partitions

; We've done all we have to do so return the flags from the RigidDiskBlock
MUGetFlags	moveq.l	#0,d1			non-error return
MUGFCont	move.l	hu_RDB(a2),d0		is there a RigidDiskBlock ?
		beq.s	MUGotFlags		nope, flags=0
		movea.l	d0,a0
		move.l	rdb_Flags(a0),d0	yes, get the real flags

; some special casing required here because XT drive only has unit 0
MUGotFlags:
		move.b	d0,hu_Flags(a2)		save flags for later use
	IFD XT_SUPPORTED
		tst.b	hu_Type(a2)		is this an XT drive
		bne.s	MUNotXT			no, so no special checks
		bset.l	#RDBFB_LASTLUN,d0	always last logical unit
		bset.l	#RDBFB_LAST,d0		and always the last physical
	ENDC

MUNotXT		movem.l	(sp)+,d2-d3/a2-a4/a6
		rts

;==============================================================================
; DosNode = PatchFS( unit, DosNode )
;   d0		      a0      a1
;
; Given a unit pointer and a DosNode that was initialised by MakeDosNode, this
; routine will check the DosType field of the environment vector and determine
; if a file system needs to be loaded from the RigidDiskBlocks.  If this is a
; ROM filing system (DosType = 'DOS\0' for 1.2 or DosType='DOS\0'|'DOS\1' for
; 1.4) then no load will be done and the DosNode will be returned 'as is'.  If
; it's determined that we do need to load a filing system, first try to find a
; file system resource that matches this DosType and patch it into the DosNode
; if we find it.  If the resource can't be found, we'll attempt to find it on
; the RigidDiskBlocks.  If it's not found, we'll fail and default to ROM filing
; system.  If it is found we'll load it, make a resource from it and patch the
; DosNode to point to this filing system.  Always returns the given DosNode.
;==============================================================================
PatchFS		movem.l	d2/a2-a4/a6,-(sp)
		movea.l	a0,a2			stash unit pointer
		movea.l	a1,a3			and DosNode pointer
		movea.l	hd_SysLib(a5),a6	we'll be needing exec
		movea.l	dn_Startup(a3),a1	get the environment vector
		adda.l	a1,a1			this is the startup message
		adda.l	a1,a1
		movea.l	fssm_Environ(a1),a1	get environment vector
		adda.l	a1,a1
		adda.l	a1,a1
		move.l	de_DosType(a1),d2	get the dosType
		cmpi.l	#$444f5300,d2		is it a normal filing system
		beq	FSPatched		yep, so no patch needed

; we got a non-standard DOS type, so see if an fs.resource is hanging around
		lea.l	FSResName(pc),a1
		jsr	_LVOOpenResource(a6)
		movea.l	d0,a4			a4 points to head of list
		tst.l	d0			did we get it ?
		bne.s	GotFSResource		yep, scan for the FS

; we didn't find fs.resource so we are responsible for creating the thing
		moveq.l	#FileSysResource_SIZEOF,d0
		bsr	GetPubMem
		movea.l	d0,a1
		tst.l	d0			did we get it ?
		beq.s	FSPatched		nope, just exit

		lea.l	fsr_FileSysEntries(a1),a0
		NEWLIST	a0
		lea.l	FSResName(pc),a0	this is it's name
		move.l	a0,LN_NAME(a1)
		move.b	#NT_RESOURCE,LN_TYPE(a1) and this is the type
		move.l	LIB_IDSTRING(a5),fsr_Creator(a1)  I made this
		movea.l	a1,a4			where we'll be searching
		jsr	_LVOAddResource(a6)	add it to the system

; we've got a file system resource in a4.  Scan for matching dostype (in d2)
GotFSResource	move.l	fsr_FileSysEntries(a4),d0   fetch first node
10$		movea.l	d0,a0
		move.l	(a0),d0			fetch the next node
		beq.s	20$			didn't get any that matched
		cmp.l	fse_DosType(a0),d2	does this match our DosType ?
		bne.s	10$			nope, keep scanning
		bra.s	GotFS			yep, patch the DOSNode

***** need to add a test for version here (not supported until 1.4) *****

; we have a filesystem resource, but the file system we need is not on it.
20$		move.l	d2,d0			we want this DOSType
		movea.l	a2,a0			from this unit
		bsr.s	LoadFS			go load the filing system
		tst.l	d0			did we get an fse struct
		beq.s	FSPatched		nope, so no patch done

; we've LoadSeg'ed a filing system so now hang it off the fs.resource chain
		move.l	d0,-(sp)		stash the pointer
		movea.l	d0,a1
		lea.l	fsr_FileSysEntries(a4),a0
		ADDHEAD
		movea.l	(sp)+,a0		get pointer to a0

; we have a pointer to the filing system to be patched into this partitions
; DOSNode in a0.  Use fse_PatchFlags to determine what changes in the DosNode
GotFS		move.l	fse_PatchFlags(a0),d0	DOSNode entries to be patched
		lea.l	fse_Type(a0),a0		where patches come from
		lea.l	dn_Type(a3),a1		what we're patching

NextPatch	lsr.l	#1,d0			see if we should patch
		bcc.s	10$			nope
		move.l	(a0),(a1)		yep, patch this field
10$		addq.l	#4,a0			bump pointers
		addq.l	#4,a1
		tst.l	d0			any patches left ?
		bne.s	NextPatch		yep

; we may not have patched this DOSNode but return a pointer to it anyway
FSPatched	move.l	a3,d0			return DosNode pointer
		movem.l	(sp)+,d2/a2-a4/a6
		rts

;==============================================================================
; FileSysEntry = LoadFS( DOSType, Unit )
;     d0		    d0     a0
;
; Given a DOSType and a unit scans the RigidDiskBlock area of the disk for a
; file system that matches the DOSType.  If one is found, it is LoadSeg'ed and
; a FileSysEntry is created ready for hanging off the fs.resource.  If no
; matching DOSType is found or an error occurs, this routine will return 0
;==============================================================================
LoadFS		movem.l	d2/a2-a4/a6,-(sp)
		movea.l	a0,a2			stash unit
		move.l	d0,d2			and DosType
		movea.l	hd_SysLib(a5),a6	need exec for GetPubMem
		suba.l	a4,a4			no FileSysEntry created yet

		moveq.l	#rh_SIZEOF,d0		we'll need a read handle
		add.l	hu_BlockSize(a2),d0	with an associated buffer
		bsr	GetPubMem
		tst.l	d0
		beq	LoadFSExit		didn't get the memory
		movea.l	d0,a3			a3 always points to it
		move.l	a2,rh_Unit(a3)		stash the unit

		movea.l	hu_RDB(a2),a0		get the RigidDiskBlock
		move.l	rdb_FileSysHeaderList(a0),d0
		bmi.s	NoFSLoaded		no file systems here

; scan all of the FileSysHeader blocks for one that matches the DOSType
CheckNextFS	move.l	#IDNAME_FILESYSHEADER,d1	what type we expect
		movea.l	a2,a0			get it from this unit
		lea.l	rh_Buff(a3),a1		and into this buffer
		bsr	GetBlock
		tst.l	d0			did it succeed ?
		beq.s	NoFSLoaded		nope, exit now
		cmp.l	rh_Buff+fhb_DosType(a3),d2  correct DOSType ?
		beq.s	FoundFS			yep, make an fse_ struct
		move.l	rh_Buff+fhb_Next(a3),d0	nope, get next in chain
		bmi.s	NoFSLoaded		none there
		bra.s	CheckNextFS		else, keep scanning

; we found a FileSysHeader that matches the DOSType, make an fs resource entry
FoundFS		moveq.l	#fse_SIZEOF,d0		get the structure
		bsr	GetPubMem
		tst.l	d0
		beq.s	NoFSLoaded		failed the allocation
		movea.l	d0,a4

; copy all the DOSNode stuff from the FileSysHeader to our FileSysEntry memory
; I'm only copying 12 longwords here because I don't think more is ever needed
		lea.l	rh_Buff+fhb_DosType(a3),a0  copying from here
		lea.l	fse_DosType(a4),a1	to here
		moveq.l	#11,d0			copying 12 entries
10$		move.l	(a0)+,(a1)+
		dbra	d0,10$

; initialise the read handle so that the first request makes us read a block
		clr.l	rh_DataLeft(a3)		no data read yet
		move.l	rh_Buff+fhb_SegListBlocks(a3),rh_Buff+lsb_Next(a3)
		move.l	a2,rh_Unit(a3)
		move.l	a5,rh_Globals(a3)	read routine needs globals

; we're ready to attempt a load on this filesystem.  Call loadseg with the
; appropriate parameters and patch fse_SegList with the return value.
		movem.l	a2-a3,-(sp)		stash our registers
		lea.l	_LVOAllocMem(a6),a0	allocfunc
		lea.l	_LVOFreeMem(a6),a1	freefunc
		suba.l	a2,a2			table ?????
		move.l	a3,d1			read handle
		lea.l	LFSRead(pc),a3		readfunc
		bsr	LoadSeg			call loadseg
		movem.l	(sp)+,a2-a3		restore our registers
		move.l	d0,fse_SegList(a4)	stash seglist pointer
		bne.s	FSLoaded		we got it, all done now

;something went wrong during the load of the filing system, free fse struct
NoFSLoaded	cmpa.w	#0,a4			did we allocate the structure
		beq.s	FSLoaded		nope, free read handle
		moveq.l	#fse_SIZEOF,d0		yes free it
		movea.l	a4,a1
		jsr	_LVOFreeMem(a6)
		suba.l	a4,a4			and return 0

; ready to clean up, a4 contains 0 or a pointer to the FileSysEntry structure
FSLoaded	moveq.l	#rh_SIZEOF,d0		free the read handle
		add.l	hu_BlockSize(a2),d0
		movea.l	a3,a1
		jsr	_LVOFreeMem(a6)

LoadFSExit	move.l	a4,d0			return 0 or FileSysEntry
		movem.l	(sp)+,d2/a2-a4/a6
		rts

;==============================================================================
; Actual = LFSRead( ReadHandle, Buffer, Length )
;   d0			d1	  a0	  d0
;
; This is actually a call-back support routine for LoadSeg.  It basically just
; copies data from the ReadHandle and refreshes it from the linked LoadSegList
; in the rigid disk block area when the data runs out.  This routine will die
; if LoadSeg ever attempts to retry a read that failed (no state info for that)
;==============================================================================
LFSRead		movem.l	d2-d3/a2-a3/a5-a6,-(sp)
		movea.l	d1,a2			readhandle to a2
		movea.l	a0,a3			caller buffer to a3
		moveq.l	#0,d3			actual = 0
		move.l	d0,d2			length to d2
		movea.l	rh_Globals(a2),a5	will need globals
		movea.l	hd_SysLib(a5),a6	may need exec

; d2 holds the amount of data left to read, d3 holds the amount we've done
ReRead		tst.l	d2			anything left to do ?
		beq.s	LFSReadDone		nope, exit now, d3=actual
		move.l	rh_DataLeft(a2),d0	any data left in buffer
		bne.s	DoCopy			yes, copy from there first

; we've exhausted the read buffer so link to the next LoadSegBlock
		move.l	rh_Buff+lsb_Next(a2),d0	get the next block
		bmi.s	LFSReadDone		nothing to get
		move.l	#IDNAME_LOADSEG,d1	this type of block
		movea.l	rh_Unit(a2),a0		from this unit
		lea.l	rh_Buff(a2),a1		to this buffer
		bsr	GetBlock
		tst.l	d0			did we get it ?
		beq.s	LFSReadDone		nope, return current actual
		move.l	rh_Buff+lsb_SummedLongs(a2),d0  get amount of data
		subq.l	#5,d0			allow for lseg header
		lsl.l	#2,d0			make into a byte count
		move.l	d0,rh_DataLeft(a2)	stash as amount left in buffer
		lea.l	rh_Buff+lsb_LoadData(a2),a1
		move.l	a1,rh_Data(a2)		initialise pointer too

; we have some data left in the buffer ready for copying to the caller buffer
DoCopy		cmp.l	d2,d0			len = min(length,left)
		ble.s	10$
		move.l	d2,d0			caller asked for less than left
10$		movea.l	rh_Data(a2),a0		source data
		movea.l	a3,a1			destination
		add.l	d0,d3			bump actual
		add.l	d0,a3			bump destination pointer
		add.l	d0,rh_Data(a2)		bump source pointer
		sub.l	d0,d2			decrement length left
		sub.l	d0,rh_DataLeft(a2)	decrement amount in buffer
		jsr	_LVOCopyMem(a6)		and do the copy
		bra.s	ReRead			and check for more work

; the read has completed for one reason or another, return actual in d0
LFSReadDone	move.l	d3,d0			return actual
		movem.l	(sp)+,d2-d3/a2-a3/a5-a6
		rts

;==============================================================================
; BagDosDups( BSTRname ), ExpansionBase
;		a0		a6
;
; Given an APTR to a BSTR, checks if that DOS handler name is already in use on
; either eb_Mountlist (a bunch of BootNodes) or DOS's device list.  If a name
; conflict occurs, then the given name is changed IN PLACE by appending dots.
; The process is iterated until no conflict occurs.
;
; NOTE: At boot time, there is no way of resolving a conflict with DF0: or any
; other DOS devices (such as SER:, PAR:, PRT:) because they are not mounted
; yet.  It would be possible to hard code them in here but it should really
; be DOS's or HDToolBox's responsibility to nix reserved names.
;==============================================================================
BagDosDups	movem.l	d2/a2-a3,-(sp)
		movea.l	a0,a2			save pointer to name

; First search eb_Mountlist for a conflicting name.  Fix for an expansion
; or DOS bug in here (probably expansion).  If the system was booted from
; floppy, even if autoboot hard drives were mounted, eb_Mountlist is corrupt!
; If boot is from a hard drive, then eb_Mountlist is OK.  So, the only
; time we can safely scan eb_Mountlist is if we're being brought in by
; expansion at autoboot time.
ReBag		tst.l	hd_DiagArea(a5)		autoboot time ?
		beq.s	CheckDosList		only check expansion if so
		move.l	eb_MountList(a6),d2	fetch first node
ExpBag		movea.l	d2,a3			node pointer to a3
		move.l	(a3),d2			look ahead to next node
		beq.s	CheckDosList		nothing matched, check DOS
		movea.l	bn_DeviceNode(a3),a0	fetch the device node
		movea.l	dn_Name(a0),a0		fetch pointer to name
		adda.l	a0,a0
		adda.l	a0,a0			it was a BPTR
		movea.l	a2,a1			get candidate name
		bsr	CompString		and check if the same
		tst.l	d0
		bne.s	ExpBag			not the same, look for next
		bra.s	GotDup			we have a duplicate name

; Now check DOS's device list for a conflicting name (a bit tougher).  If
; we are at autoboot time then there's no need to check DOS (it's not there)
CheckDosList	move.l	a6,-(sp)
		movea.l	hd_SysLib(a5),a6
		lea.l	DosName(pc),a1
		moveq.l	#0,d0			anything will do
		jsr	_LVOOpenLibrary(a6)
		movea.l	d0,a3			save DOSBase (maybe)
		tst.l	d0
		beq.s	NoClose			didn't get the library
		movea.l	a3,a1
		jsr	_LVOCloseLibrary(a6)
NoClose		movea.l	(sp)+,a6		restore expansionbase
		cmpa.w	#0,a3			did we get DOS
		beq.s	NoDups			nope, so no duplicates
		movea.l	dl_Root(a3),a3		get RootNode
		movea.l	rn_Info(a3),a3		get info BPTR
		adda.l	a3,a3
		adda.l	a3,a3
		move.l	di_DevInfo(a3),d0	get first DevInfo struct

NextDVI		lsl.l	#2,d0			convert to APTR
		beq.s	NoDups			end of list
		movea.l	d0,a3
		tst.b	dn_Type(a3)		DLT_DEVICE = 0
		bne.s	NotDosDev		it's not a DOS device
		movea.l	dn_Name(a3),a0		fetch pointer to name
		adda.l	a0,a0
		adda.l	a0,a0			it was a BPTR
		movea.l	a2,a1			get candidate name
		bsr.s	CompString		and check if the same
		tst.l	d0
		beq.s	GotDup			found a duplicate
NotDosDev	move.l	dn_Next(a3),d0
		bra.s	NextDVI			keep looking

; we've found a duplicate name so if the name doesn't already end with .n
; add .1 to it.  Otherwise just increment the number on the end (.2 .3 etc)
; Then go through the whole procedure again until no matches are found.
GotDup		clr.w	d0
		move.b	(a2),d0			get string length
		cmpi.b	#'.',-1(a2,d0.w)	does it already have .n ?
		beq.s	BumpN			yes, so just increment number
		addq.w	#1,d0			bump the length
		move.b	#'.',0(a2,d0.w)		add a dot at the end
		addq.w	#1,d0
		move.b	#'1',0(a2,d0.w)		and a number
		move.b	d0,(a2)			resave the length
		bra	ReBag			and go search again

BumpN		addq.b	#1,0(a2,d0.w)		bump the number (breaks at 9)
		cmpi.b	#':',0(a2,d0.w)
		bne	ReBag
		move.b	#'A',0(a2,d0.w)		stop names ending in :
		bra	ReBag

; No duplicate entries were found (but the name could have been modified)
NoDups		movem.l	(sp)+,d2/a2-a3
		rts

;==============================================================================
; Result = Compstring( string1, string2 )
;   d0			 a0	   a1
;
; Compares two BStrings and returns a boolean indicating equality.  TRUE means
; the string are different.  Comparison is case insensitive (like DOS).
;==============================================================================
CompString	move.w	d2,-(sp)
		moveq.l	#0,d0
		move.b	(a0)+,d0
		cmp.b	(a1)+,d0
		bne.s	CompNotSame		not equal, return now
		move.w	d0,d1
		bra.s	20$

10$		move.b	(a0)+,d0		get next character
		bsr.s	CapitalChar		make capital
		move.b	d0,d2
		move.b	(a1)+,d0
		bsr.s	CapitalChar		make capital
		cmp.b	d0,d2
		bne.s	CompNotSame		not the same, quit now
20$		dbra	d1,10$
		moveq.l	#0,d0			strings compare OK
		bra.s	CompStrDone
CompNotSame	moveq.l	#TRUE,d0		strings don't compare
CompStrDone	move.w	(sp)+,d2
		rts

;==============================================================================
; Char = CapitalChar( char )
;  d0		       d0
;
; converts lower case characters to upper case and leaves all others alone.
; NEVER trashes any registers (except d0 of course), this can be relied on!!!
;==============================================================================
CapitalChar	cmpi.b	#'a',d0
		blt.s	10$
		cmpi.b	#'z',d0
		bgt.s	10$
		addi.b	#'A'-'a',d0
10$		rts

;==============================================================================
; NumUnits = FindAndMount(),DeviceBase
;    d0				a5
;
; Finds all units that are ready on the unit chain.  If they say they are some
; kind of disk unit then we'll attempt to mount the partitions on that unit too
;==============================================================================
FindAndMount	movem.l	d2-d4,-(sp)
		moveq.l	#0,d4			number of disk units created

		moveq.l	#0,d2			physical unit 0
NextPhysical	moveq.l	#0,d3			logical unit 0
NextLogical	move.w	d3,d0			get LUN*10
		mulu.w	#10,d0
		add.w	d2,d0			plus physical
		bsr	FindUnit		see if this unit was made
		tst.l	d0			did we find it ?
		bne.s	GotAUnit		yep, no need to make it

		move.w	d3,d0			get LUN*10
		mulu.w	#10,d0
		add.w	d2,d0			plus physical
		moveq.l	#TRUE,d1		wait for ready
		bsr	MakeUnit		go make this unit
		tst.l	d0			did the unit get made ?
		beq.s	BumpLogical		nope, look for the next one

GotAUnit	movea.l	d0,a0			yes, is it a disk ?
		tst.b	hu_IsDisk(a0)
		beq.s	BumpLogical		wrong kind of device

		addq.l	#1,d4			we found a disk unit
		bsr	MountUnit		attempt to mount this unit
		tst.l	d0			did it return any flags ?
		beq.s	BumpLogical		nope, search for next unit

		btst.l	#RDBFB_LAST,d0		was this the last one ?
		bne.s	FoundAndMounted		yes, we can exit now
		btst.l	#RDBFB_LASTLUN,d0	last logical unit ?
		bne.s	BumpPhysical		yep, move to next physical

BumpLogical	tst.w	hd_Type(a5)		XT only has unit 0
		beq.s	FoundAndMounted		so no need to look further

		addq.w	#1,d3			next logical unit #
		cmpi.w	#8,d3			gone too far ?
		bne.s	NextLogical		no, keep looking
BumpPhysical	addq.w	#1,d2			next physical unit #
		cmpi.w	#8,d2			gone too far ?
		bne.s	NextPhysical		no, keep looking

FoundAndMounted	move.l	d4,d0			return number of units mounted
		movem.l	(sp)+,d2-d4
		rts

;==============================================================================
; QSort( count, array )
;	   d0	  a0
;
; Given a pointer to an array containing count 8 byte entries, sorts them in
; ascending order based on the value of the first longword in each entry.
;==============================================================================
QSort		movem.l	d2-d4,-(sp)

forgap		move.l	d0,d2			for(gap = n/2; gap>0; gap /= 2)
gaploop		asr.l	#1,d2
		ble.s	endgap

fori		move.l	d2,d3			for(i = gap; i<n; i++)
iloop		cmp.l	d0,d3
		bge.s	gaploop

forj		move.l	d3,d4			for(j=i-gap; j>=0; j -= gap);
jloop		sub.l	d2,d4
		blt.s	loopi

		asl.l	#3,d4			accessing 8 byte entries
		asl.l	#3,d2

		lea.l	0(a0,d4.l),a1		if(v[j] > v[j+gap])
		move.l	(a1),d1
		cmp.l	0(a1,d2.l),d1
		ble.s	loopj
		move.l	0(a1,d2.l),0(a0,d4.l)	swap them
		move.l	d1,0(a1,d2.l)
		move.l	4(a1,d2.l),4(a0,d4.l)
		move.l	4(a1),4(a1,d2.l)

loopj		asr.l	#3,d4			fix up array indices
		asr.l	#3,d2
		bra.s	jloop

loopi		addq.l	#1,d3			i++;)
		bra.s	iloop

endgap		movem.l	(sp)+,d2-d4
		rts


ExpanName	DC.B	'expansion.library',0
		CNOP	0,2
FSResName	DC.B	'FileSystem.resource',0
		CNOP	0,2

		END
