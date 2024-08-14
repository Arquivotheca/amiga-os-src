	NAME	MISDSR
	TITLE	Equipment check and Memory Size DSRs
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program	   *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent.			   *
;***************************************************************************
;
; MISCELLANEOUS SYSTEM ROM INTERRUPT PROCESSOR
;	THIS MODULE CONTAINS VARIOUS S/W INTERRUPT PROCESSORS
;	TO HANDLE USER REQUESTS; ROUTINES INCLUDED ARE:
;
;		INT	DSR
;		---	---
;		11H	EQUIPMENT CHECK
;		12H	MEMORY SIZE
;
;	WRITTEN: 04/17/85 Ira J. Perlow
;	REVISED: mm/dd/yy ** (initials)
;
;------------------------------------------------------------------------------
;
INCLUDE	ROMOPT.INC		; ROM Options file - must be included 1st
INCLUDE	ROMEQU.INC		; ROM General equate file
INCLUDE	MACROS.INC		; ROM Macros file
;
;------------------------------------------------------------------------------
;
	OUT1	<MISDSR - Equipment check and Memory Size DSRs>
;
DGROUP	GROUP	ROMDAT
ROMDAT	SEGMENT AT 0040h	;The following are relative to Segment 40h
	EXTRN	DEVFLG:WORD		; DEVICES INSTALLED FLAG
	EXTRN	MEMSIZE:WORD		; AVAILABLE MEMORY
ROMDAT	ENDS
;
;------------------------------------------------------------------------------
;
CGROUP	GROUP	CODE
CODE	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
;
;------------------------------------------------------------------------------
; MEMORY SIZE CHECK VIA S/W INT 12H
;
;	Output:	AX = Memory size
;------------------------------------------------------------------------------
	SYNC	MEMCHKA
;
	PUBLIC	MEMCHK
MEMCHK	PROC	FAR
	ASSUME	DS:NOTHING,ES:NOTHING
	PUSH	DS
	CALL	SETDS40			; Set DS to the ROM data segment
	ASSUME	DS:ROMDAT
	MOV	AX,MEMSIZE		; RETURN MEMORY SIZE
	POP	DS
	ASSUME	DS:NOTHING
	IRET
MEMCHK	ENDP
	PRGSIZ	<Memory check>		; Display program size
;------------------------------------------------------------------------------
; EQUIPMENT CHECK VIA S/W INT 11H
;
;	Output:	AX = Equipment Flags
;------------------------------------------------------------------------------
	SYNC	EQPMTCA
;
	PUBLIC	EQPMTC
EQPMTC	PROC	FAR
	ASSUME	DS:NOTHING,ES:NOTHING
	PUSH	DS
	CALL	SETDS40			; Set DS to the ROM data segment
	ASSUME	DS:ROMDAT
 	MOV	AX,DEVFLG		; RETURN DEVICE FLAGS
	POP	DS
	ASSUME	DS:NOTHING
	IRET
EQPMTC	ENDP
	PRGSIZ	<Equipment check>		; Display program size
;------------------------------------------------------------------------------
; CASSETTE INT 15H DSR
;
;	Output:	AH = 86h, Error code on XT or PC2 or AT if not valid function
;		Carry set if error
;		Interrupts on - Note: XT leaves interrupts off
;------------------------------------------------------------------------------
	SYNC	CASDSRA
;
	PUBLIC	CASDSR
CASDSR	PROC	FAR
	ASSUME	DS:NOTHING,ES:NOTHING
;
; NOTE: On a PC2 or XT the interrupts are not enabled on exit.
;	On a PC1 the interrupts are enabled on exit
;	On an AT the interrupts are enabled on exit
;
	STI				; Enable interrupts
	MOV	AH,86h			; Set error code to mean Error,
					;   Data Transmission Lost and No data
					;   found
	STC				; Set carry to mean an error
	RET	2			; Return
CASDSR	ENDP
	PRGSIZ	<Cassette DSR to return error>	; Display program size
;------------------------------------------------------------------------------
	EXTRN	DS40H:WORD
	EXTRN	HEXMSG:BYTE
	EXTRN	CRLFMSG:BYTE
	EXTRN	SETERF:NEAR
	OUT1	<.		Including MISCL1.INC>
INCLUDE	MISCL1.INC
	SYNC	ROMFNTA
;------------------------------------------------------------------------------
;
	TOTAL
;
CODE	ENDS
	END
