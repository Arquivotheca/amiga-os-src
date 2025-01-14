/*** pubscreen.c ***************************************************************
 *
 *  pubscreen.c -- shared public screen support
 *
 *  $Id: pubscreen.c,v 40.0 94/02/15 17:30:14 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "intuall.h"
#include "exec/memory.h"

/*****************************************************************************/

#define D(x)	;
#define DVC(x)	;

/*****************************************************************************/

extern STRPTR WBPUBNAME;

/*****************************************************************************/

/* Like FindName(), but case-insensitive */
static struct Node *findNameNC (struct List *list, STRPTR name)
{
    register struct Node *ln;

#if 1
    for (ln = list->lh_Head; ln->ln_Succ; ln = ln->ln_Succ)
    {
	if (Stricmp (ln->ln_Name, name) == 0)
	    return ln;
    }
    return NULL;
#else
    ln = list->lh_Head;
    while ((ln->ln_Succ) && Stricmp (ln->ln_Name, name))
	ln = ln->ln_Succ;
    if (!ln->ln_Succ)
	ln = NULL;
    return (ln);
#endif
}

/*****************************************************************************/

/* New news: caller CANNOT have IBASELOCK, since openSysScreen
 * needs STATELOCK (doISM in OpenScreenTagList).
 *
 * returns with VisitorCount++
 */
static struct PubScreenNode *findPubScreen (UBYTE *name)
{
    struct PubScreenNode *psn;
    struct Screen *wbscreen;

    /* bump is no-op if psn == NULL */
    LOCKIBASE ();
    bumpPSNVisitor (psn = (struct PubScreenNode *) findNameNC (&fetchIBase ()->PubScreenList, name));

    /* Peter 26-Jun-91: Now fails if screen is private
     * (simplifies code elsewhere)
     */
    if ((psn) && TESTFLAG (psn->psn_Flags, PSNF_PRIVATE))
    {
	decPSNVisitor (psn);
	psn = NULL;
    }
    UNLOCKIBASE ();

    if (!psn && jstreq (name, WBPUBNAME))
    {
	/* open WB screen, if that's what they're looking for */
	if (wbscreen = openSysScreen (WBENCHSCREEN))
	{
	    /* openSysScreen returned with VC++	*/
	    psn = XSC (wbscreen)->PSNode;
	}
	else
	{
	    /* Peter 14-Nov-90: Mostly fix race condition.  findNameNC()
	     * could have failed if WB wasn't open, then someone came
	     * along and opened Workbench.  openSysScreen() fails
	     * because WB is now there!  LockPubScreen("Workbench")
	     * was failing.  So we try again to find the Workbench screen.
	     * This should catch any but the most pathological cases.
	     * (I've seen this bug happen).
	     */
	    LOCKIBASE ();
	    bumpPSNVisitor (psn = (struct PubScreenNode *)
			    findNameNC (&fetchIBase ()->PubScreenList, name));
	    UNLOCKIBASE ();
	}
    }
    return (psn);
}

/*****************************************************************************/

/*
 * A NewWindow is declared VISITOR.  This routine
 * determines the screen to open the window on depending
 * on the values of PubScreenFlags and PubScreenName.
 * See intuition.h for algorithm.
 *
 * If visitor is asking for workbench, uses FALLBACK
 * to DefaultPubScreen rather than opening workbench.
 * Only opens workbench is DefaultPubScreen is NULL.
 *
 * returns with VisitorCount++
 */
struct Screen *windowPubScreen (UBYTE *pubname, BOOL fallback)
{
    struct PubScreenNode *psn;
    struct Screen *ps;

    /* LOCKIBASE(); VC++ is good enough */

    if (pubname)		/* not default	*/
    {
	/* look for named pub screen node	*/
	/* this will open WB if necessary	*/
	psn = findPubScreen (pubname);
	/* does VC++, also opens "Workbench" */

	/* if no find or private     */
	if (!psn)
	{
	    ps = (fallback) ? defaultPubScreen () : NULL;
	    /* does VC++, also opens wb if needed */
	}
	else
	    /* find.  not marked private       */
	{
	    ps = psn->psn_Screen;
	}
    }
    else
	/* wants default, pure and simple	*/
    {
	ps = defaultPubScreen ();	/* assumes it's not private	*/
	/* does VC++, also opens wb if needed */
    }

    /* UNLOCKIBASE(); */
    return (ps);
}

/*****************************************************************************/

