*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **		 Interrupt Processing		 **
*	   **						 **
*	   ************************************************

*************************************************************************
*									*
*   Copyright 1984,85,88,89,91 Commodore-Amiga, Inc.			*
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
*	$Id: interrupts.asm,v 39.12 93/07/14 16:19:44 jesup Exp $
*
*	$Log:	interrupts.asm,v $
* Revision 39.12  93/07/14  16:19:44  jesup
* made quick interrupts conditional, since they seem to have problems
* with the 4091.
* 
* 
* Revision 39.11  92/12/16  17:27:29  mks
* Updated the autodoc for ObtainQuickVector()
* 
* Revision 39.10  92/11/13  09:15:01  mks
* Made unallocated vectors point at an alert
*
* Revision 39.9  92/10/13  11:53:52  mks
* Autodoc fix...
*
* Revision 39.8  92/10/13  10:31:10  mks
* Finished the documentation for Quick Interrupts
*
* Revision 39.7  92/10/09  12:49:34  mks
* Now have the ObtainQuickVector() code done...
*
* Revision 39.6  92/10/06  17:01:08  mks
* Some cleanup getting ready for ObtainQuickVector
*
* Revision 39.5  92/08/14  13:36:36  mks
* Added a comment about interrupt servers and the clearing of the
* interrupt request
*
* Revision 39.4  92/06/16  18:00:54  mks
* Added the test of the TC_EXCEPTCODE for NULL as the documentation
* states (but which was not the case ;^)  Simple, 2-line fix...
*
* Revision 39.3  92/06/08  15:47:05  mks
* Removed the conditional INCLUDE_WACK since it no longer exists
*
* Revision 39.2  92/04/06  07:19:07  mks
* Now does not have an level-7 server if SAD is included (Wack, for old names)
*
* Revision 39.1  92/02/19  15:45:07  mks
* Changed ALERT macro to MYALERT
*
* Revision 39.0  91/10/15  08:27:30  mks
* V39 Exec initial checkin
*
**********************************************************************


;****** Included Files ***********************************************

    NOLIST
    INCLUDE	"assembly.i"

    INCLUDE	"types.i"
    INCLUDE	"nodes.i"
    INCLUDE	"lists.i"
    INCLUDE	"memory.i"
    INCLUDE	"libraries.i"
    INCLUDE	"interrupts.i"
    INCLUDE	"privinfo.i"
    INCLUDE	"privintr.i"
    INCLUDE	"tasks.i"
    INCLUDE	"alerts.i"
    INCLUDE	"execbase.i"
    INCLUDE	"ables.i"
    INCLUDE	"constants.i"

    INCLUDE	"hardware/custom.i"
    INCLUDE	"hardware/intbits.i"
    INCLUDE	"calls.i"
    LIST


;****** Imported *****************************************************

    INT_ABLES

    EXTERN_DATA _custom
    EXTERN_DATA _intena
    EXTERN_DATA _intreq
    EXTERN_DATA _intenar

;****** Imported Functions *******************************************

    EXTERN_CODE Enqueue
    EXTERN_CODE Exqueue
    EXTERN_CODE AddTail
    EXTERN_CODE RemHead
    EXTERN_CODE AllocMem

    EXTERN_SYS	ExitIntr
    EXTERN_SYS	Exception
    EXTERN_SYS	Supervisor
    EXTERN_SYS	AddIntServer
    EXTERN_SYS	Alert
    EXTERN_SYS	Switch


;****** Exported Functions *******************************************

    XDEF	GetCC
    XDEF	InitServers
    XDEF	ExitIntr
    XDEF	Schedule
    XDEF	Switch
    XDEF	Dispatch

    XDEF	Switch020
    XDEF	Launch
    XDEF	Launch020

    XDEF	IntServer
    XDEF	SetSR
    XDEF	SuperState
    XDEF	UserState
    XDEF	SetIntVector
    XDEF	AddIntServer
    XDEF	RemIntServer

    XDEF	SysIntr1
    XDEF	SysIntr2
    XDEF	SysIntr3
    XDEF	SysIntr4
    XDEF	SysIntr5
    XDEF	SysIntr6

    XDEF	Cause
    XDEF	SoftInt
    XDEF	Disable
    XDEF	Enable
    XDEF	Exception

	XDEF	StackSwap

*------ Interrupt Processing -----------------------------------------
*
*   DESCRIPTION
*	This code handles the autovectored prioritized interrupts.
*	All interrupts are handled by a generalized system routine
*	which calls servers attached to a given interrupt.
*
*   SPECIAL NOTES:
*    1. Interrupt servers should never call the task function Wait
*	or any system function which may in turn call Wait.
*	Most system functions protect against this.
*
*    2. Interrupt lists should not be used if empty.
*
*    3. Interrupt servers should use the DISABLE/ENABLE macros
*	when accessing shared global data.
*
*---------------------------------------------------------------------
*
*   NOTE:
*	    This code was written for speed, not for compactness.
*
*	    See SetIntVector for more info on handlers.
*


REGOFFSET   EQU     6*4

REGSAVE     MACRO
	    MOVEM.L D0/D1/A0/A1/A5/A6,-(SP)
	    ENDM

REGREST     MACRO
	    MOVEM.L (SP)+,D0/D1/A0/A1/A5/A6
	    ENDM

REGRESTNOA6 MACRO
	    MOVEM.L (SP)+,D0/D1/A0/A1/A5
	    ENDM

CHKSR	    MACRO
	    BTST.B  #5,REGOFFSET(SP)
	    ENDM

*
*	Note: the time taken to process a spurious interrupt
*	should be minimized.  These happen when a pending interrupt
*	and a DISABLE occur at the same time.
*
DOINT	    MACRO   * &LABEL
\1:
	    REGSAVE
	    LEA     _custom,A0
	    MOVE    intenar(A0),D1      * mask with Enabled ints
	    BTST    #INTB_INTEN,D1	* are we critical?
	    BEQ.S   \1Spurious		* We were disabled(!)  Panic! ...
	    AND     intreqr(A0),D1      * mask with service requests
	    MOVE.L  ABSEXECBASE,A6
	    ENDM

OTHER	    MACRO
	    REGREST
	    RTE
	    ENDM

SPURIOUS    MACRO	;exit because of critical section violation/spurious
	    MOVEM.L (SP)+,D0/D1/A0	;Restore only the modified registers
	    ADDA.W  #3*4,SP		;Compensate for 3 unmodified registers
	    RTE				;Get out ASAP
	    ENDM

IFINT	    MACRO   * &BITNAME, &EXIT, &NEXT
	    BTST    #INTB_\1,D1
	    BEQ.S   \3
	    MOVEM.L IV\1(A6),A1/A5
	    MOVE.L  A6,-(SP)		;So we don't read chip memory again
	    PEA     \2			;head 0, tail 2
	    ;!! Possible tradeoff: push _LVOExitIntr+2(a6)
	    JMP     (A5)                ;head 4, tail 0
	    ENDM

MOREINT     MACRO   * &BITS, &REPEAT, &EXIT
	    MOVE.L  (SP)+,A6		* IFINT put this on the stack
	    LEA     _custom,A0
