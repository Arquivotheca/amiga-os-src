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
int LIBENT INETSDNAccept       (register __a0 INETGLOBAL tg,
                                register __a1 struct RPacket **rp,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   struct INETRPacket *irp;
   int curfh;
   int i, len;
   struct selectargs sa;

   *rp = NULL;

   if (tg->fhscurmask == 0)
   {
      /* There are no unprocessed messages, just ask if we have anything to do */
      
      tg->fhscurmask = tg->fhsmask;
      sa.socks  = tg->_socks;         /* array of socket ptrs              */
      sa.rd_set = (fd_set *)&tg->fhscurmask; /* sockets considered for read event */
      sa.wr_set = 0;                  /* sockets considered for write event*/
      sa.ex_set = 0;                  /* sockets considered for ex events  */
      sa.task   = FindTask(0L);       /* task to signal when selecting     */
      sa.sigbit = tg->socket_sigio;   /* signal to send when selecting     */
      sa.numfds = 32;                 /* number of fds to consider         */
      sa.errno  = 0;                  /* return error value                */
      sa.rval   = 0;                  /* number of fds found ready         */

      SelectAsm(&sa);

      if (sa.errno)
      {
         /* There was some sort of error.  We really don't care, just let  */
         /* them know that it failed.                                      */
         return(SDN_ERR);
      }
   
      if (!sa.rval)
      {
         /* Nothing really happened, just let them know */
         tg->fhscurmask = 0;
         return(SDN_NONE);
      }
   }

   /* We have something that has happened on a connection               */
   /* Find out what it is.  We will give first attack to anyone that    */
   /* is establishing a new connection.                                 */
   if (tg->fhscurmask & tg->sockmask)
   {
      /* Reset the flag to indicate that we have processed it           */
      tg->fhscurmask &= ~tg->sockmask;

      if ((irp = (struct INETRPacket *)SDNAllocRPacket((APTR)tg, NULL, NETBUFSIZE)) == NULL)
      {
         BUG(("Oops, out of memory\n"));
         return(SDN_ERR);
      }
      
      /* We have a new connection */
      BUG(("Special event\n"));

      /* Find us an empty slot to hold the connection */
      for(i = 0; i < MAXCONNECT; i++)
         if (tg->connect[i].usage == -1)
            break;

      if (i >= MAXCONNECT)
      {
         BUG(("Too many connections, ignoring it\n"));
         SDNFreeRPacket((APTR)tg, irp);
      }
      else
      {
         len = sizeof(&tg->sin);
         tg->connect[i].fh = iaccept(tg, tg->sock, &tg->sin, &len);
         if (tg->connect[i].fh == -1)
         {
            BUGR("Accept Error");
            SDNFreeRPacket((APTR)tg, irp);
         }
         else
         {
            kprintf("Connect Socket=%ld 0x%08lx\n",
                         tg->connect[i].fh, tg->_socks[tg->connect[i].fh]);
            tg->fhsmask |= INETMASK(tg->connect[i].fh);
            BUG(("New connection %ld on: port=%ld Addr=%08lx\n",
                  i, tg->sin.sin_port, tg->sin.sin_addr.s_addr));
            tg->connect[i].usage = 1;
            tg->connect[i].opack = 0;

            /* Now Fabricate the new connection packet */
            irp->con = &tg->connect[i];
            irp->rp.Type = ACTION_NETWORK_START;
            irp->rp.Arg1 = (LONG)irp->con;
            *rp = &irp->rp;
            SDNSetConData((APTR)tg, *rp, NULL);
            return(SDN_OK);
         }
      }
   }
   
   /* No new connections, but someone came in with a request */
   for(i = 0; i < MAXCONNECT; i++)
   {
      if ((tg->connect[i].usage != -1) &&
          (tg->fhscurmask & INETMASK(tg->connect[i].fh)))
      {
         if ((irp = (struct INETRPacket *)SDNAllocRPacket((APTR)tg, NULL, NETBUFSIZE)) == NULL)
         {
            BUG(("Oops, out of memory\n"));
            return(SDN_ERR);
         }

         irp->con = &tg->connect[i];
         curfh = irp->con->fh;

         /* This is the one that sent us a message */
         /* Reset the flag so that we know that we have processed it    */
         tg->fhscurmask &= ~INETMASK(curfh);
         BUG(("AcceptRPacket: reading %d from Connection %ld. . .", RPSIZEN, i));
         irp->rp.DataP[0] = '\0';
         len = readn(tg, curfh, (char *)irp, RPSIZEN);

         /* DO a sanity check on what we get.  If it is even slightly wrong  */
         /* we should immediately terminate the connection.  If it was a     */
         /* legitimate user, he will have to reestablish the connection.     */
         /* Note that this will cause any outstanding locks or file handles  */
         /* to be purged automatically.                                      */
         if ((len != RPSIZEN)       ||
             (irp->rp.DLen > NETBUFSIZE) ||
             (irp->rp.DLen < 0)          ||
             ((irp->rp.DLen != 0) &&
              (readn(tg, curfh, irp->rp.DataP, irp->rp.DLen) != irp->rp.DLen)))
         {
            /* Something is wrong here, assume that the connection */
            /* is now closed and get rid of it.                    */
            tg->fhsmask ^= INETMASK(curfh);
            iclose(tg, curfh);

            /* We also need to remove the dead connection from the table of  */
            /* open connections.                                             */
            tg->connect[i].usage = -1; /* Indicate it is available           */
            
            /* Now go through and mark all outstanding rpackets for this     */
            /* connection as dead.                                           */
            /* !!!!!!!!!!!!!!!!!!!!!!                                        */

            /* We need to return a packet indicating that the connection     */
            /* has now been broken                                           */
            irp->rp.Type = ACTION_NETWORK_TERM;
            irp->rp.Arg1 = (LONG)irp->con->data;
            irp->rp.DLen = 0;
         }
         *rp = &irp->rp;
         irp->con->usage++;
         return(SDN_OK);
      }
   }
   return(SDN_NONE);
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
int LIBENT INETSDNReceive      (register __a0 INETGLOBAL tg,
                                register __d0 ULONG mask,
                                register __a1 struct RPacket **rp,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   int rc;
   long sockmask, fhscurmask;

   fhscurmask = tg->fhscurmask;
   if ((sockmask = tg->sockmask) != 0)
   {
      tg->fhscurmask &= ~sockmask;
      tg->fhsmask    &= ~sockmask;
   }
   rc = SDNAccept((APTR)tg, rp);
   tg->fhscurmask |= fhscurmask & sockmask;
   tg->fhsmask    |= sockmask;
   return(rc);
}
