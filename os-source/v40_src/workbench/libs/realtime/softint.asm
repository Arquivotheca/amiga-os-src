
        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

	INCLUDE "exec/types.i"
	INCLUDE "exec/interrupts.i"
	INCLUDE "exec/macros.i"
	INCLUDE "utility/hooks.i"
	INCLUDE "realtimebase.i"

	LIST

;---------------------------------------------------------------------------

        xdef    @SoftIntHandler
        xdef	@CIAHandler68020
        xdef	@CIAHandler68030

;---------------------------------------------------------------------------

****************************************************************
*
*   SoftIntHandler
*
*   FUNCTION
*       Handle Timer TickInt (SoftInt)
*           . Process AlarmList for hash.
*           . Call player hooks
*
*   INPUTS
*       A1 - RealTimeBase
*
*   RESULTS
*       None
*
*   WARNING
*	This handler trashes A0/A1/D0/D1/A5
*
****************************************************************

	CNOP 0,4   	; help the 020 and above...

@CIAHandler68020:
	addq.l	#1,rtl_Time(a1)

@CIAHandler68030:
	addq.l	#1,rtl_Time(a1)		; increment the time counter

@SoftIntHandler:

        ; we can't do anything if someone's got the player list locked
	tst.w	rtl_PlayerListLock+SS_QUEUECOUNT(a1)
	bge.s	1$	   ; if SS_QUEUECOUNT >= 0, semaphore is locked

        movem.l d2/d3/d4/d5/a2/a3/a4/a6,-(sp)

	move.l	rtl_ConductorList+MLH_HEAD(a1),a3 ; A3 = first conductor
	move.l  LN_SUCC(a3),d3			  ; load node after that...
	bne.s	2$				  ; if another node, enter loop
        movem.l (sp)+,d2/d3/d4/d5/a2/a3/a4/a6	  ; nothing to do, false alarm
1$:     rts					  ; bye

2$:     lea	rtl_Message(a1),a5     		  ; A5 = hook message
        move.l  rtl_SysBase(a1),a6		  ; A6 = SysBase
        move.l  rtl_Time(a1),d2         	  ; D2 = current time

th_condloop:
	tst.b	ec_Stopped(a3)			   ; is conductor stopped?
	bne	th_condnext	 		   ; if stopped, then skip it

	bclr.b	#CONDUCTB_METROSET,1+ec_Flags(a3) ; clear metronome set bit

	; begin calculation of ClockTime

	move.l	d2,d0			; D0 = current time
	sub.l	ec_StartTime(a3),d0    ; D0 = time from start

	btst.b	#CONDUCTB_EXTERNAL,1+ec_Flags(a3) ; external sync?
	bne.s	th_noextsync			   ; no external sync, so skip

	move.l	ec_ExternalTime(a3),d1	; get ExternalTime
	cmp.l	d1,d0			; ClockTime below ExternalTime?
	bge.s	th_timeok1		; no, everything ok
	move.l	d1,d0			; set ClockTime to ExternalTime

th_timeok1
	cmp.l	ec_MaxExternalTime(a3),d0 ; ClockTime above MaxExternalTime?
	blt.s	th_noextsync		; no, everything ok

	move.l  ec_ClockTime(a3),d5	; D1 = old conductor clock time
	cmp.l	d5,d0			; new ClockTime below old ClockTime?
	blt.s	th_badtime		; yes, so don't save new time

th_noextsync
	move.l	d0,ec_ClockTime(a3)
	move.l  d0,d5			; D1 = new conductor clock time

th_badtime
	move.l	d5,pmt_Time(a5) 	      ; prepare hook message
	move.l	ec_Players+MLH_HEAD(a3),a2   ; A2 = first player
	move.l	LN_SUCC(a2),d4		      ; load player after that
	; we're guaranteed of at least one node on this list, so skip
	; initial check

th_playloop
	btst.b	#PLAYERB_QUIET,1+ep_Flags(a2)   ; is player quiet?
	bne.s	th_playnext			; if quiet, skip it

	; call hook

	move.l	ep_Hook(a2),d0		; is there a hook?
	bne.s	th_dohook		; yep, go do it

th_nohook
	; fill in MetricTime
	move.l	d5,d0    		; D0 = current conductor time
	move.l	d0,ep_MetricTime(a2)
	bra.s	th_afterhook

th_dohook
	move.l	d0,a0		   	; A0 = Hook
	move.l	a5,a1 			; A1 = message
        move.l  h_Entry(a0),a4     	; A4 = address of hook function
        jsr	(a4)		   	; do hook
	move.l	ep_MetricTime(a2),d0	; D0 = current metric time

th_afterhook
	cmp.l	ep_AlarmTime(a2),d0	; MetricTime >= AlarmTime
	blt.s	th_playnext		; if not, then don't signal

	bclr.b	#PLAYERB_ALARMSET,1+ep_Flags(a2) ; test and clear
	beq.s	th_playnext			 ; alarm not set, skip

        move.l  ep_AlarmSigMask(a2),d0		 ; D0 = signal mask
        move.l  ep_Task(a2),a1       		 ; A1 = task to signal
        JSRLIB  Signal

th_playnext
	move.l	d4,a2         		; load next player
	move.l	LN_SUCC(a2),d4		; load player after that
	bne.s	th_playloop		; if another node, we're not done

th_condnext
	move.l	d3,a3			; load next node
	move.l  LN_SUCC(a3),d3		; load node after that...
	bne	th_condloop		; if another node, we're not done...

        movem.l (sp)+,d2/d3/d4/d5/a2/a3/a4/a6
        rts

;---------------------------------------------------------------------------

	END
