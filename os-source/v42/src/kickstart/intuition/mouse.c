/*** mouse.c **************************************************************
 *
 * mouse pointer control
 *
 *  $Id: mouse.c,v 40.0 94/02/15 17:33:45 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 ****************************************************************************/

#include "intuall.h"
#include "preferences.h"

/*****************************************************************************/

#define FIX_OVERRUNS	0

/*****************************************************************************/

#define D(x)	;
#define DFM(x)	;		/* freeMouse	*/

/*****************************************************************************/

/* small, fast, but independent in X and Y */

/* ZZZ: want these to be a preferences option? */
#define accelfactor	(4)
#define threshhold	(3)

/*****************************************************************************/

static void accelOne (WORD *v)
{
    LONG over;

    if ((over = *v + threshhold) < 0)	/* negative *v */
	*v = over * accelfactor - threshhold;
    if ((over = *v - threshhold) > 0)	/* positive *v */
	*v = threshhold + accelfactor * over;
}

/*****************************************************************************/

void acceloMouse (struct InputEvent *ie)
{
    accelOne (&ie->ie_X);
    accelOne (&ie->ie_Y);
}

/*****************************************************************************/

#if FIX_OVERRUNS

/* The mouse-overrun fix depends on a state kept independently
 * for x and y.  The states are:
 *
 *	STATE_NORMAL: device values interpreted as-is
 *	STATE_FASTFORWARD: extremely negative values are presumed
 *		to be wrapped positive values
 *	STATE_FASTBACKWARD: extremely positive values are presumed
 *		to be wrapped negative values
 *
 * There are two interesting thresholds.  The STATE_THRESHOLD is
 * the threshold which must be crossed to change state.  The
 * WRAP_THRESHOLD is that which must be crossed to assume that
 * a wrap has occurred.
 */

/*****************************************************************************/

#define STATE_NORMAL		0
#define STATE_FASTFORWARD	1
#define STATE_FASTBACKWARD	2
#define WRAP_THRESHOLD		63
#define STATE_THRESHOLD		63

/*****************************************************************************/

void correctoMouse (struct InputEvent *ie)
{
    struct IntuitionBase *IBase = fetchIBase ();

    if (TESTFLAG (ie->ie_Qualifier, IEQUALIFIER_RBUTTON))
    {
	correctOne (&ie->ie_X, &IBase->OverrunX);
	correctOne (&ie->ie_Y, &IBase->OverrunY);
    }
}

/*****************************************************************************/

void correctOne (WORD *value, UBYTE *state)
{
    switch (*state)
    {
	case STATE_FASTFORWARD:
	    if (*value < -WRAP_THRESHOLD)
		*value += 256;
	    break;

	case STATE_FASTBACKWARD:
	    if (*value > WRAP_THRESHOLD)
		*value -= 256;
	    break;
    }

    if (*value > STATE_THRESHOLD)
	*state = STATE_FASTFORWARD;
    else if (*value < -STATE_THRESHOLD)
	*state = STATE_FASTBACKWARD;
    else
	*state = STATE_NORMAL;
}
#endif

/*****************************************************************************/

#if USEMOUSEERROR
/* jimm: 6/8/86: delta passed by reference: scaled */
void mouseError (WORD *errorsum, WORD *pos, WORD *delta)
{
    USHORT shift, mask;
    SHORT tick, error;
    struct IntuitionBase *IBase = fetchIBase ();

    if (!*delta)
	return;

    /* jimm: 2/88: scaled both X/Y mouse coords by two
     * new mouse resolution taken as 1280x800 nominal
     */
    *delta *= 2;

    /* how many input delta corresponds to one pos change */
    tick = IBase->Preferences->PointerTicks;

    /* powers of two could be nicer */
    shift = tick >> 1;
    mask = (1 << shift) - 1;

    *errorsum += (*delta & mask);

    *delta >>= shift;		/* scale delta value passed by reference */

    *pos += *delta;

    if (*errorsum >= tick)
    {
	(*pos)++;
	/* jimm: 9/3/86: need to increment *delta, not delta (a pointer) */
	(*delta)++;		/* jimm: *delta must reflect real change */
	*errorsum -= tick;
    }
}
#endif

/*****************************************************************************/

