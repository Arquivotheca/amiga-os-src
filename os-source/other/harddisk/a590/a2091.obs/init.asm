		SECTION driver,CODE

		NOLIST
		INCLUDE	"modifiers.i"
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/nodes.i"
		INCLUDE	"exec/lists.i"
		INCLUDE	"exec/resident.i"
		INCLUDE	"exec/alerts.i"
		INCLUDE	"exec/memory.i"
		INCLUDE	"exec/tasks.i"
		INCLUDE	"exec/interrupts.i"
		INCLUDE	"exec/execbase.i"
		INCLUDE	"libraries/configvars.i"
		INCLUDE	"libraries/expansion.i"
		INCLUDE	"device.i"
		INCLUDE	"scsidirect.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XREF	_AbsExecBase,_LVOOpenLibrary,_LVOCloseLibrary
		XREF	_LVOAddTask,_LVOAlert,_LVOAddDevice
		XREF	_LVOAllocMem,_LVOForbid,_LVOPermit

		XREF	_LVOGetCurrentBinding,_LVOFindResident
		XREF	_LVOFindTask,_LVOWait

		XREF	MyMakeLibrary,IOTask,Open,EndCode

		XDEF	GetPubMem

;==============================================================================
; Put some safe code here in case some jerk tries to run the device.
;==============================================================================
	IFND	AUTOBOOT_ROM
DummyCode	moveq.l	#-1,d0			return an error
		rts
	ENDC AUTOBOOT_ROM

	IFD	AUTOBOOT_ROM
;==============================================================================
; This is the ROM diagnostic table that's used to call us at config time.
;==============================================================================
DiagTable	DC.B	DAC_WORDWIDE+DAC_CONFIGTIME	rom type and call time
		DC.B	0
		DC.W	DiagEnd-DiagTable	how much to copy
		DC.W	RelocRT-DiagTable	what's called at config time
		DC.W	BootMe-DiagTable	DOS boot code
		DC.W	BootName-DiagTable	What's this for ?
		DC.W	0
		DC.W	0

;==============================================================================
; This code is called at boot time and is expected to call DOS's init routine.
; We've allready created boot nodes, so DOS will call us normally for booting.
;==============================================================================
BootMe		lea.l	DosName(pc),a1		find DOS's RomTag
		jsr	_LVOFindResident(a6)
		tst.l	d0			did we find it
		beq.s	BootedMe		nope, bad boot
		movea.l	d0,a0
		movea.l	RT_INIT(a0),a0		call init vector from RomTag
		jsr	(a0)			DOS may boot off our device
BootedMe	rts

		CNOP	0,4			DosName must be lword aligned
DosName		DC.B	'dos.library',0
		CNOP	0,4
BootName	DC.B	'romboot.device',0
		CNOP	0,2

;==============================================================================
; This code is actually called at config time and we use it to resolve all of
; the absolute references in the RomTag so that our init routine can be found
; at "binddrivers" time.  On entry to the routine, useful registers are:-
;
; a0 = BoardAddress (For A590/2091 the offset to the ROM code is $2000)
; a2 = pointer to a memory copy of DiagTable and this code up to EndCode
; a3 = pointer to the configdev struct for this board (can stash stuff there)
;==============================================================================
RelocRT		move.l	a1,-(sp)
		move.l	#AUTOBOOT_ROM,d0	offset from board to ROM
		add.l	a0,d0			d0 is correct offsets now
		lea.l	InitDescriptor(pc),a0	point to our RomTag
		lea.l	DiagEnd(pc),a1
		move.l	a0,RT_MATCHTAG(a0)	RomTag must point to itself
		move.l	a1,RT_ENDSKIP(a0)	just skip over ram version
		add.l	d0,RT_NAME(a0)		rest of stuff is in ROM
		add.l	d0,RT_IDSTRING(a0)
		add.l	d0,RT_INIT(a0)
; We need to find our DiagTable at autoboot time to construct a bootnode.  We
; stash it in the reserved0c field of this configdev so we can find it later.
	IFND ROMBOOT_FIXED
		move.l	a2,cd_Rom+er_Reserved0c(a3)
	ENDC
		moveq.l	#1,d0			indicate success
		move.l	(sp)+,a1
		rts

		ENDC  AUTOBOOT_ROM

