
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <graphics/text.h>
#include <graphics/gfxmacros.h>
#include <libraries/gadtools.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>

/* application includes */
#include "pe_custom.h"


#define SysBase ed->ed_SysBase


/*****************************************************************************/


#define DRAG_LEFT   82
#define DRAG_TOP    8
#define DRAG_WIDTH  460
#define DRAG_HEIGHT 11
#define DRAG_RIGHT  DRAG_LEFT+DRAG_WIDTH-1
#define DRAG_BOTTOM DRAG_TOP+DRAG_HEIGHT-1

#define MENU_LEFT   435
#define MENU_TOP    37
#define MENU_WIDTH  177
#define MENU_HEIGHT 70
#define MENU_RIGHT  MENU_LEFT+MENU_WIDTH-1
#define MENU_BOTTOM MENU_TOP+MENU_HEIGHT-1

#define WINDOWA_LEFT    12
#define WINDOWA_TOP     28
#define WINDOWA_WIDTH   200
#define WINDOWA_HEIGHT  79
#define WINDOWA_RIGHT   WINDOWA_LEFT+WINDOWA_WIDTH-1
#define WINDOWA_BOTTOM  WINDOWA_TOP+WINDOWA_HEIGHT-1

#define WINDOWB_LEFT    224
#define WINDOWB_TOP     28
#define WINDOWB_WIDTH   200
#define WINDOWB_HEIGHT  79
#define WINDOWB_RIGHT   WINDOWB_LEFT+WINDOWB_WIDTH-1
#define WINDOWB_BOTTOM  WINDOWB_TOP+WINDOWB_HEIGHT-1

#define BITMAP_WIDTH  640
#define BITMAP_HEIGHT 120
#define BITMAP_DEPTH  4


/*****************************************************************************/


APTR NewPrefsObject(EdDataPtr ed, struct IClass *cl, STRPTR name, ULONG data,...)
{
    return (NewObjectA(cl,name, (struct TagItem *) &data));
}


/*****************************************************************************/


ULONG BestModeIDP(EdDataPtr ed, ULONG tag,...)
{
    return (BestModeIDA((struct TagItem *) &tag));
}


/*****************************************************************************/


APTR NewPImage(EdDataPtr ed, struct DrawInfo *di, ULONG imageNumber)
{
    return(NewPrefsObject(ed,NULL,"sysiclass", SYSIA_Which,         imageNumber,
                                               SYSIA_DrawInfo,      di,
                                               SYSIA_ReferenceFont, ed->ed_Font,
                                               TAG_DONE));

}


/*****************************************************************************/


VOID PlaceText(EdDataPtr ed, AppStringsID str, UWORD x, UWORD y)
{
STRPTR text;
struct RastPort *rp = ed->ed_SampleRastPort;

    text = GetString(&ed->ed_LocaleInfo,str);
    Move(rp,x,y);
    Text(rp,text,strlen(text));
}


/*****************************************************************************/


VOID CenterText(EdDataPtr ed, AppStringsID str, UWORD x0, UWORD x1, UWORD y)
{
struct RastPort *rp = ed->ed_SampleRastPort;
STRPTR           text;

    text = GetString(&ed->ed_LocaleInfo,str);
    Move(rp,(x1 - x0 - strlen(text)*8) / 2 + x0,y);
    Text(rp,text,strlen(text));
}


/*****************************************************************************/


VOID PlaceItem(EdDataPtr ed, AppStringsID str, struct Image *amigaKey,
               UWORD x, UWORD y)
{
struct RastPort *rp = ed->ed_SampleRastPort;
STRPTR           text;

    text = GetString(&ed->ed_LocaleInfo,str);

    Move(rp,x,y);
    Text(rp,&text[2],strlen(&text[2]));

    if (text[0] != ' ')
    {
        DrawImage(rp,amigaKey,MENU_RIGHT-38,y-6);
        Move(rp,MENU_RIGHT-13,y);
        Text(rp,text,1);
    }
}


