************************************************************************
*						     C. A. M. D.						       *
************************************************************************
* CMU Amiga Midi Driver - Carnegie Mellon University			       *
* 1988					- Commodore Amiga						       *
*																       *
* Design & Development	- Roger B. Dannenberg					       *
*						- Jean-Christophe Dhellemmes			       *
*						- Bill Barton							       *
* Copyright 1989 Carnegie Mellon University						       *
************************************************************************
*
* checkpoint.asm - CAMD MIDI Device Driver for Checkpoint serial card.
*
* Copyright 1990 Sylvan Technical Arts
*
************************************************************************
* Date	      | Change
*-----------------------------------------------------------------------
* 26-Apr-1990 : Created (Joe Pearce)
************************************************************************

		nolist
			include "exec/types.i"
			include "exec/execbase.i"
			include "hardware/custom.i"
			include "hardware/intbits.i"
			include "midi/devices.i"
			include "libraries/configvars.i"
			include "cps.i"
		list

JSRLIB		macro
			xref	_LVO\1
			jsr		_LVO\1(a6)
			endm

DISABLE		MACRO
		IFC     '\1',''
			MOVE.W  #$04000,custom+intena
			ADDQ.B  #1,IDNestCnt(A6)
     	ENDC
     	IFNC    '\1',''
			MOVE.L  4,\1
			MOVE.W  #$04000,custom+intena
			ADDQ.B  #1,IDNestCnt(\1)
     	ENDC
	     	ENDM

ENABLE      MACRO
		IFC     '\1',''
			SUBQ.B  #1,IDNestCnt(A6)
			BGE.S   ENABLE\@
			MOVE.W  #$0C000,custom+intena
ENABLE\@:
		ENDC
    	IFNC    '\1',''
			MOVE.L  4,\1
			SUBQ.B  #1,IDNestCnt(\1)
			BGE.S   ENABLE\@
			MOVE.W  #$0C000,custom+intena
ENABLE\@:
		ENDC
    		ENDM

Version		equ		0
Revision	equ		1
Ports		equ		2

AbsExecBase	EQU		4
custom		EQU		$00dff000

			section driver,code

****************************************************************
*
*   Standard MIDI Device driver header
*
****************************************************************

; code at start of file in case anyone tries to execute us as a program

			entry	FalseStart
FalseStart
			moveq	#-1,d0
			rts

MDD ; struct MidiDeviceData
			dc.l	MDD_Magic	; mdd_Magic
			dc.l	Name		; mdd_Name
			dc.l	IDString	; mdd_IDString
			dc.w	Version 	; mdd_Version
			dc.w	Revision	; mdd_Revision
			dc.l	Init		; mdd_Init
			dc.l	Expunge 	; mdd_Expunge
			dc.l	OpenPort	; mdd_OpenPort
			dc.l	ClosePort	; mdd_ClosePort
			dc.b	Ports		; mdd_NPorts
			dc.b	0			; mdd_Flags

Name	    dc.b    'Checkpoint',0
IDString    dc.b    'Checkpoint serial card MIDI device driver',0
		    ds.w    0			; force word alignment


****************************************************************
*
*   Serial Port Stuff
*
****************************************************************

		STRUCTURE CPSUnit,0
			ULONG	su_CardBase
			UBYTE	su_pad0
			UBYTE	su_IntMask
			UWORD	su_reserved0
			APTR	su_DataA
			APTR	su_XmitA
			APTR	su_RecvA
			APTR	su_DataB
			APTR	su_XmitB
			APTR	su_RecvB
			LABEL	CPSSubUnit_Size
			APTR	su_MPDA
			APTR	su_MPDB
			LABEL	CPSUnit_Size

	    STRUCTURE ExtInt,IS_SIZE
			UBYTE	si_IsActive
			UBYTE	si_pad0
			STRUCT	si_Unit0,CPSUnit_Size
			STRUCT	si_Unit1,CPSUnit_Size
			LABEL	ExtInt_Size

