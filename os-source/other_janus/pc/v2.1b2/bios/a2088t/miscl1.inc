;	NAME	MISCL1
;	TITLE	subroutines
;***************************************************************************
;* Copyright (C) 1985 by Commodore Business Machines Inc.  This program    *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent. 		   *
;***************************************************************************
;
;------------------------------------------------------------------------------
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
;------------------------------------------------------------------------------
	EXTRN	PRTCHR:NEAR
	EXTRN	WRTCRLF:NEAR
	EXTRN	WRTOUT:NEAR
	EXTRN	SPEAKR:NEAR
;***********************************************
;
; BEEP	    ( ENTRY )
;
;******************************************
	PUBLIC	BEEP

BEEP	PROC	NEAR
	PUSH	AX
	PUSH	CX
	PUSH	DX
	MOV	CX,880			; 610 Hz
	MOV	DX,100			; 100 ms
	CALL	SPEAKR
	POP	DX
	POP	CX
	POP	AX
BEEP	ENDP

	PRGSIZ	<Beep routine>

;------------------------------------------------------------------------------
; disable/ enable keyboard interrupt
;	Input:	None
;	Output:
;------------------------------------------------------------------------------
	PUBLIC	KYINON
	PUBLIC	KYINOF
KYINON	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	MOV	AH,00h			; OWC1, enable hardware interrupts for
					;   Keyboard ISR	(bit 1 = 0)
	JMP	SHORT KYINO0
KYINOF:
	MOV	AH,02h
;
KYINO0:
	PUSHF
	CLI
	IN	AL,PIC0MSK
	AND	AL,NOT (02h)
	OR	AL,AH			; OWC1, enable hardware interrupts for
					;   and disable keyboard
					; Keyboard ISR	(bit 1 = 0)
	OUT	PIC0MSK,AL
	POPF
	RET
	PRGSIZ	<Keyboard interrupts enable/disable>
KYINON	ENDP
;
;------------------------------------------------------------------------------
; NMI Interrupt Enable, Disable
;------------------------------------------------------------------------------
	PUBLIC	NMIENB
	PUBLIC	NMIDSB
NMIENB	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
       IF	NMIENAB
	MOV	AL,NMION		; Turn NMI on
	JMP	SHORT NMIEN0
       ENDIF
NMIDSB:
	MOV	AL,NMIOFF		; Turn NMI off, NMIOFF
NMIEN0:
	OUT	NMIMSK,AL
	RET
	PRGSIZ	<NMI interrupts enable/disable>
NMIENB	ENDP

;------------------------------------------------------------------------------
; Checksum BIOS ROM routine
;
; Input:	CX = Pointer to segment paragraph start
;------------------------------------------------------------------------------
	PUBLIC	CHKSM8			; Checksum an 8k area of memory
	PUBLIC	CHKSUM			; Checksum an area of memory
CHKSM8	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	MOV	DS,CX			; Move start pointer into DS
	ASSUME	DS:NOTHING
	MOV	CH,HIGH ROMSIZ		; Checksum BIOS ROM
;------------------------------------------------------------------------------
; Checksum routine
;
; Input:	DS = Pointer to segment paragraph start
;		CX = # of bytes to count
; Output:	SI = CX
;		AL = Checksum
;		Zero flag = 1 if checksum = 0, else not
;		CX = 0
;------------------------------------------------------------------------------
CHKSUM:
	XOR	AL,AL			; start checksum
	XOR	SI,SI
CHKSM0:
	ADD	AL,[SI] 		; checksum it
	INC	SI
	LOOP	CHKSM0
	OR	AL,AL			; Set zero flag if OK
	RET
CHKSM8	ENDP
	PRGSIZ	<Checksum routine>	; Display program size

;------------------------------------------------------------------------------
; SUBROUTINE TO SAVE CALLER'S REGISTER SET
;------------------------------------------------------------------------------
	PUBLIC	SAVREGS 		; SAVE REGISTERS
SAVREGS PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	STI				; BEGIN BY OPENING INTS
	PUSH	CX
	PUSH	DX
	PUSH	SI
	PUSH	DI
	PUSH	BP
	PUSH	DS
	PUSH	ES
	MOV	BP,SP
	PUSH	[BP+14] 		; PUSH RETURN ADDR ONTO STACK
	MOV	[BP+14],BX		; PUT BX WHERE RTN ADDR WAS
	MOV	BP,[BP+4]		; Restore BP to original value
	RET				; DONE
SAVREGS ENDP
	PRGSIZ	<Save registers>	; Display program size

;------------------------------------------------------------------------------
; Clear memory in 1k chunks
;
;	Input:	BP = Paragraph starting point
;		SI = Paragraph stopping point
;		Direction Flag = 0
;	Output: AX = 0
;------------------------------------------------------------------------------
	PUBLIC	CLRMEM
