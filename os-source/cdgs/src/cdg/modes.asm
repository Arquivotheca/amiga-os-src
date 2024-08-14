**
**	$Id: modes.asm,v 1.16 93/02/03 12:01:07 darren Exp $
**
**	CDTV CD+G -- modes.asm (control display modes/sprites)
**
**	Modified for CDGS - does not turn on/off VFD lights
**
**	(C) Copyright 1992,1993 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

** Includes

		INCLUDE	"cdgbase.i"
		INCLUDE	"cdgprefs.i"
		INCLUDE	"cdgs_hw.i"
		INCLUDE	"debug.i"
** Exports
		XDEF	CDGChannel
		XDEF	CDGPause
		XDEF	CDGStop
		XDEF	CDGPlay
		XDEF	CDGNextTrack
		XDEF	CDGPrevTrack
		XDEF	CDGFastForward
		XDEF	CDGRewind
		XDEF	CDGClearScreen
		XDEF	CDGDiskRemoved
** Imports

		XREF	CDGAllNotesOff

** Sprite type equates

SPRITE_CH_0	EQU	0
SPRITE_CH_1	EQU	1	
SPRITE_CH_2	EQU	2	
SPRITE_CH_3	EQU	3	
SPRITE_CH_4	EQU	4	
SPRITE_CH_5	EQU	5	
SPRITE_CH_6	EQU	6	
SPRITE_CH_7	EQU	7	
SPRITE_CH_8	EQU	8	
SPRITE_CH_9	EQU	9	
SPRITE_CH_10	EQU	10
SPRITE_CH_11	EQU	11
SPRITE_CH_12	EQU	12
SPRITE_CH_13	EQU	13
SPRITE_CH_14	EQU	14
SPRITE_CH_15	EQU	15
SPRITE_PAUSE	EQU     16
SPRITE_NTRK	EQU	17
SPRITE_PTRK	EQU	18
SPRITE_FF	EQU	19
SPRITE_REW	EQU	20
SPRITE_STOP	EQU	21
SPRITE_PLAY	EQU	22


** Assumptions

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

	IFNE	cdgp_PTrackSprite-cdgp_NTrackSprite-(4*4)
	FAIL	"cdgp_PTrackSprite ! cdgp_NTrackSprite-(4*4); recode!"
	ENDC

	IFNE	cdgp_FFSprite-cdgp_PTrackSprite-(4*4)
	FAIL	"cdgp_FFSprite ! cdgp_PTrackSprite-(4*4); recode!"
	ENDC

	IFNE	cdgp_RWSprite-cdgp_FFSprite-(4*4)
	FAIL	"cdgp_RWSprite ! cdgp_FFSprite-(4*4); recode!"
	ENDC

	IFNE	cdgp_STOPSprite-cdgp_RWSprite-(4*4)
	FAIL	"cdgp_STOPSprite ! cdgp_RWSprite-(4*4); recode!"
	ENDC

	IFNE	cdgp_PLAYSprite-cdgp_STOPSprite-(4*4)
	FAIL	"cdgp_PLAYSprite ! cdgp_STOPSprite-(4*4); recode!"
	ENDC

******* cdg.library/CDGChannel ****************************************
*
*   NAME
*	CDGChannel -- Set active channel number for FONT data
*   
*   SYNOPSIS
*	VOID CDGChannel( channel )
*			    D0
*
*	VOID CDGChannel( ULONG );
*
*   FUNCTION
*	Tells the cdg.library to use a new channel.  Channel 0 is always
*	displayed.  In addition, one of 15 other channels can also be
*	displayed.  The selected channel number is automatically displayed
*	on the CD+G screen for a reasonable amount of time (approximately
*	2-3 seconds).
*
*	Sprites, passed in a CDGPrefs structure at CDGBegin()
*	time, are used to display the channel numbers.
*
*   INPUTS
*	channel - A number from 1-15.
*
*   RETURNS
*	N/A
*
*   NOTES
*	The caller is responsible for managing user input, including
*	polling for some button press indicating that the user would
*	like to increment, or set a new active channel.
*
*   SEE ALSO
*	CDGBegin()
*
***********************************************************************

CDGChannel:
	PRINTF	DBG_ENTRY,<'CDG--CDGChannel($%lx)'>,D0

		move.l	cdg_BaseData(a6),a0

		and.b	#$0F,d0			;only 1-15 are valid
		beq.s	CDGChannel_Invalid

		move.b	d0,cdd_ActiveChannel(a0)

	; load sprite data at next VBLANK

		lsl.w	#4,d0			;4 long word ptrs per sprite
		swap	d0			;sprite code in high word
		move.w	cdd_VBlankRate(a0),d0	;# of VBlanks in low word
		add.w	d0,d0			;*2 for 2 second delay
		move.l	d0,cdd_StartSprite(a0)	;ATOMIC move!

