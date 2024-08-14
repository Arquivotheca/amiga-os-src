/*** wtags.c *****************************************************************
 *
 *  Window TagItem interface
 *
 *  $Id: wtags.c,v 40.0 94/02/15 17:48:59 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include <intuition/intuition.h>
#include "intuall.h"
#include "imageclass.h"

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef UTILITY_PACK_H
#include <utility/pack.h>
#endif

/*****************************************************************************/

#define	DB(x)		;	/* General debugging information */
#define D(x)		;
#define DST(x)		;	/* screen titles	*/
#define DOW(x)		;	/* debug open window	*/
#define DOWS(x)		;	/* debug open window	*/
#define DWB(x)		;	/* debug window border calc	*/
#define DFAIL(x)	;	/* gives query to fail OWT	*/
#define DOWF(x)		;	/* OpenWindow failure test code:
				 * Use {WA_Dummy, num} to cause failure
				 * in different places */

/*****************************************************************************/

#define NEWBORDERS		1
#define AUTOADJUST_DCLIP	0

/*****************************************************************************/

/* Structure to hold the window flags before the Window is allocated: */
struct WFlagTrap
{
    ULONG Flags;
    ULONG MoreFlags;
};

/*****************************************************************************/

/* Warning:  the choices of simple refresh, smart refresh, super bitmap
 * together share two bits.  Under V37, PackBoolTags() had an entry
 * mapping WA_SmartRefresh to SMART_REFRESH, which is zero.  That's
 * a DO NOTHING entry, since {WA_SmartRefresh,TRUE} makes PBT() do
 * nothing.  PackStructureTags() isn't as happy with a zero flag,
 * so we omit the table entry right out.
 * It might be better to have PackStructureTags() entries that do:
 *
 *	WA_SimpleRefresh: set   SIMPLE_REFRESH, clear SUPER_BITMAP
 *	WA_SmartRefresh:  clear SIMPLE_REFRESH, clear SUPER_BITMAP
 *	WA_SuperBitMap:   clear SIMPLE_REFRESH, set   SUPER_BITMAP
 *
 */
ULONG windowFlagsPackTable[] =
{
    PACK_STARTTABLE (WA_Dummy),

    PACK_LONGBIT (WA_Dummy, WA_SizeGadget, WFlagTrap, Flags, PKCTRL_BIT, WINDOWSIZING),
    PACK_LONGBIT (WA_Dummy, WA_DragBar, WFlagTrap, Flags, PKCTRL_BIT, WINDOWDRAG),
    PACK_LONGBIT (WA_Dummy, WA_DepthGadget, WFlagTrap, Flags, PKCTRL_BIT, WINDOWDEPTH),
    PACK_LONGBIT (WA_Dummy, WA_CloseGadget, WFlagTrap, Flags, PKCTRL_BIT, WINDOWCLOSE),
    PACK_LONGBIT (WA_Dummy, WA_Backdrop, WFlagTrap, Flags, PKCTRL_BIT, BACKDROP),
    PACK_LONGBIT (WA_Dummy, WA_ReportMouse, WFlagTrap, Flags, PKCTRL_BIT, REPORTMOUSE),
    PACK_LONGBIT (WA_Dummy, WA_NoCareRefresh, WFlagTrap, Flags, PKCTRL_BIT, NOCAREREFRESH),
    PACK_LONGBIT (WA_Dummy, WA_Borderless, WFlagTrap, Flags, PKCTRL_BIT, BORDERLESS),
    PACK_LONGBIT (WA_Dummy, WA_Activate, WFlagTrap, Flags, PKCTRL_BIT, ACTIVATE),
    PACK_LONGBIT (WA_Dummy, WA_SizeBRight, WFlagTrap, Flags, PKCTRL_BIT, SIZEBRIGHT),
    PACK_LONGBIT (WA_Dummy, WA_SizeBBottom, WFlagTrap, Flags, PKCTRL_BIT, SIZEBBOTTOM),
    PACK_LONGBIT (WA_Dummy, WA_RMBTrap, WFlagTrap, Flags, PKCTRL_BIT, RMBTRAP),
    PACK_LONGBIT (WA_Dummy, WA_WBenchWindow, WFlagTrap, Flags, PKCTRL_BIT, WBENCHWINDOW),

    PACK_LONGBIT (WA_Dummy, WA_SimpleRefresh, WFlagTrap, Flags, PKCTRL_BIT, SIMPLE_REFRESH),
/*    PACK_LONGBIT( WA_Dummy, WA_SmartRefresh, WFlagTrap, Flags, PKCTRL_BIT, SMART_REFRESH ),*/
    PACK_LONGBIT (WA_Dummy, WA_SuperBitMap, WFlagTrap, Flags, PKCTRL_BIT, SUPER_BITMAP),

    PACK_LONGBIT (WA_Dummy, WA_GimmeZeroZero, WFlagTrap, Flags, PKCTRL_BIT, GIMMEZEROZERO),

    PACK_LONGBIT (WA_Dummy, WA_NewLookMenus, WFlagTrap, Flags, PKCTRL_BIT, WFLG_NEWLOOKMENUS),

    PACK_LONGBIT (WA_Dummy, WA_MenuHelp, WFlagTrap, MoreFlags, PKCTRL_BIT, WMF_USEMENUHELP),
    PACK_LONGBIT (WA_Dummy, WA_NotifyDepth, WFlagTrap, MoreFlags, PKCTRL_BIT, WMF_NOTIFYDEPTH),
    PACK_LONGBIT (WA_Dummy, WA_TabletMessages, WFlagTrap, MoreFlags, PKCTRL_BIT, WMF_TABLETMESSAGES),

    PACK_ENDTABLE,
};

