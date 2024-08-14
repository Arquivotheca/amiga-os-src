	NAME	KYBDSR
	TITLE	ROM KEYBOARD DSR
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program    *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent. 		   *
;***************************************************************************
;
; ROM KEYBOARD DEVICE & INTERRUPT SERVICE ROUTINES
;	THIS MODULE CONTAINS THE DEVICE SERVICE ROUTINE AND
;	INTERRUPT SERVICE ROUTINES FOR THE KEYBOARD.
;	ACCESS TO THE KEYBOARD DSR IS VIA S/W INTERRUPT 16H;
;	ACCESS TO THE INTERRUPT SERVICE ROUTINE IS VIA S/W
;	INTERRUPT 9.  FUNCTIONS SUPPORTED:
;
;		00H - READ KEYBOARD INPUT
;		01H - READ KEYBOARD STATUS
;		02H - READ SHIFT STATUS
;		03h - (JR) Set typamatic rates		(not implemented)
;		04h - (JR) Set/Clear keyboard click	(not implemented)
;
;	WRITTEN: 10/11/83
;	REVISED: mm/dd/yy ** (initials)
;		 06/  /84 -> 12/  /84 Ira J. Perlow - tightened code and fixed
;				bugs.  Structure modified extensively
;		 12/  /84 Ira J. Perlow modified for AT
;		 05/07/85 Ira J. Perlow changed CTRL-DEL to produce no code
;	Revised: 05/20/85 by Ira J. Perlow - fixed INS key multiple makes
;	Revised: 05/24/85 by Ira J. Perlow - ALT numpad key 0 bug
;
;	Revised: 11/11/85 by Torsten Burgdorff/Commodore - install warmstart 
;							   vector SYSRESET
;	Revised: 11/13/85 by Dieter Preiss/Commodore - installed far call
;			  before JMP COMXITA in KEYISR
;			  Corrected Bug: No BREAK or SCROLL Stop
;					 if CTRL + ALT pressed
;------------------------------------------------------------------------------
;
INCLUDE ROMOPT.INC		; ROM Options file - must be included 1st
INCLUDE ROMEQU.INC		; ROM General equate file
INCLUDE MACROS.INC		; ROM Macros file
INCLUDE KYBEQU.INC		; Keyboard Equate file
;
;------------------------------------------------------------------------------
;
	OUT1	<KYBDSR - ROM Keyboard DSR and ISR code>
;
; Keyboard Secondary Shift Flags Bit equates
;
INSTATM EQU	80h			; BIT7 = INSERT DEPRESSED
CAPLCKM EQU	40h			; BIT6 = CAPS LOCK DEPRESSED
NUMLCKM EQU	20h			; BIT5 = NUM LOCK DEPRESSED
SCRLCKM EQU	10h			; BIT4 = SCROLL LOCK DEPRESSED
PAUSE	EQU	08h			; BIT3 = PAUSE STATE ON/OFF
SYSREQ	EQU	04h			; BIT2 = System request key depressed
					;   (AT only)
;	EQU	02h			; BIT1 =
;	EQU	01h			; BIT0 =
;
;------------------------------------------------------------------------------
DGROUP	GROUP	ROMDAT
ROMDAT	SEGMENT AT 0040h	;The following are relative to Segment 40h
	EXTRN	SHFLGS:BYTE		; SHIFT STATE FLAGS
	EXTRN	SHFLGS2:BYTE		; SECONDARY SHIFT STATE FLAGS
	EXTRN	KYBFLG:BYTE		; Keyboard LED and Flags Data Area
	EXTRN	ALTDATA:BYTE		; ALT KEYPAD ENTRY
	EXTRN	KBGET:WORD		; KEYBOARD BUFFER GET POINTER
	EXTRN	KBPUT:WORD		; KEYBOARD BUFFER PUT POINTER
	EXTRN	KBXTGET:WORD		; Keyboard buffer start value,
					;   referenced to DS = 40h
	EXTRN	KBXTPUT:WORD		; Keyboard buffer end value
					;   referenced to DS = 40h
	EXTRN	MSRCOPY:BYTE		; MODE-SELECT REGISTER COPY
	EXTRN	CRTADDR:WORD		; Base address of CRT controller
	EXTRN	CRTMODE:BYTE		; CRT display mode
;
	EXTRN	RSTFLG:WORD		; Reset flag
	EXTRN	BRKFLG:BYTE		; Break key flag
	EXTRN	KEYEXT:DWORD
	EXTRN	LSTKEY:BYTE
;
ROMDAT	ENDS
;
;------------------------------------------------------------------------------
;
CGROUP	GROUP	CODE
CODE	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
;
	EXTRN	RNGCHK:NEAR		; CHECK OPCODE RANGE
	EXTRN	SAVREGS:NEAR		; SAVE CALLER'S REGISTERS
	EXTRN	COMXITA:NEAR		; Common DSR return code with A register
	EXTRN	BEEP:NEAR		; SOUND SPEAKER SUBROUTINE
