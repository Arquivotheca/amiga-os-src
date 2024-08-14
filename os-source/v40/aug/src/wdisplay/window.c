/* window.c
 *
 */

#include "wdisplay.h"

UWORD chip DArrowData[] =	/* Down Arrow */
{0xFFFF, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x8001, 0xE007, 0xF81F, 0xFE7F};

UWORD chip UArrowData[] =	/* Up Arrow */
{0xFE7F, 0xF81F, 0xE007, 0x8001, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xFFFF};

UWORD chip RArrowData[] =	/* Right Arrow */
{0xFFFF, 0xFF3F, 0xFF0F, 0xC003, 0xC000, 0xC003, 0xFF0F, 0xFF3F, 0xFFFF};

UWORD chip LArrowData[] =	/* Left Arrow */
{0xFFFF, 0xFCFF, 0xF0FF, 0xC003, 0x0003, 0xC003, 0xF0FF, 0xFCFF, 0xFFFF};

struct Image Arrows[] =
{
    {0, 0, 16, 9, 1, DArrowData, 0x0001, 0x0000, NULL},
    {0, 0, 16, 9, 1, UArrowData, 0x0001, 0x0000, NULL},
    {0, 0, 16, 9, 1, RArrowData, 0x0001, 0x0000, NULL},
    {0, 0, 16, 9, 1, LArrowData, 0x0001, 0x0000, NULL},
};

/* SpecialInfo Structures & Data */
#define	PHFLAG	(AUTOKNOB | FREEHORIZ | PROPNEWLOOK)
#define	PVFLAG	(AUTOKNOB | FREEVERT | PROPNEWLOOK)

struct PropInfo WG2S =
{PHFLAG, -1, -1, -1, -1,};
struct PropInfo WG3S =
{PVFLAG, -1, -1, -1, -1,};
struct PropInfo WG4S =
{0x005, 0, 0, MAXBODY, MAXBODY,};
struct PropInfo WG5S =
{0x003, 0, 0, MAXBODY, MAXBODY,};

#define	NUM_GADGETS	6

/********* THESE ARE USED FOR 2.0 AND NEWER ********************************/

/* Gadget Structures */
struct Gadget WG[] =
{
 /* Vertical Scrollbar ================================================== */

 /* Down arrow */
    {&WG[1], -17, -19, 18, 10,
     GADGHCOMP | GRELBOTTOM | GRELRIGHT,
     RELVERIFY | GADGIMMEDIATE | RIGHTBORDER,
     BOOLGADGET, NULL, NULL, NULL, NULL, NULL, 4, NULL,},

 /* Up arrow */
    {&WG[2], -17, -29, 18, 10,
     GADGHCOMP | GRELBOTTOM | GRELRIGHT,
     RELVERIFY | GADGIMMEDIATE | RIGHTBORDER,
     BOOLGADGET, NULL, NULL, NULL, NULL, NULL, 3, NULL,},

 /* Vertical Slider */
    {&WG[3], -13, 12, 10, -43,
     GADGHCOMP | GADGIMAGE | GRELRIGHT | GRELHEIGHT,
     GACT_IMMEDIATE | GACT_RIGHTBORDER | GACT_FOLLOWMOUSE | GACT_RELVERIFY,
     PROPGADGET, NULL, NULL, NULL, NULL, NULL, 20, NULL,},

 /* Horizontal Scrollbar ================================================ */

 /* Right arrow */
    {&WG[4], -33, -9, 16, 10,
     GADGHCOMP | GRELBOTTOM | GRELRIGHT,
     RELVERIFY | GADGIMMEDIATE | BOTTOMBORDER,
     BOOLGADGET, NULL, NULL, NULL, NULL, NULL, 14, NULL,},

 /* Left arrow */
    {&WG[5], -49, -9, 16, 10,
     GADGHCOMP | GRELBOTTOM | GRELRIGHT,
     RELVERIFY | GADGIMMEDIATE | BOTTOMBORDER,
     BOOLGADGET, NULL, NULL, NULL, NULL, NULL, 13, NULL,},

