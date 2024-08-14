*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **		     Semaphores 		 **
*	   **						 **
*	   ************************************************

*************************************************************************
*									*
*   Copyright 1984,85,88,89,90,91 Commodore-Amiga, Inc.			*
*   All rights reserved.  No part of this program may be reproduced,	*
*   transmitted, transcribed, stored in retrieval system, or		*
*   translated into any language or computer language, in any form	*
*   or by any means, electronic, mechanical, magnetic, optical, 	*
*   chemical, manual or otherwise, without the prior written		*
*   permission of Commodore-Amiga, Incorporated.			*
*									*
*************************************************************************

**********************************************************************
*
*   Source Control:
*
*	$Id: semaphores.asm,v 39.13 93/02/15 17:59:34 mks Exp $
*
*	$Log:	semaphores.asm,v $
* Revision 39.13  93/02/15  17:59:34  mks
* Removed the older Vacate/Procure message code...
* 
* Revision 39.12  92/07/28  09:44:59  mks
* Fixed the Shared semaphore bug that was introduced when I added
* the semaphore promotion code.
*
* Revision 39.11  92/07/02  08:49:07  mks
* Fixed InitSemaphore() so it does not trash the LN_PRI field...
*
* Revision 39.10  92/06/10  09:04:40  mks
* Fixed the semaphore documentation error and moved them into the source
* file (where they should be)
*
* Revision 39.9  92/05/28  19:04:36  mks
* Changed NEWLIST a0 to a bsr NewList to save ROM space
*
* Revision 39.8  92/03/16  14:43:56  mks
* Obtaining shared semaphore while you also own it exclusive now works
*
* Revision 39.7  92/03/16  13:31:30  mks
* Fixed FindSemaphore() and optimized the RemSemaphore case
*
* Revision 39.6  92/02/21  17:35:00  mks
* Fixed Procure()...
*
* Revision 39.5  92/02/19  15:44:40  mks
* Changed ALERT macro to MYALERT
*
* Revision 39.4  92/02/12  08:22:34  mks
* Fixed one more bug with respect to shared semaphores!  (Arg!!!)
*
* Revision 39.3  92/02/11  10:05:25  mks
* Fixed case of release semaphore with both a procure and a simple
* waiting node from the same task.
*
* Revision 39.2  92/02/10  11:00:39  mks
* Some cleanup in the semaphore code...  ReleaseSemaphore()
* and ObtainSemaphore() were cleaned up a bit...
*
* Revision 39.1  92/02/07  19:33:48  mks
* Major rework of the Semaphore system; mainly to fix a large
* bug and to added Procure() and Vacate() support to the
* SignalSemaphores.
*
* Revision 39.0  91/10/15  08:28:46  mks
* V39 Exec initial checkin
*
*********************************************************************


;****** Included Files ***********************************************

    NOLIST
    INCLUDE	"assembly.i"
    INCLUDE	"types.i"
    INCLUDE	"ables.i"
    INCLUDE	"nodes.i"
    INCLUDE	"lists.i"
    INCLUDE	"ports.i"
    INCLUDE	"tasks.i"
    INCLUDE	"semaphores.i"
    INCLUDE	"alerts.i"
    INCLUDE	"execbase.i"
    INCLUDE	"calls.i"
    LIST


;****** Imported Globals *********************************************

    TASK_ABLES

;****** Imported Functions *******************************************

    EXTERN_CODE AddTail
    EXTERN_CODE RemHead
    EXTERN_CODE AddNode
    EXTERN_CODE FindNode
    EXTERN_CODE NewList

    EXTERN_SYS	Signal
    EXTERN_SYS	Wait
    EXTERN_SYS	PutMsg
    EXTERN_SYS	GetMsg
    EXTERN_SYS	ReplyMsg
    EXTERN_SYS	InitSemaphore
    EXTERN_SYS	ReleaseSemaphore
    EXTERN_SYS	Alert

;****** Exported Functions *******************************************

    XDEF	Procure
    XDEF	Vacate
    XDEF	InitSemaphore
    XDEF	ObtainSemaphore
    XDEF	ObtainSemaphoreShared
    XDEF	ReleaseSemaphore
    XDEF	AttemptSemaphore
    XDEF	AttemptSemaphoreShared
    XDEF	ObtainSemaphoreList
    XDEF	ReleaseSemaphoreList
    XDEF	AddSemaphore
;    XDEF	RemSemaphore
    XDEF	FindSemaphore

******* exec.library/Procure **************************************************
*
*   NAME
*	Procure -- bid for a semaphore                                   (V39)
*
*   SYNOPSIS
*	Procure(semaphore, bidMessage)
*		A0	    A1
*
*	VOID Procure(struct SignalSemaphore *, struct SemaphoreMessage *);
*
*   FUNCTION
*	This function is used to obtain a semaphore in an async manner.
*	Like ObtainSemaphore(), it will obtain a SignalSemaphore for you
*	but unlike ObtainSemaphore(), you will not block until you get
*	the semaphore.  Procure() will just post a request for the semaphore
*	and will return.  When the semaphore is available (which could
*	be at any time) the bidMessage will ReplyMsg() and you will own
*	the semaphore.  This lets you wait on multiple semaphores at once
*	and to continue processing while waiting for the semaphore.
*
*	NOTE:  Pre-V39, Procure() and Vacate() did not work correctly.
*	They also did not operate on SignalSemaphore semaphores.
*	Old (and broken) MessageSemaphore use as of V39 will no longer work.
*
*   INPUT
*	semaphore - The SignalSemaphore that you wish to Procure()
*	bidMessage- The SemaphoreMessage that you wish replied when
*		you obtain access to the semaphore.  The message's
*		ssm_Semaphore field will point at the semaphore that
*		was obtained.  If the ssm_Semaphore field is NULL,
*		the Procure() was aborted via Vacate().
*		The mn_ReplyPort field of the message must point to
*		a valid message port.
*		To obtain a shared semaphore, the ln_Name field
*		must be set to 1.  For an exclusive lock, the ln_Name
*		field must be 0.  No other values are valid.
*
*   BUGS
*	Before V39, Procure() and Vacate() used a different semaphore
*	system that was very broken.  This new system is only available
*	as of V39 even though the LVOs are the same.
*
*   SEE ALSO
*	ObtainSemaphoreShared(), InitSemaphore(), ReleaseSemaphore(),
*	AttemptSemaphore(), ObtainSemaphoreList(), Vacate(), ObtainSemaphore()
*
*******************************************************************************
*
* Note, that in order to keep the old procure working, we look for the NULL
* that has to be in the SignalSemaphore list header since it happens to be
* at the same point as the old Semaphore's list header LH_HEAD.
*
Procure:	tst.l	SS_WAITQUEUE+MLH_TAIL(a0)	; Check for NULL
		bne.s	OldProcure			; If not, old semaphore
