
* *** sfuncs2.asm *************************************************************
* 
* Services Functions Implementation Algorithms and Routines
* =Robert J. Mical=
* Copyright (C) 1988, Commodore-Amiga Inc.
* 
* CONFIDENTIAL and PROPRIETARY
* 
* Date       Name           Description
* ---------  -------------  ---------------------------------------------------
* 7-Oct-88   -RJ            Added lots of little details regarding adding 
*                           EXCLUSIVE and ONLY stuff
* 15-Jul-88  -RJ            Changed all files to work with new includes
* 19-Feb-88  =RJ Mical=     Created this file!
* 
* *****************************************************************************


	INCLUDE "assembly.i"


	NOLIST
	INCLUDE "exec/types.i"

	INCLUDE "janus/janusbase.i"
	INCLUDE "janus/memory.i"
	INCLUDE "janus/services.i"

 	INCLUDE "serviceint.i"
	LIST

	INCLUDE "asmsupp.i"


	XLIB	AllocJanusMem
	XLIB	AllocMem
	XLIB	FindTask
	XLIB	Forbid
	XLIB	FreeJanusMem
	XLIB	FreeMem
	XLIB	JanusMemBase
	XLIB	JanusMemToOffset
	XLIB	JanusMemType
	XLIB	LockServiceData
	XLIB	ObtainSemaphore
	XLIB	Permit
	XLIB	ReleaseSemaphore
	XLIB	SendJanusInt
	XLIB	UnlockServiceData
	XLIB	WaitTOF

	XREF	ExpungeService
	XREF	FindUnusedAToPC
	XREF	FreeServiceCustomer
	XREF	FreeSDSemaphore
	XREF	InformDeletedService
	XREF	KillServiceCustomer
	XREF	SignalCustomers
	XREF	WaitServiceParam

	XDEF	CallService
	XDEF	ReleaseService
	XDEF	DeleteService


	XLIB	MakeBytePtr
	XLIB	MakeWordPtr




* =============================================================================
* === IMPLEMENTATION ALGORITHMS ===============================================
* =============================================================================
* 
* This section is for Torsten and RJ.  These notes are *not* to be released 
* to the real world, please.



CallService:
* === CallService() ===========================================================
* 
* NAME
*     CallService  --  Signal all other users of the Janus Service
* 
* 
* SYNOPSIS
*     VOID CallService(ServiceData)
*                      A0
*         struct ServiceData *ServiceData;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     This routine sends a signal to the users of the service associated with 
*     the specified ServiceData structure.  Note that the task that calls 
*     CallService() will not be signalled as a result of the call.
*     
*     The ServiceData structure pointer that you provide to this routine 
*     doesn't have to point to any particular Janus memory-access address 
*     space (although it must point to Janus memory of course).  What this 
*     means is that if you translate the ServiceData pointer you get from 
*     AddService() or GetService() from word-access to byte-access or 
*     anything else, you don't have to translate it back before calling 
*     CallService().  
*     
*     
* EXAMPLE
*     struct ServiceData *SData = NULL;
*         if (GetService(&SData, ...) == JSERV_OK)
*             {
*             /* Note that turning SData into a byte pointer doesn't hurt */
*             SData = (struct ServiceData *)MakeBytePtr(SData);
*             CallService(SData);
*             ReleaseService(SData);
*             SData = NULL;
*             }
*     
*     
* INPUTS
*     ServiceData = address of a ServiceData structure.  This may point to 
*         any type of Janus memory-access address, not necessarily word-access
*     
*     
* RESULTS
*     None
*     
*     
* SEE ALSO
*     AddService(), GetService()

CALLS_REGS	REG	A2-A6/D2

		MOVEM.L	CALLS_REGS,-(SP)

		CALLSYS	MakeWordPtr	; Make sure ServiceData is word-access
		MOVE.L	A0,D2

	IFGE	INFOLEVEL-1
	MOVEM.L	D2,-(SP)
	PUTMSG	1,<'%s/CallService( $%lx )'>
	LEA	1*4(SP),SP
	ENDC


		;------	Before anything, call this completely 
		;------	non-register-destructive routine which waits until 
		;------	the ServiceTask has allocated and initialized 
		;------	the ServiceParam data structure
		JSR	WaitServiceParam


