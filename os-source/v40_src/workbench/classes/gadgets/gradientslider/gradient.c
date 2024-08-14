
/* "Color Gradient" slider BOOPSI gadget
 * By TALIN (with Joe Pearce)
 */


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/classusr.h>
#include <intuition/classes.h>
#include <intuition/cghooks.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxmacros.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/macros.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "gradient.h"
#include "utils.h"
#include "gradientslider.h"


/*****************************************************************************/


#define INSET_W         2          /* container inset from bevel      */
#define INSET_H         1

#define BORDERTHICK_W   1          /* thickness of container          */
#define BORDERTHICK_H   1

#define KNOB_SLOP       2          /* sloppiness allowed in knob pick */
#define KNOB_PIXELS     5          /* size of slider knob             */


/*****************************************************************************/


/* 16 4x4 Half-Tone Dither Patterns for Container (each pat is 16x8) */
UWORD dither_data[] =
{
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x1111,0x0000,0x0000,0x0000,0x1111,0x0000,0x0000,0x0000,
    0x1111,0x0000,0x4444,0x0000,0x1111,0x0000,0x4444,0x0000,
    0x1111,0x0000,0x5555,0x0000,0x1111,0x0000,0x5555,0x0000,
    0x5555,0x0000,0x5555,0x0000,0x5555,0x0000,0x5555,0x0000,
    0x5555,0x2222,0x5555,0x0000,0x5555,0x2222,0x5555,0x0000,
    0x5555,0x2222,0x5555,0x8888,0x5555,0x2222,0x5555,0x8888,
    0x5555,0x2222,0x5555,0xAAAA,0x5555,0x2222,0x5555,0xAAAA,
    0x5555,0xAAAA,0x5555,0xAAAA,0x5555,0xAAAA,0x5555,0xAAAA,
    0x5555,0xBBBB,0x5555,0xAAAA,0x5555,0xBBBB,0x5555,0xAAAA,
    0x5555,0xBBBB,0x5555,0xEEEE,0x5555,0xBBBB,0x5555,0xEEEE,
    0x5555,0xBBBB,0x5555,0xFFFF,0x5555,0xBBBB,0x5555,0xFFFF,
    0x5555,0xFFFF,0x5555,0xFFFF,0x5555,0xFFFF,0x5555,0xFFFF,
    0x7777,0xFFFF,0x5555,0xFFFF,0x7777,0xFFFF,0x5555,0xFFFF,
    0x7777,0xFFFF,0xDDDD,0xFFFF,0x7777,0xFFFF,0xDDDD,0xFFFF,
    0x7777,0xFFFF,0xFFFF,0xFFFF,0x7777,0xFFFF,0xFFFF,0xFFFF,
/*  0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF */
};


/*****************************************************************************/


LONG Clamp(LONG min, LONG val, LONG max)
{
    if (val < min)
        return min;

    if (val > max)
        return max;

    return val;
}


/*****************************************************************************/


/* InsetIBox: Expands or contracts all four sizes of an IBox
 *
 *  This function moves the left and right edges of an IBox inward by the
 *  amount specified in sx, and the top and bottom edges by sy.
 */

VOID InsetIBox(struct IBox *b, LONG sx, LONG sy)
{
    b->Left   += sx;
    b->Width  -= sx + sx;
    b->Top    += sy;
    b->Height -= sy + sy;
}


/*****************************************************************************/


/* DrawFrame: Sort of like DrawBevelBox, but just one color */
VOID DrawFrame(struct RastPort *rp, struct IBox *b)
{
    Move(rp, b->Left, b->Top);
    Draw(rp, b->Left, b->Top + b->Height - 1);
    Draw(rp, b->Left + b->Width - 1, b->Top + b->Height - 1);
    Draw(rp, b->Left + b->Width - 1, b->Top);
    Draw(rp, b->Left, b->Top);
}


/*****************************************************************************/


/* RenderSlider: Draw the various parts of the slider gadget.
 *
 *  This function actually draws the gadget. It has sections for each part of
 *  the gadget that might be drawn, such as the knob, the
 *  border, etc. The redraw argument controls which sections actually do
 *  get drawn since it is uneccessary to re-render the entire gadget each time.
 *
 *  Note that I have been careful to draw no pixel more than once. This helps
 *  to minimize flickering when dragging the knob.
 */

