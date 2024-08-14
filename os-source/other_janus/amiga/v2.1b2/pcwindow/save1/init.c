
/* *** init.c ****************************************************************
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
 * **************************************************************************/

#include "zaphod.h"
#include <janus/janusbase.h>
#include <exec/memory.h>
#include <graphics/gfxbase.h>

#define  DBG_INIT_ZAPHOD_ENTER            1
#define  DBG_INIT_ZAPHOD_LIBS             1
#define  DBG_INIT_ZAPHOD_RETURN           1
#define  DBG_COPY_FONT_ENTER              1
#define  DBG_INIT_DISPLAY_BIT_MAP_ENTER   1

BOOL   InitZaphod(display)
REGISTER struct Display *display;
{
   REGISTER struct   TextFont *localFont;
   REGISTER SHORT    i;
   BOOL     retvalue;

#if (DEBUG1 & DBG_INIT_ZAPHOD_ENTER)
   kprintf("Init.c:       InitZaphod(display=0x%lx)\n",display);
#endif

   retvalue = FALSE;

/*
   if (IntuitionBase == NULL)
      IntuitionBase = (struct IntuitionBase *)
            OpenLibrary((UBYTE *)"intuition.library", 0);
*/

   MakeWaitWindow();

   if (GfxBase == NULL)
   {
      if((GfxBase = (struct GfxBase *)OpenLibrary((UBYTE *)"graphics.library", 0))==NULL)
      {
         MyAlert(ALERT_NO_GRAPHICS_LIB, NULL);
         goto DONE;
      }
   }

   if (LayersBase == NULL)        
   {
      if((LayersBase = (struct LayersBase *)OpenLibrary((UBYTE *)"layers.library",0))==NULL)
      {
         MyAlert(ALERT_NO_LAYERS_LIB, NULL);
         goto DONE;
      }
   }

   if (DiskfontBase == NULL)
   {
      if((DiskfontBase = (struct DiskfontBase *)OpenLibrary((UBYTE *)"diskfont.library", 0))==NULL)
      {
         MyAlert(ALERT_NO_DISKFONT_LIB, NULL);
         goto DONE;
      }
   }

   if (JanusBase == NULL)
   {
      if((JanusBase = (struct JanusBase *)OpenLibrary((UBYTE *)"janus.library", 0))==NULL)
      {
         MyAlert(ALERT_NO_JANUS_LIB, NULL);
         goto DONE;
      }
   }

/*
   if ( (GfxBase == NULL) || (IntuitionBase == NULL)
         || (LayersBase == NULL) || (DiskfontBase == NULL)
         || (JanusBase == NULL) )
      {
      MyAlert(ALERT_NO_LIBRARY, NULL);
      goto DONE;
      }
*/

#if (DEBUG3 & DBG_INIT_ZAPHOD_LIBS)
   kprintf("Init.c:       InitZaphod: Libraries opened OK\n");
#endif

   /* These two lines take advantage of the fact that we know that 
    * the JanusAmiga structure resides at the first memory location 
    * of Janus Parameter Memory.  We feel it's safe to presume this 
    * because it's a supposedly unchangeable presumption used by 
    * the original Janus designers (both Amiga and PC sides) to 
    * locate JanusAmiga, as witnessed by the fact that JanusBase 
    * has no explicit pointer to JanusAmiga and the PC code presumes 
    * everywhere that JanusAmiga starts at location zero of janus mem.
    */
   JanusAmigaBA = (struct JanusAmiga *)MakeBytePtr((APTR)JanusBase->jb_ParamMem);
   JanusAmigaWA = (struct JanusAmiga *)MakeWordPtr((APTR)JanusAmigaBA);


#ifdef AZTEC
   /* This pushes A4 into a memory location.  This is needed because
    * Manx has implemented relative-addressing using A4.  
    * Tasks must call GetA4(); to load the A4 register with the base.
    * Task object files should be clustered around the call to SaveA4();
    */
   SaveA4();
#endif

   MainTask = FindTask(0);

   ReadKeyTable();
   ReadScanCodeTable();

   OpenAuxTools(display);

   if (DuplicateDisplay(display)) 
   {
      MyAlert(ALERT_ALREADY_OPEN, NULL);
      goto DONE;
   }

   /* "If we're in text mode, then allocate only enough memory for one
    * page of text.  If we're in graphics mode, we'll need the whole 16K."
    * Nah, just grab all 16K and to hell with it.
    * Turns out we might need all of it.
    */
   i = 0x4000;

   display->Buffer = AllocRemember(&display->BufferKey, i, MEMF_CHIP);
   AllSetPlane     = AllocRemember(&GlobalKey, 80 * CHAR_HEIGHT, MEMF_CHIP);
   AllClearPlane   = AllocRemember(&GlobalKey, 80 * CHAR_HEIGHT, MEMF_CHIP);
   SavePrefs       = (struct Preferences *)AllocRemember(&GlobalKey, 
                     sizeof(struct Preferences), NULL);

   if (display->Buffer && AllSetPlane && AllClearPlane && SavePrefs)
   {
      for (i = 0; i < (CHAR_HEIGHT * BUFFER_WIDTH); i++)
      {
         AllSetPlane[i]   = -1;
         AllClearPlane[i] = 0;
      }
      GetPrefs(SavePrefs, sizeof(struct Preferences));
   } else {
      MyAlert(ALERT_NO_DISP_BUFF_MEMORY, display);
      goto DONE;
   }
/* Bill NOTE: Gee what about freeing BufferKey? */

   if (NOT ReadSettingsFile(display, FALSE))
   {
      TextTwoPlaneColors[0] = SavePrefs->color0;
      TextTwoPlaneColors[1] = SavePrefs->color1;
      TextTwoPlaneColors[2] = SavePrefs->color2;
      TextTwoPlaneColors[3] = SavePrefs->color3;
   }

   /* === Get our special compressed-data font ========================== */
   if (FontData == NULL)
   {
      if ((localFont = OpenDiskFont(&PCFont)) == NULL)
      {
         MyAlert(ALERT_NO_PCFONT, NULL);
         localFont = OpenFont(&SafeFont);
      }
      if ((NormalFont = AllocRemember(&GlobalKey, (256 * 8 * 2),
            MEMF_CHIP | MEMF_CLEAR)) == NULL)
      {
         MyAlert(ALERT_NO_FONT_MEMORY, NULL);
         goto DONE;
      }

      CopyFont(localFont, NormalFont);

      if ((display->Modes & COLOR_DISPLAY) == NULL)
      {
         if ((UnderlineFont = AllocRemember(&GlobalKey, (256 * 8 * 2),
               MEMF_CHIP | MEMF_CLEAR)) == NULL)
         {
            MyAlert(ALERT_NO_UFONT_MEMORY, NULL);
            goto DONE;
         }

         CopyFont(localFont, UnderlineFont);
         Underliner();   /* Go add the underlining to the underline font */
      }

      CloseFont(localFont);
   }

   if (OpenTextClip() == FALSE)
   {
      MyAlert(ALERT_NO_TEXT_CLIP, NULL);
      goto DONE;
   }

   TopBorderOn  = NTSC_TOP_ONHEIGHT;
   TopBorderOff = NTSC_TOP_OFFHEIGHT;

/*BBB   BottomBorderOn = NTSC_BOTTOM_ONHEIGHT;*/
/*BBB   BottomBorderOff = NTSC_BOTTOM_OFFHEIGHT;*/

   ClearFlag(display->Modes, PAL_PRESENCE);

   { /* Bill fix 7-20-89 */
/*
      struct Screen TScreen;

      GetScreenData(&TScreen,sizeof(struct Screen),WBENCHSCREEN,NULL);
      GlobalScreenHeight = TScreen.Height;
#if (DEBUG3 & DBG_INIT_ZAPHOD_SCREEN_HEIGHT)
      kprintf("Init.c:     Global Height = %ld\n",GlobalScreenHeight);
#endif
*/
   }
   GlobalScreenHeight = GfxBase->NormalDisplayRows;
      /* End of bill fix 7-20-89 */

   if (GlobalScreenHeight >= ZAPHOD_HEIGHT + PAL_EXTRAHEIGHT)
   {
      TopBorderOn = PAL_TOP_ONHEIGHT;
      TopBorderOff = PAL_TOP_OFFHEIGHT;
/*BBB      BottomBorderOn = PAL_BOTTOM_ONHEIGHT;*/
/*BBB      BottomBorderOff = PAL_BOTTOM_OFFHEIGHT;*/
      SetFlag(display->Modes, PAL_PRESENCE);
      NewWindow.TopEdge = PAL_TOP_ONHEIGHT;
   }

   retvalue = TRUE;

DONE:

#if (DEBUG2 & DBG_INIT_ZAPHOD_RETURN)
   kprintf("Init.c:       InitZaphod: Returns(%lx)\n",retvalue);
#endif

   return(retvalue);
}



void CopyFont(font, bufptr)
struct TextFont *font;
REGISTER UBYTE *bufptr;
{
   REGISTER SHORT i, i2;
   REGISTER UBYTE *charData, *thisChar;
   REGISTER SHORT height, modulo;
   SHORT firstchar;
   REGISTER WORD *charloc;

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
REGISTER struct Display *display;
REGISTER struct SuperWindow *superwindow;
REGISTER SHORT width;
{
   REGISTER SHORT i, i2;
   REGISTER UBYTE *buffer;

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



