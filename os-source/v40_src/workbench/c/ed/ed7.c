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
/* Command: Ed7.c                                                       */
/* Author:  John A. Toebes, VIII                                        */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 23NOV89  John Toebes   Initial Creation                              */
/* 25MAR90  John Toebes   Added preservation of protection bits         */
/* 16APR90  John Toebes   Corrected insert_block routine                */
/* 30MAY90  John Toebes   Added load_file routine                       */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/
#include "ed.h"
#include "ed_rev.h"

#ifdef KDEBUG
/***
*
* Dump all the memory to the serial port for debugging
*
***/
void dumpall(void)
{
   struct LINE *l;
   int lno = 0, snum;
   char c;
   int len;
   
   snum = -1;
   l = line_first;
   while (l <= line_last)
   {
      len = l->data[0];
      c = l->data[len+1];
      l->data[len+1] = 0;
      if (l == line_ptr[0]) snum = 0;
      
      if (snum > 0)
      {
         snum++;
         kprintf("%2ld %3ld:%08lx: %s\n", snum, lno, l, l->data+1);
         if (snum >= line_num) snum = -1;
      }
      else
      {
         kprintf("%6ld:%08lx: %s\n", lno, l, l->data+1);
      }
      lno++;
      l = NEXTLINE(l);
      l->data[len+1] = c;
   }
}
#endif

/************************************************************************/
/*                                                                      */
/*  Screen editor.                                                      */
/*    Routines to handle various extended commands which                */
/*    cannot be done by existing immediate command routines.            */
/*                                                                      */
/************************************************************************/

/***
*
* Write data from specified lines to file
*
***/
BOOL do_writebuf(line,endl,name,oldfile)
struct LINE *line;
struct LINE *endl;
char *name;
char *oldfile;
{
   BPTR s;
   char buff[LINELENGTH];
   int len;
   long protect;
   BPTR lock;

   if (!*name)
   {
      msg(ERROR_BADFILE);
      return(FALSE);
   }

   /* Select the default protection bits for the file */
   protect = 0;

   lock = Lock(oldfile, SHARED_LOCK);
   if (lock != NULL)
   {
      if (Examine(lock, fib) == DOSTRUE)
      {
         /* Remember what protection bits we had - ignore any ARVHIVE     */
         /* bit that might be set because we are going to change the file */
         protect = fib->fib_Protection & ~FIBF_ARCHIVE;
      }
      UnLock(lock);
   }
   /* Clear the execute bit since this could never be an executable file */
   protect |= FIBF_EXECUTE;

   if ((s = Open(name, MODE_NEWFILE)) == NULL)
   {
      if (IoErr() == ERROR_DISK_FULL)
         mymess("Error - Disk Full");
      else
         msg(ERROR_BADFILE);
      return(FALSE);
   }

   vdu_writes(0,line_max+1,"Writing to file");
   vdu_writes(16,line_max+1,name);
   for(;;)
   {
      /* Copy text to output buffer     */
      len = line->data[0];
      memcpy(buff, line->data+1, len);
      buff[len++] = '\n';
      if (Write(s,buff,len) != len)
      {
         Close(s);
         mymess("Error - Disk Full");
         return(FALSE);
      }
      if (line >= endl) break;
      line = NEXTLINE(line);    /* ptr to next line  */
   }
   Close(s);
   SetProtection(name, protect);

   if (line != endl)
   {
      fatal("Buffer corrupt");
   }
   return(TRUE);
}

/***
*
* Display a message on the status line
*
***/
void mymess(s)
char *s;
{
   VDU_HIGHLIGHTON();
   vdu_writes(0,line_max+1,s);
   VDU_HIGHLIGHTOFF();
}

