/*** scsupp.c ***************************************************************
 *
 *  scsupp.c -- screens support routines
 *
 *  $Id: scsupp.c,v 40.0 94/02/15 17:51:48 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "intuall.h"

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef GRAPHICS_DISPLAYINFO_H
#include <graphics/displayinfo.h>
#endif

/*****************************************************************************/

#define D(x) 	;
#define DSG(x) 	;		/* screenGadgets*/
#define DST(x) 	;
#define DMS(x) 	;
#define DSB(x)	;		/* screenbar	*/
#define DDB(x)	;		/* double-buffering */
#define DMV(x)	;		/* SPOS_MAKEVISIBLE processing */
#define DAT(x)	;		/* Attached screens */

/*****************************************************************************/

/* This was a cute experiment to have ScreenPosition() with the
 * SPOS_MAKEVISIBLE option slide the screen across several frames,
 * to boost visual continuity.  Someday it should be made smarter
 * and implemented, either by default or through an extra flag.
 */
#define ANIMATE_MAKEVISIBLE	0
#define ANIM_MV_FRAMES		8

/*****************************************************************************/

/* Calculate the amount the parent is shifted right from its DClip
 * (LeftEdge - DClip.MinX) in the resolution of the child screen,
 * and wrt. its DClip.  Think of this as the "origin" of the
 * parent's position, in child resolution coordinates.
 */
static LONG parentOriginX (struct Screen *child, struct Screen *parent)
{
    return (((parent->LeftEdge - XSC (parent)->DClip.MinX) *
	     XSC (parent)->ScaleFactor.X) / XSC (child)->ScaleFactor.X +
	    XSC (child)->DClip.MinX);
}

/*****************************************************************************/

static void doMoveScreen (struct Screen *screen, WORD x, WORD y)
{
    /* rethinkVP will set vp dx/dyoffsets	*/
    XSC (screen)->VPOffsetStash.X =
    screen->ViewPort.DxOffset = screen->LeftEdge = x;
    XSC (screen)->VPOffsetStash.Y =
	screen->ViewPort.DyOffset = screen->TopEdge = y;

    newMakeScreen (screen, FALSE);
    screenMouse (screen);
}

/*****************************************************************************/

/* the intimacy of the following three routines separates me from
 * all other programmers [-RJ] */
/* if the BarLayer is still SIMPLE_REFRESH, you probably want to
 * do a screenbar() after this call to redraw the Screen Title Bar */
void forceBarFront (register struct Screen *s)
{
    register struct Layer_Info *LayerInfo;

    LayerInfo = &s->LayerInfo;
    LockLayerInfo (LayerInfo);
    s->BarLayer->Flags &= ~LAYERBACKDROP;
    UpfrontLayer (NULL, s->BarLayer);
    UnlockLayerInfo (LayerInfo);
}

/*****************************************************************************/

/* after calling this routine, you probably want to do a BorderPatrol()
 * to allow any newly-revealed SIMPLE_REFRESH Windows the knowledge
 * of the required refresh */
void forceBarBack (register struct Screen *s)
{
    register struct Layer_Info *LayerInfo;

    LayerInfo = &s->LayerInfo;
    LockLayerInfo (LayerInfo);
    s->BarLayer->Flags |= LAYERBACKDROP;
    BehindLayer (NULL, s->BarLayer);
    UnlockLayerInfo (LayerInfo);
}

/*****************************************************************************/

/* after calling this routine, you probably want to do a BorderPatrol()
 * to allow any newly-revealed SIMPLE_REFRESH Windows the knowledge
 * of the required refresh */
void forceBarCenter (register struct Screen *s)
{
    register struct Layer_Info *LayerInfo;

    LayerInfo = &s->LayerInfo;
    LockLayerInfo (LayerInfo);

    /***ZZZ: wonder about forcing to front first ***/
    forceBarFront (s);
    BehindLayer (NULL, s->BarLayer);
    s->BarLayer->Flags |= LAYERBACKDROP;
    UnlockLayerInfo (LayerInfo);
}

/*****************************************************************************/

#define SGTYPECOUNT 2

USHORT SGadgetTypes[SGTYPECOUNT] =
{
    SUPFRONTGADGET,
    SDRAGGADGET,		/* COUNTING on this to be last	*/
};

/*****************************************************************************/

/* take a lesser voodoo approach to this: we know there
 * are only two system screen gadgets */
LONG screenGadgets (struct Screen *s)
{
    LONG res = IsHires (s) ? HIRESGADGET : LOWRESGADGET;
    struct Gadget *g;

    DSG (printf ("screenGadgets s: %lx\n", s));

    /* get depth gadget */
    g = createNewSysGadget ((struct XScreen *) s, res, SUPFRONTGADGET, 0);
    DSG (printf ("got depth gadget at %lx\n", g));
    if (!g)
	return (0);

    s->FirstGadget = g;
    SETFLAG (g->GadgetType, SCRGADGET);
    if (TESTFLAG (s->Flags, SCREENQUIET))
    {
	g->GadgetRender = NULL;	/* ZZZ: should be more oopsi */
    }

    /* get draggadget	*/
    g = createNewSysGadget ((struct XScreen *)s, res, SDRAGGADGET, 0);
    DSG (printf ("got drag gadget at %lx\n", g));
    if (!g)
	return (0);

    s->FirstGadget->NextGadget = g;
    SETFLAG (g->GadgetType, SCRGADGET);
    g->Height = s->BarHeight + 1;	/* BarLayer is BarHeight+1 tall */
    DSG (printf ("make drag gadget height: %ld\n", g->Height));

    return (1);
}