/* sets up normal freedom for mouse travel */
void freeMouse (void)
{
    struct Screen *screen;
    struct IntuitionBase *IBase = fetchIBase ();
    struct Rectangle srect;	/* screen rectangle	*/
    struct Rectangle mlimits;	/* working mouse limits	*/
    void (*rectHullFunc) (struct Rectangle *, struct Rectangle *);

    assertLock ("freeMouse", IBASELOCK);

    /* don't free up if restrictions are in place */
    if (TESTFLAG (IBase->Flags, INMOUSELIMITS))
	return;

    /* Peter 3-Apr-91:  If not INMOUSELIMITS, then it's ok to
     * autoscroll in all directions
     */
    SETFLAG (IBase->ScrollFreedom, OKSCROLL_ALL);
    if (!(screen = IBase->FirstScreen))
    {
#if 0
	mlimits.MinX = mlimits.MaxX = 0;
	mlimits.MinY = mlimits.MaxY = 0;
#endif
	return;
    }

    /*
     * removed special test for Hedley
     */
    degenerateRect (&mlimits);	/* start inside out	*/

    /* We use rectHull() for the first screen, and switch to
     * upperRectHull() after (see later)
     */
    rectHullFunc = rectHull;

    while (screen)
    {
#if 0				/* rethinkVPorts() would set VP_HIDE	*/
	if (!TESTFLAG (screen->Flags, SCREENHIDDEN) &&...
#endif
	if (!TESTFLAG (screen->ViewPort.Modes, VP_HIDE))
	{
	    /* expand mouse limits to enclose screen */

	    /* restrict screen rect to clip region */
	    boxToRect ((struct IBox *) &screen->LeftEdge, &srect);
	    DFM (dumpRect ("fM: screen rectangle", &srect));

	    DFM (dumpRect ("fM: dcliprect ... ", &XSC (screen)->VPExtra->DisplayClip));

	    limitRect (&srect, &XSC (screen)->VPExtra->DisplayClip);
	    DFM (dumpRect ("fM: screen limited to dcliprect", &srect));

	    /* perform this scaling only AFTER limiting
	     * screen rect to something reasonable (dcliprect) */
	    scaleScreenMouse (&srect, screen);
	    DFM (dumpRect ("fM: screen mouse limit", &srect));

	    /* We want to expand the limits to fit the screen.
	     * Originally, we used rectHull() on the IBase->FirstScreen,
	     * and upperRectHull() (which doesn't expand downwards)
	     * on subsequent screens.  However, child screens can now
	     * be dragged so low they don't intersect their DClip.
	     * if such a child screen is the frontmost screen, we
	     * want the full rectHull() to happen for the next screen
	     * instead. */
	    (*rectHullFunc) (&mlimits, &srect);

	    /* After the first screen which intersects its DClip,
	     * switch to using upperRectHull(). */
	    if (srect.MaxY >= srect.MinY)
		rectHullFunc = upperRectHull;
	}
	DFM( else printf("fM: skipping excluded screen %lx\n", screen) );
	screen = screen->NextScreen;
    }

#if 0				/* maybe not	*/
    /* constrain mouse to preferred display limits	*/
    limitRect (&mlimits, MOUSECONSTRAINT (IBase));
#endif

    /* Peter 3-Apr-91:  FreeMouseLimits is a copy of MouseLimits that's
     * never constrained by things like dragging and sizing.
     * IESUBCLASS_TABLET input events need that.
     */
    IBase->FreeMouseLimits = IBase->MouseLimits = mlimits;
    DFM (dumpRect ("freeMouse Limits:", &mlimits));
    DFM (dumpRect ("freeMouse Limits:", &mlimits));
}

/*****************************************************************************/

#ifdef NO_SPRITES
void moverastermouse (void)
{
    struct IntuitionBase *IBase = fetchIBase ();
    LONG x, y;

    /* first remove old mouse */
    if (IBase->Reserved[2] == 1)
	rastermouse (IBase->Reserved[0], IBase->Reserved[1]);
    /* put new one in */
    x = IBase->LongMouse.LX + IBase->AXOffset;
    y = (IBase->LongMouse.LY / 2) + IBase->AYOffset;
    rastermouse (x, y);
    IBase->Reserved[2] = 1;
    IBase->Reserved[0] = x;
    IBase->Reserved[1] = y;
}

/*****************************************************************************/

USHORT mousepattern[2] =
{
    0x5555, 0xaaaa
};

/*****************************************************************************/

void rastermouse (LONG x, LONG y)
{
    struct IntuitionBase *IBase = fetchIBase ();
    struct Screen *s;

    /*kprintf("rastermouse(%lx,%lx)\n",x,y);*/
    if (x < 0)
	return;
    if (y < 0)
	return;
    if (s = IBase->FirstScreen)
    {
	LONG width = 4, height = 4;

	SetAPen (&s->RastPort, -1);
	SetDrMd (&s->RastPort, COMPLEMENT);
	SetAfPt (&s->RastPort, mousepattern, 1);
	if (x + width > MAXDISPLAYCOLUMNS)
	    x = MAXDISPLAYCOLUMNS - x;
	if (y + height > MAXDISPLAYROWS)
	    y = MAXDISPLAYROWS - y;
	/*WritePixel(&s->RastPort,x,y); */
	RectFill (&s->RastPort, x, y, x + width, y + height);
    }
}
#endif

/*****************************************************************************/

void screenMouse (struct Screen *s)
{
    struct LongPoint mouse;
    struct IntuitionBase *IBase = fetchIBase ();

    mouse = IBase->LongMouse;
    longmouseToScreen (&mouse, s);

    s->MouseX = mouse.LX - s->LeftEdge;
    s->MouseY = mouse.LY - s->TopEdge;
}

/*****************************************************************************/

/* assumes screen mouse coords correct */
void windowMouse (struct Window *w)
{
    struct Screen *s = w->WScreen;

    /* add scroll offset	*/
    w->MouseX = s->MouseX - w->LeftEdge;
    w->MouseY = s->MouseY - w->TopEdge;

    /* ZZZ: should account for gzz layer scroll values indep.	*/
    w->GZZMouseX = w->MouseX - w->BorderLeft;
    w->GZZMouseY = w->MouseY - w->BorderTop;
}

/*****************************************************************************/

/*** intuition.library/SetMouseQueue ***/
LONG SetMouseQueue (struct Window *w, ULONG qlen)
{
#if 0
    LONG retval = -1;

    D (printf ("SMQ: w: %lx, qlen: %ld\n", w, qlen));

    if (knownWindow (w))
    {
	retval = XWINDOW (w)->MouseQueueLimit;
	XWINDOW (w)->MouseQueueLimit = qlen;
    }
#else
    LONG retval;

    retval = XWINDOW (w)->MouseQueueLimit;
    XWINDOW (w)->MouseQueueLimit = qlen;
#endif
    return (retval);
}
