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
DMA_FLUSH_OK	EQU	1

;==============================================================================
; Leave this uncommented to build an A590/A2091 SCSI/XT driver
;==============================================================================
A590		EQU	1

;==============================================================================
; Leave this uncommented to build an A2090 SCSI/ST506 driver (NOT DONE YET!!!)
;==============================================================================
;A2090		EQU	1

;==============================================================================
; Offset from base of board to the start of the code in the autoboot roms.  If
; AUTOBOOT_ROM is not defined, then an expansion drawer device will be built.
;==============================================================================
;	IFD	A590
;AUTOBOOT_ROM	EQU	$2000
;	ENDC

;	IFD	A2090
;AUTOBOOT_ROM	EQU	$2000
;	ENDC

;==============================================================================
; romboot library (AddDosNode) used to screw up and not make proper bootnodes.
; this is fixed under 1.4 so this skips code in mountstuff.asm
;==============================================================================
;ROMBOOT_FIXED	EQU	1

;==============================================================================
; The A2091 does not support xt devices anymore.  Uncomment for an A590 driver.
;==============================================================================
;XT_SUPPORTED	EQU	1

;==============================================================================
; Constants used throughout and also used for tuning purposes
;==============================================================================
TRUE		EQU	-1
FALSE		EQU	0

DEVSTACK	EQU	1000			size of device task stacks
HD_PRI		EQU	10			priority of startup task
INTNUM		EQU	3			4703 interrupt we service
INTPRI		EQU	20			priority of our int server
CMD_BLKS	EQU	10			number of cmd blks to allocate
;CMD_BLKS	EQU	2			number of cmd blks to allocate

MAX_CMD		EQU	28			highest IO command #

; enable this definition to do programmed I/O for everything
;NODMA 		EQU	1

;enable this definition to build a 1.3 compatible bonus-ROM version of the driver
;BONUS		EQU	1
