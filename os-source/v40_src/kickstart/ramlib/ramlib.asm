**
**	$Id: ramlib.asm,v 36.29 93/03/05 10:01:38 mks Exp $
**
**      ramlib module
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	SECTION	ramlib,code

**	Includes

	INCLUDE	"ramlib.i"
	INCLUDE	"internal/librarytags.i"

	INCLUDE	"ramlib_rev.i"

	IFND	AG_CreateProc
AG_CreateProc	EQU	$000B0000	; CreateProc failed
	ENDC


**	Imports

	XLVO	Alert				; Exec
	XLVO	AllocMem			;
	XLVO	CloseDevice			;
	XLVO	CloseLibrary			;
	XLVO	OpenDevice			;
	XLVO	OpenLibrary			;
	XLVO	TaggedOpenLibrary		;
	XLVO	PutMsg				;
	XLVO	RemDevice			;
	XLVO	RemLibrary			;
	XLVO	SetFunction			;
	XLVO	WaitPort			;
	XLVO	AddMemHandler			;

	XLVO	CreateProc			; DOS
	XLVO	UnLoadSeg			;

	XREF	EndModule
	XREF	FindOnList
	XREF	LoadModule
	XREF	ProcSegment

	TASK_ABLES


**	Assumptions

	IFNE	rl_DosBase
	FAIL	"rl_DosBase not zero: recode references below"
	ENDC


**********************************************************************
*
*   The ramlib code will perform the following operations from the task
*   of the invoker of OpenModule [which *may* be ramlib.process]:
*
*   1.	Search in memory for the library.  If library is not found,
*	skip to 2.  If the version is sufficient, return success.  If
*	the library open count is non-zero the library is in use:
*	return failure.  Otherwise, try to remove this copy of the
*	library.  If the library is successfully removed, skip to 2.
*	Otherwise, the library is still in memory: return failure.
*
*   2.	Check if running on the ramlib.process.  If so, call LoadModule
*	directly.  Otherwise, create a message describing the module
*	and send it to the ramlib request message port and wait on the
*	reply.
*
*   3.	Return success if the library is then found in memory and had a
*	sufficient version, fail if not.
*
*   The ramlib.process will loop on the following:
*
*   4.	Wait at the request message port for a module request, invoke
*	LoadModule, and reply the message.
*
*   LoadModule will perform the following:
*
*   5.	Search in the rom resident tags for the module.  If module is
*	not found, skip to 6.  Otherwise, InitResident() the module and
*	return.
*
*   6.	Create the file name for the module -- if no ':' exists in the
*	name already, prepend DEVS: or LIBS: to the name.  LoadSeg the
*	module.  Look for a resident tag in the loaded image -- if not
*	found, initialize with a jump to the first executable
*	instruction and return.
*
*   7.	Check the version and the library name in the resident tag and
*	if sufficient and a name match, InitResident the image and
*	return.  UnLoadSeg a useless image and return.
*


**********************************************************************
*   Module resident tag and initialization code
*
residentTag:					; STRUCTURE RT,0
		dc.w	RTC_MATCHWORD		;   UWORD RT_MATCHWORD
		dc.l	residentTag		;   APTR  RT_MATCHTAG
		dc.l	EndModule		;   APTR  RT_ENDSKIP
		dc.b	RTF_AFTERDOS		;   UBYTE RT_FLAGS
		dc.b	VERSION			;   UBYTE RT_VERSION
		dc.b	0			;   UBYTE RT_TYPE
		dc.b	-100			;   BYTE  RT_PRI
		dc.l	RLName			;   APTR  RT_NAME
		dc.l	RLID			;   APTR  RT_IDSTRING
		dc.l	RLInit			;   APTR  RT_INIT
						;   LABEL RT_SIZE

RLName		dc.b	'ramlib',0
RLID		VSTRING
		ds.w	0

