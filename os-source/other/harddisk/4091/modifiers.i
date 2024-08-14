;==============================================================================
; Very first revision of the DMA chip has XTPORT1 and XTPORT2 reversed.
;==============================================================================
;REAL_OLD	EQU	1

;==============================================================================
; Early revs of the A590 DMA chip would drop a couple of words at the end of
; a read operation.  With FIXING_READS defined the last block of a transfer is
; extended to read the next block too into an internal buffer.  The required
; block is then copied with the CPU to the final destination.  This is pretty
; klugey, but not required on final DMA chips.
;==============================================================================
;FIXING_READS	EQU	1

;==============================================================================
; The latest rev of the A590 DMA chip does not handle flushing properly.  This
; means that disconnection and reselection cannot be handled because the DMA's
; FIFO cannot be flushed in the middle of a read or write.  Uncomment when the
; DMA chip gets fixed (hopefully no 2091 boards will go out with this chip in)
;
; Augi tells me that the B chip requires terminal count, and was only used in
; A590's (but it might have gotten into a few 2091's).  The product code for
; the B chip was 2, and the product code for the D and later chips is 3.  The
; D and later DO NOT work correctly with terminal count!  Because of this, the
; 590/2091 driver will check the DMAC product code and act accordingly. - REJ
;==============================================================================
;DMA_FLUSH_OK	EQU	1
DMAC_FLUSH_REV	EQU	3

;==============================================================================
; Leave this uncommented to build an A590/A2091 SCSI/XT driver
; (Really all but A2090!)
;==============================================================================
	IFND	IS_A2090
A590		EQU	1
	ENDC	IS_A2090

; for A590/A2091 ONLY
	IFD	IS_A2091
A590_A2091_ONLY		EQU	1
	ENDC
	IFD	IS_A590
A590_A2091_ONLY		EQU	1
	ENDC

;==============================================================================
; Leave this uncommented to build an A2090 SCSI/ST506 driver (NOT DONE YET!!!)
;==============================================================================
	IFD	IS_A2090
A2090		EQU	1
	ENDC

;==============================================================================
; Is this a kickstart ROM module?  Means that no expansion scsi.device can
; be inited before me, for example.
;==============================================================================
	IFD	IS_A300
IN_KICK		EQU	1
	ENDC
	IFD	IS_A1000
IN_KICK		EQU	1
	ENDC
	IFD	IS_CDTVCR
IN_KICK		EQU	1
	ENDC
	IFD	IS_SCSIDISK
IN_KICK		EQU	1
	ENDC
	IFD	IS_A4000T
IN_KICK		EQU	1
	ENDC
; for making the kludge disk-loadable driver
	IFD	IS_A3090_DISK
IN_KICK		EQU	1
	ENDC

;==============================================================================
; Is this a Zorro-3 board?  (note a3090_disk is also IS_A3090)
;==============================================================================
	IFD IS_A3090
ZORRO_3		EQU	1
	ENDC
; FIX!!!! REMOVE!!! temp for first A4000
;	IFD IS_A4000T
;ZORRO_3		EQU 1
;	ENDC

;=============================================================================
; Do we get our parameters from battmem or jumpers?
;=============================================================================
;	IFD IS_A4000T
;USE_BATTMEM	EQU	1
;	ENDC
	IFD IS_SCSIDISK
USE_BATTMEM	EQU	1
	ENDC
	IFD IS_A1000
USE_BATTMEM	EQU	1
	ENDC

	IFD IS_A4000T
USE_JUMPERS	EQU	1
	ENDC
	IFD IS_A3090
USE_JUMPERS	EQU	1
	ENDC
	IFD A590_A2091_ONLY
USE_JUMPERS	EQU	1
	ENDC

;==============================================================================
; Does this driver support CAM?
;==============================================================================
;	IFD NCR53C710
;USES_CAM	EQU	1
;	ENDC

