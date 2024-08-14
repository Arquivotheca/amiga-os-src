
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
 *  $Id: thing.c,v 38.1 92/04/07 18:00:19 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "ibase.h"

struct Thing {struct Thing *nextthing;};

/* assumes the thing is actually in the singly linked thinglist */
/* returns pointer to thing immediately previous to
 * removed thing.  pass prevthing as a pointer to a dummy 
 * thing before head of list (e.g. &window->FirstGadget)
 * jimm: 7/30/86: returns nothing.
 */
struct Thing *
delinkThing(thing, prevthing)
struct Thing *thing, *prevthing;
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

/* returns pointer, sets position of 'thing' if pointer non-null */
struct Thing *
previousThing(thing, prevthing, pos)
struct Thing *thing;	/* thing to find */
struct Thing *prevthing;/* head of list to find it in */
short int	*pos;		/* set to position if non-null */
{
    int count = 0;

    while(prevthing)
    {
	if (prevthing->nextthing == thing)	/* found it */
	{
	    goto OK;
	}
	count++;
	prevthing = prevthing->nextthing;
    }
    count = -1;
OK:
    if (pos)
    {
	*pos = count;	/* pass back position if desired */
    }
    return ((struct Thing *) prevthing);
}

/* returns last thing in list */
/* APTR */	
struct Thing *
lastThing(thing)
struct Thing *thing;
{
    while (thing->nextthing) thing = thing->nextthing;
    return ((struct Thing *) thing);
}

/* if n = 3, this will return pointer to third
 * item in list, starting count from 0 (if 
 * n = 0 will return 'thing'.)
 * if realn != NULL, *realn will be set to 
 * counter value (which may be less than 'n', if
 * list ends.
 */
struct Thing *
nthThing(thing, n, realn)
struct Thing *thing;
int	n;	/* which thing */
short int	*realn;	/* if non-NULL, put real answer there */
{
    int	count = 0;

    while (thing->nextthing) 
    {
	if (count == n) goto OUT;
	++count;
	thing = thing->nextthing;
    }
OUT:
    if (realn) *realn = count;
    return ((struct Thing *) thing);
}


/* use this for gadgets only, since it does not lock
 * IBASELOCK, but rather assumes GADGETSLOCK is held
 */
struct Thing *previousLink(first, current)
struct Thing *first, *current;
{
    if (first == current) return(0L);

    /* assertLock("previousLink", GADGETSLOCK ); */
    /** OUT replaced
    while (first->NextGadget != current) first = first->NextGadget;
    return(first);
    *** OUT  ***/
    return(previousThing(current, first, 0));
}

