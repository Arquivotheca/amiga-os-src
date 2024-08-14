/* events.c
 *
 */

#include "graffiti.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

#define	MMC_OPEN		0
#define	MMC_SAVE		1
#define	MMC_SAVEAS		2
#define	MMC_PRINT		3
#define	MMC_ABOUT		4
#define	MMC_ICONIFY		5
#define	MMC_QUIT		6

#define	MMC_MARK		10
#define	MMC_CUT			11
#define	MMC_COPY		12
#define	MMC_PASTE		13
#define	MMC_ERASE		14
#define	MMC_UNDO		15

#define	MMC_TALK		20

#define	MMC_SAVE_SETTINGS	100

/*****************************************************************************/

struct NewMenu newmenu[] =
{
    {NM_TITLE, "Project",},
    {NM_ITEM,  "Open...",		"O",	NULL,	NULL,	V (MMC_OPEN)},
    {NM_ITEM,  NM_BARLABEL,},
    {NM_ITEM,  "Save",			"S",	NULL,	NULL,	V (MMC_SAVE)},
    {NM_ITEM,  "Save As...",		"A",	NULL,	NULL,	V (MMC_SAVEAS)},
    {NM_ITEM,  NM_BARLABEL,},
    {NM_ITEM,  "Print",			"P",	NULL,	NULL,	V (MMC_PRINT)},
    {NM_ITEM,  "About...",		"?",	NULL,	NULL,	V (MMC_ABOUT)},
    {NM_ITEM,  "Iconify",		NULL,	NULL,	NULL,	V (MMC_ICONIFY)},
    {NM_ITEM,  NM_BARLABEL,},
    {NM_ITEM,  "Quit",			"Q",	NULL,	NULL,	V (MMC_QUIT)},

    {NM_TITLE, "Edit",},
    {NM_ITEM,  "Mark",			"B",	NULL,	NULL,	V (MMC_MARK)},
    {NM_ITEM,  "Cut",			"X",	NULL,	NULL,	V (MMC_CUT)},
    {NM_ITEM,  "Copy",			"C",	NULL,	NULL,	V (MMC_COPY)},
    {NM_ITEM,  "Paste",			"V",	NULL,	NULL,	V (MMC_PASTE)},
    {NM_ITEM,  NM_BARLABEL,},
    {NM_ITEM,  "Erase",			"E",	NULL,	NULL,	V (MMC_ERASE)},
    {NM_ITEM,  NM_BARLABEL,},
    {NM_ITEM,  "Undo",			"Z",	NULL,	NULL,	V (MMC_UNDO)},

    {NM_TITLE, "Extras",},
    {NM_ITEM,  "Talk...",		"T",	NULL,	NULL,	V (MMC_TALK)},

    {NM_TITLE, "Settings",},
    {NM_ITEM,  "Save Settings",		"B",	NULL,	NULL,	V (MMC_SAVE_SETTINGS)},

    {NM_END,},
};

/*****************************************************************************/

