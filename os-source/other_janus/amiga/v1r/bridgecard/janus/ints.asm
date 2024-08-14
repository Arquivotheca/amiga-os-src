
;****************************************************************************
;
; ints.asm -- low level janus interrupt handling code
;
; Copyright (c) 1986, Commodore Amiga Inc.  All rights reserved.
;
; changed CheckJanusInt to use D1.W instead of D1.L (SMB 8/27)
;****************************************************************************


		INCLUDE 'assembly.i'

		NOLIST
		INCLUDE 'exec/types.i'
		INCLUDE 'exec/nodes.i'
		INCLUDE 'exec/libraries.i'
		INCLUDE 'exec/interrupts.i'
		INCLUDE 'exec/ables.i'
		INCLUDE 'hardware/intbits.i'
		INCLUDE 'janus.i'
		INCLUDE 'janusvar.i'
		INCLUDE 'janusreg.i'
		LIST

		INCLUDE 'asmsupp.i'

		XDEF	JanusIntCode
		XDEF	SetJanusHandler
		XDEF	SetJanusEnable
		XDEF	SetJanusRequest
		XDEF	SendJanusInt
		XDEF	CheckJanusInt
		XDEF	GetParamOffset
		XDEF	SetParamOffset
		XDEF	GetJanusStart

		XREF	subsysName

		XLIB	AddIntServer

		INT_ABLES

JanusIntCode:
		;------ set up our data
		move.l	ja_IoBase(a1),a0

		;------ poll for the interrupt
		moveq	#0,d0
		move.b	jio_IntReq(a0),d0		; get int bits 
		and.b	ja_SpurriousMask(a1),d0 	; mask startup trash
		or.b	d0,ja_IntReq+3(a1)		; mark as received
	IFGE	INFOLEVEL-60
	pea	0
	move.b	ja_IntEna+3(a1),3(a7)
	move.b	d0,1(a7)
	LINEMSG 60,<'I%02xE%02x. '>
	addq.l	#4,a7
	ENDC
		move.l	ja_IntReq(a1),d0		; gather all pending
		and.l	ja_IntEna(a1),d0		; mask to enabled
		bne.s	jic_gotint			; if non-zero: do  

	PUTMSG	90,<'jic_noint'>
		;------ d0 & ccr are zero here
		rts

		;------ the int is for us.  Now check for software ints.
		;------ d0 contains the currently enabled interrupt requests
jic_gotint:

		movem.l d2-d3/a2/a5-a6,-(sp)
		move.l	a1,a6
		btst	#JINTB_SYSINT,d0
		beq	jic_processint

	PUTMSG	80,<'%s/sys req check'>

		;------ we now suspect there is a new system request.  lets
		;------ try and find it.
FIRSTSYSREQ	EQU 8

		CLEAR	d2
		CLEAR	d0

		move.l	ja_ParamMem(a6),a5
		add.l	#WordAccessOffset,a5
		move.w	jb_Interrupts(a5),d2
		sub.l	#WordAccessOffset,a5

		;------ get ptr to base of system request vectors
		;------ we start at the end and work backwards.
		;------ odd byte is our requests, even is pc's
		lea	((MAXHANDLER)*2)(a5,d2.l),a5
				   
		moveq	#-1,d3		    ; none of Amiga's are pending

		;------ loop counter -- counts down
		moveq	#MAXHANDLER-FIRSTSYSREQ-1,d1

	IFGE	INFOLEVEL-90
	movem.l d1/a5,-(sp)
	PUTMSG	90,<'%s/pre check req: cnt %ld, addr 0x%lx'>
	addq.l	#8,sp
	ENDC

	INFOMSG 70,<'tas: 0x'>

jic_checkreq:
	IFGE	INFOLEVEL-70
	pea	0	   
	move.b	(a5),3(a7)
	LINEMSG 1,<' %lx'>
	addq.l	#4,sp
	ENDC
		and.b	-(a5),d3	    ; looking for pending Amiga ints
		tas	-(a5)
		bmi.s	jic_noreq

		;------ there was a request.  go and notice it.
		bset	d1,d0

jic_noreq:
		dbra	d1,jic_checkreq

		tas	-2(a5)		    ; check PC Booted byte
		bmi.s	jic_fixup

		move.b	#$ff,ja_SpurriousMask(a6)
	    
