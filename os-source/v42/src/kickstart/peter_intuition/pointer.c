/*** pointer.c *****************************************************************
 *
 * pointer.c -- New Intuition pointer routines
 *
 *  $Id: pointer.c,v 39.31 93/05/05 11:55:42 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1992, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

/* This optimization depends on GfxBase maintaining a VBlank counter.
 * the idea is that in place of WaitTOF()/FreeXXX(), we send ourselves
 * a token (reminder) to FreeXXX after the VBlank counter has had
 * a chance to change, thus taking a WaitTOF() clean out of setMousePointer().
 */
#define WAITTOF_OPTIMIZE	0

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef GRAPHICS_DISPLAYINFO_H
#include <graphics/displayinfo.h>
#endif

#ifndef GRAPHICS_SPRITE_H
#include <graphics/sprite.h>
#endif

#ifndef UTILITY_PACK_H
#include <utility/pack.h>
#endif

#include "intuall.h"
#include "intuition.h"
#include "pointerclass.h"
#include "classusr.h"
#include "classes.h"
#include "iprefs.h"

#include "pointer_protos.h"

/*---------------------------------------------------------------------------*/

static int calcSpriteWidth(LONG width);

static int setSpriteHeight(void);

static struct ExtSprite * MakeSpriteData(struct MousePointer * mpointer,
                                         ULONG word_width,
                                         LONG scale);

static ULONG dispatchPointerClass(Class * cl,
                                  Object * o,
                                  Msg msg);

/*---------------------------------------------------------------------------*/

#define D(x)		;	/* misc debugging */
#define DSDM(x)		;	/* setDefaultMousePointer */
#define DSBP(x)		;	/* set busy pointer */
#define DSMP(x)		;	/* setMousePointer */
#define DAS(x)		;	/* MakeSpriteData() */
#define DSW(x)		;	/* Sprite word-width */

#define DPC(x)		;	/* pointerclass */
#define DSS(x)		;	/* setSpriteSpeed */
#define DSH(x)		;	/* setSpriteHeight */
#define DSP(x)		;	/* SetPointer/ClearPointer */


#define DUPM(x)		;	/* updateMousePointer */
#define DUPM2(x)	;	/* updateMousePointer */
#define DUPMC(x)	;	/* updateMousePointer change sprite */

#define DDF(x)		;	/* run time debug flag testing	*/
#define DP(x)		;	/* old setDefaultPointer */
#define DKILL(x)	;	/* pointer kill */

#if 0
#define ICOLOR(c)	 (*((SHORT *)0xdff180) = (c))
#define CKCOLOR0 (0xf00)
#define CKCOLOR1 (0x000)
#define CKCOLOR2 (0x0f0)
#define CKCOLOR3 (0x000)
#define CKCOLOR4 (0x00f)
#define CKCOLOR5 (0x000)
#define CKCOLOR6 (0xfff)
#define CKCOLOR7 (0x000)
#define CKCOLOR8 (0xff0)
#define CKCOLOR9 (0x000)
#define CKCOLOR10 (0x0ff)

#define DONECOLOR (0xfff)

#else
#define ICOLOR(c)	;
#endif


/*
 * initMousePointer()
 *	- One-time init.
 *
 * setDefaultMousePointer()
 *	- Change pref pointer.
 *
 * ISetWindowPointer()
 *	- Change the window's custom pointer.
 *
 * setMousePointer()
 *	- Establish the sprite based on the correct active pointer.
 *
 * calcSpriteWidth()
 *	- Figure out whether to use 16/32/64 bits for the sprite.
 *
 * reinstatePointer()
 *	- rebound from gfx killing our pointer
 *
 * MakeSpriteData()
 *
 */

struct ExtSprite *MakeSpriteData();

struct BitMap *setDefaultMousePointer();

extern UBYTE *PointerClassName;

/* the Intuition pointer, though defined as ROMPOINTER_HEIGHT tall,
 * has memory allocated for 16 tall, in case User wants to change it.
 * (the definition is in intuinternal.h)
 * ZZZ: This has something to do with GetPrefs() and friends.  Depending
 * what we do there, we may be able to shave the pointer image down.
 */

/* Here are the bitmap planes for the Intuition default pointer
 * and default busy pointer (note that AllocSpriteData() does not
 * require that they be in chip RAM):
 */

UWORD RomPointer0[] =
{
    0xC000,
    0x7000,
    0x3C00,
    0x3F00,

    0x1FC0,
    0x1FC0,
    0x0F00,
    0x0D80,

    0x04C0,
    0x0460,
    0x0020,
    0x0000,

    0x0000,
    0x0000,
    0x0000,
    0x0000,

};

UWORD RomPointer1[] =
{
    0x4000,
    0xB000,
    0x4C00,
    0x4300,

    0x20C0,
    0x2000,
    0x1100,
    0x1280,

    0x0940,
    0x08A0,
    0x0040,
    0x0000,

    0x0000,
    0x0000,
    0x0000,
    0x0000,
};

UWORD RomBusyPointer0[] =
{
    0x0400,
    0x0000,
    0x0100,
    0x0000,

    0x07C0,
    0x1FF0,
    0x3FF8,
    0x3FF8,

    0x7FFC,
    0x7EFC,
    0x7FFC,
    0x3FF8,

    0x3FF8,
    0x1FF0,
    0x07C0,
    0x0000,
};

UWORD RomBusyPointer1[] =
{
    0x07C0,
    0x07C0,
    0x0380,
    0x07E0,

    0x1FF8,
    0x3FEC,
    0x7FDE,
    0x7FBE,

    0xFF7F,
    0xFFFF,
    0xFFFF,
    0x7FFE,

    0x7FFE,
    0x3FFC,
    0x1FF8,
    0x07E0,
};


/* We want to look at GfxBase->MemType or GfxBase->BoardMemType to determine
 * the number of pointer sizes to deal with.  (Board)MemType is defined in a
 * manner convenient for Graphics,but not super-suitable for us:
 *
 * MemType   System Type     Number of Pointers
 *    0   -> 1X system       1
 *    1,2 -> 2X system       2
 *    3   -> 4X system       3
 *
 * This little bit of amazing math gives the right result, but would need to
 * be changed if we ever had different values of (Board)MemType.
 * Pay no attention to the man behind the curtain!
 */

#define NUMPOINTERS( memtype )	( ( (memtype) + 3 ) / 2 )

/*** initMousePointer() ***
 *
 * This routine manages the initialization of much of the pointer stuff.
 * It figures out how many pointer sizes Intuition will need to make.
 * It sets up the pointer-kill callback stuff.  It init's the standard
 * default and busy pointers.
 *
 * STATUS:  Consider putting in-line in init.c
 *
 */

