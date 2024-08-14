******************************************************************************
*
*	$Id: fileio.asm,v 40.10 93/04/16 12:11:54 brummer Exp Locker: vertex $
*
******************************************************************************
*
*	$Log:	fileio.asm,v $
* Revision 40.10  93/04/16  12:11:54  brummer
* Fix Lockhart's mungwall/enforcer hits on storenv
* 
* Revision 40.9  93/04/13  16:43:53  brummer
* Fix CI170, if file exists on NVRAM but not on NVDISK oand user updates
* it, a subdirectory will be created and not removed because the lock is
* not removed before calling DeleteNV() function.
*
* Revision 40.8  93/03/31  15:58:08  brummer
* Fix multiple requesters when App name exists and Item name does
* not and your using a write protected disk.
*
* Revision 40.7  93/03/31  11:50:41  brummer
* Fix to remove 6x requesters on storeNV to write protected disk.
*
* Revision 40.6  93/03/26  15:03:08  brummer
* Make branch short when checking for access to sys rsvd data
* area in DeleteDisk() function.
*
* Revision 40.5  93/03/26  11:24:29  brummer
* Fix to check for attempted access to the system reserved
* area in read, write, and delete NVRAM disk.
*
* Revision 40.4  93/03/18  11:11:12  brummer
* Fix code in DeleteDisk function that was accessing parameters incorrectly
* off the stack.  Code now uses free registers instead of offset from
* top of stack.
*
* Revision 40.3  93/03/09  14:40:25  brummer
* Fix save/restore of d7 in DeleteDisk per Martin's request.
*
* Revision 40.2  93/02/26  17:30:15  Jim2
* If there is a write failure will remove any empty files/directories.
*
* Revision 40.1  93/02/25  08:49:06  Jim2
* WriteDisk now will recognise bad file name errors and return
* an appropriate error.
*
* Revision 40.0  93/02/10  17:30:16  Jim2
* Added new parameter to WriteDisk to differeniate between
* updating a file and creating a new file.  Detect a write
* protect failure ane return wether it was the final file,
* or further up (AppName/disk).
*
* Revision 39.0  93/02/03  11:21:52  Jim2
* Initial Release - Tested.
*
*
*	(C) Copyright 1992,1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

		INCLUDE	'exec/macros.i'
		INCLUDE	'exec/memory.i'

		INCLUDE	'dos/dos.i'


		INCLUDE '/nonvolatile.i'
		INCLUDE	'/nonvolatilebase.i'




		XDEF	ReadDisk
		XDEF	_ReadDisk
		XDEF	WriteDisk
		XDEF	_WriteDisk
		XDEF	DeleteDisk
		XDEF	_DeleteDisk

_ReadDisk	EQU	ReadDisk
_WriteDisk	EQU	WriteDisk
_DeleteDisk	EQU	DeleteDisk


*****i* nonvolatile.library/ReadDisk *****************************************
*
*   NAME
*	ReadDisk - Reads an Item from the Disk.
*
*   SYNOPSIS
*	Data = ReadDisk(AppName, ItemName, NVBase)
*	d0		a0	 a1	   a6
*
*	APTR = ReadDisk(char *, char *, struct NVBase *)
*
*   FUNCTION
*	Searches the Disk for the AppName and ItemName.  If they exist, it
*	allocates storage for the item plus one additional long word for the
*	size of memory allocated.  The size of the allocation is stored in
*	the first long word, and the data (for the AppName and ItemName) is
*	stored beginning at the second long word.
*
*	If either the AppName and ItemName cannot be found, or the memory
*	cannot be allocated a NULL will be returned.
*
*
*   INPUTS
*	AppName,ItemName - Two string used to identify the data stored.
*	NVBase - Pointer to the base of nonvolatile.library.
*
*   RESULT
*	Data - Pointer to a copy of the data if found and memory allows it
*	       to be copied.  NULL, if this is not true.
*
*   SEE ALSO
*	ReadNVRAM()
*
******************************************************************************
*
*   REGISTERS
*	d4  Lock on Item file/file handle for Item File.
*	d5  Size of Item file
*	d6  Lock on App directory.
*	d7  Return parameter - points to any memory allocated.
*	a2  Pointer to FIB used in examine
*	a3  Original directory lock.
*	a5  NVBase
*	a6  DOSBase
*
*	a4  Untouched.
*
*	d0,d1,d2,d3,a0,a1 - used in subroutine calls, or scratch.
ReadDisk
		move.l	d7,-(sp)
		moveq.l	#0,d7			;Set up the return parameter.
		move.l	nv_DiskLock(a6),d1	;Check to see if there is a nonvolatile disk out there.
		beq	rd_NoDisk
