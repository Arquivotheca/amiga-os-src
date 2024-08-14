 
* *** ints.asm ****************************************************************
* 
* ints.asm -- low level janus interrupt handling code
* 
* Copyright (C) 1986, 1987, 1988, Commodore Amiga Inc.  
* All rights reserved.
* 
* Date        Name               Description
* ----------  -----------------  ----------------------------------------------
* 15-Jul-88   -RJ                Changed all files to work with new includes
* 19 Feb 88   -RJ Mical-         Added autodocs
* 8/27/87     Steve Beats        Changed CheckJanusInt to use D1.W instead 
*                                of D1.L
* Early 1986  Burns/Katin clone  Created this file!
* 
* *****************************************************************************


		INCLUDE 'assembly.i'

		NOLIST
		INCLUDE 'exec/types.i'
		INCLUDE 'exec/nodes.i'
		INCLUDE 'exec/libraries.i'
		INCLUDE 'exec/interrupts.i'
		INCLUDE 'exec/ables.i'
		INCLUDE 'hardware/intbits.i'

		INCLUDE 'janus/janusbase.i'
		INCLUDE 'janus/janusvar.i'
		INCLUDE 'janus/janusreg.i'
		INCLUDE 'janus/services.i'
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
		move.l	jb_IoBase(a1),a0

		;------ poll for the interrupt
		moveq	#0,d0
		move.b	jio_IntReq(a0),d0		; get int bits 
		and.b	jb_SpurriousMask(a1),d0 	; mask startup trash
		or.b	d0,jb_IntReq+3(a1)		; mark as received

	IFGE	INFOLEVEL-60
	pea	0
	move.b	jb_IntEna+3(a1),3(a7)
	move.b	d0,1(a7)
	LINEMSG 60,<'I%02xE%02x. '>
	addq.l	#4,a7
	ENDC

		move.l	jb_IntReq(a1),d0		; gather all pending
		and.l	jb_IntEna(a1),d0		; mask to enabled
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

		move.l	jb_ParamMem(a6),a5
		add.l	#WordAccessOffset,a5
		move.w	ja_Interrupts(a5),d2
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

		move.b	#$ff,jb_SpurriousMask(a6)
	    
jic_fixup:
		;------ fixup time: the loop should have run from index
		;------ 31 to index 8.	Instead it ran from 23 (MAXHANDLER-
		;------ FIRSTSYSREQ-1) to 0.  Shift the request pattern
		;------ left by eight bits to properly allign it.
		lsl.l	#8,d0



		;------ now add the interrupts into the list
		or.l	d0,jb_IntReq(a6)
	IFGE	INFOLEVEL-80
	move.l	jb_IntEna(a6),-(sp)
	move.l	jb_IntReq(a6),-(sp)
	PUTMSG	1,<'%s/processint: req 0x%lx, ena 0x%lx'>
	addq.l	#8,sp
	ENDC

;===== This section taken out because it was crashing the PC big time!!
;===== If an interrupt occurs with no $7f in the table then it was just
;===== a spurious interrupt and should not do anything. (SMB - 9/12/86)
		;------ if Amiga ints are still pending, dispatch them
;		addq.b	#1,d3
;		bne.s	jic_processint
;	INFOMSG	0,<'PAI'>
;		move.l	jb_IoBase(a6),a0
;		move.b	#JPCSENDINT,jio_PcIntGen(a0)
;========================================================================

jic_processint:
		;------ now go and dispatch all the enabled ints
		move.l	jb_IntReq(a6),d2
		and.l	jb_IntEna(a6),d2

	IFGE	INFOLEVEL-60
	move.l	d2,-(sp)
	move.l	jb_IntEna(a6),-(sp)
	move.l	jb_IntReq(a6),-(sp)
	PUTMSG	1,<'%s/processint: req 0x%lx, ena 0x%lx, dispatched 0x%lx'>
	add.w	#12,sp
	ENDC

		eor.l	d2,jb_IntReq(a6)

		;------ bookkeeping
		move.l	jb_IntHandlers(a6),a5

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


