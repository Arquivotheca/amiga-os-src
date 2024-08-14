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
/* Command: Ed6.c                                                       */
/* Author:  John A. Toebes, VIII                                        */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 23NOV89  John Toebes   Initial Creation                              */
/* 09MAR90  John Toebes   Added REXX support                            */
/* 01APR90  John Toebes   Added SI, EM commands                         */
/* 06APR90  John Toebes   Added execute_series support                  */
/* 06APR90  John Toebes   Added ^? parsing to SF/DF commands            */
/* 10APR90  John Toebes   Made E/EQ arguments optional                  */
/* 10APR90  John Toebes   Added split prompting for E/EQ commands       */
/* 11APR90  John Toebes   Added OP, NW commands                         */
/* 16APR90  John Toebes   Added CT command                              */
/* 30APR90  John Toebes   Corrected OP command, added Prompt support    */
/*                        Fixed SA command, added title to requester    */
/* 30OCT90  John Toebes   Corrected pushelv to handle out of memory     */
/* 01NOV90  John Toebes   Eliminated Unnecessary local SysBases         */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/
#include "ed.h"

/************************************************************************/
/*                                                                      */
/*  Screen editor - extended commands. Any extended command may be      */
/*  interrupted by typing any character while it is happening           */
/*                                                                      */
/************************************************************************/

/* Manifests for double letter commands */
#define X_BE (('B' << 8) | 'E')
#define X_BF (('B' << 8) | 'F')
#define X_BS (('B' << 8) | 'S')
#define X_CE (('C' << 8) | 'E')
#define X_CL (('C' << 8) | 'L')
#define X_CM (('C' << 8) | 'M')
#define X_CR (('C' << 8) | 'R')
#define X_CS (('C' << 8) | 'S')
#define X_CT (('C' << 8) | 'T')
#define X_DB (('D' << 8) | 'B')
#define X_DC (('D' << 8) | 'C')
#define X_DF (('D' << 8) | 'F')
#define X_DL (('D' << 8) | 'L')
#define X_DO (('D' << 8) | 'O')
#define X_DW (('D' << 8) | 'W')
#define X_EL (('E' << 8) | 'L')
#define X_EM (('E' << 8) | 'M')
#define X_EP (('E' << 8) | 'P')
#define X_EQ (('E' << 8) | 'Q')
#define X_EX (('E' << 8) | 'X')
#define X_FC (('F' << 8) | 'C')
#define X_FR (('F' << 8) | 'R')
#define X_IB (('I' << 8) | 'B')
#define X_IF (('I' << 8) | 'F')
#define X_LC (('L' << 8) | 'C')
#define X_MB (('M' << 8) | 'B')
#define X_NW (('N' << 8) | 'W')
#define X_OP (('O' << 8) | 'P')
#define X_PD (('P' << 8) | 'D')
#define X_PU (('P' << 8) | 'U')
#define X_RE (('R' << 8) | 'E')
#define X_RF (('R' << 8) | 'F')
#define X_RK (('R' << 8) | 'K')
#define X_RP (('R' << 8) | 'P')
#define X_RX (('R' << 8) | 'X')
#define X_RV (('R' << 8) | 'V')
#define X_SA (('S' << 8) | 'A')
#define X_SB (('S' << 8) | 'B')
#define X_SF (('S' << 8) | 'F')
#define X_SH (('S' << 8) | 'H')
#define X_SI (('S' << 8) | 'I')
#define X_SL (('S' << 8) | 'L')
#define X_SM (('S' << 8) | 'M')
#define X_SR (('S' << 8) | 'R')
#define X_ST (('S' << 8) | 'T')
#define X_TB (('T' << 8) | 'B')
#define X_UC (('U' << 8) | 'C')
#define X_VW (('V' << 8) | 'W')
#define X_WB (('W' << 8) | 'B')
#define X_WN (('W' << 8) | 'N')
#define X_WP (('W' << 8) | 'P')
#define X_XQ (('X' << 8) | 'Q')

