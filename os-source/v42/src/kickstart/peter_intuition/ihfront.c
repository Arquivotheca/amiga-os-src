/*** ihfront.c ***************************************************************
 *
 *  ihfront.c -- input handling front end: convert ievents to tokens
 *
 *  $Id: ihfront.c,v 38.13 93/01/12 16:18:28 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/


#define FIX_OVERRUNS	0

#include "intuall.h"
#include "preferences.h"

#include "ihfront_protos.h"

/*---------------------------------------------------------------------------*/

static struct InputToken * newIEToken(struct InputEvent * ie,
                                      enum ITCommand command);

static int convertRawKey(struct InputEvent * ie);

static int rawkeyButtonUpClone(struct InputEvent * ie);

static int createMouseToken(struct InputEvent * ie,
                            UWORD code,
                            enum ITCommand command);

static int rawkeyButtonDown(struct InputEvent * ie);

static int rawkeyCommandKeys(struct InputEvent * ie);

static int rawkeyCursorKeys(struct InputEvent * ie);

static int convertPointerPos(struct InputEvent * ie,
                             struct LongPoint * currpoint);

static int newPointerPos(struct LongPoint * newcoord,
                         struct InputEvent * ie,
                         struct LongPoint * currpoint,
                         int relative,
                         int compatible,
                         struct TabletData * tablet);

static int convertMouse(struct InputEvent * ie);

static int addButtonEvent(struct InputToken * it,
                          struct TabletData * tablet);

static int metaDrag(struct InputEvent * ie);

static int initCursorInc(void);

static int repeatCursorInc(void);

static int kbdVMouse(struct InputEvent * ie);

static int kbdHMouse(struct InputEvent * ie);

/*---------------------------------------------------------------------------*/

/*************************************************************/

#define ACCEL_ON	TESTFLAG( IBase->Preferences->EnableCLI, MOUSE_ACCEL)


#define DTAB(x)	;	/* Tablet action	*/
#define DTAB2(x)	;	/* Tablet action	*/
#define DF(x)	;	/* input front end	*/
#define DK(x)	;	/* keyboard button equiv*/
#define DC(x)	;	/* input front end	*/
#define DKBD(x)	;	/* kbd mouse		*/
#define DUP(x)	;	/* selectup		*/
#define DP(x)	;	/* pointerpos		*/

#define DEBUG_Z		0	/* D(x) x */

/* ************************************************************** */
/* 	utility routines for input event/token conversion	  */
/* ************************************************************** */

/*
 * gets a token and initializes it suitably
 * for use as input token in createInputTokens().
 *
 * Also, queues it, and returns pointer to enqueued token
 * so that caller can fill out some more fields
 */
static struct InputToken	*
newIEToken( ie, command )
struct InputEvent	*ie;
enum ITCommand		command;
{
    struct InputToken	*it;
    struct InputToken *newToken();

    if ( it = newToken() )
    {
	it->it_Command = command;
	it->it_IE = ie;
	/* should never defer: find out the exceptions */
	it->it_Flags = ITF_QUICK | ITF_DONOTDEFER;
	AddTail( &fetchIBase()->TokenQueue, it );
    }
    return ( it );
}

/* ************************************************************** */
/*  Master routine for dispatching to class-specific conversions  */
/* ************************************************************** */

createInputTokens( ie )
struct InputEvent		*ie;
{
    struct InputToken	*it;
    struct LongPoint	currpoint;

    /* need to track anticipated changes to
     * IBase->LongMouse in case I get multiple
     * pointerpos events in a chain
     */
    currpoint = fetchIBase()->LongMouse;

    for ( ; ie; ie = ie->ie_NextEvent )
    {
	DF( printf("-") );
	switch ( ie->ie_Class )
	{
	case IECLASS_RAWKEY:
	    convertRawKey( ie );
	    break;
	case IECLASS_POINTERPOS:
	case IECLASS_NEWPOINTERPOS:
	    DF( printf("cIT pointerpos\n") );
	    convertPointerPos( ie, &currpoint );
	    break;
	case IECLASS_RAWMOUSE:
	    DF( printf("cIT rawmouse\n") );
	    convertMouse( ie );
	    break;
	case IECLASS_TIMER:
	    DF( printf("%%") );
	    newIEToken( ie, itTIMER );
	    break;
	case IECLASS_DISKINSERTED:
	case IECLASS_DISKREMOVED:
	    /* collapse these two, and discern by obj1	*/
	    if ( it = newIEToken( ie, itDISKCHANGE ) )
	    {
		it->it_Object1 = (CPTR) ie->ie_Class;
	    }
	    break;
	default:
	    DF( printf("unknown IECLASS: %lx\n", ie->ie_Class ) );
	    newIEToken( ie, itUNKNOWNIE );
	}
    }
}
	
