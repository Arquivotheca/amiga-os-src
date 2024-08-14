;********************************************************************
;
; cmds.asm
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;********************************************************************


	INCLUDE "assembly.i"

	NOLIST
	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/io.i"
	INCLUDE "exec/errors.i"

	INCLUDE "janus/i86block.i"
	INCLUDE "janus/services.i"
	LIST

	INCLUDE "jddata.i" 
	INCLUDE "asmsupp.i"

	XDEF	JDCDummy  
	XDEF	JDCInvalid
	XDEF	JDCRead
	XDEF	JDCWrite
	XDEF	JDCMotor
	XDEF	JDCSeek
	XDEF	JDCFormat
	XDEF	JDCChangeNum
	XDEF	JDCChangeState
	XDEF	JDCProtStatus

			    
	XREF	JDCmds
	XREF	EndCommand
		
	XLVO	SendJanusInt   
		    
;------ jdisk.device/Dummy -------------------------------------------
JDCDummy:
		moveq	#0,d0
ecRts:
		bsr	EndCommand
		rts


;------ jdisk.device/Invalid -----------------------------------------
JDCInvalid:
		moveq	#IOERR_NOCMD,d0
		bra.s	ecRts

;------ jdisk.device/Motor -------------------------------------------
;------ jdisk.device/ChangeNum ---------------------------------------
JDCMotor:
JDCChangeNum:
		moveq	#1,d0
		move.l	d0,IO_ACTUAL(a1)
		bra.s	JDCDummy

;------ jdisk.device/ChangeState -------------------------------------
;------ jdisk.device/ProtStatus --------------------------------------
JDCChangeState:
JDCProtStatus:
		moveq	#0,d0
		move.l	d0,IO_ACTUAL(a1)
		bra.s	ecRts


;------ jdisk.device/Read --------------------------------------------
JDCRead:
	INFOMSG 40,<'Read'>
		move.l	a6,-(a7)	    ; must be just a6 for ReRead

		;------ there are no bytes read yet
		clr.l	IO_ACTUAL(a1)

ReRead:
		moveq	#ADR_FNCTN_READ,d0
		bsr.s	CommonSetup

		;------ set the device task to complete this routine
		move.l	#JDIRead,jd_CmdTerm+IS_CODE(a6)

		bra.s	CommonSendInt

 
;------ jdisk.device/Write -------------------------------------------
;------ jdisk.device/Format ------------------------------------------
JDCWrite:
JDCFormat:
	INFOMSG 40,<'Write'>

		move.l	a6,-(a7)	    ; must be just a6 for ReWrite

		;------ there are no bytes written yet
		clr.l	IO_ACTUAL(a1)

ReWrite:
		moveq	#ADR_FNCTN_WRITE,d0
		bsr.s	CommonSetup

		;------ set the device task to complete this routine
		move.l	#JDIWrite,jd_CmdTerm+IS_CODE(a6)

		;------ copy data to the shared buffer
		move.l	IO_DATA(a1),a0
		move.l	jd_TrackBuffer(a6),a1
		lsr.w	#2,d0
		bra.s	2$
1$
		move.l	(a0)+,(a1)+
2$
		dbf	d0,1$

CommonSendInt:
		;------ initiate the read on the hard disk
		moveq	#JSERV_HARDDISK,d0
		move.l	jd_JanusBase(a6),a6
		CALLLVO SendJanusInt

		move.l	(a7)+,a6
		rts
		  

;------ CommonSetup --------------------------------------------------
;
;   for a command in D0
;   returns length in D0
;
CommonSetup:
	IFGE	INFOLEVEL-40
	move.l	IO_UNIT(a1),-(a7) 
	move.l	IO_OFFSET(a1),-(a7)
	move.l	IO_LENGTH(a1),-(a7)
	move.l	IO_DATA(a1),-(a7)
	INFOMSG 1,<'    buff: 0x%06lx, len: %ld, offset: %ld.  Unit #%ld.'>
	add.w	#16,a7
	ENDC
		bclr	#IOB_QUICK,IO_FLAGS(a1)     ; this isn't quick

		;------ set up the AmigaDskReq
		move.l	jd_AmigaDskReq(a6),a0
		move.w	d0,adr_Fnctn(a0)
		move.w	IO_UNIT+2(a1),adr_Part(a0)
		move.l	IO_OFFSET(a1),adr_Offset(a0)

		move.l	IO_LENGTH(a1),d0
		cmp.l	jd_TBSize(a6),d0
		ble.s	1$
		move.l	jd_TBSize(a6),d0
1$
		move.l	d0,adr_Count(a0)
		move.l	d0,jd_RequestedCount(a6)

		move.l	a1,jd_CmdTerm+IS_DATA(a6)   ; save the IOR
		rts


;------ jdisk.device/Seek --------------------------------------------
JDCSeek: 
	INFOMSG 40,<'Seek'>

		move.l	a6,-(a7)

		;------ there are no bytes involved
		clr.l	IO_LENGTH(a1)
		clr.l	IO_ACTUAL(a1)

		moveq	#ADR_FNCTN_SEEK,d0
		bsr	CommonSetup

		;------ set the device task to complete this routine
		move.l	#JDISeek,jd_CmdTerm+IS_CODE(a6)

		bra	CommonSendInt



