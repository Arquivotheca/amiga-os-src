
/* *** render.c **************************************************************
 * 
 * This files contains the routines that render the various Zaphod windows
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 23 Feb 86   =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"

void RenderWindow();
void RenderWindowGrunt();
void RenderText();
void RenderGraphics();
void RefreshDisplay();

void RenderWindow(display, recalc, stuffTextBuffer, stuffLines, redrawWholeDisplay)
struct Display *display;
BOOL recalc, stuffTextBuffer, stuffLines, redrawWholeDisplay;
/* This routine calls RenderWindowGrunt() with arguments describing that
 * a rendering of the entire display should be done. 
 * Please see RenderWindowGrunt() for a description of this function and
 * its arguments.
 */
{
   RenderWindowGrunt(display, recalc, stuffTextBuffer, stuffLines,
             redrawWholeDisplay, -1, -1);
}



void RenderWindowGrunt(display, recalc, stuffTextBuffer, stuffLines,
      redrawWholeDisplay, startRow, endRow)
struct Display *display;
BOOL recalc, stuffTextBuffer, stuffLines, redrawWholeDisplay;
SHORT startRow, endRow;
/* Render the window anew.
 * If recalc is ...
 *      - TRUE:
 *        The display size has changed.  Therefore, the display parameters
 *        should be recalculated before the display is rendered.
 *      - FALSE:
 *        The display has not changed since the last render, or the
 *        parameters have already been recalculated, so leave
 *        the display parameters as they stand.
 * If stuffTextBuffer is ...
 *      - TRUE:    
 *        - Text Mode        
 *         If we're in text mode, then go and stuff the text buffer 
 *         from the PC buffer.  When stuffTextBuffer is TRUE recalc
 *        (above) should be TRUE too.
 *        - Graphics Mode
 *         if we're in graphics mode, then blast the PC memory into 
 *         the Buffer whether or not stuffTextBuffer is set.
 *      - FALSE:
 *        If we're in text mode, then FALSE means that the text buffer
 *        is still physically synchronized with the PC memory, even though
 *        the information content of the two buffers may have changed.
 *        So don't get a whole new copy of PC memory, and instead just make
 *        the comparisons based on the current state of things.
 *        If we're in graphics mode, the PC memory is blasted into the Buffer
 *   regardless, because the blast is so fast
 *        (less than 10 milliseconds under normal high-res conditions, around
 *        35 milliseconds under medium-res).
 * If stuffLines is ...
 *      - TRUE:    
 *        - Text Mode        
 *         If we're in text mode, this means that we want to force 
 *         the entire text display to be refreshed.  To achieve this, the
 *         NewStartCol and NewLength arrays won't be recalculated and 
 *         instead will be set equal to the entire text width, 
 *         which will force all of the buffer to be drawn.
 *        - Graphics Mode
 *         if we're in graphics mode, stuffLines has no meaning
 *      - FALSE:
 *        If we're in text mode, then FALSE means that the text buffer
 *        is still physically synchronized with the PC memory, even though
 *        the information content of the two buffers may have changed.
 *        So don't get a whole new copy of PC memory, and instead just make
 *        the comparisons based on the current state of things.  The
 *        NewStartCol and NewLengths arrays will be calculated
 *        based on the current states of the buffers.  If we're in graphics
 *        mode, stuffLines is meaningless.
 * If redrawWholeDisplay is ...
 *      - TRUE:
 *        We want to draw into any area of the entire display (excluding 
 *        borders) which is the usual case.
 *      - FALSE:
 *        The display doesn't need to be redrawn, only refreshed.  This 
 *        is the case when, for instance, we get a REFRESHWINDOW event 
 *        through the IDCMP, specifying that only part of the window
 *        need be refreshed.
 * startRow = the starting row for the rendering (in buffer coordinates).
 * endRow = the last row to be rendered (in buffer coordinates).       
 *
 * NOTE:  please be advised that even as you read this, users of the 
 * MoveAndSetRegion() function are being told that this routine ends 
 * with an EndUpdate(), which clears away the region created by the 
 * MoveAndSetRegion() call.
 */
{
   SHORT row, col;
   struct SuperWindow *superwindow;

   if ((superwindow = display->FirstWindow) == NULL) return;
   if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD)) return;

   MyLock(&display->DisplayLock);

   SetMaskAndLock(display, redrawWholeDisplay);

   if (recalc) RecalcDisplayParms(display);
   if (startRow < 0)
      {
      startRow = 0;
      endRow = display->TextHeight - 1;
      } 

   if (display->Modes & TEXT_MODE)
      {
      if (stuffTextBuffer) StuffTextBuffer(display, startRow, endRow);
      RenderText(display, stuffLines, startRow, endRow);

      /* Now, check if the cursor was overwritten and clear the cursor
       * flag if it was.
       */
      if (display->CursorFlags & CURSOR_ON)
         {
         row = display->CursorOldRow - display->TextStartRow;
         if ((row >= 0) || (row < display->TextHeight))
            {
            col = display->CursorOldCol - display->TextStartCol;
            if ((col < 0) || (col >= display->TextWidth))
               ClearFlag(display->CursorFlags, CURSOR_ON);
            else
               {
               if ((col >= 0) && (col < display->TextNewLength[row]))
                  ClearFlag(display->CursorFlags, CURSOR_ON);
               }
            }
         else ClearFlag(display->CursorFlags, CURSOR_ON);
         }
      }
   else
      {
      BlastGraphicsPlanes(display);
      RenderGraphics(display);
      }

   UnmaskUnlock(display);

   Unlock(&display->DisplayLock);
}



void RenderText(display, stuffLines, startRow, endRow)
struct Display *display;
BOOL stuffLines;
SHORT startRow, endRow;
{
   SHORT i;
   struct SuperWindow *superwindow;

   if ((superwindow = display->FirstWindow) == NULL) return;
   if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD)) return;

   if (stuffLines)
      {
      /* To force the refresh, simply set the NewLengths array to
       * indicate that all lines need complete refreshing
       */       
      for (i = startRow; i <= endRow; i++)
         display->TextNewLength[i] = display->TextWidth;
      }
   else RecalcNewLengths(display, startRow, endRow);

   DrawText(display);
}


void RenderGraphics(display)
struct Display *display;
{
   struct SuperWindow *superwindow;
   struct Window *window;

   if ((superwindow = display->FirstWindow) == NULL) return;
   if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD)) return;
   if ((window = superwindow->DisplayWindow) == NULL) return;

   BltBitMapRastPort(
         &display->BufferBitMap, 
         display->DisplayStartCol, display->DisplayStartRow, 
         window->RPort, 
         window->BorderLeft,
         window->BorderTop,
         display->DisplayWidth, display->DisplayHeight, 
         0x00C0);   /* Copies the planes to the window */
}
                


void RefreshDisplay(display)
struct Display *display;
{
   if (FlagIsSet(display->Modes, BORDER_VISIBLE | PAL_PRESENCE))
      {
      MyRefreshWindowFrame(display->FirstWindow->DisplayWindow, display);
      TopRightLines(display->FirstWindow->DisplayWindow);
      }

   RenderWindow(display, FALSE, TRUE, TRUE, TRUE);
}