initMousePointer()
{
    struct IntuitionBase *IBase = fetchIBase();
    struct BitMap pointer_bm;
    struct BitMap *busy_bm;

    void pointerKill();

    /* Set up the vector and data that gfx uses when it kills our
     * sprite-pointer for compatibility reasons.
     */
    IBase->GfxBase->IData = (APTR) IBase;
    IBase->GfxBase->IVector = (APTR) pointerKill;

    /* Let's set up the pointer bitmap.  Note that AllocSpriteData()
     * does not require that the plane data be in chip.
     */
    InitBitMap( &pointer_bm, 2, ROMPOINTER_WIDTH, ROMPOINTER_HEIGHT );
    pointer_bm.Planes[0] = (PLANEPTR)RomPointer0;
    pointer_bm.Planes[1] = (PLANEPTR)RomPointer1;
    DP( printf("initial sDIP ...\n"));
    setDefaultMousePointer( &pointer_bm,
	ROMPOINTER_HOTX, ROMPOINTER_HOTY,
	ROMPOINTER_XRESN, ROMPOINTER_YRESN,
	ROMPOINTER_WORDWIDTH, NPT_DEFAULT );
    DP( printf(" returned\n"));

    /* Failure here doesn't matter, since a NULL value causes
     * us to fall back to the default pointer.
     */
    DSBP( printf("About to alloc busy pointer\n"));
    /* For the default pointer, we don't need to keep the bitmap around,
     * since setDefaultMousePointer() allocates sprites in each width,
     * and then we're done with the bitmap.
     * However, the busy-pointer is handled like any custom pointer,
     * which is to say, sprite-data are generated on-demand only.  So
     * the bitmap needs to persist.
     */
    if ( busy_bm = IBase->BusyPointerBitMap =
	(struct BitMap *)AllocVec( sizeof(struct BitMap), MEMF_CLEAR ) )
    {
	InitBitMap( busy_bm, 2, ROMBUSYPOINTER_WIDTH, ROMBUSYPOINTER_HEIGHT );
	busy_bm->Planes[0] = (PLANEPTR)RomBusyPointer0;
	busy_bm->Planes[1] = (PLANEPTR)RomBusyPointer1;
	setDefaultMousePointer( busy_bm,
	    ROMBUSYPOINTER_HOTX, ROMBUSYPOINTER_HOTY,
	    ROMBUSYPOINTER_XRESN, ROMBUSYPOINTER_YRESN,
	    ROMBUSYPOINTER_WORDWIDTH, NPT_BUSY );
    }
    DSBP( printf("Done allocing busy pointer\n"));

    /* setMousePointer() done inside setDefaultMousePointer() */
}

/*** setDefaultMousePointer() ***
 *
 * Installs into Intuition a new default imagery for the standard
 * or busy pointer.  Caller supplies a BitMap, hot-spot offset
 * resolution and width information.
 *
 * For the default pointer, Intuition will make MousePointer structure
 * for it, then allocate an ExtSprite for each available sprite-size, and
 * make IBase->DefaultExtSprite point at it.  The previous default
 * pointer's ExtSprites are freed.  Note that Intuition doesn't currently
 * need to keep the BitMap of the default pointer, since all possible
 * pointers (3 sizes) are already created.  If the call fails, the partial
 * result is freed, and NULL is returned.  If it succeeds, the pointer
 * to the input BitMap is returned (so it can be freed by IPrefs).
 *
 * For the busy pointer Intuition just allocates a MousePointer structure
 * for it.  Returns 0 for failure, ~0 for success, or a pointer to the
 * previously installed busy pointer BitMap, for IPrefs to free.
 *
 * The caller must do the following, based on the return code:
 *
 *	~0 - everything is fine, do nothing.
 *	 0 - something failed, please free the BitMap you called with.
 *   other - This is a pointer to a BitMap to be freed (it could be the
 *	     one you just passed in, or it could be the one passed in
 *	     the previous time).
 *
 */

#define SUCCESS ((struct BitMap *)~0)

struct BitMap *
setDefaultMousePointer( bm, xoffset, yoffset, xresn, yresn, wordwidth, type )
struct BitMap *bm;
LONG xoffset;
LONG yoffset;
LONG xresn;
LONG yresn;
LONG wordwidth;
LONG type;
{
    struct BitMap *free_result = NULL;
    LONG size;
    struct MousePointer *newmpointer;
    LONG designsize;
    LONG numpointers;
    struct IntuitionBase *IBase = fetchIBase();

    extern UBYTE *PointerClassName;

    /* Figure out how many different pointers this system
     * potentially supports.  We need to allocate based on BoardMemType,
     * which reflects what GB->MemType would be after enlighten.
     * That is, we allocate sprites for the _potential_ of the machine,
     * but in calcSpriteWidth() we only choose from those currently
     * allowed by software conditions (eg. pre-enlighten, BootMenu display
     * options.
     */
    numpointers = NUMPOINTERS( IBase->GfxBase->BoardMemType );

    /* ZZZ: would have some trouble if we added new standard types */
    DSDM( printf( "\nsDMP: bm %lx, hotspot: %ld,%ld, xresn %ld, yresn %ld, ww %ld\n",
	bm, xoffset, yoffset, xresn, yresn, wordwidth ) );
    /* Create a new MousePointer structure... */
    if ( newmpointer = (struct MousePointer *)	NewObject( NULL, PointerClassName,
	POINTERA_BitMap, bm,
	POINTERA_XOffset, xoffset,
	POINTERA_YOffset, yoffset,
	POINTERA_WordWidth, wordwidth,
	POINTERA_XResolution, xresn,
	POINTERA_YResolution, yresn,
	TAG_DONE ) )
    {
	/* pointerclass can change wordwidth if it was -1
	 * (meaning an old-style pointer from SetPrefs)
	 */
	wordwidth = newmpointer->xmpt_WordWidth;

	DSDM( printf( "Obtained MousePointer %lx\n", newmpointer ) );
	/* use this lock to keep setMousePointer() from pointing
	 * at free memory
	 */
	LOCKIBASE();
	LOCKVIEW();

	/* If this is the default pointer, then IPrefs can free the
	 * BitMap it just passed to us (we don't keep the actual
	 * BitMap for the default pointer)
	 */
	free_result = bm;

	if ( type == NPT_BUSY )
	{
	    struct MousePointer *oldbusy;

	    DSBP( printf("Got new busy pointer %lx from bitmap %lx\n",
		newmpointer, bm ) );
	    DSBP( printf("Old one at %lx\n", IBase->DefaultBusyPointer ) );
	    /* Remember the old busy pointer for a check below */
	    oldbusy = IBase->DefaultBusyPointer;

	    /* Install the new busy pointer */
	    IBase->DefaultBusyPointer = newmpointer;

	    /* Nothing to do if this is the first time through... */
	    if ( oldbusy )
	    {
		/* Was the default busy pointer up? */
		if ( IBase->CustomMousePointer == oldbusy )
		{
		    DSBP( printf( "Calling sMP because default busy pointer in effect\n") );
		    setMousePointer();
		    /* We need to WaitTOF() if the default busy pointer was
		     * in use.
		     */
		    WaitTOF();
		}

		/* Normally, IPrefs would be freeing the BitMap of the
		 * previously installed busy pointer...
		 */
		free_result = oldbusy->mpt_BitMap;
		if ( free_result == IBase->BusyPointerBitMap )
		{
		    /* However,this was the original busy pointer that
		     * came from the ROM.  That one is our responsibility,
		     * to free, so let's do that, and tell IPrefs we
		     * succeeded but not to free anything.
		     */
		    DSBP( printf("Freeing busy-pointer bitmap\n"));
		    FreeVec( free_result );
		    IBase->BusyPointerBitMap = NULL;
		    /* Tell IPrefs we succeeded but not to free anything */
		    free_result = SUCCESS;
		}
		/* In any case, we have to free the MousePointer structure: */
		DisposeObject( oldbusy );
	    }
	    DSBP( printf("Returning bitmap %lx to be freed by IPrefs\n", free_result ) );
	}
	else /* type == NPT_DEFAULT */
	{
	    struct ExtSprite *oldExtSprite[POINTER_SIZES];
	    struct ExtSprite *newExtSprite[POINTER_SIZES];
	    /* Assume failure, which means we'll have to free
	     * the newExtSprite:
	     */
	    struct ExtSprite **freeExtSprite = newExtSprite;

	    /* Let's figure out the designed width in words (1,2,4),
	     * and also the designed size (0,1,2).
	     */
	    if ( (designsize = wordwidth - 1 ) == 3 )
	    {
		designsize = 2;
	    }
	    /* Attempt to allocate ExtSprite for the new preferences
	     * pointer imagery, for each size.  The way we designed it,
	     * each newExtSprite[] pointer points to something real
	     * or to NULL.
	     */
	    for ( size = 0; size < numpointers; size++ )
	    {
		if ( ! ( newExtSprite[ size ] = MakeSpriteData( newmpointer, 1<<size,
		    size-designsize ) ) )
		{
		    /* We failed */
		    free_result = NULL;
		}
		DSDM( printf( "newExtSprite[%ld] at %lx\n", size, newExtSprite[size] ) );
	    }

	    if ( free_result )
	    {
		/* Install the new pointer imagery as preferences,
		 * change the displayed pointer if necessary,
		 * and free the old ones.
		 */

		for ( size = 0; size < numpointers; size++ )
		{
		    oldExtSprite[size] = IBase->DefaultExtSprite[size];
		}

		/* This is safe inside LOCKIBASE(), since we only reference
		 * it inside LOCKIBASE().  Thus, no need to first copy away...
		 */
		DisposeObject( IBase->DefaultMousePointer );

		IBase->DefaultMousePointer = newmpointer;

		for ( size = 0; size < numpointers; size++ )
		{
		    IBase->DefaultExtSprite[size] = newExtSprite[size];
		}

		DSDM( printf( "DefaultExtSprites at %lx, %lx, %lx.  DefaultMP %lx\n",
		    IBase->DefaultExtSprite[0], IBase->DefaultExtSprite[1],
		    IBase->DefaultExtSprite[2], IBase->DefaultMousePointer ) );

		/* We update the display if the CustomMousePointer is
		 * NULL (meaning default is used)
		 */
		if ( !IBase->CustomMousePointer )
		{
		    DSDM( printf( "Calling sMP because default pointer in effect\n") );
		    setMousePointer();
		    /* We need to WaitTOF() if the default pointer was
		     * in use.
		     */
		    WaitTOF();
		}

		/* Since everything worked, we want to free the old ExtSprite
		 * instead:
		 */
		freeExtSprite = oldExtSprite;

	    }
	    else
	    {
		DisposeObject( newmpointer );
	    }
	    /* else we failed to allocate some of the new sprite data.
	     * Clean up any we did allocate, and return failure.
	     * ( freeExtSprite already points to newExtSprite ).
	     */

	    /* Free either the old or the new sprite data, depending
	     * on whether we succeeded or not.
	     */
	    for ( size = 0; size < numpointers; size++ )
	    {
		FreeSpriteData( freeExtSprite[ size ] );
	    }
	}
	UNLOCKVIEW();
	UNLOCKIBASE();
    }
    DSDM( printf( "sDMP returning bitmap to free %lx\n",free_result ) );
    return( free_result );
}


