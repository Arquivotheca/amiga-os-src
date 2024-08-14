/* clipview.c
 * Copyright © 1992 Commodore International Services Company
 * Written by David N. Junod
 *
 */

#include "clipview.h"
#include "clipview_rev.h"

char *version = VERSTAG;

/*****************************************************************************/

#define	DW(x)	;

/*****************************************************************************/

#undef	SysBase

/*****************************************************************************/

int cmd_start (void)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    LONG failureLevel = RETURN_FAIL;
    struct WBStartup *wbMsg = NULL;
    struct Process *process;
    struct GlobalData *gd;
    LONG failureCode = 0;
    ULONG snap = FALSE;
    BPTR old = NULL;
    LONG unit = 0;

    process = (struct Process *) SysBase->ThisTask;
    if (!(process->pr_CLI))
    {
	WaitPort (&process->pr_MsgPort);
	wbMsg = (struct WBStartup *) GetMsg (&process->pr_MsgPort);
    }
    if (SysBase->LibNode.lib_Version < 39)
    {
	failureCode = ERROR_INVALID_RESIDENT_LIBRARY;
	PrintF (NULL, 0, ERROR_REQUIRES_V39, NULL);
    }
    else if (gd = InitializeSystem ())
    {
	gd->gd_Process = process;

	if (wbMsg)
	{
	    struct DiskObject *dob;
	    struct WBArg *wbArg;
	    STRPTR tmp;
	    ULONG hold;

	    wbArg = wbMsg->sm_ArgList;
	    old = CurrentDir (wbArg->wa_Lock);

	    if (dob = GetDiskObject (wbArg->wa_Name))
	    {
		if (tmp = FindToolType (dob->do_ToolTypes, "PUBSCREEN"))
		{
		    strcpy (gd->gd_ScreenNameBuffer, tmp);
		    gd->gd_ScreenName = gd->gd_ScreenNameBuffer;
		}

		if (tmp = FindToolType (dob->do_ToolTypes, "CLIPUNIT"))
		{
		    if (StrToLong (tmp, &hold) > 0L)
			unit = hold;
		}
		FreeDiskObject (dob);
	    }
	}
	else
	{
	    if (gd->gd_RDArgs = ReadArgs (TEMPLATE, gd->gd_Options, NULL))
	    {
		if (!((SIGBREAKF_CTRL_C & CheckSignal (SIGBREAKF_CTRL_C))))
		{
		    /* Public screen? */
		    if (gd->gd_Options[OPT_PUBSCREEN])
		    {
			strcpy (gd->gd_ScreenNameBuffer, (STRPTR) gd->gd_Options[OPT_PUBSCREEN]);
			gd->gd_ScreenName = gd->gd_ScreenNameBuffer;
		    }

		    if (gd->gd_Options[OPT_CLIPUNIT])
		    {
			unit = *((ULONG *) gd->gd_Options[OPT_CLIPUNIT]);
		    }
		}
	    }
	}

	if (gd->gd_DisplayObject = newdtobject (gd, (APTR)unit,
						DTA_SourceType, DTST_CLIPBOARD,
						GA_Immediate, TRUE,
						GA_RelVerify, TRUE,
						DTA_TextAttr, (ULONG) & gd->gd_TextAttr,
						TAG_DONE))
	{
	    failureCode = ERROR_NO_FREE_STORE;

	    /* Lock the screen */
	    if (gd->gd_Screen = LockPubScreen (gd->gd_ScreenName))
	    {
		    if (gd->gd_VI = GetVisualInfoA (gd->gd_Screen, TAG_DONE))
		    {
			if (gd->gd_DrawInfo = GetScreenDrawInfo (gd->gd_Screen))
			{
			    /* Tell the object about the environment it will be going into */
			    frameobject (gd);

			    /* Load the window snapshot */
			    LoadSnapShot (gd, &gd->gd_Snapshot);

			    /* Get the window size */
			    GetAttr (DTA_NominalHoriz, gd->gd_DisplayObject, &gd->gd_TotHoriz);
			    GetAttr (DTA_NominalVert, gd->gd_DisplayObject, &gd->gd_TotVert);
			    if (gd->gd_TotHoriz == 0)
			    {
				gd->gd_TotHoriz = gd->gd_Snapshot.Width;
				snap = TRUE;
			    }
			    if (gd->gd_TotVert == 0)
			    {
				gd->gd_TotVert = gd->gd_Snapshot.Height;
			    }

			    /* Get a window for us to place our object in */
			    if (gd->gd_WindowObj = newobject (gd, gd->gd_WindowClass, NULL,
							      WA_Left,		gd->gd_Snapshot.Left,
							      WA_Top,		gd->gd_Snapshot.Top,
							      WA_InnerWidth,	gd->gd_TotHoriz + 4,
							      WA_InnerHeight,	gd->gd_TotVert + 2,
							      WA_PubScreen,	gd->gd_Screen,
							      WA_IDCMP,		IDCMP_FLAGS,
							      WOA_Title,	GetString (gd, TITLE_CLIPVIEW),
							      WOA_DrawInfo,	gd->gd_DrawInfo,
							      WOA_IDCMPPort,	gd->gd_IDCMPPort,
							      ICA_TARGET,	ICTARGET_IDCMP,
							      TAG_DONE))
			    {
				failureLevel = RETURN_OK;
				failureCode = 0;

				/* Get a pointer to the window */
				GetAttr (WOA_Window, gd->gd_WindowObj, (ULONG *) & gd->gd_Window);

				/* Add the gadget to the window object */
				DoMethod ((Object *) gd->gd_WindowObj, WOM_ADDVIEW, (ULONG) gd->gd_DisplayObject, NULL);

				/* Bring the screen to the front */
				ScreenToFront (gd->gd_Window->WScreen);

				/* Handle events */
				HandleEvents (gd);

				/* Free up any outstanding print requests */
				PrintComplete (gd);

				/* Remove the gadget from the window object */
				DoMethod ((Object *) gd->gd_WindowObj, WOM_REMVIEW, (ULONG) gd->gd_DisplayObject);

				/* Save the snapshot if we were text */
				if (snap)
				    SaveSnapShot (gd, gd->gd_Window);

				/* Dispose of the window object */
				DisposeObject (gd->gd_WindowObj);
			    }
			    FreeScreenDrawInfo (gd->gd_Screen, gd->gd_DrawInfo);
			}
			FreeVisualInfo (gd->gd_VI);
		    }
		    UnlockPubScreen (NULL, gd->gd_Screen);
	    }
	    DisposeDTObject (gd->gd_DisplayObject);
	}
	else
	{
	    if ((failureCode = IoErr ()) == ERROR_OBJECT_NOT_FOUND)
	    {
		failureCode = ERROR_CLIPBOARD_EMPTY;
	    }
	}

	if (old)
	    CurrentDir (old);

	FreeArgs (gd->gd_RDArgs);

	if (failureLevel)
	{
	    /* Display an error message */
	    PrintF (gd, 0, failureCode, NULL);
	}

	ShutdownSystem (gd);
    }
    else
    {
	failureCode = ERROR_NO_FREE_STORE;
	PrintF (NULL, 0, failureCode, NULL);
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

struct GlobalData * InitializeSystem (void)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct GlobalData *gd;

    if (gd = AllocVec (sizeof (struct GlobalData), MEMF_CLEAR))
    {
	gd->gd_SysBase = SysBase;
	gd->gd_Going   = TRUE;
	gd->gd_Trigger.MethodID = DTM_TRIGGER;

	DOSBase       = OpenLibrary ("dos.library", 39);
	IconBase      = OpenLibrary ("icon.library", 39);
	IntuitionBase = OpenLibrary ("intuition.library", 39);
	GfxBase       = OpenLibrary ("graphics.library", 39);
	UtilityBase   = OpenLibrary ("utility.library", 39);
	GadToolsBase  = OpenLibrary ("gadtools.library", 39);
	LayersBase    = OpenLibrary ("layers.library", 39);
	AslBase       = OpenLibrary ("asl.library", 37);

	if (LocaleBase = OpenLibrary ("locale.library", 38))
	    gd->gd_Catalog = OpenCatalogA (NULL, "clipview.catalog", NULL);

	if (DataTypesBase = OpenLibrary ("datatypes.library", 39))
	{
	    if (gd->gd_WindowClass = initWindowClass (gd))
	    {
		if (gd->gd_IDCMPPort = CreateMsgPort ())
		{
#undef	GfxBase
		    gd->gd_TextFont = ((struct GfxBase *) gd->gd_GfxBase)->DefaultFont;
#define	GfxBase	gd->gd_GfxBase

		    gd->gd_TextAttr.ta_Name  = gd->gd_TextFont->tf_Message.mn_Node.ln_Name;
		    gd->gd_TextAttr.ta_YSize = gd->gd_TextFont->tf_YSize;
		    gd->gd_TextAttr.ta_Style = gd->gd_TextFont->tf_Style;
		    gd->gd_TextAttr.ta_Flags = gd->gd_TextFont->tf_Flags;

		    return (gd);
		}
	    }
	}
	ShutdownSystem (gd);
    }
    return (NULL);
}

