**
**	$Id: cdgbase.i,v 1.26 93/02/18 16:25:48 darren Exp $
**
**	CDTV CD+G -- cdgbase.i (base variables)
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

** Includes

	INCLUDE	"exec/types.i"

	INCLUDE	"exec/macros.i"
	INCLUDE	"exec/resident.i"
	INCLUDE	"exec/interrupts.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/io.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/ports.i"
	INCLUDE	"exec/tasks.i"
	INCLUDE	"exec/initializers.i"
	INCLUDE	"exec/ports.i"
	INCLUDE	"exec/ables.i"
	INCLUDE	"exec/memory.i"

	INCLUDE	"graphics/rastport.i"
	INCLUDE	"graphics/view.i"
	INCLUDE	"graphics/gfx.i"
	INCLUDE	"graphics/gfxbase.i"
	INCLUDE	"graphics/sprite.i"
	INCLUDE	"graphics/copper.i"
	INCLUDE	"graphics/display.i"

	INCLUDE	"utility/utility.i"

	INCLUDE	"devices/timer.i"
	INCLUDE	"hardware/intbits.i"
	INCLUDE	"hardware/custom.i"
	INCLUDE	"hardware/blit.i"
	INCLUDE	"hardware/dmabits.i"
	INCLUDE	"hardware/cia.i"

	INCLUDE	"resources/misc.i"


** Bit definitions

		; SYMBOL bit definitions

			BITDEF	SYM,P,7
			BITDEF	SYM,Q,6
			BITDEF	SYM,R,5
			BITDEF	SYM,S,4
			BITDEF	SYM,T,3
			BITDEF	SYM,U,2
			BITDEF	SYM,V,1
			BITDEF	SYM,W,0

		; cdd_VBlankFlags

			BITDEF	VBF,NEWCOLORS,0
			BITDEF	VBF,NEWPLANES,1
			BITDEF	VBF,BACKSCREEN,2
			BITDEF	VBF,RESETDMA,3

		; cdd_ExtraFlags2

			BITDEF	EF2,IGNORE,0		;TRUE means ignore PACKET
			BITDEF	EF2,NEWMIDI,1		;TRUE if new midi data sent
			BITDEF	EF2,BAUDSET,2		;TRUE if baud set

		; cdd_ExtraFlags3

			BITDEF	EF3,NOECC,0		;TRUE means no Error Correction
			BITDEF	EF3,NOMIDI,1		;TRUE means no MIDI processing
			BITDEF	EF3,ALTSPRITES,2	;TRUE means use sprites 2-5

** TV Graphics Instruction Equates

INSTR_PRESET_MEMORY	EQU	1
INSTR_PRESET_BORDER	EQU	2
INSTR_WRITE_FONT	EQU	6
INSTR_SCROLL_PRESET	EQU	20
INSTR_SCROLL_COPY	EQU	24
INSTR_CLUT_LOW		EQU	30
INSTR_CLUT_HIGH		EQU	31
INSTR_EOR_FONT		EQU	38

** Build equates (see debug.i for debug on/off control)

ROMBUILD		EQU		1	; Make TRUE for ROM BUILD

SIMULATION		EQU		0	; Make TRUE for simulation code

SHOWBORDER		EQU		0	; Make TRUE to SHOW borders
						; For debugging purposes only

** Macros


** Data size equates

APTR_SIZE		EQU	4
PACKDATA_SIZE		EQU	16


** Graphics Equates

DISPLAY_DEPTH		EQU	5
DISPLAY_COLUMNS		EQU	50		;2 columns are hidden
DISPLAY_ROWS		EQU	18		;2 rows are hidden
FONT_WIDTH		EQU	6		;Fonts are 6 bit wide,
FONT_HEIGHT		EQU	12		;by 12 bit high graphics

DISPLAY_WIDTH		EQU	320

			IFNE	DISPLAY_WIDTH-320
			FAIL	"DISPLAY_WIDTH ! 320; recode"
			ENDC

DISPLAY_HEIGHT		EQU	228		;(18+1) * FONT_HEIGHT

