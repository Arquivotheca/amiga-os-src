/* aghelp.c
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <graphics/text.h>
#include <libraries/amigaguide.h>
#include <libraries/gadtools.h>
#include <utility/hooks.h>
#include <utility/tagitem.h>
#include <string.h>
#include <stdio.h>

#include <clib/alib_protos.h>
#include <clib/amigaguide_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/amigaguide_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>

/*****************************************************************************/

#define	DB(x)	x

/*****************************************************************************/

void kprintf (void *, ...);

/*****************************************************************************/

#define	BASENAME	"ADVAGHELP"
#define	BASENAME_LENGTH	9

/*****************************************************************************/

#define ASM		__asm __saveds
#define REG(x)		register __ ## x

/*****************************************************************************/

struct AppInfo
{
    struct Process		*ai_Process;		/* Our process address */
    struct Screen		*ai_Screen;		/* Screen that our application will open on */
    APTR			 ai_VI;			/* GadTools visual info */
    struct Window		*ai_Window;		/* Window pointer */
    struct Menu			*ai_Menu;		/* Menus */

    /* Gadgets that we want to set attributes on */
    struct Gadget		*ai_GStatus;		/* Status window */

    /* Help related information */
    LONG			 ai_NHID;		/* Unique id */
    ULONG			 ai_NHFlags;		/* Help related flags */
    UBYTE			 ai_NHName[64];		/* Unique name */
    struct Hook			 ai_NHHook;		/* Dynamic node host hook */
    struct AmigaGuideHost	*ai_NH;			/* Dynamic node host */
    AMIGAGUIDECONTEXT		 ai_AmigaGuide;		/* Pointer to the AmigaGuide context */
    struct NewAmigaGuide	 ai_NAG;		/* Used to start AmigaGuide */
    LONG			 ai_Region;		/* Region that the mouse if over */

    /* Control information */
    BOOL			 ai_Done;		/* Done yet? */
};

#define	AGHF_CONTINUOUS	(1L<<0)

/*****************************************************************************/

extern struct Library *SysBase;
extern struct Library *DOSBase;
struct Library *AmigaGuideBase;
struct Library *GadToolsBase;
struct Library *GfxBase;
struct Library *IntuitionBase;
struct Library *UtilityBase;

/*****************************************************************************/

/* Context ID's to be sent to AmigaGuide */
STRPTR context[] =
{
    "MAIN",
    "STATUS",
    "LISTVIEW",
    "ACCEPT",
    "CANCEL",
    "QUIT",
    "NONCONTINUOUS",
    "CONTINUOUS",
    NULL
};

/*****************************************************************************/

#define	MCMD_MAIN		0
#define	MCMD_STATUS		1
#define	MCMD_LISTVIEW		2
#define	MCMD_ACCEPT		3
#define	MCMD_CANCEL		4
#define	MCMD_QUIT		5
#define	MCMD_NONCONTINUOUS	6
#define	MCMD_CONTINUOUS		7

/*****************************************************************************/

/* Simple little prompts to display within the status window */
STRPTR quickhelp[] =
{
    "AGHelp main window.",
    "Quick-Help or status window.",
    "List of absolutely nothing.",
    "Accept changes.",
    "Cancel changes.",
    "",
    "Window sizing gadget.",
    "Window drag bar.",
    "Screen drag bar.",
    "Window depth gadget.",
    "Screen depth gadget.",
    "Window zoom gadget.",
    "",
    "Window close gadget.",
    NULL
};

#define	MAX_REGION	14

/*****************************************************************************/

struct TextAttr topaz8 = {"topaz.font", 8,};

/*****************************************************************************/

#define	IDCMP_FLAGS	IDCMP_RAWKEY | IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_MENUHELP | \
			IDCMP_GADGETUP | IDCMP_MOUSEMOVE | IDCMP_GADGETHELP

/*****************************************************************************/

#define	V(x)	((void *)(x))

struct NewMenu main_menu[] =
{
    {NM_TITLE,	"Project",},
    {NM_ITEM,	"Quit",			"Q", NULL, NULL, V(MCMD_QUIT),},

    {NM_TITLE,	"Settings",},
    {NM_ITEM,	"Continuous Help?",	"",  CHECKIT | MENUTOGGLE, NULL, V(MCMD_NONCONTINUOUS),},
    {NM_END,},
};

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

VOID DisplayError (LONG err)
{
    printf ("%s\n", GetAmigaGuideString (err));
}

/*****************************************************************************/

