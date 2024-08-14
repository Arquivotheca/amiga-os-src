*************************************************************************
*									*
*	Copyright (C) 1986, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* scsi.asm
*
* Source Control
* ------ -------
*
* $Header: scsi.asm,v 30.2 86/01/20 00:36:39 LCE Exp $
*
* $Locker:  $
*
* $Log: io.asm,v $
* *** empty log message ***
*
*
*************************************************************************

******* Included Files ***********************************************

	NOLIST
	IFND	EXEC_TYPES_I
	INCLUDE 'exec/types.i'
	ENDC
	IFND	EXEC_INTERRUPTS_I
	INCLUDE	"exec/interrupts.i"
	ENDC
	IFND	EXEC_LISTS_I
	INCLUDE 'exec/lists.i'
	ENDC
	IFND	EXEC_NODES_I
	INCLUDE 'exec/nodes.i'
	ENDC
	IFND	EXEC_PORTS_I
	INCLUDE 'exec/ports.i'
	ENDC
	IFND	EXEC_LIBRARIES_I
	INCLUDE 'exec/libraries.i'
	ENDC
	IFND	EXEC_IO_I
	INCLUDE 'exec/io.i'
	ENDC
	IFND	EXEC_DEVICES_I
	INCLUDE 'exec/devices.i'
	ENDC
	IFND	EXEC_TASKS_I
	INCLUDE 'exec/tasks.i'
	ENDC
	IFND	EXEC_MEMORY_I
	INCLUDE 'exec/memory.i'
	ENDC
	IFND	EXEC_EXECBASE_I
	INCLUDE 'exec/execbase.i'
	ENDC
	IFND	EXEC_ABLES_I
	INCLUDE 'exec/ables.i'
	ENDC
	IFND	EXEC_STRINGS_I
	INCLUDE 'exec/strings.i'
	ENDC

	INCLUDE 'hddisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'
	LIST

        IFD     HASSCSI
        SECTION section
*SPARITY        SET     1       ; Defined if SCSI parity errors reported

******* Imported Names ***********************************************

	XREF    hdName
*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

*------ Functions ----------------------------------------------------

*------ System Library Functions -------------------------------------

	EXTERN_LIB Debug
	EXTERN_LIB Wait
	EXTERN_LIB AddIntServer
	EXTERN_LIB SetSignal

******* Exported Names ***********************************************

*------ Functions ----------------------------------------------------
	XDEF    SExIO	   ; SCSI I/O
	XDEF    SCSIINIT	; Initialize SCSI chip

*------ Data ---------------------------------------------------------

SCSIINIT:				; Initialize SCSI chip
	MOVEM.L	D0/D1/A0/A1/A5,-(SP)
	MOVE.L	HD_BASE(A6),A5		; Get controller base addr
	IFGE	INFO_LEVEL-40
	MOVE.L	A5,-(SP)
	INFOMSG	40,<'%s/SCSIInit: BASE 0x%lx'>
	ADDQ.L	#4,SP
	ENDC

	MOVE.B  #$60,HDSTAT1(A5)	; SCSI I/O, no interrutps

	MOVE.B  #SC_STATUS,SCSIADDR(A5) ; Read SCSI Status to clear interrupt ?
	MOVE.B  SCSIIND(A5),D0

	INFOMSG 40,<'%s/SCSIInit: About to add int server'>
*       ;-----  Add the interrupt server
	LEA.L   HD_IS(A6),A1		; Point to interrupt structure
	MOVEQ   #3,D0			; Portia interrupt bit 3
	LINKSYS AddIntServer		; Now install the server

	BSET.B  #HDB_SCSI,HD_FLAGS(A6)  ; Indicate SCSI active
	INFOMSG 40,<'%s/SCSIInit: About enable interrupts'>
	MOVE.B  #$70,HDSTAT1(A5)	; Enable interrupts, SCSI I/O

*       Reset SCSI chip

	MOVE.B  #SC_OWN_ID,SCSIADDR(A5) ; Point to OWN ID register
	MOVE.L	A6,A0
	ADD.L	#HD_MYLUN,A0
	MOVE.B  #$07,(A0)		; This unit's LUN is 7 for now
        MOVE.B  (A0),SCSIIND(A5); This is LUN 7 for now
        MOVE.B  #SC_OWN_ID,SCSIADDR(A5) ; Point back at OWN_ID register
        CMP.B   #$07,SCSIIND(A5)        ; Is there a SCSI chip there?
        BNE     NoSCSI                  ;       Exit if not
        BSET.B  #HDB_HASSCSI,HD_FLAGS(A6)
	MOVEQ	#0,D0
        MOVE.L  #HDF_CMDDONE,D1         ; Make sure signal not already
        LINKSYS SetSignal               ;       pending
        MOVE.B  #SC_COMMAND,SCSIADDR(A5); Point to COMMAND register
        MOVE.B  #RESET_CMD,SCSIIND(A5)  ; Issue RESET command
        INFOMSG 40,<'%s/SCSIInit: About to wait for interrupt'>
        MOVE.L  #HDF_CMDDONE,D0         ; Wait for interrupt from
        LINKSYS Wait                    ;       the controller
