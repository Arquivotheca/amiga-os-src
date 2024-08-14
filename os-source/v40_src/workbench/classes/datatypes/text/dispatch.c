/* file: 	dispatch.c
 * module:	text.datatype
 *
 */

#include "textbase.h"
#include <graphics/gfxmacros.h>

/*****************************************************************************/

#define	DB(x)	;
#define	DR(x)	;
#define	DL(x)	;

/*****************************************************************************/

ULONG getdtattrs (struct TextLib *txl, Object * o, ULONG data,...)
{
    return (GetDTAttrsA (o, (struct TagItem *) &data));
}

/*****************************************************************************/

ULONG setdtattrs (struct TextLib *txl, Object * o, ULONG data,...)
{
    return (SetDTAttrsA (o, NULL, NULL, (struct TagItem *) &data));
}

/*****************************************************************************/

ULONG notifyAttrChanges (struct TextLib *txl, Object * o, VOID * ginfo, ULONG flags, ULONG tag1,...)
{
    return DoMethod (o, OM_NOTIFY, &tag1, ginfo, flags);
}

/*****************************************************************************/

static struct TextAttr topaz =
{"topaz.font", 8, NULL, NULL};

/*****************************************************************************/

void PrepareFont (struct TextLib *txl, Object * o, struct localData *lod, struct TextAttr *tattr)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct IBox *box;

    if (tattr)
    {
	GetAttr (DTA_Domain, o, (ULONG *) & box);

	if (lod->lod_Font)
	{
	    CloseFont (lod->lod_Font);
	    if (lod->lod_TmpPlane)
		FreeRaster (lod->lod_TmpPlane, lod->lod_TmpWidth, lod->lod_TmpHeight);
	}

	lod->lod_TextAttr = tattr;
	if (!(lod->lod_Font = OpenFont (lod->lod_TextAttr)))
	    lod->lod_Font = OpenFont (&topaz);

	si->si_VertUnit = lod->lod_TmpHeight = lod->lod_Font->tf_YSize;
	si->si_HorizUnit = (lod->lod_Font->tf_Flags & FPF_PROPORTIONAL) ? 1L : lod->lod_Font->tf_XSize;

	lod->lod_TmpWidth = box->Width;
	if (!box->Width)
	    lod->lod_TmpWidth = lod->lod_Font->tf_XSize * 80;

	if (lod->lod_TmpPlane = AllocRaster (lod->lod_TmpWidth, lod->lod_TmpHeight))
	    InitTmpRas (&lod->lod_TmpRas, lod->lod_TmpPlane, RASSIZE (lod->lod_TmpWidth, lod->lod_TmpHeight));
    }
}

/*****************************************************************************/

/* TAB, SPACE, COMMA, ... */
UBYTE delimArray[] = "	 *-,()<>[];\"";

/*****************************************************************************/

/* Methods we support */
ULONG m[] =
{
    OM_NEW,
    OM_GET,
    OM_SET,
    OM_UPDATE,
    OM_DISPOSE,

    GM_LAYOUT,
    GM_HITTEST,
    GM_GOACTIVE,
    GM_HANDLEINPUT,
    GM_RENDER,

#if 0
    DTM_SELECT,
#endif
    DTM_CLEARSELECTED,

    DTM_PRINT,
    DTM_COPY,
    DTM_WRITE,

    ~0,
};

/*****************************************************************************/

/* Inquire attribute of an object */
ULONG getTextDTAttr (struct TextLib *txl, Class * cl, Object * o, struct opGet *msg)
{
    struct localData *lod = INST_DATA (cl, o);

    switch (msg->opg_AttrID)
    {
	case DTA_Methods:
	    *msg->opg_Storage = (ULONG) m;
	    break;

	case DTA_TextAttr:
	    *msg->opg_Storage = (ULONG) lod->lod_TextAttr;
	    break;

	case DTA_TextFont:
	    *msg->opg_Storage = (ULONG) lod->lod_Font;
	    break;

	case TDTA_Buffer:
	    *msg->opg_Storage = (ULONG) lod->lod_Buffer;
	    break;

	case TDTA_BufferLen:
	    *msg->opg_Storage = (ULONG) lod->lod_BufferLen;
	    break;

	case TDTA_LineList:
	    *msg->opg_Storage = (ULONG) & lod->lod_LineList;
	    break;

	case TDTA_WordWrap:
	    *msg->opg_Storage = (lod->lod_Flags & LODF_WORDWRAP) ? TRUE : FALSE;
	    break;

	default:
	    return (DoSuperMethodA (cl, o, msg));
    }

    return (1L);
}

/*****************************************************************************/

