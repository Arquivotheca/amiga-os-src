
/*** hitgadgets.c ************************************************************
 *
 *  $Id: hitgadgets.c,v 38.8 92/11/10 17:04:19 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#define D(x)	;

#include "intuall.h"
#include "gadgetclass.h"


struct Layer *WhichLayer();

/* 
 * checks if any gadget in layer has been hit.
 * returns TRUE if so.
 *
 * if window has a requester in it, only returns true for
 * hit on system gadgets and REQGADGET.  Caller probably
 * wants to disallow the CLOSE
 *
 * makes free use of IBase->HitScreen, ->Flags&HIT_BARLAYER, ->ActiveWindow.
 * sets IBase->ActiveGadget, ->GadgetInfo, ->GadgetEnv
 */
BOOL
hitGadgets(layer)
struct Layer *layer;
{
    /* these will define the environment of the active group */
    APTR	ptr;	/* will be window or screen */
    struct	Gadget *firstg;

    struct IntuitionBase *IBase = fetchIBase();
    struct	Window *w;
    struct	Requester *req;
    BOOL	success = FALSE;
    struct IBox	gadgetbox;

    LOCKGADGETS();

    D( printf("hitGadgets:::Check (A5), and trigger on it\n") );

    /* all gadgets in windows, requester, or screenbar */
    if (!layer) goto OUT;

    /* determine which environment to use */
    /* screenbar was hit */
    if (TESTFLAG(IBase->Flags, HIT_BARLAYER))	/* barlayer was hit */
    {
	ptr = IBase->HitScreen;
	firstg = ((struct Screen *)ptr)->FirstGadget;
	goto HIT_EM;
    }

    /* no screen gadgets here */

    if (!(w = IBase->ActiveWindow))	/* no active window */
    {
	goto OUT;
    }

    /* w is active window */
    ptr = w;

    /* condition: hit was in active requester */
    /* ZZZ: limitation: this thing will pick window
     * gadgets right through inactive requesters
     */
    if ((req = w->FirstRequest) && (layer == req->ReqLayer))
    {
	firstg = req->ReqGadget;
	D( printf("hitGadgets: requester layer, firstg %lx\n", firstg));
	goto HIT_EM;
    }

    /* req is non-NULL if there is a requester, but we missed it */

    /* hit must be in window layer, or inactive requester */
    firstg = w->FirstGadget;
    /* goto HIT_EM */

HIT_EM:

    /* get environment for gadget list */
    if (!gListDomain(firstg, ptr, &IBase->GadgetEnv)) goto OUT;

    /* check each gadget */
    while (firstg)
    {
	/* here I'm setting up the active gadget info, if 
	 * there is success hitting.
	 */
	gadgetInfo(firstg, &IBase->GadgetEnv );

	gadgetBox( firstg, &IBase->GadgetEnv.ge_GInfo, &gadgetbox );
	if (success = hitGGrunt( firstg,
		&IBase->GadgetEnv.ge_GInfo, &gadgetbox, GM_HITTEST ))
	{
	    /* if hit gadget is DISABLED, you done */
	    if (firstg->Flags & GADGDISABLED)
	    {
		success = FALSE;
		goto OUT;
	    }

	    /* If requester is active, only allow requester gadgets
	     * or system gadgets to be hit.  However, the CLOSE gadget
	     * is disallowed even though it's a system gadget.
	     */
	    if (req)
	    {
		success = ( firstg->GadgetType & ( REQGADGET | GTYP_SYSTYPEMASK ) );
		if ( success == GTYP_CLOSE )
		{
		    success = FALSE;
		}
	    }

	    goto OUT;
	}

	firstg = firstg->NextGadget;
    }

OUT:
    if (success)	/* set new active gadget */
    {
	IBase->ActiveGadget = firstg;
    }
    UNLOCKGADGETS();

    return (success);
}

