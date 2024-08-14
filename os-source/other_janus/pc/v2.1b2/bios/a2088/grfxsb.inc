;	NAME	GRFXSB
;	TITLE	ROM Graphic subroutines
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program    *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent. 		   *
;***************************************************************************
;
;
; Revision History:
;
; 5-DEC-85 Commodore BSW / Dieter Preiss (DP):
;  - Changed video segment setup in proc. REGENADR
;	   was: Set up from current video mode (CRTMODE)
;	    is: Set up from current DIP switch setting (DEVFLG)
;	reason: Bug Fix for improved compatibility
;
;
; CRT ROM DEVICE SERVICE ROUTINE GRAPHICS SUBROUTINES
;	THIS MODULE CONTAINS SUBROUTINES FOR THE CRT DEVICE
;	SERVICE ROUTINE.  SUBROUTINES ARE:
;
;	    GRAFIXROLL - SCROLL SCREEN IN GRAPHICS MODE
;	    GRAFIXREAD - READ CHAR FROM SCREEN IN GRAPHICS MODE
;	    GRAFIXWRITE - WRITE CHAR TO SCREEN IN GRAPHICS MODE
;	    RTNCURP - RETURN CURSOR POSITION
;	    REGENADR - COMPUTE CHAR/ATTR ADDR IN REGEN
;
;
VERWD	EQU	3963/15*CLK10/10	; # of words written to CRT during
					;   vertical retrace for a STOSW
VERBYT	EQU	3963/11*CLK10/10	; # of bytes written to CRT during
					;   vertical retrace for a STOSB
VERWDM	EQU	3963/22*CLK10/10	; # of words written to CRT during
					;   vertical retrace for a MOVSW
;
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
	PRGSIZ	<>			; Display program size
;------------------------------------------------------------------------------
; COMPUTE ADDR OF CHAR/ATTR IN REGEN MEMORY of cursor DX in current page
;
;	Input:	DS = ROM data segment (0040h)
;		DH,DL = REQUESTED CURSOR POSITION
;	Output: DI = OFFSET INTO REGEN MEMORY (WITHIN DISPLAY PAGE)
;		ES = SEGMENT ADDRESS OF REGEN MEMORY
;		BX = Cursor position in page
;------------------------------------------------------------------------------
REGAD	PROC	NEAR			; Compute char/attr addr for DX
	ASSUME	DS:ROMDAT,ES:NOTHING
	MOV	BL,DSPYPAG		;   on current page
	JMP	SHORT REGAD1	
REGAD	ENDP
;------------------------------------------------------------------------------
; COMPUTE ADDR OF CHAR/ATTR IN REGEN MEMORY
;
;	Input:	DS = ROM data segment (0040h)
;		BH = DISPLAY PAGE
;	Output: DI = OFFSET INTO REGEN MEMORY (WITHIN DISPLAY PAGE)
;		ES = SEGMENT ADDRESS OF REGEN MEMORY
;		BX = Cursor position in page
;------------------------------------------------------------------------------
REGENADR PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	MOV	BL,BH			; Get requested display page
	CALL	RTNCURP 		; Get cursor position in DX
REGAD1:
	PUSH	AX
	XOR	DI,DI			; Clear DI to 0 for page offset
	MOV	ES,WORD PTR CS:B000H	; Set ES to address of monochrome REGEN
	MOV	AL,BYTE PTR DEVFLG	; Which monitor connected ? (DP)
	AND	AL,30H			; Isolate video bits	    (DP)
	ASSUME	ES:NOTHING
	CMP	AL,30H			; Monochrome adaptor ?	    (DP)
	JE	REGAD4			;   Yes if eq
	MOV	ES,WORD PTR CS:B800H	; Set ES to address of graphic REGEN
					;	memory
	ASSUME	ES:NOTHING
	CMP	CRTMODE,4		; In graphics mode or monochrome adaptor
	JNC	REGAD4			;   Yes
