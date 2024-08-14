/*** dclip.c **************************************************************
 *
 * display (viewport) clipping 
 *
 *  $Id: dclip.c,v 38.16 93/04/13 16:52:55 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 ****************************************************************************/

#include "intuall.h"

#ifndef GRAPHICS_DISPLAYINFO_H
#include <graphics/displayinfo.h>
#endif


#define DQO(x)	;
#define DSLP(x)	;
#define DSCROLL(x)	;

autoScroll( sc )
struct Screen	*sc;
{
    struct LongPoint scmouse, legalscmouse;
    struct Point move;
    struct Rectangle	*dclip;		/* current vport display clip	*/
    struct Rectangle limits;
    struct IntuitionBase	*IBase = fetchIBase();

    if ( !sc || ! TESTFLAG( sc->Flags, AUTOSCROLL) ) return;
    /* If you try to drag a "non-draggable" child screen, that's like
     * dragging the parent.
     */
    if ( ( !TESTFLAG( XSC(sc)->PrivateFlags, PSF_DRAGGABLE ) ) &&
    	( XSC(sc)->ParentScreen ) )
    {
	sc = XSC(sc)->ParentScreen;
    }

    /* Take MouseLimits, and convert to screen coordinates */
    limits = IBase->MouseLimits;
    limits.MinX /= XSC(sc)->ScaleFactor.X;
    limits.MinY /= XSC(sc)->ScaleFactor.Y;
    limits.MaxX /= XSC(sc)->ScaleFactor.X;
    limits.MaxY /= XSC(sc)->ScaleFactor.Y;

    dclip = &XSC(sc)->VPExtra->DisplayClip;

    move.X = move.Y = 0;

    /* IBase->LongMouse contains the updated mouse coordinates
     * in mouse ticks.  Let's work out where on the screen that
     * would be:
     */

    scmouse = IBase->LongMouse;
    longmouseToScreen( &scmouse, sc );

    /* If this point is outside the mouse limits, we must attempt to
     * autoscroll the screen.  So let's constrain the point to the
     * limits, and measure the difference:
     */
    legalscmouse = scmouse;
    limitLongPoint( &legalscmouse, &limits );
    move.X = legalscmouse.LX - scmouse.LX;
    move.Y = legalscmouse.LY - scmouse.LY;


    /* Movement due to the mouse hitting the left edge ( move.X > 0 )
     * Can't move farther than the amount of screen which is off-display.
     * Also can't move if OKSCROLL_LEFT is disallowed.
     */
    if ( move.X > 0 )
    {
	move.X = imin( move.X, imax( 0, dclip->MinX - sc->LeftEdge ) );
	if ( !TESTFLAG( IBase->ScrollFreedom, OKSCROLL_LEFT ) )
	{
	    move.X = 0;
	}
    }

    /* Movement due to the mouse hitting the right edge ( move.X < 0 )
     * Can't move farther than the amount of screen which is off-display.
     * Also can't move if OKSCROLL_RIGHT is disallowed.
     */
    if ( move.X < 0 )
    {
	move.X = imax( move.X, imin( 0, dclip->MaxX -
	    ( sc->LeftEdge + sc->Width - 1 ) ) );
	if ( !TESTFLAG( IBase->ScrollFreedom, OKSCROLL_RIGHT ) )
	{
	    move.X = 0;
	}
    }

    /* Movement due to the mouse hitting the top edge ( move.Y > 0 )
     * Can't move farther than the amount of screen which is off-display.
     * Also can't move if OKSCROLL_UP is disallowed.
     */
    if ( move.Y > 0 )
    {
	/* if screen wants to scroll up too much, restrict */
	move.Y = imin( move.Y, imax( 0, dclip->MinY - sc->TopEdge ) );
	if ( !TESTFLAG( IBase->ScrollFreedom, OKSCROLL_UP ) )
	{
	    move.Y = 0;
	}
    }

