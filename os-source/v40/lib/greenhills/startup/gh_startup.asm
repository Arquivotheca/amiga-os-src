
************************************************************************
*
*	C Program Startup/Exit (Combo Version: CLI and WorkBench)
*
*       New version for greenhills libc
*
************************************************************************
*
* Source Control:
* --------------
*
* $Header: startup.asm,v 1.1 86/08/25 12:50:07 root Exp $
*
* $Locker: root $
*
* $Log:	startup.asm,v $
* Revision 1.1  86/08/25  12:50:07  root
* Initial revision
*
* Revision 33.6  86/06/12  10:34:36  neil
* changed so any version of dos is OK
* 
* Revision 33.5  86/06/11  19:03:47  neil
* fixed CloseLibrary if open of dos failed
* 
* Revision 33.4  86/06/09  16:48:59  neil
* now processes escapes also
* 
* Revision 33.3  86/06/09  16:18:12  neil
* another checkpoint -- quoted strings now work
* 
* Revision 1.1  85/11/23  13:49:01  neil
* Initial revision
* 
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
	INCLUDE "exec/memory.i"
	INCLUDE "libraries/dos.i"
	INCLUDE "libraries/dosextens.i"
	INCLUDE "workbench/startup.i"


******* Imported *******************************************************

xlib	macro
	xref	_LVO\1
	endm

	xref	_AbsExecBase
	xref	_Input
	xref	_Output

	xref	_main			; C code entry point
	xref	__cleanup	; greenhills cleanup

	xlib	Alert
	xlib	FindTask
	xlib	Forbid
	xlib	GetMsg
	xlib	OpenLibrary
	xlib	CloseLibrary
	xlib	ReplyMsg
	xlib	Wait
	xlib	WaitPort
	xlib	AllocMem
	xlib	FreeMem

	xlib	CurrentDir
	xlib	Open


******* Exported *******************************************************

	xdef	_SysBase
	xdef	_DOSBase

	xdef	_errno
	xdef	_ghstdin
	xdef	_ghstdout
	xdef	_ghstderr

	xdef	_exit			; standard C exit function
	xdef	_WBenchMsg


callsys	macro
	CALLLIB	_LVO\1
	endm

	xref	_LVODebug

debug	macro
	movem.l	d0/d1/a0/a1,-(sp)
	jsr	_LVODebug(a6)
	movem.l	(sp)+,d0/d1/a0/a1
	endm

;defineing this will make this write into location $c0
;DEBUGSTACK	equ	1

************************************************************************
*
*	Standard Program Entry Point
*
************************************************************************
*
*	main (argc, argv)
*	   int  argc;
*	   char *argv[]; 
*
************************************************************************

startup:				; reference for Wack users
		move.l	sp,initialSP	; initial task stack pointer
		move.l	sp,_stackTop
		move.l	d0,dosCmdLen
		move.l	a0,dosCmdBuf

		;------ get Exec's library base pointer:
		move.l	_AbsExecBase,a6
		move.l	a6,_SysBase

		;------ get the address of our task
		suba.l	a1,a1
		callsys	FindTask
		move.l	d0,a4

		move.l	pr_CLI(a4),d0	; from cli?
		beq.s	stack_wb
			asl.l	#2,d0	; dos BullShit
			move.l	d0,a0
			move.l	cli_DefaultStack(a0),d0
			asl.l	#2,d0
			bra.s	do_stack
stack_wb:
		move.l	pr_StackSize(a4),d0	; get from process
do_stack:
		bsr	stack_init	; does this work? Dale
		bne.s	success
			rts		; incase we die for some reason
success:


		;------ are we running as a son of Workbench?
		tst.l	pr_CLI(A4)
		beq	fromWorkbench

;=======================================================================
;====== CLI Startup Code ===============================================
;=======================================================================
fromCLI:
		;------	attempt to open DOS library:
		bsr	openDOS
	

		;------ find command name:
		suba.l	a0,a0
		move.l	pr_CLI(a4),d0
		lsl.l   #2,d0		; bcpl pointer conversion
		move.l	cli_CommandName(a0,d0.l),d0
		lsl.l   #2,d0		; bcpl pointer conversion

		;------ create buffer and array:
		movem.l	a2/a3,-(sp)
		lea	argvBuffer,a2
		lea	argvArray,a3

		;------ fetch command name:
		move.l	d0,a0
		moveq.l	#0,d0
		move.b	(a0)+,d0	; size of command name
		clr.b	0(a0,d0.l)	; terminate the string
		move.l	a0,(a3)+

		;------	collect parameters:
		move.l	dosCmdLen,d0
		move.l	dosCmdBuf,a0

		;------ null terminate the string, eat trailing garbage
		lea	0(a0,d0.l),a1
