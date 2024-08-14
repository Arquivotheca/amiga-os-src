/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1988 The Software Distillery.  All Rights Reserved *
* |. o.| ||	     Written by Doug Walker				    *
* | .  | ||	     The Software Distillery				    *
* | o  | ||	     235 Trillingham Lane				    *
* |  . |//	     Cary, NC 27513					    *
* ======	     BBS:(919)-471-6436                                     *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
**      $Filename: dispatch.c $ 
**      $Author: J_Toebes $
**      $Revision: 1.6 $
**      $Date: 90/11/15 08:17:36 $
**      $Log:	dispatch.c,v $
 * Revision 1.6  90/11/15  08:17:36  J_Toebes
 * Correct multiple bugs associated with new version.  Sessions still are not supported.
 * 
 * Revision 1.5  90/11/05  06:56:07  J_Toebes
 * Major rewrite to make server multi-threaded with support for multiple
 * devices.  At this point in time it is not debugged and the concept of
 * sessions is missing.
 * 
 * Revision 1.4  90/11/04  17:36:27  J_Toebes
 * First major rewrite for asynchronous server
 * 
**
**/

#include "server.h"

void
Dispatch(global, spkt, nextaction, port)
GLOBAL global;
SPACKET *spkt;
int nextaction;
struct MsgPort *port;
{
   BUG(("Dispatch: Entry - spkt = 0x%08lx port 0x%08lx\n", spkt, port))

   /* This must be done each time because it is ping-ponged by the send packet  */
   /* mechanism.                                                                */
   spkt->sp_Pkt.dp_Port = global->n.port;

   if (port == NULL)
   {
      BUG(("********Dispatch: Invalid Device!\n"))
      BUGR("Invalid Device!");
      spkt->sp_RP->Arg1 = DOSFALSE;
      spkt->sp_RP->Arg2 = ERROR_OBJECT_NOT_FOUND;
   }
   else
   {
      spkt->sp_Action = nextaction;
      global->n.reply = 0;

      PutMsg(port, (struct Message *)spkt);
   }

   if (global->dosport != NULL)
   {
      /* If we did a OpenDeviceProc to get here, we need to free it */
      global->dosport = NULL;
   }
}

ACTFN(RmtReturn)
{
   spkt->sp_RP->Arg1 = spkt->sp_Pkt.dp_Res1;
   spkt->sp_RP->Arg2 = spkt->sp_Pkt.dp_Res2;
}
