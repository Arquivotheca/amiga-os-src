	NAME	PRTDSR
	TITLE	Printer DSR
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program    *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent. 		   *
;***************************************************************************
;
;	WRITTEN: 10/20/83
;	REVISED: mm/dd/yy ** (initials)
;		 02/28/84 *A (DLK)
;		  - FIX BAD CMD EXIT
;		 04/11/84 *B (DLK)
;		  - INCREASE PRINTER INITIALIZATION DELAY
;		 04/17/84 *C (DLK)
;		  - RESET PRINTER INITIALIZATION BIT
;	CODE REDUCTION: mm/dd/yy *n (initials)
;		 04/05/84 *4 (DLK)
;		  - FOURTH PASS
;		 06/21/84 *D (IJP)
;		  - Improved speed, used less bytes
;			and corrected I/O addresses.
;		  - Also improved equate structures and names
;		  - Cleaned up threaded code, increased checks
;
; Rom Parallel Printer Device Service Routine
;	This module contains the device service routine
;		for the parallel printer interface.
;	Access to the Parallel Printer DSR is via S/W Interrupt 17h.
;
;
;	UPDATED: 07-30-86 by Torsten Burgdorff (BSW)
;		 - new strobe handling to support
;		   janus interface
;
;
;******************************************************************************
;	The functions, passed in AH, are as follows;
;
;		00h - Send character in AL to printer DX,
;			return printer status in AH
;		01h - Initialize printer DX, return printer status in AH
;		02h - Return printer DX's status in AH
;
;	Notes:	All Registers are preserved except AH for all functions
;******************************************************************************
;			Printer Status bits returned in AH
;
;	-------------------------------
; bit | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
;      -------------------------------
;	|   |	|   |	|   |	|   |
;	|   |	|   |	|   |	|   +--> Printer timeout
;	|   |	|   |	|   |	+------> not used
;	|   |	|   |	|   +----------> not used
;	|   |	|   |	+--------------> I/O Error	(Pin 15 Inverted)
;	|   |	|   +------------------> Selected	(Pin 13)
;	|   |	+----------------------> Out of Paper	(Pin 12)
;	|   +--------------------------> Acknowledge	(Pin 10 Inverted)
;	+------------------------------> Not Busy	(Pin 11 Inverted)
;
;	Notes:	Pins #s are those on a 25 pin D connector
;		Bit 7, the Not Busy flag is opposite of documentation
;			which indicates it as a Busy flag.  This was
;			found through tracing and testing to be this way
;------------------------------------------------------------------------------
;
INCLUDE ROMOPT.INC		; ROM Options file - must be included 1st
;
INCLUDE ROMEQU.INC		; ROM General equate file
INCLUDE MACROS.INC		; ROM Macros file
;
;------------------------------------------------------------------------------
;
	OUT1	<PRTDSR - ROM Printer DSR code>
