/*** zoom.c ***************************************************************
 *
 *  zoom.c -- expand and shrink windows, with reposition
 *
 *  $Id: zoom.c,v 38.12 93/02/15 19:07:01 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "intuall.h"

#define D(x)		;
#define DMEM(x)		;
#define D2(x)		;
#define DZIP2(x)	;
#define DZIP(x)		;
#define DREQ(x)		;
#define DFAIL(x)	;
#define DICW(x)		;	/* IChangeWindow() messaging */
/*** intuition.library/ZipWindow ***/

ZipWindow( window )
struct Window	*window;
{
#if 1
    /* jimm: 5/11/90: better run at input device priority
     * to lessen the chance of somebody sneaking in
     * and trashing a damage list (some people don't use
     * the proper BeginRefresh() protocol, which does
     * the LockLayerInfo() ) protection.
     */
    /* use input device priority	*/
    sendISMNoQuick( itZOOMWIN, window, 0, FALSE );	/* no sizeverify */
#else
    sendISM( itZOOMWIN, window );
#endif
}

IZoomWindow( window, interactive )
struct Window		*window;
BOOL			interactive;	/* TRUE if user-initiated.  Then we
					 * must send NEWSIZE if SIZEVERIFY.
					 * If not, we must send NEWSIZE
					 * since it was programmer initiated.
					 * (Always send NEWSIZE if size changed.)
					 */

{
    struct IBox *w_newbox;
    struct IBox	*w_oldbox;
    struct IBox zoomto;
    struct IBox oldbox;

    /* Peter 11-Mar-91: In order to help IHelp and other "hostile"
     * window-movers, we try to ensure we have a real window here
     */
    if ( !knownWindow( window ) ) return;

    w_oldbox = &XWINDOW( window )->UnzoomBox;
    w_newbox = &XWINDOW( window )->ZoomBox;
    if ( TESTFLAG( window->Flags, ZOOMED ) )
    {
	w_oldbox = &XWINDOW( window )->ZoomBox;
	w_newbox = &XWINDOW( window )->UnzoomBox;
    }

    zoomto = *w_newbox;

    /* save current coords  (flying cast) */
    oldbox = *( (struct IBox * ) &window->LeftEdge );

    /* New for V39: If the ZoomBox Left and Top are both -1,
     * then we do size-only zooming, where the upper-left
     * doesn't change between zoomed/unzoomed state,
     * unless necessary to get a legal position.
     *
     * Flying cast to check left/top for being ~0, and
     * to copy both in one shot.
     */
    if ( *((ULONG *)&zoomto.Left) == ~0 )
    {
	*((ULONG *)&zoomto.Left) = *((ULONG *)&oldbox.Left);
	*((ULONG *)&oldbox.Left) = ~0;
    }

    /* make it happen	*/
    D( dumpBox( "IZW: zoom to",  &zoomto ) );
    D( dumpBox( "IZW: oldbox",  &oldbox ) );

    if ( IChangeWindow( window, &zoomto,
	( interactive ? (CWSUB_CHANGE | CWSUBQ_SIZING) :
	(CWSUB_CHANGE | CWSUBQ_PROGRAMMATIC | CWSUBQ_SIZING) ) ) )
    {
	window->Flags ^= ZOOMED;
	*w_oldbox = oldbox;
    }
}


/* returns min/max dimensions for a window, without changing its position
 */
windowSizeLimits( w, rect )
struct Window	*w;
struct Rectangle *rect;
{

#if 0
    /* two problems: you cannot inhibit SizeWindow calls just
     * because there is no WINDOWSIZING gadget, and secondly,
     * I don't see how this code worked.  Maybe there used
     * to be a return or something
     */
    if ( ! ( w->Flags & WINDOWSIZING ) )
    {
	rect->MinX = rect->MaxX = w->Width;
	rect->MinY = rect->MaxY = w->Height;
    }
#endif

