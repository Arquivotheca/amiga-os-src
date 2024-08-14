	NAME	CRTDSR
	TITLE	ROM CRTDSR
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program	   *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent.			   *
;***************************************************************************
;
; CRT ROM DEVICE SERVICE ROUTINE
;	THIS MODULE CONTAINS THE DEVICE SERVICE ROUTINE FOR
;	THE CRT.  FUNCTIONS INCLUDE VIDEO MODE
;	CONTROL, CHARACTER HANDLING, GRAPHICS INTERFACE, AND
;	ASCII TTY OUTPUT.  ACCESS TO THE CRTDSR IS VIA S/W
;	INTERRUPT 10H.	FUNCTIONS SUPPORTED:
;
;		00H - SET VIDEO MODE
;		01H - SET CURSOR TYPE
;		02H - SET CURSOR POSITION
;		03H - READ CURSOR POSITION
;		04H - READ LIGHT PEN POSITION
;		05H - SELECT ACTIVE DISPLAY PAGE
;		06H - SCROLL ACTIVE PAGE UP
;		07H - SCROLL ACTIVE PAGE DOWN
;		08H - READ ATTR/CHAR AT CURSOR
;		09H - WRITE ATTR/CHAR AT CURSOR
;		0AH - WRITE CHAR AT CURSOR
;		0BH - SET COLOR PALETTE
;		0CH - WRITE DOT
;		0DH - READ DOT
;		0EH - WRITE TTY
;		0FH - GET CURRENT VIDEO STATE
;		10h - (JR) Set palette registers	(not implemented)
;		11h - EGA Character Generator functions	(not implemented)
;		12h - EGA Alternate function select	(not implemented)
;		13h - (AT) Display string
;
;	WRITTEN: 09/26/83
;	REVISED: mm/dd/yy ** (initials)
;		 02/28/84 *A (DLK)
;		  - FIX BAD CMD EXIT
;		  - CORRECTLY SAVE ROW & COL IN CURCOOR TABLE
;		  - CHANGE STACK HANDLING IN WRITE CHAR & CHAR W/ATTR SUBS
;		  - CHANGE STACK HANDLING ON SCROLL
;		  - PERMIT SCROLL OF 0 LINES (TO BLANK OUT A LINE)
;		 03/16/84 *B (DLK)
;		  - DO NOT INITIALIZE "SCRNLEN" IN MONOCHROME MODE
;		 03/22/84 *C (DLK)
;		  - MAKE BIT 5 OF MSR A "1" FOR MODES 4 & 5
;		  - INITIALIZE COLOR-SELECT REG DIFFERENTLY
;		  - REVERSE STORAGE OF CURSOR TYPE IN "CURTYPE"
;		  - USE CURRENT "DSPYPAG" DURING WRITE-TTY
;		  - DO NOT CHECK MODE ON SET PALETTE CALL
;		 03/30/84 *D (DLK)
;		  - REALLY FIX SAVING OF CURSOR TYPE IN "CURTYPE"
;		  - REALLY FIX CURRENT "DSPYPAG" BUG IN WRITE-TTY
;		 04/01/84 *E (DLK)
;		  - FIX "PAGADDR" (CURSOR NOT SHOWN WHEN PAGE <> 0)
;		  - THIS TIME REALLY FIX CURRENT "DSPYPAG" BUG IN WRITE-TTY
;		 04/03/84 *F (DLK)
;		  - REALLY FIX "PAGADDR" (CURSOR NOT SHOWN WHEN PAGE <> 0)
;		 04/10/84 *G (DLK)
;		  - FIX CLEARING CURSOR COORDINATE TABLE
;		 6/85 stan rewrote functions 0E and 13 to use INT 10s - some
;			applications intercept only lower-level functions
;		 6/85 stan changed to get CRT params for screen size, width, &
;			MSR value directly from ROM rather than indirectly from
;			tables at [0:4*1Dh]
;	CODE REDUCTION: mm/dd/yy *n (initials)
;		 04/04/84 *1,*2,*3 (DLK)
;		  - FIRST THREE PASSES
;		 04/05/84 *4 (DLK)
;		  - FOURTH PASS
;
;------------------------------------------------------------------------------
;
INCLUDE	ROMOPT.INC		; ROM Options file - must be included 1st
INCLUDE	ROMEQU.INC		; ROM General equate file
INCLUDE	MACROS.INC		; ROM Macros file
	IF	I80286
	.286P
	ENDIF
;------------------------------------------------------------------------------
;
	OUT1	<CRTDSR - ROM CRT Device Service Routines>
;
;------------------------------------------------------------------------------
;
INTVEC	SEGMENT AT 0000h	; The following are relative to Segment 00h
	EXTRN	CRTCINT:DWORD		; Offset of pointer CRTC INIT table
	EXTRN	RAMFNT:DWORD		; Offset of user generated font table
INTVEC	ENDS
;
;------------------------------------------------------------------------------
;
DGROUP	GROUP	ROMDAT
ROMDAT	SEGMENT AT 0040h	;The following are relative to Segment 40h
	EXTRN	DEVFLG:WORD		; DEVICE FLAGS
	EXTRN	CRTMODE:BYTE		; CURRENT CRT MODE
	EXTRN	SCRNWID:WORD		; SCREEN COLUMN WIDTH
	EXTRN	SCRNLEN:WORD		; BYTE LENGTH OF SCREEN
	EXTRN	PAGADDR:WORD		; ADDR OF DISPLAY PAGE
	EXTRN	CURCOOR:WORD		; START OF CURSOR COORDINATES
	EXTRN	CURTYPE:WORD		; CURRENT CURSOR TYPE
	EXTRN	DSPYPAG:BYTE		; CURRENT DISPLAY PAGE
	EXTRN	CRTADDR:WORD		; I/O ADDR OF CRT CONTROLLER
	EXTRN	MSRCOPY:BYTE		; MODE-SELECT REGISTER COPY
	EXTRN	PALETTE:BYTE		; CURRENT COLOR PALETTE
ROMDAT	ENDS
;
;------------------------------------------------------------------------------
;
CGROUP	GROUP	CODE
CODE	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
;
;	EXTRN	VERTST:NEAR		; Vertical Retrace test
	EXTRN	ROMFNT:BYTE		; ROM FONT TABLE
;
	EXTRN	BEEP:NEAR		; SOUND THE SPEAKER
	EXTRN	COMXITA:NEAR		; Common DSR return code with A register
	EXTRN	COMEXIT:NEAR		; COMMON DSR EXIT ROUTINE	*6
	EXTRN	RNGCHK:NEAR		; CHECK OPCODE RANGE		*4
	EXTRN	SAVREGS:NEAR		; SAVE REGISTERS		*6
;
	SYNC	CRTVECA
