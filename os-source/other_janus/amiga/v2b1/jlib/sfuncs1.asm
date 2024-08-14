
* *** sfuncs1.asm *************************************************************
* 
* Services Functions Implementation Algorithms and Routines
* =Robert J. Mical=
* Copyright (C) 1988, Commodore-Amiga Inc.
* 
* CONFIDENTIAL and PROPRIETARY
* 
* Date       Name           Description
* ---------  -------------  ---------------------------------------------------
* 27-Jul-88  -RJ            In AddService() now calculate and save PCMemPtr
* 15-Jul-88  -RJ            Changed all files to work with new includes
* 19-Feb-88  =RJ Mical=     Created this file!
* 
* *****************************************************************************


	INCLUDE "assembly.i"


	NOLIST
	INCLUDE "exec/types.i"

	INCLUDE "janus/janusbase.i"
	INCLUDE "janus/janusvar.i"
	INCLUDE "janus/memory.i"
	INCLUDE "janus/services.i"

 	INCLUDE "serviceint.i"
	LIST

	INCLUDE "asmsupp.i"


	XLIB	AllocJanusMem
	XLIB	AllocMem
	XLIB	Forbid
	XLIB	FreeJanusMem
	XLIB	FreeMem
	XLIB	JanusMemBase
	XLIB	JanusMemToOffset
	XLIB	JanusMemType
;NOSEMS	XLIB	ObtainSemaphore
	XLIB	Permit
;NOSEMS	XLIB	ReleaseSemaphore
	XLIB	SendJanusInt
	XLIB	WaitTOF

	XREF	AllocServiceCustomer
	XREF	FindUnusedAToPC
	XREF	FreeServiceCustomer
	XREF	InformNewCustomers
	XREF	MakeServiceData
	XREF	UnmakeServiceData
	XREF	WaitServiceParam

	XDEF	AddService
	XDEF	GetService

	XLIB	MakeBytePtr
	XLIB	MakeWordPtr
	XLIB	JanusOffsetToMem
	XLIB	TranslateJanusPtr



* =============================================================================
* === IMPLEMENTATION ALGORITHMS ===============================================
* =============================================================================
* 
* This section is for Torsten and RJ.  These notes are *not* to be released 
* to the real world, please.




AddService:
* === AddService() ============================================================
* 
* NAME
*     AddService  --  Adds a Janus Service to the system
* 
* 
* SYNOPSIS
*     resultcode = AddService(
*              ServiceData, AppID, LocalID, MemSize, MemType, SignalNum, Flags)
*     D0:0-15  A0           D0     D1:0-15  D2:0-15  D3:0-15  D4:0-15    D5:0-15
* 
*         SHORT   resultcode;
*         struct  ServiceData **ServiceData;
*         ULONG   AppID;
*         USHORT  LocalID;
*         USHORT  MemSize;
*         USHORT  MemType;
*         SHORT   SignalNumber;
*         USHORT  Flags;
*     From assembly:  A6 has pointer to JanusBase
* 
*     
* FUNCTION
*     This routine adds a Janus service to the system.  Please refer 
*     to the Janus Services reference documentation for a description of 
*     services and adding a service to the system.  
*     
*     The ServiceData argument you supply must be an address of a pointer 
*     to a ServiceData structure.  If the service is successfully added, 
*     when this call returns your pointer will point to the ServiceData 
*     structure of your newly-added service.  If the call fails your 
*     pointer will be equal to NULL.  The address, if non-NULL, will 
*     be an address in Janus memory's word-access address space.  
*     
*     That your ServiceData is set equal to NULL if AddService() fails 
*     provides you with a convenient way to test whether or not you have 
*     to call DeleteService() during your clean-up routine.  For example:
*         struct ServiceData *SData = NULL;
*             result = AddService(&SData, ...);
*             myExit();
*         myExit()
*         {
*             if (SData) DeleteService(SData);
*             exit(0);
*         }
*     
*     The AppID, or Application ID, and LocalID that you provide are used 
*     by other programs to rendevous with your service.  The Application ID 
*     that you use should be gotten from Commodore-Amiga, Inc.  
*     Do not use an Application ID of your own making, for you risk a 
*     collision with someone else's use of the same ID number.  It's easy 
*     enough to get an Application ID number from Commodore, so do it.  
*     The LocalID is defined by you to mean anything you want it to mean.  
*     Anyone who wants to rendevous with your service needs to know both 
*     your Application ID and your Local ID.  
*     
*     SignalNumber is the number of the signal bit that you will use 
*     to Wait() for CallService() signals.  Typically you will get 
*     your SignalNumber from the Exec function AllocSignal().  
* 
*     The Flags argument lets you specify details about the type of service 
*     you want to add.  Please refer to the Janus Services reference 
*     documentation and the services.[hi] include file for information 
*     regarding the arguments that may be provided to the AddService() 
*     call via the Flags argument.  
*     
*     This routine returns a code which is either JSERV_OK or some error-return 
*     code.  The codes that this routine may return are defined in 
*     services.[hi].
*     
*     If you call AddService() successfully, you are obliged to call 
*     DeleteService() sooner or later.  
*     
*     
* INPUTS
*     ServiceData = address of a pointer to a ServiceData structure.  
*         After this call, the pointer will be set to either the 
*         word-access address of the newly-added service's ServiceData 
*         structure, or NULL if the call to this routine failed
*     AppID = a 32-bit Application ID number for the service you want to 
*         add.  This number is assigned to you by Commodore-Amiga, Inc.  
*         REMINDER:  do not use a number of your own making, as other 
*         software may then invalidly rendevous with your service with results 
*         that make the guru giggle 
*     LocalID = a 16-bit Local ID number of your own choosing
*     MemSize = the size of the memory block to be allocated for this 
*         service, if any.  Set this arg to zero if you want no memory 
*         allocated for this service
*     MemType = the type of memory to be allocated for this service, 
*         if any.  If the MemSize arg above is zero, this argument is 
*         ignored
*     SignalNumber = the number of the signal bit that you will Wait() 
*         on when waiting to receive CallService() signals.  The SignalNumber 
*         is usually gotten from a call to Exec's AllocSignal()
*     Flags = flags describing any special treatment you want during 
*         the processing of your AddService() call.  See services.[hi] and 
*         the Janus Services reference document for details and definitions
*     
*     
* RESULTS
*     resultcode = a code describing the result of your call to AddService().  
*         This will be JSERV_OK if all went well, or some error-return 
*         code if the call failed for any reason.  The codes that this 
*         routine may return are defined in services.[hi].
*     
*     
* SEE ALSO
*     GetService(), CallService(), DeleteService()


