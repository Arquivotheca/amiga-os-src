include	title.inc
subttl	Data Segment Definitions

.xlist
public	dsk_parm_ptr
public	fdc_results
public	motor_timout
public	recal_reqd
public	hf_cntrl
public	drv_0_media
public	drv_1_media
public	drv_0_cyl
public	drv_1_cyl
public	data_rate
public	dsk_opn_status
public	motor_on
public	drv_0_op
public	drv_1_op
public	wd_cmd_file
public	port_off
public	wd1_parm_vec
public	wd0_parm_vec
public	disk_stat_1
public	hf_err_reg
public	ctrl_byte
public	hf_int_flg
public	hf_num
public	hf_st_reg
public	vir_mem_size
public	out_buffer
public	flag
public	flag_1
public	flag_2
public	flag_3
public	reset_key
public	serial_base
public	timer_upper
public	key_buffer_start
public	key_buffer_end
public	timer_lower
public	in_buffer
public	equip_flag
public	special
public	vflag
public	serial_time
public	rom_address
public	rom_segment
public	memory_size
public	alt_buffer
public	in_buffer
public	key_buffer
public	video_mode
public	disp_cols
public	dbfr_size
public	dbfr_start
public	cursor_position
public	cursor_mode
public	current_page
public	crtc_base_adr
public	video_mode_reg
public	crt_color
public	break_key
public	reset_key
public	print_time
public	serial_time
public	event_flag_addr
public	event_flag_seg
public	interrupt_flag
public	count_low
public	count_high
public	disp_bfr_w
public	parallel_table
public	parm_ptr
public	count_high
public	event_wait_flag
public	print_stat
public	timer_wrap
public	disp_bfr_b
public	int_ptr2
public	int_ptr1
public	user_cgen_table
public	net_data
public	ega_parms
public	ega_rows
public	ega_char_size
.list

;
;       SEGMENT DEFINITIONS
;

ZERO    SEGMENT AT 0                   


;	define low memory interrupt vectors


	org	0

intr_vec_0	dd	0		;interrupt vector 0

intr_vec_1	dd	0		;interrupt vector 1

intr_vec_2	dd	0		;interrupt vector 2

intr_vec_3	dd	0		;interrupt vector 3

intr_vec_4	dd	0		;interrupt vector 4

intr_vec_5	dd	0		;interrupt vector 5

intr_vec_6	dd	0

intr_vec_7	dd	0

intr_vec_8	dd	0

intr_vec_9	dd	0

intr_vec_A	dd	0

intr_vec_B	dd	0

intr_vec_C	dd	0

intr_vec_D	dd	0

intr_vec_E	dd	0

intr_vec_F	dd	0

intr_vec_10	dd	0

intr_vec_11	dd	0

intr_vec_12	dd	0

intr_vec_13	dd	0

intr_vec_14	dd	0

intr_vec_15	dd	0

intr_vec_16	dd	0

intr_vec_17	dd	0

intr_vec_18	dd	0

intr_vec_19	dd	0

intr_vec_1A	dd	0

intr_vec_1B	dd	0

Intr_vec_1C	dd	0

intr_vec_1D	dd	0

intr_vec_1E	dd	0

intr_vec_1F	dd	0


	ORG 20H         

INT_PTR1        LABEL DWORD            ;START OF #1 HARDWARE INTERRUPTS         


	org	40h*4

intr_vec_40	dd	0		;floppy disk driver

wd0_parm_vec	dd	0		;address of Hdisk 1 parameters

	org	46h*4

wd1_parm_vec	dd	0		;address of Hdisk 2 parameters


;
	ORG 1C0H
INT_PTR2        LABEL DWORD            ;START OF #2 HARDWARE INTERRUPTS

;
	ORG 01DH*4
PARM_PTR        LABEL DWORD            ;POINTER TO VIDEO PARM TABLE
        
	ORG 01EH*4
DSK_PARM_PTR    LABEL DWORD            ;POINTER TO DISK PARAMETER TABLE
	dw	0
	dw	0			;
	ORG     01FH*4
USER_CGEN_TABLE LABEL DWORD            ;POINTER TO USER SUPPLIED CHAR GEN TABLE

ZERO    ENDS



;

STACK   SEGMENT at	30h		;stack segment

	dw	128  dup(0)

STACK   ENDS

;                                     
;       DATA AREA         
;
DATAA   SEGMENT AT 40H

;	EQUIPMENT TABLES
	
		ORG 0

SERIAL_BASE     label	word
		DW    0,0,0,0		;TABLE OF INSTALLED SERIAL PORTS

PARALLEL_TABLE  label	word
		DW    0,0,0,0		;TABLE OF INSTALLED PARALLEL PORTS

EQUIP_FLAG      DW	0		;EQUIPMENT FLAG
special		db	0		;autoconfig flag
vflag		db	0		;temporary video flag used by
					;video Autoconfig(tm)
		ORG 013H
MEMORY_SIZE     DW    	0	        ;MEMORY SIZE IN K BYTES
vir_mem_size	dw	0		;store size of memory above 1 meg
					; rlm   01/08/85

;	KEYBOARD DATA AREA

;		ORG 017H

