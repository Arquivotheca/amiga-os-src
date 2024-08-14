******************************************************************************
*
*	$Id: makelist.asm,v 40.2 93/02/18 10:57:58 Jim2 Exp Locker: vertex $
*
******************************************************************************
*
*	$Log:	makelist.asm,v $
* Revision 40.2  93/02/18  10:57:58  Jim2
* Added comments.  SizeItem now returns the protection state
* in the longword after the size.
* 
* Revision 40.1  93/02/16  13:51:15  Jim2
* Replaced the two different routines for getting AppNames and
* ItemNames with a single function.  Added call to get the
* size required for an ItemName.
*
* Revision 40.0  93/02/09  10:24:58  Jim2
* Changed the direction for writing the lists (starting at end
* or beginning).
*
* Revision 39.0  93/02/03  11:30:04  Jim2
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

		XDEF	GetNamesFromDisk
		XDEF	SizeItemDisk

*****i* nonvolatile.library/GetNamesFromDisk *********************************
*
*   NAME
*	GetNamesFromDisk - Creates a list of Names from disk.
*
*   SYNOPSIS
*	FreeByte = GetNamesFromDisk (MemBlock, NVBase, AppName)
*	a2			     a2	       a5      a4
*
*	char * GetNamesFromDisk (char *, struct NVBase *, char *)
*
*   FUNCTION
*	Reads the users nonvolatile device and creates a list of either
*	AppNames, if AppName is a NULL pointer, or ItemNames.  a2 points to
*	the first byte to be used in the list.
*
*   INPUTS
*	MemBlock - Pointer to memory block used for storing Names
*	NVBase - Pointer the base of nonvolatile.library.
*	AppName - Pointer identifying the AppName to be searched, or NULL
*		  if the list of AppNames is to be generated.
*
*   RESULT
*	FreeByte - Pointer to beyond the terminating NULL of AppNames.
*
*   SEE ALSO
*	GetNamesFromNVRAM()
*
******************************************************************************
GetNamesFromDisk
		movem.l	d2-d4/a3/a6,-(sp)
		move.l	nv_DiskLock(a5),d4
		beq.s	Cleanup
				;There is a user specified nonvolatile disk.
		move.l	nv_DOSBase(a5),a6
		moveq.l	#0,d3
		cmp.l	a4,d3
		beq.s	ReadyExamine
				;We are searching for Items within an App.
		move.l	d4,d1
		JSRLIB	CurrentDir		;Change to the directory the user has picked for nonvolatile storage.
		move.l	d0,-(sp)		;Remember the orginal directory.
		move.l	#ACCESS_READ,d2
		move.l	a4,d1
		JSRLIB	Lock
		move.l	d0,d4			;Ok we have the needed lock for the examine.
		move.l	(sp)+,d1
		JSRLIB	CurrentDir		;Return to the orginal directory.
		tst.l	d4
		beq.s	Cleanup
				;Have a lock on an application directory.
		move.l	#$7FFFFFFF,d3		;Change the sorting criteria from find dirs to find files.
ReadyExamine:	sub.l	#fib_SIZEOF+4,sp	;Need a FIB, but must be long word aligned.
		move.l	sp,d2
		addq.l	#3,d2
		and.b	#$FC,d2			;Forced long word alignment.
		move.l	d2,a3
		move.l	d4,d1
		JSRLIB	Examine
Skip:		move.l	d4,d1
		JSRLIB	ExNext			;Move to the next directory entry.
		tst.w	d0			;Is there another entry in this directory?
		beq.s	Done
				;Yep, have a new entry to check.
		cmp.l	fib_DirEntryType(a3),d3
		bpl.s	Skip			;Is the type what we want?
				;Yep, so we need to add the string length.
ScanLength:	addq.w	#1,d0
		move.b	fib_FileName(a3,d0.w),(a2)+
		bne.s	ScanLength		;Increment d0 until the NULL is found.

		bra.s	Skip			;This is a top testing loop.
				;All entries have been processed.
Done:		add.l	#fib_SIZEOF+4,sp	;Don't need the FIB anymore.
		tst.l	d3
		beq.s	Cleanup
				;We have a lock on the App directory that we need to release.
		move.l	d4,d1
		JSRLIB	UnLock
				;The transfer is complete.  Let's clean up and go home.
Cleanup:	movem.l	(sp)+,d2-d4/a3/a6
		rts

*****i* nonvolatile.library/SizeItemDisk *************************************
*
*   NAME
*	SizeItemDisk - Determines the size of an item.
*
*   SYNOPSIS
*	SizeItemDisk (SizePtr, NVBase, AppName, ItemName)
*		      a2       a5      a4	a3
*
*	SizeItemDisk (ULONG *, struct NVBase *, char *, char *)
*
*   FUNCTION
*	Reads the users nonvolatile device changes the reported size of the
*	item if the device reports a larger size.  Also ORs in the protection
*	bits for the file.
*
*   INPUTS
*	SizePtr - Pointer to the current size of the Item.
*	NVBase - Pointer the base of nonvolatile.library.
*	AppName - Pointer identifying the AppName.
*	ItemName - Pointer identifying the ItemName to be sized.
*
*   RESULT
*	SizePtr - The value pointed to is changed if the size on the disk is
*		  larger.
*
*   SEE ALSO
*	SizeItemNVRAM()
*
******************************************************************************
SizeItemDisk
		movem.l	d2-d4/a3/a6,-(sp)
		move.l	nv_DOSBase(a5),a6
		move.l	nv_DiskLock(a5),d1
		beq.s	sid_Cleanup

		JSRLIB	CurrentDir		;Change to the directory the user has picked for nonvolatile storage.
		move.l	d0,-(sp)		;Remember the orginal directory.
		move.l	#ACCESS_READ,d2
		move.l	a4,d1
		JSRLIB	Lock			;Lock the App directory.
		move.l	d0,d4
		JSRLIB	CurrentDir		;Change to the App directory.
		move.l	a3,d1
		JSRLIB	Lock			;Lock the Item file.
		move.l	d0,d3
		move.l	(sp)+,d1
		JSRLIB	CurrentDir		;Have the lock so the directory can be returned to the original.
		move.l	d3,d1
		beq.s	sid_Unlock		;Is the Item lock valid?
				;Have a good lock on the Item file.
		sub.l	#fib_SIZEOF+4,sp	;Need a FIB, but must be long word aligned.
		move.l	sp,d2
		addq.l	#3,d2
		and.b	#$FC,d2			;Forced long word alignment.
		move.l	d2,a3
		JSRLIB	Examine
		move.l	fib_Protection(a3),d0
		or.l	d0,4(a2)		;The protection is stored the long word after size.
		move.l	(a2),d0
		cmp.l	fib_Size(a3),d0
		bgt.s	sid_Done		;Only change this size if this size is larger.
				;It is larger so store it.
		move.l	fib_Size(a3),(a2)
sid_Done:	add.l	#fib_SIZEOF+4,sp	;Don't need the FIB anymore.
		move.l	d3,d1
		JSRLIB	UnLock			;Unlock the Item file.

sid_Unlock	move.l	d4,d1
		beq.s	sid_Cleanup		;Did we lock the App directory.
				;Yep, unlock it too.
		JSRLIB	UnLock
sid_Cleanup:	movem.l	(sp)+,d2-d4/a3/a6
		rts

		end
