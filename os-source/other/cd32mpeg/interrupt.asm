**
**	$Id: interrupt.asm,v 40.3 94/01/26 11:58:08 kcd Exp $
**
**	MPEG Device Interrupt Server.
**
**	(C) Copyright 1993 Commodore-Amiga, Inc.
**	    All Rights Reserved.
**

	INCLUDE "mpeg_device.i"
	INCLUDE "cl450.i"
	INCLUDE "l64111.i"

	XDEF	_MPEGBoardInterrupt
	XDEF	@MPEGBoardInterrupt

	XREF	_SetSCR
	XREF	_kprintf
	XREF	KPutChar

	OPT	P=68020

** DEBUG_INTS	EQU	1
** SHOW_HMEM_CRAP		EQU	1
** SHOW_HMEM_CRAP2		EQU	1
** SHOW_HMEM_CRAP4		EQU	1
**
** MPEGBoardInterrupt
**
** This is the single interrupt server for the MPEG Board.  It used to be
** two separate servers, but this really wasn't needed.
**
** On Entry:
**
** D0/D1/A0/A6 - Scratch
** A1 - Unit Pointer
** A5 - Code Pointer (Scratch)
**
_MPEGBoardInterrupt
@MPEGBoardInterrupt

	move.l	d7,-(sp)

;	Determine which (if either) of the two chips caused an interrupt.

	movea.l	mu_Control(a1),a0	;Fetch Control register pointer
	move.w	(a0),d0			;Fetch Control register contents

	and.w	mu_CL450IntMask(a1),d0	;CL450 Interrupt bit low?
	bne.b	CheckL64111		;Check L64111....

;	Clear pending interrupt from the CL450

	movea.l	mu_CL450(a1),a5		;Chip base address
	move.w	HOST_CONTROL(a5),d0	;Get host control bits.
	ori.w	#$80,d0			;Clear INT bit.
	move.w	d0,HOST_CONTROL(a5)	;Acknowlege the interrupt.

;	The following test is mainly for debugging purposes, and could
;	probably be removed eventually.

	tst.b	mu_CL450IntAble(a1)	;Signal the task?
	beq.b	CheckL64111		;Nope...

;	Let the Unit task know that the CL450 generated an interrupt.
;	We need to use a1, so store our Unit pointer in a5.

	movea.l	a1,a5			;So we don't use any stack space.
	movea.l	mu_SysBase(a1),a6	;Get ExecBase
	move.l	mu_CL450SigMask(a1),d0	;Signal mask
	movea.l	mu_Task(a1),a1		;Unit task

	jsrlib	Signal			;Let them know

	movea.l	a5,a1			;Restore Unit pointer

;	Check the L64111 bit...
;
;	Note... L64111 interrupts are never totally disabled, mainly for
;	synchronization reasons.  There is an "enable" bit stored in the
;	interrupt structure that we use to keep from constantly interrupting
;	the main proces while it's feeding data to the L64111.  It doesn't
;	do us any good to interrupt them while they're already doing a
;	data transfer.