/*****************************************************************************/

/*** intuition.library/MoveScreen ***/
/* Downcoded into intuitionface.asm */

/*****************************************************************************/

/*** intuition.library/ScreenPosition ***/
void ScreenPosition (struct Screen *screen, ULONG flags, LONG x1, LONG y1, LONG x2, LONG y2)
{
    struct Rectangle rect;

    rect.MinX = x1;
    rect.MinY = y1;
    rect.MaxX = x2;
    rect.MaxY = y2;

    /* We only allow the caller to set those flags
     * defined by SPOS_USERMASK to be user-settable.
     */
    doISM (itMOVESCREEN, (CPTR)screen, (CPTR)&rect, (flags & SPOS_USERMASK));

    /* Synchronous, so it's OK to pass a pointer to one of my local
     * variables.  Wouldn't work for sendISM()
     */
}

/*****************************************************************************/

/* Prefix routine to IMoveScreen().  Internal callers of IMoveScreen()
 * generally call that routine directly.  However, callers arriving
 * via ScreenPosition() or MoveScreen() stop here first.
 * This routine validates that the screen is known, and also
 * takes care of evaluating the SPOS_MAKEVISIBLE feature.
 */
void IPreMoveScreen (struct Screen *screen, struct Rectangle *rect, ULONG flags)
{
    struct IBox visibox, dclipbox;

#if ANIMATE_MAKEVISIBLE
    LONG animate = FALSE;

#endif

    if (knownScreen (screen))
    {
	/* If SPOS_MAKEVISIBLE was specified, then the caller wants
	 * this screen scrolled so that the specified rect is visible. */
	if ((flags & SPOS_TYPEMASK) == SPOS_MAKEVISIBLE)
	{
	    /* Get visible rectangle and dclip into IBoxes,
	     * and shift dclip box to have its origin at the
	     * screen upper-left
	     */
	    rectToBox (rect, &visibox);
#if 000
	    rectToBox (&XSC (screen)->DClip, &dclipbox);
#else
	    rectToBox (&XSC (screen)->VPExtra->DisplayClip, &dclipbox);
#endif
	    DMV (printf ("visibox: l/t/w/h = %ld,%ld,%ld,%ld\n",
			 visibox.Left, visibox.Top, visibox.Width, visibox.Height));
	    DMV (printf ("dclipbox: l/t/w/h = %ld,%ld,%ld,%ld\n",
			 dclipbox.Left, dclipbox.Top, dclipbox.Width, dclipbox.Height));
	    dclipbox.Left -= screen->LeftEdge;
	    dclipbox.Top -= screen->TopEdge;
	    DMV (printf ("dclipbox in screen coords: l/t/w/h = %ld,%ld,%ld,%ld\n",
			 dclipbox.Left, dclipbox.Top, dclipbox.Width, dclipbox.Height));

	    /* Now shift the visible box to fit in the DClip */
	    boxFit (&visibox, &dclipbox, &visibox);
	    DMV (printf ("visibox made to fit: l/t/w/h = %ld,%ld,%ld,%ld\n",
			 visibox.Left, visibox.Top, visibox.Width, visibox.Height));

	    /* The distance to move the screen is the new position
	     * less the old one...
	     */
	    rect->MinX = visibox.Left - rect->MinX;
	    rect->MinY = visibox.Top - rect->MinY;
	    DMV (printf ("Distance to move is %ld, %ld\n", rect->MinX, rect->MinY));
	    /* Convert to an SPOS_RELATIVE command */
	    flags = (flags & ~SPOS_TYPEMASK) | SPOS_RELATIVE;

#if ANIMATE_MAKEVISIBLE
	    animate = TRUE;
#endif
	}
#if ANIMATE_MAKEVISIBLE
	/* Move the screen in several steps, so that visual continuity
	 * between the old and new location is established.  This code
	 * was an experiment, worth keeping around for reference.
	 *
	 * To complete this, we'd want to tune the number of frames
	 * used for the motion, and also set a minimum motion, so that
	 * really short moves aren't done in ANIM_MV_FRAMES steps.
	 */
	if (animate)
	{
	    struct Point change;
	    LONG i;

	    change = *((struct Point *) &rect->MinX);
	    rect->MinX = change.X / ANIM_MV_FRAMES;
	    rect->MinY = change.Y / ANIM_MV_FRAMES;
	    for (i = 0; i < ANIM_MV_FRAMES; i++)
	    {
		/* On the last pass, get all the leftover */
		if (i == ANIM_MV_FRAMES - 1)
		{
		    *((struct Point *) &rect->MinX) = change;
		}
		IMoveScreen (screen, (struct Point *) rect, flags);
		change.X -= rect->MinX;
		change.Y -= rect->MinY;
	    }

	}
	else
	{
	    /* Now move the sucker... */
	    IMoveScreen (screen, (struct Point *) rect, flags);
	}
#else
	/* Now move the sucker... */
	IMoveScreen (screen, (struct Point *) rect, flags);
#endif
    }
}

/*****************************************************************************/

