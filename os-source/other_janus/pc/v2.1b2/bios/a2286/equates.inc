BRIDGEBOARD	EQU	1	;0 = pc40, 1 = A2286

; Global equate definitions:
;       
;	CMOS CLOCK CALENDAR EQUATES
;

SECONDS         EQU    	00H
MINUTES         EQU    	02H
HOURS           EQU    	04H
DAY             EQU    	06H
DATE            EQU    	07H
MONTH           EQU    	08H
year		equ	09h
SHUTDOWN        EQU    	0FH
CENTURY         EQU    	32H
SEC_ALARM       EQU    	01H
MIN_ALARM       EQU    	03H
HOUR_ALARM      EQU    	05H
REGISTER_A      EQU    	0AH
REGISTER_B      EQU    	0BH
REGISTER_C      EQU    	0CH
CMOS_INF_FLAG	EQU	33H		;CMOS information flag
CMOS_STATUS_1   EQU    	0EH             ;CMOS RAM STATUS
BATTERY         EQU    	0DH             ;BATTERY STATUS
CMOS_START      EQU    	10H             ;START OF CMOS RAM CONFIG TABLE
CMOS_SIZE       EQU    	1EH             ;SIZE OF CMOS RAM CONFIG TABLE
CHECKSUM_ADR    EQU    	2EH             ;CHECKSUM LOCATION
cmos_diskette_type	equ	10h  	;floppy diskette type byte
HDISK_TYPE      EQU    	12H             ;HARD DISK TYPE (0-15)
CMOS_EQUIP	EQU	14H
BASE_LOW_RAM    EQU    	15H             ;0-640K MEMORY SIZE
BASE_HIGH_RAM   EQU    	16H             ;0-640K MEMORY SIZE
CMOS_COMMODORE	EQU	20H		;Commodore config byte
high_ram	equ	30h		;high expansion memory byte
low_ram		equ	15h		;low expansion memory byte
CMOS_SP_FLAG	EQU	3FH		;PRESENT SPEED INDICATOR (BIT 0 & 1)
					;  MSB = 0 -- LOW SPEED HD IO
					;	 1 -- DON'T CHANGE SPEED IN HD IO

;

EPROM_ADDR_LOW  EQU    	0               ;EPROM OFFSET FOR CHECKSUM CALCULATION
EPROM_ADDR_HIGH EQU    	0E000H
EPROM_SIZE      EQU    	4096            ;SIZE OF EPROM IN WORDS
REFRESH_DIV     EQU    	18              ;REFRESH DIVISOR

;	I/O PORTS

CMOS            EQU    	70H             ;CLOCK CALENDAR
GAME_PORT       EQU    	0201H           ;GAME CARD
TIMER           EQU    	40H             ;TIMER PORT ADDR
DMA1            EQU    	0               ;DMA #1 PORT ADDR
DMA2            EQU    	0C0H            ;DMA #2 PORT ADDR
INT1            EQU    	20H             ;INTR #1 PORT ADDR
INT2            EQU    	0A0H            ;INTR #2 PORT ADDR
UPI             EQU    	60H             ;8042 (UPI)
upi_stat_port	equ	64h		;
rw_8042_A	equ	60h		;
rw_8042_B	equ	61h		;
NMI             EQU    	70H             ;NMI ENABLE, DISABLE
DMA_PAGE_REG    EQU    	80H             ;DMA PAGE REGISTER
parallel	equ    	378h		;parallel port
diag_port	equ    	378h		;diagnostics LEDs port

com1	equ	3f8h			;base address for com1
com2	equ	2f8h			;base address for com2

;	FLOPPY DISK EQUATES 

TIME_OUT        EQU    	80H             ;FDC FAILED TO RESPOND
SEEK_ERR        EQU    	40H             ;SEEK OPERATION ERROR
FDC_ERR         EQU    	20H             ;FDC FAILURE
CRC_ERR         EQU    	10H             ;CRC ERROR ON READING
DMA_BNDRY_ERR   EQU    	09H             ;ATTEMPT TO CROSS 64K BOUNDRY
DMA_OVERRUN     EQU    	08H             ;OVERRUN
REC_NOT_FOUND   EQU    	04H             ;REQUESTED SECTOR NOT FOUND
WR_PROT_ERR     EQU    	03H             ;WRITE ATTEMPTED ON WRITE PROT DISK
ADDR_MARK_ERR   EQU    	02H             ;ADDR MARK NOT FOUND ON TRACK
CMD_ERR         EQU    	01H             ;BAD COMMAND PASSED TO ROUTINE
INTR_FLAG       EQU    	80H
MOTOR_WAIT      EQU    	37              ;2 SECOND MOTOR ON TIME
FDC             EQU    	3F4H            ;FLOPPY DISK CONTROLLER (MAIN STATUS)
DOR             EQU    	3F2H            ;DIGITAL OUTPUT REG (FOR MOTOR CTRL)
fcr		equ	3f7h		;floppy control register
track_80	equ	1		;80 track capability
flp_type	equ	10h		;CMOS byte

