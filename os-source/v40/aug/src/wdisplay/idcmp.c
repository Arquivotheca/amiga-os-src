/* idcmp.c
 * Intuition event processing
 *
 */

#include "wdisplay.h"

#define	DB(x)	;

/* Process Intuition messages */
VOID HandleIDCMP (struct AppInfo * ai)
{
    struct IntuiMessage *imsg;
    BOOL mouse = FALSE;
    BOOL update;
    LONG px, py;
    LONG pw, ph;
    LONG w, h;
    WORD key;

    /* Pull all the messages from the port */
    while (imsg = (struct IntuiMessage *) GetMsg (ai->ai_IDCMP))
    {
	/* Reset the need update flag */
	update = FALSE;

	/* Remember where the mouse was */
	ai->ai_MouseX = imsg->MouseX - ai->ai_Window->BorderLeft;
	ai->ai_MouseY = imsg->MouseY - ai->ai_Window->BorderTop;

	/* Handle each type of event */
	switch (imsg->Class)
	{
	    case IDCMP_MOUSEBUTTONS:
		/* Get out if no picture */
		if (ai->ai_Picture == NULL)
		    break;

		/* Are we scaled? */
		if (ai->ai_Flags & AIF_SCALE)
		{
		    /* Get the box */
		    ai->ai_Box = DoButtons (imsg, ai->ai_Cursor);

		    /* Did they release the left mouse button */
		    if (imsg->Code == SELECTUP)
		    {
			/* Did they make a box? */
			if ((ai->ai_Box->Width > 3) && (ai->ai_Box->Height > 3))
			{
			    /* Are we zoomed in? */
			    if (ai->ai_Flags & AIF_ZOOMED)
			    {
				/* Calculate pixel size */
				pw = MAX (ai->ai_Columns * 100L / ai->ai_SWidth, 1L);
				ph = MAX (ai->ai_Rows * 100L / ai->ai_SHeight, 1L);

				/* Get the cursor box */
				px = ai->ai_CurColumn + (ULONG) (ai->ai_Box->Left - ai->ai_Window->BorderLeft) * 100 / pw;
				py = ai->ai_CurRow + (ULONG) (ai->ai_Box->Top - ai->ai_Window->BorderTop) * 100 / ph;
				w = (ULONG) ai->ai_Box->Width * 100 / pw;
				h = (ULONG) ai->ai_Box->Height * 100 / ph;
			    }
			    /* Nope, just normal scaling */
			    else
			    {
				/* Get the cursor box */
				px = (ULONG) ((ai->ai_Box->Left - ai->ai_Window->BorderLeft) * (UWORD) ai->ai_XFactor) / 100L;
				py = (ULONG) ((ai->ai_Box->Top - ai->ai_Window->BorderTop) * (UWORD) ai->ai_YFactor) / 100L;
				w = (ULONG) (ai->ai_Box->Width * (UWORD) ai->ai_XFactor) / 100L;
				h = (ULONG) (ai->ai_Box->Height * (UWORD) ai->ai_YFactor) / 100L;
			    }

			    /* How are we going to scale things? */
			    ai->ai_CurColumn = px;
			    ai->ai_CurRow = py;
			    ai->ai_SWidth = MAX (w, 1);
			    ai->ai_SHeight = MAX (h, 1);

			    /* Show that we are zoomed */
			    ai->ai_Flags |= (AIF_ZOOMED | AIF_CHANGED);

			    /* Need to freshen display */
			    update = TRUE;
			}

			/* Indicate need for update */
			ai->ai_Flags |= AIF_UPDATE;
		    }

		    /* Clear the box */
		    ai->ai_Box->Width = ai->ai_Box->Height = 0;
		}

		if (imsg->Code == SELECTDOWN)
		{
		    if (ai->ai_Flags & AIF_SCALE)
		    {
			px = (ULONG) ai->ai_MouseX * ai->ai_XFactor / 100L;
			py = (ULONG) ai->ai_MouseY * ai->ai_YFactor / 100L;
		    }
		    else
		    {
			/* Use the hand pointer */
			SetHandPointer (ai->ai_Window);

			/* Show that we are scrolling... */
			ai->ai_Flags |= AIF_SCROLL;

			px = (ULONG) (ai->ai_CurColumn + ai->ai_MouseX);
			py = (ULONG) (ai->ai_CurRow + ai->ai_MouseY);
		    }

		    /* Was there a double-click? */
		    if ((ABS(ai->ai_MouseXD - ai->ai_MouseX) <= 3) &&
			(ABS(ai->ai_MouseYD - ai->ai_MouseY) <= 3) &&
			(DoubleClick (ai->ai_DSecs, ai->ai_DMics, imsg->Seconds, imsg->Micros)))
		    {
			/* Are we sorta normal? */
			if (ai->ai_Flags & AIF_ZOOMED)
			{
			    /* Reset the scaling */
			    ai->ai_Flags &= ~AIF_ZOOMED;
			    ai->ai_Flags |= AIF_CHANGED;
			    ai->ai_CurColumn = ai->ai_CurRow = 0L;
			}
			else
			{
			    /* Toggle the scale flag */
			    ai->ai_Flags ^= AIF_SCALE;

			    ai->ai_CurColumn = px - (ai->ai_Columns / 2);
			    ai->ai_CurRow = py - (ai->ai_Rows / 2);
			}

			/* Set the modes */
			update = SetDisplayMode (ai);
		    }
		    else
		    {
			/* Remember this for next time */
			ai->ai_DSecs = imsg->Seconds;
			ai->ai_DMics = imsg->Micros;
		    }

		    /* Remember where they clicked */
		    ai->ai_MouseXD = ai->ai_MouseX;
		    ai->ai_MouseYD = ai->ai_MouseY;

		    /* Turn on IntuiTicks */
		    ModifyIDCMP (ai->ai_Window, IDCMP_SLOW);
		}
		else if (imsg->Code == SELECTUP)
		{
		    /* Turn off IntuiTicks */
		    ModifyIDCMP (ai->ai_Window, IDCMP_FLAGS);

		    /* Clear the current gadget */
		    ai->ai_CurGad = NULL;

		    /* Clear the scroll flag */
		    ai->ai_Flags &= ~AIF_SCROLL;

		    /* Clear the scroll pointer */
		    ClearPointer (ai->ai_Window);

		    /* Turn it off */
		    AbortCut (ai->ai_Window, ai->ai_Cursor);
		}
		break;

		/* Gadget down */
	    case IDCMP_GADGETDOWN:
		/* Remember the current gadget */
		ai->ai_CurGad = (struct Gadget *) imsg->IAddress;

		/* Turn on IntuiTicks */
		ModifyIDCMP (ai->ai_Window, IDCMP_SLOW);

	    case IDCMP_INTUITICKS:
	    case IDCMP_MOUSEMOVE:
		if (ai->ai_Picture == NULL)
		    break;

		if (ai->ai_CurGad)
		{
		    if (((ai->ai_CurGad->GadgetType & GTYP_GADGETTYPE) == GTYP_BOOLGADGET) &&
			!(ai->ai_CurGad->Flags & GFLG_SELECTED))
		    {
			break;
		    }

		    /* Figure out what to do */
		    switch (ai->ai_CurGad->GadgetID)
		    {
			case 3:
			    ai->ai_CurRow--;
			    update = TRUE;
			    break;

			case 4:
			    ai->ai_CurRow++;
			    update = TRUE;
			    break;

			case 20:
			    if (!(ai->ai_Flags & AIF_ZOOMED))
			    {
				ai->ai_Flags |= AIF_REFRESH;
				VScrollBarFunc (ai);
			    }
			    break;

			case 13:
			    ai->ai_CurColumn--;
			    update = TRUE;
			    break;

			case 14:
			    ai->ai_CurColumn++;
			    update = TRUE;
			    break;

			case 30:
			    if (!(ai->ai_Flags & AIF_ZOOMED))
			    {
				ai->ai_Flags |= AIF_REFRESH;
				HScrollBarFunc (ai);
			    }
			    break;
		    }
		}
		else if (ai->ai_Flags & AIF_SCALE)
		{
		    ai->ai_Box = DoMouseMove (imsg, 0, 0, ai->ai_Cursor);
		}
		else if (ai->ai_Flags & AIF_SCROLL)
		{
		    ai->ai_CurColumn += ai->ai_MouseXD - ai->ai_MouseX;
		    ai->ai_CurRow += ai->ai_MouseYD - ai->ai_MouseY;
		    ai->ai_MouseXD = ai->ai_MouseX;
		    ai->ai_MouseYD = ai->ai_MouseY;
		    mouse = TRUE;
		}

		/* If it was zoomed, then it was changed... */
		if (update && (ai->ai_Flags & AIF_ZOOMED))
		{
		    ai->ai_Flags |= AIF_CHANGED;
		}
		break;

	    case IDCMP_GADGETUP:
		if (ai->ai_Flags & AIF_ZOOMED)
		{
		    if ((ai->ai_CurGad->GadgetID == 20) ||
			(ai->ai_CurGad->GadgetID == 30))
		    {
			VScrollBarFunc (ai);
			HScrollBarFunc (ai);
			update = TRUE;
		    }

		    ai->ai_Flags |= AIF_CHANGED;
		}

		/* Clear the current gadget */
		ai->ai_CurGad = NULL;

		/* Turn off IntuiTicks */
		ModifyIDCMP (ai->ai_Window, IDCMP_FLAGS);
		break;

	    case IDCMP_CLOSEWINDOW:
		if (imsg->IDCMPWindow == ai->ai_InfoWin)
		{
		    /* Reply to the message */
		    ReplyMsg ((struct Message *) imsg);
		    imsg = NULL;

		    /* Close the information window */
		    CloseAboutWindow (ai);
		}
		else
		{
		    ai->ai_Done = TRUE;
		}
		break;

	    case IDCMP_REFRESHWINDOW:
		BeginRefresh (imsg->IDCMPWindow);
		ai->ai_Flags |= AIF_UPDATE;
		UpdateDisplay (ai);
		EndRefresh (imsg->IDCMPWindow, -1);
		break;

	    case IDCMP_SIZEVERIFY:
		break;

	    case IDCMP_NEWSIZE:
		/* Get the new inner size */
		h = ai->ai_Window->Height - ai->ai_Window->BorderTop - ai->ai_Window->BorderBottom;
		w = ai->ai_Window->Width - ai->ai_Window->BorderLeft - ai->ai_Window->BorderRight;

		/* See if the size has changed */
		if ((ai->ai_Columns != w) || (ai->ai_Rows != h))
		{
		    if (ai->ai_Flags & AIF_ZOOMED)
		    {
			/* Calculate pixel size */
			DB (printf ("W%ld H%ld : ", ai->ai_SWidth, ai->ai_SHeight));
			pw = MAX (ai->ai_Columns * 100L / ai->ai_SWidth, 1L);
			ph = MAX (ai->ai_Rows * 100L / ai->ai_SHeight, 1L);

			/* How are we going to scale things? */
			ai->ai_SWidth = MAX (((ULONG)w * 100 / pw), 1);
			ai->ai_SHeight = MAX (((ULONG)h * 100 / ph), 1);
			DB (printf ("W%ld H%ld\n", ai->ai_SWidth, ai->ai_SHeight));
		    }

		    /* Show that the window size has changed */
		    ai->ai_Flags |= (AIF_CHANGED | AIF_UPDATE);

		    /* Remember the inner size */
		    ai->ai_Columns = w;
		    ai->ai_Rows = h;

		    /* Try to allocate the secondary bitmap */
		    if (AllocSecondaryBM (ai))
		    {
			ai->ai_Flags |= AIF_SECONDARY;
		    }
		    else
		    {
			ai->ai_Flags &= ~(AIF_SECONDARY | AIF_SCALE);
		    }

		    /* Show that we need an update */
		    update = TRUE;
		}

	    case IDCMP_ACTIVEWINDOW:
	    case IDCMP_INACTIVEWINDOW:
		/* Update the status bar */
		StatusFunc (ai);
		break;

	    case IDCMP_RAWKEY:
		if (!(imsg->Code & IECODE_UP_PREFIX))
		{
		    /* Most times we want an update */
		    update = TRUE;

		    /* Handle keystrokes */
		    switch (imsg->Code)
		    {
			case 61:	/* Home */
			    ai->ai_CurRow = 0;
			    break;

			case 62:	/* 8 UpArrow */
			case 76:
			    if (imsg->Qualifier & SHIFT)
				ai->ai_CurRow -= ai->ai_Rows;
			    else if (imsg->Qualifier & ALT)
				ai->ai_CurRow = 0;
			    else
				ai->ai_CurRow--;
			    break;

			case 63:	/* PgUp */
			    ai->ai_CurRow -= ai->ai_Rows;
			    break;

			case 45:	/* 4 LeftArrow */
			case 79:
			    if (imsg->Qualifier & SHIFT)
				ai->ai_CurColumn -= ai->ai_Columns;
			    else if (imsg->Qualifier & ALT)
				ai->ai_CurColumn = 0;
			    else
				ai->ai_CurColumn--;
			    break;

			case 47:	/* 6 RightArrow */
			case 78:
			    if (imsg->Qualifier & SHIFT)
				ai->ai_CurColumn += ai->ai_Columns;
			    else if (imsg->Qualifier & ALT)
				ai->ai_CurColumn = (ai->ai_Picture) ? ai->ai_Picture->ir_BMHD.w : 0;
			    else
				ai->ai_CurColumn++;
			    break;

			case 29:	/* End */
			    ai->ai_CurRow = (ai->ai_Picture) ? ai->ai_Picture->ir_BMHD.h : 0;
			    break;

			case 30:	/* 2 DownArrow */
			case 77:
			    if (imsg->Qualifier & SHIFT)
				ai->ai_CurRow += ai->ai_Rows;
			    else if (imsg->Qualifier & ALT)
				ai->ai_CurRow = (ai->ai_Picture) ? ai->ai_Picture->ir_BMHD.w : 0;
			    else
				ai->ai_CurRow++;
			    break;

			case 31:	/* PgDn */
			    ai->ai_CurRow += ai->ai_Rows;
			    break;

			case 95:	/* HELP */
			    OpenAboutWindow (ai);
			    break;

			default:
			    update = FALSE;
			    /* Convert the keystroke */
			    if ((key = (WORD) DeadKeyConvert (imsg, ai->ai_Buffer, 15L, NULL)) > 0)
			    {
				switch (ai->ai_Buffer[0])
				{
				    case 'f':
				    case 'F':
					update = FullSizeFunc (ai);
					break;

				    case 's':
				    case 'S':
					update = ScaleSizeFunc (ai);
					break;

				    case 'Q':
				    case 'q':
				    case 27:
					ai->ai_Done = TRUE;
					break;
				}
			    }
			    break;
		    }

		    /* If it was zoomed, then it was changed... */
		    if (update && (ai->ai_Flags & AIF_ZOOMED))
		    {
			ai->ai_Flags |= AIF_CHANGED;
		    }
		}
		break;
	}

	if (imsg)
	{
	    /* Reply to the message */
	    ReplyMsg ((struct Message *) imsg);
	}

	if (update)
	{
	    UpdateDisplay (ai);
	}
    }

    if ((ai->ai_Flags & AIF_REFRESH) || mouse)
    {
	UpdateDisplay (ai);
    }
}
