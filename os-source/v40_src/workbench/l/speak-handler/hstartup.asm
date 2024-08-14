
************************************************************************
*
*	C Program Startup/Exit (handler version)
*
************************************************************************

******* Included Files *************************************************

	INCLUDE	"exec/types.i"
	INCLUDE "exec/alerts.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/ports.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/tasks.i"
	INCLUDE "libraries/dos.i"
	INCLUDE "libraries/dosextens.i"
	INCLUDE "workbench/startup.i"
	INCLUDE "my-macros.i"


******* Imported *******************************************************

xlib	macro
	xref	_LVO\1
	endm

	xref	_AbsExecBase
	xref	_Input
	xref	_Output
	xref	Printf

	xref	_main			; C code entry point

	xlib	Alert
	xlib	FindTask
	xlib	Forbid
	xlib	GetMsg
	xlib	OpenLibrary
	xlib	CloseLibrary
	xlib	ReplyMsg
	xlib	Wait
	xlib	WaitPort

	xlib	CurrentDir
	xlib	Open


******* Exported *******************************************************

	xdef	_SysBase
	xdef	_DOSBase

	xdef	_exit			; standard C exit function


callsys	macro
	CALLLIB	_LVO\1
	endm

************************************************************************
*
*	Standard Program Entry Point
*
************************************************************************

startup:			; reference for Wack users
		;------ get Exec's library base pointer:
		move.l	_AbsExecBase,a6
		move.l	a6,_SysBase

		;------ get the address of our task
		bsr findit

		move.l	SP,TC_Userdata(a4) ; initial task stack pointer
*		move.l	SP,initialSP ; initial task stack pointer

		;------	attempt to open DOS library:
		bsr	openDOS

		;------ call C main entry point

		jsr	_main

		;------ return success code:
		moveq.l	#0,D0
		bra exit2

************************************************************************
*
*	C Program Exit Function
*
************************************************************************

_exit:
		move.l	4(SP),d0	; extract return code
exit2:
		move.l  d0,-(SP)	; save return code on current stack
		;------ get the address of our task
		bsr findit
		move.l  (SP)+,d0	; restore return code

		move.l  TC_Userdata(a4),SP ; restore stack pointer
*		move.l  initialSP,SP	

		move.l	d0,-(SP)	; save return code in correct stack

		;------ close DOS library:
		move.l	_AbsExecBase,A6
		move.l	_DOSBase,d0
		beq.s	1$
		move.l	d0,a1
		callsys	CloseLibrary
1$:

		;------ this rts sends us back to DOS:
exitToDOS:
		move.l	(SP)+,d0
		rts


;-----------------------------------------------------------------------
noDOS:
		ALERT	(AG_OpenLib!AO_DOSLib)
		moveq.l	#100,d0
		bra.s	exit2


;-----------------------------------------------------------------------
;  Open the DOS library:

openDOS
		lea	DOSName(pc),A1
		moveq	#0,d0
		callsys OpenLibrary
		move.l	D0,_DOSBase
		beq	noDOS
		rts

DOSName		DOSNAME

;-----------------------------------------------------------------------
; find our task

findit
		;------ get the address of our task
		move.l	_AbsExecBase,A6
		suba.l	a1,a1
		callsys	FindTask
		move.l	d0,a4
		rts


************************************************************************

	DATA

************************************************************************

VerRev		dc.w	33,1

_SysBase	dc.l	0
_DOSBase	dc.l	0
* initialSP       dc.l    0

	END
