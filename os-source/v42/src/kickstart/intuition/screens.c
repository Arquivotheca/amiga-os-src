/*** screens.c ***************************************************************
 *
 *  screens.c -- mostly open/close screen and their support
 *
 *  $Id: screens.c,v 40.0 94/02/15 17:28:51 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "intuall.h"

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

#ifndef GRAPHICS_GFXMACROS_H
#include <graphics/gfxmacros.h>
#endif

#ifndef GRAPHICS_DISPLAYINFO_H
#include <graphics/displayinfo.h>
#endif

#ifndef HARDWARE_CUSTOM_H
#include <hardware/custom.h>
#endif

#ifndef HARDWARE_DMABITS_H
#include <hardware/dmabits.h>
#endif

#ifndef UTILITY_PACK_H
#include <utility/pack.h>
#endif

#include "imageclass.h"

/*****************************************************************************/

#define D(x)	;
#define DDPF(x)	;		/* dual playfield	*/
#define DOSS(x)	;		/* open sys screen	*/
#define DIM(x)	;		/* system images	*/
#define DPS(x)	;		/* public screens	*/
#define DCWB(x)	;		/* close workbench	*/
#define DCS(x)	;		/* CloseScreen		*/
#define DOS(x)	;		/* open screen		*/
#define DOST(x)	;		/* open screen tag dump		*/
#define DSCAN(x) ;		/* overscan		*/
#define DSG(x)	;		/* screen gadgets	*/
#define DRI(x)	;		/* debug renderinfo	*/
#define DST(x)	;		/* screen title		*/
#define DHEDLEY(x) ;		/* V35 compatibility	*/
#define DCBM(x)	;		/* custom bitmaps	*/
#define DOSF(x)	;		/* OpenScreen failure test code:
			 * Use {SA_Dummy, num} to cause failure
			 * in different places
			 */
#define DPEN(x) ;		/* Graphics ObtainPen() stuff */

/*****************************************************************************/

/*** intuition.library/OpenScreen ***/

/* Peter 23-Aug-90: In KS 2.01, SA_BitMap doesn't cause CUSTOMBITMAP
 * to be set in screen->Flags.  Although this is fixed for 2.02, we have
 * publicly stated you can set this flag yourself after your screen opens
 * with no ill effects.  Remember that years from now!
 */

BYTE loresBorders[8] =
{
    1,				/* barvborder	*/
    2,				/* barhborder	*/
    4,				/* menuvborder	*/
    4,				/* menuhborder	*/
    2,				/* wbortop	*/
    4,				/* wborleft	*/
    4,				/* wborright	*/
    2,				/* wborbottom	*/
};

/*****************************************************************************/

#if 0
#define NS(field)	(ns? (ns->field): 0)
#endif
#define NS(field)	(nscopy.field)

/*****************************************************************************/

/* Structure to hold the screen flags before the Screen is allocated: */
struct SFlagTrap
{
    UWORD Flags;
    ULONG PrivateFlags;
};

/*****************************************************************************/

ULONG screenFlagsPackTable[] =
{
    PACK_STARTTABLE (SA_Dummy),

    PACK_WORDBIT (SA_Dummy, SA_ShowTitle, SFlagTrap, Flags, PKCTRL_BIT, SHOWTITLE),
    PACK_WORDBIT (SA_Dummy, SA_Behind, SFlagTrap, Flags, PKCTRL_BIT, SCREENBEHIND),
    PACK_WORDBIT (SA_Dummy, SA_Quiet, SFlagTrap, Flags, PKCTRL_BIT, SCREENQUIET),
    PACK_WORDBIT (SA_Dummy, SA_AutoScroll, SFlagTrap, Flags, PKCTRL_BIT, AUTOSCROLL),
    PACK_WORDBIT (SA_Dummy, SA_SharePens, SFlagTrap, Flags, PKCTRL_BIT, PENSHARED),

    PACK_LONGBIT (SA_Dummy, SA_Draggable, SFlagTrap, PrivateFlags, PKCTRL_BIT, PSF_DRAGGABLE),
    PACK_LONGBIT (SA_Dummy, SA_Exclusive, SFlagTrap, PrivateFlags, PKCTRL_BIT, PSF_EXCLUSIVE),
    PACK_LONGBIT (SA_Dummy, SA_MinimizeISG, SFlagTrap, PrivateFlags, PKCTRL_BIT, PSF_MINIMIZEISG),

    PACK_ENDTABLE,
};

/*****************************************************************************/

/* default color scheme	*/
/* WARNING: keep an eye on the size of these guys */
extern UWORD fourColorPens[NUMDRIPENS + 1];
extern UWORD twoColorPens[NUMDRIPENS + 1];

/*****************************************************************************/

struct IScreenModePrefs DefaultScreenMode =
{
    HIRES_KEY,
    (UWORD) STDSCREENWIDTH,
    (UWORD) STDSCREENHEIGHT,
    2,
    0
};

/*****************************************************************************/

/* Detach any children this screen may have, then detach this screen
 * from its parent, if any.
 */
static void detachScreen (struct Screen *sc)
{
    struct MinNode *node, *succ;

    /* Automatically detach each child screen, if any */
    for (node = XSC (sc)->Family.mlh_Head; succ = node->mln_Succ; node = succ)
    {
	/* Although one of the screens on the Family list is the
	 * screen itself, it's harmless to detach it, so we
	 * don't bother with the usual (child != sc) test. */
	attachScreen (SCREENFROMNODE (node), NULL);
    }

    /* Automatically detach this screen from any parent */
    attachScreen (sc, NULL);
}

/*****************************************************************************/

/* Common code that frees things associated with screens, called
 * by OpenScreenTagList's failure, and by ICloseScreen. */
static void closeScreenCommon (struct Screen *sc)
{
    LONG gindex;

    if (sc)
    {
	if (sc->BarLayer)
	{
	    CloseFont (sc->BarLayer->rp->Font);
	    DeleteLayer (NULL, sc->BarLayer);
	}

	if (XSC (sc)->ClipLayer)
	{
	    /* Peter 18-Nov-91: We used to set the RastPort mask to
	     * zero so the "underlying bits" aren't cleared.  Before
	     * we did this, the custom bitmap was getting cleared
	     * at this point.  V39 layers supports a NULL Layer_Info
	     * backfill hook, so we use that instead.
	     */
	    DeleteLayer (NULL, XSC (sc)->ClipLayer);
	}
	if (XSC (sc)->ClipLayer_Info)
	    DisposeLayerInfo (XSC (sc)->ClipLayer_Info);

	ThinLayerInfo (&sc->LayerInfo);

	freeSysGadgets (sc->FirstGadget);
	for (gindex = 0; gindex < GADGETCOUNT; gindex++)
	{
	    DisposeObject (XSC (sc)->SysImages[gindex]);
	}

	/* Only close the font if I indeed opened it.  InitRastPort() of the
	 * screen's RastPort puts GfxBase->DefaultFont in it.  If OpenScreen()
	 * failed between the calls to InitRastPort() and the call to
	 * OpenFont(), we would CloseFont() GB->DefaultFont by mistake.
	 * We could have NULLed out RP.Font after InitRastPort(), but I
	 * feared a fontless RastPort.  This is safer.
	 */
	if ((sc->RastPort.Font) && TESTFLAG (XSC (sc)->PrivateFlags, PSF_SETFONT))
	{
	    CloseFont (sc->RastPort.Font);
	}

	FreeColorMap (XSC (sc)->ColorMap);
	if (XSC (sc)->VPExtra)
	    GfxFree (XSC (sc)->VPExtra);

	DisposeObject (XSC (sc)->SDrawInfo.dri_AmigaKey);
	DisposeObject (XSC (sc)->SDrawInfo.dri_CheckMark);

	/* Peter 28-Jan-92: HMM.  It seems we really should
	 * WaitBlit() before freeing the bitmap!!!
	 * Martin T. suggests WaitBlit() even for custom bitmap
	 * screens, to prevent some application bugs.
	 */
	WaitBlit ();
	if ((sc->Flags & CUSTOMBITMAP) == 0)
	{
	    /* New graphics.library function that frees the bitmap
	     * and its planes.
	     */
	    FreeBitMap (XSC (sc)->RealBitMap);
	}

	{
	    struct RasInfo *ri = sc->ViewPort.RasInfo;

	    if (ri)
	    {
		if (ri->Next)
		{
		    DDPF (printf ("free second rasinfo at %lx\n", ri->Next));
		    FreeMem (ri->Next, sizeof (struct RasInfo));
		}
		FreeMem (ri, sizeof (struct RasInfo));
	    }
	}

	/* finally, goodbye Screen! */
	FreeMem (sc, sizeof (struct XScreen));
    }
}

/*****************************************************************************/

/* This is the heart of OpenScreen-processing.  This function takes
 * a NewScreen structure and a taglist.  It also takes a pointer
 * to an IScreenModePrefs structure, which, if non-NULL, is to
 * be used to initialize parameters to be Workbench-like.
 * OpenWorkBench uses this feature, as does the new SA_LikeWorkbench tag. */
