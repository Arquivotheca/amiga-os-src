	IFNDEF LISTINC
.XLIST
.XCREF
	ENDIF
;	NAME	ROMEQU
;	TITLE	Equates for the ROM
;***************************************************************************
;* Copyright (C) 1985 by Phoenix Software Associates Ltd.  This program    *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent. 		   *
;***************************************************************************
;
; XT EXPANSION PORTS
;
EXPBOX		EQU	210h		; Expansion box circuitry base port
EXPLCH		EQU	EXPBOX+00h	; latch Expansion box bus data command
EXPRDD		EQU	EXPBOX+00h	; Read latched Expansion box bus data
EXPWTCL 	EQU	EXPBOX+01h	; Clear wait test latch of
					;   Expansion box
EXPHIGH 	EQU	EXPBOX+01h	; Read high-order address bits of
					;   Expansion box
EXPLOW		EQU	EXPBOX+02h	; Read low-order address bits of
					;   Expansion box
EXPCMD		EQU	EXPBOX+03h	; Expansion box command
EXPST		EQU	EXPBOX+03h	; Expansion box status
;
; CRT 6845
;
CRTCTL		EQU	3B4H		; Monochrome CRT base port
COLORCTL	EQU	3D4H		; Color CRT base port
;
; PRINTER ADDRESSES
;
PRTPT1		EQU	378H		; 1st printer I/O port address
PRTPT2		EQU	3BCH		; Printer I/O port address on
					;   monochrome board
PRTPT3		EQU	278H		; 3rd printer I/O port address
;
; RS232 Card Addresses
;
EIAPT1		EQU	3F8H		; 1st async card I/O port address
EIAPT2		EQU	2F8H		; 2nd async card I/O port address
;
; 765 Floppy Disk Controller (FDC)
;
X765		EQU	3F4H		; Base Port of 765 FDC
;
; 8237 DMA Controller equates
;
X8237		EQU	00H		; Base Port of 8237 DMA
;
; 8253 Programable Interval Timer (PIT) (or 8254)
;
X8253		EQU	40H		; Base Port of 8253 (or 8254)
;
; 8255 Programable Peripheral Interface (PPI)
;
X8255		EQU	60H		; Base Port of 8255
;
; 8259 Programable Interrupt Contoller (PIC)
;
X8259		EQU	20H		; Base Port of 8259
;
; GAME CARD ADDRESS
;
GAMEPT		EQU	201H		; Game card I/O port address
;
; Boot Diagnostic port
;
DIAGNP		EQU	X8255		; Diagnostic port for a non-AT

;******************************************************************************
;
; Miscellaneous Disk equates
;
;
;
FDCDOR		EQU	3F2H		; FDC Digital Output register
;
;******************************************************************************
;
NMIMSK		EQU	0A0h		; NMI Mask register
NMIOFF		EQU	00h		; Disable NMI'S
NMION		EQU	80h		; Enable  NMI'S
;
;******************************************************************************
;
; ASCII equates
;
NULL		EQU	00h		; Null
BELL		EQU	07h		; Bell
BS		EQU	08h		; Backspace
TAB		EQU	09h		; Tab
LF		EQU	0Ah		; Line Feed
FF		EQU	0Ch		; Printer Form Feed
CR		EQU	0Dh		; Carriage Return
ESC		EQU	1Bh		; Escape
;******************************************************************************
;
; 8237 DMA Controller equates
; (Note: the port addresses are really like they are specified below. )
;
DMAPAG		EQU	80H		; Base address (not used)
;
DMAPG0		EQU	DMAPAG+03h	; Dma page register channel 0
					;   (not needed for memory refresh
					;    except for some bad memory board
					;    designs where it must be 0 - this
					;    of course messes up DMAPG1 below)
