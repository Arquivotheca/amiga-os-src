/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 481-6436    */
/* | o  | ||   John Toebes     John Mainwaring    Jim Cooper                 */
/* |  . |//    Bruce Drake     Gordon Keener      Dave Baker                 */
/* ======      Doug Walker                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**     (C) Copyright 1989 Commodore-Amiga, Inc.
 **         All Rights Reserved
**/

/*----------------------------------------------------------------------*/
/* Command: Ed3.c                                                       */
/* Author:  John A. Toebes, VIII                                        */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 23NOV89  John Toebes   Initial Creation                              */
/* 27MAR90  John Toebes   Allowed fatal to be called in any situation   */
/* 16APR90  John Toebes   Corrected copy_text_up routine                */
/* 14OCT90  John Toebes   Moved fatal() to ed4.c                        */
/* 30OCT90  John Toebes   Added ERROR_NO_MEMORY                         */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/
#include "ed.h"

/************************************************************************/
/*                                                                      */
/* Screen editor support routines                                       */
/*                                                                      */
/************************************************************************/

/***
*
* Copy text up store from p by n words. Write in assembler if possible
*
**/
void copy_text_up(p,n)
struct LINE *p;
long n;
{
   long *source;
   long *dest;

   source = (long *)line_lastword;
   dest   = (long *)(((char *)source) + n);
   while(source >= (long *)p)
      *dest-- = *source--;
   /* For some reason this next test fails to catch the cases we are    */
   /* interested in.  However after careful consideration, you always   */
   /* want to update both the line_last and line_lastword pointers      */
   /* whenever you insert something.  I leave the statement in commented*/
   /* out for histerical purposes                                       */
   /***if (line_last >= p)***/   /* must update this ptr        */
      line_last = (struct LINE *)((char *)line_last+n);
   line_lastword += n;
}

/***
*
* Copy text down in store onto p by n words.
*   Again should be in assembler
*
**/
void copy_text_down(p,n)
struct LINE *p;
long n;
{
   char *newlast = line_lastword-n;

   if (p < line_last)
      line_last = (struct LINE *)((char *)line_last-n);
   memcpy((char *)p, (char *)p+n, newlast-(char *)p);
   line_lastword = newlast;
}

/***
*
* Check to see if enough space for N words left in buffer.
*    Worktop is set in initialisation code to allow a safety area
*    so there is no need to check when inserting lines
*
***/
BOOL room_for(n)
long n;
{
   if ((line_lastword + n) >= worktop)
      return(FALSE);
   return(TRUE);
}

/***
*
* Move cursor to position x, y
*
**/
void move_cursor(x,y)
int x;
int y;
{
   if ((x != phys_x) || (y != phys_y))
      VDU_SETCURSOR(x, y);

   phys_x = x;          phys_y = y;
}

/***
*
* Delete line n on vdu
*
**/
void vud_delete_line(n)
int n;
{
   if (n == 0)
      VDU_SCROLLUP(1);
   else
   {
      move_cursor( 0, n );
      VDU_DELL();
   }
}

/***
*
* Insert line n on vdu
*
**/
void vud_insert_line(n)
int n;
{
   if (n == 0)
      VDU_SCROLLDOWN(1);
   else
   {
      move_cursor( 0, n );
      VDU_INSL();
   }
}

/***
*
* Display routine. For every line which has been altered, re-display it
*   While doing this check to see if any characters have been typed in;
*   if so stop the update to the display and return the character
*   Once complete return the next character typed if wait is true.
*   Always leave the cursor at log_x, log_y
*
***/
int display(wait)
BOOL wait;
{
   int i;
   int ch;

   /* display lines, starting at current   */
   for (i = log_y; i <= line_num; i++)
   {
      ch = display_line(i);
      if (ch != C_NOTREADY) return(ch);
   }

   /* and now do the rest  */
   for (i = log_y-1; i >= 0; i--)
   {
      ch = display_line(i);
      if (ch != C_NOTREADY) return(ch);
   }

   /* Return character if one waiting   */
   if (!wait) return(VDU_GETCHAR(FALSE));

   /* Display message line      */
   if (msg_num != 0) display_msg();

   /* Display complete. Wait for character      */
   move_cursor( log_x, log_y ); /* Show cursor where user thinks it is  */
   ch = VDU_GETCHAR(TRUE);     /* Wait for character           */
   if (msg_num != 0)
   {
      vdu_writes(0,line_max+1,"");      /* Clear message line           */
      msg_num = 0;
   }
   return(ch);
}

/***
*
* Display line on screen, and return character typed if any
*
**/
int display_line(i)
int i;
{
   struct LINE *line;
   int l;
   int ch;

   /* If the line does not need to be displayed, skip it                */
   if (!line_mods[i]) return(C_NOTREADY);

   ch = VDU_GETCHAR(FALSE);
   if (ch != C_NOTREADY) return(ch);

   line = line_ptr[i];
   line_mods[i] = 0;                    /* mark as displayed now        */

   l = line->data[0]-window_base;     /* length of text to be displayed */
   if (l > (screen_width+1))            /* only display what we can fit */
      l = screen_width+1;
   move_cursor(0,i);                    /* move cursor to line          */
   VDU_DEOL();       /* delete what was there before         */
   vud_wrline(line->data+window_base+1,l);
   if (l >= 0)
      phys_x = l;                       /* cursor at end of line now    */
   return(VDU_GETCHAR(FALSE));  /* and collect any char typed while that happened       */
}


/***
*
* Remember message, and abort processing if required
*
**/
void msg(n)
int n;
{
   msg_num = n;
   cmdptr = 1;                  /* reset extended command pointer       */
   if (err_level != 0) longjmp(*err_level, ERRLABEL);
}

/***
*
* Display message
*
**/
void display_msg()
{
   VDU_HIGHLIGHTON();
   vdu_writes(0, line_max+1, get_msg());
   VDU_HIGHLIGHTOFF();
}

/***
*
* Get the string for the current pending error
*
***/
char *get_msg()
{
   switch(msg_num)
   {
      case 0:                  return("");
      case ERROR_DELLAST:      return("Last line deleted");
      case ERROR_NOROOM:       return("No room in buffer");
      case ERROR_NEWFILE:      return("Creating new file");
      case ERROR_LINESTR:      return("Input lines truncated");
      case ERROR_TOPFILE:      return("Top of file");
      case ERROR_ENDFILE:      return("End of file");
      case ERROR_TOOLONG:      return("Line too long");
      case ERROR_UNKNOWN:      return("Unknown command");
      case ERROR_BRACKET:      return("Unmatched ();");
      case ERROR_ABANDON:      return("Commands abandoned");
      case ERROR_SYNTAX:       return("Syntax error");
      case ERROR_BADFILE:      return("Unable to open file");
      case ERROR_NUMBER:       return("Number expected");
      case ERROR_NOTSET:       return("No block marked");
      case ERROR_INVALID:      return("Cursor inside block");
      case ERROR_BLKERR:       return("Block incorrectly specified");
      case ERROR_NOTFOUND:     return("Search failed");
      case ERROR_TABSEXPANDED: return("Tabs in input file expanded");
      case ERROR_NO_REXX:      return("Rexx not available");
      case ERROR_NO_MEMORY:    return("Out of memory for operation");
      case ERROR_STRING:       return(status_msg+1);
   }
   return("Unknown internal error");
}
