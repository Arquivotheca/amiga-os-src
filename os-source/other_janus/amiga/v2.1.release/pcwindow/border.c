
/**** border.c **************************************************************
 * 
 * Border Hither and Thither Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 1 Mar 86    =RJ Mical=  Created this file
 *
 ***************************************************************************/

#include "zaphod.h"

#define  DBG_BORDER_PATROL_ENTER          1
#define  DBG_BORDER_PATROL_TOP_EDGE_CALC  1
#define  DBG_BORDER_PATROL_RETURN         1
#define  DBG_STRIP_GADGETS_ENTER          1
#define  DBG_STRIP_GADGETS_RETURN         1
#define  DBG_REJOIN_GADGETS_ENTER         1
#define  DBG_REJOIN_GADGETS_RETURN        1
#define  DBG_MOVE_SIZE_WAIT_ENTER         1
#define  DBG_MOVE_SIZE_WAIT_RETURN        1

/****i* PCWindow/BorderPatrol ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID BorderPatrol(display, selectType, redrawDisplay)
struct Display *display;
SHORT selectType;
BOOL redrawDisplay;
{
   BOOL transition, visible;
   struct SuperWindow *superwindow;
   struct Window *window;
   struct Gadget *gadget;
   struct BorderKontrol *border;
   SHORT i, ysize;
   SHORT xmove, ymove;
   SHORT oldrow, oldcol, left, top, right, bottom;

#if (DEBUG1 & DBG_BORDER_PATROL_ENTER)
   kprintf("Border.c:     BorderPatrol(display=0x%lx,selectType=0x%lx,redrawDisplay=0x%lx)\n",display,selectType,redrawDisplay);
#endif

   /****************/
   /* Lock Display */
   /****************/
   MyLock(&display->DisplayLock);

   /*********************/
   /* If no Window exit */
   /*********************/
   if ((superwindow = display->FirstWindow) == NULL) goto OVER_THE_BORDER;
   if ((window = superwindow->DisplayWindow) == NULL) goto OVER_THE_BORDER;

   border = &superwindow->DisplayBorder;
   transition = FALSE;

   /************/
   /* PAL junk */
   /************/
   if (FlagIsSet(display->Modes, PAL_PRESENCE))
   {
      if (FlagIsSet(display->Modes, MEDIUM_RES))
         ysize = PAL_MEDIUM_OFFSET;
      else 
	     ysize = PAL_HIGH_OFFSET;
   }
   else ysize = 0L;


   /**********************/
   /* Type of transition */
   /**********************/
   switch (selectType)
   {
      case BORDER_TOGGLE:
         transition = TRUE;
         ToggleFlag(display->Modes, BORDER_VISIBLE);
         break;
      case BORDER_ON:
         if (FlagIsClear(display->Modes, BORDER_VISIBLE))
         {
            transition = TRUE;
            SetFlag(display->Modes, BORDER_VISIBLE);
         }
         break;
      case BORDER_OFF:
         if (FlagIsSet(display->Modes, BORDER_VISIBLE))
         {
            transition = TRUE;
            ClearFlag(display->Modes, BORDER_VISIBLE);
         }
         break;
   }

   /************************/
   /* Really change border */
   /************************/
   if (transition)
   {
      Forbid();      /* Zap!  This whole thing is critical! */

      if (FlagIsSet(display->Modes, BORDER_VISIBLE)) 
	     visible = TRUE;
      else 
	     visible = FALSE;

      if ( NOT visible ) {
         /* OK, so let's hide that sucker! */
         ysize = -ysize;

         do {
            gadget = window->FirstGadget;
            while (gadget) {
               if (gadget->Flags & SELECTED) {
                  /* Intuition 1.1 bug doesn't toggle the SELECTED
                   * flag of the window dragging gadget correctly.
                   * Fortunately, the purpose of this anti-SELECTED
                   * test loop is just to avoid screwing up Intuition
                   * rendering the gadgets, and no rendering takes 
                   * place with dragging gadgets, so I'm off the hook!
                   */
                  if ((gadget->GadgetType & ~GADGETTYPE) != WDRAGGING)
                     goto BREAKOUT;
               }
               gadget = gadget->NextGadget;   
            }
BREAKOUT:
            if (gadget) WaitTOF();      
         } while (gadget);

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

         if (FlagIsClear(display->Modes, PAL_PRESENCE)) {
            border->GadgetList = window->FirstGadget;
            window->FirstGadget = NULL;    /* Wham! */

         }
         else StripGadgets(border, window);

         border->Title = window->Title;
         if (TopBorderOff < TITLEBAR_HEIGHT)window->Title = NULL;  /* Boom! */
         border->Left = window->BorderLeft;
         window->BorderLeft = 0L;         /* Biff! */

         border->Top = window->BorderTop;
         window->BorderTop = TopBorderOff; /* Socko! */

         border->Right = window->BorderRight;
         window->BorderRight = 0L;         /* Crunch! */

         border->Bottom = window->BorderBottom;
         if (FlagIsClear(display->Modes, PAL_PRESENCE))
		window->BorderBottom = 0L; /* Smash! */
         else
            window->BorderBottom += ysize;

         SetFlag(window->Flags, BORDERLESS); /* (silence) */
      }
      else {
         /* OK, so let's reveal the little darling */

         if (FlagIsClear(display->Modes, PAL_PRESENCE))
            window->FirstGadget = border->GadgetList;  /* Slam! */
         else  RejoinGadgets(border, window);

         window->Title = border->Title;
         i = window->BorderLeft;
         window->BorderLeft = border->Left;
         border->Left = i;
         i = window->BorderTop;
         window->BorderTop = border->Top;
         border->Top = i;
         i = window->BorderRight;
         window->BorderRight = border->Right;
         border->Right = i;
         i = window->BorderBottom;
         window->BorderBottom = border->Bottom;
         border->Bottom = i;
         ClearFlag(window->Flags, BORDERLESS);
      }

      if (ysize)
      {
         SHORT newheight;

         newheight = window->Height + ysize;

         i = window->WScreen->Height - (window->TopEdge + newheight);
#if (DEBUG3 & DBG_BORDER_PATROL_TOP_EDGE_CALC)
   kprintf("Border.c:     BorderPatrol: TopEdge       = %ld\n",window->TopEdge);
   kprintf("                            WScreenHeight = %ld\n",window->WScreen->Height);
   kprintf("                            i             = %ld\n",i);
#endif
         if (i < 0L)
         {
            if (redrawDisplay)
            {
               MoveWindow(window, 0L, i);
            }
            else window->TopEdge += i;
         }

         if (redrawDisplay)
         {
            if (newheight > window->MaxHeight) 
            {
               window->MaxHeight += ysize;
               SizeWindow(window, 0L, ysize);
            } else {
               SizeWindow(window, 0L, ysize);
               window->MaxHeight += ysize;
            }
            MoveSizeWait();
         } else  {
            window->MaxHeight += ysize;
            window->Height += ysize;
         }
      }

      Permit();

      if (NOT redrawDisplay)
      {
         RecalcDisplayParms(display);
         goto OVER_THE_BORDER;
      }

      xmove = border->Left - window->BorderLeft;
      ymove = border->Top - window->BorderTop;

      oldrow = display->DisplayStartRow;
      oldcol = display->DisplayStartCol;
      RecalcDisplayParms(display);
      xmove += (display->DisplayStartCol - oldcol);
      ymove += (display->DisplayStartRow - oldrow);
      left = 0L;
      top = 0L;
      right = window->Width - 1L;
      bottom = window->Height - 1L;

      if (NOT visible)
      {
         left = border->Left;
         top = border->Top;
         right -= border->Right;
         bottom -= border->Bottom;
         if (xmove >= 0L)
         {
            left -= xmove;
            if (left < 0L) left = 0L;
         } else {
            right -= xmove;
            if (right >= window->Width) right = window->Width - 1L;
         }
         if (ymove >= 0L)
         {
            top -= ymove;
            if (top < 0L) top = 0L;
         } else {
            bottom -= ymove;
            if (bottom >= window->Height) bottom = window->Height - 1L;
         }
      }

      if (ysize == 0L)
         MoveAndSetRegion(display, xmove, ymove, left, top, right, bottom);

      RefreshDisplay(display);
   } /* end of "if (transition)" clause */


