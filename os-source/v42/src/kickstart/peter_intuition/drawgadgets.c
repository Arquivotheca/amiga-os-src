/*** drawgadgets.c ***********************************************************
 *
 *  $Id: drawgadgets.c,v 38.16 93/01/14 14:29:59 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#define D(x)	;
#define DPR(x)	;
#define DG(x)	;	/* ghosting	*/
#define DCG(x)	;
#define DBOX(x)	;

#include "intuall.h"
#include "gadgetclass.h"
#include "imageclass.h"
#include "classusr.h"

#include "newlook.h"

#include "drawgadgets_protos.h"

/*---------------------------------------------------------------------------*/

static int drawGGrunt(struct RastPort * rp,
                      struct Gadget * firstg,
                      struct GListEnv * ge,
                      int numgad,
                      int filter_flags);

static int getContainerPens(struct Gadget * g,
                            struct GadgetInfo * gi,
                            UBYTE * cpens);

/*---------------------------------------------------------------------------*/

/*
 * performs the standard gadget/image/text render bullshit
 *
 * New bullshit:  It turns out that if a non-GADGHIMAGE gadget is
 * drawn while selected, GadgetRender is drawn with IDS_SELECTED.
 * This has been true since V36, but GadTools string gadgets are
 * now broken because they use frameiclass for the frame (which
 * recognizes state) instead of GadTools' own code (which doesn't).
 *
 * It was tempting to disallow IDS_SELECTED if GADGHIMAGE is not set.
 * However, this broke our system gadgets because of the simple
 * way they were created (the only relevant tag was {GA_Image, image}).
 *
 * So stringRender() calls us with suppress_selected = TRUE, so we
 * suppress IDS_SELECTED for string gadgets and strgadclass gadgets.
 */

commonGadgetRender( rp, g, gi, gbox, suppress_selected )
struct RastPort	*rp;
struct Gadget		*g;
struct GadgetInfo	*gi;
struct IBox		*gbox;
LONG			suppress_selected;
{
    void		DrawBorder();
    void		DrawImageState();
    APTR 		visuals;		/* what to draw ...	*/
    void		(*render)();		/* ... and how		*/

    /* figure out what to draw, and how	*/
    visuals = g->GadgetRender;
    if ( (g->Flags & ( SELECTED|GADGHIGHBITS ) ) == ( SELECTED|GADGHIMAGE ) )
    {
	visuals = g->SelectRender;
    }

    render = DrawBorder;
    if ( TESTFLAG( g->Flags, GADGIMAGE ) )
    {
	render = DrawImageState;
    }

    /* do it */
    /* note that we pass extra ignored parametrs to drawborder */

    /* the idea here is that you can use a boopsi image for BOTH
     * the normal and select render.
     */
    (*render)( rp, visuals, gbox->Left, gbox->Top, 
	gadgetDrawState( g, gi, suppress_selected ), gi->gi_DrInfo );
}

/*
 * performs the standard ghosting of the rectangle of a disabled
 * gadget.
 */
commonGadgetTextAndGhost( rp, g, gi, gbox )
struct RastPort		*rp;
struct Gadget		*g;
struct GadgetInfo	*gi;
struct IBox		*gbox;
{
    extern USHORT	BPattern[];
    void (*renderfunc)();
    void PrintIText();
    void DrawImage();

    renderfunc = PrintIText;
    if ( ( g->Flags & GFLG_LABELMASK ) == GFLG_LABELIMAGE )
    {
	renderfunc = DrawImage;
    }

    (*renderfunc)( rp, g->GadgetText, gbox->Left, gbox->Top );

    /* if this Gadget is disabled, but we're not relying on the
     * image to do the ghosting, ghost the select box:
     */
    if ( ( g->Flags & ( GFLG_DISABLED | GFLG_IMAGEDISABLE ) ) ==
	GFLG_DISABLED )
    {
	DG( printf("ghosting gadget: %lx, flags %lx\n", g, g->Flags ) );
	BlastPatternBox( rp, gbox, BPattern, 1, gi->gi_Pens.BlockPen,0,JAM1);
    }
}