SInitLp0:
	BTST.B	#5,SCSIAUX(A5)		; See if chip still busy
	BNE.S	SInitLp0		;	Wait if it is
SInitLp:                                ; Make sure value gets into register
        MOVE.B  #SC_CONTROL,SCSIADDR(A5); Point to CONTROL register
        MOVE.B  #SCSI_EN_DMA,SCSIIND(A5); CONTROL = Use DMA, no halt on parity
        MOVE.B  #SC_CONTROL,SCSIADDR(A5); Read value back and compare
        CMP.B   #SCSI_EN_DMA,SCSIIND(A5)
        BNE.S   SInitLp
        MOVE.B  #SCSI_TIMEOUT,SCSIIND(A5); TIMEOUT = 2.04 seconds
        MOVE.B  #SC_STATUS,SCSIADDR(A5) ; Read SCSI Status to clear interrupt ?
        MOVE.B  SCSIIND(A5),D0

*       Reset DMA chip

NoSCSI:
        INFOMSG 40,<'%s/SCSIInit: NoSCSI'>
        MOVE.B  #$50,HDSTAT1(A5)        ; Indicate ST 506 I/O
        BCLR.B  #HDB_SCSI,HD_FLAGS(A6)  ; Indicate SCSI active
        MOVE.B  #SOFT_RESET,PCSS(A5)    ; Issue Software reset to DMA chip
        MOVE.B  #END_RESET,PCSS(A5)     ; End software reset

        MOVEM.L (SP)+,D0/D1/A0/A1/A5
        RTS

******* System/Drivers/HD/SExIO ***************************************
*
*   NAME
*       SExIO - do actual SCSI IO to device
*
*   SYNOPSIS
*       SExIO(DevPtr)
*               A6
*
*   FUNCTION
*       This routine is responsible for taking the built command block,
*       and getting the actual I/O done.
*
*   INPUTS
*       Device Structure
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
*---------------------------------------------------------------------
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*       A6 -- DevPtr
*       A4 -- device command block
*       A5 -- Board base address
*       D2 -- DMA address
*
**********************************************************************

SExIO:

                MOVEM.L D0-D2/A1-A5,-(SP)
                BTST.B  #HDB_HASSCSI,HD_FLAGS(A6)
                BEQ     SExExit                 ; If not set, no chip there
                MOVE.L  HD_BASE(A6),A5          ; Get controller base addr
                BSET.B  #HDB_SCSI,HD_FLAGS(A6)  ; Indicate SCSI active
                MOVE.B  #$70,HDSTAT1(A5)        ; Enable interrupts, SCSI I/O
                MOVE.L  HD_CMDPTR(A6),A4        ; Get command block address
                LEA     SCSIADDR(A5),A3         ; Point to SCSI address reg
                LEA     SCSIIND(A5),A2          ; Point to indirect SCSI reg

*       Screen for supported opcodes

SExRetry:       CLR.B   CMD_RSVD2(A4)           ; Initialize working variables
		BSET.B  #HDB_512,HD_FLAGS(A6)   ; SExDmaAdr 512 byte mode TEST
*LCE		BCLR.B  #HDB_512,HD_FLAGS(A6)   ; Tell SExDmaAdr UNLIMITED mode
                BSR     ZeroTC                  ; Zero transfer count
                MOVE.B  CMD_OPCODE(A4),D0       ; Get requested command
                CMP.B   #HDC_READ,D0            ; Is it a read?
                BEQ	SExRead                 ;       Yes, go do it
                CMP.B   #HDC_WRITE,D0           ; Is it a write?
                BEQ     SExWrite                ;       Yes, go do it
                CMP.B   #HDC_REST,D0            ; Is it a restore?
                BEQ     SExCSetup               ;       Normal, without I/O
*LCE		CMP     #HDC_SEEK,D0            ; Is it a seek?
*LCE		BEQ     SExCSetup               ;       Normal, without I/O
		CMP     #HDC_START,D0		; Is it a start/stop?
		BEQ     SExCSetup               ;       Normal, without I/O
                CMP.B   #HDC_FMTT,D0            ; Is it format track?
                BEQ.S   SExFormat               ;       Go format

*       Unsupported command.  Just return good status

                BRA     SExGoodEx               ; Return with good status

SExFormat:
                TST.W   CMD_MIDADDR(A4)         ; If not block 0,
                BNE     SExGoodEx               ;       return with good status
*LCE		AND.B	#CMD_ZLUN,CMD_LUNHIADDR(A4)
*LCE		BNE	SExGoodEx