ADDS_REGS	REG	D0/D2-D7/A0/A2-A6
A_D0_OFFSET	EQU	0*4
A_A0_OFFSET	EQU	7*4

		MOVEM.L	ADDS_REGS,-(SP)

	IFGE	INFOLEVEL-1
	MOVEM.L	D0-D5,-(SP)
	MOVE.L	A0,-(SP)
	PUTMSG	1,<'%s/AddService( $%lx $%08lx $%04lx %ld $%04lx %ld $%04lx)'>
	LEA	7*4(SP),SP
	ENDC

		;------	Before anything, call this completely 
		;------	non-register-destructive routine which waits until 
		;------	the ServiceTask has allocated and initialized 
		;------	the ServiceParam data structure
		JSR	WaitServiceParam


* Throughout this routine, except for local changes (which must be repaired
* afterwards), these registers will be constant:
*    D2 = MemSize
*    D3 = MemType
*    D4 = ApplicationID
*    D5 = LocalID
*    D6 = Flags
*    D7 = SignalNumber
*    A2 = ExecBase
*    A4 = ServiceData.WA structure (if one is allocated, else A4 == NULL)
*    A5 = Service Janus memory.WA (if allocated, else A5 == NULL)
*    A6 = JanusBase, for janus library
		MOVE.W	D4,D7
		MOVE.W	D5,D6
		MOVE.L	D0,D4
		MOVE.L	D1,D5
		MOVEA.L	jb_ExecBase(A6),A2
		MOVEA.L	#0,A4
		MOVEA.L	A4,A5

		;------	Next, to avoid compliations between the fact that 
		;------	some of our args are word-sized to make PC Services 
		;------	easier while janus.library expects these same args 
		;------	to be longs, let's turn them into long words with 
		;------	only words of significant information.
		MOVE.L	#$0000FFFF,D0
		AND.L	D0,D2
		AND.L	D0,D3
		AND.L	D0,D5
		AND.L	D0,D6
		AND.L	D0,D7


		;------	Set the D0 return value register to the OK return.  If
		;------	the AddService() call fails, the D0 on the stack will 
		;------	be changed to reflect the reason for failure.  Later, 
		;------	when the registers are restored at the end of the 
		;------	call, D0 will correctly have the return code
		MOVE.L	#JSERV_OK,A_D0_OFFSET(SP)


