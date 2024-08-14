;	NAME	DIAG
;	TITLE	Subroutines
;***************************************************************************
;* (c)Copyright 1985 by Commodore Electronics Ltd.  This program contains  *
;* proprietary and confidential information.  All rights reserved except   *
;* as may be permitted by prior written consent.			   *
;***************************************************************************
;
;------------------------------------------------------------------------------
;
; Subroutine to loop back the last failed test. Dependent on diagnostic switch
; (Bit0 of system switch) system will halt or loop back after a failed test.
;
; Diagnostic switch ON	: Return to current loop back address
; Diagnostic switch OFF : Fall into TSTHLT macro
;
; Input :  Offset begin of last testroutine on stack
; Output:  AL = 0
;------------------------------------------------------------------------------
;
	ASSUME CS:CODE,DS:NOTHING,ES:NOTHING
;
	PUBLIC	DIAGNOSTIC
;
DIAGNOSTIC PROC NEAR
;					
	IN	AL,PPIB 	; program system port to read lower half
	AND	AL,NOT CASOFF	; of system switch
	OUT	PPIB,AL
	IN	AL,PPIC
	AND	AL,POST 	; check diagnostic switch (Bit0)
	JZ	DOLOOP
	TESTHLT 		; found switch off -> HALT
DOLOOP:
	RET
;
DIAGNOSTIC ENDP
	PRGSIZ	<Diagnostic branch routine>   ; Display program size
;------------------------------------------------------------------------------
