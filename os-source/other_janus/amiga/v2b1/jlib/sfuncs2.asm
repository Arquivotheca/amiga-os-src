
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
;NOSEMS	XLIB	ObtainSemaphore
	XLIB	Permit
;NOSEMS	XLIB	ReleaseSemaphore
	XLIB	SendJanusInt
	XLIB	WaitTOF


 IFD	CHANNELING
	XREF	ChannelMatch
 ENDC	; of IFD CHANNELING

	XREF	ExpungeService
	XREF	FindUnusedAToPC
	XREF	FreeServiceCustomer
	XREF	InformDeletedService
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


* IFD CHANNELING
* VOID CallService(ChannelNumber)
*                  D0:0-7
*    UBYTE  ChannelNumber;
* ENDC ; of IFD CHANNELING
* 
* IFND CHANNELING
* VOID CallService(ServiceData)
*                  A0
*    struct ServiceData *ServiceData;
* ENDC ; of IFND CHANNELING
* 
* This routine doesn't care whether the ServiceData structure points to 
* word-access or byte-access memory.
* 
* From assembly:  JanusBase should be in A6


CALLS_REGS	REG	A2-A6/D2-D7

		MOVEM.L	CALLS_REGS,-(SP)

 IFD	CHANNELING
		MOVEQ.L	#0,D2		; Grab a safe copy of the channel num,
		MOVE.B	D0,D2		; stripped to 8 bits as advertised
 ENDC	; of IFD CHANNELING
 IFND	CHANNELING
		CALLSYS	MakeWordPtr	; Make sure ServiceData is word-access
		MOVE.L	A0,D2
 ENDC	; of IFND CHANNELING


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
*  IFD CHANNELING
*    D2.B = Channel number
*  ENDC ; of IFD CHANNELING
*  IFND CHANNELING
*    D2   = ServiceData.WA
*  ENDC ; of IFND CHANNELING
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


* - if we're the Amiga
* 	- lock the ServiceBase semaphore
		MOVEA.L	jb_ServiceBase(A6),A0
;NOSEMS		LEA	sb_ServiceSemaphore(A0),A0
		EXG.L	A6,A2
;NOSEMS		CALLSYS	ObtainSemaphore
		CALLSYS	Forbid
		EXG.L	A6,A2


* - lock the ServiceParam lock
		LOCK	spm_Lock(A4)


* - find the ServiceData with this address:  
		MOVEA.L	D2,A3
		MOVE.W	sd_Flags(A3),D0
		BTST	#SERVICE_DELETEDn,D0
		BNE	CALLSERVICE_ERROR
		MOVEA.L	D2,A0
		CALLSYS	MakeBytePtr
		TST.B	sd_AmigaUserCount(A0)
		BNE	CHANNEL_FOUND
		TST.B	sd_PCUserCount(A0)
		BEQ	CALLSERVICE_ERROR
		;------	else fall into CHANNEL_FOUND

CHANNEL_FOUND:
	PUTMSG	59,<'%s/CallService valid ServiceData'>

* NEW:  unlock the ServiceParam lock now, rather than below at the end 
* - unlock the ServiceParam lock
		UNLOCK	spm_Lock(A4)

 IFD	CHANNELING
* Here, we know that A3 has the word-access and A0 has the byte-access pointers
* to the ServiceData of the channel
* 
* - find a zero byte in the ServiceParam AmigaToPC[] array
* 	- if none found, wait politely until one is zero; if one never 
* 	  becomes zero, the other side is hung, and now so are we 
		JSR	FindUnusedAToPC


* - write the channel number to the location of the array
		MOVE.B	D2,(A0)
 ENDC	; of IFD CHANNELING

 IFND	CHANNELING
* Here, we know that A3 has the word-access and A0 has the byte-access pointers
* to the ServiceData of the channel
* 
* - find an unused field in the ServiceParam's AmigaToPC[] array
* 	- if none found, wait politely until one is found; if one never 
* 	  becomes zero, the other side is hung, and now so are we 
		JSR	FindUnusedAToPC


* - write the ServiceData offset to the location of the array
		MOVE.L	A0,-(SP)
		MOVE.L	D2,D0
		CALLSYS	JanusMemToOffset
		MOVE.L	(SP)+,A0
		MOVE.W	D0,(A0)
 ENDC	; of IFND CHANNELING

* - trigger the appropriate service interrupt
		MOVE.L	#JSERV_PCSERVICE,D0	; Amiga initiates service 
		CALLSYS	SendJanusInt


