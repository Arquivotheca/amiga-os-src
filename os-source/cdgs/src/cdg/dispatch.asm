**
**	$Id: dispatch.asm,v 1.21 93/02/03 12:01:31 darren Exp $
**
**	CDTV CD+G -- dispatch.asm (dispatch packets)
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

** Includes

		INCLUDE	"cdgbase.i"
		INCLUDE	"cdgprefs.i"
		INCLUDE	"debug.i"

** Exports
		SECTION	cdg

		XDEF	CDGDraw
		XDEF	CDGUserPack

**Imports
		XREF	_TVPresetBorder
		XREF	_TVCLUTLow
		XREF	_TVCLUTHigh
		XREF	_TVPresetMemory
		XREF	_TVWriteFont
		XREF	_TVEORFont
		XREF	_TVScrollPreset
		XREF	_TVScrollCopy

		XREF	DecodeRS	;ecc.asm

		XREF	_custom		;amiga.lib
** Equates

						;PQRSTUVW
TV_GRAPHICS_MODE	EQU	(SYMF_T!SYMF_W)	;00001001
MIDI_MODE		EQU	(SYMF_S!SYMF_T)	;00011000

** Assumptions


******* cdg.library/CDGUserPack **************************************
*
*   NAME
*	CDGUserPack -- Call cdg.library with your own PACK data
*   
*   SYNOPSIS
*	VOID CDGUserPack( pack );
*	                   A0
*
*	VOID CDGUserPack( UBYTE * );
*
*   FUNCTION
*	This function is meant for those who want to draw their
*	own graphics on the CD+G screen, or send a custom sequence
*	of MIDI bytes.
*
*	In general it is meant to be used when a CD+G disk, or
*	CD+MIDI disk is not playing.  However given a custom
*	CD+G disk, it is possible to mix real-time CD+G images
*	with software generated images on the same screen.
*
*   INPUTS
*	pack - Pointer to PACK data.  Each symbol in a PACK
*	consists of 6 bits, otherwise known as channels P
*	through W.
*
*	The P and Q channel bits MUST be 0.  The Q0-Q1 parity
*	bytes, and P0-P3 parity bytes are ignored because it
*	is assumed that the PACK you pass to this function is
*	free of bit errors.  Because no error correction is needed,
*	it is possible to draw at a much faster rate than is normally
*	possible with real-time data from a disk.
*
*   RETURNS
*	N/A
*
*   NOTES
*
*   SEE ALSO
*
***********************************************************************
CDGUserPack:
	PRINTF	DBG_ENTRY,<'CDG--CDGUserPack($%lx)'>,A0

		movem.l	a4/a6,-(sp)

		move.l	a0,a4
		move.l	cdg_BaseData(a6),a6

	; check if +G

		moveq	#00,d1
		move.b	(a4),d1
		cmp.b	#TV_GRAPHICS_MODE,d1
		bne.s	CDGUP_Midi

		move.b	cdp_Instruction(a4),d1
		cmpi.b	#INSTR_EOR_FONT,d1	;sanity check
		bhi.s	CDGUP_Exit

		lsl.b	#2,d1			;6->8 bits
		lea	_DispatchTable(pc),a0
		move.l	0(a0,d1.w),a0
		jsr	(a0)

		movem.l	(sp)+,a4/a6
		rts

CDGUP_Midi:
		cmp.b	#MIDI_MODE,d1
		bne.s	CDGUP_Exit

	; cannot do if no resource allocation

		tst.l	cdd_MiscResource(a6)
		beq.s	CDGUP_Exit

		move.l	d3,-(sp)

	; don't allow changes to +G/+MIDI lights via this function

		move.b	cdd_TypeLights(a6),d3	;cache

		swap	d3			;save type lights

		move.w	#CDGF_MIDI,d3		;enable +MIDI

		BSRSELF	CDGMidiPack

		swap	d3			;restore type lights

	; restore +MIDI light

		move.b	d3,cdd_TypeLights(a6)

		move.l	(sp)+,d3
CDGUP_Exit:

		movem.l	(sp)+,a4/a6
		rts