UWORD chip GraffitiAII1Data[] =
{
/* Plane 0 */
    0xFFFF,0xFFFF,0xFFFF,0xFFFC,0xC000,0x0000,0x0000,0x0000,
    0xC000,0xFC00,0x0000,0x0000,0xC001,0x5600,0x0000,0x0000,
    0xC001,0x5600,0x0000,0x0000,0xC001,0xFE00,0x0000,0x0000,
    0xC001,0xFE00,0x0000,0x0000,0xC001,0xFE00,0x0000,0x0000,
    0xC001,0xFE00,0x0000,0x0000,0xC001,0xFE00,0x0000,0x0040,
    0xC001,0xFE00,0x0000,0x00C0,0xC001,0xFE00,0x0000,0x00C0,
    0xC00F,0xFFFF,0xFFFF,0x80C0,0xC039,0xFE00,0x0000,0xE0C0,
    0xC021,0xFE00,0x0000,0x30C0,0xC021,0xFE00,0x0380,0x30C0,
    0xC021,0xFE00,0x00E0,0x30C0,0xC021,0xFE00,0x0030,0x30C0,
    0xC021,0xFE01,0xFF30,0x30C0,0xC021,0xFE01,0x8120,0x30C0,
    0xC021,0x0600,0xF160,0x30C0,0xC020,0xAC00,0x0F80,0x30C0,
    0xC020,0x7800,0x0300,0x30C0,0xC020,0x32A7,0xFE00,0x30C0,
    0xC020,0x0000,0x0000,0x30C0,0xC020,0x0000,0x0000,0x30C0,
    0xC038,0x0000,0x0000,0xE0C0,0xC00F,0xFFFF,0xFFFF,0x80C0,
    0xC000,0x0000,0x0000,0x00C0,0xC000,0x0000,0x0000,0x00C0,
    0xC7FF,0xFFFF,0xFFFF,0xFFC0,0xC000,0x0000,0x0000,0x0000,
    0xC000,0x0000,0x0000,0x0000,0x8000,0x0000,0x0000,0x0000,
/* Plane 1 */
    0x0000,0x0000,0x0000,0x0002,0x0000,0x0000,0x0000,0x0006,
    0x0000,0x0000,0x0000,0x0006,0x0000,0xA800,0x0000,0x0006,
    0x0000,0xA800,0x0000,0x0006,0x0000,0x0000,0x0000,0x0006,
    0x0000,0xF800,0x0000,0x0006,0x0000,0xA800,0x0000,0x0006,
    0x0000,0xA800,0x0000,0x0006,0x0FFE,0xA9FF,0xFFFF,0xFF86,
    0x0D54,0xA955,0x5555,0x5506,0x0D54,0xA955,0x5555,0x5506,
    0x0D50,0xA800,0x0000,0x5506,0x0D40,0xA800,0x0000,0x1506,
    0x0D40,0xA800,0x0000,0x0506,0x0D40,0xA800,0x0000,0x0506,
    0x0D40,0xA800,0x0000,0x0506,0x0D40,0xA800,0x0000,0x0506,
    0x0D40,0xA800,0x0000,0x0506,0x0D40,0xA800,0x0000,0x0506,
    0x0D40,0xF800,0x0000,0x0506,0x0D40,0x5000,0x0000,0x0506,
    0x0D40,0x0000,0x0000,0x0506,0x0D40,0x0000,0x0000,0x0506,
    0x0D40,0x0000,0x0000,0x0506,0x0D40,0x0000,0x0000,0x0506,
    0x0D40,0x0000,0x0000,0x1506,0x0D50,0x0000,0x0000,0x5506,
    0x0D55,0x5555,0x5555,0x5506,0x0D55,0x5555,0x5555,0x5506,
    0x0800,0x0000,0x0000,0x0006,0x0000,0x0000,0x0000,0x0006,
    0x0000,0x0000,0x0000,0x0006,0x7FFF,0xFFFF,0xFFFF,0xFFFE,
};

struct Image GraffitiAII1 =
{
    0, 0,			/* Upper left corner */
    63, 34, 2,			/* Width, Height, Depth */
    GraffitiAII1Data,		/* Image data */
    0x0003, 0x0000,		/* PlanePick, PlaneOnOff */
    NULL			/* Next image */
};

struct DiskObject appicon =
{
    WB_DISKMAGIC,		/* Magic Number */
    WB_DISKVERSION,		/* Version */
    {				/* Embedded Gadget Structure */
	NULL,			/* Next Gadget Pointer */
	0, 0, 63, 35,		/* Left,Top,Width,Height */
	GADGIMAGE | GADGHCOMP,	/* Flags */
	RELVERIFY,		/* Activation Flags */
	BOOLGADGET,		/* Gadget Type */
	(APTR)&GraffitiAII1,	/* Render Image */
	NULL,			/* Select Image */
	NULL,			/* Gadget Text */
	NULL,			/* Mutual Exclude */
	NULL,			/* Special Info */
	100,			/* Gadget ID */
	(APTR) 0x0001,		/* User Data (Revision) */
    },
    WBTOOL,			/* Icon Type */
    NULL,			/* Default Tool */
    NULL,			/* Tool Type Array */
    NO_ICON_POSITION,		/* Current X */
    NO_ICON_POSITION,		/* Current Y */
    NULL,			/* Drawer Structure */
    NULL,			/* Tool Window */
    4096			/* Stack Size */
};

/*****************************************************************************/

void CreateMainMenu (struct GlobalData *gd)
{
    if (gd->gd_Menu = createmenus (gd, newmenu, GTMN_FullMenu, TRUE, TAG_DONE))
	if (layoutmenus (gd, GTMN_NewLookMenus, TRUE, TAG_DONE))
	    SetMenuStrip (gd->gd_Window, gd->gd_Menu);
}

/*****************************************************************************/

