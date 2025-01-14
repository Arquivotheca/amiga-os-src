/*** swsizedrag.c ********************************************************
 *
 * sWSizeDrag state processing
 *
 *  $Id: swsizedrag.c,v 38.17 93/01/14 14:24:23 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/


#include "intuall.h"

#include "swsizedrag_protos.h"

/*---------------------------------------------------------------------------*/

static int returnWSDOK(int okcode);

static int updateFrame(int initial);

static int setIFrame(void);

/*---------------------------------------------------------------------------*/

#define D(x)	;

/* transition startWSizeDrag()
 * called by:
 * with state data:
 * description:
 */
startWSizeDrag( wsdreturn, size_drag )
UWORD		(*wsdreturn)();
{
    struct IntuitionBase	*IBase = fetchIBase();
    int returnWSDOK();

    D( printf("transition function startWSizeDrag, cstate: %ld\n", 
	IBase->CurrentState ));

    setIFrame();	/* snapshot window dims, mouse offset,
			 * before the heartbreak of SIZEVERIFY
			 */

    IBase->WSDReturn = wsdreturn;
    if ( ( IBase->SizeDrag = size_drag ) == WSD_SIZE )
    {
	D( printf("startWSD, get SIZEVERIFY\n") );
	startVerify( returnWSDOK, IBase->ActiveWindow, SIZEVERIFY, 0 );
    }
    else
    {
	D( printf("startWSD: direct connect for dragging\n") );
	returnWSDOK( OKOK  );	/* direct connect to own routine */
    }
}

static
returnWSDOK( okcode )
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Window		*window;
    struct Screen		*screen;
    struct Rectangle		worklimits;
    struct Point		mousespot;

    D( printf("returnWSDOK okcode %lx\n", okcode ) );

    if ( okcode != OKOK )
    {
	if ( IBase->SizeDrag == WSD_SIZE )
	{
	    /*
	     * if aborted (or timeout), send "verify clear"
	     * ZZZ: would need to change for "MOVEVERIFY"
	     */
	    activeEvent( IECLASS_SIZEWINDOW, 0 );
	}
	(*IBase->WSDReturn)();		/* back to previous state */
	return;
    }

    window = IBase->ActiveWindow;
    screen = window->WScreen;

    LockLayers( &screen->LayerInfo );

    D( printf("sWSD: layers locked OK\n") );

    if ( IBase->SizeDrag == WSD_SIZE )
    {
	D( printf("WSD_SIZE:\n") );

	/* ------ Calculate Mouse Limits ---------------------------- */

	/* define range of travel of lower right corner,
	 * window-relative
	 */
	worklimits.MinX = window->MinWidth - 1;
	worklimits.MinY = window->MinHeight - 1;

	/* warning: MaxWidth/Height might be out of signed range.
	 * (or damned near)
	 * replace with a large value that won't wrap
	 * if it is offset or doubled (for lace coords)
	 * ZZZ: this is bullshit
	 */
#define VBIG	(7000)
	if (((worklimits.MaxX = window->MaxWidth) & 0x8000)
	    || (worklimits.MaxX > VBIG) )
	{
	    worklimits.MaxX = VBIG;
	}
	if (((worklimits.MaxY = window->MaxHeight) & 0x8000)
	    || (worklimits.MaxY > VBIG) )
	{
	    worklimits.MaxY = VBIG;
	}
	/* i admit that i'm a little afraid of the signed/unsigned
	 * combination above, so i'll do the necessary decrement
	 * here, where it's safer. (jimm)
	 */
	worklimits.MaxX--;
	worklimits.MaxY--;

	/* offset to respect window position in the screen */
	offsetRect( &worklimits, window->LeftEdge, window->TopEdge);

	/* constrain borders to screen, in screen res. coordinates	*/
	worklimits.MaxX = imin(worklimits.MaxX, screen->Width - 1 );
	worklimits.MaxY = imin(worklimits.MaxY, screen->Height - 1 );

	/* account for offset between mouse position and LRC	*/
	mousespot.X = window->Width - 1;
	mousespot.Y = window->Height - 1;

	/* now we are great, but in screen coords.  */

	/** Set up for general size/drag action **/
	IBase->frameChange	= stretchIFrame;
	IBase->SDDelta = (struct Point * ) & IBase->ChangeBox.Width;

    }
    else	/* WSD_DRAG */
    {
	D( printf("WSD_DRAG:\n") );

	/* rectangle of freedom for window position within screen	*/
	worklimits.MinX = worklimits.MinY = 0;
	worklimits.MaxX = screen->Width - window->Width;
	worklimits.MaxY = screen->Height - window->Height;

	/* Measured wrt. an origin at screen 0,0	*/
	mousespot.X = mousespot.Y = 0;

	/** Set up for general size/drag action **/
	IBase->frameChange	= dragIFrame;
	IBase->SDDelta = (struct Point * ) & IBase->ChangeBox.Left;

    }

    /* Offset wrt. ideal mousespot.
     * Have to use window mouse position at time of snapshot
     */
    offsetRect( &worklimits, 
	(IBase->FirstPt.X - window->LeftEdge) - mousespot.X,
	(IBase->FirstPt.Y - window->TopEdge ) - mousespot.Y );

    /* Save these limits, which are relative to the screen: */
    IBase->ScreenMouseLimits = worklimits;
    SETFLAG( IBase->Flags, INMOUSELIMITS|SCREENMLIMITS );
    rethinkMouseLimits();

    updateFrame( 1 );	/* draw first box	*/
    updateMousePointer(NULL);	/* mouse to within limits	*/
    updateFrame( 0 );	/* and make it sync with where the mouse is */

    IBase->CurrentState = sWSizeDrag;
    D( printf("sWSD done\n") );
}


