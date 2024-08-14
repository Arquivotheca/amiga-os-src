		TTL		ClockTick
*
*******************************************************************************
*									      *
*	ClockTick 1.0							      *
*									      *
*	Copyright (c) 1991 by Michael Sinz    MKSoft Development	      *
*									      *
*	AmigaDOS EXEC release 1.2 or greater...				      *
*									      *
*******************************************************************************
*									      *
*	To assemble, I used CAPE 68K					      *
*									      *
*	# CAPE  68k MakeFile rule...					      *
*	.asm:								      *
*		@CAsm -a $*.asm -i sys:lc/include -cqsy -o $*.o		      *
*		@BLink $*.o lib lib:amiga.lib to $* sc sd nd		      *
*		@List $*						      *
*									      *
*	It should assemble without any problems on most 680x0 assemblers.     *
*									      *
*******************************************************************************
*									      *
*	This program installs a handler into the input food chain.	      *
*	The handler changes the hands of the clock every time a time	      *
*	tick comes down the chain.					      *
*									      *
*	It also setfunctions the SetPointer() call and checks what the	      *
*	the pointer looks like.  If it is the standard clock busy pointer     *
*	from 2.0, it will change the SetPointer call to use the built in one  *
*									      *
*	When you run this program, it will allocate some CHIP memory and      *
*	some of PUBLIC memory to install the handler and images.	      *
*	It then exits...						      *
*									      *
*	It will not let itself be installed more than once...		      *
*									      *
*******************************************************************************
*									      *
*	Reading legal mush can turn your brain into guacamole!		      *
*									      *
*		So here is some of that legal mush:			      *
*									      *
* Permission is hereby granted to distribute this program's source	      *
* executable, and documentation for non-comercial purposes, so long as the    *
* copyright notices are not removed from the sources, executable or	      *
* documentation.  This program may not be distributed for a profit without    *
* the express written consent of the author Michael Sinz.		      *
*									      *
* This program is not in the public domain.				      *
*									      *
* Fred Fish is expressly granted permission to distribute this program's      *
* source and executable as part of the "Fred Fish freely redistributable      *
* Amiga software library."						      *
*									      *
* Permission is expressly granted for this program and it's source to be      *
* distributed as part of the Amicus Amiga software disks, and the	      *
* First Amiga User Group's Hot Mix disks.				      *
*									      *
*******************************************************************************
*
* The following INCLUDE files are needed to make this program assemble.
* They come with the Amiga Macro-Assembler Package.
*
	NOLIST					; No need to list these
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/interrupts.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/io.i"
	INCLUDE	"exec/ables.i"
	INCLUDE	"libraries/dosextens.i"
	INCLUDE	"devices/input.i"
	INCLUDE	"devices/inputevent.i"
	INCLUDE 'intuition/intuitionbase.i'
	INCLUDE 'intuition/intuition.i'
	INCLUDE 'clocktick_rev.i'
	LIST					; Ok, lets start the listing
*
*******************************************************************************
*
* The following defines control some minor features:
*
* If DO_MKSOFT_POINTER is defined, ClockTick will also replace the busy pointer
* of older MKSoft products.  This pointer is the same as used in DiskSpeed 3.1.
*
DO_MKSOFT_POINTER	EQU	1
*
* If RETARGET_POINTER is defined, pointer changes are done by not only changing
* the image directly but by calling SetPointer() as needed.  This does take a
* bit more overhead than current methods.  This is what would be needed for
* A2024 or Moniterm support, however due to a problem with the way sprites work
* on those moniters, there is a slight side effect of a flashing sprite.
* Also, on future systems where the graphics display is retargeted to new
* hardware, this method may be needed.  (Thus the name of RETARGET_POINTER)
*
;RETARGET_POINTER	EQU	1
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
		xref	_LVO\1		; Set the external reference
		CALLLIB	_LVO\1		; Call the EXEC definied macro
		ENDM