;==============================================================================
; Offset from base of board to the start of the code in the autoboot roms.  If
; AUTOBOOT_ROM is not defined, then an expansion drawer device will be built.
; Scsidisk and ide need the autoboot code, but don't need to find the board
; (controlled by an IFD EXPANSION_DEV in the source)
;==============================================================================

	IFND IN_KICK
	IFD IS_A3090
NOT_EXECUTE_IN_PLACE EQU 1
	ENDC
	IFND IS_A3090
EXECUTE_IN_PLACE EQU	1
	ENDC
	ENDC

	IFND	IS_DISK
	IFD IS_A3090
AUTOBOOT_ROM	EQU	$0000	// rom starts at beginning of PIC
	ENDC
	IFND IS_A3090
AUTOBOOT_ROM	EQU	$2000
	ENDC
	ENDC

	IFND	IN_KICK
EXPANSION_DEV	EQU	1
	ENDC

	IFD IS_A3090
A3090_ROM_OFFSET 	EQU	$00000000
A3090_DRIVER_OFFSET	EQU	$00002000 // allow 8K/4 bytes for loader
SCSI_CHIP_OFFSET 	EQU	$00800000
A3090_VECTOR	  	EQU	$00880003
A3090_JUMPERS	  	EQU	$008C0003
NCR_WRITE_OFFSET	EQU	$00040000 // offset for longword writes...
	ENDC
	IFD IS_A3090_DISK
NCR_ADDRESS		EQU	$40000000 // assume first Z-3 card!
	ENDC

;==============================================================================
; Does this have to work under 1.3?
;==============================================================================
	IFD	IS_A590
KS_1_3	EQU	1
	ENDC
	IFD	IS_A2091
KS_1_3	EQU	1
	ENDC

;==============================================================================
; Is this a V39 kickstart module?
;==============================================================================
	IFD	IS_A300
V39		EQU	1
	ENDC
	IFD	IS_A1000
V39		EQU	1
	ENDC
	IFD	IS_SCSIDISK
V39		EQU	1
	ENDC
	IFD	IS_A4000T
V39		EQU	1
	ENDC

;==============================================================================
; romboot library (AddDosNode) used to screw up and not make proper bootnodes.
; this is fixed under 1.4 so this skips code in mountstuff.asm
;==============================================================================
	IFND	KS_1_3
ROMBOOT_FIXED	EQU	1
	ENDC

;==============================================================================
; A590/A2091/A2090 con only DMA to 24-bit space, must use programmed IO above.
;==============================================================================
	IFD	IS_A590
DMA_24_BIT	EQU	1
	ENDC
	IFD	IS_A2091
DMA_24_BIT	EQU	1
	ENDC
	IFD	IS_A2090
DMA_24_BIT	EQU	1
	ENDC

;==============================================================================
; Do we want the Get Geometry code enabled...
;==============================================================================
GET_GEOMETRY	EQU	1

;==============================================================================
; Max sector size supported by the Z3->Z2 copying code (buffer size)
;==============================================================================
Z2_BUFFERSIZE	EQU	2048

;==============================================================================
; NCR 53c710 address in A4000T
;==============================================================================
	IFD IS_A4000T
NCR_ADDRESS	 EQU	$00dd0040
NCR_WRITE_OFFSET EQU	$00000080	; 128 bytes
A3090_JUMPERS	 EQU	($00003000-$40)
NCR_DIP_ADDR	 EQU	$0		; ?????
A3090_CHIP_OFFSET EQU	0
	ENDC

;==============================================================================
; Various IDE things...
;==============================================================================

;==============================================================================
; The ide driver might as well run as one task, since it doesn't reselect
; anyways.  This can also be used for SCSI if we want.
;==============================================================================
	IFD IS_IDE
ONE_TASK	EQU	1
	ENDC

;==============================================================================
; WD Caviar 2120 (120Meg) AT IDE drives return the interrupt before the data is
; ready.  The Caviar 140 (40Meg) is ok.  Actually, about 90% of all AT drives
; have this problem to one degree or another.
;==============================================================================
WD_AT_KLUDGE	EQU	1

