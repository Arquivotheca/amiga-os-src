	NAME	VECTBL
	TITLE	Interrupt and Parameter Vector Tables
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program    *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent. 		   *
;***************************************************************************
;
;	THIS MODULE CONTAINS THE INTERRUPT VECTOR TABLE FOR THE SYSTEM 
;	ROM ROUTINES AS WELL AS THE CRT AND DISK PARAMETER TABLES.
;
;	Written: 11/23/83
;	Revised: 02/17/84 - PMG ** (initials)
;		 02/20/84 - RGV
;		 02/22/84 - T. Jennings
;		 07/  /84 Ira J. Perlow - changed structure for compatibility
;		 04/  /85 Ira J. Perlow - incorporated new unexpected interrupt
;					    handler
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
	OUT1	<VECTBL - ROM Interrupt vector table and code>
;
CGROUP	GROUP	CODE
CODE	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
;------------------------------------------------------------------------------
; Interrupt vector table moved to low memory
;------------------------------------------------------------------------------
	EXTRN	KYBDSR:NEAR		; I/O CALL ENTRY INTO KYBDSR
	EXTRN	CRTDSR:NEAR		; I/O CALL ENTRY INTO CRTDSR
IF	ENHFDD
	EXTRN	FD_ISR:NEAR		; ENHANCED INT ENTRY INTO DISKDSR
ELSE
	EXTRN	DSKISR:NEAR		; INT ENTRY INTO DISKDSR
ENDIF
	EXTRN	EQPMTC:NEAR		; EQUIPMENT CHECK
	EXTRN	CASDSR:NEAR		; Cassette DSR to return error code
	EXTRN	MEMCHK:NEAR		; MEMORY SIZE
	EXTRN	DSKDSR:NEAR		; I/O CALL ENTRY INTO DISKDSR
	EXTRN	EIADSR:NEAR		; I/O CALL ENTRY INTO EIADSR
	EXTRN	KYBISR:NEAR		; INT ENTRY INTO KYBDSR
	EXTRN	TIMDSR:NEAR		; I/O CALL ENTRY INTO TIMER DSR
	EXTRN	TIMISR:NEAR		; INT ENTRY INTO TIMER DSR
	EXTRN	PRTDSR:NEAR		; PARALLEL PRINTER DSR
	EXTRN	FDBOOT:NEAR		; INIT BOOT CODE ENTRY (INT 19H)

	EXTRN	CRTPRM:NEAR		; CRT Parameters
	EXTRN	DSKPRM:NEAR		; Disk Parameters
;
	EXTRN	FDBTER:NEAR	; INT 18h entry
;
	SYNC	VECTBLA
	PUBLIC	VECSIZ
	PUBLIC	VECTBL
VECTBL	LABEL	WORD
	DW	TIMISR		; Interrupt 08h
	DW	KYBISR		; Interrupt 09h
	DW	INT0A		; Interrupt 0Ah reserved
				;   (AT slave interrupt controller)
	DW	INT0B		; Interrupt 0Bh serial 2
	DW	INT0C		; Interrupt 0Ch serial 1
	DW	INT0D		; Interrupt 0Dh hard disk
				;   (AT parallel printer 2)
IF	ENHFDD
	DW	FD_ISR		; Enhanced Interrupt 0Eh
ELSE
	DW	DSKISR		; Interrupt 0Eh
ENDIF
	DW	INT0F		; Interrupt 0Fh parallel printer 1
;
	DW	CRTDSR		; Interrupt 10h
	DW	EQPMTC		; Interrupt 11h
	DW	MEMCHK		; Interrupt 12h
	DW	DSKDSR		; Interrupt 13h
	DW	EIADSR		; Interrupt 14h
	DW	CASDSR		; Interrupt 15h  Cassette
	DW	KYBDSR		; Interrupt 16h
	DW	PRTDSR		; Interrupt 17h
	DW	FDBTER		; Interrupt 18h  Cassette BASIC entry
				;   point, or error booting disk message
				;   or monitor entry point