void HandleEvents (struct AppInfo * ai)
{
    struct Window *win = ai->ai_Window;
    struct AmigaGuideMsg *agm;
    struct IntuiMessage *imsg;
    struct MenuItem *item;
    ULONG sigr = 0L;
    ULONG sigi = 0L;
    ULONG sigb = 0L;
    UWORD selection;
    STRPTR label;
    LONG region;
    LONG qhelp;
    LONG id;

    /* Show that we're not done running the application yet */
    ai->ai_Done = FALSE;

    /* Get our signal bits */
    sigb = AmigaGuideSignal (ai->ai_AmigaGuide);
    sigi = (1L << win->UserPort->mp_SigBit);

    /* Clear the AmigaGuide context */
    SetAmigaGuideContext (ai->ai_AmigaGuide, 0L, NULL);

    /* Continue until done */
    while (!(ai->ai_Done))
    {
	/* Wait for something to happen */
	sigr = Wait (sigb | sigi);

	/* Pull Intuition & GadTools messages */
	while (imsg = GT_GetIMsg (win->UserPort))
	{
	    switch (imsg->Class)
	    {
		case IDCMP_CLOSEWINDOW:
		    ai->ai_Done = TRUE;
		    break;

		case IDCMP_MENUPICK:
		    selection = imsg->Code;
		    while (selection != MENUNULL)
		    {
			item = ItemAddress (win->MenuStrip, selection);

			switch ((LONG)MENU_USERDATA (item))
			{
			    case MCMD_QUIT:
				ai->ai_Done = TRUE;
				break;

			    case MCMD_NONCONTINUOUS:
				if (item->Flags & CHECKED)
				    ai->ai_NHFlags |= AGHF_CONTINUOUS;
				else
				    ai->ai_NHFlags &= ~AGHF_CONTINUOUS;
				break;
			}

			selection = item->NextSelect;
		    }
		    break;

		case IDCMP_MENUHELP:
		    if (item = ItemAddress (win->MenuStrip, imsg->Code))
		    {
			/* Get the context ID */
			id = (LONG) MENU_USERDATA (item);

			/* Adjust for being selected */
			if (item->Flags & CHECKED)
			    id++;

			/* Display the node */
			SendAmigaGuideCmd (ai->ai_AmigaGuide, NULL, AGA_Context, id, TAG_DONE);
		    }

		    break;

		case IDCMP_GADGETUP:
		    switch (((struct Gadget *)imsg->IAddress)->GadgetID)
		    {
			case MCMD_ACCEPT:
			case MCMD_CANCEL:
			    ai->ai_Done = TRUE;
			    break;
		    }
		    break;

		case IDCMP_GADGETHELP:
		    qhelp = region = -1;
		    if (imsg->IAddress == NULL)
		    {
			/* Not over our window */
			DB (printf ("not over our window\n"));
		    }
		    else if (imsg->IAddress == (APTR) win)
		    {
			/* Over our window */
			DB (printf ("over window\n"));
			qhelp = region = 0;
		    }
		    else
		    {

			/*
			 * Detect system gadgets.  Figure out by looking at
			 * system-gadget-type bits in GadgetType field:
			 */
			LONG sysgtype = ((struct Gadget *) imsg->IAddress)->GadgetType & 0xF0;

			/* Set the region */
			qhelp = (sysgtype >> 4) + 5;
			region = HTFC_SYSGADS + sysgtype;

			switch (sysgtype)
			{
			    case GTYP_SIZING:
				DB (printf ("Gadget Help for window sizing gadget\n"));
				break;

			    case GTYP_WDRAGGING:
				DB (printf ("Gadget Help for window drag-bar\n"));
				break;

			    case GTYP_WUPFRONT:
				DB (printf ("Gadget Help for window depth gadget\n"));
				break;

			    case GTYP_WDOWNBACK:
				DB (printf ("Gadget Help for window zoom gadget\n"));
				break;

			    case GTYP_CLOSE:
				DB (printf ("Gadget Help for window close gadget\n"));
				break;

			    case 0:

				/*
				 * In this example, we only have one gadget, so
				 * we know which one it is.  Normally, you'd
				 * have to figure that out here, using the
				 * usual techniques you already use for other
				 * gadget messages.
				 */
				DB (printf ("Gadget id %d, code %d\n",
					((struct Gadget *)imsg->IAddress)->GadgetID, imsg->Code));
				qhelp = region = (LONG) ((struct Gadget *)imsg->IAddress)->GadgetID;
				break;

			    default:
				DB (printf ("Gadget Help on some other system gadget\n"));
				break;
			}
		    }

		    /* Remember the region */
		    ai->ai_Region = region;

		    /* Range check the region */
		    label = NULL;
		    if ((qhelp >= 0) && (qhelp < MAX_REGION))
			label = quickhelp[qhelp];

		    /* Update the quick-help information */
		    GT_SetGadgetAttrs (ai->ai_GStatus, ai->ai_Window, NULL, GTTX_Text, label, TAG_DONE);

		    /* Send the command immediately if we are continuous */
		    if ((ai->ai_Region >= 0) && (ai->ai_NHFlags & AGHF_CONTINUOUS))
		    {
			/* Display the node */
			SendAmigaGuideCmd (ai->ai_AmigaGuide, NULL,
					   AGA_Context, ai->ai_Region,
					   TAG_DONE);
		    }
		    break;

		case IDCMP_RAWKEY:
		    /* Help key pressed */
		    if (imsg->Code == 95)
		    {
			/* Display the node */
			SendAmigaGuideCmd (ai->ai_AmigaGuide, NULL,
					   AGA_Context, ai->ai_Region,
					   TAG_DONE);
		    }
		    break;
	    }

	    /* Reply to the message */
	    GT_ReplyIMsg (imsg);
	}

	/* Process AmigaGuide messages */
	if (sigr & sigb)
	{
	    /* process amigaguide messages */
	    while (agm = GetAmigaGuideMsg (ai->ai_AmigaGuide))
	    {
		/* check message types */
		switch (agm->agm_Type)
		{
			/* AmigaGuide is ready for us */
		    case ActiveToolID:
			break;

			/* This is a reply to our cmd */
		    case ToolCmdReplyID:
			if (agm->agm_Pri_Ret)
			    DisplayError (agm->agm_Sec_Ret);
			break;

			/* This is a status message */
		    case ToolStatusID:
			if (agm->agm_Pri_Ret)
			    DisplayError (agm->agm_Sec_Ret);
			break;

			/* Shutdown message */
		    case ShutdownMsgID:
			if (agm->agm_Pri_Ret)
			    DisplayError (agm->agm_Sec_Ret);
			break;

		    default:
			break;
		}

		/* Reply to the message */
		ReplyAmigaGuideMsg (agm);
	    }
	}
    }
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

STRPTR nodeNames[] =
{
    "WINDOW",
    "PROCESS",
    NULL,
};

STRPTR nodeTitles[] =
{
    "Project Window Information",
    "Project Process Information",
};

enum
{
    NM_WINDOW,
    NM_PROCESS,
};

LONG Tokenize (struct AppInfo *ai, STRPTR name)
{
    UBYTE buffer[32];
    LONG i;

    /* Chop the prefix from the node name */
    strcpy (buffer, &name[strlen(BASENAME)+1]);

    /* Find the token */
    for (i = 0; nodeNames[i]; i++)
	if (Stricmp (buffer, nodeNames[i]) == 0)
	    return i;
    return -1;
}

/*****************************************************************************/

ULONG ASM nodehost (REG (a0) struct Hook *h, REG (a2) STRPTR o, REG (a1) Msg msg)
{
    struct AppInfo *ai = (struct AppInfo *) h->h_Data;
    struct opFindHost *ofh = (struct opFindHost *)msg;
    struct opNodeIO *onm = (struct opNodeIO *)msg;
    ULONG retval = FALSE;
    LONG token;
    ULONG id;

    switch (msg->MethodID)
    {
	case HM_FINDNODE:
	    /* Get the help group */
	    id = GetTagData (AGA_HelpGroup, 0L, ofh->ofh_Attrs);

	    /* Request for the initial main node? */
	    if (id == 0)
	    {
		/* We have to have a MAIN node, even if we never use it */
		if (Stricmp (ofh->ofh_Node, "main") == 0)
		{
		    /* Show that it is ours */
		    retval = TRUE;
		}
	    }
	    /* Request for one of our true dynamic nodes? */
	    else if (id == ai->ai_NHID)
	    {
		if ((Strnicmp (ofh->ofh_Node, BASENAME, BASENAME_LENGTH) == 0) &&
		    ((token = Tokenize (ai, ofh->ofh_Node)) >= 0))
		{
		    /* Show that it is ours */
		    retval = TRUE;

		    /* Set the node title */
		    ofh->ofh_Title = nodeTitles[token];
		}
	    }
	    break;

	case HM_OPENNODE:
	    /* Allocate the document buffer */
	    if (onm->onm_DocBuffer = AllocVec (512, MEMF_CLEAR))
	    {
		/* Build the document */
		switch (token = Tokenize (ai, onm->onm_Node))
		{
		    case NM_WINDOW:
			sprintf (onm->onm_DocBuffer, "Window: %08lx\n  Size: %ld, %ld, %ld, %ld\n Group: %ld\n",
				 ai->ai_Window,
				 (LONG) ai->ai_Window->LeftEdge, (LONG) ai->ai_Window->TopEdge,
				 (LONG) ai->ai_Window->Width, (LONG) ai->ai_Window->Height,
				 ai->ai_NHID);
			break;

		    case NM_PROCESS:
			sprintf (onm->onm_DocBuffer, "Process: %08lx\n",
				 ai->ai_Process);
			break;
		}

		/* Set the buffer length */
		onm->onm_BuffLen = strlen (onm->onm_DocBuffer);

		/* Remove the node from the database when done */
		onm->onm_Flags |= HTNF_CLEAN;

		/* Show successful open */
		retval = TRUE;
	    }
	    break;

	case HM_CLOSENODE:
	    /* Free the document buffer */
	    FreeVec (onm->onm_DocBuffer);
	    retval = TRUE;
	    break;

	case HM_EXPUNGE:
	    break;
    }

    return retval;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

void main (void)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct Gadget *anchor = NULL;
    struct NewGadget ng;
    struct AppInfo *ai;
    struct Gadget *gad;

    /* Make sure we are running with V39 or greater */
    if (SysBase->LibNode.lib_Version < 39)
    {
	printf ("requires V39\n");
    }
    /* Allocate our global data */
    else if (ai = AllocVec (sizeof (struct AppInfo), MEMF_CLEAR))
    {
	/* Our process */
	ai->ai_Process = (struct Process *) FindTask (NULL);

	/* Initialize the node host hook */
	ai->ai_NHHook.h_Entry = nodehost;
	ai->ai_NHHook.h_Data  = ai;

	/* Open the ROM libraries */
	if (IntuitionBase = OpenLibrary ("intuition.library", 39))
	{
	    GadToolsBase = OpenLibrary ("gadtools.library", 39);
	    GfxBase      = OpenLibrary ("graphics.library", 39);
	    UtilityBase  = OpenLibrary ("utility.library", 39);

	    /* Open AmigaGuide */
	    if (AmigaGuideBase = OpenLibrary ("amigaguide.library", 39))
	    {
		/* Build the unique name */
		ai->ai_NHID = GetUniqueID ();
		sprintf (ai->ai_NHName, "%s.%ld", BASENAME, ai->ai_NHID);

		/* Start the Dynamic Node Host */
		ai->ai_NH = AddAmigaGuideHost (&ai->ai_NHHook, ai->ai_NHName, NULL);

		/* Lock the default public screen */
		if (ai->ai_Screen = LockPubScreen (NULL))
		{
		    /* Obtain the screen visual information for GadTools */
		    if (ai->ai_VI = GetVisualInfoA (ai->ai_Screen, NULL))
		    {
			/* Initialize the global data */
			ai->ai_Region = -1;

			anchor = NULL;
			gad = CreateContext (&anchor);

			/* Set up the constant stuff */
			ng.ng_TextAttr   = &topaz8;
			ng.ng_VisualInfo = ai->ai_VI;

			/* Create a status window */
			ng.ng_LeftEdge   = ai->ai_Screen->WBorLeft + 4;
			ng.ng_TopEdge    = ai->ai_Screen->BarHeight + 1 + 2;
			ng.ng_Width      = 312;
			ng.ng_Height     = 12;
			ng.ng_GadgetText = NULL;
			ng.ng_GadgetID   = MCMD_STATUS;
			ng.ng_Flags      = NULL;
			ng.ng_UserData   = NULL;
			gad = CreateGadget (TEXT_KIND, gad, &ng,
					    GTTX_Justification,	GTJ_CENTER,
					    GTTX_Border,	TRUE,
					    TAG_DONE);
			ai->ai_GStatus = gad;

			/* Create a list view */
			ng.ng_LeftEdge   = ai->ai_Screen->WBorLeft + 4;
			ng.ng_TopEdge   += ng.ng_Height + 4;
			ng.ng_Height     = 64;
			ng.ng_GadgetText = NULL;
			ng.ng_GadgetID   = MCMD_LISTVIEW;
			gad = CreateGadget (LISTVIEW_KIND, gad, &ng,
					    GTLV_ReadOnly,	TRUE,
					    GTLV_ScrollWidth,	18,
					    TAG_DONE);

			/* Create an Accept button */
			ng.ng_LeftEdge   = ai->ai_Screen->WBorLeft + 4;
			ng.ng_TopEdge    = ai->ai_Screen->BarHeight + 1 + 84;
			ng.ng_Width      = 87;
			ng.ng_Height     = 12;
			ng.ng_GadgetText = "Accept";
			ng.ng_GadgetID   = MCMD_ACCEPT;
			gad = CreateGadget (BUTTON_KIND, gad, &ng, TAG_DONE);

			/* Create a Cancel button */
			ng.ng_LeftEdge   = ai->ai_Screen->WBorLeft + 229;
			ng.ng_GadgetText = "Cancel";
			ng.ng_GadgetID   = MCMD_CANCEL;
			gad = CreateGadget (BUTTON_KIND, gad, &ng, TAG_DONE);

			/* Open the window */
			if (ai->ai_Window = OpenWindowTags (NULL,
							    WA_InnerWidth,	320,
							    WA_InnerHeight,	100,
							    WA_Gadgets,		anchor,
							    WA_IDCMP,		IDCMP_FLAGS,
							    WA_Title,		(ULONG) "AmigaGuide Demo",
							    WA_PubScreen,	ai->ai_Screen,
							    WA_MenuHelp,	TRUE,
							    WA_AutoAdjust,	TRUE,
							    WA_DragBar,		TRUE,
							    WA_DepthGadget,	TRUE,
							    WA_CloseGadget,	TRUE,
							    WA_SmartRefresh,	TRUE,
							    WA_NewLookMenus,	TRUE,
							    WA_Activate,	TRUE,
							    WA_HelpGroup,	ai->ai_NHID,
							    TAG_DONE))
			{
			    /* Create the menus */
			    if (ai->ai_Menu = CreateMenus (main_menu, TAG_DONE))
				if (LayoutMenus (ai->ai_Menu, ai->ai_VI,GTMN_NewLookMenus,TRUE,TAG_DONE))
				    SetMenuStrip (ai->ai_Window, ai->ai_Menu);

			    /* Turn on gadget help */
			    HelpControl (ai->ai_Window, HC_GADGETHELP);

			    /* Remember the AppInfo */
			    ai->ai_Window->UserData = (APTR) ai;

			    /* We don't want the window to automatically activate */
			    ai->ai_NAG.nag_Flags = HTF_NOACTIVATE;

			    /* Set the application base name */
			    ai->ai_NAG.nag_BaseName = BASENAME;

			    /* Set the document name */
			    ai->ai_NAG.nag_Name = "advaghelp.guide";

			    /* establish the base name to use for hypertext ARexx port */
			    ai->ai_NAG.nag_ClientPort = "AGAPP_HELP";

			    /* Set up the context table */
			    ai->ai_NAG.nag_Context = context;

			    /* Open the help system */
			    ai->ai_AmigaGuide = OpenAmigaGuideAsync (&ai->ai_NAG,
								     AGA_HelpGroup, ai->ai_NHID,
								     TAG_DONE);

			    /* Take care of business */
			    HandleEvents (ai);

			    /* Shutdown the help system */
			    CloseAmigaGuide (ai->ai_AmigaGuide);

			    /* Clear the menu strip */
			    ClearMenuStrip (ai->ai_Window);

			    /* Free the menus */
			    FreeMenus (ai->ai_Menu);

			    /* Close the application window */
			    CloseWindow (ai->ai_Window);
			}

			/* Free the gadgets */
			FreeGadgets (anchor);

			/* Free the GadTools visual info */
			FreeVisualInfo (ai->ai_VI);
		    }

		    /* Unlock the default public screen */
		    UnlockPubScreen (NULL, ai->ai_Screen);
		}

		/* Remove the dynamic node host */
		while (RemoveAmigaGuideHost (ai->ai_NH, NULL) > 0)
		    Delay (10);

		/* Close AmigaGuide */
		CloseLibrary (AmigaGuideBase);
	    }

	    /* Close the ROM libraries */
	    CloseLibrary (UtilityBase);
	    CloseLibrary (GfxBase);
	    CloseLibrary (GadToolsBase);
	    CloseLibrary (IntuitionBase);
	}
	FreeVec (ai);
    }
}
