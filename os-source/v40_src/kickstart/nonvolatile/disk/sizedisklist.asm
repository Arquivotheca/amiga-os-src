******************************************************************************
*
*	$Id: sizedisklist.asm,v 40.4 93/03/18 11:12:47 brummer Exp Locker: vertex $
*
******************************************************************************
*
*	$Log:	sizedisklist.asm,v $
* Revision 40.4  93/03/18  11:12:47  brummer
* Fix SizeDiskList to return zero when disk is not found.
* 
* Revision 40.3  93/02/25  08:53:30  Jim2
* Documentation.
*
* Revision 40.2  93/02/23  13:23:43  Jim2
* Changed the name of the data type from NVItem to NVEntry.
*
* Revision 40.1  93/02/18  10:59:29  Jim2
* Changed sizing routines to use the label for allowing for the
* size of the array elements.  Thus the next time the size
* changes I won't have to change this file.
*
* Revision 40.0  93/02/16  13:22:39  Jim2
* Included the bytes requried for the new sturctures.
*
* Revision 39.0  93/02/03  11:30:52  Jim2
* Initial Release - Tested.
*
*
*	(C) Copyright 1992,1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

		INCLUDE	'exec/macros.i'

		INCLUDE	'dos/dos.i'


		INCLUDE	'/nonvolatilebase.i'
		INCLUDE	'/nonvolatile.i'

		XDEF	SizeDiskList

*****i* nonvolatile.library/SizeDiskList *************************************
*
*   NAME
*	SizeDiskList - Byte required for a list of Disk items/applications.
*
*   SYNOPSIS
*	Size = SizeDiskList (AppName, NVBase)
*	d0		     a0	      a5
*
*	LONG SizeDiskList (STRPTR, struct NVBase *)
*
*   FUNCTION
*	Determines how large a block of memory is requried to store the
*	List of NVEntry structures and the name strings pointed to by the
*	structures.  The size is the size for the string plus the size for
*	an NVEntry structure.
*
*	If AppName is non NULL only report the size needed to create a
*	list of strings for the Items found for that AppName.
*
*	If AppName is NULL report the size needed for a list of all
*	AppNames and all ItemNames.
*
*
*   INPUTS
*	AppName - Pointer to the application name.  Or NULL if the entire
*		  contents is to be requested.
*
*   RESULT
*	Size - Number of bytes required to create a list.
*
*   SEE ALSO
*	SizeNVRAMList()
*
******************************************************************************
SizeDiskList
		moveq.l	#0,d0			; setup return value if lock fails
		move.l	nv_DiskLock(a5),d1	;Check to see if there is a nonvolatile disk out there.
		beq.s	NoDisk
				;Yes, there is a disk.
		movem.l	d2/d7/a2/a6,-(sp)
		move.l	nv_DOSBase(a5),a6
		moveq.l	#0,d7			;d7 will be used to accumulate the lengths.
		move.l	a0,-(sp)		;Temporarily save the AppName Pointer.
		JSRLIB	CurrentDir		;Change to the directory the user has picked for nonvolatile storage.
		move.l	d0,-(sp)		;Remember the orginal directory.
		move.l	4(sp),d1
		beq.s	EveryApp		;Is this for a single AppName, or all AppNames?
				;The pointer to the single AppName has been loaded into d1.
		bsr.s	SizeApp			;Determine the size needed for this AppName.
		bra.s	Cleanup			;And we are done.
				;Scan the nonvolatile directory for every AppName, size each of them.
EveryApp:	move.l	nv_DiskLock(a5),d1
		sub.l	#fib_SIZEOF+4,sp	;Need a FIB, but must be long word aligned.
		move.l	sp,d2
		addq.l	#3,d2
		and.b	#$FC,d2			;Forced long word alignment.
		move.l	d2,a2
		JSRLIB	Examine
Skip:		move.l	nv_DiskLock(a5),d1
		JSRLIB	ExNext			;Move to the next directory entry.
		tst.w	d0			;Is there another entry in this directory?
		beq.s	DoneDirs
				;Yep, have a new entry to check.
		tst.b	fib_DirEntryType(a2)
		bmi.s	Skip			;Is this another directory?
				;Yep, so we need to add the string length.
LoopingDir:	addq.w	#1,d0
		tst.b	fib_FileName(a2,d0.w)
		bne.s	LoopingDir		;Increment d0 until the NULL is found.

		add.w	#1+NVENTRY_SIZE,d0	;Size is one more than index of the NULL, plus one element to store the AppName.
		ext.l	d0
		add.l	d0,d7			;Add this to any previous value.
		movem.l	d2/a2,-(sp)		;Don't lose the pointers (SizeApp is non standard in register usage.)
		lea	fib_FileName(a2),a0
		move.l	a0,d1			;Place the AppName pointer in d1.
		bsr.s	SizeApp			;Size the AppName directory.
		movem.l	(sp)+,d2/a2		;Restore the pointers.

		bra.s	Skip			;This is a top testing loop.
				;All entries have been processed.
DoneDirs:	add.l	#fib_SIZEOF+4,sp	;Don't need the FIB anymore.
				;The sizing is complete.  Let's clean up and go home.
Cleanup:	move.l	(sp)+,d1
		JSRLIB	CurrentDir		;Restore the orginal directory.
		move.l	d7,d0			;Set up the return value.
		movem.l	(sp)+,d1/d2/d7/a2/a6	;Don't forget we pushed on one additional register.
NoDisk:		rts

*****l* a/SizeDiskList.asm/SizeApp *******************************************
*
*   NAME
*	SizeApp - Bytes required for a list of Disk items.
*
*   SYNOPSIS
*	Size = SizeApp (AppName, PreviousSize)
*	d7		d1	 d7
*
*	LONG SizeApp (STRPTR, ULONG)
*
*   FUNCTION
*	Determines how large a block of memory is requried to store the NULL
*	terminated list of NULL terminated string identifing the items
*	stored for the given AppName.
*
*
*   INPUTS
*	AppName - Pointer to the application name.
*	PrevousSize - Size required for any other AppNames.
*
*   RESULT
*	Size - Number of bytes required to save the list.
*
*   WARNING
*	Trashes a2,d2.
*
******************************************************************************
SizeApp
		move.l	#ACCESS_READ,d2
		JSRLIB	Lock			;Get a lock on the directory for the AppName.
		move.l	d0,d1
		beq.s	NotExist		;Were we sucessful?
				;Got lock now we can scan the directory.
		sub.l	#fib_SIZEOF+4,sp	;Need a FIB, but must be long word aligned.
		move.l	sp,d2
		addq.l	#3,d2
		and.b	#$FC,d2			;Forced long word alignment.
		move.l	d2,a2
		move.l	d1,-(sp)		;Don't want to lose the lock.
		JSRLIB	Examine			;Get the first entry.
ProcessFile:	move.l	(sp),d1			;Restore the lock.
		JSRLIB	ExNext			;Get next entry.
		tst.w	d0
		beq.s	Done			;Did we get a new entry?
				;Yep, so how long is the name?
Looping:	addq.w	#1,d0
		tst.b	fib_FileName(a2,d0.w)
		bne.s	Looping			;Wait for the NULL.

		add.w	#1+NVENTRY_SIZE,d0	;Add one to get the size, not the index number.  And the size of an element
		ext.l	d0
		add.l	d0,d7			;Sum.
		bra.s	ProcessFile		;This is a top testing loop.
				;All done.
Done:		move.l	(sp)+,d1		;Get the lock off the stack.
		JSRLIB	UnLock
		add.l	#fib_SIZEOF+4,sp	;Clear the FIB off the stack.
NotExist:	rts

		end
