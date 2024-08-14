/*** genv.c *************************************************************
 *
 *  File genv.c: intuition gadget environment utilities
 *	someday this can expand to environments for other objects
 *
 *  $Id: genv.c,v 40.0 94/02/15 17:59:31 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "intuall.h"
#include "gadgetclass.h"

/*****************************************************************************/

#define D(x)	;
#define DLD(x)	;

/*****************************************************************************/

/* For the given gadget with supplied GadgetInfo, and box
 * pre-initialized to the gadget's select box or bounding box,
 * evaluate relativity and stuff the answer back into box.
 */
static void gadgetBoxGrunt (struct Gadget *g, struct GadgetInfo *gi, struct IBox *box)
{
    USHORT flags;		/* cache relativity flags */
    LONG width, height;

    /* relativity is rel to right/bottom, for
     * RELRIGHT/BOTTOM, but width/height for
     * RELWIDTH/HEIGHT
     */
    width = gi->gi_Domain.Width;
    height = gi->gi_Domain.Height;

    /* cheezy optimization */
#define ALLRELFLAGS	(GRELWIDTH|GRELHEIGHT|GRELRIGHT|GRELBOTTOM)

    /* and adjust depending on rel flags */
    if (flags = TESTFLAG (g->Flags, ALLRELFLAGS))
    {
	if (TESTFLAG (flags, GRELWIDTH))
	    box->Width += width;
	if (TESTFLAG (flags, GRELRIGHT))
	    box->Left += --width;
	if (TESTFLAG (flags, GRELHEIGHT))
	    box->Height += height;
	if (TESTFLAG (flags, GRELBOTTOM))
	    box->Top += --height;
    }
}

/*****************************************************************************/

/* Evaluate the gadget's select box (including working out the relativity: */
void gadgetBox (struct Gadget *g, struct GadgetInfo *gi, struct IBox *box)
{
    /* get a copy of what's in the gadget select box: */
    *box = *((struct IBox *) &g->LeftEdge);
    gadgetBoxGrunt (g, gi, box);
}

/*****************************************************************************/

/* needs to be safe if called when (both) layers
 * are already locked
 * NOTE: no longer holds LayerInfo lock, but
 * current customer is only drawGadgets(), with whom
 * I checked: it is OK.
 */
void lockGDomain (struct GListEnv *ge)
{
    BOOL li_locked = FALSE;
    struct IntuitionBase *IBase = fetchIBase ();

    D (printf ("lGD in\n"));
    D (dumpGE ("lockGDomain (no gInfo setup yet)", ge));

    /* if more than one layer, must have linfo before
     * locking two layers.
     * if this task owns the layer lock(s), don't
     * bother with LayerInfo lock, but nest the layer locks
     */
    if ((ge->ge_GZZLayer) &&
	(ge->ge_Layer->Lock.ss_Owner != (struct Task *) FindTask (NULL)))
    {
	LockLayerInfo (&ge->ge_GInfo.gi_Screen->LayerInfo);
	li_locked = TRUE;
    }

    /* Be sure to get the lock if at all possible, which includes
     * all cases of "if I own it", since that criteria is used
     * by unlockGDomain() to decide to ReleaseSemaphore()
     * otherwise, screw it.  deadlocks are fatal, no?
     */
    if (FindTask (0) == IBase->InputDeviceTask)
    {
	/* since intuition doesn't screw up, it isn't deadlocking now ...*/
	LOCKGADGETS ();
    }
    else
	/* ... but I'm not so sure about you */
    {
	AttemptSemaphore (&(IBase->ISemaphore[GADGETSLOCK]));
    }

    LockLayerRom (ge->ge_Layer);

    if (ge->ge_GZZLayer)
    {
	LockLayerRom (ge->ge_GZZLayer);
    }

    if (li_locked)
	UnlockLayerInfo (&ge->ge_GInfo.gi_Screen->LayerInfo);
    D (printf ("lGD out\n"));
}

/*****************************************************************************/

void unlockGDomain (struct GListEnv *ge)
{
    struct IntuitionBase *IBase = fetchIBase ();

    if (ge->ge_GZZLayer)
	UnlockLayerRom (ge->ge_GZZLayer);
    UnlockLayerRom (ge->ge_Layer);

    /* might not have really gotten lock in lockGDomain */
    if ((struct Task *) FindTask (0) == IBase->ISemaphore[GADGETSLOCK].ss_Owner)
    {
	/* we know that if we have the lock now,
	 * that we got it in lockGDomain
	 */
	UNLOCKGADGETS ();
    }
}

