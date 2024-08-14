
/*** gadgets.c ***************************************************************
 *
 *  user-callable gadget routines
 *
 *  $Id: gadgets.c,v 38.12 93/01/14 14:21:32 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#define D(x)	;

#include "intuall.h"
#include "imageclass.h"
#include "gadgetclass.h"

struct  Requester   *findGadgetRequest();
USHORT AddGList();
USHORT RemoveGList();

/*** intuition.library/AddGadget ***/

/* stub now calls AddGList() directly */


/*** intuition.library/AddGList ***/

USHORT AddGList(window, gadget, pos, numgad, requester)
struct Window *window;
struct Gadget *gadget;
UWORD pos;
UWORD numgad;
/* NB: AddGadget() historically calls here with garbage in the
 * requester parameter.  We must not look at it unless REQGADGET
 * is set!
 */
struct Requester *requester;
{
    struct Gadget *g, *lastg;
    SHORT realpos = 0;

    LOCKGADGETS();

    /* Take care of initialization issues when gadgets
     * are added to the window.  This includes setting
     * or clearing GFLG_IMAGEDISABLE according to what
     * the gadget's image says, and sending the initial
     * GM_LAYOUT method to GREL_ gadgets.
     */
    initGadgets( window, gadget, numgad );

#if 1
    if ( gadget->GadgetType & REQGADGET )
    {
	g = (struct Gadget *) &requester->ReqGadget;
    }
    else	/* window gadget */
    {
	g = (struct Gadget *) &window->FirstGadget;
	sniffWindowGadgets( window, gadget, numgad );
	/* check for border redraw conditions */
    }
#else
    /* find first gadget pointer , window or requester */
    g = (struct Gadget *) ((gadget->GadgetType & REQGADGET)? 
	&requester->ReqGadget: &window->FirstGadget);
#endif

    /* find pointer to previous gadget, real position */
    g = (struct Gadget *) nthThing(g, pos, &realpos);

    /* find last gadget in list passed in */
    lastg = (struct Gadget *) nthThing(gadget, numgad - 1, NULL);

    /* link in sublist */
    lastg->NextGadget = g->NextGadget;
    g->NextGadget = gadget;

    UNLOCKGADGETS();
    return(realpos);
}


/*** intuition.library/RemoveGadget ***/

/* stub now goes directly into RemoveGList() */


/*** intuition.library/RemoveGList ***/

USHORT RemoveGList(window, gadget, numgad)
struct Window *window;
struct Gadget *gadget;
WORD numgad;
{
    struct Gadget **firstgadget;
    struct Requester *req;
    SHORT pos = -1;

    /* protect from garbage in, as before */
    if (gadget == NULL) return ( -1 );

    /* find the requester this gadget is hooked up in, if
     * any.
     */

    if (gadget->GadgetType & REQGADGET)
    {
	req = findGadgetRequest(gadget, window);

	if (req) firstgadget = &req->ReqGadget;
	else return ( -1 );
    }
    else
    {
	firstgadget = &window->FirstGadget;
    }

    D( printf("RGL: doISM\n") );

    pos =  doISM( itREMOVEGAD, gadget, firstgadget, numgad );
    D( printf("RGL: doISM returned\n") );
    return ( pos );
}

/*
 * remove gadget sublist.
 * passed gadget of reference, first gadget in list that
 * it is a member of, and numgad (-1 means till end).
 *
 * return old position, or -1 
 */
delinkGadgetList( gadget, firstgadget, numgad )
struct Gadget	*gadget;
struct Gadget	*firstgadget;
{
    SHORT	pos;
    struct Gadget	*lastg;
    struct Gadget	*prevgadget;

    LOCKGADGETS();

    /* find previous gadget, calculate position */
    prevgadget = (struct Gadget *) previousThing(gadget, firstgadget, &pos);

    if ( prevgadget ) 
    {
	/* find last gadget in list passed in */
	lastg = (struct Gadget *) nthThing(gadget, numgad - 1, NULL);

	prevgadget->NextGadget = lastg->NextGadget;
    }
    else pos = -1;

    UNLOCKGADGETS();

    return ( pos );
}

/*
 * if gadget is in 'numgad' list starting with firstgadget
 * return true
 */
inGList( gadget, firstgadget, numgad )
struct Gadget	*gadget;
struct Gadget	*firstgadget;
{
    while ( numgad-- && firstgadget )
    {
	if ( firstgadget == gadget ) return ( TRUE );
	firstgadget = firstgadget->NextGadget;
    }
    return ( 0 );
}
/*** intuition.library/RefreshGadgets ***/

