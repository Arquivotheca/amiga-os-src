		SECTION	filesystem,CODE

		NOLIST
		include	"exec/types.i"
		include	"exec/alerts.i"
		include	"libraries/dosextens.i"
		include	"intuition/intuition.i"
		include "internal/librarytags.i"
		include	"globals.i"
		include	"printf.mac"
		LIST

;		DEBUGENABLE

		XDEF	Request

		XREF	_LVOAlert,_LVOTaggedOpenLibrary,_LVOCloseLibrary,_LVOFreeMem
		XREF	GetPubMem,_LVOEasyRequestArgs,_AbsExecBase

STACKSIZE EQU 2048
NEGSTACK  EQU 4-2048

;==============================================================================
; result = Request()
;   d0
;
; builds a system requester using the string in ReqString and data stored in
; ReqArgs in the current window.  Waits for user response and returns TRUE
; if user clicked on retry or false if user clicked on cancel.
;==============================================================================
Request		movem.l	d2-d3/d6-d7/a2-a4,-(sp)
		moveq.l	#FALSE,d6		assume false return

		movea.l	ThisTask(a5),a0		test window pointer
		tst.l	pr_WindowPtr(a0)
		bmi.s	RequestDone		no window, return false

		move.l	#STACKSIZE,d0		get stack space
		bsr	GetPubMem
		tst.l	d0
		beq.s	RequestDone		no memory, return false
		exg.l	d0,sp			switch to new stack
		lea.l	STACKSIZE(sp),sp
		move.l	d0,-(sp)		and save old stack pointer

		moveq.l	#OLTAG_INTUITION,d0	intuition, any version
		jsr	_LVOTaggedOpenLibrary(a6)
		tst.l	d0
		beq.s	RequestFailed		didn't get intuition
		movea.l	d0,a6			using intuition lib now

; Switched over to EasyRequest for 2.0.  This means a different FS for 1.3
		move.l	sp,d7			mark current stack offset
		movea.l	ReqArgs(a5),a3		a3 points to args list
		move.l	#DISKINSERTED,-(sp)
		movea.l	sp,a2			a2 points to IDCMP flags needed
		move.l	ButtonString(a5),-(sp)	construct EasyStruct
		move.l	ReqString(a5),-(sp)	this string
		clr.l	-(sp)			no title
		clr.l	-(sp)			no flags
		move.l	#20,-(sp)		sizeof(struct EasyStruct)
		movea.l	sp,a1
		movea.l	ThisTask(a5),a0		get the window pointer
		movea.l	pr_WindowPtr(a0),a0
		jsr	_LVOEasyRequestArgs(a6)	display requester
		move.l	d0,d6			stash result
		movea.l	a6,a1			close down intuition
		movea.l	_AbsExecBase,a6
		jsr	_LVOCloseLibrary(a6)
		move.l	d7,sp			reclaim stack space used

RequestFailed	lea.l	NEGSTACK(sp),a1		point to beginning of stack
		move.l	(sp)+,sp		back to previous stack
		move.l	#STACKSIZE,d0
		jsr	_LVOFreeMem(a6)		free up the new stack

RequestDone	move.l	d6,d0			retrieve return code
		movem.l	(sp)+,d2-d3/d6-d7/a2-a4
		rts

		END
