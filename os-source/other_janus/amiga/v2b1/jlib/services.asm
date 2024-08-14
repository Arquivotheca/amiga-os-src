
* *** services.asm ************************************************************
* 
* Service Routines  --  Janus Library
* 
* Copyright (C) 1988, Commodore-Amiga, Inc.
* 
* CONFIDENTIAL and PROPRIETARY
* 
* Date       Name           Description
* ---------  -------------  ---------------------------------------------------
* 15-Jul-88  -RJ            Changed all files to work with new includes
* 8-Feb-88   -RJ            Got the code in this file to work
* 7-Feb-88   =RJ Mical=     Created this file!
* 
* *****************************************************************************


	INCLUDE "assembly.i"


	NOLIST
	INCLUDE "exec/types.i"
 	INCLUDE "exec/nodes.i"
 	INCLUDE "exec/lists.i"
	INCLUDE "exec/execbase.i"
 	INCLUDE "exec/memory.i"
 	INCLUDE "exec/tasks.i"
 	INCLUDE "exec/alerts.i"

	INCLUDE "janus/janusbase.i"
	INCLUDE "janus/memory.i"
	INCLUDE "janus/services.i"

 	INCLUDE "serviceint.i"
	LIST

	INCLUDE "asmsupp.i"

	XDEF	InitService


	XLIB	AddTask
	XLIB	Alert
	XLIB	AllocMem
	XLIB	AllocSignal
	XLIB	Forbid
	XLIB	FreeMem
;NOSEMS	XLIB	InitSemaphore
	XLIB	OpenLibrary
;NOSEMS	XLIB	ObtainSemaphore
	XLIB	Permit
;NOSEMS	XLIB	ReleaseSemaphore
	XLIB	Wait

	XLIB	WaitTOF

	XLIB	AllocJanusMem
	XLIB	JanusMemToOffset
	XLIB	SetParamOffset
	XLIB	SetupJanusSig


	XREF	_AbsExecBase


 IFD	CHANNELING
	XREF	ChannelMatch
 ENDC	; of IFD CHANNELING
	XREF	FindPCToA
 IFND	CHANNELING
	XREF	FindPCAdds
	XREF	FindPCDeletes
 ENDC	; of IFND CHANNELING
	XREF	InformDeletedService
	XREF	InformNewCustomers
	XREF	SignalCustomers

	XLIB	JanusOffsetToMem
	XLIB	MakeBytePtr
	XLIB	MakeWordPtr


* The ServiceTask stack size
SERVICETASK_STACKSIZE	EQU	2000




InitService:
* =============================================================================
* Here we insert the new Services initialization stuff, which will set up 
* necessary the data structures and mint the services task.
* RJM:  7 Feb 1988
* 
* ENTRY:
*    A2 = address of JanusAmiga
* 
* EXIT:
*    A2 = address of JanusAmiga
* =============================================================================

		MOVEM.L	A2-A6/D2-D7,-(SP)

	PUTMSG	1,<'%s/InitService'>

		MOVE.L	jb_ExecBase(A2),A6

		;------ Allocate the memory for the ServiceBase, which is the 
		;------ data base required by the Amiga-side of Janus services
		MOVE.L	#ServiceBase_SIZEOF,D0
		MOVE.L	#MEMF_CLEAR,D1
		CALLSYS	AllocMem
		MOVEA.L	D0,A3
		MOVE.L	D0,jb_ServiceBase(A2)
		BEQ	initError


;NOSEMS		;------ Initialize the ServiceBase Semaphore
;NOSEMS		MOVE.L	D0,A0
;NOSEMS		LEA	sb_ServiceSemaphore(A0),A0
;NOSEMS		CALLSYS	InitSemaphore


		;------ CreateTask(name, priority, routine, stacksize);
		MOVE.L	#SERVICETASK_STACKSIZE,-(SP)
		LEA.L	ServiceTask(PC),A0
		MOVE.L	A0,-(SP)
		MOVE.L	#0,-(SP)
		LEA.L	ServiceTaskName(PC),A0
		MOVE.L	A0,-(SP)
		JSR	CreateTask
		LEA	4*4(SP),SP
		MOVE.L	D0,sb_ServiceTask(A3)
		BEQ	initError


		BRA	RETURN