;si_CardBase	EQU		su_CardBase
;si_IntMask	EQU		su_IntMask
;si_DataA	EQU		su_DataA
;si_XmitA	EQU		su_XmitA
;si_RecvA	EQU		su_RecvA
;si_DataB	EQU		su_DataB
;si_XmitB	EQU		su_XmitB
;si_RecvB	EQU		su_RecvB

MPD0			; struct MidiPortData for port A
			dc.l    ActivateXmitA

MPD1			; struct MidiPortData for port B
			dc.l    ActivateXmitB

MPD2			; struct MidiPortData for port C
			dc.l    ActivateXmitC

MPD3			; struct MidiPortData for port D
			dc.l    ActivateXmitD

			cnop	0,4
CPSInt
		    dc.l    0		    		; ln_Succ
		    dc.l    0		    		; ln_Pred
	    	dc.b    NT_INTERRUPT    	; ln_Type
		    dc.b    0		    		; ln_Pri
		    dc.l    Name	    		; ln_Name
	    	dc.l    0		    		; is_Data
		    dc.l    ExtIntHandler   	; is_Code

			dc.w	0					; si_pad0
			ds.b	CPSSubUnit_Size,0	; si_Unit0 (except...)
			dc.l	MPD0				; si_Unit0+su_MPDA
			dc.l	MPD1				; si_Unit0+su_MPDB
			ds.b	CPSSubUnit_Size,0	; si_Unit1 (except...)
			dc.l	MPD2				; si_Unit1+su_MPDA
			dc.l	MPD3				; si_Unit1+su_MPDB

****************************************************************
*
*   MidiDeviceData Functions
*
****************************************************************

****************************************************************
*
*   Init
*
*   FUNCTION
*	Gets called by CAMD after being LoadSeg'ed.
*
*   INPUTS
*	None
*
*   RESULTS
*	TRUE if successful, FALSE on failure.
*
****************************************************************

init_reg 	reg a2/a6

Init		movem.l init_reg,-(sp)

			lea		CPSInt,a2				; get base of data

			lea		MDD,a1
			move.b	#2,mdd_NPorts(a1)		; reset NPorts

			move.l	AbsExecBase,a6 	    	; A6 = SysBase

			    ; get Checkpoint card base
			lea		ExpansionName,a1		; open expansion.library
			moveq	#0,d0
			JSRLIB	OpenLibrary
			move.l	d0,a6
			tst.l	d0
			beq		init_ret

			suba.l	a0,a0				; find the card (2055/0)
			move.l	#2055,d0
			move.l	a0,d1
			JSRLIB	FindConfigDev
			move.l	d0,a0
			tst.l	d0
			beq.s	1$

										; save card base
			move.l	cd_BoardAddr(a0),si_Unit0+su_CardBase(a2)

			move.l	#2055,d0			; look for a second board
			moveq	#0,d1				; a0 already has right value
			JSRLIB	FindConfigDev
			move.l	d0,a0
			tst.l	d0
			beq.s	1$

										; save second card base
			move.l	cd_BoardAddr(a0),si_Unit1+su_CardBase(a2)

			lea		MDD,a1
			addq.b	#2,mdd_NPorts(a1)	; add two more Ports

1$			move.l	a6,a1				; close expansion.library
			move.l	AbsExecBase,a6 	    ; A6 = SysBase
			JSRLIB	CloseLibrary

			move.l	si_Unit0+su_CardBase(a2),d0	; test (and set) Z-bit
			beq.s	init_ret

			moveq	#1,d0			    ; return TRUE
init_ret
			movem.l (sp)+,init_reg
			rts

ExpansionName
			dc.b	'expansion.library',0
			ds.w	0

****************************************************************
*
*   Expunge
*
*   FUNCTION
*	Gets called by CAMD immediately before being UnLoadSeg'ed.
*
*   INPUTS
*	None
*
*   RESULTS
*	None
*
****************************************************************

Expunge
			rts


****************************************************************
*
*   OpenPort
*
*   FUNCTION
*	Open a MIDI port.
*
*   INPUTS
*	D0.b - Port number (should be 0 or 1 for this driver [or 2/3])
*	A0 - Xmit function
*	A1 - Recv function
*	A2 - Data
*
*   RESULT
*	D0 - pointer to MidiPortData structure.
*
****************************************************************

