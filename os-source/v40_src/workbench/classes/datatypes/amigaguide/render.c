/* render.c
 *
 */

#include "amigaguidebase.h"
#include <graphics/gfxmacros.h>

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

#define	EG(o)	((struct ExtGadget *)(o))

/*****************************************************************************/

VOID prepareRastPorts (struct AGLib * ag, Class * cl, Object * o)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G(o)->SpecialInfo;
    struct TextFont *font;
    struct ClientData *cd;
    struct NodeData *nd;

    cd = INST_DATA (cl, o);

    if (cd->cd_Window && cd->cd_Window->RPort && (cd->cd_Flags & AGF_NEWSIZE))
    {
	/* clear it */
	cd->cd_Flags &= ~AGF_NEWSIZE;

	/* Get the data pointers */
	nd = INST_DATA (ag->ag_NodeClass, cd->cd_CurrentNode);

	/* Get the font information */
	if (nd->nd_Font)
	    font = nd->nd_Font;
	else if (cd->cd_Font)
	    font = cd->cd_Font;
	else
	    font = cd->cd_GInfo.gi_DrInfo->dri_Font;

	/* Exclusive access to the work rastport */
	ObtainSemaphore (&si->si_Lock);

	/* Initialize text RastPort */
	cd->cd_Render = *cd->cd_Window->RPort;
	SetFont (&cd->cd_Render, font);
	SetABPenDrMd (&cd->cd_Render, cd->cd_GInfo.gi_DrInfo->dri_Pens[TEXTPEN], cd->cd_GInfo.gi_DrInfo->dri_Pens[BACKGROUNDPEN], JAM2);
	cd->cd_Render.Mask = 0xFF;

	/* Initialize control panel RastPort */
	cd->cd_Control = *(&cd->cd_Render);
	SetFont (&cd->cd_Control, cd->cd_GInfo.gi_DrInfo->dri_Font);
	cd->cd_ControlHeight = cd->cd_GInfo.gi_DrInfo->dri_Font->tf_YSize + 3;

	/* Initialize clear RastPort */
	cd->cd_Clear = *(&cd->cd_Render);
	SetAPen (&cd->cd_Clear, cd->cd_GInfo.gi_DrInfo->dri_Pens[BACKGROUNDPEN]);

	/* Initialize highlight RastPort */
	cd->cd_Highlight = *(&cd->cd_Render);
	SetDrMd (&cd->cd_Highlight, COMPLEMENT);
	cd->cd_Highlight.Mask = 0x1;

	/* Initialize temporary RastPort */
	cd->cd_RPort = *(&cd->cd_Render);

	/* OK, they can use it now */
	ReleaseSemaphore (&(si->si_Lock));
    }
}

/*****************************************************************************/

static struct Region *r_installClipRegion (struct AGLib *ag, struct Window *w, struct Layer *l, struct Region *r)
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

static LONG r_renderLine (struct AGLib * ag, struct ClientData *cd, struct RastPort *rp, struct Line *ln, UWORD x, UWORD y, UWORD rmarg)
{
    Move (rp, x, y);
    Text (rp, ln->ln_Text, ln->ln_TextLen);

    if (rp->cp_x < rmarg)
    {
	RectFill (&cd->cd_Clear,
		  rp->cp_x, y - rp->TxBaseline - 1,
		  rmarg, y - rp->TxBaseline + rp->TxHeight);
    }

    return ((LONG) rp->cp_x);
}

/*****************************************************************************/