/* Test gadget hit or help hit (calls hook with GM_HITTEST or GM_HELPTEST)
 * For GM_HITTEST, returns either GMR_GADGETHIT or 0 (not hit)
 * For GM_HELPTEST, returns either GMR_NOHELPHIT or anything else (help hit)
 *
 * NOTE: we CANNOT change the values in 'gbox' here
 */
ULONG
hitGGrunt( g, gi, gbox, methodid )
struct Gadget 		*g;
struct GadgetInfo 	*gi;
struct IBox		*gbox;
ULONG methodid;
{
    struct Point mouse;

    struct IBox	localbox;	/* select box, but at 0,0 position */

    ULONG	retval = 0;
    struct Hook		*gadgetHook();

    /* get mouse coordinates from either screen or window */
    gadgetMouse( g, gi, &mouse, gbox);

    /* set up box, but relative in box-relative coordinates	*/
    localbox.Left = localbox.Top = 0;
    localbox.Width = gbox->Width;
    localbox.Height = gbox->Height;

    /** SELECT must be in gadget box for consideration **/
    if ( ptInBox( mouse, &localbox) )
    {
	D( printf("hitGG triggered in box, call hook %lx\n", gadgetHook(g)) );

	retval = callHook( gadgetHook(g), g, methodid, gi, mouse );
    }

    return (retval);
}


/* Routine to figure out gadget help.  Call this when the Help
 * rawkey is received inside idle-window or requester state.
 * Figures out which layer the mouse is over.  If the mouse
 * is over the active window or one of its requesters, then
 * search all the gadgets of that window (or requester) for
 * a gadget "help hit".
 * The first pass check for "help hits" is intersection with
 * the gadget bounding box.  GM_HELPTEST is sent to any gadget
 * whose bounding box includes the mouse pointer.
 * The gadget responds with a code to return to the user,
 * or GMR_NOHELPHIT if the mouse coordinates constitute a miss.
 *
 * This routine actually sends the IDCMP_GADGETHELP message along.
 *
 * NOTE that disabled gadgets and gadgets other than in the
 * active requester are subject to GM_HELPTEST, unlike GM_HITTEST.
 */

/* Assumes active window is non-NULL */
/* These magic numbers match autopoint's.  That's the largest distance
 * the mouse can move in one timer event and still cause help to be
 * sent...
 */
