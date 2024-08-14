/*** sscreendrag.c *********************************************************
 *
 * sScreenDrag state processing
 *
 *  $Id: sscreendrag.c,v 38.15 93/01/14 14:24:02 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/


#include "intuall.h"
#include "preferences.h"

#define D(x)	;
#define DSD(x)	;

/* state sScreenDrag
 *
 * global data used:
 *
 * local state data:
 */


/* transition startScreenDrag()
 * called by:
 *	sGadget		- screen drag gadget
 *	lots of others	- itMETADRAG
 * with state data:
 *	ActiveScreen
 * description:
 *
 * note: sdreturn function responsible for turning off drag gadget
 */
startScreenDrag( sdreturn )
int	(*sdreturn)();
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Screen		*hitScreen();
    struct Screen		*dragscreen;

    D( printf("transition function startScreenDrag, cstate: %ld\n", 
	IBase->CurrentState ));

    DSD( printf("screendrag state starting, command %ld, HitScreen %lx\n",
	IT->it_Command, IBase->HitScreen) );

    /* if METADRAG: make sure mouse is over a screen */
    /* Peter 12-Oct-90: hitScreen() now establishes IBase->HitScreen */
    if ( (  IT->it_Command == itMETADRAG )
	&& !(hitScreen()))
    {
	D( printf("sSD bailing out because mouse not on screen\n"));
	(*sdreturn)();
	return;
    }

    DSD( printf("HitScreen after MetaDrag test %lx\n", IBase->HitScreen ));

    dragscreen = IBase->HitScreen;
    /* If you try to drag a "non-draggable" child screen, that's like
     * dragging the parent.
     */
    if ( ( !TESTFLAG( XSC(dragscreen)->PrivateFlags, PSF_DRAGGABLE ) ) &&
    	( XSC(dragscreen)->ParentScreen ) )
    {
	dragscreen = XSC(dragscreen)->ParentScreen;
    }

    IBase->DragScreen = dragscreen;

    /* save original coordinates and stuff */
    SETFLAG( IBase->Flags, INMOUSELIMITS );

    /* Set up mouse limits to ensure legal screen position results.
     * We request that screenLegalPosition() always bound the bottom
     * since child screens default to unbounded, and we don't want
     * the user to be able to shove a child screen off the bottom
     * by dragging the child screen itself.
     */
    /* this limitation keeps coords in dclip range so no overflow 
     * when converted to mouse coords
     * NO WAIT: no they don't!
     */
    /* now return in screen coords	*/
    screenLegalPosition( dragscreen, &IBase->MouseLimits, SPOS_BOUNDEDBOTTOM );

    DSD( dumpRect("GSDRAG: legal position (screen coords):",
	&IBase->MouseLimits) );

    /* account for offset of mouse from screen origin	*/
    offsetRect( &IBase->MouseLimits,
	dragscreen->MouseX, dragscreen->MouseY);
    DSD( dumpRect("GSDRAG: offset screen mouse limits:",&IBase->MouseLimits));

    /* convert mouse limits to mouse coordinates	*/
    /* note limits above are dclip limited so this won't blow up	*/
    /* ZZZ: NO THEY'RE NOT!  need some work here	*/
    scaleScreenMouse( &IBase->MouseLimits, dragscreen );
    DSD(  dumpRect("GSDRAG: mouse limits:", &IBase->MouseLimits) );

    /* keep old/new point in screen mouse coords.	*/
    IBase->OldPt.X = dragscreen->MouseX;
    IBase->OldPt.Y = dragscreen->MouseY;
    DSD( dumpPt("original point, screen coords", IBase->OldPt) );

    IBase->SDragReturn = sdreturn;
    IBase->CurrentState = sScreenDrag;
}


/*
 * state dispatcher
 * state transitions called:
 */
