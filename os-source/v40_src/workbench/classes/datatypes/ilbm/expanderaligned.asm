        OPTIMON

;---------------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/ports.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/tasks.i"
	INCLUDE "graphics/gfx.i"
	INCLUDE "classbase.i"

;---------------------------------------------------------------------------

	XDEF	@BodyExpanderAligned

;---------------------------------------------------------------------------

   STRUCTURE StartupPkt,MN_SIZE
	ULONG	sp_BitMap
	UWORD	sp_DestBytesPerRow
	UWORD	sp_SrcBytesPerRow
	UWORD   sp_Depth
	UWORD   sp_MaskDepth
	UBYTE	sp_CompressionType
	UBYTE	sp_Pad

	; response from sub-task
	ULONG	sp_ExpanderPort
   LABEL StartupPkt_SIZEOF

;---------------------------------------------------------------------------

BUFFER_SIZE equ 4096

   STRUCTURE DataPkt,MN_SIZE
	LONG	dp_NumBytes	; <= 0 for termination
	LONG	dp_Error
	STRUCT	dp_Data,BUFFER_SIZE
   LABEL DataPkt_SIZEOF

;---------------------------------------------------------------------------

; General register usage:
;
;  a2  current packet from main task
;  a3  pointer to current source byte
;  a4  pointer to current destination byte
;  a5  packet port pointer
;  a6  exec base
;  d2  bytes per row
;  d3  lower word has # of rows minus 1
;  d4  bytes left to process in current packet
;  d5  lower word has number of bytes remaining to be copied in the current row
;  d6  lower word has current command byte or operand

;---------------------------------------------------------------------------

@BodyExpanderAligned:
	move.l	4,a6				; load SysBase

	; extract the startup packet
	move.l	ThisTask(a6),a2
	move.l	TC_Userdata(a2),a2

	; create the communication port
	CALL	CreateMsgPort
	move.l	d0,sp_ExpanderPort(a2)
	bne.s	GotPort

	; if we didn't get a port, bail out
	CALL	Forbid
	move.l	a2,a1
	GO	ReplyMsg

GotPort:
	move.l	d0,a5				; port pointer

	; extract required info from startup packet
	move.w	sp_DestBytesPerRow(a2),d2	; destBytesPerRow
	move.l	sp_BitMap(a2),a0		; destination BitMap pointer
	move.w	bm_Rows(a0),d3
	sub.w	#1,d3				; numRows - 1
	move.l	bm_Planes(a0),a4		; destination pointer

	; return the startup packet so the main task will start reading
	move.l	a2,a1
        CALL	ReplyMsg

	moveq.l	#0,d4		; bufBytes = 0
	sub.l	a2,a2		; data = NULL

RowLoop:
	move.w	d2,d5		; bytes in this row

LineLoop:
	subq.w	#1,d4		; bufBytes--
	bge.s	0$
	bsr.s	WaitData
	subq.w	#1,d4		; bufBytes--

0$:	move.b	(a3)+,d6	; get a control byte
	blt.s	NegativeByte	; if negative...

PositiveByte:
	; source byte >= 0, so copy d6+1 bytes litterally
	ext.w	d6
	addq.w	#1,d6		; n++

	sub.w	d6,d5
	bge.s	4$
	bra.s	Error		; if ((destBytes -= n) < 0) then error

1$:	; empty current buffer
	add.w	d6,d4		; bufBytes += n
	sub.w	d4,d6		; n -= bufBytes
	bra.s	3$
2$:	move.b	(a3)+,(a4)+	; PutByte(GetByte())
3$:	dbra	d4,2$
	bsr.s	WaitData

4$:	sub.w	d6,d4		; bufBytes -= n
	blt.s	1$

	subq.w	#1,d6
5$	move.b	(a3)+,(a4)+
	dbra	d6,5$

	tst.w	d5
        bne.s	LineLoop
	dbra	d3,RowLoop	; loop if upper bit of d6 is clear, and some planes are left
	bra.s	Exit

NegativeByte:
	; source byte < 0
	neg.b	d6		; sets V flag if d6 has -128
	bvs.s	LineLoop	; if d6 contained -128, then move on...

	; source byte < 0 and not -128, so replicate the next byte
	subq.w	#1,d4
        bge.s	0$
	bsr.s	WaitData
	subq.w	#1,d4		; bufBytes--

0$	ext.w	d6
	sub.w	d6,d5
        ble.s	Error		; if ((destBytes -= n) < 0) then error

	move.b	(a3)+,d0
1$	move.b	d0,(a4)+
	dbra	d6,1$

	subq.w	#1,d5
	bne.s	LineLoop
	dbra	d3,RowLoop	; loop if upper bit of d6 is clear, and some planes are left
	bra.s	Exit

;---------------------------------------------------------------------------

WaitData:
	; reply current message if there is one
	move.l	a2,d0   	; to set the CC
	beq.s	1$		; no message, skip
	move.l	a2,a1
        CALL	ReplyMsg

	; now wait for the next message
1$	move.l	a5,a0
	CALL	WaitPort
	move.l	a5,a0
	CALL	GetMsg
	move.l	d0,a2		; pointer to DataPkt
	move.l	dp_NumBytes(a2),d0
	bgt.s	2$

	; no bytes in packet, indicates termination, so exit
	CALL	Forbid
	move.l	a2,a1
	CALL	ReplyMsg
	addq.l	#4,sp		; remove useless return address
	bra.s	DeletePort
2$
	; set up registers from the data packet
	lea	dp_Data(a2),a3
	move.l	d0,d4
	rts

;---------------------------------------------------------------------------

Error:
	st	dp_Error(a2)

Exit:
	CALL	Forbid

	; if we have a message, reply it
        move.l	a2,d0			; current data packet
        beq.s	1$			; if no data packet, skip
	move.l	a2,a1			; set up to reply the current packet
	CALL	ReplyMsg		; reply packet

	; wait until we get a formal termination packet
1$:	move.l	a5,a0			; port pointer
	CALL	WaitPort		; wait for a message
	move.l	a5,a0			; port pointer
	CALL	GetMsg			; get the message that just arrived
	move.l	d0,a2			; packet
	move.l	dp_NumBytes(a2),d2	; extract byte count from packet
	move.l	a2,a1			; packet, to reply
	CALL	ReplyMsg		; return to sender...
	tst.l	d2			; test byte count
	bgt.s	1$			; if > 0, wait some more...

DeletePort:
	move.l	a5,a0
	CALL	DeleteMsgPort
	rts

;---------------------------------------------------------------------------

	END
