;******************************************************************************
;
; equ.inc contains equals, flag bits and register addresses 
;
;******************************************************************************
.xlist
;
;--- Assembly conditions ------------------------------------------------------
;
idle	       		=    	0	; handler active status
scroll	       		=    	0	; scroll active status
counters		=	1	;reentrancy tracking
kludge16		=	0	;use int16 "update" junk
;
;--- Flags --------------------------------------------------------------------
;
int_active     		=    	01h
Dos	       		=    	80h 	; DOS active
Janus	       		=    	40h 	; AMIGA active
IdleDOS        		=    	10h 	; DOS idle
AlreadyCalled  		=    	'T'	; Fake DOS 3.xx flag	
CalledFirst    		=    	'B'	; Fake DOS 3.xx flag	
SignalCustomer		=	55h	; Set SignalCustomer
;
;--- Partition Markers for different DOS versions ---------------------------
;
DOS12	       		=    	01h 	; 12-bit FAT, DOS 2.xx
DOS16	       		=    	04h 	; 16-bit FAT, DOS 3.xx		   
AMIGA	       		=    	0Ah 	; AMIGA Partition
; 
;--- Magic Control bits -----------------------------------------------------
;
irq3en	       		=    	not 08h	; enable IRQ3 in PIC
AmigaInt       		=    	04h 	; AMIGA interrupt # on PC (=IRQ3)
JanPCInt       		=    	40h	; disable interrupts from AMIGA
AckInt	       		=    	7fh	; Acknowledge pattern
;
;--- Misc: (for internal use) -----------------------------------------------
;
WAIT_FREE_LOOP		=	100	; loop count for waiting on a free
					; slot for service offset 
UpdateRate		=	3000	; Rate to update Int channel
;
clock			=	6Ch
;
JanInt8        		=    	8	; # of HW services
MaxInt	       		=    	32	; # of service channels
;
stacklength    		=    	4000h	; length of users stack 
;
EOI	       		=    	20h 	; End-of-interrupt pattern for PIC
;		      
D_seg	       		=    	0D000h	; Parameter segment for A2286	
F_seg	       		=    	0F000h	; Parameter segment for A2088
BD_seg			=	00040h	; BIOS data segment
;	
NULL			=	0
Scrolling      		=    	06h		 
BELL	       		=    	07h
BS	       		=    	08h
LF	       		=    	0Ah
CR	       		=    	0Dh        
WRITE_TTY     		=    	0Eh
ESC_char       		=    	1Bh
MemUnLock		=	7fh
MemLock			=	0FFh
StopScroll     		=    	0FFh	
;
;--- Int# -------------------------------------------------------------------
;
srv_int        		=    	0Bh 	; irq3 channel
video_int      		=    	10h	; video i/o	
disk_int       		=    	13h 	; disk i/o
serial_int     		=    	14h	; serial interrupt 
boot_int       		=    	19h	; boot interrupt
keyb_int       		=    	16h	; keyboard interrupt
time_int       		=    	1Ch 	; timer tick int
idle_int       		=    	28h	; DOS idle interrupt
;
;--- I/O addresses ------------------------------------------------------------
;
pic_00	       		=    	20h 	; PIC base address
pic_01	       		=    	21h 	; I/O address of interrupt enable reg.
;
;--- TrigInt register ------------------
; This is a read only register in normal PC !
; A write access to this register causes an interrupt on AMIGA side
; 
StatusReg      		=    062h	; System register I/O address (r/o)
;
;--- JanIntEn register -----------------
; This is a read only register in normal PC !
; We use this register to enable or disable interrupts 
; from AMIGA. Dependent on bit 6 of this register ALL interrupts from AMIGA
; side wil be enabled (=0) or disabled (=1). 
;
IntEn_reg      		=    0379h	; Printer status I/O address (r/o)
			 		; Bit6=0 JanInt enabled
					; Bit6=1 JanInt Disabled
;
;--- IRQ3Reset register ----------------
; This is a write only register in normal PC !
; A read access to this register resets the IRQ3 line coming from janus.
; We use this line to cause system interrupts from AMIGA on PC side.
;

;Irq3Reset      		=    03B0h	; Monochrome CRT index register (w/o)

Irq3Reset      		=    02f8h	; Monochrome CRT index register (w/o)
;
.list


