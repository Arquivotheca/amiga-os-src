	LLEN	130
	PLEN	60
	LIST

*************************************************************************
*									*
*	Copyright (C) 1986, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* init.asm
*
* Source Control
* ------ -------
* 
* $Header: init.asm,v ??.? 86/05/16 10:00:00 lce Exp $
*
* $Locker:  $
*
* $Log:	driver.asm,v $
* Revision ??.?  86/05/16  10:00:00  LCE
* Convert to Commodore Amiga Hard Disk
* 
* *** empty log message ***
* 
* 
*************************************************************************

	SECTION section

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
	INCLUDE 'hddisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE	'libraries/expansion.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'
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
	XREF	HDUMovCmdBlk

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
		MOVEM.L	D2-D6/A2-A5,-(SP)

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
		LEA	devFuncInit,A0
		LEA	devStructInit,A1
		SUB.L	A2,A2
		MOVE.L	#DV_SIZE,D0
		CALLSYS MakeLibrary

		TST.L	D0
		BNE.S	Init_Success

Init_Alert:
		ALERT	(AN_TrackDiskDev!AG_MakeLib),,A0

Init_Success:
		MOVE.L	D0,D6			; Save device structure address
*		;------ Add the device
		PUTMSG	50,<'%s/Init: About to add device'>
		MOVE.L	D0,A1
		CALLSYS AddDevice


		IFD	AUTOINST		; If Autoconfig software used
		LEA.L	ExLibName,A1		; Get expansion lib. name
		CLR	D0
		CALLSYS	OpenLibrary		; Open the expansion library
		TST.L	D0
		BNE.S	Init_OpSuccess

Init_OpFail:	ALERT	(AN_AMHDDiskDev!AG_OpenLib),,A0

Init_OpSuccess:	MOVE.L	D0,A4
		MOVEQ	#0,D3			; Index to 1st controller
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
		MOVE.L	INIT_CADDR(A5),A0	; Restore pointer

Just_Disk:
		MOVE.L	cd_BoardAddr(A0),INIT_BADDR(A5); Get Board Base addess
		BCLR.B	#CDB_CONFIGME,cd_Flags(A0); Mark board as configured
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
		MOVEQ.L	#0,D0			; No need to re-zero
		LEA	HDStructInit,A1		; Initialization structure
		CALLSYS	InitStruct		; A2 already points to HD struct

		MOVE.L	D6,A1			; Get address of DV structure
		MOVE.L	A2,DV_0(A1,D3)		; Save this HD struct address
		ADDQ.L	#4,D3			; Bump index for next controller

		LEA.L	IntuitLibName,A1	; Get intuition lib. name
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
		LEA	hdintr,A0
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
		BEQ.S	Init_Sx			; If not set, no SCSI chip
		MOVEQ	#2,D2			; Try and init UNIT 3 (SCSI)
		MOVEQ	#1,D5			; De-allocate UNIT if fails
		MOVEQ	#0,D4			; FLAGS
		BSR	InitUnit
		TST.L	D0			; If init failed,
		BNE.S	Init_Sx			;	Skip
InitScsiOK:	MOVE.L	A0,HDUS0(A6)		;	save SCSI unit pointer

Init_Sx:
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
		MOVEM.L	(SP)+,D2-D6/A2-A5
		RTS

*    InitUnit:
*    --------
*
*	A6 --> device pointer
*	D2 --> unit number
*	D4 --> unit FLAG byte
*	D5 --> Non-zero means deallocate memory if open fails
*	D6,D7 --> Scratch

		
InitUnit:
	MOVEM.L	D2/D4/D6/D7/A2/A4/A5,-(SP)
*		;------ get temp memory
		MOVE.L	#INIT_SIZE,D0
		MOVE.L	#MEMF_CLEAR!MEMF_PUBLIC,D1
		LINKSYS	AllocMem
		TST.L	D0
		BEQ	InitUnit_NoMem
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
		MOVE.B	#0,CMD_LUNHIADDR(A1)
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
		BEQ.S	INotSCSI		;	try and read
		CMP.B	#$FE,CMD_ERRORBITS(A1)	; If select TIMEOUT,
		BEQ	BAD_MAGIC		;	assume no drive
		CMP.B	#$F8,CMD_ERRORBITS(A1)	; If drive not ready
		BEQ	WAIT_READY		;	wait till ready

*	Now read in 1st readable block for this unit

*		Tell HDIO to pass this command directly to the controller
INotSCSI:	MOVE.W	#HD_SPECIAL,IO_COMMAND(A1)
		LEA	INIT_CMD(A2),A1		; Point to internal command blk
		MOVE.L	A1,IO_DATA+INIT_IOR(A2)
		MOVE.B	#HDC_READ,CMD_OPCODE(A1); Set to "read"
		MOVE.W	#$0000,CMD_MIDADDR(A1)	; Block 0
		MOVE.L	#30,D6			; Retry up to 30 times
INFO_Retry:	MOVE.B	#$01,CMD_BLOCKCNT(A1)	; 1 block
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
		BRA.S	OPEN_BB_DONE

