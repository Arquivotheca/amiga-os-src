/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 481-6436    */
/* | o  | ||   John Toebes     John Mainwaring    Jim Cooper                 */
/* |  . |//    Bruce Drake     Gordon Keener      Dave Baker                 */
/* ======      Doug Walker                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*----------------------------------------------------------------------*/
/* Command: Ed2.c                                                       */
/* Author:  John A. Toebes, VIII                                        */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 23NOV89  John Toebes   Initial Creation                              */
/* 12MAR90  John Toebes   Added arbitrary control keys                  */
/* 01APR90  John Toebes   Added menu support                            */
/* 18APR90  John Toebes   Fixed phantom file is changed bug             */
/* 30MAY90  John Toebes   Changed close window to use push_command      */
/* 29DEC90  John Toebes   Corrected enforcer hit on large windows       */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/
#include "ed.h"
#include "pragmas/intuition_pragmas.h"

/************************************************************************/
/*                                                                      */
/* Screen editor. Immediate command decoding.                           */
/*                                                                      */
/* Certain commands require the current line to be extracted from the   */
/* line store.  Others cause the line to be replaced.                   */
/*                                                                      */
/************************************************************************/

void edit()
{
   int ch;
   long args[8];
   struct MenuItem *item;

   for(;;)
   {
      ch = display(TRUE);               /* Update screen & return char */

      move_cursor(log_x,log_y);

      /* In the SWITCH, break; means data has changed, continue no change  */

      switch(ch)
      {
      case 0x9B:                 /* Escape leadin       */
         ch = parse_ansi(display(TRUE), args);  /* Get character        */
         switch (ch)
         {
            case '@':  execute_pfkey(PFKEY_RIGHT);      break;                            break;
            case 'A': 
                     if (args[0])
                        execute_pfkey(PFKEY_LEFT);
                     else
                     { do_replace();  cursor_up(); }    break;
            case 'B':  do_replace();  cursor_down();    break;
            case 'D':                 cursor_left();    break;
            case 'C':                 cursor_right();   break;
            case 'T':  execute_pfkey(PFKEY_UP);         break;
            case 'S':  execute_pfkey(PFKEY_DOWN);       break;
            case '~':  if ((args[0] >= 0) && (args[0] < MAX_PFKEY))
                          execute_pfkey(args[0]);
                       else
                          msg(ERROR_INVALID);           break;
            case '|':  switch(args[0])
                       {
                          case 2:    /* Mouse event */
                             if ((args[3] & IEQUALIFIER_LEFTBUTTON) &&
                                 (window != NULL))
                             {
                                do_replace();
                                log_x = (window->MouseX-window->BorderLeft)/
                                         window->RPort->Font->tf_XSize;
                                log_y = (window->MouseY-window->BorderTop)/
                                         window->RPort->Font->tf_YSize;
                                if (log_y > line_num) log_y = line_num;
                             }
                             break;
                          case 10: /* Menu Event */
                             /* args[2] is the menu number */
                             while(menu != NULL && args[2] != MENUNULL)
                             {
                                item = ItemAddress(menu, args[2]);
                                push_command(((char *)(MENU_USERDATA(item))) + 1,
                                             ((char *)(MENU_USERDATA(item)))[0]);
                                args[2] = item->NextSelect;
                             }
                             break;
                          case 11:  /* Close window */
                             /* For now we make it seem as if they typed */
                             /* 'XQ'                                     */
                             push_command("XQ",2);
                             break;
                          case 12: /* Resize Window */
                             do_replace();
                             resize_window();
                             break;
                          default:
                             msg(ERROR_UNKNOWN);
                             break;
                       }
                       break;
            default: 
               msg(ERROR_UNKNOWN);
         }
         continue;
      case C_RUBOUT:
        execute_pfkey(PFKEY_DEL);
        break;
      case C_AREXX:
         continue;
      default:
         if ((ch < ' ') || ((ch >= 0x7F) && (ch < 0xA0)))
         {
            /* Map the alted control keys to the same as the control keys */
            execute_pfkey(PFKEY_CTRL2+(ch & 0x1F));
         }
         else
         {
            do_extract();
            do_ins(ch);
            data_changed = TRUE;
         }
      }
   }
}