;	    MOVE.L  ABSEXECBASE,A6
	    MOVE    #\1,D1
	    AND     intenar(A0),D1      * mask with Enabled ints
	    AND     intreqr(A0),D1      * mask with service requests
	    BNE.S   \2
	    MOVE.L  A6,-(SP)		;So we don't read chip memory again
	    JMP     \3
	    ENDM

*------ Level 1 Interrupts -------------------------------------------

	    DOINT   SysIntr1
si1:	    IFINT   TBE,_LVOExitIntr(A6),not_tbe       * recvr buffer
not_tbe:    IFINT   DSKBLK,_LVOExitIntr(A6),not_dskblk * disc block
not_dskblk: IFINT   SOFTINT,_LVOExitIntr(A6),not_soft  * software int

*------ Level 2 Interrupts -------------------------------------------

	    DOINT   SysIntr2
si2:	    IFINT   PORTS,_LVOExitIntr(A6),not_ports

*------ Common Code --------------------------------------------------
not_soft:
not_ports:
not_coper:
	    OTHER

SysIntr1Spurious:
SysIntr2Spurious:
SysIntr3Spurious:
SysIntr4Spurious:
	    SPURIOUS

*------ Level 3 Interrupts -------------------------------------------

	    DOINT   SysIntr3
si3:	    IFINT   BLIT,_LVOExitIntr(A6),not_blit     * blitter dma
not_blit:   IFINT   VERTB,_LVOExitIntr(A6),not_vertb   * vertical blank
not_vertb:  IFINT   COPER,_LVOExitIntr(A6),not_coper   * coprocessor

*------ Level 4 Interrupt --------------------------------------------

	    DOINT   SysIntr4
si4:	    IFINT   AUD1,exitInt4,not_aud1    * audio dmas
not_aud1:   IFINT   AUD3,exitInt4,not_aud3
not_aud3:   IFINT   AUD0,exitInt4,not_aud0
not_aud0:   IFINT   AUD2,exitInt4,not_aud2
not_aud2:   OTHER
exitInt4:   MOREINT (INTF_AUD0+INTF_AUD1+INTF_AUD2+INTF_AUD3),si4,_LVOExitIntr(A6)

*------ Common Code --------------------------------------------------

SysIntr6Spurious:
SysIntr5Spurious:
	    SPURIOUS

*------ Level 5 Interrupt --------------------------------------------

	    DOINT   SysIntr5
si5:	    IFINT   DSKSYNC,_LVOExitIntr(A6),not_dskbyt  * disc byte
not_dskbyt: IFINT   RBF,_LVOExitIntr(A6),not_rbf         * serial read
not_exter:
not_rbf:    OTHER

*------ Level 6 Interrupt --------------------------------------------

	    DOINT   SysIntr6
si6:	    IFINT   INTEN,_LVOExitIntr(A6),not_inena   * copper
not_inena:  IFINT   EXTER,_LVOExitIntr(A6),not_exter   * external high


*****i* Internal/Exec/ExitIntr ************************************************
*
*  Standard Exit Interrupt -- check if task scheduling needed
*
*******************************************************************************

ExitIntr:
	    MOVE.L  (SP)+,A6		* IFINT put this on the stack
	    CHKSR			* check stack frame for previous mode
	    BNE.S   sysExit		* if not user mode
	    TST.B   TDNestCnt(A6)       * tasking disabled?
	    BGE.S   sysExit		* if so

	    ;------ If we got this far, then we are exiting back
	    ;	    to the current task.  This happens to be a
	    ;	    convenient time to check for the existence of
	    ;	    other tasks that might want a piece of the
	    ;	    action.
	    TST.W   SysFlags(A6)        ;Faster SB_SAR test -bryce
	    BMI.S   doIntExitReschedule

sysExit:

EXPERIMENT_0	EQU 0
	    IFEQ    EXPERIMENT_0
	      REGREST
	      RTE
	    ENDC
	    IFNE    EXPERIMENT_0
	      moveq   #~CACRF_ClearI,d0		;Do not clear I
	      and.l   ex_CacheControl(a6),d0
	      beq.s   1$
	      dc.w    $4E7B,$0002	;MOVEC D0,CACR
1$
	      REGREST
	      RTE
	    ENDC


doIntExitReschedule:
	    ;------ There must be another task ready to run.
	    ;	    Since we are still at a non-zero interrupt
	    ;	    priority level, lets give lower priority
	    ;	    interrupts a chance before scheduling the
	    ;	    next process.  This is really only helps
	    ;	    interrupt response time a little, but it
	    ;	    doesn't cost much.
	    MOVE    #$2000,SR
	    BRA.S   schedule1

*----------------------------------------------------------------
*
*  Schedule -- supervisor mode task scheduler
*
*----------------------------------------------------------------

Schedule:
	    REGSAVE

schedule1:
	    MOVE.L  ThisTask(A6),A1	* Out of disable ok

	    ;------ Because this code is not re-entrant
	    MOVE    #$2700,SR		* disable

	    BCLR.B  #(SB_SAR-8),SysFlags(A6)

	    ;------ If the current task is undergoing some sort
	    ;	    of exception, we absolutely must reschedule
	    ;	    it so that it can be launched appropriately.
	    ;	    (There is a bug here if the task runs in this
	    ;	    mode when not disabled.  We re-enter this code
	    ;	    for the same task twice!!!).
	    BTST    #TB_EXCEPT,TC_FLAGS(A1)
	    BNE.S   sched_queue

	    ;------ If there are no ready tasks, then why hang
	    ;	    around?
	    LEA     TaskReady(A6),A0
	    IFEMPTY A0,sysExit

	    ;------ If the next ready task has a higher priority
	    ;	    than the current task, we must retire the
	    ;	    current task for the time being.
	    SUCC    A0,A0
	    MOVE.B  LN_PRI(A0),D1
	    CMP.B   LN_PRI(A1),D1
	    ;Major danger change here.  This code would suspend the current
	    ;task too often.  If another task of the same priority was ready,
	    ;the current task was suspended immediatly (instead of checking
	    ;TQE).
	    BGT.S   sched_queue		;Was BGE.S   sched_queue

	    ;------ If our time quantum has not expired, then we
	    ;	    are free to continue running.
	    BTST.B  #(SB_TQE-8),SysFlags(A6)
	    BEQ.S   sysExit

sched_queue:
	    LEA     TaskReady(A6),A0    * suspend running task
	    BSR     Enqueue (A0,A1)
	    MOVE.B  #TS_READY,TC_STATE(A1)

	    MOVE    #$2000,SR		* enable all interrupts

	    REGRESTNOA6
	    MOVE.L  (SP),-(SP)
	    MOVE.L  _LVOSwitch+2(A6),4(SP)
	    MOVE.L  (SP)+,A6
	    RTS


*****i* Internal/Exec/Switch **************************************************
*
*	    Switch task contexts.
*
*	    This code is not re-entrant.  This condition is assured
*	    by simply running in supervisor mode.
*
*	    This code is mostly not critical, so it may be
*	    interrupted.
*
*	    Careful with changes!
*
*******************************************************************************

Switch:
	    MOVE.W  #$2000,SR		* allow interrupts :OPTIMIZE: Redundant

*	    ------- save the task's registers in its stack:
	    MOVE.L  A5,-(SP)            ; avail scratch register
	    MOVE.L  USP,A5		; save registers on user stack
