	NAME	TESTRM
	TITLE	Test ROM Terminate and stay Resident program
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program	   *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent.			   *
;***************************************************************************

		;******************************
		;*  Compatability ROM Tester  *
		;******************************
;This module provides a means for testing the IBM Compatability ROM
;code.  It replaces all interrupt vector but INT 19 (the bootstrap) and
;INT 3 (non-maskable interupt) with those for the ROM code, and leaves
;this code in memory as a terminate and stay resident program.
;
;------------------------------------------------------------------------------
;
INCLUDE	ROMOPT.INC		; ROM Options file - must be included 1st
INCLUDE	ROMEQU.INC		; ROM General equate file
INCLUDE	MACROS.INC		; ROM Macros file
;
;------------------------------------------------------------------------------
;
	OUT1	<TESTRM - ROM Terminate and Stay resident test code>
;
;
PFIX	EQU	false	;true		; True if using PFIX and you don't
					; want the keyboard vectors to be
					; changed
;
;------------------------------------------------------------------------------
;
SetVec	macro	Vec,Addr,MSG ;;macro for setting interrupt vector.
	mov	AL,Vec
	MOV	DX,offset Addr
	MOV	SI,OFFSET MSG
	CALL	SUBVEC
	endm
;
;------------------------------------------------------------------------------
;
DGROUP	GROUP	INTVEC,ROMDAT
INTVEC	SEGMENT AT 0000h	; The following are relative to Segment 00h
	EXTRN	CRTCINT:DWORD		; Offset of pointer CRTC INIT table
INTVEC	ENDS
;
;------------------------------------------------------------------------------
;
ROMDAT	SEGMENT AT 0040h	;The following are relative to Segment 40h
	EXTRN	BASMEM:WORD
	EXTRN	LPTOTBL:BYTE		; 
	EXTRN	KBXTGET:WORD
	EXTRN	KBXTPUT:WORD
	EXTRN	KBUFR:WORD		; START OF KEYBOARD BUFFER
	EXTRN	CRTMODE:BYTE
	EXTRN	CURTYPE:WORD
	EXTRN	SCRNWID:BYTE		; SCREEN COLUMN WIDTH
;
ROMDAT	ENDS
;
;------------------------------------------------------------------------------
CGROUP	GROUP	CODE
CODE	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
;
	extrn	DSKISR:near		; int entry into diskdsr
	extrn	DSKDSR:near		; i/o call entry into diskdsr
	EXTRN	CASDSR:NEAR
	EXTRN	NMIINT:NEAR		; NMI interrupt
;
	EXTRN	WRTINL:NEAR		; Write inline message to CRT
	EXTRN	WRTOUT:NEAR		; Write message to CRT
	EXTRN	SIGNON:NEAR		; Write sign on message
;
	EXTRN	COMTYP:BYTE		; Computer type (AT, XT, PC or JR)
	EXTRN	DS40H:WORD		; Get Segment for ROM data area

