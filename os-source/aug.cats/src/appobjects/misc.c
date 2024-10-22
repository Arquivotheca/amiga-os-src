/* misc.c
 * misc routines for boopsi classes.
 */

#include "appobjectsbase.h"
#include <hardware/blit.h>

UWORD ghost_pattern[] = {0x4444, 0x1111, 0x4444, 0x1111};

#define G(o)	((struct Gadget *)(o))

BOOL CompareRect (struct Rectangle *r1, struct Rectangle *r2)
{
    if ((r1->MinX != r2->MinX) ||
	(r1->MaxX != r2->MaxX) ||
	(r1->MinY != r2->MinY) ||
	(r1->MaxY != r2->MaxY))
    {
	return (FALSE);
    }
    return (TRUE);
}

VOID GhostRectangle (Object *o, struct gpRender *gpr, struct IBox *b)
{
    struct GadgetInfo *gi = gpr->gpr_GInfo;
    UWORD *pens = gi->gi_DrInfo->dri_Pens;
    struct RastPort *rp = gpr->gpr_RPort;

    /* See if we're disabled */
    if (G(o)->Flags & GFLG_DISABLED)
    {
	/* Ghosting is done with the shadow pen */
	SetAPen (rp, pens[SHADOWPEN]);

	/* Allow background to show through */
	SetDrMd (rp, JAM1);

	/* Turn on the special ghosting pattern */
	SetAfPt (rp, ghost_pattern, 2);

	/* Overlay the ghosting over the gadget visuals */
	RectFill (rp, b->Left, b->Top,
		      (b->Left + b->Width - 1),
		      (b->Top + b->Height - 1));

	/* Turn off pattern fill */
	SetAfPt (rp, NULL, 0);
    }
}

struct GadgetInfo *GetGadgetInfo (VOID *msg)
{
    struct gpRender *gpr = (struct gpRender *) msg;
    struct opSet *ops = (struct opSet *) msg;
    struct GadgetInfo *gi;

    switch (gpr->MethodID)
    {
	case OM_NEW:
	case OM_SET:
	case OM_NOTIFY:
	case OM_UPDATE:
	    gi = ops->ops_GInfo;
	    break;

	case GM_HITTEST:
	case GM_RENDER:
	case GM_GOACTIVE:
	case GM_HANDLEINPUT:
	case GM_GOINACTIVE:
	    gi = gpr->gpr_GInfo;
	    break;

	default:
	    gi = NULL;
	    break;
    }
    return (gi);
}

ULONG notifyAttrChanges (Object * o, VOID * ginfo, ULONG flags, ULONG tag1,...)
{
    return DoMethod (o, OM_NOTIFY, &tag1, ginfo, flags);
}

/* Determine if a point is within a rectangle */
ULONG PointInBox (struct IBox * point, struct IBox * box)
{
    if ((point->Left >= box->Left) &&
	(point->Left <= (box->Left + box->Width)) &&
	(point->Top >= box->Top) &&
	(point->Top <= (box->Top + box->Height)))
    {
	return (GMR_GADGETHIT);
    }
    return (0L);
}

#if 1
/* Determine if the mouse point is within a gadget */
BOOL hitGadget (struct Gadget * g, struct GadgetInfo * gi)
{
    WORD x, y;
    BOOL retval = FALSE;
    struct IBox gd;

    x = gi->gi_Window->MouseX;
    y = gi->gi_Window->MouseY;

    /* Get the gadget box */
    gadgetBox (g, &(gi->gi_Domain), &gd, NULL, NULL);

    /*
     * Compare the hit point to see if it is within the gadget rectangle. The
     * proper way to do this is, if the point is in the rectangle, than inquire
     * the gadget for finer hit information.
     */
    if ((x >= gd.Left) && (x <= (gd.Left + gd.Width)) &&
	(y >= gd.Top) && (y <= (gd.Top + gd.Height)))
    {
	retval = TRUE;
    }

    return (retval);
}

