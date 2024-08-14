	NAME	PRTSCR
	TITLE	Print Screen Routine
	PAGE	59,132
;******************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program       *
;* contains proprietary and confidential information.  All rights reserved    *
;* except as may be permitted by prior written consent. 		      *
;******************************************************************************
;
; ROM Print Screen Routine
;	This module is invoked by a software INT 05h or by the keyboard ISR
; when a Shift-PrtSC is done.  It will then print the ASCII contents of the
; screen to Printer device # 0
;
;	Written: 10/21/83
;	Revised: 04/11/84 (DLK) - Any Character Except Null is printed
;		   /  /84 Ira J. Perlow - fixed and squeezed code
;		 03/18/85 Ira J. Perlow
;		 06/20/85 Ira J. Perlow - compressed code with tricks
;
;	Updated: 08/18/86 Torsten Burgdorff/CBM BSW 
;			  Check printer status before print any character
;			  Exit with PRTFLG=FF, if IO- or Paper-out-error
;
;------------------------------------------------------------------------------
;
INCLUDE ROMOPT.INC			; ROM Options file - must be included 1st
;
INCLUDE ROMEQU.INC			; ROM General equate file
INCLUDE MACROS.INC			; ROM Macros file
;
NOPAPER        EQU	 20H		; Status information
IO_ERROR       EQU	 08H		;
;							
;------------------------------------------------------------------------------
;
	OUT1	<PRTSCR - ROM Print Screen code>
;
ROMDAT	SEGMENT AT 0040h		;The following are relative to Segment 40h
	EXTRN	PRTFLG:BYTE		; Flag indicating Print Screen in
					;   progress if a 1, else not
	EXTRN	RSTFLG:WORD
	EXTRN	DEVFLG:WORD
ROMDAT	ENDS
;
;------------------------------------------------------------------------------
;
CGROUP	GROUP	CODE
CODE	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
;
	EXTRN	SETDS40:NEAR		; Set DS to ROM data segment (0040h)
;
;------------------------------------------------------------------------------
;
; PRINT SCREEN ENTRY POINT VIA S/W INT 5
;
;------------------------------------------------------------------------------
	SYNC	PRTSCRA
;
	PUBLIC	PRTSCR			; Print screen to printer
PRTSCR	PROC	FAR
	ASSUME	DS:NOTHING,ES:NOTHING
	PUSH	AX
	PUSH	DS
;
	CALL	SETDS40 		; Set DS to the ROM data segment
	ASSUME	DS:ROMDAT
;
	MOV	AL,1			; Flag indicating Print Screen in
					;   progress
	XCHG	AL,PRTFLG		; Exchange with current status.  Either
					;   way we're going to be printing a
					;   screen
	DEC	AL			; Test if flag was set to 1
	JZ	PRTSC4			;   Yes, then we were already printing
	STI				;   No, Open interrupts
	PUSH	BX			;     and begin printing
	PUSH	CX
	PUSH	DX
;
	MOV	AH,2			; Get printer status information 
	MOV	BL,0FFH 		; Set error flag
	XOR	DX,DX			; Select 1st printer
	INT	17H			; Get status	  
	TEST	AH,NOPAPER+IO_ERROR	; Any errors ?
	JNZ	PRTEXIT 		; YES -> Exit

	MOV	AH,0FH			; Go get # of CRT columns,
					;   and current page in BH
	INT	10H
	MOV	BL,AH			; BL = Screen width
;
	MOV	AH,03H			; Read cursor position
	INT	10H
	PUSH	DX			; Save cursor position
;
	MOV	CL,25			; Set CL to # of screen lines
	MOV	DH,0FFh 		; Begin with cursor location (0,0)
	JMP	SHORT PRTSC2A		; Print initial CR,LF
;
PRTSC0:
	MOV	AH,02H			; Position cursor
	INT	10H
;
	MOV	AH,08H			; Read character from display
	INT	10H
;
	IF	ROMSIZ LE (8*1024)
	    OR	    AL,AL		; null?
	    JZ	    PRTSC1
	    CMP     AL,0Ah
	    JB	    PRTSC2
	    CMP     AL,0Dh
	    JA	    PRTSC2		; also gates out 0B (V form feed)
PRTSC1:
	    MOV     AL,' '		;   No, convert to space