;
DMAPG1		EQU	DMAPAG+03h	; Dma page register channel 1 (free)
DMAPG2		EQU	DMAPAG+01h	; Dma page register channel 2 (disk)
DMAPG3		EQU	DMAPAG+02h	; Dma page register channel 3 ( ? )
;
; Dma channel 4 is used for a DMA cascade and does not have a page register
;
DMAPG5		EQU	DMAPAG+0Bh	; Dma page register channel 5
DMAPG6		EQU	DMAPAG+09h	; Dma page register channel 6
DMAPG7		EQU	DMAPAG+0Ah	; Dma page register channel 7
;
PGREFR		EQU	DMAPAG+0Fh	; Refresh page register
;
PAGE00		EQU	DMAPAG+00h	; Extra page register (not used in DMA)
PAGE04		EQU	DMAPAG+04h	; Extra page register (not used in DMA)
PAGE05		EQU	DMAPAG+05h	; Extra page register (not used in DMA)
PAGE06		EQU	DMAPAG+06h	; Extra page register (not used in DMA)
PAGE08		EQU	DMAPAG+08h	; Extra page register (not used in DMA)
PAGE0C		EQU	DMAPAG+0Ch	; Extra page register (not used in DMA)
PAGE0D		EQU	DMAPAG+0Dh	; Extra page register (not used in DMA)
PAGE0E		EQU	DMAPAG+0Eh	; Extra page register (not used in DMA)

;
; 8237 DMA Register address equates
;
DMA0ADR 	EQU	X8237+00h	; CHANNEL 0 CURRENT ADDRESS
DMA0CNT 	EQU	DMA0ADR+01h	; CHANNEL 0 WORD COUNT
DMA1ADR 	EQU	DMA0ADR+02h	; CHANNEL 1 CURRENT ADDRESS
DMA1CNT 	EQU	DMA0ADR+03h	; CHANNEL 1 WORD COUNT
DMA2ADR 	EQU	DMA0ADR+04h	; CHANNEL 2 CURRENT ADDRESS
DMA2CNT 	EQU	DMA0ADR+05h	; CHANNEL 2 WORD COUNT
DMA3ADR 	EQU	DMA0ADR+06h	; CHANNEL 3 CURRENT ADDRESS
DMA3CNT 	EQU	DMA0ADR+07h	; CHANNEL 3 WORD COUNT
;
DMAST		EQU	DMA0ADR+08h	; DMA Read Status Register
DMACMD		EQU	DMA0ADR+08h	; DMA Command Register
DMAREQ		EQU	DMA0ADR+09h	; DMA Write Request Register
DMARQST 	EQU	DMA0ADR+0Ah	; DMA Write Single Mask Register bit
DMAMD		EQU	DMA0ADR+0Bh	; DMA Write Mode Register
DMACLFF 	EQU	DMA0ADR+0Ch	; DMA Clear Byte Pointer Flip Flop
DMARDT		EQU	DMA0ADR+0Dh	; DMA Read Temporary Register
DMACLR		EQU	DMA0ADR+0Dh	; DMA Master Clear
DMAMCL		EQU	DMA0ADR+0Eh	; DMA Clear Mask register
DMAMSK		EQU	DMA0ADR+0Fh	; DMA Write All Mask Register Bits
;
; 8237 DMA Master Register address equates for the AT
;
DMM0ADR 	EQU	0C0h+00h	; CHANNEL 0 CURRENT ADDRESS
DMM0CNT 	EQU	DMM0ADR+02h	; CHANNEL 0 WORD COUNT
DMM1ADR 	EQU	DMM0ADR+04h	; CHANNEL 1 CURRENT ADDRESS
DMM1CNT 	EQU	DMM0ADR+06h	; CHANNEL 1 WORD COUNT
DMM2ADR 	EQU	DMM0ADR+08h	; CHANNEL 2 CURRENT ADDRESS
DMM2CNT 	EQU	DMM0ADR+0Ah	; CHANNEL 2 WORD COUNT
DMM3ADR 	EQU	DMM0ADR+0Ch	; CHANNEL 3 CURRENT ADDRESS
DMM3CNT 	EQU	DMM0ADR+0Eh	; CHANNEL 3 WORD COUNT
;
DMMST		EQU	DMM0ADR+10h	; DMA Read Status Register
DMMCMD		EQU	DMM0ADR+10h	; DMA Command Register
DMMREQ		EQU	DMM0ADR+12h	; DMA Write Request Register
DMMRQST 	EQU	DMM0ADR+14h	; DMA Write Single Mask Register bit
DMMMD		EQU	DMM0ADR+16h	; DMA Write Mode Register
DMMCLFF 	EQU	DMM0ADR+18h	; DMA Clear Byte Pointer Flip Flop
DMMRDT		EQU	DMM0ADR+1Ah	; DMA Read Temporary Register
DMMCLR		EQU	DMM0ADR+1Ah	; DMA Master Clear
DMMMCL		EQU	DMM0ADR+1Ch	; DMA Clear Mask register
DMMMSK		EQU	DMM0ADR+1Eh	; DMA Write All Mask Register Bits
;
; 8237 DMA Register bit equates
;
DMASET		EQU	04h		; Set bit (mask or request)
DMARST		EQU	00h		; Reset bit (mask or request)
;
DMACH0		EQU	00h		; Channel 0 Select
DMACH1		EQU	01h		; Channel 1 Select
DMACH2		EQU	02h		; Channel 2 Select
DMACH3		EQU	03h		; Channel 3 Select
;
DMAVER		EQU	00h		; Verify transfer
DMAWRT		EQU	04h		; Write transfer
DMARD		EQU	08h		; Read transfer
;
DMAAUT		EQU	10h		; Auto-initialization enable
;
DMADCR		EQU	20h		; Address decrement select
;
DMADMD		EQU	00h		; Demand mode select
DMASMD		EQU	40h		; Single mode select
DMABMD		EQU	80h		; Block mode select
DMACAS		EQU	0C0h		; Cascade mode select
;
;******************************************************************************
;
; Math Co-Processor (80287)
;
MATHBSY 	EQU	0F0h		; Math Co-Processor Clear Busy latch
MATHRST 	EQU	0F1h		; Math Co-Processor Reset