OVER_THE_BORDER:
   Unlock(&display->DisplayLock);

#if (DEBUG2 & DBG_BORDER_PATROL_RETURN)
   kprintf("Border.c:     BorderPatrol: Returns(VOID)\n");
#endif
}

/****i* PCWindow/StripGadgets ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID StripGadgets(border, window)
struct Window *window;
struct BorderKontrol *border;
{
   struct Gadget *gadget, *nextgadget;
   struct Gadget *keeplist, *loselist;
   struct Gadget *keephead, *losehead;

#if (DEBUG1 & DBG_STRIP_GADGETS_ENTER)
   kprintf("Border.c:     StripGadgets(border=0x%lx,window=0x%lx)\n",border,window);
#endif

   gadget = window->FirstGadget;
   keeplist = loselist = NULL;
   keephead = losehead = NULL;
   while (gadget)
      {
      nextgadget = gadget->NextGadget;

      switch (gadget->GadgetType & ~GADGETTYPE)
      {
         case WDRAGGING:
         case WUPFRONT:
         case WDOWNBACK:
         case CLOSE:
            if (keephead == NULL) keephead = gadget;
            if (keeplist) keeplist->NextGadget = gadget;
            keeplist = gadget;
            gadget->NextGadget = NULL;
            break;
         default:
            if (losehead == NULL) losehead = gadget;
            if (loselist) loselist->NextGadget = gadget;
            loselist = gadget;
            gadget->NextGadget = NULL;
            break;
      }
      gadget = nextgadget;
   }

   window->FirstGadget = keephead;
   border->GadgetList = losehead;

#if (DEBUG2 & DBG_STRIP_GADGETS_RETURN)
   kprintf("Border.c:     StripGadgets: Returns(VOID)\n");
#endif
}

/****i* PCWindow/RejoinGadgets ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID RejoinGadgets(border, window)
struct Window *window;
struct BorderKontrol *border;
{
   struct Gadget *gadget;

#if (DEBUG1 & DBG_REJOIN_GADGETS_ENTER)
   kprintf("Border.c:     RejoinGadgets(border=0x%lx,window=0x%lx)\n",border,window);
#endif

   gadget = window->FirstGadget;
   if (gadget == NULL)
      window->FirstGadget = border->GadgetList;
   else
   {
      while (gadget->NextGadget) 
	     gadget = gadget->NextGadget;
      gadget->NextGadget = border->GadgetList;
   }

#if (DEBUG2 & DBG_REJOIN_GADGETS_RETURN)
   kprintf("Border.c:     RejoinGadgets: Returns(VOID)\n");
#endif
}

/****i* PCWindow/MoveSizeWait ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*      Waits 2/10 sec to be sure that Intuition has recieved at least
*      one input event and actually finished moving or resizing our window.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*      A more eloborate scheme using the NEWSIZE IDCMP event to synchronize
*      would be better. 
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID MoveSizeWait()
{
   SHORT i;

   for (i = 0L; i < 12L; i++) 
      WaitTOF();
}