;
CMDXFER DW	SETVIDO		; SET VIDEO MODE
	DW	SETCURT		; SET CURSOR TYPE
	DW	SETCURP		; SET CURSOR POSITION
	DW	READCURP 	; READ CURSOR POSITION
	DW	RDLGHT		; READ LIGHT PEN POSITION
	DW	SELPAG		; SELECT ACTIVE DISPLAY PAGE
	DW	ROLLUP		; SCROLL SCREEN UP
	DW	ROLLDOWN 	; SCROLL SCREEN DOWN
	DW	RDCHRATT 	; READ CHAR & ATTRIBUTE
	DW	WRTCHRATT	; WRITE CHAR & ATTRIBUTE
	DW	WRTCHR		; WRITE CHAR ONLY
	DW	SETPALET 	; SET COLOR PALLETTE
	DW	WRTDOT		; WRITE DOT
	DW	READDOT		; READ DOT
	DW	WRTTTY		; WRITE TTY
	DW	RDVIDOST 	; READ VIDEO STATE
;
;
XFEREND EQU	$-CMDXFER
	PRGSIZ	<CRT Routine vectors>	; Display program size
;------------------------------------------------------------------------------
; CRTDSR ENTRY POINT VIA S/W INT 10H
;
;	AH = OPCODE (00H - 0FH)
;------------------------------------------------------------------------------
	SYNC	CRTDSRA
;
	PUBLIC	CRTDSR
CRTDSR	PROC	FAR
	ASSUME	DS:NOTHING,ES:NOTHING
	CALL	SAVREGS 		; SAVE CALLER'S REGISTERS
	MOV	DI,OFFSET XFEREND-2	; DI=LENGTH OF CMD JUMP TABLE
	CALL	RNGCHK			; GO CHECK OPCODE
	ASSUME	DS:ROMDAT
	JC	EXITX			; JUMP ON RANGE ERROR
	CLD				; Clear direction flag
	JMP	WORD PTR CS:[DI+OFFSET CMDXFER]  ; JUMP TO PROCESSOR
CRTDSR	ENDP
	PRGSIZ	<CRT Routines entry code>	; Display program size
;------------------------------------------------------------------------------
; This routine reads a single character and attribute from the screen if in a
; character mode.  Outputs Attribute in AH and Character in AL.
;------------------------------------------------------------------------------
RDCHR	PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	PUSH	BX
	CALL	REGENADR		; GO FIND ADDR IN REGEN MEMORY
	MOV	AL,CRTMODE		; Get CRT mode
	AND	AL,0FEh			;   and test for mode 2 or mode 3
	CMP	AL,002h			;   (80x25 alpha B/W or Color mode?)
	MOV	DX,CRTADDR
	PUSH	DS
	PUSH	ES
	POP	DS
	ASSUME	DS:NOTHING
	JNE	RDCHR2			; No
	ADD	DX,06h			;  * YES, DX=STATUS REG ADDR	*A
RDCHR0:
	IN	AL,DX			; READ 6845 STATUS REGISTER
	ROR	AL,1			; Is CRT blanked in horizontal or
					;   vertical retrace ?
	JC	RDCHR0			;  * YES
	CLI				; MASK INTERRUPTS		*A
RDCHR1:
	IN	AL,DX			; READ 6845 STATUS REGISTER AGAIN
	ROR	AL,1			; Is CRT blanked in horizontal or
					;   vertical retrace ?
	JNC	RDCHR1			;  * NO, WAIT FOR IT TO START AGAIN
RDCHR2:
	MOV	AX,[DI]			; GET CHARACTER & ATTRIBUTE
	STI				; ENABLE INTERRUPTS
	POP	DS
	ASSUME	DS:ROMDAT
	POP	BX
	RET
RDCHR	ENDP
	PRGSIZ	<Read character without screen hash>	; Display program size
;------------------------------------------------------------------------------
CRTDSR1	PROC	FAR
	ASSUME	DS:NOTHING,ES:NOTHING
	IF	LIGHTPN
; Since this is an option that many do not want, we include it in another
; module since there is free space elsewhere and there is no room for it
; in this module
	EXTRN	RDLGHT:NEAR		; READ LIGHT PEN POSITION
	ELSE
RDLGHT:					; Label to use if no light pen
	XOR	AH,AH			; Return AH = 0 meaning no trigger
	ENDIF
EXITX:
	JMP	COMEXIT 		; GO TO COMMON DSR EXIT
					; Locate here so some short jumps
					;   can reach it
	PRGSIZ	<Common exit jump vector>	; Display program size
;------------------------------------------------------------------------------
; CRT Controller parameters
;------------------------------------------------------------------------------
	SYNC	CRTPRMA
	PUBLIC	CRTPRM
CRTPRM	LABEL	BYTE
	DB	38h,28h,2Dh,0Ah,1Fh,06h,19h,1Ch		; mode 0,1
	DB	02h,07h,06h,07h,00h,00h,00h,00h
;
	DB	71h,50h,5Ah,0Ah,1Fh,06h,19h,1Ch		; mode 2,3
	DB	02h,07h,06h,07h,00h,00h,00h,00h
;
	DB	38h,28h,2Dh,0Ah,7Fh,06h,64h,70h		; mode 4,5,6
	DB	02h,01h,06h,07h,00h,00h,00h,00h
;
	DB	61h,50h,52h,0Fh,19h,06h,19h,19h		; mode 7
	DB	02h,0Dh,0Bh,0Ch,00h,00h,00h,00h
;
; Amount of memory used for mode rounded up to 1k boundary
	PUBLIC	CRTPRM_SIZ
CRTPRM_SIZ LABEL WORD
	DW	2*1024	; Mode 0,1
	DW	4*1024	; Mode 2,3
	DW	16*1024	; Mode 4,5,6
	DW	16*1024	; Mode 7
;
; # of CRT columns
	PUBLIC	CRTPRM_COL
CRTPRM_COL LABEL BYTE
	DB	40	; Mode 0
	DB	40	; Mode 1
	DB	80	; Mode 2
	DB	80	; Mode 3
	DB	40	; Mode 4
	DB	40	; Mode 5
	DB	80	; Mode 6
	DB	80	; Mode 7
;
; Mode Select Register value
	PUBLIC	CRTPRM_MSR
CRTPRM_MSR LABEL BYTE
	DB	BLINK+VIDEN+BLKWHT		; MODE 0 DATA
	DB	BLINK+VIDEN			; MODE 1 DATA
	DB	BLINK+VIDEN+BLKWHT+ALPH80	; MODE 2 DATA
	DB	BLINK+VIDEN+ALPH80		; MODE 3 DATA
	DB	BLINK+VIDEN+GRF320		; MODE 4 DATA
	DB	BLINK+VIDEN+BLKWHT+GRF320	; MODE 5 DATA
	DB	GRF640+VIDEN+BLKWHT+GRF320	; MODE 6 DATA
	DB	BLINK+VIDEN+ALPH80		; MODE 7 DATA
	PRGSIZ	<CRT Mode Parameters>		; Display program size
