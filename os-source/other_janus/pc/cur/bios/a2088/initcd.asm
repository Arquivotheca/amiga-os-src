	NAME	INITCD
	TITLE	Initialization and Bootstrap Logic
	PAGE	59,132
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program    *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent. 		   *
;***************************************************************************
;
; Initialization and bootstrap code
;	This module contains hardware initialization routines
;	that are initiated by a power up jump from
;	rom location ffff:0000.
;
;	WRITTEN: 02/20/84 by Robert G. Vandette, Phoenix Software Associates
;	REVISED: 04/23/84 rgv added XT hard disk rom support
;	UPDATE : 07/25/84 -> 11/07/84  IJP
;
;	UPDATE by Commodore Business Machines Inc.:
;	V2.01:	 10/28/85 -> 12/04/85 by Torsten Burgdorff/BSW
;	HY2.1:	 03/03/86	      by TB  
;	HY2.2:	 03/05/86	      by TB  
;	HY2.3:	 03/11/86	      by TB
;	HY2.4:	 03/14/86	      by TB
;	HY2.5:	 08/20/86	      special printer strobe handling
;	HY2.6:	      /87	      release
;
;	HY3.0:	 10/24/86	      by TB for PC-Emulator of A2500
;	HY3.1:	 12/01/86	      include timer setup	    
;	HY3.2:	 12/17/86	      include 8087 recognize, old prn strobe
;	HY3.3:   02/26/87	      toggle busy line level in PRTDSR	
;       HY3.4:	 04/27/87	      set Faradays 8087 flag correct	 
;       HY3.6:	 10/27/89	      reliable sync between 8088 and 68000	 
;				      no locking of Faraday registers	
; 05/06/91 rsd	do the 3.6.1 fix such that it rechecks floppies.  the DEVFLG
;		wasn't getting set up after ctrl-alt-del.
;
;------------------------------------------------------------------------------
;
INCLUDE ROMOPT.INC		; ROM Options file - must be included 1st
INCLUDE ROMEQU.INC		; ROM General equate file
INCLUDE MACROS.INC		; ROM Macros file
;
hydra	equ    true		; true for hydra bios
faraday equ    true		; true for bios with faraday chip
sidecar equ    false		; true for sidecar bios
;
;- ADDED (TB) -----------------------------------------------------------------
;
	OUT1	<INITCD - ROM Initialization code>
;
; Macro to output testcode to both possible printer ports ($0378h or $03BCh).
; Testcode will be displayed with Commodore DIAGNOSTIC DISPLAY.
;
; Output:  AL = Testcode
;	   DX = $03BCh (Baseaddr. of 2nd printer)
;
;
DISPLAY MACRO	CODE
;
	.RADIX	16
	IFB	<CODE>
	MOV	AL,AH
	ELSE
	MOV	AL,CODE 	; Test # in AL
	ENDIF
	MOV	DX,PRTPT1
	OUT	DX,AL		; Output to 1st printer

	MOV	DX,PRTPT2
	OUT	DX,AL		; Output to 2nd printer 
				; (monochrome card compatible)
	.RADIX	10
;
	ENDM
;
;- ADDED (TB) -----------------------------------------------------------------
;
; Macro to provide a trigger for storage scopes and logic analyzers.
; Toggles Bit 3 of printer control port of both possible printer interfaces.
; This line is /SLCTIN (Pin 17 of the printer connector).
;
; Output: AL = garbage	
;	  DX = $03BCh
;
;
TRIGGER MACRO
;
	MOV	DX,PRTPT1+2	  
	IN	AL,DX		; read printer control port
	XOR	AL,OCW3 	; toggle Bit3 (/SLCTIN)
	OUT	DX,AL
	XOR	AL,OCW3 	; toggle Bit3 again
	OUT	DX,AL
;
	MOV	DX,PRTPT2+2	; same procedure with 2nd printer
	IN	AL,DX		; read printer control port
	XOR	AL,OCW3 	; toggle Bit3 (/SLCTIN)
	OUT	DX,AL
	XOR	AL,OCW3 	; toggle Bit3 again
	OUT	DX,AL
;
	ENDM
;
;- ADDED (TB) -----------------------------------------------------------------
;
; Macro to install a temporary stack in ROM  
; Set SS = CS and adjust SP to point to return address in ROM		    
;
; Input : LBA = return address	
;
; Output: AL = garbage	
;	  DX = $03BCh
;
;
STACKINIT MACRO LBA
;
	MOV	AX,CS
	MOV	SS,AX		; adjust stack segment to ROM segment
	MOV	SP,OFFSET LBA	; adjust stack pointer to return address in ROM
;
	ENDM
;
;------------------------------------------------------------------------------
;
; Macro to halt for certain test failures.  There are 3 choices which one
; could make, each with their own advantages and disadvantages.
;
TSTHLT	MACRO
	IF	XTPOST
	  IF	LPFAIL		; Loop until fail point
	    HLT 		; Uses the least amount of code and places
;				;   machine in a static state.
	  ENDIF
	  IF	LPALWY OR LPALWS	; Loop always from start to fail
		JMP	TSTLP	; Jump to code to decide if we should loop.
				; Loop forever from system reset to fail point
				;   will allow oscilloscopes to trigger and
				;   observe operation
	  ENDIF
	  IF	NOT (LPFAIL OR LPALWY OR LPALWS)
		%OUT	Error in XTPOST option.
		%OUT	  Either LPFAIL or LPALWY or LPALWS must be true
	  ENDIF
	ELSE
	  HLT			; Uses the least amount of code and places
;				;   machine in a static state.
;	  JMP	SHORT $ 	; Loop to self allows certain debuggers to
;				;   maintain control on failures
;	  JMP	SYSRESET	; Loop forever from system reset to fail point
;				;   will allow oscilloscopes to trigger and
;				;   observe operation
	ENDIF
	ENDM
;
;------------------------------------------------------------------------------
;
DGROUP	GROUP	ROMDAT
INTVEC	SEGMENT AT 0000h	; The following are relative to Segment 00h
	EXTRN	CRTCINT:DWORD		; Offset of pointer CRTC INIT table
	EXTRN	RAMFNT:DWORD		; Offset of user generated font table
INTVEC	ENDS
;
;------------------------------------------------------------------------------
;
ROMDAT	SEGMENT AT 0040h	;The following are relative to Segment 40h
	EXTRN	MEMSIZE:WORD
	EXTRN	KBUFR:WORD
	EXTRN	KBGET:WORD
	EXTRN	KBPUT:WORD
	EXTRN	LSTKEY:BYTE		; Last keybord scan code
	EXTRN	KEYEXT:WORD		; POINTER TO EXTENDED KEYBOARD 
	EXTRN	LPADRTBL:WORD
	EXTRN	EIADRTBL:WORD
	EXTRN	LPTOTBL:WORD
;
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
;
	EXTRN	EIATOTBL:WORD
	EXTRN	XROM_OFF:WORD
	EXTRN	XROM_SEG:WORD
	EXTRN	KBXTGET:WORD
	EXTRN	KBXTPUT:WORD
	EXTRN	LOTIME:WORD	; LEAST SIGNIFICANT TIMER COUNTER
	EXTRN	HITIME:WORD	; MOST SIGNIFICANT TIMER COUNTER
	EXTRN	HOUR24:byte	; 24-HOUR ROLLOVER COUNTER
	EXTRN	RSTFLG:WORD	; RESET FLAG MEMORY OFFSET
	EXTRN	CLICKP:BYTE	;
;
	EXTRN	INTLST:BYTE	; Last Interrupt that was called
	EXTRN	PRTFLG:BYTE	; Print screen flag
ROMDAT	ENDS
;
;- ADDED (TB) -----------------------------------------------------------------
;
; This module provides far labels at the diagnostic ROM location .
; At present we have three different entrys:
;					    - DIAG_NORM ($C000:3)
;					    - POST_BIOS ($C000:7)
;					    - PRE_BIOS	($C000:B)
;
;
; To decide which ROM is installed, the diagnostic ROM's have three
; checkwords. To define their offsets we need this module also.
;
;					    - CHECKWORD 1 ($C000:0 = AA55h)
;					    - CHECKWORD 2 ($C000:5 = 6A65h)
;					    - CHECKWORD 3 ($C000:9 = 656Ah)
;
;------------------------------------------------------------------------------
;
DIAG_SEG SEGMENT AT 0C000h	 ; The following are relative to Segment C000h
ASSUME CS:DIAG_SEG
;
;
		ORG   0
CHECKWORD1:			 ; = AA55h, if any ROM present
;
		ORG   2
DIAG_LENGTH	LABEL BYTE	 ; length of ROM in 256-bytes blocks
;
		ORG   3
DIAG_NORM	LABEL FAR	 ; normal rom-search entry
				 ; system is initialized
;
		ORG   5
CHECKWORD2:			 ; = 6A65h, if ROM contains software for       
				 ;	    easier trouble shooting
		ORG   7 
POST_BIOS	LABEL FAR	 ; BIOS error entry
				 ; system ready for boot
;
		ORG   9
CHECKWORD3:			 ; = 656Ah, if ROM contains self diagnostic
				 ;	    software (production diagnostic)
		ORG   11
PRE_BIOS	LABEL FAR	 ; entry after reset and power-up 
				 ; no setup is done !!!
;
DIAG_SEG ENDS
;
;------------------------------------------------------------------------------
;
CGROUP	GROUP	CODE
CODE	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:CODE,DS:NOTHING,ES:NOTHING
;
	EXTRN	DUMINT:NEAR		; Dummy interrupt vector
	EXTRN	SETDS40:NEAR		; Set DS to ROM data segment (0040h)
	EXTRN	CHKSM8:NEAR		; Checksum an 8k area of memory
	EXTRN	CHKSUM:NEAR		; Checksum an area of memory
	EXTRN	CLRMEM:NEAR		; Clear area of memory to 0
	EXTRN	PRTHEX:NEAR		; Print message followed by hex message
	EXTRN	PRTHE0:NEAR		; Print hex message
	EXTRN	PRTHE1:NEAR		; Print h, cr,lf message
;
	EXTRN	WRTCRLF:NEAR
	EXTRN	WRTOUT:NEAR
	EXTRN	WRTINL:NEAR
	EXTRN	DECMLW:NEAR
	EXTRN	HEXW:NEAR
	EXTRN	HEXB:NEAR
	EXTRN	HEXN:NEAR
	EXTRN	GETKEY:NEAR
	EXTRN	GETKE0:NEAR
	EXTRN	TSTSIZ:NEAR
	EXTRN	TSTSIZA:NEAR
	EXTRN	KYINON:NEAR
	EXTRN	KYINOF:NEAR
	EXTRN	NMIENB:NEAR
	EXTRN	NMIDSB:NEAR
	EXTRN	KBDDMY:NEAR
	EXTRN	BTMSG0:BYTE
	EXTRN	BEEP:NEAR
;
	PUBLIC	ROMBGN
ROMBGN: 					; Beginning of ROM
;
FILLSIZ EQU	0FFFFh-ROMSIZ+1
	SYNC	FILLSIZ
	RESYNC	0E000h
;
	MANSION1		; Padded space or MANufacturer's SIgnON part 1
	PRGSIZ	<Reserved / Optional Mfr Signon part 1> ; Display program size
;
	SYNC	CPYNOTA
	PUBLIC	CPYNOT
CPYNOT:
	DB	"Copyright 1986 "
	PUBLIC	CPYMSG
CPYMSG:
	DB	"Commodore Electronics Ltd.",CR,LF,0
	PRGSIZ	<Commodore Copyright Notice>	  ; Display program size
 
;- CHANGED (TB) ---------------------------------------------------------------
;
;	System reset and CTRL-ALT-DEL entry points
;
;------------------------------------------------------------------------------
	SYNC	RESETA
	PUBLIC	RESET
	PUBLIC	SYSRESET
RESET	PROC	NEAR			; power up entry
	ASSUME	DS:ROMDAT,ES:NOTHING
;
	CLI
	MOV	DS,WORD PTR CS:DS40H
	MOV	WORD PTR RSTFLG,0	; clear reset flag, because this flag 
					; survive a short power-off-on sequence
;				    
SYSRESET LABEL NEAR			; ctrl-alt-del entry
KEYRESET:				; old label
	CLI				; no interrupts 
	CLD				; loop increment
;
; Don't allow non maskable interrupts
;
	MOV	AL,NMIOFF	
	OUT	NMIMSK,AL
;
; Stop disk drives
;
	MOV	DX,FDCDOR
	OUT	DX,AL			; still AL = 0
;
;  Enable XT extender board  
;
	MOV	DX,EXPCMD		; Expansion box command
	MOV	AL,01h			;   to enable
					; ... and set high resolution on CRTs
	OUT	DX,AL			; bus delay if PC, next code will
					;   suffice		
;
;  Disable Crt's
;
	mov	dx,CRTCTL+MSRADR	; disable B/W video
	out	dx, al
	mov	DL,LOW (COLORCTL+MSRADR) ; disable Color video
	out	dx, al			; ... and set 80 x 25 mode
;
;- ADDED (TB) -----------------------------------------------------------------
;						      
; Checkpoint 0 :  8088 CPU
;		  Check registers and flags 
;
; Test # on printer port :  01h
;
; Exit :  OK -> all registers clear
;	 BAD -> System halted
; 
;------------------------------------------------------------------------------
;
public	t_cpu
T_CPU:	
;					; Loop back address
	DISPLAY 01			; Display test code
	TRIGGER 			; provide for trigger pulse
;							
	MOV	AX,0FFFFH		; all bits on 
TC1:					; Test registers now
	MOV	SI,AX			; move pattern through registers
	MOV	DS,SI
	MOV	DX,DS
	MOV	DI,DX
	MOV	ES,DI
	MOV	BX,ES
	MOV	BP,BX
	MOV	SP,BP
	MOV	SS,SP
	MOV	CX,BP
	CMP	AX,CX			; Register failed ?
	JNE	HLT0
	SUB	AX,05555H		; change test pattern
	JNC	TC1			; Loop end ?
;  
TC2:					; Test flags now
	MOV	AX,0062H
	SUB	AL,65H			; FLAGS: A,C,S=1    P,O,Z=0
	JNC	HLT0			; ERR IF C=0
	JGE	HLT0			; ERR IF S=0
	JO	HLT0			; ERR IF O=1
	JP	HLT0			; ERR IF P=1
	JZ	HLT0			; ERR IF Z=1
	ADD	AL,25H			; FLAGS: A,C,P=1    O,S,Z=0
	JS	HLT0			; ERR IF S=1
	JNP	HLT0			; ERR IF P=0
	LAHF
	MOV	CL,4
	RCL	AH,CL			; shift A-flag into C-flag
	JNC	HLT0			; ERR IF C(A)=0
	CMP	AL,22H			; FLAGS:   P,Z=1  A,C,S,O=0
	JNZ	HLT0			; ERR IF Z=0
	JC	HLT0			; ERR IF C=1
	LAHF
	MOV	CL,4
	RCL	AH,CL			; shift A-flag into C-flag
	JNC	T_PREBIOS		; ERR IF   C(A)=1
					;     ELSE Test done !
HLT0:	HLT				; CPU ERROR DETECTED
	PRGSIZ	< Check: CPU >
;
;- ADDED (TB) -----------------------------------------------------------------
;						      
; 1. Diagnostic Checkpoint :
;
; This routines looks for a diagnostic ROM at location $C000:0 and enter it
; at @ offset 11 if present. This PRE-BIOS entry is used for special test
; routines while production tests. After this entry all tests and system
; initialisations have to be done by the diagnostic routine.
;								 
; Input: DIAG_SEG   =  segment of diagnostic ROM ($C000h)
;	 XROM_CHECK =  extra rom marker 1 (AA55h)
;	 DIAG_CHECK =  diag. rom marker 3 (656Ah)
;	 PRE_BIOS   =  offset of PRE-BIOS entry  
;
;------------------------------------------------------------------------------
;
public	t_prebios
T_PREBIOS:				; (BIOS 3.3)
	MOV	AX,DIAG_SEG  
	MOV	DS,AX			; Point to diagnostic rom