 /* Horizontal Slider */
    {NULL, 3, -7, -55, 6,
     GADGHCOMP | GADGIMAGE | GRELBOTTOM | GRELWIDTH,
     GACT_IMMEDIATE | GACT_BOTTOMBORDER | GACT_FOLLOWMOUSE | GACT_RELVERIFY,
     PROPGADGET, NULL, NULL, NULL, NULL, NULL, 30, NULL,},
};

/**** THESE ARE USED FOR 1.3 AND OLDER *************************************/

/* Gadget Structures */
struct Gadget OGad[] =
{
 /* Vertical Scrollbar =================================================== */

 /* Down arrow */
    {&OGad[1], -15, -17, 16, 9,
     GADGHCOMP | GADGIMAGE | GRELBOTTOM | GRELRIGHT,
     RELVERIFY | GADGIMMEDIATE | RIGHTBORDER,
     BOOLGADGET, (APTR) & Arrows[0], NULL, NULL, NULL, NULL, 4, NULL,},

 /* Up arrow */
    {&OGad[2], -15, -26, 16, 9,
     GADGHCOMP | GADGIMAGE | GRELBOTTOM | GRELRIGHT,
     RELVERIFY | GADGIMMEDIATE | RIGHTBORDER,
     BOOLGADGET, (APTR) & Arrows[1], NULL, NULL, NULL, NULL, 3, NULL,},

 /* Vertical slider */
    {&OGad[3], -15, 18, 16, -28,
     GADGHCOMP | GADGIMAGE | GRELRIGHT | GRELHEIGHT,
     GACT_IMMEDIATE | GACT_RIGHTBORDER | GACT_FOLLOWMOUSE | GACT_RELVERIFY,
     PROPGADGET, NULL, NULL, NULL, NULL, NULL, 20, NULL,},

 /* Horizontal Scrollbar ================================================= */

 /* Right arrow */
    {&OGad[4], -31, -8, 16, 9,
     GADGHCOMP | GADGIMAGE | GRELBOTTOM | GRELRIGHT,
     RELVERIFY | GADGIMMEDIATE | BOTTOMBORDER,
     BOOLGADGET, (APTR) & Arrows[2], NULL, NULL, NULL, NULL, 14, NULL,},

 /* Left arrow */
    {&OGad[5], -47, -8, 16, 9,
     GADGHCOMP | GADGIMAGE | GRELBOTTOM | GRELRIGHT,
     RELVERIFY | GADGIMMEDIATE | BOTTOMBORDER,
     BOOLGADGET, (APTR) & Arrows[3], NULL, NULL, NULL, NULL, 13, NULL,},

 /* Horizontal slider */
    {NULL, 3, -8, -51, 9,
     GADGHCOMP | GADGIMAGE | GRELBOTTOM | GRELWIDTH,
     GACT_IMMEDIATE | GACT_BOTTOMBORDER | GACT_FOLLOWMOUSE | GACT_RELVERIFY,
     PROPGADGET, NULL, NULL, NULL, NULL, NULL, 30, NULL,},
};

struct TagItem WA_Tags[] =
{
    {WA_AutoAdjust, TRUE},
    {TAG_DONE,},
};

#define	WIN_FLAGS	WFLG_SIZEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | \
			WFLG_ACTIVATE | WFLG_REPORTMOUSE | WFLG_SIMPLE_REFRESH | \
			WFLG_SIZEBBOTTOM | WFLG_SIZEBRIGHT | WFLG_NW_EXTENDED

#if DOBACKDROP
#define	BD_FLAGS	WFLG_REPORTMOUSE | WFLG_SIMPLE_REFRESH | WFLG_BACKDROP | \
			WFLG_ACTIVATE | WFLG_BORDERLESS | WFLG_NW_EXTENDED
#endif

/* NewWindow Structures */
struct ExtNewWindow NewWindowStructure1 =
{
    0, 1, 320, 199, 0, 1,

    NULL,

    WIN_FLAGS,

    NULL, NULL, BASENAME, NULL, NULL, 67, 40, -1, -1, CUSTOMSCREEN, WA_Tags
};

struct Window *OpenDocWindow (struct AppInfo * ai)
{
    struct NewWindow *nw;
    struct Gadget *gad;
    WORD i;

