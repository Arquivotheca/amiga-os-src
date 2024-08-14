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

BOOL OpenLibraries (VOID);
VOID CloseLibraries (VOID);
VOID HandleApplication (struct MsgPort * port);

/* All text used in the application must be defined in a text array.  Then
 * referred to by numeric ID. */
STRPTR def_text[] =
{
    NULL,			/* 0 */
    "Gadget Calculator",	/* 1 */
    "_OK",			/* 2 */
    "_Cancel",			/* 3 */

    "Select File to Open",	/* 4 */
    "P_attern",			/* 5 */
    "_Drawer",			/* 6 */
    "_File",			/* 7 */
    "_Volumes",			/* 8 */
    "_Parent",			/* 9 */
    "_Open",			/* 10 */

    "OkayFunc",			/* 11 */
    "CancelFunc",		/* 12 */
    "PatternFunc",		/* 13 */
    "DrawerFunc",		/* 14 */
    "FileFunc",			/* 15 */
    "VolumesFunc",		/* 16 */
    "ParentFunc",		/* 17 */
    "OpenFunc",			/* 18 */
    "BoxFunc",			/* 19 */
    "ScrollFunc",		/* 20 */
    "DeleteFunc",		/* 21 */
    "SelectFunc",		/* 22 */

    " Buttons ",		/* 23 */
    " Text ",			/* 24 */
    NULL
};

/* This is an alternate window environment */
struct TagItem parmtags[] =
{
    {GTST_MaxChars, 512L},
    {TAG_DONE,},
};

struct TagItem frwintags[] =
{
    {WA_SizeGadget, TRUE},
    {WA_DragBar, TRUE},
    {WA_DepthGadget, TRUE},
    {WA_CloseGadget, TRUE},
    {WA_Activate, TRUE},
    {WA_SimpleRefresh, TRUE},
    {TAG_DONE,},
};

#if 1
struct Object frobjs[] =
{
    {&frobjs[1], 0, 0, OBJ_Window, NULL, NULL, NULL, "FILEREQ", 0L,
     {0, 0, 0, 0}, },

 /* Row of buttons */
    {&frobjs[2], 2, 0, OBJ_HGroup, NULL, APSH_OBJF_BORDER, NULL, "BUTTONS", 23L,
     {  8, -4, -16, 0},},
    {&frobjs[3], 2, 0, OBJ_Button, 18, APSH_OBJF_CLOSEWINDOW, 'o', "OPEN", 10L,},
    {&frobjs[4], 2, 1, OBJ_Button, 16, NULL, 'v', "VOLUMES", 8L,},
    {&frobjs[5], 2, 2, OBJ_Button, 17, NULL, 'p', "PARENT", 9L,},
    {&frobjs[6], 2, 0, OBJ_Button, 12, APSH_OBJF_CLOSEWINDOW, 'c', "CANCEL", 3L,},

 /* Column of text gadgets */
    {&frobjs[7], 3, 1, OBJ_VGroup, NULL, APSH_OBJF_BORDER, NULL, "TEXT", 24L,
     { 8,  -55, -16, 44},},
    {&frobjs[8], 3, 0, OBJ_String, 15, APSH_OBJF_ACTIVATE, 'f', "FILE", 7L,
     {83, 0, -99, 0}, parmtags,},
    {&frobjs[9], 3, 1, OBJ_String, 14, NULL, 'd', "DRAWER", 6L,
     {83, 0, -99, 0}, parmtags,},
    {&frobjs[10], 3, 2, OBJ_String, 13, NULL, 'a', "PATTERN", 5L,
     {83, 0, -99, 0}, parmtags,},

 /* Doesn't belong in any particular group */
    {&frobjs[11], 4,1, OBJ_Group, NULL, NULL, NULL, "FDISPG", 0L,
     {  8,  4, 310, 125},},
    {NULL, 4,0, OBJ_Listview, 22L, NULL, NULL, "FILEDISP", NULL,
     {  8,  4, -16, -75}, }
};
#else
struct Object frobjs[] =
{
    {&frobjs[ 1], 0,0, OBJ_Window, NULL, NULL, NULL, "FILEREQ", 4L,
     {1, 1, 100, 125}, wintags,},