void IMoveScreen (struct Screen *screen, struct Point *delta, WORD flags)
#if 0
struct Screen *screen;
struct Point *delta;		/* delta motion, unless SPOS_ABSOLUTE specified */
WORD flags;			/* magic menu can move us farther right than nml */
#endif
{
    struct Rectangle legal_positions;
    struct Point position;
    WORD dx, dy;
    struct MinNode *node, *succ;
    struct Screen *child;

    DMS (printf ("--- MoveScreen(%lx,%ld,%ld)\n", screen, dx, dy));
    DMS (printf ("   old position: %ld, %ld\n", screen->LeftEdge,
		 screen->TopEdge));

    /* Assumed to be deltas, but later we'll check SPOS_ABSOLUTE */
    dx = delta->X;
    dy = delta->Y;

    /* This is an "early bail-out" test.  Scram if the screen is not
     * known or no motion was requested.  If SPOS_REVALIDATE is set,
     * we stick around, since we're being asked to revalidate the
     * screen position even though no motion was requested.  If
     * SPOS_ABSOLUTE is set, it's too early to tell if any motion
     * occurred.
     */
    if (!(dx || dy || (TESTFLAG (flags, SPOS_ABSOLUTE | SPOS_REVALIDATE))))
    {
	return; // (FALSE);
    }

    /* If we can't slide the screen, then we have no further
     * work to do here.
     */
    if (!canSlide (screen, TESTFLAG (flags, SPOS_FORCEDRAG)))
    {
	DMS (printf ("MoveScreen: non-draggable screen, not forced!\n"));
	return; // (FALSE);
    }

    /* get the legal positions for the screen,
     * in screen coords
     * Peter 26-Apr-91: For compatibility, programmatic screen motion
     * is unbounded at the bottom.  (Note that interactive screen
     * dragging is bounded in startScreenDrag() by the
     * establishment of mouse limits)
     */
    screenLegalPosition (screen, &legal_positions, flags | SPOS_UNBOUNDEDBOTTOM);

    /* get (proposed) new position in screen coordinates, assuming
     * that they're relative (the traditional kind)
     */
    position.X = screen->LeftEdge + dx;
    position.Y = screen->TopEdge + dy;
    if (TESTFLAG (flags, SPOS_ABSOLUTE))
    {
	/* We just discovered that the caller supplied absolute
	 * coordinates instead, so grab those...
	 */
	position = *delta;
    }

    DMS (printf ("MS: new proposed position: %ld, %ld\n",
		 position.X, position.Y));

    limitPoint (&position, &legal_positions);

    DMS (printf ("constrained new position: %ld, %ld\n", position.X, position.Y));

    if ((position.Y - screen->TopEdge) || (position.X - screen->LeftEdge))
    {
	struct Screen *parent;

	/* Need the VIEWCPRLOCK (which is really the GfxBase->ActiViewCprSemaphore)
	 * to protect the copper lists between MakeScreen() and
	 * RethinkDisplay().  I also grab the IBASELOCK since MakeScreen()
	 * needs that, and I've got to get it before the VIEWLOCK.
	 */
	LOCKIBASE ();
	LOCKVIEW ();
	LOCKVIEWCPR ();

	/* Is this screen a child, attached to some parent? */
	if (parent = XSC (screen)->ParentScreen)
	{
	    /* Update its ParentYOffset wrt. the parent screen */
	    XSC (screen)->ParentYOffset = position.Y - parentOriginY (screen, parent);
	    XSC (screen)->ParentXOffset = position.X - parentOriginX (screen, parent);
	}
	doMoveScreen (screen, position.X, position.Y);

	/* If this screen has any children, move them accordingly */
	for (node = XSC (screen)->Family.mlh_Head;
	     succ = node->mln_Succ;
	     node = succ)
	{
	    /* One of the screens on the Family list is the
	     * screen itself, and we don't want to act on it.
	     */
	    if ((child = SCREENFROMNODE (node)) != screen)
	    {
		/* X-coordinate:  if the child is draggable, it doesn't
		 * move in X, so we leave the X-coordinate unchanged.
		 * For non-draggable screens, we maintain a constant
		 * X-position with respect to the parent.
		 * Y-coordinate:  draggable or not, the screen moves
		 * to maintain the constant Y-position with respect to
		 * the parent.  Draggability affects that computation
		 * slightly in parentOffsetY().
		 */
		position.X = child->LeftEdge;
		if (!TESTFLAG (XSC (child)->PrivateFlags, PSF_DRAGGABLE))
		{
		    position.X = XSC (child)->ParentXOffset + parentOriginX (child, screen);
		}
		position.Y = XSC (child)->ParentYOffset + parentOriginY (child, screen);

		/* get the legal positions for the screen,
		 * in screen coords
		 * For compatibility, programmatic screen motion
		 * is unbounded at the bottom.  (Note that interactive screen
		 * dragging is already bounded in startScreenDrag() by the
		 * establishment of mouse limits).
		 * We set SPOS_FORCEDRAG because we always want to drag
		 * (even non-draggable) child screens when the parent moves.
		 */
		screenLegalPosition (child, &legal_positions,
				     flags | SPOS_FORCEDRAG | SPOS_UNBOUNDEDBOTTOM);
		limitPoint (&position, &legal_positions);

		doMoveScreen (child, position.X, position.Y);
	    }
	}
	if (!TESTFLAG (flags, SPOS_NORETHINK))
	{
	    /* And display the result */
	    RethinkDisplay ();
	}
	UNLOCKVIEWCPR ();
	UNLOCKVIEW ();
	UNLOCKIBASE ();
    }
    DMS (
	    else
	    printf ("don't MOVE!\n"));
}

