/* dbdispatch.c
 *
 */

#include "amigaguidebase.h"
#include "hosthandle.h"
#include <stdlib.h>

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

#define	AG_PREFIX	"@{"
#define	AG_PREFIX_LEN	2

/*****************************************************************************/

static STRPTR errMsgs[] =
{
    "missing argument(s)",
    "couldn't create node",
    "must be inside a node",
    "line too long",
    "doesn't contain any nodes",
    "missing endnode(s)",
};

/*****************************************************************************/

static STRPTR warnMsgs[] =
{
    "undefined keyword",
};

/*****************************************************************************/

enum
{
    DBDK_DATABASE,
    DBDK_AUTHOR,
    DBDK_ANNOTATION,			/* Not used */
    DBDK_COPYRIGHT,
    DBDK_VERSION,

    DBDK_XREF,
    DBDK_NAME,				/* Not used */
    DBDK_INDEX,
    DBDK_HELP,

    DBDK_TITLE,
    DBDK_TOC,
    DBDK_PREVIOUS,
    DBDK_NEXT,
    DBDK_KEYWORDS,

    DBDK_ONOPEN,
    DBDK_ONCLOSE,

    DBDK_DNODE,
    DBDK_EMBED,
    DBDK_ENDNODE,
    DBDK_FONT,
    DBDK_HEIGHT,
    DBDK_MACRO,
    DBDK_MASTER,
    DBDK_NODE,
    DBDK_PROPORTIONAL,
    DBDK_REMARK,
    DBDK_SMARTWRAP,
    DBDK_TAB,
    DBDK_WIDTH,
    DBDK_WORDWRAP,
};

/*****************************************************************************/

static const struct Keyword dbd_Keywords[] =
{
    {"$VER:",		DBDK_VERSION},
    {"(C)",		DBDK_COPYRIGHT},
    {"AUTHOR",		DBDK_AUTHOR},
    {"DATABASE",	DBDK_DATABASE},
    {"DNODE",		DBDK_DNODE},
    {"EMBED",		DBDK_EMBED},
    {"ENDNODE",		DBDK_ENDNODE},
    {"FONT",		DBDK_FONT},
    {"HEIGHT",		DBDK_HEIGHT},
    {"HELP",		DBDK_HELP},
    {"INDEX",		DBDK_INDEX},
    {"KEYWORDS",	DBDK_KEYWORDS},
    {"MACRO",		DBDK_MACRO},
    {"MASTER",		DBDK_MASTER},
    {"NEXT",		DBDK_NEXT},
    {"NODE",		DBDK_NODE},
    {"ONCLOSE",		DBDK_ONCLOSE},
    {"ONOPEN",		DBDK_ONOPEN},
    {"PREV",		DBDK_PREVIOUS},
    {"PROPORTIONAL",	DBDK_PROPORTIONAL},
    {"REM",		DBDK_REMARK},
    {"REMARK",		DBDK_REMARK},
    {"SMARTWRAP",	DBDK_SMARTWRAP},
    {"TAB",		DBDK_TAB},
    {"TITLE",		DBDK_TITLE},
    {"TOC",		DBDK_TOC},
    {"WIDTH",		DBDK_WIDTH},
    {"WORDWRAP",	DBDK_WORDWRAP},
    {"XREF",		DBDK_XREF},
};

#define	DBDK_TOTAL_KEYWORDS	(sizeof (dbd_Keywords) / sizeof (struct Keyword))

/*****************************************************************************/

static LONG dbd_debugPrintf (struct AGLib *ag, struct DatabaseData *dbd, STRPTR fmt, LONG argv,...)
{
    LONG retval = -1;

    if (dbd->dbd_Flags & DBDF_DEBUG)
    {
	if (dbd->dbd_ErrorFH == NULL)
	{
	    if (Stricmp (dbd->dbd_ErrorName, "ON") == 0)
		strcpy (dbd->dbd_ErrorName, "CON:0/0/400/75/AmigaGuide errors/CLOSE/WAIT");
	    dbd->dbd_ErrorFH = Open (dbd->dbd_ErrorName, MODE_NEWFILE);
	}

	if (dbd->dbd_ErrorFH)
	    retval = VFPrintf (dbd->dbd_ErrorFH, fmt, (LONG *) & argv);
    }

    return (retval);
}

/*****************************************************************************/

static void dbd_closeDebug (struct AGLib *ag, struct DatabaseData *dbd, LONG mode)
{
    /* Close the error file */
    if (dbd->dbd_ErrorFH)
    {
	if (mode == 1)
	    dbd_debugPrintf (ag, dbd, "\n%s: %ld Errors, %ld Warnings\n",
			 (LONG) dbd->dbd_Name, (LONG) dbd->dbd_Errors, (LONG) dbd->dbd_Warnings);

	Close (dbd->dbd_ErrorFH);
	dbd->dbd_ErrorFH = NULL;
    }
}

