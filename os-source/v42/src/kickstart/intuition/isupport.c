/*** isupport.c **************************************************************
 *
 * intuition.c support routines
 *
 *  $Id: isupport.c,v 40.0 94/02/15 17:35:48 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "intuall.h"

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef GRAPHICS_GFXMACROS_H
#include <graphics/gfxmacros.h>
#endif

#ifndef HARDWARE_CUSTOM_H
#include <hardware/custom.h>
#endif

#ifndef HARDWARE_DMABITS_H
#include <hardware/dmabits.h>
#endif

#ifndef INTERNAL_LIBRARYTAGS_H
#include <internal/librarytags.h>
#endif

/*****************************************************************************/

#define D(x)	;
#define DZIP(x)	;
#define DREQ(x)	;		/* requester refresh in BP() */
#define DBP(x)	;		/* BorderPatrol() */
#define DBPIN(x)	;	/* BorderPatrol() entry */
#define DREFRESH(x)	;	/* layer damage debug */
#define DBER(x)	;		/* begin/endrefresh debug */

/*****************************************************************************/

extern struct Custom volatile custom;

/*****************************************************************************/

#ifndef DEVICES_INPUT_H
#include <devices/input.h>
#endif

/*****************************************************************************/

extern UBYTE IntuiName[];

/*****************************************************************************/

/* lock IBASELOCK before this */
BOOL knownScreen (struct Screen *s)
{
    struct Screen *reals;
    struct IntuitionBase *IBase = fetchIBase ();

    assertLock ("knownscreen", IBASELOCK);

    reals = IBase->FirstScreen;
    while (reals)
    {
	if (reals == s)
	    return (TRUE);
	else
	    reals = reals->NextScreen;
    }
    return (FALSE);
}

/*****************************************************************************/

/* this routine requires IBASELOCK, but you should probably
 * call it before this, so you know that its answer remains
 * valid */
BOOL knownWindow (struct Window *w)
{
    struct Screen *reals;
    struct Window *realw;
    BOOL retval = FALSE;

    assertLock ("knownwindow", IBASELOCK);

    if (w && (knownScreen (reals = w->WScreen)))
    {
	for (realw = reals->FirstWindow; realw; realw = realw->NextWindow)
	{
	    if (realw == w)
	    {
		retval = TRUE;
		break;
	    }
	}
    }

    return (retval);
}

/*****************************************************************************/

/* BorderPatrol() calls here with stack-swapped.  Here's why we need
 * to do this:
 *
 * BorderPatrol() can get invoked by any innocent bystander.  Say your
 * application closes its window.  BorderPatrol() is invoked on that
 * application's task to repair damage underneath the window.  Say the
 * damaged area includes some complex boopsi object.  Now this simple
 * application has to supply stack for the boopsi refresh code.
 *
 * So let's be safe and ensure the stack we need.
 */

/* FOR FUTURE OPTIMIZATION:  refresh the Gadgets of GIMMEZEROZERO and internal
 * layers only as needed.  Don't refresh GIMMEZEROZERO Gadgets just because
 * internal layer needs refreshing
 */

