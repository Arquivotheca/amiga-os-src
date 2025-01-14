
*************************************************************************
*									*
*	Copyright (C) 1986, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* io.asm
*
* Source Control
* ------ -------
* 
* $Header: io.asm,v 30.2 86/01/20 00:36:39 LCE Exp $
*
* $Locker:  $
*
* $Log:	io.asm,v $
* *** empty log message ***
* 
* 
*************************************************************************

******* Included Files ***********************************************

	NOLIST
	INCLUDE 'exec/types.i'
 	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/io.i'
	INCLUDE 'exec/devices.i'
	INCLUDE 'exec/tasks.i'
	INCLUDE 'exec/memory.i'
	INCLUDE 'exec/execbase.i'
	INCLUDE 'exec/ables.i'
	INCLUDE 'exec/strings.i'

	INCLUDE 'hddisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'
	LIST



	SECTION section


******* Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

	XREF	hdName

*------ Functions ----------------------------------------------------

	EXTERN_LIB Wait
	EXTERN_LIB SetSignal
	IFD	HASSCSI
	XREF	SExIO		; SCSI I/O defined in SCSI.ASM
	ENDC
	XDEF	ExIO		; KONAN I/O defined here
	IFD	BufWrites
	XREF	PutBuffer
	ENDC
	XREF	HDMotor
	XREF	HDDelay
	XREF	HDSeek
	XREF	TermIO
	XREF	BadTrans

*------ System Library Functions -------------------------------------

******* Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	IFND	BufferIO
	XDEF	HDIO
	ENDC	; BufferIO
	XDEF	HDUClear
	XDEF	HDUMotor
	XDEF	HDUSeek
	XDEF	HDUUpdate
	XDEF	HDURemove
	XDEF	HDUChangeNum
	XDEF	HDUChangeState
	XDEF	HDUProtStatus
	XDEF	HDUSpecial
	XDEF	HDUSCSI
	XDEF	HDUMovCmdBlk

*------ Data ---------------------------------------------------------


******* Local Definitions ********************************************

INFO_LEVEL SET 0

	IFND	BufferIO
******* System/Drivers/HD/HDIO ***************************************
*
*   NAME
*	HDIO - basic loop for track disk driver
*
*   SYNOPSIS
*	HDIO( IOBlock ), DevPtr
*	      A1	 A6
*
*   FUNCTION
*	This routine performs the basic IO for the track disk.	It
*	is given the IOBlock to do actions on.	The IOBlock will be
*	marked done when the IO is complete, and the process signaled.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
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
*	A6 -- DevPtr
*	A3 -- unit data
*	A4 -- device command block
*	A2 -- IOBlock
*

HDIO:

		MOVEM.L D2/D3/A2/A3/A4,-(SP)

		MOVE.L	HD_CMDPTR(A6),A4	; Point to device command block
		MOVE.L	IO_UNIT(A1),A3		; Get unit pointer
		CLR.B	HDU_MOTOR(A3)		; Show "Motor" being "ON"
		MOVE.W	#MAXRETRY,D3		; Retry DMA errors MAXRETRY times

		MOVE.L	A1,A2
		MOVE.L	A2,HD_IO_REQUEST(A6)

		IFGE	INFO_LEVEL-50
		PUTMSG	50,<'%s/HDIO: Entered:'>
		MOVE.L	IO_DEVICE(A2),-(SP)
		PUTMSG	50,<'%s/HDIO: IO_DEVICE %lx'>
		ADDQ.L	#4,SP
		MOVE.L	IO_UNIT(A2),-(SP)
		PUTMSG	50,<'%s/HDIO: IO_UNIT %lx'>
		ADDQ.L	#4,SP
		MOVE.W	IO_COMMAND(A2),-(SP)
		PUTMSG	50,<'%s/HDIO: IO_COMMAND %x'>
		ADDQ.L	#2,SP
		MOVE.W	IO_FLAGS(A2),-(SP)
		PUTMSG	50,<'%s/HDIO: IO_FLAGS,IO_ERROR %x'>
		ADDQ.L	#2,SP
		ENDC

