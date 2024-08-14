	NAME	TIMDSR
	TITLE	Timer DSR
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program	   *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent.			   *
;***************************************************************************
;
; ROM TIMER DEVICE & INTERRUPT SERVICE ROUTINES
;
;
; Harware Interrupt support is provided for the following;
;		Timer tick interrupt 08h
;		(AT) Alarm/Periodic interrupt 50h
;
;
; Timer Device Service Routine 1Ah supports the following;
;
;		00H - Read the clock
;		01H - Set the clock
;		02h - (AT) Read real-time clock
;		03h - (AT) Set  real-time clock
;		04h - (AT) Read date
;		05h - (AT) Set  date
;		06h - (AT) Set   alarm
;		07h - (AT) Reset alarm
;		80h - (JR) Set sound multiplexer (not implemented)
;
;      REVISED:	01/28/85 (IJP) AT code added for CMOS chip support
;		06/25/84 (IJP) Code size reduced, Timer set check corrected
;		06/25/84 (IJP) Speeded up code, sync macros
;		02/27/84 (DLK) Fixed cmd out-of-range return, Referenced FDTIMO
;			as byte, not word
;      WRITTEN:	10/19/83
;
;------------------------------------------------------------------------------
;
INCLUDE	ROMOPT.INC		; ROM Options file - must be included 1st
;
INCLUDE	ROMEQU.INC		; ROM General equate file
INCLUDE	MACROS.INC		; ROM Macros file
;
;------------------------------------------------------------------------------
;
	OUT1	<TIMDSR - ROM Timer DSR code>
;
; The following equates represent the # of clock ticks per 24 Hour period.  If
; the frequency of the timer chip changes either due to different
; initialization or a different source clock, these values must change
;
H24MSW	EQU	24			; 24 * 65k clock tics
H24LSW	EQU	176			;   + 176 = # of tics in 24 hours
;
; The time-base frequency of 32.768 kHz and Rate selection of 1.024 kHz,
; causes a period of 976.562 microseconds per interrupt. We use a close
; approximation even though there are roundoff errors
;
PERTIM	EQU	976			; Approximate # of microseconds per
					;   periodic RTC interrupt
;
;------------------------------------------------------------------------------
DGROUP	GROUP	ROMDAT
ROMDAT	SEGMENT AT 0040h	;The following are relative to Segment 40h
;
	EXTRN	FDMOTS:BYTE		; DISK MOTOR STATUS BYTE
	EXTRN	FDTIMO:BYTE		; DISK MOTOR TIMEOUT
	EXTRN	LOTIME:WORD		; LEAST SIGNIFICANT TIMER WORD
	EXTRN	HITIME:WORD		; MOST SIGNIFICANT TIMER WORD
	EXTRN	HOUR24:BYTE		; 24-HOUR ROLLOVER COUNTER
;
;
ROMDAT	ENDS
;------------------------------------------------------------------------------
;
CGROUP	GROUP	CODE
CODE	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
	EXTRN	SETDS40:NEAR
;------------------------------------------------------------------------------
; Timer Device Service Routine - INT 1Ah
;
;	AH = Function code (00h - 01h) for non-AT
;			   (00h - 07h) for AT
;------------------------------------------------------------------------------
	SYNC	TIMDSRA
;
	PUBLIC	TIMDSR
TIMDSR	PROC	FAR
;
	PUSH	DS
	CALL	SETDS40			; Set DS to the ROM data segment
	ASSUME	DS:ROMDAT
	CMP	AH,1			; Set clock ?
	JC	CLKREAD			;   No, it's a read clock
	JNZ	SETCL1			;   No, it's invalid, then return
;
;------------------------------------------------------------------------------
; SET CLOCK (AH=01h)
;
;	Input:	DS = ROM data segment (0040h)
;		CX = High word of Clock Count
;		DX = Low word of Clock Count
;	Output:	AH = 0
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
CLKSET:
	MOV	LOTIME,DX		; Store low word of current time
	MOV	HITIME,CX		; Store high word of current time
	XOR	AH,AH			; Clear AH indicating
					;   operation complete
	MOV	HOUR24,AH		; And clear 24-Hour rollover flag
