******************************************************************************
*
*	$Id: vblank.asm,v 40.5 93/07/30 16:08:05 vertex Exp $
*
******************************************************************************
*
*	$Log:	vblank.asm,v $
* Revision 40.5  93/07/30  16:08:05  vertex
* Autodoc and include cleanup
* 
* Revision 40.4  93/05/05  09:45:56  gregm
* autodoc changes
*
* Revision 40.3  93/03/23  14:39:51  Jim2
* Did some work polishing the autodocs.
*
* Revision 40.2  93/03/10  12:21:50  Jim2
* Cleaned up results of Vertex code walkthrough.
*
* Revision 40.1  93/03/02  13:37:40  Jim2
* Changed all references from game.library to lowlevel.library.
*
* Revision 40.0  93/02/22  13:13:51  Jim2
* AddVBlankInt saves the interrupt structure relative a5 not
* ExecBase (a6).
*
* Revision 39.1  93/01/18  13:21:57  Jim2
* Cleaned and formatted comments.
*
* Revision 39.0  93/01/15  13:46:46  Jim2
* Initial Release - Untested.
*
*
*
*	(C) Copyright 1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

	INCLUDE	'exec/macros.i'
	INCLUDE	'exec/interrupts.i'
	INCLUDE	'exec/memory.i'

	INCLUDE	'hardware/intbits.i'

	INCLUDE	'/macros.i'
	INCLUDE	'/lowlevelbase.i'




		XDEF	AddVBlankInt
		XDEF	RemVBlankInt


******* lowlevel.library/AddVBlankInt ****************************************
*
*   NAME
*	AddVBlankInt -- adds a routine executed every vertical blank. (V40)
*
*   SYNOPSIS
*	intHandle = AddVBlankInt(intRoutine, intData);
*	D0		         a0	  a1
*
*	APTR AddVBlankInt(APTR, APTR);
*
*   FUNCTION
*	Lets you attach a routine to the system which will get called
*	everytime a vertical blanking interrupt occurs.
*
*	The routine is called from within an interrupt, so normal
*	restrictions apply. The routine must preserve the following
*	registers: A2, A3, A4, A7, D2-D7. Other registers are
*	scratch, except for D0, which MUST BE SET TO 0 upon
*	exit. On entry to the routine, A1 holds 'intData' and A5
*	holds 'intRoutine'.
*
*	If your program is to exit without reboot, you MUST call
*	RemVBlankInt() before exiting.
*
*	Only one interrupt routine may be added to the system.  ALWAYS check
*	the return value in case some other task has previously used this
*	function.
*
*   INPUTS
*	intRoutine - the routine to invoke every vblank. This routine should
*		     be as short as possible to minimize its effect on overall
*		     system performance.
*	intData - data passed to the routine in register A1. If more than one
*		  long word of data is required this should be a pointer to
*		  a structure that contains the required data.
*
*   RESULT
*	intHandle - a handle used to manipulate the interrupt, or NULL
*		    if it was not possible to attach the routine.
*
*   SEE ALSO
*	RemVBlankInt()
*
******************************************************************************
AddVBlankInt

		movem.l	a0-a1/a5-a6,-(sp)
		move.l	a6,a5
		move.l	ll_ExecBase(a5),a6
		move.l	#IS_SIZE,d0
		move.l	#MEMF_PUBLIC|MEMF_CLEAR,d1
		JSRLIB	AllocMem		;Get memory for an interrupt structure.
		tst.l	d0
		beq.s	NoMem

		JSRLIB	Forbid			;Forbid does not change the value in d0.
		tst.l	ll_VBlankInt(a5)
		bne.s	avbi_ErrorExit		;Only one interrupt allowed.

		move.l	d0,ll_VBlankInt(a5)
		JSRLIB	Permit			;Permit does not change the value in d0.
		move.l	d0,a1
		move.b	#NT_INTERRUPT,LN_TYPE(a1)
		move.l	(sp)+,IS_CODE(a1)
		move.l	(sp)+,IS_DATA(a1)	;Initialize the structure.
		move.l	LN_NAME(a5),LN_NAME(a1)
		move.l	#INTB_VERTB,d0		;Add it to the list of interrupts.
		JSRLIB	AddIntServer
		move.l	ll_VBlankInt(a5),d0
		movem.l	(sp)+,a5-a6
		rts

avbi_ErrorExit:	JSRLIB	Permit			;Permit does not change d0.
		move.l	#IS_SIZE,a1
		exg	a1,d0
		JSRLIB	FreeMem
		CLEAR	d0
NoMem:		movem.l	(sp)+,a0-a1/a5-a6
		rts

******* lowlevel.library/RemVBlankInt ****************************************
*
*   NAME
*	RemVBlankInt -- remove a previously installed vertical blank routine.
*			(V40)
*
*   SYNOPSIS
*	RemVBlankInt(intHandle);
*		     A1
*
*	VOID RemVBlankInt(APTR);
*
*   FUNCTION
*	Removes a vertical blank interrupt routine previously added with
*	AddVBlankInt().
*
*   INPUTS
*	intHandle - handle obtained from AddVBlankInt(). This may be NULL,
*		    in which case this function does nothing.
*
*   SEE ALSO
*	AddVBlankInt()
*
******************************************************************************
RemVBlankInt
		movem.l	a5/a6,-(sp)		;Save the parameter and scratch registers.
		move.l	a6,a5
		move.l	ll_ExecBase(a5),a6
		move.l	a1,-(sp)
		beq.s	NullPtr

		clr.l	ll_VBlankInt(a5)	;Clear the remembered structure.
		move.l	#INTB_VERTB,d0
		JSRLIB	RemIntServer		;Remove it from the chain.
		move.l	(sp)+,a1
		move.l	#IS_SIZE,d0
		JSRLIB	FreeMem			;Free the memory for the structure.
		movem.l	(sp)+,a5/a6
		rts
	;NothingToDo:	JSRLIB	Permit		April 21, 1993 This is a dead line and should be removed.
NullPtr		movem.l	(sp)+,a1/a5-a6
		rts

		end