/*****************************************************************************/

/*** intuition.library/ScreenToFront ***/
/* asm stub goes through ScreenDepth() LVO */

/*****************************************************************************/

/*** intuition.library/ScreenToBack ***/
/* asm stub goes through ScreenDepth() LVO */

/*****************************************************************************/

/* reserved --- for future support as a reference screen if we add SDEPTH_INFRONTOF
 * and SDEPTH_BEHINDOF flags. */

/*** intuition.library/ScreenDepth ***/
void LIB_ScreenDepth (struct Screen *screen, ULONG flags, struct Screen *reserved)
{
    doISM (itDEPTHSCREEN, (CPTR)screen, NULL, (flags & SDEPTH_USERMASK));
}

/*****************************************************************************/

/* if screen is NULL, try WB screen */
void IScreenDepth (struct Screen *screen, LONG whichway)
{
    struct IntuitionBase *IBase = fetchIBase ();
    struct Screen *findWorkBench ();

    D (printf ("IScreenDepth %lx, %ld\n", screen, whichway));

    LOCKIBASE ();

    /* need a screen here: use WB if 'screen' is NULL */
    if ((screen) || (screen = findWorkBench ()))
    {

	/* Peter 11-Mar-91: In order to help IHelp and other "hostile"
	 * screen-movers, we try to ensure we have a real screen here
	 */
	if (knownScreen (screen))
	{
	    CLEARFLAG (IBase->Flags, POPPEDSCREEN);
	    /* Put the screen in the right place */
	    relinkScreen (screen, whichway);
	}
    }

    UNLOCKIBASE ();

    RethinkDisplay ();
}

/*****************************************************************************/

/*** intuition.library/GetScreenDrawInfo ***/
struct DrawInfo *GetScreenDrawInfo (struct Screen *s)
{
    return (&XSC (s)->SDrawInfo);
}

/*****************************************************************************/

/*** intuition.library/FreeScreenDrawInfo ***/
void FreeScreenDrawInfo (struct Screen *s, struct DrawInfo *dri)
{
}

/*****************************************************************************/

/* set this screen as the ActiveScreen */
void setScreen (struct Screen *s)
{
    struct IntuitionBase *IBase = fetchIBase ();

    /* set the system global ActiveScreen Pointer */
    if ((IBase->ActiveScreen = s) == NULL)
	return;

    IBase->MenuRPort = s->RastPort;

    IBase->MenuRPort.TmpRas = NULL;

    IBase->MenuRPort.AreaPtrn = NULL;
    IBase->MenuRPort.Layer = NULL;
    IBase->MenuRPort.AreaInfo = NULL;
}

/*****************************************************************************/

/* Common routine to draw a blank screen-bar */
void drawScreenBar (struct RastPort *rp, struct Screen *s, ULONG blockpen)
{
    drawBlock (rp, 0, 0, s->Width - 1, s->BarHeight - 1, blockpen);
    drawBlock (rp, 0, s->BarHeight, s->Width - 1, s->BarHeight, XSC (s)->SDrawInfo.dri_Pens[BARTRIMPEN]);
}

/*****************************************************************************/

#define SCREEN_TEXT_LEFT	(0)

/*****************************************************************************/

void refreshScreenBar (struct Screen *sc)
{
    struct Layer *barlayer = sc->BarLayer;

    if (TESTFLAG (barlayer->Flags, LAYERREFRESH))
    {
	BeginUpdate (barlayer);
	screenbar (sc);
	CLEARFLAG (barlayer->Flags, LAYERREFRESH);
	EndUpdate (barlayer, TRUE);
    }
}

/*****************************************************************************/

/* ZZZ: ISM: try to bring this under the state machine,
 * to eliminate the knownScreen() business (which I've done)
 * otherway: in OpenScreen: I know the screen is "known" */
/* Displays the title bar of the screen argument */
void screenbar (register struct Screen *s)
{
    struct IntuiText itext;
    struct RastPort *rp;
    struct DrawInfo *dri;

    /* respect user's wish for silence */
    if ((s->Flags & SCREENQUIET))
	return;

    /* LockLayerInfo(&s->LayerInfo); */
    /* nobody is going to be deleting this screen,
     * or doing anything else untoward with it.
     * do I need to be protected against damage?
     * I don't think so. (rev 1.5)
     *
     * Oops, I'm wrong.  This is the protection against
     * SetWindowTitles().  Maybe that function should be
     * brought under ISM someday, but I'm going to
     * let this back in.
     */

    LockLayerInfo (&s->LayerInfo);

    rp = s->BarLayer->rp;
    DSB (printf ("barlayer rastport %lx, rp layer %lx bar layer %lx\n",
		 rp, rp->Layer, s->BarLayer));

    dri = &XSC (s)->SDrawInfo;
    drawScreenBar (rp, s, dri->dri_Pens[BARBLOCKPEN]);

    if (s->Title)
    {
	DSB (printf ("new screen title %s\n", s->Title));
	itext.FrontPen = dri->dri_Pens[BARDETAILPEN];
	itext.BackPen = dri->dri_Pens[BARBLOCKPEN];
	itext.DrawMode = JAM2;
	itext.TopEdge = s->BarVBorder;
	itext.LeftEdge = s->BarHBorder;
	itext.ITextFont = NULL;
	itext.IText = s->Title;
	itext.NextText = NULL;
	PrintIText (s->BarLayer->rp, &itext, SCREEN_TEXT_LEFT, 0);
    }

    drawGadgets (s, s->FirstGadget, -1, DRAWGADGETS_ALL);

    UnlockLayerInfo (&s->LayerInfo);
}