*
* Ok, now we need to play "obtain semaphore with message" games...
*
		move.l	ThisTask(a6),d0			; Get the task...
		move.l	d0,SSM_SEMAPHORE(a1)		; Set up task pointer
		move.l	LN_NAME(a1),d1			; Get the type...
		move.l	d1,SSR_WAITER(a1)		; Set the waiter type
		beq.s	p_notShared			; If 0, not shared
		moveq.l	#0,d0				; Clear the task...
p_notShared:
		FORBID
		addq.w	#1,SS_QUEUECOUNT(a0)		; Count and test
		bne.s	p_inuse				; If in use...
*
* No one is using this so lets grab it...
*
		move.l	d0,SS_OWNER(a0)			; Set up the owner...
p_replyNow:	move.l	a0,SSM_SEMAPHORE(a1)		; Set up semaphore
		moveq.l	#0,d0				; Clear the SSR_WAITER
		move.l	d0,SSR_WAITER(a1)		; Clear it...
		addq.w	#1,SS_NESTCOUNT(a0)
		JSRSELF	ReplyMsg			; Reply it...
		JMP_PERMIT				' And exit...
*
* Ok, so the semaphore is in use...  Check the type of use...
*
p_inuse:	cmp.l	SS_OWNER(a0),d0			; Same owner?
		beq.s	p_replyNow			; If so, nest it...
*
		lea.l	SS_WAITQUEUE(a0),a0		; Get queue
		BSRSELF	AddTail				; Add to waiting list
		JMP_PERMIT				; And exit...
*
*******************************************************************************
*

* This is the old procure code for the old semaphore structures...
OldProcure:
	;	ADDQ.W	#1,SM_BIDS(A0)
	;	BNE.S	pr_bid
	;	MOVE.L	A1,SM_LOCKMSG(A0)
	;	MOVEQ.L #1,D0
;pr_exit:
	;	move.l	d0,-(sp)	; Save
		MYALERT	AN_BadSemaphore!AT_DeadEnd
	;	move.l	(sp)+,d0	; restore...
	;	RTS

pr_bid:
	;	JSRSELF PutMsg
	;	CLEAR	D0
	;	BRA.S	pr_exit


******* exec.library/Vacate ***************************************************
*
*   NAME
*	Vacate -- release a bitMessage from Procure()                    (V39)
*
*   SYNOPSIS
*	Vacate(semaphore, bidMessage)
*	       A0         A1
*
*	void Vacate(struct SignalSemaphore *,struct SemaphoreMessage *);
*
*   FUNCTION
*	This function can be used to release a semaphore obtained via
*	Procure().  However, the main purpose for this call is to be
*	able to remove a bid for a semaphore that has not yet responded.
*	This is required when a Procure() was issued and the program
*	no longer needs to get the semaphore and wishes to cancel the
*	Procure() request.  The canceled request will be replied with
*	the ssm_Semaphore field set to NULL.  If you own the semaphore,
*	the message was already replied and only the ssm_Semaphore field
*	will be cleared.
*
*	NOTE:  Pre-V39, Procure() and Vacate() did not work correctly.
*	They also did not operate on SignalSemaphore semaphores.
*	Old (and broken) MessageSemaphore use as of V39 will no longer work.
*
*   INPUT
*	semaphore - The SignalSemaphore that you wish to Vacate()
*	bidMessage- The SemaphoreMessage that you wish to abort.
*		The message's ssm_Semaphore field will be cleared.
*		The message will be replied if it is still on the waiting
*		list.  If it is not on the waiting list, it is assumed
*		that the semaphore is owned and it will be released.
*
*   BUGS
*	Before V39, Procure() and Vacate() used a different semaphore
*	system that was very broken.  This new system is only available
*	as of V39 even though the LVOs are the same.
*
*   SEE ALSO
*	ObtainSemaphoreShared(), InitSemaphore(), ReleaseSemaphore(),
*	AttemptSemaphore(), ObtainSemaphoreList(), Procure(), ObtainSemaphore()
*
*******************************************************************************
*
* Note, that in order to keep the old procure working, we look for the NULL
* that has to be in the SignalSemaphore list header since it happens to be
* at the same point as the old Semaphore's list header LH_HEAD.
*
Vacate:		move.l	SS_WAITQUEUE+MLH_TAIL(a0),d0	; Check for NULL
		bne.s	OldVacate			; If not, old semaphore
*
* Now, we need to check if the message is still a waiting message...
* Note that we will need to walk the list...  This could be slow...
*
		move.l	d0,SSM_SEMAPHORE(a1)	; Clear that value...
		move.l	d0,SSR_WAITER(a1)	; Clear waiter flag...
		FORBID				; Protect ourselves
		move.l	a1,d1			; Cache our message...
		move.l	SS_WAITQUEUE(a0),d0	; Get first node...