*	    MOVEM.L A6-A0/D7-D0,-(A5)
	    MOVEM.L A6-A0,-(A5)
	    MOVEM.L D7-D0,-(A5)

*	    ------- let interrupts function normally:
	    MOVE.L  ABSEXECBASE,A6
	    MOVE.W  IDNestCnt(A6),D0
	    MOVE.W  #-1,IDNestCnt(A6)
	    MOVE.W  #(INTF_SETCLR+INTF_INTEN),_intena

*	    ------- save remaining registers:
	    MOVE.L  (SP)+,4*13(A5)      * save user A5 register
	    MOVE.W  (SP)+,-(A5)         * save user SR
	    MOVE.L  (SP)+,-(A5)         * save user PC


Switch_Common:
	    MOVE.L  ex_LaunchPoint(A6),A4

*	    ------- task control structure:
	    MOVE.L  ThisTask(A6),A3     * pointer to current TCB
	    MOVE.W  D0,TC_IDNESTCNT(A3) * and TDNestCnt
	    MOVE.L  A5,TC_SPREG(A3)     * save user SP

*	    ------- check for special options:
	    BTST.B  #TB_SWITCH,TC_FLAGS(A3) * proctime, etc
	    BEQ.S   sw_dispatch
	    MOVE.L  TC_SWITCH(A3),A5
	    JSR     (A5)
	    BRA.S   sw_dispatch



*****i* Internal/Exec/Dispatch ************************************************
*
*	Dispatch next task
*
*******************************************************************************

Dispatch:   MOVE.L  ex_LaunchPoint(A6),A4

	    ;------ no current task to save.  make sure ints are on
	    MOVE.W  #-1,IDNestCnt(A6)
	    MOVE.W  #(INTF_SETCLR+INTF_INTEN),_intena
sw_dispatch:

	    ;------ get highest priority ready task:
	    LEA     TaskReady(A6),A0
idle:
	    ;------ list structures unrolled for faster access
	    MOVE    #$2700,SR		* fast disable
	    MOVE.L  (A0),A3
	    MOVE.L  (A3),D0
	    BNE.S   idle0

	    ;------ no one in the ready queue.	go back to sleep
	    ADDQ.L  #1,IdleCount(A6)
	    BSET.B  #(SB_SAR-8),SysFlags(A6)
	    STOP    #$2000		* wait for any interrupt
	    BRA.S   idle

idle0:
	    ;------ finish removing the head of the queue (task in A3)
	    MOVE.L  D0,(A0)
	    MOVE.L  D0,A1
	    MOVE.L  A0,LN_PRED(A1)

	    ;------ system bookkeeping
	    MOVE.L  A3,ThisTask(A6)
	    MOVE.W  Quantum(A6),Elapsed(A6)
	    BCLR.B  #(SB_TQE-8),SysFlags(A6)
	    MOVE.B  #TS_RUN,TC_STATE(A3) * running state
	    MOVE.W  TC_IDNESTCNT(A3),IDNestCnt(A6) * and TDNestCnt

	    ;------ restore task to proper disable state:
	    TST.B   IDNestCnt(A6)
	    BMI.S   sw5
	    MOVE.W  #INTF_INTEN,_intena
sw5:
	    MOVE.W  #$2000,SR
	    ADDQ.L  #1,DispCount(A6)		;Atomic instruction, ok
						;outside of disable.

	    ;------ check for special options:
	    MOVE.B  TC_FLAGS(A3),D0
	    AND.B   #(TF_LAUNCH!TF_EXCEPT),D0
	    BEQ.S   sw_Restore
	    BSR.S   sw_launchOpts

sw_Restore:
	    MOVE.L  TC_SPREG(A3),A5     * grab user stack pointer
	    JMP     (A4)


*****i* Internal/Exec/Launch **************************************************
*
*	Launch next task (pop registers and RTE)
*
*******************************************************************************

Launch:
	    LEA     2+4*16(A5),A2       * user stack adjust
	    MOVE.L  A2,USP		* setup user stack pointer
	    MOVE.L  (A5)+,-(SP)         * restore PC
	    MOVE.W  (A5)+,-(SP)         * restore SR

	    MOVEM.L (A5),D0-D7/A0-A6    * restore user registers
	    RTE 			* return to user program


sw_launchOpts:
	    BTST.L  #TB_LAUNCH,D0
	    BEQ.S   sw3
*					* don't want to mess with the stack
	    MOVE.B  D0,D2		* move here for safe keeping
	    MOVE.L  TC_LAUNCH(A3),A5
	    JSR     (A5)
	    MOVE.B  D2,D0		* restore

sw3:	    BTST.L  #TB_EXCEPT,D0
	    BNE.S   Exception
no_except:  RTS


*******************************************************************************
	;------ Handle Task Exception:
	;	User task gets called:
	;	newExcptSet = <ExcptCode> (signals, exceptData) SysLib
	;	D0			   D0	    A1		A6
	;
	;	signals - The set of signals that caused this exception.  Thes
	;	    Signals have been disabled from the current set of
	;	    signals that can cause an exception.
	;
	;	exceptData - A copy of the field in the TCB labeled
	;	    TC_EXCEPTDATA.
	;
	;	newExcptSet - The set of signals in NewExceptSet will be re-
	;	    enabled for exception generation.  Usually this will be
	;	    the same as the Signals that caused the exception.
	;	    Note that NewExcptSet can only enable new exceptions*
	;	    it cannot disable.

Exception:
	    BCLR    #TB_EXCEPT,TC_FLAGS(A3)

	;
	; The RKM said that if the TC_EXCEPTCODE is NULL, that
	; it will not be run.  Well, EXEC never did look at the TC_EXCEPTCODE
	; nor did it check for NULL...  Now it does...  -- MKS
	move.l	TC_EXCEPTCODE(a3),d1		; Cache it for later...
	beq.s	no_except

	    DISABLE
	    MOVE.L  TC_SIGRECVD(A3),D0
	    AND.L   TC_SIGEXCEPT(A3),D0
	    EOR.L   D0,TC_SIGEXCEPT(A3)
	    EOR.L   D0,TC_SIGRECVD(A3)
	    ENABLE

	    MOVE.L  TC_SPREG(A3),A1     * user stack

	    MOVE.L  TC_FLAGS(A3),-(A1)  ; save state info

*	    ------- If from waiting state
	    TST.B   IDNestCnt(A6)
	    BNE.S   ex_noEnable
	    ENABLE
ex_noEnable:

*	    ------- push the return address onto the user stack:
	    MOVE.L  #ex_afterUser,-(A1)
	    MOVE.L  A1,USP

*	    ------- enter task's exception handler:
	    BTST.B  #AFB_68010,AttnFlags+1(A6) ???
	    BEQ.S   1$
	    MOVE.W  #$20,-(SP)          * 68010 format word
1$:	    move.l  d1,-(sp)		; Cached from above...
	    CLR.W   -(SP)
	    MOVE.L  TC_EXCEPTDATA(A3),A1
	    RTE 			* go to user code


ex_afterUser:
	    MOVE.L  ABSEXECBASE,A6
	    JMPTRAP ex_super(PC)        * return to super mode
