**
**	$Id: buffer.asm,v 36.12 92/04/15 10:25:34 darren Exp $
**
**      buffer management routines
**
**      (C) Copyright 1989,1990 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"
	INCLUDE	"exec/ables.i"


**	Exports

	XDEF	GetReadIO
	XDEF	PutReadData
	XDEF	PutReadByte
	XDEF	PutReadSnip


**	Imports

	XLVO	FreeVec			; Exec
	XLVO	ObtainSemaphore		;
	XLVO	Permit			;
	XLVO	ReleaseSemaphore	;


**	Assumptions

	IFNE	cu_ReadLastOut-cu_ReadLastIn-2
	FAIL	"cu_ReadLastOut does not follow cu_ReadLastIn"
	ENDC
	IFNE	cu_ReadData-cu_ReadLastOut-2
	FAIL	"cu_ReadData does not follow cu_ReadLastOut"
	ENDC

**	Equates

aRAVLEN	EQU	4			;sequence is 4 long

	IFNE	aRAVLEN-4
	FAIL	"aRAV length not equal to 4; recode"
	ENDC


;	read buffer
;	    o   circular
;	
;	    o   typed based on top token n
;		1 <= n <= 255
;		    next n are characters
;		0 == n
;		    indirect to buffer
;		    long1   buffer length (decrementing)
;		    long2   buffer data (incrementing)
;		    long3   buffer snip address
;	
;	    >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;	     1c1			<- 1 character in buffer
;	     2cc2			<- 2 characters in buffer
;	     2cc20<l1><l2><l3>0		<- 2 characters + a snip
;	      1c10<l1><l2><l3>0		<- if we read 1, it would be like so
;	         0<l1><l2><l3>0		
;	         0<l1><l2><l3>01c1
;	         0<l1><l2><l3>02cc2
;	                       2cc2
;	    >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;

;------ GetReadIO ---------------------------------------------------
;
;	actual = GetReadIO(conUnit, ioReq), consoleBase
;	d0                 a2       a3      a6
;
;	fill the ioReq with data from the read buffer
;
;--------------------------------------------------------------------
;
;   d2	running ReadLastOut
;   d3	actual ReadLastOut to cache later
;   d4	decrementing destination length
;   a0  read data buffer base
;   a2	console unit
;   a3	io request
;   a4	incrementing destination data pointer
;   a6	console device
;
GetReadIO:
		movem.l	d2-d5/a4-a5,-(a7)
		move.l	cd_ExecLib(a6),a0
		FORBID	a0,NOFETCH

		movem.l	IO_LENGTH(a3),d4/a4	; and IO_DATA

		lea	cu_ReadLastOut(a2),a0
		move.w	(a0),d2
		cmp.w	-2(a0),d2
		beq.s	griBailOut

griLoop:        move.w	d2,d3
		;-- check for end conditions
		;--	check for remaining length
		tst.l	d4
		beq.s	griDone

		bsr	griNextD2	;.s!
		;--	check for remaining buffer contents
		beq.s	griDone

		;-- figure out what type of span this is
		moveq	#0,d5
		move.b	2(a0,d2.w),d5
		beq	griSnipSpan	;.s!

		;-- is aRAV sequence in buffer?

		cmpi.b	#aRAVLEN,d5		;is span long enough?
		blt.s	notaRAVLEN		;if not, forget it

		movem.l	d0-d3,-(sp)
		bsr	griFillD0

		cmp.l	#(($9b<<24)!('0'<<16)!(' '<<8)!('v')),d0
		bne.s	notaRAV
		bset	#IOB_SPECIAL,IO_FLAGS(a3)

notaRAV:
		movem.l	(sp)+,d0-d3
		
notaRAVLEN:
		;-- read from the char span <n><c1><c2>...<cn><n'>
		move.w	d5,d0		; guess whole span will fit
		sub.l	d5,d4
		bge.s	griSpanDBF
		add.w	d4,d0		; limit to remaining length
		clr.l	d4		;
		bra.s	griSpanDBF
griSpanLoop:
		move.w	d2,d3		; cache current index
		bsr.s	griNextD2
		move.b	2(a0,d2.w),(a4)+
		subq.w	#1,d5		; update remaining chars in span
griSpanDBF:
		dbf	d0,griSpanLoop

		;-- update span borders
		tst.b	d5
		beq.s	griFlushCSpan

		;--	part of span remains, only if no remaining destination
		move.b	d5,2(a0,d2.w)	; update span count <n>
		add.w	d5,d2
		addq.w	#1,d2
		cmp.w	#READBSIZE,d2
		blt.s	griUpdateBounded
		sub.w	#READBSIZE,d2
griUpdateBounded:
		move.b	d5,2(a0,d2.w)	; update span count <n'>
		bra.s	griDone

griFlushCSpan:
		;--	span is gone, flush <n'>
		bsr.s	griNextD2
		bne.s	griLoop
		move.w	d2,d3

griDone:
		move.w	d3,(a0)		; update ReadLastOut