/*** ISetWindowPointer() ***
 *
 * This function is invoked when the application calls
 * SetPointer(), clearPointer or SetWindowPointer(), or
 * uses the WA_Pointer tag.
 *
 * A newMousePointer of NULL requests the default pointer.
 * newMousePointer equal to BUSYPOINTER is the standard
 * busy pointer.  (Note that this magic constant is not
 * public; applications get it through an explicit tag).
 * Other values are custom pointers obtained via
 * NewObject("pointerclass").
 *
 * If we got here during OpenWindow() through WA_Pointer,
 * then we come out OK since IBase->ActiveWindow doesn't
 * yet match win.
 *
 * There is no old stuff to free, since the system default pointer
 * is not changed by this, and custom pointers belong to the
 * application.
 */

ISetWindowPointer( win, newMousePointer )
struct Window *win;
struct MousePointer *newMousePointer;
{
    /* Optimistically assume we'll succeed: */
    LONG success = TRUE;
    struct IntuitionBase *IBase = fetchIBase();

    LOCKIBASE();
    LOCKVIEW();

    /* Stop any deferred pointer action, since a new pointer is
     * coming through.  Also, this is not a new-style pointer until
     * proven otherwise.
     */
    CLEARFLAG( win->MoreFlags, WMF_DEFERREDPOINTER );

    /* Go with custom pointer, or BUSYPOINTER, meaning the default busy
     * pointer, or NULL, meaning default pointer
     */
    XWINDOW(win)->MousePointer = newMousePointer;

    /* If the new pointer is an old-style one, then we must update
     * the pre-V39 window pointer fields.  This must be done under
     * lock, because Intuition accesses the MousePointer (which points
     * into the embedded pointer fields) under other tasks under lock.
     * We then set the MousePointer field of the XWINDOW() to point at
     * the embedded copy.
     *
     * (SetPointer() calls us with newMousePointer pointing
     * at an AllocVec()'d OldMousePointer.  We are expected to free
     * it here (see SetPointer() for relevant comments as to why).
     */
    if ( newMousePointer )
    {
	if ( ( newMousePointer != BUSYPOINTER ) &&
	( newMousePointer->mpt_Width != MPT_CUSTOMWIDTH ) )
	{
	    *(( struct OldMousePointer *)&win->Pointer ) =
		*(( struct OldMousePointer *)newMousePointer );

	    XWINDOW(win)->MousePointer = (struct MousePointer *)&win->Pointer;
	    FreeVec( newMousePointer );
	}
    }
    else
    {
	/* We were ClearPointer()d, so clear the fields */
	win->Pointer = NULL;
	win->PtrHeight = 0;
	win->PtrWidth = 0;
	win->XOffset = 0;
	win->YOffset = 0;
    }
    if ( win == IBase->ActiveWindow )
    {
	success = setMousePointer();
	if ( !success )
	{
	    /* Since we failed to give him the pointer
	     * he wanted, we mark the failure
	     * ZZZ: is this right?  Or should we try to keep
	     * the old pointer for a future attempt that
	     * might succeed.
	     */
	    XWINDOW(win)->MousePointer = NULL;
	}
    }

    /* The old window pointer is now released to the
     * application, so it may free it.
     * ZZZ: Should we do a WaitTOF(), or tell apps to
     * do that before freeing their guy?
     */

    UNLOCKVIEW();
    UNLOCKIBASE();

    return( success );
}


/*** setMousePointer() ***
 *
 * Establish the sprite display based on the active pointer.
 * Will call MakeSpriteData() if needed.
 *
 * This function might be called if:
 * - the user changed the preferences default pointer
 * - if the window's custom pointer changed
 * - if a new window became active
 * - if we have to re-instate the pointer because gfx had to kill it
 *
 */

