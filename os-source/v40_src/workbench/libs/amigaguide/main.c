/* main.c
 *
 */

#include <graphics/gfxbase.h>
#include <datatypes/datatypesclass.h>
#include "amigaguidebase.h"

/*****************************************************************************/

#include "amigaguide_rev.h"

/*****************************************************************************/

extern STRPTR ARexxPortName (struct AmigaGuideLib * ag, AREXXCONTEXT ac);

/*****************************************************************************/

static void GetScreenName (struct AmigaGuideLib *ag, struct Screen *scr, STRPTR buffer)
{
    struct PubScreenNode *psn;
    BOOL got = FALSE;
    struct List *lh;
    struct Node *ln;

    /* Clear out any old name */
    memset (buffer, 0, MAXPUBSCREENNAME);
    strncpy (buffer, scr->DefaultTitle, MAXPUBSCREENNAME);

    /* Search through the public list */
    if (lh = LockPubScreenList ())
    {
	for (ln = (struct Node *) lh->lh_Head; ln->ln_Succ && (got == FALSE); ln = ln->ln_Succ)
	{
	    psn = (struct PubScreenNode *) ln;
	    if (psn->psn_Screen == scr)
	    {
		strcpy (buffer, ln->ln_Name);
		got = TRUE;
	    }
	}

	UnlockPubScreenList ();
    }
}

/*****************************************************************************/

