*******************************************************************************
*
*	Source Control
*	--------------
*	$Header: scsi.asm,v 34.27 88/04/20 15:28:38 bart Exp $
*
*	$Locker:  $
*
*	$Log:	scsi.asm,v $
*   Revision 34.27  88/04/20  15:28:38  bart
*   clear dmaactiv after setmodecmd
*   
*   Revision 34.26  88/04/08  03:33:54  bart
*   test bit 6 in service check
*   
*   Revision 34.25  88/04/06  13:08:08  bart
*   stub routine to clear "service required" interrupt
*   
*   Revision 34.24  88/04/04  20:38:42  bart
*   set_timeout/get_timeout support
*   
*   Revision 34.23  88/04/03  17:31:23  bart
*   setmodecmd was zeroing lun of cmd block
*   prior to setup for format drive
*   
*   Revision 34.22  88/04/02  14:01:19  bart
*   location out of range for short branch
*   
*   Revision 34.21  88/04/02  13:47:14  bart
*   overscan support
*   
*   Revision 34.20  88/02/22  16:02:00  bart
*   get 4 bytes in statcmd
*   
*   Revision 34.19  88/02/22  15:35:21  bart
*   BadEx if not recoverable request sense
*   
*   Revision 34.18  88/02/22  14:33:00  bart
*   checkpoint
*   
*   Revision 34.17  88/02/22  14:04:26  bart
*   *** empty log message ***
*   
*   Revision 34.16  88/02/22  12:55:34  bart
*   CMP.B   #$06,D0                 ; unit attention?
*   
*   Revision 34.15  88/02/22  12:26:02  bart
*   don't assume no drive until start attempt fails
*   
*   Revision 34.14  88/02/22  11:09:00  bart
*   sense request status processing
*   
*   Revision 34.13  88/02/19  09:54:00  bart
*   scsi direct io standardisation
*   
*   Revision 34.12  88/01/21  18:05:30  bart
*   compatible with disk based / binddriver useage
*   
*   Revision 34.11  87/12/04  19:14:54  bart
*   checkpoint
*   
*   Revision 34.10  87/12/04  12:09:36  bart
*   checkpoint before adding check for existing dosname on eb_mountlist
*   
*   Revision 34.9  87/10/26  16:31:48  bart
*   checkpoint
*   
*   Revision 34.8  87/10/25  17:08:02  bart
*   must let the interrupt server know to expect scsi commands
*   
*   Revision 34.7  87/10/15  09:32:24  bart
*   10-13 rev 1
*   
*   Revision 34.6  87/10/14  14:16:51  bart
*   beginning update to cbm-source.10.13.87
*   
*   Revision 34.5  87/07/08  14:01:43  bart
*   y
*   
*   Revision 34.4  87/06/11  15:49:08  bart
*   working autoboot 06.11.87 bart
*   
*   Revision 34.3  87/06/03  11:00:11  bart
*   checkpoint
*   
*   Revision 34.2  87/05/31  16:36:33  bart
*   chickpoint
*   
*   Revision 34.1  87/05/29  19:40:10  bart
*   checkpoint
*   
*   Revision 34.0  87/05/29  17:40:40  bart
*   added to rcs for updating
*   
*
*******************************************************************************

*************************************************************************
*									*
*	Copyright (C) 1986, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


******* Included Files ***********************************************

	SECTION section
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
	XDEF    set_timeout ; Initialize SCSI timeout
	XDEF    get_timeout ; Return SCSI timeout byte

*------ Data ---------------------------------------------------------

set_timeout:	; reset SCSI chip timeout
	BTST.B  #HDB_HASSCSI,HD_FLAGS(A6)
	BEQ.S   settime_exit        ; If not set, no chip there
	CMP.B	#MIN_TIMEOUT,D0
	BGE.S	settime_ok
	MOVE.B  #MIN_TIMEOUT,D0
settime_ok:
	MOVEM.L	A5,-(SP)
	MOVE.L	HD_BASE(A6),A5		; Get controller base addr
	MOVE.B  #SC_TIMEOUT_P,SCSIADDR(A5)	; address Timeout period register
	MOVE.B  D0,SCSIIND(A5)				; D0 = TIMEOUT
	MOVEM.L (SP)+,A5
settime_exit: 
	RTS

get_timeout:	; read SCSI chip timeout
	BTST.B  #HDB_HASSCSI,HD_FLAGS(A6)
	BNE.S     gettime_ok		; If set, chip there
	MOVE.B  #0,D0
	BRA.S	gettime_exit