gadgetDrawState( g, gi, suppress_selected )
struct Gadget		*g;
struct GadgetInfo	*gi;
{
    WORD selected = TESTFLAG( g->Flags, SELECTED );
    if ( suppress_selected )
    {
	selected = FALSE;
    }

    if ( gi && gi->gi_Window &&
	! TESTFLAG( gi->gi_Window->Flags, WINDOWACTIVE ) &&
	( TESTFLAG( g->Activation, ALLBORDERS ) ||
	  TESTFLAG( g->GadgetType, GTYP_SYSTYPEMASK ) ) ) /* a "system" gadget */
    {
	if ( selected )
	    return ( IDS_INACTIVESELECTED );
	else
	    return ( IDS_INACTIVENORMAL );
    }

    /* Is this a disabled gadget whose image knows how to do its own
     * disabled rendering?
     */
    else if ( (g->Flags & ( GFLG_DISABLED | GFLG_IMAGEDISABLE ) ) ==
	    ( GFLG_DISABLED | GFLG_IMAGEDISABLE ) )
    {
	if ( selected )
	{
	    return( IDS_SELECTEDDISABLED );
	}
	else
	{
	    return( IDS_DISABLED );
	}
    }
    else if ( selected )
    {
	return( IDS_SELECTED );
    }
    else
    {
	return( IDS_NORMAL );
    }
}


/* gets gadget environment and locks it
 * gets (locks) rastport for rendering
 * does refreshGList
 * does drawGGrunt, both refresh and toggle passes
 * unlocks everything it locked.
 * filter_flags can be used to select only border gadgets or only
 * ScrollRaster()-damaged gadgets.
 */
drawGadgets(ptr, gadget, numgad, filter_flags )
APTR ptr;		/* window or screen	*/
struct Gadget *gadget;	/* NULL means abort	*/
int numgad;		/* -1 means all, -2 means from top of list if REQGADGET */
int filter_flags;
{ 
    struct RastPort *rp;	/* for rendering gadgets */
    struct GListEnv genv;
    struct Requester *req;

    if (!gListDomain(gadget, ptr, &genv))
    {
	D(printf("drawG: gLD FAILED!! g %lx ptr %lx n %ld\n",
		gadget, ptr, numgad));
	return;
    }

    /* special handling for requesters */
    /* jimm: 8/26/86: found out that 1.1 RefreshGadgets() did ALL
     * gadgets in requester list 
     */
    if ( req = genv.ge_GInfo.gi_Requester ) 
    {
	/* don't draw in PREDRAWN requesters */
	/* ZZZ: should really draw string gadgets */
	if ( TESTFLAG( req->Flags, PREDRAWN ) )
	{
	    return;
	}

	/* special value of numgad refreshes all gadgets */
	if (numgad == -2) gadget = req->ReqGadget;
    }

    /* get locks all the way down */
    lockGDomain(  &genv  );	/* lock linfo, gadgets, appropriate layers */

    /* already have layers lock from lockGDomain */

    /* one RP lock/copy per list */
    rp = obtainRP( genv.ge_GInfo.gi_RastPort, genv.ge_Layer);

    /* drawGGrunt is allowed to swap the layers around real quick,
     * since everything is all locked up and ours.  This happens
     * for GZZ windows, since gadgets on the list can be in one
     * layer or the other.
     *
     * some gadgets will show up highlighted.
     */
    drawGGrunt(rp, gadget, &genv, numgad, filter_flags );
    D( printf("dG done w/ dGG\n") );

    /* Since drawGGrunt() may play with the layer (when both GZZ and
     * non-GZZ gadgets are on the same list), we need to restore
     * rp->Layer so we don't unlock the wrong layer.
     */
    rp->Layer = genv.ge_Layer;
    freeRP(rp);

    unlockGDomain(&genv);
}


/* draws gadgets in a list, from back to front (!)
 * expects to received safe (i.e. locked) gadget environment
 * and rastport.
 * NEW: 11/29/88: does after-the-fact toggling from back to front, too.
 */