    /* respect min/max window limits only if WINDOWSIZING */
    if (  w->Flags & WINDOWSIZING )
    {
	/* take the min's as given	*/
	rect->MinX = w->MinWidth;
	rect->MinY = w->MinHeight;

	/* handle values near signed maximum	*/
	if ( ( ( rect->MaxX = w->MaxWidth ) & 0x8000 )
	    || ( rect->MaxX > REALBIG ) )
	{
	    rect->MaxX = REALBIG;
	}

	if ( ( ( rect->MaxY = w->MaxHeight ) & 0x8000 )
	    || ( rect->MaxY > REALBIG ) ) {
	    rect->MaxY = REALBIG;
	}

	D2( dumpRect( "window max/min dims", rect ) );

	/* account for size of screen and position  */
	rect->MaxX = imin( rect->MaxX, w->WScreen->Width - w->LeftEdge );
	rect->MaxY = imin( rect->MaxY, w->WScreen->Height - w->TopEdge );
    }
    else
    {
	rect->MinX = rect->MinY = 1;
	rect->MaxX = w->WScreen->Width - w->LeftEdge;
	rect->MaxY = w->WScreen->Height - w->TopEdge;
    }
}

/*
 * legal positions for a window, without changing its size
 */
windowPositionLimits( w, rect )
struct Window	*w;
struct Rectangle	*rect;
{
     rect->MinY = rect->MinX = 0;
     rect->MaxX = w->WScreen->Width - w->Width;
     rect->MaxY = w->WScreen->Height - w->Height;
}

/*
 * modifies box to specify legal position/dimensions
 * for a window within its screen
 * this will work unless window MinWidth/Height larger than
 * screen, or larger than window MaxWidth/Height
 */
legalWindowBox( window, box )
struct Window	*window;
struct IBox	*box;
{
    struct Point minsize, maxsize;
    struct Rectangle limitrect;

    D( dumpBox( "lWR input", box ) );

    /* will construct limit rect to include all  legal left/top positions */
    limitrect.MinX = limitrect.MinY = 0;

    if ( window->Flags & WINDOWSIZING )
    {
	/* flying cast - get w & h in one shot */
	minsize = *( (struct Point *) &window->MinWidth );
	if ( ( ( maxsize.X = window->MaxWidth ) & 0x8000 ) ||
	    ( maxsize.X > REALBIG ) )
	{
	    maxsize.X = REALBIG;
	}
	if ( ( ( maxsize.Y = window->MaxHeight ) & 0x8000 ) ||
	    ( maxsize.Y > REALBIG ) )
	{
	    maxsize.Y = REALBIG;
	}
    }
    else
    {
	/* Peter 16-Aug-90: No size gadget means Window->Min/Max
	 * don't apply.
	 */
	minsize.X = minsize.Y = 1;
	maxsize.X = maxsize.Y = REALBIG;
    }
    maxsize.X = imin( window->WScreen->Width, maxsize.X );
    maxsize.Y = imin( window->WScreen->Height, maxsize.Y );

    box->Width = imax( imin( box->Width, maxsize.X ), minsize.X );
    box->Height = imax( imin( box->Height, maxsize.Y ), minsize.Y );

    limitrect.MaxX = window->WScreen->Width - box->Width;
    limitrect.MaxY = window->WScreen->Height - box->Height;

    D( dumpRect( "lWR: limitrect", &limitrect ) );

    limitPoint( &box->Left, &limitrect);	/* flying cast to "Point" */

    D( dumpBox( "lWR result", box ) );
}

#if 0 /* put this inline */
IsZoomed( window )
struct Window	*window;
{
    /***
    ZZZZZ;
    test a bit to make a guess
    see if already same size as guess
    perhaps detect "first time", decide whether he's closer
	to zoomed or unzoomed dimensions
    ****/
    D( printf("IsZoomed: window: %lx, value: %lx\n", window,
	window->Flags & ZOOMED ) );
    return ( window->Flags & ZOOMED );
}

#endif


/***************************/

#define LET_WINDOW	(1)
#define	LET_G00		(2)
#define LET_REQUEST	(3)

struct LayerEntry {
    struct Layer	*le_Layer;
    struct Requester	*le_Requester;
    WORD		le_Type;
};

/* puts layers associated with window in a list
 * returns count
 *
 * NOTE: callers are depending on the order this list is created,
 * which is front to back
 *
 */
int
windowLayerList( w, list, countnulllayers )
struct	Window	*w;
struct	LayerEntry *list;
BOOL	countnulllayers;