;
	EXTRN	SETDS40:NEAR		; Set DS to ROM data segment (0040h)
;
;------------------------------------------------------------------------------
; KYBDSR ENTRY POINT VIA S/W INT 16H
;
;	AH = OPCODE (00H - 02H)
;------------------------------------------------------------------------------
	SYNC	KYBDSRA
;
	PUBLIC	KYBDSR
KYBDSR	PROC	FAR
	ASSUME	DS:NOTHING,ES:NOTHING
	STI
	PUSH	DI
	PUSH	DS
;
; BRANCH TO REQUESTED OPERATION PROCESSOR
;
	MOV	DI,OFFSET XFEREND-2	; DI=LENGTH OF CMD JUMP TABLE
	CALL	RNGCHK			; GO CHECK OPCODE
	ASSUME	DS:ROMDAT
	JC	EXIT1			; JUMP ON RANGE ERROR
	JMP	WORD PTR CS:[DI+OFFSET CMDXFER]  ; JUMP TO PROCESSOR
;------------------------------------------------------------------------------
; READ SHIFT STATUS (AH=02H)
;
;	Input:	DS = ROM data segment (0040h)
;	Output: AL = SHIFT STATUS BYTE
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
READSHF:
	MOV	AL,SHFLGS		; GET SHIFT FLAGS
EXIT1:
	POP	DS
	ASSUME	DS:NOTHING
	POP	DI
	IRET
;
KYBDSR	ENDP
	PRGSIZ	<Keyboard DSR and code for entry 02h>
;
;------------------------------------------------------------------------------
; Buffer wrap routine
;
;	Input:	DS = ROM data segment (0040h)
;		SI = Keyboard buffer pointer
;	Output: SI = Next keyboard buffer pointer
;------------------------------------------------------------------------------
BUFWRP	PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	INC	SI
	INC	SI
;
; Do the buffer the XT way unless someone really doesn't want it
;
	IF	XTKBUF			
	CMP	SI,KBXTPUT		; Need to wrap buffer pointer ?
	JC	BUFWR0			;   No
	MOV	SI,KBXTGET		;   Yes, set pointer back to buffer
					;     start
	ELSE
	CMP	SI,OFFSET KBUFR+32	; Need to wrap buffer pointer ?
	JC	BUFWR0			;   No
	MOV	SI,OFFSET KBUFR 	;   Yes, set pointer back to buffer
					;     start
	ENDIF

BUFWR0:
	RET
BUFWRP	ENDP
	PRGSIZ	<Keyboard Buffer Pointer wrap subr>
;------------------------------------------------------------------------------
; Translate Scan codes
;
;	Input:	AL = Keyboard scan code
;		CS:SI = Translation table address
;	Output: AL = ASCII character code
;		Zero flag set if code is 0FEh, else reset
;		Carry flag set if code is below 0FEh, else rest
;------------------------------------------------------------------------------
XLAT46	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	SUB	AL,46h			; Bias to 1 for numeric keypad
					;   (it will then be adjusted to 0)
;
; Alternate Entry point for the keyboard
;
XLAT:
	DEC	AL			; Bias scan code to 0
	PUSH	DS
	PUSH	CS
	POP	DS			; set DS = CS for translation
	ASSUME	DS:CODE
	PUSH	BX
	MOV	BX,SI
	XLAT				; Translate scan code
	POP	BX
	POP	DS
	ASSUME	DS:NOTHING
	CMP	AL,0FFh 		; Set flag if it didn't translate
	RET
XLAT46	ENDP
	PRGSIZ	<Keyboard translate scan codes subr>
;------------------------------------------------------------------------------
; Check if Keyboard is Full
;
;	Input:	DS = ROM data segment (0040h)
;	Output: Zero Flag set if full, else reset
;		SI = KBPUT + 2 or the beginning of the buffer
;------------------------------------------------------------------------------
CHKFULL PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	MOV	SI,KBPUT		; Get keyboard put pointer
	CALL	BUFWRP			; Move SI to next location in buffer
	CMP	SI,KBGET		; Buffer full ?
	RET
CHKFULL ENDP
	PRGSIZ	<Keyboard Buffer Full check subr>
;------------------------------------------------------------------------------
;
; Keyboard Decoding Tables
;
; This section contains tables used in the decoding of scan codes received
; by the Keyboard ISR.
;
; An "0FFh" entry in the table means the scan code is thrown away.
;
NORMTBL LABEL	BYTE			; lowercase translation
	DB	 ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9'
	DB	 '0', '-', '=',  BS, TAB, 'q', 'w', 'e', 'r', 't'
	DB	 'y', 'u', 'i', 'o', 'p', '[', ']',  CR,0FFh, 'a'
	DB	 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'"
	DB	 '`',0FFh, '\', 'z', 'x', 'c', 'v', 'b', 'n', 'm'
	DB	 ',', '.', '/',0FFh, '*',0FFh, ' ',0FFh
