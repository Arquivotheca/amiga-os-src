;
; void DoSystemDemux(struct StreadFile *stream, struct DataPacket *dp)
;			A0				A0
;
; Parses "Does The Right Thing" with an MPEG system stream.
;
;
; First, some usefule includes"

	INCLUDE	"exec/types.i"
	INCLUDE "exec/ports.i"

; Next, some useful defines:

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

; Next, some useful structures:

 STRUCTURE StreamFile,0
 	APTR	sf_File
 	ULONG	sf_BlockSize
 	ULONG	sf_Err
 	APTR	sf_Proc
 	APTR	sf_VideoPort
 	APTR	sf_AudioPort
 	APTR	sf_ReplyPort
 	UWORD	sf_RingCnt
 	UWORD	sf_RingMax
 	UWORD	sf_VideoOffset
 	UWORD	sf_AudioOffset
 	APTR	sf_VideoData
 	APTR	sf_AudioData
 	UWORD	sf_VideoSize
 	UWORD	sf_AudioSize
 	UWORD	sf_A
 	UWORD	sf_V
 	UBYTE	sf_IsOverFlow
 	UBYTE	sf_OverFlow
 	LABEL	SF_SIZEOF

 STRUCTURE DataPacket,MN_SIZE
 	UWORD	dp_Pad
 	ULONG	dp_DataSize
 	ULONG	dp_Flags
 	UWORD	dp_SCRHi
 	UWORD	dp_SCRMid
 	UWORD	dp_SCRLo
 	UWORD	dp_PTSHi
 	UWORD	dp_PTSMid
 	UWORD	dp_PTSLo
 	APTR	dp_DataStart
 	ULONG	dp_DataLength
 	UBYTE	dp_IsOverFlow
 	UBYTE	dp_OverFlow
 	STRUCT	dp_Packet,CD_SECTOR_SIZE
 	LABEL	DP_SIZEOF

; More misc junk...

	XDEF	@DoSystemDemux
	XREF	@PrintBadFrame
	XREF	@PrintVideoFrame
	XREF	@PrintAudioFrame

jsrlib	MACRO
	IFND	_LVO\1
	XREF	_LVO\1
	ENDC
	jsr	_LVO\1(a6)
	ENDM

; And *finally*, the majik code:

@DoSystemDemux:
	movem.l	d2-d7/a2-a6,-(sp)	;Save everything for now...
	movea.l	4,a6
	movea.l	a1,a4			;Move it...
	movea.l	a0,a3			;Move it...
	lea.l	dp_Packet(a1),a5
	move.l	a5,d7
	add.l	#CD_SECTOR_SIZE,d7	;So we know when to quit looking
        clr.l	dp_Flags(a1)

	; Look for a pack start code....usually will be first thing in the pack

find_pack_code:
	cmp.l	#PACK_START_CODE,(a5)	;Pack start code?
	beq.b	parse_pack
	bra	toss_it
	addq.l	#1,a5
	cmp.l	a5,d7
	bne.b	find_pack_code
	bra	toss_it

	; We have a pack header, grab the System Clock Reference
	;
	; Note: Do we really want this?
	;

parse_pack:
	clr.l	d0
	move.b	4(a5),d0
	lsr.l	#1,d0
	and.b	#$7,d0
	move.w	d0,dp_SCRHi(a1)		; Store SCR Hi

	move.w	5(a5),d0
	lsr.l	#1,d0
	move.w	d0,dp_SCRMid(a1)	; Mid

	move.w	7(a5),d0
	lsr.l	#1,d0
	move.w	d0,dp_SCRLo(a1)		; Low

	; skip the mux rate stuff...

	adda.l	#12,a5

	; Look for a possible system stream header...

;	cmp.l	#SYSTEM_START_CODE,(a5) ; System Header Start code?
;	bne.b	do_packets		; nope...no sys header here...

	; We have a system header... just figure out the length so we can
	; skip over the stupid thing....

