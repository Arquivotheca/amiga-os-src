
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
* 7-Oct-88   -RJ            Added lots of little details regarding adding 
*                           EXCLUSIVE and ONLY stuff
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
;SEMS
	XLIB	InitSemaphore
	XLIB	OpenLibrary
;SEMS
	XLIB	ObtainSemaphore
	XLIB	Permit
;SEMS
	XLIB	ReleaseSemaphore
	XLIB	Wait

	XLIB	WaitTOF

	XLIB	AllocJanusMem
	XLIB	JanusMemToOffset
	XLIB	SetParamOffset
	XLIB	SetupJanusSig


	XREF	_AbsExecBase


	XREF	FindPCToA
	XREF	FindPCAdds
	XREF	FindPCDeletes
	XREF	InformDeletedService
	XREF	InformNewCustomers
	XREF	RunAutoloadBack
	XREF	SignalCustomers
	XREF	WaitForPC


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


;NOSEMS
		;------ Initialize the ServiceBase Semaphore
		MOVE.L	D0,A0
		LEA.L	sb_ServiceSemaphore(A0),A0
		CALLSYS	InitSemaphore
;NOSEMS

		;------ CreateTask(name, priority, routine, stacksize);
; 		MOVE.L	#SERVICETASK_STACKSIZE,-(SP)
; 		LEA.L	ServiceTask(PC),A0
; 		MOVE.L	A0,-(SP)
; 		MOVE.L	#0,-(SP)
; 		LEA.L	ServiceTaskName(PC),A0
; 		MOVE.L	A0,-(SP)
; 		JSR	CreateTask
; 		LEA	4*4(SP),SP

	MOVEM.L	D2-D4,-(SP)
	MOVE.L	#SERVICETASK_STACKSIZE,D4
	LEA.L	ServiceTask(PC),A0
	MOVE.L	A0,D3
	MOVE.L	#0,D2
	LEA.L	ServiceTaskName(PC),A0
	MOVE.L	A0,D1
	MOVEA.L	jb_DOSBase(A2),A0
	MOVEA.L	A6,A1
	JSR	CreateFakeProc
	SUB.L	#$5C,D0		; Turn process ptr into task ptr ??? It works?
	MOVEM.L	(SP)+,D2-D4

		MOVE.L	D0,sb_ServiceTask(A3)
		BNE	RETURN

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

		;------	Start up the autoloader
		JSR	RunAutoloadBack

		;------	Wait for the PC to come alive
		JSR	WaitForPC

		EXG.L	A2,A6		; Get ExecBase into A6

ServiceTaskLoop:
		MOVEQ.L	#0,D0
		MOVE.L	sb_TaskSigNum(A4),D1
		BSET	D1,D0
		CALLSYS	Wait
	PUTMSG	2,<'%s/ServiceTask has awakened'>
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
* === Service Interrupt from the other Processor ============================
* 
* VOID ProcessPCToAmiga(JanusBase, ServiceBase, ExecBase);
*                       A2         A4           A6
* 
* This algorithm should work for both processors
* 

PC_REGS	REG	A2-A6/D2-D7


		MOVEM.L	PC_REGS,-(SP)


	PUTMSG	2,<'%s/ProcessPCToAmiga()'>

		EXG.L	A2,A6


* Throughout this routine, except for local changes (which must be repaired
* afterwards), these registers will be constant:
*    A2   = ExecBase
*    A4   = ServiceBase
*    A5   = ServiceParam.WA
*    A6   = JanusBase, for janus library
		MOVE.L	sb_ServiceParam(A4),A5


* - if we're the Amiga lock the ServiceBase semaphore

		LEA.L	sb_ServiceSemaphore(A4),A0
		EXG.L	A2,A6
		CALLSYS	ObtainSemaphore
;SEMS		CALLSYS Forbid
		EXG.L	A2,A6


* - lock the ServiceParam lock
		MOVE.L	A5,A0
		CALLSYS	MakeBytePtr
		LOCK	spm_Lock(A0)