v_Search:	cmp.l	d0,d1			; Did we find it?
		beq.s	v_found			; If so, we will remove it
		move.l	d0,a1
		move.l	(a1),d0			; Get next node
		bne.s	v_Search		; If not end of list, look more
*
* Now, the node was not found so we will just clear the semaphore pointer
* and go to ReleaseSemaphore...
*
		JSRSELF	ReleaseSemaphore	; Release the Semaphore
		JMP_PERMIT			; Let people back in/exit...
*
* Ok, so the message was found...  Lets get it off the list and reply it...
*
v_found:	subq.w	#1,SS_QUEUECOUNT(a0)	; Un-count this node
		move.l	d1,a1			; Get node back...
		REMOVE				; Remove it (a0/a1 trash)
		move.l	d1,a1			; Get node back...
		JSRSELF	ReplyMsg		; Reply the message
		JMP_PERMIT			; Let people back in/exit...
*
*******************************************************************************
*
* This is the old procure code for the old semaphore structures...
OldVacate:
	;	CLR.L	SM_LOCKMSG(A0)
	;	SUBQ.W	#1,SM_BIDS(A0)
	;	BGE.S	va_next
va_exit:
		MYALERT	AN_BadSemaphore!AT_DeadEnd
	;	RTS

va_next:
	;	MOVE.L	A0,-(SP)
	;	JSRSELF GetMsg
	;	MOVE.L	(SP)+,A0
	;	MOVE.L	D0,SM_LOCKMSG(A0)
	;	BEQ.S	va_exit
	;	MOVE.L	D0,A1
	;	JSRSELF ReplyMsg
	;	BRA.S	va_exit

******* exec.library/InitSemaphore ********************************************
*
*   NAME
*	InitSemaphore -- initialize a signal semaphore
*
*   SYNOPSIS
*	InitSemaphore(signalSemaphore)
*		      A0
*
*	void InitSemaphore(struct SignalSemaphore *);
*
*   FUNCTION
*	This function initializes a signal semaphore and prepares it for
*	use.  It does not allocate anything, but does initialize list
*	pointers and the semaphore counters.
*
*	Semaphores are often used to protect critical data structures
*	or hardware that can only be accessed by one task at a time.
*	After initialization, the address of the SignalSemaphore may be
*	made available to any number of tasks.  Typically a task will
*	try to ObtainSemaphore(), passing this address in.  If no other
*	task owns the semaphore, then the call will lock and return
*	quickly.  If more tasks try to ObtainSemaphore(), they will
*	be put to sleep.  When the owner of the semaphore releases
*	it, the next waiter in turn will be woken up.
*
*	Semaphores are often preferable to the old-style Forbid()/Permit()
*	type arbitration.  With Forbid()/Permit() *all* other tasks are
*	prevented from running.  With semaphores, only those tasks that
*	need access to whatever the semaphore protects are subject
*	to waiting.
*
*   INPUT
*	signalSemaphore -- a signal semaphore structure (with all fields
*			   set to zero before the call)
*
*   SEE ALSO
*	ObtainSemaphore, ObtainSemaphoreShared, AttemptSemaphore,
*	ReleaseSemaphore, Procure, Vacate, exec/semaphores.h
*
*******************************************************************************
*
InitSemaphore:
		CLR.L	SS_OWNER(A0)
		CLR.W	SS_NESTCOUNT(A0)
		MOVE.W	#-1,SS_QUEUECOUNT(A0)
		MOVE.B	#NT_SIGNALSEM,LN_TYPE(A0)
		LEA.L	SS_WAITQUEUE(A0),A0
		bra	NewList		; (a0...  RTS)

******* exec.library/ObtainSemaphore ******************************************
*
*   NAME
*	ObtainSemaphore -- gain exclusive access to a semaphore
*
*   SYNOPSIS
*	ObtainSemaphore(signalSemaphore)
*			A0
*
*	void ObtainSemaphore(struct SignalSemaphore *);
*
*   FUNCTION
*	Signal semaphores are used to gain exclusive access to an object.
*	ObtainSemaphore is the call used to gain this access.  If another
*	user currently has the semaphore locked the call will block until
*	the object is available.
*
*	If the current task already has locked the semaphore and attempts to
*	lock it again the call will still succeed.  A "nesting count" is
*	incremented each time the current owning task of the semaphore calls
*	ObtainSemaphore().  This counter is decremented each time
*	ReleaseSemaphore() is called.  When the counter returns to zero the
*	semaphore is actually released, and the next waiting task is called.
*
*	A queue of waiting tasks is maintained on the stacks of the waiting
*	tasks.	Each will be called in turn as soon as the current task
*	releases the semaphore.
*
*	Signal Semaphores are different than Procure()/Vacate() semaphores.
*	The former requires less CPU time, especially if the semaphore is
*	not currently locked.  They require very little set up and user
*	thought.  The latter flavor of semaphore make no assumptions about
*	how they are used -- they are completely general.  Unfortunately
*	they are not as efficient as signal semaphores, and require the
*	locker to have done some setup before doing the call.
*
*   INPUT
*       signalSemaphore -- an initialized signal semaphore structure
*
*   NOTE
*	This function preserves all registers (see BUGS).
*
*   BUGS
*	Until V37, this function could destroy A0.
*
*   SEE ALSO
*	ObtainSemaphoreShared(), InitSemaphore(), ReleaseSemaphore(),
*	AttemptSemaphore(), ObtainSemaphoreList(), Procure(), Vacate()
*
*******************************************************************************
*
* must preserve a0, a1, d0, d1
*
*
* idle:		ss_NestCount  =0
* idle:		ss_QueueCount =-1
*

		CNOP	0,4	;Nice alignment for 68020
