
/* ColorWheel BOOPSI gadget module
 * By Joe Pearce (with Talin)
 */


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/cghooks.h>
#include <intuition/classusr.h>
#include <intuition/classes.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxmacros.h>
#include <gadgets/gradientslider.h>

#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/macros.h>

#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "colorwheel.h"
#include "wheel.h"
#include "sine.h"
#include "utils.h"
#include "root.h"


/*****************************************************************************/


BOOL InitColorWheelStruct(struct Wheel *wheel, struct Screen *scr);
static VOID WheelUpdate(struct GadgetInfo *gi, struct Gadget *g);
#define ShiftColor32(a) (((ULONG)(a) << 16) | (a))


/*****************************************************************************/


#define SIN_ONE ((1L << 15) - 1)

/* The color abbreviation letters are always rendered in TOPAZ/8 */
static struct TextAttr topaz8_ta = { "topaz.font", 8, };

/* the dither patterns for the lightness rings */
UWORD lite_pat[2] = { 0x5555, 0x0000 };
UWORD med_pat[2] = { 0x5555, 0xaaaa };
UWORD dark_pat[2] = { 0x5555, 0xffff };
UWORD ghost_pat[2] = { 0x8888, 0x2222 };

/* This table has the list of colors that the color wheel allocates,
 * in allocation order. The color wheel attempts to allocate colors in
 * order of importance. It first allocates white. Then, it allocates
 * the three additive primary colors, which are evenly spaced
 * 1/3 of the circumference apart. Then, it splits the distance
 * between these colors and allocates the secondary colors, which
 * are the midpoints between the three primaries. Then it splits
 * the distances again, which doubles the number of colors arranged
 * around the circumference. This process continues until the
 * end of the list is reached, or a failure is detected. In either
 * case, this determines what layout type the wheel will use.
 */

UBYTE hue_table[][3] =
{
    0xff, 0xff, 0xff,         /* white */

    0xff, 0xff, 0x00,         /* yellow */
    0x00, 0xff, 0xff,         /* cyan */
    0xff, 0x00, 0xff,         /* magenta */

    0xff, 0x00, 0x00,         /* red */
    0x00, 0xff, 0x00,         /* green */
    0x00, 0x00, 0xff,         /* blue */

    0x80, 0xff, 0x00,         /* yellowgreen */
    0x00, 0xff, 0x80,         /* bluegreen */
    0x00, 0x80, 0xff,         /* ltblue */
    0x80, 0x00, 0xff,         /* purple */
    0xff, 0x00, 0x80,         /* violet */
    0xff, 0x80, 0x00,         /* orange */

    0x40, 0xff, 0x00,         /* yellowgreen -> green */
    0x00, 0xff, 0x40,         /* green -> bluegreen */
    0x00, 0xff, 0xc0,         /* bluegreen -> cyan */
    0x00, 0xc0, 0xff,         /* cyan -> ltblue */
    0x00, 0x40, 0xff,         /* ltblue -> blue */
    0x40, 0x00, 0xff,         /* blue -> purple */
    0xc0, 0x00, 0xff,         /* purple -> magenta */
    0xff, 0x00, 0xc0,         /* magenta -> violet */
    0xff, 0x00, 0x40,         /* violet -> red */
    0xff, 0x40, 0x00,         /* red -> orange */
    0xff, 0xc0, 0x00,         /* orange -> yellow */
    0xc0, 0xff, 0x00,         /* yellow -> yellowgreen */

    /* index of next element is 25 */

    0x60, 0xff, 0x00,         /* start of extra colors for 48-color wheel */
    0x20, 0xff, 0x00,
    0x00, 0xff, 0x20,
    0x00, 0xff, 0x60,
    0x00, 0xff, 0xa0,
    0x00, 0xff, 0xe0,
    0x00, 0xe0, 0xff,
    0x00, 0xa0, 0xff,
    0x00, 0x60, 0xff,
    0x00, 0x20, 0xff,
    0x20, 0x00, 0xff,
    0x60, 0x00, 0xff,
    0xa0, 0x00, 0xff,
    0xe0, 0x00, 0xff,
    0xff, 0x00, 0xe0,
    0xff, 0x00, 0xa0,
    0xff, 0x00, 0x60,
    0xff, 0x00, 0x20,
    0xff, 0x20, 0x00,
    0xff, 0x60, 0x00,
    0xff, 0xa0, 0x00,
    0xff, 0xe0, 0x00,
    0xe0, 0xff, 0x00,
    0xa0, 0xff, 0x00,

    /* index of last element is 48 */
};

/* The remap tables. Since the colors are rendered in a different
 * order than they are allocated, a lookup table is used to
 * resolve the ordering (one lookup table for each of the 5 possible
 * layout types).
 *
 * We start with a constant declaration of the pen numbers. The first
 * entry, black, is a special case which is used by the monochrome
 * layout and no other.
 */

enum
{
    HUE_WHITE=0,
    HUE_YELLOW,
    HUE_CYAN,
    HUE_MAGENTA,
    HUE_RED,
    HUE_GREEN,
    HUE_BLUE,
    HUE_YELLOWGREEN,
    HUE_BLUEGREEN,
    HUE_LTBLUE,
    HUE_PURPLE,
    HUE_VIOLET,
    HUE_ORANGE,
    HUE_MID1
};

#define HUE_BLACK   1

#define HUE_MID2    (HUE_MID1+12)       /* for 48-color wheel */

/* And now, the actual tables */
UBYTE remap2[] = { HUE_WHITE, HUE_BLACK };
UBYTE remap5[] = { HUE_WHITE, HUE_CYAN, HUE_MAGENTA, HUE_YELLOW };
UBYTE remap8[] = { HUE_WHITE, HUE_GREEN, HUE_CYAN,
                   HUE_BLUE, HUE_MAGENTA, HUE_RED, HUE_YELLOW };
UBYTE remap14[] = { HUE_WHITE,
                    HUE_YELLOWGREEN, HUE_GREEN, HUE_BLUEGREEN, HUE_CYAN,
                    HUE_LTBLUE, HUE_BLUE, HUE_PURPLE, HUE_MAGENTA,
                    HUE_VIOLET, HUE_RED, HUE_ORANGE, HUE_YELLOW };
UBYTE remap26[] = { HUE_WHITE,
                    HUE_YELLOWGREEN, HUE_MID1, HUE_GREEN, HUE_MID1+1,
                    HUE_BLUEGREEN, HUE_MID1+2, HUE_CYAN, HUE_MID1+3,
                    HUE_LTBLUE, HUE_MID1+4, HUE_BLUE, HUE_MID1+5,
                    HUE_PURPLE, HUE_MID1+6, HUE_MAGENTA, HUE_MID1+7,
                    HUE_VIOLET, HUE_MID1+8, HUE_RED, HUE_MID1+9,
                    HUE_ORANGE, HUE_MID1+10, HUE_YELLOW, HUE_MID1+11 };
UBYTE remap50[] = { HUE_WHITE,
                    HUE_YELLOWGREEN, HUE_MID2, HUE_MID1, HUE_MID2+1,
                    HUE_GREEN, HUE_MID2+2, HUE_MID1+1, HUE_MID2+3,
                    HUE_BLUEGREEN, HUE_MID2+4, HUE_MID1+2, HUE_MID2+5,
                    HUE_CYAN, HUE_MID2+6, HUE_MID1+3, HUE_MID2+7,
                    HUE_LTBLUE, HUE_MID2+8, HUE_MID1+4, HUE_MID2+9,
                    HUE_BLUE, HUE_MID2+10, HUE_MID1+5, HUE_MID2+11,
                    HUE_PURPLE, HUE_MID2+12, HUE_MID1+6, HUE_MID2+13,
                    HUE_MAGENTA, HUE_MID2+14, HUE_MID1+7, HUE_MID2+15,
                    HUE_VIOLET, HUE_MID2+16, HUE_MID1+8, HUE_MID2+17,
                    HUE_RED, HUE_MID2+18, HUE_MID1+9, HUE_MID2+19,
                    HUE_ORANGE, HUE_MID2+20, HUE_MID1+10, HUE_MID2+21,
                    HUE_YELLOW, HUE_MID2+22, HUE_MID1+11, HUE_MID2+23 };

/* Remapping for 49-sector/2-hue is the same as remap50, with the extra
 * 48 colors above index 48 being remapped as (remap50[c-48] + 48).
 */

