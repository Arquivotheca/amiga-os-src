
;****** Included Files ***********************************************

    INCLUDE	'exec/types.i'
    INCLUDE	'exec/nodes.i'
    INCLUDE	'exec/lists.i'
    INCLUDE	'exec/io.i'
    INCLUDE	'exec/alerts.i'



JMPLIB	MACRO
	XREF	_LVO\1
	JMP	_LVO\1(A6)
	ENDM

;*********************************************************************

	XDEF	_MySendIO

_MySendIO:
	;---- if quick, request is OK
	BTST.B	#IOB_QUICK,IO_FLAGS(A1)
	BNE.S	si_OkToSend

	;---- else request must not have type NT_MESSAGE
	MOVEQ	#NT_MESSAGE,D0

	;---- unless it's a request which has never been used
	TST.L	LN_PRED(A1)
	BEQ.S	si_NewIOR

	CMP.B	LN_TYPE(A1),D0		; compare LN_TYPE to NT_MESSAGE
	BEQ.S	SendIO_Busy
si_NewIOR
	MOVE.B	D0,LN_TYPE(A1)	;Prepare for next check

	IFNE	0
	;---- all requests must have a ReplyPort
	MOVE.L	MN_REPLYPORT(A1),D0
	BEQ.S	SendIO_BadReplyPort
	MOVE.L	D0,A0
	CMP.B	#NT_MSGPORT,LN_TYPE(A0)
	BNE.S	SendIO_BadReplyPort

	;---- request must not be linked into a port
	MOVE.L	LN_SUCC(A1),A0
	MOVE.L	A0,D0
	LSR.L	#1,D0
	BLS.S	si_OK		;BLS is (Z or C)
	CMP.L	LN_PRED(A0),A1
	BNE.S	si_OK
	MOVE.L	LN_PRED(A1),A0
	MOVE.L	A0,D0
	LSR.L	#1,D0
	BLS.S	si_OK		;BLS is (Z or C)
	CMP.L	LN_SUCC(A0),A1
	BEQ.S	SendIO_LinkedIn
si_OK:
	ENDC

si_OkToSend:
	CLR.B	IO_FLAGS(A1)
	BEGINIO
	MOVEM.L	_TrashRegisters,D0/D1/A0/A1
	RTS




SendIO_Busy:
SendIO_BadReplyPort:
SendIO_LinkedIn:
	LEA.L	SendIO_Name(pc),a0
	BSR.S	PrintMessage
	BRA.S	si_OkToSend
SendIO_Name
	dc.b	'SendIO',0
	CNOP	0,2


;*********************************************************************

	XDEF	_MyDoIO

_MyDoIO:
	;---- if quick, request is OK
	BTST.B	#IOB_QUICK,IO_FLAGS(A1)
	BNE.S	di_OkToSend

	;---- else request must not have type NT_MESSAGE
	MOVEQ	#NT_MESSAGE,D0

	;---- unless it's a request which has never been used
	TST.L	LN_PRED(A1)
	BEQ.S	di_NewIOR

	CMP.B	LN_TYPE(A1),D0		; compare LN_TYPE to NT_MESSAGE
	BEQ.S	DoIO_Busy

di_NewIOR
	MOVE.B	D0,LN_TYPE(A1)	;Prepare for next check

	IFNE	0
	;---- all requests must have a ReplyPort
	MOVE.L	MN_REPLYPORT(A1),D0
	BEQ.S	DoIO_BadReplyPort
	MOVE.L	D0,A0
	CMP.B	#NT_MSGPORT,LN_TYPE(A0)
	BNE.S	DoIO_BadReplyPort

	;---- request must not be linked into a port
	MOVE.L	LN_SUCC(A1),A0
	MOVE.L	A0,D0
	LSR.L	#1,D0
	BLS.S	di_OK		;BLS is (Z or C)
	CMP.L	LN_PRED(A0),A1
	BNE.S	di_OK
	MOVE.L	LN_PRED(A1),A0
	MOVE.L	A0,D0
	LSR.L	#1,D0
	BLS.S	di_OK		;BLS is (Z or C)
	CMP.L	LN_SUCC(A0),A1
	BEQ.S	DoIO_LinkedIn
di_OK:
	ENDC

di_OkToSend:
	MOVE.L	A1,-(SP)
	MOVE.B	#IOF_QUICK,IO_FLAGS(A1)
	BEGINIO
	MOVE.L	(SP)+,A1
	JMPLIB	WaitIO


DoIO_Busy:
DoIO_BadReplyPort:
DoIO_LinkedIn:
;	move.w	#$5555,$2fe
	move.l	LN_NAME(A1),D0
	beq.s	DoIO_NotTrack
	move.l	d0,a0
	cmp.l	#'trac',(a0)
	beq.s	di_OkToSend
DoIO_NotTrack:
	LEA.L	DoIO_Name(pc),a0
	BCHG.B	#1,$BFE001
	BSR.S	PrintMessage
	BRA.S	di_OkToSend
DoIO_Name:
	dc.b	'DoIO',0
	CNOP	0,2



;*********************************************************************
	;[A0-Name of function that was called]
	;[A1-IORequest]
	;[SP-acurate stack pointer]
	XREF	_DisplayError

	;Preserve A1
PrintMessage:
	BCHG.B	#1,$BFE001
	MOVE.L	SP,-(SP)
	ADDQ.L	#4,(SP)		;Adjust for BSR
	MOVE.L	A0,-(SP)
	MOVE.L	A1,-(SP)
	;void DisplayError(Request,FunctionName,Stack)
	JSR	_DisplayError
	MOVE.L	(SP)+,A1
	ADDQ.L	#8,SP
	RTS				;preserve A1


;*********************************************************************

	XDEF	_MyOpenDevice
	XREF	_OldOpenDevice

_MyOpenDevice:
	movem.l	a1/a2,-(sp)
	move.l	_OldOpenDevice,a2
	jsr	(a2)
	movem.l	(sp)+,a1/a2
	moveq.l	#NT_REPLYMSG,D1		;mark as done (good)
	tst.l	d0			;zero means open was OK
	beq.s	od_OK
	clr.l	IO_DEVICE(a1)		;kill device pointer (bad)
	move.b	#NT_MESSAGE,d1		;mark as pending (bad)
od_OK:
	move.b	d1,LN_TYPE(a1)
	rts


;*********************************************************************

	XDEF	_MyCloseDevice
	XREF	_OldCloseDevice

_MyCloseDevice:
	move.l	_OldCloseDevice,a0
	move.l	a1,-(sp)	;save A1
	jsr	(a0)
	move.l	(sp)+,a0	;restore to A0
	clr.l	IO_DEVICE(a0)
	rts



;*********************************************************************
;
; A MOVEM instruction will be used to trash undefined registers just before
; return from the function.
;
_TrashRegisters:
	DC.L	$dfe181,$dfe183,$dfe185,$dfe187


	END