ObtainSemaphore:
		FORBID
		ADDQ.W	#1,SS_QUEUECOUNT(A0)
		BNE.S	os_inuse
		;------ no one is using the semaphore.	Set task ptr
		MOVE.L	ThisTask(A6),SS_OWNER(A0)
os_got_it:	ADDQ.W	#1,SS_NESTCOUNT(A0)
		JMP_PERMIT	;(exit)


os_inuse:
		move.l	a1,-(sp)		; Fast path for already held
		move.l	ThisTask(a6),a1		; Check if we own it...
		cmp.l	SS_OWNER(a0),a1		; If so, we will just take it
		bne.s	os_wait			; If not, we will need to wait
		move.l	(sp)+,a1		; Get back a1
		bra.s	os_got_it		; We got the semaphore...
*
* When we get here, we know we will need to wait...
* A1 is already saved on the stack and the new A1 contains ThisTask.
*
os_wait:	movem.l d0/d1/a0,-(sp)		; A1 is already on the stack

		;------ someone else has it.  wait for it
		LEA	-SSR_SIZE(SP),SP                ; local storage
		MOVE.L	A1,SSR_WAITER(SP)

		;------ clear out old signals
		BCLR.B	#SIGB_SINGLE,TC_SIGRECVD+3(A1)	; :: cheat!

		;------ link us onto waiter's list
		LEA	SS_WAITQUEUE(A0),A0
		MOVE.L	SP,A1
		BSRSELF AddTail

		;------ wait for the semaphore
		MOVEQ	#SIGF_SINGLE,D0
		JSRSELF Wait		;ReleaseSemaphore fills in SS_OWNER

		LEA	SSR_SIZE(SP),SP

		movem.l (sp)+,d0/d1/a0/a1	; Restore all
		JMP_PERMIT	;(exit)

******* exec.library/ReleaseSemaphore *****************************************
*
*   NAME
*	ReleaseSemaphore -- make signal semaphore available to others
*
*   SYNOPSIS
*	ReleaseSemaphore(signalSemaphore)
*			 A0
*
*	void ReleaseSemaphore(struct SignalSemaphore *);
*
*   FUNCTION
*	ReleaseSemaphore() is the inverse of ObtainSemaphore(). It makes
*	the semaphore lockable to other users.	If tasks are waiting for
*	the semaphore and this this task is done with the semaphore then
*	the next waiting task is signalled.
*
*	Each ObtainSemaphore() call must be balanced by exactly one
*	ReleaseSemaphore() call.  This is because there is a nesting count
*	maintained in the semaphore of the number of times that the current
*	task has locked the semaphore. The semaphore is not released to
*	other tasks until the number of releases matches the number of
*	obtains.
*
*	Needless to say, havoc breaks out if the task releases more times
*	than it has obtained.
*
*   INPUT
*       signalSemaphore -- an initialized signal semaphore structure
*
*   NOTE
*	This call is guaranteed to preserve all registers.
*
*   SEE ALSO
*	InitSemaphore(), ObtainSemaphore(), ObtainSemaphoreShared()
*
*******************************************************************************
*
* idle:		ss_NestCount  =0
* idle:		ss_QueueCount =-1
*
*	:TODO:  Document who wakes up.  (All shared waiters if any)
*	:TODO:  Add underflow tests to QUEUECOUNT (and test)
*
*
* Note:	Shared semaphores introduced a monkey wrench into the QUEUECOUNT
*	scheme.  The old code relied on the fact that only one person could
*	own the semaphore at a time.  Semaphores key off QUEUECOUNT to
*	determine if the semaphore is busy or not; thus QUEUECOUNT can't
*	also be used to count entries on the waiter list.
*
*	The check for NULL after the REMHEAD is to prevent walking the
*	waiter list when no one needs waking.
*
ReleaseSemaphore:
		FORBID				; We *MUST* protect here!
		subq.w	#1,SS_NESTCOUNT(a0)
		bne.s	rs_StillNested
*
		clr.l	SS_OWNER(a0)
		;------ this task is done.  is anyone waiting?
		subq.w	#1,SS_QUEUECOUNT(a0)
		bge.s	rs_WakeWaiters
rs_exit:	JMP_PERMIT			;exit
*
rs_StillNested:	bmi.s	rs_Alert	; release more times than obtain?
		subq.w	#1,SS_QUEUECOUNT(a0)
		bra.s	rs_exit			; get out...
*
rs_Alert:	move.l	a0,-(sp)		; Save a0
		MYALERT	AN_SemCorrupt		; checked recoverable
		move.l	(sp),a0			; Restore...  (first time)
		bsr	InitSemaphore		; Reinit it... (Best guess)
		move.l	(sp)+,a0		; Preserved a0 ;^)
		bra.s	rs_exit			; And get out (we may be dead)
*
*******************************************************************************
*
* New:  ReleaseSemaphore now needs to check for waiter nodes that are
* actually Procure() based Semaphore Messages.  The goal is to make sure
* that we do not slow down the ReleaseSemaphore for cases where it is not.
*
rs_WakeWaiters:	movem.l	d0/d1/a0/a1/a2/a3,-(sp)	; Save some registers...
		move.l	a0,a2			; Put semaphore into a2...
*
* Now, get the first waiter...  Note that there may not be any...
*
		lea.l	SS_WAITQUEUE(a2),a0	; Point at list...
		move.l	(a0),a1			; Get first node...
		move.l	(a1),d0			; Get and if it is end node
		beq.s	rs_PopPermitRts		; If end of list, exit...
		move.l	d0,(a0)			; Poke in node pointer
		exg.l	d0,a1			; swap pointer and node
		move.l	a0,LN_PRED(a1)		; Store back pointer...