/*****************************************************************************/


/* These definitions are used in creating the TmpRas which are
 * needed by the AreaInfo to draw the pie segments.
 */

#define AREA_SIZE       50
#define AREA_X_FUDGE    16              /* make one word bigger */
#define AREA_Y_FUDGE    0               /* not sure needs to be bigger */


/*****************************************************************************/


/* Two types of division, one truncates and one rounds. */

#define TruncDiv(a,b)   ((a)/(b))

LONG RoundDiv(LONG dividend, LONG divisor)
{
LONG  adjust = divisor / 2;

    if (dividend < 0)
        adjust = -adjust;

    return (dividend + adjust) / divisor;
}


/*****************************************************************************/


/* This draws one piece of the "pie" */
VOID ArcPart(struct RastPort *rp, LONG x, LONG y, LONG a, LONG b, LONG theSin)
{
LONG theCos;

    if (theSin < 0)
        theSin += SIN_TABLE_SIZE;   /* sin wraps around */

    if (theSin >= SIN_TABLE_SIZE)
        theSin -= SIN_TABLE_SIZE;

    theCos = theSin + SIN_TABLE_SIZE / 4;

    if (theCos >= SIN_TABLE_SIZE)   /* so does cos */
        theCos -= SIN_TABLE_SIZE;

    AreaDraw(rp, x + TruncDiv(a * (LONG)sin_table[theCos],SIN_ONE),
                 y + TruncDiv(b * (LONG)sin_table[theSin],SIN_ONE));
}


/*****************************************************************************/


/* This draws one segment of the border around the wheel */
VOID SegmentSideDraw(struct RastPort *rp, LONG x, LONG y,
                     LONG a1, LONG b1, LONG a2, LONG b2, LONG theSin)
{
LONG theCos;

    if (theSin < 0)
        theSin += SIN_TABLE_SIZE;   /* sin wraps around */

    if (theSin >= SIN_TABLE_SIZE)
        theSin -= SIN_TABLE_SIZE; /* so does cos */

    theCos = theSin + SIN_TABLE_SIZE / 4;

    if (theCos >= SIN_TABLE_SIZE)
        theCos -= SIN_TABLE_SIZE;

    Move(rp, x + TruncDiv(a1 * (LONG)sin_table[theCos],SIN_ONE),
             y + TruncDiv(b1 * (LONG)sin_table[theSin],SIN_ONE));

    Draw(rp, x + TruncDiv(a2 * (LONG)sin_table[theCos],SIN_ONE),
             y + TruncDiv(b2 * (LONG)sin_table[theSin],SIN_ONE));
}


/*****************************************************************************/


/* Dot Drawing Code
 *
 * Converts a Hue / Saturation value to an X-Y position for the
 * drag dot.
 */
VOID CalcNewXYFromHS(struct Wheel *wheel)
{
LONG i, j, sin, t, hue;

        /* Rotate by 90 degress to adjust for fact that origin is actually
            at green, internally.
        */

    hue = wheel->Current.cw_Hue - MAX_WHEEL_COMP/4 & 0x0000ffff;

        /* Map hue into range of sin table * 64 */

    i = TruncDiv(hue * SIN_TABLE_SIZE * 64,MAX_WHEEL_COMP + 1);
    j = i / 64;         /* j = sine table entry to interpolate from */

        /* lookup sine and perform interpolation for Y */

    sin = sin_table[j];
    sin += TruncDiv((sin_table[j+1] - sin) * (i & 63),64);

    t = RoundDiv(wheel->YRadius * wheel->Current.cw_Saturation,MAX_WHEEL_COMP);
    wheel->NewDotYCenter = RoundDiv(t * sin,SIN_ONE) + wheel->YCenter;

        /* lookup cosine and perform interpolation for X */

    j += SIN_TABLE_SIZE / 4;
    if (j >= SIN_TABLE_SIZE) j -= SIN_TABLE_SIZE;

    sin = sin_table[j];
    sin += TruncDiv((sin_table[j+1] - sin) * (i & 63),64);

    t = RoundDiv(wheel->XRadius * wheel->Current.cw_Saturation,MAX_WHEEL_COMP);
    wheel->NewDotXCenter = RoundDiv(t * sin,SIN_ONE) + wheel->XCenter;
}


/*****************************************************************************/


/* Draw the drag dot in the wheel, but backsave the stuff underneath
 * first.
 */
VOID DrawWheelDot(struct RastPort *rp, struct Wheel *wheel, struct Screen *scr)
{
struct RastPort saverp;
struct BitMap   ringbm;
UWORD           shine, shadow;
struct AreaInfo ainfo;
WORD            areabuf[AREA_SIZE];
LONG		modulo;

    if (!InitColorWheelStruct(wheel,scr))
        return;

    InitArea(&ainfo, areabuf, (AREA_SIZE*2)/5);

    rp->AreaInfo = &ainfo;
    rp->TmpRas = &wheel->TmpRas;

    InitRastPort(&saverp);

    if (wheel->OuterRing == NULL)
    {
	CalcNewXYFromHS(wheel);
	wheel->DotXCenter = wheel->NewDotXCenter;
	wheel->DotYCenter = wheel->NewDotYCenter;

	wheel->RingXSize = 2 * wheel->DotXRadius + 1 + AREA_X_FUDGE;
	wheel->RingYSize = 2 * wheel->DotYRadius + 1 + AREA_Y_FUDGE;

	if ((wheel->OuterRing = AllocRaster(wheel->RingXSize,wheel->RingYSize))
	    == NULL ) return;

	if ( (wheel->InnerRing = AllocRaster(wheel->RingXSize,wheel->RingYSize))
	    == NULL )
	{
	    FreeRaster(wheel->OuterRing,wheel->RingXSize,wheel->RingYSize);
	    wheel->OuterRing = NULL;
	    return;
	}

	InitBitMap(&ringbm,1,wheel->RingXSize,wheel->RingYSize);

	saverp.BitMap = &ringbm;
        saverp.AreaInfo = &ainfo;
        saverp.TmpRas = &wheel->TmpRas;

	SetABPenDrMd(&saverp,1,0,JAM1);   /* bpen is not important */

	ringbm.Planes[0] = wheel->OuterRing;
	SetRast(&saverp,0);
	AreaEllipse(&saverp,wheel->DotXRadius,wheel->DotYRadius,
            wheel->DotXRadius,wheel->DotYRadius);
        AreaEnd(&saverp);

	ringbm.Planes[0] = wheel->InnerRing;
	SetRast(&saverp,0);
        AreaEllipse(&saverp,wheel->DotXRadius,wheel->DotYRadius,
            wheel->DotXRadius - 2,wheel->DotYRadius - 2);
        AreaEnd(&saverp);
    }

    saverp.BitMap = wheel->DBitMap;

    /* When dragging the dot, we need to erase it by restoring the
     *  original background.
     */

    if (wheel->Flags & WHEELF_DOT)
    {
        CalcNewXYFromHS(wheel);

        /* If dot never moved, then there's nothing to do */

        if ((wheel->DotXCenter == wheel->NewDotXCenter) && (wheel->DotYCenter == wheel->NewDotYCenter))
            return;

        ClipBlit(&saverp,0,0,rp,
            wheel->DotXCenter-wheel->DotXRadius,
            wheel->DotYCenter-wheel->DotYRadius,
            2 * wheel->DotXRadius + 1,
            2 * wheel->DotYRadius + 1,0xc0);

        wheel->DotXCenter = wheel->NewDotXCenter;
        wheel->DotYCenter = wheel->NewDotYCenter;
    }

    /* Now, backsave the background of the NEW location of the dot */

    ClipBlit(rp,wheel->DotXCenter-wheel->DotXRadius,
        wheel->DotYCenter-wheel->DotYRadius,
        &saverp,0,0,
        2 * wheel->DotXRadius + 1,
        2 * wheel->DotYRadius + 1,0xc0);

    shine = wheel->DInfo->dri_Pens[SHINEPEN];
    shadow = wheel->DInfo->dri_Pens[SHADOWPEN];

    /* Now blast the two rings that compose the dot to the display */

    modulo = wheel->RingXSize + 15 >> 3 & ~1;

    SetABPenDrMd(rp,shadow,0,JAM1);   /* bpen is not important */
    BltTemplate(wheel->OuterRing,0,modulo,rp,
	wheel->DotXCenter - wheel->DotXRadius,
	wheel->DotYCenter - wheel->DotYRadius,
	2 * wheel->DotXRadius + 1,
	2 * wheel->DotYRadius + 1);

    /* need to check special case where [SHINEPEN] == [SHADOWPEN] */
    SetAPen(rp,shadow != shine ? shine : 0);

    BltTemplate(wheel->InnerRing,0,modulo,rp,
	wheel->DotXCenter - wheel->DotXRadius,
	wheel->DotYCenter - wheel->DotYRadius,
	2 * wheel->DotXRadius + 1,
	2 * wheel->DotYRadius + 1);

    wheel->Flags |= WHEELF_DOT;
}


