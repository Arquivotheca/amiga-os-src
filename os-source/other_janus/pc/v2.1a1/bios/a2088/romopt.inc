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
MSGLNG	EQU	TRUE	; Use long messages if true
CLK10	EQU	48	; Effective Clock Rate in MHz times 10 of processor	
FILBYT	EQU	0FFh	; Fill byte used by the SYNC macro
MAXDRV	EQU	4	; Maximum # of drives in system
MAXPRT	EQU	4	; Maximum # of printers in data tables
MAXSER	EQU	4	; Maximum # of serial ports in data tables
RETRY	EQU	1	; Maximum # of disk boot retries before continuing
HDRTY	EQU	1	; # of Hard Disk Retries if code is implemented
ROMSIZ	EQU	16*1024 ; Size of ROM, should be multiples of 8k
KEYTAP	EQU	TRUE	; If key click/tap when character is entered in buffer
KEYTAPS EQU	TRUE	; keytap volume control if true
CLICKV	EQU	02h	; Initial key click volume
CLICKD	EQU	04h	; Key click duration; 4 sounds good; 1 conserves stack
XTKBUF	EQU	TRUE	; Do the keyboard buffer the XT way if true
OKBEEP	EQU	TRUE	; Beep after most diagnostics but before disk test
TIMSAV	EQU	TRUE	; True if you want the # of tics past midnight preserved
HDDELAY EQU	60	; Delay in .1 seconds to allow a hard disk to come up
ERRWAIT EQU	TRUE	; Wait for an F1 key on any error during testing
NMIENHD EQU	TRUE	; Enhanced NMI interrupt handler
NMIENAB EQU	TRUE	; enable NMI at end of power-ons
INTENHD EQU	TRUE	; Unexpected interrupt handler
KEYLOOP EQU	FALSE	; If keyboard not hooked up or bad, loop
MEMSAVE EQU	03Ch	; Memory preserve option
EXPTEST EQU	false	; Expansion Box, not fully implemented yet
CRTTEST EQU	TRUE	; CRT blanking test
DSKTEST EQU	TRUE	; Disk test
DMATEST EQU	TRUE	; DMA chip test
INTTEST EQU	TRUE	; Interrupt chip test
TIMTEST EQU	TRUE	; Timer chip counter 2 test
CRTBP	EQU	TRUE	; Do a long beep - short beep if CRT card fails
SCLEAR	EQU	FALSE	; True to clear screen just before boot
MEMTEST EQU	TRUE	; True if you want to test memory and not just clear it
GAMTEST EQU	TRUE	; True if you want to check for a Game Port	
I80186	EQU	FALSE	; True if you have an 80186, not supported completely yet
I80286	EQU	FALSE	; True if you have an 80286, not supported completely yet
DMAINIT EQU	TRUE	; True if you want DMA initialization and initial tests
CRTNICE EQU	false	; Scroll CRT in 80 x 25 graphics card without flicker.
LIGHTPN EQU	TRUE	; Light Pen support
ALTCOMP EQU	FALSE	; If true, then a compatible ALT numpad is done.
BSCOMP	EQU	FALSE	; Do a compatible backspace
SCRCOMP EQU	TRUE	; If true, Do a compatible CRT video interrupt scroll
PRSIS1	EQU	false	; Blank video at vertical retrace time only
PRSIS2	EQU	false	; Un-Blank video at vertical retrace time only
BLWAIT	EQU	TRUE	;false inhibits all waits for retrace to access display
XTPOST	EQU	TRUE	; If you want to loop on the Powerup code
ZIVS	EQU	true	; True leaves vectors for most unexpected ints zeroed
WAITHLT EQU	false	; halt during some device waits instead of SW looping
LPFAIL	EQU	false	; Loop on all tests except disktest until fail
LPALWY	EQU	TRUE	; Loop always between start to fail point or the end
LPALWS	EQU	FALSE	; Loop always between start to fail point or RAM test
TSTLOOP EQU	TRUE	; Loop back to start point of last failed routine
MLTMOT	EQU	FALSE	; Multiple disk drive motors on simultaneously
CRTROM	EQU	TRUE	; True if you want to search for optional CRT ROMs
XTROM	EQU	TRUE	; If you want a to search for optional ROMs
MANSION1	MACRO	; optional signon message up to 32 bytes
MANS1	DB	'All Rights Reserved',CR,LF,LF,0
	ENDM
MANSION2	MACRO	; optional add'l signon if room at end of INITCD
MANS2 LABEL BYTE

 DB 'ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿',cr,lf
 DB '³ Commodore PC-Emulator BIOS Rev. 3.6         380 788 - 06    ³',cr,lf
 DB 'ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´',cr,lf
 DB '³ Copyright (C) 1985 - 1989 by Commodore Electronics Ltd.     ³',cr,lf
 DB '³ Copyright (C) 1984 - 1989 by Phoenix Technologies Ltd.      ³',cr,lf
 DB '³                     All Rights Reserved.                    ³',cr,lf
 DB 'ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ',cr,lf
 DB lf,0

      ENDM
;
.CREF
.LIST
