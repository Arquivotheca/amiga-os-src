************************************************************************
*			     C. A. M. D.			       *
************************************************************************
* CMU Amiga Midi Driver - Carnegie Mellon University		       *
* 1988			- Commodore Amiga			       *
*								       *
* Design & Development	- Roger B. Dannenberg			       *
*			- Jean-Christophe Dhellemmes		       *
*			- Bill Barton				       *
* Copyright 1989 Carnegie Mellon University			       *
************************************************************************
*
* internal.asm - Example CAMD MIDI Device Driver for internal serial
*		 port.
*
* This is merely an example.  The internal serial port is normally
* managed by camd.library.
*
************************************************************************
* Date	      | Change
*-----------------------------------------------------------------------
* 06-Jan-1990 : Created (BB)
************************************************************************

      nolist
	include "exec/types.i"
	include "exec/execbase.i"           ; for FlushDevice()
	include "exec/macros.i"
	include "hardware/cia.i"
	include "hardware/custom.i"
	include "hardware/intbits.i"
	include "midi/devices.i"
	include "resources/misc.i"
      list


Version  equ 0
Revision equ 0
Ports	 equ 1


	section driver,data

	    ; external
	xref	_AbsExecBase
	xref	_custom
	xref	_ciab


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
	dc.b	0		; mdd_Flags

Name	    dc.b    'Internal',0
IDString    dc.b    'Internal serial port MIDI device driver',0
	    ds.w    0	; force word alignment


****************************************************************
*
*   Serial Port Stuff
*
****************************************************************

SerFreq_NTSC equ    3579545
SerFreq_PAL equ     3546895

MIDIBPS     equ     31250

SetSerPer   macro   ; freq
	move.w	#(\1+MIDIBPS/2)/MIDIBPS-1,SerPer    ; bits 0-14: period.  bit 15: 9/8 bit selection
	endm


INTF_CLR    equ 0
	BITDEF	INT,SET,INTB_SETCLR

	    ; serdatr register bit def's (not found in any include file)

	BITDEF	SERDAT,OVRUN,15
	BITDEF	SERDAT,RBF,14
	BITDEF	SERDAT,TBE,13
	BITDEF	SERDAT,TSRE,12

    STRUCTURE SerInt,IS_SIZE
	UWORD	si_pad0
	APTR	si_OldInt
	APTR	si_Data
	APTR	si_Code
	LABEL	SerInt_Size


	section driver,data	    ; data

MiscBase    ds.l    1		    ; misc.resource pointer
SerPer	    ds.w    1		    ; serper value
OldIntEna   ds.w    1		    ; original value of TBE and RBF bits

MPD0 ; struct MidiPortData
	    dc.l    ActivateXmit

SerXmitInt
	    dc.l    0		    ; ln_Succ
	    dc.l    0		    ; ln_Pred
	    dc.b    NT_INTERRUPT    ; ln_Type
	    dc.b    0		    ; ln_Pri
	    dc.l    Name	    ; ln_Name
	    dc.l    SerXmitInt+si_Data ; is_Data
	    dc.l    XmitHandler     ; is_Code
	    dc.w    0		    ; si_pad0
	    dc.l    0		    ; si_OldInt
	    dc.l    0		    ; si_Data
	    dc.l    0		    ; si_Code

SerRecvInt
	    dc.l    0		    ; ln_Succ
	    dc.l    0		    ; ln_Pred
	    dc.b    NT_INTERRUPT    ; ln_Type
	    dc.b    0		    ; ln_Pri
	    dc.l    Name	    ; ln_Name
	    dc.l    SerRecvInt+si_Data ; is_Data
	    dc.l    RecvHandler     ; is_Code
	    dc.w    0		    ; si_pad0
	    dc.l    0		    ; si_OldInt
	    dc.l    0		    ; si_Data
	    dc.l    0		    ; si_Code

	section driver,code

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

init_reg reg a6

Init
	movem.l init_reg,-(sp)

	move.l	_AbsExecBase,a6 	    ; A6 = SysBase

	    ; get misc.resource pointer
	lea	MiscName,a1		    ; open misc.resource
	JSRLIB	OpenResource
	move.l	d0,MiscBase
	beq	init_ret

	    ; calculate SerPer
	cmp.b	#50,VBlankFrequency(a6)     ; is PAL?
	beq	init_pal
	SetSerPer SerFreq_NTSC		    ; set our local SerPer variable (not custom.serper)
	bra	init_serperdone