*		;-- {state 0} check operation for legality
		MOVE.L	IO_OFFSET(A2),D0
		MOVE.L	D0,D1

*		;-- check for being an even sector boundary
		AND.L	#HD_SECTOR-1,D1
		BNE	HDIO_Err

*		;-- check for IO within disc range
		CMP.L	HDU_LAST(A3),D0
		BGT	HDIO_Err

		MOVE.L	IO_DATA(A2),D0		; Initialize working variables
		MOVE.L	D0,HD_DMA(A6)		; Starting DMA address
		MOVE.L	#0,IO_ACTUAL(A2)	; Show no data move yet
		MOVE.L	IO_LENGTH(A2),D1	; Get byte length
		MOVEQ	#HD_SECSHIFT,D0		; Convert byte offset to block #
		LSR.L	D0,D1
		MOVE.L	D1,HD_BLOCKCNT(A6)	; Total # of blocks to move

		MOVE.L	IO_OFFSET(A2),D1	; Convert byte count to blocks
		LSR.L	D0,D1
DONT_SKIP:	MOVE.L	D1,HD_LBLOCK(A6)	; Starting logical block #
		MOVE.L	HD_IO_REQUEST(A6),A2
		MOVE.W	IO_COMMAND(A2),D0
		CMP.B	#CMD_READ,D0
		BEQ.S	HDIO_Up20

*************************************************************************
*									*
* 			WE ARE DOING A WRITE				*
*									*
*************************************************************************

		MOVE.B	#$0A,CMD_OPCODE(A4)	; Set up the write command
		BRA.S	HDIO_IO

HDIO_Up20:
*************************************************************************
*									*
* 			WE ARE DOING A READ				*
*									*
*************************************************************************

		MOVE.B	#$08,CMD_OPCODE(A4)	; Set up the read command

HDIO_IO:
		MOVE.B	#$FF,CMD_ERRORBITS(A4)	; Tell controller new command

		LEA.L	HD_DMA+1(A6),A0		; Set up DMA address
		LEA.L	CMD_HIGHDMA(A4),A1	; Point to command block
		MOVE.B	(A0)+,(A1)+		; Copy the DMA address
		MOVE.B	(A0)+,(A1)+
		MOVE.B	(A0)+,(A1)+

		MOVE.L	HD_LBLOCK(A6),D1	; Get starting logical block #
		MOVE.L	D1,HD_PBLOCK(A6)	; Physical usually = logical
		BSR	BadTrans		; Check for bad blocks
		CMP.L	HD_LBLOCK(A6),D1	; Is 1st block bad?
		BEQ.S	FirstGood		;	No - same as physical
		MOVE.L	D1,HD_PBLOCK(A6)	;	Yes - save mapped block
		MOVE.B	#1,CMD_BLOCKCNT(A4)	;	     Just 1 sect I/O
		BRA.S	MovBlkNo		;	     Go move block #
FirstGood:	MOVE.L	(A0),D0			; Get next bad block #
		SUB.L	D1,D0			; # of blocks to bad block
		MOVE.L	HD_BLOCKCNT(A6),D2	; Get desired # of blocks
		CMP.L	#255,D2			; More than 255 blocks?
		BLT.S	LT255
		MOVE.L	#255,D2			;	Yes - only do 255
LT255:		CMP.L	D0,D2			; Bad blk before end of request?
		BLE.S	NoBad			;	No - do all of them
		MOVE.B	D0,D2			;	Yes - only do up to bad
NoBad:		MOVE.B	D2,CMD_BLOCKCNT(A4)	; Save # of blocks to I/O

