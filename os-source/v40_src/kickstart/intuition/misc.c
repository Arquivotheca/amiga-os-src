
/*** misc.c *****************************************************************
 *
 *  File
 *
 *  $Id: misc.c,v 38.17 92/11/10 17:08:18 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#define TEXTPAD 4
/*???#define DEBUG 1*/
/*#define DEBUGMOUSE*/

#include "intuall.h"

#define DFR(x)		;

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef GRAPHICS_SPRITE_H
#include <graphics/sprite.h>
#endif

#ifndef GRAPHICS_GFXMACROS_H
#include <graphics/gfxmacros.h>
#endif

/*** intuition.library/AllocRemember ***/

CPTR AllocRemember(remember, size, flags)
struct Remember **remember;
ULONG size;
ULONG flags;
{
    struct Remember *nostalgia;

    nostalgia = *remember;
    DFR( printf("AllocR: nostalgia = %lx\n", nostalgia ) );

    if ((*remember = (struct Remember *)AllocMem(sizeof(struct Remember), 0))
	     == 0)
	{
	*remember = nostalgia;
	return(NULL);
	}

    if (((*remember)->Memory = (UBYTE *)AllocMem(size, flags)) == 0)
	{
	FreeMem(*remember, sizeof(struct Remember));
	*remember = nostalgia;
	return(NULL);
	}

    (*remember)->RememberSize = size;
    (*remember)->NextRemember = nostalgia;
    return((CPTR) (*remember)->Memory);
}


/*** intuition.library/FreeRemember ***/

FreeRemember(remember, reallyforget)
struct Remember **remember;
BOOL reallyforget;
{
    struct Remember *present, *past;

    DFR( printf("enter FreeRemember, key at %lx\n", remember));

    present = *remember;
    /* Peter 8-Aug-90: Now we NULL out *remember as early as possible
     * instead of at the end, in case the caller stored the handle in a
     * chunk which we will be freeing here
     */
    *remember = NULL;
    while (present)
	{
	past = present->NextRemember;
	DFR( printf("present %lx, past: %lx\n", present, past ) );
	if (reallyforget)
	{
	    DFR( printf("block %ld %lx,",
		present->RememberSize, present->Memory) );
	    FreeMem(present->Memory, present->RememberSize);
	}
	DFR( printf(" header: %lx.  ", present) );
	FreeMem(present, sizeof(struct Remember));
	present = past;
	DFR( printf("next Remember %lx\n", present ) );
	}
    DFR( printf("ok "));

    DFR( printf("  done \n"));
}


struct Layer *getGimmeLayer(w)
struct Window *w;
{
    struct Layer *l = w->WLayer;

    if (w->Flags & GIMMEZEROZERO) l = w->BorderRPort->Layer;

    return(l);
}

cloneRP( clonerp, rp )
struct RastPort	*clonerp;
struct RastPort	*rp;
{
    *clonerp = *rp;
    /* Peter 11-May-92: Geez, all this time we've been setting 
     * rp->Mask to -1.  That's the wrong RastPort.  How did
     * it ever work???
     *
     * Also, we now NULL-out the TmpRas of the clone.  The reason
     * is sinister.  Starting with V37, Gfx uses the TmpRas if there
     * is one for temporary storage during Text() calls.  This can
     * interfere with application usage of the TmpRas to do
     * normal operations like area-fills.
     */
    clonerp->Mask = -1;
    clonerp->TmpRas = NULL;
}

cloneRPFont( clonerp, rp )
struct RastPort	*clonerp;
struct RastPort	*rp;
{
    SetFont( clonerp, rp->Font );
    clonerp->AlgoStyle = rp->AlgoStyle;
}

BlastPatternBox( rp, box, pat, size, pena, penb, mode )
struct RastPort *rp;
struct IBox	*box;
USHORT 		*pat;
UBYTE 		pena, penb;
UBYTE		mode;
{
	BlastPattern( rp, box->Left, box->Top, 
	    boxRight( box ), boxBottom( box ),
	    pat, size, pena , penb, mode);
}

