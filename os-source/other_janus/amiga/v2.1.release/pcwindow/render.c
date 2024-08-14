
/**** render.c **************************************************************
 * 
 * This files contains the routines that render the various Zaphod windows
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 23 Feb 86   =RJ Mical=  Created this file
 *
 ***************************************************************************/

#include "zaphod.h"

#define  DBG_RENDER_WINDOW_ENTER          1
#define  DBG_RENDER_WINDOW_GRUNT_ENTER    1
#define  DBG_RENDER_TEXT_ENTER            1
#define  DBG_RENDER_GRAPHICS_ENTER        1
#define  DBG_REFRESH_DISPLAY_ENTER        1

/****i* PCWindow/RenderWindow ******************************************
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
* This routine calls RenderWindowGrunt() with arguments describing that
* a rendering of the entire display should be done. 
* Please see RenderWindowGrunt() for a description of this function and
* its arguments.
*
*/

VOID RenderWindow(display, recalc, stuffTextBuffer, stuffLines, redrawWholeDisplay)
struct Display *display;
BOOL recalc, stuffTextBuffer, stuffLines, redrawWholeDisplay;
{
#if (DEBUG1 & DBG_RENDER_WINDOW_ENTER)
   kprintf("render.c:     RenderWindow(display=0x%lx,recalc=0x%lx,stuffTextBuffer=0x%lx\n              stuffLines=0x%lx,redrawWholeDisplay=0x%lx)\n",display,recalc,stuffTextBuffer,stuffLines,redrawWholeDisplay);
#endif

#if (DEBUG3 & DBG_RENDER_WINDOW_ENTER)
   kprintf("render.c:     RenderWindow: Width=%ld, Height=%ld\n",display->FirstWindow->DisplayWindow->Width,
                                                                 display->FirstWindow->DisplayWindow->Height);
#endif

   RenderWindowGrunt(display, recalc, stuffTextBuffer, stuffLines,
             redrawWholeDisplay, -1L, -1L);
}

/****i* PCWindow/RenderWindowGrunt *****************************************
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
* Render the window anew.
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
*        regardless, because the blast is so fast
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
*
*/

VOID RenderWindowGrunt(display, recalc, stuffTextBuffer, stuffLines,
      redrawWholeDisplay, startRow, endRow)
struct Display *display;
BOOL recalc, stuffTextBuffer, stuffLines, redrawWholeDisplay;
SHORT startRow, endRow;
{
   SHORT row, col;
   struct SuperWindow *superwindow;

#if (DEBUG1 & DBG_RENDER_WINDOW_GRUNT_ENTER)
   kprintf("render.c:     RenderWindowGrunt(display=0x%lx,recalc=0x%lx,stuffTextBuffer=0x%lx\nstuffLines=0x%lx,redrawWholeDisplay=0x%lx,startRow=0x%lx,endRow=0x%lx)\n",display,recalc,stuffTextBuffer,stuffLines,redrawWholeDisplay,startRow,endRow);
#endif

#if (DEBUG3 & DBG_RENDER_WINDOW_GRUNT_ENTER)
   kprintf("render.c:     RenderWindow: Width=%ld, Height=%ld\n",display->FirstWindow->DisplayWindow->Width,
                                                                 display->FirstWindow->DisplayWindow->Height);
#endif

   if ((superwindow = display->FirstWindow) == NULL) return;
   if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD)) return;

   MyLock(&display->DisplayLock);

   SetMaskAndLock(display, redrawWholeDisplay);

   if (recalc) RecalcDisplayParms(display);
   if (startRow < 0L)
   {
      startRow = 0L;
      endRow = display->TextHeight - 1L;
   } 

   if (display->Modes & TEXT_MODE)
   {
      if (stuffTextBuffer) 
	     StuffTextBuffer(display, startRow, endRow);
      RenderText(display, stuffLines, startRow, endRow);

      /* Now, check if the cursor was overwritten and clear the cursor
       * flag if it was.
       */
      if (display->CursorFlags & CURSOR_ON)
      {
         row = display->CursorOldRow - display->TextStartRow;
         if ((row >= 0L) || (row < display->TextHeight))
         {
            col = display->CursorOldCol - display->TextStartCol;
            if ((col < 0L) || (col >= display->TextWidth))
               ClearFlag(display->CursorFlags, CURSOR_ON);
            else
            {
               if ((col >= 0L) && (col < display->TextNewLength[row]))
                  ClearFlag(display->CursorFlags, CURSOR_ON);
            }
         }
         else ClearFlag(display->CursorFlags, CURSOR_ON);
      }
   } else {
      BlastGraphicsPlanes(display);
      RenderGraphics(display);
   }

   UnmaskUnlock(display);

   Unlock(&display->DisplayLock);
}