/***
*
* Join current line with next
*
***/
void do_join()
{
   struct LINE *p;
   struct LINE *n;
   int nl;

   p = line_ptr[log_y];         /* ptr to this line     */
   n = NEXTLINE(p);             /* ptr to next line     */
   nl = n->data[0];             /* length of next line  */
   if (p == line_last) msg(ERROR_ENDFILE);

   if ((current_size + nl) >= MAX_LINE) msg(ERROR_TOOLONG);
   /* add contents of next line to this     */
   memcpy(current_line+current_size+1, n->data+1, nl);
   current_size += nl;
   delete_line(log_y);  /* Delete this line, cursor moves to next line  */
   replace_current(log_y);      /* replace in buffer                    */
   flag_line(log_y);    /* mark to be redisplayed                       */
}

/***
*
* Set screen to display from ptr p
*
***/
void set_pointers(p)
struct LINE *p;
{
   int i, j;

   for(i=0; i <= line_max; i++)
   {  
      line_ptr[i] = p;
      line_mods[i] = 1;
      line_num = i;
      if (p == line_last)       /* reached last line                    */
      {
         for (j = i+1; j <= line_max; j++)      /* clear rest of lines  */
            vdu_writes(0,j,"" );
         if (log_y > i)         /* ensure cursor on text                */
            log_y = i;
         return;
      }
      p = NEXTLINE(p);
   }
}

/***
*
* Set screen to top of file
*
***/
void do_tof()
{
   set_pointers(line_first);
   prev_list = 0;
}

/***
*
* Set screen to N lines on (or bottom of file if == 0)
*
***/
int do_bof(n)
int n;
{
   struct LINE *ltop, *lbot;
   int k;

   ltop = line_ptr[0];          /* Top line                     */
   lbot = line_ptr[line_num];   /* Bottom line                  */
   k = 1;
   while((line_last != lbot) && (k != n))
   {
      ltop->prev = prev_list;
      prev_list = ltop;
      ltop = NEXTLINE(ltop);
      lbot = NEXTLINE(lbot);
      k++;
   }
   set_pointers(ltop);
   return(k);
}

/***
*
* Fill in previous pointers up to p
*
***/
void fill_previous(p)
struct LINE *p;
{
   struct LINE *ltop;

   prev_list = NULL;
   for (ltop = line_first; ltop != p; ltop = NEXTLINE(ltop))
   {
      ltop->prev = prev_list;
      prev_list = ltop;
   }
}

/***
*
* Set screen top to block start
*
***/
void do_showblock()
{
   check_block();
   fill_previous(block_start);
   set_pointers(block_start);
   log_x = 0;
   set_screen_window(0);
}

/***
*
* Check that block is set
*
***/
void check_block()
{
   if ((block_start == NULL) || (block_end == NULL))
      msg(ERROR_NOTSET);
   if (block_start > block_end) msg(ERROR_INVALID);
}

/***
*
* Insert block of text pointed to by pointer SOURCE
* of SIZE words at pointer DEST.
*   LASTPTR is ptr to final inserted line.
*
***/
void insert_block(source, dest, size, lastptr)
struct LINE *source;
struct LINE *dest;
long size;
struct LINE *lastptr;
{
   if ((char *)dest >= line_lastword)
   {
      line_last = (struct LINE *)
                  ((char *)dest+((char *)lastptr-(char *)source));
      line_lastword += size;
   }
   else
   {
      copy_text_up(dest,size);  /* Shuffle the text up                  */
   }
   if (source > dest)           /* if source has been shuffled          */
      source = (struct LINE *)((char *)source + size);  /* modify it    */
   memcpy(dest, source, size);  /* Copy in the buffer                   */

   /* Now get the pointers correct      */
   line_num = log_y;            /* Reset num lines on screen            */
   while((line_num != line_max) && ((char *)dest < line_lastword))
   {
      line_num++;
      line_ptr[line_num] = dest;        /* New pointer, mark to be displayed    */
      line_mods[line_num] = 1;
      dest = NEXTLINE(dest);    /* Next line  */
   }
   log_x = left_margin;
}

