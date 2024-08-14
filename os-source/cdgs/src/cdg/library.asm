**
**	$Id: library.asm,v 1.25 93/02/03 12:00:39 darren Exp $
**
**	CDTV CD+G -- main.asm (start-up code)
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
		INCLUDE	"cdg_rev.i"
** Imports

		XREF	EndModule	;EndModule.asm
		XREF	CDGDraw		;dispatch.asm
		XREF	CDGUserPack

		XREF	_intena		;amiga.lib
		XREF	_intreq
		XREF	_custom		;amiga.lib
		XREF	_LVOPermit

		XREF	CDGChannel	;modes.asm
		XREF	CDGPause
		XREF	CDGStop
		XREF	CDGPlay
		XREF	CDGNextTrack
		XREF	CDGPrevTrack
		XREF	CDGFastForward
		XREF	CDGRewind
		XREF	CDGClearScreen
		XREF	CDGDiskRemoved

** Exports
		XDEF	cdgname

		SECTION	cdg

** ROMTAG

	IFEQ	ROMBUILD

		moveq	#-1,d0
		rts

	ENDC

** ROM TAG for cdg.library

cdgresident:
		dc.w	RTC_MATCHWORD
		dc.l	cdgresident
		dc.l	EndModule
		dc.b	RTF_COLDSTART!RTF_AUTOINIT
		dc.b	VERSION
		dc.b	NT_LIBRARY
		dc.b	7		;before playerprefs (same as classic)
		dc.l	cdgname
		dc.l	ident
		dc.l	CDInitTable


cdgname:
		CDGNAME

ident:
		VSTRING

timername:
		TIMERNAME

utilname:
		UTILITYNAME

version:
		VERSTAG

		ds.w	0

CDFuncInit:
	; library function init

		dc.w	-1
		dc.w	CDGOpen-CDFuncInit
		dc.w	CDGClose-CDFuncInit
		dc.w	CDGExpunge-CDFuncInit
		dc.w	CDGExtFunc-CDFuncInit
		dc.w	CDGBegin-CDFuncInit
		dc.w	CDGEnd-CDFuncInit
		dc.w	CDGFront-CDFuncInit
		dc.w	CDGBack-CDFuncInit
		dc.w	CDGDraw+(*-CDFuncInit)
		dc.w	CDGChannel+(*-CDFuncInit)
		dc.w	CDGPause+(*-CDFuncInit)
		dc.w	CDGStop+(*-CDFuncInit)
		dc.w	CDGPlay+(*-CDFuncInit)
		dc.w	CDGNextTrack+(*-CDFuncInit)
		dc.w	CDGPrevTrack+(*-CDFuncInit)
		dc.w	CDGFastForward+(*-CDFuncInit)
		dc.w	CDGRewind+(*-CDFuncInit)
		dc.w	CDGClearScreen+(*-CDFuncInit)
		dc.w	CDGDiskRemoved+(*-CDFuncInit)
		dc.w	CDGUserPack+(*-CDFuncInit)
		dc.w	CDGAllocPrefs-CDFuncInit
		dc.w	CDGFreePrefs-CDFuncInit
		dc.w	-1
	
CDStructInit:

	; library struct initialization

		INITLONG	LN_NAME,cdgname
		INITLONG	LIB_IDSTRING,ident
		INITWORD	LIB_REVISION,REVISION
		INITWORD	LIB_VERSION,VERSION
		INITBYTE	LN_TYPE,NT_LIBRARY

		dc.w	0
		ds.w	0

CDInitTable:
		dc.l	CDGBase_SIZEOF
		dc.l	CDFuncInit
		dc.l	CDStructInit
		dc.l	CDGMain

*****i* cdg.library/CDGMain *******************************************
*
*   NAME
*	CDGMain -- Main initialization
*   
*   SYNOPSIS
*	VOID CDGMain( CDGBase )
*	                 d0
*
*	VOID CDGMain( struct CDGBase * );
*
*   FUNCTION
*	Main initialization of small library base.  Most memory is
*	allocated when CD+G is started, and freed when ended.
*
*   RETURNS
*	N/A
*
*   NOTES
*
***********************************************************************

