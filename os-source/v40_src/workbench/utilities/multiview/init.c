/* multiview.c
 *
 */

#include "multiview.h"
#include "multiview_rev.h"

/*****************************************************************************/

#define	DB(x)	;
#define	DC(x)	;
#define	DM(x)	;
#define	DR(x)	;
#define	DX(x)	;
#define	DD(x)	;

/*****************************************************************************/

void kprintf (void *, ...);

/*****************************************************************************/

char *version = VERSTAG;

/*****************************************************************************/

extern struct Cmd cmdArray[];

/*****************************************************************************/

#undef	SysBase

/*****************************************************************************/

int cmd_start (void)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    extern STRPTR ARexxPortName (AREXXCONTEXT ac);
    LONG failureLevel = RETURN_FAIL;
    struct WBStartup *wbMsg = NULL;
    struct Process *process;
    struct GlobalData *gd;
    LONG failureCode = 0;
    struct Node *node;

    process = (struct Process *) SysBase->ThisTask;
    if (!(process->pr_CLI))
    {
	WaitPort (&process->pr_MsgPort);
	wbMsg = (struct WBStartup *) GetMsg (&process->pr_MsgPort);
    }

    if (SysBase->LibNode.lib_Version < 39)
    {
	failureCode = ERROR_INVALID_RESIDENT_LIBRARY;
	PrintF (NULL, 0, MV_ERROR_REQUIRES_V39, NULL);
    }
    else if (gd = AllocVec (sizeof (struct GlobalData), MEMF_CLEAR))
    {
	gd->gd_Process = (struct Process *) SysBase->ThisTask;
	gd->gd_SysBase = SysBase;
	gd->gd_Going = TRUE;
	gd->gd_Trigger.MethodID = DTM_TRIGGER;
	gd->gd_SourceType = DTST_FILE;

	DOSBase = OpenLibrary ("dos.library", 39);
	IntuitionBase = OpenLibrary ("intuition.library", 39);
	GfxBase = OpenLibrary ("graphics.library", 39);
	UtilityBase = OpenLibrary ("utility.library", 39);
	WorkbenchBase = OpenLibrary ("workbench.library", 39);
	IconBase = OpenLibrary ("icon.library", 39);
	GadToolsBase = OpenLibrary ("gadtools.library", 39);
	LayersBase = OpenLibrary ("layers.library", 39);

	AslBase        = OpenLibrary ("asl.library", 37);
	RexxSysBase    = OpenLibrary ("rexxsyslib.library", 0L);

	if (LocaleBase = OpenLibrary ("locale.library", 38))
	    gd->gd_Catalog = OpenCatalogA (NULL, "Sys/utilities.catalog", NULL);

	if (DataTypesBase = OpenLibrary ("datatypes.library", 0))
	{
	    /* Allocate the command processor */
	    if ((failureCode == 0) && (gd->gd_CmdHeader = AllocCmdProcessor (cmdArray, gd)))
	    {
		if (gd->gd_BlockO = AllocBlockPointer (gd))
		{
		/* Allocate the window class */
		if (gd->gd_WindowClass = initWindowClass (gd))
		{
		    /* Create the Intuition message port */
		    gd->gd_IDCMPPort = CreateMsgPort ();

		    /* Create the Workbench message port */
		    gd->gd_AWPort = CreateMsgPort ();

		    if (gd->gd_IDCMPPort && gd->gd_AWPort)
		    {
#undef	GfxBase
			gd->gd_TextFont = ((struct GfxBase *) gd->gd_GfxBase)->DefaultFont;
#define	GfxBase	gd->gd_GfxBase

			strcpy (gd->gd_FontName, gd->gd_TextFont->tf_Message.mn_Node.ln_Name);
			gd->gd_TextAttr.ta_Name = gd->gd_FontName;
			gd->gd_TextAttr.ta_YSize = gd->gd_TextFont->tf_YSize;
			gd->gd_TextAttr.ta_Style = gd->gd_TextFont->tf_Style;
			gd->gd_TextAttr.ta_Flags = gd->gd_TextFont->tf_Flags;

			NewList (&gd->gd_WindowList);
			NewList ((struct List *) &gd->gd_UserMenuList);

			strcpy (gd->gd_ARexxPortName, "MULTIVIEW");

			/***********************/
			/* Workbench arguments */
			/***********************/

			if (wbMsg)
			{
			    struct WBArg *wa;

			    wa = wbMsg->sm_ArgList;

			    /* Get arguments from the tool icon */
			    GetIconArgs (gd, wa, TRUE);

			    /* Do we have a project icon */
			    if (wbMsg->sm_NumArgs > 1)
			    {
				wa++;

				/* Get arguments from the project icon */
				GetIconArgs (gd, wa, FALSE);

				NameFromLock (wa->wa_Lock, gd->gd_NameBuffer, sizeof (gd->gd_NameBuffer));
				AddPart (gd->gd_NameBuffer, wa->wa_Name, sizeof (gd->gd_NameBuffer));
				gd->gd_SourceName = (APTR) gd->gd_NameBuffer;
			    }
			}

			/*******************/
			/* Shell Arguments */
			/*******************/

			else if (gd->gd_RDArgs = ReadArgs (TEMPLATE, gd->gd_Options, NULL))
			{
			    if (!((SIGBREAKF_CTRL_C & CheckSignal (SIGBREAKF_CTRL_C))))
			    {
				gd->gd_SourceName = (APTR) gd->gd_Options[OPT_FILE];
				if (gd->gd_Options[OPT_FILE])
				{
				    strcpy (gd->gd_NameBuffer, (STRPTR) gd->gd_Options[OPT_FILE]);
				    gd->gd_SourceName = gd->gd_NameBuffer;
				}

				if (gd->gd_Options[OPT_CLIPUNIT])
				    gd->gd_Unit = *((ULONG *) gd->gd_Options[OPT_CLIPUNIT]);

				/* Must be initialized after CLIPUNIT */
				if (gd->gd_Options[OPT_CLIPBOARD])
				{
				    gd->gd_SourceType = DTST_CLIPBOARD;
				    gd->gd_SourceName = (APTR) gd->gd_Unit;
				    gd->gd_NameBuffer[0] = 0;
				}

				if (gd->gd_Options[OPT_SCREEN])
				{
				    strcpy (gd->gd_ScreenNameBuffer, "MultiView");
				    gd->gd_Flags |= GDF_SCREEN;
				}
				else if (gd->gd_Options[OPT_PUBSCREEN])
				{
				    strcpy (gd->gd_ScreenNameBuffer, (STRPTR) gd->gd_Options[OPT_PUBSCREEN]);
				    gd->gd_ScreenName = gd->gd_ScreenNameBuffer;
				}
				else
				{
				    GetDefaultPubScreen (gd->gd_ScreenNameBuffer);
				    gd->gd_ScreenName = gd->gd_ScreenNameBuffer;
				}

				if (gd->gd_Options[OPT_FONTSIZE])
				    gd->gd_TextAttr.ta_YSize = *((ULONG *) gd->gd_Options[OPT_FONTSIZE]);

				if (gd->gd_Options[OPT_FONTNAME])
				{
				    struct Library *DiskfontBase;

				    sprintf (gd->gd_FontName, "%s.font", (STRPTR) gd->gd_Options[OPT_FONTNAME]);
				    if (DiskfontBase = OpenLibrary ("diskfont.library", 0))
				    {
					if (gd->gd_TextFont = OpenDiskFont (&gd->gd_TextAttr))
					    gd->gd_Flags |= GDF_OPENFONT;
					CloseLibrary (DiskfontBase);
				    }
				}

				if (gd->gd_Options[OPT_PORTNAME])
				{
				    strcpy (gd->gd_ARexxPortName, (STRPTR) gd->gd_Options[OPT_PORTNAME]);
				}
			    }
			    else
			    {
				failureCode = gd->gd_SecondaryResult = ERROR_BREAK;
			    }
			}
			else
			{
			    failureCode = gd->gd_SecondaryResult = IoErr ();
			}

			/* Initialize the ARexx handler */
			if (gd->gd_Options[OPT_PORTNAME])
			{
			    if (!(gd->gd_ARexxHandle = InitARexx (gd->gd_ARexxPortName, "mv", TRUE)))
			    {
				failureCode = gd->gd_SecondaryResult = ERROR_OBJECT_EXISTS;
			    }
			}
			else
			{
			    gd->gd_ARexxHandle = InitARexx (gd->gd_ARexxPortName, "mv", FALSE);
			}

			if (gd->gd_ARexxHandle)
			{
			    strcpy (gd->gd_ARexxPortName, ARexxPortName (gd->gd_ARexxHandle));
			}

			/************/
			/* Bookmark */
			/************/

			if ((failureCode == 0) && gd->gd_Options[OPT_BOOKMARK])
			{
			    BPTR fh;

			    /* Open the bookmark file */
			    if (fh = Open ("ENV:MultiView.bookmark", MODE_OLDFILE))
			    {
				/* Real simple for right now.  Just the name.
				 * I'm going to change this to topv, toph, name, node */
				if (Read (fh, gd->gd_NameBuffer, NAMEBUFSIZE) > 0)
				    gd->gd_SourceName = gd->gd_NameBuffer;

				/* Close the bookmark file */
				Close (fh);
			    }
			    else
			    {
				failureCode = IoErr ();
				gd->gd_SecondaryResult = MV_ERROR_COULDNT_LOAD_BOOKMARK;
			    }
			}

			/****************************/
			/* File requester arguments */
			/****************************/

			/* See if we need to pop up a file requester */
			if ((failureCode == 0) && (gd->gd_SourceType == DTST_FILE) && (gd->gd_Options[OPT_WINDOW] == 0))
			{
			    struct DataType *dtn;
			    BOOL request = FALSE;
			    BPTR lock;
			    UBYTE lc;

			    if (gd->gd_SourceName)
			    {
				/* Determine if it is a directory */
				if (lock = Lock (gd->gd_SourceName, ACCESS_READ))
				{
				    /* Get the object type */
				    if (dtn = ObtainDataTypeA (DTST_FILE, (APTR) lock, NULL))
				    {
					/* If the object type is a directory, then we will bring up
					 * the file requester. */
					if (Stricmp (dtn->dtn_Header->dth_Name, "directory") == 0)
					{
					    /* Tack on a / if needed */
					    lc = gd->gd_NameBuffer[(strlen (gd->gd_NameBuffer) - 1)];
					    if ((lc != '/') && (lc != ':'))
						strcat (gd->gd_NameBuffer, "/");
					    request = TRUE;
					}

					/* Done with the datatype, so release it */
					ReleaseDataType (dtn);
				    }
				    UnLock (lock);
				}
			    }
			    else
				request = TRUE;

			    if (request)
			    {
				/* Get a pointer to the destination screen */
				if (!(gd->gd_Screen = LockPubScreen (gd->gd_ScreenName)))
				    gd->gd_Screen = LockPubScreen (NULL);

				/* Load the file requester snapshot */
				LoadSnapShot (gd);

				/* Pop up the file requester */
				ScreenToFront (gd->gd_Screen);
				if (FileRequest (gd, 0, GetString (gd, MV_TITLE_SELECT_FILE_TO_OPEN), GetString (gd, MV_LABEL_OPEN), gd->gd_NameBuffer))
				{
				    gd->gd_SourceName = (APTR) gd->gd_NameBuffer;
				    strcpy (gd->gd_NameBuffer, (STRPTR) gd->gd_SourceName);
				}
				else
				{
				    failureCode = gd->gd_SecondaryResult = ERROR_BREAK;
				    /* ERROR_REQUIRED_ARG_MISSING; */
				}

				/* Unlock the temporary screen */
				UnlockPubScreen (NULL, gd->gd_Screen);
				gd->gd_Screen = NULL;
			    }
			}

			/*********/
			/* Do It */
			/*********/

			/* Success so far? */
			if (failureCode == 0)
			{
			    /* Make sure we have a name to start with */
			    if (gd->gd_SourceName || (gd->gd_SourceType == DTST_CLIPBOARD))
			    {
				if (!(gd->gd_DataObject = NewDTObject (gd->gd_SourceName,
									DTA_SourceType,		gd->gd_SourceType,
									DTA_TextAttr,		(ULONG) & gd->gd_TextAttr,
									DTA_ARexxPortName,	(ULONG)gd->gd_ARexxPortName,
									GA_Immediate,		TRUE,
									GA_RelVerify,		TRUE,
									TAG_DONE)))
				{
				    failureCode = gd->gd_SecondaryResult = IoErr ();
				}
			    }
			    else if (!gd->gd_Options[OPT_WINDOW])
			    {
				failureCode = gd->gd_SecondaryResult = ERROR_REQUIRED_ARG_MISSING;
			    }

			    if (failureCode == 0)
			    {
				/* Open the required display environment */
				if (OpenEnvironment (gd))
				{
				    /* Handle input */
				    HandleEvents (gd);

				    /* Indicate success */
				    gd->gd_SecondaryResult = 0;
				    failureCode = 0;
				    failureLevel = RETURN_OK;

				    /* Close the display environment */
				    CloseEnvironment (gd, 2);
				}
				else
				{
				    failureCode = gd->gd_SecondaryResult;
				}
			    }
			}

			/***********/
			/* Cleanup */
			/***********/

			if (gd->gd_FR)
			    FreeAslRequest (gd->gd_FR);

			while (node = RemHead ((struct List *)&gd->gd_UserMenuList))
			    FreeVec (node);

			if (gd->gd_Flags & GDF_OPENFONT)
			    CloseFont (gd->gd_TextFont);

			/* Free the shell arguments */
			FreeArgs (gd->gd_RDArgs);
		    }

		    /* Delete the IDCMP port */
		    DeleteMsgPort (gd->gd_IDCMPPort);

		    /* Delete the AppWindow message port */
		    DeleteMsgPort (gd->gd_AWPort);

		    /* Free the window class */
		    freeWindowClass (gd->gd_WindowClass);
		}

		    FreeBlockPointer (gd, gd->gd_BlockO);
		}

		/* Free the command processor */
		FreeCmdProcessor (gd->gd_CmdHeader);
	    }

	    if (failureCode != ERROR_BREAK)
		PrintF (gd, gd->gd_Options[OPT_REQUESTER], gd->gd_SecondaryResult, FilePart (gd->gd_NameBuffer));

	    /* Free the ARexx handler */
	    FreeARexx (gd->gd_ARexxHandle);

	    /* Close datatypes.library */
	    CloseLibrary (DataTypesBase);
	}
	else
	{
	    gd->gd_SecondaryResult = MV_ERROR_NO_DATATYPES;
	    failureCode = ERROR_INVALID_RESIDENT_LIBRARY;
	    PrintF (gd, gd->gd_Options[OPT_REQUESTER], gd->gd_SecondaryResult, NULL);
	}

	if (LocaleBase)
	{
	    CloseCatalog (gd->gd_Catalog);
	    CloseLibrary (LocaleBase);
	}

	CloseLibrary (RexxSysBase);
	CloseLibrary (AslBase);
	CloseLibrary (LayersBase);
	CloseLibrary (GadToolsBase);
	CloseLibrary (IconBase);
	CloseLibrary (WorkbenchBase);
	CloseLibrary (UtilityBase);
	CloseLibrary (GfxBase);
	CloseLibrary (IntuitionBase);
	CloseLibrary (DOSBase);
	FreeVec (gd);
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

#define	SysBase		 gd->gd_SysBase

/*****************************************************************************/

void GetIconArgs (struct GlobalData * gd, struct WBArg * wa, BOOL tool)
{
    struct DiskObject *dob;
    UBYTE buffer[300];
    BPTR oldLock;
    STRPTR tmp;
    ULONG hold;

    /* Make it the current directory */
    oldLock = CurrentDir (wa->wa_Lock);

    /*
     * Bug in Workbench makes it so that wa_Lock doesn't have the correct directory lock if you specify the tool name without the path in the project icon
     */
    if (tool)
	sprintf (buffer, "PROGDIR:%s", wa->wa_Name);
    else
	strcpy (buffer, wa->wa_Name);

    /* Get the tool icon */
    if (dob = GetDiskObject (buffer))
    {
	if (tmp = FindToolType (dob->do_ToolTypes, "CLIPUNIT"))
	    if (StrToLong (tmp, &hold) > 0L)
		gd->gd_Unit = hold;

	/* Must be initialized after CLIPUNIT */
	if (FindToolType (dob->do_ToolTypes, "CLIPBOARD"))
	{
	    gd->gd_SourceType = DTST_CLIPBOARD;
	    gd->gd_SourceName = (APTR) gd->gd_Unit;
	    gd->gd_NameBuffer[0] = 0;
	}

	if (FindToolType (dob->do_ToolTypes, "SCREEN"))
	{
	    strcpy (gd->gd_ScreenNameBuffer, "MultiView");
	    gd->gd_Options[OPT_SCREEN] = TRUE;
	    gd->gd_Flags |= GDF_SCREEN;
	}
	else if (tmp = FindToolType (dob->do_ToolTypes, "PUBSCREEN"))
	{
	    strcpy (gd->gd_ScreenNameBuffer, tmp);
	    gd->gd_ScreenName = gd->gd_ScreenNameBuffer;
	}
	else
	{
	    GetDefaultPubScreen (gd->gd_ScreenNameBuffer);
	    gd->gd_ScreenName = gd->gd_ScreenNameBuffer;
	}

	if (tmp = FindToolType (dob->do_ToolTypes, "FONTSIZE"))
	    if (StrToLong (tmp, &hold) > 0L)
		gd->gd_TextAttr.ta_YSize = hold;

	if (tmp = FindToolType (dob->do_ToolTypes, "FONTNAME"))
	{
	    struct Library *DiskfontBase;

	    sprintf (gd->gd_FontName, "%s.font", tmp);

	    if (DiskfontBase = OpenLibrary ("diskfont.library", 0))
	    {
		if (gd->gd_TextFont = OpenDiskFont (&gd->gd_TextAttr))
		    gd->gd_Flags |= GDF_OPENFONT;
		CloseLibrary (DiskfontBase);
	    }
	}

	if (FindToolType (dob->do_ToolTypes, "BOOKMARK"))
	    gd->gd_Options[OPT_BOOKMARK] = TRUE;

	if (FindToolType (dob->do_ToolTypes, "BACKDROP"))
	    gd->gd_Options[OPT_BACKDROP] = TRUE;

	if (FindToolType (dob->do_ToolTypes, "WINDOW"))
	    gd->gd_Options[OPT_WINDOW] = TRUE;

	if (tmp = FindToolType (dob->do_ToolTypes, "PORTNAME"))
	    strcpy (gd->gd_ARexxPortName, (STRPTR) tmp);

	/* Free the icon */
	FreeDiskObject (dob);
    }

    /* Go back to the previous directory */
    CurrentDir (oldLock);
}

/*****************************************************************************/

struct Screen *OpenEnvironment (struct GlobalData * gd)
{
    ULONG text, back, shine, shadow, fill, detail, block;
    ULONG backdrop = gd->gd_Options[OPT_BACKDROP];
    ULONG useScreen = gd->gd_Options[OPT_SCREEN];
    ULONG *cregs = NULL, numcolors, i, r, g, b;
    ULONG backfill = (ULONG) LAYERS_BACKFILL;
    LONG left, top, width, height;
    UWORD pens[NUMDRIPENS + 1];
    ULONG modeid = INVALID_ID;
    struct BitMapHeader *bmhd;
    struct BitMap *bm = NULL;
    struct DimensionInfo dim;
    struct Rectangle rect;
    struct dtFrameBox dtf;
    struct FrameInfo *fri;
    BOOL notbusy = FALSE;
    struct ColorMap *cm;
    struct gpLayout gpl;
    struct Screen *scr;
    ULONG table[8][3];
    BOOL remap = TRUE;
    ULONG woaf = NULL;
    ULONG maxnum;
    STRPTR title;
    ULONG scrTypeTag;

    title = GetString (gd, MV_TITLE_LOADING);

    /* Doesn't require a screen */
    gd->gd_Flags &= ~GDF_REQUIRES_SCREEN;

    if (gd->gd_DataObject)
    {
	/* Get the mode ID */
	GetAttr (PDTA_ModeID, gd->gd_DataObject, (ULONG *) & modeid);

	/* Ask the object what kind of environment it needs */
	dtf.MethodID = DTM_FRAMEBOX;
	dtf.dtf_FrameInfo = fri = &gd->gd_FrameInfo;
	dtf.dtf_ContentsInfo = &gd->gd_FrameInfo;
	dtf.dtf_SizeFrameInfo = sizeof (struct FrameInfo);
	dtf.dtf_FrameFlags = NULL;
	if (DoMethodA (gd->gd_DataObject, &dtf) && fri->fri_Dimensions.Depth)
	{
	    if ((fri->fri_PropertyFlags & DIPF_IS_HAM) ||
		(fri->fri_PropertyFlags & DIPF_IS_EXTRAHALFBRITE))
	    {
		/* Requires a screen */
		gd->gd_Flags |= GDF_REQUIRES_SCREEN;

		useScreen = TRUE;
	    }

	    if ((fri->fri_PropertyFlags == 0) && (modeid & 0x800) && (modeid != INVALID_ID))
	    {
		/* Requires a screen */
		gd->gd_Flags |= GDF_REQUIRES_SCREEN;

		useScreen = TRUE;
	    }
	}
    }
    else if (gd->gd_Options[OPT_WINDOW])
    {
	title = GetString (gd, MV_TITLE_MULTIVIEW);
	useScreen = FALSE;
	notbusy = TRUE;
    }
    else
    {
	return NULL;
    }

    /* Should we use a custom screen? */
    if (useScreen)
    {
	/* Lock the default public screen so we know what to look like */
	if (scr = LockPubScreen (NULL))
	{
	    /* Provide some reasonable default values */
	    pens[0] = (USHORT)(~0);
	    detail = (ULONG) scr->DetailPen;
	    block = (ULONG) scr->BlockPen;

	    /* Get the picture information */
	    getdtattrs (gd, gd->gd_DataObject,
			PDTA_CRegs, &cregs,
			PDTA_NumColors, &numcolors,
			PDTA_BitMapHeader, &bmhd,
			TAG_DONE);

	    /* If we don't have a valid mode ID, then be like the Workbench */
	    if (modeid == INVALID_ID)
	    {
		modeid = GetVPModeID (&scr->ViewPort);
		fri->fri_Dimensions.Depth = MIN (2, scr->RastPort.BitMap->Depth);
	    }

	    /* Should the picture be remapped? */
	    if (bmhd)
		remap = (bmhd->bmh_Depth == fri->fri_Dimensions.Depth) ? FALSE : TRUE;
	    setdtattrs (gd, gd->gd_DataObject, NULL, PDTA_Remap, remap, TAG_DONE);

	    /* Do we have color registers? */
	    if (cregs)
	    {
		/* Get a temporary color map */
		if (cm = GetColorMap (numcolors))
		{
		    /* Set the colors in the temporary color map */
		    for (i = 0; i < numcolors; i++)
		    {
			/* Pull the color components */
			r = cregs[i * 3 + 0];
			g = cregs[i * 3 + 1];
			b = cregs[i * 3 + 2];

			/* Set the color */
			SetRGB32CM (cm, i, r, g, b);
		    }

		    /* Determine the maximum number of colors to try finding within */
		    maxnum = numcolors - 1;
		    if (modeid & 0x800)
			maxnum = (1 << (fri->fri_Dimensions.Depth - 2)) - 1;

		    /* Get the GUI colors from the default public screen */
		    GetRGB32 (scr->ViewPort.ColorMap, 0, 4, (ULONG *) table);

		    if (remap)
		    {
			/* Get the closest colors in the picture's color map */
			back = FindColor (scr->ViewPort.ColorMap, table[0][0], table[0][1], table[0][2], 4);
			shadow = FindColor (scr->ViewPort.ColorMap, table[1][0], table[1][1], table[1][2], 4);
			shine = FindColor (scr->ViewPort.ColorMap, table[2][0], table[2][1], table[2][2], 4);
			fill = FindColor (scr->ViewPort.ColorMap, table[3][0], table[3][1], table[3][2], 4);
		    }
		    else
		    {
			/* Get the closest colors in the picture's color map */
			back = FindColor (cm, table[0][0], table[0][1], table[0][2], maxnum);
			shadow = FindColor (cm, table[1][0], table[1][1], table[1][2], maxnum);
			shine = FindColor (cm, table[2][0], table[2][1], table[2][2], maxnum);
			fill = FindColor (cm, table[3][0], table[3][1], table[3][2], maxnum);
		    }

		    /* Set the pens */
		    text = (shadow == back) ? shine : shadow;
		    pens[DETAILPEN] = detail = back;
		    pens[BLOCKPEN] = block = shadow;
		    pens[TEXTPEN] = text;
		    pens[SHINEPEN] = shine;
		    pens[SHADOWPEN] = shadow;
		    pens[FILLPEN] = fill;
		    pens[FILLTEXTPEN] = text;
		    pens[HIGHLIGHTTEXTPEN] = shine;
		    pens[BACKGROUNDPEN] = back;
		    pens[BARDETAILPEN] = shadow;
		    pens[BARBLOCKPEN] = shine;
		    pens[BARTRIMPEN] = shadow;
		    pens[NUMDRIPENS] = (USHORT)(~0);

		    /* Free the temporary color map */
		    FreeColorMap (cm);
		}
		backfill = (ULONG) LAYERS_NOBACKFILL;
		backdrop = TRUE;
	    }

	    /* Close the existing environment */
	    CloseEnvironment (gd, 0);

	    if (bmhd)
	    {
		/* Use the nominal size */
		GetDisplayInfoData (NULL, (APTR) & dim, sizeof (struct DimensionInfo), DTAG_DIMS, modeid);

		/* Default to the standard video overscan display clip */
		rect = *(&dim.StdOScan);

		/* If the width is less then the nominal width, then use the nominal width */
		if (bmhd->bmh_Width <= (dim.Nominal.MaxX - dim.Nominal.MinX + 1))
		{
		    rect.MinX = dim.Nominal.MinX;
		    rect.MaxX = dim.Nominal.MaxX;
		}
		/* If the size is less than the standard video overscan then adjust the display clip */
		else if (bmhd->bmh_Width < (rect.MaxX - rect.MinX + 1))
		    rect.MaxX = bmhd->bmh_Width + rect.MinX - 1;

		/* If the height is less then the nominal height, then use the nominal height */
		if (bmhd->bmh_Height <= (dim.Nominal.MaxY - dim.Nominal.MinY + 1))
		{
		    rect.MinY = dim.Nominal.MinY;
		    rect.MaxY = dim.Nominal.MaxY;
		}
		/* If the size is less than the standard video overscan then adjust the display clip */
		else if (bmhd->bmh_Height < (rect.MaxY - rect.MinY + 1))
		    rect.MaxY = bmhd->bmh_Height + rect.MinY - 1;

		/* Calculate the size of the screen */
		width = MAX (bmhd->bmh_Width, rect.MaxX - rect.MinX + 1);
		height = MAX (bmhd->bmh_Height, rect.MaxY - rect.MinY + 1);

		/* The picture isn't going to be remapped, so come up with the custom bitmap */
		if (!remap && (bmhd->bmh_Width >= width) && (bmhd->bmh_Height >= height))
		{
		    /* Tell the object to generate the bitmap pointer */
		    gpl.MethodID = DTM_PROCLAYOUT;
		    gpl.gpl_GInfo = NULL;
		    gpl.gpl_Initial = 1;

		    if (DoMethodA (gd->gd_DataObject, &gpl))
		    {
			/* Get a pointer to the bitmap */
			getdtattrs (gd, gd->gd_DataObject, PDTA_BitMap, &bm, TAG_DONE);

			woaf = WOAF_BITMAP;
			gd->gd_Flags |= GDF_BITMAP;
		    }
		    else
			bm = NULL;
		}
		else
		    bm = NULL;
	    }
	    else
	    {
		/* Get the overscan text size */
		QueryOverscan (modeid, &rect, OSCAN_TEXT);

		/* calculate the size of the screen */
		width = rect.MaxX - rect.MinX + 1;
		height = rect.MaxY - rect.MinY + 1;

		/* No custom bitmap */
		bm = NULL;
	    }

	    /* Open the new screen */
	    if (gd->gd_Screen = openscreentags (gd,
						SA_Title, GetString (gd, MV_TITLE_MULTIVIEW),
						SA_Width, (ULONG) width,
						SA_Height, (ULONG) height,
						SA_Depth, (ULONG) fri->fri_Dimensions.Depth,
						SA_DClip, &rect,
						SA_AutoScroll, TRUE,
						SA_DisplayID, modeid,
						SA_Interleaved, TRUE,
						SA_SharePens, TRUE,
						SA_SysFont, 1,
						SA_ShowTitle, !(backdrop),
						SA_Quiet, backdrop,
						SA_Behind, TRUE,
						SA_Pens, pens,
						SA_BackFill, backfill,
						SA_DetailPen, detail,
						SA_BlockPen, block,
						((bm) ? SA_BitMap : TAG_IGNORE), bm,
						TAG_DONE))
	    {
		/* Show that we opened the screen */
		gd->gd_Flags |= GDF_OPENSCREEN;

		/* Load the color map */
		if (cregs && !remap)
		{
		    numcolors = 2 << (gd->gd_Screen->RastPort.BitMap->Depth - 1);
#if 0
		    LoadRGB32 (&gd->gd_Screen->ViewPort,
#else
		    for (i = 0; i < numcolors; i++)
		    {
			r = cregs[i * 3 + 0];
			g = cregs[i * 3 + 1];
			b = cregs[i * 3 + 2];
			SetRGB32 (&gd->gd_Screen->ViewPort, i, r, g, b);
		    }
#endif
		}
	    }

	    /* Unlock the public screen */
	    UnlockPubScreen (NULL, scr);
	}
    }
    else
    {
	CloseEnvironment (gd, 1);

	/* get a pointer to the default public gd->gd_Screen */
	gd->gd_Screen = LockPubScreen (gd->gd_ScreenName);
    }

    if (gd->gd_Screen)
    {
	if (gd->gd_VI = GetVisualInfoA (gd->gd_Screen, TAG_DONE))
	{
	    if (gd->gd_DrawInfo = GetScreenDrawInfo (gd->gd_Screen))
	    {
		frameobject (gd);

		if (!gd->gd_WindowObj)
		{
		    LoadSnapShot (gd);

		    getdtattrs (gd, gd->gd_DataObject,
				DTA_NominalHoriz, &gd->gd_TotHoriz,
				DTA_NominalVert, &gd->gd_TotVert,
				TAG_DONE);

		    if (gd->gd_TotHoriz == 0)
		    {
			gd->gd_TotHoriz = gd->gd_Prefs.p_Window.Width;
			gd->gd_Flags |= GDF_SNAPSHOT;
		    }

		    if (gd->gd_TotVert == 0)
		    {
			gd->gd_TotVert = gd->gd_Prefs.p_Window.Height;
		    }

		    gd->gd_VisVert = gd->gd_TotVert;
		    gd->gd_VisHoriz = gd->gd_TotHoriz;

		    top = gd->gd_Prefs.p_Window.Top,
		      left = gd->gd_Prefs.p_Window.Left,
		      width = gd->gd_TotHoriz + 4,
		      height = gd->gd_TotVert + 2,

		    /* Show that we are a backdrop */
		      woaf |= ((useScreen) ? NULL : WOAF_BACKDROP);

		    /* Figure out screen type */
		    scrTypeTag = (gd->gd_Flags & GDF_SCREEN) ? WA_CustomScreen : WA_PubScreen;

		    /* allocate and initialize a scroller as a BOOPSI object */
		    gd->gd_WindowObj = newobject (gd, gd->gd_WindowClass, NULL,
						  ((backdrop) ? TAG_IGNORE : WA_Left), left,
						  ((backdrop) ? TAG_IGNORE : WA_Top), top,
						  ((backdrop) ? TAG_IGNORE : WA_InnerWidth), width,
						  ((backdrop) ? TAG_IGNORE : WA_InnerHeight), height,
						  WA_Zoom,	&gd->gd_Prefs.p_Zoom,
						  scrTypeTag,	gd->gd_Screen,
						  DTA_Title,	title,
						  DTA_TopVert,	gd->gd_TotVert,
						  DTA_TopHoriz,	gd->gd_TotHoriz,
						  WOA_DrawInfo,	gd->gd_DrawInfo,
						  WOA_IDCMPPort,gd->gd_IDCMPPort,
						  WOA_AWPort,	gd->gd_AWPort,
						  WOA_Backdrop,	backdrop,
						  WOA_Flags,	woaf,
						  ICA_TARGET,	ICTARGET_IDCMP,
						  TAG_DONE);
		}

		if (gd->gd_WindowObj)
		{
		    /* Get a pointer to the window */
		    GetAttr (WOA_Window, gd->gd_WindowObj, (ULONG *) & gd->gd_Window);

		    /* Update the object attributes */
		    SetDataObjectAttrs (gd);

		    /* Bring the screen to the front */
		    ScreenToFront (window->WScreen);

		    /* Clear the busy pointer if we should */
		    if (notbusy)
			setwindowpointer (gd, window, WA_Pointer, NULL, TAG_DONE);

		    /* Allow menus to be used */
		    gd->gd_Window->Flags &= ~WFLG_RMBTRAP;

		    return gd->gd_Screen;
		}
	    }
	}
    }
    gd->gd_SecondaryResult = MV_ERROR_COULDNT_OPEN_ENV;
    CloseEnvironment (gd, 2);
    return NULL;
}

/*****************************************************************************/

VOID CloseEnvironment (struct GlobalData * gd, UWORD which)
{

    if (gd->gd_DataObject && (which == 2))
    {
	/* Remove the object from the window */
	DoMethod ((Object *) gd->gd_WindowObj, WOM_REMVIEW, (ULONG) gd->gd_DataObject);

	if (!(gd->gd_Flags & GDF_BITMAP))
	{
	    /* Get rid of the data object */
	    DisposeDTObject (gd->gd_DataObject);
	    gd->gd_DataObject = NULL;
	}
    }

    if (gd->gd_Screen)
    {
	if ((which != 1) || (gd->gd_Flags & GDF_OPENSCREEN))
	{
	    if (gd->gd_WindowObj)
	    {
		DisposeObject (gd->gd_WindowObj);
		gd->gd_WindowObj = NULL;
	    }
	}

	FreeScreenDrawInfo (gd->gd_Screen, gd->gd_DrawInfo);
	FreeVisualInfo (gd->gd_VI);
	gd->gd_DrawInfo = NULL;
	gd->gd_VI = NULL;

	if (gd->gd_Flags & GDF_OPENSCREEN)
	{
	    while (!CloseScreen (gd->gd_Screen))
		Delay (2);
	}
	else
	    UnlockPubScreen (NULL, gd->gd_Screen);
	gd->gd_Flags &= ~(GDF_OPENSCREEN | GDF_SNAPSHOT);
	gd->gd_Screen = NULL;
    }

    if (gd->gd_DataObject && (which == 2) && (gd->gd_Flags & GDF_BITMAP))
    {
	/* Get rid of the data object */
	DisposeDTObject (gd->gd_DataObject);
	gd->gd_DataObject = NULL;
    }

    gd->gd_Flags &= ~GDF_BITMAP;
}

/*****************************************************************************/

VOID AboutObject (struct GlobalData * gd)
{
    struct DataType *dtn = NULL;
    struct EasyStruct es;
    STRPTR s1, s2;
    UWORD i;

    if (gd->gd_DataObject)
    {
	getdtattrs (gd, gd->gd_DataObject, DTA_DataType, (ULONG) & dtn, TAG_DONE);
	s1 = dtn->dtn_Header->dth_Name;
	s2 = GetDTString (dtn->dtn_Header->dth_GroupID);
    }
    else
    {
	s1 = GetString (gd, MV_TITLE_EMPTY);
	s2 = "";
    }

    strncpy (gd->gd_TempBuffer, DataTypesBase->lib_IdString, 40);
    i = strlen (gd->gd_TempBuffer);
    if (gd->gd_TempBuffer[(i - 2)] == '\r')
	gd->gd_TempBuffer[(i - 2)] = 0;

    es.es_StructSize = sizeof (struct EasyStruct);
    es.es_Flags = 0;
    es.es_Title = GetString (gd, MV_TITLE_MULTIVIEW);
    es.es_TextFormat = "MultiView %ld.%ld (%s)\n%s\n\n%s %s";
    es.es_GadgetFormat = GetString (gd, MV_LABEL_CONTINUE);

    easyrequest (gd, &es, VERSION, REVISION, DATE, gd->gd_TempBuffer, s1, s2);
}