VOID RenderSlider(struct Gadget     *gadget,
                  struct SliderInfo *si,
                  struct RastPort   *rp,
                  struct GadgetInfo *ginfo,
                  WORD              redraw)
{
WORD                ytop, ybottom;                 /* extent of container          */
WORD                xleft, xright;
struct DrawInfo     *drinfo = ginfo->gi_DrInfo;    /* DrawInfo for screen          */
UWORD               *pens = drinfo->dri_Pens;      /* array of pens                 */

    SetABPenDrMd(rp,pens[BACKGROUNDPEN],0,JAM1);

    /* render the frame around the container if needed */

    if (redraw == GREDRAW_REDRAW)
    {
    struct IBox GBox;

        /* render the bevel box around the slider */

        GBox = si->ContainerExtent;
        DrawFrame(rp, &GBox);
        InsetIBox(&GBox,-INSET_W,-INSET_H);
        QuickBevel(rp,&GBox,pens[SHINEPEN],pens[SHADOWPEN]);
    }

    /* figure out which rectangles to draw to render the container */

    xleft   = si->ContainerExtent.Left;
    xright  = si->ContainerExtent.Left + si->ContainerExtent.Width - 1;
    ytop    = si->ContainerExtent.Top;
    ybottom = si->ContainerExtent.Top + si->ContainerExtent.Height - 1;

    if (si->PenCount)
    {
        xleft   += BORDERTHICK_W;
        xright  -= BORDERTHICK_W;
        ytop    += BORDERTHICK_H;
        ybottom -= BORDERTHICK_H;
    }

    if (si->VerticalSlider)
    {
    WORD yslider_start, yslider_stop;

        yslider_start = si->KnobExtent.Top;
        yslider_stop  = si->KnobExtent.Top + si->KnobExtent.Height;

        if (xright < xleft)
            return;

        /* Code to draw a vertical half-tone gradient */

        if (si->DitherBMap) /* If there is an offscreen dither pattern */
        {
            if (yslider_start > ytop)
            {
                BltBitMapRastPort(si->DitherBMap,0,0,rp,xleft,ytop,
                                  xright - xleft + 1,yslider_start-ytop,0xc0);
            }

            if (ybottom > yslider_stop)
            {
                BltBitMapRastPort(si->DitherBMap,0,yslider_stop - ytop,
                                  rp,xleft,yslider_stop,
                                  xright - xleft + 1,ybottom - yslider_stop + 1,0xc0);
            }
        }
        else    /* If no pen-array or no dither bitmap, then just draw constant color */
        {
            SetAPen(rp,si->PenCount ? si->PenArray[0] : pens[BACKGROUNDPEN]);

            if (yslider_start > ytop)
                RectFill(rp,xleft,ytop,xright,yslider_start-1);

            if (ybottom > yslider_stop)
                RectFill(rp,xleft,yslider_stop,xright,ybottom);
        }
    }
    else
    {
    WORD xslider_start, xslider_stop;

        xslider_start = si->KnobExtent.Left;
        xslider_stop = si->KnobExtent.Left + si->KnobExtent.Width;

        if (ybottom < ytop)
            return;

        if (si->DitherBMap) /* If there is an offscreen dither pattern */
        {
            if (xslider_start > xleft)
            {
                BltBitMapRastPort(si->DitherBMap,0,0,rp,xleft,ytop,
                                  xslider_start-xleft,ybottom - ytop + 1,0xc0);
            }

            if (xright > xslider_stop)
            {
                BltBitMapRastPort(si->DitherBMap,xslider_stop - xleft,0,
                                  rp,xslider_stop,ytop,
                                  xright - xslider_stop + 1,ybottom - ytop + 1,0xc0);
            }
        }
        else    /* If no pen-array or dither bitmap, then just draw constant color */
        {
            SetAPen(rp,si->PenCount ? si->PenArray[0] : pens[BACKGROUNDPEN]);

            if (xslider_start > xleft)
                RectFill(rp,xleft,ytop,xslider_start-1,ybottom);

            if (xright > xslider_stop)
                RectFill(rp,xslider_stop,ytop,xright,ybottom);
        }
    }

    /* render the actual knob */

    if (!si->PenCount)
    {
        /* If the PenArray is empty (i.e. a "Plain" slider) then just
         *  render the knob as a black box.
         */
        SetAPen(rp,pens[SHADOWPEN]);
        RectFill(rp, si->KnobExtent.Left,
                     si->KnobExtent.Top,
                     si->KnobExtent.Left + si->KnobExtent.Width - 1,
                     si->KnobExtent.Top  + si->KnobExtent.Height - 1 );
    }
    else
    {
        SetAPen(rp, pens[BACKGROUNDPEN]);
        DrawFrame(rp, &si->KnobExtent);

        if (gadget->Flags & GFLG_SELECTED)
            SetAPen(rp, pens[FILLPEN]);
        else
            SetAPen(rp, pens[BLOCKPEN]);

        RectFill(rp,si->KnobExtent.Left + BORDERTHICK_W,
                    si->KnobExtent.Top  + BORDERTHICK_H,
                    si->KnobExtent.Left + si->KnobExtent.Width - BORDERTHICK_W - 1,
                    si->KnobExtent.Top  + si->KnobExtent.Height - BORDERTHICK_H - 1 );
    }
}


