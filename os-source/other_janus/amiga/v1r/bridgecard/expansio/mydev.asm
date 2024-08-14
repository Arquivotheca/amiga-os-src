
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
************************************************************************/


*************************************************************************
*
* mydev.asm -- skeleton device code
*
* Source Control
* ------ -------
* 
* $Header: amain.asm,v 31.3 85/10/18 19:04:04 neil Exp $
*
* $Locker: neil $
*
* $Log: amain.asm,v $
*
************************************************************************/
	SECTION section

	NOLIST
	include "exec/types.i"
	include "exec/nodes.i"
	include "exec/lists.i"
	include "exec/libraries.i"
	include "exec/devices.i"
	include "exec/io.i"
	include "exec/alerts.i"
	include "exec/initializers.i"
	include "exec/memory.i"
	include "exec/resident.i"
	include "exec/ables.i"
	include "exec/errors.i"
	include "libraries/dos.i"
	include "libraries/dosextens.i"
	include "libraries/configvars.i"
	include "libraries/expansion.i"
	include "mydevsupp.i"
	include "messages.i"

	include "mydev.i"
	LIST


	;------ These don't have to be external, but it helps some
	;------ debuggers to have them globally visible
	XDEF	Init
	XDEF	Open
	XDEF	Close
	XDEF	Expunge
	XDEF	Null
	XDEF	myName
	XDEF	BeginIO
	XDEF	AbortIO

	XREF	_AbsExecBase

	XLIB	OpenLibrary
	XLIB	CloseLibrary
	XLIB	Alert
	XLIB	FreeMem
	XLIB	Remove
	XLIB	FindTask
	XLIB	AllocMem
	XLIB	CreateProc
	XLIB	PutMsg
	XLIB	RemTask
	XLIB	ReplyMsg
	XLIB	Signal
	XLIB	GetMsg
	XLIB	Wait
	XLIB	WaitPort
	XLIB	AllocSignal
	XLIB	SetTaskPri
	XLIB	Debug
	XLIB	GetCurrentBinding

	INT_ABLES

INFOLEVEL	EQU	30

	; The first executable location.  This should return an error
	; in case someone tried to run you as a program (instead of
	; loading you as a library).
FirstAddress:
	CLEAR	d0
	rts

;-----------------------------------------------------------------------
; A romtag structure.  Both "exec" and "ramlib" look for
; this structure to discover magic constants about you
; (such as where to start running you from...).
;-----------------------------------------------------------------------

	; Most people will not need a priority and should leave it at zero.
	; the RT_PRI field is used for configuring the roms.  Use "mods" from
	; wack to look at the other romtags in the system
MYPRI	EQU	0

initDDescrip:
					;STRUCTURE RT,0
	  DC.W	  RTC_MATCHWORD 	; UWORD RT_MATCHWORD
	  DC.L	  initDDescrip		; APTR	RT_MATCHTAG
	  DC.L	  EndCode		; APTR	RT_ENDSKIP
	  DC.B	  RTF_AUTOINIT		; UBYTE RT_FLAGS
	  DC.B	  VERSION		; UBYTE RT_VERSION
	  DC.B	  NT_DEVICE		; UBYTE RT_TYPE
	  DC.B	  MYPRI 		; BYTE	RT_PRI
	  DC.L	  myName		; APTR	RT_NAME
	  DC.L	  idString		; APTR	RT_IDSTRING
	  DC.L	  Init			; APTR	RT_INIT
					; LABEL RT_SIZE


	; this is the name that the device will have
subSysName:
subsysName:
myName: 	MYDEVNAME

	; a major version number.
VERSION:	EQU	1

	; A particular revision.  This should uniquely identify the bits in the
	; device.  I use a script that advances the revision number each time
	; I recompile.	That way there is never a question of which device
	; that really is.
REVISION:	EQU	17

	; this is an identifier tag to help in supporting the device
	; format is 'name version.revision (dd MON yyyy)',<cr>,<lf>,<null>
idString:	dc.b	'mydev 1.0 (31 Oct 1985)',13,10,0

expansionName:	EXPANSIONNAME

	XDEF	_ExpansionBaseOffset
_ExpansionBaseOffset	EQU	md_ExpansionLib

	; force word allignment
	ds.w	0


	; The romtag specified that we were "RTF_AUTOINIT".  This means
	; that the RT_INIT structure member points to one of these
	; tables below.  If the AUTOINIT bit was not set then RT_INIT
	; would point to a routine to run.

