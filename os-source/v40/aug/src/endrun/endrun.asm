		TTL	EndRun
*
******* EndRun ***************************************************************
*
*   NAME
*	EndRun
*
*   SYNOPSIS
*	EndRun [command] - Execute a given command with Workbench closed
*
*   FUNCTION
*	EndRun closes the initial CLI window and Workbench screen,
*	then executes the given command.  Runing with the screen closed
*	saves memory space.  EndRun is the *only* officially supported way
*	to run with a closed Workbench screen under both 1.3 and 2.0
*	Kickstarts.
*
*   INPUTS
*	[command] is the command line to be executed.  The first
*	word in the command will be the load file as passed to LoadSeg().
*	If no command line is given, EndRun will simply halt.
*
*   RESULTS
*	The given command is run with the Workbench screen closed.
*
*   EXAMPLE
*	; startup-sequence for SnakePit
*	EndRun SnakePit datafile
*	; Note that we run the snakepit program with the
*	; argument "datafile" which is passed to the program.
*
*   NOTES
*	EndRun exists because programmers have used a variety of
*	unsupportable tricks to recover the memory used by the Workbench
*	screen.  EndRun is guaranteed to continue functioning in future
*	releases.
*
*	Note that 2.0-specific programs don't need EndRun.  Disks
*	installed with 2.0 install are set to "silent" mode.  The
*	Workbench screen remains closed until the first output.  Simply
*	ensure that all output is redirected to >NIL: <NIL:, and the
*	Workbench screen will never open.
*
*	The [command] will not have any functioning stdin/stdout.
*	stdin/stdout will be connected to NIL:
*
*	EndRUN *MUST* have V1.2 or greater kickstart.  It also *MUST*
*	be executed from a CLI (usually from the startup-sequence).
*
*	EndRun has a maximum COMMAND name (first word in the command line)
*	of 64 characters.  It will not work correctly with more.
*	The rest of the command line can be as long as you wish.
*
*	When the program exits, EndRun will just halt.  It will go
*	into a dead loop.  However, applications run with EndRun
*	are usually of the type the user uses with a reboot as this
*	utility is design for use in startup sequences.
*
******************************************************************************
*
* The following INCLUDE files are needed to make this program assemble.
* They come with the Amiga Macro-Assembler Package.
*
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/interrupts.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/io.i"
	INCLUDE	"exec/ables.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE	"libraries/dos.i"
	INCLUDE	"libraries/dosextens.i"
	INCLUDE	"intuition/intuition.i"
	INCLUDE	"intuition/screens.i"
	INCLUDE	"intuition/intuitionbase.i"
	INCLUDE	"hardware/custom.i"
	INCLUDE	"hardware/dmabits.i"
*
	INCLUDE	"endrun_rev.i"
*
*******************************************************************************
*
* This is the only fixed address in the system and it even "floats"...
*
		xref	_AbsExecBase
*
*******************************************************************************
*
* Some macros that make calling the system routines easier...
*
CALLSYS		MACRO
		IFND	_LVO\1
		xref	_LVO\1		; Set the external reference
		ENDC
		CALLLIB	_LVO\1		; Call the EXEC definied macro
		ENDM
*
*******************************************************************************
*
* Ok, so we get started here...  First save everything and check/close
* workbench if needed...
*
EndRun:		pea	rts_add(pc)		; Put this on the stack...
		movem.l	d1-d7/a1-a6,-(sp)	; We will make sure for full
						; compatibility that register
						; are the same later...
		movem.l	d0/a0,-(sp)		; we need these last...
		move.l	_AbsExecBase,a6		; Get ExecBase
		lea	DOSName(pc),a1		; Get dos's name
		moveq.l	#0,d0			; Any version will do
		CALLSYS	OpenLibrary		; Open it...
		move.l	d0,d6			; Save it in d6...
		beq.s	MajorProblem		; If we did not get it, BOOM!
*
		lea	IName(pc),a1		; Get intuition's name
		moveq.l	#0,d0			; Minimum version
		CALLSYS	OpenLibrary		; Open it...
		move.l	d0,a5			; We need it in an address reg
		move.l	a5,d0			; Check if it opened!
		bne.s	Continue1		; We have a major problem here!