{
    struct	Requester *req;
    int		count = 1;	/* always count window layer	*/
    
    /* requester layers */
    for ( req = w->FirstRequest; req && (count < MAXWINDOWLAYERS-1);
	req = req->OlderRequest )
    {
	/* Peter 28-Sep-90:  handle requesters with no layer */
	if ((list->le_Layer = req->ReqLayer) || (countnulllayers))
	{
	    list->le_Requester = req;
	    list->le_Type	= LET_REQUEST;
	    list++;
	    count++;
	}
    }

    /* normal window layer */
    /** jimm: 11/18/85 layer change **/
    list->le_Layer = w->WLayer;
    list->le_Type  = LET_WINDOW;	/* may get patched up below */
    list++;
    /* this layer already counted	*/

    /* G00 border layer */
    if (w->Flags & GIMMEZEROZERO)
    {
	list->le_Layer = w->BorderRPort->Layer;
	list->le_Type	= LET_WINDOW;
	list[ -1 ].le_Type	= LET_G00;	/* patch up previous one */

	list++;
	count++;
    }

    D( printf( "wLL: count: %ld\n", count ) );

    return ( count );
}

/*
 * move/size all window layers in one fell swoop.
 *
 * The criterion for sending NEWSIZE or CHANGEWINDOW messages
 * requires some explanation.
 *
 * The goal is to, "when possible", avoid sending these messages
 * if nothing changed.  This might prevent some programs from
 * doing unnecessary size-based refresh because they fail to
 * check for changed size.  Note that the new drag-cancel feature
 * means that it's easy for the user to do a null operation.
 *
 * It turns out that there are some involved cases where we must
 * send the messages along, regardless of any changes.
 *
 * We always send NEWSIZE (or CHANGEWINDOW) events if the size (or
 * size/position) changes.
 *
 * In the case when nothing changed, we still need to send these messages
 * in two cases, for compatibility reasons and because in these cases,
 * the application can know that sizing had begun.
 *
 * 1)  When the action was initiated through an Intuition function call
 *     such as MoveWindow(), SizeWindow(), ChangeWindowBox().
 * 2)  When action was a user-initiated sizing (or zooming) operation on a
 *     window that has SIZEVERIFY set.
 *
 * subcommand  is one of CWSUB_* in ism.h
 */
IChangeWindow( w, newbox, subcommand )
struct Window	*w;
struct IBox	*newbox;
{
    int			i;
    struct LayerEntry	*le;
    struct LayerEntry	layerlist[ MAXWINDOWLAYERS ];
    int	 		count;
    BOOL		success = FALSE;
    struct IBox		innerbox;	/* also serves as temp box */
    struct IBox		absbox;
    struct Point	dimchange;
    BOOL		sizechanged;
    BOOL		poschanged;


    if  ( ! knownWindow( w ) ) return;

    LockLayerInfo( &w->WScreen->LayerInfo);

    DICW( printf("ICW: subcommand %lx\n", subcommand ) );
    switch ( subcommand & CWSUB_SUBCOMMAND )
    {
    case CWSUB_SIZEDELTA:
	/* convert deltas to absolutes	*/
	windowBox( w, &absbox );
	absbox.Width += newbox->Width;
	absbox.Height += newbox->Height;

	/* make legal (temporarily use innerbox )	*/

	windowSizeLimits( w, &innerbox );

	D2( dumpBox( "cwsub_sizedelta, requested absbox", &absbox ) );
	D2( dumpRect( "cwsub_sizedelta, window size limits", &innerbox ) );
	limitPoint( &absbox.Width, &innerbox );

	D2( dumpBox( "cwsub_sizedelta, resulting absbox", &absbox ) );

	/* use the absolutes	*/
	newbox = &absbox;
	break;

    case CWSUB_MOVEDELTA:
	/* convert deltas to absolutes	*/
	windowBox( w, &absbox );
	absbox.Left += newbox->Left;
	absbox.Top += newbox->Top;

	/* make legal (temporarily use innerbox )	*/
	windowPositionLimits( w, &innerbox );

	/* jimm: 2/16/90: oops, had &absbox.Top here.
	 * what an idiot.  I am sentenced to one month
	 * using a strongly-typed language.  Yeah, right.
	 */
	limitPoint( &absbox.Left, &innerbox );

	/* use the absolutes	*/
	newbox = &absbox;
	break;

    case CWSUB_CHANGE:
	/* make this a legal thing	*/
	legalWindowBox( w, newbox );  /* might be redundant for Move/Size */
	break;

    default:
	UnlockLayerInfo( &w->WScreen->LayerInfo );
	return;
    }


