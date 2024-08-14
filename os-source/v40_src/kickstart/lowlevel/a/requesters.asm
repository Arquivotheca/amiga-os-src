******************************************************************************
*
*	$Id: requesters.asm,v 40.6 93/04/19 13:41:11 Jim2 Exp $
*
******************************************************************************
*
*	$Log:	requesters.asm,v $
* Revision 40.6  93/04/19  13:41:11  Jim2
* The location for the EasyRequest vector has been changed.
* 
* Revision 40.5  93/03/19  11:33:56  Jim2
* Have to use the RTS in the set functioned code.  If we don't then
* the set function could crash in an unexpected manner if the
* user forgets to release requesters.
*
* Revision 40.4  93/03/12  21:04:40  Jim2
* I changed the calling code so it nolonger says the requesters
* are a resource that locks system ownership.  This means I can
* use a single byte to remember whether requesters are enabled
* rather than seperating out the byte for system ownership.
*
* Revision 40.3  93/03/10  19:54:09  Jim2
* Allowing for expunging requires the setfunctioned code stay around.
* This changed how this code addresses it.  Also we really
* should clear the cache when we modify the code.
*
* Revision 40.2  93/03/08  10:48:22  Jim2
* Added the new entry points for nonvolatile.library.  Changed
* the mechanisim for disabling the requesters from poking a minus
* one in the screen pointer to changing the setfuntion for
* EasyRequest.
*
* Revision 40.1  93/03/02  13:21:54  Jim2
* Changed all references from game.library to lowlevel.library
*
* Revision 39.6  93/01/07  14:24:32  Jim2
* Cleaned up comments and minimized the directly included files.
*
* Revision 39.5  93/01/05  12:08:28  Jim2
* Had .s on commands where I needed .b.  Also, forgot to retrieve
* ExecBase before calling Forbid.  This was masked previously
* due to the .s.b error.
*
* Revision 39.4  92/12/14  14:57:12  Jim2
* Nest multiple calls, Only allow one task to kill requesters.  Tested.
*
* Revision 39.3  92/12/11  14:05:04  Jim2
* Only one process can affect the requesters, so all of the
* process specific information was removed from this routine
* This routine is only called by SystemControlA that checks
* the current process before calling this one.
*
* Revision 39.2  92/12/09  17:55:38  Jim2
* Now it links with the code.  Removed the code that allocates
* context information.  This is now accomplished at library
* open time.
*
* Revision 39.1  92/12/08  16:54:01  Jim2
* Corrected errors that were parallels with System.ASM version 39.0.
*
* Revision 39.0  92/12/07  16:37:28  Jim2
* Initial release prior to testing.
*
*
*	(C) Copyright 1992,1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

	INCLUDE 'exec/macros.i'
	INCLUDE	'dos/dosextens.i'

	INCLUDE	'/lowlevelbase.i'


		XDEF	TurnOffRequesters
		XDEF	KillReq
		XDEF	RestoreReq


*****l* lowlevel.library/TurnOffRequesters ***********************************
*
*   NAME
*	TurnOffRequesters - Inhibit any requesters in the system.
*
*   SYNOPSIS
*	TurnOffRequesters (requesterState, LowLevelBase)
*			   D1		   a6
*
*	TurnOffRequesters (BOOL);
*
*   FUNCTION
*	This function plays with a value monitored by the easyrequest patch
*	that determines whether any requesters should be shown or not.
*
*   INPUTS
*	SystemState - boolean that determines whether the requesters are
*		      inhibited, or allowed.
*
*   RESULT
*	NONE
*
******************************************************************************
*****l* lowlevel.library/KillReq *********************************************
*
*   NAME
*	KillReq - Inhibit all requesters in the system.
*
*   SYNOPSIS
*	KillReq ()
*
*	VOID KillReq (VOID);
*
*   FUNCTION
*	This function plays with a value monitored by the easyrequest patch
*	that determines whether any requesters should be shown or not.
*
*   INPUTS
*	NONE
*
*   RESULT
*	NONE
*
******************************************************************************
*****l* lowlevel.library/RestoreReq ******************************************
*
*   NAME
*	RestoreReq - Allow requesters to happen
*
*   SYNOPSIS
*	RestoreReq ()
*
*	VOID RestoreReq (VOID);
*
*   FUNCTION
*	This function plays with a value monitored by the easyrequest patch
*	that determines whether any requesters should be shown or not.
*
*   INPUTS
*	NONE
*
*   RESULT
*	NONE
*
******************************************************************************
TurnOffRequesters
		tst.w	d1
		bne.s	KillReq
						;The user wants requesters.
RestoreReq
		subq.b	#1,ll_NestReq(a6)	;Decrement the nest count.
		bpl.s	rr_Exit

		move.l	ll_ERSetFunction(a6),a0
		move.l	llsf_Original(a0),llsf_Routine(a0)
		move.l	a6,-(sp)
		move.l	ll_ExecBase(a6),a6
		JSRLIB	CacheClearU
		move.l	(sp)+,a6
rr_Exit:	rts
KillReq:					;The user does not wants requesters.
		addq.b	#1,ll_NestReq(a6)	;Increment the Nest count.
		bne.s	kr_Exit

		move.l	ll_ERSetFunction(a6),a1
		lea	llsf_RTS(a1),a0
		move.l	a0,llsf_Routine(a1)
		move.l	a6,-(sp)
		move.l	ll_ExecBase(a6),a6
		JSRLIB	CacheClearU
		move.l	(sp)+,a6
kr_Exit		rts

		END