CRTF	DB	0			; Flag = 01h if enhanced CRT, else 00h
HDDSKF	DB	0			; Flag = 01h if hard disk, else 00h
COMTYF	DB	0			; Computer type of actual ROM
;
INT02MSG	DB	"Interrupt 02h - NMI",CR,LF,0
INT05MSG	DB	"Interrupt 05h - Print screen",CR,LF,0
INT08MSG	DB	"Interrupt 08h - Timer ISR",CR,LF,0
INT09MSG	DB	"Interrupt 09h - Keyboard ISR",CR,LF,0
INT0EMSG	DB	"Interrupt 0Eh - Floppy Disk ISR",CR,LF,0
INT10MSG	DB	"Interrupt 10h - CRT DSR",CR,LF,0
INT11MSG	DB	"Interrupt 11h - Equipment check DSR",CR,LF,0
INT12MSG	DB	"Interrupt 12h - Memory check DSR",CR,LF,0
INT13MSG	DB	"Interrupt 13h - Floppy disk DSR",CR,LF,0
INT14MSG	DB	"Interrupt 14h - EIA DSR",CR,LF,0
INT15MSG	DB	"Interrupt 15h - Cassette",CR,LF,0
INT16MSG	DB	"Interrupt 16h - Keyboard DSR",CR,LF,0
INT17MSG	DB	"Interrupt 17h - Printer DSR",CR,LF,0
INT1AMSG	DB	"Interrupt 1Ah - Timer DSR",CR,LF,0
INT1DMSG	DB	"Interrupt 1Dh - CRT Parameters vector",CR,LF,0
INT40MSG	DB	"Interrupt 40h - Floppy disk DSR",CR,LF,0
INT70MSG	DB	"Interrupt 70h - Real Time Clock ISR",CR,LF,0
INT76MSG	DB	"Interrupt 76h - Hard Disk ISR",CR,LF,0
INTXXMSG	DB	"Interrupt     - Other",CR,LF,0
						;Interrupt vector not used
;
; Following is a table that can be used to enable/disable the replacement of
; particular interrupts by TESTROM.  Bit 0 = 1 is defined as enabling the
; replacement if the code wants to.  There are a few places where this may be
; not used.  See code if you have any questions.  Also if PFIX is true, the
; keyboard is not replaced, just in case you are using a debugger to check the
; rom code itself and the debugger does not operate properly when the keyboard
; is replaced.
;
;		 0    1    2    3    4    5    6    7	; Interrupt lower nibble
;		 8    9    A    B    C    D    E    F	; Interrupt lower nibble
	PUBLIC	INTTBL
INTTBL	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 00h -> 07h
	IF	PFIX
	DB	01h, 00h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 08h -> 0Fh
;
	DB	01h, 01h, 01h, 01h, 01h, 01h, 00h, 01h	; INT 10h -> 17h
	ELSE
	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 08h -> 0Fh
;
	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 10h -> 17h
	ENDIF
;
	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 18h -> 1Fh
;
	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 20h -> 27h
	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 28h -> 2Fh
;
	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 30h -> 37h
	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 38h -> 3Fh
;
	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 40h -> 47h
	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 48h -> 4Fh
;
	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 50h -> 57h
	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 58h -> 5Fh
;
	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 60h -> 67h
	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 68h -> 6Fh
;
	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 70h -> 77h
	DB	01h, 01h, 01h, 01h, 01h, 01h, 01h, 01h	; INT 78h -> 7Fh
;
; The following 2 words have a pointer to a location that the originally hard
; disk DSR at INT 13h is stored by DOS I/O.  DOS I/O replaces INT 13h to do
; some error handling and eventually calls the hard disk DSR stored at this
; location
;
	PUBLIC	HRDDSRP
HRDDSRP	DW	094Eh			; Offset of DOS I/O storage location
	DW	0070h			; Segment of DOS I/O storage location
;
START	PROC	FAR
	PUSH	ES
	PUSH	DS
	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DX
	PUSH	DI
	PUSH	SI
	PUSH	BP
;
; Test for Hard Disk
;
	MOV	AH,08h			; Return hard disk parameters
	MOV	DL,80h			;   for the 1st hard disk drive
	INT	13h			; See if it's there
	MOV	HDDSKF,00h		; Clear flag to say no hard disk
	JC	NOHDSK			; If carry set, then no hard disk
	OR	HDDSKF,01h		;   Else, set flag to say hard disk
					;   present
NOHDSK:
;
; Test for Enhanced Graphics Board (To be implemented)
;
	MOV	CRTF,00h		; Clear flag to say no enhanced CRT
