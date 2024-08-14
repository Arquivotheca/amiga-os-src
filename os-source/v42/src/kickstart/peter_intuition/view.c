/*** view.c ***************************************************************
 *
 *  view.c -- view modes, viewport, copper list stuff
 *
 *  $Id: view.c,v 38.35 93/03/17 17:12:46 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

/* Starting with 3.01, graphics allows a user to denote his ViewPort
 * as one where the updating of intermediate copper-lists is to be
 * skipped, for speed.  In such ViewPorts, it is essential that
 * we never MrgCop() them without first calling MakeVPort().
 */
#ifndef CMAF_NO_INTERMED_UPDATE
#define CMAF_NO_INTERMED_UPDATE	2
#endif

/* This optimization depends on GfxBase maintaining a VBlank counter.
 * the idea is that in place of WaitTOF()/FreeXXX(), we send ourselves
 * a token (reminder) to FreeXXX after the VBlank counter has had
 * a chance to change, thus taking a WaitTOF() clean out of RethinkDisplay()
 * and friends.
 */
#define WAITTOF_OPTIMIZE	0

#include "intuall.h"
#include "preferences.h"

#define D(x) 		;
#define DDP(x) 		;	/* dpaint backsync vp/screen	*/
#define DVP(x)		;
#define DC(x) 		;	/* coercion		*/
#define DSNOOP(x) 	;	/* mode snooping	*/

/* hack to sniff out VP dx/dyoffset poke-hackers	*/
#define DPAINT_BACK_SYNC	(1)
				/* rather than sync the viewport to
				 * the screen, let's try the other
				 * way around, and move the DClip too
				 */

#define LONGMATH	(1)
/* OOPS: I should have known that it wasn't worth calling the
 * existing scale routines.  I remember when the scaling was
 * not table driven, though, and was much more fun
 */

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef EXEC_ALERTS_H
#include <exec/alerts.h>
#endif

#ifndef GRAPHICS_GFXNODES_H
#include <graphics/gfxnodes.h>
#endif

#ifndef GRAPHICS_DISPLAY_H
#include <graphics/display.h>
#endif

#ifndef GRAPHICS_DISPLAYINFO_H
#include <graphics/displayinfo.h>
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

#include "view_protos.h"

/*---------------------------------------------------------------------------*/

static ULONG CalcISG(struct Screen * sc);

static int modeSnoop(struct Screen * screen,
                     int force);

static int fixBitMapPokers(struct Screen * sc);

static int rethinkVPorts(int force);

static int laceCheck(void);

static int monitorCheck(void);

static int remakeCopList(void);

/*---------------------------------------------------------------------------*/

extern struct Custom volatile custom;

/* three non-lace rows	*/
#define INTERSCREENLINES	6  /* number of interlace rows */

#define TOPMOSTLIMIT	(0)	/* screen not visible below this are VP_HIDE */


/* Calculate the inter-screen gap, according to graphics.library's
 * CalcIVG() function.  We never let the number be less than the
 * traditional INTERSCREENLINES (6 interlaced lines).
 * Returns the answer in ViewPort resolution lines.
 */

static ULONG
CalcISG( sc )
struct Screen *sc;
{
    struct IntuitionBase *IBase = fetchIBase();
    LONG isg;

    isg = CalcIVG( &IBase->ViewLord, &sc->ViewPort ) * XSC(sc)->ScaleFactor.Y;

    if ( ( !TESTFLAG( XSC(sc)->PrivateFlags, PSF_MINIMIZEISG ) ) && ( isg < INTERSCREENLINES * IBase->MouseScaleY ) )
    {
	isg = INTERSCREENLINES * IBase->MouseScaleY;
    }
    return( isg );
}


/*** intuition.library/MakeScreen ***/

MakeScreen(screen)
struct Screen *screen;
{
    newMakeScreen(screen, TRUE);
}

/* Pass in rethink=FALSE to get a lightweight MakeScreen(), however
 * you promise to follow-up with RethinkDisplay().  Dragging of
 * multiple screens uses this feature.
 */