PC_CHANNELCHECK:
* - while we find a ServiceData offset in a 
*   PCAddsService[] {or AmigaAddsService[]} field
		JSR	FindPCAdds
		CMPA.L	#0,A0
		BEQ	pcAddsNone
		MOVE.W	(A0),D0			; else get the offset
*	- set the field to -1
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
* 	- ProcessAdd()
		JSR	ProcessAdd
		BRA	PC_CHANNELCHECK		; And start again from the top
pcAddsNone:

PC_DELETE_CHECK:
* - while we find a ServiceData offset in a 
*   PCDeletesService[] {or AmigaDeletesService[]} field
		JSR	FindPCDeletes
		CMPA.L	#0,A0
		BEQ	pcDeletesNone
		MOVE.W	(A0),D0			; else get the offset
*	- set the field to -1
		MOVE.W	#-1,(A0)
		MOVE.W	#MEMF_PARAMETER+MEM_WORDACCESS,D1 ; Get the mem type
		CALLSYS	JanusOffsetToMem	; turn it into a real pointer

	IFGE	INFOLEVEL-2
	MOVE.L	A0,-(SP)
	PUTMSG	2,<'%s/ DeleteService from PC, ServiceData address=$%lx'>
	LEA.L	4(SP),SP
	XREF	validateServiceData
	JSR	validateServiceData
	ENDC	; of IFGE INFOLEVEL-2

* 	- ProcessDelete()
		JSR	ProcessDelete
		BRA	PC_DELETE_CHECK		; And start again from the top
pcDeletesNone:

* - while we find a ServiceData offset in a PCToAmiga[] {or AmigaToPC[]} field
		JSR	FindPCToA

* OLD:	- set the field to -1
* NEW:	- rather than setting to -1 only the field that had the offset, 
* 	  check all fields in the PCToAmiga[] {or AmigaToPC[]} array 
* 	  for a match with the offset and set to -1 all fields that have 
* 	  the offset.  The effect of this is small but important:  
* 	  it speeds up Amiga <-> PC services requests a little bit 
* 	  because it stops redundant calls to SignalCustomers() (which, on 
* 	  the Amiga, is not a short routine, and before we're done may not 
* 	  be short on the PC either!)

* Amiga programmer, note that the above is already done for us by the call 
* to FindPCToA()

	IFGE	INFOLEVEL-59
	MOVEM.L A0/D0-D3,-(SP)
	AND.L	#$0000FFFF,D0
	MOVEA.L	D0,A0
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
	PUTMSG	59,<'%s/FindPCToA=$%04lx fields= $%04lx $%04lx $%04lx $%04lx'>
	LEA.L	5*4(SP),SP
	MOVEM.L (SP)+,A0/D0-D3
	ENDC	; of IFGE	INFOLEVEL-59

		CMP.W	#-1,D0			; Did we find any offset?
		BEQ	PC_DONE			; If not, we're *all* done

	IFGE	INFOLEVEL-59
	ANDI.L	#$0000FFFF,D0
	MOVE.L	D0,-(SP)
	PUTMSG	59,<'%s/CallService from PC, ServiceData offset=%$%lx'>
	LEA.L	4(SP),SP
	ENDC	; of IFGE	INFOLEVEL-59

		;------	Create the pointer to the ServiceData structure
		MOVE.W	#MEMF_PARAMETER+MEM_WORDACCESS,D1 ; Get the mem type
		CALLSYS	JanusOffsetToMem	; turn it into a real pointer

	IFGE	INFOLEVEL-2
	MOVE.L	A0,-(SP)
	PUTMSG	2,<'%s/CallService from PC, ServiceData address=$%lx'>
	LEA.L	4(SP),SP
	XREF	validateServiceData
	JSR	validateServiceData
	ENDC	; of IFGE INFOLEVEL-2


* 	- SignalCustomers(ServiceData, 1)
		MOVEQ.L	#1,D0			; Designate that we're from PC
		JSR	SignalCustomers		; and go signal the customers

		BRA	PC_CHANNELCHECK		; And start again from the top


PC_DONE:
	PUTMSG	59,<'%s/ProcessPCToAmiga almost done'>