/***
*
* Insert block after current line.
*    We must be careful about the case when the insertion takes place
*    before the block, as the block pointers will move under our feet as
*    the text is shuffled in the buffer
*    ?? NB must do a setwindow ??
*
***/
void do_insblock()
{
   struct LINE *p;
   long size;

   p = line_ptr[log_y];
   size = (char *)NEXTLINE(block_end) - (char *)block_start;
   check_block();
   /* Cannot insert block inside itself (too complicated)       */
   if ((block_start <= p) && (p < block_end)) msg(ERROR_INVALID);
   /* Check there is going to be room   */
   if (!room_for(size)) msg(ERROR_NOROOM);
   /* Insert the block  */
   insert_block(block_start, NEXTLINE(p), size, block_end);
   /* Keep block pointers up to date    */
   if (block_start > p)
   {
      block_start = (struct LINE *)((char *)block_start + size);
      block_end   = (struct LINE *)((char *)block_end   + size);
   }
}

/***
*
* Delete block on screen
*
***/
void do_delblock()
{
   struct LINE *bls;
   int c, count;

   do_showblock();              /* move screen to start of block        */
   count = 0;
   for (bls = block_start; bls <= block_end; bls = NEXTLINE(bls))
      count++;

   while(count--)
   {
      c = display(FALSE);
      if ((c == 0x9B) && window_event(vud_rdch(TRUE)))
         resize_window();
      else if (c != C_NOTREADY) msg(ERROR_ABANDON);
      delete_line(0);
   }
}

/***
*
* Copy string s into current_line & buffer, flag for display
*
***/
void copy_current(s)
char *s;
{
   current_size = s[0]-1;
   memcpy(current_line, s+1, current_size+1);
   extracted = TRUE;
   replace_current(log_y);      /* put it back                          */
   flag_line(log_y);            /* and mark for redisplay               */
}

/***
*
* Insert contents of string s after current line
*
***/
void do_append(s)
char *s;
{
   insert_line(log_y);          /* create new line                      */
   copy_current(s);
}

/***
*
* Insert contents of insert_vec before current line
*
***/
void do_insert()
{
   log_x = 0;                   /* cursor to start of this line         */
   set_screen_window(0);
   move_cursor(log_x,log_y);
   do_cr();                     /* create new line above                */
   cursor_up();                 /* move to this new line                */
   copy_current(insert_vec);
}

/***
*
* Insert file given in insert_vec after current line
*
***/
void do_insfile()
{
   BPTR s;

   struct LINE *oll;
   char *olw;
   int err;
   long size;
   struct LINE *base;
   struct LINE *p, *dest;
   int i;

   oll = line_last;
   olw = line_lastword;
   base = NEXTLINE(line_last);

   if ((s = Open(insert_vec+1, MODE_OLDFILE)) == NULL)
      msg(ERROR_BADFILE);

   err = readfile(s,base,FALSE);

   if (err < RDF_FATAL)
   {
      /* File successfully read in to buffer at end of file             */
      /* (possibly with a few tabs expanded or lines truncated)         */
      /* Now simply do an 'insert block'                                */
      block_start = base;
      block_end = line_last;
      size = (char *)NEXTLINE(block_end) - (char *)block_start;
      if (room_for(size))
      {
         p = line_ptr[log_y];
         dest = NEXTLINE(p);
         do_insblock();
         /* Here, we must get all the flags correct                     */
         line_last = (struct LINE *)
                      (((char *)dest >= olw) ? (char *)dest+
                                                ((char *)block_end-
                                                (char *)block_start)
                                          : (char *)oll+size);
         line_lastword = (char *)NEXTLINE(line_last);
         /* Now get the pointers correct                                */
         line_num = log_y;      /* Reset num lines on screen            */
         for (i = line_num+1; i <= line_max; i++)
         {
            if (dest > line_last)
            {
               line_ptr[i] = 0;
               line_mods[i] = 0;
            }
            else
            {
               line_num = i;
               line_ptr[i] = dest;      /* New pointer, mark to be displayed       */
               line_mods[i] = 1;
               dest = NEXTLINE(dest);   /* Next line                    */
            }
         }
      }
      else
      {
         err = RDF_TOOBIG;
      }
   }

   block_start = block_end = 0; /* Ensure block markers invalidated      */

   if (err)
   {
      if (err >= RDF_FATAL) /* 'Fatal' error, so discard anything read in   */
      {
         line_last = oll;
         line_lastword = olw;
      }
      switch(err)
      {
         case RDF_TABS:   mymess("Tabs in file expanded");      break;
         case RDF_TRUNC:  mymess("Input lines truncated");      break;
         case RDF_BIN:    mymess("File contains binary");       break;
         case RDF_TOOBIG: mymess("No room in buffer");          break;
      }
      longjmp(*err_level, ERRLABEL);
   }
}

