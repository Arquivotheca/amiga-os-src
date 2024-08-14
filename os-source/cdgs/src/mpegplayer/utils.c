
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/rpattr.h>

#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include "mpegplayerbase.h"
#include "utils.h"


/*****************************************************************************/


// BltTemplate(), but constrain the blit vertically, and to the right, within
// the given rectangle
static VOID ConstrainBlit(struct MPEGPlayerLib *MPEGPlayerBase,
                          struct BitMap *source, struct RastPort *rp,
                          struct Rectangle *constraint,
                          WORD x, WORD y, WORD w, WORD h)
{
APTR mask;
WORD x1, y1;

    mask = source->Planes[0];

    if (constraint)
    {
        x1 = x+w-1;
        y1 = y+h-1;

        if (x1 > constraint->MaxX)
            x1 = constraint->MaxX;

        if (y < constraint->MinY)
        {
            mask = (APTR)((ULONG)mask + (ULONG)source->BytesPerRow * (constraint->MinY - y));
            y    = constraint->MinY;
        }

        if (y1 > constraint->MaxY)
            y1 = constraint->MaxY;

        if (x1 < x)
            return;

        if (y1 < y)
            return;

        w = x1-x+1;
        h = y1-y+1;
    }

    BltTemplate(mask,0,source->BytesPerRow,rp,x,y,w,h);
}


/*****************************************************************************/


VOID ShadowText(struct MPEGPlayerLib *MPEGPlayerBase,
                struct RastPort *rp, STRPTR text, UWORD len,
                struct Rectangle *constraint)
{
struct BitMap   *bm;
struct BitMap   *bm2;
struct RastPort  trp;
ULONG            oldapen, oldbpen, oldmode;

    if (len == 0)
        return;

    bm = AllocBitMap(704,40,1,BMF_CLEAR|BMF_INTERLEAVED,NULL);
    InitRastPort(&trp);
    trp.BitMap = bm;

    SetABPenDrMd(&trp,1,0,JAM2);
    SetFont(&trp,rp->Font);

    // we move to an X position of 2, since the compact font has a
    // few chars with negative kerning. If we don't move to "2", then
    // we run the risk of overwriting memory before the bitmap by 2 pixels...
    Move(&trp,2,rp->TxBaseline);
    Text(&trp,text,len);

    bm2 = AllocBitMap(704,40,1,BMF_INTERLEAVED,NULL);

    BltBitMap(bm,0,0,bm2,0,0,trp.cp_x,trp.TxHeight,0xc0,0x01,NULL);
    BltBitMap(bm,0,0,bm,1,0,trp.cp_x,trp.TxHeight,0xe0,0x01,NULL);
    BltBitMap(bm,0,0,bm,3,0,trp.cp_x+1,trp.TxHeight,0xe0,0x01,NULL);
    BltBitMap(bm,0,0,bm,0,1,trp.cp_x+3,trp.TxHeight,0xe0,0x01,NULL);
    BltBitMap(bm,0,0,bm,0,2,trp.cp_x+3,trp.TxHeight+1,0xe0,0x01,NULL);

    GetRPAttrs(rp,RPTAG_APen, &oldapen,
                  RPTAG_BPen, &oldbpen,
                  RPTAG_DrMd, &oldmode,
                  TAG_DONE);

    SetABPenDrMd(rp,oldbpen,0,oldmode);
    ConstrainBlit(MPEGPlayerBase,bm,rp,constraint,rp->cp_x,rp->cp_y - trp.TxBaseline,trp.cp_x+3,trp.TxHeight + 3);

    SetABPenDrMd(rp,oldapen,0,JAM1);
    ConstrainBlit(MPEGPlayerBase,bm2,rp,constraint,rp->cp_x + 1,rp->cp_y - trp.TxBaseline + 1, trp.cp_x, trp.TxHeight);

    SetABPenDrMd(rp,oldapen,oldbpen,oldmode);
    Move(rp,rp->cp_x + trp.cp_x + 3, rp->cp_y);