static
drawGGrunt(rp, firstg, ge, numgad, filter_flags )
struct RastPort	*rp;	/* locked rastport for gadget rendering		*/
struct Gadget	*firstg; /* head of list of gadgets			*/
struct GListEnv	*ge;	/* the environment for the list beginning g	*/
int		numgad;	/* max number of gadgets to mess with 		*/
int		filter_flags;
{
    struct Gadget	*g;
    int			pass;
    int			numgadgets;
    struct Hook		*gadgetHook();

    D( printf("dGG: rp %lx, fg %lx, ge %lx, numg %ld\n",
	rp, firstg, ge, numgad ) );

    if (firstg == NULL) return;

    /* first pass is redraw pass (redraw == 1), second is
     * toggle highlighting for gadgets which have SELECTED set
     * and are GADGHCOMP or GADGHBOX
     */
    for ( pass = 0; pass <= 1; ++pass )
    {
	/** note the back to front iteration    **/

	D( printf("dGG pass %ld\n", pass ) );

	/* find last gadget specified */
	g = firstg;
	numgadgets = numgad;
	while (g->NextGadget && --numgadgets) g = g->NextGadget;

	/* work to front	*/
	while (g)
	{
	    D( printf("do gadget %lx\n", g ) );

	    /* Whether to draw the gadget depends on the filter flags and
	     * the gadget:
	     * DRAWGADGETS_ALL:  draw all gadgets
	     * DRAWGADGETS_BORDER:  draw gadgets which are in the border
	     * DRAWGADGETS_DAMAGE:  draw gadgets which have GMORE_SCROLLRASTER set
	     */
	    if ( ( filter_flags == DRAWGADGETS_ALL ) ||

		( ( filter_flags == DRAWGADGETS_BORDER ) &&
		    TESTFLAG( g->Activation, BORDERSNIFF ) ) ||

		( ( filter_flags == DRAWGADGETS_DAMAGE ) &&
		    TESTFLAG( g->Flags, GFLG_EXTENDED ) &&
		    TESTFLAG( ((struct ExtGadget *)g)->MoreFlags, GMORE_SCROLLRASTER ) ) )
	    {
		/* only deal with gadgets you have to:
		 * all, on redraw pass (pass 0)
		 * selected toggle gadgets on toggle pass (pass 1)
		 */
		if ( (pass == 0) || ( (g->Flags & SELECTED) && ISTOGGLE(g)))
		{
		    /* get gadget specific details */
		    gadgetInfo( g, ge );
		    DBOX( dumpGI( "drawGG", &ge->ge_GInfo ) );

		    /* set gadget's proper layer (gi_Layer is set up
		     * in gadgetInfo().  It can change if there are
		     * GZZGADGETs and non-GZZGADGETs on the same
		     * list.
		     */
		    rp->Layer = ge->ge_GInfo.gi_Layer;

		    callHook( gadgetHook( g ), g,
			    GM_RENDER, &ge->ge_GInfo, rp,
			    (pass == 0)? GREDRAW_REDRAW: GREDRAW_TOGGLE );
		}
	    }

	    /* returns null if firstg == g */
	    g = (struct Gadget *) previousLink(firstg, g);
	}
    }
}


callGadgetHook( hook, gad, method, ginfo /* ,...*/ )
struct Hook *hook;
struct Gadget *gad;
ULONG method;
struct GadgetInfo *ginfo;
{
    return ( callGadgetHookPkt( hook, gad, &method ) );
}

