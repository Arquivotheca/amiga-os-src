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
 * button_test.c
 * test program for the button.gadget
 * Written by David N. Junod
 *
 */

#include <dos/dos.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <intuition/classes.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/intuitionbase.h>
#include <gadgets/button.h>
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

#define	INBORDER	TRUE

/*****************************************************************************/

#define	IDCMP_FLAGS	IDCMP_CLOSEWINDOW | IDCMP_VANILLAKEY | IDCMP_GADGETUP | IDCMP_GADGETDOWN \
			| IDCMP_MOUSEMOVE | IDCMP_INTUITICKS | IDCMP_MOUSEBUTTONS

/*****************************************************************************/

struct ClassLibrary *openclass (STRPTR name, ULONG version);

/*****************************************************************************/

static UWORD chip imrewData[14] =
{
 /* Plane 0 */
    0x0301, 0x8000, 0x0F07, 0x8000, 0x3F1F, 0x8000, 0xFF7F, 0x8000,
    0x3F1F, 0x8000, 0x0F07, 0x8000, 0x0301, 0x8000
};

struct Image im1 =
{
    0, 0,			/* LeftEdge, TopEdge */
    17, 7, 1,			/* Width, Height, Depth */
    imrewData,			/* ImageData */
    0x0001, 0x0000,		/* PlanePick, PlaneOnOff */
    NULL			/* NextImage */
};

/*****************************************************************************/

static UWORD chip impp1Data[14] =
{
 /* Plane 0 */
    0xE383, 0x0000, 0xE383, 0xC000, 0xE383, 0xF000, 0xE383, 0xFC00,
    0xE383, 0xF000, 0xE383, 0xC000, 0xE383, 0x0000
};

struct Image impp1 =
{
    0, 0,			/* LeftEdge, TopEdge */
    22, 7, 1,			/* Width, Height, Depth */
    impp1Data,			/* ImageData */
    0x0001, 0x0000,		/* PlanePick, PlaneOnOff */
    NULL			/* NextImage */
};

/*****************************************************************************/

static UWORD chip impp2Data[28] =
{
 /* Plane 0 */
    0x0003, 0x0000, 0x0003, 0xC000, 0x0003, 0xF000, 0x0003, 0xFC00,
    0x0003, 0xF000, 0x0003, 0xC000, 0x0003, 0x0000,
 /* Plane 1 */
    0xE380, 0x0000, 0xE380, 0x0000, 0xE380, 0x0000, 0xE380, 0x0000,
    0xE380, 0x0000, 0xE380, 0x0000, 0xE380, 0x0000
};

struct Image impp2 =
{
    0, 0,			/* LeftEdge, TopEdge */
    22, 7, 2,			/* Width, Height, Depth */
    impp2Data,			/* ImageData */
    0x0003, 0x0000,		/* PlanePick, PlaneOnOff */
    NULL			/* NextImage */
};

/*****************************************************************************/

static UWORD chip impp3Data[28] =
{
 /* Plane 0 */
    0xE380, 0x0000, 0xE380, 0x0000, 0xE380, 0x0000, 0xE380, 0x0000,
    0xE380, 0x0000, 0xE380, 0x0000, 0xE380, 0x0000,
 /* Plane 1 */
    0x0003, 0x0000, 0x0003, 0xC000, 0x0003, 0xF000, 0x0003, 0xFC00,
    0x0003, 0xF000, 0x0003, 0xC000, 0x0003, 0x0000
};

struct Image impp3 =
{
    0, 0,			/* LeftEdge, TopEdge */
    22, 7, 2,			/* Width, Height, Depth */
    impp3Data,			/* ImageData */
    0x0003, 0x0000,		/* PlanePick, PlaneOnOff */
    NULL			/* NextImage */
};

/*****************************************************************************/

static UWORD chip imffData[14] =
{
 /* Plane 0 */
    0xC060, 0x0000, 0xF078, 0x0000, 0xFC7E, 0x0000, 0xFF7F, 0x8000,
    0xFC7E, 0x0000, 0xF078, 0x0000, 0xC060, 0x0000
};

struct Image im4 =
{
    0, 0,			/* LeftEdge, TopEdge */
    17, 7, 1,			/* Width, Height, Depth */
    imffData,			/* ImageData */
    0x0001, 0x0000,		/* PlanePick, PlaneOnOff */
    NULL			/* NextImage */
};

/*****************************************************************************/

static UWORD chip imstopData[7] =
{
 /* Plane 0 */
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
};

struct Image im5 =
{
    0, 0,			/* LeftEdge, TopEdge */
    9, 7, 1,			/* Width, Height, Depth */
    imstopData,			/* ImageData */
    0x0000, 0x0001,		/* PlanePick, PlaneOnOff */
    NULL			/* NextImage */
};

/*****************************************************************************/

extern struct Library *SysBase, *DOSBase;
struct Library *IntuitionBase;
char *buttonClassName = "button.gadget";
struct Image *impp[3] = {&impp1, &impp2, &impp3};

#define	GID_REWIND	0
#define	GID_PP		1
#define	GID_FFORWARD	2
#define	GID_STOP	3
#define	NUMGADS		4
struct Gadget *gads[NUMGADS];

/*****************************************************************************/