/*****************************************************************************/

/* give point in window (or screen) coordinates,
 * get answer in coords rel to gadget itself.
 * (in the case of PROPGADGETS, the caller may wish to further translate
 * to coordinates with respect to the inside of the container).
 */
struct Point *gadgetPoint (g, gi, windowcoord, gadgetcoord, gbox)
struct Gadget *g;
struct GadgetInfo *gi;
struct Point windowcoord;	/* point in display element containing gadg */
struct Point *gadgetcoord;	/* point in gadget rel. coords (container for propgadget) */
struct IBox *gbox;
{
    /* method: pre-transform to coordinate system of gadget environment.
     *		then, translate again to gadget coordsystem */

    /* transform window mouse to requester or inner
     * window layer for !GZZGADGET in G00 window.
     *
     * this is identity for screen or non-GZZ window, */
    transfPoint (&windowcoord, upperLeft (&gi->gi_Domain));

    /* transform from gadget domain to gadget coordinates */
    transfPoint (&windowcoord, upperLeft (gbox));

    *gadgetcoord = windowcoord;
    return (gadgetcoord);
}

/*****************************************************************************/

struct Point *gadgetMouse (struct Gadget *g, struct GadgetInfo *gi, struct Point *mouse, struct IBox *gbox)
{
    if (g->GadgetType & SCRGADGET)
    {
	mouse->X = gi->gi_Screen->MouseX;
	mouse->Y = gi->gi_Screen->MouseY;
    }
    else
    {
	currMouse (gi->gi_Window, mouse);
    }

    /* get mouse coordinate relative to gadget select box
     * FALSE: OBSOLETE: <<or, in case of propgadget, container>> */
    gadgetPoint (g, gi, *mouse, mouse, gbox);

    return (mouse);
}

/*****************************************************************************/

/*** intuition.library/GadgetMouse ***/

void GadgetMouse (struct Gadget *g, struct GadgetInfo *gi, WORD *mouse)
{
    struct IBox gadgetbox;

    gadgetBox (g, gi, &gadgetbox);
    gadgetMouse (g, gi, (struct Point *) mouse, &gadgetbox);
}

/*****************************************************************************/

/*
 * someday, will have classes around hooks
 * THAT DAY IS TODAY.
 */
struct Hook *gadgetHook (struct Gadget *g)
{
    struct Hook *hook;
    struct IntuitionBase *IBase = fetchIBase ();

    /* gadget type specific setup	*/
    switch (g->GadgetType & GTYPEMASK)
    {
	case PROPGADGET:
	    hook = &IBase->IIHooks[IHOOK_PROPG];
	    break;

	case STRGADGET:
	    hook = &IBase->IIHooks[IHOOK_STRINGG];
	    break;

	case CUSTOMGADGET:
	    hook = CUSTOM_HOOK (g);
	    break;

	case BOOLGADGET:
	case 0:		/* system gadgets are "implicitly" bools */
	    hook = &IBase->IIHooks[IHOOK_BOOLG];
	    break;

	default:
	    D (printf ("gadgetInfo: bad gadget type: %lx\n",
		       g->GadgetType & GTYPEMASK));
	    D (Alert (AN_BadGadget));
    }

    return (hook);
}

/*****************************************************************************/

/*
 * sets up the fields in a GadgetInfo which may vary within
 * a given gadget list, from cache of information in a GListEnv
 *
 * There are "per domain" things, e.g., layer, gzz box,
 * and good old per-gadget things, such as its Box. (NOPE!)
 *
 * jimm: 9/2/89: everybody is custom, now moving toward classes.
 */