/*
 * state dispatcher
 * state transitions called:
 */
dWSizeDrag()
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Window		*window;

    window = IBase->ActiveWindow;

    switch ( IT->it_Command )
    {

    /** Done **/
    case itMENUDOWN:	/* done, erase frame */
    case itSELECTUP:	/* done, erase frame */
	updateFrame( 1 );

/* jimm: 5/29/90:
 * I don't know why I morphed the token rather than
 * just calling IChangeWindow.  Ah, I see: perhaps the
 * previous state needs to do something in additional to
 * the default IChangeWindow().
 *
 * The previous state is sGadget, which only turns off
 * the (sizing) gadget when we're done.  That should be OK.
 *
 * The states which called startGadget() are also significant,
 * but they (sRequester, sIdleWindow) both only doDefault()
 * for itCHANGEWIN.
 *
 * So it looks like this is the way to go.  Yeesh.
 *
 * Well, further investigation shows that this lock *shouldn't*
 * be necessary (the lock is held over damage/refresh in
 * IChangeWindow just fine).  Holding this lock *does* close
 * a funnel that console falls into.
 */

	/* protect damage list until I'm done with it */
	LockLayerInfo( &window->WScreen->LayerInfo );
	UnlockLayers( &window->WScreen->LayerInfo );

	/* dispatch to sizer/dragger */
	if ( IT->it_Command == itSELECTUP )  /* oh, baby, do you mean it? */
	{
	    windowBox( IBase->ActiveWindow, &IBase->ChangeBox );

	    /* change either dims or position */
	    IBase->SDDelta->X += (window->WScreen->MouseX - IBase->FirstPt.X);
	    IBase->SDDelta->Y += (window->WScreen->MouseY - IBase->FirstPt.Y);

	    IChangeWindow( IBase->ActiveWindow,
		&IBase->ChangeBox, 
		(IBase->SizeDrag == WSD_SIZE)? (CWSUB_CHANGE | CWSUBQ_SIZING) :
		CWSUB_CHANGE );
	    /* unlock done below, for all conditions	*/
	    /* UnlockLayerInfo( &window->WScreen->LayerInfo ); */
	}
	/* have to send "end of sizing" to SIZEVERIFY guys
	 * in this abort case.
	 */
	else if ( TESTFLAG( window->IDCMPFlags, SIZEVERIFY ) &&
	    ( IBase->SizeDrag == WSD_SIZE ) )
	{
	    activeEvent( IECLASS_SIZEWINDOW, 0 );
	}
	UnlockLayerInfo( &window->WScreen->LayerInfo );

	CLEARFLAG( IBase->Flags, INMOUSELIMITS|SCREENMLIMITS );
	freeMouse();
	(*IBase->WSDReturn)();		/* back to previous state */
	return;

    /** Size/Drag Frame **/
    case itMOUSEMOVE:	/* update frame */
	doDefault();	/* will update the window's MouseX/Y */
	updateFrame( 0 );
	return;

    /*** default processing (mostly skip)	***/
    case itMENUUP:	/* do default */
    case itSELECTDOWN:	/* do default */
    case itRAWKEY:	/* do default */
    case itTIMER:	/* do default */
    case itDISKCHANGE:	/* do default */
    case itACTIVATEGAD:	/* do default (default is 'fail') */
    case itMETADRAG:	/* do default */
    case itVERIFY:	/* do default */
    case itNEWPREFS:	/* do default */
    case itMODIFYIDCMP:	/* do default */
    case itSETPOINTER:	/* do default */
	doDefault();
	return;

    case itCHANGESCBUF:	/* fail: app should never double-buffer with windows */
	IT->it_Error = 1;
	return;


#if 0
    case itMOVESCREEN:	/* defer (was skip prior to 3.01) */
    case itACTIVATEWIN:	/* defer */
    case itSETREQ:	/* defer */
    case itCLEARREQ:	/* defer */
    case itOPENWIN:	/* defer */
    case itCLOSEWIN:	/* defer */
    case itREMOVEGAD:	/* defer (might be the size/drag gadget) */
    case itSETMENU:	/* defer */
    case itCLEARMENU:	/* defer */
    case itCHANGEWIN:	/* defer */
    case itZOOMWIN:	/* defer */
    case itDEPTHWIN:	/* defer */
    case itDEPTHSCREEN:	/* defer */
    case itSHOWTITLE:	/* defer */
    case itOPENSCREEN:	/* defer */
    case itCLOSESCREEN:	/* defer */
    case itCOPYSCBUF:	/* defer */
    case itGADGETMETHOD:/* defer */
    case itMODIFYPROP:	/* defer */
#endif
    default:		/* itDEFAULT: defer */
	deferToken();
	return;
    }
}