/***
*
* Read in a response to any prompt
*
***/
int do_prompt(prompt, buf)
char *prompt;
char *buf;
{
   int c;
   int save_x, save_y;
   int len = strlen(prompt);

   save_x = log_x;
   save_y = log_y;
   vdu_writes(0,line_max+1,prompt);  /* Write prompt on message line */
   log_y = phys_y;
   
   /* Loop to read command line */
   for(;;)
   {
      log_x = phys_x;
      c = display(TRUE);
      move_cursor(log_x,log_y);
      if (c == 0x9B)
      {
         if (window_event(vud_rdch(TRUE)))   /* Redraw command line  */
         {
            resize_window();
            vdu_writes(0,line_max+1,prompt);
            vud_wrline(buf+1,log_x-1);
            phys_x = log_x;
            log_y  = phys_y;
         }
         continue;   /* Ignore all cursor keys etc                   */
      }
      if (c == '\b')
      {
         if (phys_x == len) continue;  /* Line empty                 */
         VDU_LEFT();
         vud_wrch( ' ' );
         VDU_LEFT();      /* back again                   */
         phys_x--;
         continue;
      }
      if ((c == C_CR) || (c == C_ESC)) break;/* finished             */
      if (phys_x >= screen_width-5 ||/* ignore overlong lines        */
         (c < ' '))                  /* and control codes!           */
      {
         vud_wrch( 0x07 );           /* Bell                         */
         continue;
      }
      vud_wrch( c );                 /* reflect char                 */
      buf[1+(phys_x++)-len] = c;
   }

   /* restore cursor position    */
   log_x = save_x;
   log_y = save_y;
   phys_x -= len;
   buf[phys_x+1] = 0;
   buf[0] = phys_x;
   move_cursor(log_x,log_y); /* restore cursor on screen             */
   return(c);
}

/***
*
* Read in command line
*
***/
void do_extended()
{
   int c;

   do {
      c = do_prompt("*", lastcmd);
      push_command(lastcmd+1, lastcmd[0]);
   } while (c == C_ESC);
   vdu_writes(0,line_max+1,"");      /* Clear message line           */
}

/***
*
* Execute any arbitrary command safely
*
***/
void push_command(buf, len)
char *buf;
int len;
{
   LEVELBUF level;

   /* Don't use local storage here to allow us to be heavily recursive  */
   if (pushlev()) return;
   /* Protect against any errors that might occur while processing the  */
   /* file.  At the first time of trouble we will come back here and    */
   /* bail out completely.                                              */

   if (!setjmp(level))
   {
      err_level = &level;
      memcpy(cmdline+1, buf, len);
      cmdline[0] = len;
      execute_extended(0); 
   }
   poplev();
}

/***
*
* Execute a series of commands
*
***/
void execute_series(str)
char *str;
{
   int len;
   while(*str)
   {
      len = strlen(str);
      push_command(str, len);
      str += len+1;
   }
}

/***
*
* Execute a function key
*
***/
void execute_pfkey(key)
int key;
{
   char *p;
   if ((p = keydef[key]) != NULL)
   {
      memcpy(cmdline, p, p[0]+1);
      cmdptr = 1;
      execute_extended(KEY_LEV);
   }
}

/***
*
*
*
***/
int pushlev()
{
   struct CMDLEVEL *newlev;
   newlev = AllocVec(sizeof(struct CMDLEVEL)+8, MEMF_PUBLIC|MEMF_CLEAR);
   if (newlev == NULL)
   {
      msg(ERROR_NO_MEMORY);
      return(1);
   }
   newlev->save_level = err_level;
   newlev->save_buf   = cmdline;
   newlev->save_ptr   = cmdptr;
   newlev->link       = pending;
   pending = newlev;
   cmdline = newlev->buf;
   cmdptr = 1;
   return(0);
}

/***
*
*
*
***/
void poplev()
{
   struct CMDLEVEL *savecmd;

   if (pending == NULL) return;
   savecmd   = pending;
   pending   = pending->link;
   cmdptr    = savecmd->save_ptr;
   cmdline   = savecmd->save_buf;
   err_level = savecmd->save_level;
   if (savecmd->rxmsg != NULL)
      reply_rexx_command(savecmd->rxmsg,20,20,"Ed Terminated");
   FreeVec((char *)savecmd);
}

/***
*
* Test to see if edits will be lost and prompt the user before
* attempting to do an operation
*   A return code of 0 indicates that the operation should not be 
*   attempted.
*
***/
int lose_edits()
{
   if (data_changed &&
       do_query("Edits will be lost - type Y to confirm: ") != 'Y')
      return(0);
   return(1);
}