CheckL64111
	clr.l	d7
	movea.l	mu_Control(a1),a0	;Fetch Control register pointer.
	move.w	(a0),d0			;Fetch Control register contents.

	and.w	mu_L64111IntMask(a1),d0	;L64111 Interrupt bit low?
	bne	ExitInterrupt		;Nope, exit.

	movea.l	mu_L64111(a1),a5	;Get chip base
	move.w	STATUS_INT_1(a5),d1	;Read (and clear) INT_1
	move.w	STATUS_INT_2(a5),d0	;Read (and clear) INT_2

	cmp.w	#$3,mu_StreamType(a1)	;Are we playing a system stream?
	bne	No_Sync			;Nope, don't bother with sync stuff.

	btst	#$5,d0			;New PTS?
	beq.b	No_PTS

	move.l	d0,-(sp)		;Save int status bits

	; First read out the new timestamp

	move.w	PRES_TS_IV(a5),d1	;Upper 8 bits	D1 = xxx0
	lsl.w	#8,d1			;		D1 = xx0x
	move.w	PRES_TS_III(a5),d0	;Next 8 bits	D0 = xxx1
	move.b	d0,d1			;		D1 = xx01
	swap	d1			;		D1 = 01xx
	move.w	PRES_TS_II(a5),d1	;Next 8 bits	D1 = 01x2
	lsl.w	#8,d1			;		D1 = 012x
	move.w	PRES_TS_I(a5),d0	;		D0 = xxx3
	move.b	d0,d1			;		D1 = 0123

	; Determine the next channel buffer frame that will be written to,
	; and set it's timestamp for use later when that frame is played.
	; Only set the first one, though.

	move.w	CHANL_BUF_WR_CTR(a5),d0	;Next Buffer to be written
	and.l	#$7f,d0
	lea	mu_VPTS(a1),a0		;Valid Table
	tst.b	(a0,d0.l)
	bne.b	1$			;Don't write over the first timestamp!

	move.b	#1,(a0,d0.l)		;Set valid flag
	adda.w	#128,a0
	move.l	d1,(a0,d0.l*4)		;Set PTS value

1$	move.l	(sp)+,d0		;restore int bits

	; Check for a Frame Out interrupt.  If we got one, see if we have
	; a previously stored timestamp for that frame and if so, update
	; the SCR on the CL450 to bring the Video in sync.

No_PTS	btst	#$0,d0			;New Frame Out?
	IFND	CHECK_TIMES
	beq	No_Sync			;Nope.
	ELSE
	beq	No_Sync
	ENDC

75$	; The channel buffer read counter holds the value of the _next_ frame
	; to be played, so we need to subtract one, and check for possible
	; wraparound.

	move.w	CHANL_BUF_RD_CTR(a5),d0
	and.l	#$7f,d0
	subq.l	#1,d0
	bpl.b	1$

	; Oops, we wrapped.  Check out whether we are layer
	; I or II, and adjust the counter.

	move.l	#63,d0			;We know it has to be at least here

	move.w	PARAMETRIC_DATA_WD_I(a5),d1
	btst.l	#5,d1
	beq.b	1$			;Layer II, leave it at 63
	move.b	#127,d0			;Layer I highest buffer

	; Okay, now see if there's a valid timestamp for this
	; buffer.  If there isn't, we'll punt.

1$
	lea	mu_VPTS(a1),a0		;Base of array
	tst.b	(a0,d0.w)		;Valid?
	IFND	CHECK_TIMES
	beq	No_Sync			;We won't sync without a valid timestamp.
	ELSE
	beq	No_Sync
	ENDC
	clr.b	(a0,d0.w)		;Invalidate timestamp

	adda.w	#128,a0			;Next table
	move.l	(a0,d0.w*4),d1    	;Read timestamp

	tst.b	mu_CL450CmdSem(a1)	;Is the CL450 being issued a command?
	bne	No_Sync			;Yes, don't play with it now.

	IFD	SHOW_HMEM_CRAP
	clr.l	d0
	movem.l	d0-d7/a0-a6,-(sp)
	movea.l	mu_CL450(a1),a5
	move.w	HOST_RADDR1(a5),d2
	move.w	#$c,HOST_RADDR1(a5)
	move.w	HOST_RDATA1(a5),d0
	move.w	#$c,HOST_RADDR1(a5)
	move.w	#$0,HOST_RDATA1(a5)
	tst.l	d0
	beq	45$
	swap	d0
	move.w	#$d,HOST_RADDR1(a5)
	move.w	HOST_RDATA1(a5),d0
	move.w	#$d,HOST_RADDR1(a5)
	move.w	#$0,HOST_RDATA1(a5)
	swap	d0
	move.l	d0,-(sp)
	pea	msg_delay(pc)
	jsr	_kprintf
	adda.w	#8,sp

