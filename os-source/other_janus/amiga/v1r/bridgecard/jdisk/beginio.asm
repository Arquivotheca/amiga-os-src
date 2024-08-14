;********************************************************************
;
; beginio.asm
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;********************************************************************


	INCLUDE "assembly.i"

	NOLIST
	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/devices.i"
	INCLUDE "exec/ports.i"
	INCLUDE "exec/io.i"
	INCLUDE "exec/errors.i"
	INCLUDE "exec/ables.i"
	LIST

	INCLUDE "jddata.i" 
	INCLUDE "asmsupp.i"

	XDEF	JDBeginIO
	XDEF	JDAbortIO
	XDEF	JDCmds
	XDEF	EndCommand
		
	XLVO	ReplyMsg    

	XREF	JDCDummy 
	XREF	JDCInvalid
	XREF	JDCRead
	XREF	JDCWrite
	XREF	JDCMotor
	XREF	JDCSeek
	XREF	JDCFormat
	XREF	JDCChangeNum
	XREF	JDCChangeState
	XREF	JDCProtStatus
  

	INT_ABLES


;------ jdisk.device/BeginIO -----------------------------------------
JDBeginIO:
		movem.l d2/d3/a2/a3/a6,-(a7)   
		move.l	a6,a2  
		move.l	a1,a3			   
		and.b	#(~(IOF_CURRENT!IOF_DONE))&$0ff,IO_FLAGS(a3)
		clr.b	IO_ERROR(a3)
		clr.b	LN_TYPE(a3)	    ; it's NOT NT_REPLYMSG
		
		;------ is the command in bounds?
		move.w	IO_COMMAND(a3),d2
		cmp.w	#NUM_COMMANDS,d2 
		blt.s	cmdExists
		moveq	#0,d2

cmdExists:
		move.l	#IMMEDIATE_COMMANDS,d0
		btst	d2,d0
		bne	performCmd
					  
		;------ queue the command
		moveq	#0,d3		    ; guess command won't be dispatched
		DISABLE a0
		tst.b	IO_ERROR(a3)
		bne.s	queueEnable	    ; don't queue if already an error
		
		lea	jd_CmdQueue(a2),a0
		;	a1 still contains the IO Request
		ADDTAIL
		
		;------ check if this request is first in the queue
		move.l	jd_CmdQueue+LH_HEAD(a2),a0
		cmpa.l	a0,a3
		bne.s	queueEnable 

		;------ mark this as current
		bset	#IOB_CURRENT,IO_FLAGS(a3)
		seq	d3		    ; set if to be dispatched here

	PUTMSG	80,<'%s/command marked current'>
queueEnable:
		ENABLE	a0
		
		;------ is this at the top of the queue?
		tst.b	d3
		bne.s	performCmd

		;------ this command will be executed later
		bclr	#IOB_QUICK,IO_FLAGS(a3)
		bra.s	endBIO

performCmd:
	IFGE	INFOLEVEL-40
	move.w	d2,-(a7)
	move.l	MN_REPLYPORT(a3),-(a7)
	move.l	LN_NAME(a3),-(a7)
	move.l	a3,-(a7)
	INFOMSG 1,<'Perform IO 0x%lx, Name 0x%lx, ReplyPort 0x%lx, Command %d'>
	add.w	#14,a7
	ENDC
	IFGE	INFOLEVEL-80
	move.l	MN_REPLYPORT(a3),a0
	move.l	LN_NAME(a0),-(a7)
	INFOMSG 1,<'----  ReplyPort Name 0x%06lx'>
	addq.l	#4,a7
	ENDC
		lsl.w	#2,d2
		move.l	JDCmds(pc,d2.w),a0
		move.l	a3,a1
		move.l	a2,a6 
		jsr	(a0)

endBIO:
	IFGE	INFOLEVEL-90
	pea	0
	move.b	IO_FLAGS(a3),3(a7)
	PUTMSG	1,<'%s/BeginIO returns: IO_FLAGS 0x%02lx'>
	addq.l	#4,a7
	ENDC
		movem.l (a7)+,d2/d3/a2/a3/a6 
		rts


JDCmds:
		dc.l	JDCInvalid
		dc.l	JDCDummy	; Reset
		dc.l	JDCRead
		dc.l	JDCWrite
		dc.l	JDCDummy	; Update
		dc.l	JDCDummy	; Clear
		dc.l	JDCDummy	; Stop
		dc.l	JDCDummy	; Start
		dc.l	JDCDummy	; Flush

		dc.l	JDCMotor
		dc.l	JDCSeek
		dc.l	JDCFormat
		dc.l	JDCDummy	; Remove
		dc.l	JDCChangeNum
		dc.l	JDCChangeState
		dc.l	JDCProtStatus

NUM_COMMANDS	EQU	(*-JDCmds)/4
IMMEDIATE_COMMANDS  EQU $0f3f3


;------ jdisk.device/AbortIO -----------------------------------------
JDAbortIO:
		DISABLE a0
		btst	#IOB_CURRENT,IO_FLAGS(a1)
		beq.s	abortable

		;------ already running: can't make command complete faster
		ENABLE	a0
		rts

abortable:
		moveq	#IOERR_ABORTED,d0
		bra.s	abortEndCommand


;------ EndCommand ---------------------------------------------------
EndCommand:
		DISABLE a0

abortEndCommand:
		movem.l a2/a3/a6,-(a7)
		move.l	a1,a3
		move.l	a6,a2
		
		bset	#IOB_DONE,IO_FLAGS(a3)
		bne	endEnable
	IFGE	INFOLEVEL-80
	move.w	d0,-(a7)
	move.l	MN_REPLYPORT(a3),-(a7)
	move.l	LN_NAME(a3),-(a7)
	move.l	a3,-(a7)
	INFOMSG 1,<'End     IO 0x%lx, Name 0x%lx, ReplyPort 0x%lx, Error %d'>
	add.w	#14,a7
	ENDC

	IFGE	INFOLEVEL-80
	move.l	MN_REPLYPORT(a3),a0
	move.l	LN_NAME(a0),-(a7)
	INFOMSG 1,<'----  ReplyPort Name 0x%06lx'>
	addq.l	#4,a7
	ENDC

	IFGE	INFOLEVEL-2
	tst.b	d0
	beq.s	eok
	pea	0
	move.b	d0,3(a7)
	move.w	IO_COMMAND(a3),(a7)
	PUTMSG	1,<'%s/Command %d, Error 0x%02x ******'> 
	addq.l	#4,a7
eok:
	ENDC
		move.b	d0,IO_ERROR(a3)
		
		;------ is this command from the queue
		btst	#IOB_CURRENT,IO_FLAGS(a3)
		beq.s	endNotQueued

		move.l	a3,a1
		REMOVE

		;------ is this command still quick?
endNotQueued:
		btst	#IOB_QUICK,IO_FLAGS(a3)
		bne.s	endEnable
		
		;------ reply the command
		move.l	a3,a1
		move.l	jd_ExecBase(a2),a6
		CALLLVO ReplyMsg

endEnable:
		ENABLE	a0

		movem.l (a7)+,a2/a3/a6 
		rts

	END


