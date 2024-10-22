*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **		     Task Support		 **
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
*	$Id: tasks.asm,v 39.0 91/10/15 08:29:03 mks Exp $
*
*	$Log:	tasks.asm,v $
* Revision 39.0  91/10/15  08:29:03  mks
* V39 Exec initial checkin
* 
**********************************************************************


;****** Exported Functions *******************************************

    XDEF	AddTask
    XDEF	ExitTask
    XDEF	RemTask
    XDEF	FindTask
    XDEF	SetTaskPri
    XDEF	SetSignal
    XDEF	SetExcept
    XDEF	Wait
    XDEF	Signal
    XDEF	Reschedule
    XDEF	Permit
    XDEF	Forbid
    XDEF	AllocTrap
    XDEF	FreeTrap
    XDEF	AllocSignal
    XDEF	FreeSignal

	XDEF	ChildFree
	XDEF	ChildOrphan
	XDEF	ChildStatus
	XDEF	ChildWait

;****** Included Files ***********************************************

    NOLIST
    INCLUDE	"assembly.i"

    INCLUDE	"types.i"
    INCLUDE	"nodes.i"
    INCLUDE	"memory.i"
    INCLUDE	"lists.i"
    INCLUDE	"tasks.i"
    INCLUDE	"libraries.i"
    INCLUDE	"interrupts.i"
    INCLUDE	"execbase.i"
    INCLUDE	"ables.i"
    INCLUDE	"constants.i"
    TASK_ABLES
    INCLUDE	"hardware/intbits.i"

    INCLUDE	"calls.i"
    LIST


;****** Imported Globals *********************************************

    INT_ABLES

    EXTERN_DATA _intreq

    EXTERN_CODE Remove
    EXTERN_CODE AddTail
    EXTERN_CODE Enqueue
    EXTERN_CODE Exqueue

    EXTERN_SYS	Supervisor
    EXTERN_SYS	Schedule
    EXTERN_SYS	Reschedule
    EXTERN_SYS	FindName
    EXTERN_SYS	FreeEntry
    EXTERN_SYS	Dispatch
    EXTERN_SYS	Switch
    EXTERN_SYS	AvailMem
    EXTERN_SYS	AllocVec
    EXTERN_SYS	FreeVec



*****o* exec.library/AddTask ***********************************************
*
*   NAME
*	AddTask -- add a task to the system
*
*   SYNOPSIS
*	AddTask(task, initialPC, finalPC)
*		A1    A2	 A3
*   FUNCTION
*	Add a task to the system.
*
*	Certain fields of the task control block must be
*	initialized and a stack should be allocated prior
*	to calling this function.  The absolute smallest stack
*	that is allowable is 80 bytes, but in general the
*	stack size is dependent on what subsystems are called.
*	In general 200 bytes is sufficient if only exec is called,
*	and 4K will do if anything in the system is called.
*
*	This function will temporarily use space from the new
*	task's stack for the task's initial set of registers.  This
*	space is allocated starting at the SPREG location specified
*	in the task control block (not from SPUPPER).  This means
*	that a task's stack may contain static data put there prior
*	to its execution.  This is useful for providing initialized
*	global variables or some tasks may want to use this space
*	for passing the task its initial arguments.
*
*	A task's initial registers are set to zero (except the PC).
*
*   INPUTS
*	task - pointer to the task control block
*	initialPC - the initial entry point's address
*	finalPC - the finalization code entry point's address.  If
*		zero, the system will use a general finalizer.
*		This pointer is placed on the stack as if it
*		were the outermost return address.
*
*   SEE ALSO
*	RemTask, FindTask
*
**********************************************************************

AddTask:
	    IFGE    INFODEPTH-490
		MOVE.L  A1,-(SP)
		moveq   #MEMF_CHIP,d1
		JSRSELF AvailMem
		NPRINTF 151,<'C=%ld '>,d0
		moveq   #MEMF_FAST,d1
		JSRSELF AvailMem
		NPRINTF 151,<'F=%ld '>,d0
		MOVE.L  (SP)+,A1
		NPRINTF 490,<': AddTask $%lx - %s'>,a1,LN_NAME(a1)
	    ENDC

	    MOVE.B  #TS_ADDED,TC_STATE(A1)
	    moveq   #TF_ETASK,d1
	    and.b   d1,TC_FLAGS(a1)	;new mode?
	    beq.s   at_OldWay

		moveq	#ETask_SIZEOF,d0
		move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
		move.l	a1,-(sp)		;save task pointer
		JSRSELF	AllocVec
		move.l	(sp)+,a1		;restore task pointer
		move.l	d0,tc_ETask(a1)
		beq	at_MemoryError
		move.l	d0,a0
		move.l	ThisTask(a6),et_Parent(a0)
		FORBID
			addq.l	#1,ex_TaskID(A6) ;Make a "unique" task ID.
			bvc.s	at_noflow		    ;Lame attempt
			move.l	#TASK_ID_WRAP,ex_TaskID(a6) ;to keep id unique
