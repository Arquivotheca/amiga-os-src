
/* includes */
#include <libraries/gadtools.h>
#include <intuition/gadgetclass.h>

/* prototypes */
#include <clib/macros.h>
#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>

/* direct ROM interface */
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>

/* application includes */
#include "iegads.h"
#include "iemain.h"
#include "sketchpad.h"
#include "sp_tools.h"
#include "sp_arrows.h"
#include "texttable.h"
#include "ieutils.h"


/*****************************************************************************/


extern struct Library *IntuitionBase;
extern struct Library *GadToolsBase;


/*****************************************************************************/


struct NewSketchPad NewSketchPad =
{
    NULL, NULL, 4, 4,
    PANELLEFT, PANELTOP, ICON_WIDTH, ICON_HEIGHT,
    2, SP_GRID, 1, 0
};

STRPTR imageLabels[3];


/*****************************************************************************/


BOOL CreateIconGadgets(struct WindowInfo * wi)
{
struct Gadget    *gad;
struct NewGadget  ng;

    NewSketchPad.Window  = wi->win;
    NewSketchPad.Depth   = wi->w_Depth;
    NewSketchPad.TopEdge = wi->topborder + PANELTOP;
    NewSketchPad.Visual  = wi->vi;
    NewSketchPad.MagX    = wi->w_MagX;
    NewSketchPad.MagY    = wi->w_MagY;

    if (!(wi->wi_sketch = OpenSketchPad(&NewSketchPad)))
    {
        return(FALSE);
    }

    gad = wi->sketchgad = &(wi->wi_sketch->Gad);
    wi->w_LO            = (gad->LeftEdge + gad->Width + 6);
    wi->wi_sketch->Grid = wi->w_UseGrid;
    wi->type            = 2;
    wi->hilite          = 0;

    ng.ng_TextAttr   = &(wi->mydesiredfont);
    ng.ng_VisualInfo = wi->vi;
    gad = CreateContext(&wi->firstgad);

    /* "Clear" gadget: */
    ng.ng_GadgetID   = SPCLEAR_ID;
    ng.ng_TopEdge    = wi->topborder + 153;
    ng.ng_LeftEdge   = wi->w_LO;
    ng.ng_Width      = 75;
    ng.ng_Height     = 11;
    ng.ng_GadgetText = GetString(MSG_IE_CLEAR_GAD);
    ng.ng_Flags      = NULL;
    gad = CreateGadgetA(BUTTON_KIND,gad,&ng,NULL);

    /* "Undo" gadget: */
    ng.ng_GadgetID   = SPUNDO_ID;
    ng.ng_TopEdge    = wi->topborder + 141;
    ng.ng_GadgetText = GetString(MSG_IE_UNDO_GAD);
    gad = CreateGadgetA(BUTTON_KIND,gad,&ng,NULL);

    /* Edit Selection */
    imageLabels[0]   = GetString(MSG_IE_NORMAL_GAD);
    imageLabels[1]   = GetString(MSG_IE_SELECTED_GAD);
    imageLabels[2]   = NULL;
    ng.ng_LeftEdge   = (wi->w_LO + 78);
    ng.ng_TopEdge    = wi->topborder + 48;
    ng.ng_GadgetID   = IE_SELECT;
    ng.ng_Flags      = PLACETEXT_RIGHT;
    wi->mxgad = gad = CreateGadget(MX_KIND,gad,&ng,GTMX_Labels,     imageLabels,
                                                   GTMX_Active,     0,
                                                   LAYOUTA_SPACING, 4,
                                                   TAG_DONE);

    /* Normal Icon Image, showing Selection method */
    ng.ng_GadgetID   = SPREPEAT_ID;
    ng.ng_LeftEdge   = (wi->w_LO + 77) + 16;
    ng.ng_TopEdge    = wi->topborder + IMG1TOP;
    ng.ng_Width      = ICON_WIDTH;
    ng.ng_Height     = ICON_HEIGHT;
    ng.ng_GadgetText = NULL;
    ng.ng_Flags      = NULL;
    if (wi->img1 = gad = CreateGadgetA(GENERIC_KIND,gad,&ng,NULL))
    {
        gad->Flags        = GADGIMAGE | GADGHIMAGE;
        gad->Activation   = GADGIMMEDIATE | RELVERIFY;
        gad->GadgetType  |= BOOLGADGET;
        gad->GadgetRender = &wi->images[0].di_Image;
        gad->SelectRender = &wi->images[1].di_Image;
    }

    /* Selected Icon Image */
    ng.ng_GadgetID = 0;
    ng.ng_TopEdge  = wi->topborder + IMG2TOP;
    if (wi->img2 = gad = CreateGadgetA(GENERIC_KIND,gad,&ng,NULL))
    {
        gad->Flags        = GADGIMAGE | GADGHNONE;
        gad->Activation   = GADGIMMEDIATE | RELVERIFY;
        gad->GadgetType  |= BOOLGADGET;
        gad->GadgetRender = &wi->images[1].di_Image;
    }

    /* Palette gadget */
    ng.ng_LeftEdge   = wi->w_LO;
    ng.ng_TopEdge    = wi->topborder + PALETTETOP;
    ng.ng_Width      = MAXPALWIDTH;
    ng.ng_Height     = MAXPALHEIGHT;
    ng.ng_GadgetText = NULL;
    ng.ng_GadgetID   = SPPALETTE_ID;
    ng.ng_Flags      = NG_HIGHLABEL;
    gad = CreateGadget(PALETTE_KIND,gad,&ng,GTPA_Depth,		MIN (wi->w_Depth, 3),
                                            GTPA_Color,		1,
                                            GTPA_ColorOffset,	0,
					    GTPA_ColorTable,	wi->w_ColorTable,
                                            TAG_DONE);

    if (!gad)
        return(FALSE);

    /* Add the GadTools gadgets to the list */
    AddGList(wi->win,wi->firstgad,((USHORT)-1),((USHORT)-1),NULL);

    wi->Adi     = &wi->images[wi->currentwin];
    wi->draw_rp = &wi->Adi->di_RPort;

    AddToolBox(wi->wi_sketch,(wi->w_LO + 7), (SHORT)(wi->topborder + 97));
    AddMoveBox(wi->wi_sketch,((wi->w_LO + 77) - 4), (SHORT)(wi->topborder + 120));
    SPSetRepeat(wi->wi_sketch,wi->Adi,(wi->w_LO + 77) + 16,wi->wintopx[wi->currentwin]);
    SPSaveToUndo(wi->wi_sketch);
    SPRefresh(wi->wi_sketch);
    SPSetDrMd(wi->wi_sketch,0);
    SPSetAPen(wi->wi_sketch,1);

    return(TRUE);
}


/*****************************************************************************/


VOID RefreshImages(WindowInfoPtr wi)
{
    RefreshGList(wi->img1,wi->win,NULL,1);
    RefreshGList(wi->img2,wi->win,NULL,1);
}
