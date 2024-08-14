
/* *** main.c ***************************************************************
 * 
 * Main Routine.
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY    Name           Description
 * -------    ------         --------------------------------------------
 * 21 jul 89   -BK         Added code to open intuition library BEFORE
 *                         the first possible call to MyAlert since
 *                         MyAlert/AutoRequest tends to blow the fuck up
 *                         otherwise. If only RJ had a brain!
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 27 Nov 86  Dieter Preiss  Added Startup code, CLI/WB parameter check
 * 23 Feb 86  =RJ Mical=     Created this file
 *
 * *************************************************************************/
#define LINT_ARGS
#define PRAGMAS

#include "zaphod.h"
#include <exec/memory.h>
#include <janus/janusbase.h>
#include <janus/jfuncs.h>
#include <graphics/gfxbase.h>
#include <stdlib.h>
#include <string.h>

#define IS_PAL (((struct GfxBase *)GfxBase)->DisplayFlags & PAL)

extern struct Library *DiskfontBase;
extern struct Library *DOSBase;

#define D(x) ;

void main(argc, argv)
int argc;
char **argv;
{
   char     *arg;
   struct   WBArg    *WBArg;  
   struct   Display  *display;

   struct   TextFont *localFont;
   SHORT    i;
   BOOL     retvalue;
   BOOL pal;


   /******************/
   /* Open Intuition */
   /******************/
   if((IntuitionBase=(struct IntuitionBase *)
       OpenLibrary((UBYTE *)"intuition.library",0))==NULL)
   {
      return;
   }

   /******************************/
   /* Allocate Display structure */
   /******************************/
   if ((display = (struct Display *)AllocMem(sizeof(struct Display),
         MEMF_CHIP | MEMF_CLEAR)) == NULL)
   {
      MyAlert(ALERT_NO_DISPLAY_STRUC_MEMORY, NULL);
      return;
   }

   DisplayPriority = ((struct Task *)FindTask(0))->tc_Node.ln_Pri;


   /**********************/
   /* Get CLI or WB args */
   /**********************/
   if (argc)
   {
      /* Called from CLI, look for arguments */
      arg = (argc > 1) ? argv[1] : argv[0];
   } else {
      /* Called from Workbench, find the name of last icon passed
         since this is the one that called us. */
      WBArg = ((struct WBStartup *)argv)->sm_ArgList;
      WBArg += ((((struct WBStartup *)argv)->sm_NumArgs)-1);
      arg   =  WBArg->wa_Name;
   }

   /* Convert arg string to lower case */
   /*tolower(arg);*/

   /* Look for "color" in arg string */
   while (stricmp(arg,"color") && *arg) arg++;

   /* If arg not exhausted, we found the magic word.
      So we do a color show now. */
   if (*arg) display->Modes |= (OPEN_SCREEN | COLOR_DISPLAY);

   /*****************************/
   /* Start this shooting match */
   /*****************************/
/*###############################################################*/

   retvalue = FALSE;

   /***********************/
   /* Put up Wait pointer */
   /***********************/
   MakeWaitWindow();

   /*************************/
   /* Open Graphics library */
   /*************************/
   if (GfxBase == NULL)
   {
      if((GfxBase=(struct GfxBase *)OpenLibrary(
        (UBYTE *)"graphics.library", 0))==NULL)
      {
         MyAlert(ALERT_NO_GRAPHICS_LIB, NULL);
         goto DONE;
      }
   }

   /***********************/
   /* Open Layers library */
   /***********************/
   if (LayersBase == NULL)        
   {
      if((LayersBase=(struct LayersBase *)OpenLibrary(
        (UBYTE *)"layers.library",0))==NULL)
      {
         MyAlert(ALERT_NO_LAYERS_LIB, NULL);
         goto DONE;
      }
   }

   /*************************/
   /* Open DiskFont library */
   /*************************/
   if (DiskfontBase == NULL)
   {
      if((DiskfontBase=(struct Library *)OpenLibrary(
        (UBYTE *)"diskfont.library", 0))==NULL)
      {
         MyAlert(ALERT_NO_DISKFONT_LIB, NULL);
         goto DONE;
      }
   }

   /**********************/
   /* Open Janus library */
   /**********************/
   if (JanusBase == NULL)
   {
      if((JanusBase=(struct JanusBase *)OpenLibrary(
        (UBYTE *)"janus.library", 0))==NULL)
      {
         MyAlert(ALERT_NO_JANUS_LIB, NULL);
         goto DONE;
      }
   }

   /* These two lines take advantage of the fact that we know that 
    * the JanusAmiga structure resides at the first memory location 
    * of Janus Parameter Memory.  We feel it's safe to presume this 
    * because it's a supposedly unchangeable presumption used by 
    * the original Janus designers (both Amiga and PC sides) to 
    * locate JanusAmiga, as witnessed by the fact that JanusBase 
    * has no explicit pointer to JanusAmiga and the PC code presumes 
    * everywhere that JanusAmiga starts at location zero of janus mem.
    */
   JanusAmigaBA=(struct JanusAmiga *)MakeBytePtr(
                (APTR)JanusBase->jb_ParamMem);
   JanusAmigaWA=(struct JanusAmiga *)MakeWordPtr((APTR)JanusAmigaBA);

   MainTask = FindTask(0);

   /*CurrentSettings=LastReadSettings=DefaultSettings;*/

   /************************/
   /* Read keyboard tables */
   /************************/
   /*ReadKeyTable();*/
   /*ReadScanCodeTable();*/
   ReadKeymap();

   /*****************/
   /* Open AuxTools */
   /*****************/
   OpenAuxTools(display);

   /*******************************/
   /* Duplicate display rewuester */
   /*******************************/
   if (DuplicateDisplay(display)) 
   {
      MyAlert(ALERT_ALREADY_OPEN, NULL);
      goto DONE;
   }

   /**********************************/
   /* Allocate display buffer memory */
   /**********************************/
   /* "If we're in text mode, then allocate only enough memory for one
    * page of text.  If we're in graphics mode, we'll need the whole 16K."
    * Nah, just grab all 16K and to hell with it.
    * Turns out we might need all of it.
    */
   i = 0x4000;

   display->Buffer = AllocRemember(&display->BufferKey, i, MEMF_CHIP);
   AllSetPlane     = AllocRemember(&GlobalKey, 80 * CHAR_HEIGHT, MEMF_CHIP);
   AllClearPlane   = AllocRemember(&GlobalKey, 80 * CHAR_HEIGHT, MEMF_CHIP);
   /*SavePrefs       = (struct Preferences *)AllocRemember(&GlobalKey, 
                     sizeof(struct Preferences), NULL);*/

   if (display->Buffer && AllSetPlane && AllClearPlane/* && SavePrefs*/)
   {
      for (i = 0; i < (CHAR_HEIGHT * BUFFER_WIDTH); i++)
      {
         AllSetPlane[i]   = -1;
         AllClearPlane[i] = 0;
      }
      /*GetPrefs(SavePrefs, sizeof(struct Preferences));*/
   } else {
      MyAlert(ALERT_NO_DISP_BUFF_MEMORY, display);
      goto DONE;
   }
   /* Bill NOTE: Gee what about freeing BufferKey? */

   /**********************/
   /* Read settings file */
   /**********************/
   /****************/
   /* Set Defaults */
   /****************/
#if 0
   /*
     Settings.TextOnePlaneColors[2];
     Settings.TextTwoPlaneColors[4];
     Settings.TextThreePlaneColors[8];
     Settings.TextFourPlaneColors[16];
     Settings.HighGraphicsColors[2];
     Settings.LowGraphicsColors[2][2][4];
   */

   CurrentSettings.PresetMonoWidth       = 640L;
   CurrentSettings.PresetMonoHeight      = 200L;
   CurrentSettings.PresetMonoTopEdge     = 0L;
   CurrentSettings.PresetMonoLeftEdge    = 0L;
   CurrentSettings.PresetMonoDepth       = 1L;
   CurrentSettings.PresetMonoFlags       = 0L;

   CurrentSettings.PresetColorWidth      = 640L;
   CurrentSettings.PresetColorHeight     = 200L;
   CurrentSettings.PresetColorTopEdge    = 0L;
   CurrentSettings.PresetColorLeftEdge   = 0L;
   CurrentSettings.DefaultColorTextDepth=4L;
   CurrentSettings.PresetColorFlags      = 0L;

   CurrentSettings.CursorSeconds=DEFAULT_CURSOR_SECONDS;
   CurrentSettings.CursorMicros=DEFAULT_CURSOR_MICROS;

   CurrentSettings.DisplayPriority=0L;
#endif

   if (NOT ReadSettingsFile(display, FALSE))
   {
      CurrentSettings = LastSavedSettings = DefaultSettings;


#if 0
      CurrentSettings.DefaultColorTextDepth=4L;

      CurrentSettings.PresetMonoWidth       = 640L;
      CurrentSettings.PresetMonoHeight      = 200L;
      CurrentSettings.PresetMonoTopEdge     = 0L;
      CurrentSettings.PresetMonoLeftEdge    = 0L;
      CurrentSettings.PresetMonoDepth       = 1L;


/*
      TextTwoPlaneColors[0] = SavePrefs->color0;
      TextTwoPlaneColors[1] = SavePrefs->color1;
      TextTwoPlaneColors[2] = SavePrefs->color2;
      TextTwoPlaneColors[3] = SavePrefs->color3;
*/

      CurrentSettings.PresetColorWidth      = 640L;
      CurrentSettings.PresetColorHeight     = 200L;
      CurrentSettings.PresetColorTopEdge    = 0L;
      CurrentSettings.PresetColorLeftEdge   = 0L;

      CurrentSettings.PresetMonoFlags       = 0L;
      CurrentSettings.PresetColorFlags      = 0L;

      CurrentSettings.CursorSeconds=DEFAULT_CURSOR_SECONDS;
      CurrentSettings.CursorMicros=DEFAULT_CURSOR_MICROS;

      CurrentSettings.DisplayPriority=0L;
#endif
   } else {
      LastSavedSettings = CurrentSettings;
   }
   /************/
   /* Get font */
   /************/
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

   /******************/
   /* Open clipboard */
   /******************/
   if (OpenTextClip() == FALSE)
   {
      MyAlert(ALERT_NO_TEXT_CLIP, NULL);
      goto DONE;
   }

   TopBorderOn  = NTSC_TOP_ONHEIGHT;
   TopBorderOff = NTSC_TOP_OFFHEIGHT;

   ClearFlag(display->Modes, PAL_PRESENCE);

   /*GlobalScreenHeight = GfxBase->NormalDisplayRows;*/
      /* End of bill fix 7-20-89 */


   /* check for PAL */
   pal = IS_PAL;
   specialpal=pal;


   retvalue = TRUE;

DONE:


/*###############################################################*/
      UnmakeWaitWindow();
   if (retvalue)
   {
      DisplayTask(display);
   } else {
      CloseEverything();
      FreeMem(display, sizeof(struct Display));
   }

   /* Actually, we should never return from DisplayTask() */
   exit(0);
}


/*
 * Some Aux. routines
 */

/*
 * Check for String S2 to occur in String S1
 */
 
#if 0
BOOL fndstr (s1,s2)
UBYTE *s1,*s2;
/* by Dieter ... */
{

   while (*s1 && *s2 && ( *s1 == *s2))
   {
      s1++;
      s2++;
   }
   /* return TRUE only if at end of s2 */
   return((*s2)==0?TRUE:FALSE);
}      
void tolower(s)
UBYTE *s;
/* by Dieter ... */
{

   do
      if ((*s >= 'A') && (*s <= 'Z')) 
	     *s += 0x20;
   while (*s++);
}
#endif