/* ************************************************************** */
/*	Convert IECLASS_RAWKEY events				  */
/* ************************************************************** */

static
convertRawKey( ie )
struct InputEvent	*ie;
{
    struct InputToken	*it;

    DF( printf("cRK--") );

    rawkeyButtonUpClone( ie );	/* if button-up equiv.,  add a token */

    /* rawkeys are transformed into button, mousemove, or
     * command tokens, but only when left or right
     * AMIGAKEY qualified
     */
    if ( TESTFLAG( ie->ie_Qualifier, AMIGAKEYS )  )
    {
	DK( printf("cRK: amigakey\n") );

	if (   rawkeyButtonDown( ie ) 
	    || rawkeyCommandKeys( ie )
	    || rawkeyCursorKeys( ie ) )	return;
    }

    DF( printf("convertRawKey normal key\n") );

    /* survived the conversion processes.
     * add an itRAWKEY token
     */
    if ( it = newIEToken( ie, itRAWKEY ) )
    {
	it->it_Code = 0;
	MapRawKey( ie, &it->it_Code, 1, NULL );
    }
}


/* ************************************************************** *
 *	IECLASS_RAWKEY Subfunctions				  *
 * ************************************************************** *
 *	rawkeyButtonUpClone()
 *	rawkeyButtonDown()
 *	rawkeyCommandKeys()
 *	rawkeyCursorKeys()
 *
 * Each may assume that ONE of the amigakeys are held down (except
 *  rawkeyButtonUpClone() which gets called regardless).
 *
 * Each returns TRUE if they made the transformation (and
 *	queued the appropriate token).
 */

static
rawkeyButtonUpClone( ie )
struct InputEvent	*ie;
{
    struct IntuitionBase	*IBase = fetchIBase();
    BOOL			doit = FALSE;

    DK( printf("rBUC select Flags %lx\n",
	IBase->Flags & (COM_SELECT+COM_MENU )) );

    if ( TESTFLAG( IBase->Flags, COM_SELECT ) )
    {
	if ( ! TESTFLAG( ie->ie_Qualifier, ALTLEFT ) )
	{
	    DK( printf("com_select up\n") );
	    CLEARFLAG( IBase->Flags, COM_SELECT );
	    createMouseToken( ie, IECODE_UP_PREFIX+IECODE_LBUTTON, itSELECTUP);
	    doit = TRUE;
	}
    }
    if ( TESTFLAG( IBase->Flags, COM_MENU ) )
    {
	if ( ! TESTFLAG( ie->ie_Qualifier, ALTRIGHT ) )
	{
	    DK( printf("com_menu up\n") );
	    CLEARFLAG( IBase->Flags, COM_MENU );
	    createMouseToken( ie, IECODE_UP_PREFIX+IECODE_RBUTTON, itMENUUP);
	    doit = TRUE;
	}
    }

    return ( doit );
}

static
createMouseToken( ie, code, command )
struct InputEvent	*ie;
UWORD		code;
enum ITCommand	command;
{
    struct InputEvent	*cloneie;
    struct InputEvent	*cloneIEvent();

    /*
     * for buttonup:
     *	get the extra token and insert it
     */
    if ( cloneie = cloneIEvent( ie ) )
    {
	cloneie->ie_Class = IECLASS_RAWMOUSE;
	cloneie->ie_SubClass = 0;
	cloneie->ie_Code = code;
	cloneie->ie_EventAddress = NULL;
	newIEToken( cloneie, command );
    }
}

/* Return TRUE if the key event is to be swallowed because
 * it had meaning here...
 */
static
rawkeyButtonDown( ie )
struct InputEvent	*ie;
{
    struct IntuitionBase	*IBase = fetchIBase();
    enum ITCommand		command;
    BOOL			doit = FALSE;
    UWORD			code;

    DK( printf(" rBD ") );

    /* the amiga-left-alt transition */
    if ( TESTFLAG( ie->ie_Qualifier, ALTLEFT ) 
	&&  !TESTFLAG( IBase->Flags, COM_SELECT ) )
    {
	DK( printf("com_select down\n") );
	SETFLAG( IBase->Flags, COM_SELECT );
	command = itSELECTDOWN;
	code = IECODE_LBUTTON;
	doit = TRUE;
    }