* - if we're the Amiga
* 	- lock the ServiceBase semaphore
;NOSEMS		MOVEA.L	jb_ServiceBase(A6),A3
;NOSEMS		LEA	sb_ServiceSemaphore(A3),A0
		EXG.L	A6,A2
;NOSEMS		CALLSYS	ObtainSemaphore
		CALLSYS	Forbid
		EXG.L	A6,A2


* - lock the ServiceParam lock
		MOVEA.L	jb_ServiceBase(A6),A3
		MOVEA.L	sb_ServiceParam(A3),A3
		MOVEA.L	A3,A0
		CALLSYS	MakeBytePtr
		LOCK	spm_Lock(A0)


* - check to see if the service already exists:  traverse the ServiceData list, 
*   checking first that the ServiceData's SERVICE_DELETED flag isn't set and 
*   if it isn't then checking for a match with ApplicationID and LocalID
		MOVE.W	spm_FirstServiceData(A3),A3
		BRA	CHECK_END

CHECK_EXIST:
		;------	Turn offset into pointer to next ServiceData struct
		MOVE.W	A3,D0
		MOVE.L	#MEMF_PARAMETER+MEM_WORDACCESS,D1
		CALLSYS	JanusOffsetToMem
		MOVEA.L	D0,A3

		;------	If this service was already deleted, don't check it
		MOVE.W	sd_Flags(A3),D0
		BTST	#SERVICE_DELETEDn,D0
		BNE	CHECK_NEXT

		;------	Check if the user-specified ID fields match the 
		;------	values of this service, and react if so
		CMP.L	sd_ApplicationID(A3),D4	; Is this ID already used?
		BNE	CHECK_NEXT		; If not, check next
		CMP.W	sd_LocalID(A3),D5	; Is LocalID used too?
		BEQ	EXISTS			; If so, service already exists

CHECK_NEXT:
		;------	Get the offset to the next ServiceData structure
		MOVE.W	sd_NextServiceData(A3),A3

CHECK_END:
		CMPA.W	#-1,A3			; If the offset to next is -1
		BEQ	NOT_EXISTS		; then we're at end of list
		BRA	CHECK_EXIST		; else go examine next struct


EXISTS:
	PUTMSG	59,<'%s/Existing service found'>

* 	- if match found, the service already exists
* 		- if we're the Amiga
* 			- if the ServiceCustomer field is non-null 
* 			  and the ADDS_EXCLUSIVE flag is set, then some 
* 			  Amiga task is already attached with this service so 
* 			  the exclusivity request fails, so goto ERROR
		CMP.L	#0,sd_FirstAmigaCustomer(A3) ; Got any customers yet?
		BEQ	Exists2			     ; If not, we're cool
		BTST	#ADDS_EXCLUSIVEn,D6	     ; Want it exclusively?
		BEQ	Exists2			     ; If not, we're cool
		MOVE.L	#JSERV_NOTEXCLUSIVE,A_D0_OFFSET(SP) ; else oops
		BRA	ADDSERVICE_ERROR


Exists2:
* 		- if AmigaUserCount == 0 and PCUserCount == 0, 
* 		  this is a service that a
* 		  WAIT task is waiting for so proceed
		MOVEA.L	A3,A0
		CALLSYS	MakeBytePtr
		TST.B	sd_AmigaUserCount(A0)
		BNE	10$
		TST.B	sd_PCUserCount(A0)
		BNE	10$

		;------	AmigaUserCount and PCUserCount were 0
		MOVE.L	A3,A4		; Copy the new ServiceData ptr
		BRA	NEW_SERVICE_USER

10$
* 		- else if AmigaUserCount != 0 or PCUserCount != 0, 
* 		  this is an AddService() request 
* 		  for a service that already exists, so goto error
		MOVE.L	#JSERV_DUPSERVICE,A_D0_OFFSET(SP)
		BRA	ADDSERVICE_ERROR


NOT_EXISTS:
* 	- if match not found, this is a virgin service so start it from scratch
* 		- ServiceData = MakeServiceData()
* 		- if ServiceData == NULL goto error
		JSR	MakeServiceData
		TST.L	D0
		BNE	10$
		MOVE.L	#JSERV_NOJANUSMEM,A_D0_OFFSET(SP)
		BRA	ADDSERVICE_ERROR
10$		MOVEA.L	D0,A4		; Copy our new ServiceData ptr
		BSET	#SD_CREATEDn,D6	; Mark the flag that we allocated it


NEW_SERVICE_USER:
	PUTMSG	59,<'%s/new service user'>