callGadgetHookPkt( hook, gad, message )
struct Hook *hook;
struct Gadget *gad;
Msg message;
{
    LONG ghreturn;
    UWORD prior_damage;
    struct GadgetInfo *ginfo;

    assertLock( "callGadgetHook", ISTATELOCK );
    assertLock( "callGadgetHook", IBASELOCK );

    /* We need to read the GadgetInfo from different methods.
     * Unfortunately, the GadgetInfo is not at a fixed place
     * in the method's message.  For OM_NEW, OM_SET, OM_NOTIFY,
     * and OM_UPDATE, the GadgetInfo is in the third position.
     * All other methods must have the GadgetInfo in the second,
     * by example (and now by decree).
     *
     * gpInput is an example structure with GadgetInfo
     * in the second position, so use that structure
     * to install the GadgetInfo.
     */
    ginfo = ((struct gpInput *)message)->gpi_GInfo;

    if ( ( message->MethodID == OM_NEW ) || ( message->MethodID == OM_SET ) ||
	( message->MethodID == OM_NOTIFY ) || ( message->MethodID == OM_UPDATE ) )
    {
	/* opSet is an example structure with GadgetInfo
	 * in the third position, so use that structure
	 * to install the GadgetInfo.
	 */
	ginfo = ((struct opSet *)message)->ops_GInfo;
    }

    /* We've got to detect gadget damage caused when boopsi gadgets
     * use ScrollRaster().  Layers sets the LAYERI_GADGETREFRESH bit
     * when damage occurs.  We pre-clear it prior to invoking a method
     * on a gadget.  If the gadget causes damage through ScrollRaster(),
     * then we can detect that later.
     */
    if ( ( ginfo ) && ( ginfo->gi_Layer ) )
    {
	CLEARFLAG( ginfo->gi_Layer->Flags, LAYERI_GADGETREFRESH );
	prior_damage = TESTFLAG( ginfo->gi_Layer->Flags, LAYERREFRESH );
    }
    ghreturn = callHookPkt( hook, gad, message );

    /* If this window has any GMORE_SCROLLRASTER gadgets, then detect
     * and resolve any boopsi ScrollRaster() damage...
     */
    if ( ( ginfo ) &&
	( ginfo->gi_Layer ) &&
	TESTFLAG( ginfo->gi_Layer->Flags, LAYERI_GADGETREFRESH ) &&
	TESTFLAG( ginfo->gi_Window->MoreFlags, WMF_GADGETSCROLLRASTER ) )
    {
	/* Whether the gadget is in a requester or a window, the
	 * operation is analogous:  BeginRefresh(window) or 
	 * BeginUpdate(reqlayer), then draw all the gadgets with
	 * the DRAWGADGETS_DAMAGE flag, then EndRefresh or EndUpdate
	 * to match.  Discard the damage lists if there was no
	 * damage prior to this operation.
	 */
	APTR pane;			/* Window or layer */
	struct Gadget *first_gad;	/* first gadget to redraw */
	void (*beginfunc)();
	void (*endfunc)();

	void BeginRefresh();
	void EndRefresh();
	void BeginUpdate();
	void EndUpdate();

	CLEARFLAG( ginfo->gi_Layer->Flags, LAYERI_GADGETREFRESH );

	/* Lock stuff and install the damage region */
	LockLayerInfo( &ginfo->gi_Window->WScreen->LayerInfo );
	LOCKGADGETS();	/* before locklayerrom in begin refresh */

	pane = (APTR) ginfo->gi_Window;
	first_gad = ((struct Window *)pane)->FirstGadget;
	beginfunc = BeginRefresh;
	endfunc = EndRefresh;

	if ( ginfo->gi_Requester )
	{
	    /* ReqLayer can't be NULL since gi_Layer isn't! */
	    pane = ginfo->gi_Requester->ReqLayer;
	    first_gad = ((struct Requester *)pane)->ReqGadget;
	    beginfunc = BeginUpdate;
	    endfunc = EndUpdate;
	}

	beginfunc( pane );
	drawGadgets( ginfo->gi_Window, first_gad, ~0, DRAWGADGETS_DAMAGE );

	/* If there was no damage when we began, throw away the
	 * damage lists.  Otherwise, keep them around...
	 */
	endfunc( pane, !prior_damage );
	UNLOCKGADGETS();
	UnlockLayerInfo( &ginfo->gi_Window->WScreen->LayerInfo );
    }
    return( ghreturn );
}


propRender( g, cgp )
struct Gadget		*g;
struct gpRender	*cgp;
{
    struct GadgetInfo	*gi = cgp->gpr_GInfo;
    struct IBox		gadgetbox;
    struct RastPort	*rp = cgp->gpr_RPort;
    struct PGX		mypgx;
    extern UWORD	CPattern[];
    UWORD		leftthick;
    UWORD		topthick;

    UBYTE	containerpens[ NUMCONTAINERPENS ];

    DPR( printf("render hook\n") );

