**********************************************************************
*
*   Commodore Amiga -- ROM Operating System Executive Include File
*
**********************************************************************
*
*   Source Control:
*
*	$Id: calls.i,v 39.1 92/02/19 15:45:38 mks Exp $
*
**********************************************************************

;JSR via vector table to another member of this library
JSRSELF     MACRO
	    JSR     _LVO\1(A6)
	    ENDM

;JMP via vector table to another member of this library
JMPSELF     MACRO
	    JMP     _LVO\1(A6)
	    ENDM

;Cheat-BSR to an external function of this library (bypass vectors)
BSRSELF     MACRO
	    BSR     \1
	    ENDM

;Cheat-BRA to an external function of this library (bypass vectors)
BRASELF     MACRO
	    BRA     \1
	    ENDM

JMPTRAP     MACRO   ; offset	(A5 destroyed)
	    LEA     \1,A5
	    JMP     _LVOSupervisor(A6)
	    ENDM

JSRTRAP     MACRO   ; offset	(A5 destroyed)
	    LEA     \1,A5
	    JSR     _LVOSupervisor(A6)
	    ENDM

;Version of PERMIT that implies an RTS, and ExecBase in A6
JMP_PERMIT	MACRO   * ExecBase in A6!
		XREF	Permit
		BRA	Permit
		ENDM

;Version of PERMIT that implies ExecBase in A6
BSR_PERMIT	MACRO   * ExecBase in A6!
		XREF	Permit
		BSR	Permit
		ENDM

MYALERT		MACRO	; alertNumber
		move.l d7,-(sp)
		move.l	#\1,d7
		XREF	_LVOAlert
		jsr	_LVOAlert(a6)
		move.l	(sp)+,d7
		ENDM
