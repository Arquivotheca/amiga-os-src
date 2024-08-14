 TTL '$Id: alert.asm,v 39.15 92/10/08 13:54:58 mks Exp $'

*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **			 Alert			 **
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
* $Id: alert.asm,v 39.15 92/10/08 13:54:58 mks Exp $
*
* $Log:	alert.asm,v $
* Revision 39.15  92/10/08  13:54:58  mks
* Cleaned up the code for the alert and beta alert work...
* 
* Revision 39.14  92/07/28  10:11:43  mks
* Cleaned up source to make use of the _LVO defined in amiga.lib
* rather than hard coding it...  (It was commented that this
* will happen)
*
* Revision 39.13  92/06/18  09:20:19  mks
* Moved the alert.hook to be after the boot menu...
* This makes the beta alert be a bit smaller since it no longer
* needs to test for the mouse buttons.
*
* Revision 39.12  92/06/08  15:46:15  mks
* Removed the conditional INCLUDE_WACK since it no longer exists
*
* Revision 39.11  92/05/27  16:03:27  mks
* Added conditional code to make all non-ROM exec builds have the BETA
* alert in them...
*
* Revision 39.10  92/05/26  17:15:05  mks
* Changed to be 8 seconds and must use both mouse buttons before it is
* skipped.  (For bootmenu)
*
* Revision 39.9  92/05/26  08:55:59  mks
* Check to see if mouse button is down before putting up alert...
*
* Revision 39.8  92/05/22  11:36:12  mks
* Added autodoc to this file and change the timeout method
*
* Revision 39.7  92/05/21  18:22:10  mks
* Added beta release startup alert
*
* Revision 39.6  92/05/21  11:20:31  mks
* Added CR/LF to the alert.hook ID
*
* Revision 39.5  92/04/10  18:39:44  mks
* Timed alerts now in!  (Next to get the colour right...)
*
* Revision 39.4  92/04/06  07:18:19  mks
* Changed the way it calls WackCrash (which is now SADCrash)
*
* Revision 39.3  92/02/10  13:47:58  mks
* Saved some space in the allert hook...
*
* Revision 39.2  92/01/27  14:26:05  mks
* Removed the CDTV custom exec as it is no longer needed...
* (New magic in CDTV ROM)
*
* Revision 39.1  92/01/21  14:54:23  mks
* Folded in the changes for CDTV_CR
*
* Revision 39.0  91/10/15  08:26:10  mks
* V39 Exec initial checkin
*
**********************************************************************


;****** Included Files ***********************************************

    NOLIST
    INCLUDE	"assembly.i""
    INCLUDE	"constants.i"

    INCLUDE	"types.i"
    INCLUDE	"nodes.i"
    INCLUDE	"lists.i"
    INCLUDE	"ables.i"
    INCLUDE	"libraries.i"
    INCLUDE	"resident.i"
    INCLUDE	"alerts.i"
    INCLUDE	"execbase.i"

    INCLUDE	"librarytags.i"

    INCLUDE	"hardware/cia.i"
    INCLUDE	"hardware/custom.i"
    INCLUDE	"hardware/intbits.i"
    INCLUDE	"calls.i"

    INCLUDE	"romconstants.i"

    LIST


;****** Imported Globals *********************************************

    INT_ABLES

    EXTERN_DATA _ciaaddra
    EXTERN_DATA _ciaapra

    EXTERN_DATA _serper
    EXTERN_DATA _serdat
    EXTERN_DATA _serdatr
    EXTERN_DATA _potgo
    EXTERN_DATA _potinp

    EXTERN_DATA _intreq

    EXTERN_DATA VERNUM


;****** Imported Functions *******************************************

	XREF	WackCrash
	XREF	CrashReset
	XREF	TaggedOpenLibrary

	XREF	_LVORawDoFmt
	XREF	_LVOOldOpenLibrary
	XREF	_LVOCloseLibrary


;****** Exported Functions *******************************************

	XDEF	Alert
	XDEF	FatalProc
	XDEF	DfltTaskTrap