45$	clr.l	d0
	move.w	HOST_RADDR1(a5),d2
	move.w	#$e,HOST_RADDR1(a5)
	move.w	HOST_RDATA1(a5),d0
	move.w	#$e,HOST_RADDR1(a5)
	move.w	#$0,HOST_RDATA1(a5)
	tst.l	d0
	beq	46$
	swap	d0
	move.w	#$f,HOST_RADDR1(a5)
	move.w	HOST_RDATA1(a5),d0
	move.w	#$f,HOST_RADDR1(a5)
	move.w	#$0,HOST_RDATA1(a5)
	swap	d0
	move.l	d0,-(sp)
	pea	msg_skip(pc)
	jsr	_kprintf
	adda.w	#8,sp

46$
	move.w	d2,HOST_RADDR1(a5)
	movem.l	(sp)+,d0-d7/a0-a6
	ENDC

;       Debugging code.  Check the difference between the SCR we are about
;	to send to the CL450 and it's current clock value.  Once video is
;	playing smoothly, it should never vary by that much.

	movem.l	d1-d6/a0-a6,-(sp)	;Save these

	movea.l	a1,a4			;move unit pointer
	move.l	d1,-(sp)		;Save the generated clock value

        movea.l	mu_Device(a4),a6	;Get mpeg device base
	movea.l	md_TimerIO+IO_DEVICE(a6),a6 ;Get timer.device base

	suba.w	#12,sp			;Temporary storage

	lea.l	4(sp),a0		;EClockVal *
	jsrlib	ReadEClock		;Get Current EClock
	move.l	d0,(sp)			;freq
	move.l	4(sp),d0		;Get High
	move.l	8(sp),d1		;Get Low
	move.l	mu_LastEHigh(a4),d2
	sub.l	mu_LastELow(a4),d1 	;Do low part
	subx.l	d2,d0			;Do high part with carry
	move.l	#1000,d2		;Multiply by 1000
	bsr.w	Mul64by16
	move.l	#90,d2  		;Multiply by 1000
	bsr.w	Mul64by16
	move.l	(sp),d2			;get frequency
	bsr.w	_Div64by32		;
        move.l	d0,(sp)			;d0 = scr's since last "valid" pts

	move.l	12(sp),d1		;d1 = new PTS
	sub.l	mu_LastPTSCheck(a4),d1	;d1 = New PTS - Last PTS

	sub.l	d0,d1
	bpl.b	94$
	neg.l	d1

	;Do a simple bounds check for the possibility of a single audio
	;frame error.

94$	cmp.l	#90,d1
	scc	d7

93$	move.l	4(sp),mu_LastEHigh(a4)
	move.l	8(sp),mu_LastELow(a4)	;Save for next time

	adda.w	#12,sp

	move.l	(sp)+,d1
	movem.l	(sp)+,d1-d6/a0-a6

	move.l	d1,mu_LastPTSCheck(a1)  ;Save for next time
	tst.l	d7
	bne.b	No_Sync

;	Now update the SCR value on the CL450. d1 contains the new SCR
;	value to use.

	movem.l	d2-d4/a4,-(sp)		;Save registers
        movea.l	a1,a4			;Move Unit base
        movea.l	mu_Device(a4),a6	;Get Device base

	move.l	d1,d2			;d2 = hhmmmmmmmmmmmmmm mlllllllllllllll
	move.l	d1,d3                   ;d3 = hhmmmmmmmmmmmmmm mlllllllllllllll
	move.l	d1,d4			;d4 = hhmmmmmmmmmmmmmm mlllllllllllllll

	rol.l	#2,d2			;d2 = mmmmmmmmmmmmmmml llllllllllllllhh
	lsl.l	#1,d3			;d3 = hmmmmmmmmmmmmmmm lllllllllllllll0
	swap	d3			;d3 = lllllllllllllll0 hmmmmmmmmmmmmmmm

	jsr	_SetSCR			;Set SCR

;	Let the Unit process know that we've got the CL450's clock sync'd to
;	the audio stream.

	movea.l	mu_Task(a4),a1
	movea.l	mu_SysBase(a4),a6
	move.l	mu_SyncSigMask(a4),d0
	jsrlib	Signal

