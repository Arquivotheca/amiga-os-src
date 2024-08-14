************************************************************************
****************                                        ****************
****************        -=  CDTV FILE SYSTEM  =-        ****************
****************                                        ****************
************************************************************************
***                                                                  ***
***   Written by Carl Sassenrath for Commodore International, Ltd.   ***
***   Copyright (C) 1990 Commodore-Amiga, Inc. All rights reserved.  ***                                                                  ***
***                                                                  ***
***         Confidential and Proprietary System Source Code          ***
***                                                                  ***
************************************************************************

************************************************************************
***
***  CDTV File System Startup
***
***	Builds BootNode in RAM
***
***	Sets up main entry point as RAM segment
***
***	V24.1 -	VERSION, REVISION, & VSTRING now used
***
***  Monsterous hacking by me (ewhac).
***
***	9110.29	First pass at attempting to create a disk-loadable DOS
***		handler.
***		dosextens.i
***		copyright.i
***
************************************************************************

***
***	MANX SUCKS EGGS!!!!!!!!!!!!!!!!!!!!!!!!!!!
***
***	It cannot do:
***	DC.W	A-B	for relocatables
***	and:
***	MOVE.L	(sp)+,D0/D1   does not create an error
***

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/exec.i"
	INCLUDE	"exec/resident.i"
	INCLUDE	"exec/tasks.i"
	INCLUDE "stddefs.i"
	INCLUDE	"libraries/dosextens.i"
	INCLUDE	"libraries/configvars.i"
	INCLUDE	"libraries/romboot_base.i"
	INCLUDE	"libraries/expansionbase.i"
	INCLUDE	"libraries/filehandler.i"
	INCLUDE	"expansion.lvo"
	INCLUDE "cdfs.i"
	INCLUDE	"cdtv:copyright.i"

	XREF	_DOSBase
	XREF	_LibBase
	XREF	_ExpBase
	XREF	_CDFSBoot
	XREF	_DOSNode
	XREF	_FSProc
	XREF	_DebugLevel
	XREF	_InitGlobals
	XREF	_ValidDisk
	XREF	_NoReset
	XREF	_Main
	XREF	_BootMain
	XREF	_BootProc
	XREF	_BootPVDLSN
	XREF	_TMInfo
	XREF	_Supervisor
	XREF	_NoPrefs

	;------ ewhac/KMY (2414)
	XREF	_SimuSpeed

	;------ ewhac/KMY (2707)
	XREF	_InitFSE
	XREF	_QuitFSE
	XREF	_ReplyFSE
	XREF	_FSEMonitor
	XREF	_FSECollect


***
***	Versions and Revisions
***
VERREV		equ 	REVISION|(VERSION<<16)

***
***	Miscellaneous Constants
***
RTAG_PRI	equ	2
BOOT_PRI	equ	2		; V24.1
JSR_INST	equ	$4EB9
STACK_SIZE	equ	2048

***
***	File System Library Data Segment
***
LB_FLAGS	equ	LIB_SIZE
LB_DATASEG	equ	LB_FLAGS+2
LB_SIZEOF	equ	LB_DATASEG+4
GLOBAL_SIZE	equ	1024		; includes C vars as well

***
***	CLI Execution Protection
***
***	ewhac	9110.29	Modified to accept disk-based load and startup.
***
	XDEF	.begin
*.begin:		rts	; The OLD way

.begin:		bra	DiskStart

Debugging	dc.w	0

***
***	Resident Module Tag
***
	XREF	EndCode			; V24.1 moved
ResTag:		dc.w	RTC_MATCHWORD
		dc.l	ResTag
		dc.l	EndCode		; end skip
		dc.b	RTF_COLDSTART|RTF_AUTOINIT
		dc.b	VERSION		; version
		dc.b	NT_LIBRARY	; type
		dc.b	RTAG_PRI	; pri
		dc.l	LibName
		dc.l	ModIdent
		dc.l	InitRes		; structure below

***
***	Names and Strings
***
		dc.b	'CDTV File System',0
		COPYRIGHT
		dc.b	'Written by Carl Sassenrath, Ukiah, CA',0
		ds.w	0

	XDEF	_ModIdent
_ModIdent:	
ModIdent:	
		VSTRING			; V24.1 changed
		ds.w	0
LibName:	dc.b	'cdfs.library',0
		ds.w	0
ExpanName:	dc.b	'expansion.library',0
		ds.w	0