void internalBorderPatrol (struct Screen *s)
{
    struct Window *w;
    struct Requester *r;
    struct Layer *l;		/* reused */
    struct Layer *gzzlayer;	/* border layer */

    BOOL do_my_refresh;		/* is there something to do?	*/
    BOOL send_him_refresh;	/* does he get a refresh event?	*/
    BOOL i_end_refresh;		/* do I dispose damage list?	*/

    DBPIN (printf ("bp "));
    DBP (printf ("\nBorderPatrol: screen %lx\n", s));

    if (NOT knownScreen (s))
	return;

    assertLock ("BorderPatrol", IBASELOCK);

    LockLayerInfo (&s->LayerInfo);
    LOCKGADGETS ();		/* before locklayerrom in begin refresh */

    w = s->FirstWindow;

    while (w)
    {
	DREQ (printf ("BP window %lx\n", w));
	i_end_refresh = FALSE;
	send_him_refresh = do_my_refresh = FALSE;
	gzzlayer = NULL;
	l = w->WLayer;

	/* Peter 29-Apr-92:
	 * Layers now sets an extra flag bit (LAYERI_NOTIFYREFRESH) when
	 * it damages stuff.  I clear this bit when I send the
	 * IDCMP_REFRESHWINDOW event, so I can use this bit
	 * to mean "I need to send another refresh event".
	 *
	 * This optimization is useful since BorderPatrol() operates
	 * on a whole screen, and any window that has not fixed
	 * and cleared its damage gets a fresh refresh event
	 * whenever any layer activity occurs.
	 */
	if ((l->Flags & (LAYERREFRESH | LAYERI_NOTIFYREFRESH)) ==
	    (LAYERREFRESH | LAYERI_NOTIFYREFRESH))
	{
	    DREQ (printf ("window needs refreshing %lx\n", w));
	    DZIP (printf ("window needs refreshing %lx\n", w));
	    do_my_refresh = TRUE;

	    if (w->Flags & NOCAREREFRESH)
	    {
		i_end_refresh = TRUE;
	    }
	    else
	    {
		send_him_refresh = TRUE;
	    }
	}

	/* Refresh Window border layer */
	if (w->BorderRPort)
	{
	    DREQ (printf ("gzz layer needs refreshing\n"));
	    gzzlayer = w->BorderRPort->Layer;
	    if (gzzlayer->Flags & LAYERREFRESH)
	    {
		do_my_refresh = TRUE;
		gzzlayer->Flags &= ~LAYERREFRESH;
	    }
	}

	/* LAYERREFRESH was set */
	if (do_my_refresh)	/* refresh border and gadgets */
	{
	    DREFRESH (printf ("BP: refresh window\n"));
	    BeginRefresh (w);	/* get gzzlayer lock with this */

	    DREFRESH (debugRefresh (w->RPort, w->Width, w->Height));

	    if (w->Flags & GIMMEZEROZERO)
	    {
		BeginUpdate (gzzlayer);
		/* assuming that G00 flag means gzzlayer != NULL */
	    }

	    /* Intuition's refresh activity:
	     * re-render the window frame and ALL gadgets
	     */
	    drawEmbossedBorder (w, DRAWGADGETS_ALL);

	    if (gzzlayer)	/* finish up border layer */
	    {
		EndUpdate (gzzlayer, TRUE);
	    }

	    DREFRESH (printf ("bp endr\n"));
	    EndRefresh (w, i_end_refresh);
	}

	/* refresh requesters */
	for (r = w->FirstRequest; r; r = r->OlderRequest)
	{
	    DREQ (printf ("check requester: %lx\n", r));
	    /* reuse l */
	    if ((l = r->ReqLayer) == NULL)
	    {
		/* clipped offscreen */
		DREQ (printf ("clipped offscreen\n"));
		continue;
	    }

	    if (l->Flags & LAYERREFRESH)
	    {
		DREQ (printf ("needs refreshing\n"));
		DREFRESH (printf ("BP: refresh requester\n"));
		BeginUpdate (l);
		DREFRESH (debugRefresh (r->ReqLayer->rp, r->Width, r->Height));
		drawRequester (r, w);
		l->Flags &= ~LAYERREFRESH;	/* only I reset this */
		EndUpdate (l, TRUE);
	    }
	}

	/* send him his message, now that I am done with refreshing */
	if (send_him_refresh)
	{
	    DBP (printf ("sending refresh event to %lx\n", w));
	    windowEvent (w, IECLASS_REFRESHWINDOW, 0);

	    /* and tell Mr. Console	*/
	    windowEvent (w, IECLASS_EVENT, IECODE_REFRESH);
	}
	/* This new bit means that more damage happened since
	 * the last time the application was notified.  So now
	 * that we've just notified the application (if it cares,
	 * or discarded the event if it doesn't), it's time
	 * to clear the flag.
	 */
	w->WLayer->Flags &= ~LAYERI_NOTIFYREFRESH;

	w = w->NextWindow;
    }

    refreshScreenBar (s);

    UNLOCKGADGETS ();
    UnlockLayerInfo (&s->LayerInfo);
}

/*****************************************************************************/

/*** intuition.library/BeginRefresh ***/
void BeginRefresh (struct Window *window)
{
    LockLayerInfo (&window->WScreen->LayerInfo);
    window->Flags |= WINDOWREFRESH;
    if (window->BorderRPort)
    {
	LockLayerRom (window->BorderRPort->Layer);
    }
    /** jimm: 11/18/85 layer change **/
    if (BeginUpdate (window->WLayer))
    {
	SETFLAG (window->MoreFlags, WMF_BEGINUPDATE);
    }
    UnlockLayerInfo (&window->WScreen->LayerInfo);
    DBER (printf ("begin refresh window %lx\n", window));
    /* DBER( Debug() ); */
}