init_pal
	SetSerPer SerFreq_PAL		    ; set out local SerPer variable
init_serperdone

	moveq	#1,d0			    ; return TRUE
init_ret
	movem.l (sp)+,init_reg
	rts


****************************************************************
*
*   Expunge
*
*   FUNCTION
*	Gets called by CAMD immediately before being
*	UnLoadSeg'ed.
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
*	D0.b - Port number (should always be 0 for this driver)
*	A0 - Xmit function
*	A1 - Recv function
*	A2 - Data
*
*   RESULT
*	D0 - pointer to MidiPortData structure.
*
****************************************************************

op_reg	reg a6

OpenPort
	movem.l op_reg,-(sp)

	move.l	_AbsExecBase,a6 	    ; A6 = SysBase

	movem.l a0/a1,-(sp)                 ; save a0/a1
	bsr	AllocSerial
	movem.l (sp)+,a0/a1
	ext.l	d0
	beq	op_ret

		    ; set SerXmitInt
	move.l	a0,SerXmitInt+si_Code
	move.l	a2,SerXmitInt+si_Data

		    ; set SerRecvInt
	move.l	a1,SerRecvInt+si_Code
	move.l	a2,SerRecvInt+si_Data

	lea	_custom,a0		    ; A0 = custom
	move.w	#INTF_TBE!INTF_RBF,d1	    ; D1 = RBF | TBE mask and int disable

		    ; save original TBE and RBF intena settings
	move.w	intenar(a0),d0
	and.w	d1,d0			    ; mask with RBF|TBE mask
	move.w	d0,OldIntEna

	move.w	d1,intena(a0)               ; disable TBE and RBF interrupts
	move.w	d1,intreq(a0)               ; clear pending TBE and RBF interrupts
	move.w	SerPer,serper(a0)           ; set serper

		    ; set TBE interrupt vector
	moveq	#INTB_TBE,d0
	lea	SerXmitInt,a1
	JSRLIB	SetIntVector
	move.l	d0,SerXmitInt+si_OldInt     ; save old vector

		    ; set RBF interrupt vector
	moveq	#INTB_RBF,d0
	lea	SerRecvInt,a1
	JSRLIB	SetIntVector
	move.l	d0,SerRecvInt+si_OldInt     ; save old vector

	move.w	#INTF_SET!INTF_RBF,_custom+intena   ; enable RBF
	move.w	#INTF_SET!INTF_TBE,_custom+intreq   ; set a pending TBE
	bclr.b	#CIAB_COMDTR,_ciab+ciapra   ; turn on DTR bit (active low)

	lea	MPD0,a0
	move.l	a0,d0			    ; return pointer to MidiPortData

op_ret
	movem.l (sp)+,op_reg
	rts



****************************************************************
*
*   ClosePort
*
*   FUNCTION
*	Close a MIDI port.
*
*   INPUTS
*	D0.b - Port number (always 0 for this driver).
*
*   RESULT
*	None
*
****************************************************************

cp_reg	reg a6

ClosePort
	movem.l cp_reg,-(sp)

	move.l	_AbsExecBase,a6 	    ; A6 = SysBase

	bsr	WaitSerial

	bset.b	#CIAB_COMDTR,_ciab+ciapra	    ; turn off DTR bit (active low)
	move.w	#INTF_CLR!INTF_TBE!INTF_RBF,_custom+intena  ; disable RBF & TBE
	move.w	#INTF_CLR!INTF_TBE!INTF_RBF,_custom+intreq  ; clear pending RBF & TBE requests

		    ; restore original TBE interrupt vector
	moveq	#INTB_TBE,d0
	move.l	SerXmitInt+si_OldInt,a1
	JSRLIB	SetIntVector

		    ; restore original RBF interrupt vector
	moveq	#INTB_RBF,d0
	move.l	SerRecvInt+si_OldInt,a1
	JSRLIB	SetIntVector

	move.w	OldIntEna,d0
	beq	cp_nointena
	or.w	#INTF_SET,d0
	move.w	d0,_custom+intena
