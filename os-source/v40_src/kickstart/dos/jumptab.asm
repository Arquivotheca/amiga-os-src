*
*  This section of code provides a library interface for DOS
*  commands.
*
*  It works by using a global vector held within the
*  process control space and by jumping indirect through this.
*  Some global numbers are negative; others map to existing positive
*  ones.
*
*  Version 1.0 March 11 1985   TJK
*
*

BLIB	MACRO	;\1 - symbolname
	XREF	_\1
@\1	EQU	_\1
	ENDM

	BLIB	findstream
	BLIB	endstream
	BLIB	read
	BLIB	write
	BLIB	seek
	BLIB	deletefile
	BLIB	renameobj		; Rename
	BLIB	lock
	BLIB	freeobj
	BLIB	copydir
	BLIB	examine
	BLIB	exnext
	BLIB	info
	BLIB	createdir
	BLIB	CurrentDir
	BLIB	loadseg
	BLIB	unloadseg
*	BLIB	clearvec		; dos_lib.fd says WaitPkt
	BLIB	noreqloadseg		; dos_lib.gd says QueuePkt
	BLIB	deviceproc		; DeviceProc
	BLIB	setcomment
	BLIB	setprotection
	BLIB	datstamp 
	BLIB	delay   
	BLIB	waitforchar
	BLIB	parent
	BLIB	isinteractive
	BLIB	execute		; last def in 1.0-1.3

	BLIB	ReadArgs
	BLIB	FindArg
	BLIB	ReadItem
	BLIB	FreeArgs
	BLIB	Fault
	BLIB	fault
	BLIB	sendpacket
	BLIB	setdate
	BLIB	assign
	BLIB	FGetC
	BLIB	UnGetC
	BLIB	FPutC
	BLIB	relabel
	BLIB	addbuffers
	BLIB	inhibit
	BLIB	setmode
	BLIB	SetFileSize
	BLIB	adddevice
	BLIB	ErrorReport
	BLIB	makeseg
	XREF	@mysplitname
	BLIB	Flush
	BLIB	SetVBuf
	BLIB	NameFromLock
	BLIB	NameFromFH
	BLIB	assignlate
	BLIB	assignstring
	BLIB	addassign
	BLIB	FWrite
	BLIB	FRead
	BLIB	SameLock
	BLIB	FPuts
	BLIB	FGets
	BLIB	StartNotify
	BLIB	EndNotify
	BLIB	getdevproc
	BLIB	freedevproc
	BLIB	LockDosList
	BLIB	UnLockDosList
	BLIB	AttemptLockDosList
	BLIB	RemDosEntry
	BLIB	AddDosEntry
	BLIB	FindDosEntry
	BLIB	NextDosEntry
	BLIB	MakeDosEntry
	BLIB	FreeDosEntry
	BLIB	ReadLink
	BLIB	MakeLink
	BLIB	FindCli
	BLIB	OpenFromLock
	BLIB	DupLockFromFH
	BLIB	ParentOfFH
	BLIB	ExamineFH
	BLIB	LockRecord
	BLIB	UnLockRecord
	BLIB	SetCurrentDirName
	BLIB	GetCurrentDirName
	BLIB	SetPrompt
	BLIB	GetPrompt
	BLIB	SetProgramName
	BLIB	GetProgramName
	XREF	@MaxCli
	BLIB	GetVar
	BLIB	SetVar
	BLIB	DeleteVar
	BLIB	DateToStr
	BLIB	StrToDate
	BLIB	remaddassign
	BLIB	isfilesystem
	BLIB	changemode
	BLIB	system
	BLIB	CreateNewProc
	BLIB	FindVar
	BLIB	FilePart
	BLIB	PathPart
	BLIB	AddPart
	BLIB	LockRecords
	BLIB	UnLockRecords
	BLIB	VFWritef
	BLIB	VFPrintf
	BLIB	searchsegs
	BLIB	RemSegment
	BLIB	WriteChars
	BLIB	PutStr
	BLIB	VPrintf
	BLIB	AllocDosObject
	BLIB	FreeDosObject
	BLIB	FunkyMatchFirst
	BLIB	Format

	XREF	@SendPkt