at_noflow:		move.l	ex_TaskID(a6),et_UniqueID(a0)
		PERMIT

at_OldWay:  CLEAR   D1
	    MOVE.W  #-1,TC_IDNESTCNT(A1) * and TC_TDNESTCNT
	    MOVE.L  TaskSigAlloc(A6),TC_SIGALLOC(A1)
	    MOVE.L  D1,TC_SIGWAIT(A1)
	    MOVE.L  D1,TC_SIGRECVD(A1)
	    MOVE.L  D1,TC_SIGEXCEPT(A1)
	    BSR     putTrapAble		;[special registers]
	    MOVE.W  TaskTrapAlloc(A6),D1
	    BSR     putTrapAlloc	;[special registers]

*	    ------- setup trap and exception handlers:
	    TST.L   TC_TRAPCODE(A1)
	    BNE.S   at_suppliedTrap
	    MOVE.L  TaskTrapCode(A6),TC_TRAPCODE(A1)
at_suppliedTrap:
	    TST.L   TC_EXCEPTCODE(A1)
	    BNE.S   at_suppliedExcept
	    MOVE.L  TaskExceptCode(A6),TC_EXCEPTCODE(A1)
at_suppliedExcept:

*	    ------- setup register space on task's stack:
	    MOVE.L  TC_SPREG(A1),A0
	    MOVE.L  A3,-(A0)            * finalization pointer
	    BNE.S   at_default		* if user supplied
	    MOVE.L  TaskExitCode(A6),(A0) * default finalization
at_default:
	    MOVEQ   #15-1,D1		* clear initial registers
at_clear:   CLR.L   -(A0)
	    DBF     D1,at_clear
	    CLR     -(A0)               * status register
	    MOVE.L  A2,-(A0)            * program counter

	    ;------ do special stack for FPU systems
	IFNE	AFB_PRIVATE-15
	FAIL	'AFP_PRIVATE must be 15 for code to work!!!'
	ENDC
	    tst.w   AttnFlags(a6)	; Check the bit (use negative flag)
	    bpl.s   at_readylist

	    MOVEQ   #0,D0
	    MOVE.L  D0,-(A0)		* Note! Restores NULL frame to new task.
at_readylist:

	    MOVE.L  A0,TC_SPREG(A1)     * stack pointer

*	    ------- add task to ready list:
	    LEA     TaskReady(A6),A0
	    DISABLE
	    MOVE.B  #TS_READY,TC_STATE(A1)
	    BSR     Enqueue (A0,A1)
	    MOVE.L  TaskReady(A6),D0    * D0 is first node
	    ENABLE
	    MOVE.L  A1,-(SP)
	    CMP.L   A1,D0		* are we first
	    BNE.S   at_exit		* if no need to reschedule
	    JSRSELF Reschedule
at_exit:    MOVE.L  (SP)+,D0
            RTS


at_MemoryError:	CLEAR	d0
		rts


*****o* exec.library/RemTask ***********************************************
*
*   NAME
*	RemTask -- remove a task from the system
*
*   SYNOPSIS
*	RemTask(task)
*		A1
*
*   FUNCTION
*	This function removes a task from the system.  Deallocation
*	of resources should have been performed prior to calling
*	this function.
*
*   INPUTS
*	task - pointer to the task node representing the task
*		to be removed.	A zero value indicates self
*		removal, and will cause the next ready task
*		to begin execution.
*
*   SEE ALSO
*	AddTask
*
**********************************************************************


;------default task exit code----------------------------------------------
ExitTask:
	    MOVE.L  ABSEXECBASE,A6
	    SUBA.L  A1,A1
*	    FALL    THROUGH


RemTask:
	    MOVEM.L D2/D3/D4,-(SP)

	    MOVE.L  A1,D3		* save for later
	    BNE.S   rt_search		* if self removal
	    MOVE.L  ThisTask(A6),D3
	    BRA.S   rt_memory

rt_search:
*	    ------- is it current task:
	    CMP.L   ThisTask(A6),A1
	    BEQ.S   rt_memory

*	    ------- remove from whatever queue:
	    DISABLE
	    BSR     Remove
	    ENABLE

rt_memory:
	    MOVE.L  D3,A1
*	    ------- mark the task as removed
	    MOVE.B  #TS_REMOVED,TC_STATE(A1)


		;:TODO: Add don't free bit
		btst.b	#TB_ETASK,TC_FLAGS(a1)	;new mode?
		beq.s	rt_OldWay

		move.l	a1,-(sp)		;save task pointer
		move.l	tc_ETask(a1),a1
		JSRSELF	FreeVec			;(ok to pass zero here)
		move.l	(sp)+,a1		;restore task pointer
rt_OldWay:

