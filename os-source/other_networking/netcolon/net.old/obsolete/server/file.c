/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987, 1988 The Software Distillery.  All Rights  */
/* |. o.| || Reserved.	This program may not be distributed without the    */
/* | .	| || permission of the authors: 			   BBS:    */
/* | o	| ||   John Toebes     Doug Walker    Dave Baker		   */
/* |  . |//								   */
/* ======								   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* File Manipulation  */
/*  ActDelete ActRename ActSetProtection ActSetComment */
/*
**      $Filename: file.c $ 
**      $Author: J_Toebes $
**      $Revision: 1.4 $
**      $Date: 90/11/18 22:54:18 $
**      $Log:	file.c,v $
 * Revision 1.4  90/11/18  22:54:18  J_Toebes
 * Corrected bug in setfilecomment failing to use the correct argument.
 * 
 * Revision 1.3  90/11/05  06:55:21  J_Toebes
 * Major rewrite to make server multi-threaded with support for multiple
 * devices.  At this point in time it is not debugged and the concept of
 * sessions is missing.
 * 
**
**/

#include "server.h"

ACTFN(RmtDelete)
{
   /* Arg1: Lock        */
   /* Arg2: Name        */
   struct MsgPort *port;
   
   BUG(("RmtDelete, lock %lx\n", spkt->sp_RP->Arg1));
   BUGBSTR("File to delete is ", spkt->sp_RP->DataP);

   spkt->sp_RP->DLen = 0;
   if ((port = GetRDevice(global, spkt, spkt->sp_Pkt.dp_Arg1,
                                  spkt->sp_RP->DataP)) == NULL)
   {
      return;
   }
   spkt->sp_Pkt.dp_Arg2 = (LONG)MKBADDR(spkt->sp_RP->DataP);

   Dispatch(global, spkt, ACTION_RETURN, port);
}

ACTFN(RmtRename)
{
   /* Arg1: FromLock    */
   /* Arg2: FromName   in DataP */
   /* Arg3: ToLock      */
   /* Arg4: ToName     in DataP+FILENAMELEN */
   struct MsgPort *port1, *port2;

   BUG(("RmtRename\n"));
   BUGBSTR("Renaming ", spkt->sp_RP->DataP);
   BUGBSTR("New Name ", spkt->sp_RP->DataP+FILENAMELEN);

   spkt->sp_Pkt.dp_Arg2 = (LONG)MKBADDR(spkt->sp_RP->DataP);
   spkt->sp_Pkt.dp_Arg4 = (LONG)MKBADDR(spkt->sp_RP->DataP+FILENAMELEN);
   spkt->sp_RP->DLen    = 0;

   if (((port1 = GetRDevice(global, spkt, spkt->sp_Pkt.dp_Arg1,
                                   spkt->sp_RP->DataP)) == NULL) ||
       ((port2 = GetRDevice(global, spkt, spkt->sp_Pkt.dp_Arg3,
                                   spkt->sp_RP->DataP+FILENAMELEN)) == NULL))
   {
      return;
   }

   if (port1 != port2)
   {
      spkt->sp_RP->Arg1 = DOS_FALSE;
      spkt->sp_RP->Arg2 = ERROR_RENAME_ACROSS_DEVICES;
      return;
   }

   Dispatch(global, spkt, ACTION_RETURN, port1);
}

ACTFN(RmtSetProtection)
{
   /* Arg1: Unused */
   /* Arg2: Lock */
   /* Arg3: Name */
   /* Arg4: Mask of protection */
   struct MsgPort *port;

   BUG(("RmtSetProtection\n"));
   BUGBSTR("File to protect: ", spkt->sp_RP->DataP);

   spkt->sp_RP->DLen = 0;
   if ((port = GetRDevice(global, spkt, spkt->sp_Pkt.dp_Arg2,
                                  spkt->sp_RP->DataP)) == NULL)
   {
      return;
   }
   spkt->sp_Pkt.dp_Arg3 = (LONG)MKBADDR(spkt->sp_RP->DataP);

   Dispatch(global, spkt, ACTION_RETURN, port);
}

ACTFN(RmtSetComment)
{
   /* Arg1: Unused */
   /* Arg2: Lock */
   /* Arg3: Name */
   /* Arg4: Comment */
   /* DEBUG THIS ONE WELL! */
   char *comment;
   int len;
   struct MsgPort *port;

   BUG(("RmtSetComment\n"));

   len = (BSTRLEN(spkt->sp_RP->DataP)+4) & ~3;
   comment = ((char *)spkt->sp_RP->DataP)+len;

   BUGBSTR("File to Comment: ", spkt->sp_RP->DataP);
   BUGBSTR("Comment Text: ",    comment);

   spkt->sp_RP->DLen = 0;
   if ((port = GetRDevice(global, spkt, spkt->sp_Pkt.dp_Arg2,
                                  spkt->sp_RP->DataP)) == NULL)
   {
      return;
   }

   spkt->sp_Pkt.dp_Arg3 = (LONG)MKBADDR(spkt->sp_RP->DataP);
   spkt->sp_Pkt.dp_Arg4 = (LONG)MKBADDR(comment);

   Dispatch(global, spkt, ACTION_RETURN, port);
}


ACTFN(RmtSetFileDate)
{
   /* Arg1: Unused */
   /* Arg2: BPTR Lock */
   /* Arg3: Name (BPTR to BSTR) /
   /* Arg4: APTR (!) to DateStamp */
   int len;
   struct MsgPort *port;
   BUG(("RmtSetFileDate\n"))

   spkt->sp_Pkt.dp_Arg3 = MKBADDR(spkt->sp_RP->DataP);
   spkt->sp_RP->DLen = 0;

   if ((port = GetRDevice(global, spkt, spkt->sp_Pkt.dp_Arg1,
                                  spkt->sp_RP->DataP)) == NULL)
   {
      return;
   }
   len = (BSTRLEN(spkt->sp_RP->DataP)+4) & ~3;
   spkt->sp_Pkt.dp_Arg4 = (LONG)(spkt->sp_RP->DataP+len);  /* NOT A BADDR!!!! */

   Dispatch(global, spkt, ACTION_RETURN, port);
}
