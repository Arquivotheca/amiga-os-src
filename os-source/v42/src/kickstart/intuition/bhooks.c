/*** bhooks.c ****************************************************************
 *
 *  Boolean Gadget Hooks
 *
 *  $Id: bhooks.c,v 40.0 94/02/15 17:45:12 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1988, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include "gadgetclass.h"

#ifndef HARDWARE_BLIT_H
#include <hardware/blit.h>
#endif

/*****************************************************************************/

#define D(x)	;
#define DEND(x)	;

/*****************************************************************************/

static ULONG boolHitTest (struct Gadget *g, struct gpHitTest *cgp)
{
    struct BoolInfo *bi = BI (g);
    ULONG retval = GADGETON;
    struct IBox gadgetbox;

    /* masked gadgets must pass MASKTEST, too */
    if (TESTFLAG (g->Activation, BOOLEXTEND) && TESTFLAG (bi->Flags, BOOLMASK))
    {
	/* Get the gadget box */
	gadgetBox (g, cgp->gpht_GInfo, &gadgetbox);

	/* See if it is inside the mask */
	if (!MASKTEST (cgp->gpht_Mouse, gadgetbox.Width, bi->Mask))
	{
	    /* must be 0 or GADGETON */
	    retval = 0;
	}
    }
    return (retval);
}

/*****************************************************************************/

/* complement area in rastport within box, subject to mask */
static void xormask (struct RastPort *rp, struct IBox *box, UWORD * mask)
{
    struct BitMap bmap;
    short depth;

    /* set up bitmap with mask as all planes */
    InitBitMap (&bmap, depth = rp->BitMap->Depth, box->Width, box->Height);
    while (depth--)
	bmap.Planes[depth] = (PLANEPTR) mask;

    /* complement blit */
    /* rp->Mask = 0xFF; already */
    BltBitMapRastPort (&bmap, 0, 0, rp, box->Left, box->Top, box->Width, box->Height, ANBC | ABNC);
}

/*****************************************************************************/

/* does XOR highlighting to a boolean gadget.
 * toggleBoolGadget requires that this is a no-op for GREDRAW_TOGGLE and GADGHNONE */
static void toggleBoolHighlight (struct RastPort *rp, struct Gadget *g, struct IBox *gbox)
{
    struct BoolInfo *bi;
    void (*boxfunc) (struct RastPort *,LONG,LONG,LONG,LONG);

    boxfunc = (void (*)(struct RastPort *,LONG,LONG,LONG,LONG)) outbox;

    switch (g->Flags & GADGHIGHBITS)
    {
	case GADGHCOMP:
	    bi = (struct BoolInfo *) g->SpecialInfo;

	    /* call xorbox */
	    if ((g->Activation & BOOLEXTEND) && (bi->Flags & BOOLMASK))
	    {
		/* complement through user-supplied bitmask */
		xormask (rp, gbox, bi->Mask);
		break;
	    }
	    boxfunc = (void (*)(struct RastPort *,LONG,LONG,LONG,LONG)) xorbox;

	case GADGHBOX:
	    /* call outbox */
	    boxModernize (boxfunc, rp, gbox);
    }
}

/*****************************************************************************/

/* toggleBoolGadget requires that this is a no-op for GREDRAW_TOGGLE and GADGHNONE */
static void boolVisuals
    (struct RastPort *rp, struct Gadget *g, struct GadgetInfo *gi, LONG redraw_mode, struct IBox *gbox)
{
    if (redraw_mode == GREDRAW_TOGGLE)
    {
	/* XOR stuff */
	D (printf ("toggling xor gadget\n"));
	toggleBoolHighlight (rp, g, gbox);
    }
    else if (redraw_mode == GREDRAW_REDRAW)
    {
	/* redraw in highlighted state	*/
	D (printf ("toggling redraw gadget\n"));
	commonGadgetRender (rp, g, gi, gbox, FALSE);
	commonGadgetTextAndGhost (rp, g, gi, gbox);
    }
    /* nothing for GREDRAW_UPDATE */
}

/*****************************************************************************/

static void boolRender (struct Gadget *g, struct gpRender *cgp)
{
    struct IBox gadgetbox;

    /* don't redraw image if a simple toggle is called for */
    gadgetBox (g, cgp->gpr_GInfo, &gadgetbox);

    boolVisuals (cgp->gpr_RPort, g, cgp->gpr_GInfo, cgp->gpr_Redraw, &gadgetbox);
}

/*****************************************************************************/

static void toggleBoolGadget (struct Gadget *g, struct GadgetInfo *gi, struct IBox *gbox)
{
    struct RastPort *rp;
    LONG redraw_mode;

    /* Get a pointer to the rastport */
    rp = ObtainGIRPort (gi);

    /* Toggle selection */
    g->Flags ^= SELECTED;

    /* Redraw if gadghimage, toggle if comp/box. nothing, if gadghnone */
    if ((g->Flags & GADGHIGHBITS) == GADGHIMAGE)
	redraw_mode = GREDRAW_REDRAW;
    else
	redraw_mode = GREDRAW_TOGGLE;	/* no-op for GADGHNONE */

    /* Update the visuals */
    boolVisuals (rp, g, gi, redraw_mode, gbox);

    /* Free the rastport */
    FreeGIRPort (rp);
}