    /* initialize the NewWindow structure */
    nw = &NewWindowStructure1;

    /* Set the window title */
    if (ai->ai_Options[OPT_TITLE])
    {
	nw->Title = (STRPTR) ai->ai_Options[OPT_TITLE];
    }

    /* Tell about our screen */
    nw->Screen = ai->ai_Screen;

    /* Set the minimum height of the window */
    nw->MinHeight =
      ai->ai_Screen->WBorTop +	/* window border size */
      ai->ai_Screen->Font->ta_YSize +	/* font height */
      36L;			/* gadget size */

#if 0
    if (IntuitionBase->lib_Version < 36)
    {
	nw->DetailPen = ai->ai_DI->dri_Pens[BACKGROUNDPEN];
    }
#endif

    /* Load the window position from the preference file */
    nw = LoadSnapShot (ai, nw, ai->ai_Prefs, BASENAME);

#if DOBACKDROP
    /* Are we backdrop? */
    if (ai->ai_Options[OPT_BACKDROP])
    {
	nw->Title = NULL;
	nw->Flags = BD_FLAGS;
	nw->LeftEdge = 0;
	nw->TopEdge = ai->ai_Screen->BarHeight;
	nw->Width = ai->ai_Screen->Width;
	nw->Height = ai->ai_Screen->Height - ai->ai_Screen->BarHeight;
    }
    else
    {
#endif
	/* copy the template gadgets */
	for (i = 0; i < NUM_GADGETS; i++)
	{
	    /* copy the gadget */
	    if (IntuitionBase->lib_Version >= 36)
	    {
		ai->ai_Gads[i] = WG[i];
	    }
	    else
	    {
		ai->ai_Gads[i] = OGad[i];
	    }

	    gad = &(ai->ai_Gads[i]);
	    gad->NextGadget = NULL;

	    /* set up the next gadget pointer */
	    if (i > 0)
	    {
		(ai->ai_Gads[(i - 1)]).NextGadget = &ai->ai_Gads[i];
	    }
	}

	/* Correct the top of the slider */
	ai->ai_Gads[2].TopEdge =
	  ai->ai_Screen->WBorTop + ai->ai_Screen->Font->ta_YSize + 2;

	/* correct the height of the slider */
	ai->ai_Gads[2].Height = (0 - (31 + ai->ai_Gads[2].TopEdge));

	ai->ai_PVFlags = PVFLAG;
	ai->ai_PHFlags = PHFLAG;

	/* set up OS version dependant stuff */
	if (IntuitionBase->lib_Version < 36)
	{
	    ai->ai_PIV = WG4S;
	    ai->ai_PIH = WG5S;
	    ai->ai_Gads[2].TopEdge -= 3;
	    ai->ai_Gads[2].Height += 7;
	}
	else
	{
	    ai->ai_PIV = WG3S;
	    ai->ai_PIH = WG2S;

	    /* Create the gadget imagry */
	    ai->ai_Gads[0].GadgetRender = (APTR) CreateBorder (ai->ai_DI->dri_Pens, EB_UP_ARROW, 18, 10, NULL);
	    ai->ai_Gads[1].GadgetRender = (APTR) CreateBorder (ai->ai_DI->dri_Pens, EB_DOWN_ARROW, 18, 10, NULL);
	    ai->ai_Gads[0].SelectRender = (APTR) CreateBorder (ai->ai_DI->dri_Pens, EB_SGL1_IN, 18, 10, NULL);
	    ai->ai_Gads[1].SelectRender = ai->ai_Gads[0].SelectRender;

	    ai->ai_Gads[3].GadgetRender = (APTR) CreateBorder (ai->ai_DI->dri_Pens, EB_RIGHT_ARROW, 16, 10, NULL);
	    ai->ai_Gads[4].GadgetRender = (APTR) CreateBorder (ai->ai_DI->dri_Pens, EB_LEFT_ARROW, 16, 10, NULL);
	    ai->ai_Gads[3].SelectRender = (APTR) CreateBorder (ai->ai_DI->dri_Pens, EB_SGL1_IN, 16, 10, NULL);
	    ai->ai_Gads[4].SelectRender = ai->ai_Gads[3].SelectRender;
	}

	/* point the slider gadget specialinfo at the propinfo */
	ai->ai_Gads[2].SpecialInfo = (APTR) & ai->ai_PIV;
	ai->ai_Gads[5].SpecialInfo = (APTR) & ai->ai_PIH;

	/* point the render at the image */
	ai->ai_Gads[2].GadgetRender = (APTR) & ai->ai_SIV;
	ai->ai_Gads[5].GadgetRender = (APTR) & ai->ai_SIH;

	/* initialize the window first gadget pointer */
	nw->FirstGadget = &ai->ai_Gads[0];

#if DOBACKDROP
    }
#endif