RLInit:
		movem.l	d2-d4/a2-a3/a5,-(a7)

		moveq	#RamLib_SIZEOF,d0
		move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
		CALLLVO	AllocMem
		move.l	d0,ex_RamLibPrivate(a6)

	;Never	bne.s	iMemOK
	;Never	ALERT	AN_RAMLib!AG_NoMemory

iMemOK:
		move.l	d0,a5
		moveq.l	#OLTAG_DOS,d0
		CALLLVO	TaggedOpenLibrary

		move.l	d0,(a5)			; rl_DosBase

	;Never	bne.s	iDosOK
	;Never	ALERT	AN_RAMLib!AG_OpenLib!AO_DOSLib

iDosOK:
		move.l	a6,-(a7)
		move.l	d0,a6
		;-- initialize rl_LoadPort.mp_MsgList
		lea	rl_LoadPort+MP_MSGLIST(a5),a0
		NEWLIST	a0

		;-- fire up ramlib process
		lea	RLName(pc),a0
		move.l	a0,d1
		moveq	#0,d2
		lea	ProcSegment(pc),a0
		move.l	a0,d3
		lsr.l	#2,d3
		move.l	#2048,d4
		CALLLVO	CreateProc
		move.l	(a7)+,a6
		tst.l	d0

	;Never	bne.s	iWedge
	;Never	ALERT	AN_RAMLib!AG_CreateProc

		;-- take over the exec entry points
iWedge:
		lea	WedgeTable(pc),a2
		lea	rl_OriginalExecJmp(a5),a3

iWedgeLoop:
		move.w	(a2)+,d0
		beq.s	iWedgeDone
		move.w	#$4EF9,(a3)+	; jmp (addr).l
		move.l	2(a6,d0.w),(a3)+
		move.w	(a2)+,d1
		lea	WedgeTable(pc,d1.w),a0
		exg	a0,d0
		move.l	a6,a1
		CALLLVO	SetFunction
		bra.s	iWedgeLoop

iWedgeDone:
*
*******************************************************************************
*
* Now, we need to register with the memory handler in EXEC...
*
* We add our handler at priority 0 in the memory handler lists...
*
		lea	rl_MemHandler(a5),a1	; Point at handler structure
*
		; LN_PRI is already 0...
		lea	RLName(pc),a0		; Get the name...
		move.l	a0,LN_NAME(a1)		; Set name...
		; IS_DATA is not needed by RAMLIB
		lea	FlushIt(pc),a0		; The code we will run...
		move.l	a0,IS_CODE(a1)		; IS_CODE
*
		CALLLVO	AddMemHandler		; Add us to the system...
*
*******************************************************************************
*
		move.l	a5,d0
		movem.l	(a7)+,d2-d4/a2-a3/a5
		rts


		;-- list the exec routines to be replaced
		;   by lvo offset and local PC offset, and
		;   define offsets in rl_ExecEntries
WEDGEOFFSET	SET	0

WEDGE	MACRO
		dc.w	_LVO\1
		dc.w	RL\1-WedgeTable
OEJ\1		EQU	WEDGEOFFSET
WEDGEOFFSET	SET	WEDGEOFFSET+6
	ENDM

WedgeTable:
		WEDGE	CloseDevice
		WEDGE	CloseLibrary
		WEDGE	RemDevice
		WEDGE	RemLibrary
		WEDGE	OpenDevice
		WEDGE	OpenLibrary
		dc.l	0			; End of list NULL...
*
*******************************************************************************
*
* The RAMLIB MemHandler IS_CODE entry point...
*
* This entry point is called by V39 EXEC when a memory allocation failed.
* At the point this happens EXEC has already put us into Forbid() and
* we promise not to break the Forbid()...
*
FlushIt:	; This is the handler code
		; We are passed a pointer to struct MemHandler
		; in a0, the value of is_Data in a1 and
		; a pointer to the interrupt structure in a2
		; and ExecBase in a6.
		; We must not break forbid!!!