static struct Screen *realOpenScreenTagList (struct NewScreen *ns, struct IScreenModePrefs *sm, struct TagItem *tags)
{
    extern UBYTE *SysIClassName;
    struct TextFont *open_font;
    struct IntuitionBase *IBase = fetchIBase ();

    /* pointers */
    struct Screen *sc = NULL;
    struct ViewPort *vp;
    struct BitMap *bitmap;
    struct RasInfo *rinfo;

    /* screen dimensions */
    struct IBox screenbox;
    struct Rectangle *dclip;
    LONG stddclip;

    /* public screens */
    /* if this is non-NULL, it means we have a public screen */
    struct PubScreenNode *pubnode = NULL;
    UBYTE *pubname;

    /* general */
    ULONG errorcode = OSERR_NOMEM;
    ULONG sysfont;
    ULONG longflags;
    ULONG screentype;
    LONG depth;
    struct SFlagTrap screentrap;
    struct NewScreen nscopy;

    DOSF (LONG failnum = 0);

    DOS (printf ("OPENSCREEN: %lx/%lx\n", ns, tags));

    DOS (printf (" --\nOpenScreenTagList: ns at %lx tags at %lx\n", ns, tags));
    DOST (dumpTagItems (NULL, tags));

    clearWords ((UWORD *) &nscopy, sizeof (struct NewScreen) / sizeof (WORD));

    nscopy.Width = STDSCREENWIDTH;
    nscopy.Height = STDSCREENHEIGHT;
    nscopy.BlockPen = 1;

    if (ns)
    {
	nscopy = *ns;
    }

    /* ---------------------------------------------------------------- */
    /* public screens.  check if it is already open, allocate a node 	*/
    /* ---------------------------------------------------------------- */
    /* The locking issues around the uniqueness test are tricky,
     * and there was a hole in V36-V39.  We want to do the uniqueness
     * test early, so we can fail quickly and without any flashing.
     * However, we can't link the pubscreen node onto the PubScreenList
     * until the screen is fully open, since apps can access that list.
     * Also, we can't hold the IBASELOCK across the guts of OpenScreen().
     * Thus, there was a time between the calls to initPubScreen()
     * and linkPubScreen() where a second screen of the same name
     * could pass the uniqueness test in initPubScreen.
     * The fix I chose for V40 was to create a second list, the
     * PendingPubScreens list, and initPubScreen() puts pubscreen nodes
     * there while still inside the same locking session as the uniqueness
     * test, which now has two lists to examine.
     */

    DOS (printf ("handle pubscreen\n"));

    /* If you compile with DOSF(), you can force failure at different points
     * with different values of the unused SA_Dummy tag.
     */
    DOSF (failnum = GetUserTagData0 (SA_Dummy, tags));
    DOSF (printf ("DEBUG -- failnum %ld\n", failnum));

    if (pubname = (UBYTE *) GetUserTagData0 (SA_PubName, tags))
    {
	DOS (printf ("pubname: %s.\n", pubname));

	/* initPubScreen updates the errorcode (only if problems)	*/
	pubnode = initPubScreen (pubname,
				 (struct Task *) GetUserTagData0 (SA_PubTask, tags),
				 GetUserTagData (SA_PubSig, -1, tags),
				 (LONG *)&errorcode);
	if (!pubnode)
	{
	    goto BADSCREENOPEN;
	}
	DOSF (if (!(--failnum))
	      {
	      printf ("OSFailure: initPubScreen\n"); goto BADSCREENOPEN;
	      }
	);
    }

    /* ---------------------------------------------------------------- */
    /*	set up 'flags'							*/
    /* ---------------------------------------------------------------- */

    depth = NS (Depth);

    /* accumulate boolean state */
    DOS (printf (" newscreen flags (Type)  %lx\n", NS (Type)));

    screentrap.Flags = (NS (Type) & ~NS_EXTENDED) | SHOWTITLE;
    screentrap.PrivateFlags = PSF_DRAGGABLE;

    /* If this is a Workbench-like screen, then SHAREPENS is TRUE,
     * and depth and AUTOSCROLL depend on the supplied IScreenModePrefs.
     */
    if (sm)
    {
	screentrap.Flags |= PENSHARED;
	screentrap.PrivateFlags |= PSF_MINIMIZEISG;
	if (TESTFLAG (sm->ism_Control, WBAUTOSCROLL))
	{
	    screentrap.Flags |= AUTOSCROLL;
	}
	depth = sm->ism_Depth;
    }

    PackStructureTags (&screentrap, screenFlagsPackTable, tags);
    longflags = screentrap.Flags;

    DOS (printf ("longflags accumulated %lx\n", longflags));

    /* screen type */
    screentype = GetUserTagData (SA_Type, NS (Type) & SCREENTYPE, tags);

    if (screentype != WBENCHSCREEN)
    {
	screentype = pubnode ? PUBLICSCREEN : CUSTOMSCREEN;
    }
    longflags = (longflags & ~SCREENTYPE) | screentype;

    DOS (printf ("final flags/type are %lx\n", longflags));

#define NEWDEFAULTDEPTH (1)
    if (!(depth = GetUserTagData (SA_Depth, depth, tags)))
    {
	depth = NEWDEFAULTDEPTH;
    }

    /* got to be careful here.
     * I am supposed to ignore ns->CustomBitMap if the
     * bit isn't set, but I want non-zero SA_BitMap to imply
     * custombitmap without the separate flag being set.
     */

    bitmap = NULL;

    if (TESTFLAG (longflags, CUSTOMBITMAP))
	bitmap = NS (CustomBitMap);

    DCBM (printf ("NewScreen.CustomBitMap = $%lx\n", bitmap));
    bitmap = (struct BitMap *) GetUserTagData (SA_BitMap, (ULONG)bitmap, tags);
    DCBM (printf ("SA_BitMap makes it = $%lx\n", bitmap));
    if (bitmap)
    {
	depth = bitmap->Depth;
    }

    /* ---------------------------------------------------------------- */
    /*	allocate and link basic screen data structures, no rasters yet	*/
    /* ---------------------------------------------------------------- */
    DOS (printf ("basic data structures\n"));

    {
	/* Number of colormap entries to get.  Will be 1<<depth, but
	 * never less than 32.  Can be increased by SA_ColorMapEntries.
	 */
	ULONG entries;

	entries = imax (GetUserTagData0 (SA_ColorMapEntries, tags),
			imax (1 << depth, 32));

	/* Peter 26-Jun-91:  NB: we allocate rinfo last, since it won't
	 * be freeable until we stick it into the ViewPort later.
	 * That way, it won't fail to get freed.
	 */
	if (
	       (!(sc = (struct Screen *) AllocMem (
						      sizeof (struct XScreen), MEMF_PUBLIC | MEMF_CLEAR)))
	       || (!(XSC (sc)->VPExtra =
		      (struct ViewPortExtra *) GfxNew (VIEWPORT_EXTRA_TYPE)))
	       || (!(XSC (sc)->ColorMap = (struct ColorMap *) GetColorMap (entries)))
	       || (!(rinfo = (struct RasInfo *) AllocMem (
							     sizeof (struct RasInfo), MEMF_PUBLIC | MEMF_CLEAR)))
	)
	{
	    goto BADSCREENOPEN;
	}
    }

    /* init and link them all up */
    InitRastPort (&sc->RastPort);
    InitVPort (vp = &sc->ViewPort);

    /* WARNING: don't forget that InitVPort clears RasInfo pointer */
    vp->RasInfo = rinfo;

    DOSF (if (!(--failnum))
	  {
	  printf ("OSFailure: screen/VPExtra/ColorMap/RasInfo\n"); goto BADSCREENOPEN;
	  }
    );

    /* NOTE: will set up second RasInfo after I've gotten
     * display mode property flags below
     */

    DOS (printf ("call videocontrol for color map %lx\n", XSC (sc)->ColorMap));

    /* Tell graphics about attachments.
     * Peter 27-Feb-92: User can pass additional tags in
     * SA_VideoControl
     */
    VideoControlTags (XSC (sc)->ColorMap,
		      VTAG_ATTACH_CM_SET, vp,
		      VTAG_VIEWPORTEXTRA_SET, XSC (sc)->VPExtra,
		      TAG_MORE, GetUserTagData0 (SA_VideoControl, tags));

    DOS (printf ("after VCTags\n"));

    /* ---------------------------------------------------------------- */
    /*	obtain display specification from 16 or 32 bit specifiers	*/
    /* ---------------------------------------------------------------- */
    /* set up an invalid state for "old mode" */
    XSC (sc)->NaturalDisplayID = INVALID_ID;

    /* set up (uncoerced) screen mode */
    DOS (printf ("displayID: %lx\n",
		 GetUserTagData (SA_DisplayID, NS (ViewModes) & OLD_MODES, tags)));