/* jimm: 4/4/86: took out layer parameter: use one in RP */
BlastPattern(RP, left, top, right, bottom, pattern, size, 
	pena, penb, mode)
struct RastPort *RP;
int left, top, right, bottom, size;
USHORT *pattern;
int pena, penb, mode;
{
    /* assume RP can be messed with
     * jimm: 2/6/86: check all cases, either using BarLayer->rp,
     * MenuRPort, or IBase->RP (locked)
     */

/** 
    printf("BP l/t/r/b: %ld/%ld/%ld/%ld size %ld\n",
    left, top, right, bottom, size);
**/

    SetAfPt(RP, pattern, size);
    SetABPenDrMd(RP, pena, penb, mode);
    SafeRectFill(RP, left, top, right, bottom);

    SetAfPt(RP, NULL, 0);	/* Peter 12-Dec-91: no ghosted string gadgets */
}


drawBox(rp, x, y, width, height, apen, mode, hthick, vthick)
struct RastPort *rp;
int x, y, width, height, hthick, vthick;
int apen, mode;
/* draws the Box */
{
    int right, bottom, i;

    right = x + width - 1;
    bottom = y + height - 1;

    SetABPenDrMd(rp, apen, 0, mode);

    /* verticals	*/
    for (i = 0; i < hthick; i++)
	{
	Move(rp, x + i, y);
	Draw(rp, x + i, bottom);
	Move(rp, right - i, y);
	Draw(rp, right - i, bottom);
	}

    x += hthick;
    right -= hthick;

    /* horizontals	*/
    for (i = 0; i < vthick; i++)
	{
	Move(rp, x, y + i);
	Draw(rp, right, y + i);
	Move(rp, x, bottom - i);
	Draw(rp, right, bottom - i);
	}
}


xorbox(rp, left, top, width, height)
struct RastPort *rp;
int left, top, width, height;
{
    rp->Mask = -1;
    xorboxmask(rp, left, top, width, height);
}
xorboxmask(rp, left, top, width, height)
struct RastPort *rp;
int left, top, width, height;
{
    /** jimm: looks like it ain't used struct SaveRPort saver; **/

    SetDrMd(rp, COMPLEMENT);
    SetAfPt(rp, NULL, 0);	/* jimm: 4/13/90: no ghosted gadgets  */
    RectFill(rp, left, top, left + width - 1, top + height - 1);
}

/*???*/
#define MENUHPADDING 4
#define MENUVPADDING 2

outbox(rp, left, top, width, height)
struct RastPort *rp;
int left, top, width, height;
{
    /*** mask = rp->Mask; **/
    /* rp->Mask = -1; already set up */

    drawBox(rp, left - MENUHPADDING, top - MENUVPADDING, 
	    width + (MENUHPADDING << 1), height + (MENUVPADDING << 1),
	    0, COMPLEMENT, MENUHPADDING, MENUVPADDING);

    /** rp->Mask = mask; ***/
}

drawBlock(rp, left, up, right, down, apen)
struct RastPort *rp;
int left, up, right, down;
int apen;
{
    rp->Mask = -1;
    SetABPenDrMd(rp, apen, 0, JAM2);
    /* jimm: 4/25/86: got to get the pattern clear */
    SetAfPt(rp, NULL, 0);

    SafeRectFill(rp, left, up, right, down);
}


/*** intuition.library/DrawImage ***/

#if 0
/* moved temporarily to imageclass.c */
DrawImage(rport, image, xoffset, yoffset)
struct RastPort *rport;
struct Image *image;
int xoffset, yoffset;
{
    while (image)
	{
	drawImageGrunt(rport, image, xoffset, yoffset);
	image = image->NextImage;
	}
}
#endif