dScreenDrag()
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Window		*itwindow = ( struct Window *) IT->it_Object1;

    switch ( IT->it_Command )
    {
    /* screen drag over: return to previous state */
    case itSELECTUP:	/* end dragging */
	CLEARFLAG( IBase->Flags, INMOUSELIMITS );
	/* restore default mouse travel limitations,
	 * and force mouse back into a legal position.
	 */
	freeMouse();
	updateMousePointer(NULL);
        (*IBase->SDragReturn)();
	return;

    case itMOUSEMOVE:	/* move the screen */
    /* duplicates everything from doDefault() except
     * autoScroll();
     */
#if 1
	IBase->LongMouse.LX += IT->it_LongMouse.LX;
	IBase->LongMouse.LY += IT->it_LongMouse.LY;
	updateMousePointer(NULL);
#else
	doDefault();
#endif
    	screenDrag();	/* below	*/
	return;

    case itSETREQ:	/* defer: can't handle possible state change */
    case itCLEARREQ:	/* defer: can't handle possible state change */
    case itCLOSEWIN:	/* defer */
    case itDEPTHSCREEN:	/* defer */
    case itOPENSCREEN:	/* defer */
    case itCLOSESCREEN:	/* defer */
    case itMOVESCREEN:	/* defer (was skip prior to 3.01) */
    	deferToken();
	return;

    /*** ZZZ: double check default processing	***/
#if 0
    case itMENUDOWN:	/* do default */
    case itMENUUP:	/* do default */
    case itSELECTDOWN:	/* do default */
    case itRAWKEY:	/* do default */
    case itTIMER:	/* do default */
    case itDISKCHANGE:	/* do default */
    case itACTIVATEWIN:	/* do default (swallow: can't handle state change) */
    case itACTIVATEGAD:	/* do default */
    case itMETADRAG:	/* do default */
    case itOPENWIN:	/* do default */
    case itVERIFY:	/* do default */
    case itREMOVEGAD:	/* do default */
    case itSETMENU:	/* do default */
    case itCLEARMENU:	/* do default */
    case itCHANGEWIN:	/* do default */
    case itZOOMWIN:	/* do default */
    case itDEPTHWIN:	/* do default */
    case itNEWPREFS:	/* do default */
    case itMODIFYIDCMP:	/* do default */
    case itCHANGESCBUF:	/* do default */
    case itCOPYSCBUF:	/* do default */
    case itSETPOINTER:	/* do default */
    case itGADGETMETHOD:/* do default */
    case itMODIFYPROP:	/* do default */
#endif
    default:		/* itDEFAULT: do default */
	doDefault();
    }
}

/*
 * for now, can assume we're looking at MOUSEMOVE
 *
 * Let's discuss a bit what happens during screen-drag:
 * itMOUSEMOVE updates IBase->LongMouse based on the input-event,
 * then calls updateMousePointer(NULL), which ensures LongMouse
 * fits into legal mouse limits (note that the legal limits are
 * expanded during drag-state).  uMP() also calls screenMouse()
 * for each screen and windowMouse() for each window.  The
 * sprite is then positioned based on that screen's MouseX/Y and
 * the sprite scaling/hot-spot stuff.
 *
 * We arrive here with the mouse moved but the screen not yet moved.
 * This appears as a difference between dragscreen->Mouse and
 * IBase->OldPt.  We move the screen by that amount.
 */
screenDrag()
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Screen		*dragscreen = IBase->DragScreen;
    struct Point	delta;

    /* calc delta from coords prev. in screen mouse coords	*/
    delta.X = dragscreen->MouseX - IBase->OldPt.X;
    delta.Y = dragscreen->MouseY - IBase->OldPt.Y;

    if (delta.X || delta.Y)
    {
	DSD( printf("new point, mouse coords %ld,%ld\n", IBase->LongMouse.LX,
	    IBase->LongMouse.LY ) );
	DSD( dumpPt("screen drag delta", delta) );

	DSD( printf("IMoveScreen( %lx, %ld, %ld )\n",
		dragscreen, delta.X, delta.Y ) );

	IMoveScreen( dragscreen, &delta, 0 );

	/* In screen coordinates, after MoveScreen(),
	 * mouse should always be in same position.
	 * ... so we don't need this:
	 * IBase->OldPt = dragscreen->MouseX/Y;
	 */
    }
}
