/*** iequeue.c *****************************************************************
 *
 *  iequeue.c - input event queue handler
 *
 *  $Id: iequeue.c,v 40.0 94/02/15 17:39:39 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"

/*****************************************************************************/

#define D(x)	;
#define DC(x)	;		/* debug cloning */

/*****************************************************************************/

/*
    Lists:
	IEFoodList	-- being processed by other handlers
	IEQueue		-- waiting for Intuition to pass along
	IEFreeList	-- persistent pool
 */

/* this structure is our little secret	*/
struct IENode
{
    struct Node		 ien_Node;	/* can't be MinNode for pools */
    struct InputEvent	 ien_IE;
};

/*****************************************************************************/

#define IEN_ALLOC	(10)	/* this many pre-allocated input events */

/*****************************************************************************/

struct Node *RemHead ();
struct Node *poolGet ();

/*****************************************************************************/

void initIEvents (void)
{
    struct IntuitionBase *IBase = fetchIBase ();

    poolInit (&IBase->IEFreeList, sizeof (struct IENode), IEN_ALLOC);

    NewList (&IBase->IEFoodList);
    NewList (&IBase->IEQueue);
    NewList (&IBase->IECloneList);
}

/*****************************************************************************/

void reclaimIEvents (void)
{
    struct IntuitionBase *IBase = fetchIBase ();
    struct Node *ln;

    while (ln = RemHead (&IBase->IEFoodList))
    {
	poolReturn (ln);
    }

    while (ln = RemHead (&IBase->IECloneList))
    {
	DC (printf ("reclaiming cloned IE node\n"));
	poolReturn (ln);
    }
}

/*****************************************************************************/

/*
 * this allocates an input event for "additional true
 * input tokens."  The only example known is buttonup.
 * The event node is stuck on the IECloneList for
 * bookkeeping.
 *
 * New uses:
 *-don't morph rawkey events converted to
 * mousemoves, since input.device wants to see them
 * come back as they went out.
 *-clone IE for one of mousemove/mousebutton which each
 * rawmouse button event gets decomposed into.
 *-clone IE for compatible POINTERPOS event, since V34 didn't
 * really modify the event (though it was documented as doing so).
 * This fixes Jack Nicklaus Unlimited Golf
 */
struct InputEvent *cloneIEvent (struct InputEvent *ie)
{
    struct IENode *ien;
    struct IntuitionBase *IBase = fetchIBase ();

    DC (printf ("cloneIEvent\n"));
    if (ien = (struct IENode *) poolGet (&IBase->IEFreeList))
    {
	DC (printf ("cIE node at %lx, IE at  %lx\n", ien, &ien->ien_IE));
	AddTail (&IBase->IECloneList, ien);

	ien->ien_IE = *ie;	/* copy-clone */
	ien->ien_IE.ie_NextEvent = NULL;

	return (&ien->ien_IE);
    }
    return (NULL);
}

/*****************************************************************************/

/*
 * called only by appendEvent()
 * remember to fill it in ...
 */
struct InputEvent *queueIEvent (void)
{
    struct IENode *ien;
    struct IntuitionBase *IBase = fetchIBase ();

    D (printf ("queueIEvent\n"));

    if (ien = (struct IENode *) poolGet (&IBase->IEFreeList))
    {
	D (printf ("qIE node at %lx, IE at  %lx\n", ien, &ien->ien_IE));
	AddTail (&IBase->IEQueue, ien);
	return (&ien->ien_IE);
    }
    return (NULL);
}

/*****************************************************************************/

/*
 * return singly linked list of all queued input events.
 * assumes that IEFoodList has already been cleared.
 */
struct InputEvent *returnIEvents (void)
{
    struct IntuitionBase *IBase = fetchIBase ();
    struct IENode *ien;
    struct InputEvent *firstie = NULL;
    struct InputEvent *lastie;

    D (printf ("returnIEvents\n"));

    lastie = (struct InputEvent *) &firstie;

    /* move them from the queue to the return list,
     * linking them also along ie_NextEvent.
     * return pointer to first one
     */
    while (ien = (struct IENode *) RemHead (&IBase->IEQueue))
    {
	D (printf ("ien: %lx\n", ien));
	AddTail (&IBase->IEFoodList, ien);

	lastie = (lastie->ie_NextEvent = &ien->ien_IE);
	lastie->ie_NextEvent = NULL;	/* make sure	*/
    }

    D (printf ("rIE returns %lx\n", firstie));
    return (firstie);
}