* - unlock the ServiceParam lock
		MOVE.L	A5,A0
		CALLSYS	MakeBytePtr
		UNLOCK	spm_Lock(A0)

* - unlock the ServiceBase semaphore
;SEMS
		LEA.L	sb_ServiceSemaphore(A4),A0
		EXG.L	A6,A2
		CALLSYS	ReleaseSemaphore
;SEMS		CALLSYS	Permit
		EXG.L	A6,A2

	PUTMSG	59,<'%s/ProcessPCToAmiga done'>

		MOVEM.L	(SP)+,PC_REGS
		RTS



ProcessAdd:
* === ProcessAdd ============================================================
* ProcessAdd(ServiceData);
*            A0
* The other processor sent us an AddsService.  Process it here
* 
*    A2   = ExecBase
*    A3   = Address of the ServiceData.WA that's been added by the PC
*    A4   = Address of the ServiceData.BA that's been added by the PC
*    A5   = ServiceParam.WA
*    A6   = JanusBase, for janus library

	PUTMSG	2,<'%s/ProcessAdd:'>

		MOVEM.L	A3/A4,-(SP)
		MOVEA.L	A0,A3
		CALLSYS	MakeBytePtr
		MOVEA.L	A0,A4

* - if we're the Amiga, lock the ServiceData semaphore
		MOVEA.L	sd_Semaphore(A3),A0
		EXG	A2,A6
		CALLSYS	ObtainSemaphore
;SEMS		CALLSYS	Forbid
		EXG	A2,A6

* - lock the ServiceData lock
		LOCK	sd_ServiceDataLock(A4)

* - InformNewCustomers()
		MOVE.L	A3,A0
		MOVEQ.L	#0,D0		; We aren't calling from AddService()
		MOVEQ.L	#1,D1		; Designate that we're from the PC
		JSR	InformNewCustomers

* - unlock the ServiceData lock
		UNLOCK	sd_ServiceDataLock(A4)

* - if we're the Amiga unlock the ServiceData semaphore
		MOVEA.L	sd_Semaphore(A3),A0
		EXG	A2,A6
		CALLSYS	ReleaseSemaphore
;SEMS		CALLSYS	Permit
		EXG	A2,A6

ADD_DONE:
		MOVEM.L	(SP)+,A3/A4

		RTS



ProcessDelete:
* === ProcessDelete =========================================================
* ProcessDelete(ServiceData);
*               A0
* The other processor sent us a DeletesService.  Process it here
* 
* Rather than signalling just the customers of this service, 
* this algorithm checks *all* services and sends a signal to all that are 
* deleted, which redundancy is left in for its safety-margin value, 
* although if developers complain this could be left out easily 
* 
*    A0   = Address of the ServiceData.WA that's been deleted by the PC
*    A2   = ExecBase
*    A4   = ServiceBase
*    A5   = ServiceParam.WA
*    A6   = JanusBase, for janus library


	PUTMSG	2,<'%s/ProcessDELETE:'>

		MOVEM.L	A3/A4,-(SP)


* - traverse the ServiceData list, and if the SERVICE_DELETED flag is set
		MOVE.W	spm_FirstServiceData(A5),D0
DELETE_LOOP:	CMP.W	#-1,D0
		BEQ	DELETE_DONE
		MOVE.W	#MEMF_PARAMETER+MEM_WORDACCESS,D1
		CALLSYS	JanusOffsetToMem
		MOVEA.L	A0,A3
		MOVE.W	sd_Flags(A3),D1
		BTST	#SERVICE_DELETEDn,D1
		BEQ	NEXT_DELETE
		CALLSYS	MakeBytePtr
		MOVEA.L	A0,A4

* 	- if we're the Amiga, lock the ServiceData semaphore
		MOVEA.L	sd_Semaphore(A3),A0
		EXG	A2,A6
		CALLSYS	ObtainSemaphore
;SEMS		CALLSYS	Forbid
		EXG	A2,A6

* 	- lock the ServiceData lock
		LOCK	sd_ServiceDataLock(A4)