/*****************************************************************************/

/*** intuition.library/EndRefresh ***/
void EndRefresh (struct Window *window, LONG complete)
{
    /** ZZZ: shouldn't this be before EndUpdate (while still locked)? **/
    if (complete)
    {
	window->WLayer->Flags &= ~LAYERREFRESH;
    }
    /** jimm: 11/18/85 layer change **/
    EndUpdate (window->WLayer, ((complete) && TESTFLAG (window->MoreFlags, WMF_BEGINUPDATE)));
    if (window->BorderRPort)
	UnlockLayerRom (window->BorderRPort->Layer);
    if (complete)
    {
	CLEARFLAG (window->MoreFlags, WMF_BEGINUPDATE);
    }
    window->Flags &= ~WINDOWREFRESH;

    /*DBER( printf("end refresh window %lx\n", window));*/
}

/*****************************************************************************/

/* thinks about adding MenuIBase->Selected to the optionlist.
 *
 * Checks for valid selection:
 *	menunull is a no-op
 *	require an item, not just a top-level menu
 *	menu/item/subitem path completely enabled
 *	can't select a menu item with subitems, must select subitem
 *
 * Does all the CHECKIT stuff and mututal exclusion
 *
 * unless 'redraw' is false (command key selection), will update
 * visuals if needed.
 *
 * Called:
 *	menu-state- menu up
 *	menu-state- select down
 *	during dragselect
 *	commandPerformance (after MenuSelected manually set up)
 */

