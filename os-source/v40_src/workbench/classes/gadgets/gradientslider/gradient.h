#ifndef GRADIENT_H
#define GRADIENT_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/screens.h>
#include <graphics/rastport.h>

#include "gradientbase.h"


/*****************************************************************************/


struct SliderInfo
{
    LONG           CurrentValue;       /* current value of slider      */
    LONG           SaveValue;	       /* to undo when RMB is clicked  */
    LONG           MaxValue;           /* max value of slider          */
    LONG           SkipValue;          /* amount to skip w/ body click */
    UWORD         *PenArray;           /* pens to use                  */
    struct BitMap *DitherBMap;         /* bitmap holding dithering     */
    BOOL           VerticalSlider;     /* slider is vertical or not?   */
    UWORD          KnobPixels;         /* size of slider body          */

    /* calculated variables */
    struct IBox    ContainerExtent;    /* size of container            */
    struct IBox    KnobExtent;         /* size of knob                 */

    WORD           LastWidth;          /* last gadget width & height   */
    WORD           LastHeight;
    UWORD          PenCount;           /* number of pens in array      */
    UWORD          PixelRange;         /* slideable range in pixels    */
    WORD           MouseOffset;        /* offset of mouse drag...      */
    UWORD          PixelPos;           /* pixels from slider start     */
    UWORD          PixelSpan;          /* pixel width of slider        */
};


/*****************************************************************************/


LONG ASM ClassDispatcher(REG(a0) Class         *cl,
                         REG(a1) ULONG         *msg,
                         REG(a2) struct Gadget *g);


/*****************************************************************************/


#endif /* GRADIENT_H */