/* Set attributes of an object */
ULONG setTextDTAttrs (struct TextLib * txl, Class * cl, Object * o, struct opSet * msg)
{
    struct TagItem *tags = msg->ops_AttrList;
    struct localData *lod;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG refresh = 0L;
    ULONG tidata;

    lod = INST_DATA (cl, o);

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case TDTA_WordDelim:
		lod->lod_WordDelim = (UBYTE *) tidata;
		break;

		/* If this changes, then we need to layout!!! */
	    case TDTA_Buffer:
		lod->lod_Buffer = (STRPTR) tidata;
		break;

		/* If this changes, then we need to layout!!! */
	    case TDTA_BufferLen:
		lod->lod_BufferLen = tidata;
		break;

		/* If this changes, then we need to layout!!! */
	    case TDTA_WordWrap:
		if (tidata)
		    lod->lod_Flags |= LODF_WORDWRAP;
		else
		    lod->lod_Flags &= ~LODF_WORDWRAP;
		break;

	    case DTA_TextAttr:
		PrepareFont (txl, o, lod, (struct TextAttr *) tidata);
	    case DTA_VisibleVert:
	    case DTA_TotalVert:
	    case DTA_VisibleHoriz:
	    case DTA_TotalHoriz:
		refresh = 1L;
		break;

	    case DTA_Sync:
		if (tidata)
		    refresh = 1;
		break;
	}
    }

    return (refresh);
}

/*****************************************************************************/

struct Region *installclipregion (struct TextLib *txl, struct Window *w, struct Layer *l, struct Region *r)
{
    BOOL refresh = FALSE;
    struct Region *or;

    if (w->Flags & WINDOWREFRESH)
    {
	EndRefresh (w, FALSE);
	refresh = TRUE;
    }
    or = InstallClipRegion (l, r);
    if (refresh)
	BeginRefresh (w);
    return (or);
}

/*****************************************************************************/

/* Render a single line of text at a given position */
VOID RenderLine (struct TextLib * txl, struct localData * lod, UWORD x, UWORD y, UWORD w, STRPTR text, ULONG len)
{
    struct RastPort *rp = &lod->lod_Render;

    Move (rp, x, y);
    Text (rp, text, len);
    if (rp->cp_x < w)
    {
	RectFill (&lod->lod_Clear,
		  rp->cp_x, y - rp->TxBaseline,
		  w, y - rp->TxBaseline + rp->TxHeight - 1);
    }
}

/*****************************************************************************/
/*
 * This function performs most of the rendering work needed by our sample. It
 * first locks the window's layer to insure it doesn't get sized during the
 * rendering process. It then looks at the current window size and adjusts
 * its rendering variables in consequence. If the damage parameter is set to
 * TRUE, the routine then proceeds to explicitly erase any area of the
 * display to which we will not be rendering in the rendering loop. This
 * erases any left over characters that could be left if the user sizes the
 * window smaller. Finally, the routine determines which lines of the display
 * need to be updated and goes on to do it.
 */
/*****************************************************************************/