;
;	The following bit assignments are for the printer parallel port
; assigned at the printer base port + 01h.  The port allows only input.
;
NOTBSY	EQU	80h	;Printer not busy		(I/O pin # 11)
NOTACK	EQU	40h	;Print not acknowledged 	(I/O pin # 10)
PPROUT	EQU	20h	;Printer out of paper		(I/O pin # 12)
SELECT	EQU	10h	;Printer selected		(I/O pin # 13)
NOTERR	EQU	08h	;Not Printer I/O error		(I/O pin # 15)
;
;	The following bit assignments are for the printer parallel port assigned
; at the printer base port + 02h.  The default value after a reset is XXX01011b.
;
IRQEN	EQU	10h	;IRQ enable
SELINP	EQU	08h	;Select Input			(I/O pin # 17 Inverted)
PRTINO	EQU	04h	;Printer Initializing off	(I/O pin # 16	      )
AUTOFD	EQU	02h	;Auto Feed			(I/O pin # 14 Inverted)
PRTSTB	EQU	01h	;Printer Strobe 		(I/O pin # 1  Inverted)
;
;   Other Equates
;
TIMOUT	EQU	01h	;Printer timeout
;------------------------------------------------------------------------------
;
DGROUP	GROUP	ROMDAT
ROMDAT	SEGMENT AT 0040h	;The following are relative to Segment 40h
	EXTRN	LPADRTBL:WORD		; Printer Address Table
	EXTRN	LPTOTBL:BYTE		; Printer Timeout Table
ROMDAT	ENDS
;
;------------------------------------------------------------------------------
;
CGROUP	GROUP	CODE
CODE	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
;
	EXTRN	SAVREGS:NEAR		; Save registers
	EXTRN	COMEXIT:NEAR		; Common DSR Exit routine
	EXTRN	RNGCHK:NEAR		; Check function range
	extrn	waitbusy:near		; wait_for_busy signal
;
;------------------------------------------------------------------------------
; PRINTER ENTRY POINT VIA S/W INT 17H
;
;	AH = Function (00H - 02H)
;------------------------------------------------------------------------------
	SYNC	PRTDSRA
;
	PUBLIC	PRTDSR
PRTDSR	PROC	FAR
	STI
	CALL	SAVREGS 		; Save caller's Registers
;
	CMP	DX,MAXPRT		; Check if index <= highest printer #
	JNC	EXIT			; Jump on device # error
;
	MOV	DI,OFFSET XFEREND-2	; DI=Length of CMD Jump table
					;  (OFFSET needed so forward reference
					;  doesn't produce a NOP)
	CALL	RNGCHK			; Go check function range
	ASSUME	DS:ROMDAT
	JC	EXIT			; Jump on range error
;
; DI now contains index into function table
;
	MOV	BX,DX			; BX=Device # Index in bytes
;
; In case we need it, get timeout byte
;
	MOV	CL,[LPTOTBL+BX] 	; Get printer timeout
;
	SHL	BX,1			; Convert index to a word index
;
	MOV	DX,[LPADRTBL+BX]	; Get Base I/O Addr of device	*9
	OR	DX,DX			; Check if entry is zero
	JZ	EXIT			;   and if so then exit
;
; The following code is common to all routines
	INC	DX			; DX=I/O Addr of status port
	XOR	CH,CH			; Clear CH for timeout bit storage
;
	JMP	WORD PTR CS:[DI+OFFSET CMDXFER]  ; Jump to function processor
;
CMDXFER DW	OFFSET PRTCHR		; Print character to printer
	DW	OFFSET PRTINIT		; Initialize Printer
	DW	OFFSET PRTST		; Return Printer Status
XFEREND EQU	$-CMDXFER
;
;------------------------------------------------------------------------------
; PRINT CHARACTER (AH=00H)
;
;	INPUT:	AL = Character to output
;		DX = Index to Printer table port + 1 (the Status port)
;		CX = Timeout value
;	OUTPUT: AH = Printer Status
;------------------------------------------------------------------------------
PRTCHR:
	MOV	AH,AL			; Save character to print in AH
;
;
PRTCH0A:
	XOR	BX,BX			; Use BX as inner loop timer
;
PRTCH0:
	IN	AL,DX			; Read printer status
; Check for error bit(s) and loop until timeout if so
	AND	AL,NOTBSY		; +PPROUT+SELECT+NOTERR
 	XOR	AL,NOTBSY		; +SELECT+NOTERR
	JZ	PRTCH1			; Device is ready, so send character
;
	DEC	BX			; Count BX back to zero
;
	JNZ	PRTCH0			;   and loop if not done
	LOOP	PRTCH0A 		; Loop for CX # of times
;
	INC	CH			; Indicate timeout error by setting
					;  CH = 1 (Note CX = 0 after LOOP
					;  instruction)
	JMP	SHORT PRTCH2		;   and finish up with status routine
;
PRTCH1:
	DEC	DX			; Point DX to I/O addr of data port
	MOV	AL,AH			;  * Yes, OK to print
	OUT	DX,AL			; Put character on interface
;
	INC	DX			; DX=I/O Addr of status port
	INC	DX			; DX=I/O Addr of control port
	MOV	AL,SELINP+PRTINO+PRTSTB ; Strobe data to printer
;
; Note:  the time between this OUT instruction and the next must be > 1 us
; for at least one printer examined.  The code takes 9 clocks on a 286 and
; 14 clocks on anything else.
	IF	CLK10 GT 10*14
	IF	I80286
;
	IF	CLK10 GT 10*45
; **** ERROR: Not enough delay in printer strobe, must be > 1 microsecond ****
	ELSE
	PUSHA				; Extra 36 clocks on a 286
	POPA
	ENDIF
;
	ELSE
	IF	CLK10 GT 10*33
; **** ERROR: Not enough delay in printer strobe, must be > 1 microsecond ****
	ELSE
	PUSH	AX			; Extra 19 clocks on anything else
	POP	AX
	ENDIF
;
	ENDIF		; I80286
	ENDIF
;
; new code - sets strobe line and resets it when busy line becomes activ
	call	waitbusy		; not enaugh space here
					; routine in MISCL2.INC
	DEC	DX			; Point DX to I/O addr of status port
PRTCH2:
	MOV	AL,AH			; Restore the character to AL
					;   and fall through to status routine
;------------------------------------------------------------------------------
; RETURN PRINTER STATUS (AH=02H)
;
;	Input:	DX = Index to Printer table port + 1 (the Status port)
;		CH must have correct value of timeout flag
;	Output: AH = Printer Status
;
;	Notes:	This routine located here so that the character print
;			routine can fall through to it
;		To save bytes, there are 2 alternate entry points
;			called from the other functions
;------------------------------------------------------------------------------
PRTST:
	XCHG	AX,BX			; Save AL
;
; Alternate entry point, CH must have correct value of timeout flag
;	BL must contain original contents of AL
PRTST1:
	IN	AL,DX			; Read Printer Status
					; Clear bits 0-2
	AND	AL,NOTBSY+NOTACK+PPROUT+SELECT+NOTERR
	XOR	AL,NOTACK+NOTERR	; Invert acknowledge and I/O error bits
	MOV	BH,AL			;   and place Status in BH
	OR	BH,CH			; Include timeout bit if set
	XCHG	AX,BX			; Restore AL and AH and fall thru to
					;   exit
;
;  All functions for the printer eventually exit here
EXIT:
	JMP	COMEXIT 		; Go to common DSR exit
;------------------------------------------------------------------------------
; INITIALIZE PRINTER (AH=01H)
;
;	Input:	DX = Printer Status Port address = (Printer Table contents +1)
;	Output: AH = Printer Status
;------------------------------------------------------------------------------
PRTINIT:
	MOV	BL,AL			; Save AL
;
	INC	DX			; DX=I/O Address of control port
	MOV	AL,SELINP		; Set Printer initialization signal
					;   while maintaining IRQ off,
					;   select input on, auto feed off
					;   and printer strobe off
	OUT	DX,AL			; Output init signal to Printer
;
	MOV	CX,376*CLK10/17 	; Delay > 376uS
PRINI0:
	LOOP	PRINI0			; Side effect of loop leaves CH = 0
					;   which will indicate no timeout
					;   error in status routine
;
	XOR	AL,PRTINO		; Reset Printer Initialization signal
	OUT	DX,AL			;   while maintaining the other signals
;
	DEC	DX			; Point DX to I/O addr of status port
	JMP	SHORT PRTST1		;   and finish up with status routine
;------------------------------------------------------------------------------
PRTDSR	ENDP
	PRGSIZ	<Printer DSR>		; Display program size
;
	SYNC	CRTVECA
;
	TOTAL
;
CODE	ENDS
	END