CDGMain:
	PRINTF	DBG_ENTRY,<'CDG--CDGMain($%lx)'>,D0

		move.l	d0,a0
		move.l	a6,cdg_ExecLib(a0)	;cache execbase
		rts
		
******* cdg.library/OpenLibrary ***************************************
*
*   NAME
*	OpenLibrary -- Open the cdg.library
*   
*   SYNOPSIS
*	cdgbase = OpenLibrary(libName, version )
*	D0                    A0       D0
*
*	struct Library *OpenLibrary( STRPTR, ULONG );
*
*   FUNCTION
*	Returns pointer to library base if the library has not been
*	previously opened.  Because of limited hardware resources
*	(e.g., sprites, and CPU bandwidth) only one CD+G display
*	is currently feasable.
*
*	Therefore this OpenLibrary() of "cdg.library" returns a
*	pointer to the library base only if some other task has not
*	already opened it.
*	
*	You must call CDGBegin() with a CDGPrefs structure to actually
*	start CD+G.
*
*   INPUTS
*	libName - pointer to string "cdg.library".
*
*	version - Minimum version number requested should be 38.
*
*   RETURNS
*	Library base pointer, or NULL indicating the library is in use
*	or not available.
*
*   NOTES
*	Prior to V39 of this library, there were no public library
*	functions.  V39 provides many new public functions, and is
*	not compatable with the CDTV version of cdg.library.
*
*   SEE ALSO
*	CDGBegin(), exec.library/OpenLibrary, exec.library/CloseLibrary()
*
***********************************************************************

CDGOpen:
	PRINTF	DBG_ENTRY,<'CDG--CDGOpen($%lx)'>,A6

		moveq	#00,d0

	; only allow 1 opener for now

		tst.w	LIB_OPENCNT(a6)
		bne.s	CDGOpen_Inuse

		addq.w	#1,LIB_OPENCNT(a6)

	; typical case is the branch is NEVER taken

		move.l	a6,d0
CDGOpen_Inuse:
		rts


*****i* cdg.library/CDGClose ******************************************
*
*   NAME
*	CDGClose -- Close library
*   
*   SYNOPSIS
*	result = CDGClose( CDGBase )
*	D0                    A6
*
*	ULONG CDGClose( struct CDGBase * );
*
*   FUNCTION
*	
*	Closes the cdg.library.
*
*   RETURNS
*	Always NULL.
*
*   NOTES
*
***********************************************************************

CDGClose:
	PRINTF	DBG_ENTRY,<'CDG--CDGClose($%lx)'>,A6

		subq.w	#1,LIB_OPENCNT(a6)

		; fall through

*****i* cdg.library/CDGExpunge *****************************************
*
*   NAME
*	CDGExpunge -- Expunge library
*   
*   SYNOPSIS
*	result = CDGExpunge( CDGBase )
*	D0                    A6
*
*	ULONG CDGExpunge( struct CDGBase * );
*
*   FUNCTION
*	"cdg.library" does not really ever expunge.  Only a very small
*	piece of memory is used until CDBegin() is called.
*
*   RETURNS
*	Always NULL.
*
*   NOTES
*
***********************************************************************

CDGExpunge

		; fall through

*****i* cdg.library/CDGExtFunc ****************************************
*
*   NAME
*	CDGExtFunc -- Do nothing function
*   
*   SYNOPSIS
*	result = CDGExtFunc( CDGBase )
*	D0                    A6
*
*	ULONG CDGExtFunc( struct CDGBase * );
*
*   FUNCTION
*	Do nothing - reserved function vector.
*
*   RETURNS
*	Always NULL.
*
*   NOTES
*
***********************************************************************

CDGExtFunc:

		moveq	#00,d0
		rts