DOSName:	dc.b	'dos.library',0
		ds.w	0
	XDEF	_DevName
_DevName:
DevName:	dc.b	'cdtv.device',0
		ds.w	0
DOSDevName:	dc.b	'CD0',0
		ds.w	0
StopName:	dc.b	'***',0
		ds.w	0
BootProcName:	dc.b	'boot.task',0
		ds.w	0


InitRes:	dc.l	GLOBAL_SIZE
		dc.l	LibFuncs
		dc.l	0
		dc.l	InitModule

***
***	DOS Device Envrionment Table
***
		cnop	0,4		V24.1
DevEnvr:
		dc.l	DOSDevName
		dc.l	DevName
		dc.l	0
		dc.l	0
		;			DOS Envr Vector:
		dc.l	17		de_TableSize
		dc.l	2048>>2		de_SizeBlock
		dc.l	0		de_SecOrg
		dc.l	1		de_Surfaces
		dc.l	1		de_SectorPerBlock
		dc.l	1		de_BlocksPerTrack
		dc.l	0		de_Reserved
		dc.l	0		de_PreAlloc
		dc.l	0		de_Interleave
		dc.l	0		de_LowCyl
		dc.l	0		de_HighCyl
		dc.l	5		de_NumBuffers
		dc.l	1		de_BufMemType
		dc.l	$100000		de_MaxTransfer
		dc.l	$FFFFFFFE	de_Mask
		dc.l	BOOT_PRI	de_BootPri	V24.1
		dc.l	$43443031	de_DosType = "CD01"
StopEnvr:
		dc.l	StopName
		dc.l	DevName
		dc.l	0
		dc.l	0
		;			DOS Envr Vector:
		dc.l	17		de_TableSize
		dc.l	2048>>2		de_SizeBlock
		dc.l	0		de_SecOrg
		dc.l	1		de_Surfaces
		dc.l	1		de_SectorPerBlock
		dc.l	1		de_BlocksPerTrack
		dc.l	0		de_Reserved
		dc.l	0		de_PreAlloc
		dc.l	0		de_Interleave
		dc.l	0		de_LowCyl
		dc.l	0		de_HighCyl
		dc.l	5		de_NumBuffers
		dc.l	1		de_BufMemType
		dc.l	$100000		de_MaxTransfer
		dc.l	$FFFFFFFE	de_Mask
		dc.l	BOOT_PRI	de_BootPri	V24.1
		dc.l	$43443031	de_DosType = "CD01"

***
***	Dummy Config Dev Node
***
ConfDev:
		dc.l	0,0
		dc.b	0,0
		dc.l	0
		dc.b	0,0		; flags
		dc.b	ERTF_DIAGVALID,1,0,0  ; expansion ROM
		dc.w	0		; manufacturer
		dc.l	0		; serial num
		dc.w	0
		dc.l	CDDiagArea
		ds.l	10

***
***	Dummy Diag Area Structure & Boot code
***
CDDiagArea:
		dc.b	DAC_WORDWIDE|DAC_CONFIGTIME,0
		dc.w	0,0		; size & diagpoint
		dc.w	DiagArea_SIZEOF	; bootcode must follow diag area
		dc.w	0,0,0
		save	a6/a4
		bsr	BindLib		; bind to CDFS lib CES V22.3
		tst.l	_NoReset	; V24.2 check if reset ok
		blt.s	4$		; V24.2 no reset - other devs around
		move.l	#1,_NoReset	; V24.2 allow reset
4$:		lea	DOSName(pc),a1	; DOS init (must follow diag area)
		execl	FindResident
		move.l	d0,a0
		move.l	RT_INIT(a0),a0
		restore	a6/a4
		jsr	(a0)
		rts

fmt		dc.b	'<$%lx>',10,0
		ds.w	0


***		+----------------+
***		|                | <- Boot Seg Structure
***		+ - - - - - - - -+
***		|                | <- Actual Data Seg
***		|                |
***		|                | 
***		|                | <- A4 (Data Seg Pointer)
***		+----------------+

*** BootSeg Structure:		
BS_SEGLEN	equ	0	always zero
BS_NEXTSEG	equ	4	always zero
BS_JSRINST	equ	8	always $4EB9
BS_ENTRY	equ	10	entry point
BS_DATASEG	equ	14	data segment ptr (must follow ENTRY)
BS_SIZEOF	equ	18	data seg starts here