    /* jimm: 1/17/90: put support in for old V35 Hedley mode
     * specification
     */
    {
	struct TagItem *tagitem;
	ULONG displayid;

	displayid = INVALID_ID;

	if (tagitem = FindTagItem (NSTAG_EXT_VPMODE, tags))
	{
	    DHEDLEY (printf ("V35 compatibility: ext_vpmode %lx\n",
			     tagitem->ti_Data));

	    if (tagitem->ti_Data == (VPF_A2024 | VPF_TENHZ))
	    {
		DHEDLEY (printf ("doing ten hertz\n"));
		displayid = A2024TENHERTZ_KEY;
	    }
	    else if (tagitem->ti_Data == (VPF_A2024))
	    {
		DHEDLEY (printf ("doing 15 hertz\n"));
		displayid = A2024FIFTEENHERTZ_KEY;
	    }
	}

	/* If this screen is not a V35-style Hedley display, we
	 * first look at the NewScreen ViewModes.  If a ScreenMode
	 * preferences (sm) was supplied, we use that instead.
	 * If SA_DisplayID was supplied, that overrides all.
	 */
	if (displayid == INVALID_ID)
	{
	    displayid = NS (ViewModes) & OLD_MODES;
	    if (sm)
	    {
		displayid = sm->ism_DisplayID;
	    }
	    displayid = GetUserTagData (SA_DisplayID, displayid, tags);
	}

	/* now try to use the deduced displayid */
	if (setScreenDisplayID (sc, displayid, &errorcode, 0))
	{
	    DOS (printf ("setScreenDisplayID failed, returned %lx\n", errorcode));
	    goto BADSCREENOPEN;
	}
	DOSF (if (!(--failnum))
	      {
	      printf ("OSFailure: setScreenDisplayID\n"); goto BADSCREENOPEN;
	      }
	);
    }

    DOS (printf ("after setScreenDisplayID\n"));

    /* (Brace for local variables).  Fail if this screen is too deep.
     */
    {
	struct DimensionInfo diminfo;

	if ((GetDisplayInfoData ((DisplayInfoHandle)XSC (sc)->NaturalDRecord, (UBYTE *)&diminfo,
		   sizeof (struct DimensionInfo), DTAG_DIMS, NULL)) &&
	     (depth > diminfo.MaxDepth))
	{
	    errorcode = OSERR_TOODEEP;
	    goto BADSCREENOPEN;
	}
	DOSF (if (!(--failnum))
	      {
	      printf ("OSFailure: too deep\n"); goto BADSCREENOPEN;
	      }
	);
    }

    /*  jimm: 5/11/90: set up second RasInfo for DUALPF screens */
    /* too bad that I couldn't do a "more magic" setup for
     * "new" programs.  You can still define SA_DualPFMagic
     */
    if (TESTFLAG (XSC (sc)->NaturalDProperties, DIPF_IS_DUALPF))
    {
	DDPF (printf ("going for second rasinfo "));
	if (!(vp->RasInfo->Next = (struct RasInfo *) AllocMem (
								  sizeof (struct RasInfo), MEMF_PUBLIC | MEMF_CLEAR)))
	{
	    goto BADSCREENOPEN;
	}
	DOSF (if (!(--failnum))
	      {
	      printf ("OSFailure: RasInfo->Next\n"); goto BADSCREENOPEN;
	      }
	);
	DDPF (printf (" at %lx\n", vp->RasInfo->Next));
    }

    /* go figure out screen dims and dclip */

    /* ---------------------------------------------------------------- */
    /*	determine screen dimensions, including STDSCREENHEIGHT/WIDTH	*/
    /* ---------------------------------------------------------------- */
    DOS (printf ("screen dimensions\n"));

    /* default is {0,0,-1,-1}, which is set up in the nscopy */
    copyBox ((struct IBox *)&nscopy.LeftEdge, &screenbox);

    DOS (dumpBox ("initial screenbox", &screenbox));

    stddclip = 0;
    /* If an IScreenModePrefs was supplied, override width, height,
     * and stddclip from there...
     */
    if (sm)
    {
	screenbox.Width = sm->ism_Width;
	screenbox.Height = sm->ism_Height;
	stddclip = OSCAN_TEXT;
    }
    /* override with tag specs */
    screenbox.Left = GetUserTagData (SA_Left, screenbox.Left, tags);
    screenbox.Top = GetUserTagData (SA_Top, screenbox.Top, tags);
    screenbox.Width = GetUserTagData (SA_Width, screenbox.Width, tags);
    screenbox.Height = GetUserTagData (SA_Height, screenbox.Height, tags);

    DOS (dumpBox ("screenbox after tags", &screenbox));

    stddclip = GetUserTagData (SA_Overscan, stddclip, tags);
    dclip = (struct Rectangle *) GetUserTagData0 (SA_DClip, tags);

    /* get rectangle for display clip overscan to be used	*/
    if (dclip)
    {
	/* specified explicitly */
	XSC (sc)->DClip = *dclip;
    }
    else
    {
	/* standard/default values */
	DOS (printf ("using default or stddclip %ld\n", stddclip));
	displayOScan (XSC (sc)->NaturalDRecord, NULL,
		      stddclip ? stddclip : OSCAN_TEXT, &XSC (sc)->DClip);
    }
    DOS (dumpRect ("DClip: ", &XSC (sc)->DClip));
    /* get overscan dims for STDSCREENWIDTH/HEIGHT
     * jimm: 3/12/89 * make STDSCREENWIDTH/HEIGHT relative
     * to the display clip specified or implied
     */

    /* convert standard screen dims to some standard overscan dims
     * and position
     */
    if (screenbox.Width == STDSCREENWIDTH)
    {
	screenbox.Width = rectWidth (&XSC (sc)->DClip);

	/* auto-setting left/top to dclip only applies if STDSCREENdims given */
	/* use DClip autoposition if NO left/top  have been
	 * provided
	 */
	if (!ns && !FindTagItem (SA_Left, tags))
	{
	    screenbox.Left = XSC (sc)->DClip.MinX;
	}
	DOS (printf ("stdscreenwidth: %ld\n", screenbox.Width));
    }
    if (screenbox.Height == STDSCREENHEIGHT)
    {
	/* auto-setting left/top to dclip only applies if STDSCREENdims given */
	screenbox.Height = rectHeight (&XSC (sc)->DClip);
	if (!ns && !FindTagItem (SA_Top, tags))
	{
	    screenbox.Top = XSC (sc)->DClip.MinY;
	}
	DOS (printf ("stdscreenheight: %ld\n", screenbox.Height));
    }

    /* Peter 5-Feb-92:  We need to ensure that the Workbench screen
     * is always 640x200 or bigger.  That must be done here, because
     * the routine in wbench.c that opens Workbench can get STDSCREENWIDTH
     * and STDSCREENHEIGHT, which aren't evaluated until here.
     */
    if (screentype == WBENCHSCREEN)
    {
	screenbox.Width = imax (640, screenbox.Width);
	screenbox.Height = imax (200, screenbox.Height);
    }
    DOS (dumpBox ("post-standard screen box:", &screenbox));

    /* make it so */
    copyBox (&screenbox, (struct IBox *)&sc->LeftEdge);

    /* ---------------------------------------------------------------- */
    /* finish set up of display clip region				*/
    /* ---------------------------------------------------------------- */

    /* default DClip handling: stretch text oscan out
     * to include entire display (i.e., change the default
     * dclip region now that I'm done with the STDSCREEN stuff)
     * Peter 4-Apr-91: we don't let a positive screen.TopEdge cause
     * the bottom of the DClip to be dropped.
     */
    if (!(dclip || stddclip))
    {
	struct Rectangle screenrect;

	/* The bottom of the DClip should not grow if the screen
	 * was born with a screen.TopEdge > 0.
	 * (screenbox is not used from this point on, so it can
	 * be trashed in this manner.)
	 */
	screenbox.Top = imin (screenbox.Top, 0);
	boxToRect (&screenbox, &screenrect);
	DOS (dumpRect ("screen rect", &screenrect));

	rectHull (&XSC (sc)->DClip, &screenrect);
	DOS (dumpRect ("OS: default dclip", &XSC (sc)->DClip));
    }

    /* ---------------------------------------------------------------- */
    /* Attached screens							*/
    /* ---------------------------------------------------------------- */

    /* These fields exist for attached screens.  Family is a list
     * containing nodes as follows:
     * - For a parent screen, the list contains nodes for each child
     *   and the parent itself (in depth order).
     * - For a non-attached screen, the list contains the node for the
     *   screen itself.
     * - For a child-screen, the list is empty (the screen's own node
     *   will be found on its parent's Family list).
     *
     * The actual linkage to children or parent happens inside
     * IOpenScreen(), in order to preserve the integrity of
     * the screen lists.
     */
    NewList ((struct List *) &XSC (sc)->Family);
    AddHead ((struct List *) &XSC (sc)->Family, (struct Node *) &XSC (sc)->ChildNode);

    /* ---------------------------------------------------------------- */
    /*	viewport initialization						*/
    /* ---------------------------------------------------------------- */
    DOS (printf ("viewport initialization\n"));

    /* position and dimensions	*/
    /* ZZZ: all but DWidth (DyOffset?) are set up by RemakeDisplay */
    vp->DWidth = sc->Width;

    /* ---------------------------------------------------------------- */
    /*	set up 'PrivateFlags'						*/
    /* ---------------------------------------------------------------- */

