/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1990 The Software Distillery.                    *
* |. o.| ||          All Rights Reserved                                  *
* | .  | ||          Written by John Toebes and Doug Walker               *
* | o  | ||          The Software Distillery                              *
* |  . |//           207 Livingstone Drive                                *
* ======             Cary, NC 27513                                       *
*                    BBS:(919)-460-7430                                   *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
**      $Filename: SDNLocalPut.c $ 
**      $Author: J_Toebes $
**      $Revision: 1.2 $
**      $Date: 91/01/06 20:56:03 $
**      $Log:	SDNLocalPut.c,v $
 * Revision 1.2  91/01/06  20:56:03  J_Toebes
 * Correct strange bug where packets were being received by the local
 * sender.  This was due to overloading the Port field in the global structure.
 * 
**/

#include "netcomm.h"
#include "sdnlocal.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNReply                                                           */
/*                                                                             */
/* PURPOSE: Reply to a previously obtained message                             */
/*                                                                             */
/* SYNOPSIS:                                                                   */
/*    rc = SDNReply(drglobal, RP);                                             */
/*                                                                             */
/*                                                                             */
/*    drglobal  APTR              (in)     Driver-specific global data pointer */
/*                                                                             */
/*                                                                             */
/*    RP        struct RPacket *  (in)     Data to send                        */
/*                                                                             */
/*    rc        int               (ret)    SDN_ERR, SDN_OK,                    */
/*                                         SDN_PEND == write not yet complete  */
/*                                                                             */
/* NOTES:                                                                      */
/*    If SDN_PEND is returned, the handler must not return the application's   */
/*       packet until a the driver signals that the write has completed.       */
/*       When the write completes, a signal bit will be set and PluckRPacket   */
/*       will be called.  SDNReply will return a special packet type indicating*/
/*       indicating the write has completed.                                   */
/*    Calling this routine automatically does a FreeRPacket on the packet.     */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int LIBENT LOCALSDNReply        (register __a0 LOCALGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   ReplyMsg(&(RP2LRP(rp))->ms);
   return(SDN_OK);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNSend                                                            */
/*                                                                             */
/* PURPOSE: Send a packet to a specific remote node                            */
/*                                                                             */
/* SYNOPSIS:                                                                   */
/*    rc = SDNSend(drglobal, rp);                                              */
/*                                                                             */
/*                                                                             */
/*    drglobal  APTR              (in)     Driver-specific global data pointer */
/*                                                                             */
/*                                                                             */
/*    rp        struct RPacket *  (in)     Data to send                        */
/*                                                                             */
/*    rc        int               (ret)    SDN_ERR, SDN_OK,                    */
/*                                         SDN_PEND == write not yet complete  */
/*                                                                             */
/* NOTES:                                                                      */
/*    If SDN_PEND is returned, the handler must not return the application's   */
/*       packet until a the driver signals that the write has completed.       */
/*       When the write completes, a signal bit will be set and SDNSend        */
/*       will be called.  SDNSend will return a special packet type            */
/*       indicating the write has completed.                                   */
/*                                                                             */
/*    The packet is allocated by the NET: code and freed by the driver code    */
/*       when the I/O is complete                                              */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int LIBENT LOCALSDNSend         (register __a0 LOCALGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   struct LOCALRPacket *irp = RP2LRP(rp);

   irp->rp.signature = SIGVALUE;
   if (tg->sendport == NULL)
   {
      if ((tg->sendport = FindPort(tg->portname)) == NULL)
         return(SDN_ERR);
   }

   irp->ms.mn_ReplyPort = tg->replyport;
   PutMsg(tg->sendport, &irp->ms);

   return(SDN_OK);
}