/*
 * returns default pub screen, trying to open workbench if
 * that is it.
 *
 * Don't call with IBASELOCK, since I might have to open a screen
 *
 * returns with VisitorCount++
 */
struct Screen *defaultPubScreen (void)
{
    struct Screen *dpscreen = NULL;
    struct IntuitionBase *IBase = fetchIBase ();

    /* do this so that IBASELOCK isn't held around openSysScreen().  jimm: 6/6/90. */
    LOCKIBASE ();

    /* atomic: DPS won't change out from under us */
    if (bumpPSNVisitor (IBase->DefaultPubScreen))
    {
	/* now VC++ keeps it open, whether it stays default or not */
	dpscreen = IBase->DefaultPubScreen->psn_Screen;
    }
    UNLOCKIBASE ();

    return (dpscreen ? dpscreen : openSysScreen (WBENCHSCREEN));
}

/*****************************************************************************/

/* initPubScreen() performs a uniqueness test on the desired name,
 * then initialized the PubScreenNode if all goes well.
 *
 * The locking issues around the uniqueness test are tricky,
 * and there was a hole in V36-V39.  We want to call the uniqueness
 * test early in OpenScreen(), so we can fail quickly and without any
 * flashing.  However, we can't link the pubscreen node onto the
 * PubScreenList until the screen is fully open, since apps can access
 * that list.  Also, we can't hold the IBASELOCK across the guts of
 * OpenScreen().  Thus, there was a time between the calls to
 * initPubScreen() and linkPubScreen() where a second screen of the
 * same name could pass the uniqueness test in initPubScreen.
 * The fix I chose for V40 was to create a second list, the
 * PendingPubScreens list, and initPubScreen() puts pubscreen nodes
 * there while still inside the same locking session as the uniqueness
 * test, which now has two lists to examine.
 */

struct PubScreenNode *initPubScreen (char *pubname, struct Task *pubtask, UBYTE pubsig, LONG *errorptr)
{
    struct PubScreenNode *pubnode;
    LONG psnsize;
    struct IntuitionBase *IBase = fetchIBase ();
    struct Task *FindTask ();

    /* Name uniqueness check.  This happens under the IBASELOCK.
     * Check the PubScreenList, and also the list of PendingPubScreens,
     * for a different screen whose name is the same.
     */
    LOCKIBASE ();
    pubnode = (struct PubScreenNode *) findNameNC (&IBase->PubScreenList, pubname);
    if (!pubnode)
    {
	/* No existing collisions.  Is one pending? */
	pubnode = (struct PubScreenNode *) findNameNC (&IBase->PendingPubScreens, pubname);
    }
    if (!pubnode)
    {
	/* Good news: no name collision now or pending */
	/* add string buffer after node	*/
	psnsize = sizeof (struct PubScreenNode) + strlen (pubname) + 1;

	pubnode = (struct PubScreenNode *) AllocVec (psnsize,
						     MEMF_CLEAR | MEMF_PUBLIC);
	if (pubnode)
	{
	    pubnode->psn_Size = psnsize;

	    pubnode->psn_SigTask = pubtask;
	    pubnode->psn_SigBit = pubsig;

	    /* use current task if valid sig specified (but not task)	*/
	    if (pubsig != -1 && !pubnode->psn_SigTask)
		pubnode->psn_SigTask = FindTask (0L);
	    pubnode->psn_Node.ln_Name = (UBYTE *) & pubnode[1];
	    jstrncpy ((char *)&pubnode[1], (char *)pubname, MAXPUBSCREENNAME);

	    /* start 'em out private	*/
	    SETFLAG (pubnode->psn_Flags, PSNF_PRIVATE);
	    AddTail (&IBase->PendingPubScreens, pubnode);

	    D (printf ("iPS: pub screen init first part done.\n"));

	    /* still need to back link with screen and add to list */
	}
	else
	{
	    *errorptr = OSERR_NOMEM;
	    /* Going to return pubnode, which is already NULL */
	}
    }
    else
    {
	*errorptr = OSERR_PUBNOTUNIQUE;
	/* Going to return pubnode, which needs to be NULLed */
	pubnode = NULL;
    }
    UNLOCKIBASE ();
    return (pubnode);
}

/*****************************************************************************/

/*
 * called only by OpenScreen: has IBASELOCK (see initPubScreen() for
 * complications of the uniqueness test)
 */