/*****************************************************************************/

static ULONG dbd_disposeDatabase (struct AGLib *ag, Class * cl, Object * o, Msg msg, struct DatabaseData *dbd)
{
    struct Node *ln;
    Object *ostate;
    Object *member;
    LONG i;

    /* Execute the close database command */
    if (dbd->dbd_OnClose)
    {
	sprintf (dbd->dbd_ErrorLine, "%s %s", dbd->dbd_OnClose, dbd->dbd_FullName);
	m_sendCmd (ag, dbd->dbd_ErrorLine);
    }

    /* Remove all the nodes from the node list */
    ostate = (Object *) dbd->dbd_NodeList.mlh_Head;
    while (member = NextObject (&ostate))
    {
	DoMethod (member, OM_REMOVE);
	DoMethodA (member, msg);
    }

    /* Free the macro list */
    while (ln = RemHead ((struct List *) &dbd->dbd_MacroList))
	FreeVec (ln);

    /* Close the fonts */
    if (dbd->dbd_Font)
	CloseFont (dbd->dbd_Font);
    for (i = 0; i < MAX_FONTS; i++)
    {
	if (dbd->dbd_Fonts[i])
	    CloseFont (dbd->dbd_Fonts[i]);
    }

    /* Close the file */
    if (dbd->dbd_FileHandle)
	Close (dbd->dbd_FileHandle);

    /* Unlock the lock */
    UnLock (dbd->dbd_FileLock);

    /* Free other variables */
    for (i = 0; i < DBD_STRINGS; i++)
	FreeVec (dbd->dbd_Strings[i]);

    /* Pass it up to the superclass */
    DoSuperMethodA (cl, o, msg);

    return (1L);
}

/*****************************************************************************/

/* Set attributes of an object */
static ULONG dbd_setAttrs (struct AGLib * ag, Class * cl, Object * o, struct opSet * msg)
{
    struct DatabaseData *dbd = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG refresh = 0L;
    ULONG tidata;
    ULONG i;

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case DBA_Name:
	    case DBA_Index:
	    case DBA_Help:
	    case DBA_OnOpen:
	    case DBA_OnClose:
		i = tag->ti_Tag - DBA_Name;
		FreeVec (dbd->dbd_Strings[i]);
		dbd->dbd_Strings[i] = NULL;
		if (tidata && (dbd->dbd_Strings[i] = AllocVec (strlen ((STRPTR) tidata) + 1, MEMF_CLEAR)))
		    strcpy (dbd->dbd_Strings[i], (STRPTR) tidata);
		break;

	    case DBA_FileHandle:
		dbd->dbd_FileHandle = (BPTR) tidata;
		break;
#if 0
	    /* Set the cross reference for a database */
	    case DBA_XRef:
		break;
#endif
	}
    }

    return (refresh);
}

/*****************************************************************************/

