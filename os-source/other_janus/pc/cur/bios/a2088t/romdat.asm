	NAME	ROMDAT
	TITLE	SYSTEM ROM DATA
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program    *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent. 		   *
;***************************************************************************
;
; SYSTEM ROM DATA MODULE
;	THIS MODULE CONTAINS DATA FOR THE DEVICE SERVICE ROUTINES IN
;	THE SYSTEM ROM.  DATA AREAS INCLUDED TO DATE ARE:
;
;			     Byte	  Byte
;		 Name	    Offset	 Length
;		------	    ------	 ------
;		EIADSR	      00h	    8
;		LPDSR	      08h	    6
;		KYBDSR	      17h	   39
;		DSKDSR	      3Eh	   11
;		CRTDSR	      49h	   30
;		TIMDSR	      6Ch	    5
;		LPDSR	      78h	    4
;		EIADSR	      7Ch	    4
;
;	Written: 09/29/83
;	Revised: 06/26/84 Ira J. Perlow - minor changes in documentation
;		 12/  /84 Ira J. Perlow - added AT areas
;		 02/  /85 Ira J. Perlow - added Enhanced Graphics board areas
;					    and changed to absolute areas
;	Revised: 11/13/85 Dieter Priess/Commodore - changed XROM area to
;			  KEYEXT pointer for national keyboard support.
;
;------------------------------------------------------------------------------
;
INCLUDE ROMOPT.INC		; ROM Options file - must be included 1st
;
INCLUDE MACROS.INC		; ROM Macros file
;
;------------------------------------------------------------------------------
;
	OUT1	<ROMDAT - ROM Data areas definitions>
;
;
;The following locations are relative to Segment 00h
;
INTVEC	SEGMENT AT 0000h
	PUBLIC	CRTCINT
	PUBLIC	DPBPTR
	PUBLIC	RAMFNT
	PUBLIC	OLDCRT
	PUBLIC	NEWPRM
	PUBLIC	FNT128
;
	ORG	13h*4
	PUBLIC	DSKDSRI
DSKDSRI 	DD	?	; 13h - Disk Device Service Routine interrupt
	ORG	1Dh*4			; Origin is at Interrupt vector 1Dh
CRTCINT 	DD	?	; 1Dh - Offset of pointer CRTC INIT table
DPBPTR		DD	?	; 1Eh - Pointer to Disk Parameter Block
RAMFNT		DD	?	; 1Fh - User generated font with offset, segment

	ORG	40h*4
	PUBLIC	FDDSRI
FDDSRI		DD	?	; 40h - Vector of original Disk DSR (INT 13h)
				;	before optional Rom replaced it
	PUBLIC	HDPRM0
HDPRM0		DD	?	; 41h - Vector for hard disk #0 parameters
OLDCRT		DD	?	; 42h - Vector of original CRT DSR (INT 10h)
				;	before optional Rom replaced it
NEWPRM		DD	?	; 43h - Vector of new CRT parameters when
				;	using enhanced graphics
FNT128		DD	?	; 44h - Vector of ROM Font for 1st 128
				;	characters in mode 4, 5 or 6.  Any
				;	other graphic mode has all 256
				;	characters
		DD	?	; 45h - Reserved for BIOS
	PUBLIC	HDPRM1
HDPRM1		DD	?	; 46h - Vector for hard disk #1 parameters
		DD	?	; 47h - Reserved for BIOS
CORKYB		DD	?	; 48h - Vector for cordless keyboard
				;	translation (JR)
KYBTRN		DD	?	; 49h - Vector for Non-keyboard scan code
				;	translation table (JR)
;
; Interrupts 4Ah -> 5Fh - Reserved for BIOS
;
;
; Interrupts 60h -> 67h - Reserved for user software interrupts
;
	ORG	67h*4
EMMINT		DD	?	; 67h - Vector for Lotus/Intel Expanded Memory
				;	Manager support interrupt
				;	as of Release 3.0 4/2/85
;
; Interrupts 70h -> 77h - Reserved for slave interrupt controller (AT)
;
	ORG	76h*4
	PUBLIC	HDINT