*	XREF	@multiply
*	XREF	@divide
*	XREF	@rem
	XREF	@cli
	XREF	@consoletask
	XREF	@setconsole
	XREF	@filesystemtask
	XREF	@setfilesys
	XREF	@SetIoErr
	XREF	@taskwait
	XREF	@returnpkt
	XREF	@selectinput
	XREF	@selectoutput
	XREF	@requester2
	BLIB	RunCommand
	XREF	@InternalLoadSeg
	XREF	@InternalUnLoadSeg
	XREF	@input
	XREF	@output
	XREF	@getresult2
	XREF	doslib_createproc
	XREF	@exit
	XREF	@CompareDates
	XREF	@cli_init
	XREF	@cli_init_run
	XREF	@cli_init_newcli
	XREF	@Atol
	XREF	@CheckSignal
	XREF	@ExAll
	XREF	@ExAllEnd
	XREF	@GetProgramDir
	XREF	@SetProgramDir
	XREF	@ArgStr
	XREF	@SetArgStr
	BLIB	mygetstring
	XREF	@SetOwner

	XREF	FindNext
	XREF	FreeAnchorChain
	XREF	PreParse
	BLIB	MatchPattern
	XREF	PreParseNoCase
	BLIB	MatchPatternNoCase
	BLIB	SameDevice

	XREF	J_OPEN
	XREF	J_CLOSE
	XREF	J_NOP

	XDEF	JTAB

*
* readlink takes params in different regs
*
myreadlink:
	move.l	d4,d0
	move.l	d5,a0
	bra	@ReadLink
*
*
* kludge Open to put result in d0 and d1 - stupid arp/etc.
* REMOVE for 2.1!!!!! FIX!!!!
*
open_kludge:
	bsr	@findstream
	move.l	d0,d1
	rts
*
* kludge for DateStamp for WP 4.1 - they expect return in D1!
*
mydatestamp:
	bsr	@datstamp
	move.l	d0,d1
	rts
*
*
* This is the main library structure. First the jump table.
*

*
* Build table of relative vectors

MCCASM	    EQU     1

CMDDEF	    MACRO   * function
    IFEQ MCCASM
	    DC.W    \1-JTAB
    ENDC
    IFNE MCCASM
	    DC.W    \1+(*-JTAB)
    ENDC
	    ENDM

JTAB
		DC.W	-1			; word table
		CMDDEF	J_OPEN			; mandatory library entry points
		CMDDEF	J_CLOSE
		CMDDEF	J_NOP
		CMDDEF	J_NOP
		CMDDEF	open_kludge		; FIX!!!!
		CMDDEF	@endstream
		CMDDEF	@read
		CMDDEF	@write
		CMDDEF	@input
		CMDDEF	@output
		CMDDEF	@seek
		CMDDEF	@deletefile
		CMDDEF	@renameobj	;Rename
		CMDDEF	@lock
		CMDDEF	@freeobj
		CMDDEF	@copydir
		CMDDEF	@examine
		CMDDEF	@exnext
		CMDDEF	@info
		CMDDEF	@createdir
		CMDDEF	@CurrentDir
		CMDDEF	@getresult2	; IoErr
		CMDDEF	doslib_createproc	; CAREFUL!
		CMDDEF	@exit		; Exit()
		CMDDEF	@loadseg
		CMDDEF	@unloadseg
					; the next two are marked private!
		CMDDEF	J_NOP		;was clearvec! dos_lib.fd says WaitPkt
		CMDDEF	@noreqloadseg		; dos_lib.fd says QueuePkt
		CMDDEF	@deviceproc	; DeviceProc
		CMDDEF	@setcomment
		CMDDEF	@setprotection
		CMDDEF	mydatestamp	; @DateStamp
		CMDDEF	@delay   
		CMDDEF	@waitforchar
		CMDDEF	@parent
		CMDDEF	@isinteractive
		CMDDEF	@execute		; last def in 1.0-1.3

