/* dtdesc.c
 *
 */

#include "dtdesc.h"
#include "dtdesc_rev.h"

/*****************************************************************************/

char *version = VERSTAG;

struct Library *DOSBase, *IntuitionBase, *GfxBase, *GadToolsBase, *IconBase, *WorkbenchBase;
struct Library *IFFParseBase, *AslBase, *UtilityBase;

/*****************************************************************************/

static struct NewMenu newmenu[] =
{
    {NM_TITLE, "Project",},
    {NM_ITEM, "New", "N", NULL, NULL, V (MMC_NEW)},
    {NM_ITEM, "Open...", "O", NULL, NULL, V (MMC_OPEN)},
    {NM_ITEM, NM_BARLABEL, NULL, NULL, NULL, NULL},
    {NM_ITEM, "Save As...", "A", NULL, NULL, V (MMC_SAVEAS)},
    {NM_ITEM, NM_BARLABEL, NULL, NULL, NULL, NULL},
    {NM_ITEM, "Quit", "Q", NULL, NULL, V (MMC_QUIT)},

    {NM_TITLE, "Group",},
    {NM_ITEM, "System", "1", CHECKIT | CHECKED, ~1, V (MMC_SYSTEM_G)},
    {NM_ITEM, "Text", "2", CHECKIT, ~2, V (MMC_TEXT_G)},
    {NM_ITEM, "Document", "3", CHECKIT, ~4, V (MMC_DOCUMENT_G)},
    {NM_ITEM, "Sound", "4", CHECKIT, ~8, V (MMC_SOUND_G)},
    {NM_ITEM, "Instrument", "5", CHECKIT, ~16, V (MMC_INSTRUMENT_G)},
    {NM_ITEM, "Music", "6", CHECKIT, ~32, V (MMC_MUSIC_G)},
    {NM_ITEM, "Picture", "7", CHECKIT, ~64, V (MMC_PICTURE_G)},
    {NM_ITEM, "Animation", "8", CHECKIT, ~128, V (MMC_ANIMATION_G)},
    {NM_ITEM, "Movie", "9", CHECKIT, ~256, V (MMC_MOVIE_G)},

    {NM_TITLE, "Extras",},
    {NM_ITEM, "Load Samples...", "L", NULL, NULL, V (MMC_LOAD)},
    {NM_ITEM, "Load Function...", "F", NULL, NULL, V (MMC_LOADFUNC)},
    {NM_ITEM, NM_BARLABEL, NULL, NULL, NULL, NULL},
    {NM_ITEM, "View", NULL, NULL, NULL, NULL},
    {NM_SUB, "Graphic", "G", CHECKIT, ~1, V (MMC_GRAPHIC)},
    {NM_SUB, "Text", "T", CHECKIT | CHECKED, ~2, V (MMC_TEXT)},
    {NM_SUB, "Hex", "H", CHECKIT, ~4, V (MMC_HEX)},

    {NM_END,},
};