#else
/* Determine if the mouse point is within a gadget */
BOOL hitGadget (struct Gadget * g, struct GadgetInfo * gi)
{
    struct Screen *scr = gi->gi_Screen;
    struct Layer_Info *li = &(scr->LayerInfo);
    WORD x = scr->MouseX;
    WORD y = scr->MouseY;
    struct Layer *layer;
    BOOL retval = FALSE;

    /* Get a lock on our layerinfo */
    LockLayerInfo (li);

    /* Find the layer that the hit was in */
    layer = WhichLayer (li, x, y);

    /*
     * Compare the layer that the hit was in to the layer that the gadget
     * resides in
     */
    if (layer == gi->gi_Layer)
    {
	struct IBox gd;

	x = gi->gi_Window->MouseX;
	y = gi->gi_Window->MouseY;

	/* Get the gadget box */
	gadgetBox (g, &(gi->gi_Domain), &gd, NULL, NULL);

	/*
	 * Compare the hit point to see if it is within the gadget rectangle.
	 * The proper way to do this is, if the point is in the rectangle, than
	 * inquire the gadget for finer hit information.
	 */
	if ((x >= gd.Left) && (x <= (gd.Left + gd.Width)) &&
	    (y >= gd.Top) && (y <= (gd.Top + gd.Height)))
	{
	    retval = TRUE;
	}
    }

    /* Release the layerinfo */
    UnlockLayerInfo (li);

    return (retval);
}

#endif

#if 0
/* Get the visual state of a gadget */
LONG GetVisualState (struct Gadget * g, struct GadgetInfo * gi)
{
    LONG retval = IDS_INDETERMINATE;
    WORD high = (g->Flags & GADGHIGHBITS);

    if (g->Flags & GADGDISABLED)
    {
	if (gi->gi_Window->Flags & WINDOWACTIVE)
	    retval = IDS_DISABLED;
	else
	    retval = IDS_INACTIVEDISABLED;
    }
    else if ((g->Flags & SELECTED) && (high != GADGHNONE))
    {
	if (gi->gi_Window->Flags & WINDOWACTIVE)
	    retval = IDS_SELECTED;
	else
	    retval = IDS_INACTIVESELECTED;
    }
    else
    {
	if (gi->gi_Window->Flags & WINDOWACTIVE)
	    retval = IDS_NORMAL;
	else
	    retval = IDS_INACTIVENORMAL;
    }

    return (retval);
}

#endif

/*------------------------------------------------------------------------*/

/* Get the visual state of a gadget */
LONG
GetVisualState (struct Gadget * g, struct GadgetInfo * gi)
{
    WORD high = (g->Flags & GADGHIGHBITS);
    WORD border = g->Activation & 0xF0;
    LONG retval = IDS_INDETERMINATE;

    if (g->Flags & GADGDISABLED)
    {
	if (!(gi->gi_Window->Flags & WINDOWACTIVE) && border)
	    retval = IDS_INACTIVEDISABLED;
	else
	    retval = IDS_DISABLED;
    }
    else if ((g->Flags & SELECTED) && (high != GADGHNONE))
    {
	if (!(gi->gi_Window->Flags & WINDOWACTIVE) && border)
	    retval = IDS_INACTIVESELECTED;
	else
	    retval = IDS_SELECTED;
    }
    else
    {
	if (!(gi->gi_Window->Flags & WINDOWACTIVE) && border)
	    retval = IDS_INACTIVENORMAL;
	else
	    retval = IDS_NORMAL;
    }

    return (retval);
}

#if 0
VOID
gadgetBox (struct Gadget * g, struct IBox * domain, struct IBox * box)
{

    /* Set the 'normal' rectangle */
    box->Left = domain->Left + g->LeftEdge;
    box->Top = domain->Top + g->TopEdge;
    box->Width = g->Width;
    box->Height = g->Height;

    /* Check for relativity */
    if (g->Flags & GRELRIGHT)
	box->Left += domain->Width - 1;
    if (g->Flags & GRELBOTTOM)
	box->Top += domain->Height - 1;
    if (g->Flags & GRELWIDTH)
	box->Width += domain->Width;
    if (g->Flags & GRELHEIGHT)
	box->Height += domain->Height;
}

/* Compute the domain, honoring relative coordinates */
VOID computeDomain (Class * cl, Object * o, struct gpRender * gpr, struct IBox * box)
{
    struct GadgetInfo *gi;

    if (gi = gpr->gpr_GInfo)
    {
	gadgetBox (G (o), &(gi->gi_Domain), box);
    }
}

/*
 * a convenient way to construct and send an OM_NOTIFY message
 */
ULONG
notifyAttrChanges (Object * o, VOID * ginfo, ULONG flags, ULONG tag1,...)
{

    return ((ULONG) DoMethod (o, OM_NOTIFY, &tag1, ginfo, flags));
}

#endif

/*------------------------------------------------------------------------*/

#define	NEED_TEXT	FALSE

#if NEED_TEXT