BOOL ProcessCommand (struct GlobalData *gd, ULONG cmd, struct IntuiMessage *imsg)
{
    BOOL quit = FALSE;

    switch (cmd)
    {
	case MMC_ICONIFY:
	    if (gd->gd_AppIcon)
		break;

	    if (gd->gd_AppIcon = AddAppIconA (1, NULL, gd->gd_Server, gd->gd_WBPort, NULL, &appicon, NULL))
	    {
	    }
	    break;

	case MMC_QUIT:
	    quit = TRUE;
	    break;

	case MMC_ERASE:
	    if (gd->gd_SEntity == NULL)
	    {
		ClearFunc (gd, gd->gd_BGPen);
		BroadCastCommand (gd, CMD_CLEAR, &gd->gd_BGPen, sizeof (UBYTE));
	    }
	    else
	    {
		SetViewWindowAttrs (gd->gd_WinClass, gd->gd_Window, WOA_Title, "Only server can erase", TAG_DONE);
	    }
	    break;

	case MMC_TALK:
	    OpenTalkWindow (gd);
	    break;
    }
    return quit;
}

/*****************************************************************************/

void HandleEvents (struct GlobalData * gd)
{
    struct Transaction *trans;
    struct IntuiMessage *imsg;
    struct TagItem *attrs;
    struct AppMessage *amsg;
    struct MenuItem *menuitem;
    BOOL update = TRUE;
    UBYTE tmp[80];
    UWORD px, py;
    ULONG sigr;
    ULONG sigi;
    ULONG sigt;
    ULONG siga;
    ULONG sigc;
    ULONG msize;
    BOOL quit;
    ULONG id;

    /* Initialize the scrollers */
    NewSizeFunc (gd);

    /* Cache the signal bits that we are going to wait on */
    siga = 1L << gd->gd_WBPort->mp_SigBit;
    sigi = 1L << gd->gd_IDCMP->mp_SigBit;
    sigt = 1L << gd->gd_BitNumber;

    /* Keep on going til the going gets tough */
    gd->gd_Going = TRUE;
    while (gd->gd_Going)
    {
	/* Build the Talk window bit */
	sigc = NULL;
	if (gd->gd_CPort)
	    sigc = 1L << gd->gd_CPort->mp_SigBit;

	/* Do we need to update the display? */
	if (update)
	{
	    /* Display the data */
	    if (gd->gd_BitMap)
	    {
		BltBitMapRastPort (gd->gd_BitMap, gd->gd_HTop, gd->gd_VTop,
				   gd->gd_Window->RPort,
				   gd->gd_Window->BorderLeft, gd->gd_Window->BorderTop,
				   gd->gd_InnerWidth, gd->gd_InnerHeight, 0xC0);
	    }

	    /* Clear the update flag */
	    update = FALSE;
	}

	/* Wait for something to happen */
	sigr = Wait (siga | sigi | sigt | sigc | SIGBREAKF_CTRL_C);

	/********************************/
	/* Handle the NIPC transactions */
	/********************************/
	quit = HandleNIPCEvents (gd);

	/********************************/
	/* Handle the talk transactions */
	/********************************/
	if (sigr & sigc)
	{
	    quit = HandleTalkEvents (gd);
	}

	/**************************/
	/* Handle App... messages */
	/**************************/
	while (amsg = (struct AppMessage *) GetMsg (gd->gd_WBPort))
	{
	    if (amsg->am_Type == MTYPE_APPICON)
	    {
		RemoveAppIcon (gd->gd_AppIcon);
		gd->gd_AppIcon = NULL;
	    }

	    ReplyMsg ((struct Message *) amsg);
	}

	/*******************************/
	/* Pull the Intuition messages */
	/*******************************/
	while (imsg = (struct IntuiMessage *) GetMsg (gd->gd_IDCMP))
	{
	    /* Handle each message class */
	    switch (imsg->Class)
	    {
		case IDCMP_MOUSEBUTTONS:
		    if ((imsg->Code == SELECTDOWN) && (gd->gd_RPort.BitMap = gd->gd_BitMap))
		    {
			gd->gd_Flags |= GDF_DRAWING;
			gd->gd_NumPlot = 0;

			px = gd->gd_HTop + imsg->MouseX - gd->gd_Window->BorderLeft;
			py = gd->gd_VTop + imsg->MouseY - gd->gd_Window->BorderTop;

			gd->gd_Plot[gd->gd_NumPlot].p_X = px;
			gd->gd_Plot[gd->gd_NumPlot].p_Y = py;
			gd->gd_NumPlot++;

			SetAPen (gd->gd_Window->RPort, gd->gd_FGPen);
			SetBPen (gd->gd_Window->RPort, gd->gd_BGPen);
			SetDrMd (gd->gd_Window->RPort, gd->gd_DrMode);
			Move (gd->gd_Window->RPort, imsg->MouseX, imsg->MouseY);
			Draw (gd->gd_Window->RPort, imsg->MouseX, imsg->MouseY);

			SetAPen (&gd->gd_RPort, gd->gd_FGPen);
			SetBPen (&gd->gd_RPort, gd->gd_BGPen);
			SetDrMd (&gd->gd_RPort, gd->gd_DrMode);
			Move (&gd->gd_RPort, px, py);
			Draw (&gd->gd_RPort, px, py);
		    }
		    else if ((imsg->Code == SELECTUP) && (gd->gd_Flags & GDF_DRAWING))
		    {
			gd->gd_Flags &= ~GDF_DRAWING;
			BroadCastPoints (gd, NULL);
		    }
		    break;

		case IDCMP_MOUSEMOVE:
		    if ((gd->gd_NumPlot < 1024) && (gd->gd_Flags & GDF_DRAWING))
		    {
			px = gd->gd_HTop + imsg->MouseX - gd->gd_Window->BorderLeft;
			py = gd->gd_VTop + imsg->MouseY - gd->gd_Window->BorderTop;

			gd->gd_Plot[gd->gd_NumPlot].p_X = px;
			gd->gd_Plot[gd->gd_NumPlot].p_Y = py;
			gd->gd_NumPlot++;

			Draw (gd->gd_Window->RPort, imsg->MouseX, imsg->MouseY);
			Draw (&gd->gd_RPort, px, py);
		    }
		    break;

		case IDCMP_VANILLAKEY:
		    switch (imsg->Code)
		    {
			case '0':
			case '1':
			    gd->gd_FGPen = imsg->Code - '0';
			    break;

			case 27:
			case 'q':
			    quit = TRUE;
			    break;

			case 'Q':
			    gd->gd_Going = FALSE;
			    break;

			case 'c':
			case 'C':
			    ProcessCommand (gd, MMC_ERASE, imsg);
			    break;
		    }
		    break;

		case IDCMP_MENUPICK:
		    id = imsg->Code;
		    while ((id != MENUNULL) && gd->gd_Going)
		    {
			menuitem = ItemAddress (imsg->IDCMPWindow->MenuStrip, id);
			quit = ProcessCommand (gd, (ULONG) MENU_USERDATA (menuitem), imsg);
			id = menuitem->NextSelect;
		    }
		    break;

		case IDCMP_NEWSIZE:
		    /* Recalculate the scrollers */
		    NewSizeFunc (gd);

		case IDCMP_REFRESHWINDOW:
		    /* Need to update the display */
		    update = TRUE;
		    break;

		case IDCMP_CLOSEWINDOW:
		    quit = TRUE;
		    break;

		case IDCMP_IDCMPUPDATE:
		    /* Get a TagItem pointer to the attributes */
		    attrs = (struct TagItem *) imsg->IAddress;

		    /* Did either of the scrollers change? */
		    if (FindTagItem (WOA_TopVert, attrs) || FindTagItem (WOA_TopVert, attrs))
		    {
			/* Get the current top values from the window */
			GetCurrentTopValues (gd->gd_WinClass, gd->gd_Window,
					     &gd->gd_HTop, &gd->gd_VTop);

			/* Need to update the display */
			update = TRUE;
		    }
		    break;
	    }

	    /* Reply to the message */
	    ReplyMsg ((struct Message *) imsg);
	}

	/****************/
	/* Got a CTRL-C */
	/****************/
	if (sigr & SIGBREAKF_CTRL_C)
	    quit = TRUE;

	/****************************/
	/* Are we supposed to quit? */
	/****************************/
	if (quit)
	{
	    if (gd->gd_SEntity == NULL)
	    {
		if (gd->gd_NumClients)
		{
		    struct List *list = &gd->gd_ClientList;
		    struct Node *node;
		    struct Client *c;

		    lprintf (gd, "Can't quit while there are still clients.\n", NULL);

		    /* Make sure there are entries in the list */
		    if (list->lh_TailPred != (struct Node *) list)
		    {
			/* Step through the list */
			for (node = (struct Node *) list->lh_Head; node->ln_Succ; node = node->ln_Succ)
			{
			    c = (struct Client *) node;
			    lprintf (gd, "  %s\n", (void *) c->c_Name);
			}
		    }
		}
		else
		    gd->gd_Going = FALSE;
	    }
	    else
	    {
		/* Build the name */
		sprintf (tmp, "%s@%s", gd->gd_User, gd->gd_Host);
		msize = strlen (tmp) + 1;

		/* Try logging out */
		if (trans = alloctransaction (gd, TRN_AllocReqBuffer, msize, TAG_DONE))
		{
		    /* Initialize the message */
		    trans->trans_Command = CMD_LOGOUT;
		    strcpy ((char *) trans->trans_RequestData, tmp);
		    trans->trans_ReqDataActual = msize;
		    trans->trans_Timeout = 15;

		    /* Send the logout message */
		    BeginTransaction (gd->gd_SEntity, gd->gd_HEntity, trans);
		}
	    }
	}
    }
}