CLRMEM	PROC	NEAR
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
	PUSH	CX
	PUSH	DI
	PUSH	ES
	PUSH	BP
	XOR	AX,AX			; Set value to clear memory to
CLRME0:
	MOV	ES,BP			; ES = paragraph #
	ASSUME	ES:NOTHING
	XOR	DI,DI			; Start at beginning of segment
	MOV	CX,0200h		; Clear for 512 words (1024 bytes)
	REP	STOSW			; Clear it
	ADD	BP,0040h
	CMP	BP,SI			; Is it past end ?
	JC	CLRME0			;   No, continue clearing
	POP	BP
	POP	ES
	ASSUME	ES:NOTHING
	POP	DI
	POP	CX
	RET
CLRMEM	ENDP
	PRGSIZ	<Clear memory by 1k subroutine> ; Display program size
;
;------------------------------------------------------------------------------
; SUBROUTINE TO CHECK OPCODE RANGE ON DSR ENTRY
;
;   INPUT:	DI = LENGTH OF OPCODE BRANCH TABLE				*A
;   OUTPUT:	DS = Data Segment for ROM data areas
;		Carry flag set if error
;		DI = Index to opcode branch table
;------------------------------------------------------------------------------
	PUBLIC	RNGCHK			; DSR OPCODE RANGE CHECK
RNGCHK	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	PUSH	AX
	MOV	AL,AH
	XOR	AH,AH
	SHL	AX,1
	CMP	DI,AX			; CMD IN RANGE ?
	JC	RNG5			;   No, return with carry set
					;   Yes, carry flag not set for no error
	MOV	DI,AX
RNG5:
	POP	AX
;	JMP	NEAR PTR SETDS40	; Set DS to the ROM data segment
					;   and return through subroutine
	ASSUME	DS:ROMDAT
;
RNGCHK	ENDP
	PRGSIZ	<Range check>		; Display program size

;------------------------------------------------------------------------------
; Set DS to segment 0040h
;------------------------------------------------------------------------------
	PUBLIC	SETDS40
SETDS40 PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	MOV	DS,WORD PTR CS:DS40H
	ASSUME	DS:ROMDAT
	RET
SETDS40 ENDP
	PRGSIZ	<Set DS to segment 0040h> ; Display program size

;------------------------------------------------------------------------------
; Sign on message
;
;	Input:	None
;	Output: None
;------------------------------------------------------------------------------
	PUBLIC	SIGNON			; Routine to write signon message
SIGNON	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	CALL	WRTINL			; Write message to CRT
	DB	CR,0
;
	EXTRN	CPYMSG:BYTE
	MOV	SI,OFFSET CPYMSG
;
	MOV	CX,8
	CALL	WRTSTG			; Write string of length CX
;
	MOV	SI,OFFSET RSTMSG
	CALL	WRTOUT			; Write message to CRT
;
;
	CALL	WRTCRLF 		; Print CR,LF
	EXTRN	CPYNOT:BYTE
	MOV	SI,OFFSET CPYNOT	; Print copyright notice
;
;
	JMP	NEAR PTR WRTOUT 	; Write message to CRT & exit
;
SIGNON	ENDP
	PRGSIZ	<Power on reset Signon to CRT> ; Display program size
;------------------------------------------------------------------------------
	EXTRN	RSTMSG:BYTE
;CHANGED-----------------------------------------------------------------------
;
; Print message followed by hex message
;------------------------------------------------------------------------------
	PUBLIC	PRTHEX
	PUBLIC	PRTHE0
	PUBLIC	PRTHE1
PRTHEX	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	CALL	WRTOUT
PRTHE0:
	CALL	HEXB
PRTHE1:
	CALL	SETERF
	MOV	SI,OFFSET HEXMSG
	JMP	WRTOUT		  ; Return in WRTOUT subroutine
PRTHEX	ENDP
	PRGSIZ	<Print message followed by hex message> ; Display program size

;------------------------------------------------------------------------------
; Write string which is inline to CRT until a null
;
; Input:	None
; Output:	None
;------------------------------------------------------------------------------
	PUBLIC	WRTINL
WRTINL	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	PUSH	BP
	MOV	BP,SP
	PUSH	SI
	MOV	SI,[BP+2]
	CALL	WRTOUT
	INC	SI
	MOV	[BP+2],SI
	POP	SI
	POP	BP
	RET
WRTINL	ENDP
	PRGSIZ	<Write inline string to CRT> ; Display program size

;------------------------------------------------------------------------------
; Write decimal word, with no zero fill, print only the lower 3 digits
;
; Input:	AX = word to print
;------------------------------------------------------------------------------
	PUBLIC	DECMLW
DECMLW	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	PUSH	AX
	PUSH	DX
	PUSH	SI
	MOV	SI,(OFFSET DECTBLE)-(OFFSET DECTBL)