void MainLoop (struct AmigaGuideLib *ag, struct Client * cl)
{
    struct NewAmigaGuide *nag = cl->cl_NAG;
    struct Screen *scr = NULL;
    BOOL error = FALSE;
    BPTR old;

    /* Clear the error return value */
    SetIoErr (0);

    if (nag->nag_ClientPort && ag->ag_RexxSysBase)
    {
	if (!(cl->cl_RexxHandle = InitARexx (ag, nag->nag_ClientPort, "guide", (LONG) (cl->cl_Flags & AGF_UNIQUE))))
	{
	    cl->cl_ErrorLevel = RETURN_FAIL;
	    cl->cl_ErrorNumber = ERR_CANT_CREATE_PORT;
	    error = TRUE;
	}
    }

    if ((error == FALSE) && CreateMsgPorts (ag, cl))
    {
	cl->cl_Going = TRUE;
	cl->cl_Trigger.MethodID = DTM_TRIGGER;

	if (!(cl->cl_Screen = nag->nag_Screen))
	    scr = cl->cl_Screen = LockPubScreen (nag->nag_PubScreen);

	/* We have to have a screen to open on! */
	if (cl->cl_Screen)
	{
	    GetScreenName (ag, cl->cl_Screen, cl->cl_ScreenNameBuffer);

	    /* Get the default font */
#undef	GfxBase
	    cl->cl_TFont = ((struct GfxBase *) ag->ag_GfxBase)->DefaultFont;
#define	GfxBase	ag->ag_GfxBase
	    cl->cl_TAttr.ta_Name  = cl->cl_TFont->tf_Message.mn_Node.ln_Name;
	    cl->cl_TAttr.ta_YSize = cl->cl_TFont->tf_YSize;
	    cl->cl_TAttr.ta_Style = cl->cl_TFont->tf_Style;
	    cl->cl_TAttr.ta_Flags = cl->cl_TFont->tf_Flags;

	    /* Get the GadTools visual information */
	    if (cl->cl_VI = GetVisualInfoA (cl->cl_Screen, NULL))
	    {
		/* Get the Intuition screen draw information */
		if (cl->cl_DrInfo = GetScreenDrawInfo (cl->cl_Screen))
		{
		    /* See if we have a lock on a directory */
		    if (nag->nag_Lock)
			old = CurrentDir (DupLock(nag->nag_Lock));
		    else
			old = SetCurrentDirELVO (ag, NULL, nag->nag_Name);

		    if (IoErr () == 0)
		    {
			if (cl->cl_DataObject = NewDTObject (nag->nag_Name,
							     DTA_ARexxPortName,	(ULONG) ARexxPortName (ag, cl->cl_RexxHandle),
							     DTA_TextAttr,	(ULONG) & cl->cl_TAttr,
							     DTA_NodeName,	(ULONG) nag->nag_Node,
							     DTA_TopVert,	nag->nag_Line,
							     GA_Immediate,	TRUE,
							     GA_RelVerify,	TRUE,
							     AGA_HelpGroup,	cl->cl_HelpGroup,
							     AGA_HomeDir,	old,
							     TAG_DONE))
			{
			    if (cl->cl_Process || OpenAGWindow (ag, cl))
			    {
				struct AmigaGuideMsg *agm;

				cl->cl_ErrorLevel = cl->cl_ErrorNumber = 0L;

				/* Allocate a message to send */
				if (cl->cl_AsyncPort)
				{
				    if (agm = AllocPVec (ag, ag->ag_MemoryPool, AGMSIZE))
				    {
					/* Initialize the message */
					agm->agm_Type = ActiveToolID;
					agm->agm_Msg.mn_Node.ln_Type = NT_MESSAGE;
					agm->agm_Msg.mn_ReplyPort = cl->cl_ChildPort;
					agm->agm_Msg.mn_Length = AGMSIZE;

					/* Send the message */
					PutMsg (cl->cl_AsyncPort, (struct Message *) agm);
				    }
				    else
				    {
					cl->cl_ErrorLevel = RETURN_FAIL;
					cl->cl_ErrorNumber = AGERR_NOT_ENOUGH_MEMORY;
				    }
				}

				if (cl->cl_ErrorLevel == 0)
				{
				    HandleEvents (ag, cl);

				    PrintComplete (ag, cl);

				    CloseAGWindow (ag, cl);

				    /* Clear the IO error field */
				    cl->cl_ErrorLevel = RETURN_OK;
				    cl->cl_ErrorNumber = 0;
				    SetIoErr (0);
				}
			    }
			    else
			    {
				cl->cl_ErrorLevel = RETURN_FAIL;
				cl->cl_ErrorNumber = ERR_CANT_OPEN_WINDOW;
			    }

			    /* Free the data object */
			    DisposeDTObject (cl->cl_DataObject);
			}
			else
			{
			    cl->cl_ErrorLevel  = RETURN_FAIL;
			    cl->cl_ErrorNumber = ERR_CANT_OPEN_DATABASE;	/* IoErr(); */
			}

			/* Unwind */
			UnLock (CurrentDir (old));
		    }
		    else
		    {
			cl->cl_ErrorLevel  = RETURN_FAIL;
			cl->cl_ErrorNumber = ERR_CANT_OPEN_DATABASE;
		    }

		    /* Free the screen draw information */
		    FreeScreenDrawInfo (cl->cl_Screen, cl->cl_DrInfo);
		}
		else
		{
		    cl->cl_ErrorLevel = RETURN_FAIL;
		    cl->cl_ErrorNumber = ERR_CANT_OPEN_WINDOW;
		}

		/* Free the visual information */
		FreeVisualInfo (cl->cl_VI);
	    }
	    else
	    {
		cl->cl_ErrorLevel = RETURN_FAIL;
		cl->cl_ErrorNumber = ERR_CANT_OPEN_WINDOW;
	    }

	    /* Unlock any screen that we may have locked */
	    if (scr)
		UnlockPubScreen (NULL, scr);
	}
	else
	{
	    cl->cl_ErrorLevel = RETURN_FAIL;
	    cl->cl_ErrorNumber = ERR_CANT_OPEN_WINDOW;
	}

	if (cl->cl_AsyncPort && cl->cl_ErrorLevel)
	{
	    struct AmigaGuideMsg *agm;
	    BOOL die = FALSE;

	    /* The port is dead */
	    cl->cl_Flags |= AGF_PORTDEAD;

	    /* Allocate a message to send */
	    if (agm = AllocPVec (ag, ag->ag_MemoryPool, AGMSIZE))
	    {
		/* Initialize the message */
		agm->agm_Type = ToolStatusID;
		agm->agm_Msg.mn_Node.ln_Type = NT_MESSAGE;
		agm->agm_Msg.mn_ReplyPort = cl->cl_ChildPort;
		agm->agm_Msg.mn_Length = AGMSIZE;
		agm->agm_Pri_Ret = cl->cl_ErrorLevel;
		agm->agm_Sec_Ret = cl->cl_ErrorNumber;

		/* Send the message */
		PutMsg (cl->cl_AsyncPort, (struct Message *) agm);

		while (!die)
		{
		    WaitPort (cl->cl_ChildPort);
		    die = HandleAmigaGuideMsgs (ag, cl, agm);
		}
	    }
	}

	/* Delete the message ports */
	DeleteMsgPort (cl->cl_ChildPort);
	DeleteMsgPort (cl->cl_AWPort);
	DeleteMsgPort (cl->cl_IDCMPPort);
	cl->cl_ChildPort = cl->cl_AWPort = cl->cl_IDCMPPort = NULL;
    }
    else
    {
	cl->cl_ErrorLevel = RETURN_FAIL;
	cl->cl_ErrorNumber = ERR_CANT_CREATE_PORT;
    }

    FreeARexx (ag, cl->cl_RexxHandle);

    cl->cl_Flags |= AGF_DONE;
}

/*****************************************************************************/

BOOL CreateMsgPorts (struct AmigaGuideLib *ag, struct Client * cl)
{

    if (cl->cl_IDCMPPort = CreateMsgPort ())
    {
	if (cl->cl_AWPort = CreateMsgPort ())
	{
	    if (cl->cl_AsyncPort)
	    {
		if (cl->cl_ChildPort = CreateMsgPort ())
		{
		    return (TRUE);
		}
	    }
	    else
	    {
		return (TRUE);
	    }
	    DeleteMsgPort (cl->cl_AWPort);
	}
	DeleteMsgPort (cl->cl_IDCMPPort);
    }

    cl->cl_ChildPort = NULL;
    cl->cl_AWPort = NULL;
    cl->cl_IDCMPPort = NULL;

    return (FALSE);
}