* - traverse the CustomerList of the ServiceData, and for any Customer whose
*   task isn't the current one, send a signal
		MOVEA.L	A3,A0
		JSR	SignalCustomers

		BRA	CALLSERVICE_DONE


CALLSERVICE_ERROR:
	PUTMSG	59,<'%s/CallService channel not found'>

* NEW:  unlock the ServiceParam lock now, rather than below at the end 
* - unlock the ServiceParam lock
		UNLOCK	spm_Lock(A4)


CALLSERVICE_DONE:
	PUTMSG	59,<'%s/CallService almost done'>

* - if we're the Amiga
* 	- unlock the ServiceBase semaphore
		MOVEA.L	jb_ServiceBase(A6),A0
;NOSEMS		LEA	sb_ServiceSemaphore(A0),A0
		EXG.L	A6,A2
;NOSEMS		CALLSYS	ReleaseSemaphore
		CALLSYS	Permit
		EXG.L	A6,A2

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


RELS_REGS	REG	A2-A6/D2-D7

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


* - if we're the Amiga
* 	- lock the ServiceBase semaphore
;NOSEMS		LEA	sb_ServiceSemaphore(A5),A0
		EXG.L	A6,A2
;NOSEMS		CALLSYS	ObtainSemaphore
		CALLSYS	Forbid
		EXG.L	A6,A2


* - lock the ServiceParam lock
		MOVEA.L	sb_ServiceParam(A5),A0
		CALLSYS	MakeBytePtr
		LOCK	spm_Lock(A0)


* - if we're the Amiga
* 	- find the ServiceCustomer structure that matches this task
		MOVEA.L	#0,A1		; Get the address of this task into D0
		EXG.L	A2,A6
		CALLSYS	FindTask
		EXG.L	A2,A6
		MOVEA.L	#0,A3		; No struct has been examined yet
		MOVEA.L	sd_FirstAmigaCustomer(A4),A1

RELTASK_LOOP:	CMPA.L	#0,A1		; Did we reach the end of the Customers?
		BEQ	RELSERVICE_ERROR ; branch if so
		CMP.L	scs_Task(A1),D0	; Compare customer's task with caller's
		BEQ	RELTASK_FOUND	; If equal, this is the one we want!
		MOVEA.L	A1,A3		; else save this struct address 
		MOVEA.L	scs_NextCustomer(A1),A1 ; get the next struct 
		BRA	RELTASK_LOOP	; and go try again

RELTASK_FOUND:
* Here, A1 has the address of this task's ServiceCustomer structure, and A3 
* has the address of the previous structure examined or NULL if no 
* structure was previously examined
* 	- unlink the ServiceCustomer structure
		CMPA.L	#0,A3		; Did we see some other structure?
		BNE	10$		; Branch if yes
		;------	else stuff ptr to next into ServiceData's first ptr
		MOVE.L	scs_NextCustomer(A1),sd_FirstAmigaCustomer(A4)
		BRA	20$
10$		MOVE.L	scs_NextCustomer(A1),scs_NextCustomer(A3)
20$

* 	- free the memory of the ServiceCustomer structure
		JSR	FreeServiceCustomer


* - Test the UserCount of this ServiceData
* - if AmigaUserCount == 0 and PCUserCount == 0
		MOVE.L	A4,A0
		CALLSYS	MakeBytePtr
		TST.B	sd_AmigaUserCount(A0)
		BNE	REL_DECUSERS
		TST.B	sd_PCUserCount(A0)
		BNE	REL_DECUSERS

* 	- The guy who called ReleaseService() is the same guy who 
* 	  was waiting for a service that never came available.  
* 	  The ServiceCustomer structure for this task is already removed 
* 	  so check if there are any other ServiceCustomers 
* 	  waiting for this task and if not then ExpungeService()
		TST.L	sd_FirstAmigaCustomer(A4)
		BNE	REL_COUNTDONE
		TST.L	sd_FirstAmigaCustomer(A4)
		BNE	REL_COUNTDONE
		BRA	REL_EXPUNGE

REL_DECUSERS:
* - else if AmigaUserCount > 0 or PCUserCount > 0
* 	- if we're the Amiga
* 		- decrement AmigaUserCount
* 	- if we're the PC
* 		- decrement PCUserCount
* 	- if AmigaUserCount == 0, and PCUserCount == 0, 
* 	  this service has no more users so ExpungeService(ServiceData)
		SUBQ.B	#1,sd_AmigaUserCount(A0)
		BNE	REL_COUNTDONE