LONG r_renderLink (struct AGLib *ag, struct ClientData *cd, struct Line *line, LONG x, LONG y, UWORD rmarg)
{
    struct RastPort *trp = &cd->cd_Render;
    LONG lx;

    /* Is this the active line */
    if (line == cd->cd_ActiveLine)
    {
	/* Use cd_Highlight as the rast port */
	trp = &cd->cd_RPort;

	/* Prepare the rastport */
	SetABPenDrMd (trp, cd->cd_DrawInfo->dri_Pens[FILLPEN], cd->cd_DrawInfo->dri_Pens[FILLPEN], JAM2);

	/* Clear that little area between the border and the text */
	RectFill (trp,
		  x + line->ln_Box.Left,
		  y - cd->cd_Render.TxBaseline,
		  x + line->ln_Box.Left + 2,
		  y - cd->cd_Render.TxBaseline + cd->cd_LineHeight - 2);

	/* Prepare for text */
	SetABPenDrMd (trp, cd->cd_DrawInfo->dri_Pens[FILLTEXTPEN], cd->cd_DrawInfo->dri_Pens[FILLPEN], JAM2);
    }
    else
    {
	/* NEW */
	SetABPenDrMd (trp, cd->cd_GInfo.gi_DrInfo->dri_Pens[TEXTPEN], cd->cd_GInfo.gi_DrInfo->dri_Pens[BACKGROUNDPEN], JAM2);

	/* Clear that little area between the border and the text */
	RectFill (&cd->cd_Clear,
		  x + line->ln_Box.Left,
		  y - cd->cd_Render.TxBaseline,
		  x + line->ln_Box.Left + 2,
		  y - cd->cd_Render.TxBaseline + cd->cd_LineHeight - 2);
    }

    /* Text with button */
    lx = r_renderLine (ag, cd, trp, line, x + line->ln_Box.Left + 2, y, rmarg) + 2;

    /* Now draw the button */
    drawbevel (ag, NULL, &cd->cd_RPort, cd->cd_DrawInfo,
	       x + line->ln_Box.Left, y - line->ln_Font->tf_Baseline - 1,
	       line->ln_Box.Width, cd->cd_LineHeight, TRUE);

    return (lx);
}

/*****************************************************************************/

void r_makeNewActive (struct AGLib *ag, Class *cl, Object *o, struct ClientData *cd, struct Line *oln, struct Line *nln, BOOL scroll)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G(o)->SpecialInfo;
    struct RastPort *rp = &cd->cd_RPort;
    struct Rectangle rect;
    struct Region *old_r;
    struct IBox *d;
    struct IBox *b;
    LONG x, y;
    LONG vta;

    ULONG vt = TAG_IGNORE;
    ULONG ht = TAG_IGNORE;
    LONG topv, toph;

    /* Make this the active link */
    cd->cd_ActiveLine = nln;
    cd->cd_ActiveNum  = nln->ln_LinkNum;

    /* Get the domain */
    GetAttr (DTA_Domain, o, (ULONG *)&b);

    /* Calculate top adjustment */
    vta = UMult32 (si->si_TopVert, cd->cd_LineHeight);

    /* Calculate the default x/y */
    x = (LONG) b->Left - si->si_TopHoriz;
    y = (LONG) (cd->cd_Top + cd->cd_RPort.TxBaseline + 1);

    /* Set up the rectangle */
    rect.MinX = b->Left;
    rect.MinY = cd->cd_Top;
    rect.MaxX = b->Left + b->Width - 1;
    rect.MaxY = cd->cd_Top + cd->cd_UsefulHeight - 1;
    ClearRegion (cd->cd_Region);
    OrRectRegion (cd->cd_Region, &rect);
    old_r = r_installClipRegion (ag, cd->cd_Window, rp->Layer, cd->cd_Region);

    /* Update the visuals of the old button */
    r_renderLink (ag, cd, oln, x, y + (LONG)oln->ln_Box.Top - vta, 0);

    /* Update the visuals of the new button */
    r_renderLink (ag, cd, nln, x, y + (LONG) nln->ln_Box.Top - vta, 0);

    /* Restore the previous clipping region */
    r_installClipRegion (ag, cd->cd_Window, cd->cd_RPort.Layer, old_r);

    /* End early if we aren't supposed to scroll */
    if (!scroll)
	return;

    /* Get the current values */
    topv = si->si_TopVert;
    toph = si->si_TopHoriz;

    /* See if the link is visible */
    d = &nln->ln_Box;

    /* See if the button can be seen in the horizontal plane */
    if (x + d->Left + d->Width - 1 < b->Left)
    {
	toph = (LONG) d->Left;
	ht = DTA_TopHoriz;
    }
    else if (x + d->Left > b->Left + b->Width - 1)
    {
	toph = (LONG) d->Left;
	ht = DTA_TopHoriz;
    }

    /* See if the button can be seen in the vertical plane */
    if ((LONG)(d->Top + d->Height - 1) - vta < 0)
    {
	topv = (LONG) d->Top / cd->cd_LineHeight;
	vt = DTA_TopVert;
    }
    else if ((LONG)d->Top - vta > cd->cd_UsefulHeight - cd->cd_LineHeight)
    {
	topv = ((LONG)d->Top / cd->cd_LineHeight) - si->si_VisVert + 1;
	vt = DTA_TopVert;
    }

    if ((vt != TAG_IGNORE) || (ht != TAG_IGNORE))
    {
	/* Tell the world about our new data */
	notifyAttrChanges (ag, o, cd->cd_Window, cd->cd_Requester, NULL,
			   GA_ID, G (o)->GadgetID,
			   ht, toph,
			   vt, topv,
			   TAG_DONE);
    }
}