initError:
		ALERT	$7ffffff0

RETURN:
		MOVEM.L	(SP)+,A2-A6/D2-D7
		RTS



ServiceTask:
* *****************************************************************************
* Ah, the ServiceTask.  This little puppy hasn't much to do, at least 
* not today anyway.  It sets itself up as the grand and glorious watcher 
* of the JSERV_AMIGASERVICE interrupt, and then just sort of hangs around 
* waiting for some action.  When it gets signalled, it makes various Janus 
* Service calls as appropriate.  
* 
* Currently never dies, because janus.library never closes.
* *****************************************************************************
	PUTMSG	1,<'%s/ServiceTask'>

		MOVE.L	_AbsExecBase,A2
		MOVE.L	A2,A6
		LEA.L	GraphicsName(PC),A1
		MOVEQ.L	#0,D0
		CALLSYS OpenLibrary
		MOVE.L	D0,A3
		MOVE.L	A3,D0
		BEQ	error


tryAgain:
	PUTMSG	2,<'%s/ServiceTask trying again...'>
		MOVE.L	A3,A6
		CALLSYS	WaitTOF


		MOVE.L	A2,A6
		CALLSYS	Forbid

		LEA.L	LibList(A6),A0	; A0 has the pointer to the first node pointer
		MOVE.L	(A0),A0		; A0 has the pointer to the first node

NodeLoop:
		MOVE.L	LN_NAME(A0),A1
		MOVEQ.L	#0,D0
		JSR	januscheck
		TST.B	D0
		BEQ	tryOpen

		MOVE.L	(A0),A0
		MOVE.L	A0,D0
		BNE	NodeLoop

		MOVE.L	A2,A6
		CALLSYS	Permit
		BRA	tryAgain


tryOpen:
		MOVE.L	A2,A6
		CALLSYS	Permit

		LEA.L	JanusName(PC),A1
		MOVEQ.L	#0,D0

	IFGE	INFOLEVEL-1
	MOVE.L	A6,-(SP)
	MOVE.L	D0,-(SP)
	MOVE.L	A1,-(SP)
	PUTMSG	2,<'%s/Before OpenLibrary( 0x%lx 0x%lx ) using 0x%lx'>
	LEA.L	12(SP),SP
	ENDC

		CALLSYS OpenLibrary
	PUTMSG	2,<'%s/After OpenLibrary()'>
		MOVE.L	D0,A6
		MOVE.L	A6,D0
		BEQ	tryAgain


		;------ Below, A2/A6 will have JanusBase and ExecBase,
		;------	A3 will have address of GfxBase
		;------ A4 will have ServiceBase
		MOVE.L	jb_ServiceBase(A6),A4
		MOVE.L	A3,sb_GfxBase(A4)


		;------ Set up the Amiga Service signal
		EXG.L	A2,A6		; Get ExecBase into A6
		MOVE.L	#-1,D0		; Get a signal, don't bother to
		CALLSYS	AllocSignal	; check for error as SetupSig() will
		MOVE.L	D0,sb_TaskSigNum(A4)
		MOVE.L	D0,D1
		MOVE.L	#JSERV_AMIGASERVICE,D0	; Amiga service 
 		MOVE.L	#0,D2			; Parameter size
		MOVE.L	#0,D3			; Parameter type
		EXG.L	A2,A6			; Get JanusBase into A6
		CALLSYS	SetupJanusSig
		MOVE.L	D0,sb_SetupSig(A4)
		BEQ	error


		;------	Get the parameter block for this interrupt, which 
		;------	block we want to be sized an even number of bytes
 		MOVE.L	#ServiceParam_SIZEOF,D0	;Parameter size
		ADDQ.W	#1,D0			; Increment it to round up
		AND.B	#$FE,D0			; Mask off any excess
		MOVE.W	D0,D2			; Grab a copy for later
		MOVE.L	#MEMF_PARAMETER+MEM_WORDACCESS,D1   ; Parameter type
		CALLSYS	AllocJanusMem
		TST.L	D0
		BEQ	error


		;------	Initialize the ServiceParam structure
		;------ (Actually, the below cheats a bit with writing 
		;------	bytes through a word-access pointer, which should 
		;------	be safe because we made the structure even-sized)
		MOVEA.L	D0,A0