ULONG renderMethod (struct TextLib *txl, Class * cl, Object * o, struct gpRender *msg)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct localData *lod = INST_DATA (cl, o);
    struct GadgetInfo *gi = msg->gpr_GInfo;
    struct Window *win = gi->gi_Window;
    struct Layer *l = win->WLayer;
    LONG redraw = msg->gpr_Redraw;
    struct Region *old_r;
    struct Rectangle rect;
    struct Hook *oldhook;
    struct Line *line;
    struct IBox *box;
    struct IBox *sel;
    WORD curline = 0;
    ULONG style;
    UWORD rmarg;
    WORD x, y;
    LONG i, j;
    WORD cox;
    WORD cw;
    WORD lx;

    WORD ax, ay;
    WORD ex, ey;

    DR (kprintf ("  text renderMethod %ld\n", redraw));
    if ((si->si_Flags & DTSIF_LAYOUT) || !AttemptSemaphoreShared (&(si->si_Lock)))
    {
	lod->lod_Flags |= LODF_REDRAW;
	DR (kprintf ("   layout\n"));
	return 0;
    }

    if (lod->lod_Flags & LODF_REDRAW)
    {
	DR (kprintf ("   lodf_redraw\n"));
	lod->lod_Flags ^= LODF_REDRAW;
	redraw = GREDRAW_REDRAW;
    }

    getdtattrs (txl, o, DTA_SelectDomain, &sel, TAG_DONE);
    box = &lod->lod_Domain;

    DB (kprintf ("o=%08lx, lod=%08lx box=%08lx\n", o, lod, box));

    /* Compute the right margin */
    rmarg = box->Left + box->Width;

    /* The ONLY way we can get a toggle method is if we were highlighted and we want to turn it off */
    if (redraw == GREDRAW_TOGGLE)
    {
	DR (kprintf ("   toggle\n"));
	if (lod->lod_Flags & LODF_HIGHLIGHT)
	{
	    si->si_Flags |= DTSIF_HIGHLIGHT;
	    si->si_Flags &= ~DTSIF_HIGHLIGHT;
	    DrawBox (txl, cl, o, &lod->lod_Highlight, box, sel, DBS_DOWN);
	    lod->lod_Flags &= ~LODF_HIGHLIGHT;
	}
    }
    else
    {
	if (si->si_VisVert > si->si_TotVert)
	{
	    lod->lod_UsefulHeight = UMult32 (si->si_TotVert, si->si_VertUnit);
	    si->si_TopVert = 0;
	}
	else if (si->si_TopVert + si->si_VisVert > si->si_TotVert)
	{
	    si->si_TopVert = (si->si_TotVert - si->si_VisVert);
	    lod->lod_UsefulHeight = UMult32 ((si->si_TotVert - si->si_TopVert), si->si_VertUnit);
	}
	DR (kprintf ("  v %ld,%ld,%ld\n", si->si_TopVert, si->si_VisVert, si->si_TotVert));

	/* if we were called because of damage, we must erase any left over garbage */
	if (redraw == GREDRAW_REDRAW)
	{
	    DR (kprintf ("   redraw\n"));

	    /* erase anything left over on the right side of the window */
	    if (lod->lod_UsefulHeight)
	    {
		DR (kprintf ("clear right %ld,%ld,%ld,%ld\n",
			     (LONG) box->Left, (LONG) box->Top,
			     (LONG) (box->Left + box->Width - 1),
			     (LONG) (box->Top + lod->lod_UsefulHeight - 1)));

		RectFill (&lod->lod_Clear,
			  box->Left,
			  box->Top,
			  box->Left + box->Width - 1,
			  box->Top + lod->lod_UsefulHeight - 1);
	    }

	    /* erase anything left over on the bottom of the window */
	    if ((box->Left < box->Left + box->Width) &&
		(box->Top + lod->lod_UsefulHeight < box->Top + box->Height))
	    {
		DR (kprintf ("clear bottom %ld,%ld,%ld,%ld\n",
			     (LONG) box->Left, (LONG) (box->Top + lod->lod_UsefulHeight),
			     (LONG) (box->Left + box->Width - 1),
			     (LONG) (box->Top + box->Height - 1)));

		RectFill (&lod->lod_Clear,
			  box->Left,
			  box->Top + lod->lod_UsefulHeight,
			  box->Left + box->Width - 1,
			  box->Top + box->Height - 1);
	    }
	}

	/* if we at least one line and one column to render in... */
	if (lod->lod_UsefulHeight && box->Width)
	{
	    cw = box->Width;
	    cox = 0;

	    /* get a pointer to the first line currently visible */
	    DR (kprintf ("find top\n"));
	    i = si->si_TopVert;
	    line = (struct Line *) lod->lod_LineList.mlh_Head;
	    while (line->ln_Link.mln_Succ && i)
	    {
		if (line->ln_Flags & LNF_LF)
		    i--;
		line = (struct Line *) line->ln_Link.mln_Succ;
	    }

	    /* Default to no drawing */
	    DR (kprintf ("do calcs\n"));
	    i = j = 0;

	    /* Calculate the default x, y */
	    x = (LONG) box->Left - (si->si_TopHoriz * si->si_HorizUnit);
	    y = (LONG) box->Top + lod->lod_Font->tf_Baseline;

	    /* Give ourselves a NULL backfill hook */
	    DR (kprintf ("  install nobackfill hook in %08lx, %08lx\n", l, l->LayerInfo));
#if 0
	    LockLayerInfo (l->LayerInfo);
#endif
	    oldhook = InstallLayerHook (l, LAYERS_NOBACKFILL);

	    DR (kprintf ("  scroll\n"));
	    if ((redraw == GREDRAW_REDRAW)
		|| (si->si_TopVert >= si->si_OTopVert + si->si_VisVert - 1)
		|| ((si->si_OTopVert > si->si_VisVert) && (si->si_TopVert <= si->si_OTopVert - si->si_VisVert + 1))
		|| (si->si_TopHoriz >= si->si_OTopHoriz + si->si_VisHoriz - 1)
		|| ((si->si_OTopHoriz > si->si_VisHoriz) && (si->si_TopHoriz <= si->si_OTopHoriz - si->si_VisHoriz + 1))
		|| ((si->si_TopHoriz != si->si_OTopHoriz) && (si->si_TopVert != si->si_OTopVert))
		)
	    {
		/* the whole display must be redrawn */
		i = si->si_VisVert;
		j = si->si_VisHoriz;
	    }
	    else if (si->si_TopVert < si->si_OTopVert)
	    {
		/* we just need to scroll the text */
		ScrollRasterBF (msg->gpr_RPort,
				0, -(LONG) UMult32 ((si->si_OTopVert - si->si_TopVert), si->si_VertUnit),
				box->Left,
				box->Top,
				box->Left + box->Width - 1,
				box->Top + lod->lod_UsefulHeight - 1);

		/* indicates what section needs to be redrawn */
		i = si->si_OTopVert - si->si_TopVert;
	    }
	    else if (si->si_TopVert > si->si_OTopVert)
	    {
		/* we just need to scroll the text */
		ScrollRasterBF (msg->gpr_RPort,
				0, UMult32 ((si->si_TopVert - si->si_OTopVert), si->si_VertUnit),
				box->Left,
				box->Top,
				box->Left + box->Width - 1,
				box->Top + lod->lod_UsefulHeight - 1);

		/* indicates what section needs to be redrawn */
		i = si->si_VisVert - (si->si_TopVert - si->si_OTopVert);
		while (line->ln_Link.mln_Succ && i)
		{
		    if (line->ln_Flags & LNF_LF)
		    {
			curline++;
			i--;
		    }
		    line = (struct Line *) line->ln_Link.mln_Succ;
		}

		y += UMult32 (si->si_VertUnit, (si->si_VisVert - (si->si_TopVert - si->si_OTopVert)));
		i = si->si_TopVert - si->si_OTopVert;
	    }
	    /* HORIZONTAL */
	    else if (si->si_TopHoriz < si->si_OTopHoriz)
	    {
		/* indicates what section needs to be redrawn */
		j = (si->si_OTopHoriz - si->si_TopHoriz) * si->si_HorizUnit;
		i = si->si_VisVert;
		cw = (WORD) j;

		/* we just need to scroll the text */
		ScrollRasterBF (msg->gpr_RPort,
				-(j), 0,
				box->Left,
				box->Top,
				box->Left + box->Width - 1,
				box->Top + lod->lod_UsefulHeight - 1);
	    }
	    else if (si->si_TopHoriz > si->si_OTopHoriz)
	    {
		/* indicates what section needs to be redrawn */
		j = (si->si_TopHoriz - si->si_OTopHoriz) * si->si_HorizUnit;
		i = si->si_VisVert;
		cw = (WORD) j;
		cox = box->Width - cw;

		/* we just need to scroll the text */
		ScrollRasterBF (msg->gpr_RPort,
				j, 0,
				box->Left,
				box->Top,
				box->Left + box->Width - 1,
				box->Top + lod->lod_UsefulHeight - 1);
	    }

	    if (l)
	    {
		ax = ay = ex = ey = -1;
		if (sel)
		{
		    ax = sel->Left - (si->si_TopHoriz * si->si_HorizUnit);
		    ay = sel->Top - si->si_TopVert;
		    ex = sel->Width - (si->si_TopHoriz * si->si_HorizUnit);
		    ey = sel->Height - si->si_TopVert;
		}

		/* Set up the clipping rectangle */
		rect.MinX = box->Left + cox;
		rect.MinY = box->Top;
		rect.MaxX = rect.MinX + cw - 1;
		rect.MaxY = rect.MinY + lod->lod_UsefulHeight - 1;

		/* Apply the clipping region */
		DR (kprintf ("  install clip region\n"));
		ClearRegion (lod->lod_Region);
		OrRectRegion (lod->lod_Region, &rect);
		old_r = installclipregion (txl, win, l, lod->lod_Region);

#if 1
#else
		tags[0].ti_Tag = RPTAG_DrawBounds;
		tags[0].ti_Data = (ULONG) & rect;
		tags[1].ti_Tag = TAG_DONE;
		GetRPAttrsA (msg->gpr_RPort, tags);
		DB (kprintf ("%ld,%ld,%ld,%ld\n", (ULONG) rect.MinX, (ULONG) rect.MinY, (ULONG) rect.MaxX, (ULONG) rect.MaxY));
#endif

		/* Make sure all the attributes are properly set. */
		SetSoftStyle (&lod->lod_Render, style = line->ln_Style, 0xFF);

		/* render all the lines we need */
		DR (kprintf ("  render lines : %08lx, %08lx, %ld\n", line, line->ln_Link.mln_Succ, i));
		lx = x;
		while (line->ln_Link.mln_Succ && i)
		{
#if 1
		    if (1)
#else
		    if ((y >= rect.MinY) && (y <= rect.MaxX))
#endif
		    {
			SetABPenDrMd (&lod->lod_Render, line->ln_FgPen, line->ln_BgPen, JAM2);
			if (style != line->ln_Style)
			    SetSoftStyle (&lod->lod_Render, style = line->ln_Style, 0xFF);

			if (lx < (x + line->ln_XOffset))
			{
			    RectFill (&lod->lod_Clear,
				      lx, y - lod->lod_Render.TxBaseline,
				      x + line->ln_XOffset, y - lod->lod_Render.TxBaseline + si->si_VertUnit - 1);
			}

			if (x + line->ln_XOffset < box->Left + box->Width)
			{
			    RenderLine (txl, lod, x + line->ln_XOffset, y, rmarg, line->ln_Text, line->ln_TextLen);
			    lx = lod->lod_Render.cp_x;
			}

			if (line->ln_Flags & LNF_LF)
			{
			    /* Handle selected lines */
			    if (sel && (curline >= ay) && (curline <= ey))
			    {
				if (ay == ey)
				{
				    RectFill (&lod->lod_Highlight,
					      box->Left + ax,
					      y - lod->lod_Render.TxBaseline,
					      box->Left + ex,
					      y - lod->lod_Render.TxBaseline + si->si_VertUnit - 1);
				}
				else if (curline == ay)
				{
				    RectFill (&lod->lod_Highlight,
					      box->Left + ax,
					      y - lod->lod_Render.TxBaseline,
					      box->Left + box->Width - 1,
					      y - lod->lod_Render.TxBaseline + si->si_VertUnit - 1);
				}
				else if (curline == ey)
				{
				    RectFill (&lod->lod_Highlight,
					      box->Left,
					      y - lod->lod_Render.TxBaseline,
					      box->Left + ex,
					      y - lod->lod_Render.TxBaseline + si->si_VertUnit - 1);
				}
				else
				{
				    RectFill (&lod->lod_Highlight,
					      box->Left,
					      y - lod->lod_Render.TxBaseline,
					      box->Left + box->Width - 1,
					      y - lod->lod_Render.TxBaseline + si->si_VertUnit - 1);
				}
			    }
			    y += si->si_VertUnit;
			    curline++;
			    lx = x;
			    i--;
			}
		    }
		    else
		    {
			if (line->ln_Flags & LNF_LF)
			{
			    y += si->si_VertUnit;
			    curline++;
			    lx = x;
			    i--;
			}
		    }

		    line = (struct Line *) line->ln_Link.mln_Succ;
		}

		/* Restore the clip region */
		installclipregion (txl, win, l, old_r);
	    }
	    else
	    {
		DR (kprintf ("  no layer\n"));
	    }

	    /* Restore the original backfill hook */
	    InstallLayerHook (l, oldhook);
#if 0
	    UnlockLayerInfo (l->LayerInfo);
#endif
	}

	si->si_OTopVert = si->si_TopVert;
	si->si_OTopHoriz = si->si_TopHoriz;
    }

    ReleaseSemaphore (&(si->si_Lock));

    DR (kprintf ("   done\n"));
    return (1L);
}