*
*	    ------- go critical if task is freeing its own memory:
*		[Primarily this gives us a stack to continue working on]
*
	    CMP.L   ThisTask(A6),A1
	    BNE.S   rt_notSelf
	    FORBID
rt_notSelf:

*	    ------- free up any memory:
*		This code was changed not to reference the TC_MEMENTRY list
*		again after calling FreeEntry.
*
*		It works by caching the relevant part of the list header.
*		Even if the TCB is trashed during the process, we won't
*		re-reference the memory.  This works regardless of where the
*		TCB is in the TC_MEMENTRY list.
*
	    ;
	    ; This is the last time we touch/need the TCB
	    ;
	    LEA     TC_MEMENTRY(A1),A0
	    MOVE.L  (A0),D2
	    BEQ.S   rt_switch
	    CMP.L   LH_TAILPRED(A0),A0	;IFEMPTY
	    BEQ.S   rt_switch
	    CLR.L   (A0)		; destroy list for no apparent reason

	    ;
	    ; List is not empty.  Loop 'til at tail node.
	    ; Don't re-read freed node, or the (possibly freed) list header
	    ;
	    lea.l   LH_TAIL(a0),a0	; optimize to addq
	    move.l  a0,d4		; address of "empty" node at end

rt_free:    move.l  d2,a0
	    move.l  (a0),d2		; D2 = next node
	    NPRINTF 700,<'RemTask FreeEntry at $%lx'>,a0
	    JSRSELF FreeEntry
	    cmp.l   d2,d4		; Was that the last node??
	    bne.s   rt_free
rt_switch:


	    CMP.L   ThisTask(A6),D3
	    BNE.S   rt_success
	    JMPTRAP rt_dispatch(PC)

rt_dispatch:
	IFNE	AFB_PRIVATE-15
	FAIL	'AFP_PRIVATE must be 15 for code to work!!!'
	ENDC
	    tst.w   AttnFlags(a6)	; Check the bit (use negative flag)
	    bpl.s   2$

	    ADDQ.L  #2,SP
2$:	    ADDQ.L  #6,SP		; strip super stack RTE frame
	    JMPSELF Dispatch		; goodbye!

rt_success: CLEAR   D0
rt_exit:    MOVEM.L (SP)+,D2/D3/D4
	    RTS


*****o* exec.library/FindTask **********************************************
*
*   NAME
*	FindTask -- find a task with the given name or find oneself
*
*   SYNOPSIS
*	task = FindTask(name)
*	D0		A1
*
*   FUNCTION
*	This function will check all task queues for a task with the
*	given name, and return a pointer to its task control block.
*	If a null name pointer is given a pointer to the current
*	task will be returned.
*
*   INPUT
*	name - pointer to a name string
*
*   RESULT
*	task - pointer to the task
*
**********************************************************************

FindTask:
	    MOVE.L  A1,D0		* TST.L A1
	    BNE.S   ft_search		* if not self
	    MOVE.L  ThisTask(A6),D0
	    RTS

ft_search:
*	    ------- check the ready queue:
	    LEA     TaskReady(A6),A0
	    MOVE.L  A1,-(SP)		* Store name pointer at (SP)
	     DISABLE
	     JSRSELF FindName		* (A0,A1)
	     TST.L   D0
	     BNE.S   ft_enable		* if found

*	     ------- check the waiting queue:
	     LEA     TaskWait(A6),A0
	     MOVE.L  (SP),A1		* Grab name pointer from (SP)
	     JSRSELF FindName		* (A0,A1)
	     TST.L   D0
	     BNE.S   ft_enable

*	     ------- check self:
	     MOVE.L  ThisTask(A6),A0
	     MOVE.L  LN_NAME(A0),A0
	     MOVE.L  (SP),A1		* Grab name pointer from (SP)
ft_next:     CMP.B   (A0)+,(A1)+
	     BNE.S   ft_enable
	     TST.B   -1(A0)
	     BNE.S   ft_next
	     MOVE.L  ThisTask(A6),D0

ft_enable:   ENABLE
	    ADDQ.L  #4,SP		* (SP) no longer needed
	    RTS				* exit



*****o* exec.library/SetTaskPri ********************************************
*
*   NAME
*	SetTaskPri -- get and set the priority of a task
*
*   SYNOPSIS
*	oldPriority = SetTaskPri(task, priority)
*	D0-0:8			 A1    D0-0:8
*
*   FUNCTION
*	This function changes the priority of a task regardless of
*	its state.  The old priority of the task is returned.  A
*	reschedule is performed, and a context switch may result.
*
*   INPUTS
*	task - task to be affected
*	priority - the new priority for the task
*
*   RESULT
*	oldPriority - the tasks previous priority
*
*
**********************************************************************

SetTaskPri:
	    DISABLE
	    MOVE.B  LN_PRI(A1),-(SP)
	    MOVE.B  D0,LN_PRI(A1)

*	    ------- see if it is the current task:
	    CMP.L   ThisTask(A6),A1
	    BEQ.S   tp_resched