/*****************************************************************************/


VOID DrawWheel98(struct RastPort *rp, struct Wheel *wheel)
{
LONG   wheel_x_center = wheel->XCenter;
LONG   wheel_y_center = wheel->YCenter;
LONG   wheel_x_rad = wheel->XRadius;
LONG   wheel_y_rad = wheel->YRadius;
UWORD *pens = wheel->SPens;
WORD   i, k;

    SetDrMd(rp,JAM1);

    /* step 1: draw outer circle */

    for (i=0;i<48;i++)
    {
        SetAPen(rp,pens[i + 1]);
        AreaMove(rp,wheel_x_center,wheel_y_center);

        for (k=-1;k<=1;k++)
        {
            ArcPart(rp,wheel_x_center,wheel_y_center,wheel_x_rad,wheel_y_rad,
                2 * i + k);
        }

        AreaEnd(rp);
    }

    /* step 1: draw first dither circle */

    SetAfPt(rp,med_pat,1);

    for (i=0;i<48;i++)
    {
        SetAPen(rp,pens[i + 1 + 48]);
        AreaMove(rp,wheel_x_center,wheel_y_center);

        for (k=-1;k<=1;k++)
        {
            ArcPart(rp,wheel_x_center,wheel_y_center,
		wheel_x_rad * 7 / 9,wheel_y_rad * 7 / 9,
                2 * i + k);
        }

        AreaEnd(rp);
    }

    /* step 2: draw middle solid circle */

    SetAfPt(rp,NULL,0);

    for (i=0;i<48;i++)
    {
        SetAPen(rp,pens[i + 1 + 48]);
        AreaMove(rp,wheel_x_center,wheel_y_center);

        for (k=-1;k<=1;k++)
        {
            ArcPart(rp,wheel_x_center,wheel_y_center,
		wheel_x_rad * 5 / 9,wheel_y_rad * 5 / 9,
                2 * i + k);
        }

        AreaEnd(rp);
    }

    /* step 3: draw inner white dither circle */

    SetAPen(rp,pens[0]);
    SetAfPt(rp,med_pat,1);
    AreaEllipse(rp,wheel_x_center,wheel_y_center,
        wheel_x_rad * 3 / 9,wheel_y_rad * 3 / 9);
    AreaEnd(rp);

    /* step 4: draw white circle */

    SetAfPt(rp,NULL,0);
    AreaEllipse(rp,wheel_x_center,wheel_y_center,
        wheel_x_rad / 9,wheel_y_rad / 9);
    AreaEnd(rp);
}


/*****************************************************************************/


/* Tables used to draw the 12, 24 & 48 sector wheels with one function. */

UBYTE   combined[3][3] =
{
    { 48, 1, 2 },               /* COMBINED_50 (48 sectors) */
    { 24, 2, 4 },               /* COMBINED_26 (24 sectors) */
    { 12, 4, 8 }                /* COMBINED_14 (12 sectors) */
};

enum
{
    COMBINED_50=0,
    COMBINED_26,
    COMBINED_14
};

VOID DrawWheelCombined(struct RastPort *rp, struct Wheel *wheel, UBYTE *table)
{
LONG   wheel_x_center = wheel->XCenter;
LONG   wheel_y_center = wheel->YCenter;
LONG   wheel_x_rad = wheel->XRadius;
LONG   wheel_y_rad = wheel->YRadius;
UWORD *pens = wheel->SPens;
WORD   i, k;

    SetDrMd(rp,JAM1);

    /* step 1: draw outer circle */

    for (i=0;i<table[0];i++)
    {
        SetAPen(rp,pens[i + 1]);
        AreaMove(rp,wheel_x_center,wheel_y_center);

        for (k=-table[1];k<=table[1];k++)
        {
            ArcPart(rp,wheel_x_center,wheel_y_center,wheel_x_rad,wheel_y_rad,
                table[2] * i + k);
        }

        AreaEnd(rp);
    }

    /* step 1: draw light white dither circle */

    SetAPen(rp,pens[0]);

    SetAfPt(rp,lite_pat,1);
    AreaEllipse(rp,wheel_x_center,wheel_y_center,
        wheel_x_rad * 7 / 9,wheel_y_rad * 7 / 9);
    AreaEllipse(rp,wheel_x_center,wheel_y_center,
        wheel_x_rad * 5 / 9,wheel_y_rad * 5 / 9);
    AreaEnd(rp);

    /* step 2: draw medium white dither circle */

    SetAfPt(rp,med_pat,1);
    AreaEllipse(rp,wheel_x_center,wheel_y_center,
        wheel_x_rad * 5 / 9,wheel_y_rad * 5 / 9);
    AreaEllipse(rp,wheel_x_center,wheel_y_center,
        wheel_x_rad * 3 / 9,wheel_y_rad * 3 / 9);
    AreaEnd(rp);

    /* step 3: draw heavy white dither circle */

    SetAfPt(rp,dark_pat,1);
    AreaEllipse(rp,wheel_x_center,wheel_y_center,
        wheel_x_rad * 3 / 9,wheel_y_rad * 3 / 9);
    AreaEnd(rp);

    /* step 4: draw white circle */

    SetAfPt(rp,NULL,0);
    AreaEllipse(rp,wheel_x_center,wheel_y_center,
        wheel_x_rad / 9,wheel_y_rad / 9);
    AreaEnd(rp);
}


/*****************************************************************************/


/* This draws the 48 sector version of the wheel. (50 colors because it
 * includes white and the black border.)
 */
VOID DrawWheel50(struct RastPort *rp, struct Wheel *wheel)
{
     DrawWheelCombined(rp,wheel,&combined[COMBINED_50][0]);
}


/*****************************************************************************/


/* This draws the 24 sector version of the wheel. (26 colors because it
 * includes white and the black border.)
 */
VOID DrawWheel26(struct RastPort *rp, struct Wheel *wheel)
{
     DrawWheelCombined(rp,wheel,&combined[COMBINED_26][0]);
}


/*****************************************************************************/


/* This draws the 12 sector version of the wheel. (14 colors because it
 * includes white and the black border.)
 */
VOID DrawWheel14(struct RastPort *rp, struct Wheel *wheel)
{
     DrawWheelCombined(rp,wheel,&combined[COMBINED_14][0]);
}


/*****************************************************************************/


/* This draws the 6 color version of the wheel. (8 because it
 * includes white and the black border.)
 */
VOID DrawWheel8(struct RastPort *rp, struct Wheel *wheel)
{
LONG   wheel_x_center = wheel->XCenter;
LONG   wheel_y_center = wheel->YCenter;
LONG   wheel_x_rad = wheel->XRadius;
LONG   wheel_y_rad = wheel->YRadius;
UWORD *pens = wheel->SPens;
WORD   i, j, k;
LONG   a, b, m, n;

    /* step 1: draw outer ring */

    for (i=0;i<12;i++)
    {
        j = i / 2;
        m = pens[j + 1];

        if (i & 1)
        {
            SetABPenDrMd(rp,m,0,JAM1);  /* bpen is not important */
            SetAfPt(rp,NULL,0);
        }
        else
        {
            SetAfPt(rp,med_pat,1);
            n = pens[(j+5)%6 + 1];
            SetABPenDrMd(rp,j & 1 ? n : m,
                            j & 1 ? m : n,
                            JAM2);
        }

        AreaMove(rp,wheel_x_center,wheel_y_center);

        for (k=-4;k<=4;k++)
        {
            ArcPart(rp,wheel_x_center,wheel_y_center,wheel_x_rad,wheel_y_rad,
                8 * i + k);
        }

        AreaEnd(rp);
    }

    /* step 2: draw inner ring */

    SetAfPt(rp,med_pat,1);
    a = wheel_x_rad * 5 / 8;
    b = wheel_y_rad * 5 / 8;

    for (i=0;i<6;i++)
    {
        SetABPenDrMd(rp,pens[i + 1],pens[0],JAM2);

        AreaMove(rp,wheel_x_center,wheel_y_center);

        for (k=0;k<=16;k++)
        {
            ArcPart(rp,wheel_x_center,wheel_y_center,a,b,
                16 * i + k);
        }

        AreaEnd(rp);
    }

    /* step 3: draw white circle */

    SetAfPt(rp,NULL,0);
    SetABPenDrMd(rp,pens[0],0,JAM1);      /* bpen is not important */
    AreaEllipse(rp,wheel_x_center,wheel_y_center, wheel_x_rad / 4,wheel_y_rad / 4);
    AreaEnd(rp);
}


