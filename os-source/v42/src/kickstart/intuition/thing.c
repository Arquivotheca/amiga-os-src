/*** thing.c ***************************************************************
 *
 *  thing.c routines to handle intuition singly linked lists
 *	all routines handle objects which have a link as their
 *	first element.
 *
 *  NOTE WELL: these routines do NOT lock linked lists (IBASELOCK).
 *  either IBASELOCK or GADGETSLOCK (depends on what it's a list of)
 *  should be locked before calling.
 *
 *  $Id: thing.c,v 40.0 94/02/15 17:53:58 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "ibase.h"

/*****************************************************************************/

struct Thing
{
    struct Thing *nextthing;
};

/*****************************************************************************/

/* assumes the thing is actually in the singly linked thinglist */
/* returns pointer to thing immediately previous to
 * removed thing.  pass prevthing as a pointer to a dummy
 * thing before head of list (e.g. &window->FirstGadget)
 * jimm: 7/30/86: returns nothing.
 */
void delinkThing (struct Thing *thing, struct Thing *prevthing)
{
    while (prevthing)
    {
	if (prevthing->nextthing == thing)
	{
	    /* delink */
	    prevthing->nextthing = thing->nextthing;
	}
	prevthing = prevthing->nextthing;
    }
    return;
}

/*****************************************************************************/

/* returns pointer, sets position of 'thing' if pointer non-null */
struct Thing *previousThing (struct Thing *thing, struct Thing *prevthing, LONG *pos)
{
    LONG count = 0;

    while (prevthing)
    {
	/* See if we found it */
	if (prevthing->nextthing == thing)
	    goto OK;
	count++;
	prevthing = prevthing->nextthing;
    }

    /* Didn't find it */
    count = -1;

  OK:
    /* pass back position if desired */
    if (pos)
	*pos = count;
    return (prevthing);
}

/*****************************************************************************/

/* returns last thing in list */
struct Thing *lastThing (struct Thing *thing)
{
    while (thing->nextthing)
	thing = thing->nextthing;
    return (thing);
}

/*****************************************************************************/

/* if n = 3, this will return pointer to third
 * item in list, starting count from 0 (if
 * n = 0 will return 'thing'.)
 * if realn != NULL, *realn will be set to
 * counter value (which may be less than 'n', if
 * list ends.
 */
struct Thing *nthThing (struct Thing *thing, LONG n, UWORD *realn)
{
    LONG count = 0;

    while (thing->nextthing)
    {
	if (count == n)
	    goto OUT;
	++count;
	thing = thing->nextthing;
    }

  OUT:
    if (realn)
	*realn = count;
    return (thing);
}

/*****************************************************************************/

/* use this for gadgets only, since it does not lock
 * IBASELOCK, but rather assumes GADGETSLOCK is held
 */
struct Thing *previousLink (struct Thing *first, struct Thing *current)
{
    if (first == current)
	return (0L);

    /* assertLock("previousLink", GADGETSLOCK ); */
    /** OUT replaced
    while (first->NextGadget != current) first = first->NextGadget;
    return(first);
    *** OUT  ***/
    return (previousThing (current, first, 0));
}
