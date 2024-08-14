/*** text.c ***************************************************************
*
*   text.c	- Text label "gadgets" for Gadget Toolkit
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: text.c,v 39.12 93/02/11 10:37:06 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"

/*------------------------------------------------------------------------*/

/* Function Prototypes: */

/* Internal: */

struct ExtGadget *CreateTextA (struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
struct ExtGadget *CreateNumberA (struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
void SetTextAttrsA (struct ExtGadget *gad, struct Window *win,
    struct TagItem *taglist);
void SetNumberAttrsA (struct ExtGadget *gad, struct Window *win,
    struct TagItem *taglist);


/*****************************************************************************/


VOID RefreshText(struct ExtGadget *gad, struct Window *win, BOOL refresh)
{
WORD             preserve;
struct Rectangle rect;

    /* We must position the text here, in case a previous text
     * (like the initial one) was NULL.  Of course, we never
     * move its left edge so the easiest thing to do with it
     * is preserve it across the call to PlaceIntuiText()
     */
    preserve                      = TXID(gad)->txid_IText.LeftEdge;
    rect.MinX                     = 0;
    rect.MinY                     = 0;
    rect.MaxX                     = TXID(gad)->txid_TextWidth - 1;
    rect.MaxY                     = gad->Height - 1;
    TXID(gad)->txid_IText.TopEdge = 0;

    /* since we reset the LeftEdge value below, the only thing this call
     * effectively does is vertically center the text within the gadget
     * box
     */
    PlaceIntuiText(&TXID(gad)->txid_IText, &rect, PLACETEXT_IN);
    TXID(gad)->txid_IText.LeftEdge = preserve;

    printITextToFit(GetRP(gad,win), &TXID(gad)->txid_IText,
	            gad->LeftEdge, gad->TopEdge,
                    TXID(gad)->txid_TextWidth,
                    TXID(gad)->txid_Justification,
                    &TXID(gad)->txid_Extent);
}


/*****************************************************************************/


static VOID SetNumberOrTextAttr(struct ExtGadget *gad, struct Window *win,
                                struct TagItem *taglist, BOOL numberKind)
{
struct TagItem   *tag;
struct Rectangle  oldExtent;
struct RastPort  *rp;

    TXID(gad)->txid_Justification  = getGTTagData(GTTX_Justification,TXID(gad)->txid_Justification,taglist);
    TXID(gad)->txid_IText.FrontPen = getGTTagData(GTTX_FrontPen,TXID(gad)->txid_IText.FrontPen,taglist);

    if (tag = findGTTagItem(GTTX_BackPen,taglist))
    {
        TXID(gad)->txid_IText.BackPen  = tag->ti_Data;
        TXID(gad)->txid_IText.DrawMode = JAM2;
    }

    if (numberKind)
    {
        NMID(gad)->nmid_Number = getGTTagData(GTNM_Number,NMID(gad)->nmid_Number,taglist);
        NMID(gad)->nmid_Format = (STRPTR)getGTTagData(GTNM_Format,(ULONG)NMID(gad)->nmid_Format,taglist);
        sprintf(NMID(gad)->nmid_IText.IText,NMID(gad)->nmid_Format,NMID(gad)->nmid_Number);
    }

    if (rp = GetRP(gad,win))
    {
        oldExtent = TXID(gad)->txid_Extent;

        /* We can be sure that if the rectangle is degenerate, it
         * is degenerate in both directions. This would be the result
         * of a NULL IntuiText->IText:
         */
        if (TXID(gad)->txid_IText.DrawMode == JAM1)
        {
            if (oldExtent.MaxX >= oldExtent.MinX)
                EraseRect(rp,oldExtent.MinX,oldExtent.MinY,oldExtent.MaxX,oldExtent.MaxY);
            RefreshText(gad,win,FALSE);
        }
        else
        {
            RefreshText(gad,win,FALSE);
            if (oldExtent.MaxX >= oldExtent.MinX)
                EraseOldExtent(rp,&oldExtent,&TXID(gad)->txid_Extent);
        }
    }
}


/*****************************************************************************/


static struct ExtGadget *CreateNumberOrText(struct ExtGadget *gad, struct NewGadget *ng,
                                         struct TagItem *taglist, BOOL numberKind)
{
STRPTR text;
BOOL   copytext;
ULONG  extraSize;
ULONG  borderTag;

    if (numberKind)
    {
        extraSize = NUMBER_IDATA_SIZE + getGTTagData(GTNM_MaxNumberLen,10,taglist) + 1;
    }
    else
    {
        extraSize = TEXT_IDATA_SIZE;
        text      = (STRPTR)getGTTagData(GTTX_Text,NULL,taglist);
        copytext  = getGTTagData(GTTX_CopyText,FALSE,taglist) && (text != NULL);

        if (copytext)
            extraSize += strlen(text)+1;
    }

    if (gad = CreateGenericBase(gad,ng,extraSize,taglist))
    {
        gad->Flags                     |= GADGIMAGE|GADGHNONE;
        SGAD(gad)->sg_Flags             = SG_EXTRAFREE_DISPOSE;
        SGAD(gad)->sg_Refresh           = RefreshText;
        TXID(gad)->txid_IText.ITextFont = ng->ng_TextAttr;
        TXID(gad)->txid_IText.FrontPen  = VI(ng->ng_VisualInfo)->vi_textPen;
        TXID(gad)->txid_Extent.MinX     = 1;   /* causes an invalid Extent, so first EraseRect() call won't happen */

        if (numberKind)
        {
            borderTag                   = GTNM_Border;
            SGAD(gad)->sg_SetAttrs      = SetNumberAttrsA;
            SGAD(gad)->sg_GetTable      = Number_GetTable;
            NMID(gad)->nmid_Format      = "%ld";
            NMID(gad)->nmid_IText.IText = MEMORY_FOLLOWING(NMID(gad));
        }
        else
        {
            borderTag              = GTTX_Border;
            SGAD(gad)->sg_SetAttrs = SetTextAttrsA;
            SGAD(gad)->sg_GetTable = Text_GetTable;

            if (copytext)
            {
                TXID(gad)->txid_IText.IText = MEMORY_FOLLOWING(TXID(gad));
                strcpy(TXID(gad)->txid_IText.IText,text);
            }
            else
            {
                TXID(gad)->txid_IText.IText = text;
            }
        }

        placeGadgetText(gad,ng->ng_Flags,PLACETEXT_LEFT,NULL);

        /* no clipping by default, set width to something really big */
        TXID(gad)->txid_TextWidth = 9999;

        if (getGTTagData(GTTX_Clipped,FALSE,taglist)
        || (getGTTagData(GTTX_Justification,GTJ_LEFT,taglist) != GTJ_LEFT))
        {
            TXID(gad)->txid_TextWidth = gad->Width;
        }

        if (getGTTagData(borderTag,FALSE,taglist))
        {
            if (!(gad->GadgetRender = (APTR)getBevelImage(0,0,ng->ng_Width,ng->ng_Height,FRAMETYPE_RECESSED|FRAME_BUTTON)))
                return(NULL);

            TXID(gad)->txid_IText.LeftEdge = LEFTTRIM;
            TXID(gad)->txid_TextWidth = gad->Width - LRTRIM;
        }

        SetNumberOrTextAttr(gad,NULL,taglist,numberKind);
    }

    return(gad);
}


/*****************************************************************************/


struct ExtGadget *CreateTextA(struct ExtGadget *gad, struct NewGadget *ng,
                              struct TagItem *taglist)
{
    return(CreateNumberOrText(gad,ng,taglist,FALSE));
}


/*****************************************************************************/


struct ExtGadget *CreateNumberA(struct ExtGadget *gad, struct NewGadget *ng,
                             struct TagItem *taglist)
{
    return(CreateNumberOrText(gad,ng,taglist,TRUE));
}


/*****************************************************************************/


VOID SetTextAttrsA(struct ExtGadget *gad, struct Window *win,
                   struct TagItem *taglist)
{
struct TagItem *tag;

    /* This must be here and not in SetNumberOrTextAttr(), otherwise it
     * would confuse the text pointer when dealing with GTTX_CopyText
     */
    if (tag = findGTTagItem(GTTX_Text,taglist))
	TXID(gad)->txid_IText.IText = (STRPTR)tag->ti_Data;

    SetNumberOrTextAttr(gad,win,taglist,FALSE);
}


/*****************************************************************************/


VOID SetNumberAttrsA(struct ExtGadget *gad, struct Window *win,
                     struct TagItem *taglist)
{
    SetNumberOrTextAttr(gad,win,taglist,TRUE);
}
