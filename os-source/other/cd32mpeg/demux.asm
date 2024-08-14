**
**	$Id: demux.asm,v 40.3 94/01/26 11:58:02 kcd Exp $
**
**	MPEG Device system stream parser.
**
**	(C) Copyright 1993 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

	INCDIR	"include:"
	INCLUDE	"exec/types.i"
	INCLUDE "exec/ports.i"
	INCLUDE "mpeg_device.i"

**
**	Some Useful defines
**

CD_SECTOR_SIZE		EQU	2324
DPFLAGF_VALID_SCR	EQU	1
DPFLAGF_VALID_PTS	EQU	2

ISO_11172_START_CODE	EQU	$000001B9
PACK_START_CODE		EQU	$000001BA
SYSTEM_START_CODE	EQU	$000001BB
AUDIO_START_PREFIX	EQU	$000001C0
AUDIO_START_MASK	EQU	$FFFFFFE0
VIDEO_START_PREFIX	EQU	$000001E0
VIDEO_START_MASK	EQU	$FFFFFFF0
OTHER_START_PREFIX	EQU	$000001B0

DEBUG_DEMUX	EQU	0

**
**	SystemDemux - Accepts an IOMpegRequest and breaks it up
**	into parts suitable for use by the L64111 and/or CL450.
**	Each IOMpegRequest _MUST_ correspond to exactly one MPEG
**	Pack.  Device-external code is responsible for making
**	this happen.
**
**	Registers on Entry:
**
**	A2 - IOMpegRequest pointer
**	a4 - MPEG Unit pointer
**	A6 - MPEG Device pointer
**
**
**	Known Bugs:
**
**	This routine doesn't properly deal with System streams
**	that have packs containing both audio and video packets.
**	At this point, I'm not sure how much of a problem this
**	will be.  Hopefully Karaoke format streams will become
**	the de-facto standard. - KCD
**

	XDEF	_SystemDemux
	XREF	_TermIO
	XREF	_kprintf

_SystemDemux
	movem.l	d2-d7,-(sp)		;Save everything for now...
	movea.l	IO_DATA(a2),a1		;Get pointer to start of Pack
	move.l	IO_LENGTH(a2),d7	;So we don't look forever

	IFNE	DEBUG_DEMUX
	movem.l	d0-d7/a0-a6,-(sp)
	move.l	d7,-(sp)
	pea	msg6(pc)
	jsr	_kprintf
	addq.l	#8,sp
	movem.l	(sp)+,d0-d7/a0-a6
	ENDC

;	Look for a Pack start code.

search_pack_code:
        cmp.l	#PACK_START_CODE,(a1)+
        beq.b	parse_pack
        subq.l	#4,d7
        bne.b	search_pack_code

;	Hrmmm... no pack start code found...return the IO request

	IFNE	DEBUG_DEMUX
	movem.l	d0-d7/a0-a6,-(sp)
	pea	msg4(pc)
	jsr	_kprintf
	addq.l	#4,sp
	movem.l	(sp)+,d0-d7/a0-a6
	ENDC

        bra	toss_it

	; Grab the system clock reference value for this pack.

parse_pack:
	clr.l	d0
	move.b  (a1),d0
	lsr.l	#1,d0
	and.w	#$7,d0
	move.w	d0,iomr_SCRHigh(a2)
	move.w	1(a1),d0
	lsr.w	#1,d0
	move.w	d0,iomr_SCRMid(a2)
	move.w	3(a1),d0
	lsr.w	#1,d0
	move.w	d0,iomr_SCRLow(a2)
	bset.b	#6,iomr_MPEGFlags(a2)	; Valid SCR preset.
	adda.l	#8,a1			; Bump address pointer...
	sub.l	#12,d7			; ...and decrement length

;	Look for a system stream header...

	cmp.l	#SYSTEM_START_CODE,(a1)	; System Header Start code?
	bne.b	do_packets		; nope...

;	Figure out the header's length and skip over it.  I don't
;	use any of this information, but probably should.

	clr.l	d0
	move.w	4(a1),d0
	addq.l	#6,d0
	adda.l	d0,a1
	sub.l	d0,d7

;	Figure out what kind of packet we have.

do_packets:
	move.l	(a1),d0			; Start code?
	move.l	d0,d1
	and.l	#VIDEO_START_MASK,d0  	; Do video first....
	cmp.l	#VIDEO_START_PREFIX,d0	; video packet?
	IFEQ	DEBUG_DEMUX
	beq.b	parse_video_packet	; parse it...
	ELSE
	beq	parse_video_packet
	ENDC
	cmp.l	#OTHER_START_PREFIX,d0  ; Something else?
	beq.b	do_other_packet

	and.b	#$C0,d0			; everything else is already cleared...
	cmp.l	#AUDIO_START_PREFIX,d0	; audio packet?
	beq.b	parse_audio_packet

	addq.l	#1,a1			; Skip a byte
	subq.l	#1,d7			; One less byte
	bpl.b	do_packets

	IFNE	DEBUG_DEMUX
	movem.l	d0-d7/a0-a6,-(sp)
	pea	msg5(pc)
	jsr	_kprintf
	addq.l	#4,sp
	movem.l	(sp)+,d0-d7/a0-a6
	ENDC

	bra	toss_it			; Return it.

	; Ugh....some other stream...