op_reg	reg a3/a5/a6/d2

OpenPort
			movem.l op_reg,-(sp)

			move.l	AbsExecBase,a6 	    ; A6 = SysBase
			lea		CPSInt,a5			; get base of interrupt structure
			lea		si_Unit0(a5),a3		; point to first unit

			moveq	#0,d2
			move.b	d0,d2				; save port # in d2 (we have plans...)
			beq		openportA
			cmp.b	#1,d0
			beq		openportB

			lea		si_Unit1(a5),a3		; point to second unit
			cmp.b	#2,d0
			bne		openportB

openportA
			btst.b	d2,si_IsActive(a5)	; see if already open
			bne.s	is_open				; yep, that's a no-no

			move.l	a0,su_XmitA(a3)		; save func/data pointers
			move.l	a1,su_RecvA(a3)
			move.l	a2,su_DataA(a3)

			move.l	su_CardBase(a3),a0
				; reset channel A mode, disable rec & xmit
			move.b	#CR_MR1|CR_TD|CR_RCD,cps_CommandRegA(a0)
				; RTS off, RxRDY IRQ, Char Err, No Parity, 8 bits
			move.b	#MR1_8DB|MR1_NP,cps_ModeRegA(a0)
				; no special, 1.000 stop bit length
			move.b	#MR2_1SB,cps_ModeRegA(a0)
				; use timer
			move.b	#CSR_TIM|(CSR_TIM<<4),cps_ClockSelRegA(a0)
				; enable rec & xmit, clear error bits
			move.b	#CR_RCE|CR_TE|CR_RE,cps_CommandRegA(a0)
				; set clock source
			move.b	#ACR_IP2,cps_AuxControlReg(a0)
				; set the timer constant for MIDI
			move.w	#4,d1
			movep.w	d1,cps_CountTimerUR(a0)

			bsr	openportint
			bset.b	d2,si_IsActive(a5)
			bset.b	#ISR_RRA,su_IntMask(a3)

				; enable port A receice interrupt
			move.l	su_CardBase(a3),a0
			move.b	su_IntMask(a3),cps_IntMaskReg(a0)

				; return MPD
			move.l	su_MPDA(a3),d0
			bra.s	op_ret

openportB
			btst.b	d2,si_IsActive(a5)	; see if already open
			bne.s	is_open				; yep, that's a no-no

			move.l	a0,su_XmitB(a3)		; save func/data pointers
			move.l	a1,su_RecvB(a3)
			move.l	a2,su_DataB(a3)

			move.l	su_CardBase(a3),a0
				; reset channel A mode, disable rec & xmit
			move.b	#CR_MR1|CR_TD|CR_RCD,cps_CommandRegB(a0)
				; RTS off, RxRDY IRQ, Char Err, No Parity, 8 bits
			move.b	#MR1_8DB|MR1_NP,cps_ModeRegB(a0)
				; no special, 1.000 stop bit length
			move.b	#MR2_1SB,cps_ModeRegB(a0)
				; use timer
			move.b	#CSR_TIM|(CSR_TIM<<4),cps_ClockSelRegB(a0)
				; enable rec & xmit, clear error bits
			move.b	#CR_RCE|CR_TE|CR_RE,cps_CommandRegB(a0)
				; set clock source
			move.b	#ACR_IP2,cps_AuxControlReg(a0)
				; set the timer constant for MIDI
			move.w	#4,d1
			movep.w	d1,cps_CountTimerUR(a0)

			bsr	openportint
			bset.b	d2,si_IsActive(a5)
			bset.b	#ISR_RRB,su_IntMask(a3)

				; enable port B receive interrupt
			move.l	su_CardBase(a3),a0
			move.b	su_IntMask(a3),cps_IntMaskReg(a0)

				; return MPD
			move.l	su_MPDB(a3),d0

op_ret		movem.l (sp)+,op_reg
			rts

is_open			; error, port already open
			moveq	#0,d0
			movem.l (sp)+,op_reg
			rts