    /* save size deltas. */
    dimchange.X = newbox->Width - w->Width;
    dimchange.Y = newbox->Height - w->Height;
    sizechanged = (dimchange.X || dimchange.Y);
    poschanged = ( ( newbox->Left != w->LeftEdge ) ||
	( newbox->Top != w->TopEdge ) );

    DICW( printf( "dimchange.X = %ld, dimchange.Y = %ld\n",
	dimchange.X, dimchange.Y ) );
    DICW( printf( "sizechanged = %lx, poschanged = %lx\n",
	sizechanged, poschanged ) );

    /* I don't need the previous values in innerbox anymore */
    innerWindowBox( w, newbox, &innerbox );

    /* get list of layers that have to be changed, including those
     * requesters whose layer is currently NULL
     */
    i = count =  windowLayerList( w, layerlist, TRUE );

    /* given new window box, move/size layers to suit	*/
    for ( le = layerlist; i; i--, le++ )
    {
	D( printf("ICW: do window layer le %lx\n", le ) );
	if ( ! doWindowLayer( le, newbox, &innerbox ) )
	{
	    goto BAIL;
	}
    }

    success = TRUE;
    goto OK;

BAIL:
    D( printf("ICW: bail\n") );

    /* Try to invert the layer operation for layers
     * changed so far
     */

    /* reset window box to old position/dims	*/
    *newbox = *((struct IBox  *) &w->LeftEdge);
    innerWindowBox( w, newbox, &innerbox );

    /* back out, in reverse order	*/
    for ( le-- ; i < count; ++i )
    {
	D( printf("undo window layer le %lx\n") );
	doWindowLayer( le, newbox, &innerbox );
    }

OK:
    D( printf("ICW: OK\n") );

    if ( success )
    {
	*((struct IBox  *) &w->LeftEdge) = *newbox;
	w->GZZWidth = innerbox.Width;
	w->GZZHeight = innerbox.Height;
	/* Mark as invalid the cached length of the window title */
	XWINDOW(w)->WinTitleLength = ~0;
    }
    /* Peter 17-Oct-90:
     * Since the window may have moved, we'd better update
     * its idea of MouseX and Y.
     */
    windowMouse( w );

    DZIP(printf("after operation, flags %lx\n",
	w->WLayer->Flags & LAYERREFRESH));
    DZIP( printf("before sizeDamage\n"));
    DZIP2( Debug() );

    /* If sizing is involved (includes zoom), we always send
     * NEWSIZE if the size changed.  We also send NEWSIZE
     * if it was a user-initiated action on a SIZEVERIFY window,
     * or a program-initiated action on any window, since in those
     * cases the application could know that sizing had begun:
     *
     * New for V36:  We don't send NEWSIZE if the user did
     * a NULL-sizing operation on a non-SIZEVERIFY window.
     */
    if ( TESTFLAG( subcommand, CWSUBQ_SIZING ) &&
	( TESTFLAG( subcommand, CWSUBQ_PROGRAMMATIC) ||
	TESTFLAG( w->IDCMPFlags, SIZEVERIFY ) ||
	sizechanged ) )
    {
	DICW( printf( "Sending NEWSIZE\n" ) );
	D( printf("ICW: sizewindow details\n") );

	/*
	 * NOTE: sizedamage is passed these negatives
	 * if it comes AFTER the window widths are 
	 * changed
	 */
	sizeDamage( w, -dimchange.X, -dimchange.Y );
	DZIP( printf("after sizeDamage, before message, refresh %lx\n",
		w->WLayer->Flags & LAYERREFRESH ));
	DZIP2( Debug() );
	windowEvent( w, IECLASS_SIZEWINDOW, 0 );
	windowEvent( w, IECLASS_EVENT, IECODE_NEWSIZE );
	DZIP( printf("after message\n"));
	DZIP2( Debug() );
    }

    /* send CHANGEWINDOW if the size or position changed, or if
     * we got here programmatically
     */
    if ( TESTFLAG( subcommand, CWSUBQ_PROGRAMMATIC ) ||
	sizechanged || poschanged )
    {
	DICW( printf( "Sending CHANGEWINDOW\n" ) );
	windowEvent( w, IECLASS_CHANGEWINDOW, CWCODE_MOVESIZE );
    }

