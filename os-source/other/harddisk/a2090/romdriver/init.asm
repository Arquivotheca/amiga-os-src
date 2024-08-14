*******************************************************************************
*
*	Source Control
*	--------------
*	$Header: init.asm,v 34.43 88/04/08 03:33:09 bart Exp $
*
*	$Locker: bart $
*
*	$Log:	init.asm,v $
*   Revision 34.43  88/04/08  03:33:09  bart
*   idle loop waiting for scsi's to spin up
*   
*   Revision 34.42  88/04/04  20:38:03  bart
*   decaying timeout in init scsi loop
*   
*   Revision 34.41  88/04/03  12:16:40  bart
*   preserve d6 (device pointer) from scsi init loop
*   
*   Revision 34.40  88/02/22  14:32:42  bart
*   checkpoint
*   
*   Revision 34.39  88/02/22  14:07:44  bart
*   *** empty log message ***
*   
*   Revision 34.38  88/02/22  13:49:34  bart
*   *** empty log message ***
*   
*   Revision 34.37  88/02/22  12:55:09  bart
*   if start OK...
*   
*   Revision 34.36  88/02/22  12:25:39  bart
*   don't assume no drive until start attempt fails
*   
*   Revision 34.35  88/02/19  11:43:56  bart
*   issue start unit command to scsi units after testing ready
*   
*   Revision 34.34  88/02/19  11:30:55  bart
*   INotSCSI: LEA   INIT_IOR(A2),A1     ; Point to IORequest
*   
*   Revision 34.33  88/02/18  10:24:58  bart
*   InitUnit preserve A3
*   
*   Revision 34.32  88/02/16  14:15:19  bart
*   fall into cmp_dosloop after adjusting count...
*   
*   Revision 34.31  88/02/16  11:30:08  bart
*   pass correct controller number and name pointer-pointer from Open() to
*   InitUnit...
*   
*   Revision 34.30  88/02/15  14:35:10  bart
*   loop in InitScsiLoop to init scsi units 0..6
*   
*   Revision 34.29  88/02/04  17:18:36  bart
*   check diagvec for validity (running from ram?)
*   
*   Revision 34.28  88/02/01  15:31:44  bart
*   unique device name when run via binddrivers, too!
*   
*   Revision 34.27  88/02/01  14:47:16  bart
*   prevent name conflict with already MOUNTED dos devices, too.
*   
*   Revision 34.26  88/02/01  10:43:12  bart
*   reserved commands 0-9 prior to scsi direct command
*   
*   Revision 34.25  88/01/21  18:05:10  bart
*   compatible with disk based / binddriver useage
*   
*   Revision 34.24  87/12/04  19:14:18  bart
*   checkpoint
*   
*   Revision 34.23  87/12/04  18:23:56  bart
*   check for unique dosname
*   
*   Revision 34.22  87/12/04  12:09:00  bart
*   checkpoint before adding check for existing dosname on eb_mountlist
*   
*   Revision 34.21  87/11/02  09:31:36  bart
*   INFO_Retry:     LEA INIT_CMD(A2),A1     ; Point to internal command blk
*   
*   Revision 34.20  87/10/27  11:02:22  bart
*   LEA.L   Rom_DiagArea(PC),A1
*   
*   Revision 34.19  87/10/27  09:00:24  bart
*   fixed bad sense of test of D7...
*   bart
*   
*   Revision 34.18  87/10/26  16:38:28  bart
*   *** empty log message ***
*   
*   Revision 34.17  87/10/26  16:31:37  bart
*   checkpoint
*   
*   Revision 34.16  87/10/14  15:35:41  bart
*   10-13 rev 1
*   
*   Revision 34.15  87/10/14  14:16:18  bart
*   beginning update to cbm-source.10.13.87
*   
*   Revision 34.14  87/07/08  14:01:14  bart
*   y
*   
*   Revision 34.13  87/06/18  13:48:45  bart
*   ; type NT_BOOTNODE indicates
*   ; that this DeviceNode is associated
*   ; with CD address in ln_Name field
*   
*   Revision 34.12  87/06/11  15:48:24  bart
*   working autoboot 06.11.87 bart
*   
*   Revision 34.11  87/06/11  14:42:18  bart
*   *** empty log message ***
*   
*   Revision 34.10  87/06/11  12:58:37  bart
*   OK, so sue me... i am going to use the currentbinding protocol after all
*   
*   Revision 34.9  87/06/10  18:54:04  bart
*   LEA.L   IntuitLibName(PC),A1    ; Get intuition lib. name
*   
*   Revision 34.8  87/06/10  17:53:27  bart
*   ; We are NOT going to use the kludgey currentbinding protocol
*   ; In fact, we have been passed the CD directly ** poof! **
*   
*   Revision 34.7  87/06/10  15:12:45  bart
*   *** empty log message ***
*   
*   Revision 34.6  87/06/10  12:50:23  bart
*   *** empty log message ***
*   
*   Revision 34.5  87/06/10  12:41:23  bart
*   pass CD in ln_Name field of BootNode to eb_MountList
*   
*   Revision 34.4  87/06/03  11:35:14  bart
*   *** empty log message ***
*   
*   Revision 34.3  87/06/03  10:59:20  bart
*   checkpoint
*   
*   Revision 34.2  87/05/31  16:36:05  bart
*   chickpoint
*   
*   Revision 34.1  87/05/29  19:39:41  bart
*   checkpoint
*   
*   Revision 34.0  87/05/29  17:40:04  bart
*   added to rcs for updating
*   
*
*******************************************************************************

	LLEN	130
	PLEN	60
	LIST

*************************************************************************
*									*
*	Copyright (C) 1986, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

	SECTION	section

******* Included Files ***********************************************

	NOLIST
	IFND	EXEC_TYPES_I
	INCLUDE 'exec/types.i'
	ENDC
	IFND	EXEC_INTERRUPTS_I
	INCLUDE	"exec/interrupts.i"
	ENDC
	IFND	EXEC_LISTS_I
	INCLUDE 'exec/lists.i'
	ENDC
	IFND	EXEC_NODES_I
	INCLUDE 'exec/nodes.i'
	ENDC
	IFND	EXEC_PORTS_I
	INCLUDE 'exec/ports.i'
	ENDC
	IFND	EXEC_LIBRARIES_I
	INCLUDE 'exec/libraries.i'
	ENDC
	IFND	EXEC_IO_I
	INCLUDE 'exec/io.i'
	ENDC
	IFND	EXEC_DEVICES_I
	INCLUDE 'exec/devices.i'
	ENDC
	IFND	EXEC_TASKS_I
	INCLUDE 'exec/tasks.i'
	ENDC
	IFND	EXEC_MEMORY_I
	INCLUDE 'exec/memory.i'
	ENDC
	IFND	EXEC_EXECBASE_I
	INCLUDE 'exec/execbase.i'
	ENDC
	IFND	EXEC_ABLES_I
	INCLUDE 'exec/ables.i'
	ENDC
	IFND	EXEC_ERRORS_I
	INCLUDE 'exec/errors.i'
	ENDC
	IFND	EXEC_ALERTS_I
	INCLUDE 'exec/alerts.i'
	ENDC
	IFND	LIBRARIES_FILEHANDLER_I
	INCLUDE 'libraries/filehandler.i'
	ENDC
	IFND    LIBRARIES_DOSEXTENS_I
	INCLUDE 'libraries/dosextens.i'
	ENDC
	INCLUDE 'hddisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE	'libraries/expansion.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'
	INCLUDE 'bootrom.i'
	LIST

******* Imported Globals *********************************************

	XREF	VERNUM
	XREF	REVNUM
	XREF	Expunge
	XREF	Open
	XREF	Close
	XREF	BeginIO
	XREF	PerformIO
	XREF	Null
	XREF	NoIO
	XREF	Soft_Error
	XREF	HDTaskStart
	XREF	hdName
	XREF	Rom_DiagArea
	XREF	hdintr			; Interrupt server routine
	XREF	HDUClear
	XREF	HDUMotor
	XREF	HDUSeek
	XREF	HDUUpdate
	XREF	HDUFormat
	XREF	HDURemove
	XREF	HDUChangeNum
	XREF	HDUChangeState
	XREF	HDUProtStatus
	XREF	HDUSpecial
	XREF    HDUSCSI
	XREF	HDUMovCmdBlk
	XREF	set_timeout
	XREF	get_timeout

