/* lvmdemo.c
 * Copyright (C) 1991 Commodore-Amiga, Inc.
 * All Rights Reserved Worldwide
 * Written by David N. Junod
 *
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/icclass.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <libraries/gadtools.h>
#include <libraries/appobjects.h>
#include <utility/tagitem.h>
#include <workbench/workbench.h>
#include <string.h>

#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/icon_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/diskfont_pragmas.h>
#include <pragmas/icon_pragmas.h>

/* Usually allocate one large block of memory for a group of items and then
 * divy out to the appropriate pointers.  Use with caution---must be
 * consistent with field types! */
#ifndef MEMORY_FOLLOWING
#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))
#endif

extern struct Library *SysBase, *DOSBase;
struct Library *IntuitionBase, *GfxBase, *UtilityBase;
struct Library *DiskfontBase, *IconBase;
struct Library *AppObjectsBase;

struct ExtNewWindow NewWindow =
{
    0, 0, 640, 100,		/* Window placement rectangle */
    0, 1,			/* DetailPen, BlockPen */

    CLOSEWINDOW |		/* IDCMP flags */
    GADGETDOWN | GADGETUP |
    ACTIVEWINDOW | INACTIVEWINDOW |
    VANILLAKEY |
    IDCMPUPDATE,

    NOCAREREFRESH |		/* Window flags */
    SMART_REFRESH | WINDOWDRAG |
    WINDOWDEPTH | WINDOWCLOSE |
    WINDOWSIZING,

    NULL,			/* First gadget */
    NULL,			/* Menu checkmark */
    "DNJ ScrollView",		/* Window title */
    NULL,			/* Screen pointer */
    NULL,			/* Custom bitmap */
    200, 60,			/* Minimum width & height */
    1024, 1024,			/* Maximum width & height */
    WBENCHSCREEN,		/* Screen type */
    NULL,
};

BOOL OpenLibraries (VOID)
{

    if (IntuitionBase = OpenLibrary ("intuition.library", 36))
    {
	if (GfxBase = OpenLibrary ("graphics.library", 36))
	{
	    if (UtilityBase = OpenLibrary ("utility.library", 36))
	    {
		if (DiskfontBase = OpenLibrary ("diskfont.library", 36))
		{
		    if (AppObjectsBase = OpenLibrary ("appobjects.library", 36))
		    {
			if (IconBase = OpenLibrary ("icon.library", 33))
			{
			    return (TRUE);
			}

			CloseLibrary (IconBase);
		    }

		    CloseLibrary (AppObjectsBase);
		}

		CloseLibrary (DiskfontBase);
	    }

	    CloseLibrary (GfxBase);
	}

	CloseLibrary (IntuitionBase);
    }

    return (FALSE);
}

VOID CloseLibraries (VOID)
{

    if (IntuitionBase)
    {
	CloseLibrary (AppObjectsBase);
	CloseLibrary (DiskfontBase);
	CloseLibrary (UtilityBase);
	CloseLibrary (GfxBase);
	CloseLibrary (IntuitionBase);
	CloseLibrary (IconBase);
    }
}

BOOL GetScreenSize (struct Screen * scr, struct Rectangle * r)
{
    extern struct Library *IntuitionBase;
    BOOL retval = FALSE;

    if (scr)
    {
	ULONG mode;

	/* Get the mode id of the screen */
	if ((IntuitionBase->lib_Version >= 36) &&
	    (mode = GetVPModeID (&(scr->ViewPort))) != INVALID_ID)
	{
	    /* Fill in the window rectangle */
	    QueryOverscan (mode, r, OSCAN_TEXT);

	    /* Adjust */
	    r->MaxX++;
	    r->MaxY++;
	}
	else
	{
	    /* Usually 1.3 */
	    r->MinX = r->MinY = 0;
	    r->MaxX = scr->Width;
	    r->MaxY = scr->Height;
	}

	/* Positive return */
	retval = TRUE;
    }

    return retval;
}

