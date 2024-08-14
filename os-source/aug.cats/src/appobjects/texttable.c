/** aotest.c
  * Copyright (C) 1990 Commodore-Amiga, Inc.
  * written by David N. Junod
  *
  * appobjects test
  *
  */

#include <exec/types.h>
#include <libraries/appobjects.h>
#include <workbench/workbench.h>
#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/appobjects_protos.h>
#include <clib/utility_protos.h>
#include <clib/icon_protos.h>
#include <pragmas/exec.h>
#include <pragmas/intuition.h>
#include <pragmas/gadtools.h>
#include <pragmas/appobjects.h>
#include <pragmas/utility.h>
#include <pragmas/icon.h>
#include <string.h>
#include <stdio.h>

/* All text used in the application must be defined in a text array.  Then
 * referred to by numeric ID. */
STRPTR app_deftext[] =
{
    "",
    "Text Table",
    "_Okay",
    "_Add",
    "_Delete",
    "_Cancel",
    "_Help",

    "_Local",
    "_Global",
    NULL
};

struct TagItem TextTable_tags[] =
{
    {WA_SizeGadget, 1},
    {WA_MinWidth, 200},
    {WA_MinHeight, 100},
    {TAG_DONE,}
};

struct TagItem Entry_tags[] =
{
    {STRINGA_MaxChars, 256},
    {TAG_DONE,}
};

struct Object TextTable[] =
{
    {&TextTable[1], 0, 0, 71, 7, APSH_OBJF_NOADJUST, NULL, "TextTable", 1L,
     {0, 11, 323, 150}, TextTable_tags, },

    {&TextTable[2], 0, 0, 34, 8, APSH_OBJF_ACTIVATE, NULL, "Entry", NULL,
     {80, -58, -88, 30}, Entry_tags, },

    {&TextTable[3], 0, 0, 5, 9, NULL, NULL, "TextList", NULL,
     {8, 4, -16, -66}, },

 /* Column of buttons */
    {&TextTable[4], 0, 0, 2, 10, NULL, 'A', "Add", 3L,
     {8, -58, 70, 16}, },
    {&TextTable[5], 0, 0, 2, 11, NULL, 'D', "Delete", 4L,
     {8, -42, 70, 16}, },

 /* Row of buttons */
    {&TextTable[6], 2, 0, OBJ_HGroup, NULL, NULL, NULL, "Buttons", 0L,
     {  8, -8, -16, 0},},
    {&TextTable[7], 2, 0, OBJ_Button, 12, APSH_OBJF_CLOSEWINDOW, NULL, "Open", 2L,},
    {&TextTable[8], 2, 1, OBJ_Button, 13, NULL, NULL, "Help", 6L,},
    {&TextTable[9], 2, 0, OBJ_Button, 14, APSH_OBJF_CLOSEWINDOW, NULL, "Cancel", 5L,},

    {&TextTable[10],3, 0, OBJ_MGroup,},
    {&TextTable[11],3, 0, OBJ_Button, 1, NULL, NULL, "Local", 7L,
     { 8, -26, 70, 16}, },
    {NULL,          3, 0, OBJ_Button, 2, NULL, NULL, "Global", 8L,
     {78, -26, 70, 16}, },
};

#define	obj_array	TextTable

BOOL OpenLibraries (VOID);
VOID CloseLibraries (VOID);
VOID HandleApplication (struct MsgPort * port);

struct Library *IconBase, *IntuitionBase, *GfxBase;
struct Library *UtilityBase, *GadToolsBase, *AppObjectsBase;

#define	WINFLAGS (WINDOWCLOSE | WINDOWDRAG | WINDOWDEPTH \
		  | REPORTMOUSE | ACTIVATE | SMART_REFRESH)

struct NewWindow NewWindow =
{
    0, 0, 0, 0,			/* Window placement rectangle */
    0, 1,			/* DetailPen, BlockPen */
    NULL,			/* IDCMP flags */
    WINFLAGS,			/* Window flags */
    NULL,			/* First gadget */
    NULL,			/* Menu checkmark */
    NULL,			/* Window title */
    NULL,			/* Screen pointer */
    NULL,			/* Custom bitmap */
    0, 0,			/* Minimum width & height */
    0, 0,			/* Maximum width & height */
    WBENCHSCREEN,		/* Screen type */
};

struct Window *win;

struct ObjectInfo *NewObjList (ULONG tags,...)
{
    return (NewObjListA ((struct TagItem *) & tags));
}

VOID main (int argc, char **argv)
{
    if (OpenLibraries ())
    {
	struct Screen *screen;

	if (screen = LockPubScreen (NULL))
	{
	    struct ObjectInfo *oi;
	    struct Window *win;

	    /* Using a stack based stub */
	    if (oi = NewObjList (
				    APSH_Screen, screen,
				    APSH_TextAttr, screen->Font,
				    APSH_DefText, app_deftext,
				    APSH_NewWindow, &NewWindow,
				    APSH_Objects, obj_array,
				    TAG_DONE))
	    {
		/* Try to open the window */
		if (win = OpenWindowTagList (&NewWindow, oi->oi_WindowAttrs))
		{
		    /* Attach the ObjectInfo to the window */
		    win->UserData = (APTR) oi;

		    /* Refresh the AppObjects gadgets */
		    GT_RefreshWindow (win, NULL);

		    /* Handle the events */
		    HandleApplication (win->UserPort);

		    RemoveGList (win, oi->oi_GList, -1);
		    CloseWindow (win);
		}

		/* Free the object information */
		DisposeObjList (oi);
	    }

	    UnlockPubScreen (NULL, screen);
	}

	/* Close the libraries */
	CloseLibraries ();
    }
}