/*****************************************************************************/

    int cmd_start (void)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    LONG failureLevel = RETURN_FAIL;
    struct WBStartup *wbMsg = NULL;
    struct Process *process;
    LONG failureCode;

    char FontName[] = "topaz.font";
    struct AppInfo *ai;
    ULONG signals;
    UWORD i;

    /* Handle the startup message */
    process = (struct Process *) SysBase->ThisTask;
    if (!(process->pr_CLI))
    {
	WaitPort (&process->pr_MsgPort);
	wbMsg = (struct WBStartup *) GetMsg (&process->pr_MsgPort);
    }

    failureCode = ERROR_INVALID_RESIDENT_LIBRARY;

    /* Make sure we are running in the correct OS */
    if (SysBase->LibNode.lib_Version >= 39)
    {
	/* Open all the ROM libraries */
	DOSBase = OpenLibrary ("dos.library", 39);
	IntuitionBase = OpenLibrary ("intuition.library", 39);
	GadToolsBase = OpenLibrary ("gadtools.library", 39);
	GfxBase = OpenLibrary ("graphics.library", 39);
	IconBase = OpenLibrary ("icon.library", 39);
	WorkbenchBase = OpenLibrary ("workbench.library", 39);
	UtilityBase = OpenLibrary ("utility.library", 39);

	/* Open the disk libraries */
	AslBase = OpenLibrary ("asl.library", 37);
	IFFParseBase = OpenLibrary ("iffparse.library", 37);

	/* Make sure they opened */
	if (AslBase && IFFParseBase)
	{
	    failureCode = ERROR_NO_FREE_STORE;
	    if (ai = AllocMem (sizeof (struct AppInfo), MEMF_CLEAR))
	    {
		ai->ai_Flags = AIF_TEXT;
		ai->ai_Done = FALSE;
		strcpy (ai->ai_Dir, "DEVS:DataTypes");

		NewList (&ai->ai_Files);

		ai->ai_BufSize = FNBUF_SIZE;
		for (i = 0; i < ai->ai_BufSize; i++)
		    ai->ai_SBuffer[i] = ai->ai_DBuffer[i] = (-1);

		ai->ai_TextAttr.ta_Name = FontName;
		ai->ai_TextAttr.ta_YSize = TOPAZ_EIGHTY;

		if (ai->ai_Font = OpenFont (&ai->ai_TextAttr))
		{
		    if (ai->ai_Screen = LockPubScreen (NULL))
		    {
			if (ai->ai_VI = GetVisualInfo (ai->ai_Screen, TAG_DONE))
			{
			    if (ai->ai_AppWinPort = CreateMsgPort ())
			    {
				/* Get the signal bits that we wait on */
				ai->ai_SigA = 1L << ai->ai_AppWinPort->mp_SigBit;

				/* Indicate success */
				failureLevel = RETURN_OK;
				failureCode = 0;

				/* Run the application */
				AppWindowE (ai, DISPLAY);

				while (!ai->ai_Done)
				{
				    signals = Wait (ai->ai_WinSig | ai->ai_SigA | SIGBREAKF_CTRL_C);

				    if (signals & ai->ai_WinSig)
				    {
					AppWindowIDCMP (ai);
				    }

				    if (signals & SIGBREAKF_CTRL_C)
				    {
					ai->ai_Done = TRUE;
				    }

				    if (signals & ai->ai_SigA)
				    {
					struct AppMessage *am;

					GT_SetGadgetAttrs (ai->ai_WinGad[AIG_FILELIST], ai->ai_Window, NULL, GTLV_Labels, ~0, TAG_DONE);

					/* activate the window */
					ActivateWindow (ai->ai_Window);

					while (am = (struct AppMessage *) GetMsg (ai->ai_AppWinPort))
					{
					    AddWBArgs (ai, am->am_ArgList, am->am_NumArgs);

					    ReplyMsg ((struct Message *) am);
					}

					GT_SetGadgetAttrs (ai->ai_WinGad[AIG_FILELIST], ai->ai_Window, NULL, GTLV_Labels, &ai->ai_Files, TAG_DONE);
				    }
				}

				if (ai->ai_Window)
				    AppRemoveWindow (ai);
				removeallfunc (ai);

				DeleteMsgPort (ai->ai_AppWinPort);
			    }
			    FreeVisualInfo (ai->ai_VI);
			}
			UnlockPubScreen (NULL, ai->ai_Screen);
		    }
		    CloseFont (ai->ai_Font);
		}
		FreeMem (ai, sizeof (struct AppInfo));
	    }
	}

	CloseLibrary (IFFParseBase);
	CloseLibrary (AslBase);
	CloseLibrary (UtilityBase);
	CloseLibrary (WorkbenchBase);
	CloseLibrary (IconBase);
	CloseLibrary (GfxBase);
	CloseLibrary (GadToolsBase);
	CloseLibrary (IntuitionBase);
	CloseLibrary (DOSBase);
    }

    if (wbMsg)
    {
	Forbid ();
	ReplyMsg ((struct Message *) wbMsg);
    }

    process->pr_Result2 = failureCode;
    return (failureLevel);
}

/*****************************************************************************/