    D( printf("ICW: about done.\n") );

    DZIP( printf("IChange about to do BP\n") );
    DZIP2( Debug() );

    BorderPatrol( w->WScreen );
    
    DZIP( printf("IChange back from BP\n") );
    DZIP( Debug() );

    UnlockLayerInfo( &w->WScreen->LayerInfo );

    return( success );
}

/* what would the innerbox for a window if the
 * window position/dims were 'windowbox'?
 */
innerWindowBox( w, windowbox, innerbox )
struct Window	*w;
struct IBox	*windowbox;
struct IBox	*innerbox;
{
    *innerbox = *windowbox;
    innerbox->Left += w->BorderLeft;
    innerbox->Top += w->BorderTop;
    innerbox->Width -= ( w->BorderLeft + w->BorderRight );
    innerbox->Height -= ( w->BorderTop + w->BorderBottom );
}

doWindowLayer( le, windowbox, innerbox )
struct LayerEntry	*le;
struct IBox		*windowbox;
struct IBox		*innerbox;
{
    int	retval;

    switch ( le->le_Type )
    {
    case LET_REQUEST:
	D( printf("dWL: requester layer\n" ) );
	retval = establishReqLayer3( le->le_Requester, windowbox,
	    (struct Point *) &innerbox->Width );
	break;

    case LET_WINDOW:
	D( printf("dWL: window layer\n" ) );
	DFAIL( printf("pretend to FAIL!\n"); return ( 0 ) );
	retval = changeLayer( le->le_Layer, windowbox );
	break;

    case LET_G00:
	D( printf("dWL: g00 layer\n" ) );
	retval = changeLayer( le->le_Layer, innerbox );
	break;
    }

    return ( retval );
}

/*
 * calls movesize (passing deltas) to get layer to 
 * position/dims as specificed by box
 */
changeLayer( l, box )
struct Layer	*l;
struct IBox	*box;
{
    DREQ( dumpBox("changeLayer to:", box ) );
    return ( MoveSizeLayer( l,
	box->Left - l->bounds.MinX,
	box->Top - l->bounds.MinY,
	box->Width - rectWidth( &l->bounds ),
	box->Height - rectHeight( &l->bounds ) ) );
}

struct Layer	*
createReqLayer( linfo, bmap, box, flags )
struct Layer_Info	*linfo;
struct BitMap		*bmap;
struct IBox		*box;
ULONG			flags;
{
    struct Layer	*CreateBehindLayer();

    return ( CreateBehindLayer( linfo, bmap,
	box->Left, box->Top,
	box->Left + box->Width - 1, box->Top + box->Height - 1,
	flags, NULL) );
}

/* Peter 4-Apr-91:  Return the frontmost layer of a window, starting
 * with the specified requester.
 */
struct Layer	*
windowFrontLayer( window, req )
struct Window	*window;
struct Requester *req;
{

    while (req)
    {
	if (req->ReqLayer)
	{
	    return(req->ReqLayer);
	}
	req = req->OlderRequest;
    }
    return ( window->WLayer );	/* inner layer if G00	*/
}


/*
 * have to use UpfrontLayer and BehindLayer, else
 * deal with backdrops myself
 */
IWindowDepth(w, whichway, behindwindow)
struct Window *w;
int	whichway;
struct Window *behindwindow;
{
    int	i, numlayers;
    struct LayerEntry	*le;
    struct LayerEntry	layerlist[MAXWINDOWLAYERS];
    struct Layer_Info	*linfo;
    struct Layer	*behindlayer;
    int			success;
    int			fronttoback;	/* order layers are processed */

    if (NOT knownWindow(w)) return;

    LockLayerInfo( linfo = &w->WScreen->LayerInfo );

    if ( whichway == WDEPTH_INFRONTOF )
    {
	behindlayer = windowFrontLayer( behindwindow, behindwindow->FirstRequest );
    }

    /*  Peter: 1-Oct-90: Get the window layer list, excluding those
     *  requesters whose layers are currently NULL
     */
    i =  numlayers = windowLayerList( w, layerlist, FALSE );

    fronttoback = (whichway == WDEPTH_TOFRONT)? 0: 1;

