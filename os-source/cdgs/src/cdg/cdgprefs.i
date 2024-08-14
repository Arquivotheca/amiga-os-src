	IFND	CDTV_CDGPREFS_I
CDTV_CDGPREFS_I		SET	1

**
**	$Id: cdgprefs.i,v 1.9 92/03/06 09:32:26 darren Exp $
**
**	CDTV CD+G -- cdgprefs.i (CD+G preferences structure)
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

** Macros

CDGNAME			MACRO
			dc.b	'cdg.library',0
			ENDM
** Includes

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/tasks.i"

		; The order, and position of sprite data pointers in
		; this structure is important - do not change.
		; Additional fields may be appended to the structure

	STRUCTURE	CDGPrefs,0

		; Pointer array to 64 sprites for channel indicator.
		; Each channel is displayed using 4 sprites.  2 attached
		; sprites for the left side of the channel number, and 2
		; attached sprites for the right side of the channel number.
		; Because there are a possible 0-15 [hence 16 total] possible
		; channels, 64 sprites are required.
		;
		; Sprite 0 in the array is the default imagery, and should
		; generally be all 0's for an invisible image.

		STRUCT	cdgp_ChannelSprites,(64*4)

		; Pointer array to the 4 sprites used to display that
		; the CD is in PAUSE mode.
		; 
		; Should be similar to the universal PAUSE symbol used on
		; most remote controllers, and stylistically similar to the
		; imagery used to display channel selection.

		STRUCT	cdgp_PAUSESprite,(4*4)

		; Pointer array to the 4 sprites used to display that
		; the CD is in the SEARCH for NEXT TRACK mode.
		; 
		; Should be similar to the universal NEXT TRACK symbol used on
		; most remote controllers, and stylistically similar to the
		; imagery used to display channel selection.

		STRUCT	cdgp_NTrackSprite,(4*4)

		; Pointer array to the 4 sprites used to display that
		; the CD is in the SEARCH for PREVIOUS TRACK mode.
		; 
		; Should be similar to the universal PREV TRACK symbol used on
		; most remote controllers, and stylistically similar to the
		; imagery used to display channel selection.

		STRUCT	cdgp_PTrackSprite,(4*4)

		; Pointer array to the 4 sprites used to display that
		; the CD is in the FAST FORWARD mode.
		; 
		; Should be similar to the universal FF symbol used on
		; most remote controllers, and stylistically similar to the
		; imagery used to display channel selection.

		STRUCT	cdgp_FFSprite,(4*4)

		; Pointer array to the 4 sprites used to display that
		; the CD is in the REWIND mode.
		; 
		; Should be similar to the universal REWIND symbol used on
		; most remote controllers, and stylistically similar to the
		; imagery used to display channel selection.

		STRUCT	cdgp_RWSprite,(4*4)

		; Pointer array to the 4 sprites used to display that
		; the CD is in the STOP mode.
		; 
		; Should be similar to the universal STOP symbol used on
		; most remote controllers, and stylistically similar to the
		; imagery used to display channel selection.

		STRUCT	cdgp_STOPSprite,(4*4)

		; Pointer array to the 4 sprites used to display that
		; the CD is in the PLAY mode.
		; 
		; Should be similar to the universal PLAY symbol used on
		; most remote controllers, and stylistically similar to the
		; imagery used to display channel selection.

		STRUCT	cdgp_PLAYSprite,(4*4)

		; Sprite height for all sprites.  Optimal height is 32, and
		; height does not include the posctrl words, or NULL
		; terminating words.

		UWORD	cdgp_SpriteHeight;

		; 8 colors for sprites.  Attached sprites are used, but
		; Amiga color palette limitations prevent us from using
		; all 15 colors - colors are stored here as 8 UWORDS

		STRUCT	cdgp_SpriteColors,(8*2)

		; top/left screen X position (set to 0 for default)

		WORD	cdgp_DisplayX

		; top/left screen Y position (set to 0 for default)

		WORD	cdgp_DisplayY

		; see flags below

		UWORD	cdgp_Control

		; Reserved flags.  Must be set to 0 for now

		UWORD	cdgp_Reserved0

		; Reserved flags.  Must be set to 0 for now

		UWORD	cdgp_Reserved1

		; Optional signal mask (see cdgp_SigTask below)

		ULONG	cdgp_SigMask

		; Optional pointer - Task to signal when a new PACKET is
		; available.  Read the documentation for CDGDraw() regarding
		; CPU bandwith limitations.  Set to NULL if you do not
		; want your task signaled.

		APTR	cdgp_SigTask

		LABEL	CDGPrefs_SIZEOF

** Flag bits for cdgp_Control

		; The CDGB_NOECC bit should be set to DISABLE PACK
		; error correction.  Without error correction,
		; PACK data may not be very reliable, though even
		; with error correction enabled the integrity of
		; PACK data cannot be guaranteed.  Error correction
		; requires more CPU time during CDGDraw().
		;

		BITDEF	CDG,NOECC,0

		; The CDGB_NOMIDI bit should be set to DISABLE +MIDI
		; processing.  When set, CDGBegin() will not try
		; to obtain ownership of the serial port, and +MIDI
		; data is ignored during CDGDraw().  This is intended
		; for use in an environment in which the serial port is
		; being used for other purposes.
		;

		BITDEF	CDG,NOMIDI,1

		; The CDGB_ALTSPRITES bit means you want cdg.library
		; to try and use sprites 0-3 inclusive for sprite symbols.
		; By default cdg.library will try to GetSprite()
		; sprites 2-5 inclusive.  Use of sprites 2-5 is
		; required when Intition is active (since Intuition
		; uses sprites 0).  However because of DMA limitations,
		; the absolute left edge position of the CD+G screen
		; will be more restricted to avoid loss of sprite DMA
		; when sprites 2-5 are used.
		;
		; CDGBegin() will fail if the sprites can not be owned
		; via the GetSprite() function in graphics.library
		;
		; This feature may, or may not be implemented.  If
		; implemented, it will try to fall back to sprites
		; 2-5 if sprite 0-1 is in use.  Intuition cannot be
		; started via OpenLibrary() if this feature is used.
		; In general this flag is meant for Operating System
		; features like the built-in player module.

		BITDEF	CDG,ALTSPRITES,2

** return values, and argument bits for CDGDraw()


		; The CDGB_GRAPHICS bit is used as both an argument
		; and a return value for CDGDraw().  As an argument,
		; it is used to enable the processing of +G PACKS.
		; As a return value, it means that a +G PACK has
		; been processed, and the disk is believed to be a
		; +G disk.

		BITDEF	CDG,GRAPHICS,0

		; The CDGB_MIDI bit is used as both an argument
		; and a return value for CDGDraw().  As an argument,
		; it is used to enable the processing of +MIDI PACKS.
		; As a return value, it means that a +MID PACK has
		; been processed, and the disk is believed to be a
		; +MIDI disk.

		BITDEF	CDG,MIDI,1

		ENDC	; CDTV_CDGPREFS_I
