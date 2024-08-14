**********************************************************************
*                                                                    *
*   Copyright 1984-1992 Commodore Amiga Inc. All rights reserved.    *
*                                                                    *
**********************************************************************


**********************************************************************
*
* $Id: resource.asm,v 36.23 92/03/10 14:36:08 mks Exp $
*
* $Log:	resource.asm,v $
*	Revision 36.23  92/03/10  14:36:08  mks
*	Changed the priority of the handler to 120.
*	Also cleaned out the change log.
*	
**********************************************************************

	SECTION		cia

*------	Included Files -----------------------------------------------

	INCLUDE	'assembly.i'

	INCLUDE	'exec/types.i'
	INCLUDE	'exec/nodes.i'
	INCLUDE	'exec/lists.i'
	INCLUDE	'exec/ables.i'
	INCLUDE	'exec/memory.i'
	INCLUDE	'exec/interrupts.i'
	INCLUDE	'exec/libraries.i'
	INCLUDE	'exec/tasks.i'
	INCLUDE	'exec/execbase.i'

	INCLUDE	'hardware/cia.i'
	INCLUDE	'hardware/intbits.i'

	INCLUDE	'internal.i'

*------	Imported Globals ---------------------------------------------

	INT_ABLES

	EXTERN_DATA	_ciaa
	EXTERN_DATA	_ciab
	EXTERN_DATA	_intreq
	EXTERN_DATA	_intena
	EXTERN_DATA	_ciaaicr
	EXTERN_DATA	_ciabicr
	EXTERN_DATA	_ciabddra
	EXTERN_DATA	_ciabpra
	EXTERN_DATA	_custom

	EXTERN_DATA	CIAName
	EXTERN_DATA	CIAAName
	EXTERN_DATA	CIABName

*------	Imported Functions ------------------------------------------

	EXTERN_SYS	MakeLibrary
	EXTERN_SYS	AddResource
	EXTERN_SYS	FreeMem
	EXTERN_SYS	AddIntServer
	EXTERN_SYS	AllocMem
	EXTERN_SYS	OpenResource


*------	Exported ----------------------------------------------------

	XDEF	InitCode
	XDEF	EndMarker



*------ Bit Definitions (These need to be included from somewhere!!!)

CALLSYS	MACRO
	JSR	_LVO\1(A6)
	ENDM


*------ special macros:

CIA_DISABLE	MACRO	scratch,from
		move.l	\2,\1
		DISABLE	\1,NOFETCH
		ENDM


CIA_ENABLE	MACRO	scratch,from
		move.l	\2,\1
		ENABLE	\1,NOFETCH
		ENDM

SAVEREGS	MACRO
		movem.l	d0-d3/a0-a3/a6,-(sp)	;a3 used by timer.device
		ENDM

RESTREGS	MACRO
		movem.l	(sp)+,d0-d3/a0-a3/a6
		ENDM

*------ Assumptions -------------------------------------------

	IFNE	CIAICRB_TA
	FAIL	"CIAICRB_TA not 0 - recode"
	ENDC

	IFNE	CIAICRB_TB-1
	FAIL	"CIAICRB_TB not 1 - recode"
	ENDC

	IFNE	cs_TimerCode-cs_TimerBase-4
	FAIL	"cs_TimerCode does not follow cs_TimerBase - recode"
	ENDC

	IFNE	cs_CIAA-cs_CIAB-4
	FAIL	"cs_CIAA does not follow cs_CIAB - recode"
	ENDC

*------ Functions Offsets -------------------------------------

CIAFUN	MACRO
	DC.W	\1-initialFunctions
	ENDM


initialFunctions:
		DC.W	-1
		CIAFUN	AddICRVector
		CIAFUN	RemICRVector
		CIAFUN	AbleICR
		CIAFUN	SetICR
		DC.W	-1


*------ Initial Code ------------------------------------------
*
*
InitCode:
		MOVEM.L	A2/A3,-(SP)