;	move.w	4(a5),d0		; Get length...
;	addq.l	#6,d0			; Add start code and length...
;	adda.l	d0,a5

do_packets:
	move.l	(a5),d0			; Start code?
	move.l	d0,d1
	and.l	#VIDEO_START_MASK,d0  	; Do video first....
	cmp.l	#VIDEO_START_PREFIX,d0	; video packet?
	beq.b	parse_video_packet	; parse it...

	cmp.l	#OTHER_START_PREFIX,d0  ; Something else?
	beq.b	do_other_packet

	and.b	#$C0,d0			; everything else is already cleared...
	cmp.l	#AUDIO_START_PREFIX,d0	; audio packet?
	beq.b	parse_audio_packet

	addq.l	#1,a5
	cmp.l	a5,d7
	bne.b	do_packets
	bra.b	toss_it

	; Ugh....some other stream...

do_other_packet:
;	move.w	#$f00,$dff180
	move.w	4(a5),d0  		;packet length
	add.w	#6,d0			;...plus id and length field
	adda.w	d0,a5			; = where to start looking again
	bra.b	do_packets

	; This is easy...the L64111 can grok the entire thing for us...

parse_audio_packet:
	lea.l	dp_Packet(a1),a2
	move.l	a2,dp_DataStart(a1)
	move.l	#CD_SECTOR_SIZE,dp_DataLength(a1)

	; Queue this up for the main i/o task

	movem.l	(sp)+,d2-d7/a2-a6	;Save everything for now...
	movea.l	a1,a0
	bra	@PrintAudioFrame
	rts

	movea.l	sf_AudioPort(a0),a0
;	jsrlib	PutMsg
	movem.l	(sp)+,d2-d7/a2-a6	;Save everything for now...
	rts

	; Okay, this can be a little bit gross...

parse_video_packet:
	clr.l	d3
	move.w	4(a5),d3		; get packet length
	lea.l	6(a5),a5		; skip id and stuff

	; Stuffing or no header stuff?

0$	move.b	(a5)+,d0
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
	addq.l	#1,a5			;Skip second byte
	move.b	(a5)+,d0

	; PTS?
2$	move.b	d0,d1
	and.b	#$f0,d1
	cmp.b	#$20,d1			;PTS only?
	beq.b	do_pts
	cmp.b	#$30,d1
	beq.b	do_pts

	;Gack! Something is wrong... we should never get here. Abort!
toss_it:
	movem.l	(sp)+,d2-d7/a2-a6	;Save everything for now...
	movea.l	a1,a0
	bra	@PrintBadFrame

	move.w	#$ff0,$dff180
;	jsrlib	ReplyMsg		;This will place it back on our read queue
	movem.l	(sp)+,d2-d7/a2-a6	;Save everything for now...
	rts

do_pts:
	move.l	#DPFLAGF_VALID_PTS,dp_Flags(a1)
	lsr.l	#1,d0
	and.b	#$7,d0
	move.w	d0,dp_PTSHi(a1)		; Store PTS Hi

	move.w	(a5)+,d0
	lsr.l	#1,d0
	move.w	d0,dp_PTSMid(a1)	; Mid

	move.w	(a5)+,d0
	lsr.l	#1,d0
	move.w	d0,dp_PTSLo(a1)		; Low

	subq.l	#4,d3			; 4 less bytes
	cmp.b	#$30,d1			; DTS here too?
	bne.b	video_done

	addq.l	#5,a5			; skip ahead 5 bytes
	subq.l	#5,d3			; 5 less bytes of data

video_done:
	move.l	a5,dp_DataStart(a1)
	move.l	d3,dp_DataLength(a1)

	; Queue this up for the main i/o task

	movem.l	(sp)+,d2-d7/a2-a6	;Save everything for now...
	movea.l	a1,a0
	bra	@PrintVideoFrame


	movea.l	sf_VideoPort(a0),a0
;	jsrlib	PutMsg
	movem.l	(sp)+,d2-d7/a2-a6	;Save everything for now...
	rts
