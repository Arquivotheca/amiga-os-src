**
**	$Id: ramdrive.asm,v 36.26 92/05/21 11:27:02 darren Exp $
**
**	ramdrive device module
**
**	(C) Copyright 1988,1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	based on code (C) Copyright 1988 Robert R. Burns
**	used with permission
**

	SECTION ramdrive,code

BUILD_BITMAP	EQU	0

**	Included Files

	NOLIST
	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/resident.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/io.i"
	INCLUDE "exec/semaphores.i"
	INCLUDE "exec/tasks.i"
	INCLUDE "exec/memory.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/errors.i"
	INCLUDE	"exec/initializers.i"
	INCLUDE	"exec/ables.i"

	INCLUDE "libraries/configregs.i"
	INCLUDE "libraries/expansionbase.i"
	INCLUDE "libraries/dos.i"
	INCLUDE "libraries/dosextens.i"
	INCLUDE "libraries/filehandler.i"

	INCLUDE "devices/trackdisk.i"

	INCLUDE	"internal/librarytags.i"

	LIST

	INCLUDE "ramdrive_rev.i"

**	Imported Names

CALLLVO MACRO
		CALLLIB _LVO\1
	ENDM

XLVO	MACRO
	XREF	_LVO\1
	ENDM


	TASK_ABLES			;for PERMIT macro

	XLVO	AddHead			; Exec
	XLVO	AllocAbs		;
	XLVO	CloseLibrary		;
	XLVO	Enqueue			;
	XLVO	FindName		;
	XLVO	FindResident		;
	XLVO	FreeMem			;
	XLVO	InitSemaphore		;
	XLVO	InitStruct		;
	XLVO	ObtainSemaphore		;
	XLVO	OpenLibrary		;
	XLVO	AllocMem		;
	XLVO	CopyMem			;
	XLVO	TaggedOpenLibrary	;
	XLVO	ReleaseSemaphore	;
	XLVO	ReplyMsg		;
	XLVO	SumKickData		;
	XLVO	CacheClearU		;

	XLVO	MakeDosNode		; Expansion

	XLVO	UMult32			; Utility

	XLVO	DateStamp		; Dos
	XLVO	LockDosList		;
	XLVO	NextDosEntry		;
	XLVO	UnLockDosList		;


**	Local Structures

RDB_PROTECT	EQU	0
RDB_NOBOOT	EQU	1

MAXUNITS	EQU	100	; don't make more than 0-99

DEVICEMODPRI	EQU	25	; before unit module initialization
UNITMODPRI	EQU	20	; same as trackdisk device

DVN_RESERVED	EQU	64	; <=64 reserved for dn_Startup

KICKMEMTYPE	EQU	(MEMF_NO_EXPUNGE!MEMF_REVERSE!MEMF_KICK)

 STRUCTURE	RamdriveDevice,LIB_SIZE
    APTR    rd_ExecLib
    STRUCT  rd_SSemaphore,SS_SIZE
    STRUCT  rd_UnitList,MLH_SIZE
    LABEL   RamdriveDevice_SIZEOF

 STRUCTURE	RamdriveUnit,MLN_SIZE	; also for allocation corruption
    APTR    ru_Data			; pointer to start of disk data
    STRUCT  ru_ResidentTag,RT_SIZE
    STRUCT  ru_ResidentName,20		; "ramdrive.unit ##\r\n\0"
    STRUCT  ru_ResidentInitCode,6	; JSR UnitInit
    STRUCT  ru_KickMemEntry,ML_SIZE+(2*ME_SIZE)
    STRUCT  ru_KickResArray,8		; (two longword entries)
    STRUCT  ru_BootNode,BootNode_SIZEOF
    LABEL   ru_MakeDosData		; thru ru_DosEnvec
    APTR    ru_DrivePtr			; pointer to ru_DriveName
    APTR    ru_DevicePtr		; pointer to "ramdrive.device"
    ULONG   ru_UnitNum
    ULONG   ru_FirstFlags		; flags associated w/ first open
    STRUCT  ru_DosEnvec,DosEnvec_SIZEOF
    STRUCT  ru_DriveName,32		; e.g. RAD: (from mount entry)
    ULONG   ru_LastFlags		; flags associated w/ last open
    ULONG   ru_MaxBytes			; maximum size in bytes
    UBYTE   ru_BootPri			; cache boot pri
    LABEL   RamdriveUnit_SIZEOF

**********************************************************************
**	Device Initialization
**********************************************************************

**	Resident Tag