******* Imported Functions *******************************************

	IFD	AUTOINST		; If Autoconfig software used
	EXTERN_LIB AddMemList		; Add MEMORY to free list
	EXTERN_LIB GetCurrentBinding	; Get list of boards for this driver
	EXTERN_LIB MakeDosNode		; Construct AmigaDOS structures
	EXTERN_LIB AddDosNode		; Tell AmigaDOS about unit partition
	ENDC
	EXTERN_LIB AddDevice
	EXTERN_LIB AddTask
	EXTERN_LIB Alert
	EXTERN_LIB AllocMem
	EXTERN_LIB AllocSignal
	EXTERN_LIB CloseDevice
	EXTERN_LIB CloseLibrary
	EXTERN_LIB Debug
	EXTERN_LIB Enqueue
	EXTERN_LIB FindName
	EXTERN_LIB FindTask
	EXTERN_LIB FreeMem
	EXTERN_LIB InitStruct
	EXTERN_LIB MakeLibrary
	EXTERN_LIB OpenDevice
	EXTERN_LIB OpenLibrary
	EXTERN_LIB PutMsg
	EXTERN_LIB RemDevice
	EXTERN_LIB ReplyMsg
	EXTERN_LIB Signal
	EXTERN_LIB Wait

	XREF	HDIO
	IFD	AUTOINST
	XREF	ExLibName
	ENDC
	XREF	IntuitLibName

	INT_ABLES
	TASK_ABLES

******* Exported Functions *******************************************

	XDEF	Get_INFO	; Extract info from boot sector
	XDEF	Init
	XDEF	InitUnit
	XDEF	cmdTable
	XDEF	open_vect
	XDEF	close_vect
	XDEF	expunge_vect
	XDEF	reserved_vect
	XDEF	beginio_vect
	XDEF	abortio_vect
	XDEF	dev_Name1
	XDEF	dev_Name2
	XDEF	dev_Name3
	XDEF	dev_Name4
	XDEF	dev_ICode

***** Local Definitions **********************************************

***** Let the Code Begin *********************************************

	XDEF	SA_Driver
SA_Driver:

******* Device/HDDisc/Initialize **********************************
*
*   NAME
*	Initialize -- initialize the device from nothing
*
*   SYNOPSIS
*	Error = Initialize(DeviceName), SysLib
*	D0		   A1		A6
*
*   FUNCTION
*	This routine will initialize the device after it is closed.
*	The routine is optional; it is intended to make restoring
*	devices easier if they need to be purged "temporarily".
*
*	THIS ENTRY POINT MUST REMAIN VALID FOR ALL TIME (since it
*	is the only part of the device entry points that others are
*	allowed to cache).  It makes sense mostly for ROM based
*	drivers.  It MAY NOT be part of cartridge ROM (since cartridges
*	may be pulled).
*
*
*
*   INPUTS
*	DeviceName -- the name of the device to be loaded.  It is
*	    explicitly given because the init vector may point to
*	    a common routine, or to a search routine for disc libraries.
*
*
*   RESULTS
*	Error -- if the Init succedded, then Error will be null.  If
*	    it failed then it will be non-zero.
*
*
*   SEE ALSO
*
*
**********************************************************************
*
*
*   REGISTER USAGE
*
*
*   IMPLEMENTATION
*	Remember that Init is run with SysLib in A6, unlike most of
*	the rest of the driver, which has the Device structure in A6
*
*

*	D3 is used to keep track of which controller is being init
*	D6 holds the actual Device (DV) structure address

Init:
		MOVEM.L	D2-D7/A2-A5,-(SP)

*		;------ get temp memory
		MOVE.L	#INIT_SIZE,D0
		MOVE.L	#MEMF_CLEAR!MEMF_PUBLIC,D1
		CALLSYS	AllocMem
		TST.L	D0
		BNE.S	Alloc_Success

		ALERT	(AN_AMHDDiskDev!AG_NoMemory),,A0

Alloc_Success:	MOVE.L	D0,A5

		PUTMSG	50,<'%s/Init: called'>

*		;----- call the library initialization routine
		LEA	devFuncInit(PC),A0		; relative offsets for relocatability
		LEA devStructInit(PC),A1
		SUB.L	A2,A2
		MOVE.L	#DV_SIZE,D0
		CALLSYS MakeLibrary

		TST.L	D0
		BNE.S	Init_Success

Init_Alert:
		ALERT	(AN_TrackDiskDev!AG_MakeLib),,A0

Init_Success:

		MOVE.L	D0,D6			; Save device structure address

		; begin -- do some data initialization manually
		; can use registers D0, D7, A0, A1 here without penalty

		LEA.L	hdName(PC),A1
		MOVE.L	A1,D7

		; check to make sure that there is no other similarly named device		
		LEA.L	DeviceList(A6),A0
		CALLSYS	FindName

Init_Name: 

*		; keep track of which controller we are using

		MOVE.L	D6,A1
		LEA.L	LN_NAME(A1),A1
		MOVE.L	D7,(A1)			; name was unique, stuff pointer into device
		MOVEQ	#0,D7			; no need to modify name anymore

		TST.L	D0				; now test result of find name...
		BNE.S	fix_name

		BRA.S	name_done

fix_name: ; name was not unique... let's fool the sytem into thinking it was

		MOVE.L	A1,D7			; save pointer to name field of device in d7

name_done: 

		; end -- some data initialization done manually

*		;------ Add the device
		PUTMSG	50,<'%s/Init: About to add device'>
		MOVE.L	D6,A1
		CALLSYS AddDevice

		IFD	AUTOINST		; If Autoconfig software used
		LEA.L	ExLibName(PC),A1		; Get expansion lib. name
		CLR.L	D0
		CALLSYS	OpenLibrary		; Open the expansion library
		TST.L	D0
		BNE.S	Init_OpSuccess

Init_OpFail:	ALERT	(AN_AMHDDiskDev!AG_OpenLib),,A0

Init_OpSuccess:	MOVE.L	D0,A4

		MOVEQ	#0,D3			; Index to 1st controller, assume default

********************************************************************************
*		; OK, so i was wrong, we ARE going to use the kludgey protocol
*
		LEA	INIT_CBIND(A5),A0	; Get the Current Bindings
		MOVE.L	#CurrentBinding_SIZEOF,D0;Size of Current Binding struct
		LINKLIB	_LVOGetCurrentBinding,A4
		MOVE.L	INIT_CBIND+cb_ConfigDev(A5),D0 ; Get start of list

Config_Loop:	TST.L	D0			; If controller not found
		BEQ	Init_End		;	Exit and unload driver
		MOVE.L	D0,A0			; Save config address
		MOVE.L	A0,INIT_CADDR(A5)	; Save for later use
		CMP.B	#2,cd_Rom+er_Product(A0); Is it HD+
		BNE.S	Just_Disk		;	No - Disk, no RAM
HD_PLUS:
		MOVE.L	cd_BoardAddr(A0),D0	; Get board base address
		ADD.L	#$100000,D0		; Point to 2nd MEG of board
		MOVE.L	D0,A1
		MOVE.L	#$5555AAAA,(A1)		; Test to see if RAM is present
		CMP.L	#$5555AAAA,(A1)
		BNE.S	Just_Disk
		NOT.L	(A1)
		CMP.L	#$AAAA5555,(A1)
		BNE.S	Just_Disk

*	RAM found, add to system

		MOVE.L	A1,A0			; RAM address
		MOVE.L	#0,A1			; No name supplied
		MOVE.L	#$100000,D0		; Has 1 MEG of RAM
		MOVE.L	#MEMF_PUBLIC!MEMF_FAST,D1; RAM is PUBLIC and FAST
		MOVEQ.L	#0,D2			; 0 Priority
		CALLLIB	_LVOAddMemList		; Add RAM to sys free pool

