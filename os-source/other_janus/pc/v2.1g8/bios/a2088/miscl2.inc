;	NAME	MISCL2
;	TITLE	Subroutines
;***************************************************************************
;* (c)Copyright 1985 by Phoenix Software Associates Ltd.  This program	   *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent. 		   *
;***************************************************************************
;
	ASSUME CS:CODE,DS:NOTHING,ES:NOTHING
;
	IF	ROMSIZ GT (8*1024)
;------------------------------------------------------------------------------
; These macros include code here if AT or PC/XT ROM greater than 8k, else they
; are located in other modules in order to make room in an 8k PC/XT.
;------------------------------------------------------------------------------
;	PNXMAC3 			; Include PRTCHK code
	PNXMAC4 			; Include DS40H, BOOTCODE,
	PNXMAC5 			; Include COMXITA, COMEXIT
	PNXMAC6 			; Include HEXMSG, CRLFMSG
	ELSE
	EXTRN	PRTCHK:NEAR
	EXTRN	DS40H:WORD		; Segment of ROM data area
	ENDIF
;
	if	false
	EXTRN	HEXN:NEAR
	EXTRN	NMIENB:NEAR
;
	EXTRN	WRTCRLF:NEAR
	EXTRN	WRTOUT:NEAR
	EXTRN	WRTINL:NEAR
	EXTRN	HEXW:NEAR
	EXTRN	HEXB:NEAR
	EXTRN	PRTCHR:NEAR
	EXTRN	NMIDSB:NEAR
	EXTRN	SETDS40:NEAR		; Set DS to ROM data segment (0040h)
	endif

;------------------------------------------------------------------------------
; Set Boot code error flag
;
;	Input:	None
;	Output: RSTFLG Bit 0 = 1
;------------------------------------------------------------------------------
	PUBLIC	SETERF
SETERF	PROC	NEAR			; Set error flag
	ASSUME	DS:NOTHING,ES:NOTHING
	PUSH	DS
	CALL	SETDS40 		; Set DS to the ROM data segment
	ASSUME	DS:ROMDAT
	OR	BYTE PTR RSTFLG,001h	; Set Boot code error flag in Bit 0
	POP	DS
	ASSUME	DS:NOTHING
	RET
SETERF	ENDP
	PRGSIZ	<Set Boot Code diagnostic error flag>	; Display program size

;------------------------------------------------------------------------------
;	CLICK
;------------------------------------------------------------------------------
;This routine simulates a keyboard click via the speaker.  It must be
;called each time a character is read from the keyboard.
;
	IF	KEYTAP
	PUBLIC	CLICK
CLKFREQ EQU	10			; kHz * 10
;
CLICK	PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	EXTRN	ACKINT:NEAR		; Acknowledge keyboard interrupt
	PUSH	AX
	OR	AL,AL			; 
	JS	CLKEND		       ; No Click on break codes
	CMP	AL,LSTKEY		; Suppress auto repeat
	JE	CLKEND

	XOR	CX,CX			; Clear CX
	OR	CL,CLICKP     ; Get click time and use trick to set
					;   flag as well
	JZ	CLKEND			; It was 0, so we don't want a click
	IN	AL,PPIB
	PUSH	AX
	AND	AL,NOT (SPKDAT+SPKGAT)
	IF	CLICKD NE 1		; stan 6/85
	MOV	AH,CLICKD		; Get duration of pulse - IMMEDIATE MODE
	ENDIF				; stan 6/85
CLICK0:
	MOV	CL,CLICKP     ;
	XOR	AL,SPKDAT		; Change speaker data
	CLI				; stan 5/85
	OUT	PPIB,AL
WAIT0:
	LOOP	WAIT0
;
	XOR	AL,SPKDAT		; Change speaker data
	OUT	PPIB,AL
	STI				; stan 5/85
	IF	CLICKD NE 1		; stan 6/85
	DEC	AH			; stan 6/85
	JZ	CLICK9			; stan 6/85
	MOV	CL,CLICKP
	NEG	CX
	ADD	CX,CLK10*100/17*20/CLKFREQ
WAIT1:
	LOOP	WAIT1
;
	JMP	CLICK0
	ENDIF				; stan 6/85
