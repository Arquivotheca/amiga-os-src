
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
* 
* $Header: internal.i
*
*************************************************************************

		INCLUDE 'libraries/configvars.i'

AUTOINST	SET	1	; Defined if to be used with autoconfig software
HANDSHAKE	SET	1	; Defined if using KONAN code without default
AUTOCONF	SET	1	; Defined if for autoconfig version of board
*				  command block address
HASSCSI		SET	1	; Defined if board has SCSI chip
BufferIO	SET	1	; Defined if IO is to be buffered
		IFD	BufferIO
*BufWrites	SET	1	; Writes will be buffered if defined
		ENDC
NBAD		EQU	126	; Number of bad blocks supported
NUM_BUFS	EQU	1	; Number of buffers in the system (was 6)
BUF_SECTS	EQU	17	; Number of sectors per buffer
EN_SIZE		EQU	50*4	; Environment Size

AN_AMHDDiskDev	equ $33000000	; Alert #
CBM_MANF	EQU	514	; Commodore West Chester Manufacturer #
PROD_NUM	EQU	1	; Hard disk controller product #


*	WD SCSI Chip directly addressable registers

SCSIAUX		EQU	$60	; SCSI Chip auxiliary register - READ
SCSIADDR	EQU	$60	; SCSI Chip auxiliary register - WRITE
SCSIIND		EQU	$62	; SCSI Chip indirect data register - R/W

*	WD SCSI Chip indirectly addressable registera

SC_OWN_ID	EQU	$00	; WD Own ID register address
SC_CONTROL	EQU	$01	; WD Control register address
SC_TIMEOUT_P	EQU	$02	; WD Timeout Period register address
SC_CDB1		EQU	$03	; WD Command Descriptor Block byte 1
SC_CDB2		EQU	$04	; WD Command Descriptor Block byte 2
SC_CDB3		EQU	$05	; WD Command Descriptor Block byte 3
SC_CDB4		EQU	$06	; WD Command Descriptor Block byte 4
SC_CDB5		EQU	$07	; WD Command Descriptor Block byte 5
SC_CDB6		EQU	$08	; WD Command Descriptor Block byte 6
SC_CDB7		EQU	$09	; WD Command Descriptor Block byte 7
SC_CDB8		EQU	$0A	; WD Command Descriptor Block byte 8
SC_CDB9		EQU	$0B	; WD Command Descriptor Block byte 9
SC_CDB10	EQU	$0C	; WD Command Descriptor Block byte 10
SC_CDB11	EQU	$0D	; WD Command Descriptor Block byte 11
SC_CDB12	EQU	$0E	; WD Command Descriptor Block byte 12
SC_TARGET_LUN	EQU	$0F	; WD Target LUN register address
SC_CMD_PHASE	EQU	$10	; WD Command Phase register address
SC_SYNC_T	EQU	$11	; WD Synchronous Transfer register address
SC_TC0		EQU	$12	; WD Transfer Count HIGH byte register address
SC_TC1		EQU	$13	; WD Transfer Count MID byte register address
SC_TC2		EQU	$14	; WD Transfer Count LOW byte register address
SC_DEST_ID	EQU	$15	; WD Destination ID register address
SC_SRC_ID	EQU	$16	; WD Source ID register address
SC_STATUS	EQU	$17	; WD STATUS register address
SC_COMMAND	EQU	$18	; WD COMMAND register address
SC_DATA		EQU	$19	; WD DATA register address

*	WD SCSI Chip COMMANDS

RESET_CMD	EQU	$00
ABORT_CMD	EQU	$01
NEG_ACK_CMD	EQU	$03	; Negate ACK
DISC_CMD	EQU	$04	; Disconnect
SEL_WO_ATN_CMD	EQU	$07	; Select without ATN
ST_WITH_ATN_CMD	EQU	$08	; Select and transfer WITH ATN
ST_WO_ATN_CMD	EQU	$09	; Select and transfer without ATN
TRANS_INFO_CMD	EQU	$20	; Transfer info command
TRANS_PAD_CMD	EQU	$21	; Transfer PAD command
 
*	WD SCSI Chip STATUS

SEL_TRN_OK	EQU	$16	; Select-and-transfer completed successfully
TGT_PARITY_ERR	EQU	0	; Bit to test for parity error

*	SCSI Chip Init constants

SCSI_EN_DMA	EQU	$80	; Enable DMA, no interupt on parity
SCSI_NO_DMA	EQU	$00	; Disable DMA, no interupt on parity
SCSI_TIMEOUT	EQU	$FF	; SCSI Timeout period -> 2.04 seconds