/*****************************************************************************/

ULONG printMethod (struct TextLib * txl, Class * cl, Object * o, struct dtPrint * msg)
{
    struct localData *lod = INST_DATA (cl, o);
    struct Preferences *prefs;
    struct PrinterData *pd;
    union printerIO *pio;
    LONG pstatus = 0L;
    ULONG retval = 0L;
    struct Line *line;
    UBYTE spaces[80];
    UBYTE attrs[80];
    ULONG style = ~0;
    BYTE fgpen = ~0;
    BYTE bgpen = ~0;
    UBYTE tmp[16];
    ULONG tspace;
    ULONG bsig;
    UWORD i, j;
    UWORD lx;

    if (pio = msg->dtp_PIO)
    {
	pd = (struct PrinterData *) pio->iodrp.io_Device;
	prefs = &pd->pd_Preferences;

	/* Initialize the tab line */
	for (i = 0; i < 80; i++)
	    spaces[i] = ' ';

	/* Compute the tab size */
	tspace = (ULONG) ((lod->lod_Font->tf_CharSpace && lod->lod_Font->tf_CharKern) ? 8 : lod->lod_Font->tf_XSize);

	/* Initialize variables */
	lx = 0;
	i = 1;

	line = (struct Line *) lod->lod_LineList.mlh_Head;
	while (line->ln_Link.mln_Succ && (pstatus == 0L) && !(bsig = CheckSignal (SIGBREAKF_CTRL_C)))
	{

	    /* Perform TAB spacing */

	    if (line->ln_XOffset > lx)
	    {
		/* Spaces from previous character divided by size of space */
		j = (line->ln_XOffset - lx) / tspace;
		spaces[j] = 0;
		pio->ios.io_Length = -1;
		pio->ios.io_Data = (APTR) spaces;
		pio->ios.io_Command = CMD_WRITE;
		DoIO ((struct IORequest *) pio);
		pstatus = (LONG) pio->ios.io_Error;
		spaces[j] = ' ';
	    }

	    if ((pstatus == 0L) &&
		((style != line->ln_Style) || (fgpen != line->ln_FgPen) || (bgpen != line->ln_BgPen)))
	    {
		/* Set font attributes */
		style = line->ln_Style;
		strcpy (attrs, "\033[0m");
		if (style & FSF_BOLD)
		    strcat (attrs, "\033[1m");
		if (style & FSF_ITALIC)
		    strcat (attrs, "\033[3m");
		if (style & FSF_UNDERLINED)
		    strcat (attrs, "\033[4m");

		/* Set foreground color attributes */
		fgpen = line->ln_FgPen;
		sprintf (tmp, "\033[3%ldm", (LONG) fgpen);
		strcat (attrs, tmp);

		/* Set background color attributes */
		bgpen = line->ln_FgPen;
		sprintf (tmp, "\033[4%ldm", (LONG) bgpen);
		strcat (attrs, tmp);
		pio->ios.io_Length = -1;
		pio->ios.io_Data = (APTR) attrs;
		pio->ios.io_Command = CMD_WRITE;
		DoIO ((struct IORequest *) pio);
		pstatus = (LONG) pio->ios.io_Error;
	    }

	    /* Print line */

	    if (pstatus == 0L)
	    {
		pio->ios.io_Length = line->ln_TextLen;
		pio->ios.io_Data = (APTR) line->ln_Text;
		pio->ios.io_Command = CMD_WRITE;
		DoIO ((struct IORequest *) pio);
		pstatus = (LONG) pio->ios.io_Error;

		lx = line->ln_XOffset + line->ln_Width;

		if ((line->ln_Flags & LNF_LF) && (pstatus == 0L))
		{
		    pio->ios.io_Length = 1;
		    pio->ios.io_Data = (APTR) "\n";
		    pio->ios.io_Command = CMD_WRITE;
		    DoIO ((struct IORequest *) pio);
		    pstatus = (LONG) pio->ios.io_Error;

		    lx = 0;
		    i++;

		    if ((i > prefs->PaperLength) && (pstatus == 0L))
		    {
			pio->ios.io_Length = 1;
			pio->ios.io_Data = (APTR) "";
			pio->ios.io_Command = CMD_WRITE;
			DoIO ((struct IORequest *) pio);
			pstatus = (LONG) pio->ios.io_Error;
			i = 1;
		    }
		}
	    }
	    line = (struct Line *) line->ln_Link.mln_Succ;
	}

	/* Show that we tried to print */
	retval = (ULONG) pstatus;
    }

    return retval;
}