;******************************************************************************
;
; 8253 Programable Interval Timer (PIT) Equates
;
PIT0		EQU	X8253		; Timer 0
PIT1		EQU	X8253+1h	; Timer 1
PIT2		EQU	X8253+2h	; Timer 2
PITMD		EQU	X8253+3h	; Mode port
;
;	Mode Port Bit Equates
;
PITSL2		EQU	80h		; Select counter 2
PITSL1		EQU	40h		; Select counter 1
PITSL0		EQU	00h		; Select counter 0
;
PITRL		EQU	30h		; Read/load LSB 1st, then MSB
PITRLM		EQU	20h		; Read/load MSB only
PITRLL		EQU	10h		; Read/load LSB only
PITLCH		EQU	00h		; Counter latching operation
;
PITMD0		EQU	00h		; Mode 0, Interrupt on terminal count
PITMD1		EQU	02h		; Mode 1, Programmable 1 shot
PITMD2		EQU	04h		; Mode 2, Rate generator
PITMD3		EQU	06h		; Mode 3, Square wave generator
PITMD4		EQU	08h		; Mode 4, Software triggered strobe
PITMD5		EQU	0Ah		; Mode 5, Hardware triggered strobe
;
PITBCD		EQU	01h		; BCD Counter, 4 decades
;******************************************************************************
;
; 8255 Programable Peripheral Interface (PPI)
;
PPIA		EQU	X8255+0h	; Port A, used for Keyboard Scan code
					;   input
PPIB		EQU	X8255+1h	; Port B
PPIC		EQU	X8255+2h	; Port C
PPICTL		EQU	X8255+3h	; Control port
;
;	Port PPIB Bit Equates
SPKGAT		EQU	01h		; Timer 2 Gate Speaker
SPKDAT		EQU	02h		; Speaker data
;
NMICTL		EQU	PPIB		; NMI Control port
NMISTP		EQU	PPIC		; NMI Status port
;		EQU	04h		; Read/Write Memory Size
CASOFF		EQU	08h		; Cassette Motor Off
MEMOFF		EQU	10h		; Disable Main board Read/Write Memory
					;   and reset parity error flop for it
IOCHKO		EQU	20h		; Disable I/O Channel check -
					;   disable Read/Write Memory for I/O
					;   boards and reset parity error flop
KBDCLK		EQU	40h		; Enable Keyboard Clock
					;   If held low for ? ms, performs a
					;     reset of keyboard
