
/**** alert.c **************************************************************
 * 
 * Alert and Abort Routines --   Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 11 Apr 86   =RJ Mical=  Created this file
 *
 **************************************************************************/

#include "zaphod.h"
#include "pcwindow_rev.h"
#include <intuition/intuitionbase.h>
#include <string.h>

extern struct IntuitionBase *IntuitionBase;
VOID ErrorReq(struct Window *window,UBYTE *title,UBYTE *text);

#define EXTRA_TOP   6
#define EXTRA_LEFT  6

static UBYTE *AlertStrings[] =
    {
/*  (UBYTE *)"|--- Maximum string length ------>|", */
    (UBYTE *)"System failure, unable to continue.",
    (UBYTE *)"No SYS:PC/System/SideCarKeys.table",
    (UBYTE *)"Task Signals Not Available",
    (UBYTE *)"Incomplete System",
    (UBYTE *)"Not Enough Memory",
    (UBYTE *)"No SYS:Epansion/Janus.Library",
    (UBYTE *)"Couldn't Make Region",
    (UBYTE *)"Couldn't Create Port",
    (UBYTE *)"Couldn't Find FONTS:pcfont",
    (UBYTE *)"Couldn't Create Timer",
    (UBYTE *)"Lock List Corrupt",
    (UBYTE *)"Can't Make Sidecar Signals",
    (UBYTE *)"No SYS:PC/System/SidecarSettings.table",
    (UBYTE *)"Please see the HELP page about COPY",
    (UBYTE *)"You've already opened that display!",
    (UBYTE *)"No SYS:PC/System/ScanCode.table",
    (UBYTE *)"Can't hide frozen window's border",
    (UBYTE *)"Can't change frozen window's text",
    (UBYTE *)"RJ was here",
    (UBYTE *)"Copr.(c) 1985,89,92 Commodore-Amiga",
    /*(UBYTE *)VERS,*/
    (UBYTE *)"Warning: old SidecarKeys.Table fi",

    (UBYTE *)"No memory for Display struc",
    (UBYTE *)"Could not open Graphics.library",
    (UBYTE *)"Could not open Layers.library",
    (UBYTE *)"Could not open DiskFont.library",
    (UBYTE *)"Could not open Janus.library",
    (UBYTE *)"You've already opened that display!",
    (UBYTE *)"No memory for Display Buffer",
    (UBYTE *)"No memory for Normal Font",
    (UBYTE *)"No memory for Underline Font",
    (UBYTE *)"No Text Clip",
    (UBYTE *)"No memory for SuperWindow Struc",
    (UBYTE *)"No memory for Color Window",
    (UBYTE *)"No Task Stack memory",
    (UBYTE *)"No PCDisplay memory",
    (UBYTE *)"No ExtraScreen memory",
    (UBYTE *)"No Window memory",
    (UBYTE *)"No Extension Window memory",
    (UBYTE *)"",
    };

static struct IntuiText AlertText =
   {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      AUTOLEFTEDGE + EXTRA_LEFT, AUTOTOPEDGE + EXTRA_TOP +12,
      &SafeFont,
      NULL, NULL,
   };
static struct IntuiText AlertTitleText =
   {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      AUTOLEFTEDGE + EXTRA_LEFT, AUTOTOPEDGE + EXTRA_TOP,
      &SafeFont,
      "PCWindow:",
	  &AlertText,
   };

/****i* PCWindow/MyAlert ******************************************
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

VOID MyAlert(USHORT abortNumber,struct Display *display)
{
   struct Window *window;

   window = NULL;

   if (display)
   {
      if (display->FirstWindow)
         window = display->FirstWindow->DisplayWindow;
   }
   ErrorReq(window,VERS,AlertStrings[abortNumber]);
}

static struct TextAttr MySafeFont =
   {
      (UBYTE *)"topaz.font",
      TOPAZ_EIGHTY,      /* equals 8 */
      0,
      FPB_ROMFONT,
   };

static struct IntuiText MyOKText =
   {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      AUTOLEFTEDGE, AUTOTOPEDGE,
      &MySafeFont,
      (UBYTE *)"Cancel", 
      NULL,
   };

static struct IntuiText ERText[]= {
      {
	     AUTOFRONTPEN,   /* FrontPen */
		 AUTOBACKPEN,    /* BackPen */
		 AUTODRAWMODE,   /* DrawMode */
		 AUTOLEFTEDGE,   /* LeftEdge */
		 AUTOTOPEDGE+2L,    /* TopEdge */
		 &MySafeFont,      /* *ItextFont */
		 "0123456789012345678901234567890123456789012345678901234567890",
		 NULL,           /* *NextText */
	  },
	  {
	     AUTOFRONTPEN,   /* FrontPen */
		 AUTOBACKPEN,    /* BackPen */
		 AUTODRAWMODE,   /* DrawMode */
		 AUTOLEFTEDGE,   /* LeftEdge */
		 AUTOTOPEDGE+10+2L,    /* TopEdge */
		 &MySafeFont,      /* *ItextFont */
		 NULL,           /* *IText */
		 NULL,           /* *NextText */
	  }
   };

   /***************/
   /* 2.0 Support */
   /***************/
static struct EasyStruct ES = {
       sizeof (struct EasyStruct),
       0,
       NULL,
       NULL,
       "Cancel",
   };

/****i* Support/ErrorReq ******************************************
*
*   NAME   
*      ErrorReq -- Display an error requester.
*
*   SYNOPSIS
*      ErrorReq(window,title,text)
*
*      VOID ErrorReq(struct Window *window,UBYTE *title,UBYTE *text);
*
*   FUNCTION
*      Puts up an error requester under V1.3 or V2.0. Uses EasyReq()
*      under V2.0 to auto adjust to system font sizes.
*
*   INPUTS
*      window	- Pointer to a Window or NULL.
*	   title	- Pointer to NULL terminated title text, usually version
*                 string.
*      text     - Pointer to NULL terminated error text. 
*                 Under 1.3 requester is 320 pixels wide and uses
*                 topaz 8 so text must be an appropriate length.
*
*   RESULT
*      Puts up the requester and waits for the user to cancle it.
*
*   EXAMPLE
*
*      if(!(MyBase=OpenLibrary("my.library",0)))
*      {
*         ErrorReq(NULL,VERS,"Could not open my.library");
*         exit(20);
*      }
*
*   NOTES
*      Intuition.library must be open!
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

VOID ErrorReq(struct Window *window,UBYTE *title,UBYTE *text)
{
   LONG width,height;

   if(IntuitionBase->LibNode.lib_Version>=36)
   { 
	  /*kprintf("2.0\n");*/
	  ES.es_Title=title;
	  ES.es_TextFormat=text;
      EasyRequestArgs(window,&ES,0,0);
   } else {
	  /*kprintf("1.3\n");*/
      if(title)
      {
         ERText[0].IText = title;
		 height=AUTOTOPEDGE+10L+35L;
		 width=strlen(title)*8+AUTOLEFTEDGE+27L;
      }

      if(text)
      {
         ERText[1].IText=text;
         ERText[0].NextText=&ERText[1];
		 height=AUTOTOPEDGE+20L+35L;
		 if(strlen(text)>strlen(title))
		 {
		    width=strlen(text)*8+AUTOLEFTEDGE+27L;
		 }
      } else {
         ERText[0].NextText=NULL;
      }

      /*MyOKText.TopEdge=height-25L;*/
      AutoRequest(window,&ERText[0],NULL,&MyOKText,0L,0L,width,height);
   }
}