; Call an external library
JSRLIB		MACRO	;FunctionName
		XREF	_LVO\1
		jsr	_LVO\1(a6)
		ENDM

*----------------------------------------------------------------
*
*   FatalProc -- process fatal system exceptions
*
*	no registers have been saved, CPU priority is the same
*	00(sp) == exception number
*	04(sp) == exception frame
*
*----------------------------------------------------------------
FatalProc:
DfltTaskTrap:
	    AZ_TRIG 1			; Trigger analyzer
	    MOVE.W  #$4000,_intena	; disable all interrupts
	    IFGE INFODEPTH-2
	      MOVEM.L D0-D7/A0-A7,REG_STORE
	      move.l	(sp),d0	;Get Exception number
	      move.l	sp,d1	;Get SSP
	      move.l	usp,a0	;Get USP
	      NPRINTF	2,<'Fatal Exception %lx!  SSP-%lx USP-%lx'>,d0,d1,a0
	      move.l	 4(sp),d0
	      move.l	 8(sp),d1
	      move.l	12(sp),d2
	      move.l	16(sp),d3
	      move.l	20(sp),d4
	      NPRINTF	2,<'Supr Stack frame: %08lx %08lx %08lx %08lx'>,d0,d1,d2,d3,d4
	      move.l	 0(a0),d0
	      move.l	 4(a0),d1
	      move.l	 8(a0),d2
	      move.l	12(a0),d3
	      move.l	16(sp),d4
	      NPRINTF	2,<'User Stack frame: %08lx %08lx %08lx %08lx'>,d0,d1,d2,d3,d4
	      MOVEM.L   REG_STORE,D0-D7/A0-A7
	    ENDC

	    MOVE.L  (SP)+,D7        ; get Alert number & set proper wack frame.
	    BSET    #31,D7	    ; Niel disabled all recoverable exceptions
	    BRA.S   Exception_Alert ; Alerts.

******* exec.library/Alert ****************************************************
*
*   NAME
*	Alert -- alert the user of an error
*
*   SYNOPSIS
*	Alert(alertNum)
*	      D7
*
*	void Alert(ULONG);
*
*   FUNCTION
*	Alerts the user of a serious system problem.  This function will
*	bring the system to a grinding halt, and do whatever is necessary
*	to present the user with a message stating what happened.
*	Interrupts are disabled, and an attempt to post the alert is made.
*	If that fails, the system is reset.  When the system comes up
*	again, Exec notices the cause of the failure and tries again to
*	post the alert.
*
*	If the Alert is a recoverable type, this call MAY return.
*
*	This call may be made at any time, including interrupts.
*	(Well, only in interrupts if it is non-recoverable)
*
*	New, for V39:
*	The alert now times out based on the value in LastAlert[3]
*	This value is transfered accross warm-reboots and thus will let
*	you set it once.  The value is the number of frames that need to
*	be displayed before the alert is auto-answered.  A value of
*	0 will thus make the alert never be displayed.  Note that
*	it is recommended that applications *NOT* change the value in
*	LastAlert[] since the main reason for this is to make
*	unattended operation of the Amiga (in production enviroments)
*	possible.
*
*   POST-MORTEM DIAGNOSIS
*	There are several options for determining the cause of a crash.
*	Descriptions of each alert number can be found in the "alerts.h"
*	include file.
*
*	A remote terminal can be attached to the Amiga's first built-in
*	serial port.  Set the communication parameters to 9600 baud, 8 bits,
*	no parity.  Before resetting the machine, the Alert function will
*	blink the power LED 10 times.  While the power indicator is flashing,
*	pressing DELETE on the remote terminal will invoke the ROM debugger.
*
*   INPUT
*	alertNum   - a number indicating the particular alert.  -1 is
*	             not a valid input.
*
*   NOTE
*	Much more needs to be said about this function and its implications.
*
*   SEE ALSO
*	exec/alerts.h
*
*******************************************************************************
Alert:	    AZ_TRIG 1			; Trigger analyzer
;------ Stop everything for the moment.  The system could be in very bad
;	shape.	Since we now have control, we don't want to take the
;	chance of losing it.  Make a note in location 0 that we are
;	processing an alert.
;	Note that Intuition's DisplayAlert() breaks our DISABLE.
	    MOVE.W  #$4000,_intena	; disable all interrupts
   ; Kill!  MOVEM.L D0-D7/A0-A7,REG_STORE
	    NPRINTF 6,<'Called Alert! %lx  address=%lx'>,d7,(sp)