DeviceResidentTag:
		dc.w	RTC_MATCHWORD
		dc.l	DeviceResidentTag
		dc.l	DeviceResidentEnd
		dc.b	RTF_AUTOINIT!RTF_COLDSTART
		dc.b	VERSION
		dc.b	NT_DEVICE
		dc.b	DEVICEMODPRI
		dc.l	DeviceName
		dc.l	DeviceID
		dc.l	DeviceAutoInit

**	AUTOINIT Table

DeviceAutoInit:
		dc.l	RamdriveDevice_SIZEOF
		dc.l	FuncInitTable
		dc.l	0		; no InitStruct data
		dc.l	DeviceInitFunction

**	Device Function Initialization Table

FuncInitTable:
		dc.w	-1
		dc.w	DeviceOpen-FuncInitTable
		dc.w	DeviceClose-FuncInitTable
		dc.w	DeviceExpunge-FuncInitTable
		dc.w	DeviceNil-FuncInitTable
		dc.w	DeviceBeginIO-FuncInitTable
		dc.w	DeviceAbortIO-FuncInitTable
		dc.w	DeviceKillRAD0-FuncInitTable
		dc.w	DeviceKillRAD-FuncInitTable
		dc.w	-1

**	Device Initialization Code

DeviceInitFunction:
		move.l	a5,-(a7)
		move.l	d0,a5
		;--	too few to justify InitStruct table
		move.w	#REVISION,LIB_REVISION(a5)
		move.w	#1,LIB_OPENCNT(a5)

		move.l	a6,rd_ExecLib(a5)

		lea	rd_UnitList(a5),a1
		NEWLIST	a1

		lea	rd_SSemaphore(a5),a0
		CALLLVO InitSemaphore

		move.l	a5,d0
		move.l	(a7)+,a5
		rts


**********************************************************************
**	Unit Initialization
**********************************************************************

UnitInit:
		move.l	(a7)+,a0	; get @ ru_ResidentInitCode+6
		movem.l	d7/a4-a5,-(a7)
		lea	-ru_ResidentInitCode-6(a0),a4

		;-- find ramdrive.device
		lea	DeviceList(a6),a0
		lea	DeviceName(pc),a1
		CALLLVO	FindName
		tst.l	d0
		beq.s	uiFail1
		move.l	d0,a5

		;-- mount the unit
		;--	get the expansion library that know how to mount things
		moveq	#OLTAG_EXPANSION,d0
		CALLLVO TaggedOpenLibrary
		move.l	d0,d7
		beq.s	uiFail1

		;--	set up Mount node
		exg	d7,a6
		lea	ru_MakeDosData(a4),a0
		CALLLVO MakeDosNode
		lea	eb_MountList(a6),a0	; get eb_MountList for Enqueue
		exg	d7,a6
		move.l	d0,ru_BootNode+bn_DeviceNode(a4)
		beq.s	uiFail2

		;-- restore boot priority

		move.b	ru_BootPri(a4),ru_BootNode+LN_PRI(a4)
		
		;--	add ru_BootNode (containing Mount node) to system list
		lea	ru_BootNode(a4),a1
		CALLLVO Enqueue

		;-- add unit to device
		lea	rd_UnitList(a5),a0
		move.l	a4,a1
		CALLLVO	AddHead

uiFail2:
		move.l	d7,a1		; ExpansionBase
		CALLLVO CloseLibrary

uiFail1:
		movem.l	(a7)+,d7/a4-a5
		rts


**********************************************************************
**	Drive Boot Code
**********************************************************************

**	Overlapped Fake Expansion Structures

	; -- ConfigDev		; -- BootDiagArea
	; Type	; (Prod)
	; (Flag	; Res03)	; Config; (Flags)
	; (Manufacturer)	; (Size)
	; (Serial ...		; (DiagPoint) 
	;   ... Number)		; Autoboot
	; (InitDiagVec)		; DeviceName
	; BootDiagArea
	;   ...

FakeConfigDev	equ	*-cd_Rom
		dc.b	ERTF_DIAGVALID			; er_Type
		dc.b	0				; er_Product
BootDiag	dc.b	DAC_CONFIGTIME	; da_Config	; er_Flags
		dc.b	0		; da_Flags	; er_Reserved03
		dc.w	0		; da_Size	; er_Manufacturer
		dc.w	0		; da_DiagPoint	; er_SerialNumber
		dc.w	AutoBoot-BootDiag ; da_BootPoint ;  ...
		dc.w	DeviceName-BootDiag ; da_Name	; er_InitDiagVec
rrPatch10	dc.l	BootDiag			; er_Reserved0c...0f

		;--	AutoBoot code
AutoBoot:

		moveq	#OLTAG_DOS,d0
		CALLLVO	TaggedOpenLibrary		;does InitResident()!
							;should not return