******* cdg.library/CDGBegin ******************************************
*
*   NAME
*	CDGBegin -- Start the CD+G display system
*   
*   SYNOPSIS
*	result = CDGBegin( CDGPrefs )
*	D0                    A0
*
*	BOOL CDGBegin( struct CDGPrefs * );
*
*   FUNCTION
*	Starts CD+G with given preferences.  You must call CDGFront()
*	to bring the CD+G screen to front.  CD+G uses graphics.library
*	functions, and is not meant to be used in an Intuition
*	screen environment.
*
*   INPUTS
*	CDGPrefs -- pointer to a CDGPrefs structure.  The structure
*	includes the following information -
*
*		Sprite image data for CD+G Channel selection.
*
*		Sprite image data for CD play, pause, etc.
*
*		Sprite height, and color table.
*
*		Top/Left edge corder of CD+G screen.
*
*		Enable/Disable +G and/or +MIDI PACK processing.
*
*		Enable/Disable error correction of PACKS.
*
*		An optional task pointer, and signal mask to
*		Signal() when a new PACKET is available.
*
*   RETURNS
*	TRUE if CD+G started.  FALSE indicates failure, most likely
*	due to lack of memory, or inability to obtain some other
*	resource (e.g, use of sprites 2-5, unable to open the cd.device,
*	etc.)
*
*   NOTES
*	Once started, the CD+G library will start buffering CD+G PACKETS.
*	These packets will be lost if you do not call CDGDraw() often
*	enough.  The CD+G screen can be made visible by calling
*	CDGFront().
*
*   SEE ALSO
*	CDGFront(), cdg/cdgprefs.h, cdg/cdgprefs.i
*
***********************************************************************

CDGBegin:

	PRINTF	DBG_ENTRY,<'CDG--CDGBegin($%lx)'>,A0

		movem.l	a4/a5,-(sp)

		move.l	a0,a4		;Pass to _CDGInit()
		move.l	a6,a5

		BSRSELF	_CDGInit

		tst.l	d0
		beq.s	CDGBegin_Fail

		move.l	d0,cdg_BaseData(a6)

	; IMPORTANT -- return a BOOL - not the lower word of a data area!!!
	; BOOL's are SHORTS

		moveq	#01,d0

CDGBegin_Fail:
		movem.l	(sp)+,a4/a5

	PRINTF	DBG_EXIT,<'CDG--$%lx=CDGBegin()'>,D0

		rts

******* cdg.library/CDGEnd ********************************************
*
*   NAME
*	CDGEnd -- End CDG playing
*   
*   SYNOPSIS
*	VOID CDGEnd( VOID )
*
*	VOID CDGEnd( VOID );
*
*   FUNCTION
*	Stops CD+G.  All memory is freed.  Should never be called
*	if CDGBegin() failed.
*
*   RETURNS
*	N/A
*
*   NOTES
*
*   SEE ALSO
*	CDGBegin()
*
***********************************************************************

CDGEnd:

	PRINTF	DBG_ENTRY,<'CDG--CDGEnd()'>

		move.l	a5,-(sp)

	; restore SPRITE DMA

		bsr	CDGBack

		move.l	a6,a5

		BSRSELF	_CDGFree

		clr.l	cdg_BaseData(a6)

		move.l	(sp)+,a5
		rts

******* cdg.library/CDGFront ******************************************
*
*   NAME
*	CDGFront -- Bring CDG screen to front
*   
*   SYNOPSIS
*	VOID CDGFront( VOID )
*
*
*	VOID CDGFront( VOID );
*
*   FUNCTION
*	Brings CD+G screen to front.  Programmer must call CDGBegin()
*	first - failure to do so will result in a crash.
*
*   RETURNS
*	N/A
*
*   NOTES
*	Recommend calling CDGDraw() once before calling this function.
*	This will cause the CD+G display to get up to date before
*	its screen is brought to front.  Do not call this function
*	in a busy loop; not needed.
*
*	See CDGBack() which should be called before bringing your
*	own screen to front.
*
*   SEE ALSO
*	CDGDraw(), CDGBegin(), CDGBack()
*
***********************************************************************