    /* i = ..., 3, 2, 1 (not 0)	*/
    /*
     * too bad: have to kludge in some back/front sensitivity, now
     * that I'm back to using UpfrontLayer
     */
    for ( le = fronttoback? layerlist: &layerlist[i-1];
	i;
	--i, le = fronttoback? (le + 1): (le - 1) )
    {
	/* use BehindLayer() to account for backdrops */
	switch ( whichway )
	{
	case WDEPTH_TOFRONT:
	    success = UpfrontLayer( NULL, le->le_Layer );
	    break;
	case WDEPTH_TOBACK:
	    success = BehindLayer( NULL, le->le_Layer );
	    break;
	case WDEPTH_INFRONTOF:
	    success = MoveLayerInFrontOf( le->le_Layer, behindlayer );
	    break;
	}

	if ( ! success )
	{
	    D( printf("infrontof failed!\n") );
	    /* try to bail out of partial failure
	     * note that 'le' is the layer which failed to move
	     */
	    while  ( i++ < numlayers )
	    {
		/* put layers in front of one that failed to move
		 * (behind, if not frontoback)
		 */
		behindlayer = fronttoback? le->le_Layer: le->le_Layer->back;
		le = fronttoback? (le - 1): (le + 1);
		MoveLayerInFrontOf( le->le_Layer, behindlayer );
	    }
	    break;	/* done done done	*/
	}
    }

OK:
    /* V39 now sends CHANGEWINDOW messages upon depth-arrange,
     * if {WA_NotifyDepth,TRUE} is requested.
     */
    if ( TESTFLAG( w->MoreFlags, WMF_NOTIFYDEPTH ) )
    {
	D( printf( "Sending depth-CHANGEWINDOW\n" ) );
	windowEvent( w, IECLASS_CHANGEWINDOW, CWCODE_DEPTH );
    }
    BorderPatrol(w->WScreen );
    UnlockLayerInfo(linfo);
}


#if TRY_OFFSCREEN_WINDOWS /* this would need some rework	*/
/*
 * returns new scrollx
 */
setupLCS( w, lcs, le, wbox )
struct Window	*w;
struct LayerChangeSpec	*lcs;
struct LayerEntry	*le;
struct IBox		*wbox;
{
    int			scrollx = 0;
    struct IBox		vlayerbox;
    struct Layer	*l = le->le_Layer;

    /* calculate virtual position of layer	*/
    vlayerbox.Left = wbox->Left + le->le_BorderBox.Left;
    vlayerbox.Top = wbox->Top + le->le_BorderBox.Top;
    vlayerbox.Width = wbox->Width - le->le_BorderBox.Width;
    vlayerbox.Height = wbox->Height - le->le_BorderBox.Height;

    /* calculate actual layer box, plus scroll values	*/
    vlayerbox.Width =
	    imin( vlayerbox.Width, w->WScreen->Width - vlayerbox.Left );
    vlayerbox.Height =
	    imin( vlayerbox.Height, w->WScreen->Height - vlayerbox.Top );
    if ( vlayerbox.Left < 0 )
    {
	scrollx = -vlayerbox.Left;
	vlayerbox.Left = 0;
	vlayerbox.Width -= scrollx;
    }

    /* calculate deltas from current layer values to recommended */
    lcs->lcs_Dx = vlayerbox.Left - l->bounds.MinX;
    lcs->lcs_Dy = vlayerbox.Top - l->bounds.MinY;
    lcs->lcs_Dw = vlayerbox.Width - rectWidth( &l->bounds );
    lcs->lcs_Dh = vlayerbox.Height - rectHeight( &l->bounds );

    return ( scrollx );
}
#endif


/* change the size of a requester layer so that it
 * fits in a window about to be position/dimension 'box'
 *
 * delete the layer if it is completely clipped, create it if
 * necessary
 *
 * ZZZ: change: damageWholeLayer whenever created.
 */
establishReqLayer3( req, windowbox, gzzdims )
struct	Requester	*req;
struct IBox		*windowbox;
struct Point		*gzzdims;
{
    struct	Window	*window = req->RWindow;
    struct 	IBox	reqbox;
    BOOL	retval = TRUE;	/* signifies that couldn't do LayerOp	*/
    struct	Layer_Info	*linfo;
    ULONG		layerflags;

    /** jimm: 11/21/85 ** can't let the user drag the windoww
     * out from under me: actually, block until the user is done dragging
     */

    /* get down on that layer thing	*/
    linfo = &window->WScreen->LayerInfo;
    LockLayerInfo(linfo);