*
		move.l	a2,-(sp)		; Need some play room...
		sub.l	a1,a1			; Clear a1...
		move.l	memh_Flags(a0),d0	; Get the flags...
		btst.l	#MEMHB_RECYCLE,d0	; Are we in RECYCLE mode?
		bne.s	Continue_Flush		; If so, continue flush...
*
* Ok, so this is the first time into the flush for this allocation.  We
* need to mark the devices and libraries as not being flush yet...
*
		bra.s	Flag_Loop		; Jump into the loop...
Flag_NotDone:	bclr.b	#LIBB_EXP0CNT,LIB_FLAGS(a1)	; Clear done flag...
Flag_Loop:	bsr.s	Next_LibDev		; Get next Library/Device
		bne.s	Flag_NotDone		; Loop until all done...
		; Note that a1 is now NULL again...
*
* Ok, so we need to check for the next library to flush...
* To find the next one, we will loop here until we find one that has
* LIB_OPENCNT == 0 and the EXP0CNT flag is 0.
*
Continue_Flush:	bsr.s	Next_LibDev		; Get next one...
		beq.s	FlushIt_Exit		; No more, we exit...
		; Note, d0 is NULL if we take this branch...
		tst.w	LIB_OPENCNT(a1)		; Check if open...
		bne.s	Continue_Flush		; If open, skip...
		bset.b	#LIBB_EXP0CNT,LIB_FLAGS(a1)	; Check/set flag...
		bne.s	Continue_Flush		; Continue looking...
*
* Ok, so we have one that seems to be ready for expunge and has not yet
* been expunged in this failed AllocMem()...  Thus, call RemDevice()
* or RemLibrary() as needed.  (Next_LibDev set that up for me...)
* Then, set the return result to try the allocation again and call
* me if it failed...
*
		jsr	(a2)			; Call RemXXXX()
		moveq.l	#MEM_TRY_AGAIN,d0	; Tell handler to try again...
*
* This is where we exit...  D0 is set up as needed and we just clean up
* and return to EXEC.  Whatever magic needs to happen will happen there...
*
FlushIt_Exit:	move.l	(sp)+,a2		; restore registers...
		rts
*
*******************************************************************************
*
* A little routine that will loop from the first node given until no
* more.  Given a NULL it will get the first node.  The nodes are the
* library and device list nodes...
*
* Input:	a1 - Current library/device node...
*		a6 - ExecBase...
* Output:	a2 - Function Pointer to RemLibrary() or RemDevice()
*		a1 - Next library/device node...
*		d0 - Same as a1 (used to set the flag)
*		Z flags - Set if no more (a1 is also NULL)
*
Next_LibDev:	move.l	a1,d0			; Check the input for NULL
		bne.s	Next_One		; If not NULL, we do next...
		lea	DeviceList(a6),a1	; Point at "0" Dev node...
		lea	_LVORemDevice(a6),a2	; RemDevice for device list
Next_One:	move.l	LN_SUCC(a1),a1		; Get next one...
		tst.l	LN_SUCC(a1)		; Is it end of list?
		bne.s	Next_Done		; We are done...
		lea	DeviceList+LH_TAIL(a6),a0	; Get tail...
		cmp.l	a1,a0			; Is it end of device list?
		bne.s	Next_End		; If not, no more...
		lea	LibList(a6),a1		; Point at "0" Lib node...
		lea	_LVORemLibrary(a6),a2	; RemLibrary for library list
		bra.s	Next_One		; Get the next node...
*
Next_End:	sub.l	a1,a1			; Clear return (NULL)
Next_Done	move.l	a1,d0			; Set the return flags...
		rts				; Exit with a1 and flags set
*
*******************************************************************************
*


**********************************************************************
*   Wedges that deal with module expunging
*

;------ RLRemDevice
RLRemDevice:
		moveq	#OEJRemDevice,d0
		bra.s	closeCommon

;------ RLRemLibrary
RLRemLibrary:
		moveq	#OEJRemLibrary,d0
		bra.s	closeCommon

