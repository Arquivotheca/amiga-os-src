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
/* Command: Ed4.c                                                       */
/* Author:  John A. Toebes, VIII                                        */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 23NOV89  John Toebes   Initial Creation                              */
/* 10MAR90  John Toebes   Corrected getsp to allocate bytes             */
/* 24MAR90  John Toebes   Corrected tab expansion algorithm             */
/* 01APR90  John Toebes   Allocate FIB for global usage                 */
/* 01APR90  John Toebes   Added menu support calls                      */
/* 06APR90  John Toebes   Changed initialization method for keys        */
/* 16APR90  John Toebes   Deleted unused variable, changed ^] to CT     */
/* 23MAY90  John Toebes   Eliminated getsp completely                   */
/* 30MAY90  John Toebes   Consolidated file reading into load_file      */
/* 30AUG90  John Toebes   Corrected B9203 - Memory loss                 */
/* 14OCT90  John Toebes   Corrected B10056 - Added Workbench support    */
/* 21OCT90  John Toebes   Corrected crashing on limits.                 */
/* 01NOV90  John Toebes   Eliminated Unnecessary local SysBases         */
/* 29DEC90  John Toebes   Corrected Enforcer hit on Large windows       */
/* 18APR90  John Toebes   Made rdargs a global to eliminate enforce hits*/
/* Notes:                                                               */
/*----------------------------------------------------------------------*/
#include "ed.h"
#include "ed_rev.h"
#include "clib/icon_protos.h"
#include "pragmas/icon_pragmas.h"
#include "workbench/startup.h"

/************************************************************************/
/*                                                                      */
/* Screen editor  - Initialisation functions and single character i/o   */
/*                                                                      */
/*   Initialises globals, gets space and arguments.                     */
/*   Sets file_name, and reads it in into buffer.                       */
/*   Sets up buffer data structure and first screen full                */
/*                                                                      */
/************************************************************************/

#define TEMPLATE    "FROM/A,SIZE/N,WITH/K,WINDOW/K,TABS/N,WIDTH=COLS/N,HEIGHT=ROWS/N" CMDREV
#define OPT_FROM   0
#define OPT_SIZE   1
#define OPT_WITH   2
#define OPT_WINDOW 3
#define OPT_TABS   4
#define OPT_WIDTH  5
#define OPT_HEIGHT 6
#define OPT_COUNT  7

/***
*
* Examine file to find suitable size for buffer
*
***/
long get_size(name)
char *name;
{
   long size;
   int rc;
   BPTR lock;

   size = DEFAULT_SIZE;

   lock = Lock(name, SHARED_LOCK);

   if (lock != NULL)      /* New file             */
   {
      rc = Examine(lock, fib);

      UnLock(lock);

      if (rc != DOSTRUE)
      {
         rc = IoErr();
         if (WBenchMsg != NULL) fatal("Cannot Examine() file");
         VPrintf("Unable to obtain information about %s\n",
                 (long *)&name);
         PrintFault(rc, NULL);
         tidyup(RETURN_FAIL);
      }

      if (fib->fib_DirEntryType >= 0)
      {
         if (WBenchMsg != NULL) fatal("Cannot edit directories");
         VPrintf("%s is a directory and cannot be edited\n",
                 (long *)&name);
         tidyup(RETURN_FAIL);
      }

      size = fib->fib_Size * 2;
      /* The size is specified bytes, but we must allow for        */
      /* each \n which causes word alignment       */
      if (size < DEFAULT_SIZE) size = DEFAULT_SIZE;
   }
   return(size);
}

/***
*
* Get the value for a specified tool (or the default value) from the 
* tooltypes array of an icon
*
***/
int gettoolval(name, tt, dflt)
char *name;
char **tt;
int dflt;
{
   char *p;
   int val;
   
   p = FindToolType(tt, name);
   if (p == NULL) return(dflt);
   val = 0;
   while(*p)
   {
      if (*p < '0' || *p > '9') return(dflt);
      val = (val * 10) + *p++ - '0';
   }
   return(val);
}