void updateOptions (UWORD *firstoption, BOOL draw)
{
    USHORT num;
    struct MenuItem *pickptr, *firstpick;
    struct Menu *strip, *menu;
    struct Window *AWindow;
    BOOL CrudeRedraw;
    ULONG mutual;
    LONG i, max;
    struct IntuitionBase *IBase = fetchIBase ();

    AWindow = IBase->ActiveWindow;
    CrudeRedraw = FALSE;

    /* if we haven't selected both a valid Menu header and a valid
     * MenuItem then return empty-handed
     */
    if ((USHORT) (num = IBase->MenuSelected) == MENUNULL)
	return;
    if (ITEMNUM (num) == NOITEM)
	return;

    strip = AWindow->MenuStrip;

    menu = grabMenu (strip, num);
    /* if this whole menu is disabled, don't add anything */
    if ((menu->Flags & MENUENABLED) == NULL)
	return;

    firstpick = grabItem (menu, num);
    /* if this item is disabled, don't add anything */
    if ((firstpick->Flags & ITEMENABLED) == NULL)
	return;

    /* test if subnum is not a sub while there IS a sublist available */
    if ((SUBNUM (num) == NOSUB))
    {
	if (firstpick->SubItem)
	    return;
    }
    else
    {
	/* SUBNUM is for a real sublist selection */
	firstpick = grabSub (firstpick, num);	/* advance our pointer */
	/* if this item is disabled, don't add anything */
	if ((firstpick->Flags & ITEMENABLED) == NULL)
	    return;
    }

    /* set CHECKED.
     * if MENUTOGGLE, may reset CHECKED.
     */
    if (firstpick->Flags & CHECKIT)
    {
	/* has changed state already this "session"
	 * won't operate until resetMenu() called
	 */
	if (firstpick->Flags & MENUTOGGLED)
	{
	    return;
	}

	if (firstpick->Flags & CHECKED)
	{
	    /* This item is already selected.
	     * Is it a menu toggle item?
	     */
	    if (firstpick->Flags & MENUTOGGLE)
	    {
		/* want to toggle it off */
		firstpick->Flags &= ~CHECKED;
	    }

	    /* otherwise: checked, but not toggle.
	     * for compatibility, user needs to hear selection
	     */
	}
	else
	    /* was off: turn on, menutoggle or not */
	{
	    /* turn it on */
	    firstpick->Flags |= CHECKED;
	}

	/* won't get here unless CHECKIT really was toggled */
	firstpick->Flags |= MENUTOGGLED;	/* no more changes until resetMenu() */
	CrudeRedraw = TRUE;
    }

    /*****************************************************************/
    /* add selected menu "option" to NextSelect chain 		     */
    /* RJ code, with FIX_OPTION_DELINK and comments by jimm	     */
    /*	(let's make that clear ...)				     */
    /*****************************************************************/

#define FIX_OPTION_DELINK	1
/* jimm: 2/12/90: fix NextSelect chain */

/* don't kiss off the items hanging off of this item yet! */
#if ! FIX_OPTION_DELINK
    /* since this one will end up at the end of the list,
     * initialize the NextSelect parameter to NULL
     */
    firstpick->NextSelect = MENUNULL;
#endif

    /* if the option list is currently NULL, add it at the beginning */
    if ((USHORT) (*firstoption) == MENUNULL)
    {
	*firstoption = num;
	firstpick->NextSelect = MENUNULL;	/* and terminate */
    }

    /* there already exists something in the NextSelect chain	*/
    else
    {
	/* get the address of the first option in the list */
	pickptr = ItemAddress (strip, *firstoption);

	/* the options list is currently not null, so test if this one is
	 * currently in the list
	 */
	if (*firstoption == num)/* it's at the head of the list */
	{

	    /* if it's the only one in the list, return, else extract it */
	    if (pickptr->NextSelect == MENUNULL)
		goto EXCLUDETEST;

	    /* 'extracting' now	*/
	    *firstoption = pickptr->NextSelect;
	}

	/* advance pickptr to point to last in the list */
	/* check if this one is already in the list, extract it if so */
	do
	{
	    /* if this is already in the list, remove it */
	    /* FIX_OPTION_DELINK: here's where we need to have
	     * the original link intact in firstpick
	     */
	    if (pickptr->NextSelect == num)
		pickptr->NextSelect = firstpick->NextSelect;
	    if (pickptr->NextSelect != MENUNULL)
		pickptr = ItemAddress (strip, pickptr->NextSelect);
	}
	while (pickptr->NextSelect != MENUNULL);

	/* pickptr points to the last in the list.  link it to the new one */
	pickptr->NextSelect = num;

#if FIX_OPTION_DELINK
	/* since this one is NOW at the end of the list,
	 * initialize the NextSelect parameter to NULL
	 */
	firstpick->NextSelect = MENUNULL;
#endif
    }

  EXCLUDETEST:
    /* now, the mutual-exclude stuff */
    /*??? this is the crude way for now:  if there's any change, set the
     *??? redraw flag so that the entire item/subitem display will be
     *??? redrawn as needed.  someday, it will be better to just check
     *??? and uncheck and ghost those items that need it!
     */
    mutual = firstpick->MutualExclude;

    /* if this selected has no subitem component, get the address of the
     * first item, else get the address of the first subitem
     */
    if (SUBNUM (num) == NOSUB)
	pickptr = ItemAddress (strip,
			       SHIFTMENU (MENUNUM (num))
			       | SHIFTITEM (0)
			       | SHIFTSUB (NOSUB));
    else
	pickptr = ItemAddress (strip,
			       SHIFTMENU (MENUNUM (num))
			       | SHIFTITEM (ITEMNUM (num))
			       | SHIFTSUB (0));

    /* get the max number of items on this plane */
    firstpick = pickptr;
    max = 0;
    while (pickptr)
    {
	max++;
	pickptr = pickptr->NextItem;
    }

    for (i = 0; i < max; i++)
    {
	if (mutual & 0x1)	/* exclude this one? */
	{
	    /* yes, exclude this one */
	    if (firstpick->Flags & CHECKIT)
		if (firstpick->Flags & CHECKED)
		{
		    /* this one is checkable and currently checked
		 * uncheck it
		 * ??? someday, redraw just this item, and ghost it if
		 * ??? it's not SELECTed
		 */
		    firstpick->Flags &= ~CHECKED;
		    CrudeRedraw = TRUE;	/* now we'll redraw (sigh) */
		}
	}
	/* to allow for planes that have more than 32 items, make sure
	 * that the top mutual bit is always cleared (so after 32,
	 * no further excludes)
	 */
	mutual = (mutual >> 1) & 0x7FFFFFFF;
	firstpick = firstpick->NextItem;
    }

  CRUDETEST:
#if 1
    if (draw && CrudeRedraw)
/*???    if (CrudeRedraw) */
/*???	if ((IBase->MenuDrawn & ~NOSUB) == (IBase->MenuSelected & ~NOSUB))*/
    {
	/* ok, erase and redraw menu (for now!) */
	if (SUBNUM (num) == NOSUB)
	{
	    eraseItem ();
	    drawItem (num);
	    highItem ();
	}
	else
	{
	    eraseSub ();
	    drawSub (num);
	    highSub ();
	}
    }
#else
    if (draw && CrudeRedraw)
    {
	/* ok, erase and redraw menu (for now!) */
	if (SUBNUM (num) == NOSUB)
	{
	    if (IBase->Flags & GOODITEMDRAWN)
	    {
		highItem ();
		checkmarkMenu (menu, menu->FirstItem, FALSE);
		highItem ();
	    }
	}
	else
	{
	    if (IBase->Flags & GOODSUBDRAWN)
	    {
		highSub ();
		checkmarkMenu (menu, grabItem (menu, num), TRUE);
		highSub ();
	    }
	}
    }
#endif
}