    /* set up reqbox in screen coordinates,
     * starting with the largest width/height they
     * possibly can be.
     * This is from the requesters upperleft to
     * the inner limits of the window.
     */
    reqbox = *windowbox;

    reqbox.Left += req->LeftEdge;
    reqbox.Top += req->TopEdge;
    reqbox.Width -= ( window->BorderRight + req->LeftEdge );
    reqbox.Height -= ( window->BorderBottom + req->TopEdge );

    if ( window->Flags & GIMMEZEROZERO )
    {
	reqbox.Left += window->BorderLeft;
	reqbox.Top += window->BorderTop;
	reqbox.Width = gzzdims->X - req->LeftEdge;
	reqbox.Height = gzzdims->Y - req->TopEdge;
    }

    /* reqbox isn't any larger than the requester, after all	*/
    reqbox.Width = imin( reqbox.Width, req->Width );
    reqbox.Height = imin( reqbox.Height, req->Height );

    /* see if there is supposed to be a layer at all	*/
    if ( reqbox.Width <= 0 || reqbox.Height <= 0 )
    {
	if (req->ReqLayer != NULL)	/* need to delete layer	*/
	{
	    DMEM( printf("eRL3 deleting old reqlayer at %lx\n",
		req->ReqLayer ) );
	    DeleteLayer(NULL, req->ReqLayer);
	    req->ReqLayer = NULL;
	}
	retval = TRUE;	/* OK, just no layer	*/
	goto GETOUT;
    }

    /* create a layer if there isn't one */
    if (req->ReqLayer == NULL)
    {
	layerflags = (req->Flags & SIMPLEREQ)? LAYERSIMPLE: LAYERSMART;

	/* requester layers for backdrops should be backdrops
	 * jimm 1/18/90
	 */
	layerflags |= ( window->WLayer->Flags & LAYERBACKDROP );

	/* jimm: 4/10/86:
	 * make layer smart, so users can render in it, 
	 * even though they won't get requester refresh messages
	 *
	 * (now depends on SIMPLEREQ flag)
	 */
	/** create behind, move to middle **/
	DMEM( printf("call createReqLayer ... "));
	req->ReqLayer = createReqLayer( linfo,
	    window->WScreen->RastPort.BitMap, &reqbox,
	    layerflags );
	DMEM( printf("returns %lx\n", req->ReqLayer ) );

	if (req->ReqLayer == NULL)	/* new layer failed	*/
	{
	    DMEM( printf("eRL3 no reqlayer, bailout\n" ) );
	    retval = FALSE;	/* failure	*/
	    goto GETOUT;
	}
	else	/* new layer successful	*/
	{	
	    /* stash window pointer */
	    req->ReqLayer->Window = window;

	    DMEM( printf("layer to front ... ") );
	    MoveLayerInFrontOf(req->ReqLayer, windowFrontLayer( window, req->OlderRequest ) );
	    DMEM( printf("returns.\n") );

	    damageWholeLayer( req->ReqLayer );	/* only when sizing? */
	}
    }
    else	/* layer exists, need to adjust its size */
    {
	DREQ( dumpBox("change requester box", &reqbox ));
	if ( ! changeLayer( req->ReqLayer, &reqbox ) )
	{
	    DREQ(printf("change failed\n"));
	    retval = FALSE;	/* failure	*/
	    req->ReqLayer = NULL;
	}
	DREQ( else printf("layer %lx layerrefresh: %lx\n",
		req->ReqLayer, LAYERREFRESH & req->ReqLayer->Flags ));

	DREQ( printf("setting the bit myself\n"));
	DREQ( SETFLAG( req->ReqLayer->Flags, LAYERREFRESH ));
    }

GETOUT:
    UnlockLayerInfo(linfo);
    return (retval);
}

damageWholeLayer( l )
struct Layer	*l;
{
    struct Rectangle	layerrect;

    layerrect.MinX = layerrect.MinY = 0;

    layerrect.MaxX = rectWidth( &l->bounds ) - 1;
    layerrect.MaxY = rectHeight( &l->bounds ) - 1;
    OrRectRegion( l->DamageList, &layerrect );
    l->Flags |= ( LAYERREFRESH | LAYERI_NOTIFYREFRESH | LAYERI_GADGETREFRESH );
}