openportint
			DISABLE a0

			tst.b	si_IsActive(a5)	    ; if int active, don't add again!!
			bne.s	int_is_on

				    ; add EXTER interrupt vector
			moveq	#INTB_EXTER,d0
			move.l	a5,a1				; CPSInt structure
			JSRLIB	AddIntServer

			move.w	#INTF_SETCLR!INTF_EXTER,custom+intena   ; enable EXTER

int_is_on
			ENABLE a0
			rts

****************************************************************
*
*   ClosePort
*
*   FUNCTION
*	Close a MIDI port.
*
*   INPUTS
*	D0.b - Port number (0 or 1 [or 2 or 3] for this driver).
*
*   RESULT
*	None
*
****************************************************************

cp_reg	reg a3/a5/a6/d2

ClosePort
			movem.l cp_reg,-(sp)

			move.l	AbsExecBase,a6 	    ; A6 = SysBase
			lea		CPSInt,a5			; get base of interrupt structure
			lea		si_Unit0(a5),a3		; point to first unit

			moveq	#0,d2
			move.b	d0,d2				; save port # in d2
			beq		closeportA
			cmp.b	#1,d0
			beq		closeportB

			lea		si_Unit1(a5),a3		; point to second unit
			cmp.b	#2,d0
			bne		closeportB

closeportA
			move.l	su_CardBase(a3),a0

				; expect a0 = CardBase
			bsr		WaitSerialA

			bclr.b	#ISR_RRA,su_IntMask(a3)
			bclr.b	#ISR_TRA,su_IntMask(a3)
			move.b	su_IntMask(a3),cps_IntMaskReg(a0)
			bclr.b	d2,si_IsActive(a5)
			move.b	si_IsActive(a5),d0
			bne.s	closed

closeint	    ; remove EXTER interrupt handler
			moveq	#INTB_EXTER,d0
			move.l	a5,a1				; CPSInt structure
			JSRLIB	RemIntServer
			bra.s	closed

closeportB
			move.l	su_CardBase(a3),a0

				; expect a0 = CardBase
			bsr		WaitSerialB

			bclr.b	#ISR_RRB,su_IntMask(a3)
			bclr.b	#ISR_TRB,su_IntMask(a3)
			move.b	su_IntMask(a3),cps_IntMaskReg(a0)
			bclr.b	d2,si_IsActive(a5)
			move.b	si_IsActive(a5),d0
			beq.s	closeint

closed		movem.l (sp)+,cp_reg
			rts


****************************************************************
*
*   ActivateXmit
*
*   FUNCTION
*	Activate the transmit interrupt.  There is normally
*	one of these functions for every port in order to
*	reduce table lookup time.
*
*   INPUTS
*	A2 - Data passed to OpenPort() (not used here)
*
*   RESULT
*	None
*
****************************************************************

ActivateXmitA
				; enable port A xmit interrupt
			lea		CPSInt+si_Unit0,a1
			move.l	su_CardBase(a1),a0
			bset.b	#ISR_TRA,su_IntMask(a1)
			move.b	su_IntMask(a1),cps_IntMaskReg(a0)
			rts

ActivateXmitB
				; enable port B xmit interrupt
			lea		CPSInt+si_Unit0,a1
			move.l	su_CardBase(a1),a0
			bset.b	#ISR_TRB,su_IntMask(a1)
			move.b	su_IntMask(a1),cps_IntMaskReg(a0)
			rts

ActivateXmitC
				; enable port A xmit interrupt
			lea		CPSInt+si_Unit1,a1
			move.l	su_CardBase(a1),a0
			bset.b	#ISR_TRA,su_IntMask(a1)
			move.b	su_IntMask(a1),cps_IntMaskReg(a0)
			rts

ActivateXmitD
				; enable port B xmit interrupt
			lea		CPSInt+si_Unit1,a1
			move.l	su_CardBase(a1),a0
			bset.b	#ISR_TRB,su_IntMask(a1)
			move.b	su_IntMask(a1),cps_IntMaskReg(a0)
			rts

****************************************************************
*
*   WaitSerial
*
*   FUNCTION
*	Wait until Port TXR bit is set.  This
*	indicates that the serial shift register is empty.
*
*   INPUTS
*	a0 = CardBase
*
*   RESULTS
*	None
*
*   NOTE
*	This function busy waits!!!  It should only be called
*	once the transmit queue has emptied such that no more
*	than 320us is wasted here.
*
****************************************************************