/*****************************************************************************/

VOID ShutdownSystem (struct GlobalData * gd)
{
    if (gd)
    {
	if (LocaleBase)
	{
	    CloseCatalog (gd->gd_Catalog);
	    CloseLibrary (LocaleBase);
	}

	DeleteMsgPort (gd->gd_IDCMPPort);
	freeWindowClass (gd->gd_WindowClass);

	CloseLibrary (DataTypesBase);
	CloseLibrary (AslBase);
	CloseLibrary (GadToolsBase);
	CloseLibrary (UtilityBase);
	CloseLibrary (GfxBase);
	CloseLibrary (IntuitionBase);
	CloseLibrary (IconBase);
	CloseLibrary (DOSBase);

	FreeVec (gd);
    }
}

/*****************************************************************************/

void ProcessCommand (struct GlobalData * gd, ULONG id, struct IntuiMessage * imsg)
{
    struct Window *win = gd->gd_Window;
    struct DTSpecialInfo *si = NULL;
    struct dtGeneral dtg;

    if (!id)
	return;

    switch (id)
    {
	case MMC_SAVEAS:
	    if (gd->gd_DisplayObject)
	    {
		SaveObject (gd, DTWM_RAW);
	    }
	    break;

	case MMC_PRINT:
	    if (gd->gd_DisplayObject)
	    {
		PrintObject (gd);
	    }
	    break;

	case MMC_ABOUT:
	    if (gd->gd_DisplayObject)
	    {
		AboutObject (gd);
	    }
	    break;

	case MMC_QUIT:
	    gd->gd_Going = FALSE;
	    break;

	case MMC_MARK:
	    if (gd->gd_DisplayObject)
	    {
		si = (struct DTSpecialInfo *) G(gd->gd_DisplayObject)->SpecialInfo;
		si->si_Flags |= DTSIF_DRAGSELECT;
		SetBlockPointer (gd, win);
	    }
	    break;

	case MMC_COPY:
	    dtg.MethodID = DTM_COPY;
	    DoDTMethodA (gd->gd_DisplayObject, win, NULL, &dtg);
	    break;
    }
}