/*
 * make it sync up with current mouse coordinates
 */
static
updateFrame( initial )
int initial;
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Point 		mouse;
    struct Point delta;
    struct Rectangle	oldrect;
    struct Rectangle	newrect;
    struct RastPort *RP;
    struct Window		*window;

    window = IBase->ActiveWindow;

    mouse.X = window->WScreen->MouseX;
    mouse.Y = window->WScreen->MouseY;


    /* ZZZ: should have function to subtract points */
    delta.X = mouse.X - IBase->OldPt.X;
    delta.Y = mouse.Y - IBase->OldPt.Y;

    if ( ( initial ) || ( delta.X ) || ( delta.Y ) )	/* some movement */
    {
	IBase->OldPt = mouse;

	boxToRect( &IBase->IFrame, &oldrect );
	if ( !initial )
	{
	    (*IBase->frameChange)( delta );
	    boxToRect( &IBase->IFrame, &newrect );
	}

	RP = obtainRP( XSC(window->WScreen)->ClipLayer->rp,
	    XSC(window->WScreen)->ClipLayer );

	SetDrMd( RP, COMPLEMENT );

	/* Draw the oldrect and optionally the newrect */
	drawFrames( RP, &oldrect, ( initial ) ? NULL : &newrect );

	freeRP(RP);
    }
}

static
setIFrame()	/* save current window/mouse stuff */
{
    struct IntuitionBase *IBase = fetchIBase();

    /* copy frame dimensions from window */
    IBase->IFrame = *((struct IBox *) &IBase->ActiveWindow->LeftEdge);

    IBase->FirstPt.X = IBase->OldPt.X = IBase->ActiveScreen->MouseX;
    IBase->FirstPt.Y = IBase->OldPt.Y = IBase->ActiveScreen->MouseY;

    /**  I'm doing this before SIZEVERIFY now
     updateFrame( 1 );
     *** OUT ***/
}