    /*
     * the amiga-right-alt transition
     */
    else if ( TESTFLAG( ie->ie_Qualifier, ALTRIGHT )
	&& !TESTFLAG( IBase->Flags, COM_MENU ) )
    {
	DK( printf("com_menu down\n") );
	SETFLAG( IBase->Flags, COM_MENU );
	command = itMENUDOWN;
	code = IECODE_RBUTTON;
	doit = TRUE;
    }

    if ( doit )
    {
	ie->ie_Class = IECLASS_RAWMOUSE;
	ie->ie_Code = code;
	ie->ie_EventAddress = NULL;
	newIEToken( ie, command );
    }
    return ( doit );
}

/* Return TRUE if the key event is to be swallowed because
 * it had meaning here...
 *
 * Peter 27-May-92: Starting now, we call ScreenDepth()
 * directly, instead of injecting an itDEPTHSCREEN token.
 * This allows people to patch Amiga-M/N under 3.0.
 * Note that this relies on the fact that doISM() converts
 * to sendISM() if called on input.device...
 */
static
rawkeyCommandKeys( ie )
struct InputEvent	*ie;
{
    BOOL			doit = FALSE;
    UBYTE			code;
    struct IntuitionBase	*IBase = fetchIBase();

    DF( printf(" rCK ") );

    /* amiga qualified down-stroke */
    if ( TESTFLAG( ie->ie_Qualifier, AMIGALEFT ) &&
	    ( MapRawKey( ie, &code, 1, NULL )  == 1 ) )
    {
	struct Screen *sc;
	ULONG flags;

	code = ToUpper( code );

	if ( code == IBase->FtoBCode )
	{
	    DF( printf("amigaM BEHIND\n") );
	    sc = IBase->FirstScreen;
	    flags = SDEPTH_TOBACK;
	    doit = TRUE;
	}
	else if ( code == IBase->WBtoFCode )
	{
	    DF( printf("amigaN wbfront\n") );
	    sc = NULL;		/* NULL will be the Workbench screen */
	    flags = SDEPTH_TOFRONT;
	    doit = TRUE;
	}
	if ( doit )
	{
	    /* Peter 27-Nov-90:  Ignore screen-shuffling keys if there are
	     * no screens around, such as during BootMenu or before the initial
	     * Shell has opened.
	     */
	    if ( ( IBase->FirstScreen )
		 && ! TESTFLAG( ie->ie_Qualifier, IEQUALIFIER_REPEAT ) )
	    {
		ScreenDepth( sc, flags, NULL );
	    }   
	}
#if DEBUG_Z
this fragment will need updating before it can be used
	    case 'Z':
		if (  ! TESTFLAG( ie->ie_Qualifier, IEQUALIFIER_REPEAT )  )
		{
		    kdebug_hook();
		}
		break;
#endif
    }

    return ( doit );
}

/* Return TRUE if the key event is to be swallowed because
 * it had meaning here...
 */
static
rawkeyCursorKeys( ie )
struct InputEvent	*ie;
{
    struct InputToken		*it;
    BOOL			doit = TRUE;
    WORD			xmove;
    WORD			ymove;
    struct InputEvent		*cloneie;
    struct InputEvent		*cloneIEvent();
    struct IntuitionBase	*IBase = fetchIBase();

    DC( printf("rawkeyCursorKeys: class %lx, code %lx qual %lx\n",
	ie->ie_Class, ie->ie_Code, ie->ie_Qualifier ) );

    /* recognize cursors by scancode */
    switch ( ie->ie_Code )
    {
    case CURSORUP:
	DC( printf("up\n") );
	xmove = 0;
	ymove = -kbdVMouse( ie );
	break;

    case CURSORDOWN:
	DC( printf("down\n") );
	xmove = 0;
	ymove = kbdVMouse( ie );
	break;

    case CURSORLEFT:
	DC( printf("left\n") );
	xmove = -kbdHMouse( ie );
	ymove = 0;
	break;

    case CURSORRIGHT:
	DC( printf("right\n") );
	xmove = kbdHMouse( ie );
	ymove = 0;
	break;

    default:
	/* patch back if not converted */
	doit = FALSE;
    }