CDGFront:
	PRINTF	DBG_ENTRY,<'CDG--CDGFront()'>
		movem.l	d2/d3/a2/a5/a6,-(sp)
		move.l	cdg_BaseData(a6),a5
		move.l	cdd_GfxLib(a5),a6

	; patch up ViewPort color map so that gfx copinit is set-up
	; properly when we bring this screen to front.

		lea	cdd_ViewPort1(a5),a0
		move.l	cdd_BorderPen(a5),a1
		moveq	#00,d0		;pen #
		move.l	d0,d1
		move.w	(a1),d1
		move.l	d1,d2
		move.l	d1,d3
		lsr.w	#8,d1
		lsr.w	#4,d2
		and.w	#$0f,d1		;red
		and.w	#$0f,d2		;green
		and.w	#$0f,d3		;blue

	PRINTF	DBG_OSCALL,<'CDG--SetRGB4($%lx,$%lx,$%lx,$%lx,$%lx)'>,A0,D0,D1,D2,D3

		JSRLIB	SetRGB4

		lea	cdd_View1(a5),a1

	PRINTF	DBG_OSCALL,<'CDG--LoadView($%lx,$%lx)'>,A1,A6

		JSRLIB	LoadView

		lea	cdd_ViewPort1(a5),a0
		lea	cdd_SimpleSprite2(a5),a1
		move.l	cdd_4SpriteData(a5),a2

	PRINTF	DBG_OSCALL,<'CDG--ChangeSprite($%lx,$%lx,$%lx)'>,A0,A1,A2

		JSRLIB	ChangeSprite

		lea	cdd_ViewPort1(a5),a0
		lea	cdd_SimpleSprite3(a5),a1
		move.l	cdd_SpriteData3(a5),a2

	PRINTF	DBG_OSCALL,<'CDG--ChangeSprite($%lx,$%lx,$%lx)'>,A0,A1,A2

		JSRLIB	ChangeSprite

	; ARGG - SPRITE_ATTACHED is not defined in graphics/sprite.i!!!

	IFND	SPRITE_ATTACHED
SPRITE_ATTACHED	EQU	$80
	ENDC

		ori.b	#SPRITE_ATTACHED,3(a2)

		lea	cdd_ViewPort1(a5),a0
		lea	cdd_SimpleSprite4(a5),a1
		move.l	cdd_SpriteData4(a5),a2

	PRINTF	DBG_OSCALL,<'CDG--ChangeSprite($%lx,$%lx,$%lx)'>,A0,A1,A2

		JSRLIB	ChangeSprite

		lea	cdd_ViewPort1(a5),a0
		lea	cdd_SimpleSprite5(a5),a1
		move.l	cdd_SpriteData5(a5),a2

	PRINTF	DBG_OSCALL,<'CDG--ChangeSprite($%lx,$%lx,$%lx)'>,A0,A1,A2

		JSRLIB	ChangeSprite

		ori.b	#SPRITE_ATTACHED,3(a2)

	; Make sure sprite dma is turned ON!  Save old value too since
	; playerprefs tries to turn it off.

		lea	_custom,a0

		move.w	dmaconr(a0),cdd_CacheDMACONR(a5)
		move.w	#(DMAF_SETCLR!DMAF_SPRITE),dmacon(a0)

	; indicate sprite imagery may be displayed

		bclr	#VBFB_BACKSCREEN,cdd_VBlankFlags(a5)

	; reload colors so copinit is also right

		bset	#VBFB_NEWCOLORS,cdd_VBlankFlags(a5)

		movem.l	(sp)+,d2/d3/a2/a5/a6
		rts