jic_fixup:
		;------ fixup time: the loop should have run from index
		;------ 31 to index 8.	Instead it ran from 23 (MAXHANDLER-
		;------ FIRSTSYSREQ-1) to 0.  Shift the request pattern
		;------ left by eight bits to properly allign it.
		lsl.l	#8,d0



		;------ now add the interrupts into the list
		or.l	d0,ja_IntReq(a6)
	IFGE	INFOLEVEL-80
	move.l	ja_IntEna(a6),-(sp)
	move.l	ja_IntReq(a6),-(sp)
	PUTMSG	1,<'%s/processint: req 0x%lx, ena 0x%lx'>
	addq.l	#8,sp
	ENDC

;===== This section taken out because it was crashing the PC big time!!
;===== If an interrupt occurs with no $7f in the table then it was just
;===== a spurious interrupt and should not do anything. (SMB - 9/12/86)
		;------ if Amiga ints are still pending, dispatch them
;		addq.b	#1,d3
;		bne.s	jic_processint
;		move.l	ja_IoBase(a6),a0
;		move.b	#JPCSENDINT,jio_PcIntGen(a0)
;=======================================================================

jic_processint:
		;------ now go and dispatch all the enabled ints
		move.l	ja_IntReq(a6),d2
		and.l	ja_IntEna(a6),d2

	IFGE	INFOLEVEL-60
	move.l	d2,-(sp)
	move.l	ja_IntEna(a6),-(sp)
	move.l	ja_IntReq(a6),-(sp)
	PUTMSG	1,<'%s/processint: req 0x%lx, ena 0x%lx, dispatched 0x%lx'>
	add.w	#12,sp
	ENDC

		eor.l	d2,ja_IntReq(a6)

		;------ bookkeeping
		move.l	ja_IntHandlers(a6),a5

jic_processloop:
		;------ for each real request, go and displatch it
		move.l	(a5)+,a0
		btst	#0,d2
		beq.s	jic_processincr

		;------ make sure there is a handler
		move.l	a0,d0
		beq.s	jic_processincr

		;------ call the handler
		movem.l IS_DATA(a0),a1/a2

	IFGE	INFOLEVEL-90
	movem.l a1/a2,-(sp)
	PUTMSG	1,<'%s/processloop: calling 0x%lx/0x%lx'>
	addq.l	#8,sp
	ENDC

		jsr	(a2)

jic_processincr:
		;------ bump the loop counters, look for termination
		lsr.l	#1,d2
		bne.s	jic_processloop

jic_end:

	PUTMSG	90,<'%s/jic_end'>

		moveq	#1,d0
		movem.l (sp)+,d2-d3/a2/a5/a6
		rts


; oldhandler = SetJanusHandler( jintnum, intserv )
; d0				d0	 a1
;
; set up an int handler for a janus interrupt.	the old value for this
; interrupt is returned
;
SetJanusHandler:
		move.l	d0,d1
		lsl.l	#2,d1 
		move.l	ja_IntHandlers(a6),a0
		lea	0(a0,d1.l),a0
		move.l	a1,d1

	IFGE	INFOLEVEL-80
	movem.l d0/a0/a1,-(sp)
	PUTMSG	1,<'%s/SetJanusHandler: jnum %ld, @hand 0x%lx to 0x%lx'>
	lea	12(sp),sp
	ENDC

		DISABLE a1
		move.l	(a0),d0
		move.l	d1,(a0)
		ENABLE	a1

		rts

; oldenables = SetJanusEnable( jintnum, newenable )
; d0			       d0	d1
;
; return the current values of the specified interrupt enable bit,
; then set it to newenable.  Newenable should be either 0 or 1.

SetJanusEnable:
		move.l	d2,-(sp)
		DISABLE a0
		move.l	ja_IntEna(a6),d2

		;------ see if we are setting or clearing
		tst.l	d1
		bne.s	sje_set

		;------ clear the bit
		bclr	d0,d2
		bra.s	sje_return

sje_set:
		;------ set the bit
		bset	d0,d2

sje_return:
		;------ set up the return value
		sne	d0
		moveq	#1,d1
		and.l	d1,d0


		;------ ensure the SYSINT bit is correct
		move.l	d2,d1
		clr.b	d1
		tst.l	d1		    ; check for any software enabled
		beq.s	disableSys
		bset	#JINTB_SYSINT,d2
		bra.s	updateEna
disableSys:
		bclr	#JINTB_SYSINT,d2
updateEna:
		;------ put the enable mask back
		move.l	d2,ja_IntEna(a6)

		;------ ensure the hardware mask is correct
		move.l	ja_IoBase(a6),a0
		not.b	d2
		move.b	d2,jio_IntEna(a0)
		not.b	d2
		beq.s	disableAll
		move.b	#$ff&~JCNTRLF_ENABLEINT,jio_Control(a0)
		bra.s	endEnable
