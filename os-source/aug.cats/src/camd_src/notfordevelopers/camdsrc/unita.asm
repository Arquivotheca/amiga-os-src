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
* unita.asm   - Unit related functions
*
*************************************************************************

      nolist
	include "exec/types.i"
	include "exec/interrupts.i"
	include "exec/macros.i"
	include "exec/semaphores.i"
	include "midi/camddevices.i"
	include "midi/mididefs.i"
	include "midi/midiprefs.i"
	include "utility/hooks.i"

	include "camdlib.i"
      list


; DEBUG set 1

	idnt	unita.asm

	    ; internal
	xdef	RecvHandler
	xdef	SendMsgToUnits
	xdef	SendRTToUnits
	xdef	SendSXListToUnits
	xdef	SendSysExToUnits
	xdef	XmitHandler
	xdef	_CloseMDPort
	xdef	_OpenMDPort
	xdef	_UnitTaskLoop
	xdef	_UnitTaskSegment
	xdef	_OutputFunc

	    ; external
	xref	MidiMsgLen
	xref	ParseMidiTime
	xref	_UnitTask
	xref	GetSysEx

	section __MERGED,DATA
	xref	_AbsExecBase
	xref	_CamdBase
	xref	_UnitData
	CODE


    ifd DEBUG
	    ; debug
	xdef	SendByte
	xdef	SendLong
	xdef	SendShort
	xdef	SetXmitTail
    endc


    ; MidiUnit stuff

NByteHooks equ 2    ; number of Byte hooks per unit

    STRUCTURE MidiUnit,0
	STRUCT	mu_UnitLock,SS_SIZE
	UBYTE	mu_UnitNum	    ; index of unit in MidiUnit array.	used when given a pointer and need unit #.
	UBYTE	mu_Flags	    ; status flags

	APTR	mu_UnitTask	    ; this unit's task
	APTR	mu_UnitTaskName     ; allocated name for task
	ULONG	mu_UnitRecvSigMask  ; queue active signal mask for this unit
	BYTE	mu_UnitQuitSigBit   ; shutdown signal bit for this unit
	BYTE	mu_UnitOpenCount    ; 
	UWORD	mu_pad1    	    ; long word alignment

	APTR	mu_XmitQueue
	APTR	mu_XmitQueueEnd
	APTR	mu_XmitQueueHead    ; next filled byte in xmit queue
	APTR	mu_XmitQueueTail    ; next available byte in xmit queue
	ULONG	mu_XmitQueueSize    ; size of queue in bytes (includes overflow pad)

	APTR	mu_RecvQueue
	APTR	mu_RecvQueueEnd
	APTR	mu_RecvQueueHead
	APTR	mu_RecvQueueTail
	ULONG	mu_RecvQueueSize    ; size of queue in words (includes overflow pad)

	APTR	mu_InLink	    ; link to input MidiCluster
	APTR	mu_OutLink	    ; link to output MidiCluster
    APTR	mu_InName		; allocated name for input MidiCluster
    APTR	mu_OutName		; allocated name for output MidiCluster

	APTR	mu_ParserData

	ULONG	mu_OutPortFilter

	UBYTE	mu_RunningStatus    ; running status.  0 if not set.
	BYTE	mu_RunningCount     ; number of remaining on running status before reset.

	UBYTE	mu_ErrFlags	    ; error flags (CMEF_)

	UBYTE	mu_MidiDevicePort
	APTR	mu_MidiDeviceData
	FPTR	mu_ActivateXmit

	APTR	mu_MidiUnitDef

	LABEL	MidiUnit_Size


MaxRunningCount equ 9		    ; Max times a running status value is used

	BITDEF	MU,SerOpen,0	    ; serial open
	BITDEF	MU,XmitActive,1     ; transmit interrupt active


    ; UnitData structure

    STRUCTURE UnitData,0
	STRUCT	ud_UnitsLock,SS_SIZE
	UBYTE	ud_NUnits
	UBYTE	ud_pad0 	    ; word alignment

	APTR	ud_MidiUnit
	ULONG	ud_MidiUnitSize

	APTR	ud_MidiPrefs
	ULONG	ud_MidiPrefsSize
	    ; other stuff here, no UnitData_Size


; common registers

unitr	equr	a2	; A2 = MidiUnit pointer
mir	equr	a2	; A2 = MidiNode pointer