static void creategads (struct Window *win)
{
    struct Gadget *g;
    LONG x, w, h;

    /* Determine the width and offset of the gadgets */
    for (g = win->FirstGadget, w = 30, h = win->BorderTop - 2; g; g = g->NextGadget)
    {
	if ((g->GadgetType & GTYP_SYSGADGET) && (g->GadgetType & GTYP_CLOSE))
	{
	    x = g->LeftEdge + g->Width - 1;
	    w = MAX (w, g->Width);
	    break;
	}
    }

    /* Create the rewind button */
    gads[GID_REWIND] = NewObject (NULL, buttonClassName,
				  GA_Top, 1,
				  GA_Left, x,
				  GA_Width, w,
				  GA_Height, h,
				  GA_TopBorder, TRUE,
				  GA_Immediate, TRUE,
				  GA_RelVerify, TRUE,
				  GA_ID, GID_REWIND,
				  BUTTON_Glyph, &im1,
				  TAG_DONE);

    /* Create the play/pause button */
    x += w;
    gads[GID_PP] = NewObject (NULL, buttonClassName,
			      GA_Previous, gads[GID_PP - 1],
			      GA_Top, 1,
			      GA_Left, x,
			      GA_Width, w,
			      GA_Height, h,
			      GA_TopBorder, TRUE,
			      GA_Immediate, TRUE,
			      GA_RelVerify, TRUE,
			      BUTTON_PushButton, TRUE,
			      GA_ID, GID_PP,
			      BUTTON_Glyph, impp,
			      BUTTON_Array, 3,
			      TAG_DONE);

    /* Create the fast forward button */
    x += w;
    gads[GID_FFORWARD] = NewObject (NULL, buttonClassName,
				    GA_Previous, gads[GID_FFORWARD - 1],
				    GA_Top, 1,
				    GA_Left, x,
				    GA_Width, w,
				    GA_Height, h,
				    GA_TopBorder, TRUE,
				    GA_Immediate, TRUE,
				    GA_RelVerify, TRUE,
				    GA_ID, GID_FFORWARD,
				    BUTTON_Glyph, &im4,
				    TAG_DONE);

    /* Create the stop button */
    x += w;
    gads[GID_STOP] = NewObject (NULL, buttonClassName,
				GA_Previous, gads[GID_STOP - 1],
				GA_Top, 1,
				GA_Left, x,
				GA_Width, w,
				GA_Height, h,
				GA_TopBorder, TRUE,
				GA_Immediate, TRUE,
				GA_RelVerify, TRUE,
				GA_Selected, TRUE,
				BUTTON_PushButton, TRUE,
				GA_ID, GID_STOP,
				BUTTON_Glyph, &im5,
				TAG_DONE);

    /* Add and refresh the gadget list */
    AddGList (win, gads[0], 0, -1, NULL);
    RefreshGList (gads[0], win, NULL, -1);
}

/*****************************************************************************/

static void selectgad (struct Window *win, UWORD sel, BOOL leave)
{
    register int i;

    for (i = 0; i < NUMGADS; i++)
    {
	if ((i != sel) || ((i == sel) && leave))
	{
	    if (!((i == sel) && (gads[i]->Flags & GFLG_SELECTED)))
		SetGadgetAttrs (gads[i], win, NULL, GA_Selected, (i == sel), TAG_DONE);
	}
    }
}

/*****************************************************************************/

static void deletegads (struct Window *win)
{
    register int i;

    for (i = 0; i < NUMGADS; i++)
    {
	if (gads[i])
	{
	    RemoveGList (win, gads[i], 1);
	    DisposeObject (gads[i]);
	}
    }
}

/*****************************************************************************/

void main (int argc, char **argv)
{
    struct IntuiMessage *imsg;
    struct ClassLibrary *gLib;
    struct Screen *scr;
    struct Window *win;
    struct Gadget *g;
    BOOL going = TRUE;
    ULONG sigr;

    if (IntuitionBase = OpenLibrary ("intuition.library", 37))
    {
	scr = ((struct IntuitionBase *) IntuitionBase)->FirstScreen;

	if (gLib = openclass ("gadgets/button.gadget", 37))
	{
	    if (win = OpenWindowTags (NULL,
				      WA_InnerWidth,	202,
				      WA_Height,	scr->BarHeight + 1,
				      WA_IDCMP,		IDCMP_FLAGS,
				      WA_DragBar,	TRUE,
				      WA_DepthGadget,	TRUE,
				      WA_CloseGadget,	TRUE,
				      WA_SimpleRefresh,	TRUE,
				      WA_NoCareRefresh,	TRUE,
				      WA_Activate,	TRUE,
				      WA_CustomScreen,	scr,
				      TAG_DONE))
	    {
		creategads (win);
		{
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

				case IDCMP_GADGETDOWN:
				    g = (struct Gadget *) imsg->IAddress;
				    switch (g->GadgetID)
				    {
					case GID_REWIND:	/* Rew */
					case GID_FFORWARD:	/* FF */
					    Printf ("v id=%ld code=%04lx : %04lx\n",
						    (ULONG) g->GadgetID, (ULONG) imsg->Code, (ULONG) g->SpecialInfo);
					    break;
				    }
				    break;

				case IDCMP_GADGETUP:
				    Printf ("^ id=%ld code=%04lx : %04lx\n",
					    (ULONG) g->GadgetID, (ULONG) imsg->Code, (ULONG) g->SpecialInfo);
				    switch (g->GadgetID)
				    {
					case GID_PP:	/* Play */
					case GID_STOP:	/* Stop */
					    selectgad (win, g->GadgetID, TRUE);
					    break;
				    }
				    g = NULL;
				    break;
			    }

			    ReplyMsg ((struct Message *) imsg);
			}
		    }
		    deletegads (win);
		}
		CloseWindow (win);
	    }
	    else
		Printf ("couldn't open the window\n");

	    CloseLibrary ((struct Library *) gLib);
	}
	else
	    Printf ("couldn't open classes:gadgets/button.gadget\n");

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
