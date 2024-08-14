**
**	$Id: interrupts.asm,v 1.15 93/02/03 16:18:22 darren Exp $
**
**	CDTV CD+G -- interrupts.asm (interrupt server code)
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

** Includes

		INCLUDE	"cdgbase.i"
		INCLUDE	"cdgprefs.i"
		INCLUDE	"cdgs_hw.i"
		INCLUDE	"debug.i"

		SECTION	cdg
** Imports
		XREF	_custom		;amiga.lib

** Assumptions

	IFNE	cdc_Color0
	FAIL	"cdc_Color0 not 0; recode!"
	ENDC

	IFNE	cdd_ActivePlanes-cdd_InactivePlanes-4
	FAIL	"cdd_ActivePlanes does not follow cdd_InactivePlanes; recode!"
	ENDC

	IFNE	cdd_SpriteDelay-cdd_StartSprite-2
	FAIL	"cdd_SpriteDelay does not follow cdd_StartSprite; recode!"
	ENDC

	IFNE	cdgp_ChannelSprites
	FAIL	"cdgp_ChannelSprites not 0; recode!"
	ENDC

	IFNE	cdgp_PAUSESprite-cdgp_ChannelSprites-(64*4)
	FAIL	"cdgp_PAUSESprite ! cdgp_ChannelSprites+(64*4); recode!"
	ENDC

	IFNE	cdgp_NTrackSprite-cdgp_PAUSESprite-(4*4)
	FAIL	"cdgp_NTrackSprite ! cdgp_PAUSESprite-(4*4); recode!"
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

	IFNE	cdd_BitPlane1_Extra-cdd_BitPlane1_3-4
	FAIL	"cdd_Bitplane1_Extra does not follow cdd_Bitplane1_3; recode!"
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

	IFNE	cdd_BitPlane2_Extra-cdd_BitPlane2_3-4
	FAIL	"cdd_Bitplane2_Extra does not follow cdd_Bitplane2_3; recode!"
	ENDC

	IFNE	cdd_CDIO
	FAIL	"cdd_CDIO not first in CDGData struct; recode!"
	ENDC


** Exports

		XDEF	CDGPackInt
		XDEF	CDGVertInt

** Equates

		CNOP	0,4

*****i* cdg/CDGPackInt ************************************************
*
*   NAME
*	CDGPackInt -- Interrupt for copying new packets
*   
*   SYNOPSIS
*	VOID CDGPackInt( data )
*			  A1
*
*	VOID CDGPackInt( struct CDGData * );
*
*   FUNCTION
*	Interrupt code for copying packet data to circular buffer.
*	The interrupt is routed through the CD.DEVICE via the FRAMECALL
*	command.  We are set-up to get every FRAME.
*
*   RETURNS
*	N/A
*
*   NOTES
*	There are 4 PACKS per PACKET, and these are buffered
*	by the 4510 in INTERLEAVED data order.  We get roughly
*	75 PACKETS per second - efficiency throughout is therefore
*	critical.
*
*	This routines only responsibility is to make a copy of the
*	PACKET, and store it in a circular buffer.  If the buffer
*	is FULL - ARGGHH!!!  Data is then lost, however this should
*	rarely (if ever happen) unless the main CD+G task is not given
*	CPU time for some reason.  In the world of CDTV-CR, its a
*	closed system when CD+G is running, and the system software
*	needs to be structured to guarantee the CD+G task gets most CPU
*	time (except for interrupts).
*
*	!!!NOTE!!!
*
*	Optimization of this code is IMPORTANT!!!  For the sake of
*	efficiency, if data is lost, it is overwritten with the most
*	recent data - tolerable, particularly when one considers it
*	doesn't matter much that we lost the last moment of data, or
*	the next moment of data.  Lost is lost.
*
*	!!!NEW!!!
*
*	The CDGS system is faster than the CDTV-CR, but gack, we
*	have to use a much slower interrupt copy routine here
*	because we lack the 4510 to do the P&Q bit stripping.
*
***********************************************************************