;------ RLCloseDevice
RLCloseDevice:
		moveq	#OEJCloseDevice,d0
		bra.s	closeCommon

;------ RLCloseLibrary
RLCloseLibrary:
		moveq	#OEJCloseLibrary,d0

closeCommon:
		move.l	ex_RamLibPrivate(a6),a0
		jsr	rl_OriginalExecJmp(a0,d0.w)
		move.l	d0,d1
		beq.s	rlRts

		move.l	a6,-(a7)
		move.l	ex_RamLibPrivate(a6),a0
		move.l	(a0),a6			; rl_DosBase
		CALLLVO	UnLoadSeg
		move.l	(a7)+,a6

rlRts:
		rts


**********************************************************************
*
*   The ramlib code will perform the following operations from the task
*   of the invoker of OpenModule [which *may* be ramlib.process]:
*
*   1.	Try exec version of Open for module.  If module is not found,
*	skip to 2.  If the version is sufficient, return success.  If
*	the library open count is non-zero the library is in use:
*	return failure.  Otherwise, try to remove this copy of the
*	library.  If the library is successfully removed, skip to 2.
*	Otherwise, the library is still in memory: return failure.
*
*   2.	Check if running on the ramlib.process.  If so, call LoadModule
*	directly.  Otherwise, create a message describing the module
*	and send it to the ramlib request message port and wait on the
*	reply.
*
*   3.	Return success if the library is then found in memory and had a
*	sufficient version, fail if not.
*

;   a4	OpenParms
;   a5	RamLib
;   a6	SysBase

;------ tailName
tailName:
		move.l	ex_RamLibPrivate(a6),a5	; load local variables
		move.l	a1,a2			; save original name
		move.l	a1,a4			; guess tail
		move.l	a1,d2			; We need trash for a bit...
		subq.l	#8,d2			; Subtract 8...
		bcs.s	tnRts			; Less than 8...
tnLoop:
		move.b	(a1)+,d2
		beq.s	tnRts
		cmp.b	#'/',d2
		beq.s	tnNewTail
		cmp.b	#':',d2
		bne.s	tnLoop
tnNewTail:
		move.l	a1,a4
		bra.s	tnLoop
tnRts:
		rts


;------ RLOpenDevice
RLOpenDevice:
		movem.l	OR_SAVEREGS,-(a7)
		move.l	a0,a1
		bsr.s	tailName
		move.l	a4,a0
		move.l	or_A1(a7),a1
		jsr	rl_OriginalExecJmp+OEJOpenDevice(a5)
		move.l	d0,d2
		beq.s	omExecDone

		;--	exec open failed try loading from process
		lea	-OpenParms_SIZEOF(a7),a7
		move.l	a4,op_OpenName(a7)
		move.l	a7,a4
		clr.b	op_OpenLibFlag(a4)
		move.l	#'DEVS',op_MergedName(a4)
		lea	DeviceList(a6),a0
		move.l	a0,op_ExecList(a4)
		bra.s	omAttemptLoad


;------ RLOpenLibrary
RLOpenLibrary:
		movem.l	OR_SAVEREGS,-(a7)
		bsr.s	tailName
		move.l	a4,a1
		jsr	rl_OriginalExecJmp+OEJOpenLibrary(a5)
		move.l	d0,d2
		bne.s	omExecDone

		;--	exec open failed try loading from process
		lea	-OpenParms_SIZEOF(a7),a7
		move.l	a4,op_OpenName(a7)
		move.l	a7,a4
		st.b	op_OpenLibFlag(a4)
		move.l	#'LIBS',op_MergedName(a4)
		lea	LibList(a6),a0
		move.l	a0,op_ExecList(a4)
		bsr	FindOnList
		beq.s	omAttemptLoad

		;--	check if failure due to wrong version
		move.l	d0,a1
		move.w	OpenParms_SIZEOF+or_D0+2(a4),d0
		cmp.w	LIB_VERSION(a1),d0
		ble.s	omFail

		;--	    try to expunge old library
		FORBID

		tst.w	LIB_OPENCNT(a1)
		bne.s	olRemLibrary


