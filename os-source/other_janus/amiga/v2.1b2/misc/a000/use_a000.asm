CSEG	SEGMENT
	ASSUME DS:CSEG, SS:CSEG, CS:CSEG, ES:CSEG

;**********************************************************************
;*        Janus Handler (ROM Extension) Initialization Trigger        *
;*                                                                    *
;*       Fully Public Domain by Jeff Rush - September 7th, 1989       *
;*                                                                    *
;*       BIX: jrush         BBS: (214) 231-1372 Rising Star Opus      *
;**********************************************************************
	ORG	100H
BEGIN:
	MOV	AX,0A000H
	MOV	ES,AX		; Pickup Segment of Expected ROM Extension

	CMP	WORD PTR ES:[0],0AA55H
	JNE	NOT_HERE

	CALL	CS:FARPTR		; Invoke ROM via Far Call to Init Entry

	LEA	SI,ERR_MSG
	CALL	PRTSTR		; Print Success Message

	MOV	AX,4C00H
	INT	21H		; Terminate with an Exit Code of 0 (ok)
NOT_HERE:
	LEA	SI,ERR_MSG
	CALL	PRTSTR		; Print Error Message

	MOV	AX,4C01H
	INT	21H		; Terminate with an Exit Code of 1 (error)

FARPTR	DD	0A0000003H

OK_MSG:	DB	'Janus Rom Extension Initialized.',0DH,0AH,0
ERR_MSG:	DB	'No Rom Extension Found at Segment A000!',0DH,0AH,0

;**********************************************************************
;*                           Subroutine                               *
;*           Print Zero-Terminated String Pointed To By SI            *
;**********************************************************************
PRTSTR	PROC	NEAR
	PUSH	AX
PRTSTR_LOOP:
	LODSB
	OR	AL,AL
	JZ	PRTSTR_EXIT
	MOV	AH,0EH

	PUSH	SI
	INT	10H
	POP	SI

	JMP	PRTSTR_LOOP

PRTSTR_EXIT:
	POP	AX
	RET
PRTSTR	ENDP

CSEG	ENDS
;**********************************************************************
;*                                                                    *
;**********************************************************************
	END	BEGIN