newMakeScreen(screen,rethink)
struct Screen *screen;
BOOL rethink;
{
    LONG result = 0;
    struct IntuitionBase *IBase = fetchIBase();

    if ( ! screen ) return;

    /* Forbid() replaced by lockIBase(): jimm: 1/6/86 */
    LOCKIBASE();
    LOCKVIEW();

    modeSnoop( screen, 0 );	/* check to see if he's poked his modes */

    /* signal to rethinkVPorts that you need new copper lists remade */
    SETFLAG( XSC(screen)->PrivateFlags, PSF_REMAKEVPORT );

    if ( rethink )
    {
	LOCKVIEWCPR();
	IBase->ViewFailure = 0;
	DVP( printf("MakeScreen calling modeVerify\n") );
	if ( !modeVerify( NOFORCE) )
	{
	    DVP( printf("MakeScreen calling rVP\n") );
	    rethinkVPorts( NOFORCE );
	}
	result = IBase->ViewFailure;
	UNLOCKVIEWCPR();

	/* Peter 4-Feb-91:
	 * updateMousePointer() no longer happens inside rethinkVPorts(), so
	 * we've got to do our own.  See the comment at the end of
	 * rethinkVPorts() for a discussion as to why.
	 */
	updateMousePointer( IBase->SimpleSprite );

    }
    /* Permit replaced by unlockIBase() */
    UNLOCKVIEW();
    UNLOCKIBASE();
    return( result );
}

/*
 * detect and resolve mode changes done by poking viewport.modes
 */
static modeSnoop( screen, force )
struct Screen	*screen;
int force;
{
    ULONG		newdisplayid;

    /* Fix any pokers of BitMap depth or planes that we see.
     * ProPage and ProDraw need this detector here, because
     * they poke their BitMap depth.
     */
    fixBitMapPokers( screen );

    newdisplayid = XSC(screen)->NaturalDisplayID;

    /* Most callers of modeSnoop() call with force=0.  That means,
     * only go through modeSnoop() if the VPModeStash changed.
     *
     * However, for screens of the default monitor, RemakeDisplay()
     * calls us with force=FORCE.  This means we're to call
     * setScreenDisplayID(), because the mapping of the default
     * monitor may have changed due to promotion.
     *
     * Note that the ordering in the OR expression is important,
     * since it affects the value of newdisplayid!!
     */
    if ( ( force ) ||
	( ( newdisplayid = screen->ViewPort.Modes & OLD_MODES ) !=
	XSC(screen)->VPModeStash ) )
    {
	DSNOOP( printf("MakeScreen %lx MODESNOOP new mode: %lx, old mode %lx\n",
		screen, newdisplayid, XSC(screen)->VPModeStash ) );

	/* setup new mode */
	setScreenDisplayID( screen, newdisplayid, NULL, force );

	/* Under V37, we used to reset the DClip to the hull of the
	 * TextOverscan and the screen rectangle.  However, we must
	 * not reset the DClip if we come here through promotion.
	 *
	 * On balance, it was simpler to just remove the code, since
	 * mode-poking is discouraged anyways and hulling with the
	 * screen rectangle ensures non-scrollability provided no-one
	 * spoofs the screen dimensions.
	 */
	/* coerce and xform dclip */
	coerceScreens( screen, FORCE );

	DSNOOP( dumpRect("new DClip", &XSC(screen)->DClip ) );
	DSNOOP( printf("ID now set, new mode stash %lx\n",
		XSC(screen)->VPModeStash ) );

    }
    DSNOOP( else printf("MakeScreen %lx, no mode change: %lx, old mode %lx\n",
		screen, newdisplayid, XSC(screen)->VPModeStash ) );
}


