;******************************************************************************
;
; equ.inc contains equals, flag bits and register addresses 
;
;******************************************************************************
.xlist
;--- Assembly conditions ------------------------------------------------------
;
infolevel      		=    	10	; message output level 
video	       		=    	1	; enable video output
serial	       		=    	0	; enable serial output
;    
idle	       		=    	0	; handler active status
scroll	       		=    	1	; scroll active status
;
;--- Message Flags ------------------------------------------------------------
;
; Load_irq:
DBG_LoadMsg		=	1	; Handler loaded at %x		    
DBG_BaseMsg		=	0	; Janus Base found at %x
DBG_NoBaseMsg		=	1	; No Janus Base warning
DBG_NoServiceBaseMsg 	=	1	; No Service Base warning
DBG_BufferMsg		=	0	; Buffer mem found at %x
DBG_IntsMsg		=	0	; Int table offset = %x
DBG_ParasMsg		=	0	; Int Paras offset = %x
DBG_JPartBase		=	0	; Janus Partition Info
;
; Irq3Serv:
DBG_HWMsgP		=	0	; Pending HW interrupt								     
DBG_HWMsgR  		=	0	; HW interrupt ready
DBG_HWMsgPC 		=	0	; PC request for janus
DBG_IntCtrlMes		=	0	; Status of interrupt controller
DBG_DosMsgA 		=	0	; Pending INT13
DBG_DosMsgN 		=	0	; INT13	ready
DBG_IdleMsg 		=	0	; DOS idle status
;
; JI_Han:
DBG_IntTblMes		=	0	; interrupt table
DBG_FoundIntMes		=	0	; interrupt number
DBG_NoIntMes		=	0	; no interrupt warning
DBG_ExeIntMes		=	0	; execute interrupt							  
;
; HD I/O:
DBG_HDPart		=	0	; partition messages
DBG_regmsg   		=	0	; show diskIO regs
DBG_diskmsg1		=	0	; access HD
DBG_BlockMove		=	0	; copy parameter
;
; Scroll:
DBG_Scroll 		=	0	; scroll diagnostic
DBG_UpdateMsg		=	0	; PIC update warning
;
;--- Flags --------------------------------------------------------------------
;
int_active     		=    	01h
Dos	       		=    	80h 	; DOS active
Janus	       		=    	40h 	; AMIGA active
IdleDOS        		=    	10h 	; DOS idle
AlreadyCalled  		=    	'T'	; Fake DOS 3.xx flag	
CalledFirst    		=    	'B'	; Fake DOS 3.xx flag	
DOS12	       		=    	01h 	; DOS PARTITION MARKERS
DOS16	       		=    	04h 			   
AMIGA	       		=    	0Ah 	; AMIGA PART. MARKERS (COULD BE MORE)

; 
;--- Contol bits --------------------------------------------------------------
;
irq3en	       		=    	not 08h	; enable IRQ3 in PIC
AmigaInt       		=    	04h 	; AMIGA interrupt # on PC (=IRQ3)
JanPCInt       		=    	40h	; disable interrupts from AMIGA
AckInt	       		=    	7fh	; Acknowledge pattern
;		      
JanInt8        		=    	8	; first SW interrupt
MaxInt	       		=    	32	; last SW interrupt
;
stacklength    		=    	4000h	; length of users stack 
EOI	       		=    	20h 	; End-of-interrupt pattern for PIC
;		      
D_seg	       		=    	0D000h	; Parameter segment for A2286	
F_seg	       		=    	0F000h	; Parameter segment for A2088
;	
CR	       		=    	0Dh        
LF	       		=    	0Ah
BS	       		=    	08h
ESC_char       		=    	1Bh
BELL	       		=    	07h
WRITE_TTY     		=    	0Eh
Scrolling      		=    	06h		 
StopScroll     		=    	0FFh	
;
MaxPart        		=    	04	; partitions on harddisk
sectorsize     		=    	200h	; 512 BYTE/SECTOR
;
;--- Int# ------------------------------
;
hd_int	       		=    	13h 	; disk i/o
video_int      		=    	10h	; video i/o	
srv_int        		=    	0Bh 	; irq3 channel
time_int       		=    	1ch 	; timer tick int
DISK	       		=    	13h	; disk interrupt
COM            		=    	14h	; serial interrupt 
BOOT	       		=    	19h	; boot interrupt
keyb_int       		=    	16h	; Keyboard interrupt
idle_int       		=    	28h	; DOS idle interrupt
;




;
;--- I/O addresses ------------------------------------------------------------
;
pic_00	       		=    20h 	; PIC base address
pic_01	       		=    21h 	; I/O address of interrupt enable reg.
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
Irq3Reset      		=    03B0h	; Monochrome CRT index register (w/o)
;
.list


