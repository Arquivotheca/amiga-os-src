**
**	$Id: font.asm,v 1.4 92/03/02 13:37:32 darren Exp $
**
**	CDTV CD+G -- font.asm (draw 6x12 graphic in 2 colors)
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

		SECTION	cdg

** Exports

		XDEF	_TVWriteFont
		XDEF	_TVEORFont

** Imports
		XREF	_custom		; amiga.lib

** Assumptions

	IFNE	cdd_BitPlane1_1-cdd_BitPlane1_0-4
	FAIL	"cdd_BitPlane1_1 does not follow cdd_BitPlane1_0; recode!"
	ENDC

	IFNE	cdd_BitPlane1_2-cdd_BitPlane1_1-4
	FAIL	"cdd_BitPlane1_2 does not follow cdd_BitPlane1_1; recode!"
	ENDC

	IFNE	cdd_BitPlane1_3-cdd_BitPlane1_2-4
	FAIL	"cdd_BitPlane1_3 does not follow cdd_BitPlane1_2; recode!"
	ENDC

	IFNE	cdd_BitPlane2_1-cdd_BitPlane2_0-4
	FAIL	"cdd_BitPlane2_1 does not follow cdd_BitPlane2_0; recode!"
	ENDC

	IFNE	cdd_BitPlane2_2-cdd_BitPlane2_1-4
	FAIL	"cdd_BitPlane2_2 does not follow cdd_BitPlane2_1; recode!"
	ENDC

	IFNE	cdd_BitPlane2_3-cdd_BitPlane2_2-4
	FAIL	"cdd_BitPlane2_3 does not follow cdd_BitPlane2_2; recode!"
	ENDC

	IFNE	cdd_ExtraFlags2-cdd_MOVEMPTRS-1
	FAIL	"cdd_ExtraFlags2 does not follow cdd_MOVEMPTRS; recode!"
	ENDC

	IFNE	cdd_ExtraFlags3-cdd_ExtraFlags2-1
	FAIL	"cdd_ExtraFlags3 does not follow cdd_ExtraFlags2; recode!"
	ENDC

	IFNE	cdd_VBlankFlags-cdd_ExtraFlags3-1
	FAIL	"cdd_VBlankFlags does not follow cdd_ExtraFlags3; recode!"
	ENDC

	IFNE	cdd_ActivePlanes-cdd_InactivePlanes-4
	FAIL	"cdd_ActivePlanes does not follow cdd_InactivePlanes; recode!"
	ENDC

	IFNE	cdd_InactivePlanes-cdd_VBlankFlags-1
	FAIL	"cdd_InactivePlanes does not follow cdd_VBlankFlags; recode!"
	ENDC

	IFNE	cdd_NegPlane-cdd_FontPlane-4
	FAIL	"cdd_NegPlane does not follow cdd_FontPlane; recode!"
	ENDC

	IFNE	cdd_0sPlane-cdd_1sPlane-4
	FAIL	"cdd_0sPlane does not follow cdd_1sPlane; recode!"
	ENDC

	IFNE	bltcon1-bltcon0-2
	FAIL	"bltcon1 does not follow bltcon0; recode!"
	ENDC

	IFNE	bltafwm-bltcon1-2
	FAIL	"bltafwm does not follow bltcon1; recode!"
	ENDC

	IFNE	bltalwm-bltafwm-2
	FAIL	"bltalwm does not follow bltafwm; recode!"
	ENDC

	IFNE	bltcpt-bltalwm-2
	FAIL	"bltcpt does not follow bltalwm; recode!"
	ENDC

	IFNE	bltbpt-bltcpt-4
	FAIL	"bltbpt does not follow bltcpt; recode!"
	ENDC

	IFNE	bltapt-bltbpt-4
	FAIL	"bltapt does not follow bltbpt; recode!"
	ENDC

	IFNE	bltdpt-bltapt-4
	FAIL	"bltdpt does not follow bltapt; recode!"
	ENDC

	IFNE	bltbmod-bltcmod-2
	FAIL	"bltbmod does not follow bltcmod; recode!"
	ENDC

	IFNE	bltamod-bltbmod-2
	FAIL	"bltamod does not follow bltbmod; recode!"
	ENDC

	IFNE	bltdmod-bltamod-2
	FAIL	"bltdmod does not follow bltamod; recode!"
	ENDC

** Equates

BLTCOPY		EQU	(ABC!ABNC!NABC!NANBC)
BLTEOR		EQU	(ABNC!ANBC!NABC!NANBC)