PRTSC2:
	ELSE				;ROMSIZ
	    SUB     AL,LF		; Is it a line feed ?
	    JZ	    PRTSC1		;   Yes, then print space
	    DEC     AX			;   No, is it a 0Bh
					;     (No carry will effect AH because
					;      AL<>0 from above)
	    JZ	    PRTSC2		;   Yes, then print it
	    CMP     AL,CR-(LF+1)	; Is it above a CR or below a LF ?
	    JA	    PRTSC2		;   Yes, then print it
					;   No, then it's a FF or CR,
					;     print a space
PRTSC1:
	    MOV     AL,' '-(LF+1)	;   No, convert to space
PRTSC2:
	    ADD     AL,LF+1
	    JZ	    PRTSC1		; Test for a null and do strange loop
	ENDIF				;ROMSIZ ELSE
;
	CALL	PRINT			; Print the character
	JNZ	PRTSC3			; If error, go to error exit
	INC	DX			; Increment column in DL, use 16 bit
					;   increment because it is faster and
					;   uses less code
	CMP	DL,BL			; End of row ?
	JC	PRTSC0			;   No
;
PRTSC2A:


	CALL	PRTCRLF 		; Go print CR/LF
	JNZ	PRTSC3			; If error, go to error exit
;
	MOV	DL,0FFh 		; Put 0FFh in DL to force column 0
					;   and next row for increment below
	INC	DX			; Increment to next column
;
	CMP	DH,CL			; Past end of screen ?
	JC	PRTSC0			;   No
	XOR	BL,BL			;   Yes, then we printed screen OK
					;     Set BL = 0 for status byte
PRTSC3:
	POP	DX			; Recall former cursor position
	MOV	AH,02H			;   and restore to CRT
	INT	10H			;
PRTEXIT:
	MOV	PRTFLG,BL		; Save printer status byte
;
	POP	DX
	POP	CX
	POP	BX
PRTSC4:
	POP	DS
	ASSUME	DS:NOTHING
	POP	AX
	IRET
;
PRTSCR	ENDP
	PRGSIZ	<Print screen>		; Display program size
;------------------------------------------------------------------------------
;
; Print Carriage Return, Line Feed to Printer Subroutine
;
;	Input:	AL = Character to print
;	Output: Zero Flag = 1 if good print, else 0 if error
;		AH = Printer status error bits
;		BL = 0FFh if error
;
;------------------------------------------------------------------------------
	PUBLIC	PRTCRLF 		; Make public for other graphic boards
PRTCRLF PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	MOV	AL,CR			; Print a carriage return
	CALL	PRINT
	JNZ	CRLFERR 		; If error, exit with flags
	MOV	AL,LF			; Print a line feed by falling through
					;   to Print character subroutine
;
;------------------------------------------------------------------------------
;
; Print Character to Printer Subroutine
;
;	Input:	None
;	Output: BL = 0FFh if error
;		AH = Printer status error bits
;		Zero Flag = 1 if good print, else 0 if error
;
;------------------------------------------------------------------------------
	ASSUME	DS:NOTHING,ES:NOTHING
	PUBLIC	PRINT			; Make public for other graphic boards
PRINT:
	PUSH	DX
	XOR	AH,AH			; Print character function
	XOR	DX,DX			; Select 1st printer
	INT	17H			; Print character
	POP	DX
	AND	AH,09h			; Mask all but error bits
					; Print successful ? (AH = 0)
	JZ	CRLFERR 		;   Yes
	MOV	BL,0FFh 		;   No
CRLFERR:
	RET
PRTCRLF ENDP
	PRGSIZ	<Print CR,LF and Print char to LP subr> ; Display program size
;------------------------------------------------------------------------------
	IF	ROMSIZ LE (8*1024)
	    PNXMAC3			; Include PRTCHK code
	ENDIF
;
;------------------------------------------------------------------------------
; ROM version # for even/odd ROMS stored in BCD
;   Version #/100 = byte
;------------------------------------------------------------------------------
	SYNCA	POWRUPA-6
	PUBLIC	ROMVER
ROMVER	DB	13h,13h 		; Even/Odd ROM version # LSB
	DB	02h,02h 		; Even/Odd ROM version # MSB
	PRGSIZ	<ROM even/odd Version #> ; Display program size
;------------------------------------------------------------------------------
; ROM checksum bytes for even/odd ROMS
;------------------------------------------------------------------------------
	SYNCA	POWRUPA-2
	PUBLIC	ROMCKS
ROMCKS	DB	00h			; Even ROM checksum
	DB	00h			; Odd  ROM checksum
	PRGSIZ	<ROM even/odd checksum bytes> ; Display program size
;------------------------------------------------------------------------------
;
	SYNC	POWRUPA
;
	TOTAL
;
CODE	ENDS
	END