Exception_Alert:
	    MOVEM.L D0/D1/A0/A1/A5/A6,-(sp)
	    MOVE.L  SP,A5		; default alert parameter
	    MOVE.L  #'HELP',D0          ; alert "in progress" indicator
	    CMP.L   LOCATION_ZERO,D0	; alert indicator address
	    BEQ.s   al_stackin		; we are recursing!
	    MOVE.L  D0,LOCATION_ZERO
	    LEA     ALERT_STORE,A0	; temp storage for alert
	    MOVE.L  D7,(A0)+            ;   number and
	    MOVE.L  A5,(A0)             ;     parameter (leave for later)
	    ;[A0-ALERT_STORE+4]

	;------ Evaluate the state of Exec.
	;	Since this routine may have been called as the
	;	result of a cached pointer, we need to validate
	;	everything (as much as we have code space for).

	    ;------ Validate Exec's library base pointer:
	    MOVE.L  SysBase,D0
	    BTST.L  #0,d0
	    BNE.S   al_stackin
	    MOVE.L  D0,A6
	    ADD.L   ChkBase(A6),D0      ; does it checksum?
	    ADDQ.L  #1,D0
	    BNE.S   al_stackin

	    ;------ ExecBase is ok, extract the task pointer
	    MOVE.L  ThisTask(A6),A5     ; true alert parameter: task
	    MOVE.L  A5,(A0)             ; stuff at ALERT_STORE+4

	    ;------ Is our current stack in memory?
	    MOVE.L  #$F1E2D3C4,D0	; an unusual pattern
	    MOVE.L  D0,-(SP)
	    CMP.L   (SP)+,D0            ; did we pop what we pushed?
	    BNE.S   al_nostack

	    ;------ If this is a deadend alert, don't take the chance of
	    ;------ calling intuition right now.  Restart the machine and
	    ;------ then display the alert.
	    TST.L   D7			; is it a deadend?
	    BMI.S   al_stackin

	    ;------ Display the alert message:
	    LEA     LastAlert(A6),A0
	    MOVE.L  D7,(A0)+		;stuff here in case we need to
	    MOVE.L  A5,(A0)+		;reboot to get the message out
	    BSR     dispAlert

	    ;------ Enable interrupts if necessary:
	    TST.L   IDNestCnt(A6)
	    MOVEM.L (sp)+,D0/D1/A0/A1/A5/A6
	    BGE.S   al_noEnable
	    MOVE.W  #$C000,_intena		;enable interrupts
al_noEnable:

	    ;------ Added new delay here to prevent alert retry race
	    ;------ conditions.  Alerting task is slowed.
	    ; Assumes all registers preserved.
al_delay:   MOVEM.L D0/D1,-(SP)
            MOVEQ.L #1,D0
	    MOVEQ.L #5,D1
al_delay2:  TST.W   SLOW_READ	;Non-cachable chip bus read (slow on any CPU!)
	    DBRA    D0,al_delay2
	    DBRA    D1,al_delay2
            MOVEM.L (SP)+,D0/D1
	    RTS 		;void



	    ;------ Handle a bad stack condition:
al_nostack: LEA.L   TEMP_SUP_STACK,SP	; 68000 	  68010
	    CLR.L   -(sp)               ; PC & SR       PC, SR & Format word
	    CLR.L   -(sp)               ;    "              "

	    ;------ We end up here if anything went wrong above.
al_severe:  MOVEM.L D0/D1/A0/A1/A5/A6,-(sp)
;
al_stackin: AND.B   #~(CIAB_LED!CIAB_OVERLAY),_ciaapra
	    OR.B    #(CIAF_OVERLAY!CIAF_LED),_ciaaddra