/*****************************************************************************/

/*** intuition.library/ShowTitle ***/
void ShowTitle (struct Screen *screen, BOOL showit)
{
    DST (printf ("ShowTitle: %lx, %ld\n", screen, showit));
    doISM (itSHOWTITLE, (CPTR)screen, NULL, showit);
}

/*****************************************************************************/

void IShowTitle (struct Screen *screen, BOOL showit)
{
    DST (printf ("IShowTitle: %lx, %ld\n", screen, showit));
    if (knownScreen (screen))
    {
	/* protect my BorderPatrol */
	LockLayerInfo (&screen->LayerInfo);

	if (showit)
	{
	    screen->Flags |= SHOWTITLE;
	    forceBarCenter (screen);
	    screenbar (screen);
	}
	else
	{
	    screen->Flags &= ~SHOWTITLE;
	    forceBarBack (screen);
	}

	BorderPatrol (screen);	/* synchronous */

	UnlockLayerInfo (&screen->LayerInfo);
    }
}

/*****************************************************************************/

/* The following silly combinations are handled thusly:
 *
 * Non-CUSTOMBITMAP, bm!=NULL				fail
 * CUSTOMBITMAP, SB_SCREEN_BITMAP set			fail
 * SB_SCREEN_BITMAP and SB_COPY_BITMAP set		copy not done
 * Non-CUSTOMBITMAP, bm=original, SB_COPY_BITMAP set
 */

/*** intuition.library/AllocScreenBuffer ***/
struct ScreenBuffer *AllocScreenBuffer (struct Screen *sc, struct BitMap *bm, ULONG flags)
{
    struct ScreenBuffer *sb;

    DDB (printf ("AllocScreenBuffer(screen %lx, bitmap %lx, flags %lx)\n", sc, bm, flags));

    /* We need an extended ScreenBuffer structure */
    if (sb = (struct ScreenBuffer *) AllocVec (sizeof (struct XScreenBuffer), MEMF_CLEAR))
    {
	/* Get a double-buffering handle from graphics */
	if (sb->sb_DBufInfo = AllocDBufInfo (&sc->ViewPort))
	{
	    if (TESTFLAG (flags, SB_SCREEN_BITMAP))
	    {
		/* Caller wants the screen's actual bitmap */
		sb->sb_BitMap = XSC (sc)->RealBitMap;
		DDB (printf ("SB_SCREEN_BITMAP:  "));
		/* We would free this BitMap in the event that
		 * FreeScreenBuffer() is called on this buffer and
		 * this isn't the current buffer.
		 */
		XSB (sb)->sb_Flags = SB_FREEBITMAP;
		/* We never copy the screen to itself */
		CLEARFLAG (flags, SB_COPY_BITMAP);
	    }
	    else if (!bm)
	    {
		/* Caller wants Intuition to create a bitmap.
		 * This screen may be CUSTOMBITMAP or not.
		 */
		sb->sb_BitMap = AllocBitMap (sc->Width, sc->Height,
					     sc->RastPort.BitMap->Depth, BMF_DISPLAYABLE | BMF_CLEAR,
					     sc->RastPort.BitMap);
		XSB (sb)->sb_Flags = SB_FREEBITMAP;
		DDB (printf ("Allocated bitmap:  "));
	    }
	    else if (TESTFLAG (sc->Flags, CUSTOMBITMAP))
	    {
		/* Caller supplied his own bitmap.  This is
		 * only allowed for CUSTOMBITMAP screens.
		 */
		sb->sb_BitMap = bm;
		DDB (printf ("Custom bitmap:  "));
	    }
	    /* else we fail, on account of bm!=NULL but a non-CUSTOMBITMAP. */
	}

	/* Failure is (currently) always detectable by the sb_BitMap
	 * pointer being NULL.
	 */
	if (!sb->sb_BitMap)
	{
	    FreeScreenBuffer (sc, sb);
	    sb = NULL;
	}
	else if (TESTFLAG (flags, SB_COPY_BITMAP))
	{
	    /* Want to start subsequent buffers to match the first.
	     * We do it state-machine synchronous to ensure that
	     * the menu bar isn't up and gadgets aren't active.
	     */
	    doISM (itCOPYSCBUF, (CPTR)sc, (CPTR)sb, NULL);
	}

    }
    DDB (printf ("ScreenBuffer %lx, sb_BitMap %lx, sb_DBufInfo %lx\n\n",
		 sb, sb ? sb->sb_BitMap : 0, sb ? sb->sb_DBufInfo : 0));
    return (sb);
}

/*****************************************************************************/

/* Copy the screen's bitmap into the the new ScreenBuffer's BitMap.
 * We do it in a state-synchronous way to ensure that the menu bar
 * isn't up and gadgets aren't active. */
void ICopyScreenBuffer (struct Screen *sc, struct ScreenBuffer *sb)
{
    /* Start the new ScreenBuffer's BitMap out to match the old */
    BltBitMap (sc->RastPort.BitMap, 0, 0,
	       sb->sb_BitMap, 0, 0,
	       sc->Width, sc->Height, 0x00C0, ~0, 0);
}

