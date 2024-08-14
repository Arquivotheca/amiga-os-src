/* memcheck.c
 *
 */

#include "memcheck.h"

/*****************************************************************************/

#include "memcheck_rev.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

struct GlobalData *gd = NULL;

/* Stored in 2 pieces so memory tools won't pick it up */
ULONG TAG1 = 0xC0DE0000;
ULONG TAG2 = 0x0000F00D;

/*****************************************************************************/

#define	CX_NAME		"MemCheck"
#define	CX_TITLE	CX_NAME
#define	CX_DESCRIPTION	"Memory Garbage Detector"

/*****************************************************************************/

static struct NewBroker nb =
{
    NB_VERSION,			/* Version of NewBroker structure */
    CX_NAME,			/* Name of commodity */
    CX_TITLE,			/* Title */
    CX_DESCRIPTION,		/* Description of commodity */
    NBU_UNIQUE | NBU_NOTIFY,	/* nb_Unique */
    COF_SHOW_HIDE,		/* nb_Flags */
    0,				/* nb_Pri */
    0,				/* nb_Port - message port, filled in later */
    0,
};

/*****************************************************************************/

LONG main (void)
{

#undef	SysBase
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    LONG failureLevel = RETURN_FAIL;
    struct WBStartup *wbMsg = NULL;
    struct IntuiMessage *imsg;
    struct MenuItem *menuitem;
    LONG failureCode = 0;
    ULONG msgid, msgtype;
    struct MemCheckInfo *mci;
    struct Process *pr;
    CxMsg *cmsg;
    ULONG rsig;
    ULONG sigw;
    ULONG sigc;
    UWORD id;

    pr = (struct Process *) SysBase->ThisTask;
    if (!(pr->pr_CLI))
    {
	WaitPort (&pr->pr_MsgPort);
	wbMsg = (struct WBStartup *) GetMsg (&pr->pr_MsgPort);
    }

    if (SysBase->LibNode.lib_Version < 37)
    {
	failureCode = ERROR_INVALID_RESIDENT_LIBRARY;
    }
    /* Allocate the global data */
    else if (gd = AllocVec (sizeof (struct GlobalData), MEMF_CLEAR | MEMF_PUBLIC))
    {
	/* Provide reasonable defaults */
	mci = &gd->gd_MemCheckInfo;
	mci->mci_Version = VERSION;
	mci->mci_Revision = REVISION;
	mci->mci_Stack = 2;
	mci->mci_InfoSize = sizeof (struct MemCheck);
	mci->mci_PreSize = PRESIZE;
	mci->mci_PostSize = POSTSIZE;
	mci->mci_FillChar = FILLCHAR;

	/* Open the window and be active by default */
	gd->gd_Flags |= GDF_OPENWINDOW | GDF_ACTIVE;

	/* Default popup key */
	gd->gd_CxPopKey = "control alt m";

	gd->gd_Process = pr;
	pr->pr_Task.tc_UserData = mci;

	/* Open the ROM libraries that we need */
	gd->gd_SysBase = SysBase;
#define	SysBase		 gd->gd_SysBase
	gd->gd_DOSBase = OpenLibrary ("dos.library", 37);
	gd->gd_UtilityBase = OpenLibrary ("utility.library", 37);
	gd->gd_IntuitionBase = OpenLibrary ("intuition.library", 37);
	gd->gd_GadToolsBase = OpenLibrary ("gadtools.library", 37);

	/* Find the layers start and end */
	FindLayers (gd);

	/* Open commodities */
	if (gd->gd_CxBase = OpenLibrary ("commodities.library", 37))
	{
	    /***********************/
	    /* Workbench arguments */
	    /***********************/
	    if (wbMsg)
	    {
		struct WBArg *wa;

		/* Get arguments from the tool icon */
		wa = wbMsg->sm_ArgList;
		GetIconArgs (gd, wa);

		/* See if we have a project icon */
		if (wbMsg->sm_NumArgs > 1)
		{
		    /* Get arguments from the project icon */
		    wa++;
		    GetIconArgs (gd, wa);
		}
	    }
	    /*******************/
	    /* Shell Arguments */
	    /*******************/
	    else if (gd->gd_RDArgs = ReadArgs (TEMPLATE, gd->gd_Options, NULL))
	    {
		if (!((SIGBREAKF_CTRL_C & CheckSignal (SIGBREAKF_CTRL_C))))
		{
		    if (gd->gd_Options[OPT_PRIORITY])
			nb.nb_Pri = (BYTE) * ((LONG *) gd->gd_Options[OPT_PRIORITY]);

		    if (gd->gd_Options[OPT_POPUP])
			if (Stricmp ((STRPTR) gd->gd_Options[OPT_POPUP], "NO") == 0)
			    gd->gd_Flags &= ~GDF_OPENWINDOW;

		    if (gd->gd_Options[OPT_POPKEY])
			gd->gd_CxPopKey = (STRPTR) gd->gd_Options[OPT_POPKEY];

		    /* What screen should we open on? */
		    if (gd->gd_Options[OPT_PUBSCREEN])
		    {
			strcpy (gd->gd_ScreenNameBuffer, (STRPTR) gd->gd_Options[OPT_PUBSCREEN]);
			gd->gd_ScreenName = gd->gd_ScreenNameBuffer;
		    }

		    /* How much stack should we show */
		    if (gd->gd_Options[OPT_STACKLINES])
			mci->mci_Stack = (UWORD) (*((ULONG *) gd->gd_Options[OPT_STACKLINES]));

		    /* Supposed to be compatible with MemCheck? */
		    if (gd->gd_Options[OPT_COMPAT])
			gd->gd_Flags |= GDF_COMPATIBLE;
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
		if (gd->gd_MemHeader = AllocMemHeader (gd))
		{
		    /* Get the current time */
		    DateStamp (&mci->mci_DateStamp);

		    /* Create the message ports that we need */
		    if (CreateMsgPorts (gd))
		    {
			/* Set the commodity message port */
			nb.nb_Port = gd->gd_CxPort;

			/* Create the commodity broker */
			if (gd->gd_CxObj = CxBroker (&nb, NULL))
			{
			    /* Create the rendevous port */
			    if (gd->gd_MsgPort = CreateMsgPort ())
			    {
				gd->gd_MsgPort->mp_Node.ln_Name = NAME;
				gd->gd_MsgPort->mp_Node.ln_Pri = -1;
				AddPort (gd->gd_MsgPort);
			    }

			    /* Install a hotkey for popping up window */
			    gd->gd_CxFilter = CxFilter (gd->gd_CxPopKey);
			    AttachCxObj (gd->gd_CxFilter, CxSender (gd->gd_CxPort, 1L));
			    AttachCxObj (gd->gd_CxFilter, CxTranslate (NULL));
			    AttachCxObj (gd->gd_CxObj, gd->gd_CxFilter);

			    /* Activate the commodity */
			    ActivateCxObj (gd->gd_CxObj, 1L);

			    /* Turn off everything */
			    Disable ();

			    /* Clear all free memory to 0xDEADBEEF */
			    SetMemory (gd, CHECK_FREE);

			    /* Install our hooks */
			    gd->gd_AllocMem = SetFunction (SysBase, LVO_ALLOCMEM, (ULONG (*) ()) nAllocMem);
			    gd->gd_AllocVec = SetFunction (SysBase, LVO_ALLOCVEC, (ULONG (*) ()) nAllocVec);
			    gd->gd_AllocAbs = SetFunction (SysBase, LVO_ALLOCABS, (ULONG (*) ()) nAllocAbs);
			    gd->gd_FreeMem  = SetFunction (SysBase, LVO_FREEMEM, (ULONG (*) ()) nFreeMem);
			    gd->gd_FreeVec  = SetFunction (SysBase, LVO_FREEVEC, (ULONG (*) ()) nFreeVec);
			    gd->gd_AvailMem = SetFunction (SysBase, LVO_AVAILMEM, (ULONG (*) ()) nAvailMem);

			    /* Enable everything */
			    Enable ();

			    /* Get the signal bits that we're going to wait on */
			    sigc = 1L << gd->gd_CxPort->mp_SigBit;
			    sigw = 0;

			    /* Indicate success */
			    failureLevel = RETURN_OK;

			    /* Keep going until the end */
			    gd->gd_Going = TRUE;
			    while (gd->gd_Going)
			    {
				/* See if we are supposed to close our window */
				if (gd->gd_Flags & GDF_CLOSEWINDOW)
				{
				    gd->gd_Flags &= ~GDF_CLOSEWINDOW;
				    CloseCWindow (gd);
				    sigw = 0;
				}

				/* See if we are supposed to open our window */
				if (gd->gd_Flags & GDF_OPENWINDOW)
				{
				    if (OpenCWindow (gd))
				    {
					gd->gd_Flags &= ~GDF_OPENWINDOW;
					sigw = 1L << gd->gd_Window->UserPort->mp_SigBit;
				    }
				}

				/* Wait for something to happen */
				rsig = Wait (SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F | sigc | sigw);

				if (rsig & SIGBREAKF_CTRL_C)
				    gd->gd_Going = FALSE;

				if (rsig & SIGBREAKF_CTRL_E)
				    gd->gd_Flags |= GDF_OPENWINDOW;

				if (rsig & SIGBREAKF_CTRL_F)
				    FreePrivateList (gd);

				/* Did we get a commodity message? */
				if (rsig & sigc)
				{
				    while (cmsg = (CxMsg *) GetMsg (gd->gd_CxPort))
				    {
					msgid = CxMsgID (cmsg);
					msgtype = CxMsgType (cmsg);

					switch (msgtype)
					{
					    case CXM_IEVENT:
						switch (msgid)
						{
						    case 1:
							gd->gd_Flags |= GDF_OPENWINDOW;
							break;
						}
						break;

					    case CXM_COMMAND:
						switch (msgid)
						{
						    case CXCMD_DISABLE:
							ActivateCxObj (gd->gd_CxObj, 0L);
							gd->gd_Flags &= ~GDF_ACTIVE;
							break;

						    case CXCMD_ENABLE:
							DateStamp (&mci->mci_DateStamp);
							ActivateCxObj (gd->gd_CxObj, 1L);
							gd->gd_Flags |= GDF_ACTIVE;
							break;

						    case CXCMD_UNIQUE:
						    case CXCMD_APPEAR:
							gd->gd_Flags |= GDF_OPENWINDOW;
							break;

						    case CXCMD_DISAPPEAR:
							gd->gd_Flags |= GDF_CLOSEWINDOW;
							break;

						    case CXCMD_KILL:
							gd->gd_Going = FALSE;
							break;
						}
						break;
					}

					ReplyMsg ((struct Message *) cmsg);
				    }
				}

				/* Do we have a window open? */
				if (gd->gd_Window)
				{
				    /* Pull all the window messages */
				    while (imsg = GT_GetIMsg (gd->gd_Window->UserPort))
				    {
					switch (imsg->Class)
					{
					    case IDCMP_CLOSEWINDOW:
						gd->gd_Flags |= GDF_CLOSEWINDOW;
						break;

					    case IDCMP_MENUPICK:
						id = imsg->Code;
						while ((id != MENUNULL) && gd->gd_Going)
						{
						    menuitem = ItemAddress (imsg->IDCMPWindow->MenuStrip, id);
						    ProcessCommand (gd, (ULONG) MENU_USERDATA (menuitem), imsg);
						    id = menuitem->NextSelect;
						}
						break;

					    case IDCMP_GADGETUP:
						ProcessCommand (gd, (ULONG) ((struct Gadget *) imsg->IAddress)->GadgetID, imsg);
						break;
					}

					GT_ReplyIMsg (imsg);
				    }
				}
			    }

			    /* If the window is open, then close it */
			    CloseCWindow (gd);

			    Disable ();

			    /* Free the temporary memory storage */
			    FreeMemHeader (gd, gd->gd_MemHeader);

			    /* Remove our hooks */
			    SetFunction (SysBase, LVO_AVAILMEM, (APTR) gd->gd_AvailMem);
			    SetFunction (SysBase, LVO_FREEVEC,  (APTR) gd->gd_FreeVec);
			    SetFunction (SysBase, LVO_FREEMEM,  (APTR) gd->gd_FreeMem);
			    SetFunction (SysBase, LVO_ALLOCABS, (APTR) gd->gd_AllocAbs);
			    SetFunction (SysBase, LVO_ALLOCVEC, (APTR) gd->gd_AllocVec);
			    SetFunction (SysBase, LVO_ALLOCMEM, (APTR) gd->gd_AllocMem);

			    Enable ();

			    /* Remove the commodity */
			    DeleteCxObjAll (gd->gd_CxObj);
			    while (cmsg = (CxMsg *) GetMsg (gd->gd_CxPort))
				ReplyMsg ((struct Message *) cmsg);

			    /* Remove the rendevous port */
			    if (gd->gd_MsgPort)
			    {
				RemPort (gd->gd_MsgPort);
				DeleteMsgPort (gd->gd_MsgPort);
			    }
			}

			/* Delete the message ports */
			DeleteMsgPorts (gd);
		    }
		}
	    }

	    /* Free the shell arguments */
	    FreeArgs (gd->gd_RDArgs);
	}
	else
	    lprintf (gd, "couldn't open commodities.library V37\n", NULL);

	/* Close the libraries */
	CloseLibrary (gd->gd_CxBase);
	CloseLibrary (gd->gd_GadToolsBase);
	CloseLibrary (gd->gd_IntuitionBase);
	CloseLibrary (gd->gd_UtilityBase);
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

    pr->pr_Result2 = failureCode;
    return (failureLevel);
}

/*****************************************************************************/

#define	SysBase			 gd->gd_SysBase

/*****************************************************************************/

struct Gadget *creategadget (struct GlobalData * gd, ULONG kind, struct Gadget * prev, struct NewGadget * ng, ULONG data,...)
{

    return (CreateGadgetA (kind, prev, ng, (struct TagItem *) & data));
}

/*****************************************************************************/

void setgadgetattrs (struct GlobalData * gd, ULONG id, ULONG data,...)
{

    GT_SetGadgetAttrsA (gd->gd_Gads[id], gd->gd_Window, NULL, (struct TagItem *) & data);
}

/*****************************************************************************/

struct Menu *createmenus (struct GlobalData * gd, struct NewMenu * nm, ULONG data,...)
{

    return (CreateMenusA (nm, (struct TagItem *) & data));
}

/*****************************************************************************/

BOOL layoutmenus (struct GlobalData * gd, ULONG data,...)
{

    return (LayoutMenusA (gd->gd_Menu, gd->gd_VI, (struct TagItem *) & data));
}

/*****************************************************************************/

struct Window *openwindowtags (struct GlobalData * gd, ULONG data,...)
{

    return (OpenWindowTagList (NULL, (struct TagItem *) & data));
}

/*****************************************************************************/

void lprintf (struct GlobalData * gd, STRPTR fmt, void *arg1,...)
{

    VPrintf (fmt, (LONG *) & arg1);
}

/*****************************************************************************/

VOID FindLayers (struct GlobalData * gd)
{
    struct Resident *rt;
    LONG i, j, k;
    ULONG *rm;

    if (rt = FindResident ("layers.library"))
    {
	gd->gd_LStart = gd->gd_LEnd = (ULONG) rt;
	rm = (ULONG *) SysBase->ResModules;

	/* It's unlikely layers will end up being 256 K */
	gd->gd_LEnd += 0x40000;
	j = 1000;
	k = 0;
	/* Bubble up gd->gd_LEnd */
	for (i = 0; i <= j; i++)
	{
	    while (*rm != NULL)
	    {
		if (*rm > gd->gd_LStart && *rm < gd->gd_LEnd)
		    gd->gd_LEnd = *rm;
		*rm++;
		k++;
	    }
	    j = k;
	    k = 0;
	    rm = (ULONG *) SysBase->ResModules;
	}
    }
}

/*****************************************************************************/

void GetIconArgs (struct GlobalData * gd, struct WBArg * wa)
{
    struct MemCheckInfo *mci = &gd->gd_MemCheckInfo;
    struct Library *IconBase;
    struct DiskObject *dob;
    BPTR oldLock;
    STRPTR tmp;
    ULONG hold;

    if (IconBase = OpenLibrary ("icon.library", 37))
    {
	/* Make it the current directory */
	oldLock = CurrentDir (wa->wa_Lock);

	/* Get the tool icon */
	if (dob = GetDiskObject (wa->wa_Name))
	{
	    /* What's the priority of our commodity */
	    if (tmp = FindToolType (dob->do_ToolTypes, "CX_PRIORITY"))
	    {
		StrToLong (tmp, &hold);
		nb.nb_Pri = (BYTE) hold;
	    }

	    /* Should the window start out open? */
	    if (tmp = FindToolType (dob->do_ToolTypes, "CX_POPUP"))
		if (MatchToolValue (tmp, "NO"))
		    gd->gd_Flags &= ~GDF_OPENWINDOW;

	    /* Find the window popup key */
	    if (tmp = FindToolType (dob->do_ToolTypes, "CX_POPKEY"))
		gd->gd_CxPopKey = tmp;

	    /* Get the public screen name */
	    if (tmp = FindToolType (dob->do_ToolTypes, "PUBSCREEN"))
	    {
		strcpy (gd->gd_ScreenNameBuffer, tmp);
		gd->gd_ScreenName = gd->gd_ScreenNameBuffer;
	    }

	    /* How many lines of stack should we show */
	    if (tmp = FindToolType (dob->do_ToolTypes, "STACKLINES"))
	    {
		StrToLong (tmp, &hold);
		mci->mci_Stack = (UWORD) hold;
	    }

	    /* Supposed to be compatible with MemCheck? */
	    if (FindToolType (dob->do_ToolTypes, "COMPAT"))
		gd->gd_Flags |= GDF_COMPATIBLE;

	    /* Free the icon */
	    FreeDiskObject (dob);
	}

	/* Go back to the previous directory */
	CurrentDir (oldLock);

	CloseLibrary (IconBase);
    }
}

/*****************************************************************************/

BOOL CreateMsgPorts (struct GlobalData * gd)
{

    if (gd->gd_CxPort = CreateMsgPort ())
	return TRUE;
    return FALSE;
}

/*****************************************************************************/

void DeleteMsgPorts (struct GlobalData * gd)
{

    DeleteMsgPort (gd->gd_CxPort);
}

/*****************************************************************************/

void ProcessCommand (struct GlobalData * gd, ULONG id, struct IntuiMessage * imsg)
{

    switch (id)
    {
	    case MMC_HIDE:
	    gd->gd_Flags |= GDF_CLOSEWINDOW;
	    break;

	case MMC_QUIT:
	    gd->gd_Going = FALSE;
	    break;

	case GID_SET_FREE:
	    SetMemory (gd, CHECK_FREE);
	    break;

	case GID_SET_ALLOC:
	    SetMemory (gd, CHECK_ALLOC);
	    break;

	case GID_SET_ALL:
	    SetMemory (gd, CHECK_FREE | CHECK_ALLOC);
	    break;

	case GID_CHECK_FREE:
	    CheckMemory (gd, CHECK_FREE);
	    break;

	case GID_CHECK_ALLOC:
	    CheckMemory (gd, CHECK_ALLOC);
	    break;

	case GID_CHECK_ALL:
	    CheckMemory (gd, CHECK_FREE | CHECK_ALLOC);
	    break;
    }
}

/*****************************************************************************/

void CheckMemory (struct GlobalData * gd, ULONG flags)
{
    struct MemChunk *chunk;
    struct MemHeader *mem;
    ULONG ustart, usize;
    ULONG fstart, fsize;
    ULONG addrs, value;
    ULONG errorcnt = 0;
    ULONG cnt, sum;
    BOOL error;
    char *ptr;
    ULONG i;

    /* Display header */
    kprintf ("Check all memory for random writes\n");

    /* Disable multitasking */
    Forbid ();

    /* Step through the system memory free list */
    for (mem = (struct MemHeader *) SysBase->MemList.lh_Head;
	 mem->mh_Node.ln_Succ;
	 mem = (struct MemHeader *) mem->mh_Node.ln_Succ)
    {
	/* Show the memory header */
	kprintf ("Lower=%08lx Upper=%08lx Size=%-10ld A=%04lx\n",
		 mem->mh_Lower, mem->mh_Upper, mem->mh_Free, (ULONG) mem->mh_Attributes);

	/* See if there is allocated memory before the first chunk */
	if ((flags & CHECK_ALLOC) && ((ULONG) mem->mh_First != (ULONG) mem->mh_Lower))
	{
	    ustart = (ULONG) mem->mh_Lower;
	    usize = (ULONG) mem->mh_First - (ULONG) mem->mh_Lower;
	    errorcnt += CheckAllocatedMemory (gd, (ULONG *) ustart, usize);
	}

	/* Make sure we have a list of memory chunks */
	if (mem->mh_First)
	{
	    /* Step through the memory chunks for this block of memory */
	    for (chunk = mem->mh_First, cnt = sum = 0, fstart = (ULONG) mem->mh_First;
		 chunk;
		 chunk = chunk->mc_Next, fstart = (ULONG) chunk, cnt++)
	    {
		/* Sum the amount of memory in the list */
		sum += chunk->mc_Bytes;

		/* Remember the size */
		fsize = chunk->mc_Bytes;

		if (chunk->mc_Next)
		{
		    if (flags & CHECK_ALLOC)
		    {
			ustart = fstart + fsize;
			usize = ((ULONG) chunk->mc_Next) - ustart;
			errorcnt += CheckAllocatedMemory (gd, (ULONG *) ustart, usize);
		    }
		}
		else if ((flags & CHECK_ALLOC) && ((ustart = fstart + fsize) < (ULONG) mem->mh_Upper))
		{
		    usize = (ULONG) mem->mh_Upper - ustart;
		    errorcnt += CheckAllocatedMemory (gd, (ULONG *) ustart, usize);
		}

		/* Make sure we have memory to check */
		if ((flags & CHECK_FREE) && (chunk->mc_Bytes > 8))
		{
		    /* Go past the header */
		    ptr = (char *) chunk;
		    ptr += 8;

		    /* Make sure memory contains the DEADBEEF pattern */
		    for (i = 0, error = FALSE; i < chunk->mc_Bytes - 8; i += 4)
		    {
			/* See if memory contains DEADBEEF */
			if (!((ptr[i + 0] == 0xDE) && (ptr[i + 1] == 0xAD) &&
			      (ptr[i + 2] == 0xBE) && (ptr[i + 3] == 0xEF)))
			{
			    /* Convert this block and the next into long words */
			    addrs = MAKE_LONG (ptr[i + 0], ptr[i + 1], ptr[i + 2], ptr[i + 3]);
			    value = MAKE_LONG (ptr[i + 4], ptr[i + 5], ptr[i + 6], ptr[i + 7]);

			    /* See if it is a coalesced memory chunk */
			    if ((addrs > (ULONG) chunk) && (addrs <= ((ULONG) mem->mh_Upper)) &&
				(value > 0) && (value < chunk->mc_Bytes))
			    {
				/* Restore dead beef */
				ptr[i + 0] = 0xDE;
				ptr[i + 1] = 0xAD;
				ptr[i + 2] = 0xBE;
				ptr[i + 3] = 0xEF;
				ptr[i + 4] = 0xDE;
				ptr[i + 5] = 0xAD;
				ptr[i + 6] = 0xBE;
				ptr[i + 7] = 0xEF;
			    }
			    else
			    {
				/* Found an error */
				error = TRUE;
			    }
			}
		    }

		    /* If there was an error, then display the memory that had it */
		    if (error)
		    {
			/* Increment the error count */
			errorcnt++;

			/* Display the block information */
			kprintf (" %08lx,%-6ld\n", chunk, chunk->mc_Bytes);

			/* Dump the garbaged memory */
			MemoryDump (gd, "  ", ptr, chunk->mc_Bytes - 8);
		    }
		}
	    }

	    if (sum != mem->mh_Free)
	    {
		kprintf (" Found %ld chunks, totalling %ld bytes.\n", cnt, sum);
		kprintf (" !!! memory list insane !!!\n");
	    }
	}
	else
	    kprintf (" No chunk list.\n");
    }

    /* Enable multitasking */
    Permit ();

    /* Print trailer */
    kprintf ("Found %ld blocks with errors\n\n", errorcnt);
}

/*****************************************************************************/

void ShowMungBlock (struct GlobalData * gd, struct MemCheck * mc)
{
    struct MemCheckInfo *mci = &gd->gd_MemCheckInfo;

    /* Show the memory block information */
    kprintf (" %08lx,%-6ld TCB=%08lx %.16s\n",
	     (ULONG) mc + INFOSIZE + mci->mci_PreSize, mc->mc_Size, mc->mc_Task, mc->mc_Name);
}

/*****************************************************************************/

ULONG CheckAllocatedMemory (struct GlobalData * gd, ULONG * ustart, ULONG usize)
{
    struct MemCheckInfo *mci = &gd->gd_MemCheckInfo;
    struct MemCheck *mc;
    ULONG errorcnt = 0;
    ULONG *uend, *up;
    register ULONG i;
    LONG errors;
    ULONG *zero;
    BOOL error;
    UBYTE *ptr;
    UWORD *wp;

    /* Compute the end */
    uend = (ULONG *) (((ULONG) ustart) + usize - 4);

    /* Step through the allocated memory, searching for a MemCheck cookie */
    for (up = ustart; up < uend; up += 1)
    {

	/*
	 * Check for C0DEF00D in two pieces, so loadsegged munglist program doesn't cause a bogus match.
	 */
	wp = (UWORD *) up;
	if ((wp[0] == 0xC0DE) && (wp[1] == 0xF00D) && (up[1] <= usize))
	{
	    /* No error yet */
	    error = FALSE;

	    /* Cast the memory block to a MemCheck structure */
	    mc = (struct MemCheck *) up;

	    /* Generate checksum and check it */
	    zero = (ULONG *) up;
	    errors = *zero++ + *zero++ + *zero++ + *zero++ + *zero++;
	    if (*zero != errors)
	    {
		/* Show the memory block */
		ShowMungBlock (gd, mc);

		/* Show the error message */
		kprintf ("  Cookie checksum failure\n");

		/* Show that we had an error */
		error = TRUE;
	    }

	    /* Examine pre-wall for damage */
	    ptr = (char *) up + INFOSIZE;
	    for (i = errors = 0; i < mci->mci_PreSize; i++)
		if (ptr[i] != mci->mci_FillChar)
		    errors++;
	    if (errors)
	    {
		/* Show the memory block */
		ShowMungBlock (gd, mc);

		/* Show the error message */
		kprintf ("  At least %ld bytes before allocation were hit.\n", errors);

		/* Show the trashed wall */
		MemoryDump (gd, "  ", ptr, mci->mci_PreSize);

		/* Show that we had an error */
		error = TRUE;
	    }

	    /* Examine post-wall for damage */
	    ptr = (char *) up + INFOSIZE + mci->mci_PreSize + mc->mc_Size;
	    for (i = errors = 0; i < mci->mci_PostSize; i++)
		if (ptr[i] != mci->mci_FillChar)
		    errors++;
	    if (errors)
	    {
		/* Show the memory block */
		ShowMungBlock (gd, mc);

		/* Show the error message */
		kprintf ("  At least %ld bytes after allocation were hit.\n", errors);

		/* Show the trashed wall */
		MemoryDump (gd, "  ", ptr, mci->mci_PostSize);

		/* Show that we had an error */
		error = TRUE;
	    }

	    /* Any errors, then increment the counter */
	    if (error)
		errorcnt++;

	    /* Next... */
	    up += (up[1] >> 2);
	}
    }

    return errorcnt;
}