/*****************************************************************************/

struct Line *r_activateLink (struct AGLib *ag, Class *cl, Object *o, struct ClientData *cd, LONG dir)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G(o)->SpecialInfo;
    struct Line *oln;
    struct Line *ln;

    /* Can't handle this while in layout */
    if (si->si_Flags & DTSIF_LAYOUT)
	return NULL;

    /* If there isn't an active link to begin with, that means that there aren't any links */
    if ((ln = oln = cd->cd_ActiveLine) == NULL)
	return NULL;

    /* Exclusive access to the work rastport */
    ObtainSemaphore (&si->si_Lock);

    /* Find the button to activate */
    if (dir > 0)
    {
	while (TRUE)
	{
	    ln = (struct Line *) ln->ln_Link.mln_Succ;
	    if (ln->ln_Link.mln_Succ == NULL)
		ln = (struct Line *) cd->cd_LineList.mlh_Head;

	    if (ln->ln_Flags & LNF_LINK)
		break;
	}
    }
    else if (dir < 0)
    {
	while (TRUE)
	{
	    ln = (struct Line *) ln->ln_Link.mln_Pred;
	    if (ln->ln_Link.mln_Pred == NULL)
		ln = (struct Line *) cd->cd_LineList.mlh_TailPred;

	    if (ln->ln_Flags & LNF_LINK)
		break;
	}
    }

    /* Make this the new active button */
    r_makeNewActive (ag, cl, o, cd, oln, ln, TRUE);

    /* OK, they can use it now */
    ReleaseSemaphore (&(si->si_Lock));

    return (ln);
}

/*****************************************************************************/

UWORD ghost_pattern[] = {0x4444, 0x1111, 0x4444, 0x1111};

/*****************************************************************************/