*	DMA Chip addresses

PCSS		EQU	$64	; DMA chip PCSS - STATUS
PCSD		EQU	$68	; DMA chip PCSD - DATA

*	DMA Chip Commands

LD2		EQU	$FB	; Load high order address, and direction bit
LD1		EQU	$FD	; Load middle address
LD0512		EQU	$FE	; Load low order address, 512 byte mode
LD0		EQU	$F6	; Load low order address, unlimted mode
START_DMA	EQU	$F7	; Start DMA transfer
STOP_DMA	EQU	$FF	; Stop unlimited DMA transfer
SOFT_RESET	EQU	$7F	; Software reset
END_RESET	EQU	$FF	; Issue after SOFT_RESET
DMA_STATUS	EQU	$EF	; Get DMA chip status
DMA_ERROR_BIT	EQU	5	; Bit LOW if FIFO overrun/underrun error

HDSCSIO		EQU	$30	; Offset of nibble to test for SCSI ONLY bit
HDSCSIB		EQU	7	; BIT to test for SCSI ONLY bit
HDCS		EQU	$52
HDSTAT		EQU	$40
HDSTAT1		EQU	$42
PCNFGOUT	EQU	$48
HDINTACK	EQU	$50
ErrorMonitor	EQU	0
ERRORMSGS	EQU	0
INFO_LEVEL	SET	0

	IFND	LIBRARIES_DOS_I
	INCLUDE 'libraries/dos.i'
	ENDC

*--------------------------------------------------------------------
*
* TYPE Macros
*
*--------------------------------------------------------------------

BLOCKNO     MACRO
\1          EQU     SOFFSET
SOFFSET     SET     SOFFSET+4
            ENDM

*--------------------------------------------------------------------
*
* Defines Boot sector, controller disk parameters, bad block tables
*
*--------------------------------------------------------------------

HD_STKSIZE	EQU	$A00	; Stack size for disk task
HD_BD_BASE	EQU	$E80000	; Base address of controller board
HD_INIT_CMD	EQU	$060000	; Default command block address

*	Controller command codes

HDC_TDR		EQU	$00	; test drive ready
HDC_REST	EQU	$01	; restore to cyl 0
HDC_RSS		EQU	$03	; Request status
HDC_FMT		EQU	$04	; format drive
HDC_CTF		EQU	$05	; check track format
HDC_FMTT	EQU	$06	; format track
HDC_READ	EQU	$08	; Read
HDC_WRITE	EQU	$0A	; Write
HDC_SEEK	EQU	$0B	; seek
HDC_SDP		EQU	$0C	; set drive parameters all unit
HDC_CCBA	EQU	$0F	; change command block address
SCSI_SET_MODE	EQU	$15	; SCSI set mode command
HDC_START	EQU	$1B	; SCSI START/STOP command
HDC_DDIAG	EQU	$E3	; run drive diagnostics
HDC_CDIAG	EQU	$E4	; run controller diagnostics
HDC_SDP1	EQU	$CC	; set drive parameters for unit 1

CMD_ZLUN	EQU	$1F	; Mask LUN to ZERO
CMD_LUNS	EQU	5	; Shift Right to justify LUN

    STRUCTURE	CMD,0		; Hard disk controller command block layout
	UBYTE	CMD_OPCODE		; Command class & opcode
	UBYTE	CMD_LUNHIADDR		; LUN [7:5] & sector addr [4:0]
	UBYTE	CMD_MIDADDR		; Middle sector address
	UBYTE	CMD_LOWADDR		; Low part of sector address
	UBYTE	CMD_BLOCKCNT		; Number of blocks to I/O
	UBYTE	CMD_CONTROL		; Reserved control byte
	UBYTE	CMD_HIGHDMA		; High byte of DMA address
	UBYTE	CMD_MIDDMA		; Middle byte of DMA address
	UBYTE	CMD_LOWDMA		; Low byte of DMA address
	UBYTE	CMD_RSVD1		; Reserved
	UBYTE	CMD_RSVD2		; Reserved
	UBYTE	CMD_RSVD3		; Reserved
	UBYTE	CMD_ERRORBITS		; Error information
	UBYTE	CMD_LUNLADD2		; Error LUN and high sector addr
	UBYTE	CMD_LADD1		; Error middle sector address
	UBYTE	CMD_LADD0		; Error low sector address
	LABEL	CMD_SIZE

	IFD	HASSCSI
    STRUCTURE	CMD0,0			; SCSI Class 0 Command block
	UBYTE	CMD0_OPCODE		; Command class [7:5] & opcode [4:0]
	UBYTE	CMD0_LUNHIADDR		; LUN [7:5] & sector addr [4:0]
	UBYTE	CMD0_MIDADDR		; Middle sector address
	UBYTE	CMD0_LOWADDR		; Low part of sector address
	UBYTE	CMD0_BLOCKCNT		; Number of blocks to I/O
	UBYTE	CMD0_CONTROL		; Reserved control byte
	LABEL	CMD0_SIZE
	ENDC