    /* Movement due to the mouse hitting the bottom edge ( move.Y < 0 )
     * Can't move farther than the amount of screen which is off-display.
     * Also can't move if OKSCROLL_DOWN is disallowed.
     *
     * New special case:  if a child screen is way off the bottom of
     * the display (because its parent is pulled down too), but the
     * child screen is still active, we MUST suppress autoscroll.
     * The reason is that if we don't, it will scroll up as far as
     * it can, which is to say the parent's top edge.  If the parent
     * is pulled down as far as can be, we would have trapped the
     * parent and child forever.  (Can't drag the child up or down
     * from there, can't reach the parent to drag it).
     */
    if ( move.Y < 0 )
    {
	move.Y = imax( move.Y, imin( 0, dclip->MaxY -
	    ( sc->TopEdge + sc->Height - 1 ) ) );
	if ( ( !TESTFLAG( IBase->ScrollFreedom, OKSCROLL_DOWN ) ) ||
	    ( sc->TopEdge > dclip->MaxY ) )
	{
	    move.Y = 0;
	}
    }

    /* check for actual change */
    if ( *( (ULONG *) &move ) )
    {
	DSCROLL( dumpPt( "scroll move", move ) );

	IMoveScreen( sc, &move, 0 );

	if TESTFLAG( IBase->Flags, SCREENMLIMITS )
	    rethinkMouseLimits();
    }
}

/* (re)establish mouse limits based on screen-relative mouse limits.
 * Constrains screen-limits based on DClip, and sets up scroll freedom
 * to allow scrolling only in those directions where the DClip was
 * the limiting factor, then establishes IBase->MouseLimits.
 */

rethinkMouseLimits()
{
    struct IntuitionBase *IBase = fetchIBase();
    struct Rectangle worklimits;
    struct Screen *sc = IBase->ActiveScreen;
    struct Rectangle *dclip = &XSC(sc)->VPExtra->DisplayClip;

    worklimits = IBase->ScreenMouseLimits;

    offsetRect( &worklimits, sc->LeftEdge, sc->TopEdge );

    /* Prohibit scrolling in all directions until we know otherwise */
    CLEARFLAG( IBase->ScrollFreedom, OKSCROLL_ALL );

    if ( worklimits.MinX < dclip->MinX )
    {
	worklimits.MinX = dclip->MinX;
	SETFLAG( IBase->ScrollFreedom, OKSCROLL_LEFT );
    }

    if ( worklimits.MinY < dclip->MinY )
    {
	worklimits.MinY = dclip->MinY;
	SETFLAG( IBase->ScrollFreedom, OKSCROLL_UP );
    }

    if ( worklimits.MaxX > dclip->MaxX )
    {
	worklimits.MaxX = dclip->MaxX;
	SETFLAG( IBase->ScrollFreedom, OKSCROLL_RIGHT );
    }

    if ( worklimits.MaxY > dclip->MaxY )
    {
	worklimits.MaxY = dclip->MaxY;
	SETFLAG( IBase->ScrollFreedom, OKSCROLL_DOWN );
    }

    /* Set up limits in mouse coordinates */
    scaleScreenMouse( &worklimits, sc );
    IBase->MouseLimits = worklimits;
}
/*** intuition.library/QueryOverscan ***/


/*
 * doesn't fill out ns.Left/Top/Width/Height but just
 * rect (which can be dclip)
 * if ns is  NULL, returns just mouse coordinates
 * if rect is NULL, won't do anything with it
 * if it doesn't recognize ostype, uses OSCAN_TEXT
 *
 * returns non-zero if successful
 */

QueryOverscan( displayID, rect, ostype )
ULONG			displayID;
struct Rectangle	*rect;
WORD			ostype;
{
    return ( displayOScan( NULL, displayID, ostype, rect ) );
}

/* defines legal range of values for screen->Top/LeftEdge
 * returns in Screen coordinates
 */
