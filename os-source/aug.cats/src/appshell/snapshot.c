/* snapshot.c
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * Routines to save/load a snapshot window position.  Uses the GetPrefsDrawer
 * function that shown in the "Application Preferences" article.
 * written by David N. Junod
 */

#if 1
#include "appshell_internal.h"
#else
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <libraries/dos.h>
#include <graphics/gfx.h>
#include <graphics/displayinfo.h>
#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec.h>
#include <pragmas/intuition.h>
#include <pragmas/graphics.h>
#include <pragmas/dos.h>
#include <stdio.h>
#include <string.h>

/* external library references for our pragmas */
extern struct Library *SysBase, *DOSBase, *IntuitionBase, *GfxBase;

#endif

void kprintf(void *, ...);
#define	DB(x)	;
#define	DW(x)	;

/* The extension for window snapshots */
#define	WINEXT "win"

/* Simple scaling of coordinates */
#define OSCALE(a,b,c) (((b) * (c)) / (a))
#define	SCALE(a,b,c)	( (((a) / 2) + ((b) * (c))) / (a) )

#ifndef MEMORY_FOLLOWING
/* Usually allocate one large block of memory for a group of items and then
 * divy out to the appropriate pointers.  Use with caution---must be
 * consistent with field types! */
#define MEMORY_FOLLOWING(ptr) ((void *)((ptr)+1))
#endif

/****** snapshot/LoadSnapShot ***********************************************
*
*   NAME
*	LoadSnapShot - Load a window snapshot
*
*   SYNOPSIS
*	new_win = LoadSnapShot ( new_win, prefs_drawer, win_name )
*
*	struct NewWindow *new_win;
*	BPTR prefs_drawer;
*	STRPTR win_name;
*
*   FUNCTION
*	This function loads the previously snapshotted window position.
*	Scales the window rectangle based on the screen OSCAN_TEXT width
*	and height.  If the window isn't sizeable, then the window width
*	and height fields are left alone.
*
*	The NewWindow structure, that is passed, should have a valid screen
*	pointer in the Screen field.
*
*	The window title may be used as the window name, if the title is
*	always the same (IE The user can't change it).
*
*   INPUTS
*	new_win		- Requires a valid NewWindow structure with the
*			  destination screen already placed in the Screen
*			  field.
*
*	prefs_drawer	- Lock on the directory to load preference files
*			  from.
*
*	win_name	- Name assigned to the window.
*
*   RETURNS
*	The original NewWindow structure with the window rectangle set to
*	the scaled snapshot rectangle.
*
*   SEE ALSO
*	SaveSnapShot(), GetPrefsDrawer()
*
**********************************************************************
*
* Created:  10-Jul-90, David N. Junod
* Updated:  03-Aug-90, cleaned up for AmigaMail
*
*/