* - if user's ADDS_LOCKDATA flag is set, lock the ServiceDataLock now
		BTST	#ADDS_LOCKDATAn,D6
		BEQ	2001$
		MOVEA.L	A4,A0
		CALLSYS	MakeBytePtr
		MOVE.B	#$FF,sd_ServiceDataLock(A0)
2001$


* - allocate janus memory of MemType and MemSize
* 	- if allocation not successful, goto error
		MOVE.L	D2,D0
		MOVE.L	D3,D1
		CALLSYS	AllocJanusMem
		TST.L	D0
		BNE	10$
		MOVE.L	#JSERV_NOJANUSMEM,A_D0_OFFSET(SP)
		BRA	ADDSERVICE_ERROR
10$		MOVEA.L	D0,A5


* - clear the ServiceData's memory buffer
		MOVEA.L	A5,A0
		CALLSYS	MakeBytePtr
		MOVE.W	D2,D1
		SUBQ.W	#1,D1
		MOVEQ.L	#0,D0
100$		MOVE.B	D0,(A0)+
		DBRA	D1,100$


* - set ServiceData's MemType, MemSize and MemOffset
		MOVE.W	D2,sd_MemSize(A4)
		MOVE.W	D3,sd_MemType(A4)
		MOVE.L	A5,D0
		CALLSYS	JanusMemToOffset
		MOVE.W	D0,sd_MemOffset(A4)


* - if we're the Amiga
* 	- save the MemPtr in the ServiceData's AmigaMemPtr field
* 	- calculate and set the PCMemPtr
* - else if we're the PC
* 	- save the MemPtr in the ServiceData's PCMemPtr field
* 	- calculate and set the AmigaMemPtr
		MOVE.L	A5,sd_AmigaMemPtr(A4)

		;------	New:  calculate the PC address of the memory too.
		;------	First get the 16 bits of Offset as the high 16 bits.  
		;------	Then get PC's segment of the memory and use this 
		;------	as the low 16 bits of the address.  
		;------	This is the PC address.
		MOVE.L	jb_ParamMem(A6),A0	; Get the address of JanusAmiga
		CALLSYS	MakeWordPtr		; Use word access
		BTST	#MEMB_PARAMETER,D3	; Is memory type param?
		BEQ	32010$			; If not try next type
		LEA	ja_ParamMem(A0),A0	; Use param mem 
		BRA	32100$
32010$		LEA	ja_BufferMem(A0),A0	; No other type, so use buffer
32100$
		MOVE.W	sd_MemOffset(A4),D0	; Get the offset into low 16
		SWAP	D0			; Put offset into high 16
		MOVE.W	jmh_8088Segment(A0),D0	; Fetch the segment
		MOVE.L	D0,sd_PCMemPtr(A4)	; Save the address

* - if we're the Amiga, perform the ServiceCustomer stuff for this new service
* 	- allocate a ServiceCustomer structure in non-janus memory
* 		- if can't allocate, goto error
		MOVE.W	D7,D0
		JSR	AllocServiceCustomer
		TST.L	D0
		BNE	20$
		MOVE.L	#JSERV_NOAMIGAMEM,A_D0_OFFSET(SP)
		BRA	ADDSERVICE_ERROR
20$

	IFGE	INFOLEVEL-59
	MOVEM.L	D0,-(SP)
	PUTMSG	59,<'%s/AllocServiceCustomer=0x%lx'>
	MOVEM.L	(SP)+,D0
	ENDC

* 	- InformNewCustomers()
* 	NOTE:  FROM HERE TO THE END there can be no error exit, because 
*	we are about to tell the customers (if any) that the service has 
*	become available, so it would not be nice to fail after this
		MOVE.L	D0,-(SP)	; Stash ptr to new ServiceCustomer
		MOVE.L	A4,A0
		MOVEQ.L	#1,D0		; We are calling from AddService()
		JSR	InformNewCustomers
		MOVEA.L	(SP)+,A0	; Restore ptr to new ServiceCustomer

* 	- initialize and add the new ServiceCustomer to the ServiceData list
		MOVE.L	sd_FirstAmigaCustomer(A4),scs_NextCustomer(A0)
		MOVE.L	A0,sd_FirstAmigaCustomer(A4)


* New:  unlock the ServiceParam lock now, not below
* - unlock the ServiceParam lock
		MOVEA.L	jb_ServiceBase(A6),A3
		MOVEA.L	sb_ServiceParam(A3),A0
		CALLSYS	MakeBytePtr
		UNLOCK	spm_Lock(A0)