/****i* PCWindow/RenderText ******************************************
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

VOID RenderText(display, stuffLines, startRow, endRow)
struct Display *display;
BOOL stuffLines;
SHORT startRow, endRow;
{
   SHORT i;
   struct SuperWindow *superwindow;

#if (DEBUG1 & DBG_RENDER_TEXT_ENTER)
   kprintf("render.c:     RenderText(display=0x%lx,stuffLines=0x%lx,startRow=0x%lx,endRow=0x%lx)\n",display,stuffLines,startRow,endRow);
#endif

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

/****i* PCWindow/RenderGraphics ******************************************
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

VOID RenderGraphics(display)
struct Display *display;
{
   struct SuperWindow *superwindow;
   struct Window *window;

#if (DEBUG1 & DBG_RENDER_GRAPHICS_ENTER)
   kprintf("render.c:     RenderGraphics(display=0x%lx)\n",display);
#endif

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
                
/****i* PCWindow/RefreshDisplay ******************************************
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

VOID RefreshDisplay(display)
struct Display *display;
{
#if (DEBUG1 & DBG_REFRESH_DISPLAY_ENTER)
   kprintf("render.c:     RefreshDisplay(display=0x%lx)\n",display);
#endif

   if (FlagIsSet(display->Modes, BORDER_VISIBLE | PAL_PRESENCE))
   {
	  RefreshWindowFrame(display->FirstWindow->DisplayWindow);
   }
   RenderWindow(display, FALSE, TRUE, TRUE, TRUE);
}

/****i* PCWindow/StuffTextBuffer ******************************************
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
* This routine stuffs the text buffer with data from the current text page
* of the color graphics card according to the current text buffer parameters
*
*/

VOID StuffTextBuffer(display, startRow, endRow)
struct Display *display;
SHORT startRow, endRow;
{
   SHORT i, i2, rowwidth, longstartcol, longtextwidth;
   ULONG *pagerow, *pagepos, *bufferpos;
   struct SuperWindow *superwindow;

   if (superwindow = display->FirstWindow)
      if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD)) 
	     return;

   if (FlagIsSet(display->Modes, SELECTED_AREA)) 
      return;

   if (display->Modes & MEDIUM_RES)
      rowwidth = 20L;         /* 20 longs per row */
   else 
      rowwidth = 40L;         /* 40 longs per row */   

   longstartcol  = display->TextStartCol >> 1L;
   longtextwidth = display->TextWidth    >> 1L;

   pagerow = (ULONG *)(display->PCDisplay);

   /* Stupid C compiler makes me do this assignment first, else the
    * smartly-named CMX33 variable is flagged as undefined.
    */
   i = display->TextStartRow + startRow;
   pagerow += i * rowwidth;
   bufferpos = (ULONG *)(display->Buffer);
   bufferpos += startRow * longtextwidth;

   for(i = startRow; i <= endRow; i++)
   {
      pagepos = pagerow + longstartcol;
      pagerow += rowwidth;

      for (i2 = 0L; i2 < longtextwidth; i2++)
         *bufferpos++ = *pagepos++;
   }
}
