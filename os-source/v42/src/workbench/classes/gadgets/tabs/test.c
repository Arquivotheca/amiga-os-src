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
 * tabs_test.c
 * test program for the tabs.gadget
 * Written by David N. Junod
 *
 */

#include <dos/dos.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/intuitionbase.h>
#include <gadgets/tabs.h>
#include <stdlib.h>
#include <stdio.h>

#include <clib/macros.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>

/*****************************************************************************/

#define	IDCMP_FLAGS	IDCMP_CLOSEWINDOW | IDCMP_VANILLAKEY | IDCMP_GADGETUP \
			| IDCMP_MOUSEMOVE | IDCMP_INTUITICKS | IDCMP_MOUSEBUTTONS

/*****************************************************************************/

struct ClassLibrary *openclass (STRPTR name, ULONG version);

/*****************************************************************************/

extern struct Library *SysBase, *DOSBase;
struct Library *IntuitionBase;

/*****************************************************************************/

TabLabel labels[] =
{
    {"Display",		-1, -1, -1, -1},
    {"Edit",		-1, -1, -1, -1},
    {"File",		-1, -1, -1, -1},
    NULL
};

/*****************************************************************************/

struct TextAttr topaz8 = {"topaz.font", 8,};

/*****************************************************************************/

void main (int argc, char **argv)
{
    struct IntuiMessage *imsg;
    struct ClassLibrary *gLib;
    struct Screen *scr;
    struct Window *win;
    struct Gadget *gad;
    struct Gadget *g;
    BOOL going = TRUE;
    ULONG sigr;

    if (IntuitionBase = OpenLibrary ("intuition.library", 37))
    {
	scr = ((struct IntuitionBase *)IntuitionBase)->FirstScreen;

	if (gLib = openclass ("gadgets/tabs.gadget", 37))
	{
	    if (win = OpenWindowTags (NULL,
				      WA_Title,		"tabs.gadget Test",
				      WA_InnerWidth,	320,
				      WA_InnerHeight,	8 + 6 + 34,
				      WA_IDCMP,		IDCMP_FLAGS,
				      WA_DragBar,	TRUE,
				      WA_DepthGadget,	TRUE,
				      WA_CloseGadget,	TRUE,
				      WA_SimpleRefresh,	TRUE,
				      WA_NoCareRefresh,	TRUE,
				      WA_Activate,	TRUE,
				      WA_SizeGadget,	TRUE,
				      WA_MinWidth,	300,
				      WA_MinHeight,	scr->BarHeight + 1 + 34,
				      WA_MaxWidth,	1024,
				      WA_MaxHeight,	1024,
				      WA_CustomScreen,	scr,
				      TAG_DONE))
	    {
		/* The height of the gadget should be the font height + 6 */
		if (gad = NewObject (NULL, "tabs.gadget",
					GA_Top,		win->BorderTop + 2,
					GA_Left,	win->BorderLeft + 4,
					GA_Height,	8 + 6,
					GA_RelWidth,	-(win->BorderLeft + 8 + win->BorderRight),
					GA_TextAttr,	&topaz8,
					GA_RelVerify,	TRUE,
					GA_Immediate,	TRUE,
					TABS_Labels,	labels,
					TABS_Current,	0,
					TAG_DONE))
		{
		    /* Add the gadget */
		    AddGList (win, gad, -1, 1, NULL);
		    RefreshGList (gad, win, NULL, -1);

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
					case  27:
					case 'q':
					case 'Q':
					    going = FALSE;
					    break;
				    }
				    break;

				case IDCMP_GADGETUP:
				    g = (struct Gadget *) imsg->IAddress;
				    Printf ("id=%ld code=%04lx\n", (ULONG)g->GadgetID, (ULONG)imsg->Code);
				    break;
			    }

			    ReplyMsg ((struct Message *) imsg);
			}
		    }

		    /* Delete the gadget */
		    RemoveGList (win, gad, 1);
		    DisposeObject (gad);
		}

		CloseWindow (win);
	    }
	    else
		Printf ("couldn't open the window\n");

	    CloseLibrary ((struct Library *) gLib);
	}
	else
	    Printf ("couldn't open classes:gadgets/tabs.gadget\n");

	CloseLibrary (IntuitionBase);
    }
}

/*****************************************************************************/

/* Try opening the class library from a number of common places */
struct ClassLibrary *openclass (STRPTR name, ULONG version)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
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
