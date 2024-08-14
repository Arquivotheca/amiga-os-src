	NAME	POWRUP
	TITLE	POWERUP LOGIC
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program    *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent. 		   *
;***************************************************************************
;
; POWERUP VECTOR INITIALIZATION LOGIC
;
;	This module contains a long JMP to initialization code.  It is branched
;	to by the CPU power-up reset, Keyboard ISR when CTRL-ALT-DEL is
;	pressed, and by NMI and Interrupt handlers, as well as some application
;	programs
;
;	WRITTEN: 11/21/83
;	Revised: 06/  /84 Ira J. Perlow
;
;------------------------------------------------------------------------------
;
INCLUDE ROMOPT.INC			; ROM Options file - must be included 1st
INCLUDE ROMEQU.INC			; ROM General equate file
INCLUDE MACROS.INC			; ROM Macros file
;
;------------------------------------------------------------------------------
;
	OUT1	<POWRUP - ROM Powerup jump and date code>
;
CGROUP	GROUP	CODE
CODE	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
;
	EXTRN	RESET:NEAR		; BEGINNING INITIALIZATION CODE
;------------------------------------------------------------------------------
; POWERUP ENTRY POINT
;------------------------------------------------------------------------------
	SYNC	POWRUPA
	PUBLIC	POWRUP
	PUBLIC	ROMSEG
POWRUP	LABEL	NEAR
;	JMP	FAR PTR RESET		; Branch to initialization code
	DB	0EAH			; (hard-coded long jump)
	DW	OFFSET RESET
ROMSEG	DW	0F000H			; The ROM's segment
	PRGSIZ	<Power up entry point>	; Display program size
;------------------------------------------------------------------------------
;	Default Version date
;------------------------------------------------------------------------------
	SYNC	VERDATA
	PUBLIC	VERDAT
VERDAT	DB	"11/08/85"
	PRGSIZ	<Date in ASCII> 	; Display program size
;------------------------------------------------------------------------------
;
;------------------------------------------------------------------------------
;	DB	0			; Unused location, reserved
;------------------------------------------------------------------------------
; Computer Type byte
;------------------------------------------------------------------------------
	SYNC	COMTYPA
	PUBLIC	COMTYP
COMTYP	LABEL	BYTE
	IF	PC
	    DB	    0FFh
	    PRGSIZ  <Computer type byte 0FFh = PC> ; Display program size
	ENDIF
	IF	XT
	    DB	    0FEh
	    PRGSIZ  <Computer type byte 0FEh = XT> ; Display program size
	ENDIF
;------------------------------------------------------------------------------
; Location of checksum byte
;------------------------------------------------------------------------------
	SYNC	CHKSUMA
	DB	0			; Checksum byte location, set initially
					;   to 0
	PRGSIZ	<Checksum byte> 	; Display program size
;------------------------------------------------------------------------------
;	SYNC	ROMENDA 	; Note that this is one less due to a bug
					;   in the Sync macro with values over 64k
	TOTAL
CODE	ENDS
	END

OMENDA 	; Note that this is one less due to a bug
					;   in the Sync macro with values over 64k
	TOTAL
CODE	ENDS
	END