/*****************************************************************************/

/*
 * precalculate window border dimensions.
 * This would make a nice external entry point, but at this time
 * (jan 90), it's hard to say exactly what information would be
 * required, and it would be a shame to require the entire
 * NewWindow *and tags*
 *
 * fills out rectangle with the answers.
 *
 * ZZZ: limitation noted: rightborder and bottomborder only
 * expand (and should?) if gadget is grelright (resp grelbottom)
 *
 * 2/12/90: nope, this thing doesn't work.  Since it's
 * not an external entry point for v36, I'm going back to
 * the gadgetinfo approach ala old userBorders().
 * see symbol USER_BORDERS
 *
 * OH NO, I'M HOSED!! I don't have enough info to do gadgetInfo
 * yet (no window dimensions) since I want to do INNERWIDTH/HEIGHT.
 * But how do I do GRELWIDTH/HEIGHT ....???
 */

#define USER_BORDERS	(0)	/* fall back to old userBorders */

/*****************************************************************************/

/*
 * 'width' and 'height' can only be valid if the user provides
 * them, so WA_InnerWidth will be providing 0.  The
 * restrictions on border gadgets that this imposes are
 * reasonable and documented with WA_InnerWidth
 */
static void windowBorders (struct Rectangle *rect, struct Screen *sc, ULONG flags, struct Gadget *g, UBYTE * title, LONG width, LONG height)
{
    extern struct Rectangle nullrect;

#if USER_BORDERS
    struct GListEnv genv;
    LONG temp;
    struct Rectangle grect;
    struct IBox gadgetbox;

#else
    UWORD gflags;
    UWORD requirement;

#endif

    UWORD activation;

    /* start with screen window border specifications */
    if (!TESTFLAG (flags, BORDERLESS))
    {
	DWB (printf (" there are borders! \n"));
	rect->MinX = sc->WBorLeft;
	rect->MinY = sc->WBorTop;
	rect->MaxX = sc->WBorRight;
	rect->MaxY = sc->WBorBottom;
    }
    else
    {
	*rect = nullrect;
    }

    /* if system top border gadgets or title text,
     * make size dependent on font height (look, I didn't
     * invent this algorithm)
     */
#define wTOPBORDER_STUFF (WINDOWDEPTH|WINDOWDRAG|WINDOWCLOSE|HASZOOM)
    if (TESTFLAG (flags, wTOPBORDER_STUFF) || title)
    {
	rect->MinY += (sc->Font->ta_YSize + 1);
	DWB (printf ("OW: border top is from screen font: %ld\n",
		     rect->MinY));
    }

    /* right or bottom border is expanded to include sizing gadget,
     * if any.
     */
    if (TESTFLAG (flags, WINDOWSIZING))
    {
	UWORD sizingdims[2];

	/* this function returns space needed by sizing gadget,
	 * somewhat larger than sizing gadget itself
	 */
	getSizeGDims (sc, sizingdims);

	/* if not SIZEBBOTTOM, you get SIZEBRIGHT whether you've asked for
	 * it or not (I didn't invent this, either.)
	 */
	if (TESTFLAG (flags, SIZEBBOTTOM))
	{
	    rect->MaxY = sizingdims[1];
	    DWB (printf ("make bottom border equal %ld\n", sizingdims[1]));
	}
	if (TESTFLAG (flags, SIZEBRIGHT) || (!TESTFLAG (flags, SIZEBBOTTOM)))
	{
	    rect->MaxX = sizingdims[0];
	    DWB (printf ("make right border equal %ld\n", sizingdims[0]));
	}
    }

#if USER_BORDERS
    /* expand borders to include border gadgets.
     * 2/12/90: jimm: returns to using gadgetBox().
     *
     * This approach requires that I get a window pointer,
     * so it won't work as an external function called *before*
     * OpenWindow().  A lot more of gadgetRelativity/gadgetBox will
     * need to be duplicated than I did.
     *
     * See the comment in the #else clause.
     */

    /* ZZZ: Note compatibility problem with new look: the borders are
     * not expanded enough to allow the "inner shadow"
     * to get compatible dimensions.
     */
    if (gListDomain (g, w, &genv))
    {
	while (g)
	{
	    gadgetInfo (g, &genv);
	    gadgetBox (g, &genv.ge_GInfo, &gadgetbox);
	    activation = g->Activation;

	    boxToRect (&gadgetbox, &grect);

	    if (activation & LEFTBORDER)
		if (w->BorderLeft < grect.MaxX)
		    w->BorderLeft = grect.MaxX;
	    if (activation & RIGHTBORDER)
		if (w->BorderRight < (temp = w->Width - gadgetbox.Left))
		    w->BorderRight = temp;
	    if (activation & TOPBORDER)
		if (w->BorderTop < grect.MaxY)
		    w->BorderTop = grect.MaxY;
	    if (activation & BOTTOMBORDER)
	    {
		if (w->BorderBottom < (temp = w->Height - gadgetbox.Top))
		    w->BorderBottom = temp;
	    }

	    g = g->NextGadget;
	}
    }

#else

    /* NOTE: this code didn't handle relative dim gadgets
     * well at all.
     */

    /* now expand borders to include user border gadgets.
     * this is a little weird because I want to be able to
     * make this a separate function.  So rather than
     * using the gadgetBox( gadgetinfo ) approach, I take
     * a hands on approach looking at GREL positions that I
     * consider meaningful.  Note that GRELWIDTH for left border
     * is not supported (nor should it be) likewise GRELHEIGHT and top.
     */

    while (g)
    {
	activation = g->Activation;
	gflags = g->Flags;

	DWB (printf ("wB: gadget %lx, activation %lx, flags %lx\n",
		     g, activation, gflags));

	if (TESTFLAG (activation, LEFTBORDER))
	{
	    requirement = g->LeftEdge + g->Width;

	    /* perform relwidth conversion (to g->Width in above sum)
	     * which only works if 'width' is well defined,
	     * so don't use RELWIDTH here if you use WA_InnerWidth
	     */
	    if (TESTFLAG (gflags, GRELWIDTH))
		requirement += width;

	    /** ZZZ: V33 uses requirement-1 !! **/
	    rect->MinX = imax (rect->MinX, requirement);
	}

	if (TESTFLAG (activation, RIGHTBORDER))
	{
	    /* left edge is window rightmost pixel if relleft = 0 */
	    /* MUST be GRELRIGHT if WA_InnerWidth */
	    requirement = TESTFLAG (gflags, GRELRIGHT) ?
		1 - g->LeftEdge :
		width - g->LeftEdge;

	    rect->MaxX = imax (rect->MaxX, requirement);
	}

	if (TESTFLAG (activation, TOPBORDER))
	{
	    requirement = g->TopEdge + g->Height;

	    /* cannot be GRELHEIGHT if WA_InnerWidth */
	    if (TESTFLAG (gflags, GRELHEIGHT))
		requirement += height;

	    /** ZZZ: V33 uses requirement-1 !! **/
	    rect->MinY = imax (rect->MinY, requirement);
	}

	if (TESTFLAG (activation, BOTTOMBORDER))
	{
	    /* MUST be GRELBOTTOM if WA_InnerWidth */
	    requirement = TESTFLAG (gflags, GRELBOTTOM) ?
		1 - g->TopEdge :
		height - g->TopEdge;

	    rect->MaxY = imax (rect->MaxY, requirement);
	}

	g = g->NextGadget;
    }
#endif
}