main (int argc, char **argv)
{
    struct DrawInfo *di;
    struct IntuiMessage *imsg;
    struct Gadget *agad = NULL;
    struct Gadget *gad0;
    struct Screen *scr;
    struct Window *win;
    struct Rectangle rect;
    BOOL going = TRUE;
    LONG topborder;
    LONG size = SYSISIZE_HIRES;
    struct TagItem *tags;
    ULONG gid;
    LONG vtop = 0;
    LONG htop = 0;
    UBYTE buff[61];
    struct IBox *b;
    struct LVSpecialInfo *si;
    struct DiskObject *dob;
    struct Image *im = NULL;
    struct BitMap BMap;
    SHORT i, planes;
    LONG image_data;
    LONG w, h;

    /* Open the required libraries */
    if (OpenLibraries ())
    {
	if (argc > 1)
	{
	    dob = GetDiskObject (argv[1]);
	}
	else
	{
	    dob = GetDiskObject (":disk");
	}

	/* Get a pointer to the default screen */
	if (dob &&
	    (scr = LockPubScreen (NULL)))
	{
	    im = (struct Image *) dob->do_Gadget.GadgetRender;

	    printf ("Width %d, Height %d, Depth %d\n",
		    im->Width, im->Height, im->Depth);

	    /* Map the image to a BitMap */
	    InitBitMap (&BMap, (BYTE) im->Depth, (SHORT) im->Width, (SHORT) im->Height);
	    planes = RASSIZE (im->Width, im->Height);
	    image_data = (LONG) im->ImageData;
	    for (i = 0; i < im->Depth; ++i)
	    {
		BMap.Planes[i] = (PLANEPTR) (image_data + i * planes);
	    }

	    GetScreenSize (scr, &rect);

	    if (di = GetScreenDrawInfo (scr))
	    {
		if (rect.MaxY < 400)
		{
		    size = SYSISIZE_MEDRES;
		}

		/* Calculate the top border */
		topborder = (LONG) (scr->WBorTop + (scr->Font->ta_YSize + 1));

		/* The entire listview */
		if (gad0 = (struct Gadget *)
		    NewObject (NULL, "listviewgclass",
				/* Basic gadget information */
			       GA_Immediate, TRUE,
			       GA_RelVerify, TRUE,
			       GA_Left, scr->WBorLeft + 8,
			       GA_Top, topborder + 4 + scr->Font->ta_YSize + 4,
			       GA_RelWidth, -36,
			       GA_RelHeight, -(topborder + 4 + scr->Font->ta_YSize + 4 + scr->WBorBottom + 4),
			       GA_ID, 1,
			       GA_DrawInfo, di,

				/* We want to hear changes on our IDCMP port. */
			       ICA_TARGET, ICTARGET_IDCMP,

				/* Screen size */
			       SYSIA_Size, size,

				/* We are read-only */
			       AOLV_ReadOnly, TRUE,

				/* What scrollers do we need */
			       AOLV_Freedom, (FREEHORIZ | FREEVERT),

				/* Any borders */
			       AOLV_Borderless, FALSE,

				/* Vertical size info */
			       AOLV_TopVert, vtop,
			       AOLV_TotalVert, (LONG)im->Height,

				/* Height of each row */
			       AOLV_UnitHeight, 1,

				/* Horizontal size info */
			       AOLV_TopHoriz, htop,
			       AOLV_TotalHoriz, (LONG)im->Width,

				/* Width of each horizontal unit */
			       AOLV_UnitWidth, 1,

			       TAG_DONE))
		{
		    /* Cache the pointer */
		    si = (struct LVSpecialInfo *) gad0->SpecialInfo;
		    b = &(si->si_View);

		    /* Initialize the NewWindow structure */
		    NewWindow.FirstGadget = gad0;

		    /* Open the window */
		    if (win = OpenWindow (&NewWindow))
		    {
			SetAPen (win->RPort, di->dri_Pens[TEXTPEN]);

			printf ("View X=%d, Y=%d, W=%d, H=%d\n",
				b->Left, b->Top,
				b->Width, b->Height);

			/* Set the gadget attributes */
			SetGadgetAttrs (gad0, win, NULL,

				/* Vertical size info */
			       AOLV_TopVert, vtop,
			       AOLV_TotalVert, (LONG)im->Height,

				/* Height of each row */
			       AOLV_UnitHeight, 1L,

				/* Horizontal size info */
			       AOLV_TopHoriz, htop,
			       AOLV_TotalHoriz, (LONG)im->Width,

				/* Width of each horizontal unit */
			       AOLV_UnitWidth, 1L,

			       TAG_DONE);

			/*
			 * Hang around until the Close gadget has been hit.
			 */
			while (going)
			{
			    /* Wait for an event */
			    Wait (1L << win->UserPort->mp_SigBit);

			    /* Pull each message and handle it */
			    while (imsg = (struct IntuiMessage *) GetMsg (win->UserPort))
			    {
				/* Process the events */
				switch (imsg->Class)
				{
				    case VANILLAKEY:
					if (imsg->Code == 'd')
					{
					    SetGadgetAttrs (gad0, win, NULL,
							    GA_Disabled, TRUE,
							    TAG_DONE);
					}
					else if (imsg->Code == 'D')
					{
					    SetGadgetAttrs (gad0, win, NULL,
							    GA_Disabled, FALSE,
							    TAG_DONE);
					}
					break;

				    case GADGETDOWN:
					agad = (struct Gadget *) imsg->IAddress;
					printf ("Gadget down ID %ld\n", agad->GadgetID);
					break;

				    case IDCMPUPDATE:
					tags = (struct TagItem *) imsg->IAddress;
					gid = GetTagData (GA_ID, 0, tags);
					vtop = si->si_TopVert;
					htop = si->si_TopHoriz;

					sprintf (buff, "ID %d V%03ld, H%03ld", gid, vtop, htop);
					Move (win->RPort, b->Left + 8, topborder + 4 + scr->Font->ta_YSize);
					Text (win->RPort, buff, strlen (buff));
					w = (LONG) MIN (b->Width, im->Width);
					h = (LONG) MIN (b->Height, im->Height);
					BltBitMapRastPort (&BMap, htop, vtop,
							   win->RPort, (LONG) b->Left, (LONG) b->Top, w, h, 0xC0);
					break;

				    case GADGETUP:
					if (agad == gad0)
					{
					    vtop = si->si_TopVert;
					    htop = si->si_TopHoriz;
					    sprintf (buff, "ID %d V%03ld, H%03ld", gad0->GadgetID, vtop, htop);
					    Move (win->RPort, b->Left + 8, topborder + 4 + scr->Font->ta_YSize);
					    Text (win->RPort, buff, strlen (buff));
					}

					printf ("Gadget up ID %d, Code %d\n", agad->GadgetID, imsg->Code);
					agad = NULL;
					break;

				    case CLOSEWINDOW:
					going = FALSE;
					break;
				}

				/* Reply to the message */
				ReplyMsg ((struct Message *) imsg);
			    }
			}

			/* Remove the gadgets from the list */
			RemoveGList (win, gad0, -1L);

			/* Close the window */
			CloseWindow (win);
		    }

		    /* Free the gadget */
		    DisposeObject (gad0);
		}

		FreeScreenDrawInfo (scr, di);
	    }

	    UnlockPubScreen (NULL, scr);
	}

	if (dob)
	{
	    FreeDiskObject (dob);
	    dob = NULL;
	}

	/* Close our libraries */
	CloseLibraries ();
    }
}