static fixBitMapPokers( sc )
struct Screen *sc;
{
    /* Disney Animation Studio changes the bitplane pointers in
     * the screen->sc_BitMap, then calls ScrollVPort() and
     * RethinkDisplay().  Under V37 and prior, this worked since
     * the ViewPort.RasInfo->BitMap pointed at screen->sc_BitMap.
     * ScrollVPort() remade the copper lists from the bitplane
     * pointers.
     *
     * Under V39, the RasInfo->BitMap is the RealBitMap we got
     * from AllocBitMap().  Poking sc_BitMap thus has no effect.
     * To keep Mickey smiling, we look for changes to the sc_BitMap
     * planes at RethinkDisplay() time.  If we find any, then we
     * do a one-time one-way switch to using the embedded sc_BitMap
     * instead of the RealBitMap.  From that point on, they work
     * just like V37.  To get the first frame to come out right,
     * we note that we must remake the ViewPort.
     *
     * MKS 14-Aug-92:  Now only check the valid plane pointers up
     * to the bitmap's depth.  Also, the test is only done if the
     * RasInfo BitMap itself was not poked.  Some double-buffering
     * programs do that, and we don't want to break them.
     *
     * Note that one last case is not handled by this:
     * Someone poking the plane pointers of the RasInfo->BitMap.
     * So far no one has been shown to have done this.  My feeling
     * is that it is not important since most likely they would have
     * poked the whole bitmap rather than just the plane pointers
     * since it is easier to poke just the bitmap.
     * We check against the hidden screen layer rastport bitmap
     * since the screen rastport bitmap may have been changed
     * at the same time as the RasInfo bitmap was (for rendering
     * reasons)
     */
    if (XSC(sc)->ClipLayer->rp->BitMap == sc->ViewPort.RasInfo->BitMap)
    {
	int plane;
	struct BitMap *embedded_bm = &sc->sc_BitMap;

	for ( plane = 0; plane < embedded_bm->Depth ; plane++ )
	{
	    if ( ( embedded_bm->Depth != XSC(sc)->sc_BitMapCopy.Depth ) ||
		( XSC(sc)->sc_BitMapCopy.Planes[ plane ] != embedded_bm->Planes[ plane ] ) )
		
	    {
		/* Found a bitmap plane poker (eg. Disney Animator) */
		SETFLAG( XSC(sc)->PrivateFlags, PSF_REMAKEVPORT );
		sc->RastPort.BitMap = embedded_bm;
		sc->ViewPort.RasInfo->BitMap = embedded_bm;
		XSC(sc)->ClipLayer->rp->BitMap = embedded_bm;
		sc->BarLayer->rp->BitMap = embedded_bm;
		break;
	    }
	}
    }
}


/*** intuition.library/RemakeDisplay ***/

RemakeDisplay()
/* parameter removed by jimm, 11/4/85 */
{
    struct Screen	*s;
    LONG result;
    struct IntuitionBase *IBase = fetchIBase();

    /* Forbid() replaced by lockIBase(): jimm: 1/6/86 */
    LOCKIBASE();
    LOCKVIEW();
    LOCKVIEWCPR();

    IBase->ViewFailure = 0;

    /* jimm: 5/14/90: put mode snooping for all screens in
     * here, too
     */
    {
	for ( s = fetchIBase()->FirstScreen; s; s = s->NextScreen )
	{
	    /* To handle promotion, we must force modeSnoop() if
	     * the screen's natural mode is of the default monitor
	     */
	    int force = 0;
	    if ( !MONITOR_PART( XSC(s)->NaturalDisplayID ) )
	    {
		force = FORCE;
	    }
	    modeSnoop( s, force );
	}
    }

    modeVerify( FORCE );
    remakeCopList();

    result = IBase->ViewFailure;

    /* Permit replaced by unlockIBase() */
    UNLOCKVIEWCPR();
    UNLOCKVIEW();
    UNLOCKIBASE();

    return( result );
}


/*** intuition.library/RethinkDisplay ***/