;==============================================================================
; Here's the RomTag that lets BindDrivers find out where to call my init code.
; Since we're living in ROM this will get copied and relocated at config time.
;==============================================================================
InitDescriptor	DC.W	RTC_MATCHWORD		so we can be found
		DC.L	InitDescriptor		firewall consistency check
		DC.L	EndCode			how to skip over this code
		DC.B	RTW_COLDSTART		flags
		DC.B	VERSION			version number
		DC.B	NT_DEVICE		node type when we are linked
		DC.B	HD_PRI			priority to run this task at
		DC.L	DeviceName		root name of this device
		DC.L	IDString		descriptive ID string
		DC.L	Init			code to call for initialisation

DiagEnd:	;config code will copy from the beginning to here

	IFD A590
		CNOP	0,4
DeviceName	DC.B	'A590/A2091 IORequest handler',0
		CNOP	0,4

	IFD XT_SUPPORTED
IDString	DC.B	'SCSI/XT driver by SMB (01 May 89)',0
		CNOP	0,4
	ENDC

	IFND XT_SUPPORTED
IDString	DC.B	'SCSI driver by SMB (07 June 89)',0
		CNOP	0,4
	ENDC

ExpanName	DC.B	'expansion.library',0
		CNOP	0,2
SCSIRootName	DC.B	'scsi.device',0
		CNOP	0,2

	IFD XT_SUPPORTED
XTRootName	DC.B	'xt.device',0
		CNOP	0,2
	ENDC XT_SUPPORTED
	ENDC A590


	IFD A2090
		CNOP	0,4
DeviceName	DC.B	'A2090 IORequest handler',0
		CNOP	0,4
IDString	DC.B	'SCSI/ST506 driver by SMB',0
		CNOP	0,4
ExpanName	DC.B	'expansion.library',0
		CNOP	0,2
SCSIRootName	DC.B	'scsi.device',0
		CNOP	0,2
XTRootName	DC.B	'st506.device',0
		CNOP	0,2
	ENDC A2090

;==============================================================================
; Error = Init( DeviceName ),_AbsExecBase
;  d0		    a1		  a6
;
; Called by BindDrivers to initialise the device driver from nothing either
; at boot time (as part of the exec.library task) or at binddrivers time (as
; part of the initial CLI task).  This initialisation finds the board and
; then creates the main IOTask and the two library entries (xt.device and
; scsi.device).  Once this is complete, this init sequence attempts to open
; unit 0 on both devices using a flags value of -1 to force mounting of all
; known units and their partitions and file systems.  If this is boot time
; then the mounted partitions will be put on eb_Mountlist for DOS to boot from
;==============================================================================
Init		movem.l	d2-d5/d7/a2-a5,-(sp)

		suba.l	a1,a1			find this task
		jsr	_LVOFindTask(a6)
		move.l	d0,d5			stash for IOTask

		lea.l	ExpanName(pc),a1	first, find our board
		moveq.l	#0,d0			using any expansion.library
		jsr	_LVOOpenLibrary(a6)
		tst.l	d0			make sure we got it
		bne.s	GetBoardAddr		yep, no problems

; We didn't get the expansion library.  This is very serious so put up an alert
		move.l	#AN_TrackDiskDev!AG_OpenLib,d7
		jsr	_LVOAlert(a6)
		bra	BadExit			do we come back to here ?

; We opened the expansion library OK so now use it to find our board address
GetBoardAddr	movea.l	d0,a6			stash expansion lib base
		lea.l	-CurrentBinding_SIZEOF(sp),sp	get temp workspace
		movea.l	sp,a0			and point a0 at it
		moveq.l	#CurrentBinding_SIZEOF,d0
		jsr	_LVOGetCurrentBinding(a6)
		movea.l	a6,a1			done with expansion lib now
		movea.l	_AbsExecBase,a6		so it's safe to close it
		jsr	_LVOCloseLibrary(a6)