    /* I want to change the input event to mirror the
     * the token, but if I change the original, I think
     * input.device screws up.
     */
    if ( doit &&
	(cloneie = cloneIEvent( ie ) ) &&
	(it = newIEToken( cloneie, itMOUSEMOVE ) ) )
    {
	it->it_LongMouse.LX = xmove;
	it->it_LongMouse.LY = ymove;

	cloneie->ie_Class = IECLASS_RAWMOUSE;

#if 0
	/*** DEBUG ***/
	ie->ie_Class = 0x99;
	ie->ie_Code = IECODE_NOBUTTON;
	ie->ie_SubClass = 0;
	ie->ie_X = it->it_LongMouse.LX/IBase->MouseScaleX;
	ie->ie_Y = it->it_LongMouse.LY/IBase->MouseScaleY;
#endif

	cloneie->ie_SubClass = 0;
	cloneie->ie_Code = IECODE_NOBUTTON;

	cloneie->ie_X = it->it_LongMouse.LX/IBase->MouseScaleX;
	cloneie->ie_Y = it->it_LongMouse.LY/IBase->MouseScaleY;
    }

    return ( doit );
}

/* ************************************************************** */
/*	Convert IECLASS_POINTERPOS and IECLASS_RAWMOUSE events	  */
/* ************************************************************** */

/*
 * convert IECLASS_POINTERPOS and IECLASS_NEWPOINTERPOS to itMOUSEMOVE.
 * This version assumes ie_xy in "old" coordinates (640x400).
 * RELATIVEMOUSE qualifier means deltas (same as mousemove,
 * but without acceleration, preferences scale).
 */