ClearParam:	MOVE.B	#0,(A0)+
		DBRA	D2,ClearParam


		;------	Preset some ServiceParam fields
		MOVEA.L	D0,A0
		MOVEQ.L	#-1,D1
		MOVE.W	D1,spm_FirstServiceData(A0) ; No service data
 IFD	CHANNELING
		;------	The following presumes that the first 16 channels 
		;------	are reserved, which indeed they are
		MOVE.W	D1,spm_ChannelMasks(A0)
 ENDC	; of IFD CHANNELING
 IFND	CHANNELING
		;------	The following presumes that the AmigaToPC array is 
		;------	comprised of 4 contiguous 16-bit fields
		MOVE.L	D1,spm_AmigaToPC(A0)
		MOVE.L	D1,spm_AmigaToPC+4(A0)
		;------	The following presumes that the PCToAmiga array is 
		;------	comprised of 4 contiguous 16-bit fields
		MOVE.L	D1,spm_PCToAmiga(A0)
		MOVE.L	D1,spm_PCToAmiga+4(A0)
		;------	The following presumes that the Adds and Deletes arrays 
		;------	are comprised of 2 contiguous 16-bit fields each
		MOVE.L	D1,spm_AmigaAddsService(A0)
		MOVE.L	D1,spm_AmigaDeletesService(A0)
		MOVE.L	D1,spm_PCAddsService(A0)
		MOVE.L	D1,spm_PCDeletesService(A0)
 ENDC	; of IFND CHANNELING
		;------	Initialize the Lock
		CALLSYS	MakeBytePtr
		MOVE.B	#$7F,spm_Lock(A0)


		;------	Tell the Amiga that the ServiceParam is ready
		CALLSYS	MakeWordPtr
		MOVE.L	D0,sb_ServiceParam(A4)


		;------	Tell the PC that the ServiceParam is ready
		CALLSYS	JanusMemToOffset	; D0 still has address of struct
		MOVE.W	D0,D1
		MOVE.L	#JSERV_AMIGASERVICE,D0	; Amiga service 
		CALLSYS	SetParamOffset

		EXG.L	A2,A6		; Get ExecBase into A6

ServiceTaskLoop:
		MOVEQ.L	#0,D0
		MOVE.L	sb_TaskSigNum(A4),D1
		BSET	D1,D0
		CALLSYS	Wait
	PUTMSG	1,<'%s/ServiceTask has awakened'>
		JSR	ProcessPCToAmiga
		BRA	ServiceTaskLoop


error:
		ALERT	$7ffffff1
error2:
		MOVE.L	_AbsExecBase,A6
		MOVEQ.L	#0,D0
		CALLSYS	Wait
		BRA	error2



ProcessPCToAmiga:
* === Service Interrupt from PC ===============================================
* 
* VOID ProcessPCToAmiga(JanusBase, ServiceBase, ExecBase);
*                      A2         A4           A6
* 
* This is an interrupt handled by the Amiga Service Dispatcher Task
* 
* This algorithm is for the Amiga side, but the idea should be much the same 
* on the PC

PC_REGS	REG	A2-A6/D2-D7


		MOVEM.L	PC_REGS,-(SP)


	PUTMSG	2,<'%s/ProcessPCToAmiga()'>

		EXG.L	A2,A6


* Throughout this routine, except for local changes (which must be repaired
* afterwards), these registers will be constant:
*  IFD CHANNELING
*    D2.B = The channel number we're trying to match
*  ENDC ; of IFD CHANNELING
*    A2   = ExecBase
*    A4   = ServiceBase
*    A5   = ServiceParam.WA
*    A6   = JanusBase, for janus library
		MOVE.L	sb_ServiceParam(A4),A5


* - lock the ServiceBase semaphore
;NOSEMS		LEA	sb_ServiceSemaphore(A4),A0
		EXG.L	A2,A6
;NOSEMS		CALLSYS	ObtainSemaphore
		CALLSYS Forbid
		EXG.L	A2,A6


* - lock the ServiceParam lock
		MOVE.L	A5,A0
		CALLSYS	MakeBytePtr
		LOCK	spm_Lock(A0)