/*****************************************************************************/

/*** intuition.library/OpenWindowTagList ***/

/*
 * Some new stuff: 4/18/90: all the "get me a screen" functions
 * return a window with the visitor count already bumped.
 * I just leave it that way.  The only non-VISITOR windows
 * are windows open on Custom screens using CUSTOMSCREEN or
 * WA_CustomScreen.
 */
struct Window *OpenWindowTagList (struct NewWindow *nw, struct TagItem *tags)
{

/* In the "old days" (2.04 and early 3.0), NW(field) was a macro
 * that returned 0 if nw was NULL, else returned NW->field.  Now,
 * since we're stack-swapping around OpenWindow() anyways, we
 * create a NULL-filled NewWindow on the stack (48 bytes), and
 * copy nw into that (if one is supplied).  This simplifies
 * the reference, saving much ROM space.
 */

#define NW(field)	(nwcopy.field)

    struct IntuitionBase *IBase = fetchIBase ();

    UWORD failure = 1;

    /* take the pessimistic choice until the very end */

    ULONG flags;		/* note: VISITOR bit used in error bailout */
    UBYTE gadgetres;
    struct Gadget *usergadgets;

    /*struct Gadget *specialgadget; *//* size, then drag	*/
    struct Window *window = NULL;
    struct Screen *sc;		/* used in error bailout	*/
    struct WFlagTrap flagtrap;

    LONG addzoom;
    LONG wbench_not_visitor = FALSE;

/* new ones ...*/
    struct IBox scbox;
    struct IBox wbox;
    UWORD inner;
    UWORD autoadjust;
    struct NewWindow nwcopy;

    DOWF (LONG failnum = 0);

    DB (printf ("OWTagList\n"));
    DB (dumpTagItems (NULL, tags));

    /* If no NewWindow is supplied, WA_AutoAdjust defaults to TRUE */
    autoadjust = TRUE;

    clearWords ((UWORD *) & nwcopy, sizeof (struct NewWindow) / sizeof (WORD));

    if (nw)
    {
	nwcopy = *nw;
	autoadjust = FALSE;
    }

    DOW (printf ("---\nOpenWindowTagList( %lx, %lx )\n", nw, tags));
    DFAIL (if (!kquery ("OW: want me to let it through?")) return (0);)

	DOWF (failnum = GetUserTagData0 (WA_Dummy, tags));
    DOWF (printf ("DEBUG -- failnum %ld\n", failnum));

    DOW (kpause ("check sems?"));

    /* ---------------------------------------------------------------- */
    /*	Accumulate Flags and do consistency checks			*/
    /* ---------------------------------------------------------------- */

    flags = nwcopy.Flags;
    DOW (printf ("flags from newwindow:%lx\n", flags));

    /* additional "bulk" flag initialization from tag WA_Flags */
    flagtrap.Flags = flags | GetUserTagData0 (WA_Flags, tags);
    flagtrap.MoreFlags = 0;

