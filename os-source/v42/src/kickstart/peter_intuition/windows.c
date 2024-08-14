
/*** windows.c ***************************************************************
 *
 *  windows.c for Windows and Screens
 *
 *  $Id: windows.c,v 38.36 93/01/14 14:22:58 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "intuall.h"

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#include "newlook.h"

#include "windows_protos.h"

/*---------------------------------------------------------------------------*/

static int getleft(struct Window * w);

static int getright(struct Window * w);

static int drawEmbossedWFill(struct Window * w);

static int drawEmbossedEdges(struct Window * w);

static int updateEmbossedTitle(struct Window * w);

static int printEmbossedTitle(struct RastPort * rp,
                              struct Window * w,
                              BOOL clearout);

/*---------------------------------------------------------------------------*/

#define DST(x)	;		/* show title		*/
#define DOW(x)	;		/* debug open window	*/
#define DGR(x)	;		/* debug getright() 	*/
#define DADD(x)	;		/* addSWGadgets()	*/
#define DEMB(x)	;		/* embossed look	*/
#define DSWT(x)	;		/* setwindowtitles	*/
#define DSW(x)	;		/* SizeWindow		*/

#define D(x)	;
#define DPS(x)	;	/* pub screens	*/
#define DCW(x)	;	/* close window	*/
#define DCW2(x)	;	/* close window	*/
#define DSIZE(x) ;	/* open window size checks */

/* = declarations ======================================================== */
struct Layer *getGimmeLayer(), *WhichLayer();
BOOL WindowLimits();

/* these arrays pair system gadget types with the
 * flags which are necessary to have them added.
 */
#define WGTYPECOUNT 5
USHORT WGadgetTypes[WGTYPECOUNT] =
    {
    WINDOWDEPTH,
    0,		/* special processing uses  'addzoom'	*/
    WINDOWSIZING,
    WINDOWCLOSE,
    WINDOWDRAG,
    };
USHORT WGadgets[WGTYPECOUNT] =
    {
    UPFRONTGADGET,
    ZOOMGADGET,
    SIZEGADGET, 
    CLOSEGADGET, 
    DRAGGADGET, 
    };




/* ======================================================================= */
/* ======================================================================= */
/* ======================================================================= */

struct Window *hitUpfront(layerhit)
struct Layer **layerhit;
/* uses global coordinates and window lists to find the window hit
 * this routine presumes that IBase->FirstScreen points to the
 * frontmost screen, and that they are sorted to the back
 * 
 * OLD WAY (V29): if a Screen is hit but no Window in the Screen is hit, call 
 * setWindow(NULL) and then setScreen() for that Screen 
 * before returning NULL
 *
 * NEW WAY (V1): if a Screen BarLayer is hit, set IBase->Flag HIT_BARLAYER and
 * don't disturb the active state of anything else
 */
{
    struct Layer *layer = NULL;
    struct Window *returnwindow;
    struct Screen *s;
    struct IntuitionBase *IBase = fetchIBase();
    struct Screen *hitScreen();

    returnwindow = NULL;

    CLEARFLAG(IBase->Flags, HIT_BARLAYER);

    if (s = hitScreen())
    {
	if (layer = WhichLayer(&s->LayerInfo, s->MouseX, s->MouseY))
	    goto LAYERHIT;
	goto SCREENHIT;
    }

    goto HITUPDONE;

LAYERHIT:
    /* we hit some layer in Screen s */
    /* did we hit the Screen BarLayer? */
    if (layer == s->BarLayer) 
    {
	returnwindow = IBase->ActiveWindow;
	SETFLAG(IBase->Flags, HIT_BARLAYER);
    }

    /* else we MUST have hit one of the Window's Layers
     * or requester layer
     */
    else
    {
	returnwindow = layer->Window;
    }

    goto HITUPDONE;

SCREENHIT:
    /* we hit a screen, but no layer in that screen */
    /* layer == NULL */
    /* returnwindow == NULL */

HITUPDONE:
    
    *layerhit = layer;
    return(returnwindow);
}

struct Screen *
hitScreen()
{
    struct IntuitionBase *IBase = fetchIBase();
    struct Screen *s;

    assertLock("hitScreen", IBASELOCK );

    for (s = IBase->FirstScreen; s; s = s->NextScreen)
    {
	if (s->ViewPort.Modes & VP_HIDE) continue;

	/* jimm: 2/25/88: use screen mouse instead of IBase->MouseX	*/

	/* this is the test to see if it is in screen	*/
	if (s->MouseY >= 0)
	{
	    return( IBase->HitScreen = ((s->MouseY >= s->Height)? NULL: s) );
	}
    }
    return( IBase->HitScreen = NULL );
}

/*** intuition.library/OpenWindow ***/


struct Window
*OpenWindow( nw )
struct ExtNewWindow *nw;
{
    struct Window *OpenWindowTagList();
    struct TagItem	*tlist = NULL;

    if ( nw && TESTFLAG( nw->Flags, NW_EXTENDED ) )
    {
	tlist = nw->Extension;

	/* A frustrating number of people put IDCMP_ACTIVEWINDOW into
	 * nw.Flags by mistake, where it's numerically equal to NW_EXTENDED.
	 * In hope of catching some of them, we look for a bad pointer,
	 * which is any odd pointer or any pointer not in RAM as reported
	 * by exec/TypeOfMem().
	 */

	if ( ( ( (ULONG) tlist ) & 1 ) || ( !TypeOfMem( tlist ) ) )
	{
	    tlist = NULL;
	}
    }
    return ( OpenWindowTagList( nw, tlist ) );
}

