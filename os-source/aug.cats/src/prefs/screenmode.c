#include "prefsbase.h"

#define	ID_PREF	MAKE_ID('P','R','E','F')
#define	ID_PRHD	MAKE_ID('P','R','H','D')
#define	ID_SCRM MAKE_ID('S','C','R','M')

struct ScreenModePref * 
getscreenmodepref (BPTR drawer, STRPTR name, struct TagItem * attrs)
{
    struct Process *proc = (struct Process *) FindTask (NULL);
    LONG msize = sizeof (struct ScreenModePref);
    struct ScreenModePref *smp;
    BPTR lock = NULL;

    if (lock = CurrentDir (drawer))
    {
	/* Allocate the preference structure */
	if (smp = (struct ScreenModePref *) AllocVec (msize, MEMF_CLEAR))
	{
	    struct IFFHandle *iff;

	    /* Allocate an IFF handle */
	    if (iff = AllocIFF ())
	    {
		/* Open the preference file */
		if (iff->iff_Stream = Open (name, MODE_OLDFILE))
		{
		    /* Indicate that the IFF handle is for a file */
		    InitIFFasDOS (iff);

		    /* Open the IFF handle for reading */
		    if (!(OpenIFF (iff, IFFF_READ)))
		    {
			/* Register where we want to stop */
			StopChunk (iff, ID_PREF, ID_SCRM);

			/* Parse the file, stopping at the screen mode field */
			if (ParseIFF (iff, IFFPARSE_SCAN) == 0L)
			{
			    LONG tsize = msize - sizeof (struct Prefs);

			    /* Read in the preference structure */
			    ReadChunkBytes (iff, &(smp->smp_Reserved), tsize);
			}
			else
			{
			    /* Wrong kind of preference file */
			    proc->pr_Result2 = ERROR_OBJECT_WRONG_TYPE;
			}

			/* Close the IFF handle */
			CloseIFF (iff);
		    }

		    /* Close the file */
		    Close (iff->iff_Stream);
		}

		/* Free the IFF handle */
		FreeIFF (iff);
	    }
	    else
	    {
		proc->pr_Result2 = ERROR_NO_FREE_STORE;
	    }

	    /* See if we were able to read the pref file */
	    if (proc->pr_Result2)
	    {
		/* An error occurred, free the structure */
		FreeVec ((APTR) smp);
		smp = NULL;
	    }

	}
	else
	{
	    /* Not enough memory */
	    proc->pr_Result2 = ERROR_NO_FREE_STORE;
	}

	CurrentDir (lock);
    }

    return (smp);
}

struct ScreenModePref * GetScreenModePref (
	BPTR drawer,
	STRPTR name,
	struct ScreenModePref *osmp,
	struct TagItem * attrs)
{
    struct LibBase *lb = ((struct ExtLibrary *) PrefsBase)->el_Base;
    struct Process *proc = (struct Process *) FindTask (NULL);
    LONG msize = sizeof (struct ScreenModePref);
    struct ScreenModePref *smp;

    /* First try getting it from the passed directory */
    if (!(smp = getscreenmodepref (drawer, name, attrs)))
    {
	/* If the file wasn't found, then try in the system drawer */
	if (!(CompairLocks(drawer,lb->lb_SysPrefs)))
	{
	    /* Try getting the file from the system drawer */
	    if (smp = getscreenmodepref (lb->lb_SysPrefs, name, attrs))
	    {
		struct Prefs *p = &(smp->smp_Header);

		/* Tell where we came from */
		p->p_Flags |= PREFS_SYSTEM_F;
	    }
	}
    }

    /* See if we don't have a preference record due to no file */
    if (proc->pr_Result2 == ERROR_OBJECT_NOT_FOUND)
    {
	/* Allocate a default preference file */
	if (smp = (struct ScreenModePref *) AllocVec (msize, MEMF_CLEAR))
	{
	    struct Prefs *p = &(smp->smp_Header);

	    /* Tell where we came from */
	    p->p_Flags |= PREFS_DEFAULT_F;

	    /* Fill in with my idea of a good time */
	    smp->smp_ModeID = 0x8000;
	    smp->smp_Width = 65535;
	    smp->smp_Height = 65535;
	    smp->smp_Depth = 2;
	    smp->smp_AutoScroll = TRUE;
	}
    }

    /* See if we're freshening */
    if (osmp && smp)
    {
	VOID FreePref (VOID *);
	struct Prefs *op = &(osmp->smp_Header);

	/* Clear the where from flags */
	op->p_Flags &= ~PREFS_WHERE_F;

	/* See if we need to shutdown */
	if ((osmp->smp_ModeID != smp->smp_ModeID) ||
	    (osmp->smp_Width != smp->smp_Width) ||
	    (osmp->smp_Height != smp->smp_Height) ||
	    (osmp->smp_Depth != smp->smp_Depth) ||
	    (osmp->smp_AutoScroll != smp->smp_AutoScroll))
	{
	    /* Found a reason to shutdown */
	    op->p_Flags |= PREFS_CLOSEALL_F;
	}

	/* Copy the values */
	osmp->smp_ModeID = smp->smp_ModeID;
	osmp->smp_Width = smp->smp_Width;
	osmp->smp_Height = smp->smp_Height;
	osmp->smp_Depth = smp->smp_Depth;
	osmp->smp_AutoScroll = smp->smp_AutoScroll;

	/* Free the new preference file */
	FreePref (smp);

	/* Return the same old preference file with new information */
	smp = osmp;
    }

    return (smp);
}