; and the function keys
	DB	03Bh,03Ch,03Dh,03Eh,03Fh,040h,041h,042h,043h,044H
	DB	0FFh,0FFh
;------------------------------------------------------------------------------
UPPRTBL LABEL	BYTE			; UPPER CASE translation
	DB	 ESC, '!', '@', '#', '$', '%', '^', '&', '*', '('
	DB	 ')', '_', '+',  BS,00Fh, 'Q', 'W', 'E', 'R', 'T'
	DB	 'Y', 'U', 'I', 'O', 'P', '{', '}',  CR,0FFh, 'A'
	DB	 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"'
	DB	 '~',0FFh, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M'
	DB	 '<', '>', '?',0FFh,0FFh,0FFh, ' ',0FFh
; and the function keys
	DB	054h,055h,056h,057h,058h,059h,05Ah,05Bh,05Ch,05Dh
	DB	0FFh,0FFh
;------------------------------------------------------------------------------
CTRLTBL LABEL	BYTE			; CTRL-KEY TRANSLATION
;		Esc	  Nul		       RS
	DB	 ESC,0FFh,NULL,0FFh,0FFh,0FFh,01Eh,0FFh,0FFh,0FFh
;		      US       Del	 DC1  ETB  ENQ	DC2  DC4
	DB	0FFh,01Fh,0FFh,07Fh,0FFh,011h,017h,005h,012h,014h
;		 EM  NAK   HT	SI  DLE  Esc   GS   LF	     SOH
	DB	019h,015h,009h,00Fh,010h, ESC,01Dh,  LF,0FFh,001h
;		DC3  EOT  ACK  Bel   BS   LF   VT   FF
	DB	013h,004h,006h,BELL,  BS,  LF,00Bh,  FF,0FFh,0FFh
;			   FS  SUB  CAN  ETX  SYN  STX	 SO   CR
	DB	0FFh,0FFh,01Ch,01Ah,018h,003h,016h,002h,00Eh,  CR
;				  CTL-PRTSC	SP
	DB	0FFh,0FFh,0FFh,0FFh,072h,0FFh, ' ',0FFh
; and the function keys
	DB	05Eh,05Fh,060h,061h,062h,063h,064h,065h,066h,067h
	DB	0FFh,0FFh
;------------------------------------------------------------------------------
ALTTBL	LABEL	BYTE			; ALT-KEY TABLE
;		      1    2	3    4	  5    6    7	 8    9
	DB	0FFh,078h,079h,07Ah,07Bh,07Ch,07Dh,07Eh,07Fh,080h
;		 0    -    =		  Q    W    E	 R    T
	DB	081h,082h,083h,0FFh,0FFh,010h,011h,012h,013h,014h
;		 Y    U    I	O    P			      A
	DB	015h,016h,017h,018h,019h,0FFh,0FFh,0FFh,0FFh,01Eh
;		 S    D    F	G    H	  J    K    L
	DB	01Fh,020h,021h,022h,023h,024h,025h,026h,0FFh,0FFh
;				Z    X	  C    V    B	 N    M
	DB	0FFh,0FFh,0FFh,02Ch,02Dh,02Eh,02Fh,030h,031h,032h
;					       SP
	DB	0FFh,0FFh,0FFh,0FFh,0FFh,0FFh,020h,0FFh
; and the function keys
	DB	068h,069h,06Ah,06Bh,06Ch,06Dh,06Eh,06Fh,070h,071h
	DB	0FFh,0FFh
 
;	PRGSIZ	<Non-numeric pad keyboard translate tables>
						; Display program size
;------------------------------------------------------------------------------
; Keyboard Interrupt Service Routine (ISR)
;
;	Input:	None
;	Output: None
;------------------------------------------------------------------------------
	SYNC	KYBISRA
;
	PUBLIC	KYBISR
KYBISR	PROC	FAR
	ASSUME	DS:NOTHING,ES:NOTHING
	CALL	SAVREGS 		; Save caller's registers
	PUSH	AX
;
;
	MOV	DX,PPIA 		; DX = Keyboard Input port I/O address
	IN	AL,DX			; Read keyboard scan code
;
	CALL	SETDS40 		; Set DS to the ROM data segment
	ASSUME	DS:ROMDAT
;
	IF	KEYTAP			; If keyboard has a key click sound
	EXTRN	CLICK:NEAR		; This routine placed here DP
	CALL	CLICK			;   make key tap click sound when
					;   putting character in buffer
	ELSE				;   save character in LSTKEY	     
	MOV	LSTKEY,AL		;   required by international	    
					;   keyboard drivers
	ENDIF

;
	MOV	BX,WORD PTR SHFLGS	; BL = Primary shift flags
					; BH = Secondary shift flags
;
;
KBDFUL:
	CMP	AL,0FFH 		; Scancode buffer in keyboard full code
					;   returned ?
	JNE	KBDRDY			;   No
	JMP	EXITBP			;   Yes, exit with a beep indicating
					;     a full keyboard buffer
