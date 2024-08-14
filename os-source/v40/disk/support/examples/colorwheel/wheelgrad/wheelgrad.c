;/*
SC RESOPT PARM=REGISTERS UCHAR CONSTLIB STREQ ANSI NOSTKCHK NOICONS OPT OPTPEEP wheelgrad.c
Slink LIB:c.o wheelgrad.o TO WheelGrad LIB LIB:sc.lib SC SD
Quit
*/

/*
COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992-1993
Commodore-Amiga, Inc.  All rights reserved.

DISCLAIMER: This software is provided "as is".  No representations or
warranties are made with respect to the accuracy, reliability, performance,
currentness, or operation of this software, and all use is at your own risk.
Neither commodore nor the authors assume any responsibility or liability
whatsoever with respect to your use of this software.
*/

/* WheelGrad.c - simple example of colorwheel and gradient slider
 *
 * Puts up a colorwheel and gradient slider and changes the gradient slider
 * color based on where the colorwheel knob is moved.
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <graphics/displayinfo.h>
#include <intuition/gadgetclass.h>
#include <gadgets/colorwheel.h>
#include <gadgets/gradientslider.h>

#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/colorwheel_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/colorwheel_pragmas.h>

#include <stdio.h>


/****************************************************************************/


#define MAXGRADPENS 4


/****************************************************************************/


extern struct Library *DOSBase;
extern struct Library *SysBase;
struct Library        *IntuitionBase;
struct Library        *GfxBase;
struct Library        *ColorWheelBase;
struct Library        *GradientSliderBase;


/****************************************************************************/


VOID main(VOID)
{
struct Screen        *screen;
struct Window        *window;
struct IntuiMessage  *intuiMsg;
struct Gadget        *wheel;
struct Gadget        *slider;
struct ColorWheelRGB  rgb;
struct ColorWheelHSB  hsb;
BOOL                  quit;
ULONG                 colortable[3];
WORD                  pens[16];
WORD                  numPens;
WORD                  i;

    if (IntuitionBase      = OpenLibrary("intuition.library",39))
    {
        GfxBase            = OpenLibrary("graphics.library",39);
        ColorWheelBase     = OpenLibrary("gadgets/colorwheel.gadget",39);
        GradientSliderBase = OpenLibrary("gadgets/gradientslider.gadget",39);
    }

    if (IntuitionBase && GfxBase && ColorWheelBase && GradientSliderBase)
    {
        if (screen = OpenScreenTags(NULL,SA_Depth,         4,
                                         SA_LikeWorkbench, TRUE,
                                         SA_Title,         "WheelGrad",
                                         TAG_DONE))
        {
            /* get the RGB components of color 0 */
            GetRGB32(screen->ViewPort.ColorMap,0,1,colortable);
            rgb.cw_Red   = colortable[0];
            rgb.cw_Green = colortable[1];
            rgb.cw_Blue  = colortable[2];

            /* now convert the RGB values to HSB, and max out B component */
            ConvertRGBToHSB(&rgb,&hsb);
            hsb.cw_Brightness = 0xffffffff;

            numPens = 0;
            while (numPens < MAXGRADPENS)
            {
                hsb.cw_Brightness = 0xffffffff - ((0xffffffff / MAXGRADPENS) * numPens);
                ConvertHSBToRGB(&hsb,&rgb);

                pens[numPens] = ObtainPen(screen->ViewPort.ColorMap,-1,
                                 rgb.cw_Red,rgb.cw_Green,rgb.cw_Blue,PEN_EXCLUSIVE);

                if (pens[numPens] == -1)
                    break;

                numPens++;
            }
            pens[numPens] = ~0;

            /* Create gradient slider and colorwheel gadgets */
            slider = (struct Gadget *)NewObject(NULL,"gradientslider.gadget",
                                                GA_Top,        50,
                                                GA_Left,       177,
                                                GA_Width,      20,
                                                GA_Height,     100,
                                                GRAD_PenArray, pens,
                                                PGA_Freedom,   LORIENT_VERT,
                                                TAG_END);

            wheel = (struct Gadget *)NewObject(NULL, "colorwheel.gadget",
                                               GA_Top,               50,
                                               GA_Left,              50,
                                               GA_Width,             120,
                                               GA_Height,            100,
                                               WHEEL_Red,            colortable[0],
                                               WHEEL_Green,          colortable[1],
                                               WHEEL_Blue,           colortable[2],
                                               WHEEL_Screen,         screen,
                                               WHEEL_GradientSlider, slider,
                                               GA_FollowMouse,       TRUE,
                                               GA_Previous,          slider,
                                               TAG_END);


            if (slider && wheel)
            {
                if (window = OpenWindowTags(NULL,WA_Height,       200,
                                                 WA_Width,        400,
                                                 WA_CustomScreen, screen,
                                                 WA_IDCMP,        IDCMP_CLOSEWINDOW | IDCMP_MOUSEMOVE,
                                                 WA_SizeGadget,   TRUE,
                                                 WA_DragBar,      TRUE,
                                                 WA_CloseGadget,  TRUE,
                                                 WA_Gadgets,      slider,
                                                 WA_Activate,     TRUE,
                                                 TAG_DONE))
                {
                    quit = FALSE;
                    while (!quit)
                    {
                        WaitPort(window->UserPort);
                        intuiMsg = (struct IntuiMessage *)GetMsg(window->UserPort);

                        switch (intuiMsg->Class)
                        {
                            case IDCMP_CLOSEWINDOW: quit = TRUE;
                                                    break;

                            case IDCMP_MOUSEMOVE:
                                 /* Change gradient slider color each time
                                  * colorwheel knob is moved.  This is one
                                  * method you can use.
                                  */

                                 /* Query the colorwheel */
                                 GetAttr(WHEEL_HSB,wheel,(ULONG *)&hsb);

                                 i = 0;
                                 while (i < numPens)
                                 {
                                     hsb.cw_Brightness = 0xffffffff - ((0xffffffff / numPens) * i);
                                     ConvertHSBToRGB(&hsb,&rgb);

                                     SetRGB32(&screen->ViewPort,pens[i],rgb.cw_Red,rgb.cw_Green,rgb.cw_Blue);
                                     i++;
                                 }
                                 break;
                        }
                        ReplyMsg(intuiMsg);
                    }
                    CloseWindow(window);
                }
            }

            /* Get rid of the gadgets */
            DisposeObject(wheel);
            DisposeObject(slider);

            /* Always release the pens */
            while (numPens > 0)
            {
                numPens--;
                ReleasePen(screen->ViewPort.ColorMap,pens[numPens]);
            }

            CloseScreen(screen);
        }
    }

    if (IntuitionBase)
    {
        CloseLibrary(GradientSliderBase);
        CloseLibrary(ColorWheelBase);
        CloseLibrary(GfxBase);
        CloseLibrary(IntuitionBase);
    }
}
