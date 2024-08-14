/*** requester.c ************************************************************
 *
 *  File
 *
 *  $Id: requester.c,v 40.0 94/02/15 17:29:52 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#define TEXTPAD 4
/** #define DEBUG 1 **/

#include "intuall.h"
#include "imageclass.h"

/*****************************************************************************/

static BOOL newDMRequest (struct Window *w, struct Requester *req);
static void pointRel (struct Requester *r, struct Window *window, LONG x, LONG y);

/*****************************************************************************/

#define WORRY_REQDRAWN	FALSE

/*****************************************************************************/

#define D(x)	;

/*****************************************************************************/

void drawRequester (struct Requester *req, struct Window *w)
{
    struct RastPort *rport;
    LONG right, bottom;

    if (req->ReqLayer == NULL)
    {
	return;
    }

    if (req->Flags & PREDRAWN)
    {
	/* this requester's imagery comes pre-drawn.
	 * copy that into our new requester rastport */
	BltBitMapRastPort (req->ImageBMap, 0, 0, req->ReqLayer->rp, 0, 0,
			   req->Width, req->Height, 0x00C0);
    }
    else
    {
	bottom = req->Height - 1;
	right = req->Width - 1;

	rport = obtainRP (&w->WScreen->RastPort, req->ReqLayer);

	/* rp->Mask = -1; already */
	/* SetDrMd(rport, JAM1); already */

	/* perhaps inhibit this with a new flag	*/
	if (!TESTFLAG (req->Flags, NOREQBACKFILL))
	    drawBlock (rport, 0, 0, right, bottom, req->BackFill);

	/** Draw Imagery **/
	/* offsets taken care of when layer created	*/

	if (TESTFLAG (req->Flags, USEREQIMAGE))
	{
	    struct DrawInfo *drinfo;
	    struct DrawInfo *GetScreenDrawInfo ();

	    drinfo = GetScreenDrawInfo (w->WScreen);
	    DrawImageState (rport, req->ReqImage, 0, 0, IDS_NORMAL, drinfo);
	    FreeScreenDrawInfo (w->WScreen, drinfo);
	}

	if (req->ReqBorder)
	    DrawBorder (rport, req->ReqBorder, 0, 0);
	if (req->ReqText)
	    PrintIText (rport, req->ReqText, 0, 0);

	freeRP (rport);

	/** Get Gadgets Drawn **/
	/** NOTE: that we pass requester window address once again 2/7/86 */
	drawGadgets (req->RWindow, req->ReqGadget, -1, DRAWGADGETS_ALL);
    }
}

/*****************************************************************************/

static BOOL newDMRequest (struct Window *w, struct Requester *req)
{
    if (w->DMRequest)
	if (w->DMRequest->Flags & REQACTIVE)
	    return (FALSE);
    w->DMRequest = req;
    return (TRUE);
}

/*****************************************************************************/

/*** intuition.library/ClearDMRequest ***/
BOOL ClearDMRequest (struct Window *w)
{
    return (newDMRequest (w, NULL));
}

/*****************************************************************************/

/*** intuition.library/SetDMRequest ***/
BOOL SetDMRequest (struct Window *w, struct Requester *req)
{
    return (newDMRequest (w, req));
}

/*****************************************************************************/

/*** intuition.library/Request ***/
BOOL Request (struct Requester *req, struct Window *window)
{
    LONG retval;

    retval = doISM (itSETREQ, (CPTR)window, (CPTR)req, NULL);
    D (printf ("Request after doISM. retval %lx\n", retval));
    return (BOOL)(!retval);
    /*** return ( ! doISM( itSETREQ, window, req ) ); ****/
}

/*****************************************************************************/

/* returns TRUE if it succeeded
 * pass bool 'isdmr' true if this is a DMRequester */
BOOL IRequest (struct Requester *req, struct Window *window, LONG isdmr)
{
    struct Gadget *gadget;
    LONG relx, rely;

    /** link used in drawGGrunt **/
    req->RWindow = window;

    /** Get a layer for the new requester **/
    req->ReqLayer = NULL;	/* just to be very sure */

    /* link in (tail) so establishReqLayer knows where to put layer */
    req->OlderRequest = window->FirstRequest;	/* works even if NULL */

    SETFLAG (req->Flags, REQACTIVE);

    /* either bring up the requester under the mouse, or
     * relative to the center of the screen, if POINTREL
     */
    if (TESTFLAG (req->Flags, POINTREL))
    {
	if (isdmr)		/* position under mouse */
	{
	    relx = window->MouseX;
	    rely = window->MouseY;
	}
	else
	    /* position over center of window */
	{
	    relx = (window->Width - req->Width) / 2;
	    rely = (window->Height - req->Height) / 2;
	}

	pointRel (req, window, relx, rely);
    }

    /* use new eRL3 -- flying cast to (struct IBox *) 	*/
    /* layer is created with damage, until further notice */
    if (!establishReqLayer3 (req, (struct IBox *) &window->LeftEdge,
			     (struct Point *) &window->GZZWidth))
    {
	goto BAD;
    }

    /* help that programmer remember to set his REQGADGET flag */
    /* no GADGETSLOCK here, since the gadgets are not
     * yet available to another task.
     */
    for (gadget = req->ReqGadget; gadget; gadget = gadget->NextGadget)
    {
	gadget->GadgetType |= REQGADGET;
    }

    /* link requester into the Window */
    /* above -- req->OlderRequest = window->FirstRequest; */
    window->FirstRequest = req;
    window->ReqCount++;
    window->Flags |= INREQUEST;

    /* since the requester layer is created with damage, let's
     * just use BorderPatrol to draw it the first time
     */
    BorderPatrol (window->WScreen);
    D (printf ("eRL3: after BP, layer refresh = %lx\n",
	       req->ReqLayer->Flags & LAYERREFRESH));

    windowEvent (window, IECLASS_REQUESTER, IECODE_REQSET);

  GOOD:
    return (TRUE);
  BAD:
    CLEARFLAG (req->Flags, REQACTIVE);
    return (FALSE);
}

