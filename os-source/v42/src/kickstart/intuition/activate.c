/*** activate.c ***************************************************************
 *
 *  File activate.c -- real tricky ActivateGadget() function.
 *
 *  $Id: activate.c,v 40.0 94/02/15 17:44:04 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

/*** #define DEBUG 1 **/

#define D(x)	;
#define DG(x)	;

#include "intuall.h"

/*****************************************************************************/

/* call only with it == itACTIVATEGAD stuff error code in IT, return it too,
 * for caller to deal with.  0 means success */
LONG IActivateGadget (struct InputToken *it)
{
    struct IntuitionBase *IBase = fetchIBase();
    struct Hook	*gadgetHook();
    struct Window *window;
    struct Gadget *gadget;

    window = (struct Window *)it->it_Object1;
    gadget = (struct Gadget *)it->it_Object2;

    DG (printf("IActivateGadget g %lx w %lx\n", gadget, window));

    /* Peter 23-Aug-90: Only non-system-type string gadgets and
     * custom gadgets may be affected by ActivateGadget() */
    if ((!gadget) ||
	(((gadget->GadgetType & GTYPEMASK) != STRGADGET) && ((gadget->GadgetType & GTYPEMASK) != CUSTOMGADGET)) ||
	(gadget->GadgetType & GTYP_SYSTYPEMASK))
    {
	/* error */
	DG( printf("IAG: failing due to invalid GadgetType\n") );
	return (1);
    }

    /* try to get gadget environment and info */

    /* use offical gadget env and info, but this would change if
     * this thing might want to abort other gadgets, but not if
     * these next two operations don't succeed. */
    if (!gListDomain (gadget, window, &IBase->GadgetEnv))
    {
	/* error */
	DG (printf("IAG: gldomain failed\n"));
	return (1);
    }

    gadgetInfo (gadget, &IBase->GadgetEnv);
    IBase->ActiveGadget = gadget;
    return (0);
}

/*****************************************************************************/

/*** intuition.library/ActivateGadget ***/
BOOL ActivateGadget (struct Gadget *gadget, struct Window *window, struct Requester *request)
{
    DG (printf("activategadget %lx, window %lx, request %lx\n", gadget, window,  request));
    return (BOOL)(!doISM (itACTIVATEGAD, (CPTR)window, (CPTR)gadget, (ULONG)request));
}