;
;------------------------------------------------------------------------------
	PUBLIC	B000H
B000H	DW	0B000H
	PRGSIZ	<Monochrome CRT Segment register values> ; Display program size
;------------------------------------------------------------------------------
	PUBLIC	B800H
B800H	DW	0B800H
	PRGSIZ	<Graphic CRT Segment register values>	; Display program size
;------------------------------------------------------------------------------
; SET VIDEO MODE (AH=00H)
;
;	INPUT:	AL = VIDEO MODE (0-7)
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
SETVIDO:
	MOV	AH,BYTE PTR DEVFLG	; GET DEVICE FLAGS
	AND	AH,30H			; MASK OFF ALL BUT DISPLAY INFO
	CMP	AH,30H			; Q: MONOCHROME DISPLAY IN USE
	JNE	SETVI0			;  * NO
	MOV	AL,7			;  * YES, MUST REMAIN MONOCHROME
SETVI0:
	CMP	AL,7			; Mode valid ?
	JA	EXITX			;   No
	MOV	CRTMODE,AL		; Save mode, whatever it is
;
	MOV	AH,AL			; Put mode in AH
;
	MOV	DX,COLORCTL		; DX=BASE I/O ADDR OF CONTROLLER
	MOV	AL,BLINK+BLKWHT+GRF640+ALPH80	; AH = MSR value to disable video
					; Having the GRF640 and ALPH80 bits
					;   seems to allow the video to stay in
					;   vertical sync better so we do it
					;   this way
	JNE	SETVI1			; It was a graphic card mode
	MOV	DL,LOW (CRTCTL)		; DX = Base I/O address of monochrome
	MOV	AL,BLINK+ALPH80		; AL = MSR value to disable video
					; Having the ALPH80 bit
					;   seems to allow the video to stay in
					;   vertical sync better so we do it
					;   this way
SETVI1:
	MOV	CRTADDR,DX		; SAVE I/O ADDRESS
;
	XOR	SI,SI			; Set ES segment to 0
	MOV	PAGADDR,SI		; Initialize display page offset to 0
	MOV	ES,SI			; 
	ASSUME	ES:INTVEC
	LES	SI,ES:CRTCINT		; DI = Segment offset to CRTC init table
					; ES = Segment address of table
	ASSUME	ES:NOTHING
;
	XOR	BX,BX			; Set offset for memory size used
	CMP	AH,02h			; Is it mode 0 or 1 ?
	JC	SETVI2			;   Yes
	ADD	BL,02h
	ADD	SI,10h			; Next table line
;
	CMP	AH,04h			; Is it mode 2 or 3 ?
	JC	SETVI2			;   Yes
	ADD	BL,02h
	ADD	SI,10h			; Next table line
;
	CMP	AH,07h			; Is it mode 7 ?
	JNE	SETVI2			;   No
	ADD	BL,02h
	ADD	SI,10h			; Next table line
SETVI2:
;	
; MONITOR TYPE DETERMINED -- PROCEED WITH INITIALIZATION
;
;    AH = MSF REGISTER VALUE TO DISABLE VIDEO
;    DX = BASE I/O ADDRESS OF CRT CONTROLLER
;
	PUSH	DX
	ADD	DX,MSRADR-CRTCAD	; DX=ADDR OF MODE CONTROL REGISTER
	OUT	DX,AL			; DISABLE VIDEO
	POP	DX
;
;
; LOOP TO INITIALIZE CRTC FOR NEW MODE
;
	MOV	CX,16			; Set CX to do 16 registers
	ADD	SI,CX			; Offset to end of registers entries
	MOV	DSPYPAG,CH		; Initialize current page # to 0
;
L00H10:
	MOV	AL,CL			; AL=CRTC REGISTER #
	DEC	AL
	OUT	DX,AL			; SELECT REGISTER
;
	INC	DX			; DX=CRTC DATA REGISTER
	DEC	SI
	MOV	AL,BYTE PTR ES:[SI]	; GET TABLE VALUE
	OUT	DX,AL			; PROGRAM THE CTRC
	DEC	DX
;
	LOOP	L00H10			; Repeat for all 16 registers
;
	MOV	CURTYPE,0607H		; INITIALIZE CURSOR TYPE
;
	MOV	CX,WORD PTR CS:[BX+OFFSET CRTPRM_SIZ]	; GET TABLE VALUE
	MOV	SCRNLEN,CX		; Initialize screen length
;
	MOV	BL,AH			; Set BX to mode index
	MOV	CL,BYTE PTR CS:[BX+OFFSET CRTPRM_COL]	; GET TABLE VALUE
	XOR	CH,CH
	MOV	SCRNWID,CX		; INITIALIZE SCREEN WIDTH	*6
;
	MOV	AL,BYTE PTR CS:[BX+OFFSET CRTPRM_MSR]	; GET TABLE VALUE
	MOV	MSRCOPY,AL		; SAVE MODE CONTROL REGISTER DATA
;
; CLEAR SCREEN REGEN MEMORY
;
	MOV	AX,07*256+' '		; ATTR/CHAR USED TO CLEAR SCREEN
	CMP	CRTMODE,4		; Q: GRAPHICS MODE
	JL	L00H14			;  * NO
	XOR	AX,AX			;  * YES, CLEAR ALL BITS IN REGEN MEM
L00H14:
	MOV	BX,0B800H		; BX=ADDR OF COLOR REGEN MEMORY
	MOV	CX,2000H		; CX=LENGTH OF REGEN MEMORY (IN WORDS)
	CMP	CRTMODE,7		; Q: COLOR ADAPTER
	JNE	L00H20			;  * YES
	MOV	AX,07*256+' '		; ATTR/CHAR USED TO CLEAR SCREEN
	MOV	BH,HIGH (0B000h)	; BX=ADDR OF MONO REGEN MEMORY
	MOV	CH,HIGH (00800h)	; CX=LENGTH OF REGEN MEMORY (IN WORDS)
L00H20:
	MOV	ES,BX			; SET SEGMENT REGISTER
	ASSUME	ES:NOTHING
	XOR	DI,DI						       ;*6
	REP	STOSW			; CLEAR REGEN MEMORY		*3
;
; CLEAR CURSOR ROW/COL TABLE
;
	MOV	AX,DS
	MOV	ES,AX			; SET ES=DS
	ASSUME	ES:ROMDAT
	XOR	AX,AX
	MOV	DI,OFFSET CURCOOR	; DI = address of cursor table
	MOV	CX,8						       ;*G
	REP	STOSW			; CLEAR CURSOR TABLE		*3
;
; Set up CSR
;
	ADD	DX,CSRADR-CRTCAD	; DX=I/O ADDRESS OF CSR
	MOV	AL,SELCOL+ALTCOL	; CSR for all modes but 6