SetJanusHandler:
* === SetJanusHandler() =======================================================
* 
* NAME
*     SetJanusHandler  --  Set up an interrupt handler for a Janus interrupt
* 
* 
* SYNOPSIS
*     OldHandler = SetJanusHandler(JanusInterruptNumber, InterruptHandler);
*     D0                           D0                    A1
* 
*         APTR    OldHandler;
*         ULONG   JanusInterruptNumber;
*         APTR    InterruptHandler;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     This routine sets up an interrupt handler for a particular Janus 
*     interrupt.  The address of the old interrupt routine is returned.  
*     A NULL means that there was no previous handler for this interrupt.  
*     If you provide an InterruptHandler argument of NULL then interrupts 
*     will not be processed for that JanusInterruptNumber.
*     
*     This is a very low-level Janus routine.  Scary things lurk in the 
*     shadows down here.  You are probably better served by dealing with one 
*     of the higher-up routines such as SetupJanusSig() or Add/GetService().
*     
*     
* INPUTS
*     JanusInterruptNumber = the number of the Janus interrupt that you 
*         want to service.  These are defined in services.[hi]
*     InterruptHandler = the address of your interrupt code
*     
*     
* RESULTS
*     OldHandler = the address of the handler that previously managed 
*         this interrupt
*     
*     
* SEE ALSO
*     SetJanusEnable(), SetJanusRequest()

		move.l	d0,d1
		lsl.l	#2,d1 
		move.l	jb_IntHandlers(a6),a0
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



SetJanusEnable:
* === SetJanusEnable() ========================================================
* 
* NAME
*     SetJanusEnable  --  Enable or disable a Janus interrupt
* 
* 
* SYNOPSIS
*     OldEnable = SetJanusEnable(JanusInterruptNumber, Enable);
*     D0                         D0                    D1
* 
*         ULONG   OldEnable;
*         ULONG   JanusInterruptNumber;
*         ULONG   Enable;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     This routine enables or disables a particular Janus interrupt.  
*     
*     Each Janus interrupt may be individually enable or disabled (this is 
*     in addition to the control of setting the interrupt handler 
*     to NULL).  If the interrupt is disabled then requests that are 
*     received will not generate interrupts.  These requests may be 
*     detected via SetJanusRequest.  
*     
*     If Enable is 0 then the interrupt is disabled.  If it is 
*     1 then the interrupt is enabled.  All other values are reserved.  
*     
*     "This routine will initiate interrupt processing if a newly-enabled 
*     interrupt has a pending request."  Unfortunately, this, like Intuition 
*     mutually-excluding gadgets, does not currently happen.  
*     
*     This is a very low-level Janus routine.  You sure you want to be 
*     down here?  You are probably better served by dealing with one of 
*     the higher-up routines such as SetupJanusSig() or Add/GetService().
*     
*     
* INPUTS
*     JanusInterruptNumber = the number of the Janus interrupt that you 
*         want to modify.  These are defined in services.[hi]
*     Enable = 1 if you want to enable the interrupt, 0 if you want to 
*         disable the interrupt.  All other values are reserved
*     
*     
* RESULTS
*     OldEnable = 1 if the interrupt was enabled at the time of the call, 
*         0 if the interrupt was disabled
*     
*     
* SEE ALSO
*     SetJanusHandler(), SetJanusRequest()


		move.l	d2,-(sp)
		DISABLE a0
		move.l	jb_IntEna(a6),d2

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
		move.l	d2,jb_IntEna(a6)

		;------ ensure the hardware mask is correct
		move.l	jb_IoBase(a6),a0
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



SetJanusRequest:
* === SetJanusRequest() ========================================================
* 
* NAME
*     SetJanusRequest  --  Set or clear interrupt request for a Janus interrupt
* 
* 
* SYNOPSIS
*     OldRequest = SetJanusRequest(JanusInterruptNumber, Request);
*     D0                           D0                    D1
* 
*         ULONG   OldRequest;
*         ULONG   JanusInterruptNumber;
*         ULONG   Request;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     This routine sets or clears an interrupt request for a Janus 
*     interrupt.  If the Request argument is zero then the request is 
*     cleared.  If the Request argument is one then the request is set.  
*     In either case the old value of the request state is returned.  
*     
*     Setting a request one day will generate an interrupt if the request is 
*     "enabled".  This does not currently happen.  Tant pis.
*     
*     This is a very low-level Janus routine.  You sure you want to be 
*     down here?  You are probably better served by dealing with one of 
*     the higher-up routines such as SetupJanusSig() or Add/GetService().
*     
*     
* INPUTS
*     JanusInterruptNumber = the number of the Janus interrupt that you 
*         want to service.  These are defined in services.[hi]
*     Request = 1 if you want to set an interrupt request, 0 if you want 
*         to clear any interrupt request.  All other values are reserved
*     
*     
* RESULTS
*     OldRequest = 1 if an interrupt request was pending, 0 if 
*         no interrupt request was pending
*     
*     
* SEE ALSO
*     SetJanusHandler(), SetJanusEnable()


		move.l	d2,-(sp)

		DISABLE a0
		move.l	jb_IntReq(a6),d2

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
		move.l	d2,jb_IntReq(a6)
		ENABLE	a0

		move.l	(sp)+,d2
		rts