LONG
setMousePointer()
{
    /* Let's assume success */
    LONG success = TRUE;
    struct ExtSprite *newCustomExtSprite = NULL;
    struct ExtSprite *newSprite = NULL;
    struct MousePointer *custompointer;

    struct IntuitionBase *IBase = fetchIBase();

    DSMP( printf( "\nsetMousePointer: enter\n" ) );
    LOCKIBASE();
    LOCKVIEW();

    /* Note the new active MousePointer (could be unchanged, but we
     * don't care yet).
     * ZZZ: Might be a good optimization, if we had a "force"
     * parameter to allow certain cases through anyways
     */
    IBase->CustomMousePointer = NULL;
    if ( IBase->ActiveWindow )
    {
	/* The busy-pointer is recognized by a special constant, which
	 * is only to be evaluated under IBase lock, since it's subject
	 * to change.
	 */
	if ( ( IBase->CustomMousePointer = XWINDOW(IBase->ActiveWindow)->MousePointer ) ==
	    BUSYPOINTER )
	{
	    IBase->CustomMousePointer = IBase->DefaultBusyPointer;
	}
    }
    DSMP( printf( "ActiveWindow %lx, IB->CustomMousePointer %lx\n",
	IBase->ActiveWindow, IBase->CustomMousePointer ) );

    /* Is active pointer a custom pointer of some kind?
     * Remember that CustomMousePointer is NULL when the default
     * pointer is up.
     */
    if ( custompointer = IBase->CustomMousePointer )
    {
	/* Assuming an old-style pointer, which has a width of 1 word,
	 * and a resolution of LORES in X and DEFAULT in Y.
	 * Also, if this is an old-style pointer from SetPrefs(), the
	 * caller pre-compensated for the old MoveSprite() off-by-one
	 * bug.  But since Intuition is using bug-free ExtSprites,
	 * we have to unshift to account for the x-error.
	 */
	LONG width = 1;
	LONG xres = POINTERXRESN_LORES;
	LONG yres = POINTERYRESN_DEFAULT;
	LONG xerror = -SPRITEERROR;
	/* Only new-style pointers have width/resolution information */
	if ( custompointer->mpt_Width == MPT_CUSTOMWIDTH )
	{
	    width = custompointer->xmpt_WordWidth;
	    xres = custompointer->xmpt_PointerXRes;
	    yres = custompointer->xmpt_PointerYRes;
	    xerror = 0;
	    DSW( printf("sMP: word-width of custom new-style pointer %ld\n",
		width ) );
	}
	/* Allocate a suitable ExtSprite representation of the pointer,
	 * based on what we figure to be the correct-sized pointer (16/32/64)
	 * we need to put up.
	 */
	DSW( printf("sMP: from calcSpriteWidth(%ld) = %ld\n", width, calcSpriteWidth(width) ) );
	if ( newCustomExtSprite = MakeSpriteData( custompointer,
	    calcSpriteWidth( width ), 0 ) )
	{
	    DSMP( printf( "got ExtSprite at %lx for CustomMousePointer\n",
		newCustomExtSprite ) );
	    /* Let's install the chosen sprite */
	    newSprite = newCustomExtSprite;
	    IBase->PointerXRes = xres;
	    IBase->PointerYRes = yres;
	    IBase->AXOffset = custompointer->mpt_XOffset + xerror;
	    IBase->AYOffset = custompointer->mpt_YOffset;
	}
	else
	{
	    /* We failed to give the guy what he wanted because MakeSpriteData()
	     * failed.  Let's note the failure, but proceed by giving him the
	     * system default arrow instead.
	     */
	    DSMP( printf( "Failed to get ExtSprite for CustomMousePointer\n") );
	    success = FALSE;
	    IBase->CustomMousePointer = NULL;
	}
    }

    if ( !newCustomExtSprite )
    {
	LONG width;

	/* Figure the sprite width, hoping to use our default word-width. */
	DSW( printf("sMP: Default sprite, calcSpriteWidth(%ld) = %ld\n",
	    IBase->DefaultMousePointer->xmpt_WordWidth,
	    calcSpriteWidth( IBase->DefaultMousePointer->xmpt_WordWidth ) ) );
	width = calcSpriteWidth( IBase->DefaultMousePointer->xmpt_WordWidth );
	/* GfxBase SpriteWidth is 1,2,4 words.  We want to convert
	 * that to 0,1,2 respectively.  This is the cheapest way
	 */
	if ( --width == 3 )
	{
	    width = 2;
	}
	/* If we get here, it's either because we failed to allocate
	 * a suitable size of the custom pointer, or else the active
	 * pointer is supposed to be the default one.
	 */

	/* We always keep an ExtSprite representation of the default pointer
	 * in each possible size, so we can't fail.  Let's determine what
	 * size to use, and use it.
	 *
	 * Note that old Pointers coming through Preferences have the
	 * exact correct hotspot position, so no decompensation is
	 * required.
	 */
	DSMP( printf( "Using DefaultExtSprite[%ld] at %lx\n",
	    width, IBase->DefaultExtSprite[width] ) );
	newSprite = IBase->DefaultExtSprite[ width ];
	IBase->PointerXRes = IBase->DefaultMousePointer->xmpt_PointerXRes;
	IBase->PointerYRes = IBase->DefaultMousePointer->xmpt_PointerYRes;
	IBase->AXOffset = IBase->DefaultMousePointer->mpt_XOffset;
	IBase->AYOffset = IBase->DefaultMousePointer->mpt_YOffset;
    }

    UNLOCKVIEW();

    /* Change and move the sprite to the right place */
    updateMousePointer( newSprite );

    if ( IBase->CustomExtSprite )
    {
#if WAITTOF_OPTIMIZE
    /* Up until 3.00, we used to do a WaitTOF() here, then call
     * FreeSpriteData() on IBase->CustomExtSprite.  However, we take
     * an optimization that depends on GfxBase maintaining a VBlank
     * counter.  the idea is that in place of WaitTOF()/FreeXXX(), we send
     * ourselves a token (reminder) to FreeXXX after the VBlank counter has
     * had a chance to change, thus taking a WaitTOF() clean out of
     * setMousePointer().
     */
    sendISMNoQuick( itFREESPRITEDATA, IBase->CustomExtSprite, 0,
	IBase->GfxBase->VBCounter );
#else
	/* If there was a custom ExtSprite in use, free it after
	 * waiting for it to no longer be in use:
	 */
	WaitTOF();
	FreeSpriteData( IBase->CustomExtSprite );
#endif
    }
    /* Remember the CustomExtSprite (if any) we're currently using: */
    IBase->CustomExtSprite = newCustomExtSprite;
    UNLOCKIBASE();

    return( success );
}



/*** updateMousePointer() ***
 *
 * Figure out where to move the sprite, and in whose ViewPort. 
 * Then actually call MoveSprite().  If the newSprite parameter is
 * non-NULL, then we also do ChangeExtSprite() to the newSprite.
 *
 * NOTE on the interaction between updateMousePointer() and RethinkDisplay():
 *
 * updateMousePointer() may discover that to change the sprite pixel-speed
 * requires calling RethinkDisplay().  This will rebuild the affected
 * viewports and then the copper lists.  Conversely, certain changes
 * (eg. promotion on/off) may require that ChangeExtSprite() be called
 * (in particular, to change the setting of sprite-scan-doubling).
 * Thus, routines in view.c call updateMousePointer( IBase->SimpleSprite ).
 * We pass IBase->SimpleSprite back to uMP() so that it rethinks the
 * sprite.  Note that VideoControl() (hence setSpriteSpeed()) will never
 * return rethink=TRUE if the sprite resolution is unchanged.
 *
 * ZZZ: Before I rewrote all this, the matching routine used to
 * live in mouse.c.  Jimm had a weird locking order where he did:
 *
 *	LOCKIBASE()
 *	LOCKVIEW()
 *	UNLOCKIBASE()	<-- Note VIEWLOCK still held!
 *	ChangeSprite()
 *	MoveSprite()
 *	UNLOCKVIEW()
 *
 * Things have gotten harder, so I now have to hold the IBase lock
 * across the calls to ChangeSprite() and MoveSprite().
 *
 */

updateMousePointer( newSprite )
struct ExtSprite *newSprite;
{
    struct Screen *s;
    struct Screen *mouse_screen = NULL;
    struct Window *w;
    struct Screen *prevscreen = NULL;

    struct IntuitionBase *IBase = fetchIBase();

    DUPM( printf( "rm: in %ld,%ld\n", IBase->LongMouse.LX,
	IBase->LongMouse.LY ) );
    limitLongPoint( &IBase->LongMouse, &IBase->MouseLimits );
    DUPM( printf( "rm: after limit %ld,%ld\n", IBase->LongMouse.LX,
	IBase->LongMouse.LY ) );

    LOCKIBASE();

    /* maintain the old MouseX/Y	*/
    /* too bad, eh?	*/
    IBase->MouseY = IBase->LongMouse.LY / IBase->MouseScaleY;
    IBase->MouseX = IBase->LongMouse.LX / IBase->MouseScaleX;

    ICOLOR( CKCOLOR8 );