******* cdg.library/CDGDraw ******************************************
*
*   NAME
*	CDGDraw -- Main CD+G PACKET handling routine
*   
*   SYNOPSIS
*	flags=CDGDraw( types );
*	D0		D0
*
*	ULONG CDGDraw( ULONG );
*
*   FUNCTION
*	Process pending packets.  CDGDraw() must be called in
*	a loop from an efficient, relatively high priority task.
*	Failure to call this function often enough will result
*	in CD+G data being lost.
*
*	Currently this function must be called at least once every
*	1/2 a second, but while the CD+G screen is in front, it
*	should be called as frequently as possible.  Ideally it is
*	called in a loop which does little more than check for
*	user input, and use the cd.device to control disk play,
*	stop, pause, etc.
*
*	While a +MIDI disk is playing, your code should call this
*	function at least 75x per second, or the +MIDI output may not be
*	synchronized with the CD audio output.
*
*   INPUTS
*	types - Type(s) of data to process.  See cdgprefs.h/i
*	for defined flags.  For example -
*
*		CDGDraw((ULONG)CDGF_GRAPHICS|CDGF_MIDI);
*
*	Processes +MIDI, and +G PACKS.
*
*		CDGDraw((ULONG)CDGF_GRAPHICS);
*
*	Processes +G PACKS only.
*
*   RETURNS
*	flags - A set of flags as defined in cdgprefs.h/i.  The
*	flags are used by the caller to determine if any +G, or
*	+MIDI PACKS have been processed.
*
*   NOTES
*	Currently this function knows how to process TVGRAPHICS
*	PACKS, and MIDI PACKS.  MIDI PACKS causes data to be sent
*	at MIDI baud rates via the serial port.
*
*	It is also possible to use Wait() instead of calling this
*	function in a pure busy loop.  This requires providing a
*	pointer to a task structure, and signal mask in the CDGPrefs
*	structure when you call CDGBegin().  Because your task
*	will be signaled approximately 75 times per second, your
*	code needs to be efficient.
*
*	In addition, the calling task must be of a high enough
*	priority to avoid long delays.  In the typical CDTV-LIKE
*	application only one task is effectively running, so this may
*	not be a significant problem.  Even short delays can result in
*	rough animation which is undesirable.
*
*	It is allowable to call this function a pure busy loop,
*	and not wait for a signal.  This function will simply
*	return if there are no PACKs to be processed.
*
*   SEE ALSO
*	cdg/cdgprefs.h, cdg/cdgprefs.i
*
***********************************************************************

*	Code scans PACKS stored via an interrupt.  There are 4 PACKS
*	per PACKET.  There is an interrupt routine which duplicates
*	a PACKET at a time using an optimized copy, and the PACKS
*	within those PACKETS are decoded here.  Currently only
*	TV_GRAPHICS_MODE, and MIDI_MODE are recognized.
*
*	PACKETs are stored in a circular buffer, so it is possible
*	to get behind somewhat, and still not lose data.  The maximum
*	buffer size can be set in cdgbase.i.
*
*	I was hoping that the PACKS would be de-interleaved by the
*	4510, but no such luck, so I do it in this code.
*
CDGDraw:

		movem.l	d2-d3/a4-a6,-(sp)

		move.l	d0,d3		;cache 'types'

	; obtain pointer to data

		move.l	cdg_BaseData(a6),a6
		
	; Obtain pointer to PACKET data from circular buffer.
	; The interrupt updates cdd_PutPacket, and this code
	; updates cdd_GetPacket, so there is never any race condition.
	;
	; Typical case is there is only 1 PACKET pending - 2 or more
	; means we are behind, and really shouldn't be.
	;
	; We have to de-interleave this data, so we only take a PACK
	; at a time, and store in a circular bucket buffer.  This
	; means we have a PACK bucket ready, and 7 more being built
	; hence we have to handle each PACK bucket before we can
	; get the next PACK's worth of data, and store it in the
	; PACK buckets.
	;
		move.w	cdd_PackCount(a6),d2

CDGDraw_NextPacket:

		move.l	cdd_GetPacket(a6),a5

	; check for any pending packets

CDGDraw_PrimePack:

		cmp.l	cdd_PutPacket(a6),a5
		beq	CDGDraw_Exit

		cmp.l	cdd_MaxPacket(a6),a5
		bne.s	CDGDraw_Process

		lea	cdd_PacketData(a6),a5

