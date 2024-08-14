/* hypernozy.c
 * Written by David N. Junod
 *
 */

#include "hyperbrowser.h"

/*****************************************************************************/

int main (void)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    LONG failureLevel = RETURN_FAIL;
    struct WBStartup *wbMsg = NULL;
    struct GlobalData *gd = NULL;
    LONG failureCode = 0;
    struct Process *pr;
    UBYTE name[64];
    BPTR fh;

    pr = (struct Process *) SysBase->ThisTask;
    if (!(pr->pr_CLI))
    {
	WaitPort (&pr->pr_MsgPort);
	wbMsg = (struct WBStartup *) GetMsg (&pr->pr_MsgPort);
    }

    /* Make sure we are running with V39 or greater */
    if (SysBase->LibNode.lib_Version < 39)
    {
	/* DOS isn't open!!!! */
	vprintf (gd, "requires V39\n", NULL);
    }
    /* Allocate our global data */
    else if (gd = AllocVec (sizeof (struct GlobalData), MEMF_CLEAR))
    {
	/* Open the ROM libraries */
	DOSBase       = OpenLibrary ("dos.library", 39);
	IntuitionBase = OpenLibrary ("intuition.library", 39);
	GfxBase       = OpenLibrary ("graphics.library", 39);
	UtilityBase   = OpenLibrary ("utility.library", 39);

	/* Our process */
	gd->gd_Process = pr = (struct Process *) FindTask (NULL);

	/* Initialize the node host hook */
	gd->gd_NHHook.h_Entry = nodehost;
	gd->gd_NHHook.h_Data = gd;

	/* Open AmigaGuide */
	if (AmigaGuideBase = OpenLibrary ("amigaguide.library", 33))
	{
	    /* Build the unique name */
	    gd->gd_NHID = GetUniqueID ();
	    sprintf (gd->gd_NHName, "%s.%ld", BASENAME, gd->gd_NHID);

	    /* Create the temporary guide file */
	    sprintf (name, "T:%s.guide", gd->gd_NHName);
	    if (fh = Open (name, MODE_NEWFILE))
	    {
		/* Create the database */
		vfprintf (gd, fh, "@database %s.guide\n\n", (void *) gd->gd_NHName);
		vfprintf (gd, fh, "@node Main HyperBrowser\n", NULL);
		vfprintf (gd, fh, "\nExec Lists\n\n", NULL);
		vfprintf (gd, fh, "  @{\"Resource\" link HYPERNOZY.RESOURCELIST}\n", NULL);
		vfprintf (gd, fh, "  @{\"Device\" link HYPERNOZY.DEVICELIST}\n", NULL);
		vfprintf (gd, fh, "  @{\"Interrupt\" link HYPERNOZY.INTRLIST}\n", NULL);
		vfprintf (gd, fh, "  @{\"Library\" link HYPERNOZY.LIBRARYLIST}\n", NULL);
		vfprintf (gd, fh, "  @{\"Port\" link HYPERNOZY.PORTLIST}\n", NULL);
		vfprintf (gd, fh, "  @{\"TaskReady\" link HYPERNOZY.TASKREADYLIST}\n", NULL);
		vfprintf (gd, fh, "  @{\"TaskWait\" link HYPERNOZY.TASKWAITLIST}\n\n", NULL);
		vfprintf (gd, fh, "Intuition Lists\n\n", NULL);
		vfprintf (gd, fh, "  @{\"Class\" link HYPERNOZY.CLASSLIST}\n", NULL);
		vfprintf (gd, fh, "  @{\"Screen\" link HYPERNOZY.SCREENLIST}\n", NULL);
		vfprintf (gd, fh, "@endnode\n", NULL);

		/* Close it */
		Close (fh);

		/* Start the Dynamic Node Host */
		gd->gd_NH = AddAmigaGuideHostA (&gd->gd_NHHook, gd->gd_NHName, NULL);

		/* Lock the default public screen */
		if (gd->gd_Screen = LockPubScreen (NULL))
		{
		    /* We don't want the window to automatically activate */
		    gd->gd_NAG.nag_Flags = HTF_NOACTIVATE;

		    /* Set the document name */
		    gd->gd_NAG.nag_Name = name;

		    /* Set the screen */
		    gd->gd_NAG.nag_Screen = gd->gd_Screen;

		    /* Set the application base name */
		    gd->gd_NAG.nag_ClientPort = gd->gd_NAG.nag_BaseName = BASENAME;

		    /* Open the help system */
		    if (gd->gd_AmigaGuide = openamigaguide (gd, &gd->gd_NAG,
							    AGA_HelpGroup, gd->gd_NHID,
							    TAG_DONE))
		    {
			/* Show that we are OK */
			failureLevel = RETURN_OK;

			/* Close the help system */
			CloseAmigaGuide (gd->gd_AmigaGuide);
		    }
		    else
		    {
			DisplayError (gd, pr->pr_Result2);
		    }

		    /* Unlock the default public screen */
		    UnlockPubScreen (NULL, gd->gd_Screen);
		}

		/* Remove the dynamic node host */
		while (RemoveAmigaGuideHostA (gd->gd_NH, NULL) > 0)
		    Delay (10);

		/* Delete the temporary file */
		DeleteFile (name);
	    }

	    /* Close AmigaGuide */
	    CloseLibrary (AmigaGuideBase);
	}

	/* Close the ROM libraries */
	CloseLibrary (UtilityBase);
	CloseLibrary ((struct Library *) GfxBase);
	CloseLibrary ((struct Library *) IntuitionBase);
	CloseLibrary (DOSBase);

	FreeVec (gd);
    }

    if (wbMsg)
    {
	Forbid ();
	ReplyMsg ((struct Message *) wbMsg);
    }
    pr->pr_Result2 = failureCode;
    return (failureLevel);
}

/*****************************************************************************/

APTR openamigaguide (struct GlobalData * gd, struct NewAmigaGuide * nag, Tag tag1,...)
{
    if (AmigaGuideBase->lib_Version > 33)
	return OpenAmigaGuideA (nag, (struct TagItem *) & tag1);
    else
	return OpenAmigaGuideA (nag, NULL);
}

/*****************************************************************************/

void bprintf (struct GlobalData *gd, STRPTR fmt, void *arg1, ...)
{
    asprintf (gd->gd_Buffer, fmt, &arg1);
    strcat (gd->gd_Node, gd->gd_Buffer);
}

/*****************************************************************************/

void vprintf (struct GlobalData * gd, STRPTR fmt, void *arg1,...)
{
    VPrintf (fmt, (LONG *) & arg1);
}

/*****************************************************************************/

void vfprintf (struct GlobalData * gd, BPTR fh, STRPTR fmt, void *arg1,...)
{
    VFPrintf (fh, fmt, (LONG *) & arg1);
}

/*****************************************************************************/

VOID DisplayError (struct GlobalData * gd, LONG err)
{
    vprintf (gd, "%s\n", (void *) GetAmigaGuideString (err));
}