****************************************************************
*
*   Single Unit locking macros
*
*   FUNCTION
*	Lock/Unlock a unit.  Calls Obtain/ReleaseSemaphore().
*
*   INPUTS
*	A2 - MidiUnit
*	A5 - CamdBase
*	A6 - SysBase
*
*   RESULTS
*	None
*
****************************************************************

LockUnit macro
	lea.l	mu_UnitLock(unitr),a0
	JSRLIB	ObtainSemaphore
	endm

UnlockUnit macro
	lea.l	mu_UnitLock(unitr),a0
	JSRLIB	ReleaseSemaphore
	endm



****************************************************************
*
*   Pause (macro)
*
*   SYNOPSIS
*	Pause [regmask]
*
*   FUNCTION
*	Delay transmission.  Used to wait until xmit queue can
*	accept more bytes.
*
*   INPUTS
*	regmask - optional set of registers to preserve around
*		  pause call.  Should contain only a subset
*		  of D0-D1/A0-A1.
*	A5	- CamdBase
*
*   RESULTS
*	None
*
*   NOTE !!!
*	Use of this macro delays the caller.  This can cause
*	things like the Unit Task to delay message distribution
*	to everyone else until a given transmit queue is
*	sufficiently clear.
*
*	The current implementation delays for 20ms.
*
****************************************************************

Pause	macro

      ifgt narg
	movem.l \1/a6,-(sp)
      else
	move.l	a6,-(sp)
      endc

	move.l	camd_DOSBase(a5),a6
	moveq	#1,d1
	JSRLIB	Delay

      ifgt narg
	movem.l (sp)+,\1/a6
      else
	move.l	(sp)+,a6
      endc

	endm


****************************************************************
*
*   UnitTaskSegment
*
*   Used to CreateProc() UnitTask
*
****************************************************************

	; !!! drop this if camd becomes 2.0 only!

	cnop 0,4	; long word align (assumes linker is long word aligning code in each module)

	dc.l	uts_end-*	    ; segment length
_UnitTaskSegment
	dc.l	0		    ; next segment BPTR
	jmp	_UnitTask	    ; beginning of code for segment
uts_end


***************************************************************
*
*   OutputFunc
*
*   FUNCTION
*	Writes a MidiMsg to a hardware unit.
*
*   INPUTS
*	a1 - MidiMsg
*	a2 - MidiLink
*	a3 - SysEx Source Buffer
*	d0 - SysEx Length
*	A5 - CamdBase (from PostMsg)
*	A6 - SysBase (from PostMsg)
*
*   RESULTS
*	none
*
***************************************************************

_OutputFunc
	Push	a3/a4/d2-d4

	move.l	d0,d3			    ; save possible SysEx info
	move.l	a3,d4

	move.l	a2,a3
	move.l	a1,a4
	move.l	ml_UserData(a3),unitr	    ; unitr (a2) = unit pointer

	LockUnit

	tst.l	mu_UnitTask(unitr)	    ; is unit active?
	beq.s	outf_done		    ; nope, so don't send

	move.b	mm_Status(a4),d0            ; d0 = status byte
	bsr	MidiMsgLen		    ; d0 = length (w/ upper bits cleared)

	lea	mm_Msg(a4),a0               ; a0 = msg pointer
	move.b	(a0),d1                     ; d1 = status byte

	btst.b	#MUB_XmitActive,mu_Flags(unitr) ; is the queue active?
	beq.s	outf_norunstat		    ; if not, don't bother doing running status

	cmp.b	mu_RunningStatus(unitr),d1  ; status byte match running status?
	beq.s	outf_runstat

outf_norunstat
	cmp.b	#MS_System,d1		    ; a system message?
	bcs.s	outf_nonsystem
	clr.b	mu_RunningStatus(unitr)     ; running status isn't valid for system msg's
	bra.s	outf_chkmsg

outf_nonsystem
	move.b	d1,mu_RunningStatus(unitr)  ; set running status
	move.b	#MaxRunningCount,mu_RunningCount(unitr) ; set running count
	bra.s	outf_chkmsg

outf_runstat
	subq.b	#1,mu_RunningCount(unitr)   ; decrement running count
	bne.s	outf_keeprunning
	clr.b	mu_RunningStatus(unitr)     ; running count expired, clear running status
outf_keeprunning
	addq.w	#1,a0			    ; skip status byte
	subq.w	#1,d0			    ; subtract 1 from length

