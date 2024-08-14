**
**	$Id: scroll.asm,v 1.9 92/03/11 12:22:56 darren Exp Locker: darren $
**
**	CDTV CD+G -- preset.asm (scroll with preset)
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

		XDEF	_TVScrollPreset
		XDEF	_TVScrollCopy
** Imports
		XREF	_custom			;amiga.lib

** Equates

DISPLAY_BYTES	EQU	(DISPLAY_WIDTH/8)
DISPLAY_WORDS	EQU	(DISPLAY_BYTES/2)
SCREEN_ROWS	EQU	(DISPLAY_ROWS*FONT_HEIGHT)
SCREEN_END	EQU	((SCREEN_ROWS)*DISPLAY_BYTES)
DONTPRESET	EQU	-1

** Structures

	; structure for storing blitter scroll set-up info
	; MUST be 32 bytes long for quick look-up

 STRUCTURE	BLIT_CONSTANTS,0
	UWORD	bcc_bltcon0
	UWORD	bcc_bltcon1
	UWORD	bcc_modulo
	UWORD	bcc_bltsize
	UWORD	bcc_presetcol
	UWORD	bcc_copycol
	ULONG	bcc_aflwm
	ULONG	bcc_aoffset
	ULONG	bcc_doffset
	ULONG	bcc_presetrow
	ULONG	bcc_copyrow
	LABEL	BLIT_CONSTANTS_SIZEOF

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

	IFNE	BLIT_CONSTANTS_SIZEOF-32
	FAIL	"BLIT_CONSTANTS_SIZEOF != 32; recode!"
	ENDC

*****i* cdg.library/_TVScrollCopy *************************************
*
*   NAME
*	_TVScrollCopy -- CD+G Scroll with COPY
*   
*   SYNOPSIS
*	VOID _TVScrollCopy( data, pack )
*	                      A6    A4
*
*	VOID _TVScrollCopy( struct CDGData *, struct CDGPack * );
*
*   FUNCTION
*	Perform simple, or hard scroll with COPY.  COPY means
*	that the vacated area of the screen is filled with a COPY
*	of the data which is about to be scrolled off.
*
*	See TVScrollPreset() for more info.
*
*   RETURNS
*	N/A
*
*   NOTES
*
***********************************************************************

_TVScrollCopy:

	PRINTF	DBG_ENTRY,<'CDG--_TVScrollCopy($%lx,$%lx)'>,A6,A4

 		lea	cdp_Data+1(a4),a0

		movem.l	d2-d7,-(sp)
		movem.l	a2-a5,-(sp)

	; obtain COPH & PH

		moveq	#00,d0
		move.b	(a0)+,d0
		lsl.b	#2,d0			;6->8 bits

		lea	TVSCR_HTable(pc),a1
		move.l	0(a1,d0.w),d2
		bmi	TVSCC_Exit

	; obtain COPV & PV

		move.b	(a0)+,d0
		lsl.b	#2,d0			;6->8 bits

		lea	TVSCR_VTable(pc),a1
		move.l	0(a1,d0.w),d3
		bmi	TVSCC_Exit


	; We now have VALID COPH, COPV, PH, and PV values
	;
	; We can worry about PH, and PV later.  All we really
	; care about right now is do we need to do a scroll
	; which requires memory copy
	; 

		
		moveq	#00,d7			;assume not combo scroll

		move.w	d3,d5

		lsl.b	#2,d5
		beq.s	TVSCC_NotCombo
		tst.b	d2
		beq.s	TVSCC_NotCombo

		moveq	#01,d7