/*****************************************************************************/

UBYTE InputDeviceName[] = "input.device";

/*****************************************************************************/

struct IntuitionBase *OpenIntuition (void)
{
    ULONG mon;
    struct IntuitionBase *IBase = fetchIBase ();

    if (IBase->LibNode.lib_OpenCnt)
	return (IBase);
    IBase->LibNode.lib_OpenCnt++;

    /* jimm: 5/30/90: set up mode aliasing in gfxbase	*/
    mon = NTSC_MONITOR_ID >> 16;
    if (IBase->GfxBase->DisplayFlags & PAL)
    {
	IBase->Flags |= MACHINE_ISPAL;
	mon = PAL_MONITOR_ID >> 16;
	IBase->MouseScaleY = PAL_MOUSESCALEY;
    }
    SetDefaultMonitor (mon);

    /* Under V37, setPrefs was called inside initIntuition().  We
     * moved it here after the determination of the default monitor
     * for this machine.
     *
     * One subtle twist on all this is that OpenIntuition() used
     * to init IBase->Flags to be equal to VIRGINDISPLAY.  This
     * actually cleared the SEENSETPREFS flag that had been set by
     * the earlier call to setPrefs(), allowing dos's SetPrefs()
     * of devs:system-configuration to be heeded.  So here we
     * have to clear the flag explicitly by hand...
     *
     * setprefs sets up TextOverscan from WB Normal, ...
     */
    D (printf ("OpenIntuition calling SetPrefs\n"));
    /* We don't want our own call to SetPrefs() to affect the pointer */
    SETFLAG (IBase->Flags, SEENIPOINTER);
    setPrefs (IBase->Preferences, sizeof (struct Preferences), FALSE);

    CLEARFLAG (IBase->Flags, SEENSETPREFS | SEENIPOINTER);

    OFF_SPRITE;
    OFF_DISPLAY;

    IBase->Flags |= VIRGINDISPLAY;
    /** jimm: 3/24/86: never used * IBase->Selected = NULL; **/
    IBase->MenuDrawn = MENUNULL;

    /* the IOReq block is temporary and thrown away when exiting */
    IBase->KeymapBase = (APTR) TaggedOpenLibrary (OLTAG_KEYMAP);
    if (!IBase->KeymapBase)
    {
	D (printf ("NO KEYMAP LIBRARY!!!! INTUITION GOING SOUTH!!\n"));
	D (Alert (AN_NoConsole));
    }

    IBase->CurrentState = sNoWindow;

/* install Intuition() and IntuiLeap() into the input stream path.
 * after this installation, the Input Stream Merger will call IntuiLeap()
 * with two pointers, the first of which is a pointer to an IntuiMessage,
 * the second of which is currently unused by Intuition().  IntuiLeap()
 * then configures the data (stuffs it on the stack) in a way that
 * Intuition, a C procedure, can recognize
 */
    if (OpenDevice (InputDeviceName, 0, &IBase->InputRequest, 0) == 0)
    {
	/* successful open of the Input Device */

	/* that the ReplyPort is NULL means that Intuition can't close, which
	 * it currently doesn't do anyway
	 */
	IBase->InputRequest.io_Message.mn_ReplyPort = NULL;
	IBase->InputRequest.io_Command = IND_ADDHANDLER;
	IBase->InputRequest.io_Data = &IBase->InputInterrupt;

	IBase->InputInterrupt.is_Node.ln_Pri = 50;	/* serious priority */
	IBase->InputInterrupt.is_Node.ln_Name = IntuiName;
	IBase->InputInterrupt.is_Code = (void (*)) IntuiLeap;	/* IntuiLeap gets events */
	IBase->InputInterrupt.is_Data = (APTR) IBase;

	SendIO (&IBase->InputRequest);

	/* stash this guy for later use	*/
	IBase->InputDeviceTask = (struct Task *) FindTask (InputDeviceName);
    }

    return (IBase);
}
