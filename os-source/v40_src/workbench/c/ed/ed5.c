/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 481-6436    */
/* | o  | ||   John Toebes     John Mainwaring    Jim Cooper                 */
/* |  . |//    Bruce Drake     Gordon Keener      Dave Baker                 */
/* ======      Doug Walker                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**     (C) Copyright 1989 Commodore-Amiga, Inc.
 **        All Rights Reserved
**/

/*----------------------------------------------------------------------*/
/* Command: Ed5.c                                                       */
/* Author:  John A. Toebes, VIII                                        */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 23NOV89  John Toebes   Initial Creation                              */
/* 24MAR90  John Toebes   Rewrote scroll routines                       */
/* 30MAY90  John Toebes   Added fpinit/fpterm                           */
/* 21OCT90  John Toebes   Corrected crashing on long lines              */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/
#include "ed.h"

/************************************************************************/
/*                                                                      */
/*  Screen editor.                                                      */
/*  Start routine and editing procedures for immediate commands.        */
/*                                                                      */
/************************************************************************/

int __stdargs _main()
{
   LEVELBUF level;
   init_ed();

   setjmp(level);
   err_level = &level;
   edit();
   return(0);
}

void __stdargs MemCleanup() {}
void __stdargs __fpinit() {}
void __stdargs __fpterm() {}
/***
*
* Move cursor left.
* Return TRUE if possible
*
***/
BOOL cursor_left()
{
   if (log_x == 0)      /* at left hand end of screen                   */
   {
      if (window_base == 0) return(FALSE);      /* Can't scroll any more*/
      log_x = 9;        /* Place cursor under char                      */
      set_screen_window( window_base-10 );      /* Reset window         */
   }
   else
   {
      log_x--;
   }
   return(TRUE);
}

/***
*
* Move cursor right
*
***/
void cursor_right()
{
   if (log_x >= screen_width)   /* cursor in last position on screen    */
   {
      log_x = screen_width-9;
      set_screen_window( window_base+10 );
   }
   else
     log_x++;
}

/***
*
* Move cursor up
*
***/
void cursor_up()
{
   if (log_y == 0)              /* try to scroll backwards              */
   {
      if (!prev_line()) msg(ERROR_TOPFILE);
   }
   else
      log_y--;
}

/***
*
* Move cursor down
*
***/
void cursor_down()
{
   if (log_y == line_num)       /* try to scroll forwards               */
   {
      if (!next_line()) msg(ERROR_ENDFILE);
   }
   else
      log_y++;
}

/***
*
* Move cursor to next tab stop
*
***/
void do_tab()
{
   do {
      cursor_right();
   } while (((log_x + window_base) % tab_stop) != 0);
}

/***
*
* Return pointer to next place in text, possibly moving to next line
*
***/
int check_next()
{
   cursor_right();
   if ((log_x+window_base) > current_size)      /* end of this line     */
   {
      do_replace();
      cursor_down();                            /* to next line         */
      log_x = 0;                                /* start of it          */
      set_screen_window(0);
      extracted = TRUE;
      extract_current(log_y);
   }
   return(log_x+window_base);
}

/***
*
* Move cursor to start of next word
*
***/
void cursor_nxw()
{
   int pos = log_x + window_base;
   while (current_line[pos] != ' ')
   {
      pos = check_next();
      if (pos == 0) break; /* start of line counts as start of word     */
   }
   while ((current_size < 0) || (current_line[pos] == ' '))
      pos = check_next();
}

/***
*
* Return pointer to previous place in text, possibly moving to prev line
*
***/
int check_prev()
{
   if (!cursor_left())
   {
      do_replace();
      cursor_up();              /* Move up */
      do_extract();
      cursor_eol();      /* To end, because at start of line before      */
   }
   return(log_x + window_base);
}

/***
*
* Move cursor to space beyond the end of the previous word
*
***/
void cursor_prw()
{
   int pos = check_prev();
   while (pos > (current_size+1))
      pos = check_prev();       /* off right edge */
   while ((pos != (current_size+1)) && (current_line[pos] != ' '))
      pos = check_prev();
   while ((pos == 0) || (current_line[pos-1] == ' ')) pos = check_prev();
}

/***
*
* Remove trailing spaces in current line
*
***/
void remove_trailing_spaces()
{
   while((current_line[current_size] == ' ') && (current_size >= 0))
      current_size--;
}