drawImageGrunt(rport, image, xoffset, yoffset, minterm )
struct RastPort *rport;
struct Image *image;
int xoffset, yoffset;
{
    struct BitMap BMap;
    int	plane;
    unsigned int planesize;
    UBYTE	mask;
    UWORD	*planedata;

    /* jimm: 5/23/90: sanity check on image dims used to
     * be done "implicitly" by AllocMem() of planeonoff planes.
     * Now we must be explicit for compatibility.
     */
    if ( image->Height <= 0 || image->Width <= 0 ) return;

    /* initialize a bitmap */
    InitBitMap( &BMap, rport->BitMap->Depth, image->Width, image->Height );

    mask = ( 1 << 0 );	/* identifies plane 0 */

    planedata = image->ImageData;
    planesize = ( BMap.BytesPerRow * BMap.Rows ) / 2;	   /* in words */

    /* init plane pointers to one of: data, all ones, all zeroes */
    for ( plane = 0, mask = 1; plane < BMap.Depth; ++plane, mask <<= 1 )
    {
	if ( mask & image->PlanePick ) 
	{
	    BMap.Planes[ plane ] = (PLANEPTR) planedata;
	    planedata += planesize;
	}
	else if ( mask & image->PlaneOnOff )
	{
	    BMap.Planes[ plane ] = (PLANEPTR) -1;
	}
	else
	{
	    BMap.Planes[ plane ] = NULL;
	}
    }


    BltBitMapRastPort(&BMap, 0, 0, rport, xoffset + image->LeftEdge,
	    yoffset + image->TopEdge, image->Width, image->Height, minterm );
}

/*** intuition.library/DrawBorder ***/

DrawBorder(rp, border, left, top)
struct RastPort *rp;
struct Border *border;
int left, top;
{
    int i, x, y;
    SHORT *xypair;
    int trueleft, truetop;
    struct RastPort clonerp;

    cloneRP( &clonerp, rp );

    /* clonerp.Mask = -1; *already */

    while (border)
	{
	SetABPenDrMd(&clonerp, border->FrontPen, border->BackPen, border->DrawMode);
	trueleft = left + border->LeftEdge;
	truetop = top + border->TopEdge;
	xypair = border->XY;
	x = *xypair++ + trueleft;
	y = *xypair++ + truetop;
	Move(&clonerp, x, y);
	for (i = border->Count - 1; i > 0; i--)
	    {
	    x = *xypair++ + trueleft;
	    y = *xypair++ + truetop;
	    Draw(&clonerp, x, y);
	    }
	border = border->NextBorder;
	}
}


/*** intuition.library/PrintIText ***/

PrintIText(rp, itext, left, top)
struct RastPort *rp;
struct IntuiText *itext;
int left, top;
{
    printIText(rp, itext, left, top, TRUE );

    /* side-effect compatibility	*/
    return ( TRUE );
}

/* need a version which ignores pens and draw mode
 * for newlook compatible AutoRequest
 */
printIText(rp, itext, left, top,  use_it_pens_and_mode )
struct RastPort *rp;
struct IntuiText *itext;
int left, top;
{
    struct RastPort	clonerp;
    /** MODIFY: jimm 11/7/85 **/
    struct TextFont	*ISetFont();
    struct TextFont	*tmpfp = NULL;

    cloneRP( &clonerp, rp );

    /* clonerp.Mask = -1; *already */

    while (itext)
    {
	if ( use_it_pens_and_mode )
	{
	    SetABPenDrMd( &clonerp, itext->FrontPen, itext->BackPen, itext->DrawMode );
	}

	/** MODIFY: jimm 11/7/85 , default to rp font, not sys font **/
	if (itext->ITextFont != NULL)
	{
	    tmpfp = ISetFont( &clonerp, itext->ITextFont);
	}

	Move( &clonerp, left+itext->LeftEdge,
		top+itext->TopEdge+ clonerp.TxBaseline);

	/* jimm: 7/22/86: someone has to protect against
	 * NULL text pointers, don't they?!
	 */
	if (itext->IText) Text( &clonerp, itext->IText, strlen(itext->IText));

	/** MODIFY: jimm 11/7/85 **/
	if (itext->ITextFont != NULL)
	{
		ICloseFont(tmpfp);
		/* restore rp default font	*/
		cloneRPFont( &clonerp, rp );
	}

	itext = itext->NextText;
    }

    /* jimm: 9/16/88:
     * clone approach screws people expecting cp to change
     */
    rp->cp_x = clonerp.cp_x;
    rp->cp_y = clonerp.cp_y;
}


