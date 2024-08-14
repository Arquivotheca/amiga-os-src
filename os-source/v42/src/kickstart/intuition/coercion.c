/*** coercion.c ***************************************************************
 *
 *  coercion.c -- new chips modes, mixed modes, gfx database
 *
 *  $Id: coercion.c,v 40.0 94/02/15 17:47:34 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) 1989, Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "intuall.h"
#include "preferences.h"

/*****************************************************************************/

#ifndef GRAPHICS_DISPLAY_H
#include <graphics/display.h>
#endif

#ifndef GRAPHICS_COERCE_H
#include <graphics/coerce.h>
#endif

#ifndef GRAPHICS_DISPLAYINFO_H
#include <graphics/displayinfo.h>
#endif

/*****************************************************************************/

#define DX(x)		;	/* Exclusive screens */
#define D(x) 		;
#define DC(x) 		;	/* coercion */
#define DID(x)		;
#define DSDID(x)	;
#define DSNOOP(x)	;
#define DPANIC(x)	;

/*****************************************************************************/

/*
 * sets up everything implied by the screen mode.
 * assumes these are correct in the screen:
 *	NaturalDRecord
 *	NaturalDisplayID
 *	ColorMap
 *	DClip		(in natural native pixel coord's)
 *	position	(but will be validated here)
 */
static void coerceScreenGrunt (struct Screen *s, ULONG frontmonitor)
{
    struct IntuitionBase *IBase = fetchIBase ();

    /* buffer shared by several GetDisplayInfoData calls */
    /* be careful out there ... */
    union
    {
	struct MonitorInfo minfo;
	struct DisplayInfo dinfo;
    } di;
    ULONG coercion_flags;
    ULONG coerced_mode;

    /************************************************************
     * coerce: set up current DRecord (for scale factors
     * and mouse transformations) and setup current monitor spec.
     ************************************************************/

    /* We take advantage of the fact that the two coercion control
     * flags from IControl match (by design) the same properties
     * in the graphics CoerceMode() function.
     *
     * We don't pass the PROMOTE flag, because that's handled through
     * a different mechanism, and really only applies to the front screen.
     */

    coercion_flags = (IBase->NewIControl & (AVOID_FLICKER | PRESERVE_COLORS));

#ifndef IGNORE_MCOMPAT
#define IGNORE_MCOMPAT 4
#endif

    /* Ask graphics what to coerce this mode to.  Ordinarily,
     * CoerceMode() will handle exclusion based on MCOMPAT_xxx, returning
     * INVALID_ID for incompatible modes.  For the front screen, we
     * have to ask graphics to ignore compatibility relationships,
     * since it'll always be visible, so we pass the IGNORE_MCOMPAT flag.
     * You might think that calling CoerceMode() on the front screen
     * is a do-nothing operation (coercing to match its own monitor),
     * but in fact it detects when the BitMap is not properly aligned,
     * and in that case returns a lower resolution of that monitor.
     */
    if (s == IBase->FirstScreen)
	coercion_flags |= IGNORE_MCOMPAT;
    coerced_mode = CoerceMode (&s->ViewPort, frontmonitor, coercion_flags);

    /* If the first screen or this screen is SA_Exclusive,
     * then this screen needs to be excluded, unless they're
     * of the same family (in which case it's an exclusive family).
     * NB: We can't be here if IBase->FirstScreen is NULL,
     * so not to worry! */
    if ((TESTFLAG (XSC (IBase->FirstScreen)->PrivateFlags, PSF_EXCLUSIVE) ||
	 TESTFLAG (XSC (s)->PrivateFlags, PSF_EXCLUSIVE)) &&
	(screenFamily (IBase->FirstScreen) != screenFamily (s)))
    {
	DX (printf ("Screen %lx excluded due to SA_Exclusive\n", s));
	coerced_mode = INVALID_ID;
    }

    D (printf ("natural drecord %lx, coerced ID: %lx\n", XSC (s)->NaturalDRecord, coerced_mode));

    /* Look, we're compatible with the front screen! */
    if (coerced_mode != INVALID_ID)
    {
	DisplayInfoHandle coerced_drecord;

	/************************************************************
	 * poof: you appear
	 ************************************************************/
	D (printf ("do not hide screen: %lx\n", s));
	CLEARFLAG (s->Flags, SCREENHIDDEN);

	coerced_drecord = FindDisplayInfo (coerced_mode);

	/* tell graphics about the coerced display mode	*/
	D (printf ("coerceScreens: do VCT ... "));
	VideoControlTags (XSC (s)->ColorMap,
			  VTAG_COERCE_DISP_SET, coerced_drecord,
			  TAG_DONE);
	D (printf ("coerceScreens: done VCT\n"));

	/************************************************************
	 * get scale resolution, other stuff from DisplayInfo
	 ************************************************************/
	if (GetDisplayInfoData (coerced_drecord, (UBYTE *) &di.dinfo, sizeof (di.dinfo), DTAG_DISP, NULL))
	{
	    XSC (s)->ScaleFactor.X = di.dinfo.Resolution.x;
	    XSC (s)->ScaleFactor.Y = di.dinfo.Resolution.y;
	    XSC (s)->PixelSpeed = di.dinfo.PixelSpeed;
	    XSC (s)->SpriteRes.X = di.dinfo.SpriteResolution.x;
	    XSC (s)->SpriteRes.Y = di.dinfo.SpriteResolution.y;

	    XSC (s)->DProperties = di.dinfo.PropertyFlags;

	    XSC (s)->SpriteFactor.X = (di.dinfo.SpriteResolution.x * SPRITEMULT_X) /
		XSC (s)->ScaleFactor.X;
	    XSC (s)->SpriteFactor.Y = (di.dinfo.SpriteResolution.y * SPRITEMULT_Y) /
		XSC (s)->ScaleFactor.Y;
	}
	DPANIC (
		   else
		   printf ("PANIC:  can't get displayinfo in sDID\n"));
	/* ZZZ: else panic */

	/* Start with the natural DClip.  Scale it based on the
	 * initial resolution and the coerced resolution.
	 */
	XSC (s)->VPExtra->DisplayClip = XSC (s)->DClip;
	scaleConversion ((struct Point *)&XSC (s)->VPExtra->DisplayClip.MinX,
			 XSC (s)->InitialScaleFactor, XSC (s)->ScaleFactor);
	scaleConversion ((struct Point *)&XSC (s)->VPExtra->DisplayClip.MaxX,
			 XSC (s)->InitialScaleFactor, XSC (s)->ScaleFactor);

	D (dumpRect ("new DClip", &XSC (s)->VPExtra->DisplayClip));
    }
    else
    {
	/************************************************************
	 * this screen is hereby invisible by decree
	 ************************************************************/
	D (printf ("SCREEN HIDDEN: %lx\n", s));
	SETFLAG (s->Flags, SCREENHIDDEN);
    }
}