 /* Row of buttons */
    {&frobjs[ 2], 1,0, OBJ_HGroup, NULL, NULL, NULL, "BUTTONS", 0L,
     {  8, -4, -16, 14},},
    {&frobjs[ 3], 1,0, OBJ_Button, 18, APSH_OBJF_CLOSEWINDOW, 'o', "OKAY", 10L,},
    {&frobjs[ 4], 1,1, OBJ_Button, 16, NULL, 'v', "VOLUMES", 8L,},
    {&frobjs[ 5], 1,2, OBJ_Button, 17, NULL, 'p', "PARENT", 9L,},
    {&frobjs[ 6], 1,0, OBJ_Button, 12, APSH_OBJF_CLOSEWINDOW, 'c', "CANCEL", 3L,},

 /* Column of text gadgets */
    {&frobjs[ 7], 2,1, OBJ_VGroup, NULL, NULL, NULL, "TEXT", 0L,
     { 8, -74,  -16, 60},},
    {&frobjs[ 8], 2,2, OBJ_String, 13, NULL, 'a', "PATTERN", 5L,
     {83, 0, -100, 0}, parmtags,},
    {&frobjs[ 9], 2,1, OBJ_String, 14, NULL, 'd', "DRAWER", 6L,
     {83, 0, -100, 0}, parmtags,},
    {&frobjs[10], 2,0, OBJ_String, 15, APSH_OBJF_ACTIVATE, 'f', "FILE", 7L,
     {83, 0, -100, 0}, parmtags,},

 /* File display group */
    {NULL,        3,0, OBJ_Listview, 20L, NULL, NULL, "FILEDISP", NULL,
     {8, 4, -16, -82},},
};
#endif

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
	    WORD height = frobjs[0].o_Outer.Height;
	    WORD width = frobjs[0].o_Outer.Width;
	    struct Objects *obj_array = frobjs;
	    struct ObjectInfo *oi;
	    struct Window *win;

	    /* Use the fake file requester objects */
	    obj_array = frobjs;

	    if (argc >= 3)
	    {
		width = atoi (argv[1]);
		frobjs[0].o_Outer.Width = width;

		height = atoi (argv[2]);
		frobjs[0].o_Outer.Height = height;
	    }

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
	BOOL new = FALSE;

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
			/* Pass the following command to your command parser */
			printf ("%s\n", oi->oi_TextTable[tidata]);
		    }

		    break;

		case GADGETUP:
		    /* See who went up */
		    on = (struct ObjectNode *) gad->UserData;

		    /* See if we have command to execute */
		    if (tidata = on->on_Funcs[EG_RELEASE])
		    {
			/* Pass the following command to your command parser */
			printf ("%s\n", oi->oi_TextTable[tidata]);
		    }

		    /* See if this is a close-the-window gadget */
		    if (on->on_Object.o_Flags & APSH_OBJF_CLOSEWINDOW)
		    {
			going = FALSE;
		    }

		    /*
		     * See if we got a change field command.  Note that other
		     * gadgets may return this value as meaning something else,
		     * but the GetAttr will filter those out.
		     */
		    else if (msg->Code == 9)
		    {
			struct Gadget *next = NULL;
			LONG tag;

			/* Default to next field */
			tag = CGTA_NextField;

			/* If shift was pressed, then previous */
			if (msg->Qualifier & SHIFTED)
			{
			    tag = CGTA_PrevField;
			}

			/* Ask the gadget who's next */
			GetAttr (tag, gad, &next);

			/* See if there is a next... */
			if (next)
			{
			    /* Activate it */
			    ActivateGadget (next, win, NULL);

			    /* We can remember who to activate */
			    oi->oi_Active = next;
			}
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
