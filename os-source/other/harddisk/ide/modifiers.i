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
;==============================================================================
	IFND	IS_A590
DMA_FLUSH_OK	EQU	1
	ENDC

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
; Offset from base of board to the start of the code in the autoboot roms.  If
; AUTOBOOT_ROM is not defined, then an expansion drawer device will be built.
; Scsidisk and ide need the autoboot code, but don't need to find the board
; (controlled by an IFD EXPANSION_DEV in the source)
;==============================================================================

	IFND	IS_A590_DISK
	IFND	IS_A2091_DISK
	IFND	IS_A2090_DISK
AUTOBOOT_ROM	EQU	$2000
	ENDC
	ENDC
	ENDC

	IFND	IS_SCSIDISK
	IFND	IS_A300
	IFND	IS_A1000
EXPANSION_DEV	EQU	1
	ENDC
	ENDC
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
; romboot library (AddDosNode) used to screw up and not make proper bootnodes.
; this is fixed under 1.4 so this skips code in mountstuff.asm
;==============================================================================
	IFND	KS_1_3
ROMBOOT_FIXED	EQU	1
	ENDC

;==============================================================================
; Is this a kickstart ROM module?  Means that no expansion scsi.device can
; be inited before me, for example.
;==============================================================================
	IFD	IS_A300
IN_KICK		EQU	1
	ENDC
	IFD	IS_1000
IN_KICK		EQU	1
	ENDC
;	IFD	IS_CDTVCR			FIX!!!!! ADD
;IN_KICK		EQU	1
;	ENDC
	IFD	IS_SCSIDISK
IN_KICK		EQU	1
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
; Various IDE things...
;==============================================================================

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
	IFND SCSI_SUPPORTED
MAX_LUN		EQU	0			i.e. IDE only
MAX_ADDR	EQU	1
	ENDC

MAX_CMD		EQU	28			highest IO command #

; enable this definition to do programmed I/O for everything
;NODMA 		EQU	1

; enable this definition to force longward access to SASR for writes
;LONGWORD_SASR	EQU	1

;enable this definition to build a 1.3 compatible bonus-ROM version of the driver
;BONUS		EQU	1

