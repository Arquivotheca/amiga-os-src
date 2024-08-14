
/**** ErrorReq.c **********************************************************
 * 
 * 1.3/2.0 compatible Error Requester
 *
 * Copyright (C) 1991, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ----------  -------------------------------------------------
 *  6 Nov 91   B. Koester   Created this file
 *
 **************************************************************************/

#if 0
#include <exec/types.h>
#include <exec/libraries.h>
#include <pragmas/intuition_pragmas.h>
#include <proto/intuition.h>
#endif
#include <intuition/intuitionbase.h>
#include <string.h>

static struct TextAttr SafeFont =
   {
      (UBYTE *)"topaz.font",
      TOPAZ_EIGHTY,      /* equals 8 */
      0,
      FPB_ROMFONT,
   };

static struct IntuiText OKText =
   {
      AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE,
      AUTOLEFTEDGE, AUTOTOPEDGE,
      &SafeFont,
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
		 &SafeFont,      /* *ItextFont */
		 "0123456789012345678901234567890123456789012345678901234567890",
		 NULL,           /* *NextText */
	  },
	  {
	     AUTOFRONTPEN,   /* FrontPen */
		 AUTOBACKPEN,    /* BackPen */
		 AUTODRAWMODE,   /* DrawMode */
		 AUTOLEFTEDGE,   /* LeftEdge */
		 AUTOTOPEDGE+10L+2L,    /* TopEdge */
		 &SafeFont,      /* *ItextFont */
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
extern struct IntuitionBase *IntuitionBase;

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

      /*AutoRequest(window,&ERText[0],NULL,&OKText,0,0,320,150);*/
      AutoRequest(window,&ERText[0],NULL,&OKText,0L,0L,width,height);
   }
}
