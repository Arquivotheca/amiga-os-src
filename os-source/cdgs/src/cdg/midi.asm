**
**	$Id: midi.asm,v 1.11 92/04/02 18:47:40 darren Exp $
**
**	CDTV CD+G -- midi.asm (midi specific code)
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

** Includes

		INCLUDE	"cdgbase.i"
		INCLUDE "cdgprefs.i"
		INCLUDE	"debug.i"

** Exports
		SECTION	cdg

		XDEF	CDGMidiQuiet
		XDEF	CDGAllNotesOff

		XDEF	CDGInitMidiOut
		XDEF	CDGFreeMidiOut

		XDEF	CDGMidiPack
**Imports

		XREF	cdgname

		XREF	_custom		;amiga.lib
		XREF	_intena
		XREF	_intenar
		XREF	_intreq
		XREF	_serdat
		XREF	_serdatr
		XREF	_ciab
** Equates

** Assumptions

** Notes

*****i* cdg.library/CDGAllNotesOff ************************************
*
*   NAME
*	CDGAllNotesOff -- Tell MIDI devices to turn off all notes
*   
*   SYNOPSIS
*	VOID CDGAllNotesOff( VOID );
*
*	VOID CDGAllNotesOff( VOID );
*
*   FUNCTION
*	Private function for telling MIDI devices to be quiet.
*	Takes library base pointer in A6, and preserves ALL registers.
*
*	This function is meant to be used when the CD has stopped,
*	paused, etc., so that we don't leave a note hanging.
*
*	This function checks if the disk is +MIDI, else it doesn't
*	bother sending anything (and not only must it be a +MIDI
*	disk, but we must have also obtained the resource serial
*	bits, and port).
*
*   RETURNS
*	N/A
*
*   NOTES
*	PRESERVES ALL REGISTERS - not a time critical function
*
***********************************************************************

**
** Private entry; takes library base in A6, fetches, and sends ALL notes
** off if, and only if this disk has been recognized as a +MIDI disk,
** and then only if we could obtain the resource bits
**

CDGAllNotesOff:

		move.l	a6,-(sp)

	; obtain pointer to data

		move.l	cdg_BaseData(a6),a6

	; check IF midi disk

		btst	#CDGB_MIDI,cdd_TypeLights(a6)
		beq.s	CDGNotesOff

	; check IF we have sent any midi data since last time this function
	; was called

		bclr	#EF2B_NEWMIDI,cdd_ExtraFlags2(a6)
		beq.s	CDGNotesOff

		movem.l	d0-d1/a0-a1,-(sp)
		bsr.s	CDGMidiQuiet
		movem.l	(sp)+,d0-d1/a0-a1

CDGNotesOff:
		move.l	(sp)+,a6
		rts

		
*****i* cdg.library/CDGMidiQuiet **************************************
*
*   NAME
*	CDGMidiQuiet -- Tell MIDI devices to be quiet
*   
*   SYNOPSIS
*	VOID CDGMidiQuiet( data );
*			    A6
*
*	VOID CDGMidiQuiet( struct CDGData * );
*
*   FUNCTION
*	Routine to get connected MIDI devices to be quiet.  MIDI
*	codes, and sequencing information borrowed from cdg.classic
*	CDGDISPATCH.ASM (and orginally based on information
*	provided by Scott Etherton -- see cdg.classic source code).
*
*
*   RETURNS
*	N/A
*
*   NOTES
*
***********************************************************************


	;;;
	;;; The following equates, and code is borrowed from cdg.classic,
	;;; based on information provided by Scott Etherton.
	;;;
	;;; However I've rewritten it to be more efficient, and to use
	;;; serial interrupts, not wasteful kprintf() style busy loop
	;;; code like cdg.classic.
	;;;

STAT_CONTROLLER		EQU	-80	; $B0
STAT_AFTERTOUCH		EQU	-48	; $D0
STAT_PITCHBEND		EQU	-32	; $E0

SUSTAIN_PEDAL		EQU	$40
MOD_WHEEL		EQU	$01
BREATH_CONTROL		EQU	$02