ex_super:
	;------ if we are running with a 68010, this code executes
	;	with the format word on the top of the super stack.
	; dale change 9/29/86 ---- for 68010 fix, after Neil fixed it
	    MOVE.L  ex_LaunchPoint(A6),A4
	    BTST.B  #AFB_68010,AttnFlags+1(A6)
	    BEQ.S   2$
		ADDQ.L	#2,SP		* clean up format word also!
2$	    ADDQ.L  #6,SP		* clean super frame
	    MOVE.L  ThisTask(A6),A3

	    OR.L    D0,TC_SIGEXCEPT(A3) * put exceptions back

	    MOVE.L  USP,A1
	    MOVE.L  (A1)+,TC_FLAGS(A3)  ; TC_STATE and nest counts
	    MOVE.L  A1,TC_SPREG(A3)

	    MOVE.W  TC_IDNESTCNT(A3),IDNestCnt(A6) * and TDNestCnt

	;------- restore task to proper disable state:
	    TST.B   IDNestCnt(A6)
	    BMI.S   ex_noDisable
	    MOVE.W  #INTF_INTEN,_intena
ex_noDisable:
	    RTS



*****i* Switch020 *************************************************************
*
*	Switch for 68020 with floating point crap
*
*	Functionally similar to Switch, but special processing for
*	68020's and coprocessors.  Used only when a 68881 is detected.
*
*	Normally this would not be duplicated code, but this is time
*	critical code...
*
*******************************************************************************


Switch020:
	    MOVE.W  #$2000,SR		* allow interrupts :OPTIMIZE: redundant

*	    ------- save the task's registers in its stack:
	    MOVE.L  A5,-(SP)            * avail scratch register
	    MOVE.L  USP,A5		* save registers on user stack
	    MOVEM.L D0-D7/A0-A6,-(A5)

*	    ------- let interrupts function normally:
	    MOVE.L  ABSEXECBASE,A6
	    MOVE.W  IDNestCnt(A6),D0
	    MOVE.W  #-1,IDNestCnt(A6)
	    MOVE.W  #(INTF_SETCLR+INTF_INTEN),_intena

*	    ------- save remaining registers:
	    MOVE.L  (SP)+,4*13(A5)      * save user A5 register
	    MOVE.W  (SP)+,-(A5)         * save user SR
	    ;:TODO: Stack is misaligned for one instruction.  It used to
	    ;be misaligned basically forever.  Same on restore.
	    ;Note that the suspended task's stack is still misaligned.
	    MOVE.L  (SP)+,-(A5)         * save user PC
	    MOVE.W  (SP)+,D1		* clean up stack & prevent misalignment

	    ;------ do an FSAVE, and see if we need to do more...
	    DC.W    $F325		; FSAVE -(A5)
	    TST.B   (A5)
	    BEQ     Switch_Common	; NULL coprocessor stack frame...

	    ;------ save stack type, and look for coproc mid instruction frame
	    MOVEQ   #-1,D2
	    MOVE.W  D2,-(A5)		; Align user stack
	    AND.W   #$F000,D1
	    CMP.W   #$9000,D1
	    BNE.S   sw020_saveffp

	    MOVE.L  (SP)+,-(A5)
	    MOVE.L  (SP)+,-(A5)
	    MOVE.L  (SP)+,-(A5)
	    MOVE.W  D1,D2

sw020_saveffp:
	    ;------ save all user visible floating point regs
	    DC.L    $F225E0FF		; FMOVEM.X FP0-FP7,-(A5)
	    DC.L    $F225BC00		; FMOVEM.L FPCR/FPSR/FPIAR,-(A5)

	    MOVE.W  D2,-(A5)		; Flag "full context save"
	    BRA     Switch_Common




Launch020:  MOVEQ.L #$0020,D1		; Probable stack frame type

	    ;------ see if the 68881 was active
	    MOVE.B  (A5),D0
	    BEQ.S   l020_frestore	; Was not active...


	    ADDQ.L  #2,A5		; remove stack holder
	    DC.L    $F21D9C00		; FMOVEM.L (A5)+,FPCR/FPSR/FPIAR
	    DC.L    $F21DD0FF		; FMOVEM.X (A5)+,FP0-FP7

	    ;------ check for mid-instruction stack frame
	    CMP.B   #$90,D0		; Test BYTE of WORD pushed to stack
	    BNE.S   l020_notmid		;	(confusing, eh?)
	    ;------ restore the mid-instruction frame
	    MOVE.L  (A5)+,-(SP)         ; three longs of frame
	    MOVE.L  (A5)+,-(SP)
	    MOVE.L  (A5)+,-(SP)
	    MOVE.W  #$9020,D1		; Fake coprocessor mid-instruction frame
l020_notmid:
	    ADDQ.L  #2,A5		; Alignment

l020_frestore:
	    DC.W    $F35D		; FRESTORE (A5)+

	    LEA     2+4*16(A5),A2       * user stack adjust
	    MOVE.L  A2,USP		* setup user stack pointer
	    MOVE.W  D1,-(SP)		* restore a frame type
	    MOVE.L  (A5)+,-(SP)         * restore PC
	    MOVE.W  (A5)+,-(SP)         * restore SR

	    MOVEM.L (A5),D0-D7/A0-A6    * restore user registers
	    RTE 			* return to user program



******* exec.library/SetSR ****************************************************
*
*   NAME
*	SetSR -- get and/or set processor status register
*
*   SYNOPSIS
*	oldSR = SetSR(newSR, mask)
*	D0	      D0     D1
*
*	ULONG SetSR(ULONG, ULONG);
*
*   FUNCTION
*	This function provides a means of modifying the CPU status register
*	in a "safe" way (well, how safe can a function like this be
*	anyway?).  This function will only affect the status register bits
*	specified in the mask parameter.  The prior content of the entire
*	status register is returned.
*
*   INPUTS
*	newSR - new values for bits specified in the mask.
*	    All other bits are not effected.
*	mask - bits to be changed
*
*   RESULTS
*	oldSR - the entire status register before new bits
*
*   EXAMPLES
*	To get the current SR:
*	    currentSR = SetSR(0,0);
*	To change the processor interrupt level to 3:
*	    oldSR = SetSR($0300,$0700);
*	Set processor interrupts back to prior level:
*	    SetSR(oldSR,$0700);
*
*******************************************************************************
SetSR:
	    MOVE.L  A5,A0
	    JMPTRAP setSR(PC)

setSR:
	    MOVE.L  A0,A5
	    MOVE    (SP),A0             * hold oldSR
	    AND     D1,D0
	    NOT     D1
	    AND     D1,(SP)             * clear the bits we will reset
	    OR	    D0,(SP)             * set those bits
	    CLEAR   D0
	    MOVE    A0,D0		* put oldSR in its final place
	    RTE