* 	- InformDeletedService(ServiceData)
		MOVEA.L	A3,A0
		JSR	InformDeletedService

* 	- unlock the ServiceData lock
		UNLOCK	sd_ServiceDataLock(A4)

* 	- if we're the Amiga unlock the ServiceData semaphore
		MOVEA.L	sd_Semaphore(A3),A0
		EXG	A2,A6
		CALLSYS	ReleaseSemaphore
;SEMS		CALLSYS	Permit
		EXG	A2,A6

NEXT_DELETE:	MOVE.W	sd_NextServiceData(A3),D0
		BRA	DELETE_LOOP


DELETE_DONE:

		MOVEM.L	(SP)+,A3/A4
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



; CreateTask:
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
; 
; ARGS	EQU	(11*4+4)
; 		MOVEM.L	A2-A6/D2-D7,-(SP)
; 
; 		MOVE.L	ARGS+0(SP),A2	; Name
; 		MOVE.L	ARGS+4(SP),D2	; Priority
; 		MOVE.L	ARGS+8(SP),A3	; Initial PC
; 		MOVE.L	ARGS+12(SP),D3	; Stack size
; 
; 		MOVE.L	#0,A4
; 
; 
; 		;------ Allocate the memory for the task structure
; 		MOVE.L	_AbsExecBase,A6
; 		MOVE.L	#TC_SIZE,D0
; 		MOVE.L	#MEMF_CLEAR,D1
; 		CALLSYS	AllocMem
; 		MOVE.L	D0,A4
; 		MOVE.L	A4,D0
; 		BEQ	createError
; 
; 
; 		;------ Allocate the user stack
; 		ADD.L	#3,D3		; Round the memory request up to 
; 		ANDI.L	#$FFFFFFFC,D3	; the nearest longword byte count
; 		MOVE.L	D3,D0
; 		MOVE.L	#MEMF_CLEAR,D1
; 		CALLSYS	AllocMem
; 		MOVE.L	D0,A5
; 		MOVE.L	A5,D0
; 		BEQ	createError
; 
; 
; 		;------ Set up the stack pointers in the task control block 
; 		MOVE.L	A5,TC_SPLOWER(A4)
; 		ADD.L	D3,A5
; 		SUB.L	#4,A5			; This can't be right
; 		MOVE.L	A5,TC_SPUPPER(A4)
; 		MOVE.L	A5,TC_SPREG(A4)
; 
; 
; 		;------ Also, initialize the node with the supplied information
; 		MOVE.B	#NT_TASK,LN_TYPE(A4)
; 		MOVE.B	D2,LN_PRI(A4)
; 		MOVE.L	A2,LN_NAME(A4)
; 
; 
; 		;------ Set up to call AddTask
; 		MOVE.L	A4,A1
; 		MOVE.L	A3,A2
; 		MOVE.L	#0,A3
; 		CALLSYS	AddTask
; 
; 
; 		;------ OK, we're successful, so return happily
; 		BRA	createDone
; 
; 
; createError:
; * Oops, error.  
; * If A4 was set, free the memory
; 		MOVE.L	A4,D0
; 		BEQ	createDone
; 		MOVE.L	A4,A1
; 		MOVE.L	#TC_SIZE,D0
; 		CALLSYS	FreeMem
; 
; 
; createDone:
; 		MOVE.L	A4,D0
; 
; 		MOVEM.L	(SP)+,A2-A6/D2-D7
; 		RTS



ServiceTaskName:
		DC.B	'ZaphodServiceTask',0
		CNOP	0,2
JanusName:
		DC.B	'janus.library',0
		CNOP	0,2

GraphicsName:	DC.B	'graphics.library',0
		CNOP	0,2