;------ Allocate memory for shared data (assumed to succeed for such
;------ a small chunk of memory early on in the system - note that
;------ there has been no error checking for MakeLibrary() calls below.

		moveq	#cs_SIZEOF,d0
		move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
		CALLSYS	AllocMem
		move.l	d0,a3		;cache

	;------	make resource for _ciaa
		BSR	makeCIA

	;------ cache ptr to CIAABase

		move.l	a2,cs_CIAA(a3)

	;------ cache ptr to CIABBase, bit number, and index used by
	;------ the timer.device (MOVEM these always in order of
	;------ Dx-Dy/An where x > y - this guarantees that the
	;------ values are moved to/from memory in the same order)

		moveq	#CIAICRB_TA,d0
		moveq	#PREFSCOUNT,d1
		movem.l	d0-d1/a2,cs_TimerUsage(a3)
		bset	d0,CR_TimerAlloc(a2)


		MOVE.L	#_ciaa,CR_HWADDR(A2)
		move.l	a6,CR_EXECBASE(a2)

		;------	finish connecting the interrupt handler
		MOVE.W	#INTF_PORTS,CR_IntMask(A2)
		MOVE.L	#intrHandler2,IS_CODE(A1)
		MOVEQ	#INTB_PORTS,D0
		CALLSYS	AddIntServer

*****		MOVE.W	#(INTF_SETCLR+INTF_PORTS),_intena

		LEA	CIAAName(PC),A0
		BSR.S	addCIA


	;------	make resource for _ciab
		BSR.S	makeCIA

	;------ cache ptr to CIABBase

		move.l	a2,cs_CIAB(a3)

	;
	;
	;

		LEA	_ciab,A0
		MOVE.L	A0,CR_HWADDR(A2)
		move.l	a6,CR_EXECBASE(a2)

		MOVEQ	#$FFFFFF00!(CIAF_COMDTR!CIAF_COMRTS),D0
		OR.B	D0,ciaddra(A0)		; initialize serial stuff
		OR.B	D0,ciapra(A0)		;

		;------	finish connecting the interrupt handler
		MOVE.W	#INTF_EXTER,CR_IntMask(A2)
		MOVE.L	#intrHandler6,IS_CODE(A1)
		MOVEQ	#INTB_EXTER,D0
		CALLSYS	AddIntServer

*****		MOVE.W	#(INTF_SETCLR+INTF_EXTER),_intena

		LEA	CIABName(PC),A0
		BSR.s	addCIA

		MOVEM.L	(SP)+,A2/A3
		RTS

addCIA:
		MOVE.L	A0,LN_NAME(A1)		; put name in intr handler
		MOVE.L	A0,LN_NAME(A2)		;   and resource
		;------	tell system about resource:
		MOVE.L	A2,A1
		CALLSYS	AddResource
		RTS

makeCIA:
		LEA	initialFunctions(PC),A0
		SUB.L	A1,A1		* no initial data
		SUB.L	A2,A2		* no initializer
		move.l	#CR_SIZE,D0
		CALLSYS	MakeLibrary
		MOVE.L	D0,A2
		MOVE.B	#NT_RESOURCE,LN_TYPE(A2)
		CLR.B	LN_PRI(A2)

		;------	connect the interrupt handler
		LEA	CR_INTNODE(A2),A1
		MOVE.B	#NT_INTERRUPT,LN_TYPE(A1)
		move.b	#120,LN_PRI(A1)
		MOVE.L	A2,IS_DATA(A1)

		;------ Ptr to shared data
		move.l	a3,CR_SharedData(a2)

		RTS


*-----------------------------------------------------------------------
*
*	CIA Interrupt Handlers
*
*	CIA-A Level 2 interrupts
*
*-----------------------------------------------------------------------


intrHandler2:
		movem.l	d2/a2,-(sp)

		move.l	a1,a2				;cache

	;-- Loops back, and checks for new interrupts before
	;-- returning to exec (useful for parallel.device, and
	;-- other frequent level 2 interrupts)
	;

Dispatcher:

	;-- Check to see if the timer.device generated this
	;-- interrupt.  The bit will only be set if the timer
	;-- device is using CIAB, which sets this bit in CIAA.

		bclr	#TIMERINT,CR_TimerINT(a2)	;atomic test/clear
		beq.s	ctcheck_port			;only please!

		move.l	CR_SharedData(a2),a1
		MOVEM.L	cs_TimerBase(a1),A1/A5
		JSR	(A5)

ctcheck_port:


	;-- Set CPU interrupt mask to level 7 (NMI) - this is a faster
	;-- way to DISABLE (faster than hitting Paula's interrupt master
	;-- interrupt enable/disable bit), and gets rid of the spurious
	;-- interrupt problem.
	;
	;-- This trick works here because we KNOW we are in supervisor mode.

		move.w		SR,d0		; save SR
		ori.w		#$0700,SR	; Raise I0,I1,I2 to level 7

		move.b	_ciaaicr,d2

		BCLR	#7,D2			; never store upper bit

		OR.B	CR_IActive(A2),D2
		MOVE.B	D2,CR_IActive(A2)

		AND.B	CR_IEnable(A2),D2
		move.b	d2,CR_IProcessing(A2)	; for setICR
		BEQ.S	ct_done

		eor.b	d2,CR_IActive(a2)	; clear what we'll process

		move.w		d0,SR		; ENABLE

	;
	; Setup A6=Execbase.   While not necessarily documented as
	; having a ptr to execbase, we anticipate that many 1.3
	; programmers expected A6 to have execbase in it.  V36
	; and V37 timer.device EClock_Int traditionally has trashed
	; A6 (I plan on fixing that), but the point to note here is
	; that cia under V36/V37 loops back up, and checks for fresh
	; interrupts before going back to exec.
	;
	; The implication is that someone using TIMER A could be
	; hosed if an interrupt routine trashes A6, and cia
	; catches the TIMER A interrupt on the second pass.
	;
	; If you read page 310 of the RKM Libs & Devices, its
	; pretty easy to see why programmers got confused, and
	; they even would have worked under 1.3.  And if thats not
	; enough, A0 wasn't being used by the old 1.3 cia code either,
	; so programmers could have worked if they thought A0 contained
	; _custom.
	;

		move.l		CR_EXECBASE(a2),a6

	;
	; lea _custom,a0 here for 1.3 compatability
	;


		lea	_custom,a0

		;------	we will test each bit in turn.  LSR shifts bit zero
		;------	out into the carry bit.  if the carry is clear
		;------	then there is no int for this level

		;------	This code has been optimized for the case of
		;------	sparse interrupts.

		LSR.B	#1,D2
		BCS.S	ctdo_ta

ctcheck_tb:
		LSR.B	#1,D2
		BCS.S	ctdo_tb
		BEQ.S	Dispatcher

ctcheck_alrm:
		LSR.B	#1,D2
		BCS.S	ctdo_alrm
		BEQ.S	Dispatcher

ctcheck_sp:
		LSR.B	#1,D2
		BCS.S	ctdo_sp
		BEQ.S	Dispatcher

ctcheck_flg:
		LSR.B	#1,D2
		BCS.S	ctdo_flg
                BRA.s	Dispatcher

ct_done:
		move.w	d0,SR			; ENABLE
		movem.l	(sp)+,d2/a2
		moveq.l	#0,d0
		rts


ctdo_ta:
		;------	test timer A
		MOVEM.L	CR_IVTA(A2),A1/A5
		JSR	(A5)
		BRA.S	ctcheck_tb

ctdo_tb:
		;------	test timer B
		MOVEM.L	CR_IVTB(A2),A1/A5
		JSR	(A5)
		BRA.S	ctcheck_alrm

ctdo_alrm:
		;------	test Alarm
		MOVEM.L	CR_IVALRM(A2),A1/A5
		JSR	(A5)
		BRA.S	ctcheck_sp

ctdo_sp:
		;------	test serial
		MOVEM.L	CR_IVSP(A2),A1/A5
		JSR	(A5)
		BRA.S	ctcheck_flg

ctdo_flg:
		;------	test flag register
		MOVEM.L	CR_IVFLG(A2),A1/A5
		JSR	(A5)
		BRA	Dispatcher


*-----------------------------------------------------------------------
*
*	CIA Interrupt Handlers
*
*	CIA-A Level 6 interrupts
*
*-----------------------------------------------------------------------

intrHandler6:
		movem.l	d2/a2,-(sp)

	; Disable is implied here - we are already running at level 6.
	; Nothing short of an NMI can interrupt us here

		move.b	_ciabicr,d2

		BCLR	#7,D2			; never store upper bit

		OR.B	CR_IActive(A1),D2
		MOVE.B	D2,CR_IActive(A1)

		AND.B	CR_IEnable(A1),D2
		BEQ.S	ctb_none		; exit ASAP if nothing

		move.b	d2,CR_IProcessing(A1)	; for setICR

		move.l	a1,a2
		eor.b	d2,CR_IActive(a2)	; clear what we'll process

		LSR.B	#1,D2
		BCS.S	ctbdo_ta

ctbcheck_tb:
		LSR.B	#1,D2
		BCS.S	ctbdo_tb
		BEQ.S	ctb_done

ctbcheck_alrm:
		LSR.B	#1,D2
		BCS.S	ctbdo_alrm
		BEQ.S	ctb_done

ctbcheck_sp:
		LSR.B	#1,D2
		BCS.S	ctbdo_sp
		BEQ.S	ctb_done

ctbcheck_flg:
		LSR.B	#1,D2
		BCS.S	ctbdo_flg

ctb_done:
		clr.b	CR_IProcessing(a2)	; done processing
ctb_none:
		movem.l	(sp)+,d2/a2
		moveq.l	#0,d0
		rts


ctbdo_ta:
		;------	test timer A
		MOVEM.L	CR_IVTA(A2),A1/A5
		JSR	(A5)
		BRA.S	ctbcheck_tb

ctbdo_tb:
		;------	test timer B
		MOVEM.L	CR_IVTB(A2),A1/A5
		JSR	(A5)
		BRA.S	ctbcheck_alrm

ctbdo_alrm:
		;------	test Alarm
		MOVEM.L	CR_IVALRM(A2),A1/A5
		JSR	(A5)
		BRA.S	ctbcheck_sp

ctbdo_sp:
		;------	test serial
		MOVEM.L	CR_IVSP(A2),A1/A5
		JSR	(A5)
		BRA.S	ctbcheck_flg

ctbdo_flg:
		;------	test flag register
		MOVEM.L	CR_IVFLG(A2),A1/A5
		JSR	(A5)
		BRA.S	ctb_done


******* cia.resource/SetICR ******************************************
*
*   NAME
*	SetICR -- Cause, clear, and sample ICR interrupts.
*
*   SYNOPSIS
*	oldMask = SetICR( Resource, mask )
*	D0                A6        D0
*
*	WORD SetICR( struct Library *, WORD );
*
*   FUNCTION
*	This function provides a means of reseting, causing, and
*	sampling 8520 CIA interrupt control registers.
*
*   INPUTS
*	mask            A bit mask indicating which interrupts to be
*	                    effected. If bit 7 is clear the mask
*	                    indicates interrupts to be reset.  If bit
*	                    7 is set, the mask indicates interrupts to
*	                    be caused. Bit positions are identical to
*	                    those in 8520 ICR.
*
*   RESULTS
*	oldMask         The previous interrupt register status before
*	                    making the requested changes.  To sample
*	                    current status without making changes,
*	                    call the function with a null parameter.
*
*   EXAMPLES
*	Get the interrupt mask:
*	    mask = SetICR(0)
*	Clear serial port interrupt:
*	    SetICR(0x08)
*
*   NOTE
*	The CIA resources are special in that there is more than one
*	of them in the system. Because of this, the C language stubs
*	in amiga.lib for the CIA resources require an extra parameter
*	to specify which CIA resource to use. The synopsys for the
*	amiga.lib stubs is as follows:
*
*	oldMask = SetICR( Resource, mask )
*	D0                A6        D0
*
*	WORD SetICR( struct Library *, WORD );
*
*	***WARNING***
*
*	Never read the contents of the CIA interrupt control registers
*	directly.  Reading the contents of one of the CIA interrupt
*	control registers clears the register.  This can result in
*	interrupts being missed by critical operating system code, and
*	other applications.
*
*   EXCEPTIONS
*	Setting an interrupt bit for an enabled interrupt will cause
*	an immediate interrupt.
*
*   SEE ALSO
*	cia.resource/AbleICR()
*
**********************************************************************

SetICR:
		CIA_DISABLE	A0,CR_EXECBASE(a6)

		MOVE.L	CR_HWADDR(A6),A0
		MOVE.B	ciaicr(A0),D1
		BCLR	#7,D1
		OR.B	D1,CR_IActive(A6)
		CLEAR	D1
		MOVE.B	CR_IActive(a6),D1		; old value
		or.b	CR_IProcessing(a6),d1
		TST.B	D0
		BEQ.S	ICR_Common

; -------	reflect new setting in mirror register:
		BCLR	#7,D0
		BNE.S	sI_add

		NOT.B	D0
		AND.B	D0,CR_IActive(a6)
		and.b	d0,CR_IProcessing(a6)
		BRA.S	ICR_Common

sI_add:		OR.B	D0,CR_IActive(a6)

ICR_Common:
; -------	cause an interrupt if necessary:
		MOVE.B	CR_IEnable(A6),D0
		AND.B	CR_IActive(A6),D0
		BEQ.S	pi_noIntr

		MOVE.W	CR_IntMask(A6),D0
		OR.W	#INTF_SETCLR,D0
		MOVE.W	D0,_intreq

pi_noIntr:
		CIA_ENABLE	A0,CR_EXECBASE(a6)
		MOVE.L	D1,D0
		RTS

******* cia.resource/AbleICR *****************************************
*
*   NAME
*	AbleICR -- Enable/disable ICR interrupts.
*
*   SYNOPSIS
*	oldMask = AbleICR( Resource, mask )
*	D0                 A6        D0
*
*	WORD AbleICR( struct Library *, WORD );
*
*   FUNCTION
*	This function provides a means of enabling and disabling 8520
*	CIA interrupt control registers. In addition it returns the
*	previous enable mask.
*
*   INPUTS
*	mask            A bit mask indicating which interrupts to be
*	                    modified. If bit 7 is clear the mask
*	                    indicates interrupts to be disabled. If
*	                    bit 7 is set, the mask indicates
*	                    interrupts to be enabled. Bit positions
*	                    are identical to those in 8520 ICR.
*
*   RESULTS
*	oldMask         The previous enable mask before the requested
*	                    changes. To get the current mask without
*	                    making changes, call the function with a
*	                    null parameter.
*
*   EXAMPLES
*	Get the current mask:
*	    mask = AbleICR(0)
*	Enable both timer interrupts:
*	    AbleICR(0x83)
*	Disable serial port interrupt:
*	    AbleICR(0x08)
*
*   EXCEPTIONS
*	Enabling the mask for a pending interrupt will cause an
*	immediate processor interrupt (that is if everything else is
*	enabled). You may want to clear the pending interrupts with
*	SetICR() prior to enabling them.
*
*   NOTE
*	The CIA resources are special in that there is more than one
*	of them in the system. Because of this, the C language stubs
*	in amiga.lib for the CIA resources require an extra parameter
*	to specify which CIA resource to use. The synopsys for the
*	amiga.lib stubs is as follows:
*
*	oldMask = AbleICR( Resource, mask )
*	D0                 A6        D0
*
*	WORD AbleICR( struct Library *, WORD );
*
*   SEE ALSO
*	cia.resource/SetICR()
*
**********************************************************************

AbleICR:
		CIA_DISABLE	A0,CR_EXECBASE(a6)

		MOVE.L	CR_HWADDR(A6),A0
		MOVE.B	D0,ciaicr(A0)
		CLEAR	D1
		MOVE.B	CR_IEnable(a6),D1		; old value
		TST.B	D0
		BEQ.S	ICR_Common

; -------	reflect new setting in mirror register:
		BCLR	#7,D0
		BNE.S	aI_add

		NOT.B	D0
		AND.B	D0,CR_IEnable(a6)
		BRA.S	ICR_Common

aI_add:		OR.B	D0,CR_IEnable(a6)
		bra.s	ICR_Common

******* cia.resource/AddICRVector ************************************
*
*   NAME
*	AddICRVector -- attach an interrupt handler to a CIA bit.
*
*   SYNOPSIS
*	interrupt = AddICRVector( Resource, iCRBit, interrupt )
*	D0                        A6        D0      A1
*
*	struct Interrupt *AddICRVector( struct Library *, WORD,
*		struct Interrupt * );
*
*   FUNCTION
*	Assign interrupt processing code to a particular interrupt bit
*	of the CIA ICR.  If the interrupt bit has already been
*	assigned, this function will fail, and return a pointer to the
*	owner interrupt.  If it succeeds, a null is returned.
*
*	This function will also enable the CIA interrupt for the given
*	ICR bit.
*
*   INPUTS
*	iCRBit          Bit number to set (0..4).
*	interrupt       Pointer to interrupt structure.
*
*   RESULT
*	interrupt       Zero if successful, otherwise returns a
*	                    pointer to the current owner interrupt
*	                    structure.
*
*   NOTE
*	A processor interrupt may be generated immediatly if this call
*	is successful.
*
*	In general, it is probably best to only call this function
*	while DISABLED so that the resource to which the interrupt
*	handler is being attached may be set to a known state before
*	the handler is called. You MUST NOT change the state of the
*	resource before attaching your handler to it.
*
*	The CIA resources are special in that there is more than one
*	of them in the system. Because of this, the C language stubs
*	in amiga.lib for the CIA resources require an extra parameter
*	to specify which CIA resource to use. The synopsys for the
*	amiga.lib stubs is as follows:
*
*	interrupt = AddICRVector( Resource, iCRBit, interrupt )
*	D0                        A6        D0      A1
*
*	struct Interrupt *AddICRVector( struct Library *, WORD,
*		struct Interrupt *);
*
*	***WARNING***
*
*	Never assume that any of the CIA hardware is free for use.
*	Always use the AddICRVector() function to obtain ownership
*	of the CIA hardware registers your code will use.
*
*	Note that there are two (2) interval timers per CIA.  If
*	your application needs one of the interval timers, you
*	can try to obtain any one of the four (4) until AddICRVector()
*	succeeds.  If all four interval timers are in-use, your
*	application should exit cleanly.
*
*	If you just want ownership of a CIA hardware timer, or register,
*	but do not want interrupts generated, use the AddICRVector()
*	function to obtain ownership, and use the AbleICR() function
*	to turn off (or on) interrupts as needed.
*
*	Note that CIA-B generates level 6 interrupts (which can degrade
*	system performance by blocking lower priority interrupts).  As
*	usual, interrupt handling code should be optimized for speed.
*
*	Always call RemICRVector() when your code exits to release
*	ownership of any CIA hardware obtained with AddICRVector().
*
*   SEE ALSO
*	cia.resource/RemICRVector(), cia.resource/AbleICR()
*
**********************************************************************

AddICRVector:

		CLEAR	D1
		MOVE.B	D0,D1
		MULU	#IV_SIZE,D1

		CIA_DISABLE	A0,CR_EXECBASE(a6)

		LEA	CR_IVTA(A6,D1.W),A0
		MOVE.L	IV_NODE(A0),D1
		BNE.S	vi_error

	;-- come back to here if able to swap timer.device's
	;-- cia.  all registers had been saved, and restored
	;-- if we got back here.

AddICRAvailable:

		MOVE.L	A1,IV_NODE(A0)
		MOVE.L	IS_CODE(A1),IV_CODE(A0)
		MOVE.L	IS_DATA(A1),IV_DATA(A0)

	;-- track allocation of TA/TB bits

		BSET	D0,CR_Allocated(a6)

		MOVE.W	#$80,D1		* enable

		BSR.S	vi_able
vi_enable:
		CIA_ENABLE	A0,CR_EXECBASE(a6)
		RTS

vi_noalloc:
		MOVE.L	D1,D0
		BRA.S	vi_enable


;
;  1.) See if they want the CIA that the timer.device is using.
;      If not, we are done.
;
;  2.) Find a free timer; if none, we are done.
;
;  3.) If one is free, call-back timer hook, and give the requested
;      ICRVector to the caller.
;