;
; Yes, there is a disk, check for access to system reserved area :
;
		tst.b	(a0)			; is App string NULL ?
		bne.s	6$			; if no, j to continue
		tst.b	(a1)			; is Item string also NULL ?
		bne.s	6$			; if no, j to continue
		bra	rd_NoDisk		; j to error return
6$:
		movem.l	d2-d6/a0-a6,-(sp)
		move.l	a6,a5
		move.l	nv_DOSBase(a5),a6
		JSRLIB	CurrentDir		;Change to the directory the user has picked for nonvolatile storage.
		move.l	d0,a3			;Remember the orginal directory.
				;Lock the AppName.
		move.l	4*5(sp),d1
		move.l	#ACCESS_READ,d2
		JSRLIB	Lock			;Get a lock on the directory for the AppName.
		move.l	d0,d6
		beq	NoApp
				;Change to the AppName directory.
		move.l	d0,d1
		JSRLIB	CurrentDir
				;Lock the ItemName.
		move.l	4*6(sp),d1
		move.l	#ACCESS_READ,d2
		JSRLIB	Lock
		move.l	d0,d5
		beq	FinishedApp
				;Find the size of the ItemName file.
		sub.l	#fib_SIZEOF+4,sp	;Need a FIB, but must be long word aligned.
		move.l	sp,d2
		addq.l	#3,d2
		and.b	#$FC,d2			;Forced long word alignment.
		move.l	d2,a2
		move.l	d5,d1
		JSRLIB	Examine			;Get the first entry.
		move.l	fib_Size(a2),d4
		add.l	#fib_SIZEOF+4,sp
		tst.w	d0
		beq.s	NoMemory
				;Got the information
		addq.l	#4,d4			;Extra long word to remember the size.
		move.l	d4,d0
		move.l	#MEMF_PUBLIC,d1
		move.l	nv_ExecBase(a5),a6
		JSRLIB	AllocMem
		move.l	nv_DOSBase(a5),a6
		tst.l	d0
		beq.s	NoMemory
				;Have a memory block for copying.
		move.l	d0,a0
		move.l	d4,(a0)+		;Store the memory block size.
		subq.l	#4,d4			;Restore the size of the read.
		move.l	a0,d7
		move.l	d5,d1
		JSRLIB	OpenFromLock		;Before reading we must open the file.
		move.l	d0,d1
		beq.s	NotOpened
				;Opened the ItemName file.
		move.l	d1,d5
		move.l	d7,d2
		move.l	d4,d3
		JSRLIB	Read			;Read the block.
		sub.l	d0,d4			;If the memory block wasn't filled there was a failure.
		move.l	d5,d1
		JSRLIB	Close
		tst.l	d4
		bne.s	FailedRead
				;Read successfully.
		bra.s	FinishedApp
				;Couldn't open the file.
NotOpened:	move.l	d5,d1
		JSRLIB	UnLock			;Still have a pesky lock.
FailedRead:	move.l	d7,a1			;Couldn't read, so free the memory block.
		move.l	-(a1),d0
		move.l	nv_ExecBase(a5),a6
		JSRLIB	FreeMem
		move.l	nv_DOSBase(a5),a6
		moveq.l	#0,d7
		bra.s	FinishedApp
				;Couldn't get the memory, but still have a pesky lock on ItemName.
NoMemory:	move.l	d5,d1
		JSRLIB	UnLock
FinishedApp:	move.l	d6,d1			;All done with the AppName directory.
		JSRLIB	UnLock
NoApp:		move.l	a3,d1			;All done with the users nonvolatile device.
		JSRLIB	CurrentDir
		movem.l	(sp)+,d2-d6/a0-a6	;Restore the registers.
rd_NoDisk:	move.l	d7,d0			;Set up return value.
		move.l	(sp)+,d7
		rts

