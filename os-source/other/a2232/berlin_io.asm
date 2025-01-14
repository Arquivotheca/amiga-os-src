**
**	$Id: berlin_io.asm,v 1.3 91/08/10 22:29:27 bryce Exp $
**
**	Generic device driver:	Device-specifc BeginIO/AbortIO
**
**	(C) Copyright 1989,1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

**
**  ASSUMPTIONS:
**	IORequests on the txlist/rxlist must always be kept self-consistent
**	so AbortIO will work.
**

	SECTION main,CODE

;---------- Includes --------------------------------------------------------
	IFND	PREASSEMBLED_INCLUDES
	NOLIST
	INCLUDE "exec/types.i"
	INCLUDE "exec/io.i"
	INCLUDE "exec/errors.i"
	INCLUDE "exec/ables.i"
	INCLUDE "exec/lists.i"
	INCLUDE "devices/serial.i"
	LIST
	ENDC

	INT_ABLES

	INCLUDE "macros.i"
	INCLUDE "berlin.i"

;---------- Exports ---------------------------------------------------------
	XDEF	_MD_BeginIO
	XDEF	_MD_AbortIO
	XDEF	InterruptNameText
	XDEF	PORTS_IRQ

;---------- Domestic imports ------------------------------------------------
;---------- Foreign imports -------------------------------------------------




*****i* multidev/AbortIO ****************************************************
*
*  FUNCTION
*	This is a request to abort or "finish early" the specified
*	IORequest.  This may or may not be possible or safe.
*
*  ASSUMPTIONS:
*	IORequests on the rxlist must always be kept self-consistent
*	so AbortIO will work.
*
* INPUTS
*	a1 - pointer to the ioRequest block to be aborted
*	a6 - pointer to this device
*
* RESULTS
*	d0 - Zero if the request was aborted, error code otherwise
*****************************************************************************
*
*	    _MD_AbortIO
*	      /     \
*   ai_NotActive    ai_AbortIt
*
*

_MD_AbortIO:
	    PRINTF  10,<'AbortIO-$%lx command/flags/error $%lx'>,a1,IO_COMMAND(a1)

	    move.l  a6,-(sp)
	    move.l  IO_UNIT(a1),a0
	    move.l  mdu_SysBase(a0),a6
	    lea.l   mdu_SS(a0),a0
	    JSRLIB  ObtainSemaphore	;registers preserved
		moveq	#-1,d1		;flag "not aborted"
		STOPIRQ
		  btst.b  #IOSERB_QUEUED,IO_FLAGS(a1)
		  beq.s   ai_NotActive
		  cmp.b   #NT_MESSAGE,LN_TYPE(a1)
		  bne.s   ai_NotActive

		  moveq   #0,d1 	    ;flag "aborted"
		  clr.b   IO_FLAGS(a1)      ;Clear processing while protected

		  ;---- Remove from pending list, then reply.
		  ;---- Requests on the pending list always have updated
		  ;---- IO_ACTUAL and IO_ERROR fields.
		  REMOVE_A1 ;d0,a0 destroyed

ai_NotActive:	STARTIRQ
	    move.l  IO_UNIT(a1),a0
	    lea.l   mdu_SS(a0),a0
	    JSRLIB  ReleaseSemaphore	;registers preserved

	    PRINTF  550,<'A-Reply a1=$%lx IO_ACTUAL=%ld, Flag=%ld'>,a1,IO_ACTUAL(a1),d1
	    tst.l   d1
	    bne.s   ai_NoReply
	    JSRLIB  ReplyMsg
	    moveq   #0,d0	    ;flag "aborted"
ai_NoReply:

	    move.l  (sp)+,a6
	    rts 		    ;0=aborted.  -1=not aborted



*****i* multidev/BeginIO ****************************************************
*
*   FUNCTION
*	BeginIO is the starting point for all incomming IO.  This may be
*	queued up for the device's own task to deal with, or done "QUICKLY"
*
*	A1 - IORequest
*	A6 - DeviceBase
*
*****************************************************************************
*
*   REGISTER USE
*	a1 - IoReqest
*	a5 - DeviceBase
*	a6 - ExecBase
*
*   Code optimized for speed.	Could be faster/smaller if dispatch table
*   was word length, and if branch was moved.
*
*

		CNOP	0,4 ;Be nice to 68020's