;
	CMP	CRTMODE,6		; 640 x 200 graphics mode
	JA	L00H30			;   No, it's Monochrome mode
	JL	L00H27			;   No, it's another mode < 6
	MOV	AL,SELCOL+ALTCOL+INTENS+RED+GREEN+BLUE	; CSR=03FH FOR MODE 6
L00H27:
	OUT	DX,AL			; INITIALIZE CSR
L00H30:
	MOV	PALETTE,AL		; Initialize palette
;
; INITIALIZE MODE-SELECT REGISTER (MSR) & ENABLE VIDEO
;
	DEC	DX			; RESTORE I/O ADDR OF MSR
	MOV	AL,MSRCOPY		; AL=VALUE FOR MSR
L00H35:
	OUT	DX,AL			; PROGRAM THE CRTC MODE REGISTER
	JMP	SHORT SETCUR0		; GO TO COMMON DSR EXIT 	*6
;------------------------------------------------------------------------------
; SET COLOR PALETTE (AH=0BH)
;
;   INPUT:	BH = 0 FOR BACKGROUND COLOR IN BL
;		     1 FOR COLOR SET NUMBER IN BL
;		BL = BITS 0-4 IF BH=0
;		     0 FOR COLOR SET GREEN/RED/YELLOW IF BH=1
;		     1 FOR COLOR SET CYAN/MAGENTA/WHITE IF BH=1
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
SETPALET:
	MOV	DX,CRTADDR
	ADD	DX,CSRADR-CRTCAD	; DX=COLOR-SELECT REGISTER
	MOV	AL,PALETTE		; GET CURRENT PALETTE VALUE
	OR	BH,BH			; Q: SETTING BACKGROUND COLOR (=0)
	JNZ	L0BH5			;  * NO
	AND	AL,11100000B		;  * YES
	OR	AL,BL			; PUT NEW BACKGROUND COLOR IN AL
	JMP	SHORT L0BH10
L0BH5:
	AND	AL,NOT SELCOL		; MASK ALL BUT ACTIVE COLOR SET BIT
	OR	BL,BL			; Q: SELECTED GREEN/RED/YELLOW (=0)
	JE	L0BH10			;  * YES
	OR	AL,SELCOL		;  * NO, ENABLE CYAN/MAGENTA/WHITE BIT
L0BH10:
	MOV	PALETTE,AL		; SAVE IT
	JMP	SHORT L00H35		; SET NEW COLOR-SELECT REGISTER
					;   and go to common DSR exit
;------------------------------------------------------------------------------
; SET CURSOR TYPE (AH=01H)
;
;   INPUT:	CH = START LINE OF CURSOR (BITS 0-4)
;		     CURSOR CONTROL OPERATION (BITS 5-6)
;			00 = NON-BLINK
;			01 = DON'T DISPLAY CURSOR
;			10 = BLINK @ 1/16 FIELD RATE
;			11 = BLINK @ 1/32 FIELD RATE
;		CL = END LINE OF CURSOR (BITS 0-4)
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
SETCURT:
	MOV	BX,CURSTRT*256+CUREND	; BH=CRTC R10			*3
					; BL=CRTC R11			*3
	CALL	WRTCRT 		; WRITE CURSOR TYPE TO 6845	*3
;
	MOV	CURTYPE,CX		; SAVE CURSOR TYPE
SETCUR0:
	JMP	COMEXIT			; GO TO COMMON DSR EXIT 	*6
;------------------------------------------------------------------------------
; READ CURSOR POSITION (AH=03H)
;
;   INPUT:	BH = ACTIVE DISPLAY PAGE
;			(ignored and set to 0 if graphics or monochrome mode)
;
;   RETURNED:	DH = ROW LOCATION OF CURSOR
;		DL = COLUMN LOCATION OF CURSOR
;		CX = CURSOR TYPE
;
;   OUTPUT:	AX = Undefined (however we return it unchanged)
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
READCURP:
	CMP	CRTMODE,4		; Q: COLOR ADAPTER ALPHA MODE
	JC	RDCUR0			;  * YES
	XOR	BH,BH			;  * NO, DISPLAY PAGE = 0
RDCUR0:
	MOV	BL,BH			; BL=REQUESTED DISPLAY PAGE
	CALL	RTNCURP 		; GET CURSOR POSITION
	MOV	BP,SP
	MOV	[BP+10],DX		; PUT INTO STACK FOR LATER POP
	MOV	DX,CURTYPE		; GET CURSOR TYPE
	MOV	[BP+12],DX		; PUT INTO STACK FOR LATER POP
	JMP	SHORT SETCUR0		; GO TO COMMON DSR EXIT 	*6
;------------------------------------------------------------------------------
; READ LIGHT PEN (AH=04H)
;
;   INPUT:	None
;   OUTPUT:	AH = 0 if light pen not triggered, 1 if it is
;		DH = Character row of light pen
;		DL = Character column of light pen
;		CH = Pixel row
;		BX = Pixel column, best estimate
;------------------------------------------------------------------------------
	IF	LIGHTPN
; Since this is an option that many do not want, we include it in the
; INITCD.ASM module since that is free space area and there is no room for it
; in this module
	ENDIF
;------------------------------------------------------------------------------
; SELECT ACTIVE DISPLAY PAGE (AH=05H)
;
;   INPUT:	AL = NEW ACTIVE DISPLAY PAGE
;   OUTPUT:	AX = (?)
;------------------------------------------------------------------------------
SELPAG:
	ASSUME	DS:ROMDAT,ES:NOTHING
	CMP	CRTMODE,4		; Q: COLOR ADAPTER ALPHA MODE
	JNC	SELPA1			;  * NO, JUST RETURN
					;  * YES
	MOV	DSPYPAG,AL		; SAVE NEW DISPLAY PAGE
;
	XOR	AH,AH		; AX = page # extended
	MOV	BX,SCRNLEN	; BX=Page length in bytes
	XOR	CX,CX		; Clear product
;
	INC	AX		; CX = BX * AX
	SUB	CX,BX		
SELPA0:		;Loop time is 13*(page+1)+4
	ADD	CX,BX		
	DEC	AX
	JNZ	SELPA0
;
	MOV	PAGADDR,CX	; Save page address
	SAR	CX,1		;   and convert copy in CX to word address
;
; PROGRAM REGEN MEMORY ADDRESS IN CRTC
;
	MOV	BX,MEMADRH*256+MEMADRL	; BH=CRTC R12			*3
					; BL=CRTC R13			*3
	CALL	WRTCRT 		; WRITE REGEN ADDR TO 6845	*3
;
; FIND CURSOR COORDINATES & GO PROGRAM CURSOR ADDRESS
;
	MOV	BL,DSPYPAG		; BL=REQUESTED DISPLAY PAGE
	CALL	RTNCURP 		; GET CURSOR POSITION
	CALL	UPCADDR 		; UPDATE CURSOR ADDRESS