static BOOL dbd_parseCommand (struct AGLib * ag, Object * o, struct DatabaseData * dbd, STRPTR cmd, ULONG * text)
{
    struct NodeData *nd;
    BOOL retval = FALSE;
    STRPTR argv[MAXARG];
    ULONG errnum = 0;
    ULONG token;
    ULONG argc;

    *text = FALSE;
    if ((argc = ParseString (ag, cmd, argv, MAXARG)) > 0)
    {
	retval = TRUE;

	if (strncmp (argv[0], AG_PREFIX, AG_PREFIX_LEN) == 0)
	{
	    if (dbd->dbd_CurrentNode)
		*text = TRUE;
	    else
		errnum = 3;
	}
	else
	{
	    /* Get the node data */
	    nd = NULL;
	    if (dbd->dbd_CurrentNode)
		nd = INST_DATA (ag->ag_NodeClass, dbd->dbd_CurrentNode);

	    /* Upper case the command to make comparision quicker */
	    StrToUpper (ag, argv[0]);

	    /* Tokenize and handle the command */
	    switch (token = m_binSearch (&argv[0][1], strlen (&argv[0][1]), dbd_Keywords, DBDK_TOTAL_KEYWORDS))
	    {
		/***************************************************************************/
		/***************************************************************************/
		/***************************************************************************/
		case DBDK_DATABASE:
		case DBDK_AUTHOR:
		case DBDK_ANNOTATION:
		case DBDK_COPYRIGHT:
		case DBDK_VERSION:
		    if (argc > 1)
			setattrs (ag, o, DTA_ObjName + (token - DBDK_DATABASE), argv[1], TAG_DONE);
		    else
			errnum = 1;
		    break;

		/***************************************************************************/
		/***************************************************************************/
		/***************************************************************************/
		case DBDK_XREF:
		case DBDK_INDEX:
		case DBDK_HELP:
		    if (argc > 1)
			setattrs (ag, o, DBA_XRef + (token - DBDK_XREF), argv[1], TAG_DONE);
		    else
			errnum = 1;
		    break;

		/***************************************************************************/
		/***************************************************************************/
		/***************************************************************************/
		case DBDK_TITLE:
		case DBDK_TOC:
		case DBDK_PREVIOUS:
		case DBDK_NEXT:
		case DBDK_KEYWORDS:
		    if (dbd->dbd_CurrentNode && (argc > 1))
			setattrs (ag, dbd->dbd_CurrentNode, HNA_Title + (token - DBDK_TITLE), argv[1], TAG_DONE);
		    else if (!dbd->dbd_CurrentNode)
			errnum = 3;
		    else
			errnum = 1;
		    break;

		/***************************************************************************/
		/***************************************************************************/
		/***************************************************************************/
		case DBDK_ONOPEN:
		case DBDK_ONCLOSE:
		    if (argc > 1)
		    {
			if (dbd->dbd_CurrentNode)
			    setattrs (ag, dbd->dbd_CurrentNode, HNA_OnOpen + (token - DBDK_ONOPEN), argv[1], TAG_DONE);
			else
			    setattrs (ag, o, DBA_OnOpen + (token - DBDK_ONOPEN), argv[1], TAG_DONE);
		    }
		    else
			errnum = 1;
		    break;

		/***************************************************************************/
		/***************************************************************************/
		/***************************************************************************/
		case DBDK_PROPORTIONAL:
		    if (dbd->dbd_CurrentNode)
			nd->nd_Flags |= HNF_PROPORTIONAL;
		    else
			dbd->dbd_Flags |= DBDF_PROPORTIONAL;
		    break;

		case DBDK_SMARTWRAP:
		    if (dbd->dbd_CurrentNode)
			nd->nd_Flags |= HNF_SMARTWRAP;
		    else
			dbd->dbd_Flags |= DBDF_SMARTWRAP;
		    break;

		case DBDK_WORDWRAP:
		    if (dbd->dbd_CurrentNode)
			nd->nd_Flags |= HNF_WRAP;
		    else
			dbd->dbd_Flags |= DBDF_WRAP;
		    break;

		/***************************************************************************/
		/***************************************************************************/
		/***************************************************************************/
		case DBDK_ENDNODE:
		    if (dbd->dbd_CurrentNode)
		    {
			dbd->dbd_NumEndNodes++;
			if (nd->nd_Offset == (-1))
			    nd->nd_Offset = dbd->dbd_Offset;
			nd->nd_BufferLen = dbd->dbd_Offset - nd->nd_Offset;
		    }
		    else
			errnum = 3;
		    dbd->dbd_CurrentNode = NULL;
		    break;

		case DBDK_FONT:
		    if (argc > 2)
		    {
			ULONG *font, size = 0;
			struct TextAttr *ta;
			STRPTR name;

			if (dbd->dbd_CurrentNode)
			{
			    font = (ULONG *) & nd->nd_Font;
			    name = nd->nd_FontName;
			    ta = &nd->nd_TextAttr;
			}
			else
			{
			    font = (ULONG *) & dbd->dbd_Font;
			    name = dbd->dbd_FontName;
			    ta = &dbd->dbd_TextAttr;
			}

			strncpy (name, argv[1], 32);
			StrToLong (argv[2], (LONG *) & size);

			ta->ta_Name = name;
			ta->ta_YSize = (UWORD) size;
			ta->ta_Style = NULL;
			ta->ta_Flags = NULL;

			if (*font)
			    CloseFont ((struct TextFont *) *font);
			if (!(*font = (ULONG) OpenDiskFont (ta)))
			    *font = (ULONG) OpenFont (ta);
		    }
		    else
			errnum = 1;
		    break;

		case DBDK_MACRO:
		    if (argc >= 2)
		    {
			ULONG size1, size2, size3;
			struct MacroNode *mn;

			/* How much space do we need */
			size1 = sizeof (struct MacroNode);

			size2 = strlen (argv[1]) + 2;
			size3 = strlen (argv[2]) + 2;

			/* Allocate a node structure */
			if (mn = AllocVec (size1 + size2 + size3, MEMF_CLEAR))
			{
			    /* Copy the macro name */
			    mn->mn_Node.ln_Name = (char *) mn + size1;
			    strcpy (mn->mn_Node.ln_Name, argv[1]);
			    StrToUpper (ag, mn->mn_Node.ln_Name);

			    /* Copy the macro */
			    mn->mn_Macro = mn->mn_Node.ln_Name + size2;
			    strcpy (mn->mn_Macro, argv[2]);

			    /* Add the node to the list */
			    AddTail ((struct List *) &dbd->dbd_MacroList, &mn->mn_Node);
			}
		    }
		    else
			errnum = 1;
		    break;

		case DBDK_MASTER:
		    if (argc > 1)
		    {
			struct Process *pr = (struct Process *) FindTask (NULL);
			struct FileInfoBlock *fib;
			struct DateStamp mds;
			APTR oldWP;
			BPTR lock;

			/* Turn off requesters for a moment */
			oldWP = pr->pr_WindowPtr;
			pr->pr_WindowPtr = (APTR) - 1;

			/* Try to lock the master file */
			if (lock = Lock (argv[1], ACCESS_READ))
			{
			    /* Get a temporary FIB to work with */
			    if (fib = AllocDosObject (DOS_FIB, NULL))
			    {
				/* Examine the master file */
				if (Examine (lock, fib))
				{
				    /* Copy the date of the master file. */
				    mds = *(&fib->fib_Date);

				    /* Examine the AmigaGuide file */
				    if (Examine (dbd->dbd_FileLock, fib))
				    {
					/* Compare the dates to see if the master has been updated. */
					if (CompareDates (&mds, &fib->fib_Date) < 0)
					{
					    /* Show that the file is stale */
					    dbd->dbd_Flags |= DBDF_STALE;
					}
				    }
				}
				/* Get rid of the temporary FIB */
				FreeDosObject (DOS_FIB, fib);
			    }
			    /* Unlock the master */
			    UnLock (lock);
			}

			/* Restore the window pointer */
			pr->pr_WindowPtr = oldWP;
		    }
		    else
			errnum = 1;
		    break;

		case DBDK_NODE:
		    /* End any node */
		    if (dbd->dbd_CurrentNode)
		    {
			errnum = 6;
		    }
		    /* Create a new Node object */
		    else if (argc > 1)
		    {
			STRPTR title, name;

			name = argv[1];
			title = (argv[2]) ? argv[2] : name;

			if (dbd->dbd_CurrentNode = NewObject (ag->ag_NodeClass, NULL,
								HNA_Name,	name,
								HNA_Title,	title,
								HNA_Database,	o,
								HNA_FileHandle,	dbd->dbd_FileHandle,
								TAG_DONE))
			    dbd->dbd_NumNodes++;
			else
			    errnum = 2;
		    }
		    else
			errnum = 1;
		    break;

		case DBDK_TAB:
		    if (argc > 1)
		    {
			ULONG i, j;

			StrToLong (argv[1], (LONG *) &i);
			if (dbd->dbd_CurrentNode)
			{
			    nd->nd_DefTabs = i;
			    for (i = j = 0; i < MAX_TABS; i++, j += nd->nd_DefTabs)
				nd->nd_Tabs[i] = j;
			}
			else
			{
			    dbd->dbd_DefTabs = i;
			    for (i = j = 0; i < MAX_TABS; i++, j += dbd->dbd_DefTabs)
				dbd->dbd_Tabs[i] = j;
			}
		    }
		    else
			errnum = 1;
		    break;

		/***************************************************************************/
		/***************************************************************************/
		/***************************************************************************/
		case DBDK_EMBED:
#if 0
		    if (dbd->dbd_CurrentNode && (argc > 1))
			setattrs (ag, dbd->dbd_CurrentNode,
				  HNA_EmbedName, argv[1],
				  HNA_EmbedLine, dbd->dbd_Offset - nd->nd_Offset,
				  TAG_DONE);
		    else if (!dbd->dbd_CurrentNode)
			errnum = 3;
		    else
			errnum = 1;
		    break;
#endif
		case DBDK_HEIGHT:
		case DBDK_REMARK:
		case DBDK_WIDTH:
		    break;

		/***************************************************************************/
		/***************************************************************************/
		/***************************************************************************/
		case DBDK_DNODE:
		    retval = FALSE;
		    break;

		default:
		    errnum = 100;
		    break;
	    }
	}
    }

    if (errnum)
    {
	if (errnum < 100)
	{
	    dbd->dbd_Errors++;
	    dbd_debugPrintf (ag, dbd, "%s, Line %ld, Error %ld: %s\n",
			 (LONG) dbd->dbd_Name, (LONG) dbd->dbd_NumLines, (LONG) errnum, (LONG) errMsgs[(errnum - 1)]);
	}
	else
	{
	    dbd->dbd_Warnings++;
	    dbd_debugPrintf (ag, dbd, "%s, Line %ld, Warning %ld: %s\n",
			 (LONG) dbd->dbd_Name, (LONG) dbd->dbd_NumLines, (LONG) errnum, (LONG) warnMsgs[(errnum - 100)]);
	}
    }

    return (retval);
}