SendJanusInt:
* === SendJanusInt() ========================================================
* 
* NAME
*     SendJanusInt  --  Mark a Janus system request and interrupt the PC
* 
* 
* SYNOPSIS
*     VOID SendJanusInt(JanusInterruptNumber);
*                       D0
*         ULONG   JanusInterruptNumber;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     The routine marks the request in the system interrupt area and then 
*     posts a hardware interrupt to the PC.  
*     
*     This call is useful for "system" requests -- e.g. those requests 
*     not directly defined by the hardware.   
*     
*     This is a mid-level Janus routine.  You are probably better 
*     served by dealing with the Janus Service routines.  
*     
*     
* INPUTS
*     JanusInterruptNumber = the number of the Janus interrupt that you 
*         want to signal.  These are defined in services.[hi]
*     
*     
* RESULTS
*     None
*     
*     
* SEE ALSO
*     CheckJanusInt()


	IFGE	INFOLEVEL-50
	MOVE.L	D0,-(SP)
	INFOMSG	50,<'Janus: Sending int #%lx'>
	ADDQ.L	#4,SP
	ENDC
		move.l	jb_ParamMem(a6),a0

		;------ get an offset to the interrupt
		lsl.w	#1,d0
		add.l	#WordAccessOffset,a0
		add.w	ja_Interrupts(a0),d0
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
		move.l	jb_IoBase(a6),a0
		move.b	#JPCSENDINT,jio_PcIntGen(a0)
	INFOMSG	10,<'Sent an interrupt'>
		rts 



CheckJanusInt:
* === CheckJanusInt() ========================================================
* 
* NAME
*     CheckJanusInt  --  Check whether the PC has noticed a Janus interrupt
* 
* 
* SYNOPSIS
*     Status = CheckJanusInt(JanusInterruptNumber);
*     D0                     D0
*         ULONG   Status;
*         ULONG   JanusInterruptNumber;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     This call returns the status byte from the interrupt area 
*     associated with JanusInterruptNumber.  It can be used to tell 
*     if the PC has noticed a pending interrupt yet.  A value of 
*     JNOINT (0xFF) means no interrupt is pending (which probably 
*     means that the PC has already processed it).  JSENDINT (0x7F) 
*     means that the interrupt is pending.  Anything else should 
*     be treated with suspicion.  
*     
*     This is a very low-level Janus routine.  You are probably better 
*     served by dealing with one of the higher-up routines such as 
*     SetupJanusSig() or, better yet, Add/GetService().
*     
*     
* INPUTS
*     JanusInterruptNumber = the number of the Janus interrupt that you 
*         want to check.  These are defined in services.[hi]
*     
*     
* RESULTS
*     Status = the status byte from the interrupt area of JanusInterruptNumber
*     
*     
* SEE ALSO
*     SendJanusInt()


		move.l	jb_ParamMem(a6),a0

		;------ get and index to the interrupt
		lsl.w	#1,d0
		add.l	#WordAccessOffset,a0
		add.w	ja_Interrupts(a0),d0
		sub.l	#WordAccessOffset,a0

		;------ check the software interrupt
		move.w	d0,d1
		CLEAR	d0
		move.b	1(a0,d1.w),d0

		;------ top bit set implies no interrupt
		spl	d0
		rts