    setupPGX( g, gi, &mypgx );

    if ( cgp->gpr_Redraw == GREDRAW_TOGGLE )
    {
	DPR( printf("ignore _TOGGLE\n"));
	/* do nothing */;
    }
    else if ( cgp->gpr_Redraw == GREDRAW_UPDATE )
    {
	/* This is from NewModifyProp.  */
	DPR( printf("_UPDATE calls renderNewKnob\n"));
	/* Pass in RastPort so renderNewKnob() doesn't call
	 * ObtainGIRPort() (which would nest, which is not good...)
	 */
	renderNewKnob( g, gi, &mypgx, rp );
    }
    else	/* GREDRAW_REDRAW */
    {
	DPR( printf("_REDRAW\n"));
	syncKnobs( g, gi, &mypgx );	
	/* needed?  Yes, at present: NewModP will screw up if
	 * knobs aren't somehow sync'd for newly added gadgets.
	 */

        getContainerPens( g, gi, containerpens );

	/* init thicknesses for newlook borders (maybe override)	*/
	leftthick = PI( g )->LeftBorder;
	topthick = PI( g )->TopBorder;

	gadgetBox( g, gi, &gadgetbox );

	if ( newlookProp( g ) )
	{
	    DPR( printf("calling BPB ..." ) );
	    BlastPatternBox( rp,
		&mypgx.pgx_Container,
		CPattern, 1,
		containerpens[ CP_PATTERN ],
		containerpens[ CP_FILL ],
		JAM2 );
	    DPR( printf("returned.\n"));
	}
	else
	{
	    /* draw a box half as thick as the border	*/
	    leftthick >>= 1;
	    topthick >>= 1;

#if 1	/* jimm:  5/27/90: handle thinner borders	*/
	    DPR( printf("call interiorBox ..." ) );
	    interiorBox( rp, &gadgetbox,
	    /* jimm: 6/5/90: gadgetbox was container, which is wrong. */
		leftthick, topthick,
		containerpens[ CP_FILL ] );
	    DPR( printf("returned.\n"));
#else
	    FillBox( rp, &mypgx.pgx_Container, containerpens[ CP_FILL ] );
#endif
	}

	/* draw border, maybe */
	if ( ! borderlessProp( g ) )
	{
	    DPR( printf("call drawBox for border ..." ) );
	    drawBox( rp, gadgetbox.Left, gadgetbox.Top,
		gadgetbox.Width, gadgetbox.Height,
		containerpens[ CP_BORDER ], JAM1, 

		/* jimm:  5/27/90: handle thinner borders	*/
		leftthick, topthick );
	    DPR( printf("returned.\n"));
	}

	DPR( printf("call drawKnob ..." ) );
	drawKnob( g, gi, rp, &mypgx );	/* if selected, will highlight */
	DPR( printf("returned.\n"));

	/* ZZZ: ghosting doesn't pick up on newlook pen trick */
	DPR( printf("call cGTAG ..." ) );
	commonGadgetTextAndGhost( rp, g, gi, &gadgetbox );
	DPR( printf("returned.\n"));
    }
    /* handle toggle select in refresh routine	*/

    DPR( printf("propRender done ok\n\n"));

    return ( 0 );
}

/*
 * sync up knob position with NewKnob,
 * draw in new place,
 * erase remnants
 */
renderNewKnob( g, gi, pgx, rp )
struct Gadget		*g;
struct GadgetInfo	*gi;
struct PGX		*pgx;
struct RastPort		*rp;
{
    struct IBox oldbox;
    struct Image *gadim;
    struct IBox	*newknob;
#if 1
    UBYTE	containerpens[ NUMCONTAINERPENS ];
#else
    UBYTE	fillpen;
    UBYTE	patternpen;	 /* ~0 if just fill	*/
#endif

    newknob = &pgx->pgx_NewKnob;

    gadim = (struct Image *) g->GadgetRender;

    /* save where-it-was */
    getImageBox(gadim, &oldbox);