    FreeBitMap(bm);
    WaitBlit();
    FreeBitMap(bm2);
}


/*****************************************************************************/


UWORD ShadowTextLength(struct MPEGPlayerLib *MPEGPlayerBase,
                       struct RastPort *rp, STRPTR text, UWORD len)
{
    if (len == 0)
        return 0;

    return ((UWORD)(TextLength(rp,text,len)+5));
}


/*****************************************************************************/


UWORD ShadowFit(struct MPEGPlayerLib *MPEGPlayerBase,
                struct RastPort *rp, STRPTR text, UWORD len, UWORD max)
{
struct TextExtent extent;

    if ((max <= 5) || (len == 0))
        return 0;

    return((UWORD)TextFit(rp,text,len,&extent,NULL,1,max-5,32767));
}


/*****************************************************************************/


VOID ClipText(struct MPEGPlayerLib *MPEGPlayerBase,
              struct RastPort *rp, STRPTR text, UWORD len, UWORD max)
{
    ShadowText(MPEGPlayerBase,rp,text,ShadowFit(MPEGPlayerBase,rp,text,len,max),NULL);
}


/*****************************************************************************/


VOID CenterText(struct MPEGPlayerLib *MPEGPlayerBase,
                struct RastPort *rp, STRPTR text, UWORD len, UWORD max)
{
UWORD pix;

    pix = ShadowTextLength(MPEGPlayerBase,rp,text,len);

    if (pix <= max)
    {
        Move(rp,rp->cp_x + (max-pix) / 2,rp->cp_y);
        ShadowText(MPEGPlayerBase,rp,text,len,NULL);
    }
    else
    {
        ClipText(MPEGPlayerBase,rp,text,len,max);
    }
}


/*****************************************************************************/


#define NUM_FADE_STEPS 16

VOID FadeIn(struct MPEGPlayerLib *MPEGPlayerBase, BOOL midWay)
{
ULONG            i,j,start;
struct ColorPack colors;

    colors.cp_NumColors  = NUM_COLORS;
    colors.cp_FirstColor = 0;
    colors.cp_EndMarker  = 0;

    start = 0;
    if (midWay)
        start = 6;

    for (i = start; i <= NUM_FADE_STEPS; i++)
    {
        for (j = 0; j < NUM_COLORS; j++)
        {
            colors.cp_Colors[j].Red   = (MPEGPlayerBase->mp_BgColors.cp_Colors[j].Red / NUM_FADE_STEPS) * i;
            colors.cp_Colors[j].Green = (MPEGPlayerBase->mp_BgColors.cp_Colors[j].Green / NUM_FADE_STEPS) * i;
            colors.cp_Colors[j].Blue  = (MPEGPlayerBase->mp_BgColors.cp_Colors[j].Blue / NUM_FADE_STEPS) * i;
        }

        LoadRGB32(&MPEGPlayerBase->mp_Screen->ViewPort,(ULONG *)&colors);
    }

    LoadRGB32(&MPEGPlayerBase->mp_Screen->ViewPort,(ULONG *)&MPEGPlayerBase->mp_BgColors);
}


/*****************************************************************************/


