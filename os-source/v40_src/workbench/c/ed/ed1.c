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
/* Command: Ed1.c                                                       */
/* Author:  John A. Toebes, VIII                                        */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 23NOV89  John Toebes   Initial Creation                              */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/
#include "ed.h"

/************************************************************************/
/*                                                                      */
/*  Screen editor.                                                      */
/*                                                                      */
/* Storage management.                                                  */
/*                                                                      */
/* Text is held as a contiguous set of lines. Each line is a BCPL       */
/* string - ie it starts on a word boundary and has the length as the   */
/* first byte.  The newline at the end of lines is not stored. In       */
/* addition each line is preceded by a pointer to the previous line if  */
/* it has passed the screen window. The head of this previous list is   */
/* held in prev_list.                                                   */
/*                                                                      */
/* -------------------------                                            */
/* | ptr (word) | String ...                                            */
/* -------------------------                                            */
/*                                                                      */
/* When text is being altered the current line is copied into the       */
/* vector current_line. When alteration is finished it is copied back,  */
/* and the stored text is shuffled as required. Note that this also     */
/* allows an 'undo' command, and that there is no need to update the    */
/* previous pointers when shuffling.                                    */
/*                                                                      */
/* The lines currently on the screen are pointed at by the vector       */
/* line_ptr.  This pointer is negative if the the line has yet to be    */
/* displayed on the screen, and is positive if the screen version is    */
/* correct. The pointers are to the previous pointer field of the       */
/* stored line, so the text is at (ptr+1).                              */
/*                                                                      */
/* The globals line_first and line_last point to the first and last     */
/* lines of the file.  line_num is the index of the last line on the    */
/* screen. line_max is the maximum value of line_num. line_lastword     */
/* points at the last word of the buffer.                               */
/*                                                                      */
/************************************************************************/

/***
*
* Shuffle lines up starting at line n
*
***/
void shuffle_up(n)
int n;
{
   int i;
   for(i = n; i < line_num; i++)
   {
      line_ptr[i] = line_ptr[i+1];      /* Shuffle pointers             */
      line_mods[i] = line_mods[i+1];
   }
   vud_delete_line(n);                  /* Remove line from screen      */
}

/***
*
* Bring a new bottom line onto screen area
*
***/
/* Passed pointer to bottom line        */
void get_next(p)
struct LINE *p;
{
   line_ptr[line_num] = NEXTLINE(p);
   line_mods[line_num] = 1;
}

/***
*
* Save top line from screen area
*
***/
void store_prev()
{
   struct LINE *p;

   p = line_ptr[0];          /* ptr to top line      */
   p->prev = prev_list;
   prev_list = p;       /* add to chain         */
}

/***
*
* Shuffle lines down starting at line n
*
***/
void shuffle_down(n)
int n;
{
   int i;
   if (line_num != line_max)    /* screen not full yet          */
      line_num++;
   for (i=line_num; i > n; i--)
   {
      line_ptr[i] = line_ptr[i-1];
      line_mods[i] = line_mods[i-1];
   }
   vud_insert_line(n);          /* Add blank line on screen     */
}

/***
*
* Move to next line in file.
*    Update data structure representing lines on the screen.
*    Return TRUE if possible.
*
***/
BOOL next_line()
{  struct LINE *p;

   p = line_ptr[line_num];   /* pointer to last line                         */
   if (p == line_last) return(FALSE);
   store_prev();        /* Save top line                                */
   shuffle_up(0);       /* Shuffle up from line 0 & alter display       */
   get_next(p);         /* get next line                                */
   return(TRUE);
}

/***
*
* Move to previous line in file if possible.
*    return TRUE if worked
*
***/
BOOL prev_line()
{
   struct LINE *p;

   if (line_ptr[0] == line_first) return(FALSE);
   p = prev_list;
   prev_list = p->prev;
   shuffle_down(0);     /* Move text ptrs down & update screen  */
   line_ptr[0] = p;     /* Install new line flagged as changed   */
   line_mods[0] = 1;
   vud_delete_line(line_max+1); /* Clear message line   */
   return(TRUE);
}

/***
*
* Delete all lines in the file
*
***/
void empty_buffer()
{
   log_y = line_num = 0;

   line_first->data[0] = 0;
   line_lastword = (char *)NEXTLINE(line_first); /* create empty line        */
   line_lastword[0] = 0;
   line_last = line_first;
   do_tof();
   block_start = block_end = 0;
}