ASSUME	DS:DIAG_SEG
;
	CMP	WORD PTR CHECKWORD1,XROM_CHECK
	JNE	T_ROM			; is there an extra rom ?
;
	CMP	WORD PTR CHECKWORD3,DIAG_CHECK
	JNE	T_ROM			; ..and is it the diagnostic rom ?
;
	JMP	DIAG_SEG:PRE_BIOS	; YES -> jump far to PRE-BIOS entry
					; NO  -> carry on with normal BIOS
;
;- ADDED (TB) -----------------------------------------------------------------
;						      
; Checkpoint 1 :  BIOS CHECKSUM
;		  Calculates checksum and check result for 0.
;		  The checksum is performed by calculating the 8 bit
;		  cumulative sum of all bytes in the ROM. Any carries 
;		  out of the cumulative sum are ignored during summation.
;
; Test # on printer port :  30h
;								 
; Input: ROM_SEG = segment of ROM to test ($F000h)
;	 ROMSIZE = length of ROM in bytes ($4000h=16k)
;	 ROM_OFF = offset of ROM ($C000h)
;
; Exit :  OK -> AX,CX = 0 ; DS,BX contains garbage
;	 BAD -> System halted
; 
;------------------------------------------------------------------------------
;
LBA1	DW	$+2			; Label to return without stack
public	t_rom
T_ROM:
	DISPLAY 30			; Display test code
	TRIGGER 			; provide for trigger pulse
;
	MOV	AX,ROM_SEG		; Init segment	       
	MOV	DS,AX			; DS points to ROM segment (F000h)
;
	MOV	CX,ROMSIZE		; # of bytes to count
	XOR	BX,BX			; init counter
;
SUM_LOOP:
	ADD	AL,BYTE PTR [ROM_OFF+BX] ; add byte without carry 
	INC	BX
	LOOP	SUM_LOOP

	OR	AL,AL			; ..summed to 0 ?
	JE	T_TIMER 		; CHECKSUM OK ! 
;
	STACKINIT LBA1			; init stack in ROM	  
	JMP	DIAGNOSTIC		; ..FAILED, call for help
	PRGSIZ	< Check: ROM checksum >
;
;- CHANGED (TB) --------------------------------------------------------------
;						      
; Checkpoint 2 :  8253 TIMER 1 (Memory refresh counter)
;		  Make sure that it toggles all data bits both ways when 
;		  it's running. Test this by ORing and ANDing the contents
;		  into a preset pattern.
;
; Test # on printer port :  10h
;
; Exit :  OK -> Timer 1 is been setup for RAM refresh procedure
;	 BAD -> System halted or test will start again
; 
;------------------------------------------------------------------------------
;
LBA2	DW	$+2			; Label to return without stack
public	t_timer
T_TIMER:	     
;
	DISPLAY 10			; Display test code
	TRIGGER 			; provide for trigger pulse
;
	if	faraday
	 jmp	 tim1t1 		; skip this test, because faraday's 
	else				; clock emulation is not compatible
 
;
	MOV	AL,41h			; Disable speaker gate
	OUT	PPIB,AL
;
; Set modes of the timers as appropriate
;
	MOV	DX,PITMD
	MOV	AL,PITSL1+PITRL+PITMD3	; set up Memory refresh timer for test
	OUT	DX,AL			;   mode 3, square wave
					; Read/load LSB 1st, then MSB
;
; Set up memory refresh timer
;
	DEC	DX
	DEC	DX			; set count to 0000
	MOV	AL,0			; delay 1000ns for chip I/O recov. time
	OUT	DX,AL			; Store LSB 1st
;
; Since we have to do the following anyway, do it here to increase delay
;   between consecutive accesses to a peripheral chips
	XOR	CX,CX			; Set timeout for test,
	MOV	BX,000FFh
;
	OUT	DX,AL			;   now store MSB
;
TIM1TS:
	INC	DX
	INC	DX
	MOV	AL,PITSL1+PITLCH	; Latch counter
	OUT	DX,AL			; 
;
	CMP	BX,0FF00h		; If we reach this value then it's good
	JZ	TIM1T1			; Delay 1000ns for chip I/O recov. time
;
	DEC	DX			; Now read latched value
	DEC	DX
	IN	AL,DX			; Get contents of LSB
	OR	BH,AL
	AND	BL,AL
;
	IN	AL,DX			; Get contents of MSB
	OR	BH,AL
	AND	BL,AL
;
	LOOP	TIM1TS			; All bits haven't toggled, keep trying
;
	STACKINIT LBA2			; init stack in ROM
	JMP	DIAGNOSTIC		; ...FAILED, call for help

	endif	; faraday
	       
	PRGSIZ	< Check: Timer 1 >
;
;------------------------------------------------------------------------------
;
; Initialize 8253-5 Programable Interval Timer memory refresh counter
;
TIM1T1:
;
	mov	dx,pitmd
	MOV	AL,PITSL1+PITRLL+PITMD2 ; Memory refresh timer
	OUT	DX,AL			;   set up as a rate generator
;
	DEC	DX
	DEC	DX
	MOV	AL,12H			; count 
	OUT	DX,AL			; Store LSB only, MSB is automatically
					;   cleared to zero
;- CHANGED (TB) ---------------------------------------------------------------
;						      
; Checkpoint 3 :  8253 TIMER 0 (Time-of-day counter)
;		  Same test procedure as timer 1. 
;
; Test # on printer port :  10h
;
; Exit :  OK -> Timer 0 is been setup to trigger time-of-day interrupt .
;	 BAD -> System halted or test will start again at checkpoint 2 !
; 
;------------------------------------------------------------------------------
;
public	t_timer0
T_TIMER0:
	if	faraday
	 jmp	 tim0t1 		; skip this test, because faraday's 
	else				; clock emulation is not compatible
 
	INC	DX
	INC	DX
	MOV	AL,PITSL0+PITRL+PITMD3	; set up time of day interrupt
	OUT	DX,AL			;   mode 3, square wave
					; Read/load LSB 1st, then MSB
;
; Test 8253-5 Programable Interval Timer time of day interrupt counter
;
	SUB	DL,3			; set count to 0000
	MOV	AL,00h			; 
	OUT	DX,AL			; Store LSB 1st
;
; Since we have to do the following anyway, do it here to increase delay
;   between consecutive accesses to a peripheral chips
	XCHG	BH,BL			; Set BX to 000FFh, since to get here
					;   means it is 0FF00h
	XOR	CX,CX			; Set timeout for test
;
	OUT	DX,AL			;   now store MSB
;
TIM0TS:
	MOV	DX,PITMD
	MOV	AL,PITSL0+PITLCH	; Latch counter
	OUT	DX,AL			; 
;
	CMP	BX,0FF00h		; If we reach this value then it's good
	JZ	TIM0T1
	SUB	DL,3			; Delay 1000ns for chip I/O recov. time
					; Now read latched value
	IN	AL,DX			; Get contents of LSB
	OR	BH,AL
	AND	BL,AL
;
	IN	AL,DX			; Get contents of MSB
	OR	BH,AL
	AND	BL,AL
;
	LOOP	TIM0TS			; All bits haven't toggled, keep trying
;
	STACKINIT LBA2			; init stack in ROM
	JMP	DIAGNOSTIC		; ...FAILED, call for help

	endif	; faraday

	PRGSIZ	< Check: Timer 0 >
;
;------------------------------------------------------------------------------
;
; Set time_of_day timer
;
TIM0T1:
;
	mov	dx,pitmd
	MOV	AL,PITSL0+PITRL+PITMD3	; set up time of day interrupt
	OUT	DX,AL			;   mode 3, square wave
					; Read/load LSB 1st, then MSB
	SUB	DL,3			; set count to 0000
	MOV	AL,00h			; 
	OUT	DX,AL			; Store LSB 1st
	nop
	nop				; wait for chip recovery time
	nop
	OUT	DX,AL			; and now store MSB
	JMP	SHORT T_DMA
;
;- CHANGED (TB) ---------------------------------------------------------------
;						      
; Checkpoint 4 :  8237 DMA CONTROLLER
;		  Check DMA chan 0 and page registers with walking "0" and "1".
;
; Test # on printer port :  40h
;
; Exit :  OK -> DMA chan 0 is been setup for RAM refresh operation  
;		DMA chan 1,2,3 are prepared for later transfers
;	 BAD -> System halted or test will start again
; 
;------------------------------------------------------------------------------
;
LBA3	DW	$+2			; Label to return without stack
public	t_dma
T_DMA:
	DISPLAY 40			; Display test code
	TRIGGER 			; provide for trigger pulse
;
; Initialize 8237-A Dma Controller
;
	XOR	AX,AX
	MOV	CX,3			; set the 3 page registers to 0
	MOV	DX,DMAPAG+1		; base address of dma page registers+1
					;   to get past unused address
FILPAG:
	OUT	DX,AL
	INC	DX			; next port
	LOOP	FILPAG			; do them all
;
	MOV	DX,DMACLR		; Do a DMA Master Clear
	OUT	DX,AL			; sending anything will do it
;
; Test DMA channel # 0 locations
;
	MOV	DX,DMA0ADR
	MOV	BX,8000h		; Initial value to use
	MOV	CL,64			; Since CH is already 0 from
					;   above LOOP FILPAG, use this form
DMA0TS1:
	TEST	CL,01h
	JZ	DMA0TS2
	INC	DX
DMA0TS2:
;
	MOV	AL,BH
	OUT	DX,AL			; Output 1st
;
	XCHG	AX,BX			; Delay 600ns for chip I/O recovery time
	OUT	DX,AL			; Output 2nd
	XCHG	AX,BX			; Delay 600ns for chip I/O recovery time
;
	IN	AL,DX
	XCHG	AH,AL			; Delay 600ns for chip I/O recovery time
	IN	AL,DX
	CMP	BX,AX
	JNZ	DMA0TS5

	TEST	CL,01h
	JZ	DMA0TS3
	DEC	DX	
	SAR	BX,1
DMA0TS3:	
	CMP	CL,33
	JNZ	DMA0TS4
	MOV	BH,7Fh
DMA0TS4:
	LOOP	DMA0TS1
	JMP	SHORT DMA0TS6
DMA0TS5:
;
	STACKINIT LBA3			; init stack in ROM
	JMP	DIAGNOSTIC
	PRGSIZ	< Check: DMA >
;
;------------------------------------------------------------------------------
;
DMA0TS6:
	INC	DX			; init dma chan 0 count for memory
					;   refresh
	MOV	AL,0FFh 		; set word count to FFFFH
	OUT	DX,AL			; Output 1st 0FFh
	NOP				; Delay 600ns for chip I/O recovery time
	OUT	DX,AL			; Output 2nd 0FFh
;
	MOV	AL,DMASMD+DMAAUT+DMARD	; set up mode for refresh operation
	MOV	DX,DMAMD
	OUT	DX,AL
;
	MOV	AL,DMAWRT+DMACH1	; init mode of chan 1
	OUT	DX,AL
	INC	AX			; chan 2, use 16 bit increment to
					;   increment AL, because it is faster
					;   and uses less code
	NOP				; Delay 600ns for chip I/O recovery time
	OUT	DX,AL
	INC	AX			; chan 3, use 16 bit increment to
					;   increment AL, because it is faster
					;   and uses less code
	NOP				; Delay 600ns for chip I/O recovery time
	OUT	DX,AL
;
	XOR	AL,AL			; enable dma (AL = 0)
	MOV	DX,DMACMD		; this port
	OUT	DX,AL
;
	MOV	DX,DMAMSK		; unmask channel 0
	MOV	AL,0EH
	OUT	DX,AL	
;
	if	faraday  
;------------------------------------------------------------------------------
;
; Setup control byte for FARADAY Configuration register
;
;------------------------------------------------------------------------------
public	f_setup
f_setup:
	 mov	 al,FA_Setup256 	; Dis_Parity, no_8087, Dis_RAM
	 out	 FA_config,al
	else							      
	 IN	 AL,PPIB		; Clear and enable parity checker
	 OR	 AL,MEMOFF+IOCHKO	; mask parity bits
	 OUT	 PPIB,AL
	 AND	 AL,NOT (MEMOFF+IOCHKO) ; toggle parity bits
	 OUT	 PPIB,AL
	endif
	JMP	SHORT T_RAM
;
;- CHANGED (TB) ---------------------------------------------------------------
;						      
; Checkpoint 5 :  1st k RAM		  
;		  The dynamic RAMs are being refreshed now, and parity checker
;		  is enabled. Let's check the 1st k of memory on this way:
;		   Fill RAM with 00h, look for this value and fill RAM now 
;		   with FFh, check now this new value and fill RAM with
;		   00h again.
;
; Test # on printer port :  61h
;
; Exit :  OK -> 1st k of RAM is cleared 	   
;	 BAD -> System halted or test will start again
; 
;------------------------------------------------------------------------------
;
LBA4	DW	$+2			; Label to return without stack
public	t_ram
T_RAM:
	DISPLAY 61			; display test code
	TRIGGER 			; provide for trigger pulse
;
; Clear the 1st k of memory
;
	CLD
	XOR	AX,AX		; Set value to clear memory to
	MOV	ES,AX		; ES = starting paragraph #
	ASSUME	ES:INTVEC
	XOR	DI,DI		; Start at beginning of segment
	MOV	CX,0200h	; Clear for  512 words (1024 bytes)
	REP	STOSW		; Clear it
;
; Test memory 1st k of memory
;
	DEC	AL		; Set AL = 0FFh
	XOR	DI,DI		; Start with 1st word of starting paragraph
	MOV	CH,04h		; Do 1k words by bytes,
				;   NOTE: CL already equals 0 from REP STOSW
TST1ST:
	CMP	ES:[DI],AH	; Check word for cleared value
	STOSB			; Store the new value in there
	LOOPZ	TST1ST		; Repeat for next byte
	JZ	TST1S0		; If zero, then no error and we're done
	JMP	RAMHLT		;   If not OK, then we're in trouble   
TST1S0:
	XCHG	AH,AL		; Exchange the two compare values
;
	XOR	DI,DI		; Start with 1st word of starting paragraph
	MOV	CH,04h		; Do 1k words by bytes,
				;   NOTE: CL already equals 0 from LOOP TST1ST
TST1S1:
	CMP	ES:[DI],AH	; Check word for cleared value
	STOSB			; Store the new value in there
	LOOPZ	TST1S1		; Repeat for next byte
	JNZ	RAMHLT		; If zero, then no error and we're done
;
TST1S2:
;-----------------------------------------------------------------------------
;
; The 1st k is alright (at least for the data bits),
; so lets set up a stack so we can have interrupts & subroutines
;
	MOV	AX,30h			; Set stack to 30:100
	MOV	SS,AX			; = $00400 = top of 1st k RAM
	MOV	SP,100H
;
	EXTRN	CHKPAR:NEAR
	CALL	CHKPAR
	jnz	ramhlt			; found error, stop!
;

;
	if	faraday
	 jmp	 short	  T_F1		; If zero, then no error and we're done
	else
	 jmp	 short	  T_PIC
	endif
;
RAMHLT: 				; If not OK, then we're in trouble
	STACKINIT LBA4			; init stack in ROM
	JMP	DIAGNOSTIC		; ...FAILED, call for help
	PRGSIZ	< Check: RAM1 >

	if	faraday