void gadgetInfo (struct Gadget *g, struct GListEnv *ge)
{
    struct GadgetInfo *gi = &ge->ge_GInfo;
    USHORT type;

    D (printf ("gadgetInfo in g %lx, ge %lx\n", g, ge));

    type = g->GadgetType;

    /* rendering layer and coordinate domain ... */
    /* this is the reason it would have been nice to
     * have TWO lists, for G00 window border gadgets.
     */
    if (TESTFLAG (type, GZZGADGET))
    {
	gi->gi_Layer = ge->ge_GZZLayer;
	gi->gi_Domain = ge->ge_GZZdims;
    }
    else
    {
	gi->gi_Layer = ge->ge_Layer;
	gi->gi_Domain = ge->ge_Domain;
    }

#if 0
    /* Gadget specific stuff ...	*/
    gi->gi_Gadget = g;
#endif

#if 0
    /* get gadget select box, depends on domain,
     * and relative position/sizing
     */
    gadgetRelativity (g, gi);
#endif

#if 0
    gi->gi_Hook = gadgetHook (g);

    /* class specific setup (of per class data) */
    D (printf ("gI: call setupinfo hook\n"));
    callHook (gi->gi_Hook, g, GH_SETUPINFO, gi);
    D (printf ("gI: setupinfo hook returned, gI done\n"));
#endif
}

/*****************************************************************************/

void clearWords (UWORD *ptr, ULONG numwords)
{
    while (numwords--)
	*ptr++ = 0;
}

/*****************************************************************************/

/* calculates the gadget environment for a given gadget.
 * takes as argument a pointer to the window the gadget
 * is associated with.  this pointer may also be a screen
 * gadget, but that is only used for system gadgets.
 * if the gadget is a requester gadget, pass the window
 * the requester is in, the correct requester will be searched
 * for.
 *
 * Sets up "constant" part of GadgetInfo.  Doesn't set up
 * the things done by gadgetInfo, but does all the rest.
 *
 * Doesn't set up GZZ fields for screens or requester, so if you
 * put a GZZGADGET into one of these, you might get an explosion.
 */

/* returns FALSE if requester offscreen, or couldn't be found */
/* Window may actually be a screen */
BOOL gListDomain (struct Gadget *g, struct Window *window, struct GListEnv *ge)
{
    USHORT type;
    BOOL retval = TRUE;
    struct GadgetInfo *gi = &ge->ge_GInfo;	/* for "constant" part */

    DLD (printf ("gLD: enter for gadget %lx, window %lx, ge %lx\n",
		 g, window, ge));

    if (g == NULL)
	return (FALSE);
    type = g->GadgetType;

    /* jeez, I had to write this, below */
    clearWords ((UWORD *) ge, sizeof *ge / 2);

    if (TESTFLAG (type, SCRGADGET))
    {
	DLD (printf ("gLD: screen gadget\n"));
	gi->gi_Screen = (struct Screen *) window;
	gi->gi_Pens.DetailPen = gi->gi_Screen->DetailPen;
	gi->gi_Pens.BlockPen = gi->gi_Screen->BlockPen;
	gi->gi_RastPort = &gi->gi_Screen->RastPort;

	ge->ge_Domain.Width = gi->gi_Screen->Width;
	ge->ge_Domain.Height = gi->gi_Screen->Height;

	ge->ge_Layer = gi->gi_Screen->BarLayer;	/* jimm: 4/15/86 */
    }
    else
	/* windows, requesters, (groups) */
    {
	gi->gi_Window = window;
	gi->gi_Screen = window->WScreen;
	gi->gi_Pens.DetailPen = window->DetailPen;
	gi->gi_Pens.BlockPen = window->BlockPen;
	gi->gi_RastPort = gi->gi_Screen->BarLayer->rp;

	if (TESTFLAG (type, REQGADGET))
	{
	    /* RemoveGList, for one, doesn't know requester */
	    gi->gi_Requester = findGadgetRequest (g, window);

	    DLD (printf ("gLD: reqgadget requester: %lx\n", gi->gi_Requester));

	    /* may be clipped offscreen */
	    if (!(gi->gi_Requester && gi->gi_Requester->ReqLayer))
	    {
		DLD (printf ("gLD: reqgadget: unhappy.\n"));
		retval = FALSE;
		goto OUT;
	    }

	    ge->ge_Domain = *((struct IBox *) &gi->gi_Requester->LeftEdge);

	    /* need requester region rel to outer window,
	     * for mouse coord transformations later
	     */
	    if (TESTFLAG (window->Flags, GIMMEZEROZERO))
	    {
		ge->ge_Domain.Left += window->BorderLeft;
		ge->ge_Domain.Top += window->BorderTop;
	    }

	    ge->ge_Layer = gi->gi_Requester->ReqLayer;
	}
	else
	    /* window */
	{
	    /* window outer dimensions */
	    ge->ge_Domain.Width = window->Width;
	    ge->ge_Domain.Height = window->Height;
	    ge->ge_Layer = window->WLayer;

	    /* if G00, move outer dims to GZZ, get inner dims */
	    if (TESTFLAG (window->Flags, GIMMEZEROZERO))
	    {
		ge->ge_GZZdims = ge->ge_Domain;	/* outer goes in GZZ */

		ge->ge_GZZLayer = (window->BorderRPort) ?
		    window->BorderRPort->Layer : NULL;

		/* domain becomes inner rectangle */
		ge->ge_Domain.Left = window->BorderLeft;
		ge->ge_Domain.Top = window->BorderTop;
		ge->ge_Domain.Width = window->GZZWidth;
		ge->ge_Domain.Height = window->GZZHeight;
	    }
	}
    }

    /* Do I need a Get/Set protocol for GadgetInfo??
     */
#if 1
    /* I at least need to be sure that the "first use" init happens */
    gi->gi_DrInfo = GetScreenDrawInfo (gi->gi_Screen);

    /* only I know that it's OK to do this */
    FreeScreenDrawInfo (gi->gi_Screen, gi->gi_DrInfo);
#else
    gi->gi_DrInfo = &gi->gi_Screen->SDrawInfo;
#endif

  OUT:
    return (retval);
}

