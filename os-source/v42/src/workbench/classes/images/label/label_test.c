/* SC LINK LIB lib:Debug.lib DEBUG=L DATA=FAR NOSTKCHK PARMS=REG test */

/*****************************************************************************
 *
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992-1994
 * Commodore-Amiga, Inc. All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability,
 * performance, currentness, or operation of this software, and all use is at
 * your own risk. Neither Commodore nor the authors assume any responsibility
 * or liability whatsoever with respect to your use of this software.
 *
 *****************************************************************************
 * label_test.c
 * test program for the label.image
 * Written by John J. Szucs
 *
 */

#include <dos/dos.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <intuition/imageclass.h>
#include <intuition/intuitionbase.h>
#include <stdlib.h>
#include <stdio.h>

#include <clib/macros.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
//#include <clib/debug_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>

/*****************************************************************************/

#define DO_WINDOW   TRUE

/*****************************************************************************/

#define IDCMP_FLAGS IDCMP_CLOSEWINDOW | IDCMP_VANILLAKEY | IDCMP_GADGETUP | IDCMP_GADGETDOWN \
            | IDCMP_MOUSEMOVE | IDCMP_INTUITICKS | IDCMP_MOUSEBUTTONS

/*****************************************************************************/

extern struct Library *SysBase, *DOSBase;
struct Library *IntuitionBase;
STRPTR label = "Hello, world\n@{B}Bold@{UB} Unbold\n@{I}Italic@{UI} Unitalic\n@{U}Underline@{UU} Ununderline\nUnderscore &Character\n";

/*****************************************************************************/

#define ASM     __asm
#define REG(x)  register __ ## x

/*****************************************************************************/

/* Try opening the class library from a number of common places */
struct ClassLibrary *openclass (STRPTR name, ULONG version)
{
    struct Library *retval;
    UBYTE buffer[256];

    if ((retval = OpenLibrary (name, version)) == NULL)
    {
    sprintf (buffer, ":classes/%s", name);
    if ((retval = OpenLibrary (buffer, version)) == NULL)
    {
        sprintf (buffer, "classes/%s", name);
        retval = OpenLibrary (buffer, version);
    }
    }
    return (struct ClassLibrary *) retval;
}

/*****************************************************************************/

LONG DoClassLibraryMethod (struct ClassLibrary *cl, Msg msg)
{
    LONG (ASM * disp) (REG (a0) Class *, REG (a2) Object *, REG (a1) Msg msg);

    kprintf ("cl=%08lx, class=%08lx\n", cl, cl->cl_Class);
    disp = cl->cl_Class->cl_Dispatcher.h_Entry;
    return (*disp) (cl->cl_Class, (Object *) cl->cl_Class, msg);
}

/*****************************************************************************/

void main (int argc, char **argv)
{
    struct ClassLibrary *imageLib;
    struct IntuiMessage *imsg;
    struct DrawInfo *dri;
    struct Screen *scr;
    struct Window *win;
    struct Image *im;
    BOOL going = TRUE;
    ULONG sigr;

    struct TagItem ti[6];
    struct gpDomain gpd;
    LONG i;

    if (IntuitionBase = OpenLibrary ("intuition.library", 39))
    {
    scr = ((struct IntuitionBase *) IntuitionBase)->FirstScreen;

    dri = GetScreenDrawInfo (scr);

#if 1
    if (imageLib = (struct ClassLibrary *) OpenLibrary ("images/label.image", 0))
#else
    if (imageLib = openclass ("images/label.image", 0))
#endif
    {
        /* Get the domain of the object */
        gpd.MethodID    = GM_DOMAIN;
        gpd.gpd_GInfo   = NULL;
        gpd.gpd_RPort   = &scr->RastPort;
        gpd.gpd_Which   = GDOMAIN_MINIMUM;
        i = 0;
        ti[i].ti_Tag    = IA_Text;
        ti[i++].ti_Data = (LONG) label;
        ti[i].ti_Tag    = SYSIA_DrawInfo;
        ti[i++].ti_Data = (LONG) dri;
        ti[i].ti_Tag    = IA_Underscore;
        ti[i++].ti_Data   = (LONG) '&';
        ti[i].ti_Tag    = TAG_DONE;
        gpd.gpd_Attrs   = ti;
        DoClassLibraryMethod (imageLib, (Msg)&gpd);

#ifdef DO_WINDOW

        if (win = OpenWindowTags (NULL,
                    WA_InnerWidth,      gpd.gpd_Width + 4,
                    WA_InnerHeight,     gpd.gpd_Height + 4,
                    WA_IDCMP,       IDCMP_FLAGS,
                    WA_DragBar,     TRUE,
                    WA_DepthGadget,     TRUE,
                    WA_CloseGadget,     TRUE,
                    WA_SimpleRefresh,   TRUE,
                    WA_NoCareRefresh,   TRUE,
                    WA_Activate,        TRUE,
                    WA_CustomScreen,    scr,
                    TAG_DONE))
        {
        /* Create the led image */
        if (im = NewObject (NULL, "label.image",
                    SYSIA_DrawInfo,     dri,
                    IA_FGPen,       1,
                    IA_BGPen,       0,
                    IA_Width,       gpd.gpd_Width,
                    IA_Height,      gpd.gpd_Height,
                    IA_Text,        (ULONG)label,
                    IA_Underscore,      (ULONG)'&',
                    TAG_DONE))
        {
            /* Draw the image */
            DrawImage (win->RPort, im, win->BorderLeft + 2, win->BorderTop + 2);

            while (going)
            {
            sigr = Wait ((1L << win->UserPort->mp_SigBit | SIGBREAKF_CTRL_C));

            if (sigr & SIGBREAKF_CTRL_C)
                going = FALSE;

            while (imsg = (struct IntuiMessage *) GetMsg (win->UserPort))
            {
                switch (imsg->Class)
                {
                case IDCMP_CLOSEWINDOW:
                    going = FALSE;
                    break;

                case IDCMP_VANILLAKEY:
                    switch (imsg->Code)
                    {
                    case 27:
                    case 'q':
                    case 'Q':
                        going = FALSE;
                        break;
                    }
                    break;

                }

                ReplyMsg ((struct Message *) imsg);
            }
            }
            DisposeObject (im);
        }

        CloseWindow (win);
        }
        else
        Printf ("couldn't open the window\n");

#else

        printf("Width=%d, Height=%d\n", gpd.gpd_Width, gpd.gpd_Height);

#endif /* DO_WINDOW */

        CloseLibrary ((struct Library *) imageLib);
    }
    else
        Printf ("couldn't open classes:images/label.image\n");

    CloseLibrary (IntuitionBase);
    }
}