    /* Gather up PrivateFlags based on supplied tags
     * SA_Draggable defaults to TRUE, so we set that in the initial-flags.
     * Note also that setScreenDisplayID() may have already set the
     * PSF_INCOMPATIBLE flag, so we have to preserve any flags already set
     * in PrivateFlags.
     */
    XSC (sc)->PrivateFlags |= screentrap.PrivateFlags;

    /* ---------------------------------------------------------------- */
    /*	set up screen raster						*/
    /* ---------------------------------------------------------------- */

    DOS (printf ("get screen raster\n"));

    /* 'bitmap' pointer was resolved earlier, since we need to know
     * this screens' depth before allocating the ColorMap.
     */
    if (bitmap)
    {
	/* SA_BitMap implies CUSTOMBITMAP */

	longflags |= CUSTOMBITMAP;
    }
    else
    {
	ULONG bmflags = BMF_DISPLAYABLE | BMF_CLEAR;

	DCBM (printf ("Intuition supplies a bitmap of depth %ld, size %ld x %ld\n",
		      depth, sc->Width, sc->Height));

	/* Workbench-like screens get their interleaved-ness
	 * from the IScreenModePrefs.
	 */
	if (GetUserTagData (SA_Interleaved,
			    sm ? TESTFLAG (sm->ism_Control, WBINTERLEAVED) : FALSE,
			    tags))
	{
	    bmflags |= BMF_INTERLEAVED;
	}
	/* Peter 13-Feb-92:  From now on, we allocate a whole BitMap with
	 * planes, and not just the planes.  This is so there's a whole
	 * bitmap available that can be passed back from the
	 * AllocScreenBuffer(sc,SB_SCREEN_BITMAP) call.  Also, looking
	 * forward to strange new modes, we'd like to pick up hypothetically
	 * extended BitMaps.
	 * We use the new graphics.library function that takes care of
	 * allocating and initializing the BitMap and planes, while being
	 * aware of plane-alignment issues that AA brings.
	 *
	 */
	if (!(XSC (sc)->RealBitMap = bitmap = AllocBitMap (sc->Width, sc->Height,
							   depth, bmflags, NULL)))
	{
	    errorcode = OSERR_NOCHIPMEM;
	    goto BADSCREENOPEN;
	}
	DOSF (if (!(--failnum))
	      {
	      printf ("OSFailure: AllocBitMap\n"); goto BADSCREENOPEN;
	      }
	);
    }

    /* The embedded bitmap in the screen is a copy of the bitmap the
     * caller supplied (for CUSTOMBITMAP) or of the bitmap we allocated.
     */
    sc->sc_BitMap = *bitmap;
    XSC (sc)->sc_BitMapCopy = *bitmap;

    /* We'd like to use a pointer to the real bitmap, but when an
     * application supplies a CUSTOMBITMAP, there is no guarantee that
     * the supplied struct BitMap will be kept around.
     * We start by assuming we can use the real bitmap, then override
     * with a pointer to &sc->sc_BitMap for CUSTOMBITMAP screens.
     */
    sc->RastPort.BitMap = bitmap;
    if (TESTFLAG (longflags, CUSTOMBITMAP))
    {
	sc->RastPort.BitMap = &sc->sc_BitMap;
    }

    sc->ViewPort.RasInfo->BitMap = sc->RastPort.BitMap;

    /* ---------------------------------------------------------------- */
    /*	AttachPalExtra							*/
    /* ---------------------------------------------------------------- */

    /* Need to have ViewPort->RasInfo->BitMap in place, and ViewPort
     * modes established before calling AttachPalExtra().
     * The PaletteExtra structure can tell us what the correct LastColor
     * is, even for HAM and EHB screens.
     */
    if (AttachPalExtra (XSC (sc)->ColorMap, vp))
    {
	goto BADSCREENOPEN;
    }
    DOSF (if (!(--failnum))
	  {
	  printf ("OSFailure: AttachPalExtra\n"); goto BADSCREENOPEN;
	  }
    );
    XSC (sc)->LastColor = XSC (sc)->ColorMap->PalExtra->pe_SharableColors;
    DOS (printf ("after AttachPalExtra\n"));

    /* ---------------------------------------------------------------- */
    /*	screen colors							*/
    /* ---------------------------------------------------------------- */
    DOS (printf ("colors time\n"));

    /* Now need to have Screen PaletteExtra set up, since setWBColorsGrunt()
     * now cares about the screen's highest settable color, which came
     * from pe_SharableColors.
     */
    /* need this be after VP linked in?  (can I move this back up?)*/
    /* initialize colors if user has specified a colorspec	*/
    /* ZZZ:
     *    do I want to actually SetRGB4? (no vp copper lists yet)
     *	  could a LoadRGB4() work out here?
     */

    DOS (printf ("call setWBCG, for screen %lx\n", sc));

    /* provide for a way that screens can inherit *ALL* the default colors.
     * Workbench-like screens do this.
     */
    setWBColorsGrunt (sc, GetUserTagData (SA_FullPalette, (ULONG)sm, tags));

    DOS (printf ("setWBCG returned\n"));

    /* Extra brace to make local variables */
    {
	struct ColorSpec *cspec;
	WORD cindex;

	if (cspec = (struct ColorSpec *) GetUserTagData0 (SA_Colors, tags))
	{
	    DOS (printf ("colorspec at %lx\n", cspec));

	    while ((cindex = cspec->ColorIndex) != -1)
	    {
		SetRGB4 (vp, cindex, cspec->Red, cspec->Green, cspec->Blue);
		++cspec;
	    }
	}
    }

    /* LoadRGB32() takes a table.  We allow screen openers to
     * pass that table right to us.  Note that LoadRGB32( vp, NULL )
     * is a safe NOP.
     */
    LoadRGB32 (vp, (ULONG *) GetUserTagData0 (SA_Colors32, tags));

    /* ---------------------------------------------------------------- */
    /*	set up screen font						*/
    /* ---------------------------------------------------------------- */
    DOS (printf ("screen font\n"));

    {
	struct TextAttr *font;

	/* if his font won't open, use one of the system fonts,
	 * assume it will open
	 */

	font = (struct TextAttr *) GetUserTagData (SA_Font, (ULONG)NS (Font), tags);

	if (font && (open_font = ISetFont (&sc->RastPort, font)))
	{
	    /* use his font	*/
	    DOS (printf ("use his font: %lx\n", font));
	    sc->Font = font;
	}
	else
	{
	    struct TAttrBuff *tbuff;

	    /* use a sysfont (he must specify a legal one) */
	    sysfont = 0;
	    /* Workbench-like screens use sysfont=1 */
	    if (sm)
	    {
		sysfont = 1;
	    }
	    sysfont = GetUserTagData (SA_SysFont, sysfont, tags);

	    DOS (printf ("use a system font: %ld\n", sysfont));

	    /* jimm: 2/12/90: change to font inheritance: if
	     * using a special SYSFONT, window rastports will
	     * inherit the vanilla system font
	     */
	    if (sysfont > 0)
	    {
		SETFLAG (XSC (sc)->PrivateFlags, PSF_SCREENFONT);
	    }

	    /* Peter 9-Jan-91:  We store the Workbench screen's TextAttr
	     * in a place that doesn't go away when the wbscreen closes.
	     * This fixes MicroFiche Filer, which references (not copies)
	     * wbscreen->Font, then calls CloseWorkBench().
	     */
	    tbuff = &XSC (sc)->TAttrBuff;
	    if (screentype == WBENCHSCREEN)
	    {
		/* Use permanent storage instead */
		tbuff = &IBase->MFF_TAttrBuff;
	    }

	    copyTAttr (&IBase->SysFontPrefs[sysfont], tbuff);
	    sc->Font = &tbuff->tab_TAttr;
	    DOS (printf ("my font at:%lx, scFont %lx\n",
			 &IBase->SysFontPrefs[sysfont], sc->Font));
	    open_font = ISetFont (&sc->RastPort, sc->Font);
	    /* had better work */
	}
	/* Mark that we've set the font in the screen's RastPort.
	 * closeScreenCommon() must avoid CloseFont()ing the RP.Font
	 * unless we were the opener.
	 */
	SETFLAG (XSC (sc)->PrivateFlags, PSF_SETFONT);
    }

    /* ---------------------------------------------------------------- */
    /*	initialize misc. Intuition constructs				*/
    /* ---------------------------------------------------------------- */
    DOS (printf ("misc. init\n"));

    /* nscopy is initialized to have DetailPen=0, BlockPen=1 */
    sc->DetailPen = GetUserTagData (SA_DetailPen, nscopy.DetailPen, tags);
    sc->BlockPen = GetUserTagData (SA_BlockPen, nscopy.BlockPen, tags);

    sc->DefaultTitle = (UBYTE *) GetUserTagData (SA_Title, (ULONG)NS (DefaultTitle), tags);
    DST (printf ("title from tags/newscreen <%s>.\n", sc->DefaultTitle));

    if ((!sc->DefaultTitle) && pubnode)
    {
	DST (printf ("use pubnode name for title\n"));
	sc->DefaultTitle = pubnode->psn_Node.ln_Name;
    }
    sc->Title = sc->DefaultTitle;

    /* Flags might already have SCREENHIRES set in them	*/
    sc->Flags |= longflags;
    DOS (printf ("setting sc->Flags %lx\n", sc->Flags));