/*****************************************************************************/


static USHORT GhostPattern[2] =
{
     0x8888,
     0x2222
};

static USHORT PropPattern[2] =
{
     0x5555,
     0xAAAA
};


/*****************************************************************************/


VOID MenuBar(EdDataPtr ed, struct RastPort *rp, UWORD dPen, UWORD bPen,
             UWORD x0, UWORD y0, UWORD x1, UWORD y1)
{
    SetABPenDrMd(rp,dPen,0,JAM1);
    RectFill(rp,x0,y0,x1,y1);
    SetAPen(rp,bPen);
    SetAfPt(rp,GhostPattern,1);
    RectFill(rp,x0,y0,x1,y1);
    SetAfPt(rp,NULL,0);
}


/*****************************************************************************/


/* for meeting_type parameter of EmbossedBoxTrim() */
#define MEET_ULCOLOR  0
#define MEET_LRCOLOR  1
#define MEET_NONE     2
#define MEET_GADTOOLS 3

/* Behavior at "joints" in upper-right and lower-left corners
 * is controlled by value of 'meeting_type':
 *	MEET_ULCOLOR : joints same color as upper left
 *	MEET_LRCOLOR : joints same color as lower right
 *	MEET_NONE    : joints not drawn
 *      MEET_GADTOOLS: bottom/left joint same color as left, top/right joint
 *	               same color as right edge (gadtools look)
 */

VOID EmbossedBoxTrim(EdDataPtr ed,
                     struct RastPort *rp,
                     LONG left, LONG top, LONG right, LONG bottom,
                     ULONG hthick, ULONG vthick,
                     ULONG ulpen, ULONG lrpen,
                     ULONG meeting_type )
{
LONG cnt;
LONG scum1, scum2, scum3, scum4;
LONG i;

    scum1 = 1;
    scum2 = 1;
    scum3 = 1;
    scum4 = 1;

    if (meeting_type < MEET_NONE)
    {
        scum1 = meeting_type;
        scum2 = meeting_type;
        scum3 = 1-meeting_type;
        scum4 = 1-meeting_type;
    }
    else if (meeting_type == MEET_GADTOOLS)
    {
        scum2 = 0;
        scum3 = 0;
    }

    SetAPen(rp,ulpen);

    /* top edge */
    cnt = scum1;
    for (i = 0; i < vthick; i++)
    {
        RectFill(rp,left,top+i,right-cnt,top+i);

        if (cnt < hthick)
            cnt++;
    }

    /* left edge */
    cnt = scum2;
    for (i = 0; i < hthick; i++)
    {
        RectFill(rp,left+i,top+vthick,left+i,bottom-cnt);

        if (cnt < vthick)
            cnt++;
    }

    SetAPen(rp,lrpen);

    /* right edge */
    cnt = scum3;
    for (i = 0; i < hthick; i++)
    {
        RectFill(rp,right-i,top+cnt,right-i,bottom-vthick);

        if (cnt < vthick)
            cnt++;
    }

    /* bottom edge */
    cnt = scum4;
    for (i = 0; i < vthick; i++)
    {
        RectFill(rp,left+cnt,bottom-i,right,bottom-i);

        if (cnt < hthick)
            cnt++;
    }
}


/*****************************************************************************/