omPermitFail:
		PERMIT

omFail:
		move.l	d2,d0

omDone:
		lea	OpenParms_SIZEOF(a7),a7
omExecDone:
		addq.l	#8,a7
		movem.l	(a7)+,OR_RESTOREREGS
		rts


olRemLibrary:
		CALLLVO	RemLibrary

		PERMIT
		;--	fall thru to omAttemptLoad


omAttemptLoad:
		;--	create load name
		move.l	a2,op_ReqName(a4)
		lea	op_MergedName(a4),a0
		move.l	a2,a1
		move.l	a0,op_LoadName(a4)	; guess won't find ':'
		addq.l	#4,a0			; bump past DEVS or LIBS
		moveq	#':',d2
		move.b	d2,(a0)+
		moveq	#00,d0			; limit length of name
lnLoop:
		addq.b	#1,d0
		beq.s	omFinalOpen		; if wraps, fail LoadModule

		move.b	(a1)+,d1
		move.b	d1,(a0)+
		beq.s	lnEnd
		cmp.b	d2,d1
		bne.s	lnLoop
		move.l	a2,op_LoadName(a4)

lnEnd:
		;-- check if this is already ramlib.process
		move.l	ThisTask(a6),d0
		cmp.l	rl_LoadPort+MP_SIGTASK(a5),d0
		bne.s	omUseRamlibProc

		bsr	LoadModule

omFinalOpen:
		move.l	op_OpenName(a4),a0
		tst.b	op_OpenLibFlag(a4)
		bne.s	omFinalOpenLib

		;--	final OpenDevice
		move.l	OpenParms_SIZEOF+or_D0(a4),d0
		move.l	OpenParms_SIZEOF+or_D1(a4),d1
		move.l	OpenParms_SIZEOF+or_A1(a4),a1
		jsr	rl_OriginalExecJmp+OEJOpenDevice(a5)
		bra	omDone

omFinalOpenLib:
		move.l	OpenParms_SIZEOF+or_D0(a4),d0
		move.l	a0,a1		; op_OpenName
		jsr	rl_OriginalExecJmp+OEJOpenLibrary(a5)
		bra	omDone

omUseRamlibProc:
		move.l	d0,a0
		cmp.b	#NT_PROCESS,LN_TYPE(a0)
		beq.s	omFromProcess

		;--	task has no CurrentDir or HomeDir

		clr.l	op_CurrentDir(a4)
		clr.l	op_HomeDir(a4)

		;-- called from task, but up until 2/7/91 we
		;-- has been using ramlib's process windowptr.
		;-- therefore because I DONT want to change existing
		;-- 2.0 behavior, use 0 here instead of -1.

		clr.l	op_WindowPtr(a4)

		bra.s	omQueueMsg

omFromProcess:
		move.l	pr_CurrentDir(a0),op_CurrentDir(a4)
		move.l	pr_HomeDir(a0),op_HomeDir(a4)
		move.l	pr_WindowPtr(a0),op_WindowPtr(a4)

		;-- set up message and reply port on stack
omQueueMsg:
		lea	op_LMPort(a4),a1
		move.l	a1,op_SR_LMMsg+MN_REPLYPORT(a4)
		;--	clr MP_FLAGS & set MP_SIGBIT
		move.w	#SIGB_SINGLE,MP_FLAGS(a1)
		move.l	d0,MP_SIGTASK(a1)
		lea	MP_MSGLIST(a1),a1
		NEWLIST	a1

		lea	rl_LoadPort(a5),a0
		lea	op_SR_LMMsg(a4),a1
		CALLLVO	PutMsg
		lea	op_LMPort(a4),a0
		CALLLVO	WaitPort
		;-- (no need to fix up message port on stack)
		bra	omFinalOpen

	END