_MD_BeginIO:	PUSHM	a2/a5/a6
		move.l	IO_UNIT(a1),a5
		move.l	mdu_SysBase(a5),a6  ;[a6-sysbase a5-unit a1=IOreqest]

		;
		;   Prepare IORequest.
		;
		;   NT_MESSAGE must be stuffed into LN_TYPE so AbortIO() and
		;   CheckIO() can determine if this request has finished yet.
		;
		move.b	#NT_MESSAGE,LN_TYPE(a1)
		move.l	IO_COMMAND(a1),d0   ;IO_COMMAND+IO_FLAGS+IO_ERROR
		andi.w	#IOF_QUICK<<8,d0    ;clear all but IOF_QUICK
		move.w	d0,IO_FLAGS(a1)     ;IO_FLAGS+IO_ERROR

		;
		;   Range check command
		;
		swap	d0		    ;get IO_COMMAND
		cmp.w	#HIGHEST_COMMAND,d0
		bls.s	bio_inrange
		moveq.l #0,d0		    ;Set command to invalid
bio_inrange:

		;
		;   Dispatch
		;
		PRINTF	301,<'!$%lx/$%lx '>,a1,d0

		lsl.w	#2,d0
		;[D0=table index]

		lea.l	mdu_SS(a5),a0
		JSRLIB	ObtainSemaphore     ;registers preserved
			;
			move.l	commandTable(PC,d0.w),a0

			;Call device command
			;   A6-sysbase	(could be made scratch)
			;   A5-unit	(preserve)
			;   A2-scratch
			;   A1-IORequest (preserve)
			;   A0-scratch
			;
			;   D0=IO_FLAGS.B
			;
			jsr	(a0)
			;
		lea.l	mdu_SS(a5),a0
		JSRLIB	ReleaseSemaphore    ;registers preserved

;---- Reply to IORequest if all done ----------------------------------------
;
;(Note-request may already have been replied by the interrupt)
;
	       ;move.b	IO_FLAGS(a1),d0     ;(already in D0)
		and.b	#IOSERF_QUEUED!IOF_QUICK,d0
		bne.s	noReplyYet
		PRINTF	490,<'B-Reply a1=$%lx'>,a1
		JSRLIB	ReplyMsg ;If quick, no reply. If queued, no reply YET
noReplyYet:	POPM
		rts


;-------------- dispatch table for BeginIO ---------------------------------
commandTable:	dc.l my_CMD_INVALID	;0  $00
		dc.l my_CMD_RESET	;1  $01
		dc.l my_CMD_READ	;2  $02
		dc.l my_CMD_WRITE	;3  $03
		dc.l my_CMD_UPDATE	;4  $04
		dc.l my_CMD_CLEAR	;5  $05
		dc.l my_CMD_STOP	;6  $06
		dc.l my_CMD_START	;7  $07
		dc.l my_CMD_FLUSH	;8  $08
		dc.l my_SDCMD_QUERY	;9  $09
		dc.l my_SDCMD_BREAK	;10 $0A
		dc.l my_SDCMD_SETPARAMS ;11 $0B
HIGHEST_COMMAND equ  11


*****************************************************************************
*****************************************************************************
*****************************************************************************