DECML0:
	XOR	DX,DX
	DIV	WORD PTR CS:[DECTBL-2+SI]
	ADD	AL,'0'
	CALL	PRTCHR
	MOV	AX,DX
	DEC	SI
	DEC	SI
	JNZ	DECML0
	POP	SI
	POP	DX
	POP	AX
	RET
DECTBL:
	DW	1,10,100		;,1000,10000
DECTBLE:

DECMLW	ENDP
	PRGSIZ	<Write Decimal word to CRT> ; Display program size

;------------------------------------------------------------------------------
; Write hex word, byte or nibble to CRT
;
; Input:	AX = word to print
;------------------------------------------------------------------------------
	PUBLIC	HEXW
	PUBLIC	HEXB
	PUBLIC	HEXN
HEXW	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	XCHG	AH,AL
	CALL	HEXB
	XCHG	AH,AL
HEXB:
	PUSH	AX

	PUSH	CX
	MOV	CL,4
	SHR	AL,CL
	POP	CX
	CALL	HEXN
	POP	AX
HEXN:					; Print a hex nibble to the CRT
	PUSH	AX
;
	AND	AL,0Fh			; Strip off lower bits if necessary
;
; The following is a trick to convert Hex to ASCII
;
	ADD	AL,090h
	DAA				; Decimal adjust for addition
	ADC	AL,040h
	DAA				; Decimal adjust for addition
;
	CALL	PRTCHR
	POP	AX
	RET
HEXW	ENDP
	PRGSIZ	<Write Hex word, byte or nibble to CRT> ; Display program size

;------------------------------------------------------------------------------
; Get key in uppercase from keyboard
;
; Output:	AL = Keystroke in uppercase
;		AH = Keyboard scan code
;------------------------------------------------------------------------------
	PUBLIC	GETKEY
	PUBLIC	GETKE0
GETKEY	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	CALL	WRTOUT			; Output prompt
GETKE0:
	XOR	AH,AH			; Read keyboard input
	INT	16h			;
	AND	AL,7Fh			; Strip off upper bit
	CMP	AL,'a'
	JC	GETKE1			; It's < 'a' so don't change case
	CMP	AL,'z'
	JA	GETKE1			; It's > 'z' so don't change case
	SUB	AL,'a'-'A'		; Else convert to upper case
GETKE1:
	RET
GETKEY	ENDP
	PRGSIZ	<Get key in uppercase subr> ; Display program size

;------------------------------------------------------------------------------
; Test size of memory and clear it too
;
;	Input:	BP = Paragraph starting point
;		SI = Paragraph stopping point
;		Direction Flag = 0
;	Output: AX = ?
;		SI = Last segment paragraph+1 that was good in test
;		CX = ?
;------------------------------------------------------------------------------
	PUBLIC	TSTSIZ
	PUBLIC	TSTSIZA
TSTSIZ	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	CALL	CLRMEM			; Clear the memory for an abort
TSTSIZA:				; Alternate entry doesn't clear it
	PUSH	DI
	PUSH	DS
	PUSH	BP
;
	PUSH	DX
	CALL	NMIDSB			; Disable NMI
	CALL	NMIACK
	PUSH	AX
	PUSH	DX
;
	MOV	DI,03FFh		; Test last word of each k
;
TSTSI0:
	MOV	DS,BP			; Get segment paragraph address
	ASSUME	DS:NOTHING
	MOV	AL,[DI] 		; Get original value
	MOV	AH,AL			; Save original value in AH
	NOT	AL			;   and complement in AL
	MOV	[DI],AL 		; Store complement in word
;
	XCHG	[DI],AH 		; Read complement into AH and original value
					;   into word
	CMP	AH,AL			; Check if complement values compare
	JNE	TSTSI1			;   No
	NOT	AL			;   Yes, make AL original value
	CMP	[DI],AL 		; Check word for original value
	JNE	TSTSI1			;   No
;
	CALL	CHKPAR			; Any parity ?
	JNZ	TSTSI1			;   Yes

	ADD	BP,0040h		; Set BP to next 1k
	CMP	BP,SI			; Are we past the last location+1 to do ?
	JC	TSTSI0			;   No, then do next 1k
TSTSI1:
	POP	DX
	POP	AX
	OUT	DX,AL			; Restore original value of parity
	CALL	NMIENB			; Enable NMI
	POP	DX
;
	MOV	SI,BP			; Set last (paragraph address+1) location to
					;   last good 1k+1 segment paragraph
	POP	BP
	POP	DS
	ASSUME	DS:NOTHING
	POP	DI
	RET
TSTSIZ	ENDP
	PRGSIZ	<Clear & Test memory size> ; Display program size

;------------------------------------------------------------------------------
; NMI Interrupt Handler
;------------------------------------------------------------------------------
	PUBLIC	NMIINT			; NMI interrupt