/* TextExtent that honors the '_' as being a non-printable character (once) */
WORD aTextExtent (struct RastPort * rp, STRPTR string, LONG count, struct TextExtent * te)
{
    WORD retval;
    LONG i;

    /* Get the extent */
    retval = TextExtent (rp, string, count, te);

    /* Search for an '_' sign */
    for (i = 0; i < (count - 1); i++)
    {
	/* Did we find an '_' sign? */
	if (string[i] == '_')
	{
	    /* Get the width of the '_' sign */
	    WORD w = TextLength (rp, "_", 1);

	    /* Adjust the extent */
	    te->te_Width -= w;
	    te->te_Extent.MaxX -= w;
	    break;
	}
    }

    /* Return whatever textextent returned */
    return (retval);
}

#endif

/* Text printer that honors '_' as indicating the keystroke used to trigger
 * the gadget */
UWORD aText (struct RastPort * rp, STRPTR label, WORD left, WORD top)
{
    WORD len = strlen (label);
    WORD i;

    /* Move to the start */
    Move (rp, left, top);

    /* Step through string */
    for (i = 0; i < (len - 1); i++)
    {
	/* Is this an '_' sign? */
	if (label[i] == '_')
	{
	    WORD bot = (top + rp->TxHeight - rp->TxBaseline);
	    WORD mark;

	    /* Draw the first part of the string */
	    Text (rp, label, i);

	    /* Remember where we are */
	    mark = rp->cp_x;

	    /* Draw the underscore */
	    Move (rp, mark, bot);
	    Draw (rp, (mark + TextLength (rp, &label[(i + 1)], 1L) - 2), bot);

#if 0
	    /* Slightly bold the underlined character */
	    Move (rp, mark, top);
	    Text (rp, &label[(i + 1)], 1);

	    /* Return to where we were */
	    Move (rp, (mark + 1), top);
#else
	    /* Return to where we were */
	    Move (rp, mark, top);
#endif

	    /*
	     * Draw the rest of the string.  This one is done last so that the
	     * cursor could be positioned after the text.
	     */
	    Text (rp, &label[(i + 1)], (len - i - 1));

	    /* Return the underlined character */
	    return ((UWORD) label[i]);
	}
    }

    /* Didn't find an '_' sign */
    Text (rp, label, len);

    /* No character... */
    return (0);
}

/* Display a list of IntuiText, underscoring the '_' sign */
UWORD printIText (struct RastPort * rp, struct IntuiText * itext,
		   WORD left, WORD top, WORD pens)
{
    struct TextFont *tmpfp = NULL;
    struct RastPort clonerp;
    UWORD retval = 0;

    /* Copy the RastPort */
    clonerp = *rp;

    /* Step through the list of IntuiText structures */
    while (itext)
    {
	/* Do we honor the IntuiText pens? */
	if (pens)
	{
	    SetAPen (&clonerp, itext->FrontPen);
	    SetBPen (&clonerp, itext->BackPen);
	    SetDrMd (&clonerp, itext->DrawMode);
	}

	/* Are they asking for a font change? */
	if (itext->ITextFont)
	{
	    /* Change the font */
	    tmpfp = ISetFont (&clonerp, itext->ITextFont);
	}

	/* Do we have text to display? */
	if (itext->IText)
	{
	    register UWORD temp;

	    /* Use the text routine that underscores the first '_' sign */
	    if (temp = aText (&clonerp, itext->IText,
			      (left + itext->LeftEdge),
			      (top + itext->TopEdge + clonerp.TxBaseline)))
	    {
		retval = temp;
	    }
	}

	/* Did they ask for a font change? */
	if (itext->ITextFont)
	{
	    /* Close the font that we opened */
	    ICloseFont (tmpfp);

	    /* Restore the default font */
	    SetFont (&clonerp, rp->Font);
	    clonerp.AlgoStyle = rp->AlgoStyle;
	}

	/* Get the next IntuiText */
	itext = itext->NextText;
    }

    /* Set the parent RastPort cursor position */
    rp->cp_x = clonerp.cp_x;
    rp->cp_y = clonerp.cp_y;

    return (retval);
}

VOID gadgetBox (struct Gadget * g, struct IBox * domain,
		 struct IBox * box, struct IBox * cbox, ULONG flags)
{

    /* Set the 'normal' rectangle */
    box->Left = domain->Left + g->LeftEdge;
    box->Top = domain->Top + g->TopEdge;
    box->Width = g->Width;
    box->Height = g->Height;