/***
*
*
*
***/
int ReadCmdFile(name)
char *name;
{
   BPTR fh;
   LEVELBUF level;
   int len;

   if ((fh = Open(name, MODE_OLDFILE)) == NULL)
      return(1);  /* Couldn't open the file     */

   /* Don't use local storage here to allow us to be heavily recursive  */
   if (pushlev()) return(1);

   /* Protect against any errors that might occur while processing the  */
   /* file.  At the first time of trouble we will come back here and    */
   /* bail out completely.                                              */
   if (!setjmp(level))
   {
      err_level = &level;
      /* Loop through reading the lines one at a time and putting them  */
      /* into our command buffer.  Note that we have to save the        */
      /* existing command buffer before we start this mess so that any  */
      /* commands that happen to be there are also processed.  This     */
      /* can get to be a problem in recursive situations.               */
      while(FGets(fh, cmdline+1, MAX_LINE))
      {
	 len=strlen(cmdline+1);
         if (cmdline[len] == '\n') len--;
         cmdline[0] = len;
         execute_extended(0); 
      }
   }
   Close(fh);
   poplev();
   return(0);
}

/***
*
* Execute stored command line
*
***/
void execute_extended(edit_level)
int edit_level;
{
   int ch, ch2;
   int count;
   int n;
   int savecmdptr;

   count = 0;
   for(;;)
   {
      n = cmdptr;
      ch = capch();

      if ((ch >= '0') && (ch <= '9'))   /* count                        */
      {
         count = count*10 + ch - '0';
         savecmdptr = cmdptr;           /* remember position in line    */
         continue;                      /* for next character           */
      }

      /* Look at next character & see if part of this command   */
      if (((ch >= 'A') && (ch <= 'Z')) || (ch =='@'))
      {
         ch2 = capitalch( getch() );
         if ((ch2 >= 'A') && (ch2 <= 'Z'))      /* it is                */
            ch = (ch << 8) | ch2;       /* pack both characters together*/
         else if (ch2 != ENDSTREAMCH)   /* otherwise stuff it back      */
            cmdptr--;
      }

nextcmd:
      switch(ch)
      {
         default:
            if (!send_rexx_command(cmdline+n, cmdline[0]-n+1))
            {
               /* Eat up all remaining characters in the buffer */
               cmdptr = cmdline[0]+1;
            }
            else
            {
               msg(ERROR_UNKNOWN);
            }
            break;

         case ENDSTREAMCH: break;

         case '(':      /*** Start of command group                   ***/
            execute_extended(edit_level+1);
            break;

         case ')':      /*** End of command group                     ***/
            if (edit_level <= 0)
               msg(ERROR_BRACKET);
            return;

         case 'A':      /*** Add text as line After current line      ***/
            gets(insert_vec, 0);
            do_replace();
            do_append(insert_vec);
            data_changed = TRUE;
            break;

         case 'B':      /*** Bottom of file                           ***/
            do_replace();
            do_bof(0);
            log_y = line_num;   /* Cursor to bottom line                */
            /* Drop trough to CE */
         case X_CE:     /* Cursor to End of line (CS see below)         */
            log_x = -window_base;       /* to fix cursor_eol            */
            do_extract();
            cursor_eol();
            break;

         case X_CT:     /*++ Cursor to Beginning/End of line            */
            do_extract();
            cursor_eol();
            break;

         case X_CM:     /*++ Handle extended command                  ***/
            do_extended();
            break;

         case X_BE:     /*** Block End                                ***/
            replace();
            block_end = line_ptr[log_y];
            break;

         case X_BS:     /*** Block Start                              ***/
            replace();
            block_start = line_ptr[log_y];
            break;

         case X_CL:     /*** Cursor Left                              ***/
            cursor_left();
            break;

         case X_CR:     /*** Cursor Right                             ***/
            cursor_right();
            break;

         case 'D':      /*** Delete line                              ***/
            do_replace();
            delete_line(log_y);
            data_changed = TRUE;
            break;

         case X_DB:     /*** Delete block                             ***/
            do_replace();
            do_delblock();
            data_changed = TRUE;
            break;

         case X_DC:     /*** Delete character at cursor               ***/
            do_extract();
            do_delch();
            data_changed = TRUE;
            break;

         case X_DF:     /*++ Display function key                     ***/
            /* Allow them to say DF ^C in addition to the number        */
            if (capch() == '^')
            {
               n = (getch() & 0x1f) + PFKEY_CTRL2;
            }
            else
            {
               cmdptr--;
               n = getn()-1; /* Normalize it */
            }
            if ((n >= 0) && (n < MAX_PFKEY))
            {
               if (keydef[n] == NULL)
                  status_msg[0] = 0;
               else
                  memcpy(status_msg, keydef[n], keydef[n][0]+1);
               status_msg[status_msg[0]+1] = 0; /* Null terminate it */
               msg(ERROR_STRING);
            }
            break;

         case X_DL:     /*++ delete to left of cursor                 ***/
            if (cursor_left())   /* ignore at start of line */
            {
               move_cursor(log_x,log_y); 
               /* drop through after setting cursor correct  */
               do_extract();
               do_delch();
            }
            break;

/*       case X_DO:   // Execute command */
/*          gets(insert_vec, 0);       */
/*          do_replace(do_halt,insert_vec); */
/*          break;        */

         case X_DW:     /*++ Delete word                              ***/
            do_extract();
            do_delw();
            break;

         case 'E':      /*** Exchange                                 ***/
         case X_EQ:     /*** Exchange & query                         ***/
            if (!checkomitted())
            {
               if (!gets(search_vec, 0))
               {
                  cmdptr--;   /* stuff delimiter back         */
               }
               gets(insert_vec, 0);
            }
            do_replace();
            do_exchange((BOOL)(ch==X_EQ));
            data_changed = TRUE;
            break;

         case X_EL:     /*++ delete to end of line                    ***/
            do_extract();
            do_deol();
            break;

         case X_EM:     /*++ Enable any changed menu                  ***/
            enable_menu();
            break;

         case X_EP:     /*++ Cursor to end of page                    ***/
            do_replace();
            cursor_eop();
            break;

         case X_EX:     /*** Extend margin                            ***/
            extend_margin = TRUE;
            break;

         case X_FC:     /*++ Flip case                                ***/
            do_extract();
            do_invertcase();
            break;

         case X_FR:     /*++ File Requester                           ***/
            requestflag = getn();
            if (window == NULL) requestflag = 0;
            break;

         case X_BF:     /*** Backwards find                           ***/
         case 'F':      /*** Find                                     ***/
             if (!checkomitted())
                gets(search_vec, 0);       /* get new string               */
             do_replace();
             find_string((ch == 'F')?1:-1);
                                /* start one position right or left     */
             break;

/*       case 'H':    // Halt editor     */
/*          do_replace(do_halt,0);  */
/*          break;        */

         case 'I':      /*** Insert line after current                ***/
            gets(insert_vec, 0);
            do_extract();
            do_insert();
            data_changed = TRUE;
            break;

         case X_IB:     /*** Insert Block                             ***/
            do_replace();
            do_insblock();
            data_changed = TRUE;
            break;

         case X_IF:     /*** Insert File                              ***/
            gets(insert_vec, requestflag);
            do_replace();
            do_insfile();
            data_changed = TRUE;
            break;

         case 'J':      /*** Join lines                               ***/
            do_extract();
            do_join();
            data_changed = TRUE;
            break;

         case X_LC:     /*** Do not equate cases                      ***/
            forcecase = FALSE;
            break; 

         case 'M':      /*** Move to line N                           ***/
            n = getn();
            if (n < 1) n = 1;
            do_replace();
            do_tof();           /* First to top;                     */
            n -= do_bof(n);     /* Then to line N or bottom (returns num) */
            log_y = 0;
            while (n--) cursor_down();
            goto sollbl;

         case 'N':      /*** Cursor to start of Next line             ***/
            do_replace();
            cursor_down();
            goto sollbl;

         case X_NW:     /*++ New project                              ***/
            if (!lose_edits()) break;
            empty_buffer();
            file_name[0] = 0;
            data_changed = FALSE;
            break;

         case X_OP:     /*++ Open file                                ***/
            if (lose_edits())
            {
               empty_buffer();
               file_name[0] = 0;
               gets(insert_vec, requestflag);
               if (!load_file(insert_vec+1))
               {
                  strcpy(file_name, insert_vec+1);
               }
               else
               {
                  VDU_CLEAR();
               }
               data_changed = FALSE;
            }
            break;

         case 'P':      /*** Cursor to start of Previous line         ***/
            do_replace();
            cursor_up();     /* and drop through             */

         case X_CS:     /*** Cursor to Start of line                  ***/
sollbl:
            log_x = 0;  /* back to start of line                        */
            set_screen_window( 0 );
            break;

         case X_PD:     /*++ Next page                                ***/
            do_replace();
            do_nextline(12);       /* Move 12 lines      */
            break;

         case X_PU:     /*++ Previous page                            ***/
            do_replace();
            do_prevline(12);       /* Move 12 lines      */
            break;

         case 'Q':      /*** Quit without saving                      ***/
            if (lose_edits())
            {
               VDU_CLEAR();
               tidyup(0);
            }
            break;

         case X_RE:     /*++ Perform current extended command line    ***/
	    if((edit_level & KEY_LEV))push_command(lastcmd+1, lastcmd[0]);
            break;

         case X_RF:     /*** Run File                                 ***/
            gets(insert_vec, requestflag);
            if (ReadCmdFile(insert_vec+1))
               msg(ERROR_BADFILE);
            break;

         case X_RK:     /*++ Reset the definition of all keys         ***/
            init_keys();
            break;

         case X_RP:     /*** Repeat                                   ***/
            count = -1;         /* so that repeats for ever             */
            savecmdptr = cmdptr;/* Remember where we are                */
            break;

         case X_RX:     /*** Rexx command                             ***/
            gets(insert_vec, requestflag);
            if (send_rexx_command(insert_vec+1, insert_vec[0]))
            {
               msg(ERROR_NO_REXX);
            }
            break;

         case X_RV:     /*** Rexx variable                            ***/
            gets(insert_vec, 0);
            set_rexx_vars(insert_vec);
            break;

         case 'S':      /*** Split line at cursor                     ***/
            do_extract();
            do_cr();
            data_changed = TRUE;
            break;

         case X_SA:     /*** Save to specified file or default        ***/
            {
               char *file = file_name;          /* Use this if omitted  */
               if (!checkomitted())
               {
                  gets(insert_vec, requestflag);
                  file = insert_vec+1;
                  /* Uncomment this next line if you do NOT want SA <file> */
                  /* to change the name of the file you are editing     */
                  /* if (!file_name[0]) */ 
                     strcpy(file_name, file);
               }
               replace();
               if (!do_writebuf(line_first,line_last,file,file))
                  longjmp(*err_level,ERRLABEL);
               data_changed = FALSE;
            }
            break;

         case X_SB:     /*** Show Block                               ***/
            do_replace();
            do_showblock();
            break;

         case X_SF:     /*** Set function key                         ***/
            /* Allow them to say SF ^C in addition to the number        */
            if (capch() == '^')
            {
               n = (getch() & 0x1f) + PFKEY_CTRL2;
            }
            else
            {
               cmdptr--;
               n = getn()-1; /* Normalize it */
            }
            gets(insert_vec, 0);
            if ((n >= 0) && (n < MAX_PFKEY))
            {
               if (keydef[n] != NULL) FreeVec(keydef[n]);
               if ((keydef[n] = AllocVec(insert_vec[0]+8, MEMF_PUBLIC)) == NULL)
                  msg(ERROR_NOROOM);
               memcpy(keydef[n], insert_vec, insert_vec[0]+1);
            }
            break;
        
         case X_SH:     /*** Show information                         ***/
            do_replace();
            do_show();
            break;

         case X_SI:     /*++ Set menu item information                ***/
            n = getn();
            ch2 = getn();
            if (ch2 & 3)
            {
               gets(insert_vec, 0);
            }
            else
            {
               insert_vec[0] = 0;
            }
            if (ch2 == 2)
            {
               gets(status_msg, 0);
            }
            else
            {
               status_msg[0] = 0;
            }
            set_menu_item(n, ch2, insert_vec, status_msg);
            break;

         case X_SL:     /*** Set Left margin                          ***/
            left_margin = getn()-1;
            if (left_margin < 0) left_margin = 0;
            break;

         case X_SM:     /*++ Status line message                      ***/
            gets(status_msg, 0);
            msg(ERROR_STRING);
            break;

         case X_SR:     /*** Set Right margin                         ***/
            right_margin = getn()-1;
            if (right_margin < 1) right_margin = 1;
            break;

         case X_ST:     /*** Set tab stop                             ***/
            tab_stop = getn();
            if (tab_stop < 1) tab_stop = 1;
            break;

         case 'T':      /*** Top of file                              ***/
            do_replace();
            do_tof();
            log_y = 0;
            goto sollbl;

         case X_TB:     /*++ Cursor to next tab position              ***/
            do_tab();
            break;

         case 'U':      /*** Undo changes on current line             ***/
            extracted = FALSE;
            flag_line(log_y);   /* Mark line so it will be redisplayed  */
            break;

         case X_UC:     /*** Equate cases                             ***/
            forcecase = TRUE;
            break;

         case X_VW:     /*++ Redisplay text                           ***/
            do_replace();
            do_verify();
            break;

         case X_WB:     /*** Write Buffer                             ***/
            gets(insert_vec, requestflag);
            replace();
            check_block();
            if (!do_writebuf(block_start,block_end,insert_vec+1,insert_vec+1))
               longjmp(*err_level,ERRLABEL);
            break;

         case X_WN:     /*** Next word                                ***/
            do_extract();
            cursor_nxw();
            break;

         case X_WP:     /*** Previous word                            ***/
            do_extract();
            cursor_prw();
            break;

         case 'XQ':     /*** Exit with query                          ***/
            if (data_changed &&
                do_query("File has been changed - type Y to save and exit: ") != 'Y')
            {
               break;
            }
            if (!data_changed)
            {
               VDU_CLEAR();
               tidyup(0);
               break;
            }
         case 'X':      /*** Normal exit                              ***/
            replace();
            DeleteFile(BACKUPNAME);
            Rename(file_name,BACKUPNAME);
            if (do_writebuf(line_first,line_last,file_name,BACKUPNAME))
            {
               VDU_CLEAR();
               tidyup(0);
            }
            else
            {
               DeleteFile(file_name);
               Rename(BACKUPNAME,file_name);
               longjmp(*err_level,ERRLABEL);
            }
      }  /* end of CASE    */

      /* Check for abandonement, and verify        */

      if(edit_level & KEY_LEV)ch=C_NOTREADY;
      else ch = display(FALSE);

      /* We would like for this to have been a switch as in the BCPL    */
      /* version, but that wouldn't work out right with the continues   */
      if (ch == C_EXECUTE)
      {
         cmdptr = 1;
         continue;              /* Do it again!                 */
      }
      else if (ch == 0x9B)       /* Some function key event      */
      {
         if (window_event(vud_rdch(TRUE)))
            resize_window();
      }
      else if (ch != C_NOTREADY && ((count != 0) || !(edit_level & KEY_LEV)))
      {
         msg(ERROR_ABANDON);
      }

      /* Check for repetition      */
      if (count > 0) count--;
      if (count != 0)   /* must do it again     */
      {
         cmdptr = savecmdptr;
         continue;
      }

      /* Check end of line or semicolon    */
      ch = capch();
      if (ch == ENDSTREAMCH) break;     /* All over                     */
      if (ch==')') goto nextcmd;        /* terminating bracket is OK    */
      if (ch != ';')
         msg(ERROR_SYNTAX);
   }

   /* Finish */
   cmdptr = 1;  /* back to start again          */
   if ((edit_level & ~KEY_LEV) != 0) msg(ERROR_BRACKET);
}