/* assembler stub calls drawGadgets() directly */


/*** intuition.library/RefreshGList ***/

/* assembler stub calls drawGadgets() directly */


/*** intuition.library/OnGadget ***/

OnGadget(gadget, ptr, req)
struct Gadget *gadget;
APTR ptr;
struct Requester *req;
{
    gadget->Flags &= ~GADGDISABLED;

    /* jimm: refresh ALL requester gadgets */
    drawGadgets( ptr, gadget, -2, DRAWGADGETS_ALL );
}

/*** intuition.library/OffGadget ***/

OffGadget(gadget, ptr, req)
struct Gadget *gadget;
APTR ptr;
struct Requester *req;
{
    gadget->Flags |= GADGDISABLED;

    /* jimm: refresh ALL requester gadgets */
    drawGadgets( ptr, gadget, -2, DRAWGADGETS_ALL );
}

/*** intuition.library/ModifyProp ***/

/*** intuition.library/NewModifyProp ***/

#if 000
/* Peter 1-Dec-90:  Some flags should not be altered.  Since I'm
 * a sucker for compatibility with perverse software, I'll chicken
 * out and only block PROPNEWLOOK :-(
 */
#define HANDS_OFF ( PROPNEWLOOK /* | PROPBORDERLESS | AUTOKNOB*/ )

NewModifyProp(gadget, aptr, req, flags, hpot, vpot, hbody, vbody, numgad)
struct Gadget *gadget;
APTR aptr;
struct Requester *req;
USHORT flags, hpot, vpot, hbody, vbody;
WORD numgad;	/* passed as -2 from ModifyProp interface	*/
{
    struct PropInfo	 *pinfo;
    struct GListEnv	genv;
    struct PGX		mypgx;

    pinfo = (struct PropInfo *)gadget->SpecialInfo;

    pinfo->Flags = (pinfo->Flags & HANDS_OFF) | (flags & ~HANDS_OFF) ;
    pinfo->HorizPot = hpot;
    pinfo->VertPot = vpot;
    pinfo->HorizBody = hbody;
    pinfo->VertBody = vbody;

    /* Jimm optimized things so that NMP() of a single gadget did
     * incremental updating instead of complete re-rendering.
     * (numgad passed as -2 from _modifyprop)
     * Peter 23-Mar-92: Now do full-redraw if the single gadget is
     * disabled, because NMP() of a disabled gadget was erasing
     * the ghosting pattern.
     */
    if ( ( numgad != 1 ) || ( TESTFLAG( gadget->Flags, GFLG_DISABLED ) ) )
    {
	drawGadgets( aptr, gadget, numgad, DRAWGADGETS_ALL );
    }
    else
    {
	if ( !gListDomain(gadget, aptr, &genv))
	{
	    return;
	}
	gadgetInfo( gadget, &genv  );

	setupPGX( gadget, &genv.ge_GInfo, &mypgx );

	/* ZZZ: relying on NewKnob being set up */

	renderNewKnob( gadget, &genv.ge_GInfo, &mypgx, NULL );
    }
}
#else
/* Peter 1-Dec-90:  Some flags should not be altered.  Since I'm
 * a sucker for compatibility with perverse software, I'll chicken
 * out and only block PROPNEWLOOK :-(
 */
#define HANDS_OFF ( PROPNEWLOOK /* | PROPBORDERLESS | AUTOKNOB*/ )
struct PropPacket
{
    UWORD Flags;
    UWORD HorizPot;
    UWORD VertPot;
    UWORD HorizBody;
    UWORD VertBody;
    UWORD NumGad;
};

NewModifyProp(gad, win, req, flags, hpot, vpot, hbody, vbody, numgad)
struct Gadget *gad;
struct Window *win;
struct Requester *req;
USHORT flags, hpot, vpot, hbody, vbody;
WORD numgad;	/* passed as -2 from ModifyProp interface	*/
{
    struct PropPacket pp;

    lockFree("NewModifyProp", LAYERINFOLOCK );
    lockFree("NewModifyProp", LAYERROMLOCK );
    lockFree("NewModifyProp", IBASELOCK );

    pp.Flags = flags;
    pp.HorizPot = hpot;
    pp.VertPot = vpot;
    pp.HorizBody = hbody;
    pp.VertBody = vbody;
    pp.NumGad = numgad;

    doISM( itMODIFYPROP, win, gad, &pp );
}