SNSWEN		EQU	80h		; Clear keyboard and Enable Sense
					;   Switches
;
;	Port PPIC Bit and device word Equates
;
POST		EQU	01h		; Loop on Powerup code
COPROC		EQU	02h		; 8087 installed
RAM0		EQU	04h		; Mother board RAM size installed
RAM1		EQU	08h		;    "     "	"    "	    "
;
DIAG		EQU	POST
;
CASIN		EQU	10h		; Cassette Data Input
TM2OUT		EQU	20h		; Timer 2 Out
IOCHK		EQU	40h		; I/O Channel Check - RAM Parity check
					;   for memory on I/O boards
PARCHK		EQU	80h		; RAM Parity Check for memory on main
					;   board
;
;	Port PPICTL Mode control Bit Equates
;
PPIMSF		EQU	80h		; Mode Set Flag, else bit set/reset mode
PPI1M2		EQU	40h		; Group 1 - Mode 2
PPI1M1		EQU	20h		; Group 1 - Mode 1
PPI1M0		EQU	00h		; Group 1 - Mode 0
PPIAIN		EQU	10h		; PPI port A is input, else output
PPICUI		EQU	08h		; PPI port C Upper bits are input,
					;   else output
PPI2M1		EQU	04h		; Group 2 - Mode 1
PPI2M0		EQU	00h		; Group 2 - Mode 0
PPIBIN		EQU	02h		; PPI port B is input, else output
PPICLI		EQU	01h		; PPI port C Lower bits are input,
					;   else output
;******************************************************************************
;
; FARADAY chip special registers and setups
;
FA_control	EQU	61h		; (r/w) normal system control register
FA_switch	EQU	62h		; (w) system switch register
FA_config	EQU	63h		; (w) chip configuration register
;
					; values to set this register:
FA_setup256	EQU	01h		; no parity, no 8087, enable ext. RAM
FA_setup512	EQU	05h		; no parity, no 8087, enable int. RAM
FA_lock 	EQU	08h		; lock register
FA_8087 	EQU	02h		; enable 8087
FA_lock8087	EQU	0ah		; 8087, lock reg.
FA_swhigh	EQU	04h		; read higher half of switch if set
FA_xxxxxxxx	EQU	00h		; free


;******************************************************************************
;
; 8259 Programable Interrupt Contoller (PIC)
;
PIC0		EQU	X8259+0 	; PIC #0
PIC0MSK 	EQU	PIC0+1		; PIC #0 (master) mask
;
PIC1		EQU	0A0h		; PIC #1
PIC1MSK 	EQU	PIC1+1		; PIC #1 (slave) mask
;
;	Port PIC Bit Equates for ICW1
ICW1		EQU	10h		; ICW1 is being issued
LTIM		EQU	08h		; Level Triggered Interrupt Mode, else
					;   edge-triggered
ADI		EQU	04h		; Call Address Interval = 4, else 8
					;   no effect in an 8086 or 8088 system
SNGL		EQU	02h		; Single 8259 in system, no ICW3 will be
					;   issued
IC4		EQU	01h		; ICW4 will be issued, else not
;
;	Port PIC Bit Equates for ICW4
SFNM		EQU	10h		; Special Fully Nested Mode
BUFMAS		EQU	0Ch		; Buffered Master mode
BUFSLV		EQU	08h		; Buffered Slave mode
AEOI		EQU	02h		; Automatic End of Interrupt Mode
UPM86		EQU	01h		; Sets for 8088/8086 operation, else
					;   8080/8085 mode