    /* ---------------------------------------------------------------- */
    /* initialize DrawInfo						*/
    /* ---------------------------------------------------------------- */
    XSC (sc)->SDrawInfo.dri_Version = DRI_VERSION;
    XSC (sc)->SDrawInfo.dri_NumPens = NUMDRIPENS;
    XSC (sc)->SDrawInfo.dri_Font = open_font;
    XSC (sc)->SDrawInfo.dri_Depth = depth;
    XSC (sc)->SDrawInfo.dri_Resolution.X = XSC (sc)->ScaleFactor.X;
    XSC (sc)->SDrawInfo.dri_Resolution.Y = XSC (sc)->ScaleFactor.Y;

    {
	UWORD *custompens, *scpens, *spens;
	UBYTE dpen, bpen;

	spens = XSC (sc)->SDrawInfo.dri_Pens = XSC (sc)->SPens;

	LOCKIBASE ();
	/* init pen array from one of two equal size hardcoded tables 	*/
	DRI (printf ("render info pens size %ld %ld\n",
		     sizeof twoColorPens, sizeof XSC (sc)->SPens));

	/* Typical applications begin with the fourColorPens array.
	 * Workbench-like applications actually use the user's
	 * preferred pens.  Monochrome applications begin with the
	 * twoColorPens array.
	 * A little farther on, if neither SA_Pens or SA_LikeWorkbench
	 * is specified, we clobber the pens with the monochrome
	 * new-look.
	 */

	scpens = fourColorPens;
	if (sm)
	{
	    scpens = IBase->ScreenPens4;
	    if (depth > 2)
	    {
		scpens = IBase->ScreenPens8;
	    }
	}
	if (depth == 1)
	{
	    scpens = twoColorPens;
	}

	/* Copy the selected initial pens into the Screen's
	 * DrawInfo dri_Pens:
	 */
	CopyMem (scpens, spens, sizeof (UWORD) * (NUMDRIPENS + 1));
	UNLOCKIBASE ();

	/* override penspecs with (partial) list passed in SA_Pens	*/

	/* copies of Screen values	*/
	dpen = spens[DETAILPEN] = sc->DetailPen;
	bpen = spens[BLOCKPEN] = sc->BlockPen;

	/* If he does not specify SA_Pens, or he is not Workbench-like,
	 * he does not get full treatment:
	 */
	if ((custompens = (UWORD *) GetUserTagData0 (SA_Pens, tags)) ||
	    (sm))
	{
	    /* Mask pens down to the number of planes we really
	     * have.  We base this XSC(sc)->LastColor, which is correct
	     * correctly for HAM, EHB, etc.
	     */

	    LONG numpens = 0;

	    XSC (sc)->SDrawInfo.dri_Flags |= DRIF_NEWLOOK;

	    /* Copy pens, if any, until ~0 is encountered in source
	     * or destination.
	     */
	    if (custompens)
	    {
		scpens = spens;

		while (*custompens != ~0 && *scpens != ~0)
		{
		    numpens++;
		    *scpens++ = *custompens++;
		}
		/* If the three "new for V39" pens are not specified,,
		 * they get V37-compatible defaults.
		 * Exception: if you pass {~0} for your pens (i.e.
		 * all you want are defaults), then you get the V39
		 * defaults.
		 */
		if ((numpens != 0) && (numpens <= BARDETAILPEN))
		{
		    spens[BARDETAILPEN] = dpen;
		    spens[BARBLOCKPEN] =
			spens[BARTRIMPEN] = bpen;
		}
	    }
	    /* Now resolve the special constants that map to
	     * pens ~0 to ~3
	     */
	    scpens = spens;
	    while (*scpens != ~0)
	    {
		*scpens &= XSC (sc)->LastColor;
		scpens++;
	    }
	}
	/* otherwise, we rely on his (initial, screen) Detail/BlockPens	*/
	else
	{
	    LONG i;

	    /* if detail and block are the same, we
	     * use 0 and 1 (the defaults)
	     */
	    if (dpen == bpen)
	    {
		dpen = 0;
		bpen = 1;
	    }

	    /* Transcript has blockpen = 0, which makes all window borders
	     * invisible.  If block = 0, we switch detail and block
             */
	    if (bpen == 0)
	    {
		bpen = dpen;
		dpen = 0;
	    }

	    /* Most pens are initialized to the BlockPen value,
	     * except for BARDETAILPEN and FILLTEXTPEN, which are
	     * initialized to the DetailPen value, and BACKGROUNDPEN,
	     * which is initialized to zero.
	     */
	    for (i = TEXTPEN; i < NUMDRIPENS; i++)
	    {
		spens[i] = bpen;
	    }
	    spens[BARDETAILPEN] =
		spens[FILLTEXTPEN] = dpen;
	    spens[BACKGROUNDPEN] = 0;
	}
    }

    /* ---------------------------------------------------------------- */
    /*	Default checkmark and Amiga-key imagery				*/
    /* ---------------------------------------------------------------- */

    if (!(XSC (sc)->SDrawInfo.dri_CheckMark = (struct Image *) NewObject (NULL, SysIClassName,
							 SYSIA_DrawInfo, &XSC (sc)->SDrawInfo,
							 SYSIA_Which, MENUCHECK,
							 TAG_DONE)))
    {
	goto BADSCREENOPEN;
    }

    if (!(XSC (sc)->SDrawInfo.dri_AmigaKey = (struct Image *) NewObject (NULL, SysIClassName,
							SYSIA_DrawInfo, &XSC (sc)->SDrawInfo,
							SYSIA_Which, AMIGAKEY,
							TAG_DONE)))
    {
	goto BADSCREENOPEN;
    }
    DOSF (if (!(--failnum))
	  {
	  printf ("OSFailure: AmigaKey/CheckMark\n"); goto BADSCREENOPEN;
	  }
    );

    DRI (dumpDrawInfo ("screen drawinfo", &XSC (sc)->SDrawInfo));

    /* ---------------------------------------------------------------- */
    /*	Graphics pen-sharing (ObtainPen())				*/
    /* ---------------------------------------------------------------- */

    /* Brace for local variables */
    {
	LONG pen, p;
	LONG allocate, isDRIpen;
	ULONG mode;

	/* The rule for allocating colors is as follows:
	 * Any pen that's in the DrawInfo pens must be sharable, since
	 * SA_Pens is all about sharing.  Pens 17-19 are the sprite pens,
	 * and are also made sharable.
	 *
	 * For unaware public and custom, all other pens are allocated as
	 * exclusive, because the screen could be a paint program,
	 * or any program that feels free to muck with those colors.
	 *
	 * For aware public and custom screens (SA_SharePens,TRUE),
	 * all other pens are left unallocated. (The pubscreen owner can
	 * allocate some before making the screen public).
	 *
	 * The Workbench screen and other Workbench-like screens are
	 * considered "aware" (SA_SharePens, TRUE).  Additionally,
	 * on Workbench-like screens, we allocate pens ~0 and ~1, so
	 * that complementing of string gadget and console cursors tends
	 * to work.
	 *
	 * On the true Workbench screen, we also allocate colors 0 to 3
	 * and ~0 to ~3 as sharable.  This is a compromise because we want
	 * the Workbench to be as aware as possible, but some
	 * palette-sharing-unaware programs will be opening on the WB.
	 * For them, we tie down 0 to 3 (which are common rendering colors),
	 * and ~0 to ~3, (which are common complement colors).
	 */

	for (p = 0; p <= XSC (sc)->LastColor; p++)
	{
	    /* By default, we'll allocate this pen, and in shared mode.
	     * (shared is the default.)
	     */
	    allocate = TRUE;
	    mode = PENF_NO_SETCOLOR;

	    /* We need to know if this pen is in the DrawInfo pens */
	    isDRIpen = FALSE;
	    for (pen = 0; pen < NUMDRIPENS; pen++)
	    {
		if (p == XSC (sc)->SPens[pen])
		{
		    isDRIpen = TRUE;
		    break;
		}
	    }

	    if ((isDRIpen) || ((p >= 17) && (p <= 19)))
	    {
		/* Pens in the pen-array are intended to be shared, so
		 * that's how we'll allocate them.  Likewise for the
		 * sprite pens.  We need do nothing here.
		 */
		DPEN (printf ("Pen %ld in SA_Pens or is a sprite pen.  Will be shared.\n", p));
	    }
	    else if (!TESTFLAG (sc->Flags, PENSHARED))
	    {
		/* Screens that do not request {SA_SharePens,TRUE}
		 * must have all other pens allocated as exclusive,
		 * since it could be a paint-program or something
		 * that owns the screen.
		 */
		mode = PENF_NO_SETCOLOR | PENF_EXCLUSIVE;
		DPEN (printf ("Pen %ld on unaware screen.  Will be exclusive.\n", p));
	    }
	    else if ((screentype == WBENCHSCREEN) &&
		     ((p < 4) || (p >= XSC (sc)->LastColor - 3)))
	    {
		/* On the Workbench screen, pens 0 to 3 and ~0 to ~3 are
		 * allocated as shared.  We need do nothing here.
		 */
		DPEN (printf ("Pen %ld on Workbench screen.  Will be shared.\n", p));
	    }
	    else if ((sm) && (p >= XSC (sc)->LastColor - 1))
	    {
		/* On LikeWorkbench screens, pens ~0 and ~1 are
		 * allocated as shared.  We need do nothing here.
		 */
		DPEN (printf ("Pen %ld on WorkbenchLike screen.  Will be shared.\n", p));
	    }
	    else
	    {
		/* This is a custom screen, or else an aware pubscreen
		 * (i.e. one that specifies {SA_SharePens,TRUE}).
		 * Don't allocate other pens.
		 */
		allocate = FALSE;
		DPEN (printf ("Pen %ld on custom or aware pubscreen.  Will be free.\n", p));
	    }

	    if (allocate)
	    {
		/* PENF_NO_SETCOLOR means ignore the RGB values
		 * supplied here
		 */
		ObtainPen (XSC (sc)->ColorMap, p, 0, 0, 0, mode);
		DPEN (printf ("Pen %ld obtained as mode %ld.\n", p, mode));
	    }
	}
    }

