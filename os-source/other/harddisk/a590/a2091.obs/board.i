**********************************************************************
******************* DEFINITIONS FOR THE DMAC CHIP ********************
**********************************************************************
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
SDMA    EQU $e0		start DMA			(strobe)
SRST	EQU $e2		software reset (stop DMA)	(strobe)
CINT	EQU $e4		clear interrupts		(strobe)
FDMA    EQU $e8		strobe to flush DMA		(strobe)

; ISTR bits
 BITDEF DMA,FOLLOW,7	some interrupt pending (even if disabled)
 BITDEF DMA,PINT,6	peripheral interrupt pending (SCSI or XT)
 BITDEF DMA,EOP,5	end of process interrupt (terminal count)
 BITDEF DMA,PEND,4	some interrupt pending (only when enabled)
 BITDEF DMA,UNDER,3	DMA underrun interrupt (never happens, poof)
 BITDEF DMA,OVER,2	DMA overrun interrupt (never happens)
 BITDEF DMA,FIFOF,1	FIFO full  (1=TRUE)
 BITDEF DMA,FIFOE,0	FIFO empty (1=TRUE)

; CNTR bits
 BITDEF DMA,TCE,7	terminal count enable 		(active high)
 BITDEF DMA,PRESET,6	peripheral reset 		(active high)(strobe)
 BITDEF DMA,PMODE,5	peripheral mode 1=SCSI 0=XT
 BITDEF DMA,INTENA,4	interrupt enable		(active high)
 BITDEF DMA,DDIR,3	data direction 1=write 0=read
 BITDEF DMA,DMAMD,2	DMA mode 1=memory 0=peripheral
 BITDEF DMA,NASTY,1	same as blitter nasty in mem to mem mode
 BITDEF DMA,XTEOP,0	external EOP enable or XT unit number in XT mode

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
VERIFY			EQU $2f
READ_DEFECT_DATA	EQU $37
WRITE_BUFFER		EQU $3b
READ_BUFFER		EQU $3c

; Western Digital SCSI controller chip register addresses (off BoardAddress)
 IFD A590
SASR	EQU $91		SCSI auxiliary status register	(read)
;			SCSI register select (write)
SCMD	EQU $93		where all other registers appear (read/write)
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
