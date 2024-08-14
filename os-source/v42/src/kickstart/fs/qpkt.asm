		SECTION filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"libraries/dosextens.i"
		INCLUDE	"globals.i"
		INCLUDE	"printf.mac"
		LIST

		XREF	_LVOPutMsg
		XDEF	Qpkt

;==============================================================================
; Qpkt( pkt ),_AbsExecBase
;       a0	   a6
; when a packet received from TaskWait() has been processed it is returned to
; the sender (with result fields filled in) by this routine.
;==============================================================================
Qpkt		move.l	a2,-(sp)		save regs
		movea.l	a0,a2			stash packet address
		move.l	dp_Link(a2),d0		is message address valid ?
		beq.s	Qpkt_Err		nope, return an error
		move.l	ThisDevProc(a5),d0	get our message port address
		move.l	dp_Port(a2),a0		port we will PutMsg to
		move.l	d0,dp_Port(a2)		point packet back at our port
		movea.l	dp_Link(a2),a1		get the message pointer
		move.l	a2,LN_NAME(a1)		make sure it points to packet
		jsr	_LVOPutMsg(a6)		send it on its way
		moveq.l	#TRUE,d0		return no error
Qpkt_Err	move.l	(sp)+,a2
		rts

		END