*	    ------- is task ready?
	    CMP.B   #TS_READY,TC_STATE(A1)
	    BNE.S   tp_exit		* if waiting

*	    ------- re-queue the ready task:
	    MOVE.L  A1,D0
	    BSR     Exqueue
	    LEA     TaskReady(A6),A0
	    MOVE.L  D0,A1
	    BSR     Enqueue

*	    ------- is a resched necessary?
	    CMP.L   TaskReady(A6),A1    * LH_HEAD
	    BNE.S   tp_exit		* if no need to reschedule

tp_resched:
	    JSRSELF Reschedule

tp_exit:
	    ENABLE
	    CLEAR   D0
	    MOVE.B  (SP)+,D0
	    RTS


*****o* exec.library/SetExcept *********************************************
*
*   NAME
*	SetExcept -- define certain signals to cause exceptions
*
*   SYNOPSIS
*	oldSignals = SetExcept(newSignals, signalMask)
*	D0		       D0	   D1
*
*   FUNCTION
*	This function defines which of the task's signals will
*	cause an exception.  When any of the signals occurs the
*	task's exception handler will be dispatched.  If the signal
*	occurred prior to calling SetExcept, the exception will
*	happen immediately.
*
*   INPUTS
*	newSignals - the new values for the signals specified in
*		signalMask.
*	signalMask - the set of signals to be effected
*
*   RESULTS
*	oldSignals - the prior exception signals
*
*   EXAMPLE
*	Get the current state of all exception signals:
*	    SetExcept(0,0)
*	Change a few exception signals:
*	    SetExcept($1374,$1074)
*
*   SEE ALSO
*	Signal, SetSignal
*
**********************************************************************

SetExcept:
	    MOVE.L  ThisTask(A6),A1
	    LEA     TC_SIGEXCEPT(A1),A0
	    BRA.S   postSignal


*****o* exec.library/SetSignal *********************************************
*
*   NAME
*	SetSignal -- define the state of this task's signals
*
*   SYNOPSIS
*	oldSignals = SetSignal(newSignals, signalMask)
*	D0		       D0	   D1
*
*   FUNCTION
*	This function defines the states of the task's signals.
*
*	This function is considered dangerous.
*
*   INPUTS
*	newSignals - the new values for the signals specified in
*		signalSet.
*	signalMask - the set of signals to be effected
*
*   RESULTS
*	oldSignals - the prior values for all signals
*
*   EXAMPLE
*	Get the current state of all signals:
*	    SetSignal(0,0)
*	Clear all signals:
*	    SetSignal(0,FFFFFFFFH)
*
*   SEE ALSO
*	Signal, Wait
*
**********************************************************************

SetSignal:
	    MOVE.L  ThisTask(A6),A1
	    LEA     TC_SIGRECVD(A1),A0

postSignal:
	    AND.L   D1,D0		* clear irrelevant bits
	    NOT.L   D1			* outside of DISABLE
	    DISABLE
	    MOVE.L  (A0),-(SP)          * get old signals
	    AND.L   (A0),D1             * clear relevant signals
	    OR.L    D0,D1		* new signal values
	    MOVE.L  D1,(A0)             * update TCB
;	    MOVE.L  D1,D0		* old code
	    MOVE.L  TC_SIGRECVD(A1),D0  * new code for SIGEXCEPT, dale
	    BRA.S   PostSignal


*****o* exec.library/Signal ************************************************
*
*   NAME
*	Signal -- signal a task
*
*   SYNOPSIS
*	Signal(task, signals)
*	       A1    D0
*
*   FUNCTION
*	This function signals a task with the given signals.  If
*	the task is currently waiting for one or more of these
*	signals, it will be made ready and a reschedule will occur.
*	If the task is not waiting for any of these signals, the
*	signals will be posted to the task for possible later use.
*	A signal may be sent to a task regardless of whether it's
*	running, ready, or waiting.
*
*	This function is considered "low level".  Its main purpose
*	is to support multiple higher level functions like PutMsg.
*	Generally a user need not perform Signals directly.
*
*   INPUT
*	task - the task to be signalled
*	signals - the signals to be sent
*
*   SEE ALSO
*	Wait, SetSignal
*
**********************************************************************

Signal:
	    LEA     TC_SIGRECVD(A1),A0
	    DISABLE
	    MOVE.L  (A0),-(SP)          * get old signals
	    OR.L    D0,(A0)             * update TCB

*	    FALL    THROUGH


* ***** Internal/Exec/PostSignal **********************************************
*
*   NAME
*	PostSignal -- define the state of any task's signals
*
*   SYNOPSIS
*	oldSignals = PostSignal(newSignals, sigField, task)
*	D0			D0	    A0	       A1
*
*   FUNCTION
*	This function defines the states of a task's signals.
*	It is an internal only routine, and does not appear in
*	the library.
*
*   INPUTS
*	newSignals - the new values for the signals specified in
*		signalSet.
*	sigField - task field of interest (RECVD or EXCEPT)
*	task - a pointer to a task control block.
*
*   RESULTS
*	oldSignals - the prior values for all signals
*
*
*   SEE ALSO
*	Signal,PostSignal
*
*******************************************************************************