/*****************************************************************************/

struct Window *OpenAGWindow (struct AmigaGuideLib *ag, struct Client * cl)
{
    LoadSnapShot (ag, cl);
    GetAttr (DTA_NominalHoriz, cl->cl_DataObject, &cl->cl_TotHoriz);
    GetAttr (DTA_NominalVert, cl->cl_DataObject, &cl->cl_TotVert);
    if (cl->cl_TotHoriz == 0)
	cl->cl_TotHoriz = cl->cl_Prefs.p_Window.Width;
    if (cl->cl_TotVert == 0)
	cl->cl_TotVert = cl->cl_Prefs.p_Window.Height;

    /* Build the window */
    if (cl->cl_WinObject = NewObject (ag->ag_WindowClass, NULL,
				      WA_Activate,	!(cl->cl_Flags & AGF_NOACTIVATE),
				      WA_Left,		cl->cl_Prefs.p_Window.Left,
				      WA_Top,		cl->cl_Prefs.p_Window.Top,
				      WA_InnerWidth,	cl->cl_TotHoriz + 4,
				      WA_InnerHeight,	cl->cl_TotVert + 2,
				      WA_PubScreen,	cl->cl_Screen,
				      WA_HelpGroup,	cl->cl_HelpGroup,
				      WOA_Title,	(ULONG)GetAmigaGuideStringLVO (ag, TITLE_LOADING),
				      WOA_VisualInfo,	cl->cl_VI,
				      WOA_DrawInfo,	cl->cl_DrInfo,
				      WOA_IDCMPPort,	cl->cl_IDCMPPort,
				      WOA_AWPort,	cl->cl_AWPort,
				      WOA_TextAttr,	&cl->cl_TAttr,
				      ICA_TARGET,	ICTARGET_IDCMP,
				      TAG_DONE))
    {
	/* Get the window pointers */
	GetAttr (WOA_Window, cl->cl_WinObject, (ULONG *) & cl->cl_Window);
	DoMethod (cl->cl_WinObject, WOM_ADDVIEW, (ULONG) cl->cl_DataObject, NULL);
    }
    return (cl->cl_Window);
}

/*****************************************************************************/

VOID CloseAGWindow (struct AmigaGuideLib *ag, struct Client * cl)
{

    if (cl->cl_Window)
    {
	DoMethod (cl->cl_WinObject, WOM_REMVIEW, (ULONG) cl->cl_DataObject);
	DisposeObject (cl->cl_WinObject);
	cl->cl_WinObject = NULL;
	cl->cl_Window = NULL;
    }
}

/*****************************************************************************/

ULONG ProcessStrCommand (struct AmigaGuideLib *ag, struct Client * cl, STRPTR cmd)
{
    ULONG retval = 1L;

    if (Stricmp (cmd, "QUIT") == 0)
    {
	cl->cl_Going = FALSE;
    }
    else
    {
	if (cl->cl_Window == NULL)
	{
	    OpenAGWindow (ag, cl);
	}

	if (Stricmp (cmd, "WINDOWTOFRONT") == 0)
	{
	    WindowToFront (cl->cl_Window);
	}
	else if (Stricmp (cmd, "WINDOWTOBACK") == 0)
	{
	    WindowToBack (cl->cl_Window);
	}
	else if (Stricmp (cmd, "ACTIVATEWINDOW") == 0)
	{
	    ActivateWindow (cl->cl_Window);
	}
	else if (Stricmp (cmd, "ZOOMWINDOW") == 0)
	{
	    if (!(cl->cl_Window->Flags & WFLG_ZOOMED))
		ZipWindow (cl->cl_Window);
	}
	else if (Stricmp (cmd, "UNZOOMWINDOW") == 0)
	{
	    if (cl->cl_Window->Flags & WFLG_ZOOMED)
		ZipWindow (cl->cl_Window);
	}
	else
	{
	    struct dtTrigger dtt;

	    dtt.MethodID = DTM_TRIGGER;
	    dtt.dtt_GInfo = NULL;
	    dtt.dtt_Function = STM_COMMAND;
	    dtt.dtt_Data = cmd;
	    retval = DoGadgetMethodA ((struct Gadget *) cl->cl_DataObject, cl->cl_Window, NULL, (Msg) &dtt);
	}
    }
    return retval;
}

/*****************************************************************************/

