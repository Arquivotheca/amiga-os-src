/*** ihooks.c **************************************************************
 *
 * rom defintions of Intuition internal hooks
 *
 *  $Id: ihooks.c,v 40.0 94/02/15 17:39:51 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 ****************************************************************************/

#include "intuall.h"

/*****************************************************************************/

/* initialized hook data structures internal to Intuition	*/

/* asm-to-C hook interface (hookface.asm) */
ULONG hookEntry ();

/*****************************************************************************/

typedef ULONG (*hookfunc_t) ();

/*****************************************************************************/

hookfunc_t myhookfuncs[] =
{
    (ULONG (*)()) iEdit,
    stringHook,
    propHook,
    boolHook,
};

/*****************************************************************************/

#if 0
/* rom array of initialized Hook structures */
struct Hook IIHooks[] =
{
    {
	{}, hookEntry, iEdit},
    {
	{}, hookEntry, stringHook},
    {
	{}, hookEntry, propHook},
    {
	{}, hookEntry, boolHook},
};

#endif

/*****************************************************************************/

void initHook (struct Hook *hook, ULONG (*dispatch) ())
{
    hook->h_Entry = hookEntry;
    hook->h_SubEntry = dispatch;
    hook->h_Data = (VOID *) fetchIBase ();	/* my context */
}

/*****************************************************************************/

void InitIIHooks (void)
{
    struct Hook *ihook;
    LONG hx;

    ihook = fetchIBase ()->IIHooks;
    for (hx = 0; hx < NUM_IHOOKS; ++hx)
    {
	initHook (ihook, myhookfuncs[hx]);
	ihook++;
    }
}