/***
*
* Load a completely new file into the buffer
*
***/
int load_file(file_name)
char *file_name;
{
   BPTR in_stream;

   empty_buffer();

   in_stream = Open(file_name, MODE_OLDFILE);

   if (in_stream == NULL)       /* No input file, attempt to create new one     */
   {
      /* Only if file not found error    */
      if (IoErr() != ERROR_OBJECT_NOT_FOUND)
      {
         strcpy(status_msg, "\x11" "Invalid file name");
         msg_num = ERROR_STRING;
         return(1);
      }
      msg_num = ERROR_NEWFILE;
   }
   else    /* Read file into buffer     */
   {
      vdu_writes(26,9,"Ed (2.00)");
      move_cursor(0,12);
      vud_flush();
      switch(readfile(in_stream,line_first,TRUE))
      {
         case RDF_BIN:    strcpy(status_msg, "\x14" "File contains binary");
                          msg_num = ERROR_STRING;
                          return(1);
         case RDF_TOOBIG: strcpy(status_msg, "\x0c" "File too big");
                          msg_num = ERROR_STRING;
                          return(1);
         case RDF_TABS:   msg_num = ERROR_TABSEXPANDED;
                          break;
         case RDF_TRUNC:  msg_num = ERROR_LINESTR;
                          break;
      }
   }

   return(0);
}

/***
*
* Locate string s, starting at offset p in text buffer t 
*    Return offset, or -1 if failed
*
***/ 
int locate_string(s,p,t,forwards)
char *s;
int p;
char *t;
BOOL forwards;
{
   int i;
   int lens = s[0];             /* search string length                 */
   int lent = t[0];             /* text length                          */
   int hpos = lent - lens;      /* highest search position in buffer    */
   if ((!forwards) && (p > hpos))
      p = hpos;                 /* First possible start                 */

   for(;;)
   { 
      if (forwards)
      {
         if (p > hpos) return(-1);
      }
      else
      {
         if (p < 0) return(-1);
      }
      for (i = 1; i <= lens; i++)
      {
         if (forcecase)
         {
            if (capitalch(t[i+p]) != capitalch(s[i])) goto nextl;
         }
         else
         {
            if (t[i+p] != s[i]) goto nextl;
         }
      }
      return(p);
nextl:
      if (forwards) p++;
      else          p--;
   }
}


/***
*
* Find the string held in search_vec, starting at cursor + argument n
*
***/
void find_string(n)
int n;
{
   int pos;
   struct LINE *line;
   int p, i;
   int sline;
   BOOL forwards;

   pos = log_x + window_base + n;
   sline = log_y;
   forwards = (n >= 0);         /* Forward search if n positive         */

   /* See if on screen  */
   while((sline <= line_num) && (sline >= 0))
   {
      line = line_ptr[sline];
      p = locate_string(search_vec, pos, line->data, forwards);
      if (p >= 0) break;        /* found it                             */
      if (forwards)
      {
         sline++;
         pos = 0;
      }
      else
      {
         sline--;
         pos = MAX_LINE;
      }
   }

   if (p < 0)
   {
      /* not found on screen       */
      sline = 0;  /* searched for line will appear at top of screen     */
      if (!forwards)
         line->prev = prev_list;   /* Set up for backwards search          */
      while (p < 0)
      {
         if ((line == line_last) || (line == line_first))
            msg(ERROR_NOTFOUND);
         if (forwards)
            line = NEXTLINE(line);
         else
            line = line->prev;
         p = locate_string(search_vec, pos, line->data, forwards);
      }
      fill_previous(line);      /* fill previous ptrs to here           */
      set_pointers(line);       /* set up display                       */
   }
   log_y = sline;               /* place cursor on correct line         */
   log_x = 0;                   /* place cursor under first character   */
   set_screen_window(0);
   for (i=0; i < p; i++)
      cursor_right();           /* cursor under found character         */
}