void linkPubScreen (struct Screen *sc, struct PubScreenNode *pubnode)
{
    struct IntuitionBase *IBase = fetchIBase ();

    /* links	*/
    pubnode->psn_Screen = sc;
    XSC (sc)->PSNode = pubnode;
    /* The node itself is on the PendingPubScreens list, so we
     * need to remove it from there...
     */
    Remove (pubnode);
    AddTail (&IBase->PubScreenList, pubnode);
}

/*****************************************************************************/

/* MUST be called inside LOCKIBASE!
 * atomic test-and-commit for the removal of a pubscreen
 *	check for no visitors (try makePrivate)
 *	delink psnode
 *	clear default node, if this is it
 *	free pub node
 *
 * returns FALSE if you may not close the screen.
 */
LONG freePubScreenNode (struct PubScreenNode *psn)
{
    struct IntuitionBase *IBase = fetchIBase ();
    LONG retval = TRUE;

    if (psn)
    {
	assertLock ("freePubScreenNode", IBASELOCK);

	if (psn->psn_VisitorCount)
	{
	    DVC (printf ("freePSN: deny screen closing, count = %ld\n",
			 psn->psn_VisitorCount));

	    retval = FALSE;	/* ! */
	}
	else
	{
	    Remove (psn);

	    /* was this node's screen the default?	*/
	    if (IBase->DefaultPubScreen == psn)
	    {
		IBase->DefaultPubScreen = NULL;
	    }

	    FreeVec (psn);
	}
    }

    return (retval);
}

/*****************************************************************************/

LONG bumpPSNVisitor (struct PubScreenNode *psn)
{
    if (psn)
    {
	DVC (printf ("bumpPSNV, screen %lx, count %lx\n",
		     psn->psn_Screen, psn->psn_VisitorCount + 1));
	return (++psn->psn_VisitorCount);
    }
    DVC (printf ("bumpPSNVisitor for NULL psn\n"));
    return (0);
}

/*****************************************************************************/

LONG decPSNVisitor (struct PubScreenNode *psn)
{
    if (psn)
    {
	DVC (printf ("decPSNV, screen %lx, count %lx\n",
		     psn->psn_Screen, psn->psn_VisitorCount - 1));
	return (--psn->psn_VisitorCount);
    }
    DVC (printf ("decPSNVisitor for NULL psn\n"));
    return (0);
}

/*****************************************************************************/

/*** intuition.library/LockPubScreen ***/
/*
 * keeps pubscreen around and non-PRIVATE until you get around to
 * opening on it (at which time you call UnlockPubScreen.)
 * returns TRUE if successful.
 */
struct Screen *LockPubScreen (UBYTE *name)
{
    struct PubScreenNode *psn;
    struct Screen *retval = NULL;

    /* done in findPubScreen()	LOCKIBASE(); */

    D (printf ("LPS: name: %s\n", name));

    /* ZZZ: no good: what if dPS fails? */
    if (psn = (name ? findPubScreen (name) : XSC (defaultPubScreen ())->PSNode))
    {
	D (printf ("LPS: psn = %lx\n", psn));
	/* both fPS and dPS return VC++ */

	retval = psn->psn_Screen;

	D (printf ("LPS returns %lx\n", retval));
    }

    /* UNLOCKIBASE(); */
    return (retval);
}

/*****************************************************************************/

/*** intuition.library/UnlockPubScreen ***/
void UnlockPubScreen (UBYTE *name, struct Screen *screen)
{
    struct PubScreenNode *psn = NULL;

    /* Until V40, we used to not do LOCKIBASE().  Jimm left me a
     * comment that VC++ was good enough to keep the screen around.
     * however, as we unlock the screen, we Signal() after decrementing
     * the visitor count.  That's unsafe without a lock.
     */
    LOCKIBASE ();

    if (name)
    {
	psn = findPubScreen (name);	/* returns VC++ !! 	*/
	decPSNVisitor (psn);	/* so, cancel that	*/
    }
    else if (screen)
	psn = XSC (screen)->PSNode;

    /* Peter: 7-Aug-90 - signal pubscreen owner here too */
    if (decPSNVisitor (psn) == 0)	/* checks for NULL	*/
    {
	if (psn && psn->psn_SigTask)	/* does he want a signal	*/
	{
	    Signal (psn->psn_SigTask, 1 << psn->psn_SigBit);
	}
    }
    UNLOCKIBASE ();
}

/*****************************************************************************/