MovBlkNo:	LEA.L	HD_PBLOCK+1(A6),A0	; Set up starting block number
		LEA.L	CMD_LUNHIADDR(A4),A1	; Point to command block
		MOVE.B	(A0)+,(A1)+		; Copy the block number
		MOVE.B	(A0)+,(A1)+
		MOVE.B	(A0)+,(A1)+

		MOVE.B	HDU_UNITNUM(A3),D0	; Add in Unit number
		OR.B	D0,CMD_LUNHIADDR(A4)
		
		IFGE	INFO_LEVEL-50
		MOVE.L	$0(A4),-(SP)
		PUTMSG	50,<'%s/HDIO: CMDBLK 0-3 %lx'>
		ADDQ.L	#4,SP
		MOVE.L	$4(A4),-(SP)
		PUTMSG	50,<'%s/HDIO: CMDBLK 4-7 %lx'>
		ADDQ.L	#4,SP
		MOVE.L	$8(A4),-(SP)
		PUTMSG	50,<'%s/HDIO: CMDBLK 8-B %lx'>
		ADDQ.L	#4,SP
		MOVE.L	$C(A4),-(SP)
		PUTMSG	50,<'%s/HDIO: CMDBLK C-F %lx'>
		ADDQ.L	#4,SP
		ENDC

		BSR	ExIO			; Do the actual I/O

*		Test for error

		MOVE.B	CMD_ERRORBITS(A4),D0	; Get controller status
		AND.B	#$7F,D0			; Clear high order bit
		BEQ.S	HDIO_CLen		; If no, error, jump
		CMP.B	#$18,D0			; Is it fixed ECC error ?
		BEQ.S	HDIO_CLen		;	Yes - Ignore
		CMP.B	#$22,D0			; Is it soft read header error ?
		BEQ.S	HDIO_CLen		;	Yes - Ignore
		CMP.B	#$23,D0			; Is it soft write header err ?
		BEQ.S	HDIO_CLen		;	Yes - Ignore
		CMP.B	#$24,D0			; Is it soft data error ?
		BEQ.S	HDIO_CLen		;	Yes - Ignore
		CMP.B	#$28,D0			; Is it soft DMA error ?
		BEQ.S	1$			; yes, count down
		CMP.B	#$32,D0			; Is it a DMA error ?
		BNE	HDIO_ErrSkp		;	No - Error
		IFGE	INFO_LEVEL-50
		PUTMSG	0,<'%s/      io DMA Error detected:'>
		ENDC
1$:		SUBQ.B	#1,D3			; Retry DMA errors MAXRETRY times
		BNE	HDIO_IO
HDIO_ErrSkp:	CLR.B	D3			; LSB D3 = 0 means error

*		;------ compute the "# of bytes transferred"

HDIO_CLen:	MOVE.L	CMD_ERRORBITS(A4),D0	; Get ending block #
		AND.L	#$001FFFFF,D0		; Clear status and LUN bits
		ADDQ.L	#1,D0			; Compute how many bytes moved
		MOVE.L	CMD_OPCODE(A4),D1	;	Get starting block #
		AND.L	#$001FFFFF,D1		;	Clear opcode & LUN
		SUB.L	D1,D0			;	Get # blocks moved
		MOVE.L	D0,D2			;	Save block count
		MOVEQ.L	#HD_SECSHIFT,D1		; 	Convert to byte count
		LSL.L	D1,D0
		ADD.L	D0,IO_ACTUAL(A2)	; Update # bytes moved
		TST.B	D3			; Fatal I/O error?
		BEQ	HDIO_Err		;	Yes - error exit

		ADD.L	D0,HD_DMA(A6)		; Bump DMA address
		ADD.L	D2,HD_LBLOCK(A6)	; Update logical block #
		SUB.L	D2,HD_BLOCKCNT(A6)	; Update # of blocks to move
		BGT	HDIO_IO			; Loop until done or error