KBDRDY:
	MOV	AH,AL			; Save scan code
	AND	AL,7FH			; Mask high-order bit
;
; Handle special keys
;
	PUSH	CS			; Get segment of table
	POP	ES			;   Into ES
	ASSUME	ES:CODE
	MOV	DI,OFFSET SPCTBL
	MOV	CX,(OFFSET SPCTBLE - OFFSET SPCTBL)
	CLD
	REPNE	SCASB
	JNE	INSERT			; If not found, then scan code was not
					;   special
	SUB	DI,OFFSET SPCTBL+1	; If scan code was found, subtract table
					;   offset to get the index
	MOV	CL,CS:[DI+OFFSET SPCMSK]	; Get keycode mask
	CMP	DI,(OFFSET LCKTBL - OFFSET SPCTBL)
	JNC	KEYLCK			; It is a lockable key
;
;
ALTKEY:
	CMP	AH,38H			; ALT make key ?
	JNE	NOTALT			;   No
	TEST	BL,ALTSHF		; Is it already depressed ?
	JNZ	NOTALT			;   Yes
	MOV	ALTDATA,0		;   No, reset ALT/NUMERIC pad
					;     accumulator
NOTALT:
;
	OR	BL,CL			; Set appropriate bits for shift key
;
	OR	AH,AH			; Is it a break ?
	JS	KEYBRK			;   Yes
;
K0:	JMP	EXITISR
;------------------------------------------------------------------------------
KEYLKB:
	NOT	CL			; Invert bits
	AND	BH,CL			; Set key not depressed flag
	JMP	SHORT K0
;------------------------------------------------------------------------------
INSRT1: 
	TEST	BL,ALTSHF+CTRLSHF+LEFTSHF+RGHTSHF ; SHIFT, ALT or CTRL state ?
INSRT0:
	JNE	K0			;  Yes, ignore INS key
	AND	BH,NOT INSTATM		; Set INSERT key not depressed flag
INSRT2:
	JMP	SHORT K0
;------------------------------------------------------------------------------
KEYBRK:
	XOR	BL,CL			; RESET STATE ON BREAK INTERRUPT
;
	CMP	AL,38H			; ALT break key ?
	JNE	K0			;   No
;
	XOR	AX,AX			; Set up AX as normal "ASCII"
	OR	AL,ALTDATA		; Get ALTDATA value and check for 0
					;   with this trick
	JZ	K0			; There was no numeric pad input
;
	IF	ALTCOMP ; If doing a compatible, if not strange translation
	EXTRN	ALTTRN:NEAR
	CALL	ALTTRN			; Translate ALT numpad value
	ENDIF
;
	JMP	SHORT CASOKB		; Go place it in buffer
;------------------------------------------------------------------------------
INSERT:
	CMP	AH,80h+52H		; INS key break?
	JNE	ISR29			;   No
;
; HANDLE "BREAK" INTERRUPT ON INSERT KEY
;
	TEST	BL,NUMLCK		; In NUMLOCK state ?
	JE	INSRT1			;   No
	TEST	BL,LEFTSHF+RGHTSHF	; Shift key depressed ?
	JZ	K0			;   No, then ignore it
;
	TEST	BL,ALTSHF+CTRLSHF	;   Yes, is it a CTRL or ALT function ?
	JMP	SHORT INSRT0		;   
;------------------------------------------------------------------------------
KEYLCK: OR	AH,AH			; Keyboard "Make" keycode ?
	JS	KEYLKB			;   No, it's a "Break" keycode
;

	TEST	BL,CTRLSHF		; In CTRL state ?
	JNZ	K4			;   Yes, process CTRL-BREAK
					;     or CTRL-NUMLOCK
;
	TEST	BH,CL			; Lockable key already depressed ?
	JNE	K0			;   Yes
	OR	BH,CL			; Set key depressed flag
	XOR	BL,CL			; Reverse state of LOCK state
KEYLC0: JMP	SHORT K0
;------------------------------------------------------------------------------
ISR29:
	OR	AH,AH			; Keyboard "Make" keycode ?
	JS	K0			;   No, it's a "Break" keycode,
					;     ignore it
	CMP	AH,52h			; INS key make ?
	JNE	ISR28A			;   No
INSRT4:
	TEST	BL,ALTSHF+CTRLSHF	;   Yes, is it an ALT or CTRL function ?
	JZ	INSRT5			;     No, then treat as NUMPAD
ISR28A:
	TEST	BH,PAUSE		; In PAUSE state ?
	JZ	INSRT3			;   No
ISR28:
	AND	BH,NOT PAUSE		;   Yes, exit pause state
	CALL	ackint		       ; ACKNOWLEDGE INTERRUPT	       *B
	JMP	ISR88			; GET OUT			*B
