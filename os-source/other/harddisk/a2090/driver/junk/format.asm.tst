
*************************************************************************
*									*
*	Copyright (C) 1986, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* format.asm
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
	IFND	EXEC_ALERTS_I
	INCLUDE 'exec/alerts.i'
	ENDC
	INCLUDE	'libraries/filehandler.i'
	INCLUDE 'hddisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'messages.i'
	LIST

	SECTION section


******* Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

*------ Functions ----------------------------------------------------

	XREF	BadTrans	; Check for bad block
	XREF	ClearBuf	; Routine to clear buffers
	XREF	ExIO		; Routine to do actual I/O
	XREF	HDIO		; If not trk 0, then this routine called
				;	to use normal "WRITE" routine
	IFD	BufWrites	; If buffered writes
	XREF	PutBuffer	; Routine to write dirty buffer
	ENDC
	XREF	Soft_Error	; Check to see if soft or hard error detected
	XREF	TermIO
	EXTERN_LIB Alert
	EXTERN_LIB AllocRemember; Routine for memory allocation
	EXTERN_LIB FindTask	; Find this task
	EXTERN_LIB FreeRemember	; Routine for memory deallocation
	EXTERN_LIB PutMsg	; EXEC Put message command
	EXTERN_LIB Wait		; EXEC Wait-for-signal command

*------ System Library Functions -------------------------------------

******* Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	XDEF	HDUFormat
*------ Data ---------------------------------------------------------


******* Local Definitions ********************************************


******* System/Drivers/HD/HDUFormat ***********************************
*
*   NAME
*	HDUFormat - Track format
*
*   SYNOPSIS
*	HDUFormat( IOBlock ), UnitPtr, DevPtr
*		     A1		A3       A6
*
*   FUNCTION
*	This formats the specified # of tracks, checking for and re-mapping bad
*	blocks.
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
*

HDUFormat:
		MOVEM.L D2-D6/A1-A5,-(SP)

*		;-- D2 => track #
*		;-- D3 => number of tracks
*		;-- D4 => Temp Counter
*		;-- D5 => Fill Pattern
*		;-- D6 => # of long words in buffer
*		;-- A3 => unit ptr
*		;-- A2 => Temp working storage area pointer
*		;-- A4 => ioblock
*		;-- A5 => Data passed by caller

		MOVE.L	A1,A4
		MOVE.L	IO_UNIT(A4),A3
		MOVE.L	IO_DATA(A4),A5

		IFD	BufWrites		; If buffered writes
*		;------ Flush buffer if necessary
		TST.L	HDU_WRITEB(A3)		; Is there a dirty buffer?
		BEQ.S	FMNoDirty		;	No, don't worry about it
		MOVE.L	HDU_WRITEB(A3),A0	; 	Yes: output it
		BSR	PutBuffer		; Go write out buffer
FMNoDirty:
		ENDC

*		;------ Clear any buffers necessary
		BSR	ClearBuf

*		;------ Allocate temporary storage

		LEA	HDU_FMTMEM(A3),A0
		CLR.L	(A0)
		MOVE.L	#(MEMF_PUBLIC!MEMF_CLEAR),D1 ; Need "PUBLIC" memory
		MOVE.L	#INIT_SIZE,D0	; Get required size
*		; Get RAM for buffer
		LINKLIB	_LVOAllocRemember,HD_INTUITLIB(A6)
		TST.L	D0
		BNE.S	Fmt_Aloc_Succ

		ALERT	(AN_AMHDDiskDev!AG_NoMemory),,A0

Fmt_Aloc_Succ:	MOVE.L	D0,A2			; A2 points to TEMP storage

		;------ verify the arguments
		TST.L	IO_OFFSET(A4)
		BNE	Not_Trk_Zero	; If this is trk 0,

;	Extract info from boot sector

		MOVE.L	D3,-(SP)