/*
 * default ISM processing for itOPENWIN
 * returns 0 for success: it's put back in it_Error
 */
IOpenWindow( w, superbmap, backfill )
struct Window 	*w;
struct BitMap	*superbmap;
struct Hook 	*backfill;		/* layer backfill hook */
{
    struct Screen 	*sc = w->WScreen;

    /* refresh mode flags: key point is that for gimmezerozero window,
     * the larger layer is always simple refresh, and the inner one
     * is as requested in the Flags.  For normal window, the large
     * layer is the only layer, and refreshes as requested
     */
    ULONG 		refresh;
    ULONG		simple;
    ULONG		firstflags;

    struct Window	*parent;
    struct Layer	*layer = NULL;	
    struct Layer	*bdrlayer = NULL;
    struct RastPort	*rport;
    struct Layer 	*CreateUpfrontHookLayer();
    struct TextAttr	*wfont;

    DOW( printf("IOW: w %lx\n", w ) );

    /* ---------------------------------------------------------------- */
    /*	Calculate refresh mode for regular and gimmezerozero layers	*/
    /* ---------------------------------------------------------------- */

    if ((w->Flags & REFRESHBITS) == SUPER_BITMAP) refresh = LAYERSUPER;
    else if ((w->Flags & REFRESHBITS) == SIMPLE_REFRESH) refresh = LAYERSIMPLE;
    else refresh = LAYERSMART;

    simple = LAYERSIMPLE;
    if ( TESTFLAG( w->Flags, BACKDROP ) )
    {
	simple |= LAYERBACKDROP;
	refresh |= LAYERBACKDROP;
    }

    /* border layer for G00 is always simple refresh	*/
    if ( TESTFLAG( w->Flags, GIMMEZEROZERO ) )
	firstflags = simple;
    else
	firstflags = refresh;

    /* ---------------------------------------------------------------- */
    /*	Get Layers							*/
    /* ---------------------------------------------------------------- */
    LockLayerInfo( &sc->LayerInfo );

    DOW( printf("IOW: call CreateUpfrontHookLayer backfill %lx\n", 
	backfill) );
    DST( printf("IOW: about to create layer, flags %lx\n", firstflags));

    layer = CreateUpfrontHookLayer( &sc->LayerInfo, sc->RastPort.BitMap,
	    w->LeftEdge, w->TopEdge,
	    w->LeftEdge + w->Width - 1,
	    w->TopEdge + w->Height - 1,
	    firstflags, 
	    TESTFLAG( w->Flags, GIMMEZEROZERO )? NULL: backfill,
	    superbmap );

    DOW( printf("IOW: layer returned:%lx\n",layer) );

    if ( ! layer )	goto OUTBAD;

    rport = layer->rp;			/* comes already initialized	*/
    w->RPort = rport;

    /* jimm: 2/12/90: inherit vanilla font if screen has
     * used tricky new feature.
     */
    if ( TESTFLAG( XSC(sc)->PrivateFlags, PSF_SCREENFONT ) )
    {
	wfont = &fetchIBase()->SysFontPrefs[ 0 ];	/* vanilla sysfont */
    }
    else
    {
	wfont = sc->Font;
    }
    ISetFont( rport, wfont );
    w->IFont = rport->Font; /* save font just opened, for CloseWindow	*/

    layer->Window = w;
    w->WLayer = layer;

    /* if this Window is opened as BACKDROP, while the Screen is
     * supposed to have its Title behind all else, push the
     * Screen back now
     */
    if ( TESTFLAG( w->Flags, BACKDROP)  && (sc->Flags & SHOWTITLE)  )
    {
	struct Layer	*barlayer;

	/* move barlayer in front of new layer */
	DST( kpause("about to move bar layer in front of window"));

	barlayer = sc->BarLayer;
	MoveLayerInFrontOf( barlayer, layer );

	/* jimm: 1/17/90: update screenbar visuals */
	refreshScreenBar( sc );
    }

    /* ---------------------------------------------------------------- */
    /*	Get smaller GIMMEZEROZERO layer in front of border layer	*/
    /* ---------------------------------------------------------------- */
    if ( TESTFLAG( w->Flags, GIMMEZEROZERO ) )
    {
	/* large layer becomes the border layer */
	bdrlayer = layer;

	layer = CreateUpfrontHookLayer( &sc->LayerInfo, sc->RastPort.BitMap,
	    w->LeftEdge + w->BorderLeft,
	    w->TopEdge + w->BorderTop,
	    w->LeftEdge + w->Width - 1 - w->BorderRight,
	    w->TopEdge + w->Height - 1 - w->BorderBottom, 
	    refresh, 
	    backfill,
	    superbmap );

	/* move new layer in front of border layer	*/
	if ( layer == NULL || ! MoveLayerInFrontOf( layer, bdrlayer ) ) {
	    goto OUTBAD;
	}

	layer->Window = w;
	w->WLayer = layer;

	rport = layer->rp;
	rport->TmpRas = NULL;
	rport->AreaInfo = NULL;

	/* swap out old window rport and use new layer rport */
	w->BorderRPort = w->RPort;
	w->RPort = rport;
    }
    UnlockLayerInfo( &sc->LayerInfo );