************************************************************************
************************************************************************
***             Object creation/deletion                            ****
************************************************************************
************************************************************************
		CMDDEF	@AllocDosObject
		CMDDEF	@FreeDosObject
************************************************************************
************************************************************************
***             Packet Level routines                               ****
************************************************************************
************************************************************************
		CMDDEF	@sendpacket	; @DoPkt
		CMDDEF	@SendPkt
		CMDDEF	@taskwait	; @WaitPkt
		CMDDEF	@returnpkt	; @ReplyPkt
		CMDDEF	J_NOP		; @AbortPkt
************************************************************************
************************************************************************
***             Record Locking                                      ****
************************************************************************
************************************************************************
		CMDDEF	@LockRecord
		CMDDEF	@LockRecords
		CMDDEF	@UnLockRecord
		CMDDEF	@UnLockRecords
************************************************************************
************************************************************************
***             Buffered File I/O                                   ****
************************************************************************
************************************************************************
		CMDDEF	@selectinput	; @SelectInput
		CMDDEF	@selectoutput	; @SelectOutput
		CMDDEF	@FGetC
		CMDDEF	@FPutC
		CMDDEF	@UnGetC
		CMDDEF	@FRead
		CMDDEF	@FWrite
		CMDDEF	@FGets
		CMDDEF	@FPuts
		CMDDEF	@VFWritef
		CMDDEF	@VFPrintf
		CMDDEF	@Flush
		CMDDEF	@SetVBuf
************************************************************************
************************************************************************
***             Object Management                                   ****
************************************************************************
************************************************************************
		CMDDEF	@DupLockFromFH
		CMDDEF	@OpenFromLock
		CMDDEF	@ParentOfFH
		CMDDEF	@ExamineFH
		CMDDEF	@setdate	; @SetFileDate
		CMDDEF	@NameFromLock
		CMDDEF	@NameFromFH
		CMDDEF	@mysplitname	; @SplitName
		CMDDEF	@SameLock
		CMDDEF	@setmode	; @SetMode
		CMDDEF	@ExAll
		CMDDEF  myreadlink	; @ReadLink
		CMDDEF  @MakeLink
		CMDDEF	@changemode	; @ChangeMode
		CMDDEF	@SetFileSize
************************************************************************
************************************************************************
***             Error Handling                                      ****
************************************************************************
************************************************************************
		CMDDEF	@SetIoErr
		CMDDEF	@Fault
		CMDDEF	@fault		; @PrintFault
		CMDDEF	@ErrorReport
		CMDDEF	@requester2	; @Requester - NO LONGER PUBLIC!
************************************************************************
************************************************************************
***             Process Management                                  ****
************************************************************************
************************************************************************
		CMDDEF	@cli		; @Cli
		CMDDEF	@CreateNewProc
		CMDDEF	@RunCommand
		CMDDEF	@consoletask	; @GetConsoleTask
		CMDDEF  @setconsole	; @SetConsoleTask
		CMDDEF	@filesystemtask ; @GetFileSysTask
		CMDDEF  @setfilesys	; @SetFileSysTask
		CMDDEF  @ArgStr		; @GetArgStr
		CMDDEF	@SetArgStr
		CMDDEF	@FindCli	; @FindCliProc
		CMDDEF	@MaxCli
		CMDDEF	@SetCurrentDirName
		CMDDEF	@GetCurrentDirName
		CMDDEF	@SetProgramName
		CMDDEF	@GetProgramName
		CMDDEF	@SetPrompt
		CMDDEF	@GetPrompt
		CMDDEF	@SetProgramDir
		CMDDEF	@GetProgramDir
		CMDDEF	@system		; @System