    PackStructureTags (&flagtrap, windowFlagsPackTable, tags);
    flags = flagtrap.Flags;

#define zoomSTUFF	(WINDOWSIZING | WINDOWDEPTH)
    /* guy gets a zoom gadget if:
     * - he asks for one using WA_Zoom, or
     * - he has both sizing and depth gadgets specified
     */
    addzoom = (FindTagItem (WA_Zoom, tags) ||
	       ((flags & zoomSTUFF) == zoomSTUFF));

    /* so I know the zoom gadget is there in getright() */
    if (addzoom)
	SETFLAG (flags, HASZOOM);

    DOW (printf ("OW: flags accumulated from tags %lx\n", flags));

    if (TESTFLAG (flags, BACKDROP))
    {
	/* ZZZ: are these restrictions documented? */
	/* jimm doesn't know why these restrictions are in
	 * here  (I can guess about WINDOWDEPTH).
	 */
	CLEARFLAG (flags, WINDOWSIZING | WINDOWDRAG | WINDOWDEPTH | HASZOOM);
    }

    /* ---------------------------------------------------------------- */
    /*	Determine and obtain screen for new window			*/
    /* ---------------------------------------------------------------- */

    /* all windows except custom windows are VISITORS,
     * and all VISITORS will cause the VisitorCount to
     * be incremented BEFORE they leave here.
     * ZZZ: remove bumpPSNV at EOF
     * ZZZ: unlock pubscreen if owt fails!
     */
    SETFLAG (flags, VISITOR);

    DOWS (printf ("OW: figure out screen, nw: %lx\n", nw));
    /*
     * Custom screen windows are the only non-VISITOR windows
     */
    if (FindTagItem (WA_CustomScreen, tags) || (NW (Type) == CUSTOMSCREEN))
    {
	sc = (struct Screen *) GetUserTagData (WA_CustomScreen, (ULONG) NW (Screen), tags);
	DOWS (printf ("use customscreen, %lx\n", sc));
	CLEARFLAG (flags, VISITOR);	/* I Am Custom	*/
    }

    /* Next are named public screens, which are searched for with
     * optional fallback processing.  A NULL name means "the default"
     * which might be *Workbench."
     */
    else if (FindTagItem (WA_PubScreenName, tags))
    {
	DOWS (printf ("use pubscreen, <%s>\n",
		      GetUserTagData (WA_PubScreenName, "null", tags)));

	/* he specified pubname, ignore type */
	sc = windowPubScreen ((STRPTR) GetUserTagData0 (WA_PubScreenName, tags),
			      GetUserTagData0 (WA_PubScreenFallBack, tags));

    }

    /* Next, check for pubscreen being opened by pointer.
     * The caller *should* have the screen locked already,
     * but that doesn't concern me, since  he'll unlock it
     * when I return, anyway.
     *
     * WARNING: Comma operator in effect:
     *    If you have EITHER WA_PubScreen or nw->Type == PUBLICSCREEN,
     *    will use public screen pointer from WA_PubScreen or nw->Screen,
     *	  respectively.
     *
     * WA_PubScreen takes precedence.
     *
     * Null value of screen pointer in either case means "default".
     */
    else if (
		((sc = (struct Screen *) GetUserTagData (WA_PubScreen, ~0, tags)) !=
		 (struct Screen *) ~0) ||
		(sc = NW (Screen), NW (Type) == PUBLICSCREEN))	/* left-to-right */
    {
	DOWS (printf ("pubscreen pointer\n"));
	if (sc == NULL)
	{
	    DOWS (printf ("pointer is null, use defaultPS\n"));
	    /* but it was null, use default */
	    sc = windowPubScreen (NULL, 0);	/* returns VC++ */
	}
	else
	    /* got legitimate public screen */
	{
	    DOWS (printf ("pointer is %lx\n", sc));
	    bumpPSNVisitor (XSC (sc)->PSNode);
	}
    }

    else
	/* not pubscreen, not customscreen */
    {
	DOWS (printf ("no screen spec (wb or shanghai)\n"));
#if 1
	if (!nw || nw->Type == WBENCHSCREEN)	/* window for wb */
	{
	    DOWS (printf ("window for WB\n"));
	    /* All non-WBENCHWINDOW's are VISITORS	*/
	    if (TESTFLAG (flags, WBENCHWINDOW))	/* wb.library */
	    {
		DOWS (printf ("it's one of dave's windows\n"));
		/* Thinking cap time: WBENCHWINDOW's aren't
		 * visitor windows, but WB doesn't really own
		 * the Workbench Screen.  We want to leave the
		 * visitor lock on for a while, to help keep it
		 * from being closed out from under us (jeez, we're
		 * unsafe already?).
		 *
		 * But we need to clear the VISITOR flag, and decrement
		 * the visitor count after the window is linked in.
		 * Note at OUT: that we don't need to see this VISITOR flag.
		 */
		sc = openSysScreen (WBENCHSCREEN);
		wbench_not_visitor = TRUE;
		CLEARFLAG (flags, VISITOR);
	    }
	    else if (TESTFLAG (IBase->PubScreenFlags, SHANGHAI))
	    {
		DOWS (printf ("it's time to shanghai\n"));
		sc = defaultPubScreen ();
	    }
	    else
	    {
		DOWS (printf ("just use the wb screen\n"));
		sc = openSysScreen (WBENCHSCREEN);
	    }
	}
	else
	    /* "other" SysScreens	*/
	{
	    DOWS (printf ("opening 'other' sys screen.  nw = %lx, type = %lx\n",
			  nw, nw->Type));
	    DOWS (Debug ());
	    /* I know that nw is non-NULL	*/
	    sc = openSysScreen (nw->Type);
	}
#else
	if ((!nw || nw->Type == WBENCHSCREEN) &&
	    TESTFLAG (IBase->PubScreenFlags, SHANGHAI)
	    && !TESTFLAG (flags, WBENCHWINDOW))
	{
	    sc = windowPubScreen (NULL, 0);	/* default pub screen */

	}
	else
	{
	    sc = openSysScreen (NW (Type) ? NW (Type) : WBENCHSCREEN);
	    /* returns VC++ */
	}
#endif
    }