static void r_renderControls (struct AGLib * ag, Class * cl, Object * o, struct gpRender *msg)
{
    struct Window *win = msg->gpr_GInfo->gi_Window;
    struct ClientData *cd = INST_DATA (cl, o);
    struct Region *old_r = NULL;
    register struct Controls *c;
    struct Rectangle rect;
    BOOL clip = FALSE;
    struct IBox *box;
    UWORD i;

    GetAttr (DTA_Domain, o, (ULONG *) & box);

    for (i = 0; (i < MAX_CONTROLS) && !clip; i++)
    {
	c = &cd->cd_Controls[i];
	if (((c->c_Left + cd->cd_BoxWidth - 1) > box->Width) ||
	    ((c->c_Top + cd->cd_ControlHeight - 1) > box->Height))
	{
	    clip = TRUE;
	}
    }

    /* SET UP CLIPPING REGION */
    if (clip)
    {
	/* Set up the rectangle */
	rect.MinX = box->Left;
	rect.MinY = box->Top;
	rect.MaxX = box->Left + box->Width - 1;
	rect.MaxY = MIN (box->Top + box->Height - 1, cd->cd_Top);
	ClearRegion (cd->cd_Region);
	OrRectRegion (cd->cd_Region, &rect);
	old_r = r_installClipRegion (ag, win, msg->gpr_RPort->Layer, cd->cd_Region);
    }

    for (i = 0; i < MAX_CONTROLS; i++)
    {
	c = &cd->cd_Controls[i];

	/* Draw the frame around the button */
	drawbevel (ag, cd->cd_Frame, msg->gpr_RPort, msg->gpr_GInfo->gi_DrInfo,
		   box->Left + c->c_Left, c->c_Top, cd->cd_BoxWidth, cd->cd_ControlHeight, TRUE);

	/* Render the text for the button */
	Move (&cd->cd_Control, box->Left + c->c_TLeft, c->c_Top + cd->cd_Control.TxBaseline + 2);
	Text (&cd->cd_Control, c->c_Label, c->c_LabelLen);

	/* Ghost the button if it is disabled */
	if (c->c_Flags & CF_DISABLED)
	{
	    /* Ghost it */
	    SetAfPt (msg->gpr_RPort, ghost_pattern, 2);
	    SetABPenDrMd (msg->gpr_RPort, msg->gpr_GInfo->gi_DrInfo->dri_Pens[TEXTPEN], 0, JAM1);
	    RectFill (msg->gpr_RPort,
		      box->Left + c->c_Left + 2,
		      c->c_Top + 1,
		      box->Left + c->c_Left + cd->cd_BoxWidth - 3,
		      c->c_Top + cd->cd_ControlHeight - 2);
	    SetAfPt (msg->gpr_RPort, NULL, 0);
	}
    }

#if 0
    /* Draw the white separator line */
    SetABPenDrMd (msg->gpr_RPort, msg->gpr_GInfo->gi_DrInfo->dri_Pens[SHADOWPEN], 0, JAM1);
    RectFill (msg->gpr_RPort, box->Left, cd->cd_Top - 2, box->Left + box->Width - 1, cd->cd_Top - 2);

#else
    /* Draw the white separator line */
    SetABPenDrMd (msg->gpr_RPort, msg->gpr_GInfo->gi_DrInfo->dri_Pens[SHINEPEN], 0, JAM1);
    RectFill (msg->gpr_RPort, box->Left, cd->cd_Top - 3, box->Left + box->Width - 1, cd->cd_Top - 3);

    /* Draw the black separator line */
    SetABPenDrMd (msg->gpr_RPort, msg->gpr_GInfo->gi_DrInfo->dri_Pens[SHADOWPEN], 0, JAM1);
    RectFill (msg->gpr_RPort, box->Left, cd->cd_Top - 2, box->Left + box->Width - 1, cd->cd_Top - 2);
#endif

    if (clip)
	r_installClipRegion (ag, win, msg->gpr_RPort->Layer, old_r);
}

/*****************************************************************************/

void r_MakeActiveVisible (struct AGLib *ag, Class *cl, Object *o, struct ClientData *cd, struct IBox *box)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G(o)->SpecialInfo;
    register struct Line *aline = cd->cd_ActiveLine;
    register struct Line *line;
    struct Line *fline = NULL;
    struct Line *lline = NULL;
    register LONG i;
    LONG x, y, vta;
    BOOL visible;

    i = si->si_TopVert;
    line = (struct Line *) cd->cd_LineList.mlh_Head;
    while (line->ln_Link.mln_Succ && i)
    {
	if (line->ln_Flags & LNF_LF)
	    i--;
	line = (struct Line *) line->ln_Link.mln_Succ;
    }

    i = si->si_VisVert;
    x = (LONG) box->Left - si->si_TopHoriz;
    while (line->ln_Link.mln_Succ && i)
    {
	if (line->ln_Flags & LNF_LINK)
	{
	    /* See if it is in the horizontal view area */
	    visible = FALSE;
	    if ((x + line->ln_Box.Left < box->Left + box->Width - 1) &&
		(x + line->ln_Box.Left + line->ln_Box.Width - 1 > box->Left))
		visible = TRUE;

	    if (visible)
	    {
		if (aline == line)
		    return;

		if (fline == NULL)
		    fline = line;
		lline = line;
	    }
	}

	if (line->ln_Flags & LNF_LF)
	    i--;
	line = (struct Line *) line->ln_Link.mln_Succ;
    }

    if (fline && lline)
    {
	struct RastPort *rp = &cd->cd_RPort;
	struct Rectangle rect;
	struct Region *old_r;

	if (cd->cd_ActiveNum < fline->ln_LinkNum)
	{
	    cd->cd_ActiveLine = line = fline;
	    cd->cd_ActiveNum  = fline->ln_LinkNum;
	}
	else if (cd->cd_ActiveNum > lline->ln_LinkNum)
	{
	    cd->cd_ActiveLine = line = lline;
	    cd->cd_ActiveNum  = lline->ln_LinkNum;
	}

	/* Calculate the y position of the active button */
	vta = UMult32 (si->si_TopVert, cd->cd_LineHeight);
	y = (LONG) (cd->cd_Top + cd->cd_RPort.TxBaseline + 1) + (LONG) line->ln_Box.Top - vta;

	/* Set up the rectangle */
	rect.MinX = box->Left;
	rect.MinY = cd->cd_Top;
	rect.MaxX = box->Left + box->Width - 1;
	rect.MaxY = cd->cd_Top + cd->cd_UsefulHeight - 1;
	ClearRegion (cd->cd_Region);
	OrRectRegion (cd->cd_Region, &rect);
	old_r = r_installClipRegion (ag, cd->cd_Window, rp->Layer, cd->cd_Region);

	/* Highlight the active button */
	r_renderLink (ag, cd, line, (LONG) box->Left - si->si_TopHoriz, y, 0);

	/* Restore the previous clipping region */
	r_installClipRegion (ag, cd->cd_Window, rp->Layer, old_r);
    }
}

