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
**      $Revision: 1.7 $
**      $Date: 91/03/11 11:14:21 $
**      $Log:	file.c,v $
 * Revision 1.7  91/03/11  11:14:21  J_Toebes
 * Correct parameters for the SetFileDate packet.
 * 
 * Revision 1.6  91/01/15  02:35:09  J_Toebes
 * Correct problem with SetData packet.
 * 
 * Revision 1.5  90/12/31  15:22:46  J_Toebes
 * Implement security restrictions for read/write access and prevention of
 * moving up too many levels.
 * Corrected the code for ACTION_SET_DATE packet
 * 
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
   if ((port = GetRDevice(global, spkt, (ELOCK)spkt->sp_Pkt.dp_Arg1,
                                  spkt->sp_RP->DataP, ATTEMPT_WRITE)) == NULL)
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

   if ((port2 = GetRDevice(global, spkt, (ELOCK)spkt->sp_RP->Arg3,
                            spkt->sp_RP->DataP+FILENAMELEN, ATTEMPT_ACCESS)) == NULL)
   {
      return;
   }
   spkt->sp_Pkt.dp_Arg3 = spkt->sp_Pkt.dp_Arg1;

   if ((port1 = GetRDevice(global, spkt, (ELOCK)spkt->sp_RP->Arg1,
                                   spkt->sp_RP->DataP, ATTEMPT_WRITE)) == NULL)
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
   if ((port = GetRDevice(global, spkt, (ELOCK)spkt->sp_Pkt.dp_Arg2,
                                  spkt->sp_RP->DataP, ATTEMPT_WRITE)) == NULL)
   {
      return;
   }
   spkt->sp_Pkt.dp_Arg2 = spkt->sp_Pkt.dp_Arg1;
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
   if ((port = GetRDevice(global, spkt, (ELOCK)spkt->sp_Pkt.dp_Arg2,
                                  spkt->sp_RP->DataP, ATTEMPT_WRITE)) == NULL)
   {
      return;
   }

   spkt->sp_Pkt.dp_Arg2 = spkt->sp_Pkt.dp_Arg1;
   spkt->sp_Pkt.dp_Arg3 = (LONG)MKBADDR(spkt->sp_RP->DataP);
   spkt->sp_Pkt.dp_Arg4 = (LONG)MKBADDR(comment);

   Dispatch(global, spkt, ACTION_RETURN, port);
}


ACTFN(RmtSetFileDate)
{
   /* Arg1: Unused */
   /* Arg2: BPTR Lock */
   /* Arg3: Name */
   /* Arg4: BPTR DateStamp */
   struct MsgPort *port;
   char *buf;
   
   BUG(("RmtSetFileDate\n"))
   buf = spkt->sp_RP->DataP+sizeof(struct DateStamp);
   
   spkt->sp_RP->DLen = 0;

   if ((port = GetRDevice(global, spkt, (ELOCK)spkt->sp_Pkt.dp_Arg2,
                                  buf, ATTEMPT_WRITE)) == NULL)
   {
      return;
   }

   spkt->sp_Pkt.dp_Arg2 = spkt->sp_Pkt.dp_Arg1;
   spkt->sp_Pkt.dp_Arg3 = (LONG)MKBADDR(buf);
   spkt->sp_Pkt.dp_Arg4 = (LONG)spkt->sp_RP->DataP;

   Dispatch(global, spkt, ACTION_RETURN, port);
}