FUNCNT		set	0
LIBFUN		macro
		dc.l	\1
FUNCNT		set	FUNCNT+1
		endm

LibFuncs
		LIBFUN	Open		; -6
		LIBFUN	Close		; -12
		LIBFUN	Expunge		; -18
		LIBFUN	Reserved	; -24
		LIBFUN	ResetCDTV	; -30
		LIBFUN	SetDebug	; -36
		LIBFUN	ValidDisk	; -42
		LIBFUN	MountFS		; -48
		LIBFUN	GetTMInfo	; -54
		LIBFUN	IsNoPrefs	; -60
		LIBFUN	SetSpeed	; -66 (ewhac 9111.20)
		LIBFUN	InitFSE		; -72 (ewhac/KMY (2707))
		LIBFUN	QuitFSE		; -78 (ewhac/KMY (2707))
		LIBFUN	ReplyFSE	; -84 (ewhac/KMY (2707))
		LIBFUN	SetFSEMonitor	; -90 (ewhac/KMY (2707/2721))
		LIBFUN	StartFSE	; -96 (KMY (2710))
		LIBFUN	StopFSE		; -102 (KMY (2710))
	IFNE FUNCNT&1
		LIBFUN	DOSSucks	; Library Base must be aligned!
	ENDC
		DC.L	-1

************************************************************************
***
***	Resident Module Initialization
***
************************************************************************
InitModule:
		movem.l	d0-d7/a0-a6,-(sp)
		move.l	d0,a6		; library base pointer

	;------	Setup library structure:
		move.b	#NT_LIBRARY,LN_TYPE(a6)
		lea	LibName(pc),a0
		move.l	a0,LN_NAME(a6)
		lea	ModIdent(pc),a0
		move.l	a0,LIB_IDSTRING(a6)
		move.b	#LIBF_SUMUSED|LIBF_CHANGED,LIB_FLAGS(a6)
		move.w	#VERSION,LIB_VERSION(a6)
		move.w	#REVISION,LIB_REVISION(a6)

	;------	Set data segment register:
 		move.l	a6,a4
		add	#LB_SIZEOF,a4
		move.l	a4,d4		; save for later
		add.l	#32766+BS_SIZEOF,a4 ; Manx backwords segment
		move.l	a4,LB_DATASEG(a6)
		move.l	a6,_LibBase

		clear	d0
		move.w	Debugging(pc),d0
		move.l	d0,_DebugLevel
		move.l	#$12344321,_FSProc  ; temp test pattern

	;------	Open Expansion library:
		lea.l	ExpanName(pc),a1
		moveq.l	#33,d0		; 1.2 KS or better
		execl	OpenLibrary
		move.l	d0,_ExpBase	; CES 22.0
		beq	exit
		movea.l	d0,a6		for exp lib

	;------	Create new DOS device node:
		lea	DevEnvr(pc),a0  where parameter packet goes
		jsr	_LVOMakeDosNode(a6)	make the node
		move.l	d0,_DOSNode	did we get it?
		beq	exit
		move.l	d0,a3

	;------	Set up boot segment:
		move.l	d4,a1
		move.w	#JSR_INST,BS_JSRINST(a1) ; store a JSR.L instr
		lea	MainEntry(pc),a0	 ; get entry address
		move.l	a0,BS_ENTRY(a1)		 ; store as JSR address
		move.l	a4,BS_DATASEG(a1)

	;------	Set up rest of device node:
		move.l	a1,d0		; !!! or +4?
		lsr.l	#2,d0		; convert to BPTR
		move.l	d0,dn_SegList(a3)
		move.l	#-1,dn_GlobalVec(a3)
		move.l	#STACK_SIZE,dn_StackSize(a3)

	;------	Setup our Boot Node:
		lea	_CDFSBoot,a1
		move.b	#NT_BOOTNODE,LN_TYPE(a1)
		move.b	#BOOT_PRI,LN_PRI(a1)
		lea	ConfDev(pc),a0
		move.l	a0,LN_NAME(a1)
		move.l	a3,bn_DeviceNode(a1)

	;------	Initialize globals:
		move.l	SysBase,a6
		jsr	_InitGlobals

;		move.l	#$20000,d0
;2$:		move.w	#$64a,$DFF180	; set color
;		subq.l	#1,d0
;		bgt	2$

exit:
		movem.l	(sp)+,d0-d7/a0-a6
		rts

