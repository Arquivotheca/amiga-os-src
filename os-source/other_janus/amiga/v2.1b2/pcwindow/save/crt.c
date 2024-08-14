
/* *** crt.c *****************************************************************
 * 
 * CRT Control Register Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 5 Mar 86    =RJ Mical=  Created this file
 *   
 * **************************************************************************/

#include "zaphod.h"
#include <janus/memory.h>
#include <janus/janusreg.h>

                      
#ifndef JANUS
UBYTE FakeColorCRT[CRT_IOBLOCKSIZE];
UBYTE FakeColorControl, FakeColorSelect, FakeColorStatus;
#endif
                   

UBYTE GetColorCRTSpecials();
void GetCRTSetUp();
void GetMonoModes();
void CopyCRTData();


void GetCRTSetUp(display, invert, forcedrevamp)
REGISTER struct Display *display;
BOOL invert, forcedrevamp;
/* All subsections of this routine which will change Modes must change
 * newModes and see to it that the assignment is made in the end.
 */
{
   REGISTER BOOL revampWindow, revampScreen;
   BOOL redrawWindow;
   REGISTER UBYTE reg;
   REGISTER SHORT offset;
   SHORT width;
   SHORT newModes;
   REGISTER struct SuperWindow *superwindow;

   if (superwindow = display->FirstWindow)
      if (FlagIsSet(superwindow->Flags, FROZEN_HOSEHEAD))
         return;

   MyLock(&display->DisplayLock);

   if (invert) display->ColorControl ^= -1;

   if (forcedrevamp) revampWindow = revampScreen = redrawWindow = TRUE;
   else revampWindow = revampScreen = redrawWindow = FALSE;

   newModes = display->Modes;

   /* If this is a color display, go get the extra register settings */
   if (FlagIsSet(display->Modes, COLOR_DISPLAY))
      {
      switch (GetColorCRTSpecials(display, &newModes))
         {
         case COLOR_CRT_REVAMP:
            revampWindow = revampScreen = TRUE;
            break;
         case COLOR_CRT_REDRAW:
            redrawWindow = TRUE;
            break;
         }
      }
   else GetMonoModes(display, &newModes);



/* === CRT_R10 is "Text Cursor Start Register" ========================= */ 
   reg = *(display->PCCRTRegisterBase + CRT_R10) & CRT_R10MASK;
   if (reg != (display->ZaphodCRT[CRT_R10] & CRT_R10MASK))
      {
      display->ZaphodCRT[CRT_R10] = reg;
      switch (reg & CURSOR_BLINK_MASK)
         {
         case CURSOR_STEADY:
            break;
         case CURSOR_OFF:
            break;
         case CURSOR_BLINK16:
            break;
         case CURSOR_BLINK32:
            break;
         }

      reg &= CURSOR_START_MASK;

      /* First, if this is a Monochrome cursor, round it down to
       * our humble size
       */
      if (FlagIsClear(display->Modes, COLOR_DISPLAY))
         {
         if (reg) reg = (reg + 2) >> 1;
         }
      if (reg >= CHAR_HEIGHT) reg = CHAR_HEIGHT - 1;

      if (reg > display->CursorBottom)
         {
         display->CursorTop = 0;
         display->CursorAltTop = reg;
         }
      else
         {
         display->CursorTop = reg;
         display->CursorAltTop = -1;
         }

      SetFlag(display->CursorFlags, CURSOR_MOVED);
      }


/* === CRT_R11 is "Horizontal Display Register" =================== */ 
   reg = *(display->PCCRTRegisterBase + CRT_R11) & CRT_R11MASK;
   if (reg != (display->ZaphodCRT[CRT_R11] & CRT_R11MASK))
      {
      display->ZaphodCRT[CRT_R11] = reg;

      /* This is the cursor height - 1 */

      /* First, if this is a Monochrome cursor, round it down to
       * our humble size
       */
      if (FlagIsClear(display->Modes, COLOR_DISPLAY))
         {
         if (reg) reg = (reg + 2) >> 1;
         }
      if (reg >= CHAR_HEIGHT) reg = CHAR_HEIGHT - 1;

      display->CursorBottom = reg;
      if ((reg = display->CursorAltTop) == -1) reg = display->CursorTop;
      if (reg > display->CursorBottom)
         {
         display->CursorTop = 0;
         display->CursorAltTop = reg;
         }
      else
         {
         display->CursorTop = reg;
         display->CursorAltTop = -1;
         }

      SetFlag(display->CursorFlags, CURSOR_MOVED);
      }


/* === CRT_R12-R13 comprise the "Start Address Register" ========== */
   if (FlagIsSet(display->Modes, COLOR_DISPLAY))
      {
      offset = *(display->PCCRTRegisterBase + CRT_R12) & CRT_R12MASK;
      offset = (offset << 8) 
            | (*(display->PCCRTRegisterBase + CRT_R13) & CRT_R13MASK);
      offset <<= 1;

      if (offset != display->PCDisplayOffset)
         {
         /* This is the address of the first displayed byte */
         display->PCDisplayOffset = offset;
         GetPCDisplay(display);
         redrawWindow = TRUE;
         }
      }


/* === CRT_R14-R15 comprise the "Text Cursor Register" ================ */
   offset = *(display->PCCRTRegisterBase + CRT_R14) & CRT_R14MASK;
   offset = (offset << 8) 
         | (*(display->PCCRTRegisterBase + CRT_R15) & CRT_R15MASK);

   
   if (offset != display->PCCursorOffset)
      {
      /* This is the byte address of the cursor location */
      display->PCCursorOffset = offset;
      offset = ((offset << 1) - display->PCDisplayOffset) >> 1;
      if (newModes & MEDIUM_RES) width = 40;
      else width = 80;
      /* I am about to change the cursor task variables.  I can do this
       * with impugnity only because I know that the cursor task locks
       * me out of doing this while it is accessing these same variables.
       */
      display->CursorRow = offset / width;
      display->CursorCol = offset - (display->CursorRow * width);
      SetFlag(display->CursorFlags, CURSOR_MOVED);
      }


/* === Final Dramatic Vamps ======================================= */
   if (revampWindow || revampScreen)
      RevampDisplay(display, newModes, FALSE, FALSE);
   else
      {
      display->Modes = newModes;
      if (display->FirstWindow)
         {
         display->FirstWindow->DisplayModes = newModes;
         if ((redrawWindow) && (display->FirstWindow->DisplayWindow))
            RenderWindow(display, TRUE, TRUE, TRUE, TRUE);
         }
      }
   Unlock(&display->DisplayLock);
}


