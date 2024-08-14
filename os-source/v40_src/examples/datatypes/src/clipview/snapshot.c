/******************************************************************************
 *
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992, 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 *
 ******************************************************************************
 * snapshot.c
 * Written by David N. Junod
 *
 */

#include "clipview.h"

/* The extension for window snapshots */
#define	NAME	"ENV:ClipView/ClipView.win"

/* Simple scaling of coordinates */
#define SCALE(a,b,c) (((b) * (c)) / (a))

struct IBox *LoadSnapShot (struct GlobalData * gd, struct IBox * box)
{
    struct Rectangle rect;
    SHORT Width, Height;
    ULONG mode;
    BPTR fh;

    /* load user preferences */
    if (fh = Open (NAME, MODE_OLDFILE))
    {
	/* read in the screen width */
	Read (fh, (UBYTE *) & (Width), sizeof (SHORT));

	/* read in the screen height */
	Read (fh, (UBYTE *) & (Height), sizeof (SHORT));

	/* read in the window rectangle */
	Read (fh, (UBYTE *) box, sizeof (struct IBox));

	/* close the file */
	Close (fh);

	/* validate the screen pointer */
	if (gd->gd_Screen)
	{
	    /* see if the screen size has changed */
	    if ((Width != gd->gd_Screen->Width) || (Height != gd->gd_Screen->Height))
	    {
		/* calculate new upper-left coordinates */
		box->Left = (WORD) SCALE (Width, gd->gd_Screen->Width, box->Left);
		box->Top = (WORD) SCALE (Height, gd->gd_Screen->Height, box->Top);

		box->Width = (WORD) SCALE (Width, gd->gd_Screen->Width, box->Width);
		box->Height = (WORD) SCALE (Height, gd->gd_Screen->Height, box->Height);
	    }

	    /* make sure we fit on the screen */
	    if ((box->Left + box->Width) > gd->gd_Screen->Width)
		box->Left -= (box->Left + box->Width - gd->gd_Screen->Width);
	    if ((box->Top + box->Height) > gd->gd_Screen->Height)
		box->Top -= (box->Top + box->Height - gd->gd_Screen->Height);
	}
    }
    else
    {
	box->Top = 1;
	box->Width = gd->gd_Screen->Width;
	box->Height = gd->gd_Screen->Height - 1;

	if ((mode = GetVPModeID (&gd->gd_Screen->ViewPort)) != INVALID_ID)
	{
	    QueryOverscan (mode, &rect, OSCAN_TEXT);
	    box->Width  = MIN (gd->gd_Screen->Width,  rect.MaxX - rect.MinX + 1);
	    box->Height = MIN (gd->gd_Screen->Height, rect.MaxY - rect.MinY + 1) - 1;
	}
	box->Width  -= 22;
	box->Height -= (gd->gd_Screen->BarHeight + 11);
    }

    /* return what was passed */
    return (box);
}

BOOL SaveSnapShot (struct GlobalData * gd, struct Window * win)
{
    BOOL retval = FALSE;
    struct IBox box;
    BPTR lock;
    LONG len;
    BPTR fh;

    if (win)
    {
	if (lock = CreateDir ("ENV:ClipView"))
	    UnLock (lock);

	if (fh = Open (NAME, MODE_NEWFILE))
	{
	    box = * ((struct IBox *)&win->LeftEdge);
	    box.Width -= (win->BorderLeft + win->BorderRight + 4);
	    box.Height -= (win->BorderTop + win->BorderBottom + 2);

	    len = 2L * sizeof (SHORT);
	    if ((Write (fh, (UBYTE *) & (gd->gd_Screen->Width), len)) == len)
		if ((Write (fh, (UBYTE *) &box, sizeof (struct IBox))) == sizeof (struct IBox))
		    retval = TRUE;

	    /* close the file */
	    Close (fh);
	}
    }
    return (retval);
}