**		lea	DosName(pc),a1
**		CALLLVO FindResident
**		tst.l	d0
**		beq.s	abRts
**
**		move.l	d0,a0
**		move.l	RT_INIT(a0),a0
**		jsr	(a0)
abRts:
		rts

**********************************************************************
**	Byte-aligned Constants
**********************************************************************

DeviceName	dc.b	'ramdrive.device',0
RDNAMELEN	EQU	*-DeviceName-1
**DosName		dc.b	'dos.library',0
DeviceID	VSTRING
		ds.w	0


**********************************************************************
**	Device Commands
**********************************************************************

*****i* ramdrive.device/Open *****************************************
*
*   error = OpenDevice(name, unit, ioReq, flags);
*   d0			     d0	   a1	  d1
*
*   a6 contains device node
*   Invoked while Forbid
*
*   0x01 flag write-protects the disk
*   0x02 flag inhibits booting
*
**********************************************************************
DeviceOpen:
		cmp.l	#MAXUNITS,d0
		bcc.s	doFail		; bge unsigned

		move.l	rd_UnitList(a6),a0
doUnitLoop:
		tst.l	(a0)
		beq.s	doBuildUnit
		cmp.l	ru_UnitNum(a0),d0
		beq.s	doFoundUnit
		move.l	(a0),a0
		bra.s	doUnitLoop

doFoundUnit:
		move.l	d1,ru_LastFlags(a0)
		move.l	a0,IO_UNIT(a1)
		moveq	#0,d0
		rts

doFail:
		moveq	#IOERR_OPENFAIL,d0
		move.b	d0,IO_ERROR(a1)
		rts

doBuildUnit:
		movem.l	d2-d7/a1-a6,-(a7)
		move.l	d0,d4		; cache unit
		move.l	d1,d5		;   and flags
		move.l	a6,a5		;   and device
		move.l	rd_ExecLib(a5),a6

		moveq	#IOERR_OPENFAIL,d2

		;-- acquire needed libraries
		;--	dos.library
		moveq	#OLTAG_DOS,d0	; need XxxxDosList() functions
		CALLLVO TaggedOpenLibrary
		move.l	d0,d6		; cache & test DosBase
		beq	doDone1		; fail w/ IOERR_OPENFAIL

		;--	utility.library
		moveq	#OLTAG_UTILITY,d0
		CALLLVO TaggedOpenLibrary
		move.l	d0,d7		; cache & test UtilityBase
		beq	doDone2		; fail w/ IOERR_OPENFAIL

		;-- find out how big to make the ram disk
		;--	find the device in the DOS DevInfo list
		exg.l	d6,a6		; cache SysBase & use DosBase
		moveq	#LDF_DEVICES!LDF_READ,d1
		CALLLVO	LockDosList
		move.l	d0,d1