PC_CHANNELCHECK:
* - find a non-zero ServiceData offset (if any) in PCAddsService[]
		JSR	FindPCAdds
		CMPA.L	#0,A0
		BEQ	pcAddsNone
		MOVE.W	(A0),D0			; else get the offset
		MOVE.W	#-1,(A0)
		MOVE.W	#MEMF_PARAMETER+MEM_WORDACCESS,D1 ; Get the mem type
		CALLSYS	JanusOffsetToMem	; turn it into a real pointer

	IFGE	INFOLEVEL-2
	MOVE.L	A0,-(SP)
	PUTMSG	2,<'%s/AddService from PC, ServiceData address=$%lx'>
	LEA.L	4(SP),SP
	XREF	validateServiceData
	JSR	validateServiceData
	ENDC	; of IFGE INFOLEVEL-2

		JSR	ProcessAdd
		BRA	PC_CHANNELCHECK		; And start again from the top
pcAddsNone:


* - find a non-zero ServiceData offset (if any) in PCDeletesService[]
		JSR	FindPCDeletes
		CMPA.L	#0,A0
		BEQ	pcDeletesNone
		MOVE.W	(A0),D0			; else get the offset
		MOVE.W	#-1,(A0)
		MOVE.W	#MEMF_PARAMETER+MEM_WORDACCESS,D1 ; Get the mem type
		CALLSYS	JanusOffsetToMem	; turn it into a real pointer

	IFGE	INFOLEVEL-2
	MOVE.L	A0,-(SP)
	PUTMSG	2,<'%s/DeleteService from PC, ServiceData address=$%lx'>
	LEA.L	4(SP),SP
	XREF	validateServiceData
	JSR	validateServiceData
	ENDC	; of IFGE INFOLEVEL-2

		JSR	ProcessDelete
		BRA	PC_CHANNELCHECK		; And start again from the top
pcDeletesNone:


* - find a non-zero ServiceData offset (if any) in PCToAmiga[]
		JSR	FindPCToA

	IFGE	INFOLEVEL-59
	MOVEM.L D0-D3,-(SP)
	MOVEQ.L #0,D0
	MOVE.W	spm_PCToAmiga(A5),D0
	MOVEQ.L #0,D1
	MOVE.W	spm_PCToAmiga+2(A5),D1
	MOVEQ.L #0,D2
	MOVE.W	spm_PCToAmiga+4(A5),D2
	MOVEQ.L #0,D3
	MOVE.W	spm_PCToAmiga+6(A5),D3
	MOVEM.L D0-D3,-(SP)
	MOVE.L	A0,-(SP)
	PUTMSG	59,<'%s/FindPCToA=$%lx fields= $%04lx $%04lx $%04lx $%04lx'>
	LEA.L	5*4(SP),SP
	MOVEM.L (SP)+,D0-D3
	ENDC	; of IFGE	INFOLEVEL-59

		CMPA.L	#0,A0
		BEQ	PC_DONE			; We must be *all* done
		MOVE.W	(A0),D0			; else get the offset
		MOVE.W	#-1,(A0)
		MOVE.W	#MEMF_PARAMETER+MEM_WORDACCESS,D1 ; Get the mem type

	IFGE	INFOLEVEL-59
	ANDI.L	#$0000FFFF,D0
	MOVE.L	D0,-(SP)
	PUTMSG	59,<'%s/CallService from PC, ServiceData offset=%$%lx'>
	LEA.L	4(SP),SP
	ENDC	; of IFGE	INFOLEVEL-59

		CALLSYS	JanusOffsetToMem	; turn it into a real pointer

	IFGE	INFOLEVEL-2
	MOVE.L	A0,-(SP)
	PUTMSG	2,<'%s/CallService from PC, ServiceData address=$%lx'>
	LEA.L	4(SP),SP
	XREF	validateServiceData
	JSR	validateServiceData
	ENDC	; of IFGE INFOLEVEL-2

		JSR	SignalCustomers		; and go signal the customers

		BRA	PC_CHANNELCHECK		; And start again from the top