CLICK9: 				; stan 6/85
	POP	AX
	OUT	PPIB,AL
CLKEND:
	POP	AX
	MOV	LSTKEY,AL
	RET
CLICK	ENDP
	ENDIF		;KEYTAP
	PRGSIZ	<Keyboard Click code>	; Display program size

;------------------------------------------------------------------------------
;	CLICKS
;------------------------------------------------------------------------------
;This routine is used to adjust the number of pulses used to
;simulate a keyboard click (see above).  The routine increments CLKDLY1
;when Ctrl-Alt-Plus is pressed, and decrements CLKDLY1 when
;Ctrl-Alt-Minus is pressed.  Only the numeric keypad -/+ are used.
;All registers are preserved.
;	The flags are set with the final compare statement CMP AH,53h on return
;
	IF	KEYTAP AND KEYTAPS
	PUBLIC	CLICKS
CLICKS	PROC	NEAR			;   allow setting of values with a
					;   CTRL-ALT-key code.	Before return,
					;   do a CMP AH,53h
	ASSUME	DS:ROMDAT,ES:NOTHING
	PUSH	AX
	MOV	AL,CLICKP     ; Get delay
	INC	AL			; Increase delay for now
	CMP	AH,4Eh			; Is it the + on the numeric keypad ?
	JZ	CLKCHK			;   Yes, then just check range
	DEC	AL			;   No, restore original value
	CMP	AH,4Ah			; Is it the - on the numeric keypad ?
	JNZ	CLKOK			;   No, then either Del or ignore it
	OR	AL,AL			;   Yes, is it >= 0 and <= 40h ?
	JZ	CLKCHG			; Leave it at 0
	DEC	AL			;    else decrease it
CLKCHK: CMP	AL,40h			; Is it > 40h ?
	JNA	CLKCHG			;    Yes, then OK
	MOV	AL,40h			;    No, make it within range.
CLKCHG:
	MOV	CLICKP,AL      ;
CLKOK:
	POP	AX			;   sound level
	CMP	AH,53h			; Is it the Del on the numeric keypad
	RET
CLICKS	ENDP
	ENDIF		;KEYTAP AND KEYTAPS
	PRGSIZ	<Keyboard Click set code>	; Display program size

;------------------------------------------------------------------------------
; ALT Numpad key translation
;
;	INPUT:	BL = Shift key values
;		AL = Character to translate
;	OUTPUT: AL = Translated Value of character
;------------------------------------------------------------------------------
	IF	ALTCOMP ; If doing a compatible, if not strange translation
	PUBLIC	ALTTRN
ALTTRN	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	TEST	BL,CAPLCK		; Q: IN CAPSLOCK STATE
	JE	KEYBR0			;   No, then codes are OK
	TEST	BL,LEFTSHF+RGHTSHF	; Is it shifted as well ?
	JNZ	KEYBR1			;   Yes, then codes are 
	CMP	AL,'a'
	JC	KEYBR0
	CMP	AL,'z'
	JA	KEYBR0
	SUB	AL,'a'-'A'
	RET
;
KEYBR1:
	CMP	AL,'A'
	JC	KEYBR0
	CMP	AL,'Z'
	JA	KEYBR0
	ADD	AL,'a'-'A'
KEYBR0:
	RET
	PRGSIZ	<Translate ALT numpad keys>	; Display program size
ALTTRN	ENDP
	ENDIF		; ALTCOMP

;------------------------------------------------------------------------------
; Wait for a vertical retrace period and set up MSR register
;
;	Input:	DS = ROM Data segment (0040h)
;	Output: DX = Points to MSR port address
;		AL = Value of current MSR
;
; This section of code tests for the vertical retrace in a manner that does not
; disable interrupts interrupts for over 1 millisecond.  If we were to wait for
; the vertical retrace with interrupts off, this would be approximately 16
; milliseconds.  Remember that serial interrupts at 9600 baud occur at a 1
; millisecond rate.  Also a fast typist can burst type a few characters at
; approximately 600 characters/second.
;------------------------------------------------------------------------------
	IF	PRSIS1 OR PRSIS2
	PUBLIC	VERTRC