;
;	Port PIC Bit Equates for OCW2
OCW2		EQU	00h		; OCW2 is being issued
;
EOISR		EQU	0E0h		; Rotate on specific EOI
PRIORT		EQU	0C0h		; Set priority command
EOINSR		EQU	0A0h		; Rotate on non-specific EOI
RAEOIS		EQU	80h		; Rotate in Automatic EOI mode (SET)
EOISP		EQU	60h		; Specific End of Interrupt
EOINOP		EQU	40h		; No operation
EOINSP		EQU	20h		; Non-specific End of Interrupt
RAEOIC		EQU	00h		; Rotate in Automatic EOI mode (CLEAR)
;
;	Port PIC Bit Equates for OCW3
OCW3		EQU	08h		; OCW3 is being issued
SPMSKS		EQU	30h		; Set Special Mask
SPMSKR		EQU	20h		; Reset Special Mask
POLL		EQU	04h		; Poll command
IRREGR		EQU	02h		; Read IR register on next read pulse
ISREGR		EQU	02h		; Read IS register on next read pulse
;
;******************************************************************************
;
; CRTDSR.ASM EQUATES
;
;
CRTCAD		EQU	0000h	; CRTC register address select port offset
CRTCDT		EQU	0001h	; CRTC data register port offset
;
;
MSRADR		EQU	0004h	; Mode Select Register port address offset
ALPH80		EQU	01h		; Select 80 x 25 Alphanumeric mode
					;   else 40 x 25 alphanumeric mode
GRF320		EQU	02h		; Selects 320 x 200 graphics mode
					;   else alphanumeric mode
BLKWHT		EQU	04h		; Selects black and white mode,
					;   else color
VIDEN		EQU	08h		; Enables video out, else disables
GRF640		EQU	10h		; Selects 640 x 200 graphics mode
BLINK		EQU	20h		; Selects attribute bit # ?  to blink
					;   instead of changing intensity
;
;
CSRADR		EQU	0005h	; Color Select Register port address offset
BLUE		EQU	01h		;
GREEN		EQU	02h		;
RED		EQU	04h		;
INTENS		EQU	08h		; Intensified color
ALTCOL		EQU	10h		;
SELCOL		EQU	20h		; Select Active color set in 320 x 200
					;   graphics mode
;
;
CRTST		EQU	0006h	; CRT Graphic Status Register port offset
BLANK		EQU	01h		; CRT Display is blanked for horizontal
					;   or vetical retrace
LGHTRG		EQU	02h		; Light Pen trigger made
LGHTSM		EQU	04h		; Light Pen Switch made,
					;   it is not debounced
VERSYC		EQU	08h		; Vertical Sync blanking period
;
;
LGHTRST 	EQU	0007h	; Reset Light Pen Latch port offset
LGHTSET 	EQU	0008h	; Set Light Pen Latch port offset
;
; CRTC 6845 REGISTER DEFINITIONS
;
HZTOT		EQU	0		; R0: HORIZONTAL TOTAL
HZDISP		EQU	1		; R1: HORIZONTAL DISPLAYED
HZSYNCP 	EQU	2		; R2: HORIZONTAL SYNC POSITION
HZSYNCW 	EQU	3		; R3: HORIZONTAL SYNC WIDTH
VTTOT		EQU	4		; R4: VERTICAL TOTAL
VTDISP		EQU	5		; R5: VERTICAL DISPLAYED
VTSYNCP 	EQU	6		; R6: VERTICAL SYNC POSITION
VTSYNCW 	EQU	7		; R7: VERTICAL SYNC WIDTH
INTLACE 	EQU	8		; R8: INTERLACE MODE
MAXSCNL 	EQU	9		; R9: MAXIMUM SCAN LINES
CURSTRT 	EQU	0AH		; R10: CURSOR START
CUREND		EQU	0BH		; R11: CURSOR END
MEMADRH 	EQU	0CH		; R12: REGEN MEMORY ADDR (HI)
MEMADRL 	EQU	0DH		; R13: REGEN MEMORY ADDR (LO)
CURADRH 	EQU	0EH		; R14: CURSOR ADDRESS (HI)
CURADRL 	EQU	0FH		; R15: CURSOR ADDRESS (LO)
LPADRH		EQU	10H		; R16: LIGHT PEN ADDR (HI)
LPADRL		EQU	11H		; R17: LIGHT PEN ADDR (LO)
;
;******************************************************************************
;
; Descriptor Access byte equates
;
TSKAVB	EQU	01h		; Available Task State segment
LDTDSC	EQU	02h		; LDT Descriptor
BUSTSK	EQU	03h		; Busy task state segment
CALLGT	EQU	04h		; Call Gate
TASKGT	EQU	05h		; Task Gate
INTRGT	EQU	06h		; Interrupt Gate
TRAPGT	EQU	07h		; Trap Gate