    /* ---------------------------------------------------------------- */
    /* initialize border sizes and gadgets (this is too rez-inflexible)	*/
    /* ---------------------------------------------------------------- */
    DOS (printf ("border sizes\n"));

    {
	CopyMem (loresBorders, &sc->BarVBorder, 8);

	/* patch up two discrepancies	*/
	if (IsHires (sc))
	{
	    sc->BarHBorder = 5;
	    sc->MenuVBorder = 2;
	}

	/* jimm: 1/16/90: allow short little screens */
	/* note: 4/8/90: BarHeight is one less than layer height */
	sc->BarHeight =
	    imin (sc->BarVBorder * 2 + sc->Font->ta_YSize, sc->Height - 1);
    }

    DSG (printf ("OS: about to get system gadgets\n"));

    if (!screenGadgets (sc))
    {
	DSG (printf ("OS: failed to get system gadgets\n"));
	goto BADSCREENOPEN;
    }
    DOSF (if (!(--failnum))
	  {
	  printf ("OSFailure: screenGadgets\n"); goto BADSCREENOPEN;
	  }
    );
    DSG (printf ("OS: got system gadgets\n"));
#if 0
    /** SHRINK: WHAT USER GADGETS???!!! **/
    /* link user screen gadgets at end of sytem list */
    ((struct Gadget *) lastThing (&sc->FirstGadget))->NextGadget =
	ns->Gadgets;
#endif

    /* ---------------------------------------------------------------- */
    /*	initialize layers structures for this screen, incl. BarLayer	*/
    /* ---------------------------------------------------------------- */
    {
	struct Hook *screenbackfill;

	DOS (printf ("screen layers init\n"));

	InitLayers (&sc->LayerInfo);
	if (!FattenLayerInfo (&sc->LayerInfo))
	{
	    goto BADSCREENOPEN;
	}
	DOSF (if (!(--failnum))
	      {
	      printf ("OSFailure: FattenLayerInfo\n"); goto BADSCREENOPEN;
	      }
	);

	/* New for V39:  SA_BackFill for Screen's LayerInfo backfill.
	 * Defaults to NULL, which is the good ol' layers behavior of old.
	 */
	screenbackfill = (struct Hook *) GetUserTagData0 (SA_BackFill, tags);
	InstallLayerInfoHook (&sc->LayerInfo, screenbackfill);

	/*  When we create the layer, we use a stub-Hook so that the bitmap
	    doesn't get cleared, in case somebody has supplied a custom bitmap
	    with data in it already */
	if ((XSC (sc)->ClipLayer_Info = NewLayerInfo ()) &&
	    (XSC (sc)->ClipLayer = CreateUpfrontHookLayer (XSC (sc)->ClipLayer_Info,
							   sc->RastPort.BitMap,
							   0, 0,
							   sc->Width - 1, sc->Height - 1,
							   LAYERSIMPLE | LAYERBACKDROP,
							   LAYERS_NOBACKFILL,
							   NULL)))
	{
	    /* Specify a hook that does no backfill, so that deleting the
	     * menu clip-layer doesn't blank the bitmap.  Under V37 and early
	     * V39, we set the RastPort mask to zero to have the same effect.
	     */
	    InstallLayerInfoHook (XSC (sc)->ClipLayer_Info, LAYERS_NOBACKFILL);
	}
	else
	{
	    goto BADSCREENOPEN;
	}
	DOSF (if (!(--failnum))
	      {
	      printf ("OSFailure: ClipLayerInfo/ClipLayer\n"); goto BADSCREENOPEN;
	      }
	);

	sc->BarLayer = (struct Layer *) CreateUpfrontHookLayer (
								   &sc->LayerInfo,
								   sc->RastPort.BitMap, 0, 0, sc->Width - 1, sc->BarHeight,
								   LAYERSIMPLE | LAYERBACKDROP,
								   LAYERS_NOBACKFILL,
								   NULL);
	if (sc->BarLayer == NULL)
	    goto BADSCREENOPEN;
	DOSF (if (!(--failnum))
	      {
	      printf ("OSFailure: BarLayer\n"); goto BADSCREENOPEN;
	      }
	);

	/* This one merits a little explanation.  Under V37, the screen
	 * BarLayer had a do-nothing backfill hook.  This meant that
	 * creating the BarLayer did not erase the underlying data
	 * (which might have been placed in a CustomBitMap).  However,
	 * deleting a layer which obscured the BarLayer nevertheless
	 * caused the BarLayer area to be cleared, because layers would
	 * clear the area under a deleted layer as well as invoking
	 * backfill on each revealed area.
	 *
	 * V39 fixed this layers "double-clear" bug, causing an Intuition
	 * bug, namely that closing a window didn't erase the part of
	 * the window which obscured the title bar area.  We correct
	 * this by immediately setting the BarLayer hook of a SCREENQUIET
	 * screen to the same hook as the screen's Layer_Info backfill hook.
	 */
	if (TESTFLAG (sc->Flags, SCREENQUIET))
	{
	    InstallLayerHook (sc->BarLayer, screenbackfill);
	}

	ISetFont (sc->BarLayer->rp, sc->Font);
	sc->BarLayer->Window = NULL;

	/* render into the BarLayer */
	DSG (printf ("OS: render screenbar\n"));
	screenbar (sc);
	DSG (printf ("OS: render done\n"));
    }

    /* ---------------------------------------------------------------- */
    /* link into lists, turn 'er on and let 'er rip		*/
    /* ---------------------------------------------------------------- */
    DOS (printf ("OS: call state machine\n"));

    /* get in sync with state machine to link this screen
     * into the screen list.
     * also, set up the coercion stuff in there, so that no screen
     * without consistent information is ever in the list
     * when things are running freely
     */

    if (errorcode = doISM (itOPENSCREEN, (CPTR)sc, (CPTR)tags, NULL))
    {
	DOS (printf ("OS: doISM returned failure!!\n"));
	goto BADSCREENOPEN;
    }
#if 0
    /* Can't put a failure test here since IOpenScreen() is assumed to
     * clean up after itself when it really fails.  This fake failure
     * hardly takes care of that...
     */
    DOSF (if (!(--failnum))
	  {
	  printf ("OSFailure: itOPENSCREEN\n"); goto BADSCREENOPEN;
	  }
    );
#endif

    DOS (printf ("OS:proceed with other linkages\n"));

    /* Other linkages: not state machine dependent	*/
    LOCKIBASE ();

    /* link in public screen node */
    if (pubnode)
    {
	/* If OpenScreen is extended in a way that it can fail after
	 * the pubnode is linked in, then the jFreeMem() needs to
	 * be preceded by a Remove().
	 */
	linkPubScreen (sc, pubnode);
    }

    UNLOCKIBASE ();

    /*** Turn it on	***/
    DOS (printf ("OST: call RemakeDisplay\n"));
    RemakeDisplay ();
    DOS (printf ("OST: RemDis returned\n"));

    /* move it up above	setWBColorsGrunt(sc); */

    goto OUT;			/* success	*/

  BADSCREENOPEN:
    DOS (printf ("badscreenopen\n"));

    /* Peter 13-Aug-90: use closeScreenCommon to save space: */
    closeScreenCommon (sc);
    sc = NULL;

    if (pubnode)
    {
	/* Only good enough since OpenScreenTagList() can't fail
	 * after the point where the pubnode is linked in to the
	 * real list.  At this point of failure, the node is on
	 * the pending screens list, so we must be sure to remove it.
	 */
	Remove (pubnode);
	FreeVec (pubnode);
    }

    {
	LONG *errptr;

	DOS (printf ("bailing out:  errorcode %ld\n", errorcode));

	if (errptr = (ULONG *) GetUserTagData0 (SA_ErrorCode, tags))
	{
	    *errptr = errorcode;
	}
    }

  OUT:
    DOS (printf ("returning sc:%lx\n --\n", sc));
    DST (kpause ("end of OpenScreenTagList."));

    DOS (printf ("OPENSCREEN DONE, tags: %lx\n", tags));

    return (sc);
}

/*****************************************************************************/

