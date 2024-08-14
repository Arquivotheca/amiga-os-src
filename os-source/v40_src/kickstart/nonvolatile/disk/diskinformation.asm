******************************************************************************
*
*	$Id: diskinformation.asm,v 40.5 93/03/31 18:54:41 brummer Exp Locker: vertex $
*
******************************************************************************
*
*	$Log:	diskinformation.asm,v $
* Revision 40.5  93/03/31  18:54:41  brummer
* Fix missplaced push so that if ever expansion library is not
* available (which should never happen) the code will work
* 
* Revision 40.4  93/03/12  11:37:43  brummer
* Remove 68020 assembler option and instructions.  This required calling
* utility.library to do a 32 bit multiply previously done by mulu instruction.
*
* Revision 40.3  93/03/11  14:26:42  brummer
* Remove LLName definition.  This is fix related to nonvolatilebase.i
* 40.5
*
* Revision 40.2  93/03/08  18:02:09  brummer
* Add DiskInit semaphore per Martin's request.
* Always call KillReq for DiskInit.
*
* Revision 40.1  93/02/19  14:54:38  Jim2
* If DOS hasn't been opened don't try to use it.
*
* Revision 40.0  93/02/18  10:41:58  Jim2
* Initial release - tested.
*
*
*
*	(C) Copyright 1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

		INCLUDE	'exec/macros.i'

		INCLUDE	'dos/dos.i'
		INCLUDE	'dos/filehandler.i'

		INCLUDE	'libraries/expansionbase.i'

		INCLUDE	'/nonvolatilebase.i'
		INCLUDE	'/nonvolatile.i'

		XDEF	InitDisk
		XDEF	ReleaseDisk
		XDEF	GetDiskInfo

OFFSET_TO_NAME	EQU	0
SPARE_BLOCKS	EQU	3

*****i* nonvolatile.library/GetDiskInfo **************************************
*
*   NAME
*	GetDiskInfo - Reports information on the disk
*
*   SYNOPSIS
*	GetDiskInfo (MemBlock)
*		    d0
*
*	VOID GetDiskInfo(APTR)
*
*   FUNCTION
*	Returns information about the current users nonvolatile disk device.
*
*	Currently this information consists of the size of the disk and the
*	number of bytes free on the disk.  These values are stored
*	consecutively, in the first two long words in MemBlock.
*
*   INPUTS
*	MemBlock - Pointer to memory block used for storing information on
*		   the disk.
*
*   RESULT
*	NONE.
*
******************************************************************************
GetDiskInfo
		movem.l	d0/d2-d4/a5-a6,-(sp)	;Save the address for copying and d2 is needed for Info call.
		move.l	a6,a5
		bsr	ReleaseDisk
		bsr.s	InitDisk
		move.l	nv_DOSBase(a5),a6
		move.l	nv_DiskLock(a5),d1
		beq.s	Error			;If we don't have a lock on it we can't get info on it.
				;Have a lock on the user's prefered nonvolatile device.
		sub.l	#id_SIZEOF+4,sp
		move.l	sp,d2
		addq.l	#3,d2
		and.b	#$FC,d2			;Need a long word aligned struct InfoData.
		JSRLIB	Info
		move.l	d2,a0			; a0 gets data pointer
		move.l	id_BytesPerBlock(a0),d2	; d2 gets # of bytes/block
		move.l	id_NumBlocks(a0),d3	; d3 gets # of blocks
		move.l	d3,d4			; d4 gets # of free blocks
		sub.l	id_NumBlocksUsed(a0),d4	;
		add.l	#id_SIZEOF+4,sp		; Reset the stack pointer.
		tst.w	d0			; error on info call ?
		beq.s	Error			; if yes, j to return zeros

;		mulu.l	d3,d1
;		bvc.s	No_d1_Overflow
;		moveq.l	#-1,d1
; No_d1_Overflow:
;		sub.l	#SPARE_BLOCKS,d2
;		bcs.s	NoSpace
;		mulu.l	d3,d2
;		bvc.s	Exit
;		moveq.l	#-1,d2
;		bra.s	Exit

;
; Calculate total bytes on disk :
;
		move.l	nv_UTILBase(a5),a6	; a6 gets utility.library base
		move.l	d2,d0			; d0 gets # of bytes per block
		move.l	d3,d1			; d1 gets # of blocks
		JSRLIB	UMult32			; d0 gets total bytes on disk
		move.l	d0,d3			; save this in d3
;
; Calculate total free bytes on disk :
;
		sub.l	#SPARE_BLOCKS,d4
		bcs.s	NoSpace
		move.l	d2,d0			; d0 gets # of bytes per block
		move.l	d4,d1			; d1 gets # of free blocks
		JSRLIB	UMult32			; d0 gets total free bytes on disk
		bra.s	Exit			; j to return calculated values

NoSpace:	CLEAR	d0			; zero amount of free space
		bra.s	Exit			; j to return calculated values
;
; Could not get information on the nonvolatile device.
; It is safest to report no space available.
;
Error:		moveq.l	#0,d0
		move.l	d0,d3

Exit:		move.l	(sp)+,a0		;Get pointer to where the data is to be written.
		move.l	d3,(a0)+		;Save the maximum storage.
		move.l	d0,(a0)+		;Save the free storage
		movem.l	(sp)+,d2-d4/a5-a6	;Restore the registers.
		rts