RethinkDisplay()
{
    LONG result;
    struct IntuitionBase *IBase = fetchIBase();

    LOCKIBASE();
    LOCKVIEW();
    LOCKVIEWCPR();

    IBase->ViewFailure = 0;
    /* makes all viewports if mode change occured, else minimal amount */
    DVP( printf("RethinkDisplay calling modeVerify\n") );
    if ( ! modeVerify( NOFORCE ) )
    {
	DVP( printf("RethinkDisplay calling RVP\n") );
	rethinkVPorts( NOFORCE );	/* minimal amount done here */
    }
    /* else, modeVerify remade 'em all	*/
    remakeCopList();

    result = IBase->ViewFailure;

    /* Permit replaced by unlockIBase() */
    UNLOCKVIEWCPR();
    UNLOCKVIEW();
    UNLOCKIBASE();

    return( result );
}

/** make sure IBASELOCK is set before calling **/
/*
 * if 'force' is FORCE or MEGAFORCE, will either remake or set VP_HIDE for
 * each viewport.  If MEGAFORCE, will sync up vp dx/yoffsets.
 */

#ifndef VPXF_LAST
#define VPXF_LAST	2
#endif

static rethinkVPorts( force )
int		force;
{
    LONG		alltop;	/* topmost edge above which screens must peek */
    LONG		sctop;	/* screen top in mouse coordinates */
    ULONG		vpheight;	/* viewport height in mouse coords */
    LONG		show_vp;

	/* tracks topmost obscuring screen, in mouse coordinates.	*/
    struct Screen	*sc, *first_screen;
    struct ViewPort	*vport;
    struct IntuitionBase *IBase = fetchIBase();

    assertLock("rethinkVP", IBASELOCK );
    assertLock("rethinkVP", VIEWCPRLOCK );

    if ( !(sc = first_screen = IBase->FirstScreen) )
    {
	return;
    }

    /* start with top most at bottom of front screen, which
     * may well be off display, and clipped by bart.
     * everything is done in mouse coordinates
     */
    alltop = (sc->TopEdge + sc->Height) *
		XSC(sc)->ScaleFactor.Y;

    DVP( printf("first sc top: %ld\n", sc->TopEdge ));
    DVP( printf("initial alltop: %ld\n", alltop ));

    /*
     * for each screen/viewport:
     *	determine if it's visible (won't be if hedley-incompatible)
     *  set up its visible DHeight
     *  remake the viewport if the height changed or if
     *    a mode change mandates that they all be remade
     */
    for ( ; sc; sc = sc->NextScreen )
    {
	/* Fix any pokers of BitMap depth or planes that we see.
	 * Disney Animation Studio is helped by this call here,
	 * since they call ScrollVPort() then RethinkDisplay(),
	 * with no call to MakeScreen().
	 */
	fixBitMapPokers( sc );

	vport = &sc->ViewPort;
	/* For a AA-kludge, graphics needs to know which is the
	 * bottom-most viewport on the screen.  That would be the
	 * viewport of the first screen.
	 */
	XSC(sc)->VPExtra->Flags &= ~VPXF_LAST;
	if ( sc == IBase->FirstScreen )
	{
	    XSC(sc)->VPExtra->Flags |= VPXF_LAST;
	}

	/* Ordinarily, the screen Left/Top are slaved to the ViewPort
	 * Dx/yOffset, which is required in order to support various
	 * overscan techniques of old.  So we only reset the screen VP
	 * position based on the screen Left/Top under drastic circumstances,
	 * which are:
	 * - change in scan rate (=active monitor) (causes MEGAFORCE)
	 * - coercion moving a screen (causes PSF_NEWSCREENOFFSETS)
	 */
	if ( ( force == MEGAFORCE ) ||
	    TESTFLAG( XSC(sc)->PrivateFlags, PSF_NEWSCREENOFFSETS ) )
	{
	    XSC(sc)->VPOffsetStash.X =
		    vport->DxOffset = sc->LeftEdge;
	    XSC(sc)->VPOffsetStash.Y =
		    vport->DyOffset = sc->TopEdge;
	}
#if DPAINT_BACK_SYNC
	else
	{
	    WORD	delleft;
	    WORD	deltop;

	    /* move the screen and the DClip to match VP	*/
	    delleft =  vport->DxOffset - XSC(sc)->VPOffsetStash.X;
	    deltop = vport->DyOffset - XSC(sc)->VPOffsetStash.Y;

	    DDP( printf( "delleft/top %ld %ld\n", delleft, deltop));

	    if ( ( deltop ) || ( delleft ) )
	    {
		DDP( printf("detect vp poke of %ld, %ld\n",
		    delleft, deltop ) );

		/* sync up	*/
		XSC(sc)->VPOffsetStash.X =
		    sc->LeftEdge = vport->DxOffset;
		XSC(sc)->VPOffsetStash.Y =
		    sc->TopEdge = vport->DyOffset;

		/* translate dclip to shadow vp poke	*/
		offsetRect( &XSC(sc)->DClip, delleft, deltop );
		coerceScreens( sc, FORCE );
	    }
	}
#endif

	CLEARFLAG( XSC(sc)->PrivateFlags, PSF_NEWSCREENOFFSETS );

	/* Will assume this viewport is to be hidden until
	 * proven otherwise:
	 */
	show_vp = FALSE;

	/* exclude screens which are not compatible with the
	 * frontmost
	 */

	if ( !TESTFLAG( sc->Flags, SCREENHIDDEN ) )
	{
	    sctop = sc->TopEdge * XSC(sc)->ScaleFactor.Y;
	    if ( sctop < alltop && alltop > TOPMOSTLIMIT )
	    {
		/* vpheight is the amount the screen "peeks over"
		 * the ones in front (or of its own bottom edge,
		 * if it is the frontmost).
		 */
		vpheight = alltop - sctop;
		DVP( printf("vpheight in mouse coords: %ld\n", vpheight));

		vpheight /= XSC(sc)->ScaleFactor.Y;
		DVP( printf("vpheight in screen coords: %ld\n", vpheight));

		/* limit vp allowed height to actual screen height
		 * ("short screens float")
		 */
		vpheight = imin( vpheight, sc->Height );
		DVP( printf("vpheight limited: %ld\n", vpheight ) );

		/* Here's an explanation of when we call MakeVPort()
		 * on a ViewPort.  First, we always MakeVPort() if
		 * in the process of a drastic remake (force = FORCE
		 * or MEGAFORCE, indicating RethinkDisplay() or somesuch).
		 * Second, we remake if the ViewPort height changed
		 * (which could result from sliding a screen which is
		 * below us).  Third, we remake any ViewPort whose screen
		 * has PSF_REMAKEVPORT set in it.  This flag is set
		 * in MakeScreen(), in fixBitMapPokers() if a poker is caught,
		 * or when we determine in this routine that a ViewPort
		 * is hidden (either due to exclusion, geometry or
		 * MakeVPort()-failure.  So, ALWAYS remake the
		 * PSF_REMAKEVPORT guys.
		 *
		 * Starting with 3.01, graphics allows a user to
		 * denote his ViewPort as one where the updating of
		 * intermediate copper-lists is to be skipped, for
		 * speed.  In such ViewPorts, it is essential that we
		 * never MrgCop() them without first calling
		 * MakeVPort().
		 *
		 * There's no explicit check for ViewPort DxOffset/DyOffset
		 * changes, because that only happens inside screen-dragging
		 * (which calls MakeScreen()), or due to coercion (which
		 * sets the force variable).  There's actually an
		 * external reliance on that now, since we can have an
		 * off-the-bottom ViewPort, which Intuition doesn't care
		 * about, but graphics (in 3.01 and up) generates no
		 * intermediate copper lists for.  We count on MakeVPort()
		 * being called for this ViewPort if ever it's moved
		 * on-screen.  As I just said, screen-dragging and coercion
		 * are the possible ways, and they're covered.
		 */

		/* According to the geometry of the screens in the system,
		 * this ViewPort is supposed to be visible.  The only
		 * thing that will stop us now is a MakeVPort() failure.
		 */
		show_vp = TRUE;

		if ( ( force != NOFORCE ) ||
		    ( vport->DHeight != vpheight ) ||
		    TESTFLAG( XSC(sc)->PrivateFlags, PSF_REMAKEVPORT ) ||
		    TESTFLAG( XSC(sc)->ColorMap->AuxFlags, CMAF_NO_INTERMED_UPDATE ) )
		{
		    vport->DHeight = vpheight;
		    /* to coerceScreens(): rethinkScreenClip( sc ); */
		    setSpriteSpeed( sc );
		    if ( MakeVPort( &IBase->ViewLord, vport ) )
		    {
			/* Oh no!  Hide that screen! */
			show_vp = FALSE;
			IBase->ViewFailure = 1;
		    }
		}
	    }
	    DVP( else printf("rVP: hidden on account of VP coordinates out of range\n") );
	}
	DVP( else printf("rVP: excluding screen: %lx\n", sc ) );


	if ( show_vp )
	{
	    /* The screen is visible, so mark it accordingly */
	    CLEARFLAG( vport->Modes, VP_HIDE );
	    CLEARFLAG( XSC(sc)->PrivateFlags, PSF_REMAKEVPORT );

	    /* CalcISG() is an Intuition function to figure out the
	     * interscreen-gap, given a screen.  It returns an
	     * answer in ticks based on graphics.library CalcIVG().
	     */
	    alltop = sctop - CalcISG( sc );
	    DVP( printf(" last screentop: %ld\n", sctop ) );
	    DVP( printf(" alltop now left at: %ld\n", alltop ) );
	}
	else
	{
	    /* The screen is hidden on account of one of the following:
	     * - it was excluded by the front screen
	     * - its viewport coordinates are out of range
	     * - MakeVPort() failed.
	     *
	     * We set PSF_REMAKEVPORT because we want to try MakeVPort()
	     * again next time anything changes.
	     */
	    SETFLAG( vport->Modes, VP_HIDE );
	    SETFLAG( XSC(sc)->PrivateFlags, PSF_REMAKEVPORT );
	}
    }

    /* won't wrongly free up mouse if INMOUSELIMITS is set,
     * by screen drag, window size/drag, menu, ...
     */
    freeMouse();

    /* Peter 4-Feb-91:
     * If we got here as a result of a mode switch, then the screens'
     * SpriteFactors resemble the future display, not the currently
     * displayed copper lists.  In such cases, remakeCopList() is
     * imminent, and that'll fix everything up.  In fact, remakeCopList()
     * will cause updateMousePointer() to happen anyways.
     * We used to call updateMousePointer() here, but that would cause the
     * sprite to jump.  So instead, we rely on our callers to call
     * updateMousePointer(), either through remakeCopList() or directly.
     * Our callers (direct and those through modeVerify()):
     *     MakeScreen() - needs its own updateMousePointer()
     *     RethinkDisplay() - OK since it calls remakeCopList() immediately
     *     RemakeDisplay() - OK since it calls remakeCopList() immediately
     *     initIntuition() - OK since it eventually calls setPrefs() which
     *         calls setIBasePrefs() which calls RemakeDisplay()
     *
     * So we don't do this here any more:
     *
     * updateMousePointer(NULL);
     *
     */
}

