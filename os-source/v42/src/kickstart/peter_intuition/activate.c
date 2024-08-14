
/*** activate.c ***************************************************************
 *
 *  File activate.c -- real tricky ActivateGadget() function.
 *
 *  $Id: activate.c,v 38.4 92/11/10 17:02:29 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

/*** #define DEBUG 1 **/

#define D(x)	;
#define DG(x)	;

#include "intuall.h"

/*** intuition.library/ActivateGadget ***/

BOOL
ActivateGadget(gadget, window, request)
struct Gadget *gadget;
struct Window *window;
struct Requester *request;
{
    DG( printf("activategadget %lx, window %lx, request %lx\n",
	gadget, window,  request ) );

    return ( !doISM( itACTIVATEGAD, window, gadget, request )  );
}

/*
 * call only with it == itACTIVATEGAD
 * stuff error code in IT, return it too, for caller to
 * deal with.  0 means success
 */
IActivateGadget( it )
struct InputToken	*it;
{
    struct Hook	*gadgetHook();
    struct Window *window;
    struct Gadget *gadget;
    struct IntuitionBase *IBase = fetchIBase();

    window = ( struct Window *) it->it_Object1;
    gadget = ( struct Gadget *) it->it_Object2;

    DG( printf("IActivateGadget g %lx w %lx\n", gadget, window ) );

    /* Peter 23-Aug-90: Only non-system-type string gadgets and
     * custom gadgets may be affected by ActivateGadget() */
    if ( ( !gadget ) ||
	( ( ( gadget->GadgetType & GTYPEMASK ) != STRGADGET ) &&
	( ( gadget->GadgetType & GTYPEMASK ) != CUSTOMGADGET ) ) ||
	( gadget->GadgetType & GTYP_SYSTYPEMASK ) )
	{
	DG( printf("IAG: failing due to invalid GadgetType\n") );
	return( 1 );	/* error */
	}

    /* try to get gadget environment and info */

    /* use offical gadget env and info, but this would change if
     * this thing might want to abort other gadgets, but not if
     * these next two operations don't succeed.
     */

    if ( ! gListDomain( gadget, window, &IBase->GadgetEnv ) ) 
    {
	DG( printf("IAG: gldomain failed\n") );
	return ( 1 );	/* error */
    }
    gadgetInfo(gadget, &IBase->GadgetEnv );
    IBase->ActiveGadget = gadget;
    return ( 0 );
}