cp_nointena

	bsr	FreeSerial

	movem.l (sp)+,cp_reg
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

ActivateXmit
	move.w	#INTF_SET!INTF_TBE,_custom+intena   ; enable TBE interrupt, but leave request pending
	rts


****************************************************************
*
*   WaitSerial
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

ws_reg reg

WaitSerial
	movem.l ws_reg,-(sp)

ws_loop
	move.w	_custom+serdatr,d0
	btst.l	#SERDATB_TSRE,d0
	beq	ws_loop

	movem.l (sp)+,ws_reg
	rts



****************************************************************
*
*   Serial Interrupt Handlers
*
****************************************************************


****************************************************************
*
*   XmitHandler
*
*   FUNCTION
*	Amiga serial port transmit interrupt handler.  Calls
*	the general transmit handler and expects a byte in
*	D0 and a last byte flag in D1.
*
*	When the last byte is detected, the TBE interrupt
*	is disabled, thus deferring calling XmitHandler
*	again until there are some bytes ready to send.
*	CAMD calls ActivateXmit when there are more bytes
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
*   NOTE
*	A5 is scratch.
*
****************************************************************

sxh_reg reg a2

XmitHandler
	movem.l sxh_reg,-(sp)

	movem.l (a1),a2/a5                  ; A2 = Data, A5 = XmitHandler

sxh_loop
	move.w	#INTF_CLR!INTF_TBE,_custom+intreq   ; clear TBE interrupt request

	jsr	(a5)                        ; returns byte in D0, last byte flag in D1

	or.w	#$100,d0		    ; add stop bit
	move.w	d0,_custom+serdat	    ; send byte

	tst.b	d1			    ; is last byte?
	beq	sxh_notlast
					    ; xmit queue is empty
	move.w	#INTF_CLR!INTF_TBE,_custom+intena   ; disable TBE interrupt.
	bra	sxh_done		    ; next request remains pending until there are more bytes to send

sxh_notlast				    ; test for another TBE
	btst.b	#INTB_TBE&7,_custom+intreqr+(15-INTB_TBE)/8
	bne	sxh_loop

sxh_done
	movem.l (sp)+,sxh_reg
	rts



    if 0

****************************************************************
*
*   XmitHandler
*
*   FUNCTION
*	Amiga serial port transmit interrupt handler.  Called
*	by TBE interrupt vector.  Calls the CAMD transmit
*	handler and expects one of the following return values
*	in d0.w:
*
*	    $00mm - a MIDI byte in the lower byte to transmit.
*	    $ffff - if no byte to send (queue was empty).
*
*   INPUTS
*	A0 - custom
*	A1 - pointer to SerInt.Data
*	A6 - SysBase
*
*   RESULTS
*	None
*
*   NOTE
*	A5 is scratch.
*
****************************************************************

sxh_reg reg a2

XmitHandler
	movem.l sxh_reg,-(sp)

	movem.l (a1),a2/a5