PC_ERROR:
	IFGE	INFOLEVEL-59
 IFD	CHANNELING
	ANDI.L	#$000000FF,D2
	MOVE.L	D2,-(SP)
	PUTMSG	59,<'%s/ProcessPCToAmiga:  channel %ld not found'>
 ENDC	; of CHANNELING
 IFND	CHANNELING
	MOVE.L	D2,-(SP)
	PUTMSG	59,<'%s/ProcessPCToAmiga:  ServiceData $%lx not found'>
 ENDC	; of not CHANNELING
	LEA	1*4(SP),SP
	ENDC

		BRA	PC_CHANNELCHECK


PC_DONE:
	PUTMSG	59,<'%s/ProcessPCToAmiga almost done'>

* - unlock the ServiceParam lock
		MOVE.L	A5,A0
		CALLSYS	MakeBytePtr
		UNLOCK	spm_Lock(A0)

* - unlock the ServiceBase semaphore
;NOSEMS		LEA	sb_ServiceSemaphore(A4),A0
		EXG.L	A6,A2
;NOSEMS		CALLSYS	ReleaseSemaphore
		CALLSYS	Permit
		EXG.L	A6,A2

	PUTMSG	59,<'%s/ProcessPCToAmiga done'>

		MOVEM.L	(SP)+,PC_REGS
		RTS



ProcessAdd:
* === Service Interrupt PCAddsService =========================================
* This is an interrupt handled by the Amiga Service Dispatcher Task
* 
* This algorithm is for the Amiga side, but the idea should be much the same 
* on the PC
* 
*    A0   = Address of the ServiceData.WA that's been added by the PC
*    A2   = ExecBase
*    A4   = ServiceBase
*    A5   = ServiceParam.WA
*    A6   = JanusBase, for janus library

	PUTMSG	2,<'%s/ProcessAdd:'>

		MOVEM.L	A3,-(SP)
		MOVEA.L	A0,A3


* - lock the ServiceBase sempahore (note:  already done by caller)
* - lock the ServiceParam lock (note:  already done by caller)

* NOPE:  don't set Amiga address anymore, as PC does it for us
* - Set the pointer to Amiga memory
* 		MOVE.W	sd_MemOffset(A3),D0
* 		MOVE.W	sd_MemType(A3),D1
* 		CALLSYS	JanusOffsetToMem
* 		MOVE.L	A0,sd_AmigaMemPtr(A3)
* 		MOVE.W	sd_Flags(A3),D0
* 		BSET	#AMIGA_MEMPTRn,D0
* 		MOVE.W	D0,sd_Flags(A3)


* - InformNewCustomers(ServiceData)
		MOVE.L	A3,A0
		MOVEQ.L	#0,D0		; We aren't calling from AddService()
		JSR	InformNewCustomers

		BRA	ADD_DONE


ADD_ERROR:
	PUTMSG	2,<'%s/ADD_ERROR:'>

ADD_DONE:


* - unlock the ServiceParam lock  (note:  will be done by caller)
* - unlock the ServiceBase sempahore  (note:  will be done by caller)

		MOVEM.L	(SP)+,A3

		RTS



ProcessDelete:
* === Service Interrupt of DELETE_SERVICE =====================================
* This is an interrupt handled by the Amiga Service Dispatcher Task
* 
* This algorithm is for the Amiga side, but the idea should be much the same 
* on the PC
* 
* This algorithm checks *all* services and sends a signal to all that are 
* deleted, which redundancy is left in for its safety-margin value
* 
*    A0   = Address of the ServiceData.WA that's been deleted by the PC
*    A2   = ExecBase
*    A4   = ServiceBase
*    A5   = ServiceParam.WA
*    A6   = JanusBase, for janus library


	PUTMSG	2,<'%s/ProcessDELETE:'>

		MOVEM.L	A3,-(SP)
		MOVEA.L	A0,A3


* - lock the ServiceBase sempahore (note:  already done by caller)
* - lock the ServiceParam lock (note:  already done by caller)

* - traverse the ServiceData list, and if the SERVICE_DELETED flag is set
* 	- InformDeletedService(ServiceData)
		MOVE.W	spm_FirstServiceData(A5),D0
DELETE_LOOP:	CMP.W	#-1,D0
		BEQ	DELETE_DONE
		MOVE.W	#MEMF_PARAMETER+MEM_WORDACCESS,D1
		CALLSYS	JanusOffsetToMem
		MOVE.W	sd_Flags(A0),D1
		BTST	#SERVICE_DELETEDn,D1
		BEQ	NOT_DELETED

		MOVE.L	A0,-(SP)
		JSR	InformDeletedService
		MOVE.L	(SP)+,A0