HDINT		DD	?	; 76h - Vector for hard disk interrupt,
;
; changed ------------------------------------------------------------------
;
	ORG	0C0h*4		; C0h - Vector for GWBASIC FRCINT and MAKINT
				; Aux storage for XROM init code
	PUBLIC	XROM_OFF
	PUBLIC	XROM_SEG

XROM_OFF	DW	?	; OLD 67h - Address offset of optional ROM
XROM_SEG	DW	?	; OLD 69h - Address segment of optional ROM

				;	Interrupt controller 1, level 6
INTVEC	ENDS
;
;------------------------------------------------------------------------------
;
;The following locations are relative to Segment 40h
;
ROMDAT	SEGMENT AT 0040h
;
; EIA Communication data
;
	ORG	0000h
	PUBLIC	EIADRTBL
EIADRTBL DW	4 DUP (?)	; 00h - EIA Communication Base I/O port address table
;
; Printer data
;
	PUBLIC	LPADRTBL
LPADRTBL DW	4 DUP (?)	; 08h - Parallel Printer Base I/O port address table
;
; Miscellaneous data
;
	PUBLIC	DEVFLG
	PUBLIC	SPEED
;	PUBLIC	MANTST
	PUBLIC	MEMSIZE
	PUBLIC	LSTKEY
	PUBLIC	CLICKP
DEVFLG	DW	?		; 10h - Devices Installed word
SPEED   DB	0		; 12h - SpeedFlag for Faradays 2010A
                               	;	BIT0: 0/1=14.318/28.636 CRYSTAL SPEED
                               	;	BIT1-5: RESERVED 
				;	BIT6,7: CLOCK SPEED 
;MANTST	DB	?		; 12h - Manufacturer Test
MEMSIZE DW	?		; 13h - Available system memory size in k
LSTKEY	DB	?		; 15h - Last key depressed on keyboard
CLICKP	DB	?		; 16h - Click duration of key klick	
;
; Keyboard data
;
	PUBLIC	SHFLGS
	PUBLIC	SHFLGS2
	PUBLIC	ALTDATA
SHFLGS	DB	?		; 17h - SHIFT STATE FLAGS
SHFLGS2 DB	?		; 18h - SECONDARY SHIFT STATE FLAGS
ALTDATA DB	?		; 19h - ALT KEYPAD ENTRY
; These pointers must be relative to data segment of 40h in case user comes
;and blows away the current value
	PUBLIC	KBGET
	PUBLIC	KBPUT
	PUBLIC	KBUFR
KBGET	DW	?		; 1Ah - KEYBOARD BUFFER GET POINTER
KBPUT	DW	?		; 1Ch - KEYBOARD BUFFER PUT POINTER
KBUFR	DW	16 DUP (?)	; 1Eh - START OF KEYBOARD BUFFER
;
; Floppy Disk data
;
	PUBLIC	DRVSTAT
	PUBLIC	FDMOTS
	PUBLIC	FDTIMO
	PUBLIC	ERRSTAT
	PUBLIC	DSKST
DRVSTAT 	DB	?	; 3Eh - DRIVE STATUS
FDMOTS		DB	?	; 3Fh - Floppy Disk Motor Status
FDTIMO		DB	?	; 40h - Floppy Disk Timeout value
ERRSTAT 	DB	?	; 41h - DISK STATUS
;
; These 7 bytes also used by hard disk
;
	PUBLIC	HDCMD
HDCMD	LABEL	BYTE		; hard disk command/param port copies
;
DSKST	DB	7 DUP (?)	; 42h - FLOPPY DISK CONTROLLER STATUS
;
;
; CRT Data
;
	PUBLIC	CRTMODE
	PUBLIC	SCRNWID
	PUBLIC	SCRNLEN
	PUBLIC	PAGADDR
	PUBLIC	CURCOOR
	PUBLIC	CURTYPE
	PUBLIC	DSPYPAG
	PUBLIC	CRTADDR
	PUBLIC	MSRCOPY
	PUBLIC	PALETTE