int do_query(msg)
char *msg;
{
   BOOL need_redisplay = FALSE;
   int c;

   for(;;)
   { 
      VDU_HIGHLIGHTON();
      vdu_writes(0,line_max+1, msg);
      VDU_HIGHLIGHTOFF();

      /* at this point someone might resize the window        */
      /* we must redisplay all (including prompt line)        */
      c = need_redisplay ? display(TRUE) : vud_rdch(TRUE);
      need_redisplay = FALSE;
      if (c != 0x9B) break;
      if (window_event(vud_rdch(TRUE)))
      {
         resize_window();
         need_redisplay = TRUE;
      }
   }
   vdu_writes(0,line_max+1, "");   /* clear line           */
   return(capitalch(c));
}

/***
*
* Return next non space character from command line, capitalised
*
***/
int capch()
{
   int c;
   while((c = getch()) == ' ');
   return(capitalch( c ));
}

#ifndef capitalch
/***
*
* Return a capitalized character
*
***/
int capitalch(c)
int c;
{
   if (c < 'a' || c > 'z') return(c);
   return(c - 'a' + 'A');
}
#endif

/***
*
* Return number from command line
*
***/
int getn()
{
   int n, c, i; 

   n = 0;

   c = capch();
   if (c == '?')
   {
      cmdptr--;
      gets(status_msg, 0);
      i = 1;
      while(c = status_msg[i++], (c >= '0') && (c <= '9'))
      {
         n = (n * 10) + (c - '0');
      }
   }
   else
   {
      while ((c >= '0') && (c <= '9'))
      {
         n = n*10 + c - '0';
         c = getch();
      }
      if (c != ENDSTREAMCH) cmdptr--;
   }
   return(n);
}