*
* Now we have a node in d0.  We need to check it.  The node can be a
* number of things.  It is one of 4 possible answers:
*	A simple task waiting for the semaphore
*	A simple task waiting for the semaphore (shared)
*	A procure message waiting for the semaphore
*	A procure message waiting for the semaphore (shared)
*
		move.l	d0,a1			; Get node...
		move.l	SSR_WAITER(a1),d0	; Get the wait task pointer
*
* Now, shared semaphores have bit 0 set in the task pointer...
*
		bclr	#0,d0			; Clear the bit (and test it)
		bne.s	rs_WakeShared		; If not 0, we are shared...
*
* Now, if the task pointer is NULL, we have a Procure() message waiter...
*
		tst.l	d0			; Check if task is NULL
		beq.s	rs_WakeProcure
*
* The simple case of a semaphore:  Wake the task and all that...
*
		move.l	d0,a1			; Set up the task...
		move.l	a1,SS_OWNER(a2)		; Mark him as the owner...
rs_Simple:	addq.w	#1,SS_NESTCOUNT(a2)	; Count the fact...
		moveq.l	#SIGF_SINGLE,d0		; Get the signal bit
		JSRSELF	Signal			; Wake him up...
*
* Pop stack and exit...
*
rs_PopPermitRts:
		movem.l	(sp)+,d0/d1/a0/a1/a2/a3	; Clean up
		JMP_PERMIT			; And exit (tail recursion)
*
*******************************************************************************
*
* Now, for the procure case:  We have a SemaphoreMessage in a1.  We
* need to set it up as needed and reply it.  We also need to reply
* all of the messages for the same task...
*
* In addition, any SignalSemaphore that happens to be the same task will
* be replied as a signal to prevent locking...
*
rs_WakeProcure:	move.l	SSM_SEMAPHORE(a1),SS_OWNER(a2)	; Get the real owner
		move.l	SS_WAITQUEUE(a2),a3		; Get the node list
*
* Wake the current one and check for more...
*
rs_WakeMore:	move.l	a2,SSM_SEMAPHORE(a1)	; Set up return
		addq.w	#1,SS_NESTCOUNT(a2)	; Count the semaphore
		JSRSELF	ReplyMsg		; Reply the message
*
* Loop to look for same owners...
*
rs_CheckSame:	move.l	SS_OWNER(a2),d1		; Cache the owner for search
*
* Now, check for more...
*
rs_LookMore:	move.l	(a3),d0			; Get next node
		beq.s	rs_PopPermitRts		; If not more, exit...
		move.l	a3,a1			; Get node into a1...
		move.l	d0,a3			; Get next node into a3...
		move.l	SSR_WAITER(a1),d0	; Get waiting task...
		beq.s	rs_FoundProcure		; Found a procure case?
*
* Check if this standard SingalSemaphore is for the same task as the
* replied Procure() is...
*
		cmp.l	d0,d1			; Is it our guy?
		bne.s	rs_LookMore		; If not, keep looking...
*
* Ok, so we have a SignalSemaphore (ObtainSemaphore()) that is called
* and it is the same task as the Produre semaphore we just replied.  We
* must wake this guy since otherwise he will be stuck forever...
*
		move.l	LN_PRED(a1),a0		; Get previous node
		move.l	a3,(a0)			; a3 is next node...
		move.l	a0,LN_PRED(a3)		; a0 is previous node
		move.l	d1,a1			; Get task pointer...
		bra.s	rs_Simple		; And wake it...
*
* Ok, so we have a Procure() semaphore...  Check if the same owner...
*
rs_FoundProcure:
		cmp.l	SSM_SEMAPHORE(a1),d1	; Check if the same task
		bne.s	rs_LookMore		; If not, skip...
*
* Ok, so we have another message with the same task
*
		move.l	LN_PRED(a1),a0		; Get previous node
		move.l	a3,(a0)			; a3 is next node...
		move.l	a0,LN_PRED(a3)		; a0 is previous node
		addq.w	#1,SS_NESTCOUNT(a2)	; Nest once for this guy...
		bra.s	rs_WakeMore		; Wake him and check for more
*
*******************************************************************************
*
* Ok, so the new owner will be "shared"  This means that we don't care about
* the task who owns it...
*
rs_WakeShared:	move.l	SS_WAITQUEUE(a2),a3	; Get first node waiters
*
* Now, in d0 is the data, in a1 is the node...
*
rs_WakeIt:	addq.w	#1,SS_NESTCOUNT(a2)	; Count the semaphore
		tst.l	d0			; Are we procure?
		beq.s	rs_WakeIt_Msg		; If so, we do message...
*
* Wake the simple task
*
		move.l	d0,a1			; Get the task
		moveq.l	#SIGF_SINGLE,d0		; The signal to use
		JSRSELF	Signal			; Signal the task...
		bra.s	rs_CheckMore		; Check for more...
*
* Reply the procure message...
*
rs_WakeIt_Msg:	move.l	a2,SSM_SEMAPHORE(a1)	; Set up the result
		move.l	d0,SSR_WAITER(a1)	; Clear the fake shared bit
		JSRSELF	ReplyMsg		; Reply it...
*
* Ok, now check for more shared waiters...
*
rs_CheckMore:	move.l	(a3),d0			; Check if we are at the end
		beq.s	rs_PopPermitRts		; If not more, exit...
*
* Check this one out...
*
		move.l	a3,a1			; Get the node into a1
		move.l	d0,a3			; Get the next node into a3...
		move.l	SSR_WAITER(a1),d0	; Get task that is waiting...
		bclr	#0,d0			; Test (and clear it)
		beq.s	rs_CheckMore		; If not shared, skip it...