************************************************************************
************************************************************************
***             Device List Management                              ****
************************************************************************
************************************************************************
		CMDDEF	@assign		; @AssignLock
		CMDDEF	@assignlate	; @AssignLate
		CMDDEF	@assignstring	; @AssignPath
		CMDDEF	@addassign	; @AssignAdd
		CMDDEF  @remaddassign	; @RemAssignList
		CMDDEF	@getdevproc	; @GetDeviceProc
		CMDDEF	@freedevproc	; @FreeDeviceProc
		CMDDEF	@LockDosList
		CMDDEF	@UnLockDosList
		CMDDEF	@AttemptLockDosList
		CMDDEF	@RemDosEntry
		CMDDEF	@AddDosEntry
		CMDDEF	@FindDosEntry
		CMDDEF	@NextDosEntry
		CMDDEF	@MakeDosEntry
		CMDDEF	@FreeDosEntry
		CMDDEF	@isfilesystem	; @IsFileSystem
************************************************************************
************************************************************************
***             Handler Interface                                   ****
************************************************************************
************************************************************************
		CMDDEF	@Format
		CMDDEF	@relabel	; @Relabel
		CMDDEF	@inhibit	; @Inhibit
		CMDDEF	@addbuffers	; @AddBuffers
************************************************************************
************************************************************************
***             Date Time Routines                                  ****
************************************************************************
************************************************************************
		CMDDEF	@CompareDates
		CMDDEF  @DateToStr
		CMDDEF  @StrToDate
************************************************************************
************************************************************************
***             Image Management                                    ****
************************************************************************
************************************************************************
		CMDDEF	@InternalLoadSeg
		CMDDEF	@InternalUnLoadSeg
		CMDDEF	@loadseg	; @NewLoadSeg	/* FIX! */
		CMDDEF	@makeseg	; @AddSegment
		CMDDEF	@searchsegs	; @FindSegment
		CMDDEF	@RemSegment
************************************************************************
************************************************************************
***             Command Support                                     ****
************************************************************************
************************************************************************
		CMDDEF	@CheckSignal
		CMDDEF	@ReadArgs
		CMDDEF	@FindArg
		CMDDEF	@ReadItem
		CMDDEF	@Atol		; @StrToLong
		CMDDEF	@FunkyMatchFirst	; @MatchFirst
		CMDDEF	FindNext	; @MatchNext
		CMDDEF	FreeAnchorChain	; @MatchEnd
		CMDDEF	PreParse	; @ParsePattern
		CMDDEF	@MatchPattern
		CMDDEF	J_NOP		; was @AllocVecType
		CMDDEF	@FreeArgs	; was @AllocVecType!!!
		CMDDEF	J_NOP		; was @FreeVec
		CMDDEF  @FilePart
		CMDDEF  @PathPart
		CMDDEF  @AddPart
************************************************************************
************************************************************************
***             Notification                                        ****
************************************************************************
************************************************************************
		CMDDEF	@StartNotify
		CMDDEF	@EndNotify
************************************************************************
************************************************************************
***             Environment Variable functions                      ****
************************************************************************
************************************************************************
		CMDDEF	@SetVar
		CMDDEF	@GetVar
		CMDDEF  @DeleteVar
		CMDDEF  @FindVar

*
* Private entries for Shell/Cli
*
		CMDDEF  @cli_init
		CMDDEF  @cli_init_newcli
		CMDDEF  @cli_init_run
*
		CMDDEF  @WriteChars
		CMDDEF  @PutStr
		CMDDEF  @VPrintf
		CMDDEF  J_NOP		; @MatchReplace FIX!

* these were nop's in v36 up to ks 36.303
		CMDDEF  PreParseNoCase	; @ParsePatternNoCase
		CMDDEF  @MatchPatternNoCase
		CMDDEF  @mygetstring
		CMDDEF  @SameDevice
*
		CMDDEF  @ExAllEnd	; added after ks 36.303
		CMDDEF  @SetOwner	; added after ks 39.71
		CMDDEF  J_NOP
		CMDDEF  J_NOP
*
		CMDDEF  J_NOP		; added at first V37 build
		CMDDEF  J_NOP
*
		CMDDEF  J_NOP		; added after V37.23 build
		CMDDEF  J_NOP
*
		CMDDEF  J_NOP		; added after V39.71 build
		CMDDEF  J_NOP

		DC.W	-1

	END