/***
*
* Editor Initialization
*
***/
void init_ed()
{
   long worksize, workhold;
   char *name;
   int i,my;
   long opts[OPT_COUNT];
   struct InfoData *infodata;
   struct WBArg *arg;

   if (DOSBase->dl_lib.lib_Version < 36)
   {
      if (WBenchMsg == NULL)
      {
         Write(Output(), "Ed requires Version 2.0 to run\n", 31);
      }
      Result2(ERROR_INVALID_RESIDENT_LIBRARY);
      EXIT(RETURN_FAIL);
   }

   if ((!(IntuitionBase = OpenLibrary("intuition.library", 0))) ||
       (!(GadToolsBase  = OpenLibrary("gadtools.library",  0))) ||
       (!(AslBase       = OpenLibrary("asl.library",       0))))
   {
      if (WBenchMsg == NULL)
      {
         Write(Output(), "Unable to open needed library\n", 31);
      }
      if (IntuitionBase != NULL)  CloseLibrary((struct Library *)IntuitionBase);
      if (GadToolsBase  != NULL)  CloseLibrary(GadToolsBase);
      Result2(ERROR_INVALID_RESIDENT_LIBRARY);
      EXIT(RETURN_FAIL);
   }

   fib = AllocVec(sizeof(struct FileInfoBlock), MEMF_PUBLIC|MEMF_CLEAR);
   if (fib == NULL)
   {
      fatal("Not enough free store");
   }

   memset((char *)opts, 0, sizeof(opts));
   tab_stop = DEFAULT_TABS;             /* default value                */

   term_width = term_height = 0;  /* No Specified height or width */

   /* If we were invoked from workbench, we need to set up the environment */
   /* so that it will work.  We fill in the appropriate items from the     */
   /* tool types and set up the global file name.  After this, anyone who  */
   /* Attempts to write to the standard output should check WBenchMsg and  */
   /* instead call fatal in order to exit.  fatal() now understands how to */
   /* put up a requester in the workbench situation.                       */
   /* We now support the following tooltypes                               */
   /*   FROM=xxx        Name of file to edit.  Defaults to name of icon    */
   /*   WITH=xxx        Name of with file to use                           */
   /*   WINDOW=xxx      Name of window to use                              */
   /*   HEIGHT=n        Height of window in characters                     */
   /*   WIDTH=n         Width of window in characters                      */
   /*   TABS=n          Tabstop setting                                    */
   /*   SIZE=n          Size of buffer in bytes                            */
   if (WBenchMsg != NULL)
   {
      if ((IconBase = OpenLibrary("icon.library", 0)) != NULL)
      {
         arg = WBenchMsg->sm_ArgList;
         arg += WBenchMsg->sm_NumArgs-1;

         CurrentDir(arg->wa_Lock);
         diskobj = GetDiskObject(arg->wa_Name);

         opts[OPT_FROM]   = (long)FindToolType(diskobj->do_ToolTypes, "FROM");
         if (opts[OPT_FROM] == 0)  opts[OPT_FROM] = (long)arg->wa_Name;

         opts[OPT_WITH]   = (long)FindToolType(diskobj->do_ToolTypes, "WITH");
         opts[OPT_WINDOW] = (long)FindToolType(diskobj->do_ToolTypes, "WINDOW");

         workhold    = gettoolval("SIZE",   diskobj->do_ToolTypes, -1);
         if (workhold != -1)
            opts[OPT_SIZE] = (long)&workhold;
         tab_stop    = gettoolval("TABS",   diskobj->do_ToolTypes, DEFAULT_TABS);
         term_width  = gettoolval("WIDTH",  diskobj->do_ToolTypes, 0);
         term_height = gettoolval("HEIGHT", diskobj->do_ToolTypes, 0);
      }
      rdargs = NULL;
   }
   else
   {
      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL)
      {
         PrintFault(IoErr(), NULL);
         FreeVec(fib);
         EXIT(RETURN_ERROR);
      }
      if (opts[OPT_TABS] != NULL)   tab_stop    = *(long *)opts[OPT_TABS];
      if (opts[OPT_WIDTH]  != NULL) term_width  = *(long *)opts[OPT_WIDTH];
      if (opts[OPT_HEIGHT] != NULL) term_height = *(long *)opts[OPT_HEIGHT];
   }

   worksize = get_size((char *)opts[OPT_FROM]); /* Get suitable default size   */
   if (opts[OPT_SIZE] != NULL)
   {
      worksize = *(long *)opts[OPT_SIZE];
      if (worksize < MINSIZE)
      {
         if (WBenchMsg) fatal("SIZE= value too small");
         VPrintf("SIZE of %ld too small\n", &worksize);
         FreeVec(fib);
         if (rdargs != NULL) FreeArgs(rdargs);
         EXIT(RETURN_ERROR);
      }
   }

   /* Open new window   */
   name = (char *)opts[OPT_WINDOW];
   if (name == NULL) name = WINDOWNAME;
   console_stream = Open(name, MODE_NEWFILE);
   /* Make sure the we get a file that we can do a sendpacket interface on */
   if (console_stream == NULL ||
       ((struct FileHandle *)(BADDR(console_stream)))->fh_Type == NULL)
   {
      /* Now we might have to close the file if we didn't find it acceptable */
      if (console_stream) Close(console_stream);
      if (WBenchMsg) fatal("Invalid WINDOW= Specification");
      VPrintf("Unable to open window %s\n", (long *)&name);
      if (rdargs != NULL) FreeArgs(rdargs);
      FreeVec(fib);
      EXIT(RETURN_FAIL);
   }

   /* Turn it into raw mode so that they can get away with specifying   */
   /* '*' or even CON: as the current stream.                           */
   SetMode(console_stream, 1);

   /* See if we can figure out that we have a window pointer attached   */
   if (IsInteractive(console_stream) &&
       (infodata = (struct InfoData *)
                   AllocVec(sizeof(struct InfoData), MEMF_PUBLIC|MEMF_CLEAR)) != NULL)
   {
#if 0
      if (Info(console_stream, &infodata))
#else
      if (DoPkt4(((struct FileHandle *)BADDR(console_stream))->fh_Type,
                ACTION_DISK_INFO, MKBADDR(infodata), 0, 0, 0))
#endif
      {
         window = (struct Window *)infodata->id_VolumeNode;
#ifndef CONSOLE_IS_FIXED
         /* In order to get around some problems with previous version of */
         /* the console.device we need to check for simple refresh windows*/
         /* and arbitrarily disallow them.  We will need to take this out */
         /* in some later version of ED.                                  */
         if (window->Flags & SIMPLE_REFRESH)
         {
            fatal("Simple refresh window illegal for ED");
         }
#endif
      }
      FreeVec(infodata);
   }

   /* Get our work space */
   line_first = AllocVec(worksize+8, MEMF_PUBLIC|MEMF_CLEAR);
   if (line_first == NULL)
   {
      fatal("Not enough free store");
   }
   if ((FileRequester = AllocFileRequest()) == NULL)
   {
      fatal("Cannot create file requester");
   }

   worktop = (char *)line_first+worksize;
   line_first->data[0] = 0;
   line_last = line_first;      /* no lines in buffer at first          */

   /* Save away the name of the file that they want us to edit          */
   strcpy(file_name, (char *)opts[OPT_FROM]);

   /* Disable window heights and widths that will cause us trouble      */
   if ( (term_width && (term_width < 10)) ||
        (term_height && (term_height < 3)))
   {
      fatal("HEIGHT or WIDTH too small");
   }

   /* Do a sanity check on the tabs.                                    */
   if ((tab_stop < 1) || (tab_stop >= MAX_LINE))
   {
      fatal("Invalid TABS specification");
   }

   lastcmd[0] = 0;

   cmdline = cmdbuf;                    /* Room for command line        */
   cmdline[0] = 0;                      /* empty to start with          */
   cmdptr = 1;                          /* and nothing in it            */

   search_vec[0] = 0;                   /* and empty                    */

   worktop -= SAFETYAREA;               /* allow a safety margin        */

   window_base = 0;                     /* no horizontal offset         */
   extracted = FALSE;                   /* no current line extracted    */
   data_changed = FALSE;                /* nothing changed              */

   memset(line_ptr, 0, MAXLINES*4);
   memset(line_mods, 0, MAXLINES);
   line_ptr[0] = line_first;            /* always display first line    */
   line_num = 0;
   prev_list = NULL;
   phys_x = phys_y = log_x = log_y = 0;
   block_start = block_end = 0;
   extend_margin = FALSE;
   forcecase = FALSE;
   msg_num = 0;
   err_level = NULL;
   initpkts();
   rexx_port = initrexx();

   /* Now we can set up the proper width & height       */
   line_max     = VDU_LENGTH() - 2;
   if (line_max >= MAXLINES) line_max = MAXLINES-1;
   screen_width = VDU_WIDTH() - 1;
   left_margin  = 0;
   right_margin = screen_width;
   VDU_CLEAR();

   for(my=0; my<MAXMENU; my++) {
	   newmenu[my].nm_UserData = NULL;
   	   newmenu[my].nm_Label = NULL;
   }

   if (window != NULL)
   {
      requestflag = 1;
      init_menus();
   }


   /* Initialize all the key definitions to be empty                    */
   for(my = 0; my < MAX_PFKEY; my++)keydef[my] = NULL;

   init_keys();
   ReadCmdFile("S:Ed-Startup");
   if (load_file(file_name))
   {
      i = status_msg[0];
      status_msg[i+1] = 0;
      fatal(status_msg+1);
   }
   if ((opts[OPT_WITH] != NULL) &&
       ReadCmdFile((char *)opts[OPT_WITH]))
      msg(ERROR_BADFILE);

   if (rdargs != NULL)
   {
      FreeArgs(rdargs);
      rdargs = NULL;
   }
}


