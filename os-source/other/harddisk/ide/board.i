**********************************************************************
******************* DEFINITIONS FOR THE DMAC CHIP ********************
**********************************************************************
	IFD	IS_SCSIDISK
DAWR	EQU $03		DACK width register		(byte write only)
WTCH	EQU $04		word transfer count 		(long read/write)
WTCL	EQU $06		actually never used
CNTR	EQU $0b		control register		(byte read/write)
SACH	EQU $0c		DMA address register
SDMA	EQU $13		start DMA			(byte strobe)
FDMA	EQU $17		flush DMA			(strobe)
CINT	EQU $1b		clear interrupts		(strobe)
ISTR	EQU $1f		interrupt status register
SRST	EQU $3f		halt DMA			(strobe)
	ENDC

	IFND	IS_IDE
	IFND	IS_SCSIDISK
ISTR	EQU $41		interrupt status		(read only)
CNTR	EQU $43		control				(read/write)
PBAR	EQU $48		peripheral base address		(read/write)
PDVD	EQU $4c		peripheral device disable	(write only)
WTCH	EQU $80		word transfer count hi		(read/write)
WTCL	EQU $82		word transfer count lo		(read/write)
SACH	EQU $84		source address counter hi	(write only)
SACL	EQU $86		source address counter lo	(write only)
DACH	EQU $88		destination address counter hi	(write only)
DACL	EQU $8a		destination address counter lo	(write only)
DAWR	EQU $8f		data acknowledge width		(read/write)
SDMA   EQU $e0		start DMA			(strobe)
SRST	EQU $e2		software reset (stop DMA)	(strobe)
CINT	EQU $e4		clear interrupts		(strobe)
FDMA   EQU $e8		strobe to flush DMA		(strobe)
	ENDC
	ENDC

; ISTR bits
 BITDEF DMA,ATINT,8	AT port interrupt
 BITDEF DMA,FOLLOW,7	some interrupt pending (even if disabled)
 BITDEF DMA,PINT,6	peripheral interrupt pending (SCSI or XT)
 BITDEF DMA,EOP,5	end of process interrupt (terminal count)
 BITDEF DMA,PEND,4	some interrupt pending (only when enabled)
 BITDEF DMA,UNDER,3	DMA underrun interrupt (never happens, poof)
 BITDEF DMA,OVER,2	DMA overrun interrupt (never happens)
 BITDEF DMA,FIFOF,1	FIFO full  (1=TRUE)
 BITDEF DMA,FIFOE,0	FIFO empty (1=TRUE)

; CNTR bits
	IFD	IS_SCSIDISK
 BITDEF DMA,TCE,5	terminal count enable 		(active high)
 BITDEF DMA,PRESET,4	peripheral reset 		(active high)(strobe)
 BITDEF DMA,PMODE,3	peripheral mode 1=SCSI 0=XT
 BITDEF DMA,INTENA,2	interrupt enable		(active high)
 BITDEF DMA,DDIR,1	data direction 1=write 0=read
 BITDEF DMA,IODX,0	leave low, some random thing
	ENDC

	IFND	IS_IDE
	IFND	IS_SCSIDISK
 BITDEF DMA,TCE,7	terminal count enable 		(active high)
 BITDEF DMA,PRESET,6	peripheral reset 		(active high)(strobe)
 BITDEF DMA,PMODE,5	peripheral mode 1=SCSI 0=XT
 BITDEF DMA,INTENA,4	interrupt enable		(active high)
 BITDEF DMA,DDIR,3	data direction 1=write 0=read
 BITDEF DMA,DMAMD,2	DMA mode 1=memory 0=peripheral
 BITDEF DMA,NASTY,1	same as blitter nasty in mem to mem mode
 BITDEF DMA,XTEOP,0	external EOP enable or XT unit number in XT mode
	ENDC
	ENDC