FLAG            DB    0                ;SHIFT STATE
FLAG_1          DB    0                ;SHIFT STATE

;		ORG 019H

ALT_BUFFER      DB    0                ;ALTERNATE KEYPAD BUFFER
OUT_BUFFER      DW    0                ;KEYBOARD OUT BUFFER POINTER
IN_BUFFER       DW    0                ;KEYBOARD IN BUFFER POINTER

KEY_BUFFER      label	word
		DW    16 DUP (0)       ;KEYBOARD BUFFER

;	DISK DATA AREA 

;		ORG 03EH

RECAL_REQD      DB    0                ;RECALIBRATION STATUS(0-3)
MOTOR_ON        DB    0                ;MOTOR ON STATUS
MOTOR_TIMOUT    DB    0                ;TIMEOUT FOR TURNING MOTOR OFF
DSK_OPN_STATUS  DB    0                ;STATUS OF THE DISKETTE OPERATION
wd_cmd_file	label	byte		;command file for hard disk
FDC_RESULTS     DB    7 DUP (0)        ;STATUS BYTES FROM FDC STORED HERE

;	VIDEO DATA AREA

;		ORG 049H

VIDEO_MODE      DB    0                ;CURRENT VIDEO MODE
DISP_COLS       DW    0                ;NUMBER OF DISPLAY COLUMNS ON THE SCREEN
DBFR_SIZE       DW    0                ;SIZE OF DISPLAY BUFFER (IN BYTES)
DBFR_START      DW    0                ;STARTING ADDRESS IN DISPLAY BUFFER

CURSOR_POSITION label	word
		DW    8 DUP(0)         ;CURSOR POSITION FOR EACH PAGE (8 MAX)

CURSOR_MODE     DW    0                ;CURRENT CURSOR MODE
CURRENT_PAGE    DB    0                ;CURRENT DISPLAY PAGE  
CRTC_BASE_ADR   DW    0                ;BASE ADDRESS OF VIDEO CARD           
VIDEO_MODE_REG  DB    0                ;CURRENT MODE REG SETTING         
CRT_COLOR       DB    0                ;CURRENT COLOR SETTING

;	Data areas used by diagnostics routines

;		ORG 67H

ROM_ADDRESS     DW    0			;pointer to init routine
ROM_SEGMENT     DW    0			;pointer to IO ROM segment
INTERRUPT_FLAG  DB    0			;flag indicating occurance of an 
					;interrupt
;	TIMER DATA AREA

;		ORG 06CH

TIMER_LOWER     DW    0			;low word of timer counter
TIMER_UPPER     DW    0			;high word of timer counter
TIMER_WRAP      DB    0			;timer has overflowed since last 
					;setting
;	BREAK FLAGS
	
;		ORG 071H

BREAK_KEY       DB    0			;bit 7 = 1 means break key has been hit
RESET_KEY       DW    0			;=1234h if ctrl/alt/del keys hit

;	Hard file data area
	
;		org 74h

disk_stat_1	db	0
hf_num		db	0
ctrl_byte	db	0
port_off	db	0

;	TIMEOUTS

;		ORG 078H

PRINT_TIME      label	byte
		DB     0,0,0,0		;timeout in secs for four printers

SERIAL_TIME     label	byte
		DB     0,0,0,0		;timeout in secs for four serial ports

;	Keyboard buffer addresses

;		org 80h

key_buffer_start	dw	0
key_buffer_end		dw	0


;	define new parameters for EGA card
ega_rows	db	0		;rows on current screen-1
ega_char_size	dw	0		;bytes per character
		db	0,0,0,0		;other parameters


;	Additional floppy data area
	
;		org 	8bh

data_rate	db	0		;last data rate selected for floppy
hf_st_reg	db	0		;status register
hf_err_reg	db	0		;error register
hf_int_flg	db	0		;interrupt flag
hf_cntrl	db	0		;special floppy card bit 0=1

;	Additional floppy diskette data areas

;		org 	90h

drv_0_media	db	0		;state of media on drive 0
drv_1_media	db	0		;state of media on drive 1
drv_0_op	db	0		;drive 0 operation start state
drv_1_op	db	0		;drive 1 operation start state
drv_0_cyl	db	0		;drive 0 present cylinder
drv_1_cyl	db	0		;drive 1 present cylinder

;	Keyboard led flag

;		org	96h		
flag_3		db	0		;keyboard mode states
flag_2		db	0		;keyboard LEDs states

;	PERIODIC INTERRUPT

;		ORG 98H

event_flag_addr	DW    	0		;
event_flag_seg	dw	0		;
COUNT_LOW       DW    	0		;
COUNT_HIGH      DW    	0		;
event_wait_flag	DB    	0		;

;		org	0a1h

;	define reserved area for networks

net_data	db	7 dup(0)

ega_parms	dd	0		;pointer to EGA parameters


;	PRINTER DATA AREA

		ORG 100H

PRINT_STAT      DB    0			;printer status
DATAA 	ENDS

;
;	VIDEO DISPLAY BUFFER AREA
;

DISP_BFR        SEGMENT AT 0B800H
DISP_BFR_B      LABEL BYTE
DISP_BFR_W      LABEL WORD
DB              16384 DUP (0)
DISP_BFR        ENDS

	end