CDGPackInt:


	; The following flag allows us to ignore subcodes if
	; the cdg.library thinks the disk is stopped, or otherwise
	; not in CDGPlay() mode.  This allows CDGDraw() to exit
	; quickly, and if Signal() is being used, to never
	; Signal().  Hence the application could do something
	; else while the cdg.library thinks the disk is not
	; playing.
	;
	;
		btst	#EF2B_IGNORE,cdd_ExtraFlags2(a1)
		BNE_S	CDGPackInt_Ignore

	; Periodically reset +G/+MIDI pack validity counters

		addq.b	#1,cdd_FRollCount(a1)
		bne.s	CDPackInt_Count

		move.b	#GPACKCOUNT,cdd_GPackCount(a1)
		move.b	#MPACKCOUNT,cdd_MPackCount(a1)

CDPackInt_Count:

	; have we exceeded our max buffer space?

		move.l	cdd_PutPacket(a1),a0
		cmp.l	cdd_MaxPacket(a1),a0
		bne.s	CDGPackInt_Copy

	; if so, wrap PutPacket

		lea	cdd_PacketData(a1),a0

CDGPackInt_Copy:

		movem.l	d2/a2,-(sp)
		move.l	a1,a2

		move.l	cdd_SubCodeBank(a2),a1

		cmp.b	#$80,(CDHARDWARE+CDSUBINX)
		bcs.s	CDGPackInt_CopyPacks

		lea	$80(a1),a1	;plus $80 hex

CDGPackInt_CopyPacks:


	; use inline move for speed here

		move.l	#$3F3F3F3F,d1	;strip P & Q

		moveq	#(24-1),d2	;Move 4 * 24 == 96 bytes

	; the 020 has a pretty decent cache, so what the heck, a loop
	; is pretty much free

CDGPackInt_CopyPack:

		move.l	(a1)+,d0	;Move 4 bytes at a time
		and.l	d1,d0
		move.l	d0,(a0)+

		dbf	d2,CDGPackInt_CopyPack

		move.l	a0,cdd_PutPacket(a2)

	;;
	;; See if we need to signal task

		move.l	cdd_SigMask(a2),d0
		beq.s	CDGPackInt_Ending

		move.l	a6,-(sp)
		move.l	cdd_SigTask(a2),a1
		move.l	cdd_ExecLib(a2),a6
		JSRLIB	Signal
		move.l	(sp)+,a6

CDGPackInt_Ending:

		movem.l	(sp)+,d2/a2

CDGPackInt_Ignore:
		moveq	#00,d0		; Return 0 for future compatability

		rts