;
;
;
	MOV	AX,0F000h		; Set segment of ROM
	MOV	DS,AX
	ASSUME	DS:NOTHING
	MOV	AL,BYTE PTR DS:[0FFFEh]	; Get computer type
	MOV	COMTYF,AL		; Store computer type
	CMP	AL,0FCh			; Is it less than an AT type ?
	JNC	KNWNRM
	MOV	COMTYF,0FEh		; Pretend like it's an XT type if
					;   unknown
KNWNRM:
;
; Clear screen
;
	MOV	DS,DS40H
	ASSUME	DS:ROMDAT
	MOV	BH,7			; Set to blank attribute if not graphic
	CMP	CRTMODE,4		; GRAPHICS MODE ?
	JC	GRAPHC			;   No
	CMP	CRTMODE,7		; MONOCHROME ?
	JE	GRAPHC			;   Yes
	MOV	BH,0			; Attribute for blanked lines
GRAPHC:
	MOV	AX,6*256+0		; Scroll whole page up
	MOV	CX,0			; Top left corner
	MOV	DH,24			; Bottom Row
	MOV	DL,SCRNWID		; Right column
	INT	10h			; Clear screen
;
; Print header message
;
	CALL	WRTINL			; Print initial message
	DB	" Phoenix Test ",0
;
	MOV	AL,COMTYP
	CALL	PRTTYP			; Print TESTROM computer type byte in AL
;
	CALL	WRTINL			; Print middle message
	DB	" ROM running under ",0
;
	MOV	AL,COMTYF
	CALL	PRTTYP			; Print computer type of byte in AL
;
	CALL	WRTINL
	DB	" environment",CR,LF,LF
	DB	"The following interrupt vectors are being replaced;",CR,LF
	DB	"---------------------------------------------------",CR,LF,0
;
; Set EIA and printer timeouts - 
;	old PC rom doesn't do it!
;
	MOV	ES,DS40H
	ASSUME	ES:ROMDAT
	MOV	DI,OFFSET LPTOTBL
	MOV	AL,14H
	mov	CX,4
	cld			; clear direction flag
	rep	stosb
	mov	AL,1
	mov	CX,4
	rep	stosb
;
; Set Keyboard buffer start and end location pointers
;
	CLI
	MOV	ES:KBXTGET,OFFSET KBUFR
	MOV	ES:KBXTPUT,OFFSET KBUFR+32
	STI
;
;
;
	POP	BP
	POP	SI
	POP	DI
	POP	DX
	POP	CX
	POP	BX
	POP	AX
;
; Put in new vectors for interrupts
;
	mov	AX,CS			;DS := CS for setting vectors
	mov	DS,AX
	ASSUME	DS:CODE
;
	CMP	COMTYF,0FCh		; Test if we're running on an AT
	JNZ	NOTATE
	JMP	ATCOMP			; Yes, then don't do following
					;   interrupts
NOTATE:
;
	setVec	0EH,DSKISR,INT0EMSG	; Disk Driver
;
	CMP	HDDSKF,01h
	JE	HARDSK			; There is a hard disk
;
	setVec	13H,DSKDSR,INT13MSG	; Replace floppy disk DSR
	JMP	SHORT DISKFN
HARDSK:
	setVec	40h,DSKDSR,INT40MSG	; Replace floppy disk DSR
DISKFN:
;
	SetVec	02H,NMIINT,INT02MSG	;NMI
;
	setVec	15H,CASDSR,INT15MSG	; Interrupt vector Cassette I/O
;
	JMP	NOTAT
;
ATCOMP:
;
	CMP	HDDSKF,01h
	JE	ATHDSK			; There is a hard disk
;
;
	JMP	NOATHD			; If carry set, then there is not a
					;   hard disk
;
ATHDSK:
;
;
;
NOATHD:
;
;
;
;
NOTAT:
;
	extrn	TIMISR:near		; int entry into timer dsr
	setVec	08H,timisr,INT08MSG	;Time of Day, also controls disk
;
	extrn	TIMDSR:near		; i/o call entry into timer dsr
	setVec	1AH,timdsr,INT1AMSG