static VOID DoSample(EdDataPtr ed, BOOL complete, struct RastPort *rp,
                     UWORD top, struct DrawInfo *di, UWORD *pens, struct Window *wp)
{
struct Image    *checkMark;
struct Image    *amigaKey;
struct Image    *closeGadget;
struct Image    *sizeGadget;
struct Image    *depthGadget;
struct Image    *zoomGadget;
struct Image    *upGadget;
struct Image    *downGadget;
struct Image    *checkBox;
struct Image    *mxKnob;
struct Image    *scrDepthGadget;
STRPTR           text;

    ed->ed_SampleRastPort = rp;

    checkMark      = NewPImage(ed,di,MENUCHECK);
    amigaKey       = NewPImage(ed,di,AMIGAKEY);
    closeGadget    = NewPImage(ed,di,CLOSEIMAGE);
    sizeGadget     = NewPImage(ed,di,SIZEIMAGE);
    depthGadget    = NewPImage(ed,di,DEPTHIMAGE);
    zoomGadget     = NewPImage(ed,di,ZOOMIMAGE);
    upGadget       = NewPImage(ed,di,UPIMAGE);
    downGadget     = NewPImage(ed,di,DOWNIMAGE);
    checkBox       = NewPImage(ed,di,CHECKIMAGE);
    mxKnob         = NewPImage(ed,di,MXIMAGE);
    scrDepthGadget = NewPImage(ed,di,SDEPTHIMAGE);

    if (checkMark && amigaKey && closeGadget && sizeGadget && zoomGadget
    && depthGadget && upGadget && downGadget && checkBox && mxKnob && scrDepthGadget)
    {
        /* background */

        if (complete)
        {
            SetAPen(rp,pens[BACKGROUNDPEN]);
            if (wp)
                RectFill(rp, wp->BorderLeft, top + DRAG_TOP - 7, wp->Width - wp->BorderRight - 1, wp->Height - wp->BorderBottom-1);
            else
                RectFill(rp,0,0,BITMAP_WIDTH-1,BITMAP_HEIGHT-1);
        }

        /* fake menu */

        SetABPenDrMd(rp,pens[BARDETAILPEN],0,JAM1);
        RectFill(rp,MENU_LEFT,top + MENU_TOP,MENU_RIGHT,top + MENU_TOP);
        RectFill(rp,MENU_LEFT,top + MENU_BOTTOM,MENU_RIGHT,top + MENU_BOTTOM);
        RectFill(rp,MENU_LEFT,top + MENU_TOP,MENU_LEFT+1,top + MENU_BOTTOM);
        RectFill(rp,MENU_RIGHT-1,top + MENU_TOP,MENU_RIGHT,top + MENU_BOTTOM);
        RectFill(rp,MENU_LEFT,top + MENU_TOP-9,MENU_LEFT+7+8*strlen(GetString(&ed->ed_LocaleInfo,MSG_PAL_SAMPLE_MENU)),top + MENU_TOP-1);

        SetAPen(rp,pens[BARBLOCKPEN]);
        PlaceText(ed,MSG_PAL_SAMPLE_MENU,MENU_LEFT+4,top + MENU_TOP-2);
        RectFill(rp,MENU_LEFT+2,top + MENU_TOP+1,MENU_RIGHT-2,top + MENU_BOTTOM-1);

        SetAPen(rp,pens[BARDETAILPEN]);
        PlaceItem(ed,MSG_PAL_SAMPLE_ITEM_1,amigaKey,MENU_LEFT+6,top + MENU_TOP+9);
        PlaceItem(ed,MSG_PAL_SAMPLE_ITEM_2,amigaKey,MENU_LEFT+6,top + MENU_TOP+18);
        PlaceItem(ed,MSG_PAL_SAMPLE_ITEM_3,amigaKey,MENU_LEFT+6,top + MENU_TOP+33);
        PlaceItem(ed,MSG_PAL_SAMPLE_ITEM_4,amigaKey,MENU_LEFT+6,top + MENU_TOP+42);
        PlaceItem(ed,MSG_PAL_SAMPLE_ITEM_5,amigaKey,MENU_LEFT+25,top + MENU_TOP+52);
        PlaceItem(ed,MSG_PAL_SAMPLE_ITEM_6,amigaKey,MENU_LEFT+6,top + MENU_TOP+66);

        SetWrMsk(rp,pens[BARDETAILPEN] ^ pens[BARBLOCKPEN]);   /* don't ask... */
        SetDrMd(rp,COMPLEMENT);
        RectFill(rp,MENU_LEFT+4,top + MENU_TOP+35,MENU_RIGHT-5,top + MENU_TOP+44);
        SetWrMsk(rp,-1);

        MenuBar(ed,rp,pens[BARDETAILPEN],pens[BARBLOCKPEN],MENU_LEFT+6,top + MENU_TOP+22,MENU_RIGHT-6,top + MENU_TOP+23);
        DrawImage(rp,checkMark,MENU_LEFT+6,top + MENU_TOP+46);
        MenuBar(ed,rp,pens[BARDETAILPEN],pens[BARBLOCKPEN],MENU_LEFT+6,top + MENU_TOP+55,MENU_RIGHT-6,top + MENU_TOP+56);

        /* fake active window */

        EmbossedBoxTrim(ed,rp,WINDOWA_LEFT,top + WINDOWA_TOP,WINDOWA_RIGHT,top + WINDOWA_BOTTOM,
                        1,1,pens[SHINEPEN],pens[SHADOWPEN],MEET_ULCOLOR);

        EmbossedBoxTrim(ed,rp,WINDOWA_LEFT+3,top + WINDOWA_TOP+10,WINDOWA_RIGHT-17,top + WINDOWA_BOTTOM-1,
                        1,1,pens[SHADOWPEN],pens[SHINEPEN],MEET_ULCOLOR);

        SetAPen(rp,pens[FILLPEN]);
        RectFill(rp,WINDOWA_LEFT+3,top + WINDOWA_TOP+1,WINDOWA_RIGHT-3,top + WINDOWA_TOP+9);
        RectFill(rp,WINDOWA_LEFT+1,top + WINDOWA_TOP+1,WINDOWA_LEFT+2,top + WINDOWA_BOTTOM-1);
        RectFill(rp,WINDOWA_RIGHT-16,top + WINDOWA_TOP+1,WINDOWA_RIGHT-1,top + WINDOWA_BOTTOM-1);

        DrawImageState(rp,closeGadget,WINDOWA_LEFT,top + WINDOWA_TOP,IDS_NORMAL,di);
        DrawImageState(rp,zoomGadget,WINDOWA_RIGHT-44,top + WINDOWA_TOP,IDS_NORMAL,di);
        DrawImageState(rp,depthGadget,WINDOWA_RIGHT-22,top + WINDOWA_TOP,IDS_NORMAL,di);
        DrawImageState(rp,upGadget,WINDOWA_RIGHT-17,top + WINDOWA_BOTTOM-31,IDS_NORMAL,di);
        DrawImageState(rp,downGadget,WINDOWA_RIGHT-17,top + WINDOWA_BOTTOM-20,IDS_NORMAL,di);
        DrawImageState(rp,sizeGadget,WINDOWA_RIGHT-17,top + WINDOWA_BOTTOM-9,IDS_NORMAL,di);

        SetAfPt(rp,PropPattern,1);
        SetAPen(rp,pens[BLOCKPEN]);
        RectFill(rp,WINDOWA_RIGHT-13,top + WINDOWA_TOP+12,WINDOWA_RIGHT-4,top + WINDOWA_BOTTOM-33);
        SetAfPt(rp,NULL,0);

        EmbossedBoxTrim(ed,rp,WINDOWA_RIGHT-13,top+WINDOWA_TOP+25,
                              WINDOWA_RIGHT-4,top+WINDOWA_BOTTOM-40,
                              1,1,pens[SHINEPEN],pens[SHADOWPEN],MEET_ULCOLOR);

        SetAPen(rp,pens[FILLPEN]);
        RectFill(rp,WINDOWA_RIGHT-12,top + WINDOWA_TOP+26,WINDOWA_RIGHT-5,top + WINDOWA_BOTTOM-41);

        SetAPen(rp,pens[FILLTEXTPEN]);
        PlaceText(ed,MSG_PAL_SAMPLE_ACTIVE,WINDOWA_LEFT+27,top + WINDOWA_TOP+7);

        /* fake inactive window */

        EmbossedBoxTrim(ed,rp,WINDOWB_LEFT,top + WINDOWB_TOP,WINDOWB_RIGHT,top + WINDOWB_BOTTOM,
                        1,1,pens[SHINEPEN],pens[SHADOWPEN],MEET_ULCOLOR);

        EmbossedBoxTrim(ed,rp,WINDOWB_LEFT+3,top + WINDOWB_TOP+10,WINDOWB_RIGHT-17,top + WINDOWB_BOTTOM-1,
                        1,1,pens[SHADOWPEN],pens[SHINEPEN],MEET_ULCOLOR);

        DrawImageState(rp,closeGadget,WINDOWB_LEFT,top + WINDOWB_TOP,IDS_INACTIVENORMAL,di);
        DrawImageState(rp,zoomGadget,WINDOWB_RIGHT-44,top + WINDOWB_TOP,IDS_INACTIVENORMAL,di);
        DrawImageState(rp,depthGadget,WINDOWB_RIGHT-22,top + WINDOWB_TOP,IDS_INACTIVENORMAL,di);
        DrawImageState(rp,upGadget,WINDOWB_RIGHT-17,top + WINDOWB_BOTTOM-31,IDS_INACTIVENORMAL,di);
        DrawImageState(rp,downGadget,WINDOWB_RIGHT-17,top + WINDOWB_BOTTOM-20,IDS_INACTIVENORMAL,di);
        DrawImageState(rp,sizeGadget,WINDOWB_RIGHT-17,top + WINDOWB_BOTTOM-9,IDS_INACTIVENORMAL,di);

        SetAfPt(rp,PropPattern,1);
        SetAPen(rp,pens[BLOCKPEN]);
        RectFill(rp,WINDOWB_RIGHT-13,top + WINDOWB_TOP+12,WINDOWB_RIGHT-4,top + WINDOWB_BOTTOM-33);
        SetAfPt(rp,NULL,0);

        EmbossedBoxTrim(ed,rp,WINDOWB_RIGHT-13,top+WINDOWB_TOP+25,
                              WINDOWB_RIGHT-4,top+WINDOWB_BOTTOM-40,
                              1,1,pens[SHINEPEN],pens[SHADOWPEN],MEET_ULCOLOR);

        SetAPen(rp,pens[BACKGROUNDPEN]);
        RectFill(rp,WINDOWB_RIGHT-12,top + WINDOWB_TOP+26,WINDOWB_RIGHT-5,top + WINDOWB_BOTTOM-41);

        SetAPen(rp,pens[TEXTPEN]);
        PlaceText(ed,MSG_PAL_SAMPLE_INACTIVE,WINDOWB_LEFT+27,top + WINDOWB_TOP+7);

        /* fake screen drag bar */

        SetAPen(rp,pens[BARBLOCKPEN]);
        RectFill(rp,DRAG_LEFT,top + DRAG_TOP,DRAG_RIGHT - 17,top + DRAG_BOTTOM-1);
        SetAPen(rp,pens[BARTRIMPEN]);
        RectFill(rp,DRAG_LEFT,top + DRAG_BOTTOM,DRAG_RIGHT - 17,top + DRAG_BOTTOM);
        DrawImageState(rp,scrDepthGadget,DRAG_RIGHT - 17,top + DRAG_TOP,IDS_NORMAL,di);

        SetAPen(rp,pens[BARDETAILPEN]);
        PlaceText(ed,MSG_PAL_SAMPLE_SCREENDRAG,DRAG_LEFT+5,top + DRAG_TOP+7);

        /* sample text */

        SetAPen(rp,pens[HIGHLIGHTTEXTPEN]);
        CenterText(ed,MSG_PAL_SAMPLE_SPECIAL,WINDOWA_LEFT,WINDOWA_RIGHT-17,top + WINDOWA_TOP+23);

        /* sample button */

        EmbossedBoxTrim(ed,rp,WINDOWA_LEFT+30,top + WINDOWA_TOP+33,WINDOWA_RIGHT-44,top + WINDOWA_TOP+46,
                        2,1,pens[SHINEPEN],pens[SHADOWPEN],MEET_GADTOOLS);

        if (complete)
        {
            SetAPen(rp,pens[TEXTPEN]);
            CenterText(ed,MSG_PAL_SAMPLE_BUTTON,WINDOWA_LEFT+30,WINDOWA_RIGHT-44,top + WINDOWA_TOP+42);

            /* sample string gadget */
            PlaceText(ed,MSG_PAL_SAMPLE_STRING,WINDOWA_LEFT+95-8*strlen(GetString(&ed->ed_LocaleInfo,MSG_PAL_SAMPLE_STRING)),top + WINDOWA_TOP+63);
        }

        SetAPen(rp,pens[TEXTPEN]);
        PlaceText(ed,MSG_PAL_SAMPLE_NUMBER,WINDOWA_LEFT+106,top + WINDOWA_TOP+63);

        if (ed->ed_Depth == 2)
            SetABPenDrMd(rp,ed->ed_ColorTable[2],ed->ed_ColorTable[3],JAM2);
        else
            SetABPenDrMd(rp,ed->ed_ColorTable[6],ed->ed_ColorTable[7],JAM2);

        text = GetString(&ed->ed_LocaleInfo,MSG_PAL_SAMPLE_NUMBER);
        Move(rp,WINDOWA_LEFT+106+16,top + WINDOWA_TOP+63);
        Text(rp,&text[2],1);

        EmbossedBoxTrim(ed,rp,WINDOWA_LEFT+101,top + WINDOWA_TOP+54,WINDOWA_RIGHT-32,top + WINDOWA_TOP+67,
                        2,1,pens[SHINEPEN],pens[SHADOWPEN],MEET_GADTOOLS);
        EmbossedBoxTrim(ed,rp,WINDOWA_LEFT+103,top + WINDOWA_TOP+55,WINDOWA_RIGHT-34,top + WINDOWA_TOP+66,
                        2,1,pens[SHADOWPEN],pens[SHINEPEN],MEET_GADTOOLS);

        /* sample mx */

        SetABPenDrMd(rp,pens[TEXTPEN],pens[BACKGROUNDPEN],JAM2);
        DrawImageState(rp,mxKnob,WINDOWB_LEFT+52,top + WINDOWB_TOP+20,IDS_NORMAL,di);
        DrawImageState(rp,mxKnob,WINDOWB_LEFT+52,top + WINDOWB_TOP+30,IDS_SELECTED,di);
        DrawImageState(rp,mxKnob,WINDOWB_LEFT+52,top + WINDOWB_TOP+40,IDS_NORMAL,di);

        if (complete)
        {
            PlaceText(ed,MSG_PAL_SAMPLE_FOOD_1,WINDOWB_LEFT+75,top + WINDOWB_TOP+28);
            PlaceText(ed,MSG_PAL_SAMPLE_FOOD_2,WINDOWB_LEFT+75,top + WINDOWB_TOP+38);
            PlaceText(ed,MSG_PAL_SAMPLE_FOOD_3,WINDOWB_LEFT+75,top + WINDOWB_TOP+48);

            /* sample checkbox */

            PlaceText(ed,MSG_PAL_SAMPLE_SALT,WINDOWB_LEFT+77,top + WINDOWB_TOP+68);
        }
        DrawImageState(rp,checkBox,WINDOWB_LEFT+47,top + WINDOWB_TOP+60,IDS_SELECTED,di);
    }

    DisposeObject(checkMark);
    DisposeObject(amigaKey);
    DisposeObject(closeGadget);
    DisposeObject(sizeGadget);
    DisposeObject(depthGadget);
    DisposeObject(zoomGadget);
    DisposeObject(upGadget);
    DisposeObject(downGadget);
    DisposeObject(checkBox);
    DisposeObject(mxKnob);
    DisposeObject(scrDepthGadget);
}