/*****************************************************************************/

/* Evaluate the gadget's bounding box (including working out
 * the relativity:
 */
void gadgetBoundingBox (struct Gadget *g, struct GadgetInfo *gi, struct IBox *box)
{
    /* By default, the bounding box is the gadget's select box: */
    *box = *((struct IBox *) &g->LeftEdge);

    /* If this is an extended gadget with a bounding box specified,
     * then use that instead:
     */
    if (TESTFLAG (g->Flags, GFLG_EXTENDED) &&
	TESTFLAG (XGAD (g)->MoreFlags, GMORE_BOUNDS))
    {
	/* get a copy of what's in the gadget select box: */
	*box = *((struct IBox *) &XGAD (g)->BoundsLeftEdge);
    }
    gadgetBoxGrunt (g, gi, box);
}

/*****************************************************************************/

/* find out which requester a REQGADGET lives in
 * assumes you checked for REQGADGET already.
 * returns NULL if no requesters, or
 * can't find gadget in any requester
 */
/* jimm feels real terrible about this, but there is no link
  * from gadget to requester, and i can't see keeping track of
  * IBase->ActiveRequester or something, so this is it.
  */

/*
 * jimm feels even more terrible about this after reading the
 * above.  now that the active requester is no problemo,
 * we should try to remove this nasty little mother f*cker.
 */
struct Requester *findGadgetRequest (struct Gadget *gadget, struct Window *window)
{
    REGISTER struct Requester *r;
    REGISTER struct Gadget *g;

    if (!window)
	return (NULL);

    /* dumpLocks("findGR"); */
    LOCKGADGETS ();

    for (r = window->FirstRequest; r; r = r->OlderRequest)
	for (g = r->ReqGadget; g; g = g->NextGadget)
	    if (g == gadget)
		goto OUT;
  OUT:
    UNLOCKGADGETS ();
    return (r);
}

/*****************************************************************************/

/* set box to position/dims of image */
/* copy from im to box */
struct IBox *getImageBox (struct IBox *im, struct IBox *box)
{
    /* magic is cast */
    /** OUT? *box = *((struct IBox *) (&(im->LeftEdge))); ***/

    *box = *im;

#ifdef TRYTOSHRINK
    box->Left = im->LeftEdge;
    box->Top = im->TopEdge;
    box->Width = im->Width;
    box->Height = im->Height;
#endif

    return (box);
}

/*****************************************************************************/

/* this thing is replaced by a call to getImageBox (with parameters reversed) */
#ifdef TRYTOSHRINK
/* set image dims to those found in box */
void newImageBox (struct Image *im, struct IBox *box)
{
    im->LeftEdge = box->Left;
    im->TopEdge = box->Top;
    im->Width = box->Width;
    im->Height = box->Height;
}
#endif