;- ADDED 10/24/86 TB ---------------------------------------------------------
;
; Checkpoint F1:  FARDAY's SWITCH and CONTROL register
;		  Checks the config register with test pattern "AA" and "15".
;		  Loads switch register with "A5" and "5A" and reads it back.
;		  Initialize both registers when tests passed. 
;
; Test # on printer port :  F1h
;
; Exit :  OK -> The switch and control register has been initialized
;	 BAD -> System halted or test will start again
;
;-----------------------------------------------------------------------------
;
LBAF1	DW	$+2			; Label to return without stack
public	t_f1
T_F1:
	DISPLAY 0f1			; display test code
	TRIGGER 			; provide for trigger pulse
;
	CALL   SETDS40			; Set DS to the ROM data segment
	ASSUME DS:ROMDAT
	CMP    RSTFLG,1234h		; If this was a CTRL-ALT-DEL
	je     init_regs		; skip this test

	mov    al,0aah
	out    FA_control,al		; test control port
	xor    al,al
	out    FA_switch,al		; toggle the bus
	in     al,FA_control 
	cmp    al,0aah
	jne    F1_halt
;
	mov    al,15h
	out    FA_control,al		; test control port again
	mov    al,0ffh 
	out    FA_switch,al		; toggle the bus
	in     al,FA_control
	cmp    al,015h
	jne    F1_halt
;		     
	mov    bl,0a5h			; test switch register now
test_sw:
	mov    al,bl
	out    FA_switch,al
	push   ax
	call   swthrd			; read switch into bl
	pop    ax
	and    al,0cfh			; mask video bits
	and    bl,0cfh
	cmp    al,bl
	jne    F1_halt
	cmp    al,4ah			; test done
	je     init_regs
	mov    bl,5ah			; second test pattern
	jmp    test_sw
;
init_regs:
	mov    al,1
	out    FA_switch,al		; reset all switches, no diagnostic
	mov    al,0b1h
	out    FA_control,al		; init system control register
	jmp    short  T_PIC 
;
F1_halt:
	STACKINIT LBAF1 		; init stack in ROM
	JMP	DIAGNOSTIC		; ...FAILED, call for help
	PRGSIZ	< Check: SWITCH and CONTROL reg >
	endif
;
;- CHANGED (TB) ---------------------------------------------------------------
;						      
; Checkpoint 6 :  8259 PROGRAMABLE INTERRUPT CONTROLLER
;		  Setup the PIC and then test the mask register for all values
;		  by marching a 1 and a 0 through it.
;		  Leave the mask register with an 0FFh to disable all 
;		  interrupts, so that the keyboard does not abort the
;		  CRT memory tests.
;
; Test # on printer port :  50h
;
; Exit :  OK -> PIC is been setup for our system, but it's disabled
;	 BAD -> System halted or test will start again
; 
;------------------------------------------------------------------------------
;
LBA5	DW	$+2			; Label to return without stack
public	t_pic
T_PIC:
	DISPLAY 50			; display test code
	TRIGGER 			; provide for trigger pulse
;
; Set up the 8259A Programmable Interrupt Controller
;
	mov	dx,PIC0 		; control port
	mov	al,ICW1+ADI+SNGL+IC4	;
					; ICW1 is being issued;
					; Call Address Interval = 4, else 8;
					; No effect in an 8086 or 8088 system;
					; Single 8259 in system; 
					; No ICW3 will be issued; 
					; ICW4 will be issued, else not.
	out	dx, al			; write icw1
	inc	dx
	mov	al,08h
	out	dx, al			; icw2 - load vector table address
	mov	al,BUFMAS+UPM86 	; icw4 - 8086 mode, non auto eoi
	out	dx, al			;      - master buffered mode
;
	IF	INTTEST
	CALL	INTTST
	JZ	INTEST
;
	STACKINIT LBA5			; init stack in ROM
	JMP	DIAGNOSTIC		; ...FAILED, call for help
INTEST:
	ENDIF
	PRGSIZ	< Check: PIC >
;-----------------------------------------------------------------------------
;
; Set up the interrupt vector table so we can handle interrupts like
; divide overflow etc.	The hardware and software interrupts will be 
; controlled for now.
;
	IF	NOT INTENHD		; If not enhanced interrupt handler
	EXTRN	VECSIZ:ABS
	EXTRN	VECTBL:WORD
	EXTRN	VECTB1:WORD
	EXTRN	NMIINT:NEAR
	EXTRN	PRTSCR:NEAR
;
	MOV	AX,CS			; Get our code segment in AX
	MOV	DS,AX			;   and in DS
	ASSUME	DS:CODE
;
	MOV	SI,OFFSET VECTBL	; Interrupt vector table
	MOV	DI,08h*4		; Set DI to address 20h for Interrupt 8
	MOV	CX,VECSIZ		; Length of table
MOVLP:
	MOVSW				; Move the interrupt value to the
					;   vector location
	STOSW				; Now store code segment
	LOOP	MOVLP			; Repeat until done
	DEC	DI			; Point back to RAM font segment
	DEC	DI	
	MOV	WORD PTR ES:[DI],CX	; CX = 0000h
;
	EXTRN	NMIINT:NEAR
	EXTRN	PRTSCR:NEAR
	MOV	SI,OFFSET VECTB1
	XOR	DI,DI			; Starting at 0, we have 8 more
	MOV	CL,8			;   more interrupts to move
					;   NOTE: CH = 0 from LOOP MOVLP above
MOVLP1:
	MOVSW				; Move the interrupt value to the
					;   vector location
	STOSW				; Now store code segment
	LOOP	MOVLP1			; Repeat until done
;
	ELSE	;NOT INTENHD
;
;------------------------------------------------------------------------------
;
;   Vector all 256 interrupts to unexpected interrupt handler
;   All unexpected interrupts will vector to UNEXINT, however, upon entry, each
;   INT will have its own CS:IP mapping to the single physical
;   address UNEXINT so UNEXINT can identify the interrupt type.
;
	EXTRN	UNEXINT:NEAR
	XOR	DI,DI			; ES:DI = 0:0
	MOV	ES,DI
	ASSUME	ES:NOTHING
;
	MOV	BX,CS			; Get our code segment in BX
	MOV	DS,BX			;   and in DS for later
	ASSUME	DS:CODE
;
	MOV	AX,OFFSET UNEXINT	; get UNEXINT offset in AX
	MOV	DX,AX			;   and copy to DX for later
;
	IF	ZIVS
	MOV	CX,20h			; Length of table (changed in Rev.2.13)
	ELSE	;ZIVS
	MOV	CX,100h 		; Length of table
	ENDIF	;ZIVS ELSE
FILLP:
	STOSW				; ISR off = offset UNEXINT -10h*int type
	XCHG	AX,BX
	STOSW				; ISR seg = normal code seg + int type
	INC	AX			; update seg
	XCHG	AX,BX
	SUB	AX,10h			; update offset
	LOOP	FILLP			; Repeat until done
;------------------------------------------------------------------------------
; Set up the vectors for expected interrupts
;
; Set up vectors 8 thru 1F (except those already loaded):
;
	EXTRN	VECTBL:NEAR
	EXTRN	VECSIZ:ABS
	EXTRN	NMIINT:NEAR
	EXTRN	PRTSCR:NEAR
;
	MOV	SI,OFFSET VECTBL	; Interrupt vector table
	MOV	DI,08h*4		; Set DI to address 20h for Interrupt 8
	MOV	CX,VECSIZ		; Length of table
MOVLP:
	LODSW				; Get ISR offset
	CMP	AX,DX			; is it unexpected int handler?
	JNE	DOTHISVEC		; if so,
	ADD	DI,4			;  skip this vector
	JMP	SHORT MOVLP0
DOTHISVEC:				; endif
	STOSW				; store it
	MOV	AX,CS			; Get our code segment in AX
	STOSW				; Now store code segment
MOVLP0:
	LOOP	MOVLP			; Repeat until done
;
	DEC	DI			; Set to RAM font segment
	DEC	DI
	XOR	AX,AX			;   and clear to 0
	STOSW				; Now store code segment
;
; Load NMI vector and screen print int vector:
;
	MOV	WORD PTR ES:[2*4],OFFSET NMIINT
	MOV	WORD PTR ES:[2*4+2],CS
	MOV	WORD PTR ES:[5*4],OFFSET PRTSCR
	MOV	WORD PTR ES:[5*4+2],CS
;
	PRGSIZ	<Initialize Interrupt vectors>
	ENDIF	;NOT INTENHD ELSE
;
;------------------------------------------------------------------------------
;

; Save 1234 ctrl-alt del flag and time and day variables
;
	CALL	SETDS40 		; Set DS to the ROM data segment
	ASSUME	DS:ROMDAT
;
	IF	TIMSAV
	MOV	BL,HOUR24		; 24-HOUR ROLLOVER COUNTER
	PUSH	BX
	PUSH	LOTIME			; LEAST SIGNIFICANT TIMER COUNTER
	PUSH	HITIME			; MOST SIGNIFICANT TIMER COUNTER
	ENDIF
;
	MOV	DI,RSTFLG		; RESET FLAG MEMORY OFFSET
;
	CALL	CHKPAR
	JZ	MEMTST
;
; If parity error on ROM data area ignore them, but assume we are cold booting
;
	XOR	DI,DI			; Clear reset flag
;
public	memtst
MEMTST:
;
; Test 2nd k of memory
;
;	TESTNO				; Test # port PPIA to 0
	MOV	BP,0040h		; Start at 1st k of memory
	MOV	SI,0080h		; Finish at 2nd k of memory
;
	XOR	BL,BL			; Reset flag to not display tested memory
	CALL	CLRTST			; Test the memory
	CALL	CLRMEM			; Just clear the memory
;
; Restore 1234 ctrl-alt del flag and time and day variables
;   it may not be desireable to restore the time of day variables
;
	CMP	DI,1234h		; If this wasn't a CTRL-ALT-DEL
;
	IF	TIMSAV
	JNZ	NORST			;   don't restore variables
	MOV	RSTFLG,DI		; Restore Reset flag
	POP	HITIME			; MOST SIGNIFICANT TIMER COUNTER
	POP	LOTIME			; LEAST SIGNIFICANT TIMER COUNTER
	POP	BX
	MOV	HOUR24,BL		; 24-HOUR ROLLOVER COUNTER
	JMP	SHORT RSTFIN		; Were done
NORST:
	ADD	SP,06h			; Get garbage off stack
	ELSE
	JNZ	RSTFIN			;   don't restore variables
	MOV	RSTFLG,DI		; Restore Reset flag
	ENDIF
;
RSTFIN:
;
; Enable timer interrupt now because we may need to know what time it is
;   when we try to boot up a hard disk
;
	MOV	AL,0BEh 		; OWC1, enable hardware interrupts for
					;   Timer ISR		(Bit 0 = 0)
	OUT	PIC0MSK,AL		;   Disk  ISR		(Bit 6 = 0)
	STI

	if	Faraday
;
;- ADDED 10/24/86 TB ----------------------------------------------------------
;						      
; Checkpoint F2 :  FLOPPY DRIVE configuration	  
;		   - Init floppy interface
;		   - Test how many floppy drives are connected
;		   - Updates switch register	       
;
; Test # on printer port :  F2h
;
; Exit :  OK -> Switch contains # of present drives
;	 BAD -> Print error message.
; 
;------------------------------------------------------------------------------
;
public	t_f2
T_F2:
	EXTRN	DOSEEK:NEAR		; Disk seek routine
;
	MOV	AH,0F2h    
	CALL	C_DISPLAY		; display test code
	CALL	C_TRIGGER		; provide for trigger pulse
;
	CALL	SETDS40 		; Set DS to the ROM data segment
	ASSUME	DS:ROMDAT

	xor	ah,ah
	int	13h			; reset disk system
	XOR	dl,dl			; Drive 0
ChkDrive:
	xor	dh,dh			; Head 0
	XOR	CH,CH			; Track 0
	XOR	AH,AH			; Don't wait for motor to come to speed
	CALL	DOSEEK
	JC	NoDisk
	MOV	CH,34			; Track 34
	CALL	DOSEEK
	JC	NoDisk
	inc	dl			; check next drive
	cmp	dl,3			; all done ?
	jle	ChkDrive
NoDisk:
	cmp	dl,0			; is there any drive ?
	je	NO_Drive
	dec	dl
NO_Drive:
	mov	cl,6
	shl	dl,cl			; shift # of drives to correct position
	or	ds:byte ptr DEVFLG,dl	; setup device register
	mov	al,dl			; set disk switches
	or	al,1			; set diagnostic switch off
	out	FA_switch,al		; set switch register
;
	PRGSIZ	< Check: # of drives present>  ; Display program size
;
;------------------------------------------------------------------------------

	call	lock_faraday 		;VMR14.11.89

	endif				;VMR14.11.89

	if	hydra
	 display 6a 
	 call	 wait_for_amiga 	 ; here we stop until amiga has done
	endif				 ;	   the setups on janus board

;
;- CHANGED (TB) ---------------------------------------------------------------
;						      
; Checkpoint 7 :  VIDEO -CRT, -RAM		 
;		  - Get cuurent video mode from system switch.
;		  - Check the video RAM with normal RAM-test. Beeps three
;		    times if this test failed.
;		  - Init video controller.
;
; Test # on printer port :  80h
;
; Exit :  OK -> Ready for print on screen
;	 BAD -> Speaker beeps, but the video controller will be initialized.
; 
;------------------------------------------------------------------------------
;
public	t_crt
T_CRT:
	DISPLAY 80			; display test code
	TRIGGER 			; provide for trigger pulse
;
; Set DEVFLG based upon switches
;
	CALL	SWTHRD			; Read switches into BL
	XOR	BH,BH			; keep high byte clear for now
;
; Data segment is set to ROM data area from 1st k memory test
;
	MOV	DEVFLG,BX		; store switches
;
; Test CRT memory 
;
	MOV	BP,0B000h		; Start at 0th k of monochrome CRT
	MOV	SI,0B100h		; Finish at 4K of memory
	MOV	AL,BYTE PTR DEVFLG	; Is no video card selected ?
	AND	AL,30h			
	JZ	NOCRT			; A card is not selected
	XOR	AL,30h			; Is it the monochrome card ?
	JZ	MONOCH			;   Yes
	MOV	BP,0B800h		; Start at 0th k of graphic CRT
	MOV	SI,0BC00h		; Finish at 16K of memory
MONOCH:
	PUSH	SI
	XOR	BL,BL			; Reset flag to not display tested memory
	CALL	CLRTST			; Test the memory
	POP	BP
;
;	CALL	CHKPAR
;	JNZ	CRTBAD			; If parity error on CRT memory, 
;
	IF	CRTBP
	CMP	SI,BP			; If SI is less than end of CRT memory
	JNC	GRAFOK
CRTBAD:
	CALL	CRTBEEP
	ELSE
CRTBAD:
	ENDIF
;
	CALL	CLRMEM			; Just clear the memory
;
GRAFOK:
;
NOCRT:
;
; Initialize both CRTs
;
	push	DEVFLG			; save current setting
	mov	BYTE PTR DEVFLG,30H	; set black/white
	XOR	AX,AX			; set mode to ? bw (AH = 0, AL = 0)
	int	10H
	mov	BYTE PTR DEVFLG,20h	; set color
	mov	ax, 3			; init color monitor to 80 x 25
	int	10H
	pop	AX
	MOV	DEVFLG,AX		; restore state
;
; Set CRT state
;
	MOV	AH,07h
	XOR	AL,030H 		; see what state
	JZ	CRTSET			; if monochrome
	MOV	AH,01h
	XOR	AL,020H 		; color 40 x 25 mode ?
	JZ	CRTSET			;   Yes
	MOV	AH,03h			;   No, set it to 80 x 25 color mode