NOT_DELETED:	MOVE.W	sd_NextServiceData(A0),D0
		BRA	DELETE_LOOP


DELETE_DONE:
* - unlock the ServiceParam lock  (note:  will be done by caller)
* - unlock the ServiceBase sempahore  (note:  will be done by caller)

		MOVEM.L	(SP)+,A3

		RTS



januscheck:
* Expects string in A1, returns D0.B clear if equal to JanusName, 
* set if not equal, so after the call you can TST and then 
* BEQ or BNE with syntactic correctitude.
* The routine actually cheats and matches only as many characters as are 
* in JanusName, which means that it will match 
* both 'janus.library' and 'janus.libraryxjkhjlkzasfjlkhaioqwuerofiuqertiu'
* No register other than D0.B is disturbed.
	MOVEM.L	A0/A1,-(SP)
	LEA.L	JanusName(PC),A0
again:
	MOVE.B	(A0)+,D0	; Get the next byte of JanusName
	BEQ	checkend	; if null, we reached the end with a match!
	SUB.B	(A1)+,D0	; Subtract next character of string 
	BEQ	again		; with zero result if they were equal
				; ... else fall with non-zero into return
checkend:
	MOVEM.L	(SP)+,A0/A1
	RTS



CreateTask:
* My own CreateTask, to take away one extra unknown
* 
* On entry, on stack, expects:
*     - Pointer to name string (or NULL)
*     - Task initial priority
*     - Take initial execution address
*     - Stack size for new task
*
* Returns:
*     - If task created, address of task block
*     - If any error, returns NULL

ARGS	EQU	(11*4+4)
		MOVEM.L	A2-A6/D2-D7,-(SP)

		MOVE.L	ARGS+0(SP),A2	; Name
		MOVE.L	ARGS+4(SP),D2	; Priority
		MOVE.L	ARGS+8(SP),A3	; Initial PC
		MOVE.L	ARGS+12(SP),D3	; Stack size

		MOVE.L	#0,A4


		;------ Allocate the memory for the task structure
		MOVE.L	_AbsExecBase,A6
		MOVE.L	#TC_SIZE,D0
		MOVE.L	#MEMF_CLEAR,D1
		CALLSYS	AllocMem
		MOVE.L	D0,A4
		MOVE.L	A4,D0
		BEQ	createError


		;------ Allocate the user stack
		ADD.L	#3,D3		; Round the memory request up to 
		ANDI.L	#$FFFFFFFC,D3	; the nearest longword byte count
		MOVE.L	D3,D0
		MOVE.L	#MEMF_CLEAR,D1
		CALLSYS	AllocMem
		MOVE.L	D0,A5
		MOVE.L	A5,D0
		BEQ	createError


		;------ Set up the stack pointers in the task control block 
		MOVE.L	A5,TC_SPLOWER(A4)
		ADD.L	D3,A5
		SUB.L	#4,A5			; This can't be right
		MOVE.L	A5,TC_SPUPPER(A4)
		MOVE.L	A5,TC_SPREG(A4)


		;------ Also, initialize the node with the supplied information
		MOVE.B	#NT_TASK,LN_TYPE(A4)
		MOVE.B	D2,LN_PRI(A4)
		MOVE.L	A2,LN_NAME(A4)


		;------ Set up to call AddTask
		MOVE.L	A4,A1
		MOVE.L	A3,A2
		MOVE.L	#0,A3
		CALLSYS	AddTask


		;------ OK, we're successful, so return happily
		BRA	createDone


createError:
* Oops, error.  
* If A4 was set, free the memory
		MOVE.L	A4,D0
		BEQ	createDone
		MOVE.L	A4,A1
		MOVE.L	#TC_SIZE,D0
		CALLSYS	FreeMem


createDone:
		MOVE.L	A4,D0

		MOVEM.L	(SP)+,A2-A6/D2-D7
		RTS



ServiceTaskName:
		DC.B	'ZaphodServiceTask',0
		CNOP	0,2
JanusName:
		DC.B	'janus.library',0
		CNOP	0,2

GraphicsName:	DC.B	'graphics.library',0
		CNOP	0,2

	END