gettime_ok:
	MOVEM.L	A5,-(SP)
	MOVE.L	HD_BASE(A6),A5		; Get controller base addr
	MOVE.B  #SC_TIMEOUT_P,SCSIADDR(A5)	; address Timeout period register
	MOVE.B  SCSIIND(A5),D0				; D0 = TIMEOUT
	MOVEM.L (SP)+,A5
	CMP.B  	#MIN_TIMEOUT,D0
	BGE.S	gettime_exit
	MOVE.B  #MIN_TIMEOUT,D0
gettime_exit: 
	RTS

SCSIINIT:				; Initialize SCSI chip
	MOVEM.L	D0/D1/A0/A1/A5,-(SP)
	MOVE.L	HD_BASE(A6),A5		; Get controller base addr
	IFGE	INFO_LEVEL-40
	MOVE.L	A5,-(SP)
	INFOMSG	40,<'%s/SCSIInit: BASE 0x%lx'>
	ADDQ.L	#4,SP
	ENDC

	MOVE.B  #$60,HDSTAT1(A5)	; SCSI I/O, no interrupts

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

*BART	-- MUST TELL THE INTERRUPT ROUTINE TO EXPECT I/O !!!
*		BSET.B	#HDB_ACTIVE,HD_FLAGS(A6) ; Indicate this UNIT active 

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
		MOVE.B	#SCSI_TIMEOUT,D0		; TIMEOUT - 2.0 seconds
		BSR		set_timeout
        MOVE.B  #SC_STATUS,SCSIADDR(A5) ; Read SCSI Status to clear interrupt ?
        MOVE.B  SCSIIND(A5),D0
		MOVE.B  #SC_SYNC_T,SCSIADDR(A5) ; Read SCSI Status to clear interrupt ?
		MOVE.B  #$40,SCSIIND(A5)

*       Reset DMA chip

NoSCSI:
        INFOMSG 40,<'%s/SCSIInit: NoSCSI'>
        MOVE.B  #$50,HDSTAT1(A5)        ; Indicate ST 506 I/O
        BCLR.B  #HDB_SCSI,HD_FLAGS(A6)  ; Indicate SCSI inactive
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
*       D3 -- major scsi bus target address (as encoded in lun field)
*
**********************************************************************

SExIO:

                MOVEM.L D0-D3/A1-A5,-(SP)
                BTST.B  #HDB_HASSCSI,HD_FLAGS(A6)
                BEQ     SExExit                 ; If not set, no chip there
                MOVE.L  HD_BASE(A6),A5          ; Get controller base addr
                BSET.B  #HDB_SCSI,HD_FLAGS(A6)  ; Indicate SCSI active
                BCLR.B  #HDB_DMAACTIV,HD_FLAGS(A6);     Show DMA not active
                MOVE.B  #$70,HDSTAT1(A5)        ; Enable interrupts, SCSI I/O
                MOVE.L  HD_CMDPTR(A6),A4        ; Get command block address
                LEA     SCSIADDR(A5),A3         ; Point to SCSI address reg
                LEA     SCSIIND(A5),A2          ; Point to indirect SCSI reg

*       Screen for supported opcodes

SExRetry:
		MOVE.L	HD_IO_REQUEST(A6),A1	; Point to IO request	
		CMP.W	#HD_SCSI,IO_COMMAND(A1)	; If is HD_SCSI command
		BNE	RegCmd
*	This is for SCSI commands built by the task calling this device
		MOVE.L	IO_DATA(A1),A0		; Point to SCSIB structure
		MOVE.B	#0,SCSIB_STATUS(A0)	; No SCSI error yet
		BTST.B  #0,SCSIB_FLAGS(A0)  ; Set DMA direction based on bit in flags
		BNE.S	IsIN
		BCLR.B	#HDB_IN,HD_FLAGS(A6)	; Indicate DMA is OUT
		BRA.S	IsOUT
IsIN:		BSET.B	#HDB_IN,HD_FLAGS(A6)	; Indicate DMA is IN
IsOUT:		
		MOVE.B	#SOFT_RESET,PCSS(A5)	; Reset DMA
		MOVE.B	#END_RESET,PCSS(A5)
                MOVE.B  CMD_LUNHIADDR(A4),D0    ; Get LUN
                LSR.B   #CMD_LUNS,D0            ; Right justify LUN
                MOVE.B  #SC_TARGET_LUN,(A3)	; Point to TARGET_LUN
                MOVE.B  D0,(A2)                 ; Set up target lun
		MOVE.L	HD_IO_REQUEST(A6),A1	; Point to IO request	
		MOVE.L  IO_UNIT(A1),A0		; Get ID #
		MOVE.B	HDU_UNIT(A0),D0
		SUB.B	#2,D0		
                MOVE.B  #SC_DEST_ID,(A3)        ; Point to destination ID
                MOVE.B  D0,(A2)                 ; Set up target ID
		MOVE.L	IO_DATA(A1),A0		; Point to SCSIB struct
		MOVE.L	SCSIB_DATA_BUFFER(A0),D0; Get buffer address
		LSL.L	#7,D0			; SetDmaAdr wants it shifted
		BSR	SExDmaAdr		; Ready DMA part