    /* Check for relativity */
    if (g->Flags & GRELRIGHT)
	box->Left += domain->Width - 1;
    if (g->Flags & GRELBOTTOM)
	box->Top += domain->Height - 1;
    if (g->Flags & GRELWIDTH)
	box->Width += domain->Width;
    if (g->Flags & GRELHEIGHT)
	box->Height += domain->Height;

#if 0
    box->Height = (box->Height < 1) ? 1 : box->Height;
    box->Width = (box->Width < 1) ? 1 : box->Width;

#endif

    /* Make sure we have constraint box */
    if (cbox)
    {
	/* Check minimum horizontal constraints */
	if ((cbox->Left != (~0)) && (box->Width < cbox->Left))
	{

	    /*
	     * Set the width ... this doesn't really work... Would have to set
	     * the gadget's real width, but I don't know how to handle relative
	     * gadgets without forgetting the relativity.
	     */
	    box->Width = cbox->Left;
	}

	/* Check minimum vertical constraints */
	if ((cbox->Top != (~0)) && (box->Height < cbox->Top))
	{
	    /* Set the height ... this doesn't really work... */
	    box->Height = cbox->Top;
	}

	/* Check maximum horizontal constraints */
	if ((cbox->Width != (~0)) && (box->Width > cbox->Width))
	{
	    /* Handle the justification */
	    if (flags & FRAME_HCENTER)
		box->Left += ((box->Width - cbox->Width) / 2);
	    else if (flags & FRAME_RIGHT)
		box->Left += (box->Width - cbox->Width);

	    /* Constrain it */
	    box->Width = cbox->Width;
	}

	/* Check maximum vertical constraints */
	if ((cbox->Height != (~0)) && (box->Height > cbox->Height))
	{
	    /* Handle the justification */
	    if (flags & FRAME_VCENTER)
		box->Top += ((box->Height - cbox->Height) / 2);
	    else if (flags & FRAME_BOTTOM)
		box->Top += (box->Height - cbox->Height);

	    /* Constrain it */
	    box->Height = cbox->Height;
	}
    }
}

/* Compute the domain, honoring relative coordinates */
VOID computeDomain (Class * cl, Object * o, struct gpRender * gpr,
		     struct IBox * box, struct IBox * cbox, ULONG flags)
{
    struct GadgetInfo *gi = gpr->gpr_GInfo;

    gadgetBox (G (o), &(gi->gi_Domain), box, cbox, flags);
}

WORD clamp (WORD min, WORD cur, WORD max)
{

    if (cur < min)
	cur = min;
    else if (cur > max)
	cur = max;
    return (cur);
}

/* Draw an image */
VOID drawImage (struct RastPort * rp, struct Image * im,
		 WORD left, WORD top, LONG state, struct DrawInfo * drinfo)
{
    /* Draw the image */
    DrawImageState (rp, im, left, top, state, drinfo);

    /* Make sure we have something to display */
    if (rp && im && (im->Height > 0 && im->Width > 0))
    {
	/*
	 * Set the ghosting pattern, could probably be done with masks or
	 * templates or some such blitter magic.
	 */
	if ((state == IDS_DISABLED) || (state == IDS_INACTIVEDISABLED))
	{
	    rp->Mask = -1;
	    BNDRYOFF (rp);
	    SetAPen (rp, drinfo->dri_Pens[shadowPen]);

	    SetDrMd (rp, JAM1);
	    SetAfPt (rp, ghost_pattern, 2);

	    /* Fill the region */
	    RectFill (rp,
		      (left + im->LeftEdge),
		      (top + im->TopEdge),
		      (left + im->LeftEdge + im->Width - 1),
		      (top + im->TopEdge + im->Height - 1));

	    /* Turn of pattern fill */
	    SetAfPt (rp, NULL, 0);
	}
    }
}

VOID offsetRect (struct Rectangle * r, int dx, int dy)
{

    r->MinX += dx;
    r->MaxX += dx;
    r->MinY += dy;
    r->MaxY += dy;
}

VOID rectHull (struct Rectangle * big, struct Rectangle * r2)
{

    if (r2->MinX < big->MinX)
	big->MinX = r2->MinX;
    if (r2->MinY < big->MinY)
	big->MinY = r2->MinY;
    if (r2->MaxX > big->MaxX)
	big->MaxX = r2->MaxX;
    if (r2->MaxY > big->MaxY)
	big->MaxY = r2->MaxY;
}

struct IBox *rectToBox (struct IBox * rect, struct IBox * box)
{