/*****************************************************************************/


/* CalcSliderPosition: Calculate the current position of the slider.
 *
 *  Inputs: A SliderInfo structure and a pointer to the gadget.
 *
 *  This function first figures out the size of the knob and container.
 *  It then calculates where the knob might actually be positioned, based on
 *  the values (min,max,current,span) of the slider, as well as the settings
 *  of various flags.
 */

static VOID CalcSliderPosition(struct SliderInfo *si, struct Gadget *g)
{
LONG slide_value;
WORD body_free;
LONG container_size;

    si->KnobExtent = si->ContainerExtent;       /* initialize Knob to fill size */

    container_size = si->VerticalSlider ? si->ContainerExtent.Height : si->ContainerExtent.Width;

    if ((container_size <= 0) || (si->MaxValue <= 0))
        return;     /* no "negative" sliders, or <0 box sizes */

    si->PixelRange = container_size;
    si->PixelSpan  = MIN(si->KnobPixels,container_size); /* body size in pixels          */

    /* value of slider, clamped to range */

    slide_value = Clamp(0, si->CurrentValue, si->MaxValue);

    /* calculate new slider position - return if body == infinite... */

    body_free = si->PixelRange - si->PixelSpan;

    slide_value = ((slide_value * body_free) + (si->MaxValue/2)) / si->MaxValue;
    si->PixelPos = slide_value;

    if (si->VerticalSlider)
    {
        si->KnobExtent.Top += slide_value;
        si->KnobExtent.Height = si->PixelSpan;
    }
    else
    {
        si->KnobExtent.Left += slide_value;
        si->KnobExtent.Width = si->PixelSpan;
    }
}


/*****************************************************************************/


/* SetupContainerInfo: Fill in the SliderInfo structure for this gadget
 *
 *  This function computes the various "derived" fields in the SliderInfo
 *  structure, such as the actual size of the container (which may be inset from
 *  the actual gadget hit box), and also calls SetupIBox to figure out how big
 *  the gadget currently is based on the size of the window.
 */

static VOID SetupContainerInfo(struct Gadget *g, struct SliderInfo *si,
                               struct GadgetInfo *gi)
{
    /* Compute the gadget's current select box based on window size
     * and gadget relativity.
     */
    si->ContainerExtent.Width  = g->Width;
    si->ContainerExtent.Height = g->Height;
    si->ContainerExtent.Left   = g->LeftEdge;
    si->ContainerExtent.Top    = g->TopEdge;

    /* Shrink the container rectangle */
    InsetIBox(&si->ContainerExtent,INSET_W,INSET_H);
}


/*****************************************************************************/


/* SetupDitherBMap: Attempt to allocate an offscreen bitmap for dither pattern
 *
 *  This function attempts to allocate an offscreen bitmap to store the
 *  color gradient image for the container.
 */

static VOID SetUpDitherBMap(struct Gadget *g, struct SliderInfo *si,
                            struct RastPort *rp)
{
struct RastPort drp;
UWORD           depth;
UWORD           width;
UWORD           height;
WORD            containersize;
WORD            segments;
WORD            lastsegpos;
WORD            i;

    if (si->DitherBMap && (si->LastWidth == g->Width) && (si->LastHeight == g->Height))
        return;