*	Copy SCSI Command into SCSI chip
* !!!! Maybe a check that command length is less than 12 !!!!
* !!!! Maybe a check that command length is greater than 0 !!!!

		MOVE.W	SCSIB_CMD_LENGTH(A0),D0	; Get command length
		MOVE.W  D0,SCSIB_CMD_ACTUAL(A0) ; echo command length for now
		BEQ		SExGoodEx				; always good at doing nothing
		SUBQ.W	#1,D0
		MOVE.L	SCSIB_CMD(A0),A0	; Point to command itself
		MOVE.B	#SC_CDB1,(A3)		; Point to SCSI chip cmd regs.
SCSICPYLP1:	MOVE.B	(A0)+,(A2)		; Copy byte into chip
		DBRA	D0,SCSICPYLP1		; Loop until done
*	Set up SCSI chip transfer count
		MOVE.L	IO_DATA(A1),A0		; Point to SCSIB struct
		MOVE.L	SCSIB_DATA_LENGTH(A0),D0; Get buffer size
		MOVE.L	#16,D1			; Need to output high-order
		ROR.L	D1,D0			;   byte first, so rotate
		MOVE.B	#SC_TC0,(A3)		; Point to chip transfer cnt
		MOVE.B	D0,(A2)			; Output high-order byte
		ROL.L	#8,D0
		MOVE.B	D0,(A2)			; Output middle byte
		ROL.L	#8,D0
		MOVE.B	D0,(A2)			; Output low-order byte

		MOVE.L  SCSIB_DATA_LENGTH(A0),SCSIB_DATA_ACTUAL(A0) ; echo only, for now
		BEQ.S	NoScsiData		;   if not 0 show DMA as active
		BSET.B  #HDB_DMAACTIV,HD_FLAGS(A6);Show using DMA
NoScsiData:		
		BRA	DoSCSICmd

RegCmd:		CLR.B	HD_SCSI_ST_0(A6)	; Initialize working variables
		BCLR.B  #HDB_512,HD_FLAGS(A6)   ; Tell SExDmaAdr UNLIMITED mode
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
		CMP.B	#HDC_TDR,D0		; Is it "test drive ready"?
                BEQ     SExCSetup               ;       Normal, without I/O
		CMP.B   #HDC_START,D0		; Is it a start/stop?
		BEQ     SExCSetup               ;       Normal, without I/O
                CMP.B   #HDC_FMTT,D0            ; Is it format track?
                BEQ.S   SExFormat               ;       Go format

*       Unsupported command.  Just return good status

                BRA     SExGoodEx               ; Return with good status

SExFormat:
		BCLR.B	#HDB_IN,HD_FLAGS(A6)	; Indicate DMA is OUT
                TST.W   CMD_MIDADDR(A4)         ; If not block 0,
                BNE     SExGoodEx               ;       return with good status
                MOVE.B  #SOFT_RESET,PCSS(A5)	;	Reset DMA
                MOVE.B  #END_RESET,PCSS(A5)
*LCE		AND.B	#CMD_ZLUN,CMD_LUNHIADDR(A4)
*LCE		BNE	SExGoodEx

*       Issue a SCSI "set mode" command to set block length to 512 bytes

				MOVE.B	CMD_LUNHIADDR(A4),D3	; save lun
				AND.B	#~CMD_ZLUN,D3           ; and mask

                BSR     SExSetup        ; Set up SCSI chip, this unit

*       Tell DMA chip where to find sense command data

                LEA     SETMODEDAT,A0	        ; Address of SELECT MODE command
                MOVE.L  A0,D0                   ; SExDmaAdr wants it in D0
                LSL.L   #7,D0                   ;       shifted so
                BSET.L  #31,D0			; Indicate is a READ from memory
		BCLR.B	#HDB_512,HD_FLAGS(A6)	; Tell SExDmaAdr UNLIMITED mode
*LCE		BSET.B	#HDB_512,HD_FLAGS(A6)	; Tell SExDmaAdr UNLIMITED mode
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

* bart -check dmadone

dma_done_lp:	MOVE.B	#DMA_STATUS,PCSS(A5)	; Yes, Get DMA status
				BTST.B	#7,PCSD(A5)				; DMA DONE?
				BEQ.S	dma_done_lp				;	No, wait until it is
dma_out:		MOVE.B	#DMA_STATUS,PCSS(A5)	; Get DMA status
				MOVE.B	PCSD(A5),D0
				BTST.B	#DMA_ERROR_BIT,D0		; ERROR?
                BNE.S   dmaok					; No, 
