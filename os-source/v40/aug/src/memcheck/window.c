/* window.c
 *
 */

#include "memcheck.h"

/*****************************************************************************/

#include "memcheck_rev.h"

/*****************************************************************************/

static struct TextAttr TOPAZ8 = {"topaz.font", 8,};

/*****************************************************************************/

#define	IDCMP_FLAGS	IDCMP_MENUPICK | IDCMP_CLOSEWINDOW | IDCMP_GADGETDOWN | IDCMP_GADGETUP

/*****************************************************************************/

static struct NewMenu newmenu[] =
{
    {NM_TITLE, "Project",},
    {NM_ITEM, "Hide", "H", NULL, NULL, V (MMC_HIDE)},
    {NM_ITEM, "Quit", "Q", NULL, NULL, V (MMC_QUIT)},

    {NM_END,}
};

/*****************************************************************************/

BOOL OpenCWindow (struct GlobalData * gd)
{
    struct NewGadget ng;

    if (gd->gd_Window)
    {
	/* Bring the screen to the front */
	ScreenToFront (gd->gd_Window->WScreen);

	/* Bring the window to front */
	WindowToFront (gd->gd_Window);

	/* Activate the window */
	ActivateWindow (gd->gd_Window);

	return TRUE;
    }
    else if (gd->gd_Screen = LockPubScreen (gd->gd_ScreenName))
    {
	if (gd->gd_DrInfo = GetScreenDrawInfo (gd->gd_Screen))
	{
	    if (gd->gd_VI = GetVisualInfoA (gd->gd_Screen, NULL))
	    {
		sprintf (gd->gd_WindowTitle, "%s : %s", VERS, gd->gd_CxPopKey);
		if (gd->gd_Window = openwindowtags (gd,
						    WA_Title, (ULONG) gd->gd_WindowTitle,
						    WA_PubScreen, gd->gd_Screen,
						    WA_Left, (ULONG) gd->gd_Memory.Left,
						    WA_Top, (ULONG) gd->gd_Memory.Top,
						    WA_InnerWidth, 301,
						    WA_InnerHeight, 34,
						    WA_IDCMP, IDCMP_FLAGS,
						    WA_DragBar, TRUE,
						    WA_DepthGadget, TRUE,
						    WA_CloseGadget, TRUE,
						    WA_SimpleRefresh, TRUE,
						    WA_AutoAdjust, TRUE,
						    WA_NewLookMenus, TRUE,
						    WA_Activate, TRUE,
						    TAG_DONE))
		{
		    /* Create the context gadget */
		    gd->gd_Gads[GID_ANCHOR] = CreateContext (&gd->gd_Gads[GID_CONTEXT]);

		    /* Fill in the constant information */
		    memset (&ng, 0, sizeof (struct NewGadget));
		    ng.ng_VisualInfo = gd->gd_VI;
		    ng.ng_TextAttr = &TOPAZ8;

		    /* Set Free */
		    ng.ng_LeftEdge = gd->gd_Window->BorderLeft + 4;
		    ng.ng_TopEdge = gd->gd_Window->BorderTop + 2;
		    ng.ng_Width = 95;
		    ng.ng_Height = 14;
		    ng.ng_GadgetText = "Set Free";
		    ng.ng_GadgetID = GID_SET_FREE;
		    gd->gd_Gads[GID_SET_FREE] = creategadget (gd, BUTTON_KIND, gd->gd_Gads[GID_SET_FREE - 1], &ng, TAG_DONE);

		    /* Set Alloc */
		    ng.ng_LeftEdge += ng.ng_Width + 4;
		    ng.ng_GadgetText = "Set Alloc";
		    ng.ng_GadgetID = GID_SET_ALLOC;
		    gd->gd_Gads[GID_SET_ALLOC] = creategadget (gd, BUTTON_KIND, gd->gd_Gads[GID_SET_ALLOC - 1], &ng, TAG_DONE);

		    /* Set All */
		    ng.ng_LeftEdge += ng.ng_Width + 4;
		    ng.ng_GadgetText = "Set All";
		    ng.ng_GadgetID = GID_SET_ALL;
		    gd->gd_Gads[GID_SET_ALL] = creategadget (gd, BUTTON_KIND, gd->gd_Gads[GID_SET_ALL - 1], &ng, TAG_DONE);

		    /* Check Free */
		    ng.ng_LeftEdge = gd->gd_Window->BorderLeft + 4;
		    ng.ng_TopEdge += ng.ng_Height + 2;
		    ng.ng_GadgetText = "Check Free";
		    ng.ng_GadgetID = GID_CHECK_FREE;
		    gd->gd_Gads[GID_CHECK_FREE] = creategadget (gd, BUTTON_KIND, gd->gd_Gads[GID_CHECK_FREE - 1], &ng, TAG_DONE);

		    /* Check Alloc */
		    ng.ng_LeftEdge += ng.ng_Width + 4;
		    ng.ng_GadgetText = "Check Alloc";
		    ng.ng_GadgetID = GID_CHECK_ALLOC;
		    gd->gd_Gads[GID_CHECK_ALLOC] = creategadget (gd, BUTTON_KIND, gd->gd_Gads[GID_CHECK_ALLOC - 1], &ng, TAG_DONE);

		    /* Check All */
		    ng.ng_LeftEdge += ng.ng_Width + 4;
		    ng.ng_GadgetText = "Check All";
		    ng.ng_GadgetID = GID_CHECK_ALL;
		    gd->gd_Gads[GID_CHECK_ALL] = creategadget (gd, BUTTON_KIND, gd->gd_Gads[GID_CHECK_ALL - 1], &ng, TAG_DONE);

		    /* Make sure we have gadgets */
		    if (gd->gd_Gads[GID_MAX - 1])
		    {
			/* Add the gadgets to the window and refresh it */
			AddGList (gd->gd_Window, gd->gd_Gads[GID_CONTEXT], -1, -1, NULL);
			RefreshGList (gd->gd_Gads[GID_CONTEXT], gd->gd_Window, NULL, -1);
			GT_RefreshWindow (gd->gd_Window, NULL);

			/* Bring the screen to the front */
			ScreenToFront (gd->gd_Window->WScreen);

			/* Add the menus */
			if (gd->gd_Menu = createmenus (gd, newmenu, GTMN_FullMenu, TRUE, TAG_DONE))
			    if (layoutmenus (gd, GTMN_NewLookMenus, TRUE, TAG_DONE))
				SetMenuStrip (gd->gd_Window, gd->gd_Menu);

			return TRUE;
		    }
		}
	    }
	}
    }

    CloseCWindow (gd);

    return FALSE;
}