* Throughout this routine, except for local changes (which must be repaired
* afterwards), these registers will be constant:
*    D2   = ServiceData.WA
*    A2   = ExecBase
*    A4   = ServiceParam.BA
*    A5   = ServiceParam.WA
*    A6   = JanusBase, for janus library
		MOVEA.L	jb_ExecBase(A6),A2
		MOVEA.L	jb_ServiceBase(A6),A5
		MOVEA.L	sb_ServiceParam(A5),A5
		MOVEA.L	A5,A0
		CALLSYS	MakeBytePtr
		MOVEA.L	A0,A4


	PUTMSG	59,<'%s/CallService about to LockSD()'>
* - if this side of the Bridge supports multiple customers 
* 	- LockServiceData()
		MOVEA.L	D2,A0
		CALLSYS	LockServiceData

* 	- if the UserCount on this side of the Bridge is greater than zero, 
* 	  SignalCustomers() on this side of the Bridge
		MOVEA.L	D2,A0
		CALLSYS	MakeBytePtr
		TST.B	sd_AmigaUserCount(A0)
		BEQ	customersDone
	MOVE.L	A1,-(SP)
	MOVE.L	#0,A1
	EXECLIB	FindTask
	MOVE.L	(SP)+,A1
	MOVE.L	D0,-(SP)
	PUTMSG	59,<'%s/$%lx: CallService about to SignalCustomers()'>
	LEA	4(SP),SP
		MOVEA.L	D2,A0
		MOVEQ.L	#2,D0	; from Amiga
		JSR	SignalCustomers
customersDone:

	PUTMSG	59,<'%s/CallService about to UnlockSD'>
* 	- UnlockServiceData()
		MOVEA.L	D2,A0
		CALLSYS	UnlockServiceData

	PUTMSG	59,<'%s/CallService about to ObtainSemaphore()'>
* - tell the other CPU about this CallService()
* 	- if we're the Amiga lock the ServiceBase semaphore
		MOVEA.L	jb_ServiceBase(A6),A0
		LEA.L	sb_ServiceSemaphore(A0),A0
		EXG.L	A6,A2
		CALLSYS	ObtainSemaphore
;SEMS		CALLSYS	Forbid
		EXG.L	A6,A2

	PUTMSG	59,<'%s/CallService about to find unused AToPC'>
* 	- call FindUnusedAToPC() if we're the Amiga or 
* 	  call FindUnusedPCToA() if we're the PC
		JSR	FindUnusedAToPC

	PUTMSG	59,<'%s/CallService about to write field'>
* 	- write the ServiceData offset to the unused field
		MOVE.L	A0,-(SP)
		MOVE.L	D2,D0
		CALLSYS	JanusMemToOffset
		MOVE.L	(SP)+,A0
		MOVE.W	D0,(A0)

* 	- if we're the Amiga unlock the ServiceBase semaphore
	PUTMSG	59,<'%s/CallService ReleaseSemaphore()'>
		MOVEA.L	jb_ServiceBase(A6),A0
		LEA.L	sb_ServiceSemaphore(A0),A0
		EXG.L	A6,A2
		CALLSYS	ReleaseSemaphore
;SEMS		CALLSYS	Permit
		EXG.L	A6,A2

* 	- trigger the appropriate service interrupt
		MOVE.L	#JSERV_PCSERVICE,D0	; Amiga initiates service 
		CALLSYS	SendJanusInt

	PUTMSG	59,<'%s/CallService done'>

		MOVEM.L	(SP)+,CALLS_REGS
		RTS



ReleaseService:
* === ReleaseService() ========================================================
* 
* NAME
*     ReleaseService  --  Release a Janus Service 
* 
* 
* SYNOPSIS
*     VOID ReleaseService(ServiceData)
*                         A0
*         struct ServiceData *ServiceData;
*     From assembly:  A6 has pointer to JanusBase
* 
* FUNCTION
*     After you are through using a service or after you detect that a 
*     service's ServiceData structure has its SERVICE_DELETED flag set, 
*     you release your usage of the service by calling ReleaseService().  
*     After all who are attached to a service have released it, the 
*     service can then be removed from the system.  
*     
*     If you call GetService(), you are obliged to call ReleaseService() 
*     sooner or later.  
*     
*     The ServiceData structure pointer that you provide to this routine 
*     doesn't have to point to any particular Janus memory-access address 
*     space (although it must point to Janus memory of course).  What this 
*     means is that if you translate the ServiceData pointer you get from 
*     AddService() or GetService() from word-access to byte-access or 
*     anything else, you don't have to translate it back before calling 
*     ReleaseService().  
*     
*     
* EXAMPLE
*     struct ServiceData *SData = NULL;
*         if (GetService(&SData, ...) == JSERV_OK)
*             {
*             /* Note that turning SData into a byte pointer doesn't hurt */
*             SData = (struct ServiceData *)MakeBytePtr(SData);
*             CallService(SData);
*             ReleaseService(SData);
*             SData = NULL;
*             }
*     
*     
* INPUTS
*     ServiceData = address of a ServiceData structure.  This may be any 
*         type of Janus memory-access address, not necessarily word-access
*     
*     
* RESULTS
*     None
*     
*     
* SEE ALSO
*     GetService()


