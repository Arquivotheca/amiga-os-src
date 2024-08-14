
/* *** border.c **************************************************************
 * 
 * Border Hither and Thither Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 1 Mar 86    =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"

void BorderPatrol();
void StripGadgets();
void RejoinGadgets();
void MoveSizeWait();

void BorderPatrol(display, selectType, redrawDisplay)
REGISTER struct Display *display;
SHORT selectType;
BOOL redrawDisplay;
{
   BOOL transition, visible;
   struct SuperWindow *superwindow;
   REGISTER struct Window *window;
   struct Gadget *gadget;
   REGISTER struct BorderKontrol *border;
   REGISTER SHORT i, ysize;
   REGISTER SHORT xmove, ymove;
   SHORT oldrow, oldcol, left, top, right, bottom;

   MyLock(&display->DisplayLock);

   if ((superwindow = display->FirstWindow) == NULL) goto OVER_THE_BORDER;
   if ((window = superwindow->DisplayWindow) == NULL) goto OVER_THE_BORDER;

   border = &superwindow->DisplayBorder;
   transition = FALSE;
   if (FlagIsSet(display->Modes, PAL_PRESENCE))
      {
      if (FlagIsSet(display->Modes, MEDIUM_RES))
         ysize = PAL_MEDIUM_OFFSET;
      else ysize = PAL_HIGH_OFFSET;
      }
   else ysize = 0;

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

   if (transition)
      {
      Forbid();      /* Zap!  This whole thing is critical! */

      if (FlagIsSet(display->Modes, BORDER_VISIBLE)) visible = TRUE;
      else visible = FALSE;

      if ( NOT visible )
         {
         /* OK, so let's hide that sucker! */
         ysize = -ysize;

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
                  if ((gadget->GadgetType & ~GADGETTYPE) != WDRAGGING)
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

         if (FlagIsClear(display->Modes, PAL_PRESENCE))
            {
            border->GadgetList = window->FirstGadget;
            window->FirstGadget = NULL;    /* Wham! */
            }
         else StripGadgets(border, window);

         border->Title = window->Title;
         if (TopBorderOff < TITLEBAR_HEIGHT)
            window->Title = NULL;         /* Boom! */

         border->Left = window->BorderLeft;
         window->BorderLeft = 0;         /* Biff! */

         border->Top = window->BorderTop;
         window->BorderTop = TopBorderOff; /* Socko! */

         border->Right = window->BorderRight;
         window->BorderRight = 0;         /* Crunch! */

         border->Bottom = window->BorderBottom;
         if (FlagIsClear(display->Modes, PAL_PRESENCE))
            window->BorderBottom = 0; /* Smash! */
         else
            window->BorderBottom += ysize;

         SetFlag(window->Flags, BORDERLESS); /* (silence) */
         }
      else
         {
         /* OK, so let's reveal the little darling */

         if (FlagIsClear(display->Modes, PAL_PRESENCE))
            window->FirstGadget = border->GadgetList;  /* Slam! */
         else
            RejoinGadgets(border, window);

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
         if (i < 0)
            {
            if (redrawDisplay)
               {
               MoveWindow(window, 0, i);
               }
            else window->TopEdge += i;
            }

         if (redrawDisplay)
            {
            if (newheight > window->MaxHeight) 
               {
               window->MaxHeight += ysize;
               SizeWindow(window, 0, ysize);
               }
            else
               {
               SizeWindow(window, 0, ysize);
               window->MaxHeight += ysize;
               }
            MoveSizeWait();
            }
         else 
            {
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

      if (ysize == 0)
         MoveAndSetRegion(display, xmove, ymove, left, top, right, bottom);

      RefreshDisplay(display);
      } /* end of "if (transition)" clause */


OVER_THE_BORDER:
   Unlock(&display->DisplayLock);
}



void StripGadgets(border, window)
struct Window *window;
struct BorderKontrol *border;
{
   REGISTER struct Gadget *gadget, *nextgadget;
   REGISTER struct Gadget *keeplist, *loselist;
   struct Gadget *keephead, *losehead;

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
}



void RejoinGadgets(border, window)
REGISTER struct Window *window;
REGISTER struct BorderKontrol *border;
{
   REGISTER struct Gadget *gadget;

   gadget = window->FirstGadget;
   if (gadget == NULL)
      window->FirstGadget = border->GadgetList;
   else
      {
      while (gadget->NextGadget) gadget = gadget->NextGadget;
      gadget->NextGadget = border->GadgetList;
      }
}



void MoveSizeWait()
{
   REGISTER SHORT i;
   for (i = 0; i < 5; i++) WaitTOF();
}