VERTRC	PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	MOV	DX,CRTADDR
	ADD	DX,CRTST-CRTCAD 	; Set DX = CRT Status Register Address
	STI				; Set interrupts on
WAITBL:
	IN	AL,DX			; Read 6845 status register
	TEST	AL,BLANK		;   and test for retrace period
	JE	WAITBL			; Loop until present
	CLI				; Now shut off interrupts
	PUSH	CX
;
	IF	34000/CLK10 GE 1900	; Delay of code is
					;  already >= 19.00 microseconds
	MOV	CX,1
	ELSE
	MOV	CX,(190*CLK10-3400)/1700+1
	ENDIF
;
WAIT5:
	LOOP	WAIT5			; Wait 19 microseconds (at least) and
					;   see if it's still there
	POP	CX
	IN	AL,DX			;
	TEST	AL,BLANK		;
	JE	VERTRC			; No, it was a horizontal retrace or an
					;   interrupt took the vertical retrace
					;   time
	DEC	DX			;
	DEC	DX			; Set DX to MSR port
	MOV	AL,MSRCOPY		; AL = Current contents of
					;   Mode Select Register
	RET
VERTRC	ENDP
	ENDIF		; PRSIS1 OR PRSIS2
	PRGSIZ	<Wait for Vertical retrace period>	; Display program size

;-----------------------------------------------------------------------------
;
;	Wait for busy signal to support janus interface
;	Input: AL = strobe bit active
;	       CH = timeout value
;	       DX = I/O add. for control port
;	       These registers will be preserved
;
;	Updated version: HYDRA-BIOS 2.05
;
;-----------------------------------------------------------------------------
	public	waitbusy

PRTSTB	=	01h
NOTBUSY =	80h

waitbusy proc	near

	OUT	DX,AL			; Set the strobe bit

	if	sidecar 		; normal procedure with A2000
	 push	 ax
	 push	 cx
	 dec	 dx			; Point DX to status port
	 mov	 cx,10000*CLK10/17	; Wait for max. 10ms 
wbusy:	 in	 al,dx
	 and	 al,NOTBUSY		; test 'NOT busy bit' for zero
	 loopnz  wbusy			; OK => found zero = busy is activ
	 inc	 dx			; Point DX to control port
	 pop	 cx
	 pop	 ax
	endif

	XOR	AL,PRTSTB		; Reset the strobe bit signal
					;   while maintaining the other signals
	OUT	DX,AL			; reset strobe signal
	ret				; old code continues here

waitbusy endp

	PRGSIZ	<Special strobe signal (Wait for busy ) >  ; Display program								; size

	IF	OKBEEP
;****************************************************************************
;
; GENERATE A POWRUP SOUND LIKE CBM 80XX SERIES
;
;****************************************************************************
	PUBLIC	OK_BEEP

OK_BEEP PROC	NEAR
	MOV	CX,5
OK_BEEP2:
	MOV	SI,OFFSET BEEPTBL
	PUSH	CX
OK_BEEP1:
	MOV	CH,CS:[SI]
	SUB	CL,CL
	MOV	DX,17
	CALL	SPEAKR
	INC	SI
	CMP	SI,OFFSET BEEPTBL_END
	JB	OK_BEEP1

	POP	CX
	LOOP	OK_BEEP2
	RET

BEEPTBL DB	32,16,8,4,8,16,32
BEEPTBL_END LABEL BYTE

OK_BEEP ENDP

	PRGSIZ	<OK - Beep>
	ENDIF
;**************************************************************
;
; SPEAKER - CPU CARD SPEAKER DRIVER FOR COMMODORE PC BIOS.
; -------
;
; THIS HANDLES THE SYSTEM SPEAKER, USING THE 8253A PROGRAMMABLE
; INTERVAL TIMER ON THE SYSTEM CARD.
;
; EXPECTS:  CX = FREQUENCY IN HERTZ
;	    DX = DURATION IN MS
;
; DEYTROYS AX,CX,DXD
;
;
; IMPORTANT!!! - WILL CAUSE DIVIDE OVERFLOW IF REQUESTED TO
; ------------	 PLAY A NOTE WITH FREQUENCY < 19 HZ.
;
;********************************************************
;
; THE SYSTEM CARD DRIVES THE SPEAKER THROUGH OUTPUT 2 OF THE 8253A
; PROGRAMMABLE INTERVAL TIMER/COUNTER.
; TIMER 2 IS CLOCKED AT A RATE OF 1.19318 MHZ.