RELS_REGS	REG	A2-A6

		MOVEM.L	RELS_REGS,-(SP)


	IFGE	INFOLEVEL-1
	MOVEM.L	A0,-(SP)
	PUTMSG	1,<'%s/ReleaseService( $%lx )'>
	LEA	1*4(SP),SP
	ENDC


		;------	Before anything, call this completely 
		;------	non-register-destructive routine which waits until 
		;------	the ServiceTask has allocated and initialized 
		;------	the ServiceParam data structure
		JSR	WaitServiceParam


* Throughout this routine, except for local changes (which must be repaired
* afterwards), these registers will be constant:
*    A2   = ExecBase
*    A4   = ServiceData.WA
*    A5   = ServiceBase.WA
*    A6   = JanusBase, for janus library
		CALLSYS	MakeWordPtr
		MOVEA.L	A0,A4
		MOVEA.L	jb_ExecBase(A6),A2
		MOVEA.L	jb_ServiceBase(A6),A5


* - if we're the Amiga lock the ServiceBase semaphore
;SEMS
		LEA.L	sb_ServiceSemaphore(A5),A0
		EXG.L	A6,A2
		CALLSYS	ObtainSemaphore
;SEMS		CALLSYS	Forbid
		EXG.L	A6,A2


* - lock the ServiceParam lock
		MOVEA.L	sb_ServiceParam(A5),A0
		CALLSYS	MakeBytePtr
		LOCK	spm_Lock(A0)


* - LockServiceData() 
		MOVEA.L	A4,A0
		CALLSYS	LockServiceData


* - if this processor's UserCount is greater than zero, the service was
*   added so decrement the user count by us
* 	- if we're the Amiga decrement AmigaUserCount
* 	- if we're the PC decrement PCUserCount
		MOVEA.L	A4,A0
		CALLSYS	MakeBytePtr
		TST.B	sd_AmigaUserCount(A0)
		BEQ	1234$
		SUBQ.B	#1,sd_AmigaUserCount(A0)
1234$

* - KillServiceCustomer() for this customer
		MOVEA.L	A4,A0
		JSR	KillServiceCustomer

* - if we're the Amiga and there are no more Amiga customers, 
*   then FreeSDSemaphore(ServiceData)
		TST.L	sd_FirstAmigaCustomer(A4)
		BNE	REL_UNLOCK
		MOVEA.L	A4,A0
		JSR	FreeSDSemaphore

* - if the ServiceData's FirstAmigaCustomer equals NULL and the 
*   FirstPCCustomer equals -1 then the service has no more 
*   customers on either side of the Bridge so ExpungeService() 
*   [Torsten, is this right for the PC?  
*   Also, make sure you see the ExpungeService() 
*   logic for stuff that that routine is supposed to do]
		TST.L	sd_FirstAmigaCustomer(A4)
		BNE	REL_UNLOCK
		CMP.L	#-1,sd_FirstPCCustomer(A4)
		BNE	REL_UNLOCK
		MOVE.L	A4,A0
		JSR	ExpungeService
		BRA	REL_UNLOCK_DONE

REL_UNLOCK:
* - else there's still some other customers out there, so set things back 
*   the way they should be
* 	- unlock the ServiceData lock
		MOVEA.L	A4,A0
		CALLSYS	MakeBytePtr
		UNLOCK	sd_ServiceDataLock(A0)

* 	- if we're the Amiga and there's still an Amiga customer, 
* 	  the ServiceData semaphore must still be out there so unlock it
		TST.L	sd_FirstAmigaCustomer(A4)
		BEQ	REL_UNLOCK_DONE
		MOVEA.L	sd_Semaphore(A4),A0
		EXG.L	A6,A2
		CALLSYS	ReleaseSemaphore
