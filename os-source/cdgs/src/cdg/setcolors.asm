**
**	$Id: setcolors.asm,v 1.14 93/02/18 16:27:41 darren Exp $
**
**	CDTV CD+G -- setcolors.asm (set border color, and color tables)
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

		XDEF	SniffCopperList

		XDEF	_TVPresetBorder
		XDEF	_TVCLUTLow
		XDEF	_TVCLUTHigh
		XDEF	_TVPresetMemory

** Assumptions

	IFNE	cdc_Color0
	FAIL	"cdc_Color0 not 0; recode!"
	ENDC

	IFNE	cdc_BPL0-cdc_Color0-4
	FAIL	"cdc_BPL0 does not follow cdc_Color0; recode!"
	ENDC

	IFNE	cdc_BPLCON1-cdc_BPL0-4
	FAIL	"cdc_BPL0 does not follow cdc_BPL0; recode!"
	ENDC

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

*****i* cdg.library/_TVPresetBorder ***********************************
*
*   NAME
*	_TVPresetBorder -- Set border color
*   
*   SYNOPSIS
*	VOID _TVPresetBorder( data, pack, ecc )
*	                      A6    A4    D0
*
*	VOID _TVPresetBorder( struct CDGData *, struct CDGPack *, BOOL );
*
*   FUNCTION
*	Set new border color (0-15).
*
*   INPUTS
*	data - Pointer to CDGData structure.
*
*	pack - Pointer to a CDGPack structure.  The mode, and instruction
*	       bytes have already been decoded.
*
*	ecc -  TRUE if the PACK has good data, else FALSE.
*
*   RETURNS
*	N/A
*
*   NOTES
*	This function changes the copper list by changing color 0,
*	hence the change takes effect after the next VBLANK.
*
*	!!!NOTE!!!
*
*	This code has been optimized for speed, but needs to be
*	recoded for an RTG system - not a priority at the time
*	of this writing - speed is!!!  However the code
*	does at least sniff out the position of the color table
*	copper list instructions at init time.
*
***********************************************************************

_TVPresetBorder:

	PRINTF	DBG_ENTRY,<'CDG--_TVPresetBorder($%lx,$%lx)'>,A6,A4

	DEBUG_TRAP	2

		lea	cdp_Data(a4),a0
		lea	cdd_CLUTTable(a6),a1

	; obtain color # for border

		moveq	#00,d0
		move.b	(a0),d0

	; mask all but TUVW (4 bits for pen color to use)

		and.b	#(SYMF_T!SYMF_U!SYMF_V!SYMF_W),d0

	; multiply by 2 for lookup in word size table; since
	; TUVW are the lower bits, a byte wide shift will do

		add.b	d0,d0
		add.w	d0,a1

	; Just save pointer to border pen for now; copper list
	; will be updated during VBLANK with the new color.
	; Since the MOVE.L is ATOMIC, no need to clear the
	; NEWCOLORS flag like we do for CLUTLow, and CLUTHigh

		move.l	a1,cdd_BorderPen(a6)


** Fix Gfx CopInit now so we can avoid top edge border flash
FixGfxCopInit:

	; set flag first incase we take a task switch, or interrupt
	; after the flag set, we will at least be assured of fixing
	; up gfx->copinit within 1 VBLANK

		bset	#VBFB_NEWCOLORS,cdd_VBlankFlags(a6)

	; Average case is we fix this up before next VBLANK.
	;
	; At worst the top border is out of sync with my VP's color
	; 0 for 1 VBLANK - tolerable.

		btst	#VBFB_BACKSCREEN,cdd_VBlankFlags(a6)
		bne.s	CDGV_NoFixCopInit

		move.l	cdd_BorderPen(a6),a0
		move.w	(a0),d0
		move.l	cdd_CopperList1+cdc_COPINITColor0(a6),a0
		move.w	d0,2(a0)
		move.l	cdd_CopperList1+cdc_COPINITColor0b(a6),a0
		move.w	d0,2(a0)

CDGV_NoFixCopInit:

		rts