static
convertPointerPos( ie, currpoint )
struct InputEvent	*ie;
struct LongPoint		*currpoint;
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct LongPoint		mousecoord;
    BOOL			compatible = FALSE;
    struct TabletData		*tablet = NULL;

    DP( printf("cPP: Class %lx SubClass %lx Code %lx Qual %lx\n",
	ie->ie_Class, ie->ie_SubClass, ie->ie_Code, ie->ie_Qualifier ) );

    if ( ie->ie_Class == IECLASS_POINTERPOS ||
	ie->ie_SubClass == IESUBCLASS_COMPATIBLE )
    {
	compatible = TRUE;
	mousecoord.LX = ie->ie_X * IBase->MouseScaleX;
	mousecoord.LY = ie->ie_Y * IBase->MouseScaleY;
    }

    else if ( ie->ie_SubClass == IESUBCLASS_PIXEL )
    {
	struct IEPointerPixel	*iepp;

	iepp = (struct IEPointerPixel *) ie->ie_EventAddress;
	if ( iepp->iepp_Screen == NULL ) return;

	mousecoord.LX = iepp->iepp_Position.X;
	mousecoord.LY = iepp->iepp_Position.Y;
	if ( !TESTFLAG( ie->ie_Qualifier, IEQUALIFIER_RELATIVEMOUSE ) ) 
	{
	    mousecoord.LX += iepp->iepp_Screen->LeftEdge;
	    mousecoord.LY += iepp->iepp_Screen->TopEdge;
	}
	screenToLongMouse( &mousecoord, iepp->iepp_Screen );
    }

    else if ( ie->ie_SubClass == IESUBCLASS_TABLET )
    {
	struct IEPointerTablet	*iept;
	int			range;
	struct Rectangle	mlimits;

	iept = (struct IEPointerTablet *) ie->ie_EventAddress;

	/* can't handle relativemouse	*/
	CLEARFLAG( ie->ie_Qualifier, IEQUALIFIER_RELATIVEMOUSE );

	DP( printf("newpointerpos tablet\n") );

	/* resulting absolute coordinate is calculated from
	 * "extended mouse limits". 
	 * First try: current mouse limits expanded upwards
	 * to 0.
	 * Peter 3-Apr-91:  Use non-constrained mouse limits instead.
	 */
	mlimits = IBase->FreeMouseLimits;
	if ( mlimits.MinY > 0 ) mlimits.MinY = 0;

	range = rectWidth( &mlimits );
	mousecoord.LX = mlimits.MinX + 
		( range * iept->iept_Value.X )/iept->iept_Range.X;

	range = rectHeight( &mlimits );
	mousecoord.LY = mlimits.MinY + 
		( range * iept->iept_Value.Y )/iept->iept_Range.Y;
    }
    else if ( ie->ie_SubClass == IESUBCLASS_NEWTABLET )
    {
	struct IENewTablet *ient;
	struct Rectangle pointer_rect;
	LONG rangeX, rangeY;
	LONG new;
	struct Screen *ascreen = NULL;

	pointer_rect.MinX = pointer_rect.MinY = 0;
	pointer_rect.MaxX = 639;
	pointer_rect.MaxY = 399;

	/* can't handle relativemouse	*/
	CLEARFLAG( ie->ie_Qualifier, IEQUALIFIER_RELATIVEMOUSE );

	/* We're going to work out the tablet limits based on the
	 * active screen or its parent:
	 */
	if ( IBase->ActiveScreen )
	{
	    struct IBox tempbox;
	    struct Screen *screenFamily();

	    ascreen = screenFamily( IBase->ActiveScreen );

	    /* Start with the active screen's true dimensions,
	     * but ignore that it's pulled down, if at all
	     */
	    tempbox = *(struct IBox *)&ascreen->LeftEdge;
	    DTAB( dumpBox( "ascreen box", &tempbox ) );
	    tempbox.Top -= imax( 0,
		ascreen->TopEdge - XSC(ascreen)->VPExtra->DisplayClip.MinY );
	    boxToRect( &tempbox, &pointer_rect );
	    DTAB( dumpRect( "ascreen rect top-shifted", &pointer_rect ) );

	    /* Grow the screen's dimensions to include DClip */
	    rectHull( &pointer_rect, &XSC(ascreen)->VPExtra->DisplayClip);
	    DTAB2( dumpRect( "now includes dclip", &pointer_rect ) );
	}

	/* Let's do the driver a favor and tell him when the
	 * screen changes.  This allows him to trivially cache
	 * more complex screen info.
	 * LastTabletScreen needs to be reset when the tablet screen
	 * closes or when MEGAFORCE remaking happens (coercion)
	 */
	/* ZZZ: doesn't support "new" notification if multiple players
	 * are sending input-events.
	 */
	new = FALSE;
	if ( IBase->LastTabletScreen != ascreen )
	{
	    IBase->LastTabletScreen = ascreen;
	    new = TRUE;
	}

	ient = (struct IENewTablet *) ie->ie_EventAddress;

	/* Now we have the mouse-travel range for this screen! */
	rangeX = rectWidth( &pointer_rect );
	rangeY = rectHeight( &pointer_rect );

	/* The driver will fill in the ient_ScaledX, ient_ScaledY,
	 * ient_ScaledXFraction, and ient_ScaledYFraction fields
	 * based on the active screen information we pass here.
	 * We perform default scaling if the hook pointer is NULL
	 * or if the hook returns a non-NULL value, which means
	 * "reserved for future use".
	 */
	if ( ( !ient->ient_CallBack ) ||
	    ( callHook( ient->ient_CallBack, ie, ascreen, rangeX, rangeY, new ) ) )
	{
	    ULONG remainder;

	    /* Default processing: take the pointer_rect, and scale based on
	     * the range the tablet gave us to work with:
	     */
	    ient->ient_ScaledX = divMod( rangeX * ient->ient_TabletX,
		ient->ient_RangeX, &remainder );

	    ient->ient_ScaledXFraction = ( remainder << 16 ) / ient->ient_RangeX;

	    ient->ient_ScaledY = divMod( rangeY * ient->ient_TabletY,
		ient->ient_RangeY, &remainder );

	    ient->ient_ScaledYFraction = ( remainder << 16 ) / ient->ient_RangeY;

	    DTAB2( printf("Default scale, X,Y = %ld,%ld\n",
		ient->ient_ScaledX, ient->ient_ScaledY ) );
	}

	DTAB2( printf("Pulldown %ld Shifted back X,Y = %ld,%ld\n", pulldown,
	    ient->ient_ScaledX, ient->ient_ScaledY ) );
	mousecoord.LX = pointer_rect.MinX + ient->ient_ScaledX;
	mousecoord.LY = pointer_rect.MinY + ient->ient_ScaledY;
	/* This one scales the coordinate by the screen ScaleFactor, but
	 * screen LeftEdge/TopEdge is not taken into consideration.  Therefore
	 * we're done as soon as we scale.
	 */
	if ( ascreen )
	{
	    screenToLongMouse( &mousecoord, ascreen );
	}
	else
	{
	    mousecoord.LX *= MOUSESCALEX;
	    mousecoord.LY *= NTSC_MOUSESCALEY;
	}

	DTAB( printf("Final Ticks X,Y = %ld,%ld\n", mousecoord.LX, mousecoord.LY ) );

	/* Here's the catch:  Nobody else needs to know that
	 * struct TabletData is embedded in the end of struct
	 * IENewTablet!
	 */
	tablet = (struct TabletData *) &ient->ient_ScaledXFraction;
    }

    else return;

    newPointerPos( &mousecoord, ie, currpoint,
	TESTFLAG( ie->ie_Qualifier, IEQUALIFIER_RELATIVEMOUSE ), compatible, tablet );
}

