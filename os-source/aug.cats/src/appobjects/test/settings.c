/** aotest.c
  * Copyright (C) 1990 Commodore-Amiga, Inc.
  * written by David N. Junod
  *
  * appobjects test
  *
  */

#include <exec/types.h>
#include <libraries/appobjects.h>
#include <graphics/gfx.h>
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

BOOL OpenLibraries (VOID);
VOID CloseLibraries (VOID);
VOID HandleApplication (struct MsgPort * port);

/* All text used in the application must be defined in a text array.  Then
 * referred to by numeric ID. */
STRPTR app_deftext[] =
{
    "",
    "AppBuilder Settings",
    "_Okay",
    "_Cancel",
    "App_Shell Client",
    "_Load/Save Icons",
    "_Make Backups",
    "Auto _Save",
    "_Icon Tool:",
    "Backup _Template:",
    "Max _Version #:",
    "Auto Save _Delay:",
    "_Help",

    " AppShell ",
    " Backup ",
    " Icons ",
    " Auto Save ",
    NULL
};

#define	def_text	app_deftext

struct TagItem Border_Tags[] =
{
    {APSH_GTFlags, PLACETEXT_ABOVEC},
    {TAG_DONE,},
};

struct TagItem PlaceText_tags[] =
{
    {APSH_GTFlags, PLACETEXT_RIGHT},
    {TAG_DONE,}
};

struct Object ABSet[] =
{
    {&ABSet[1], 0, 0, OBJ_Window, 0, NULL, NULL, "ABSet", 1L,
     {5, 23, 5, 5}, },

    {&ABSet[2], 0, 0, OBJ_DblBevelIn, 0, NULL, NULL, "Border1", 13L,
     {8, 8, 264, 23}, Border_Tags, },
    {&ABSet[3], 0, 0, OBJ_Checkbox, 0, NULL, 'S', "AppShellClient", 4L,
     {20, 14, 26, 11}, PlaceText_tags, },


    {&ABSet[4], 0, 0, OBJ_DblBevelIn, 0, NULL, NULL, "Border2", 14L,
     {8, 37, 264, 55}, Border_Tags, },
    {&ABSet[5], 0, 0, OBJ_Checkbox, 0, NULL, 'M', "MakeBackups", 6L,
     {20, 43, 26, 11}, PlaceText_tags, },
    {&ABSet[6], 0, 0, OBJ_String, 0, NULL, 'T', "BackupTemplate", 9L,
     {152, 56, 108, 14}, },
    {&ABSet[7], 0, 0, OBJ_Integer, 0, NULL, 'V', "MaxVersion", 10L,
     {152, 72, 40, 14}, },


    {&ABSet[8], 0, 0, OBJ_DblBevelIn, 0, NULL, NULL, "Border3", 15L,
     {280, 8, 264, 39}, Border_Tags, },
    {&ABSet[9], 0, 0, OBJ_Checkbox, 0, NULL, 'L', "LoadSaveIcons", 5L,
     {292, 14, 26, 11}, PlaceText_tags, },
    {&ABSet[10], 0, 0, OBJ_String, 0, NULL, 'I', "IconTool", 8L,
     {375, 27, 157, 14}, },


    {&ABSet[11],   0, 0, OBJ_DblBevelIn, 0, NULL, NULL, "Border4", 16L,
     {280, 53, 264, 39}, Border_Tags, },
    {&ABSet[12], 0, 0, OBJ_Checkbox, 0, NULL, 'S', "AutoSave", 7L,
     {292, 59, 26, 11}, PlaceText_tags, },
    {&ABSet[13], 0, 0, OBJ_Integer, 0, NULL, 'D', "AutoSaveDelay", 11L,
     {424, 72, 40, 14}, },


    {&ABSet[14], 2, 0, OBJ_HGroup, NULL, NULL, NULL, "Buttons", 0L,
     { 8, 96, -16, 16},},
    {&ABSet[15], 2, 0, OBJ_Button, 100035, APSH_OBJF_CLOSEWINDOW, 'O', "Okay", 2L,},
    {&ABSet[16], 2, 0, OBJ_Button, 100057, NULL, 'H', "Help", 12L,},
    {NULL,       2, 0, OBJ_Button, 100034, APSH_OBJF_CLOSEWINDOW, 'C', "Cancel", 3L,},
};

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
	    struct Objects *obj_array;
	    struct ObjectInfo *oi;
	    struct Window *win;

	    /* Use the fake file requester objects */
	    obj_array = ABSet;

	    /* Using a stack based stub */
	    if (oi = NewObjList (
				    APSH_Screen, screen,
				    APSH_TextAttr, screen->Font,
				    APSH_DefText, def_text,
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
	struct ObjectNode *on;
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
#if 0
			/* Pass the following command to your command parser */
			printf ("%s\n", oi->oi_TextTable[tidata]);
#endif
		    }

		    break;

		case GADGETUP:
		    /* See who went up */
		    on = (struct ObjectNode *) gad->UserData;

		    /* See if we have command to execute */
		    if (tidata = on->on_Funcs[EG_RELEASE])
		    {
#if 0
			/* Pass the following command to your command parser */
			printf ("%s\n", oi->oi_TextTable[tidata]);
#endif
		    }

		    /* See if this is a close-the-window gadget */
		    if (on->on_Object.o_Flags & APSH_OBJF_CLOSEWINDOW)
		    {
			going = FALSE;
		    }
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