struct NewWindow *
LoadSnapShot (struct NewWindow * nw, BPTR prefs_drawer, STRPTR name)
{
    STRPTR config_file;
    LONG msize;

    /* Make sure everything was passed to us */
    if (nw && prefs_drawer && name)
    {
	LONG num = (2L * sizeof (SHORT));
	struct Screen *scr = nw->Screen;
	struct ViewPort *vp = NULL;
	ULONG mode = INVALID_ID;
	struct Rectangle rect;
	SHORT Width, Height;

	/* Do we have a screen? */
	if (scr)
	{
	    /* Get a pointer to the ViewPort */
	    vp = &(scr->ViewPort);

	    /* Initialize */
	    rect.MaxX = scr->Width;
	    rect.MaxY = scr->Height;

	    /* Get the mode id of the screen */
	    if ((mode = GetVPModeID (vp)) != INVALID_ID)
	    {
		/* Fill in the window rectangle */
		QueryOverscan (mode, &rect, OSCAN_TEXT);

		/* Adjust */
		rect.MaxX++;
		rect.MaxY++;
	    }
	    else
	    {
		DB (kprintf("Couldn't GetVPModeID\n"));
	    }

	    /* Adjust for smaller than OSCAN_TEXT screens */
	    rect.MaxX = MIN (rect.MaxX, scr->Width);
	    rect.MaxY = MIN (rect.MaxY, scr->Height);
	}
	else
	{
	    DB (kprintf("No screen!\n"));
	}

	/* compute buffer size */
	msize = strlen (name) + strlen (WINEXT) + 2L;

	/* allocate temporary work area */
	if (config_file = (STRPTR) AllocVec (msize, (MEMF_CLEAR | MEMF_PUBLIC)))
	{
	    BPTR fh;
	    BPTR lock;

	    /* go into the preference directory */
	    lock = CurrentDir (prefs_drawer);

	    /* build the preference file name */
	    sprintf (config_file, "%s.%s", name, WINEXT);

	    /* load user preferences */
	    if (fh = Open (config_file, MODE_OLDFILE))
	    {
		/* read in the screen width */
		Read (fh, (UBYTE *) & (Width), sizeof (SHORT));

		/* read in the screen height */
		Read (fh, (UBYTE *) & (Height), sizeof (SHORT));

		/* see if we need to restore the width and height */
		if (nw->Flags & WINDOWSIZING)
		{
		    DW (kprintf ("Sizable\n"));
		    num = (4L * sizeof (SHORT));
		}
		else
		{
		    DW (kprintf ("Fixed\n"));
		}

		/* read in the window rectangle */
		Read (fh, (UBYTE *) & (nw->LeftEdge), (num * sizeof (nw->LeftEdge)));

		/* close the file */
		Close (fh);

		DW (kprintf ("0) %ld,%ld,%ld,%ld\n", (LONG)nw->LeftEdge, (LONG)nw->TopEdge,
						     (LONG)nw->Width, (LONG)nw->Height));

		/* validate the screen pointer */
		if (scr)
		{
		    /* see if the screen size has changed */
		    if ((Width != rect.MaxX) || (Height != rect.MaxY))
		    {
			/* calculate new upper-left coordinates */
			nw->LeftEdge = (WORD) SCALE (Width, rect.MaxX, nw->LeftEdge);
			nw->TopEdge = (WORD) SCALE (Height, rect.MaxY, nw->TopEdge);

			/* see if we need to scale the width and height */
			if (num == (4L * sizeof (SHORT)))
			{
			    nw->Width = (WORD) SCALE (Width, rect.MaxX, nw->Width);
			    nw->Height = (WORD) SCALE (Height, rect.MaxY, nw->Height);
			}
		    }

		    /* make sure we fit on the screen */
		    if ((nw->LeftEdge + nw->Width) > rect.MaxX)
			nw->LeftEdge -= (nw->LeftEdge + nw->Width - rect.MaxX);
		    if ((nw->TopEdge + nw->Height) > rect.MaxY)
			nw->TopEdge -= (nw->TopEdge + nw->Height - rect.MaxY);
		}

		DW (kprintf ("1) %ld,%ld,%ld,%ld\n", (LONG)nw->LeftEdge, (LONG)nw->TopEdge,
						     (LONG)nw->Width, (LONG)nw->Height));

		/* make sure we aren't under minimum size */
		if (nw->Width < nw->MinWidth)
		    nw->Width = nw->MinWidth;
		if (nw->Height < nw->MinHeight)
		    nw->Height = nw->MinHeight;

		DW (kprintf ("2) %ld,%ld,%ld,%ld\n", (LONG)nw->LeftEdge, (LONG)nw->TopEdge,
						     (LONG)nw->Width, (LONG)nw->Height));

		DW (kprintf ("Max %ld,%ld\n", (ULONG)nw->MaxWidth, (ULONG)nw->MaxHeight));

		/* make sure we aren't over maximum size */
		if ((nw->MaxWidth != 65535) && (nw->Width > nw->MaxWidth))
		    nw->Width = nw->MaxWidth;
		if ((nw->MaxHeight != 65535) && (nw->Height > nw->MaxHeight))
		    nw->Height = nw->MaxHeight;

		DW (kprintf ("3) %ld,%ld,%ld,%ld\n", (LONG)nw->LeftEdge, (LONG)nw->TopEdge,
						     (LONG)nw->Width, (LONG)nw->Height));
	    }

	    /* return to the previous directory */
	    CurrentDir (lock);

	    /* free our temporary buffer */
	    FreeVec (config_file);
	}			/* end of if allocate work area */

	/* Do we have a ViewPort pointer? */
	if (vp)
	{
	    /* Adjust for virtual screen */
	    nw->LeftEdge += -(vp->DxOffset);
	    nw->TopEdge += -(vp->DyOffset);

	    DB (kprintf("Adjust x %ld y %ld\n",
		(LONG)vp->DxOffset, (LONG)vp->DyOffset));
	}
    }
    else
    {
	DB (kprintf("NW 0x%lx Drawer 0x%lx Name 0x%lx\n",
	    nw, prefs_drawer, name));
    }

    /* return what was passed */
    return (nw);
}