/*
 * once you know where (in mouse ticks) you want to position the mouse,
 * call this routine right here.  If you 'relative' is true, then
 * newcoord is taken as a relative offset in mouse ticks.
 */
static
newPointerPos( newcoord, ie, currpoint,  relative, compatible, tablet )
struct LongPoint	*newcoord;
struct LongPoint	*currpoint;
struct InputEvent	*ie;
struct TabletData	*tablet;
{
    struct InputToken	*it;
    struct InputEvent	*maybecloneie;
    struct IntuitionBase	*IBase = fetchIBase();

    /* Peter 8-Mar-91:  Jack Nicklaus Unlimited Golf relies on
     * Intuition-V34's behavior of not modifying the IECLASS_POINTERPOS
     * InputEvent it supplies, even though the docs were very
     * clear in this regard.  So we try to use a copy if possible.
     * If not, we'll use the original anyways.
     */
    if ( ( !compatible ) || ( ! ( maybecloneie = cloneIEvent( ie ) ) ) )
    {
	/* Using the original is better than failing! */
	maybecloneie = ie;
    }

    if ( it = newIEToken( maybecloneie, itMOUSEMOVE ) )
    {
	/* calculate delta to desired tick position */
	it->it_LongMouse.LX = newcoord->LX;
	it->it_LongMouse.LY = newcoord->LY;

	/* Install tablet data if any */
	it->it_Tablet = tablet;

	if ( ! relative )
	{
	    /* need to convert all mousemove things into
	     * some relative coordinates, so I subtract
	     * and update a shadow copy of IBase->LongMouse.
	     * That shadow copy is used to handle multiple
	     * chained input events.
	     */
	    it->it_LongMouse.LX -= currpoint->LX;
	    it->it_LongMouse.LY  -= currpoint->LY;
	    *currpoint = *newcoord;
	}

	/* make consistent input event */
	maybecloneie->ie_Class = IECLASS_RAWMOUSE;
	maybecloneie->ie_SubClass = 0;

	/* convert to equivalent old-style dx/dy for DELTAMOVE customers */
	maybecloneie->ie_X = it->it_LongMouse.LX/IBase->MouseScaleX;
	maybecloneie->ie_Y = it->it_LongMouse.LY/IBase->MouseScaleY;

	/* Peter 30-Jan-91: Turns out that 1.3 used to heed button
	 * transitions for POINTERPOS events as well as RAWMOUSE.
	 * Sounds reasonable enough...
	 *
	 * V39 News flash!  NEWTABLET events send button transitions
	 * too, and with TabletInfo!
	 */
	if ( ( compatible ) || ( tablet ) )
	{
	    addButtonEvent( it, tablet );
	}
	/* Peter 8-Mar-91: Have to leave the code info around
	 * long enough for addButtonEvent() to see it.
	 */
	maybecloneie->ie_Code = IECODE_NOBUTTON;
    }
}

/*
 * convert IECLASS_RAWMOUSE to InputToken(s).
 * Will add itMOUSEMOVE and possibly an extra button token
 * ZZZ: WAIT A MINUTE:  This didn't work.
 *	I was trying to regularize the old voodoo where
 *	a rawmouse event was processed first as a mousemove,
 *	and then as a mouse button event.
 *
 *	The approach I put in here separated into two events,
 *	itMOUSEMOVE and itSELECTUP, for instance.  The problem is
 *	that the itMOUSEMOVE input event still has code == SELECTUP,
 *	which is processed like itSELECTUP by gadgets.  This sux,
 *	since the subsequent itSELECTUP gets passed through subsequently.
 *
 *	Two approaches are: 
 *	1) Clone a new event for the added token, and set the itMOUSEMOVE
 *	one to have its Code set to IECODE_NOBUTTON, and its qualifier
 *	cleared/set properly for the "pre-button event" conditions.
 *
 *	2) Screw it, convert to itSELECTUP, and skip the equivalent
 *	itMOUSEMOVE.  This might suck, if the input device collapses
 *	mouse events, which I assume it does: I'd miss the mouse movement
 *	associated with the up click.  So, mouse up/downs while moving
 *	would suck.
 */
static
convertMouse( ie )
struct InputEvent	*ie;
{
    struct InputToken	*it;
    struct IntuitionBase	*IBase = fetchIBase();

