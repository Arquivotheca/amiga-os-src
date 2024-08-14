
**********************************************************************
*
*   Commodore Amiga -- ROM Operating System Executive Include File
*
**********************************************************************
*
*   Source Control:
*
*	$Id: calls.i,v 1.0 91/02/22 14:19:06 peter Exp $
*
*	$Locker:  $
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
JMP_PERMIT  MACRO   * ExecBase in A6!
	    JMP   _LVOPermit(A6)
	    ENDM