    /* Fill in the coordinates */
    if (ai->ai_Options[OPT_LEFT] >= 0)
    {
	nw->LeftEdge = (WORD) ai->ai_Options[OPT_LEFT];
    }
    if (ai->ai_Options[OPT_TOP] >= 0)
    {
	nw->TopEdge = (WORD) ai->ai_Options[OPT_TOP];
    }
    if (ai->ai_Options[OPT_WIDTH] >= 0)
    {
	nw->Width = (WORD) ai->ai_Options[OPT_WIDTH];
    }
    if (ai->ai_Options[OPT_HEIGHT] >= 0)
    {
	nw->Height = (WORD) ai->ai_Options[OPT_HEIGHT];
    }

    /* open the window */
    if (ai->ai_Window = OpenWindow (nw))
    {
	/* Set the window font */
	SetFont (ai->ai_Window->RPort, ai->ai_TextFont);

	/* Set to zero */
	SetAPen (ai->ai_Window->RPort, 0);

	/* Get the inner size */
	ai->ai_Rows = ai->ai_Window->Height - ai->ai_Window->BorderTop - ai->ai_Window->BorderBottom;
	ai->ai_Columns = ai->ai_Window->Width - ai->ai_Window->BorderLeft - ai->ai_Window->BorderRight;

	/* Remember the AppInfo */
	ai->ai_Window->UserData = (APTR) & ai;

	/* See if we're capable of being an AppWindow */
	if (ai->ai_AWMPort)
	{
	    STRPTR menu;

	    /* Start the AppWindow */
	    ai->ai_AW = AddAppWindow (0L, NULL, ai->ai_Window, ai->ai_AWMPort, NULL);

	    /* Start the AppMenuItem */
	    menu = (STRPTR) (ai->ai_Options[OPT_MENUITEM] ? ai->ai_Options[OPT_MENUITEM] : GetWDisplayString (ai, MISC_APPMENUITEM));
	    ai->ai_AM = AddAppMenuItemA (1L, NULL, menu, ai->ai_AWMPort, NULL);
	}

	/* set up the message port information */
	ai->ai_Window->UserPort = ai->ai_IDCMP;
	ModifyIDCMP (ai->ai_Window, IDCMP_FLAGS);
    }

    return (ai->ai_Window);
}

VOID CloseDocWindow (struct AppInfo * ai)
{
    /* Remove the AppWindow, if there is one */
    if (ai->ai_AW)
    {
	RemoveAppWindow (ai->ai_AW);
	ai->ai_AW = NULL;
    }

    /* Remove the AppWindow, if there is one */
    if (ai->ai_AM)
    {
	RemoveAppMenuItem (ai->ai_AM);
	ai->ai_AM = NULL;
    }

    if (ai->ai_Window)
    {
	/* Snapshot the window position */
	SaveSnapShot (ai, ai->ai_Window, ai->ai_Prefs, BASENAME);

	/* Close the application window */
	CloseWindowSafely (ai->ai_Window);
    }

    if (IntuitionBase->lib_Version >= 36)
    {
	/* Get rid of the borders */
	DisposeBorder (ai->ai_Gads[0].GadgetRender);
	DisposeBorder (ai->ai_Gads[1].GadgetRender);
	DisposeBorder (ai->ai_Gads[0].SelectRender);

	DisposeBorder (ai->ai_Gads[3].GadgetRender);
	DisposeBorder (ai->ai_Gads[4].GadgetRender);
	DisposeBorder (ai->ai_Gads[3].SelectRender);
    }
}
