	NAME	FDBOOT
	TITLE	Floppy Disk boot and BASIC boot DSRs
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program    *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent. 		   *
;***************************************************************************
;
; Floppy Disk Boot DSR and Cassette BASIC entry (or monitor etc.)
;
;	WRITTEN: 04/17/85 Ira J. Perlow
;	REVISED: 11/18/85 DP Dieter Preiss /Commodore - allow boot
;		 also from Drive B:
;------------------------------------------------------------------------------
;
INCLUDE ROMOPT.INC		; ROM Options file - must be included 1st
;
INCLUDE ROMEQU.INC		; ROM General equate file
INCLUDE MACROS.INC		; ROM Macros file
;
;------------------------------------------------------------------------------
;
	OUT1	<FDBOOT - Floppy Disk boot and BASIC boot DSRs>
;
;------------------------------------------------------------------------------
;
CGROUP	GROUP	CODE
CODE	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
;
	EXTRN	BOOTCODE:DWORD
	EXTRN	WRTCRLF:NEAR
	EXTRN	GETKEY:NEAR
	EXTRN	BTMSG0:BYTE
;------------------------------------------------------------------------------
;		       INT 19H
;
;	***	B O O T   D I S K E T T E      ***
;------------------------------------------------------------------------------
	SYNC	FDBOOTA 		;INT 19H service (1) 
;
	PUBLIC	FDBOOT			; INT 19H entry
FDBOOT	PROC	FAR
	ASSUME	DS:NOTHING,ES:NOTHING
	SUB	DX,DX			; Head=0, Drive = A:
BOOTNXT:
	MOV	CX,RETRY
BOOTDK0:
	PUSH	CX
	STI				; enable interrupts
	XOR	AH,AH			; Reset diskette
	INT	13h			; ....
	LES	BX,CS:BOOTCODE		; read boot code here
	ASSUME	ES:NOTHING
	MOV	CX,0001H		; CH = Track 0, CL = sector 1
	MOV	AX,0201H		; AH = read command, AL = read 1 sector
	INT	13h			; do the read
	POP	CX			; count restore
	JC	BOOTDK1 		; failed start again
	JMP	DWORD PTR CS:BOOTCODE	; execute boot code
BOOTDK1:
	LOOP	BOOTDK0
	INC	DL			; Next Drive
	CMP	DL,2			; All drives done ?
	JB	BOOTNXT 		; no if below
;
	INT	18H			; Call INT 18h but don't return
;
	IRET

	PRGSIZ	<Boot disk code INT 19h> ; Display program size
; Fall through to user routine INT 18h, which might be a monitor
; or a ROM BASIC, an error message etc.
; Should INT 18h return, which is unlikely, we'll return to caller of INT 19h
;	
;
;------------------------------------------------------------------------------
;		       INT 18H
;
; Not able to boot diskette, go to Monitor, error message
;	or a ROM BASIC
;------------------------------------------------------------------------------
	PUBLIC	FDBTER			; INT 18h entry
FDBTER:
	ASSUME	DS:NOTHING,ES:NOTHING
	CALL	WRTCRLF
	MOV	SI,OFFSET BTMSG0
	CALL	GETKEY			; Get key in uppercase
;
	IRET
;
FDBOOT	ENDP
	PRGSIZ	<Error in booting disk code INT 18h>	; Display program size
;------------------------------------------------------------------------------
;
	SYNC	BAUDTBA 		;Baud table
	TOTAL
;
CODE	ENDS
	END	