/*****************************************************************************/


VOID RenderSample(EdDataPtr ed, UWORD *opens, BOOL complete)
{
struct DrawInfo  di;
struct Window   *wp;
struct RastPort *rp;
UWORD            top;
UWORD            pens[NUM_KNOWNPENS+1];
UWORD            i;

    wp  = ed->ed_Window;
    top = wp->BorderTop + 187;
    if (ed->ed_SampleWindow)
    {
        wp  = ed->ed_SampleWindow;
        top = wp->BorderTop;
    }
    rp = wp->RPort;

    i = 0;
    while (i < NUM_KNOWNPENS)
    {
        pens[i] = ed->ed_ColorTable[opens[i]];
        i++;
    }
    pens[i] = ~0;

    di.dri_Version      = DRI_VERSION;
    di.dri_NumPens      = NUM_KNOWNPENS;
    di.dri_Pens         = pens;
    di.dri_Font         = ed->ed_Font;
    di.dri_Depth        = ed->ed_Screen->BitMap.Depth;
    di.dri_Resolution.X = 10;
    di.dri_Resolution.Y = 11;
    di.dri_Flags        = DRIF_NEWLOOK;
    di.dri_CheckMark    = NULL;
    di.dri_AmigaKey     = NULL;

    DoSample(ed,complete,rp,top,&di,pens,wp);
}


