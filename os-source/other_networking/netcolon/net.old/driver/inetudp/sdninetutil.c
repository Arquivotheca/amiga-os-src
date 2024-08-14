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
int LIBENT INETSDNSerInit      (register __a0 INETGLOBAL *tgp,
                                register __a1 ULONG *maskp,
                                register __a2 char *data,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   INETGLOBAL tg;
   int hostaddr;
   struct selectargs sa;
   struct listenargs la;
   int rc;

   BUGP("SDNSerInit: Entry")

   if ((rc = SDNHanInit((APTR *)tgp, maskp, data)) != SDN_OK)
      return(rc);

   tg = *tgp;

   tg->fhsmask = tg->sockmask = INETMASK(tg->sock);
   set_sockopts(tg, tg->sock);

   tg->sin.sin_family       = AF_INET;
   tg->sin.sin_port         = htons(tg->hostaddr);
   tg->sin.sin_addr.s_addr  = INADDR_ANY;
   if(ibind(tg, tg->sock, &tg->sin, sizeof(tg->sin)) < 0)
   {
      BUG(("********ERROR: Can't Bind Socket - errno=%ld\n", errno))
      BUGR("Can't Bind!")
      FreeMem((void *)tg, sizeof(*tg));
      return(SDN_ERR);
   }

   tg->timeout.tv_sec = 0;
   tg->timeout.tv_usec = 1;

   BUGP("SDNSerInit: Exit")
   
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
int LIBENT INETSDNSerTerm      (register __a0 INETGLOBAL tg,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   if(tg != NULL)
   {
      iclose(tg, tg->sock);
      SDNSerTerm((APTR)tg);
   }
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
int LIBENT INETSDNHanInit      (register __a0 INETGLOBAL *tgp,
                                register __a1 ULONG *maskp,
                                register __a2 char *data,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   INETGLOBAL tg = *tgp;
   int i;
   int hostaddr;
   struct INETRPacket *ip;
#undef SysBase
   struct Library *SysBase = ABSEXECBASE;

   BUGP("SDNSerInit: Entry")
   *tgp = 0;
   *maskp = 0;

   if(!tg)
   {
      if(!(tg = (void *)AllocMem(sizeof(*tg), MEMF_CLEAR)))
      {
         return(SDN_ERR);
      }
   }

   tg->tg_SysBase       = SysBase;
   tg->DriverBase       = DriverBase;
   tg->tg_InetBase      = OpenLibrary("inet:libs/inet.library", 0L) ;
   
   if (tg->tg_InetBase == NULL)
   {
      FreeMem((void *)tg, sizeof(*tg));
      return(SDN_ERR);
   }

   tg->socket_sigurg = AllocSignal(-1);
   tg->socket_sigio  = AllocSignal(-1);

   tg->sock = isocket(tg);
   if(tg->sock < 0)
   {
      BUG(("********ERROR: Create Socket - errno=%ld\n", errno))
      BUGR("No Socket!")
      FreeMem((void *)tg, sizeof(*tg));
      return(SDN_ERR) ;
   }

   for(i=0; i<MAXCONNECT; i++) tg->connect[i].usage = -1;

   /* Allocate our pool of 10 RPackets */
   if ((tg->ppack = (struct INETRPacket *)
                    AllocMem(DEFPACKETS*sizeof(struct INETRPacket), MEMF_CLEAR)) == NULL)
   {
      FreeMem((void *)tg, sizeof(*tg));
      return(SDN_ERR) ;
   }
   for (ip = tg->ppack, i=0; i < DEFPACKETS; i++)
   {
      ip->next = ip+1;
      ip->class = PACKET_FREE;
      ip++;
   }
   ip--;
   ip->next = NULL;

   hostaddr = 0;
   if (data)
   {
      while(*data == ' ') data++;
      while(*data >= '0' && *data <= '9')
      {
         hostaddr = (((hostaddr << 2) + hostaddr)<<1) + *data++ - '0';
      }
   }
   tg->hostaddr = hostaddr;
   if (hostaddr == 0) tg->hostaddr = 7656;

   *maskp = 1L << tg->socket_sigio;
   *tgp = tg;

   BUGP("SDNSerInit: Exit")
   
   return(SDN_OK);
}
#define SysBase (tg->tg_SysBase)

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
int LIBENT INETSDNHanTerm      (register __a0 INETGLOBAL tg,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   int i;

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
    LIBENT INETSDNAllocRPacket (register __a0 INETGLOBAL tg,
                                register __a1 APTR con,
                                register __d0 int len,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   struct INETRPacket *ip;

   for (ip = tg->ppack; ip != NULL && ip->class != PACKET_FREE; ip = ip->next);
   if (ip != NULL)
   {
      ip->class = PACKET_PERM;
   }
   else
   {
      /* Have to allocate a temporary one from system storage */
      ip = (struct INETRPacket *)AllocMem(sizeof(*ip), 0);
      if (ip == NULL) return(NULL);
      ip->next = tg->tpack;
      ip->class = PACKET_TEMP; /* mark it to be freed later     */
      tg->tpack = ip;
      ip->rp.AllocP = NULL;
   }
   if(ip->con = (inetcon *)con) ip->con->usage++;
   ip->rp.DataP = ip->rp.AllocP;
   if (ip->rp.DataP == NULL && len > 0)
   {
      ip->rp.AllocP = ip->rp.DataP = AllocMem(NETBUFSIZE, 0);
      if (ip->rp.AllocP == NULL)
      {
         SDNFreeRPacket((APTR)tg, ip);
         ip = NULL;
      }
   }
   return(ip);
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
    LIBENT INETSDNDupRPacket   (register __a0 INETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   struct INETRPacket *nrp;
   inetcon *con;

   if(con = ((struct INETRPacket *)rp)->con)
      con->usage++;

   if(nrp = (struct INETRPacket *)SDNAllocRPacket((APTR)tg, NULL, NETBUFSIZE))
   {
      /* Actually, this is wrong, we need to copy less, but we aren't using it  */
      /* for now...                                                             */
      memcpy(nrp, rp, sizeof(struct INETRPacket));
   }

   return(nrp);
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
void LIBENT INETSDNFreeRPacket (register __a0 INETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   struct INETRPacket *ipkt = (struct INETRPacket *)rp;
   struct INETRPacket *ip;
   inetcon *con;

   if(con = ((struct INETRPacket *)rp)->con)
      con->usage--;

   if (ipkt == NULL) return;
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
               BUGR("Missing temporary packet");
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
         BUGR("Bad packet type");
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
void *LIBENT INETSDNGetConData (register __a0 INETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   inetcon *con;

   if(con = ((struct INETRPacket *)rp)->con)
   {
      return(con->data);
   }
   return(NULL);
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
void LIBENT INETSDNSetConData  (register __a0 INETGLOBAL tg,
                                register __a1 struct RPacket *rp,
                                register __a2 void *data,
                                register __a6 struct DrLibrary *DriverBase
                               )
{
   inetcon *con;

   if(con = ((struct INETRPacket *)rp)->con)
   {
      con->data = data;
   }
}
