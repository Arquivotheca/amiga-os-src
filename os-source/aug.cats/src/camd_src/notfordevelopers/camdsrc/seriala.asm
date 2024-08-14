*************************************************************************
*     C. A. M. D.	(Commodore Amiga MIDI Driver)                   *
*************************************************************************
*									*
* Design & Development	- Roger B. Dannenberg				*
*			- Jean-Christophe Dhellemmes			*
*			- Bill Barton					*
*                       - Darius Taghavy                                *
*                                                                       *
* Copyright 1990 by Commodore Business Machines				*
*                                                                       *
*************************************************************************
*
* seriala.asm - Amiga internal serial port I/O handlers
*
*************************************************************************

      nolist
	include "exec/types.i"
	include "exec/macros.i"
	include "hardware/custom.i"
	include "hardware/intbits.i"

	include "camdlib.i"
      list


	idnt	seriala.asm

	    ; internal
	xdef	_ActivateIntSerXmit
	xdef	_IntSerRecvHandler
	xdef	_IntSerXmitHandler
	xdef	_WaitIntSer

	    ; external
	xref	RecvHandler
	xref	XmitHandler
	xref	_custom


TUNERBF  set 0			    ; set to 1 to fine tune RBFRetry

RBFRetry equ 20 		    ; retry count to catch odd RBF latency

INTF_SET equ INTF_SETCLR
INTF_CLR equ 0

	    ; serdatr register bit def's (not found in any include file)
	BITDEF	SERDAT,OVRUN,15
	BITDEF	SERDAT,RBF,14
	BITDEF	SERDAT,TBE,13
	BITDEF	SERDAT,TSRE,12



****************************************************************
*
*   WaitIntSer
*
*   FUNCTION
*	Wait until Amiga serial port TSRE bit is set.  This
*	indicates that the serial shift register is empty.
*
*   INPUTS
*	None
*
*   RESULTS
*	None
*
*   NOTE
*	This function busy waits!!!  It should only be called
*	once the transmit queue has emptied (i.e. TBE has
*	already been set) such that no more than 320us is
*	wasted here.
*
****************************************************************

_WaitIntSer	    ; (C addressable symbol)
	lea	_custom+serdatr+(15-SERDATB_TSRE)/8,a0
wis_loop
	btst.b	#SERDATB_TSRE&7,(a0)
	beq	wis_loop
	rts


****************************************************************
*
*   ActivateIntSerXmit
*
*   FUNCTION
*	Activate internal serial port Xmit interrupt handler.
*	This simply enables the TBE interrupt.	The TBE interrupt
*	must always be pending when disabled in order to use
*	this trick.
*
*   INPUTS
*	None
*
*   RESULTS
*	None
*
****************************************************************

_ActivateIntSerXmit	; (C addressable symbol, but not called from C)
	move.w	#INTF_SET!INTF_TBE,_custom+intena   ; enable TBE interrupt, but leave request pending
	rts


****************************************************************
*
*   IntSerXmitHandler
*
*   FUNCTION
*	Amiga serial port transmit interrupt handler.  Calls
*	the general transmit handler and expects a byte in
*	D0 and a last byte flag in D1.
*
*	When the last byte is detected, the TBE interrupt
*	is disabled, thus deferring calling IntSerXmitHandler
*	again until there are some bytes ready to send.
*	ActivateIntSerXmit is called when there are more bytes
*	to send.
*
*   INPUTS
*	A0 - custom
*	A1 - Data
*	A6 - SysBase
*
*   RESULTS
*	None
*
****************************************************************

; timing:  42 + N * (102 + XmitHandler time)

_IntSerXmitHandler	; (C addressable symbol)
	move.l	a2,-(sp)

	move.l	a1,a2			    ; A2 = Data

sxh_loop
	move.w	#INTF_CLR!INTF_TBE,_custom+intreq   ; clear TBE interrupt request

	bsr	XmitHandler		    ; returns byte in D0, last byte flag in D1

	or.w	#$100,d0		    ; add stop bit
	move.w	d0,_custom+serdat	    ; send byte

	tst.b	d1			    ; is last byte?
	beq.s	sxh_notlast
					    ; xmit queue is empty
	move.w	#INTF_CLR!INTF_TBE,_custom+intena   ; disable TBE interrupt.
	bra.s	sxh_done		    ; next request remains pending until there are more bytes to send

sxh_notlast				    ; test for another TBE
	btst.b	#INTB_TBE&7,_custom+intreqr+(15-INTB_TBE)/8
	bne	sxh_loop

sxh_done
	move.l	(sp)+,a2
	rts


****************************************************************
*
*   IntSerRecvHandler
*
*   FUNCTION
*	Amiga serial port receive interrupt handler.
*
*   INPUTS
*	A0 - custom
*	A1 - pointer to SerInt.Data
*	A6 - SysBase
*
*   RESULTS
*	None
*
****************************************************************

; timing:  50 + NBytes * (76 + RecvHandler + 22 * NRetry)
;	   (65us for 1 byte + 61us max for retry)


; !!! test

    ifgt TUNERBF
	dseg
	public	foo
foo	ds.w	RBFRetry+1
	cseg
    endc


_IntSerRecvHandler	; (C addressable symbol)
	move.l	a2,-(sp)

	move.l	a1,a2			    ; A2 = Data
	lea	serdatr(a0),a0              ; A0 = serdatr

srh_byteloop
	move.w	(a0),d0
	move.w	#INTF_CLR!INTF_RBF,intreq-serdatr(a0)   ; clear the interrupt request

	bsr	RecvHandler

		    ; test RBF bit several times before leaving (suggested by David Silver)
	lea	_custom+serdatr,a0	    ; A0 = serdatr
	moveq	#SERDATB_RBF&7,d0
	moveq	#RBFRetry,d1
srh_retryloop
	btst.b	d0,(15-SERDATB_RBF)/8(a0)
	dbne	d1,srh_retryloop

    ifgt TUNERBF
; !!! test
	beq	1$

	lea	foo,a1
	add.w	d1,d1
	addq.w	#1,(a1,d1.w)        ; add 1 to histogram entry for cnt
1$
; !!! test
    endc

	bne	srh_byteloop

	move.l	(sp)+,a2
	rts


	end