    DOWS (printf ("OW: screen %lx\n\n", sc));

    if (sc == NULL)
	goto OUT;
    DOWF (if (!(--failnum))
	  {
	  printf ("OWFailure: No screen\n"); goto OUT;
	  }
    );
    /* cache gadget resolution for this screen */
    gadgetres = IsHires (sc) ? HIRESGADGET : LOWRESGADGET;

    /* ---------------------------------------------------------------- */
    /*	Create the Window data structure and init some fields		*/
    /* ---------------------------------------------------------------- */
    if ((window = (struct Window *) AllocVec (sizeof (struct XWindow),
					      MEMF_PUBLIC | MEMF_CLEAR)) == 0)
	 goto OUT;

    DOWF (if (!(--failnum))
	  {
	  printf ("OWFailure: Window alloc\n"); goto OUT;
	  }
    );

    XWINDOW (window)->RptQueueLimit =
	GetUserTagData (WA_RptQueue, DEFAULTRPTQUEUE, tags);
    XWINDOW (window)->MouseQueueLimit =
	GetUserTagData (WA_MouseQueue, DEFAULTMOUSEQUEUE, tags);
    /* window->MousePending = 0;	*/

    /* Mark as invalid the cached length of the window title */
    XWINDOW (window)->WinTitleLength = (UWORD) ~ 0;

    {
	struct Screen *s;
	struct Window *w;

	/* Each window is assigned a unique help-group identifier, by default.
	 * you can override it by calling GetUniqueID() yourself, then passing
	 * the result as WA_HelpGroup to each of your windows.  Then, no matter
	 * which one is active, you'll hear help for any of them.
	 *
	 * Alternately, you can pass a reference window as WA_HelpGroupWindow.
	 */
	XWINDOW (window)->HelpGroup = (ULONG) GetUserTagData (WA_HelpGroup,
							      GetUniqueID (), tags);

	if (w = (struct Window *) GetUserTagData0 (WA_HelpGroupWindow, tags))
	{
	    XWINDOW (window)->HelpGroup = XWINDOW (w)->HelpGroup;
	}

	for (s = IBase->FirstScreen; s; s = s->NextScreen)
	{
	    for (w = s->FirstWindow; w; w = w->NextWindow)
	    {
		if (XWINDOW (w)->HelpGroup == XWINDOW (window)->HelpGroup)
		{
		    /* Make help-state for this window match its group's */
		    flagtrap.MoreFlags = w->MoreFlags & WMF_GADGETHELP;

		    goto DONEHELP;
		}
	    }
	}
      DONEHELP:
    }

    DOW (printf ("queue limits gotten\n"));

    window->Flags = flags;
    window->MoreFlags = flagtrap.MoreFlags;

    window->WScreen = sc;
    window->Title = (UBYTE *) GetUserTagData (WA_Title, (ULONG) NW (Title), tags);
    window->ScreenTitle =
	(UBYTE *) GetUserTagData (WA_ScreenTitle, (ULONG) sc->DefaultTitle, tags);
    DST (printf ("OWT: window title <%s> screen title <%s>\n",
		 window->Title, window->ScreenTitle));
    DST (kpause ("point 1"));

    window->DetailPen = GetUserTagData (WA_DetailPen, (ULONG) sc->DetailPen, tags);
    if (nw && ((BYTE) nw->DetailPen != (BYTE) - 1))
	window->DetailPen = nw->DetailPen;

    window->BlockPen = GetUserTagData (WA_BlockPen, (ULONG) sc->BlockPen, tags);
    if (nw && ((BYTE) nw->BlockPen != (BYTE) - 1))
	window->BlockPen = nw->BlockPen;

    /* ---------------------------------------------------------------- */
    /*	Initialize the border dimensions				*/
    /* ---------------------------------------------------------------- */

    usergadgets = (struct Gadget *)
	GetUserTagData (WA_Gadgets, (ULONG) NW (FirstGadget), tags);

    /** calculate window border dimensions in advance **/
    {
	struct Rectangle tmprect;

	/* pass along the dimensions, if I know them yet.
	 * I don't yet know WA_InnerWidth
	 */
	windowBorders (&tmprect, sc, flags, usergadgets, window->Title,
		       GetUserTagData (WA_Width, NW (Width), tags),
		       GetUserTagData (WA_Height, NW (Height), tags)
	    );

	/* copy back to byte slots (should be ubyte?)	*/
	window->BorderLeft = tmprect.MinX;
	window->BorderTop = tmprect.MinY;
	window->BorderRight = tmprect.MaxX;
	window->BorderBottom = tmprect.MaxY;
    }

