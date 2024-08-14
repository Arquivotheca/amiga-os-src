	NAME	EIADSR
	TITLE	ROM RS232 (EIA) DSR
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program    *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent. 		   *
;***************************************************************************
;
; ROM RS232 (EIA) DEVICE SERVICE ROUTINE
;	THIS MODULE CONTAINS THE DEVICE SERVICE ROUTINE
;	FOR THE SERIAL RS232 EIA INTERFACE.
;	ACCESS TO THE EIADSR IS VIA S/W INTERRUPT 14H.
;
;******************************************************************************
;	The functions, passed in AH, are as follows;
;
;		00H - Initialize serial port DX per AL,
;			return status in AX
;		01H - Send character in AL to serial port DX
;			return status in AX
;		02H - Receive a character from serial port DX to AL
;			and return only the error bits in AH
;		03H - Return serial port DX's status in AX
;
;	Notes:	All Registers are preserved except AX for all functions
;******************************************************************************
;		Serial Port Status bits returned in AX Register
;
;		AH Register
;	-------------------------------
; bit | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
;      -------------------------------
;	|   |	|   |	|   |	|   |
;	|   |	|   |	|   |	|   +--> Data Ready
;	|   |	|   |	|   |	+------> Overrun Error
;	|   |	|   |	|   +----------> Parity Error
;	|   |	|   |	+--------------> Framing Error
;	|   |	|   +------------------> Break Error
;	|   |	+----------------------> Transmit hold register empty
;	|   +--------------------------> Transmit shift register empty
;	+------------------------------> Timeout Error
;
;		AL Register
;	-------------------------------
; bit | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
;      -------------------------------
;	|   |	|   |	|   |	|   |
;	|   |	|   |	|   |	|   +--> Delta Clear To Send
;	|   |	|   |	|   |	+------> Delta Data Set Ready
;	|   |	|   |	|   +----------> Trailing edge Ring Detect
;	|   |	|   |	+--------------> Delta Receive Line Signal detect
;	|   |	|   +------------------> Clear To Send
;	|   |	+----------------------> Data Set Ready
;	|   +--------------------------> Ring Indicator
;	+------------------------------> Receive Line Signal Detect
;******************************************************************************
;		Serial Port Control bits in AL Register
;
;	-------------------------------
; bit | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
;      -------------------------------
;	|   |	|   |	|   |	|   |
;	|   |	|   |	|   |	+---+--> Data Word Length
;	|   |	|   |	|   +----------> # of stop bits
;	|   |	|   |	+--------------> Parity Enabled
;	|   |	|   +------------------> Even Parity
;	+---+---+----------------------> Baud Rate Bits
;******************************************************************************
;
;	WRITTEN: 10/20/83
;	REVISED: mm/dd/yy ** (initials)
;		 11/22/83 *A (DLK)
;		 02/27/84 *B (DLK)
;		 03/30/84 *C (DLK)
;	CODE REDUCTION: mm/dd/yy *n (initials)
;		 04/05/84 *4 (DLK)
;		  - FOURTH PASS
;
;------------------------------------------------------------------------------
;
INCLUDE ROMOPT.INC		; ROM Options file - must be included 1st
;
INCLUDE ROMEQU.INC		; ROM General equate file
INCLUDE MACROS.INC		; ROM Macros file
;
;------------------------------------------------------------------------------
;
	OUT1	<EIADSR - ROM EIA DSR (serial port) code>
;
;	UART control bits defined for AL Register
;
URTMSK	EQU	01FH	;Mask for UART parameters	
;
BDRMSK	EQU	0E0h	;Mask for the baud rate bits
;
B110	EQU	000h	;110  Baud
B150	EQU	020h	;150  Baud
B300	EQU	040h	;300  Baud
B600	EQU	060h	;600  Baud
B1200	EQU	080h	;1200 Baud
B2400	EQU	0A0h	;2400 Baud
B4800	EQU	0C0h	;4800 Baud
B9600	EQU	0E0h	;9600 Baud
;
EVNPAR	EQU	018h	;Even Parity
ODDPAR	EQU	008h	;Odd  Parity
NOPAR	EQU	000h	;No   Parity
;
STOP2	EQU	004h	;2 Stop Bits (or 1-1/2 stop bits for 5 data bits)
STOP1	EQU	000h	;1 Stop Bit
;
LEN8	EQU	003h	;8 Data Bits
LEN7	EQU	002h	;7 Data Bits
LEN6	EQU	001h	;6 Data Bits
LEN5	EQU	000h	;5 Data Bits
;******************************************************************************
;	Status bits returned in AH
;
TIMOUT	EQU	080h	;Timeout Error
;
TSRE	EQU	040h	;Transmit shift register empty
XMTRDY	EQU	020h	;Transmit hold register empty
			;  (and hence ready for next character)