/*****************************************************************************/

VOID AboutObject (struct GlobalData * gd)
{
    struct DataType *dtn;
    struct EasyStruct es;

    getdtattrs (gd, gd->gd_DisplayObject, DTA_DataType, (ULONG)&dtn, TAG_DONE);

    es.es_StructSize = sizeof (struct EasyStruct);
    es.es_Flags = 0;
    es.es_Title = GetString (gd, TITLE_CLIPVIEW);
    es.es_TextFormat = "ClipView %ld.%ld (%s)\nWritten by David N. Junod\n\nContents Type: %s %s";
    es.es_GadgetFormat = GetString (gd, LABEL_CONTINUE);

    easyrequest (gd, &es, VERSION, REVISION, DATE, dtn->dtn_Header->dth_Name, GetDTString (dtn->dtn_Header->dth_GroupID));
}

/*****************************************************************************/

void HandleEvents (struct GlobalData * gd)
{
    struct MenuItem *menuitem;
    struct IntuiMessage *imsg;
    struct TagItem *attrs;
    struct TagItem *tag;
    LONG errorNum;
    ULONG sigw;
    ULONG sigr;
    ULONG sigp;
    UWORD id;

    sigw = 1L << gd->gd_IDCMPPort->mp_SigBit;
    sigp = NULL;

    while (gd->gd_Going)
    {
	if (LAYERREFRESH & gd->gd_Window->WLayer->Flags)
	{
	    BeginRefresh (gd->gd_Window);
	    EndRefresh (gd->gd_Window, TRUE);
	}
	sigp = NULL;
	if (gd->gd_PrintWin)
	{
	    sigp = 1L << gd->gd_PrintWin->UserPort->mp_SigBit;
	}
	sigr = Wait ((1L << gd->gd_Window->UserPort->mp_SigBit) | sigw | sigp);

	if ((sigr & sigp) && gd->gd_PrintWin)
	{
	    if (SysReqHandler (gd->gd_PrintWin, NULL, FALSE) == 0)
	    {
		DoMethod (gd->gd_DisplayObject, DTM_ABORTPRINT, NULL);
	    }
	}

	while (imsg = (struct IntuiMessage *) GetMsg (gd->gd_Window->UserPort))
	{
	    switch (imsg->Class)
	    {
		case IDCMP_MENUPICK:
		    id = imsg->Code;
		    while ((id != MENUNULL) && gd->gd_Going)
		    {
			menuitem = ItemAddress (imsg->IDCMPWindow->MenuStrip, id);
			ProcessCommand (gd, (ULONG) MENU_USERDATA (menuitem), imsg);
			id = menuitem->NextSelect;
		    }
		    break;

		case IDCMP_CLOSEWINDOW:
		    gd->gd_Going = FALSE;
		    break;

		case IDCMP_VANILLAKEY:
		    switch (imsg->Code)
		    {
			case '/':
			    gd->gd_Trigger.dtt_Function = STM_RETRACE;
			    DoDTMethodA (gd->gd_DisplayObject, gd->gd_Window, NULL, &gd->gd_Trigger);
			    break;

			case 9:
			    gd->gd_Trigger.dtt_Function = STM_NEXT_FIELD;
			    DoDTMethodA (gd->gd_DisplayObject, gd->gd_Window, NULL, &gd->gd_Trigger);
			    break;

			case 13:
			    gd->gd_Trigger.dtt_Function = STM_ACTIVATE_FIELD;
			    DoDTMethodA (gd->gd_DisplayObject, gd->gd_Window, NULL, &gd->gd_Trigger);
			    break;

			case '<':
			    gd->gd_Trigger.dtt_Function = STM_BROWSE_PREV;
			    DoDTMethodA (gd->gd_DisplayObject, gd->gd_Window, NULL, &gd->gd_Trigger);
			    break;

			case '>':
			    gd->gd_Trigger.dtt_Function = STM_BROWSE_NEXT;
			    DoDTMethodA (gd->gd_DisplayObject, gd->gd_Window, NULL, &gd->gd_Trigger);
			    break;
		    }
		    break;

		case IDCMP_IDCMPUPDATE:
		    attrs = (struct TagItem *) imsg->IAddress;

		    if (tag = FindTagItem (DTA_ErrorLevel, attrs))
		    {
			errorNum = (LONG) GetTagData (DTA_ErrorNumber, NULL, attrs);
			PrintF (gd, 2, errorNum, (STRPTR) GetTagData (DTA_ErrorString,NULL,attrs));
		    }

		    if (tag = FindTagItem (DTA_PrinterStatus, (struct TagItem *) imsg->IAddress))
		    {
			PrintComplete (gd);

			if (tag->ti_Data)
			{
			    PrintF (gd, 0, 990 + tag->ti_Data, NULL);
			}
		    }
		    if (tag = FindTagItem (DTA_Busy, (struct TagItem *) imsg->IAddress))
		    {
			if (tag->ti_Data)
			    setwindowpointer (gd, gd->gd_Window, WA_BusyPointer, TRUE, TAG_DONE);
			else
			    setwindowpointer (gd, gd->gd_Window, WA_Pointer, NULL, TAG_DONE);
		    }

		    if (tag = FindTagItem (DTA_Title, attrs))
			setattrs (gd, gd->gd_WindowObj, WOA_Title, tag->ti_Data, TAG_DONE);

		    if (tag = FindTagItem (DTA_Sync, attrs))
			setattrs (gd, gd->gd_WindowObj, WOA_Sync, tag->ti_Data, TAG_DONE);

		    if (tag = FindTagItem (TDTA_WordSelect, attrs))
		    {
			if (tag->ti_Data)
			{
			    DW (kprintf ("TDTA_WordSelect, \"%s\"\n", tag->ti_Data));
			}
		    }
		    break;
	    }

	    ReplyMsg ((struct Message *) imsg);
	}
    }
}