    /* ---------------------------------------------------------------- */
    /*	Link window in to system					*/
    /* ---------------------------------------------------------------- */
    LOCKIBASE();

    w->Descendant = NULL;
    parent = fetchIBase()->ActiveWindow;

    if ( w->Parent = parent )
    {
	if ( parent->Descendant )
	{
	    parent->Descendant->Parent = w;
	    w->Descendant = parent->Descendant;
	}
	parent->Descendant = w;
    }

    w->NextWindow = sc->FirstWindow;
    sc->FirstWindow = w;

    UNLOCKIBASE();

    return ( 0 );	/* no error */

OUTBAD:		/* I am holding LayerInfo lock	*/
    /* delete what I got	*/
    if ( layer ) DeleteLayer( NULL, layer );
    if ( bdrlayer ) DeleteLayer( NULL, bdrlayer );
    if ( w->IFont ) CloseFont( w->IFont );
    /* Repair any damage that might have occurred during failure
     * to create a layer
     */
    BorderPatrol( w->WScreen );
    UnlockLayerInfo( &sc->LayerInfo );
    return ( 1 );
}



/*** intuition.library/CloseWindow ***/

CloseWindow(window)
struct Window *window;
{
    struct PubScreenNode *psn = NULL;

    /* right away */
    DCW2( printf("CloseWindow %lx ", window ) );
    DCW( printf("CloseWindow %lx %s.  ", window, window->Title ) );
    ModifyIDCMP(window, NULL);
    DCW( printf("CW: modifyidcmp returned ...") );

    /* We'll need to care for the public screen, if
     * this is a visitor.
     */
    if ( TESTFLAG( window->Flags, VISITOR ) )
    {
	/* make sure it's really a public screen,
	 * make sure I'm not being cheated on by a 
	 * replacement workbench screen with no PSNode!
	 */
	psn = XSC(window->WScreen)->PSNode;
    }

    /* Failure level of 2 indicates we must undo everything */
    closeWindowCommon( window, 2 );

    if ( psn )
    {
	/* Decrement the visitor count, and signal the screen's
	 * owner if this was the last visitor.
	 */
	if ( decPSNVisitor( psn ) == 0 )
	{
	    if ( psn->psn_SigTask )	/* does he want a signal	*/
	    {
		Signal( psn->psn_SigTask, 1 << psn->psn_SigBit );
	    }
	}
    }
    DCW( printf( "CloseWindow done\n") );
}


/* Window cleanup-code common to CloseWindow() and OpenWindow()-failure.
 * 'failure' = 1 indicates we didn't get to doISM in OpenWindowTagList().
 * 'failure' = 2 indicates we did, or we came from CloseWindow().
 */

closeWindowCommon( window, failure )

struct Window *window;
ULONG failure;

{
    struct IntuiMessage *message, *nextmessage;

    /* Peter 18-Dec-90: failure > 1 indicates we passed doISM.  In this
     * case, we must undo it with doISM(itCLOSEWIN).
     */
    if ( failure > 1 )
    {
	/* delink, activate Parent if I was active */
	doISM( itCLOSEWIN, window );
	DCW( printf("CW: doISM returned\n") );

	/* jimm: 6/25/86: close font i might have opened */
	/* jimm: 7/29/86: close just the font I opened   */
	CloseFont(window->IFont);
    }

    freeSysGadgets( window->FirstGadget );

    /* this linked list is safe because the window is delinked */
    message = window->MessageKey;
    while (message)
    {
	nextmessage = message->SpecialLink;
	/* Use AllocVec/FreeVec so freeing them is independent of
	 * size differences between tablet & non-tablet windows
	 */
	FreeVec( message );
	message = nextmessage;
    }

    FreeVec( window );
}

/*** intuition.library/ActivateWindow ***/

VOID
ActivateWindow(window)
struct Window *window;
{
    sendISM( itACTIVATEWIN, window, AWIN_NORMAL );
}

    
/* add the system window gadgets to the window, link them in.
 * special handling for drag gadget, since it isn't a "newstyle"
 * gadget and its height depends on the window top border height
 */
addSWGadgets(w, addzoom )
struct Window *w;
{
    ULONG flags, res;
    struct Gadget *g, *linkg;
    struct Gadget *createNewSysGadget();
    int count;
    BOOL gimme;

    /* determine resolution from screen */
    /* jimm: 8/26/86: eliminate screen parameter of addSWGadgets() */
    res = LOWRESGADGET;
    if (IsHires(w->WScreen)) res = HIRESGADGET;

    gimme = w->Flags & GIMMEZEROZERO;

    flags = w->Flags;
    linkg = (struct Gadget *) &w->FirstGadget;

    for (count = 0; count < WGTYPECOUNT; count++)
    {
	if ( ( flags & WGadgetTypes[count] ) ||
		(( WGadgets[count] == ZOOMGADGET ) && addzoom) )
	{

	    g = createNewSysGadget( w->WScreen, res, WGadgets[count], gimme );
	    /* link in new gadget to end of list */
	    if ( g )
	    {
		linkg->NextGadget = g;
		linkg = g;
		/* g->NextGadget = NULL; automatic */
	    }
	    else
	    {
		return 0;	/* caller will want to free list */
	    }

	    /* special adjustment for the height of the draggadget */
	    /* ZZZ: should be more oops? */
	    if ( WGadgets[count] == DRAGGADGET )
	    {
		g->Height = w->BorderTop - 1;
	    }
	}
    }