*
*******************************************************************************
*									      *
* Register usage through this system...					      *
*									      *
*	a0	- Scrap							      *
*	a1	- Scrap							      *
*	a2	- IO Block						      *
*	a3	- MsgPort						      *
*	a4	- Task							      *
*	a5	- New Memory						      *
*	a6	- ExecBase pointer					      *
*	a7	- Stack pointer...  What else?				      *
*									      *
*	d0	- Scrap							      *
*	d1	- Scrap							      *
*	d2	- Scrap							      *
*	d3	- Scrap							      *
*	d4	- Scrap							      *
*	d5	- Scrap							      *
*	d6	- Scrap							      *
*	d7	- Zero...						      *
*									      *
*******************************************************************************
*
* Now, for the start of the code...
*
ClockTick:	move.l	_AbsExecBase,a6		; Get the EXEC library base
*
		clr.l	d7		; Clear d7...
*
		move.l	d7,a1		; Clear a1
		CALLSYS	FindTask	; Get our task pointer...
		move.l	d0,a4		; Now, move it to a1 for addressing use
		lea	pr_MsgPort(a4),a3
		tst.l	pr_CLI(a4)	; Check if this is a CLI task...
		bne.s	QuickCLI	; If so, skip the next section
*
*******************************************************************************
*
* This section deals with starting from the WorkBench...
* It is just here for completeness...
*
QuickWorkBench:	move.l	a3,a0		; Get message port
		CALLSYS	WaitPort	; Wait for the WorkBench message...
		move.l	a3,a0		; ...it's here, so let's get it
		CALLSYS	GetMsg		; off the message port...
		bra.s	QuickCont	; ...and go to the continue routine
*
*******************************************************************************
*
* The routine was started from the CLI  (prefered)
* Let's store a NULL pointer so that there is no WB message...
*
QuickCLI:	move.l	d7,d0		; No reply message...
*
*******************************************************************************
*
* Continue with the Quick initialization
*
QuickCont:
		move.l	d0,-(sp)	; Save the message pointer...
*
*******************************************************************************
*
* Check if we are running already...
*
		lea	portName(PC),a1	; Get the special port name...
		CALLSYS	FindPort	; Look for it...
		tst.l	d0		; If it is there, we are installed...
		bne	abortNoInput	; If installed, don't do it again...
*
		lea	InputBlock(pc),a2	; Get the I/O block
		move.b	#NT_MESSAGE,LN_TYPE(a2)	; initialize it...
		move.w	#IOSTD_SIZE,MN_LENGTH(a2)
		move.l	a3,MN_REPLYPORT(a2)	; Our reply port...
*
		lea	inputName(pc),a0	; Get input.device name
		move.l	d7,d0		; Clear d0
		move.l	d7,d1		;	and d1
		move.l	a2,a1
		CALLSYS	OpenDevice	; a1 is still set to the I/O block
		tst.l	d0
		bne	abortNoInput
*
*******************************************************************************
*
* We now allocate and copy the image...
*
		moveq.l	#clockSize,d0		; Size of memory
		move.l	d0,d6			; Save it here...
		moveq.l	#MEMF_CHIP,d1		; Type of memory...
		CALLSYS	AllocMem		; Allocate it
		lea	TheClockPtr(pc),a0	; Get pointer to storage...
		move.l	d0,(a0)			; Save image pointer...
		beq	NormalExit		; No memory, no go...
		move.l	d0,a1			; Get pointer...
		lea	TheClock(pc),a0		; Original data...
		subq.l	#1,d6			; Drop by 1...
CopyImageLoop:	move.b	(a0)+,(a1)+		; Now copy it down
		dbra	d6,CopyImageLoop	; Copy all of it...
*
*******************************************************************************
*
* We now allocate and copy the handler...
*
		move.l	#CodeSize,d0		; Size of memory
		move.l	d0,d6			; Save it...
		moveq.l	#MEMF_PUBLIC,d1		; Type of memory
		CALLSYS	AllocMem		; Get the memory
		move.l	d0,a5			; Save it...
		tst.l	d0			; Check it...
		beq	NormalExit		; No memory, no copy...
		move.l	d0,a1			; Destination...
		lea	StartOfCode(pc),a0	; Source
		subq.l	#1,d6			; From 0 to n-1
CopyLoop:	move.b	(a0)+,(a1)+		; Copy it...
		dbra	d6,CopyLoop		; All of it...
*
*******************************************************************************
*
* Now that the copy worked, we clear the local pointer...
*
		lea	TheClockPtr(pc),a0	; Get address...
		move.l	d7,(a0)			; Clear it...