*		;-- {state 5} mark IOBLOCK as done
HDIO_Signal


		IFGE	INFO_LEVEL-50
		PUTMSG	0,<'%s/HDIO: Exit:'>
		MOVE.L	IO_DEVICE(A2),-(SP)
		PUTMSG	0,<'%s/HDIO: IO_DEVICE %lx'>
		ADDQ.L	#4,SP
		MOVE.L	IO_UNIT(A2),-(SP)
		PUTMSG	0,<'%s/HDIO: IO_UNIT %lx'>
		ADDQ.L	#4,SP
		MOVE.W	IO_COMMAND(A2),-(SP)
		PUTMSG	0,<'%s/HDIO: IO_COMMAND %x'>
		ADDQ.L	#2,SP
		MOVE.W	IO_FLAGS(A2),-(SP)
		PUTMSG	0,<'%s/HDIO: IO_FLAGS,IO_ERROR %x'>
		ADDQ.L	#2,SP
		MOVE.L	IO_ACTUAL(A2),-(SP)
		PUTMSG	0,<'%s/HDIO: IO_ACTUAL %lx'>
		ADDQ.L	#4,SP
		MOVE.L	IO_OFFSET(A2),-(SP)
		PUTMSG	0,<'%s/HDIO: IO_OFFSET %lx'>
		ADDQ.L	#4,SP
		ENDC

		MOVE.L	A2,A1
		BSR	TermIO

HDIO_End:
		MOVEM.L (SP)+,D2/D3/A2/A3/A4
		RTS

HDIO_Err:
		IFGE	INFO_LEVEL-50
		PUTMSG	0,<'%s/      IO Error detected:'>
		MOVE.L	$0(A4),-(SP)
		PUTMSG	0,<'%s/HDIO: CMDBLK 0-3 %lx'>
		ADDQ.L	#4,SP
		MOVE.L	$4(A4),-(SP)
		PUTMSG	0,<'%s/HDIO: CMDBLK 4-7 %lx'>
		ADDQ.L	#4,SP
		MOVE.L	$8(A4),-(SP)
		PUTMSG	0,<'%s/HDIO: CMDBLK 8-B %lx'>
		ADDQ.L	#4,SP
		MOVE.L	$C(A4),-(SP)
		PUTMSG	0,<'%s/HDIO: CMDBLK C-F %lx'>
		ADDQ.L	#4,SP
		ENDC

		MOVE.B	#-1,IO_ERROR(A2)
		BRA	HDIO_Signal
		ENDC	; BufferIO


******* System/Drivers/HD/HDUMovCmdBlk ***************************************
*
*   NAME
*	HDUMovCmdBlk - Move the controller command block from its default
*		      address to the area in the device structure
*
*   SYNOPSIS
*	HDUMovCmdBlk( IOBlock ), DevPtr
*		      A1	 A6
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
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
*	A6 -- DevPtr
*	A2 -- IOBlock
*

HDUMovCmdBlk:

		IFND	HANDSHAKE
		MOVEM.L A2/A3/A4/A5,-(SP)

		MOVE.L	HD_CMDPTR(A6),A4	; Point to device command block

		MOVE.L	A1,A2
		MOVE.L	A2,HD_IO_REQUEST(A6)

		PUTMSG	50,<'%s/HDUMovCmdBlk: Entered:'>

*	Initialize command block at default address to move the command block

		MOVE.W	#$0F00,HD_INIT_CMD+CMD_OPCODE ; "Move Cmd Blk" command
		MOVE.L	#HD_INIT_CMD+$200,A1	; Tell controller new address
		MOVE.L	HD_CMDPTR(A6),(A1)
		MOVE.L	A1,D0
		MOVE.B	D0,HD_INIT_CMD+CMD_LOWDMA
		ROR.L	#8,D0
		MOVE.B	D0,HD_INIT_CMD+CMD_MIDDMA
		ROR.L	#8,D0
		MOVE.B	D0,HD_INIT_CMD+CMD_HIGHDMA
		MOVE.B	#$FF,HD_INIT_CMD+CMD_ERRORBITS

		BSR	ExIO			; Do the I/O
		ENDC
		BRA	HDUSp_exit		; Mark complete and exit