*****i* cdg.library/_TVEORFont ****************************************
*
*   NAME
*	_TVEORFont -- EOR a 6x12 graphic in 2 colors
*   
*   SYNOPSIS
*	VOID _TVEORFont( data, pack )
*	                  A6    A4
*
*	VOID _TVEORFont( struct CDGData *, struct CDGPack * );
*
*   FUNCTION
*	EOR a 6 bit wide, by 12 bit high graphic in 2 colors at
*	COLUMN & ROW position (part of the data).
*
*   INPUTS
*	data - Pointer to CDGData structure.
*
*	pack - Pointer to a CDGPack structure.  The mode, and instruction
*	       bytes have already been decoded.
*
*   RETURNS
*	N/A
*
*   NOTES
*	EOR is equivalent to the WriteFont function, except for the
*	LF control byte used in BLTCON0.
*
***********************************************************************

_TVEORFont:

	PRINTF	DBG_ENTRY,<'CDG--_TVEORFont($%lx,$%lx)'>,A6,A4

	DEBUG_TRAP	38

 		lea	cdp_Data(a4),a0

		movem.l	d2-d7,-(sp)

		moveq	#BLTEOR,d7

	; allow time for interrupts

		movem.l	a2-a6,-(sp)
		bra.s	TVWF_Process

*****i* cdg.library/_TVWriteFont **************************************
*
*   NAME
*	_TVWriteFont -- Draw a 6x12 graphic in 2 colors
*   
*   SYNOPSIS
*	VOID _TVWriteFont( data, pack )
*	                    A6    A4
*
*	VOID _TVWriteFont( struct CDGData *, struct CDGPack * );
*
*   FUNCTION
*	Draw a 6 bit wide, by 12 bit high graphic in 2 colors at
*	COLUMN & ROW position (part of the data).
*
*   INPUTS
*	data - Pointer to CDGData structure.
*
*	pack - Pointer to a CDGPack structure.  The mode, and instruction
*	       bytes have already been decoded.
*
*   RETURNS
*	N/A
*
*   NOTES
*
***********************************************************************

_TVWriteFont:

	PRINTF	DBG_ENTRY,<'CDG--_TVWriteFont($%lx,$%lx)'>,A6,A4

	DEBUG_TRAP	6

 		lea	cdp_Data(a4),a0

		movem.l	d2-d7,-(sp)

		move.w	#BLTCOPY,d7

	; allow time for interrupts

		movem.l	a2-a6,-(sp)

TVWF_Process:

	; extract channel #, and color - quickly ignore channels
	; we are not drawing

		move.b	(a0)+,d2	;CH0 & COLOR0
		move.b	(a0)+,d3	;CH1 & COLOR1
		move.b	d3,d4		;move CH1 -> D4
		lsr.b	#4,d4		;move RS->VW; also clear RSTU
		move.b	d2,d0
		and.b	#(SYMF_R!SYMF_S),d0
		lsr.b	#2,d0		;RS->TU
		or.b	d0,d4		;add TU

	; if result is 0, then its channel 0, and we always draw it

		beq.s	TVWF_OKChannel

	; else compare with secondary channel

		cmp.b	cdd_ActiveChannel(a6),d4
		bne	TVWF_Exit