dmaerror:       MOVE.B  #SOFT_RESET,PCSS(A5)    ; Yes - Reset DMA
                MOVE.B  #END_RESET,PCSS(A5)
				OR.B    D3,CMD_LUNHIADDR(A4)    ; restore target lun for retry
				BEQ	SExFormat					; Yes, retry
dmaok:

				MOVE.B	#SOFT_RESET,PCSS(A5)	; Reset DMA
				MOVE.B  #END_RESET,PCSS(A5)
                BCLR.B  #HDB_DMAACTIV,HD_FLAGS(A6); Show DMA no longer active
				OR.B    D3,CMD_LUNHIADDR(A4)    ; restore target lun for setup

*       This is a request to format track 0.  Change it to "FORMAT DRIVE"

				MOVE.B  #HDC_FMT,CMD_OPCODE(A4)
				MOVE.B  #1,CMD_BLOCKCNT(A4)     ; Set interleave to 1:1
				BRA.S   SExCSetup

SExWrite:
*       Set up DMA chip for output of data from specified DMA address
		BCLR.B	#HDB_IN,HD_FLAGS(A6); Indicate DMA is OUT
                MOVE.L  CMD_HIGHDMA(A4),D0 ; Get DMA address
                LSR.L   #1,D0           ; Convert BYTE address to WORD address
                BSET.L  #31,D0          ; Indicate is a READ from memory
				CMP.B   #1,CMD_BLOCKCNT(A4) 	; If Not Single Block transfer, 
				BNE.S	sexwmultiple
				BSET.B	#HDB_512,HD_FLAGS(A6)	; Else 512 byte mode
                BRA.S   SExSetAdr       ;       Go set up DMA address
sexwmultiple:
				BCLR.B	#HDB_512,HD_FLAGS(A6)	; Else unlimited byte mode
                BRA.S   SExSetAdr       ;       Go set up DMA address

SExRead:
*       Set up DMA chip for input of data to specified DMA address
		BSET.B	#HDB_IN,HD_FLAGS(A6); Indicate DMA is IN
                MOVE.L  CMD_HIGHDMA(A4),D0 ; Get DMA address
                LSR.L   #1,D0           ; Convert BYTE address to WORD address
				CMP.B   #1,CMD_BLOCKCNT(A4) 	; If Not Single Block transfer, 
				BNE.S	sexrmultiple
				BSET.B	#HDB_512,HD_FLAGS(A6)	; Else 512 byte mode
                BRA.S   SExSetAdr       ;       Go set up DMA address
sexrmultiple:
				BCLR.B	#HDB_512,HD_FLAGS(A6)	; Else unlimited byte mode
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

DoSCSICmd:  	CLR.L   D0
                MOVE.L  #HDF_CMDDONE,D1         ; Make sure signal not already
                LINKSYS SetSignal               ;       pending
                BSR     SEL_TRANS               ; Select and transfer command
                BTST.B  #HDB_DMAACTIV,HD_FLAGS(A6); If DMA was used
                BEQ		SExChkTgs
                BCLR.B  #HDB_DMAACTIV,HD_FLAGS(A6);     Show DMA not active
				BTST.B	#HDB_IN,HD_FLAGS(A6)	; Is DMA IN (read)?
				BEQ.S	DMA_OUT
DMA_DONE_LP:	
				BSR		SERVICE_CHK
				MOVE.B	#DMA_STATUS,PCSS(A5)	; Yes, Get DMA status
				MOVE.B	PCSD(A5),D0
				BTST.B	#DMA_ERROR_BIT,D0	; overrun?
                BEQ.S   DMAError			; yes reset dma and exit...
				BTST.B	#7,D0				; DMA DONE?
				BEQ.S	DMA_DONE_LP			; No, wait until it is
DMA_OUT:		
				BSR		SERVICE_CHK
				MOVE.B	#DMA_STATUS,PCSS(A5)	; Get DMA status
				MOVE.B	PCSD(A5),D0
				BTST.B	#DMA_ERROR_BIT,D0	; ERROR?
                BNE	   	ChkDMA512               ;       No, if 512 mode, test
DMAError:       MOVE.B  #SOFT_RESET,PCSS(A5)    ;               Yes - Reset DMA
                MOVE.B  #END_RESET,PCSS(A5)
                MOVE.B  #$32,CMD_ERRORBITS(A4)  ;                 Show DMA error
                BRA     SExBadEx                ;                       and exit

SExSetup:
                MOVE.B  CMD_LUNHIADDR(A4),D0    ; Get LUN
                LSR.B   #CMD_LUNS,D0            ; Right justify LUN
                MOVE.B  #SC_DEST_ID,(A3)        ; Point to destination ID
                MOVE.B  D0,(A2)                 ; Set up target ID
                AND.B   #CMD_ZLUN,CMD_LUNHIADDR(A4);Zero LUN field
                MOVE.B  #SC_TARGET_LUN,(A3)     ; Point to TARGET LUN
                MOVE.B	#0,(A2)			;       and zero it
                RTS