crtset:
	XCHG	AL,AH			; Set mode (AL = Mode)
	XOR	AH,AH
	INT	10H			; set mode according to switches
;
; Test if no video card is selected and if so place a dummy interrupt into
;   vector INT 10h
;
	TEST	BYTE PTR DEVFLG,30H	; Is no video card selected ?
	JNZ	CRTCHK			;   A card is selected
;
	XOR	AX,AX			; Set DS to segment paragraph 0
	MOV	DS,AX
	ASSUME	DS:INTVEC
	MOV	WORD PTR DS:[10h*4],OFFSET DUMINT
	JMP	SHORT CRTCH0
;
;- CHANGED (TB) ---------------------------------------------------------------
;						      
; Checkpoint 8 :  VIDEO -RETRACE	       
;		  - Test vertical and horizontal retraces.	
;
; Test # on printer port :  81h
;
; Exit :  OK -> Ready for print on screen
;	 BAD -> Speaker beeps, but the video controller will be initialized.
; 
;------------------------------------------------------------------------------
;
public	crtchk
CRTCHK:
	DISPLAY 81			; display test code
	TRIGGER 			; provide for trigger pulse

	CALL	CRTTST			; Test CRT blanking
CRTCH0:
;
; Test for alternate video roms C0000 up to but not including C8000
;
	IF	CRTROM
	MOV	SI,0C000H		; Starting segment of optional ROMs
	MOV	CX,0C800h		;   up to but not including paragraph
	MOV	DL,01h			; Set flag to not wait for hard disk to
					;   spin up
	CALL	XTRROM			; Search for extra ROM

	ENDIF
;
	PRGSIZ	<Check video section>
;
;------------------------------------------------------------------------------
;
; Print manufacturer's signon
;
	JMP	OVERMA2
	MANSION2			; optional MANufacturer's SIgnON part 2
OVERMA2:
	MOV	SI,OFFSET MANS2
	CALL	WRTOUT
	JMP	MORETESTS		; continue in 2nd part of BIOS-ROM
;
	PRGSIZ	<End of non screen-displayed part of power-up> 
;
;------------------------------------------------------------------------------
;
	IF	ROMSIZ GT (8*1024)
	SYNC	FDBOOTA 		;INT 19H service (1) 
	RESYNC	FILLSIZ
;	DW	0AA55h
;	DB	(ROMSIZ-(8*1024))/512	; Size in 512 byte blocks
; If this is an optional ROM, initialization code starts here
; A checksum byte should be explicitly reserved at the end of the ROM as
; defined by the size above.  (For this ROM it is reserved following the
; SYNC 0E000h-1 later in this module.)
;! Space for checksum temporarily placed here until ROMCNV is updated stan 5/85;
;	DB 0	;!! remove these two lines shortly
	PRGSIZ	<Optional ROM header data structure>	; Display program size
    
	ENDIF
;
; changed ---------------------------------------------------------------------
;
;
; FRCINT   ( ENTRY )
;
;******************************************************************************
;  LOCATION MUST BE 0F000:0C003H       ; ..ADDRESS FOR IBM COMPATIBILITY.
;
;  which maps from 0F000:4003h in the IBM ROM BASIC
;  where original entries are located.
;
;  So we trap them, since our BIOS ROM is not fully decoded.
;
;------------------------------------------------------------------------------
	SYNC	BASIC_INT		; creates ORG at $C003 in BIOS segment
;	RESYNC	FILLSIZ
FRCINT	PROC	FAR
	CLC				; ..INDICATE THIS ROUTINE
	INT	0C0h			; ..INTO OUR BASICA
	RET
FRCINT	ENDP

;****************************************************************************
;
; MAKINT   ( ENTRY )
;
MAKINT	PROC	FAR
	STC				; INDICATE THIS ROUTINE
	INT	0C0h			; ..OUR BASICA HANDLES THE ROUTINE
	RET
MAKINT	ENDP
	PRGSIZ	<BASIC trap>

;****************************************************************************
;
; KEYDUMMY ( ENTRY )
;
KEYDUMMY PROC	FAR
					; If we have no extended keyboard
	 RET				; routine, we return via this  
					; FAR RETURN
KEYDUMMY ENDP
	 PRGSIZ  <KEYBOARD dummy return>
;
;------------------------------------------------------------------------------
;
; Synchronize PC- and AMIGA- power up  - set "refresh ready" flag on janus
;				       - clear "wake up 8088" flag on janus 
;				       - wait for setting this flag from AMIGA 
; Update: 10/24/86 TB		       - return, if flag set or after timeout
;
;-----------------------------------------------------------------------------
;
janus_seg SEGMENT AT 0f000h	 ; The following are relative to Segment f000h

	       org   0
ref_rdy        label byte		; lock byte of janus_base

	       org   1
wu8088	       label byte		; pad0 of janus_base
 
janus_seg ENDS


		if   hydra

wait_for_amiga	proc near
public wait_for_amiga
assume ds:janus_seg 
 
		push ds
		push ax
		push cx

		mov  ax,janus_seg
		mov  ds,ax		; setup segment register
; done by AMIGA
;		mov  wu8088,0ffh	; clear "wake up 8088"
		mov  ax,50		; timeout value
sleep:		mov  cx,0
 		mov  ref_rdy,0		; set "refresh ready"
sleep_deep:
		or   wu8088,0
		loopnz	sleep_deep
		dec  ax
		jnz  sleep

		pop  cx
		pop  ax
		pop  ds
		ret

wait_for_amiga	endp 

	PRGSIZ	<Synchronize PC and AMIGA start up>    ; Display program size
	endif

	if	Faraday
;
;- ADDED 10/27/86 TB ---------------------------------------------------------
;
; Checkpoint F3 :  RAM and 8087 configuration	   
;		   - figure out where and how many RAM is installed
;		   - figure out wether 8087 is present or not
;		   - set SWITCH and CONFIG register
;
; Test # on printer port :  F3h
;
; Exit :  OK -> Set and lock configuration register 
;	 BAD -> Lock configuration register 
;
;  NOT implemented
;-----------------------------------------------------------------------------
; 
Lock_Faraday  proc near
public Lock_Faraday  

	 MOV	 AH,0F3h    
	 CALL	 C_DISPLAY		; display "power up ready "-code
	 CALL	 C_TRIGGER		; provide for trigger pulse

	 mov	 al,FA_Setup256 	; Dis_Parity, no_8087, Dis_RAM
	 out	 FA_config,al
	 push	 ax    
	 mov	 ax,7000h		; See, if there is memory installed
					; above 256k on our board
	 call	 ram_avail		; RAM present ?
	 je	 ext_RAM 
	 pop	 ax			; clear stack
	 mov	 al,FA_Setup512 	; Dis_Parity, no_8087, En_RAM
	 out	 FA_config,al
	 push	 ax			; save config reg
ext_RAM:
	call   swthrd			; read switches into bl
	mov    ax,7000h 		; check 512k avail
	call   ram_avail
	jne    ram_256k
	mov    ax,9000h 		; check 640k avail
	call   ram_avail
	jne    ram_512k
ram_640k:
	or     bl,0ch			; set switch to 640k
	jmp    short set_ram
ram_512k:
	or     bl,08h
	and    bl,0fbh			; set switch to 512k
	jmp    short set_ram
ram_256k:
	or     bl,04h
	and    bl,0f7h			; set switch to 256k
set_ram:				; switch setting in BL

	xor    ax,ax			; redirect NMI vector
	mov    es,ax
	mov    word ptr es:[2*4],offset no_8087
	mov    al,NMION
	out    NMIMSK,al		; enable NMI

	pop    ax			; recover config reg
	or     al,FA_8087		; enable 8087
	out    FA_config,al 
	mov    bh,al			; save config reg (BIOS 3.4)
	nop
	nop				; waiting for NMI
	nop 
	or     bl,FA_8087		; set switch
	jmp    short present_8087
		 
no_8087:				; this routine will be executed,  
	add    sp,6			; if 8087 is not present
	and    al,not(FA_8087)
	out    FA_config,al		; update config reg
	mov    bh,al			; save config reg
	and    bl,not(FA_8087)		; update switch reg

present_8087:
	mov    al,bl
	out    FA_switch,al

	mov    al,NMIOFF
	out    NMIMSK,al		; disable NMI
	mov    word ptr es:[2*4],offset NMIINT ; restore old pointer
;
;- ADDED 10/27/86 TB ---------------------------------------------------------
;
;  Lock FARADAY configuration register by setting Bit 3.
;
;-----------------------------------------------------------------------------
; 
	mov	al,bh			; recover config reg
;v3.6	or	al,FA_lock
	out	FA_config,al		; lock register until RESET
	ret			      
;
;-----------------------------------------------------------------------------
ram_avail:
	 MOV	 ES,AX			; ES = segment #	     
	 mov	 di,0fff0h		; offset			 
	 mov	 ax,55aah		; test pattern			    
	 stosw				; fill it
	 sub	 di,2
	 scasw				; and read it back
	 ret

Lock_Faraday  endp 

	PRGSIZ	<Lock Faraday Chip>    ; Display program size
	endif 
;
;------------------------------------------------------------------------------
; Test memory
;
;	Input:	BP = Paragraph starting point
;		SI = Paragraph stopping point
;		AH = 1st compare value
;		AL = 2nd compare value
;		Direction Flag = 0
;		BL = Flag to print message
;		DH = 0 if waiting for keyboard abort, else just test size
;	Output: AX = 0
;		SI = Last segment paragraph+1 that was good in test
;		DH = Flag to abort test
;		DL = Garbaged by KBDDMY interrupt routine
;
; Update:
; V2.02 :  without memory size test (Torsten Burgdorff)
;
;------------------------------------------------------------------------------
	PUBLIC	CLRTST
CLRTST	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	XOR	DH,DH			; Clear keyboard abort flag
CLRTSTA:
	PUSH	CX
	PUSH	DS
	XOR	CX,CX
	MOV	DS,CX
	ASSUME	DS:INTVEC
	MOV	CX,OFFSET KBDDMY	; Get dummy keyboard service routine
	XCHG	CX,DS:[24h]
	PUSH	CX
;
	MOV	CX,OFFSET NMIMEM
	XCHG	CX,DS:[08h]
	PUSH	CX
;
	CALL	CLRMEM			; Clear memory
	DEC	AL			; Set compare values of AH and AL

	CALL	TSTMEM
;
;	CALL	TSTSIZ			; (not in rev. 2.02)
NOABRT:
;
	POP	DS:[08h]
;
	POP	DS:[24h]	; Restore keyboard interrupt vector
	POP	DS
	ASSUME	DS:NOTHING
	POP	CX
ABTST2:
	RET
;
;- CHANGED (TB) ---------------------------------------------------------------
;
; Test memory
;
;	Input:	BP = Paragraph starting point
;		SI = Paragraph stopping point
;		AH = 1st compare value
;		AL = 2nd compare value
;		Direction Flag = 0
;		BL = Flag to print message
;		DH = 0 if waiting for keyboard abort, else just test size
;	Output: AX = 0
;		SI = Last segment paragraph+1 that was good in test
;		DH = Flag to abort test
;		DL = Garbaged by KBDDMY interrupt routine
;
; update: 11/11/85 by Torsten Burgdorff/Commodore
;		   - message only on page boundarys
;		   - no keyboard abort
; 
;------------------------------------------------------------------------------
public	tstmem
TSTMEM:
	ASSUME	DS:NOTHING,ES:NOTHING
	CALL	TSTPAS			; Do pass 1, fall thru for pass 2
TSTPAS:
	
	PUSH	CX
	PUSH	DI
	PUSH	ES
	PUSH	DS
	PUSH	BP
;
TSTPA0:
	MOV	ES,BP			; Get start segment paragraph address
	MOV	DS,BP			; Get start segment paragraph address
	ASSUME	DS:NOTHING,ES:NOTHING
	XOR	DI,DI		; Start with 1st word of starting paragraph
	MOV	CX,400h 	; Do 1k words by bytes
TSTPA1:
	CMP	[DI],AH 	; Check word for cleared value
	STOSB			; Store the new value in there
	LOOPZ	TSTPA1		; Repeat for next word
	JNZ	TSTPA2		;   If not OK, then either
				;     1. we wrote it wrong, but parity is OK
				;  or 2. we have an even # of errors
				;  or 3. parity checker is not reporting fault
				;	 which means a double fault
				;     4. we don't have memory there
	ADD	BP,0040h	; Do next k
;
	PUSH	AX  
	MOV	AX,BP
	AND	AX,0FFFh		; 64k page boundary ?
	JNZ	TSTPA5			; NO: no message
;
	CALL	KYINOF			; Turn off keyboard interrupts while
					;   printing to CRT
	IF	MSGLNG			; Use long messages if true
	CALL	WRTINL
	DB	CR,'RAM:   ',0
	ENDIF
;		  
	MOV	AX,BP			; Print size being tested in k
	MOV	CL,6			; Convert to # of k
	SHR	AX,CL
	CALL	DECMLW			; Print it
;
	CALL	WRTINL
	DB	' KBytes  OK ',CR,0	     ; Good memory size
;
	CALL	KYINON			; Turn keyboard back on to allow
					;   test abort
TSTPA5:
	POP	AX		
;	OR	DH,DH		; If keyb. flag is set then abort memory test
;	JNZ	TSTPA3
	CMP	BP,SI		; Are we past the last location+1 to do ?
	JC	TSTPA0		;   No, then do next 1k


TSTPA6:
	MOV	SI,BP		; Set last (paragraph address+1) location to
				;   last good 1k+1 segment paragraph
TSTPA3:
	XCHG	AH,AL		; Exchange the two compare values
;
	POP	BP
	POP	DS
	POP	ES
	POP	DI
	POP	CX
	RET
;
TSTPA2:
	OR	BL,BL			; Print message flag ?
	JZ	TSTPA6			; No
	PUSH	AX
	DEC	DI
	JNZ	RAMEXS			; If DI <> 0 then it's definitely bad
					;    RAM

					; Get Segment and check if 16k boundary
	TEST	BP,03FFh		; Test if any bit is set		
;
	if	false
	JNZ	RAMEXS
; This code doesn't work unless a non-existent memory definitely returns an
;   0FFh.  The trouble is that the data bus returns a floating value if not
;   pulled up to logic 1 during a non-existent read.  The capacitance of the
;   data bus will read a value that may be similar to the last valid transfer
;   on the bus, which could include 0's.  An attempt is made below to get rid
;   of this charge but doesn't always work
	MOV	AL,0FFh 		; Else test it.  Put 0FFh on bus
	XCHG	AL,[DI] 		;   and try not to let CPU do BUS
	CMP	AL,0FFh
	JZ	NONEXS			; It's non-existent memory
	else
	jz	short nonexs	; go here til we can make above code work
	endif
RAMEXS:
	CALL	KYINOF			; Turn off keyboard interrupts while
					;   printing to CRT
	POP	AX
	PUSH	AX
	MOV	[DI],AH 		; Put value back before the STOSB
;
	PUSH	SI
	CALL	BADRAM			; Print bad RAM information
	MOV	[DI],AL 		; Put original value back
	POP	SI
	INC	DI
NONEXS:
	CALL	KYINON			; Turn keyboard back on to allow
					;   test abort
	POP	AX
	JMP	SHORT TSTPA6
;
; NMI Parity Interrupt Handler for memory tests
;
NONEX1:
	POP	AX
;
	POP	CX		; Get rid of return address from stack
	POP	CX
	POPF			; Get back the flags
;
	PUSH	AX
	PUSH	DX
	CALL	NMIACK			; Acknowledge and reset NMI interrupt
	POP	DX
	CALL	NMIENB		; allow non maskable interrupts
	JMP	SHORT NONEXS
