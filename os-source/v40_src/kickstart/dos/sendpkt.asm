*
* sendpkt.asm
*
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/memory.i"
	INCLUDE "exec/alerts.i"
	INCLUDE "exec/ports.i"
	INCLUDE "exec/execbase.i"
	INCLUDE	"dos/dos.i"
	INCLUDE "dos/dosextens.i"


	XREF	@abort
	XREF	@taskwait
	XREF	@setresult2
	XREF	@qpkt
	XREF	@qpkt_task
	XREF	_LVOPutMsg
	XREF	_LVOCreateMsgPort
	XREF	_LVODeleteMsgPort
*
* External symbols
*
	XDEF	@sendpacket
	XDEF	_sendpacket
	XDEF	@sendpkt4
	XDEF	_sendpkt4
	XDEF	@sendpkt3
	XDEF	_sendpkt3
	XDEF	@sendpkt2
	XDEF	_sendpkt2
	XDEF	@sendpkt1
	XDEF	_sendpkt1
	XDEF	@sendpkt0
	XDEF	_sendpkt0
	XDEF	@SendPkt
	XDEF	_SendPkt

	IFD	OLD_CREATEPKT
	XDEF	@CreatePkt
	XDEF	_CreatePkt
	XDEF	@DeletePkt
	XDEF	_DeletePkt
	ENDC

*
* Call EXEC with A6 already set
*
CALL    MACRO
        JSR          _LVO\1(A6)
        ENDM

* /* WARNING: called with variable # of args. */
* /* also must be callable from a task! */
*
* type == action
* 
* LONG
* sendpacket (port,type,arg1,arg2,arg3,arg4,arg5)
* 	struct MsgPort *port;
* 	LONG type,arg1,arg2,arg3,arg4,arg5;
* {
* 	struct StandardPacket pkt;
*	struct MsgPort *port;
*	LONG free_it = FALSE;
* 
* 	pkt.sp_Pkt.dp_Link = &(pkt.sp_Msg);
* 	pkt.sp_Pkt.dp_Port = port;
* 	pkt.sp_Pkt.dp_Type = type;
* 	pkt.sp_Pkt.dp_Arg1 = arg1;
* 	pkt.sp_Pkt.dp_Arg2 = arg2;
* 	pkt.sp_Pkt.dp_Arg3 = arg3;
* 	pkt.sp_Pkt.dp_Arg4 = arg4;
* 	pkt.sp_Pkt.dp_Arg5 = arg5;
*
*	if (this is a task)
*	{
*		port = CreateMsgPort();
*		if (!port) return 0;
*		free_it = TRUE;
*	} else {
*		port = &(SysBase->ThisTask.pr_MsgPort);
*	}
* 	if (qpkt_task(&(pkt.sp_Pkt),port,SysBase) == 0)
* 		abort(AN_QPktFail);
* 
* 	if (taskwait(port) != &(pkt.sp_Pkt))
* 		abort(AN_AsyncPkt);
*
*	if (free_it)
*		DeleteMsgPort(port);
*
* 	setresult2(pkt.sp_Pkt.dp_Res2);
* 	return pkt.sp_Pkt.dp_Res1;
* }

@sendpacket:
_sendpacket:
@sendpkt4:
_sendpkt4:
@sendpkt3:
_sendpkt3:
@sendpkt2:
_sendpkt2:
@sendpkt1:
_sendpkt1:
@sendpkt0:
_sendpkt0:
	movem.l	a2-a3/a6,-(sp)

	; first check stack alignment - need LW alignment!!!
	move.l	a7,d0
	and.b	#3,d0
	beq.s	aligned
	link	a5,#-(sp_SIZEOF+2)	; not LW aligned!
	bra.s	linkdone
aligned:
	link	a5,#-(sp_SIZEOF)	; put a standard packet on the stack

linkdone:
	move.l	a7,d0			; &(pkt.sp_Msg)
	movem.l d0-d2,sp_Pkt(a7)	; sets link, port, and type fields

	; now we need to set the arg fields
	movem.l	d3-d7,sp_Pkt+dp_Arg1(a7) ; sets arg fields 1-5

	moveq	#sp_Pkt,d1
	add.l	a7,d1			; get addr of packet
	move.l	SysBase,a6
	move.l	ThisTask(a6),a2		; current task/process
	cmp.b	#NT_PROCESS,LN_TYPE(a2)	; are we a task?
	beq.s	is_process

	; this is a task - allocate a port for the replymsg
	move.l	d1,a2			; save pkt ptr
	CALL	CreateMsgPort		; make a msgport (a6 is execbase)
	tst.l	d0			; did it succeed?
	beq.s	nomem			; hope that 0 is a reasonable failure
	move.l	a2,d1			; put pkt back
	move.l	d0,a2			; port for return
	move.l	d0,a3			;  also deallocate it
	bra.s	do_qpkt