BAD_MAGIC:	; Jump here if don't know what type of drive it is

		MOVE.W	#0,BBR_COUNT+HDU_BB(A3) ; Zero count of blocks
		MOVE.L	#$7FFFFFFF,BBR_TABLE+BBM_BAD+HDU_BB(A3)
		MOVE.L	#$7FFFFFFF,BBR_TABLE+BBM_GOOD+HDU_BB(A3)
		MOVE.L	D5,D0		; If zero, means open no matter what
		IFD	junk
		TST.L	D5
		BEQ	InitUnit_End		
; This was a conditional init that failed, so free the unit's memory
		MOVE.L	HDU_ENTRY(A3),A1
		MOVE.L	#HDU_SIZE,D0
		LINKSYS	FreeMem
		MOVE.L	D5,D0
		ENDC
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
		MOVE.B	HDU_HEADS(A3),D1 ; is # heads specified ?
		BEQ.S	1$		;no, HDU_SECTORS must be sectors/cyl
	 
		MULU	D1,D0		; D0 now contains # of sectors/cyl
1$		LSR.W	#1,D0		; Assume two cylinders reserved
		SUBQ.W	#3,D0		; 3 blocks used
		MOVE.W	D0,BBR_LEFT+HDU_BB(A3) ; Show as number of free blocks
		MOVE.L	#3,BBR_NEXT_FREE+HDU_BB(A3) ; 3 is next free block;
OPEN_BB_DONE:

* Now tell AmigaDOS about 1st partition on the disk

		IFD	AUTOINST		; If Autoconfig software used
		LEA	hdName,A0		; Get address of driver name
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
		MOVE.L	D3,D0		; Get (controller # + 1) * 4
		LSR.L	#2,D0		; Divide by 4
		SUBQ.L	#1,D0
		MULU	#10,D0		
		MOVEQ.L	#0,D1
		MOVE.B	HDU_UNIT(A3),D1
		ADD.L	D1,D0
		ADDQ.L	#1,D0
		MOVE.L	D0,HDB_UNITNO+INIT_SECT(A2) ; and save in environment

		LEA	HDB_DOSNPTR+INIT_SECT(A2),A0
		LINKLIB	_LVOMakeDosNode,A4	; Build AmigaDOS structures
		MOVE.L	D0,A0			; Get deviceNode address
		MOVEQ.L	#0,D0			; Set device priority to 0
		MOVEQ.L	#0,D1
*		MOVEQ.L	#ADNF_STARTPROC,D1	; Start handler : DOESN'T WORK!!
		LINKLIB	_LVOAddDosNode,A4
		ENDC

		CLEAR	D0

InitUnit_End:
		MOVE.L	D0,A4			; Preserve D0

		MOVE.L	A2,A1
		MOVE.L	#INIT_SIZE,D0
		LINKSYS	FreeMem
		MOVE.L	A4,D0
		MOVE.L	A3,A0
		MOVEM.L	(SP)+,D2/D4/D6/D7/A2/A4/A5
		RTS

InitUnit_NoMem:
		IFGE	INFO_LEVEL-30
		PUTMSG	0,<'%s/InitUnit: Can not allocate memory'>
		ENDC

		MOVEQ	#HDERR_NoMem,D0
		BRA.S	InitUnit_End

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
		InitLong	LN_NAME,hdName
		InitByte	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		InitWord	LIB_VERSION,VERNUM
		InitWord	LIB_REVISION,REVNUM
		DC.L		0

		CNOP	0,4	; MUST BE LONG WORD ALIGNED?
		XDEF HDStructInit
HDStructInit:
*		;------ Initialize the device

		InitByte	HD_IS+LN_PRI,4	; Int priority 4
		InitLong	HD_IS+IS_CODE,hdintr	; Interrupt routine addr
		InitLong	HD_IS+LN_NAME,hdName
		InitByte	HD_MP+LN_TYPE,NT_MSGPORT;
		InitLong	HD_MP+LN_NAME,hdName		
		InitLong	HD_TCB+LN_NAME,hdName		
		InitByte	HD_TCB+LN_TYPE,NT_TASK
		InitByte	HD_TCB+LN_PRI,5
		DC.L		0

		CNOP	0,4	; MUST BE LONG WORD ALIGNED?
*		;---!!! Must fix the absolute function posistions.  How?
		XDEF devFuncInit
devFuncInit:
		DC.L	Open		; - 4
		DC.L	Close		; - 8
		DC.L	Expunge		; -10
		DC.L	Null		; -14	; reserved
		DC.L	BeginIO		; -18
		DC.L	Null		; -1c
		DC.L	-1

cmdTable:
		DC.L	NoIO		;  0 invalid
		DC.L	NoIO		;  1 reset
		DC.L	HDIO		;  2 read
		DC.L	HDIO		;  3 write
		DC.L	HDUUpdate	;  4 update
		DC.L	HDUClear	;  5 clear
		DC.L	NoIO		;  6 stop
		DC.L	NoIO		;  7 start
		DC.L	NoIO		;  8 flush
		DC.L	HDUMotor	;  9 motor
		DC.L	HDUSeek		;  a seek
		DC.L	HDUFormat	;  b format
		DC.L	HDURemove	;  c remove
		DC.L	HDUChangeNum	;  d changenum
		DC.L	HDUChangeState	;  e changestate
		DC.L	HDUProtStatus	;  f protstatus
		DC.L	HDUSpecial	; 10 SPECIAL direct IO
		DC.L	HDUMovCmdBlk	; 11 Move the CMD BLK to DEV STRUCT

	END