BRKRCV	EQU	010h	;Break Received
FRMERR	EQU	008h	;Framing Error
PARERR	EQU	004h	;Parity Error
OVRERR	EQU	002h	;Overrun Error
DATRDY	EQU	001h	;Data Ready
;******************************************************************************
;	Equates for INS8250 device
;
; Line Control Register
;
BAUDIV	EQU	11520	; This #  times 10 divided by the baud rate
			;   will give the # to load into the baud rate
			;   generator for the serial port
;
DLAB	EQU	080h	;Divisor Latch Access Bit, allows access to baud rate
			;  generator's divisor latches
BREAK	EQU	040h	;Send Break
STKPAR	EQU	020h	;Stick Parity Enable
EVENPS	EQU	010h	;Even Parity select
PAREN	EQU	008h	;Parity Enable
STOPBT	EQU	004h	;Stop Bits, "0" = 1 stop bit, "1" = 2 stop bits
WRDLEN	EQU	003h	;Data word length, 5 -> 8 bits
;
; Modem Control register
;
MDLOOP	EQU	010h	;Modem loopback for diagnostic testing
MDOUT2	EQU	008h	;Auxiliary user-designated output 2
MDOUT1	EQU	004h	;Auxiliary user-designated output 1
RTS	EQU	002h	;Request to Send
DTR	EQU	001h	;Data Terminal Ready
;
; Modem Status Register
;
MDRLSD	EQU	080h	;Receive Line Signal Detect
MDRI	EQU	040h	;Ring Indicator
MDDSR	EQU	020h	;Data Set Ready
MDCTS	EQU	010h	;Clear To Send
MDDRLSD EQU	008h	;Delta Receive Line Signal detect
MDTERI	EQU	004h	;Trailing edge Ring Detect
MDDDSR	EQU	002h	;Delta Data Set Ready
MDDCTS	EQU	001h	;Delta Clear To Send
;
; Line Status Register
;
LSTSRE	EQU	040h	;Transmit shift register empty
LSTHRE	EQU	020h	;Transmit hold register empty
			;  (and hence ready for next character)
LSBI	EQU	010h	;Break Interrupt Received
LSFE	EQU	008h	;Framing Error
LSPE	EQU	004h	;Parity Error
LSOR	EQU	002h	;Overrun Error
LSDR	EQU	001h	;Data Ready
;******************************************************************************
;
DGROUP	GROUP	ROMDAT
ROMDAT	SEGMENT AT 0040h	;The following are relative to Segment 40h
	EXTRN	EIADRTBL:WORD		; COMM PORT ADDRESS TABLE
	EXTRN	EIATOTBL:BYTE		; COMM PORT TIMEOUT TABLE
	EXTRN	DEVFLG:WORD		; Device Flag register
ROMDAT	ENDS
;
;------------------------------------------------------------------------------
;
CGROUP	GROUP	CODE
CODE	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:CODE,DS:ROMDAT,ES:NOTHING
;
	EXTRN	SAVREGS:NEAR		; Save registers 
	EXTRN	COMEXIT:NEAR		; Common DSR Exit routine
	EXTRN	RNGCHK:NEAR		; Check function range
;
;------------------------------------------------------------------------------
;
; BAUD Rate Generator Table
;
;------------------------------------------------------------------------------
	SYNC	BAUDTBA
BAUDTB	DW	BAUDIV/11		; 110 Baud
	DW	BAUDIV/15		; 150 Baud
	DW	BAUDIV/30		; 300 Baud
	DW	BAUDIV/60		; 600 Baud
	DW	BAUDIV/120		; 1200 Baud
	DW	BAUDIV/240		; 2400 Baud
	DW	BAUDIV/480		; 4800 Baud
	DW	BAUDIV/960		; 9600 Baud
	PRGSIZ	<BAUD Rate Table>	; Display program size
	SYNC	EIADSRA
;------------------------------------------------------------------------------
;
; EIA DSR ENTRY POINT VIA S/W INT 14H
;
;	AH = OPCODE (00H - 03H)
;
;
;------------------------------------------------------------------------------
	PUBLIC	EIADSR
EIADSR	PROC	FAR
	ASSUME	DS:NOTHING,ES:NOTHING
	STI
	CALL	SAVREGS 		; Save caller's Registers
;
	CMP	DX,MAXSER		; Check if device # index <= highest
					;   serial port #
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
	MOV	BX,DX			; BX=Device # Index by bytes
;
; In case we need it, get timeout byte
;
	MOV	CL,[EIATOTBL+BX]	; Get serial timeout value
;
	SHL	BX,1			; Convert index to a word index