CENTER_X		EQU	16		;visible screen is 320-32 wide
CENTER_Y		EQU	2		;visible screen is 220-4 tall

			IFNE	SHOWBORDER

VISUAL_WIDTH		EQU	DISPLAY_WIDTH
VISUAL_HEIGHT		EQU	(DISPLAY_ROWS*FONT_HEIGHT)
VISUAL_X		EQU	0
VISUAL_Y		EQU	0

			ENDC

			IFEQ	SHOWBORDER

VISUAL_WIDTH		EQU	((DISPLAY_COLUMNS-2)*FONT_WIDTH)
VISUAL_HEIGHT		EQU	((DISPLAY_ROWS-2)*FONT_HEIGHT)
VISUAL_X		EQU	16
VISUAL_Y		EQU	12

			ENDC

RELSPRITE_X		EQU	240
RELSPRITE_Y		EQU	150
SPRITE_WIDTH		EQU	16

** Other structure equates

	; 75 PACKETS is 1 seconds worth of data

PACKETSTORE		EQU	75		; # of PACKETs to buffer

	; The size of the midi output buffer is a guestimate based on
	; the maximum number of characters per second out (roughly
	; 3125 bytes/second), plus some pad for a 256 byte sequence to
	; tell all midi devices to be quiet, plus some extra.  Probably
	; much larger than needed, but not a major problem since we
	; cannot do much else while CD+G/+MIDI is running


MIDISTORE		EQU	4096		; midi out circular buffer

** Valid PACK counter equates

GPACKCOUNT		EQU	(4-1)		; must get 4 to be valid?
MPACKCOUNT		EQU	(4-1)		; must get 4 to be valid?