; get the first board on the list and stash its address for use by IOTask
		movea.l	cb_ConfigDev(sp),a0	save first configdev ptr
		lea.l	CurrentBinding_SIZEOF(sp),sp	reclaim workspace
		move.l	cd_BoardAddr(a0),a4	board address to a4
		bclr.b	#CDB_CONFIGME,cd_Flags(a0)	we've configured it
		move.l	cd_Rom+er_Reserved0c(a0),d3	d3=0 if not autoboot
		move.l	a0,d4				save ConfigDev anyway

; allocate the message port we want IOTask to use, we stash it for later use
		moveq.l	#MP_SIZE,d0		get a message port
		bsr	GetPubMem
		tst.l	d0
		beq.s	NoMemory
		movea.l	d0,a5			save port for library code

		move.l	#DEVSTACK,d0		get the stack memory
		bsr	GetPubMem
		move.l	d0,d2			stash for later AddTask call
		beq.s	NoMemory		didn't get the memory

		move.l	#TC_SIZE,d0		and the task control block
		bsr	GetPubMem
		tst.l	d0
		bne.s	StartIOTask		got it, now start the IOTask

; if we fail to get any memory, this code is called to put up an alert.
NoMemory	move.l	#AN_TrackDiskDev!AG_NoMemory,d7
		jsr	_LVOAlert(a6)
		bra	BadExit			do we come back to here ?

; we got the memory we need so start up the main IOTask (which handles
; IORequests).  It will, in turn, start up the SCSITask that talks to
; the hardware directly.  When it has completed we'll receive a signal.
StartIOTask	movea.l	d0,a1			point to TCB
		movea.l	d2,a0			and to the stack
		move.l	a0,TC_SPLOWER(a1)
		lea.l	DEVSTACK(a0),a0		top of stack
		move.l	a0,TC_SPUPPER(a1)
		move.l	a5,-(a0)		pass message port address
		move.l	d5,-(a0)		and a pointer to this task
		move.l	a4,-(a0)		and board address
		move.l	a6,-(a0)		and exec library
		move.l	a0,TC_SPREG(a1)		set initial sp

		move.b	#NT_TASK,LN_TYPE(a1)	this is a task
		move.b	#HD_PRI+1,LN_PRI(a1)	1 higher than current task
		lea.l	DeviceName(pc),a0	give it some kinda name
		move.l	a0,LN_NAME(a1)
		lea.l	IOTask(pc),a2		initialPC
		suba.l	a3,a3			no finalPC
		jsr	_LVOAddTask(a6)		and start it up

		moveq.l	#SIGF_SINGLE,d0
		jsr	_LVOWait(a6)		wait for init complete signal

; The task we just added has signalled us that it finished initialisation of
; itself and SCSITask.  Now we make the actual library nodes for both the
; xt.device and the scsi.device.
	IFD XT_SUPPORTED
		lea.l	XTRootName(pc),a0	make the xt.device
		bsr	MakeDevice		makes it but doesn't add it
		tst.l	d0			returns 0 = error
		beq.s	LibFailed
		movea.l	d0,a2			a2 points to xt.device
	ENDC

		lea.l	SCSIRootName(pc),a0	make the scsi.device
		bsr	MakeDevice		makes it but doesn't add it
		tst.l	d0			returns 0 = error
		beq.s	LibFailed
		movea.l	d0,a3			a3 points to scsi.device

; initialise some of the globals for each of the device entries
	IFD XT_SUPPORTED
		move.l	a6,hd_SysLib(a2)	exec.library pointer
		move.l	a5,hd_IOPort(a2)	IOTasks message port
		move.l	d3,hd_DiagArea(a2)	may be a diag table ptr here
		move.l	d4,hd_ConfigDev(a2)	so we'll need configdev ptr
		clr.w	hd_Type(a2)		mark this as an xt device
		lea.l	hd_HDUnits(a2),a1	initialise unit lists
		NEWLIST	a1
	ENDC

		move.l	a6,hd_SysLib(a3)
		move.l	a5,hd_IOPort(a3)
		move.l	d3,hd_DiagArea(a3)
		move.l	d4,hd_ConfigDev(a3)
		move.w	#1,hd_Type(a3)		mark this as a scsi device
		lea.l	hd_HDUnits(a3),a1
		NEWLIST	a1
		bra.s	AddDevices		everything is ready now