*
* A shared waiter:  Remove from list and wake it...
*
		move.l	LN_PRED(a1),a0		; Get previous node
		move.l	a3,(a0)			; a3 is next node...
		move.l	a0,LN_PRED(a3)		; a0 is previous node
		bra.s	rs_WakeIt		; Go and wake it...


******* exec.library/AttemptSemaphore *****************************************
*
*   NAME
*	AttemptSemaphore -- try to obtain without blocking
*
*   SYNOPSIS
*	success = AttemptSemaphore(signalSemaphore)
*	D0			   A0
*
*	LONG AttemptSemaphore(struct SignalSemaphore *);
*
*   FUNCTION
*	This call is similar to ObtainSemaphore(), except that it will not
*	block if the semaphore could not be locked.
*
*   INPUT
*       signalSemaphore -- an initialized signal semaphore structure
*
*   RESULT
*	success -- TRUE if the semaphore was locked, false if some
*	    other task already possessed the semaphore.
*
*    NOTE
*	This call does NOT preserve registers.
*
*   SEE ALSO
*	ObtainSemaphore() ObtainSemaphoreShared(), ReleaseSemaphore(),
*	exec/semaphores.h
*
*******************************************************************************
*
AttemptSemaphore:
		MOVE.L	ThisTask(A6),D0		;Ok to trash D0

		FORBID
		ADDQ.W	#1,SS_QUEUECOUNT(A0)
		BEQ.S	as_instantsuccess

		CMP.L	SS_OWNER(A0),D0
		BEQ.S	as_success

		;------ someone else has it
		SUBQ.W	#1,SS_QUEUECOUNT(A0)
		CLEAR	D0
		JMP_PERMIT	;(exit)


as_instantsuccess:
		MOVE.L	D0,SS_OWNER(A0)
as_success:	ADDQ.W	#1,SS_NESTCOUNT(A0)
		MOVEQ	#1,D0
		JMP_PERMIT	;(exit)



******* exec.library/ObtainSemaphoreList **************************************
*
*   NAME
*	ObtainSemaphoreList -- get a list of semaphores.
*
*   SYNOPSIS
*	ObtainSemaphoreList(list)
*			    A0
*
*	void ObtainSemaphoreList(struct List *);
*
*   FUNCTION
*	Signal semaphores may be linked together into a list. This function
*	takes a list of these semaphores and attempts to lock all of them at
*	once. This call is preferable to applying ObtainSemaphore() to each
*	element in the list because it attempts to lock all the elements
*	simultaneously, and won't deadlock if someone is attempting to lock
*	in some other order.
*
*	This function assumes that only one task at a time will attempt to
*	lock the entire list of semaphores.  In other words, there needs to
*	be a higher level lock (perhaps another signal semaphore...) that is
*	used before someone attempts to lock the semaphore list via
*	ObtainSemaphoreList().
*
*	Note that deadlocks may result if this call is used AND someone
*	attempts to use ObtainSemaphore() to lock more than one semaphore on
*	the list.  If you wish to lock more than semaphore (but not all of
*	them) then you should obtain the higher level lock (see above)
*
*   INPUT
*       list -- a list of signal semaphores
*
*   SEE ALSO
*	ObtainSemaphoreShared(), InitSemaphore(), ReleaseSemaphore(),
*	AttemptSemaphore(), ObtainSemaphoreShared(), Procure(), Vacate()
*
*******************************************************************************
*
* REGISTER USAGE:
* -------- -----
* D2 -- next node
* A2 -- current semaphore
* D1 -- failed flag -- did any of the locks fail
* A1 -- current task pointer
*
*
*

ObtainSemaphoreList:
		MOVEM.L D2/A2/A3,-(SP)

		CLEAR	D1
		MOVE.L	ThisTask(A6),A2

		FORBID

		MOVE.L	A0,A3
		MOVE.L	LH_HEAD(A3),D2

osl_lockloop:
		NEXTNODE.S D2,A1,osl_failcheck

		ADDQ.W	#1,SS_QUEUECOUNT(A1)
		BEQ.S	osl_locked

		;------ it is currently locked
		CMP.L	SS_OWNER(A1),A2
		BEQ.S	osl_ourlock

		;------ locked by someone else
		MOVE.L	A2,SS_MULTIPLELINK+SSR_WAITER(A1)
		LEA	SS_WAITQUEUE(A1),A0
		LEA	SS_MULTIPLELINK(A1),A1
		BSR	AddTail
		MOVEQ	#1,D1

		BRA.S	osl_lockloop

osl_locked:
		MOVE.L	A2,SS_OWNER(A1)

osl_ourlock:
		ADDQ.W	#1,SS_NESTCOUNT(A1)
		BRA.S	osl_lockloop

osl_failcheck:
		;------ we have tried to lock everyone.  See if we need to wait
		TST.L	D1
		BEQ.S	osl_end

		;------ wait for all the laggards
		MOVE.L	LH_HEAD(A3),D2

osl_waitloop:
		NEXTNODE.S D2,A3,osl_end

osl_ownercheck:
		;------ see if we own the semaphore
		CMP.L	SS_OWNER(A3),A2
		BNE.S	osl_wait

		;------ we do own it. nestcnt == 0 if we just got it.
		TST.W	SS_NESTCOUNT(A3)
		BNE.S	osl_waitloop

		;------ we just got it.  set the nest count correctly
		ADDQ.W	#1,SS_NESTCOUNT(A3)
		BRA.S	osl_waitloop

osl_wait:
		;------ someone else has this one.  wait for it.
		MOVEQ	#SIGF_SINGLE,D0
		JSRSELF Wait

		BRA.S	osl_ownercheck

osl_end:
		BSR_PERMIT	;:TODO: Could be JMP_PERMIT

		MOVEM.L (SP)+,D2/A2/A3
		RTS