stripjunk:
		cmp.b	#' ',-(a1)

		;-- jimm: 8/25/86: per kodiak's recommendation
		; bls.s	stripjunk
		dbhi	D0,stripjunk

		clr.b	1(a1)

newarg:
		;------ skip spaces
		move.b	(a0)+,d1
		beq.s	parmExit
		cmp.b	#' ',d1
		beq.s	newarg
		cmp.b	#9,d1			; tab
		beq.s	newarg

		;------ push address of the next parameter
		move.l	a2,(a3)+

		;------ process quotes
		cmp.b	#'"',d1
		beq.s	doquote

		;------ copy the parameter in
		move.b	d1,(a2)+

nextchar:
		;------ null termination check
		move.b	(a0)+,d1
		beq.s	parmExit
		cmp.b	#' ',d1
		beq.s	endarg

		move.b	d1,(a2)+
		bra.s	nextchar

endarg:
		clr.b	(a2)+
		bra.s	newarg

doquote:
		;------ process quoted strings
		move.b	(a0)+,d1
		beq.s	parmExit
		cmp.b	#'"',d1
		beq.s	endarg

		;------ '*' is the BCPL escape character
		cmp.b	#'*',d1
		bne.s	addquotechar

		move.b	(a0)+,d1
		cmp.b	#'N',d1
		beq.s	1$
		cmp.b	#'n',d1
		bne.s	2$

1$:
		;------ got a *N -- turn into a newline
		moveq	#10,d1
		bra.s	addquotechar

2$:
		cmp.b	#'E',d1
		beq.s	3$
		cmp.b	#'e',d1
		bne.s	addquotechar

3$:
		;------ got a *E -- turn into a escape
		moveq	#27,d1

addquotechar:
		move.b	d1,(a2)+
		bra.s	doquote

parmExit:
		;------ all done -- null terminate the baby
		clr.b	(a2)
		clr.l	(a3)

		;------ compute the # of arguments
		move.l	#dosCmdBuf,d0
		sub.l	a3,d0
		not.l	d0
		lsr.l	#2,d0

		movem.l	(sp)+,a2/a3
		pea	envvArray
		pea	argvArray
		move.l	d0,-(sp)


*
*  The above code relies on the end of line containing a control
*  character of any type, i.e. a valid character must not be the
*  last.  This fact is ensured by DOS.
*
		
;
;		;------ get standard input handle:
;		jsr	_Input
;		move.l	d0,_ghstdin
;
;		;------ get standard output handle:
;		jsr	_Output
;		move.l	d0,_ghstdout
;		move.l	d0,_ghstderr
;		we let Init_IO initialize all this

		xref	_Init_IO

		jsr	_Init_IO	; call greenhills

		;------ call C main entry point

		jsr	_main


		;------ return success code:
		moveq.l	#0,D0
		bra	exit2		; dale added 7-22-87

;=======================================================================
;====== Workbench Startup Code =========================================
;=======================================================================
fromWorkbench:
		;------ open the DOS library:
		bsr	openDOS

		tst.w	$1	; cannot run from wb yet

		;------ we are now set up.  wait for a message from our starter
		bsr	waitmsg

		;------ save the message so we can return it later
		move.l	d0,_WBenchMsg

		;------ push the message on the stack for wbmain
		move.l	d0,-(SP)
		clr.l	-(SP)		indicate: run from Workbench

		;------ get the first argument
		move.l	d0,a2
		move.l	sm_ArgList(a2),d0
		beq.s	docons

		;------ and set the current directory to the same directory
		move.l	_DOSBase,a6
		move.l	d0,a0
		move.l	wa_Lock(a0),d1
		callsys	CurrentDir