;
; This section of code takes 15+13*(page #+1) clocks
	MOV	AX,SCRNLEN		; AX=Page length in bytes
	XOR	BH,BH			; BX = page # extended
;
	INC	BX			; DI = BX * AX
	SUB	DI,AX
REGAD3: 	;Loop time is 13*(page+1)+4
	ADD	DI,AX
	DEC	BX
	JNZ	REGAD3
;
; DI = Address of requested display page
;
REGAD4:
	MOV	AL,DH			; AL=Requested row
	MUL	BYTE PTR SCRNWID	;   multiplied by screen width
	ADD	AL,DL			; Add column to row offset in AX
	ADC	AH,0			;   and add carry if any
	SAL	AX,1			; Multiply by 2 bytes per screen
					;   location
	MOV	BX,AX			; Return cursor address from page
					;   origin in BX
	ADD	DI,AX			; Return cursor address from segment ES
					;   in DI
	POP	AX
	RET
REGENADR ENDP
	PRGSIZ	<Character cursor address subr> ; Display program size
;------------------------------------------------------------------------------
; GRAPHICS MODE SCROLL
;
;	Input:	DS = ROM data segment (0040h)
;		DL = Top Row # of window
;		CH = Left Side column #
;		CL = Width in characters
;		DH = Bottom Row #
;		BH = Attribute of blanked lines
;		BL = 0, Scroll up.  All other values scroll down
;		AH = # of lines in window
;		AL = # of lines to scroll or 0 to blank all lines
;		SI = Bottom Row #, Top Row # of window
;   RETURNED:	None
;------------------------------------------------------------------------------
GRAFIXROLL	PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	CMP	CRTMODE,06h		; Get CRT mode and see if 640 x 200 mode
	JE	GROLL1			; If yes, then OK
	SHL	CX,1			;   Else pretend like it has twice
					;     as many columns	
GROLL1:
;
	MOV	DL,CH			; DL = Bottom left side
	CALL	GRFXAD			; Get DI = Address of left corner
	MOV	ES,WORD PTR CS:B800H	; Set ES=B800H
	ASSUME	ES:NOTHING
	XOR	CH,CH			; Extend word count of
					;   each line of window to 16 bits
;
; Compute offset to odd scan line
;
	MOV	DX,2000h		; Add 2000h for offset to odd scan
	SUB	DX,CX			;   - window width done on even scan
;
; Compute offset from end of an odd scan line to beginning of the next even
;   scan line
;
	MOV	BP,2000h-80		; BP = Offset to even scan line from
					;   odd scan line + screen width in bytes
	ADD	BP,CX			; BP = -(Screen width - window width)
;
	PUSH	CX			; Save window width
	MOV	CX,320			; CX = bytes between rows
;
	OR	BL,BL			; Scroll direction ?
	JZ	GROLL2			;   It's scroll up
;
	ADD	DI,240			; Start from bottom
	ADD	BP,160			; BP = -(Screen width + window width)
	NEG	CX			; CX = -(bytes between rows)
GROLL2:
	OR	AL,AL			; If AL = 0,
	JZ	GROLL5			;   then clear window
;
	MOV	SI,DI			; Compute source address
GROLL3:
	ADD	SI,CX			;   and add in offset
	DEC	AL
	JNZ	GROLL3
;
	POP	CX			; Restore window column width
;
	MOV	DS,WORD PTR CS:B800H	; Set DS=B800H
	ASSUME	DS:NOTHING
	SHL	AH,1			; Multiply # of rows by 4 because we
	SHL	AH,1			;   have to move 4 pair of scans/row
GROLL4:
	PUSH	CX			; Save count
	SHR	CX,1			; Divide by 2 and see if odd
	CLI				; Disable interrupts for short time
	JNC	GROLL4A
	MOVSB				; Move 1 character if odd
GROLL4A:
	REP	MOVSW			; Move 2 characters at a time
	STI				; Set interrupts back on
	POP	CX			; Restore count,
;
	ADD	SI,DX			; Switch to address of odd scan
	ADD	DI,DX			; Switch to address of odd scan
	PUSH	CX			; Save count
	SHR	CX,1			; Divide by 2 and see if odd
	CLI				; Disable interrupts for short time
	JNC	GROLL4B
	MOVSB				; Move 1 character if odd
GROLL4B:
	REP	MOVSW			; Move 2 characters at a time
	STI				; Set interrupts back on
	POP	CX			; Restore count,
;
	SUB	SI,BP			; Compute next scan for each
	SUB	DI,BP
	DEC	AH			; Next pair of scans or rows
	JNZ	GROLL4			; Loop until finished all scans & rows
;
	PUSH	CX			; Save count to even up POP below
;
;  No, set up to blank rest of rows
;
GROLL5:
	POP	CX			; Restore window column width
	MOV	AH,BH			; AH = Attribute of blank lines
	MOV	AL,BH			; AL = Attribute of blank lines
;
	POP	BX			; Get (top row + rows moved)
					;   and bottom row
	INC	BH
	SUB	BH,BL
	SHL	BH,1			; Multiply # of rows by 4 because we
	SHL	BH,1			;   have to move 4 pair of scans/row
;		
	MOV	SI,CX			; Save count in SI
GROLL6:
	SHR	CX,1			; Divide by 2 and see if odd
	CLI				; Disable interrupts for short time
	JNC	GROLL7A
	STOSB				; Write 1 blank character if odd
GROLL7A:
	REP	STOSW			; Write 2 blank characters at a time
	STI				; Set interrupts back on
	MOV	CX,SI			; Restore count from SI
;
	ADD	DI,DX			; Switch to address of odd scan
	SHR	CX,1			; Divide by 2 and see if odd
	CLI				; Disable interrupts for short time
					;   see discussion above
	JNC	GROLL7B
	STOSB				; Write 1 blank character if odd
GROLL7B:
	REP	STOSW			; Write 2 blank characters at a time
	STI				; Set interrupts back on
	MOV	CX,SI			; Restore count from SI
	SUB	DI,BP			; Switch to address of next even scan
;
	DEC	BH
	JNZ	GROLL6			; Loop if we're not done
;
	RET

GRFXROLL:	JMP	GRAFIXROLL
;
GRAFIXROLL ENDP
;------------------------------------------------------------------------------
; Screen roll subroutine
;
;	Input:	DS = ROM data segment (0040h)
;		DL = Top Row # of window
;		CH = Left Side column #
;		CL = Width in characters
;		DH = Bottom Row #
;		BH = Attribute of blanked lines
;		BL = 0, Scroll up.  All other values scroll down
;		AH = # of lines in window
;		AL = # of lines to scroll or 0 to blank all lines
;   RETURNED:	None
;	Note:  For video modes 2 & 3, this routine does not wait for vertical
;		retrace to disable or enable the screen.  This has not been
;		determined as the cause of any hash on screen.	However, if it
;		does, the equate for PRSIS1 and/or PRSIS2 should be changed to
;		true.  It was also determined that the characters could be moved
;		during the horizontal and vertical retraces only, however this
;		was determined to be unacceptably slow. (Note that when moving
;		during the horizontal retrace, only one character could be
;		moved at a time as opposed to 2 writes that could be done as
;		used in WRTSUB
;------------------------------------------------------------------------------
	PUBLIC	ROLLSUB
ROLLSUB PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	MOV	SI,DX			; Save Top row position in SI
	ADD	DL,AH
	PUSH	DX			; Save top and bottom row #'s
	MOV	DX,SI			; Get top and bottom row #'s
;
	OR	BL,BL			; Scroll direction ?
	JNZ	ROLL1A			;   It's scroll down
	MOV	DH,DL			; Get Top row position and move it to DH
ROLL1A:
	CALL	GRFXCHK 		; Are we in graphics mode ?
	JC	GRFXROLL		;   Yes, go do a graphics scroll
;
	MOV	DL,CH			; DL = Bottom left side
	PUSH	BX			; Save attribute & flag
	CALL	REGAD			; Get ES:DI = Address of left corner
	ASSUME	ES:NOTHING
	POP	BX			; Restore attribute & flag
	XOR	CH,CH			; Extend word count of
					;   each line of window to 16 bits
	PUSH	CX
	SHL	CX,1
	MOV	DX,CX			; Get window width in DX
;
	MOV	BP,SCRNWID		;
	SHL	BP,1
;
	MOV	CX,BP			; CX = Screen width in bytes
;
	SUB	BP,DX			; BP = Screen width - window width
;
	OR	BL,BL			; Test scroll direction
	JZ	ROLL2			; Scroll Up
;
	SUB	BP,CX			;   Else down, BP = -(Window width)
	SUB	BP,CX			; BP = -(Screen width + window width)
	NEG	CX			; CX = -(Screen width)
ROLL2:
	MOV	BL,CRTMODE		; Get CRT mode and prepare for test
	AND	BL,0FEh 		;   of mode 2 or mode 3
	CMP	BL,002h 		;   (80x25 alpha B/W or Color mode?)
	JNE	VOFF10			; No
	PUSH	DX			; Save compare address
	PUSH	AX
;
	IF	PRSIS1
	EXTRN	VERTRC:NEAR
	CALL	VERTRC			; Wait for vertical retrace
	AND	AL,LOW (NOT VIDEN)	; Turn off video enable bit
	OUT	DX,AL			; Turn off video
	STI				; Set interrupts back on

	ELSE		;PRSIS1
	MOV	DX,CRTADDR
	ADD	DX,MSRADR-CRTCAD	; Set DX = Mode Select Register
	MOV	AL,MSRCOPY		; AL = Current contents of
					;   Mode Select Register
	AND	AL,LOW (NOT VIDEN)	; Turn off video enable bit
	OUT	DX,AL			; Turn off video
	ENDIF		;PRSIS1
	POP	AX
	POP	DX			; Get back compare address
;
VOFF10:
	OR	AL,AL			; If AL = 0,
	JZ	ROLL5			;   then blank screen
;
	MOV	SI,DI			; Compute source address
ROLL3:
	ADD	SI,CX			;   and add in offset
	DEC	AL
	JNZ	ROLL3
;
	POP	CX			; Restore window column width
	PUSH	DS			; Save data segment
	PUSH	ES			; Copy video source segment
	POP	DS			;   into data segment
	ASSUME	DS:NOTHING
ROLL4:
	PUSH	CX			; Save count
; Disable interrupts so that control numlock can't interrupt us.  If CX were
; to be so large that we couldn't service interrupts for over 1 millisecond
; we couldn't do this.  Remember that serial interrupts at 9600 baud occur
; at this rate.  Also a fast typist can burst type a few characters at
; approximately 600 characters/second.
	CLI				; Disable interrupts for short time
	REP	MOVSW			; Move character and attribute
	STI				; Set interrupts back on
	POP	CX			; Restore count,
	ADD	SI,BP			; Compute next row for each
	ADD	DI,BP
	DEC	AH			; Next row
	JNZ	ROLL4			; Another row to move ?
	POP	DS			; Restore data segment so we can access
					;   data variables
	ASSUME	DS:ROMDAT
	PUSH	CX			; Save count to even up POP below
;
;  No, set up to blank rest of rows
;
ROLL5:
	POP	CX			; Restore count
	MOV	AH,BH			; AH = Attribute of blank lines
	MOV	AL,' '			; AL = Space character
	POP	BX			; Get (top row + rows moved)
					;   and bottom row
	INC	BH
	SUB	BH,BL
	MOV	SI,CX			; Save count in SI
ROLL6:
	CLI				; Disable interrupts for short time
					;   see discussion above
	REP	STOSW			; Write character and attribute to CRT
	STI				; Set interrupts back on
	MOV	CX,SI			; Restore count from SI
	ADD	DI,BP			; Switch to address of next even scan
;
	DEC	BH
	JNZ	ROLL6			; Loop if we're not done
;
	MOV	BL,CRTMODE		; Get CRT mode and prepare for test
	AND	BL,0FEh 		;   of mode 2 or mode 3
	CMP	BL,002h 		;   (80x25 alpha B/W or Color mode?)
	JNE	VOFF20			; No
;
	IF	PRSIS2
	CALL	VERTRC			; Wait for vertical retrace
	OUT	DX,AL			; Turn on video
	STI				; Set interrupts back on
;
	ELSE		;PRSIS2
;
	MOV	DX,CRTADDR
	ADD	DX,MSRADR-CRTCAD	; Set DX = Mode Select Register
	MOV	AL,MSRCOPY		; AL = Current contents of
					;   Mode Select Register
	OUT	DX,AL			; Turn on video
	ENDIF		;PRSIS2
VOFF20:
	RET
ROLLSUB ENDP
	PRGSIZ	<Scroll window subr>	; Display program size
;------------------------------------------------------------------------------
; Write Subroutine
;
;	Input:	DS = ROM data segment (0040h)
;		BH = Active display page #
;		CX = # of times to write character
;		AL = Character to write
;		BL = Attribute to write to screen if DL = 0 or if graphic mode
;		DL = 0 if writing attribute, else don't
;   RETURNED:	None
;	Note:  For video modes 2 & 3, this routine outputs 2 characters for
;		each horizontal retrace.  This has not been determined as the
;		cause of any hash on screen.  However, if it does, the code
;		should be changed back to 1 character per horizontal retrace
;------------------------------------------------------------------------------
WRTSUB	PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	MOV	AH,BL			;   Yes, save attribute (if any)
	CALL	GRFXCHK 		; Graphics mode ?
	MOV	SI,SCRNLEN		; Get length of page
	JC	L09H02a 		;  * Yes, go do graphics write
;
	push	dx
	CALL	REGENADR		; Get address of cursor in REGEN memory
	ASSUME	ES:NOTHING
	pop	dx
;
;	MOV	SI,SCRNLEN		; Get length of page
	SUB	SI,BX			; Get length to end of screen,
	JNA	VOFF20	; if at very end of page, CX=0, don't write 64K words
;	
	SAR	SI,1			; Shift right to get # of words
					;  i.e. the maximum value of CX
	CMP	CX,SI			; If CX is smaller or equal OK
	JBE	WRTSU0
	MOV	CX,SI			;   Else, set CX to go to end of screen
;
WRTSU0:
	MOV	SI,AX			; Save character (and attribute if any)
	MOV	BL,CRTMODE		; Get CRT mode and prepare for test
	AND	BL,0FEh 		;   of mode 2 or mode 3

	OR	DL,DL			; Test flag for storing attribute
	MOV	DX,CRTADDR
	JNZ	HTRA20			;   If not storing attribute
	ADD	DX,CRTST-CRTCAD 	; Set DX = CRT Status Register Address
	CMP	BL,002h 		;   (80x25 alpha B/W or Color mode?)
	JNE	HTRA11			; No

HTRA5:
	IF	BLWAIT
	IN	AL,DX			; READ 6845 STATUS REGISTER
	ROR	AL,1			; Q: Screen being blanked ? 
					;  test lowest bit
	JC	HTRA5			;  * Yes, wait for it to finish
	CLI				; MASK INTERRUPTS		*A
HTRA6:
	IN	AL,DX			; READ 6845 STATUS REGISTER AGAIN
	ROR	AL,1			; Q: Screen being blanked ? 
					;  test lowest bit
	JNC	HTRA6			;  * NO, WAIT FOR IT TO START AGAIN
	ENDIF	;BLWAIT
	MOV	AX,SI			; Get value to write
	STOSW				; Write character and attribute to CRT
;
; make this time useful
;
	MOV	BP,VERWD		; This is the # of characters to move
					;   if it is a vertical retrace
	IF	BLWAIT
	CALL	VERTST			; Is this a vertical retrace time
	JZ	HTRA7			;   No, continue for next word
	ENDIF	;BLWAIT
;******************************************************************************
	SUB	CX,BP			; We're going to move this many
					;   characters
	JA	htra8
	ADD	CX,BP			; Add back all
	DEC	CX			; For previous one done
	jcxz	L09H10
	MOV	BP,001h 		; Set CX for exit value
	XCHG	CX,BP
htra8:
	XCHG	CX,BP
	STI				; ENABLE INTERRUPTS
	call	htra11			;   vertical retrace
	MOV	CX,BP			; Restore count from BP
;******************************************************************************
HTRA7:
	STI				; ENABLE INTERRUPTS
	LOOP	HTRA5			; End of count ?
L09H10:
	STI				; ENABLE INTERRUPTS
	RET
L09H02a:	jmp	short l09h02
;******************************************************************************
HTRA11:
	MOV	AX,SI			; Get value to write
	REPNZ	STOSW			; Write character and attribute to CRT
	RET				;   till finished
;******************************************************************************
;******************************************************************************
;
HTRA20:
	ADD	DX,CRTST-CRTCAD 	; Set DX = CRT Status Register Address
	CMP	BL,002h 		;   (80x25 alpha B/W or Color mode?)
	JNE	HTRA28			; No
HTRA25:
	IF	BLWAIT
	IN	AL,DX			; READ 6845 STATUS REGISTER
	ROR	AL,1			; Q: Screen being blanked ? 
					;  test lowest bit
	JC	HTRA25			;  * Yes, wait for it to finish
	CLI				; MASK INTERRUPTS		*A
HTRA26:
	IN	AL,DX			; READ 6845 STATUS REGISTER AGAIN
	ROR	AL,1			; Q: Screen being blanked ? 
					;  test lowest bit
	JNC	HTRA26			;  * NO, WAIT FOR IT TO START AGAIN
	ENDIF	;BLWAIT
	MOV	AX,SI			; Get value to write
	STOSB				; Write character to CRT
	INC	DI
;
; make this time useful
;
	MOV	BP,VERBYT		; This is the # of characters to move
					;   if it is a vertical retrace
	IF	BLWAIT
	CALL	VERTST			; Is this a vertical retrace time
	JZ	HTRA27			;   No, continue for next word
	ENDIF	;BLWAIT
;******************************************************************************
	SUB	CX,BP			; We're going to move this many
					;   characters
	JA	htra22
	ADD	CX,BP			; Add back all
	DEC	CX			; For previous one done
	jcxz	htra23
	MOV	BP,001h 		; Set CX for exit value
	XCHG	CX,BP
htra22:
	XCHG	CX,BP
	STI				; ENABLE INTERRUPTS
	call	htra28			;   vertical retrace
	MOV	CX,BP			; Restore count from BP
;******************************************************************************
htra27:
	STI				; ENABLE INTERRUPTS
	LOOP	HTRA25			; End of count ?
htra23:
	STI				; ENABLE INTERRUPTS
	RET
HTRA28:
	MOV	AX,SI			; Get value to write
HTRA29:
	STOSB				; Write character to CRT
	INC	DI
	LOOP	HTRA29
QUIKRET:
	RET				;   till finished
;------------------------------------------------------------------------------
; GRAPHICS MODE WRITE
;
;	Input:	DS = ROM data segment (0040h)
;		CX = # of times to write character
;		AL = Character to write
;		AH = Attribute to write to screen
;		SI = end of page
;	Note:  If RAM font does not exist, the character is displayed as the
;		(character-80h) with inverted foreground and background
;------------------------------------------------------------------------------
L09H02:
	ASSUME	DS:ROMDAT,ES:NOTHING
	XOR	BL,BL			; Ask for display page = 0
	CALL	RTNCURP 		; Get current cursor position
;
; Check to see if we specified too many characters to write to CRT
;
	MOV	BP,AX			; Save character and attribute in BP
	MOV	AL,DH			; Set AL to # of rows
;
	MOV	AH,40			; 40 characters per row
	MUL	AH
;
;	MOV	SI,SCRNLEN
;	MOV	SI,40*25		; Get the length of graphic screen
					;   in chars
	XOR	BH,BH			; Set BX to # of columns extended
	MOV	BL,DL			;   to 16 bits	
	SHL	DL,1			; Pretend like it has twice
					;     as many columns	
	CMP	CRTMODE,06h		; Get CRT mode and see if 640 x 200 mode
	JNE	WRITE0			; If no, then OK
	MOV	DL,BL			; Restore if it doesn't
	SHL	AX,1			; It has twice as many characters/row
;	SHL	SI,1			; Same for characters/screen
WRITE0:
	ADD	BX,AX			; Add in row offset to # of columns
	SUB	SI,BX			; Get characters to end of screen
	JNA	QUIKRET ; if at very end of screen, don't write 64K words
	MOV	AX,BP			; Restore character and attribute
	CMP	CX,SI			; If CX is smaller or equal OK
	JBE	WRITE2
	MOV	CX,SI			;   Else, set CX to go to end of screen
WRITE2:
	AND	AH,0BFh 		; Clear bit 40h of the attribute to
					;   allow testing if RAM font is there
					;   If user set this bit,he should't
					;   have, because its not defined nor
					;   is likely to be
;
	OR	AL,AL			; Using ROM Font table ?
	JNS	WRITE2A 		;   Yes
	SUB	AL,080H 		;   No, make character relative to 0
	XOR	SI,SI			; Set ES = 0 to get RAM Font table
	MOV	ES,SI			;
	ASSUME	ES:INTVEC
	LES	SI,RAMFNT		; Get RAM Font table offset address
	ASSUME	ES:NOTHING
	MOV	BX,ES			; Get contents of segment register
	OR	BX,SI			;   and test if ES and SI are both 0
	JNZ	WRITE2B 		; If no, use the RAM font
	OR	AH,40h			;  Else set flag (Bit 6 of attribute)
					;    to invert graphic character font
WRITE2A:
	MOV	SI,OFFSET ROMFNT	; Set up SI for ROM Font table
	PUSH	CS			;   and the segment is CS
	POP	ES			; ES = ROM segment address for now
	ASSUME	ES:CODE
;
WRITE2B:
	MOV	BL,AL			; BX = Character extended
	XOR	BH,BH			;   to 16 bits
	SHL	BX,1			; Multiply by 8 to get table offset
	SHL	BX,1			;  *
	SHL	BX,1			;  *
	ADD	SI,BX			; And add to SI to get start
					;   of font data
WRITE2C:
	PUSH	CX			; Save the character count
	PUSH	DX			; Save row, column
	PUSH	SI			; Save Font table character address
;
	CALL	GRFXAD			; Get address of graphic cursor in DI
;
	MOV	BP,8			; Set the # of rows per character
WRITE3:
	PUSH	AX			; Save character and attribute
	MOV	AL,ES:[SI]		; Get font byte
	TEST	AH,40h			; Test if we should invert character
					;   font
	JZ	WRITE3A 		; No
	NOT	AL			; Yes, invert the byte
WRITE3A:
	MOV	DH,AH			; Save attribute
	CMP	CRTMODE,6		; 640 X 200 mode ?
	JE	WRITE6			;  Yes
;
	MOV	CX,8
	AND	AH,00000011B		; Mask all but 2 bit color attribute
	XOR	BX,BX			; Assemble font data word in BX
WRITE4: 				; BUILD 320x200 BIT PATTERN (WORD)
	SHL	BX,1
	SHL	BX,1
	SHL	AL,1			; Left-most bit a 1 ?
	JNC	WRITE5			;  No, so this dot is off
	OR	BL,AH			;  Yes, this dot on, so include it
					;    in bit pattern
WRITE5:
	LOOP	WRITE4
	MOV	AL,BH			; Get assembled pattern in AX
	MOV	AH,BL			;   and exchange for proper storing
WRITE6:
	PUSH	ES
	MOV	ES,WORD PTR CS:B800H	; Set ES=B800H
	ASSUME	ES:NOTHING
	OR	DH,DH			; Test XOR bit 7 of attribute
	JS	WRITE6C 		;   Yes, then XOR
	CMP	CRTMODE,6		; 640 X 200 mode ?
	JE	WRITE6A 		;  Yes
	MOV	ES:[DI],AX		; Store font word
	JMP	SHORT WRITE6B
;
WRITE8A:
	JMP	SHORT WRITE2C		; Place this jump here to allow short
					;   jump
WRITE6A:
	MOV	ES:[DI],AL		; Store font byte
	JMP	SHORT WRITE6B
;
WRITE6C:
	CMP	CRTMODE,6		; 640 X 200 mode ?
	JE	WRITE6D 		;  Yes
	XOR	ES:[DI],AX
	JMP	SHORT WRITE6B
WRITE6D:
	XOR	ES:[DI],AL
WRITE6B:
	POP	ES
	ASSUME	ES:NOTHING
	ADD	DI,2000h		; Set odd scan address
	TEST	BP,01h			; Is this an odd scan ?
	JE	WRITE7			;   No
	SUB	DI,4000h-80		;   Yes, go to next even scan
WRITE7:
	POP	AX			; Restore character and attribute	
	INC	SI
	DEC	BP
	JNZ	WRITE3
;
	POP	SI			; Restore Font table character address
	POP	DX			; Get back row and column
	POP	CX			; Do another character
;
	INC	DX			; Go to next column
					;   NOTE: since we know that DL < 0FFh,
					;   we can increment DX because its
					;   faster and uses less code
	CMP	CRTMODE,06h		; Get CRT mode and see if 640 x 200 mode
	JE	WRITE9			; If Yes
	INC	DX			;   else we count columns by 2
					;   NOTE: since we know that DL < 0FFh,
					;   we can increment DX because its
					;   faster and uses less code
WRITE9:
	CMP	DL,80			; Next row ?
	JC	WRITE8			;   No
	XOR	DL,DL			;   Yes, Column = 0
	INC	DH			;     and next row
WRITE8:
	LOOP	WRITE8A
	RET
;
WRTSUB	ENDP
	PRGSIZ	<Write character & attribute subr>	; Display program size
;------------------------------------------------------------------------------
; RETURN CURSOR POSITION
;
;	Input:	DS = ROM data segment (0040h)
;		BL = REQUESTED DISPLAY PAGE
;
;	Output: DH = ROW
;		DL = COLUMN
;------------------------------------------------------------------------------
RTNCURP PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	PUSH	BX
	XOR	BH,BH			;  *
	SAL	BX,1			;  *
	MOV	DX,[CURCOOR+BX] 	; GET COLUMN/ROW
	POP	BX
	RET
RTNCURP ENDP
	PRGSIZ	<Cursor position subr>	; Display program size
;------------------------------------------------------------------------------
; SUBROUTINE TO CHECK IF IN GRAPHICS MODE
;
;	Input:	DS = ROM data segment (0040h)
;	Output: Carry Flag set if graphics mode
;------------------------------------------------------------------------------
	PUBLIC	GRFXCHK
GRFXCHK PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	CMP	CRTMODE,7		; Q: MONOCHROME
	JE	GRFXIT			;  * YES
	CMP	CRTMODE,4		; Q: GRAPHICS MODE
	CMC
GRFXIT:
	RET				; Set carry if a graphics mode
GRFXCHK ENDP
	PRGSIZ	<Graphic check subr>	; Display program size
;------------------------------------------------------------------------------
; SUBROUTINE WITH CODE COMMON TO READDOT & WRTDOT
;
;	Input:	DS = ROM data segment (0040h)
;		DX = ROW NUMBER (MODE DEPENDENT)
;		CX = COLUMN NUMBER (MODE DEPENDENT)
;	Output: BX = OFFSET INTO REGEN MEMORY
;		CL = SHIFT COUNT TO ACCESS PEL IN BYTE AT ES:[BX]
;		CH = PEL MASK
;		ES = SEG ADDR TO REGEN MEMORY
;		Carry Flag set if error
;------------------------------------------------------------------------------
DOTSUB	PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	CMP	DX,200			; Q: ROW NUMBER IN RANGE
	JNC	DOTXIT			;  * NO, EXIT
	MOV	BX,640
	MOV	AH,CRTMODE
	CMP	AH,6			; 640x200 B/W MODE ?
	JZ	HIRES			;  * YES, HI RES B/W MODE
	JA	DOTXIT			;   AH >= Mode 7, then error
;
	CMP	AH,4			; Is mode a 320 x 200 mode ?
	JC	DOTXIT1 		;   No, it's an alphanumeric mode
	SHR	BX,1			;   Yes, then half as many columns
HIRES:					; 
	CMP	CX,BX			; Column # in range ?
	JNC	DOTXIT			;   No, then exit with carry set
					;   Yes, then compute mask and shift
	XCHG	SI,AX			; Save AX in SI
	MOV	ES,WORD PTR CS:B800H	; SET SEG ADDR TO REGEN MEMORY
	ASSUME	ES:NOTHING
;
	XOR	BX,BX			; Set to an even scan
	ROR	DX,1			; Was it really an odd scan ?
	JNC	GRFAD0			;   No
	MOV	BH,020h 		; Yes, then set offset to odd scans
GRFAD0:
	SHL	DX,1			; Strip the odd bit
	MOV	AX,DX			; Multiply the (even row #) * 40
					;   which is really saying 80 bytes
					;   per row
	SHL	AX,1			; Multiply AX times 4
	SHL	AX,1
	ADD	AX,DX			; Add to AX making AX = 5 * (row #)
	SHL	AX,1			; Multiply AX times 8 giving
	SHL	AX,1
	SHL	AX,1			;   AX = 40 * (row #)
	ADD	BX,AX			; Add to offset
;
	MOV	AX,CX			; Get column #
	SHR	AX,1			; Get column memory offset
	SHR	AX,1			;  "     "     "      "
	NOT	CL			; Get the shift count which is the
					;   complement because dot 0 is bit 7
	CMP	CRTMODE,6		; 640x200 mode ?
	JNE	GRFAD1			;   No
	SHR	AX,1			; Shift 1 more time for memory offset
	MOV	DL,01h			; DL = mask for 640 mode
	JMP	SHORT GRFAD2		;   Yes
GRFAD1:
	SHL	CL,1			; Shift 1 more time for 2 bit/dots
	MOV	DL,03h			; DL = mask for 320 mode
;
GRFAD2:
	ADD	BX,AX			; Add in column memory offset
	AND	CL,07h
	XCHG	SI,AX			; Restore AX from SI
	RET
;
DOTXIT:
	STC				; Set carry to indicate a flag
DOTXIT1:
	RET				; 
DOTSUB	ENDP
	PRGSIZ	<Pixel address and bit subr>	; Display program size