UBYTE GetColorCRTSpecials(display, newModes)
REGISTER struct Display *display;
REGISTER USHORT *newModes;
/* This routine gets the values specific to the display representing
 * the Color Graphics adapter.      This routine returns TRUE if the window
 * (and screen) require revamping, else returns FALSE.
 */
{
   UBYTE returnvalue;
   REGISTER UBYTE reg;
   REGISTER USHORT color;
   USHORT savemodes;
   REGISTER UBYTE mask;
   BOOL intense;

   returnvalue = COLOR_CRT_NOTHING;

/* === CRT_COLORCONTROL Register ====================================== */
/* These flags represent a weird piece of design logic.  Let me make sure
 * that I've got this straight ...
 * TEXT_MODE:      is TRUE if bit 0x02 is clear.
 * MEDIUM_RES:      is TRUE if:
 *               - TEXT_MODE and bit 0x01 is clear.
 *               - NOT TEXT_MODE and bit 0x10 is clear.
 * BLACK_AND_WHITE:  seems that I can ignore this altogether.
 * BLINK_TEXT:      is TRUE if bit 0x20 is set.
 */
#ifdef JANUS
   reg = *(display->JanusStart + COLOR_CONTROL_OFFSET);
#else
   reg = FakeColorControl;
#endif

   reg &= CRT_CCMASK;
      
   if (reg != display->ColorControl)
      {
      if (reg & 0x02)
         {
         ClearFlag(*newModes, TEXT_MODE);
         if (reg & 0x10) ClearFlag(*newModes, MEDIUM_RES);
         else SetFlag(*newModes, MEDIUM_RES);
         }
      else
         {
         SetFlag(*newModes, TEXT_MODE);
         if (reg & 0x01) ClearFlag(*newModes, MEDIUM_RES);
         else SetFlag(*newModes, MEDIUM_RES);
         }
         
      /* I think that (reg & 0x08) is the video enable bit, which
       * programmers can use to cause the video to be enabled 
       * automatically after the mode is changed.  Maybe I should
       * copy this bit into the status register someday ... ???
       */
   
      /* This flag, when set, specifies that the high-order attribute
       * bit is to be used to designate that the character is blinking.
       * When this bit is clear, the high-order attribute bit is a pen
       * selector for the background color, allowing background colors
       * to use all 16 colors that the foreground can use.
       */
      if (reg & 0x20) SetFlag(*newModes, BLINK_TEXT);
      else ClearFlag(*newModes, BLINK_TEXT);

      if ((reg & 0x02) != (display->ColorControl & 0x02))
         returnvalue = COLOR_CRT_REVAMP;
      else
         {
         if (reg & 0x02) mask = 0x10;
         else mask = 0x01;
         if ((reg & mask) != (display->ColorControl & mask))
            returnvalue = COLOR_CRT_REVAMP;
         }

      if (returnvalue == COLOR_CRT_NOTHING)
         {
         if ((reg & 0x20) != (display->ColorControl & 0x20))
            returnvalue = COLOR_CRT_REDRAW;
         }

      display->ColorControl = reg;
      }


/* === CRT_COLORSELECT Register ======================================= */ 
#ifdef JANUS
   reg = *(display->JanusStart + COLOR_SELECT_OFFSET);
#else
   reg = FakeColorSelect;
#endif

   reg &= CRT_CSMASK;

   if (reg != display->ColorSelect)
      {
      display->ColorSelect = reg;
      /* The ColorSelect color can be:
       *      - background color in medium-res graphics
       *      - foreground color in high-res graphics
       *      - overscan border of medium-res alphanumeric (not supported)
       */
      if (reg & 0x08) intense = TRUE;
      else intense = FALSE;
      color = 0;
      if (reg & 0x04)
         {
         if (intense) color = HIGH_RED;
         else color = LOW_RED;
         }
      if (reg & 0x02)
         {
         if (intense) color |= HIGH_GREEN;
         else color |= LOW_GREEN;
         }
      if (reg & 0x01)
         {
         if (intense) color |= HIGH_BLUE;
         else color |= LOW_BLUE;
         }

      LowGraphicsColors[0][0][0] = LowGraphicsColors[0][1][0] = color;
      LowGraphicsColors[1][0][0] = LowGraphicsColors[1][1][0] = color;
      HighGraphicsColors[1] = color;

      /* reg & 0x10 is supposed to "select an alternate, intensified
       * set of colors."
       */
      if (reg & 0x10) ColorIntenseIndex = 1;
      else ColorIntenseIndex = 0;

      /* If reg & 0x20 is ...
       *      0   -      Set medium-res color palette to green/red/brown
       *      1   -      Set medium-res color pallete to cyan/magenta/white
       */
      if (reg & 0x20) SetFlag(*newModes, PALETTE_1);
      else ClearFlag(*newModes, PALETTE_1);
   
      savemodes = display->Modes;
      display->Modes = *newModes;
      SetColorColors(display, -1);
      display->Modes = savemodes;
      }


/* === CRT_COLORSTATUS Register ======================================= */ 
   /* So far, I don't see any need for this register */
   /*        reg = *(GetJanusStart() + ColorStatusOffset); */

               
   return (returnvalue);
}