;==============================================================================
; The Conner CP2024 2.5" AT drive (20MB) comes up in translate mode (615/4/17)
; and refuses to listen to InitializeDriveParameters commands.  It says it's
; 653/2/32.  This violates the CAM-ATA spec.  We need to kludge for this drive
; by explicitly recognizing it.
;==============================================================================
CP2024_KLUDGE	EQU	1

;==============================================================================
; Define this if you want removable ide media to be identified as tape drives.
; Leave it undefined if removable ide media is to be identified as a disk.
;==============================================================================
IDE_TAPES_SUPPORTED	EQU	1

;==============================================================================
; Randy Hilton says we must do an enable interrupt, though it seems most
; drives come up with them enabled (except the PrairieTek 342).
; The CDTVCR can't access that register
;==============================================================================
	IFND IS_CDTVCR
USE_ENABLE_INTS	EQU	1
	ENDC

;==============================================================================
; Randy Hilton says we must do an Initialize Drive Parameters for the same
; parameters it has told us in it's identify drive response.  According to
; the CAM-ATA spec, this should NOT be needed, but Randy insists.  This
; breaks the WD 2120 (120MB) drive (it refuses to allow read-multiple after
; this command, even if a Set Multiple is done).
;==============================================================================
USE_INIT_PARAM	EQU	1

;==============================================================================
; The A2091 does not support xt devices anymore.  Uncomment for an A590 driver.
;==============================================================================
	IFD	IS_A590
XT_SUPPORTED	EQU	1
XT_OR_AT_IDE	EQU	1
	ENDC

	IFD	IS_IDE
AT_SUPPORTED	EQU	1
XT_OR_AT_IDE	EQU	1
	IFND	A1000
FOR_68000_ONLY	EQU	1
	ENDC
	ENDC

	IFND	IS_IDE
SCSI_SUPPORTED	EQU	1
	ENDC

;==============================================================================
; SCSI hardware selection - 590 may have A part nowadays.  Should we
; adjust dynamically??  FIX!
;==============================================================================
	IFD IS_SCSIDISK
CLOCK_14MHZ	EQU	1
WD33C93A	EQU	1
	ENDC
	IFD IS_A590
CLOCK_7MHZ	EQU	1
WD33C93		EQU	1
	ENDC
	IFD IS_A2091
; FIX!  is this always true?  I hear it isn't!!!!!!!! FIX!!!!!!
CLOCK_7MHZ	EQU	1
WD33C93A	EQU	1
	ENDC

;==============================================================================
; Constants used throughout and also used for tuning purposes
;==============================================================================
TRUE		EQU	-1
FALSE		EQU	0

LONG_SPINUP_DELAY	EQU	15	; 15 second spinup time for slow drives

; Make sure DEVSTACK is a multiple of 4!
DEVSTACK	EQU	1000			size of device task stacks
HD_PRI		EQU	10			priority of startup task
INTNUM		EQU	3 	(INTB_PORTS)	4703 interrupt we service
;INTNUM		EQU	13
INTPRI		EQU	20			priority of our int server

	IFD IS_IDE
CMD_BLKS	EQU	2			I don't think I need more?
	ENDC
	IFND IS_IDE
CMD_BLKS	EQU	10			number of cmd blks to allocate
;CMD_BLKS	EQU	2			number of cmd blks to allocate
	ENDC

	IFD SCSI_SUPPORTED
MAX_LUN		EQU	7
MAX_ADDR	EQU	7
	ENDC
	IFD IS_IDE
MAX_LUN		EQU	0			i.e. IDE only
MAX_ADDR	EQU	1
	ENDC
MAX_XT_ADDR	EQU	1			A590 only

MAX_CMD		EQU	28			highest IO command #

; enable this definition to do programmed I/O for everything
;NODMA 		EQU	1

; enable this definition to force longward access to SASR for writes
;LONGWORD_SASR	EQU	1

;enable this definition to build a 1.3 compatible bonus-ROM version of the driver
;BONUS		EQU	1