TVWF_OKChannel:

	; extract ROW, and COLUMN

		moveq	#00,d0
		move.b	(a0)+,d0	;cache ROW

	; only 5 bits are valid in ROW

		andi.b	#(SYMF_S!SYMF_T!SYMF_U!SYMF_V!SYMF_W),d0

		cmp.b	#(DISPLAY_ROWS-),d0		;sanity check
		bhi	TVWF_Exit			;gack!!!


		moveq	#00,d1
		move.b	(a0)+,d1	;and COLUMN

		cmp.b	#(DISPLAY_COLUMNS-1),d1		;sanity check
		bhi	TVWF_Exit			;gack!!!

	; copy FONT data so the blitter can get at it; also make
	; NOT mask

		movem.l	cdd_FontPlane(a6),a3/a4

		moveq	#02,d5
		move.b	#%11111100,d6

		; faster to do this INLINE

		move.b	(a0)+,d4
		lsl.b	d5,d4		;left justify
		move.b	d4,(a3)
		eor.b	d6,d4		;make inverse pattern
		move.b	d4,(a4)

		move.b	(a0)+,d4
		lsl.b	d5,d4		;left justify
		move.b	d4,04(a3)
		eor.b	d6,d4		;make inverse pattern
		move.b	d4,04(a4)

		move.b	(a0)+,d4
		lsl.b	d5,d4		;left justify
		move.b	d4,08(a3)
		eor.b	d6,d4		;make inverse pattern
		move.b	d4,08(a4)

		move.b	(a0)+,d4
		lsl.b	d5,d4		;left justify
		move.b	d4,12(a3)
		eor.b	d6,d4		;make inverse pattern
		move.b	d4,12(a4)

		move.b	(a0)+,d4
		lsl.b	d5,d4		;left justify
		move.b	d4,16(a3)
		eor.b	d6,d4		;make inverse pattern
		move.b	d4,16(a4)

		move.b	(a0)+,d4
		lsl.b	d5,d4		;left justify
		move.b	d4,20(a3)
		eor.b	d6,d4		;make inverse pattern
		move.b	d4,20(a4)

		move.b	(a0)+,d4
		lsl.b	d5,d4		;left justify
		move.b	d4,24(a3)
		eor.b	d6,d4		;make inverse pattern
		move.b	d4,24(a4)

		move.b	(a0)+,d4
		lsl.b	d5,d4		;left justify
		move.b	d4,28(a3)
		eor.b	d6,d4		;make inverse pattern
		move.b	d4,28(a4)

		move.b	(a0)+,d4
		lsl.b	d5,d4		;left justify
		move.b	d4,32(a3)
		eor.b	d6,d4		;make inverse pattern
		move.b	d4,32(a4)

		move.b	(a0)+,d4
		lsl.b	d5,d4		;left justify
		move.b	d4,36(a3)
		eor.b	d6,d4		;make inverse pattern
		move.b	d4,36(a4)

		move.b	(a0)+,d4
		lsl.b	d5,d4		;left justify
		move.b	d4,40(a3)
		eor.b	d6,d4		;make inverse pattern
		move.b	d4,40(a4)

		move.b	(a0)+,d4
		lsl.b	d5,d4		;left justify
		move.b	d4,44(a3)
		eor.b	d6,d4		;make inverse pattern
		move.b	d4,44(a4)

	; The "FONT" data (really a 6x12 graphic image) is
	; a single bit-plane image in the PACK.  A bit set
	; to '1' is drawn in color 1, and a bit set to 0
	; is drawn in color 0.  Color 1, and Color 0 are
	; PEN numbers (also stored in the PACK).
	;
	; Knowing this, there are basically 4 cases.
	;
	;	1.) FONT bitplane -> SCREEN bitplane
	;	2.) NOT FONT bitplane -> SCREEN bitplane
	;	3.) 1's -> SCREEN bitplane
	;	4.) 0's -> SCREEN bitplane
	;
	;
	; The rule is simple enough --
	;
	; FOR COLOR 0
	;
	;	Wherever a bit is TRUE in the PEN #, use the
	;	INVERSE FONT pattern as SOURCE
	;
	; FOR COLOR 1
	;
	;	Wherever a bit is TRUE in the PEN #, use the
	;	ORIGINAL FONT pattern as SOURCE
	;
	; FOR BOTH PENS
	;
	;	Wherever a bit is TRUE in BOTH PEN #, use a
	;	plane of 1's as a pattern for SOURCE
	;
	;	Whereever a bit is FALSE in BOTH PEN #, use a
	;	plane of 0's as a pattern for SOURCE
	;
	; Hence we end up doing 4 blits, not 8 like the original
	; CD+G code
	;

		
		move.l	cdd_ActivePlanes(a6),a5