    FreeBitMap(si->DitherBMap);
    si->DitherBMap = NULL;

    si->LastWidth  = g->Width;
    si->LastHeight = g->Height;

    if (si->PenCount <= 1)
        return;

    depth  = GetBitMapAttr(rp->BitMap,BMA_DEPTH);
    width  = si->ContainerExtent.Width - 2 * BORDERTHICK_W;
    height = si->ContainerExtent.Height - 2 * BORDERTHICK_H;

    /* try allocating a friend bitmap */
    if (!(si->DitherBMap = AllocBitMap(width,height,depth,0,rp->BitMap)))
        return;

    InitRastPort(&drp);
    drp.BitMap = si->DitherBMap;

    /* Code to draw a half-tone gradient */

    containersize = (si->VerticalSlider ? height : width);
    segments = ((si->PenCount - 1) << 4) + 1;

    lastsegpos = 0;

    for (i=0; i<segments; i++)
    {
    WORD nextsegpos;
    WORD ditherpat;

        nextsegpos = (containersize * (i + 1)) / segments;
        ditherpat = i & 15;

        SetAfPt(&drp, &dither_data[ditherpat * 8], 3);
        if (ditherpat == 0)
            SetABPenDrMd(&drp,si->PenArray[(i>>4) + 1],si->PenArray[i>>4],JAM2);

        if (nextsegpos > lastsegpos)
        {
            if (si->VerticalSlider)
                RectFill(&drp,0,lastsegpos,width-1,nextsegpos-1);
            else
                RectFill(&drp,lastsegpos,0,nextsegpos-1,height-1);
        }

        lastsegpos = nextsegpos;
    }
}


/*****************************************************************************/


/* HandleFunc: Gadget handler for low-level Intuition events.
 *
 *  This function handles all the low-level Intuition events for the gadget,
 *  except for pure rendering (which is handles by drawfunc) and the initial
 *  hit test (which is handles by hitfunc).
 */