/***
*
* Routine for reading file on startup as well as to
*   cope with the extended command 'IF'
*    N_B. Modifies global 'line_last'.
*
***/
BOOL readfile(stream,base,startup)
BPTR stream;
struct LINE *base;
BOOL startup;
{
   int ch, len, nlines, i;
   BOOL truncated, tabsexpanded;
   long quantum = (worktop - (char *)line_first) / screen_width;
   char *trigger = (char *)line_first + quantum;

   truncated = tabsexpanded = FALSE;
   len = 0;
   nlines = 0;

   for(;;)
   {
      ch = FGetC(stream);
nextin:
      if (((ch >= ' ' ) && (ch <  0x7F)) ||
          ((ch >= 0xA0) && (ch <= 0xFF)))
      {
         /* Normal text character        */
         if (len < MAX_LINE)
            base->data[1+len++] = ch;
         else
            truncated = TRUE;
      }
      /* Not a normal text character    */
      else switch(ch)
      {
         default: /* File contains binary       */
            Close(stream);
            return(RDF_BIN);
         case '\n':
         case '\r':
         case '\f':
         case '\x1b':
         case ENDSTREAMCH:
         case 0:
            base->data[0] = len;
            line_last = base;
            if (startup)
            {
               if (nlines <= line_max)
               {
                  /* Screen ptr, flag to be displayed        */
                  line_ptr[nlines] = line_last;
                  line_mods[nlines] = 1;
                  nlines++;
               }
            }
            base = NEXTLINE(base);
            ch = FGetC(stream);
            if (ch == ENDSTREAMCH) goto done;
            len = 0;
            if (((char *)base + 256) >= worktop)
            {
               Close(stream);
               return(RDF_TOOBIG);  /* File too big     */
            }
            if (startup)
            {
               /* Display something to keep punters happy    */
               if ((char *)base > trigger)
               {
                  trigger += quantum;
                  vud_wrch('@');
                  phys_x++;
                  vud_flush();
               }
            }
            goto nextin;

         case '\t':  /* Tab      */
#if 0
                /* The BCPL code always expands tabs to 3.  We will */
                /* Ignore what they did and expand tabs correctly   */
                i = 3;
#else
                i = tab_stop - (len % tab_stop);
#endif
                if ((len + i) >= MAX_LINE)
                {
                   truncated = TRUE;
                   i = MAX_LINE - len;
                }
                while(i--) base->data[1+len++] = ' ';

                tabsexpanded = TRUE;
                break;
      }
   }

done:
   Close(stream);
   if (startup)
   {
      VDU_CLEAR();
      line_num = nlines - 1;
   }
   line_lastword = (char *)NEXTLINE(line_last);

   if (tabsexpanded)
      return(RDF_TABS);
   if (truncated)
      return(RDF_TRUNC);
   return(RDF_OK);
}