sxh_loop
	move.w	#INTF_TBE,intreq(a0)    ; clear the interrupt request

	jsr	(a5)

	or.w	#$100,d0		; add stop bit
	bmi	sxh_skip		; if negative, there's no byte to send

	lea	_custom,a0		; see if TBE is still set
	move.w	d0,serdat(a0)           ; send byte
	move.w	intreqr(a0),d0          ; (don't know if btst.b is safe on custom chips)
	and.w	#INTF_TBE,d0			;	^^^ JOE: It's not!!
	bne	sxh_loop

sxh_skip
	movem.l (sp)+,sxh_reg
	rts

    endc


****************************************************************
*
*   RecvHandler
*
*   FUNCTION
*	Amiga serial port receive interrupt handler.  Called
*	by RBF interrupt vector.  Calls the CAMD receive
*	handler with received data until there are no more bytes
*	pending in the serial receive hardware.
*
*   INPUTS
*	A0 - custom
*	A1 - pointer to SerInt.Data
*	A6 - SysBase
*
*   RESULTS
*	None
*
*   NOTE
*	A5 is scratch.
*
****************************************************************

srh_reg reg a2

RecvHandler
	movem.l srh_reg,-(sp)

	movem.l (a1),a2/a5
	move.w	serdatr(a0),d0

srh_loop
	move.w	#INTF_CLR!INTF_RBF,intreq(a0)   ; clear the interrupt request

	jsr	(a5)

	lea	_custom,a0		; see if RBF is still set
	move.w	serdatr(a0),d0
	btst.l	#SERDATB_RBF,d0
	bne	srh_loop

	movem.l (sp)+,srh_reg
	rts



****************************************************************
*
*   misc.resource serial stuff
*
****************************************************************

****************************************************************
*
*   AllocSerial
*
*   FUNCTION
*	Allocate misc.resource channels for the serial port.
*	Call FlushDevice("serial.device") on failure and try
*	again.
*
*   INPUTS
*	A6 - SysBase
*
*   RESULT
*	D0.w - TRUE on success.
*
****************************************************************

as_reg	reg

AllocSerial
	movem.l as_reg,-(sp)

	JSRLIB	Forbid			    ; remain atomic

	bsr	AttemptSerMisc		    ; attempt to get serial misc.resource channels
	tst.w	d0
	bne	as_done 		    ; if OK, return

	lea	SerialName,a1		    ; otherwise, FlushDevice("serial.device")
	bsr	FlushDevice
	bsr	AttemptSerMisc		    ; and try the serial misc.resource stuff again

as_done
	move.w	d0,-(sp)
	JSRLIB	Permit
	move.w	(sp)+,d0

	movem.l (sp)+,as_reg
	rts


****************************************************************
*
*   AttemptSerMisc
*
*   FUNCTION
*	Attempt to allocate misc.resource channels for the
*	serial port.
*
*   INPUTS
*	None
*
*   RESULT
*	D0.w - TRUE on success.
*
****************************************************************

asm_reg reg a6

AttemptSerMisc
	movem.l asm_reg,-(sp)

	move.l	MiscBase,a6		    ; A6 = MiscBase

	moveq	#MR_SERIALBITS,d0	    ; alloc SERIALBITS
	lea	Name,a1
	JSRLIB	AllocMiscResource
	tst.l	d0			    ; if return is non-zero, it's in use
	bne	asm_fail

	moveq	#MR_SERIALPORT,d0	    ; alloc SERIALPORT
	lea	Name,a1
	JSRLIB	AllocMiscResource
	tst.l	d0			    ; if return is zero, we got it
	beq	asm_ok

	moveq	#MR_SERIALBITS,d0	    ; free SERIALBITS on SERIALPORT failure
	JSRLIB	FreeMiscResource

asm_fail
	moveq	#0,d0
	bra	asm_ret

asm_ok
	moveq	#1,d0

asm_ret
	movem.l (sp)+,asm_reg
	rts



****************************************************************
*
*   FreeSerial
*
*   FUNCTION
*	Free misc.resource channels for the serial port.
*
*   INPUTS
*	None
*
*   RESULT
*	None
*
****************************************************************

fs_reg	reg a6

FreeSerial
	movem.l fs_reg,-(sp)

	move.l	MiscBase,a6		    ; A6 = MiscBase

	moveq	#MR_SERIALBITS,d0	    ; free SERIALBITS
	JSRLIB	FreeMiscResource

	moveq	#MR_SERIALPORT,d0	    ; free SERIALPORT
	JSRLIB	FreeMiscResource

	movem.l (sp)+,fs_reg
	rts



****************************************************************
*
*   FlushDevice
*
*   FUNCTION
*	Force expunge of a Device.
*
*   INPUTS
*	A1 - Name of device.
*	A6 - SysBase
*
*   RESULT
*	None
*
*   NOTE
*	Assumes caller has surrounded this call with
*	Forbid/Permit
*
****************************************************************

fd_reg	reg

FlushDevice
	movem.l fd_reg,-(sp)

	lea	DeviceList(a6),a0
	JSRLIB	FindName
	tst.l	d0			    ; if not in the device list, skip
	beq	fd_ret

	move.l	d0,a1			    ; otherwise force expunge
	JSRLIB	RemDevice

fd_ret
	movem.l (sp)+,fd_reg
	rts


MiscName MISCNAME
SerialName dc.b 'serial.device',0

	end