    *box = *rect;
    box->Width -= (box->Left - 1);
    box->Height -= (box->Top - 1);
    return (box);
}

struct TextFont *ISetFont (struct RastPort * rp, struct TextAttr * font)
{
    struct TextFont *OpenFont ();
    struct TextFont *fp = NULL;

    if (font && (fp = OpenFont (font)))
    {
	SetFont (rp, fp);
	SetSoftStyle (rp, font->ta_Style, 0xFF);
    }
    return (fp);
}

VOID ICloseFont (struct TextFont * fp)
{

    if (fp)
    {
	CloseFont (fp);
    }
}

RastPortITextExtent (struct RastPort * rp, struct IntuiText * itext, struct TextExtent * te)
{
    struct RastPort clonerp;
    struct TextFont *tmpfp;

    clonerp = *rp;

    tmpfp = ISetFont (&clonerp, itext->ITextFont);

    aTextExtent (&clonerp, itext->IText, strlen (itext->IText), te);

    offsetRect (&te->te_Extent, itext->LeftEdge,
		itext->TopEdge + clonerp.TxBaseline);

    ICloseFont (tmpfp);

    return (0);
}

ITextLayout (rp, it, numitext, box, do_layout, xoffset, yoffset)
    struct RastPort *rp;
    struct IntuiText *it;
    struct IBox *box;
    int numitext;
    BOOL do_layout;
{
    struct TextExtent localte;	/* extent of a single itext item      */
    struct TextExtent textent;	/* extent of grand answer       */

    if (it == NULL || numitext == 0)
    {
	return (FALSE);
    }

    /* start us off     */
    if (do_layout)
    {
	it->LeftEdge = xoffset;
	it->TopEdge = yoffset;
    }

    /* initialize summary text extent with this first one       */
    RastPortITextExtent (rp, it, &textent);
    it = it->NextText;
    numitext--;

    /* second and subsequent itexts     */
    while (it && numitext--)
    {
	if (do_layout)
	{
	    /* put next IText just below the accumulated extent box     */
	    it->LeftEdge = xoffset;

	    /*
	     * regarding leading: using the RastPort's (default) font to do
	     * spacing is OK since we only automatically layout ezreq's, which
	     * are all default font.
	     */
	    it->TopEdge = textent.te_Extent.MaxY +
	      (rp->TxHeight - rp->TxBaseline);
	}

	RastPortITextExtent (rp, it, &localte);

	/* grow the accumulated extent rectangle        */
	rectHull (&textent.te_Extent, &localte.te_Extent);

	it = it->NextText;
    }

    rectToBox ((struct IBox *) & textent.te_Extent, box);
    return (TRUE);
}

/* All nodes on the list must have been allocated using AllocVec() */
VOID FreeExecList (struct List * list, struct Hook * hook)
{

    /* Make sure we have a list */
    if (list)
    {
	/* Make sure there are entries in the list */
	if (list->lh_TailPred != (struct Node *) list)
	{
	    struct Node *node, *nxtnode;

	    /* Let's start at the very beginning... */
	    node = list->lh_Head;

	    /* Continue while there are still nodes to remove */
	    while (nxtnode = node->ln_Succ)
	    {
		/* Remove the node from the list */
		Remove (node);

		/* Do we have a special removal hook? */
		if (hook)
		{
		}

		/* Free the node itself */
		FreeVec ((APTR) node);

		/* Go on to the next node */
		node = nxtnode;
	    }
	}
    }
}

/* Fill in a TagItem */
VOID PrepTag (struct TagItem * tag, ULONG label, LONG data)
{

    tag->ti_Tag = label;
    tag->ti_Data = data;
}

struct TagItem *MakeNewTagList (ULONG data,...)
{
    struct TagItem *tags = (struct TagItem *) & data;

    return (CloneTagItems (tags));
}

/* This function takes a string buffer with text seperated by '|' characters
 * and breaks it up into seperate strings. The addresses of those strings are
 * stored into an array.
 *
 * David Joiner. */
LONG BreakString (STRPTR text, STRPTR * array, LONG maxstrings)
{
    LONG count;
    STRPTR lastbutton = text;

    for (count = 0; count < maxstrings; text++)
    {
	if (*text == '|' || *text == '\0')
	{
	    array[count] = lastbutton;
	    count++;

	    if (*text == '\0')
	    {
		break;
	    }

	    *text = '\0';
	    lastbutton = text + 1;
	}
    }
    return count;
}
