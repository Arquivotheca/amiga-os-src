/* AddDataTypes.c
 *
 */

#include "adddatatypes.h"
#include "adddatatypes_rev.h"

/*****************************************************************************/

#define	TEMPLATE   "FILES/M,QUIET/S,REFRESH/S" VERSTAG
#define	OPT_FILES	0
#define	OPT_QUIET	1
#define	OPT_REFRESH	2
#define	OPT_COUNT	3

/*****************************************************************************/

#define	DB(x)		;
VOID kprintf (STRPTR,...);

/*****************************************************************************/

LONG cmd_start (void)
{
    struct WBStartup *WBenchMsg = NULL;
    LONG failureLevel = RETURN_FAIL;
    struct GlobalData global;
    struct RDArgs *rdargs;
    struct GlobalData *gd;
    LONG opts[OPT_COUNT];
    struct LocaleInfo li;
    struct WBArg *wbarg;
    BPTR oldLock;
    STRPTR *ptr;
    LONG wbcnt;

    BOOL scan = TRUE;

    gd = &global;

    memset (gd, 0, sizeof (struct GlobalData));

    SysBase = (*((struct Library **) 4));

    gd->gd_Process = (struct Process *) FindTask (NULL);
    if (!(gd->gd_Process->pr_CLI))
    {
	WaitPort (&gd->gd_Process->pr_MsgPort);
	WBenchMsg = (struct WBStartup *) GetMsg (&gd->gd_Process->pr_MsgPort);
    }

    if (SysBase->lib_Version >= 39)
    {
	DOSBase = OpenLibrary ("dos.library", 39);
	UtilityBase = OpenLibrary ("utility.library", 39);
	IntuitionBase = OpenLibrary ("intuition.library", 39);

	if (IFFParseBase = OpenLibrary ("iffparse.library", 37))
	{
	    if (LocaleBase = OpenLibrary ("locale.library", 38))
		li.li_Catalog = OpenCatalogA (NULL, "sys/c.catalog", NULL);

	    /* Get a pointer to the token that contains the datatypes list */
	    if (gd->gd_Token = ObtainDataTypesToken (gd))
	    {
		ParsePatternNoCase ("#?.(INFO|BACKDROP)", gd->gd_InfoPattern, sizeof (gd->gd_InfoPattern));

		/* Lock the list for exclusive access */
		ObtainSemaphore (&gd->gd_Token->t_Lock);

		if (WBenchMsg)
		{
		    failureLevel = RETURN_OK;
		    wbarg = WBenchMsg->sm_ArgList;
		    wbcnt = WBenchMsg->sm_NumArgs;

		    while (--wbcnt > 0)
		    {
			wbarg++;
			oldLock = CurrentDir (wbarg->wa_Lock);
			ReadDataTypeList (gd, wbarg->wa_Name);
			CurrentDir (oldLock);
		    }
		}
		else
		{
		    memset (opts, 0, sizeof (opts));
		    if (rdargs = ReadArgs (TEMPLATE, opts, NULL))
		    {
			if (opts[OPT_QUIET])
			    gd->gd_Flags |= GDF_QUIET;

			failureLevel = RETURN_OK;

			if (opts[OPT_REFRESH])
			{
			    if (GetDateStamp (gd))
			    {
				ScanDir (gd, DEFAULT_WILD, opts);
			    }
			}
			else if (ptr = (STRPTR *) opts[OPT_FILES])
			{
			    while ((*ptr) && (failureLevel == RETURN_OK))
			    {
				ScanDir (gd, *ptr, opts);

				ptr++;
			    }
			}

			FreeArgs (rdargs);
		    }
		    else
		    {
			if (!opts[OPT_QUIET])
			    PrintFault (IoErr (), NULL);
		    }
		}

		/* Release the lock */
		ReleaseSemaphore (&gd->gd_Token->t_Lock);
	    }

	    if (LocaleBase)
	    {
		CloseCatalog (li.li_Catalog);
		CloseLibrary (LocaleBase);
	    }

	    CloseLibrary (IFFParseBase);
	}

	CloseLibrary (IntuitionBase);
	CloseLibrary (UtilityBase);
	CloseLibrary (DOSBase);
    }

    if (WBenchMsg)
    {
	Forbid ();
	ReplyMsg (WBenchMsg);
    }

    return (failureLevel);
}

/*****************************************************************************/

BOOL GetDateStamp (struct GlobalData * gd)
{
    struct FileInfoBlock *fib;
    BOOL scan = TRUE;
    BPTR lock;

    /* Get the date stamp */
    if (lock = Lock (DEFAULT_DIR, ACCESS_READ))
    {
	if (fib = AllocDosObject (DOS_FIB, NULL))
	{
	    if (Examine (lock, fib))
	    {
		if (CompareDates (&fib->fib_Date, &gd->gd_Token->t_Date) == 0)
		{
		    DB (kprintf ("DateStamp the same\n"));
		    scan = FALSE;
		}
		else
		{
		    gd->gd_Token->t_Date = *(&fib->fib_Date);
		}
	    }

	    FreeDosObject (DOS_FIB, (APTR) fib);
	}

	UnLock (lock);
    }
    return (scan);
}