/*****************************************************************************/

/*** intuition.library/FreeScreenBuffer ***/
void FreeScreenBuffer (struct Screen *sc, struct ScreenBuffer *sb)
{
    DDB (printf ("FreeScreenBuffer(%lx, %lx)\n", sc, sb));
    if (sb)
    {
	FreeDBufInfo (sb->sb_DBufInfo);
	/* Free the bitmap if we allocated it here (or during OpenScreen()),
	 * provided it's not the one currently in use.
	 * Don't need to sync up with anything since the ChangeScreenBuffer()
	 * call returns synchronously.
	 */
	if ((TESTFLAG (XSB (sb)->sb_Flags, SB_FREEBITMAP)) &&
	    (sb->sb_BitMap != XSC (sc)->RealBitMap))
	{
	    DDB (printf (" Freeing BitMap %lx\n", sb->sb_BitMap));
	    WaitBlit ();
	    FreeBitMap (sb->sb_BitMap);
	}
	FreeVec (sb);
    }
    DDB (printf ("FSB: out\n\n"));
}

/*****************************************************************************/

/*** intuition.library/ChangeScreenBuffer ***/

LONG ChangeScreenBuffer (struct Screen *sc, struct ScreenBuffer *sb)
{
    DDB (printf ("doISM( itCHANGESCBUF, sc=%lx,sb=%lx )\n", sc, sb));
    return (!doISM (itCHANGESCBUF, (CPTR)sc, (CPTR)sb, NULL));
}

/*****************************************************************************/

void IChangeScreenBuffer (struct Screen *screen, struct ScreenBuffer *sb)
{
    DDB (printf ("Calling CVPBitMap(vp=%lx, bm=%lx, dbi=%lx)\n",
		 &screen->ViewPort, sb->sb_BitMap, sb->sb_DBufInfo));
    ChangeVPBitMap (&screen->ViewPort, sb->sb_BitMap, sb->sb_DBufInfo);
    DDB (printf ("Back from CVPBitMap()\n"));
    /* ChangeVPBitMap() has changed the ViewPort's RasInfo BitMap,
     * but I also have a few things of my own to change.
     * First, update the embedded copy of the screen's BitMap.
     * Second, update the screen's RastPort's BitMap, which
     * is the one everyone should use.
     */
    screen->sc_BitMap = *sb->sb_BitMap;
    screen->RastPort.BitMap = sb->sb_BitMap;

    /* RealBitMap is non-NULL for non-CUSTOMBITMAP screens.  Thus,
     * it's pointing to a BitMap that Intuition got from AllocBitMap(),
     * initially during OpenScreen().  If non-NULL, we may replace it
     * here with the newly-installed bitmap.  We KNOW we allocated
     * that one, since AllocScreenBuffer() doesn't permit user-allocated
     * BitMaps for non-CUSTOMBITMAP screens.
     */
    if (XSC (screen)->RealBitMap)
    {
	XSC (screen)->RealBitMap = sb->sb_BitMap;
    }

    /* Mike assured me that since these are my layers, it's
     * safe to change their bitmaps.  Ok, Mike...
     */
    XSC (screen)->ClipLayer->rp->BitMap = sb->sb_BitMap;
    screen->BarLayer->rp->BitMap = sb->sb_BitMap;

    DDB (printf ("CSB: out\n\n"));
}

/*****************************************************************************/

/* Calculate the amount the parent is pulled down from its DClip
 * (TopEdge - DClip.MinY) in the resolution of the child screen,
 * and wrt. its DClip.  Think of this as the "origin" of the
 * parent's position, in child resolution coordinates. */
LONG parentOriginY (struct Screen *child, struct Screen *parent)
{
#if 1
    LONG result;

    result = parent->TopEdge - XSC (parent)->DClip.MinY;

    /* A negative result indicates that the parent is taller than
     * its DClip and pulled up above the DClip top.  We want draggable
     * child screen to stop following up when the parent is dragged
     * off the top.
     */
    if (TESTFLAG (XSC (child)->PrivateFlags, PSF_DRAGGABLE) &&
	(result < 0))
    {
	result = 0;
    }

    return ((result *
	     XSC (parent)->ScaleFactor.Y) / XSC (child)->ScaleFactor.Y +
	    XSC (child)->DClip.MinY);

#else
    return (((parent->TopEdge - XSC (parent)->DClip.MinY) *
	     XSC (parent)->ScaleFactor.Y) / XSC (child)->ScaleFactor.Y +
	    XSC (child)->DClip.MinY);
#endif
}

/*****************************************************************************/

/* Returns a screen that uniquely identifies this screen's family,
 * namely the parent screen if there is one, else the screen itself.
 */
struct Screen *screenFamily (struct Screen *s)
{
    if (XSC (s)->ParentScreen)
	s = XSC (s)->ParentScreen;
    return (s);
}

/*****************************************************************************/

/* Attach or detach a child screen from a parent.
 * child must be a valid pointer to a screen.
 * If parent is non-NULL, attach this screen to that parent.
 * (Neither parent nor child may have a parent at the time this
 * function is called).
 *
 * This function does not ensure that the newly attached
 * screens are adjacent depth-wise nor does it ensure that
 * the child's top edge doesn't exceed the parent.  That's
 * handled elsewhere.
 *
 * If parent is NULL, detach. (It's safe to call this with
 * parent=NULL even when the child screen is not attached
 * to any parent.)
 *
 * Please hold the ISTATELOCK when calling...
 *
 * Returns zero for success, OSERR_ATTACHFAIL for failure.
 */