Init:
	DC.L	MyDev_Sizeof		; data space size
	DC.L	funcTable		; pointer to function initializers
	DC.L	dataTable		; pointer to data initializers
	DC.L	initRoutine		; routine to run


funcTable:

	;------ standard system routines
	dc.l	Open
	dc.l	Close
	dc.l	Expunge
	dc.l	Null

	;------ my device definitions
	dc.l	BeginIO
	dc.l	AbortIO

	;------ function table end marker
	dc.l	-1


	; The data table initializes static data structures.
	; The format is specified in exec/InitStruct routine's
	; manual pages.  The INITBYTE/INITWORD/INITLONG routines
	; are in the file "exec/initializers.i".  The first argument
	; is the offset from the device base for this byte/word/long.
	; The second argument is the value to put in that cell.
	; The table is null terminated
dataTable:
	INITBYTE	LH_TYPE,NT_DEVICE
	INITLONG	LN_NAME,myName
	INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
	INITWORD	LIB_VERSION,VERSION
	INITWORD	LIB_REVISION,REVISION
	INITLONG	LIB_IDSTRING,idString
	DC.L	0


	; This routine gets called after the device has been allocated.
	; The device pointer is in D0.	The segment list is in a0.
	; If it returns non-zero then the device will be linked into
	; the device list.
initRoutine:

	;------ get the device pointer into a convenient A register
	move.l	a5,-(sp)
	move.l	d0,a5

		move.l	d0,-(sp)
		INFOMSG 1,<'%s/init: called. lib 0x%lx'>
		addq.l	#4,sp

	;------ save a pointer to exec
	move.l	a6,md_SysLib(a5)

	;------ save a pointer to our loaded code
	move.l	a0,md_SegList(a5)

	;------ open the expansion library
	lea	expansionName(pc),A1
	moveq	#0,d0
	CALLSYS OpenLibrary

	move.l	d0,md_ExpansionLib(a5)
	bne.s	init_expanOK

		INFOMSG 1,<'%s/init: no expansion lib'>
		CALLSYS Debug

init_expanOK:

	link	a4,#-CurrentBinding_SIZEOF

	;------ find all the devices and mark them as used
	move.l	sp,a0
	LINKSYS GetCurrentBinding,md_ExpansionLib(a5)

	move.l	cb_ConfigDev(sp),a0

init_markLoop:
	move.l	a0,d0
	beq.s	init_markEnd

		move.l	a0,-(sp)
		INFOMSG 1,<'%s/init: found dev 0x%lx'>
		addq.l	#4,sp

	bclr	#CDB_CONFIGME,cd_Flags(a0)
	move.l	a5,cd_Driver(a0)
	move.l	cd_NextCD(a0),a0
	bra.s	init_markLoop

init_markEnd:

	;------ clean up the stack
	unlk	a4

	move.l	a5,d0
	move.l	(sp)+,a5
	rts

;----------------------------------------------------------------------
;
; here begins the system interface commands.  When the user calls
; OpenLibrary/CloseLibrary/RemoveLibrary, this eventually gets translated
; into a call to the following routines (Open/Close/Expunge).  Exec
; has already put our device pointer in a6 for us.  Exec has turned
; off task switching while in these routines (via Forbid/Permit), so
; we should not take too long in them.
;
;----------------------------------------------------------------------


	; Open sets the IO_ERROR field on an error.  If it was successfull,
	; we should set up the IO_UNIT field.

Open:		; ( device:a6, iob:a1, unitnum:d0, flags:d1 )
	movem.l d2/a2/a3/a4,-(sp)

	move.l	a1,a2		; save the iob

	;------ see if the unit number is in range
	moveq	#MD_NUMUNITS,d2
	cmp.l	d2,d0
	bcc.s	Open_Error	; unit number out of range

	;------ see if the unit is already initialized
	move.l	d0,d2		; save unit number
	lsl.l	#2,d0
	lea.l	md_Units(a6,d0.l),a4
	move.l	(a4),d0
	bne.s	Open_UnitOK

	;------ try and conjure up a unit
	bsr	InitUnit

	;------ see if it initialized OK
	move.l	(a4),d0
	beq.s	Open_Error

Open_UnitOK:
	move.l	d0,a3		; unit pointer in a3

	move.l	d0,IO_UNIT(a2)

	;------ mark us as having another opener
	addq.w	#1,LIB_OPENCNT(a6)
	addq.w	#1,UNIT_OPENCNT(a3)

	;------ prevent delayed expunges
	bclr	#LIBB_DELEXP,md_Flags(a6)