************************************************************************
***
***  Open Library
***
************************************************************************
Open:
		bclr.b	#LIBB_DELEXP,LIB_FLAGS(a6)
		addq.w	#1,LIB_OPENCNT(a6)
		move.l	a6,d0
		rts

************************************************************************
***
***  Close Library
***
************************************************************************
Close:
		subq.w	#1,LIB_OPENCNT(a6)
Expunge:
Reserved:
DOSSucks:
		clear	d0		; prevent expunge
		rts

************************************************************************
***
***  SetDebug
***
***	D0 == new debug level
***	Return old level.
***
************************************************************************
SetDebug:
		save	a4
		move.l	LB_DATASEG(a6),a4
		move.l	_DebugLevel,d1
		move.l	d0,_DebugLevel
		move.l	d1,d0
		restore	a4
		rts

************************************************************************
***
***  ValidDisk
***
***	Returns TRUE if disk is valid CDFS boot disk.
***
************************************************************************
ValidDisk:
		save	d2-d3/a2-a6
		move.l	LB_DATASEG(a6),a4
		move.l	SysBase,a6
		jsr	_ValidDisk
		restore	d2-d3/a2-a6
		rts

************************************************************************
***
***  GetTMInfo
***
***	Get Trademark Info pointer.  If zero, then no info found.
***
************************************************************************
GetTMInfo:
		save	d2-d3/a2-a6
		move.l	LB_DATASEG(a6),a4
		lea	_TMInfo,a0
		move.l	a0,d0
		restore	d2-d3/a2-a6
		rts

************************************************************************
***
***  IsNoPrefs
***
***	Return TRUE if no prefs should be done on startup.
***
************************************************************************
IsNoPrefs:
		save	a4
		move.l	LB_DATASEG(a6),a4
		clear	d0
		move.b	_NoPrefs,d0
		restore	a4
		rts

************************************************************************
***
***  SetSpeed
***
***	Enable/disable 150K/sec (or whatever) speed simulation.
***
***	ewhac/KMY (2414)
***
************************************************************************
SetSpeed:
		save	a4
		move.l	LB_DATASEG(a6),a4
		move.l	d0,_SimuSpeed
		restore	a4
		rts

************************************************************************
***
***  InitFSE(Mem:A0,Length:D0,Monitor:D1)
***
***	Attempt to procure the FileSystem Event reporting system.
***	Provide memory space, and its length, to FSE for storing the
***	messages.  Monitor has the bits set according to what events
***	are to be monitored.
***
***	ewhac/KMY (2707)
***	KMY (2713/2721)
***
************************************************************************
InitFSE:
		save	d2/d3/a4

		move.l	LB_DATASEG(a6),a4
		push	d1		; FSEMonitor
		push	d0		; Memory Length
		push	a0		; Memory
		jsr	_InitFSE
		add.l	#12,sp		; Drop parameters

		restore	d2/d3/a4
		rts

************************************************************************
***
***  QuitFSE()
***
***	Release the FileSystem Event reporting system.
***
***	ewhac/KMY (2707)
***
************************************************************************
QuitFSE:
		save	d2/d3/a4

		move.l	LB_DATASEG(a6),a4
		jsr	_QuitFSE

		restore	d2/d3/a4
		rts

************************************************************************
***
***  ReplyFSE(msg:A0)
***
***	Return a FileSystem Event.
***
***	ewhac/KMY (2707)
***
************************************************************************
ReplyFSE:
		save	d2/d3/a4

		move.l	LB_DATASEG(a6),a4
		push	a0		; msg
		jsr	_ReplyFSE
		addq.w	#4,sp		; Drop parameter

		restore	d2/d3/a4
		rts

************************************************************************
***
***  SetFSEMonitor
***
***	Set Monitor bits for FileSystem Event reporting.
***
***	ewhac/KMY (2707/2721)
***
************************************************************************
SetFSEMonitor:
		save	a4

		move.l	LB_DATASEG(a6),a4
		move.l	d0,_FSEMonitor

		restore	a4
		rts

************************************************************************
***
***  StartFSE
***
***	Allow FSE monitoring to start
***
***	KMY (2710)
***
************************************************************************
StartFSE:
		save	a4

		move.l	LB_DATASEG(a6),a4
		move.b	#1,_FSECollect

		restore	a4
		rts