REL_EXPUNGE:	MOVE.L	A4,A0
		JSR	ExpungeService

REL_COUNTDONE:
		BRA	RELSERVICE_DONE


RELSERVICE_ERROR:
	PUTMSG	1,<'%s/ReleaseService customer for this task not found'>


RELSERVICE_DONE:
	PUTMSG	59,<'%s/ReleaseService almost done'>

* - unlock the ServiceParam lock
		MOVEA.L	sb_ServiceParam(A5),A0
		CALLSYS	MakeBytePtr
		UNLOCK	spm_Lock(A0)

* - if we're the Amiga
* 	- unlock the ServiceBase semaphore
;NOSEMS		LEA	sb_ServiceSemaphore(A5),A0
		EXG.L	A6,A2
;NOSEMS		CALLSYS	ReleaseSemaphore
		CALLSYS	Permit
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


DELS_REGS	REG	A2-A6/D2-D7

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


* - if we're the Amiga
* 	- lock the ServiceBase semaphore
;NOSEMS		MOVEA.L	jb_ServiceBase(A6),A0
;NOSEMS		LEA	sb_ServiceSemaphore(A0),A0
		EXG.L	A6,A2
;NOSEMS		CALLSYS	ObtainSemaphore
		CALLSYS	Forbid
		EXG.L	A6,A2


* - lock the ServiceParam lock
		MOVEA.L	A5,A0
		CALLSYS	MakeBytePtr
		LOCK	spm_Lock(A0)


* - set the ServiceData's SERVICE_DELETED flag
		MOVE.W	sd_Flags(A4),D0
		BSET	#SERVICE_DELETEDn,D0
		MOVE.W	D0,sd_Flags(A4)


* - if we're the Amiga
* 	- decrement AmigaUserCount
* - if we're the PC
* 	- decrement PCUserCount
		MOVEA.L	A4,A0
		CALLSYS	MakeBytePtr
		SUBQ.B	#1,sd_AmigaUserCount(A0)


* NEW:  we have to do some fancy unlock-dancing:
* NEW:     If AmigaUserCount == 0 and PCUserCount == 0, we are going to: 
* NEW:         o  expunge the service
* NEW:         o  unlock the ServiceParam
* NEW:         o  continue
* NEW:     else if AmigaUserCount != 0 and PCUserCount != 0:
* NEW:         o  unlock the ServiceParam
* NEW:         o  signal the PC
* NEW:         o  continue

* - if AmigaUserCount == 0 and PCUserCount == 0
* 	- ExpungeService(ServiceData)
		TST.B	sd_AmigaUserCount(A0)
		BNE	10$
		TST.B	sd_PCUserCount(A0)
		BNE	10$
		MOVEA.L	A4,A0
		JSR	ExpungeService

* 	o NEW:  unlock the ServiceParam here after expunging 
* 	- unlock the ServiceParam lock
		MOVEA.L	A5,A0
		CALLSYS	MakeBytePtr
		UNLOCK	spm_Lock(A0)

		BRA	DELCOUNT_DONE

10$
* - else if AmigaUserCount != 0 or PCUserCount != 0
* 	o NEW:  unlock the ServiceParam here before trying to tell the 
* 	  PC about it 
* 	- unlock the ServiceParam lock
		MOVEA.L	A5,A0
		CALLSYS	MakeBytePtr
		UNLOCK	spm_Lock(A0)

* - signal the other processor with an DeletesService
		MOVEA.L	jb_ServiceBase(A6),A3
		MOVE.L	sb_ServiceParam(A3),A3

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

		MOVE.L	#JSERV_PCSERVICE,D0	; Amiga initiates service 
		CALLSYS	SendJanusInt

		BRA	DELSERVICE_DONE


* 	- if we're the Amiga
* 		- InformDeletedService(ServiceData)
		MOVEA.L	A4,A0
		JSR	InformDeletedService


DELCOUNT_DONE:


DELSERVICE_DONE:
	PUTMSG	59,<'%s/DeleteService almost done'>

* - if we're the Amiga
* 	- unlock the ServiceBase semaphore
		MOVEA.L	jb_ServiceBase(A6),A0
;NOSEMS		LEA	sb_ServiceSemaphore(A0),A0
		EXG.L	A6,A2
;NOSEMS		CALLSYS	ReleaseSemaphore
		CALLSYS	Permit
		EXG.L	A6,A2

	PUTMSG	59,<'%s/DeleteService done'>

		MOVEM.L	(SP)+,DELS_REGS
		RTS



		END