/*****************************************************************************/

static BOOL dbd_initDB (struct AGLib * ag, Class * cl, Object * o, struct opSet * msg)
{
    ULONG (*ASM he) (REG (a0) struct Hook * h, REG (a2) VOID * obj, REG (a1) VOID * msg);
    struct DatabaseData *dbd = INST_DATA (cl, o);	/* Object data */
    struct NodeData *nd = NULL;	/* Current Node */
    BOOL amigaguide = FALSE;
    struct ClientData *cd;
    struct HostHandle *hh;
    struct opFindHost ofh;	/* Used to find the Main node */
    struct DataType *dtn;
    BOOL success = FALSE;	/* Able to initialize? */
    struct TextAttr *ta;
    BOOL going = TRUE;		/* Keep processing file? */
    LONG cof, offset;
    ULONG tbufsize;		/* Size of temporary buffer */
    LONG lineofs;		/* Offset of last line */
    STRPTR tbuf;		/* Temporary buffer */
    STRPTR name;		/* Name of the database */
    LONG dblen;			/* Length of entire database */
    ULONG text;
    LONG ofs;			/* Current offset in file */
    LONG len;			/* Length of Read() */
    UBYTE ch;
    LONG i;			/* Counter */
    LONG j;

    ULONG errLevel = RETURN_WARN;
    ULONG errNumber;
    STRPTR errString;

    STRPTR title;
    Object *no;

    /*********************/
    /* Initialize Layout */
    /*********************/

    /* Initialize the default layout information */
    NewList ((struct List *) &dbd->dbd_MacroList);
    dbd->dbd_MinWidth = 15;
    dbd->dbd_DefTabs = 8;
    for (i = j = 0; i < MAX_TABS; i++, j += dbd->dbd_DefTabs)
	dbd->dbd_Tabs[i] = j;

    /* Initialize the default monospace font */
    ta = &dbd->dbd_TAttr[0];
    {
#undef	GfxBase
#include <graphics/gfxbase.h>
	struct GfxBase *gfxbase = (struct GfxBase *) ag->ag_GfxBase;

	ta->ta_Name = gfxbase->DefaultFont->tf_Message.mn_Node.ln_Name;
	ta->ta_YSize = gfxbase->DefaultFont->tf_YSize;
	ta->ta_Style = gfxbase->DefaultFont->tf_Style;
	ta->ta_Flags = gfxbase->DefaultFont->tf_Flags;
#define	GfxBase	ag->ag_GfxBase
	dbd->dbd_Fonts[0] = OpenFont (ta);
	dbd->dbd_Font = OpenFont (ta);
    }

    /***************************/
    /* Continue Initialization */
    /***************************/

    /* Initialize the semaphore */
    InitSemaphore (&dbd->dbd_Lock);

    /* Initialize the node list */
    NewList ((struct List *) &dbd->dbd_NodeList);
    NewList ((struct List *) &dbd->dbd_XRefList);

    /* See if they want debugging information */
    if (GetVar ("AmigaGuide/Debug", dbd->dbd_ErrorName, sizeof (dbd->dbd_ErrorName), NULL) > 0)
    {
	if (Stricmp (dbd->dbd_ErrorName, "OFF") != 0)
	    dbd->dbd_Flags |= DBDF_DEBUG;
    }

    /* Get the name of the database */
    errString = name = (STRPTR) GetTagData (DBA_Name, NULL, ((struct opSet *) msg)->ops_AttrList);
    hh = (struct HostHandle *) GetTagData (DBA_HostHandle, NULL, ((struct opSet *) msg)->ops_AttrList);
    cd = (struct ClientData *) GetTagData (DBA_ClientData, NULL, ((struct opSet *) msg)->ops_AttrList);

    /* We are coming from a file */
    if (hh == NULL)
    {
	/* Get a lock on the file */
	if (dbd->dbd_FileLock = LVOLockE (ag, NULL, name, ACCESS_READ))
	{
	    if (NameFromLock (dbd->dbd_FileLock, dbd->dbd_FullName, sizeof (dbd->dbd_FullName)))
		name = dbd->dbd_FullName;

	    /* Sniff the file */
	    if (dtn = ObtainDataTypeA (DTST_FILE, (APTR) dbd->dbd_FileLock, NULL))
	    {
		/* Is it an AmigaGuide document? */
		if (Stricmp (dtn->dtn_Header->dth_BaseName, "amigaguide") == 0)
		    amigaguide = TRUE;

		/* Release the datatype */
		ReleaseDataType (dtn);
	    }
	}
    }

    /* Set the attributes */
    dbd_setAttrs (ag, cl, o, msg);

    /* We are coming from a HyperHost */
    if (hh)
    {
	struct TagItem tags[2];

	/* Initialize the tag list */
	tags[0].ti_Tag = AGA_HelpGroup;
	tags[0].ti_Data = (cd) ? cd->cd_HelpGroup : 0L;
	tags[1].ti_Tag = TAG_DONE;

	/* Try creating the main node for this database */
	memset (&ofh, 0, sizeof (struct opFindHost));

	ofh.MethodID = HM_FINDNODE;
	ofh.ofh_Attrs = tags;
	ofh.ofh_Node = MAIN_NODE;
	ofh.ofh_Title = MAIN_NODE;

	/* Show that the database is an AmigaGuide database */
	dbd->dbd_Flags |= DBDF_NODEHOST;

	/* Remember the host handle */
	dbd->dbd_HH = hh;

	/* Get a pointer to the entry point */
	he = hh->hh_Dispatcher.h_Entry;

	/* Try creating the main node for the node host */
	if ((*he) (hh, name, &ofh))
	{
	    /* Create the node object. */
	    if (dbd->dbd_CurrentNode = NewObject (ag->ag_NodeClass, NULL,
						  HNA_Name, (ULONG) ofh.ofh_Node,
						  HNA_Title, (ULONG) ofh.ofh_Title,
						  HNA_TOC, (ULONG) ofh.ofh_TOC,
						  HNA_Next, (ULONG) ofh.ofh_Next,
						  HNA_Previous, (ULONG) ofh.ofh_Prev,
						  HNA_Database, o,
						  HNA_HostHandle, hh,
						  TAG_DONE))
	    {
		/* Indicate success */
		success = TRUE;
	    }
	}
    }
    else if (amigaguide)
    {
	/* Show that the database is an AmigaGuide database */
	dbd->dbd_Flags |= DBDF_AMIGAGUIDE;

	/* Open the file */
	if (dbd->dbd_FileHandle = LVOOpenE (ag, NULL, name, MODE_OLDFILE))
	{
	    if (Seek (dbd->dbd_FileHandle, 0L, OFFSET_END) >= 0L)
	    {
		if ((dblen = Seek (dbd->dbd_FileHandle, 0L, OFFSET_BEGINNING)) > 0L)
		{
		    tbufsize = MIN (10240, dblen);
		    if (tbuf = AllocVec (tbufsize + 1, MEMF_CLEAR))
		    {
			/* Initialize the working variables */
			dbd->dbd_NumLines = dbd->dbd_NumNodes = 0;
			success = TRUE;
			ofs = 0;

			/* Read from the file into our working buffer */
			while (going && ((len = Read (dbd->dbd_FileHandle, tbuf, tbufsize)) > 0L))
			{
			    for (i = j = lineofs = 0; going && (i < len); i++)
			    {
				if ((tbuf[i] == 10) || (tbuf[i] == 0))
				{
				    j++;
				    dbd->dbd_NumLines++;
				    dbd->dbd_Offset = ofs + lineofs;

				    ch = tbuf[i];
				    tbuf[i] = 0;

				    text = TRUE;
				    if (tbuf[lineofs] == '@')
				    {
					/* Command line */
					going = dbd_parseCommand (ag, o, dbd, (tbuf + lineofs), &text);
					nd = INST_DATA (ag->ag_NodeClass, dbd->dbd_CurrentNode);
				    }

				    if (text)
				    {
					/* Text line */
					if (nd)
					{
					    if (nd->nd_Offset == (-1))
						nd->nd_Offset = dbd->dbd_Offset;
					    nd->nd_TotVert++;
					}
				    }

				    lineofs = i + 1;
				    tbuf[i] = ch;
				}
			    }

			    /* Seek to the correct position within the file */
			    ofs += lineofs;
			    cof = Seek (dbd->dbd_FileHandle, 0, OFFSET_CURRENT);
			    if (offset = ofs - cof)
				Seek (dbd->dbd_FileHandle, offset, OFFSET_CURRENT);

			    /* See if the line was too long */
			    if (j == 0)
			    {
				dbd->dbd_Errors++;
				dbd_debugPrintf (ag, dbd, "%s, Line %ld, Error 4: %s\n",
					     (LONG) dbd->dbd_Name, (LONG) dbd->dbd_NumLines, (LONG) errMsgs[3]);
				success = going = FALSE;
			    }

			    /* If the length read is not equal to the buffer size, then we
			     * must be done reading */
			    if (len != tbufsize)
				going = FALSE;
			}

			if (len < 0)
			    success = FALSE;

			if (dbd->dbd_NumNodes == 0)
			{
			    dbd_debugPrintf (ag, dbd, "%s, Line %ld, Error 5: %s\n",
					     (LONG) dbd->dbd_Name, (LONG) dbd->dbd_NumLines, (LONG) errMsgs[4]);
			    SetIoErr (DTERROR_NOT_ENOUGH_DATA);
			    dbd->dbd_Errors++;
			    success = FALSE;
			}
			else if (dbd->dbd_NumEndNodes < dbd->dbd_NumNodes)
			{
			    dbd_debugPrintf (ag, dbd, "%s, Line %ld, Error 6: %s\n",
					     (LONG) dbd->dbd_Name, (LONG) dbd->dbd_NumLines, (LONG) errMsgs[5]);
			    SetIoErr (DTERROR_NOT_ENOUGH_DATA);
			    dbd->dbd_Errors++;
			    success = FALSE;
			}

			if (success)
			{
			    errLevel = RETURN_OK;
			}

			FreeVec (tbuf);
		    }
		}
	    }
	}
    }
    else if (going)
    {
	/* It wasn't an AmigaGuide database.  So create a database with a main node that is the new object. */

	/* Create the object. When we enter the object we will do AddDTObject() and RefreshDTObject().
	 * When we exit the object, we will do RemoveDTObject(). */
	if (no = newdtobject (ag, name,
			      DTA_SourceType, DTST_FILE,
			      GA_Immediate, TRUE,
			      GA_RelVerify, TRUE,
			      TAG_DONE))
	{
	    /* Get the object name */
	    GetAttr (DTA_ObjName, no, (ULONG *) & title);

	    /* Create the node object. */
	    if (dbd->dbd_CurrentNode = NewObject (ag->ag_NodeClass, NULL,
						  HNA_Name, (ULONG) "Main",
						  HNA_Title, (ULONG) ((title) ? title : name),
						  HNA_Database, o,
						  HNA_Object, no,
						  TAG_DONE))
	    {
		success = TRUE;
	    }
	    else
	    {
		DisposeDTObject (no);
	    }
	}
    }

    errNumber = IoErr ();

    if (success)
    {
	/* Execute the open database command */
	if (dbd->dbd_OnOpen)
	{
	    sprintf (dbd->dbd_ErrorLine, "%s %s", dbd->dbd_OnOpen, dbd->dbd_FullName);
	    if ((i = m_sendCmd (ag, dbd->dbd_ErrorLine)) > 0)
	    {
		errNumber = DTERROR_COULDNT_OPEN;
		errLevel = i;
		success = FALSE;
		SetIoErr (errNumber);
	    }
	}
    }

    if (success)
    {
	/* Increment the use count */
	dbd->dbd_UseCnt++;

	/* Add the database to the list */
	ObtainSemaphore (&ag->ag_Lock);
	DoMethod (o, OM_ADDTAIL, &ag->ag_DatabaseList);
	ReleaseSemaphore (&ag->ag_Lock);
    }
    else if (cd)
    {
	cd->cd_ErrorLevel = errLevel;

	switch (cd->cd_ErrorNumber = errNumber)
	{
	    case ERROR_INVALID_RESIDENT_LIBRARY:
		cd->cd_ErrorNumber = DTERROR_UNKNOWN_DATATYPE;
		break;

	    case ERROR_OBJECT_NOT_FOUND:
		cd->cd_ErrorNumber = DTERROR_COULDNT_OPEN;
		break;
	}
	cd->cd_ErrorString = (ULONG) errString;
    }

    /* Close the debugging window (doesn't really because of WAIT) */
    dbd_closeDebug (ag, dbd, 1);

    return (success);
}