;SEMS		CALLSYS	Permit
		EXG.L	A6,A2

REL_UNLOCK_DONE:


	PUTMSG	59,<'%s/ReleaseService almost done'>

* - unlock the ServiceParam lock
		MOVEA.L	sb_ServiceParam(A5),A0
		CALLSYS	MakeBytePtr
		UNLOCK	spm_Lock(A0)

* - if we're the Amiga unlock the ServiceBase semaphore
;SEMS
		LEA.L	sb_ServiceSemaphore(A5),A0
		EXG.L	A6,A2
		CALLSYS	ReleaseSemaphore
;SEMS		CALLSYS	Permit
		EXG.L	A6,A2

	PUTMSG	59,<'%s/ReleaseService done'>

		MOVEM.L	(SP)+,RELS_REGS
		RTS



DeleteService:
* === DeleteService() =========================================================
* 
* NAME
*     DeleteService  --  Delete a Janus Service 
* 
* 
* SYNOPSIS
*     VOID DeleteService(ServiceData)
*                        A0
*         struct ServiceData *ServiceData;
*     From assembly:  A6 has pointer to JanusBase
* 
*     
* FUNCTION
*     After you are through with a service that you have added to the 
*     system, you initiate the process of removing the service from 
*     the system by calling DeleteService().  The service will then 
*     be marked as SERVICE_DELETED and all users of the service will 
*     be signalled, at which time they are supposed to notice that 
*     the service has been deleted and call ReleaseService().  
*     
*     After calling this routine, subsequent attempts to GetService() 
*     will find the service not present, and a subsequent call to 
*     AddService() with the same Application ID and Local ID will succeed, 
*     even though this particular instance of the service may not have 
*     been completely removed from the system yet (the service isn't 
*     completely removed until all users of the service have made 
*     the ReleaseService() call).
*     
*     If you call AddService() successfully, you are obliged to call 
*     DeleteService() sooner or later.  
*     
*     The ServiceData structure pointer that you provide to this routine 
*     doesn't have to point to any particular Janus memory-access address 
*     space (although it must point to Janus memory of course).  What this 
*     means is that if you translate the ServiceData pointer you get from 
*     AddService() into a byte-access pointer or anything else, you don't 
*     have to translate it back before calling ReleaseService().  
*     
*     
* EXAMPLE
*     struct ServiceData *SData = NULL;
*         if (AddService(&SData, ...) == JSERV_OK)
*             {
*             /* Note that turning SData into a byte pointer doesn't hurt */
*             SData = (struct ServiceData *)MakeBytePtr(SData);
*             DeleteService(SData);
*             SData = NULL;
*             }
*     
*     
* INPUTS
*     ServiceData = address of a ServiceData structure.  This may be any 
*         type of Janus memory-access address, not necessarily word-access
*     
*     
* RESULTS
*     None
*     
*     
* SEE ALSO
*     AddService()


DELS_REGS	REG	A2-A6

		MOVEM.L	DELS_REGS,-(SP)

	IFGE	INFOLEVEL-1
	MOVEM.L	A0,-(SP)
	PUTMSG	1,<'%s/DeleteService( $%lx )'>
	LEA	1*4(SP),SP
	ENDC


		;------	Before anything, call this completely 
		;------	non-register-destructive routine which waits until 
		;------	the ServiceTask has allocated and initialized 
		;------	the ServiceParam data structure
		JSR	WaitServiceParam


* Throughout this routine, except for local changes (which must be repaired
* afterwards), these registers will be constant:
*    A2   = ExecBase
*    A4   = ServiceData.WA
*    A5   = ServiceParam.WA
*    A6   = JanusBase, for janus library
		CALLSYS	MakeWordPtr
		MOVEA.L	A0,A4
		MOVEA.L	jb_ExecBase(A6),A2
		MOVEA.L	jb_ServiceBase(A6),A5
		MOVEA.L	sb_ServiceParam(A5),A5


* - if we're the Amiga lock the ServiceBase semaphore
;SEMS
		MOVEA.L	jb_ServiceBase(A6),A0
		LEA.L	sb_ServiceSemaphore(A0),A0
		EXG.L	A6,A2
		CALLSYS	ObtainSemaphore
;SEMS		CALLSYS	Forbid
		EXG.L	A6,A2

* - lock the ServiceParam lock
		MOVEA.L	A5,A0
		CALLSYS	MakeBytePtr
		LOCK	spm_Lock(A0)