PHASE_CHK:
				MOVE.B  #SC_CMD_PHASE,(A3)      ; See if the command complete
                MOVE.B  (A2),D1                 ;       Get phase
				MOVE.B  #SC_CMD_PHASE,(A3)		; Check phase again
				CMP.B	(A2),D1					; 		phase still changing?
				BNE		PHASE_CHK				; 		yes, status invalid
				RTS
SERVICE_CHK:					
				BSR	PHASE_CHK
                CMP.B   #$60,D1                 ; Command complete?
                BEQ		SERVICE_DONE				;       yes , check dma
                MOVE.B  #SC_STATUS,(A3)         ; Get SCSI_STATUS
                MOVE.B  (A2),D1                 ; 
				BTST.B	#7,D1					; service another device?
				BNE.S	SERVICE					; yes
				BTST.B  #6,D1					; no, terminated ?
				BNE.S	TERMINATE				; yes
				BRA		SERVICE_DONE			; 
SERVICE:
                MOVE.B  #SC_COMMAND,(A3)        ; Point to command register
                MOVE.B  #ABORT_CMD,(A2)    		; abort data transfer
                BSR     SEL_WAIT_LP	            ; Wait for command complete
				BSR	PHASE_CHK
                CMP.B   #$60,D1                 ; Command complete?
                BEQ		SERVICE_DONE				;       yes , check dma
                MOVE.B  #SC_STATUS,(A3)         ; Get SCSI_STATUS
                MOVE.B  (A2),D1                 ; 
                MOVE.B  #SC_COMMAND,(A3)        ; Point to command register
                MOVE.B  #TRANS_PAD_CMD,(A2)    	; service target
                BSR     SEL_WAIT_LP             ; Wait for command complete
				BSR	PHASE_CHK
                CMP.B   #$60,D1                 ; Command complete?
                BEQ		SERVICE_DONE				;       yes , check dma
                MOVE.B  #SC_STATUS,(A3)         ; Get SCSI_STATUS
                MOVE.B  (A2),D1                 ; 
                MOVE.B  #SC_COMMAND,(A3)        ; Point to command register
                MOVE.B  #NEG_ACK_CMD,(A2)    	; accept transfer
                BSR     SEL_WAIT_LP             ; Wait for command complete
				BSR	PHASE_CHK
                CMP.B   #$60,D1                 ; Command complete?
                BEQ		SERVICE_DONE				;       yes , check dma
                MOVE.B  #SC_STATUS,(A3)         ; Get SCSI_STATUS
                MOVE.B  (A2),D1                 ; 
				ADDQ.L	#4,SP					; pop return address
				PEA.L	DMAError(PC)			; reset dma and exit
SERVICE_DONE:
				RTS

TERMINATE:
                MOVE.B  #SOFT_RESET,PCSS(A5)    ; reset dma
                MOVE.B  #END_RESET,PCSS(A5)		; neg ack if necessary
				ADDQ.L	#4,SP					; pop return address
				PEA.L	SNegAck(PC)				; and return hard error
TERMINATE_DONE:
				RTS

ChkDMA512:
                BTST.B  #HDB_512,HD_FLAGS(A6)   ;       In 512 byte mode ?
                BEQ.S   SExChkTgs               ;               No - chk target
				BTST.B	#HDB_IN,HD_FLAGS(A6)	; Is DMA OUT (write)?
				BNE.S	Chk_7			;	No - skip test
				BTST.B	#6,D0			;	Yes - test bit 6
				BNE		DMAError
Chk_7:			BTST.B	#7,D0
				BEQ		DMAError
SExChkTgs:
                MOVE.B  #SOFT_RESET,PCSS(A5)    ; Reset DMA
                MOVE.B  #END_RESET,PCSS(A5)
ExGtStat:		MOVE.B  #SC_CMD_PHASE,(A3)      ; See if the command complete
                MOVE.B  (A2),D1                 ;       Get phase
				MOVE.B  #SC_CMD_PHASE,(A3)	; See if the command complete
				CMP.B	(A2),D1
				BNE.S	ExGtStat
                CMP.B   #$60,D1                 ; Select & transfer complete ?
                BEQ	SExChkSt                ;       Yes - check status
				MOVE.B  D1,HD_SCSI_ST_0(A6) ;       No - return phase
                MOVE.B  #$F1,CMD_ERRORBITS(A4)  ;       
                MOVE.B  #SC_STATUS,(A3)         ; Get STATUS
                MOVE.B  (A2),D1
				CMP.B	#$42,D1			; If TIMED OUT
				BNE.S	NotTimedOut
				MOVE.B	#$FE,CMD_ERRORBITS(A4)	;	Return "FE" error
				BRA	SExBadEx