/*** intuition.library/ReportMouse ***/

#if 0	/* downcoded	*/
ReportMouse(w, bool)
struct Window *w;
BOOL bool;
{
	/* this function should be rewritten to reverse the parameters */
    /* Forbid(); */	/* jimm:5/20/86 */
    if (bool) w->Flags |= REPORTMOUSE;
    else w->Flags &= ~REPORTMOUSE;
    /* Permit(); */
}
#endif


/*** intuition.library/ItemAddress ***/

struct MenuItem *ItemAddress(menustrip, menunum)
struct Menu *menustrip;
USHORT menunum;
{
    struct Menu *menu;
    struct MenuItem *item, *sub;

    item = NULL;
    if (menu = grabMenu(menustrip, menunum))
	if (item = grabItem(menu, menunum))
	    if (sub = grabSub(item, menunum)) item = sub;

    return(item);
}


/*** intuition.library/DoubleClick ***/

BOOL
DoubleClick(startsec, startmicro, endsec, endmicro)
ULONG startsec, startmicro, endsec, endmicro;
{
    LONG longi;
    struct IntuitionBase *IBase = fetchIBase();

    if (endmicro < startmicro)
	{
	endsec--;
	endmicro += 1000000;
	}
    longi = (endsec - startsec) - IBase->DoubleSeconds;
    if (longi < 0) return(TRUE);
    if (longi == 0) 
	if (endmicro - startmicro < IBase->DoubleMicros) return(TRUE);
    return(FALSE);
}


/*** intuition.library/CurrentTime ***/

CurrentTime(secs, micros)
ULONG *secs, *micros;
{
    struct IntuitionBase *IBase = fetchIBase();

    *secs = IBase->Seconds;
    *micros = IBase->Micros;
}


/*** intuition.library/ViewAddress ***/

struct View *ViewAddress()
{
    struct IntuitionBase *IBase = fetchIBase();

    return(&IBase->ViewLord);
}


/*** intuition.library/ViewPortAddress ***/

#if 0	/* do this in the vector	*/
struct ViewPort *ViewPortAddress(window)
struct Window *window;
{
    return(&window->WScreen->ViewPort);
}
#endif


/*** intuition.library/DisplayBeep ***/

IDisplayBeep(screen)
struct Screen *screen;
{
    struct IntuitionBase *IBase = fetchIBase();

    /* flash color 0 of the window */
    LOCKIBASE();
    /* still need this lock to protect screen list, but
     * an additional Forbid() is used to protect against
     * save/restore conflicts
     */

    /* screen of NULL or ~0 means beep all screens. */
    if ( ( screen == (struct Screen *) ~0 ) || ( !screen ) )
    {
	screen = IBase->FirstScreen;
	while (screen)
	{
	    if ( !TESTFLAG( screen->Flags, SCREENHIDDEN ) )
	    {
		beepGrunt( screen );
	    }
	    screen = screen->NextScreen;
	}
    }
    else
    {
	beepGrunt(screen);
    }
    UNLOCKIBASE();
}

/* This routine does a kind of complementing of the color, but
 * which is good for grays, too.  (The simple complement of
 * a middle-gray is almost the same shade).
 * We set each color's component to 5 or 12, whichever is farther
 * from its current value.
 */
ULONG beepComplement(color)
ULONG color;
{
    if ( color > 0x88888888 )
    {
	/* If the color component is bright, return dark */
	return( 0x55555555 );
    }
    else
    {
	/* Else return bright */
	return( 0xCCCCCCCC );
    }
}

beepGrunt(screen)
struct Screen *screen;
{
    ULONG red, green, blue;