GetParamOffset:
* === GetParamOffset() ========================================================
* 
* NAME
*     GetParamOffset  --  Get the Janus offset to an interrupt parameter block
* 
* 
* SYNOPSIS
*     Offset = GetParamOffset(JanusInterruptNumber);
*     D0:0-15                 D0
*         SHORT   Offset;
*         ULONG   JanusInterruptNumber;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     Gets the Janus offset to the parameter block, if any, of the specified 
*     JanusInterruptNumber.  If the result is -1 then that interrupt has 
*     no parameter block.  If not -1, the offset refers to Janus parameter 
*     memory, which has a memory area type descriptor of MEMF_PARAMETER.  
*     
*     
* EXAMPLE
*     /* Get a byte-access pointer to a Janus interrupt's parameter memory */
*     UBYTE   *paramptr;
*     SHORT   offset;
*         offset = GetParamOffset(JSERV_READPC);
*         if (offset != -1)
*             paramptr = JanusOffsetToMem(offset, 
*                     MEMF_PARAMETER | MEM+BYTEACCESS);
*     
*     
* INPUTS
*     JanusInterruptNumber = the number of the Janus interrupt that you 
*         want to examine.  These are defined in services.[hi]
*     
*     
* RESULTS
*     Offset = 16-bit offset into Janus memory.  The type of the memory 
*         referred to by this offset is MEMF_PARAMETER
* 
* 
* SEE ALSO
*     SetParamOffset(), JanusOffsetToMem(), JanusMemToOffset()


		move.l	jb_ParamMem(a6),a0
		add.l	#WordAccessOffset,a0

		;------ get an offset to the parameter word
		lsl.w	#1,d0
		add.w	ja_Parameters(a0),d0
		move.l	d0,d1

		;------ and return it to the user
		CLEAR	d0
		move.w	0(a0,d1.l),d0
		rts



SetParamOffset:
* === SetParamOffset() ========================================================
* 
* NAME
*     SetParamOffset  --  Set the Janus offset of an interrupt parameter block
* 
* 
* SYNOPSIS
*     OldOffset = SetParamOffset(JanusInterruptNumber, Offset);
*     D0:0-15                    D0                    D1:0-15
*         USHORT  OldOffset;
*         ULONG   JanusInterruptNumber;
*         USHORT  Offset;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     Sets the Janus offset of the parameter block of the specified 
*     JanusInterruptNumber.  By convention, -1 means that the interrupt 
*     has no parameter block.  
* 
*     The type of the memory referred to by this offset must be 
*     MEMF_PARAMETER.  
*     
* 
* EXAMPLE
*     UBYTE  *ptr;
*         if ( ptr = AllocJanusMem(100, MEMF_PARAMETER | MEM_BYTEACCESS) )
*             SetParamOffset( JSERV_READPC, JanusMemToOffset(ptr) );
*     
*     
* INPUTS
*     JanusInterruptNumber = the number of the Janus interrupt that you 
*         want to set.  These are defined in services.[hi]
*     Offset = 16-bit offset into Janus memory.  The type of the memory 
*         referred to by this offset is MEMF_PARAMETER
* 
* 
* RESULTS
*     OldOffset = 16-bit offset into Janus memory.  The type of the memory 
*         referred to by this offset is MEMF_PARAMETER
* 
* 
* SEE ALSO
*     GetParamOffset(), JanusOffsetToMem(), JanusMemToOffset()


		move.l	jb_ParamMem(a6),a0
		add.l	#WordAccessOffset,a0
		lsl.w	#1,d0
		add.w	ja_Parameters(a0),d0
		DISABLE a1
		move.w	0(a0,d0.l),-(a7)
		move.w	d1,0(a0,d0.l)
		ENABLE	a1
		move.w	(a7)+,d0
		rts



GetJanusStart:
* === GetJanusStart() ========================================================
* 
* NAME
*     GetJanusStart  --  Get the address of the base of the Janus board
* 
* 
* SYNOPSIS
*     MemPtr =  GetJanusStart();
*     D0,A0
*         APTR    MemPtr;
*     From assembly:  A6 has pointer to JanusBase
* 
* 
* FUNCTION
*     Returns the address of the base of the Janus board.
*     
*     Assembly language programmers note that the address is returned 
*     in both registers D0 and A0.
*     
*     
* INPUTS
*     None
*     
*     
* RESULTS
*     MemPtr = address of the base of the Janus board
*     
*     
* SEE ALSO
*     None

		move.l	jb_ExpanBase(a6),d0
		MOVE.L	D0,A0
		rts


	  END