/****** snapshot/SaveSnapShot ***********************************************
*
*   NAME
*	SaveSnapShot - Snapshot a window rectangle.
*
*   SYNOPSIS
*	SaveSnapShot ( win, prefs_drawer, win_name )
*
*	struct Window *win;
*	BPTR prefs_drawer;
*	STRPTR win_name;
*
*   FUNCTION
*	This function snapshots a windows rectangle.
*
*	The window title may be used as the window name, if the title is
*	always the same (IE The user can't change it).
*
*   INPUTS
*	win		- Requires an open Window structure.
*
*	prefs_drawer	- Lock on directory to store preference files in.
*
*	win_name	- Name assigned to the window.
*
*   SEE ALSO
*	LoadSnapShot(), GetPrefsDrawer()
*
**********************************************************************
*
* Created:  10-Jul-90, David N. Junod
* Updated:  03-Aug-90, cleaned up for AmigaMail
*
*/

BOOL
SaveSnapShot (struct Window * win, BPTR prefs_drawer, STRPTR name)
{
    struct FileInfoBlock *fib;
    BOOL retval = FALSE;
    LONG msize;

    /* Make sure everything was passed to us */
    if (win && prefs_drawer && name)
    {
	/* compute buffer size */
	msize = sizeof (struct FileInfoBlock) + strlen (name) + strlen (WINEXT) + 2L;

	/* allocate temporary work area */
	if (fib = (struct FileInfoBlock *)
	    AllocVec (msize, (MEMF_CLEAR | MEMF_PUBLIC)))
	{
	    STRPTR config_file;
	    BPTR lock = NULL;
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
		LONG len;
		struct Screen *scr = win->WScreen;
		struct ViewPort *vp = &(scr->ViewPort);
		struct Rectangle rect, winr;
		ULONG mode;

		/* Initialize */
		winr = *((struct Rectangle *)&(win->LeftEdge));
		DW (kprintf ("Copy %ld,%ld,%ld,%ld\n", (LONG)winr.MinX, (LONG)winr.MinY,
							(LONG)winr.MaxX, (LONG)winr.MaxY));

		rect.MaxX = scr->Width;
		rect.MaxY = scr->Height;

		/* Get the mode id of the screen */
		if ((mode = GetVPModeID (vp)) != INVALID_ID)
		{
			/* Fill in the window rectangle */
			QueryOverscan (mode, &rect, OSCAN_TEXT);

			/* Adjust */
			rect.MaxX++;
			rect.MaxY++;
		}

		/* Adjust for smaller than OSCAN_TEXT screens */
		rect.MaxX = MIN (rect.MaxX, scr->Width);
		rect.MaxY = MIN (rect.MaxY, scr->Height);

		/* Adjust for virtual screen */
		winr.MinX -= -(vp->DxOffset);
		winr.MinY -= -(vp->DyOffset);

		/* Subtract the border sizes */
		winr.MaxX -= (win->BorderLeft + win->BorderRight);
		winr.MaxY -= (win->BorderTop + win->BorderBottom);

		DW (kprintf ("Save %ld,%ld,%ld,%ld\n", (LONG)winr.MinX, (LONG)winr.MinY,
							(LONG)winr.MaxX, (LONG)winr.MaxY));

		/* write the current screen size */
		len = 2L * sizeof (SHORT);
		if ((Write (fh, (UBYTE *) & (rect.MaxX), len)) == len)
		{
		    /* write the current window positions */
		    len = 4L * sizeof (SHORT);
		    if ((Write (fh, (UBYTE *) & (winr), len)) == len)
		    {
			/* indicate successful completion */
			retval = TRUE;
		    }
		}

		/* close the file */
		Close (fh);
	    }			/* end of if opened pref file */

	    /* return to the previous directory */
	    CurrentDir (lock);

	    /* free our temporary buffer */
	    FreeVec (fib);
	}			/* end of if allocate work area */
    }

    /* return with success or failure */
    return (retval);
}