vi_error:
		btst	d0,CR_TimerAlloc(a6)
		beq.s	vi_noalloc

		SAVEREGS
		bsr.s	FindFreeVec
		bne.s	vi_done
		bsr.s	CallTimer

		RESTREGS
		bra.s	AddICRAvailable


******* cia.resource/RemICRVector ************************************
*
*   NAME
*	RemICRVector -- Detach an interrupt handler from a CIA bit.
*
*   SYNOPSIS
*	RemICRVector( Resource, iCRBit, interrupt )
*	              A6        D0      A1
*
*	void RemICRVector( struct Library *, WORD, struct Interrupt *);
*
*   FUNCTION
*	Disconnect interrupt processing code for a particular
*	interrupt bit of the CIA ICR.
*
*	This function will also disable the CIA interrupt for the
*	given ICR bit.
*
*   INPUTS
*	iCRBit          Bit number to set (0..4).
*	interrupt       Pointer to interrupt structure.
*
*   RESULT
*
*   NOTE
*	The CIA resources are special in that there is more than one
*	of them in the system. Because of this, the C language stubs
*	in amiga.lib for the CIA resources require an extra parameter
*	to specify which CIA resource to use. The synopsys for the
*	amiga.lib stubs is as follows:
*
*	RemICRVector( Resource, iCRBit, interrupt )
*	              A6        D0      A1
*
*	void RemICRVector( struct Library *, WORD, struct Interrupt *);
*
*   SEE ALSO
*	cia.resource/AddICRVector()
*
**********************************************************************