/*****************************************************************************/

/* sets up everything that needs to be changed when the
 * active monitor switches, including scale factors,
 * coercion/exclusion, view position, ...
 * 'force' is as follows:
 * MEGAFORCE is expected if the monitor changed
 * FORCE is expected if someone upstream asked for FORCE (that would be
 *	if we came from RemakeDisplay() or initIntuition())
 * NOFORCE is expected if just the INTERLACE state changed. */
void setupMonitor (ULONG force)
{
    struct IntuitionBase *IBase = fetchIBase ();

    /* WARNING: ActiveMonitorSpec was setup in monitorCheck(). Copy it to ViewLord now */
    IBase->ViewExtra->Monitor = IBase->ActiveMonitorSpec;
    SETFLAG (IBase->ViewLord.Modes, EXTEND_VSTRUCT);

    /* setup all screens for new monitor */
    DSNOOP (printf ("setupMonitor coercing all\n"));
    coerceScreens (NULL, force);
}

/*****************************************************************************/

/* establishes coerced screen mode information, and performs
 * scale distortions for mixed-mode screens.
 *
 * if 'onescreen' is non-NULL, then setup the mode
 * information for just this screen, else all.
 *
 * 'force' is as follows:
 * Reset view-poker tracking if MEGAFORCE
 * Rethink coercion if FORCE or MEGAFORCE */