Open_End:

	movem.l (sp)+,d2/a2/a3/a4
	rts

Open_Error:
	move.b	#IOERR_OPENFAIL,IO_ERROR(a2)
	bra.s	Open_End

	; There are two different things that might be returned from
	; the Close routine.  If the device is no longer open and
	; there is a delayed expunge then Close should return the
	; segment list (as given to Init).  Otherwise close should
	; return NULL.

Close:		; ( device:a6, iob:a1 )
	movem.l a2/a3,-(sp)

	move.l	a1,a2

	move.l	IO_UNIT(a2),a3

	;------ make sure the iob is not used again
	moveq.l #-1,d0
	move.l	d0,IO_UNIT(a2)
	move.l	d0,IO_DEVICE(a2)

	;------ see if the unit is still in use
	subq.w	#1,UNIT_OPENCNT(a3)
	bne.s	Close_Device

	bsr	ExpungeUnit

Close_Device:
	;------ mark us as having one fewer openers
	subq.w	#1,LIB_OPENCNT(a6)

	;------ see if there is anyone left with us open
	bne.s	Close_End

	;------ see if we have a delayed expunge pending
	btst	#LIBB_DELEXP,md_Flags(a6)
	beq.s	Close_End

	;------ do the expunge
	bsr	Expunge

Close_End:
	movem.l (sp)+,a2/a3
	rts


	; There are two different things that might be returned from
	; the Expunge routine.	If the device is no longer open
	; then Expunge should return the segment list (as given to
	; Init).  Otherwise Expunge should set the delayed expunge
	; flag and return NULL.
	;
	; One other important note: because Expunge is called from
	; the memory allocator, it may NEVER Wait() or otherwise
	; take long time to complete.

Expunge:	; ( device: a6 )

	movem.l d2/a5/a6,-(sp)
	move.l	a6,a5
	move.l	md_SysLib(a5),a6
	
	;------ see if anyone has us open
	tst.w	LIB_OPENCNT(a5)
	beq	1$

	;------ it is still open.  set the delayed expunge flag
	bset	#LIBB_DELEXP,md_Flags(a5)
	CLEAR	d0
	bra.s	Expunge_End

1$:
	;------ go ahead and get rid of us.  Store our seglist in d2
	move.l	md_SegList(a5),d2

	;------ unlink from device list
	move.l	a5,a1
	CALLSYS Remove
	
	move.l	md_ExpansionLib(a5),a1
	CALLSYS CloseLibrary

	;------ free our memory
	CLEAR	d0
	move.l	a5,a1
	move.w	LIB_NEGSIZE(a5),d0

	sub.w	d0,a1
	add.w	LIB_POSSIZE(a5),d0

	CALLSYS FreeMem

	;------ set up our return value
	move.l	d2,d0

Expunge_End:
	movem.l (sp)+,d2/a5/a6
	rts


Null:
	CLEAR	d0
	rts


InitUnit:	; ( d2:unit number, a3:scratch, a6:devptr )
	movem.l d2/d3/d4,-(sp)

	;------ allocate unit memory
	move.l	#MyDevUnit_Sizeof,d0
	move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
	LINKSYS AllocMem,md_SysLib(a6)

	tst.l	d0
	beq	InitUnit_End

	move.l	d0,a3
	move.b	d2,mdu_UnitNum(a3)	; initialize unit number

	;------ mark us as ready to go
	move.l	d2,d0			; unit number
	lsl.l	#2,d0
	move.l	a3,md_Units(a6,d0.l)	; set unit table


InitUnit_End:
	movem.l (sp)+,d2/d3/d4
	rts

	;------ got an error.  free the unit structure that we allocated.
InitUnit_FreeUnit:
	bsr	FreeUnit
	bra.s	InitUnit_End

FreeUnit:	; ( a3:unitptr, a6:deviceptr )
	move.l	a3,a1
	move.l	#MyDevUnit_Sizeof,d0
	LINKSYS FreeMem,md_SysLib(a6)
	rts