    /*
     * convert to RAWMOUSE or directly to MOUSEMOVE token,
     *
     * coordinate values:
     *	old: 640x400 pixels
     *	raw mouse ticks
     *	screen relative pixels
     *	normalized within mouse-limit boundaries (stdoscan?)
     * check IEQUALIFIER_RELATIVE
     */

    /*
     * accelerate (not for pointerpos guys)
     * scale by prefs scale ratio (not for pointerpos guys)
     * let 'er rip, even if no motion (???)
     *
     * For MOUSEBUTTON events, first put an itMOUSEMOVE
     * token in the queue, then do a mousebuttons.
     */

     /*
      * create an itMOUSEMOVE token for everything.
      * if a button is pressed, will get separate token
      */
    if ( it = newIEToken( ie, itMOUSEMOVE ) )
    {
#if FIX_OVERRUNS
	/* Correct for mouse-counter overruns */
	correctoMouse( ie );
#endif
	/* scale the mouse movement into unused slots */
	if ( ACCEL_ON ) acceloMouse( ie );

	/* ZZZ: what if I don't want ie scaled by acceloMouse()?
	 *  What I'd do is have acceloMouse operate on a Point.
	 */

	it->it_LongMouse.LX = ie->ie_X * IBase->EffectiveScaleFactor.X;
	it->it_LongMouse.LY = ie->ie_Y * IBase->EffectiveScaleFactor.Y;
    }

    addButtonEvent( it, NULL );
}

/* addButtonEvent()
 *
 * If this RAWMOUSE event or old or compatible POINTERPOS event
 * has button transitions, add a button event.
 */

static
addButtonEvent( it, tablet )
struct InputToken	*it;
struct TabletData *tablet;
{
    BOOL		add_a_button;
    enum ITCommand	extracommand;
    struct InputEvent	*cloneie;
    struct InputEvent *ie = it->it_IE;
    struct IntuitionBase *IBase = fetchIBase();

    /* If button, add another event.  Assume true to start */
    add_a_button = TRUE;

    switch ( ie->ie_Code )
    {
    case SELECTUP:
	DUP( printf("add a selectup\n") );
	extracommand = itSELECTUP;
	break;

    case SELECTDOWN:
	extracommand = itSELECTDOWN;
	if ( metaDrag( ie ) )
	{
	    extracommand = itMETADRAG;
	}
	break;

    case MENUDOWN:
	extracommand = itMENUDOWN;
	break;

    case MENUUP:
	extracommand = itMENUUP;
	break;

    case MIDDLEDOWN:
    case MIDDLEUP:
	extracommand = itUNKNOWNIE;	/* should let it get through */
	break;

    default:
	add_a_button = FALSE;
	break;
    }

    /* Clone an event for the extra button, and fudge the
     * InputEvent and InputToken.
     */
    if ( add_a_button )
    {
	/* In 2.0x -> 3.00, we went back and eunuched the
	 * itMOUSEMOVE event, in other words, we removed the
	 * button code and the button qualifier from the
	 * first event, leaving a pure movement event.  Then,
	 * we made a new event for the button information.
	 * 
	 * But, this is WRONG, because the pressure
	 * information associated with that MOUSEMOVE would
	 * indicate that the button transition should have
	 * already occurred.  So we send the button through
	 * first, which involves cloning the input event,
	 * removing the button-part from the original,
	 * creating a it(whatever-button) token, and slipping
	 * that token ahead of the itMOUSEMOVE token.
	 */
	if ( cloneie = cloneIEvent( ie ) )
	{
	    struct InputToken *newit;
	    ie->ie_Code = IECODE_NOBUTTON;
	    if ( newit = newIEToken( cloneie, extracommand ) )
	    {
		newit->it_Tablet = tablet;
	    }
	    /* Move the itMOUSEMOVE token to the end */
	    Remove( it );
	    AddTail( &IBase->TokenQueue, it );
	}
    }
}

#define METAQUALS (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT| \
		IEQUALIFIER_CONTROL | \
		IEQUALIFIER_LCOMMAND | IEQUALIFIER_RCOMMAND | \
		IEQUALIFIER_LALT |  IEQUALIFIER_RALT)

static
metaDrag( ie )
struct InputEvent *ie;
{
    struct IntuitionBase	*IBase = fetchIBase();

    /* Peter 7-Aug-90: Was returning true if all four qualifiers were
     * turned off and a regular SELECTDOWN happened.
     * Now "all qualifiers off" = metadrag off
     */
    return ( (IBase->MetaDragQual) && 
	(ie->ie_Qualifier & METAQUALS) == (IBase->MetaDragQual & METAQUALS) );
}

