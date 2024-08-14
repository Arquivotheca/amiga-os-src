/* aowrapper.c
 * AppObjects wrapper
 * Copyright (C) 1990, 1991 Commodore-Amiga, Inc.
 * written by David N. Junod
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

#include "test.c"

BOOL OpenLibraries (VOID);
VOID CloseLibraries (VOID);
VOID HandleApplication (struct MsgPort * port);

struct Library *IconBase, *IntuitionBase, *GfxBase;
struct Library *UtilityBase, *GadToolsBase, *AppObjectsBase;

#define	WINFLAGS (WINDOWCLOSE | WINDOWDRAG | WINDOWDEPTH \
		  | REPORTMOUSE | ACTIVATE | SMART_REFRESH)

struct NewWindow TemplateNW =
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


ULONG OpenAppWindow (struct Screen *scr, struct Objects *objs, STRPTR tt)
{
    struct ObjectInfo *oi;
    struct NewWindow nw;
    struct Window *win;

    nw = *(&TemplateNW);

    /* Using a stack based stub */
    if (oi = NewObjList (APSH_Screen, scr,
			 APSH_TextAttr, scr->Font,
			 APSH_DefText, tt,
			 APSH_NewWindow, &nw,
			 APSH_Objects, objs,
			 TAG_DONE))
    {
	/* Try to open the window */
	if (win = OpenWindowTagList (&nw, oi->oi_WindowAttrs))
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

	    OpenAppWindow (screen, Trace, Trace_deftext);

	    UnlockPubScreen (NULL, screen);
	}

	/* Close the libraries */
	CloseLibraries ();
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