*****i* cdg/CDGVertInt *************************************************
*
*   NAME
*	CDGVertInt -- Vertical blank interrupt
*   
*   SYNOPSIS
*	VOID CDGVertInt( data )
*			  A1
*
*	VOID CDGVertInt( struct CDGData * );
*
*   FUNCTION
*	Vertical blank interrupt.  Used for timing purposes when
*	sprites are displayed (e.g., channel number selection), and
*	for reloading color registers in the active copper list.
*
*	This function also swaps the bitplane pointers around for
*	double buffered stuff.
*
*	Because this stuff is handled during VBLANK, no display
*	glitches!
*
*   RETURNS
*	N/A
*
*   NOTES
*	Timing is adjusted for VBlank RATE.
*
*	Sprites for user feedback-
*
*	This interrupt fetches a LONG WORD starting at cdd_StartSprite
*	which makes it possible for the cdg.library to stash a sprite
*	offset, and time delay value using an ATOMIC MOVE.L instruction -
*	hence, no DISABLE is needed to move both values at task time.
*
*	Sprite data is copied quickly, or cleared quickly, hence
*	it is possible to use data stored in ROM to save RAM
*	space.  While more expensive than poking copper list
*	pointers, the later does not allow sprite data to be in ROM.
*	Since there is so much sprite data, and a real need for
*	CHIP ram when PlayerPrefs, and CD+G is up ...
*
*	Also note that the displaying of sprites is a rare event
*	in the overall scheme of things; the extra CPU cycles really
*	don't hurt us here.  Because the copy is done at VBLANK time,
*	no visual sprite glitchies!
*
*	Color changes -
*
*	The 16 colors can be moved into the copper list at
*	VBLANK time.  In addition, the border color is also updated
*	within this routine, and must be one of the PEN colors.
*
*	NOTE!!!
*
*	Cheating like a slime here (performance needed on a 68000
*	machine), but more importantly is the need to batch up color
*	changes for the next frame, and update all colors atomically -
*	can't be done with LoadRGB4 or SetRGB4 because they are not
*	safe from interrupts.  Worse, we want a solid top border, and
*	screen color change (no slipping in between copinit color change
*	0, and the screen's color change 0).  
*
*	Double buffering -
*
*	The bitplane pointers can be swapped in the copper list
*	during VBLANK, hence no visual glitches.
*
***********************************************************************

		CNOP	0,4

CDGVertInt:

		move.l	a1,a5

	; See if we need to turn off sprite DMA for the Player program

		bclr	#VBFB_RESETDMA,cdd_VBlankFlags(a5)
		beq.s	CDGVertInt_NoResetDMA

		move.w	cdd_CacheDMACONR(a5),d0

	; Since I turn on SPRITE DMA, I'm not going to turn it ON again
	; if it was already on when CDGFront() was called

		and.w	#DMAF_SPRITE,d0
		bne.s	CDGVertInt_NoResetDMA

		move.w	#DMAF_SPRITE,_custom+dmacon

CDGVertInt_NoResetDMA:

	; See if there are any new colors to be loaded into copper list

		bclr	#VBFB_NEWCOLORS,cdd_VBlankFlags(a5)
		beq	CDVertInt_NoNewColors

	PRINTF	(DBG_INTERRUPT+DBG_FLOW),<'CDG--Copy New Colors'>

		move.l	cdd_BorderPen(a5),a0
		move.w	(a0),d0			;obtain color

		lea	cdd_CLUTTable(a5),a0

	; set graphics copinit color 0 so our top border looks right

		btst	#VBFB_BACKSCREEN,cdd_VBlankFlags(a5)
		bne.s	CDGV_SkipCopInit

		move.l	cdd_CopperList1+cdc_COPINITColor0(a5),a1
		move.w	d0,2(a1)

		move.l	cdd_CopperList1+cdc_COPINITColor0b(a5),a1
		move.w	d0,2(a1)

CDGV_SkipCopInit:

		move.l	a2,d1

		move.l	cdd_CopperList1+cdc_Color0(a5),a1
		move.l	cdd_CopperList1+cdc_Color0b(a5),a2

	; set new border color

		move.w	d0,2(a1)		;change border color
		move.w	d0,2(a2)		;change border color

	; update colors 0..7 mapped to hardware colors 8-15
	; 1 LONG WORD per instruction, so 8*4 = 32, plus 2 for instruction

		move.w	(a0),34(a1)		;maybe same or second bank in AA
		move.w	(a0)+,34(a2)		;primary first then secondary
						;just in case we are late
		move.w	(a0),38(a1)
		move.w	(a0)+,38(a2)

		move.w	(a0),42(a1)
		move.w	(a0)+,42(a2)

		move.w	(a0),46(a1)
		move.w	(a0)+,46(a2)

		move.w	(a0),50(a1)
		move.w	(a0)+,50(a2)

		move.w	(a0),54(a1)
		move.w	(a0)+,54(a2)

		move.w	(a0),58(a1)
		move.w	(a0)+,58(a2)

		move.w	(a0),62(a1)
		move.w	(a0)+,62(a2)

	; update colors 8..15 mapped to hardware 24-31
	; 1 LONG WORD per instruction, so 24*4 = 96, plus 2 for instruction

		move.w	(a0),098(a1)
		move.w	(a0)+,098(a2)

		move.w	(a0),102(a1)
		move.w	(a0)+,102(a2)

		move.w	(a0),106(a1)
		move.w	(a0)+,106(a2)

		move.w	(a0),110(a1)
		move.w	(a0)+,110(a2)

		move.w	(a0),114(a1)
		move.w	(a0)+,114(a2)

		move.w	(a0),118(a1)
		move.w	(a0)+,118(a2)

		move.w	(a0),122(a1)
		move.w	(a0)+,122(a2)

		move.w	(a0),126(a1)
		move.w	(a0)+,126(a2)

		move.l	d1,a2			;restore

CDVertInt_NoNewColors

	; see if we need to swap bitplane pointers

		bclr	#VBFB_NEWPLANES,cdd_VBlankFlags(a5)
		beq.s	CDVertInt_NoNewPlanes

	PRINTF	(DBG_INTERRUPT+DBG_FLOW),<'CDG--Swap bitplanes'>

		moveq	#00,d1

		IFEQ	SHOWBORDER


	; set horizontal scroll; PH is pre-calced

		move.l	cdd_CopperList1+cdc_BPLCON1(a5),a0
		move.w	cdd_PHPV(a5),2(a0)

		move.w	cdd_PHPV+2(a5),d1	;PV (pre-calced)

		ENDC

		move.l	cdd_ActivePlanes(a5),a0
		move.l	cdd_CopperList1+cdc_BPL0(a5),a1

	; Assumes bitplane pointer copper lists are stored in contiguous
	; memory!!

		; plane 0

		move.l	(a0)+,d0
		add.l	d1,d0
		move.w	d0,6(a1)		;store low word
		swap	d0
		move.w	d0,2(a1)		;store high word

		; plane 1

		move.l	(a0)+,d0
		add.l	d1,d0			;skip first word
		move.w	d0,14(a1)		;store low word
		swap	d0
		move.w	d0,10(a1)		;store high word

		; plane 2

		move.l	(a0)+,d0
		add.l	d1,d0			;skip first word
		move.w	d0,22(a1)		;store low word
		swap	d0
		move.w	d0,18(a1)		;store high word

		; plane 3 (mapped to plane 4)

		move.l	(a0)+,d0
		add.l	d1,d0			;skip first word
		move.w	d0,38(a1)		;store low word
		swap	d0
		move.w	d0,34(a1)		;store high word

		; plane 4 (mapped to plane 3)

		move.l	(a0)+,d0
		add.l	d1,d0			;skip first word
		move.w	d0,30(a1)		;store low word
		swap	d0
		move.w	d0,26(a1)		;store high word


CDVertInt_NoNewPlanes:

	; See if we need to put up a new sprite

		move.l	cdd_StartSprite(a5),d0
		bne.s	CDGVertInt_newsprite	;typical case is ==
		lea	_custom,a0		;compatability!
	;;;	moveq	#00,d0			;CC == EQ

		rts

	; check for new sprite before decrementing delay
	; counter.  allows us to display a new sprite before
	; the previous one has been turned off

CDGVertInt_newsprite:

		move.l	a5,a1

	; swap cdd_StartSprite into lower word, and check if <>

		swap	d0
		tst.w	d0

		BEQ_S	CDGVertInt_checkdelay

	PRINTF	(DBG_INTERRUPT+DBG_FLOW),<'CDG--Copy Sprite'>

		move.w	d2,-(sp)

	; Don't bother copying sprite data if the screen is in the back

		btst	#VBFB_BACKSCREEN,cdd_VBlankFlags(a1)
		bne.s	CDGV_backscreen

	; Copy sprite data quickly during VBLANK - no display glitchies,
	; and no copper list poking.
	;
	; This takes longer than poking graphics private copper list,
	; but is faster than ChangeSprite() (which really thinks it
	; runs on TASK time), and it makes it possible to share sprite
	; images (e.g., the caller can provide pointers to the same NULL
	; sprite if they do not want to draw all of the possible
	; sprites).

		move.l	cdd_CDGPrefs(a1),a6

		move.w	cdgp_SpriteHeight(a6),d2
		subq.w	#1,d2

		add.w	d0,a6		;add preadjusted offset

		move.l	(a6),a5		;sprite data ptr

		addq.w	#4,a5		;skip posctrl

		move.l	cdd_4SpriteData(a1),a0

		addq.w	#4,a0		;skip posctrl

		
                move.w	d2,d1

CDGV_copyleft1:

		move.l	(a5)+,(a0)+
		dbf	d1,CDGV_copyleft1

		move.l	4(a6),a5	;get next pointer
		addq.w	#4,a5		;skip posctrl

		addq.w	#8,a0		;skip terminator, and posctrl

                move.w	d2,d1

CDGV_copyleft2:
		move.l	(a5)+,(a0)+
		dbf	d1,CDGV_copyleft2

		move.l	8(a6),a5	;get next pointer
		addq.w	#4,a5		;skip posctrl

		addq.w	#8,a0		;skip terminator, and posctrl

                move.w	d2,d1

CDGV_copyright1:
		move.l	(a5)+,(a0)+
		dbf	d1,CDGV_copyright1

		move.l	12(a6),a5	;get next pointer
		addq.w	#4,a5		;skip posctrl

		addq.w	#8,a0		;skip terminator, and posctrl

CDGV_copyright2:
		move.l	(a5)+,(a0)+
		dbf	d2,CDGV_copyright2

CDGV_backscreen:

	; indicate sprite has been copied - don't do it again

		clr.w	cdd_StartSprite(a1)

		move.w	(sp)+,d2

	PRINTF	(DBG_INTERRUPT+DBG_EXIT),<'CDG--VBL XIT'>

		lea	_custom,a0		;compatability!
		moveq	#00,d0
		rts

	; check if we need to decrement our delay counter

CDGVertInt_checkdelay

		swap	d0
		tst.w	d0

	; this counter portion will be 0 if nothing to decrement,
	; NEGATIVE means no decrement (e.g., PAUSE sprite is displayed),
	; otherwise the counter is decremented until 0, at which time
	; the sprite is blanked.

		beq.s	CDGVertInt_Exit
		bmi.s	CDGVertInt_Exit

		subq.w	#1,d0
		move.w	d0,cdd_SpriteDelay(a1)
		bne.s	CDGVertInt_Exit		;0 yet?

	PRINTF	(DBG_INTERRUPT+DBG_FLOW),<'CDG--Clear Sprite'>

	; clear sprite imagery for all 4 sprites

		move.w	d2,-(sp)

		move.l	cdd_CDGPrefs(a1),a0
		move.w	cdgp_SpriteHeight(a0),d2
		subq.w	#1,d2

		move.l	cdd_4SpriteData(a1),a0

		moveq	#00,d0

		addq.w	#4,a0		;skip posctrl

		move.w	d2,d1

CDGV_clearleft1:
		move.l	d0,(a0)+
		dbf	d1,CDGV_clearleft1

		addq.w	#8,a0		;skip posctrl, and terminator

		move.w	d2,d1

CDGV_clearleft2:
		move.l	d0,(a0)+
		dbf	d1,CDGV_clearleft2

		addq.w	#8,a0		;skip posctrl, and terminator

		move.w	d2,d1

CDGV_clearright1:
		move.l	d0,(a0)+
		dbf	d1,CDGV_clearright1

		addq.w	#8,a0		;skip posctrl, and terminator

CDGV_clearright2:
		move.l	d0,(a0)+
		dbf	d2,CDGV_clearright2

		move.w	(sp)+,d2
		
CDGVertInt_Exit:

		lea	_custom,a0		;compatability!
		moveq	#00,d0			;CC == EQ
		rts


		END