******* System/Drivers/HD/HDUSpecial ***************************************
*
*   NAME
*	HDUSpecial - Pass the command block pointed to by IO_DATA
*		    directly to the controller
*
*   SYNOPSIS
*	HDUSpecial( IOBlock ), DevPtr
*		      A1	 A6
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
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
*	A6 -- DevPtr
*	A2 -- IOBlock
*

HDUSpecial:

		MOVEM.L A2/A3/A4/A5,-(SP)

		MOVE.L	HD_CMDPTR(A6),A4	; Point to device command block

		MOVE.L	A1,A2
		MOVE.L	A2,HD_IO_REQUEST(A6)
		MOVE.L	IO_UNIT(A2),A3		; Get unit pointer

*		PUTMSG	50,<'%s/HDUSpecial: Entered:'>

		MOVE.L	IO_DATA(A2),A0		; Get passed command block addr
		MOVE.L	A4,A1			; Point to actual command blk
		MOVE.L	(A0)+,(A1)+		; Copy the command block
		MOVE.L	(A0)+,(A1)+
		MOVE.L	(A0)+,(A1)+
		MOVE.L	(A0)+,(A1)+

		BSR	ExIO			; Do the I/O

*	Copy command block back to caller's IO_DATA area

		MOVE.L	IO_DATA(A2),A1		; Get passed command block addr
		MOVE.L	A4,A0			; Point to actual command blk
		MOVE.L	(A0)+,(A1)+		; Copy the command block
		MOVE.L	(A0)+,(A1)+
		MOVE.L	(A0)+,(A1)+
		MOVE.L	(A0)+,(A1)+

HDUSp_exit:
		MOVE.L	HD_IO_REQUEST(A6),A1
		BSR	TermIO
		MOVEM.L (SP)+,A2/A3/A4/A5
		RTS

******* System/Drivers/HD/HDUSCSI ***************************************
*
*   NAME
*	HDUSCSI - Pass a SCSI command block pointed to by IO_DATA
*		    directly to the SCSI code.
*
*   SYNOPSIS
*	HDUSCSI( IOBlock ), DevPtr
*		      A1	 A6
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
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
*	A6 -- DevPtr
*	A2 -- IOBlock
*

HDUSCSI:

		MOVEM.L A2/A3/A4/A5,-(SP)

		MOVE.L	HD_CMDPTR(A6),A4	; Point to device command block

		MOVE.L	A1,A2
		MOVE.L	A2,HD_IO_REQUEST(A6)
		MOVE.L	IO_UNIT(A2),A3		; Get unit pointer
		MOVE.B	#MAXRETRY,HDU_RETRY(A3)	; Set retry counter

		PUTMSG	50,<'%s/HDUSCSI: Entered:'>

HDUSCSILP1:	BSR	ExIO			; Do the I/O
		CMP.B	#$32,CMD_ERRORBITS(A4)	; If DMA error
		BNE.S	NotSCSIDMA
		MOVE.L	HD_IO_REQUEST(A6),A1
		MOVE.L	IO_UNIT(A1),A0		; Get unit pointer
		SUBQ.B	#1,HDU_RETRY(A0)	; Loop until retries gone
		BNE.S	HDUSCSILP1		;

NotSCSIDMA:	; Not a DMA error, or retries exhausted
		
		CMP.B	#$80,CMD_ERRORBITS(A4)	; If no error
		BNE.S	ReportSCSIError
		MOVE.B	#HD_NO_ERROR,IO_ERROR(A1) ; Set good status
		BRA	HDUSp_exit
ReportSCSIError:
		MOVE.B	#HDERR_NotSpecified,IO_ERROR(A1); Set BAD status
		BRA	HDUSp_exit


