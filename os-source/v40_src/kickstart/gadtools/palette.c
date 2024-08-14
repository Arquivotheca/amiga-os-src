/*** palette.c ************************************************************
*
*   palette.c	- Palette gadget routines
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: palette.c,v 39.14 92/10/16 18:22:38 vertex Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "gtinclude.h"

/*------------------------------------------------------------------------*/

/* Function Prototypes: */

/* Internal: */
struct ExtGadget *CreatePaletteA (struct ExtGadget *gad, struct NewGadget *ng,
    struct TagItem *taglist);
BOOL HandlePalette (struct ExtGadget *gad, struct IntuiMessage *imsg);
void SetPaletteAttrsA (struct ExtGadget *gad, struct Window *win,
    struct TagItem *taglist);


#define MINCOLORWIDTH  4
#define MINCOLORHEIGHT 3
/*
#define abs(x) (((x)<0)? -(x): (x))
*/

/*****************************************************************************/


static UWORD GetColorIndex(struct ExtGadget *gad, UWORD boxNum)
{
    if (PAID(gad)->paid_ColorTable)
        return((UWORD)PAID(gad)->paid_ColorTable[boxNum]);

    return((UWORD)(boxNum + PAID(gad)->paid_ColorOffset));
}


/*****************************************************************************/


static UWORD GetBoxNum(struct ExtGadget *gad, UWORD colorIndex)
{
UWORD result;

    if (PAID(gad)->paid_ColorTable)
    {
        result = 0;
        while (PAID(gad)->paid_ColorTable[result] != colorIndex)
            result++;
        return(result);
    }

    return((UWORD)(colorIndex - PAID(gad)->paid_ColorOffset));
}


/*****************************************************************************/


/* Determine which color-box the mouse is over.  Returns the color
 * number
 */
static ULONG WhichBoxNum(struct ExtGadget *gad, struct gpInput *msg)
{
WORD x;
WORD y;

    if ((x = msg->gpi_Mouse.X - 3) < 0)
        x = 0;

    if (x > gad->Width - 6)
        x = gad->Width - 6;

    if ((y = msg->gpi_Mouse.Y - 2) < 0)
        y = 0;

    if (y > gad->Height - 4)
        y = gad->Height - 4;

    return((ULONG)( (x / PAID(gad)->paid_EachWidth) + PAID(gad)->paid_XCount
             * (y / PAID(gad)->paid_EachHeight)));
}


/*****************************************************************************/


static VOID SelectBox(struct RastPort *rp, struct ExtGadget *gad, UWORD boxNum,
                      BOOL selected, BOOL drawSquare)
{
UWORD         x0,y0,x1,y1,w,h;
struct Image *bevelImage;

    if (boxNum == ~0)
        return;

    w  = PAID(gad)->paid_EachWidth;
    h  = PAID(gad)->paid_EachHeight;
    x0 = gad->LeftEdge + w * (boxNum % PAID(gad)->paid_XCount) + 2;
    y0 = gad->TopEdge + h * (boxNum / PAID(gad)->paid_XCount) + 1;
    x1 = x0 + w;
    y1 = y0 + h;

    if (selected)
    {
        PAID(gad)->paid_SelectedBox = boxNum;
        if (bevelImage = getBevelImage(0,0,w+1,h+1,FRAME_BUTTON))
        {
            DrawImageState(rp,bevelImage,x0,y0,IDS_SELECTED,PAID(gad)->paid_DrawInfo);
            DisposeObject(bevelImage);
        }

        if ((x1 - x0 > MINCOLORWIDTH+6) && (y1 - y0 > MINCOLORHEIGHT+4))
        {
            EraseRect(rp,x0+2,y0+1,x1-2,y0+1);
            EraseRect(rp,x0+2,y0+1,x0+2,y1-1);
            EraseRect(rp,x1-2,y0+1,x1-2,y1-1);
            EraseRect(rp,x0+2,y1-1,x1-2,y1-1);
        }
    }
    else
    {
        EraseRect(rp,x0,y0,x1,y0);
        EraseRect(rp,x0,y0,x0+1,y1);
        EraseRect(rp,x1-1,y0,x1,y1);
        EraseRect(rp,x0,y1,x1,y1);
        drawSquare = TRUE;
    }

    if (drawSquare)
    {
        setAPen(rp,GetColorIndex(gad,boxNum));
        RectFill(rp, x0+2, y0+1, x1-2, y1-1);
    }
}


/*****************************************************************************/


static VOID RenderPalette(struct RastPort *rp, struct ExtGadget *gad)
{
WORD count;
WORD selected;

    DrawImageState(rp,gad->GadgetRender,gad->LeftEdge,gad->TopEdge,IDS_NORMAL,PAID(gad)->paid_DrawInfo);
    PrintIText(rp,gad->GadgetText,gad->LeftEdge,gad->TopEdge);

    selected = -1;
    if (PAID(gad)->paid_Indicator)
        selected = GetBoxNum(gad,PAID(gad)->paid_Color);

    for (count = 0; count < PAID(gad)->paid_Count; count++)
        SelectBox(rp,gad,count,FALSE,TRUE);
    SelectBox(rp,gad,selected,TRUE,FALSE);

    if (gad->Flags & GADGDISABLED)
        Ghost(rp,PAID(gad)->paid_DrawInfo->dri_Pens[BLOCKPEN],
              gad->LeftEdge,
              gad->TopEdge,
              gad->LeftEdge+gad->Width-1,
              gad->TopEdge+gad->Height-1);
}