RESET_ALL		EQU	$79
ALLNOTESOFF		EQU	$7B


CDGMidiQuiet:

	PRINTF	DBG_ENTRY,<'CDG--CDGMidiQuiet()'>

		moveq	#15,d1

quietloop:
		moveq	#STAT_CONTROLLER,d0
		or.b	d1,d0
		bsr.s	CDGAddMidiByte
		moveq	#RESET_ALL,d0
		bsr.s	CDGAddMidiWord

		moveq	#SUSTAIN_PEDAL,d0
		bsr.s	CDGAddMidiWord

		moveq	#MOD_WHEEL,d0
		bsr.s	CDGAddMidiWord

		moveq	#BREATH_CONTROL,d0
		bsr.s	CDGAddMidiWord

		moveq	#ALLNOTESOFF,d0
		bsr.s	CDGAddMidiWord

		moveq	#STAT_PITCHBEND,d0
		or.b	d1,d0
		bsr.s	CDGAddMidiByte
		moveq	#0,d0
		bsr.s	CDGAddMidiWord		;send 2 0's per original code

		moveq	#STAT_AFTERTOUCH,d0
		or.b	d1,d0
		bsr.s	CDGAddMidiWord

	; Wake TBE interrupt once per loop - the TBE interrupt will
	; take care of itself unless it runs out of characters to
	; send, so this may or may not generate a spurious TBE
	; interrupt depending on whats going on in the system

		move.l	cdd_ExecLib(a6),a0
		lea	_custom,a1
		move.w	#INTF_SETCLR!INTF_TBE,d0

		DISABLE	A0,NOFETCH

		move.w	d0,intreq(a1)
		move.w	d0,intena(a1)

		ENABLE	A0,NOFETCH

		dbf	d1,quietloop

		rts

*****i* cdg.library/CDGAddMidiByte ************************************
*
*   NAME
*	CDGAddMidiByte -- Add a byte to the circular midi output buffer
*   
*   SYNOPSIS
*	VOID CDGAddMidiByte( data, byte );
*		              A6    D0
*
*	VOID CDGAddMidiByte( struct CDGData *, UBYTE );
*
*   FUNCTION
*	Stuff a byte in the circular midi buffer.
*
*   RETURNS
*	TRUE if the byte could be added to the midi output buffer.
*	FALSE if the buffer is already full.
*
*   NOTES
*	PRESERVE D1/A1 for some optimization speed
*
***********************************************************************

**
** Adds a word to the circular midi buffer; a0 is trashed
** The first byte is in D0, and the second byte is always a NULL byte
**

CDGAddMidiWord:
		bsr.s	CDGAddMidiByte
		moveq	#00,d0

	; fall through
**
** Adds a byte to the circular midi buffer; a0 is trashed
**

CDGAddMidiByte:

		move.l	cdd_PutMidiByte(a6),a0
		cmp.l	cdd_MaxMidiByte(a6),a0
		bne.s	CDGAMB_Add
		lea	cdd_MidiBytes(a6),a0

CDGAMB_Add:
		move.b	d0,(a0)+
		move.l	a0,cdd_PutMidiByte(a6)

		rts