* - signal the other processor with an AddsService
		MOVEA.L	jb_ServiceBase(A6),A3
		MOVE.L	sb_ServiceParam(A3),A3

ADDS_wait:
		CMP.W	#-1,spm_AmigaAddsService(A3)
		BEQ	ADDS_add0
		CMP.W	#-1,spm_AmigaAddsService+2(A3)
		BEQ	ADDS_add1
	PUTMSG	2,<'%s/still waiting to ADD_SERVICE...'>
		;------	Call WaitTOF() to let the system run while we're 
		;------	waiting to check again
		MOVE.L	A3,-(SP)	; Stash A3
		MOVEA.L	jb_ServiceBase(A6),A3
		MOVEA.L	sb_GfxBase(A3),A3
		EXG.L	A3,A6
		;------	Note that if you are in Forbid() when the following 
		;------	call to WaitTOF is made, the Forbid() is broken 
		;------	until control is returned to you, at which time the 
		;------	Forbid() is already reestablished
		CALLSYS	WaitTOF
		MOVE.L	A3,A6
		MOVE.L	(SP)+,A3
		BRA	ADDS_wait
ADDS_add1:
		;------	We're going to transmit using the second element of the 
		;------	16-bit array, so offset the structure pointer by two
		ADD.W	#2,A3
ADDS_add0:
		;------	Here, A3 points to either the first or second word 
		;------	of the ServiceParam structure, so that offsetting with
		;------	spm_AmigaAddsService will result in the address of 
		;------	either the first or second element of the AddsService
		;------	array
		MOVE.L	A4,D0
		CALLSYS	JanusMemToOffset
		MOVE.W	D0,spm_AmigaAddsService(A3)

		MOVE.L	#JSERV_PCSERVICE,D0	; Amiga initiates service 
		CALLSYS	SendJanusInt

		BRA	ADDSERVICE_DONE


ADDSERVICE_ERROR:
* - if we're the Amiga and ServiceCustomer allocated, free
* - if janus memory allocated, free
	PUTMSG	2,<'AddService error'>

		CMPA.L	#0,A4		; Is there a ServiceData pointer?
		BEQ	ADDS_FREE1	;   branch if not
		;------	set ServiceData's AmigaUserCount back to 0
		MOVEA.L	A4,A0
		CALLSYS	MakeBytePtr
		MOVE.B	#0,sd_AmigaUserCount(A0)
		BTST	#SD_CREATEDn,D6	; Did we allocate this structure?
		BEQ	ADDS_FREE1	;   branch if not
		MOVEA.L	A4,A0
		JSR	UnmakeServiceData
		MOVE.L	#0,A4
ADDS_FREE1:

		CMPA.L	#0,A5		; Was buffer memory allocated?
		BEQ	ADDS_FREE2	;   branch if not
		MOVEA.L	A5,A1
		MOVE.L	D2,D0
		CALLSYS	FreeJanusMem
		MOVE.L	#0,A5
ADDS_FREE2:

* New:  unlock the ServiceParam lock now, not below
* - unlock the ServiceParam lock
		MOVEA.L	jb_ServiceBase(A6),A3
		MOVEA.L	sb_ServiceParam(A3),A0
		CALLSYS	MakeBytePtr
		UNLOCK	spm_Lock(A0)


ADDSERVICE_DONE:
	PUTMSG	59,<'%s/AddService done'>

* - if we're the Amiga
* 	- unlock the ServiceBase semaphore
;NOSEMS		MOVEA.L	jb_ServiceBase(A6),A3
;NOSEMS		LEA	sb_ServiceSemaphore(A3),A0
		EXG.L	A6,A2
;NOSEMS		CALLSYS	ReleaseSemaphore
		CALLSYS	Permit
		EXG.L	A6,A2

* - return the address of the ServiceData structure, or NULL, as appropriate
		MOVEA.L	A_A0_OFFSET(SP),A0	; Get the address of the ptr
		MOVE.L	A4,(A0)

		MOVEM.L	(SP)+,ADDS_REGS
		RTS