NotTimedOut:	BTST.B  #3,D1                   ; If not "unexpected phase"
                BEQ     SExBadEx                ;       exit
                MOVE.B  #$F2,CMD_ERRORBITS(A4)  ;       
                BTST.B  #0,D1                   ; If not "input"
                BEQ     SExBadEx                ;       exit
BExLoop:        MOVE.B  #SC_STATUS,(A3)         ; Get STATUS
                MOVE.B  (A2),D1
                BTST.B  #3,D1                   ; If not "unexpected phase"
                BEQ     SNegAck                 ;       exit
                BTST.B  #0,D1                   ; If not "input"
                BEQ     SNegAck                 ;       exit
				MOVEQ	#0,D0
				MOVE.L  #HDF_CMDDONE,D1			; Make sure signal not already
				LINKSYS SetSignal				;       pending
                MOVE.B  #SC_COMMAND,(A3)        ; Point to command register
*BART	-- MUST TELL THE INTERRUPT ROUTINE TO EXPECT I/O !!!
*				BSET.B	#HDB_ACTIVE,HD_FLAGS(A6) ; Indicate this UNIT active 
                MOVE.B  #TRANS_PAD_CMD,(A2)     ; TRANSFER PAD
                BSR     SEL_WAIT                ; Wait for command complete
                BRA.S   BExLoop
SNegAck:
                MOVE.B  #$F3,CMD_ERRORBITS(A4)  ;       
                MOVE.B  #SC_STATUS,(A3)         ; Get STATUS
                MOVE.B  (A2),D1                 ; If "Message-in phase
                CMP.B   #$20,D1                 ;   paused with ACK asserted"
                BNE     SExBadEx
				MOVEQ	#0,D0
				MOVE.L  #HDF_CMDDONE,D1		; Make sure signal not already
				LINKSYS SetSignal		;       pending
                MOVE.B  #SC_COMMAND,(A3)        ; Point to COMMAND register
                MOVE.B  #NEG_ACK_CMD,(A2)       ;       Negate ACK
                MOVE.L  #HDF_CMDDONE,D0		; Wait for interrupt from
                LINKSYS Wait                    ;       the controller
SNegAckLp0:
		BTST.B	#5,SCSIAUX(A5)		; See if chip still busy
		BNE.S	SNegAckLp0		;	Wait if it is
                MOVE.B  #$F4,CMD_ERRORBITS(A4)  ;       

*				some confusion here as to whether this is really an errror
                BRA     SExBadEx                ;       exit

SExChkSt:
                MOVE.B  #SC_TARGET_LUN,(A3)	; Get unit's status
                MOVE.B  (A2),D1
		MOVE.B  D1,HD_SCSI_ST_1(A6)
		MOVE.B	#SC_TARGET_LUN,(A3)	; Get unit's status
		CMP.B	(A2),D1
		BNE.S	SExChkSt
		MOVE.L	HD_IO_REQUEST(A6),A1	; Point to IO request	
		CMP.W	#HD_SCSI,IO_COMMAND(A1)	; If is HD_SCSI command
		BNE.S	ChkFurther
		MOVE.L	IO_DATA(A1),A0		; Point to SCSIB structure
		MOVE.B	D1,SCSIB_STATUS(A0)	; Save SCSI status
		TST.B   D1                      ; If no error,
		BEQ SExChkParity                ; Check for parity error
		BRA SExExit
ChkFurther:
		TST.B   D1                      ; If no error,
		BEQ SExChkParity                ; Check for parity error
ChkTestDrvRdy:
		CMP.B	#HDC_TDR,CMD_OPCODE(A4) ; If "test drive ready"
		BNE.S	SExTgtErr

		AND.B	#$3E,D1					; mask status byte
		CMP.B	#$02,D1					; check condition?
		BEQ.S	SExTgtErr

		MOVE.B	#$F8,CMD_ERRORBITS(A4)	;	Indicate "not ready"
		BRA	SExBadEx

SExTgtErr:      ;       Need to get the status from the target device

*       Tell DMA chip to store status in command block

				LEA     HD_SCSI_ST_2(A6),A0 ; Address of where status goes
                MOVE.L  A0,D0                   ; SExDmaAdr wants it in D0
                LSL.L   #7,D0                   ;       shifted so
                BCLR.B	#HDB_512,HD_FLAGS(A6)   ; Tell SExDmaAdr UNLIMITED mode
*	            BSET.B	#HDB_512,HD_FLAGS(A6)   ; Tell SExDmaAdr 512 mode
                BSR     SExDmaAdr               ; Ready DMA part