/***
*
* Exchange search_vec for insert_vec
*    Query user if flag TRUE
*
***/
void do_exchange(query)
BOOL query;
{
   int slen, ilen;
   int c;
   int i;

   slen = search_vec[0];
   ilen = insert_vec[0];

   /* find occurrence, starting at current position             */
   find_string(0);
   extract_current(log_y);      /* fish out new current line    */
   extracted = TRUE;
   if (query)
   {
      for(;;)
      {
         VDU_HIGHLIGHTON();
         vdu_writes(0, line_max+1, "Exchange? ");
         VDU_HIGHLIGHTOFF();
         c = display(TRUE);
         if ((c == 0x9B) && window_event(vud_rdch(TRUE)))
         {
            resize_window();
            continue;
         }
         break;
      }
      vdu_writes(0, line_max+1, "");    /* clear line                   */
      c = capitalch(c);
      if (c == 'N')
      {
         /* No change                           */
         /* place cursor after string           */
         for (i = 0; i < slen; i++) cursor_right();
         return;
      }
      if (c != 'Y') msg(ERROR_ABANDON); /* Abandon if anything else     */
   }

   extend_margin = TRUE;
   for (i = 0; i < slen; i++)
   {
      move_cursor(log_x,log_y);
      do_delch();
   }
   for (i = 1; i <= ilen; i++)
   {
      move_cursor(log_x,log_y);
      do_ins(insert_vec[i]);
   }
   replace_current(log_y);  /* place back in buffer for redisplay        */
   extend_margin = FALSE;
}
/***
*
* Display part line of marked block
*
***/
void display_part(line,p,s,v)
int line;
struct LINE *p;
char *s;
char v[];
{ 
   char *text = "Unset";
   int len;

   vdu_writes(0,line,s);        /* Write name                           */
   if (p != NULL)
   {
      len = p->data[0];         /* Length of text                       */
      if (len > 19)             /* Line longer than can be displayed    */
         len = 19;
      memcpy(v, p->data+1, len);
      v[len] = 0;
      text = v;
   }
   vdu_writes(15,line,text);
}

/***
*
* Display state of program
*
***/
void do_show()
{
   char v[40];
   long opts[1];

   VDU_CLEAR();

   VDU_HIGHLIGHTON();
   vdu_writes(  0, 0, VSTRING " updated by John A. Toebes, VIII");
   VDU_HIGHLIGHTOFF();

   vdu_writes(  0, 2, "Editing file");
   vdu_writes( 15, 2, file_name);

   opts[0] = tab_stop;
   vsprintf(v, "Tab distance   %ld", opts);
   vdu_writes(  0, 3, v);

   opts[0] = left_margin+1;
   vsprintf(v, "Left margin    %ld", opts);
   vdu_writes(  0, 4, v);

   opts[0] = right_margin+1;
   vsprintf(v, "Right margin   %ld", opts);
   vdu_writes(  0, 5, v);

   /* Display block start & end */
   display_part(6, block_start, "Block start",v);
   display_part(7, block_end,   "Block end",  v);

   opts[0] = ((line_lastword-(char *)line_first)*100)/
                   (worktop-(char *)line_first);
   vsprintf(v, "Buffer usage   %ld%%", opts);
   vdu_writes(  0, 8, v);

   opts[0] = worktop-(char *)line_first;
   vsprintf(v, "Buffer size    %ld Bytes", opts);
   vdu_writes(  0, 9, v);

   /* Wait until character typed        */

   VDU_HIGHLIGHTON();
   vdu_writes(  0, 11, "Type any character to continue ");
   VDU_HIGHLIGHTOFF();

   /* The window could be resized here  */
   if ((vud_rdch(TRUE) == 0x9B) && (window_event(vud_rdch(TRUE))))
   {
      resize_window();
   }
   else
   {
      /* Restore screen       */
      do_verify();
   }
}