void GetMonoModes(display, newModes)
REGISTER struct Display *display;
REGISTER USHORT *newModes;
/* This routine gets the values specific to the display representing
 * the Mono Graphics adapter.  This routine just sets up the newModes 
 * variable to reflect monochrome characteristics.
 */
{
   SetFlag(*newModes, TEXT_MODE);
   ClearFlag(*newModes, MEDIUM_RES);
   SetFlag(*newModes, BLINK_TEXT); /* Currently, MONO ignores this bit */
}



void CopyCRTData(display, invert)
REGISTER struct Display *display;
REGISTER BOOL invert;
/* The invert argument allows the data to be copied inverted.  With the
 * data inverted, you can then call GetCRTSetUp() and hope
 * that all the data will be considered and acted upon (since *all*
 * of the zaphod register copies will be different from the PC ones (poof)).
 */
{
   REGISTER SHORT i;

   for (i = CRT_R0; i < CRT_IOBLOCKSIZE; i += 2)
      {
      display->ZaphodCRT[i] = *(display->PCCRTRegisterBase + i);
      if (invert) display->ZaphodCRT[i] ^= -1;
      }

#ifdef JANUS
   display->ColorSelect = *(display->JanusStart + COLOR_SELECT_OFFSET);
   display->ColorControl = *(display->JanusStart + COLOR_CONTROL_OFFSET);
#else
   display->ColorSelect = FakeColorSelect;
   display->ColorControl = FakeColorControl;
#endif

   if (invert)
      {
      display->ColorSelect ^= -1;
      display->ColorControl ^= -1;
      }
}