/*****************************************************************************/

ULONG ScanDir (struct GlobalData * gd, STRPTR path, LONG * opts)
{
    struct AnchorPath __aligned anchor;
    ULONG failureLevel = RETURN_OK;
    BPTR oldLock;
    LONG result;
    BOOL bool;

    memset (&anchor, 0, sizeof (struct AnchorPath));
    anchor.ap_Flags = APF_DOWILD;
    anchor.ap_BreakBits = SIGBREAKF_CTRL_C;

    if (!(result = MatchFirst (path, &anchor)))
    {
	while (TRUE)
	{
	    if (CheckSignal (SIGBREAKF_CTRL_C))
	    {
		failureLevel = RETURN_WARN;
		if (!opts[OPT_QUIET])
		    PrintFault (ERROR_BREAK, NULL);
		break;
	    }

	    if (anchor.ap_Info.fib_DirEntryType < 0)
	    {
		bool = TRUE;
		oldLock = CurrentDir (anchor.ap_Current->an_Lock);
		if (!MatchPatternNoCase (gd->gd_InfoPattern, anchor.ap_Info.fib_FileName))
		{
		    ReadDataTypeList (gd, anchor.ap_Info.fib_FileName);
		}
		CurrentDir (oldLock);

		if (!bool)
		{
		    failureLevel = RETURN_FAIL;
		    break;
		}
	    }

	    if (result = MatchNext (&anchor))
	    {
		if (result != ERROR_NO_MORE_ENTRIES)
		{
		    failureLevel = RETURN_FAIL;
		    if (!opts[OPT_QUIET])
			PrintFault (result, NULL);
		}
		break;
	    }
	}
	MatchEnd (&anchor);
    }
    return failureLevel;
}

/*****************************************************************************/

VOID PrintF (struct GlobalData * gd, AppStringsID str, STRPTR arg1,...)
{
    struct EasyStruct est;

    if (gd->gd_Flags & GDF_QUIET)
	return;

    if (gd->gd_Process->pr_CLI)
    {
	VPrintf (GetString (&gd->gd_LocaleInfo, str), (LONG *) & arg1);
    }
    else
    {
	est.es_StructSize = sizeof (struct EasyStruct);
	est.es_Flags = 0;
	est.es_Title = GetString (&gd->gd_LocaleInfo, MSG_MT_REQ_TITLE);
	est.es_TextFormat = GetString (&gd->gd_LocaleInfo, str);
	est.es_GadgetFormat = GetString (&gd->gd_LocaleInfo, MSG_MT_OK_GAD);

	EasyRequestArgs (NULL, &est, NULL, &arg1);
    }
}

/*****************************************************************************/

static struct NamedObject *ano (struct GlobalData * gd, STRPTR name, Tag tag1,...)
{

    return (AllocNamedObjectA (name, (struct TagItem *) & tag1));
}

/*****************************************************************************/

struct Token *ObtainDataTypesToken (struct GlobalData * gd)
{
    struct NamedObject *no;
    struct Token *t = NULL;

    /* See if the token already exists */
    if (no = FindNamedObject (NULL, TOKEN_NAME, NULL))
    {
	t = (struct Token *) no->no_Object;
	ReleaseNamedObject (no);
    }
    /* No token, so create one */
    else if (no = ano (gd, TOKEN_NAME,
		       ANO_NameSpace, TRUE,
		       ANO_UserSpace, TKSIZE,
		       ANO_Flags, NSF_NODUPS | NSF_CASE,
		       TAG_DONE))
    {
	if (no->no_Object)
	{
	    t = (struct Token *) no->no_Object;

	    /* Initialize the token */
	    InitSemaphore (&t->t_Lock);
	    NewList (&t->t_AlphaList);
	    InitList (gd, "binary", &t->t_AlphaList, &t->t_BinaryList, DTF_BINARY, GID_SYSTEM, MAKE_ID ('b', 'i', 'n', 'a'));
	    InitList (gd, "ascii", &t->t_AlphaList, &t->t_ASCIIList, DTF_ASCII, GID_TEXT, MAKE_ID ('a', 's', 'c', 'i'));
	    InitList (gd, "iff", &t->t_AlphaList, &t->t_IFFList, DTF_IFF, GID_SYSTEM, MAKE_ID ('i', 'f', 'f', 0));
	    InitList (gd, "directory", &t->t_AlphaList, &t->t_AuxList, DTF_MISC, GID_SYSTEM, MAKE_ID ('d', 'i', 'r', 'e'));

	    /* Make the name public */
	    if (!AddNamedObject (NULL, no))
	    {
		FreeNamedObject (no);
		t = NULL;
	    }

	    /* Now attempt to get a handle on the public token */
	    if (no = FindNamedObject (NULL, TOKEN_NAME, NULL))
	    {
		t = (struct Token *) no->no_Object;
		ReleaseNamedObject (no);
	    }
	}
	else
	{
	    ReleaseNamedObject (no);
	}
    }
    return (t);
}