;------------------------------------------------------------------------------
K4:
	TEST	BL,ALTSHF		; Done if CTRL + ALT - Bug fix DP
	JNZ	K0			; Pass if CTRL only
	CMP	AH,45h			; Is it a NUM lock make ?
	JNE	BREAK			;   No, then is it scroll lock ?
;
	OR	BH,PAUSE		;  Yes, enter pause state
	CMP	CRTMODE,7		;Unless we're in mono mode
	JE	K0			; (in which case video wasn't disabled),        MOV     DX,CRTADDR              ; Get base of CRT controller
	ADD	DX,04h			; Set DX = Mode Select Register
	MOV	AL,MSRCOPY		; AL = Current contents of
					;   mode status register
	OUT	DX,AL			; Turn on video
	JMP	SHORT KEYLC0
;------------------------------------------------------------------------------
; BREAK KEY ENTERED
;
BREAK:
	TEST	BH,PAUSE		; Are we in pause state ?
	JNE	ISR28			;   Yes, then just exit pause state
;
	CMP	AH,46h			; Is it scroll lock make ?
	JNE	KEYLC0			;   No, then it's a CTRL-CAPSLOCK,
					;     which we ignore
;
; Do the buffer the XT way unless someone really doesn't want it
;
	IF	XTKBUF			
	MOV	AX,KBXTGET		; Get start of keyboard buffer
	ELSE
	MOV	AX,OFFSET KBUFR 	; Get start of keyboard buffer
	ENDIF
;
	MOV	KBGET,AX		; Set pointers to start
	MOV	KBPUT,AX
	MOV	BRKFLG,80H		; SET BREAK HAPPENED FLAG
;
; To be compatible the DS register is the only one that is required to be
; preserved by INT 1Bh.  Since we need to preserve BX, we must save and restore
; it.
;
	PUSH	BX
	INT	1BH			; CALL USER BREAK ROUTINE (IF ANY)
	POP	BX
	XOR	AX,AX
CASOKB: JMP	SHORT CASOKA
;------------------------------------------------------------------------------
ISR35:
	MOV	ALTDATA,0		; CLEAR ALT/NUMERIC BYTE
	TEST	BL,LEFTSHF+RGHTSHF	; Q: SHIFT KEY DEPRESSED	*4
	JE	ISR45			;  * NO 			*4
	CMP	AL,37H			; POSSIBLE PRINT SCREEN
	JNE	ISR45			;  * NO
;
; HANDLE PRINT SCREEN REQUEST
;
	CALL	ACKINT			; FIRST ACKNOWLEDGE KEYBOARD INTERRUPT
	CALL	CHKFULL 		; Q: KEYBOARD BUFFER FULL
	MOV	WORD PTR SHFLGS,BX	; Restore Primary Shift flags from BL
					;   and Secondary Shift Flags from BH
	JZ	PRINT			;  * YES, DON'T PRINT ANYTHING  *8
	INT	5H			; GO TO PRINT SCREEN ROUTINE
PRINT:
	JMP	ISR89
;------------------------------------------------------------------------------
INSRT3:
	CMP	AH,47H			; Scan code from numeric/cursor pad ?
	JC	ISR35			;   No
INSRT5:
	JMP	NUMPAD			;   Yes
;------------------------------------------------------------------------------
;
; BY NOW, ALL SPECIAL OPERATION KEYS HAVE BEEN FERRETED OUT
;    ONLY ASCII & EXTENDED ASCII SCAN CODES REMAIN
;
ISR45:
	MOV	SI,OFFSET ALTTBL	; Set ALT translation table
	TEST	BL,ALTSHF		; ALT key depressed ?
	JNZ	ISR75			;   Yes
;
	MOV	SI,OFFSET CTRLTBL	; Set CTRL translation table
	TEST	BL,CTRLSHF		; CTRL key depressed ?
	JNZ	ISR75			;   Yes
;
	MOV	SI,OFFSET UPPRTBL	;   Else switch to UPPER CASE XLAT table
	TEST	BL,LEFTSHF+RGHTSHF	; Shift Keys depressed
	JNZ	ISR75			;   Yes
;
	MOV	SI,OFFSET NORMTBL	; DEFAULT TO NORMAL XLAT TABLE
ISR75:
	CALL	XLAT			; GO TRANSLATE SCAN CODE
	JE	EXITISR 		;  * NO, IGNOR IT

	CMP	AH,3AH			; Q: ENTRY A FUNCTION KEY
	JA	ISR80			;  * YES, USE "EXTENDED ASCII" FORMAT
;
	TEST	BL,CTRLSHF		; Q: CTRL KEY DEPRESSED 	*D
	JE	ISR76A			;  * NO 			*D
	CMP	AH,37H			; Q: PrtSc KEY			*D
	JE	ISR80			;  * YES, USE "EXTENDED" FORMAT *D
ISR76A:
	TEST	BL,ALTSHF		; Q: ALT KEY DEPRESSED		*D
	JE	ISR76B			;  * NO 			*D
	CMP	AH,39H			; Q: SPACE BAR			*D
	JE	CASOK			;  * YES, USE "NORMAL" FORMAT	*D
ISR76B:
	CMP	AH,0FH			; Q: TAB/BACKTAB SCAN CODE
	JNE	ISR77			;  * NO
	CMP	AL,0FH			; Q: BACKTAB CHAR
	JE	ISR80			;  * YES, USE "EXTENDED ASCII" FORMAT
ISR77:
	TEST	BL,ALTSHF		; Q: IN ALT SHIFT STATE
	JE	ISR85			;  * NO, USING "NORMAL ASCII" FORMAT
ISR80:					; USE "EXTENDED ASCII" FORMAT
	MOV	AH,AL			; AH=ASCII CODE
	XOR	AL,AL			; AL=ZERO
CASOKA: JMP	SHORT CASOK
;
ISR85:
	TEST	BL,CAPLCK		; Q: IN CAPSLOCK STATE
	JE	CASOK			;   No, then codes are OK
	CMP	AL,'A'
	JC	CASOK
	CMP	AL,'Z'
	JA	NOTCAP
	ADD	AL,'a'-'A'
	JMP	SHORT CASOK
;	
NOTCAP:
	CMP	AL,'a'
	JC	CASOK
	CMP	AL,'z'
	JA	CASOK
	SUB	AL,'a'-'A'
;
;	AH=SCAN CODE; AL=ASCII CODE
;
CASOK:
;
; PUT CHARACTER INTO KEYBOARD BUFFER
;
;   INPUT:	AL = ASCII CHARACTER
;		AH = SCAN CODE
;
	CLI				; MASK INTERRUPTS
	CALL	CHKFULL 		; Q: BUFFER FULL
	JZ	EXITBP			;   Yes
	MOV	SI,KBPUT		; GET KEYBOARD PUT POINTER
	MOV	[SI],AX 		;  * YES, PUT IT IN BUFFER
	CALL	BUFWRP			; Move SI to next location in buffer
	MOV	KBPUT,SI		; UPDATE PUT POINTER
	JMP	SHORT EXITISR
;
EXITBP:
	STI				; Unmask interrupts because a beep is
					;   a long time
	CALL	BEEP			; Beep speaker because we're full
;
EXITISR:
; Added (DP)--------------------------------------------------------------
;
; The following pointer in set up in INITCD and changed when DOS
; loads a national Keyboard Driver  (KEYBxx.COM):
;
	CALL	DWORD PTR KEYEXT	; Invoke national keyboard routine    
					; If not loaded, KEYEXT is a pointer
					; to a far return (init by INITCD)
					; In this case it`s a dummy call.
;
;-------------------------------------------------------------------------
;
	CALL	ACKINT			; ACKNOWLEDGE INTERRUPT
	
;EXITIS0:
	TEST	BH,PAUSE		; In pause state ?
	JE	ISR88			;   No
	TEST	SHFLGS2,PAUSE		;   Yes, are we already in pause state ?
	JNE	ISR88			;     Yes
	MOV	WORD PTR SHFLGS,BX	; Restore Primary Shift flags from BL
					;   and Secondary Shift Flags from BH
ISR86:
	TEST	SHFLGS2,PAUSE		; Q: STILL IN PAUSE STATE
	CLI				; MASK INTERRUPTS AGAIN
	JE	ISR89			;  * NO
	STI				;  * YES, ENABLE INTERRUPTS
	JMP	SHORT ISR86		; SPIN

ISR88:
	MOV	WORD PTR SHFLGS,BX	; Restore Shift flags from BX
ISR89:								       ;*8
	JMP	COMXITA 		; Common DSR return code with A register
;
; HANDLE INPUT FROM NUMERIC/CURSOR PAD
;
NUMPAD:
	TEST	BL,ALTSHF		; In ALT state ?
	JZ	ISR140			;   No
	TEST	BL,CTRLSHF		; In CTRL state ?
	JZ	ISR110			;   No

;
	IF	KEYTAP AND KEYTAPS	; If keyboard has a key click sound
	EXTRN	CLICKS:NEAR
	CALL	CLICKS			;   allow setting of values with a
					;   CTRL-ALT-key code.	Before return,
					;   do a CMP AH,53h
	ELSE
	CMP	AH,53H			; DEL key ?
	ENDIF
;
	JNE	ISR110			;   No
;
;CHANGED-----------------------------------------------------------------------
;
; SYSTEM RESET (ALT-CTRL-DEL) ENTERED
;
	MOV	WORD PTR RSTFLG,1234H	; Signal reset in progress
	EXTRN	RESET:NEAR		; SYSTEM RESET ROUTINE (cold boot)
	EXTRN	SYSRESET:NEAR		; WARM BOOT ENTRY
	JMP	SYSRESET		; Branch to reset code (warm boot)
;
;
; HANDLE ALT-NUMPAD ENTRY
;
ISR110:
	MOV	BP,AX			; Save scancode
	MOV	AL,ALTDATA		; GET RUNNING ALT-NUMPAD ENTRY
;
	MOV	AH,10			; Multiply accumulated # by 10
	MUL	AH
	XCHG	AX,BP			; Put scan code in AX and (# * 10) in BP
;
	MOV	SI,OFFSET SHFTNP	; Use Shifted numeric pad translation
	CALL	XLAT46			; CONVERT SCAN CODE TO ASCII
	CMP	AL,"-"			; Was it a "-" ?
	JE	ISR112			;   Yes, then ignore
	SUB	AL,'0'			; Convert decimal ASCII to binary
	JC	ISR112			; It was a "+" or ".", so ignore
	ADD	AX,BP			; AX=NEW ACCUMULATED ALT-NUMPAD ENTRY
	JMP	SHORT ISR112A
;
ISR112:
	XOR	AL,AL			; Clear ALT/NUMERIC byte
ISR112A:
	MOV	ALTDATA,AL		; SAVE IT
ISR113:
	JMP	EXITISR
;
ISR140:
	MOV	SI,OFFSET CTRLNP	; Use CTRL numeric pad translation
	TEST	BL,CTRLSHF		; In CTRL state ?
	JNZ	SHFTED			;   Yes
;
	MOV	SI,OFFSET SHFTNP	; Use Shifted numeric pad translation
	TEST	BL,NUMLCK		; In NUMLOCK state ?
	JNE	NUMST			;   Yes

	TEST	BL,LEFTSHF+RGHTSHF	; Shift key depressed ?
	JNE	SHFTED			;   Yes, use Shifted numeric pad XLAT
;
	JMP	NRMLNM			;   No, use normal numeric pad XLAT
;
NUMST:
	TEST	BL,LEFTSHF+RGHTSHF	; Shift key depressed ?
	JE	SHFTED			;   No, use Shifted numeric pad XLAT
;
NRMLNM: 
	MOV	SI,OFFSET NORMNP	; Use Normal Case numeric pad translation
	CMP	AL,52H			; Ins KEY ?
	JNE	SHFTED			;   No, then no need to change flags
	TEST	BH,INSTATM		; Is it already depressed ?
	JNZ	ISR112			;   Yes, then ignore codes
	XOR	BL,INSTAT		; Reverse state of INSERT state if 1st
					;   time
	OR	BH,INSTATM		; Set INSERT key depressed flag
SHFTED:
	CALL	XLAT46			; TRANSLATE SCAN CODE
	JE	ISR113			;   Ignore it if not valid
	CMP	SI,OFFSET SHFTNP	; Using the numbers on pad ?
	JE	ISR146			;   Yes, use normal format
	CMP	AL,"-"			;   No, then test if it's a "-"
					;     or a "+"
	JBE	ISR146			; It is a "-" or "+" so use normal
					;   format
	JMP	ISR80			; Else use "Extended ASCII" format
ISR146:
	JMP	CASOK			; Use normal ASCII format
KYBISR	ENDP
	PRGSIZ	<Keyboard ISR>
;
;------------------------------------------------------------------------------
;
KYBDSC	PROC	FAR
;
CMDXFER DW	OFFSET READINP		; READ KEYBOARD INPUT
	DW	OFFSET READSTAT 	; READ KEYBOARD STATUS
	DW	OFFSET READSHF		; READ SHIFT STATUS
XFEREND EQU	$-CMDXFER
;
;------------------------------------------------------------------------------
; READ KEYBOARD INPUT (AH=00H)
;
;	Input:	DS = ROM data segment (0040h)
;	Output: AL = ASCII CHARACTER
;		AH = SCAN CODE
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
READINP:
	PUSH	SI
;
;
;
READI2:
	CLI				; MASK INTERRUPTS
	MOV	SI,KBGET		; GET KEYBOARD GET POINTER
	CMP	SI,KBPUT		; Q: ANYTHING IN BUFFER
	JNE	READI0			;   Yes, key was pressed
	STI				; UNMASK INTERRUPTS
	IF	WAITHLT
	HLT				; Wait for keyboard or timer interrupt
	ENDIF	;WAITHLT
	JMP	SHORT READI2		; Check to see if any keyboard inpu
;
READI0:
	MOV	AX,[SI] 		; PLUCK CHAR/SCAN CODE FROM BFR *D
	CALL	BUFWRP			; Move SI to next location in buffer
	MOV	KBGET,SI		; SAVE NEW GET POINTER
	STI				; UNMASK INTERRUPTS
;
;
	POP	SI
	POP	DS
	POP	DI
	IRET
;------------------------------------------------------------------------------
; READ KEYBOARD STATUS (AH=01H)
;
;	Input:	DS = ROM data segment (0040h)
;	Output: AL = ASCII CHARACTER
;		AH = SCAN CODE
;		Z FLAG = 1 if no character available
;		Z FLAG = 0 if character available
;		Carry flag = ?
;		Aux carry = ?
;		Overflow flag = ?
;		Sign flag = ?
;		Parity flag = ?
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
READSTAT:
;
	CLI				; MASK INTERRUPTS
	MOV	DI,KBGET		; GET KEYBOARD GET POINTER
	CMP	DI,KBPUT		; Q: ANYTHING IN BUFFER
	JZ	EXIT			;   No, exit with Zero flag set
	MOV	AX,[DI] 		; Get the character and status without
					;   touching Zero flag
;
; COMMON EXIT
;
EXIT:
	STI				; UNMASK INTERRUPTS
	POP	DS
	ASSUME	DS:NOTHING
	POP	DI
	RET	2			; THROW AWAY CALLER'S FLAGS
;
KYBDSC	ENDP
	PRGSIZ	<Keyboard DSR code>
;------------------------------------------------------------------------------
; Shift key type table
;------------------------------------------------------------------------------
SPCTBL:
	DB	1Dh			; Ctrl Key ?
	DB	2Ah			; Left shift key ?
	DB	36h			; Right shift key ?
	DB	38h			; ALT key ?
;
;
LCKTBL:
	DB	3Ah			; CAPSLOCK key ?
	DB	45h			; NUMLOCK key ?
	DB	46h			; SCROLL-LOCK key ?
SPCTBLE:
;------------------------------------------------------------------------------
; Shift Key bit table
;------------------------------------------------------------------------------
SPCMSK	LABEL	BYTE
	DB	CTRLSHF 		; Control Key make
	DB	LEFTSHF 		; Left shift key make
	DB	RGHTSHF 		; Right shift key make
	DB	ALTSHF			; Alt key make
;
;
	DB	CAPLCK			; Caps lock make
	DB	NUMLCK			; Num lock make
	DB	SCRLCK			; Scroll lock make
	PRGSIZ	<Special scan code table>
;------------------------------------------------------------------------------
; NUMERIC PAD TABLES
;------------------------------------------------------------------------------
;	Normal Numeric Pad Translation
;
NORMNP	DB	047H,048H,049H,"-",04BH,0FFH,04DH,"+",04FH,050H,051H,052H,053H
;
;	Shifted Numeric Pad Translation
;
SHFTNP	DB	 "7", "8", "9","-", "4", "5", "6","+", "1", "2", "3", "0", "."
;
;	Control Numeric Pad Translation
;
CTRLNP	DB	077H,0FFH,084H,0FFH,073H,0FFH,074H,0FFH,075H,0FFH
	DB	076H,0FFH,0FFH
;
	PRGSIZ	<Numeric pad keyboard translate tables>
						; Display program size
;------------------------------------------------------------------------------
; Acknowledge Keyboard Interrupt
;
;	Input:	None
;	Output: DX = ?
;		AL = ?
;		Interrupts are disabled
;------------------------------------------------------------------------------
	PUBLIC	ACKINT			; hidden
ACKINT	PROC	NEAR			; DS must pointo BIOS data seg
;
	ASSUME	DS:NOTHING,ES:NOTHING
	CLI				; MASK INTERRUPTS
;
	MOV	DX,PPIB 		; DX=I/O ADDRESS OF KEYBOARD CTRL PORT
	IN	AL,DX			; READ CONTROL PORT
	OR	AL,SNSWEN		; TURN CLEAR KEYBOARD BIT
					; Delay 850ns for chip I/O recovery time
	OUT	DX,AL			; WRITE TO CONTROL PORT, CLEARING KEYBOARD
	XOR	AL,SNSWEN		; Delay 850ns for chip I/O recovery time
	OUT	DX,AL			; RE-ENABLE KEYBOARD
;
	MOV	AL,EOINSP		; AL=END OF INTERRUPT BIT
	MOV	DX,PIC0 		; DX=SYSTEM INTERRUPT ACKNOWLEDGE ADDR
	OUT	DX,AL			; ACKNOWLEDGE 8259 INTERRUPT
;
;
	RET

ACKINT	ENDP
	PRGSIZ	<Keyboard Interrupt ack subr>
;------------------------------------------------------------------------------
; Dummy keyboard interrupt routine used during init code
;------------------------------------------------------------------------------
	PUBLIC	KBDDMY
KBDDMY	PROC	FAR
	ASSUME	DS:NOTHING,ES:NOTHING
	PUSH	AX
;
	MOV	AH,DL		
	OR	DH,DH			; If we have already gotten a character
	JNZ	KBDDM0			;   ignore them til we clear DL
;
	IN	AL,PPIA 		; 
	MOV	AH,AL
KBDDM0:
	CALL	ackint		       ; ack interrupt
;
	MOV	DL,AH			; Put input character in DL
	MOV	DH,1			; tell boot code we interrupted
	POP	AX
	IRET				; return
KBDDMY	ENDP
	PRGSIZ	<Keyboard dummy code>	; Display program size
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;
	SYNC	DSKDSRA
;
	TOTAL
;
CODE	ENDS
	END