TVWF_getplanes:

	; calculate bitplane offset for rows, and blitter control

		moveq	#00,d4
		lea	TVWF_RowTable(pc),a1
		add.b	d0,d0			;5 bits -> 6 bits
		move.w	0(a1,d0.w),d4
		lea	TVWF_ColTable(pc),a1
		add.b	d1,d1			;6 bits -> 7 bits
		add.w	0(a1,d1.w),d4

		lea	TVWF_BltB1Table(pc),a1
		or.w	0(a1,d1.w),d7

	; final set-up

		movem.l	cdd_1sPlane(a6),d5/d6
		move.l	cdd_GfxLib(a6),a6
	;
	; At this point we have the following in registers -
	;
	; A3 -> ORIGINAL data in CHIP RAM, JUSTIFIED, MASKED
	; A4 -> INVERSE  data in CHIP RAM, JUSTIFIED, MASKED
	; A5 -> Points -> 4 pointers to bitplane memory to draw in
	; A6 -> GfxBase
	;
	; D2 -> PEN 0
	; D3 -> PEN 1
	; D4 -> Bitplane offset (calced from ROW & COLUMN)
	; D5 -> Plane of 1's
	; D6 -> Plane of 0's
	; D7 -> Blitter A&B SHIFT for BPLCON0, and BPLCON1
	;       OR'ed with ENABLE (BPLCON0), OR'ed with LF


		JSRLIB	OwnBlitter
		JSRLIB	WaitBlit

		lea	_custom,a1

	; Start first BLIT

		; figure out which plane of data to use as source

		move.l	d6,a0		;assume 0's plane

		; use original pattern?

		btst	#0,d3
		beq.s	TVWF_BlitC1
		move.l	a3,a0		;use original pattern


TVWF_BlitC1:
		; or use NOT pattern?

		btst	#0,d2
		beq.s	TVWF_BlitC2

		; or use Plane of 1's?
		
		cmp.l	a0,d6
		movea.l	a4,a0		;does not affect CC
		beq.s	TVWF_BlitC2

		move.l	d5,a0

TVWF_BlitC2:

	; A5 points to source, and now we get pointer to destination

		move.l	(a5)+,d0
		add.l	d4,d0
		
		; poke blitter control registers

		lea	bltcon0(a1),a2

		move.w	d7,d1
		move.w	d7,(a2)+			;bltcon0
		andi.w	#$f000,d1			;mask all except B SHIFT
		move.w	d1,(a2)+			;bltcon1

		; set-up d1 for use as masks, and lower word as 0 constant

		move.l	#$FC000000,d1

		move.l	d1,(a2)+			;bltafwm+lwn
		move.l	d0,(a2)+			;bltcpt
		move.l	a0,(a2)+			;bltbpt
		addq.w	#4,a2				;bltapt
		move.l	d0,(a2)				;bltdpt

		moveq	#((DISPLAY_WIDTH/8)-4),d7

		lea	bltcmod(a1),a2
		move.w	d7,(a2)+			;bltcmod
		move.w	d1,(a2)+			;bltbmod
		addq.w	#2,a2				;bltamod
		move.w	d7,(a2)				;bltdmod

		move.w	#$FFFF,bltadat(a1)		;preset A data

		move.w	#((FONT_HEIGHT<<HSIZEBITS)!(2)),d7

		move.w	d7,bltsize(a1)			;start blit

	;;;
	; Set-up for PLANE 1 while PLANE 0 is busy
	;
	; Note that this code executes in parallel with the blitter,
	; so has the advantage of being virtually FREE in terms of
	; TIME required.  Same for the next planes!
	;
	; Analyzer timing shows nearly perfect parallel processing
	; when the code is running out of CHIP ram, and better
	; is expected when running out of ROM (no DMA contention).
	; Generally 1-2 extra busy loop checks are required in
	; WaitBlit() before the blitter finishes, and WaitBlit()
	; returns (per examination with Analyzer).
	;
	; The only real cost is the RTS from WaitBlit() which requires
	; a memory fetch which would not be needed IF WaitBlit
	; like code was put inline.  Worth considering, but not
	; enough benefit to justify ignoring the OS routine at
	; this time (particularly considering the LONG history of
	; problems detecting blitter busy status).
	;

	;;;
		; figure out which plane of data to use as source

		move.l	d6,a0		;assume 0's plane

		; use original pattern?

		btst	#1,d3
		beq.s	TVWF_BlitC3
		move.l	a3,a0		;use original pattern

TVWF_BlitC3:
		; or use NOT pattern?

		btst	#1,d2
		beq.s	TVWF_BlitC4

		; or use Plane of 1's?

		cmp.l	a0,d6
		move.l	a4,a0		;does not affect CC
		beq.s	TVWF_BlitC4
		move.l	d5,a0

