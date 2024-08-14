**
** $Source: HOG:Other/cd32mpeg/RCS/datapump.asm,v $
** $State: Exp $
** $Revision: 40.3 $
** $Date: 94/01/26 11:58:16 $
** $Author: kcd $
**
** Amiga MPEG device driver.
**
** Efficient routines to feed CL450 and L64111
**
** (C) Copyright 1993 Commodore-Amiga, Inc.
**
**
        INCLUDE "exec/types.i"
        INCLUDE "mpeg_device.i"
        INCLUDE "cl450.i"
        INCLUDE "l64111.i"

        XDEF    _FeedCL450
        XDEF	_FeedL64111
	XDEF	@CmpSCR
	XDEF	_ReadLumaBuffer
	XDEF	_ReadChromaBuffer
	XDEF	@WaitCL450Cmd
        XREF    _TermIO
        XREF    _InquireCL450BufferFullness
        XREF    _GetNextVideoIO
	XREF	_GetNextAudioIO
	XREF	_kprintf

        OPT     P=68020

**
** FeedCL450
**
** Register Usage
**
** a6 - mpegDevice
** a4 - mpegUnit
** a5 - CL450 Registers
** a3 - CL450 Fifo
** a2 - Control register pointer
** a0/a1 Scratch
**
** d7 - Transfer count
** d6 - FIFO Busy count
** d5 - cflevel bits
** d2 - StatFlags

_FeedCL450:
	movem.l	d2/d5-d6/a2-a5,-(sp)
	movea.l	a5,a4
	movea.l	mu_CL450(a4),a5
	movea.l	mu_CL450Fifo(a4),a3
	movea.l	mu_Control(a4),a2
	move.w	mu_CFLevelBit(a4),d5
	move.l	#100000,d6

loop_Start:
	move.w	(a2),d0		;Get status register
	and.w	d5,d0		;Check _CFLEVEL pin
	bne.b	fifo_notBusy

2$	sub.w	#1,d6
	beq.b   clearCL450Int
	bra.b	loop_Start

1$	move.w	(a2),d0
	and.w	d5,d0
	beq.b	2$

fifo_notBusy:
	move.l	#100000,d6
	tst.l	mu_VideoIO(a4)	;Do we have a current IO request?
	bne.b	do_transfer	;Yes, use it...

	bsr.w	_GetNextVideoIO	;Try to fetch another one
	tst.l	d0		;Did it work?
	bne.b	do_transfer	;Yep...
	bclr.l	#3,d2		;No data.  Clear flag and exit
	bra.b	exit_FeedCL450	;bye

do_transfer:
	movea.l	mu_VideoData(a4),a1	;Get data pointer
	move.w	#10,d0			;max of 10 words
	move.w	mu_VideoSize(a4),d1	;data left
	cmp.w	d1,d0			;compare
	bcs.b	1$			;less than 12?
	move.w	d1,d0			;yes, use smaller value
1$	sub.w	d0,d1			;subtract from count

	; Do the transfer

	bra.b	3$			;d0 is 1 higher than we need
2$	move.w	(a1)+,(a3)		;transfer a word
3$	dbra.w	d0,2$			;loop

	move.l	a1,mu_VideoData(a4)	;put data pointer back
	move.w	d1,mu_VideoSize(a4)	;save it

	bne.b	loop_Start		;Not yet, send some more

	; Complete the current video io request

	movea.l	mu_VideoIO(a4),a1	;Set up for call
	bsr.w	_TermIO			;Finish it
	clr.l	mu_VideoIO(a4)		;Clear pointer

	; Check CL450 Buffer fullness

	bsr.w	_InquireCL450BufferFullness

	add.b	#1,mu_CL450CmdSem(a4)	;Make sure we don't collide with ints.
	move.w	#READ_BS_BUF_FULL,HOST_RADDR1(a5)
	cmp.w	#CL450_UPPER_THRESHOLD,HOST_RDATA1(a5)
	bcc.b	clearCL450Int2
	btst	#2,mu_PlayFlags+1(a4)	;Clear int if we're scanning.
	bne.b	clearCL450Int2
	cmp.b	#1,mu_DPState(a4)
	beq.b	clearCL450Int2
	bra.b	clearCL450IntDone

clearCL450Int:
	add.b	#1,mu_CL450CmdSem(a4)	;Increment counter

clearCL450Int2:

	; Clear FEEDVIDEO flag and CL450 Int
	bclr.l	#2,d2			;Clear FEEDVIDEO flag
	move.w	#READ_INT_STATUS,HOST_RADDR1(a5)
	clr.w	HOST_RDATA1(a5)

clearCL450IntDone:
	sub.b	#1,mu_CL450CmdSem(a4)	;Enable playing again

	; fall through...

exit_FeedCL450:
	move.l	d2,d0			;Return new status bits
	movem.l	(sp)+,d2/d5-d6/a2-a5
	rts


**
** FeedL64111
**
** Register Usage
**
** a6 - mpegDevice
** a4 - mpegUnit
** a5 - L64111 Registers
** a3 - CL450 Fifo
** a2 - Control register pointer
** a0/a1 Scratch
**
** d7 - Transfer count
** d6 - FIFO Busy count
** d5 - cflevel bits
** d2 - StatFlags

_FeedL64111:
	movem.l	d2/a4,-(sp)
	movea.l	a5,a4
	tst.l	mu_AudioIO(a4)
	bne.b	do_transferA

	bsr.w	_GetNextAudioIO
	tst.l	d0
	bne.b	do_transferA

	bclr.l	#1,d2			;No data.  Clear flag and exit
	bra.b	exit_FeedL64111		;bye