/*****************************************************************************/

ULONG renderMethod (struct AGLib * ag, Class * cl, Object * o, struct gpRender * msg)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G(o)->SpecialInfo;
    struct ClientData *cd = INST_DATA (cl, o);
    struct GadgetInfo *gi = msg->gpr_GInfo;
    struct Window *win = gi->gi_Window;
    struct Layer *l = win->WLayer;
    struct IBox *box = NULL;
    struct TextFont *font;
    struct Rectangle rect;
    struct Region *old_r;
    struct Hook *oldhook;
    struct NodeData *nd;
    struct Line *line;
    struct IBox *sel;
    UWORD rmarg;
    ULONG style;
    LONG i, j;
    LONG x, y;
    LONG lx;
    WORD cox;
    WORD cw;

    /***************************/
    /* Make sure we can render */
    /***************************/
    prepareRastPorts (ag, cl, o);

    /* Make sure we have a current node */
    if (cd->cd_CurrentNode == NULL)
	return (0);

    if (!(cd->cd_Flags & AGF_LAYOUT))
	return (0);

    if ((si->si_Flags & DTSIF_LAYOUT) || (!AttemptSemaphoreShared (&(si->si_Lock))))
    {
	cd->cd_Flags |= AGF_RENDER;
	return (0);
    }

    if (cd->cd_Flags & AGF_RENDER)
    {
	cd->cd_Flags ^= AGF_RENDER;
	msg->gpr_Redraw = GREDRAW_REDRAW;
    }

    /*************************/
    /* Initialize the domain */
    /*************************/
    nd = INST_DATA (ag->ag_NodeClass, cd->cd_CurrentNode);

    GetAttr (DTA_Domain, o, (ULONG *) & box);

    if (nd->nd_Font)
	font = nd->nd_Font;
    else if (cd->cd_Font)
	font = cd->cd_Font;
    else
	font = cd->cd_GInfo.gi_DrInfo->dri_Font;

    rmarg = box->Left + box->Width;

    /****************************/
    /* Start with a fresh slate */
    /****************************/
    if (msg->gpr_Redraw == GREDRAW_REDRAW)
    {
	/* Clear the box */
	RectFill (&cd->cd_Clear,
		  box->Left, box->Top,	// cd->cd_Top,
		  box->Left + box->Width - 1, box->Top + box->Height - 1);

	/* Draw the controls */
	if (cd->cd_Flags & AGF_CONTROLS)
	    r_renderControls (ag, cl, o, msg);
    }

    /* See if we have an embedded object */
    if (nd->nd_Object)
    {
	DoMethodA (nd->nd_Object, msg);
    }
    else if (msg->gpr_Redraw == GREDRAW_TOGGLE)
    {
	si->si_Flags |= DTSIF_HIGHLIGHT;
	getdtattrs (ag, o, DTA_SelectDomain, &sel, TAG_DONE);
	si->si_Flags &= ~DTSIF_HIGHLIGHT;
	DrawBox (ag, cl, o, &cd->cd_Highlight, box, sel, DBS_DOWN);
    }
    else
    {
	/* Calculate sizes */
	si->si_VisVert      = UDivMod32 (cd->cd_Height, cd->cd_LineHeight);
	cd->cd_UsefulHeight = UMult32 (si->si_VisVert, cd->cd_LineHeight);

	/* if we at least one line and one column to render in... */
	if (cd->cd_UsefulHeight && box->Width)
	{
	    cw = box->Width;
	    cox = 0;

	    /* get a pointer to the first line currently visible */
	    line = (struct Line *) cd->cd_LineList.mlh_Head;
	    i = si->si_TopVert;
	    while (line->ln_Link.mln_Succ && i)
	    {
		if (line->ln_Flags & LNF_LF)
		    i--;
		line = (struct Line *) line->ln_Link.mln_Succ;
	    }

	    /* Default to no drawing */
	    i = j = 0;

	    /* Calculate the default xy */
	    x = (LONG) box->Left - si->si_TopHoriz;
	    y = (LONG) (cd->cd_Top + font->tf_Baseline + 1);

	    /* Give ourselves a NULL backfill hook */
	    oldhook = InstallLayerHook (l, LAYERS_NOBACKFILL);

	    if ((msg->gpr_Redraw == GREDRAW_REDRAW)
		|| (si->si_TopVert >= cd->cd_OTopVert + si->si_VisVert - 1)
		|| ((cd->cd_OTopVert > si->si_VisVert) && (si->si_TopVert <= cd->cd_OTopVert - si->si_VisVert + 1))
		|| (si->si_TopHoriz >= cd->cd_OTopHoriz + si->si_VisHoriz - 1)
		|| ((cd->cd_OTopHoriz > si->si_VisHoriz) && (si->si_TopHoriz <= cd->cd_OTopHoriz - si->si_VisHoriz + 1))
		|| ((si->si_TopHoriz != cd->cd_OTopHoriz) && (si->si_TopVert != cd->cd_OTopVert)))
	    {
		/* the whole display must be redrawn */
		i = si->si_VisVert;
	    }
	    /* VERTICAL */
	    else if (si->si_TopVert < cd->cd_OTopVert)
	    {
		/* we just need to scroll the text */
		ScrollRasterBF (msg->gpr_RPort,
				    0, -(LONG) UMult32 ((cd->cd_OTopVert - si->si_TopVert), cd->cd_LineHeight),
				    box->Left,
				    cd->cd_Top,
				    box->Left + box->Width - 1,
				    cd->cd_Top + cd->cd_UsefulHeight - 1);

		/* indicates what section needs to be redrawn */
		i = cd->cd_OTopVert - si->si_TopVert;
	    }
	    else if (si->si_TopVert > cd->cd_OTopVert)
	    {
		/* we just need to scroll the text */
		ScrollRasterBF (msg->gpr_RPort,
				    0, UMult32 ((si->si_TopVert - cd->cd_OTopVert), cd->cd_LineHeight),
				    box->Left,
				    cd->cd_Top,
				    box->Left + box->Width - 1,
				    cd->cd_Top + cd->cd_UsefulHeight - 1);

		/* indicates what section needs to be redrawn */
		i = si->si_VisVert - (si->si_TopVert - cd->cd_OTopVert);
		while (line->ln_Link.mln_Succ && i)
		{
		    if (line->ln_Flags & LNF_LF)
			i--;
		    line = (struct Line *) line->ln_Link.mln_Succ;
		}

		y = (LONG) (cd->cd_Top + font->tf_Baseline + UMult32 (cd->cd_LineHeight, (si->si_VisVert - (si->si_TopVert - cd->cd_OTopVert))) + 1);
		i = si->si_TopVert - cd->cd_OTopVert;
	    }
	    /* HORIZONTAL */
	    else if (si->si_TopHoriz < cd->cd_OTopHoriz)
	    {
		/* indicates what section needs to be redrawn */
		j = cd->cd_OTopHoriz - si->si_TopHoriz;
		i = si->si_VisVert;
		cw = (WORD) j;

		/* we just need to scroll the text */
		ScrollRasterBF (msg->gpr_RPort,
				    -(j), 0,
				    box->Left,
				    cd->cd_Top,
				    box->Left + box->Width - 1,
				    cd->cd_Top + cd->cd_UsefulHeight - 1);
	    }
	    else if (si->si_TopHoriz > cd->cd_OTopHoriz)
	    {
		/* indicates what section needs to be redrawn */
		j = si->si_TopHoriz - cd->cd_OTopHoriz;
		i = si->si_VisVert;
		cw = (WORD) j;
		cox = box->Width - cw;

		/* we just need to scroll the text */
		ScrollRasterBF (msg->gpr_RPort,
				    j, 0,
				    box->Left,
				    cd->cd_Top,
				    box->Left + box->Width - 1,
				    cd->cd_Top + cd->cd_UsefulHeight - 1);
	    }

	    if (i)
	    {
		/* Make sure a visible node is active */
		r_MakeActiveVisible (ag, cl, o, cd, box);

		/* Set up the rectangle */
		rect.MinX = box->Left + cox;
		rect.MinY = cd->cd_Top;
		rect.MaxX = rect.MinX + cw - 1;
		rect.MaxY = box->Top + box->Height - 1;
		ClearRegion (cd->cd_Region);
		OrRectRegion (cd->cd_Region, &rect);
		old_r = r_installClipRegion (ag, win, cd->cd_Render.Layer, cd->cd_Region);

		/* Make sure all the attributes are properly set. */
		SetABPenDrMd (&cd->cd_Render, line->ln_FgPen, line->ln_BgPen, JAM2);
		SetSoftStyle (&cd->cd_Render, style = line->ln_Style, 0xFF);

		/* render all the lines we need */
		lx = x;
		while (line->ln_Link.mln_Succ && i)
		{
		    /* Set the line attributes */
		    SetABPenDrMd (&cd->cd_Render, line->ln_FgPen, line->ln_BgPen, JAM2);
		    if (style != line->ln_Style)
			SetSoftStyle (&cd->cd_Render, style = line->ln_Style, 0xFF);

		    /* Clear before the line */
		    if (lx < (x + line->ln_Box.Left))
		    {
			RectFill (&cd->cd_Clear,
				  lx, y - cd->cd_Render.TxBaseline - 1,
				  x + line->ln_Box.Left, y - cd->cd_Render.TxBaseline + cd->cd_LineHeight - 2);
		    }

		    if (x + line->ln_Box.Left < box->Left + box->Width)
		    {
			if (line->ln_Flags & LNF_LINK)
			{
			    lx = r_renderLink (ag, cd, line, x, y, rmarg);
			}
			else
			{
			    /* See if we need to clear away any garbage */
			    if (msg->gpr_Redraw != GREDRAW_REDRAW)
			    {
				/* Clear above the text */
				RectFill (&cd->cd_Clear,
				      x + line->ln_Box.Left,
				      y - cd->cd_Render.TxBaseline - 1,
				      x + line->ln_Box.Left + line->ln_Box.Width,
				      y - cd->cd_Render.TxBaseline);

				/* Clear below the text */
				RectFill (&cd->cd_Clear,
				      x + line->ln_Box.Left,
				      y - cd->cd_Render.TxBaseline + cd->cd_LineHeight - 3,
				      x + line->ln_Box.Left + line->ln_Box.Width,
				      y - cd->cd_Render.TxBaseline + cd->cd_LineHeight - 2);
			    }

			    /* Just plain text */
			    lx = r_renderLine (ag, cd, &cd->cd_Render, line, x + line->ln_Box.Left, y, rmarg);
			}
		    }

		    if (line->ln_Flags & LNF_LF)
		    {
			lx = x;
			y += cd->cd_LineHeight;
			i--;
		    }

		    line = (struct Line *) line->ln_Link.mln_Succ;
		}

		/* Restore the soft style */
		SetSoftStyle (&cd->cd_Render, FS_NORMAL, 0xFF);

		/* Remove the clipping region */
		r_installClipRegion (ag, win, cd->cd_Render.Layer, old_r);
	    }

	    /* Restore the original backfill hook */
	    InstallLayerHook (l, oldhook);
	}

	si->si_OTopVert = cd->cd_OTopVert = si->si_TopVert;
	si->si_OTopHoriz = cd->cd_OTopHoriz = si->si_TopHoriz;
    }

    ReleaseSemaphore (&si->si_Lock);
    return (1L);
}