SELPA1:
	JMP	COMEXIT 		; GO TO COMMON DSR EXIT
;------------------------------------------------------------------------------
; READ CHAR & ATTRIBUTE AT CURSOR POSITION (AH=08H)
;
;   INPUT:	BH = ACTIVE DISPLAY PAGE
;
;   OUTPUT:	AL = CHARACTER
;		AH = ATTRIBUTE
;			(not defined for graphics, however we return an ORing
;			 of any and all color bits set as the attribute, a
;			 reasonable compromise)
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
RDCHRATT:
	CALL	GRFXCHK 		; Q: GRAPHICS MODE		*2
	JC	L08H05			;   Yes
	CALL	RDCHR
	JMP	COMEXIT 		; GO TO COMMON DSR EXIT 	*6
;------------------------------------------------------------------------------
; GRAPHICS MODE READ
;
;	OUTPUT:	AL = CHAR READ (If recognized, else 0)
;		AH = ATTRIBUTE (COLOR) (If recognized, else 0)
;	All characters above 80h are recognized if the RAM font vector is other
;		than 0, else not
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
L08H05:
	XOR	BL,BL			; Ask for display page = 0
	CALL	RTNCURP 		; Get current cursor position
	CMP	CRTMODE,06h		; Get CRT mode and see if 640 x 200 mode
	JE	READ1			; If yes, then OK
	SHL	DL,1			;   Else pretend like it has twice
					;     as many columns	
READ1:
	CALL	GRFXAD			; Get address of graphic cursor
;
; DI = Address of character on screen
;
	MOV	ES,WORD PTR CS:B800H	; ES = 0B800h
	ASSUME	ES:NOTHING
	MOV	CX,8			; CX = LOOP COUNTER
;
	SUB	SP,10			; Make some room on the stack
					;   Note that 2 locations at SP
					;   are dead space but make this
					;   run faster
	MOV	BP,SP			; BP=START OF RESERVED AREA
	XOR	DH,DH			; Clear attribute
;
READ10:
	MOV	AX,ES:[DI]		; Grab a word (or byte)
	CMP	CRTMODE,6		; Q: 640x200 MODE
	JE	READ15			;  * YES
;
	XCHG	AH,AL			; IT'S 320x200 MODE
;
	MOV	BH,8
	XOR	DL,DL
READ11: 				; DECODE 320x200 BIT PATTERN (WORD)
	SHL	DL,1
	SHL	AX,1			; Q: LEFT-MOST BIT A 1
	JNC	READ12			;  * NO
	OR	DX,02h*256+01h		;  Yes, this dot will be on & set 
					;    attribute bit
READ12:
	SHL	AX,1			; Q: NEXT BIT A 1
	JNC	READ14			;  * NO
	OR	DX,01h*256+01h		;  Yes, this dot will be on & set 
					;    attribute bit
READ14:
	DEC	BH
	JNZ	READ11
	MOV	AL,DL			; AL = Bit pattern on display
	JMP	SHORT READ17
;
READ15:
	OR	AL,AL			; Q: ANY BITS ON
	JZ	READ17			;  * NO
	MOV	DH,01h			;  * YES, SET ATTRIBUTE
READ17:
	ADD	DI,2000h		; SET ODD SCAN ADDR		*5
	TEST	CX,1			; Q: ODD TIME THROUGH LOOP
	JE	READ22			;   No
	SUB	DI,4000h-80		; GO TO NEXT SCAN LINE		*5
READ22:
	INC	BP			; NEXT LOCATION
	MOV	[BP],AL 		; SAVE BIT PATTERN ON STACK
	LOOP	READ10			; LOOP FOR 8 SCAN LINES
;
	SUB	BP,7			; BACK TO START OF SAVED BIT PATTERNS
;
	MOV	AH,DH			; AH = attribute
	XOR	AL,AL			; Start off with character 000h
;
	PUSH	CS
	POP	ES			; ES = ROM segment address
	ASSUME	ES:CODE
	MOV	DI,OFFSET ROMFNT	; DI = Start of ROM Font table
;
	PUSH	SS
	POP	DS			; DS = SS
	ASSUME	DS:NOTHING
READ26:
	MOV	SI,BP			; SI = Start of bit pattern to find
					;   on stack
	MOV	CX,4			; CX = Compare word count
					;   (search counter)
	REPE	CMPSW			; Try to find bit pattern match
	JE	READ45			; If match, then done
	ADD	DI,CX			; Add in remainder left in CX (if any)
	ADD	DI,CX			;   to offset to next pattern group
	INC	AL			; Past end of ROM Font table ?
	JNS	READ26			;   No
;
	XOR	DI,DI			; ES = RAM Font vector segment address
	MOV	ES,DI			; 
	ASSUME	ES:INTVEC
	LES	DI,ES:RAMFNT		; Get RAM Font pointer and segment
	ASSUME	ES:NOTHING
	MOV	CX,ES			; Get contents of segment register
	OR	CX,DI			;   and test if ES and DI are both 0
	JZ	READ40			; If yes, return no character
READ27:
	MOV	SI,BP			; SI = Start of bit pattern to find on stack
	MOV	CX,4			; CX = Compare word count
					;   (search counter)
	REPE	CMPSW			; Try to find bit pattern match
	JE	READ45			; If match, then done
	ADD	DI,CX			; Add in remainder left in CX (if any)
	ADD	DI,CX			;   to offset to next pattern group
	INC	AL			; If AL = 0, then we wrapped around
					;   and are finished past end of
					;   RAM Font table
	JNZ	READ27			;   No
READ40:
	XOR	AX,AX			; Yes, then return no character or
					;   attribute
READ45:
	ADD	SP,10			; RESTORE STACK POINTER
	JMP	SHORT RDDOT0		; GO TO COMMON DSR EXIT 	*6
;------------------------------------------------------------------------------
; WRITE DOT (AH=0CH)
;
;   INPUT:	DX = ROW NUMBER (MODE DEPENDENT)
;		CX = COLUMN NUMBER (MODE DEPENDENT)
;		AL = COLOR VALUE
;	OUTPUT:	AH = ?
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
WRTDOT:
	CALL	DOTSUB			; Get address and shift count
	ASSUME	ES:NOTHING
	JC	RDDOT0			; Exit if error
;
	MOV	DH,DL			; Save mask in DH
	AND	DL,AL			; Get color value into DL
	SHL	DX,CL			; Shift mask the shift count, DL and DH
	OR	AL,AL			; Test the sign bit for XOR operation
	JS	WRTDO0			;   Yes, then do XOR
	NOT	DH			; Complement the mask in DH
	AND	ES:[BX],DH		; Clear dot bits
	OR	ES:[BX],DL		; Put in new dot color
	JMP	SHORT RDDOT0		; Go to common DSR exit