ExpungeUnit:	; ( a3:unitptr, a6:deviceptr )
	move.l	d2,-(sp)

	;------ get rid of the unit's task.  We know this is safe
	;------ because the unit has an open count of zero, so it
	;------ is 'guaranteed' not in use.
	move.l	mdu_Process(a3),a1
	lea	-(pr_MsgPort)(a1),a1
	LINKSYS RemTask,md_SysLib(a6)

	;------ save the unit number
	CLEAR	d2
	move.b	mdu_UnitNum(a3),d2

	;------ free the unit structure.
	bsr	FreeUnit

	;------ clear out the unit vector in the device
	lsl.l	#2,d2
	clr.l	md_Units(a6,d2.l)

	move.l	(sp)+,d2

	rts

;----------------------------------------------------------------------
;
; here begins the device specific functions
;
;----------------------------------------------------------------------

; cmdtable is used to look up the address of a routine that will
; implement the device command.
cmdtable:
	DC.L	Invalid 	; $00000001
	DC.L	MyReset 	; $00000002
	DC.L	Read		; $00000004
	DC.L	Write		; $00000008
	DC.L	Update		; $00000010
	DC.L	Clear		; $00000020
	DC.L	MyStop		; $00000040
	DC.L	Start		; $00000080
	DC.L	Flush		; $00000100
	DC.L	Foo		; $00000200
	DC.L	Bar		; $00000400
cmdtable_end:

; this define is used to tell which commands should not be queued
; command zero is bit zero.
; The immediate commands are Invalid, Reset, Stop, Start, Flush
IMMEDIATES	EQU	$000001c3

;
; BeginIO starts all incoming io.  The IO is either queued up for the
; unit task or processed immediately.
;

BeginIO:	; ( iob: a1, device:a6 )
	move.l	a3,-(sp)

	;------ bookkeeping
	move.l	IO_UNIT(a1),a3

	;------ see if the io command is within range
	move.w	IO_COMMAND(a1),d0
	cmp.w	#MYDEV_END,d0
	bcc.s	BeginIO_NoCmd

	DISABLE a0

	;------ process all immediate commands no matter what
	move.w	#IMMEDIATES,d1
	btst	d0,d1
	bne.s	BeginIO_Immediate

	;------ see if the unit is STOPPED.  If so, queue the msg.
	btst	#MDUB_STOPPED,UNIT_FLAGS(a3)
	bne.s	BeginIO_QueueMsg

	;------ this is not an immediate command.  see if the device is
	;------ busy.
	bset	#UNITB_ACTIVE,UNIT_FLAGS(a3)
	beq.s	BeginIO_Immediate

	;------ we need to queue the device.  mark us as needing
	;------ task attention.  Clear the quick flag
BeginIO_QueueMsg:
	BSET	#UNITB_INTASK,UNIT_FLAGS(a3)
	bclr	#IOB_QUICK,IO_FLAGS(a1)

	ENABLE	a0

	move.l	a3,a0
	LINKSYS PutMsg,md_SysLib(a6)
	bra.s	BeginIO_End

BeginIO_Immediate:
	ENABLE	a0

	bsr	PerformIO

BeginIO_End:
	move.l	(sp)+,a3
	rts

BeginIO_NoCmd:
	move.b	#IOERR_NOCMD,IO_ERROR(a1)
	bra.s	BeginIO_End


;
; PerformIO actually dispatches an io request.	It expects a3 to already
; have the unit pointer in it.	a6 has the device pointer (as always).
; a1 has the io request.  Bounds checking has already been done on
; the io request.
;

PerformIO:	; ( iob:a1, unitptr:a3, devptr:a6 )
	move.l	a2,-(sp)
	move.l	a1,a2

	move.w	IO_COMMAND(a2),d0
	lea	cmdtable(pc),a0
	move.l	0(a0,d0.w),a0

	jsr	(a0)

	move.l	(sp)+,a2
	rts

;
; TermIO sends the IO request back to the user.  It knows not to mark
; the device as inactive if this was an immediate request or if the
; request was started from the server task.
;

TermIO: 	; ( iob:a1, unitptr:a3, devptr:a6 )
	move.w	IO_COMMAND(a1),d0
	move.w	#IMMEDIATES,d1
	btst	d0,d1
	bne.s	TermIO_Immediate

	;------ we may need to turn the active bit off.
	btst	#UNITB_INTASK,UNIT_FLAGS(a3)
	bne.s	TermIO_Immediate

	;------ the task does not have more work to do
	bclr	#UNITB_ACTIVE,UNIT_FLAGS(a3)

TermIO_Immediate:
	;------ if the quick bit is still set then we don't need to reply
	;------ msg -- just return to the user.
	btst	#IOB_QUICK,IO_FLAGS(a1)
	bne.s	TermIO_End

	LINKSYS ReplyMsg,md_SysLib(a6)

