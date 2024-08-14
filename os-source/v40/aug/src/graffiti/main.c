/* main.c --- main for Graffiti
 * Copyright © 1992 Commodore-Amiga, Inc.
 * Written by David N. Junod
 *
 */

#include "graffiti.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

#define	IDCMP_FLAGS	IDCMP_CLOSEWINDOW | IDCMP_NEWSIZE | IDCMP_REFRESHWINDOW | \
			IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_MENUPICK | \
			IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | \
			IDCMP_VANILLAKEY | IDCMP_RAWKEY | IDCMP_INTUITICKS | \
			IDCMP_IDCMPUPDATE

/*****************************************************************************/

LONG main (void)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    LONG failureLevel = RETURN_FAIL;
    struct WBStartup *wbMsg = NULL;
    struct Transaction *trans;
    ULONG maxwidth, maxheight;
    struct Process *process;
    struct GlobalData *gd;
    LONG failureCode = 0;
    BOOL success = FALSE;
    struct Hook h;
    UBYTE tmp[80];
    ULONG msize;

    process = (struct Process *) SysBase->ThisTask;
    if (!(process->pr_CLI))
    {
	WaitPort (&process->pr_MsgPort);
	wbMsg = (struct WBStartup *) GetMsg (&process->pr_MsgPort);
    }

    if (SysBase->LibNode.lib_Version < 37)
    {
	failureCode = ERROR_INVALID_RESIDENT_LIBRARY;
    }
    /* Allocate the global data */
    else if (gd = AllocVec (sizeof (struct GlobalData), MEMF_CLEAR | MEMF_PUBLIC))
    {
	gd->gd_Process = process;

	/* Initialize the list of clients */
	NewList (&gd->gd_ClientList);

	/* Open the ROM libraries that we need */
	gd->gd_SysBase = SysBase;
	gd->gd_DOSBase = OpenLibrary ("dos.library", 37);
	gd->gd_GfxBase = OpenLibrary ("graphics.library", 37);
	gd->gd_IconBase = OpenLibrary ("icon.library", 37);
	gd->gd_IntuitionBase = OpenLibrary ("intuition.library", 37);
	gd->gd_UtilityBase = OpenLibrary ("utility.library", 37);
	gd->gd_WorkbenchBase = OpenLibrary ("workbench.library", 37);
	gd->gd_LayersBase = OpenLibrary ("layers.library", 37);
	gd->gd_GadToolsBase = OpenLibrary ("gadtools.library", 37);

	/* Open the disk-based libraries */
	if (!(gd->gd_DataTypesBase = OpenLibrary ("datatypes.library", 39)))
	    lprintf (gd, "Couldn't open datatypes.library V39\n", NULL);
	if (!(gd->gd_AslBase = OpenLibrary ("asl.library", 38)))
	    lprintf (gd, "Couldn't open asl.library V38\n", NULL);
	if (!(gd->gd_NIPCBase = OpenLibrary ("nipc.library", 37)))
	    lprintf (gd, "Couldn't open nipc.library V37\n", NULL);

	/* Make sure the disk libraries are open */
	if (gd->gd_NIPCBase)
	{
	    /* Get the host name */
	    if (GetHostName (0, tmp, 64))
	    {
		stcgfp (gd->gd_HostRealm, tmp);
		stcgfn (gd->gd_Host, tmp);
	    }
	    else
		GetVar ("hostname", gd->gd_Host, sizeof (gd->gd_Host), NULL);
	    GetVar ("user", gd->gd_User, sizeof (gd->gd_User), NULL);

	    /* Provide reasonable defaults */
	    strcpy (gd->gd_Server, SERVER_NAME);
	    strcpy (gd->gd_ServerRealm, gd->gd_HostRealm);
	    gd->gd_Width = gd->gd_Height = 1024;
	    gd->gd_Depth = 1;

	    /***********************/
	    /* Workbench arguments */
	    /***********************/
	    if (wbMsg)
	    {
		struct DiskObject *dob;
		struct WBArg *wa;
		BPTR oldLock;
		STRPTR tmp;
		ULONG hold;

		wa = wbMsg->sm_ArgList;
		oldLock = CurrentDir (wa->wa_Lock);
		dob = GetDiskObject (wa->wa_Name);
		CurrentDir (oldLock);

#if 0
		if (wbMsg->sm_NumArgs > 1)
		{
		    wa++;
		    NameFromLock (wa->wa_Lock, gd->gd_NameBuffer, sizeof (gd->gd_NameBuffer));
		    AddPart (gd->gd_NameBuffer, wa->wa_Name, sizeof (gd->gd_NameBuffer));
		    gd->gd_SourceName = (APTR) gd->gd_NameBuffer;
		}
#endif

		if (dob)
		{
		    if (tmp = FindToolType (dob->do_ToolTypes, "SERVER"))
			strcpy (gd->gd_Server, tmp);

		    if (tmp = FindToolType (dob->do_ToolTypes, "REALM"))
			strcpy (gd->gd_ServerRealm, tmp);

		    if (tmp = FindToolType (dob->do_ToolTypes, "PUBSCREEN"))
		    {
			strcpy (gd->gd_ScreenNameBuffer, tmp);
			gd->gd_ScreenName = gd->gd_ScreenNameBuffer;
		    }

		    if (tmp = FindToolType (dob->do_ToolTypes, "WIDTH"))
			if (StrToLong (tmp, &hold) > 0L)
			    gd->gd_Width = hold;

		    if (tmp = FindToolType (dob->do_ToolTypes, "HEIGHT"))
			if (StrToLong (tmp, &hold) > 0L)
			    gd->gd_Height = hold;

		    if (tmp = FindToolType (dob->do_ToolTypes, "DEPTH"))
			if (StrToLong (tmp, &hold) > 0L)
			    gd->gd_Depth = hold;

		    FreeDiskObject (dob);
		}
	    }
	    /*******************/
	    /* Shell Arguments */
	    /*******************/
	    else if (gd->gd_RDArgs = ReadArgs (TEMPLATE, gd->gd_Options, NULL))
	    {
		if (!((SIGBREAKF_CTRL_C & CheckSignal (SIGBREAKF_CTRL_C))))
		{
		    if (gd->gd_Options[OPT_SERVER])
			strcpy (gd->gd_Server, (STRPTR) gd->gd_Options[OPT_SERVER]);

		    if (gd->gd_Options[OPT_REALM])
			strcpy (gd->gd_ServerRealm, (STRPTR) gd->gd_Options[OPT_REALM]);

		    if (gd->gd_Options[OPT_PUBSCREEN])
		    {
			strcpy (gd->gd_ScreenNameBuffer, (STRPTR) gd->gd_Options[OPT_PUBSCREEN]);
			gd->gd_ScreenName = gd->gd_ScreenNameBuffer;
		    }

		    if (gd->gd_Options[OPT_WIDTH])
			gd->gd_Width = *((ULONG *) gd->gd_Options[OPT_WIDTH]);

		    if (gd->gd_Options[OPT_HEIGHT])
			gd->gd_Height = *((ULONG *) gd->gd_Options[OPT_HEIGHT]);

		    if (gd->gd_Options[OPT_DEPTH])
			gd->gd_Depth = *((ULONG *) gd->gd_Options[OPT_DEPTH]);
		}
		else
		{
		    failureCode = gd->gd_ErrorNumber = ERROR_BREAK;
		}
	    }
	    else
	    {
		failureCode = gd->gd_ErrorNumber = IoErr ();
	    }

	    if (failureCode == 0)
	    {
		/* Range check values */
		if (gd->gd_Depth  < 1)
		    gd->gd_Depth  = 1;
		if (gd->gd_Depth  > 8)
		    gd->gd_Depth  = 8;
		if (gd->gd_Width  < 80)
		    gd->gd_Width  = 80;
		if (gd->gd_Height < 40)
		    gd->gd_Height = 40;

		/* Show the local information */
		lprintf (gd, "user=\"%s\", host=\"%s\", realm=\"%s\".\n", (void *) gd->gd_User, (void *) gd->gd_Host, (void *) gd->gd_HostRealm);

		/* Prepare the temporary hook */
		h.h_Entry = FindServer;
		h.h_Data = gd;

		/* See if Graffiti is running anywhere */
		lprintf (gd, "query entity=\"%s\", realm=\"%s\".\n", (void *) gd->gd_Server, (void *) gd->gd_ServerRealm);
		if (nipcinquiry (gd, &h, 15, 1,
				 MATCH_ENTITY,	(ULONG) gd->gd_Server,
				 MATCH_REALM,	(ULONG) gd->gd_ServerRealm,
				 QUERY_HOSTNAME,0,
				 TAG_DONE))
		{
		    Wait (SIGBREAKF_CTRL_E);
		}
		else
		{
		    lprintf (gd, "network inquiry failed.\n", NULL);
		}

		/* Provide simple status information */
		if (strlen (gd->gd_ServerHost))
		{
		    lprintf (gd, "server \"%s\" on \"%s\" in realm \"%s\".\n", (void *) gd->gd_Server, (void *) gd->gd_ServerHost, (void *) gd->gd_ServerRealm);

		    /* Create the client */
		    sprintf (tmp, "%s.%s@%s", gd->gd_Server, gd->gd_User, gd->gd_Host);
		    DB (lprintf (gd, "Entity is \"%s\".\n", (void *) tmp));
		    if (gd->gd_HEntity = createentity (gd,
						       ENT_AllocSignal, &gd->gd_BitNumber,
						       ENT_Name, (ULONG) tmp,
						       ENT_Public, TRUE,
						       TAG_DONE))
		    {
			/* Build the server host name */
			if (strlen (gd->gd_ServerRealm))
			    sprintf (tmp, "%s:%s", gd->gd_ServerRealm, gd->gd_ServerHost);
			else
			    strcpy (tmp, gd->gd_ServerHost);

			/* Find the server. */
			if (gd->gd_SEntity = FindEntity (tmp, gd->gd_Server, gd->gd_HEntity, NULL))
			{
			    /* Build the name */
			    sprintf (tmp, "%s@%s", gd->gd_User, gd->gd_Host);
			    msize = strlen (tmp) + 1;

			    /* Create the login message */
			    if (trans = alloctransaction (gd, TRN_AllocReqBuffer, msize, TAG_DONE))
			    {
				/* Initialize the message */
				trans->trans_Command = CMD_LOGIN;
				strcpy ((char *) trans->trans_RequestData, tmp);
				trans->trans_ReqDataActual = msize;
				trans->trans_Timeout = 15;
				DB (lprintf (gd, "Send login message \"%s\".\n", (void *) tmp));

				/* Send the login message */
				BeginTransaction (gd->gd_SEntity, gd->gd_HEntity, trans);

				/* Show that we can continue to run */
				success = TRUE;
			    }
			    else
			    {
				lprintf (gd, "Couldn't allocate login transaction.\n", NULL);
			    }
			}
			else
			{
			    lprintf (gd, "Couldn't find server entity \"%s\" on \"%s\".\n", gd->gd_Server, tmp);
			}
		    }
		    else
		    {
			lprintf (gd, "Couldn't create client.\n", NULL);
		    }
		}
		/* Create the server */
		else if (gd->gd_HEntity = createentity (gd,
							ENT_AllocSignal, &gd->gd_BitNumber,
							ENT_Name, (ULONG) gd->gd_Server,
							ENT_Public, TRUE,
							TAG_DONE))
		{
		    lprintf (gd, "%s wasn't running.  Created server.\n", (void *) gd->gd_Server);

		    /* Show that we are the server */
		    gd->gd_Flags |= GDF_SERVER;

		    /* Show that we can continue to run */
		    success = TRUE;
		}
		else
		{
		    lprintf (gd, "Couldn't create server.\n", NULL);
		}

		/* Are we able to run still? */
		if (success)
		{
		    /* Create the window class */
		    if (gd->gd_WinClass = initViewWindowClass ())
		    {
			/* Initialize the screen information */
			if (ObtainScreen (gd))
			{
			    /* Create the message ports that we need */
			    if (CreatePorts (gd))
			    {
				/* Create the data need for this simple demo */
				if (CreateDemoData (gd))
				{
				    /* Compute the maximum size for the window */
				    maxwidth = gd->gd_Screen->WBorLeft + gd->gd_Width + 18;
				    maxheight = gd->gd_Screen->BarHeight + 1 + gd->gd_Height + 10;

				    /* Create a new window */
				    if (gd->gd_Window = OpenViewWindow (gd->gd_WinClass,
									WA_InnerWidth, 320,
									WA_InnerHeight, 100,
									WA_ReportMouse, TRUE,
									WA_Activate, TRUE,
									WA_PubScreen, (ULONG) gd->gd_Screen,
									WA_SimpleRefresh, TRUE,
									WA_Flags, WFLG_SUPER_BITMAP,
									WA_MaxWidth, maxwidth,
									WA_MaxHeight, maxheight,

									WOA_IDCMP, IDCMP_FLAGS,
									WOA_IDCMPPort, (ULONG) gd->gd_IDCMP,
									WOA_Title, (ULONG) gd->gd_Server,
									WOA_VertInc, 8,
									WOA_HorizInc, 8,

								        /* This is needed for scroller info */
									ICA_TARGET, ICTARGET_IDCMP,

									TAG_DONE))
				    {
					/* Create the menus */
					CreateMainMenu (gd);

					/* Indicate success */
					failureLevel = RETURN_OK;

					/* Bring the screen to the front */
					ScreenToFront (gd->gd_Window->WScreen);

					/* Handle the events */
					HandleEvents (gd);

					/* Close the talk window */
					CloseTalkWindow (gd);

					/* Remove the AppIcon, if there is one */
					RemoveAppIcon (gd->gd_AppIcon);

					/* Remove the clipping region */
					if (gd->gd_Region)
					{
					    InstallClipRegion (gd->gd_Window->RPort->Layer, gd->gd_ORegion);
					    DisposeRegion (gd->gd_Region);
					}

					/* Clear the menus */
					ClearMenuStrip (gd->gd_Window);
					FreeMenus (gd->gd_Menu);

					/* Close the window object */
					CloseViewWindow (gd->gd_WinClass, gd->gd_Window);
				    }
				    else
					lprintf (gd, "Couldn't open window.\n", NULL);

				    /* Delete the demo data */
				    DeleteDemoData (gd);
				}
				else
				    lprintf (gd, "Couldn't allocate whiteboard data.\n", NULL);

				/* Delete the message ports */
				DeletePorts (gd);
			    }
			    else
				lprintf (gd, "Couldn't obtain message ports.\n", NULL);

			    /* Release the screen information */
			    ReleaseScreen (gd);
			}
			else
			    lprintf (gd, "Couldn't obtain screen.\n", NULL);

			/* All done with the window class */
			freeViewWindowClass (gd->gd_WinClass);
		    }
		    else
		    {
			lprintf (gd, "Couldn't init view window class.\n", NULL);
		    }
		}

		/* Delete the entities */
		if (gd->gd_SEntity)
		    LoseEntity (gd->gd_SEntity);
		if (gd->gd_HEntity)
		    DeleteEntity (gd->gd_HEntity);
	    }
	}

	/* Close the libraries */
	CloseLibrary (gd->gd_NIPCBase);
	CloseLibrary (gd->gd_AslBase);
	CloseLibrary (gd->gd_DataTypesBase);
	CloseLibrary (gd->gd_LayersBase);
	CloseLibrary (gd->gd_WorkbenchBase);
	CloseLibrary (gd->gd_UtilityBase);
	CloseLibrary (gd->gd_IntuitionBase);
	CloseLibrary (gd->gd_IconBase);
	CloseLibrary (gd->gd_GfxBase);
	CloseLibrary (gd->gd_DOSBase);

	/* Free the global data */
	FreeVec (gd);
    }
    else
	failureCode = ERROR_NO_FREE_STORE;

    if (wbMsg)
    {
	Forbid ();
	ReplyMsg ((struct Message *) wbMsg);
    }

    process->pr_Result2 = failureCode;
    return (failureLevel);
}