    for ( s = IBase->FirstScreen; s; s = s->NextScreen )
    {
	screenMouse( s );

	/* keep track of first visible screen higher than mouse pos */

	/* Only consider visible screens */
	if ( !TESTFLAG(s->ViewPort.Modes, VP_HIDE ) )
	{
	    if ( !prevscreen )
	    {
		/* In the event that the mouse is below the first visible
		 * screen, that's the screen that the mouse must be in
		 */
		prevscreen = s;
	    }

	    if ( s->MouseY >= 0 )
	    {
		/* Peter 4-Dec-90:  The mouse_screen will be the screen that
		 * contains the mouse hotspot.  In the event that the hotspot
		 * is located in the interscreen gap, we use the previous
		 * screen.
		 */
		if ( !mouse_screen )
		{
		    DUPM2( printf("s->MouseY: %ld, s->ViewPort.DHeight %ld\n",
			s->MouseY, s->ViewPort.DHeight) );
		    DUPM2( printf("IBase->AYOffset * XSC(s)->SpriteFactor.Y / SPRITEMULT_Y %ld\n",
			( IBase->AYOffset * XSC(s)->SpriteFactor.Y ) / SPRITEMULT_Y ) );
		    if ( ( s->MouseY - s->ViewPort.DHeight ) >
			    -( ( IBase->AYOffset * XSC(s)->SpriteFactor.Y ) / SPRITEMULT_Y ) )
		    {
			/* Hot spot is below this screen and so is the pointer's
			 * top line, so we must use the previous visible screen
			 */
			DUPM2( printf("Mouse in previous screen %lx\n", prevscreen) );
			mouse_screen = prevscreen;
		    }
		    else
		    {
			/* Hot spot is in this screen, or if it's below, then
			 * at least the top line of pointer-data is in.
			 */
			DUPM2( printf("Mouse in this screen %lx\n", s) );
			mouse_screen = s;
		    }
		}
	    }
	    prevscreen = s;
	}

	for ( w = s->FirstWindow; w; w = w->NextWindow ) windowMouse( w );
    }

    if ( !mouse_screen )
    {
	/* When dragging a rear screen behind a front screen, the rear
	 * screen's ViewPort can end up hidden even when the mouse's
	 * y is above the front screen.  In that case, we really want
	 * the mouse's screen to be the topmost one we saw.
	 */
	mouse_screen = prevscreen;
    }
    ICOLOR( CKCOLOR9 );

    /* figure out where to put the pointer sprite	*/

    LOCKVIEW();	
    /* jimm: ZZZ: trying to synchronize changes to
     * view mode in MakeScreen with copper list changes 
     */

    DUPM( printf("mouse_screen: %lx, newSprite: %lx\n", mouse_screen, newSprite ) );
    ICOLOR( CKCOLOR10 );

    /* If mouse_screen is NULL, we must still be sure to call ChangeExtSprite()
     * on the NULL ViewPort, or else the initial sprite isn't set up right
     * in the case of a pre-DOS game which opens a screen but no window.
     */
    {
	struct Point  spritepos;
	struct ViewPort *mousevp = NULL;
	ULONG specialspritekill = IBase->SpecialSpriteKill;

	if ( mouse_screen )
	{
	    mousevp = &mouse_screen->ViewPort;
	}

	if ( newSprite )
	{
	    struct Screen *sc;

	    DUPMC( printf( "About to setSpriteHeight\n") );

	    setSpriteHeight();

	    /* Some callers call us with newSprite == IBase->SimpleSprite,
	     * which is just to force us to rethink things.  If newSprite
	     * is some other value, then blank the sucker around size/shape
	     * changes.
	     */
	    if ( newSprite != (struct ExtSprite *)IBase->SimpleSprite )
	    {
		if ( !specialspritekill )
		{
		    ChangeExtSpriteA( mousevp, IBase->SimpleSprite, IBase->BlankSprite,
			NULL );
		}
		IBase->SimpleSprite = IBase->BlankSprite;
	    }
	    /* Sprite speed needs to be set for each ViewPort. */
	    for ( sc = IBase->FirstScreen; sc; sc = sc->NextScreen )
	    {
		setSpriteSpeed( sc );
	    }
	}

	/* We need to figure out the sprite's new position, regardless
	 * of whether we have a newSprite or not.  However, the SpriteFactors
	 * depend on the results of setSpriteSpeed(), so we have to evaluate
	 * the spritepos after setSpriteSpeed().  We need the sprite position
	 * before we change to the new sprite.  Thus, we have to leave the
	 * conditional temporarily.
	 *
	 * SpriteFactor.X is four times bigger than reality, since
	 * V39 needs to be able to scale up and down, too.
	 *
	 * SpriteFactor.Y is two times bigger than reality, for the
	 * same reason.
	 */
	spritepos.X = spritepos.Y = 0;
	if ( mouse_screen )
	{
	    spritepos.X = ( ( IBase->AXOffset * XSC(mouse_screen)->SpriteFactor.X ) /
		SPRITEMULT_X ) + mouse_screen->MouseX;
	    spritepos.Y = ( ( IBase->AYOffset * XSC(mouse_screen)->SpriteFactor.Y ) /
		SPRITEMULT_Y ) + mouse_screen->MouseY;
	}

	if ( newSprite )
	{
	    newSprite->es_SimpleSprite.x = spritepos.X;
	    newSprite->es_SimpleSprite.y = spritepos.Y;

	    DUPMC( printf( "ChangeExtSpriteA( from old %lx[%lx] to new %lx )\n",
		IBase->SimpleSprite, IBase->SimpleSprite->num, newSprite ) );
	    DUPMC( printf( "  posctldata %lx, height %lx, x/y %lx/%lx, num %lx, ww %lx ydouble %lx\n",
		newSprite->es_SimpleSprite.posctldata,
		newSprite->es_SimpleSprite.height,
		newSprite->es_SimpleSprite.x,
		newSprite->es_SimpleSprite.y,
		newSprite->es_SimpleSprite.num,
		newSprite->es_wordwidth,
		TESTFLAG( IBase->Flags, POINTER_YDOUBLE ) ) );
	    /* ZZZ: We could "legitimately" fail here if some nasty app
	     * did a FreeSprite(0), because that would clear the SoftSprite
	     * bit.  So we should try to handle failure by ? staying out
	     * of the sprite game?
	     */
	    if ( !specialspritekill )
	    {
		ChangeExtSprite( mousevp, IBase->SimpleSprite, newSprite,
		    GSTAG_SCANDOUBLED, TESTFLAG( IBase->Flags, POINTER_YDOUBLE ),
		    TAG_DONE );
	    }

	    DUPMC( printf( "Done ChangeExtSpriteA\n") );
	    IBase->TetrisSprite = IBase->SimpleSprite = newSprite;
	}

	if ( mouse_screen )
	{
	    DDF( if ( IBase->DebugFlag & IDF_SPRITE ) {
		printf("movesprite to: %ld, %ld\n", spritepos.X,spritepos.Y );
		printf("mouse screen: %lx\n", mouse_screen ); });
	    DUPM( printf("rM: spritepos.X = %ld, IBase->LongMouse.X: %ld\n",
		spritepos.X, IBase->LongMouse.LX ) );

	    D( printf( "MoveSprite of IBase->SS %lx\n", IBase->SimpleSprite ) );
	    if ( !specialspritekill )
	    {
		/* Because of specialspritekill, we run the risk that
		 * we get to MoveSprite() a sprite that was never installed
		 * with ChangeExtSprite().  Forcing its ss_num to zero
		 * cures any resulting issues.
		 */
		IBase->SimpleSprite->num = 0;
		MoveSprite( mousevp, IBase->SimpleSprite, spritepos.X, spritepos.Y );
	    }
	}
    }

    UNLOCKVIEW();
    UNLOCKIBASE();
}