/*****************************************************************************/

void CloseCWindow (struct GlobalData * gd)
{
    register ULONG i;

    if (gd->gd_Window)
    {
	/* Make sure we have gadgets */
	if (gd->gd_Gads[GID_MAX - 1])
	{
	    /* Remove the gadgets */
	    RemoveGList (gd->gd_Window, gd->gd_Gads[GID_CONTEXT], -1);
	}

	/* Free the gadgets */
	FreeGadgets (gd->gd_Gads[GID_CONTEXT]);

	/* Copy the position */
	gd->gd_Memory = *((struct IBox *) & gd->gd_Window->LeftEdge);

	/* Get rid of the menus */
	ClearMenuStrip (gd->gd_Window);
	FreeMenus (gd->gd_Menu);

	/* Close the window */
	CloseWindow (gd->gd_Window);
    }

    if (gd->gd_Screen)
    {
	FreeVisualInfo (gd->gd_VI);
	FreeScreenDrawInfo (gd->gd_Screen, gd->gd_DrInfo);
	UnlockPubScreen (NULL, gd->gd_Screen);
    }

    /* Clear the dangling pointers */
    for (i = 0; i < GID_MAX; i++)
	gd->gd_Gads[i] = NULL;
    gd->gd_Window = NULL;
    gd->gd_VI = NULL;
    gd->gd_DrInfo = NULL;
    gd->gd_Screen = NULL;
}