RemICRVector:
		CIA_DISABLE	A0,CR_EXECBASE(a6)

;  1.) Free the interrupt vector passed in by the caller
;
;  2.) Check to see if there is now something free which the
;      timer.device could use (better than what the timer.device
;      has already).  An index number is used (3..0) to make
;      this determination.
;
;  3.) If so, free what the timer.device is using, and reassign it.
;
;  4.) Note that this is all done within DISABLE()/ENABLE()
;

		bsr.s	rm_icrvec

	;-- check to see if we can jump the timer

		SAVEREGS
		bsr.s	FindFreeVec
		movem.l	cs_TimerUsage(a1),d0-d1/a6

                cmp.b	d1,d3		; d1 (3..0) d3 (3..-1)
		ble.s	vi_done		;signed branch

		bsr.s	rm_icrvec	;release what timer.device has
		bsr.s	CallTimer	;and reassign timer.device

vi_done:
		RESTREGS
		bra.s	vi_noalloc

********************************************************************
**
** Clears interrupt node for bit/CIA specified in D0/a6
** Also marks interrupt bit as free.
**

rm_icrvec:
		CLEAR	D1
		MOVE.B	D0,D1

		MULU	#IV_SIZE,D1

		LEA	CR_IVTA(A6,D1.W),A0
		CLR.L	IV_NODE(A0)
		CLEAR	D1

	;-- Mark as free

		BCLR	D0,CR_Allocated(a6)

	;-- disable/enable ICR bit