/*****************************************************************************/

BOOL ObtainScreen (struct GlobalData * gd)
{
    if (gd->gd_Screen = LockPubScreen (gd->gd_ScreenName))
    {
	if (gd->gd_VI = GetVisualInfoA (gd->gd_Screen, NULL))
	    if (gd->gd_DrInfo = GetScreenDrawInfo (gd->gd_Screen))
		return TRUE;
    }
    else
	lprintf (gd, "Couldn't lock public screen.\n", NULL);

    return FALSE;
}

/*****************************************************************************/

void ReleaseScreen (struct GlobalData * gd)
{

    if (gd->gd_Screen)
    {
	FreeVisualInfo (gd->gd_VI);
	FreeScreenDrawInfo (gd->gd_Screen, gd->gd_DrInfo);
	UnlockPubScreen (NULL, gd->gd_Screen);
    }
}

/*****************************************************************************/

BOOL CreatePorts (struct GlobalData * gd)
{
    if (gd->gd_IDCMP = CreateMsgPort ())
	if (gd->gd_WBPort = CreateMsgPort ())
	    return TRUE;
    return FALSE;
}

/*****************************************************************************/

void DeletePorts (struct GlobalData * gd)
{
    DeleteMsgPort (gd->gd_WBPort);
    DeleteMsgPort (gd->gd_IDCMP);
}