/*****************************************************************************/


/* This draws the 3 color version of the wheel. (5 because it
 * includes white and the black border.)
 */
VOID DrawWheel5(struct RastPort *rp, struct Wheel *wheel)
{
LONG   wheel_x_center = wheel->XCenter;
LONG   wheel_y_center = wheel->YCenter;
LONG   wheel_x_rad = wheel->XRadius;
LONG   wheel_y_rad = wheel->YRadius;
UWORD *pens = wheel->SPens;
WORD   i, j, k;

    /* step 1: draw outer ring */

    for (i=0;i<6;i++)
    {
        j = i / 2;

        if (i & 1)
        {
            SetABPenDrMd(rp,pens[j + 1],0,JAM1);  /* bpen is not important */
            SetAfPt(rp,NULL,0);
        }
        else
        {
            SetABPenDrMd(rp,pens[j + 1],pens[(j+2)%3 + 1],JAM2);
            SetAfPt(rp,med_pat,1);
        }

        AreaMove(rp,wheel_x_center,wheel_y_center);

        for (k=0;k<=16;k++)
        {
            ArcPart(rp,wheel_x_center,wheel_y_center,wheel_x_rad,wheel_y_rad,
                16 * i + k);
        }

        AreaEnd(rp);
    }

    /* step 2: draw white circle */

    SetAfPt(rp,NULL,0);
    SetABPenDrMd(rp,pens[0],0,JAM1);  /* bpen is not important */
    AreaEllipse(rp,wheel_x_center,wheel_y_center, wheel_x_rad / 3,wheel_y_rad / 3);
    AreaEnd(rp);
}


/*****************************************************************************/


/* This draws the monochrome version of the wheel. */
VOID DrawWheel2(struct RastPort *rp, struct Wheel *wheel)
{
LONG              wheel_x_center = wheel->XCenter;
LONG              wheel_y_center = wheel->YCenter;
LONG              wheel_x_rad = wheel->XRadius;
LONG              wheel_y_rad = wheel->YRadius;
UWORD             shine, shadow;
UWORD            *pens = wheel->SPens;
char             *abbrv = wheel->HueAbbrv;
WORD              i, s, c;
LONG              m, n;
LONG              a = wheel_x_rad - 2 * wheel->BorWidth;
LONG              b = wheel_y_rad - 2 * wheel->BorHeight;
char              str[2];
struct IntuiText  hueText;

    shine = wheel->DInfo->dri_Pens[SHINEPEN];
    shadow = wheel->DInfo->dri_Pens[SHADOWPEN];

    /* step 0: draw background color */

    SetAPen(rp,0);
    AreaEllipse(rp,wheel_x_center,wheel_y_center,
        wheel_x_rad - wheel->BorWidth + 1,wheel_y_rad - wheel->BorHeight + 1);
    AreaEllipse(rp,wheel_x_center,wheel_y_center,
        wheel_x_rad - 2 * wheel->BorWidth,
        wheel_y_rad - 2 * wheel->BorHeight);
    AreaEnd(rp);

    /* step 1: draw outer ring */

    SetABPenDrMd(rp,pens[1],0,JAM1);  /* bpen is not important */
    AreaEllipse(rp,wheel_x_center,wheel_y_center,a,b);
    AreaEnd(rp);

    /* step 2: draw "gray" ring */

    SetABPenDrMd(rp,pens[0],pens[1],JAM2);
    SetAfPt(rp,med_pat,1);
    AreaEllipse(rp,wheel_x_center,wheel_y_center,a * 3 / 5,b * 3 / 5);
    AreaEnd(rp);

    /* step 3: draw white circle */

    SetDrMd(rp,JAM1);
    SetAfPt(rp,NULL,0);
    AreaEllipse(rp,wheel_x_center,wheel_y_center,a / 5,b / 5);
    AreaEnd(rp);

    /* step 4: draw white lines */

    for (i=0;i<6;i++)
    {
        SegmentSideDraw(rp,wheel_x_center,wheel_y_center,
            a * 3 / 5,b * 3 / 5,a,b,16 * i);
    }

    /* step 5: draw legend */

    hueText.LeftEdge = hueText.TopEdge = 0;
    hueText.FrontPen = (shadow != shine ? shine : 0);
    hueText.DrawMode = JAM1;
    hueText.ITextFont = &topaz8_ta;
    hueText.IText = str;
    hueText.NextText = NULL;
    str[1] = 0;

    for (i=0;i<6;i++)
    {
        s = (SIN_TABLE_SIZE / 6) * i + (SIN_TABLE_SIZE / 12);
        c = s + (SIN_TABLE_SIZE / 4);
        if (c >= SIN_TABLE_SIZE) c -= SIN_TABLE_SIZE;

        m = wheel_x_center +
            RoundDiv((a * 4 / 5 + 1) * (LONG)sin_table[c],SIN_ONE) - 3;
        n = wheel_y_center +
            RoundDiv((b * 4 / 5 + 1) * (LONG)sin_table[s],SIN_ONE) - 3;

        str[0] = abbrv[i];
        if (i == 2 || i == 3) m--;
        PrintIText(rp,&hueText,m,n);
    }
}


/*****************************************************************************/


/* This draws the entire color wheel gadget. It first draws the wheel,
 * then it draws the dot and finnaly it draws a bevel box (if enabled).
 */

VOID DrawWheel(struct RastPort *rp, struct Gadget *g, struct Screen *scr,
               struct DrawInfo *dinfo)
{
struct Wheel    *wheel = (struct Wheel *)g->SpecialInfo;
struct AreaInfo  ainfo;
WORD             areabuf[AREA_SIZE];

    InitArea(&ainfo, areabuf, (AREA_SIZE*2)/5);

    rp->AreaInfo = &ainfo;
    rp->TmpRas = &wheel->TmpRas;

    (wheel->DrawFunc)(rp,wheel);

    /* draw the border around the wheel */
    SetAPen(rp,wheel->DInfo->dri_Pens[SHADOWPEN]);
    AreaEllipse(rp,wheel->XCenter,wheel->YCenter, wheel->XRadius, wheel->YRadius);
    AreaEllipse(rp,wheel->XCenter,wheel->YCenter, wheel->XRadius - wheel->BorWidth + 1,wheel->YRadius - wheel->BorHeight + 1);
    AreaEnd(rp);

    if (wheel->Flags & WHEELF_BEVELBOX)
    {
        QuickBevel(rp,(struct IBox *)&g->LeftEdge,
                   dinfo->dri_Pens[SHINEPEN],
                   dinfo->dri_Pens[SHADOWPEN]);
    }

    wheel->Flags &= ~WHEELF_DOT;

    if (g->Flags & GFLG_DISABLED)
    {
        /* draw ghosting pattern over wheel if disabled */

        SetAPen(rp,wheel->DInfo->dri_Pens[SHADOWPEN]);
        SetAfPt(rp,med_pat,1);

        if (wheel->Flags & WHEELF_BEVELBOX)
        {
            RectFill(rp,g->LeftEdge,g->TopEdge,
                g->LeftEdge + g->Width - 1,g->TopEdge + g->Height - 1);
        }
        else
        {
            AreaEllipse(rp,wheel->XCenter,wheel->YCenter, wheel->XRadius, wheel->YRadius);
            AreaEllipse(rp,wheel->XCenter,wheel->YCenter, wheel->XRadius - wheel->BorWidth + 1,wheel->YRadius - wheel->BorHeight + 1);
            AreaEnd(rp);
        }

        SetAfPt(rp,NULL,0);
    }
    else
    {
        /* if not disabled, draw dot */

        DrawWheelDot(rp,wheel,scr);
    }
}


