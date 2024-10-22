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
**      $Filename: SDNLocalGet.c $ 
**      $Author: J_Toebes $
**      $Revision: 1.2 $
**      $Date: 91/01/06 20:56:41 $
**      $Log:	SDNLocalGet.c,v $
 * Revision 1.2  91/01/06  20:56:41  J_Toebes
 * Correct strange bug where packets were being received by the local
 * sender.  This was due to overloading the Port field in the global structure.
 * 
**/

#include "netcomm.h"
#include "sdnlocal.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNAccept                                                          */
/*                                                                             */
/* PURPOSE: Read the next incoming packet and accept new connections           */
/*                                                                             */
/* SYNOPSIS:                                                                   */
/*    rc = SDNAccept(drglobal, RP);                                            */
/*                                                                             */
/*    drglobal  APTR              (in)     Driver-specific global data pointer */
/*                                                                             */
/*                                                                             */
/*    RP        struct RPacket ** (out)    Packet that has arrived             */
/*                                                                             */
/*    rc        int               (ret)    SDN_OK, SDN_ERR == read error,      */
/*                                         SDN_NONE == nothing to read         */
/*                                                                             */
/* NOTES:                                                                      */
/*    Driver must be prepared to handle this being called when nothing is      */
/*       pending since multiple drivers may be sharing a signal bit.  If a     */
/*       packet did come in, but is munged, SDN_ERR is returned;  if no packet */
/*       was available, SDN_NONE is returned.  If a pending write request has  */
/*       been completed, a special packet type is returned and a copy of the   */
/*       original RPacket structure's 'statedata' pointer is returned.         */
/*                                                                             */
/*    The memory pointed to by RP is owned by the NET: code                    */
/*       until FreeRPacket is called.  FreeRPacket should be called ASAP after */
/*       calling PluckRPacket to allow the driver to reuse the RPacket for the */
/*       next read request.                                                    */
/*                                                                             */
/*    When a new connection is established, this routine will return a filled  */
/*       in RPACKET with an action indicating the establishment of the         */
/*       connection.  For connections that are broken, a phoney rpacket is     */
/*       also returned.                                                        */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int LIBENT LOCALSDNAccept       (register __a0 LOCALGLOBAL tg,
                                register __a1 struct RPacket **rp,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   struct LOCALRPacket *lrp;

   *rp = NULL;

   /* Handle the one special case of letting them know when we have a connection */
   if (tg->state == CSTATE_CONNECT)
   {
      *rp = SDNAllocRPacket((APTR)tg, NULL, NETBUFSIZE);
      if (*rp == NULL)
         return(SDN_ERR);

      (*rp)->Type = ACTION_NETWORK_START;
      (*rp)->Arg1 = 1;
      tg->state = CSTATE_ACTIVE;
      return(SDN_OK);
   }

   /* Process and free any RPackets that have been returned to us */
   if (tg->replyport != NULL)
   {
      if ((lrp = (struct LOCALRPacket *)GetMsg(tg->replyport)) != NULL)
      {
         if (lrp->rp.DataP != lrp->rp.AllocP)
         {
            memcpy(lrp->rp.AllocP, lrp->rp.DataP, lrp->rp.DLen);
            lrp->rp.DataP = lrp->rp.AllocP;
         }
         *rp = LRP2RP(lrp);
         return(SDN_OK);
      }
   }

   if (tg->port == NULL)
   {
      if (tg->replyport == NULL)
         return(SDN_ERR);
      return(SDN_NONE);
   }

   if ((lrp = (struct LOCALRPacket *)GetMsg(tg->port)) == NULL)
      return(SDN_NONE);

   if (lrp->rp.DataP != lrp->rp.AllocP)
   {
      memcpy(lrp->rp.AllocP, lrp->rp.DataP, lrp->rp.DLen);
      lrp->rp.DataP = lrp->rp.AllocP;
   }
   *rp = LRP2RP(lrp);
   return(SDN_OK);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNReceive                                                         */
/*                                                                             */
/* PURPOSE: Get the next response packet                                       */
/*                                                                             */
/* SYNOPSIS:                                                                   */
/*    rc = SDNReceive(drglobal, mask, RP);                                     */
/*                                                                             */
/*    drglobal  APTR              (in)  Driver-specific global data pointer    */
/*                                                                             */
/*    mask      ULONG             (in)  Mask of signal bits that have gone off */
/*                                                                             */
/*    RP        struct RPacket ** (out) RPacket that has arrived               */
/*                                                                             */
/*    rc        int               (ret) SDN_OK, SDN_ERR == read error,         */
/*                                      SDN_NONE == nothing to read            */
/*                                                                             */
/* NOTES:                                                                      */
/*    Driver must be prepared to handle this being called when nothing is      */
/*       pending since multiple drivers may be sharing a signal bit.  If a     */
/*       packet did come in, but is munged, SDN_ERR is returned;  if no packet */
/*       was available, SDN_NONE is returned.  If a pending write request has  */
/*       been completed, a special packet type is returned and a copy of the   */
/*       original RPacket structure's 'statedata' pointer is returned.         */
/*                                                                             */
/*    The memory pointed to by RP is owned by the NET: code                    */
/*       until FreeRPacket is called.  FreeRPacket should be called ASAP after */
/*       calling SDNReceive to allow the driver to reuse the RPacket for the   */
/*       next read request.                                                    */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int LIBENT LOCALSDNReceive      (register __a0 LOCALGLOBAL tg,
                                register __d0 ULONG mask,
                                register __a1 struct RPacket **rp,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   return(SDNAccept((APTR)tg, rp));
}