GetService:
* === GetService() ==========================================================
*
* NAME
*     GetService  --  Gets a Janus Service 
* 
* 
* SYNOPSIS
*     resultcode = GetService(ServiceData, AppID, LocalID, SignalNumber, Flags)
*     D0:0-15                 A0           D0     D1:0-15  D2:0-15       D3:0-15
* 
*         SHORT   resultcode;
*         struct  ServiceData **ServiceData;
*         ULONG   AppID;
*         USHORT  LocalID;
*         SHORT   SignalNumber;
*         USHORT  Flags;
*     From assembly:  A6 has pointer to JanusBase
* 
*     
* FUNCTION
*     This routine gets a Janus service from the system.  Please refer 
*     to the Janus Services reference documentation for a complete 
*     description of services and getting a service from the system.  
*     
*     The ServiceData argument you supply must be an address of a pointer 
*     to a ServiceData structure.  If the service is successfully acquired, 
*     when this call returns your pointer will point to the ServiceData 
*     structure of your newly-acquired service.  If the call fails your 
*     pointer will be equal to NULL.  The address, if non-NULL, will 
*     be an address in Janus memory's word-access address space.  
*     
*     That your ServiceData is set equal to NULL if GetService() fails 
*     provides you with a convenient way to test whether or not you have 
*     to call ReleaseService() during your clean-up routine.  For example:
*         struct ServiceData *SData = NULL;
*             result = GetService(&SData, ...);
*             myExit();
*         myExit()
*         {
*             if (SData) ReleaseService(SData);
*             exit(0);
*         }
*     
*     The AppID, or Application ID, and LocalID that you provide are used 
*     to rendevous with a service that is added by some other program.  
*     You must learn of the Application ID and Local ID of the service from 
*     the creator of the service before you can call GetService().  
*     
*     SignalNumber is the number of the signal bit that you will use 
*     to Wait() for CallService() signals.  Typically you will get 
*     your SignalNumber from the Exec function AllocSignal().  
* 
*     The Flags argument lets you specify details about the type of service 
*     you want to get.  Please refer to the Janus Services reference 
*     documentation and the services.[hi] include file for information 
*     regarding the arguments that may be provided to the GetService() 
*     call via the Flags argument.  
*     
*     This routine returns a code which is either JSERV_OK or some error-return 
*     code.  The codes that this routine may return are defined in 
*     services.[hi].
*     
*     If you call GetService() successfully, you are obliged to call 
*     ReleaseService() sooner or later.  
*     
*     
* INPUTS
*     ServiceData = address of a pointer to a ServiceData structure.  
*         After this call, the pointer will be set to either the 
*         word-access address of the newly-acquired service's ServiceData 
*         structure, or NULL if the call to this routine failed
*     AppID = a 32-bit Application ID number for the service you want to find
*     LocalID = a 16-bit Local ID number of the service you want to find
*     SignalNumber = the number of the signal bit that you will Wait() 
*         on when waiting to receive CallService() signals.  The SignalNumber 
*         is usually gotten from a call to Exec's AllocSignal()
*     Flags = flags describing any special treatment you want during 
*         the processing of your GetService() call.  See services.[hi] and 
*         the Janus Services reference document for details and definitions
*     
*     
* RESULTS
*     resultcode = a code describing the result of your call to GetService().  
*         This will be JSERV_OK if all went well, or some error-return 
*         code if the call failed for any reason.  The codes that this 
*         routine may return are defined in services.[hi].
*     
*     
* BY THE WAY
*     If you want to say "hi" to RJ Mical, you can try to find him at 
*     AppID 123, LocalID 0x8000.  If found, Forbid() (on the Amiga side), 
*     lock the ServiceDataLock byte, check the flags, read or write a message, 
*     unlock, and Permit() (on the Amiga side).  First byte in MemPtr is the 
*     maximum string length (don't change this field, it's read-only), 
*     second byte is the current string length, third byte is flags where 
*     bit 0 (mask 0x0001) means that this time I want you to read only.  
*     Check this flag ( from within Forbid()/Permit() ) every time you're 
*     about to write to the buffer.  All other bits of the third byte are 
*     reserved by me.  Fourth through n bytes are the current string.  
*     If you write a message, be sure to CallService(), eh?  
*     
*     
* SEE ALSO
*     AddService(), CallService(), ReleaseService()


* This algorithm is for the Amiga side, but the idea should be much the same 
* on the PC if you take out the ServiceCustomer stuff (or implement 
* ServiceCustomer-type stuff on the PC side)


GETS_REGS	REG	D0/D2-D7/A0/A2-A7
G_D0_OFFSET	EQU	0*4
G_A0_OFFSET	EQU	7*4


		MOVEM.L	GETS_REGS,-(SP)


	IFGE	INFOLEVEL-1
	MOVEM.L	D0-D3,-(SP)
	MOVE.L	A0,-(SP)
	PUTMSG	1,<'%s/GetService( $%lx $%08lx $%04lx %ld $%04lx )'>
	LEA.L	5*4(SP),SP
	ENDC


		;------	Before anything, call this completely 
		;------	non-register-destructive routine which waits until 
		;------	the ServiceTask has allocated and initialized 
		;------	the ServiceParam data structure
		JSR	WaitServiceParam