docons:
		;------ get the toolwindow argument
		move.l	sm_ToolWindow(A2),d1
		beq.s	domain

		;------ open up the file
		move.l	#MODE_OLDFILE,d2
		callsys	Open

		;------ set the C input and output descriptors
		move.l	d0,_ghstdin
		move.l	d0,_ghstdout
		move.l	d0,_ghstderr
		beq.s	domain

		;------ set the console task (so Open( "*", mode ) will work
		;	waitmsg has left the task pointer in A4 for us
		lsl.l	#2,d0
		move.l	d0,a0
		move.l	fh_Type(a0),pr_ConsoleTask(A4)

domain:
		jsr	_main
		moveq.l	#0,d0		Successful return code
		bra.s	exit2


************************************************************************
*
*	C Program Exit Function
*
************************************************************************
*
*  Warning: this function really needs to do more than this.
*
************************************************************************

		xdef	__exit
		xref	__cleanup2
__exit:
		move.l	4(sp),d0
		move.l	initialSP,SP
		move.l	d0,-(sp)
		bra.s	no_stdio_cleanup

_exit:
		move.l	4(SP),d0	; extract return code
exit2:
		move.l  initialSP,SP	; restore stack pointer
		move.l	d0,-(SP)	; save return code

*		must be done before closing dos library
		jsr	__cleanup	; greenhills stdio cleanup

no_stdio_cleanup:
		jsr	__cleanup2	; low level cleanup


		;------ close DOS library:
		move.l	_AbsExecBase,A6
		move.l	_DOSBase,d0
		beq.s	1$
		move.l	d0,a1
		callsys	CloseLibrary
1$:

		;------ if we ran from CLI, skip workbench cleanup:
		tst.l	_WBenchMsg
		beq.s	exitToDOS

		;------ return the startup message to our parent
		;------	we forbid so workbench can't UnLoadSeg() us
		;------	before we are done:
		callsys Forbid
		move.l	_WBenchMsg,a1
		callsys	ReplyMsg

		;------ this rts sends us back to DOS:
exitToDOS:
		move.l	(SP)+,d0
		bsr	freedosstack
		rts


;-----------------------------------------------------------------------
noDOS:
		ALERT	(AG_OpenLib!AO_DOSLib)
		moveq.l	#100,d0
		bra.s	exit2


;-----------------------------------------------------------------------
; This routine gets the message that workbench will send to us
; called with task id in A4

waitmsg:
		lea	pr_MsgPort(A4),a0  	* our process base
		callsys	WaitPort
		lea	pr_MsgPort(A4),a0  	* our process base
		callsys GetMsg
		rts

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

************************************************************************

; fill the empty stack with abcd for debugging purposes.
; enter with sp -> current stack
; we will get the other part from the trask control block
;	a4 points to task control block
;	d0,a1 are scratch registers
;	enter with d0 number of bytes in stack
stack_init:
	move.l	d0,_stackSize		; just in case we need it again
	ifd	DEBUGSTACK
	move.l	sp,$c0
	endc
	move.l	sp,a1
	sub.l	d0,a1
	add.l	#16,a1			; for good measure
	ifd	DEBUGSTACK
	move.l	a1,$c4
	endc
	move.l	a1,_stackBottom		; for program usage
again:	move.w	#$abcd,(a1)+
	cmp.l	a1,sp			; have we reached it yet?
	bne	again
;	allocate 2k ($800) bytes for dos stack, will this be enough?
;	the source says $5dc
	move.l	#MEMF_CLEAR,d1
	move.l	#$800,d0		; size 2k bytes
	jsr	_LVOAllocMem(a6)
	add.l	#$700,d0		; give us $700 bytes of stack
					; and 256 bytes of pad
	ifd DEBUGSTACK
	move.l	d0,$c8
	endc
	move.l	d0,_dosStack		; check for error by caller
	rts

freedosstack:
	movem.l	d0/d1/a0/a1/a6,-(sp)
	move.l	_dosStack,a1
	sub.w	#$700,a1		; reset to actual start
	move.l	#$800,d0
	move.l	_SysBase,a6
	jsr	_LVOFreeMem(a6)
	movem.l	(sp)+,d0/d1/a0/a1/a6
	rts

	DATA

************************************************************************

VerRev		dc.w	33,1

_SysBase	dc.l	0
_DOSBase	dc.l	0

_errno		dc.l	0
_ghstdin		dc.l	-1
_ghstdout		dc.l	-1
_ghstderr		dc.l	-1

		xdef	_dosStack
_dosStack	ds.l	1	; we allocate 600 hex bytes to keep that junk
				; away from C
;		stuff for standard stack
		xdef	_stackTop,_stackBottom,_stackSize
_stackTop	ds.l	1
_stackBottom	ds.l	1
_stackSize	ds.l	1


initialSP	dc.l	0
_WBenchMsg	dc.l	0

dosCmdLen	dc.l	0
dosCmdBuf	dc.l	0

argvArray	ds.l	32
argvBuffer	ds.b	256
envvArray	dc.l	0



	END