** Structures


	; CD+G PACK structure

	STRUCTURE	CDGPack,0
		UBYTE	cdp_ModeItem
		UBYTE	cdp_Instruction
		UBYTE	cdp_ParityQ0
		UBYTE	cdp_ParityQ1
		STRUCT	cdp_Data,PACKDATA_SIZE
		UBYTE	cdp_ParityP0
		UBYTE	cdp_ParityP1
		UBYTE	cdp_ParityP2
		UBYTE	cdp_ParityP3
		LABEL	CDGPack_SIZEOF

	; CD+G PACKET structure (note that the SYNC bytes really come
	; after the PACKET, except it wasn't clear this was going to
	; be the case until after I had already written much of this
	; code, hence I use the cdk_SYNCSX label - cdk_STARTPACKET to
	; check the sync bytes


	STRUCTURE	CDGPacket,0
		UBYTE	cdk_SYNCS0
		UBYTE	cdk_SYNCS1
		LABEL	cdk_STARTPACKET
		STRUCT	cdk_PACK0,CDGPack_SIZEOF
		STRUCT	cdk_PACK1,CDGPack_SIZEOF
		STRUCT	cdk_PACK2,CDGPack_SIZEOF
		STRUCT	cdk_PACK3,CDGPack_SIZEOF
		LABEL	cdk_SYNCSX
		LABEL	CDGPacket_SIZEOF

	; CD+G PACK BUCKET structure

	STRUCTURE	CDGBucket,0
		APTR	cdt_Next
		STRUCT	cdt_PackData,CDGPack_SIZEOF
		LABEL	CDGBucket_SIZEOF

	; Structure for saving copper list pointers

	STRUCTURE	CDGCopper,0
		APTR	cdc_Color0
		APTR	cdc_BPL0
		APTR	cdc_BPLCON1
		APTR	cdc_COPINITColor0
		APTR	cdc_COPINITColor0b	;if AA (else same as above)
		APTR	cdc_Color0b		;if enlightened
		UWORD	cdc_BPLCON1_DEF
		UWORD	cdc_BPLCON1_HIGHDEF	;AA crude scroll
		ALIGNLONG
		LABEL	CDGCopper_SIZEOF

	; Minimal library base - little memory usage when not running

	STRUCTURE	CDGBase,LIB_SIZE
		UBYTE	cdg_Pad1
		UBYTE	cdg_Pad2		;long word align
		APTR	cdg_ExecLib
		APTR	cdg_SegList
		APTR	cdg_BaseData		;struct CDGData *
		LABEL	CDGBase_SIZEOF

	; This memory is allocated at run-time only.  This allows CD+G's
	; base to hang around in memory if desired

	STRUCTURE	CDGData,0
		STRUCT	cdd_CDIO,IOSTD_SIZE	;MUST be FIRST!!
		STRUCT	cdd_CDINT,IS_SIZE	;for frame call interrupt
		STRUCT	cdd_CDPORT,MP_SIZE
		ALIGNLONG
		APTR	cdd_CDGBase		;back pointer
		APTR	cdd_ExecLib
		APTR	cdd_GfxLib
		APTR	cdd_MiscResource	;non-NULL if serial stuff owned
		APTR	cdd_CDGPrefs

	; pointer to subcode data

		APTR	cdd_SubCodeBank		;from device base

	; graphics bit map stuff - double buffered display

		STRUCT	cdd_View1,v_SIZEOF
		STRUCT	cdd_ViewPort1,vp_SIZEOF
		STRUCT	cdd_RasInfo1,ri_SIZEOF
		STRUCT	cdd_ColorMap1,cm_SIZEOF
		STRUCT	cdd_BitMap1,bm_SIZEOF

	; pointers to bitplanes; ORDER dependent!

		APTR	cdd_BitPlane1_0
		APTR	cdd_BitPlane1_1
		APTR	cdd_BitPlane1_2
		APTR	cdd_BitPlane1_3
		APTR	cdd_BitPlane1_Extra

		APTR	cdd_BitPlane2_0
		APTR	cdd_BitPlane2_1
		APTR	cdd_BitPlane2_2
		APTR	cdd_BitPlane2_3
		APTR	cdd_BitPlane2_Extra

		APTR	cdd_ExtraRaster1	;extra bitplane - all 1's

	;
	; The active/inactive pointers are swapped by the routines
	; that use them, so in a WORST case scenerio I'm drawing in
	; a plane that is still visible because VBLANK hasn't happened
	; yet, and the visible planes haven't actually been swapped.  In
	; real usage this is not the case though; scrolling is generally
	; limited to something usable (e.g., less than 50x per second).
	;
	; Still, in the WORST case no data is lost, its just going to
	; look poor for < a VBLANK.
	;
	; At one time these pointers, and the VBLANK flag were being grabbed
	; atomically via MOVEM; no longer the case, but could be due
	; to the order of these.
	;

		UBYTE	cdd_MOVEMPTRS		;currently unused
		UBYTE	cdd_ExtraFlags2		;state tracking flags
		UBYTE	cdd_ExtraFlags3		;more space for flags
		UBYTE	cdd_VBlankFlags

		; Various VBLANK flags

		APTR	cdd_InactivePlanes	;pointer to inactive bitplanes
		APTR	cdd_ActivePlanes	;pointer to active bitplanes

	; PH & PV values for scrolling

		ULONG	cdd_PHPV

	; pointer to memory for 4 sprites + CHIP ram for temp font
	; data so the blitter can get at it

		APTR	cdd_4SpriteData

	; pre-calced pointers to start of sprites in cdd_4SpriteData

		APTR	cdd_SpriteData3
		APTR	cdd_SpriteData4
		APTR	cdd_SpriteData5

	; 0 or 2 (for sprites 0-3, or sprites 2-5)

		ULONG	cdd_SpriteBase

	; pre-calced pointers to 4 TV GFX images in CHIP ram

		APTR	cdd_FontPlane
		APTR	cdd_NegPlane
		APTR	cdd_1sPlane
		APTR	cdd_0sPlane

	; total memory allocated in CHIP ram for 4 sprites

		ULONG	cdd_4SpriteSize

	; 4 SimpleSprite structures

		STRUCT	cdd_SimpleSprite2,ss_SIZEOF
		STRUCT	cdd_SimpleSprite3,ss_SIZEOF
		STRUCT	cdd_SimpleSprite4,ss_SIZEOF
		STRUCT	cdd_SimpleSprite5,ss_SIZEOF

	; space for 32 color table (for LoadRGB4)

		STRUCT	cdd_ColorTable1,(32*2)

	; space for a 16 color table for CD+G CLUT

		STRUCT	cdd_CLUTTable,(16*2)

	; Copper list pointers - sniffed out at run time, and cached
	; here for speed

		STRUCT	cdd_CopperList1,CDGCopper_SIZEOF
		
	; Pointer to current border PEN number

		APTR	cdd_BorderPen

	; Interrupt structs for int servers

		ALIGNLONG

		STRUCT	cdd_IntVBlank,IS_SIZE

		ALIGNLONG

		STRUCT	cdd_IntPackets,IS_SIZE

	; Various flags, and fields

		ALIGNLONG

	; The following two UWORDS must be together so an atomic
	; MOVE.L can be used to update the sprite index value, and
	; delay counter.

		; indicate show new sprite(s) by setting field index
		; into sprite pointer array(s).  0 is nominal - no
		; new sprite.  Sprite data is updated at next VLBANK,
		; so no display glitches.

		UWORD	cdd_StartSprite

		; and a delay counter for how many VBLANK's the sprite
		; should be turned on for.  0 means turn sprite off,
		; and a negative number means leave it on forever!

		UWORD	cdd_SpriteDelay

		; VBlank rate stored here - used to adjust timings
		; for turning off sprites

		UWORD	cdd_VBlankRate

		; Active FONT channel (1-15) 0 is currently always ON

		UBYTE	cdd_ActiveChannel

		; CD type flags (for LIGHTS, and return() value from CDGDraw())

		UBYTE	cdd_TypeLights

		; cache CIABDDRA

		UBYTE	cdd_CacheDDRA

		; cache CIABPRA

		UBYTE	cdd_CachePRA

		; count +G packs

		UBYTE	cdd_GPackCount

		; count +MIDI packs

		UBYTE 	cdd_MPackCount

		; count packet roll-over; reset valid counters

		UBYTE	cdd_FRollCount

		; Pad byte

		UBYTE	cdd_FRollPad

		; cache DMACONR so we know if we need to turn sprites
		; OFF when CDGBack() is called

		UWORD	cdd_CacheDMACONR

		; calculate minimum X display positions so we don't lose
		; any sprites due to sprite DMA limitations

		WORD	cdd_MinDisplayX

	; cache info for signal task per cdtv.device FRAMECALL

		ALIGNLONG

		ULONG	cdd_SigMask
		APTR	cdd_SigTask

	; storage for midi out buffer

		ALIGNLONG

		APTR	cdd_PutMidiByte
		APTR	cdd_GetMidiByte
		APTR	cdd_MaxMidiByte
		STRUCT	cdd_MidiBytes,MIDISTORE
		LABEL	cdd_EndMidiBytes

		ALIGNLONG
		APTR	cdd_PrevTBEVec
		STRUCT	cdd_MidiInt,IS_SIZE

	; storage for de-interleave buckets

		ALIGNLONG

		; space for 8 de-interleaved PACKETS, a pointer to the
		; next bucket, and a counter, so we know how many packs
		; have been de-interleaved.

		APTR	cdd_NextBucket
		STRUCT	cdd_Buckets,(CDGBucket_SIZEOF*8)

		ALIGNWORD

		UWORD	cdd_PackCount

	; storage for PACKETS - Must at least be WORD ALIGNED

		ALIGNLONG

		APTR	cdd_PutPacket		;incremented by interrupt
		APTR	cdd_GetPacket		;incremented by task
		APTR	cdd_MaxPacket		;end of buffer
		STRUCT	cdd_PacketData,((CDGPacket_SIZEOF-cdk_STARTPACKET)*PACKETSTORE)
		LABEL	cdd_EndPackets

		STRUCT	cdd_ExtraPacket,CDGPacket_SIZEOF
		ALIGNWORD

		LABEL	CDGData_SIZEOF