/***
*
* Perform carriage return (split line)
*
***/
void do_cr()
{
   int real_length = current_size;      /* Size of entire line          */
   int pos = current_size+1;    /* Possibly split here at end of line   */
   int chposn = log_x+window_base;      /* Ptr to char                  */
   int lm = left_margin;                /* Amount to shuffle up         */

   if (current_size >= chposn)          /* Line must actually be split  */
   {
      current_size = chposn-1;          /* Amount left on previous line */
      VDU_DEOL();                       /* ensure correct on screen     */
      pos = chposn;                     /* split is here                */
   }
   remove_trailing_spaces();
   replace_current(log_y);              /* put that much back into buffer*/
   insert_line(log_y);                  /* possibly change log_y, log_x */
   current_size = real_length-pos;      /* length of new line           */
   if (current_size < 0)                /* nothing more to do in common case */
      return;
   if (pos < left_margin) lm = pos;
   if (lm)
      memset(current_line, ' ', lm-1);
   /* shuffle data down        */
   memcpy(current_line+lm, current_line+pos, current_size+1);
   current_size += lm;
   replace_current(log_y);      /* replace in main buffer for DISPLAY   */
   line_mods[log_y] = 1;                /* flag as needing displaying   */
}

/***
*
* Delete character at cursor, and any spaces which are now trailing
*
***/
void do_delch()
{
   int p = log_x+window_base;   /* offset of character in current line  */

   if (p <= current_size)
   {
      /* Internal structure up to date     */
      memcpy(current_line+p, current_line+p+1, current_size-p);
      current_size--;
#if 0
      /* This is never true on the AMIGA as we have a delete character  */
      /* screen ops.  For now we can leave the code here.               */
      if (!VDU_DELCH())       /* and attempt on screen        */
      {
         /* Another stupid VDU  */
         int chtop = (current_size > screen_width+window_base) 
                                              ? screen_width+window_base
                                              : current_size;
         int len = chtop - p + 1;
         VDU_DEOL();
         vud_wrline(current_line+p,len);
         phys_x += len;
         move_cursor(log_x,log_y);
      }
#else
      VDU_DELCH();            /* and attempt on screen        */
#endif
      /* Display char if any that was off RHS of screen                 */
      if ((current_size-window_base) >= screen_width)
      {
         move_cursor(screen_width,log_y);
         vud_wrch(current_line[screen_width+window_base]);
         phys_x = screen_width+1;
         move_cursor(log_x,log_y);
      }
      /* This seems very strange to be done here.  I believe we can get */
      /* A tiny amount of performance by removing this.                 */
      remove_trailing_spaces();
   }
}

/***
*
* Delete word. Delete all spaces if at space, else delete until space
*
***/
void do_delw()
{
   int pos = log_x + window_base;
   if (current_line[pos] == ' ')        /* Delete all spaces            */
   {
      while ((current_size >= pos) && (current_line[pos] == ' '))
         do_delch();    /* delete char at cursor until condition satisfied      */
   }
   else
   {
      while ((current_size >= pos) && (current_line[pos] != ' '))
         do_delch();
   }
}