******* exec.library/GetCC ****************************************************
*
*   NAME
*	GetCC -- get condition codes in a 68010 compatible way.
*
*   SYNOPSIS
*	conditions = GetCC()
*	  D0
*
*	UWORD GetCC(void);
*
*   FUNCTION
*	The 68000 processor has a "MOVE SR,<ea>" instruction which gets a
*	copy of the processor condition codes.
*
*	On the 68010,20 and 30 CPUs, "MOVE SR,<ea>" is privileged.  User
*	code will trap if it is attempted.  These processors need to use
*	the "MOVE CCR,<ea>" instruction instead.
*
*	This function provides a means of obtaining the CPU condition codes
*	in a manner that will make upgrades transparent.  This function is
*	VERY short and quick.
*
*   RESULTS
*	conditions - the 680XX condition codes
*
*    NOTE
*	This call is guaranteed to preserve all registers.  This function
*	may be implemented as code right in the jump table.
*
*******************************************************************************
**GetCC is implemented inside the library vector.  See startexec.asm
GetCC:
**	    MOVE    SR,D0
**	    AND.W   #$FF,D0
**	    RTS
*	    MOVE.L  A5,A0		; save A5 in temp register
*	    JSRTRAP getCC(PC)
*	    RTS
*
*getCC:
*	    MOVE.L  A0,A5		; restore A5
*	    MOVE.W  (SP),D0
*	    AND.W   #$FF,D0
*	    RTE


******* exec.library/SuperState ***********************************************
*
*   NAME
*	SuperState -- enter supervisor state with user stack
*
*   SYNOPSIS
*	oldSysStack = SuperState()
*	D0
*
*	APTR SuperState(void);
*
*   FUNCTION
*	Enter supervisor mode while running on the user's stack. The user
*	still has access to user stack variables.  Be careful though, the
*	user stack must be large enough to accommodate space for all
*	interrupt data -- this includes all possible nesting of interrupts.
*	This function does nothing when called from supervisor state.
*
*   RESULTS
*	oldSysStack - system stack pointer; save this.	It will come in
*		      handy when you return to user state.  If the system
*		      is already in supervisor mode, oldSysStack is zero.
*
*   SEE ALSO
*	UserState/Supervisor
*
*
*******************************************************************************
SuperState:
	    MOVE.L  A5,A0
	    JMPTRAP superState(PC)

superState:
	    MOVE.L  A0,A5
	    CLR.L   D0
	    BSET.B  #5,(SP)             * modify pre-trap status
	    BNE.S   ss_exit
	    MOVE    (SP)+,SR            * new status register
	    MOVE.L  SP,D0		* save system stack pointer
	    MOVE.L  USP,SP		* make user stk the sys stk
	    BTST.B  #AFB_68010,AttnFlags+1(A6)
	    BEQ.S   1$
	    ADDQ.L  #2,D0
1$:	    ADDQ.L  #4,D0		* remove old PC
	    RTS 			* in supervisor state
ss_exit:
	    RTE


******* exec.library/UserState ************************************************
*
*   NAME
*	UserState -- return to user state with user stack
*
*   SYNOPSIS
*	UserState(sysStack)
*		  D0
*
*	void UserState(APTR);
*
*   FUNCTION
*	Return to user state with user stack, from supervisor state with
*	user stack.  This function is normally used in conjunction with the
*	SuperState function above.
*
*	This function must not be called from the user state.
*
*   INPUT
*	sysStack - supervisor stack pointer
*
*   BUGS
*	This function is broken in V33/34 Kickstart.  Fixed in V1.31 setpatch.
*
*   SEE ALSO
*	SuperState/Supervisor
*
*******************************************************************************
UserState:
		;------ change our stacks, remembering the user's return PC
		MOVE.L	(SP)+,A0
		MOVE.L	SP,USP
		MOVE.L	D0,SP
		AND.W	#$DFFF,SR		* clear supervisor mode
		JMP	(A0)


	IFNE	0
	    ;------ first, change our stacks, remembering the user' return PC
	    MOVE.L  (SP)+,D1
	    MOVE.L  SP,USP
	    MOVE.L  D0,SP

	    ;------ Now build a stack frame
	    MOVE.L  A5,A0
	    JMPTRAP userState(PC)

userState:
	    MOVE.L  A0,A5

	    ;------ poke the return address of the stack frame back to user
	    MOVE.L  D1,2(SP)
	    AND     #$dfff,(SP)         * clear supervisor mode
	    RTE
	ENDC

