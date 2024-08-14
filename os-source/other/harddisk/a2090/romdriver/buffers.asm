*******************************************************************************
*
*	Source Control
*	--------------
*	$Header: buffers.asm,v 34.13 88/04/06 18:42:05 bart Exp $
*
*	$Locker:  $
*
*	$Log:	buffers.asm,v $
*   Revision 34.13  88/04/06  18:42:05  bart
*   OHDIO calls BgenIO if overscan requires single block transfers
*   
*   Revision 34.12  88/04/02  13:55:33  bart
*   overscan support (BGenIO)
*   
*   Revision 34.11  88/04/02  13:46:17  bart
*   overscan support
*   
*   Revision 34.10  88/01/21  18:04:41  bart
*   compatible with disk based / binddriver useage
*   
*   Revision 34.9  87/12/04  19:13:37  bart
*   checkpoint
*   
*   Revision 34.8  87/12/04  12:08:09  bart
*   checkpoint before adding check for existing dosname on eb_mountlist
*   
*   Revision 34.7  87/10/14  14:40:24  bart
*   10-13 rev 1
*   
*   Revision 34.6  87/10/14  14:15:40  bart
*   beginning update to cbm-source.10.13.87
*   
*   Revision 34.5  87/07/08  14:00:46  bart
*   y
*   
*   Revision 34.4  87/06/11  15:47:35  bart
*   working autoboot 06.11.87 bart
*   
*   Revision 34.3  87/06/03  10:58:47  bart
*   checkpoint
*   
*   Revision 34.2  87/05/31  16:35:30  bart
*   chickpoint
*   
*   Revision 34.1  87/05/29  19:39:08  bart
*   checkpoint
*   
*   Revision 34.0  87/05/29  17:39:23  bart
*   added to rcs for updating
*   
*
*******************************************************************************


*************************************************************************
*									*
*	Copyright (C) 1986, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* buffers.asm
*
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
	IFD	BufferIO
	LIST



******* Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

*------ Functions ----------------------------------------------------

	XREF	TermIO
	XREF	Soft_Error
	XREF	ExIO
	XREF	BadTrans

*------ System Library Functions -------------------------------------

******* Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	XDEF	HDIO		; Main I/O routine for driver
	XDEF	ClearBuf	; Mark all buffers as empty
	XDEF	GetSector	; Get this block
	XDEF	PutSector	; Put this block into a buffer
	XDEF	LoadBuffer	; Fill buffer from disk
	IFD	BufWrites
	XDEF	PutBuffer	; Write out buffer
	ENDC

*------ Data ---------------------------------------------------------


******* Local Definitions ********************************************


******* System/Drivers/HD/GetBuf ***************************************
*
*   NAME
*	GetBuf	- Get an empty buffer.  If an empty buffer currently exists,
*		  then return its address.  Otherwise, find the least
*		  recently used buffer,
*		  initialize it to empty state, and return its address.
*
*   SYNOPSIS
*	GetBuf(DevPtr)
*		A6
*
*   FUNCTION
*	Get an empty buffer.  If an empty buffer currently exists, then
*	return its BI_structure.  Otherwise, find the least recently used
*	buffer, initialize it to empty state,
*	and return the address of its BI_structure in A0.
*
*   INPUTS
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
*

GetBuf:
		MOVEM.L	D1/A1,-(SP)		; Preserve registers
		LEA	HD_BUFS(A6),A0		; Point to BI_structures
		MOVE.L	#NUM_BUFS-1,D0
		MOVE.L	A0,A1			; Start with 1st block LRU
		CLR	D1			; Zero highest LRU count

GBLoop1:	; Find empty buffer, or least recently used
		TST.B	BI_BLKS(A0)		; If this buffer is empty,
		BEQ.S	GBFound			;	Jump
		CMP.W	BI_ACC(A0),D1		; If this buffer used more
		BGT.S	GBELoop1		;  recently, jump
		MOVE.W	BI_ACC(A0),D1		; Else save its LRU count
		MOVE.L	A0,A1			;	and its address
GBELoop1:
		ADD.L	#BI_SIZE,A0		; Point to next buffer
		DBRA	D0,GBLoop1		; Check all buffers
		MOVE.L	A1,A0			; Found LRU buffer