*****i* cdg.library/CDGMidiPack ***************************************
*
*   NAME
*	CDGMidiPack -- Decode a MIDI PACK
*   
*   SYNOPSIS
*	VOID CDGMidiPack( data, pack, enabled );
*	                   A6    A4     D3
*
*	VOID CDGMidiPack( struct CDGData *, struct CDGPack *, ULONG );
*
*   FUNCTION
*	Decode a +MIDI pack, and send MIDI bytes.  MIDI bytes
*	are stored as 8 bit bytes in 6 bit symbols.  This means
*	shifting/masking operations.  4 6 bit symbols equals 3
*	midi bytes, except it is important to note that the
*	total number of bytes in the pack may be any number from
*	0-12.
*
*	The specification indicates a maximum data rate of 3125
*	bytes per second per the MIDI specification, so there is
*	an implied maximum of no more than 125 midi bytes per 12
*	consecutive packs.  Infact the spec recommends no more than
*	110 bytes per 12 consecutive packs be sent.
*	number
*
*   RETURNS
*	N/A
*
*   NOTES
*	Checks if +MIDI enabled before actually sending any data
*
***********************************************************************
CDGMidiPack:

		move.b	cdp_Instruction(a4),d1	;# of bytes

	; Only T->W are valid, and if the total bytes is 0,
	; we are done.  Use D1 here for some speed since I've
	; purposely set-up the send character routines to preserve
	; D1 & A1.


		and.b	#(SYMF_T!SYMF_U!SYMF_V!SYMF_W),d1
		beq	CDGMidiPack_Exit

		cmp.b	#12,d1		;sanity check
		bhi	CDGMidiPack_Exit

	; mark is +MIDI (and turn on light maybe [if at least 4 packs in one track ])

		subq.b	#1,cdd_MPackCount(a6)
		bpl.s	CDGM_NoMidiLight

		bset	#CDGB_MIDI,cdd_TypeLights(a6)

		clr.b	cdd_MPackCount(a6)

CDGM_NoMidiLight:

		btst	#CDGB_MIDI,d3
		beq.s	CDGMidiPack_Exit

	; if midi data has not been sent, init bps rate

		bset	#EF2B_BAUDSET,cdd_ExtraFlags2(a6)
		beq.s	CDGSetBPS
		
CDGMP_ISFirst:

	; indicate new midi data sent

		bset	#EF2B_NEWMIDI,cdd_ExtraFlags2(a6)

	; decode # of bytes in d1

		lea	cdp_Data(a4),a1

	; Inline code for efficiency - most of this code is free, in
	; that it is interleaved with serial data output running
	; at the same time (unless we take an interrupt, or task
	; switch)

CDGAddMidiPack_Loop:

		move.b	(a1)+,d0
		lsl.w	#6,d0
		or.b	(a1)+,d0
		lsl.l	#6,d0
		or.b	(a1)+,d0
		lsl.l	#6,d0
		or.b	(a1)+,d0

		swap	d0

		bsr.s	CDGAddMidiByte

		subq.b	#1,d1
		beq.s	CDGMidiPack_Done

		rol.l	#8,d0		;get next byte

		bsr.s	CDGAddMidiByte

		subq.b	#1,d1
		beq.s	CDGMidiPack_Done

		rol.l	#8,d0		;get next byte

		bsr.s	CDGAddMidiByte

		subq.b	#1,d1
		bne.s	CDGAddMidiPack_Loop

CDGMidiPack_Done:

	; Wake TBE interrupt, and start work on next PACK.  The
	; TBE interrupt will take care of itself until it runs
	; out of data.

		move.l	cdd_ExecLib(a6),a0
		lea	_custom,a1
		move.w	#INTF_SETCLR!INTF_TBE,d0

		DISABLE	A0,NOFETCH

		move.w	d0,intreq(a1)
		move.w	d0,intena(a1)

		ENABLE	A0,NOFETCH

CDGMidiPack_Exit:

		rts


	; Precalc 31500 bps for PAL/NTSC machine.  Based on the
	; numbers given in the Hardware manual -
	;
	; Per Hardware docs -
	;
	; NTSC	112.6 = ((3,579,545/31500)-1)
	;
	; PAL   111.6 = ((3,546,895/31500)-1)
	;
	; Because the difference is less than 1%, for now I will use
	; a constant of 112, which is the average of the two numbers.
	;
	; Doesn't set serper until first MIDI pack is received
	;

CDGSetBPS:
		nop
		move.w	_custom+serdatr,d0
		btst	#12,d0				;TSR Empty?
		beq.s	CDGSetBPS			;wait for pending output

		move.w	#112,_custom+serper		;set baud rate

		bra	CDGMP_ISFirst