do_other_packet:
	clr.l	d0
	move.w	4(a1),d0  		;packet length
	add.w	#6,d0			;...plus id and length field
	adda.w	d0,a1			; = where to start looking again
	sub.l	d0,d7			;decrement byte counter.
	bne.b	do_packets

	IFNE	DEBUG_DEMUX
	movem.l	d0-d7/a0-a6,-(sp)
	pea	msg5(pc)
	jsr	_kprintf
	addq.l	#4,sp
	movem.l	(sp)+,d0-d7/a0-a6
	ENDC

	bra.b	toss_it

	; This is easy...the L64111 can grok the entire thing for us...

parse_audio_packet:
	adda.w	4(a1),a1	;Add in packet length
	move.l	a1,d0		;Into data register
	addq.l	#6,d0		;Plus id and length field
	sub.l	IO_DATA(a2),d0	;Start of Pack
	move.l	d0,iomr_DataSize(a2)

	IFNE	DEBUG_DEMUX
	movem.l	d0-d7/a0-a6,-(sp)
	move.l	d0,-(sp)
	pea	msg7(pc)
	jsr	_kprintf
	addq.l	#8,sp
	movem.l	(sp)+,d0-d7/a0-a6
	ENDC

	move.l	a6,-(sp)
	movea.l	md_SysBase(a6),a6
	move.l	IO_DATA(a2),iomr_DataStart(a2)
;	move.l	IO_LENGTH(a2),iomr_DataSize(a2)
;	move.l	#2324,iomr_DataSize(a2)
	lea.l	mu_AudioStream(a4),a0	; List
	movea.l	a2,a1			; Node
	jsrlib	AddTail
	add.w	#1,mu_APackets(a4)
	movea.l	(sp)+,a6

	IFNE	DEBUG_DEMUX
	movem.l	d0-d7/a0-a6,-(sp)
	pea	msg1(pc)
	jsr	_kprintf
	addq.l	#4,sp
	movem.l	(sp)+,d0-d7/a0-a6
	ENDC

	movem.l	(sp)+,d2-d7
	rts

	; Okay, this can be a little bit gross...

parse_video_packet:
	bclr	#7,iomr_MPEGFlags(a2)
	clr.l	d3
	move.w	4(a1),d3		; get packet length
	lea.l	6(a1),a1		; skip id and stuff

	; Stuffing or no header stuff?

0$	move.b	(a1)+,d0
 	subq.l	#1,d3			; one less data byte...
	cmp.b	#$0f,d0
	beq.b	video_done
	cmp.b	#$ff,d0
	bne.b	1$

	bra.b	0$

	; STD buffer scale?

1$	move.b	d0,d1
	and.b	#$C0,d1
	cmp.b	#$40,d1
	bne.b	2$
	addq.l	#1,a1			;Skip second byte
;	move.b	(a1)+,d0
	subq.l	#1,d3
	bra	0$

	; PTS?

2$	move.b	d0,d1
	and.b	#$f0,d1
	cmp.b	#$20,d1			;PTS only?
	beq.b	do_pts
	cmp.b	#$30,d1
	beq.b	do_pts

	;Gack! Something is wrong... we should never get here. Abort!

toss_it:
	IFNE	DEBUG_DEMUX
	movem.l	d0-d7/a0-a6,-(sp)
	pea	msg3(pc)
	jsr	_kprintf
	addq.l	#4,sp
	movem.l	(sp)+,d0-d7/a0-a6
	ENDC
	movea.l	a2,a1
	bsr	_TermIO

	movem.l	(sp)+,d2-d7		;Save everything for now...
	rts

do_pts:
	bset	#7,iomr_MPEGFlags(a2)
	lsr.l	#1,d0
	and.w	#$7,d0
	move.w	d0,iomr_PTSHigh(a2)	; Store PTS Hi

	move.w	(a1)+,d0
	lsr.w	#1,d0
	move.w	d0,iomr_PTSMid(a2)	; Mid

	move.w	(a1)+,d0
	lsr.w	#1,d0
	move.w	d0,iomr_PTSLow(a2)	; Low

	subq.l	#4,d3			; 4 less bytes
	cmp.b	#$30,d1			; DTS here too?
	bne.b	video_done

	addq.l	#5,a1			; skip ahead 5 bytes
	subq.l	#5,d3			; 5 less bytes of data

video_done:
	move.l	a1,iomr_DataStart(a2)
	move.l	d3,iomr_DataSize(a2)

	; Queue this up for the main i/o task

	move.l	a6,-(sp)
	movea.l	md_SysBase(a6),a6
	lea.l	mu_VideoStream(a4),a0
	movea.l	a2,a1
	jsrlib	AddTail
	add.w	#1,mu_VPackets(a4)
	movea.l	(sp)+,a6

	IFNE	DEBUG_DEMUX
	movem.l	d0-d7/a0-a6,-(sp)
	pea	msg2(pc)
	jsr	_kprintf
	addq.l	#4,sp
	movem.l	(sp)+,d0-d7/a0-a6
	ENDC

	movem.l	(sp)+,d2-d7		;Save everything for now...
	rts

msg1	dc.b	'Demux: Audio',10,0
msg2	dc.b	'Demux: Video',10,0
msg3	dc.b	't',0
msg4	dc.b	'Demux: No pack start',10,0
msg5	dc.b	'Demux: No stream start code',10,0
msg6	dc.b	'Demux Length: %ld',10,0
msg7	dc.b	'A: %ld',10,0
	end