/***
*
* Insert character at cursor
*
***/
void do_ins(ch)
int ch;
{
   int i;
   int p = log_x + window_base; /* offset of cursor in current line     */
   int newsize = current_size + 1;
   if ((p >= MAX_LINE) || (newsize >= MAX_LINE))   /* line too long     */
   {
      msg(ERROR_TOOLONG);
      return;
   }

   /* Automatic margin wrap, only when at end of line                   */
   if ((newsize == right_margin) && (p==newsize) && (!extend_margin))
   {
      int pos = current_size;
      if (ch == ' ')    /* inserting a space - just do a CR             */
      {
         do_cr();
         return;
      }
      while ((current_line[pos] != ' ') && (pos >= 0))
      {
         cursor_left(); /* move to after first space                    */
         pos--;
      }
      move_cursor(log_x,log_y); /* place cursor there                   */
      do_cr();          /* split line (replaces current in buffer)      */
      if (current_size < 0)     /* generated line is empty              */
      {
         memset(current_line, ' ', left_margin);
         current_size = left_margin;
      }
      else
      {
         cursor_eol();          /* move to end of new line              */
         current_size++;        /* line is longer                       */
      }
      current_line[current_size] = ch;  /* with this at end             */
      replace_current(log_y);   /* replace in store so will be displayed*/
      flag_line(log_y);         /* and ensure it is displayed           */
      cursor_right();
      return;
   }
   if (p > newsize)             /* pad line with spaces if beyond end   */
   {
      memset(current_line+newsize, ' ', p-newsize);
      current_size = p;
   }
   else
   {
      /* In the middle, or at the very end (normal case)                */
      /* This is a performance point for inserting characters.          */
      /* We need to optimize this with an ASM subroutine                */
      for (i = current_size; i >= p; i--)
         current_line[i+1] = current_line[i];   /* correct in store data structure      */
      current_size = newsize;
   }
   current_line[p] = ch;
   cursor_right();      /* Update log_x and maybe scroll                */
   /* get correct on screen - cursor is at correct place                */
   if (p < newsize)     /* only if required                             */
   {
#if 0
      if (!VDU_INSCH())              /* See if it can be done*/
      {
         /* Stupid VDU - must do it by steam                            */
         int chtop = (current_size > screen_width+window_base)
                                               ? screen_width+window_base
                                               : current_size;
         int len = chtop - p;
         move_cursor(log_x-1,log_y);    /* Get the cursor to the right place    */
         VDU_DEOL();                    /* Cancel rest of line          */
         move_cursor(log_x,log_y);      /* to the character after the new one   */
         vud_wrline(current_line+p+1,len);
         phys_x += len;
         move_cursor( log_x-1, log_y ); /* Put cursor back              */
      }
#else
      VDU_INSCH();
#endif
   }
   vud_wrch(ch);
   phys_x++;
}

/***
*
* Delete from cursor to end of line
*
***/
void do_deol()
{
   int p = log_x + window_base;
   if (p <= current_size)
   {
      current_size = p-1;               /* Ignore rest of text          */
      VDU_DEOL();                       /* Clear on screen              */
      remove_trailing_spaces();
   }
}

/***
*
* Scroll text back n lines
*
***/
void do_prevline(n)
int n;
{
   struct LINE *p;
   int count;
   int i;

   /* First figure out how many lines we can really move back */
   if (line_ptr[0] == line_first)
   {
      msg(ERROR_TOPFILE);
      return;
   }
   else
   {
      p = prev_list;
      n--;
      count = 1;
      while((p != line_first) && (n-- > 0))
      {
         p = p->prev;
         count++;
      }
   }

   /* Now we can do a single multi-line delete */
   if (count > 3)
   {
      /* If we insert more than 3 lines there is no need to eliminate   */
      /* the message line as it will get scrolled off in the process    */
      vud_delete_line(line_max);        /* Clear the message line       */
   }
   VDU_SCROLLDOWN(count);

   if (line_num != line_max)    /* screen not full yet          */
   {
      line_num += count;
      if (line_num > line_max) line_num = line_max;
   }

   prev_list = p->prev;
   for (i = 0; i <= line_num; i++)
   {
      line_ptr[i] = p;
      p = NEXTLINE(p);
      if (i + count <= line_num)
         line_mods[i+count] = line_mods[i];
      if (i < count)
         line_mods[i] = 1;
   }
}

/***
*
* Scroll text forward n lines
*
***/
void do_nextline(n)
int n;
{
   struct LINE *p;
   int count;
   int i;

   /* First figure out how many lines we can really move back */
   p = line_ptr[line_num];
   count = 0;
   while((p != line_last) && (n-- > 0))
   {
      p = NEXTLINE(p);
      count++;
   }

   if (count == 0)
   {
      msg(ERROR_ENDFILE);
      return;
   }

   /* Now we can do a single multi-line delete */
   VDU_SCROLLUP(count);

   /* Determine the first of the hidden lines to be scrolled onto the   */
   /* screen.  Normally this will be the next line after the last line  */
   /* but in the case that we are scrolling more lines than are on the  */
   /* screen currently, this will simply skip over the discarded lines  */
   p = line_ptr[0];
   for (i = 0; i < count; i++)
   {
      p->prev = prev_list;
      prev_list = p;
      p = NEXTLINE(p);
   }

   /* p should now point at the first line to put on the screen */

   /* Compute the screen update information.  For those lines that were */
   /* scrolled, compute their new position in the array, for new lines  */
   /* set up the pointers and mark them for redisplay                   */
   for (i=0; i <= line_num; i++)
   {
      line_ptr[i] = p;
      p = NEXTLINE(p);
      
      if (i <= (line_num-count))
      {
         line_mods[i] = line_mods[i+count];
      }
      else
      {
         line_mods[i] = 1;
      }
   }
}