/* ISM portion of OpenScreen()
 *
 * Perhaps this is best thought of as being named ILinkScreen
 *
 * also coerces screen to monitor of the screen current list,
 * arbitrated with other screen list users
 *
 * Attaching screens can fail if SA_Parent points to a screen
 * that is itself a child, or if SA_Front|BackChild points to
 * a screen that is itself a parent.
 *
 * Returns zero for success, non-zero for failure.
 *
 * IMPORTANT:  The caller of this routine currently assumes that
 * this routine cleans up everything it does!
 */
LONG IOpenScreen (struct Screen *sc, struct TagItem *taglist)
{
    struct IntuitionBase *IBase = fetchIBase ();
    struct Screen *parent, *child;
    LONG whichway;
    struct MinNode *node, *succ;
    static struct Point no_point = {0, 0};
    LONG error = 0;

    /* This screen may be a child screen (if its parent is specified
     * with SA_Parent) or a parent screen (if its children are specified
     * with SA_FrontChild or SA_BackChild), but not both.  SA_Parent
     * has priority...
     */
    if (parent = (struct Screen *) GetUserTagData0 (SA_Parent, taglist))
    {
	/* This screen is intended to be the child of some other screen */
	error = attachScreen (sc, parent);
    }
    else
    {
	/* This screen may be the parent of one or more screens.
	 * For _each_ such screen (specified through the SA_FrontChild
	 * and SA_BackChild tags), attach the screen and note if
	 * the programmer wanted it to come to front or to back.
	 */
	struct TagItem *tstate = taglist;
	struct TagItem *tag;

	while ((!error) && (tag = NextTagItem (&tstate)))
	{
	    /* Assuming a child... */
	    if (child = (struct Screen *) tag->ti_Data)
	    {
		if (tag->ti_Tag == SA_FrontChild)
		{
		    SETFLAG (XSC (child)->PrivateFlags, PSF_CHILDFRONT);
		    error = attachScreen (child, sc);
		}
		else if (tag->ti_Tag == SA_BackChild)
		{
		    CLEARFLAG (XSC (child)->PrivateFlags, PSF_CHILDFRONT);
		    error = attachScreen (child, sc);
		}
	    }
	}
    }

    if (!error)
    {
	struct ViewPort *vp = &sc->ViewPort;

	/* When SDEPTH_INITIAL is set, the SDEPTH_INFAMILY flag has no
	 * bearing if this screen is a parent.  If it is a child, it
	 * indicates we want to be placed at the front/back of the family,
	 * not the whole list.
	 */
	whichway = SDEPTH_TOFRONT | SDEPTH_INITIAL | SDEPTH_CHILDONLY;

	if (TESTFLAG (sc->Flags, SCREENBEHIND))
	{
	    whichway = SDEPTH_TOBACK | SDEPTH_INITIAL | SDEPTH_CHILDONLY;
	}
	/* relinkScreen() takes care of linking the new screen into
	 * the correct place.
	 */
	LOCKIBASE ();
	LOCKVIEW ();
	relinkScreen (sc, whichway);

	/* coerce to proper mode */
	DOS (printf ("OST: call coerceScreens\n"));
	/* Note that coerceScreens() will ensure that the screen's
	 * position is legal, which means that if the new screen is
	 * the child of some screen, then child's TopEdge is made
	 * to not exceed its parent's, so we have no further work
	 * in that regard.
	 */
	coerceScreens (sc, FORCE);

	/* jimm: 2/28/90: need to init vp offsets here, because
	 * they aren't sync'd by RemakeDisplay
	 * Peter 22-Mar-92:  Moved this to after the doISM()
	 * because inside doISM() coerceScreens() gets called,
	 * which ensures that the screen LeftEdge/TopEdge has
	 * a legal position.  We need for that to go into
	 * the VPOffsets and the stash.
	 *
	 * Peter 16-Aug-92:  Moved to inside doISM() because we
	 * need to link the screen and the viewport under the
	 * same lock-set.  This is because another task might
	 * be calling RethinkDisplay().  That function evaluates
	 * which ViewPorts are hidden based on the screen
	 * list.  However, MrgCop() works based on the View ViewPort
	 * list.  If RethinkDisplay() occured after the screen was
	 * linked but before the viewport was linked, then the
	 * screen might obscure all other screens without itself
	 * having a ViewPort.  This will cause MrgCop() to fail and
	 * result in LoadView(NULL) even though screens are present.
	 */
	XSC (sc)->VPOffsetStash.X = vp->DxOffset = sc->LeftEdge;
	XSC (sc)->VPOffsetStash.Y = vp->DyOffset = sc->TopEdge;

	/* installVPort(), inline  */
	vp->Next = IBase->ViewLord.ViewPort;
	IBase->ViewLord.ViewPort = vp;

	UNLOCKVIEW ();
	UNLOCKIBASE ();

	/* screenMouse(sc); * done in coerceScreens() */

	/* If the new screen has children, then we must ensure that
	 * each child-screen is adjacent (in depth) to a family member,
	 * and that each child's top edge doesn't exceed the parent's.
	 */

	for (node = XSC (sc)->Family.mlh_Head;
	     succ = node->mln_Succ;
	     node = succ)
	{
	    /* One of the screens on the Family list is the
	     * screen itself, and we don't want to act on it.
	     */
	    if ((child = SCREENFROMNODE (node)) != sc)
	    {
		whichway = SDEPTH_TOBACK | SDEPTH_CHILDONLY;

		if (TESTFLAG (XSC (child)->PrivateFlags, PSF_CHILDFRONT))
		{
		    whichway = SDEPTH_TOFRONT | SDEPTH_CHILDONLY;
		}
		relinkScreen (child, whichway);

		/* Ensure that the child's TopEdge doesn't exceed
		 * its parent's:
		 */
		IMoveScreen (child, &no_point, SPOS_REVALIDATE | SPOS_NORETHINK);
	    }
	}
    }
    else
    {
	/* Clean up EVERYTHING we did inside this function, which is
	 * just the attachment.
	 */
	detachScreen (sc);
    }
    return (error);
}

/*****************************************************************************/

/* returns non-zero if OK */
ULONG displayOScan (CPTR drecord, ULONG displayID, LONG ostype, struct Rectangle *rect)
{
    struct Rectangle *doscan;
    struct DimensionInfo diminfo;

    DSCAN (printf ("dOScan: type: %lx\n", ostype));

    if (GetDisplayInfoData ((DisplayInfoHandle)drecord, (UBYTE *)&diminfo, sizeof (diminfo), DTAG_DIMS, displayID))
    {
	switch (ostype)
	{
	    case OSCAN_STANDARD:
		doscan = &diminfo.StdOScan;
		break;
	    case OSCAN_MAX:
		doscan = &diminfo.MaxOScan;
		break;
	    case OSCAN_VIDEO:
		doscan = &diminfo.VideoOScan;
		break;
#if 0
	    case OSCAN_TEXT:
#endif
	    default:
		doscan = &diminfo.TxtOScan;
		break;
	}

	*rect = *doscan;
	DSCAN (dumpRect ("dOS returning oscan", rect));

	return (1);
    }

    DSCAN (printf ("dOScan failing: ID: %08lx\n", displayID));
    return (0);
}

/*****************************************************************************/

/*** intuition.library/CloseScreen ***/

BOOL CloseScreen (struct Screen *OScreen)
{
    DCS (printf ("---\nCloseScreen entry, sc = %lx\n", OScreen));

    if (!OScreen)
	return (1);

    if (doISM (itCLOSESCREEN, (CPTR)OScreen, NULL, NULL))
    {
	if (fetchIBase ()->FirstScreen == NULL)
	{
	    DCS (printf ("all screens gone, openwb true\n"));
	    OpenWorkBench ();	/* gives Dave a poke	*/
	}
	return (TRUE);
    }
    return (FALSE);
}

/*****************************************************************************/

/* "aloha" in hawaiian means both hello and goodbye */
void AlohaWorkbench (struct MsgPort *wbport)
{
    DCWB (printf ("AlohaWorkBench: port %lx\n", wbport));
    fetchIBase ()->WBPort = wbport;
}

/*****************************************************************************/

LONG CloseWorkBench (void)
{
    struct IntuiMessage imsg;
    struct MsgPort *port;
    LONG retval;

    DCWB (printf ("===\nCloseWorkBench enter, task %lx\n", FindTask (NULL)));

    /* create me a private port	*/
    if (!(port = CreateMsgPort ()))
	return (FALSE);

    DCWB (printf ("CWB: reply port at %lx\n", port));

    initWBMessage (&imsg, port, WBENCHCLOSE);

    /* processing the itCLOSEWB token will check to see if there
     * is a WB screen, and if there are no application windows
     * on it.  If OK, it sends WB the message, returns
     * TRUE, and you wait for the message to come back
     * from WB before proceeding
     */

    DCWB (printf ("CWB doISM itCLOSEWB, imsg = %lx\n", &imsg));
    retval = doISM (itCLOSEWB, (CPTR)&imsg, NULL, NULL);
    DCWB (printf ("CWB: doISM for itCLOSEWB returned %ld\n", retval));

    if (retval)			/* doISM returns 0 if OK to try closescreen */
    {
	DCWB (printf ("CWB: doISM failed\n"));
	/* failure: the message wasn't sent */
	DeleteMsgPort (port);
	return (FALSE);
    }

    /* wait for message reply */
    DCWB (printf ("CWB: doISM succeeded, wait for reply on port %lx\n", port));
    WaitPort (port);
    DCWB (printf ("CWB: reply from dave received\n"));
    DeleteMsgPort (port);

    /* now do the actual screen close, which might fail
     * if there are windows open after all this is done.
     */

    DCWB (printf ("CWB: doISM itCLOSESCREEN task %lx\n", FindTask (0L)));
    if (!(retval = doISM (itCLOSESCREEN, NULL, NULL, NULL)))
    {
	/* close screen failed for some unknown reason,
	 * probably somebody snuck a window in there.
	 * Resummon the WB.
	 */
	DCWB (printf ("screen would not close\n"));
	pokeWorkbench ();	/* give dave a poke */
    }
    return (retval);
}

