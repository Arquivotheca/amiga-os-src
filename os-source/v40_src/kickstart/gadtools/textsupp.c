/*** textsupp.c ***********************************************************
*
*   textsupp.c	- Text support routines for Gadget Toolkit
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: textsupp.c,v 39.10 92/12/11 10:50:36 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"

/*------------------------------------------------------------------------*/

/* Function Prototypes: */

/* Public: */

/* Internal: */
void intuiTextSize (struct IntuiText *itext, Point *textsize);
void printITextToFit (struct RastPort *rp, struct IntuiText *itext,
    WORD LeftOffset, WORD TopOffset, UWORD Width, UWORD justification,
    struct Rectangle *resultExtent);
void PlaceIntuiText (struct IntuiText *itext, struct Rectangle *extent,
    ULONG where);

void cloneRastPort ( struct RastPort *clonerp, struct RastPort *rp );
static struct TextFont *ISetFont (struct RastPort *rp, struct TextAttr *font);

/*------------------------------------------------------------------------*/

/*/ intuiTextSize()
 *
 * A function to return the Size (width & height) of a single IntuiText.
 *
 * Created:  26-Oct-89, Peter Cherna
 * Modified: 16-May-90, Peter Cherna
 *
 */

void intuiTextSize( struct IntuiText *itext, Point *textsize )
{
    /* !!!  I think we really do want "intuiTextExtent" here, and work
     * with the whole extent.  I don't know what meaning we want to
     * attach to extenders, as far as positioning text strings goes,
     * though
     */
    struct RastPort RPort;
    struct TextFont *font;
    struct TextExtent extent;
    struct Rectangle *rect = &extent.te_Extent;

    DP(("mITE:  Enter\n"));
    InitRastPort(&RPort);	/* system default font */

    DP(("mITE:  Setting font to TextAttr $%lx\n", itext->ITextFont));
    font = ISetFont(&RPort, itext->ITextFont);

    if (itext->IText)
    {
	DP(("mITE:  Calling TextExtent(&RPort $%lx, text '%s', len %ld, extent $%lx)\n",
	    &RPort, itext->IText, strlen(itext->IText), &extent));
	TextExtent(&RPort, itext->IText, (UWORD)strlen(itext->IText), &extent);
	DP(("mITE:  extent: [%ld,%ld]-[%ld,%ld]\n",
	    rect->MinX, rect->MinY,
	    rect->MaxX, rect->MaxY));
	textsize->x = rect->MaxX - rect->MinX + 1;
	textsize->y = rect->MaxY - rect->MinY + 1;
    }
    else
    {
	textsize->x = 0;
	textsize->y = 0;
    }

    DP(("mITE:  Size: [%ld,%ld]\n", textsize->x, textsize->y));
    closeFont(font);
    DP(("mITE:  Done\n"));
}

/*------------------------------------------------------------------------*/

/*/ printITextToFit()
 *
 * A function to print as much of an IntuiText as fits in the constraining
 * bit width.  Affects the RastPort cp_x and cp_y variables only.
 *
 * Created:   9-Nov-89, Peter Cherna (parts cribbed from PrintIText()
 * Modified:  4-Apr-90, Peter Cherna
 *
 */

VOID printITextToFit(struct RastPort *rp, struct IntuiText *itext,
                     WORD LeftOffset, WORD TopOffset, UWORD Width,
                     UWORD justification, struct Rectangle *resultExtent)
{
struct RastPort    clonerp;
UWORD              len;
struct TextExtent  extent;
struct TextExtent  resExtent;
struct TextFont   *font;
STRPTR             str;
WORD               left;
WORD               top;
WORD               xoffset;
WORD               slen;
WORD               plen, rlen;

    if (rp && (str = itext->IText))
    {
        cloneRastPort(&clonerp,rp);

        font    = ISetFont(&clonerp,itext->ITextFont);
        left    = LeftOffset + itext->LeftEdge;
        top     = TopOffset + itext->TopEdge;
        slen    = strlen(str);
        len     = 0;
        xoffset = 0;

        if (justification == GTJ_RIGHT)
        {
            /* below deals with (slen == 0) just fine */
            len     = TextFit(&clonerp,&str[slen-1],slen,&extent,NULL,-1,Width,32767);
            str     = &str[slen - len];
            xoffset = Width - (extent.te_Extent.MaxX - extent.te_Extent.MinX + 1);
        }
        else if (justification == GTJ_CENTER)
        {
            if (slen)
            {
                TextExtent(&clonerp,str,slen,&extent);

/*
                /* !!! These two lines compensate for a bug in graphics.library
                 *     Remove them once TextFit() is fixed. Bug should be
                 *     fixed in graphics > 39.98
                 */
                extent.te_Extent.MinY = -20000;
                extent.te_Extent.MaxY = 20000;
*/

                /* pixel length of whole string */
                plen = extent.te_Extent.MaxX - extent.te_Extent.MinX + 1;

                extent.te_Extent.MaxX -= (plen - Width) / 2;

                rlen = TextFit(&clonerp,str,slen,&resExtent,&extent,1,32767,32767);

                extent.te_Extent.MaxX -= (plen - Width + 1) / 2;

                len = TextFit(&clonerp,&str[rlen-1],rlen,&resExtent,&extent,-1,32767,32767);

                str = &str[rlen - len];

                xoffset = (Width - (resExtent.te_Extent.MaxX - resExtent.te_Extent.MinX + 1)) / 2;
            }
        }
        else  /* GTJ_LEFT */
        {
            len = TextFit(&clonerp,str,slen,&extent,NULL,1,Width,32767);
        }

        SetABPenDrMd(&clonerp,itext->FrontPen,itext->BackPen,itext->DrawMode);
        Move(&clonerp,left+xoffset,top+clonerp.TxBaseline);
        Text(&clonerp,str,len);

        if (resultExtent)
        {
            resultExtent->MinX = left+xoffset;
            resultExtent->MinY = top;
            resultExtent->MaxX = left+xoffset+TextLength(&clonerp,str,len)-1;
            resultExtent->MaxY = top+clonerp.TxHeight-1;
        }

        closeFont(font);

        rp->cp_x = clonerp.cp_x;
        rp->cp_y = clonerp.cp_y;
    }
}