PostSignal:

*------- see if we need to change the state of the handler
*	    ------- check for pending exception signals:
	    MOVE.L  TC_SIGEXCEPT(A1),D1 * check if exceptional
	    AND.L   D0,D1
	    BNE.S   ev_except		* if exceptional

*	    ------- is wait satisfied?
ev_checkWait:
	    CMP.B   #TS_WAIT,TC_STATE(A1)
	    BNE.S   ev_exit		* no longer waiting
	    AND.L   TC_SIGWAIT(A1),D0
	    BEQ.S   ev_exit		* no waking signals

*	    ------- wake it up:
ev_wakeup:
	    MOVE.L  A1,D0
	    REMOVE
	    MOVE.L  D0,A1
	    MOVE.B  #TS_READY,TC_STATE(A1)
	    LEA     TaskReady(A6),A0
	    BSR     Enqueue		* Note: Lengthy operation under DISABLE
*	    BSET.B  #(SB_SAR-8),SysFlags(A6)
*	    ISSUPER.S ev_exit
	    CMP.L   TaskReady(A6),A1    * LH_HEAD
	    BNE.S   ev_exit		* then no need to reschedule

*	    ------- re-evaluate scheduling:
ev_resched:
	    ENABLE
	    MOVE.L  (SP)+,D0
	    JMPSELF Reschedule
*	    ------ NEVER returns here!

ev_except:
	    BSET    #TB_EXCEPT,TC_FLAGS(A1)
	    CMP.B   #TS_WAIT,TC_STATE(A1)
	    BEQ.S   ev_wakeup
	    BRA.S   ev_resched

ev_exit:
	    ENABLE

*	    ------- return the old set of signals
	    MOVE.L  (SP)+,D0
	    RTS


*****o* exec.library/Wait **************************************************
*
*   NAME
*	Wait -- wait for one or more signals
*
*   SYNOPSIS
*	signals = Wait(signalSet)
*	D0	       D0
*
*   FUNCTION
*	This function will cause the current task to suspend
*	waiting for one or more signals.  When any of the specified
*	signals occurs, the task will return to the ready state.
*	If a signal occurred prior to calling Wait, the wait
*	condition will be immediately satisfied, and the task will
*	continue to run.
*
*	This function cannot be called while in supervisor mode!
*
*   INPUT
*	signalSet - the set of signals for which to wait.
*		Each bit represents a particular signal.
*
*   RESULTS
*	signals - signals which caused this task to awake.  Other
*		signals may have been posted, but only signals enabled by
*		the signalSet will be returned.
*
*   SEE ALSO
*	Signal, SetSignal
*
**********************************************************************
*
*	Note major modification to keep down interrupt latency.  Interrupts
*	are not critical after the ADDTAIL *except* that you can't
*	afford a task switch.  The Switch() function would have enabled
*	interrupts in a few hundred cycles -- not soon enough.
*
*	So I FORBID, drain the interrupt nest count and ENABLE.  This neatly
*	solves the problem. 		-Bryce
*


Wait:       MOVE.L  ThisTask(A6),A1
	    MOVE.L  D0,TC_SIGWAIT(A1)

	    MOVE.W  #$4000,_intena	* DISABLE (no nest count)
	    AND.L   TC_SIGRECVD(A1),D0
	    BNE.S   wt_exit		* (exception may have caused wakeup)

	    ADDQ.B  #1,TDNestCnt(A6)	* FORBID
wt_sleep:   MOVE.B  #TS_WAIT,TC_STATE(A1)
	    LEA.L   TaskWait(A6),A0
	    ADDTAIL			* A1-preserved

	    MOVE.B  IDNestCnt(A6),D1
	    MOVE.B  #-1,IDNestCnt(A6)
	    MOVE.W  #$C000,_intena	* ENABLE (nest count set to -1)
	    MOVE.L  A5,A0
	    JSRTRAP _LVOSwitch(A6)      * go to sleep
	    MOVE.L  A0,A5
	    MOVE.W  #$4000,_intena	* DISABLE (then restore nest count)
	    MOVE.B  D1,IDNestCnt(A6)

	    MOVE.L  TC_SIGWAIT(A1),D0
	    AND.L   TC_SIGRECVD(A1),D0
	    BEQ.S   wt_sleep		* (exception may have caused wakeup)
	    SUBQ.B  #1,TDNestCnt(A6)	* PERMIT (trivial case-while disabled)

wt_exit:    EOR.L   D0,TC_SIGRECVD(A1)  * reset signals
	    TST.B   IDNestCnt(A6)
	    BGE.S   wt_disable
	    MOVE.W  #$C000,_intena	* ENABLE (nest count set to -1)