/*****************************************************************************/

BOOL IEndRequest (struct Window *window, struct Requester *req)
{
    struct Requester *prevr;
    BOOL has_reqlayer = FALSE;

    /* jimm: 6/9/86: shut off REQACTIVE */
    CLEARFLAG (req->Flags, REQACTIVE);

    /* check that the requester really exists */
    prevr = window->FirstRequest;
    while (prevr && (prevr != req))
	prevr = prevr->OlderRequest;

    if (!prevr)
    {
	return (FALSE);
    }

    /* unlink requester */
    if (prevr = (struct Requester *) previousLink ((struct Thing *)window->FirstRequest, (struct Thing *)req))
    {
	prevr->OlderRequest = req->OlderRequest;
    }
    else
	/* first in list	*/
    {
	window->FirstRequest = req->OlderRequest;	/* may == NULL */
    }

    /* decrement now, test in two places later */
    --window->ReqCount;

    /*  deleteLayer */
    if (req->ReqLayer != NULL)
    {
	has_reqlayer = TRUE;
	LockLayerInfo (&window->WScreen->LayerInfo);
	DeleteLayer (NULL, req->ReqLayer);
	req->ReqLayer = NULL;

	/* The REQCLEAR event has to sneak in before BorderPatrol
	 * sends a REFRESHWINDOW message to the guy.  I like
	 * keeping the message inside the LockLayerInfo
	 * protection for the underlying window's damage list.
	 * So, let's exit this condition (ReqLayer != NULL)
	 * temporarily
	 */
    }

    /* Peter 24-Jan-92: UNCONDITIONALLY send REQCLEAR.  2.04 had
     * a bug where REQCLEAR wasn't sent for non-last requesters
     * that had no layer.
     */

    windowEvent (window, IECLASS_REQUESTER, IECODE_REQCLEAR);
    if (window->ReqCount == 0)
    {
	CLEARFLAG (window->Flags, INREQUEST);
    }

    /* Now we need to finish the job for the case when ReqLayer is
     * non-NULL.
     */

    if (has_reqlayer)
    {
	/* only BP if I had a layer to delete (real damage),
	 * and do it under the protection of the LInfo lock
	 */
	BorderPatrol (window->WScreen);
	UnlockLayerInfo (&window->WScreen->LayerInfo);
    }

    return (TRUE);
}

/*****************************************************************************/

/*** intuition.library/EndRequest ***/
void EndRequest (struct Requester *req, struct Window *window)
{
    doISM (itCLEARREQ, (CPTR)window, (CPTR)req, NULL);
}

/*****************************************************************************/

/*
 * sets up requester position, best fit in window
 * while trying to open at x,y plus Reqester.RelLeft/Top
 */
static void pointRel (struct Requester *r, struct Window *window, LONG x, LONG y)
{
    /* do a best fit thing */
    struct IBox reqbox;
    struct IBox windowbox;

    /* get correct mouse position */
    reqbox.Left = x;
    reqbox.Top = y;

    /* do the mouse relative stuff */
    reqbox.Left += r->RelLeft;
    reqbox.Top += r->RelTop;

    reqbox.Width = r->Width;
    reqbox.Height = r->Height;
    /* have advisory requester box */

    /* get containing dimensions */
    windowInnerBox (window, &windowbox);

    /* get best fit */
    boxFit (&reqbox, &windowbox, &reqbox);

    /* convert upper left to GZZ if necessary */
    if (TESTFLAG (window->Flags, GIMMEZEROZERO))
    {

	reqbox.Left -= window->BorderLeft;
	reqbox.Top -= window->BorderTop;
    }

    /* set true requester left/top attempt */
    r->LeftEdge = reqbox.Left;
    r->TopEdge = reqbox.Top;
    D (printf ("final top/left: %ld/%ld\n", r->LeftEdge, r->TopEdge));
}