    /* Peter: 6-Aug-90
     * beepGrunt (from IDisplayBeep) is the only place where BEEPING
     * is set, so we are protected by IDB's LOCKIBASE.
     * The protocol will be: test flag, change the color, change flag.
     */
    /* is this one Beeping yet? */
    if ((screen->Flags & BEEPING) == 0)
    {
	/* Save off color zero, at 32 bits per gun.  Relies
	 * on SaveRed, SaveGreen, SaveBlue being consecutive in the
	 * XScreen structure.
	 */
	GetRGB32( screen->ViewPort.ColorMap, 0, 1, &XSC(screen)->SaveRed );

	red = beepComplement( XSC(screen)->SaveRed );
	green = beepComplement( XSC(screen)->SaveGreen );
	blue = beepComplement( XSC(screen)->SaveBlue );

	SetRGB32( &screen->ViewPort, 0, red, green, blue );

	/* Extend the beep */
	XSC(screen)->PrivateFlags |= PSF_DELAYBEEP;
	/* not Beeping yet, so start it up now */
	screen->Flags |= BEEPING;
    }
}


/* lock system shared rastport, but first lock layer rom
 * for layer passed.  if this rastport is to be layered,
 * lots of calls will lockrom, but order is layers before rp
 *
 * WARNING: because of UserClipRegions: this routine should not
 * be 'nested' since IBase->OldClipRegion, etc., get clobbered.
 * jimm made a change in drawGadgets() to avoid nesting, but if
 * it must be done, skip the save/restore of layer data if 
 * ss_NestCount > 1 (in freeRP(), too).
 *
 * V36: protect self: set mode to JAM1, Mask to ~0 guaranteed.
 */
struct RastPort *obtainRP( clonee, layer )
struct RastPort *clonee;	/* will copy this guy if non-NULL */
struct Layer *layer;
{
    struct IntuitionBase *IBase = fetchIBase();

    if ( !clonee ) return ( NULL );

    if (layer)
    {
	LockLayerRom( layer );
    }

    LOCKRP();

#ifdef DEBUG
    /* DEBUG */
    if (IBase->ISemaphore[RPLOCK].ss_NestCount > 1)
    {
	printf("double rp lock %ld.\n",IBase->ISemaphore[RPLOCK].ss_NestCount);
	Debug();
    }
#endif

    if (layer)
    {
	/* save and change some things */
	IBase->OldClipRegion = (struct Region *) InstallClipRegion(layer, NULL);

	/* So we render good whether the guy called ScrollLayer
	 * or not:
	 */
	IBase->OldScroll.X = layer->Scroll_X;
	layer->Scroll_X = 0;	/* rendering offset */

	IBase->OldScroll.Y = layer->Scroll_Y;
	layer->Scroll_Y = 0;
    }

    cloneRP( IBase->RP, clonee );

    IBase->RP->Layer = layer;

    SetDrMd( IBase->RP, JAM1 );

    return( IBase->RP );
}

/*** intuition.library/ReleaseGIRPort ***/

/* I really only need this for in-rom gadget classes.
 */
ReleaseGIRPort( rp )
struct RastPort *rp;
{
    freeRP(rp);
}

freeRP(rp)
struct RastPort *rp;	/* may be from pool */
{
    struct IntuitionBase *IBase = fetchIBase();

    if ( !rp ) return;

    if (rp->Layer) 
    {
	/* So we render good whether the guy called ScrollLayer
	 * or not:
	 */
	rp->Layer->Scroll_X = IBase->OldScroll.X;
	rp->Layer->Scroll_Y = IBase->OldScroll.Y;

	InstallClipRegion(rp->Layer, IBase->OldClipRegion);
	UnlockLayerRom(rp->Layer);
    }
    UNLOCKRP();
}

/*** intuition.library/ObtainGIRPort ***/

struct RastPort	*
ObtainGIRPort( gi )
struct GadgetInfo	*gi;
{
    if ( gi == NULL ) return ( NULL );	/* didn't lock anything */
    /* ZZZ: maybe there are "partially setup" ginfo's that
     * a gadget can use to get resolution, font, and color
     * from, but you can't render.  Also, perhaps, the
     * gadget might not be added to a window yet.
     */
    return ( obtainRP( gi->gi_RastPort, gi->gi_Layer ) );
}