CDGDraw_Process:


	; This is a bucket sorting algolrithim, and ARGHH! adds
	; nearly 80 microseconds of overhead per PACK (ouch).  A
	; brute force approach is used, which turns out to be
	; nearly 4x faster than the the scheme used by the original
	; CD+G code.
	;
	; We could have done something like the original CD+G code
	; too which used a lookup table for the destination offsets,
	; and bumped the pointer for every byte in the source.  Timing
	; turned out to be 3x longer for this kind of code - ACK, need
	; to look at what else might be done to optimize this.
	;
	; The PACKS come in 8x interleaved, hence, 3 bytes per PACK
	; belong to one of 8 different PACKS.  In addition, some bytes
	; are scrambled (bytes 1, 2, 3, 5, 18, and 23)
	; 
	;
		move.l	cdd_NextBucket(a6),a1

		move.b	(a5),cdt_PackData(a1)
		move.b	8(a5),cdt_PackData+8(a1)
		move.b	16(a5),cdt_PackData+16(a1)

		move.l	(a1),a1
                                                      
		move.b	1(a5),cdt_PackData+18(a1)
		move.b	9(a5),cdt_PackData+9(a1)
		move.b	17(a5),cdt_PackData+17(a1)

		move.l	(a1),a1
                                                      
		move.b	2(a5),cdt_PackData+5(a1)
		move.b	10(a5),cdt_PackData+10(a1)
		move.b	18(a5),cdt_PackData+1(a1)

		move.l	(a1),a1

		move.b	3(a5),cdt_PackData+23(a1)
		move.b	11(a5),cdt_PackData+11(a1)
		move.b	19(a5),cdt_PackData+19(a1)

		move.l	(a1),a1

		move.b	4(a5),cdt_PackData+4(a1)
		move.b	12(a5),cdt_PackData+12(a1)
		move.b	20(a5),cdt_PackData+20(a1)

		move.l	(a1),a1

		move.b	5(a5),cdt_PackData+2(a1)
		move.b	13(a5),cdt_PackData+13(a1)                 
		move.b	21(a5),cdt_PackData+21(a1)                  
                                                      
		move.l	(a1),a1

		move.b	6(a5),cdt_PackData+6(a1)
		move.b	14(a5),cdt_PackData+14(a1)                   
		move.b	22(a5),cdt_PackData+22(a1)                 
                                                      
		move.l	(a1),a1

		move.b	7(a5),cdt_PackData+7(a1)
		move.b	15(a5),cdt_PackData+15(a1)                   
		move.b	23(a5),cdt_PackData+3(a1)                 

	; We leave the bucket pointer alone this time, so that
	; the next PACK is not copied to the same place; essentially
	; cdd_NextBucket rolls through all 8 buffers
	;
		move.l	a1,cdd_NextBucket(a6)

		adda.w	#CDGPack_SIZEOF,a5	;skip to next buffered PACK
		move.l	a5,cdd_GetPacket(a6)
		addq.w	#1,d2			;processed 1 pack

	; Check if we have de-interleaved enough PACKS to form at least 1
	; de-interleaved PACK.

		cmpi.w	#15,d2
		bcs	CDGDraw_PrimePack

	; A1 now points to the last bucket we dropped 3 bytes in to.  This
	; also happens to be de-interleaved if the PACK count is large
	; enough.  The actual data is stored after the pointer in the
	; bucket structure.  Unlike cdg.classic, I don't copy this data
	; yet again, but rather pass a pointer to it in register A4 (faster).


		lea	cdt_PackData(a1),a4	;CASM should optimize as ADDQ #4

	; HERE is the perfect spot to call a REED-SOLOMON ERROR CORRECTION 
	; routine with a pointer to the PACK in A4

		bsr	DecodeRS
		beq.s	CDGDraw_skippack	;good pack?

		moveq	#00,d1
		move.b	(a4),d1			;MODE byte

		cmp.b	#TV_GRAPHICS_MODE,d1
		bne.s	CDGDraw_midi		;fast path for TV_GRAPHICS!

		move.b	cdp_Instruction(a4),d1	;INSTRUCTION byte

		cmpi.b	#INSTR_EOR_FONT,d1	;sanity check else we could crash
		bhi.s	CDGDraw_skippack

		lsl.b	#2,d1			; * 4 (6 bits->8 bits)!
		move.l	_DispatchTable(pc,d1.w),a0

		cmp.l	#TVNOP,a0
		beq.s	CDGDraw_skippack

	; mark is +G disk


		subq.b	#1,cdd_GPackCount(a6)
		bpl.s	CDGDraw_MaybeG

		bset	#CDGB_GRAPHICS,cdd_TypeLights(a6)

		clr.b	cdd_GPackCount(a6)