is_process:
	moveq	#pr_MsgPort,d0
	add.l	d0,a2			; return port is process port
	sub.l	a3,a3			; no port to free

do_qpkt:
	; do the actual qpkt, d1 is pkt
	bsr	@qpkt_task		; replyport in A2, a6 = execbase
	tst.l	d0
	bne.s	ok

	; packet send failed - impossible
	move.l	#AN_QPktFail!AT_DeadEnd,d1
abort:	bra	@abort			; dead-end alert! no return!

ok:	; wait for packet to return
	move.l	a2,a1			; if it's a task, a1 must be port
	bsr	@taskwait
	lea	sp_Pkt(a7),a0
	cmp.l	a0,d0
	beq.s	1$

	; got wrong packet - give the ever-popular Async packet error
	; dead-end alert!  No return!
	move.l	#AN_AsyncPkt!AT_DeadEnd,d1
	bra.s	abort

1$:	; got right packet back
	move.l	a3,d0			; do we need to deallocate?
	beq.s	no_port
	move.l	a3,a0			; port address
	CALL	DeleteMsgPort		; a6 is execbase
	move.l	sp_Pkt+dp_Res2(a7),d1	; secondary result in d1, I guess
	bra.s	sendpkt_done		; don't do setresult2 for task!

no_port:				; didn't allocate a port - I'm a proc
	move.l	sp_Pkt+dp_Res2(a7),d0
	bsr	@setresult2

sendpkt_done:
	; return dp_res1
	move.l	sp_Pkt+dp_Res1(a7),d0
nomem:
	unlk	a5			; frees pkt
	movem.l	(a7)+,a2-a3/a6
	rts

	IFD OLD_CREATEPKT
******* dos.library/CreatePkt ***********************************************
*
*   NAME
*	CreatePkt -- Creates a StandardPacket
*
*   SYNOPSIS
*	packet = CreatePkt()
*	D0
*
*	struct DosPacket *CreatePkt(void)
*
*   FUNCTION
*	Creates a StandardPacket and returns it to you.  You are returned a
*	pointer to the sp_Pkt portion of the standard packet.  The ln_Name and
*	dp_Link fields are set up correctly.
*
*   RESULT
*	packet - pointer to the sp_Pkt of the StandardPacket or NULL
*

@CreatePkt:
_CreatePkt:
	move.l	d2,-(a7)
	move.l	#MEMF_PUBLIC|MEMF_CLEAR,d2
	moveq	#sp_SIZEOF,d1
	bsr	@getmem
	tst.l	d0
	beq.s	error

	move.l	d0,a0
	lea	sp_Pkt(a0),a1		; pointer to dospacket
	move.l	a1,LN_NAME(a0)		; set up ln_name of node
	move.l	a0,dp_Link(a1)		; and the converse pointer
	move.l	a1,d0			; return value

error:
	move.l	(a7)+,d2
	rts

******* dos.library/DeletePkt **********
*
*   NAME
*	DeletePkt -- Deletes a StandardPacket
*
*   SYNOPSIS
*	DeletePkt(packet)
*		    D1
*
*	void DeletePkt(struct DosPacket *)
*
*   FUNCTION
*	Deletes a StandardPacket created by CreatePkt().  Should not be called
*	for any packets allocated in any other way.
*
*   INPUTS
*	packet - pointer to sp_Pkt of StandardPacket to be deleted.
*

@DeletePkt:
_DeletePkt:
	moveq	#sp_Pkt,d0
	sub.l	d0,d1		; get original pkt ptr (ONLY FOR StandardPkts!)
	bra	@freevec	; avoid bsr;rts pair


	ENDC	; OLD_CREATEPKT


******* dos.library/SendPkt **********
*
*   NAME
*	SendPkt -- Sends a packet to a handler
*
*   SYNOPSIS
*	SendPkt(packet,port,replyport)
*		  D1    D2     D3
*
*	void SendPkt(struct DosPacket *,struct MsgPort *,struct MsgPort *)
*
*   FUNCTION
*	Sends a packet to a handler and does not wait.  All fields in the
*	packet must be initialized (other than dp_Port and mn_ReplyPort)
*	before calling this routine.  Normally you would use your pr_MsgPort
*	for the replyport.
*
*   INPUTS
*	packet      - packet to send, must be initialized and have a message.
*	port        - pr_MsgPort of handler process to send to.
*	replyport   - port to return the message to.
*
SysBase	EQU	4

@SendPkt:
_SendPkt:
	move.l	a6,-(a7)
	move.l	d1,a0
	move.l	dp_Link(a0),a1		; message pointer
	move.l	d3,dp_Port(a0)		; reply port
	move.l	d3,MN_REPLYPORT(a1)	; also here for amusement
					; ln_Name MUST be set up already
	move.l	d2,a0			; for putmsg
	move.l	SysBase,a6
	jsr	_LVOPutMsg(a6)		; send it
	move.l	(a7)+,a6
	rts

	END