#define XHELP	6
#define YHELP	3
gadgetHelpTimer()
{
    struct IntuitionBase *IBase = fetchIBase();
    WORD mousex = IBase->MouseX;
    WORD mousey = IBase->MouseY;

    /* If the active window has help on, and the mouse has travelled
     * some, but not too far in the last timer interval, do help:
     */
    if ( TESTFLAG( IBase->ActiveWindow->MoreFlags, WMF_GADGETHELP ) &&
	( iabs( mousex - IBase->LastTimeX ) <= XHELP ) &&
	( iabs( mousey - IBase->LastTimeY ) <= YHELP ) &&
	( ( mousex != IBase->LastHelpX ) ||
	( mousey != IBase->LastHelpY ) ) )
    {
	/* NULL means not in a window whose HelpGroup matches the
	 * active one.
	 */
	struct Gadget *helpgad = NULL;
	struct Layer *hitlayer;
	struct Window *hitwin;
	ULONG result = GMR_HELPHIT;
	LONG sendhelp = 1;

	IBase->LastHelpX = mousex;
	IBase->LastHelpY = mousey;

	/* Check and send gadget-help if any */

	/* Which window/layer is the mouse over?
	 * (Help can only apply to the active window or any windows
	 * of the same HelpGroup.  If this is not the case, we'll send
	 * GADGETHELP with an IAddress of zero.
	 */
	hitwin = hitUpfront( &hitlayer );
	if ( ( !TESTFLAG( IBase->Flags, HIT_BARLAYER ) ) && ( hitwin ) &&
	    ( XWINDOW(hitwin)->HelpGroup == XWINDOW(IBase->ActiveWindow)->HelpGroup ) )
	{
	    struct Requester *req;
	    struct IBox gadgetbox;

	    /* Assume the hit was over the window, in which case
	     * we need to examine the window's gadget list:
	     */
	    helpgad = hitwin->FirstGadget;

	    /* Now check to see if the hitlayer was actually a
	     * requester layer, in which case we use that requester's
	     * gadget list:
	     */
	    for ( req = hitwin->FirstRequest; req; req = req->OlderRequest )
	    {
		if ( req->ReqLayer == hitlayer )
		{
		    helpgad = req->ReqGadget;
		    break;
		}
	    }

	    LOCKGADGETS();

	    /* get environment for gadget list */
	    if ( gListDomain( helpgad, hitwin, &IBase->GadgetEnv ) )
	    {
		/* check each gadget */
		while ( helpgad )
		{
		    if ( TESTFLAG( helpgad->Flags, GFLG_EXTENDED ) &&
			TESTFLAG( XGAD(helpgad)->MoreFlags, GMORE_GADGETHELP ) )
		    {
			/* here I'm setting up the gadget info, if 
			 * there is success hitting.
			 */
			gadgetInfo( helpgad, &IBase->GadgetEnv );

			gadgetBoundingBox( helpgad, &IBase->GadgetEnv.ge_GInfo,
			    &gadgetbox );
			if ( ( result = hitGGrunt( helpgad,
			    &IBase->GadgetEnv.ge_GInfo,
			    &gadgetbox, GM_HELPTEST ) ) != GMR_NOHELPHIT )
			{
			    goto OUT;
			}
		    }
		    helpgad = helpgad->NextGadget;
		}
		/* For the IDCMP_GADGETHELP message, a special IAddress
		 * value equal to the window denotes "in the window but
		 * not over any gadget".
		 */
		if ( !helpgad )
		{
		    helpgad = (struct Gadget *) hitwin;
		    result = GMR_HELPHIT;
		}
	    }
	    else
	    {
		sendhelp = 0;
	    }

OUT:
	    UNLOCKGADGETS();
	}
	else
	{
	    /* Mouse was over no window, or was over a window in
	     * a different Help Group.  We have to be sure to
	     * send the message to the active window in this case.
	     * helpgad is zero, which produces a GADGETHELP message
	     * with an IAddress of zero, meaning the mouse is not
	     * inside a window whose HelpGroup matches the active window.
	     */
	    hitwin = IBase->ActiveWindow;
	}

	/* Store the help-gadget so windowEvent() can fill in the
	 * IAddress and ie_EventAddress.
	 */
	if ( ( sendhelp ) && ( ( helpgad != IBase->HelpGadget ) ||
	    ( result != IBase->HelpGadgetCode ) ) )
	{
	    IBase->HelpGadget = helpgad;
	    IBase->HelpGadgetCode = result;
	    windowEvent( hitwin, IECLASS_GADGETHELP, result );
	}
    }

    /* Update mouse position at this timer interval, for use next time */
    IBase->LastTimeX = mousex;
    IBase->LastTimeY = mousey;
}

/*** HelpControl() ***/
HelpControl( helpwin, help )
struct Window *helpwin;
ULONG help;
{
    struct Screen *sc;
    struct Window *win;
    struct IntuitionBase *IBase = fetchIBase();

    LOCKIBASE();

    /* We need to change WMF_GADGETHELP in all windows whose
     * HelpGroup matches the HelpGroup of the helpwin.
     */
    for ( sc = IBase->FirstScreen; sc; sc = sc->NextScreen )
    {
	for ( win = sc->FirstWindow; win; win = win->NextWindow )
	{
	    if ( XWINDOW(win)->HelpGroup == XWINDOW(helpwin)->HelpGroup )
	    {
		if ( help )
		{
		    SETFLAG( win->MoreFlags, WMF_GADGETHELP );
		}
		else
		{
		    CLEARFLAG( win->MoreFlags, WMF_GADGETHELP );
		}
	    }
	}
    }

    UNLOCKIBASE();
}