WRTDO0:
	XOR	ES:[BX],DL		; XOR in the new dot color
WRTDO1:
	JMP	SHORT RDDOT0		; Go to common DSR exit
;------------------------------------------------------------------------------
; READ DOT (AH=0DH)
;
;	INPUT:	DX = ROW NUMBER (MODE DEPENDENT)
;		CX = COLUMN NUMBER (MODE DEPENDENT)
;	OUTPUT:	AH = ?
;		AL = COLOR VALUE
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
READDOT:
	CALL	DOTSUB			; Get address and shift count
	ASSUME	ES:NOTHING
	JC	RDDOT0			; Exit if error
;
	MOV	AL,ES:[BX]		; GET COLOR VALUE
	SHR	AL,CL			; SHIFT TO PROPER POSITION
	AND	AL,DL			; MASK ALL BUT COLOR VALUE BITS *3
RDDOT0:
	JMP	COMEXIT 		; GO TO COMMON DSR EXIT 	*6
;------------------------------------------------------------------------------
; SCROLL ACTIVE PAGE DOWN (AH=07H)
;
;   INPUT:	AL = LINES TO SCROLL (CLEAR WINDOW IF 0)
;		BH = ATTRIBUTE FOR BLANK LINE(S)
;    		CH,CL = ROW/COLUMN OF UPPER LEFT CORNER OF WINDOW
;    		DH,DL = ROW/COLUMN OF LOWER RIGHT CORNER OF WINDOW
;   OUTPUT:	None
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
ROLLDOWN:
	MOV	BL,01h			; Set flag to indicate scroll down
	JMP	SHORT ROLLU0		;   Finish up in rollup routine
;------------------------------------------------------------------------------
; SCROLL ACTIVE PAGE UP (AH=06H)
;
;   INPUT:	AL = LINES TO SCROLL (CLEAR WINDOW IF 0)
;		BH = ATTRIBUTE FOR BLANK LINE(S)
;    		CH,CL = ROW/COLUMN OF UPPER LEFT CORNER OF WINDOW
;    		DH,DL = ROW/COLUMN OF LOWER RIGHT CORNER OF WINDOW
;   OUTPUT:	None
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
ROLLUP:
	XOR	BL,BL			; Set flag to indicate scroll up
ROLLU0:
	CMP	DH,25			; Is the row # past 24 ?
	JC	ROLLU1			;   No, then check column
	MOV	DH,24			;   Yes, then make it row 24
;
ROLLU1:
	CMP	DL,BYTE PTR SCRNWID	; Is the column past end of screen ?
	JC	ROLLU2			;   No, then check order
	MOV	DL,BYTE PTR SCRNWID	;   Yes, then make it the screen width
	DEC	DL			;     minus 1
;
ROLLU2:
	SUB	DL,CL			; Is the left column <= right column ?
	PUSH	AX
;
	IF	NOT SCRCOMP		; Don't do a compatible scroll
	JC	ROLLU5			;   No, then exit
	ENDIF
;
	MOV	AH,DH
	SUB	AH,CH			; AH = (Length of window-1) lines
;
	IF	NOT SCRCOMP		; Don't do a compatible scroll
	JC	ROLLU5			; If the top row > bottom row,
					;   then exit
	ENDIF
;
	INC	DL			; Column width is really 1 more
;
	XCHG	CH,CL
	XCHG	DL,CL			; CH = Left side, CL = Width (right side)
					; DH = Bottom,    DL = Top
	SUB	AH,AL			; Scroll the whole window ?
	JNC	ROLLU3			;   No
	XOR	AL,AL			;   Yes, set AL = 0 to force it
ROLLU3:
	OR	AL,AL
	JNZ	ROLLU6
	MOV	AH,0FFh			;   Make AH = 0 - 1 
ROLLU6:
	INC	AH			; AH = # of lines to move in window
ROLLU4:
	CALL	ROLLSUB 		; Go do scroll
ROLLU5:
	JMP	SHORT WRTCH1		; Common DSR return code with A register
;------------------------------------------------------------------------------
; WRITE CHAR & ATTRIBUTE AT CURSOR POSITION (AH=09H)
;
;	INPUT:	BH = ACTIVE DISPLAY PAGE
;		CX = NUMBER OF TIMES TO WRITE CHARACTER
;		AL = CHAR TO WRITE
;		BL = CHARACTER ATTRIBUTE
;	OUTPUT:	None
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
WRTCHRATT:
	XOR	DL,DL			; Set flag to indicate write of both
					;   character & attribute	
	JMP	SHORT WRTCH0		; Finish up in write character routine
;------------------------------------------------------------------------------
; WRITE CHAR AT CURSOR POSITION (AH=0AH)
;
;	INPUT:	BH = ACTIVE DISPLAY PAGE
;		CX = NUMBER OF TIMES TO WRITE CHARACTER
;		AL = CHAR TO WRITE
;		BL = Character Attribute if in a graphics mode otherwise
;			ignored
;	OUTPUT:	None
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
WRTCHR:
	MOV	DL,01h			; Set flag to indicate write of
					;   character only
WRTCH0:
	JCXZ	RDDOT0			; If CX = 0, then we're done
	PUSH	AX
	CALL	WRTSUB			; Go do write
WRTCH1:
	JMP	COMXITA			; Common DSR return code with A register

;------------------------------------------------------------------------------
; WRITE TELETYPE (AH=0EH)
;
;	 INPUT:	AL = CHARACTER TO BE WRITTEN
;		BL = FOREGROUND COLOR OF CHAR (USED ONLY IN GRAPHICS MODE)
;		BH = REQUESTED DISPLAY PAGE (REALLY IS IGNORED)
;	OUTPUT:	None
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
WRTTTY:
	PUSH	AX
;*			    get actual display page in BH
	MOV	CL,BL			; save graphics attribute
	CALL	GETDP			; Get current display page in BH & BL
	MOV	BL,CL			; restore graphics attribute
;*			    get current cursor position	in DX
	PUSH	AX			;save char to be written
	MOV	AH,03h
	INT	10h			;cursor position in DX, type in CX
	POP	AX
;*			    call subroutine to do all the work
	MOV	CL,0Ah			;use func 0A to write if displayable
	CALL	WTTYSUB
;*			    write new cursor
	MOV	AH,02h
		      			;DX still cursor loc, BH still page
	INT	10h
;*			    exit
	JMP	COMXITA	

;------------------------------------------------------------------------------
; WRITE TELETYPE SUBROUTINE
;	CALLED BY INT 10 SERVICE FOR BOTH FUNC 10 AND 13
;	INPUT:	AL = CHARACTER TO BE WRITTEN
;		BL = color/attribute (only if graphics mode or if CL = 09)
;		BH = DISPLAY PAGE (really)
;		DX = current cursor location
;		CL = function w/ which to write displayable char - 09 or 0A
;	OUTPUT:	DX = updated cursor location
;		CX,AX spoiled
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
WTTYSUB	PROC	NEAR
;*			    if BELL,
	CMP	AL,BELL			; BELL character ?
	JNE	NOTBEL