/*****************************************************************************/

static Object *dbd_findDatabase (struct AGLib *ag, STRPTR name, struct HostHandle *hh)
{
    struct DatabaseData *dbd;
    Object *retval = NULL;
    BPTR lock = NULL;
    Object *member;
    Object *ostate;
    BOOL comp;

    comp = TRUE;
    if (hh == NULL)
    {
	if ((lock = LVOLockE (ag, NULL, name, ACCESS_READ)) == NULL)
	{
	    if (IoErr () != ERROR_OBJECT_NOT_FOUND)
		return NULL;
	    comp = FALSE;
	}
    }

    SetIoErr (ERROR_OBJECT_NOT_FOUND);

    if (comp)
    {
	/* Lock the database list */
	ObtainSemaphore (&ag->ag_Lock);

	/* Search through the database list for a matching database */
	ostate = (Object *) ag->ag_DatabaseList.mlh_Head;
	while (member = NextObject (&ostate))
	{
	    /* Convert the member to a database object */
	    dbd = INST_DATA (ag->ag_DatabaseClass, member);

	    /* See if it matches */
	    if ((Stricmp (dbd->dbd_Name, name) == 0) && (SameLock (lock, dbd->dbd_FileLock) == LOCK_SAME))
	    {
		retval = member;
		SetIoErr (0);
		break;
	    }
	}

	/* Release the database lock */
	ReleaseSemaphore (&ag->ag_Lock);
    }

    /* Release the temporary lock */
    UnLock (lock);

    return (retval);
}