*****i* cdg.library/_TVCLUTLow ****************************************
*
*   NAME
*	_TVCLUTLow -- Set colors 0-7
*   
*   SYNOPSIS
*	VOID _TVCLUTLow( data, pack, ecc )
*	                  A6    A4   D0
*
*	VOID _TVCLUTLow( struct CDGData *, struct CDGPack *, BOOL );
*
*   FUNCTION
*	Set new colors (0-7).
*
*   INPUTS
*	data - Pointer to CDGData structure.
*
*	pack - Pointer to a CDGPack structure.  The mode, and instruction
*	       bytes have already been decoded.
*
*	ecc -  TRUE if the pack has good data, else FALSE.
*
*   RETURNS
*	N/A
*
*   NOTES
*	This function updates the CLUT table, and tells the VBLANK
*	interrupt to copy the colors to the copper list.
*
*	!!!NOTE!!!
*
*	This code has been optimized for speed, but needs to be
*	recoded for an RTG system - not a priority at the time
*	of this writing - speed is!!!  However the code
*	does at least sniff out the position of the color table
*	copper list instructions at init time.
*
***********************************************************************

_TVCLUTLow:

	PRINTF	DBG_ENTRY,<'CDG--_TVCLUTLow($%lx,$%lx)'>,A6,A4

	DEBUG_TRAP	30

		lea	cdp_Data(a4),a0
		lea	cdd_CLUTTable(a6),a1

	; Tell VBLANK not to bother with updating colors right now.
	;
	; A BIT CLEAR acts like a semaphore in this case, so no
	; DISABLE is needed.
	;
	; Since the colors are moved into the copper list at
	; VBLANK interrupt time, the move is ATOMIC because
	; the following code runs as a TASK, and a TASK cannot
	; interrupt an IRQ!
	;
	; We BIT CLEAR below in case we had already set the
	; BIT, but the previous color table has not yet been
	; copied to the copper list.  The CD+G graphics designer
	; is responsible for recognizing that the colors can only
	; be changed once per VBLANK; changing them more often
	; then VBLANK rate doesn't work anyway (per specification).
	;
		bclr	#VBFB_NEWCOLORS,cdd_VBlankFlags(a6)

	; move colors into CLUT table
	;
	; Colors are stored as 8 entries of RGB in 2 6 bit bytes like so
	;
	; R S   T U   V W
	; RED   3-0   GR3-2
	; GR1-0 BLUE   3-0
	;
		moveq	#7,d1		; loop 8 times

TVCLow_Copy:
		moveq	#00,d0
		move.b	(a0)+,d0
		lsl.w	#6,d0		;*2 for word wide table
		or.b	(a0)+,d0	;P&Q bits are assumed to be 0

		move.w	d0,(a1)+
		dbf	d1,TVCLow_Copy

	; Indicate new LOW colors need to be copied to copper list
	; at next VBLANK - the VBLANK routine will clear this
	; bit when it copies the new colors to the copper list.

		bra.s	FixGfxCopInit

*****i* cdg.library/_TVCLUTHigh ***************************************
*
*   NAME
*	_TVCLUTHigh -- Set colors 8-15
*   
*   SYNOPSIS
*	VOID _TVCLUTHigh( data, pack, ecc )
*	                   A6    A4   D0
*
*	VOID _TVCLUTHigh( struct CDGData *, struct CDGPack *, BOOL );
*
*   FUNCTION
*	Set new colors (8-15).
*
*   INPUTS
*	data - Pointer to CDGData structure.
*
*	pack - Pointer to a CDGPack structure.  The mode, and instruction
*	       bytes have already been decoded.
*
*	ecc -  TRUE if the pack has good data, else FALSE.
*
*   RETURNS
*	N/A
*
*   NOTES
*	This function updates the CLUT table, and tells the VBLANK
*	interrupt to copy the colors to the copper list.
*
*	!!!NOTE!!!
*
*	This code has been optimized for speed, but needs to be
*	recoded for an RTG system - not a priority at the time
*	of this writing - speed is!!!  However the code
*	does at least sniff out the position of the color table
*	copper list instructions at init time.
*
***********************************************************************