/***
*
* Single character interaction stuff
*
***/

void initpkts()
{
   reply_port = CreateMsgPort();
   read_pkt = (struct StandardPacket *)
              AllocVec(sizeof(struct StandardPacket), MEMF_PUBLIC|MEMF_CLEAR);
   if (read_pkt == NULL) fatal("Not enough free store");

   read_pkt->sp_Msg.mn_Node.ln_Name = (char *)&(read_pkt->sp_Pkt);
   read_pkt->sp_Pkt.dp_Link = &read_pkt->sp_Msg;    /* Insert back mc pointer       */
   read_pkt->sp_Pkt.dp_Port = reply_port;
   read_pkt->sp_Pkt.dp_Type = 'R';      /* A read request               */
   read_pkt->sp_Pkt.dp_Arg1 = ((struct FileHandle *)
                                 BADDR(console_stream))->fh_Arg1;
   read_pkt->sp_Pkt.dp_Arg2 = (long)&read_pkt->sp_Pkt.dp_Arg4;      /* A suitable buffer    */
   read_pkt->sp_Pkt.dp_Arg3 = 1;

   /* send out read pkt        */
   PutMsg(((struct FileHandle *)BADDR(console_stream))->fh_Type,
          (struct Message *)read_pkt);

   VDU_INIT();  /* Initialise VDU driver (doesn't do anything!)         */
   /* The next chars ask that the console device                        */
   /* inform us of changes to the window size                           */
   vud_wrline("\x9B" "12{"
              "\x9B"  "2{"
              "\x9B" "10{"
              "\x9B" "11{",15);
}

