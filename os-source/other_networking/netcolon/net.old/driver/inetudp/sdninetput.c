/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1990 The Software Distillery.                    *
* |. o.| ||          All Rights Reserved                                  *
* | .  | ||          Written by John Toebes and Doug Walker               *
* | o  | ||          The Software Distillery                              *
* |  . |//           207 Livingstone Drive                                *
* ======             Cary, NC 27513                                       *
*                    BBS:(919)-460-7430                                   *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "netcomm.h"
#include "sdninet.h"

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
int LIBENT INETSDNReply        (register __a0 INETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   struct INETRPacket *irp = (struct INETRPacket *)rp;
   int rc;

   rc = SDNSend((APTR)tg, rp);
   return(rc);
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
int LIBENT INETSDNSend         (register __a0 INETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   struct INETRPacket *irp = (struct INETRPacket *)rp;

   BUG(("SDNSend: Con: %08lx type %d, Args %lx %lx %lx %lx\n", 
       irp->con->addr, irp->rp.Type, 
       irp->rp.Arg1, irp->rp.Arg2, 
       irp->rp.Arg3, irp->rp.Arg4));

   BUG(("SDNSend: writing %d to confd %ld. . .", RPSIZEN, irp->con->addr));
   irp->rp.signature = SIGVALUE;
   if (writen(tg, irp->con->addr, (char *)rp, RPSIZEN) != RPSIZEN)
   {
      /* Something is really bad here - I bet they closed the connection        */
      /* We need to return a failure.                                           */
      BUGR("NET write error")
      return(SDN_ERR);
   }

   BUG(("%ld written\n", RPSIZEN));

   if(irp->rp.DLen)
   {
      BUG(("SDNSend: writing %d to confd %ld. . .", irp->rp.DLen, irp->con->addr));
      if (writen(tg, irp->con->addr, irp->rp.DataP, irp->rp.DLen) != irp->rp.DLen)
      {
        BUGR("NET write error");
        return(SDN_ERR) ;
      }
      BUG(("%d written\n", irp->rp.DLen));
   }
   SDNFreeRPacket((APTR)tg, &irp->rp);

   return(SDN_OK);
}
