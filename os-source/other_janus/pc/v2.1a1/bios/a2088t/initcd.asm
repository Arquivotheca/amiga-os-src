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
;
;	HY4.0:	 09/05/89	      by TB for 10MHz PC-Emulator A2088T
;	HY4.1:   03/09/90	      support HD FDD's (add floppy.asm)
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
	EXTRN	SPEED:BYTE	; Crystal speed flag
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
	EXTRN	DS40H:NEAR
	EXTRN	SWTHRD:NEAR
	EXTRN	INTTST:NEAR
	EXTRN	DIAGNOSTIC:NEAR
	EXTRN	CLRTST:NEAR
	EXTRN	C_DISPLAY:NEAR
	EXTRN	C_TRIGGER:NEAR
	EXTRN	LOCK_FARADAY:NEAR
	EXTRN	WAIT_FOR_AMIGA:NEAR
	EXTRN	CRTBEEP:NEAR
	EXTRN	CRTTST:NEAR
	EXTRN	XTRROM:NEAR
	EXTRN	MORETESTS:NEAR

	PUBLIC	RESET

;	PUBLIC	ROMBGN
;ROMBGN: 					; Beginning of ROM
;
;FILLSIZ EQU	0FFFFh-ROMSIZ+1
;	SYNC	FILLSIZ
	SYNC	0E000h
;
	MANSION1		; Padded space or MANufacturer's SIgnON part 1
	PRGSIZ	<Reserved / Optional Mfr Signon part 1> ; Display program size
;
	SYNC	CPYNOTA
	PUBLIC	CPYNOT
CPYNOT:
	DB	"Copyright 1989 "
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
;
;  Switch system to normal (4.77MHz) speed
;
	 mov	 al,FA_Setup477 	; 4.77MHz, Dis_Parity, no_8087, Dis_RAM
;	 mov	 al,FA_Setup256 	; Dis_Parity, no_8087, Dis_RAM
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
	CMP	RSTFLG,1234h		; If this was a CTRL-ALT-DEL
	je	no_Lock 		; skip next test

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

;	call	Lock_Faraday		; lock faraday configuration register
	endif				; VMR14.11.89

no_Lock:
	call	lock_faraday		; VMR14.11.89
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
RESET	ENDP
	PRGSIZ	<Power on reset code>	; Display program size
;
	SYNC	FDBOOTA 		;INT 19H service (1) 
;
	TOTAL
;
CODE	ENDS
	END	
