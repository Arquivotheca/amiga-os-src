******************************************************************************
*
*	$Id: setdiskprotection.asm,v 40.3 93/03/09 14:41:19 brummer Exp Locker: vertex $
*
******************************************************************************
*
*	$Log:	setdiskprotection.asm,v $
* Revision 40.3  93/03/09  14:41:19  brummer
* Check status after call to examine per Martin's request.
* 
* Revision 40.2  93/02/25  08:52:10  Jim2
* Documentation change.
*
* Revision 40.1  93/02/19  14:53:18  Jim2
* Changed the labels.
*
* Revision 40.0  93/02/18  10:38:45  Jim2
* Initial release - tested.
*
*
*	(C) Copyright 1992,1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

		INCLUDE	'exec/macros.i'

		INCLUDE	'dos/dos.i'


		INCLUDE	'/nonvolatilebase.i'

		XDEF	SetDiskProtection

*****i* nonvolatile.library/SetDiskProtection ********************************
*
*   NAME
*	SetDiskProtection - Attempts to change the protection of a file.
*
*   SYNOPSIS
*	SetDiskProtection (AppName, ItemName, NVBase, Protection, Success)
*			   a3	    a4	      a5      d2	   d3
*
*	SetDiskProtection (STRPTR, STRPTR, struct NVBase *, BOOL)
*
*   FUNCTION
*	Changes the delete bit for a file to the state indicated by the
*	corresponding bit it Protection.
*
*	Since there is no change protection call, this routine will read
*	the current protection and if there is a change in the delete
*	bit it will call the DOS routine to set the protection bit.  The
*	value returned by this DOS call is ORed into the Success flag.
*
*   INPUTS
*	AppName - Pointer to a NULL terminated string.  This is the
*		  directory that should contain the file.
*	ItemName - Pointer to a NULL terminated string.  This is the file
*		   file.
*	NVBase - Pointer the base of nonvolatile.library.
*	Protection - New protection status.  Only the least significant bit
*		     bit (delete) is used.
*	Success - Whether the protection was changed on a copy of this
*		  data on another device.
*
*   RESULT
*	Success - The return value from SetProtection ORed with the previous
*		  value of Success.
*
*   SEE ALSO
*	SetNVRAMProtection()
*
******************************************************************************
SetDiskProtection
		movem.l	d4-d5/a3/a6,-(sp)
		move.l	nv_DOSBase(a5),a6
		move.l	nv_DiskLock(a5),d1
		beq.s	sdp_Cleanup

		JSRLIB	CurrentDir		;Change to the directory the user has picked for nonvolatile storage.
		move.l	d0,-(sp)		;Remember the orginal directory.
		move.l	d2,-(sp)
		move.l	#ACCESS_READ,d2
		move.l	a3,d1
		JSRLIB	Lock
		move.l	d0,d5
		JSRLIB	CurrentDir
		move.l	a4,d1
		JSRLIB	Lock
		move.l	d0,d4
		move.l	d4,d1
		beq.s	sdp_Unlock

		sub.l	#fib_SIZEOF+4,sp	;Need a FIB, but must be long word aligned.
		move.l	sp,d2
		addq.l	#3,d2
		and.b	#$FC,d2			;Forced long word alignment.
		move.l	d2,a3
		JSRLIB	Examine
		tst.l	d0			; success ?
		beq.s	sdp_Unlock		; if no, j to exit
		move.l	fib_Protection(a3),d2
		add.l	#fib_SIZEOF+4,sp	;Don't need the FIB anymore.
		move.l	d4,d1
		JSRLIB	UnLock
		move.l	(sp),d0
		lsr.b	#1,d2
		lsr.b	#1,d0
		roxl.b	#1,d2
		move.l	a4,d1
		JSRLIB	SetProtection
		or.w	d0,d3			;Or the return value with the previous value of Success.
sdp_Unlock:	move.l	(sp)+,d2
		move.l	(sp)+,d1
		JSRLIB	CurrentDir
		move.l	d5,d1
		beq.s	sdp_Cleanup
		JSRLIB	UnLock
sdp_Cleanup:	movem.l	(sp)+,d4-d5/a3/a6
		rts

		end