*	Now tell controller what type of drive this is

		MOVE.B	#$4F,HDP_OPTSTEP+INIT_HDP(A2) ; Fast seek
		MOVE.L	HDB_PRECOMP(A5),D0	; Get PRECOMP cylinder
		LSR.L	#4,D0			; 	and divide by 16
		MOVE.B	D0,HDP_PRECOMP+INIT_HDP(A2)
		MOVE.L	HDB_REDUCE(A5),D0	; Get REDUCE cylinder
		LSR.L	#4,D0			; 	and divide by 16
		MOVE.B	D0,HDP_REDUCE+INIT_HDP(A2)
		MOVE.W	HDB_SECTORS(A5),D0
		MOVE.W	D0,HDU_SECTORS(A3)
		MOVE.B	D0,HDP_SECTORS+INIT_HDP(A2)
		MOVE.L	HDB_CYLINDERS(A5),D0	; Get # of cylinders
		MOVE.B	D0,HDP_CYL+INIT_HDP(A2)	; Save low order # of cyls
		LSR.L	#8,D0
		MOVE.W	HDB_HEADS(A5),D1
		MOVE.B	D1,HDU_HEADS(A3)
		LSL.B	#4,D1
		OR.B	D1,D0
		MOVE.B	D0,HDP_HEADHICYL+INIT_HDP(A2)

		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.B	#HDC_SDP,CMD_OPCODE(A1)	;"Set drive paramaters"
		TST.B	HDU_UNITNUM(A3)		; If isn't unit 0,
		BEQ.S	IsUnit0
		MOVE.B	#HDC_SDP1,CMD_OPCODE(A1);change to opcode CC
IsUnit0:	MOVE.B	#$FF,CMD_ERRORBITS(A1)	;Tell new command
		LEA	INIT_HDP(A2),A0		; Point to drive type parameter
		MOVE.L	A0,D0			;	information
		MOVE.B	D0,CMD_LOWDMA(A1); Store Low byte of address
		ROR.L	#8,D0
		MOVE.B	D0,CMD_MIDDMA(A1); Store Middle byte of address
		ROR.L	#8,D0
		MOVE.B	D0,CMD_HIGHDMA(A1); Store High byte address
		MOVE.L	HDB_PARK(A5),D3		; Get PARK cylinder
		BEQ.S	NO_PARK1
		ADDQ.L	#1,D3
		MOVE.B	D3,HDP_CYL+INIT_HDP(A2)
		MOVE.B	HDP_HEADHICYL+INIT_HDP(A2),D0
		AND.B	#$F0,D0
		LSR.L	#8,D3
		OR.B	D3,D0
		MOVE.B	D0,HDP_HEADHICYL+INIT_HDP(A2)
NO_PARK1:	CMP.B	#NUMUNITS,HDU_UNIT(A3)	; If SCSI drive,
		BGE.S	FmIsSCSI		;	don't issue command
		bsr	ExIO			; Go do IO

*	Now compute last block on drive

FmIsSCSI:	MOVEQ.L	#0,D1
		MOVE.L	HDB_CYLINDERS(A5),D3	; Get # of cylinders
		MOVE.B	HDP_HEADHICYL+INIT_HDP(A2),D1	; Get # of heads
		LSR.B	#4,D1
		MOVEQ.L	#0,D0
		MOVE.B	HDP_SECTORS+INIT_HDP(A2),D0
		TST.L	D1			;any heads ?
		BEQ.S	1$			;no, must be SCSI and sectors/cyl
		MULU	D1,D0			; Multiply by # of sectors/track
1$		MULU	D3,D0			; Multiply by park cylinder #
		SUBQ.L	#1,D0			; Block numbers start at 0
		MOVE.L	D0,HDU_LAST(A3)		; Save in unit structure

*	Now compute park block number

		MOVEQ.L	#0,D1
		MOVE.L	HDB_PARK(A5),D3		; Get PARK cylinder
		BEQ.S	NO_PARK2		; If zero, DON'T PARK
		MOVE.B	HDP_HEADHICYL+INIT_HDP(A2),D1	; Get # of heads
		LSR.B	#4,D1
		MOVEQ.L	#0,D0
		MOVE.B	HDP_SECTORS+INIT_HDP(A2),D0
		TST.L	D1			;any heads ?
		BEQ.S	1$			;no, must be SCSI and sectors/cyl
		MULU	D1,D0			; Multiply by # of sectors/track
1$		SUBQ.L	#1,D3			; Re-adjust from before
		MULU	D3,D0			; Multiply by park cylinder #
NO_PARK2:	MOVE.L	D0,HDU_PARK(A3)		; Save in unit structure
		MOVE.L	(SP)+,D3

		CLR.W	BBR_COUNT+HDU_BB(A3) ; Empty bad block table
		MOVE.L	#$7FFFFFFF,BBR_TABLE+BBM_BAD+HDU_BB(A3)
		MOVE.W	#$BAD1,BBR_MAGIC1+HDU_BB(A3); Initialize "MAGIC"
		MOVE.W	#$BAD2,BBR_MAGIC2+HDU_BB(A3); Initialize "MAGIC"
		CLR.L	D0		; Compute # of sectors per cylinder
		CLR.L	D1
		MOVE.W	HDU_SECTORS(A3),D0
		MOVE.B	HDU_HEADS(A3),D1
		BEQ.S	1$		; 0 means sectors/cly already 
		MULU	D1,D0		; D0 now contains # of sectors/cyl