/*****************************************************************************/


static ULONG HandlePaletteInput(struct ExtGadget *gad, struct gpInput *msg)
{
ULONG              newBox, oldBox;
struct RastPort   *rp;
struct InputEvent *ie = msg->gpi_IEvent;
ULONG              result = GMR_MEACTIVE;
UWORD              icode;

    if (ie->ie_Class == IECLASS_RAWMOUSE)
    {
        icode = ie->ie_Code;

	oldBox = PAID(gad)->paid_SelectedBox;
        newBox = WhichBoxNum(gad,msg);

	if (!PAID(gad)->paid_Indicator && (icode == ( IECODE_LBUTTON | IECODE_UP_PREFIX)))
            newBox = ~0;

        if (icode == IECODE_RBUTTON)
            newBox = PAID(gad)->paid_BoxBackup;

	if (newBox != oldBox)
	{
	    if (rp = ObtainGIRPort(msg->gpi_GInfo))
	    {
                if (newBox != ~0)
                    PAID(gad)->paid_Color = GetColorIndex(gad,newBox);

                SelectBox(rp,gad,oldBox,FALSE,FALSE);
                SelectBox(rp,gad,newBox,TRUE,FALSE);

                PAID(gad)->paid_SelectedBox = newBox;

                ReleaseGIRPort(rp);
            }
	}

	if (icode == ( IECODE_LBUTTON | IECODE_UP_PREFIX))
	    result = GMR_NOREUSE | GMR_VERIFY;

        if (icode == IECODE_RBUTTON)
            result = GMR_NOREUSE;
    }

    return(result);
}


/*****************************************************************************/


ULONG ASM DispatchPalette(REG(a0) struct Hook *hook,
                          REG(a2) struct ExtGadget *gad,
                          REG(a1) ULONG *msg)
{
ULONG result;

    switch (*msg)
    {
        case GM_HELPTEST   : result = GMR_HELPHIT;
                             break;

	case GM_HITTEST    : result = GMR_GADGETHIT;
                             break;

	case GM_GOACTIVE   : PAID(gad)->paid_BoxBackup = PAID(gad)->paid_SelectedBox;
	case GM_HANDLEINPUT: result = HandlePaletteInput(gad,(struct gpInput *)msg);
                             break;

	case GM_RENDER     : RenderPalette(((struct gpRender *)msg)->gpr_RPort,gad);

        default            : result = 0;
                             break;
    }

    return(result);
}


/*****************************************************************************/


struct ExtGadget *CreatePaletteA(struct ExtGadget *gad, struct NewGadget *ng,
                                 struct TagItem *taglist)
{
struct NewGadget   mod_ng;
LONG               colorWidth, colorHeight;
UWORD              depth  = getGTTagData(GTPA_Depth, 1, taglist);
UWORD              colors = getGTTagData(GTPA_NumColors, 1 << depth, taglist);
UWORD              indicatorwidth = getGTTagData(GTPA_IndicatorWidth, 0, taglist);
UWORD              columns, rows;
UWORD              bestcolumns, bestrows;
UWORD              availWidth, availHeight;
UWORD              divisor;
LONG               ratio;
LONG               bestratio;
ULONG              resX, resY;
struct DrawInfo   *di;

    di          = VI(ng->ng_VisualInfo)->vi_DrawInfo;
    resX        = di->dri_Resolution.X * 256;
    resY        = di->dri_Resolution.Y;
    availWidth  = ng->ng_Width - 5;
    availHeight = ng->ng_Height - 3;
    bestrows    = 0;
    bestcolumns = 0;
    bestratio   = 0x00800000;      /* a big positive number... */

    while (colors > 0)
    {
        divisor = 1;
        do
        {
            if (colors % divisor == 0)
            {
                columns = colors / divisor;
                rows    = divisor;

                colorWidth  = availWidth / columns - 3;
                colorHeight = availHeight / rows - 1;
                if ((colorWidth >= MINCOLORWIDTH) && (colorHeight >= MINCOLORHEIGHT))
                {
                    ratio = (colorWidth*resX) / (colorHeight*resY);
                    if (abs(256-ratio) < abs(256-bestratio))
                    {
                        /* if aspect ratio better than current best */
                        bestcolumns = columns;
                        bestrows    = rows;
                        bestratio   = ratio;
                    }
                }
            }

            divisor++;
        }
        while (divisor <= colors);

        if (bestcolumns > 0)
            break;

        colors--;
    }

    if (bestcolumns <= 0)
        return(NULL);