void coerceScreens (struct Screen *onescreen, ULONG force)
{
    struct IntuitionBase *IBase = fetchIBase ();
    struct MonitorInfo minfo;
    struct Screen *screen;

    /* gotten from monitor for frontmost screen */
    ULONG frontid = 0;

    D (printf ("\ncoerceScreens 'onescreen' %lx\n", onescreen));

    /**************************************************************
     * Get monitor information on frontmost screen (if none: lores)
     **************************************************************/
    D (printf ("coerceScreens: get frontmost info\n"));
    if (GetDisplayInfoData ((DisplayInfoHandle)(IBase->FirstScreen ? XSC (IBase->FirstScreen)->NaturalDRecord : 0),
	      (UBYTE *) &minfo, sizeof (minfo), DTAG_MNTR, NULL))
    {
	/* information that each screen will need to know */
	frontid = MONITOR_PART (minfo.Header.DisplayID);

	/* reset view only if we had a monitor change */
	if (force > FORCE)
	{
	    /* Reset this upon coercion, to force tablet guys
	     * to re-inquire about screen parameters: */
	    IBase->LastTabletScreen = NULL;

	    /* set view position for active monitor */
	    IBase->TrackViewDx = IBase->ViewLord.DxOffset =
		minfo.ViewPosition.x;
	    IBase->TrackViewDy = IBase->ViewLord.DyOffset =
		minfo.ViewPosition.y;

	    /* MouseScaleX and MouseScaleY are the ticks to move per
	     * IECLASS_RAWMOUSE unit, subject to acceleration and
	     * speed-scaling.  The idea is that these numbers are
	     * almost constants, but slightly tuned to be an exact
	     * multiple of pixel dimensions of that monitor's modes.
	     * Starting with the 3.01 monitors, these numbers are
	     * available from the graphics database, but prior to
	     * that, we have a kludgy little thing in Intuition
	     * to guess the mouse scales.
	     */

#if 0
	    IBase->MouseScaleX = minfo.MouseTicks.x;
	    IBase->MouseScaleY = minfo.MouseTicks.y;
#else
	    IBase->MouseScaleX = *((WORD *) & minfo.pad[32]);
	    IBase->MouseScaleY = *((WORD *) & minfo.pad[34]);
#endif

	    if (!IBase->MouseScaleY)
	    {
		IBase->MouseScaleX = MOUSESCALEX;
		/* MouseScaleY is variable, depending on the active monitor.
		 * We note that PAL_MOUSESCALEY=VGA_MOUSESCALEY<NTSC_MOUSESCALEY,
		 * and that for unknown modes, smaller is better (fewer or no
		 * skipped pixels that way).  So let's default to this number.
		 */
		IBase->MouseScaleY = PAL_MOUSESCALEY;

		switch (frontid)
		{
		    case NTSC_MONITOR_ID:
		    case DBLNTSC_MONITOR_ID:
			/* Use NTSC mouse scale factor */
			IBase->MouseScaleY = NTSC_MOUSESCALEY;
			break;

		    case DEFAULT_MONITOR_ID:
		    case A2024_MONITOR_ID:
			/* For NTSC machines, use NTSC, else use PAL. */
			if (!TESTFLAG (IBase->Flags, MACHINE_ISPAL))
			{
			    IBase->MouseScaleY = NTSC_MOUSESCALEY;
			}
			break;
#if 0
		    case PAL_MONITOR_ID:
		    case EURO36_MONITOR_ID:
		    case VGA_MONITOR_ID:
		    case EURO72_MONITOR_ID:
		    case DBLPAL_MONITOR_ID:
		    case SUPER72_MONITOR_ID:	/* Actual scale should be 17x20, but oh well */
			/* We already have the right value, PAL_MOUSESCALEY,
			 * or equivalently, VGA_MOUSESCALEY  */
			break;
#endif
		}
	    }

	    IBase->EffectiveScaleFactor.X = IBase->MouseScaleX /
		IBase->Preferences->PointerTicks;
	    IBase->EffectiveScaleFactor.Y = IBase->MouseScaleY /
		IBase->Preferences->PointerTicks;
	}
	else
	{
	    /*  Get the new view info into the database and IBase->TrackView */
	    trackViewPos ();
	}
    }

    /************************************************************
     * set up each screen's mode-specific information
     ************************************************************/

    /* Peter 15-Nov-91:  New: NOFORCE means don't re-figure coercion.
     * Most old callers want to use FORCE instead of NOFORCE now.
     * This is to avoid re-figuring coercion when screen dragging
     * changes the INTERLACE state of the ViewLord. */
    if (force != NOFORCE)
    {
	struct Screen *firstscreen = onescreen;
	LONG loop = FALSE;

	if (!firstscreen)
	{
	    firstscreen = IBase->FirstScreen;
	    loop = TRUE;
	}

	screen = firstscreen;
	while (screen)
	{
	    D (printf ("coerceScreens, screen: %lx\n", screen));
	    coerceScreenGrunt (screen, frontid);
	    screen = screen->NextScreen;
	    if (!loop)
		screen = NULL;
	}

	screen = firstscreen;
	while (screen)
	{
	    struct Rectangle legal_positions;

	    D (printf ("coerceScreens, legalizing screen: %lx\n", screen));
	    if (!TESTFLAG (screen->Flags, SCREENHIDDEN))
	    {
		/************************************************************
		 * validate screen position for this monitor
		 ************************************************************/
		D (dumpPt ("original sLP", *((struct Point *) &screen->LeftEdge)));
		screenLegalPosition (screen, &legal_positions, 0);
		limitPoint ((struct Point *) &screen->LeftEdge, &legal_positions);
		/* Ordinarily, a coercion is followed by a RethinkDisplay()
		 * in which the active monitor changes.  This causes a MEGAFORCE
		 * level processing, which causes the VP Dx/yOffsets to be
		 * derived from the screen Left/Top.  When not MEGAFORCE,
		 * the screen Left/Top are actually slaved to changes to the
		 * vp Dx/DyOffsets, which is needed to support some overscan
		 * techniques of old.  The problem here is that the user
		 * can change coercion preferences and cause a screen to be
		 * moved into screenLegalPosition(), without a MEGAFORCE
		 * remake happening.  But we need the VP coordinates to
		 * slave to the screen coordinates in that case, so we set
		 * the PSF_NEWSCREENOFFSETS flag to demand that.
		 */
		SETFLAG (XSC (screen)->PrivateFlags, PSF_NEWSCREENOFFSETS);
		D (dumpRect ("legal positions", &legal_positions));
		D (dumpPt ("new sLP", *((struct Point *) &screen->LeftEdge)));
	    }
	    screen = screen->NextScreen;
	    if (!loop)
		screen = NULL;
	}
    }

    D (printf ("cS: done: do updateMousePointer\n"));
    updateMousePointer (NULL);	/* jimm: 2/22/89: pointer jumping on monitor switch */
}