make_unique:
		MOVE.L	INIT_CADDR(A5),A0	; Restore pointer

Just_Disk:
		MOVE.L	cd_BoardAddr(A0),INIT_BADDR(A5); Get Board Base addess
		BCLR.B	#CDB_CONFIGME,cd_Flags(A0); Mark board as configured

		TST.L	D7
		BEQ.S	unique_name

ram_name:

		BTST.B  #ERTB_DIAGVALID,cd_Rom+er_Type(A0) 
		BNE.S   diag_valid                      ; running from copy ?

no_diagcopy:

		LEA.L   hdName(PC),A0                   ; or already in ram ...
		BRA.S	fix_ram_name
						  
diag_valid:

		MOVE.L  cd_Rom+er_Reserved0c(A0),D0     ; copy of diagarea?
		BEQ.S   no_diagcopy						; no, must be running from ram

		MOVE.L  D0,A0							; where is our copy of diagarea?

		LEA.L   hdName(PC),A1					; where is hdName?
		MOVE.L	A1,D0							; and what is its
		LEA.L   Rom_DiagArea(PC),A1				; offset from Rom_DiagArea?
		SUB.L	A1,D0
		ADDA.L	D0,A0							; find where the ram copy is...

fix_ram_name:

		ADDQ.B	#1,(A0)							; iddisk, jddisk, etc.
		MOVE.L	A0,D0							; save ram device name ptr
		MOVE.L	D7,A0							; ptr to device name field
		MOVE.L	D0,(A0)							; stuff new ram device name ptr

		; so, now we have to test for the uniqueness of THIS name...
		; uniqueness criteria here is to find ONLY once!!!

		LEA.L   DeviceList(A6),A0

find_loop:

		MOVE.L	D0,A1
		CALLSYS FindName						; will always find our own node!
		TST.L   D0
		BEQ.S   unique_name				

		CMP.L	D6,D0 							; did we find ourself ?
		BNE.S	make_unique
		MOVE.L	D0,A0
		BRA.S	find_loop						; look for another instance...

unique_name:

		ENDC

*		;------ Get memory for this controller's task
		MOVE.L	#HD_SIZE,D0
		MOVE.L	#MEMF_CLEAR!MEMF_PUBLIC,D1
		CALLSYS	AllocMem
		TST.L	D0
		BNE.S	ATask_Success

		ALERT	(AN_AMHDDiskDev!AG_NoMemory),,A0

ATask_Success:	MOVE.L	D0,A2

;
;	Initialize HD structure
;
		MOVEQ.L	#0,D0				; No need to re-zero
		LEA	HDStructInit(PC),A1		; Initialization structure
		CALLSYS	InitStruct			; A2 already points to HD struct

		; begin -- do some data initialization manually
		; can use registers D0, A1 here without penalty

		LEA.L	hdintr(PC),A1
		MOVE.L	A1,D0

		LEA.L	HD_IS+IS_CODE(A2),A1
		MOVE.L	D0,(A1)

		LEA.L	hdName(PC),A1
		TST.L	D7
		BEQ.S	rom_name2
		MOVE.L	D7,A1 			; fetch name pointer back again
		MOVE.L	(A1),A1			; ok, got it!
rom_name2:
		MOVE.L	A1,D0

		MOVE.L	D6,A1
		LEA.L	HD_MP+LN_NAME(A2),A1
		MOVE.L	D0,(A1)

		MOVE.L	D6,A1
		LEA.L	HD_TCB+LN_NAME(A2),A1
		MOVE.L	D0,(A1)

		; end -- some data initialization done manually

		MOVEQ.L	#0,D0			; need to re-zero

		MOVE.L	D6,A1			; Get address of DV structure
		MOVE.L	A2,DV_0(A1,D3)	; Save this HD struct address
		ADDQ.L	#4,D3			; Bump index for next controller

		LEA.L	IntuitLibName(PC),A1	; Get intuition lib. name
		MOVEQ.L	#0,D0
		CALLSYS	OpenLibrary
		TST.L	D0			; If open failed,
		BEQ	Init_OpFail		;	alert
		MOVE.L	D0,HD_INTUITLIB(A2)	; Save Intuition base

		IFGE	INFO_LEVEL-50
		MOVE.L	A2,-(SP)
		PUTMSG	50,<'%s/Init: device is at %lx'>
		ADDQ.L	#4,SP
		ENDC

*		;------ Initialize the device structures
		MOVE.L	A6,HD_SYSLIB(A2)
		MOVE.L	A2,HD_IS+IS_DATA(A2)	; Pass int. server dev. addr.
		LEA	hdintr(PC),A0
		MOVE.L	A0,HD_IS+IS_CODE(A2)	; Put in addr.of intr. server

*		;------ Initialize the stack information
		LEA	HD_STACK(A2),A0		; Low end of stack
		MOVE.L	A0,HD_TCB+TC_SPLOWER(A2)
		LEA	HD_STKSIZE(A0),A0	; High end of stack
		MOVE.L	A0,HD_TCB+TC_SPUPPER(A2)
		MOVE.L	A2,-(A0)		; argument -- device ptr
		MOVE.L	A0,HD_TCB+TC_SPREG(A2)

		MOVE.L	INIT_BADDR(A5),HD_BASE(A2); Save base pointer
		LEA.L	HD_CMDAREA(A2),A0	; Compute address of controller
		MOVE.L	A0,D0
		ADD.L	#$1FF,D0		;	command block, aligned
		AND.L	#$FFFFFE00,D0		;	on a 512 byte boundary
		MOVE.L	D0,HD_CMDPTR(A2)	; Save address in dev. structure

		;------ initialize the device's list
		LEA	HD_MP+MP_MSGLIST(A2),A0
		NEWLIST	A0
		LEA	HD_TCB(A2),A0
		MOVE.L	A0,HD_MP+MP_SIGTASK(A2)
		MOVE.B	#HDB_NEWIOREQ,HD_MP+MP_SIGBIT(A2)
		MOVE.B	#NT_MSGPORT,HD_MP+LN_TYPE(A2)	; Indicate is MsgPort

*	Startup the task
		PUTMSG	50,<'%s/Init: About to startup task'>
		MOVE.L	A2,-(SP)		; save device pointer
		LEA	HD_TCB(A2),A1
		LEA	HDTaskStart(PC),A2
		LEA	-1,A3			; generate address error
						; if task ever "returns"
		CLEAR	D0
		CALLSYS AddTask
		MOVE.L	(SP)+,A2		; Restore device pointer

*		Initialize INIT's internal Message Port and IORequest Block

		SUB.L	A1,A1			; Find this task's TCB
		CALLSYS	FindTask		
		LEA	INIT_MP(A5),A0		; Point to message port
		MOVE.L	D0,MP_SIGTASK(A0)	; Store TCB in MsgPort
		MOVE.B	#HDB_OWNIO,MP_SIGBIT(A0); Indicate which signal to use	
		MOVE.B	#NT_MSGPORT,LN_TYPE(A0)	; Indicate is MsgPort
		LEA	INIT_IOR(A5),A1		; Point to IORequest
		MOVE.L	A0,MN_REPLYPORT(A1)	; Make this port the replyport
		LEA	MP_MSGLIST(A0),A0	; Initialize ports msglist
		NEWLIST	A0
		MOVE.W	#IOSTD_SIZE,MN_LENGTH(A1) ; Set message length
		MOVE.L	D6,IO_DEVICE(A1)	; Set device pointer

		MOVE.L	A6,-(SP)		; Preserve SYSLIB pointer
		MOVE.L	A2,A6			; Put DEVICE addr in A6
		MOVEQ	#0,D2			; Try and init UNIT 1
		MOVEQ	#1,D5			; De-allocate UNIT if fails
		MOVE.L	#0,D4			; FLAGS
		BSR	InitUnit
		TST.L	D0			; If init succeeded,
		BNE.S	Init1
		MOVE.L	A0,HDU0(A6)		;	save unit pointer