/*****************************************************************************/

static ULONG boolGoActive (struct Gadget *g, struct gpInput *cgp)
{
    struct IntuitionBase *IBase = fetchIBase ();

    gadgetBox (g, cgp->gpi_GInfo, &IBase->ActiveGBox);

    /* set SELECTED and draw */
    toggleBoolGadget (g, cgp->gpi_GInfo, &IBase->ActiveGBox);

    /* NOTE: GADGETON is used to detect transitions as the pointer
     * rolls on and off the gadget, and is used to know
     * that this gadget should be turned off. */
    SETFLAG (IBase->Flags, GADGETON);	/* am over gadget */

#if 1				/* tighten this up */

    /* tell him GMR_VERIFY because I have happily completed
     * my GADGIMMEDIATE action, rather than "refused to go
     * active.  No GADGETUP message will be sent unless
     * the RELVERIFY Activation flag is set (see ghReturn in
     * sgadget.c). */
    return (ULONG) (TESTFLAG (g->Activation, RELVERIFY) ? GMR_MEACTIVE : (GMR_NOREUSE | GMR_VERIFY));
#else
    /* handle non-relverify gadgets quicklike */
    if (!TESTFLAG (g->Activation, RELVERIFY))
    {
	return (GMR_NOREUSE);
    }
    return (GMR_MEACTIVE);
#endif
}

/*****************************************************************************/

static void boolGoInactive (struct Gadget *g, struct gpGoInactive *cgp)
{
    struct IntuitionBase *IBase = fetchIBase ();

    D (printf ("goinactive\n"));

    /* ZZZ: this doesn't support abort operations very well */
    if (TESTFLAG (IBase->Flags, GADGETON))
    {
	/* turn him off, since he is not to be left SELECTED */
	if (!TESTFLAG (g->Activation, TOGGLESELECT))
	{
	    toggleBoolGadget (g, cgp->gpgi_GInfo, &IBase->ActiveGBox);
	}
    }
}

/*****************************************************************************/

static ULONG boolHandleInput (struct Gadget *g, struct gpInput *cgp)
{
    struct IntuitionBase *IBase = fetchIBase ();
    struct GadgetInfo *gi = cgp->gpi_GInfo;

    /* transition: gadgeton has changed	*/

    /** ZZZZ call my own hit routine **** with what mouse? **/

    if ((IBase->Flags ^ hitGGrunt (g, gi, &IBase->ActiveGBox, GM_HITTEST)) & GADGETON)
    {
	IBase->Flags ^= GADGETON;	/* toggle gadgeton */
	toggleBoolGadget (g, gi, &IBase->ActiveGBox);
    }

    if (isSelectup (cgp->gpi_IEvent))
    {
	if (!TESTFLAG (IBase->Flags, GADGETON))
	{
#if 1				/* this should do the trick */
	    DEND (printf ("bool done, no verify, saying reuse\n"));
	    return (GMR_REUSE);
#else
	    /* ZZZ: I don't like this here very much	*/
	    activeEvent (IECLASS_RAWMOUSE, SELECTUP);
	    return (GMR_NOREUSE);
#endif
	}
	DEND (printf ("bool done, saying noreuse\n"));
	return (GMR_NOREUSE | GMR_VERIFY);
    }
    return (GMR_MEACTIVE);
}

#if 0
/*****************************************************************************/

void stubReturn ();

PFU boolHookTable[] =
{
    boolHitTest,		/* GM_HITTEST	*/
    boolRender,			/* GM_RENDER	*/
    boolGoActive,		/* GM_GOACTIVE	*/
    boolGoInactive,		/* GM_GOINACTIVE	*/
    boolHandleInput,		/* GM_HANDLEINPUT	*/
    stubReturn,			/* GM_SETUPINFO	*/
};

#endif

/*****************************************************************************/

ULONG boolHook (struct Hook *h, struct Gadget *g, ULONG * cgp)
{
    ULONG retval = 0;

    switch ((WORD) * cgp)
    {
#if 0
	case GM_SETUPINFO:
	    retval = 0;
	    break;
#endif
	case GM_HITTEST:
	    retval = boolHitTest (g, (struct gpHitTest *) cgp);
	    break;

	case GM_HELPTEST:
	    retval = GMR_HELPHIT;
	    break;

	case GM_RENDER:
	    boolRender (g, (struct gpRender *) cgp);
	    break;

	case GM_GOACTIVE:
	    retval = boolGoActive (g, (struct gpInput *) cgp);
	    break;

	case GM_GOINACTIVE:
	    boolGoInactive (g, (struct gpGoInactive *) cgp);
	    break;

	case GM_HANDLEINPUT:
	    retval = boolHandleInput (g, (struct gpInput *) cgp);
	    break;

    }
    return (retval);
}