*
*******************************************************************************
*
* Ok, now we put our port name into the port list...
*
		lea	portOffset(a5),a0	; Port structure address...
		move.l	a0,a1			; Save it in a1...
		move.w	#MP_SIZE-1,d0		; Size of message port...
ClearPort:	move.b	d7,(a0)+		; Clear byte...
		dbra	d0,ClearPort		; Do all of it...
		move.b	d0,LN_PRI(a1)		; Set PRI to -1...
		move.b	#NT_MSGPORT,LN_TYPE(a1)	; Set type of node...
		lea	portNameOffset(a5),a0	; Port name address...
		move.l	a0,LN_NAME(a1)		; Save pointer to name
		CALLSYS	AddPort			; Add it to the list...
*
*******************************************************************************
*
* Check to see if we are running in V37 ROM or better.  If so,
* we want to call CacheClearU() to make sure we are safe on future
* hardware such as the 68040.  This section of code assumes that
* a6 points at ExecBase.  a0/a1/d0/d1 are trashed in CacheClearU()
*
		cmpi.w	#37,LIB_VERSION(a6)	; Check if exec is >= V37
		bcs.s	TooOld			; If less than V37, too old...
		CALLSYS	CacheClearU		; Clear the cache...
TooOld:
*
*******************************************************************************
*
* We also need to add our own input handler into the food chain...
*
		lea	HandlerOffset(a5),a0	; Pointer to handler...
		move.b	#51,LN_PRI(a0)		; Handler priority
		move.l	a5,IS_DATA(a0)		; Handler data
		addq.l	#codeOffset,a5		; Point at code...
		move.l	a5,IS_CODE(a0)		; Set the handler code pointer
		subq.l	#codeOffset,a5		; Restore pointer...
		move.l	a2,a1			; Get i/o block
		move.l	a0,IO_DATA(a1)		; Set the data address...
		move.w	#IND_ADDHANDLER,IO_COMMAND(a1)	; the ADD command...
		CALLSYS	DoIO			; All set, handler is running
*
*******************************************************************************
*
* Now, we patch intuition.library
*
* We do not close intuition.library since the patch will need to stay
* Also, we do not check the open of intuition.library as if it did not work,
* we are in more trouble than we could ever report (since not even alert would
* work since that uses intuition!)
*
		lea	intuitionName(pc),a1	; We need to open
		moveq.l	#0,d0			; any version of intuition
		CALLSYS	OpenLibrary		; We assume this works...
		FORBID				; Stop task switching...
		move.l	d0,IBaseOffset(a5)	; Save IBase...
		move.l	d0,a1			; Get ready for the
		lea	NewSetOffset(a5),a3	; Pointer to new entry point
		move.l	a3,d0			; Needed for SetFunction...
		XREF	_LVOSetPointer		; We need this reference...
		lea	_LVOSetPointer,a0	; Function offset...
		CALLSYS	SetFunction		; Put me in...
		subq.l	#4,a3			; Point at old pointer...
		move.l	d0,(a3)			; Save it...
		CALLSYS	Permit
*
*******************************************************************************
*
* The standard exit routines...
* Close anything that we have opened...
*
NormalExit:
		move.l	TheClockPtr(pc),d0	; Get the first allocation...
		beq.s	NoFree			; If we don't need to free it
		move.l	d0,a1			; Pointer to the memory...
		move.l	#clockSize,d0		; Size of memory block...
		CALLSYS	FreeMem			; Free it...
*
NoFree:		move.l	a2,a1		; Get I/O block...
		CALLSYS	CloseDevice	; Close the thing...
abortNoInput:
		move.l	(sp)+,d0	; Get that message back
		beq.s	abortNoWB	; If none, we are done
		move.l	d0,a1		; Get the pointer ready
		FORBID			; manual says we must forbid...
		CALLSYS	ReplyMsg	; ...when returning WB message
abortNoWB:
		rts			; RTS out of this task...