wt_disable: RTS


*****i* Internal/Exec/Rescedule ***********************************************
*
*   Reschedule -- re-evalutate task scheduling
*
*----------------------------------------------------------------
* Notes:
*
*	1) will always set SAR to indicate that scheduling needs
*	   some attention.
*
*	2) if interrupts are disabled, we must defer scheduling until
*	   later.  This is easily accomplished by poking the software
*	   interrupt bit.  When interrupts are re-enabled, at least
*	   this single interrupt will occur, causing the system to
*	   schedule (during interrupt exiting).
*
*	   If SB_SAR is already set, then don't bother to post the
*	   interrupt.
*
*	3) if tasking is disabled, then we will ignore the request
*	   to reschedule.  It is the duty of Permit to carry out
*	   the request when it again enables tasking.
*
*	4) if we are in supervisor mode, we will do the same thing
*	   as when tasking is disabled.
*******************************************************************************

Reschedule:
	    BSET.B  #(SB_SAR-8),SysFlags(A6)
	    SNE     D0

	    TST.B   TDNestCnt(A6)       * tasking disabled?
	    BGE.S   resched_exit

	    TST.B   IDNestCnt(A6)       * interrupts disabled?
	    BLT.S   pm_UserSched

	    ;------ don't post if SB_SAR already was pending
	    TST.B   D0
	    BNE.S   resched_exit
	    MOVE.W  #(INTF_SETCLR!INTF_SOFTINT),_intreq

resched_exit:
	    RTS

*****o* exec.library/Forbid ****************************************************
*
*    NAME
*	Forbid -- forbid task rescheduling.
*
*    SYNOPSIS
*	Forbid();
*
*    FUNCTION
*	Prevents other tasks from being scheduled to run by the
*	dispatcher, until a matching Permit() is executed, or
*	this task is scheduled to Wait.
*
*    INPUTS
*
*    RESULTS
*	The current task will not be rescheduled as long as it is
*	ready to run.  In the event that the current task enters a wait
*	state, other tasks will be recheduled if they are ready to run.
*	Upon return from it's wait state, the original task will continue
*	to run without breaking the Forbid(). Calls to Forbid() nest.
*	In order to restore normal task rescheduling, the programmer must
*	execute exactly one call to Permit() for every call to Forbid().
*
*    BUGS
*
*    SEE ALSO
*	Permit, Disable, Enable, exec/tasks.i
*
********************************************************************************

Forbid:
	    FORBID
	    RTS

*****o* exec.library/Permit ****************************************************
*
*    NAME
*	Permit -- permit task rescheduling.
*
*    SYNOPSIS
*	Permit();
*
*    FUNCTION
*	Allow other tasks to be scheduled to run by the
*	dispatcher, after a matching Forbid() has been executed.
*
*    INPUTS
*
*    RESULTS
*	Other tasks will be rescheduled as they are ready to run.
*	In order to restore normal task rescheduling, the programmer must
*	execute exactly one call to Permit() for every call to Forbid().
*
*    BUGS
*
*    SEE ALSO
*	Forbid, Disable, Enable, exec/tasks.i
*
********************************************************************************
*
*	Note: WShell may be clearning TDNestCnt() then calling us.

	CNOP	0,4	;Nice alignment for 68020's
Permit:
	    SUBQ.B  #1,TDNestCnt(A6)
	    BGE.S   pm_exit
	    TST.B   IDNestCnt(A6)
	    BGE.S   pm_exit
	    TST.W   SysFlags(A6)        * Speedup, faster than BTST.B
	    BMI.S   pm_UserSched
pm_exit:    RTS

**	    BTST.B  #(SB_SAR-8),SysFlags(A6)    ;Replaced per
**	    BEQ.S   pm_exit			;E4289

pm_UserSched:
	    MOVE.L  A5,-(SP)
	    JSRTRAP userSched(PC)
	    MOVE.L  (SP)+,A5
	    RTS

userSched:  BTST.B  #(13-8),(SP)        * check SR for super
	    BNE.S   us_Sup
	    JMPSELF Schedule		* does not return to here
us_Sup:     RTE

*****o* exec.library/AllocTrap ********************************************
*
*   NAME
*	AllocTrap -- allocate a processor trap vector
*
*   SYNOPSIS
*	trapNum = AllocTrap(trapNum)
*	D0		    D0
*
*   FUNCTION
*	Allocate a trap number from the current task's pool.  These
*	trap numbers are those associated with the 68000 TRAP type
*	instructions.  Either a particular nnumber, or the next
*	free number may be allocated.
*
*	If the trap is already in use (or no free traps are
*	available) a -1 is returned.
*
*	This function can only be used by the currently running
*	task.
*
*   WARNING
*	Signals may not be allocated or freed from exception
*	handling code.
*
*   INPUTS
*	trapNum - the desired trap number {of 0..15} or -1
*		for no preference.
*
*   RESULTS
*	trapNum - the trap number allocated {of 0..15}.  If no traps are
*		available, this function returns -1.
*
*   SEE ALSO
*	FreeTrap
*
**********************************************************************