;
	MOV	DX,[EIADRTBL+BX]	; Get Base I/O Addr of device
	OR	DX,DX			; Check if entry is zero
	JZ	EXIT			;  if so then exit w/ regs unchanged
;
	XOR	CH,CH			; Upper bits of timeout are zero
	XOR	BX,BX			; USE BX AS TIMEOUT COUNTER
					;   or clear upper bits of baud rate
					;   index
	ADD	DX,4			; DX=I/O ADDR OF MODEM CONTROL REGISTER
;
	JMP	WORD PTR CS:[DI+OFFSET CMDXFER]  ; Jump to function processor
;
CMDXFER DW	OFFSET EIAINIT		; Initialize Serial Port
	DW	OFFSET EIAXMT		; Send a character to Serial Port
	DW	OFFSET EIARCV		; Receive a character from Serial Port
	DW	OFFSET EIAST		; Return Serial Port Status
XFEREND EQU	$-CMDXFER
;------------------------------------------------------------------------------
;
; INITIALIZE COMM PORT (AH=00H)
;
;	INPUT:	DX = Modem Control Register port
;		AL = Baud Rate and UART control parameters
;		BH = 0, upper bits of baud rate index
;	OUTPUT: AH = Line Status
;		AL = Modem Status
;
;
; Start with Baud Rate initialization
;
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
EIAINIT:
	MOV	AH,AL			; Save parameters in AH
;
	DEC	DX			; DX=I/O Addr of Line CTRL Reg (LCR)
;
	MOV	BL,AH			; Restore parameter byte to BL
	AND	BL,BDRMSK		; Get baud rate bits
	MOV	CL,4			;  and convert to a word index for
	SHR	BL,CL			;  baud rate table

	MOV	BX,[BAUDTB+BX]		; Get Base BAUD Rate divisor value
	CALL	SETBD
;
; Set Parity, Stop Bits and Word Length
;
	MOV	AL,AH			; Restore Parameters
	AND	AL,URTMSK		; Get only UART control bits
;
	OUT	DX,AL			; Initialize Line Control Register
;
	INC	DX			; DX=I/O Base address of device
					;   fall through to status routine
	IF	I80286
	JMP	$+2			; Delay for I/O
	ENDIF
;
;------------------------------------------------------------------------------
;
; RETURN SERIAL PORT STATUS (AH=03H)
;
;	INPUT:	DX = Modem Control Register port
;	OUTPUT: AH = Line Status
;		AL = Modem Status
;
;	Notes:	This routine located here so that conditional jumps
;			can reach label EXIT
;
;------------------------------------------------------------------------------
	ASSUME	DS:NOTHING,ES:NOTHING
EIAST:
	INC	DX			; DX=I/O Addr of Line Status Reg
	IN	AL,DX			; Read Line Status Register 
	XCHG	AH,AL			;   and return in AH
					; Delay for chip I/O recovery time
	INC	DX			; DX=I/O Addr of Modem Status Reg
;
	IF	I80286
	JMP	$+2			; Delay for I/O
	ENDIF
;
EXITA:
	IN	AL,DX			; Read Modem Status Register to AL
EXIT:
	JMP	COMEXIT 		; Go to common DSR exit
;------------------------------------------------------------------------------
;
; TRANSMIT A CHAR (AH=01H)
;
;	INPUT:	DX = Index into device table
;		AL = Character to transmit
;		CX = Timeout value
;		BX = 0, used as timeout counter
;	OUTPUT: AH = Line Status
;
;------------------------------------------------------------------------------
	ASSUME	DS:NOTHING,ES:NOTHING
EIAXMT:
	MOV	AH,AL			; SAVE CHAR TO XMT
	MOV	AL,RTS+DTR	;
	OUT	DX,AL			; TURN ON DTR & RTS
	INC	DX
	INC	DX		; DX=I/O ADDR OF MODEM STATUS REGISTER
L01H5:
	IN	AL,DX			; READ MSR
	AND	AL,MDDSR+MDCTS		; DSR and CTS present ?
	CMP	AL,MDDSR+MDCTS		; 
	JNE	L01H15			;   No
	DEC	DX		; DX=I/O ADDR OF LINE STATUS REG
L01H10A:
	XOR	BX,BX			; RESTORE BX IF NECESSARY	*B
L01H10:
	IN	AL,DX			; READ LSR
	TEST	AL,LSTHRE	; Transmit hold register empty ?
	JE	L01H11			;  * NO
	SUB	DX,5
	MOV	AL,AH			; AL=CHAR TO XMT		*B
	OUT	DX,AL			; PUT CHAR ON INTERFACE 	*B
	XOR	AH,AH			; INDICATE NO ERROR
	JMP	SHORT EXIT		; FINISHED