    /* all gadgets needed were gotten */
    return( 1 );
}


freeSysGadgets(g)
struct Gadget *g;
{
    struct Gadget *oldg, *workg;

    oldg = NULL;

    while (g)
    {
	workg = g->NextGadget;

	if (g->GadgetType & SYSGADGET)
	{
	    if (oldg) oldg->NextGadget = workg;
	    /* dispose, not free, system custom gadgets */
	    if (( g->GadgetType & GTYPEMASK ) == CUSTOMGADGET )
	    {
		DisposeObject( g );
	    }
	    else
	    {
		FreeMem(g, sizeof(struct Gadget));
	    }
	}
	else oldg = g;

	g = workg;
    }
}


static int
getleft(w)
struct Window *w;
/* welcome to kludge city.  here you will find me using hard-coded constants
 * rather than fetching the width values from the CloseWindow Gadgets
 * that will come someday, honest
 */
{
    if (w->Flags & WINDOWCLOSE)
    {
	return( IsHires(w->WScreen)? 30: 16);
    }
    else return(w->WScreen->WBorLeft);
}

#define GADGETPAD	(4)		/* margin before gadget */
/*
 * getright() returns the right edge of the drag-bar part.  This
 * starts at the window width.  If the window has zoom or depth
 * gadgets, the right-edge is moved accordingly.
 * We also account for GFLG_TOPBORDER GACT_RELRIGHT gadgets that
 * the application installed.
 *
 * Finally, the result has GADGETPAD taken off, for good measure.
 * One pixel is for the stripe, and the rest keep the window title
 * from abutting the last gadget.
 */
static int
getright( w )
struct Window	*w;
{
    struct Gadget *g;
    int	relright = 0;

    /* Snoop through the right-hand titlebar gadgets.  Application
     * gadgets that are there are GFLG_RELRIGHT and GACT_TOPBORDER.
     * For whatever reason, system gadgets in the top border are
     * not marked as TOPBORDER (though they are BORDERSNIFF).
     * So until we decide to change that, we have to look especially
     * for the two (GTYP_WUPFRONT and GTYP_WDOWNBACK, zoom and depth)
     * that we care about.
     */
    for ( g = w->FirstGadget; g; g=g->NextGadget )
    {
	UWORD sysgtype = g->GadgetType & GTYP_SYSTYPEMASK;
	if ( TESTFLAG(g->Flags, GRELRIGHT) &&
	    ( TESTFLAG( g->Activation, TOPBORDER ) ||
	    ( sysgtype == GTYP_WUPFRONT ) ||
	    ( sysgtype == GTYP_WDOWNBACK ) ) )
	{
	    DGR(printf("getright:top/rightgadget,old relright %ld\n",relright));
	    relright = imin( relright, g->LeftEdge );
	}
    }
    return ( relright + w->Width - 1 - GADGETPAD );
}

/*** intuition.library/RefreshWindowFrame ***/

RefreshWindowFrame( w )
struct Window *w;
{
    if ( knownWindow( w ) )
    {
	/* Bloody idiots are poking win->Title and calling
	 * RefreshWindowFrame(), so now I have to invalidate
	 * the WinTitleLength.  Here's to you, my chums...
	 */
	XWINDOW(w)->WinTitleLength = (UWORD) ~0;
	drawEmbossedBorder( w, DRAWGADGETS_ALL );
    }
}

/*
 * The draw_flags variable can be:
 *
 * DRAWGADGETS_ALL - means redraw the border and ALL gadgets in the window
 *	(used by RefreshWindowFrame(), BorderPatrol(),
 *	OpenWindowTagList(), and fixWindowBorders()).
 *
 * DRAWGADGETS_BORDER - means redraw the border, and just the gadgets
 *	clobbered by the border redraw (used by setWindow() to change
 *	activation).
 *
 * The final rendering order must be:
 *
 *	- Draw the window border fill
 *	- Draw the gadgets
 *	- Draw the window's 3D edges (after the gadgets, so the edges
 *	  "win" any fights over pixels)
 *
 * However, we've noted that it looks better if the edges get drawn
 * sooner, so that a repaired or new window's boundary becomes
 * well-defined sooner, not later.  Thus, the edges are drawn twice,
 * except that starting with 3.01, we skip the first drawing of edges
 * if draw_flags is DRAWGADGETS_BORDER, since that's just a change
 * in window activation, and our window is already well-bounded visually.
 *
 */
drawEmbossedBorder( w, draw_flags )
struct Window *w;
int draw_flags;
{
    if (! knownWindow(w)) return;

    LockLayerInfo(&w->WScreen->LayerInfo);
    LOCKGADGETS();
    if (w->BorderRPort) LockLayerRom( w->BorderRPort->Layer );
    LockLayerRom(w->WLayer);

    if ( draw_flags == DRAWGADGETS_ALL )
    {
	drawEmbossedEdges( w );
    }

    drawEmbossedWFill( w );	/* fill in border */

    drawGadgets( w, w->FirstGadget, -1, draw_flags );

    /* draw hilite/shadow edges again (!)	*/
    drawEmbossedEdges( w );

    UnlockLayerRom(w->WLayer);
    if (w->BorderRPort) UnlockLayerRom(w->BorderRPort->Layer);
    UNLOCKGADGETS();
    UnlockLayerInfo(&w->WScreen->LayerInfo);
}