;
; These three functions are the kludge that allows extended task
; structures.  If an ETask is present, we get the values from the ETask.
; Else we look at the old location.
;
;	A0 - destroyed
;	A1 - input TCB pointer
;	D1 - input/output value
;
;--Put to old TC_TRAPABLE--
putTrapAble:	btst.b	#TB_ETASK,TC_FLAGS(a1)	;new mode?
		beq.s	ptb_oldway
		move.l	tc_ETask(a1),a0
		move.w	d1,et_TRAPABLE(a0)
		rts				;(exit)
ptb_oldway	move.w	d1,tc_ETask+2(A1)	;magic old offset
		rts				;(exit)


;--Put to old TC_TRAPALLOC--
putTrapAlloc:	btst.b	#TB_ETASK,TC_FLAGS(a1)	;new mode?
		beq.s	pta_oldway
		move.l	tc_ETask(a1),a0
		move.w	d1,et_TRAPALLOC(a0)
		rts				;(exit)
pta_oldway	move.w	d1,tc_ETask+0(A1)	;magic old offset
		rts				;(exit)


;--Get from old TC_TRAPALLOC--
getTrapAlloc:	btst.b	#TB_ETASK,TC_FLAGS(a1)	;new mode?
		beq.s	gta_oldway
		move.l	tc_ETask(a1),a0
		move.w	et_TRAPALLOC(a0),d1
		rts				;(exit)
gta_oldway	move.w	tc_ETask+0(A1),d1	;magic old offset
		rts				;(exit)



AllocTrap:  MOVE.L  ThisTask(A6),A1
	    BSR     getTrapAlloc	;[D0/D1/A0]
	    CMP.B   #-1,D0
	    BEQ.S   al_alloc

	    BSET    D0,D1
	    BEQ.S   al_exit
	    BRA.S   al_fail
al_alloc:
	    MOVEQ   #15,D0		* scan from highest trap
al_next:
	    BSET    D0,D1		* test and set alloc bit
	    BEQ.S   al_exit		* bra if free
	    DBF     D0,al_next
al_fail:
	    MOVEQ   #-1,D0
al_exit:    BRA     putTrapAlloc	;[D0/D1/A0] (exit)


*****o* exec.library/FreeTrap **********************************************
*
*   NAME
*	FreeTrap -- free a processor trap
*
*   SYNOPSIS
*	FreeTrap(trapNum)
*		 D0
*
*   FUNCTION
*	This function frees a previously allocated trap number for
*	reuse.	This call must be performed while running in the
*	same task in which the trap was allocated.
*
*   WARNING
*	Traps may not be allocated or freed from exception
*	handling code.
*
*   INPUTS
*	trapNum - the trap number to free {of 0..15}
*
**********************************************************************

FreeTrap:   MOVE.L  ThisTask(A6),A1
	    BSR     getTrapAlloc	;[D0/D1/A0]
	    BCLR    D0,D1
	    BRA     putTrapAlloc	;[D0/D1/A0] (exit)


*****o* exec.library/AllocSignal ******************************************
*
*   NAME
*	AllocSignal -- allocate a signal bit
*
*   SYNOPSIS
*	signalNum = AllocSignal(signalNum)
*	D0			D0
*
*   FUNCTION
*	Allocate a signal bit from the current tasks pool.  Either
*	a particular bit, or the next free bit may be allocated.
*	The signal associated with the newly allocated bit will be
*	properly initialized (cleared).
*
*	If the signal is already in use (or no free signals are
*	available) a -1 is returned.
*
*	This function can only be used by the currently running
*	task.
*
*   WARNING
*	Signals may not be allocated or freed from exception
*	handling code.
*
*   INPUTS
*	signalNum - the desired signal number {of 0..31} or -1
*		for no preference.
*
*   RESULTS
*	signalNum - the signal bit number allocated {0..31}.
*		If no signals are available, this function returns -1.
*
*   SEE ALSO
*	FreeSignal
*
**********************************************************************

AllocSignal:
	    MOVE.L  ThisTask(A6),A1
	    MOVE.L  TC_SIGALLOC(A1),D1
	    CMP.B   #-1,D0
	    BEQ.S   as_alloc
	    BSET    D0,D1
	    BEQ.S   as_pass
	    BRA.S   as_fail
as_alloc:
	    MOVEQ   #31,D0
as_next:
	    BSET    D0,D1
	    BEQ.S   as_pass
	    DBF     D0,as_next
as_fail:
	    MOVEQ   #-1,D0
	    BRA.S   as_exit