/***
*
* Read string enclosed in delimiters
*  Note that we return a BSTR with a null terminator
*  This routine probably should check buffer lengths...
*
***/
int gets(v, flag)
char *v;
int flag;
{
   int n, c, delim, doprompt;
   char pbuf[30];
   struct TagItem taglist[2];

   n = 0;
   doprompt = 0;
   delim = c = capch();
   /* Check to see if they want to prompt from the keyboard         */
   /* If they do, the string that follows is a prompt string to use */
   if (c == '?')
   {
      delim = c = capch();
      doprompt = 1;
   }
   /* Check valid delimiter     */
   if (((c >= 'A') && (c <= 'Z')) ||
       ((c >= '0') && (c <= '9')) ||
        (c == ';') ||
        (c == '(') ||
        (c == ')') ||
        (c == ENDSTREAMCH))
      msg(ERROR_SYNTAX);
   for(;;)
   {
      c = getch();
      if ((c == delim) || (c == ENDSTREAMCH)) break;
      v[++n] = c;
   }
   v[0] = n;
   v[n+1] = 0;  /* Null terminate it */
   if (doprompt)
   {
      if (flag)
      {
         taglist[0].ti_Tag = ASL_Hail;
         taglist[0].ti_Data = (ULONG)(v+1);

         taglist[1].ti_Tag = TAG_DONE;

         if (AslRequest( (APTR)FileRequester, taglist))
         {
            n = strlen(FileRequester->rf_Dir);
            if (n > 255) n = 255;
            memcpy(v+1, FileRequester->rf_Dir, n);

            if (n > 0 && v[n] != ':') v[++n] = '/';
            c = strlen(FileRequester->rf_File);
            if ((n + c) > 255) c = 255-n;
            memcpy(v+1+n, FileRequester->rf_File, c);
            v[0] = n + c;
            v[n+c+1] = 0;
         }
         else
            msg(ERROR_ABANDON);
      }
      else
      {
         if (n > 30) v[30] = 0;
         strcpy(pbuf, v+1);
         do_prompt(pbuf, v);
         vdu_writes(0,line_max+1,pbuf);  /* Write prompt on message line */
      }
   }

   return(doprompt);
}

