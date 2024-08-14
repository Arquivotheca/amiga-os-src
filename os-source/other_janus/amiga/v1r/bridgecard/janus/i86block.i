
;**********************************************************************
;
; i86block.i -- interface definitions between amiga and commodore-pc
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;**********************************************************************



	IFND	JANUS_I86BLOCK_I
JANUS_I86BLOCK_I    SET 1

; All registers in this section are arranged to be read and written
; from the WordAccessOffset area of the shared memory.	If you really
; need to use the ByteAccessArea, all the words will need to be byte
; swapped.


; Syscall86 -- how the 8086/8088 wants its parameter block arranged
 STRUCTURE Syscall86,0
    UWORD   s86_AX
    UWORD   s86_BX
    UWORD   s86_CX
    UWORD   s86_DX
    UWORD   s86_SI
    UWORD   s86_DS
    UWORD   s86_DI
    UWORD   s86_ES
    UWORD   s86_BP
    UWORD   s86_PSW
    UWORD   s86_INT		; 8086 int # that will be called
    LABEL   Syscall86_SIZEOF



; Syscall68 -- the way the 68000 wants its parameters arranged
 STRUCTURE Syscall68,0
    ULONG   s68_D0
    ULONG   s68_D1
    ULONG   s68_D2
    ULONG   s68_D3
    ULONG   s68_D4
    ULONG   s68_D5
    ULONG   s68_D6
    ULONG   s68_D7
    ULONG   s68_A0
    ULONG   s68_A1
    ULONG   s68_A2
    ULONG   s68_A3
    ULONG   s68_A4
    ULONG   s68_A5
    ULONG   s68_A6
    ULONG   s68_PC		; pc to start execution from
    ULONG   s68_ArgStack	; array to be pushed onto stack
    ULONG   s68_ArgLength	; number of bytes to be pushed (must be even)
    ULONG   s68_MinStack	; minimum necessary stack (0 = use default)
    ULONG   s68_CCR		; condition code register
    ULONG   s68_Process 	; ptr to process for this block.
    UWORD   s68_Command 	; special commands: see below
    UWORD   s68_Status		; 
    UWORD   s68_SigNum		; internal use: signal to wake up process
    LABEL   Syscall68_SIZEOF


S68COM_DOCALL	EQU 0		; normal case -- jsr to specified Program cntr
S68COM_REMPROC	EQU 1		; kill process
S68COM_CRPROC	EQU 2		; create the proces, but do not call anything


; disk request structure for raw amiga access to 8086's disk
; goes directly to PC BOIS (via PC int 13 scheduler)
 STRUCTURE DskAbsReq,0
    UWORD   dar_FktCode 	; bios function code (see ibm tech ref)
    UWORD   dar_Count		; sector count
    UWORD   dar_Track		; cylinder #
    UWORD   dar_Sector		; sector #
    UWORD   dar_Drive		; drive
    UWORD   dar_Head		; head
    UWORD   dar_Offset		; offset of buffer in MEMF_BUFFER memory
    UWORD   dar_Status		; return status
    LABEL   DskAbsReq_SIZEOF


; definition of an AMIGA disk partition.  returned by info function
 STRUCTURE DskPartition,0
    UWORD   dp_Next		; 8088 ptr to next part.  0 -> end of list
    UWORD   dp_BaseCyl		; cyl # where partition starts
    UWORD   dp_EndCyl		; last cyclinder # of this partition
    UWORD   dp_DrvNum		; DOS drive number (80H, 81H, ...)
    UWORD   dp_NumHeads 	; number of heads for this drive
    UWORD   dp_NumSecs		; number of sectors per track for this drive
    LABEL   DskPartition_SIZEOF


; disk request structure for higher level amiga disk request to 8086
 STRUCTURE AmigaDskReq,0
    UWORD   adr_Fnctn		; function code (see below)
    UWORD   adr_Part		; partition number (0 is first partition)
    ULONG   adr_Offset		; byte offset into partition
    ULONG   adr_Count		; number of bytes to transfer
    UWORD   adr_BufferAddr	; offset into MEMF_BUFFER memory for buffer
    UWORD   adr_Err		; return code, 0 if all OK
    LABEL   AmigaDskReq_SIZEOF


; function codes for AmigaDskReq adr_Fnctn word
ADR_FNCTN_INIT	    EQU     0	; given nothings, sets adr_Part to # partitions
ADR_FNCTN_READ	    EQU     1	; given partition, offset, count, buffer
ADR_FNCTN_WRITE     EQU     2	; given partition, offset, count, buffer
ADR_FNCTN_SEEK	    EQU     3	; given partition, offset
ADR_FNCTN_INFO	    EQU     4	; given part, buff adr, cnt, copys in a
				;   DskPartition structure. cnt set to actual
				;   number of bytes copied.

; error codes for adr_Err, returned in low byte

ADR_ERR_OK	    EQU     0	; no error
ADR_ERR_OFFSET	    EQU     1	; offset not on sector boundary
ADR_ERR_COUNT	    EQU     2	; dsk_count not a multiple of sector size
ADR_ERR_PART	    EQU     3	; partition does not exist
ADR_ERR_FNCT	    EQU     4	; illegal function code
ADR_ERR_EOF	    EQU     5	; offset past end of partition
ADR_ERR_MULPL	    EQU     6	; multiple calls while pending service


; error condition from IBM-PC BIOS, returned in high byte

ADR_ERR_SENSE_FAIL	EQU	$ff
ADR_ERR_UNDEF_ERR	EQU	$bb
ADR_ERR_TIME_OUT	EQU	$80
ADR_ERR_BAD_SEEK	EQU	$40
ADR_ERR_BAD_CNTRLR	EQU	$20
ADR_ERR_DATA_CORRECTED	EQU	$11	; data corrected
ADR_ERR_BAD_ECC 	EQU	$10
ADR_ERR_BAD_TRACK	EQU	$0b
ADR_ERR_DMA_BOUNDARY	EQU	$09
ADR_ERR_INIT_FAIL	EQU	$07
ADR_ERR_BAD_RESET	EQU	$05
ADR_ERR_RECRD_NOT_FOUND EQU	$04
ADR_ERR_BAD_ADDR_MARK	EQU	$02
ADR_ERR_BAD_CMD 	EQU	$01




	ENDC	!JANUS_I86BLOCK_I