IModifyProp( gad, win, pp )
struct Gadget *gad;
struct Window *win;
struct PropPacket *pp;
{
    struct PropInfo	 *pinfo;
    struct GListEnv	genv;
    struct PGX		mypgx;

    pinfo = (struct PropInfo *)gad->SpecialInfo;

    pinfo->Flags = (pinfo->Flags & HANDS_OFF) | (pp->Flags & ~HANDS_OFF) ;
    pinfo->HorizPot = pp->HorizPot;
    pinfo->VertPot = pp->VertPot;
    pinfo->HorizBody = pp->HorizBody;
    pinfo->VertBody = pp->VertBody;

    /* Jimm optimized things so that NMP() of a single gadget did
     * incremental updating instead of complete re-rendering.
     * (numgad passed as -2 from _modifyprop)
     * Peter 23-Mar-92: Now do full-redraw if the single gadget is
     * disabled, because NMP() of a disabled gadget was erasing
     * the ghosting pattern.
     */
    if ( ( pp->NumGad != 1 ) || ( TESTFLAG( gad->Flags, GFLG_DISABLED ) ) )
    {
	drawGadgets( win, gad, pp->NumGad, DRAWGADGETS_ALL );
    }
    else
    {
	if ( !gListDomain(gad, win, &genv))
	{
	    return;
	}
	gadgetInfo( gad, &genv  );

	setupPGX( gad, &genv.ge_GInfo, &mypgx );

	/* ZZZ: relying on NewKnob being set up */

	renderNewKnob( gad, &genv.ge_GInfo, &mypgx, NULL );
    }
}
#endif

/* If this many pixels of a gadget which intersects the border is in
 * the window clear, then we don't bother BORDERSNIFFing it, since
 * we deem that enough of it is visible.
 */
#define GAD_MINVISIBLE	(4)


sniffWindowGadgets( w, g, numgad )
struct Window	*w;
struct Gadget	*g;
{
    struct GListEnv	genv;
    struct IBox		box;

    for ( ; numgad-- && g; g = g->NextGadget )
    {
	/* By default, a gadget won't be BORDERSNIFFed */
	CLEARFLAG( g->Activation, BORDERSNIFF );
	/* Gadgets that are declared as being in a window border
	 * or are system gadgets (Gadget->Type & GTYP_SYSTYPEMASK) or are
	 * GZZGADGETs are always to be redrawn.  Therefore, we
	 * mark them as BORDERSNIFFed.
	 */
	if ( TESTFLAG( g->Activation, (LEFTBORDER|RIGHTBORDER|TOPBORDER|BOTTOMBORDER) )
	    || TESTFLAG( g->GadgetType, GZZGADGET | GTYP_SYSTYPEMASK ) )
	{
	    SETFLAG( g->Activation, BORDERSNIFF );
	    continue;
	}
	/* Non-GZZGADGETs in a GIMMEZEROZERO window should never
	 * be BORDERSNIFFed. (Note by this point, we only have
	 * non-GZZGADGETs to consider).
	 * Peter 18-Jan-91: until recently, we used to never BORDERSNIFF
	 * gadgets in a BORDERLESS window, but even a BORDERLESS window
	 * can have borders, if somebody sets one of the xxxBORDER flags
	 * in one of their gadgets.  (Fixes Appetizer/Write)
	 */
	if ( TESTFLAG( w->Flags, GIMMEZEROZERO ) )
	{
	    continue;
	}

	/* get (relatively defined) box for gadget */
	if ( ! gListDomain( g, w, &genv))
	{
	    continue;
	}

	gadgetInfo( g, &genv  );
	gadgetBox( g, &genv.ge_GInfo, &box );

	/* now check box against window border dims, 
	 * and make the BORDERSNIFF bit correct.
	 *
	 * far borders take longer to check, but
	 * near borders are extremely rare.
	 *
	 * Peter 17-Aug-90: If more than GAD_MINVISIBLE pixels stick
	 * out into the window proper (or if that many pixels are beyond
	 * the bottom or right of the window, then we declare that this
	 * gadget doesn't need to be BORDERSNIFF, since it's either
	 * sufficiently reachable or so far outside the window as
	 * to be considered gone.
	 *
	 * Peter 12-Sep-90: Some prog's like Transcript use a 1x1 gadget
	 * (at x=y=0) with a solid color image to do color backfills.
	 * For them, we don't BORDERSNIFF any gadgets of size 1x1 or smaller.
	 */

	if (
	    ( ( box.Width > 1 ) || ( box.Height > 1 ) )
	    && (
	    (( box.Left >= w->Width - w->BorderRight - GAD_MINVISIBLE )
	     && ( box.Left + box.Width <= w->Width + GAD_MINVISIBLE ))
	    ||
	    (( box.Top >= w->Height - w->BorderBottom - GAD_MINVISIBLE )
	     && ( box.Top + box.Height <= w->Height + GAD_MINVISIBLE ))
	    ||
	    ( ( w->BorderLeft - box.Left ) > ( box.Width - GAD_MINVISIBLE ) )
	    ||
	    ( ( w->BorderTop - box.Top ) > ( box.Height - GAD_MINVISIBLE ) )
	       )
	    )
	{
	    SETFLAG( g->Activation, BORDERSNIFF );
	}
    }
}