******* exec.library/ReleaseSemaphoreList *************************************
*
*   NAME
*	ReleaseSemaphoreList -- make a list of semaphores available
*
*   SYNOPSIS
*	ReleaseSemaphoreList(list)
*			     A0
*
*	void ReleaseSemaphoreList(struct List *);
*
*   FUNCTION
*	ReleaseSemaphoreList() is the inverse of ObtainSemaphoreList(). It
*	releases each element in the semaphore list.
*
*	Needless to say, havoc breaks out if the task releases more times
*	than it has obtained.
*
*   INPUT
*       list -- a list of signal semaphores
*
*   SEE ALSO
*	ObtainSemaphoreList()
*
*******************************************************************************
*
ReleaseSemaphoreList:

		MOVE.L	D2,-(SP)

		MOVE.L	LH_HEAD(A0),D2

rsl_loop:
		NEXTNODE.S D2,A0,rsl_end

		JSRSELF ReleaseSemaphore
		BRA.S	rsl_loop

rsl_end:
		MOVE.L	(SP)+,D2
		RTS


******* exec.library/AddSemaphore *********************************************
*
*   NAME
*	AddSemaphore -- initialize then add a signal semaphore to the system
*
*   SYNOPSIS
*	AddSemaphore(signalSemaphore)
*		     A1
*
*	void AddSemaphore(struct SignalSemaphore *);
*
*   FUNCTION
*	This function attaches a signal semaphore structure to the system's
*	public signal semaphore list.  The name and priority fields of the
*	semaphore structure must be initialized prior to calling this
*	function.  If you do not want to let others rendezvous with this
*	semaphore, use InitSemaphore() instead.
*
*	If a semaphore has been added to the naming list, you must be
*	careful to remove the semaphore from the list (via RemSemaphore)
*	before deallocating its memory.
*
*	Semaphores that are linked together in an allocation list (which
*	ObtainSemaphoreList() would use) may not be added to the system
*	naming list, because the facilities use the link field of the
*	signal semaphore in incompatible ways
*
*   INPUTS
*       signalSemaphore -- an signal semaphore structure
*
*   BUGS
*	Does not work in Exec <V36.  Instead use this code:
*
*	    #include <exec/execbase.h>
*	    #include <exec/nodes.h>
*	    extern struct ExecBase *SysBase;
*		...
*	    void LocalAddSemaphore(s)
*	    struct SignalSemaphore *s;
*	    {
*		s->ss_Link.ln_Type=NT_SIGNALSEM;
*		InitSemaphore(s);
*		Forbid();
*		Enqueue(&SysBase->SemaphoreList,s);
*		Permit();
*	    }
*
*   SEE ALSO
*	RemSemaphore, FindSemaphore, InitSemaphore
*
*******************************************************************************
*
AddSemaphore:
		MOVE.L	A1,A0
		MOVE.L	A1,-(SP)
		JSRSELF InitSemaphore
		MOVE.L	(SP)+,A1
		LEA.L	SemaphoreList(A6),A0
		BRASELF AddNode


******* exec.library/RemSemaphore *********************************************
*
*   NAME
*	RemSemaphore -- remove a signal semaphore from the system
*
*   SYNOPSIS
*	RemSemaphore(signalSemaphore)
*		     A1
*
*	void RemSemaphore(struct SignalSemaphore *);
*
*   FUNCTION
*	This function removes a signal semaphore structure from the
*	system's signal semaphore list.  Subsequent attempts to
*	rendezvous by name with this semaphore will fail.
*
*   INPUTS
*       signalSemaphore -- an initialized signal semaphore structure
*
*   SEE ALSO
*	AddSemaphore, FindSemaphore
*
*
*******************************************************************************
*
;RemSemaphore:	equ	RemNode		; Just do RemNode...


******* exec.library/FindSemaphore ********************************************
*
*   NAME
*	FindSemaphore -- find a given system signal semaphore
*
*   SYNOPSIS
*	signalSemaphore = FindSemaphore(name)
*	D0		                A1
*
*	struct SignalSemaphore *FindSemaphore(STRPTR);
*
*   FUNCTION
*	This function will search the system signal semaphore list for a
*	semaphore with the given name.	The first semaphore matching this
*	name will be returned.
*
*	This function does not arbitrate for access to the semaphore list,
*	surround the call with a Forbid()/Permit() pair.
*
*   INPUT
*	name - name of the semaphore to find
*
*   RESULT
*	semaphore - a pointer to the signal semaphore, or zero if not
*		    found.
*
*******************************************************************************
*
FindSemaphore:
	LEA     SemaphoreList(A6),A0
	bra	FindNode	; (a0,a1)

******* exec.library/AttemptSemaphoreShared ********************************
*
*   NAME
*	AttemptSemaphoreShared -- try to obtain without blocking       (V37)
*
*   SYNOPSIS
*	success = AttemptSemaphoreShared(signalSemaphore)
*	D0			         A0
*
*	LONG AttemptSemaphoreShared(struct SignalSemaphore *);
*
*   FUNCTION
*	This call is similar to ObtainSemaphoreShared(), except that it
*	will not block if the semaphore could not be locked.
*
*   INPUT
*       signalSemaphore -- an initialized signal semaphore structure
*
*   RESULT
*	success -- TRUE if the semaphore was granted, false if some
*	    other task already possessed the semaphore in exclusive mode.
*
*   NOTE
*	This call does NOT preserve registers.
*
*	Starting in V39 this call will grant the semaphore if the
*	caller is already the owner of an exclusive lock on the semaphore.
*	In pre-V39 systems this would not be the case.  If you need this
*	feature you can do the following workaround:
*
*	LONG myAttemptSempahoreShared(struct SignalSemaphore *ss)
*	{
*	LONG result;
*
*		\* Try for a shared semaphore *\
*		if (!(result=AttemptSemaphoreShared(ss)))
*		{
*			\* Now try for the exclusive one... *\
*			result=AttempSemaphore(ss);
*		}
*		return(result);
*	}
*
*   SEE ALSO
*	ObtainSemaphore() ObtainSemaphoreShared(), ReleaseSemaphore(),
*	exec/semaphores.h
*
*****************************************************************************
*
AttemptSemaphoreShared:
		FORBID
		addq.w	#1,SS_QUEUECOUNT(a0)
		bne.s	ass_InUse
		;------ grant semaphore w/ clear SS_OWNER indicating "shared"