WaitSerialA
			move.b	cps_StatusRegA(a0),d0
			btst.l	#SRB_TXR,d0
			beq		WaitSerialA

			rts

WaitSerialB
			move.b	cps_StatusRegB(a0),d0
			btst.l	#SRB_TXR,d0
			beq		WaitSerialB

			rts

****************************************************************
*
*   Serial Interrupt Handlers
*
****************************************************************


****************************************************************
*
*   ExtIntHandler
*
*   FUNCTION
*	Checkpoint serial card interrupt handler.  Handles both
*	receive and transmit.
*
****************************************************************

ext_reg reg a2/a3/a5/d2

ExtIntHandler
			movem.l ext_reg,-(sp)

			moveq	#0,d2					; init result to "didn't handle"

			lea		CPSInt+si_Unit0,a3		; get first unit
			bsr		extloop					; check out interrupts

			lea		CPSInt+si_Unit1,a3		; get second unit
			bsr		extloop					; check it out too

			move.l	d2,d0
			movem.l (sp)+,ext_reg
			rts

extloop		move.b	su_IntMask(a3),d0		; get ints we care about
			beq.s	extdone					; if zero, not ours!

			move.l	su_CardBase(a3),a1
			and.b	cps_IntStatusReg(a1),d0	; AND in card interrupts

			btst	#ISR_RRA,d0				; port A receiver interrupt?
			beq.s	not_recA

			move.l	su_RecvA(a3),a5			; return user supplied values
			move.l	su_DataA(a3),a2

			moveq	#0,d0
			move.b	cps_RecBufferA(a1),d0	; get a received byte

			move.b	cps_StatusRegA(a1),d1	; get the status of channel
			and.b	#SR_OVR|SR_FRA,d1		; framing/overrun error?
			beq.s	1$						; no, skip

			or.w	#$8000,d0				; report an error
			move.b	#CR_RE,cps_CommandRegA(a1)	; clear error bits

1$			jsr		(a5)
			moveq	#1,d2
			bra.s	extloop

not_recA	btst	#ISR_RRB,d0			; port B receiver interrupt?
			beq.s	not_recB

			move.l	su_RecvB(a3),a5
			move.l	su_DataB(a3),a2

			moveq	#0,d0
			move.b	cps_RecBufferB(a1),d0	; get a received byte

			move.b	cps_StatusRegB(a1),d1	; get the status of channel
			and.b	#SR_OVR|SR_FRA,d1		; framing/overrun error?
			beq.s	1$						; no, skip

			or.w	#$8000,d0				; report an error
			move.b	#CR_RE,cps_CommandRegB(a1)	; clear error bits

1$			jsr		(a5)
			moveq	#1,d2
			bra.s	extloop

not_recB	btst	#ISR_TRA,d0			; port A xmit interrupt?
			beq.s	not_xmitA

			move.l	su_XmitA(a3),a5
			move.l	su_DataA(a3),a2
			jsr		(a5)                   	; returns byte in D0, last byte flag in D1

			move.l	su_CardBase(a3),a1
			move.b	d0,cps_XmitBufferA(a1)

			moveq	#1,d2

			tst.b	d1		    ; is last byte?
			beq.s	extloop

			bclr	#ISR_TRA,su_IntMask(a3)
			move.b	su_IntMask(a3),cps_IntMaskReg(a1)
			bra.s	extdone

not_xmitA	btst	#ISR_TRB,d0			; port B xmit interrupt?
			beq.s	extdone

			move.l	su_XmitB(a3),a5
			move.l	su_DataB(a3),a2
			jsr		(a5)                   	; returns byte in D0, last byte flag in D1

			move.l	su_CardBase(a3),a1
			move.b	d0,cps_XmitBufferB(a1)

			moveq	#1,d2

			tst.b	d1		    ; is last byte?
			beq		extloop

			bclr	#ISR_TRB,su_IntMask(a3)
			move.b	su_IntMask(a3),cps_IntMaskReg(a1)

extdone		rts

			end