    /* Gratuitous brace so we can have some locally-scoped variables. */
    {
	/* Peter 10-Jun-91:  Fallback images for CheckMark and
	 * AmigaKey default to the good old traditional ones:
	 */
	struct Image *defCheckMark = IBase->MenuImage[MENUCHECK - MENUCHECK][gadgetres];
	struct Image *defAmigaKey = IBase->MenuImage[AMIGAKEY - MENUCHECK][gadgetres];

	if (TESTFLAG (window->Flags, WFLG_NEWLOOKMENUS))
	{
	    struct DrawInfo *GetScreenDrawInfo ();
	    struct DrawInfo *dri = GetScreenDrawInfo (sc);

	    XWINDOW (window)->MenuBlockPen = dri->dri_Pens[BARBLOCKPEN];
	    XWINDOW (window)->MenuDetailPen = dri->dri_Pens[BARDETAILPEN];
	    defCheckMark = dri->dri_CheckMark;
	    defAmigaKey = dri->dri_AmigaKey;
	    FreeScreenDrawInfo (sc, dri);
	}
	else
	{
	    XWINDOW (window)->MenuBlockPen = window->BlockPen;
	    XWINDOW (window)->MenuDetailPen = window->DetailPen;
	}

	if (!(window->CheckMark = (struct Image *) GetUserTagData (WA_Checkmark, (ULONG) NW (CheckMark), tags)))
	{
	    window->CheckMark = defCheckMark;
	}

	XWINDOW (window)->AmigaIcon =
	    (struct Image *) GetUserTagData (WA_AmigaKey, (ULONG) defAmigaKey, tags);
    }

    /* ---------------------------------------------------------------- */
    /*	System Gadgets							*/
    /* ---------------------------------------------------------------- */

    DOW (printf ("do system gadgets\n"));

    /* add system gadgets, uses Flags and Screen */
    /* identifies drag gadget for Height fixup below	*/
    if (!addSWGadgets (window, TESTFLAG (flags, HASZOOM)))
    {
	DOW (printf ("bailing out of window, no gadgets.\n"));
	goto OUT;
    }

    DOWF (if (!(--failnum))
	  {
	  printf ("OWFailure: addSWGadgets\n"); goto OUT;
	  }
    );

    /* ---------------------------------------------------------------- */
    /*	User Gadgets							*/
    /* ---------------------------------------------------------------- */
    DOW (printf ("do user gadgets\n"));

    ((struct Gadget *) lastThing ((struct Thing *) &window->FirstGadget))->NextGadget =
	usergadgets;

    /* make sure the REQGADGET flag is clear */
    while (usergadgets)
    {
	CLEARFLAG (usergadgets->GadgetType, REQGADGET);
	usergadgets = usergadgets->NextGadget;
    }
    /* note: 'usergadgets' now NULL */

    /* ---------------------------------------------------------------- */
    /*	Work out window dimensions (borders are now known)		*/
    /* ---------------------------------------------------------------- */

    DOW (printf ("do window dimensions\n"));

    /* get screen limits box and default window box
     *
     * when there is no NewWindow, use full screen but exposing
     * the screen bar.
     *
     * ZZZ: perhaps wiser to use DCLIP for screen dimensions,
    , * rather than full screen.
     */

    /* jimm: 3/6/90: screen box for window should be
     * at origin 0,0, which is the origin of the screen
     * in *window* coordinates.
     */

    /* Ugly but compact way of writing:
     *    scbox.Top = scbox.Left = 0;
     *    scbox.Width = sc->Width;
     *    scbox.Height = sc->Height;
     */
    *((ULONG *) (&scbox.Left)) = (ULONG) 0;
    *((ULONG *) (&scbox.Width)) = *((ULONG *) (&sc->Width));

    wbox = scbox;
    /* Default window position is just below the screen bar */
    wbox.Top = sc->BarHeight + 1;

    /* at this point, we have a good default window box,
     * and a screen box useful for autoadjust, with
     * it's left/top at 0,0, i.e., window coordinate system.
     */

    if (nw)
    {
	DOW (printf ("new  window provides initial dims\n"));
	/* override defaults with NewWindow values */
	wbox = *((struct IBox *) &nw->LeftEdge);
    }

    /*
     * override values with tags, if any
     */
    wbox.Left = GetUserTagData (WA_Left, wbox.Left, tags);
    wbox.Top = GetUserTagData (WA_Top, wbox.Top, tags);
    wbox.Width = GetUserTagData (WA_Width, wbox.Width, tags);
    wbox.Height = GetUserTagData (WA_Height, wbox.Height, tags);

    /*
     * new "inner" dimensions processing
     * (you probably want AUTOADJUST).
     */
    if ((inner = GetUserTagData (WA_InnerWidth, -1, tags)) != -1)
    {
	DOW (printf ("doing inner width\n"));
	wbox.Width = inner + window->BorderLeft + window->BorderRight;
    }
    if ((inner = GetUserTagData (WA_InnerHeight, -1, tags)) != -1)
    {
	DOW (printf ("doing inner height\n"));
	wbox.Height = inner + window->BorderTop + window->BorderBottom;
    }

    DOW (dumpBox ("doing dims, ready for adjust", &wbox));
    DOW (dumpBox ("here is screen box", &scbox));

    if (GetUserTagData (WA_AutoAdjust, autoadjust, tags))