; various time values for the different drivers
; this is for 250ms (select timeout), (250 * Speed(Mhz))/80
; A3000 is 14.32Mhz (about 70ns), A590/A2091 is 7.16Mhz (about 140ns)
; a2090 = ??????
; for TRANSFER_CYCLE, the calculation is divisor/(2 * freq(in Mhz))
; so, 14Mhz is 105ns, 7Mhz is 140ns.
;
; MIN_SYNC_XFER is the minimum sync transfer time for the WD chip 
; (Tcyc-10 + Tcyc-25).  Absolute minimum is 200ns (assertion + negation +
; 2? cable delays.  However, we can't program it for less than 2*CLOCK_PERIOD
; for send, though we can accept it for input.

	IFD	CLOCK_14MHZ
TIMEOUT_VAL	EQU	44
CLOCK_PERIOD	EQU	105
MIN_SYNC_XFER	EQU	200
	ENDC

	IFD	CLOCK_7MHZ
TIMEOUT_VAL	EQU	23
CLOCK_PERIOD	EQU	140
MIN_SYNC_XFER	EQU	245
	ENDC

; The maximum offset allowed by the WD chip
; WD33C93
MAX_OFFSET		EQU	4
MIN_XFER_PERIOD		EQU	3

; WD33C93A
MAX_OFFSET_A		EQU	12
MIN_XFER_PERIOD_A	EQU	2

	IFD	CLOCK_PERIOD
MIN_MSG_PERIOD		EQU	((MIN_XFER_PERIOD*CLOCK_PERIOD)/4)
MIN_MSG_PERIOD_A	EQU	((MIN_XFER_PERIOD_A*CLOCK_PERIOD)/4)
	ENDC


; SCSI commands used by this driver (some may not be used at this time)
TEST_UNIT_READY		EQU $00
REZERO_UNIT		EQU $01
REQUEST_SENSE		EQU $03
FORMAT_UNIT		EQU $04
REASSIGN_BLOCKS		EQU $07
READ			EQU $08
WRITE			EQU $0a
SEEK			EQU $0b
INQUIRY			EQU $12
MODE_SELECT		EQU $15
RESERVE			EQU $16
RELEASE			EQU $17
MODE_SENSE		EQU $1a
START_STOP_UNIT		EQU $1b
SEND_DIAGNOSTIC		EQU $1d
READ_CAPACITY		EQU $25
READ_EXTENDED		EQU $28
WRITE_EXTENDED		EQU $2a
SEEK_EXTENDED		EQU $2b
VERIFY			EQU $2f
READ_DEFECT_DATA	EQU $37
WRITE_BUFFER		EQU $3b
READ_BUFFER		EQU $3c

; Values returned as errors
CHECK_CONDITION		EQU $02
BUSY			EQU $08

; Western Digital SCSI controller chip register addresses (off BoardAddress)
 IFND IS_SCSIDISK
 IFND IS_IDE
 IFND IS_A2090
; A590/A2091
SASR	EQU $91		SCSI auxiliary status register	(read)
;			SCSI register select (write)
SCMD	EQU $93		where all other registers appear (read/write)
 ENDC
 ENDC
 ENDC

 IFD IS_SCSIDISK
; a3000
SASR	EQU $49		SCSI auxiliary status register	(read as a byte)

	IFD	LONGWORD_SASR
SASRW	EQU $48		SCSI register select (write as a longword)
	ENDC

SCMD	EQU $43		where all other registers appear (read/write as a byte)
 ENDC

 IFD A2090
SASR	EQU $61		SCSI auxiliary status register	(read)
;			SCSI register select (write)
SCMD	EQU $63		where all other registers appear (read/write)
 ENDC

; Western Digital SCSI controller chip internal register addresses (absolute)
OWN_ID		EQU	$00		our SCSI buss address (usually 7)
CONTROL		EQU	$01		general DMA and parity controls
TIMEOUT		EQU	$02		timeout period register for select
TOTAL_SECTORS	EQU	$03		used for translate address
TOTAL_HEADS	EQU	$04
TOTAL_CYLS_MSB	EQU	$05
TOTAL_CYLS_LSB	EQU	$06
LOGICAL_ADDR_3	EQU	$07		used for translate address
LOGICAL_ADDR_2	EQU	$08
LOGICAL_ADDR_1	EQU	$09
LOGICAL_ADDR_0	EQU	$0a
SECTOR_NUM	EQU	$0b		used for translate address
HEAD_NUM	EQU	$0c
CYL_NUM_MSB	EQU	$0d
CYL_NUM_LSB	EQU	$0e
TARGET_LUN	EQU	$0f		for identify during select and transfer
COMMAND_PHASE	EQU	$10		select and transfer phase codes
SYNC_TRANSFER	EQU	$11		synchronous transfer timings
TRANSFER_MSB	EQU	$12		transfer length
TRANSFER_MB	EQU	$13
TRANSFER_LSB	EQU	$14
DEST_ID		EQU	$15		SCSI address of target controller
SOURCE_ID	EQU	$16		SCSI address of re-selecting device
SCSI_STATUS	EQU	$17		general status register
COMMAND		EQU	$18		command start register
DATA_REG	EQU	$19		data register for SBT mode

; auxiliary status register bit definitions
 BITDEF WDC,INT,7	an interrupt is pending
 BITDEF WDC,LCI,6	last command ignored
 BITDEF WDC,BSY,5	chip is busy with a level 2 command
 BITDEF WDC,CIP,4	command in progress
 BITDEF WDC,PE,1	a parity error was detected
 BITDEF WDC,DBR,0	data buffer ready during programmed I/O

; control register bit definitions
 BITDEF WDC,DMA,7	DMA mode is enabled for data transfers
 BITDEF WDC,WDB,6	direct buffer access for data transfers
 BITDEF WDC,HA,1	halt on attention (target mode only)
 BITDEF WDC,HPE,0	halt on parity error enable

; source ID register control bits
 BITDEF WDC,ER,7	enable reselection
 BITDEF WDC,ES,6	enable selection (target or multiple initiators only)
 BITDEF WDC,SIV,3	source ID valid

; command register control bits
 BITDEF WDC,SBT,7	enable single byte transfer mode

; own id register bits
 BITDEF WDC,EAF,3	enable advanced features on reset
 BITDEF WDC,EHP,4	enable host parity on bus

; Western digital command codes that are jammed into the command register
wd_RESET		EQU	$00	reset the WDC chip
wd_ABORT		EQU	$01	abort last command
wd_ASSERT_ATN		EQU	$02	we want to send a message
wd_NEGATE_ACK		EQU	$03	we accepted the last message
wd_DISCONNECT		EQU	$04
wd_RESELECT		EQU	$05
wd_SELECT_WITH_ATN	EQU	$06	select with ATN
wd_SELECT		EQU	$07	select without ATN
wd_SELECT_ATN_TRANS	EQU	$08
wd_SELECT_TRANS		EQU	$09
wd_RESEL_AND_RECEIVE	EQU	$0a
wd_RESEL_AND_SEND	EQU	$0b
wd_WAIT_SEL_AND_RECEIVE	EQU	$0c
wd_RECEIVE_COMMAND	EQU	$10
wd_RECEIVE_DATA		EQU	$11
wd_RECEIVE_MSG_OUT	EQU	$12
wd_RECEIVE_INFO_OUT	EQU	$13
wd_SEND_STATUS		EQU	$14
wd_SEND_DATA		EQU	$15
wd_SEND_MSG_IN		EQU	$16
wd_SEND_INFO_IN		EQU	$17
wd_TRANSLATE_ADDRESS	EQU	$18	convert logical offset to head,cyl etc.
wd_TRANSFER_INFO	EQU	$20	do a transfer (MCI says what)
wd_TRANSFER_PAD		EQU	$21	transfer all the same bytes

; Western digital XT controller register addresses for XT unit zero.  Add an
; offset of 20hex to access the second set of XT registers (for XT unit 1)
 IFND REAL_OLD
XTPORT0		EQU	$a1			use byte addressing
XTPORT1		EQU	$a3			switch PORT1 and PORT2...
XTPORT2		EQU	$a5			...for very old boards
XTPORT3		EQU	$a7
 ENDC

 IFD REAL_OLD
XTPORT0		EQU	$a1			use byte addressing
XTPORT2		EQU	$a3			switch PORT1 and PORT2...
XTPORT1		EQU	$a5			...for very old boards
XTPORT3		EQU	$a7
 ENDC

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; IDE hardware definitions
;
	IFD	IS_IDE

; Slow uses PC/AT timings for compatibility, fast goes at bus speed
IDE_Slow	EQU	$da0000
IDE_Fast	EQU	$da2000

; A300/A1000+/A500++ ide register mapping
; offsets from $da0000.  All regs 8 bit unless noted.
AT_Data		EQU	$00		; 16 bit wide port
AT_Error	EQU	$04		; error register
AT_SectorCnt	EQU	$08		; # of sectors, 0 = 256
AT_SectorNum	EQU	$0c		; sector to start (1-based!)
AT_CylLow	EQU	$10		; low byte of Cylinder number
AT_CylHigh	EQU	$14		; high byte
AT_DriveHead	EQU	$18		; Drive and head number | $A0
AT_Status	EQU	$1c		; (read) status reg, clears interrupt
AT_Command	EQU	$1c		; (write) command value, starts it
AT_AltStatus	EQU	$1018		; (read) alt status, doesn't clear int
AT_DeviceCtrl	EQU	$1018		; (write) device control - int/reset
AT_DriveAddress	EQU	$101c		; not used (pc-ish)
AT_IntStatus	EQU	$1000		; interrupt status bit, active low

; Gayle address and offsets
GAYLE_ADDR	EQU	$da8000
GAYLE_ID_ADDR	EQU	$de1000
GAYLE_VERSION	EQU	%00001101	; version 1101 (first)
GAYLE_IntStatus	EQU	0
GAYLE_IntChange	EQU	$1000
GAYLE_IntEnable	EQU	$2000
GAYLE_Control	EQU	$3000

; address of CDTV-CR hardware
CDTV_ADDR	EQU	$a40000

; CDTV IDE status bit
	BITDEF	CDTV,IDE,0

; IDE commands
AT_RECALIBRATE		EQU	$10
AT_READ_SECTORS		EQU	$20
AT_WRITE_SECTORS	EQU	$30
AT_READ_VERIFY_SECTORS	EQU	$40
AT_FORMAT		EQU	$50
AT_SEEK			EQU	$70
AT_DIAGNOSTICS		EQU	$90
AT_INIT_PARAMETERS	EQU	$91	; actually Initialize Drive Parameters
AT_READ_MULTIPLE	EQU	$c4
AT_WRITE_MULTIPLE	EQU	$c5
AT_SET_MULTIPLE_MODE	EQU	$c6
AT_IDENTIFY_DRIVE	EQU	$EC

; Status bits
	BITDEF	AT,ERR,0
	BITDEF	AT,IDX,1
	BITDEF	AT,DRQ,3
	BITDEF	AT,RDY,6
	BITDEF	AT,BSY,7

; Drive/Head bit - set to access drive 1
	BITDEF	DH,SLAVE,4

; enable interrupts bit - device control register
	BITDEF	DC,INT_ENABLE,1

; Error bits
	BITDEF	ERR,AMNF,0
	BITDEF	ERR,TK0NF,1
	BITDEF	ERR,ABRT,2
	BITDEF	ERR,IDNF,4
	BITDEF	ERR,UNC,6
	BITDEF	ERR,BBK,7

; errors I added
	BITDEF	ERR,ADDR,8	; LBA out of range
	BITDEF	ERR,NOT_READY,9	; LUN doesn't respond
	BITDEF	ERR,SELFTEST,10	; self-test failure

	ENDC