*****i* cdg.library/CDGInitMidiOut ************************************
*
*   NAME
*	CDGInitMidiOut -- Initialize serial port for midi output
*   
*   SYNOPSIS
*	VOID CDGInitMidiOut( data );
*			      A4
*
*	VOID CDGAInitMidiOut( struct CDGData * );
*
*   FUNCTION
*	Private function
*
*   RETURNS
*	N/A
*
*   NOTES
*	MUST PRESERVE ALL REGISTERS!!
*
***********************************************************************

PARREGBITS	SET	(CIAF_PRTRSEL!CIAF_PRTRPOUT!CIAF_PRTRBUSY)
SERREGBITS	SET	(~(PARREGBITS))

CDGInitMidiOut:

	PRINTF	DBG_ENTRY,<'CDInitMidiOut()'>

		movem.l	d0-d1/a0-a1/a6,-(sp)

	; initialize the interrupt structure

		lea	cdd_MidiInt(a4),a1

		move.b	#NT_INTERRUPT,LN_TYPE(a1)

		move.l	#cdgname,LN_NAME(a1)

		move.l	a4,IS_DATA(a1)

		move.l	#CDGMidiOutInt,IS_CODE(a1)

		move.l	cdd_ExecLib(a4),a6

		moveq	#INTB_TBE,d0

		JSRLIB	SetIntVector

		move.l	d0,cdd_PrevTBEVec(a4)

		lea	_ciab,a0

		DISABLE

PARREGBITS	SET	(CIAF_PRTRSEL!CIAF_PRTRPOUT!CIAF_PRTRBUSY)
SERREGBITS	SET	(~(PARREGBITS))


	; cache previous values of DDRA, and PRA

		move.b	ciaddra(a0),cdd_CacheDDRA(a4)

	; and initialize ddra, and pra bits which belong to serial

		and.b	#PARREGBITS,ciaddra(a0)

		move.b	ciapra(a0),cdd_CachePRA(a4)

		or.b	#(CIAF_COMDTR!CIAF_COMRTS),ciaddra(a0)

		and.b	#((~(CIAF_COMRTS!CIAF_COMDTR!CIAF_COMCD))&$ff),ciapra(a0)
		and.b	#(~(CIAF_COMCTS!CIAF_COMDSR!CIAF_COMCD)),ciaddra(a0)

	;;; Disable TBE interrupts
	;;;
		move.w	#INTF_TBE,_intena	;disable TBE ints for now

		ENABLE

		movem.l	(sp)+,d0-d1/a0-a1/a6

		rts

*****i* cdg.library/CDGFreeMidiOut ************************************
*
*   NAME
*	CDGFreeMidiOut -- Restore previous ciab/serial port settings
*   
*   SYNOPSIS
*	VOID CDGFreeMidiOut( data );
*			      A4
*
*	VOID CDGFreeMidiOut( struct CDGData * );
*
*   FUNCTION
*	Private function.
*
*   RETURNS
*	N/A
*
*   NOTES
*	MUST PRESERVE ALL REGISTERS!!
*
***********************************************************************
CDGFreeMidiOut:

	PRINTF	DBG_ENTRY,<'CDGFreeMidiOut()'>

		movem.l	d0-d2/a0-a1/a6,-(sp)

	; If data is still being sent, then we can spin for a bit
	; waiting for the last of the data to be sent.  This is
	; a little bit kludgy, but it is intended to make sure
	; that when CDGEnd() is called, that we finish sending any
	; final info to midi devices (such as ALLNOTESOFF!!)
	;
	; For example, someone can call CDGStop(), CDGEnd(), and
	; easily do so before pending midi data has been sent.
	;
	; On the other hand, maybe there is some fundamental problem,
	; so there is an arbitrary time-out implied of approx 60
	; calls to WaitTOF() - about 1+ second, after which we
	; give up


		moveq	#59,d2

CDGRemMI_Spin:
		move.l	cdd_ExecLib(a4),a0

		move.l	cdd_PutMidiByte(a4),d0

		lea	_ciab,a1

	; Simple restoration of baud rate to 9600 bps for NTSC/PAL machines.
	;
	; Ideally this would get preferences, and use that, but I don't
	; want to open Intuition.library, nor did I want to add the
	; code needed to search to see if it has been opened yet.
	;
	; I could have just left it at 31500 even, but this seemed
	; a little more friendly.
	;

		move.w	#371,d1			;default NTSC 9600 bps
		cmp.b	#50,cdd_VBlankRate(a4)
		bne.s	CDGRemMI_NTSC
		move.w	#368,d1			;PAL 9600 bps