************************************************************************
***
***  StopFSE
***
***	Stop FSE monitoring
***
***	KMY (2710)
***
************************************************************************
StopFSE:
		save	a4

		move.l	LB_DATASEG(a6),a4
		move.b	#0,_FSECollect

		restore	a4
		rts

************************************************************************
***
***  MountFS	CES V22.1 V24.2 (all new)
***
***	Add DOS device node for cases where we are not booting from CD.
***
************************************************************************
MountFS:
		save	d2/a2-a6
		move.l	LB_DATASEG(a6),a4
		exec	Forbid

		move.l	_ExpBase,a0
		lea	eb_MountList(a0),a0

	;------	If other volumes around, don't allow reset:
		move.l	(a0),a1		; check mount list
		tst.l	(a1)		; is it empty?
		beq.s	2$		; yes
		move.l	#-1,_NoReset	; no reset
2$:
	;------	Did we find a bootable volume?
		tst.l	_BootPVDLSN
		beq.s	4$

	;------	Mount the bootable volume:
		lea	_CDFSBoot,a1
		tst.l	(a1)		; already mounted?
		bne.s	9$		; yes
		exec	Enqueue		; a0/a1 (pri already set)
		bra.s	9$

	;------	Nothing to mount:
4$:		moveq.l	#-128,d0	; low priority V24.2
		moveq.l	#1,d1		; fire up handler
		move.l	_DOSNode,a0	; node to use
		move.l	_ExpBase,a6
		jsr	_LVOAddDosNode(a6)

9$:		exec	Permit
		restore	d2/a2-a6
		rts

************************************************************************
***
***	CDFS Main Entry Point
***
***		Called by AmigaDOS during bootstrap via the dummy
***		segment put into the device node segment list.
***
************************************************************************
MainEntry:
		move.l	(sp)+,a0
		move.l	(a0),a4		; initial data segment pointer

		move.l	SysBase,a6

	;------	Open DOS library:
		lea	DOSName(pc),a1
		clear	d0
		exec	OpenLibrary
		move.l	d0,_DOSBase	; MUST OPEN !!!

		jmp	_Main

************************************************************************
***
***  Crash
***
***	Cannot recover from our problems.
***
************************************************************************
Crash:
		move.l	#$20000,d0
2$:		move.w	#$a00,$DFF180	; set color
		subq.l	#1,d0
		bgt	2$

		rts			; where does it go???

************************************************************************
***
***  StopBootProcess
***
***	Setups up a low priority DOS device node that when executed
***	will terminate the DOS boot process.  This code must be
***	executed prior to DOS booting.
***
***	I hope we can find a better way of doing this before we ship.
***
************************************************************************
	XDEF	_StopBootProcess
_StopBootProcess:
	;------	Create stop-boot device node:
		lea	StopEnvr(pc),a0 ; where parameter packet goes
		move.l	_ExpBase,a6
		jsr	_LVOMakeDosNode(a6)
		move.l	d0,a0

	;------	Set up rest of device node:
		lea	StopDOS(pc),a1
		move.l	a1,d0
		lsr.l	#2,d0		; convert to BPTR
		move.l	d0,dn_SegList(a0)
		move.l	#-1,dn_GlobalVec(a0)
	;	move.l	#4096,dn_StackSize(a0)

	;------	Add node to DOS device list:
		moveq.l	#1,d1		start-proc (now?!!!)
		moveq.l	#-100,d0	boot priority
		jsr	_LVOAddDosNode(a6)  ; d1,d0,a0

		move.l	SysBase,a6
		rts

	;------	Called as last node in boot:
		cnop	0,4
		dc.l	0
StopDOS		dc.l	0		; for DOS
		save	a4/a6

		bsr	BindLib		; (leaves A6->Sysbase)

;		public	_FindTask,_Debug0
;		pea	0
;		bsr	_FindTask
;		addq.l	#4,sp
;		move.l	d0,a0
;		move.l	LN_NAME(a0),-(sp)
;		pea	pr
;		bsr	_Debug0
;		addq.l	#8,sp

	;------	Signal our new Boot Proc and stop current proc:
		move.l	_BootProc,d0
		beq.s	3$
		move.l	d0,a1
		sub.l	#TC_SIZE,a1	; convert to task ptr
		moveq.l	#1,d0		; wake boot proc signal
		exec	Signal

		move.l	#$20000,d0