******* System/Drivers/HD/HDUMotor ***********************************
*
*   NAME
*	HDUMotor - user visible control for motor
*
*   SYNOPSIS
*	HDUMotor( IOBlock ), UnitPtr, DevPtr
*		  A1	     A3       A6
*
*   FUNCTION
*	This routine allows the user to control the disc motor. He
*	may turn it either on or off.  Note that the motor will be
*	automatically turned on during an I/O request, but is never
*	turned of except by this command.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
*	    IO_ACTUAL -- returns the previous state of the motor
*	    IO_LENGTH -- the requested state of the motor
*		0 ==> turn motor off
*		1 ==> turn motor on
*
*  For hard disk, parks head when told to turn unit off
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- DevPtr
*	A3 -- unit data
*	A2 -- IOBlock
*


HDUMotor:

		MOVEM.L A0/A1/A2/A3/A4/A5,-(SP)
		MOVE.L	A1,A2			; Save A1
		MOVE.L	IO_UNIT(A1),A3		; Get unit pointer
		CLEAR	D0
		MOVE.B	HDU_MOTOR(A3),D0	; Get old state
		MOVE.L	D0,IO_ACTUAL(A1)	; 	and return to caller
		MOVE.L	IO_LENGTH(A1),D1	; Get requested state
		SEQ.B	D1
		CMP.B	D0,D1
		BEQ	Motor_exit		; If the same, do nothing
		MOVE.B	D1,HDU_MOTOR(A3)	; Save new status
		BEQ	Motor_on

*		Park the heads at the address contained in unit structure

		MOVE.L	HD_CMDPTR(A6),A4	; Point to device command block
		MOVE.B	#HDC_SEEK,CMD_OPCODE(A4); "Seek" command
		MOVE.B	#$FF,CMD_ERRORBITS(A4)	; Tell controller new command
		TST.L	HDU_PARK(A3)		; If park block = 0,
		BEQ.S	Motor_exit		;	DON'T PARK IT!
		BMI.S	Motor_exit
		LEA.L	HDU_PARK+1(A3),A0	; Set up starting block number
		LEA.L	CMD_LUNHIADDR(A4),A1	; Point to command block
		MOVE.B	(A0)+,(A1)+		; Copy the block number
		MOVE.B	(A0)+,(A1)+
		MOVE.B	(A0)+,(A1)+

		MOVE.B	HDU_UNITNUM(A3),D0	; Add in Unit number
		OR.B	D0,CMD_LUNHIADDR(A4)
		
		BSR	ExIO			; Do the I/O

*		BRA.S	Motor_exit
Motor_on:
Motor_exit:
		MOVE.L	A2,A1			; Restore A1
		BSR	TermIO
		MOVEM.L (SP)+,A0/A1/A2/A3/A4/A5

		RTS





******* System/Drivers/HD/HDUSeek ************************************
*
*   NAME
*	HDUSeek - user visible control for the heads
*
*   SYNOPSIS
*	HDUSeek( IOBlock ), DevPtr
*		  A1	     A6
*
*   FUNCTION
*	This routine allows the user to control the seek position.
*	Note that the heads will be automatically seeked during an
*	I/O request; this command allows the heads to be preseeked
*	if the next position is known prior to the I/O being ready.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
*	    IO_OFFSET -- the location to seek to
*
*	Currently ignored for hard disk
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- DevPtr
*	A3 -- unit data
*	A2 -- IOBlock
*

HDUSeek:
		PUTMSG	50,<'%s/HDUSeek called:'>
*		;-- !!! Signal User
		BSR	TermIO
		RTS

******* System/Drivers/HD/HDUProtStatus ******************************
*
*   NAME
*	HDUProtStatus - return whether the current disk is write protected
*
*   SYNOPSIS
*	HDUProtStatus( IOBlock ), UnitPtr, DevPtr
*		  	A1	     A3       A6
*
*   FUNCTION
*	This routine tells whether the current disk is write protected.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
*		IO_ACTUAL - nonzero if the disk is protected, 0 otherwise
*		If there is no disk in the drive, then IO_ERROR is set
*		to HDERR_DiskChanged
*
* For the hard disk, routine always indicates the disk isn't protected
*	and hasn't been changed
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- DevPtr
*	A3 -- unit data
*	A2 -- IOBlock
*

