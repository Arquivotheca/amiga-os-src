**
**      $Id: ddebug.i,v 1.1 92/07/08 14:24:20 darren Exp $
**
**      debug equates
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	INCLUDE	"exec/types.i"
	INCLUDE "exec/macros.i"

**
** Darren's debug stuff
**

* Set debug level of -
*
*	0	: No debugging code
*
*	>= 1	: Debugging code generated (requires debug.lib)
*
*	Even	: Even numbers print LF/CR at end of line
*
*	Odd	: Odd numbers do not print LF/CR at end of line
*
* See exec/macros.i for PRINTF macro
*

* Useful equates for setting debug level

DBG_BASIC	EQU	2		; very basic level of reporting

DBG_ENTRY	EQU	10		; report entry to routine

DBG_EXIT	EQU	100		; report exit from routine

DBG_FLOW	EQU	500		; report flow

DBG_OSCALL	EQU	1000		; debug OS call going in, or out

DBG_HARDWARE	EQU	10000		; report when poking hardware

DBG_INTERRUPT	EQU	20000		; report within an interrupt
					; add to lower-level DBGging

DBG_ALL		EQU	40000		; report everything

*DEBUG_DETAIL	SET	DBG_ALL
DEBUG_DETAIL	SET	0

* Push BYTE on stack as long

PUSHBYTE	MACRO
		IFNE	DEBUG_DETAIL
		clr.l	-(sp)
		move.b	\1,3(sp)
		ENDC
		ENDM
* Push WORD on stack as a long

PUSHWORD	MACRO
		IFNE	DEBUG_DETAIL
		clr.l	-(sp)
		move.w	\1,2(sp)
		ENDC
		ENDM

* Push LONG on stack as a long

PUSHLONG	MACRO
		IFNE	DEBUG_DETAIL
		move.l	\1,-(sp)
		ENDC
		ENDM

* PULL LONG off of stack

POPLONG		MACRO	;number of longs to pop
		IFNE	DEBUG_DETAIL
POPCOUNTER	SET	\1*4
		lea.l	POPCOUNTER(sp),sp
		ENDC
		ENDM

* Extend byte on stack

EXTBYTE		MACRO
		IFNE	DEBUG_DETAIL
		move.l	d0,-(sp)
		move.l	4(sp),d0
		ext.w	d0
		ext.l	d0
		move.l	d0,4(sp)
		move.l	(sp)+,d0
		ENDC
		ENDM

* Extend word on stack

EXTWORD		MACRO
		IFNE	DEBUG_DETAIL
		move.l	d0,-(sp)
		move.l	4(sp),d0
		ext.l	d0
		move.l	d0,4(sp)
		move.l	(sp)+,d0
		ENDC
		ENDM


* Useful macro for checking that stack hasn't been corrupted

STACKCHECK	MACRO
	PRINTF	DBG_FLOW,<'***SP=$%lx (SP)=$%lx***'>,sp,(sp)
		ENDM

** Traps

DEBUG_TRAP	MACRO	;value stored at $0
		IFNE	DEBUG_DETAIL
		move.w	#\1,$0
		ENDC
		ENDM

* Short branch conditional macros (too bad CAPE doesn't have a swtich
* to do this)


BSR_S		MACRO	;label
		IFNE	DEBUG_DETAIL
		bsr	\1
		ENDC
		IFEQ	DEBUG_DETAIL
		bsr.s	\1
		ENDC
		ENDM

BEQ_S		MACRO	;label
		IFNE	DEBUG_DETAIL
		beq	\1
		ENDC
		IFEQ	DEBUG_DETAIL
		beq.s	\1
		ENDC
		ENDM

BNE_S		MACRO	;label
		IFNE	DEBUG_DETAIL
		bne	\1
		ENDC
		IFEQ	DEBUG_DETAIL
		bne.s	\1
		ENDC
		ENDM



BRA_S		MACRO	;label
		IFNE	DEBUG_DETAIL
		bra	\1
		ENDC
		IFEQ	DEBUG_DETAIL
		bra.s	\1
		ENDC
		ENDM

BLS_S		MACRO	;label
		IFNE	DEBUG_DETAIL
		bls	\1
		ENDC
		IFEQ	DEBUG_DETAIL
		bls.s	\1
		ENDC
		ENDM

BLE_S		MACRO	;label
		IFNE	DEBUG_DETAIL
		ble	\1
		ENDC
		IFEQ	DEBUG_DETAIL
		ble.s	\1
		ENDC
		ENDM

BMI_S		MACRO	;label
		IFNE	DEBUG_DETAIL
		bmi	\1
		ENDC
		IFEQ	DEBUG_DETAIL
		bmi.s	\1
		ENDC
		ENDM

BHI_S		MACRO	;label
		IFNE	DEBUG_DETAIL
		bhi	\1
		ENDC
		IFEQ	DEBUG_DETAIL
		bhi.s	\1
		ENDC
		ENDM

BCS_S		MACRO	;label
		IFNE	DEBUG_DETAIL
		bcs	\1
		ENDC
		IFEQ	DEBUG_DETAIL
		bcs.s	\1
		ENDC
		ENDM

BCC_S		MACRO	;label
		IFNE	DEBUG_DETAIL
		bcc	\1
		ENDC
		IFEQ	DEBUG_DETAIL
		bcc.s	\1
		ENDC
		ENDM

**
** Kodiak debug stuff
**
DEBUG	EQU	0

D_LOCKIRPORT	EQU	0
D_LOCKDRPORT	EQU	1
D_UNLOCKRPORT	EQU	2
D_USURPLOCK	EQU	3
D_LOCKLAYER	EQU	4
D_UNLOCKLAYER	EQU	5

DINIT	MACRO
	IFNE	DEBUG
	IFND	DInit
	XREF	DInit
	ENDC
		bsr	DInit
	ENDC
	ENDM

DTRACE	MACRO
	IFNE	DEBUG
	IFND	DTrace
	XREF	DTrace
	ENDC
		bsr	DTrace
		dc.w	\1
	ENDC
	ENDM

D_TEXT	MACRO
\@D_TEXT	EQU	*
		dc.w	\@D_LOCKIRPORT-\@D_TEXT
		dc.w	\@D_LOCKDRPORT-\@D_TEXT
		dc.w	\@D_UNLOCKRPORT-\@D_TEXT
		dc.w	\@D_USURPLOCK-\@D_TEXT
		dc.w	\@D_LOCKLAYER-\@D_TEXT
		dc.w	\@D_UNLOCKLAYER-\@D_TEXT
\@D_LOCKIRPORT	dc.b	'LockIRPort',0
\@D_LOCKDRPORT	dc.b	'LockDRPort',0
\@D_UNLOCKRPORT dc.b	'UnLockRPort',0
\@D_USURPLOCK	dc.b	'(usurp lock)',0
\@D_LOCKLAYER	dc.b	'(locklayer)',0
\@D_UNLOCKLAYER	dc.b	'(unlocklayer)',0
	ENDM