/*****************************************************************************/

BOOL CreateDemoData (struct GlobalData * gd)
{
    BOOL error = FALSE;
    ULONG i;

    /* Initialize the rastport */
    InitRastPort (&gd->gd_RPort);
    gd->gd_FGPen = 1;
    gd->gd_DrMode = JAM1;

    /* Only create the bitmap if we are the server. */
    if (gd->gd_Flags & GDF_SERVER)
    {
	/* Allocate the bitmap */
	if (gd->gd_BitMap = AllocVec (sizeof (struct BitMap), MEMF_CLEAR))
	{
	    /* Allocate the planes */
	    InitBitMap (gd->gd_BitMap, gd->gd_Depth, gd->gd_Width, gd->gd_Height);
	    for (i = 0; (i < gd->gd_Depth) && (error == FALSE); i++)
	    {
		if (gd->gd_BitMap->Planes[i] = AllocRaster (gd->gd_Width, gd->gd_Height))
		    BltClear (gd->gd_BitMap->Planes[i],
			      (((ULONG) gd->gd_BitMap->Rows << 16L) | (ULONG) gd->gd_BitMap->BytesPerRow),
			      0x2L);
		else
		    error = TRUE;
	    }

	    if (error)
		DeleteDemoData (gd);
#if 0
	    else
		BltBitMap (&gd->gd_Screen->BitMap, 0, 0,
			   gd->gd_BitMap, 0, 0, gd->gd_Width, gd->gd_Height, 0xC0, 0xFF, NULL);
#endif
	}
    }

    return (!error);
}

/*****************************************************************************/

void DeleteDemoData (struct GlobalData * gd)
{
    UWORD i;

    if (gd->gd_BitMap)
    {
	for (i = 0; i < gd->gd_BitMap->Depth; i++)
	    if (gd->gd_BitMap->Planes[i])
		FreeRaster (gd->gd_BitMap->Planes[i], gd->gd_Width, gd->gd_Height);
	FreeVec (gd->gd_BitMap);
	gd->gd_BitMap = NULL;
    }
}