******* exec.library/SetIntVector *********************************************
*
*   NAME
*	SetIntVector -- set a new handler for a system interrupt vector
*
*   SYNOPSIS
*	oldInterrupt = SetIntVector(intNumber, interrupt)
*	D0			    D0         A1
*
*	struct Interrupt *SetIntVector(ULONG, struct Interrupt *);
*
*   FUNCTION
*	This function provides a mechanism for setting the system interrupt
*	vectors.  These are non-sharable; setting a new interrupt handler
*	disconnects the old one.  Installed handlers are responsible for
*	processing, enabling and clearing the interrupt.  Note that interrupts
*	may have been left in any state by the previous code.
*
*	The IS_CODE and IS_DATA pointers of the Interrupt structure will
*	be copied into a private place by Exec.  A pointer to the previously
*	installed Interrupt structure is returned.
*
*	When the system calls the specified interrupt code, the registers are
*	setup as follows:
*
*	    D0 - scratch
*	    D1 - scratch (on entry: active
*			  interrupts -> equals INTENA & INTREQ)
*
*	    A0 - scratch (on entry: pointer to base of custom chips
*			  for fast indexing)
*	    A1 - scratch (on entry: Interrupt's IS_DATA pointer)
*
*	    A5 - jump vector register (scratch on call)
*	    A6 - Exec library base pointer (scratch on call)
*
*	    all other registers must be preserved
*
*   INPUTS
*	intNum - the Paula interrupt bit number (0..14).  Only non-chained
*		 interrupts should be set.  Use AddIntServer() for server
*		 chains.
*	interrupt - a pointer to an Interrupt structure containing the
*		 handler's entry point and data segment pointer.  A NULL
*		 interrupt pointer will remove the current interrupt and
*		 set illegal values for IS_CODE and IS_DATA.
*
*		 By convention, the LN_NAME of the interrupt structure must
*		 point a descriptive string so that other users may
*		 identify who currently has control of the interrupt.
*
*   RESULT
*	A pointer to the prior interrupt structure which had control
*	of this interrupt.
*
*   SEE ALSO
*	AddIntServer(),exec/interrupts.i,hardware/intbits.i
*
*******************************************************************************
SetIntVector:

	    MULU    #IV_SIZE,D0
	    LEA     IntVects(A6,D0.W),A0

	    DISABLE

	    ;------ fetch previous node pointer:
	    MOVE.L  IV_NODE(A0),D0

	    ;------ set the node
	    MOVE.L  A1,IV_NODE(A0)
	    BEQ.S   si_isEmpty

	    ;------ setup new vectors
	    MOVE.L  IS_DATA(A1),IV_DATA(A0)
	    MOVE.L  IS_CODE(A1),IV_CODE(A0)
	    BRA.S   si_end

si_isEmpty:
	    ;------ setup bogus values
	    MOVEQ.L #-1,D1
	    MOVE.L  D1,IV_DATA(A0)
	    MOVE.L  D1,IV_CODE(A0)

si_end:
	    ENABLE

	    RTS


******* exec.library/AddIntServer *********************************************
*
*   NAME
*	AddIntServer -- add an interrupt server to a system server chain
*
*   SYNOPSIS
*	AddIntServer(intNum, interrupt)
*		     D0-0:4  A1
*
*	void AddIntServer(ULONG, struct Interrupt *);
*
*   FUNCTION
*	This function adds a new interrupt server to a given server chain.
*	The node is located on the chain in a priority dependent position.
*	If this is the first server on a particular chain, interrupts will
*	be enabled for that chain.
*
*	Each link in the chain will be called in priority order until the
*	chain ends or one of the servers returns with the 68000's Z condition
*	code clear (indicating non-zero).  Servers on the chain should return
*	with the Z flag clear if the interrupt was specifically for that
*	server, and no one else.  VERTB servers should always return Z set.
*	(Take care with High Level Language servers, the language may not
*	have a mechanism for reliably setting the Z flag on exit).
*
*	Servers are called with the following register conventions:
*
*	    D0 - scratch
*	    D1 - scratch
*
*	    A0 - scratch
*	    A1 - server is_Data pointer (scratch)
*
*	    A5 - jump vector register (scratch)
*	    A6 - scratch
*
*	    all other registers must be preserved
*
*   INPUTS
*	intNum - the Paula interrupt bit number (0 through 14). Processor
*		 level seven interrupts (NMI) are encoded as intNum 15.
*		 The PORTS, COPER, VERTB, EXTER and NMI interrupts are
*		 set up as server chains.
*	interrupt - pointer to an Interrupt structure.
*		 By convention, the LN_NAME of the interrupt structure must
*		 point a descriptive string so that other users may
*		 identify who currently has control of the interrupt.
*
*   WARNING
*	Some compilers or assemblers may optimize code in unexpected ways,
*	affecting the conditions codes returned from the function.  Watch
*	out for a "MOVEM" instruction (which does not affect the condition
*	codes) turning into "MOVE" (which does).
*
*   BUGS
*	The graphics library's VBLANK server, and some user code, currently
*	assume that address register A0 will contain a pointer to the custom
*	chips. If you add a server at a priority of 10 or greater, you must
*	compensate for this by providing the expected value ($DFF000).
*
*   SEE ALSO
*	RemIntServer, SetIntVector, hardware/intbits.i,exec/interrupts.i
*
*******************************************************************************
AddIntServer:
	    MOVE.L  D2,-(SP)
	    MOVE.L  D0,D2
	    MOVE.L  D0,D1

	    MULU    #IV_SIZE,D0
	    LEA     IntVects(A6,D0.W),A0
	    MOVE.L  IV_DATA(A0),A0

	    DISABLE
	    BSR     Enqueue

	    MOVE.W  #INTF_SETCLR,D0
	    BSET    D2,D0
	    MOVE.W  D0,_intena		* enable it

	    ENABLE
	    MOVE.L  (SP)+,D2
	    RTS

******* exec.library/RemIntServer *********************************************
*
*   NAME
*	RemIntServer -- remove an interrupt server from a server chain
*
*   SYNOPSIS
*	RemIntServer(intNum, interrupt)
*		     D0      A1
*
*	void RemIntServer(ULONG,struct Interrupt *);
*
*   FUNCTION
*	This function removes an interrupt server node from the given
*	server chain.
*
*	If this server was the last one on this chain, interrupts for this
*	chain are disabled.
*
*   INPUTS
*	intNum - the Paula interrupt bit (0..14)
*	interrupt - pointer to an interrupt server node
*
*   BUGS
*	Before V36 Kickstart, the feature that disables the interrupt
*	would not function.  For most server chains this does not
*	cause a problem.
*
*   SEE ALSO
*	AddIntServer, hardware/intbits.h
*
*******************************************************************************
RemIntServer:
	    MOVE.L  D2,-(SP)
	    MOVE.L  D0,D2

	    MULU    #IV_SIZE,D0
	    LEA     IntVects(A6,D0.W),A0
	    MOVE.L  IV_DATA(A0),A0


	    MOVE.L  A0,D1
	    DISABLE
	    BSR     Exqueue
	    MOVE.L  D1,A0

	    cmp.l   LH_TAIL+LN_PRED(a0),a0
	    bne.s   ri_exit

*	    ------- this was the last server
	    CLEAR   D1
	    BSET    D2,D1
	    MOVE.W  D1,_intena

ri_exit:
	    ENABLE
	    MOVE.L  (SP)+,D2
	    RTS


	PAGE
************************************************************************
*
*	Initialize Standard Interrupt Server Handlers
*
************************************************************************

MAXSERVERS  EQU     5

InitServers:
	    MOVEM.L D2/D3/A2/A3,-(SP)

	    ;------ allocate memory for all servers
	    MOVE.L  #IL_SIZE*MAXSERVERS,D0
	    MOVE.L  #MEMF_PUBLIC!MEMF_CLEAR,D1
	    BSRSELF AllocMem		; Must use BSR !
	;   TST.L   D0
	;   BNE.S   is_memory

	;AZ_STOP 50
	;ALERT	AN_IntrMem ;:BUG:bryce-Alert does not work at this stage in powerup.
	;Just Crash!
is_memory:
	    MOVEQ.L #MAXSERVERS-1,D2
	    MOVE.L  D0,A2
	    LEA     is_parms(PC),A3
is_loop:
	    MOVE.L  A2,D1
	    NEWLIST A2
	    MOVE.W  (A3)+,D3                    ;int bit number
	    MOVE.W  (A3)+,IL_SIntReq(A2)        ;INTF_XXX
	    LEA     IL_SIZE(A2),A2

	    LEA     IntServer(PC),A1    ; generic interrupt server
	    MULU    #IV_SIZE,D3 	; calc offset
	    MOVEM.L D1/A1,IntVects+IV_DATA(A6,D3.W)

	    DBF     D2,is_loop

	    MOVE.L  #SoftInt,IntVects+(INTB_SOFTINT*IV_SIZE)+IV_CODE(A6)
	    MOVE.W  #INTF_SETCLR+INTF_SOFTINT,_intena

	    MOVEM.L (SP)+,D2/D3/A2/A3
	    RTS

;This table was simplified to save space
is_parms:
	    DC.W    INTB_PORTS,INTF_PORTS
	    DC.W    INTB_VERTB,INTF_VERTB
	    DC.W    INTB_COPER,INTF_COPER
	    DC.W    INTB_EXTER,INTF_EXTER
	    DC.W    INTB_NMI,0


************************************************************************
*
*	General Purpose Interrupt Server Handler
*
************************************************************************
*
*	Called with:
*		A0 -> chips
*		A1 -> server data
*
************************************************************************

*** Neil, 30 Mar 86 -- try taking out "extra" enables and request clearings
****	MOVE	IL_SIntReq(A1),intreq(A0)
****	MOVE	IL_SIntEna(A1),intena(A0)
****	MOVE	IL_EIntEna(A1),intena(A0)
*** Bryce -- change so supervisor stack is not left misaligned, tighten loop

IntServer:
	IFNE	TORTURE_TEST
	MOVE.W	#1,A6	;Not guaranteed to have ExecBase
	ENDC

	MOVE.L	A2,-(SP)
	MOVE.L	A1,A2	;soak up wait-state on '030
	MOVE.L	A1,-(SP)

	;------ call servers on interrupt chain (time critical code):
is_next:
	SUCC	A2,A2			* get next server node
	SUCC	A2,D0
	BEQ.S	is_end			* bra if so
	MOVEM.L IS_DATA(A2),A1/A5       * get server data & code
	JSR	(A5)                    * call server
	BEQ.S	is_next

is_end:
	MOVE.L	(SP)+,A1
*-----
* Note!!!  This is really a bug.  We should not be clearing the
* interrupt request *after* the servers run but before they run
* since an interrupt may come in while they are running and it
* needs to be serviced.  The fix is simple, just move the following
* line up to the line right before the "move.l a1,a2 ;soak ..."
* and all will be better except that we now have changed the way
* the system works.  Maybe next time we will really fix this...
*-----
	MOVE.W	IL_SIntReq(A1),_intreq
	MOVE.L	(SP)+,A2
	RTS


******* exec.library/Cause ****************************************************
*
*   NAME
*       Cause -- cause a software interrupt
*
*   SYNOPSIS
*       Cause(interrupt)
*	     A1
*
*       void Cause(struct Interrupt *);
*
*   FUNCTION
*	This function causes a software interrupt to occur.  If it is
*	called from user mode (and processor level 0), the software
*	interrupt will preempt the current task.  This call is often used
*	by high-level hardware interrupts to defer medium-length processing
*	down to a lower interrupt level.  Note that a software interrupt is
*	still a real interrupt, and must obey the same restrictions on what
*	system function it may call.
*
*	Currently only 5 software interrupt priorities are implemented:
*	-32, -16, 0, +16, and +32.  Priorities in between are truncated,
*	values outside the -32/+32 range are not allowed.
*
*   NOTE
*	When setting up the Interrupt structure, set the node type to
*	NT_INTERRUPT, or NT_UNKOWN.
*
*   IMPLEMENTATION
*	1> Checks if the node type is NT_SOFTINT.  If so does nothing since
*	   the softint is already pending.  No nest count is maintained.
*	2> Sets the node type to NT_SOFTINT.
*	3> Links into one of the 5 priority queues.
*	4> Pokes the hardware interrupt bit used for softints.
*
*	The node type returns to NT_INTERRUPT after removal from the list.
*
*   INPUTS
*	interrupt - pointer to a properly initialized interrupt node
*
*   BUGS
*	Unlike other Interrupts, SoftInts must preserve the value of A6.
*
*******************************************************************************
*	Do we want some sort of counter to tell us how many times
*	the interrupt has been caused (without being serviced)?

CALC_Q	    MACRO
	    AND.W   #SIH_PRIMASK,\1	* upper pri bits
	    EXT.W   \1
	    LEA     (SoftInts+(SH_SIZE<<1))(A6),\2 * base of header area
	    ADD.W   \1,\2
	    ENDM

    ;Kludge for argasm bug
    IFNE -(SIH_PRIMASK+1)&SH_SIZE
	    FAIL    !!! mask or structure wrong size !!!
    ENDC
    IFLT 255-NT_SOFTINT
	    FAIL    !!! Recode NT_SOFTINT assumption !!!
    ENDC

Cause:
	    DISABLE

*	    ------- is this software interrupt already active?
	    MOVEQ   #NT_SOFTINT,D0
	    CMP.B   LN_TYPE(A1),D0
	    BEQ.S   ca_exit		* if so, exit
	    MOVE.B  D0,LN_TYPE(A1)

*	    ------- calculate s.i. queue base and queue it:
	    ;[D0.L=NT_SOFTINT]
	    MOVE.B  LN_PRI(A1),D0
	    CALC_Q  D0,A0
	    ADDTAIL

*	    ------- schedule it to occur:
	    MOVE.W  #INTF_SETCLR+INTF_SOFTINT,_intreq   ; tickle interrupt
	    BSET    #(SB_SINT-8),SysFlags(A6)		; (we're disabled)
ca_exit:    ENABLE
	    NOP		; Kludge to ensure write to _intena has completed.
	    RTS		; Fixes a few programs.  Note that some instructions
			; may still execute before the interrupt is taken.



*----------------------------------------------------------------
*
*   Software interrupt processing
*
*	Runs in supervisor mode at priority 1.
*
*----------------------------------------------------------------

	CNOP	0,4	;Nice alignment for 68020
SoftInt:
	    MOVE.W  #INTF_SOFTINT,_intreq
	    BCLR    #(SB_SINT-8),SysFlags(A6)
	    BNE.S   si_disable
	    RTS

si_disable:
	    MOVE.W  #INTF_SOFTINT,_intena * disable softints
	    BRA.S   si_start

si_remove:
	    MOVE    #$2700,SR		* fast disable
	    REMHEADQ A0,A1,A5
	    MOVE.B  #NT_INTERRUPT,LN_TYPE(A1) * allow requeuing
	    MOVE    #$2000,SR		* open for all interrupts

	    MOVEM.L IS_DATA(A1),A1/A5
	    JSR     (A5)

si_start:
	    MOVEQ.L #SIH_QUEUES-1,D0	* foreach queue
	    LEA     SoftInts+((SIH_QUEUES-1)*SH_SIZE)(A6),A0
	    MOVE.W  #INTF_SOFTINT,_intreq
si_next:
	    IFNOTEMPTY A0,si_remove	* do it all again
	    LEA     -SH_SIZE(A0),A0     * next queue
	    DBF     D0,si_next

	    MOVE    #$2100,SR
	    MOVE.W  #INTF_SETCLR+INTF_SOFTINT,_intena
	    RTS


******* exec.library/Disable **************************************************
*
*    NAME
*	Disable -- disable interrupt processing.
*
*    SYNOPSIS
*	Disable();
*
*	void Disable(void);
*
*    FUNCTION
*	Prevents interrupts from being handled by the system, until a
*	matching Enable() is executed.  Disable() implies Forbid().
*
*	DO NOT USE THIS CALL WITHOUT GOOD JUSTIFICATION.  THIS CALL IS
*	VERY DANGEROUS!
*
*    RESULTS
*	All interrupt processing is deferred until the task executing makes
*	a call to Enable() or is placed in a wait state.  Normal task
*	rescheduling does not occur while interrupts are disabled.  In order
*	to restore normal interrupt processing, the programmer must execute
*	exactly one call to Enable() for every call to Disable().
*
*	IMPORTANT REMINDER:
*
*	It is important to remember that there is a danger in using
*	disabled sections.  Disabling interrupts for more than ~250
*	microseconds will prevent vital system functions (especially serial
*	I/0) from operating in a normal fashion.
*
*	Think twice before using Disable(), then think once more.
*	After all that, think again.  With enough thought, the need
*	for a Disable() can often be eliminated.  For the user of many
*	device drivers, a write to disable *only* the particular interrupt
*	of interest can replace a Disable().  For example:
*			MOVE.W	#INTF_PORTS,_intena
*	Do not use a macro for Disable(), insist on the real thing.
*
*	This call may be made from interrupts, it will have the effect
*	of locking out all higher-level interrupts (lower-level interrupts
*	are automatically disabled by the CPU).
*
*        Note: In the event of a task entering a Wait() after disabling
*              interrupts, the system "breaks" the disabled state and runs
*              normally until the task which called Disable() is rescheduled.
*
*    NOTE
*	This call is guaranteed to preserve all registers.
*
*    SEE ALSO
*	Forbid, Permit, Enable
*
*******************************************************************************
Disable:
	    DISABLE
	    RTS

******* exec.library/Enable ***************************************************
*
*   NAME
*	Enable -- permit system interrupts to resume.
*
*   SYNOPSIS
*	Enable();
*
*	void Enable(void);
*
*   FUNCTION
*	Allow system interrupts to again occur normally, after a matching
*	Disable() has been executed.
*
*   RESULTS
*	Interrupt processing is restored to normal operation. The
*	programmer must execute exactly one call to Enable() for every call
*	to Disable().
*
*    NOTE
*	This call is guaranteed to preserve all registers.
*
*   SEE ALSO
*	Forbid, Permit, Disable
*
*
*******************************************************************************
Enable:
	    ENABLE
	    RTS



******* exec.library/StackSwap ************************************************
*
*   NAME
*	StackSwap - EXEC supported method of replacing task's stack      (V37)
*
*   SYNOPSIS
*	StackSwap(newStack)
*	          A0
*
*	VOID StackSwap(struct StackSwapStruct *);
*
*   FUNCTION
*	This function will, in an EXEC supported manner, swap the
*	stack of your task with the given values in StackSwap.
*	The StackSwapStruct structure will then contain the values
*	of the old stack such that the old stack can be restored.
*	This function is new in V37.
*
*   NOTE
*	If you do a stack swap, only the new stack is set up.
*	This function does not copy the stack or do anything else
*	other than set up the new stack for the task.  It is
*	generally required that you restore your stack before
*	exiting.
*
*   INPUTS
*	newStack - A structure that contains the values for the
*		new upper and lower stack bounds and the new stack
*		pointer.  This structure will have its values
*		replaced by those in you task such that you can
*		restore the stack later.
*
*   RESULTS
*	newStack - The structure will now contain the old stack.
*		This means that StackSwap(foo); StackSwap(foo);
*		will effectively do nothing.
*
*   SEE ALSO
*	AddTask, RemTask, exec/tasks.h
*
*******************************************************************************
;
;	a0 - 12 byte memory array
;		APTR	stk_Lower	;Lowest byte of stack
;		ULONG	stk_Upper	;Upper end of stack (size + Lowest)
;		APTR	stk_Pointer	;Stack pointer to switch to
;
StackSwap:	move.l	ThisTask(a6),a1
		;Take care if removing! (it can be)
		DISABLE

		;:TODO: investigate 68881 stack!!!
		move.l	TC_SPUPPER(a1),d0
		move.l	stk_Upper(a0),TC_SPUPPER(a1)
		move.l	d0,stk_Upper(a0)

		move.l	TC_SPLOWER(a1),d0
		move.l	stk_Lower(a0),TC_SPLOWER(a1)
		move.l	d0,stk_Lower(a0)

		move.l	stk_Pointer(a0),TC_SPREG(a1)	;worthless

		;[register switch]
		move.l	stk_Lower(a0),a1
		move.l	#STACK_COOKIE,(a1)
		move.l	stk_Pointer(a0),a1
		move.l	(sp)+,d0	   ;steal return address from old stack
		move.l	sp,stk_Pointer(a0) ;store location to go back to
		move.l	d0,-(a1)	   ;stuff RTS address on new stack
		move.l	a1,sp		   ;switcherooo
		ENABLE
		rts


******* exec.library/ObtainQuickVector ****************************************
*
*   NAME
*	Function to obtain an install a Quick Interrupt vector            (V39)
*
*   SYNOPSIS
*	vector=ObtainQuickVector(interruptCode)
*	d0                       a0
*
*	ULONG ObtainQuickVector(APTR);
*
*   FUNCTION
*	This function will install the code pointer into the quick interrupt
*	vector it allocates and returns to you the interrupt vector that
*	your Quick Interrupt system needs to use.
*
*	This function may also return 0 if no vectors are available.  Your
*	hardware should be able to then fall back to using the shared
*	interrupt server chain should this happen.
*
*	The interrupt code is a direct connect to the physical interrupt.
*	This means that it is the responsibility of your code to do all
*	of the context saving/restoring required by interrupt code.
*
*	Also, due to the performance of the interrupt controller, you may
*	need to also watch for "false" interrupts.  These are interrupts
*	that come in just after a DISABLE.  The reason this happens is
*	because the interrupt may have been posted before the DISABLE
*	hardware access is completed.  For example:
*
*	myInt:		move.l	d0,-(sp)	; Save d0...
*			move.w	_intenar,d0	; Get interrupt enable state
*			btst.l	#INTB_INTEN,d0	; Check if pending disable
*			bne.s	realInt		; If not, do real one...
*	exitInt:	move.l	(sp)+,d0	; Restore d0
*			rte			; Return from int...
*	;
*	realInt:	; Now do your int code...  d0 is already saved
*			; ALL other registers need to be saved if needed
*			; This includes a0/a1/d0/d1 as this is an interrupt
*			; and not a function call...
*			;
*			bra.s	exitInt		; Exit interrupt...
*
*	If your interrupt will not play with system (OS) structures and your
*	own structures are safe to play with you do not need to check for
*	the disable.  It is only needed for when the system is in disable but
*	that "one last interrupt" still got through.
*
*   NOTE
*	This function was not implemented fully until V39.  Due to a mis-cue
*	it is not safe to call in V37 EXEC.  (Sorry)
*
*   INPUTS
*	A pointer to your interrupt code.  This code is not an EXEC interrupt
*	but is dirrectly connected to the hardware interrupt.  Thus, the
*	interrupt code must not modify any registers and must return via
*	an RTE.
*
*   RESULTS
*	The 8-bit vector number used for Zorro-III Quick Interrupts
*	If it returns 0, no quick interrupt was allocatable.  The device
*	should at this point switch to using the shared interrupt server
*	method.
*
*   SEE ALSO
*
*******************************************************************************
ObtainQuickVector:	xdef	ObtainQuickVector

	IFND QUICK_INTS_ENABLED
		moveq.l	#0,d0
		rts
	ENDC

	IFD QUICK_INTS_ENABLED
		move.l	a5,a1		; Save a5...
		JMPTRAP	GetQuickVec(pc)	; The real routine...
GetQuickVec:	move.l	a1,a5		; Restore a5...
		sub.l	a1,a1		; Clear a1
		or.w	#$0700,sr	; Disable...
		btst.b  #AFB_68010,AttnFlags+1(a6)
		beq.s	1$		; If not >= 68010, skip
*
		opt	p=68010
		movec.l	vbr,a1		; Get vector base register
		opt	p=68000
*
1$		lea	$400(a1),a1	; Point at user vectors
		move.l	#$FF,d0		; Last vector number...
		move.l	#Bad_Quick_Int,d1	; Empty vecs point at this...
2$		cmp.l	-(a1),d1	; Check if empty...
		dbeq.s	d0,2$		; Loop around for is not found...
		bne.s	NoVector	; If we did not find one, exit...
		cmp.w	#68,d0		; Check if we are below limit...
		bcs.s	NoVector	; If invalid, exit...
		move.l	a0,(a1)		; Store the vector...
		rte			; Return...
NoVector:	moveq.l	#0,d0		; For now, we just return failure...
		rte			; Return...
	ENDC

*
* This is the "default" quick interrupt vector that causes the
* Bad Quick Interrupt ALERT.
*
Bad_Quick_Int:	xdef	Bad_Quick_Int
		ALERT	AN_BadQuickInt

    END
