**
**	$Id: dev.asm,v 36.23 93/03/05 17:00:48 darren Exp $
**
**      console.device Init and Expunge
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/initializers.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE	"devices/input.i"
	INCLUDE "internal/librarytags.i"
	INCLUDE	"debug.i"

        LIST

**	Exports

	XDEF	CDFuncInit
	XDEF	CDStructInit
	XDEF	CDInit
	XDEF	CDExpunge


**	Imports

	XLVO	AddTask			; Exec
	XLVO	AllocMem		;
	XLVO	CloseDevice		;
	XLVO	CloseLibrary		;
	XLVO	FreeMem			;
	XLVO	InitSemaphore		;
	XLVO	OpenDevice		;
	XLVO	OpenLibrary		;
	XLVO	TaggedOpenLibrary	; V39
	XLVO	RemTask			;
	XLVO	DoIO			;

	XLVO	InitRastPort		; Graphics

	XLVO	CDInputHandler		; Console

	XREF	CDName
	XREF	REVISION

	XREF	CDOpen
	XREF	CDClose
	XREF	BeginIO
	XREF	AbortIO
	XREF	CDInputHandler
	XREF	RawKeyConvert
	XREF	GetConSnip
	XREF	SetConSnip
	XREF	AddConSnipHook
	XREF	RemConSnipHook

	XREF	Invalid
	XREF	ResetCmd
	XREF	CDRead
	XREF	CDWrite
	XREF	Update
	XREF	CDClear
	XREF	StopCmd
	XREF	Start
	XREF	Flush

	XREF	CDAskKeyMap
	XREF	CDSetKeyMap
	XREF	CDAskDefaultKeyMap
	XREF	CDSetDefaultKeyMap

	XREF	CBInvalid
	XREF	CBResetCmd
	XREF	CDCBRead
	XREF	CDCBWrite
	XREF	CBUpdate
	XREF	CDCBClear
	XREF	CBStopCmd
	XREF	CBStart
	XREF	CBFlush

	XREF	CDCBAskKeyMap
	XREF	CDCBSetKeyMap
	XREF	CDCBAskDefaultKeyMap
	XREF	CDCBSetDefaultKeyMap

	XREF	CDTaskStart


**********************************************************************
CDInit:
		movem.l	a2-a3/a5-a6,-(sp)
	    ;------ This is called from within MakeDevice, after
	    ;------ all the memory has been allocated

		move.l	d0,a5
		move.l	a6,cd_ExecLib(a5)

*
*		DINIT			; initialize debugging
*
		;-- get the keymap library
		moveq	#OLTAG_KEYMAP,d0
		CALLLVO TaggedOpenLibrary
		move.l	d0,cd_KeymapLib(a5)
		beq	initKLErr

		;-- get the graphics library
		moveq	#OLTAG_GRAPHICS,d0
		CALLLVO TaggedOpenLibrary
		move.l	d0,cd_GraphicsLib(a5)
		beq	initGLErr

		;-- get the layers library
		moveq	#OLTAG_LAYERS,d0
		CALLLVO TaggedOpenLibrary
		move.l	d0,cd_LayersLib(a5)
		beq	initLLErr

		;-- get the input device
		move.w	#ODTAG_INPUT,a0
	;;	lea	IDName(pc),a0
		moveq	#0,d0
		lea	cd_HandlerIOR(a5),a1
		moveq	#0,d1
		CALLLVO	OpenDevice
		tst.l	d0
		bne	initIDErr

		;-- initialize input handler
		lea	_LVOCDInputHandler(a5),a1
		move.l	a1,cd_HandlerIS+IS_CODE(a5)
		move.l	a5,cd_HandlerIS+IS_DATA(a5)

		;-- initialize the Unit semaphore
		lea	cd_USemaphore(a5),a0
		CALLLVO	InitSemaphore

		;-- initialize the Selection semaphore
		lea	cd_SelectionSemaphore(a5),a0
		CALLLVO	InitSemaphore

		;-- initialize the Event handling semaphore
		lea	cd_EVSemaphore(a5),a0
		CALLLVO	InitSemaphore

		lea	cd_SelectionHooks(a5),a1
		NEWLIST	a1

		;-- initialize the RastPort semaphore
		lea	cd_RPSemaphore(a5),a0
		CALLLVO	InitSemaphore

		;-- initialize the shared RastPort
		lea	cd_RastPort(a5),a1
		move.l	a6,-(a7)
		move.l	cd_GraphicsLib(a5),a6
		CALLLVO	InitRastPort
		move.l	(a7)+,a6

		;-- initialize the unit list
		lea	cd_UHead(a5),a0
		NEWLIST	a0

		;-- initialize console device task
		lea	cd_TC(a5),a1
		lea	cd_Stk(a5),a0
		move.l	a0,TC_SPLOWER(a1)
		move.w	#(CD_STKSIZE/2)-1,d0
		move.w	#$09999,d1
pattern0Loop:
		move.w	d1,(a0)+
		dbf	d0,pattern0Loop
	;---	lea	cd_Stk+CD_STKSIZE(a5),a0
		move.l	a0,TC_SPUPPER(a1)
		move.l	a5,-(a0)		; argument (console device)
		move.l	a0,TC_SPREG(a1)

		lea	CDTaskStart,a2
		moveq	#0,d0
		move.l	d0,a3
		CALLLVO AddTask

		;--	add input handler
		sub.w	#MP_SIZE,a7
		move.l	a7,a0
		bsr.s	initSyncIO
		lea	cd_HandlerIS(a5),a0
		move.l	a0,IO_DATA(a1)
		move.w	#IND_ADDHANDLER,IO_COMMAND(a1)
		CALLLVO	DoIO			; could break Forbid()!
		add.w	#MP_SIZE,a7

