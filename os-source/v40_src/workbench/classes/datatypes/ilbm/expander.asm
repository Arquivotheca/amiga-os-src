        OPTIMON

;---------------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/ports.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/tasks.i"
	INCLUDE "graphics/gfx.i"
	INCLUDE "classbase.i"

;---------------------------------------------------------------------------

	XDEF	@BodyExpander

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

; Stack usage:
;
;     (sp)  24 longwords of plane pointers, initialized to bm_Planes[i]
;  $60(sp)  contains a byte boolean for compression or not
;  $64(sp)  message port pointer
;
; General register usage:
;
;  a0  pointer to current plane pointer on stack
;  a1  temporary, used for message pointer in ReplyMsg() calls
;  a2  current packet from main task
;  a3  pointer to current source byte
;  a4  pointer to current destination byte
;  a5  bytes per row in source (modulo)
;  a6  exec base
;  d0  temporary stuff
;  d2  bytes per row in destination (modulo)
;  d3  upper word has # of planes to process minus 1, lower word has # of rows minus 1
;  d4  bytes left to process in current packet
;  d5  upper word has # of mask planes minus 1, lower word has number of bytes
;      remaining to be copied in the current row
;  d6  upper bit indicates whether there is a mask, lower word has
;      current command byte or operand
;  d7  lower word serves as plane counter, upper word serves as temp storage

;---------------------------------------------------------------------------

@BodyExpander:
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
	lea	-$68(sp),sp			; reserve room on the stack
	move.l	d0,$64(sp)			; port pointer

	; extract required info from startup packet
	moveq	#0,d2
	move.w	sp_DestBytesPerRow(a2),d2	; destBytesPerRow
	move.w	sp_SrcBytesPerRow(a2),a5	; srcBytesPerRow
	move.b	sp_CompressionType(a2),$60(sp)	; compression
	move.l	sp_BitMap(a2),a0		; destination BitMap pointer
	move.w	sp_Depth(a2),d3         	; numPlanes
	subq.w	#1,d3
	move.w	d3,d0
	swap	d3
	move.w	bm_Rows(a0),d3
	sub.w	#1,d3				; numPlanes - 1, numRows - 1

	moveq	#0,d5				; clear mask plane count
	moveq	#0,d6
	move.w	sp_MaskDepth(a2),d5		; is there a mask?
	beq.s	0$
	move.l	#$80000000,d6			; upper bit of d6 tells us if there is a mask
	subq.w	#1,d5
	swap	d5				; upper word has # mask planes - 1

	; copy the bm_Planes pointer to the local stack-based "lines" array
0$:	move.l	sp,a3		 		; lines array on stack
	lea	bm_Planes(a0),a0
1$:	move.l	(a0)+,(a3)+
2$:	dbra	d0,1$

	; return the startup packet so the main task will start reading
	move.l	a2,a1
        CALL	ReplyMsg

	moveq.l	#0,d4			; bufBytes = 0
	sub.l	a2,a2			; data = NULL

	tst.b	$60(sp)			; check compression type
	beq	Uncompressed		; no compression
	bra.s	Compressed

;---------------------------------------------------------------------------

; these are here just to let certain of the branches in the code below be
; short instead of long, in order for the code to fit in the instruction
; cache
ErrorStub1:
	bra	Error

WaitDataStub1:
	bra	WaitData

;---------------------------------------------------------------------------

Compressed:

CRowLoop:
	move.l	d3,d7
	swap	d7		; d7 contains numPlanes-1
	move.l	sp,a0		; address of lines array

CPlaneLoop:
	move.l	(a0),a4		; dest          = lines[plane]
	add.l	d2,(a0)+	; lines[plane] += destBytesPerRow
	move.w	a5,d5		; destBytes     = srcBytesPerRow

CLineLoop:
	subq.w	#1,d4		; bufBytes--
	bge.s	0$
	bsr.s	WaitDataStub1
	subq.w	#1,d4		; bufBytes--