LONG OpenNewData (struct AmigaGuideLib *ag, struct Client * cl, ULONG stype)
{
    struct dtTrigger dtt;
    struct DataType *dtn;
    LONG retval = 0L;
    BPTR lock;

    /* Show that we're loading a new object */
    setattrs (ag, cl->cl_WinObject, WOA_Title, (ULONG) GetAmigaGuideStringLVO (ag, TITLE_LOADING), TAG_DONE);

    if (stype == DTST_FILE)
    {
	sprintf (cl->cl_TempBuffer, "LINK \"%s/Main\"", cl->cl_NameBuffer);
	dtt.MethodID = DTM_TRIGGER;
	dtt.dtt_GInfo = NULL;
	dtt.dtt_Function = STM_COMMAND;
	dtt.dtt_Data = cl->cl_TempBuffer;

	retval = DoGadgetMethodA ((struct Gadget *) cl->cl_DataObject, cl->cl_Window, NULL, (Msg) &dtt);
    }

    if (retval == 0)
    {
	if (cl->cl_DataObject)
	{
	    DoMethod ((Object *) cl->cl_WinObject, WOM_REMVIEW, (ULONG) cl->cl_DataObject);

	    SetAPen (cl->cl_Window->RPort, 0);
	    RectFill (cl->cl_Window->RPort,
		      cl->cl_Window->BorderLeft,
		      cl->cl_Window->BorderTop,
		      cl->cl_Window->Width - cl->cl_Window->BorderRight - 1,
		      cl->cl_Window->Height - cl->cl_Window->BorderBottom - 1);

	    DisposeDTObject (cl->cl_DataObject);
	}

	if (cl->cl_DataObject = NewDTObject (cl->cl_NameBuffer,
					     GA_Immediate,	TRUE,
					     GA_RelVerify,	TRUE,
					     AGA_HelpGroup,	cl->cl_HelpGroup,
					     DTA_SourceType,	stype,
					     DTA_TextAttr,	(ULONG) & cl->cl_TAttr,
					     TAG_DONE))
	{
	    DoMethod (cl->cl_WinObject, WOM_ADDVIEW, (ULONG) cl->cl_DataObject, NULL);
	    retval = 1L;
	}
	else
	{
	    cl->cl_ErrorNumber = IoErr ();

	    if (lock = Lock (cl->cl_NameBuffer, ACCESS_READ))
	    {
		if (dtn = ObtainDataTypeA (DTST_FILE, (APTR) lock, NULL))
		{
		    if (Stricmp (dtn->dtn_Header->dth_Name, "directory") == 0)
			retval = 2;
		    ReleaseDataType (dtn);
		}
		UnLock (lock);
	    }

	    if (retval == 0)
	    {
		PrintF (ag, cl, 2, cl->cl_ErrorNumber, FilePart (cl->cl_NameBuffer));

		/* Clear the sliders */
		SetGadgetAttrs ((struct Gadget *)cl->cl_WinObject, cl->cl_Window, NULL,
			  DTA_TopVert, 0L,
			  DTA_TopHoriz, 0L,
			  DTA_TotalVert, 0L,
			  DTA_TotalHoriz, 0L,
			  TAG_DONE);
	    }
	    else
	    {
		setattrs (ag, cl->cl_WinObject, WOA_Title, (ULONG) GetAmigaGuideStringLVO (ag, TITLE_AMIGAGUIDE), TAG_DONE);
	    }

	    /* Clear the window pointer */
	    setwindowpointer (ag, cl->cl_Window, WA_Pointer, NULL, TAG_DONE);
	}
    }
    return retval;
}

/*****************************************************************************/

BOOL HandleAmigaGuideMsgs (struct AmigaGuideLib *ag, struct Client * cl, struct AmigaGuideMsg * dagm)
{
    struct AmigaGuideMsg *agm;
    BOOL die = FALSE;

    while (agm = (struct AmigaGuideMsg *) GetMsg (cl->cl_ChildPort))
    {
	if (agm == dagm)
	    die = TRUE;

	if (agm->agm_Msg.mn_Node.ln_Type == NT_MESSAGE)
	{
	    switch (agm->agm_Type)
	    {
		case ShutdownToolID:
		    cl->cl_Going = FALSE;
		    die = TRUE;
		    break;

		case ToolCmdID:
		    strcpy (cl->cl_WorkText, agm->agm_Data);
		    if (!(cl->cl_Flags & AGF_PORTDEAD))
			ProcessStrCommand (ag, cl, cl->cl_WorkText);
		    break;
	    }
	    ReplyMsg ((struct Message *) agm);
	}
	else if (agm->agm_Msg.mn_Node.ln_Type == NT_REPLYMSG)
	{
	    /* Return the message to the pool */
	    FreePVec (ag, ag->ag_MemoryPool, agm);
	}
    }
    return die;
}

/*****************************************************************************/