; We failed to make a device node so put up an alert (this is pretty drastic)
LibFailed	move.l	#AN_TrackDiskDev!AG_MakeLib,d7
		jsr	_LVOAlert(a6)
		bra.s	BadExit

; Finally, now we got this far, we can add our devices to the systems list. We
; are ready to be Opened and called now. This will happen when we mount stuff
; using the Open call with a flags value of -1 (find and mount all units).
AddDevices:
	IFD XT_SUPPORTED
		movea.l	a2,a1			add xt.device
		jsr	_LVOAddDevice(a6)
	ENDC

		movea.l	a3,a1			add scsi.device
		jsr	_LVOAddDevice(a6)

; since we know where the devices are in memory it is quite valid for us to
; call the Open routine of each of these devices directly.  We use a flags
; value of -1 which causes each device to allocate and initialise all units
; found on that particular device. Using this flags value means we need no
; IORequest or unit number since the Open routine will simply return here.
; Open will also return the number of device actually mounted so we know to
; retry the open operation if nothing at all responded.
		movea.l	a6,a5			stash exec.library
		moveq.l	#4,d2			maximum retry count
		moveq.l	#0,d3			total units found

ReInitUnits:
	IFD XT_SUPPORTED
		movea.l	a2,a6			xt device pointer to a6
		moveq.l	#-1,d1			flags are -1 (init units)
		bsr	Open			call directly (this is OK)
; took this out so the timeout jumper only affects scsi drives (XT not noted)
;		add.l	d0,d3
	ENDC

		movea.l	a3,a6			scsi device pointer to a6
		moveq.l	#-1,d1			flags are -1 (init units)
		bsr	Open			call directly
		add.l	d0,d3
		dbne	d2,ReInitUnits		none found, try again

		movea.l	a5,a6			restore exec.library

; if this was autoboot time, then available devices will have been added to
; the eb_Mountlist instead of DOS's device list.  To prevent this from being
; done on subsequent Opens with non zero flags the DiagArea ptrs are cleared
	IFD XT_SUPPORTED
		clr.l	hd_DiagArea(a2)		don't boot anymore....
	ENDC
		clr.l	hd_DiagArea(a3)		...just add to device list

;==============================================================================
; Everything initialised OK, so return non-zero in d0 to mark this fact.
;==============================================================================
GoodExit	moveq.l	#-1,d0			this means all went OK
		bra.s	Exit

BadExit		moveq.l	#0,d0			this means we screwed up
Exit		movem.l	(sp)+,d2-d5/d7/a2-a5
		rts

;==============================================================================
; DeviceBase = MakeDevice( rootname )
;     d0		      a0
;
; Makes a library node for the given root device name and constructs a unique
; name for that device based on the root name and the names already known by
; the system.  I'm calling a routine in a separate module because the linker
; or assembler were screwing up the initial library table if it had relative
; references to external routines.  Now the table is in the same module as
; the routines being referenced, there are no link problems.
;==============================================================================
MakeDevice	movem.l	a2-a3,-(sp)
		movea.l	a0,a2			stash the root name
		bsr	MyMakeLibrary		make the library node
		tst.l	d0			did we get it ?
		beq	MDFailed		nope, return 0
		movea.l	d0,a3			yes, stash lib base

		move.b	#NT_DEVICE,LN_TYPE(a3)	make our 'lib' a device
		move.b	#LIBF_SUMUSED!LIBF_CHANGED,LIB_FLAGS(a3)
		move.w	#VERSION,LIB_VERSION(a3)
		move.w	#REVISION,LIB_REVISION(a3)
		lea.l	IDString(pc),a0		some kinda descriptive ID
		move.l	a0,LIB_IDSTRING(a3)

		moveq.l	#24,d0			get memory for largest name
		bsr	GetPubMem
		move.l	d0,LN_NAME(a3)		this will be device name
		beq.s	MDFailed		no, return 0

		movea.l	d0,a1			now construct the name here
		movea.l	a2,a0
		bsr	MakeDeviceName
		tst.l	d0			did we get the name ?
		beq.s	MDFailed		nope, error out

		move.l	a3,d0			return devicebase in d0

MDFailed	movem.l	(sp)+,a2-a3
		rts

