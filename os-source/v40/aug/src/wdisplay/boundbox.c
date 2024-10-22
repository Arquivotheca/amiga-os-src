/* boundbox.c
 *
 */

#include "wdisplay.h"

#define	NUMPOINTS 5L
#define	TOTPOINTS (NUMPOINTS * 2L)

struct Cursor
{
    ULONG cs_IDCMP;		/* Previous IDCMP settings */
    ULONG cs_WinFlags;		/* Previous window flags */
    UWORD cs_Points[TOTPOINTS];	/* Cursor point array */
    UWORD cs_Pattern;		/* Pattern to use for cursor */

    /* These could easily be flags */
    SHORT cs_Mode;		/* Drawing mode */
    BOOL cs_Going;		/* Doing cursor? */

    struct IBox cs_Box;		/* Cursor rectangle */
};

BOOL BoundCheckMouse (struct IntuiMessage * msg)
{
    BOOL inside = TRUE;

    WORD tx = msg->IDCMPWindow->BorderLeft;
    WORD ty = msg->IDCMPWindow->BorderTop;
    WORD bx = msg->IDCMPWindow->Width - msg->IDCMPWindow->BorderRight - 1;
    WORD by = msg->IDCMPWindow->Height - msg->IDCMPWindow->BorderBottom - 1;

    /* Freshen the values */
    msg->MouseX = msg->IDCMPWindow->MouseX;
    msg->MouseY = msg->IDCMPWindow->MouseY;

    /* Range check the horizontal value */
    if (msg->MouseX < tx)
    {
	inside = FALSE;
        msg->MouseX = tx;
    }
    else if (msg->MouseX > bx)
    {
	inside = FALSE;
        msg->MouseX = bx;
    }

    /* Range check the vertical value */
    if (msg->MouseY < ty)
    {
	inside = FALSE;
        msg->MouseY = ty;
    }
    else if (msg->MouseY > by)
    {
	inside = FALSE;
        msg->MouseY = by;
    }

    return (inside);
}

static VOID UpdateBox (struct Cursor * cs)
{
    /* Revise the box */
    cs->cs_Box.Left = MIN (cs->cs_Points[0], cs->cs_Points[4]);
    cs->cs_Box.Top = MIN (cs->cs_Points[1], cs->cs_Points[5]);
    cs->cs_Box.Width = MAX (cs->cs_Points[0], cs->cs_Points[4]);
    cs->cs_Box.Height = MAX (cs->cs_Points[1], cs->cs_Points[5]);
    cs->cs_Box.Width = cs->cs_Box.Width - cs->cs_Box.Left + 1;
    cs->cs_Box.Height = cs->cs_Box.Height - cs->cs_Box.Top + 1;
}

static VOID AdjUpperLeft (SHORT xadj, SHORT yadj, struct Cursor * cs)
{
    cs->cs_Points[0] += xadj;
    cs->cs_Points[6] = cs->cs_Points[8] = cs->cs_Points[0];

    cs->cs_Points[1] += yadj;
    cs->cs_Points[3] = cs->cs_Points[9] = cs->cs_Points[1];
}

static VOID SetUpperLeft (struct IntuiMessage * msg, struct Cursor * cs)
{
    cs->cs_Points[0] = cs->cs_Points[2] = msg->MouseX;
    cs->cs_Points[4] = cs->cs_Points[6] = msg->MouseX;
    cs->cs_Points[8] = msg->MouseX;

    cs->cs_Points[1] = cs->cs_Points[3] = msg->MouseY;
    cs->cs_Points[5] = cs->cs_Points[7] = msg->MouseY;
    cs->cs_Points[9] = msg->MouseY;
}

static VOID AdjLowerRight (SHORT xadj, SHORT yadj, struct Cursor * cs)
{
    cs->cs_Points[2] += xadj;
    cs->cs_Points[4] = cs->cs_Points[2];

    cs->cs_Points[5] += yadj;
    cs->cs_Points[7] = cs->cs_Points[5];
}

static VOID SetLowerRight (struct IntuiMessage * msg, struct Cursor * cs)
{
    cs->cs_Points[2] = cs->cs_Points[4] = msg->MouseX;
    cs->cs_Points[5] = cs->cs_Points[7] = msg->MouseY;
}

struct Cursor *AllocAnts (VOID)
{
    struct Cursor *cs;

    /* Allocate our record */
    if (cs = (struct Cursor *) AllocVec (sizeof (struct Cursor), MEMF_CLEAR))
    {
	/* Initialize the pattern */
	cs->cs_Pattern = 0xF0F0;
    }

    return (cs);
}

VOID FreeAnts (struct Cursor * cs)
{
    /* Free the record */
    FreeVec (cs);
}