LONG attachScreen (struct Screen *child, struct Screen *parent)
{
    LONG error = 0;

    DAT (printf ("attachScreen( child %lx to parent %lx)\n", child, parent));
    assertLock ("attachScreen", ISTATELOCK);

    /* parent != NULL implies we're to attempt to attach a screen */
    if (parent)
    {
	struct MinNode *node, *succ;

	/* Let's check for illegal attachments.  A bit later,
	 * we'll check if the parent-candidate already has a parent
	 * or if the child-candidate already has a parent.
	 * Here, we test to see if the child-candidate already
	 * has children.
	 */
	for (node = XSC (child)->Family.mlh_Head;
	     succ = node->mln_Succ;
	     node = succ)
	{
	    /* One of the screens on the Family list is the
	     * screen itself, and we don't want to panic
	     * if we see that
	     */
	    if (SCREENFROMNODE (node) != child)
	    {
		DAT (printf ("aS: Failing because child already has children\n"));
		error = OSERR_ATTACHFAIL;
		break;
	    }
	}

	DAT (printf ("aS: c:%lx->ParentScreen = %lx\n", child, XSC (child)->ParentScreen));
	DAT (printf ("aS: p:%lx->ParentScreen = %lx\n", parent, XSC (parent)->ParentScreen));
	if ((XSC (parent)->ParentScreen) ||
	    (XSC (child)->ParentScreen) ||
	    TESTFLAG (XSC (parent)->PrivateFlags, PSF_EXCLUSIVE) !=
	    TESTFLAG (XSC (child)->PrivateFlags, PSF_EXCLUSIVE))
	{
	    /* It is illegal to attempt to attach a child which is already
	     * attached to some parent, and it's illegal to attempt
	     * to attach a child to a parent which is itself a child
	     * screen.  Also, if one of the two screens is exclusive,
	     * then fail.  If they're both exclusive, then it's an
	     * exclusive family.
	     */
	    DAT (printf ("aS: Failing because parent or child has parent, or exclusive\n"));
	    error = OSERR_ATTACHFAIL;
	}

	if (!error)
	{
	    /* Let's set up the family linkages, including indicating
	     * the parent and adding the child to the end of its
	     * parent's list.
	     */
	    XSC (child)->ParentScreen = parent;
	    Remove ((struct Node *) &XSC (child)->ChildNode);
	    AddTail ((struct List *) &XSC (parent)->Family, (struct Node *) &XSC (child)->ChildNode);

	    /* Let's establish the ParentYOffset */
	    XSC (child)->ParentXOffset = child->LeftEdge -
		parentOriginX (child, parent);
	    XSC (child)->ParentYOffset = child->TopEdge -
		parentOriginY (child, parent);
	    DAT (printf ("aS: attaching.  c:%lx->ParentScreen now %lx\n",
			 child, XSC (child)->ParentScreen));
	}
    }
    else
    {
	/* parent == NULL means detach screen if attached */
	if (XSC (child)->ParentScreen)
	{
	    /* Take the ChildNode off the parent's Family list,
	     * and put it back at the head of its own Family list.
	     */
	    Remove ((struct Node *) &XSC (child)->ChildNode);
	    AddHead ((struct List *) &XSC (child)->Family, (struct Node *) &XSC (child)->ChildNode);
	    XSC (child)->ParentScreen = NULL;
	    DAT (printf ("aS: detaching\n"));
	}
	/* else was not attached; do nothing */
    }

    return (error);
}

/*****************************************************************************/

/* Routine to put a screen in the right place in the screen-list.  Here
 * are the rules:
 *
 * For regular screens (neither child nor parent), SDEPTH_TOFRONT puts
 * that screen at the very head of the screen-list, and SDEPTH_TOBACK
 * puts that screen at the very end.
 *
 * Normal depth-arrangement of a child or parent screen sends that
 * screen's whole family to the front or the back of the screen list,
 * without affecting the ordering within the family.  This type
 * of depth-arrangement includes Amiga-M/N, the depth-gadgets, and
 * the ScreenToFront()/ScreenToBack() calls.
 *
 * Special SDEPTH_INFAMILY depth-arrangement moves a family member
 * to the front or rear of their family.  This feature is
 * used at OpenScreen() time to establish the original position.
 * It will also be accessible through new screen-depth functions.
 *
 * NOTE WELL:  This function assumes that screens on the list
 * are well-ordered, i.e. no family is split up.
 */
