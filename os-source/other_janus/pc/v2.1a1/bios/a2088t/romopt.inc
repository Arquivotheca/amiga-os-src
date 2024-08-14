	IFNDEF LISTINC
.XLIST
.XCREF
	ENDIF
;	NAME	ROMOPT
;	TITLE	Options for the ROM
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program    *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent. 		   *
;***************************************************************************
;
;	Global Equates
FALSE	EQU	0		; Logical conventions
TRUE	EQU	NOT FALSE
;
;
PC	EQU	false	; PC if true
XT	EQU	true	; XT if true
XTSWTH	EQU	true	; If you are using an XT type Switch 1, then set true
PCSWTH	EQU	false	; If you are using a PC type switch 1, then set true
AT	EQU	false
ALTCOMP EQU	FALSE	; If true, then a compatible ALT numpad is done.
BLWAIT	EQU	TRUE	; false inhibits all waits for retrace to access display
BSCOMP	EQU	FALSE	; Do a compatible backspace
CLICKD	EQU	04h	; Key click duration; 4 sounds good; 1 conserves stack
CLICKV	EQU	02h	; Initial key click volume
CLK10	EQU	48	; Effective Clock Rate in MHz times 10 of processor	
CMOSBDR	EQU	false	; Allow tape drives as drive B:
CMOSCFG	EQU	false	; Read system configuration from CMOS
CRTBP	EQU	TRUE	; Do a long beep - short beep if CRT card fails
CRTNICE EQU	false	; Scroll CRT in 80 x 25 graphics card without flicker.
CRTTEST EQU	TRUE	; CRT blanking test
DIAG	EQU	FALSE	; (wds) disable diagnostics 
DMAINIT EQU	TRUE	; True if you want DMA initialization and initial tests
DMATEST EQU	TRUE	; DMA chip test
DSKTST	EQU	TRUE	; Disk test
ENHFDD	EQU	true	; (vtb6.3.90) supports HD FDD's
ERRWAIT EQU	TRUE	; Wait for an F1 key on any error during testing
EXPTEST EQU	false	; Expansion Box, not fully implemented yet
FILBYT	EQU	0FFh	; Fill byte used by the SYNC macro
FDSW	EQU	true	; (vtb 9.3.90) Floppy muxer without disk change signal
GAMTEST EQU	TRUE	; True if you want to check for a Game Port	
HDDELAY EQU	60	; Delay in .1 seconds to allow a hard disk to come up
HDRTY	EQU	1	; # of Hard Disk Retries if code is implemented
I80186	EQU	FALSE	; True if you have an 80186, not supported completely yet
I80286	EQU	FALSE	; True if you have an 80286, not supported completely yet
INTENHD EQU	TRUE	; Unexpected interrupt handler
INTTEST EQU	TRUE	; Interrupt chip test
KEYLOOP EQU	FALSE	; If keyboard not hooked up or bad, loop
KEYTAP	EQU	TRUE	; If key click/tap when character is entered in buffer
KEYTAPS EQU	TRUE	; keytap volume control if true
LIGHTPN EQU	TRUE	; Light Pen support
MAXDRV	EQU	4	; Maximum # of drives in system
MAXPRT	EQU	4	; Maximum # of printers in data tables
MAXSER	EQU	4	; Maximum # of serial ports in data tables
MEMSAVE EQU	03Ch	; Memory preserve option
MEMTEST EQU	TRUE	; True if you want to test memory and not just clear it
MLTMOT	EQU	FALSE	; Multiple disk drive motors on simultaneously
MSGLNG	EQU	TRUE	; Use long messages if true
NMIENAB EQU	TRUE	; Enable NMI at end of power-ons
NMIENHD EQU	TRUE	; Enhanced NMI interrupt handler
NOFLOP	EQU 	true    ; (vtb 9.3.90) Don't report floppy init problems
OKBEEP	EQU	TRUE	; Beep after most diagnostics but before disk test
PRSIS1	EQU	false	; Blank video at vertical retrace time only
PRSIS2	EQU	false	; Un-Blank video at vertical retrace time only
RETRY	EQU	1	; Maximum # of disk boot retries before continuing
ROMNUM	EQU	1	; Number of ROMs	
ROMSIZ	EQU	32*1024 ; Size of ROM, should be multiples of 8k
ROMSET	EQU	false	; Setup for CMOS as part of this BIOS
SCLEAR	EQU	FALSE	; True to clear screen just before boot
SCRCOMP EQU	TRUE	; If true, Do a compatible CRT video interrupt scroll
SLCTSPD EQU	true	; true if we have selectable clock speed
TIMSAV	EQU	TRUE	; True if you want the # of tics past midnight preserved
TIMTEST EQU	false	; (vtb 9.3.90) Timer chip counter 2 test
TURBO	EQU	false	; enable Phoenix method of speed switching
XTKBUF	EQU	TRUE	; Do the keyboard buffer the XT way if true

WAITHLT EQU	false	; halt during some device waits instead of SW looping
LPFAIL	EQU	false	; Loop on all tests except disktest until fail
LPALWY	EQU	TRUE	; Loop always between start to fail point or the end
LPALWS	EQU	FALSE	; Loop always between start to fail point or RAM test
TSTLOOP EQU	TRUE	; Loop back to start point of last failed routine

CRTROM	EQU	TRUE	; True if you want to search for optional CRT ROMs
XTROM	EQU	TRUE	; If you want a to search for optional ROMs
XTPOST	EQU	TRUE	; If you want to loop on the Powerup code
ZIVS	EQU	true	; True leaves vectors for most unexpected ints zeroed
; My global equates (wds)
; Revision A and earlier
; REV_A_FIX	equ	TRUE

MANSION1	MACRO	; optional signon message up to 32 bytes
MANS1	DB	'All Rights Reserved',CR,LF,LF,0
	ENDM
MANSION2	MACRO	; optional add'l signon if room at end of INITCD
MANS2 LABEL BYTE
 DB 'ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿',cr,lf
 DB '³ Commodore A2088T BIOS   Rev. 4.1x          390 657 - 03     ³',cr,lf
IFDEF	REV_A_FIX
 DB '³               For REV 3.1 and REV A Boards only             ³',cr,lf
ENDIF
 DB 'ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´',cr,lf
 DB '³ Copyright (C) 1985 - 1990  by Commodore Electronics Ltd.    ³',cr,lf
 DB '³ Copyright (C) 1984 , 1985  by Phoenix Technologies Ltd.     ³',cr,lf
 DB '³                     All Rights Reserved.                    ³',cr,lf
 DB 'ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ',cr,lf
 DB lf,0

      ENDM
;
.CREF
.LIST