*    STRUCTURE	SCSIB,0			; Direct SCSI command structure
* WARNING!! SCSIB_DATA_BUFFER MUST be EVEN!!
*	APTR	SCSIB_DATA_BUFFER	; Points to data to send/receive
* SCSIB_DATA_LENGTH is the length in BYTES! (may not work if odd)
*	Only low order 3 bytes are used
*	ULONG	SCSIB_DATA_LENGTH	; Amount of data to transfer
*	APTR	SCSIB_CMD		; Points to SCSI command/parameter list
*	UWORD	SCSIB_CMD_LENGTH	; # of bytes in actual command
*	UBYTE	SCSIB_DIRECTION		; 0 means DMA TO SCSI device
					; 1 means DMA FROM SCSI device
*	UBYTE	SCSIB_STATUS		; Status returned by SCSI device
					;  if IO_STATUS is HD_NO_ERROR.
*	UBYTE	SCSIB_CTLR_STATUS	; Controller status
					; $80 = GOOD
					; $32 = DMA error
					; $F1 = Illegal SCSI bus phase
					; $F2 = Unexpected SCSI bus phase
					; $F3
					; $F4
					; $FD = Parity Error
					; $FE = Select timed out
*	LABEL	SCSIB_SIZE

    STRUCTURE HDP,0		; Hard disk controller parameters
	UBYTE	HDP_OPTSTEP		; Options and step rate
	UBYTE	HDP_HEADHICYL		; heads <7:4>, hi cyl count <3:0>
	UBYTE	HDP_CYL			; Low byte of cylinder count
	UBYTE	HDP_PRECOMP		; Precomp cylinder / 16
	UBYTE	HDP_REDUCE		; Reduced write current / 16
	UBYTE	HDP_SECTORS		; Sectors per track
	UWORD	HDP_PARK_CYL		; Cylinder to park heads at
	LABEL	HDP_SIZE

    STRUCTURE HDB,0		; Boot sector layout
	UWORD	HDB_MAGIC1		; 0xBABE indicates valid block
	UWORD	HDB_FILL1		; Skip 16 bits	
	BLOCKNO	HDB_PT_BBLOCKS		; Pointer to 1st bad block record
	ULONG	HDB_CYLINDERS		; # of cylinders on drive
*		Don't re-arrange order of next 10 lines - driver.asm uses it!
	ULONG	HDB_PRECOMP		; Pre-comp cylinder #
HDB_DOSNPTR	EQU	HDB_PRECOMP	; Ptr to Device name for AmigaDOS
	ULONG	HDB_REDUCE		; Reduced write current cylinder