;	DMA EQUATES

DMA_PAGE        EQU    	81H             ;DMA PAGE REGISTER

;

CR              EQU    	0DH             ;CARRIAGE RETURN
LF              EQU    	0AH             ;LINE FEED
BS              EQU    	08H             ;BACKSPACE
EOI             EQU    	20H             ;END OF INTERRUPT

;        
;	KEYBOARD EQUATES
;

READ_KBD_ID	EQU	0F2H		;REQUEST IDENTIFICATION INFO. FROM KEYBOARD
;        
;	VIDEO EQUATES
;

DISP_BFR_SEG    EQU    	0B000H          ;STARTING ADDR FOR MONOCHROME DISP
PARM_LENGTH     EQU    	16
COLOR_MONITOR   EQU    	3D0H            ;ADDRESS OF COLOR CARD
BW_EQU          EQU    	3B0H            ;ADDRESS OF BW CARD

;

DATA            EQU    	040H            ;DATA SEGMENT 

;define position of various registers on the stack when save_all
;routine is executed
;bp would point to the top of the stack

bp_ax	equ	16
bp_bx	equ	14
bp_cx	equ	12
bp_dx	equ	10
bp_ds	equ	8
bp_es	equ	6
bp_si	equ	4
bp_di	equ	2
bp_bp	equ	0

;
;	define some equates for floppy routine
;
floppy_dsdd	equ	93h	;250 kbs 320/360 media in 320/360 drive est.
floppy_hd	equ	15h	;500 kbs 1.2M media in 1.2M drive est.
Default		equ	61h	;300 kbs 360 media in 1.2M drive not est.
flp_dd_hd	equ	74h	;300 kbs 360 media in 1.2M drive est.
flp_dd_dd	equ	80h	;250kbs 360 media in 360 drive not est.
established	equ	10h	;mask for established bit
max_drv		equ	2	;2 maximum drives
double_rate	equ	20h	;mask for double rate bit in drive status


;
;	new eqautes defined by llj
;
pio	equ	61h
coprocessor_clr	equ	0f0h
coprocessor	equ	0f8h
cmos_equip	equ	14h

;	define stack segment

stack_seg	equ	30h		;rlm  01/09/85
top_of_stack	equ	100h		;rlm  01/18/85


;	define some equates for fixed disk

;	error codes returned to the user

bad_cmd_err	equ	1		;bad command passed to wd_drv
bad_addr_err	equ	2		;bad address mark error
bad_rec_err	equ	4		;specified record not found
reset_err	equ	5		;reset error
wd_init_err	equ	7		;initialization error
dma_bound_err	equ	9		;DMA boubdary error
bad_sec_err	equ	0Ah		;
bad_trk_err	equ	0Bh		;
ecc_err		equ	10h		;ECC error during read i/o
data_valid_err	equ	11h		;ECC corrected data error
wd_cntrl_err	equ	20h		;bad hard disk controller
wd_seek_err	equ	40h		;
wd_time_out	equ	80h		;


;	define bit positions for the Error register

err_reg_bit0	equ	1		;bit 0 = data address mark not found
err_reg_bit1	equ	2		;bit 1 = track 00 index not found
err_reg_bit2	equ	4		;bit 2 = command terminated
err_reg_bit4	equ	10h		;bit 4 = sector id not found
err_reg_bit6	equ	40h		;bit 6 = ECC data error
err_reg_bit7	equ	80h		;bit 7 = bad block detected

;	define some controller error codes

drv_not_rdy	equ	0aah		;drive not ready
cntrl_err	equ	0bbh		;undefined controller error
cntrl_wrt_flt	equ	0cch		;write fault error

;	define controller commands

wd_recal	equ	10h		;drive recaliberate
wd_read		equ	20h		;
wd_write	equ	30h		;
wd_verify	equ	40h		;
wd_fmt_trk	equ	50h		;
wd_init		equ	60h		;
wd_seek		equ	70h		;
wd_diag		equ	90h		;
wd_set_parms	equ	91h		;set drive parameters

retries		equ	1		;retries enable or disable
ecc_mode	equ	2		;ecc mode (long or short)
buffer_mode	equ	8		;
;op_wait	equ	20h		;wait for operation complete
op_wait		equ	25h		;wait for operation complete
rdy_wait	equ	3c00h		;wait for drive ready (increased
					;from 600h in v1.2)
drq_wait	equ	400h		;wait for data request line
					;(increased from 100h in v1.2)

hd_fail		equ	8		;
hd_pass		equ	0f7h		;

wd_port		equ	01F0h		;base port address
wd_stat_port	equ	wd_port+7	;status port
                                                                                                                            