/*** calcSpriteWidth() ***
 *
 * Based on the desired sprite width, and the state of GfxBase,
 * figure out what the sprite width should be.
 *
 * Returns 1, 2, or 4.
 */

static
calcSpriteWidth( width )
LONG width;
{
    /* Forbid() to get an atomic peek at GfxBase.  Now, if the sprites
     * in use (SpriteReserved) are not just the soft-sprites (i.e. ours),
     * then we have to use the width currently in effect.
     * If not, we can use the correct one.
     */
    struct GfxBase *GfxBase = fetchIBase()->GfxBase;

    /* Don't let the sprite width exceed what is currently allowed,
     * based on hardware restrictions and software restrictions
     * (eg. pre-enlighten or BootMenu display options).
     */
    width = imin( width, 1 << ( NUMPOINTERS( GfxBase->MemType ) - 1 ) );
    Forbid();
    if ( GfxBase->SoftSprites != GfxBase->SpriteReserved )
    {
	/* This will be 1, 2, or 4 */
	width = GfxBase->SpriteWidth;
    }
    Permit();
    return( width );
}


/*** notePointerKill() ***
 *
 * When graphics kills our pointer, it calls us with our data-pointer
 * (IntuitionBase) in A1.  We have a little assembler stub that
 * calls this routine.  The routine is in C since IBase is only
 * defined in the C-header, not the asm-header.
 *
 * We're called inside Forbid().
 *
 */

notePointerKill( passedIBase )
struct IntuitionBase *passedIBase;
{
    DKILL( printf("Pointer killed\n") );
    passedIBase->PointerKilled = 1;
}

/*** reinstatePointer() ***
 *
 * If our pointer becomes incompatible with other sprites on the
 * screen, graphics.library will blank our pointer and call us.
 * The notePointerKill() function is a very simple one that
 * notes the fact down.  Later, when we wake up, we get the
 * chance to come in and restore things via this function.
 *
 */

reinstatePointer()
{
    struct IntuitionBase *IBase = fetchIBase();

    if ( IBase->PointerKilled )
    {
	DKILL( printf("Reinstating pointer\n") );
	IBase->PointerKilled = 0;
	setMousePointer();
    }
}

/*** setSpriteSpeed ***
 *
 * For a given screen, and a desired resolution (one of the POINTERXRESN_
 * flags from intuition.h), calculate and set the sprite resolution.
 *
 * The result depends on:
 * - Pointer POINTERXRESN_ flags
 * - Ability to change sprite res on this screen
 * - Scan rate of this screen
 * - IBase->Flags POINTER_XDOUBLE (set/cleared by setSpriteHeight()
 *   in order to preserve aspect)
 *
 * Also sets up the screen's SpriteFactor.X and SpriteFactor.Y values!
 *
 * This routine should be called for each screen inside rethinkVPorts(),
 * and inside setMousePointer().
 *
 * Officially, we're supposed to MakeScreen/RethinkDisplay if rethink
 * comes back non-zero from VideoControl().  However, we risk infinite
 * recursion if we do, since RethinkDisplay() can call updateMousePointer().
 * Chris Green assures me that there should be no problem, and in all
 * normal cases, rethink will come back zero.  Guess who's the abnormal
 * case?  DPaint 4.1.  They steal their screen's ColorMap and stick it
 * into their own custom view during animation.  So it's a good idea
 * to ignore rethink.
 */

#define PIXEL_XRESOLUTION_LORES	44	/* 44 ticks per lores pixel */
#define PIXEL_XRESOLUTION_HIRES	22	/* 22 ticks per hires pixel */

UWORD SpriteFactorTable[] =
{
    140 * SPRITEMULT_X,	/* res = 1 is 140 ns */
     70 * SPRITEMULT_X,	/* res = 2 is 70 ns */
     35 * SPRITEMULT_X,	/* res = 3 is 35 ns */
};

setSpriteSpeed( sc )
struct Screen *sc;
{
    struct IntuitionBase *IBase = fetchIBase();
    LONG desired_res = IBase->PointerXRes;
    LONG res = desired_res;
    LONG rethink = 0;
    LONG changable;

    /* The ViewPort might not support changable sprite resolutions */
    changable = TESTFLAG( XSC(sc)->DProperties, DIPF_IS_SPRITES_CHNG_RES );

    /* If any old-style sprites (non-ExtSprites) are in use, we must not
     * change the VTAG_DEFSPRITERESN.  Users of ExtSprites can rely on
     * Intuition's choice, or use VTAG_SPRITERESN to control the sprite
     * resolution (this VTAG overrides the VTAG_DEFSPRITERESN we set).
     */
    Forbid();
    if ( IBase->GfxBase->ExtSprites != IBase->GfxBase->SpriteReserved )
    {
	changable = 0;
    }
    Permit();

    if ( !changable )
    {
	/* No can change sprite resolution! */
	res = SPRITERESN_ECS;
    }
    /* We've chosen POINTERXRESN_DEFAULT, POINTERXRESN_140NS, POINTERXRESN_70NS,
     * and POINTERXRESN_35NS to match numerically the gfx SPRITERESN_ defines,
     * so we have no work for those choices.
     */
    else if ( desired_res >= POINTERXRESN_SCREENRES )
    {
	LONG pointer_resolution;

	DSS( printf( "sSS: Screen %lx, pixel speed %ld, resolution.x %ld\n",
	    sc, XSC(sc)->PixelSpeed, XSC(sc)->ScaleFactor.X ) );
	/* Default here is for POINTERXRESN_SCREENRES, we want the
	 * pointer's resolution to match the mode's:
	 */
	pointer_resolution = XSC(sc)->ScaleFactor.X;
	if ( desired_res == POINTERXRESN_LORES )
	{
	    /* Want the pointer_resolution to match a regular
	     * lores pixel
	     */
	    pointer_resolution = PIXEL_XRESOLUTION_LORES;
	}
	else if ( desired_res == POINTERXRESN_HIRES )
	{
	    /* Want the pointer_resolution to match a regular
	     * hires pixel
	     */
	    pointer_resolution = PIXEL_XRESOLUTION_HIRES;
	}
	DSS( printf( "sSS: desired_res %ld, pointer_resolution %ld\n",
	    desired_res, pointer_resolution ) );

	{
	    /* Calculate what the target_pixelspeed should be.  The
	     * valid values are 35, 70, and 140 ns.  But target_pixelspeed
	     * may not be exactly one of these values, so find the nearest.
	     */
	    LONG target_pixelspeed;
	    LONG delta;
	    LONG delta2;

	    target_pixelspeed = ( XSC(sc)->PixelSpeed * pointer_resolution ) /
		XSC(sc)->ScaleFactor.X;
	    /* Because of scaling restrictions in the y-direction,
	     * we may have been forced to double the scaling in
	     * the x-direction to preserve aspect...
	     */
	    if ( TESTFLAG( IBase->Flags, POINTER_XDOUBLE ) )
	    {
		DSS( printf( "sSS: pointer is to be xdoubled\n"));
		target_pixelspeed *= 2;
	    }

	    DSS( printf( "sSS: Target target_pixelspeed %ld\n",
		target_pixelspeed ) );

	    res = SPRITERESN_35NS;
	    delta = iabs( target_pixelspeed - 35 );
	    delta2 = iabs( target_pixelspeed - 70 );
	    if ( delta2 < delta )
	    {
		delta = delta2;
		res = SPRITERESN_70NS;
	    }
	    delta2 = iabs( target_pixelspeed - 140 );
	    if ( delta2 < delta )
	    {
		res = SPRITERESN_140NS;
	    }
	}
    }
    DSS( printf( "sSS: Setting screen %lx sprite resn to %ld\n", sc, res ) );
    /* We set the default sprite resolution for the ViewPort,
     * which allows applications to override:
     */
    VideoControlTags( XSC(sc)->ColorMap,
	VTAG_DEFSPRITERESN_SET, res,
	VTAG_IMMEDIATE, &rethink,
	TAG_DONE );