    /* see if it moved (new: or changed sizes) */
    if ( oldbox != *newknob )
    {
	LONG obtainedRP = FALSE;

	/* I need to be very careful here.  ObtainGIRPort() may not
	 * be nested, and sometimes I come in here through the GM_RENDER
	 * method, which already has called ObtainGIRPort().  I ensure
	 * that in that case, I'm passed the RastPort to use, and I
	 * don't obtain or free the RastPort in here.
	 */
	if ( !rp )
	{
	    /* lock display environment */
	    rp = ObtainGIRPort( gi );
	    obtainedRP = TRUE;
	}

	/* use structure copy */
	/* update image position fields */
	syncKnobs( g, gi, pgx );

	/* draw knob in new location */
	drawKnob( g, gi, rp, pgx );

        getContainerPens( g, gi, containerpens );
	eraseRemnants( rp, &oldbox, newknob, upperLeft(&pgx->pgx_Container),
		    containerpens[ CP_FILL ], 
		    containerpens[ CP_PATTERN ] );

	if ( obtainedRP )
	{
	    FreeGIRPort( rp );
	}
    }
}


/*
 * gets pens to use for proportional gadgets
 * container.
 */
static
getContainerPens( g, gi, cpens )
struct Gadget		*g;
struct GadgetInfo	*gi;
UBYTE			cpens[ NUMCONTAINERPENS ];
{
    UWORD	*penspec;
    int		prop_in_border;
    struct DrawInfo	*drinfo;

    drinfo =  &XSC(gi->gi_Screen)->SDrawInfo;
    penspec = drinfo->dri_Pens;

    prop_in_border = ( TESTFLAG( g->Activation, ALLBORDERS )
		|| TESTFLAG( g->GadgetType, GZZGADGET ) );

    if ( newlookProp( g ) )
    {
	if (  (! TESTFLAG( drinfo->dri_Flags, DRIF_NEWLOOK ) )
	    || ( drinfo->dri_Depth < 2 ) )
	{
	    /* mono look	*/
	    cpens[ CP_FILL ] = penspec[ BACKGROUNDPEN ];
	    cpens[ CP_PATTERN ] =  penspec[ SHADOWPEN ];

	    if ( prop_in_border &&
	       TESTFLAG( gi->gi_Window->Flags,  WINDOWACTIVE ) )
	    {
		/* background against pen 1	*/
		cpens[ CP_BORDER ] = penspec[ BACKGROUNDPEN ];
	    }
	    else
	    {
		/* the "inactive look", also used inside
		 * window interiors
		 */
		cpens[ CP_BORDER ] = penspec[ SHADOWPEN ];
	    }
	}
	else
	{
	    cpens[ CP_PATTERN ] = cpens[ CP_BORDER ] =
		penspec[ SHADOWPEN ];
	    cpens[ CP_FILL ] = prop_in_border?
			embossedFillPen( gi->gi_Window ): 
			penspec[ BACKGROUNDPEN ];
	}
    }
    else
    {
	/* don't fill out the pattern/fill pens, since they
	 * aren't used by callers for non-newlook gadgets
	 */
	cpens[ CP_FILL ] =  gi->gi_Pens.DetailPen;
	cpens[ CP_PATTERN ] = (UBYTE)~0;	/* makes pattern a no-op */
	cpens[ CP_BORDER ] = gi->gi_Pens.BlockPen;
    }
}


drawKnob( g, gi, rp, pgx )
struct Gadget		*g;
struct GadgetInfo 	*gi;
struct RastPort 	*rp;
struct PGX		*pgx;
{
    int			left, top, knobpen;
    struct Image	*gimage;
    struct IBox		autobox;
    UWORD		freedom;
    int			hthick, vthick;
    int			knob_hit;

    left = pgx->pgx_Container.Left;
    top = pgx->pgx_Container.Top;

    gimage = g->GadgetRender;
    knob_hit = (g->Flags & SELECTED) && (PI(g)->Flags & KNOBHIT);

