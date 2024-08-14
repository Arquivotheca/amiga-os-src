/* lvmdemo.c
 * Copyright (C) 1991 Commodore-Amiga, Inc.
 * All Rights Reserved Worldwide
 * Written by David N. Junod
 *
 */

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/memory.h>
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
#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/icon_protos.h>
#include <pragmas/exec.h>
#include <pragmas/intuition.h>
#include <pragmas/graphics.h>
#include <pragmas/utility.h>
#include <pragmas/diskfont.h>
#include <pragmas/icon.h>
#include <string.h>
#include <stddef.h>

struct Record
{
    struct Node r_Node;		/* Embedded node */
    UBYTE r_Desc[30];		/* Description */
    LONG r_Lines;		/* Lines */
};

/* Usually allocate one large block of memory for a group of items and then
 * divy out to the appropriate pointers.  Use with caution---must be
 * consistent with field types! */
#ifndef MEMORY_FOLLOWING
#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))
#endif

VOID NewList (struct List *);

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
    150, 60,			/* Minimum width & height */
    1024, 1024,			/* Maximum width & height */
    WBENCHSCREEN,		/* Screen type */
    NULL,
};

struct Record *MakeRecord (struct List * list, STRPTR name, STRPTR desc, LONG lines)
{
    struct Record *rec = NULL;

    if (list && name)
    {
	LONG msize;

	/* Compute the size of the memory allocation */
	msize = sizeof (struct Record) + strlen (name) + 1L;

	/* Allocate the Record */
	if (rec = (struct Record *) AllocVec (msize, MEMF_CLEAR))
	{
	    /* Copy the name over */
	    rec->r_Node.ln_Name = MEMORY_FOLLOWING (rec);
	    strcpy (rec->r_Node.ln_Name, name);

	    /* Copy the description over */
	    strcpy (rec->r_Desc, desc);

	    /* Set the number of lines */
	    rec->r_Lines = lines;

	    /* Add the rec to the list */
	    AddTail (list, (struct Node *) rec);
	}
    }

    return (rec);
}

/* All nodes on the list must have been allocated using AllocVec() */
VOID FreeExecList (struct List * list)
{

    /* Make sure we have a list */
    if (list)
    {
	/* Make sure there are entries in the list */
	if (list->lh_TailPred != (struct Node *) list)
	{
	    struct Node *node, *nxtnode;

	    /* Let's start at the very beginning... */
	    node = list->lh_Head;

	    /* Continue while there are still nodes to remove */
	    while (nxtnode = node->ln_Succ)
	    {
		/* Remove the node from the list */
		Remove (node);

		/* Free the node itself */
		FreeVec ((APTR) node);

		/* Go on to the next node */
		node = nxtnode;
	    }
	}
    }
}

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

VOID ShowSelected (struct List * list)
{
    struct Node *node, *nxtnode;

    /* Make sure there are entries in the list */
    if (list->lh_TailPred != (struct Node *) list)
    {
	/* Let's start at the very beginning... */
	node = list->lh_Head;

	/* Continue while there are still nodes to remove */
	while (nxtnode = node->ln_Succ)
	{
	    /* Is this item selected? */
	    if (node->ln_Type & LNF_SELECTED)
	    {
		/* Display it */
		printf ("%s\n", node->ln_Name);
	    }

	    /* Go on to the next node */
	    node = nxtnode;
	}
    }

    /* Get a blank line in there */
    printf ("\n");
}