NMIINT	PROC	NEAR			; Interrupt 02h  Non Maskable Interrupt
	ASSUME	DS:NOTHING,ES:NOTHING
	PUSH	BP
	MOV	BP,SP
	PUSH	AX
	PUSH	SI
;
;
	EXTRN	NMIHDL:NEAR
	CALL	NMIHDL
;
	CALL	WRTINL			; Write error message to CRT
	DB	'.',CR,LF
	DB	' Type (S)hut off NMI,',0
;
	EXTRN	INTMSG5:BYTE
	MOV	SI,OFFSET INTMSG5
	PUSH	BX
	MOV	BH,AH
	CALL	GETKEY			; Get key in uppercase
	CALL	WRTCRLF
	MOV	AH,BH
	POP	BX
	CMP	AL,'R'
	JNZ	NMIHD3			; It's not an R so just continue
	EXTRN	RESET:NEAR
	JMP	RESET			; It was an R so reboot the disk
NMIHD3:
	CMP	AL,'S'
	JE	NMIHD4			; if 'S', don't reenable NMI int
	OR	AH,AH			; If it's an 8087 NMI interrupt,
	JZ	NMIHD4			;   don't reenable NMI int
	IN	AL,PPIB
	XOR	AL,AH
	OUT	PPIB,AL 		; else clear specific parity
;
	CALL	NMIENB			; allow non maskable interrupts
NMIHD4:
	POP	SI
	POP	AX
	POP	BP
	IRET
NMIINT	ENDP
	PRGSIZ	<NMI interrupt handler>

;------------------------------------------------------------------------------
	PUBLIC	BTMSG0
BTMSG0	DB	CR,'Boot disk failure. Type key to retry',0

;------------------------------------------------------------------------------
; Write string until terminated by a null to CRT
;
;	Input:	CS:SI = Pointer to string to print
;	Output: CS:SI = Pointer to string end (null)
;		CX = 0
;------------------------------------------------------------------------------
	PUBLIC	WRTSTI
	PUBLIC	WRTSTG
;
WRTSTI	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	OR	CX,0FFFFh		; Set CX = 0FFFFh for a string of infinite
					;   (well almost) length
; Alternate entry
;	Write string of length CX to CRT, or terminated by a null
;	Input:	CS:SI = Pointer to string to print
;		CX = Length of string
;	Output: CS:SI = Pointer to string end (null)
;		CX = 0
WRTSTG:
	PUSH	AX
WRTST0:
	MOV	AL,CS:[SI]		; Get character to write
	OR	AL,AL			; Is it a null ?
	JZ	WRTST1			;   Yes, then we're finished
	CALL	PRTCHR			; Write teletype function
	INC	SI			; Next character in string
	LOOP	WRTST0
WRTST1:
	POP	AX
	RET
WRTSTI	ENDP
	PRGSIZ	<Write string of length CX to CRT> ; Display program size

;------------------------------------------------------------------------------
; Test to see if a parity error occured while checking the 1st k of memory
;
;	Input:	None
;	Output: Zero flag set if OK, else reset if parity error
;		Also shuts off offending parity
;		AH = NMI status port
;		AL = NMI control port new value
;------------------------------------------------------------------------------
	PUBLIC	CHKPAR
CHKPAR	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	PUSH	DX
	MOV	DX,NMISTP		; Read NMI status port
	IN	AL,DX
;
	XCHG	AH,AL			; Delay 850ns for chip I/O recovery time
					;   and save in AH
;
	CALL	NMIACK			; Acknowledge and reset NMI interrupt
	AND	AH,PARCHK+IOCHK

	SHL	AH,1			; Divide Parity status by 4
	SHL	AH,1
;
;
	OR	AL,AH			; And disable any parity that was set
	OUT	DX,AL
;
	TEST	AH,(PARCHK+IOCHK)/4	; See if a parity error occured
					; Zero flag not set if error
	POP	DX
	RET
CHKPAR	ENDP
	PRGSIZ	<Check parity error>	; Display program size

;------------------------------------------------------------------------------
; Acknowledge and reset NMI parity interrupt
;
;	Input:	None
;	Output: AL = Value of NMI control port
;		DX = NMI Control port address
;------------------------------------------------------------------------------
	PUBLIC	NMIACK
NMIACK	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	MOV	DX,NMICTL		; Set to NMI control port
	IN	AL,DX			; Get current value
	XOR	AL,IOCHKO+MEMOFF	;
	OUT	DX,AL			; Reset any parity errors
	XOR	AL,IOCHKO+MEMOFF	; Delay 850ns for chip I/O recovery time
	OUT	DX,AL			; Reset any parity errors
	RET
NMIACK	ENDP
	PRGSIZ	<Acknowledge and reset NMI interrupt> ; Display program size
;******************************************************************************