/**
*
* Initialize all the default keys
*
**/
void init_keys()
{

   execute_series("SF21'CS';SF22'CE';SF23'T';SF24'B';SF25'DC'"     "\0"

                  "SF^@'BS';SF^A'A //';SF^B'D';SF^C''"             "\0"

                  "SF^D'PD';SF^E'EP';SF^F'FC';SF^G'RE'"            "\0"

                  "SF^H'DL';SF^I'TB';SF^J'';SF^K''"                "\0"

                  "SF^L'';SF^M'S';SF^N'';SF^O'DW'"                 "\0"

                  "SF^P'';SF^Q'';SF^R'WP';SF^S''"                  "\0"

                  "SF^T'WN';SF^U'PU';SF^V'VW';SF^W''"              "\0"

                  "SF^X'';SF^Y'EL';SF^Z'';SF^['CM'"                "\0"

                  "SF^\\'';SF^]'CT';SF^^'';SF^_''"                 "\0"
                 );
}


/***
*
* Read a character from the keyboard.
*
***/
int vud_rdch(wait)
BOOL wait;
{
   int c;
   ULONG sigmask;
   struct StandardPacket *sp;
   struct Message *msg;

   sigmask = 1L << reply_port->mp_SigBit;
   if (rexx_port != NULL)
      sigmask |= 1L << rexx_port->mp_SigBit;

   while ((sp = (struct StandardPacket *)GetMsg(reply_port)) == NULL)
   {
      c = 0;
      while ((rexx_port != NULL) &&
          ((msg = GetMsg(rexx_port)) != NULL))
      {
         do_rexx_cmd(msg);
         c = C_AREXX;
      }

      /* No pkt already waiting    */

      if (!wait) return(C_NOTREADY);

      if (c) return(c);

      vud_flush();

      /* wait for something to happen  */
      Wait(sigmask);
   }

   if (sp != read_pkt)
   {
      /* Something is seriously wrong here... */
      fatal("Unexpected packet");
   }
   if (read_pkt->sp_Pkt.dp_Res1 == 0)
      fatal("Unable to read a character");

   c = *(char *)&sp->sp_Pkt.dp_Arg4;
   read_pkt->sp_Pkt.dp_Port = reply_port;

   /* send out read pkt        */
   PutMsg(((struct FileHandle *)BADDR(console_stream))->fh_Type,
          (struct Message *)read_pkt);

   while ((rexx_port != NULL) &&
          ((msg = GetMsg(rexx_port)) != NULL))
      do_rexx_cmd(msg);

   return(c);
}