;
; Broken!  Does not work with VBR...  (arg!)
;
	    ;------ Stop all activities and assert supervisor mode:
	    MOVE.L  #al_super,PrivTrapVector	;loop back to grab control!
al_super:   MOVE    #$2700,SR	;Set supervisor mode & Disable

	    MOVEQ   #5,D1
	    MOVE.W  #BAUD_9600,_serper

al_wait:    ;------ blink the LED:
	    MOVEQ   #-1,D0
al_on:      BSET.B  #CIAB_LED,_ciaapra		; inside loop for delay
	    DBF     D0,al_on
al_off:     BCLR.B  #CIAB_LED,_ciaapra		; inside loop for delay
	    DBF     D0,al_off

	    ;------- poll for delete
	    MOVE.W  _serdatr,D0
	    MOVE.W  #INTF_RBF,_intreq	; clear rbf flag
	    AND.B   #$7f,D0		; parity doesn't count
	    CMP.B   #$7f,D0		; looking for DEL
	    DBEQ    D1,al_wait

	    movem.l (sp)+,d0/d1/a0/a1/a5/a6	; Restore...
	    beq     WackCrash		; Call dead-end crash!

	    bra     CrashReset		; Reboot the system...


;-----------------------------------------------------------------------
;	dispAlert -- format strings and display the alert
;-----------------------------------------------------------------------
;	A6=ExecBase.  LastAlert(a6)=alert number
alertTagEntry:
	;----- Clear location zero.  Under 1.3 zero was cleared when
	;----- all of chip was set to NULL.  Under 2.0 we can't always
	;----- find ExecBase in time to decide.  Games sometimes store
	;----- data at zero and reboot.  So, I clear it here after all
	;----- the captures have triggered.
	CLR.L	LOCATION_ZERO

dispAlert:
	IFNE	BETA_BUILD
		pea	Beta_Alert(pc)	; We will run this on exit...
	ENDC

		movem.l d2/d7/a2/a3/a6,-(sp)

		;------ verify that an alert condition exists:
		move.l	LastAlert(a6),d2
		NPRINTF 100,<'LastAlert = %lx'>,d2
		moveq.l #-1,d0
		cmp.l	d0,d2
		beq.s	da_rts		; there is no alert!

		;------ allocate local workspace from stack:
		lea	-STRINGBUF(sp),sp ; string buffer space
		lea	(sp),a3           ; string buffer pointer

		;------ determine what message to display:
		lea	msgHardFail(pc),a0
		move.l	d2,d0
		swap	d0
		cmp.b	#((AG_NoMemory>>16)&$ff),d0
		bne.s	da_realFail
		lea	msgNoMemory(pc),a0
		bra.s	da_makeString
da_realFail:
		;------ it wasn't 'out of memory'.  check if 'recoverable'
		btst	#31,d2
		bne.s	da_makeString

		;------ strip out exception alerts
		tst.w	d0
		beq.s	da_makeString

		;------ put in 'recoverable' message
		lea	msgSoftFail(pc),a0
da_makeString:

		;------ construct the alert failure line:
		bsr.s	consStr ;(a0,a3)
		lea	msgMouse(pc),a0
		bsr.s	consStr ;(a0,a3)

		;------ construct the alert number line:
		clr.b	(a3)+           ; first byte of length
		lea	msgAlertNum(pc),a0
		lea	LastAlert(a6),a1 ;parametrs (word,word,long)
		lea	stuffChar(pc),a2
		JSRSELF	RawDoFmt	; (a0,a1,a2,a3)

		;------ open the intuition library:
		moveq.l	#OLTAG_INTUITION,d0	; Open intuition, tagged...
		bsr	TaggedOpenLibrary	; Open it up...
		tst.l	d0
		beq	al_severe	;(what can we do? !!!)
		;We go to al_severe and pick up alert number after RESET.

		;------ display the alert condition:
		move.l	LastAlert+4*3(a6),a1	; Get timeout...

		move.l	a6,a3		; temp storage
		move.l	d0,a6
		move.l	d2,d0		; alert number for Intuition color code
		lea	(sp),a0         ; message
		moveq.l #40,d1		; height
		JSRLIB	TimedDisplayAlert	;(d0,a0,d1,a1)
		move.l	d0,a2

		;------ close the intuition library:
		move.l	a6,a1
		move.l	a3,a6
		JSRSELF	CloseLibrary

		;------ clean up the stack:
		lea	STRINGBUF(sp),sp

