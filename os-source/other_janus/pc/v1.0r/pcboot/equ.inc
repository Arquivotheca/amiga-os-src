;******************************************************************************
;
; equ.inc contains equals, flag bits and register addresses 
;
;******************************************************************************
.xlist
;--- Assembly conditions ------------------------------------------------------
;
infolevel      =    48			; message output level 
video	       =    1			; enable video output
serial	       =    0			; enable serial output
;    
idle	       =    0			; handler active status
;		    
;--- Flags --------------------------------------------------------------------
;
int_active     =    01h
Dos	       =    80h 		; DOS active
Janus	       =    40h 		; AMIGA active
AlreadyCalled  =    'T'			; Fake DOS 3.xx flag	
DOS12	       =    01h 		; DOS PARTITION MARKERS
DOS16	       =    04h 				   
AMIGA	       =    0Ah 		; AMIGA PART. MARKERS (COULD BE MORE)

; 
;--- Contol bits --------------------------------------------------------------
;
irq3en	       =    not 08h		; enable IRQ3 in PIC
AmigaInt       =    04h 		; AMIGA interrupt # on PC (=IRQ3)
JanPCInt       =    040h		; disable interrupts from AMIGA
AckInt	       =    07fh		; Acknowledge pattern
;		      
JanInt8        =    8			; first SW interrupt
MaxInt	       =    16			; last SW interrupt
stacklength    =    4000h		; length of users stack 
EOI	       =    20h 		; End-of-interrupt pattern for PIC
;		      

CR	       =    0Dh        
LF	       =    0Ah
BS	       =    08h
ESC	       =    1Bh
BELL	       =    07h
WRITE_TTY      =    0Eh
;
MaxPart        =    04h 		; partitions on harddisk
sectorsize     =    200h		; 512 BYTE/SECTOR
;
;--- Int# ------------------------------
;
hd_int	       =    13h 		; disk i/o
srv_int        =    0Bh 		; irq3 channel
DISK	       =    13h			; disk interrupt
COM            =    14h			; serial interrupt 
BOOT	       =    19h
idle_int       =    28h

;




;
;--- I/O addresses ------------------------------------------------------------
;
pic_00	       =    20h 		; PIC base address
pic_01	       =    21h 		; I/O address of interrupt enable reg.
;
;--- TrigInt register ------------------
; This is a read only register in normal PC !
; A write access to this register causes an interrupt on AMIGA side
; 
StatusReg      =    062h		; System register I/O address (r/o)
;
;--- JanIntEn register -----------------
; This is a read only register in normal PC !
; We use this register to enable or disable interrupts 
; from AMIGA. Dependent on bit 6 of this register ALL interrupts from AMIGA
; side wil be enabled (=0) or disabled (=1). 
;
IntEn_reg      =    0379h		; Printer status I/O address (r/o)
					; Bit6=0 JanInt enabled
					; Bit6=1 JanInt Disabled
;
;--- IRQ3Reset register ----------------
; This is a write only register in normal PC !
; A read access to this register resets the IRQ3 line coming from janus.
; We use this line to cause system interrupts from AMIGA on PC side.
;
Irq3Reset      =    03B0h		; Monochrome CRT index register (w/o)
;

;
;--- JANUS PC Function calls -----------
;
; Known function codes:
;
J_GET_SERVICE	=	0
J_GET_BASE	=	1
J_ALLOC_MEM	=	2
J_FREE_MEM	=	3
J_SET_PARAM	=	4
J_SET_SERVICE	=	5
J_STOP_SERVICE	=	6
J_CALL_AMIGA	=	7
J_WAIT_AMIGA	=	8 
J_CHECK_AMIGA	=	9
;
; Status returns:
;
J_NO_SERVICE	=	0ffh		; no service available
J_PENDING	=	0
J_FINISHED	=	1
J_OK		=	0		; general good return
J_NO_MEMORY	=	3		; requested memory not available
J_ILL_FNCTN	=	4		; Illegal function call
;
 
.list