    /* ZZZ: This code is correct for the default sprite resolution only.
     * If someone overrides, then this is not right.  I need to enquire
     * what the real resolution is.
     */
    if ( res == SPRITERESN_ECS )
    {
	XSC(sc)->SpriteFactor.X = ( XSC(sc)->SpriteRes.X * SPRITEMULT_X ) /
	    XSC(sc)->ScaleFactor.X;
    }
    else
    {
	XSC(sc)->SpriteFactor.X = SpriteFactorTable[ res-1 ] / XSC(sc)->PixelSpeed;
    }
    XSC(sc)->SpriteFactor.Y = ( XSC(sc)->SpriteRes.Y * SPRITEMULT_Y ) /
	XSC(sc)->ScaleFactor.Y;
    if ( TESTFLAG( IBase->Flags, POINTER_YDOUBLE ) )
    {
	XSC(sc)->SpriteFactor.Y <<= 1;
    }
}

/* Routine to figure out whether sprite scan-doubling should be
 * turned on for this sprite.
 *
 * Depends on:
 * - Pointer POINTERYRESN_ flags
 * - Ability to scan-double on this display
 * - Vertical resolution of screen and pointer
 *
 * Sets POINTER_YDOUBLE flag if it would be desirable and possible
 * to scan-double the pointer.
 * Sets POINTER_XDOUBLE flag if it would be desirable to preserve aspect
 * by horizontal doubling on the pointer.
 */

/* These are the ticks-per-pixel of a pointer designed to be about
 * 200 lines-per-screen and 400 lines-per-screen.  I use the
 * PAL ticks because the comparison for doubling in X assumes that
 * actualres >= targetres, which wouldn't be true on PAL systems
 * if we used the NTSC numbers.  So we use the smaller numbers
 * here.
 */
#define PIXEL_YRESOLUTION_200	44
#define PIXEL_YRESOLUTION_400	22

/* All aspect-preserving choices are odd-numbered: */
#define POINTERYRESNFLAG_ASPECT	1

static
setSpriteHeight()
{
    LONG actualres, targetres;
    LONG aspect;
    LONG delta;

    struct IntuitionBase *IBase = fetchIBase();
    struct Screen *ascreen = IBase->ActiveScreen;

    assertLock( "setSpriteHeight", IBASELOCK );

    actualres = PIXEL_YRESOLUTION_200;
    if ( ascreen )
    {
	actualres = XSC(ascreen)->SpriteRes.Y;
    }

    CLEARFLAG( IBase->Flags, POINTER_XDOUBLE|POINTER_YDOUBLE );

    DSH( printf("sSH: PointerYRes is %ld\n", IBase->PointerYRes ));
    switch ( IBase->PointerYRes )
    {
    case POINTERYRESN_DEFAULT:
	targetres = PIXEL_YRESOLUTION_200;
	break;

    case POINTERYRESN_HIGH:
    case POINTERYRESN_HIGHASPECT:
	targetres = PIXEL_YRESOLUTION_400;
	break;

    case POINTERYRESN_SCREENRES:
    case POINTERYRESN_SCREENRESASPECT:
	targetres = PIXEL_YRESOLUTION_200;
	if ( ascreen )
	{
	    targetres = XSC(ascreen)->ScaleFactor.Y;
	}
	break;
    }

    aspect = TESTFLAG( IBase->PointerYRes, POINTERYRESNFLAG_ASPECT );

    DSH( printf("sSH: actualres %ld, targetres %ld, aspect %ld\n", actualres,
	targetres, aspect ) );
    delta = iabs( targetres - actualres );

    if ( ( IBase->ActiveMonitorSpec ) && 
	TESTFLAG( IBase->ActiveMonitorSpec->ms_Flags, MSF_DOUBLE_SPRITES ) &&
	( delta > iabs( targetres - 2*actualres ) ) )
    {
	SETFLAG( IBase->Flags, POINTER_YDOUBLE );
	actualres *= 2;
	DSH( printf("sSH: vertical doubling on\n"));
    }

    /* If we can improve the pointer aspect ratio by doubling
     * it in the x-direction (via hardware), then do it,
     * and note the fact:
     */
    if ( ( actualres >= 2*targetres ) && ( aspect ) )
    {
	SETFLAG( IBase->Flags, POINTER_XDOUBLE );
	DSH( printf(" sSH: To preserve aspect, horizontal doubling on\n"));
    }
}

/*** MakeSpriteData() ***
 *
 * Take a MousePointer structure, a word-width, and a scaling factor,
 * and return a pointer to an ExtSprite allocated from that.
 * To free, just call plain graphics FreeSpriteData().
 */

static struct ExtSprite *
MakeSpriteData( mpointer, word_width, scale )
struct MousePointer *mpointer;
ULONG word_width;
LONG scale;
{
    ULONG olddata_tag = TAG_IGNORE;
    ULONG height_tag = TAG_IGNORE;

    struct ExtSprite *AllocSpriteData();

    DAS( printf("MakeSpriteData: mpointer at %lx\n", mpointer ) );

    /* We never scale a sprite bigger, since we'd rather just leave
     * blank pixels.
     */
    if ( scale >= 0 )
    {
	scale = 0;
    }

    DAS( printf("Assuming New Sprite, bitmap at %lx\n", mpointer->mpt_BitMap ) );

    /* If mpt_Width is not MPT_CUSTOMWIDTH, then we have old-style
     * sprite data.
     */
    if ( mpointer->mpt_Width != MPT_CUSTOMWIDTH )
    {
	DAS( printf("Old SpriteData at %lx, height %ld\n",
	    mpointer->mpt_BitMap, mpointer->mpt_Height ) );
	olddata_tag = SPRITEA_OldDataFormat;
	height_tag = SPRITEA_OutputHeight;
    }
    return( AllocSpriteData( mpointer->mpt_BitMap,
	SPRITEA_Width, 16*word_width,
	SPRITEA_XReplication, scale,
	height_tag, mpointer->mpt_Height,
	olddata_tag, TRUE,
	TAG_DONE ) );
}

/*** intuition.library/SetWindowPointerA ***/

/* New version of SetPointer().  Recognizes the following tags:
 * WA_Pointer - custom pointer from pointerclass
 * WA_BusyPointer,TRUE - standard busy pointer
 * WA_PointerDelay,TRUE - delay a bit before posting (busy) pointer
 */
LIB_SetWindowPointerA( win, taglist )
struct Window *win;
struct TagItem *taglist;
{
    struct MousePointer *mpointer;

    /* To protect the DeferredPointer fields */
    LOCKIBASE();

    /* NULL means the default pointer, but override with his
     * selection
     */
    mpointer = (struct MousePointer *)GetUserTagData0( WA_Pointer, taglist );
    /* If caller asked for the busy pointer, we pull that out
     * of IBase:
     */
    if ( GetUserTagData0( WA_BusyPointer, taglist ) )
    {
	/* We use a special numeric value to denote the
	 * busy-pointer, because we want to grab IBase->BusyPointer
	 * in a state-synchronous way (inside ISetWindowPointer).
	 */
	mpointer = BUSYPOINTER;
    }

    /* We support pointer-delay, which is really only useful for
     * the busy pointer.  The idea here is that you can do a deferred
     * NewSetPointer() call, which will take effect in a few timer ticks
     * if you haven't restored some other pointer first.  This allows
     * you to fearlessly set the busy pointer even if there's a chance
     * you'll only be busy for an instant.
     */