void HandleEvents (struct AmigaGuideLib *ag, struct Client * cl)
{
    struct MenuItem *menuitem;
    struct IntuiMessage *imsg;
    struct AppMessage *amsg;
    LONG error = RETURN_OK;
    struct TagItem *attrs;
    struct RexxMsg *rmsg;
    struct WBArg *wbarg;
    struct TagItem *tag;
    STRPTR text = NULL;
    BOOL die = FALSE;
    ULONG sigpr;
    ULONG siga;
    ULONG sigw;
    ULONG sigr;
    ULONG sigc;
    ULONG sigp;
    UWORD id;

    /* Get the signal bits */
    siga = 1L << cl->cl_AWPort->mp_SigBit;
    sigc = (cl->cl_ChildPort) ? (1L << cl->cl_ChildPort->mp_SigBit) : 0L;
    sigw = 1L << cl->cl_IDCMPPort->mp_SigBit;
    sigp = ARexxSignal (ag, cl->cl_RexxHandle);

    /* Keep going until we are told to stop */
    while (cl->cl_Going)
    {
	/*******************************/
	/* Any damage to take care of? */
	/*******************************/
	if (cl->cl_Window && (LAYERREFRESH & cl->cl_Window->WLayer->Flags))
	{
	    BeginRefresh (cl->cl_Window);
	    EndRefresh (cl->cl_Window, TRUE);
	}

	sigpr = NULL;
	if (cl->cl_PrintWin)
	{
	    sigpr = 1L << cl->cl_PrintWin->UserPort->mp_SigBit;
	}

	/* Wait for something to happen */
	sigr = Wait (siga | sigc | sigw | sigp | sigpr | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);

	/************/
	/* Aborted? */
	/************/
	if (sigr & SIGBREAKF_CTRL_C)
	{
	    cl->cl_Going = FALSE;
	}

	/**************/
	/* Activated? */
	/***************/
	if (sigr & SIGBREAKF_CTRL_F)
	{
	    if (cl->cl_Window)
	    {
		if (cl->cl_Window->Flags & WFLG_ZOOMED)
		    ZipWindow (cl->cl_Window);
		WindowToFront (cl->cl_Window);
		ActivateWindow (cl->cl_Window);
	    }
	}

	/******************/
	/* Printer window */
	/******************/
	if ((sigr & sigpr) && cl->cl_PrintWin)
	{
	    if (SysReqHandler (cl->cl_PrintWin, NULL, FALSE) == 0)
	    {
		DoMethod (cl->cl_DataObject, DTM_ABORTPRINT, NULL);
	    }
	}

	/*****************/
	/* ARexx message */
	/*****************/
	if (cl->cl_RexxHandle && (sigr & sigp))
	{
	    rmsg = GetARexxMsg (ag, cl->cl_RexxHandle);

	    /* Make sure it is something we can handle */
	    if (rmsg == REXX_RETURN_ERROR)
	    {
		/* Put the error message here someday... */
	    }
	    else if (rmsg)
	    {
		strcpy (cl->cl_WorkText, ARG0 (rmsg));
		ProcessStrCommand (ag, cl, cl->cl_WorkText);
		ReplyARexxMsg (ag, cl->cl_RexxHandle, rmsg, text, error);
	    }
	}

	/************************/
	/* Asynchronous message */
	/************************/
	if (sigr & sigc)
	{
	    die = HandleAmigaGuideMsgs (ag, cl, NULL);
	}

	/*********************/
	/* AppWindow message */
	/*********************/
	if (sigr & siga)
	{
	    ActivateWindow (cl->cl_Window);
	    setwindowpointer (ag, cl->cl_Window, WA_BusyPointer, TRUE, TAG_DONE);

	    while (amsg = (struct AppMessage *) GetMsg (cl->cl_AWPort))
	    {
		if (amsg->am_NumArgs)
		{
		    wbarg = amsg->am_ArgList;
		    NameFromLock (wbarg->wa_Lock, cl->cl_NameBuffer, NAMEBUFSIZE);
		    AddPart (cl->cl_NameBuffer, wbarg->wa_Name, NAMEBUFSIZE);

		    if (OpenNewData (ag, cl, DTST_FILE) == 2)
		    {
			if (FileRequest (ag, cl, 0, GetAmigaGuideStringLVO (ag, TITLE_SELECT_FILE_TO_OPEN), GetAmigaGuideStringLVO (ag, LABEL_OPEN), cl->cl_NameBuffer))
			{
			    OpenNewData (ag, cl, DTST_FILE);
			}
		    }
		}

		ReplyMsg ((struct Message *) amsg);
	    }
	}

	/*********************/
	/* Intuition message */
	/*********************/
	while (imsg = (struct IntuiMessage *) GetMsg (cl->cl_IDCMPPort))
	{
	    switch (imsg->Class)
	    {
		case IDCMP_MENUPICK:
		    id = imsg->Code;
		    while ((id != MENUNULL) && cl->cl_Going)
		    {
			menuitem = ItemAddress (imsg->IDCMPWindow->MenuStrip, id);
			ProcessCommand (ag, cl, (ULONG) MENU_USERDATA (menuitem), imsg);
			id = menuitem->NextSelect;
		    }
		    break;

		case IDCMP_CLOSEWINDOW:
		    cl->cl_Going = FALSE;
		    break;

		case IDCMP_MOUSEBUTTONS:
		    setwindowpointer (ag, cl->cl_Window, WA_Pointer, NULL, TAG_DONE);
		    break;

		case IDCMP_GADGETUP:
		    setwindowpointer (ag, cl->cl_Window, WA_Pointer, NULL, TAG_DONE);
		    {
			switch (((struct Gadget *) imsg->IAddress)->GadgetID)
			{
			    case GID_CONTENTS:
				cl->cl_Trigger.dtt_Function = STM_CONTENTS;
				DoDTMethodA (cl->cl_DataObject, cl->cl_Window, NULL, (Msg) &cl->cl_Trigger);
				break;

			    case GID_INDEX:
				cl->cl_Trigger.dtt_Function = STM_INDEX;
				DoDTMethodA (cl->cl_DataObject, cl->cl_Window, NULL, (Msg) &cl->cl_Trigger);
				break;

			    case GID_HELP:
				break;

			    case GID_RETRACE:
				cl->cl_Trigger.dtt_Function = STM_RETRACE;
				DoDTMethodA (cl->cl_DataObject, cl->cl_Window, NULL, (Msg) &cl->cl_Trigger);
				break;

			    case GID_BROWSEP:
				cl->cl_Trigger.dtt_Function = STM_BROWSE_PREV;
				DoDTMethodA (cl->cl_DataObject, cl->cl_Window, NULL, (Msg) &cl->cl_Trigger);
				break;

			    case GID_BROWSEN:
				cl->cl_Trigger.dtt_Function = STM_BROWSE_NEXT;
				DoDTMethodA (cl->cl_DataObject, cl->cl_Window, NULL, (Msg) &cl->cl_Trigger);
				break;
			}
		    }
		    break;

		case IDCMP_RAWKEY:
		    switch (imsg->Code)
		    {
			    /* Shift Tab */
			case 66:
			    Navigate (ag, cl, AC_FIELD_PREV, FALSE);
			    break;

			    /* Up Arrow */
			case 76:
			    if (imsg->Qualifier & ALTED)
				Navigate (ag, cl, AC_TOP, TRUE);
			    else if (imsg->Qualifier & SHIFTED)
				Navigate (ag, cl, AC_PAGE_UP, TRUE);
			    else
				Navigate (ag, cl, AC_UNIT_UP, TRUE);
			    break;

			    /* Down Arrow */
			case 77:
			    if (imsg->Qualifier & ALTED)
				Navigate (ag, cl, AC_BOTTOM, TRUE);
			    else if (imsg->Qualifier & SHIFTED)
				Navigate (ag, cl, AC_PAGE_DOWN, TRUE);
			    else
				Navigate (ag, cl, AC_UNIT_DOWN, TRUE);
			    break;

			    /* Right Arrow */
			case 78:
			    if (imsg->Qualifier & ALTED)
				Navigate (ag, cl, AC_FAR_RIGHT, TRUE);
			    else if (imsg->Qualifier & SHIFTED)
				Navigate (ag, cl, AC_PAGE_RIGHT, TRUE);
			    else
				Navigate (ag, cl, AC_UNIT_RIGHT, TRUE);
			    break;

			    /* Left Arrow */
			case 79:
			    if (imsg->Qualifier & ALTED)
				Navigate (ag, cl, AC_FAR_LEFT, TRUE);
			    else if (imsg->Qualifier & SHIFTED)
				Navigate (ag, cl, AC_PAGE_LEFT, TRUE);
			    else
				Navigate (ag, cl, AC_UNIT_LEFT, TRUE);
			    break;
		    }
		    break;

		case IDCMP_VANILLAKEY:
		    switch (imsg->Code)
		    {
			    /* Backspace */
			case 8:
			    Navigate (ag, cl, AC_PAGE_UP, TRUE);
			    break;

			    /* Tab */
			case 9:
			    if (imsg->Qualifier & SHIFTED)
				Navigate (ag, cl, AC_FIELD_PREV, FALSE);
			    else
				Navigate (ag, cl, AC_FIELD_NEXT, FALSE);
			    break;

			    /* Return */
			case 13:
			    Navigate (ag, cl, AC_ACTIVATE_FIELD, FALSE);
			    break;

			    /* Space */
			case 32:
			    Navigate (ag, cl, AC_PAGE_DOWN, TRUE);
			    break;

			case '/':
			    Navigate (ag, cl, AC_RETRACE, FALSE);
			    break;

			case '<':
			    Navigate (ag, cl, AC_NODE_PREV, FALSE);
			    break;

			case '>':
			    Navigate (ag, cl, AC_NODE_NEXT, FALSE);
			    break;

			case 'q':
			case 'Q':
			case 27:
			    cl->cl_Going = FALSE;
			    break;
		    }
		    break;

		case IDCMP_IDCMPUPDATE:
		    attrs = (struct TagItem *) imsg->IAddress;

		    if (tag = FindTagItem (DTA_ErrorLevel, attrs))
		    {
			cl->cl_ErrorNumber = GetTagData (DTA_ErrorNumber, NULL, attrs);
			PrintF (ag, cl, 2, cl->cl_ErrorNumber, (STRPTR) GetTagData (DTA_ErrorString,NULL,attrs));
		    }

		    if (tag = FindTagItem (DTA_PrinterStatus, attrs))
		    {
			PrintComplete (ag, cl);

			if (tag->ti_Data)
			{
			    PrintF (ag, cl, 1, 5100 + tag->ti_Data, NULL);
			}
		    }

		    if (tag = FindTagItem (DTA_Busy, attrs))
		    {
			if (tag->ti_Data)
			    setwindowpointer (ag, cl->cl_Window, WA_BusyPointer, TRUE, TAG_DONE);
			else
			    setwindowpointer (ag, cl->cl_Window, WA_Pointer, NULL, TAG_DONE);
		    }

		    if (tag = FindTagItem (DTA_Title, attrs))
			setattrs (ag, cl->cl_WinObject, WOA_Title, tag->ti_Data, TAG_DONE);

#if 0
		    if (tag = FindTagItem (DTA_Sync, attrs))
			setattrs (ag, cl->cl_WinObject, WOA_Sync, tag->ti_Data, TAG_DONE);
#endif
		    break;
	    }

	    ReplyMsg ((struct Message *) imsg);
	}

	if (!cl->cl_Going && !die && cl->cl_Process)
	{
	    CloseAGWindow (ag, cl);
	    cl->cl_Going = TRUE;
	}
    }
}