;*				beep and exit - no cursor change
	JMP	BEEP			;return to caller after BEEP
;*			     elseif CR,
;*				go reset cursor column
NOTBEL:
	CMP	AL,CR			; Q: CARRIAGE RETURN
	JE	L0ERCC
;*			     elseif BS,
NOTCR:
	CMP	AL,BS			; Q: BACKSPACE CHARACTER
	JNE	NOTBS			;  * NO
;*				update cursor
	SUB	DL,1			; Q: BACKED INTO PREVIOUS ROW
	JNC	L0EENDIF		;  * NO
;
	IF	BSCOMP
	JMP	SHORT L0ERCC		; leave at same row, column 0
	ELSE
	OR	DH,DH			; Q: ALREADY IN ROW 0
	JZ	L0ERCC			;   Yes, leave at row 0, column 0
	DEC	DH			; BACKUP ONE ROW
	ADD	DL,BYTE PTR SCRNWID	; SET TO LAST COLUMN (note DL = -1)
	JMP	SHORT L0EENDIF
	ENDIF
;*			     elseif LF,
NOTBS:
	CMP	AL,LF			; Q: LINE FEED
	JNE	NOTLF			;  * NO
;*				update cursor
	INC	DH
;*			     else,
	JMP	SHORT L0EENDIF
NOTLF:
;*				write displayable character
	MOV	AH,CL			;write character function - 09 or 0A
	MOV	CX,1			;number of times to write char
					;BL still is char attribute	
					;BH still is display page
	INT	10h			;write AL at current cursor loc
;*				update cursor column
	INC	DX ;DL
;*				if end of row,
	CMP	DL,BYTE PTR SCRNWID	; Past end of row ?
	JNE	L0ECOK
;*				    update cursor row
	INC	DH
;*				    reset cursor column
L0ERCC:
	XOR	DL,DL			; set cursor to 1st column
;*				 endif
L0ECOK:
;*			     endif
L0EENDIF:
;*			    if cursor row > 24,
	CMP	DH,25
	JNE	L0EEIF2			; don't worry about SCRCOMP here -
					;  DH can't be > 25 - stan 6/85
;*				decrement it
	DEC	DH
;*				scroll screen:
	PUSH	DX
	PUSH	BX
;*				    get attribute @ cursor loc prior to call
	MOV	BH,00			;attribute for background if graphics
	CALL	GRFXCHK 		; graphics mode?
	JC	L0EEIF3
	MOV	AH,08h			; if not, read attribute
	INT	10h			;AH = attribute
	MOV	BH,AH			;attribute for background if alpha
L0EEIF3:
;*				    scroll whole screen 1 row
	MOV	AX,06h*100h+1		;06h = scroll request, 1 = # of lines
	MOV	CX,0000h		;first row & column
	MOV	DH,24			;last row
	MOV	DL,BYTE PTR SCRNWID
	DEC	DX ;DL			;last column
	INT	10h			;request scroll
	POP	BX			;restore page select
	POP	DX			;restore cursor position
;*			     endif
L0EEIF2:
;*			    return
	RET
WTTYSUB	ENDP

;------------------------------------------------------------------------------
; SET CURSOR POSITION (AH=02H)
;
;	INPUTS:	BH = Page # if CRT mode is 0 -> 3 (0 if graphics or monochrome)
;		DH = Row # of cursor
;		DL = Column # of cursor
;	OUTPUT:	None
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
SETCURP:
	CALL	SETCUR 		; GO SET CURSOR POSITION
	JMP	COMEXIT		; Common DSR return code with A register
;------------------------------------------------------------------------------
; READ CURRENT VIDEO STATE (AH=0FH)
;
;	Input:	DS = ROM Data segment
;   OUTPUT:	AH = NUMBER OF SCREEN COLUMNS
;		AL = CURRENT VIDEO MODE
;		BH = ACTIVE DISPLAY PAGE
;------------------------------------------------------------------------------
	ASSUME	DS:ROMDAT,ES:NOTHING
RDVIDOST:
	MOV	AH,BYTE PTR SCRNWID	; Get screen width (lowest 8 bits)
	MOV	AL,CRTMODE		; Get video mode
	MOV	BH,DSPYPAG		; Get current display page
	MOV	BP,SP			; Get pointer to stack space
	MOV	[BP+15],BH		; Put new BH into stack space
	JMP	COMEXIT 		; GO TO COMMON DSR EXIT 	*6
CRTDSR1	ENDP
	PRGSIZ	<CRT Routines>		; Display program size
;------------------------------------------------------------------------------
; Compute address of graphic row and column
;
;   INPUT:	DL = Graphic column #
;		DH = Graphic row #
;   OUTPUT:	DI = Address of graphic cursor
;------------------------------------------------------------------------------
	ASSUME	DS:NOTHING,ES:NOTHING
GRFXAD	PROC	NEAR
	PUSH	AX
	PUSH	DX
	XOR	AX,AX			; AX = 0
	XCHG	AH,DH			; AH = Requested row, DH = 0
; Multiply Row in DH * 80 bytes per scan * 4 scans/row
	MOV	DI,AX			; DI = Row * 256
	SAR	DI,1
	SAR	DI,1			; DI = Row * 64
	ADD	DI,AX			; DI = Row * 320
	ADD	DI,DX			;   plus offset to requested column
	POP	DX
	POP	AX
	RET
GRFXAD	ENDP
	PRGSIZ	<Address of graphic row & column subr>	; Display program size
;------------------------------------------------------------------------------
; SUBROUTINE TO GET CURRENT DISPLAY PAGE
;
;   INPUT:	DS = Segment of Rom data area
;   OUTPUT:	BH, BL = CURRENT DISPLAY PAGE					*E*D
;------------------------------------------------------------------------------
GETDP	PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	MOV	BL,DSPYPAG		; Get current page
	CMP	CRTMODE,4		; Is it a graphic or monochrome mode ?
	JL	GETDP10 		;  No, then exit
	XOR	BL,BL			;  Yes, set display page = 0
GETDP10:
	MOV	BH,BL			; Return it in BH and BL		*E
	RET
;
GETDP	ENDP
	PRGSIZ	<Current Display Page>	; Display program size
;------------------------------------------------------------------------------
;  Subroutine to set cursor
;
;	Input:	DS = ROM data segment (0040h)
;		BH = Page # if CRT mode is 0 -> 3
;		DX = row/column
;	Output:	BH=CRTC R14
;		BL=CRTC R15
;		CX=REGEN ADDR OF CURSOR
;		DX = CRTADDR+1
;------------------------------------------------------------------------------
	PUBLIC	SETCUR			; Make public for advanced graphic
					;   board's code