CRTMODE 	DB	?	; 49h - Current CRT mode
SCRNWID 	DW	?	; 4Ah - CRT screen column width
SCRNLEN 	DW	?	; 4Ch - Length of screen in bytes
PAGADDR 	DW	?	; 4Eh - Offset address of current display page
CURCOOR 	DW	8 DUP (?) ; 50h - Cursor coordinates for 8 pages - row,column
CURTYPE 	DW	?	; 60h - Current cursor mode setting
DSPYPAG 	DB	?	; 62h - Current display page
CRTADDR 	DW	?	; 63h - Base I/O port address of CRT controller
MSRCOPY 	DB	?	; 65h - Mode Select Register copy
PALETTE 	DB	?	; 66h - CRT Color Palette register copy
;
;
;
;
; Changed ----------------------------------------------------------------
	       PUBLIC	KEYEXT
KEYEXT	       DD	?	; 67H TO 6AH Indirect call pointer for
				; national keyboard drivers
;--------------------------------------------------------------------------
;
	PUBLIC	INTLST
INTLST		DB	?	; 6Bh - Last interrupt that occured,
				;	0FFh if a software interrupt or
				;	00 if unexspected interrupt handler 
				;	is switched off.
;
; TIMER DATA
;
	PUBLIC	LOTIME
	PUBLIC	HITIME
	PUBLIC	HOUR24
LOTIME		DW	?	; 6Ch - Least significant word of timer tick count
HITIME		DW	?	; 6Eh - Most significant word of timer tick count
HOUR24		DB	?	; 70h - 24-Hout timer tick rollover counter
;
; Keyboard data
;
	PUBLIC	BRKFLG
	PUBLIC	RSTFLG
BRKFLG		DB	?	; 71h - Keyboard Break flag, Bit 7 = "1" when break
RSTFLG		DW	?	; 72h - Reset flag, when not 1234h, it's a cold boot
;
; Hard Disk Data Area (AT only)
;
	PUBLIC	HDSTAT
	PUBLIC	HDNUMB
	PUBLIC	HDCTRL
	PUBLIC	HDPORT
HDSTAT		DB	?	; 74h - Hard Disk SW status of last INT 13h operation
HDNUMB		DB	?	; 75h - # of Hard Disks
HDCTRL		DB	?	; 76h - Hard Disk Control byte copy
HDPORT		DB	?	; 77h - Hard Disk Port offset - not used 2/85
;
; Printer data
;
	PUBLIC	LPTOTBL
LPTOTBL DB	4 DUP (?)	; 78h - Parallel Printer timeout table
;
; EIA Communication data
;
	PUBLIC	EIATOTBL
EIATOTBL DB	4 DUP (?)	; 7Ch - EIA communication timeout table
;
; These pointers must be relative to data segment of 40h in case user comes
;and blows away the current value
	PUBLIC	KBXTGET
	PUBLIC	KBXTPUT
KBXTGET 	DW	?	; 80h - Keyboard buffer start location pointer
KBXTPUT 	DW	?	; 82h - Keyboard buffer end   location pointer
;
; Enhanced Graphic Adaptor (EGA) Data Area
;
	PUBLIC	CRTROW
	PUBLIC	CHRSIZ
	PUBLIC	EGAINF
	PUBLIC	EGAIN2
CRTROW		DB	?	; 84h - # of CRT rows minus 1
CHRSIZ		DW	?	; 85h - # of bytes per character in Font table
EGAINF		DB	?	; 87h - EGA miscellaneous information
				;  Bit 7 - High bit of mode, 1 = no clear
				;  Bit 6,5 - EGA memory size in 64k chunks
				;		00 = 64k, 01 = 128k, etc.
				;  Bit 4 - Reserved
				;  Bit 3 -	0 = EGA active
				;		1 = EGA not active
				;  Bit 2 -	0 = Write to CRT anytime
				;		1 = wait for display enable
				;  Bit 1 -	1 = monochrome attached to EGA
				;  Bit 0 -	0 = emulate cursor type