Init1:		MOVEQ	#1,D2			; Try and init UNIT 2
		MOVEQ	#1,D5			; De-allocate UNIT if fails
		MOVE.L	#0,D4			; FLAGS
		PUTMSG	50,<'%s/Init: About to InitUnit'>
		BSR	InitUnit
		TST.L	D0			; If init succeeded,
		BNE.S	Init2
		MOVE.L	A0,HDU1(A6)		;	save unit pointer

Init2:
		BSET.B	#HDB_MSGING,HD_FLAGS(A6); Set, until an open clears it
		BTST.B	#HDB_HASSCSI,HD_FLAGS(A6)

		MOVE.L	D6,-(SP)		; preserve device pointer
		BEQ.S	Init_Sx			; If not set, no SCSI chip
		MOVEQ	#0,D6			; Try and init dos UNIT 3 thru 8 (SCSI 0..6)

	    ;------ delay to allow scsi units a chance to spin up....

	    MOVEQ   #20,D1
	    MOVEQ   #-1,D0
idle:  	NOP
		NOP
	    DBF	    D0,idle
	    DBF	    D1,idle

InitScsiLoop:

		CMP.L	#24,D6			; only seven units possible for now
		BGT		Init_Sx			; done for all possible units?

		MOVE.L	D6,D2			; Try and init SCSI UNITS 0 thru 6 (we're 7)
		LSR.L	#2,D2
		ADDQ.L	#2,D2

		MOVEQ	#1,D5			; De-allocate UNIT if failure
		MOVEQ	#0,D4			; FLAGS

		BSR	InitUnit
		TST.L	D0			; If init failed,

		BNE		Init_next	;	try next

InitScsiOK:	

		MOVE.L	A0,HDUS0(A6,D6.L)		;	save SCSI unit pointer

Init_next:

		BSR		get_timeout
		TST.B	D0
		BEQ.S	no_timeout
		LSR.B	#1,D0 				; make scsi timeout half again as long
		BSR		set_timeout
no_timeout:
		ADDQ.L	#4,D6
		BRA		InitScsiLoop

Init_Sx: 
		MOVE.L	(SP)+,D6		; restore device pointer
		MOVE.L	INIT_CADDR(A5),A0
		MOVE.L	A0,cd_Driver(A0)	; Save pointer to this device

*	Check all boards on the list		; Need loop for mutiple boards
		PUTMSG	50,<'%s/Init: Checking next board'>
		MOVE.L	cd_NextCD(A0),D0
		MOVE.L	(SP)+,A6		; Restore SYSBASE
		BRA	Config_Loop

Init_End:

		PUTMSG	50,<'%s/Init: Closing libs'>
		MOVE.L	A4,A1			; Now close expansion library
		CALLSYS	CloseLibrary

		MOVE.L	A5,A1
		MOVE.L	#INIT_SIZE,D0
		CALLSYS	FreeMem
		MOVEQ.L	#1,D0			; Indicate success
		PUTMSG	50,<'%s/Init: Exiting'>
		MOVEM.L	(SP)+,D2-D7/A2-A5
		RTS

*    InitUnit:
*    --------
*
*	A6 --> device pointer
*	A4 --> expansionbase pointer
*	A2 --> temp Memory
*	A3 --> unit memory
*	D2 --> unit number
*   D3 --> controller number
*	D4 --> unit FLAG byte
*	D5 --> Non-zero means deallocate memory if open fails
*	D6 --> Scratch
*	D7 --> Pointer to pointer to RAM device.name

		
InitUnit:
	MOVEM.L	D2/D3/D4/D6/D7/A2/A3/A4/A5,-(SP)
*		;------ get temp memory
		MOVE.L	#INIT_SIZE,D0
		MOVE.L	#MEMF_CLEAR!MEMF_PUBLIC,D1
		LINKSYS	AllocMem
		TST.L	D0
		BEQ	InitUnit_NoTempMem
		MOVE.L	D0,A2

*		;------ get unit memory
		MOVE.L	#HDU_SIZE,D0
		MOVE.L	#MEMF_CLEAR!MEMF_PUBLIC,D1
		LINKSYS	AllocMem
		TST.L	D0
		BEQ	InitUnit_NoMem

		MOVE.L	D0,A3

		MOVE.B	D4,HDU_FLAGS(A3)	; Set unit's FLAGS byte
*		;------ store the unit constants
		MOVE.B	D2,HDU_UNIT(A3)		; Save the actual unit #
		MOVE.L	A6,HDU_CTLR(A3)		; Save pointer to controller
		CMP.B	#2,D2			; If > 1, is a SCSI unit
		BLT.S	UNotSCSI
*LCE		BTST.B	#HDUB_MSGING,D4		;    If device doesn't support
*LCE		BNE.S	Has_Messaging		;	messaging,
		BCLR.B	#HDB_MSGING,HD_FLAGS(A6);	No device can use it!
Has_Messaging:
		SUBQ.B	#2,D2			; Have mask for SCSI start at 0
UNotSCSI:	LSL.B	#5,D2			; Convert to mask for LUNHIADDR
		MOVE.B	D2,HDU_UNITNUM(A3)

*		Initialize OPEN's internal Message Port and IORequest Block

		SUB.L	A1,A1			; Find this task's TCB
		LINKSYS	FindTask,HD_SYSLIB(A6)		
		LEA	INIT_MP(A2),A0		; Point to message port
		MOVE.L	D0,MP_SIGTASK(A0)	; Store TCB in MsgPort
		MOVE.B	#HDB_OWNIO,MP_SIGBIT(A0); Indicate which signal to use	
		MOVE.B	#NT_MSGPORT,LN_TYPE(A0)	; Indicate is MsgPort
		LEA	INIT_IOR(A2),A1		; Point to IORequest
		MOVE.L	A0,MN_REPLYPORT(A1)	; Make this port the replyport
		LEA	MP_MSGLIST(A0),A0	; Initialize ports msglist
		NEWLIST	A0
		MOVE.W	#IOSTD_SIZE,MN_LENGTH(A1) ; Set message length
		MOVE.L	A6,IO_DEVICE(A1)	; Set device pointer
		MOVE.L	A3,IO_UNIT(A1)		; Set unit pointer

		CMP.B	#2,HDU_UNIT(A3)		; If > 1, is a SCSI unit
		BLT	INotSCSI

WAIT_READY:	LEA	INIT_IOR(A2),A1		; Point to IORequest
		MOVE.W	#HD_SPECIAL,IO_COMMAND(A1)
		LEA	INIT_CMD(A2),A1		; Point to internal command blk
		MOVE.L	A1,IO_DATA+INIT_IOR(A2)
		MOVE.B	#HDC_TDR,CMD_OPCODE(A1) ; Set to "test drive ready"
		MOVE.W	#$0000,CMD_MIDADDR(A1)	; Block 0
		MOVE.B	#$00,CMD_BLOCKCNT(A1)

*		MOVE.B	#0,CMD_LUNHIADDR(A1)	; using lunhiaddr of #0 is incorrect
*		bart says -- using lunhiaddr for target address is dumb design and wrong
*		driver must be rewritten to address both target address 0-6 and
*		lun (sub addresses) 0-7 for each target, if supported.

		MOVE.B	D2,CMD_LUNHIADDR(A1)	; Set correct unit number

		MOVE.B	#$FF,CMD_ERRORBITS(A1)	; Tell controller new command
		LEA	INIT_SECT(A2),A0	; Point to sector buffer
		MOVE.L	A0,D0
		MOVE.B	D0,CMD_LOWDMA(A1)	; Store Low byte of address
		ROR.L	#8,D0
		MOVE.B	D0,CMD_MIDDMA(A1)	; Store Middle byte of address
		ROR.L	#8,D0
		MOVE.B	D0,CMD_HIGHDMA(A1)	; Store High byte address
		LEA	HD_MP(A6),A0		; Point to message port
		LEA	INIT_IOR(A2),A1		; Point to IORequest
		LINKSYS	PutMsg,HD_SYSLIB(A6)	; Pass to driver task
		MOVE.L	#HDF_OWNIO,D0		; Wait for completion
		LINKSYS	Wait,HD_SYSLIB(A6)
		LEA	INIT_CMD(A2),A1		; Point to internal command blk
		CMP.B	#$80,CMD_ERRORBITS(A1)	; If OK,
		BEQ.S	StartSCSI		;	try and read
		CMP.B	#$FE,CMD_ERRORBITS(A1)	; If select TIMEOUT,
		BEQ	BAD_MAGIC		;	assume no drive
		CMP.B	#$F8,CMD_ERRORBITS(A1)	; If drive not ready
		BEQ	WAIT_READY		;	wait till ready

*       other error?
*		don't assume no drive until start attempt fails
*		BRA BAD_MAGIC       ;   assume no drive

*	Issue START UNIT command to unit and hope for the best

StartSCSI: LEA   INIT_IOR(A2),A1     ; Point to IORequest	
		MOVE.W	#HD_SPECIAL,IO_COMMAND(A1)
		LEA	INIT_CMD(A2),A1		; Point to internal command blk
		MOVE.L	A1,IO_DATA+INIT_IOR(A2)
		MOVE.B	#HDC_START,CMD_OPCODE(A1); Set to "start/stop unit "
		MOVE.W	#$0000,CMD_MIDADDR(A1)	; Block 0
		MOVE.B	#$01,CMD_BLOCKCNT(A1)	; start unit
		MOVE.B	#$FF,CMD_ERRORBITS(A1)	; Tell controller new command
		MOVE.B	D2,CMD_LUNHIADDR(A1)	; Set correct unit number, not immediate
		LEA	HD_MP(A6),A0		; Point to message port
		LEA	INIT_IOR(A2),A1		; Point to IORequest
		LINKSYS	PutMsg,HD_SYSLIB(A6)	; Pass to driver task
		MOVE.L	#HDF_OWNIO,D0		; Wait for completion
		LINKSYS	Wait,HD_SYSLIB(A6)
		LEA	INIT_CMD(A2),A1		; Point to internal command blk
		CMP.B	#$80,CMD_ERRORBITS(A1)	; If OK,
		BEQ.S	INotSCSI	;	try and read
		BRA	BAD_MAGIC		;	otherwise exit

*	Now read in 1st readable block for this unit

*		Tell HDIO to pass this command directly to the controller
INotSCSI: LEA   INIT_IOR(A2),A1     ; Point to IORequest	
		MOVE.W	#HD_SPECIAL,IO_COMMAND(A1)
		LEA	INIT_CMD(A2),A1		; Point to internal command blk
		MOVE.L	A1,IO_DATA+INIT_IOR(A2)
		MOVE.B	#HDC_READ,CMD_OPCODE(A1); Set to "read"
		MOVE.W	#$0000,CMD_MIDADDR(A1)	; Block 0
		MOVE.L	#MAXMAXTRY,D6			; Retry up to 30 times
INFO_Retry:		LEA	INIT_CMD(A2),A1		; Point to internal command blk
		MOVE.B	#$01,CMD_BLOCKCNT(A1)	; 1 block
		MOVE.B	#$FF,CMD_ERRORBITS(A1)	; Tell controller new command
		MOVE.B	D2,CMD_LUNHIADDR(A1)	; Set correct unit number
		LEA	INIT_SECT(A2),A0	; Point to sector buffer
		MOVE.L	A0,D0
		MOVE.B	D0,CMD_LOWDMA(A1)	; Store Low byte of address
		ROR.L	#8,D0
		MOVE.B	D0,CMD_MIDDMA(A1)	; Store Middle byte of address
		ROR.L	#8,D0
		MOVE.B	D0,CMD_HIGHDMA(A1)	; Store High byte address
		LEA	HD_MP(A6),A0		; Point to message port
		LEA	INIT_IOR(A2),A1		; Point to IORequest
		LINKSYS	PutMsg,HD_SYSLIB(A6)	; Pass to driver task
		MOVE.L	#HDF_OWNIO,D0		; Wait for completion
		LINKSYS	Wait,HD_SYSLIB(A6)

		MOVE.B	CMD_ERRORBITS+INIT_CMD(A2),D0 ; See if hard error
		BSR	Soft_Error
		BEQ.S	Got_SECT		;	No - GOT IT!
		CMP.B	#2,HDU_UNIT(A3)		; If a SCSI device,
		BLT.S	NotSCSI
		CMP.B	#$FE,CMD_ERRORBITS+INIT_CMD(A2) ; Timeout error ?
		BEQ	BAD_MAGIC		;		 exit
		CMP.B	#$F3,CMD_ERRORBITS+INIT_CMD(A2)
		BEQ.S	RETRY_SAME
NotSCSI:	ADD.W	#1,CMD_MIDADDR+INIT_CMD(A2); Hard error - try next block
RETRY_SAME:	DBRA	D6,INFO_Retry		; 	but not past 30
		BRA	BAD_MAGIC	; No info - new/bad disk
		
Got_SECT:
		CLR.L	D4
		MOVE.W	CMD_MIDADDR+INIT_CMD(A2),D4 ; Save this block #
		MOVE.L	D4,HDU_BOOT(A3)
		CMP.W	#$BABE,INIT_SECT(A2)	; See if valid INFO sector
		BEQ.S	Got_INFO
		ADD.W	#1,CMD_MIDADDR+INIT_CMD(A2) ; Hard error - try next block
		DBRA	D6,INFO_Retry		; 	but not past 30
		BRA	BAD_MAGIC		; No info - new/bad disk

Got_INFO:	BSR	Get_INFO	; Extract info from sector

*	Now check for bad block records, and load the bad block table if present

		MOVE.L	INIT_SECT+HDB_PT_BBLOCKS(A2),D0; Get Bad block pointer
		MOVE.L	D0,HDU_1BAD(A3)		; Save in unit structure
		BEQ	OPEN_NO_BAD_BLOCKS

*	Now read in bad block structure

OPEN_BB_READ:
		MOVE.B	#HDC_READ,CMD_OPCODE+INIT_CMD(A2); Set to "read"
		MOVE.B	#$01,CMD_BLOCKCNT+INIT_CMD(A2)	; 1 block
		MOVE.B	#$FF,CMD_ERRORBITS+INIT_CMD(A2); New command
		MOVE.W	D0,CMD_MIDADDR+INIT_CMD(A2);Store low order word
		SWAP	D0			; Get high order byte of BLK #
		ADD.B	D2,D0			; Add in LUN
		MOVE.B	D0,CMD_LUNHIADDR+INIT_CMD(A2);	and save in command blk
 		LEA	HDU_BB(A3),A0		; Point to Bad Block structure
		MOVE.L	A0,D0
		MOVE.B	D0,CMD_LOWDMA+INIT_CMD(A2); Store Low byte of address
		ROR.L	#8,D0
		MOVE.B	D0,CMD_MIDDMA+INIT_CMD(A2); Store Middle byte of address
		ROR.L	#8,D0
		MOVE.B	D0,CMD_HIGHDMA+INIT_CMD(A2); Store High byte address
		LEA	HD_MP(A6),A0		; Point to message port
		LEA	INIT_IOR(A2),A1		; Point to IORequest
		LINKSYS	PutMsg,HD_SYSLIB(A6)	; Pass to driver task
		MOVE.L	#HDF_OWNIO,D0		; Wait for completion
		LINKSYS	Wait,HD_SYSLIB(A6)
		MOVE.B	INIT_CMD+CMD_ERRORBITS(A2),D0 ; See if hard error
		BSR	Soft_Error
		BNE	OPEN_BB_FAIL		; 	Hard Error - fail

		CMP.W	#$BAD1,HDU_BB+BBR_MAGIC1(A3); Valid bad block tbl?
		BNE	OPEN_BB_FAIL		; Branch if not

*		Load the 2nd half of Bad Block structure

*			2nd block starts at middle of table

 		LEA	HDU_BB+BBR_TABLE+(BBM_SIZE*NBAD/2)(A3),A0
		MOVE.L	A0,D0
		MOVE.B	D0,CMD_LOWDMA+INIT_CMD(A2); Store Low byte of address
		ROR.L	#8,D0
		MOVE.B	D0,CMD_MIDDMA+INIT_CMD(A2); Store Middle byte of address
		ROR.L	#8,D0
		MOVE.B	D0,CMD_HIGHDMA+INIT_CMD(A2); Store High byte address

*			2nd block # contained in 1st block

		MOVE.L	BBR_NEXT_REC+HDU_BB(A3),D0 ; Get block #		
		MOVE.L	D0,HDU_2BAD(A3)		; Save in unit structure
		MOVE.W	D0,CMD_MIDADDR+INIT_CMD(A2) ; Store low order word
		SWAP	D0			; Get high order byte of BLK #
		ADD.B	D2,D0			; Add in LUN
		MOVE.B	D0,CMD_LUNHIADDR+INIT_CMD(A2) ;	and save in command blk

		LEA	HD_MP(A6),A0		; Point to message port
		LEA	INIT_IOR(A2),A1		; Point to IORequest
		LINKSYS	PutMsg,HD_SYSLIB(A6)	; Pass to driver task
		MOVE.L	#HDF_OWNIO,D0		; Wait for completion
		LINKSYS	Wait,HD_SYSLIB(A6)
		MOVE.B	INIT_CMD+CMD_ERRORBITS(A2),D0 ; See if hard error
		BSR	Soft_Error
		BNE.S	OPEN_BB_FAIL		; 	Hard Error - fail

		CMP.W	#$BAD2,HDU_BB+BBR_MAGIC2(A3); Valid bad block tbl?
		BNE	OPEN_BB_FAIL		; Branch if not

*		Mark the end of the table with a very large block number

		MOVE.W	BBR_COUNT+HDU_BB(A3),D1 ; Get block count
		MOVE.W	#BBM_SIZE,D0		; Compute Destination address
		MULU	D1,D0
		LEA	HDU_BB+BBR_TABLE(A3,D0),A0
		MOVE.L	#$7FFFFFFF,BBM_BAD(A0)	; Make last #'s very big
		MOVE.L	#$7FFFFFFF,BBM_GOOD(A0)
		BRA		OPEN_BB_DONE

BAD_MAGIC:	; Jump here if don't know what type of drive it is

		MOVE.W	#0,BBR_COUNT+HDU_BB(A3) ; Zero count of blocks
		MOVE.L	#$7FFFFFFF,BBR_TABLE+BBM_BAD+HDU_BB(A3)
		MOVE.L	#$7FFFFFFF,BBR_TABLE+BBM_GOOD+HDU_BB(A3)
		MOVE.L	D5,D0		; If zero, means open no matter what

		TST.L	D0
		BEQ	InitUnit_End		
; This was a conditional init that failed, so free the unit's memory
		MOVE.L	A3,A1
		MOVE.L	#HDU_SIZE,D0
		LINKSYS	FreeMem
		MOVEQ	#HDERR_NoMem,D0

		BRA	InitUnit_End

OPEN_BB_FAIL:	; Bad Block table build failed, show table as empty
OPEN_NO_BAD_BLOCKS:

		MOVE.W	#$BAD1,BBR_MAGIC1+HDU_BB(A3); Initialize Magic 1
		MOVE.W	#$BAD2,BBR_MAGIC2+HDU_BB(A3); Initialize Magic 2
		MOVE.L	#1,HDU_1BAD(A3)		; Show 1st half of table in 1
		MOVE.L	#2,HDU_2BAD(A3)		; Show 2nd half of table in 2
		MOVE.W	#0,BBR_COUNT+HDU_BB(A3) ; Zero count of blocks
		MOVE.L	#$7FFFFFFF,BBR_TABLE+BBM_BAD+HDU_BB(A3)
		MOVE.L	#$7FFFFFFF,BBR_TABLE+BBM_GOOD+HDU_BB(A3)
		CLR.L	D0		; Compute # of sectors per cylinder
		CLR.L	D1
		MOVE.W	HDU_SECTORS(A3),D0
		MOVE.B	HDU_HEADS(A3),D1
		MULU	D1,D0		; D0 now contains # of sectors/cyl
		LSR.W	#1,D0		; Assume two cylinders reserved
		SUBQ.W	#3,D0		; 3 blocks used
		MOVE.W	D0,BBR_LEFT+HDU_BB(A3) ; Show as number of free blocks
		MOVE.L	#3,BBR_NEXT_FREE+HDU_BB(A3) ; 3 is next free block;
OPEN_BB_DONE:

* Now tell AmigaDOS about 1st partition on the disk
* If AmigaDos not open yet, put this device node on eb_MountList

		IFD	AUTOINST		; If Autoconfig software used
		LEA	hdName(PC),A0		; Get address of driver name

		TST.L	D7
		BEQ.S	rom_name3
		MOVE.L	D7,A0 			; fetch ram name pointer back again
		MOVE.L	(A0),A0			; ok, got it!
rom_name3:

		MOVE.L	A0,HDB_EXECNAME+INIT_SECT(A2) ; Save for MakeDosNode
		MOVE.L	#0,HDB_DEVFLGS+INIT_SECT(A2)  ; Zero device flags
		LEA	HDB_DOSNAME+INIT_SECT(A2),A0 ; Get addr of Device name
		MOVE.L	A0,HDB_DOSNPTR+INIT_SECT(A2) ;	and save in environment

		MOVEQ	#0,D0		; Build device name
		MOVE.B	HDU_UNIT(A3),D0
		LSL.L	#8,D0
		ADD.W	#$3000,D0	; Make ASCII
		MOVE.W	D0,2(A0)	;	and store in name

		MOVE.W	D3,D0		; Get (controller # + 1) * 4
		LSR.W	#2,D0		; Convert to controller #
		ADD.W	#$4447,D0	; Add 'DH' to it (controller DH, DI,..)
		MOVE.W	D0,0(A0)	; Store 'DH' in name

*		; must check to make sure this dosname is unique

		MOVE.L	A2,-(SP)				; save temp regs

dosname_check:

		MOVE.L	(A0),D0					; get current "name"

		LEA.L	eb_MountList(A4),A1		
		MOVE.L	LH_HEAD(A1),D1
		BEQ.S	dosname_unique

		MOVE.L	D1,A1					; lh_Head in A1

dosnode_loop:

		MOVE.L	LN_SUCC(A1),D1			; more bootnodes?
		BEQ.S	dosnode_loopexit		; no

		MOVE.L	bn_DeviceNode(A1),D2	; cptr to devnode
		BEQ.S	dosnode_next

		MOVE.L	D2,A1					; devnode ptr in A1
		MOVE.L	dn_Name(A1),D2			; bptr to devname
		BEQ.S	dosnode_next

		LSL.L	#2,D2
		MOVE.L	D2,A1					; ptr to string size in A1

		MOVEQ.L	#0,D2
		MOVE.B	(A1)+,D2				; fetch string size and point to string
		CMP.L	#3,D2					; same as current string size?
		BNE.S	dosnode_next

		MOVE.L	A0,A2					; copy of current nameptr in A2
		BRA.S	cmpname_next

cmpname_loop:

		CMPM.B	(A1)+,(A2)+				; byte-by-byte comparison

cmpname_next:

		DBNE	D2,cmpname_loop			; fail early
		BNE.S	dosnode_next			; names differ...

dosname_fix:							; names are identical, change current

		ADDI.L	#$00010000,D0			; DIn:, DJn:, DKn:, etc.
		MOVE.L	D0,(A0)					; store current "name"
		BRA.S	dosname_check			; now check the entire list again...

dosnode_next:

		MOVE.L	D1,A1
		BRA.S	dosnode_loop

dosnode_loopexit:

		MOVE.L	D0,(A0)					; store current "name"

dosname_unique:

		MOVE.L	(SP)+,A2				; restore temp regs

*		; done checking to make sure this dosname is unique

		MOVE.L	D3,D0		; Get (controller # + 1) * 4
		LSR.L	#2,D0		; Divide by 4
		SUBQ.L	#1,D0
		MULU	#10,D0		
		MOVEQ.L	#0,D1
		MOVE.B	HDU_UNIT(A3),D1
		ADD.L	D1,D0
		ADDQ.L	#1,D0
		MOVE.L	D0,HDB_UNITNO+INIT_SECT(A2) ; and save in environment

********************************************************************************
*
*		AddDosNode does not currently allow us to pass the CD in the
*		ln_Name field of the BootNode structure added to the eb_MountList.
*		So, we will just add it to the MountList directly...
*
********************************************************************************

		XREF	AutoBoot_Dosname

********************************************************************************
*
*		Unless we are disk-based at this point... if so then DOSLib will
*		exist and AddDosNode will mount device directly onto Dos rather than
*		onto eb_MountList
*
********************************************************************************

check_dos:

		LEA.L	AutoBoot_Dosname(PC),A1		; Get dos lib. name
		CLR.L	D0
		LINKSYS	OpenLibrary					; Open the dos library
		TST.L	D0
		BEQ		no_dos

********************************************************************************
*
*		We're disk based... skip the manual eb_MountList stuff
*
********************************************************************************

is_dos:

*		; more checking to make sure this dosname is unique

		MOVEM.L	D0/A2,-(SP)			; save registers

dosinfo_check:

		MOVE.L	(SP),A0				; dosbase in A0
		MOVE.L	dl_Root(A0),A0
		MOVE.L	rn_Info(A0),D0
		LSL.L	#2,D0				; convert from bptr
		MOVE.L	D0,A1

devinfo_next:

		MOVE.L	di_DevInfo(A1),D0
		BEQ.S	devinfo_done
		LSL.L	#2,D0				; convert from bptr
		MOVE.L	D0,A1

device_next:

		MOVE.L	A1,D0
		BEQ.S	devinfo_done
		MOVE.L	D0,A0				; current Node

		MOVE.L	dn_Next(A1),D0		; walk list
		LSL.L	#2,D0				; convert from bptr
		MOVE.L	D0,A1				; next Node

		MOVE.L	dn_Type(A0),D0
		CMP.L	#DLT_DEVICE,D0		; is this our type of node?
		BNE.S	device_next			; not our type, ignore

		MOVE.L	dn_Name(A0),D0
		LSL.L	#2,D0				; convert from bptr
		MOVE.L	D0,A0				; current bcpl dos device Name string

		MOVEQ	#0,D1
		MOVE.B	(A0)+,D1			; number of characters in Name

		CMP.L	#3,D1				; same as current proposed string size?
		BNE.S	device_next

		MOVE.L	4(SP),A2			; retreive A2
		LEA	HDB_DOSNAME+INIT_SECT(A2),A2 ; Get addr of our proposed Device name
		MOVE.L	A2,D0				; save ptr in case we need to fix string

		SUBQ.L	#1,D1					; adjust count, and fall through...

cmp_dosloop:

		CMPM.B	(A0)+,(A2)+				; byte-by-byte comparison

cmp_dosname:

		DBNE	D1,cmp_dosloop			; fail early
		BNE.S  	device_next 			; names differ... 

devinfo_fix:

		MOVE.L	D0,A0					; device Name string ptr
		MOVE.L	(A0),D0					; current Name string

		ADDI.L	#$00010000,D0			; DIn:, DJn:, DKn:, etc.
		MOVE.L	D0,(A0)					; store current "name"
		BRA.S	dosinfo_check			; now check the entire list again...

devinfo_done:

		MOVEM.L	(SP)+,D0/A2			; restore registers

*		; finally done checking to make sure this dosname is unique

		LEA	HDB_DOSNPTR+INIT_SECT(A2),A0
		LINKLIB	_LVOMakeDosNode,A4		; Build AmigaDOS structures

		TST.L	D0
		BEQ		InitUnit_NoMem

		MOVE.L  D0,A0           ; Get deviceNode address 
		MOVEQ.L #0,D0           ; Set device priority to 0 
		MOVEQ.L #0,D1
		LINKLIB _LVOAddDosNode,A4

		BRA.S	BootNode_Done

********************************************************************************
*
*		We're not disk based... perform the manual eb_MountList stuff
*
********************************************************************************

no_dos:

		CLEAR   D1
		MOVEQ.L	#BootNode_SIZEOF,d0
		LINKSYS	AllocMem
		TST.L	D0
		BEQ		InitUnit_NoMem
		MOVE.L	D0,A1					; Get BootNode address

		MOVE.L	A1,-(SP)				; save pointer to BootNode
		LEA	HDB_DOSNPTR+INIT_SECT(A2),A0
		LINKLIB	_LVOMakeDosNode,A4		; Build AmigaDOS structures
		MOVE.L	(SP)+,A1				; restore pointer to BootNode

		TST.L	D0
		BNE.S	got_node

not_node:
		MOVEQ.L	#BootNode_SIZEOF,D0
		LINKSYS	FreeMem					; remember to free the bootnode mem
		BRA.S	InitUnit_NoMem			; and to return an error code

got_node:
		MOVE.W	#0,bn_Flags(A1)
		MOVE.B	#0,LN_PRI(A1)			; priority 0 for run-of-the mill hddisk
		MOVE.B	#NT_BOOTNODE,LN_TYPE(A1)	; type NT_BOOTNODE indicates
		MOVE.L	D0,bn_DeviceNode(A1)		; that this DeviceNode is associated
		MOVE.L	INIT_CADDR(A5),LN_NAME(A1)	; with CD address in ln_Name field

		LEA.L	eb_MountList(A4),A0		
		LINKSYS	Enqueue					; add BootNode to MountList

		BRA.S	BootNode_Done

free_BootNode:

		MOVEQ.L	#BootNode_SIZEOF,D0
		LINKSYS	FreeMem

BootNode_Done:

********************************************************************************

		ENDC

		CLEAR	D0

InitUnit_End:
		MOVE.L	D0,A4			; Preserve D0

		MOVE.L	A2,A1
		MOVE.L	#INIT_SIZE,D0
		LINKSYS	FreeMem
		MOVE.L	A4,D0
		MOVE.L	A3,A0
		MOVEM.L	(SP)+,D2/D3/D4/D6/D7/A2/A3/A4/A5
		RTS

InitUnit_NoMem:
		IFGE	INFO_LEVEL-30
		PUTMSG	0,<'%s/InitUnit: Can not allocate memory'>
		ENDC

		MOVEQ	#HDERR_NoMem,D0
		BRA.S	InitUnit_End

InitUnit_NoTempMem:

		MOVEQ	#HDERR_NoMem,D0
		MOVEM.L	(SP)+,D2/D3/D4/D6/D7/A2/A3/A4/A5
		RTS

Get_INFO:	;	Extract info from boot sector

		MOVE.L	D3,-(SP)
*	Now tell controller what type of drive this is

		MOVE.B	#$4F,HDP_OPTSTEP+INIT_HDP(A2) ; Fast seek
		MOVE.L	HDB_PRECOMP+INIT_SECT(A2),D0 ; Get PRECOMP cylinder
		LSR.L	#4,D0			; 	and divide by 16
		MOVE.B	D0,HDP_PRECOMP+INIT_HDP(A2)
		MOVE.L	HDB_REDUCE+INIT_SECT(A2),D0 ; Get REDUCE cylinder
		LSR.L	#4,D0			; 	and divide by 16
		MOVE.B	D0,HDP_REDUCE+INIT_HDP(A2)
		MOVE.W	HDB_SECTORS+INIT_SECT(A2),D0
		MOVE.W	D0,HDU_SECTORS(A3)
		MOVE.B	D0,HDP_SECTORS+INIT_HDP(A2)
		MOVE.L	HDB_CYLINDERS+INIT_SECT(A2),D0 ; Get # of cylinders
		MOVE.B	D0,HDP_CYL+INIT_HDP(A2)	; Save low order # of cyls
		LSR.L	#8,D0
		MOVE.W	HDB_HEADS+INIT_SECT(A2),D1
		MOVE.B	D1,HDU_HEADS(A3)
		LSL.B	#4,D1
		OR.B	D1,D0
		MOVE.B	D0,HDP_HEADHICYL+INIT_HDP(A2)

		MOVE.B	#HDC_SDP,CMD_OPCODE+INIT_CMD(A2);"Set drive paramaters"
		TST.B	HDU_UNITNUM(A3)		; If isn't unit 0,
		BEQ.S	IsUnit0
		MOVE.B	#HDC_SDP1,CMD_OPCODE+INIT_CMD(A2);change to opcode CC
IsUnit0:	MOVE.B	#$FF,CMD_ERRORBITS+INIT_CMD(A2);Tell new command
		LEA	INIT_HDP(A2),A0		; Point to drive type parameter
		MOVE.L	A0,D0			;	information
		MOVE.B	D0,CMD_LOWDMA+INIT_CMD(A2); Store Low byte of address
		ROR.L	#8,D0
		MOVE.B	D0,CMD_MIDDMA+INIT_CMD(A2); Store Middle byte of address
		ROR.L	#8,D0
		MOVE.B	D0,CMD_HIGHDMA+INIT_CMD(A2); Store High byte address
		MOVE.L	HDB_PARK+INIT_SECT(A2),D3; Get PARK cylinder
		BEQ.S	NO_PARK1
		BMI.S   NO_PARK1

		ADDQ.L	#1,D3
		MOVE.B	D3,HDP_CYL+INIT_HDP(A2)
		MOVE.B	HDP_HEADHICYL+INIT_HDP(A2),D0
		AND.B	#$F0,D0
		LSR.L	#8,D3
		OR.B	D3,D0
		MOVE.B	D0,HDP_HEADHICYL+INIT_HDP(A2)
NO_PARK1:	CMP.B	#NUMUNITS,HDU_UNIT(A3)	; If SCSI drive,
		BGE.S	InIsSCSI		;	don't issue command
		LEA	HD_MP(A6),A0		; Point to message port
		LEA	INIT_IOR(A2),A1		; Point to IORequest
		LINKSYS	PutMsg,HD_SYSLIB(A6)	; Pass to driver task
		MOVE.L	#HDF_OWNIO,D0		; Wait for completion
		LINKSYS	Wait,HD_SYSLIB(A6)

*	Now compute last block on drive

InIsSCSI:	MOVEQ.L	#0,D0
		MOVE.L	HDB_CYLINDERS+INIT_SECT(A2),D3 ; Get # of cylinders
		MOVE.B	HDP_HEADHICYL+INIT_HDP(A2),D0	; Get # of heads
		LSR.B	#4,D0
		MOVEQ.L	#0,D1
		MOVE.B	HDP_SECTORS+INIT_HDP(A2),D1
		MULU	D1,D0			; Multiply by # of sectors/track
		MULU	D3,D0			; Multiply by park cylinder #
		SUBQ.L	#1,D0			; Block numbers start at 0
		MOVE.L	D0,HDU_LAST(A3)		; Save in unit structure

*	Now compute park block number

		MOVEQ.L	#0,D0
		MOVE.L	HDB_PARK+INIT_SECT(A2),D3; Get PARK cylinder
		BEQ.S	NO_PARK2		; If zero, DON'T PARK
		BMI.S   NO_PARK2

		MOVE.B	HDP_HEADHICYL+INIT_HDP(A2),D0	; Get # of heads
		LSR.B	#4,D0
		MOVEQ.L	#0,D1
		MOVE.B	HDP_SECTORS+INIT_HDP(A2),D1
		MULU	D1,D0			; Multiply by # of sectors/track
		SUBQ.L	#1,D3			; Re-adjust from before
		MULU	D3,D0			; Multiply by park cylinder #
NO_PARK2:	MOVE.L	D0,HDU_PARK(A3)		; Save in unit structure
		MOVE.L	(SP)+,D3
		RTS

*----------------------------------------------------------------------
*
* Definitions for Device Library Initialization
*
*----------------------------------------------------------------------


InitByte	MACRO	* &offset,&value
			DC.B	$e0
			DC.B	0
			DC.W	\1
			DC.B	\2
			DC.B	0
			ENDM

InitWord	MACRO	* &offset,&value
			DC.B	$d0
			DC.B	0
			DC.W	\1
			DC.W	\2
			ENDM

InitLong	MACRO	* &offset,&value
			DC.B	$c0
			DC.B	0
			DC.W	\1
			DC.L	\2
			ENDM
			

		CNOP	0,4	; MUST BE LONG WORD ALIGNED?
		XDEF devStructInit
devStructInit:
*		;------ Initialize the device

		InitByte	LN_TYPE,NT_DEVICE
dev_Name1:
*       ; initialize this absolute manually for relocatability -- bart
*		InitLong	LN_NAME,hdName
		InitByte	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		InitWord	LIB_VERSION,VERNUM
		InitWord	LIB_REVISION,REVNUM
		DC.L		0

		CNOP	0,4	; MUST BE LONG WORD ALIGNED?
		XDEF HDStructInit
HDStructInit:
*		;------ Initialize the device
		InitByte	HD_IS+LN_PRI,4	; Int priority 4
dev_ICode:
*       ; initialize this absolute manually for relocatability -- bart
*		InitLong	HD_IS+IS_CODE,hdintr	; Interrupt routine addr
dev_Name2:
*       ; initialize this absolute manually for relocatability -- bart
*		InitLong	HD_IS+LN_NAME,hdName
		InitByte	HD_MP+LN_TYPE,NT_MSGPORT;
dev_Name3:
*       ; initialize this absolute manually for relocatability -- bart
*		InitLong	HD_MP+LN_NAME,hdName
dev_Name4:
*       ; initialize this absolute manually for relocatability -- bart
*		InitLong	HD_TCB+LN_NAME,hdName
		InitByte	HD_TCB+LN_TYPE,NT_TASK
		InitByte	HD_TCB+LN_PRI,5
		DC.L		0

		CNOP	0,4	; MUST BE LONG WORD ALIGNED?

FUNCDEF MACRO
		XREF	_\1
		DC.W    \1+(*-devFuncInit)
		ENDM

		XDEF devFuncInit
devFuncInit:
		DC.W	-1			;--- use relative word offsets
open_vect:
		FUNCDEF	Open
close_vect:
		FUNCDEF	Close
expunge_vect:
		FUNCDEF	Expunge
reserved_vect:
		FUNCDEF	Null
beginio_vect:
		FUNCDEF	BeginIO
abortio_vect:
		FUNCDEF	Null
bad_vect:
		DC.W	-1

cmdTable:
		BRA		NoIO		;  0 invalid
		BRA		NoIO		;  1 reset
		BRA		HDIO		;  2 read
		BRA		HDIO		;  3 write
		BRA		HDUUpdate	;  4 update
		BRA		HDUClear	;  5 clear
		BRA		NoIO		;  6 stop
		BRA		NoIO		;  7 start
		BRA		NoIO		;  8 flush
		BRA		HDUMotor	;  9 motor
		BRA		HDUSeek		;  a seek
		BRA		HDUFormat	;  b format
		BRA		HDURemove	;  c remove
		BRA		HDUChangeNum	;  d changenum
		BRA		HDUChangeState	;  e changestate
		BRA		HDUProtStatus	;  f protstatus
		BRA		HDUSpecial		; 10 SPECIAL direct IO
		BRA		HDUMovCmdBlk	; 11 Move the CMD BLK to DEV STRUCT
		BRA		NoIO			; 12 reserved
		BRA		NoIO			; 13 reserved
		BRA		NoIO			; 14 reserved
		BRA		NoIO			; 15 reserved
		BRA		NoIO			; 16 reserved
		BRA		NoIO			; 17 reserved
		BRA		NoIO			; 18 reserved
		BRA		NoIO			; 19 reserved
		BRA		NoIO			; 1a reserved
		BRA		NoIO			; 1b reserved
		BRA		HDUSCSI			; 1c SCSI direct IO

	END