/*
 * determines correct display modes (sets ScanRate and LACE)
 * if changes or 'force', syncs other display mode dependent
 * parameters (view extend, view offsets, dclip, mouse constraint)
 * and rebuilds all viewports.
 *
 * Returns TRUE if it decided to remake all viewports, so caller
 * shouldn't bother to call rethinkVPorts again.
 *
 * called by RethinkDisplay (force == NOFORCE),
 * RemakeDisplay (force == FORCE)
 *
 * might call rethinkVPorts with MEGAFORCE if monitorCheck()
 * requires global remake and sync.
 */
modeVerify( force )
int	force;
{
    int megaforce;

    assertLock("modeVerify", VIEWCPRLOCK );
    /* always do monitorCheck, but skip first laceCheck if 'force'	*/
    /* side effects of these check functions:
     *	monitorCheck sets up IBase->ActiveMonitorSpec
     *	laceCheck sets up IBase->ViewLord.Modes:INTERLACE;
     */
    if ( (megaforce = monitorCheck()) ||
	  (force != NOFORCE)  ||
	  laceCheck() )
    {
	/* set scalefactors, positions, of screens for
	 * current monitor.  If we have a monitor change, this
	 * function will resync ViewLord position to per-view
	 * preferences positions.  Otherwise,  let 'em float.
	 *
	 * If the monitor changed, we want to pass MEGAFORCE
	 * (comes from 'megaforce').  If we were called with
	 * 'force' = FORCE, we want to pass FORCE along.
	 * If none of this is true, then we're here only because
	 * laceCheck() sent us here, in which case we want NOFORCE.
	 * This avoids coercion-recalculation when the ViewPort
	 * INTERLACE changes during screen sliding.
	 *
	 */
	setupMonitor( imax(force,megaforce) );

	laceCheck();		/* recalc lace after setupMonitor()	*/

	/* here's the tricky part: we definitely want all copper lists
	 * remade, but we only want to sync viewport positions if
	 * we have a monitor change
	 */
	rethinkVPorts( imax(FORCE, megaforce) );
				/* make all vp's and clips		*/
	return ( TRUE );
    }
    return ( FALSE );
    /* caller will probably be calling rethinkVPorts( NOFORCE );	*/
}