*       Copy Request Sense Status command block into SCSI chip

                MOVE.B  #$F5,CMD_ERRORBITS(A4)  ;       
                MOVE.L  #CMD0_SIZE-1,D0         ; Moving group 0 command block
                LEA     GetStatCMD(PC),A0		; Point to SCSI command block
                MOVE.B  #SC_CDB1,(A3)           ; Point to Comnd. Desc. block
SExIOLp3:       MOVE.B  (A0)+,(A2)              ; Copy byte of command block
                DBRA    D0,SExIOLp3             ; Loop till done

                MOVE.B  #SC_TC0,(A3)    ; Point to transfer count registers
                MOVE.B	#0,(A2)		;       Zero 2 most significant bytes
                MOVE.B	#0,(A2)
                MOVE.B  #4,(A2)         ;       Set count to 4

				MOVEQ	#0,D0
				MOVE.L  #HDF_CMDDONE,D1		; Make sure signal not already
				LINKSYS SetSignal		;       pending
                BSR     SEL_TRANS               ; Select and transfer command

SExIOLp4:	BTST.B	#7,PCSD(A5)		; Wait for FIFO empty
                BEQ.S   SExIOLp4
                MOVE.B  #SOFT_RESET,PCSS(A5)    ; Yes - Reset DMA
                MOVE.B  #END_RESET,PCSS(A5)

				MOVE.B	HD_SCSI_ST_2(A6),D0		; error class
				MOVE.B	D0,D1					; and error code
				AND.B	#$70,D0
				CMP.B	#$70,D0					; extended sense?
				BNE		SExBadEx				; no
				AND.B	#$0F,D1
				CMP.B	#$00,D1					; extended sense data format?
				BNE		SExBadEx				; no
				MOVE.B	HD_SCSI_ST_4(A6),D0		; get sense info
				AND.B	#$0F,D0
				CMP.B	#$02,D0					; recoverable?
				BLT.S	SExChkParity			; yes, continue 
				CMP.B   #$06,D0                 ; unit attention?
				BNE     SExBadEx                ; hard error
not_ready:
				MOVE.B  #$F8,CMD_ERRORBITS(A4)  ; Indicate "not ready"
				BRA     SExBadEx                ; return error

*		CMP.B	#1,CMD_RSVD3(A4)	; What was this for?
*		BGT	SExBadEx
*       this was a primitive test for extended sense data which was incorrect

SExChkParity:
                IFD     SPARITY
                BTST.B  #TGT_PARITY_ERR,HD_SCSI_ST_0(A6); Parity error?
                BNE.S   SExParErr               ;       Yes - indicate it
                BTST.B  #1,SCSIAUX(A5)          ; SCSI Parity error?
                BEQ.S   SExGoodEx               ;       No - Good I/O
SExParErr:      MOVE.B  #$FD,CMD_ERRORBITS(A4)  ;       Yes - Return "FD" error
                ENDC                            ; SPARITY
                IFND    SPARITY
                BRA.S   SExGoodEx               ; Don't test parity, good exit
                ENDC                            ; NOT SPARITY
SExBadEx:       ;       Error detected, indicate 1st block in transfer bad
                MOVE.L  CMD_OPCODE(A4),D0       ; Get starting address
                ROL.L   #8,D0                   ; Replace opcode with
                MOVE.B  CMD_ERRORBITS(A4),D0    ;       error code
                ROR.L   #8,D0
                MOVE.L  D0,CMD_ERRORBITS(A4)
                BRA.S   SExExit

SExGoodEx:      ;       No error detected, indicate all blocks transferred
                MOVE.L  CMD_OPCODE(A4),D0       ; Get starting address
                SUBQ.L  #1,D0
                MOVEQ.L #0,D1
                MOVE.B  CMD_BLOCKCNT(A4),D1     ; Add in # of blocks moved - 1
                ADD.L   D1,D0
                MOVE.L  D0,CMD_ERRORBITS(A4)    ; Save updated block #
                MOVE.B  #$80,CMD_ERRORBITS(A4)  ; Set status to success

SExExit:
		MOVE.L  HD_IO_REQUEST(A6),A1    ; Point to IO request
		CMP.W   #HD_SCSI,IO_COMMAND(A1) ; If isn't HD_SCSI command
		BNE.S   SExExit2        ;   just exit
							; Else
		MOVE.L  IO_DATA(A1),A0      ; Point to SCSIB structure
							; Copy status
*							; will copy status to io_error upon return
*		MOVE.B  CMD_ERRORBITS(A4),SCSIB_CTLR_STATUS(A0)