*
*******************************************************************************
*
* The input block...
*
InputBlock:	ds.b	IOSTD_SIZE
*
*******************************************************************************
*
* This is the mouse accel value...
*
StartOfCode:	dc.w	0		; Clock timer count...
*
*******************************************************************************
*
* This is the handler...  It is the hardest working piece of code here...
*
MyHandler:	move.l	a0,-(sp)	; We MUST save what we play with...
*
*******************************************************************************
*
* Now, for the handler loop...  We will look at every event that has been given
* to us.
*
HandlerLoop:	cmpi.b	#IECLASS_TIMER,ie_Class(a0)	; Check for timer
		bne.s	HandlerNext			; If skip it...
*
*	if IECLASS_TIMER, we move the hands by one...
*
*	Since we found a timer event, we do not need to check
*	for more and thus we can trash all of the scrap registers...
*
		move.w	(a1),d0			; Get clock word index...
		addq.l	#1,d0			; Bump by 1...
		moveq.l	#15,d1			; Mask value...
		and.l	d1,d0			; Mask it to the 0-15 range...
		move.w	d0,(a1)			; Store it...
		moveq.l	#changeSize,d1		; The amount to change...
		mulu	d1,d0			; Get offset...
		move.l	TheClockPtr(pc),a1	; Get clock image...
		lea	changeOffset(a1),a1	; Get change area
		lea	Clock0(pc),a0		; Get first clock
		add.l	d0,a0			; Point to real one...
		moveq.l	#(changeSize/4)-1,d1	; Number of long-words (-1)
CopyClock:	move.l	(a0)+,(a1)+		; Copy the words that changed
		dbra	d1,CopyClock		; in the clock...
*
*******************************************************************************
*
* The following code is not needed on current hardware other than A2024
* However, due to a bug in the A2024 support, many sprite movements cause
* the sprite to flash all over the display.
* The code below should be enabled if the sprite system is ever
* made more complex or retargetable graphics is used.
*
	IFD	RETARGET_SPRITE
*
* Now, check if we should SetPointer it...
*
		move.l	IBase(pc),a0		; Get IBase
		move.l	ib_ActiveWindow(a0),d0	; Get active window
		beq.s	NoSet			; No window, no SetPointer...
		move.l	d0,a0			; Now, get the window's
		move.l	wd_Pointer(a0),d0	; pointer...
		move.l	TheClockPtr(pc),a1	; Get our pointer...
		cmp.l	a1,d0			; Check if the same...
		bne.s	NoMoreEvents
*
* The window is active, it has our pointer, and the pointer changed.
* So reset...
*
		movem.l	d2/d3/a6,-(sp)		; Save these...
		moveq.l	#16,d0			; Set up the values used
		move.l	d0,d1			; by SetPointer...
		moveq.l	#-6,d2			; We reset to make A2024 and
		moveq.l	#0,d3			; future hardware work...
		move.l	IBase(pc),a6		; Get IntuitionBase
		CALLSYS	SetPointer		; Call SetPointer
		movem.l	(sp)+,d2/d3/a6		; Restore
	ENDC
*
*******************************************************************************
*
		bra.s	NoMoreEvents
*
* Check for next event...
*
HandlerNext:
		move.l	ie_NextEvent(a0),d0	; Get next event...
		move.l	d0,a0
		bne.s	HandlerLoop		; Go back and do this one...
NoMoreEvents:
		move.l	(sp)+,d0	; Restore our stuff...  d0=a0...
		rts			; Done with handler...
*
*******************************************************************************
*
*	SetPointer( Window, Pointer, Height, Width, XOffset, YOffset )
*		    A0      A1       D0      D1     D2       D3
*
OldSetPointer:	dc.l	0
NewSetPointer:	move.l	OldSetPointer(pc),-(sp)	; We will return to old code
		movem.l	a2/a3/d0/d1/d2/d3,-(sp)	; We need these...
		move.l	a1,d0			; If no pointer, skip...
		beq.s	EndSet			; ...
		cmp.l	TheClockPtr(pc),d0	; Check if it is "ours"
		beq.s	EndSet			; If so, we don't do this...
*
*******************************************************************************
*
* This section checks for the MKSoft ZZZ busy pointer and replaces it too.
* (That pointer is available in the Diskspeed 3.1 source)
*
	IFD	DO_MKSOFT_POINTER
		lea	changeOffset(a1),a2	; Point at the changes area
		lea	OldZZZ(pc),a3		; Point at old image
		moveq.l	#(changeSize/4)-1,d1	; Number of long-words (-1)