SETCL1:
;
	POP	DS
	ASSUME	DS:NOTHING
	IRET
;------------------------------------------------------------------------------
; READ CLOCK (AH=00h)
;
;	Input:	DS = ROM data segment (0040h)
;	Output:	AL = 24-Hour Rollover flag
;		CX = High word of Clock Count
;		DX = Low word of Clock Count
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
CLKREAD:
	MOV	DX,LOTIME		; GET LOW WORD OF CURRENT TIME
	MOV	CX,HITIME		; GET HIGH WORD OF CURRENT TIME
	XOR	AX,AX			; Clear AL and restore AH to 0
	XCHG	AL,HOUR24		;   and exchange it with 24-Hour
					;   rollover flag
	POP	DS
	ASSUME	DS:NOTHING
	IRET
;
TIMDSR	ENDP
	PRGSIZ	<Timer DSR>		; Display program size
;------------------------------------------------------------------------------
	IF	ROMSIZ LE (8*1024)
	PNXMAC5				; Include HEXMSG, CRLFMSG, COMXITA,
					;   COMEXIT
	ENDIF
;------------------------------------------------------------------------------
; TIMER INTERRUPT SERVICE ROUTINE
;
;	Input: None
;	Output:	None
;------------------------------------------------------------------------------
	SYNC	TIMISRA
;
	PUBLIC	TIMISR
TIMISR	PROC	FAR
	ASSUME	DS:NOTHING,ES:NOTHING
	PUSH	AX			; Save at least AX, DX, DS because
	PUSH	DX			;   the user timer routine can
	PUSH	DS			;   destroy them
;
	CALL	SETDS40			; Set DS to the ROM data segment
	ASSUME	DS:ROMDAT
;
	MOV	AX,LOTIME
	MOV	DX,HITIME
;
	INC	AX			; Increment Least significant time count
					; Did Count overflow ?
	JNZ	TIMIS0			;   No
;
	INC	DX			; Increment most significant time count
TIMIS0:
	CMP	DX,H24MSW		; Hit 24-Hour period ?
	JA	TIMIS1			; Its already over, must have been bad
					;   load of memory location
	JC	TIMIS2			;   No, finish up
	CMP	AX,H24LSW		; Hit 24-Hour period ?
	JC	TIMIS2			;   No, finish up
TIMIS1:
	INC	HOUR24			;   Yes, increment 24-Hour counter
	XOR	AX,AX			; Clear seconds of day time values
	XOR	DX,DX			;
TIMIS2:
	MOV	LOTIME,AX
	MOV	HITIME,DX
;
	DEC	FDTIMO			; Decrement Disk Timeout time
	JNZ	TIMIS3			;   It didn't time out
	MOV	DX,FDCDOR		;   It did time out
	MOV	AL,0CH
	OUT	DX,AL			; Turn off all diskette motors
					;   (a side effect selects drive A)
	AND	FDMOTS,0F0H		; Reset all motor on bits (0-3)
TIMIS3:
;
	INT	1CH			; Call User timer tick routine (if any)
;
	MOV	AL,EOINSP		; End of Interrupt bit
	OUT	PIC0,AL			; Acknowledge 8259 interrupt
;
	POP	DS			; The user interrupt 1Ch can trash the
	ASSUME	DS:NOTHING
	POP	DX			;   registers DS, DX, AX, and flags,
	POP	AX			;   but must preserve all others.  This
	IRET				;   maintains compatability.
;
TIMISR	ENDP
	PRGSIZ	<Timer ISR>		; Display program size
;
	IF	ROMSIZ LE (8*1024)
	PNXMAC4				; Include DS40H, BOOTCODE,
	ENDIF
;
;
;
	SYNC	VECTBLA
;
	TOTAL
;
CODE	ENDS
	END
