/* main.c
 */

#include "wdisplay.h"
#include "wdisplay_rev.h"

char *version = VERSTAG;

struct Library *DOSBase, *IntuitionBase, *GfxBase, *IconBase;
struct Library *KeymapBase, *WorkbenchBase, *IFFParseBase;
struct Library *LocaleBase;
struct ConsoleDevice *ConsoleDevice;
struct IOStdReq IOReq;

struct TextAttr Topaz80 = {"topaz.font", 8, NULL, NULL};

LONG main (int argc, char **argv)
{
    struct WBStartup *wb = (struct WBStartup *) argv;
    struct DiskObject *dob = NULL;
    ULONG sigr, sigi, siga;
    struct AppInfo *ai;
    struct WBArg *wa;
    BPTR old = NULL;

    /* Allocate the work memory */
    if (ai = (struct AppInfo *) AllocMem (ASIZE, MEMF_CLEAR))
    {
	/* Open the required libraries */
	if (OpenLibraries (ai))
	{
	    /* Provide defaults */
	    ai->ai_Options[OPT_LEFT] = ai->ai_Options[OPT_TOP] = -1;
	    ai->ai_Options[OPT_WIDTH] = ai->ai_Options[OPT_HEIGHT] = -1;

	    /* Show that we're not done running the application yet */
	    ai->ai_Done = FALSE;

	    /* Started from Workbench? */
	    if (argc == 0)
	    {
		/* Show that we where started from Workbench */
		ai->ai_Flags |= AIF_WORKBENCH;

		/* Make sure we have arguments */
		if (wb->sm_NumArgs > 1)
		{
		    /* skip the Tool icon */
		    wa = wb->sm_ArgList;
		    wa++;

		    /* Set the current directory */
		    old = CurrentDir (wa->wa_Lock);

		    /* Get the file name from the icon */
		    if (ai->ai_Options[OPT_FILE] = (LONG) wa->wa_Name)
		    {
			if (dob = GetDiskObject (wa->wa_Name))
			{
			    readargs (ai, 0L, dob->do_ToolTypes);
			}
		    }
		}
	    }
	    /* Started from Shell */
	    else
	    {
		readargs (ai, argc, argv);
	    }

	    /* Make sure we are able to get a picture to display */
	    if (!ai->ai_Done &&
		(WorkbenchBase == NULL) &&
		(ai->ai_Options[OPT_FILE] == NULL))
	    {
		NotifyUser (ai, ERR_REQUIRES_FILE_NAME);
		ai->ai_Done = TRUE;
	    }

	    /* Open the environment */
	    if (!ai->ai_Done && OpenEnvironment (ai))
	    {
		/* Open the window */
		if (OpenDocWindow (ai))
		{
		    /* Did they provide a file name? */
		    if (ai->ai_Options[OPT_FILE])
		    {
			/* Load the picture */
			LoadPicture (ai, NULL, (STRPTR) ai->ai_Options[OPT_FILE]);
		    }

		    /* Get our signal bits */
		    sigi = (1L << ai->ai_IDCMP->mp_SigBit);
		    siga = ai->ai_AWMPort ? (1L << ai->ai_AWMPort->mp_SigBit) : 0;

		    /* Continue until done */
		    while (!(ai->ai_Done))
		    {
			/* Wait for something to happen */
			sigr = Wait (sigi | siga | SIGBREAKF_CTRL_C);

			if (sigr & SIGBREAKF_CTRL_C)
			{
			    ai->ai_Done = TRUE;
			}

			/* Process Intuition messages */
			if (sigr & sigi)
			{
			    HandleIDCMP (ai);
			}

			/* Process AppWindow messages */
			if (sigr & siga)
			{
			    HandleAppMessage (ai);
			}
		    }

		    /* Close the information window */
		    CloseAboutWindow (ai);

		    /* Close the document window */
		    CloseDocWindow (ai);
		}
	    }

	    /* Close the work space */
	    CloseEnvironment (ai);

	    if (dob)
	    {
		FreeDiskObject (dob);
	    }

	    /* Restore the original startup directory */
	    if (old)
	    {
		CurrentDir (old);
	    }
	}

	/* Close the libraries */
	CloseLibraries (ai);

	/* Free the work buffer */
	FreeMem (ai, ASIZE);
    }
}

/* Try opening the library from a number of common places */
struct Library *openlibrary (STRPTR name, ULONG version)
{
    struct Library *retval;
    STRPTR buffer;

    if (!(retval = OpenLibrary (name, version)))
    {
	if (buffer = (STRPTR) AllocMem (128, MEMF_CLEAR))
	{
	    sprintf (buffer, ":libs/%s", name);

	    if (!(retval = OpenLibrary (buffer, version)))
	    {
		sprintf (buffer, "libs/%s", name);

		retval = OpenLibrary (buffer, version);
	    }

	    FreeMem (buffer, 128);
	}
    }

    return retval;
}

BOOL OpenLibraries (struct AppInfo * ai)
{

    if (DOSBase = OpenLibrary ("dos.library", 0))
    {
	/* Open Intuition library */
	if (IntuitionBase = OpenLibrary ("intuition.library", 0))
	{
	    if (GfxBase = OpenLibrary ("graphics.library", 0))
	    {
		if (IconBase = openlibrary ("icon.library", 0))
		{
		    if (IFFParseBase = openlibrary ("iffparse.library", 0))
		    {
			/* These libraries are optional */
			WorkbenchBase = openlibrary ("workbench.library", 0);
			if (LocaleBase = openlibrary ("locale.library", 0))
			{
			    ai->ai_Catalog = OpenCatalogA (NULL, CATALOGNAME, NULL);
			}

			/* Need this for converting raw keys to vanilla */
			if (!(OpenDevice ("console.device", -1L,
					  (struct IORequest *) & IOReq, 0L)))
			{
			    ConsoleDevice = (struct ConsoleDevice *) IOReq.io_Device;
			}
			else
			{
			    return (FALSE);
			}

			return (TRUE);
		    }
		}
	    }
	}
    }

    return (FALSE);
}

VOID CloseLibraries (struct AppInfo * ai)
{

    /* shutdown the console device */
    if (ConsoleDevice)
    {
	CloseDevice ((struct IORequest *) & IOReq);
	ConsoleDevice = NULL;
    }

    if (LocaleBase)
    {
	CloseCatalog (ai->ai_Catalog);
	CloseLibrary (LocaleBase);
    }

    if (IFFParseBase)
	CloseLibrary (IFFParseBase);
    if (IntuitionBase)
	CloseLibrary (IntuitionBase);
    if (GfxBase)
	CloseLibrary (GfxBase);
    if (KeymapBase)
	CloseLibrary (KeymapBase);
    if (WorkbenchBase)
	CloseLibrary (WorkbenchBase);
    if (IconBase)
	CloseLibrary (IconBase);
    if (DOSBase)
	CloseLibrary (DOSBase);
}