CLOCK_HIGH EQU	0012H			; CLOCK_HIGH:CLOCK_LOW ==
CLOCK_LOW EQU	34DDH			; DOUBLEWORD HEX 1.19318 MHZ

; ( THIS VALUE IS DIVIDED BY THE SPECIFIED FREQUENCY; DIVIDING BY
;   LESS THAN 13H WILL RESULT IN A DIVIDE OVERFLOW)
;
;***********************************************************


	PUBLIC	SPEAKR

SPEAKR	PROC	NEAR			; ( ENTRY )

	PUSHF 
	STI				; BEEPING TAKES A LONG TIME ...

	MOV	AL,PITSL2+PITRL+PITMD3	; CTRL WORD
	OUT	PITMD,AL		; SELECT TIMER

	PUSH	DX			; SAVE DURATION
	MOV	DX,CLOCK_HIGH		; DX:AX <- TIMER CLOCK FREQ
	MOV	AX,CLOCK_LOW
	DIV	CX			; GET COUNT FOR TIMER TO AX

	OUT	PIT2,AL 		; OUTPUT LSB OF COUNT
	MOV	AL,AH
	OUT	PIT2,AL 		; OUTPUT MSB OF COUNT

	IN	AL,PPIB 		; READ CURRENT SETTINGS
	MOV	CH,AL
	OR	AL,SPKGAT+SPKDAT
	OUT	PPIB,AL 		; TURN ON THE SPEAKER

	POP	AX			; ..NOW AX = DURATION TO PLAY
	CALL	MS_DELAY

	MOV	AL,CH			; RECOVER ORIGINAL SETTINGS
	OUT	PPIB,AL 		; ..SPEAKER OFF

	POPF

	RET

SPEAKR	ENDP

	PRGSIZ	<Speaker beep subr>	; Display program size

;*********************************************************************
;
; MS_DELAY - STANDARD DELAY LOOP (4.77 MHZ SYSTEM CLOCK).
; --------   EXPECTS AX = REQUIRED DELAY, IN MS (1 TO 65,535).
; (NEAR)     FOR 65,536 MS DELAY, ENTER WITH AX = 0.
;	     RETURNS AFTER THE DESIRED DELAY. ALTERS AX.
;
;*****************************************************************

PUBLIC	MS_DELAY

MS_DELAY PROC	NEAR

	PUSH	CX			; SAVE CX
					; 1 MS = 4770 CYCLES
