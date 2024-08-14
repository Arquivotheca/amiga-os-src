/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1990 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 481-6436    */
/* | o  | ||   John Toebes     John Mainwaring    Jim Cooper                 */
/* |  . |//    Bruce Drake     Gordon Keener      Dave Baker                 */
/* ======      Doug Walker                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1990 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*-----------------------------------------------------------------------*/
/* Command: EdRexx.c                                                     */
/* Author:  John A. Toebes, VIII                                         */
/* Change History:                                                       */
/*  Date    Person        Action                                         */
/* -------  ------------- -----------------                              */
/* 01MAR90  John Toebes   Initial Creation from fancydemo.c by Bill Hawes*/
/* 30MAY90  John Toebes   Changed CreatePort/DeletePort to use the new   */
/*                        exec.library routines                          */
/* 30OCT90  John Toebes   Handled out of memory situations               */
/* 01NOV90  John Toebes   Eliminated Unnecessary local SysBases          */
/* Notes:                                                                */
/*-----------------------------------------------------------------------*/
#include "ed.h"
#include <rexx/rxslib.h>
#include "rexx_protos.h"
#include "rexx_pragmas.h"

/***
*
* Initialize the REXX environment
*  Allocate a port of the appropriate name (if we can)
*
***/
struct MsgPort *initrexx()
{
   struct MsgPort *the_port;
   int num;

   strcpy(portname_buf, HOST_PORT_NAME);

   Forbid();

   num = 0;
   /* look for someone else that looks just like us! */
   while (FindPort(portname_buf))
   {
      if (!num)
      {
         strcpy(portname_buf, ALT_PORT_NAME);
      }
      else if (num < 10)
      {
         portname_buf[strlen(ALT_PORT_NAME)-1]++;
      }
      else
      {
         Permit();
         return(NULL);
      }
      num++;
   }

   /* allocate the port */
   the_port = CreateMsgPort();
   if (the_port == NULL) return(NULL);
   the_port->mp_Node.ln_Name = portname_buf;
   the_port->mp_Node.ln_Pri   = 0L;
   AddPort(the_port);
   /* CreatePort(portname_buf,0L); */

   Permit();

   return(the_port);
}

/***
*
* Terminate any rexx environment established
*
***/
void shut_down_rexx()
{
   struct Message *msg;

   if (rexx_port != NULL)
   {
      while (outstanding_rexx_commands)
      {
        /* now wait for something to come from rexx */
        Wait(1L << rexx_port->mp_SigBit);
        if ((msg = GetMsg(rexx_port)) != NULL)
            do_rexx_cmd(msg);
      }
      RemPort(rexx_port);
      DeleteMsgPort(rexx_port);
   }
   if (RexxSysBase != NULL) {
     CloseLibrary((struct Library *)RexxSysBase);
       RexxSysBase = NULL;
   }
}


/***
*
* Handle an incomming rexx message.
*  If it is a request to do a command, pass it on to ED
*  if it is a reply to a request we send, process and free it
*
***/
void do_rexx_cmd(msg)
struct Message *msg;
{
   struct RexxMsg *rm = (struct RexxMsg *)msg;
   char buf[255];
   long args[3];

   /* is this a reply to a previous message? */
   if (rm->rm_Node.mn_Node.ln_Type == NT_REPLYMSG)
   {
      if (rm->rm_Result1 != 0)
      {
         args[0] = (long)rm->rm_Args[0];
         args[1] = rm->rm_Result1;
         args[2] = rm->rm_Result2;

         vsprintf(buf, "REXX: '%s' R(%ld, %ld)", args);
         VDU_HIGHLIGHTON();
         vdu_writes(0, line_max+1, buf);
         VDU_HIGHLIGHTOFF();
      }

      free_rexx_command(rm);
   }
   else
   {
      /* a rexx macro has sent us a command, deal with it */
      /* THE MESSAGE WILL HAVE BEEN REPLIED INSIDE OF execute_command */
      execute_command(rm);
   }
}