;
	DW	FDBOOT		; Interrupt 19h  Floppy Disk boot
	DW	TIMDSR		; Interrupt 1Ah  Timer DSR
	DW	DUMINT		; Interrupt 1Bh  User supplied Keyboard
				;			break
	DW	DUMINT		; Interrupt 1Ch  User supplied Timer
				;			tick
	DW	CRTPRM		; Interrupt 1Dh  CRT parameters
	DW	DSKPRM		; Interrupt 1Eh  Floppy Disk parameters
	DW	0		; This contains the RAM font offset.
				;   If 0, and the segment is 0, an
				;   inverted ROM font will be used
VECSIZ	EQU	($-VECTBL)/2	; Length of vector table
	PRGSIZ	<Interrupt Vector table>	; Display Program Size
;------------------------------------------------------------------------------
; General unexpected interrupt handler
;
;	Entry:	CS = code segment + int #, IP = OFFSET UNEXINT - int # * 10h
;	Exit:	CS = code segment + int #, IP = OFFSET INTERR  - int # * 10h
;
; ***** Note: This will only work because a near JMP is relative to the *****
; *****   current CS:IP.  The code at INTERR must change to the real	*****
; *****   ROM's segment                                                 *****
;------------------------------------------------------------------------------
	PUBLIC	INT00
	PUBLIC	INT01
	PUBLIC	INT03
	PUBLIC	INT04
	PUBLIC	INT06
	PUBLIC	INT07
	PUBLIC	INT0A
	PUBLIC	INT0B
	PUBLIC	INT0C
	PUBLIC	INT0D
	PUBLIC	INT0F
	PUBLIC	UNEXINT
	EXTRN	INTERR:NEAR
	SYNC	INTHDLA
UNEXINT PROC FAR
INT00:
INT01:
INT03:
INT04:
INT06:
INT07:
INT0A:
INT0B:
INT0C:
INT0D:
INT0F:
	JMP	NEAR PTR INTERR
UNEXINT ENDP
	PRGSIZ	<Unexpected Interrupt handler jump>

;------------------------------------------------------------------------------
; Interrupt vector table for Interrupt's 0h -> 7h
;------------------------------------------------------------------------------
	EXTRN	PRTSCR:NEAR		; PRINT SCREEN ROUTINE
	EXTRN	NMIINT:NEAR
	PUBLIC	VECTB1
VECTB1	LABEL	WORD
	DW	INT00		; Interrupt 0h offset
	DW	INT01		; Interrupt 1h offset
	DW	NMIINT		; Interrupt 2h offset
	DW	INT03		; Interrupt 3h offset
	DW	INT04		; Interrupt 4h offset
	DW	PRTSCR		; Interrupt 5h offset
	DW	INT06		; Interrupt 6h offset
	DW	INT07		; Interrupt 7h offset
	PRGSIZA <Interrupt 0h - 7h vector offset table>
;
;------------------------------------------------------------------------------
; Signon message
;------------------------------------------------------------------------------
	PUBLIC	RSTMSG
RSTMSG	LABEL	BYTE
	DB	'ROM BIOS Ver 2.12',0		; Signon message
			; The version # is an indication of the current
			; revision at PHOENIX and should not be changed
			; except by PHOENIX.  If a manufacturer wishes to
			; have a version # and signon message, the macros
			; MANSION1 and MANSION2 in MANEQU.INC can be loaded
			; w/ a string to be displayed.
	PRGSIZ	<ROM BIOS Sign-on message portion>
;------------------------------------------------------------------------------
	IF	ROMSIZ LE (8*1024)
	PNXMAC6 		; Include HEXMSG, CRLFMSG
	ENDIF
;
;------------------------------------------------------------------------------
; Dummy interrupt vector
;------------------------------------------------------------------------------
	SYNC	DUMINTA
	PUBLIC	DUMINT
DUMINT:
	IRET
	PRGSIZ	<Interrupt dummy vector>
;------------------------------------------------------------------------------
;
	SYNC	PRTSCRA
;
	TOTAL
;
CODE	ENDS
	END