screenLegalPosition( screen, limitrect, flags )
struct Screen	*screen;
struct Rectangle *limitrect;	/* returned information	*/
WORD	flags;	/* Magic menu can send screen right of legal */
{
    /* We use the VPExtra (i.e. coerced) DClip, since we need to
     * be legal wrt. the actual DClip then in use.
     */
    struct Rectangle	*displayclip = &XSC(screen)->VPExtra->DisplayClip;
    struct Screen *parent = XSC(screen)->ParentScreen;
    int	extrawidth;
    int	extraheight;
    int unbounded;

    DSLP( printf("----- screenLegalPosition ------\n") );

    /* copy DisplayClip	*/
    *limitrect = *displayclip;
    DSLP( dumpRect("sLP: Display Clip:", limitrect) );

    /* freeze ULC to left edge of display clip */
    limitrect->MaxX = limitrect->MinX;

    /* Programmatic screen movement is unbounded on the bottom
     * (and must remain so for compatibility).  The other thing
     * which is unbounded on the bottom are child screens that
     * got pushed far off the bottom when their parent went downward.
     * Note that the parent itself is bounded.  The important thing
     * is we never want to take such a child screen and accidentally
     * apply the lower bound (say during coercion caused by RethinkDisplay()).
     * In fact, 3.00 has this exact bug.
     *
     * So the rule for bottom-bounding is:
     * - If this is a child screen, default to unbounded
     * - If SPOS_UNBOUNDEDBOTTOM is set, force unbounded (IMoveScreen()
     *   sets this for compatibility)
     * - If SPOS_BOUNDEDBOTTOM is set, force bounded (startScreenDrag()
     *   sets this, so that a user can't drag a child screen off the
     *   bottom).
     */
    unbounded = (int) parent;
    if ( TESTFLAG( flags, SPOS_UNBOUNDEDBOTTOM ) )
    {
	unbounded = TRUE;
    }
    if ( TESTFLAG( flags, SPOS_BOUNDEDBOTTOM ) )
    {
	unbounded = FALSE;
    }
    if ( unbounded )
    {
#define VBIG	(7000)
	limitrect->MaxY = VBIG;
    }

    /* For screens which are incompatible with any other screen
     * according to the graphics database (PSF_INCOMPATIBLE), or
     * exclusive by design (PSF_EXCLUSIVE), there's no point letting
     * the user drag them down below the DClip.  This is a good
     * idea anyways, and is required for scrollable Hedley screens.
     * However, if the screen is a PSF_EXCLUSIVE child, then that
     * doesn't apply, since it's part of an exclusive family.
     */
    if ( TESTFLAG( XSC(screen)->PrivateFlags, PSF_INCOMPATIBLE ) ||
	    ( TESTFLAG( XSC(screen)->PrivateFlags, PSF_EXCLUSIVE ) && ( !parent ) ) )
    {
	limitrect->MaxY = limitrect->MinY;
    }

    extrawidth = screen->Width - rectWidth( displayclip );
    extraheight = screen->Height - rectHeight( displayclip );

    /**  let him float about within his display clip	**/
    if (extrawidth >= 0)	/* screen wider than display clip	*/
    {
	limitrect->MinX -= extrawidth;
    }
    else		/* screen fits within display clip	*/
    {
	/* 'extrawidth' is negative	*/
	limitrect->MaxX -= extrawidth;
    }

    /* For magic-menu action, want to let him move right as far
     * as text overscan, even though that's not in the usual
     * legal range.
     */
    if ( TESTFLAG( flags, SPOS_MENUSNAPRANGE ) )
    {
	limitrect->MaxX = imax( 0, limitrect->MaxX );
    }

    if (extraheight > 0) limitrect->MinY -= extraheight;
    /* already bottomed out	*/

    /* If this screen is a child screen, don't let the top edge
     * be higher than the parent's.  We have no constraints to
     * apply to the left edge, because draggable children are
     * horizontally independent of the parent (as are non-draggable
     * children sent to us with SPOS_FORCEDRAG), and non-draggable
     * without SPOS_FORCEDRAG are locked to their current spot if
     * possible, just before exiting from here.
     */
    DSLP( dumpRect("screenLegalPosition before parent test:", limitrect) );
    if ( parent )
    {
	limitrect->MinY = imax( limitrect->MinY, parentOriginY( screen, parent ) );
    }

    DSLP( dumpRect("screenLegalPosition before drag test:", limitrect) );

    if ( !canSlide( screen, TESTFLAG( flags, SPOS_FORCEDRAG ) ) )
    {
	/* For non-draggable screens, the screen's legal position
	 * is the exact point where it already is, with no movement
	 * allowed, provided that point is legal in the first place.
	 * So let's set the limitrect to a single point, which is the
	 * nearest legal position to screen->LeftEdge/TopEdge
	 */
	struct Point ulcorner;
	ulcorner = *((struct Point *)&screen->LeftEdge);
	limitPoint( &ulcorner, limitrect );
	limitrect->MinX = limitrect->MaxX = ulcorner.X;
	limitrect->MinY = limitrect->MaxY = ulcorner.Y;
    }

    DSLP( dumpRect("screenLegalPosition after drag test:", limitrect) );
}