SETCUR	PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	CMP	CRTMODE,4		; Color Adaptor Alphanumeric mode ?
	JC	SETCU0			;   Yes
	XOR	BH,BH			;   No, then display page = 0
SETCU0:
	MOV	BL,BH			; COMPUTE TABLE INDEX
	XOR	BH,BH			;  *
	SAL	BX,1			;  *
	MOV	[CURCOOR+BX],DX	; Save column/row in cursor table + page
	SAR	BX,1		; Restore Display page to BL
	CMP	DSPYPAG,BL		; Q: NEED TO UPDATE CRTC
	JNE	WRTCR0		;  * NO, DISPLAYING DIFFERENT PAGE
				;  * YES, MUST UPDATE CURSOR POSITION
;------------------------------------------------------------------------------
; COMPUTE CURSOR ADDRESS IN REGEN MEMORY
;
;   ADDRESS COMPUTED IS A WORD ADDRESS: ADDRESS MUST BE MULTIPLIED BY
;   2 TO GET ACTUAL ADDRESS OF CHAR/ATTR PAIR IN REGEN MEMORY
;
;	Input:	DS = ROM data segment (0040h)
;		DX = ROW/COL OF CURSOR
;	Output:	BH=CRTC R14
;		BL=CRTC R15
;		CX=REGEN ADDR OF CURSOR
;		DX = CRTADDR+1
;------------------------------------------------------------------------------
UPCADDR:
	ASSUME	DS:ROMDAT,ES:NOTHING
	PUSH	AX
	MOV	AL,DH			; AL=ROW NUMBER
	MUL	BYTE PTR SCRNWID	; MULTIPLY BY SCREEN WIDTH
	XOR	DH,DH
	ADD	AX,DX			; AX=REGEN WORD ADDR OF CURSOR

	MOV	CX,PAGADDR	; Get REGEN address of page
	SHR	CX,1		;  times 2
	ADD	CX,AX		; Add in REGEN word address of cursor
				;   CX=REGEN address of cursor
	MOV	BX,CURADRH*256+CURADRL	; BH=CRTC R14			*3
					; BL=CRTC R15			*3
	POP	AX
; WRITE CURSOR ADDR TO 6845 by falling through to WRTCRT routine
;
;------------------------------------------------------------------------------
; SUBROUTINE TO WRITE BYTES TO 6845 CRTC
;
;	Input:	DS = ROM data segment (0040h)
;		BH = 1ST CTRC REGISTER #
;		BL = 2ND CRTC REGISTER #
;		CH = 1ST BYTE TO WRITE
;		CL = 2ND BYTE TO WRITE
;   OUTPUT:	DX = CRTADDR+1
;------------------------------------------------------------------------------
WRTCRT:
	ASSUME	DS:ROMDAT,ES:NOTHING
	PUSH	AX
	MOV	DX,CRTADDR		; DX=ADDR OF CRTC INDEX REG
	MOV	AL,BH			; AL=1ST CRTC REGISTER
	OUT	DX,AL			; SELECT REG
	INC	DX			; DX=CRTC DATA REGISTER
	MOV	AL,CH			; 1ST DATA BYTE
       IF	I80286
	JMP $+2
       ENDIF
	OUT	DX,AL			; SET CURSOR START
;
	DEC	DX			; RESTORE DX
	XCHG	AX,BX			; AL=2ND CRTC REGISTER
					; Delay for chip I/O recovery time
       IF	I80286
	JMP $+2
       ENDIF
	OUT	DX,AL			; SELECT REG
	XCHG	AX,BX			; Delay for chip I/O recovery time
;
	INC	DX			; DX=CRTC DATA REGISTER
	XCHG	AX,CX			; 2ND DATA BYTE
					; Delay for chip I/O recovery time
       IF	I80286
	JMP $+2
       ENDIF
	OUT	DX,AL			; SET CURSOR END
	XCHG	AX,CX			; Delay for chip I/O recovery time
;
	POP	AX
WRTCR0:	RET				; DONE
SETCUR	ENDP
	PRGSIZ	<Set cursor, Update Cursor & Write CRT subr>	; Display program size
;------------------------------------------------------------------------------
;
	OUT1	<.		Including GRFXSB.INC>
INCLUDE	GRFXSB.INC
;

; More CRT support subroutines:
;------------------------------------------------------------------------------
; Print character to CRT
;
;	Input:	AL = Character to print
;------------------------------------------------------------------------------
	PUBLIC	PRTCHR
PRTCHR	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	PUSH	BX
	MOV	BL,82h		; XOR in the foreground color if in graphic
				;   mode.  Note: If you do a CR, LF the
				;   graphic mode clears line to 0 for teletype
	MOV	AH,0Eh		; Write teletype function
	INT	10h		; Call CRT routine
	POP	BX
	RET
PRTCHR	ENDP
	PRGSIZ	<Write Character to CRT>	; Display program size

;------------------------------------------------------------------------------
; Write string to CRT until a null
;
; Input:	SI = Pointer to string to print
; Output:	SI = Pointer to string end (null)
;------------------------------------------------------------------------------
	PUBLIC	WRTCRLF
	PUBLIC	WRTOUT
	EXTRN	CRLFMSG:BYTE
	EXTRN	WRTSTI:NEAR
WRTCRLF	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	MOV	SI,OFFSET CRLFMSG
WRTOUT:
	PUSH	CX
	CALL	WRTSTI
	POP	CX
	RET
WRTCRLF	ENDP
	PRGSIZ	<Write string to CRT>	; Display program size

	IF	BLWAIT
;------------------------------------------------------------------------------
; Wait 19 microseconds after a horizontal retrace and test for a vertical
;	retrace
;
;	Input:	DX = CRT Status register port
;	Output:	AL = Value of current MSR
;		Zero Flag is set if not a vertical retrace period, else not set
;------------------------------------------------------------------------------
	PUBLIC	VERTST
VERTST	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	PUSH	CX
;
	IF	34000/CLK10 GE 1900	; Delay of code is
					;  already >= 19.00 microseconds
	MOV	CX,1
	ELSE
	MOV	CX,(190*CLK10-3400)/1700+1
	ENDIF
;
WAIT19:
	LOOP	WAIT19			; Wait 19 microseconds (at least) and
					;   see if it's still there
	POP	CX
	IN	AL,DX			; Test to see if it was a vertical
	TEST	AL,BLANK		;   retrace.  If it is still being
					;   blanked then it is
	RET
VERTST	ENDP
	ENDIF	;BLWAIT
	PRGSIZ	<Test for Vertical retrace period>	; Display program size
;
	SYNC	MEMCHKA
;
	TOTAL
;
CODE	ENDS
	END