disableAll:
		move.b	#$ff&~JCNTRLF_DISABLEINT,jio_Control(a0)
endEnable:
		ENABLE	a1

		move.l	(sp)+,d2
		rts



; oldrequest = SetJanusRequest( jintnum, newrequest )
; d0				d0	 d1
;
; return the current values of the specified interrupt request bit,
; then set it to newrequest.  Newrequest should be either 0 or 1.

SetJanusRequest:
		move.l	d2,-(sp)

		DISABLE a0
		move.l	ja_IntReq(a6),d2

		tst.l	d1
		bne.s	sjr_set

		;------ clear the bit
		bclr	d0,d2
		bra.s	sjr_return

sjr_set:
		;------ set the bit
		bset	d0,d2

sjr_return:
		;------ set up the return value
		sne	d0
		moveq	#1,d1
		and.l	d1,d0

		;------ put the request mask back
		move.l	d2,ja_IntReq(a6)
		ENABLE	a0

		move.l	(sp)+,d2
		rts

; SendJanusInt( jintnum )
;		d0
;
; the janus side.  This routine can only send
; system request interrupts (e.g. software style interrupts).

SendJanusInt:
	IFGE	INFOLEVEL-50
	MOVE.L	D0,-(SP)
	INFOMSG	50,<'Janus: Sending int #%lx'>
	ADDQ.L	#4,SP
	ENDC
		move.l	ja_ParamMem(a6),a0

		;------ get an offset to the interrupt
		lsl.w	#1,d0
		add.l	#WordAccessOffset,a0
		add.w	jb_Interrupts(a0),d0
		sub.l	#WordAccessOffset,a0

		;------ set the software interrupt
		move.b	#JSETINT,1(a0,d0.l)
	IFGE	INFOLEVEL-50
	MOVE.L	D0,-(SP)
	INFOMSG	50,<'Janus: Offset to int = %ld'>
	ADDQ.L	#4,SP
	MOVE.L	A0,-(SP)
	INFOMSG	50,<'Janus: Base address used was $%lx'>
	ADDQ.L	#4,SP
	ENDC

		;------ and set the hardware interrupt
		move.l	ja_IoBase(a6),a0
		move.b	#JPCSENDINT,jio_PcIntGen(a0)
	INFOMSG	10,<'Sent an interrupt'>
		rts 

; status = CheckJanusInt( jintnum )
; d0			  d0
;
; check to see if the pc has received the interrupt.  non-zero means the
; pc has not yet noticed the interrupt.

CheckJanusInt:
		move.l	ja_ParamMem(a6),a0

		;------ get and index to the interrupt
		lsl.w	#1,d0
		add.l	#WordAccessOffset,a0
		add.w	jb_Interrupts(a0),d0
		sub.l	#WordAccessOffset,a0

		;------ check the software interrupt
		move.w	d0,d1
		CLEAR	d0
		move.b	1(a0,d1.w),d0

		;------ top bit set implies no interrupt
		spl	d0
		rts

; offset = GetParamOffset( jintnum )
; d0			   d0
;
; return the offset in parameter memory of the parameter block for interrupt
; 'jintnum'.  A $ffff implies that there is no parameter.

GetParamOffset:
		move.l	ja_ParamMem(a6),a0
		add.l	#WordAccessOffset,a0

		;------ get an offset to the parameter word
		lsl.w	#1,d0
		add.w	jb_Parameters(a0),d0
		move.l	d0,d1

		;------ and return it to the user
		CLEAR	d0
		move.w	0(a0,d1.l),d0
		rts

; oldOffset = SetParamOffset( jintnum, offset )
; d0			      d0       d1:0-15
;
; set the parameter of interrupt 'jintnum' to offset.

SetParamOffset:
		move.l	ja_ParamMem(a6),a0
		add.l	#WordAccessOffset,a0
		lsl.w	#1,d0
		add.w	jb_Parameters(a0),d0
		DISABLE a1
		move.w	0(a0,d0.l),-(a7)
		move.w	d1,0(a0,d0.l)
		ENABLE	a1
		move.w	(a7)+,d0
		rts


; startAddr = GetJanusStart()
; d0
;
; return a pointer to the start of common janus memory	

GetJanusStart:
		move.l	ja_ExpanBase(a6),d0
		rts


	  END