_TVCLUTHigh:

	PRINTF	DBG_ENTRY,<'CDG--_TVCLUTHigh($%lx,$%lx)'>,A6,A4

	DEBUG_TRAP	31

		lea	cdp_Data(a4),a0
		lea	cdd_CLUTTable+16(a6),a1

	; Tell VBLANK not to bother with updating colors right now.
	;
	; A BIT CLEAR acts like a semaphore in this case, so no
	; DISABLE is needed.
	;
	; Since the colors are moved into the copper list at
	; VBLANK interrupt time, the move is ATOMIC because
	; the following code runs as a TASK, and a TASK cannot
	; interrupt an IRQ!
	;
	; We BIT CLEAR below in case we had already set the
	; BIT, but the previous color table has not yet been
	; copied to the copper list.  The CD+G graphics designer
	; is responsible for recognizing that the colors can only
	; be changed once per VBLANK; changing them more often
	; then VBLANK rate doesn't work anyway (per specification).
	;
		bclr	#VBFB_NEWCOLORS,cdd_VBlankFlags(a6)

	; move colors into CLUT table
	;
	; Colors are stored as 8 entries of RGB in 2 6 bit bytes like so
	;
	; R S   T U   V W
	; RED   3-0   GR3-2
	; GR1-0 BLUE   3-0
	;
		moveq	#7,d1		; loop 8 times

TVCHigh_Copy:
		moveq	#00,d0
		move.b	(a0)+,d0
		lsl.w	#6,d0		;*2 for word wide table
		or.b	(a0)+,d0	;P&Q bits are assumed to be 0

		move.w	d0,(a1)+
		dbf	d1,TVCHigh_Copy

	; Indicate new LOW colors need to be copied to copper list
	; at next VBLANK - the VBLANK routine will clear this
	; bit when it copies the new colors to the copper list.

		bra.s	FixGfxCopInit


*****i* cdg.library/_TVPresetMemory ***********************************
*
*   NAME
*	_TVPresetMemory -- Preset FONT memory with a PEN number
*   
*   SYNOPSIS
*	VOID _TVPresetMemory( data, pack, ecc )
*	                       A6    A4   D0
*
*	VOID _TVPresetMemory( struct CDGData *, struct CDGPack *, BOOL );
*
*   FUNCTION
*	Preset FONT memory with a PEN number (0-15).  The PH & PV
*	pointers (for scrolling) are also reset to 0.
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
*	This function sets the off-screen memory, and tells the
*	VBLANK interrupt to switch bitplanes at next VBLANK,
*	hence no display glitch.
*
*	!!!NOTE!!!
*
*	This code does not need to be exceedingly fast since
*	the Preset MEMORY instruction must be sent 16 times
*	in a row.  This means we have 16/75ths of a second
*	to work with; more than enough.  The PACK actually
*	has a REPEAT counter in it, so we do not do the memory
*	set more than once.
*
***********************************************************************