/***
*
* Return FALSE if there is something to read, TRUE if omitted.
*
***/
BOOL checkomitted()
{
   int c;

   c = capch();         /* Look ahead at next character                 */
   if (c != ENDSTREAMCH) cmdptr--;      /* Stuff it back anyway         */
   return((BOOL)((c == ';') || (c == ENDSTREAMCH) || (c=='(')||(c==')')));
}

/***
*
* Return next character from command line
*
***/
int getch()
{
   if (cmdptr > cmdline[0]) return(ENDSTREAMCH);
   return((int)cmdline[cmdptr++]);
}

/***
*
* Replace current line
*
***/
void replace()
{
   if (extracted)
      replace_current(log_y);
}

#if 0
/* Pause edit session or execute another command        */
and do_halt(command)
{ let termin == findinput("**")
   move_cursor( 0, line_max+1 )
   VDU_UNINIT();    /* Undo any VDU options  */
   /* Terminate SC mode */
   sendpkt(notinuse,consoletask,act_sc_mode,?,?,FALSE)
   dqpkt(read_pkt)
   test termin==0 then writes("No more store\n")
   else
   { if (!command==0 do execute(command)
      writes("ED halted - type RETURN to continue\n")
      /* Re-enter here - wait for RETURN        */
      selectinput(termin)
      until rdch();=='\n' loop
      endread();
   }
   /* Into SC mode again, requesting message handling   */
   sendpkt(notinuse,consoletask,act_sc_mode,?,?,TRUE,TRUE)
   read_pkt->pkt_id = consoletask
   qpkt(read_pkt)
   VDU_INIT();    /* Reset any VDU options  */
   do_verify();
}
#endif