griBailOut:
		move.l	cd_ExecLib(a6),a0
		PERMIT	a0,NOFETCH

		move.l	IO_LENGTH(a3),d0
		sub.l	d4,d0		; construct actual
		movem.l	(a7)+,d2-d5/a4-a5
		rts


griNextD2:
		;--	bump ReadLastOut
		addq.w	#1,d2		; bump to next location
		cmp.w	#READBSIZE,d2	; compare w/ read buffer size
		blt.s	griBoundedLastOut
		clr.w	d2
griBoundedLastOut:
		cmp.w	-2(a0),d2
		rts

		;-- read from snip span 0<l1><l2><l3>0
griSnipSpan:
		bsr.s	griFillD0
		move.l	d0,d5		; buffer length (decrementing)
		bsr.s	griFillD0
		move.l	d0,a1		; buffer data (incrementing)
		bsr.s	griFillD0
		move.l	d0,a5		; buffer snip address

		move.l	d5,d0
		sub.l	d5,d4
		bge.s	griSnipLoop
		add.l	d4,d0		; limit to remaining length
		clr.l	d4		;
griSnipLoop:
		move.b	(a1)+,d1
		cmp.b	#10,d1		; LF?
		bne.s	griPutSnipCh
		moveq	#13,d1		; LF -> CR
griPutSnipCh:
		move.b	d1,(a4)+
		subq.l	#1,d5
		subq.l	#1,d0
		bgt.s	griSnipLoop

griSnipDone:
		tst.l	d5
		beq.s	griFlushSnip

		;-- update <l1><l2>
		move.w	d3,d2
		bsr.s	griNextD2
		move.l	d5,d0
		bsr	prsFillD0
		move.l	a1,d0
		bsr	prsFillD0
		bra.s	griDone

griFlushSnip:
		;--	snip is gone, flush it
		;--	BUG fix here
		;--	Fixes corrupt read data
		;--	when doing io_Length > 1
		;--	and fixes read mem loc 0 bug
		;--	in revisons prior to 36.521

		subq.w	#1,snip_Access(a5)
		bpl	griFlushCSpan
		move.l	a5,a1
		LINKEXE	FreeVec
		lea	cu_ReadLastOut(a2),a0
		bra	griFlushCSpan


griFillD0:
		moveq	#0,d0
		moveq	#3,d1
grifLoop:
		bsr	griNextD2
		lsl.l	#8,d0
		move.b	2(a0,d2.w),d0
		dbf	d1,grifLoop
		rts


;------ PutReadData -------------------------------------------------
;
;	actual = PutReadData(len, data, conUnit), consoleBase
;	d0                   d0.w a0    a2        a6
;
;	Put bytes in the read buffer from the data buffer supplied
;	-- all or none.
;
;--------------------------------------------------------------------
PutReadData:
		movem.l	d2-d3/a3,-(a7)
		move.l	d0,d2
		beq.s	prdEmpty
		move.l	a0,a3
		move.l	cd_ExecLib(a6),a0
		FORBID	a0,NOFETCH

		;-- figure out if new border bytes needed for span
		lea	cu_ReadLastIn(a2),a0
		move.w	(a0)+,d1	; cu_ReadLastIn
		;--	check for empty buffer
		cmp.w	(a0),d1		; cu_ReadLastOut
		beq.s	prdNeedBorder
		;--	check for char span in progress
prdChkSpanning:
		tst.b	2(a0,d1.w)
		bne.s	prdChkSpace
prdNeedBorder:
		addq.w	#2,d0		; need border bytes

		;-- calculate number of bytes used in buffer already
prdChkSpace:
		sub.w	(a0),d1		; cu_ReadLastOut
		bpl.s	prdHaveUsed
		add.w	#READBSIZE,d1

		;-- see if there are enough for this data
prdHaveUsed:
		add.w	d0,d1
		cmp.w	#READBSIZE-1,d1
		bge.s	prdDone		; no, bail out
		move.w	d2,d3
		bra.s	prdDBF

prdLoop:
		move.b	(a3)+,d0
		bsr.s	PutReadByte
prdDBF:
		dbf	d3,prdLoop

prdDone:
		move.l	cd_ExecLib(a6),a0
		PERMIT	a0,NOFETCH
prdEmpty:
		move.l	d2,d0
		movem.l	(a7)+,d2-d3/a3
		rts