ZZZCheckLoop:	move.l	(a2)+,d0		; Get real value...
		sub.l	(a3)+,d0		; Subtract expected value...
		dbne	d1,ZZZCheckLoop		; Check some more...
		beq.s	ReplaceImage		; If not same, we bail...
	ENDC
*
*******************************************************************************
*
* Now, check the area where the hands are and see if it is the same as
* our pointer.  We do not check the whole pointer since we do not have
* one available.  This should be enough of a check anyway.
*
		lea	changeOffset(a1),a2	; Point at the changes area
		lea	Clock0(pc),a3		; Point at our version
		moveq.l	#(changeSize/4)-1,d1	; Number of long-words (-1)
CheckLoop:	move.l	(a2)+,d0		; Get real value...
		sub.l	(a3)+,d0		; Subtract expected value...
		dbne	d1,CheckLoop		; Check some more...
		bne.s	EndSet			; If not same, we bail...
*
* We found a clock that looks like ours...
* So, we now change the SetPointer() parameters to match our
* pointer parameters and call the "real" SetPointer()
*
ReplaceImage:	move.l	TheClockPtr(pc),a1	; Get our replacement...
		moveq.l	#16,d0			; Set our height
		move.l	d0,d1			; and width
		moveq.l	#-6,d2			; and XOffset
		moveq.l	#0,d3			; and YOffset
		move.l	OldSetPointer(pc),a2	; Get address...
		jsr	(a2)			; Call it...
*
* This is a nasty trick:  We just did the SetPointer() with our values
* and do not want to do it again.  Since the address we will RTS to
* is that of the old code, we will change the address on the stack to
* point at our RTS instruction and thus make it a NOP.
*
		lea	EndSetRTS(pc),a2	; Get blank RTS...
		move.l	a2,6*4(sp)		; Change the return address...
*
* Now restore back to the caller's registers and get out...
*
EndSet:		movem.l	(sp)+,a2/a3/d0/d1/d2/d3	; Restore data...
EndSetRTS:	rts				; Return to old code...
*
*******************************************************************************
*
* Pointer to the CHIP memory clock image
*
TheClockPtr:	dc.l	0		; Pointer to clock image...
IBase:		dc.l	0		; IBase pointer...
*
*******************************************************************************
*
* These are the clock images.  Clock0 is the one used to compare with
* the given clock...  (Clock numbers 0 to 15)
*
Clock0:	dc.l	$1FF03FEC,$3FF87FDE,$3FF87FBE,$7FFCFF7F,$7EFCFFFF,$7FFCFFFF,$3FF87FFE,$3FF87FFE,$1FF03FFC
Clock1:	dc.l	$1FF03FFC,$3FF87FFE,$3FF87FE6,$7FFCFF9F,$7EFCFF7F,$7FFCFFFF,$3FF87FFE,$3FF87FFE,$1FF03FFC
	dc.l	$1FF03FFC,$3FF87FFE,$3FF87FFE,$7FFCFFFF,$7EFCFF07,$7FFCFFFF,$3FF87FFE,$3FF87FFE,$1FF03FFC
	dc.l	$1FF03FFC,$3FF87FFE,$3FF87FFE,$7FFCFFFF,$7EFCFF7F,$7FFCFF9F,$3FF87FE6,$3FF87FFE,$1FF03FFC
	dc.l	$1FF03FFC,$3FF87FFE,$3FF87FFE,$7FFCFFFF,$7EFCFFFF,$7FFCFF7F,$3FF87FBE,$3FF87FDE,$1FF03FEC
	dc.l	$1FF03FFC,$3FF87FFE,$3FF87FFE,$7FFCFFFF,$7EFCFFFF,$7FFCFF7F,$3FF87F7E,$3FF87FBE,$1FF03FBC
	dc.l	$1FF03FFC,$3FF87FFE,$3FF87FFE,$7FFCFFFF,$7EFCFFFF,$7FFCFEFF,$3FF87EFE,$3FF87EFE,$1FF03EFC
	dc.l	$1FF03FFC,$3FF87FFE,$3FF87FFE,$7FFCFFFF,$7EFCFFFF,$7FFCFDFF,$3FF87DFE,$3FF87BFE,$1FF03BFC
	dc.l	$1FF03FFC,$3FF87FFE,$3FF87FFE,$7FFCFFFF,$7EFCFFFF,$7FFCFDFF,$3FF87BFE,$3FF877FE,$1FF02FFC
	dc.l	$1FF03FFC,$3FF87FFE,$3FF87FFE,$7FFCFFFF,$7EFCFDFF,$7FFCF3FF,$3FF84FFE,$3FF87FFE,$1FF03FFC
	dc.l	$1FF03FFC,$3FF87FFE,$3FF87FFE,$7FFCFFFF,$7EFCC1FF,$7FFCFFFF,$3FF87FFE,$3FF87FFE,$1FF03FFC
	dc.l	$1FF03FFC,$3FF87FFE,$3FF84FFE,$7FFCF3FF,$7EFCFDFF,$7FFCFFFF,$3FF87FFE,$3FF87FFE,$1FF03FFC
	dc.l	$1FF02FFC,$3FF877FE,$3FF87BFE,$7FFCFDFF,$7EFCFFFF,$7FFCFFFF,$3FF87FFE,$3FF87FFE,$1FF03FFC
	dc.l	$1FF03BFC,$3FF87BFE,$3FF87DFE,$7FFCFDFF,$7EFCFFFF,$7FFCFFFF,$3FF87FFE,$3FF87FFE,$1FF03FFC
	dc.l	$1FF03EFC,$3FF87EFE,$3FF87EFE,$7FFCFEFF,$7EFCFFFF,$7FFCFFFF,$3FF87FFE,$3FF87FFE,$1FF03FFC
	dc.l	$1FF03FBC,$3FF87FBE,$3FF87F7E,$7FFCFF7F,$7EFCFFFF,$7FFCFFFF,$3FF87FFE,$3FF87FFE,$1FF03FFC