/*****************************************************************************/


VOID TrackWheel(struct Gadget *gad, struct gpInput *msg)
{
struct Wheel    *wheel = (struct Wheel *)gad->SpecialInfo;
LONG             xoff, yoff;
LONG             axoff, ayoff;
LONG             x2, y2;
LONG             d;
LONG             r2, d2;
LONG             sat, hue;
LONG             sin, sin2;
LONG             i, t;
BOOL             didcos = FALSE;

    /*  (xoff,yoff) is the distance from (DXCenter,DYCenter), with
     *  y distance aspect scaled based on pixel resolution [from
     *  (XRadius,YRadius)]
     */

    xoff = msg->gpi_Mouse.X - wheel->DXCenter;
    yoff = RoundDiv((msg->gpi_Mouse.Y - wheel->DYCenter) * wheel->XRadius,
        wheel->YRadius);

    x2 = xoff * xoff;               /* square of x distance from center */
    y2 = yoff * yoff;               /* square of y distance from center */
    d2 = x2 + y2;                   /* square of the hypoteneuse        */
    r2 = wheel->XRadius * wheel->XRadius;   /* square of wheel radius   */

    d = RndSqRoot(d2);              /* Pythagorean distance             */

    /* calculate saturation (0-MAX_WHEEL_COMP) */
    sat = (d2 >= r2 ? MAX_WHEEL_COMP : RoundDiv(d * MAX_WHEEL_COMP,wheel->XRadius));
    if (sat > MAX_WHEEL_COMP) sat = MAX_WHEEL_COMP;

    /* calculate hue (0-MAX_WHEEL_COMP) */
    if (xoff == 0)
    {
        /* special case of 90 degrees */

        if (yoff >= 0)
            hue = MAX_WHEEL_RANGE/4;
        else
            hue = 3 * MAX_WHEEL_RANGE/4;
    }
    else
    {
        /* do an arcsin approximation using the sine table */
        axoff = ABS(xoff);
        ayoff = ABS(yoff);
        if (ayoff > axoff)
        {
            /* in this octant, do as cosine since it's more accurate */
            sin = TruncDiv(ABS(xoff) * SIN_ONE,d);
            didcos = TRUE;
        }
        else
        {
            sin = TruncDiv(ABS(yoff) * SIN_ONE,d);     /* do as sine */
        }

        /* search for closest match in sine table */

        for (i = 1; i <= SIN_TABLE_SIZE/4; i++)
            if (sin < sin_table[i])
                break;

        /* do interpolation between entries */

        sin2 = sin_table[--i];
        t = TruncDiv((sin - sin2) * (MAX_WHEEL_RANGE/4),sin_table[i+1] - sin2);

        hue = TruncDiv((MAX_WHEEL_RANGE/4) * i + t,SIN_TABLE_SIZE/4);
        if (didcos) hue = MAX_WHEEL_RANGE/4 - hue;

        /* deal with other quadrants */

        if (xoff < 0)
            hue = MAX_WHEEL_RANGE/2 - hue;

        if (yoff < 0)
            hue = MAX_WHEEL_RANGE - hue;
    }

    /* rotate 90 degrees to adjust for origin */
    hue += MAX_WHEEL_RANGE/4;

    /* store results */
    wheel->Current.cw_Hue = hue & 0x0000ffff;
    wheel->Current.cw_Saturation = sat;

    /* draw the dot! (in a BOOPSI friendly way...) */
    WheelUpdate(msg->gpi_GInfo,gad);
}


/*****************************************************************************/


static LONG WheelHandle(struct gpInput *msg, struct Gadget *g)
{
LONG           result = GMR_MEACTIVE;
struct Wheel  *wheel  = (struct Wheel *)g->SpecialInfo;
struct TagItem output_tags[4];
ULONG          oldHue, oldSat;

    if (msg->gpi_IEvent->ie_Class == IECLASS_RAWMOUSE)
    {
        oldHue = wheel->Current.cw_Hue;
        oldSat = wheel->Current.cw_Saturation;

        /* its a mouse event */
        if (msg->gpi_IEvent->ie_Code == (IECODE_LBUTTON | IECODE_UP_PREFIX))
        {
            /* left button up, deactivate gadget */
            result = (GMR_NOREUSE | GMR_VERIFY);
        }
        else if (msg->gpi_IEvent->ie_Code == IECODE_RBUTTON)
        {
            /* check for menu button hit... if so, cancel action */
            wheel->Current = wheel->Saved;

            WheelUpdate(msg->gpi_GInfo,g);
            result = (GMR_NOREUSE | GMR_VERIFY);
        }
        else
        {
            /* otherwise, track user's dragging of dot */
            TrackWheel(g,msg);
        }

        if ((result != GMR_MEACTIVE) || (wheel->Current.cw_Hue != oldHue) || (wheel->Current.cw_Saturation != oldSat))
        {
            /* notify user of new Hue and Saturation levels */

            output_tags[0].ti_Tag  = WHEEL_Hue;
            output_tags[0].ti_Data = ShiftColor32(wheel->Current.cw_Hue);

            output_tags[1].ti_Tag  = WHEEL_Saturation;
            output_tags[1].ti_Data = ShiftColor32(wheel->Current.cw_Saturation);

            output_tags[2].ti_Tag  = GA_ID;
            output_tags[2].ti_Data = g->GadgetID;

            output_tags[3].ti_Tag = TAG_DONE;

            /* send notification */
            DoMethod((Object *)g, OM_NOTIFY, output_tags, msg->gpi_GInfo,
                     (result != GMR_MEACTIVE ? 0 : OPUF_INTERIM));
        }
    }

    return result;
}


/*****************************************************************************/


static LONG WheelHitTest(struct gpHitTest *msg, struct Gadget *g)
{
struct Wheel *wheel = (struct Wheel *)g->SpecialInfo;
LONG          xoff, yoff;
LONG          x2, y2;
LONG          xf, yf;

    if ( !(wheel->Flags & WHEELF_BEVELBOX) )
    {
        /*  (xoff,yoff) is the distance from (DXCenter,DYCenter), with
        *  y distance aspect scaled based on pixel resolution [from
        *  (XRadius,YRadius) adjusted by dot size]
        */

        xf = wheel->XRadius + wheel->DotXRadius - 1;
        yf = wheel->YRadius + wheel->DotYRadius - 1;
        xoff = msg->gpht_Mouse.X - wheel->DXCenter;
        yoff = RoundDiv((msg->gpht_Mouse.Y - wheel->DYCenter) * xf,yf);

        x2 = xoff * xoff;               /* square of x distance from center */
        y2 = yoff * yoff;               /* square of y distance from center */

        /* if square of distance to click > square of wheel radius,
        * then click "outside" of gadget.
        */

        if (x2 + y2 > xf * xf)
            return (0);
    }

    return(GMR_GADGETHIT);
}

/*****************************************************************************/


static VOID WheelRender(struct gpRender *msg, struct Gadget *g)
{
    if (msg->gpr_GInfo != NULL &&
        (msg->gpr_Redraw == GREDRAW_REDRAW || msg->gpr_Redraw == GREDRAW_UPDATE))
    {
    struct Wheel *wheel = (struct Wheel *)g->SpecialInfo;
    PLANEPTR      raster;

        if ((raster = AllocRaster(wheel->RasXSize,wheel->RasYSize)) == NULL )
            return;

        InitTmpRas(&wheel->TmpRas,raster,RASSIZE(wheel->RasXSize,wheel->RasYSize));

        if (msg->gpr_Redraw == GREDRAW_REDRAW)
            DrawWheel(msg->gpr_RPort,g,msg->gpr_GInfo->gi_Screen,msg->gpr_GInfo->gi_DrInfo);

        if (msg->gpr_Redraw == GREDRAW_UPDATE)
            DrawWheelDot(msg->gpr_RPort,(struct Wheel *)g->SpecialInfo,msg->gpr_GInfo->gi_Screen);

        FreeRaster(raster,wheel->RasXSize,wheel->RasYSize);
    }
}


/*****************************************************************************/