2$	movea.l	a4,a1			;Restore unit pointer
	movem.l	(sp)+,d2-d4/a4		;Restore registers

	; Check the channel buffer depth.  If it drops below a certain
	; amount then signal the main task if we haven't already.

No_Sync	move.w	CHAN_BUF_STAT_CTR(a5),d0
	cmp.w	#32,d0
	bcc.b	ExitL64111Interrupt

	tst.b	mu_L64111IntAble(a1)	;Signal the task?
	beq.b	ExitL64111Interrupt

        clr.b	mu_L64111IntAble(a1)	;"Disable" the interrupt.
	movea.l	mu_SysBase(a1),a6	;Get ExecBase
	move.l	mu_L64111SigMask(a1),d0
        movea.l	mu_Task(a1),a1

	jsrlib	Signal			;Signal the main process.

ExitL64111Interrupt

	IFD	DEBUG_INTS
	move.w	#$f,$dff180
	movem.l	d0-d7/a0-a6,-(sp)
	pea	msg5(pc)
	jsr	_kprintf2
	adda.w	#4,sp
	movem.l	(sp)+,d0-d7/a0-a6
	ENDC

ExitInterrupt
	move.l	(sp)+,d7

	moveq	#0,D0
	rts

*
* Div64by32
*
* Divides a 64 bit unsigned number in D0 (ms), D1 (ls) by a 32 bit unsigned
* number in D2.  Returns the quotient in D0.
*
*
_Div64by32:
        movem.l         d2-d6,-(sp)
        moveq.l         #0,d3           * Overflow
        move.l          d3,d6           * Answer
        move.l          #64+1,d5        * Count
1$
        tst.l           d5
        beq.s           99$
        subq.l          #1,d5
        lsl.l           #1,d6

        cmp.l           d2,d3
        blo.s           2$
        bset            #0,d6
        sub.l           d2,d3
2$

        asl.l           #1,d1
        roxl.l          #1,d0
        roxl.l          #1,d3
        bra.s           1$
99$     move.l          d6,d0
        movem.l         (sp)+,d2-d6
        rts

*
* Mul64by16
*
* Entry:
*       D0, D1 make up the 64 bit number
*       the lower portion of D2 makes up the 16 bit number
*
* Exit:
*       D0, D1 make up the 64-bit product
*
Mul64by16:
        movem.l     d2-d4/a0,-(sp)
        sub.l       #8,sp
        move.l      sp,a0
        move.l      #0,(a0)      * product msl

        move.w      d2,d3
        move.w      d1,d4
        mulu        d3,d4
        move.l      d4,4(a0)       * product lsl

        move.l      d1,d4
        swap        d4
        mulu        d3,d4
        add.l       2(a0),d4
        move.l      d4,2(a0)

        move.w      d0,d4
        mulu        d3,d4
        add.l       (a0),d4
        move.l      d4,(a0)

        move.l      d0,d4
        swap        d4
        mulu        d3,d4
        add.w       (a0),d4
        move.w      d4,(a0)

        move.l      (a0),d0
        move.l      4(a0),d1

        add.l       #8,sp
        movem.l     (sp)+,d2-d4/a0
        rts

	IFNE	0

msg3	dc.b	'I',0
msg_pts	dc.b	'P %012ld %03ld |',0
msg_nopts dc.b	'- ------------ --- |',0
msg_pts2 dc.b	'F %012ld %03ld |',0
msg_scr dc.b	'S %012ld |',0
msg_dt	dc.b	'D %012ld | P %012ld',0
msg1:	dc.b	'c: %04lx m: %04lx',10,0
msg9:	dc.b	'i',0
msg99:	dc.b	'%012ld',10,0
msg_delay	dc.b	'D %012ld',10,0
msg_skip	dc.b	'S %012ld',10,0
msg_scrb	dc.b	'%08lx ',10,0
msg_scra	dc.b	'%02lx %04lx %04lx ',0
msg_scrc	dc.b	'%02lx %04lx %04lx',10,0
	cnop	0,2

_kprintf2
	rts

	ENDIF

	end