/*****************************************************************************/

BOOL writeObject (struct TextLib * txl, APTR handle, Object * o, struct localData * lod, struct GadgetInfo * gi, LONG mode)
{
    struct IFFHandle *iff = (struct IFFHandle *) handle;
    BPTR fh = (BPTR) handle;
    BOOL success = FALSE;
    struct IBox *sel;

    GetAttr (DTA_SelectDomain, o, (ULONG *) & sel);

    if (sel == NULL)
    {
	if (mode == DTWM_IFF)
	{
	    if (OpenIFF (iff, IFFF_WRITE) == 0)
	    {
		if (PushChunk (iff, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN) == 0)
		{
		    if (PushChunk (iff, 0, ID_CHRS, lod->lod_BufferLen) == 0)
			if (WriteChunkBytes (iff, lod->lod_Buffer, lod->lod_BufferLen) == lod->lod_BufferLen)
			    if (PopChunk (iff) == 0)
				success = TRUE;
		    PopChunk (iff);
		}
		CloseIFF (iff);
	    }
	}
	else if (mode == DTWM_RAW)
	{
	    if (Write (fh, lod->lod_Buffer, lod->lod_BufferLen) == lod->lod_BufferLen)
		success = TRUE;
	}

    }
    else
    {
	ULONG s = MIN (lod->lod_AWord, lod->lod_EWord);
	ULONG e = MAX (lod->lod_AWord, lod->lod_EWord);
	ULONG len;

	len = e - s;

	if (mode == DTWM_IFF)
	{
	    if (OpenIFF (iff, IFFF_WRITE) == 0)
	    {
		if (PushChunk (iff, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN) == 0)
		{
		    if (PushChunk (iff, 0, ID_CHRS, len) == 0)
			if (WriteChunkBytes (iff, (STRPTR) s, len) == len)
			    if (PopChunk (iff) == 0)
				success = TRUE;
		    PopChunk (iff);
		}
		CloseIFF (iff);
	    }
	}
	else if (mode == DTWM_RAW)
	{
	    if (Write (fh, (STRPTR) s, len) == len)
		success = TRUE;
	}

	DoMethod (o, DTM_CLEARSELECTED, gi);
    }

    if (!success)
	DisplayBeep (NULL);

    return (success);
}