static VOID WheelUpdate(struct GadgetInfo *gi, struct Gadget *g)
{
struct RastPort *rp;

    if (rp = ObtainGIRPort(gi))
    {
        DoMethod( (Object *)g, GM_RENDER, gi, rp, GREDRAW_UPDATE);
        ReleaseGIRPort(rp);
    }
}


/*****************************************************************************/


/* ============================================================================ *
 *  initialize Color Wheel environment based on Screen it's on
 * ============================================================================ */

static ULONG ExpandHueTableEntry(UBYTE val)
{
    if (val == (UBYTE)0xff) return (0xffffffff);
    return ((ULONG)val << 24);
}


/*****************************************************************************/


static void ExpandRGBEntry(ULONG *rgb, UWORD i)
{
UWORD e = i;

    if (i >= 49) e -= 48;

    rgb[0] = ExpandHueTableEntry(hue_table[e][0]);
    rgb[1] = ExpandHueTableEntry(hue_table[e][1]);
    rgb[2] = ExpandHueTableEntry(hue_table[e][2]);

    if (i >= 49)
    {
    	rgb[0] = 0x80000000 + rgb[0] / 2;
        rgb[1] = 0x80000000 + rgb[1] / 2;
        rgb[2] = 0x80000000 + rgb[2] / 2;
    }
}


/*****************************************************************************/


static void FreeUnsedPens(struct Screen *scr, struct Wheel *wheel, UWORD needed)
{
    /* free unused pens */
    while (wheel->Entries > MAX(needed,wheel->FirstObtained))
    {
        ReleasePen(scr->ViewPort.ColorMap,wheel->CPens[--wheel->Entries]);
    }
}


/*****************************************************************************/


BOOL InitColorWheelStruct(struct Wheel *wheel, struct Screen *scr)
{
static struct TagItem obtain_tags[] =
{
    OBP_FailIfBad,  TRUE,
    OBP_Precision,  PRECISION_IMAGE,
    TAG_END
};
struct RastPort *rp = &scr->RastPort;
struct DrawInfo *dinfo;
LONG             xdot, ydot, mag, pen;
UWORD           *pens;
LONG             depth;
UWORD            i, j, max;
ULONG            rgb[3];
UBYTE           *remap;

    /* if no backsave bitmap has been allocated, try to do so... */
    if (!wheel->DBitMap)
    {
        wheel->Screen = scr;

        /* default dot and border sizes */
        wheel->BorWidth = wheel->BorHeight = WHEEL_BORDER_WIDTH;
        wheel->DotXRadius = wheel->DotYRadius = WHEEL_DOT_RADIUS;

        if (!(dinfo = wheel->DInfo = GetScreenDrawInfo(scr)))
            return(FALSE);

        if (dinfo->dri_Resolution.X < dinfo->dri_Resolution.Y)
        {
            mag = RoundDiv(dinfo->dri_Resolution.Y,dinfo->dri_Resolution.X);

            wheel->BorWidth *= mag;
            wheel->DotXRadius += (mag - 1) * (WHEEL_DOT_RADIUS / 2);
        }
        else
        {
            mag = RoundDiv(dinfo->dri_Resolution.X,dinfo->dri_Resolution.Y);

            wheel->BorHeight *= mag;
            wheel->DotYRadius += (mag - 1) * (WHEEL_DOT_RADIUS / 2);
        }

        /* calculate dimensions of backsave bitmap */
        xdot = 2 * wheel->DotXRadius + 1;
        ydot = 2 * wheel->DotYRadius + 1;
        depth = GetBitMapAttr(rp->BitMap,BMA_DEPTH);

        /* adjust wheel radius based on dot size */
        if (wheel->Flags & WHEELF_BEVELBOX)
        {
            wheel->XRadius -= wheel->DotXRadius + 4;
            wheel->YRadius -= wheel->DotYRadius + 2;
        }

        /* first try allocating a friend bitmap */
        wheel->DBitMap = AllocBitMap(xdot,ydot,depth,0,rp->BitMap);

        /* if that failed, return failure... */
        if (!wheel->DBitMap)
        {
            FreeScreenDrawInfo(scr,dinfo);
            wheel->DInfo = NULL;
            return(FALSE);
        }

        /* now decide what type of wheel to draw and how to color it    */

        /* NOTE: ignore fixed color devices (they don't exist in V39) */

        wheel->FirstObtained = wheel->Entries;

        while ((wheel->Entries < WHEEL_MAX_NEED)
            && (wheel->Entries - wheel->FirstObtained < wheel->MaxPens))
        {
	    ExpandRGBEntry(rgb,wheel->Entries);

            pen = ObtainBestPenA(scr->ViewPort.ColorMap, rgb[0], rgb[1], rgb[2],
                                 obtain_tags);

            if (pen == -1)
                break;

#if 1
            /* if two colors are the same, consider that a failure */

            for (i = wheel->FirstObtained; i < wheel->Entries; i++)
            {
                if (wheel->CPens[i] == pen)
                {
                    ReleasePen(scr->ViewPort.ColorMap,pen);
                    goto endloop;
                }
            }
#endif

            wheel->CPens[wheel->Entries++] = pen;
        }

endloop:
        if (wheel->Entries == WHEEL_98_NEED)
        {
            /* got needed colors for full-blown, 48-sector/2-hue color wheel */
            wheel->DrawFunc = DrawWheel98;
            remap = remap50;
        }
        else if (wheel->Entries >= WHEEL_50_NEED)
        {
            /* got needed colors for 48-sector wheel, free unused pens */
            FreeUnsedPens(scr,wheel,WHEEL_50_NEED);

            wheel->DrawFunc = DrawWheel50;
            remap = remap50;
        }
        else if (wheel->Entries >= WHEEL_26_NEED)
        {
            /* got needed colors for 24-sector wheel, free unused pens */
            FreeUnsedPens(scr,wheel,WHEEL_26_NEED);

            wheel->DrawFunc = DrawWheel26;
            remap = remap26;
        }
        else if (wheel->Entries >= WHEEL_14_NEED)
        {
            /* got needed colors for 12-sector wheel, free unused pens */
            FreeUnsedPens(scr,wheel,WHEEL_14_NEED);

            wheel->DrawFunc = DrawWheel14;
            remap = remap14;
        }
        else if (wheel->Entries >= WHEEL_8_NEED)
        {
            /* got needed colors for 6-sector wheel, free unused pens */
            FreeUnsedPens(scr,wheel,WHEEL_8_NEED);

            wheel->DrawFunc = DrawWheel8;
            remap = remap8;
        }
        else if (wheel->Entries >= WHEEL_5_NEED)
        {
            /* got needed colors for 3-sector wheel, free unused pens */
            FreeUnsedPens(scr,wheel,WHEEL_5_NEED);

            wheel->DrawFunc = DrawWheel5;
            remap = remap5;
        }
        else
        {
            /* now entering monochrome land... free unused pens */
            FreeUnsedPens(scr,wheel,0);

            /* see if can get black and white, else use shine/shadow pens */
            while (wheel->Entries < WHEEL_2_NEED)
            {
            ULONG bw;

                bw = (wheel->Entries ? 0 : 0xffffffff);

                pen = ObtainBestPenA(scr->ViewPort.ColorMap,
                    bw, bw, bw, &obtain_tags[1]);

                if (pen == -1)
                    break;

                wheel->CPens[wheel->Entries++] = pen;
            }

            wheel->DrawFunc = DrawWheel2;
            remap = remap2;
        }

        /* terminate pen array */
        wheel->CPens[wheel->Entries] = (UWORD)~0;

        /* set colors for donated pens */
        max = MIN(wheel->FirstObtained,wheel->Entries);
        for (i = 0, pens = wheel->CPens; i < max; i++)
        {
	    ExpandRGBEntry(rgb,i);

            SetRGB32(&scr->ViewPort,*pens++,rgb[0],rgb[1],rgb[2]);
        }

        /* created remapped pen array for drawing */
        for (i=0,pens = wheel->SPens;i<wheel->Entries;i++,remap++)
        {
	    if (i >= 49) j = remap[-48] + 48;		/* hideous, isn't it */
	    else j = *remap;

	    *pens++ = wheel->CPens[j];
	}
    }

    return TRUE;
}


/*****************************************************************************/


#define HUE_SIDE (0xffff / 6 + 1)