/* returns non-FALSE if LACE state has changed
 *
 * NOTE: duplicates rethinkVPorts loop to calculate viewport visibility.
 */
static laceCheck()
{
    struct IntuitionBase *IBase = fetchIBase();
    LONG alltop, sctop;
    struct Screen *sc;
    UWORD needlace = 0;
    UWORD oldlace = 0;

    /* save lace state to detect changes */
    oldlace = IBase->ViewLord.Modes & INTERLACE;

    /* starting assumption, might override below	*/
    CLEARFLAG( IBase->ViewLord.Modes, INTERLACE );

    /* this "screen visibility loop" is from rethinkVPorts,
     * where it is nicely commented
     */
    if ( sc = IBase->FirstScreen )
    {
	alltop = (sc->TopEdge + sc->Height) * XSC(sc)->ScaleFactor.Y;
    }
    for ( ; sc; sc = sc->NextScreen )
    {
	if ( TESTFLAG( sc->Flags, SCREENHIDDEN ) )
	{
	    continue;
	}
	else
	{
	    sctop = sc->TopEdge * XSC(sc)->ScaleFactor.Y;
	    if ( sctop < alltop && alltop > TOPMOSTLIMIT )
	    {
		/* set LACE bit in needlace if any of
		 * these visible screens are interlaced
		 */
		needlace |= XSC(sc)->DProperties & DIPF_IS_LACE;
		alltop = sctop - CalcISG( sc );
	    }
	}
    }

    if ( needlace ) SETFLAG( IBase->ViewLord.Modes, INTERLACE );

    /* return TRUE if lace mode has toggled */
    DSNOOP( if ( (needlace && !oldlace ) || (oldlace && !needlace ) )
	printf("laceCheck hit %lx/%lx\n", needlace, oldlace));
    return ( (needlace && !oldlace ) || (oldlace && !needlace ) );
}