******* cdg.library/CDGBack *******************************************
*
*   NAME
*	CDGBack -- Called before you bring your own screen to front.
*   
*   SYNOPSIS
*	VOID CDGBack( VOID )
*
*
*	VOID CDGBack( VOID );
*
*   FUNCTION
*	Tells CD+G you are going to bring your own screen in front
*	of CD+G's screen.  This tells CD+G that it should not display
*	any sprites, or bring its own screen to front.  The programmer
*	must call CDGBegin() first - failure to do so will result in a
*	crash.
*
*   RETURNS
*	N/A
*
*   NOTES
*
*   SEE ALSO
*	CDGBegin(), CDGFront()
*
***********************************************************************

CDGBack:
	PRINTF	DBG_ENTRY,<'CDG--CDGBack()'>

		movem.l	a5/a6,-(sp)

		move.l	cdg_BaseData(a6),a5

	; Tell VBLANK to remove any active sprite at next VBLANK
	;
	; At next VBLANK the active sprite delay counter will roll
	; from 0->1, causing the VBLANK routine to clear the sprite
	; image (if any).  Works out well since the caller will not
	; be able to load his own view until after he has called
	; this function.

		moveq	#01,d0
		move.l	d0,cdd_StartSprite(a5)

	; indicate sprite imagery should not be displayed

		or.b	#(VBFF_BACKSCREEN!VBFF_RESETDMA),cdd_VBlankFlags(a5)	;atomic!

	; wait for VBLANK before returning, and reset of SPRITE DMA before returning

		move.l	cdd_GfxLib(a5),a6
CDGBack_Spin:
		JSRLIB	WaitTOF
		btst	#VBFB_RESETDMA,cdd_VBlankFlags(a5)
		bne.s	CDGBack_Spin

		movem.l	(sp)+,a5/a6


		rts


******* cdg.library/CDGAllocPrefs *************************************
*
*   NAME
*	CDGAllocPrefs -- Used to allocate a CDGPrefs structure
*   
*   SYNOPSIS
*	cdgprefs = CDGAllocPrefs( VOID )
*	D0
*
*	struct CDGPrefs *CDGAllocPrefs( VOID );
*
*   FUNCTION
*	Used to allocate a CDGPrefs structure.  All fields are initialized
*	to NULL.
*
*	For future compatability, this function should be used to
*	allocate a CDGPrefs structure for use with CDGBegin().
*
*   INPUTS
*	N/A
*
*   RETURNS
*	cdgprefs - pointer to CDGPrefs structure, or NULL if the
*	memory could not be allocated.
*
*   NOTES
*
*   SEE ALSO
*	CDGFreePrefs(), CDGBegin(), cdg/cdgprefs.h, cdg/cdgprefs.i
*
***********************************************************************

CDGAllocPrefs:

	PRINTF	DBG_ENTRY,<'CDG--CDGAllocPrefs()'>

		move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1	;accessed by interrupt
		move.l	#CDGPrefs_SIZEOF,d0

		move.l	a6,-(sp)
		move.l	cdg_ExecLib(a6),a6
		JSRLIB	AllocVec
		move.l	(sp)+,a6
		rts

******* cdg.library/CDGFreePrefs **************************************
*
*   NAME
*	CDGFreePrefs -- Used to free a CDGPrefs structure
*   
*   SYNOPSIS
*	VOID CDGFreePrefs( cdgprefs )
*			      A1
*
*	VOID CDGFreePrefs( struct CDGPrefs * );
*
*   FUNCTION
*	Used to free a CDGPrefs structure allocated with CDGAllocPrefs().
*
*	For future compatability, this function should be used to
*	free a CDGPrefs structure.
*
*   INPUTS
*	cdgprefs - pointer to CDGPrefs structure allocated with
*	CDGAllocPrefs().
*
*   RETURNS
*	N/A
*
*   NOTES
*
*   SEE ALSO
*	CDGAllocPrefs()
*
***********************************************************************

CDGFreePrefs:

	PRINTF	DBG_ENTRY,<'CDG--CDGAFreePrefs($%lx)'>,A1

		move.l	a6,-(sp)
		move.l	cdg_ExecLib(a6),a6
		JSRLIB	FreeVec
		move.l	(sp)+,a6
		rts

		END