void relinkScreen (struct Screen *screen, ULONG flags)
{
    struct IntuitionBase *IBase = fetchIBase ();
    struct Screen *parent = XSC (screen)->ParentScreen;
    struct Screen *frontmember;
    struct Screen *backmember;

    /* Is this screen a parent screen?  If so, we want to set
     * the parent-pointer.
     *
     * If this is the initial linking of the parent screen, then
     * we want to link it alone.  By not setting parent, we ensure
     * we don't do something to the whole family.  So we explicitly
     * fail this test in that case.
     *
     * Now the way we tell is unusual, because I can't think of
     * a more straightforward way right now (everywhere else where
     * we care to know about parent screens, we're running a loop
     * of "do something for each child".  Now a screen is either a
     * parent, a child, or unattached:
     *
     * - child screens have a non-NULL XSC(sc)->ParentScreen.
     * - Non-attached screens have only a single node on the Family
     *   list (their own node).  A single node can be detected by
     *   looking for lh_Head == lh_TailPred ( == the single node)
     * - Parent screens always have more than one node (their own plus
     *   at least one child's).
     */
    if (!TESTFLAG (flags, SDEPTH_INITIAL) && (!XSC (screen)->ParentScreen) &&
	(XSC (screen)->Family.mlh_Head) != (XSC (screen)->Family.mlh_TailPred))
    {
	parent = screen;
    }

    if (parent)
    {
	/* Let's save off pointers to the first and last screens
	 * in this screen's family.  We need those to handle
	 * all the de/re-linking issues.
	 */
	frontmember = SCREENFROMNODE (XSC (parent)->Family.mlh_Head);
	backmember = SCREENFROMNODE (XSC (parent)->Family.mlh_TailPred);
    }

    /* We want to move the parent's whole family as a unit under
     * the following conditions:
     *
     * - We are dealing with a real screen family (parent != NULL)
     * - and the caller didn't ask to just move within the family
     *
     * To move the family as a unit, we delink from frontmember to
     * backmember, then relink the whole gang up front or down back.
     */
    if ((parent) && (!TESTFLAG (flags, SDEPTH_INFAMILY)))
    {
	/* Delink family: */
	((struct Screen *) previousThing ((struct Thing *)frontmember,
			  (struct Thing *)&IBase->FirstScreen, 0))->NextScreen = backmember->NextScreen;

	if ((flags & SDEPTH_DIRECTIONMASK) == SDEPTH_TOFRONT)
	{
	    /* Link family at the head */
	    backmember->NextScreen = IBase->FirstScreen;
	    IBase->FirstScreen = frontmember;
	}
	else
	    /* ( ( flags & SDEPTH_DIRECTIONMASK ) == SDEPTH_TOBACK ) */
	{
	    /* Link family to the end */
	    ((struct Screen *) lastThing ((struct Thing *)&IBase->FirstScreen))->NextScreen =
		frontmember;
	    backmember->NextScreen = NULL;
	}
    }
    else
	/* Screen has no parent or children, or screen is a family
	  * member to be moved on its own:
	  */
    {
	/* By default, assume we'll insert at the head of the list */
	struct Screen **scptr = &IBase->FirstScreen;

	/* Let's delink the screen from the screen list, and, if it's
	 * a child screen, remove its ChildNode from the Family list it's on.
	 * Now, if this is the initial linking (SDEPTH_INITIAL), then the
	 * screen is not on the screen list.  However, its ChildNode IS
	 * ALWAYS on some list.
	 */
	if (!TESTFLAG (flags, SDEPTH_INITIAL))
	{
	    delinkScreen (screen);
	}
	if (parent)
	{
	    Remove ((struct Node *) &XSC (screen)->ChildNode);
	}

	if ((flags & SDEPTH_DIRECTIONMASK) == SDEPTH_TOFRONT)
	{
	    if (parent)
	    {
		/* This is a child screen, so insert it ahead of
		 * the parent's first child, and make this screen
		 * the first child
		 */
		scptr = (struct Screen **) previousThing ((struct Thing *) SCREENFROMNODE (XSC (parent)->Family.mlh_Head),
							  (struct Thing *) scptr, 0);
		AddHead ((struct List *) &XSC (parent)->Family, (struct Node *) &XSC (screen)->ChildNode);
	    }
	    screen->NextScreen = *scptr;
	    *scptr = screen;
	}
	else
	    /* SDEPTH_TOBACK */
	{
	    struct Screen *tailsc = (struct Screen *) lastThing ((struct Thing *)scptr);

	    if (parent)
	    {
		/* This is a child screen, so insert it after
		 * the parent's last child, and make this screen
		 * the last child
		 */
		tailsc = SCREENFROMNODE (XSC (parent)->Family.mlh_TailPred);
		AddTail ((struct List *) &XSC (parent)->Family, (struct Node *) &XSC (screen)->ChildNode);
	    }
	    screen->NextScreen = tailsc->NextScreen;
	    tailsc->NextScreen = screen;
	}
    }
}

/*****************************************************************************/

/* Remove this screen from the screen list. */
void delinkScreen (struct Screen *OScreen)
{
    delinkThing ((struct Thing *)OScreen, (struct Thing *)&(fetchIBase ())->FirstScreen);
}

/*****************************************************************************/

/* Returns TRUE if a screen can slide.
 *
 * A screen cannot slide if it does not have the DIPF_IS_DRAGGABLE
 * graphics database property.
 *
 * A screen cannot slide if is made non-draggable through the use
 * of the {SA_Draggable,FALSE} attribute.  This particular decision
 * can be overridden with the override parameter.
 *
 * This override is set when a programmer calls ScreenPosition() with
 * the SPOS_FORCEDRAG option.  The override is also set when Intuition
 * moves a child screen due to the parent moving.
 */
LONG canSlide (struct Screen *s, ULONG override)
{
    return (TESTFLAG (XSC (s)->DProperties, DIPF_IS_DRAGGABLE) &&
	    (TESTFLAG (XSC (s)->PrivateFlags, PSF_DRAGGABLE) || (override)));
}

/*****************************************************************************/

BOOL IsHires (struct Screen *s)
{
    return (BOOL)(s->Flags & SCREENHIRES);
}