/* RectFill() that filters out degenerate rect's */
SafeRectFill( rp, xmin, ymin, xmax, ymax )
struct RastPort *rp;
WORD xmin, ymin, xmax, ymax;
{
    if ( (xmax >= xmin) && (ymax >= ymin) )
    {
	RectFill( rp, xmin, ymin, xmax, ymax );
    }
}

static
drawEmbossedWFill( w )
struct Window *w;
{
    register struct RastPort *rp;
    register int right, bottom;
    int intop, inbottom;

    rp = obtainRP(&w->WScreen->RastPort, getGimmeLayer(w));
    /* rp->Mask = -1; already */

    /* Peter 26-Nov-90:  To reduce flashing and increase speed, we
     * will only fill INSIDE the border, trusting drawEmbossedEdges()
     * to do the rest.  That's why right and bottom are one less
     * than you'd expect.
     */
    right = w->Width - 2;
    bottom = w->Height - 2;

    /* fill borders with color for window activation */
    SetAPen(rp, embossedFillPen( w ) );

    /* inbottom is the pixel of the top embossed edge of the 
     * bottom border.  Likewise intop is the bottom embossed edge of
     * the top border.  We fill the four corners of the window in
     * the horizontal, not vertical RectFills.
     */
    inbottom = bottom - (w->BorderBottom-2);
    intop = w->BorderTop-1;

    SafeRectFill(rp, 1, intop, w->BorderLeft-2, inbottom);
    SafeRectFill(rp, right - (w->BorderRight-3), intop, right, inbottom);
    SafeRectFill(rp, 1, inbottom + 1, right, bottom);
    if ( w->BorderTop > 2 )
    {
	SafeRectFill(rp, 1, 1, right, intop - 1 );
	/* Don't bother RectFill()ing the remnant after the title */
	printEmbossedTitle( rp, w, FALSE );
    }
    freeRP(rp);
}


static
drawEmbossedEdges( w )
struct Window *w;
{
    register struct RastPort *rp;
    struct IBox	wbox;
    UWORD	*pens = XSC(w->WScreen)->SPens;

    if ( TESTFLAG( w->Flags, BORDERLESS ) ) return;

    rp = obtainRP(&w->WScreen->RastPort, getGimmeLayer(w));
    /* rp->Mask = -1; already */

    windowBox( w, &wbox );
    wbox.Left = wbox.Top = 0;	/* relative to window RPort */

    embossedBoxTrim( rp, &wbox,
	pens[ SHINEPEN ], pens[ SHADOWPEN ], JOINS_UPPER_LEFT_WINS );
    wbox.Left += w->BorderLeft - 1;
    wbox.Top += w->BorderTop - 1;
    wbox.Width -= (w->BorderLeft + w->BorderRight) - 2;
    wbox.Height -= (w->BorderTop + w->BorderBottom) - 2;

    embossedBoxTrim( rp, &wbox,
	pens[ SHADOWPEN ], pens[ SHINEPEN ], JOINS_UPPER_LEFT_WINS );

    freeRP(rp);
}

/*
 * refesh a possibly new  title string,
 * and refresh any gadgets you need to.
 */
static
updateEmbossedTitle( w )
struct Window	*w;
{
    struct RastPort	*rp;
    struct Gadget	*g;

    if ( w->BorderTop == 0 ) return;	/* compatibility */

    LockLayerInfo(&w->WScreen->LayerInfo);
    LOCKGADGETS();
    if (w->BorderRPort) LockLayerRom( w->BorderRPort->Layer );
    LockLayerRom(w->WLayer);
    rp = obtainRP( &w->WScreen->RastPort, getGimmeLayer( w ) );

    printEmbossedTitle( rp, w, TRUE );

    /* Peter 15-Jan-91: This freeRP used to be after the drawGadgets()
     * loop, but drawGadgets() itself calls obtainRP(), and you can't
     * nest those guys.
     */
    freeRP( rp );

    /* refresh all top border gadgets which were
     * not snooped out by getright
     */
    for (g = w->FirstGadget; g; g = g->NextGadget)
    {
	/* If a gadget is in the top border, and is not GRELRIGHT,
	 * we need to redraw it.  Note that we never try this
	 * for the drag-bar (which has no imagery, but only drawGadgets()
	 * knows that) because the drag-gadget is BORDERSNIFF but
	 * not explicitly TOPBORDER.
	 */
	if ( TESTFLAG(g->Activation, TOPBORDER) &&
	    ! TESTFLAG( g->Flags, GRELRIGHT ) )

	{
	    drawGadgets( w, g, 1, DRAWGADGETS_ALL );	/* refresh 1 */
	}
    }

    UnlockLayerRom(w->WLayer);
    if (w->BorderRPort) UnlockLayerRom(w->BorderRPort->Layer);
    UNLOCKGADGETS();
    UnlockLayerInfo(&w->WScreen->LayerInfo);
}

#define VBIG	(7000)

/*
 * clear old text from text portion of title bar,
 * and put new text in, if any
 */