    colorWidth  = availWidth / bestcolumns;
    colorHeight = availHeight / bestrows;

    mod_ng           = *ng;
    mod_ng.ng_Width  = colorWidth * bestcolumns + 5;
    mod_ng.ng_Height = colorHeight * bestrows + 3;

    if (gad = CreateGenericBase(gad, &mod_ng, PALETTE_IDATA_SIZE, taglist))
    {
        gad->Flags        |= GADGIMAGE | GADGHNONE;
        gad->Activation    = GADGIMMEDIATE | RELVERIFY;
        gad->GadgetType   |= CUSTOMGADGET;
        gad->MutualExclude = (ULONG)&GadToolsBase->gtb_PaletteGHook;

        placeGadgetText(gad, ng->ng_Flags, indicatorwidth ? PLACETEXT_LEFT : PLACETEXT_ABOVE,NULL);

        SGAD(gad)->sg_SetAttrs     = SetPaletteAttrsA;
        SGAD(gad)->sg_GetTable     = Palette_GetTable;
        SGAD(gad)->sg_EventHandler = HandlePalette;
        SGAD(gad)->sg_Flags        = SG_EXTRAFREE_DISPOSE;

        if (!(gad->GadgetRender = getBevelImage(0,0,gad->Width,gad->Height,FRAME_BUTTON)))
            return(NULL);

        PAID(gad)->paid_XCount      = bestcolumns;
        PAID(gad)->paid_EachWidth   = colorWidth;
        PAID(gad)->paid_EachHeight  = colorHeight;
        PAID(gad)->paid_Count       = bestcolumns * bestrows;
        PAID(gad)->paid_Color       = 1;
        PAID(gad)->paid_DrawInfo    = di;
        PAID(gad)->paid_SelectedBox = (UWORD)~0;

        if (indicatorwidth || findGTTagItem(GTPA_IndicatorHeight,taglist))
            PAID(gad)->paid_Indicator = TRUE;

        SetPaletteAttrsA(gad,NULL,taglist);
    }

    return(gad);
}


/*****************************************************************************/


VOID SetPaletteAttrsA(struct ExtGadget *gad, struct Window *win,
                      struct TagItem *taglist)
{
struct RastPort  crp;
struct TagItem  *tag;
BOOL             render;
BOOL             active;
ULONG            lock;
UWORD            i;
UWORD            newBox, oldBox;

    render = FALSE;

    lock = LockIBase(0);

    if (tag = findGTTagItem(GTPA_ColorTable,taglist))
    {
        PAID(gad)->paid_ColorTable = (UBYTE *)tag->ti_Data;
        render = TRUE;
    }

    if (tag = findGTTagItem(GTPA_ColorOffset,taglist))
    {
        PAID(gad)->paid_ColorOffset = tag->ti_Data;
        render = TRUE;
    }

    if (tag = findTagItem(GA_Disabled,taglist))
    {
        active = (gad->Flags & GFLG_DISABLED);
        if (!tag->ti_Data)
        {
            if (active)
            {
                gad->Flags &= ~(GFLG_DISABLED);
                render = TRUE;
            }
        }
        else
        {
            if (!active)
            {
                gad->Flags |= GFLG_DISABLED;
                render = TRUE;
            }
        }
    }

    PAID(gad)->paid_Color = getGTTagData(GTPA_Color,PAID(gad)->paid_Color,taglist);

    /* bounds check the color number */
    if (PAID(gad)->paid_ColorTable)
    {
        i = 0;
        while (i < PAID(gad)->paid_Count)
        {
            if (PAID(gad)->paid_Color == PAID(gad)->paid_ColorTable[i])
                break;

            i++;
        }

        if (i == PAID(gad)->paid_Count)  /* didn't find the color in the table */
            PAID(gad)->paid_Color = PAID(gad)->paid_ColorTable[0];
    }
    else if (PAID(gad)->paid_Color >= PAID(gad)->paid_ColorOffset + PAID(gad)->paid_Count)
    {
        PAID(gad)->paid_Color = PAID(gad)->paid_Count - PAID(gad)->paid_ColorOffset - 1;
    }

    if (win)
    {
        cloneRastPort(&crp,GetRP(gad,win));

        oldBox = PAID(gad)->paid_SelectedBox;
        newBox = GetBoxNum(gad,PAID(gad)->paid_Color);

        if (render || (oldBox != newBox))
        {
            if (render || (gad->Flags & GFLG_DISABLED))
            {
                RenderPalette(&crp,gad);
            }
            else
            {
                SelectBox(&crp,gad,oldBox,FALSE,FALSE);
                SelectBox(&crp,gad,newBox,TRUE,FALSE);
            }
        }
    }

    UnlockIBase(lock);
}


/*****************************************************************************/


BOOL HandlePalette(struct ExtGadget *gad, struct IntuiMessage *imsg)
{
    if (imsg->Class == IDCMP_GADGETUP)
    {
	imsg->Code = PAID(gad)->paid_Color;
	return(TRUE);
    }

    return(FALSE);
}