CDGChannel_Invalid:

		rts

******* cdg.library/CDGPause ******************************************
*
*   NAME
*	CDGPause -- Tell CD+G that the disk is in PAUSE mode
*   
*   SYNOPSIS
*	VOID CDGPause( VOID )
*
*	VOID CDGPause( VOID );
*
*   FUNCTION
*	Tell the cdg.library that the disk is in PAUSE mode.
*
*	Sprites, passed in a CDGPrefs structure at CDGBegin()
*	time, are used to display the mode.
*
*   INPUTS
*	N/A
*
*   RETURNS
*	N/A
*
*   NOTES
*	The PAUSE mode sprite(s) is displayed until CDGPlay() is called,
*	CDGBack() is called, or some other cdg.library mode function
*	is called. 
*
*	If +MIDI data has been sent, this function will also cause
*	a sequence of bytes to be transmitted out the serial/midi
*	port telling all midi channels to be quiet.
*
*   SEE ALSO
*	CDGBegin(), CDGPlay(), CDGBack()
*
***********************************************************************

CDGPause:
	PRINTF	DBG_ENTRY,<'CDG--CDGPause()'>

		bsr	CDGAllNotesOff

	; Display PAUSE mode sprite

		move.l	cdg_BaseData(a6),a0

		move.l	#(((SPRITE_PAUSE)<<20)!$FFFF),cdd_StartSprite(a0)

		bra	CDGRestartPack

	;;;	rts

******* cdg.library/CDGStop *******************************************
*
*   NAME
*	CDGStop -- Tell CD+G that the disk is in STOP mode
*   
*   SYNOPSIS
*	VOID CDGStop( VOID )
*
*	VOID CDGStop( VOID );
*
*   FUNCTION
*	Tell the cdg.library that the disk is in STOP mode.
*
*	Sprites, passed in a CDGPrefs structure at CDGBegin()
*	time, are used to display the mode.
*
*   INPUTS
*	N/A
*
*   RETURNS
*	N/A
*
*   NOTES
*	The STOP mode sprite(s) is displayed until CDGPlay() is called,
*	CDGBack() is called, or some other cdg.library mode function
*	is called. 
*
*	If +MIDI data has been sent, this function will also cause
*	a sequence of bytes to be transmitted out the serial/midi
*	port telling all midi channels to be quiet.
*
*   SEE ALSO
*	CDGBegin(), CDGPlay(), CDGBack()
*
***********************************************************************

CDGStop:
	PRINTF	DBG_ENTRY,<'CDG--CDGStop()'>

		bsr	CDGAllNotesOff

	; Display STOP mode sprite

		move.l	cdg_BaseData(a6),a0

		move.l	#(((SPRITE_STOP)<<20)!$FFFF),cdd_StartSprite(a0)

		bra	CDGRestartPack

	;;;	rts

******* cdg.library/CDGPlay *******************************************
*
*   NAME
*	CDGPlay -- Tell CD+G that the disk is in PLAY mode
*   
*   SYNOPSIS
*	VOID CDGPlay( Show )
*	               D0
*
*	VOID CDGPlay( BOOL );
*
*   FUNCTION
*	Tell the cdg.library that the disk is in PLAY mode.  You must call
*	this function each time you start the CD playing to enable
*	subcodes (R through W).  Note that enabling means that
*	the cdg.library does not ignore subcode interrupts, however
*	subcode interrupts must still be enabled by the cd.device
*	driver.
*	
*	Sprites, passed in a CDGPrefs structure at CDGBegin()
*	time, are used to display the mode.
*
*   INPUTS
*	Show -- If TRUE, the PLAY sprite is displayed on the
*	CD+G screen for a few moments.
*
*   RETURNS
*	N/A
*
*   NOTES
*	The PLAY mode sprite(s) is automatically turned off after
*	a few moments (e.g., 1/2 second), or when CDGBack() is called.
*
*	During FF or REW, it probably makes more sense to call this
*	function with a FALSE value for 'Show' since PLAY is implied
*	after a FF or REW.
*
*   SEE ALSO
*	CDGBegin(), CDGBack()
*
***********************************************************************

CDGPlay:

	PRINTF	DBG_ENTRY,<'CDG--CDGPlay(%ld)'>,D0

		move.w	#(SPRITE_PLAY*16),d1

		move.l	cdg_BaseData(a6),a0