GBFound:
		CLR.B	BI_BLKS(A0)		; Show buffer is empty
		CLR.L	BI_UNIT(A0)
		CLR.W	BI_ACC(A0)		; Clear LRU reference counter
		MOVEM.L	(SP)+,D1/A1		; Restore registers
		RTS

******* System/Drivers/HD/ClearBuf ***************************************
*
*   NAME
*	ClearBuf - Mark all of this unit's  buffers as empty.
*
*   SYNOPSIS
*	ClearBuf(UnitPtr,DevPtr)
*		   A3	   A6
*
*   FUNCTION
*	Marks all buffers as empty, without previously flushing their contents.
*
*   INPUTS
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
*	A3 -- UnitPtr
*	A6 -- DevPtr
*

ClearBuf:
		MOVE.L	A3,D1
		LEA	HD_BUFS(A6),A0		; Point to BI_structures
		MOVE.L	#NUM_BUFS-1,D0		; Init loop count = all buffers

CLLoop1:	; Mark each buffer empty
		CMP.L	BI_UNIT(A0),D1		; If this buffer for this unit
		BNE.S	CLELoop1;
		CLR.L	BI_UNIT(A0)		;	Zero Unit Ptr
		CLR.B	BI_BLKS(A0)		;	Zero Block count
CLELoop1:
		ADD.L	#BI_SIZE,A0		; Point to next buffer
		DBRA	D0,CLLoop1		; Check all buffers
		RTS