/*****************************************************************************/

/* Set NaturalDisplayID to INVALID_ID before calling for initial
 * screen setup, or I might close your monitor.
 *
 * after using this and setting up DClip, call coerceScreens()
 *
 * returns 0 if OK, else, errorcode (also will be copied
 * to ecodeptr );
 *
 * Peter 29-May-92: The new force parameter is used to handle
 * coercion.  If force is set, then we re-do the database
 * inquiries for this screen, because promotion might have
 * changed the mapping of the default monitor. */
ULONG setScreenDisplayID (struct Screen *sc, ULONG displayID, ULONG *ecodeptr, ULONG force)
{
    struct MonitorSpec *displayMonitorSpec ();
    DisplayInfoHandle drecord;
    ULONG errorcode = 0;
    union
    {
	struct DisplayInfo dinfo;
	struct MonitorInfo minfo;
    } di;

    DSDID (printf ("setScreenDID, sc %lx, displayID %lx, ecodeptr %lx\n", sc, displayID, ecodeptr));

    /* see if we know this mode	*/
    if (!(drecord = FindDisplayInfo (displayID)) ||
	!GetDisplayInfoData (drecord, (UBYTE *) &di.dinfo, sizeof (di.dinfo), DTAG_DISP, NULL))
    {
	DSDID (printf ("OS: can't find display ID: %lx\n", displayID));
	errorcode = OSERR_UNKNOWNMODE;
    }
    else
    {
	UWORD notavail = di.dinfo.NotAvailable;

	/* get some data out of the DisplayInfo record	*/
	DSDID (printf ("sSDID: found drecord at %lx\n", drecord));

	if (notavail)
	{
	    /* General "unavailable" code, overridden in more specific cases. */
	    errorcode = OSERR_NOTAVAILABLE;
	    if (TESTFLAG (notavail, DI_AVAIL_NOMONITOR))
		errorcode = OSERR_NOMONITOR;
	    if (TESTFLAG (notavail, DI_AVAIL_NOCHIPS))
		errorcode = OSERR_NOCHIPS;
	}
#if 1
	/* need safe initial values in here */
	/* ZZZ: maybe not, if coerceScreens moves to IOpenScreen() */
	XSC (sc)->ScaleFactor.X = di.dinfo.Resolution.x;
	XSC (sc)->ScaleFactor.Y = di.dinfo.Resolution.y;
	XSC (sc)->PixelSpeed = di.dinfo.PixelSpeed;
#endif
	/* This denotes the initial call to setScreenDisplayID(),
	 * from within the bowels of OpenScreen()... */
	if (XSC (sc)->NaturalDisplayID == INVALID_ID)
	{
	    /* Since the DClip was created based on the initial
	     * mode, we need to store the scale-factor of that
	     * mode.  Before promotion existed, we used to just
	     * ask the database what that resolution was.  But
	     * now, the scale factor of the default mode can
	     * change as promotion changes the default monitor.
	     * So we remember the initial one, for use in scaling
	     * dclips, down in coercion.c/coerceScreenGrunt() */
	    XSC (sc)->InitialScaleFactor.X = di.dinfo.Resolution.x;
	    XSC (sc)->InitialScaleFactor.Y = di.dinfo.Resolution.y;
	}

	/* jimm: 5/11/90: I need these for DUALPF, and probably other things ... */
	XSC (sc)->NaturalDProperties = di.dinfo.PropertyFlags;

	/* determine image resolution (from two choices)
	 * ZZZ: this should be fancied up */
	if (di.dinfo.Resolution.x <= MOUSESCALEX)
	    SETFLAG (sc->Flags, SCREENHIRES);
	else
	    CLEARFLAG (sc->Flags, SCREENHIRES);

	/* check to see if monitor change is being requested */
	if ((force) || (MONITOR_PART (XSC (sc)->NaturalDisplayID) != MONITOR_PART (displayID)))
	{
	    DSDID (printf ("setScreenDisplayID: changing monitor %lx/%lx\n",
			   XSC (sc)->NaturalDisplayID, displayID));

	    /* now use di. to get MonitorInfo	*/
	    if (GetDisplayInfoData (drecord, (UBYTE *) &di.minfo, sizeof (di.minfo), DTAG_MNTR, NULL))
	    {
		/* install official new monitor */
		XSC (sc)->NaturalMSpec = di.minfo.Mspc;

		/* Peter 28-Jan-92: We now have draggable Hedley screens,
		 * but they can't be dragged down below the DClip.  Such
		 * an operation doesn't make sense for MCOMPAT_NOBODY
		 * screens, so we note that property here, to pick it
		 * up later as needed.
		 */
		CLEARFLAG (XSC (sc)->PrivateFlags, PSF_INCOMPATIBLE);
		if (di.minfo.Compatibility == MCOMPAT_NOBODY)
		    SETFLAG (XSC (sc)->PrivateFlags, PSF_INCOMPATIBLE);
	    }
	    else
		errorcode = OSERR_UNKNOWNMODE;
	}

	/* setup official mode stuff */
	XSC (sc)->NaturalDRecord = (ULONG) drecord;
	XSC (sc)->NaturalDisplayID = displayID;

	/* set up "artifact" in viewport Modes */
	sc->ViewPort.Modes = (XSC (sc)->NaturalDisplayID & 0xffff) | SPRITES;
	XSC (sc)->VPModeStash = sc->ViewPort.Modes & OLD_MODES;

	DSNOOP (printf ("set mode stash for %lx to %lx\n", sc, XSC (sc)->VPModeStash));

	/* tell graphics */
	VideoControlTags (XSC (sc)->ColorMap,
			  VTAG_NORMAL_DISP_SET, drecord,
			  /* jimm: 5/30/90: stash ID "alias"	*/
			  VTAG_VPMODEID_SET, displayID,
			  TAG_DONE);
    }

    /* copy over ecodeptr only if trouble */
    if (errorcode && ecodeptr)
	*ecodeptr = errorcode;
    DSDID (printf ("sSDID: return %lx\n", errorcode));
    DSDID (kpause ("end of sSDID\n"));
    return (errorcode);
}