CDGPlay_GO:

	; enable collection of PACKETS

		bclr	#EF2B_IGNORE,cdd_ExtraFlags2(a0)

	; display PLAY mode sprite?

		tst.w	d0
		beq.s	CDGPlay_NoSprite

		swap	d1

		move.w	cdd_VBlankRate(a0),d1	;# of VBlanks in low word
		lsr.w	#1,d1			;1/2 second
		move.l	d1,cdd_StartSprite(a0)	;ATOMIC move!
		
		rts

	; clear active sprite (if any - such as a FF sprite)

CDGPlay_NoSprite:
		moveq	#01,d1
		move.l	d1,cdd_StartSprite(a0)	;ATOMIC move!

		rts

******* cdg.library/CDGNextTrack **************************************
*
*   NAME
*	CDGNextTrack -- Tell CD+G that the disk is in NEXT TRACK mode
*   
*   SYNOPSIS
*	VOID CDGNextTrack( VOID )
*
*	VOID CDGNextTrack( VOID );
*
*   FUNCTION
*	Tell the cdg.library that the disk is in the NEXT TRACK mode.
*	The screen is cleared, and all colors are reset to black.
*
*	Sprites, passed in a CDGPrefs structure at CDGBegin()
*	time, are used to display the mode.
*
*   INPUTS
*	N/A
*
*   RETURNS
*	N/A
*
*   NOTES
*	The NEXT TRACK mode sprite(s) is automatically turned off after
*	a few moments (e.g., 1/2 second), or when CDGBack() is called.
*
*	CDGNextTrack() is much like CDGPlay(), except a different
*	sprite is displayed, and the screen is automatically cleared.
*
*	If +MIDI data has been sent, this function will also cause
*	a sequence of bytes to be transmitted out the serial/midi
*	port telling all midi channels to be quiet.
*
*   SEE ALSO
*	CDGBegin(), CDGPlay(), CDGBack()
*
***********************************************************************

CDGNextTrack:
	PRINTF	DBG_ENTRY,<'CDG--CDGNextTrack()'>

		bsr	CDGAllNotesOff

		bsr.s	CDGClearScreen

		move.l	cdg_BaseData(a6),a0

		bsr	CDGRestartPack

		move.w	#(SPRITE_NTRK*16),d1
		moveq	#01,d0
		BRA_S	CDGPlay_GO

******* cdg.library/CDGPrevTrack **************************************
*
*   NAME
*	CDGPrevTrack -- Tell CD+G that the disk is in PREV TRACK mode
*   
*   SYNOPSIS
*	VOID CDGPrevTrack( VOID )
*
*	VOID CDGPrevTrack( VOID );
*
*   FUNCTION
*	Tell the cdg.library that the disk is in the PREV TRACK mode.
*	The screen is cleared, and all colors are reset to black.
*
*	Sprites, passed in a CDGPrefs structure at CDGBegin()
*	time, are used to display the mode.
*
*   INPUTS
*	N/A
*
*   RETURNS
*	N/A
*
*   NOTES
*	The PREV TRACK mode sprite(s) is automatically turned off after
*	a few moments (e.g., 1/2 second), or when CDGBack() is called.
*
*	CDGPrevTrack() is much like CDGPlay(), except a different
*	sprite is displayed, and the screen is automatically cleared.
*
*	If +MIDI data has been sent, this function will also cause
*	a sequence of bytes to be transmitted out the serial/midi
*	port telling all midi channels to be quiet.
*
*   SEE ALSO
*	CDGBegin(), CDGPlay(), CDGBack()
*
***********************************************************************

CDGPrevTrack:
	PRINTF	DBG_ENTRY,<'CDG--CDGPrevTrack()'>

		bsr	CDGAllNotesOff

		bsr.s	CDGClearScreen

		move.l	cdg_BaseData(a6),a0

		bsr	CDGRestartPack

		move.w	#(SPRITE_PTRK*16),d1
		moveq	#01,d0
		BRA_S	CDGPlay_GO

******* cdg.library/CDGFastForward ************************************
*
*   NAME
*	CDGFastForward -- Tell CD+G that the disk is in FF mode
*   
*   SYNOPSIS
*	VOID CDGFastForward( VOID )
*
*	VOID CDGFastForward( VOID );
*
*   FUNCTION
*	Tell the cdg.library that the disk is in the FF mode.
*
*	Sprites, passed in a CDGPrefs structure at CDGBegin()
*	time, are used to display the mode.
*
*   INPUTS
*	N/A
*
*   RETURNS
*	N/A
*
*   NOTES
*	The FF mode sprite(s) is displayed until CDGPlay() is called,
*	CDGBack() is called, or some other cdg.library mode function
*	is called. 
*
*	If +MIDI data has been sent, this function will also cause
*	a sequence of bytes to be transmitted out the serial/midi
*	port telling all midi channels to be quiet.
*
*   SEE ALSO
*	CDGBegin(), CDGPlay(), CDGBack()
*
***********************************************************************

