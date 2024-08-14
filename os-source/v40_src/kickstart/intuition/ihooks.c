/*** ihooks.c **************************************************************
 *
 * rom defintions of Intuition internal hooks
 *
 *  $Id: ihooks.c,v 38.0 91/06/12 14:20:58 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 ****************************************************************************/

#include "intuall.h"

initHook( hook, dispatch )
struct Hook	*hook;
ULONG  		(*dispatch)();
{
    ULONG    hookEntry();

    hook->h_Entry = hookEntry;
    hook->h_SubEntry = dispatch;
    hook->h_Data = (VOID *) fetchIBase();	/* my context */
}

/* initialized hook data structures internal to Intuition	*/

/* asm-to-C hook interface (hookface.asm) */
ULONG	hookEntry();

/* default string gadget editing */
ULONG	iEdit();

/* burned in gadget hooks	*/
ULONG boolHook();
ULONG propHook();
ULONG stringHook();

typedef ULONG (*hookfunc_t)();
hookfunc_t	myhookfuncs[] = {
    iEdit,
    stringHook,
    propHook,
    boolHook,
};

#if 0
/* rom array of initialized Hook structures */
struct Hook	IIHooks[] = {
    { {}, hookEntry, iEdit },
    { {}, hookEntry, stringHook },
    { {}, hookEntry, propHook },
    { {}, hookEntry, boolHook },
};
#endif

InitIIHooks()
{
    struct Hook	*ihook;
    int		hx;

    ihook = fetchIBase()->IIHooks;
    for ( hx = 0; hx < NUM_IHOOKS; ++hx ) 
    {
	initHook( ihook, myhookfuncs[ hx ] );
	ihook++;
    }

}