VOID HandleApplication (struct MsgPort * port)
{
    BOOL going = TRUE;

    /* Do application stuff... */
    while (going)
    {
	struct IntuiMessage *msg;
	struct ObjectNode *on, *con = NULL;
	struct ObjectInfo *oi;
	struct Gadget *gad;
	struct Window *win;
	LONG tidata;

	/* Wait for something to happen */
	Wait (1 << port->mp_SigBit);

	/* Handle the messages */
	while (msg = (struct IntuiMessage *) GT_GetIMsg (port))
	{
	    /* Cache some of the fields */
	    win = msg->IDCMPWindow;
	    oi = (struct ObjectInfo *) win->UserData;
	    gad = (struct Gadget *) msg->IAddress;

	    /* Handle the event */
	    switch (msg->Class)
	    {
		case SIZEVERIFY:
		    RemoveObjList (oi, win, NULL);
		    break;

		/* Don't really care about this one */
		case NEWSIZE:
		    {
			struct IBox *nb = (struct IBox *)&(win->LeftEdge);
			struct IBox *ob = &(oi->oi_WinBox);

			/* Only do this if the window DIDN'T change */
			if ((nb->Width == ob->Width) &&
			    (nb->Height == ob->Height))
			{
			    if (oi->oi_Flags & AOIF_REMOVED)
			    {
				/* Remove the list */
				AddGList (win, oi->oi_GList, -1L, -1L, NULL);

				/* Don't remove it again */
				oi->oi_Flags &= ~AOIF_REMOVED;
			    }
			}
		    }
		    break;

		    /* refresh the window */
		case REFRESHWINDOW:
		    /* Do the GadTools refresh */
		    GT_BeginRefresh (win);
		    GT_EndRefresh (win, TRUE);

		    /* Refresh the object list (relative) */
		    RefreshObjList (oi, win, NULL);
		    break;

		case CLOSEWINDOW:
		    going = FALSE;
		    break;

		case RAWKEY:
		    /* Clear the command id */
		    tidata = 0L;

		    /* See if keystroke belongs to gadget */
		    if (on = LookupKeystroke (oi, msg))
		    {
			/* Get the command assigned to the current state */
			if ((tidata = on->on_Current) &&
			    (on->on_Object.o_Flags & APSH_OBJF_CLOSEWINDOW))
			{
			    going = FALSE;
			}
		    }
		    else
		    {
			/* Look up keystroke in your own key-command table */
		    }

		    /* See if we have a command to execute */
		    if (tidata)
		    {
			printf ("Function ID %ld\n", tidata);
		    }

		    break;

		case GADGETDOWN:
		    /* See who went up */
		    con = on = (struct ObjectNode *) gad->UserData;

		    /* See if we have command to execute */
		    if (tidata = on->on_Funcs[EG_DOWNPRESS])
		    {
			printf ("Downpress function ID %ld\n", tidata);
		    }
		    break;


		case MOUSEMOVE:
		case INTUITICKS:
		    /* See if we have command to execute */
		    if (con && (tidata = con->on_Funcs[EG_HOLD]))
		    {
			printf ("hold function ID %ld\n", tidata);
		    }
		    break;


		case GADGETUP:
		    /* See who went up */
		    on = (struct ObjectNode *) gad->UserData;

		    /* See if we have command to execute */
		    if (tidata = on->on_Funcs[EG_RELEASE])
		    {
			printf ("Release function ID %ld, with %d\n",
				tidata, msg->Code);
		    }

		    /* See if this is a close-the-window gadget */
		    if (on->on_Object.o_Flags & APSH_OBJF_CLOSEWINDOW)
		    {
			going = FALSE;
		    }

		    /* Clear the current gadget */
		    con = NULL;
		    break;

		    /* Window went active */
		case ACTIVEWINDOW:
		    /* See if we have an activate candidate */
		    if (oi->oi_Active)
		    {
			/* Activate the gadget */
			ActivateGadget (oi->oi_Active, win, NULL);
		    }
		    break;

		    /* Window went inactive */
		case INACTIVEWINDOW:
		    /* Always do this when you change the input focus */
		    AbortKeystroke (oi, win);
		    break;
	    }

	    /* Reply to the message */
	    GT_ReplyIMsg (msg);
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
		if (GadToolsBase = OpenLibrary ("gadtools.library", 36))
		{
		    if (AppObjectsBase = OpenLibrary ("appobjects.library", 36))
		    {
			if (IconBase = OpenLibrary ("icon.library", 36))
			{
			    return (TRUE);
			}
			else
			    printf ("Couldn't open icon.library\n");

			CloseLibrary (AppObjectsBase);
			AppObjectsBase = NULL;
		    }
		    else
			printf ("Couldn't open appobjects.library\n");

		    CloseLibrary (GadToolsBase);
		    GadToolsBase = NULL;
		}
		else
		    printf ("Couldn't open gadtools.library\n");

		CloseLibrary (UtilityBase);
		UtilityBase = NULL;
	    }
	    else
		printf ("Couldn't open utility.library\n");

	    CloseLibrary (GfxBase);
	    GfxBase = NULL;
	}
	else
	    printf ("couldn't open graphics.library version 36\n");

	CloseLibrary (IntuitionBase);
	IntuitionBase = NULL;
    }
    else
	printf ("couldn't open intuition.library version 36\n");

    return (FALSE);
}

VOID CloseLibraries (VOID)
{

    if (IconBase)
	CloseLibrary (IconBase);
    if (AppObjectsBase)
	CloseLibrary (AppObjectsBase);
    if (AppObjectsBase)
	CloseLibrary (AppObjectsBase);
    if (UtilityBase)
	CloseLibrary (UtilityBase);
    if (IntuitionBase)
	CloseLibrary (IntuitionBase);
    if (GfxBase)
	CloseLibrary (GfxBase);
}
