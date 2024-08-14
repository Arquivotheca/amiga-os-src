
/* ***************************************************************************
 * 
 * Border Hither and Thither Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name	    Description
 * -------	------	    ------------------------------------------------
 * 1 Mar 86	=RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"


BorderPatrol(display, selectType, redrawDisplay)
struct Display *display;
SHORT selectType;
BOOL redrawDisplay;
{
    BOOL transition, visible;
    struct SuperWindow *superwindow;
    struct Window *window;
    struct Gadget *gadget;
    struct BorderKontrol *border;
    SHORT xmove, ymove, oldrow, oldcol, left, top, right, bottom;

    Lock(&display->DisplayLock);
				  
    if ((superwindow = display->FirstWindow) == NULL) goto OVER_THE_BORDER;
    if ((window = superwindow->DisplayWindow) == NULL) goto OVER_THE_BORDER;

    border = &superwindow->DisplayBorder;
    transition = FALSE;
			  
    switch (selectType)
	{
	case BORDER_TOGGLE:
	    transition = TRUE;
	    ToggleFlag(display->Modes, BORDER_VISIBLE);
	    break;
	case BORDER_ON:
	    if ( (display->Modes & BORDER_VISIBLE) == 0 )
		{
		transition = TRUE;
		SetFlag(display->Modes, BORDER_VISIBLE);
		}
	    break;
	case BORDER_OFF:
	    if (display->Modes & BORDER_VISIBLE)
		{
		transition = TRUE;
		ClearFlag(display->Modes, BORDER_VISIBLE);
		}
	    break;
	}

    if (transition)
	{
	Forbid();	/* Zap!  This whole thing is critical! */

	if (display->Modes & BORDER_VISIBLE) visible = TRUE;
	else visible = FALSE;

	if ( NOT visible )
	    {
	    /* OK, so let's hide that sucker! */
	    do
		{
		gadget = window->FirstGadget;
		while (gadget)
		    {
		    if (gadget->Flags & SELECTED)
			{
			/* Intuition 1.1 bug doesn't toggle the SELECTED
			 * flag of the window dragging gadget correctly.
			 * Fortunately, the purpose of this anti-SELECTED
			 * test loop is just to avoid screwing up Intuition
			 * rendering the gadgets, and no rendering takes 
			 * place with dragging gadgets, so I'm off the hook!
			 */
			if ((gadget->GadgetType & ~GADGETTYPE) NOT= WDRAGGING)
			    goto BREAKOUT;
			}
		    gadget = gadget->NextGadget;   
		    }
BREAKOUT:
		if (gadget) WaitTOF();	
		}
	    while (gadget);

	    /* Remember, we're still in Forbid() at this point, so
	     * it's safe to do weird things to the gadgets now, since
	     * we've stopped the system (except interrupts) from doing
	     * anything to step on our efforts, unless of course we do
	     * something that could cause us to Wait() (like practically
	     * any of the graphics functions), and at the same time we are
	     * now sure that no gadget is selected nor will be until we
	     * relinquish kontrol or do something stupid.
	     * ??? For Hanover, just delete the entire gadget list from
	     * the window and restore it later.  The final solution
	     * uses Selectomatica, which I may not have time to
	     * implement for Hanover.  Tant pis!
	     */
	    border->GadgetList = window->FirstGadget;
	    window->FirstGadget = NULL;     /* Wham! */
	    
	    border->Title = window->Title;
	    window->Title = NULL;	    /* Boom! */

	    border->Left = window->BorderLeft;
	    window->BorderLeft = 0;	    /* Biff! */

	    border->Top = window->BorderTop;
	    window->BorderTop = 0;	    /* Socko! */

	    border->Right = window->BorderRight;
	    window->BorderRight = 0;	    /* Crunch! */

	    border->Bottom = window->BorderBottom;
	    window->BorderBottom = 0;	    /* Smash! */

	    SetFlag(window->Flags, BORDERLESS); /* (silence) */
	    }
	else
	    {
	    /* OK, so let's reveal the little darling */

	    window->FirstGadget = border->GadgetList;  /* Slam! */
	    window->Title = border->Title;     
	    window->BorderLeft = border->Left;
	    window->BorderTop = border->Top;
	    window->BorderRight = border->Right;
	    window->BorderBottom = border->Bottom;
	    ClearFlag(window->Flags, BORDERLESS);
	    }

	Permit();

	if (NOT redrawDisplay) goto OVER_THE_BORDER;

	xmove = border->Left;
	ymove = border->Top;
	if (visible)
	    {
	    xmove = -xmove;
	    ymove = -ymove;
	    }
	oldrow = display->DisplayStartRow;
	oldcol = display->DisplayStartCol;
	RecalcDisplayParms(display);
	xmove += (display->DisplayStartCol - oldcol);
	ymove += (display->DisplayStartRow - oldrow);
	left = 0;
	top = 0;
	right = window->Width - 1;
	bottom = window->Height - 1;

	if (NOT visible)
	    {
	    left = border->Left;
	    top = border->Top;
	    right -= border->Right;
	    bottom -= border->Bottom;
	    if (xmove >= 0)
		{
		left -= xmove;
		if (left < 0) left = 0;
		}
	    else
		{
		right -= xmove;
		if (right >= window->Width) right = window->Width - 1;
		}
	    if (ymove >= 0)
		{
		top -= ymove;
		if (top < 0) top = 0;
		}
	    else
		{
		bottom -= ymove;
		if (bottom >= window->Height) bottom = window->Height - 1;
		}
	    }

	MoveAndSetRegion(display, xmove, ymove, left, top, right, bottom);

	RefreshDisplay(display);
	}


OVER_THE_BORDER:
    Unlock(&display->DisplayLock);
}