static VOID ConvertHSB2RGB(struct ColorWheelHSB *hsb, struct ColorWheelRGB *rgb)
{
ULONG i, f, p, q, t, v;

    if (hsb->cw_Saturation == 0)
    {
        rgb->cw_Red = rgb->cw_Green = rgb->cw_Blue = hsb->cw_Brightness;
    }
    else
    {
        v = hsb->cw_Brightness;
        i = hsb->cw_Hue / HUE_SIDE;
        f = (hsb->cw_Hue % HUE_SIDE) * 6;
        f += f >> 14;

        p = hsb->cw_Brightness * (0xffff - hsb->cw_Saturation) >> 16;
        q = hsb->cw_Brightness * (0xffff - (hsb->cw_Saturation * f >> 16)) >> 16;
        t = hsb->cw_Brightness * (0xffff - (hsb->cw_Saturation * (0xffff - f) >> 16)) >> 16;

        switch (i)
        {
            case 0: rgb->cw_Red = v; rgb->cw_Green = t; rgb->cw_Blue = p; break;
            case 1: rgb->cw_Red = q; rgb->cw_Green = v; rgb->cw_Blue = p; break;
            case 2: rgb->cw_Red = p; rgb->cw_Green = v; rgb->cw_Blue = t; break;
            case 3: rgb->cw_Red = p; rgb->cw_Green = q; rgb->cw_Blue = v; break;
            case 4: rgb->cw_Red = t; rgb->cw_Green = p; rgb->cw_Blue = v; break;
            case 5: rgb->cw_Red = v; rgb->cw_Green = p; rgb->cw_Blue = q; break;
        }
    }
}


/*****************************************************************************/


static VOID ConvertRGB2HSB(struct ColorWheelRGB *rgb, struct ColorWheelHSB *hsb)
{
ULONG min, max, diff, rc, gc, bc;
LONG  h;

    max = MAX(rgb->cw_Red,rgb->cw_Green);
    max = MAX(max,rgb->cw_Blue);

    min = MIN(rgb->cw_Red,rgb->cw_Green);
    min = MIN(min,rgb->cw_Blue);

    diff = max - min;

    hsb->cw_Brightness = max;
    hsb->cw_Saturation = (max ? diff * 0xffff / max : 0);

    if (hsb->cw_Saturation != 0)
    {
        rc = (max - rgb->cw_Red) * 0xffff / diff;
        gc = (max - rgb->cw_Green) * 0xffff / diff;
        bc = (max - rgb->cw_Blue) * 0xffff / diff;

        if (rgb->cw_Red == max)
        {
            h = bc - gc;
            if (h < 0)
                h += 6 * 0xffff;
        }
        else if (rgb->cw_Green == max)
        {
            h = 2 * 0xffff + rc - bc;
        }
        else
        {
            h = 4 * 0xffff + gc - rc;
        }

        hsb->cw_Hue = h / 6;
    }
}


/*****************************************************************************/


static LONG SetColorWheelAttrs(Class *cl, Object *o, struct opSet *msg, BOOL init)
{
struct TagItem       *tags = msg->ops_AttrList;
struct TagItem       *tlist = tags;
struct Wheel         *wheel = INST_DATA( cl, o );
struct ColorWheelHSB  old_HSB;
struct ColorWheelRGB  new_RGB;
struct ColorWheelRGB  cur_RGB;
BOOL                  red = FALSE;
BOOL                  green = FALSE;
BOOL                  blue = FALSE;
LONG                  changes = 0;
ULONG                 tagval;
struct TagItem        tl[2];

    old_HSB = wheel->Current;                   /* save old state               */

    /* iterate through the other tags and setup what values are needed */
    while (tags = NextTagItem(&tlist))
    {
        tagval = tags->ti_Data;
        switch (tags->ti_Tag)
        {
            case WHEEL_Hue       : wheel->Current.cw_Hue = (tagval >> 16);
                                   break;

            case WHEEL_Saturation: wheel->Current.cw_Saturation = (tagval >> 16);
                                   break;

            case WHEEL_Brightness: wheel->Current.cw_Brightness = (tagval >> 16);
                                   break;

            case WHEEL_HSB       : wheel->Current = *(struct ColorWheelHSB *)tagval;
                                   wheel->Current.cw_Hue        = (wheel->Current.cw_Hue        >> 16);
                                   wheel->Current.cw_Saturation = (wheel->Current.cw_Saturation >> 16);
                                   wheel->Current.cw_Brightness = (wheel->Current.cw_Brightness >> 16);
                                   break;

            case WHEEL_Red       : new_RGB.cw_Red = (tagval >> 16);
                                   red = TRUE;
                                   break;

            case WHEEL_Green     : new_RGB.cw_Green = (tagval >> 16);
                                   green = TRUE;
                                   break;

            case WHEEL_Blue      : new_RGB.cw_Blue = (tagval >> 16);
                                   blue = TRUE;
                                   break;

            case WHEEL_RGB       : new_RGB = *(struct ColorWheelRGB *)tagval;
                                   new_RGB.cw_Red   = (new_RGB.cw_Red   >> 16);
                                   new_RGB.cw_Green = (new_RGB.cw_Green >> 16);
                                   new_RGB.cw_Blue  = (new_RGB.cw_Blue  >> 16);
                                   red = green = blue = TRUE;
                                   break;

            case WHEEL_MaxPens   : wheel->MaxPens = (UWORD)tagval;
                                   break;

            case WHEEL_GradientSlider: wheel->GradientSlider = (Object *)tagval;
                                       break;
        }
    }

    /* determine if anything changed */

    if (red == TRUE || green == TRUE || blue == TRUE)
    {
        ConvertHSB2RGB(&wheel->Current,&cur_RGB);

        if (red)   cur_RGB.cw_Red   = new_RGB.cw_Red;
        if (green) cur_RGB.cw_Green = new_RGB.cw_Green;
        if (blue)  cur_RGB.cw_Blue  = new_RGB.cw_Blue;

        ConvertRGB2HSB(&cur_RGB,&wheel->Current);
    }

    if (wheel->Current.cw_Hue != old_HSB.cw_Hue || wheel->Current.cw_Saturation != old_HSB.cw_Saturation)
        changes = 1;

    if ((wheel->Current.cw_Brightness != old_HSB.cw_Brightness) || init)
    {
        if (wheel->GradientSlider)
        {
            tl[0].ti_Tag  = GRAD_CurVal;
            tl[0].ti_Data = 0xffff - wheel->Current.cw_Brightness;
            tl[1].ti_Tag  = TAG_DONE;
            DoMethod(wheel->GradientSlider,OM_SET,tl,msg->ops_GInfo);
        }
        changes |= 2;
    }

    return changes;
}


/*****************************************************************************/


static VOID InitColorWheelAttrs(Class *cl, Object *o, struct opSet *msg)
{
struct TagItem *tags = msg->ops_AttrList;
struct TagItem *tlist = tags;
struct Wheel   *wheel = INST_DATA( cl, o );
ULONG           tagval;
UWORD          *pens;
struct Screen  *screen = NULL;

    /* iterate through the other tags and setup what values are needed */
    while (tags = NextTagItem(&tlist))
    {
        tagval = tags->ti_Data;
        switch (tags->ti_Tag)
        {
            case WHEEL_Abbrv   : wheel->HueAbbrv = (char *)tagval;
                                 break;

            case WHEEL_Screen  : screen = (struct Screen *)tagval;
                                 break;

            case WHEEL_Donation: pens = (UWORD *)tagval;
                                 while (*pens != (UWORD)~0)
                                 {
                                     wheel->CPens[wheel->Entries++] = *pens;
                                     pens++;
                                 }
                                 break;

            case WHEEL_BevelBox: if (tagval)
                                     wheel->Flags |= WHEELF_BEVELBOX;
                                 else
                                     wheel->Flags &= ~(WHEELF_BEVELBOX);
                                 break;
        }
    }

    /* check for atrributes that can be SET */
    SetColorWheelAttrs( cl, o, msg, TRUE );

    if (screen)
	InitColorWheelStruct(wheel,screen);
}


/*****************************************************************************/


LONG DoOpSet(Class *cl, struct opSet *msg, struct Gadget *g)
{
LONG refresh;

    refresh = (ULONG) DoSuperMethodA( cl, (Object *)g, (Msg *)msg ) ? 4 : 0;

    /* set attibutes */
    refresh |= SetColorWheelAttrs( cl, (Object *)g, msg, FALSE );

    if ( refresh && OCLASS(g) == cl && msg->ops_GInfo != NULL )
    {
	if (refresh & 1)
            WheelUpdate(msg->ops_GInfo,g);

	return 0;                          /* don't need refresh any more */
    }

    return refresh;
}