/*****************************************************************************/

ULONG copyMethod (struct TextLib * txl, Class * cl, Object * o, struct dtGeneral * msg)
{
    struct localData *lod = INST_DATA (cl, o);
    struct IFFHandle *iff;
    BOOL success = FALSE;

    if (iff = AllocIFF ())
    {
	if (iff->iff_Stream = (ULONG) OpenClipboard (0L))
	{
	    InitIFFasClip (iff);
	    success = writeObject (txl, iff, o, lod, msg->dtg_GInfo, DTWM_IFF);
	    CloseClipboard ((struct ClipboardHandle *) iff->iff_Stream);
	}

	FreeIFF (iff);
    }
    return ((ULONG) success);
}

/*****************************************************************************/

ULONG writeMethod (struct TextLib * txl, Class * cl, Object * o, struct dtWrite * msg)
{
    struct localData *lod = INST_DATA (cl, o);
    struct IFFHandle *iff;
    BOOL success = FALSE;

    if (msg->dtw_Mode == DTWM_IFF)
    {
	if (iff = AllocIFF ())
	{
	    if (iff->iff_Stream = msg->dtw_FileHandle)
	    {
		InitIFFasDOS (iff);
		success = writeObject (txl, iff, o, lod, msg->dtw_GInfo, DTWM_IFF);
	    }
	    FreeIFF (iff);
	}
    }
    else if (msg->dtw_Mode == DTWM_RAW)
    {
	success = writeObject (txl, (APTR) msg->dtw_FileHandle, o, lod, msg->dtw_GInfo, DTWM_RAW);
    }

    return ((ULONG) success);
}

/*****************************************************************************/