TVWF_BlitC4:

	; A5 points to source, and now we get pointer to destination

		move.l	(a5)+,d0
		add.l	d4,d0
		
		lea	bltcpt(a1),a2

	; ACK - turns out that WaitBlit() never worked right
	; with a 5-6 bitplane screen (and still doesn't).  This is a kludge
	; to work around the problem.  Since our blits are relatively short,
	; this bit of lost CPU time doesn't hurt anything.

		move.w	#(DMAF_SETCLR!DMAF_BLITHOG),dmacon(a1)

		JSRLIB	WaitBlit		;d0-d1/a0-a1 PRESERVED!

		move.w	#(DMAF_BLITHOG),dmacon(a1)

		move.l	d0,(a2)+		;bltcpt
		move.l	a0,(a2)+		;bltbpt
		addq.w	#4,a2			;bltapt
		move.l	d0,(a2)			;bltdpt

		move.w	d7,bltsize(a1)		;start blit


	; Set-up for PLANE 2 while PLANE 1 is busy

		; figure out which plane of data to use as source

		move.l	d6,a0		;assume 0's plane

		; use original pattern?

		btst	#2,d3
		beq.s	TVWF_BlitC5
		move.l	a3,a0		;use original pattern

TVWF_BlitC5:
		; or use NOT pattern?

		btst	#2,d2
		beq.s	TVWF_BlitC6

		; or use Plane of 1's?

		cmp.l	a0,d6
		move.l	a4,a0		;does not affect CC
		beq.s	TVWF_BlitC6
		move.l	d5,a0

TVWF_BlitC6:

	; A5 points to source, and now we get pointer to destination

		move.l	(a5)+,d0
		add.l	d4,d0
		
		lea	bltcpt(a1),a2

		move.w	#(DMAF_SETCLR!DMAF_BLITHOG),dmacon(a1)

		JSRLIB	WaitBlit		;d0-d1/a0-a1 PRESERVED!

		move.w	#(DMAF_BLITHOG),dmacon(a1)

		move.l	d0,(a2)+		;bltcpt
		move.l	a0,(a2)+		;bltbpt
		addq.w	#4,a2			;bltapt
		move.l	d0,(a2)			;bltdpt

		move.w	d7,bltsize(a1)		;start blit

	; Set-up for PLANE 3 while PLANE 2 is busy

		; figure out which plane of data to use as source

		move.l	d6,a0		;assume 0's plane

		; use original pattern?

		btst	#3,d3
		beq.s	TVWF_BlitC7
		move.l	a3,a0		;use original pattern

TVWF_BlitC7:
		; or use NOT pattern?

		btst	#3,d2
		beq.s	TVWF_BlitC8

		; or use Plane of 1's?

		cmp.l	a0,d6
		move.l	a4,a0		;does not affect CC
		beq.s	TVWF_BlitC8
		move.l	d5,a0

TVWF_BlitC8:

	; A5 points to source, and now we get pointer to destination

		move.l	(a5)+,d0
		add.l	d4,d0
		
		lea	bltcpt(a1),a2

		move.w	#(DMAF_SETCLR!DMAF_BLITHOG),dmacon(a1)

		JSRLIB	WaitBlit		;d0-d1/a0-a1 PRESERVED!

		move.w	#(DMAF_BLITHOG),dmacon(a1)

		move.l	d0,(a2)+		;bltcpt
		move.l	a0,(a2)+		;bltbpt
		addq.w	#4,a2			;bltapt
		move.l	d0,(a2)			;bltdpt

		move.w	d7,bltsize(a1)		;start blit

	;;;
	; Perhaps this last WaitBlit() isn't necessary, however
	; it requires a bit of thought (to be looked in too).  We
	; are only going to wait approximately 28 micro-seconds, so
	; its not bad.
	;
	; If this code is removed, then this routine needs to make sure
	; the blitter is done before it modifies CHIP ram buffers containing
	; the original FONT data, and the inverted data.
	;
	; An interesting idea would be to call DisownBlitter, and then
	; WaitBlit(); during the average case we'd save a few cycles,
	; but the problem is that also leaves a small timing hole.
	;
	; Should we take a task switch between the two calls, we could
	; end up waiting for some other tasks LONG blit to finish.
	; On the other hand, CD+G will be running in a virtual single
	; tasking environment, so such would rarely (probably never)
	; be the case.  An optimization to consider for later.
	;
	; Note that if you do change this code, you'll have to rearrange
	; things above since A6 is being used to reference cdg.library
	; data before the first graphics.library call.
	;;
		move.w	#(DMAF_SETCLR!DMAF_BLITHOG),dmacon(a1)

		JSRLIB	WaitBlit		;d0-d1/a0-a1 PRESERVED!

		move.w	#(DMAF_BLITHOG),dmacon(a1)

		JSRLIB	DisownBlitter

TVWF_Exit:
		movem.l	(sp)+,a2-a6
		movem.l	(sp)+,d2-d7

	PRINTF	DBG_EXIT,<'CDG--Exit _TVWriteFont()'>

		rts



	SECTION	DATA
**
** Precalculated SHIFT, and ENABLE for BLTCON0, and BLTCON1 (where only
** the SHIFT is needed for BLTCON1, and the LF control bits are pre-determined
** in the code above.
**


CALCBLTC1	MACRO	; COLUMN #
		dc.w	(((((\1*FONT_WIDTH)+10)&%00001111)<<BSHIFTSHIFT)!(SRCB!SRCC!DEST))
		ENDM

TVWF_BltB1Table:

		CALCBLTC1	00
		CALCBLTC1	01
		CALCBLTC1	02
		CALCBLTC1	03
		CALCBLTC1	04
		CALCBLTC1	05
		CALCBLTC1	06
		CALCBLTC1	07
		CALCBLTC1	08
		CALCBLTC1	09
		CALCBLTC1	10
		CALCBLTC1	11
		CALCBLTC1	12
		CALCBLTC1	13
		CALCBLTC1	14
		CALCBLTC1	15
		CALCBLTC1	16
		CALCBLTC1	17
		CALCBLTC1	18
		CALCBLTC1	19
		CALCBLTC1	20
		CALCBLTC1	21
		CALCBLTC1	22
		CALCBLTC1	23
		CALCBLTC1	24
		CALCBLTC1	25
		CALCBLTC1	26
		CALCBLTC1	27
		CALCBLTC1	28
		CALCBLTC1	29
		CALCBLTC1	30
		CALCBLTC1	31
		CALCBLTC1	32
		CALCBLTC1	33
		CALCBLTC1	34
		CALCBLTC1	35
		CALCBLTC1	36
		CALCBLTC1	37
		CALCBLTC1	38
		CALCBLTC1	39
		CALCBLTC1	40
		CALCBLTC1	41
		CALCBLTC1	42
		CALCBLTC1	43
		CALCBLTC1	44
		CALCBLTC1	45
		CALCBLTC1	46
		CALCBLTC1	47
		CALCBLTC1	48
		CALCBLTC1	49
		CALCBLTC1	50

**
** Precalculated ROW offsets
**

CALCROW		MACRO	; ROW #
		dc.w	((DISPLAY_WIDTH/8)*(\1*FONT_HEIGHT))
		ENDM

TVWF_RowTable:
		CALCROW	00
		CALCROW	01
		CALCROW	02
		CALCROW	03
		CALCROW	04
		CALCROW	05
		CALCROW	06
		CALCROW	07
		CALCROW	08
		CALCROW	09
		CALCROW	10
		CALCROW	11
		CALCROW	12
		CALCROW	13
		CALCROW	14
		CALCROW	15
		CALCROW	16
		CALCROW	17
		CALCROW	18
		CALCROW	19

**
** Precalculated COLUMN offsets
**
** Calculate WORD offset into destination bitplane for a given
** COLUMN
**

CALCCOL		MACRO	; COL #
		dc.w	((((\1*FONT_WIDTH)+10)/16)*2)
		ENDM

TVWF_ColTable:
		CALCCOL	00
		CALCCOL	01
		CALCCOL	02
		CALCCOL	03
		CALCCOL	04
		CALCCOL	05
		CALCCOL	06
		CALCCOL	07
		CALCCOL	08
		CALCCOL	09
		CALCCOL	10
		CALCCOL	11
		CALCCOL	12
		CALCCOL	13
		CALCCOL	14
		CALCCOL	15
		CALCCOL	16
		CALCCOL	17
		CALCCOL	18
		CALCCOL	19
		CALCCOL	20
		CALCCOL	21
		CALCCOL	22
		CALCCOL	23
		CALCCOL	24
		CALCCOL	25
		CALCCOL	26
		CALCCOL	27
		CALCCOL	28
		CALCCOL	29
		CALCCOL	30
		CALCCOL	31
		CALCCOL	32
		CALCCOL	33
		CALCCOL	34
		CALCCOL	35
		CALCCOL	36
		CALCCOL	37
		CALCCOL	38
		CALCCOL	39
		CALCCOL	40
		CALCCOL	41
		CALCCOL	42
		CALCCOL	43
		CALCCOL	44
		CALCCOL	45
		CALCCOL	46
		CALCCOL	47
		CALCCOL	48
		CALCCOL	49
		CALCCOL	50

		END