L01H11:
	DEC	BX			; Q: COUNTED BACK TO ZERO
;
	JNZ	L01H10			; LOOP FOR SPECIFIED NUMBER OF TIMES
	LOOP	L01H10A 		; LOOP FOR SPECIFIED NUMBER OF TIMES
	JMP	SHORT SERSN5		; JUMP WHEN TIMED OUT
L01H15:
	DEC	BX			; Q: COUNTED BACK TO ZERO
	JNZ	L01H5			; LOOP FOR SPECIFIED NUMBER OF TIMES
;
;
	LOOP	L01H5			; LOOP FOR SPECIFIED NUMBER OF TIMES
SERSN5:
	IN	AL,DX			; READ LSR
	XCHG	AH,AL			; Swap byte order
	AND	AH,BRKRCV+FRMERR+PARERR+OVRERR
					; Return only the error bits
	OR	AH,TIMOUT		; OR in timeout bit
	JMP	SHORT EXIT		; FINISHED
;------------------------------------------------------------------------------
;
; RECEIVE A CHAR (AH=02H)
;
;	INPUT:	DX = Index into device table
;		CX = Timeout value
;		BX = 0, used as timeout counter
;	OUTPUT: AH = Line Status (error bits only, = 0 if OK)
;		AL = Received Character
;
;------------------------------------------------------------------------------
	ASSUME	DS:NOTHING,ES:NOTHING
EIARCV:
	MOV	AL,DTR			; Turn DTR on and RTS off
	OUT	DX,AL
	INC	DX
	INC	DX			; DX = I/O Address port of Modem Status
					;   Register (MSR)
L02H5:
	IN	AL,DX			; Read MSR
	TEST	AL,MDDSR		; Is Data Set Ready ?
	JNE	L02H9			;   Yes
;
	DEC	BX			; Next loop
;
	JNZ	L02H5			; TIMEOUT LOOP			*B
;
;
	LOOP	L02H5			; TIMEOUT LOOP			*B
	DEC	DX
	JMP	SHORT L02H30		; JUMP WHEN TIMED OUT		*B
L02H9:								       ;*B
	DEC	DX		; DX=I/O ADDR OF LINE STATUS REGISTER
L02H10A:
	XOR	BX,BX			; RESTORE BX IF NECESSARY	*B
L02H10:
	IN	AL,DX			; READ LSR
	TEST	AL,LSDR 	; Received Data Ready ?
	JE	L02H15			;  * NO 			*B
	MOV	AH,AL			; SAVE STATUS
	AND	AH,BRKRCV+FRMERR+PARERR+OVRERR
	SUB	DX,5			; READ CHAR
	JMP	SHORT EXITA
L02H15:
	DEC	BX			; Q: COUNTED BACK TO ZERO
;
	JNZ	L02H10			; TIMEOUT LOOP			*B
	LOOP	L02H10A 		; TIMEOUT LOOP			*B
L02H30:
;
; Set character which will eventually be in AL to a NULL if nothing received
; and only allow error bits to be returned in AH, actually AL could be anything
;
	XOR	AH,AH
	JMP	SHORT SERSN5		; FINISHED
;
EIADSR	ENDP
	PRGSIZ	<EIA DSR>		; Display program size
;------------------------------------------------------------------------------
;
; Set baud rate divisor
;
;	INPUT:	DX = I/O address of LCR
;		BX = Baud rate divisor value
;	OUTPUT: DX = I/O address of LCR
;		AL = MSB of baud rate divisor
;		DI = I/O address of data register
;
;------------------------------------------------------------------------------
	PUBLIC	SETBD			; Set baud rate
SETBD	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	MOV	AL,DLAB 		; Set Divisor Latch Access Bit
	OUT	DX,AL			;   to allow programming of baud rate
	MOV	DI,DX			; Save LCR address in SI
;
	DEC	DX			; 
	DEC	DX			;  
	MOV	AL,BH			; Set Baud rate divisor (MSB)
;
	IF	I80286
	JMP	$+2			; Delay for I/O
	ENDIF
;
	OUT	DX,AL			;
	DEC	DX			; DX=I/O Addr of 
					;   Baud Rate Divisor Latch (LSB)
	MOV	AL,BL			; Get LSB
;
	IF	I80286
	JMP	$+2			; Delay for I/O
	ENDIF
;
	OUT	DX,AL			; Set Baud rate divisor (LSB)
;
	XCHG	DI,DX			; DX=I/O Addr of Line CTRL Reg (LCR)
	RET
	PRGSIZ	<Set baud rate code>	; Display program size
SETBD	ENDP
;------------------------------------------------------------------------------
;
	SYNC	KYBDSRA
;
	TOTAL
;
CODE	ENDS
	END