/*** intuition.library/LockPubScreenList ***/
struct List *LockPubScreenList (void)
{
    LOCKIBASE ();
    return (&fetchIBase ()->PubScreenList);
}

/*****************************************************************************/

/*** intuition.library/UnlockPubScreenList ***/
void UnlockPubScreenList (void)
{
    UNLOCKIBASE ();
}

/*****************************************************************************/

/*** intuition.library/NextPubScreen ***/
UBYTE *NextPubScreen (struct Screen *screen, UBYTE *namebuf)
{
    struct IntuitionBase *IBase = fetchIBase ();
    struct PubScreenNode *psn = NULL;
    UBYTE *retval = NULL;

    LOCKIBASE ();

    if (screen)
	psn = XSC (screen)->PSNode;

    /* Not a public screen, perhaps, or last in list */
    if ((!psn) || ((struct Node *) psn == IBase->PubScreenList.lh_TailPred))
	psn = (struct PubScreenNode *) IBase->PubScreenList.lh_Head;
    else
	psn = (struct PubScreenNode *) psn->psn_Node.ln_Succ;

    /* If now still tail, means list was empty */
    if (psn->psn_Node.ln_Succ != NULL)
    {
	jstrncpy (namebuf, psn->psn_Node.ln_Name, MAXPUBSCREENNAME);
	namebuf[MAXPUBSCREENNAME - 1] = '\0';
	retval = namebuf;
    }
    UNLOCKIBASE ();
    return (retval);
}

/*****************************************************************************/

/*** intuition.library/SetDefaultPubScreen ***/
void SetDefaultPubScreen (UBYTE *name)
{
    struct IntuitionBase *IBase = fetchIBase ();
    struct PubScreenNode *psn;

    /* LOCKIBASE(); VC++ is good enough */
    if (name)
    {
	/* returns VC++ */
	if (psn = findPubScreen (name))
	{
	    IBase->DefaultPubScreen = psn;

	    /* cancel the VC++ */
	    decPSNVisitor (psn);
	}
    }
    else
	IBase->DefaultPubScreen = NULL;
    /* UNLOCKIBASE(); */
}

/*****************************************************************************/

/*** intuition.library/GetDefaultPubScreen ***/
struct Screen *GetDefaultPubScreen (UBYTE *Namebuff)
{
    struct PubScreenNode *psn;
    STRPTR pubname;

    /* Get the name */
    pubname = (psn = fetchIBase ()->DefaultPubScreen) ?	psn->psn_Node.ln_Name : WBPUBNAME;

    /* If they provide a name buffer, then copy the name into it */
    if (Namebuff)
	jstrncpy (Namebuff, pubname, MAXPUBSCREENNAME);

    /* "undocumented feature" return the screen pointer
     * in D0.  This is undocumented because:
     * - you can't rely on the persistence of this
     *   pointer.  The only safe use I can think of
     *   is to compare it for equality with a screen pointer
     *   that you know is persistent (window open, lockpubscreen).
     * - this idea missed the freezes, but Bill Hawes really needs it. */
    return (struct Screen *)(psn ? psn->psn_Screen : NULL);
}

/*****************************************************************************/

/*** intuition.library/SetPubScreenModes ***/
UWORD SetPubScreenModes (UWORD modes)
{
    UWORD oldflags;
    struct IntuitionBase *IBase = fetchIBase ();

    /* no lock needed */
    oldflags = IBase->PubScreenFlags;
    IBase->PubScreenFlags = modes;
    return (oldflags);
}

/*****************************************************************************/

/*** intuition.library/PubScreenStatus ***/
UWORD PubScreenStatus (struct Screen *screen, UWORD status)
{
    struct PubScreenNode *psn;
    struct IntuitionBase *IBase = fetchIBase ();
    UWORD retval = 0;

    LOCKIBASE ();
    if (psn = XSC (screen)->PSNode)
    {
	if (TESTFLAG (status, PSNF_PRIVATE))
	{
	    if (psn->psn_VisitorCount == 0)
	    {
		/* Make him private */
		SETFLAG (psn->psn_Flags, PSNF_PRIVATE);

		/* Make sure he isn't default */
		if (IBase->DefaultPubScreen == psn)
		    IBase->DefaultPubScreen = NULL;
		retval = 1;
	    }
	}
	else
	{
	    CLEARFLAG (psn->psn_Flags, PSNF_PRIVATE);
	    retval = 1;
	}
    }
    UNLOCKIBASE ();
    return (retval);
}