static
printEmbossedTitle( rp, w, clearout )
struct RastPort	*rp;
struct Window	*w;
BOOL clearout;
{
    struct TextExtent te;
    struct Rectangle oldrect;
    int textright;
    int textleft;
    UWORD numchars;

    /* bail out if there is no title bar	*/
    if ( !w->Title  && !TESTFLAG( w->Flags, WINDOWDRAG ) ) return;

    /* text fit into title area */
    textright = getright( w );	/* and snoop out gadgets */
    textleft = getleft( w );

    oldrect = XWINDOW(w)->TitleExtent;

    /* position on baseline, at left of text region */
    Move( rp, textleft, FRAMEHTHICK + rp->TxBaseline );

    if (w->Title)
    {
	/* Peter 4-Feb-91: The WinTitleLength field stores the
	 * number of characters that fit for this window.
	 * By convention, ~0 means "unknown, recalculate and save".
	 */
	if ( ( numchars = XWINDOW(w)->WinTitleLength ) == ((UWORD)~0) )
	{
	    numchars = XWINDOW(w)->WinTitleLength =
		TextFit( rp, w->Title,
		    strlen( w->Title), &te, NULL, 1,
		    textright - textleft, VBIG );

	    TextExtent( rp, w->Title, numchars, &te );
	    XWINDOW(w)->TitleExtent = te.te_Extent;
	    offsetRect( &XWINDOW(w)->TitleExtent, textleft,
		FRAMEHTHICK + rp->TxBaseline );
	}
    }
    else
    {
	degenerateRect( &XWINDOW(w)->TitleExtent );
    }

    /* refill in any leftover title region */
    if ( clearout )
    {
	fillAround( rp, &oldrect, &XWINDOW(w)->TitleExtent, embossedFillPen( w ), ~0 );
    }

    /* print any text I might have here */
    if ( (w->Title) && ( numchars > 0 ) )
    {
	SetABPenDrMd( rp, XSC(w->WScreen)->SPens[ 
	    TESTFLAG( w->Flags,  WINDOWACTIVE )? FILLTEXTPEN: TEXTPEN ],
	    embossedFillPen( w ), JAM2 );
	/* ZZZ: do I get the right font? */

	/* I am already in position */
	Text( rp, w->Title, numchars );
    }
}

embossedFillPen( w )
struct Window	*w;
{
    long pen = XSC(w->WScreen)->SPens[ BACKGROUNDPEN ];

    if ( TESTFLAG( w->Flags, WINDOWACTIVE ) )
    {
	pen = XSC(w->WScreen)->SPens[ FILLPEN ];
    }
    return( pen );
}

/***********************************************************/

/* Sets the window as the ActiveWindow.  If the Window arg is NULL,
 * then the ActiveWindow will be NULL
 * if this Window has its own Pointer, we use it, else we use Intuition's
 *
 * Note for things in this routine that rely on IBase->ActiveWindow:
 *
 * startIdleWindow() NULL's out IBase->ActiveWindow so an
 * active window doesn't get an INACTIVATEWINDOW message and a
 * re-rendering as it closes.
 *
 * The variable "initial" is TRUE to indicate the initial activation
 * of this window.  This tells us to draw all the gadgets.
 */
setWindow( w, initial )
struct Window *w;
LONG initial;
{
    struct Window *AWindow;
    struct Screen *OldAScreen;
    struct IntuitionBase *IBase = fetchIBase();

    assertLock( "setWindow", IBASELOCK );

    /* Peter 2-Dec-90: knownWindow() returns FALSE if w==NULL */
    if (! knownWindow(w)) 
    {
	w = NULL;
    }

    /* Restore things so that we're sure that we'll send a new
     * GADGETHELP event as soon as possible:
     */
    IBase->LastTimeX = IBase->MouseX;
    IBase->LastTimeY = IBase->MouseY;
    IBase->HelpGadget = NULL;

    /* Peter 2-Dec-90:  The idea is that the only screen title which
     * may not be its default is the title of the screen of the active
     * window.
     * Thus, if we're setting "no active window" or changing
     * which screen has the active window, we have to restore the
     * title of the previous ActiveScreen (if there was one) to default.
     */
    if ( (OldAScreen = IBase->ActiveScreen ) &&
	( ( w == NULL ) || ( w->WScreen != OldAScreen ) ) )
    {
	/* Update title of the old screen if it's not the default */
	if (OldAScreen->Title != OldAScreen->DefaultTitle)
	{
	    OldAScreen->Title = OldAScreen->DefaultTitle;
	    screenbar( OldAScreen );
	}
    }

    if (AWindow = IBase->ActiveWindow) 
    {
	/* De-activate the window
	 *
	 * jimm: 5/2/86: got to let application know to
	 * let go of any layerromlock
	 */
	activeEvent(IECLASS_INACTIVEWINDOW, NULL );
	CLEARFLAG( AWindow->Flags, WINDOWACTIVE );

	/* redraw border and border gadgets only */
	drawEmbossedBorder( AWindow, DRAWGADGETS_BORDER );
    }

    /* set the system global ActiveWindow Pointer */
    IBase->ActiveWindow = w;
    IBase->MenuSelected = MENUNULL;
    IBase->MenuDrawn = MENUNULL;

    /* old one of two * activeEvent( IECLASS_EVENT, IECODE_NEWACTIVE ); */

    /* tell console device about active window change */
    activeEvent( IECLASS_EVENT, IECODE_NEWACTIVE );

    if (w == NULL)
    {
	setMousePointer();
    }
    else
    {
	resetMenu(w, ~MIDRAWN,
	    ~ISDRAWN & ~HIGHITEM & ~MENUTOGGLED, ~HIGHITEM & ~MENUTOGGLED);

	SETFLAG( w->Flags, WINDOWACTIVE );

	activeEvent(IECLASS_ACTIVEWINDOW, NULL );

	/* Screen Title Algorithm:
	 * Every time a Window is opened, it's ScreenTitle is set equal to the
	 * Title of the Screen.  Every time a Window is activated, its 
	 * ScreenTitle variable is compared with the Title variable of the Screen.
	 * If they are different, the Screen's Title variable will get a copy
	 * of the Screen's ScreenTitle variable, and the Screen Title Bar will
	 * be redrawn
	 */
	if (w->WScreen->Title != w->ScreenTitle)
	    {
	    w->WScreen->Title = w->ScreenTitle;
	    screenbar(w->WScreen);
	    }
	
	setScreen( w->WScreen );

	/** jimm: i don't know why it was here, but it's smaller now */
	windowMouse( w );

	setMousePointer();

	/* Redraw the border and the border gadgets.  However, if this is the
	 * initial activation of a window, draw all the gadgets.
	 */
	{
	    int draw_flags = DRAWGADGETS_ALL;
	    if ( !initial )
	    {
		draw_flags = DRAWGADGETS_BORDER;
	    }
	    drawEmbossedBorder( w, draw_flags );
	}
    }
}