NMIMEM: 	; Interrupt 02h  Non Maskable Interrupt
	PUSH	SI
	PUSH	AX
	CALL	KYINOF			; Turn off keyboard interrupts while
					;   printing to CRT
	CALL	NMIBAD			; Write initial NMI message
	POP	AX
	CALL	BADRA0			; Print bad RAM information
;
	POP	SI
	PUSH	AX
;
; Get the memory address k that failed
	CMP	BP,2000h	       ; Is it >= 128k ?
	JNC	NONEX1		       ;   Yes, then use this memory
;
	CALL	WRTINL
;
	IF	MSGLNG			; Use long messages if true
	DB	CR,LF,'< 128k not OK, parity disabled',CR,LF,0
	ELSE
	DB	CR,LF,'<128k not OK, par dsb',CR,LF,0
	ENDIF
;
	CALL	KYINON			; Turn keyboard back on to allow
					;   test abort
	CALL	CHKPAR			;   Else disable NMI parity
					; Test this area again but this time 
					;  with no parity
	CALL	NMIENB			; allow non maskable interrupts
	POP	AX
	IRET
;
CLRTST	ENDP
	PRGSIZ	<Clear and test memory with NMI>	; Display program size
;------------------------------------------------------------------------------
; Print Bad RAM location, value and expected value subroutine
;
;	INPUT:	None
;	OUTPUT: AX = ?
;		SI = ?
;		CX = ?
;------------------------------------------------------------------------------
BADRAM	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	CALL	WRTCRLF
BADRA0:
	PUSH	AX
;
	IF	MSGLNG			; Use long messages if true
	CALL	WRTINL			; Write error message to CRT
	DB	'Bad RAM at ',0 	; error 201
	MOV	AX,DI			; Write failed address
	MOV	CL,4
	SHR	AX,CL
	MOV	CX,ES
	OR	AX,CX
	CALL	HEXW			; Write word in hex to CRT
	MOV	AX,DI
	CALL	HEXN			; Write nibble in hex to CRT
	CALL	WRTINL			; Write error message to CRT
	DB	' = ',0 			; error 201
	MOV	AL,[DI] 		; Get bad word
	CALL	HEXB			; Write byte in hex to CRT
	CALL	WRTINL			; Write error message to CRT
	DB	'h, expected = ',0				; error 201
	POP	AX
	XCHG	AH,AL
	CALL	HEXB			; Write byte in hex to CRT
	XCHG	AH,AL
	JMP	NEAR PTR PRTHE1 	;    and return from subroutine
;
	ELSE
;
	MOV	AL,[DI] 		; Get bad byte
	CALL	HEXB			; Write byte in hex to CRT
	CALL	WRTINL			; Write error message to CRT
	DB	' Ram Bad ',0				; error 201
	MOV	AX,DI			; Write failed address
	MOV	CL,4
	SHR	AX,CL
	MOV	CX,ES
	ADD	AX,CX
	CALL	HEXW			; Write word in hex to CRT
	MOV	AX,DI
	CALL	HEXN			; Write nibble in hex to CRT
	CALL	SETERF			; Set error flag
	POP	AX
	JMP	NEAR PTR WRTCRLF	;    and return from subroutine
;
	ENDIF
BADRAM	ENDP
	PRGSIZ	<Print bad RAM information>	; Display program size
  
;- ADDED (TB) -----------------------------------------------------------------
;
; Print NMI location :	- Disable NMI and clear parity latches.
;			- Looks to parity latches, while reading
;			  the RAM. 
;			- Print location, when one of the parity 
;			  latches toggles.   
;
;------------------------------------------------------------------------------
	PUBLIC	NMIPLACE
;
NMIPLACE PROC	NEAR

	PUSH   AX
	PUSH   SI
	PUSH   DS
 ;
	MOV    AL,NMIOFF
	OUT    NMIMSK,AL		   ; DISABLE NMI's
	IN     AL,PPIB			   ; CLEAR PARITY LATCHES
	OR     AL,MEMOFF+IOCHKO
	OUT    PPIB,AL
	AND    AL, NOT (MEMOFF+IOCHKO)
	OUT    PPIB,AL
;
	SUB    AX,AX
	MOV    SI,AX
PLOOP1:
	MOV    DS,AX			   ; CHECK 64K
PLOOP:
	LODSB				   ; MEMORY READ
	IN     AL,PPIC			   ; PARITY TOGGLED ?
	AND    AL,PARCHK+IOCHK
	JNE    P_FOUND			   ; YES, IF NE
	LOOP   PLOOP			   ; ELSE CONTINUE
	MOV    AX,DS			   ; SI,CX SHOULD BE 0 HERE
	ADD    AX,1000H 		   ; NEXT 64K BLOCK
	CMP    AX,0A000H		   ; 640K DONE ?
	JNE    PLOOP1			   ; IF NE, CONTINUE
	JMP    SHORT PRET		   ; ELSE DONE	 
;
P_FOUND:
	CALL  WRTINL
	DB    CR,LF,' Found parity error at ',0
	MOV   AX,DS			   ; SEGMENT
	CALL  HEXW			   ; 1ST 4 OF 8 DIGITS
	CALL  WRTINL
	DB    ':',0
	MOV   AX,SI			   ; OFFSET		   
	CALL  HEXW 
;
PRET:
	CALL  WRTINL
	DB    CR,LF,' Programm  stopped  at ',0
;
	POP   DS
	POP   SI
	POP   AX
	RET
;
NMIPLACE ENDP
	PRGSIZ	<Print NMI place>      ; Display program size
; 
;------------------------------------------------------------------------------
; Read switches from PPI port without modifying the PPI A or B ports
;
;	Input:	None
;	Output: BL = Switch values
;		AL = AL
;		AH = ?
;		CL = ?
;		DX = PPIA
;------------------------------------------------------------------------------
public	swthrd
SWTHRD	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	MOV	AH,PPIMSF+PPI1M0+PPICUI+PPI2M0+PPICLI
					; Default to PPIA out after
SWTHRDA:
	PUSH	AX
;
	IF	XTSWTH
	 MOV	 DX,PPIB		 ; read XT switches	
	 IN	 AL,DX			 ; @@@
	 MOV	 AH,AL			 ; Save for restoring later
	if	faraday
	 and	 al,NOT(FA_swhigh)
	else
	 OR	 AL,CASOFF		    
	endif
	 OUT	 DX,AL			 ; choice upper switches
	 INC	 DX
	 MOV	 CL,4			 ; Must shift BL into position
	 IN	 AL,DX			 ; read switches 5-8
	 MOV	 BL,AL			 ; Save in BL
	 SHL	 BL,CL
	 DEC	 DX			 ; Set DX to PPIB
	 IN	 AL,DX			 ; @@@
	if	faraday
	 xor	 al,FA_swhigh
	else
	 XOR	 AL,CASOFF		    
	endif
	 OUT	 DX,AL			 ; Read XT's switches 1-4
	 INC	 DX			 ; Set DX to PPIC and delay 850ns
	 NOP				 ; for chip I/O recovery time
	 IN	 AL,DX			 ; read XT switches
	 AND	 AL,0FH 		 ; keep low nibble
	 OR	 BL,AL			 ; combine
;
; don't restore normally will keep cassette motor bit set to 1 which means off
;
	 DEC	 DX
	 MOV	 AL,AH			 ; Restore value to PPIB
	 OUT	 DX,AL
	ENDIF		;XTSWTH
;
; Set up the 8255 Programable Peripheral Interface (PPI) for reading the
;   keyboard or sense switches
;
	IF	PCSWTH
	 MOV	 AH,PPIMSF+PPI1M0+PPIAIN+PPICUI+PPI2M0+PPICLI
	 CALL	 PPISET
	 INC	 DX
	 IN	 AL,DX			 ;@@@
	 PUSH	 AX
	 OR	 AL,SNSWEN
	 OUT	 DX,AL
	 DEC	 DX
	 NOP				 ; Delay 850ns 
	 IN	 AL,DX			 ; read switch from PPIA, DX = PPIA
	 MOV	 BL,AL			 ; Save switch byte in BL
	 POP	 AX
	 INC	 DX
	 OUT	 DX,AL
	ENDIF		;PCSWTH
;
	POP	AX			; Fall through to set PPI
;------------------------------------------------------------------------------
; Set PPI control port without modifying the PPI A or B ports
;
;	Input:	AH = value to set PPI control port
;	Output: 
;		AL = AL
;		DX = PPIA
;------------------------------------------------------------------------------
PPISET:
	if	faraday
	 nop				; does not exists
	else

	ASSUME	DS:NOTHING,ES:NOTHING
	PUSH	AX
	MOV	DX,PPIA
	IN	AL,DX			; Get contents of PPIA
	PUSH	AX			;   and save on stack
	INC	DX			; Next port
	IN	AL,DX			; @@@ Get contents of PPIB
	PUSH	AX			;   and save on stack
	INC	DX			; Next port
	INC	DX			; Next port
	MOV	AL,AH			; Get value to set PPICTL
	OUT	DX,AL			;   and set it
	POP	AX			; Now restore ports
	DEC	DX			;   starting with PPIB
	DEC	DX			; 
	OUT	DX,AL
	POP	AX			; 
	DEC	DX			; Restore port PPIA
	OUT	DX,AL
	POP	AX

	endif	;faraday

	RET
SWTHRD	ENDP
	PRGSIZ	<Read dip switch and set control port> ; Display program size
;------------------------------------------------------------------------------
; Test of Interrupt mask set/reset,
;   can also be used for expansion box data bus test
;
;	Input:	DX = Port to test
;	Output: Zero flag reset if error, else it is set
;		AL = ?
;		AH = ?
;		CX = 0
;------------------------------------------------------------------------------
public	inttst
INTTST	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	MOV	CX,16
	MOV	BH,7Fh
INTTS1:
	MOV	AL,BH
	OUT	DX,AL
;;**** some delay may be needed between I/O instructions here
	IN	AL,DX
	CMP	BH,AL
	JNZ	INTTS2
;
	SAR	BH,1
;
	CMP	CL,9
	JNZ	INTTS3
	MOV	BH,80h
INTTS3:
	LOOP	INTTS1
	XOR	AL,AL			; Set zero flag if OK
INTTS2:
	RET
;
	PUBLIC	TIMBAD
TIMBAD:
	IF	MSGLNG			; Use long messages if true
	DB	'Timer or Interrupt Controller Bad',0	; error 101
	ELSE
	DB	'Timer or Int Cntrl Bad',0		; error 101
	ENDIF
;
INTTST	ENDP
	PRGSIZ	<Interrupt mask set/reset test> ; Display program size
;------------------------------------------------------------------------------
; Test of DMA address and word count registers
;
;	Input:	DX = Port to start test
;		CX = # of ports to test
;	Output: flags ?
;		AL = ?
;		AH = ?
;		CX = 0
;------------------------------------------------------------------------------
public	dmatst
DMATST	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	MOV	DX,DMA1ADR
	MOV	CX,6			; Do 6 DMA port addresses
DMATS0:
	MOV	BX,8000h		; Initial value to use
	PUSH	CX
	MOV	CX,32
DMATS1:
	MOV	AL,BH
	OUT	DX,AL			; Output 1st
	XCHG	AX,BX			; Delay 600ns for chip I/O recovery time
	OUT	DX,AL			; Output 2nd
	XCHG	AX,BX			; Delay 600ns for chip I/O recovery time
;
	IN	AL,DX
	XCHG	AH,AL			; Delay 600ns for chip I/O recovery time
	IN	AL,DX
	CMP	BX,AX
	JNZ	DMATS5
;
	SAR	BX,1
;
	CMP	CL,17
	JNZ	DMATS3
	MOV	BH,7Fh
DMATS3:
	LOOP	DMATS1
DMATS4:
	INC	DX
	POP	CX
	LOOP	DMATS0
	RET
;
; If error, we have a problem, but it may not be in an area that kills us
;
DMATS5:
	CALL	WRTINL
	DB	'Bad DMA port = ',0
	MOV	AX,DX			; Get DMA port that failed and
	CALL	HEXW			;   print it
	CALL	PRTHE1
	JMP	SHORT DMATS4
;
DMATST	ENDP
	PRGSIZ	<DMA address and word count registers test>
					; Display program size
;- CHANGED (TB) ---------------------------------------------------------------
;
; Routine to output 1 long and 2 short beeps on CRT Error
;
;------------------------------------------------------------------------------
	IF	CRTBP
	PUBLIC	CRTBEEP
CRTBEEP PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
;
	MOV	CX,880			; one long beep
	MOV	DX,1000
	CALL	SPEAKR
;
	MOV	CX,2			; two short beeps
CRTBC:
	PUSH	CX
	SUB	CX,CX
CRTBLP:
	LOOP	CRTBLP 
	MOV	CX,880
	MOV	DX,100
	CALL	SPEAKR
	POP	CX
	LOOP	CRTBC	
;
	RET
;
CRTBEEP ENDP
	ENDIF
	PRGSIZ	<Beep on CRT error>	; Display program size
;------------------------------------------------------------------------------
; Test subroutines for horizontal and vertical retraces
;
;	Input:	
;	Output: 
;------------------------------------------------------------------------------
	IF	CRTTEST
public	crttst
CRTTST	PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	MOV	DX,CRTADDR		; Get address of CRT status register
	ADD	DX,06h			;
	IN	AL,DX			; READ 6845 STATUS REGISTER
	AND	AL,BLANK		; check blank bit
	MOV	AH,AL
	XOR	CX,CX
CRTTLOOP:				; 17 '286 clocks/loop
	IN	AL,DX			; READ 6845 STATUS REGISTER AGAIN
	AND	AL,BLANK		; check blank bit
	XOR	AL,AH			; has it changed? CLEAR C!
	LOOPZ	CRTTLOOP		; keep going if so
	JZ	CRTTERR
	RET				;   Yes, exit
;
CRTTERR:
	IF	CRTBP
;	CALL	CRTBEEP 		; no second beep here for compatibility
	ELSE				; with old bios version 
	TSTHLT				; Timeout failure
	ENDIF
	RET
CRTTST	ENDP
	ENDIF	 ;CRTTEST
	PRGSIZ	<Test subr for horizontal and vertical retrace>
					; Display program size
;------------------------------------------------------------------------------
; Test of Expansion box address bus
;
;	Input:	BX = starting value
;	Output: Zero flag reset if error, else it is set
;		AL = ?
;		AH = ?
;		CX = 0
;------------------------------------------------------------------------------
	IF	EXPTEST
EXPTST	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
	MOV	DX,0F000h
	MOV	DS,DX
	ASSUME	DS:NOTHING
	MOV	DX,EXPRDD	; Read latched Expansion box bus data
	MOV	CX,16
EXPTS1:
	MOV	DS:[BX],BX
;
	IN	AL,DX
;;**** some delay may be needed between I/O instructions here
	XCHG	AH,AL			; Delay for chip I/O recovery time
	INC	DX
	IN	AL,DX
	CMP	BX,AX
	JNZ	EXPTS2
	DEC	DX
	SAR	BX,1
	LOOP	EXPTS1
	XOR	AL,AL			; Set zero flag if OK
EXPTS2:
	RET
;
	PUBLIC	EXPBAD
EXPBAD:
	IF	MSGLNG			; Use long messages if true
	DB	'Expansion Box Bad',0	; error 1801 expansion
;					;   box
	ELSE
	DB	'Exp Bad',0		; error 1801 expansion
;					;   box
	ENDIF
;
EXPTST	ENDP
	ENDIF
	PRGSIZ	<Expansion box address bus test> ; Display program size