* - LockServiceData()
		MOVEA.L	A4,A0
		CALLSYS	LockServiceData

* - KillServiceCustomer() for this customer
		MOVEA.L	A4,A0
		JSR	KillServiceCustomer

* - if we're the Amiga decrement AmigaUserCount
* - if we're the PC decrement PCUserCount
		MOVEA.L	A4,A0
		CALLSYS	MakeBytePtr
		SUBQ.B	#1,sd_AmigaUserCount(A0)

* - if we're the Amiga and AmigaUserCount == 0 then free up 
*   the ServiceData semaphore
		TST.B	sd_AmigaUserCount(A0)
		BNE	5$
		MOVEA.L	A4,A0
		JSR	FreeSDSemaphore
5$

* - if AmigaUserCount == 0 and PCUserCount == 0
* 	- ExpungeService(ServiceData)
		MOVEA.L	A4,A0
		CALLSYS	MakeBytePtr
		TST.B	sd_AmigaUserCount(A0)
		BNE	DEL_OTHERS
		TST.B	sd_PCUserCount(A0)
		BNE	DEL_OTHERS
		MOVEA.L	A4,A0
		JSR	ExpungeService

* 	- unlock the ServiceParam lock
		MOVEA.L	A5,A0
		CALLSYS	MakeBytePtr
		UNLOCK	spm_Lock(A0)

* 	- if we're the Amiga unlock the ServiceBase semaphore
		MOVEA.L	jb_ServiceBase(A6),A0
;SEMS
		LEA.L	sb_ServiceSemaphore(A0),A0
		EXG.L	A6,A2
		CALLSYS	ReleaseSemaphore
;SEMS		CALLSYS	Permit
		EXG.L	A6,A2

*	- goto DELSERVICE_DONE
		BRA	DELSERVICE_DONE

DEL_OTHERS:
* - else as there's still some customers out there, we're going to tell them 
*   about the delete:
* 	- set the ServiceData's SERVICE_DELETED flag
		MOVE.W	sd_Flags(A4),D0
		BSET	#SERVICE_DELETEDn,D0
		MOVE.W	D0,sd_Flags(A4)

* 	- InformDeletedService(ServiceData) to tell customers on this 
* 	  side of the Bridge
		MOVEA.L	A4,A0
		JSR	InformDeletedService

* 	- UnlockServiceData()
		MOVEA.L	A4,A0
		CALLSYS	UnlockServiceData

* 	- unlock the ServiceParam lock
		MOVEA.L	A5,A0
		CALLSYS	MakeBytePtr
		UNLOCK	spm_Lock(A0)

* 	- if we're the Amiga unlock the ServiceBase semaphore
		MOVEA.L	jb_ServiceBase(A6),A0
;SEMS
		LEA.L	sb_ServiceSemaphore(A0),A0
		EXG.L	A6,A2
		CALLSYS	ReleaseSemaphore
;SEMS		CALLSYS	Permit
		EXG.L	A6,A2

* 	- signal the other processor with an DeletesService
		MOVEA.L	jb_ServiceBase(A6),A3
		MOVE.L	sb_ServiceParam(A3),A3

	EXG.L	A6,A2
	CALLSYS	Forbid
	EXG.L	A6,A2

DELETES_wait:
		CMP.W	#-1,spm_AmigaDeletesService(A3)
		BEQ	DELETES_del0
		CMP.W	#-1,spm_AmigaDeletesService+2(A3)
		BEQ	DELETES_del1
	PUTMSG	2,<'%s/still waiting to DELETE_SERVICE...'>
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
		BRA	DELETES_wait

DELETES_del1:
		;------	We're going to transmit using the second element of the 
		;------	16-bit array, so offset the structure pointer by two
		ADD.W	#2,A3
DELETES_del0:
		MOVE.L	A4,D0
		CALLSYS	JanusMemToOffset
		MOVE.W	D0,spm_AmigaDeletesService(A3)

	EXG.L	A6,A2
	CALLSYS	Permit
	EXG.L	A6,A2

		MOVE.L	#JSERV_PCSERVICE,D0	; Amiga initiates service 
		CALLSYS	SendJanusInt


DELSERVICE_DONE:
* - bye, thanks for calling
	PUTMSG	59,<'%s/DeleteService done'>

		MOVEM.L	(SP)+,DELS_REGS
		RTS



		END