/***
*
* Having got a '0x9B' check to see if this is a message from
* the console.device telling us the window has been resized.
* Check the sequence and return TRUE if it's a resize.
*
***/

BOOL window_event(ch)
int ch;
{
   long args[8];

   ch = parse_ansi(ch, args);

   if (ch != '|' ||
       args[0] != 12)
      return(FALSE);

   return(TRUE);
}

/***
*
* Parse an incomming ANSI escape sequence
*
***/
int parse_ansi(ch, args)
int ch;
long args[8];
{
   int arg;

   arg = 0;
   memset(args, 0, 8*sizeof(long));

   for(;;)
   {
      if ((ch < ' ') || (ch > '?'))
         return(ch);

      if (ch >= '0' && ch <= '9')
      {
         args[arg] *= 10;
         args[arg] += ch - '0';
      }
      else
      {
         if (ch == '?')
            args[arg] = -1;
         else if (ch == ' ')
            { if (args[arg] == 0) args[arg] = -2; }
         else if (ch != ';')
            args[arg] = ch;            
         if (arg < 8) arg++;
      }
      ch = vud_rdch(TRUE);
   }
}

/***
*
* Get new window parameters & redraw window
*
***/
void resize_window()
{
   line_max     = VDU_LENGTH() - 2;
   if (line_max >= MAXLINES) line_max = MAXLINES-1;
   screen_width = VDU_WIDTH() - 1;
   if (line_num > line_max)     line_num = line_max;
   if (log_y    > line_max)     log_y    = line_max;
   if (log_x    > screen_width) log_x = screen_width;
   /* Reset display to be from current first line       */
   set_pointers( line_ptr[0] );
   do_replace();
   do_verify();
}

/***
*
* Replace current line in store if it is extracted
*
***/
void do_replace()
{
   if (extracted)
   {
      extracted = FALSE;
      replace_current(log_y);
   }
}

/***
*
* Extract current line unless already
*
***/
void do_extract()
{
   if (extracted != TRUE)
   {
      extracted = TRUE;
      extract_current(log_y);
   }
}

/***
*
* Perform verify
*
***/
void do_verify()
{
   int i;

   VDU_CLEAR();
   phys_x = phys_y = 0;
   VDU_SETCURSOR(0, 0);
   for (i = 0; i <= line_num; i++)
      flag_line(i);
}

void do_invertcase()
{
   int p = log_x + window_base;
   if (p <= current_size)
   {
      int c = current_line[p];
      if ((c >= 'A') && (c <= 'Z')) c += 'a'-'A';
      else if ((c >= 'a') && (c <= 'z')) c = capitalch(c);
      current_line[p] = c;
      vud_wrch(c);
      phys_x++;
   }
   cursor_right();
}

/***
*
* Move cursor to end(s) of line
*
***/
void cursor_eol()
{
   int eol  = current_size+1;
   int base = window_base;

   if (log_x + base == eol)     /* already there       */
   {
      log_x = 0;
      base = 0;
   }
   else
   {
      for (;;)
      {
         /* Move to end of line, and scroll horizontal if required    */
         log_x = eol-base;
         if (log_x < 0)         /* end of line on left of screen       */
         {
            base -= 10;
            continue;
         }
         if (log_x <= screen_width) break;      /* end of line on screen */
         base += 10;    /* end of line on right of screen       */
      }
   }
   set_screen_window(base);
}

/***
*
* Move cursor to top or bottom of page
*
***/
void cursor_eop()
{
   if ((log_x != 0) || (log_y != 0))  /* move to top       */
   {
      log_x = log_y = 0;
      set_screen_window(0);
      return;
   }
   /* Move to end of last line, and scroll horizontal if required       */
   extract_current(line_num);/* Make last line on screen current        */
   extracted = TRUE;
   log_y = line_num;
   log_x = 0;    /* so that cursor_eol will work  */
   cursor_eol(); /* move to end of line, and possibly scroll    */
}

/***
*
* Handle horizontal scrolling.
*   This resets the window base and shifts screen if required.
*   Could be made clever for a memory mapped vud.
*
***/
void set_screen_window(col)
int col;
{
   if (window_base != col)
   {
      window_base = col;
      /* flag for redraw before poss error in replace_current       */
      do_verify();
      if (extracted)    /* put back current line  */
         replace_current(log_y);
   }
}