/*****************************************************************************/

/* Martin Taillefer's block pointer */
static UWORD chip Block_sdata[] =
{
    0x0000, 0x0000,		/* reserved, must be NULL */

    0x0000, 0x0100,
    0x0100, 0x0280,
    0x0380, 0x0440,
    0x0100, 0x0280,
    0x0100, 0x0ee0,
    0x0000, 0x2828,
    0x2008, 0x5834,
    0x783c, 0x8002,
    0x2008, 0x5834,
    0x0000, 0x2828,
    0x0100, 0x0ee0,
    0x0100, 0x0280,
    0x0380, 0x0440,
    0x0100, 0x0280,
    0x0000, 0x0100,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,

    0x0000, 0x0000,		/* reserved, must be NULL */
};

VOID SetBlockPointer (struct GlobalData * gd, struct Window * win)
{
    SetPointer (win, Block_sdata, 32, 16, -8, -7);
}

/*****************************************************************************/

ULONG frameobject (struct GlobalData * gd)
{
    struct FrameInfo *fri = &gd->gd_FrameInfo;
    struct DisplayInfo di;
    struct dtFrameBox dtf;
    ULONG modeid;

    /* Get the display information */
    modeid = GetVPModeID (&(gd->gd_Screen->ViewPort));
    GetDisplayInfoData (NULL, (APTR) & di, sizeof (struct DisplayInfo), DTAG_DISP, modeid);

    /* Fill in the frame info */
    fri->fri_PropertyFlags = di.PropertyFlags;
    fri->fri_Resolution = *(&di.Resolution);
    fri->fri_RedBits = di.RedBits;
    fri->fri_GreenBits = di.GreenBits;
    fri->fri_BlueBits = di.BlueBits;
    fri->fri_Dimensions.Width = gd->gd_Screen->Width;
    fri->fri_Dimensions.Height = gd->gd_Screen->Height;
    fri->fri_Dimensions.Depth = gd->gd_Screen->BitMap.Depth;
    fri->fri_Screen = gd->gd_Screen;
    fri->fri_ColorMap = gd->gd_Screen->ViewPort.ColorMap;

    /* Send the message */
    dtf.MethodID = DTM_FRAMEBOX;
    dtf.dtf_ContentsInfo = &gd->gd_FrameInfo;
    dtf.dtf_SizeFrameInfo = sizeof (struct FrameInfo);
    dtf.dtf_FrameFlags = FRAMEF_SPECIFY;
    return (DoDTMethodA (gd->gd_DisplayObject, gd->gd_Window, NULL, &dtf));
}