/* ************************************************************** */
/*	Keyboard mousemove distance calculations		  */
/* ************************************************************** */


/* CURSOR_REPEAT_INIT is the number of ticks the mouse will move per
 * repeat-keystroke, before acceleration kicks in.  CURSOR_STEADY_DELAY
 * is the number of repeat-keystrokes before acceleration kicks in.
 * CURSOR_ACCEL_SLOPE is the amount that the motion will grow per
 * keystroke during acceleration.  CURSOR_MAX_SPEED is the cap on
 * acceleration.
 *
 * Maybe these should be made into factors to be multiplied by
 * IBase->MouseScaleX or MouseScaleY, but exact multiples of pixels
 * matter mostly for single taps, not repeats.
 */

#define CURSOR_STEADY_DELAY	( 20 )
#define CURSOR_ACCEL_SLOPE	(  1 * MOUSESCALEX )
#define CURSOR_MAX_SPEED	( 30 * MOUSESCALEX )
#define CURSOR_SHIFT_MOVE	( 40 * MOUSESCALEX )
#define CURSOR_REPEAT_INIT	(  3 * MOUSESCALEX )

static
initCursorInc( )
{
    struct IntuitionBase	*IBase = fetchIBase();

    /* if ( IBase->CursorSteady == 0 ) */
	IBase->CursorInc = CURSOR_REPEAT_INIT;	/* nah: only second time */
    IBase->CursorSteady = 0;
}

static
repeatCursorInc()
{
    struct IntuitionBase	*IBase = fetchIBase();


    if ( ++IBase->CursorSteady >= CURSOR_STEADY_DELAY )
    {
	DKBD( printf("ramp up\n") );
	if ( (IBase->CursorInc += CURSOR_ACCEL_SLOPE ) > CURSOR_MAX_SPEED )
	{
	    DKBD( printf("maxed out\n") );
	    IBase->CursorInc = CURSOR_MAX_SPEED;
	}
    }
    return ( IBase->CursorInc );
}

/*
 * return increment (in mouse ticks) for vertical mouse
 * motion from keyboard.  Uses qualifier only.
 */
static
kbdVMouse( ie )
struct InputEvent	*ie;
{
    UWORD	qual = ie->ie_Qualifier;
    int	retval;
    struct IntuitionBase	*IBase = fetchIBase();

    if ( TESTFLAG( qual, SHIFTY ) )
    {
	retval = CURSOR_SHIFT_MOVE;
	initCursorInc( );	/* reset repeat speed */
    }
    else if ( TESTFLAG( qual, IEQUALIFIER_REPEAT ) )
    {
	/* retval = IBase->MouseScaleY * 25; */

	retval = repeatCursorInc();

    }
    else	/* one pixel	*/
    {
	if ( IBase->ActiveScreen )
	{
	    DC( printf("try for a single pixel Y\n") );
	    retval = XSC(IBase->ActiveScreen)->ScaleFactor.Y;
	}
	else
	{
	    DC( printf("kVM: no active window\n") );
	    retval = IBase->MouseScaleY;
	}

	initCursorInc( );
    }

    DC( printf("kbdVMouse returning %ld\n", retval ) );
    return ( retval );
}
/*
 * return increment (in mouse ticks) for horizontal mouse
 * motion from keyboard.  Uses qualifier only.
 */
static
kbdHMouse( ie )
struct InputEvent	*ie;
{
    UWORD	qual = ie->ie_Qualifier;
    int	retval;
    struct IntuitionBase	*IBase = fetchIBase();

    DC( printf("kHM: qual: %lx\n", qual ) );

    if ( TESTFLAG( qual, SHIFTY ) )
    {
	retval = CURSOR_SHIFT_MOVE;
	initCursorInc( );	/* reset repeat speed */
    }
    else if ( TESTFLAG( qual, IEQUALIFIER_REPEAT ) )
    {
	/* retval = MOUSESCALEX * 25; */
	retval = repeatCursorInc();
    }
    else	/* one pixel	*/
    {
	if ( IBase->ActiveScreen )
	{
	    DC( printf("try for a single pixel X\n") );
	    retval = XSC(IBase->ActiveScreen)->ScaleFactor.X;
	}
	else
	{
	    DC( printf("kVM: no active window\n") );
	    retval = IBase->MouseScaleX;
	}

	initCursorInc( );
    }

    DC( printf("kbdHMouse returning %ld\n", retval ) );
    return ( retval );
}