VOID AbortCut (struct Window * win, struct Cursor * cs)
{
    /* Clear the scroll pointer */
    ClearPointer (win);

    if (cs)
    {
	/* Is the cursor active? */
	if (cs->cs_Going)
	{
	    struct RastPort crp = *win->RPort;

	    SetDrMd (&crp, COMPLEMENT);
	    SetDrPt (&crp, cs->cs_Pattern);
	    Move (&crp, cs->cs_Points[0], cs->cs_Points[1]);
	    PolyDraw (&crp, NUMPOINTS, (WORD *) & (cs->cs_Points[0]));
	}

	/* Clear stuff */
	cs->cs_Going = FALSE;
	cs->cs_Mode = 0;
    }
}

struct IBox *DoButtons (struct IntuiMessage * msg, struct Cursor * cs)
{
    BOOL inside;

    if (cs)
    {
	/* Freshen & revise the coordinates */
	inside = BoundCheckMouse (msg);

	switch (msg->Code)
	{
	    case SELECTDOWN:
		/* Turn it off */
		AbortCut (msg->IDCMPWindow, cs);

		if (!inside)
		    break;

		/* Use the block pointer */
		SetBlockPointer (msg->IDCMPWindow);

		/* Set coordinates */
		SetUpperLeft (msg, cs);
		SetLowerRight (msg, cs);

		/* Revise the box */
		UpdateBox (cs);

		/* Draw the first frame */
		{
		    struct RastPort crp = *msg->IDCMPWindow->RPort;

		    SetDrMd (&crp, COMPLEMENT);
		    Move (&crp, cs->cs_Points[0], cs->cs_Points[1]);
		    PolyDraw (&crp, NUMPOINTS, (WORD *) & (cs->cs_Points[0]));
		}

		/* Set some pertinant values */
		cs->cs_Going = TRUE;
		cs->cs_Mode = 1;

		/* Save & modify the window flags */
		cs->cs_IDCMP = msg->IDCMPWindow->IDCMPFlags;
		cs->cs_WinFlags = msg->IDCMPWindow->Flags;
		msg->IDCMPWindow->Flags |= RMBTRAP;
		break;

	    case MENUDOWN:
		/* Turn it off */
		AbortCut (msg->IDCMPWindow, cs);

		/* Restore the window flags */
		if (!(cs->cs_WinFlags & RMBTRAP))
		{
		    msg->IDCMPWindow->Flags &= ~RMBTRAP;
		}

		/* Clear the box */
		cs->cs_Box.Width = cs->cs_Box.Height = 0;
		break;

	    case SELECTUP:
		/* set the bounding box rectangle if we where doing bounds */
		if (cs->cs_Mode == 1)
		{
		    /* Revise the box */
		    UpdateBox (cs);
		}

		/* Clear some values */
		cs->cs_Mode = 2;

		/* Restore the window flags */
		if (!(cs->cs_WinFlags & RMBTRAP))
		{
		    msg->IDCMPWindow->Flags &= ~RMBTRAP;
		}

		/* Turn off the cursor */
		AbortCut (msg->IDCMPWindow, cs);
		break;
	}

	return (&(cs->cs_Box));
    }

    return (NULL);
}

struct IBox *DoMouseMove (struct IntuiMessage * msg, SHORT xadj, SHORT yadj, struct Cursor * cs)
{
    if (cs)
    {
	if (cs->cs_Going)
	{
	    struct RastPort crp;
	    USHORT dir;

	    /* Copy the RastPort */
	    crp = *(msg->IDCMPWindow->RPort);

	    /* Check the mouse coordinates */
	    BoundCheckMouse (msg);

	    /* Erase the prior image */
	    SetDrMd (&crp, COMPLEMENT);
	    SetDrPt (&crp, cs->cs_Pattern);
	    Move (&crp, cs->cs_Points[0], cs->cs_Points[1]);
	    PolyDraw (&crp, NUMPOINTS, (WORD *) & (cs->cs_Points[0]));

	    /* Get the direction that we're traveling */
	    dir = ((cs->cs_Points[0] < cs->cs_Points[2]) ||
		   (cs->cs_Points[1] > cs->cs_Points[5])) ? 0 : 1;

	    /* Adjust the ants according to direction */
	    if (dir)
	    {
		cs->cs_Pattern = ((cs->cs_Pattern << 1) & 0xfffe) |
		  ((cs->cs_Pattern & 0x8000) >> 15);
	    }
	    else
	    {
		cs->cs_Pattern = ((cs->cs_Pattern >> 1) & 0x7fff) |
		  ((cs->cs_Pattern & 1) << 15);
	    }

	    if (xadj && yadj)
	    {
		AdjUpperLeft (xadj, yadj, cs);
	    }
	    else
	    {
		SetLowerRight (msg, cs);
	    }

	    /* Draw the new ants */
	    SetDrPt (&crp, cs->cs_Pattern);
	    Move (&crp, cs->cs_Points[0], cs->cs_Points[1]);
	    PolyDraw (&crp, NUMPOINTS, (WORD *) & (cs->cs_Points[0]));

	    /* Update the box */
	    if (cs->cs_Mode == 1)
	    {
		/* Revise the box */
		UpdateBox (cs);
	    }
	}

	return (&(cs->cs_Box));
    }
    return (NULL);
}