CDGDraw_MaybeG:

	; check if +G enabled

		btst	#CDGB_GRAPHICS,d3
		beq.s	CDGDraw_skippack

		jsr	(a0)

CDGDraw_skippack:

		subq.w	#1,d2			;PACKS -1
		bra	CDGDraw_PrimePack

**
** CD+MIDI handling code
**

CDGDraw_midi:

		cmp.b	#MIDI_MODE,d1
		bne.s	CDGDraw_skippack

	; CD+MIDI only works if we own the SERIAL port, and bits.

		tst.l	cdd_MiscResource(a6)
		beq.s	CDGDraw_skippack

		bsr	CDGMidiPack
	;
		subq.w	#1,d2			;PACKS -1
		bra	CDGDraw_PrimePack

**
** Exit code - Front Panel handling, and RETURN value(s)
**
CDGDraw_Exit:
		move.w	d2,cdd_PackCount(a6)	;cache for next time through

	; Return +G and/or +MIDI bits in D0; also happens to be
	; perfect for poking the VFD (Vac. Flor. Display) control
	; byte.

		moveq	#00,d0
		move.b	cdd_TypeLights(a6),d0

		movem.l	(sp)+,d2-d3/a4-a6
		rts

*****i* cdg.library/CDGNOP ********************************************
*
*   NAME
*	CDGNOP -- Do nothing instruction (unknown)
*   
*   SYNOPSIS
*	VOID CDGNOP ( data, pack )
*			A6   A4
*
*	VOID _CDGNOP( struct CDGData *, struct CDGPack * );
*
*   FUNCTION
*	Do nothing - undefined/unknown instruction.
*
*   RETURNS
*	None.
*
***********************************************************************
CDGNOP:
TVNOP:

	rts

***********************************************************************
**
** PACK INSTRUCTION look-up table.  Unknown/Undefined instructions
** call a NOP function.
**
***********************************************************************

INSTRUCTION	MACRO	;label
		IFND	\1
			XREF	\1
		ENDC
		DC.L	\1
		ENDM

_DispatchTable:
		INSTRUCTION	TVNOP			;0
		INSTRUCTION	_TVPresetMemory		;1
		INSTRUCTION	_TVPresetBorder		;2
		INSTRUCTION	TVNOP			;3
		INSTRUCTION	TVNOP			;4
		INSTRUCTION	TVNOP			;5
		INSTRUCTION	_TVWriteFont		;6
		INSTRUCTION	TVNOP			;7
		INSTRUCTION	TVNOP			;8
		INSTRUCTION	TVNOP			;9
		INSTRUCTION	TVNOP			;10
		INSTRUCTION	TVNOP			;11
		INSTRUCTION	TVNOP			;12
		INSTRUCTION	TVNOP			;13
		INSTRUCTION	TVNOP			;14
		INSTRUCTION	TVNOP			;15
		INSTRUCTION	TVNOP			;16
		INSTRUCTION	TVNOP			;17
		INSTRUCTION	TVNOP			;18
		INSTRUCTION	TVNOP			;19
		INSTRUCTION	_TVScrollPreset		;20
		INSTRUCTION	TVNOP			;21
		INSTRUCTION	TVNOP			;22
		INSTRUCTION	TVNOP			;23
		INSTRUCTION	_TVScrollCopy		;24
		INSTRUCTION	TVNOP			;25
		INSTRUCTION	TVNOP			;26
		INSTRUCTION	TVNOP			;27
		INSTRUCTION	TVNOP			;28
		INSTRUCTION	TVNOP			;29
		INSTRUCTION	_TVCLUTLow		;30
		INSTRUCTION	_TVCLUTHigh		;31
		INSTRUCTION	TVNOP			;32
		INSTRUCTION	TVNOP			;33
		INSTRUCTION	TVNOP			;34
		INSTRUCTION	TVNOP			;35
		INSTRUCTION	TVNOP			;36
		INSTRUCTION	TVNOP			;37
		INSTRUCTION	_TVEORFont		;38

		END