/***
*
* Delete line n
*
***/
void delete_line(n)
int n;
{
   struct LINE *p;              /* pointer to this line                 */
   int l;                       /* length of line store                 */
   struct LINE *lp;             /* ptr to last line on screen           */

   p = line_ptr[n];
   l = LINELEN(p->data[0]);
   lp = line_ptr[line_num];

   block_start = block_end = 0; /* Reset block markers                  */
 
   /*--- A number of special cases here.   ---*/
   if (lp == line_last)
   {
      /* last line on screen                                            */
      if (n==line_num)
      {
         /* deleting last line                   */
         line_lastword -= l;    /* no need to shuffle text      */
         if (n==0)
         {
            /* last line is at top of screen       */
            line_last = prev_list; /* attempt to fetch previous line      */
            if (line_last == NULL)
            { 
               /* no more, start of file     */
               line_lastword = (char *)NEXTLINE(line_first); /* create empty line        */
               line_lastword[0] = 0;
               line_last = line_first;
               line_ptr[0] = line_last;  /* display it   */
               line_mods[0] = 1;
               line_first->data[0] = 0;
               msg(ERROR_DELLAST);
            }
            /* Previous line available - display it     */
            prev_list = line_last->prev;
            line_ptr[0] = line_last;  /* install & flag for display      */
            line_mods[0] = 1;
            return;
         }
         /* Last line not at top of screen      */
         line_last = line_ptr[n-1];  /* keep line_last correct       */
         vdu_writes(0,n,"");      /* clear deleted line  */
      }
      else
      {
         /* last line on screen, but not being deleted       */
         delete_and_shuffle(n,p,l);  /* do the delete     */
         vud_delete_line(line_max);  /* clear (scrolled) message line    */
      }
      /* Now reduce screen size & move cursor if required       */
      line_num--;
      if (log_y > line_num)
         log_y = line_num;
      return;
   }
   /* Normal case. Delete line & pull in next one at bottom     */
   delete_and_shuffle(n,p,l);
   if (n == line_num)   /* deleted last line, only need to flag new one     */
      flag_line(n);
   else
      /* display next line, passing ptr to last line  */
      get_next((struct LINE *)((char *)lp-l));
}

/***
*
* Delete line & shuffle text (used by delete)
*
***/
void delete_and_shuffle(n,p,l)
int n;
struct LINE *p;
int l;
{
   int i;
   vud_delete_line(n);   /* delete on screen      */
   copy_text_down(p,l);  /* copy rest of text over deleted line  */
   /* correct pointers after text moved     */
   for(i = n+1; i < line_num; i++)
   {
      line_ptr[i] = (struct LINE *)((char *)line_ptr[i+1] - l);
      line_mods[i] = line_mods[i+1];
   }
}

/***
*
* Correct line pointers from n to m by a
*
***/
void adjust_pointers(n,m,a)
int n;
int m;
long a;
{
   int i;
   for(i = n; i <= m; i++)
   {
      line_ptr[i] = (struct LINE *)((char *)line_ptr[i] + a);
   }
}

/***
*
* Insert a new blank line after line n.
*   Screen is scrolled up if full, otherwise line is inserted on screen.
*
***/
void insert_line(n)
int n;
{
   int i;
   struct LINE *p = line_ptr[n];        /* pointer to line              */
   struct LINE *nextp = NEXTLINE(p);    /* pointer to start of next line*/
   block_start = block_end = 0;         /* reset block markers          */
   set_screen_window(0);                /* back to extreme lhs          */
   if (line_num == line_max)            /* Screen full                  */
   {
      VDU_SCROLLUP(1);                  /* Scroll text on screen up     */
      vud_insert_line(n);  /* Insert new line before line n+1 (as scrolled) */
      store_prev();                     /* save old previous line       */
      for (i = 0; i < n; i++)
         line_ptr[i] = line_ptr[i+1];   /* shuffle ptrs                 */
   }
   else
   {
      n++;
      log_y++;                          /* Cursor moved down as well    */
      shuffle_down(n);                  /* Move text down screen        */
   }

   /* Now add empty line and reset pointers     */
   if (p == line_last)                  /* adding after last line       */
   {
      line_last = nextp;
      line_lastword += LINELEN(0);
   }
   else
   {
      copy_text_up(nextp,LINELEN(0));/* move text in buffer    */
   }
   nextp->data[0] = 0;                 /* empty line                   */
   line_ptr[n] = nextp;         /* set line vector to point to new line */
   adjust_pointers(n+1,line_num,LINELEN(0));  /* reset lower ptrs        */
   log_x = left_margin;         /* reset cursor to left margin   */
}

/***
*
* Extract line n and place into vector current_line
*
***/
void extract_current(n)
int n;
{
   struct LINE *p = line_ptr[n];     /* pointer to data                      */
   current_size = p->data[0]-1; /* highest offset in current_line        */
   /* copy into vector      */
   memcpy(current_line, p->data+1, current_size+1);
}

/***
*
* Replace line n by contents of vector current_line
*
***/
void replace_current(n)
int n;
{
   struct LINE *p = line_ptr[n];        /* pointer to old data          */
   int oldlen = LINELEN(p->data[0]);    /* old length                   */
   int newlen = LINELEN(current_size+1);/* length of current line */
   int dif = (newlen - oldlen);         /* difference                   */

   extend_margin = FALSE;               /* no margin extension now      */
   if (dif != 0)                        /* don't copy unless we have to */
   {
      if (dif > 0)                      /* new version bigger           */
      {
         if (!room_for(dif))            /* see if room in buffer        */
         {
            flag_line(n);               /* no - redisplay previous contents      */
            msg(ERROR_NOROOM);
         }
         if (p == line_last)            /* no copy required             */
            line_lastword += dif;
         else                   /* copy from next line on up to make room       */
            copy_text_up((struct LINE *)((char *)p+oldlen),dif);
      }
      else                              /* new version smaller          */
         copy_text_down((struct LINE *)((char *)p+newlen),-dif);
      /* Correct pointers       */
      adjust_pointers(n+1,line_num,dif);
      block_start = block_end = 0;      /* reset block markers          */
   }
   /* copy data back in    */
   memcpy(p->data+1, current_line, current_size+1);
   p->data[0] = current_size+1;         /* number of bytes              */
}

/***
*
* Mark line n as to be redisplayed
*
***/
void flag_line(n)
int n;
{
   line_mods[n] = 1;
}