/* make sure IBASELOCK is set before calling
 * sets ActiveMonitorSpec and returns MEGAFORCE if it has changed.
 */
static monitorCheck()
{
    struct IntuitionBase *IBase = fetchIBase();
    struct Screen *screen;
    struct MonitorSpec *oldmspec;

    assertLock("monitorCheck", IBASELOCK );

    if ( !(screen =  IBase->FirstScreen) )
    {
	/* defaultest mode	*/
	IBase->ActiveMonitorSpec = NULL;
	return( 0 );
    }

    /* else	*/
    oldmspec = IBase->ActiveMonitorSpec;

    DSNOOP( if ( XSC(screen)->NaturalMSpec != oldmspec )
	printf("monitorCheck hit %lx/%lx\n",
	XSC(screen)->NaturalMSpec, oldmspec));
     /* Peter 8-Aug-90: Now return MEGAFORCE if FirstScreen is SCREENHIDDEN.
      * We need this to allow the user to pop an excluded screen to the front.
      */
    if ( ( (IBase->ActiveMonitorSpec = XSC(screen)->NaturalMSpec)!=oldmspec) ||
       (screen->Flags & SCREENHIDDEN) )
	return ( MEGAFORCE );
    /* else */
	return ( NOFORCE );
}


static remakeCopList()
{
    struct IntuitionBase *IBase = fetchIBase();
    struct cprlist *LOFSave, *SHFSave;
    BOOL	valid_cop_lists;

    assertLock("remakeCopList", VIEWLOCK);
    assertLock("remakeCopList", VIEWCPRLOCK);

    /* first, save the pointers to the current copper lists */
    LOFSave = IBase->ViewLord.LOFCprList;
    SHFSave = IBase->ViewLord.SHFCprList;

    /* now zero out the pointers so that the next call to MrgCop() will
     * cause brand-new copper lists to be allocated and initialized
     */
    IBase->ViewLord.LOFCprList = NULL;
    IBase->ViewLord.SHFCprList = NULL;
    /* Peter 15-Nov-90: MrgCop() now returns NULL for success */
    if ( !MrgCop(&IBase->ViewLord) )
    {
	/* call LoadView() to create the display with these new copper lists */
	LoadView(&IBase->ViewLord);
	valid_cop_lists = TRUE;
	/* When the active monitor changes, graphics needs a chance
	 * to smell the sprite.  So calling updateMousePointer()
	 * here will cause MoveSprite() to happen.
	 */
	updateMousePointer( IBase->SimpleSprite );
    }
    else
    {
	/* mrgcop failure -- bart */
	LoadView(NULL);
	valid_cop_lists = FALSE;
    }

#if WAITTOF_OPTIMIZE
    /* Up until 3.00, we used to do a WaitTOF() here, then call FreeCprList()
     * on the LOFSave and SHFSave saved copper list pointers.  However,
     * we take an optimization that depends on GfxBase maintaining a VBlank
     * counter.  the idea is that in place of WaitTOF()/FreeXXX(), we send
     * ourselves a token (reminder) to FreeXXX after the VBlank counter has
     * had a chance to change, thus taking a WaitTOF() clean out of
     * RethinkDisplay() and friends.
     */
    sendISMNoQuick( itFREECPRLIST, LOFSave, SHFSave,
	IBase->GfxBase->VBCounter );
#else
    /* wait to the top of the frame to make sure that the old lists are
     * for sure no longer being displayed
     */
    WaitTOF();

    /* now, feel through the old copper list structures, freeing the memory */
    FreeCprList(LOFSave);
    FreeCprList(SHFSave);
#endif

    /* Peter 15-Nov-90: We couldn't get the copper lists the first time,
     * so retry now that the old ones are freed */
    if ( ( !valid_cop_lists ) && ( !MrgCop(&IBase->ViewLord) ) )
    {
	/* call LoadView() to create the display with these new copper lists */
	LoadView(&IBase->ViewLord);
	valid_cop_lists = TRUE;

	/* When the active monitor changes, graphics needs a chance
	 * to smell the sprite.  So calling updateMousePointer()
	 * here will cause MoveSprite() to happen.
	 */
	updateMousePointer( IBase->SimpleSprite );
    }

    if ( valid_cop_lists )
    {
	/* ZZZ: We could get away with a single updateMousePointer()
	 * in here, instead of the two above, except for the bug in which the
	 * mouse disappears if it's one scan line above the screen.
	 */

	/* According to jimm, this is needed for the very first
	 * time the display is turned on.
	 */
	ON_DISPLAY;
	ON_SPRITE;
    }
    else
    {
	IBase->ViewFailure = 1;
    }
}