0$:	move.b	(a3)+,d6	; get a control byte
	blt.s	BE_32		; if negative...

	; source byte >= 0, so copy d6+1 bytes litterally
	ext.w	d6
	addq.w	#1,d6		; n++

	sub.w	d6,d5
	bge.s	BE_28
	bra.s	ErrorStub1	; if ((destBytes -= n) < 0) then error

BE_20:	; empty current buffer
	add.w	d6,d4		; bufBytes += n
	sub.w	d4,d6		; n -= bufBytes
	bra.s	2$
1$:	move.b	(a3)+,(a4)+	; PutByte(GetByte())
2$:	dbra	d4,1$
	bsr.s	WaitDataStub1

BE_28:	sub.w	d6,d4		; bufBytes -= n
	blt.s	BE_20

	subq.w	#1,d6
1$	move.b	(a3)+,(a4)+
	dbra	d6,1$

	tst.w	d5
        bne.s	CLineLoop

	dbra	d7,CPlaneLoop
	bra.s	BE_47

BE_32:	; source byte < 0
	neg.b	d6		; sets V flag if d6 has -128
	bvs.s	CLineLoop	; if d6 contained -128, then move on...

	; source byte < 0 and not -128, so replicate the next byte
	subq.w	#1,d4
        bge.s	0$
	bsr.s	WaitData
	subq.w	#1,d4		; bufBytes--

0$	ext.w	d6
	sub.w	d6,d5
        ble.s	ErrorStub1	; if ((destBytes -= n) < 0) then error

	move.b	(a3)+,d0
1$	move.b	d0,(a4)+
	dbra	d6,1$

	subq.w	#1,d5
	bne.s	CLineLoop

	dbra	d7,CPlaneLoop

;--

BE_47:
	; throw away the mask planes, if any
	tst.l	d6		; is upper bit set?
	dbmi	d3,CRowLoop	; loop if upper bit of d6 is clear, and some planes are left
	bpl	Exit		; if upper bit of d6 was clear, then we ran out of planes

	move.l	d5,d7
	swap	d7		; num mask planes - 1

CPlaneLoopMask:
	move.w	a5,d5		; destBytes = srcBytesPerRow

CLineLoopMask:
	subq.w	#1,d4
	bge.s	0$
	bsr.s	WaitData
	subq.w	#1,d4

0$:	move.b	(a3)+,d6
	blt.s	BE_68

	ext.w	d6
	addq.w	#1,d6
	sub.w	d6,d5
	bge.s	BE_64
	bra.s	ErrorStub2	; if ((destBytes -= n) < 0) then error

BE_59:	move.w	d6,d0
	sub.w	d4,d0
	sub.w	d6,d0
	move.w	d0,d6
	bsr.s	WaitData

BE_64:	sub.w	d6,d4
        blt.s	BE_59

BE_65:	addq.l	#1,a3
	subq.w	#1,d6
	bgt.s	BE_65
	tst.w	d5
	bra.s	BE_78

BE_68:
	; source byte < 0
	neg.b	d6		; sets V flag if d6 has -128
	bvs.s	CLineLoopMask   ; if d6 contained -128, then move on...

	; source byte < 0 and not -128, so replicate the next byte
	ext.w	d6

	subq.w	#1,d4
	bge.s	0$
	bsr.s	WaitData
	subq.w	#1,d4

0$:	sub.w	d6,d5
	ble.s	ErrorStub2	; if ((destBytes -= n) < 0) then error
	addq.l	#1,a3
	subq.w	#1,d5

BE_78:
	bne.s	CLineLoopMask
	dbra	d7,CPlaneLoopMask

	dbra	d3,CRowLoop
	bra.s	Exit

;---------------------------------------------------------------------------

; this is here just to let certain of the branches in the code below be
; short instead of long, in order for the code to fit in the instruction
; cache
ErrorStub2:
	bra.s	Error

;---------------------------------------------------------------------------

WaitData:
	move.l	a0,d4

	; reply current message if there is one
	move.l	a2,d0   	; to set the CC
	beq.s	1$		; no message, skip
	move.l	a2,a1
        CALL	ReplyMsg

	; now wait for the next message