do_transferA:
	movea.l	mu_L64111(a4),a0	;L64111 Registers

	movea.l	mu_AudioData(a4),a1
	move.w	#36,d0
	move.w	mu_AudioSize(a4),d1
	cmp.w	d1,d0
	bcs.b	1$
	move.w	d1,d0
1$	sub.w	d0,d1

	; Do the transfer

	bra.b	3$

2$      move.w  (a1)+,(a0)
3$	dbra.w	d0,2$

	move.l	a1,mu_AudioData(a4)
	move.w	d1,mu_AudioSize(a4)

	bne.b	exit_FeedL64111

	move.l	a0,-(sp)

	movea.l	mu_AudioIO(a4),a1
	bsr.w	_TermIO
	clr.l	mu_AudioIO(a4)

	movea.l	(sp)+,a0

	cmp.w	#L64111_UPPER_THRESHOLD,CHAN_BUF_STAT_CTR(a0)

	bcs.b	exit_FeedL64111

	bclr.l	#0,d2
	move.b	#1,mu_L64111IntAble(a4)

exit_FeedL64111:
	move.l	d2,d0
	movem.l	(sp)+,d2/a4
	rts


**
** CmpSCR
**
** Register Usage
**
** a0 - req1
** a1 - req2
**
@CmpSCR:
	clr.l	d0
	clr.l	d1
	move.w	iomr_SCRHigh(a0),d0
	lsl.l	#8,d0
	lsl.l	#7,d0
	or.w	iomr_SCRMid(a0),d0
	lsl.l	#8,d0
	lsl.l	#7,d0
	or.w	iomr_SCRLow(a0),d0

	move.w	iomr_PTSHigh(a1),d1
	lsl.l	#8,d1
	lsl.l	#7,d1
	or.w	iomr_PTSMid(a1),d1
	lsl.l	#8,d1
	lsl.l	#7,d1
	or.w	iomr_PTSLow(a1),d1

	cmp.l	d0,d1
	scc	d0

	rts

	cnop	0,4
**
** ReadLumaBuffer
**
** Register Usage
**
** a0 - Base
** a1 - Destination
** d0 - Width
** d1 - Height
**

_ReadLumaBuffer:
	movem.l	d2-d7/a2-a4,-(sp)

	;Calculate row_size (in bytes)
	;
	; row_Size = Width+15/16*64

	move.l	d0,d3
	add.l	#15,d3
	and.l	#$fffffff0,d3
	asl.l	#3,d3
	move.l	d3,d7
	add.l	d7,d7
	add.l	d7,d3		;row_Size*3

	;Calculate number of vertical 8-pixel chunks

	move.l	d1,d4		;Copy it
	add.l	#7,d4		;Round up
	asr.l	#3,d4		;Divide by 8
	subq.l	#1,d4		;One less

do_mblock:
	clr.l	d5		;reset y counter
	moveq.l	#7,d7		;(rows per block) - 1

	;Transfer a block row
do_block:
	movea.l	a0,a2		;Row base + 2*(y%8)
	adda.w	d5,a2		;two pixels per word

	;Transfer a single row

	movea.l	d0,a4		;Save it...
	asr.l	#1,d0		;number
	bra.b	do_r

do_row:	move.w	(a2),d2		;grab two pixels
	ror.w	#8,d2		;byte swap them
	move.w	d2,(a1)+	;save two pixels
	adda.w	#16,a2		;skip 8 words
do_r:	dbra.w	d0,do_row	;loop

	addq.l	#2,d5		;next real y value*2
	move.l	a4,d0		;restore width
        dbra.w	d7,do_block	;do next row

        ; Skip to next block

        adda.l	d3,a0
	dbra.w	d4,do_mblock

	movem.l	(sp)+,d2-d7/a2-a4
	rts

**
** ReadChromaBuffer
**
** Register Usage
**
** a0 - Base
** a1 - Destination
** d0 - Width
** d1 - Height
**

_ReadChromaBuffer:
	movem.l	d2-d7/a2-a4,-(sp)

	;Calculate row_size (in bytes)
	;
	; row_Size = (Width*2)+15/16*64

	move.l	d0,d3
	add.l	d3,d3
	add.l	#15,d3
	and.l	#$fffffff0,d3
	asl.l	#3,d3
	move.l	d3,d7
	add.l	d7,d7
	add.l	d7,d3		;row_Size*3

	;Calculate number of vertical 8-pixel chunks

	move.l	d1,d4		;Copy it
	add.l	#7,d4		;Round up
	asr.l	#3,d4		;Divide by 8
	subq.l	#1,d4		;One less

do_cmblock:
	clr.l	d5		;reset y counter
	moveq.l	#7,d7		;(rows per block) - 1

	;Transfer a block row
do_cblock:
	movea.l	a0,a2		;Row base + 2*(y%8)
	adda.w	d5,a2		;two pixels per word

	;Transfer a single row

	movea.l	d0,a4		;Save it...
	asr.l	#1,d0		;number to do
	bra.b	do_cr

do_crow move.w	(a2),d2		;grab two pixels
	ror.w	#8,d2		;byte swap them
	move.w	d2,(a1)+	;save two pixels
	adda.w	#32,a2		;skip 8 words
do_cr:	dbra.w	d0,do_crow	;loop

	addq.l	#2,d5		;next real y value*2
	move.l	a4,d0		;restore width
        dbra.w	d7,do_cblock	;do next row

        ; Skip to next block

        adda.l	d3,a0
	dbra.w	d4,do_cmblock

	movem.l	(sp)+,d2-d7/a2-a4
	rts

@WaitCL450Cmd:
	move.w	CPU_NEWCMD(a0),d0
	and.b	#1,d0
	bne.b	@WaitCL450Cmd
	rts