/***
*
* Flush out any pending I/O
*
***/
void vud_flush()
{
   if (vud_bpos)
      Write(console_stream, vud_buf, vud_bpos);

#if 0
   {
   int i, val;
   /* Enable this to see debugging of messages that are output */
   for (i = 0; i < vud_bpos; i++)
   {
      val = vud_buf[i];
      if (val >= ' ' && val <= 0x7e)
         VPrintf("%lc", &val);
      else
         VPrintf("<%lx>", &val);
   }
   }
#endif
   vud_bpos = 0;
}

/***
*
* Write a single character to the console
*
***/
void vud_wrch(ch)
int ch;
{
   if (vud_bpos >= VUD_SIZE) vud_flush();
   vud_buf[vud_bpos++] = ch;
}

/***
*
* Write string S on screen at X, Y
*
***/
void vdu_writes(x,y,s)
int x;
int y;
char *s;
{
   int len = strlen(s);

   move_cursor(x,y);
   VDU_DEOL();
   vud_wrline(s,len);
   phys_x = x+len;
}

/***
*
* Write a line to the screen
*
***/
void vud_wrline(buf,len)
char *buf;
int len;
{
   int size;

   if (len > 0)
   {
      if (len >= VUD_SIZE)
      {
         /* Can't be put into the buffer so just flush          */
         /* and write it by hand                                */
         vud_flush();
         Write(console_stream, buf, len);
      }
      else
      {
         if ((len + vud_bpos) >= VUD_SIZE)
         {
            size = VUD_SIZE - vud_bpos;
            memcpy(vud_buf+vud_bpos, buf, size);
            vud_bpos = VUD_SIZE;
            vud_flush();
            buf += size;
            len -= size;
         }
         memcpy(vud_buf+vud_bpos, buf, len);
         vud_bpos += len;
      }
   }
}

/***
*
* Fatal error - give message and stop
*
**/
void fatal(s)
char *s;
{
   struct IntuiText req, ok;

   if (console_stream != NULL)
      VDU_CLEAR();
   if (WBenchMsg == NULL)
   {
      /* From the CLI, life is easy, just print a message */
      VPrintf("%s\n", (long *)&s);
   }
   else
   {
      memset((char *)&req, 0, sizeof(req));
      req.FrontPen = 1;
      req.DrawMode = JAM1;
      req.LeftEdge = 4;
      req.TopEdge =  4;
      ok = req;
      req.IText = s;
      ok.IText  = "OK";
      
      /* From Workbench we have to give them a requester.  this is a pain */
      AutoRequest(NULL,
	           &req,
	             &ok, &ok,
	             DISKINSERTED,
	             0,
	             320,
	             72);

   }
   tidyup(RETURN_FAIL);
}

/***
*
* Terminate and clean up
*
***/
void tidyup(rc)
int rc;
{
   int i;

   /* We no longer need the parsed command line so throw it away        */
   if (rdargs != NULL) FreeArgs(rdargs);

   free_menus();
	if (diskobj) FreeDiskObject(diskobj);
   if (console_stream != NULL)
   {
      VDU_UNINIT();
      vud_wrch('\n');
      vud_flush();
      SetMode(console_stream, 0);
      Close(console_stream);
   }
   if (fib != NULL)        { FreeVec(fib);        fib = NULL;        }
   if (line_first != NULL) { FreeVec(line_first); line_first = NULL; }
   if (read_pkt != NULL)   { FreeVec(read_pkt);   read_pkt = NULL;   }
   if (FileRequester != NULL)	FreeFileRequest(FileRequester);

   for(i = 0; i < MAX_PFKEY; i++)
   {
      if (keydef[i] != NULL)
         FreeVec(keydef[i]);
      keydef[i] = NULL;
   }
   if (reply_port != NULL) DeleteMsgPort(reply_port);
   while(pending != NULL) poplev();
   shut_down_rexx();
	if (IconBase) CloseLibrary(IconBase);
   CloseLibrary((struct Library *)IntuitionBase);
   CloseLibrary(GadToolsBase);
   CloseLibrary((struct Library *)AslBase);
   SetSignal(0,SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D|
             SIGBREAKF_CTRL_E|SIGBREAKF_CTRL_F); /* Clear flags */
   EXIT(rc);
}
