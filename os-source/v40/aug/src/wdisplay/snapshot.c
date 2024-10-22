/* snapshot.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * Routines to save/load a snapshot window position.  Uses the GetPrefsDrawer
 * function that shown in the "Application Preferences" article.
 * written by David N. Junod
 */

#include "wdisplay.h"

/* The extension for window snapshots */
#define	WINEXT "win"

/* Simple scaling of coordinates */
#define SCALE(a,b,c) (((b) * (c)) / (a))

#ifndef MEMORY_FOLLOWING
/* Usually allocate one large block of memory for a group of items and then
 * divy out to the appropriate pointers.  Use with caution---must be
 * consistent with field types! */
#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#endif

#define	SWORD		sizeof(WORD)
#define	SSCRBOX		(SWORD * 2)
#define	SWINBOX		(SWORD * 4)
#define	SINFOBOX	(SWORD * 2)

struct NewWindow *LoadSnapShot (struct AppInfo * ai, struct NewWindow * nw, BPTR prefs_drawer, STRPTR name)
{
    STRPTR config_file;
    LONG msize;
    BPTR lock;
    BPTR fh;

    /* compute buffer size */
    msize = strlen (name) + strlen (WINEXT) + 2L;

    /* allocate temporary work area */
    if (config_file = (STRPTR) AllocVec (msize, (MEMF_CLEAR | MEMF_PUBLIC)))
    {
	/* go into the preference directory */
	lock = CurrentDir (prefs_drawer);

	/* build the preference file name */
	sprintf (config_file, "%s.%s", name, WINEXT);

	/* load user preferences */
	if (fh = Open (config_file, MODE_OLDFILE))
	{
	    struct Screen *scr = nw->Screen;
	    SHORT Width, Height;

	    /* read in the screen width */
	    Read (fh, (UBYTE *) & Width, SWORD);

	    /* read in the screen height */
	    Read (fh, (UBYTE *) & Height, SWORD);

	    /* read in the window rectangle */
	    Read (fh, (UBYTE *) & nw->LeftEdge, SWINBOX);

	    /* read in the window rectangle */
	    Read (fh, (UBYTE *) & ai->ai_InfoLeft, SINFOBOX);

	    /* close the file */
	    Close (fh);

	    /* see if the screen size has changed */
	    if ((Width != scr->Width) || (Height != scr->Height))
	    {
		/* calculate new upper-left coordinates */
		nw->LeftEdge = (WORD) SCALE (Width, scr->Width, nw->LeftEdge);
		nw->TopEdge = (WORD) SCALE (Height, scr->Height, nw->TopEdge);
		nw->Width = (WORD) SCALE (Width, scr->Width, nw->Width);
		nw->Height = (WORD) SCALE (Height, scr->Height, nw->Height);
	    }

	    /* make sure we fit on the screen */
	    if ((nw->LeftEdge + nw->Width) > scr->Width)
		nw->LeftEdge -= (nw->LeftEdge + nw->Width - scr->Width);
	    if ((nw->TopEdge + nw->Height) > scr->Height)
		nw->TopEdge -= (nw->TopEdge + nw->Height - scr->Height);
	}

	/* make sure we aren't under minimum size */
	if (nw->Width < nw->MinWidth)
	    nw->Width = nw->MinWidth;
	if (nw->Height < nw->MinHeight)
	    nw->Height = nw->MinHeight;

	/* make sure we aren't over maximum size */
	if ((nw->MaxWidth != ~0) && (nw->Width > nw->MaxWidth))
	    nw->Width = nw->MaxWidth;
	if ((nw->MaxHeight != ~0) && (nw->Height > nw->MaxHeight))
	    nw->Height = nw->MaxHeight;

	/* return to the previous directory */
	CurrentDir (lock);

	/* free our temporary buffer */
	FreeVec (config_file);
    }

    /* return what was passed */
    return (nw);
}

BOOL SaveSnapShot (struct AppInfo * ai, struct Window * win, BPTR prefs_drawer, STRPTR name)
{
    struct FileInfoBlock *fib;
    BOOL retval = FALSE;
    LONG msize;

    /* compute buffer size */
    msize = sizeof (struct FileInfoBlock) + strlen (name) + strlen (WINEXT) + 2L;

    /* allocate temporary work area */
    if (fib = (struct FileInfoBlock *) AllocVec (msize, (MEMF_CLEAR | MEMF_PUBLIC)))
    {
	STRPTR config_file;
	BPTR lock;
	BPTR fh;

	/* go into the preference directory */
	lock = CurrentDir (prefs_drawer);

	/* setup the config_file pointer */
	config_file = MEMORY_FOLLOWING (fib);

	/* build the preference file name */
	sprintf (config_file, "%s.%s", name, WINEXT);

	/* save user preferences */
	if (fh = Open (config_file, MODE_NEWFILE))
	{
	    struct Screen *scr = win->WScreen;

	    /* write the current screen size */
	    if ((Write (fh, (UBYTE *) & (scr->Width), SSCRBOX)) == SSCRBOX)
	    {
		/* write the current window positions */
		if ((Write (fh, (UBYTE *) & (win->LeftEdge), SWINBOX)) == SWINBOX)
		{
		    /* write the information window positions */
		    if ((Write (fh, (UBYTE *) & (ai->ai_InfoLeft), SINFOBOX)) == SINFOBOX)
		    {
			/* indicate successful completion */
			retval = TRUE;
		    }
		}
	    }

	    /* close the file */
	    Close (fh);
	}			/* end of if opened pref file */

	/* return to the previous directory */
	CurrentDir (lock);

	/* free our temporary buffer */
	FreeVec (fib);
    }

    /* return with success or failure */
    return (retval);
}