* Throughout this routine, except for local changes (which must be repaired
* afterwards), these routines will be constant:
*    D2 = SignalNumber
*    D3 = Flags
*    D4 = ApplicationID
*    D5 = LocalID
*    A2 = ExecBase
*    A4 = ServiceCustomer struct.WA, if one is allocated, NULL until 
*    A5 = Address of ServiceData.WA, NULL until one is found (if ever)
*    A6 = JanusBase, for janus library
		MOVE.L	D0,D4
		MOVE.L	D1,D5
		MOVE.L	#$0000FFFF,D0
		AND.L	D0,D2
		AND.L	D0,D3
		AND.L	D0,D5
		MOVEA.L	jb_ExecBase(A6),A2
		MOVEA.L	#0,A4
		MOVEA.L	A4,A5


		;------	Set the D0 return value register to the OK return.  If
		;------	the AddService() call fails, the D0 on the stack will 
		;------	be changed to reflect the reason for failure.  Later, 
		;------	when the registers are restored at the end of the 
		;------	call, D0 will correctly have the return code
		MOVE.L	#JSERV_OK,G_D0_OFFSET(SP)


* - lock the ServiceBase semaphore
		MOVEA.L	jb_ServiceBase(A6),A3
;NOSEMS		LEA	sb_ServiceSemaphore(A3),A0
		EXG.L	A6,A2
;NOSEMS		CALLSYS	ObtainSemaphore
		CALLSYS	Forbid
		EXG.L	A6,A2


* - lock the ServiceParam lock
		MOVEA.L	sb_ServiceParam(A3),A3
		MOVEA.L	A3,A0
		CALLSYS	MakeBytePtr
		LOCK	spm_Lock(A0)


* - try to find the service:  traverse the ServiceData list, checking that the 
*   ServiceData's SERVICE_DELETED flag isn't set and if it isn't then check 
*   for a match with ApplicationID and LocalID
		MOVE.W	spm_FirstServiceData(A3),A3
		BRA	GETCHECK_END

GETCHECK_EXIST:
		;------	Turn offset into pointer to next ServiceData struct
		MOVE.W	A3,D0
		MOVE.L	#MEMF_PARAMETER+MEM_WORDACCESS,D1
		CALLSYS	JanusOffsetToMem
		MOVEA.L	D0,A3

		;------	If this service was already deleted, don't check it
		MOVE.W	sd_Flags(A3),D0
		BTST	#SERVICE_DELETEDn,D0
		BNE	GETCHECK_NEXT

		;------	Check if the user-specified ID fields match the 
		;------	values of this service, and react if so
		CMP.L	sd_ApplicationID(A3),D4	; Does this ID match?
		BNE	GETCHECK_NEXT		; If not, check next
		CMP.W	sd_LocalID(A3),D5	; Does LocalID match too?
		BNE	GETCHECK_NEXT		; If not, try again
		MOVEA.L	A3,A5			; else we have a ServiceData
		MOVEA.L	A3,A0
		CALLSYS	MakeBytePtr
		MOVEA.L	A0,A3
		BRA	GET_RESOLVED		; so let's go process it

GETCHECK_NEXT:
		MOVE.W	sd_NextServiceData(A3),A3
GETCHECK_END:
		CMP.W	#-1,A3			; If the offset to next isn't -1
		BNE	GETCHECK_EXIST		; then go examine next struct


GET_RESOLVED:
* ON ENTRY TO GET_RESOLVED, either A5 is NULL or A5 is a word-access ptr to 
* a ServiceData and A3 is a byte-access ptr to the ServiceData

* - if match found and AmigaUserCount > 0 or PCUserCount > 0, 
*   the service is available
		CMPA.L	#0,A5
		BEQ	GET_RES2
		TST.B	sd_AmigaUserCount(A3)
		BNE	10$
		TST.B	sd_PCUserCount(A3)
		BEQ	GET_RES2	; Branch if Amiga and PC have no users
10$

* 	- allocate and initialize a ServiceCustomer structure in non-janus mem
* 		- if not able to allocate, goto error
		MOVE.W	D2,D0
		JSR	AllocServiceCustomer
		TST.L	D0
		BNE	1000$
		MOVE.L	#JSERV_NOAMIGAMEM,G_D0_OFFSET(SP)
		BRA	GETSERVICE_ERROR