as_pass:
	    MOVE.L  D1,TC_SIGALLOC(A1)
	    MOVEQ.L #-1,D1
	    BCLR    D0,D1
	    ;------ DISABLE not required -- only alloc signal effected.
	    AND.L   D1,TC_SIGRECVD(A1)
	    AND.L   D1,TC_SIGEXCEPT(A1)
	    AND.L   D1,TC_SIGWAIT(A1)
as_exit:
	    RTS


*****o* exec.library/FreeSignal **********************************************
*
*   NAME
*	FreeSignal -- free a signal bit
*
*   SYNOPSIS
*	FreeSignal(signalNum)
*		   D0:8
*
*   FUNCTION
*	This function frees a previously allocated signal bit for
*	reuse.	This call must be performed while running in the
*	same task in which the signal was allocated.
*
*   WARNING
*	Signals may not be allocated or freed from exception
*	handling code.
*
*   INPUTS
*	signalNum - the signal number to free {0..31}
*
**********************************************************************
*
*   CAUTION
*	This function does not protect itself against illegal
*	use in interrupts or exceptions.  Using this function
*	in such pieces of code will compromise the task's
*	integrity.
*
*   NOTE
*	DeleteMsgPort() and Intuition depend on the new -1 check added
*	for Exec 36.131.
*
**********************************************************************

FreeSignal:
	    IFGE    INFODEPTH-490
	    MOVEQ   #32,D1
	    CMP.L   D1,D0
	    BCS.S   fs_Ok
	    MOVE.L  (SP),D1
	    NPRINTF 490,<'Bad FreeSignal D0=%lx PC=%lx Task=%lx'>,D0,D1,$114(A6)
fs_Ok:
	    ENDC

	    CMP.B   #-1,D0
	    BEQ.S   fs_Insane
	    MOVE.L  ThisTask(A6),A1
	    MOVE.L  TC_SIGALLOC(A1),D1
	    BCLR.L  D0,D1
	    MOVE.L  D1,TC_SIGALLOC(A1)
fs_Insane:  RTS



******************************************************************************
*
*	This was a bogus rough cut placeholder for Ray's child tasking
*	scheme.  Ray and I were to work out the final details "later".
*	It won't work.
*
;FindChild -internal
;
;	D0 - result
;	D1 - SEARCHTID
;
FindChild:	move.l	ThisTask(a6),a1
		move.l	tc_ETask(a1),a1
		lea.l	et_Children(a1),a1	;base of child list
		move.l	LN_SUCC(a1),d0		;get first node
fc_NextNode:	move.l	d0,a1
		move.l	LN_SUCC(a1),d0
		beq.s	fc_End
		cmp.l	et_UniqueID(a1),d1
		bne.s	fc_NextNode
		move.l	a1,d0
		rts	;exit

fc_End:		moveq	#0,d0
		rts




;Search all children for a matching ID
;		d0 - ID
;
ChildStatus:	move.l	d0,d1
		bsr	FindChild
		beq.s	cs_notfound
		moveq	#CHILD_ACTIVE,d0
		rts

cs_notfound:	moveq	#CHILD_NOTFOUND,d0
		rts





;Wait for a child to terminate, or wait for any child to terminate
ChildWait:	move.l	d0,d1

		;Convenience function...
		rts





;Remove and clean up after a child.  This is automatically done for
;any child that has no parent.
;		d0 - ID
;:TODO: Remove all children?
;
ChildFree:	move.l	a2,-(sp)
		move.l	d0,d1
		bsr.s	FindChild
		beq.s	cr_ignore	;ignore the lack of anything to remove
		move.l	d0,a2		;ETask pointer

		move.l	et_Result2(a2),a1
		move.l	a1,d0
		beq.s	cr_noarg
		JSRSELF	FreeVec		;free extended argument
cr_noarg:

		move.l	a2,a1
		JSRSELF	FreeVec		;free ETask structure itself

cr_ignore:	move.l	(sp)+,a2
		rts




;Orphan a child, or all children of the current task.
;		d0 - ID
;
;	A1-Task
;	D0-TID
ChildOrphan:	move.l	d0,d1		;test TID
		bne.s	co_OrphanOne

		;Empty the nest!  Ok kids, you're on your own...
		move.l	ThisTask(a6),a1
		move.l	tc_ETask(a1),a1
		lea.l	et_Children(a1),a1	;base of child list
		move.l	LN_SUCC(a1),d0		;get first node
co_NextNode:	move.l	d0,a1
		move.l	LN_SUCC(a1),d0
		beq.s	co_End
		clr.l	et_Parent(a1)
		bra.s	co_NextNode

co_End:		move.l	ThisTask(a6),a1
		move.l	tc_ETask(a1),a1
		lea.l	et_Children(a1),a1	;base of child list
		NEWLIST	a1
		rts	;exit



		;Sorry, Son, but you've gotta go seek your fortune.
co_OrphanOne:	bsr	FindChild
		beq.s	co_notfound	;already gone? ignore it...
		move.l	d0,a0
		REMOVE	a0
co_notfound:	rts


    END