4$:		move.w	#$a00,$DFF180	; set color
		subq.l	#1,d0
		bgt	4$

	;	public	_BootFile
	;	tst.b	_BootFile
	;	beq.s	3$
	;	jsr	_BootMain	; C entry point

3$:		clear	d0
		exec	Wait		; wait forever.

		restore	a4/a6
		clear	d0
		rts

************************************************************************
***
***  BootFileSeg
***
***	Entry code for a new process for booting up a Boot File.
***	This code only gets executed when a boot file is specified
***	as part of a PVD.  It is run after strap as part of the FS
***	startup.
***
************************************************************************
	XDEF	_BootFileSeg
		cnop	0,4		; for DOS
		dc.l	0
_BootFileSeg:	dc.l	0
	;------	Get back to CDFS:
		bsr	BindLib

	;------	Enter our C boot code:
		jsr	_BootMain	; C entry point
		clear	d0		; should not return.
		move.l	SysBase,a6
		exec	Wait		; wait forever.

************************************************************************
***
***  BindLib
***
***	Bind a "task" to the CDFS main data segment (in A4).
***
************************************************************************
BindLib:
		lea.l	LibName(pc),a1	; CDFS library
		clear	d0
		move.l	SysBase,a6
		exec	OpenLibrary
		move.l	d0,a0		; must open.
		move.l	LB_DATASEG(a0),a4
		move.l	SysBase,a6
		rts


		cnop	0,4
	XDEF	ResetAmiga
ResetAmiga:				; V22.4
		lea.l	2,a0
		reset
		jmp	(a0)

ResetCDTV:
		move.l	SysBase,a6
		lea	ResetAmiga(pc),a5
		bsr	_Supervisor	; does not return

************************************************************************
***
***  DiskStart
***
***	Code to be executed when started either as a disk-loadable DOS
***	handler, or when someone tries to run it.
***	KMY 2415 - removed message printing when run from CLI - just
***	return -- no need to bite off users head.
***
************************************************************************

DiskStart:	move.l	(4).w,a5		; KMY (2415) (was a copy of a6)
		sub.l	a1,a1
		execl	FindTask
		move.l	d0,a0			; Current task in A5
		move.l	pr_CLI(a0),d0
		bne	CLIStart

* Not a CLI start.  This means either a WorkBench startup, or a DOS mount.
* Currently, we don't defend against WB startups.
*
init_diskstart:
	;------	Make cdfs.library
		lea	LibFuncs(pc),a0
		sub.l	a1,a1
		sub.l	a2,a2
		move.l	#GLOBAL_SIZE,d0
		moveq	#0,d1
		execl	MakeLibrary

		move.l	d0,a5
		tst.l	d0
		beq	Crash

	;------	Setup library structure:
		move.l	a5,a6
		move.b	#NT_LIBRARY,LN_TYPE(a6)
		lea	LibName(pc),a0
		move.l	a0,LN_NAME(a6)
		lea	ModIdent(pc),a0
		move.l	a0,LIB_IDSTRING(a6)
		move.b	#LIBF_SUMUSED!LIBF_CHANGED,LIB_FLAGS(a6)
		move.w	#VERSION,LIB_VERSION(a6)
		move.w	#REVISION,LIB_REVISION(a6)

	;------	Set data segment register:
		move.l	a6,a4
		add	#LB_SIZEOF,a4
		move.l	a4,d4			; save for later
		add.l	#32766+BS_SIZEOF,a4 ; Manx backwords segment
		move.l	a4,LB_DATASEG(a6)
		move.l	a6,_LibBase

		clear	d0
		move.w	Debugging(pc),d0
		move.l	d0,_DebugLevel
		move.l	#$12344321,_FSProc	; temp test pattern

	;------ Open expansion.library
		lea.l	ExpanName(pc),a1
		moveq.l	#33,d0			; 1.2 KS or better
		execl	OpenLibrary
		move.l	d0,_ExpBase		; CES 22.0

	;------ Initialize globals
		jsr	_InitGlobals

	;------	Add library to system.
		move.l	a5,a1			; We saved a copy, remember?
		execl	AddLibrary

	;------	Proceed with rest of setup.
		lea	DOSName(pc),a1
		clear	d0
		exec	OpenLibrary
		move.l	d0,_DOSBase		; MUST OPEN !!!

		jmp	_Main

	;------ There was a CLI structure attached, indicating a command
	;------ line invocation.  Return with code
CLIStart:
		clear	d0
		rts				; Return to user.

		END