/*****************************************************************************/

VOID Navigate (struct AmigaGuideLib *ag, struct Client * cl, LONG cmd, BOOL needvisual)
{
    LONG otoph, toph, vish, toth;
    LONG otopv, topv, visv, totv;

    if (needvisual)
    {
	GetDTAttrs (cl->cl_DataObject,
		    DTA_TopVert, &topv,
		    DTA_VisibleVert, &visv,
		    DTA_TotalVert, &totv,
		    DTA_TopHoriz, &toph,
		    DTA_VisibleHoriz, &vish,
		    DTA_TotalHoriz, &toth,
		    TAG_DONE);
	otoph = toph;
	otopv = topv;
    }

    switch (cmd)
    {
	case AC_TOP:
	    topv = 0;
	    break;

	case AC_BOTTOM:
	    topv = totv - visv;
	    break;

	case AC_FAR_LEFT:
	    toph = 0;
	    break;

	case AC_FAR_RIGHT:
	    toph = toth - vish;
	    break;

	case AC_UNIT_UP:
	    topv--;
	    break;

	case AC_UNIT_DOWN:
	    topv++;
	    break;

	case AC_UNIT_LEFT:
	    toph--;
	    break;

	case AC_UNIT_RIGHT:
	    toph++;
	    break;

	case AC_PAGE_UP:
	    topv -= (visv - 1);
	    break;

	case AC_PAGE_DOWN:
	    topv += (visv - 1);
	    break;

	case AC_PAGE_LEFT:
	    toph -= (vish - 1);
	    break;

	case AC_PAGE_RIGHT:
	    toph += (vish - 1);
	    break;

	case AC_FIELD_NEXT:
	    cl->cl_Trigger.dtt_Function = STM_NEXT_FIELD;
	    DoDTMethodA (cl->cl_DataObject, cl->cl_Window, NULL, (Msg) &cl->cl_Trigger);
	    break;

	case AC_FIELD_PREV:
	    cl->cl_Trigger.dtt_Function = STM_PREV_FIELD;
	    DoDTMethodA (cl->cl_DataObject, cl->cl_Window, NULL, (Msg) &cl->cl_Trigger);
	    break;

	case AC_ACTIVATE_FIELD:
	    cl->cl_Trigger.dtt_Function = STM_ACTIVATE_FIELD;
	    DoDTMethodA (cl->cl_DataObject, cl->cl_Window, NULL, (Msg) &cl->cl_Trigger);
	    break;

	case AC_NODE_NEXT:
	    cl->cl_Trigger.dtt_Function = STM_BROWSE_NEXT;
	    DoDTMethodA (cl->cl_DataObject, cl->cl_Window, NULL, (Msg) &cl->cl_Trigger);
	    break;

	case AC_NODE_PREV:
	    cl->cl_Trigger.dtt_Function = STM_BROWSE_PREV;
	    DoDTMethodA (cl->cl_DataObject, cl->cl_Window, NULL, (Msg) &cl->cl_Trigger);
	    break;

	case AC_RETRACE:
	    cl->cl_Trigger.dtt_Function = STM_RETRACE;
	    DoDTMethodA (cl->cl_DataObject, cl->cl_Window, NULL, (Msg) &cl->cl_Trigger);
	    break;
    }

    if (needvisual)
    {
	if (topv < 0)
	    topv = 0;
	if (toph < 0)
	    toph = 0;
	if (topv > totv - visv)
	    topv = totv - visv;
	if (toph > toth - vish)
	    toph = toth - vish;

	if ((topv != otopv) || (toph != otoph))
	{
	    SetGadgetAttrs ((struct Gadget *)cl->cl_WinObject, cl->cl_Window, NULL,
			DTA_TopVert, topv,
			DTA_TopHoriz, toph,
			DTA_Sync, TRUE,
			TAG_DONE);
	}
    }
}