vi_able:
		BSET	D0,D1
		MOVE.W	D1,D0
		BSR	AbleICR
		CLEAR	D0
		RTS


**********************************************************************
**
** Support routines used by jumpy timer.device/cia
**
*********************************************************************
**
**********************************************************************
**
** Find a free interrupt vector.
**
** Search count down timers on CIAA, and CIAB in order of:
**
**   3         2
** CIAA A -> CIAB B   CIAA A is checked twice - this was done
**         /          to make the code SMALLER.  It works out fine.
**        /
** CIAA A -> CIAB A   preference order is 3, 2, 0
**   1         0
**
**
** In:
** a6 -- ptr to either CIA Base
**
** Out:
** CC -- EQUAL if a free one found; NOT EQUAL otherwise
** a1 -- ptr to SharedData
** a2 -- ptr to CIABase with free interval timer
** a3 -- ptr to other CIABase
** d2 -- bit number of free interval timer (lower 3 bits)
** d3 -- index number (3..0 or -1)
**
**
** WARNING -
**
**	Becareful about changing this code (e.g., make sure you
**	understand what it returns, and how it works).  This
**	code tests the 3 possible interval timers that the
**	timer.device might use to find the next free one, and
**	tests them in a specific order.
**
**	Note that when this loop is done, we have the following info:
**
**	CC - EQUAL if a free interval timer was found.
**
**	a1 - pts to the shared data area which is used
**	     by both CIA's, and the timer.device to store
**	     shared data.
**
**	a2 - pts to the CIABase with the free interval timer.
**	     This is the interval timer that we MIGHT reassign
**	     the timer.device to use.
**
**	a3 - pts to the OTHER CIABase (there are two CIA libraries)
**	     This pointer MIGHT be used later if we reassign the
**	     the timer.device.
**
**	d2 - The bit number of the free interval timer.  This is
**	     the interval timer that we MIGHT reassign the
**	     timer.device to use.  Only the lower 3 bits are valid.
**
**	d3 - An index number (3, 2, 1, 0, -1) where the only
**	     valid numbers are 3, 2, and 0.  -1 means no free
**	     interval timer.  1 is impossible.
**
**	     The index number is used to compare the free
**	     timer found by this routine with the what the
**	     timer.device is already using.  If the index
**	     number returned by this routine is less than
**	     what the timer.device is already using, it means
**	     we don't need to reassign the timer.device.
**