;
;------------------------------------------------------------------------------
; Miscellaneous subroutines
;------------------------------------------------------------------------------
	EXTRN	PRTCHR:NEAR
	OUT1	<.		Including MISCL2.INC>
INCLUDE KYBEQU.INC		; Keyboard Equate file
INCLUDE MISCL2.INC
;
;------------------------------------------------------------------------------
; This part of the CRT DSR is included in this module because it is code that
; not everyone requires and may be equated out.  Also it uses about 78h bytes
; and will fit in this module if enabled
;------------------------------------------------------------------------------
	IF	LIGHTPN
	OUT1	<.		Including LGHTPN.INC>
INCLUDE LGHTPN.INC
	ENDIF
;------------------------------------------------------------------------------
; Diagnostic subroutines
;------------------------------------------------------------------------------
;
; Subroutine to loop back the last failed test. Dependent on diagnostic switch
; (Bit0 of system switch) system will halt or loop back after a failed test.
;
; Diagnostic switch ON	: Return to current loop back address
; Diagnostic switch OFF : Fall into TSTHLT macro
; !! WARNING :	This switch will be allways OFF with FARADAY's chipset !!
;
; Input :  Offset begin of last testroutine on stack
; Output:  AL = 0
;------------------------------------------------------------------------------
;
	ASSUME CS:CODE,DS:NOTHING,ES:NOTHING
;
	PUBLIC	DIAGNOSTIC
;
DIAGNOSTIC PROC NEAR
;					
	IF	XTPOST AND (LPALWS OR TSTLOOP)
;
	IN	AL,PPIB 	; @@@ program system port to read lower half
	AND	AL,NOT CASOFF	; of system switch
	OUT	PPIB,AL
	IN	AL,PPIC
	AND	AL,POST 	; check diagnostic switch (Bit0)
	JZ	DOLOOP
	JMP	SHORT $ 	; found switch off -> HALT
DOLOOP:
	     IF      TSTLOOP
	     RET		     ; loop back to start of last test
	     ELSE		     ; = LPALWS is true
	     JMP     SYSRESET	     ; loop back to warm start
	     ENDIF
;
	ENDIF
;
;- CHANGED (TB) ---------------------------------------------------------------
;
; Diagnostic loop routine !!OLD!!
;
TSTLPF:
TSTLP:
; There are 2 choices which one could make to stop code execution,
;   each with their own advantages and disadvantages.
;
;	HLT			; Uses the least amount of code and places
;				;   machine in a static state.
	JMP	SHORT $ 	; Loop to self allows certain debuggers to
				;   maintain control on failures
;
DIAGNOSTIC ENDP
;
;-----------------------------------------------------------------------------
;
C_DISPLAY PROC NEAR		; Calls display makro and returns
;
	  DISPLAY
	  RET
;
C_DISPLAY ENDP
;
;
C_TRIGGER PROC NEAR		; Calls trigger makro and returns
;
	  TRIGGER
	  RET
;
C_TRIGGER ENDP
	PRGSIZ	<Diagnostic branch routine>   ; Display program size
;
;PNXMAC3 MACRO			 ; Old macro		
;- CHANGED (TB) ---------------------------------------------------------------
;
; Printer Check routine
;
;	Input:	DS = ROM Data segment (0040h)
;		DX = Printer base I/O port address
;		BX = Printer port table pointer
;	Output: AX = ?
;
;	Added:	11/11/85 by Torsten Burgdorff /BSW 
;			 display printer base address
;
;------------------------------------------------------------------------------
	EXTRN	PRTCHR:NEAR
	PUBLIC	PRTCHK
;
PRTCHK	PROC	NEAR
	ASSUME	DS:ROMDAT,ES:NOTHING
	MOV	AL, 0Ch 	;test data (& init value for other port - 1)
	OUT	DX, AL
	INC	DX
	INC	DX			;DX points to base + 2
	INC	AX			;flush I/O bus; init printer if there
	OUT	DX, AL
	DEC	DX
	DEC	DX			;DX points to base
	IN	AL, DX			; get data
	CMP	AL, 0Ch 		; is printer there
	JNZ	PRTCH0			; no, finish up
	MOV	[BX], DX		; save address
	INC	BX			; set for next printer address
	INC	BX
	ADD	BYTE PTR [DEVFLG+1],40H ; count printers in devflag
	PUSH	BX
;
	CALL	WRTINL			; Print printer base address
	DB	CR,'LPT',0
;
	MOV	AL,BYTE PTR [DEVFLG+1]	; Print # of Printer
	MOV	CL,6
	SHR	AL,CL
	ADD	AL,30h			; convert to ASCII (simple)
	CALL	PRTCHR			
;
	CALL	WRTINL
	DB	'   at ',0
;
	MOV	AX,DX			; Print base address	   
	CALL	HEXW   
	MOV	SI,OFFSET HEXMSG
	CALL	WRTOUT
;
	POP	BX
PRTCH0:
	RET
PRTCHK	ENDP
	PRGSIZ	<Printer port check code>	; Display program size
;
;- CHANGED (TB) ---------------------------------------------------------------
;
; Check if EIA port is there
;
;	INPUT:	DX = I/O address of LCR
;		SI = Pointer to EIA port table
;		DS = ROM Data area
;	OUTPUT: DX = I/O address of LCR
;		AL = ?
;		DI = I/O address of data register
;		BX = 00AAh
;		DEVFLG+1 = ?
;
;	ADDED : by Torsten Burgdorff/COMMODORE -
;		 - display base address of serial port
;	H2.05 :  - reset modem control port
;
;------------------------------------------------------------------------------
	PUBLIC	EIACHK
	EXTRN	SETBD:NEAR		; Set baudrate
;
EIACHK	PROC	NEAR			; Check if EIA port is there
	ASSUME	DS:ROMDAT,ES:NOTHING
	MOV	BX,1			; Set baud rate divisor to 1
	CALL	SETBD			; Set it




	XOR	AL,AL			; Disable access to baud rate
					;   generator's divisor latches
	inc	dx			; DX points at modem control reg. 
	out	dx,al			; reset all modem control lines
	dec	dx			; DX points at line control reg.
	OUT	DX,AL
;
	IF	I80286
	JMP	$+2			; Delay for I/O
	ENDIF
;
	XCHG	DX,DI
;	OUT	DX,AL			; transmit any character
					; (not in BIOS 2.02)
;
	XCHG	DX,DI			; Set DX to LCR register
	MOV	BL,0AAh 		; Test pattern to see if EIA port is
					;   there
	CALL	SETBD
	XCHG	DI,DX			; Set DX to 
					;   write/read divisor latch
	IN	AL,DX			; read back
	CMP	AL,BL			; is it there
	JNZ	EIACH0			; no
	MOV	[SI],DX 		; store address
	ADD	BYTE PTR [DEVFLG+1],02h ; Count serial ports in high order byte
					;    of DEVFLG
	INC	SI			; next addr storage word
	INC	SI
	PUSH	SI
;
	CALL	WRTINL
	DB	CR,'COM',0
;
	MOV	AL,BYTE PTR [DEVFLG+1]	; Print # of ACIA
	SHR	AL,1
	AND	AL,3
	ADD	AL,30h			; convert to ASCII (simple)
	CALL	PRTCHR			
;
	CALL	WRTINL
	DB	'   at ',0
;
	MOV	AX,DX			; Print ACIA base address	
	CALL	HEXW   
	MOV	SI,OFFSET HEXMSG
	CALL	WRTOUT
;
	POP	SI    
EIACH0:
	RET
EIACHK	ENDP
	PRGSIZ <EIA serial port check code>
;
;----------------------------------------------------------------------------
;
; Second part of powerup routine continues here with screen displayed tests
;
;-----------------------------------------------------------------------------
;
MORETESTS:				; continue tests here
;
;
;OTHER PLACE------------------------------------------------------------------
;						      
; Checkpoint 9 :  PRINTER			 
;		  - Find out which printer is installed and print base address.
;		  - Check R/W register with test pattern. 
;		  - Print message, if no printer found.
;
; Test # on printer port :  E1h
;
; Exit :  OK -> Print base address on screen
;	 BAD -> Print error message.
;
; Update:
; H1.1	: no error if no printer found
; H2.05 : error if no printer found
;
;------------------------------------------------------------------------------
ASSUME DS:ROMDAT
public t_prn
T_PRN:
;	
	MOV	AX,40H
	MOV	DS,AX
;
	MOV	AH,0E1h    
	CALL	C_DISPLAY		; display test code
	CALL	C_TRIGGER		; provide for trigger pulse
;
	MOV	BX,OFFSET LPADRTBL	; where to store existing lp addresses
	MOV	DX,PRTPT1		; Printer port on board
					; 1st printer adapter I/O port address
	CALL	PRTCHK
	MOV	DX,PRTPT2		; 2nd printer adapter I/O port address
	CALL	PRTCHK
	MOV	DX,PRTPT3		; 3rd printer adapter I/O port address
	CALL	PRTCHK
;	 jmp	 T_ACIA 		 ; NO printer available on sidecard
;
	TEST	BYTE PTR [DEVFLG+1],0C0h  ; No printer found => ERROR
	JNE	T_ACIA
	CALL	WRTINL			; Print error message
	DB	CR,'No printer found ',0
;
	CALL	SWTHRD			; read dip switch
	TEST	BL,1			; test diagnostic switch
	JNE	T_ACIACR		; jump, if not set
;
	MOV	AX,OFFSET T_PRN 	; loop back address
	PUSH	AX			; Return address on stack
;
	JMP	DIAGNOSTIC		; Call diagnostic,
					; loop back address is on stack
	PRGSIZ	< Check: Printer>	; Display program size
;
;OTHER PLACE------------------------------------------------------------------
;						      
; Checkpoint 10 :  RS232			  
;		   - Find out how many RS232 interfaces are installed 
;		     and print the base addresses.
;		   - Print message, if no RS232 found.
;
; Test # on printer port :  E2h
;
; Exit :  OK -> Print base address on screen
;	 BAD -> Print error message.
;
; Update:
; H1.1	: no error if no acia found
; 
;------------------------------------------------------------------------------
;
public	t_acia
T_ACIACR:
	CALL	PRTLI0			; Set error flag and write CR,LF
T_ACIA: 
;
	MOV	AH,0E2h    
	CALL	C_DISPLAY		; display test code
	CALL	C_TRIGGER		; provide for trigger pulse
;
; Find out how many RS232 cards are installed.	This is early because of bug
; in some 8250 chips.  The EIACHK routine has code to get around bug (WERE?)
; but should be done before CRT is initialized. 			    
;
; Sidecar: Skip the first serial device base address
;	   This address table is occupied by janus interface, but not
;	   supported by janus.library
;

	mov	SI,OFFSET EIADRTBL	; where to store existing eia addrs

	mov	dx, EIAPT1+3		; line control register
	CALL	EIACHK			; Check if EIA port is there

	if	hydra
	 jmp	 T_TIM2 		; NO acia available on sidecard
	else
	 mov	 dx, EIAPT2+3		 ; lcr
	 CALL	 EIACHK 		 ; Check if EIA port is there
	endif
;
	TEST	BYTE PTR [DEVFLG+1],6	; No ACIA found => ERROR
	JNE	T_TIM2
	CALL	WRTINL			; Print error message
	DB	CR,'No RS232 found ',0
;
	CALL	SWTHRD			; read dip switch
	TEST	BL,1			; test diagnostic switch
	JNE	T_TIM2CR		; jump, if not set
;
	MOV	AX,OFFSET T_ACIA	; loop back address
	PUSH	AX			; Return address on stack
 ;
	JMP	DIAGNOSTIC		; Call diagnostic,
					; loop back address is on stack
	PRGSIZ	< Check: RS232> 	; Display program size
;
;DELETED-----------------------------------------------------------------------
;
; Checksum the BIOS ROM
;
ROMMSG:
	IF	MSGLNG			; Use long messages if true
	DB	'ROM bad Checksum = ',0
	ELSE
	DB	'ROM bad Chksm = ',0
	ENDIF
;
;- CHANGED (TB) ---------------------------------------------------------------
;						      
; Checkpoint 11 :  TIMER 2			  
;
;		   - Test internal counting of timer 2
;		     Test spare timer counter 2 to make sure it toggles
;		     all data bits both ways. We do this by ORing and
;		     ANDing the contents into a preset pattern.  Since
;		     the timer gate is enabled and could produce a sound
;		     on the speaker if we enabled the speaker, we can not
;		     allow this.  Fortunately the only thing that would
;		     enable it is a speaker beep in an Interrupt Service
;		     Routine. At this time, however, only the time-of-day
;		     interrupt is enabled and it won't do a beep.
;
;		   - Test external timer control lines
;		     Control timer 2 with GATE2 (Bit0 of system port1)
;		     Test timer 2 output level (Bit5 of system port2)
;
; Test # on printer port :  20h
;
; Exit :  OK -> 
;	 BAD -> Print error message.
; 
;------------------------------------------------------------------------------
;
public	t_tim2
T_TIM2CR:
	CALL	PRTLI0			; Set error flag and write CR,LF
T_TIM2:
;
	MOV	AH,020h    
	CALL	C_DISPLAY		; display test code
	CALL	C_TRIGGER		; provide for trigger pulse
;
	IF	TIMTEST

        MOV     AL,PITSL2+PITRL+PITMD3	; =10110110B
        OUT     PITMD,AL 		; TIMER2, LSB+MSB, MODE3, HEX-COUNT
        MOV     AL,0FFH
        OUT     PIT2,AL			; LOAD TIMER2 LSB
        MOV     AL,0FFH
        OUT     PIT2,AL			; LOAD TIMER2 MSB

 	MOV	DX,PPIB 		; Get current port value
	PUSH	DX			; Save registers so we can restore
					;   it later
	IN	AL,DX
	PUSH	AX
	AND	AL,LOW (NOT SPKDAT+SPKGAT)	; Turn off speaker
	OR	AL,SPKGAT		; and enable Timer 2 Counter gate enable					; (Its not really gated to the speaker)
	OUT	DX,AL			; And start it counting
;
	MOV	BX,000FFh
	XOR	CX,CX			; Set timeout for test
;
TIM2TS:
	MOV	DX,PITMD
	MOV	AL,PITSL2+PITLCH	; Latch counter
	OUT	DX,AL			; 
;
	DEC	DX			; Now read latched value
	PUSH	DX			; Delay 1000ns for chip I/O recov. time
;
	IN	AL,DX			; Get contents of LSB
	OR	BH,AL
	AND	BL,AL
	POP	DX			; Delay 1000ns for chip I/O recovery time
;
	IN	AL,DX			; Get contents of MSB
	OR	BH,AL
	AND	BL,AL
;
	CMP	BX,0FF00h		; If we reach this value then it's good
	JZ	TIM2_C
	LOOP	TIM2TS			; All bits haven't toggled, keep trying
	CALL	WRTINL
;
	IF	MSGLNG			; Use long messages if true
	DB	'Timer chip counter 2 failed',0
	ELSE
	DB	'Timer2 bad',0

	ENDIF
;
	CALL	PRTLI0			; Set error flag and write CR,LF
;
;- ADDED (TB) -----------------------------------------------------------------
;
TIM2_C: 				; Test timer2 control functions
       IN      AL,PPIB			; @@@
       MOV     BL,AL
       AND     AL,NOT SPKGAT
       OUT     PPIB,AL			; DISABLE TIMER2-GATE
       IN      AL,PPIC
       AND     AL,TM2OUT 
       JZ      ERR20			; TIMER2-OUT MUST BE HIGH