CDGFastForward:
	PRINTF	DBG_ENTRY,<'CDG--CDGFastForward()'>

		bsr	CDGAllNotesOff

		bsr.s	CDGClearScreen

	; Display FF mode sprite

		move.l	cdg_BaseData(a6),a0

		move.l	#(((SPRITE_FF)<<20)!$FFFF),cdd_StartSprite(a0)

		bra	CDGRestartPack

	;;;	rts

******* cdg.library/CDGRewind *****************************************
*
*   NAME
*	CDGRewind -- Tell CD+G that the disk is in REW mode
*   
*   SYNOPSIS
*	VOID CDGRewind( VOID )
*
*	VOID CDGRewind( VOID );
*
*   FUNCTION
*	Tell CD+G that the disk is in the REW mode.
*
*	Sprites, passed in a CDGPrefs structure at CDGBegin()
*	time, are used to display the mode.
*
*   INPUTS
*	N/A
*
*   RETURNS
*	N/A
*
*   NOTES
*	The REW mode sprite(s) is displayed until CDGPlay() is called,
*	CDGBack() is called, or some other cdg.library mode function
*	is called. 
*
*	If +MIDI data has been sent, this function will also cause
*	a sequence of bytes to be transmitted out the serial/midi
*	port telling all midi channels to be quiet.
*
*   SEE ALSO
*	CDGBegin(), CDGPlay(), CDGBack()
*
***********************************************************************

CDGRewind:
	PRINTF	DBG_ENTRY,<'CDG--CDGRewind()'>

		bsr	CDGAllNotesOff

		bsr.s	CDGClearScreen

	; Display FF mode sprite

		move.l	cdg_BaseData(a6),a0

		move.l	#(((SPRITE_REW)<<20)!$FFFF),cdd_StartSprite(a0)

		bra.s	CDGRestartPack

	;;;	rts

******* cdg.library/CDGDiskRemoved ************************************
*
*   NAME
*	CDGDiskRemoved -- Tell CD+G that the CD has been removed
*   
*   SYNOPSIS
*	VOID CDGDiskRemoved( VOID )
*
*	VOID CDGDiskRemoved( VOID );
*
*   FUNCTION
*	Tell the cdg.library  that the CD has been removed.  If a front
*	panel VFD is available, the "+G", and/or "+MIDI" control panel
*	lights are turned off (if they are already on).
*
*   INPUTS
*	N/A
*
*   RETURNS
*	N/A
*
*   NOTES
*	If a front panel VFD is available, the cdg.library will turn
*	on "+G" and/or "+MIDI" indicator lights if a +G/+MIDI PACK
*	has been processed during CDGDraw().  There is currently
*	no good way to determine of a disk is +G/+MIDI before a
*	track has been played.
*
*	If +MIDI data has been sent, this function will also cause
*	a sequence of bytes to be transmitted out the serial/midi
*	port telling all midi channels to be quiet.
*
*   SEE ALSO
*
***********************************************************************

CDGDiskRemoved:
	PRINTF	DBG_ENTRY,<'CDG--CDGDiskRemoved()'>

		bsr	CDGAllNotesOff

		bsr.s	CDGClearScreen

		move.l	cdg_BaseData(a6),a0
	; turn off lights

		clr.b	cdd_TypeLights(a0)

	; Tell VBLANK to remove any active sprite at next VBLANK
	;
	; At next VBLANK the active sprite delay counter will roll
	; from 0->1, causing the VBLANK routine to clear the sprite
	; image (if any).  Works out well since the caller will not
	; be able to load his own view until after he has called
	; this function.

		moveq	#01,d0
		move.l	d0,cdd_StartSprite(a0)

		bra.s	CDGRestartPack

	;;;	rts

******* cdg.library/CDGClearScreen ************************************
*
*   NAME
*	CDGClearScreen -- Clear CD+G screen/reset colors to black
*   
*   SYNOPSIS
*	VOID CDGClearScreen( VOID )
*
*	VOID CDGClearScreen( VOID );
*
*   FUNCTION
*	This function is used to reset the screen.  The screen, and
*	border color(s) are reset to black.  The bitplanes are
*	cleared.
*
*   INPUTS
*	N/A
*
*   RETURNS
*	N/A
*
*   NOTES
*       The following functions automatically clear the CD+G screen --
*
*		CDGFastForward()
*		CDGRewind()
*		CDGNextTrack()
*		CDGPrevTrack()
*
*	CDGPlay() does not automatically clear the screen, so you
*	may wish to do so by calling this function.  In the case where
*	the user is changing tracks, or position via Next Track,
*	Previous Track, Fast Forward, or Rewind, there is no need to
*	clear the screen since functions are already provided which
*	do so.
*
*    SEE ALSO
*	CDGFastForward(), CDGRewind(), CDGNextTrack(), CDGPrevTrack(),
*	CDGPlay()
*
***********************************************************************

