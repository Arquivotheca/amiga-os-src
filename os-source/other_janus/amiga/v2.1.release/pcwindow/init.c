
/**** init.c ****************************************************************
 * 
 * Initialization Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 20 jul 89   -BK         Fixed init of GlobalScreenHeight to use
 *                         GetScreenData() instead of hard coded at 200
 *                         this was to allow windows to open below the
 *                         200 line boundy on interlaced screens.
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 28 Jul 88   -RJ         Changed InitZaphod() to return BOOL, now use 
 *                         Alert() rather than Abort() with errors and 
 *                         return FALSE to callers if any error
 * 27 Jul 88   -RJ         Moved data file read calls to before OpenAuxTools()
 *                         so that IMTask will have valid key definitions 
 *                         when it begins to run
 * 7 Mar 86    =RJ Mical=  Created this file
 *
 ***************************************************************************/


#include "zaphod.h"

void CopyFont(font, bufptr)
struct TextFont *font;
UBYTE *bufptr;
{
   SHORT i, i2;
   UBYTE *charData, *thisChar;
   SHORT height, modulo;
   SHORT firstchar;
   WORD *charloc;

#if (DEBUG1 & DBG_COPY_FONT_ENTER)
   kprintf("Init.c:       CopyFont(font=0x%lx,bufptr=0x%lx)\n",font,bufptr);
#endif

   charData = (UBYTE *)font->tf_CharData;
   height = font->tf_YSize;
   modulo = font->tf_Modulo;
   firstchar = font->tf_LoChar;
   charloc = (WORD *)font->tf_CharLoc;

   bufptr += (firstchar * height * 2);

   for (i = firstchar; i <= font->tf_HiChar; i++)
      {
      thisChar = charData + ((*charloc) >> 3);
      charloc += 2;

      for (i2 = 0; i2 < (height * modulo); i2 += modulo)
         {
         *bufptr++ = *(thisChar + i2);
         *bufptr++ = 0;
         }
      }
}



void InitDisplayBitMap(display, superwindow, width)
struct Display *display;
struct SuperWindow *superwindow;
SHORT width;
{
   SHORT i, i2;
   UBYTE *buffer;

#if (DEBUG1 & DBG_INIT_DISPLAY_BIT_MAP_ENTER)
   kprintf("Init.c:       InitDisplayBitMap(display=0x%lx,superwindow=0x%lx,width=0x%lx)\n",display,superwindow,width);
#endif

   InitBitMap(&display->BufferBitMap, superwindow->DisplayDepth,
         width, ZAPHOD_HEIGHT);
   InitBitMap(&display->TextBitMap, superwindow->DisplayDepth, 
         BUFFER_WIDTH * CHAR_WIDTH, CHAR_HEIGHT);

   i = (width >> 3) * ZAPHOD_HEIGHT;   /* This are for graphics mode */ 
   i2 = i;
   buffer = display->Buffer;

   display->BufferBitMap.Planes[0] = buffer;
   display->BufferBitMap.Planes[1] = buffer + i2;
   /* These four should never be used, but what the hell */
   i2 += i;
   display->BufferBitMap.Planes[2] = buffer + i2;
   i2 += i;
   display->BufferBitMap.Planes[3] = buffer + i2;
   i2 += i;
   display->BufferBitMap.Planes[4] = buffer + i2;
   i2 += i;
   display->BufferBitMap.Planes[5] = buffer + i2;
}