/***
*
* Send a command to rexx to be processed
*  If not successful for any reason, return failure so ED can issue an
*  error message
*
***/
int send_rexx_command(buff, len)
char *buff;
int len;
{

   struct MsgPort *rexxport;      /* this will be rexx's port */
   struct RexxMsg *rexx_command_message;   /* this is the message */

   /* lock things temporarily */
   Forbid();

   /* if rexx is not active, just return 1 */
   if ((rexxport = FindPort(RXSDIR)) == NULL)
   {
     Permit();
     return(1);
   }

   /* now open the library, THIS SHOULD NEVER FAIL BECAUSE REXX IS ACTIVE*/
   if (outstanding_rexx_commands == 0)
     if ((RexxSysBase = (struct RxsLib *)OpenLibrary(RXSNAME,0L)) == NULL)
     {
       Permit();
       return(1);
     }

   /* allocate a message packet for our command */
   /* note that this is a very important call.  Much flexibility is */
   /* available to you here by using multiple host port names, etc. */
   if ((rexx_command_message = CreateRexxMsg(rexx_port,
                    REXX_EXTENSION,
                    rexx_port->mp_Node.ln_Name))
         /* the last parameter could have been HOST_PORT_NAME */
      == NULL)
   {
     if (outstanding_rexx_commands == 0)
     {
       CloseLibrary((struct Library *)RexxSysBase);
       RexxSysBase = NULL;
     }
     Permit();
     return(1);
   }

   /* create an argument string and install it in the message */
   if ((rexx_command_message->rm_Args[0] =
      CreateArgstring(buff,len)) == NULL)
   {
     DeleteRexxMsg(rexx_command_message);
     if (outstanding_rexx_commands == 0)
     {
       CloseLibrary((struct Library *)RexxSysBase);
       RexxSysBase = NULL;
     }
     Permit();
     return(1);
   }

   /* tell rexx that this is a COMMAND, not a FUNCTION, etc. */
   rexx_command_message->rm_Action = RXCOMM;

   /* and now the EASY part! */
   PutMsg(rexxport,rexx_command_message);

   /* keep a count of outstanding messages for graceful cleanup */
   outstanding_rexx_commands++;

   /* we're done hogging */
   Permit();

   /* successful, finally... */
   return(0);
}

/***
*
* Free up any storage allocated for a message sent to rexx
*
***/
void free_rexx_command(rexxmessage)
struct RexxMsg *rexxmessage;
{
   /* delete the argument that we originally sent */
   DeleteArgstring(rexxmessage->rm_Args[0]);

   /* delete the extended message */
   DeleteRexxMsg(rexxmessage);

   /* decrement the count of outstanding messages */
   outstanding_rexx_commands--;
   if (outstanding_rexx_commands == 0)
   {
     CloseLibrary((struct Library *)RexxSysBase);
     RexxSysBase = NULL;
   }
}

/***
*
* Execute an incomming command from REXX
*
***/
void execute_command(rexxmessage)
struct RexxMsg *rexxmessage;
{
   int len;
   LEVELBUF level;
   LEVEL    templev;

   /* Don't use local storage here to allow us to be heavily recursive  */

   if(!rexxmessage)return;

   templev = err_level;
   err_level = 0;
   if (pushlev())
   {
      reply_rexx_command(rexxmessage,msg_num,0,get_msg());
      err_level = templev;
      return;
   }
   pending->save_level = templev;

   /* Protect against any errors that might occur while processing the  */
   /* file.  At the first time of trouble we will come back here and    */
   /* bail out completely.                                              */
   if (!setjmp(level))
   {
      err_level = &level;
      len = strlen(rexxmessage->rm_Args[0]);
      if (len >= LINELENGTH) len = LINELENGTH-1;
      memcpy(cmdline+1, rexxmessage->rm_Args[0], len);
      pending->rxmsg = rexxmessage;
      cmdline[0] = len;
      /* Note that we have to save the existing command buffer before   */
      /* we start this mess so that any commands that happen to be      */
      /* there are also processed.                                      */
      execute_extended(0); 
      reply_rexx_command(rexxmessage,msg_num,0,get_msg());
      pending->rxmsg = NULL;
   }