HDB_EXECNAME	EQU	HDB_REDUCE	; Ptr to Device driver name for AmigaDOS
	ULONG	HDB_PARK		; Cylinder to park heads at (0 = don't)
HDB_UNITNO	EQU	HDB_PARK	; Unit number for AmigaDOS
	UWORD	HDB_SECTORS		; Sectors per track
HDB_DEVFLGS	EQU	HDB_SECTORS	; Flags for OpenDevice
	UWORD	HDB_HEADS		; Surfaces on device
	STRUCT	HDB_ENVIRONMENT,50*4	; AmigaDOS Environment for 1st partition
	ULONG	HDB_TBUFFS		; # of track buffers (if applicable)
	STRUCT	HDB_FILL2,280		; Round out to 512 bytes
HDB_DOSNAME	EQU	HDB_FILL2	; Device name for AmigaDOS
	LABEL	HDB_SIZE

    STRUCTURE BAO,0			; Bad disk address structure
	UWORD	BAO_CYL			; Cylinder number of bad area
	UWORD	BAO_OFFSET		; Offset to bad areas
	UBYTE	BAO_HEAD		; Head number of bad area
	UBYTE	BAO_FILL		; Make structure EVEN length
	LABEL	BAO_SIZE

    STRUCTURE BBM,0		; Bad block / good block pair
        BLOCKNO	BBM_BAD			;  Bad block #
        BLOCKNO	BBM_GOOD		;  Good block #
	LABEL	BBM_SIZE

    STRUCTURE BBR,0			; Bad block table layout
	UWORD	BBR_MAGIC1		; Magic ($BAD1)
	UWORD	BBR_COUNT		; Number of blocks in this record
	BLOCKNO	BBR_NEXT_REC		; Pointer to 2nd half of this table
	STRUCT	BBR_TABLE,BBM_SIZE*NBAD	; Table of bad-to-good block #s
	BLOCKNO	BBR_NEXT_FREE		; Next free map block
	UWORD	BBR_LEFT		; # remaining unused map blocks
	UWORD	BBR_MAGIC2		; Magic ($BAD2)
	LABEL	BBR_SIZE

	BITDEF	IO,IMMEDIATE,7

	IFD	BufferIO
*-- Buffer information structure
 STRUCTURE BI,0
	APTR	BI_UNIT			; Ptr to UNIT structure
	BLOCKNO	BI_START		; Logical # of 1st block in buffer
	BLOCKNO	BI_END			; Logical # + 1 of last block
	BLOCKNO	BI_PHYSICAL		; Actual # of first block in buffer
	STRUCT	BI_DATA,BUF_SECTS*512	; Actual BUFFER
	UWORD	BI_ACC			; Access count for LRU
	UBYTE	BI_BLKS			; # of blocks currently in buffer
	UBYTE	BI_FILL			; Make size long word aligned
	LABEL	BI_SIZE
	ENDC

 STRUCTURE	SCP,0			; Set of saved pointers, 1 per LUN
	APTR	SCP_DATA		; SCSI Saved Data pointer
	APTR	SCP_COMMAND		; SCSI Saved Command pointer
	APTR	SCP_STATUS		; SCSI Saved Status pointer
	UBYTE	SCP_PHASE		; Phase of command at last interrupt
	LABEL	SCP_SIZE

*-- Unit Structure
 STRUCTURE HDU,UNIT_SIZE
	APTR	HDU_ENTRY		; ptr to MemEntry for unit
	APTR	HDU_FMTMEM		; RememberKey for FORMAT
	BLOCKNO	HDU_PARK		; Block to park heads at (if not 0)
	BLOCKNO	HDU_LAST		; Block # of last accessible block
	BLOCKNO	HDU_BOOT		; Block # of "boot" block
	BLOCKNO	HDU_1BAD		; Block # of 1st half of bad block table
	BLOCKNO	HDU_2BAD		; Block # of 2nd half of bad block table
	STRUCT	HDU_BB,BBR_SIZE		; Bad block structure
	UWORD	HDU_SECTORS		; # of sectors per track
	UBYTE	HDU_HEADS		; # of surfaces per cylinder
	UBYTE	HDU_FLAGS		; unit flags
	UBYTE	HDU_RETRY		; count of retries
	UBYTE	HDU_UNITNUM		; the mask for the unit number
	UBYTE	HDU_UNIT		; the "actual" number of our unit
	UBYTE	HDU_MOTOR		; the state of the "Motor" (parked?)
	IFD	HASSCSI
	STRUCT	HDU_SCP,SCP_SIZE*8	; SAVED SCSI pointers for all LUNS
	ENDC
	IFD	BufWrites
	APTR	HDU_WRITEB		; Buffer that has been written to
	ENDC
	APTR	HDU_CTLR		; Pointer to controller's structure
	LABEL	HDU_SIZE

*-- HDU flags
	BITDEF	HDU,WRITEBAD,7	; Bad block table needs writing to disk
	BITDEF	HDU,BADFND,6	; Bad block found during operation
	BITDEF	HDU,IOERR,5	; HARD IO error detected
; HDU_MSGING is ignored unless it is set for all attached SCSI devices.  That
; must mean that they all support messaging!  This is necessary if disconnects
; are to be possible, allowing for overlapped seeks.
; (When this driver supports it!)
	BITDEF	HDU,MSGING,4	; Use messaging
	BITDEF	HDU,EX_SCSI,3	; Use the extended SCSI command format
				; Bits 2-0 are reserved for SCSI LUN

*-- Pseudo-device Structure : one per controller board
 STRUCTURE DV,DD_SIZE
	UWORD	DV_FILL1		; Align to longword boundary
	APTR	DV_0			; ptr to controller 0 structure
	APTR	DV_1			; ptr to controller 1 structure
	APTR	DV_2			; ptr to controller 2 structure
	APTR	DV_3			; ptr to controller 3 structure
	APTR	DV_4			; ptr to controller 4 structure
	APTR	DV_5			; ptr to controller 5 structure
	APTR	DV_6			; ptr to controller 6 structure
	APTR	DV_7			; ptr to controller 7 structure
	APTR	DV_8			; ptr to controller 8 structure
	APTR	DV_9			; ptr to controller 9 structure
	LABEL	DV_SIZE

*-- Pseudo-device Structure : one per controller board
 STRUCTURE HD,0
	APTR	HD_CMDPTR		; ptr to controller's command block
	APTR	HD_SYSLIB		; ptr to system library
	APTR	HD_INTUITLIB		; ptr to intuition library
	APTR	HD_BASE			; Base address of controller card
	APTR	HD_DMA			; IO DMA Address
	APTR	HD_UNIT			; Pointer to currently active unit
	APTR	HDU0			; unit structure for unit 0
	APTR	HDU1			; unit structure for unit 1
	IFD	HASSCSI
	APTR	HDUS0			; unit structure for SCSI unit 0
	APTR	HDUS1			; unit structure for SCSI unit 1
	APTR	HDUS2			; unit structure for SCSI unit 2
	APTR	HDUS3			; unit structure for SCSI unit 3
	APTR	HDUS4			; unit structure for SCSI unit 4
	APTR	HDUS5			; unit structure for SCSI unit 5
	APTR	HDUS6			; unit structure for SCSI unit 6
	APTR	HDUS7			; unit structure for SCSI unit 7
	ENDC
	APTR	HD_IO_REQUEST		; Current IO Request being processed
	ULONG	HD_LBLOCK		; Logical IO Sector Address
	ULONG	HD_PBLOCK		; Physical IO Sector Address
	ULONG	HD_BLOCKCNT		; Remaining blocks to I/O
	STRUCT	HD_STACK,HD_STKSIZE	; Stack for disk task
	STRUCT	HD_IS,IS_SIZE		; Interrupt structure
	UBYTE	HD_SCSI_ST_1		; Save SCSI chip status
	UBYTE	HD_SCSI_ST_2
	STRUCT	HD_MP,MP_SIZE		; Message port for I/O requests
	UBYTE	HD_FLAGS		; general device flags
	UBYTE	HD_CMD			; Current controller command to execute
	STRUCT	HD_TCB,TC_SIZE		; TCB for disk task
	STRUCT	HD_CMDAREA,512+CMD_SIZE	; Area where command block will
	IFD	BufferIO
	STRUCT	HD_BUFS,BI_SIZE*NUM_BUFS; Data buffer information
	ENDC
					;	be located on 512 byte boundary
	IFD	HASSCSI
	UBYTE	HD_MYLUN		; This controllers SCSI LUN
	ENDC
	LABEL	HD_SIZE

*-- HD flags
	BITDEF	HD,DMAACTIV,7	; The DMA chip is currently active
	BITDEF	HD,ACTIVE,6	; Controller currently processing command
	BITDEF	HD,512,5	; Use DMA chip 512 byte mode for this write
	BITDEF	HD,SCSI,4	; TRUE means SCSI device active, else ST-506
; If HD_MSGING will be set if the flags byte for ANY unit specifies HDU_MSGING.
; This is necessary if disconnects are to be possible, allowing for overlapped
; seeks (if this driver supports it!)
	BITDEF	HD,MSGING,3	; Use messaging
	BITDEF	HD,HASSCSI,2	; TRUE if controller has SCSI hardware installed
	BITDEF	HD,NO512,1	; TRUE - controller DOESN'T have KONAN hardware
	BITDEF	HD,IN,0		; TRUE if SCSI INPUT (Read)

*	Signal definitions

	BITDEF	HD,CMDDONE,8	; Signal that controller has interrupted
	BITDEF	HD,NEWIOREQ,9	; IO Request message
	BITDEF	HD,DEVDONE,10
	BITDEF	HD,OWNIO,11	; Signal bit for own internal IO requests

*    ;------	temporary storage structure - used by
*    ;------	init.asm, format.asm
    STRUCTURE INIT,0
	STRUCT	INIT_SECT,512		; Sector workspace
	STRUCT	INIT_MP,MP_SIZE		; Internal Message Port
	UWORD	INIT_DH			; DHx ASCII
	STRUCT	INIT_IOR,IOSTD_SIZE	; IORequest block
	STRUCT	INIT_CMD,CMD_SIZE	; Controller command block image
	APTR	INIT_CADDR		; Temp storage for addresses
	APTR	INIT_BADDR		; Temp storage for addresses
	STRUCT	INIT_CBIND,CurrentBinding_SIZEOF; Current binding structure
	STRUCT	INIT_HDP,HDP_SIZE	; KONAN controller parameters
	STRUCT	INIT_BUFFER,64*512	; Max 64 sectors/track
	LABEL	INIT_SIZE