/*** intuition.library/MenuLend ***/

/* Set up menu lending in the 'fromwindow'.  'towindow' is the
 * window whose menus are to be used, or NULL to disconnect.
 */
LendMenus( fromwindow, towindow )
struct Window *fromwindow;
struct Window *towindow;
{
    /* The XWINDOW()->MenuLend field is examined inside the
     * state machine...
     */
    LOCKSTATE(); /* OK since we're not doing any complex work here... */
    XWINDOW(fromwindow)->MenuLend = towindow;
    UNLOCKSTATE();
}

/*** intuition.library/SetWindowTitles ***/

SetWindowTitles(window, windowtitle, screentitle)
struct Window *window;
UBYTE *windowtitle, *screentitle;
{
    if ((LONG)windowtitle != -1)
    {
	window->Title = windowtitle;
	/* Mark as invalid the cached length of the window title */
	XWINDOW(window)->WinTitleLength = (UWORD)~0;
	updateEmbossedTitle( window );
    }

    /* Peter 1-Feb-91:
     * SimCity calls SetWindowTitles( garbage, -1, valid_scr_title ),
     * so to help them we put in a quick kludge that is likely to
     * tell us when the first parameter is garbage or a window.
     */
    if ( ((LONG)screentitle != -1) &&
	( window->WLayer->Window == window ) )
	{
	window->ScreenTitle = screentitle;
	if (window->Flags & WINDOWACTIVE)
	    {
	    window->WScreen->Title = screentitle;
	    screenbar(window->WScreen);
	    }
	}
}


/*** intuition.library/WindowLimits ***/

/* You could pass in negative minwidth/minheight before, and that
 * snuck through this code because those variables are defined as
 * signed.  You can't turn a window inside-out because something
 * (probably around layer resizing) fails.  Anyways, it might
 * be a good idea to limit negative values to 1.
 */

BOOL WindowLimits(window, minwidth, minheight, maxwidth, maxheight)
struct Window *window;
SHORT minwidth, minheight;
USHORT maxwidth, maxheight;
{
    BOOL goodsizes;

    goodsizes = TRUE;

    if (minwidth)
    {
	if (minwidth > window->Width) goodsizes = FALSE;
	else window->MinWidth = minwidth;
    }

    if (minheight)
    {
	if (minheight > window->Height) goodsizes = FALSE;
	else window->MinHeight = minheight;
    }

    if (maxwidth)
    {
	if (maxwidth < window->Width) goodsizes = FALSE;
	else window->MaxWidth = maxwidth;
    }

    if (maxheight)
    {
	if (maxheight < window->Height) goodsizes = FALSE;
	else window->MaxHeight = maxheight;
    }

    return(goodsizes);
}


/*** intuition.library/MoveWindow ***/

MoveWindow(window, dx, dy)
struct Window *window;
int dx, dy;
{
    struct IBox	newbox;
    /* Can't convert to absolutes upon entry, since a Move-Size succession
     * would then behave differently under 2.0.
     */
    newbox.Left = dx;
    newbox.Top = dy;
    sendISMNoQuick( itCHANGEWIN, window, &newbox,
	CWSUB_MOVEDELTA | CWSUBQ_PROGRAMMATIC );
}



/*** intuition.library/SizeWindow ***/

SizeWindow(window, dx, dy)
struct Window *window;
SHORT dx, dy;
{
    struct IBox	newbox;


    DSW( printf("SizeWindow %lx, %ld/%ld\n", window, dx, dy ) );
    newbox.Width = dx;
    newbox.Height = dy;
    sendISMNoQuick( itCHANGEWIN, window, &newbox,
	CWSUB_SIZEDELTA | CWSUBQ_PROGRAMMATIC | CWSUBQ_SIZING );
}

/*** intuition.library/ChangeWindowBox ***/

ChangeWindowBox( window, l, t, w, h )
struct Window	*window;
{
    struct IBox newbox;

    newbox.Left = l;
    newbox.Top = t;
    newbox.Width = w;
    newbox.Height = h;

