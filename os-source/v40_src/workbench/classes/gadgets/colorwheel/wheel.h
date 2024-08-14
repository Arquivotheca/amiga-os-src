#ifndef WHEEL_H
#define WHEEL_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/screens.h>
#include <graphics/rastport.h>

#include "wheelbase.h"
#include "colorwheel.h"


/*****************************************************************************/


/* maximum number of pens that will be obtained by the color wheel */
#define MAX_COLORS_IN_WHEEL     (96 + 2 + 1)


/*****************************************************************************/


typedef VOID (*DRAWFUNC)(struct RastPort *, struct Wheel *);

struct Wheel
{
    DRAWFUNC              DrawFunc;               /* call this to render      */
    UWORD                 Flags;                  /* flags (see below)        */
    UWORD	          MaxPens;		  /* max # of pens wheel can allocate */
    WORD                  XCenter;                /* absolute center of wheel */
    WORD                  YCenter;
    WORD                  DXCenter;               /* relative center of wheel */
    WORD                  DYCenter;
    WORD                  XRadius;                /* radii of wheel           */
    WORD                  YRadius;
    WORD                  BorWidth;               /* size of black border     */
    WORD                  BorHeight;
    WORD                  DotXCenter;             /* current dot location     */
    WORD                  DotYCenter;
    WORD                  DotXRadius;             /* radii of dot             */
    WORD                  DotYRadius;
    WORD                  NewDotXCenter;          /* new dot location         */
    WORD                  NewDotYCenter;
    Object               *GradientSlider;         /* optional slider attached to this wheel */
    struct ColorWheelHSB  Current;                /* current HSB settings     */
    struct ColorWheelHSB  Saved;                  /* previous HSB settings    */
    char                 *HueAbbrv;               /* hue abbreviations        */
    UWORD                 RasXSize;               /* size of TmpRas           */
    UWORD                 RasYSize;
    struct Screen        *Screen;                 /* screen wheel is on       */
    struct DrawInfo      *DInfo;                  /* DrawInfo for that screen */
    struct TmpRas         TmpRas;                 /* TmpRas for AreaEllipse   */
    struct BitMap        *DBitMap;                /* bitmap for dot backsave  */
    PLANEPTR		  OuterRing;		  /* cached dot outer ring    */
    PLANEPTR		  InnerRing;		  /* cached dot inner ring    */
    UWORD                 RingXSize;              /* size cached dot buffers  */
    UWORD                 RingYSize;
    UWORD                 Entries;                /* total pen entries        */
    UWORD                 FirstObtained;          /* first via ObtainBestPen  */
    UWORD                 CPens[MAX_COLORS_IN_WHEEL];     /* pens to use      */
    UWORD                 SPens[MAX_COLORS_IN_WHEEL];     /* sorted pens      */
};

#define WHEELF_DOT      (1<<0)          /* dot save buffer valid        */
#define WHEELF_BEVELBOX (1<<1)          /* draw bevel box around wheel  */


/*****************************************************************************/


/* both of these values are adjusted by pixel resolution */
#define WHEEL_BORDER_WIDTH  2               /* standard black border radius */
#define WHEEL_DOT_RADIUS    3               /* standard dor radius          */


/*****************************************************************************/


/* handy constants */
#define MAX_WHEEL_RANGE (1L<<16)
#define MAX_WHEEL_COMP  (MAX_WHEEL_RANGE-1)


/*****************************************************************************/


/* count of pens needed by each mode */
#define WHEEL_2_NEED  2               /* actually, can get by with 0  */
#define WHEEL_5_NEED  4
#define WHEEL_8_NEED  7
#define WHEEL_14_NEED 13
#define WHEEL_26_NEED 25
#define WHEEL_50_NEED 49
#define WHEEL_98_NEED 97

#define WHEEL_MAX_NEED WHEEL_98_NEED


/*****************************************************************************/


LONG ASM ClassDispatcher(REG(a0) Class         *cl,
                         REG(a1) ULONG         *msg,
                         REG(a2) struct Gadget *g);

VOID ASM ConvertRGBToHSBLVO(REG(a0) struct ColorWheelRGB *rgb,
                            REG(a1) struct ColorWheelHSB *hsb);

VOID ASM ConvertHSBToRGBLVO(REG(a0) struct ColorWheelHSB *hsb,
                            REG(a1) struct ColorWheelRGB *rgb);


/*****************************************************************************/


#endif /* WHEEL_H */