static ULONG ASM dispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct TextLib *txl = (struct TextLib *) cl->cl_UserData;
    struct localData *lod = INST_DATA (cl, o);
    struct RastPort *rp;
    struct gpRender gpr;
    struct Node *node;
    Object *newobj;
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    DB (kprintf ("\n--- New -----------------------------------------\n"));
	    if (newobj = (Object *) DoSuperMethodA (cl, o, msg))
	    {
		struct DataTypeHeader *dth;
		struct DataType *dtn;
		BOOL success = FALSE;
		STRPTR title;
		UWORD dtype;
		BPTR fh;

		lod = INST_DATA (cl, newobj);

		/* Show that we use scrollraster */
		EG (newobj)->MoreFlags |= GMORE_SCROLLRASTER;

		/* Initialize object data */
		NewList ((struct List *) &lod->lod_LineList);

		/* Establish defaults */
		lod->lod_WordDelim = delimArray;

		/* Prepare the font */
		PrepareFont (txl, newobj, lod, &topaz);

		/* Get the default title */
		title = (STRPTR) GetTagData (DTA_Name, NULL, ((struct opSet *) msg)->ops_AttrList);

		/* Get the handles */
		getdtattrs (txl, newobj,
			    DTA_Handle, (ULONG) & fh,
			    DTA_DataType, (ULONG) & dtn,
			    TAG_DONE);

		/* If there are no handles, then the source must be RAM */
		if ((dtn == NULL) && (fh == NULL))
		{
		    success = TRUE;
		}
		else if (dtn && fh)
		{
		    dth = dtn->dtn_Header;
		    dtype = dth->dth_Flags & DTF_TYPE_MASK;

		    if (dtype == DTF_IFF)
		    {
			struct IFFHandle *iff = (struct IFFHandle *) fh;
			struct StoredProperty *sp;
			struct ContextNode *cn;

			/* We want a name if there is one */
			PropChunk (iff, ID_FTXT, ID_NAME);

			/* IFF Handle is already opened for reading! */
			if (StopChunk (iff, ID_FTXT, ID_CHRS) == 0L)
			{
			    /* Parse the file, stopping at a character chunk */
			    if (ParseIFF (iff, IFFPARSE_SCAN) == 0)
			    {
				/* Get information on the current chunk */
				if ((cn = CurrentChunk (iff)) && (cn->cn_Size > 0L))
				{
				    lod->lod_BufferLen = cn->cn_Size;
				    if (lod->lod_Buffer = AllocVec (lod->lod_BufferLen + 1L, MEMF_CLEAR))
				    {
					if (ReadChunkBytes (iff, lod->lod_Buffer, lod->lod_BufferLen) == lod->lod_BufferLen)
					{
					    if (sp = FindProp (iff, ID_FTXT, ID_NAME))
						title = (STRPTR) sp->sp_Data;
					    success = TRUE;
					}
				    }
				}
			    }
			}
		    }
		    else if (dtype == DTF_MISC)
		    {
			if (fh)
			    success = TRUE;
		    }
		    else
		    {
			if (dtype == DTF_BINARY)
			{
			    DB (kprintf ("binary data, %s\n", dtn->dtn_Header->dth_Name));
			}

			if (Seek (fh, 0L, OFFSET_END) >= 0L)
			{
			    if ((lod->lod_BufferLen = Seek (fh, 0L, OFFSET_BEGINNING)) > 0L)
			    {
				if (lod->lod_Buffer = AllocVec (lod->lod_BufferLen + 1L, MEMF_CLEAR))
				{
				    if (Read (fh, lod->lod_Buffer, lod->lod_BufferLen) == lod->lod_BufferLen)
				    {
					success = TRUE;
				    }
				}
			    }
			}
		    }
		}

		if (success)
		{
		    if ((lod->lod_Region = NewRegion ()) == NULL)
			success = FALSE;
		}

		if (success)
		{
		    struct TagItem tags[2];

		    tags[0].ti_Tag = DTA_ObjName;
		    tags[0].ti_Data = (ULONG) title;
		    tags[1].ti_Tag = TAG_DONE;
		    DoSuperMethod (cl, newobj, OM_SET, tags, NULL);

		    setTextDTAttrs (txl, cl, newobj, (struct opSet *) msg);
		}
		else
		{
		    CoerceMethod (cl, newobj, OM_DISPOSE);
		    newobj = NULL;
		}
	    }

	    retval = (ULONG) newobj;
	    break;

	case OM_GET:
	    retval = getTextDTAttr (txl, cl, o, (struct opGet *) msg);
	    break;

	case OM_NOTIFY:
	    retval = DoSuperMethodA (cl, o, msg);
	    {
		struct TagItem *attrs = ((struct opSet *) msg)->ops_AttrList;
		struct Line *ln;
		UBYTE mp = 0;

		if (GetTagData (DTA_Sync, NULL, attrs))
		{
		    ln = (struct Line *) lod->lod_LineList.mlh_Head;
		    while (ln->ln_Link.mln_Succ)
		    {
			mp = (ln->ln_FgPen > mp) ? ln->ln_FgPen : mp;
			mp = (ln->ln_BgPen > mp) ? ln->ln_BgPen : mp;
			ln = (struct Line *) ln->ln_Link.mln_Succ;
		    }
		    SetMaxPen (&lod->lod_Render, mp);
		    SetMaxPen (&lod->lod_Clear, mp);
		}
	    }
	    break;

	case OM_UPDATE:
	case OM_SET:
	    retval = DoSuperMethodA (cl, o, msg);
	    retval += setTextDTAttrs (txl, cl, o, (struct opSet *) msg);

	    if (retval && (OCLASS (o) == cl))
	    {
		/* Get a pointer to the rastport */
		if (rp = ObtainGIRPort (((struct opSet *) msg)->ops_GInfo))
		{
		    /* Force a redraw */
		    gpr.MethodID = GM_RENDER;
		    gpr.gpr_GInfo = ((struct opSet *) msg)->ops_GInfo;
		    gpr.gpr_RPort = rp;
		    gpr.gpr_Redraw = GREDRAW_UPDATE;
		    DR (kprintf ("  force %ld\n", GREDRAW_UPDATE));
		    DoMethodA (o, &gpr);

		    /* Release the temporary rastport */
		    ReleaseGIRPort (rp);
		}

		/* Clear the return value */
		retval = 0L;
	    }
	    break;

	case GM_RENDER:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    retval += renderMethod (txl, cl, o, (struct gpRender *) msg);
	    break;

	case GM_GOACTIVE:
	case GM_HANDLEINPUT:
	    retval = handleInput (txl, cl, o, (struct gpInput *) msg);
	    break;

	case GM_GOINACTIVE:
	    retval = goinactive (txl, cl, o, (struct gpInput *) msg);
	    break;

	case DTM_CLEARSELECTED:
	    ((struct DTSpecialInfo *) (G (o)->SpecialInfo))->si_Flags &= ~DTSIF_HIGHLIGHT;
	case DTM_SELECT:
	    {
		struct dtSelect *dts = (struct dtSelect *) msg;

		/* Get a pointer to the rastport */
		if (rp = ObtainGIRPort (dts->dts_GInfo))
		{
		    /* Toggle the currently selected line */
		    gpr.MethodID = GM_RENDER;
		    gpr.gpr_GInfo = dts->dts_GInfo;
		    gpr.gpr_RPort = rp;
		    gpr.gpr_Redraw = GREDRAW_TOGGLE;
		    DoMethodA (o, &gpr);

		    /* Release the temporary rastport */
		    ReleaseGIRPort (rp);
		}
		/* Clear the return value */
		retval = 0L;
	    }
	    break;

	case DTM_PRINT:
	    retval = printMethod (txl, cl, o, (struct dtPrint *) msg);
	    break;

	case DTM_COPY:
	    retval = copyMethod (txl, cl, o, (struct dtGeneral *) msg);
	    break;

	case DTM_WRITE:
	    retval = writeMethod (txl, cl, o, (struct dtWrite *) msg);
	    break;

	case GM_LAYOUT:
	case DTM_PROCLAYOUT:
	    /* Make sure the RastPort are set up */
	    {
		struct gpLayout *gpl = (struct gpLayout *) msg;
		struct GadgetInfo *gi = gpl->gpl_GInfo;
		UWORD *pens, pena[NUMDRIPENS + 1];
		struct IBox *box;

		DL (kprintf ("Layout %ld\n", ((struct gpLayout *) msg)->gpl_Initial));

		if (gi && gi->gi_Window)
		{
		    struct RastPort *rp = gi->gi_Window->RPort;

		    /* Initialize pen array */
		    pens = gi->gi_DrInfo->dri_Pens;

		    /* Initialize the RastPorts */
		    DB (kprintf ("copy clear rastport\n"));
		    lod->lod_Clear = *rp;
		    lod->lod_Render = *rp;
		    lod->lod_Highlight = *rp;
		}
		else
		{
		    /* Initialize pen array */
		    pena[BACKGROUNDPEN] = 0;
		    pena[TEXTPEN] = 1;
		    pena[FILLPEN] = 3;
		    pens = pena;

		    /* Initialize the RastPorts */
		    DB (kprintf ("initialize clear rastport\n"));
		    InitRastPort (&lod->lod_Clear);
		    InitRastPort (&lod->lod_Render);
		    InitRastPort (&lod->lod_Highlight);
		}

		/* Set the TmpRas */
		lod->lod_Render.TmpRas = &lod->lod_TmpRas;
		lod->lod_Highlight.TmpRas = &lod->lod_TmpRas;
		lod->lod_Clear.TmpRas = &lod->lod_TmpRas;

		SetABPenDrMd (&lod->lod_Render, pens[TEXTPEN], pens[BACKGROUNDPEN], JAM2);
		SetFont (&lod->lod_Render, lod->lod_Font);
		SetWrMsk (&lod->lod_Render, 0xFF);

		SetABPenDrMd (&lod->lod_Clear, pens[BACKGROUNDPEN], pens[BACKGROUNDPEN], JAM2);
		SetFont (&lod->lod_Clear, lod->lod_Font);
		SetWrMsk (&lod->lod_Render, 0xFF);

		SetABPenDrMd (&lod->lod_Highlight, pens[FILLPEN], pens[BACKGROUNDPEN], COMPLEMENT);
		SetWrMsk (&lod->lod_Highlight, 0x1);
		SetFont (&lod->lod_Highlight, lod->lod_Font);

		lod->lod_Flags |= LODF_REDRAW;
		retval = (ULONG) DoSuperMethodA (cl, o, msg);
		FreeVec (lod->lod_Selected);
		lod->lod_Selected = NULL;

		/* Get the current size */
		getdtattrs (txl, o, DTA_Domain, &box, TAG_DONE);
		lod->lod_Domain = *box;
		si->si_VisVert = UDivMod32 (box->Height, si->si_VertUnit);
		lod->lod_UsefulHeight = UMult32 (si->si_VisVert, si->si_VertUnit);
	    }
	    break;

	    /* Let the superclass handle everything else */
	case DTM_REMOVEDTOBJECT:
	    DL (kprintf ("Remove object\n"));
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;

	case OM_DISPOSE:
	    FreeVec (lod->lod_Buffer);
	    while (node = RemHead ((struct List *) &lod->lod_LineList))
		FreeVec (node);
	    if (lod->lod_Font)
		CloseFont (lod->lod_Font);
	    if (lod->lod_TmpPlane)
		FreeRaster (lod->lod_TmpPlane, lod->lod_TmpWidth, lod->lod_TmpHeight);
	    FreeVec (lod->lod_Selected);
	    DisposeRegion (lod->lod_Region);

	    /* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/

Class *initClass (struct TextLib * txl)
{
    Class *cl;

    if (cl = MakeClass (TEXTDTCLASS, DATATYPESCLASS, NULL, sizeof (struct localData), 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)()) dispatch;
	cl->cl_UserData = (ULONG) txl;
	AddClass (cl);
    }
    return (cl);
}
