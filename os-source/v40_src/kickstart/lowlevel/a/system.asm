******************************************************************************
*
*	$Id: system.asm,v 40.1 93/03/12 21:06:30 Jim2 Exp $
*
******************************************************************************
*
*	$Log:	system.asm,v $
* Revision 40.1  93/03/12  21:06:30  Jim2
* When playing with the calling routine I noted that it already had
* ExecBase in a useful place.  Also that LowLevelBase was in a5.
* 
* Revision 40.0  93/03/02  13:22:19  Jim2
* Changed all references from game.library to lowlevel.library
*
* Revision 39.6  93/01/13  13:44:03  Jim2
* No longer use Forbid when taking over the system.
*
* Revision 39.5  93/01/07  14:25:40  Jim2
* Cleaned up the comments and minimized the list of included files.
*
* Revision 39.4  93/01/05  12:10:20  Jim2
* Had .s modifing commands when I wanted .b.
*
* Revision 39.3  92/12/14  14:56:22  Jim2
* Nest multiple calls for owning the system.  Tested.
*
* Revision 39.2  92/12/11  14:06:36  Jim2
* This became greatly simpler.  The process checking has
* migrated out to SystemControlA and nest counts have been
* added.
*
* Revision 39.1  92/12/08  16:53:40  Jim2
* Tested the functions here in.
*
* Revision 39.0  92/12/07  16:38:17  Jim2
* Initial release prior to testing.
*
*
*	(C) Copyright 1992,1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

	INCLUDE	'exec/macros.i'

	INCLUDE	'/lowlevelbase.i'



		XDEF	TakeOverSystem





*****i* lowlevel.library/TakeOverSystem **************************************
*
*   NAME
*	TakeOverSystem - Shutdown or reenable multitasking.
*
*   SYNOPSIS
*	Success = TakeOverSystem (systemState, LowLevelBase, ExecBase)
*	D0			  D1	       a5	     a6
*
*	BOOL TakeOverSystem (BOOL);
*
*   FUNCTION
*	This function is used to give the game developers a warm and fuzzy
*	feeling.  To 'shutdown' multitasking the process is given a high
*	priority.
*
*	The result is the process that 'owns' the system will stay on the
*	CPU until a wait is invoked.  Since this places the process on the
*	waiting queue.  When this process returns to the ready queue it
*	will be rescheduled at the next available time quanta.
*
*   INPUTS
*	SystemState - boolean that determines whether the multitasking is to
*		      be shutdown or reenabled.
*
*   RESULT
*	NONE
*
******************************************************************************
TakeOverSystem
		tst.w	d1
		bne.s	OwnSystem
						;The user wants give up the system.
		subq.b	#1,ll_NestOwn(a5)	;Decrement the nest count.
		bpl.s	Exit
						;The nest count is -1 so return the system.
		bra.s	SwapPriority
OwnSystem					;The user wants to own the system.
		addq.b	#1,ll_NestOwn(a5)	;Increment the Next count.
		bne.s	Exit
						;The nest count is 0 so take over the system.
						;Swap the current task priority with the stored task priority.
SwapPriority:	move.l	ll_SystemOwner(a5),a1
		move.b	ll_TaskPri(a5),d0
		JSRLIB	SetTaskPri		;SetTaskPri(a1>Task,d0[7:0]>Priority,a6>ExecBase)
		move.b	d0,ll_TaskPri(a5)	;Save the original Priority.
Exit:		rts
		END