EGAIN2		DB	?	; 88h - EGA miscellaneous information 2
				;  Bit 7-4	Feature bits
				;  Bit 3-0	Dip switches
;
;
;
	DB	(8Bh-089h) DUP (?)	; 89h - Unused
;
; Floppy Disk Data Area (AT only)
;
	ORG	008Bh
	PUBLIC	FDRATE
FDRATE		DB	?	; 8Bh - Last Floppy Disk data rate selected
;
; Hard Disk Data Area (AT only)
;
	PUBLIC	HDSTRG
	PUBLIC	HDERRG
	PUBLIC	HDINTR
	PUBLIC	HDCFLG
HDSTRG		DB	?	; 8Ch - Hard Disk status register copy
HDERRG		DB	?	; 8Dh - Hard Disk error register copy
HDINTR		DB	?	; 8Eh - Hard Disk Interrupt flag
HDCFLG		DB	?	; 8Fh - Hard Disk Controller Flag = 1 if
				;   using combined controller, else 0
;
; Advanced Floppy Data Area (AT only)
;
	PUBLIC	FDMED
	PUBLIC	FDOPER
	PUBLIC	FDTRCK
	PUBLIC	FDRESV
FDMED	DB	2 DUP (?)	; 90h - Drive 0/1 Media state
FDOPER	DB	2 DUP (?)	; 92h - Drive 0/1 Operation state
FDTRCK	DB	2 DUP (?)	; 94h - Drive 0/1 Current track
FDRESV	DB	?		; 96h - Floppy disk reserved extra byte
;
; Keyboard LED Data Area (AT only)
;
	PUBLIC	KYBFLG
KYBFLG		DB	?	; 97h - Keyboard LED and Flags Data Area (AT only)
;
; Real-Time Clock Data Area (AT only)
;
	PUBLIC	WTADDR
	PUBLIC	WTSGMT
	PUBLIC	WTCNTL
	PUBLIC	WTCNTH
	PUBLIC	WTACTF
WTADDR		DW	?	; 98h - User wait flag address offset
WTSGMT		DW	?	; 9Ah - User wait flag address segment
WTCNTL		DW	?	; 9Ch - Wait count low word
WTCNTH		DW	?	; 9Eh - Wait count high word
WTACTF		DB	?	; 0A0h - Wait active flag
;
;
;
	DB	(0A8h-0A1h) DUP (?)	; 0A1h - Unused
;
; Enhanced Graphic Adaptor (EGA) Data Area
;
	PUBLIC	EGATBL
EGATBL	DD	?		; 0A8h - Pointer to table of EGA pointers
;
;
;
	DB	(0B0h-0ACh) DUP (?)	; 0ACh - 0AFh - Unused
;
; Miscl. ROM data locations
;
	PUBLIC	DLY_CNT
DLY_CNT		DB	?	; 0B0h - Delay count for DLY100
;
	DB	(0100h-0B1h) DUP (?)	; 0ACh - Unused
;
; Print Screen Status Flag (PC, XT, JR, AT)
;
	PUBLIC	PRTFLG
PRTFLG	DB	?		; 100h - Flag indicating a Print Screen in
				;   progress if a 1
	DB	(104h-101h) DUP (?)	; 101h - Unused
	DB	?		; 104h - Single drive status (Drive A or B)
	DB	(110h-105h) DUP (?)	; 105h - Unused
	DW	?		; 110h - BASIC's segment address store
	DW	?		; 112h - BASIC's timer interrupt vector offset
	DW	?		; 114h - BASIC's timer interrupt vector segment
	DW	?		; 116h - BASIC's CTL-Break interrupt vector
				;		offset
	DW	?		; 118h - BASIC's CTL-Break interrupt vector
				;		segment
	DW	?		; 11Ah - BASIC's disk error interrupt vector
				;		offset
	DW	?		; 11Ch - BASIC's disk error interrupt vector
				;		segment
;
ROMDAT	ENDS
;
;------------------------------------------------------------------------------
;
	END