PRESENT EQU	80h		; Segment is present or valid
DPLMSK	EQU	60h		; Descriptor privledge level mask
SEGDSC	EQU	10h		; Segment descriptor
XQTABL	EQU	08h		; Executable
EXPDWN	EQU	04h		; Expand down
CONFRM	EQU	04h		; Conforming
WRTABL	EQU	02h		; Writeable
READBL	EQU	02h		; Readable
ACCESS	EQU	01h		; Accessed
;
;******************************************************************************
;
;	Define location and length of BIOS-ROM
;	Date : 11/11/85 , now we use only 16k-version
;				
ROM_SEG 	EQU	0F000h	; Segment of BIOS-ROM
ROMSIZE 	EQU	04000h	; Size of BIOS-ROM
ROM_OFF 	EQU	0C000h	; Offset of BIOS-ROM
;
;******************************************************************************
;
;	Define location, entrys and checkwords of diagnostic ROM
;	Date : 11/11/85, we use two checkwords	
;			 now we have three entrys
;				
XROM_CHECK	EQU	0AA55h	; extra rom checkword (offset 0)
ERR_CHECK	EQU	06A65h	; trouble shooting rom checkword (offset 5)
DIAG_CHECK	EQU	0656Ah	; diag. rom checkword (offset 9)
;
;******************************************************************************
;
;	Locations in the ROM that have been determined or are believed to be
;	important to maintain compatibility
;
BASIC_INT	EQU	0C003h	;BASIC trap
RESETA		EQU	0E05Bh	;Reset routine (1)
NMIINTA 	EQU	0E2C3h	;(AT only) NMI Handler
HDPARMA 	EQU	0E401h	;(AT only) Hard disk drive
				; characteristics table
FDBOOTA 	EQU	0E6F2h	;INT 19H service (1)
BAUDTBA 	EQU	0E729h	;Baud table
EIADSRA 	EQU	0E739h	;EIA INT
KYBDSRA 	EQU	0E82Eh	;INT 16H service
KYBISRA 	EQU	0E987h	;INT 09H keyboard interrupt
DSKDSRA 	EQU	0EC59h	;INT 13H service
DSKISRA 	EQU	0EF57h	;INT 0EH diskette interrupt
DSKPRMA 	EQU	0EFC7h	;INT 1EH pointer to disk parms
PRTDSRA 	EQU	0EFD2h	;INT 17H service
CRTVECA 	EQU	0F045h	;table of video routines (2)
CRTDSRA 	EQU	0F065h	;INT 10H service
CRTPRMA 	EQU	0F0A4h	;INT 1DH
MEMCHKA 	EQU	0F841h	;INT 12H
EQPMTCA 	EQU	0F84Dh	;INT 11H
CASDSRA 	EQU	0F859h	;INT 15h Cassette interrupt
ROMFNTA 	EQU	0FA6Eh	;character maps for graphics
TIMDSRA 	EQU	0FE6Eh	;INT 1AH
TIMISRA 	EQU	0FEA5h	;INT 08H
VECTBLA 	EQU	0FEF3h	;interrupt vector table (3)
INTHDLA 	EQU	0FF23h	;Interrupt handler in non-AT, in AT
				;   table of Interrupt vectors 70h -> 77h
DUMINTA 	EQU	0FF53h	;IRET instruction (4)
PRTSCRA 	EQU	0FF54h	;INT 5 service
POWRUPA 	EQU	0FFF0h	;Power up routine entry point
VERDATA 	EQU	0FFF5h	;Date in ASCII
COMTYPA 	EQU	0FFFEh	;Computer type, PC = 0FFh, XT = 0FEh, ...
CHKSUMA 	EQU	0FFFFh	;Location of checksum byte, not currently
				;  known where it should be located
ROMENDA 	EQU	0FFFFh	;ROM end
;
CPYNOTA 	EQU	0E000h+32	;Allow space for copyright notice
;
;******************************************************************************
;
.CREF
.LIST