/*****************************************************************************/

/* initializes message for CloseWB or OpenWB.
 * Note that mouse, timestamp, and qualifiers are garbage.
 * The IDCMPWindow and IAddress fields are both NULL */
void initWBMessage (struct IntuiMessage *imsg, struct MsgPort *replyport, UWORD code)
{
    imsg->ExecMessage.mn_ReplyPort = replyport;
    imsg->Class = WBENCHMESSAGE;
    imsg->Code = code;
    imsg->IAddress = imsg->IDCMPWindow = NULL;
}

/*****************************************************************************/

/* if OScreen == NULL, means I'm to close Workbench */
BOOL ICloseScreen (struct Screen *OScreen)
{
    struct IntuitionBase *IBase = fetchIBase ();

    DCS (printf ("ICloseScreen: screen %lx task %lx\n", OScreen, FindTask (0L)));

    /* by now, CloseWorkBench() has sent WB the news, and
     * given him a chance to clean up */

    /* WB  is already gone */
    if (!OScreen && !(OScreen = findWorkBench ()))
    {
	DCWB (printf ("ICS: wbscreen already gone\n"));
	/* 4/30/90: jeez, has this been incompatible for very long? */
	return (FALSE);
    }

    DCS (printf ("ICS: screen to close %lx\n", OScreen));

    /* The calls to freePubScreenNode() and delinkScreen() MUST
     * happen inside the same LOCKIBASE session.  freePubScreenNode()
     * will dispose the PSNode of the Workbench screen if there
     * are no visitors.  Later, before we delink the screen, someone
     * might trigger openSysScreen(WBENCHSCREEN) (e.g. via
     * LockPubScreen(NULL)), which finds the screen by walking
     * the screen list.  Holding the IBASELOCK around all this
     * is the protection we need.
     */

    LOCKIBASE ();
    /*
     * notice that the fPSN is tested last, so that it can
     * commit to freeing the stuff and know that this screen
     * is definitely going bye-bye.
     */
    if (OScreen->FirstWindow || !freePubScreenNode (XSC (OScreen)->PSNode))
    {
	DCS (printf ("ICS: window (%lx) open or visitor count failed.\n",
		     OScreen->FirstWindow));
	UNLOCKIBASE ();
	return (0);
    }

    /* Detach any children from this screen, and detach this screen
     * from any parent.
     */
    detachScreen (OScreen);

    /* delink ViewPort and Screen.
     * These must occur under the same lock-set because another task
     * might be calling RethinkDisplay().  That function evaluates
     * which ViewPorts are hidden based on the screen list.  However,
     * MrgCop() works based on the View ViewPort list.  If RethinkDisplay()
     * occured after the ViewPort was unlinked but while the screen
     * was still linked, then the screen might obscure all other screens
     * without itself having a ViewPort.  This will cause MrgCop() to fail
     * and result in LoadView(NULL) even though screens are present.
     */

    LOCKVIEW ();

    delinkThing ((struct Thing *)&OScreen->ViewPort, (struct Thing *)&IBase->ViewLord.ViewPort);
    delinkScreen (OScreen);

    UNLOCKVIEW ();
    UNLOCKIBASE ();

    /* jimm: 3/22/90: do this right after delinking, before
     * freeing bitplanes
     */
    RemakeDisplay ();
    freeMouse ();		/* jimm: PAL: */

    FreeVPortCopLists (&OScreen->ViewPort);

    closeScreenCommon (OScreen);

    DCS (printf ("ICloseScreen: think about workbench reopen\n"));

    if (IBase->FirstScreen == NULL || OScreen == IBase->ActiveScreen)
    {
	IBase->ActiveScreen = NULL;	/* probably some old bandaid */
    }

    DCS (printf ("ICloseScreen done\n"));

    return (1);
}

/*****************************************************************************/

/* returns screen with VisitorCount++ */
struct Screen *openSysScreen (UWORD type)
{
    struct Screen *workscreen;
    struct IntuitionBase *IBase = fetchIBase ();

    DOSS (printf ("openSysScreen type: %lx\n", type));

    /*** jimm: 3/23/90:
     * I'm not too happy about this, but a semaphore is too
     * dangerous, since OpenScreen gets semaphores, calls
     * doISM(), and so on.  This works.
     *
     * Look, beats the hell out of busyWaitForbid() from
     * the Old Days.
     *
     * This protection is to keep more than one caller from opening
     * a workbench screen.
     */
    for (;;)
    {
	Forbid ();
	if (IBase->SysScreenProtect == 0)
	{
	    IBase->SysScreenProtect++;	/* I am in 	*/
	    Permit ();
	    break;
	}
	Permit ();
	WaitTOF ();		/* ick	*/
    }

    /* only one caller at a time can get here	*/

    LOCKIBASE ();		/* protect my march down the list	*/
    /* also keeps people from taking screen private */

    workscreen = IBase->FirstScreen;
    while (workscreen)
    {
	DOSS (printf ("check screen %lx flags %lx against %lx\n", workscreen,
		      workscreen->Flags & SCREENTYPE, type));
	if ((workscreen->Flags & SCREENTYPE) == type)
	{
	    bumpPSNVisitor (XSC (workscreen)->PSNode);
	    UNLOCKIBASE ();
	    goto out;
	}

	workscreen = workscreen->NextScreen;
    }
    UNLOCKIBASE ();

    DOSS (printf ("oSS couldn't find, call owb\n"));

    if (IBase->Flags & VIRGINDISPLAY)
    {
	IBase->LongMouse.LX = 0;
	IBase->LongMouse.LY = 0;
	IBase->Flags &= ~VIRGINDISPLAY;
    }

    /* switch replaced by if-else: 2/4/86 */
    if (type == WBENCHSCREEN)
    {
	/* Can't have IBASELOCK when calling this.
	 */
	workscreen = openwbscreen ();	/* returns with VCount++ */
	goto out;
    }
    else
    {
	DOSS (printf ("type again %lx\n", type));
	DOSS (Alert (AN_SysScrnType));
    }

  out:
    IBase->SysScreenProtect--;	/* I am out 	*/
    return (workscreen);
}

/*****************************************************************************/

/*** intuition.library/OpenScreenTagList ***/

/* This routine is a wrapper around the real meat of OpenScreen().
 * It looks for the SA_LikeWorkbench tag, and if it finds it, makes
 * up to three attempts at opening this screen.  First, it tries
 * with the most recent ScreenMode prefs.  Second, it tries with
 * the most recent prefs that succeeded.  Lastly, it resorts to
 * the default screen mode.
 */

struct Screen *openScreenTagList (struct NewScreen *ns, struct TagItem *tags)
{
    struct Screen *sc = NULL;
    struct IScreenModePrefs *sm = NULL;
    LONG likewb;
    LONG attempt = 1;
    struct IntuitionBase *IBase = fetchIBase ();

    /* likewb will be zero for most screens, 1 for workbench cloners,
     * and LIKEWB_WORKBENCH for the actual Workbench screen.
     */
    if (likewb = GetUserTagData0 (SA_LikeWorkbench, tags))
    {
	/* Make 3 attempts to open Workbench-like screen:
	 * as preferred, as last achieved, and as default
	 */
	attempt = 3;
	sm = &IBase->CoolScreenMode;
    }

    while ((!sc) && (attempt--))
    {
	sc = realOpenScreenTagList (ns, sm, tags);

	/* If the screen opened and it's the actual Workbench,
	 * then store the last mode that succeeded.
	 */
	if ((sc) && (likewb == LIKEWB_WORKBENCH))
	{
	    IBase->LastScreenMode = *sm;
	}

	/* Here we assume we failed to open.  The second attempt
	 * will be with the last mode that succeeded, and the
	 * third attempt will be with the default.
	 */
	sm = &IBase->LastScreenMode;
	if (attempt == 1)
	{
	    sm = &DefaultScreenMode;
	}
    }
    return (sc);
}

/*****************************************************************************/

struct Screen *openScreen (struct NewScreen *ns)
{
    struct TagItem *taglist = NULL;

    /* If an extended NewScreen was supplied, be sure to extract the taglist: */
    if ((ns) && (TESTFLAG (ns->Type, NS_EXTENDED)))
	taglist = XNS (ns)->Extension;
    return OpenScreenTagList (ns, taglist);
}