1$	move.l	$68(sp),a3	; address of port
	move.l	a3,a0
	CALL	WaitPort
	move.l	a3,a0
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
	move.l	d4,a0
	lea	dp_Data(a2),a3
	move.l	d0,d4
	rts

;---------------------------------------------------------------------------

Error:
	st	dp_Error(a2)

Exit:
	CALL	Forbid

	; if we have a message, reply it
	move.l	$64(sp),a3		; port pointer
        move.l	a2,d0			; current data packet
        beq.s	1$			; if no data packet, skip
	move.l	a2,a1			; set up to reply the current packet
	CALL	ReplyMsg		; reply packet

	; wait until we get a formal termination packet
1$:	move.l	a3,a0			; port pointer
	CALL	WaitPort		; wait for a message
	move.l	a3,a0			; port pointer
	CALL	GetMsg			; get the message that just arrived
	move.l	d0,a2			; packet
	move.l	dp_NumBytes(a2),d2	; extract byte count from packet
	move.l	a2,a1			; packet, to reply
	CALL	ReplyMsg		; return to sender...
	tst.l	d2			; test byte count
	bgt.s	1$			; if > 0, wait some more...

DeletePort:
	move.l	a3,a0
	CALL	DeleteMsgPort
	lea	$68(sp),sp
	rts

;---------------------------------------------------------------------------

Uncompressed:

URowLoop:
	move.l	d3,d7
	swap	d7		; d7 has numPlanes - 1
	move.l	sp,a0		; address of lines array

UPlaneLoop:
	tst.w	d4		; any bytes left in buffer?
	bne.s	0$		; if so, just enter loop
	bsr	WaitData	; if not, get some more

0$:	move.l	(a0),a4		; dest          = lines[plane];
	add.l	d2,(a0)+	; lines[plane] += destBytesPerRow;
	move.w	a5,d5		; destBytes     = srcBytesPerRow;

ULineLoop:
	move.w	(a3)+,(a4)+	; copy two bytes of data
	subq.w	#2,d5		; subtract the bytes we copied from the dest count
	beq.s	1$		; no more bytes to copy in this line, go do next
	subq.w	#2,d4		; subtract the bytes we copied from the src count
	bne.s	ULineLoop	; yep, so keep going
	bsr	WaitData	; nope, get more bytes
	bra.s	ULineLoop	; reenter loop

1$:	subq.w	#2,d4		; subtract the bytes we copied from the src count
	dbra	d7,UPlaneLoop   ; done with this line, do to next plane
	dbra	d3,URowLoop	; done with this row, do the next one

	; throw away the mask planes, if any
	tst.l	d6		; is upper bit set?
	dbmi	d3,CRowLoop	; loop if upper bit of d6 is clear, and some planes are left
	bpl	Exit		; if upper bit of d6 was clear, then we ran out of planes

	move.l	d5,d7
	swap	d7		; num mask planes - 1

UPlaneLoopMask:
	tst.w	d4		; any bytes left in buffer?
	bne.s	0$		; if so, just enter loop
	bsr	WaitData	; if not, get some more

0$:	move.l	(a0),a4		; dest          = lines[plane];
	add.l	d2,(a0)+	; lines[plane] += destBytesPerRow;
	move.w	a5,d5		; destBytes     = srcBytesPerRow;

ULineLoopMask:
	addq.l	#2,a3		; throw away two bytes of data
	subq.w	#2,d5		; subtract the bytes we copied from the dest count
	beq.s	1$		; no more bytes to copy in this line, go do next
	subq.w	#2,d4		; subtract the bytes we copied from the src count
	bne.s	ULineLoopMask	; yep, so keep going
	bsr	WaitData	; nope, get more bytes
	bra.s	ULineLoopMask	; reenter loop

1$:	subq.w	#2,d4		; subtract the bytes we copied from the src count
	dbra	d7,UPlaneLoopMask; done with this line, do to next plane
	dbra	d3,URowLoop	; done with this row, do the next one
	bra	Exit		; life is over...

;---------------------------------------------------------------------------

	END