/*------------------------------------------------------------------------*/

/*/ cloneRastPort()
 *
 * Clone a RastPort and set its mask to -1, APen to 1, BPen to zero.
 * Also sets DrawMode to JAM1, AreaPtrn to zero.
 * AOlPen to zero, LinePtrn,
 *
 */

void cloneRastPort( struct RastPort *clonerp, struct RastPort *rp )
{
    *clonerp = *rp;

    /* Set Mask to -1, APen to 1, DrawMode to JAM1,
     * turn off Area Pattern and Line Pattern:
     * And don't forget to clear our friend TmpRas
     */

    SetWriteMask(clonerp,-1);
    SetABPenDrMd(clonerp,1,0,JAM1);
    SetAfPt(clonerp,0,0);
    clonerp->LinePtrn = 0xFFFF;
    clonerp->TmpRas = NULL;
    BNDRYOFF(clonerp);
}

/*------------------------------------------------------------------------*/

/*/ ISetFont()
 *
 * Set font and softstyle with protection against NULLs and failures.
 *
 * Created:  26-Oct-89, Peter Cherna (copied from Intuition)
 * Modified: 26-Oct-89, Peter Cherna
 *
 */

static struct TextFont *ISetFont(struct RastPort *rp, struct TextAttr *attr)
{
struct TextFont *font = NULL;

    if (attr && (font = OpenFont(attr)))
    {
	SetFont(rp,font);
	SetSoftStyle(rp,attr->ta_Style, 0xFF);
    }

    return (font);
}


/*------------------------------------------------------------------------*/

/*/ PlaceIntuiText()
 *
 * Function to position IntuiText with respect to a rectangle representing
 * the extent of the "visual" of the gadget wrt the actual Intuition
 * gadget's upper-left corner.  You must supply a flag to indicate
 * where you'd like the label placed.
 *
 * It is safe to call this with a NULL IntuiText, if you're too lazy to
 * check.
 *
 * Created:  26-Oct-89, Peter Cherna
 * Modified: 06-Nov-89, Peter Cherna
 */

void PlaceSizedIntuiText( struct IntuiText *itext, struct Rectangle *extent,
    ULONG where, Point *textsize )
{
    WORD LeftEdge = 0;
    WORD TopEdge = 0;

    if (itext)
    {
#define PLACETEXT_VCENTER (PLACETEXT_LEFT | PLACETEXT_RIGHT | PLACETEXT_IN)

	if (where & PLACETEXT_VCENTER)
	{
	    /* We must do vertical centering:
	     * Consider the height of the extent (MaxY-MinY+1).  Subtract
	     * the height of the font.  That's what we want to distribute
	     * evenly above and below the font, so divide it by two and
	     * add it to the MinY to position it correctly.  We add an
	     * extra 1 so that if we have an odd amount of space to split,
	     * the extra pixel goes above, since we have some illusion
	     * of extra space below due to descenders.
	     * Note that it is critical to use ">> 1" and not "/ 2",
	     * because /2 rounds positive numbers down and negatives
	     * up (i.e. always towards zero).  We want always rounded
	     * down.
	     *
	     * From that, we can show that:
	     */

	    TopEdge =  (extent->MaxY + extent->MinY + 2 - textsize->y) >> 1;
	}
	else if (where == PLACETEXT_ABOVE)
	{
	    TopEdge = (extent->MinY - textsize->y - INTERHEIGHT);
	}
	else if (where == PLACETEXT_BELOW)
	{
	    TopEdge = (extent->MaxY + INTERHEIGHT);
	}

#define PLACETEXT_HCENTER (PLACETEXT_ABOVE | PLACETEXT_BELOW | PLACETEXT_IN)

	if (where & PLACETEXT_HCENTER)
	{
	    /* We must do horizontal centering: */
	    /* Follow a similar argument to vertical centering: */

	    LeftEdge = (extent->MaxX + extent->MinX + 2 - textsize->x) >> 1;
	}
	else if (where == PLACETEXT_LEFT)
	{
	    LeftEdge = (extent->MinX - textsize->x - INTERWIDTH);
	}
	else if (where == PLACETEXT_RIGHT)
	{
	    LeftEdge = (extent->MaxX + INTERWIDTH);
	}

	itext->LeftEdge += LeftEdge;
	itext->TopEdge += TopEdge;
	/* Because of GT_Underscore, we have to offset any IText->NextText */
	if (itext->NextText)
	{
	    itext->NextText->LeftEdge += LeftEdge;
	    itext->NextText->TopEdge += TopEdge;
	}
    }
}