TVSCC_NotCombo:
		or.b	d2,d5
		beq	TVSCC_FastScroll

                lsl.w	#5,d5			;* size of struct(BLIT_CONSTANTS)


	; A memory copy is required.  Own blitter, and use
	; an optimized copy routine.

		move.l	a6,a2
		move.l	cdd_GfxLib(a2),a6

		JSRLIB	OwnBlitter
		JSRLIB	WaitBlit

	; clear bitplane swap bit (so VBLANK won't try to swap
	; planes while we are in the process of blitting

		bclr	#VBFB_NEWPLANES,cdd_VBlankFlags(a2)

		lea	_custom,a1

		movem.l	cdd_InactivePlanes(a2),a3-a4

		moveq	#03,d6				;loop * 4

		lea	TVSCR_Blits(pc),a5
		add.w	d5,a5

		move.l	bcc_bltcon0(a5),bltcon0(a1)	;+ bltcon1

		move.l	bcc_aflwm(a5),bltafwm(a1)	;+ lwm

		move.w	bcc_modulo(a5),d0

		move.w	d0,bltamod(a1)
		move.w	d0,bltdmod(a1)

TVSCC_Blit4:
		move.l	(a4)+,d0

		move.l	d0,d4

		add.l	bcc_aoffset(a5),d0

		move.l	d0,bltapt(a1)

		move.l	(a3)+,d0

		move.l	d0,d5

		add.l	bcc_doffset(a5),d0

		move.l	d0,bltdpt(a1)

		move.w	bcc_bltsize(a5),bltsize(a1)

	; Peform memory COPY of ROW while this blit is running

		move.l	bcc_presetrow(a5),d1
		bmi.s	TVSCC_DontCopyRow

		tst.w	d7
		bne	TVSCC_SlowComboCopy

		move.l	d1,a1
		add.l	d5,a1

		move.l	d4,a0
		add.l	bcc_copyrow(a5),a0

		move.w	#(((FONT_HEIGHT*DISPLAY_BYTES)/4)-1),d1


TVSCC_DoCopyRow:
		move.l	(a0)+,(a1)+
		dbf	d1,TVSCC_DoCopyRow

TVSCC_DontCopyRow:

	; Perform memory COPY of COLUMN while bit is running

		move.l	d5,a1
		move.w	bcc_presetcol(a5),d1
		bmi.s	TVSCC_DontCopyCol

		add.w	d1,a1

		move.l	d4,a0
		move.w	bcc_copycol(a5),d0
		add.w	d0,a0

		moveq	#(DISPLAY_BYTES),d5
		moveq	#10,d4			;shift

		cmp.w	d0,d1
		blt.s	TVSCC_DoCopyLeft

		move.w	#((FONT_HEIGHT*DISPLAY_ROWS)-1),d1

TVSCC_DoCopyRight:
		move.w	(a0),d0
		lsl.w	d4,d0
		move.w	d0,(a1)
		add.w	d5,a0
		add.w	d5,a1
		dbf	d1,TVSCC_DoCopyRight
		bra.s	TVSCC_DontCopyCol

TVSCC_DoCopyLeft:
		move.w	#((FONT_HEIGHT*DISPLAY_ROWS)-1),d1

TVSCC_DoCopyLeftI:
		move.w	(a0),d0
		lsr.w	d4,d0
		move.w	d0,(a1)
		add.w	d5,a0
		add.w	d5,a1
		dbf	d1,TVSCC_DoCopyLeftI

TVSCC_DontCopyCol:

		lea	_custom,a1

	;
	; Wait for this blit, and the last blit to be done.  This way
	; we don't swap bitplanes until we are visually ready
	;
		JSRLIB	WaitBlit

		dbf	d6,TVSCC_Blit4

		JSRLIB	DisownBlitter

	; And swap Active/Inactive pointers

		movem.l	cdd_InactivePlanes(a2),d0/a0
		exg	d0,a0
		movem.l	d0/a0,cdd_InactivePlanes(a2)

		move.l	a2,a6			;restore A6

	; Change PH & PV values; update screen during VBLANK

TVSCC_FastScroll:


	; Calculate new PH, and PV

		swap	d3
		add.w	d3,d3
		lea	TVScroll_PVTable(pc),a0
		move.w	0(a0,d3.w),d1

		swap	d2
		add.w	d2,d2
		lea	TVScroll_PHTable(pc),a0
		move.w	0(a0,d2.w),d0
		bne.s	TVSCC_ValidScroll

		subq.w	#2,d1
	;	moveq	#00,d0

TVSCC_ValidScroll:
		move.w	d1,cdd_PHPV+2(a6)
		move.w	d0,cdd_PHPV(a6)


	; Tell VBLANK to modify screen pointers

		bset	#VBFB_NEWPLANES,cdd_VBlankFlags(a6)
		
TVSCC_Exit:

		movem.l	(sp)+,a2-a5
		movem.l	(sp)+,d2-d7

	PRINTF	DBG_EXIT,<'CDG--Exit _TVSCCollPreset()'>

		rts

	; yuck, do combo scrolls in 1 pass during blit

TVSCC_SlowComboCopy:

		move.l	d7,-(sp)

		moveq	#6,d7		;shift
		move.w	#(((FONT_HEIGHT*DISPLAY_BYTES)/2)-1),d1

		cmp.b	#2,d2
		beq.s	TVSCC_SlowRowRight

	; copy/shift row 6 pixels

TVSCC_LeftCopy:

		move.l	(a0),d0
		lsl.l	d7,d0
		swap	d0
		move.w	d0,(a1)+
		addq.w	#2,a0

		dbf	d1,TVSCC_LeftCopy
		bra.s	TVSCC_CopyCorner

TVSCC_SlowRowRight:

		move.w	#(FONT_HEIGHT*DISPLAY_BYTES),d0

		adda.w	d0,a0
		adda.w	d0,a1

TVSCC_RightCopy:
		
		move.l	(a0),d0
		lsr.l	d7,d0
		move.w	d0,-(a1)
		subq.w	#2,a0

		dbf	d1,TVSCC_RightCopy

TVSCC_CopyCorner:

	; copy corner

		move.l	d4,a0
		add.w	bcc_copycol(a5),a0
		add.l	bcc_copyrow(a5),a0

		move.l	d5,a1
		add.w	bcc_presetcol(a5),a1
		add.l	bcc_presetrow(a5),a1

		move.w	#(FONT_HEIGHT-1),d1

		moveq	#10,d7

		cmp.b	#2,d2
		beq.s	TVSCC_SlowCornerRight

TVSCC_CopyCornerLeft:
		move.w	(a0)+,d0
		lsl.w	d7,d0
		move.w	d0,(a1)+
		dbf	d1,TVSCC_CopyCornerLeft




*****i* cdg.library/_TVScrollPreset ***********************************
*
*   NAME
*	_TVScrollPreset -- CD+G Scroll with PRESET
*   
*   SYNOPSIS
*	VOID _TVScrollPreset( data, pack )
*	                      A6    A4
*
*	VOID _TVScrollPreset( struct CDGData *, struct CDGPack * );
*
*   FUNCTION
*	Perform simple, or hard scroll with PRESET.  PRESET means
*	that the vacated area of the screen is filled with a PRESET
*	color encoded in the PACK data.  
*
*	There is a neat trick which is taken advantage of by this
*	routine.  It turns out that the first, and last columns
*	of the display fit within their own 16 bit words using a
*	320 pixel wide bitplane.
*
*	48 visible columns * 6 pixels wide == 288 visible pixels
*
*	16 pixels (for column 0) + 16 pixels for (column 1) == 32 pixels.
*
*	288 pixels + 32 pixels == 320 pixels.
*
*	Because a double buffered display is used, it is possible
*	to perform the memory copy such that the vacated area can be
*	filled with the CPU in parallel with the blitter copy.  Since
*	the blitter need only do a simple A->D copy, the copy can be
*	fairly efficient.  Turns out the same is true when doing
*	a scroll with COPY.  The destination column can be filled with
*	the CPU using source data which is only being READ by the
*	blitter.
*
*	The caller must first open the cdg.library, and have called
*	CDGBegin() to start CD+G.
*
*   RETURNS
*	N/A
*
*   NOTES
*
***********************************************************************

_TVScrollPreset:

	PRINTF	DBG_ENTRY,<'CDG--_TVScrollPreset($%lx,$%lx)'>,A6,A4

	DEBUG_TRAP	12

 		lea	cdp_Data(a4),a0

		movem.l	d2-d6,-(sp)
		movem.l	a2-a5,-(sp)

		move.b	(a0)+,d4		;color
		andi.b	#(SYMF_T!SYMF_U!SYMF_V!SYMF_W),d4

	; obtain COPH & PH

		moveq	#00,d0
		move.b	(a0)+,d0
		lsl.b	#2,d0			;6->8 bits

		lea	TVSCR_HTable(pc),a1
		move.l	0(a1,d0.w),d2
		bmi	TVSCR_Exit

	; obtain COPV & PV

		move.b	(a0)+,d0
		lsl.b	#2,d0			;6->8 bits

		lea	TVSCR_VTable(pc),a1
		move.l	0(a1,d0.w),d3
		bmi	TVSCR_Exit


	; We now have VALID COPH, COPV, PH, and PV values
	;
	; We can worry about PH, and PV later.  All we really
	; care about right now is do we need to do a scroll
	; which requires memory copy
	; 

		move.w	d3,d5

		lsl.b	#2,d5
		or.b	d2,d5
		beq	TVSCR_FastScroll

                lsl.w	#5,d5			;* size of struct(BLIT_CONSTANTS)


	; A memory copy is required.  Own blitter, and use
	; an optimized copy routine.

		move.l	a6,a2
		move.l	cdd_GfxLib(a2),a6

		JSRLIB	OwnBlitter
		JSRLIB	WaitBlit

	; clear bitplane swap bit (so VBLANK won't try to swap
	; planes while we are in the process of blitting

		bclr	#VBFB_NEWPLANES,cdd_VBlankFlags(a2)

		lea	_custom,a1

		movem.l	cdd_InactivePlanes(a2),a3-a4

		moveq	#03,d6				;loop * 4

		lea	TVSCR_Blits(pc),a5
		add.w	d5,a5

		move.l	bcc_bltcon0(a5),bltcon0(a1)	;+ bltcon1

		move.l	bcc_aflwm(a5),bltafwm(a1)	;+ lwm

		move.w	bcc_modulo(a5),d0

		move.w	d0,bltamod(a1)
		move.w	d0,bltdmod(a1)

TVSCR_Blit4:
		move.l	(a4)+,d0

		add.l	bcc_aoffset(a5),d0

		move.l	d0,bltapt(a1)

		move.l	(a3)+,d0

		move.l	d0,d5

		add.l	bcc_doffset(a5),d0

		move.l	d0,bltdpt(a1)

		move.w	bcc_bltsize(a5),bltsize(a1)

	; Peform memory PRESET of ROW while this blit is running

		moveq	#00,d0
		lsr.b	#1,d4			;obtain 1's, or 0's for this
		bcc.s	TVSCR_PresetColor	;plane from PRESET PEN #
		moveq	#-1,d0
TVSCR_PresetColor:

		move.l	bcc_presetrow(a5),d1
		bmi.s	TVSCR_DontPresetRow
		move.l	d1,a0
		add.l	d5,a0

		move.w	#(((FONT_HEIGHT*DISPLAY_BYTES)/4)-1),d1

TVSCR_DoPresetRow:
		move.l	d0,(a0)+
		dbf	d1,TVSCR_DoPresetRow

TVSCR_DontPresetRow:

	; Perform memory PRESET of COLUMN while bit is running

		move.l	d5,a0
		move.w	bcc_presetcol(a5),d1
		bmi.s	TVSCR_DontPresetCol

		add.w	d1,a0

		move.w	#((FONT_HEIGHT*DISPLAY_ROWS)-1),d1

		moveq	#(DISPLAY_BYTES),d5
TVSCR_DoPresetCol:
		move.w	d0,(a0)
		add.w	d5,a0
		dbf	d1,TVSCR_DoPresetCol

TVSCR_DontPresetCol:

	;
	; Wait for this blit, and the last blit to be done.  This way
	; we don't swap bitplanes until we are visually ready
	;
		JSRLIB	WaitBlit

		dbf	d6,TVSCR_Blit4

		JSRLIB	DisownBlitter

	; And swap Active/Inactive pointers

		movem.l	cdd_InactivePlanes(a2),d0/a0
		exg	d0,a0
		movem.l	d0/a0,cdd_InactivePlanes(a2)

		move.l	a2,a6			;restore A6

	; Change PH & PV values; update screen during VBLANK

TVSCR_FastScroll:

	; Calculate new PH, and PV

		swap	d3
		add.w	d3,d3
		lea	TVScroll_PVTable(pc),a0
		move.w	0(a0,d3.w),d1

		swap	d2
		add.w	d2,d2
		lea	TVScroll_PHTable(pc),a0
		move.w	0(a0,d2.w),d0
		bne.s	TVSCR_ValidScroll

		subq.w	#2,d1
	;	moveq	#00,d0

TVSCR_ValidScroll:
		move.w	d1,cdd_PHPV+2(a6)
		move.w	d0,cdd_PHPV(a6)

	; Tell VBLANK to modify screen pointers

		bset	#VBFB_NEWPLANES,cdd_VBlankFlags(a6)
		
TVSCR_Exit:

		movem.l	(sp)+,a2-a5
		movem.l	(sp)+,d2-d6

	PRINTF	DBG_EXIT,<'CDG--Exit _TVScrollPreset()'>

		rts

		SECTION DATA

**
** Data tables for blits.  Illegal values for COPH, and COPV have
** already been caught above, so this is not a complete table
**
** COPV<<2|COPH
**

TVSCR_Blits:

	; COPV == 0, COPH == 0 - INVALID

		dc.w	0
		dc.w	0

		dc.w	0
		dc.w	0

		dc.w	0
		dc.w	0

		dc.l	0
		dc.l	0
		dc.l	0
		dc.l	0
		dc.l	0

	; COPV == 0, COPH == 1 -- Copy RIGHT

		dc.w	((10<<ASHIFTSHIFT)!SRCA!DEST!ABC!ABNC!ANBC!ANBNC)
		dc.w	BLITREVERSE

		dc.w	2
		dc.w	((SCREEN_ROWS<<HSIZEBITS)!(DISPLAY_WORDS-1))

		dc.w	0
		dc.w	DISPLAY_BYTES-2

		dc.l	$FFFF003F

		dc.l	SCREEN_END-4
		dc.l	SCREEN_END-2

		dc.l	DONTPRESET
		dc.l	DONTPRESET

	; COPV == 0, COPH == 2 -- Copy LEFT

		dc.w	((10<<ASHIFTSHIFT)!SRCA!DEST!ABC!ABNC!ANBC!ANBNC)
		dc.w	0

		dc.w	2
		dc.w	((SCREEN_ROWS<<HSIZEBITS)!(DISPLAY_WORDS-1))

		dc.w	DISPLAY_BYTES-2
		dc.w	0

		dc.l	$FFFFFC00

		dc.l	2
		dc.l	0

		dc.l	DONTPRESET
		dc.l	DONTPRESET

	; COPV == 0, COPH == 3 -- INVALID

		dc.w	0
		dc.w	0

		dc.w	0
		dc.w	0

		dc.w	0
		dc.w	0

		dc.l	0
		dc.l	0
		dc.l	0
		dc.l	0
		dc.l	0

	; COPV == 1, COPH == 0 -- Copy DOWN

		dc.w	(SRCA!DEST!ABC!ABNC!ANBC!ANBNC)
		dc.w	0

		dc.w	0
		dc.w	(((SCREEN_ROWS-FONT_HEIGHT)<<HSIZEBITS)!DISPLAY_WORDS)

		dc.w	DONTPRESET
		dc.w	DONTPRESET

		dc.l	$003FFC00

		dc.l	0
		dc.l	(FONT_HEIGHT*DISPLAY_BYTES)

		dc.l	0
		dc.l	((SCREEN_ROWS-FONT_HEIGHT)*DISPLAY_BYTES)

	; COPV == 1, COPH == 1 -- Copy DOWN, and RIGHT


		dc.w	((10<<ASHIFTSHIFT)!SRCA!DEST!ABC!ABNC!ANBC!ANBNC)
		dc.w	BLITREVERSE

		dc.w	2
		dc.w	(((SCREEN_ROWS-FONT_HEIGHT)<<HSIZEBITS)!DISPLAY_WORDS-1)

		dc.w	0
		dc.w	DISPLAY_BYTES-2

		dc.l	$FFFF003F

		dc.l	SCREEN_END-4-(FONT_HEIGHT*DISPLAY_BYTES)
		dc.l	SCREEN_END-2

		dc.l	0
		dc.l	((SCREEN_ROWS-FONT_HEIGHT)*DISPLAY_BYTES)

	; COPV == 1, COPH == 2 -- Copy DOWN, and LEFT

		dc.w	((10<<ASHIFTSHIFT)!SRCA!DEST!ABC!ABNC!ANBC!ANBNC)
		dc.w	0

		dc.w	2
		dc.w	(((SCREEN_ROWS-FONT_HEIGHT)<<HSIZEBITS)!DISPLAY_WORDS-1)

		dc.w	DISPLAY_BYTES-2
		dc.w	0

		dc.l	$FFFFFC00

		dc.l	2
		dc.l	(FONT_HEIGHT*DISPLAY_BYTES)

		dc.l	0
		dc.l	((SCREEN_ROWS-FONT_HEIGHT)*DISPLAY_BYTES)

	; COPV == 1, COPH == 3 -- INVALID

		dc.w	0
		dc.w	0

		dc.w	0
		dc.w	0

		dc.w	0
		dc.w	0

		dc.l	0
		dc.l	0
		dc.l	0
		dc.l	0
		dc.l	0

	; COPV == 2, COPH == 0 -- Copy UP

		dc.w	(SRCA!DEST!ABC!ABNC!ANBC!ANBNC)
		dc.w	0

		dc.w	0
		dc.w	(((SCREEN_ROWS-FONT_HEIGHT)<<HSIZEBITS)!DISPLAY_WORDS)

		dc.w	DONTPRESET
		dc.w	DONTPRESET

		dc.l	$003FFC00

		dc.l	(FONT_HEIGHT*DISPLAY_BYTES)
		dc.l	0

		dc.l	((SCREEN_ROWS-FONT_HEIGHT)*DISPLAY_BYTES)
		dc.l	0

	; COPV == 2, COPH == 1 -- Copy UP, and RIGHT


		dc.w	((10<<ASHIFTSHIFT)!SRCA!DEST!ABC!ABNC!ANBC!ANBNC)
		dc.w	BLITREVERSE

		dc.w	2
		dc.w	(((SCREEN_ROWS-FONT_HEIGHT)<<HSIZEBITS)!DISPLAY_WORDS-1)

		dc.w	0
		dc.w	DISPLAY_BYTES-2

		dc.l	$FFFF003F

		dc.l	SCREEN_END-4
		dc.l	SCREEN_END-2-(FONT_HEIGHT*DISPLAY_BYTES)

		dc.l	((SCREEN_ROWS-FONT_HEIGHT)*DISPLAY_BYTES)
		dc.l	0

	; COPV == 2, COPH == 2 -- Copy UP, and LEFT

		dc.w	((10<<ASHIFTSHIFT)!SRCA!DEST!ABC!ABNC!ANBC!ANBNC)
		dc.w	0

		dc.w	2
		dc.w	(((SCREEN_ROWS-FONT_HEIGHT)<<HSIZEBITS)!DISPLAY_WORDS-1)

		dc.w	DISPLAY_BYTES-2
		dc.w	0

		dc.l	$FFFFFC00

		dc.l	2+(FONT_HEIGHT*DISPLAY_BYTES)
		dc.l	0

		dc.l	((SCREEN_ROWS-FONT_HEIGHT)*DISPLAY_BYTES)
		dc.l	0

	; Table end - all else is invalid, and already screened


**
** Pre calculated look-up table for COPH/PH.  Converts the
** COPH/PH byte into a LONG word consisting of COPH in the lower
** word, and a new PH value in the upper word.  Invalid values
** are also caught as part of the table.
**

INVALID_SCROLL	EQU	-1

SCROLLH		MACRO	;Table entry

		; Note that the AND masks SYMF_T, which is suppose
		; to be 0.  The masking allows it to be set in some
		; future CD+G standard in a upwardly compatable way.
		; The problem is, its not clear if the SYMF_T bit would
		; ever be used, and if so, if it would be used in a
		; backwards compatable fashion.  So I assume that
		; any decent designer will realize the need for backwards
		; compatability, and select that kind of solution.
		;

PHTMP		SET	(\1&(SYMF_U!SYMF_V!SYMF_W))
COPHTMP		SET	((\1&(SYMF_R!SYMF_S))>>4)

HRZTMP		SET	((PHTMP<<16)!COPHTMP)

		IFGT	PHTMP-5		; PH must be 0-5
HRZTMP		SET	INVALID_SCROLL
		ENDC

		IFGT	COPHTMP-2	; COPH must be 0-2
HRZTMP		SET	INVALID_SCROLL
		ENDC

		dc.l	HRZTMP

		ENDM

TVSCR_HTable:
		SCROLLH	00
		SCROLLH	01
		SCROLLH	02
		SCROLLH	03
		SCROLLH	04
		SCROLLH	05
		SCROLLH	06
		SCROLLH	07
		SCROLLH	08
		SCROLLH	09
		SCROLLH	10
		SCROLLH	11
		SCROLLH	12
		SCROLLH	13
		SCROLLH	14
		SCROLLH	15
		SCROLLH	16
		SCROLLH	17
		SCROLLH	18
		SCROLLH	19
		SCROLLH	20
		SCROLLH	21
		SCROLLH	22
		SCROLLH	23
		SCROLLH	24
		SCROLLH	25
		SCROLLH	26
		SCROLLH	27
		SCROLLH	28
		SCROLLH	29
		SCROLLH	30
		SCROLLH	31
		SCROLLH	32
		SCROLLH	33
		SCROLLH	34
		SCROLLH	35
		SCROLLH	36
		SCROLLH	37
		SCROLLH	38
		SCROLLH	39
		SCROLLH	40
		SCROLLH	41
		SCROLLH	42
		SCROLLH	43
		SCROLLH	44
		SCROLLH	45
		SCROLLH	46
		SCROLLH	47
		SCROLLH	48
		SCROLLH	49
		SCROLLH	50
		SCROLLH	51
		SCROLLH	52
		SCROLLH	53
		SCROLLH	54
		SCROLLH	55
		SCROLLH	56
		SCROLLH	57
		SCROLLH	58
		SCROLLH	59
		SCROLLH	60
		SCROLLH	61
		SCROLLH	62
		SCROLLH	63


**
** Pre calculated look-up table for COPV/PV.  Converts the
** COPV/PV byte into a LONG word consisting of COPV in the lower
** word, and a new PV value in the upper word.  Invalid values
** are also caught as part of the table.
**


SCROLLV		MACRO	;Table entry

PVTMP		SET	(\1&(SYMF_T!SYMF_U!SYMF_V!SYMF_W))
COPVTMP		SET	((\1&(SYMF_R!SYMF_S))>>4)

VERTMP		SET	((PVTMP<<16)!COPVTMP)

		IFGT	PVTMP-11	; PV must be 0-11
VERTMP		SET	INVALID_SCROLL
		ENDC

		IFGT	COPVTMP-2	; COPV must be 0-2
VERTMP		SET	INVALID_SCROLL
		ENDC

		dc.l	VERTMP

		ENDM

TVSCR_VTable:
		SCROLLV	00
		SCROLLV	01
		SCROLLV	02
		SCROLLV	03
		SCROLLV	04
		SCROLLV	05
		SCROLLV	06
		SCROLLV	07
		SCROLLV	08
		SCROLLV	09
		SCROLLV	10
		SCROLLV	11
		SCROLLV	12
		SCROLLV	13
		SCROLLV	14
		SCROLLV	15
		SCROLLV	16
		SCROLLV	17
		SCROLLV	18
		SCROLLV	19
		SCROLLV	20
		SCROLLV	21
		SCROLLV	22
		SCROLLV	23
		SCROLLV	24
		SCROLLV	25
		SCROLLV	26
		SCROLLV	27
		SCROLLV	28
		SCROLLV	29
		SCROLLV	30
		SCROLLV	31
		SCROLLV	32
		SCROLLV	33
		SCROLLV	34
		SCROLLV	35
		SCROLLV	36
		SCROLLV	37
		SCROLLV	38
		SCROLLV	39
		SCROLLV	40
		SCROLLV	41
		SCROLLV	42
		SCROLLV	43
		SCROLLV	44
		SCROLLV	45
		SCROLLV	46
		SCROLLV	47
		SCROLLV	48
		SCROLLV	49
		SCROLLV	50
		SCROLLV	51
		SCROLLV	52
		SCROLLV	53
		SCROLLV	54
		SCROLLV	55
		SCROLLV	56
		SCROLLV	57
		SCROLLV	58
		SCROLLV	59
		SCROLLV	60
		SCROLLV	61
		SCROLLV	62
		SCROLLV	63


**
** Precalculated PV lookup table.  Is essentially a Y offset which includes
** the first hidden row, plus N lines of the next row, plus 2 so we skip the
** first hidden COLUMN
**
TVScroll_PVTable:
		dc.w	((((DISPLAY_WIDTH/8)*0)+2)+((DISPLAY_WIDTH/8)*FONT_HEIGHT))
		dc.w	((((DISPLAY_WIDTH/8)*1)+2)+((DISPLAY_WIDTH/8)*FONT_HEIGHT))
		dc.w	((((DISPLAY_WIDTH/8)*2)+2)+((DISPLAY_WIDTH/8)*FONT_HEIGHT))
		dc.w	((((DISPLAY_WIDTH/8)*3)+2)+((DISPLAY_WIDTH/8)*FONT_HEIGHT))
		dc.w	((((DISPLAY_WIDTH/8)*4)+2)+((DISPLAY_WIDTH/8)*FONT_HEIGHT))
		dc.w	((((DISPLAY_WIDTH/8)*5)+2)+((DISPLAY_WIDTH/8)*FONT_HEIGHT))
		dc.w	((((DISPLAY_WIDTH/8)*6)+2)+((DISPLAY_WIDTH/8)*FONT_HEIGHT))
		dc.w	((((DISPLAY_WIDTH/8)*7)+2)+((DISPLAY_WIDTH/8)*FONT_HEIGHT))
		dc.w	((((DISPLAY_WIDTH/8)*8)+2)+((DISPLAY_WIDTH/8)*FONT_HEIGHT))
		dc.w	((((DISPLAY_WIDTH/8)*9)+2)+((DISPLAY_WIDTH/8)*FONT_HEIGHT))
		dc.w	((((DISPLAY_WIDTH/8)*10)+2)+((DISPLAY_WIDTH/8)*FONT_HEIGHT))
		dc.w	((((DISPLAY_WIDTH/8)*11)+2)+((DISPLAY_WIDTH/8)*FONT_HEIGHT))


**
** Precalculated PH lookup table.  Fine scrolling is adjusted by playing
** with bplcon1 in the copper list
**

FINEPHSCROLL	MACRO
		dc.w	(((15-\1)<<PFB_FINE_SCROLL_SHIFT)!(15-\1))
		ENDM
TVScroll_PHTable:


		FINEPHSCROLL	15	;0 means backup 1 word, and delay is 0
		FINEPHSCROLL	0
		FINEPHSCROLL	1
		FINEPHSCROLL	2
		FINEPHSCROLL	3
		FINEPHSCROLL	4

		END



**
** About the scroll routines -
**
**	The scroll routines used double buffering to avoid screen
**	trash during the blits.  This also makes it possible to
**	use a simple A->D blitter copy (perhaps with SHIFT, which
**	is free for the blitter), and also allows us to interleave
**	PRESET or COPY operations with the blits.
**
**	The bitplanes are layed out such that the first FONT column
**	is the last 6 bits of the first WORD, and the last FONT column
**	is the first 6 bits of the last WORD.  Like so -
**
**	111111           111111           111111          
**	5432109876543210 5432109876543210 5432109876543210
**	----------------+----------------+----------------
**                ^    ^                  ^    ^
**                |    |                  |    |
**	1st FONT -------                  ------- last FONT
**
**
**	Therefore to COPY LEFT we have -
**
**	111111           111111           111111           <-src
**	5432109876543210 5432109876543210 5432109876543210
**	----------------+----------------+----------------
**                      /
**                     /
**                    /
**                   /
**		    /
**                 /
**	111111           111111           111111           <-dest
**	5432109876543210 5432109876543210 5432109876543210
**	----------------+----------------+----------------
**
**	A = SRC + 1 WORD
**
**	D = DEST
**
**	Right SHIFT = 10
**
**	ASCENDING MODE
**
**	MODULO = 1 WORD (2 BYTES)
**
**
**
**	Therefore to COPY RIGHT we have -
**
**	111111           111111           111111           <-src
**	5432109876543210 5432109876543210 5432109876543210
**	----------------+----------------+----------------
**                \      
**                 \     
**                  \ 
**                   \
**		      \
**                     \
**	111111           111111           111111           <-dest
**	5432109876543210 5432109876543210 5432109876543210
**	----------------+----------------+----------------
**
**	A = Last Line - 2 WORDS
**
**	D = Last Line - 1 WORD
**
**	Left Shift = 10
**
**	DESCENDING MODE
**
**	MODULO = 1 WORD (2 BYTES)
**
**
**
**	COPY UP/DOWN is as easy as adjusting the SRC (A), and DEST (D)
**	pointers, and combinations are simply combinations.  During
**	COPY UP/DOWN, we copy one less font row.
**
**
**	PRESET --
**
**	Because the first, and last FONTs are contained within their own
**	WORDS, the CPU can PRESET a COLUMN during the BLIT.  ROWS
**	likewise, because this is memory that is not written to (via the
**	blitter during the memory copy.
**
**	COPY --
**
**	Copy is like PRESET, except we fetch the original data from
**	the opposite column/row during the blits.  The data to be copied
**	is in the SRC bitplanes, and is copied to the opposing
**	ROW/COLUMN while the blit (one bitplane at a time) is running.
**
**	This works because the DES memory is not being written to by
**	the blitter.  When doing column copy, the SRC->DEST is shifted
**	L/R 10 pixels (also handled by the CPU).
**
**	Even though this seems complex, the value is that the PRESET and/or
**	COPY portions of the scrolls are free since they are done while
**	the blitter is busy moving the bulk of the data. [Infact these
**	operations are done well before the blitter is done if the CPU
**	is available for use!]
**
**	OPTIMIZATIONS -
**
**	Another optimization is a table look-up scheme to decode PH/COPH,
**	PV/COPV.  While somewhat larger than using just code, it is
**	faster, and simpler.  The tables also mark invalid combinations
**	as -1, and can be easily caught with a BMI test.
**
**	PH/PV values are stored as actual bitplane offset, and scroll
**	register values so these do not need to be looked up during VBLANK
**	when the copper list is updated.
**