outf_chkmsg
	cmp.b	#MS_SysEx,d1		    ; a SysEx message?
	beq.s	outf_sysex

	bsr	SendShort

outf_done
	UnlockUnit
	move.l	a3,a2
	Pop
	rts

outf_sysex
	move.l	d4,a0
	move.l	d3,d0
	bsr	SendLong

	bra	outf_done


****************************************************************
*
*   SendRTToUnits
*
*   FUNCTION
*	Send a real time message to units.
*
*   INPUTS
*	A4 - pointer to MidiMsg
*	A5 - CamdBase
*	A6 - SysBase
*
*   RESULTS
*	None
*
****************************************************************

SendRTToUnits
	Push	unitr/d2

	lea	_UnitData+ud_MidiUnit,unitr ; unitr = unit pointer
	CLEAR	d2
	move.b	_UnitData+ud_NUnits,d2	    ; D2 = unit count
	subq.w	#1,d2
	bcs.s	srt_ret

srt_loop	    ; loop on MidiUnits
	move.b	mm_Port(a4),d0              ; d0 = port #
	move.l	mu_OutPortFilter(unitr),d1  ; d1 = PortFilter for Unit
	btst.l	d0,d1
	beq.s	srt_skip

	LockUnit
	move.b	mm_Status(a4),d0
	bsr	SendByte
	UnlockUnit

srt_skip
	lea	MidiUnit_Size(unitr),unitr  ; next unit pointer
	dbra	d2,srt_loop

srt_ret
	Pop
	rts


****************************************************************
*
*   SendMsgToUnits
*
*   FUNCTION
*	Send a MidiMsg to units.
*
*   INPUTS
*	A4 - pointer to MidiMsg
*	A5 - CamdBase
*	A6 - SysBase
*
*   RESULTS
*	None
*
****************************************************************

SendMsgToUnits
	Push	unitr/d2

	lea	_UnitData+ud_MidiUnit,unitr ; unitr = unit pointer
	CLEAR	d2
	move.b	_UnitData+ud_NUnits,d2	    ; d2 = unit count
	subq.w	#1,d2
	bcs.s	smsg_ret

smsg_unitloop	    ; loop on MidiUnits
	move.b	mm_Port(a4),d0              ; d0 = port #
	move.l	mu_OutPortFilter(unitr),d1  ; d1 = PortFilter for Unit
	btst.l	d0,d1
	beq.s	smsg_skip

	LockUnit

	move.b	mm_Status(a4),d0            ; d0 = status byte
	bsr	MidiMsgLen		    ; d0 = length (w/ upper bits cleared)

	lea	mm_Msg(a4),a0               ; a0 = msg pointer
	move.b	(a0),d1                     ; d1 = status byte

	btst.b	#MUB_XmitActive,mu_Flags(unitr) ; is the queue active?
	beq.s	smsg_norunstat		    ; if not, don't bother doing running status

	cmp.b	mu_RunningStatus(unitr),d1  ; status byte match running status?
	beq.s	smsg_runstat

smsg_norunstat
	cmp.b	#MS_System,d1		    ; a system message?
	bcs.s	smsg_nonsystem
	clr.b	mu_RunningStatus(unitr)     ; running status isn't valid for system msg's
	bra.s	smsg_sendmsg

smsg_nonsystem
	move.b	d1,mu_RunningStatus(unitr)  ; set running status
	move.b	#MaxRunningCount,mu_RunningCount(unitr) ; set running count
	bra.s	smsg_sendmsg

smsg_runstat
	subq.b	#1,mu_RunningCount(unitr)   ; decrement running count
	bne.s	smsg_keeprunning
	clr.b	mu_RunningStatus(unitr)     ; running count expired, clear running status
smsg_keeprunning
	addq.w	#1,a0			    ; skip status byte
	subq.w	#1,d0			    ; subtract 1 from length

smsg_sendmsg
	bsr	SendShort
	UnlockUnit

smsg_skip
	lea	MidiUnit_Size(unitr),unitr  ; next unit pointer
	dbra	d2,smsg_unitloop

smsg_ret
	Pop
	rts



****************************************************************
*
*   SendSysExToUnits
*
*   FUNCTION
*	Send a Sys/Ex Msg to units.
*
*   INPUTS
*	A3 - pointer to sys/ex
*	D3 - length of sys/ex
*	A4 - MidiMsg (for mm_Port only)
*	A5 - CamdBase
*	A6 - SysBase
*
*   RESULTS
*	None
*
****************************************************************