1000$		MOVEA.L	D0,A4

* 	- if we're the Amiga
* 		- increment ServiceData AmigaUserCount
* 	- if we're the PC
* 		- increment ServiceData PCUserCount
		ADDQ.B	#1,sd_AmigaUserCount(A3)

* 	- add ServiceCustomer to ServiceData list of customers
		MOVE.L	sd_FirstAmigaCustomer(A5),scs_NextCustomer(A4)
		MOVE.L	A4,sd_FirstAmigaCustomer(A5)
		BRA	GETSERVICE_DONE


GET_RES2: 
* - else if match found and AmigaUserCount == 0 and PCUserCount == 0 
*   and Wait is TRUE, or match not found but Wait is TRUE, 
*   then the service isn't available yet but this user 
*   is willing to wait for it
		CMP.L	#0,A5
		BEQ	GET_RES2B
		TST.B	sd_AmigaUserCount(A3)
		BNE	GET_RES2B
		TST.B	sd_PCUserCount(A3)
		BNE	GET_RES2B
		BTST	#GETS_WAITn,D3
		BNE	GET_2_RESOLVED

GET_RES2B:
		CMP.L	#0,A5
		BEQ	10$
		;------	else match is found, user count is 0, Wait is FALSE
		MOVE.L	#JSERV_NOSERVICE,G_D0_OFFSET(SP)
		BRA	GETSERVICE_ERROR
10$		BTST	#GETS_WAITn,D3
		BNE	20$
		;------	else match is not found, Wait is FALSE
		MOVE.L	#JSERV_NOSERVICE,G_D0_OFFSET(SP)
		BRA	GETSERVICE_ERROR
20$

GET_2_RESOLVED:
* 	- allocate and initialize a ServiceCustomer structure in non-janus mem
* 		- if not able to allocate, goto error
		MOVE.W	D2,D0
		JSR	AllocServiceCustomer
		TST.L	D0
		BNE	10$
		MOVE.L	#JSERV_NOAMIGAMEM,G_D0_OFFSET(SP)
		BRA	GETSERVICE_ERROR
10$		MOVEA.L	D0,A4

* 	- if match not found, create a ServiceData structure at this time 
* 		- ServiceData = MakeServiceData()
* 		- if ServiceData == NULL goto error
		CMP.L	#0,A5
		BNE	110$
		JSR	MakeServiceData
		TST.L	D0
		BNE	100$
		MOVE.L	#JSERV_NOAMIGAMEM,G_D0_OFFSET(SP)
		BRA	GETSERVICE_ERROR
100$		MOVEA.L	D0,A5
110$

* NOTE THAT FROM HERE TO THE END we can't have any more error exits, 
* because we no longer know whether A5 points to a found ServiceData 
* structure or an allocated one.

* 	- add ServiceCustomer to ServiceData list of customers 
		MOVE.L	sd_FirstAmigaCustomer(A5),scs_NextCustomer(A4)
		MOVE.L	A4,sd_FirstAmigaCustomer(A5)
		BRA	GETSERVICE_DONE


GETSERVICE_ERROR:
	PUTMSG	2,<'%s/GETSERVICE_ERROR:'>

* - if we allocated a ServiceCustomer, free it
		CMPA.L	#0,A4
		BEQ	10$
		MOVEA.L	A4,A1
		JSR	FreeServiceCustomer
		MOVEA.L	#0,A4
10$
		;------	It's safe to just NULL out the ServiceData pointer
		;------	(see comments above to find out why)
		MOVEA.L	#0,A5


GETSERVICE_DONE:
	PUTMSG	59,<'%s/GETSERVICE almost done:'>

* - unlock the ServiceParam lock
		MOVEA.L	jb_ServiceBase(A6),A3
		MOVEA.L	sb_ServiceParam(A3),A0
		CALLSYS	MakeBytePtr
		UNLOCK	spm_Lock(A0)


* - unlock the ServiceBase semaphore
;NOSEMS		LEA	sb_ServiceSemaphore(A3),A0
		EXG.L	A6,A2
;NOSEMS		CALLSYS	ReleaseSemaphore
		CALLSYS	Permit
		EXG.L	A6,A2


* - return the address of the ServiceData structure, or NULL, as appropriate
		MOVEA.L	G_A0_OFFSET(SP),A0	; Get the address of the ptr
		MOVE.L	A5,(A0)


		MOVEM.L	(SP)+,GETS_REGS
		RTS



		END