*****i* nonvolatile.library/WriteDisk ****************************************
*
*   NAME
*	WriteDisk - Writes an Item to the Disk.
*
*   SYNOPSIS
*	Error = WriteDisk(AppName, ItemName, Data, Length, Mode, NVBase)
*	d0		  a0	   a1	     a2    d0	   d1	 a6
*
*	UWORD = WriteDisk(char *, char *, APTR, ULONG, LONG, struct NVBase *)
*
*   FUNCTION
*	Attempts to store the information pointed to by Data to the Disk.
*	The data will be associated with AppName and ItemName.
*
*   INPUTS
*	AppName,ItemName - Two string used to identify the data stored.
*	Data - Pointer to data to be stored.
*	Length - Size of data in bytes.
*	NVBase - Pointer to the base of nonvolatile.library.
*
*   RESULT
*	Error:
*	0 - Data stored no error.
*	NVERR_BADNAME - DOS reports either ERROR_BAD_STREAM_NAME, or
*		ERROR_INVAID_COMPONENT_NAME.
*	NVERR_WRITEPROT - Disk/directory write protected.
*	NVERR_FATAL - Failure writing the file.
*	NVERR_FAIL - Other failure in the process of getting to the write.
*	$80000000|NVERR_WRITEPROT - File write protected.
*
*   SEE ALSO
*	WriteNVRAM()
*
******************************************************************************
*
*   REGISTERS
*	d3  Number of bytes to write/Number of bytes not written
*	d4  file handle for Item File.
*	d5  Lock on App directory.
*	d6  Original directory lock.
*	d7  Return parameter.
*	a5  NVBase
*	a6  DOSBase
*
*	a2,a3,a4  Untouched.
*
*	d0,d1,d2,d3,a0,a1 - used in subroutine calls, or scratch.
WriteDisk
		move.l	d7,-(sp)
		move.l	#NVERR_FAIL,d7		;Set up the return parameter.
		movem.l	d1-d6/a0-a1/a4-a6,-(sp)
		move.l	a6,a5
		move.l	d0,d3
		move.l	#31,d4	;Needed to indicated the file write protected.
		move.l	nv_DOSBase(a5),a6
		move.l	nv_DiskLock(a5),d1	;Check to see if there is a nonvolatile disk out there.
		beq	wd_NoDisk
;
; Yes, there is a disk, check for access to system reserved area :
;
		tst.b	(a0)			; is App string NULL ?
		bne.s	6$			; if no, j to continue
		tst.b	(a1)			; is Item string also NULL ?
		bne.s	6$			; if no, j to continue
		bra	wd_NoDisk		; j to error return
;
; Change to user specified nonvolatile storage directory :
;
6$:		CLEARA	a4			; clear disk update flag
		JSRLIB	CurrentDir		;Change to directory
		move.l	d0,d6			;Remember the orginal directory.
;
; Lock the AppName directory :
;
LockApp:	move.l	4*6(sp),d1		; d1 gets pointer to App name string
		move.l	#ACCESS_READ,d2		; read access for
		JSRLIB	Lock			; Get a lock on the directory for the AppName.
		move.l	d0,d5			; d5 gets status from lock of App
		bne.s	App			; j if lock is successful
;
; Lock on App unsuccessful, attempt to create the subdirectory :
;
		move.l	4*6(sp),d1		; d1 gets pointer to App name string
		JSRLIB	CreateDir		;
		move.l	d0,d1			; was creation successful ?
		beq.s	wd_NoAccess		; if no, j to return
		move.l	#1,a4			; set disk update flag
		JSRLIB	UnLock			;
		bra.s	LockApp			; j to reacquire lock on subdirectory
;
; Change to the AppName directory and attempt to open the file :
;
App:		move.l	d0,d1
		JSRLIB	CurrentDir
		move.l	4*7(sp),d1		; d1 gets ptr to item name string
		move.l	4*0(sp),d2		; d2 gets mode for open
		JSRLIB	Open			; open the file
		move.l	d0,d4			; d4 gets open status
		beq.s	wd_NoAccess		; if error, j to check
;
; Open file successful, attempt to write the file :
;
		cmpi.l	#MODE_NEWFILE,d2	; was mode newfile ?
		bne.s	8$			; if no, j to continue
		move.l	#1,a4			; set disk update flag

8$:		move.l	d4,d1
		move.l	a2,d2
		JSRLIB	Write			; Write the block.
		sub.l	d0,d3
		move.l	d4,d1
		JSRLIB	Close			; close even if error
		move.l	#NVERR_FATAL,d7
		tst.l	d3			; was write sucessful ?
;		bne.s	wd_FailedWrite
		bne.s	wd_NoAccess		; if no, j to determine error
		move.l	d3,d7			; d7 gets return parameter
		bra.s	wd_FinishedApp		; j to remove locks and return
;
; Write failed, determine the error :
;
wd_NoAccess:	JSRLIB	IoErr
		cmp.l	#ERROR_BAD_STREAM_NAME,d0
		beq.s	wd_BadName
		cmp.l	#ERROR_INVALID_COMPONENT_NAME,d0
		beq.s	wd_BadName
		cmp.l	#ERROR_DISK_WRITE_PROTECTED,d0
		beq.s	wd_WritePro
		cmp.l	#ERROR_WRITE_PROTECTED,d0
		bne.s	wd_FailedWrite
;
; Disk was write protected :
;
wd_WritePro:	move.l	#($80000000|NVERR_WRITEPROT),d7	; d7 gets return status
;		bclr.l	d4,d7
;		bne.s	wd_FinishedApp
		bra.s	wd_FinishedApp		; j to skip call to delete