    {
#if AUTOADJUST_DCLIP
	/* This code will move windows onto the DClip if possible.
	 * We decided not to go for this code right now...
	 */
	struct IBox dclipbox;

	DOW (printf ("window autoadjust\n"));
	/* Limit window size to fit inside screen's dimensions: */
	wbox.Width = imin (wbox.Width, scbox.Width);
	wbox.Height = imin (wbox.Height, scbox.Height);

	/* Get a box representing the DClip, but in screen coordinates */
	rectToBox (&XSC (sc)->DClip, &dclipbox);
	transfPoint ((struct Point *) &dclipbox, *((struct Point *) &sc->LeftEdge));
	/* boxFit() will try fit the window into the DClip, minimizing
	 * movement.  If the window is bigger than the DClip, then
	 * its upper left will end up the same as the DClip's, which
	 * might put the lower-right off-screen.  So we then call
	 * boxFit() to fit the window into the screen's box.  This
	 * will succeed, since we've limited its size to the screen's
	 * size.
	 */
	boxFit (&wbox, &dclipbox, &wbox);
	boxFit (&wbox, &scbox, &wbox);
#else
	/* dims are easy these days */
	/* ZZZ: maybe want to squeeze into some dclip box? */
	DOW (printf ("window autoadjust\n"));
	wbox.Width = imin (wbox.Width, scbox.Width);
	wbox.Height = imin (wbox.Height, scbox.Height);

	/* move upper left to fit width/height (guaranteed) */
	boxFit (&wbox, &scbox, &wbox);
#endif
    }
    else
	/* fail if pos/dims illegal		*/
    {
	if (wbox.Left < 0
	    || wbox.Top < 0
	    || (((UWORD) wbox.Left) + ((UWORD) wbox.Width)) > scbox.Width
	    || (((UWORD) wbox.Top) + ((UWORD) wbox.Height)) > scbox.Height)
	{
	    DOW (printf ("OW: failing size check\n"));
	    goto OUT;
	}
	DOWF (if (!(--failnum))
	      {
	      printf ("OWFailure: illegal dims\n"); goto OUT;
	      }
	);
    }

    *((struct IBox *) &window->LeftEdge) = wbox;

    /* set limits, no magic */
    window->MaxWidth = window->MinWidth = window->Width;
    window->MaxHeight = window->MinHeight = window->Height;

    WindowLimits (window,
		  GetUserTagData (WA_MinWidth, NW (MinWidth), tags),
		  GetUserTagData (WA_MinHeight, NW (MinHeight), tags),
		  GetUserTagData (WA_MaxWidth, NW (MaxWidth), tags),
		  GetUserTagData (WA_MaxHeight, NW (MaxHeight), tags));

    /* jimm: 6/25/86: initialize mouse coords */
    windowMouse (window);

    window->GZZWidth = window->Width - window->BorderLeft
	- window->BorderRight;
    window->GZZHeight = window->Height - window->BorderTop
	- window->BorderBottom;

    /* ---------------------------------------------------------------- */
    /*	Zoom stuff: after dimensions worked out				*/
    /* ---------------------------------------------------------------- */

    DOW (printf ("do zoom stuff\n"));

#define ZOOM_INIT_MIN	(1)

#if ZOOM_INIT_MIN
    /* if window is already min size, init zoom to max,
     * else, init to min.
     */
    XWINDOW (window)->ZoomBox.Left = window->LeftEdge;
    XWINDOW (window)->ZoomBox.Top = window->TopEdge;

    if ((window->MinWidth == window->Width) &&
	(window->MinHeight == window->Height))
    {
	XWINDOW (window)->ZoomBox.Width = imin (REALBIG, window->MaxWidth);
	XWINDOW (window)->ZoomBox.Height = imin (REALBIG, window->MaxHeight);
    }
    else
    {
	XWINDOW (window)->ZoomBox.Width = window->MinWidth;
	XWINDOW (window)->ZoomBox.Height = window->MinHeight;
    }

#else
    /* new ZOOM stuff 6/88	*/
    /*
     * 3/21/89: add zoominit newwindow tag, also,
     * don't bother to initialize unzoom box.
     */
    XWINDOW (window)->ZoomBox.Left = window->LeftEdge;
    XWINDOW (window)->ZoomBox.Top = window->TopEdge;
    /* convert big unsigned to real big signed	*/
    XWINDOW (window)->ZoomBox.Width = imin (REALBIG, window->MaxWidth);
    XWINDOW (window)->ZoomBox.Height = imin (REALBIG, window->MaxHeight);
#endif

    /* override those carefully laid plans if the guy passed me a box */
    XWINDOW (window)->ZoomBox = *((struct IBox *)
				  GetUserTagData (WA_Zoom, (ULONG) & XWINDOW (window)->ZoomBox, tags));

    /* ---------------------------------------------------------------- */
    /*	complete the job (call IOpenWindow) via state machine 		*/
    /* ---------------------------------------------------------------- */
    DOW (printf ("OW: calling doISM\n"));
    DST (kpause ("call doISM point 2"));
    if (doISM (itOPENWIN, (CPTR) window,
	       GetUserTagData (WA_SuperBitMap, (ULONG) NW (BitMap), tags),
	       GetUserTagData0 (WA_BackFill, tags)) != 0)
    {
	DOW (printf ("OW: doISM returned failure\n"));
	goto OUT;
    }
    DOW (
	    else
	    printf ("OW: doISM succeeded\n"));
    DST (kpause ("doISM done, point 3"));
    failure++;			/* signifies we passed doISM */

    DOWF (if (!(--failnum))
	  {
	  printf ("OWFailure: doISM\n"); goto OUT;
	  }
    );