FindFreeVec:
		move.l	CR_SharedData(a6),a1
		movem.l	cs_CIAB(a1),a2/a3	;a2==CIAB a3==CIAA
		moveq	#PREFSCOUNT,d3
		moveq	#%01000000,d2
	;
	; bit mask in d2 where only bits 0-2 are used for the BTST
	;
	; D3	CIA	D2 (bits 2-0)
	;
	; 3	CIAA	000		interval timer A
	; 2	CIAB	001		interval timer B
	; 1	CIAA	000		interval timer A again
	; 0	CIAB	000		interval timer A
	;
	;
	; note - an initial EXG, and LSR instruction is done BEFORE
	; the BTST.
	;

findfreevec:
		exg	a3,a2
		lsr.b	#3,d2
		btst	d2,CR_Allocated(a2)
		dbeq	d3,findfreevec
		rts

**********************************************************************
**
** Call timer hook - reassign timer.device to use a new count-down timer
**
** Caches ptr to CIABase, timer bit, and index which will be used
** by the timer.device.  Also mark which interval timer the timer.device
** is using in the CIA libraries.
**
** In:
**
** a1 -- ptr to SharedData
** a2 -- ptr to CIABase to use
** a3 -- ptr to other CIA Base
** d2 -- bit number (CIAICRB_TA or CIAICRB_TB) to use
** d3 -- index number (3..0)
**
CallTimer:
		and.b	#$01,d2
		movem.l	d2-d3/a2,cs_TimerUsage(a1)
		clr.b	CR_TimerAlloc(a3)
		clr.b	CR_TimerAlloc(a2)
		bset	d2,CR_TimerAlloc(a2)
		move.l	cs_TimerCall(a1),a0
		jmp	(a0)
EndMarker:


	END