static ULONG HandleFunc(Class *cl, struct gpInput *msg, struct Gadget *g)
{
struct SliderInfo *si =INST_DATA( cl, g );/* state    */
struct SliderInfo  save_si;                /* copy of slider state         */
UWORD              oldSelected;
ULONG              result = GMR_MEACTIVE;  /* result of this function      */
BOOL               draw;
LONG               newval = si->CurrentValue; /* new slider value         */
BOOL               final  = FALSE;

    /* copy the SliderInfo so that we can test for changes later */

    save_si     = *si;
    oldSelected = (g->Flags & GFLG_SELECTED);

    /* if it's a GM_GOACTIVE it must mean somebody clicked on us. */

    if (msg->MethodID == GM_GOACTIVE)           /* classify type of hit         */
    {
    UWORD mpos, inset;

        si->SaveValue = si->CurrentValue;

        SetupContainerInfo(g,si,msg->gpi_GInfo);

        inset = (si->VerticalSlider) ? INSET_H : INSET_W;
        mpos  = (si->VerticalSlider) ? msg->gpi_Mouse.Y : msg->gpi_Mouse.X;

        if (mpos - inset + KNOB_SLOP < si->PixelPos)
        {
            newval -= si->SkipValue;
            result  = (GMR_NOREUSE | GMR_VERIFY);
        }
        else if (mpos - inset - KNOB_SLOP >= si->PixelPos + si->PixelSpan)
        {
            newval += si->SkipValue;
            result  = (GMR_NOREUSE | GMR_VERIFY);
        }
        else
        {
            si->MouseOffset  = mpos - si->PixelPos;
            g->Flags        |= GFLG_SELECTED;
        }
    }
    else if (msg->MethodID == GM_GOINACTIVE)
    {
        g->Flags &= ~GFLG_SELECTED;
        final     = TRUE;                           /* we are done                  */
        result    = 0;                             /* mystery value needed by Intuition */
    }

    if (msg->gpi_IEvent->ie_Class == IECLASS_RAWMOUSE)
    {
        if (msg->gpi_IEvent->ie_Code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
        {
            /* check for mouse up */

            g->Flags &= ~GFLG_SELECTED;
            result    = (GMR_NOREUSE | GMR_VERIFY);
        }
        else if (msg->gpi_IEvent->ie_Code == IECODE_RBUTTON)
        {
            /* check for menu button hit */

            newval    = si->SaveValue;
            g->Flags &= ~GFLG_SELECTED;
            result    = (GMR_NOREUSE | GMR_VERIFY);
        }
        else if (g->Flags & GFLG_SELECTED)
        {
        WORD body_free;

            /* knob: handle dragging of knob */

            body_free = si->PixelRange - si->PixelSpan;
            newval = si->MaxValue;

            if (newval <= 0 || body_free <= 0)
            {
                newval = 0;
            }
            else
            {
            WORD mousepos;

                mousepos = Clamp(0,
                    ((si->VerticalSlider) ?
                        msg->gpi_Mouse.Y : msg->gpi_Mouse.X)
                        - si->MouseOffset,
                    body_free);

                newval = ((mousepos * newval) + body_free/2) / body_free;
            }
        }
    }

    /* clamp the calculated slider value between the limits as defined
     * by min and max.
     */

    si->CurrentValue = Clamp(0, newval, si->MaxValue);

    /* if highlight state changed, then redraw the knob */
    draw = ((g->Flags & GFLG_SELECTED) != oldSelected);

    /* if knob position changed, then redraw the knob */
    if (si->CurrentValue != save_si.CurrentValue)
    {
        CalcSliderPosition(si,g);

        if (save_si.PixelPos != si->PixelPos)
            draw = TRUE;
    }

    /* re-draw the knob here. */

    if (draw)
    {
    struct RastPort *rp;

        if (rp = ObtainGIRPort(msg->gpi_GInfo))
        {
            DoMethod( (Object *)g, GM_RENDER, msg->gpi_GInfo, rp, GREDRAW_UPDATE);
            ReleaseGIRPort(rp);
        }
    }

    if ((result & ~GMR_VERIFY) == GMR_NOREUSE)
        final = TRUE;

    /* "final" is a flag which indicates if the slider has stopped moving */

    if (si->CurrentValue != save_si.CurrentValue || final)
    {
    struct TagItem output_tags[3];

        output_tags[0].ti_Tag  = GA_ID;
        output_tags[0].ti_Data = g->GadgetID;
        output_tags[1].ti_Tag  = GRAD_CurVal;
        output_tags[1].ti_Data = si->CurrentValue;
        output_tags[2].ti_Tag  = TAG_DONE;

        DoMethod( (Object *)g, OM_NOTIFY, output_tags,msg->gpi_GInfo,
                 final ? 0 : OPUF_INTERIM);
    }

    return result;
}


/*****************************************************************************/


/* SetSliderAttrs: handles attribute changes for slider.
 *
 *  This function updates the slider variables based on a tag list. It is called
 *  by several different functions below.
 *
 *  The function returns a non-zero value if any of the data structures were
 *  actually changed.
 */

static LONG SetSliderAttrs(Class *cl, Object *o, struct opSet *msg, BOOL init)
{
struct TagItem    *tags = msg->ops_AttrList;
struct TagItem    *tlist = tags;
struct SliderInfo *si = INST_DATA( cl, o );
struct SliderInfo  old_si;
LONG               result = 0;

    old_si = *si;                               /* save old state               */

    /* iterate through the other tags and setup what values are needed */

    while (tags = NextTagItem(&tlist))
    {
        switch (tags->ti_Tag)
        {
            case PGA_Freedom     : if (init)
                                   {
                                       if (tags->ti_Data == LORIENT_VERT)
                                           si->VerticalSlider = TRUE;
                                       else
                                           si->VerticalSlider = FALSE;
                                   }
                                   break;

            case GRAD_MaxVal     : si->MaxValue     = tags->ti_Data;
                                   si->CurrentValue = Clamp(0,si->CurrentValue,si->MaxValue);
                                   break;

            case GRAD_CurVal     : si->CurrentValue = Clamp(0,tags->ti_Data,si->MaxValue);
                                   break;

            case GRAD_KnobPixels : if (init)
                                       si->KnobPixels = tags->ti_Data;
                                   break;

            case GRAD_PenArray   : si->PenCount = 0;
                                   if (si->PenArray = (UWORD *)tags->ti_Data)
                                   {
                                       while (si->PenArray[si->PenCount] != ~0)
                                           si->PenCount++;
                                   }
                                   result = 1;
                                   break;
        }
    }

    if ((old_si.MaxValue != si->MaxValue) || (old_si.CurrentValue != si->CurrentValue))
        result = 1;

    return (result);
}


/*****************************************************************************/


static ULONG GetFunc(Class *cl, struct opGet *msg, struct Gadget *g)
{
    if (msg->opg_AttrID == GRAD_CurVal)
    {
    struct SliderInfo *si = INST_DATA( cl, g );

        *msg->opg_Storage = (ULONG)si->CurrentValue;
        return 1;
    }

    return (ULONG) DoSuperMethodA( cl, (Object *)g, msg );
}


/*****************************************************************************/


static Object *NewFunc(Class *cl, struct opSet *msg, struct Gadget *g)
{
struct Gadget *newobj;

    /* this is the "default" slider info for newly created sliders */

    if ( newobj = (struct Gadget *) DoSuperMethodA( cl, (Object *)g, msg ) )
    {
    struct SliderInfo *si = INST_DATA( cl, newobj );

        /* set up some defaults for the slider instance variables */
        si->MaxValue   = 0xffff;
        si->SkipValue  = 0x1111;
        si->KnobPixels = KNOB_PIXELS;

        /* set up the slider attributes from the tag list */
        SetSliderAttrs( cl, (Object *)newobj, msg, TRUE );
    }

    return (Object *)newobj;
}


/*****************************************************************************/


static ULONG DrawFunc(Class *cl, struct gpRender *msg, struct Gadget *g)
{
    if (msg->gpr_Redraw == GREDRAW_REDRAW || msg->gpr_Redraw == GREDRAW_UPDATE)
    {
    struct SliderInfo *si = INST_DATA( cl, g );  /* state */

        SetupContainerInfo(g,si,msg->gpr_GInfo);
        SetUpDitherBMap(g,si,msg->gpr_RPort);
        CalcSliderPosition(si,g);
        RenderSlider(g,si,msg->gpr_RPort,msg->gpr_GInfo,msg->gpr_Redraw);
    }

    return 0;
}


/*****************************************************************************/


static ULONG UpdateFunc(Class *cl, struct opSet *msg, struct Gadget *g)
{
LONG             refresh;
LONG             flags = g->Flags;
struct RastPort *rp;

    refresh  = DoSuperMethodA( cl, (Object *)g, msg );
    refresh |= SetSliderAttrs( cl, (Object *)g, msg, FALSE );  /* set attibutes              */
    refresh |= (g->Flags ^ flags) & GFLG_SELECTED;             /* update if SELECTED changed */

    if (refresh && ( OCLASS(g) == cl ) && msg->ops_GInfo)
    {
        if (rp = ObtainGIRPort(msg->ops_GInfo))
        {
            DoMethod( (Object *)g, GM_RENDER, msg->ops_GInfo, rp, GREDRAW_REDRAW);
            ReleaseGIRPort(rp);
            return 0L;                          /* don't need refresh any more */
        }
    }

    return (ULONG) refresh;
}


/*****************************************************************************/


LONG ASM ClassDispatcher(REG(a0) Class         *cl,
                         REG(a1) ULONG         *msg,
                         REG(a2) struct Gadget *g)
{
struct SliderInfo *si = INST_DATA( cl, g );

    switch (*msg)
    {
        case GM_RENDER     : return (LONG)DrawFunc(cl, (struct gpRender *)msg, g);

        case GM_HITTEST    : return(GMR_GADGETHIT);

        case GM_GOACTIVE   : if (((struct gpInput *)msg)->gpi_IEvent == NULL)
                                 return(GMR_NOREUSE);

        case GM_HANDLEINPUT:
        case GM_GOINACTIVE : return (LONG)HandleFunc(cl, (struct gpInput *)msg,g);

        case OM_NEW        : return (LONG)NewFunc(cl, (struct opSet *)msg, g);

        case OM_SET        :
        case OM_UPDATE     : return (LONG)UpdateFunc(cl, (struct opSet *)msg, g);

        case OM_GET        : return (LONG)GetFunc(cl, (struct opGet *)msg, g);

        case OM_DISPOSE    : FreeBitMap(si->DitherBMap);

        default            : return DoSuperMethodA( cl, (Object *)g, (Msg *)msg );
    }
}