    /* ---------------------------------------------------------------- */
    /*  Set GFLG_IMAGEDISABLE and do GM_LAYOUT as needed		*/
    /* ---------------------------------------------------------------- */
    /* Take care of initialization issues when gadgets
     * are added to the window.  This includes setting
     * or clearing GFLG_IMAGEDISABLE according to what
     * the gadget's image says, and sending the initial
     * GM_LAYOUT method to GREL_ gadgets.
     */
    initGadgets (window, window->FirstGadget, -1);

    /* ---------------------------------------------------------------- */
    /*  snoop for border/gadget intersections, after dims worked out	*/
    /* ---------------------------------------------------------------- */
    sniffWindowGadgets (window, window->FirstGadget, -1);

    /* Pick up the guy's preferred pointer! */
    SetWindowPointerA (window, tags);

    /* ---------------------------------------------------------------- */
    /*	Do ModifyIDCMP after the state machine				*/
    /* ---------------------------------------------------------------- */

    if (!ModifyIDCMP (window, GetUserTagData (WA_IDCMP, NW (IDCMPFlags), tags)))
    {
	DOW (printf ("OW: ModifyIDCMP returned failure\n"));
	goto OUT;
    }

    DOWF (if (!(--failnum))
	  {
	  printf ("OWFailure: ModifyIDCMP\n"); goto OUT;
	  }
    );

    {
	/* Peter 21-Jan-92: Let's be efficient here.  We need
	 * to get the border and gadgets drawn.
	 *
	 * The key here is that the routine which draws the window
	 * border is already redrawing the border gadgets.  So it is
	 * more efficient and visually pleasing for it to draw ALL the
	 * gadgets in the event it is called for the initial activation
	 * of the window.  This is denoted by an it_Object2 of AWIN_INITIAL.
	 *
	 * The per-state processing of itACTIVATEWIN has been set up so
	 * that even if the token is deferred or fails, the window
	 * border and all gadgets are drawn for us (by calling the
	 * routine fixWindowBorders()).
	 *
	 * If the window isn't being opened active, then we have to
	 * draw all the gadgets here (in inactive state).
	 */

	if (TESTFLAG (flags, ACTIVATE))
	{
	    /* Originally, the itACTIVATEWIN token was invoked
	     * here with sendISM().  However, we may need some assurances
	     * that the borders are drawn before we get back, so doISM()
	     * got used here.  Turns out that this means that OpenWindow()
	     * now blocks if a gadget is held down.  An easy fix for V40
	     * was to make sgadget-state not defer itACTIVATEWIN tokens
	     * having AWIN_INITIAL.  Instead, that state calls
	     * fixWindowBorders(), then lets the token complete and
	     * inserts an asynchronous ActivateWindow() request to
	     * finish the job.  Other deferrers of this token aren't
	     * changed, since they defer for more drastic reasons
	     * or for shorter times.
	     */
	    doISM (itACTIVATEWIN, (ULONG) window, AWIN_INITIAL, NULL);
	}
	else
	{
	    /* We're not activating the window, hence we're responsible
	     * for its initial rendering.  Draw the border and ALL gadgets.
	     */
	    drawEmbossedBorder (window, DRAWGADGETS_ALL);
	}
    }

    /* public screen stuff:
     * can assume if VISITOR is set that screen is legit pubscreen
     */
    if (TESTFLAG (flags, VISITOR))
    {
	if (TESTFLAG (IBase->PubScreenFlags, POPPUBSCREEN))
	{
	    ScreenDepth (window->WScreen, SDEPTH_TOFRONT, NULL);
	}
    }

    failure = 0;		/* we seem to be in the clear */
  OUT:
    DOW (printf ("OW: OUT: failure: %ld, window = %lx\n", failure, window));

    if ((failure) && (window))
    {
	closeWindowCommon (window, failure);
    }

    /* decrement the visitor count if I am bailing out, or if
     * we've got a WBENCHWINDOW, which isn't a visitor
     */
    if ((wbench_not_visitor || (failure && TESTFLAG (flags, VISITOR)))
	&& sc)
    {
	/* no window, so don't count one */
	decPSNVisitor (XSC (sc)->PSNode);
    }

    DOW (printf ("return from OpenWindow = %lx\n", failure ? NULL : window));
    DOW (if (failure)
	 {
	 printf ("failure %lx\n", failure);
	 kpause ("check semaphores?");
	 }
    );

    DST (kpause ("end of openwindowtags"));

    return (failure ? NULL : window);
}

/*****************************************************************************/

/* Intuition relies on itACTIVATEWIN with an Object2 of AWIN_INITIAL to render
 * all the gadgets of the window border.  This is used to avoid drawing
 * the border and border gadgets inactive then immediately active.
 * In the event that itACTIVATEWIN goes through straight away, there is
 * no problem.  However, if the token needs to be deferred or to fail,
 * we need to get the gadgets and border displayed somehow.  In particular,
 * when it's deferred, we want to still get ALL the gadgets up right away.
 */

void fixWindowBorders (void)
{
    struct IntuitionBase *IBase = fetchIBase ();

    if (IT->it_Object2 == AWIN_INITIAL)
    {
	drawEmbossedBorder ((struct Window *) IT->it_Object1, DRAWGADGETS_ALL);
	/* Neuter the part of this token that says the borders
	 * need redrawing
	 */
	IT->it_Object2 = AWIN_NORMAL;
    }
}