*
*******************************************************************************
*
* This is the section of the pointer that needs to match the MKSoft pointer
*
	IFD	DO_MKSOFT_POINTER
OldZZZ:	dc.l	$FBFCFFFC,$70FC7FFC,$7FFE7FFE,$7F0E7FFE,$3FDF3FFF,$7FBE7FFE,$3F0E3FFE,$1FFC1FFC,$07F807F8
	ENDC
*
*******************************************************************************
*
HandlerInfo:	ds.b	IS_SIZE		; The handler structure...
*
portStruct	ds.b	MP_SIZE		; The message port structure...
*
portName	dc.b	'MKSoft ClockTick Installed',0
*
EndOfCode:
*
*******************************************************************************
*
* This is the clock image that will be used to replace clocks from the
* SetPointer call...
*
		ds.w	0
TheClock:	dc.l	0,$040007C0,$000007C0,$01000380,$000007E0,$07C01FF8
ChangeArea:	dc.l	$1FF03FEC,$3FF87FDE,$3FF87FBE,$7FFCFF7F,$7EFCFFFF,$7FFCFFFF,$3FF87FFE,$3FF87FFE,$1FF03FFC
ClockBottom:	dc.l	$07C01FF8,$000007E0,0
TheClockEnd:
*
*******************************************************************************
*
* The data section...
*
inputName	dc.b	'input.device',0
intuitionName	dc.b	'intuition.library',0
		VERSTAG
*
*******************************************************************************
*
* Now some offset type symbols...
*
CodeSize	EQU	EndOfCode-StartOfCode
HandlerOffset	EQU	HandlerInfo-StartOfCode
portOffset	EQU	portStruct-StartOfCode
portNameOffset	EQU	portName-StartOfCode
NewSetOffset	EQU	NewSetPointer-StartOfCode
changeSize	EQU	Clock1-Clock0
changeOffset	EQU	ChangeArea-TheClock
codeOffset	EQU	MyHandler-StartOfCode
IBaseOffset	EQU	IBase-StartOfCode
clockSize	EQU	TheClockEnd-TheClock
*
*******************************************************************************
*
* "A master's secrets are only as good as the
*  master's ability to explain them to others."  -  Michael Sinz
*
*******************************************************************************
*
* And, with that we come to the end of another full-length feature staring
* that wonderful MC680x0 and the Amiga system.  Join us again next time
* for more Wonderful World of Amiga...  Until then, keep boinging...
*
		end
