**
**	$Id: proc.asm,v 36.4 91/02/07 12:50:24 darren Exp $
**
**      ramlib process
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	SECTION	ramlib,code

**	Includes

	INCLUDE	"ramlib.i"

**	Exports

	XDEF	ProcSegment

**	Imports

	XLVO	GetMsg			; Exec
	XLVO	ReplyMsg		;
	XLVO	WaitPort		;

	XREF	LoadModule


**********************************************************************
	CNOP	0,4
		dc.l	0		; allocation size for fake segment list
ProcSegment:
		; fake ramlib process segment
		dc.l	0		; next segment in fake segment list

*
*   The ramlib.process will loop on the following:
*
*   4.	Wait at the request message port for a module request, invoke
*	LoadModule, and reply the message.
*

		;-- find exec & ramlib data
		move.l	ABSEXECBASE,a6
		move.l	ex_RamLibPrivate(a6),a5

		;-- initialize the rest of the rl_LoadPort
		move.l	ThisTask(a6),a2
		move.l	a2,rl_LoadPort+MP_SIGTASK(a5)
		;	clr FLAGS, set SIGBIT
		move.w	#SIGB_SINGLE,rl_LoadPort+MP_FLAGS(a5)
		bra.s	pGetMsg


		;-- wait for module request
pWaitPort:
		lea	rl_LoadPort(a5),a0
		CALLLVO	WaitPort

		;-- get any module requests
pGetMsg:
		lea	rl_LoadPort(a5),a0
		CALLLVO	GetMsg
		tst.l	d0
		beq.s	pWaitPort

		;-- invoke LoadModule with this request
		move.l	d0,a3
		lea	-op_SR_LMMsg(a3),a4
		move.l	op_CurrentDir(a4),pr_CurrentDir(a2)
		move.l	op_HomeDir(a4),pr_HomeDir(a2)
		move.l	op_WindowPtr(a4),pr_WindowPtr(a2)

		bsr	LoadModule

		;-- reply the module request
		move.l	a3,a1
		CALLLVO	ReplyMsg

		bra.s	pGetMsg

	END