SExExit2:   	MOVEQ   #0,D0
				MOVE.L  #HDF_CMDDONE,D1		; Make sure signal not already
				LINKSYS SetSignal		;       pending
                BCLR.B  #HDB_SCSI,HD_FLAGS(A6)  ; Indicate SCSI inactive
                MOVE.B  #$50,HDSTAT1(A5)        ; Indicate ST 506 I/O
                MOVEM.L (SP)+,D0-D3/A1-A5

                RTS

SExDmaAdr:                              ; Output address in D0 to DMA chip
                ROL.L   #8,D0           ; Put High order address in low byte
                MOVE.B  #LD2,PCSS(A5)   ; Output to DMA chip
                MOVE.B  D0,PCSD(A5)
                ROL.L   #8,D0           ; Put middle address in low byte
                MOVE.B  #LD1,PCSS(A5)   ; Output to DMA chip
                MOVE.B  D0,PCSD(A5)
                ROL.L   #8,D0           ; Put Low order address in low byte
                BTST.B  #HDB_512,HD_FLAGS(A6)
                BEQ.S   SExDmaJp1
                MOVE.B  #LD0512,PCSS(A5);       Use 512 byte only mode
                BRA.S   SExDmaJp2
SExDmaJp1:                              ; Else
                MOVE.B  #LD0,PCSS(A5)   ;       Use unlimited mode
SExDmaJp2:
                MOVE.B  D0,PCSD(A5)
                MOVE.B  #START_DMA,PCSS(A5)     ; Tell chip to start DMA
                BSET.B  #HDB_DMAACTIV,HD_FLAGS(A6); Show DMA chip active
                RTS

*       Set up transfer count in SCSI chip

SetTransfer:    CLR.L   D0
                MOVE.B  CMD_BLOCKCNT(A4),D0     ; Get block count
                MOVE.L  #25,D1                  ; Multiply by 512, and have
                ROL.L   D1,D0                   ;       most significant byte
                MOVE.L  #2,D1                   ;       to output first
OutTCount:      MOVE.B  #SC_TC0,(A3)            ; Point to Transfer Count reg.
SExLp2:         MOVE.B  D0,(A2)                 ; Output byte of transfer count
                ROL.L   #8,D0                   ; Get next byte
                DBRA    D1,SExLp2               ; Do for 3 bytes
                RTS

                IFD     JUNK                    ; No longer used
*       Set SCSI chip transfer count to 512 bytes

Transfer512:    MOVE.L  #$020000,D0
                MOVE.L  #2,D1
                MOVE.B  #SC_TC0,(A3)            ; Point to Transfer Count reg.
T512Lp:         MOVE.B  D0,(A2)                 ; Output byte of transfer count
                ROL.L   #8,D0                   ; Get next byte
                DBRA    D1,T512Lp               ; Do for 3 bytes
                RTS
                ENDC                    ; JUNK

ZeroTC:         MOVE.B  #SC_TC0,(A3)    ; Point to transfer count registers
                MOVE.B	#0,(A2)		;       and zero them
                MOVE.B	#0,(A2)
                MOVE.B	#0,(A2)
                RTS

SEL_TRANS:                              ; Select and transfer command
                MOVE.B  #SC_COMMAND,(A3)        ; Point to command register
                BTST.B  #HDB_MSGING,HD_FLAGS(A6) ; !!!If devices support messaging
                BEQ.S   No_Messaging
                MOVE.B  #ST_WITH_ATN_CMD,(A2)   ;       DO IT with messaging!
                BRA.S   SEL_WAIT
No_Messaging:   
				MOVE.B  #ST_WO_ATN_CMD,(A2)     ;       DO IT without messaging!
SEL_WAIT:
				MOVE.L  #HDF_CMDDONE,D0         ; Wait for interrupt from
                LINKSYS Wait                    ;       the controller
SEL_WAIT_LP:
				MOVE.B	SCSIAUX(A5),D0
				BTST.B	#5,D0					; See if chip still busy
				BNE.S	SEL_WAIT_LP			;	Wait if it is
                RTS

GetStatCMD:     DC.B    3      				; SCSI command to get sense status
                DC.B    0
                DC.B    0
                DC.B    0
                DC.B    4					; "everybody" supports four
                DC.B    0

SETMODECMD:     DC.B    SCSI_SET_MODE
                DC.B    1               	; Save mode on disk
                DC.B    0
                DC.B    0
                DC.B    12
                DC.B    0
SETMODEDAT:     DC.B    0
                DC.B    0
                DC.B    0
                DC.B    8
                DC.B    0
                DC.B    0
                DC.B    0
                DC.B    0
                DC.B    0
                DC.B    0                    ; 512 byte blocks
                DC.B    2
                DC.B    0
PAGE_INFO:
				DC.B    0
				DC.B    0
	        ENDC    ; HASSCSI

        END