;
; Bad name provided by user :
;
wd_BadName:	move.l	#NVERR_BADNAME,d7	; d7 gets return status
;
; Call delete function to remove possible App subdirectory :
;
wd_FailedWrite:	move.l	a4,d1			; update flag set ?
		beq.s	wd_FinishedApp		; if no, j to continue

		move.l	d5,d1			; is there lock on APP directory ?
		beq.s	10$			; if no, j to skip
		JSRLIB	UnLock			; unlock App directory

10$:		move.l	4*6(sp),a0		; a0 gets AppName pointer
		move.l	4*7(sp),a1		; a1 gets ItemName pointer
		exg	a5,a6			; a6 gets NVBase, a5 gets DOSBase
		bsr.s	DeleteDisk		; call delete function
		exg	a5,a6			; a5 gets NVBase, a6 gets DOSBase
		bra.s	wd_NoAppLock		; skip unlock of App
;
; Remove outstanding locks, restore saved current directory, and return :
;
wd_FinishedApp:	move.l	d5,d1			; is there lock on APP directory ?
		beq.s	wd_NoAppLock		; if no, j to skip
		JSRLIB	UnLock			; unlock App directory
wd_NoAppLock:	move.l	d6,d1			; d1 gets saved current directory
		JSRLIB	CurrentDir		; change to saved direcotry
wd_NoDisk:	movem.l	(sp)+,d1-d6/a0-a1/a4-a6	; restore registers
		move.l	d7,d0			; d0 gets return value
		move.l	(sp)+,d7
		rts


*****i* nonvolatile.library/DeleteDisk ***************************************
*
*   NAME
*	DeleteDisk - Deletes an Item from the Disk.
*
*   SYNOPSIS
*	Success = DeleteDisk(AppName, ItemName, NVBase)
*	d0		     a0       a1	a6
*
*	BOOL = DeleteDisk(char *, char *, struct NVBase *)
*
*   FUNCTION
*	Searches the disk for the AppName and ItemName.  If they exist, any
*	data assocated with them is freed.  If there are no other ItemNames
*	assocated with AppName it is also removed.
*
*   INPUTS
*	AppName,ItemName - Two string used to identify the data stored.
*	NVBase - Pointer to the base of nonvolatile.library.
*
*   RESULT
*	Success - TRUE if AppName and ItemName are found.  FALSE if they are
*		  not.
*
*   SEE ALSO
*	DeleteNVRAM()
*
******************************************************************************
*
*   REGISTERS
*	d3  Number of bytes to write/Number of bytes not written
*	d4  file handle for Item File.
*	d5  Lock on App directory.
*	d6  Original directory lock.
*	d7  Return parameter.
*	a5  NVBase
*	a6  DOSBase
*
*	a2,a3,a4  Untouched.
*
*	d0,d1,d2,d3,a0,a1 - used in subroutine calls, or scratch.
DeleteDisk
		movem.l	d3-d7/a0-a1/a5-a6,-(sp)
		moveq.l	#0,d7			;Set up the return parameter.
		move.l	nv_DiskLock(a6),d1	;Check to see if there is a nonvolatile disk out there.
		beq.s	dd_NoDisk
;
; Yes, there is a disk, check for access to system reserved area :
;
		tst.b	(a0)			; is App string NULL ?
		bne.s	6$			; if no, j to continue
		tst.b	(a1)			; is Item string also NULL ?
		bne.s	6$			; if no, j to continue
		bra.s	dd_NoDisk		; j to error return
6$:
		move.l	a0,d3			; d3 gets App name pointer
		move.l	a1,d4			; d4 gets Item name pointer
		move.l	a6,a5
		move.l	nv_DOSBase(a5),a6
		JSRLIB	CurrentDir		;Change to the directory the user has picked for nonvolatile storage.
		move.l	d0,d6			;Remember the orginal directory.
				;Lock the AppName.
		move.l	d3,d1
		move.l	#ACCESS_READ,d2
		JSRLIB	Lock			;Get a lock on the directory for the AppName.
		move.l	d0,d5
		beq.s	dd_NoApp
				;Change to the AppName directory.
		move.l	d0,d1
		JSRLIB	CurrentDir
		move.l	d4,d1
		JSRLIB	DeleteFile
		move.l	d0,d7
		move.l	d5,d1			;All done with the AppName directory.
		JSRLIB	UnLock
dd_NoApp:	move.l	nv_DiskLock(a5),d1	;All done with the users nonvolatile device.
		JSRLIB	CurrentDir
		move.l	d3,d1
		JSRLIB	DeleteFile		;Try deleting the AppName directory.  Will only succeed if it is empty.
		move.l	d6,d1			;All done with the users nonvolatile device.
		JSRLIB	CurrentDir
dd_NoDisk:	move.l	d7,d0			;Set up return value.
		movem.l	(sp)+,d3-d7/a0-a1/a5-a6	;Restore the registers.
		rts


		end