/*****************************************************************************/


VOID CreateClickMap(EdDataPtr ed)
{
struct DrawInfo di;
UWORD           pens[NUMDRIPENS+1];
UWORD           i;

    InitRastPort(&ed->ed_ClickRP);
    if (ed->ed_ClickMap = AllocBitMap(BITMAP_WIDTH,BITMAP_HEIGHT,BITMAP_DEPTH,BMF_INTERLEAVED|BMF_MINPLANES,NULL))
    {
        ed->ed_ClickRP.BitMap = ed->ed_ClickMap;
        SetFont(&ed->ed_ClickRP,ed->ed_Font);

        i = 0;
        while (i < NUMDRIPENS)
        {
            pens[i] = i;
            i++;
        }
        pens[i] = ~0;

        di.dri_Version      = DRI_VERSION;
        di.dri_NumPens      = NUM_KNOWNPENS;
        di.dri_Pens         = pens;
        di.dri_Font         = ed->ed_Font;
        di.dri_Depth        = 4;
        di.dri_Resolution.X = 10;
        di.dri_Resolution.Y = 11;
        di.dri_Flags        = DRIF_NEWLOOK;
        di.dri_CheckMark    = NULL;
        di.dri_AmigaKey     = NULL;

        DoSample(ed,TRUE,&ed->ed_ClickRP,0,&di,pens,NULL);
    }
}


/*****************************************************************************/


VOID DisposeClickMap(EdDataPtr ed)
{
    FreeBitMap(ed->ed_ClickMap);
}


/*****************************************************************************/


WORD WhichPen(EdDataPtr ed, WORD x, WORD y)
{
struct Window *wp;
UWORD          top;

    if (ed->ed_ClickMap)
    {
        wp  = ed->ed_Window;
        top = wp->BorderTop + 187;
        if (ed->ed_SampleWindow)
        {
            wp  = ed->ed_SampleWindow;
            top = wp->BorderTop;
        }

        y -= top;

        if ((x < 0) || (y < 0) || (x >= BITMAP_WIDTH) || (y >= BITMAP_HEIGHT))
            return(-1);

        return (ReadPixel(&ed->ed_ClickRP,x,y));
    }

    return (-1);
}