CDGClearScreen:
		movem.l	a5/a6,-(sp)

		move.l	cdg_BaseData(a6),a5

	; Tell VBLANK interrupt not to bother swapping bitplanes, or
	; changing colors now.  An atomic AND instruction is used to
	; clear both flags.

		and.b	#(~(VBFF_NEWCOLORS!VBFF_NEWPLANES)),cdd_VBlankFlags(a5)

	; reset border color to color 0

		lea	cdd_CLUTTable(a5),a0
		move.l	a0,cdd_BorderPen(a5)

	; clear CLUT

		moveq	#15,d0
		moveq	#00,d1
CDGCLS_CLUT:
		move.w	d1,(a0)+
		dbf	d0,CDGCLS_CLUT

	; swap Active/Inactive plane pointers

		movem.l	cdd_InactivePlanes(a5),d0/a0
		exg	d0,a0
		movem.l	d0/a0,cdd_InactivePlanes(a5)

	; Clear the memory we cannot see

                move.l	cdd_ActivePlanes(a5),a1
		move.l	(a1),a1			;clear 4 planes at once

	; clear bitplanes

		move.l	cdd_GfxLib(a5),a6

	; BltClear this memory - Use BytesPerRow mode, and WAIT
	; Shift DISPLAY_HEIGHT << 18 (16 to move to upper word, plus
	; * 4 for 4 bitplanes)

		move.l	#((DISPLAY_HEIGHT<<18)!(DISPLAY_WIDTH/8)),d0
		moveq	#((1<<1)!(1<<0)),d1

	PRINTF	DBG_OSCALL,<'CDG--BltClear($%lx,$%lx,$%lx)'>,A1,D0,D1

		JSRLIB	BltClear

	; reset PH & PV

RESET_PV	SET	(((DISPLAY_WIDTH/8)*FONT_HEIGHT))

		move.w	cdd_CopperList1+cdc_BPLCON1_DEF(a5),d0
		move.w	d0,d1
		lsl.w	#PFB_FINE_SCROLL_SHIFT,d1
		or.w	d1,d0			;odd/even planes

		move.w	#(RESET_PV),cdd_PHPV+2(a5)
		move.w	d0,cdd_PHPV(a5)

	; Tell VBLANK to swap bitplane pointers, and modify colors in
	; copper list at next VBLANK (uses an ATOMIC OR instruction)

		or.b	#(VBFF_NEWCOLORS!VBFF_NEWPLANES),cdd_VBlankFlags(a5)
		
		movem.l	(sp)+,a5/a6
		rts

*****i* cdg.library/CDGRestartPack ************************************
*
*   NAME
*	CDGRestartPack -- Restart PACK counter, and buffer
*   
*   SYNOPSIS
*	VOID CDGRestartPack( data )
*	                     A0
*
*	VOID CDGRestartPack( struct CDGData * );
*
*   FUNCTION
*	Private function to restart de-interleaving process.
*
*   INPUTS
*	data - pointer to private data area
*
*   RETURNS
*	N/A
*
*   NOTES
*	MUST preserve A0!
*
*	We need to restart the de-interleaving process because
*	we've stopped play, paused, ff, etc., which means the next
*	PACKET we get should not be de-interleaved with data from
*	another portion of the disk.
*
***********************************************************************

CDGRestartPack:


	; disable collection of PACKETS -- the interrupt will stop
	; modifying the PutPacket, and PackCount variables, so we
	; can change them here.

		bset	#EF2B_IGNORE,cdd_ExtraFlags2(a0)

	; reset validity checking counters

		move.b	#GPACKCOUNT,cdd_GPackCount(a0)
		move.b	#MPACKCOUNT,cdd_MPackCount(a0)

		clr.b	cdd_FRollCount(a0)

	; reset pack counter, and GetPacket pointer

		clr.w	cdd_PackCount(a0)
		move.l	cdd_PutPacket(a0),cdd_GetPacket(a0)

	; we leave with PACKET data collection disabled, and it will
	; remain disabled until we re-enable it by calling CDGPlay()

		rts


		END