   if (pending->rxmsg != NULL)
   {
      reply_rexx_command(pending->rxmsg,msg_num,0,get_msg());
      pending->rxmsg = NULL;
   }
   poplev();
}

/***
*
* Replies a REXX message, filling in the appropriate codes.  If the macro
* program has requested a result string, the return argstring is allocated
* and installed in the rm_Result2 slot.
*
* A result is returned ONLY IF REQUESTED AND THE PRIMARY RESULT == 0.
*
***/
void reply_rexx_command(rexxmessage,primary,secondary,result)
struct RexxMsg *rexxmessage;
long           primary,secondary;
char           *result;
{
   if(!rexxmessage)return;

   if (primary == ERROR_STRING) primary = 0;
   /* set an error code */
   if (primary == 0 && (rexxmessage->rm_Action & RXFF_RESULT))
   {
     if (RexxSysBase == NULL)
        RexxSysBase = (struct RxsLib *)OpenLibrary(RXSNAME,0L);
      secondary = (result && (RexxSysBase != NULL))
                         ? (long) CreateArgstring(result,strlen(result))
                         : (long) NULL;
      msg_num = 0;
   }
   rexxmessage->rm_Result1 = primary;
   rexxmessage->rm_Result2 = secondary;
   ReplyMsg(rexxmessage);
}

/***
*
* Set the rexx variables to the appropriate values.
* Note that this routine knows that it is called with a buffer that
* it can write into safely.
*
***/
void set_rexx_vars(stem)
char *stem;
{
   int len, i;
   struct LINE *p;

   if (pending->rxmsg == NULL)
      return;

   if (RexxSysBase == NULL)
      RexxSysBase = (struct RxsLib *)OpenLibrary(RXSNAME,0L);

   if (RexxSysBase == NULL)
      return;

   len = *stem++;
   if (len > 32) len = 32;

   /* Uppercase the stem portion */
   for (i = 0; i < len; i++)
      stem[i] = capitalch(stem[i]);

   /* Figure out the current line number */
   i = phys_y;
   if (line_ptr[0] != line_first)
      for(p = prev_list; p != line_first; p = p->prev, i++);

   setnum(stem, len, ".LEFT",      left_margin+1);
   setnum(stem, len, ".RIGHT",     right_margin+1);
   setnum(stem, len, ".TABSTOP",   tab_stop);
   setnum(stem, len, ".LMAX",      line_max);
   setnum(stem, len, ".WIDTH",     screen_width);
   setnum(stem, len, ".X",         phys_x+1);
   setnum(stem, len, ".Y",         phys_y+1);
   setnum(stem, len, ".BASE",      window_base);
   setnum(stem, len, ".EXTEND",    extend_margin);
   setnum(stem, len, ".FORCECASE", forcecase);
   setnum(stem, len, ".LINE",      i+1);

   strcpy(stem+len, ".FILENAME");
   SetRexxVar(pending->rxmsg,stem,file_name, strlen(file_name));

   do_extract();
   strcpy(stem+len, ".CURRENT");
   SetRexxVar(pending->rxmsg,stem,current_line, current_size+1);

   strcpy(stem+len, ".LASTCMD");
   SetRexxVar(pending->rxmsg,stem,lastcmd+1, lastcmd[0]);

   strcpy(stem+len, ".SEARCH");
   SetRexxVar(pending->rxmsg,stem,search_vec+1, search_vec[0]);

   /* pfkeys... keydef */
}

void setnum(stem, len, suffix, val)
char *stem;
int len;
char *suffix;
int val;
{
   char v[15];
   long opts[1];

   opts[0] = val;
   strcpy(stem+len, suffix);
   len = vsprintf(v, "%ld", opts);
   SetRexxVar(pending->rxmsg, stem, v, len);
}