_TVPresetMemory:

	PRINTF	DBG_ENTRY,<'CDG--_TVPresetMemory($%lx,$%lx)'>,A6,A4

	DEBUG_TRAP	1

		lea	cdp_Data(a4),a0

		move.b	1(a0),d0

	; mask all but TUVW (repeat counter); start plane clear if
	; repeat counter == 0

		and.b	#(SYMF_T!SYMF_U!SYMF_V!SYMF_W),d0
		bne	TVPM_Exit

		movem.l	d2/a2/a6,-(sp)

	; extract color

		move.b	(a0),d2
		and.b	#(SYMF_T!SYMF_U!SYMF_V!SYMF_W),d2


	; Tell VBLANK not to bother swapping bit-plane pointers
	; right now (in case this bit is already set)

		bclr	#VBFB_NEWPLANES,cdd_VBlankFlags(a6)

	; And swap Active/Inactive pointers

		movem.l	cdd_InactivePlanes(a6),d0/a0
		exg	d0,a0
		movem.l	d0/a0,cdd_InactivePlanes(a6)

	; set/clear the memory we cannot see

                move.l	cdd_ActivePlanes(a6),a2
		move.l	cdd_GfxLib(a6),a6

	; obtain bitplane memory pointer, size, and default clear
	; method (0's), and do NOT wait for blit to finish

		move.l	(a2)+,a1

		move.l	#((DISPLAY_HEIGHT<<16)!(DISPLAY_WIDTH/8)),d0
		moveq	#(1<<1),d1

		lsr.b	#1,d2
		bcc.s	TVPM_Clear0

		move.l	#(($FFFF<<16)!(1<<1)!(1<<2)),d1

TVPM_Clear0:

	PRINTF	DBG_OSCALL,<'CDG--BltClear($%lx,$%lx,$%lx)'>,A1,D0,D1

		JSRLIB	BltClear


		move.l	(a2)+,a1

		move.l	#((DISPLAY_HEIGHT<<16)!(DISPLAY_WIDTH/8)),d0
		moveq	#(1<<1),d1

		lsr.b	#1,d2
		bcc.s	TVPM_Clear1

		move.l	#(($FFFF<<16)!(1<<1)!(1<<2)),d1

TVPM_Clear1:

	PRINTF	DBG_OSCALL,<'CDG--BltClear($%lx,$%lx,$%lx)'>,A1,D0,D1

		JSRLIB	BltClear


		move.l	(a2)+,a1

		move.l	#((DISPLAY_HEIGHT<<16)!(DISPLAY_WIDTH/8)),d0
		moveq	#(1<<1),d1

		lsr.b	#1,d2
		bcc.s	TVPM_Clear2

		move.l	#(($FFFF<<16)!(1<<1)!(1<<2)),d1

TVPM_Clear2:

	PRINTF	DBG_OSCALL,<'CDG--BltClear($%lx,$%lx,$%lx)'>,A1,D0,D1

		JSRLIB	BltClear


	; This final blit MUST wait, so we don't swap bit-plane pointers
	; until after all memory is set!

		move.l	(a2)+,a1

		move.l	#((DISPLAY_HEIGHT<<16)!(DISPLAY_WIDTH/8)),d0
		moveq	#((1<<0)!(1<<1)),d1

		lsr.b	#1,d2
		bcc.s	TVPM_Clear3

		move.l	#(($FFFF<<16)!(1<<0)!(1<<1)!(1<<2)),d1

TVPM_Clear3:

	PRINTF	DBG_OSCALL,<'CDG--BltClear($%lx,$%lx,$%lx)'>,A1,D0,D1

		JSRLIB	BltClear

		movem.l	(sp)+,d2/a2/a6

	; reset PH & PV per spec

RESET_PV	SET	(((DISPLAY_WIDTH/8)*FONT_HEIGHT))

		move.w	cdd_CopperList1+cdc_BPLCON1_DEF(a6),d0
		move.w	d0,d1
		lsl.w	#PFB_FINE_SCROLL_SHIFT,d1
		or.w	d1,d0			;odd/even planes

		move.w	#(RESET_PV),cdd_PHPV+2(a6)
		move.w	d0,cdd_PHPV(a6)

	; Tell VBLANK to swap bitplane pointers around!

		bset	#VBFB_NEWPLANES,cdd_VBlankFlags(a6)

TVPM_Exit:
		

		rts

*****i* cdg.library/SniffCopperList ***********************************
*
*   NAME
*	SniffCopperList -- Sniff out important positions in copper list
*   
*   SYNOPSIS
*	VOID SniffCopperList( data )
*	                        A4
*
*	VOID SniffCopperList( struct CDGData * );
*
*   FUNCTION
*	Sniffs out things I want to poke in the copper list.
*
*   INPUTS
*	data - Pointer to CDGData structure.
*
*   RETURNS
*	N/A
*
*   NOTES
*	This is initialization code; once I know the address
*	of important copper list stuff, I do not fetch it again.
*
***********************************************************************

DUMPCOPPER	EQU	1

SniffCopperList:

	PRINTF	DBG_ENTRY,<'CDG--SniffCopperList($%lx)'>,A4

		bsr	Debug_CopperList

		move.l	d2,-(sp)
		moveq	#00,d2
		move.w	cdd_MinDisplayX(a4),d2

	; write window control values in copper list

		lea	cdd_View1(a4),a0
		move.l	v_LOFCprList(a0),a0
		move.l	crl_start(a0),a0

		; find copper instruction which moves color into
		; color register 0

		move.w	#color,d0
		bsr	ScanCopperLists
		move.l	d0,cdd_CopperList1+cdc_Color0(a4)

		move.w	#ddfstrt,d0		
		bsr	ScanCopperLists
		move.l	d0,a1
		move.w	d2,d0
		lsr.w	#4,d0			;divide by 16 pixels
		lsl.w	#3,d0			;*8 color clocks
		add.w	#$38,d0			;minimum start
		move.w	d0,2(a1)

		move.w	#ddfstop,d0		
		bsr	ScanCopperLists
		move.l	d0,a1
		move.w	d2,d0
		lsr.w	#4,d0			;divide by 16 pixels
		lsl.w	#3,d0			;*8 color clocks
		add.w	#$d0,d0			;minimum stop
		move.w	d0,2(a1)

		move.w	#bpl1mod,d0		
		bsr	ScanCopperLists
		move.l	d0,a1
		clr.w	2(a1)

		move.w	#bpl2mod,d0		
		bsr	ScanCopperLists
		move.l	d0,a1
		clr.w	2(a1)

		move.w	#bplcon1,d0		
		bsr	ScanCopperLists
		move.l	d0,a1
		move.l	d0,cdd_CopperList1+cdc_BPLCON1(a4)
		move.w	d2,d0
		and.w	#$0F,d0			;make scroll value
		move.w	d0,d1
		lsl.w	#4,d1
		or.w	d1,d0
		move.w	d0,2(a1)

		and.w	#PF_FINE_SCROLL_MASK,d0
		move.w	d0,cdd_CopperList1+cdc_BPLCON1_DEF(a4)

		move.w	#fmode,d0
		bsr	ScanCopperLists
		tst.l	d0
		beq.s	no_fmode
		move.l	d0,a1

		and.w	#$FC,2(a1)	;reset BPAGEM, and BPL32

no_fmode:
		move.w	#bplpt,d0		
		bsr.s	ScanCopperLists
		move.l	d0,a1
		move.l	d0,cdd_CopperList1+cdc_BPL0(a4)

		moveq	#04,d1
		lea	cdd_BitMap1(a4),a0
		lea	bm_Planes(a0),a0
setplanes:
		move.l	(a0)+,d0
		add.l	#$1E0,d0		;12 rows

		move.w	d0,6(a1)
		swap	d0
		move.w	d0,2(a1)
		addq.l	#8,a1			;assume plane pointers in order
		dbf	d1,setplanes

		move.l	(sp)+,d2

	; finally sniff out color 0 poke in graphics copinit so we
	; get a solid background image

		move.l	cdd_GfxLib(a4),a1
		move.l	gb_copinit(a1),a0
		move.w	#color,d0
		bsr.s	ScanCopperLists
		move.l	d0,cdd_CopperList1+cdc_COPINITColor0(a4)
		move.l	d0,cdd_CopperList1+cdc_COPINITColor0b(a4)

	; and scan for second poke to color 0 (next bank) under AA
	; machines

		move.l	d0,a0
copinit_0bfind:
		addq.l	#4,a0
		move.w	(a0),d1
		move.w	d1,d0
		and.w	#$0001,d0
		bne.s	copinit_0bnot		;terminate if WAIT

		cmp.w	#color,d1
		bne.s	copinit_0bfind

		move.l	a0,cdd_CopperList1+cdc_COPINITColor0b(a4)

copinit_0bnot:

	; and scan for second color 0 in main list (AA Enlighted mode)

		move.l	cdd_CopperList1+cdc_Color0(a4),a0
		move.l	a0,cdd_CopperList1+cdc_Color0b(a4)
color_0bfind:
		addq.l	#4,a0
		move.w	(a0),d1
		move.w	d1,d0

		and.w	#$0001,d0
		bne.s	color_0bnot

		cmp.w	#color,d1
		bne.s	color_0bfind

		move.l	a0,cdd_CopperList1+cdc_Color0b(a4)

color_0bnot:

	PRINTF	DBG_EXIT,<'CDG--Exit SniffCopperLists()'>

		bsr.s	Debug_CopperList
		rts

*****i* cdg.library/ScanCopperLists ***********************************
*
*   NAME
*	ScanCopperLists -- Find a specific instruction
*   
*   SYNOPSIS
*	APTR ScanCopperLists( copper, instruction )
*	D0                     A0,    D0
*
*	APTR ScanCopperLists( APTR, UWORD );
*
*   FUNCTION
*	Finds an instruction in a copper list... disgusting,
*	but better than assuming hardcoded offsets.
*
*   INPUTS
*	copper - Pointer to start of copper list
*
*	instruction - Instruction to find.
*   RETURNS
*	N/A
*
*   NOTES
*	A0 preserved!!!
*
***********************************************************************


ScanCopperLists:

	PRINTF	DBG_ENTRY,<'CDG--ScanCopperLists($%lx,$%lx)'>,A0,D0

		move.l	a0,-(sp)

ScanCopper_Search:

		cmp.w	(a0),d0
		beq.s	ScanCopper_Match

		move.w	(a0),d1
		cmp.w	#-1,d1
		beq.s	ScanCopper_Failed

		addq.w	#4,a0
		bra.s	ScanCopper_Search

ScanCopper_Match:
		move.l	a0,d0
		move.l	(sp)+,a0

	PRINTF	DBG_EXIT,<'CDG--$%lx=ScanCopperLists()'>,D0

		rts

ScanCopper_Failed:
		suba.l	a0,a0
		bra.s	ScanCopper_Match

Debug_CopperList:
	IFNE	DUMPCOPPER

		lea	cdd_View1(a4),a0
		move.l	v_LOFCprList(a0),a0
		move.l	crl_start(a0),a0

		move.w	#diwstrt,d0
		bsr	ScanCopperLists
		move.l	d0,a1

	PUSHWORD	2(a1)
	PRINTF	DBG_FLOW,<'	dc.w	$%lx	;diwstart'>
	POPLONG		1

		move.w	#diwstop,d0
		bsr	ScanCopperLists
		move.l	d0,a1

	PUSHWORD	2(a1)
	PRINTF	DBG_FLOW,<'	dc.w	$%lx	;diwstop'>
	POPLONG		1

		move.w	#ddfstrt,d0
		bsr	ScanCopperLists
		move.l	d0,a1

	PUSHWORD	2(a1)
	PRINTF	DBG_FLOW,<'	dc.w	$%lx	;ddfstrt'>
	POPLONG		1

		move.w	#ddfstop,d0
		bsr	ScanCopperLists
		move.l	d0,a1

	PUSHWORD	2(a1)
	PRINTF	DBG_FLOW,<'	dc.w	$%lx	;ddfstop'>
	POPLONG		1

		move.w	#bplcon1,d0
		bsr	ScanCopperLists
		move.l	d0,a1

	PUSHWORD	2(a1)
	PRINTF	DBG_FLOW,<'	dc.w	$%lx	;bplcon1'>
	POPLONG		1

		move.w	#bpl1mod,d0
		bsr	ScanCopperLists
		move.l	d0,a1

	PUSHWORD	2(a1)
	PRINTF	DBG_FLOW,<'	dc.w	$%lx	;bplmod'>
	POPLONG		1

		move.w	#diwhigh,d0
		bsr	ScanCopperLists
		move.l	d0,a1

	PUSHWORD	2(a1)
	PRINTF	DBG_FLOW,<'	dc.w	$%lx	;diwhigh'>
	POPLONG		1

		move.w	#bplpt,d0
		bsr	ScanCopperLists
		move.l	d0,a1

		move.w	2(a1),d0
		swap	d0
		move.w	6(a1),d0
		
		sub.l	cdd_BitPlane1_0(a4),d0
		
	PUSHWORD	D0
	PRINTF	DBG_FLOW,<'	dc.w	$%lx	;bplpt offset'>
	POPLONG		1

	PRINTF	DBG_FLOW,<'	;--'>

	ENDC

		rts

		END