ass_BumpNest:	addq.w	#1,SS_NESTCOUNT(a0)
		moveq	#1,D0			;got it!
		JMP_PERMIT			;exit


ass_InUse:	;------ the semaphore is in use.  See if it is sharable
		move.l	SS_OWNER(a0),d0
		beq.s	ass_BumpNest
		;------ the semaphore is exclusive use.  Check if self...
		cmp.l	ThisTask(a6),d0
		beq.s	ass_BumpNest

		;------ some exclusive locker has it
		subq.w	#1,SS_QUEUECOUNT(A0)
		moveq	#0,D0			;failure
		JMP_PERMIT			;failed!



******* exec.library/ObtainSemaphoreShared ************************************
*
*    NAME
*	ObtainSemaphoreShared -- gain shared access to a semaphore (V36)
*
*    SYNOPSIS
*	ObtainSemaphoreShared(signalSemaphore)
*	                      a0
*
*	void ObtainSemaphoreShared(struct SignalSemaphore *);
*
*    FUNCTION
*	A lock on a signal semaphore may either be exclusive, or shared.
*	Exclusive locks are granted by the ObtainSemaphore() and
*	AttemptSemaphore() functions.  Shared locks are granted by
*	ObtainSemaphoreShared().  Calls may be nested.
*
*	Any number of tasks may simultaneously hold a shared lock on a
*	semaphore.  Only one task may hold an exclusive lock.  A typical
*	application is a list that is often read, but only occasionally
*	written to.
*
*	Any exlusive locker will be held off until all shared lockers
*	release the semaphore.  Likewise, if an exlusive lock is held,
*	all potential shared lockers will block until the exclusive lock
*	is released.  All shared lockers are restarted at the same time.
*
*    EXAMPLE
*		ObtainSemaphoreShared(ss);
*		/* read data */
*		ReleaseSemaohore(ss);
*
*		ObtainSemaphore(ss);
*		/* modify data */
*		ReleaseSemaohore(ss);
*
*    NOTES
*	While this function was added for V36, the feature magically works
*	with all older semaphore structures.
*
*	A task owning a shared lock must not attempt to get an exclusive
*	lock on the same semaphore.
*
*	Starting in V39, if the caller already has an exclusive lock on the
*	semaphore it will return with another nesting of the lock.  Pre-V39
*	this would cause a deadlock.  For pre-V39 use, you can use the
*	following workaround:
*
*		\* Try to get the shared semaphore *\
*		if (!AttemptSemaphoreShared(ss))
*		{
*			\* Check if we can get the exclusive version *\
*			if (!AttemptSemaphore(ss))
*			{
*				\* Oh well, wait for the shared lock *\
*				ObtainSemaphoreShared(ss));
*			}
*		}
*		:
*		:
*		ReleaseSemaphore(ss);
*
*    INPUT
*	signalSemaphore -- an initialized signal semaphore structure
*
*    NOTE
*	This call is guaranteed to preserve all registers, starting with
*	V37 exec.
*
*    RESULT
*
*    SEE ALSO
*	ObtainSemaphore(), InitSemaphore(), ReleaseSemaphore(),
*	AttemptSemaphore(), ObtainSemaphoreList(), Procure(), Vacate()
*
*******************************************************************************
*
ObtainSemaphoreShared:
		FORBID
		addq.w	#1,SS_QUEUECOUNT(a0)
		bne.s	oss_InUse
		;------ grant semaphore w/ clear SS_OWNER indicating "shared"
oss_BumpNest:	addq.w	#1,SS_NESTCOUNT(a0)
		JMP_PERMIT			;exit


oss_InUse:	;------ the semaphore is in use.  See if it is sharable
		tst.l	SS_OWNER(a0)
		beq.s	oss_BumpNest

		movem.l	a0/a1/d0/d1,-(sp)
		;------ the semaphore is exclusive use.  Check if self...
		move.l	ThisTask(a6),d1		; Get my task...
		cmp.l	SS_OWNER(a0),d1
		beq.s	oss_Mine		; It is mine!

		;------ someone else has it exclusive.  wait for it
		lea.l	-SSR_SIZE(sp),sp	; local storage
		move.l	d1,a1			; Get TCB into a1...
		bclr.b	#SIGB_SINGLE,TC_SIGRECVD+3(a1)	;clear out old signals
		addq.l	#1,a1			;...quickly set low bit of TCB
		move.l	a1,SSR_WAITER(sp)	;low bit means shared waiter...

		;------ link us onto waiter's list
		lea.l	SS_WAITQUEUE(a0),a0
		move.l	sp,a1
		BSRSELF	AddTail

		;------ wait for the semaphore
		moveq	#SIGF_SINGLE,d0
		JSRSELF	Wait
		lea.l	SSR_SIZE(sp),sp
		bra.s	oss_Exit

oss_Mine:	; 'tis my semaphore!
		addq.w	#1,SS_NESTCOUNT(a0)

oss_Exit:	movem.l	(sp)+,a0/a1/d0/d1
		JMP_PERMIT			;exit


		END