/*------------------------------------------------------------------------*/


void PlaceIntuiText(struct IntuiText *itext, struct Rectangle *extent,
                    ULONG where )
{
Point textsize;

    if (itext)
	intuiTextSize(itext, &textsize);

    PlaceSizedIntuiText(itext,extent,where,&textsize);
}


/*------------------------------------------------------------------------*/


VOID CombineRects(struct Rectangle *r1, struct Rectangle *r2,
                  struct Rectangle *result)
{
    *result = *r1;

    if (r2->MinX < result->MinX)
        result->MinX = r2->MinX;

    if (r2->MinY < result->MinY)
        result->MinY = r2->MinY;

    if (r2->MaxX > result->MaxX)
        result->MaxX = r2->MaxX;

    if (r2->MaxY > result->MaxY)
        result->MaxY = r2->MaxY;
}


/*------------------------------------------------------------------------*/


/* Combines the gadget's hit box with the area occupied by the gadget text,
 * to form the gadget help box
 */
VOID PlaceHelpBox(struct ExtGadget *gad)
{
struct Rectangle  gadgetrect, textrect, result;
Point             point;
struct IntuiText *itext;

    if ((itext = gad->GadgetText) && itext->IText[0])
    {
        gadgetrect.MinX  = gad->BoundsLeftEdge;
        gadgetrect.MinY  = gad->BoundsTopEdge;
        gadgetrect.MaxX  = gadgetrect.MinX + gad->BoundsWidth - 1;
        gadgetrect.MaxY  = gadgetrect.MinY + gad->BoundsHeight - 1;

        intuiTextSize(itext,&point);
        textrect.MinX  = gad->LeftEdge + itext->LeftEdge;
        textrect.MinY  = gad->TopEdge + itext->TopEdge;
        textrect.MaxX  = textrect.MinX + point.x;
        textrect.MaxY  = textrect.MinY + point.y;

        CombineRects(&gadgetrect,&textrect,&result);

        gad->BoundsLeftEdge = result.MinX;
        gad->BoundsTopEdge  = result.MinY;
        gad->BoundsWidth    = result.MaxX - result.MinX + 1;
        gad->BoundsHeight   = result.MaxY - result.MinY + 1;
    }
}


/*------------------------------------------------------------------------*/


/*/ placeGadgetText()
 *
 * This function incorporates the most common use of PlaceIntuiText(),
 * which is to align the IntuiText against the gadget's box.
 *
 */

void placeGadgetText(struct ExtGadget *gad, ULONG flags, ULONG default_place,
                     struct Rectangle *rect)
{
struct Rectangle gadgetrect;
ULONG            place;

    gadgetrect.MinX = 0;
    gadgetrect.MinY = 0;
    gadgetrect.MaxX = gad->Width - 1;
    gadgetrect.MaxY = gad->Height - 1;

    if ( ! ( place = ( flags & PLACETEXT_MASK ) ) )
	place = default_place;

    PlaceIntuiText(gad->GadgetText, &gadgetrect, place );

    if (rect)
	*rect = gadgetrect;

    PlaceHelpBox(gad);
}


/*------------------------------------------------------------------------*/

#if 0
/* get a pointer to the requester containing the gadget, or NULL if the gadget
 * is not in a requester and is in the main window
 */
struct Requester *GetReq(struct ExtGadget *gad, struct Window *win)
{
struct Requester *req;
struct ExtGadget *g;

    if (win)
        for (req = win->FirstRequest; req; req = req->OlderRequest)
            for (g = req->ReqGadget; g; g = g->NextGadget)
                if (g == gad)
                    return(req);

    return(NULL);
}
#endif

/*------------------------------------------------------------------------*/


/* get a rastport, given a gadget and a window, accounting for requesters */
struct RastPort *GetRP(struct ExtGadget *gad, struct Window *win)
{
#ifdef DOREQUESTERS
struct Requester *req;

    if (req = GetReq(gad,win))
    {
        if (req->ReqLayer)
            return(req->ReqLayer->rp);
    }
    else
#endif

    if (win)
        return(win->RPort);

    return(NULL);
}


/*****************************************************************************/


static USHORT GhostPattern[2] =
{
     0x4444,
     0x1111,
};


VOID Ghost(struct RastPort *rp, UWORD pen, UWORD x0, UWORD y0, UWORD x1, UWORD y1)
{
    SetABPenDrMd(rp,pen,0,JAM1);
    SetAfPt(rp,GhostPattern,1);
    RectFill(rp,x0,y0,x1,y1);
    SetAfPt(rp,NULL,0);
}