*
* MajorProblem - Well, we are about to bomb out here...
*
MajorProblem:	; What do we do?
		; Alert maybe?
*
* We get here if there is no CLI!
*
No_CLI:		; What to do if we are not a CLI?
		; I say PUNT...
*
* Punt just "exits" which will return to the extra return address
* which then will (possibly) at some point return to the DeadLoop.
*
Punt:		lea	DeadLoop(pc),a0		; Get exit deadloop...
		move.l	a0,16*4(sp)		; Set into stack...
		movem.l	(sp)+,d0/a0
		movem.l	(sp)+,d1-d7/a1-a6	; Restore
rts_add:	rts				; return...
*
* This is just a "dead loop" that will be placed into the stack in order
* to prevent this from ever exiting...
*
DeadLoop:	bra.s	DeadLoop
*
* This is a nasty hack.  We clean up the file handle such that the
* file handle's buffers are "gone"  The BPTR for the filehandle
* is passed in d0.  It also closes the file handle...
*
CleanUpFH:	move.l	d0,d1			; Put into d1 also...
		beq.s	NoClose			; If NULL, skip...
		add.l	d0,d0			; Convert to real pointer
		add.l	d0,d0			; from BCPL
		move.l	d0,a0			; Put into a0
		moveq.l	#0,d0			; Get a ZERO
		move.l	d0,fh_Buf(a0)		; Clear fh_BUF
		subq.l	#1,d0			; Get a -1
		move.l	d0,fh_Pos(a0)		; Set POS and
		move.l	d0,fh_End(a0)		; END to invalid...
		CALLSYS	Close			; Close it...
NoClose:	rts
*
* Now, get the task/process and the CLI structures...
*
Continue1:	move.l	ThisTask(a6),a4		; Our task...
		clr.l	pr_WindowPtr(a4)	; We don't want a window
		move.l	pr_CLI(a4),d0		; Get CLI pointer...
		beq.s	No_CLI			; If we have no CLI?@!
		add.l	d0,d0			; Shift...
		add.l	d0,d0			; ...by 2 (mul by 4)
		move.l	d0,a3			; It now points to my CLI...
*
* Now, we need to check if the screen is open.  This is don't by checking
* for the screen...  Even under 2.0, it is possible that the screen is opened
* and for this reason we want to make sure we are clean...
*
		tst.l	ib_FirstScreen(a5)	; Check first screen...
		beq.s	No_Screen		; If none, we are golden...
*
* Ok, now we want to close down the current screen.  Since we are in a CLI
* we will need to close the current input and output file handles.  This
* will close our window.  After that, we can call intuition's CloseWorkbench()
* function to close the screen.
*
*
* Now make sure that the current input is closed...
*
		moveq.l	#-1,d0
		move.l	d0,cli_Background(a3)	; Set Background to TRUE
		clr.l	cli_Interactive(a3)	; Set Interactive to FALSE
		clr.l	pr_ConsoleTask(a4)	; Clear console task
*
		exg	a6,d6			; Get DOS_Base into a6
*
		move.l	cli_CurrentInput(a3),d1	; Get CurrentInput
		beq.s	No_CInput		; If NULL...
		cmp.l	cli_StandardInput(a3),d1	; Check it...
		beq.s	No_CInput		; If same, skip...
		CALLSYS	Close			; Close the file handle
		move.l	cli_StandardInput(a3),cli_CurrentInput(a3)
*
* Ok, so now we need to make sure we have closed everything else...
*
No_CInput:	CALLSYS	Input			; Get the input stream
		bsr.s	CleanUpFH		; Clean up the FH...
		CALLSYS	Output			; Get the output stream
		bsr.s	CleanUpFH		; Clean up the FH...
		lea	FName(pc),a0		; Get NIL:
		move.l	a0,d1			; into d1...
		move.l	#MODE_NEWFILE,d2	; Open mode...
		CALLSYS	Open			; Open it...
		move.l	d0,cli_StandardInput(a3)	; Set new input
		move.l	d0,cli_StandardOutput(a3)	; and output...
		move.l	d0,cli_CurrentInput(a3)		; and current in...
		move.l	d0,cli_CurrentOutput(a3)	; and current out...