/* Take care of initialization issues when gadgets
 * are added to the window.  This includes setting
 * or clearing GFLG_IMAGEDISABLE according to what
 * the gadget's image says, and sending the initial
 * GM_LAYOUT method to GREL_ gadgets.  Also sets
 * WMF_GADGETSCROLLRASTER if any gadget has the
 * GMORE_SCROLLRASTER flag set.
 */
initGadgets( win, g, numgad )
struct Window	*win;
struct Gadget	*g;
LONG		numgad;
{
    struct Image *image;
    struct GListEnv genv;
    LONG gotdomain;

    gotdomain = gListDomain( g, win, &genv );

    for ( ; numgad-- && g; g = g->NextGadget )
    {
	/* Force GFLG_IMAGEDISABLE off */
	CLEARFLAG( g->Flags, GFLG_IMAGEDISABLE );

	/* Is GadgetRender a non-NULL image? */
	if ( TESTFLAG( g->Flags, GFLG_GADGIMAGE ) && ( image = g->GadgetRender ) )
	{
	    /* Only perform GetAttr() if this is a boopsi image.
	     * Peter 1-Sep-92:  ProVector and others may be passing
	     * an uninitialized struct Image for an AUTOKNOB prop gadget.
	     * This was OK because Intuition never used fields it didn't
	     * itself initialize.
	     *
	     * Just testing for CUSTOMIMAGEDEPTH is not good enough,
	     * because the AUTOKNOB image could happen to have that value
	     * in the Depth field.  So we never ask this of imagery of
	     * AUTOKNOB PROPGADGETs.
	     */
	    if ( ( image->Depth == CUSTOMIMAGEDEPTH ) &&
		( ( ( g->GadgetType & GTYP_GTYPEMASK ) != GTYP_PROPGADGET ) ||
		( !TESTFLAG( PI(g)->Flags, AUTOKNOB ) ) ) )
	    {
		ULONG answer = 0;

		/* Does it support its own disabling? */
		GetAttr( IA_SupportsDisable, image, &answer );
		if ( answer )
		{
		    SETFLAG( g->Flags, GFLG_IMAGEDISABLE );
		}
	    }
	}

	/* If this gadget is GREL, then we must send it an initial
	 * GM_LAYOUT method, so it can establish its size.
	 */
	if ( ( gotdomain ) && ( TESTFLAG( g->Flags,
	    GFLG_RELRIGHT|GFLG_RELBOTTOM|GFLG_RELWIDTH|GFLG_RELHEIGHT|GFLG_RELSPECIAL) ) )
	{
	    gadgetInfo( g, &genv );
	    /* We send a GM_LAYOUT method to the gadget, expecting
	     * it to fill in its bounds field.  For gadgets that
	     * don't handle this method, nothing happens.
	     */
	    callHook( gadgetHook( g ), g, GM_LAYOUT, &genv.ge_GInfo, 1 );
	}	

	/* ScrollRaster() detection for boopsi gadgets is important,
	 * but it's incompatible with applications calling ScrollRaster()
	 * (really any non-Intuition damage-causing function).  We
	 * set WMF_GADGETSCROLLRASTER if we find any GMORE_SCROLLRASTER
	 * gadgets, and perform this incompatible handling for windows
	 * that have this flag.
	 */
	if ( TESTFLAG( g->Flags, GFLG_EXTENDED ) &&
	    TESTFLAG( ((struct ExtGadget *)g)->MoreFlags, GMORE_SCROLLRASTER ) )
	{
	    SETFLAG( win->MoreFlags, WMF_GADGETSCROLLRASTER );
	}
    }
}