/*****************************************************************************/

void ProcessCommand (struct AmigaGuideLib *ag, struct Client * cl, ULONG id, struct IntuiMessage * imsg)
{
    struct Window *win = cl->cl_Window;
    struct DTSpecialInfo *si = NULL;
    ULONG left, top, width, height;
    struct dtGeneral dtg;
    BOOL change = FALSE;
    struct Gadget *g;

    if (!id)
	return;

    if (g = G (cl->cl_DataObject))
	si = (struct DTSpecialInfo *) g->SpecialInfo;

    switch (id)
    {
	case MMC_OPEN:
	    if (FileRequest (ag, cl, 0, GetAmigaGuideStringLVO (ag, TITLE_SELECT_FILE_TO_OPEN), GetAmigaGuideStringLVO (ag, LABEL_OPEN), cl->cl_NameBuffer))
	    {
		OpenNewData (ag, cl, DTST_FILE);
	    }
	    break;

	case MMC_SAVEAS:
	    if (cl->cl_DataObject)
	    {
		SaveObject (ag, cl, DTWM_RAW);
	    }
	    break;

	case MMC_PRINT:
	    if (cl->cl_DataObject)
	    {
		PrintObject (ag, cl, 0);
	    }
	    break;

	case MMC_ABOUT:
	    if (cl->cl_DataObject)
	    {
		AboutObject (ag, cl);
	    }
	    break;

	case MMC_QUIT:
	    cl->cl_Going = FALSE;
	    break;

	case MMC_MARK:
	    if (si)
	    {
		si->si_Flags |= DTSIF_DRAGSELECT;
		SetBlockPointer (ag, win);
	    }
	    break;

	case MMC_COPY:
	    dtg.MethodID = DTM_COPY;
	    DoDTMethodA (cl->cl_DataObject, win, NULL, (Msg) &dtg);
	    break;

	/* WINDOW menu */
	case MMC_MINIMIZE:
	    left   = cl->cl_Window->LeftEdge;
	    top    = cl->cl_Window->TopEdge;
	    width  = cl->cl_Window->MinWidth;
	    height = cl->cl_Window->MinHeight;
	    change = TRUE;
	    break;

	case MMC_NORMAL:
	    left   = cl->cl_Window->LeftEdge;
	    top    = cl->cl_Window->TopEdge;
	    width  = 0;
	    height = 0;
	    if (cl->cl_DataObject)
	    {
		GetDTAttrs (cl->cl_DataObject,
			    DTA_NominalHoriz,	&width,
			    DTA_NominalVert,	&height,
			    TAG_DONE);
	    }

	    if (width == 0)
		width  = cl->cl_Window->RPort->TxWidth * 80;
	    if (height == 0)
		height = cl->cl_Window->RPort->TxHeight * 24;
	    width  += 4 + cl->cl_Window->BorderLeft + cl->cl_Window->BorderRight;
	    height += 2 + cl->cl_Window->BorderTop  + cl->cl_Window->BorderBottom;
	    change = TRUE;
	    break;

	case MMC_MAXIMIZE:
	    left   = 0;
	    top    = 0;
	    width  = cl->cl_Window->WScreen->Width;
	    height = cl->cl_Window->WScreen->Height;
	    change = TRUE;
	    break;

	case MMC_SAVE_AS_DEFAULTS:
	    SaveSnapShot (ag, cl);
	    break;
    }

    if (change)
	ChangeWindowBox (cl->cl_Window, left, top, width, height);
}

/*****************************************************************************/

VOID AboutObject (struct AmigaGuideLib *ag, struct Client * cl)
{
    struct EasyStruct es;
    struct DataType *dtn;

    GetDTAttrs (cl->cl_DataObject, DTA_DataType, (ULONG) & dtn, TAG_DONE);

    es.es_StructSize = sizeof (struct EasyStruct);
    es.es_Flags = 0;
    es.es_Title = GetAmigaGuideStringLVO (ag, TITLE_AMIGAGUIDE);
    es.es_TextFormat = "AmigaGuide %ld.%ld (%s)\n%s %s";
    es.es_GadgetFormat = GetAmigaGuideStringLVO (ag, LABEL_CONTINUE);

    easyrequest (ag, cl->cl_Window, &es, VERSION, REVISION, DATE, dtn->dtn_Header->dth_Name, GetDTString (dtn->dtn_Header->dth_GroupID));
}