;
	extrn	PRTSCR:near		; print screen routine
	setVec	05H,PRTSCR,INT05MSG	;PrintScreen
;
	extrn	KYBISR:near		; int entry into kybdsr
	setVec	09H,kybisr,INT09MSG	;Keyboard
	extrn	KYBDSR:near		; i/o call entry into kybdsr
	setVec	16H,kybdsr,INT16MSG
;
	extrn	EQPMTC:near		; equipment check
	setVec	11H,eqpmtc,INT11MSG	;Equipment Check
	extrn	MEMCHK:near		; memory size
	setVec	12H,memchk,INT12MSG	;Memory Check
;
	extrn	EIADSR:near		; i/o call entry into eiadsr
	setVec	14H,eiadsr,INT14MSG	;Communications
	extrn	PRTDSR:near		; parallel printer dsr
	setVec	17H,PRTDSR,INT17MSG	;Printer
;
	extrn	CRTPRM:BYTE		; 6845 CRT controller parameters
	setVec	1DH,CRTPRM,INT1DMSG
;
	extrn	CRTDSR:near		; i/o call entry into crtdsr
	setVec	10H,crtdsr,INT10MSG	;Video
;
;
	IF	false	;true		; Other interrupts that you may want to
					;   substitute
;
	EXTRN	INT00:NEAR
	setVec	00H,INT00,INTXXMSG	;Interrupt vector not used
	EXTRN	INT01:NEAR
	setVec	01H,INT01,INTXXMSG	;Interrupt vector not used
	EXTRN	INT03:NEAR
	setVec	03H,INT03,INTXXMSG	;Interrupt vector not used
	EXTRN	INT04:NEAR
	setVec	04H,INT04,INTXXMSG	;Interrupt vector not used
	EXTRN	INT06:NEAR
	setVec	06H,INT06,INTXXMSG	;Interrupt vector not used
	EXTRN	INT07:NEAR
	setVec	07H,INT07,INTXXMSG	;Interrupt vector not used
	EXTRN	INT0A:NEAR
	setVec	0AH,INT0A,INTXXMSG	;Interrupt vector not used
	EXTRN	INT0B:NEAR
	setVec	0BH,INT0B,INTXXMSG	;Interrupt vector not used
	EXTRN	INT0C:NEAR
	setVec	0CH,INT0C,INTXXMSG	;Interrupt vector not used
	EXTRN	INT0D:NEAR
	setVec	0DH,INT0D,INTXXMSG	;Interrupt vector not used
	EXTRN	INT0F:NEAR
	setVec	0FH,INT0F,INTXXMSG	;Interrupt vector not used
