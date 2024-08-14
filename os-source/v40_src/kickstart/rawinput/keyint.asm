	TTL	'$Id: keyint.asm,v 36.11 90/12/14 15:28:17 darren Exp $
**********************************************************************
*
*			---------------
*   keyint.asm		KEYBOARD DEVICE		keyboard interrupt code
*			---------------
*
*   Copyright 1985, 1987 Commodore-Amiga Inc.
*
*   Source Control	$Locker:  $
*
*   $Log:	keyint.asm,v $
*   Revision 36.11  90/12/14  15:28:17  darren
*   *** empty log message ***
*   
*   Revision 36.10  90/04/13  12:44:07  kodiak
*   use Id instead of Header for 4.x rcs
*   
*   Revision 36.9  90/04/02  13:00:42  kodiak
*   for rcs 4.x header change
*   
*   Revision 36.8  89/08/31  18:24:56  kodiak
*   cmp, not sub, in KeyHandshakeEnd
*   
*   Revision 36.7  89/08/31  17:27:28  kodiak
*   perform key handshake timeout w/ timer ReadEClock function
*   
*   Revision 36.6  89/07/31  17:14:39  kodiak
*   remove timer cia-a-a use in preparation for timer device use of it
*   
*   Revision 36.5  89/03/25  14:08:48  kodiak
*   use timer a to ensure 75us handshake
*   make handshake code public for keydev's use
*   
*   Revision 36.4  89/03/16  15:57:59  kodiak
*   use timer a, not b.
*   
*   Revision 36.3  89/02/20  16:11:33  kodiak
*   wait for 75us
*   
*   Revision 36.2  88/11/03  12:35:34  kodiak
*   use AUTOINIT in ResidentTag
*   
*   Revision 35.3  88/08/02  12:21:38  kodiak
*   first cut at timer-based handshake
*   
*   Revision 35.2  88/01/25  11:36:43  kodiak
*   remove unnecessary Disable Enable pair
*   
*   Revision 35.1  87/11/12  13:36:24  kodiak
*   make a500/a2000 numeric ()*/+ keys have NUMERICPAD qualifier
*   
*   Revision 35.0  87/10/26  11:32:20  kodiak
*   initial from V34, but w/ stripped log
*   
**********************************************************************

	SECTION		rawinput

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/devices.i"
	INCLUDE		"exec/tasks.i"
	INCLUDE		"exec/io.i"
	INCLUDE		"exec/strings.i"

	INCLUDE		"hardware/cia.i"
	INCLUDE		"devices/timer.i"
	INCLUDE		"devices/inputevent.i"

	INCLUDE		"macros.i"
	INCLUDE		"kbdata.i"


*------ Imported Globals ---------------------------------------------

	XREF		_ciaa
	XREF		_ciaacra
	XREF		_ciaasdr

*------ Imported Functions -------------------------------------------

	XREF_EXE	Cause

	XREF		_LVOReadEClock

	XREF		KDReadEvent


*------ Exported Functions -------------------------------------------

	XDEF		KDInterrupt

	XDEF		KeyHandshakeBegin
	XDEF		KeyHandshakeEnd


*------ keyboard.device/KDInterrupt ----------------------------------
*
*   NAME
*	KDInterrupt - 8520 serial interrupt routine.
*
*   SYNOPSIS
*	continue = KDInterrupt(KeyboardDev)
*	CC-zero-bit	       a1
*
*   FUNCTION
*	KDInterrupt gets a key code from the 8520, updates the key
*	matrix, and either buffers it or satisfies I/O with it.
*	It is initiated in response to an interrupt by 8520-B control
*	pair B shift completion.
*
*   INPUTS
*	KeyboardDev - the Keyboard device pointer.
*
*   RETURNS
*	CC-zero if interrupt not handled by this routine.
*	CC-not-zero if interrupt handled here.
*
*---------------------------------------------------------------------
*
*   IMPLEMENTATION NOTES
*
*	The rules of 8520 (mother board) to 6500/01 (keyboard)
*	communication are:
*
*	After power-on --
*	    6500/01:test processor, test ram, checksum rom.
*		    check for key switches pressed (possible stuck
*			keys).
*		    scan keyswitches.		   \
*		    wait for 8520 data handshake.  /
*		    send any errors and keys pressed (w/ handshaking).
*	    (Reset starts here: reset key combination is
*		CTL-ALT-AMIGA)
*		    send "start" token (w/ handshaking).
*
*		    scan keyswitches: wait for transition.
*		    send key token.
*		    if handshake not received in 1. seconds,
*			clock zero data bits @ 0.25s, waiting for
*			handshake, then retransmit character.
*
*	    8520:   reset operating system ... configure keyboard
*			driver.
*		 /> handshake data (i.e. set low then high then serial
*		 |	input).
*		 \_ wait for token.
*
*
*
*---------------------------------------------------------------------
KeyHandshakeBegin:
;
;  the key handshake is spec'd to take at least 75usec.
;  ciaata counts ticks at 715909 Hz.
;  So 54 ticks guarantees 75usec.
;
	    ;-- drive data line low, start one-shot timer to
	    ;	initiate the low signal of the handshake and start
		or.b	#CIACRAF_SPMODE,_ciaacra
		lea	kd_EClock1(a6),a0
		move.l	a6,-(a7)
		move.l	kd_Tick+IO_DEVICE(a6),a6
		jsr	_LVOReadEClock(a6)
		move.l	(a7)+,a6
		moveq	#54,d0
		add.l	d0,kd_EClock1+EV_LO(a6)
		bcc.s	khbDone
		addq.l	#1,kd_EClock1+EV_HI(a6)