*
		move.l	d0,pr_CIS(a4)		; ??
		move.l	d0,pr_COS(a4)		; ??
		exg	a6,d6			; Put DOS_Base back...
*
* Ok, so we now are ready to try to close workbench...
*
CloseWB:	exg	a6,a5			; Get IntuitionBase
		CALLSYS	CloseWorkBench		; Close it...
		exg	a6,a5			; Restore IntuitionBase
*
* Now, turn off the RASTER DMA such that we do not see junk in the display
* as we load data in that area of memory (possibly)...  (Only pre-V37)
*
		cmpi.w	#37,LIB_VERSION(a6)	; Check if exec is >= V37
		bcc.s	TooNew			; If less than V37, turn off...
		move.w	#BITCLR!DMAF_RASTER,dmacon+$dff000	; Turn it off
TooNew:
*
* Ok, so we now have no screen or we got here without a screen...
* or the screen did not want to close...
* Lets LoadSeg the file given...
*
No_Screen:	movem.l	(sp)+,d0/a0		; Get command line back...
*
* Now, we will copy the command line to the COMName buffer...
*
		lea	COMName(pc),a2		; Get new buffer...
		move.l	a0,a1			; Get old command line...
check_loop:	cmp.b	#' ',(a1)+		; Check for leading spaces
		beq.s	check_loop		; and strip them...
		subq.l	#1,a1			; Back up to the non-space
com_loop:	move.b	(a1),d1			; Get next byte...
		beq.s	end_com			; if NULL, end of command
		cmp.b	#$A,d1			; If LF, end of command...
		beq.s	end_com
		cmp.b	#' ',d1			; if a space, end of command
		beq.s	end_com
		move.b	(a1)+,(a2)+		; Store it...
		bra.s	com_loop		; and get some more...
end_com:	clr.b	(a2)			; Clear the last byte...
		move.l	a2,d1			; Store it...
		lea	COMName(pc),a2		; Get start of buffer
		sub.l	a2,d1			; Subtract from end...
		lea	COMNameSize(pc),a2	; Get size location
		move.b	d1,(a2)			; Save it...
*
* now copy the arguments
*
		move.l	a0,a2			; Copy arguments
arg_loop:	move.b	(a1)+,(a2)+		; Get next byte...
		bne.s	arg_loop		; Continue until NULL...
		subq.l	#1,a2			; Drop back one...
		move.l	a2,d0			; Get end location.
		sub.l	a0,d0			; Calculate size
*
* Put new size/location on stack again
*
		movem.l	a0/d0,-(sp)		; Save them again
*
* Ok, so we now have the command we want to load, so go and load it...
*
		lea	COMNameSize(pc),a1	; BSTR of comand...
		move.l	a1,d1			; Put into d1
		asr.l	#2,d1			; Make into BPTR
		move.l	d1,cli_CommandName(a3)	; save...
*
* Now, we need to loadseg this guy...
*
		lea	COMName(pc),a0		; Get name...
		move.l	a0,d1			; Put it into the right reg...
		exg	a6,d6			; Get DOS_Base
		CALLSYS	LoadSeg			; Load the segment
		exg	a6,d6			; Get execbase back...
		move.l	d0,cli_Module(a3)	; Store CLI module
		beq	MajorProblem		; No LoadSeg?!?  Major problem!
		add.l	d0,d0			; Shift into real...
		add.l	d0,d0			; ...pointer...
		addq.l	#4,d0			; Point at start of code...
		move.l	d0,15*4(sp)		; Set into stack...
		bra	Punt			; Start the loaded code...
*
*******************************************************************************
*
* We reuse most of this area for the command name string we build...
* Or at least we use parts of it, depending on the command.
*
		cnop	0,4			; Long word allign please...
COMNameSize:	dc.b	0			; Size of the command name
COMName:	VERSTAG
		dc.b	'BufferBufferBuffer',0
DOSName:	dc.b	'dos.library',0
IName:		dc.b	'intuition.library',0
FName:		dc.b	'NIL:',0
*
*******************************************************************************
*
		end
