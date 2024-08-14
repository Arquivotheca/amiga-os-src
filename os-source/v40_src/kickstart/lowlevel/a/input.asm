******************************************************************************
*
*	$Id: input.asm,v 40.3 93/03/19 11:29:40 Jim2 Exp Locker: gregm $
*
******************************************************************************
*
*	$Log:	input.asm,v $
* Revision 40.3  93/03/19  11:29:40  Jim2
* Used the port structure associated with the setfunction code
* to add ports to all the DoIO incase they every cease being
* IOQuick calls.
* 
* Revision 40.2  93/03/12  20:28:22  Jim2
* During the simplification of the calling routine LowLevelBase
* moved to a5 and ExecBase appeared in a6.
*
* Revision 40.1  93/03/02  13:21:14  Jim2
* Changed all references from game.library to lowlevel.library
*
* Revision 39.0  93/01/19  15:43:25  Jim2
* Initial Release.
*
*
*	(C) Copyright 1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

	INCLUDE	'exec/macros.i'
	INCLUDE 'exec/io.i'
	INCLUDE	'exec/tasks.i'
	INCLUDE	'exec/ports.i'

	INCLUDE	'/macros.i'
	INCLUDE	'/lowlevelbase.i'



		XDEF	TrashInput





*****i* lowlevel.library/TrashInput ******************************************
*
*   NAME
*	TrashInput - Shutdown or reenable input.device.
*
*   SYNOPSIS
*	TrashInput (inputState, LowLevelBase, ExecBase)
*		    D1		a5	      a6
*
*	VOID TakeOverSystem (BOOL);
*
*   FUNCTION
*	This function is used to give the game developers a very simple way
*	to shutdown input without needing to open stuff they don't want.
*
*   INPUTS
*	inputState - boolean that determines whether the input is to be
*		     shutdown or reenabled.
*
*   RESULT
*	NONE
*
******************************************************************************
TrashInput
		tst.w	d1
		bne.s	KillInput
				;The user wants reenable input.
		subq.b	#1,ll_NestKillIn(a5)	;Decrement the nest count.
		bpl.w	Exit
				;The nest count is -1 so turn on input.
		CLEARA	a1
		JSRLIB	FindTask
		move.l	ll_ERSetFunction(a5),a0
		move.l	d0,MP_SIGTASK(a0)	; SigTask
		move.b	#SIGB_SINGLE,MP_SIGBIT(a0)	; SigBit
		move.b	#PA_SIGNAL,MP_FLAGS(a0)	; Signal...

TEMP_SIZE	set	0
		ARRAYVAR Request,IOSTD_SIZE

		ALLOCLOCALS sp
		move.l	a0,Request+MN_REPLYPORT(sp)
		move.l	ll_KBDevice(a5),Request+IO_DEVICE(sp)
		move.w	#CMD_CLEAR,Request+IO_COMMAND(sp)
		lea	Request(sp),a1
		JSRLIB	DoIO
		lea	GAMEPDEVICE(pc),a0
		lea	Request(sp),a1
		moveq	#0,d0
		move.l	d0,d1
		JSRLIB	OpenDevice
		bne.s	NoGamePort
		move.w	#CMD_CLEAR,Request+IO_COMMAND(sp)
		lea	Request(sp),a1
		JSRLIB	DoIO
		lea	Request(sp),a1
		JSRLIB	CloseDevice
NoGamePort
		lea	INPUTDEVICE(pc),a0
		lea	Request(sp),a1
		moveq	#0,d0
		move.l	d0,d1
		JSRLIB	OpenDevice
		bne.s	NoInput
		move.w	#CMD_RESET,Request+IO_COMMAND(sp)
		bra.s	Finish
KillInput			;The user wants to kill input.
		addq.b	#1,ll_NestKillIn(a5)	;Increment the Nest count.
		bne.s	Exit
				;The nest count is 0 so stop input.device.
		CLEARA	a1
		JSRLIB	FindTask
		move.l	ll_ERSetFunction(a5),a0
		move.l	d0,MP_SIGTASK(a0)	; SigTask
		move.b	#SIGB_SINGLE,MP_SIGBIT(a0)	; SigBit
		move.b	#PA_SIGNAL,MP_FLAGS(a0)	; Signal...
		ALLOCLOCALS sp
		move.l	a0,Request+MN_REPLYPORT(sp)
		lea	INPUTDEVICE(pc),a0
		lea	Request(sp),a1
		moveq	#0,d0
		move.l	d0,d1
		JSRLIB	OpenDevice
		bne.s	NoInput
		move.w	#CMD_STOP,Request+IO_COMMAND(sp)
Finish		lea	Request(sp),a1
		JSRLIB	DoIO
		lea	Request(sp),a1
		JSRLIB	CloseDevice
NoInput:	move.l	Request+MN_REPLYPORT(sp),a0
		DONELOCALS sp
		move.b	#PA_IGNORE,MP_FLAGS(a0)
Exit:		rts

GAMEPDEVICE	dc.b	'gameport.device',0
INPUTDEVICE	dc.b	'input.device',0
		END