SendSysExToUnits
	Push	unitr/d2

	lea	_UnitData+ud_MidiUnit,unitr ; unitr = unit pointer
	CLEAR	d2
	move.b	_UnitData+ud_NUnits,d2	    ; d2 = unit count
	subq.w	#1,d2
	bcs.s	ssx_ret

ssx_unitloop	    ; loop on MidiUnits
	move.b	mm_Port(a4),d0              ; d0 = port #
	move.l	mu_OutPortFilter(unitr),d1  ; d1 = PortFilter for Unit
	btst.l	d0,d1
	beq.s	ssx_skip

	LockUnit

	clr.b	mu_RunningStatus(unitr)     ; clear running status (sys/ex doesn't do running status)
	move.l	a3,a0			    ; A0 = src
	move.l	d3,d0			    ; D0 = length
	bsr	SendLong		    ; send bytes

	UnlockUnit

ssx_skip
	lea	MidiUnit_Size(unitr),unitr  ; next unit pointer
	dbra	d2,ssx_unitloop

ssx_ret
	Pop
	rts



****************************************************************
*
*   SendSXListToUnits
*
*   FUNCTION
*	Send an SXList to units.
*
*   INPUTS
*	A3 - ParserData
*	A5 - CamdBase
*	A6 - SysBase
*
*   RESULTS
*	None
*
****************************************************************

SendSXListToUnits
	Push	unitr/d2/d4/a4

	lea	_UnitData+ud_MidiUnit,unitr ; unitr = unit pointer
	CLEAR	d2
	move.b	_UnitData+ud_NUnits,d2	    ; d2 = unit count
	subq.w	#1,d2
	bcs.s	ssxl_ret

ssxl_unitloop	     ; loop on MidiUnits
	move.b	pd_CurMsg+mm_Port(a3),d0    ; d0 = port #
	move.l	mu_OutPortFilter(unitr),d1  ; d1 = PortFilter for Unit
	btst.l	d0,d1
	beq.s	ssxl_skip

	LockUnit

	clr.b	mu_RunningStatus(unitr)     ; clear running status (sys/ex doesn't do running status)

	move.w	pd_NSXNodes(a3),d4          ; d4 = node count
	beq.s	ssxl_lastbuf

	lea	pd_SXShortBuffer(a3),a0     ; A0 = src
	move.l	#PDSX_ShortBufSize,d0	    ; D0 = length
	bsr	SendLong		    ; send bytes

	subq.w	#1,d4			    ; d4-- (count-1)
	move.l	pd_SXList(a3),a4            ; walk SXList, A4 = SXNode
	bra.s	ssxl_sxnloopend

ssxl_sxnloop		; loop on SXNodes
	lea	psxn_Buffer(a4),a0          ; A0 = src
	move.l	#PDSX_LongBufSize,d0	    ; D0 = length
	bsr	SendLong		    ; send bytes
	move.l	(a4),a4
ssxl_sxnloopend
	dbra	d4,ssxl_sxnloop

ssxl_lastbuf
	move.l	pd_SXBuff(a3),a0            ; A0 = src
	move.l	pd_SXBCur(a3),d0
	sub.l	a0,d0			    ; D0 = length
	bsr	SendLong		    ; send bytes

	UnlockUnit

ssxl_skip
	lea	MidiUnit_Size(unitr),unitr  ; next unit pointer
	dbra	d2,ssxl_unitloop

ssxl_ret
	Pop
	rts



****************************************************************
*
*   SendByte
*
*   FUNCTION
*	Send one byte.	If queue isn't currently active, the
*	byte is sent to the hardware.  Otherwise, the byte
*	is added to the transmit queue.
*
*   INPUTS
*	D0.b - byte
*	A2 - unit pointer (unitr)
*	A5 - CamdBase
*	A6 - SysBase
*
*   RESULTS
*	None
*
*   NOTE
*	Assumes caller has Unit Lock held.
*
****************************************************************

SendByte
	move.l	mu_XmitQueueTail(unitr),a1  ; a1 = buffer tail
	move.b	d0,(a1)+                    ; put byte in transmit queue

	cmp.l	mu_XmitQueueEnd(unitr),a1   ; wrap around?
	bcs.s	sb_nowrap
	move.l	mu_XmitQueue(unitr),a1
sb_nowrap

sb_retry
	cmp.l	mu_XmitQueueHead(unitr),a1  ; is the buffer about to overflow?
	bne	SetXmitTail
	Pause	a1			    ; save a1
	bra	sb_retry



****************************************************************
*
*   PostByte (macro)
*
*   SYNOPSIS
*	PostByte
*
*   FUNCTION
*	Move a byte to a unit's transmit queue.  Called by
*	SendShort.
*
*   INPUTS
*	A0 - src buffer pointer
*	A1 - dst buffer pointer
*	D1 - dst buffer end (wrap around point)
*	unitr - pointer to unit.
*
*   RESULTS
*	A0 & A1 are incremented to next byte.  A1 is wrapped
*	if necessary.
*
****************************************************************

PostByte macro
	move.b	(a0)+,(a1)+
	cmp.l	d1,a1			    ; wrap around?
	bcs.s	pb\@_nowrap
	move.l	mu_XmitQueue(unitr),a1
pb\@_nowrap
	endm


****************************************************************
*
*   SendShort
*
*   FUNCTION
*	Send 1, 2, or 3 bytes.
*
*   INPUTS
*	D0 - length of 1, 2, or 3 (assumes sign extension to 32 bits)
*	A0 - pointer to bytes to send
*	A2 - unit pointer (unitr)
*	A5 - CamdBase
*	A6 - SysBase
*
*   RESULTS
*	None
*
*   NOTE
*	Assumes caller has Unit Lock held.
*
****************************************************************

SendShort

ss_retry
	move.l	mu_XmitQueueHead(unitr),d1  ; D1 = head-tail
	sub.l	mu_XmitQueueTail(unitr),d1
	bhi.s	ss_ovwrap
	add.l	mu_XmitQueueSize(unitr),d1  ; if occupied part of buffer doesn't wrap
					    ; complement by buffer size (end-tail+head-buff)
ss_ovwrap
	cmp.l	d0,d1
	bls.s	ss_pause		    ; if avail <= len, pause (takes pad byte into account)

	move.l	mu_XmitQueueTail(unitr),a1  ; a1 = buffer tail
	move.l	mu_XmitQueueEnd(unitr),d1   ; d1 = buffer end

	subq.w	#2,d0			    ; branch to appropriate # of posts
	bmi.s	ss_post1
	beq.s	ss_post2
ss_post3	    ; post 3 bytes
	PostByte
ss_post2	    ; post 2 bytes
	PostByte
ss_post1	    ; post 1 byte
	PostByte
	bra	SetXmitTail

ss_pause
	Pause	d0/a0
	bra	ss_retry



MinBulkCopy equ 64	; min number of bytes to bulk copy

****************************************************************
*
*   SendLong
*
*   FUNCTION
*	Send a lot of bytes.  This will try to optimize by
*	copying as much as possible into the Xmit queue at
*	a time.  If the xmit queue gets full, Pause is called
*	and the process is repeated until all bytes have
*	been sent.
*
*   INPUTS
*	D0 - src buffer length
*	A0 - src buffer
*	A2 - unit pointer (unitr)
*	A5 - CamdBase
*	A6 - SysBase
*
*   RESULTS
*	None
*
*   NOTE
*	Assumes caller has Unit Lock held.
*
****************************************************************

SendLong
	Push	d2-d3/a3
	move.l	a0,a3			    ; A3 = src
	move.l	d0,d3			    ; D3 = length

sl_loop
	move.l	mu_XmitQueueHead(unitr),d1  ; d1 = head-tail
	sub.l	mu_XmitQueueTail(unitr),d1
	bhi.s	sl_ovwrap
	add.l	mu_XmitQueueSize(unitr),d1  ; if occupied part of buffer doesn't wrap
					    ; complement by buffer size (end-tail+head-buff)
sl_ovwrap
	subq.l	#1,d1			    ; subtract overflow pad byte from remaining space
					    ; D1 = avail bytes in queue

	cmp.l	d1,d3			    ; if len <= avail, move all remaining source bytes into queue
	bhi.s	sl_checkavail		    ; (len > avail, skip)
	move.l	d3,d0			    ; D0 = remaining source bytes
	bra.s	sl_move

sl_checkavail
	moveq	#MinBulkCopy,d0
	cmp.l	d0,d1			    ; if avail >= MinBulkCopy, fill xmit queue
	bcs.s	sl_pause		    ; (avail < min, skip)
	move.l	d1,d0			    ; D0 = available bytes in xmit queue

sl_move 				    ; D0 = bytes to move
	sub.l	d0,d3			    ; update remaining byte count
	move.l	a3,a0			    ; A0 = src buffer
	move.l	mu_XmitQueueTail(unitr),a1  ; A1 = dest buffer
	move.l	mu_XmitQueueEnd(unitr),d1
	sub.l	a1,d1			    ; D1 = bytes before wrap
	move.l	d0,d2
	sub.l	d1,d2			    ; D2 = bytes after wrap
	bls.s	sl_copysimple		    ; if <= 0, simple copy

		    ; fragmented copy
	move.l	d1,d0
	CopyMem

	move.l	mu_XmitQueue(unitr),a1
	move.l	d2,d0
sl_copysimple	   ; simple copy
	CopyMem

	move.l	a0,a3			    ; save src pointer
	cmp.l	mu_XmitQueueEnd(unitr),a1   ; normalize dst pointer
	bcs.s	sl_nowraptail
	move.l	mu_XmitQueue(unitr),a1
sl_nowraptail

	bsr.s	SetXmitTail		    ; post xmit tail
	tst.l	d3
	bne	sl_loop

	Pop
	rts

sl_pause
	Pause
	bra	sl_loop


****************************************************************
*
*   SetXmitTail
*
*   FUNCTION
*	Updates the XmitQueueTail value for a unit.  Used
*	when bytes have just been written to the queue.  This
*	function starts transmission if it is idle by calling
*	(*ActivateXmit)() for the unit.
*
*   INPUTS
*	A1 - new XmitQueueTail value
*	A2 - unit pointer (unitr)
*	A5 - CamdBase
*	A6 - SysBase
*
*   RESULTS
*	None
*
*   NOTE
*	Assumes caller has Unit Lock held.
*
*   THEORY
*	This method has the advantage that Disable() never
*	needs to be called to keep it in sync with the serial
*	RecvHandler.  Moving the BTST (BSET?) above the MOVE
*	instruction breaks this certainty because the queue
*	could go inactive between the execution of the BSET
*	and the MOVE.
*
****************************************************************

SetXmitTail
	move.l	a1,mu_XmitQueueTail(unitr)  ; store queue pointer in MidiUnit

	btst.b	#MUB_XmitActive,mu_Flags(unitr) ; test queue active bit
	bne.s	sxt_ret 		    ; if already set, return

				    ; not active
	cmp.l	mu_XmitQueueHead(unitr),a1  ; is the buffer empty?
	beq.s	sxt_ret 		    ; if so, return

				    ; activate
	bset.b	#MUB_XmitActive,mu_Flags(unitr) ; set queue active bit
	move.l	mu_ActivateXmit(unitr),a0   ; a0 = ActivateXmit function
	jmp	(a0)                        ; call driver to activate xmit interrupt
					    ; A2 = unit data pointer.  might be handy for driver
sxt_ret
	rts


****************************************************************
*
*   XmitHandler
*
*   FUNCTION
*	General reentrant serial transmit handler.  Assumes that
*	there is a byte in the queue waiting for transmission.
*	Otherwise this function should not get called.
*
*   INPUTS
*	A2 - MidiUnit pointer (unitr)
*
*   RESULTS
*	D0 - byte to be sent.  Upper 32 bits are cleared.
*	D1.b - TRUE if last byte, FALSE otherwise.
*
****************************************************************

; !!! optimize and adjust timing notes

; timing:
;    empty:  82
;   active: 130 max

XmitHandler
	move.l	mu_XmitQueueHead(unitr),a0  ; a0 = buffer head

	CLEAR	d0			    ; clear upper bits of D0 for return
	move.b	(a0)+,d0                    ; get byte

	cmp.l	mu_XmitQueueEnd(unitr),a0
	bcs.s	xh_nowrap
	move.l	mu_XmitQueue(unitr),a0
xh_nowrap
	move.l	a0,mu_XmitQueueHead(unitr)
	cmp.l	mu_XmitQueueTail(unitr),a0  ; is this the last byte?
	seq	d1			    ; D1 is non-zero if last byte
	bne.s	xh_done
	bclr	#MUB_XmitActive,mu_Flags(unitr)
xh_done
	rts


****************************************************************
*
*   CallByteHooks (macro)
*
*   SYNOPSIS
*	CallByteHooks hook,count
*
*   FUNCTION
*	Calls all hooks in the ByteHook array for a unit.
*	Called by RecvHandler.
*
*   INPUTS
*	hook - mu_ByteHook
*	count - NByteHooks
*	D0.b - unit number
*	D2.w - received data
*	A2 - MidiUnit pointer (unitr)
*	A6 - SysBase
*
*   RESULTS
*	None
*
****************************************************************

    ifgt 0
CallByteHooks macro

count\@ set \2
hook\@	set \1

	ifle count\@
	mexit
	endc

	move.l	hook\@+hte_hook(unitr),d0
	beq.s	skiphook\@

	Push	a2/a5

	move.l	d0,a0			; set Hook pointer

	    ; push MidiByteHookMsg onto stack
	move.w	d2,-(sp)
	clr.w	-(sp)
	move.b	mu_UnitNum(unitr),(sp)
	moveq	#MBHM_Byte,d0
	move.l	d0,-(sp)

	move.l	sp,a1			; set Message pointer
	move.l	hook\@+hte_mi(unitr),a2 ; set Object pointer (zaps unitr)

	move.l	h_Entry(a0),a5
	jsr	(a5)
	lea	MidiByteHookMsg_Size(sp),sp ; pop MidiTickHookMsg
	Pop

skiphook\@

	CallByteHooks hook\@+HookTblEntry_Size,count\@-1

	endm
    endc

****************************************************************
*
*   RecvHandler
*
*   FUNCTION
*	General reentrant serial receive handler.  This places
*	received bytes into a unit's receive queue and handles
*	signalling the unit task.
*
*   INPUTS
*	D0.w - bits 0-7 are received byte, bit 15 indicates a
*	       receiver error.	Bits 8-14 are don't care
*	       (i.e. they may contain miscellaneous junk from
*	       a hardware register)
*	A2 - MidiUnit pointer (unitr)
*
*   RESULTS
*	None
*
****************************************************************

; timing:
;	min: 336 (47us)
;  buf wrap:  14
;    signal:  56 + exec.lib/Signal()
;  bytehook:  46 + hook execution
    ; !!! update hook timing

RecvHandler
	Push	d2/a5-a6

	move.l	_CamdBase,a5		    ; A5 = CamdBase
	move.l	camd_SysBase(a5),a6         ; A6 = SysBase

	move.w	d0,d2			    ; D2 = recv'd byte + error flags
	move.l	mu_RecvQueueTail(unitr),a0  ; a0 = buffer tail
	move.l	a0,a1			    ; save original buffer tail in a1 for empty check below

	clr.b	(a0)+       				; zero to upper byte of word
;	move.b	camd_Time+3(a5),(a0)+       ; low 8 bits of time to upper byte of word
	move.b	d2,(a0)+                    ; midi byte to lower byte of word

	cmp.l	mu_RecvQueueEnd(unitr),a0
	bcs.s	rh_nowrap
	move.l	mu_RecvQueue(unitr),a0
rh_nowrap

	cmp.l	mu_RecvQueueHead(unitr),a0  ; is the buffer about to overflow?
	bne.s	rh_settail

	bset	#CMEB_RecvOverflow,mu_ErrFlags(unitr)
	bra.s	rh_signal

rh_settail
	move.l	a0,mu_RecvQueueTail(unitr)

	tst.w	d2			    ; check for receiver error
	bpl.s	rh_noerr
	bset	#CMEB_RecvErr,mu_ErrFlags(unitr)
	bra.s	rh_signal

rh_noerr
	cmp.l	mu_RecvQueueHead(unitr),a1  ; was buffer empty?
	bne.s	rh_nosignal

rh_signal
	move.l	mu_UnitRecvSigMask(unitr),d0
	move.l	mu_UnitTask(unitr),a1
	JSRLIB	Signal
rh_nosignal

    ifgt 0
	CallByteHooks mu_ByteHookTbl,NByteHooks
    endc

rh_ret
	Pop
	rts



****************************************************************
*
*   OpenMDPort
*
*   SYNOPSIS
*	struct MidiPortData *OpenMDPort (struct MidiUnit *unit)
*						a0
*
*   FUNCTION
*	Open the MIDI Device port for a unit.
*
*   INPUTS
*	unit (a0) - pointer to MidiUnit structure.  Assumes
*		    unit->MidiDeviceData != NULL.
*
*   RESULTS
*	Pointer to MidiPortData structure if successful, NULL
*	otherwise.
*
****************************************************************

_OpenMDPort	; C addressable symbol
	Push	a2/a3
	move.l	a0,a2			    ; a2 = MidiUnit
	move.b	mu_MidiDevicePort(a2),d0    ; d0.b = port #
	lea	XmitHandler,a0
	lea	RecvHandler,a1

	move.l	mu_MidiDeviceData(a2),a3    ; call OpenPort function
	move.l	mdd_OpenPort(a3),a3
	jsr	(a3)
	Pop
	rts


****************************************************************
*
*   CloseMDPort
*
*   SYNOPSIS
*	void CloseMDPort (struct MidiUnit *unit)
*				   a0
*
*   FUNCTION
*	Close the MIDI Device port for a unit.
*
*   INPUTS
*	unit (a0) - pointer to MidiUnit structure.  Assumes
*		    unit->MidiDeviceData != NULL.
*
*   RESULTS
*	None
*
****************************************************************

_CloseMDPort	; C addressable symbol
	move.b	mu_MidiDevicePort(a0),d0    ; d0.b = port #

	move.l	mu_MidiDeviceData(a0),a0    ; jump to ClosePort function
	move.l	mdd_ClosePort(a0),a0
	jmp	(a0)


****************************************************************
*
*   UnitTaskLoop
*
*   SYNOPSIS
*	void UnitTaskLoop (struct MidiUnit *unit)
*				     a0
*
*   FUNCTION
*	Service of the given unit's receive queue.  Calls
*	ParseMidiTime() with each received byte.
*
*   INPUTS
*	None
*
*   RESULTS
*	None
*
****************************************************************

;   while (!(Wait(recv | quit) & quit)) {
;	if (error) SendErrToPort()
;	while (GetByte (unit,&buf))
;	    ParseMidiTime (parser,buf)
;   }

; !!! might want to roll the GetByte() stuff into the parser
;     instead of putting it here!

utl_qpr       equr a3	    ; RecvQueue pointer
utl_pdr       equr a4	    ; cached mu_ParserData
utl_quitbitr  equr d2	    ; cached mu_UnitQuitSigBit
utl_waitmaskr equr d3	    ; Wait() mask

_UnitTaskLoop	    ; C addressable symbol
	Push	utl_qpr/utl_pdr/utl_quitbitr/utl_waitmaskr/unitr/a5-a6

	move.l	a0,unitr		    ; unitr = pointer to MidiUnit
	move.l	_CamdBase,a5		    ; A5 = CamdBase
	move.l	camd_SysBase(a5),a6         ; A6 = SysBase

	move.b	mu_UnitQuitSigBit(unitr),utl_quitbitr
	move.l	mu_UnitRecvSigMask(unitr),utl_waitmaskr
	bset.l	utl_quitbitr,utl_waitmaskr

	move.l	mu_RecvQueueHead(unitr),utl_qpr ; init utl_qpr
	move.l	mu_ParserData(unitr),utl_pdr    ; init utl_pdr

utl_waitloop
	move.l	utl_waitmaskr,d0
	JSRLIB	Wait
	btst.l	utl_quitbitr,d0 	    ; if quit signal bit, exit
	bne.s	utl_done

		    ; check for recv errors
	move.b	mu_ErrFlags(unitr),d1       ; read current error bits
	beq.s	utl_checkrecv

	eor.b	d1,mu_ErrFlags(unitr)       ; clear error bits we just read

;	move.b	pd_CurMsg+mm_Port(utl_pdr),d0
;;	bsr	SendErrToMI		    ; sent to hardware MidiNode???

	bra.s	utl_checkrecv


	    ; check Unit's recv queue (entry point is utl_checkrecv)
utl_recvloop
	move.w	(utl_qpr)+,d0               ; get MIDI byte + time stamp from queue

	cmp.l	mu_RecvQueueEnd(unitr),utl_qpr
	bcs.s	utl_nowrap
	move.l	mu_RecvQueue(unitr),utl_qpr
utl_nowrap
	move.l	utl_qpr,mu_RecvQueueHead(unitr)

	move.l	utl_pdr,a0		    ; a0 = ParserData
	move.l	mu_InLink(unitr),a1	    ; a1 = MidiLink
	bsr	ParseMidiTime		    ; call parser

utl_checkrecv
	cmp.l	mu_RecvQueueTail(unitr),utl_qpr ; is buffer empty?
	bne	utl_recvloop

	bra	utl_waitloop

utl_done
	Pop
	rts


	end
