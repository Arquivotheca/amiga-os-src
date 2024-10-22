/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .	| || the authors:			      BBS: (919) 481-6436    */
/* | o	| ||   John Toebes     John Mainwaring	  Jim Cooper		     */
/* |  . |//    Bruce Drake     Gordon Keener	  Dave Baker		     */
/* ======      Doug Walker						     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*-----------------------------------------------------------------------*/
/* Command: Ed8.c							 */
/* Author:  John A. Toebes, VIII					 */
/* Change History:							 */
/*  Date    Person	  Action					 */
/* -------  ------------- -----------------				 */
/* 23NOV89  John Toebes   Initial Creation				 */
/* Notes:								 */
/*-----------------------------------------------------------------------*/
#include "ed.h"

/************************************************************************/


/* Implementation of VDU stuff on Amiga */

void screenops(code)
int code;
{
   vud_wrline("\x1B[1", 3);
   vud_wrch(code);
}

void screenopv(code, val)
int code;
int val;
{
   char buf[10];
   long args[2];
   int len;

   args[0] = val;
   args[1] = code;

   len = vsprintf(buf, "\x1B[%ld%lc", args);
   vud_wrline(buf, len);
}

void vdu_setcursor(x, y)
int x, y;
{
   char buf[20];
   long args[2];
   int len;

   args[0] = y+1;
   args[1] = x+1;

   len = vsprintf(buf, "\x1B[%ld;%ldH", args);
   vud_wrline(buf, len);
}

int make_window_enquiry(width)
BOOL width;
{
   /* Send the console device 'ESC[ q' to make the enquiry.             */
   /* The answer will be returned in the read stream as follows 	*/
   /* ESC[1;1;20;60 r  (top,left,bottom and right margins)              */
   /* Currently it always returns 1;1 as the first 2 params as		*/
   /* this is the upper left home position.				*/

   int c;
   long args[8];	/* space for the parms				*/

   if (width)
   {
      if (term_width)  return(term_width);
   }
   else
   {
      if (term_height) return(term_height);
   }

   /* send out request for info 					*/
   vud_wrline("\x9b q", 3);

   /* It's just possible that a new mouse event will occur in the       */
   /* middle of us sending the above string to the console device.	*/
   /* We cannot guarantee that the first thing the console device	*/
   /* returns to us is the new window size.				*/

   do {
      while(vud_rdch(TRUE) != 0x9B);    /* Wait for ESC                 */

      c = parse_ansi(vud_rdch(TRUE), args);

      /* Right, now we have collected the info. If 'c' now              */
      /* holds the character 'r' then the buffer contains useful info.  */
      /* If not, It's info about another mouse event so ignore and      */
      /* wait for the info we requested initially.			*/

   } while (c != 'r');

   if (width)
      return(args[3] - args[1] + 1);
   return(args[2] - args[0] + 1);
}

/***
*
* This is a tiny implementation of sprintf for use within the editor
*  It should elminate most needs to format numbers
*  Note that it assumes that the buffer is large enough to hold any
*  formatted string
*
***/
#pragma syscall RawDoFmt 20a ba9804
extern void __builtin_emit(UWORD);

static void __regargs prbuf(char c);
/* void RawDoFmt(char *, long[], void(*)(), char *); */

int vsprintf(buf, ctl, args)
char *buf;
char *ctl;
long args[];
{
   RawDoFmt(ctl, args, prbuf, buf);
   return(strlen(buf));
}

/***
*
* The following stub routine is called from RawDoFmt for each
* character in the string.  At invocation, we have:
*    D0 - next character to be formatted
*    A3 - pointer to data buffer
*
***/
static void __regargs prbuf(char c)
{
__builtin_emit(0x16c0);   /* move.b D0,(A3)+ */
}