*****************************************************************************
*
*   Clear IO_ACTUAL
*	    (at all processing points IO_ACTUAL is kept valid)
*   Caclulate bytes in buffer
*	    (head-tail.  If negative then (bufsize+(head-tail)
*   compare with request
*	    complete request, if possible
*	    else hang on rxlist and let interrupt get it
*
*	my_CMD_READ
*	 /	 \
* cr_ENOUGH    cr_QUEUE_IT
*
*
*   NEW MODE:
*	If IO_LENGTH is zero (a read request for zero bytes), all available
*	bytes will be returned.  IO_OFFSET must contain the size of your
*	read buffer (the maximum number of characters to return in one swat).
*	IO_LENGTH will be trashed.
*
*****************************************************************************
*   [A5-Unit]
*   [A1-IORequest]
*    A2-Scratch
*
my_CMD_READ:	PRINTF	201,<'READ %ld bytes. '>,IO_LENGTH(a1)
		clr.l	IO_ACTUAL(a1)

		STOPIRQ

		    ;-- check if other read requests are pending --
		    ;-- (the interrupt might remove nodes, but not add them) --
		    lea.l   mdu_rcvr_LIST(a5),a0
		    TSTLIST a0
		    bne.s   cr_QUEUE_IT     ;other queued requests exist...


		    ;----Calculate total pending bytes in 6502's buffer
		    move.l  mdu_ControlArea(a5),a0
		    NUMBYTES a0,a5
		    ;[D0-num bytes in buffer]
		    PRINTF  201,<'[%ld bytes in buffer '>,d0


		    ;----Check for special read mode
		    move.l  IO_LENGTH(a1),d1
		    bne.s   cr_NotSpecial
			;
			;   They want everything we have.  Give 'em min of
			;   pending bytes or their read buffer size.
			;
			move.l	d0,d1		    ;# pending bytes to D1
			cmp.l	IO_OFFSET(a1),d1
			bls.s	cr_OK
			move.l	IO_OFFSET(a1),d1
cr_OK:			move.l	d1,IO_LENGTH(a1)
			;
			;
cr_NotSpecial:		;
		    ;[D0-num bytes in buffer]
		    ;[D1-IO_LENGTH]



		    ;---- Compare "num bytes" with "IO_LENGTH"
		    cmp.l   d1,d0		;Can we complete read NOW?
		    bhs.s   cr_ENOUGH		;Yes...


;----------------------------------------------------------------------------
;---- There are not enough pending bytes in the read buffer; queue it.
;----	  note: IO_LENGTH() is guaranteed non-zero.
;----------------------------------------------------------------------------
cr_QUEUE_IT:	    PRINTF  200,<'queue IO-$%lx (a5=$%lx)]'>,a1,a5
		    move.b  #IOSERF_QUEUED,IO_FLAGS(a1) ;Clear IOB_QUICK
		    lea.l   mdu_rcvr_LIST(a5),a0
		    ADDTAIL ;A0-list (destroyed) A1-node
		    move.b  IO_FLAGS(a1),d0
		STARTIRQ
		rts	;exit my_CMD_READ


;----------------------------------------------------------------------------
;---- Enough bytes are available in the buffer to satisfy the current
;---- request (note that IO_LENGTH might be zero)
;----------------------------------------------------------------------------
;	D1-IO_LENGTH
;	D0-bytes in buffer
;
cr_ENOUGH:	PRINTF	200,<'process %ld bytes]'>,d1
		STARTIRQ

		move.l	d2,-(sp)    ;PUSHM   d2
		move.l	d1,d2	    ;copy IO_LENGTH bytes
		bra.s	cr_loopin
cr_Loop:	bsr.s	cr_ONE_BYTE
cr_loopin:	dbra	d2,cr_Loop
		move.l	(sp)+,d2    ;POPM

		move.b	IO_FLAGS(a1),d0
		rts	;exit my_CMD_READ


;---------------------------------------------------------------------------
;---- Grab one byte							----
;---------------------------------------------------------------------------
;	A5-Unit
;	A2-scratch
;	A1-IORequest (preserve)
cr_ONE_BYTE:
		move.l	mdu_BoardBase(a5),a0    ;calculate where in 6502
		add.w	mdu_rcvr_tail(a5),a0    ;space to grab data
		;[A0-true index to rcvr_tail]	;	    "

		move.l	IO_DATA(a1),a2          ;index to user buffer
		add.l	IO_ACTUAL(a1),a2
		move.b	(a0)+,(a2)+             ;get data byte

		;update local and 6502 pointers
		move.w	a0,d0			;cache lower 16 bits
		cmp.w	mdu_rcvr_max(a5),d0     ;check if at buffer end (+1)
		bne.s	cr_notatend
		move.w	mdu_rcvr_min(a5),d0
cr_notatend:	move.w	d0,mdu_rcvr_tail(a5)    ;update our tail pointer
		SWAZZLE 			;D0=6502 pointer D1-destroyed
		move.l	mdu_ControlArea(a5),a2  ;update 6502's tail pointer
		move.w	d0,acia_rcvr_tail(a2)   ;tell 6502 about new TAIL

		addq.l	#1,IO_ACTUAL(a1)        ;save in user IORequest
		move.b	IO_FLAGS(a1),d0
		rts



*****************************************************************************
*
*   my_CMD_WRITE.  Handle all flavors of write packets.
*
*   If the hardware has free space, stuff as many characters as possible &
*   return with QUICK IO.  If other requests are pending, or the buffer is
*   filled, we queue the request.
*
*	Special cases of IO_LENGTH:
*		 0  - return
*		-1  - count bytes until NULL termination (trashes IO_LENGTH)
*		big - split request into parts
*
*****************************************************************************
*   A6-sysbase	(could be made scratch)
*   A5-unit	(preserve)
*   A2-scratch
*   A1-IOreqest (preserve)
*
my_CMD_WRITE:	PRINTF	201,<'Write %ld bytes IO-$%lx '>,IO_LENGTH(a1),a1
		IFNE	HANDSHAKE_KLUDGE
		bclr.b	#IOB_QUICK,IO_FLAGS(a1)
		ENDC

		;-- Clear number of bytes processed so far
		clr.l	IO_ACTUAL(a1)

		;:DEF: may need interrupt protection
		;-- check if other write requests are pending --
		lea.l	mdu_xmit_LIST(a5),a0
		TSTLIST a0
		bne.s	cw_DeferWrite	    ;other queued requests exist...

		;-- Process as much of IORequest as possible --
		BSR.S	cw_MANY_BYTES

		move.l	IO_LENGTH(a1),d0
		sub.l	IO_ACTUAL(a1),d0
		bne.s	cw_MorePending	    ;processing was not complete...
		move.b	IO_FLAGS(a1),d0
		rts


;--- Write requests that come here are queued up for the interrupt to
;--- examine.  IO_ACTUAL holds any partially completed counts.
cw_DeferWrite:
cw_MorePending: PRINTF	202,<'%ld unwritten'>,IO_ACTUAL(a1)
		move.b	#IOSERF_QUEUED,IO_FLAGS(a1) ;Clear IOB_QUICK

		move.b	mdu_ACIANumber(a5),d0   ;Precalc D0
		move.l	mdu_BoardBase(a5),a0    ;Precalc A0
		STOPIRQ
		    bset.b  d0,global_need_tbe_int(a0)  ;Need interrupt
		    lea.l   mdu_xmit_LIST(a5),a0
		    ADDTAIL ;A0-list (destroyed) A1-node D0-nuked
		    move.b  IO_FLAGS(a1),d0
		STARTIRQ
		rts	;exit my_CMD_WRITE


;---------------------------------------------------------------------------
;	General case for writes.  Copy either IO_LENGTH bytes, or fill
;	the curcular write buffer to the brim.	Account for processed bytes
;	in IO_ACTUAL.
;
;	:TODO: Check at end if last byte can be snuck in (512 byte writes
;	       get broken into 511 and 1 byte chunks)
;---------------------------------------------------------------------------
;D0-D1:scratch A0:sctatch A2:scratch
;		       [A5  ;Unit]
Dest		EQUR	A2  ;scratch
;		       [A1  ;IORequest]
Source		EQUR	A0
Counter 	EQUR	D1
Write_Free	EQUR	D0


cw_MANY_BYTES:
		;---- Calculate number of write buffer bytes (Write_Free)
		move.l	mdu_ControlArea(a5),a0
		NUMBYTESW   a0,a5
		subq.l	#1,Write_Free	;Think about it.
		PRINTF	400,<'%ld bytes free in write buffer'>,Write_Free
		;[D0-Number of available bytes in write buffer]

		;---- Set Counter to the maximum of IO_LENGTH or Write_Free.
		move.l	IO_LENGTH(a1),Counter
		bpl.s	cw_Normal

		;
		;   Process -1 length writes (we cheat and use bit 31)
		;
		move.l	IO_DATA(a1),Source
		add.l	IO_ACTUAL(a1),Source
		moveq	#0,Counter
cw_Strlen:	addq.l	#1,Counter
		tst.b	(Source)+
		bne.s	cw_Strlen
		PRINTF	500,<'string length of %ld'>,Counter
		move.l	Counter,IO_LENGTH(a1)   ;Trash IO_LENGTH!

		;
		;   Process normal writes
		;
cw_Normal:	sub.l	IO_ACTUAL(a1),Counter   ;bytes already processed
		cmp.l	Write_Free,Counter	;Calculate the maximum of
		bls.s	cw_OneShot		; the request or the
		move.l	Write_Free,Counter	; free bytes in buffer

cw_OneShot:	;---- Calculate Source,Dest and update IO_ACTUAL
		move.l	IO_DATA(a1),Source
		add.l	IO_ACTUAL(a1),Source
		add.l	Counter,IO_ACTUAL(a1)   ;# of bytes copied this pass
		move.l	mdu_BoardBase(a5),Dest  ;calculate where in 6502...
		add.w	mdu_xmit_head(a5),Dest  ;...space to put data

		;---- Copy the data, wrapping at end of buffer
		move.l	Dest,d0
		bra.s	cw_not_at_end2
cw_LoopA:	move.b	(Source)+,(Dest)+
		move.l	Dest,d0
		cmp.w	mdu_xmit_max(a5),d0
		bne.s	cw_not_at_end2
		move.w	mdu_xmit_min(a5),d0
		move.l	d0,Dest
cw_not_at_end2: dbra	Counter,cw_LoopA

		;[D0-68000 pointer]
		move.w	d0,mdu_xmit_head(a5)
		SWAZZLE
		move.l	mdu_ControlArea(a5),a0
		move.w	d0,acia_xmit_head(a0)   ;tell 6502 about new byte(s)
		rts				;(function)




*****************************************************************************
my_SDCMD_QUERY: ;
		;   Calculate total pending bytes in 6502's buffer
		;
		move.l	mdu_ControlArea(a5),a0
		NUMBYTES a0,a5
		move.l	d0,IO_ACTUAL(a1)
		PRINTF	202,'Query: %ld bytes ready.'>,d0,a1



		;
		;   Update status word - what a pain!
		;
		move.b	acia_status_channel(a0),d1
		move.b	d1,d0
		and.w	#%0000000001110000,d0	;extract CD|CTS|DSR, clear word
		lsr.b	#1,d0

		lsr.b	#1,d1			;TRANSMIT-XOFF to C
		bcc.s	10$
		bset.l	#11,d0
10$

		lsr.b	#2,d1			;RTS to C
		bcc.s	20$
		bset.l	#6,d0
20$

		lsr.b	#1,d1			;RECEIVE-XOFF to C
		bcc.s	30$
		bset.l	#12,d0
30$

		move.w	d0,IO_STATUS(a1)

		move.b	IO_FLAGS(a1),d0
		rts



*****************************************************************************
my_SDCMD_SETPARAMS:
		move.l	mdu_ControlArea(a5),a2
		BUSYWAIT acia_command_flag(a2)  ;ensure 6502 is clear FIRST!!



;-------- IO_CTRLCHAR	- set new Xon/Xoff characters
		move.w	IO_CTLCHAR(a1),acia_xon_character(a2)



;-------- IO_BAUD	- calculate baud
		move.b	#SerErr_BaudMismatch,IO_ERROR(a1)
		move.l	IO_BAUD(a1),d1
		moveq	#0,d0
		cmp.l	#115200,d1
		beq.s	sp_SpecialBaud
		PRINTF	200,<'Baud rate request %ld '>,d1
		tst.w	IO_BAUD(a1)
		bne	sp_BadBaud	;High word must be zero
		moveq.l #num_bauds-1,d0
		lea.l	baud_6551_end(pc),a0
sp_baudSearch:	cmp.w	-(a0),d1
		dbeq	d0,sp_baudSearch ;dbhs/dbcc for "BEST FIT" test
		bne	sp_BadBaud	;negatory, pilot (IO_ERROR set)
sp_SpecialBaud: or.w	#%00010000,d0	;set "baud clock" bit



		;[D0-acia_control]
;-------- IO_READLEN	- Read & Write bits per character must be
;-------- IO_WRITELEN	- identical.  Range: 5-8
		;---- calculate bits per character ----
		move.b	#SerErr_InvParam,IO_ERROR(a1)

		moveq	 #0,d1
		move.b	IO_WRITELEN(a1),d1
		cmp.b	IO_READLEN(a1),d1
		bne	sp_BadParam	;Read length must equal write length
		;-
		;-  00	- 8 bits
		;-  01	- 7 bits
		;-  10	- 6 bits
		;-  11	- 5 bits
		cmp.b	#8,d1
		bhi	sp_BadParam	;Error, length greater than 8...
		subq.b	#5,d1
		bcs	sp_BadParam	;Error, length less than 5
		not.b	d1		;generate bitfield in lower 2 bits
		and.b	#%00000011,d1
		asl.b	#5,d1
		or.b	d1,d0


		;[D0-acia_control]
;-------- IO_STOPBITS - Range 0-1
		move.b	IO_STOPBITS(a1),d1
		subq.b	#1,d1
		ble.s	sp_OneStopBit	;IO_STOPBITS <= 1
		bset.b	#7,d0
sp_OneStopBit:	move.b	d0,acia_control(a2) ;save baud+length+stop bits
		PRINTF	281,<'acia_control $%lx '>,d0



;-------- IO_SERFLAGS	- RAD_BOOGIE
		btst.b	#SERB_RAD_BOOGIE,IO_SERFLAGS(a1)
		beq.s	sp_NoRythm
		or.b	#SERF_XDISABLED,IO_SERFLAGS(a1) ;disabled Xon/Xoff
sp_NoRythm:

;-------- IO_SERFLAGS	- XDISABLED, 7WIRE
		moveq.l #FLOWF_7IN!FLOWF_7OUT,d0
		btst.b	#SERB_7WIRE,IO_SERFLAGS(a1)
		bne.s	sp_7shake

		moveq	#0,d0		    ;no shake
		btst.b	#SERB_XDISABLED,IO_SERFLAGS(a1)
		bne.s	sp_Nshake

		moveq.l #FLOWF_XIN!FLOWF_XOUT,d0
sp_Xshake:
sp_7shake:
sp_Nshake:	move.b	d0,acia_flow_control(a2)
		PRINTF	281,<'acia_flow_control $%lx '>,d0


;-------- IO_SERFLAGS	- PARTY_ON, PARTY_ODD
;-------- IO_EXTFLAGS	- MSPON, MARK
;[the nutty bit defintions are not my fault!!]
		move.b	#%000001011,d0	    ;Starting point - parity disabled
		btst.b	#SERB_RAD_BOOGIE,IO_SERFLAGS(a1)
		bne.s	sp_SetParity

		move.b	IO_SERFLAGS(a1),d1
		btst.b	#SERB_PARTY_ON,d1
		beq.s	sp_NoOddEven
		and.b	#%00011111,d0
		btst.b	#SERB_PARTY_ODD,d1
		bne.s	sp_SetOdd
		or.b	#%01100000,d0
sp_SetOdd:	or.b	#%00100000,d0
sp_NoOddEven:

		move.b	IO_EXTFLAGS+3(a1),d1
		btst.b	#SEXTB_MSPON,d1
		beq.s	sp_NoMarkSpace
		and.b	#%00011111,d0
		btst.b	#SEXTB_MARK,d1
		bne.s	sp_SetMark
		or.b	#%11100000,d0
sp_SetMark:	or.b	#%10100000,d0
sp_NoMarkSpace:

sp_SetParity:	move.b	d0,acia_command(a2)
		PRINTF	280,<'acia_command $%lx '>,d0

;-------- IO_SERFLAGS	- :DEF: EOFMODE,SHARED,QUEUEDBRK


;-------- IO_BRKTIME	- :DEF: calculate time
		move.b	#250,acia_break_length(a2)  ;250 ms break
;-------- IO_TERMARRAY	-
;-------- IO_RBUFLEN	- not adjustable


;-------- write control byte and ping 6502 ---------
		move.b	#$c0,acia_command_flag(a2)  ;ping 6502, "install"
		clr.b	IO_ERROR(a1)
sp_BadParam:
sp_BadBaud:
		move.b	IO_FLAGS(a1),d0
		rts


;We support these baud rates, 115200 buad as a special kludge
num_bauds	equ	16
baud_6551:	dc.w	0
		dc.w	50
		dc.w	75
		dc.w	110	;109.92
		dc.w	135	;134.58
		dc.w	150
		dc.w	300
		dc.w	600
		dc.w	1200
		dc.w	1800
		dc.w	2400
		dc.w	3600
		dc.w	4800
		dc.w	7200
		dc.w	9600
		dc.w	19200
baud_6551_end:


*****************************************************************************
*
*   Due to funny constrants put on by the serial.device, we only clear
*   the read buffer here!!
*
my_CMD_FLUSH:	;abort all IORequests. :DEF:
my_CMD_CLEAR:	;Data in the input [and output] buffer[s] will be discarded.
		move.l	mdu_ControlArea(a5),a2
		BUSYWAIT acia_command_flag(a2)  ;ensure 6502 is clear FIRST!!
		move.b	#$88,acia_command_flag(a2)  ;ping 6502, "flushr"
		move.b	IO_FLAGS(a1),d0
		rts



*****************************************************************************
my_CMD_START:	;stated start: should Xon our side, not reset buffers.
my_CMD_RESET:	;Same as CloseDevice()/OpenDevice(), restore defaults
		move.l	mdu_ControlArea(a5),a2
		BUSYWAIT acia_command_flag(a2)
		move.b	#$c0,acia_command_flag(a2)  ;"install"
*****************************************************************************
my_CMD_UPDATE:	;:DEF: Wait for all bits to dribble out the end of the wires
my_CMD_STOP:	;stated stop
		move.b	IO_FLAGS(a1),d0
		rts



*****************************************************************************
my_SDCMD_BREAK: ;
		;	IO_BRKTIME/(1000 * 18)
		;	min 1, max 255
		;
		move.l	mdu_ControlArea(a5),a2
		BUSYWAIT acia_command_flag(a2)  ;ensure 6502 is clear FIRST
		move.b	#250,acia_break_length(a2)  ;250 ms break
		move.b	#$81,acia_command_flag(a2)  ;"break"
		move.b	IO_FLAGS(a1),d0
		rts


*****************************************************************************
my_CMD_INVALID:
		move.b	#IOERR_NOCMD,IO_ERROR(a1)
		move.b	IO_FLAGS(a1),d0
		rts





*****************************************************************************
*****************************************************************************
*****************************************************************************
*****************************************************************************
*
*   This interrupt function is hooked up to the INT2* interrupt.  The 6502
*   causes an interrupt each time the interrupt counter times out.
*   Adjust MAGIC_INTERRUPT_FREQUENCY to change the timeout.  We could
*   dynamically adjust the magic number based on the load, but this has
*   not been implemented.
*
*   This IS_DATA pointer passed in references the first element of the
*   Unit structure array.  To access other unit numbers add an offset.
*
*   D0/D1,A0/A1,A5,A6 are scratch
*   A1 = IS_DATA
*   A5 points to us
*   A6 is garbage.
*

		CNOP	0,4 ;Be nice to 68020's

PORTS_IRQ:	BLINK
		move.l	mdu_BoardBase(a1),a0        ;From first Unit struct
		move.b	global_service_channel(a0),d0
		bne.s	irq_OurBoard
		rts	;exit with Z=true;don't terminate chain.


;----------------------------------------------------------------------------
irq_OurBoard:	PUSHM	a0/a2/d2
		or.b	global_service_misc(a0),d0
		move.b	d0,d2

		move.l	a1,a5
		move.l	mdu_SysBase(a5),a6  ;From first Unit structure


		lsr.b	#1,d2
		bcc.s	irq_Not0
		bsr.s	DoUnit
irq_Not0:	add.w	#mdu_SIZE,a5	;Point to next unit structure

		lsr.b	#1,d2
		bcc.s	irq_Not1
		bsr.s	DoUnit
irq_Not1:	add.w	#mdu_SIZE,a5	;Point to next unit structure

		lsr.b	#1,d2
		bcc.s	irq_Not2
		bsr.s	DoUnit
irq_Not2:	add.w	#mdu_SIZE,a5	;Point to next unit structure

		lsr.b	#1,d2
		bcc.s	irq_Not3
		bsr.s	DoUnit
irq_Not3:	add.w	#mdu_SIZE,a5	;Point to next unit structure

		lsr.b	#1,d2
		bcc.s	irq_Not4
		bsr.s	DoUnit
irq_Not4:	add.w	#mdu_SIZE,a5	;Point to next unit structure

		lsr.b	#1,d2
		bcc.s	irq_Not5
		bsr.s	DoUnit
irq_Not5:	add.w	#mdu_SIZE,a5	;Point to next unit structure

		lsr.b	#1,d2
		bcc.s	irq_Not6
		bsr.s	DoUnit
irq_Not6:      ;add.w	#mdu_SIZE,a5	;Point to next unit structure


irq_AllDone:	POPM
		PRINTF	950,<'-=Exit interrupt. BoardBase=$%lx=-'>,a0

		;
		;   Retrigger the interrupt countdown.
		;
		clr.w	global_service_channel(a0)  ;ack processed ints
		tst.w	global_ack_irq(a0)          ;ack 6502 interrupt (HW)
		move.b	global_interrupt_stash(a0),global_interrupt_freq(a0)
		moveq	#1,d0	;exit with Z=flase; terminate chain.
		rts



;----------------------------------------------------------------------------
;   Registers d0/d1/a0/a1/a2/a5/a6 used
;   Registers		     a5/a6 preserved
;   a6 - SysBase
;   a5 - Unit
;   a2 - Scratch
;
DoUnit: 	PRINTF	900,<'-=Interrupt! Unit %lx=-'>,a5
		PRINTF	899,<'~'>,a5

*****************************************************************************
******* Process pending reads ***********************************************
*****************************************************************************

		;-- check if any read requests are pending --
		move.l	mdu_rcvr_LIST+MLH_HEAD(a5),a1   ;[a1-first node]
		tst.l	LN_SUCC(a1)                     ;Test if end node
		beq.s	irq_NoReadPending		;If empty...


		;-- check if enough bytes are avilable to fill request --
		move.l	mdu_ControlArea(a5),a0
		NUMBYTES a0,a5
		PRINTF	391,<'Unit-$%lx Read IO-$%lx Requested-'>,a5,a1

		move.l	IO_LENGTH(a1),d1
		sub.l	IO_ACTUAL(a1),d1
		PRINTF	390,<'%ld ?= %ld avail'>,d1,d0
		cmp.l	d1,d0
		bhs.s	irq_FillNow	;Request can be %100 filled

		move.l	mdu_rcvr_thresh(a5),d1
		cmp.l	d1,d0
		blo.s  irq_NotEnough
		move.l	d0,d1		;Copy all bytes available now


		;------ fill out request ------------------------------------
irq_FillNow:	move.l	d2,-(sp)
		move.l	d1,d2		;# bytes to copy
		bra.s	irq_In
irq_CountDown:	jsr	cr_ONE_BYTE	;A1-preserved  A2-destroyed
irq_In: 	dbra	d2,irq_CountDown
		move.l	(sp)+,d2


		;------ all done yet? ---------------------------------------
		PRINTF	900,<'act=%ld len=%ld '>,IO_ACTUAL(a1),IO_LENGTH(a1)
		move.l	IO_ACTUAL(a1),d0
		cmp.l	IO_LENGTH(a1),d0
		bne.s	irq_NotEnough


		;------ return request --------------------------------------
		REMOVE_A1   ;d0,a0 destroyed
		clr.b	IO_FLAGS(a1)
		PRINTF	360,<'I-Reply %lx '>,a1
		JSRLIB	ReplyMsg

irq_NotEnough:
irq_NoReadPending:

*****************************************************************************
******* Process pending writes **********************************************
*****************************************************************************
*
*   A1-IORequest
		;-- check if any read requests are pending --
		move.l	mdu_xmit_LIST+MLH_HEAD(a5),a1   ;[a1-first node]
		tst.l	LN_SUCC(a1)                     ;Test if end node
		beq.s	irq_NoWritePending		;If empty...
		PRINTF	361,<'-Deferred write IO %lx. %ld,%ld length/actual-'>,a1,IO_LENGTH(a1),IO_ACTUAL(a1)

		;D0-D1:scratch A0:sctatch A2:scratch A5:unit
		BSR	cw_MANY_BYTES	    ;Process IORequest completely...
		PRINTF	361,<'-After processig IO %lx. %ld,%ld length/actual-'>,a1,IO_LENGTH(a1),IO_ACTUAL(a1)
		move.l	IO_LENGTH(a1),d0
		sub.l	IO_ACTUAL(a1),d0
		bne.s	irq_MoreWPending    ;processing was not complete...

		;------ return request --------------------------------------
		REMOVE_A1   ;d0,a0 destroyed

		;-- check if any more read requests are pending --
		move.l	mdu_xmit_LIST+MLH_HEAD(a5),a0   ;[a1-first node]
		tst.l	LN_SUCC(a0)                     ;Test if end node
		bne.s	irq_WritePending		;Non-emtpty...
		    move.l  mdu_BoardBase(a5),a0
		    move.b  mdu_ACIANumber(a5),d0
		    bclr.b  d0,global_need_tbe_int(a0)  ;Disable interrupt
irq_WritePending:

		clr.b	IO_FLAGS(a1)
		PRINTF	360,<'W-Reply %lx '>,a1
		JSRLIB	ReplyMsg
irq_MoreWPending:
irq_NoWritePending:

*****************************************************************************
*****************************************************************************
*****************************************************************************
		rts	;(exit)



InterruptNameText:
		dc.b	'A2232 multiserial card',0
		ds.w	0

		END