;AAA		XLIB	LoadSeg
;AAA		XLIB	CreateProc
;AAAAutoloadName:
;AAA		DC.B	'SYS:PC/Services/Autoload',0
;AAA		CNOP	0,2
;AAA
;AAAAutoloader:
;AAA		RTS
;AAA
;AAA		MOVEM.L	A2-A6/D2-D7,-(SP)
;AAA
;AAA		MOVEA.L	jb_DOSBase(A6),A6
;AAA
; 	XLIB	Delay
; 	MOVEM.L	A0-A1/D0-D1,-(SP)
; 	MOVE.L	#50*60,D1
; 	CALLSYS	Delay
; 	MOVEM.L	(SP)+,A0-A1/D0-D1
;AAA
;AAA		LEA.L	AutoloadName(PC),A2
;AAA		MOVE.L	A2,D1
;AAA	IFGE	INFOLEVEL-2
;AAA	MOVE.L	A6,-(SP)
;AAA	MOVE.L	D1,-(SP)
;AAA	PUTMSG	2,<'%s/Trying to LoadSeg [%s] with $%lx'>
;AAA	LEA.L	2*4(SP),SP
;AAA	ENDC	; of IFGE INFOLEVEL-2
;AAA		CALLSYS	LoadSeg
;AAA	IFGE	INFOLEVEL-2
;AAA	MOVE.L	D0,-(SP)
;AAA	PUTMSG	2,<'%s/Result of LoadSeg = $%lx'>
;AAA	LEA.L	1*4(SP),SP
;AAA	ENDC	; of IFGE INFOLEVEL-2
;AAA		TST.L	D0
;AAA		BEQ	aload_DONE
;AAA
;AAA		MOVE.L	A2,D1
;AAA		MOVEQ.L	#0,D2
;AAA		MOVE.L	D0,D3
;AAA		MOVE.L	#4000,D4
;AAA		CALLSYS	CreateProc
;AAA	IFGE	INFOLEVEL-2
;AAA	MOVE.L	D0,-(SP)
;AAA	PUTMSG	2,<'%s/Result of CreateProc = $%lx'>
;AAA	LEA.L	1*4(SP),SP
;AAA	ENDC	; of IFGE INFOLEVEL-2
;AAA
;AAAaload_DONE:
;AAA		MOVEM.L	(SP)+,A2-A6/D2-D7
;AAA		RTS



		XLIB	CreateProc
JMP_INSTRUCTION	EQU	$4EF9
* ULONG NextFake;
* USHORT JMP
* ULONG ProcAddress

CreateFakeProc:
* process = CreateFakeProc(name, priority, code, stacksize, DOSBase, ExecBase);
* D0,A0                    D1    D2        D3    D4         A0       A1
* Returns address of struct Process or NULL

CFP_REGS	REG	A2-A3/A6/D3

	IFGE	INFOLEVEL-2
	MOVE.L	D4,-(SP)
	MOVE.L	D3,-(SP)
	MOVE.L	D2,-(SP)
	MOVE.L	D1,-(SP)
	PUTMSG	2,<'%s/CreateFakeProc name[%s] pri=%ld code=$%lx size=%ld'>
	MOVE.L	A1,-(SP)
	MOVE.L	A0,-(SP)
	PUTMSG	2,<'%s/CreateFakeProc DOSBase=$%lx ExecBase=$%lx'>
	LEA.L	6*4(SP),SP
	ENDC	; of IFGE INFOLEVEL-2

		MOVEM.L	CFP_REGS,-(SP)

		MOVEA.L	A1,A3
		MOVEA.L	A1,A6

		MOVEM.L	D1/A0,-(SP)
		MOVE.L	#16,D0
		MOVE.L	#MEMF_CLEAR,D1
		CALLSYS	AllocMem
		MOVEM.L	(SP)+,D1/A0
		TST.L	D0
		BEQ	CFP_DONE
		MOVEA.L	D0,A2
		MOVE.W	#JMP_INSTRUCTION,4(A2)
		MOVE.L	D3,6(A2)
		MOVE.L	A2,D3
		LSR.L	#2,D3

		MOVEA.L	A0,A6
		CALLSYS	CreateProc
		TST.L	D0
		BNE	CFP_DONE

		MOVEA.L	A2,A1
		MOVE.L	#16,D0
		MOVEA.L	A3,A6
		CALLSYS	FreeMem
		MOVEQ.L	#0,D0

CFP_DONE:
		MOVEM.L	(SP)+,CFP_REGS
		RTS



		END