VOID FadeOut(struct MPEGPlayerLib *MPEGPlayerBase, BOOL midWay)
{
ULONG            i,j,end;
struct ColorPack colors;

    colors.cp_NumColors  = NUM_COLORS;
    colors.cp_FirstColor = 0;
    colors.cp_EndMarker  = 0;

    end = NUM_FADE_STEPS;
    if (midWay)
        end = NUM_FADE_STEPS-6;

    for (i = 0; i <= end; i++)
    {
        for (j = 0; j < NUM_COLORS; j++)
        {
            colors.cp_Colors[j].Red   = (MPEGPlayerBase->mp_BgColors.cp_Colors[j].Red / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
            colors.cp_Colors[j].Green = (MPEGPlayerBase->mp_BgColors.cp_Colors[j].Green / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
            colors.cp_Colors[j].Blue  = (MPEGPlayerBase->mp_BgColors.cp_Colors[j].Blue / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
        }

        LoadRGB32(&MPEGPlayerBase->mp_Screen->ViewPort,(ULONG *)&colors);
    }

    if (!midWay)
    {
        for (i = 0; i < NUM_COLORS; i++)
        {
            colors.cp_Colors[i].Red   = 0;
            colors.cp_Colors[i].Green = 0;
            colors.cp_Colors[i].Blue  = 0;
        }
        LoadRGB32(&MPEGPlayerBase->mp_Screen->ViewPort,(ULONG *)&colors);
    }
}


/*****************************************************************************/


ULONG __asm OpenWorkBenchPatch(void)
{
    return(0);
}


/*****************************************************************************/


VOID CloseScreenQuiet(struct MPEGPlayerLib *MPEGPlayerBase,
                      struct Screen *screen)
{
APTR OWB;

    if (screen)
    {
        OWB = SetFunction(IntuitionBase, -0xd2, (APTR)OpenWorkBenchPatch);
        CloseScreen(screen);
        SetFunction(IntuitionBase, -0xd2, OWB);
    }
}


/*****************************************************************************/


//#define ICOLOR(c) (*((SHORT *)0xdff180)=(c))
//#define ICOLOR(c) ;

void SwapBuffers(struct MPEGPlayerLib *MPEGPlayerBase)
{
    WaitBlit();
#if (DOUBLE_BUFFER)
    ChangeScreenBuffer(MPEGPlayerBase->mp_Screen,MPEGPlayerBase->mp_DBBuffers[MPEGPlayerBase->mp_DBCurrentBuffer]);

    WaitPort(MPEGPlayerBase->mp_DBNotify);
    GetMsg(MPEGPlayerBase->mp_DBNotify);

    MPEGPlayerBase->mp_DBCurrentBuffer = 1 - MPEGPlayerBase->mp_DBCurrentBuffer;

    MPEGPlayerBase->mp_RenderRP = &MPEGPlayerBase->mp_DBRastPort[MPEGPlayerBase->mp_DBCurrentBuffer];
#endif
}


/*****************************************************************************/


VOID GetBM(struct MPEGPlayerLib *MPEGPlayerBase, APTR deboxData,
           struct BitMap **resultBM, struct BMInfo **resultBMInfo)
{
    // BMF_MINPLANES is required by the init code, don't remove!

    *resultBMInfo = DecompBMInfo(NULL, NULL, deboxData);
    *resultBM     = AllocBitMap((*resultBMInfo)->bmi_Width,(*resultBMInfo)->bmi_Height,(*resultBMInfo)->bmi_Depth, BMF_MINPLANES, NULL);
    DecompBitMap(NULL, deboxData, *resultBM, NULL);
}


/*****************************************************************************/


void CloseWindowSafely(struct MPEGPlayerLib *MPEGPlayerBase, struct Window *wp)
{
struct IntuiMessage *msg;
struct Node         *succ;

    Forbid();

    msg = (struct IntuiMessage *) wp->UserPort->mp_MsgList.lh_Head;
    while (succ = msg->ExecMessage.mn_Node.ln_Succ)
    {
        if (msg->IDCMPWindow == wp)
        {
            Remove(msg);
            ReplyMsg(msg);
        }
        msg = (struct IntuiMessage *) succ;
    }

    wp->UserPort = NULL;
    ModifyIDCMP(wp,NULL);

    Permit();

    CloseWindow(wp);
}


/*****************************************************************************/


UWORD Random(WORD range, ULONG *seed)
{
LONG rseed;
LONG rval;
extern UWORD volatile __far vhposr;     /* Custom chip register */

    rseed = *seed;
    if  (rseed == 0)
        rseed = 323214521 + vhposr;

    rseed = rseed * 123213 + 121;
    rval  = (rseed >> 5) & 65535;
    *seed = rseed;

    return ((UWORD)((range * rval) >> 16));
}
