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
**      $Filename: SDNLocalUtil.c $ 
**      $Author: J_Toebes $
**      $Revision: 1.3 $
**      $Date: 91/01/06 20:57:13 $
**      $Log:	SDNLocalUtil.c,v $
 * Revision 1.3  91/01/06  20:57:13  J_Toebes
 * Correct strange bug where packets were being received by the local
 * sender.  This was due to overloading the Port field in the global structure.
 * 
**/

#include "netcomm.h"
#include "sdnlocal.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNSerInit                                                         */
/*                                                                             */
/* PURPOSE: Perform Server Specific Initialization                             */
/* SYNOPSIS:                                                                   */
/*   rc = SDNSerInit(drglobal, mask, data);                                    */
/*                                                                             */
/*     drglobal     APTR *      (out)    Server-specific global data pointer   */
/*                                                                             */
/*     mask         ULONG *     (in/out) On input, mask of signal bits already */
/*                                          being used by all other drivers;   */
/*                                       On output, mask of signal bits this   */
/*                                          driver will be using.              */
/*     data         char *      (in)     Pointer to driver specific            */
/*                                                Initialization string.       */
/*                                                                             */
/*     rc           int         (ret)    SDN_ERR, SDN_OK                       */
/*                                                                             */
/*   NOTES:                                                                    */
/*      Driver is free to reuse signal bits or allocate new ones.              */
/*      Called by the server to initialize communications.  A signal bit should*/
/*         be set when a new connection is detected or when a pending read or  */
/*         write completes.                                                    */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int LIBENT LOCALSDNSerInit      (register __a0 LOCALGLOBAL *tgp,
                                register __a1 ULONG *maskp,
                                register __a2 char *data,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   LOCALGLOBAL tg;

   *tgp = NULL;

   if ((tg = LocalInit(maskp, data)) == NULL)
      return(SDN_ERR);

   tg->DriverBase       = DriverBase;

   /* If the port already exists then we can not start the server.       */
   if (tg->sendport != NULL)
   {
      FreeMem((void *)tg, sizeof(*tg));
      return(SDN_ERR);
   }
   
   tg->port = (struct MsgPort *)
              AllocMem(sizeof(struct MsgPort), MEMF_CLEAR|MEMF_PUBLIC);
   if (tg->port == NULL)
   {
      FreeMem((void *)tg, sizeof(*tg));
      return(SDN_ERR);
   }
   tg->port->mp_Node.ln_Name = tg->portname;
   tg->port->mp_Node.ln_Pri = 0;
   tg->port->mp_Node.ln_Type = NT_MSGPORT;
   tg->port->mp_Flags = PA_SIGNAL;
   tg->port->mp_SigBit = tg->signalbit;
   tg->port->mp_SigTask = FindTask(0L);
   AddPort(tg->port);

   *tgp = tg;

   tg->state = CSTATE_CONNECT;
   return(SDN_OK);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNSerTerm                                                         */
/*                                                                             */
/* PURPOSE: Perform Server Specific Termination                                */
/* SYNOPSIS:                                                                   */
/*   rc = SDNSerTerm(drglobal);                                                */
/*                                                                             */
/*    drglobal     APTR        (in)     Driver-specific global data pointer    */
/*                                                                             */
/* NOTES:                                                                      */
/*    Driver must not free ANY signal bits, even if it allocated them.         */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int LIBENT LOCALSDNSerTerm      (register __a0 LOCALGLOBAL tg,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   if (tg->port != NULL)
   {
      RemPort(tg->port);
      FreeMem(tg->port, sizeof(struct MsgPort));
   }
   SDNHanTerm((APTR)tg);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNHanInit                                                         */
/*                                                                             */
/* PURPOSE: Perform Server Specific Initialization                             */
/* SYNOPSIS:                                                                   */
/* rc = SDNHanInit(drglobal, mask, data);                                      */
/*                                                                             */
/*    drglobal     APTR *        (out)    Driver-specific global data pointer  */
/*                                                                             */
/*    mask         ULONG *       (in/out) On input, mask of signal bits already*/
/*                                           being used by all other drivers;  */
/*                                        On output, mask of signal bits this  */
/*                                           driver will be using.             */
/*    data         char *        (in)     Pointer to driver specific           */
/*                                           Initialization string.            */
/*                                                                             */
/*    rc           int           (ret)    SDN_ERR, SDN_OK                      */
/*                                                                             */
/* NOTES:                                                                      */
/*    Driver is free to reuse signal bits or allocate new ones.                */
/*                                                                             */
/*    Called by handler to initialize communication.  A signal bit should be   */
/*       set when a pending write completes and when a pending read completes. */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int LIBENT LOCALSDNHanInit      (register __a0 LOCALGLOBAL *tgp,
                                register __a1 ULONG *maskp,
                                register __a2 char *data,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   LOCALGLOBAL tg;

   *tgp = NULL;

   if ((tg = LocalInit(maskp, data)) == NULL)
      return(SDN_ERR);

   tg->DriverBase       = DriverBase;

   tg->replyport = (struct MsgPort *)
              AllocMem(sizeof(struct MsgPort), MEMF_CLEAR|MEMF_PUBLIC);
   if (tg->replyport == NULL)
   {
      FreeMem((void *)tg, sizeof(*tg));
      return(SDN_ERR);
   }
   tg->replyport->mp_Node.ln_Type = NT_MSGPORT;
   tg->replyport->mp_Flags = PA_SIGNAL;
   tg->replyport->mp_SigBit = tg->signalbit; /* Note shared signal bit */
   tg->replyport->mp_SigTask = FindTask(0L);
   NewList(&(tg->replyport->mp_MsgList));

   *tgp = tg;

   return(SDN_OK);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: LocalInit                                                          */
/*                                                                             */
/* PURPOSE: Perform Local Initializaion                                        */
/* SYNOPSIS:                                                                   */
/* drgloabl = LocalInit(drglobal, mask, data);                                 */
/*                                                                             */
/*    drglobal     LOCALGLOBAL   (out)    Driver-specific global data pointer  */
/*                                                                             */
/*    mask         ULONG *       (in/out) On input, mask of signal bits already*/
/*                                           being used by all other drivers;  */
/*                                        On output, mask of signal bits this  */
/*                                           driver will be using.             */
/*    data         char *        (in)     Pointer to driver specific           */
/*                                           Initialization string.            */
/*                                                                             */
/* NOTES:                                                                      */
/*    Driver is free to reuse signal bits or allocate new ones.                */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
LOCALGLOBAL LocalInit(maskp, data)
ULONG *maskp;
char *data;
{
   LOCALGLOBAL tg;
   int i;
   struct LOCALRPacket *ip;
#undef SysBase
   struct Library *SysBase = ABSEXECBASE;

   *maskp = 0;

   if(!(tg = (void *)AllocMem(sizeof(*tg), MEMF_CLEAR)))
   {
      return(NULL);
   }

   tg->tg_SysBase       = SysBase;

   tg->signalbit = AllocSignal(-1);
   if (tg->signalbit == -1)
   {
      if (!*maskp)
      {
         return(NULL);
      }
      for (i = 0; i < 32; i++)
         if (*maskp & (1L << i)) break;
      tg->signalbit = i;
   }

   /* Allocate our pool of 10 RPackets */
   if ((tg->ppack = (struct LOCALRPacket *)
                    AllocMem(DEFPACKETS*sizeof(struct LOCALRPacket), MEMF_CLEAR)) == NULL)
   {
      FreeMem((void *)tg, sizeof(*tg));
      return(NULL) ;
   }

   for (ip = tg->ppack, i=0; i < DEFPACKETS; i++)
   {
      ip->next = ip+1;
      ip->class = PACKET_FREE;
      ip++;
   }
   ip--;
   ip->next = NULL;

   if (data)
   {
      while(*data == ' ') data++;
      i = 0;
      while(*data > ' ')
      {
         if (i < MAXNAME) tg->portname[i++] = *data;
         data++;
      }
      tg->portname[i] = 0; /* Null terminate the name */
      if (i == 0)
      {
         strcpy(tg->portname, "LOCAL_NET_PORT");
      }
   }

   tg->sendport = FindPort(tg->portname);
   *maskp = 1L << tg->signalbit;
   return(tg);
}

#define SysBase    (tg->tg_SysBase)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNHanTerm                                                         */
/*                                                                             */
/* PURPOSE: Terminate all resources associated with a handler                  */
/* SYNOPSIS:                                                                   */
/*                                                                             */
/* SDNHanTerm(drglobal);                                                       */
/*                                                                             */
/*    drglobal     APTR          (in)     Driver-specific global data pointer  */
/*                                                                             */
/* NOTES:                                                                      */
/*    Driver must not free ANY signal bits, even if it allocated them.         */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int LIBENT LOCALSDNHanTerm      (register __a0 LOCALGLOBAL tg,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   if(tg != NULL)
   {
      FreeMem((void *)tg, sizeof(*tg));
   }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNAllocRPacket                                                    */
/*                                                                             */
/* PURPOSE: Allocate an RPacket for general use                                */
/*                                                                             */
/* SYNOPSIS:                                                                   */
/*    RP = SDNAllocRPacket(drglobal);                                          */
/*                                                                             */
/*    drglobal  APTR              (in)     Driver-specific global data pointer */
/*                                                                             */
/*    RP        struct RPacket *  (out)    created RPacket (or NULL for error) */
/*                                                                             */
/* NOTES:                                                                      */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
struct RPacket *
    LIBENT LOCALSDNAllocRPacket (register __a0 LOCALGLOBAL tg,
                                register __a1 APTR con,
                                register __d0 int len,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   struct LOCALRPacket *ip;

   for (ip = tg->ppack; ip != NULL && ip->class != PACKET_FREE; ip = ip->next);
   if (ip != NULL)
   {
      ip->class = PACKET_PERM;
   }
   else
   {
      /* Have to allocate a temporary one from system storage */
      ip = (struct LOCALRPacket *)AllocMem(sizeof(*ip), 0);
      if (ip == NULL) return(NULL);
      ip->next = tg->tpack;
      ip->class = PACKET_TEMP; /* mark it to be freed later     */
      tg->tpack = ip;
      ip->rp.AllocP = NULL;
   }
   ip->rp.DataP = ip->rp.AllocP;
   if (ip->rp.DataP == NULL)
   {
      ip->rp.AllocP = ip->rp.DataP = AllocMem(NETBUFSIZE, 0);
      if (ip->rp.AllocP == NULL)
      {
         SDNFreeRPacket((APTR)tg, LRP2RP(ip));
         ip = NULL;
      }
   }
   return(LRP2RP(ip));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNDupRPacket                                                      */
/*                                                                             */
/* PURPOSE: Allocate a second RPacket on the same connection                   */
/*                                                                             */
/* SYNOPSIS:                                                                   */
/*    NewRP = SDNDupRPacket(drglobal, RP);                                     */
/*                                                                             */
/*    drglobal  APTR              (in)     Driver-specific global data pointer */
/*                                                                             */
/*    RP        struct RPacket *  (in)     RPacket to duplicate                */
/*                                                                             */
/*    NewRP     struct RPacket *  (ret)    Duplicated RPacket (NULL if error)  */
/*                                                                             */
/* NOTES:                                                                      */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
struct RPacket *
    LIBENT LOCALSDNDupRPacket   (register __a0 LOCALGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   struct RPacket *newrp;

   newrp = SDNAllocRPacket((APTR)tg, NULL, NETBUFSIZE);
   if (newrp != NULL)
   {
      /* Actually, this is wrong, we need to copy less, but we aren't using it  */
      /* for now...                                                             */
      memcpy(RP2LRP(newrp), rp, sizeof(struct LOCALRPacket));
   }

   return(newrp);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNFreeRPacket                                                     */
/*                                                                             */
/* PURPOSE: Indicate that the driver may reuse a previously issued RPacket     */
/*                                                                             */
/* SYNOPSIS:                                                                   */
/*    SDNFreeRPacket(drglobal, RP);                                            */
/*                                                                             */
/*    drglobal  APTR              (in)     Driver-specific global data pointer */
/*                                                                             */
/*    RP        struct RPacket *  (in)     RPacket struct returned from        */
/*                                         PluckRPacket/AcceptRPacket          */
/*                                                                             */
/* NOTES:                                                                      */
/*    This routine must be called before the driver is allowed to reuse an     */
/*       RPacket structure.                                                    */
/*                                                                             */
/*    Both handler and server can call this routine.                           */
/*                                                                             */
/*    FreeRPacket should be called ASAP on any packets returned from           */
/*    PluckRPacket or AcceptRPacket since the returned packets are really      */
/*    driver-allocated input buffers;  holding on to them may result in lots   */
/*    of memory being allocated to the driver for buffering.  The server, in   */
/*    particular, must call FreeRPacket BEFORE calling Dispatch() to process   */
/*    the packet.                                                              */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void LIBENT LOCALSDNFreeRPacket (register __a0 LOCALGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   struct LOCALRPacket *ipkt;
   struct LOCALRPacket *ip;

   if (rp == NULL) return;
   ipkt = RP2LRP(rp);

   switch(ipkt->class)
   {
      case PACKET_TEMP:
         if (ipkt == tg->tpack)
         {
            tg->tpack = ipkt->next;
         }
         else
         {
            for(ip = tg->tpack; ip != NULL && ip->next != ipkt; ip = ip->next);
            if (ip == NULL)
            {
               return;
            }
            ip->next = ipkt->next;
         }
         if (ipkt->rp.AllocP != NULL)
            FreeMem((void *)ipkt->rp.AllocP, NETBUFSIZE);
         FreeMem((void *)ipkt, sizeof(*ipkt));
         break;

      case PACKET_PERM:
         ipkt->class = PACKET_FREE;
         break;

      default:
         break;
   }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNGetConData                                                      */
/*                                                                             */
/* PURPOSE: Get data associated with a connection for a RPacket                */
/*                                                                             */
/* SYNOPSIS:                                                                   */
/*    data = SDNGetConData(drglobal, RP);                                      */
/*                                                                             */
/*    drglobal  APTR              (in)     Driver-specific global data pointer */
/*                                                                             */
/*    RP        struct RPacket *  (in)     RPacket struct returned from        */
/*                                         PluckRPacket/AcceptRPacket          */
/*                                                                             */
/*    data      void *            (out)    Data for given connection           */
/*                                                                             */
/* NOTES:                                                                      */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void *LIBENT LOCALSDNGetConData (register __a0 LOCALGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   return(tg->data);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNSetConData                                                      */
/*                                                                             */
/* PURPOSE: Set data associated with a connection for a RPacket                */
/*                                                                             */
/* SYNOPSIS:                                                                   */
/*    SDNSetConData(drglobal, RP, data);                                       */
/*                                                                             */
/*    drglobal  APTR              (in)     Driver-specific global data pointer */
/*                                                                             */
/*    RP        struct RPacket *  (in)     RPacket struct returned from        */
/*                                         PluckRPacket/AcceptRPacket          */
/*                                                                             */
/*    data      void *            (in)     Data for given connection           */
/*                                                                             */
/* NOTES:                                                                      */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void LIBENT LOCALSDNSetConData  (register __a0 LOCALGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a2 void *data,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   tg->data = data;
}

int LOCALprivate()
{
   return(0);
}