main (int argc, char **argv)
{
    struct List list;
    struct DrawInfo *di;
    struct IntuiMessage *imsg;
    struct Gadget *agad = NULL;
    struct Gadget *gad0 = NULL;
    struct Image *col1 = NULL;
    struct Image *col2 = NULL;
    struct Image *col3 = NULL;
    struct Screen *scr;
    struct Window *win;
    struct Rectangle rect;
    BOOL going = TRUE;
    LONG topborder;
    LONG size = SYSISIZE_HIRES;
    UBYTE buff[61];
    struct IBox *b;
    struct LVSpecialInfo *si;
    ULONG offset;

    /* Open the required libraries */
    if (OpenLibraries ())
    {
	/* Initialize the list */
	NewList (&list);

	/* Make some nodes */
	MakeRecord (&list, "Keys", "Key bindings", 20);
	MakeRecord (&list, "Menus", "Window menu strips", 32);
	MakeRecord (&list, "Labels", "Text string labels", 10);
	MakeRecord (&list, "Buttons", "User defineable buttons", 5);
	MakeRecord (&list, "Images", "Pretty little pictures", 20);
	MakeRecord (&list, "Sounds", "Short sounds", 20);
	MakeRecord (&list, "Anims", "Interesting cartoons", 20);

	/* Get a pointer to the default screen */
	if (scr = LockPubScreen (NULL))
	{
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

		/* Screen size */
			       SYSIA_Size, size,

		/* Indicate that we want multi-select */
			       AOLV_MultiSelect, (LONG) (argc > 1),

			       AOLV_UnitHeight, scr->Font->ta_YSize + 1,
			       AOLV_List, &list,

			       TAG_DONE))
		{
		    /* First column */
		    col1 = (struct Image *)
		      NewObject (NULL, "columniclass",
				 AOLV_ColumnWidth, 25,
				 TAG_DONE);

		    /* Add the column to the list view */
		    DoMethod ((Object *) gad0, LV_ADDCOLUMN, (LONG) col1, NULL);

		    /* Get the field offset of the description string */
		    offset = offsetof (struct Record, r_Desc);

		    /* Second column */
		    col2 = (struct Image *)
		      NewObject (NULL, "columniclass",
				 AOLV_ColumnWidth, 65,
				 AOLV_FieldOffset, offset,
				 AOLV_FieldType, FT_STRING,
				 TAG_DONE);

		    /* Add the column to the list view */
		    DoMethod ((Object *) gad0, LV_ADDCOLUMN, (LONG) col2, NULL);

		    /* Get the field offset of the description string */
		    offset = offsetof (struct Record, r_Lines);

		    /* Third column */
		    col3 = (struct Image *)
		      NewObject (NULL, "columniclass",
				 AOLV_ColumnWidth, 10,
				 AOLV_Justification, AOLVF_RIGHT,
				 AOLV_FieldOffset, offset,
				 AOLV_FieldType, FT_ULONG,
				 TAG_DONE);

		    /* Add the column to the list view */
		    DoMethod ((Object *) gad0, LV_ADDCOLUMN, (LONG) col3, NULL);

		    /* Cache the pointer */
		    si = (struct LVSpecialInfo *) gad0->SpecialInfo;
		    b = &(si->si_View);

		    /* Initialize the NewWindow structure */
		    NewWindow.FirstGadget = gad0;
		    NewWindow.MinHeight = (scr->Font->ta_YSize * 5) + 8;

		    /* Open the window */
		    if (win = OpenWindow (&NewWindow))
		    {
			/* Prepare for drawing text */
			SetAPen (win->RPort, di->dri_Pens[TEXTPEN]);

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
					break;

				    case GADGETUP:
					if (agad == gad0)
					{
					    sprintf (buff, "%6d", imsg->Code);
					    Move (win->RPort, b->Left + 8, topborder + 4 + scr->Font->ta_YSize);
					    Text (win->RPort, buff, strlen (buff));
					}
					agad = NULL;

					/* Show the selected items */
					ShowSelected (&list);
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

		/* Free the screen information */
		FreeScreenDrawInfo (scr, di);
	    }

	    /* Unlock the public screen */
	    UnlockPubScreen (NULL, scr);
	}

	/* Free our temporary list */
	FreeExecList (&list);

	/* Close our libraries */
	CloseLibraries ();
    }
}