*       Issue a SCSI "set mode" command to set block length to 512 bytes

                BSR     SExSetup        ; Set up SCSI chip, this unit

*       Tell DMA chip where to find sense command data

                LEA     SETMODEDAT,A0	        ; Address of SELECT MODE command
                MOVE.L  A0,D0                   ; SExDmaAdr wants it in D0
                LSL.L   #7,D0                   ;       shifted so
                BSET.L  #31,D0			; Indicate is a READ from memory
*LCE                BCLR.B  #HDB_512,HD_FLAGS(A6)   ; Tell SExDmaAdr UNLIMITED mode
		BSET.B	#HDB_512,HD_FLAGS(A6)	; Tell SExDmaAdr UNLIMITED mode
                BSR     SExDmaAdr               ; Ready DMA part

*       Copy Request Sense Status command block into SCSI chip

                MOVE.L  #CMD0_SIZE-1,D0         ; Moving group 0 command block
                LEA     SETMODECMD,A0           ; Point to SCSI command block
                MOVE.B  #SC_CDB1,(A3)           ; Point to Comnd. Desc. block
SFMTLp3:	MOVE.B  (A0)+,(A2)              ; Copy byte of command block
                DBRA    D0,SFMTLp3		; Loop till done

                MOVE.B  #SC_TC0,(A3)		; Point to transfer count registers
                MOVE.B	#0,(A2)			; Zero 2 most significant bytes
                MOVE.B	#0,(A2)
                MOVE.B	#12,(A2)		; Set count to 12

		MOVEQ	#0,D0
		MOVE.L  #HDF_CMDDONE,D1		; Make sure signal not already
		LINKSYS SetSignal		;       pending
                BSR     SEL_TRANS               ; Select and transfer command
                MOVE.B  #STOP_DMA,PCSS(A5)      ;       Tell DMA chip it's done
                MOVE.B  #SOFT_RESET,PCSS(A5)    ;               Yes - Reset DMA
                MOVE.B  #END_RESET,PCSS(A5)
                BSET.B  #HDB_512,HD_FLAGS(A6)   ; Tell SExDmaAdr 512 byte mode

*       This is a request to format track 0.  Change it to "FORMAT DRIVE"

                MOVE.B  #HDC_FMT,CMD_OPCODE(A4)
                MOVE.B  #1,CMD_BLOCKCNT(A4)     ; Set interleave to 1:1
                BRA.S   SExCSetup

SExWrite:
*       Set up DMA chip for output of data from specified DMA address
                MOVE.L  CMD_HIGHDMA(A4),D0 ; Get DMA address
                LSR.L   #1,D0           ; Convert BYTE address to WORD address
                BSET.L  #31,D0          ; Indicate is a READ from memory
                CMP.B   #1,CMD_BLOCKCNT(A4) ; If Not Single Block transfer,
*LCE                BNE.S   SExSetAdr       ;       Go set up DMA address
*LCE                BSET.B  #HDB_512,HD_FLAGS(A6); Else 512 byte mode
		BEQ.S	SExSetAdr	;	Go set up DMA address TEST
		BCLR.B	#HDB_512,HD_FLAGS(A6); Else Unlimited byte mode TEST!!!
                BRA.S   SExSetAdr       ;       Go set up DMA address

SExRead:
*       Set up DMA chip for input of data to specified DMA address
                MOVE.L  CMD_HIGHDMA(A4),D0 ; Get DMA address
                LSR.L   #1,D0           ; Convert BYTE address to WORD address
                CMP.B   #1,CMD_BLOCKCNT(A4) ; If Single Block transfer, TEST!!
		BEQ.S	SExSetAdr	;	Go set up DMA address TEST
		BCLR.B	#HDB_512,HD_FLAGS(A6); Else Unlimited byte mode TEST!!!
		BRA	SExSetAdr	;	Go set up DMA address

SExSetAdr:                              ; Set up DMA address and activate chip
                MOVE.L  D0,D2           ; Preserve DMA address
                BSR     SExDmaAdr       ; Set up DMA chip
                BSR     SetTransfer     ; Set Transfer Count from cmd block

*       Set up destination ID, only target LUN 0 supported now

SExCSetup:      BSR     SExSetup

*       Copy block command into SCSI chip

SExIOLp0:       MOVE.L  #CMD0_SIZE-1,D0         ; Moving group 0 command block
                MOVE.L  A4,A0                   ; Point to command block
                MOVE.B  #SC_CDB1,(A3)           ; Point to Comnd. Desc. block
SExIOLp1:       MOVE.B  (A0)+,(A2)              ; Copy byte of command block
                DBRA    D0,SExIOLp1             ; Loop till done

                CLR.L   D0
                MOVE.L  #HDF_CMDDONE,D1         ; Make sure signal no