da_disable:
		;------ disable the alert condition:
		IFEQ	TORTURE_TEST_0
		  clr.l   LOCATION_ZERO
		ENDC
		IFNE	TORTURE_TEST_0
		  move.l  #ZERO_COOKIE,LOCATION_ZERO
		ENDC
		moveq.l #-1,d0
		move.l	d0,LastAlert(a6)

da_rts:		move.l	a2,d0
		movem.l (sp)+,d2/d7/a2/a3/a6
		rts

consStr:
		;------ A0 -> the string to move into the display buffer.
		clr.b	(a3)+           ; first byte of length
ic_move:	move.b	(a0)+,(a3)+
		bne.s	ic_move
		st.b	(a3)+           ; continuation condition
		rts

stuffChar:
		;------ d0 == character to print
		;	a3 -> string to deposit char into
		move.b	d0,(a3)+        ; deposit current character
		clr.b	(a3)            ; clear next byte (for EOL).
		rts

;=======================================================================
;	*** END OF EXEC ***
;=======================================================================

EndMarker:	XDEF	EndMarker	; used in the Exec resident tag

;-----------------------------------------------------------------------
;	System Reset Alert Capture Tag
;-----------------------------------------------------------------------
;	This tag will catch the system after a reset, and after
;	the intuition library has been initialized, but before
;	the DOS bootstrap.
;-----------------------------------------------------------------------

AlertresidentTag:
		DC.W	RTC_MATCHWORD
		DC.L	AlertresidentTag
		DC.L	alertEndMarker
		DC.B	RTF_COLDSTART
		DC.B	VERNUM
		DC.B	0		; no type
		DC.B	-55		; priority
		DC.L	idString
		DC.L	idString
		DC.L	alertTagEntry

	IFNE	BETA_BUILD
*******************************************************************************
*
* Magic code to put up special alert for BETA releases
*
Beta_Alert:	movem.l	d0-d1/a0-a2,-(sp)	; Save...
		moveq.l	#OLTAG_INTUITION,d0
		bsr	TaggedOpenLibrary	; Open it up...
		tst.l	d0			; Did we get it?
		beq.s	NoBetaAlert
		move.l	a6,a2			; Save here...
		move.l	d0,a6			; Intuitionbase
*
		moveq.l	#0,d0			; Clear d0 (recovery)
		lea	BetaString(pc),a0	; Get beta string
		moveq.l	#46,d1			; Height...
		move.l	#8*60,a1		; Timeout (8 seconds)
		JSRLIB	TimedDisplayAlert
*
		move.l	a6,a1			; Get IntuitionBase
		move.l	a2,a6			; ExecBase again
		JSRSELF	CloseLibrary
NoBetaAlert:	movem.l	(sp)+,d0-d1/a0-a2	; Restore
		rts
*
*******************************************************************************
*
* Strings for special BETA alert...
*
BetaString:	dc.b	$00,$90,$10
		dc.b	'BETA release for registered developers only!',$00,$01
		dc.b	$00,$F8,$20
		dc.b	'DO NOT DISTRIBUTE!',$00,$00
*
*******************************************************************************
	ENDC

;-----------------------------------------------------------------------
;	Strings
;-----------------------------------------------------------------------

STRINGBUF	EQU	200	;maximum possible string length
msgNoMemory	dc.b	38,15,'Not enough memory. ',0
*
;msgHardFail	dc.b	38,15,'Hardware Failure :-) ',0
msgHardFail	dc.b	38,15,'Software Failure. ',0
*
msgSoftFail	dc.b	38,15,'Recoverable Alert. ',0
msgMouse:	dc.b	234,15,'Press left mouse button to continue.',0
msgAlertNum	dc.b	142,30,'Error:  %04x %04x   Task:  %08lx',0
idString	dc.b	'alert.hook',13,10,0

alertEndMarker:

    END