cdInitDone:
		move.l	a5,d0	    ; return the library

cdInitRts:
		movem.l	(sp)+,a2-a3/a5-a6
		rts

initMemErr:
		lea	cd_HandlerIOR(a5),a1
		CALLLVO	CloseDevice

initIDErr:
initILErr:
		move.l	cd_LayersLib(a5),a1
		CALLLVO	CloseLibrary

initLLErr:
		move.l	cd_GraphicsLib(a5),a1
		CALLLVO	CloseLibrary

initGLErr:
		move.l	cd_KeymapLib(a5),a1
		CALLLVO	CloseLibrary

initKLErr:
		moveq	#0,d0
		bra	cdInitRts


*------ initSyncIO ---------------------------------------------------
*
*   NAME
*	initSyncIO - initialize cd_HandlerIOR for synchronous use
*
*   SYNOPSYS
*	initSyncIO(msgPort)
*	           a0
*
*---------------------------------------------------------------------
initSyncIO:
		move.b	#NT_MSGPORT,LN_TYPE(a0)
		clr.b	MP_FLAGS(a0)
		move.b	#SIGF_SINGLE,MP_SIGBIT(a0)
		move.l	cd_ExecLib(a5),a1
		move.l	ThisTask(a1),MP_SIGTASK(a0)

		lea	cd_HandlerIOR(a5),a1
		move.l	a0,MN_REPLYPORT(a1)

		lea	MP_MSGLIST(a0),a0
		;	NEWLIST a0
		move.l	a0,LH_TAILPRED(a0)
		addq.l	#4,a0
		clr.l	(a0)
		move.l	a0,-(a0)
		rts

CDStructInit:
	;------ Initialize the device node
	    INITWORD    LIB_REVISION,REVISION

	    INITLONG	cd_CmdVectors,cdCmdTable
	    INITLONG	cd_CmdBytes,cdCmdBytes
	    INITWORD	cd_NumCommands,CD_NUMCOMMANDS

	    INITBYTE	cd_HandlerIS+LN_TYPE,NT_INTERRUPT
	    INITBYTE	cd_HandlerIS+LN_PRI,CD_HANDLERPRI
	    INITLONG	cd_HandlerIS+LN_NAME,CDName

	    INITBYTE	cd_TC+LN_TYPE,NT_TASK
	    INITBYTE	cd_TC+LN_PRI,CD_TASKPRI
	    INITLONG	cd_TC+LN_NAME,CDName

		dc.w	0


CDFuncInit:
		dc.w	-1

		dc.w	CDOpen+(*-CDFuncInit)
		dc.w	CDClose+(*-CDFuncInit)
		dc.w	CDExpunge-CDFuncInit
		dc.w	ExtFunc-CDFuncInit

		dc.w	BeginIO+(*-CDFuncInit)
		dc.w	AbortIO+(*-CDFuncInit)
		dc.w	CDInputHandler+(*-CDFuncInit)
		dc.w	RawKeyConvert+(*-CDFuncInit)

		dc.w	GetConSnip+(*-CDFuncInit)
		dc.w	SetConSnip+(*-CDFuncInit)
		dc.w	AddConSnipHook+(*-CDFuncInit)
		dc.w	RemConSnipHook+(*-CDFuncInit)

		dc.w	-1

cdCmdTable:
		dc.l	Invalid
		dc.l	ResetCmd
		dc.l	CDRead
		dc.l	CDWrite
		dc.l	Update
		dc.l	CDClear
		dc.l	StopCmd
		dc.l	Start
		dc.l	Flush

		dc.l	CDAskKeyMap
		dc.l	CDSetKeyMap
		dc.l	CDAskDefaultKeyMap
		dc.l	CDSetDefaultKeyMap

cdCmdBytes:
		dc.b	CBInvalid
		dc.b	CBResetCmd
		dc.b	CDCBRead
		dc.b	CDCBWrite
		dc.b	CBUpdate
		dc.b	CDCBClear
		dc.b	CBStopCmd
		dc.b	CBStart
		dc.b	CBFlush

		dc.b	CDCBAskKeyMap
		dc.b	CDCBSetKeyMap
		dc.b	CDCBAskDefaultKeyMap
		dc.b	CDCBSetDefaultKeyMap

CD_NUMCOMMANDS	EQU	(*-cdCmdBytes)


		ds.w	0


*------ console.device/Expunge ---------------------------------------
*
*   NAME
*	Expunge - indicate a desire to remove the Console device
*
*   SYNOPSIS
*	Result = Expunge()
*
*   FUNCTION
*	The Expunge routine is called when a user issues a RemDevice
*	call.  The existance of any other users of the device, as
*	determined by the library open count being non-zero, will
*	cause the Expunge to be deferred.  When the device is not
*	in use, or no longer in use, the Expunge is actually
*	performed, and the device removed from the system list.
*---------------------------------------------------------------------

*---------------------------------------------------------------------
*
*       As of V37 we decided to start the inputhandler at init
*       time for compatability with 1.x.  In particular INFESTATION
*       by Psygnosis broke because it assumed that the POTGO register
*	was set-up.  By adding the inputhandler at init time, input.task
*       gets gameport.device going which sets up POTGO as a mouse.
*
*       Because its not cool to Wait() during an expunge, and because
*       ROM space is tight, the expunge code was removed from here,
*       and the allocation/deallocation of the TmpRas was moved to
*       Open/Close time.
*---------------------------------------------------------------------
CDExpunge:

;-- Don't need - we are not delayed expunge cause we can't expunge.
;--		bset	#LIBB_DELEXP,LIB_FLAGS(a6)

	;-- also a good place for a bail out for extended function
ExtFunc:

		moveq	#0,d0		    ;still in use
		rts

	

	END