    if ( GetUserTagData0( WA_PointerDelay, taglist ) )
    {
	XWINDOW(win)->DeferredPointer = mpointer;
	XWINDOW(win)->DeferredCounter = DEFERREDPOINTERCOUNT;
	SETFLAG( win->MoreFlags, WMF_DEFERREDPOINTER );
	UNLOCKIBASE();
    }
    else
    {
	UNLOCKIBASE();
	/* Change the pointer. */
	doISM( itSETPOINTER, win, mpointer );
    }
}


/*** intuition.library/SetPointer ***/
/*** intuition.library/ClearPointer ***/

/* In intuitionface.asm, ClearPointer(win) just calls
 * SetPointer( win, NULL, 0, 0, 0, 0 )
 */
SetPointer(win, pointer, height, width, xoffset, yoffset)
struct Window *win;
USHORT *pointer;
int height, width, xoffset, yoffset;
{
    struct OldMousePointer *mpointer = NULL;

    DSP( printf("SetPointer window %lx, pointer %lx, height %ld\n", win, pointer, height));

    /* SetPointer() used to have very simple locking.  Some versions
     * of ProWrite (eg. 3.2.3) and other New Horizons software end up
     * calling SetPointer() inside Begin/EndUpdate().  With the new
     * locking we added around SetPointer(), there is now a real chance
     * of deadlock.  To avoid this, we short-circuit SetPointer()
     * if the LAYERUPDATING flag is set.  Note that ProWrite uses
     * Begin/EndUpdate() directly, so we can't check the WINDOWREFRESH
     * flag.
     */
    if ( !TESTFLAG( win->WLayer->Flags, LAYERUPDATING ) )
    {
	/* We want to pass pointer=NULL for itSETPOINTER if the pointer
	 * parameter is NULL, such as it would be from ClearPointer().
	 * Otherwise, we pass an allocated OldMousePointer.  ISetWindowPointer()
	 * knows to copy old-style pointers into the window field and
	 * free the OldMousePointer.
	 *
	 * The reason we don't make mpointer a local variable is because some
	 * mouse-blankers (eg. CygnusEd, DMouse?) call SetPointer() from an
	 * input handler.  To fix them, we made doISM() convert to sendISM(),
	 * so the local variable wouldn't persist long enough.
	 */

	if ( pointer )
	{
	    /* If the allocation fails, we'd better do an effective
	     * ClearPointer(), which will result anyways since mpointer is NULL.
	     */
	    mpointer = (struct OldMousePointer *)
		AllocVec( sizeof( struct OldMousePointer ), MEMF_PUBLIC );
	}

	/* Make an old MousePointer structure out of the passed-in data.
	 * ISetWindowPointer() will copy that into 
	 */
	if ( mpointer )
	{
	    mpointer->mpt_BitMap = (struct BitMap *)pointer;
	    mpointer->mpt_Height = height;
	    mpointer->mpt_Width = width;
	    mpointer->mpt_XOffset = xoffset;
	    mpointer->mpt_YOffset = yoffset;
	}

	/* Change the pointer */
	doISM( itSETPOINTER, win, mpointer );

	DSP( printf("done.\n"));
    }
}


/**********************************************
 *                                            *
 * "pointerclass" - boopsi sprite image class *
 *                                            *
 **********************************************/

ULONG pointerPackTable[] =
{
    PACK_STARTTABLE( POINTERA_Dummy ),
    PACK_ENTRY( POINTERA_Dummy, POINTERA_BitMap, MousePointer, mpt_BitMap, PKCTRL_ULONG ),
    PACK_ENTRY( POINTERA_Dummy, POINTERA_XOffset, MousePointer, mpt_XOffset, PKCTRL_BYTE ),
    PACK_ENTRY( POINTERA_Dummy, POINTERA_YOffset, MousePointer, mpt_YOffset, PKCTRL_BYTE ),
    PACK_ENTRY( POINTERA_Dummy, POINTERA_WordWidth, MousePointer, xmpt_WordWidth, PKCTRL_WORD ),
    PACK_ENTRY( POINTERA_Dummy, POINTERA_XResolution, MousePointer, xmpt_PointerXRes, PKCTRL_WORD ),
    PACK_ENTRY( POINTERA_Dummy, POINTERA_YResolution, MousePointer, xmpt_PointerYRes, PKCTRL_WORD ),
    PACK_ENDTABLE,    
};

/* Since setPointerClassAttrs() is nothing more than a call to PackStructureTags(),
 * it's smaller to take it in-line.  It's clearer to leave it as a macro.
 */
#define setPointerClassAttrs( cl, o, msg ) \
	PackStructureTags( INST_DATA( cl, o ), pointerPackTable, \
	((struct opSet *)msg)->ops_AttrList )


/*** dispatchPointerClass ***
 *
 * Boopsi dispatcher for pointerclass.  All MousePointers that go
 * through pointerclass are full-sized, hence it's legal to access
 * the xmpt_ fields.
 *
 */

#define MPT(o) ((struct MousePointer *)(o))

static ULONG
dispatchPointerClass( cl, o, msg )
Class *cl;
Object *o;
Msg msg;
{
    Object *newobj;

    DPC( printf( "dispatchPointerClass: class %lx, object %lx, msg %lx\n", cl,o,msg ) );

    switch ( msg->MethodID )
    {
    case OM_NEW:
	if ( newobj = (Object *) SSM( cl, o, msg ) )
	{
	    /* Reasonable defaults:
	     * Height of 16 is required for SetPrefs() of the old
	     *   pointer to work.
	     * Width of MPT_CUSTOMWIDTH indicates a full boopsi pointer
	     * WordWidth of 1 is a reasonable default.
	     */
	    MPT(newobj)->mpt_Height = 16;
	    MPT(newobj)->mpt_Width = MPT_CUSTOMWIDTH;
	    MPT(newobj)->xmpt_WordWidth = 1;

	    setPointerClassAttrs( cl, newobj, msg );
	    /* WordWidth of -1 means pretend we're an old-style pointer.
	     * This is used to support the initial SetPrefs().  Note
	     * that the pointer data needs to be put into chip RAM!
	     */
	    if ( MPT(newobj)->xmpt_WordWidth == -1 )
	    {
		APTR pointercopy;

		if ( pointercopy = (APTR) AllocVec( (16+2)*4, MEMF_CHIP|MEMF_PUBLIC ) )
		{
		    CopyMem( MPT(newobj)->mpt_BitMap, pointercopy, (16+2)*4 );
		    MPT(newobj)->mpt_BitMap = pointercopy;
		    MPT(newobj)->mpt_Width = 16;
		    MPT(newobj)->xmpt_WordWidth = 1;
		}
		else
		{
		    DisposeObject( newobj );
		    newobj = NULL;
		}
	    }		
	}
	return ( (ULONG) newobj );

    case OM_SET:
	SSM( cl, o, msg );
	setPointerClassAttrs( cl, o, msg );
	return( 1 );

    case OM_DISPOSE:
	/* If this pointer has a width of 16, that denotes an old-style
	 * pointer that came from SetPrefs().  We have to free the
	 * chip RAM copy.
	 */
	if ( MPT(o)->mpt_Width == 16 )
	{
	    FreeVec( MPT(o)->mpt_BitMap );
	}
	/* FALL THROUGH! */
    default:
	return( SSM( cl, o, msg ) );
    }
}

/*** initPointerClass() ***
 *
 * Set up pointerclass.
 *
 */

Class	*
initPointerClass()
{
    ULONG dispatchPointerClass();
    Class	*makePublicClass();
    extern UBYTE	*PointerClassName;
    extern UBYTE	*RootClassName;

    DPC( printf("initPointerClass\n") );
    return ( makePublicClass( PointerClassName, RootClassName,
		sizeof( struct MousePointer ), dispatchPointerClass ) );
}