CDGRemMI_NTSC:
		DISABLE	A0,NOFETCH

		cmp.l	cdd_GetMidiByte(a4),d0
		beq.s	CDGRemMI_OK

		ENABLE	A0,NOFETCH

		move.l	cdd_GfxLib(a4),a6
		JSRLIB	WaitTOF			;sleep for a bit

		dbf	d2,CDGRemMI_Spin

CDGRemMI_OK:

	; disable TBE interrupts

		move.w	#INTF_TBE,_intena

	; 9600 bps NTSC = 371.9 / 9600 bps PAL = 368.4

		move.w	d1,_custom+serper	;reset baud rate (9600 bps)

PARREGBITS	SET	(CIAF_PRTRSEL!CIAF_PRTRPOUT!CIAF_PRTRBUSY)
SERREGBITS	SET	(~(PARREGBITS))

	; restore CIAB DDRA and PRA

		move.b	ciaddra(a1),d0
		and.b	#PARREGBITS,d0		;don't change parallel bits
		or.b	#SERREGBITS,d0		;Do change these bits
		move.b	d0,ciaddra(a1)		;set ddr

		move.b	ciapra(a1),d0
		move.b	cdd_CachePRA(a4),d1
		and.b	#SERREGBITS,d1
		and.b	#PARREGBITS,d0
		or.b	d1,d0
		move.b	d0,ciapra(a1)

		move.b	ciaddra(a1),d0		;read ddr
		and.b	#PARREGBITS,d0		;don't change parallel bits
		move.b	cdd_CacheDDRA(a4),d1	;read old ddr
		and.b	#SERREGBITS,d1		;only use serial bits
		or.b	d1,d0
		move.b	d0,ciaddra(a1)		;restore serial bits in ddr

		ENABLE	A0,NOFETCH

	; now restore the previous vector

		move.l	cdd_ExecLib(a4),a6
		move.l	cdd_PrevTBEVec(a4),a1
		moveq	#INTB_TBE,d0

		JSRLIB	SetIntVector

		movem.l	(sp)+,d0-d2/a0-a1/a6

		rts

*****i* cdg.library/CDGMidiOutInt *************************************
*
*   NAME
*	CDGMidiOutInt -- Level 1 interrupt for midi output
*   
*   SYNOPSIS
*	VOID CDGMidiOutInt( data, custom );
*			      A1  A0
*
*	VOID CDGMidiOutInt( struct CDGData *, struct custom * );
*
*   FUNCTION
*	Private functions - interrupt when ready to send next character
*
*   RETURNS
*	N/A
*
*   NOTES
*
***********************************************************************
CDGMidiOutInt:


		move.w	#INTF_TBE,intreq(a0)	;clear interrupt

	;
	; Determine if this is a real TBE interrupt, or just NOISE (e.g.,
	; woken by task) by checking to see if the TBE is empty.
	; 
		move.w	serdatr(a0),d0
		btst	#13,d0
		beq.s	CDGMOI_NotTBE

		move.l	cdd_GetMidiByte(a1),a5
		cmp.l	cdd_PutMidiByte(a1),a5
		beq.s	CDGMOI_Empty		;any more to send?

		cmp.l	cdd_MaxMidiByte(a1),a5
		bne.s	CDGMidiOut_SendNext

		lea	cdd_MidiBytes(a1),a5

CDGMidiOut_SendNext:

		move.w	#$100,d0
		move.b	(a5)+,d0
		move.l	a5,cdd_GetMidiByte(a1)
		move.w	d0,serdat(a0)

CDGMOI_NotTBE:
		rts

CDGMOI_Empty:
		move.w	#INTB_TBE,intena(a0)	;disable TBE again
		rts

		END