;------ PutReadByte -------------------------------------------------
;
;	PutReadByte(byte, conUnit), consoleBase
;	            d0    a2        a6
;
;	Put a byte in the read buffer
;
;--------------------------------------------------------------------
PutReadByte:
		movem.l	d2-d3,-(a7)
		move.l	cd_ExecLib(a6),a0
		FORBID	a0,NOFETCH
		;-- check for empty buffer
		lea	cu_ReadLastIn(a2),a0
		move.w	(a0)+,d2	; cu_ReadLastIn
		cmp.w	(a0),d2		; cu_ReadLastOut
		beq.s	prbNewSpan

		;-- check for char span in progress
		moveq	#0,d1
		move.b	2(a0,d2.w),d1
		beq.s	prbNewSpan	; no, a buffer preceeds this

		;-- add to the char span <n><c1><c2>...<cn><n'>
		move.w	d2,d3		; save this location old <n'>
		bsr.s	prbNextD2
		beq.s	prbDone		; full: don't add byte

		;--	update bytes at head
		move.b	d0,2(a0,d3.w)	; put new byte in old <n'>
		addq.w	#1,d1
		move.b	d1,2(a0,d2.w)	; put new count in new <n'>
		;--	calculate tail end of char span
		sub.w	d1,d3		; calculate <n>
		bpl.s	prbFoundN
		add.w	#READBSIZE,d3
prbFoundN:
		addq.b	#1,2(a0,d3.w)	; update <n>

prbUpdate:
		move.w	d2,-(a0)	; update cu_ReadLastIn

prbDone:
		move.l	cd_ExecLib(a6),a0
		PERMIT	a0,NOFETCH
		movem.l	(a7)+,d2-d3
		rts

		;-- start a new char span of size 1
prbNewSpan:
		bsr.s	prbNextD2
		beq.s	prbDone
		move.b	#1,2(a0,d2.w)
		bsr.s	prbNextD2
		beq.s	prbDone
		move.b	d0,2(a0,d2.w)
		bsr.s	prbNextD2
		beq.s	prbDone
		move.b	#1,2(a0,d2.w)
		bra.s	prbUpdate


prbNextD2:
		addq.w	#1,d2		; bump to next free location
		cmp.w	#READBSIZE,d2	; compare w/ read buffer size
		blt.s	prbBoundedLastIn
		clr.w	d2
prbBoundedLastIn:
		cmp.w	(a0),d2		; cu_ReadLastOut
		rts


;------ PutReadSnip -------------------------------------------------
;
;	success = PutReadSnip(), consoleBase
;	d0                       a6
;
;	Paste to the read buffer
;
;--------------------------------------------------------------------
PutReadSnip:
		movem.l	d2-d3,-(a7)
		lea	cd_SelectionSemaphore(a6),a0
		LINKEXE	ObtainSemaphore
		move.l	cd_ExecLib(a6),a0
		FORBID	a0,NOFETCH

		;-- test if any hook(s) have been installed
		;-- if so, the clipboard.device is suppose
		;-- to be used, meaning that we disable
		;-- the internal pasting capability.

		tst.b	cd_Hooks(a6)
		beq.s	pastesnip

		;-- Put private control sequence into read
		;-- buffer <CSI>0 v means user pressed
		;-- RIGHT AMIGA V (paste text).
		;
		;-- <CSI>[parameter 0-9] v where future
		;-- numbers are reserved.  Possible
		;-- uses - e.g., <CSI>1 v meaning paste
		;-- graphics, overstrike paste, etc.


		moveq	#04,d0		;length
		lea	aRAV(pc),a0
		bsr	PutReadData
		move.l	d0,d3
		bra.s	prsDone

pastesnip:
		;-- check for appropriate snip buffer
		move.l	cd_SelectionSnip(a6),d3
		beq.s	prsDone
		move.l	d3,a1
		moveq	#0,d3
		move.w	snip_Length(a1),d3
		beq.s	prsDone

		;-- put snip record in read buffer
		lea	cu_ReadLastIn(a2),a0
		move.w	(a0)+,d2	; cu_ReadLastIn
		bsr.s	prbNextD2
		beq.s	prsFull		; full: don't add byte
		move.b	#0,2(a0,d2.w)
		move.l	d3,d0
		bsr.s	prsFillD0
		move.l	a1,d0
		addq.l	#snip_Data,d0
		bsr.s	prsFillD0
		move.l	a1,d0
		bsr.s	prsFillD0
		bsr.s	prbNextD2
		beq.s	prsFull		; full: don't add byte
		move.b	#0,2(a0,d2.w)

		move.w	d2,-(a0)	; update cu_ReadLastIn
		addq.w	#1,snip_Access(a1) ; show snip in use

prsDone:
		move.l	cd_ExecLib(a6),a0
		PERMIT	a0,NOFETCH
		lea	cd_SelectionSemaphore(a6),a0
		LINKEXE	ReleaseSemaphore
		move.l	d3,d0
		movem.l	(a7)+,d2-d3
		rts

prsPopFull:
		addq.l	#4,a7
prsFull:
		moveq	#0,d3
		bra.s	prsDone

prsFillD0:
		moveq	#3,d1
prsfLoop:
		bsr	prbNextD2
		beq.s	prsPopFull
		rol.l	#8,d0
		move.b	d0,2(a0,d2.w)
		dbf	d1,prsfLoop
		rts

		;-- RIGHT AMIGA V sequence
		;-- parameter 0 == text snip

aRAV:		dc.b	$9b,$30,$20,$76		;<CSI>0 v

	END