HDUProtStatus:
	PUTMSG	50,<'%s/HDUProtStatus called:'>
	MOVE.L	#0,IO_ACTUAL(A1)


HDUProtStatus_End:
		BSR	TermIO
		RTS


******* System/Drivers/HD/HDUClear ***********************************
*
*   NAME
*	HDUClear - clear all buffers without writting
*
*   SYNOPSIS
*	HDUClear( IOBlock ), UnitPtr, DevPtr
*		  A1	     A3       A6
*
*   FUNCTION
*	This routine clears all buffers in the driver.  Any pending
*	data is deleted.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
*
* Just a no-op for the hard disk
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- DevPtr
*	A3 -- unit data
*	A2 -- IOBlock
*

HDURemove:
		PUTMSG	50,<'%s/HDURemove called:'>
		BRA	HSkip
HDUChangeNum:
		PUTMSG	50,<'%s/HDUChangeNum called:'>
		BRA	HSkip
HDUChangeState:
		PUTMSG	50,<'%s/HDUChangeState called:'>
		BRA.S	HSkip
HDUClear:
		PUTMSG	50,<'%s/HDUClear called:'>
		NOP
HSkip:
		MOVE.L	#0,IO_ACTUAL(A1)
		BSR	TermIO
		RTS


******* System/Drivers/HD/HDUUpdate **********************************
*
*   NAME
*	HDUUpdate - write all dirty buffers out to disc
*
*   SYNOPSIS
*	HDUUpdate( IOBlock ), UnitPtr, DevPtr
*		   A1	      A3       A6
*
*   FUNCTION
*	This routine clears all buffers in the driver.  Any pending
*	data is deleted.
*
*   INPUTS
*	IOBlock - the command block for this IO operation.
*
*
*   EXCEPTIONS
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*	A6 -- DevPtr
*	A3 -- unit data
*	A2 -- IOBlock
*

HDUUpdate:
		PUTMSG	50,<'%s/HDUUpdate called:'>
		IFD	BufWrites
		MOVEM.L D0/D1/A0/A1/A3,-(SP)
		MOVE.L	IO_UNIT(A2),A3		; Get unit pointer
		TST.L	HDU_WRITEB(A3)	; Is there a dirty buffer?
		BEQ.S	HDUUpEx		;	No, exit
		MOVE.L	HDU_WRITEB(A3),A0; Get Buffer address
		BSR	PutBuffer
		BEQ.S	HDUUpEx		; Exit good status if No error
		MOVE.B	#-1,IO_ERROR(A2); Else report error
HDUUpEx:	MOVEM.L (SP)+,D0/D1/A0/A1/A3
		ENDC
		BSR	TermIO
		PUTMSG	50,<'%s/HDUUpdate exit:'>
		RTS

ExIO:
		CMP.B	#2,HDU_UNIT(A3)		; If unit > 1, is a SCSI device
		BGE	SExIO			; Is SCSI, have SExio handle it

		BTST.B	#HDB_NO512,HD_FLAGS(A6)
		BNE.S	No512
		MOVEQ	#0,D0
	        MOVE.L  #HDF_CMDDONE,D1         ; Make sure signal not already
        	LINKSYS SetSignal               ;       pending
		MOVE.L	A5,-(SP)
		MOVE.L	HD_BASE(A6),A5		; Get controller base addr
		BCLR.B	#HDB_SCSI,HD_FLAGS(A6)	; Indicate ST 506 I/O
		MOVE.B	#$50,HDSTAT1(A5)
		MOVE.B	#0,HDCS(A5)		; Wake up controller
		MOVE.B	#1,HDCS(A5)
		MOVE.L	#HDF_CMDDONE,D0		; Wait for interrupt from
		LINKSYS Wait			;	the controller
		MOVE.B	#0,HDCS(A5)		; Clear controller init
		MOVE.L	(SP)+,A5
No512:
		RTS

	END