*****i* nonvolatile.library/InitDisk *****************************************
*
*   NAME
*	InitDisk - Finds the user's specified nonvolatile device.
*
*   SYNOPSIS
*	InitDisk (NVBase)
*		  a5
*
*	VOID InitDisk (struct NVBase *)
*
*   FUNCTION
*	Finds the user's preferred nonvolatile device and gets a shared lock
*	on it.
*
*	The user's preferred nonvolatile device is indicated by the contents
*	of the file sys:prefs/env-archive/sys/nv_location.  This file is read
*	and a read lock is attempted using the string contained in this file.
*
*   INPUTS
*	NONE
*
*   RESULT
*	Sets the nv_DiskLock field in the base of nonvolatile.library.
*
******************************************************************************
InitDisk
		movem.l	d2-d4/a4-a6,-(sp)	;Need some scratch registers.

		move.l	nv_ExecBase(a5),a6	; a6 gets exec base
		lea	nv_DiskInitSema(a5),a0	; a0 gets ptr to semaphore
		JSRLIB	ObtainSemaphore		; obtain semaphore

		move.l	#1,d1			; force requesters off
		ChkKillRequesters		;

		move.l	#256,d3			;Maximum path length for a string+1.  Also gives a NULL byte.
		lea	EXPANSION_NAME(pc),a1
		moveq.l	#0,d0
		JSRLIB	OpenLibrary		;Open the Expansion library.
		move.l	d0,d4
		beq	id_Exit
		move.l	d4,-(sp)		; save ExpanLib base
;
; Successful open of expansion library, check if DOS is available :
;
		tst.l	nv_DOSBase(a5)
;		tst.l	a6
		beq	CloseAndExit
		move.l	nv_DOSBase(a5),a6
;		move.l	d4,-(sp)		;Remember the library base for a later close.
		add.l	#eb_MountList,d4	;Get a pointer to the start of the mount list.
		move.l	d4,a0
		move.l	(a0),d4
		lea	SYS_BSTR(pc),a0
NextDevice
				;Got the next node to look at.
		move.l	sp,a4			;Remember the current stack pointer since we don't know how much we are going to add.
		sub.l	d3,sp
		move.w	#SIZE_NVLOC,d0
		lea	NVDiskStorage(pc),a1
CopyFileName:	move.b	0(a1,d0.w),-(a4)
		dbf	d0,CopyFileName		;Copy the nv_location file name on the stack, backwards.
		move.b	(a0)+,d0		;Get size of string and move to the first real character.
		ext.w	d0
		bra.s	EndCopyDevice		;Using the dbf, it's best just to skip to the test.

CopyDevice:	move.b	0(a0,d0.w),-(a4)
EndCopyDevice:	dbf	d0,CopyDevice		;Copy the device name.

		move.l	a4,d1			;Place the pointer to the filename into d1.
		move.l	#MODE_OLDFILE,d2
		JSRLIB	Open
		add.l	d3,sp			;Clear the filename off the stack.
		move.l	d0,d1			;File pointer in d1.  This sets up the Read.
		beq.s	NoFile			;The file does not exist.
				;Opened the file.
		sub.l	d3,sp			;Allocate space for the read.
		move.l	sp,d2
		move.l	d1,-(sp)
		JSRLIB	Read			;Read from d1 into d2, size d3.  Didn't check failure.
		move.l	(sp)+,d1
		JSRLIB	Close			;We are in big trouble if this fails.
		add.l	#OFFSET_TO_NAME,d2	;Skip any frou-frou in the file.
		move.l	d2,d1
		move.l	d2,a1
PlaceNull:	cmp.b	#$20,(a1)+
		bge.s	PlaceNull		;Skip to the first control character.
				;Got a control character.
		move.b	d3,-(a1)
		move.l	#ACCESS_READ,d2
		JSRLIB	Lock			;Get a lock on the users preferred device.
		add.l	d3,sp			;Clear the data read from the stack.
		move.l	d0,nv_DiskLock(a5)	;Don't error check, just save.
		bne.s	CloseAndExit		;Now error check.
NoFile
		NEXTNODE.s	d4,a0,CloseAndExit
		move.l	bn_DeviceNode(a0),a0
		move.l	dn_Name(a0),d0		;BString.
		lsl.l	#2,d0
		move.l	d0,a0
		bra.s	NextDevice

CloseAndExit:	move.l	(sp)+,a1		;Retrieve the pointer to the expansion library.
		move.l	nv_ExecBase(a5),a6
		JSRLIB	CloseLibrary		;And close it.

id_Exit:	ChkRestoreRequesters		;
		lea	nv_DiskInitSema(a5),a0	; a0 gets ptr to semaphore
		JSRLIB	ReleaseSemaphore	; release semaphore

		movem.l	(sp)+,d2-d4/a4-a6	;Restore all scratch registers.
		rts

*****i* nonvolatile.library/ReleaseDisk **************************************
*
*   NAME
*	ReleaseDisk - Releases the lock from the users nonvolatile device.
*
*   SYNOPSIS
*	ReleaseDisk (NVBase)
*		     a5
*
*	VOID ReleaseDisk (struct NVBase *)
*
*   FUNCTION
*	Unlocks the users prefered nonvolatile device.
*
*   INPUTS
*	NONE
*
*   RESULT
*	Unlocks and clears nv_DiskLock
*
******************************************************************************
ReleaseDisk
		move.l	nv_DiskLock(a5),d1
		beq.s	rd_Exit
		move.l	a6,-(sp)
		moveq.l	#0,d0
		move.l	d0,nv_DiskLock(a5)
		move.l	nv_DOSBase(a5),a6
		JSRLIB	UnLock
		move.l	(sp)+,a6
rd_Exit:	rts

SYS_BSTR	dc.b	3,'SYS'
NVDiskStorage	dc.b	':prefs/env-archive/sys/nv_location',0
EXPANSION_NAME	dc.b	'expansion.library',0


SIZE_NVLOC	equ	EXPANSION_NAME-NVDiskStorage-1

		end