;
TP1:
       MOV     AL,PITSL2+PITRLL+PITMD3	; =10010110B
       OUT     PITMD,AL 		; TIMER2, LSB, MODE3, HEX-COUNT
       MOV     AL,0FFH
       OUT     PIT2,AL			; LOAD TIMER2
       MOV     AL,PITSL2+PITLCH
       OUT     PITMD,AL 		; TIMER2 IN LATCHING COUNT
       IN      AL,PIT2			; READ TIMER2-COUNT
       CMP     AL,0FFH			; TIMER STILL STANDING ?
       JNE     ERR20			; ERR2 IF NOT
       MOV     AL,BL
       OR      AL,SPKGAT    
       OUT     PPIB,AL			; ENABLE TIMER2-GATE
       MOV     CX,100H
;
TP2:
       IN      AL,PPIC			; READ TIMER2-OUT
       AND     AL,TM2OUT
       JE      TIM_OK			; TEST2 DONE ?
       LOOP    TP2			; WAIT FOR TIMER2-OUT LOW
;
ERR20:
	CALL	WRTINL
	DB	'Timer2 control function failed',0
;
	CALL	PRTLI0			; Set error flag and write CR,LF
;
TIM_OK:
	POP	AX
	POP	DX
	OUT	DX,AL			; Restore value of PPIB
	ENDIF		;TIMTEST
;
;-----------------------------------------------------------------------------
;
; Test DMA locations
;
	IF	DMATEST 		; DMA port tests
	CALL	DMATST			; Test ports
	ENDIF
;
;- CHANGED (TB) ---------------------------------------------------------------
;						      
; Checkpoint 12 :  KEYBOARD			  
;		   - Enable only the keyboard interrupt for now      
;
; Test # on printer port :  A0h
;
; Exit :  OK -> Keyboard enabled
;	 BAD -> Print error message.
; 
; Update:
; H1.1	: no missing keyboard error 
;
;------------------------------------------------------------------------------
;
public	t_key
T_KEY:
;
	MOV	AH,0A0h    
	CALL	C_DISPLAY		; display test code
	CALL	C_TRIGGER		; provide for trigger pulse
;
; Enable only the keyboard interrupt for now
;
; Set up the 8255 Programable Peripheral Interface (PPI) for reading the
;   keyboard
;
;	MOV	AH,PPIMSF+PPI1M0+PPIAIN+PPICUI+PPI2M0+PPICLI
;	CALL	PPISET
;
; Now let the Keyboard run
;
	XOR	DI,DI
	MOV	DS,DI
	ASSUME	DS:INTVEC
	MOV	DI,OFFSET KBDDMY	; Get dummy keyboard service routine
	XCHG	DI,DS:[24h]
;
KEYTST:
	MOV	DX,PPIB 		; Get contents of port PPIB
	IN	AL,DX			; @@@
	AND	AL,NOT (SNSWEN+KBDCLK)	;   and reset keyboard by turning off
					; Delay 850ns for chip I/O recovery time
	OUT	DX,AL			;   keyboard and it's clock

; NOT IMPLEMENTED 
;	 IF	 V20INST
;	 MOV	 CX,50000/10/9*CLK10	 ; delay, if NEC V20 CPU installed
;	 ELSE
	MOV	CX,50000/10/17*CLK10	; delay to allow keybd reset for 60 ms
;	 ENDIF 
;
KEYRST:
	LOOP	KEYRST			; wait around for ? ms
;
	CLI				; disable interrupts
	PUSH	AX
	CALL	KYINON			; Turn the keyboard interrupt on
	POP	AX
;
	XOR	AL,SNSWEN+KBDCLK	; now allow keyboard to run
	OUT	DX,AL			; and clear data port
;
	XOR	AL,SNSWEN		; turn on keyboard acknowledge
	XOR	CX,CX			; wait for interrupt 64K times
					; Delay 850ns for chip I/O recovery time
	OUT	DX,AL
;
	XOR	DH,DH			; Clear flag indicating keyboard OK
	STI				; allow interrupts !
;
WAITKEY:
	OR	DH,DH			; See if we got an interrupt
	LOOPZ	WAITKEY
;
	CALL	KYINOF			; Disable keyboard interrupt for now
;
	if	hydra
	 JMP	 GOTKEY 		; got an interrupt (h)
	else
	 jnz	 gotkey
	endif
BADKEY: 				; No keyboard during initialisation
;
	IF	KEYLOOP
	MOV	SI,OFFSET KY1MSG	; message with CR,LF
;
	CALL	WRTOUT			; Interrupt never occured
	JMP	SHORT KEYTST		; No, keyboard has a bad clock, reset,
					;   Ground or +5V wire not hooked up
					;   or is broken internally
	ELSE
	MOV	SI,OFFSET KY1MSG	; message with CR,LF
;
	CALL	WRTOUT			; Interrupt never occured
	CALL	WRTCRLF
	JMP	SHORT WAITK
	ENDIF
;
	IF	MSGLNG			; Use long messages if true
KEYMSG: DB	'h = Scancode, check Keyboard',CR,0
KY1MSG: DB	'No scancode from keyboard',CR,0
	ELSE
KEYMSG: DB	' '
KY1MSG: DB	'Keyboard Bad',CR,0			; error 301
	ENDIF
;
; If we get the interrupt we come here
;
GOTKEY:
	IF	KEYLOOP
	CMP	DL,0AAH 		; did keyboard do reset
	if	hydra
	 JMP	 okkey			; we got an 0AAh (h)
	else
	 jnz	 okkey
	endif

	CALL	PRTKEY
	JMP	SHORT BADKEY		; we didn't get an 0AAh, bad keyboard
					;  it could be the keyboard or the data
					;  input wire
OKKEY:
	MOV	DX,PPIB 		; Get current values
	IN	AL,DX			; @@@
	OR	AL,SNSWEN		; turn on keyboard acknowledge
					; Delay 850ns for chip I/O recovery time
	OUT	DX,AL
	XOR	AL,SNSWEN		; turn off keyboard acknowledge
					; Delay 850ns for chip I/O recov. time
;
;******** DO WE WAIT FOR A INTERRUPT OF A 0H CHARACTER OR JUST READ IT *******
;
	ELSE
	    CMP     DL,0AAH		; did keyboard do reset
	if	hydra
	 JMP	 waitk			; doesn't matter (h)
	else
	 jnz	 waitk 
	endif

	    CALL    PRTKEY		; Get scan code and print it
WAITK:
	ENDIF
;
; Test for stuck key
;
	XOR	DH,DH			; Clear flag indicating keyboard OK
	XOR	CX,CX			; wait for interrupt 64K times
;
	CALL	KYINON			; Enable keyboard interrupt
;
	IF	MEMSAVE
	    XOR     BL,BL		; clear flag
	ENDIF
;
WAITKY:
	OR	DH,DH			; See if we got an interrupt
	LOOPZ	WAITKY			; No interrupt yet
	CALL	KYINOF			; Disable keyboard interrupt for now
	if	hydra
	 JMP	 waitk0 		; no stuck key (h)
	else
	 jnz	 waitk0
	endif

;
	IF	MEMSAVE 
	MOV	BL,01h
	CMP	DL,MEMSAVE		; Get the memory save scan code
;
	IF	MSGLNG			; Use long messages if true
	JMP	WAITK0
	CALL	WRTINL
	DB	'Memory space preserved',CR,LF,0
	JMP	SHORT WAITK0
	ELSE
	JZ	WAITK0			;   Save memory, so not a real failure
	ENDIF
WAITK1:
	XOR	BL,BL			; Else clear flag
	ENDIF
;
	IF	MSGLNG			; Use long messages if true
	CALL	WRTINL
	DB	'Stuck key scancode = ',0
	MOV	AL,DL			; Get scan code
	CALL	PRTHE0			;   and print it
	ELSE
	CALL	PRTKEY			; Get scan code and print it
	ENDIF
;
WAITK0:
	MOV	DS:[24h],DI		; Put back the original interrupt vector
;
	CALL	SETDS40 		; Set DS to the ROM data segment
	ASSUME	DS:ROMDAT
;
; Set up the keyboard buffers
;
	MOV	AX,OFFSET KBUFR 	; set circular buffer
	MOV	KBGET,AX		; make empty
	MOV	KBPUT,AX
	MOV	KBXTGET,AX		; set up xt buffer
	ADD	AX,32
	MOV	KBXTPUT,AX
;
	MOV	AX,OFFSET KEYDUMMY	; Install keyboard interrupt vector
	MOV	KEYEXT,AX		; for extended keyboard routines
	MOV	AX,CS
	MOV	KEYEXT+2,AX
       PRGSIZ  < Check: Keyboard>      ; Display program size
;
;- ADDED (TB) -----------------------------------------------------------------
;						      
; Checkpoint 13 :  RAM ADDRESS	     
;		   The dynamic RAMs are being refreshed now, and parity checker
;		   is enabled. 
;		   - Write an address depending pattern to RAM 10000h...9FFFFh
;		   - Check installed RAM (dip-switch) for correct contents.
;		   - Find out: 1. RAM can't store datas at this location
;				  => R/W failure, will be tested later.
;			       2. RAM access at another location overwrites the
;				  datas here 
;				  => Address failure, print location.
;
; Test # on printer port :  60h
;
; Exit :  OK -> Leaves RAM cleared	      
;	 BAD -> Display failed RAM location	       
; 
;------------------------------------------------------------------------------
;
public	t_adr
T_ADR:
	DISPLAY 60			; display test code
	TRIGGER 			; provide for trigger pulse
;
	PUSH	BX			; save preserve flag 
	PUSH	ES			; and segment register
	PUSH	DS
;
	MOV	AL,DS:BYTE PTR DEVFLG	; read dip switch
	AND	AL,0CH			; ..MASK OUT MEMORY SIZE BITS
	SHR	AL,1			; USE AS SHIFT COUNTER
	SHR	AL,1
	MOV	CL,AL			; MEMORY SIZE  : 128K 256K 512K 640K
					; DIP SWITCH 3 :  on   off  on	 off
					; DIP SWITCH 4 :  on   on   off  off
	MOV	DX,128			; DIP SWITCH 4/3  00   01   10	 11
	SHL	DX,CL			; SHIFT TO REQUIRED POSITION
	CMP	DX,512			; MORE THAN 512KB?
	JBE	SETSIZE1		; IF NOT, OK	   
	MOV	DX,640			; MAXIMUM IS 640 KB
;
SETSIZE1:
	MOV	CL,6			; AND CONVERT TO # OF PARAGRAPH (1kB)
	SHL	DX,CL			; RESULT IN BX
	PUSH	DX			; save a copy for RAM R/W test
;
; Skip next test ?
; YES: reboot or preserve memory (Press F2 while power up)
; NO : coldboot and reset preserve flag
;
	PUSH	DS			; need other extra segment 
	POP	ES			; to test reboot flag
ASSUME	ES:ROMDAT
;
	CALL	TSTFLG			; we are on reboot ?
	JE	T_MAIN			; skip this test
	OR	BL,BL			; preserve flag set ?
	JNZ	T_MAIN			; skip this test
;								      
; Start of RAM address test
;
	MOV	BX,DX			; Get memsize in paragraphs
	MOV	CL,4			; and convert into # of 64KB blocks
	ROL	BX,CL			; result in BL
 ;
	DEC	BL			; SKIP 1ST PAGE
	MOV	BH,9			; WRITE UP TO $9FFFF  (640KB)
	MOV	DX,1000H		; ..DX INDICATES SEGMENT TO CLEAR
	XOR	AX,AX			; AX IS FILLED IN
	MOV	DS,AX
	CLD				; ..ENSURE AUTO INCRERMENTS
;
CLEAR_LOOP:
	MOV	ES,DX			
	XOR	DI,DI			; ES:DI -> BASE OF BANK TO CLEAR
	MOV	CX,8000H		; CLEAR 32K WORDS
;
CLEAR1:
	INC	AX			; FILL RAM WITH ADDRESS 
	STOSW				; DEPENDING CONTENTS
	LOOP	CLEAR1			; UNTIL BANK COMPLETE
	ADD	DX,1000H		; POINT TO NEXT 64K BANK
	INC	AX			; MAKE CONTENTS LOOKING DIFFERENT
	DEC	BH			; ..MORE TO DO?
	JNE	CLEAR_LOOP
;
	SUB    AX,AX			; NOW COMPARE INSTALLED MEMORY
	MOV    DX,1000H 		; STARTING FROM 10000
;
CLEAR2: 				; # OF 64 BLOCKS IN .BL
	MOV	ES,DX			
	XOR	DI,DI			; ES:DI -> BASE OF BANK TO CHECK
	MOV	CX,8000H		; NEXT 32K WORDS
;
CLEAR3:
	INC	AX			; COMPARE WITH EXPECTED CONTENTS
	SCASW
	LOOPE	CLEAR3
	JNE	ADDERR			; ERROR IF DIFFERENT
;
	ADD	DX,1000H		; POINT TO NEXT 64K BANK
	INC	AX	
	DEC	BL			; ..MORE TO DO?
	JNE	CLEAR2	  
	JMP	SHORT T_MAIN		; DONE !
;
ADDERR:
	SUB	DI,2			; ADJUST DI
	MOV	ES:[DI],AX		; FIND OUT: DATA OR ADDRESS ERROR
	CMP	AX,ES:[DI]		; DOES RAM STORE ?
	JNE	T_MAIN			; IF NE, RAM ERROR - WE WILL SEE LATER
;
	CALL	BADRAM			; ...FAILED, SHOW LOCATION
	PRGSIZ	< Check: Address >
;
;- CHANGED (TB) ---------------------------------------------------------------
;						      
; Checkpoint 14 :  MAIN RAM			  
;		   - Test memory from 2k to end which will assume 
;		     is 640k for now
;
; Test # on printer port :  62h
;
; Exit :  OK -> Leave RAM cleared.	     
;	 BAD -> Print error message.
; 
;------------------------------------------------------------------------------
;
public	t_main
T_MAIN:
;
	MOV	AH,062h    
	CALL	C_DISPLAY		; display test code
	CALL	C_TRIGGER		; provide for trigger pulse
;			 
	MOV	BP,0080h		; Start at 2nd k of memory
;		     
	POP	SI			; restore # of paragraph,
	POP	DS			; segment register 
	POP	ES		       
	POP	BX			; and preserve flag
;
	IF	MEMSAVE 
	OR	BL,BL
;
	IF	MEMTEST
	JZ	NOSAVE
	CALL	TSTSIZA
	JMP	SHORT MEM2		; Skip main memory test
	ELSE
	JNZ	MEM2
	ENDIF
;
NOSAVE:
	ENDIF
;
	IF	MEMTEST
	MOV	BL,01h			; Set flag to display tested memory
	PUSH	DS
	POP	ES
	ASSUME	ES:ROMDAT

	PUSH	SI			; Save it, next test will destroy it
;
; Enable NMI for memory tests etc.  (NOT for PC-Emulator)
;
;	CALL	NMIENB			; Enable NMI for memory tests
;
	CALL	TSTFLG			; Test reset flag for 1234h
	POP	SI
	JZ	MEM0
;
	CALL	CLRTST			; Clear and test memory
	PUSH	SI
	CALL	WRTCRLF
	POP	SI
MEM0:
	CALL	TSTSIZ			; 
	ENDIF				; MEMTEST
MEM2:
;
	MOV	AX,SI			; Move SI to AX
	MOV	CL,6			; Shift paragraph # right by 6 to get
	SHR	AX,CL			;   memory size in k
;
; Data segment is set to the ROM data segment already
;
	MOV	MEMSIZE,AX		; Save good memory space in # of k
					;   We may lower it later if switches
					;   tell us to.
	PRGSIZ	< Check: RAM main>	; Display program size
;
;- CHANGED (TB) ---------------------------------------------------------------
;
	IF	KEYTAP
	MOV	CLICKP,CLICKV	       ; Keyboard click volume
	ENDIF