/*****************************************************************************/


static LONG DoOpGet(Class *cl, struct opGet *msg, struct Gadget *g)
{
struct Wheel         *wheel = INST_DATA( cl, (Object *)g );
struct ColorWheelRGB  cur_RGB;
struct ColorWheelHSB  hsb;

    if (wheel->GradientSlider)
    {
        DoMethod(wheel->GradientSlider,OM_GET,GRAD_CurVal,&wheel->Current.cw_Brightness);
        wheel->Current.cw_Brightness = 0xffff - wheel->Current.cw_Brightness;
    }

    switch (msg->opg_AttrID)
    {
        case WHEEL_Hue       : *msg->opg_Storage = ShiftColor32(wheel->Current.cw_Hue);
                               break;

        case WHEEL_Saturation: *msg->opg_Storage = ShiftColor32(wheel->Current.cw_Saturation);
                               break;

        case WHEEL_Brightness: *msg->opg_Storage = ShiftColor32(wheel->Current.cw_Brightness);
                               break;

        case WHEEL_HSB       : hsb.cw_Hue        = ShiftColor32(wheel->Current.cw_Hue);
                               hsb.cw_Saturation = ShiftColor32(wheel->Current.cw_Saturation);
                               hsb.cw_Brightness = ShiftColor32(wheel->Current.cw_Brightness);
                               *(struct ColorWheelHSB *)msg->opg_Storage = hsb;
                               break;

        case WHEEL_Red       : ConvertHSB2RGB(&wheel->Current,&cur_RGB);
                               *msg->opg_Storage = ShiftColor32(cur_RGB.cw_Red);
                               break;

        case WHEEL_Green     : ConvertHSB2RGB(&wheel->Current,&cur_RGB);
                               *msg->opg_Storage = ShiftColor32(cur_RGB.cw_Green);
                               break;

        case WHEEL_Blue      : ConvertHSB2RGB(&wheel->Current,&cur_RGB);
                               *msg->opg_Storage = ShiftColor32(cur_RGB.cw_Blue);
                               break;

        case WHEEL_RGB       : ConvertHSB2RGB(&wheel->Current,&cur_RGB);
                               cur_RGB.cw_Red   = ShiftColor32(cur_RGB.cw_Red);
                               cur_RGB.cw_Green = ShiftColor32(cur_RGB.cw_Green);
                               cur_RGB.cw_Blue  = ShiftColor32(cur_RGB.cw_Blue);
                               *(struct ColorWheelRGB *)msg->opg_Storage = cur_RGB;
                               break;

        default              : return(DoSuperMethodA( cl, (Object *)g, (Msg *)msg ));
    }

    return 1;
}


/*****************************************************************************/


LONG ASM ClassDispatcher(REG(a0) Class         *cl,
                         REG(a1) ULONG         *msg,
                         REG(a2) struct Gadget *g)
{
struct Wheel *wheel;
LONG          i;
LONG          x_center, y_center;
LONG          x_rad, y_rad;

    switch (*msg)
    {
        case GM_RENDER     : WheelRender((struct gpRender *)msg,g);
                             break;

        case GM_HELPTEST   : if (WheelHitTest((struct gpHitTest *)msg,g) == GMR_GADGETHIT)
                                 return(GMR_HELPHIT);

                             return(GMR_NOHELPHIT);

        case GM_HITTEST    : return(WheelHitTest((struct gpHitTest *)msg,g));

        case GM_GOACTIVE   : if (((struct gpInput *)msg)->gpi_IEvent == NULL)
                                 return GMR_NOREUSE;

                             wheel = INST_DATA( cl, (Object *)g );
                             wheel->Saved = wheel->Current;
                             g->Flags |= GFLG_SELECTED;

        case GM_HANDLEINPUT: return(WheelHandle((struct gpInput *)msg,g));

        case GM_GOINACTIVE : g->Flags &= ~(GFLG_SELECTED);
                             break;

        case OM_NEW        : g = (struct Gadget *)DoSuperMethodA( cl, (Object *)g, (Msg *)msg);
                             if (!g)
                                 return(NULL);

                             /* save pointer to instance data in gadget SpecialInfo */

                             wheel = INST_DATA( cl, (Object *)g );
                             g->SpecialInfo = (APTR)wheel;

                             /* initialize instance data */

                             x_rad    = g->Width / 2;
                             x_center = g->LeftEdge + x_rad;
                             y_rad    = g->Height / 2;
                             y_center = g->TopEdge + y_rad;

                             wheel->XCenter               = x_center;
                             wheel->YCenter               = y_center;
                             wheel->XRadius               = wheel->DXCenter = x_rad;
                             wheel->YRadius               = wheel->DYCenter = y_rad;
                             wheel->DotXCenter            = x_center;
                             wheel->DotYCenter            = y_center;
                             wheel->Current.cw_Brightness = 0xffff;
                             wheel->Current.cw_Hue        = 0;
                             wheel->Current.cw_Saturation = 0xffff;
                             wheel->RasXSize              = 2 * wheel->XRadius + 1 + AREA_X_FUDGE;
                             wheel->RasYSize              = 2 * wheel->YRadius + 1 + AREA_Y_FUDGE;
                             wheel->HueAbbrv              = "GCBMRY";
                             wheel->MaxPens               = 256;

                             InitColorWheelAttrs( cl, (Object *)g, (struct opSet *)msg );
                             return (LONG)g;

        case OM_GET        : return(DoOpGet(cl,(struct opGet *)msg,g));

        case OM_SET        :
        case OM_UPDATE     : return(DoOpSet(cl,(struct opSet *)msg,g));

        case OM_DISPOSE    : wheel = INST_DATA( cl, (Object *)g );

                             /* dispose of any allocated memory */

                             FreeBitMap(wheel->DBitMap);

			     if (wheel->OuterRing)
			     {
	    			 FreeRaster(wheel->OuterRing,wheel->RingXSize,wheel->RingYSize);
                                 FreeRaster(wheel->InnerRing,wheel->RingXSize,wheel->RingYSize);
                             }

                             if (wheel->DInfo)
                                 FreeScreenDrawInfo(wheel->Screen,wheel->DInfo);

                             if (wheel->Screen)
                             {
                                 for (i = wheel->FirstObtained; i < wheel->Entries; i++)
                                     ReleasePen(wheel->Screen->ViewPort.ColorMap,wheel->CPens[i]);
                             }

        default            : return(DoSuperMethodA( cl, (Object *)g, (Msg *)msg ));
    }
}


/*****************************************************************************/


VOID ASM ConvertRGBToHSBLVO(REG(a0) struct ColorWheelRGB *rgb,
                            REG(a1) struct ColorWheelHSB *hsb)
{
struct ColorWheelRGB rgb16;
struct ColorWheelHSB hsb16;

    rgb16.cw_Red   = rgb->cw_Red >> 16;
    rgb16.cw_Green = rgb->cw_Green >> 16;
    rgb16.cw_Blue  = rgb->cw_Blue >> 16;

    ConvertRGB2HSB(&rgb16,&hsb16);

    hsb->cw_Hue        = ShiftColor32(hsb16.cw_Hue);
    hsb->cw_Saturation = ShiftColor32(hsb16.cw_Saturation);
    hsb->cw_Brightness = ShiftColor32(hsb16.cw_Brightness);
}


/*****************************************************************************/


VOID ASM ConvertHSBToRGBLVO(REG(a0) struct ColorWheelHSB *hsb,
                            REG(a1) struct ColorWheelRGB *rgb)
{
struct ColorWheelRGB rgb16;
struct ColorWheelHSB hsb16;

    hsb16.cw_Hue        = hsb->cw_Hue >> 16;
    hsb16.cw_Saturation = hsb->cw_Saturation >> 16;
    hsb16.cw_Brightness = hsb->cw_Brightness >> 16;

    ConvertHSB2RGB(&hsb16,&rgb16);

    rgb->cw_Red   = ShiftColor32(rgb16.cw_Red);
    rgb->cw_Green = ShiftColor32(rgb16.cw_Green);
    rgb->cw_Blue  = ShiftColor32(rgb16.cw_Blue);
}