    /* will copy box into event */
    sendISMNoQuick( itCHANGEWIN, window, &newbox,
	CWSUB_CHANGE | CWSUBQ_PROGRAMMATIC | CWSUBQ_SIZING );
}


/*** intuition.library/WindowToFront ***/

WindowToFront(window)
struct Window *window;
{
    /* jimm: 5/11/90: better run at input device priority
     * to lessen the chance of somebody sneaking in
     * and trashing a damage list (some people don't use
     * the proper BeginRefresh() protocol, which does
     * the LockLayerInfo() ) protection.
     */
    sendISMNoQuick( itDEPTHWIN, window, NULL, WDEPTH_TOFRONT );
}

/*** intuition.library/MoveWindowInFrontOf ***/

MoveWindowInFrontOf( w, behindw )
struct Window	*w;
struct Window	*behindw;
{
    /* jimm: 5/11/90: better run at input device priority
     * to lessen the chance of somebody sneaking in
     * and trashing a damage list (some people don't use
     * the proper BeginRefresh() protocol, which does
     * the LockLayerInfo() ) protection.
     */
    sendISMNoQuick( itDEPTHWIN, w, behindw, WDEPTH_INFRONTOF );
}



/*** intuition.library/WindowToBack ***/

WindowToBack(window)
struct Window *window;
{
    /* jimm: 5/11/90: better run at input device priority
     * to lessen the chance of somebody sneaking in
     * and trashing a damage list (some people don't use
     * the proper BeginRefresh() protocol, which does
     * the LockLayerInfo() ) protection.
     */
    sendISMNoQuick( itDEPTHWIN, window, NULL, WDEPTH_TOBACK );
}


#if 0
/* Function to find out if a window is obscured by a window higher
 * up.  It needs to walk the layer list for the ordering, but it
 * had better not get confused by requester layers, which aren't
 * deemed to obscure their parent window.
 * Needs to be able to handle a layer with a NULL Window pointer
 * (eg. the bar-layer)
 */
windowObscured( win )
struct Window *win;
{
    struct Layer *l;
    struct Window *win2;
    BOOL obscured = FALSE;

    LockLayerInfo( &win->WScreen->LayerInfo );

    l = win->WScreen->LayerInfo.top_layer;

    while ( l )
    {
	/* If this layer is our window or a requester of our window,
	 * then we know we're not obscured...
	 */
	if ( ( win2 = l->Window ) == win )
	{
	    /* I'm done, not obscured! */
	    break;
	}
	else
	{
	    /* If this layer's window intersects our window, then we
	     * are obscured...
	     */
	    if ( ( win2 ) && intersectBox( (struct IBox *)&win->LeftEdge,
		(struct IBox *)&win2->LeftEdge ) )
	    {
		/* I'm done, obscured */
		obscured = TRUE;
		break;
	    }
	}
	/* Ok, move to the next layer back */
	l = l->back;
    }
    UnlockLayerInfo( &win->WScreen->LayerInfo );

    return( obscured );
}

/* Returns TRUE if the two boxes intersect, FALSE otherwise */ 
intersectBox( box1, box2 )
struct IBox *box1, *box2;
{
    struct Rectangle rect1, rect2;
    boxToRect( box1, &rect1 );
    boxToRect( box2, &rect2 );

    return( !( ( rect1.MinX > rect2.MaxX ) || ( rect2.MinX > rect1.MaxX ) ||
	( rect1.MinY > rect2.MaxY ) || ( rect2.MinY > rect1.MaxY ) ) );
}
#endif

/*** intuition.library/ScrollWindowRaster ***/


void ScrollWindowRaster( win, dx, dy, xmin, ymin, xmax, ymax )
struct Window *win;
LONG dx, dy;
LONG xmin, ymin;
LONG xmax, ymax;
{
    LockLayerInfo( &win->WScreen->LayerInfo );
    LOCKIBASE();
    ScrollRasterBF( win->RPort, dx, dy, xmin, ymin, xmax, ymax );
    /* ScrollRasterBF() only causes damage if this window is simple-refresh
     * and some bits that were obscured by another window were scrolled
     * into view.  The damage region will be that area.
     * There is no point in doing a full gadget refresh, etc., since
     * if the caller scrolled a gadget or window border, the damage
     * region will not include the bulk of the affected area.
     * The basic idea is not to scroll gadget or border imagery.
     *
     * Because of that, we skip doing BorderPatrol(), and just
     * do a quick refresh-handling here...
     */
    if ( ( win->WLayer->Flags & ( LAYERREFRESH|LAYERI_NOTIFYREFRESH ) ) ==
	( LAYERREFRESH|LAYERI_NOTIFYREFRESH ) )
    {
	win->WLayer->Flags &= ~LAYERI_NOTIFYREFRESH;

	if (win->Flags & NOCAREREFRESH)
	{
	    /* Throw away his damage */
	    BeginRefresh(win);
	    EndRefresh(win,TRUE);
	}
	else
	{
	    /* Tell him about damage */
	    windowEvent( win, IECLASS_REFRESHWINDOW, 0 );

	    /* and tell Mr. Console	*/
	    windowEvent( win, IECLASS_EVENT, IECODE_REFRESH );
	}
    }

    UNLOCKIBASE();
    UnlockLayerInfo( &win->WScreen->LayerInfo );
}