;------------------------------------------------------------------------------
; Enable hardware interrupts now that we have set up for all of them
;
	CLI
;
	MOV	AL,0BCh 		; OWC1, enable hardware interrupts for
	MOV	DX,PIC0MSK		;   Timer ISR		(bit 0 = 0)
	OUT	DX,AL			;   Keyboard ISR	(bit 1 = 0)
					;   Disk ISR		(bit 6 = 0)
;
	STI
;
; looks for additional diagnostic rom in expansion slot
;
	CALL	CHECK_DIAG
;
; set up printer timeout table
;
	MOV	AX,1414h
	MOV	LPTOTBL,AX		; set printer 1 and 2
	MOV	LPTOTBL+2,AX		; 3 and 4 too	
;
; set eia timeouts
;
	MOV	AX,0101h
	MOV	EIATOTBL,AX
	MOV	EIATOTBL+2,AX
;
;------------------------------------------------------------------------------
;
; Test for a Game card
;
	IF	GAMTEST
	MOV	AH,0C3h    
	CALL	C_DISPLAY		; display test code
	CALL	C_TRIGGER		; provide for trigger pulse
;
	MOV	DX,GAMEPT		; Game controller I/O port address
	IN	AL,DX			;   Get values
	AND	AL,0Fh			; Test for the resistive inputs
					;   timed out to 0 on lower bits
	CMP	AL,0Fh
	JNE	SHORT T_EXTROM		; sorry no games allowed
;
	OR	BYTE PTR [DEVFLG+1],10h ; tell all we have a game card
;	 CALL	 WRTINL
;	 DB	 0 ;CR,'GAME   at ',0
;	 MOV	 AX,DX	
;	 CALL	 HEXW
;	 MOV	 SI,OFFSET HEXMSG
;	 CALL	 WRTOUT
;	 CALL	 WRTCRLF
;
	PRGSIZ	< Check: Game card>
	ENDIF
;
;------------------------------------------------------------------------------
;
; Test for alternate roms C8000 up to but not including F0000 **
;
public	t_extrom
T_EXTROM:
	IF	XTROM
	MOV	AH,0C2h    
	CALL	C_DISPLAY		; display test code
	CALL	C_TRIGGER		; provide for trigger pulse
;
	PUSH	LOTIME			; Save time-of-day
	MOV	SI,0C800h		; Starting segment of optional ROMs
	MOV	CX,0F000h		;  up to but not including paragraph **
	MOV	DL,00h			; Zero flag in DL saying we found at
					;   least 1 ROM
	CALL	XTRROM			; Search for extra ROM
	POP	LOTIME			; restore time-of-day
;
	ENDIF
	PRGSIZ	< Check: External ROM>	; Display program size
;
;- CHANGED (TB) ---------------------------------------------------------------
;						      
; Checkpoint 15 :  FLOPPY DRIVE 		  
;		   - Inits floppy drive
;		   - Seeks to track 0 and to track 34
;
; Test # on printer port :  D0h
;
; Exit :  OK -> Try boot		     
;	 BAD -> Print error message.
; 
;------------------------------------------------------------------------------
;
public	t_fdc
T_FDC:
;
	MOV	AH,0D0h    
	CALL	C_DISPLAY		; display test code
	CALL	C_TRIGGER		; provide for trigger pulse
;
	CALL	SETDS40 		; Set DS to the ROM data segment
	ASSUME	DS:ROMDAT
					; This routine needs this segment
	EXTRN	DOSEEK:NEAR		; Disk seek routine
	IF	DSKTEST
	XOR	AH,AH			; Reset disk system
	INT	13H			;   and ignore errors
	XOR	DX,DX			; Head 0, Drive 0
	XOR	CH,CH			; Track 0
	XOR	AH,AH			; Don't wait for motor to come to speed
	CALL	DOSEEK
	JC	DSKBD
	MOV	CH,34			; Track 34
	CALL	DOSEEK
	JNC	DSKOK
;
DSKBD:
	CALL	WRTINL			; Write error message to CRT
	DB	CR,'Drive A:  Bad',0	; error 601
	CALL	PRTLI0			; Set error flag and write CR,LF
DSKOK:
	ENDIF	;dsktest
	PRGSIZ	< Check: Floppy drive controller>  ; Display program size
;
;
;OTHER PLACE------------------------------------------------------------------
;
public	btdisk
BTDISK:
	MOV	AH,0FFh    
	CALL	C_DISPLAY		; display "power up ready "-code
	CALL	C_TRIGGER		; provide for trigger pulse
;
	CALL	OK_BEEP
;-----------------------------------------------------------------------------
; Enable interrupts from AMIGA side
;
	if	hydra
	 mov	 dx,prtpt1+1		; hidden JanIntEnable register (w/o)
	 xor	 al,al			; Bit6=0 enables interrupts
	 out	 dx,al

; and toggle keyboard enable line instad of R.J.'s "fake key"- routines 

	 mov	 dx,ppib
	 in	 al,dx
	 xor	 al,snswen 
	 out	 dx,al
	 xor	 al,snswen
	 out	 dx,al
;
	PRGSIZ	<Enable janus interrupts>  ; Display program size
	endif
;
;------------------------------------------------------------------------------
; Power up loop, if diagnostic switch is ON
;------------------------------------------------------------------------------
;
LOOPB:
	if	faraday
	 nop				; do nothing with faraday's chip
	else
	 IF	 XTPOST AND LPALWY
	 CALL	 SWTHRD 		 ; Read switches into BL
	 TEST	 BL,POST		 ; Should we loop on power up reset
					 ;   code ?
	 JNZ	 TSTLP0 		 ; No
	 JMP	 NEAR PTR RESET 	 ;   Yes, loop back to beginning
	ENDIF	;xtpost
	endif	;faraday
TSTLP0:
;------------------------------------------------------------------------------
; Found error:	decide, if we will boot or loop power up.
;------------------------------------------------------------------------------
;
	IF	ERRWAIT
	TEST	BYTE PTR RSTFLG,01h	; If error flag = 1
	JZ	ERRWT1			;   No, then everything is OK
;
ERRWT0: 				;	 and then wait for F1
	CALL	PRTLI0
	CALL	WRTINL
	DB	'Error. Press F1 key',0
;
	MOV	SI,OFFSET CONMSG
	CALL	GETKEY			; Get key in uppercase
;
	CMP	AX,03B00h		; Code for F1 key
	JNE	LOOPB			; Repeat until we get it, note that SI
					;   now points at a 0, so we print no
					;   string
	CALL	WRTCRLF
ERRWT1:
	ENDIF
;
	AND	BYTE PTR RSTFLG,0FEh	; Clear reset flag
;
public reboot
REBOOT:
	INT	19H			; boot up system
	JMP	SHORT REBOOT		; loop doing init
;
;
RESET	ENDP
	PRGSIZ	<Power on reset code>	; Display program size
;
;- CHANGED (TB) ---------------------------------------------------------------
;
; Search for Option ROMs
;
;	We use new vector location to call far the extra ROM:	    !!!
;	OLD = 0040:XROM_OFF ;  NEW = 0000:XROM_OFF		    !!!
;
;	Input:	SI = Starting paragraph segment
;		CX = 1st paragraphs not to test
;		DL, Bit 0 = 1, then don't wait for hard disk spin up
;		    Bit 1 = 1, then print CR,LF after finding optional ROM
;	Output:
;------------------------------------------------------------------------------
	IF	XTROM OR CRTROM
XTRROM	PROC	NEAR
	ASSUME	DS:NOTHING,ES:NOTHING
XROMLP:
	MOV	DS,SI
	ASSUME	DS:NOTHING
	PUSH	CX			; Save registers that the Optional ROMs
	PUSH	DS			;   may smash
	MOV	SI,800h/10h		; # of paragraphs per 2k ROM chunk
	CMP	WORD PTR DS:[0000h],0AA55H
					; word 0 should be AA55 if rom is there
	JNZ	XROMN			; no, next
;
; May have found a rom checksum it.	
;
	XOR	CL,CL
	MOV	CH,DS:[0002h]		; # of 512 byte blocks
;
	MOV	SI,CX
	SHR	SI,1			; Convert to paragraphs
	SHR	SI,1			; 
	SHR	SI,1			; 
	PUSH	SI
;
	SHL	CX,1			; CX = 512 * (# of blocks)
	CALL	CHKSUM
	JNZ	BADCK			; bad checksum
;
	XOR	AX,AX
	MOV	ES,AX			; Set ES to the INTVEC segment
	ASSUME	ES:INTVEC 
	MOV	ES:XROM_OFF,0003h	; offset of start of rom init code 
	MOV	ES:XROM_SEG,DS		; segment of rom init code
;
	IF	HDDELAY
	CALL	HRDDLY			; Delay if necessary for Hard disk
	ENDIF
;
	OR	DL,03h			; Set flag saying we found an optional
					;   ROM and yhave printed hard disk
					;   message
	PUSH	DX
	CALL	DWORD PTR ES:XROM_OFF	; call init code
	POP	DX
;
	IN	AL,PIC0MSK		; Get interrupt masks
;
	AND	AL,010111100b		; OWC1, enable hardware interrupts for
					;   Timer ISR		(bit 0 = 0)
					;   Keyboard ISR	(bit 1 = 0)
					;   Disk ISR		(bit 6 = 0)
					; Reset Masks for interrupts back to
	OUT	PIC0MSK,AL		;   initial values because some Optional
					;   ROMs kill them (at least they do
					;   kill the Timer interrupt)
	JMP	SHORT CHKSOK		; Checksum was OK
BADCK:
	PUSH	AX
	MOV	AX,DS
	CALL	HEXW
	CALL	WRTINL
;
;
	IF	MSGLNG			; Use long messages if true
	DB	'0h Optional ',0
	ELSE
	DB	' Op ',0
	ENDIF
;
	POP	AX
	MOV	SI,OFFSET ROMMSG
	CALL	PRTHEX
;
CHKSOK:
	POP	SI			; Restore # of paragraphs to next ROM
XROMN:
	POP	CX			; Get old paragraph from DS into CX
	ADD	SI,CX			; Add it to SI and try next ROM block
;
	POP	CX			; Restore count
	CMP	SI,CX			; Is it past end ?
	JC	XROMLP			;   No, then do next ROM
	TEST	DL,02h			; If an optional ROM was found,
	JZ	XROMN0			;   No
	CALL	WRTCRLF 		;   Yes, output a CR,LF in case a
					;     message was displayed and didn't
					;     do one
XROMN0:
	RET				; 
XTRROM	ENDP
	ENDIF	;	XTROM OR CRTROM
	PRGSIZ	<Search for option ROMs>	; Display program size
;
;- ADDED (TB) -----------------------------------------------------------------
;						      
; 2. Diagnostic Checkpoint :
;
; This routines looks for a diagnostic ROM at location $C000:0 and enter it
; at @ offset 7 if present. This POST-BIOS entry is used if errors, but
; not fatal errors, occurs during power-up.
;								 
; Input: DIAG_SEG   =  segment of diagnostic ROM ($C000h)
;	 XROM_CHECK =  extra rom marker 1 (AA55h)
;	 ERR_CHECK  =  diag. rom marker 2 (6A65h)
;	 POST_BIOS  =  offset of POST-BIOS entry  
;
;------------------------------------------------------------------------------
;
public	check_diag
CHECK_DIAG PROC NEAR
;
	PUSH	AX
	PUSH	DS
;
	MOV	AX,DIAG_SEG  
	MOV	DS,AX			; Point to diagnostic rom
ASSUME	DS:DIAG_SEG
;
	CMP	WORD PTR CHECKWORD1,XROM_CHECK
	JNE	BOOT			; is there an extra rom ?
;
	CMP	WORD PTR CHECKWORD2,ERR_CHECK
	JNE	BOOT			; ..and is it the diagnostic rom ?
;
	CALL	DIAG_SEG:POST_BIOS	; YES -> call far DIAG-BIOS entry
;					  NO  -> wait for F1 and try to boot 
BOOT:
	POP	DS
	POP	AX
ASSUME	DS:ROMDAT
	RET
;
CHECK_DIAG ENDP
	PRGSIZ <Search for diagnostic ROM>
;
;------------------------------------------------------------------------------
; Print message followed by hex message
;------------------------------------------------------------------------------
PNXMAC2 MACRO				; Include PRTKEY code
	ASSUME	DS:NOTHING,ES:NOTHING
	MOV	SI,OFFSET KEYMSG	; message with CR,LF
	MOV	AL,DL			; Get scan code
	CALL	HEXB			;   and print it
	ENDM
;------------------------------------------------------------------------------
; Print message followed by hex message
;------------------------------------------------------------------------------
PNXMAC1 MACRO
PRTLIN	PROC
	ASSUME	DS:NOTHING,ES:NOTHING
	CALL	WRTOUT
PRTLI0:
	CALL	SETERF			; Set error flag
	JMP	NEAR PTR WRTCRLF	; and return
PRTLIN	ENDP
	ENDM
;
PRTKEY	PROC	NEAR
	PNXMAC2 			; Include PRTKEY code
	PNXMAC1 			; Include PRTLIN code
PRTKEY	ENDP
;
;------------------------------------------------------------------------------
; Test reboot flag 
;
;	Input:	ES = ROM data segment (0040h)
;	Output: SI = Reset Flag, lower bit is 0
;		Zero Flag set if upper 15 bits is 1234h, else reset
;------------------------------------------------------------------------------
TSTFLG	PROC	NEAR			; Test reset flag for 1234h
	ASSUME	DS:NOTHING,ES:ROMDAT
	MOV	SI,RSTFLG		; Get Reset flag
	AND	SI,0FFFEh		; Get rid of lowest bit in case this
					;   flag has been set by an error
	CMP	SI,1234h		; Check RESET FLAG MEMORY
	RET
TSTFLG	ENDP
	PRGSIZ	<Test reboot flag>	; Display program size
;------------------------------------------------------------------------------
; Delay for hard disk spin up
;------------------------------------------------------------------------------
HRDDLY	PROC	NEAR
	ASSUME	DS:NOTHING,ES:ROMDAT
	IF	HDDELAY
	TEST	DL,01h			; If we already waited, skip it
	JNZ	HDWAI0
	CALL	TSTFLG			; Test reset flag for 1234h
	JZ	HDWAI0			; It was a reboot, skip spin up
;
	IF	MSGLNG			; Use long messages if true
	CMP	LOTIME,HDDELAY*182/100	; Time to wait for hard disk to speed
					;  up
	JNC	HDWAI0
	CALL	WRTINL
	DB	'Waiting for hard disk spin up...',0
	ENDIF
;
HDWAIT:
	CMP	LOTIME,HDDELAY*182/100	; Time to wait for hard disk to speed
					;  up
	JC	HDWAIT
;
	IF	MSGLNG			; Use long messages if true
	CALL	WRTINL
	DB	'OK',CR,LF,0
	ENDIF
;
HDWAI0:
	RET
	ENDIF		;HDDELAY
HRDDLY	ENDP
	PRGSIZ	<Delay for hard disk spin up>	; Display program size
;
;------------------------------------------------------------------------------
	IF	ROMSIZ GT (8*1024)
	SYNC	0E000h-1		; This sync address makes sure that
					;   the code not in the normal 8k does
					;   not overflow into it
	DB	0			; Space for checksum for lower 8K of ROM
	ENDIF
;------------------------------------------------------------------------------
;
	IF	ROMSIZ GT (8*1024)
	RESYNC	FDBOOTA 		;INT 19H service (1) 
	ELSE
	SYNC	FDBOOTA 		;INT 19H service (1) 
	ENDIF
;------------------------------------------------------------------------------
;
	TOTAL
;
CODE	ENDS
	END	