******* System/Drivers/HD/LoadBuffer ***************************************
*
*   NAME
*	LoadBuffer - Load a buffer from disk.  If no bad blocks exist on the
*		     track containing the block, it will load the whole track.
*		     If bad blocks exist on that track, it will load the load
*		     the good portion of the track surrounding the requested
*		     block (unless the requested block itself is bad, then it
*		     gets it's own buffer).  This should prevent any buffer
*		     overlap.
*
*   SYNOPSIS
*	status = LoadBuffer(BI_Ptr,UnitPtr,DevPtr)
*	  D0			A0	A3   A6
*
*   FUNCTION
*
*   INPUTS
*
*	HD_LBLOCK in the device structure points to the requested block,
*	HD_UNIT points to the appropriate unit.
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
*	A3 -- UnitPtr
*	A0 -- Pointer to BI structure
*

LoadBuffer:
		MOVEM.L	D1/D2/D3/D4/A4,-(SP)	; Preserve registers
		MOVE.L	A0,A4			; Save BI pointer
		CLR.W	BI_ACC(A4)		; Clear LRU reference counter
		CLR.L	BI_UNIT(A4)		; Show buffer currently empty
		MOVE.B	#HDC_READ,HD_CMD(A6)	; Request a read
		MOVE.L	HD_LBLOCK(A6),D1	; Get block #
		DIVU	#BUF_SECTS,D1		; Compute beginning of track
		MULU	#BUF_SECTS,D1
		MOVE.L	D1,D4
		ADD.L	#BUF_SECTS,D4		; D4 points to start of next trk
		MOVE.L	D1,BI_START(A4)		; Store in BI structure
		BSR	BadTrans		; Check the bad block list
		CMP.L	BI_START(A4),D1		; If start of track bad,
		BNE.S	LDHasBad		;	go figure out what to do
		MOVE.L	(A0),D0			; Get next bad block #
		SUB.L	D1,D0			; # of blocks to bad block
		CMP.L	#BUF_SECTS,D0		; If before end of buffer
		BLE.S	LDHasBad		;	need to figure out what
		MOVE.L	#BUF_SECTS,D0		; Else fill the whole buffer
		BRA.S	LDGetBuf

LDHasBad:	MOVE.L	HD_LBLOCK(A6),D1	; Get desired block #
		CMP.L	(A0),D1			; Is bad block before or after?
		BEQ.S	LDIsBad			;	Block itself is bad
		BLT.S	LDBadAfter		;	Bad block after request
*	A bad block exists on this track before desired block
		CMP.L	BBM_SIZE(A0),D1		; Next bad before desired block?
		BLT.S	LDBadBef1		;	No - Next blk start buf
		MOVE.L	(A0),D1			;	Yes, try after bad blk
		ADDQ.L	#1,D1			;	  Point to next block
		MOVE.L	D1,BI_START(A4)		;	  Is 1st block in buffer
		LEA	BBM_SIZE(A0),A0		;	  Point to next bad
		BRA.S	LDHasBad		;	      and repeat test
LDBadBef1:	MOVE.L	(A0),D1			; Get bad block #
		ADDQ.L	#1,D1			; Point to next, GOOD block
		MOVE.L	D1,BI_START(A4)		; Show 1st block in buffer
		LEA	BBM_SIZE(A0),A0		; Point to next bad block
		CMP.L	(A0),D4			; Next bad in this track?
		BLT.S	LNNoMoreBad		;	No - use rest of track
		MOVE.L	(A0),D4			;	Yes - end before bad blk
LNNoMoreBad:	SUB.L	BI_START(A4),D4		; Compute length of buffer data
		MOVE.L	D4,D0
		BRA.S	LDGetBuf		; Go load the buffer
LDBadAfter:
*	A bad block may exist on this track after the desired block
		CMP.L	(A0),D4			; Next bad in this track?
		BLE.S	LNNo2MoreBad		;	No - use rest of track
		MOVE.L	(A0),D4			;	Yes - end before bad blk
LNNo2MoreBad:	SUB.L	BI_START(A4),D4		; Compute length of buffer data
		MOVE.L	D4,D0
		BRA.S	LDGetBuf		; Go load the buffer		

LDIsBad:
*	Desired block is a bad block - give  it it's own buffer

		MOVE.L	BBM_GOOD-BBM_BAD(A0),HD_PBLOCK(A6); Show mapped block actual
		MOVE.L	HD_LBLOCK(A6),BI_START(A4) ; Buffer starts this block
		MOVEQ.L	#1,D0			; Buffer is 1 sector long
		BRA.S	LDGetBad
LDGetBuf:
*	Come here go actually load the buffer
		MOVE.L	BI_START(A4),HD_PBLOCK(A6); Logical=Physical block
LDGetBad:	MOVE.L	HD_CMDPTR(A6),A0	; A4 points to command blk
		MOVE.B	D0,CMD_BLOCKCNT(A0)	; Save # of blocks to read
		MOVE.B	D0,BI_BLKS(A4)
		ADD.L	BI_START(A4),D0		; Compute last block in buffer
		MOVE.L	D0,BI_END(A4)		;	and save it
		MOVE.L	HD_UNIT(A6),BI_UNIT(A4)	; Set unit pointer
		MOVE.L	HD_PBLOCK(A6),BI_PHYSICAL(A4); Save actual buffer block
		LEA	BI_DATA(A4),A1
		MOVE.L	A1,HD_DMA(A6)		; Buffer address for I/O
		BSR	GenIO			; Go build cmd blk & do the I/O
		TST.B	D0
		BEQ.S	LDNoErr			; If error detected,
		CLR.L	BI_UNIT(A4)		; 	show buffer empty
		CLR.B	BI_BLKS(A4)
		TST.B	D0			; Set flag to indicate error
LDNoErr:
		MOVEM.L	(SP)+,D1/D2/D3/D4/A4	; Restore registers
		RTS

******* System/Drivers/HD/GetSector ***************************************
*
*   NAME
*	GetSector - Locate the specified sector, and move to specified address
*
*   SYNOPSIS
*	status = GetSector(DestinationAddress,DevPtr)
*	  D0			A0		A6
*
*   FUNCTION
*	Searches the buffers to see if sector already loaded.  If it isn't,
*	it locates and frees the least recently used buffer, and fill that
*	buffer with a much of the desired track as possible.  Once the sector
*	is loaded, it is copied to the address passed in A0.
*
*   INPUTS
*
*	HD_LBLOCK in the device structure points to the requested block,
*	HD_UNIT points to the appropriate unit.
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
*	A0 -- Pointer to DMA address
*

GetSector:
		MOVEM.L	A4/A5,-(SP)		; Preserve registers
		MOVE.L	A0,A5			; Preserve DMA address
		BSR	FindSector		; See if sector already loaded
		MOVE.L	A0,D0			; If so,
		BNE.S	GSGotSect		;	go copy to DMA addr
		BSR	GetBuf			; Else	go find an empty buffer
		MOVE.L	A0,A4			; Save BI address
		BSR	LoadBuffer		; 	and load this sector
		TST.B	D0
		BNE.S	GSErrOut		; D0 not zero, means error
 		MOVE.L	HD_LBLOCK(A6),D0	; Get desired block #
		SUB.L	BI_START(A4),D0		; # of blocks from desired block
		MULU	#HD_SECTOR,D0		; Compute # of bytes offset
		LEA.L	BI_DATA(A4),A0		; Get address of buffer
		ADD.L	A0,D0			; Add address of buffer
		MOVE.L	D0,A0
GSGotSect:		
		MOVEQ.L	#127,D0			; Set loop count to move 512b
GSLoop:
		MOVE.L	(A0)+,(A5)+		; Copy 4 bytes to DMA address
		DBRA	D0,GSLoop		; Loop 128 times
		CLR.L	D0			; Show no error
GSErrOut:					; D0 not zero, means error
		MOVEM.L	(SP)+,A4/A5		; Restore registers
		RTS

******* System/Drivers/HD/PutSector ***************************************
*
*   NAME
*	PutSector - Write specified sector to disk.  If copy exists in buffer,
*		    update that buffer also.
*
*   SYNOPSIS
*	status = PutSector(SourceAddress,DevPtr)
*	  D0			A0	   A6
*
*   FUNCTION
*	Searches the buffers to see if sector already loaded.  If so, it
*	copies the new version into the buffer.  It always updates the copy
*	on disk.
*
*   INPUTS
*
*	HD_LBLOCK in the device structure points to the requested block,
*	HD_UNIT points to the appropriate unit.
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
*	A0 -- Pointer to DMA address
*

PutSector:
		MOVEM.L	D1/D3/D4/D5/A4/A5,-(SP)	; Preserve registers
		MOVE.L	A0,A5			; Preserve DMA address
		BSR	FindSector		; See if sector in buffer
		MOVE.L	A0,D0			; If not,
		IFND	BufWrites		; If not buffered writes
		BEQ.S	PSWriteSect		;	just write it out
		ENDC
		IFD	BufWrites		; If buffered writes
		BNE.S	InBuff			; Is already in buffer
		BSR	GetBuf			; Else	go find an empty buffer
		MOVE.L	A0,A4			; Save BI address
		BSR	LoadBuffer		; 	and load this sector
		TST.B	D0
		BNE.S	PSErrOut		; D0 not zero, means error
		MOVE.L	A4,HDU_WRITEB(A3)	; Show this buffer modified
 		MOVE.L	HD_LBLOCK(A6),D0	; Get desired block #
		SUB.L	BI_START(A4),D0		; # of blocks from desired block
		MULU	#HD_SECTOR,D0		; Compute # of bytes offset
		LEA.L	BI_DATA(A4),A0		; Get address of buffer
		ADD.L	A0,D0			; Add address of buffer
		MOVE.L	D0,A0
		ENDC
		
InBuff:		MOVEQ.L	#127,D0			; Set loop count to move 512b
		MOVE.L	A5,A1
PSLoop:
		MOVE.L	(A5)+,(A0)+		; Copy 4 bytes to DMA address
		DBRA	D0,PSLoop		; Loop 128 times
		MOVE.L	A1,A5

		IFND	BufWrites		; If not buffered writes
PSWriteSect:
		MOVE.L	HD_CMDPTR(A6),A0	; A4 points to command blk
		MOVE.B	#1,CMD_BLOCKCNT(A0)	; Always write single sector
		MOVE.B	#HDC_WRITE,HD_CMD(A6)	; Request a write
		MOVE.L	A5,HD_DMA(A6)		; Set source address
		MOVE.L	HD_LBLOCK(A6),D1	; Get logical address
		BSR	BadTrans		; Check the bad block list
		MOVE.L	D1,HD_PBLOCK(A6)	; Save physical address	
		BSR.S	GenIO			; Go build cmd blk & do the I/O
		ENDC
		IFD	BufWrites
		CLR.L	D0			; Indicate no error
		ENDC
PSErrOut:
		MOVEM.L	(SP)+,D1/D3/D4/D5/A4/A5	; Restore registers
		RTS

******* System/Drivers/HD/FindSector ***************************************
*
*   NAME
*	FindSector - Locate the specified sector in a buffer if present
*
*   SYNOPSIS
*	address = FindSector(DevPtr)
*	  A0			A6
*
*   FUNCTION
*	Searches the buffers for the sector in HD_LBLOCK for unit HD_UNIT.
*	If found, it returns the address in A0, else A0 is zero.
*	If the sector is found, A1 points to the BI structure for its buffer.
*
*   INPUTS
*
*	HD_LBLOCK in the device structure points to the requested block,
*	HD_UNIT points to the appropriate unit.
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
*

FindSector:
		MOVEM.L	D1/D2/D3,-(SP)		; Preserve registers
		MOVE.L	HD_LBLOCK(A6),D1		; Get block #
		MOVE.L	A3,D3			; Get unit pointer
		LEA	HD_BUFS(A6),A0		; Point to BI_structures
		MOVE.L	#NUM_BUFS-1,D0		; Get # of buffers
FSLoop0:
		ADDQ.W	#1,BI_ACC(A0)		; Make sure all buffers are aged
		ADD.L	#BI_SIZE,A0		; Point to next buffer
		DBRA	D0,FSLoop0		; 

		IFD	BufWrites		; If buffered writes
		TST.L	HDU_WRITEB(A3)		; Is there a dirty buffer?
		BEQ.S	FNoDirty		;	No, don't worry about it
		MOVE.L	HDU_WRITEB(A3),A0	; 	Yes:
		CMP.L	BI_START(A0),D1		; If buffer starts after
		BLT.S	FSWrtBuf		;	this block, write buf
		CMP.L	BI_END(A0),D1		; If block in this buffer
		BLT.S	FSFound			;	update buffer

FSWrtBuf:	BSR	PutBuffer		; Go write out buffer
		ENDC

FNoDirty:	LEA	HD_BUFS(A6),A0		; Point to BI_structures
		MOVE.L	#NUM_BUFS-1,D0
FSLoop1:
		CMP.L	BI_UNIT(A0),D3		; If this buffer for this unit
		BNE.S	FSELoop1;
		CMP.L	BI_START(A0),D1		; If buffer starts after
		BLT.S	FSELoop1		;	this block, skip
		CMP.L	BI_END(A0),D1		; If buffer ends before 
		BGE.S	FSELoop1		;	this block, skip

*	Found buffer containing sector

FSFound:	CLR.W	BI_ACC(A0)		; Clear LRU counter
		SUB.L	BI_START(A0),D1		; # of blocks from desired block
		MULU	#HD_SECTOR,D1		; Compute # of bytes offset
		LEA	BI_DATA(A0),A1		; Get address of buffer
		ADD.L	A1,D1			; Add in offset
		BRA.S	FSExLp1

FSELoop1:
		ADD.L	#BI_SIZE,A0		; Point to next buffer
		DBRA	D0,FSLoop1		; Check all buffers

		CLEAR	D1			; Zero to indicate not found
FSExLp1:
		MOVE.L	A0,A1			; Save BI pointer
		MOVE.L	D1,A0			; Return address of buffer
		MOVEM.L	(SP)+,D1/D2/D3		; Restore registers
		RTS

		IFD	BufWrites		; If buffered writes
; A0 points to buffer to be written
; A3 points to Unit structure
; A6 points to Device structure
; D0 low byte 0 on exit if no error
PutBuffer:
		MOVE.L	HD_CMDPTR(A6),A1	; A1 points to command blk
		MOVE.B	BI_BLKS(A0),CMD_BLOCKCNT(A1) ; # of sectors to write
		MOVE.B	#HDC_WRITE,HD_CMD(A6)	; Request a write
		MOVE.L	BI_PHYSICAL(A0),HD_PBLOCK(A6) ; Actual start block #
		LEA.L	BI_DATA(A0),A0		; Get buffer address
		MOVE.L	A0,HD_DMA(A6)		; Set source address
		BSR.S	GenIO			; Go build cmd blk & do the I/O
		BNE.S	PBOut			; If error, report it
		CLR.L	HDU_WRITEB(A3)		; Else show it isn't dirty
PBOut:		RTS
		ENDC

******* System/Drivers/HD/GenIO ***************************************
*
*   NAME
*	GenIO - Execute requested I/O
*
*   SYNOPSIS
*	status = GenIO(DevPtr)
*	  D0		A6
*
*   FUNCTION
*	Does the I/O specified by fields in the Device structure
*
*   INPUTS
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
*	A3 -- UnitPtr
*	A0 -- Pointer to BI structure
*

GenIO:
		MOVEM.L	D3/A0/A1/A2/A4,-(SP)	; Preserve registers
		MOVE.L	HD_CMDPTR(A6),A4	; A4 points to command blk
		MOVE.L	#MAXRETRY,D3		; Retry DMA errors MAX times

GenIO_IO:	; Come here for retrys
		MOVE.B	HD_CMD(A6),CMD_OPCODE(A4); Set up the command
		MOVE.B	#$FF,CMD_ERRORBITS(A4)	; Tell controller new command
		LEA.L	HD_DMA+1(A6),A0		; Set up DMA address
		LEA.L	CMD_HIGHDMA(A4),A1	; Point to command block
		MOVE.B	(A0)+,(A1)+		; Copy the DMA address
		MOVE.B	(A0)+,(A1)+
		MOVE.B	(A0)+,(A1)+

		LEA.L	HD_PBLOCK+1(A6),A0	; Set up starting block number
		LEA.L	CMD_LUNHIADDR(A4),A1	; Point to command block
		MOVE.B	(A0)+,(A1)+		; Copy the block number
		MOVE.B	(A0)+,(A1)+
		MOVE.B	(A0)+,(A1)+

		MOVE.B	HDU_UNITNUM(A3),D0	; Add in Unit number
		OR.B	D0,CMD_LUNHIADDR(A4)
		
		BSR	ExIO				; Do the actual I/O

		MOVE.B	CMD_ERRORBITS(A4),D0	; Get controller status
		BSR	Soft_Error		; Is it a hard error?
		BEQ.S	GenIO_CLen		; 	No, jump
		CMP.B	#$32,D0			; Is it a DMA error ?
		BNE	GenIO_ErrSkp		;	No - Error
		SUBQ.L	#1,D3			; Retry DMA errors MAX times
		BNE	GenIO_IO
		BSR	BGenIO				; break it down into block transfers
GenIO_ErrSkp:
		TST.B	D0			; Error!
		BRA.S	GenIO_OUT		; D0 not 0, means error!
GenIO_CLen:
		CLR.L	D0			; No error
GenIO_OUT:
		MOVEM.L	(SP)+,D3/A0/A1/A2/A4	; Restore registers
		RTS

******* System/Drivers/HD/BGenIO ***************************************
*
*   NAME
*	BGenIO - Execute requested I/O A BLOCK AT A TIME
*
*   SYNOPSIS
*	status = BGenIO(DevPtr)
*	  D0		A6
*
*   FUNCTION
*	Does the I/O specified by fields in the Device structure
*	but breaks larger I/O down into single block transfers
*
*   INPUTS
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
*	A3 -- UnitPtr
*	A0 -- Pointer to BI structure
*

BGenIO:
		MOVEM.L D3/D4/D5/D6/D7/A0/A1/A2/A4,-(SP)    ; Preserve registers
		MOVE.L	HD_CMDPTR(A6),A4	; A4 points to command blk

		MOVEQ.L	#0,D7				; block counter
		MOVE.B	CMD_BLOCKCNT(A4),D7	; save the number of blocks to transfer
		MOVE.B	#1,CMD_BLOCKCNT(A4)	; temp set the blocks to transfer to 1
		MOVE.L	D7,D6				; set the block-at-a-time loop counter

		LEA.L	HD_PBLOCK+1(A6),A0	; Set up starting block number
		MOVEQ.L	#0,D4				; zero the current block number
		MOVE.B	(A0)+,D4			; Copy the starting block number
		LSL.L	#8,D4
		MOVE.B	(A0)+,D4		
		LSL.L	#8,D4
		MOVE.B	(A0)+,D4		

		LEA.L	HD_DMA+1(A6),A0		; Set up DMA address
		MOVEQ.L	#0,D5				; zero the current block dma address
		MOVE.B	(A0)+,D5			; Copy the starting DMA address 
		LSL.L	#8,D5
		MOVE.B	(A0)+,D5		
		LSL.L	#8,D5
		MOVE.B	(A0)+,D5		

		MOVEQ.L	#0,D0				; clear error on entry
		BRA		BGenIO_OUT

BGenIO_Bloop:                       ; Come back here for block loop

		LEA.L	CMD_HIGHDMA(A4),A1	; Point to command block
		SWAP    D5
		MOVE.B  D5,(A1)+
		ROL.L   #8,D5
		MOVE.B  D5,(A1)+
		ROL.L   #8,D5
		MOVE.B  D5,(A1)+            ; and set current block dma address
	 	ADD.L   #512,D5             ; next block dma address

		LEA.L   CMD_LUNHIADDR(A4),A1    ; Point to command block
		SWAP	D4
		MOVE.B	D4,(A1)+	
		ROL.L	#8,D4
		MOVE.B	D4,(A1)+
		ROL.L	#8,D4
		MOVE.B	D4,(A1)+			; and set current block number
		ADDQ.L  #1,D4               ; next block number

		MOVEQ.L	#1,D3 				; Retry DMA algorithmically
		ADD.W	D6,D3				; based on i/o REMAINING so that
		MULU	#MAXMAXRETRY,D3		; errors tend to 1/MAXMAXRETRY for all i/o

BGenIO_IO:	; Come here for retrys
		MOVE.B	HD_CMD(A6),CMD_OPCODE(A4); Set up the command
		MOVE.B	#$FF,CMD_ERRORBITS(A4)	; Tell controller new command
		LEA.L	HD_DMA+1(A6),A0		; Set up DMA address

		LEA.L	HD_PBLOCK+1(A6),A0	; Set up starting block number
		LEA.L	CMD_LUNHIADDR(A4),A1	; Point to command block

		MOVE.B	HDU_UNITNUM(A3),D0	; Add in Unit number
		OR.B	D0,CMD_LUNHIADDR(A4)

		BSR	ExIO				; Do the actual I/O

		MOVE.B	CMD_ERRORBITS(A4),D0	; Get controller status
		BSR	Soft_Error		; Is it a hard error?
		BEQ		BGenIO_CLen		; 	No, jump
		CMP.B	#$32,D0			; Is it a DMA error ?
		BNE	BGenIO_ErrSkp		;	No - Error
		SUBQ.L	#1,D3			; Retry DMA errors MAX times
		BNE	BGenIO_IO
BGenIO_ErrSkp:
		TST.B	D0			; Error!
		BRA.S	BGenIO_OUT		; D0 not 0, means error!
BGenIO_CLen:
		CLR.L	D0			; No error
BGenIO_OUT:
        DBNE    D6,BGenIO_Bloop     ; loop until D6 exhausted OR error
		MOVE.B	D7,CMD_BLOCKCNT(A4)		; restore number of blocks transferred
		MOVEM.L	(SP)+,D3/D4/D5/D6/D7/A0/A1/A2/A4	; Restore registers
		RTS

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
*	A2 -- IOBlock
*	D3 -- Block count
*	D4 -- DMA address
*

HDIO:

		MOVEM.L D2/D3/D4/A2/A3/A4,-(SP)
		MOVE.L	IO_UNIT(A1),A3		; Get unit pointer
		MOVE.L	A3,HD_UNIT(A6)		; Indicate is current unit
		CLR.B	HDU_MOTOR(A3)		; Show "Motor" being "ON"

		MOVE.L	A1,A2
		MOVE.L	A2,HD_IO_REQUEST(A6)

*		;-- {state 0} check operation for legality
		MOVE.L	IO_OFFSET(A2),D0
		MOVE.L	D0,D1

*		;-- check for being an even sector boundary
		AND.L	#HD_SECTOR-1,D1
		BNE	HDIO_Err

		MOVE.L	IO_DATA(A2),D4		; Initialize working variables
		MOVE.L	IO_LENGTH(A2),D3	; Get byte length
		MOVEQ	#HD_SECSHIFT,D0		; Convert byte offset to block #
		LSR.L	D0,D3			; D3 contains block count
		MOVE.L	IO_OFFSET(A2),D1	; Convert byte count to blocks
 		LSR.L	D0,D1
		MOVE.L	D1,HD_LBLOCK(A6)	; Starting block #
		CMP.L	#1,D3			; Single sector I/O ?
		BNE	OHDIO			;	No - don't buffer
*LCE		BRA	OHDIO			;	No buffering for test!

HDIO_IO:
		
		MOVE.L	D4,A0			; Pass DMA address
		MOVE.W	IO_COMMAND(A2),D0
		CMP.B	#CMD_WRITE,D0
		BNE.S	HDIO_Read

* 			WE ARE DOING A WRITE

		BSR	PutSector		; Put data into buffer
		BRA.S	HDIO_Next

HDIO_Read:
* 			WE ARE DOING A READ

		BSR	GetSector		; Go get sector
HDIO_Next:

		ADDQ.L	#1,HD_LBLOCK(A6)	; Bump block #
		ADD.L	#HD_SECTOR,D4		; Bump DMA address
		ADD.L	#HD_SECTOR,IO_ACTUAL(A2); Update byte count

*		Test for error

		TST.B	D0
		BNE	HDIO_Err		; Jump if error

		SUBQ.L	#1,D3			; Decriment block count
		BNE.S	HDIO_IO			; Loop until done or error

*		;-- {state 5} mark IOBLOCK as done
HDIO_Signal

		MOVE.L	HD_IO_REQUEST(A6),A1
		BSR	TermIO

HDIO_End:
		MOVEM.L (SP)+,D2/D3/D4/A2/A3/A4
		RTS

HDIO_Err:

		MOVE.L	HD_IO_REQUEST(A6),A1
		MOVE.B	#-1,IO_ERROR(A1)
		BRA	HDIO_Signal

OHDIO:
		MOVE.L	HD_CMDPTR(A6),A4	; Point to device command block

		MOVE.L	#MAXRETRY,D3		; how did we ever forget this ? bart

*		;-- {state 0} check operation for legality
		MOVE.L	IO_OFFSET(A2),D0
		MOVE.L	D0,D1

*		;-- check for being an even sector boundary
		AND.L	#HD_SECTOR-1,D1
		BNE	HDIO_Err

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
		BEQ.S	OHDIO_Up20

*************************************************************************
*									*
* 			WE ARE DOING A WRITE				*
*									*
*************************************************************************

		MOVE.B	#$0A,CMD_OPCODE(A4)	; Set up the write command
		BSR	ClearBuf		; Show all buffers are empty
		BRA.S	OHDIO_IO

OHDIO_Up20:
*************************************************************************
*									*
* 			WE ARE DOING A READ				*
*									*
*************************************************************************

		MOVE.B	#$08,CMD_OPCODE(A4)	; Set up the write command

OHDIO_IO:
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

		BSR	ExIO			; Do the actual I/O

*		Test for error

		MOVE.B	CMD_ERRORBITS(A4),D0	; Get controller status
		BSR	Soft_Error		; Is it a hard error?
		BEQ.S	OHDIO_CLen		;	No, jump
		CMP.B	#$32,D0			; Is it a DMA error ?
		BNE	OHDIO_ErrSkp		;	No - Error
		SUBQ.L	#1,D3			; Retry DMA errors MAX times
		BNE	OHDIO_IO

*		Retry a block at a time
	
		BSR		BGenIO			; can we force block i/o ?
		TST.L	D0
		BNE.S	OHDIO_ErrSkp	; if i/o failed error exit
		MOVE.L	D2,D0			; else restore block count 
		MOVEQ.L	#1,D3			; and indicate success
		BRA.S	OHDIO_BClen
OHDIO_ErrSkp:	CLR.L	D3			; LSB D3 = 0 means error

*		;------ compute the "# of bytes transferred"

OHDIO_CLen:	MOVE.L	CMD_ERRORBITS(A4),D0	; Get ending block #
		AND.L	#$001FFFFF,D0		; Clear status and LUN bits
		ADDQ.L	#1,D0			; Compute how many bytes moved
		MOVE.L	CMD_OPCODE(A4),D1	;	Get starting block #
		AND.L	#$001FFFFF,D1		;	Clear opcode & LUN
		SUB.L	D1,D0			;	Get # blocks moved
OHDIO_BClen:
		MOVE.L	D0,D2			;	Save block count
		MOVEQ.L	#HD_SECSHIFT,D1		; 	Convert to byte count
		LSL.L	D1,D0
		ADD.L	D0,IO_ACTUAL(A2)	; Update # bytes moved
		TST.L	D3			; Fatal I/O error?
		BEQ	HDIO_Err		;	Yes - error exit

		ADD.L	D0,HD_DMA(A6)		; Bump DMA address
		ADD.L	D2,HD_LBLOCK(A6)	; Update logical block #
		SUB.L	D2,HD_BLOCKCNT(A6)	; Update # of blocks to move
		BGT	OHDIO_IO			; Loop until done or error

*		;-- {state 5} mark IOBLOCK as done
		BRA	HDIO_Signal

		ENDC	; BufferIO

		END