BOOL AppWindowIDCMP (struct AppInfo * ai)
{
    UWORD id;

    while (ai->ai_IMsg = GT_GetIMsg (ai->ai_Window->UserPort))
    {
	switch (ai->ai_IMsg->Class)
	{
	    case IDCMP_VANILLAKEY:
		switch (ai->ai_IMsg->Code)
		{
		    case 'q':
		    case 'Q':
			ai->ai_Done = TRUE;
			break;
		}
		break;

	    case IDCMP_MOUSEMOVE:
		FindPoint (ai);
		break;

	    case IDCMP_MOUSEBUTTONS:
		if (ai->ai_IMsg->Code == SELECTDOWN)
		{
		    if (FindPoint (ai))
		    {
			ai->ai_Window->Flags |= (WFLG_RMBTRAP | WFLG_REPORTMOUSE);
			ai->ai_Flags |= AIF_CHANGED;
		    }
		    else
		    {
			ai->ai_Flags |= AIF_ABORTED;
		    }
		}
		else if (ai->ai_IMsg->Code == MENUDOWN)
		{
		    ai->ai_Window->Flags &= ~(WFLG_RMBTRAP | WFLG_REPORTMOUSE);
		    ai->ai_Flags |= AIF_ABORTED;
		}
		else if (ai->ai_IMsg->Code == SELECTUP)
		{
		    ai->ai_Window->Flags &= ~(WFLG_RMBTRAP | WFLG_REPORTMOUSE);
		    if (!(ai->ai_Flags & AIF_ABORTED))
		    {
		    }
		    ai->ai_Flags &= ~(AIF_ABORTED | AIF_DOWN | AIF_SET);
		}
		break;

	    case IDCMP_GADGETUP:
		id = ((struct Gadget *) ai->ai_IMsg->IAddress)->GadgetID;
		ProcessCommand (ai, id);
		break;

	    case IDCMP_GADGETDOWN:
		id = ((struct Gadget *) ai->ai_IMsg->IAddress)->GadgetID;
		ProcessCommand (ai, id);
		break;

	    case IDCMP_MENUPICK:
		id = ai->ai_IMsg->Code;
		while ((id != MENUNULL) && (!ai->ai_Done))
		{
		    ai->ai_MenuItem = ItemAddress (ai->ai_IMsg->IDCMPWindow->MenuStrip, id);
		    ProcessCommand (ai, (ULONG) MENU_USERDATA (ai->ai_MenuItem));
		    id = ai->ai_MenuItem->NextSelect;
		}
		break;

	    case IDCMP_CLOSEWINDOW:
		ai->ai_Done = TRUE;
		break;

	    case IDCMP_REFRESHWINDOW:
		GT_BeginRefresh (ai->ai_Window);
		GT_EndRefresh (ai->ai_Window, TRUE);
		break;

	}

	GT_ReplyIMsg (ai->ai_IMsg);
    }

    if (!(ai->ai_WinFlags & TMWF_OPEN) && ai->ai_Window)
    {
	AppRemoveWindow (ai);
    }
    return (ai->ai_Done);
}

/*****************************************************************************/

VOID SetAppAttrs (struct AppInfo * ai)
{

    FindSelected (ai, NULL, MIN (ai->ai_CurNum, (ai->ai_NumFiles - 1)));

    GT_SetGadgetAttrs (ai->ai_WinGad[AIG_REMOVE], ai->ai_Window, NULL,
		       GA_Disabled, ((ai->ai_CurFile) ? FALSE : TRUE),
		       TAG_DONE);

    GT_SetGadgetAttrs (ai->ai_WinGad[AIG_REMOVEALL], ai->ai_Window, NULL,
		       GA_Disabled, ((ai->ai_NumFiles) ? FALSE : TRUE),
		       TAG_DONE);

    GT_SetGadgetAttrs (ai->ai_WinGad[AIG_FILELIST], ai->ai_Window, NULL,
		       GTLV_Labels, &ai->ai_Files,
		       GTLV_Selected, ai->ai_CurNum,
		       TAG_DONE);
}

/*****************************************************************************/