;==============================================================================
; name = MakeDeviceName( rootname, buffer ), _AbsExecBase
;  d0			    a0	     a1		   a6
;
; Constructs a device name in buffer based on the provided root name.  Scans
; execs list of devices to make sure there are no name collisions and prepends
; second, third, fourth, fifth etc. to the name until no collision occurs.
; The buffer should be large enough to hold the root name plus 10 characters.
;==============================================================================
MakeDeviceName	movem.l	d2-d3/a2-a4,-(sp)
		movea.l	a0,a2			save root name
		movea.l	a1,a3			and destination buffer
		lea.l	Prepends(pc),a4		list of names we use
		jsr	_LVOForbid(a6)		stop race conditions
		moveq.l	#7,d3			maximum names - 1

NextName	movea.l	a3,a1			get destination buffer
10$		move.b	(a4)+,(a1)+		copy pre-string to buffer
		bne.s	10$			up to the null terminator
		subq.l	#1,a1			went one byte too far
		movea.l	a2,a0			now append root name
20$		move.b	(a0)+,(a1)+
		bne.s	20$

		move.l	a2,-(sp)		stash root name pointer
		lea.l	DeviceList+LH_HEAD(a6),a2  point to first link
ScanList	move.l	(a2),d0			is there a next pointer ?
		beq.s	NoMatch			nope, we finished the scan
		movea.l	d0,a2			yep, check this entry
		movea.l	LN_NAME(a2),a0		devices must have names
		movea.l	a3,a1			check against constructed name
		bsr.s	strcmp
		tst.l	d0			was it the same ?
		bne.s	ScanList		nope, check the next one
		movea.l	(sp)+,a2		found a matching name
		dbra	d3,NextName		so construct another one
		suba.l	a3,a3			ran out of names, return 0
		bra.s	NoSpace			stack fixed up already

NoMatch		addq.l	#4,sp			scrap root name pointer
NoSpace		jsr	_LVOPermit(a6)		we were in Forbid() state
		move.l	a3,d0			return constructed name pointer
		movem.l	(sp)+,d2-d3/a2-a4	restore work registers
		rts				all done

Prepends	DC.B	0			first name is      xxxx.device
		DC.B	'2nd.',0		followed by    2nd.xxxx.device
		DC.B	'3rd.',0		followed by    3rd.xxxx.device
		DC.B	'4th.',0		followed by    4th.xxxx.device
		DC.B	'5th.',0		probably never more than this..
		DC.B	'6th.',0		...but just for safety...
		DC.B	'7th.',0		...I'm putting these in too
		DC.B	'8th.',0
		CNOP	0,2

;==============================================================================
; Length = strlen( string )
;  d0.w		     a0
;
; Returns the length of the given NULL terminated string in d0.w
;==============================================================================
strlen		moveq.l	#-1,d0		initialise count
10$		tst.b	(a0)+		check for ending byte
		dbeq	d0,10$		and branch back if not there yet
		addq.w	#1,d0		adjust count back up (started at -1)
		neg.w	d0		convert from negative to positive
		rts			and that's all

;==============================================================================
; result = strcmp( string1, string2 )
;   d0		     a0       a1
;
; Returns the relationship between string1 and string2.  Results returned are;
;
; -1	string2 < string1
;  0	string2 = string1
;  1	string2 > string1
;
; both string1 and string2 must be null terminated.
;==============================================================================
strcmp		move.l	a0,-(sp)	save string1 pointer
		bsr.s	strlen		get length of string1
		movea.l	(sp)+,a0	get back string1 pointer
10$		cmpm.b	(a0)+,(a1)+	start comparing memory
		dbne	d0,10$		loop until unequal
		blt.s	20$		string2 < string1
		bgt.s	30$		string2 > string1
		moveq.l	#0,d0		string2 = string1
		rts
20$		moveq.l	#-1,d0
		rts
30$		moveq.l	#1,d0
		rts

;==============================================================================
; Memory = GetPubMem( size )
;  d0  		       d0
;
; Allocates the required amount of public memory and returns its address or 0
;==============================================================================
GetPubMem	move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
		jmp	_LVOAllocMem(a6)

		END