MS_DELAY1:
	MOV	CX,((4770-22-5)/17)+1	; = 4 CYCLES
					; (CX GETS # OF LOOPS + 1)

MS_DELAY2:
	LOOP	MS_DELAY2		; = 17 CYCLES IF LOOPING, ELSE 5
	DEC	AX			; = 2 CYCLES
	JNZ	MS_DELAY1		; = 16 CYCLES IN JUMP
					; == 22 CYCLES OVERHEAD/ITERATION
	POP	CX			; RESTORE REGISTER
	RET

MS_DELAY ENDP

	PRGSIZ	<MS-Delay Loop> 	; Display program size

;------------------------------------------------------------------------------
; NMI Interrupt Leading message subroutine
;
;	INPUT:	None
;	OUTPUT: AH = Value to XOR with PPIB input to shut off interrupts
;		AL = Value of parity bits from PPIC input
;		SI = ?
;------------------------------------------------------------------------------
	PUBLIC	NMIBAD
NMIBAD	PROC	NEAR
       ASSUME	DS:NOTHING,ES:NOTHING
	CALL	NMIDSB			; Don't allow non maskable interrupts
;
	IN	AL,NMISTP
;
	CALL	WRTCRLF
	MOV	SI,OFFSET NMIMSG4	; It was an unknown type of NMI
	XOR	AH,AH
;
	AND	AL,IOCHK+PARCHK 	; I/O Channel Check or
					;   RAM Parity Check ?
	JZ	NMIBD1			; It wasn't either, probably the 8087
	MOV	SI,OFFSET NMIMSG0	; It was an I/O memory board parity
	JS	NMIBD0			; It was a memory parity
	CALL	WRTOUT
	MOV	AH,MEMOFF+IOCHKO
NMIBD0:
	XOR	AH,MEMOFF
	MOV	SI,OFFSET NMIMSG1
NMIBD1: 
	CALL	WRTOUT			; Write error message to CRT
	CALL	WRTINL			; Write error message to CRT
;
	IF	MSGLNG			; Use long messages if true
	DB	'NMI Interrupt, ',0
	ELSE
	DB	'NMI Int, ',0
	ENDIF
;
	RET
;
NMIBAD	ENDP
	PRGSIZ	<NMI leading information>	; Display program size

;------------------------------------------------------------------------------
; NMI Interrupt Handler subroutine
;
;	INPUT:	None
;	OUTPUT: AH = Value to XOR with PPIB input to shut off interrupts
;		AL = Value of parity bits from PPIC input
;		SI = ?
;------------------------------------------------------------------------------
	EXTRN	NMIACK:NEAR
;	EXTRN	NMIPLACE:NEAR
	PUBLIC	NMIHDL
NMIHDL	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
;
	CALL	NMIBAD			; Write initial NMI message
	PUSH	AX
	OR	AL,AL			; Parity error ?
	JZ	NOPAR 
	CALL	NMIPLACE		; Find place, where NMI happened
NOPAR:
	CALL	WRTINL			; Write error message to CRT
	DB	'CS:IP = ',0
;
	MOV	AX,[BP+2*2]		;go past old BP, IP
	CALL	HEXW			; Write CS in hex to CRT
;
	MOV	AL,':'
	CALL	PRTCHR			; Write char to CRT
	MOV	AX,[BP+2*1]		;go past old BP
	CALL	HEXW			; Write IP in hex to CRT
;
	PUSH	DX
	CALL	NMIACK			; Acknowledge and reset NMI interrupt
	POP	DX
	POP	AX
NMIIN0:
	RET
NMIMSG0:
	DB	'I/O Board ',0
NMIMSG1:
       IF	MSGLNG			; Use long messages if true
	DB	'Memory Parity ',0
       ELSE
	DB	'Mem Par ',0
       ENDIF
NMIMSG4:
	DB	'8087 ',0
NMIHDL	ENDP
	PRGSIZ	<NMI interrupt subr>
;
;------------------------------------------------------------------------------
; Unexpected Interrupt Handler - Main code
;
;	Input:	CS = code segment + int #, IP = OFFSET INTERR - int # * 10H
;	Output: INTLST = Interrupt # if hardware, 0FFh if software
;		If Hardware Interrupt, it is disabled
;------------------------------------------------------------------------------
;****** WARNING - because of the way the unexpected interrupt handler works,
;****** 	  the offset of INTERR must be >= 1000h ,
;****** 	  which is offset 10h*(256 interrupts);
;****** 	  and the segment of INTERR must be < 0FF00h ,
;****** 	  which is segment 0FFFF-(256 interrupts).
;****** 	  In practice, since the ROM BIOS code segment is 
;****** 	  always 0F000h, the offset must be after the 1st 4k,
;****** 	  which is almost always the case.
;****** 	  To facilitate this requirement on an AT, we will make
;****** 	  this at the end of the module that it is located in.
;******
;
	PUBLIC	INTERR
INTERR	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	PUSH	AX			; Save AX
;
	IF	INTENHD 		; If enhanced interrupt handler
	MOV	AX,CS			; AL = interrupt type
;	JMP	FAR PTR INTER0		; Hard code FAR jump
	DB	0EAH
	DW	INTER0
	PUBLIC	UNEXCS			; Make public for test code to modify
UNEXCS	DW	0F000H
;
INTER0:
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
	ENDIF		; INTENHD
;
	PUSH	DS
	CALL	SETDS40 		; Set DS to the ROM data segment
	ASSUME	DS:ROMDAT
	CMP	INTLST,0		; we are enabled ?
	JNE	PRINT_DIAGS		; YES
	JMP	QUIET_IRET		; NO, => exit
;
PRINT_DIAGS:
	PUSH	DX
	IF	INTENHD 		; If enhanced interrupt handler
	PUSH	SI
	PUSH	CX
	MOV	CH,AL			; Copy interrupt # to CH
	ENDIF
;
	MOV	AX,0FF0Bh		; say its a SW int for now (AH = FFh)
					; read in-service register (AL = 0Bh)
	MOV	DX,PIC0
	OUT	DX,AL			; ask to read in-service register
	IN	AL,DX			; read in-service register
	CMP	DH,AL			; any HW ints active? C set if so
	ADC	AH,AL			; if so, AH is ISR contents, else FFh
	MOV	INTLST,AH
	IF	INTENHD 		; If enhanced interrupt handler
	MOV	CL,'S'
	ENDIF		; INTENHD
	JNC	INTER1			; C was set by ADC if active int
	INC	DX
	IN	AL,DX			; Get interrupt masks
	OR	AL,AH			; Mask out the hardware interrupt
	JMP	$+2			; delay for chip I/O recovery time
	OUT	DX,AL
;
	DEC	DX
	MOV	AL,EOINSP		; perform end-of-interrupt on PIC
	OUT	DX,AL
;
	IF	INTENHD 		; If enhanced interrupt handler
	MOV	CL,'H'			; indicate HW interrupt
INTER1:
;
;* IF IT'S A HARDWARE INTERRUPT OR AN EXCEPTION, ------------------------------
	CMP	CH,0Fh			; is it an exception or a HW interrupt?
	JA	QUIETEXIT		;  if not, exit quietly w/ AL not 'R'
;*  BEGIN UNEXPECTED INTERRUPT ANNOUNCEMENT, RESPONSE -------------------------
;
	STI				; Enable interrupts now
	CALL	WRTINL			; Write error message to CRT
	DB	CR,LF,'Unexpected ',0
	MOV	AL,CL			; Get 'H' or 'S'
	CALL	PRTCHR			; Write 'H' or 'S'
; Print a message here saying interrupt # (AH) and wait for key to continue
	CALL	WRTINL			; Write error message to CRT
	IF	MSGLNG			; Use long messages if true
	DB	'W interrupt ',0
	ELSE
	DB	'W Int # ',0
	ENDIF
;
	MOV	AL,CH			; Get interrupt # in AL
	CALL	HEXB			; Write byte in hex to CRT
;
	CALL	WRTINL
	DB	' at ',0
;
	PUSH	BP
	MOV	BP,SP
	MOV	AX,[BP+2*7]	;go past BP, CX, SI, DX, DS, AX, IP for CS
	CALL	HEXW
;
	MOV	AL,':'
	CALL	PRTCHR
;
	MOV	AX,[BP+2*6]	;go past BP, CX, SI, DX, DS, AX for IP
	CALL	HEXW
	POP	BP
;
	CALL	WRTINL			; Write error message to CRT
	DB	'.  Type'
	PUBLIC	INTMSG5
INTMSG5:
	DB	' (R)eboot, other keys'
CONMSG: DB	' to continue',0
	CALL	GETKE0			; Get key in uppercase
	CALL	WRTCRLF
;*   END UNEXPECTED INTERRUPT ANNOUNCEMENT, RESPONSE

QUIETEXIT:
	POP	CX
	POP	SI
	CMP	AL,'R'
	ELSE		; INTENHD
INTER1:
	ENDIF		; INTENHD ELSE
;
	POP	DX
;
QUIET_IRET:
	POP	DS
	POP	AX
	IF	INTENHD 	; If enhanced interrupt handler
	JNE	INTER2		; It's not an R so just continue
	JMP	NEAR PTR RESET		; It was an R so reboot the disk
	ENDIF
;
INTER2:
	IRET

	IF	NOT INTENHD		; If not enhanced interrupt handler
	PUBLIC	INTMSG5
INTMSG5:
	DB	' (R)eboot, other keys'
CONMSG: DB	' to continue',0
	ENDIF
;
INTERR	ENDP
	PRGSIZ	<Unexpected interrupt handler>

;------------------------------------------------------------------------------