BOOL AppWindowE (struct AppInfo * ai, UBYTE command)
{
    struct NewGadget ng;
    UWORD height;

    switch (command)
    {
	case DISPLAY:
	    if (ai->ai_Window)
	    {
		WindowToFront (ai->ai_Window);
		ActivateWindow (ai->ai_Window);
	    }
	    else
	    {
		ai->ai_WinGad[AIG_LASTGADGET] = CreateContext (&ai->ai_WinGad[AIG_CONTEXT]);

		ng.ng_VisualInfo = ai->ai_VI;
		ng.ng_TextAttr = &ai->ai_TextAttr;

		ng.ng_LeftEdge = 351;
		ng.ng_TopEdge = 16 + ai->ai_Screen->BarHeight;
		ng.ng_Width = 186;
		ng.ng_Height = ai->ai_TextAttr.ta_YSize + 6;
		ng.ng_Flags = PLACETEXT_LEFT;

		ng.ng_GadgetText = "File Type";
		ng.ng_GadgetID = MMC_DATATYPE;
		ai->ai_WinGad[AIG_DATATYPE] = CreateGadget (STRING_KIND, ai->ai_WinGad[AIG_DATATYPE - 1], &ng,
							    GTST_MaxChars, 32,
							    GTST_String, "",
							    TAG_DONE);

		ng.ng_TopEdge += ng.ng_Height + 1;
		ng.ng_GadgetText = "Base Name";
		ng.ng_GadgetID = MMC_BASENAME;
		ai->ai_WinGad[AIG_BASENAME] = CreateGadget (STRING_KIND, ai->ai_WinGad[AIG_BASENAME - 1], &ng,
							    GTST_MaxChars, 32,
							    GTST_String, "",
							    TAG_DONE);

		ng.ng_TopEdge += ng.ng_Height + 1;
		ng.ng_GadgetText = "Name Pattern";
		ng.ng_GadgetID = MMC_PATTERN;
		ai->ai_WinGad[AIG_PATTERN] = CreateGadget (STRING_KIND, ai->ai_WinGad[AIG_PATTERN - 1], &ng,
							   GTST_MaxChars, 22,
							   GTST_String, "#?",
							   TAG_DONE);

		ng.ng_TopEdge += ng.ng_Height + 1;
		ng.ng_GadgetText = "Function";
		ng.ng_GadgetID = MMC_FUNCTION;
		ai->ai_WinGad[AIG_FUNCTION] = CreateGadget (STRING_KIND, ai->ai_WinGad[AIG_FUNCTION - 1], &ng,
							    GTST_MaxChars, 256,
							    GTST_String, "",
							    TAG_DONE);

		ng.ng_TopEdge += ng.ng_Height + 1;
		ng.ng_Width = 26;
		ng.ng_Height = 11;
		ng.ng_GadgetText = "Case Sensitive?";
		ng.ng_GadgetID = MMC_CASE;
		ai->ai_WinGad[AIG_CASE] = CreateGadget (CHECKBOX_KIND, ai->ai_WinGad[AIG_CASE - 1], &ng,
							GTCB_Checked, FALSE,
							TAG_DONE);

		ng.ng_TopEdge += ng.ng_Height + 1;
		ng.ng_Width = 48;
		ng.ng_Height = ai->ai_TextAttr.ta_YSize + 6;
		ng.ng_GadgetText = "Priority";
		ng.ng_GadgetID = MMC_PRIORITY;
		ai->ai_WinGad[AIG_PRIORITY] = CreateGadget (INTEGER_KIND, ai->ai_WinGad[AIG_PRIORITY - 1], &ng,
							    GTIN_MaxChars, 3L,
							    TAG_DONE);

		ng.ng_TopEdge += ng.ng_Height + 1;
		ng.ng_Width = 100;
		ng.ng_Height = ai->ai_TextAttr.ta_YSize + 4;
		ng.ng_GadgetText = "Type";
		ng.ng_GadgetID = 0;
		ai->ai_WinGad[AIG_TYPE] = CreateGadget (TEXT_KIND, ai->ai_WinGad[AIG_TYPE - 1], &ng,
							GTTX_Text, "",
							GTTX_Border, 1,
							TAG_DONE);

		height = ng.ng_TopEdge + ng.ng_Height + 4 + VB_HEIGHT + 4 - ai->ai_Screen->BarHeight - 1;

		ng.ng_LeftEdge = 10;
		ng.ng_TopEdge = 16 + ai->ai_Screen->BarHeight;
		ng.ng_Width = 208;
		ng.ng_Height = height - (16 + 2 + ai->ai_TextAttr.ta_YSize + 6 + 4 - 1);
		ng.ng_Flags = PLACETEXT_ABOVE;
		ng.ng_GadgetText = "Sample Files";
		ng.ng_GadgetID = MMC_SAMPLE;
		ai->ai_WinGad[AIG_FILELIST] = CreateGadget (LISTVIEW_KIND, ai->ai_WinGad[AIG_FILELIST - 1], &ng,
							    GTLV_Labels, &ai->ai_Files,
							    GTLV_Top, 0,
							    GTLV_ShowSelected, 0,
							    TAG_DONE);

		ng.ng_TopEdge += ng.ng_Height + 1;
		ng.ng_Width = 100;
		ng.ng_Height = ai->ai_TextAttr.ta_YSize + 6;
		ng.ng_Flags = PLACETEXT_IN;
		ng.ng_GadgetText = "Remove";
		ng.ng_GadgetID = MMC_REMOVE;
		ai->ai_WinGad[AIG_REMOVE] = CreateGadget (BUTTON_KIND, ai->ai_WinGad[AIG_REMOVE - 1], &ng,
							  GA_Disabled, TRUE,
							  TAG_DONE);

		ng.ng_LeftEdge = 118;
		ng.ng_Flags = PLACETEXT_IN;
		ng.ng_GadgetText = "Remove All";
		ng.ng_GadgetID = MMC_REMOVEALL;
		ai->ai_WinGad[AIG_REMOVEALL] = CreateGadget (BUTTON_KIND, ai->ai_WinGad[AIG_REMOVEALL - 1], &ng,
							     GA_Disabled, TRUE,
							     TAG_DONE);

		if (ai->ai_WinGad[AIG_NUM - 1] &&
		    (ai->ai_Window = OpenWindowTags (NULL,
						     WA_PubScreen, ai->ai_Screen,
						     WA_Title, "Define File Type",
						     WA_IDCMP, IDCMP_FLAGS,
						     WA_Gadgets, ai->ai_WinGad[AIG_CONTEXT],
						     WA_InnerWidth, 537,
						     WA_InnerHeight, height,
						     WA_DragBar, 1,
						     WA_DepthGadget, 1,
						     WA_CloseGadget, 1,
						     WA_Activate, 1,
						     WA_NewLookMenus, TRUE,
						     TAG_DONE)))
		{
		    SetFont (ai->ai_Window->RPort, ai->ai_Font);

		    if (ai->ai_Menu = CreateMenus (newmenu, TAG_DONE))
		    {
			if (LayoutMenus (ai->ai_Menu, ai->ai_VI, GTMN_NewLookMenus, TRUE, TAG_DONE))
			{
			    SetMenuStrip (ai->ai_Window, ai->ai_Menu);
			}
		    }

		    ai->ai_AppWin = AddAppWindowA (0, 0, ai->ai_Window, ai->ai_AppWinPort, NULL);

		    GT_RefreshWindow (ai->ai_Window, NULL);

		    ai->ai_ViewBox.Left = VB_LEFT;
		    ai->ai_ViewBox.Top = ai->ai_WinGad[AIG_TYPE]->TopEdge + ai->ai_WinGad[AIG_TYPE]->Height + 4;
		    ai->ai_ViewBox.Width = VB_WIDTH;
		    ai->ai_ViewBox.Height = VB_HEIGHT;

		    DrawBevelBox (ai->ai_Window->RPort,
				  ai->ai_ViewBox.Left, ai->ai_ViewBox.Top,
				  ai->ai_ViewBox.Width, ai->ai_ViewBox.Height,
				  GT_VisualInfo, ai->ai_VI,
				  TAG_DONE);

		    ai->ai_WinSig = 1L << ai->ai_Window->UserPort->mp_SigBit;
		    ai->ai_WinFlags |= TMWF_OPEN;
		}
		else
		{
		    goto cleanexit;
		}
	    }
	    break;

	case REMOVE:
	    ai->ai_WinFlags &= ~TMWF_OPEN;
	    break;
    }

    return (TRUE);

  cleanexit:
    AppRemoveWindow (ai);

    return (FALSE);
}

/*****************************************************************************/

VOID AppRemoveWindow (struct AppInfo * ai)
{

    if (ai->ai_AppWin)
    {
	RemoveAppWindow (ai->ai_AppWin);
	ai->ai_AppWin = NULL;
    }

    if (ai->ai_Window)
    {
	if (ai->ai_Window->MenuStrip)
	{
	    ClearMenuStrip (ai->ai_Window);
	}

	if (ai->ai_Menu)
	{
	    FreeMenus (ai->ai_Menu);
	    ai->ai_Menu = NULL;
	}

	CloseWindow (ai->ai_Window);
	ai->ai_Window = NULL;
    }

    if (ai->ai_WinGad[AIG_CONTEXT])
    {
	FreeGadgets (ai->ai_WinGad[AIG_CONTEXT]);
	ai->ai_WinGad[AIG_CONTEXT] = NULL;
    }
}