;------ JDI routines -------------------------------------------------
;
;   called at interrupt time, using it's register environment.
;   a1 -- IO Request
;

;------ JDIRead ------
JDIRead:
		move.l	a6,-(a7)	    ; must be just a6 for ReRead

		bsr	CheckError
		tst.l	d0
		bne	TermCommand

		move.l	adr_Count(a0),d0
		lsr.w	#2,d0
		move.l	a1,-(a7)
		move.l	jd_TrackBuffer(a6),a0
		move.l	IO_DATA(a1),a1
		sub.l	jd_RequestedCount(a6),a1
		bra.s	2$
1$
		move.l	(a0)+,(a1)+
2$
		dbf	d0,1$
		move.l	(a7)+,a1

		move.l	IO_LENGTH(a1),d0
		bne	ReRead

		;------ d0 is zero 'cause length was zero
		bra.s	TermCommand


;------ JDISeek ------
JDISeek:
		move.l	a6,-(a7)
		bsr.s	CheckError

			  
;------ TermCommand ------
TermCommand:
		bsr	EndCommand

		;------ initiate any waiting command
		move.l	jd_CmdQueue+LH_HEAD(a6),a1
		tst.l	(a1)		    ; command queue is empty
		beq.s	jdiRts
		bset	#IOB_CURRENT,IO_FLAGS(a1)
		bne.s	jdiRts		    ; command already running
		move.w	IO_COMMAND(a1),d0
	IFGE	INFOLEVEL-40
	move.w	d0,-(a7)
	move.l	MN_REPLYPORT(a1),-(a7)
	move.l	LN_NAME(a1),-(a7)
	move.l	a1,-(a7)
	INFOMSG 1,<'IntNext IO 0x%lx, Name 0x%lx, ReplyPort 0x%lx, Command %d'>
	add.w	#14,a7
	ENDC

		lsl.w	#2,d0
		lea	JDCmds(pc),a0
		move.l	0(a0,d0.w),a0
		jsr	(a0)

jdiRts:
		move.l	(a7)+,a6
		rts


;------ CheckError ------
CheckError:
		move.l	IO_DEVICE(a1),a6
		move.l	jd_AmigaDskReq(a6),a0
		moveq	#0,d0
		move.w	adr_Err(a0),d0
		tst.b	d0
		bne.s	cmdError
		lsr.w	#8,d0
		bne.s	cmdBIOSErr
		move.l	adr_Count(a0),d0
		cmp.l	jd_RequestedCount(a6),d0
		bne.s	lenError
		add.l	d0,IO_ACTUAL(a1)
		sub.l	d0,IO_LENGTH(a1)
		add.l	d0,IO_DATA(a1)
		add.l	d0,IO_OFFSET(a1)

		moveq	#0,d0

cmdError:
		rts
				  
cmdBIOSErr:
		BSET	#IOB_BIOSERR,IO_FLAGS(a1)
		bra.s	cmdError
lenError:
		moveq	#JD_WRONGLENGTH,d0
		bra.s	cmdError



;------ JDIWrite ------
JDIWrite:
		move.l	a6,-(a7)	    ; must be just a6 for ReWrite

		bsr.s	CheckError
		tst.l	d0
		bne	TermCommand

		tst.w	IO_UNIT(a1)	    ; is this to be verified?  
		bne.s	verifyRead

		tst.l	IO_LENGTH(a1)
		beq	TermCommand
		bra	ReWrite

verifyRead:
		;------ back up those IO fields
		move.l	jd_RequestedCount(a6),d0
		sub.l	d0,IO_ACTUAL(a1)
		add.l	d0,IO_LENGTH(a1)
		sub.l	d0,IO_DATA(a1)
		sub.l	d0,IO_OFFSET(a1)

		moveq	#ADR_FNCTN_READ,d0
		bsr	CommonSetup

		;------ set the device task to complete this routine
		move.l	#JDIVerify,jd_CmdTerm+IS_CODE(a6)

		bra	CommonSendInt

			   
JDIVerify:
		move.l	a6,-(a7)	    ; must be just a6

		bsr	CheckError
		tst.l	d0
		bne	TermCommand

		move.l	adr_Count(a0),d0
		lsr.w	#2,d0
		move.l	a2,-(a7)
		move.l	jd_TrackBuffer(a6),a0
		move.l	IO_DATA(a1),a2
		sub.l	jd_RequestedCount(a6),a2
		moveq	#0,d1	    ; set zero
		bra.s	2$
1$
		cmp.l	(a0)+,(a2)+
2$
		dbne	d0,1$
		movea.l (a7)+,a2
		bne.s	failedVerify

		move.l	IO_LENGTH(a1),d0
		bne	ReWrite

		;------ d0 is zero 'cause length was zero
		bra	TermCommand

failedVerify:
		;------ back up those IO fields
		move.l	jd_RequestedCount(a6),d0
		sub.l	d0,IO_ACTUAL(a1)
		add.l	d0,IO_LENGTH(a1)
		sub.l	d0,IO_DATA(a1)
		sub.l	d0,IO_OFFSET(a1)
		bra	ReWrite

	END