/*****************************************************************************/

static ULONG ASM dbd_dispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct AGLib *ag = (struct AGLib *) cl->cl_UserData;
    struct DatabaseData *dbd = INST_DATA (cl, o);
    struct HostHandle *hh;
    ULONG retval = 0L;
    Object *newobj;
    Object *member;
    STRPTR name;
    ULONG err;

    switch (msg->MethodID)
    {
	    /* Create a new database object */
	case OM_NEW:
	    /* Find the name */
	    name = (STRPTR) GetTagData (DBA_Name, NULL, ((struct opSet *) msg)->ops_AttrList);
	    hh = (struct HostHandle *) GetTagData (DBA_HostHandle, NULL, ((struct opSet *) msg)->ops_AttrList);

	    /* See if the database already exists */
	    if (newobj = dbd_findDatabase (ag, name, hh))
	    {
		/* Get a pointer to the instance data */
		dbd = INST_DATA (cl, newobj);

		/* Bump the use count */
		ObtainSemaphore (&dbd->dbd_Lock);
		dbd->dbd_UseCnt++;
		ReleaseSemaphore (&dbd->dbd_Lock);
	    }
	    /* It didn't exist, so try creating it */
	    else if (IoErr () == ERROR_OBJECT_NOT_FOUND)
	    {
		if (newobj = (Object *) DoSuperMethodA (cl, o, msg))
		{
		    if (!(dbd_initDB (ag, cl, newobj, (struct opSet *) msg)))
		    {
			CoerceMethod (cl, newobj, OM_DISPOSE);
			newobj = NULL;
		    }
		}
	    }
	    else
	    {
		CoerceMethod (cl, newobj, OM_DISPOSE);
		newobj = NULL;
	    }

	    retval = (ULONG) newobj;
	    break;

	    /* Add a node object to the database's node list */
	case OM_ADDMEMBER:
	    member = ((struct opAddMember *) msg)->opam_Object;
	    DoMethod (member, OM_ADDTAIL, &dbd->dbd_NodeList);
	    break;

	    /* Remove a node object from the database's node list */
	case OM_REMMEMBER:
	    member = ((struct opAddMember *) msg)->opam_Object;
	    retval = (ULONG) DoMethod (member, OM_REMOVE);
	    break;

	    /* Set database attributes */
	case OM_SET:
	    retval = dbd_setAttrs (ag, cl, o, (struct opSet *) msg);
	    break;

	case OM_DISPOSE:
	    err = IoErr();

	    /* Any users? */
	    if (dbd->dbd_UseCnt == 0)
	    {
		/* Lock the database */
		ObtainSemaphore (&dbd->dbd_Lock);

		retval = dbd_disposeDatabase (ag, cl, o, msg, dbd);
	    }
	    else
	    {
		/* Lock the database */
		ObtainSemaphore (&dbd->dbd_Lock);

		/* Decrement the use count */
		dbd->dbd_UseCnt--;

		/* Any users? */
		if (dbd->dbd_UseCnt == 0)
		{
		    /* Remove the object from the database list */
		    ObtainSemaphore (&ag->ag_Lock);
		    DoMethod (o, OM_REMOVE);
		    ReleaseSemaphore (&ag->ag_Lock);

		    retval = dbd_disposeDatabase (ag, cl, o, msg, dbd);
		}
		else
		{
		    /* Release the lock */
		    ReleaseSemaphore (&dbd->dbd_Lock);
		}
	    }
	    SetIoErr (err);
	    break;

#if 0
	    /* Get database attributes */
	case OM_GET:
#endif
	    /* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/

Class *initDBClass (struct AGLib * ag)
{
    Class *cl;

    if (cl = MakeClass (NULL, ROOTCLASS, NULL, sizeof (struct DatabaseData), 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)()) dbd_dispatch;
	cl->cl_UserData = (ULONG) ag;
    }

    return (cl);
}