TermIO_End:
	rts
	

AbortIO:	; ( iob: a1, device:a6 )

;----------------------------------------------------------------------
;
; here begins the functions that implement the device commands
; all functions are called with:
;	a1 -- a pointer to the io request block
;	a2 -- another pointer to the iob
;	a3 -- a pointer to the unit
;	a6 -- a pointer to the device
;
; Commands that conflict with 68000 instructions have a "My" prepended
; to them.
;----------------------------------------------------------------------

Invalid:
	move.b	#IOERR_NOCMD,IO_ERROR(a1)
	bsr	TermIO
	rts

MyReset:

	; !!! fill me in !!!
	; !!! fill me in !!!
	; !!! fill me in !!!
	; !!! fill me in !!!


;
; the Read command acts as an infinite source of nulls.  It clears
; the user's buffer and marks that many bytes as having been read.
;

Read:
	move.l	IO_DATA(a1),a0
	move.l	IO_LENGTH(a1),d0
	move.l	d0,IO_ACTUAL(a1)

	;------ deal with a zero length read
	beq.s	Read_End

	;------ now copy the data
	CLEAR	d1

Read_Loop:
	move.b	d1,(a0)+
	subq.l	#1,d0
	bne.s	Read_Loop

Read_End:
	bsr	TermIO
	rts


;
; the Write command acts as bit bucket.  It clears acknowledges all
; the bytes the user has tried to write to it.
;

Write:
	move.l	IO_LENGTH(a1),IO_ACTUAL(a1)

	bsr	TermIO
	rts

;
; Update and Clear are internal buffering commands.  Update forces all
; io out to its final resting spot, and does not return until this is
; done.  Clear invalidates all internal buffers.  Since this device
; has no internal buffers, these commands do not apply.
;

Update:
Clear:
	bra	Invalid

;
; the Stop command stop all future io requests from being
; processed until a Start command is received.	The Stop
; command is NOT stackable: e.g. no matter how many stops
; have been issued, it only takes one Start to restart
; processing.
;

MyStop:
	bset	#MDUB_STOPPED,UNIT_FLAGS(a3)

	bsr	TermIO
	rts
	
Start:
	bsr	InternalStart

	move.l	a2,a1
	bsr	TermIO

	rts

InternalStart:
	;------ turn processing back on
	bclr	#MDUB_STOPPED,UNIT_FLAGS(a3)

	;------ kick the task to start it moving
	move.l	a3,a1
	CLEAR	d0
	move.l	MP_SIGBIT(a3),d1
	bset	d1,d0
	LINKSYS Signal,md_SysLib(a3)

	rts

;
; Flush pulls all io requests off the queue and sends them back.
; We must be careful not to destroy work in progress, and also
; that we do not let some io requests slip by.
;
; Some funny magic goes on with the STOPPED bit in here.  Stop is
; defined as not being reentrant.  We therefore save the old state
; of the bit and then restore it later.  This keeps us from
; needing to DISABLE in flush.	It also fails miserably if someone
; does a start in the middle of a flush.
;

Flush:
	movem.l d2/a6,-(sp)

	move.l	md_SysLib(a6),a6

	bset	#MDUB_STOPPED,UNIT_FLAGS(a3)
	sne	d2

Flush_Loop:
	move.l	a3,a0
	CALLSYS GetMsg

	tst.l	d0
	beq.s	Flush_End

	move.l	d0,a1
	move.b	#IOERR_ABORTED,IO_ERROR(a1)
	CALLSYS ReplyMsg

	bra.s	Flush_Loop

Flush_End:

	move.l	d2,d0
	movem.l (sp)+,d2/a6

	tst.b	d0
	beq.s	1$

	bsr	InternalStart
1$:

	move.l	a2,a1
	bsr	TermIO

	rts

;
; Foo and Bar are two device specific commands that are provided just
; to show you how to add your own commands.  The currently return that
; no work was done.
;

Foo:
Bar:
	CLEAR	d0
	move.l	d0,IO_ACTUAL(a1)

	bsr	TermIO
	rts


;----------------------------------------------------------------------
; EndCode is a marker that show the end of your code.
; Make sure it does not span sections nor is before the
; rom tag in memory!  It is ok to put it right after
; the rom tag -- that way you are always safe.	I put
; it here because it happens to be the "right" thing
; to do, and I know that it is safe in this case.
;----------------------------------------------------------------------
EndCode:

	END