khbDone:
		rts


KDInterrupt:
		movem.l d2-d4/a6,-(a7)
		move.l	a1,a6			; get keyboard device
		moveq	#0,d2			; clear upper byte
		move.b	_ciaasdr,d2		; get the key.

		;-- start the keyboard handshake
		bsr.s	KeyHandshakeBegin

	    ;-- this handshake is completed at the end of this routine

		moveq	#0,d4		    ;clear qualifier word
*	    ;-- save the raw keystroke
		not.b	d2		    ;signal is active low
		ror.b	#1,d2		    ;move up/down to sign bit

	    ;-- check for second reset
		cmpi.b	#RESET_CODE,d2
		bne.s	notReset
		bset	#KDB_RESET1,kd_Flags(a6)
		beq.s	processKey
		bset	#KDB_RESET2,kd_Flags(a6)
		bne	intRts

*	    ;-- cause software interrupts for all handlers and exit
		move.l	kd_HandlerList(a6),a1
		tst.l	(a1)
		beq	handshake		; no handlers
causeLoop:
		move.l	(a1),d3
		beq	intRts
		addq.w	#1,kd_OutstandingResetHandlers(a6)
		LINKEXE Cause
		move.l	d3,a1
		bra.s	causeLoop

notReset:
		bclr	#KDB_RESET1,kd_Flags(a6)
processKey:
		move.w	d2,d1		    ;get keycode index into
		and.w	#$0007F,d1	    ;  keycode matrix
		cmp	#HIGH_KEYCODE,d1    ;check that this is in the
		bhi	enqueueKey	    ;  range of valid keys
		move	d1,d0		    ;get byte and bit for use in
		and.w	#$00007,d0	    ;  setting or clearing matrix
		lsr	#3,d1		    ;  bit
		lea	kd_Matrix(a6),a0    ;get the matrix base location
		btst	#7,d2		    ;check key up/down flag
		bne.s	keyUp
		bset	d0,0(a0,d1.w)	    ;show key down
		cmp.w	#$60>>3,d1	    ;check if shift key
		bne.s	checkNumeric	    ;
		bset	d0,kd_Shifts(a6)    ;  update shift keys
		bra.s	checkNumeric
keyUp:
		bclr	d0,0(a0,d1.w)	    ;show key up
		cmp.w	#$60>>3,d1	    ;check if shift key
		bne.s	checkNumeric	    ;
		bclr	d0,kd_Shifts(a6)    ;  update shift keys
		bra.s	checkNumeric	    ;


numericMatrix:
		dc.b	%00000000,%10000000,%00000000,%11100000
		dc.b	%00000000,%11100000,%00000000,%11110000
		dc.b	%00001000,%00000100,%00000000,%01111100
		dc.b	%00000000,%00000000,%00000000,%00000000

checkNumeric:
*	    ;-- check if numeric
		move.w	d2,d1
		and.w	#$0007F,d1
		move	d1,d0		    ;get byte and bit for use in
		and.w	#$00007,d0	    ;  testing numeric table
		lsr	#3,d1		    ;
		lea	numericMatrix(pc),a0	;get the matrix base location
		btst	d0,0(a0,d1.w)	    ;check if numeric
		beq.s	enqueueKey
		or.w	#IEQUALIFIER_NUMERICPAD,d4

enqueueKey:
*	    ;-- enqueue the key
		move.w	kd_BufTail(a6),d1
		move.w	d1,d0
		addq.w	#4,d0
		andi.w	#(KBBUFSIZE-1),d0
		cmp.w	kd_BufHead(a6),d0
		bne.s	buffOk

*	    ;------ keyboard overflow!
		move.w	d1,d0		    ; overwrite the last key
		move.w	#OVERFLOW_CODE,d2   ;	with overflow code
		subq.w	#4,d1
		andi.w	#(KBBUFSIZE-1),d1

buffOk:
		move.w	d2,kd_BufQueue(a6,d1.w)
		or.b	kd_Shifts(a6),d4
		move.w	d4,kd_BufQueue+2(a6,d1.w)
		move.w	d0,kd_BufTail(a6)

		;------ don't check to satisfy I/O if keyboard stopped
		btst	#DUB_STOPPED,kd_Unit+du_Flags(a6)
		bne.s	handshake

	    ;-- if I/O is already pending, see if this will satisfy it
		move.l	kd_Unit+MP_MSGLIST(a6),a1
		tst.l	(a1)
		beq.s	handshake

		bsr	KDReadEvent

handshake:
		;-- complete the high signal of the handshake
		bsr.s	KeyHandshakeEnd

intRts:
		movem.l (a7)+,d2-d4/a6
		rts



KeyHandshakeEnd:
		lea	kd_EClock2(a6),a0
		move.l	a6,-(a7)
		move.l	kd_Tick+IO_DEVICE(a6),a6
		jsr	_LVOReadEClock(a6)
		move.l	(a7)+,a6
		move.l	kd_EClock1+EV_HI(a6),d0
		cmp.l	kd_EClock2+EV_HI(a6),d0
		bhi.s	KeyHandshakeEnd
		move.l	kd_EClock1+EV_LO(a6),d0
		cmp.l	kd_EClock2+EV_LO(a6),d0
		bhi.s	KeyHandshakeEnd

		; set up to shift in next key
		and.b	#~(CIACRAF_SPMODE)&$ff,_ciaacra
		rts

	END