    if ( PI(g)->Flags & AUTOKNOB )
    {
	left += gimage->LeftEdge;
	top += gimage->TopEdge;

	/* special treatment for "new look" propgadgets in borders */
	if ( newlookProp( g ) )
	{
	    /* The knob is normally rendered in SHINEPEN.
	     * However, an inactive prop gadget sitting in
	     * a window border uses the border fill pen.
	     */
	    knobpen = gi->gi_DrInfo->dri_Pens[ SHINEPEN ];
	    if ( TESTFLAG( g->Activation, ALLBORDERS ) && ( !knob_hit ) )
	    {
		/* Inactive props in window borders
		 * are always done up in shine-pen:
		 */
		knobpen = embossedFillPen( gi->gi_Window );
	    }

	    autobox.Left = left;
	    autobox.Top = top;
	    autobox.Width = gimage->Width;
	    autobox.Height = gimage->Height;

	    freedom =  PI(g)->Flags & (FREEVERT | FREEHORIZ );
	    if (   freedom == 0
		|| freedom == (FREEVERT | FREEHORIZ )
		|| borderlessProp( g ) )
	    {

		DPR( printf("drawing borderless knob block\n"));
		/* draw knob block, even if window inactive	*/
		drawBlock(rp, left, top,
			left + gimage->Width - 1 - FRAMEVTHICK,
			top + gimage->Height - 1 - FRAMEHTHICK, knobpen);

		DPR( printf("drawing embossed block trim\n"));
		embossedBoxTrim( rp, &autobox,
		    gi->gi_DrInfo->dri_Pens[ SHINEPEN ],
		    gi->gi_DrInfo->dri_Pens[ SHADOWPEN ],
		    JOINS_LOWER_RIGHT_WINS );
		goto ALL_DONE;	/* skip the drawBox() */
	    }
	    else if ( freedom == FREEHORIZ )
	    {
		/* Actually, not so sure the drawBox version of 
		 * hthick vs vthick are in sync here
		 */
		hthick = FRAMEHTHICK;
		vthick = 0;
	    }
	    else	/* freedom == FREEVERT */
	    {
		hthick = 0;
		vthick = FRAMEVTHICK;
	    }
#if 1
	    DPR( printf("draw knob block 1\n"));
	    drawBlock( rp, autobox.Left + hthick, autobox.Top + vthick,
		    boxRight( &autobox )  - hthick,
		    boxBottom( &autobox )  - vthick, knobpen );
#endif

	    /* draw vert or horiz edges	*/
	    DPR( printf("draw knob edges box 1\n"));
	    drawBox( rp, autobox.Left, autobox.Top, 
		    autobox.Width, autobox.Height, 
		    gi->gi_DrInfo->dri_Pens[ SHADOWPEN ],
		    JAM1,
		    hthick, vthick);
	}
	else
	{
	    knobpen = gi->gi_Pens.BlockPen;

	    /* jimm: 1/6/88: no toggle 1 bitplane autoknob	*/
	    if ( ( knob_hit ) && ( rp->BitMap->Depth > 1 ) )
	    {
		/* draw the Block with knobpen complemented */
		knobpen = ~knobpen;
	    }

	    DPR( printf("draw oldstyle knob\n"));
	    drawBlock(rp, left, top, left + gimage->Width - 1,
			top + gimage->Height - 1, knobpen );
	}
    }
    else
    {
	ULONG state = IDS_NORMAL;
	/* We need to know if this gadget is currently SELECTED and KNOBHIT,
	 * and also what highlighting is chosen.  This is a compact-code
	 * way of doing that.
	 */
	ULONG sel_hi = g->Flags & ( SELECTED | GADGHIGHBITS );

	if ( !TESTFLAG( PI(g)->Flags, KNOBHIT ) )
	{
	    sel_hi = 0;
	}

	/* highlight using alternate image unless null or no knobhit*/
	if ( ( sel_hi == ( SELECTED | GADGHIMAGE ) ) && ( g->SelectRender ) )
	{
	    gimage = (struct Image *) g->SelectRender;
	    state = IDS_SELECTED;
	}

	DrawImageState(rp, gimage, left, top, state, NULL);
	if ( sel_hi == ( SELECTED | GADGHCOMP ) )
	{
	    xorbox(rp, left + gimage->LeftEdge, top + gimage->TopEdge, 
		    gimage->Width, gimage->Height);
	}
    }
ALL_DONE:
    DPR( printf("drawKnob done\n"));
    return;
}