*	Get number of reserved cylinders (= # of first cyl. in 1st partition)
1$		MOVE.L	HDB_ENVIRONMENT+DE_LOWCYL*4(A5),D1
		MULU	D1,D0		; D0 now contains # of reserved blocks
		MOVE.W	D0,BBR_LEFT+HDU_BB(A3) ; Store in bad blk structure
		CLR.L	BBR_NEXT_FREE+HDU_BB(A3); Block 0 1st reserved block

Not_Trk_Zero:
;		TST.B	HDU_HEADS(A3)	; See if unit initialized (PREPed?)
;		BEQ	Format_Err
		MOVE.L	IO_OFFSET(A4),D0
		BSR	HDSectorize
		MOVE.L	D0,D2		; Save track # in D2
		TST.L	D1
		BNE	Format_Err

		MOVE.L	IO_LENGTH(A4),D0
		BSR	HDSectorize
		MOVE.L	D0,D3		; Save # of tracks in D3
		TST.L	D1
		BNE	Format_Err

		;------ check for null length format request
		TST.L	D3
		BEQ	Format_Term

		;------ OK, the args are fine.  Lets fill some tracks!

* Initialize "constant" portion of disk controller command block
		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		LEA	INIT_BUFFER(A2),A0	; Point to track buffer
		MOVE.L	A0,D0
		MOVE.B	D0,CMD_LOWDMA(A1)	; Store Low byte of address
		ROR.L	#8,D0
		MOVE.B	D0,CMD_MIDDMA(A1)	; Store Middle byte of address
		ROR.L	#8,D0
		MOVE.B	D0,CMD_HIGHDMA(A1)	; Store High byte address
* Initialize flags
		BCLR.B	#HDUB_BADFND,HDU_FLAGS(A3) ; No bad blocks found yet
		BCLR.B	#HDUB_WRITEBAD,HDU_FLAGS(A3) ; Table doesn't need update
		BCLR.B	#HDUB_IOERR,HDU_FLAGS(A3) ; No I/O errors yet

dw_trackloop:
		BSR	TrkFormat	; Format the track
		MOVE.L	#$6DB6DB6D,D5	; Test cylinder with 1st pattern
		BSR	B_Fill		; Generate test pattern
		BSR	HDTrkWrite	; Write pattern to cylinder
		BSR	HDTrkRead	; Read pattern from cylinder
		MOVE.L	#$DB6DB6DB,D5	; Test cylinder with 1st pattern
		BSR	B_Fill		; Generate test pattern
		BSR	HDTrkWrite	; Write pattern to cylinder
		BSR	HDTrkRead	; Read pattern from cylinder
		MOVE.L	#$B6DB6DB6,D5; Test cylinder with 1st pattern
		BSR	B_Fill		; Generate test pattern
		BSR	HDTrkWrite	; Write pattern to cylinder
		BSR	HDTrkRead	; Read pattern from cylinder
		BTST.B	#HDUB_BADFND,HDU_FLAGS(A3) ; If bad block(s) found,
		BEQ.S	NoReFMT
		BSR	TrkFormat	; Re-format the track, mark them bad
		BCLR.B	#HDUB_BADFND,HDU_FLAGS(A3) ; Clear flag
NoReFMT:
		TST.B	D0
		BEQ.S	Format_WriteOK

		;------ we had an error on write
		MOVE.B	D0,IO_ERROR(A4)
		BRA.S	Format_Term

Format_WriteOK:

		ADDQ.L	#1,D2
		SUBQ.L	#1,D3
		BNE	dw_trackloop

Format_Term:
		MOVE.L	IO_OFFSET(A4),D0
		BNE	ChkWrtBad	; If this is trk 0
*	Allocate 1st three free good sectors for BOOT info, bad block table
		MOVE.L	BBR_NEXT_FREE+HDU_BB(A3),D5 ; Get # of next free blk
FFindBoot:	SUBQ.W	#1,BBR_LEFT+HDU_BB(A3) ; Show one less free
		MOVE.L	D5,D1
		BSR	BadTrans	; See if it is a bad block itself
		BNE.S	FFndBoot	; If not, use it for boot info
		ADDQ.L	#1,D5		;	else check next block
		BRA.S	FFindBoot
FFndBoot:	MOVE.L	D5,HDU_BOOT(A3)	; Save found block #
		MOVE.L	D5,HDB_PT_BBLOCKS(A5)
FFindBAD1:	ADDQ.L	#1,D5		; Check next block for BAD1
		SUBQ.W	#1,BBR_LEFT+HDU_BB(A3) ; Show one less free
		MOVE.L	D5,D1
		BSR	BadTrans	; See if it is a bad block itself
		BEQ.S	FFindBAD1	; If it is, check next block
		MOVE.L	D5,HDU_1BAD(A3)	; Save found block #
		MOVE.L	D5,HDB_PT_BBLOCKS(A5)
FFindBAD2:	ADDQ.L	#1,D5		; Check next block for BAD2
		SUBQ.W	#1,BBR_LEFT+HDU_BB(A3) ; Show one less free
		MOVE.L	D5,D1
		BSR	BadTrans	; See if it is a bad block itself
		BEQ.S	FFindBAD2	; If it is, check next block
		MOVE.L	D5,HDU_2BAD(A3)	; Save found block #
		MOVE.L	D5,BBR_NEXT_REC+HDU_BB(A3)
		ADDQ.L	#1,D5		; Save # of next free block
		MOVE.L	D5,BBR_NEXT_FREE+HDU_BB(A3)

		LEA	HDB_SIZE(A5),A1	; Address of table passed by caller
GetBadLp1:	CMP.B	#$FF,BAO_HEAD(A1) ; If end of table,
		BEQ	WrtBoot		;	exit loop
		MOVE.L	A1,-(SP)	; Preserve A1
		BSR	CvtOftBlk	; Convert the physical address to blk
		BSR	AddBadBlock	; Add bad block to table
		MOVE.L	(SP)+,A1	; Restore A1
		LEA	BAO_SIZE(A1),A1	; Point to next table entry
		BRA.S	GetBadLp1	;	and process it

*	output boot sector		

WrtBoot:	BSET.B	#HDUB_WRITEBAD,HDU_FLAGS(A3) ; Table need writing
		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.B	#HDC_WRITE,CMD_OPCODE(A1)
*				 Set low bytes block #
		MOVE.W	HDU_BOOT+2(A3),CMD_MIDADDR(A1)
		MOVE.B	HDU_BOOT+1(A3),D0	; Get hi byte
		OR.B	HDU_UNITNUM(A3),D0	; Add in Unit number
		MOVE.B	D0,CMD_LUNHIADDR(A1)
		MOVE.B	#1,CMD_BLOCKCNT(A1)	; Write one sector
		MOVE.B	#$FF,CMD_ERRORBITS(A1)	; New command
		MOVE.L	A5,D0
		MOVE.B	D0,CMD_LOWDMA(A1)	; Store Low byte addr.
		ROR.L	#8,D0
		MOVE.B	D0,CMD_MIDDMA(A1)	; Store Middle byte addr.
		ROR.L	#8,D0
		MOVE.B	D0,CMD_HIGHDMA(A1)	; Store High byte address
		BSR	ExIO

		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.B	CMD_ERRORBITS(A1),D0	; If hard error
		BSR	Soft_Error
		BEQ.S	ChkWrtBad
		BSET.B	#HDUB_IOERR,HDU_FLAGS(A3) ;		set flag

ChkWrtBad:	BTST.B	#HDUB_WRITEBAD,HDU_FLAGS(A3) ; If table changed
		BEQ	BADNoChange
*	Bad Block table has been changed, it needs to be written
		BCLR.B	#HDUB_WRITEBAD,HDU_FLAGS(A3) ; Clear flag
*			Store block number of 1st half of table
		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.B	#HDC_WRITE,CMD_OPCODE(A1); Is a WRITE
		MOVE.W	HDU_1BAD+2(A3),CMD_MIDADDR(A1)
		MOVE.B	HDU_1BAD+1(A3),D0	; Get hi byte
		OR.B	HDU_UNITNUM(A3),D0	; Add in Unit number
		MOVE.B	D0,CMD_LUNHIADDR(A1)
		MOVE.B	#$FF,CMD_ERRORBITS(A1)	; Tell controller new command
		LEA	HDU_BB(A3),A0	; Point to 1st half of bad block tbl
		MOVE.L	A0,D0
		MOVE.B	D0,CMD_LOWDMA(A1)	; Store Low byte addr.
		ROR.L	#8,D0
		MOVE.B	D0,CMD_MIDDMA(A1)	; Store Middle byte addr.
		ROR.L	#8,D0
		MOVE.B	D0,CMD_HIGHDMA(A1)	; Store High byte address
		BSR	ExIO

		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.B	CMD_ERRORBITS(A1),D0	; If hard error
		BSR	Soft_Error
		BNE.S	Format_Err		; Exit with error

*			Store block number of 2nd half of table
		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.W	HDU_2BAD+2(A3),CMD_MIDADDR(A1)
		MOVE.B	HDU_2BAD+1(A3),D0	; Get hi byte
		OR.B	HDU_UNITNUM(A3),D0	; Add in Unit number
		MOVE.B	D0,CMD_LUNHIADDR(A1)
		MOVE.B	#$FF,CMD_ERRORBITS(A1)	; New command
		LEA	HDU_BB+HD_SECTOR(A3),A0	; Point to 2nd half of tbl
		MOVE.L	A0,D0
		MOVE.B	D0,CMD_LOWDMA(A1)	; Store Low byte addr.
		ROR.L	#8,D0
		MOVE.B	D0,CMD_MIDDMA(A1)	; Store Middle byte addr.
		ROR.L	#8,D0
		MOVE.B	D0,CMD_HIGHDMA(A1)	; Store High byte address
		BSR	ExIO

		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.B	CMD_ERRORBITS(A1),D0	; If hard error
		BSR	Soft_Error
		BNE.S	Format_Err		; Exit with error
BADNoChange:
		MOVE.L	IO_OFFSET(A4),D0
		BEQ.S	JustExit		; If is track 0, just exit

* Track has now been formatted.  Since not Track/Cylinder 0, treat as
* a normal WRITE command.

		MOVE.L	#-1,D0			; Free all memory
		LEA	HDU_FMTMEM(A3),A0	; that FORMAT allocated
		LINKLIB	_LVOFreeRemember,HD_INTUITLIB(A6)

		MOVEM.L (SP)+,D2-D6/A1-A5 ; Restore regs
		BRA	HDIO		; Go process as normal write

		;------ Mark the operation as done

JustExit:	BTST.B	#HDUB_IOERR,HDU_FLAGS(A3) ; If error detected
		BEQ.S	ExitNoErr
Format_Err:
		MOVE.B	#20,IO_ERROR(A4)	; Indicate error
ExitNoErr:
		MOVE.L	A4,A1
		BSR	TermIO

		IFGE	INFO_LEVEL-60
		MOVE.L	D2,-(SP)
		INFOMSG 60,<'%s/HDUFormat: exiting.  error %ld'>
		ADDQ.L	#4,SP
		ENDC

		MOVE.L	#-1,D0			; Free all memory
		LEA	HDU_FMTMEM(A3),A0	; that FORMAT allocated
		LINKLIB	_LVOFreeRemember,HD_INTUITLIB(A6)
		MOVEM.L (SP)+,D2-D6/A1-A5

		RTS

******* Device/GetBlockNum *******************************************
*
*   NAME
*	GetBlockNum - Get the block number returned by the controller
*
*   SYNOPSIS
*	BlockNumber = GetBlockNum
*	D0
*
*
**********************************************************************
*
GetBlockNum:
		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.L	CMD_ERRORBITS(A1),D0
		AND.L	#$001FFFFF,D0		; Clear error flag & unit num.
		RTS


******* Device/AddBadBlock *******************************************
*
*   NAME
*	AddBadBlock - Add a bad block to this unit's bad block table
*
*   SYNOPSIS
*	status = AddBadBlock(blockNumber)
*	D0	      	   D0
*
*   FUNCTION
*
**********************************************************************
*
AddBadBlock:				; Add bad block to unit's table
		MOVE.L	D2,-(SP)
		MOVE.L	D0,D1
		BSR	BadTrans	; Search bad block table for spot
		BEQ.S	IsBad		; If already in table, exit
		MOVE.W	BBR_COUNT+HDU_BB(A3),D2 ; If table is already full	
		CMP.W	#NBAD,D2
		BGE.S	TableFull	; Exit with error
		ADDQ.W	#1,D2
		MOVE.W	D2,BBR_COUNT+HDU_BB(A3)	; Show 1 more in table
		BSET.B	#HDUB_WRITEBAD,HDU_FLAGS(A3) ; Table needs writing
		BSET.B	#HDUB_BADFND,HDU_FLAGS(A3) ; Show a new bad block found

* A0 now points to the position in the table where the new bad block
* should be added.  We need to move the rest of the table down to make
* room, then insert the new entry.

		MOVEQ	#(BBM_SIZE/4)-1,D2	; Counter of how many longs to move
ABLoop1:
		CMP.B	#$7F,(A0)		; If end of table
		BEQ.S	ABELoop1		;	Exit loop
		LEA	BBM_SIZE(A0),A0		; Point to next entry
		ADDQ.L	#BBM_SIZE/4,D2		; Update counter
		BRA.S	ABLoop1			;	and loop till end
ABELoop1:
		LEA	BBM_SIZE(A0),A0		; A0 = End of table
		LEA	BBM_SIZE(A0),A1		; A1 = Next free slot

ABLoop2:
		MOVE.L	-(A0),-(A1)		; Move long & bump pointers
		DBRA	D2,ABLoop2		; Move all entries

		MOVE.L	BBR_NEXT_FREE+HDU_BB(A3),D2 ; Get # of next free blk
ABLoop3:	SUBQ.W	#1,BBR_LEFT+HDU_BB(A3) ; Show one less free
		MOVE.L	D2,D1
		MOVE.L	A1,-(SP)	; Preserve A1
		BSR	BadTrans	; See if it is a bad block itself
		MOVE.L	(SP)+,A1	; Restore A1
		BNE.S	ABELoop3	; If not, use it for boot info
		ADDQ.L	#1,D5		;	else check next block
		BRA.S	ABLoop3
ABELoop3:
		MOVE.L	D2,-(A1)	; Store good block #
		ADDQ.L	#1,D2		; Show next block is 1st free
		MOVE.L	D2,BBR_NEXT_FREE+HDU_BB(A3)
		MOVE.L	D0,-(A1)	; Store bad block #

IsBad:		MOVE.L	(SP)+,D2
		RTS

TableFull:	
		BSET.B	#HDUB_IOERR,HDU_FLAGS(A3) ; Fatal error
		BRA.S	IsBad

******* Device/CvtOftBlk *******************************************
*
*   NAME
*	CvtOftBlk - Convert cylinder-head-offset to block number
*
*   SYNOPSIS
*	blockNumber = CvtOftBlk(OfstPointer)
*	D0	      		A1
*
*   FUNCTION
*	Takes cylinder-head-offset form of disk location and
*	figures out what block it is in.
*
**********************************************************************
*
CvtOftBlk:
		MOVE.L	D3,-(SP)
		MOVEQ.L	#0,D0
		MOVE.W	BAO_OFFSET(A1),D0 ; Get offset from index
		SUB.W	#16,D0
		BGE.S	Ge_16
		MOVEQ	#0,D0
Ge_16		DIVU	#609,D0		; Determine which sector,
		MOVE.L	D0,D1		;	609 bytes / sector
		SWAP	D1
		CMP.W	#576,D1		; If in gap, closer to next sector
		BLE.S	ThisSector
		ADDQ.W	#1,D0		;	mark that sector bad
ThisSector:	MOVEQ.L	#0,D3		; Set D3 to # of sectors / track	
		MOVE.W	HDU_SECTORS(A3),D3
		CMP.W	D3,D0		; If > # of sectors / track
		BLE.S	NotToBig
		MOVE.W	D3,D0		;	use maximum # of sectors/trk
NotToBig:	AND.L	#$0000FFFF,D0	; D0 = sectors
		ADD.L	D3,D0		; D0 = nsec + sectors
		MOVEQ.L	#0,D1
		MOVE.B	BAO_HEAD(A1),D1
		ADD.L	D1,D0		; D0 = (nsec + head + sectors)
		DIVU	D3,D0
		CLR.W	D0
		SWAP	D0		; D0 = (nsec+head+sectors )%nsec
		MULU	D3,D1		; D1 = (nsec * head)
		ADD.L	D1,D0
		CLR.L	D1
		MOVE.B	HDU_HEADS(A3),D1
		BNE.S	1$		;sectors/track or sectors/cyl
		MOVEQ.L #1,D1
1$		MULU	D3,D1		; D1 = number sectors / cylinder
		MULU	BAO_CYL(A1),D1	; D1 = (nblkcyl * cyl)
		ADD.L	D1,D0		; D0 = appropriate block number
		MOVE.L	(SP)+,D3
		RTS

******* Device/HDTrkRead *******************************************
*
*   NAME
*	HDTrkRead - Read a track into the track buffer.  Investigate and
*		    record any errors in the bad block table.
*
*   SYNOPSIS
*	HDTrkRead(), unit
*		     A3
*
*   FUNCTION
*	HDTrkRead does a full track read of the specified track.
*	If a hard error is reported by the controller, it is recorded in the
*	bad block table.  If any error (hard or soft) is detected, then each
*	sector is read individually.  Those sectors reporting any error
*	(hard or soft) are recorded in the bad block table.
*
**********************************************************************
*
HDTrkRead:	; Read full track, detect and record errors

		MOVEM.L D3/D4,-(SP)
*	Build command for controller to read the track

		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.B	#HDC_READ,CMD_OPCODE(A1); Set command to  "READ"
		BRA	HDTrkComon		; Jump to common read/write rtn

******* Device/HDTrkWrite *******************************************
*
*   NAME
*	HDTrkWrite - Write out the track in track buffer, investigate and
*		     record any errors in the bad block table
*
*   SYNOPSIS
*	HDTrkWrite(), unit
*		      A3
*
*   FUNCTION
*	HDTrkWrite does a full track write of the data in the track buffer.
*	If a hard error is reported by the controller, it is recorded in the
*	bad block table.  If any error (hard or soft) is detected, then each
*	sector is written individually.  Those sectors reporting any error
*	(hard or soft) are recorded in the bad block table.
*
**********************************************************************
*
HDTrkWrite:	; Write full track, detect and record errors

		MOVEM.L D3/D4,-(SP)
*	Build command for controller to write the track

		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.B	#HDC_WRITE,CMD_OPCODE(A1); Set command to  "WRITE"
HDTrkComon:	;the rest of this routine is identical for reads and writes!
		MOVE.L	D2,D0			; Get Track #
		MULU	HDU_SECTORS(A3),D0	; Convert to block #
		MOVE.W	D0,CMD_MIDADDR(A1)	; Set low bytes block #
		SWAP	D0			; Get hi byte
		OR.B	HDU_UNITNUM(A3),D0	; Add in Unit number
		MOVE.B	D0,CMD_LUNHIADDR(A1)
		MOVE.W	HDU_SECTORS(A3),D0	; Write entire track
		MOVE.B	D0,CMD_BLOCKCNT(A1)
		MOVE.B	#$FF,CMD_ERRORBITS(A1)	; Tell controller new command
		BSR	ExIO

		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.B	CMD_ERRORBITS(A1),D0	; If hard error
		AND.B	#$7F,D0		; If no error, exit
		BEQ	HDWrtExit
		BSR	Soft_Error
		BEQ.S	WrtSects	; If soft error, don't know where
		BSR	GetBlockNum	; Else mark this block bad
		BSR	AddBadBlock

WrtSects:	; Error was detected, try out each sector in track

		MOVE.L	D2,D3			; Get Track #
		MOVE.W	HDU_SECTORS(A3),D4	: Get # sectors/track
		MULU	D4,D3			; Convert to block #
		SUBQ.L	#1,D4			; Ready count for DBRA
		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.B	#1,CMD_BLOCKCNT(A1)	; Write single sector

WrtSectLp1:	MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.W	D3,CMD_MIDADDR(A1)	; Set low bytes block #
		SWAP	D3			; Get hi byte
		OR.B	HDU_UNITNUM(A3),D3	; Add in Unit number
		MOVE.B	D3,CMD_LUNHIADDR(A1)
		SWAP	D3			; Restore D3
*	; Tell controller new command
		MOVE.B	#$FF,CMD_ERRORBITS(A1)
		BSR	ExIO

		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.B	CMD_ERRORBITS(A1),D0	; If hard error
		AND.B	#$7F,D0		; If no error, check next sector
		BEQ.S	WrtSectELp1
		BSR	GetBlockNum	; Else mark this block bad
		BSR	AddBadBlock
WrtSectELp1:	ADDQ.L	#1,D3		; Point to next block
		DBRA	D4,WrtSectLp1	; Loop till whole track done
HDWrtExit:
		MOVEM.L (SP)+,D3/D4
		RTS


******* Device/B_Fill *******************************************
*
*   NAME
*	B_Fill -- fill the track buffer with the pattern specified
*
*   SYNOPSIS
*	B_Fill( pattern ), temp_storage, unit
*		D5,	   A2		 A3
*
*   FUNCTION
*	B_Fill takes the 32 bit pattern supplied in D5, and
*	fills the track buffer in temporary storage with it.
*
* INPUTS:
*	Pattern = 32 bit pattern
*
**********************************************************************
*
B_Fill:		; Fill the track buffer
		LEA	INIT_BUFFER(A2),A0	; Buffer address
		CLR.L	D0		; Compute # of sectors per cylinder
		MOVE.W	HDU_SECTORS(A3),D0
		LSL.L	#7,D0		; Convert to long words (512/4)
		SUBQ.L	#1,D0		; Adjust for DBRA loop
BFillLoop:
		MOVE.L	D5,(A0)+		; Copy 4 bytes to DMA address
		DBRA	D0,BFillLoop		; Loop thru whole buffer
		RTS

******* Device/TrkFormat *******************************************
*
*   NAME
*	TrkFormat -- format the track specified in D2
*
*   SYNOPSIS
*	IOStatus = TrkFormat( track ), unit, devLib
*	D0		      D2	A3,   A6
*
*   FUNCTION
*	TrkFormat formats the specified track.  It uses 1:1 interleave,
*	with a one sector offset between tracks on the same cylinder.
*	Sectors which are contained in the bad block table are marked bad.
*
* INPUTS:
*	Track = The number of the track to be formatted.
*		Track * Sectors_per_track = block # of 1st block on track
**********************************************************************
*
TrkFormat:	; Format the track
		MOVEM.L	D2-D7/A2/A4,-(SP)
		LEA	INIT_BUFFER(A2),A4 ; A4 points to track buffer
		CLR.L	D3
		MOVE.W	HDU_SECTORS(A3),D3 ; # of sects/track
		MOVE.L	D3,D7		; Save # of sectors
		MOVE.L	D2,D4
		MULU	D3,D4		; D4 = current block #
		MOVE.L	D4,D5		; D5 = track's block #
		MOVE.L	D2,D6
		SUBQ.W	#1,D3		; Adjust D3 for DBRA
		CLR.L	D2
		MOVE.B	HDU_HEADS(A3),D2; D2 = # of trks/cyl
		DIVU	D2,D6
		CLR.W	D6
		SWAP	D6		; D6 holds starting sect #
TFSecLoop1:
		MOVE.L	D5,D1		; Get # of block (offset 0)
		ADD.L	D6,D1		; Add in offset of this sector
		BSR	BadTrans	; See if is a good block
		BEQ.S	TFIsBad
		CLR.B	(A4)+		;     Show sector is good
		BRA.S	TFSecJmp1
TFIsBad:	MOVE.B	#$80,(A4)+	;     else show is bad
TFSecJmp1:	MOVE.B	D6,(A4)+	; Write sector # to buffer
		ADDQ.B	#1,D6		; Next sector mod # sectors
		DIVU	D7,D6
		CLR.W	D6
		SWAP	D6		; D6 holds next sect. #
		DBRA	D3,TFSecLoop1	; Loop for all sectors

*	Now build command for controller to format the track

		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.B	#HDC_FMTT,CMD_OPCODE(A1); Set to "Format Track"
		MOVE.W	D5,CMD_MIDADDR(A1)	; Set low bytes block #
		SWAP	D5			; Get hi byte
		OR.B	HDU_UNITNUM(A3),D5	; Add in Unit number
		MOVE.B	D5,CMD_LUNHIADDR(A1)
INFO_Retry:	MOVE.B	D7,CMD_BLOCKCNT(A1)	; Length = # of sectors
		MOVE.B	#$FF,CMD_ERRORBITS(A1)	; Tell controller new command
		BSR	ExIO

		MOVE.L	HD_CMDPTR(A6),A1	; Point to device command block
		MOVE.B	CMD_ERRORBITS(A1),D0	; If hard error
		BSR	Soft_Error
		MOVEM.L	(SP)+,D2-D7/A2/A4
		RTS

******* Device/HDSectorize *******************************************
*
*   NAME
*	HDSectorize -- divide a long byte offset into sectors and tracks
*
*   SYNOPSIS
*	track, sector = HDSectorize( offset ), unit, devLib
*	D0     D1		     D0		A3,	A6
*
*   FUNCTION
*	HDSectorize turns a byte offset into tracks and sectors,
*	suitable for the rest of the system.  If the offset is
*	not legal (e.g. not an integral number of sectors) then
*	track and sector will be set to -1.  An error is also
*	returned if the offset is too large.
*
*   INPUTS
*	track -- the track offset
*	sector -- the sector offset
*
**********************************************************************
*

HDSectorize:
		MOVE.L	D2,-(SP)	; Preserve register
		;------ check operation for legality
		MOVE.L	D0,D1

		;------ check for being an integral sector boundary
		AND.L	#HD_SECTOR-1,D1
		BNE	Sectorize_Err

		;------ convert to sector offset
		MOVEQ	#HD_SECSHIFT,D1
		LSR.L	D1,D0

		;------ check for IO within disc range
		CMP.L	HDU_LAST(A3),D0
		BGT.S	Sectorize_Err

		;------ divide by sector number
		MOVE.W	HDU_SECTORS(A3),D2
		DIVU	D2,D0	; sector in high word, track
						; in low word

		;------ and put it in its final resting spot
		MOVEQ	#0,D1
		SWAP	D0
		MOVE.W	D0,D1		; set the sector return value
		MOVE.W	#0,D0		; clear out the sector portion
		SWAP	D0		; put track back where it should be
Sectorize_End:
		IFGE	INFO_LEVEL-60
		MOVEM.L	D0/D1,-(SP)
		INFOMSG	60,<'%s/Sectorize: track %ld sector %ld'>
		ADDQ.L	#8,SP
		ENDC

		MOVE.L	(SP)+,D2	; Restore register
		RTS

Sectorize_Err:
		MOVEQ	#-1,D0
		MOVEQ	#-1,D1
		RTS

		END