doDosEntryLoop:
		moveq	#LDF_DEVICES!LDF_READ,d2
		CALLLVO	NextDosEntry
		move.l	d0,a2

		;--	    end of list?
		tst.l	d0
		beq	doGotDevNode

		;--	    compare the startup structures
		move.l	dn_Startup(a2),d0 ; get startup BPTR
		cmpi.l	#DVN_RESERVED,d0
		bls.s	doNextDosEntry	; not there, not this device

		lsl.l	#2,d0		; must be > than 256 per check
		move.l	d0,a3		; above against DVN_RESERVED

		;--	    compare unit
		cmp.l	fssm_Unit(a3),d4
		bne.s	doNextDosEntry

		;--	    compare the device names
		move.l	fssm_Device(a3),d0
		lsl.l	#2,d0
		move.l	d0,a0
		moveq	#0,d1

		;-- coded such that BSTR length may, or may not include
		;-- trailing 0 (lets not read 1 past end of string
		;-- if it doesn't!)


		move.b	(a0)+,d1	; BSTR length
		lea	DeviceName(pc),a1

1$:
		move.b	(a1)+,d0	; 'ramdrive.device',0 
		beq.s	ifGotDevNode

		sub.b	#1,d1		; BSTR length -1
		bmi.s	doNextDosEntry

		cmp.b	(a0)+,d0	; string,[0]
		beq.s	1$

doNextDosEntry:
		move.l	a2,d1
		bra.s	doDosEntryLoop

		;-- found NULL in 'ramdrive.device', see if we are
		;-- already at end of BSTR, or if we found ending NULL
		;-- in BSTR

ifGotDevNode:	

		tst.b	d1
		beq.s	doGotDevNode

		tst.b	(a0)		; found NULL @ end of BSTR?
		bne.s	doNextDosEntry

doGotDevNode:
		moveq	#LDF_DEVICES!LDF_READ,d1
		CALLLVO	UnLockDosList
		exg	d6,a6		; cache DosBase & use SysBase 

		move.l	a2,d0		; check for failure to find entry
		beq	doFail3

		;-- allocate the RamdriveUnit
		move.l	#RamdriveUnit_SIZEOF,d0
		move.l	#KICKMEMTYPE,d1
		CALLLVO	AllocMem
		tst.l	d0
		beq	doFail3
		move.l	d0,a4

		;-- fill the RamdriveUnit
		;--	initialize the constants
		lea	UnitInitTable(pc),a1
		exg	a2,a4
		move.l	#RamdriveUnit_SIZEOF,d0
		CALLLVO	InitStruct
		exg	a2,a4

		;--	initialize the self-pointers
		move.l	a4,ru_KickMemEntry+ML_ME+ME_ADDR(a4)
		lea	ru_ResidentTag(a4),a0
		move.l	a0,ru_ResidentTag+RT_MATCHTAG(a4)
		move.l	a0,ru_KickResArray(a4)
		lea	ru_ResidentName(a4),a0
		move.l	a0,ru_ResidentTag+RT_NAME(a4)
		move.l	a0,ru_ResidentTag+RT_IDSTRING(a4)
		move.l	a0,ru_KickMemEntry+LN_NAME(a4)
		lea	ru_ResidentInitCode(a4),a0
		move.l	a0,ru_ResidentTag+RT_INIT(a4)
		lea	ru_DriveName(a4),a0
		move.l	a0,ru_DrivePtr(a4)
		lea	RamdriveUnit_SIZEOF(a4),a0
		move.l	a0,ru_ResidentTag+RT_ENDSKIP(a4)

		;--	initialize the rest
		;--	    set the numeric digits in the unit name
		move.l	d4,d0
		divu	#10,d0
		add.b	d0,ru_ResidentName+14(a4)
		swap	d0
		add.b	d0,ru_ResidentName+15(a4)

		;--	    set the unit & flags
		move.l	d4,ru_UnitNum(a4)
		move.l	d5,ru_FirstFlags(a4)
		move.l	d5,ru_LastFlags(a4)

		;--	    copy environment vector
		move.l	fssm_Environ(a3),d0
		lsl.l	#2,d0
		move.l	d0,a0
		lea	ru_DosEnvec(a4),a1
		move.l	(a0)+,d0		; de_TableSize
		cmp.l	#DE_UPPERCYL,d0
		blt	doFail4		; not enough information to calc size

		moveq	#(DosEnvec_SIZEOF/4)-1,d1
		cmp.l	d0,d1
		bls.s	doCopyEnvironment	; ble unsigned
		move.l	d0,d1
doCopyEnvironment:
		move.l	d1,d0
		bra.s	doCopyEnvironmentEntry
doCopyEnvironmentLoop:
		move.l	(a0)+,d0
doCopyEnvironmentEntry:
		move.l	d0,(a1)+
		dbf	d1,doCopyEnvironmentLoop

		;--	    copy the dos drive name
		move.l	dn_Name(a2),d0
		lsl.l	#2,d0
		move.l	d0,a0
		lea	ru_DriveName(a4),a1
		moveq	#0,d0
		move.b	(a0)+,d0
		subq	#1,d0
doCopyDriveName:
		move.b	(a0)+,(a1)+
		dbf	d0,doCopyDriveName
		clr.b	(a1)

		;--	check if booting is enabled

		btst	#RDB_NOBOOT,d5
		bne.s	doGetDiskData	; explicitly disabled
		move.l	ru_DosEnvec+de_BootPri(a4),d0
		cmp.l	#-128,d0	; disabled because priority < -128
		blt.s	doGetDiskData
		cmp.l	#127,d0
		ble.s	doSetBootable
		moveq	#127,d0		; clip high priority to highest byte val
doSetBootable:
		move.b	#NT_BOOTNODE,ru_BootNode+LN_TYPE(a4)
		move.b	d0,ru_BootNode+LN_PRI(a4)

doGetDiskData:
		;-- cache priority
		move.b	ru_BootNode+LN_PRI(a4),ru_BootPri(a4)

		;-- get disk data size
		cmp.l	#128,ru_DosEnvec+de_SizeBlock(a4)
		bne	doFail4		; not a known disk format

		move.l	ru_DosEnvec+de_HighCyl(a4),d0
		sub.l	ru_DosEnvec+de_LowCyl(a4),d0
		addq.w	#1,d0
		;--	perform 32x32 multiplies w/ utility.library
		exg	d7,a6
		move.l	ru_DosEnvec+de_Surfaces(a4),d1
		CALLLVO	UMult32
		move.l	ru_DosEnvec+de_BlocksPerTrack(a4),d1
		CALLLVO	UMult32
		exg	d7,a6
		move.l	d0,d3		; save for root block calculation
		moveq	#9,d1		; 2^9 == 512
		lsl.l	d1,d0		; multiply by 512 to get byte size
		beq	doFail4

		;-- cache max size for safety check at r/w time

		move.l	d0,ru_MaxBytes(a4)

		;-- allocate the disk data
		addq.l	#8,d0		; for allocation corruption
		move.l	d0,d2		; save allocation size
		move.l	#KICKMEMTYPE,d1
		CALLLVO	AllocMem
		move.l	d0,ru_KickMemEntry+ML_ME+ME_SIZE+ME_ADDR(a4)
		beq	doFail4

		move.l	d2,ru_KickMemEntry+ML_ME+ME_SIZE+ME_LENGTH(a4)

		addq.l	#8,d0		; skip over corruption pad
		move.l	d0,ru_Data(a4)


		;-- format the drive
		;--	initialize boot block
		move.l	ru_DosEnvec+de_DosType(a4),d1
		bne.s	doDosTypeSet
		move.l	#ID_DOS_DISK,d1		; default dos type
doDosTypeSet:
		move.l	d0,a0
		moveq	#127,d0
doBootBlockLoop:
		move.l	d1,(a0)+
		dbf	d0,doBootBlockLoop

		;--	calculate the root key offset
		move.l	d3,d0
		subq.l	#1,d0
		add.l	ru_DosEnvec+de_Reserved(a4),d0
		lsr.l	#1,d0
	IFNE	BUILD_BITMAP
		move.l	d0,d5		; cache root block number
	ENDC
		moveq	#9,d1		; 2^9 == 512
		lsl.l	d1,d0		; multiply by 512 to get byte offset

		add.l	ru_Data(a4),d0
		move.l	d0,a2

		;--	clear the root block
		move.l	a2,a1
		moveq	#127,d0
		moveq	#0,d1
doClearRootLoop:
		move.l	d1,(a1)+
		dbf	d0,doClearRootLoop

		;--	fill in root key
		;--	    constants
		moveq	#2,d0		; Type = T.SHORT
		move.l	d0,(a2)		;
		moveq	#1,d0		; Secondary Type = ST.ROOT
		move.l	d0,508(a2)	;
		moveq	#(128-56),d0	; Hash Table Size
		move.l	d0,12(a2)	;

		;--	    dates
		exg	d6,a6
		lea	484(a2),a0	; Create Date
		move.l	a0,d1
		CALLLVO DateStamp
		lea	420(a2),a0	; Last Altered Date
		move.l	a0,d1
		CALLLVO DateStamp
		exg	d6,a6

		;--	    default volume name
		move.l	#$0552414D,432(a2)	; 5,'RAM'
		move.b	#'_',436(a2)		;      '_'
		cmp.w	#10,d4
		blt.s	doVolNameN
		move.b	#$06,432(a2)		; 6,
		move.b	ru_ResidentName+14(a4),437(a2) ;'#'
		move.b	ru_ResidentName+15(a4),438(a2) ; '#'
		bra.s	doBitmap
doVolNameN:
		move.b	ru_ResidentName+15(a4),437(a2) ;'#'

doBitmap:
	IFNE	BUILD_BITMAP
		;--	Check if bitmap can fit in one block.
		;	Compare the amount of memory reserved for disk data
		;	with that required by 127 longwords of 32 block
		;	descriptors (i.e. one bitmap block).
		;	If the disk is greater than that (~2M), it's not
		;	really an error -- the Validator will build the
		;	bitmap, but with an annoying requestor.
		sub.l	ru_DevEnvec+de_Reserved(a4),d3
		cmpi.l	#(127*32),d3
		bgt.s	doRootCheckSum
		;--	    point to the bitmap from the root
		move.l	d5,d0
		addq.l	#1,d0
		move.l	d0,316(a2)	; first bitmap page
		moveq	#-1,d0		; $FFFFFFFF
		move.l	d0,312(a2)	; BMFLAG true
		;--	    indicate the allocated blocks
		;		beyond the end of the bit map can be trash
		;		so make them ones (easy to set & sum)
		lea	516(a2),a1	; first bits
		move.l	a1,a0
		moveq	#(127-1),d1	; dbf counter for 127 longs
doBitMapSetLoop:
		move.l	d0,(a0)+
		dbf	d1,doBitMapSetLoop

		;--	    root block and this block are not free
		;		clear those bits and adjust the checksum
		;		(checksum of 127 $FFFFFFFFs is $0000007F)
		;--		root block
		move.l	d3,d1
		move.l	d3,d2
		and.w	#$1f,d1		; get bit number
		bclr	d1,d0		; clear the bit in the longword
		lsr.l	#5,d2		; get long offset
		lsl.l	#2,d2		;
		and.l	d0,0(a1,d2.l)	; clear this block's bit
		not.l	d0		; get the bit not set
		moveq	#$7F,d1
		add.l	d0,d1		; add it back to checksum
		;--		bitmap block
		addq.l	#1,d3
		move.l	d3,d2
		and.w	#$1f,d3		; get bit number
		moveq	#-1,d0
		bclr	d3,d0		; clear the bit in the longword
		lsr.l	#5,d2		; get long offset
		lsl.l	#2,d2		;
		and.l	d0,0(a1,d2.l)	; clear this block's bit
		not.l	d0		; get the bit not set
		add.l	d0,d1		; add it back to checksum
		;--	    store bitmap checksum
		move.l	d1,512(a2)

	ENDC
		;--	    checksum root key
doRootCheckSum:
		move.l	a2,a0
		moveq	#127,d0
		moveq	#0,d1
doRootCheckSumLoop:
		add.l	(a0)+,d1
		dbf	d0,doRootCheckSumLoop
		neg.l	d1
		move.l	d1,20(a2)

		;-- We built a JSR in high memory; clear cache

		CALLLVO	CacheClearU

		;-- add disk unit to KickMem
		;-- (under Forbid of this exec's invocation of this Open)
		;--	add KickMem to head of exec list
		lea	ru_KickMemEntry(a4),a0
		move.l	KickMemPtr(a6),(a0)
		move.l	a0,KickMemPtr(a6)
		;--	add KickTag to head of exec list
		lea	ru_KickResArray(a4),a0
		move.l	KickTagPtr(a6),4(a0)
		beq.s	tagListOK
		bset	#7,4(a0)	; link to previous head tag
tagListOK:
		move.l	a0,KickTagPtr(a6)
		CALLLVO SumKickData
		move.l	d0,KickCheckSum(a6)

		;-- add disk unit to library
		lea	rd_UnitList(a5),a0
		move.l	a4,a1
		CALLLVO	AddHead

		moveq	#0,d2		; no error
doDone3:
		move.l	d7,a1
		CALLLVO	CloseLibrary
doDone2:
		move.l	d6,a1
		CALLLVO	CloseLibrary
doDone1:
		move.l	d2,d0
		move.l	a4,d1
		movem.l	(a7)+,d2-d7/a1-a6
		move.l	d1,IO_UNIT(a1)
		move.b	d0,IO_ERROR(a1)
		rts

doFail4:
		move.l	a4,a1
		move.l	#RamdriveUnit_SIZEOF,d0
		CALLLVO	FreeMem
		suba.l	a4,a4

doFail3:
		moveq	#IOERR_OPENFAIL,d2
		bra.s	doDone3


UnitInitTable:
	    INITWORD	ru_ResidentTag+RT_MATCHWORD,RTC_MATCHWORD
	    INITBYTE	ru_ResidentTag+RT_FLAGS,RTF_COLDSTART
	    INITBYTE	ru_ResidentTag+RT_PRI,UNITMODPRI
	    INITSTRUCT	1,ru_ResidentName,,8
		dc.b	'ramdrive.unit 00',13,10
	    INITWORD	ru_ResidentInitCode,$4eb9	; JSR ...
	    INITLONG	ru_ResidentInitCode+2,UnitInit	;   UnitUnit
	    INITWORD	ru_KickMemEntry+ML_NUMENTRIES,2
	    INITLONG	ru_KickMemEntry+ML_ME+ME_LENGTH,RamdriveUnit_SIZEOF
	    INITLONG	ru_BootNode+LN_NAME,FakeConfigDev
	    INITLONG	ru_DevicePtr,DeviceName
		dc.w	0


*****i*	ramdrive.device/Close ****************************************
DeviceClose:
DeviceExpunge:
DeviceNil:
DeviceAbortIO:
		moveq	#0,d0
		rts


*****i* ramdrive.device/BeginIO***************************************
DeviceBeginIO:
		movem.l a2-a6,-(a7)

		move.l	a1,a3		; cache IO request
		move.l	IO_UNIT(a3),a4	; get unit address
		move.l	a6,a5		; cache device address
		move.l	rd_ExecLib(a5),a6

		;--	protect access to disk data
		lea	rd_SSemaphore(a5),a0
		CALLLVO ObtainSemaphore

		;--	check bounds of command
		move.w	IO_COMMAND(a3),d0
		andi.w	#$7fff,d0	;mask out extended commands
		cmpi.w	#DISKCOMMANDCNT,d0
		bcs.s	dbKnownCommand	; blt unsigned
		clr.w	d0
dbKnownCommand:
		;--	find command entry point
		add.w	d0,d0
		jsr	DiskCommands(pc,d0.w)

		move.b	d0,IO_ERROR(a3)

		;--	allow access to disk data
		lea	rd_SSemaphore(a5),a0
		CALLLVO ReleaseSemaphore

		btst	#IOB_QUICK,IO_FLAGS(a3)
		bne.s	dbQuickOut

		;--	reply the command
		move.l	a3,a1
		CALLLVO ReplyMsg

dbQuickOut:
		movem.l (a7)+,a2-a6
		rts

DiskCommands:
		bra.s	DiskInvalid
		bra.s	DiskReset
		bra.s	DiskRead
		bra.s	DiskWrite

		bra.s	DiskUpdate
		bra.s	DiskClear
		bra.s	DiskStop
		bra.s	DiskStart

		bra.s	DiskFlush
		bra.s	DiskMotor
		bra.s	DiskSeek
		bra.s	DiskFormat

		bra.s	DiskRemove
		bra.s	DiskChangeNum
		bra.s	DiskChangeState
		bra.s	DiskProtStatus

DISKCOMMANDCNT	EQU (*-DiskCommands)/2


**********************************************************************
**********************************************************************
*
*   Disk Commands
*
**********************************************************************
**********************************************************************
DiskInvalid:
DiskReset:
DiskStop:
DiskStart:
DiskFlush:
DiskSeek:
		moveq	#IOERR_NOCMD,d0
		rts


*****i* Motor ********************************************************
*
*   The motor is always on.
*
*****i* ChangeNum ****************************************************
*
*   The disk change count is always 1
*
**********************************************************************
DiskMotor:
DiskChangeNum:
oneActual:
		moveq	#1,d0
		move.l	d0,IO_ACTUAL(a3)
		;------ fall through to report no error

*****i* Update *******************************************************
*
*   This driver always immediately writes
*
*****i* Clear ********************************************************
*
*   This driver always reads fresh data
*
*****i* Remove *******************************************************
*
*   Since the disk is always in the drive, the associated software
*   interrupt is never invoked, but this command is accepted without
*   error.
*
**********************************************************************
DiskUpdate:
DiskClear:
DiskRemove:
		moveq	#0,d0
		rts


*****i* ProtStatus ***************************************************
*
*   The disk protection is forever, as DiskChange is not implemented.
*
**********************************************************************
DiskProtStatus:
		btst	#RDB_PROTECT,ru_LastFlags+3(a4)
		bne.s	oneActual

	;------ fall thru to zero actual

*****i* ChangeState **************************************************
*
*   The disk is always in the drive
*
**********************************************************************
DiskChangeState:
		moveq	#0,d0
		move.l	d0,IO_ACTUAL(a3)
		rts


*****i* Read *********************************************************
*
*   Read data from the ram disk.
*
**********************************************************************
DiskRead:
		move.l	ru_Data(a4),d0
		beq.s	killedData
		move.l	IO_OFFSET(a3),d1
		add.l	d1,d0
		move.l	d0,a0
		move.l	IO_DATA(a3),a1

		bra.s	transfer

**********************************************************************
killedData:
		moveq	#TDERR_DiskChanged,d0
		rts

*****i* Write ********************************************************
*
*   Write data to the ram disk.
*
**********************************************************************
*****i* Format *******************************************************
*
*   Format data to the ram disk -- same as a write.
*
**********************************************************************
DiskWrite:
DiskFormat:
		btst	#RDB_PROTECT,ru_LastFlags+3(a4)
		bne.s	protectedDisk

		move.l	IO_DATA(a3),a0
		move.l	ru_Data(a4),d0
		beq.s	killedData
		move.l	IO_OFFSET(a3),d1
		add.l	d1,d0
		move.l	d0,a1

transfer:
		move.l	IO_LENGTH(a3),d0
		add.l	d0,d1		;length + offset
		cmp.l	ru_MaxBytes(a4),d1
		bhi.s	maxexceeded

		CALLLVO	CopyMem		; A6 is ready to go

emptyTransfer:
		move.l	IO_LENGTH(a3),IO_ACTUAL(a3)
		moveq	#0,d0
		rts

protectedDisk:
		moveq	#TDERR_WriteProt,d0
		rts

maxexceeded:
		moveq	#IOERR_BADLENGTH,d0
		rts

******* ramdrive.device/KillRAD0 *************************************
*
*   NAME
*	KillRAD0 -- kill ramdrive.device unit 0
*
*   SYNOPSIS
*	devName = KillRAD(unit)
*
*	char *KillRAD( ULONG );
*
*   FUNCTION
*	Perform a KillRAD(0).  This function is retained for
*	historical reasons.
*
*   SEE ALSO
*	ramdrive.device/KillRAD
*
**********************************************************************
DeviceKillRAD0:
		moveq	#0,d0

******* ramdrive.device/KillRAD **************************************
*
*   NAME
*	KillRAD -- kill ramdrive.device unit
*
*   SYNOPSIS
*	devName = KillRAD(unit)
*
*	char *KillRAD( ULONG );
*
*   FUNCTION
*	Kill a RAD: disk and return its memory.  First, remove the
*	system hooks that recover the disk memory after reset, then
*	free the disk memory, and return the dos device name.
*
*   INPUTS
*	unit - the unit of the ramdrive.device to kill.
*
*   RESULTS
*	devName - a pointer to the colon terminated device name that
*		was associated with this unit.  This is a null
*		terminated C string.
*
**********************************************************************
DeviceKillRAD:
		movem.l a4-a6,-(a7)
		move.l	a6,a5
		move.l	rd_ExecLib(a5),a6

		;-- find the specified unit
		move.l	rd_UnitList(a5),a4
dkuFindUnit:
		move.l	(a4),d1
		beq	dkuNoUnit
		cmp.l	ru_UnitNum(a4),d0
		beq.s	dkuFoundUnit
		move.l	d1,a4
		bra.s	dkuFindUnit

dkuFoundUnit:
		;-- protect access to disk data
		lea	rd_SSemaphore(a5),a0
		CALLLVO ObtainSemaphore

		;-- protect exec's Kick lists

		FORBID

		;-- inhibit reset recovery of the device
		;--	remove KickTag from exec list
		lea	ru_KickResArray(a4),a0
		move.l	KickTagPtr(a6),a1
		cmp.l	a1,a0		; check if this is our tag list
		beq.s	dkuKickTagSlot

dkuNextTagSlot:
		move.l	(a1)+,d0
		bgt.s	dkuNextTagSlot
		beq.s	dkuKillMemList	; (huh? This shouldn't happen!)

		bclr	#31,d0		; get link to next
		cmp.l	d0,a0		; check if this is our tag list
		beq.s	dkuFoundTagSlot
		move.l	d0,a1
		bra.s	dkuNextTagSlot

dkuFoundTagSlot:
		;--	    their next will be what my next was
		move.l	4(a0),-(a1)
		bra.s	dkuKillMemList

dkuKickTagSlot:
		;--	    KickTagPtr will be what my next was
		move.l	4(a0),d0
		bclr	#31,d0
		move.l	d0,KickTagPtr(a6)

dkuKillMemList:
		;	remove KickMem from exec list
		lea	ru_KickMemEntry(a4),a0
		lea	KickMemPtr(a6),a1
dkuCheckMemSlot:
		move.l	(a1),d0
		beq.s	dkuSumKickData	; (huh? This shouldn't happen!)
		cmp.l	d0,a0
		beq.s	dkuFoundMemSlot
		move.l	d0,a1
		bra.s	dkuCheckMemSlot

dkuFoundMemSlot:
		;--	    their next is my next
		move.l	(a0),(a1)

dkuSumKickData:
		CALLLVO SumKickData
		move.l	d0,KickCheckSum(a6)

		PERMIT

		;-- free the disk data
		move.l	ru_KickMemEntry+ML_ME+ME_SIZE+ME_ADDR(a4),a1
		move.l	ru_KickMemEntry+ML_ME+ME_SIZE+ME_LENGTH(a4),d0
		CALLLVO	FreeMem

		clr.l	ru_Data(a4)	; show it's gone

		;-- allow access to (empty) disk data
		lea	rd_SSemaphore(a5),a0
		CALLLVO ReleaseSemaphore

		;-- return drive name terminated with a colon
		lea	ru_DriveName(a4),a0
		move.l	a0,d0
dkuColonedName:
		tst.b	(a0)+
		bne.s	dkuColonedName
		move.b	#':',-(a0)

dkuDone:
		movem.l (a7)+,a4-a6
		rts

dkuNoUnit:
		moveq	#0,d0
		bra.s	dkuDone


**********************************************************************
*
*   end of ram driver
*
**********************************************************************

DeviceResidentEnd:

	END