;
	extrn	DSKPRM:BYTE		; diskette parameter table,
	SetVec	1EH,DSKPRM,INTXXMSG	; (can't use:  MSDOS 2.0 changes it)
	extrn	FDBOOT:NEAR
	SetVec	19H,FDBOOT,INTXXMSG	;Bootstrap disk loader (will use real)
;
	endif		; true
;------------------------------------------------------------------------------
;
	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DS

;
	IF	INTENHD
	EXTRN	UNEXCS:WORD
	MOV	CS:UNEXCS,CS		; Set proper segment of far jump
	ENDIF
;
	XOR	AX,AX			; Set to interrupt vector segment
	MOV	DS,AX
	ASSUME	DS:INTVEC
	LES	BX,CRTCINT		; Get CRT Parameters address pointer
	ASSUME	ES:NOTHING
;
; The following stores the cursor to use when changing modes
;
; Store the graphics cursors for testrom but reverse the bytes for table
	MOV	WORD PTR ES:[BX+00h+0Ah],0704h
	MOV	WORD PTR ES:[BX+10h+0Ah],0704h
	MOV	WORD PTR ES:[BX+20h+0Ah],0704h
; Store monochrome cursor for testrom but reverse the bytes for table
	MOV	WORD PTR ES:[BX+30h+0Ah],000Dh
;
	MOV	DS,DS40H
	ASSUME	DS:ROMDAT
;------------------------------------------------------------------------------
	IF	KEYTAP
	MOV	WORD PTR BASMEM,CLICKD*256+CLICKV
						; Keyboard click duration and
						;   click high time pulse width
	ENDIF
;------------------------------------------------------------------------------
	MOV	CX,0D00h		; INITIALIZE CURSOR TYPE
	CMP	CRTMODE,7		; MONOCHROME MODE ?
	JE	start0			;   YES, DONE
	MOV	CX,0407H		; INITIALIZE CURSOR TYPE for graphics
start0:
	MOV	AH,01h			; Set cursor type
	INT	10h
	CMP	CRTMODE,7		; MONOCHROME MODE ?
	JNE	start1			;   No, DONE
	MOV	CURTYPE,0607H		; Set ROM data area back to this value
					;   if in monochrome mode because of a
					;   bug in original ROM
start1:
	PUSH	SI
	CALL	WRTINL
	DB	CR,LF,LF,0
;
	CALL	SIGNON
	POP	SI
	POP	DS
	POP	CX
	POP	BX
	POP	AX
;
;
	pop	DS			;restore my seg regs.
	ASSUME	DS:NOTHING
	pop	ES
	ASSUME	ES:NOTHING
	mov	byte ptr ES:1, 27H	;set interrupt 27 in program prefix
	mov	DX,offset cgroup:endcseg+600H	;terminate and stay resident
					; 200h for stack above, 100h for
					;   program prefix and 300h for slop
	push	ES
	xor	AX,AX
	push	AX
	ret

Start	endp
;------------------------------------------------------------------------------
; Print computer type of byte in AL
;------------------------------------------------------------------------------
PRTTYP	PROC	NEAR
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
	CMP	AL,0FFh
	JNZ	PRTTY0
	CALL	WRTINL
	DB	"PC",0
	JMP	SHORT PRTTY4
;
PRTTY0:
	CMP	AL,0FEh
	JNZ	PRTTY1
	CALL	WRTINL
	DB	"XT",0
	JMP	SHORT PRTTY4
;
PRTTY1:
	CMP	AL,0FDh
	JNZ	PRTTY2
	CALL	WRTINL
	DB	"JR",0
	JMP	SHORT PRTTY4
;
PRTTY2:
	CMP	AL,0FCh
	JNZ	PRTTY3
	CALL	WRTINL
	DB	"AT",0
	JMP	SHORT PRTTY4
;
PRTTY3:
	CALL	WRTINL
	DB	"??",0
PRTTY4:	RET
PRTTYP	ENDP
;------------------------------------------------------------------------------
; Substitute vector and print message routine
;------------------------------------------------------------------------------
SUBVEC	PROC	NEAR
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
	PUSH	AX
	PUSH	BX
	XOR	BH,BH
	MOV	BL,AL			; Get index to INT enable table
	TEST	[INTTBL+BX],01h		; Test enable byte
	JZ	SUBVE0
	CALL	WRTOUT			; Write string
	MOV	AH,25h
	INT	21h
SUBVE0:
	POP	BX
	POP	AX
	RET	
SUBVEC	ENDP
;------------------------------------------------------------------------------
POWRES	DW	0	  		; Power-on address of real ROM BIOS.
	DW	0FFFFh
	PUBLIC	RESETALT
RESETALT:				; This public symbol is substituted
	JMP	DWORD PTR POWRES	;   for the power up reset symbol by
					;   PLINK to allow TESTROM to reboot
;------------------------------------------------------------------------------
;
endcseg:			;mark end of ROM code
;
CODE	ENDS
;
STACK	SEGMENT	'STACK' STACK		; There might be an interrupt before
	DW	256 DUP (?)		;   program terminates
STACK	ENDS
	END	START
