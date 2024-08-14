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
#include "netinet.h"
#include "proto.h"

#ifdef BUGR
#undef BUGR
#endif
#define BUGR(s)

extern void *_socks[FD_SETSIZE];

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
int SDNSerInit(drgp, maskp, data)
APTR *drgp;
ULONG *maskp;
char *data;
{

   INETGLOBAL tg = (INETGLOBAL)*drgp;
   int on = 1;
   int i;
   int hostaddr;
   struct INETRPacket *ip;
   struct selectargs sa;

   BUGP("SDNSerInit: Entry")
   *drgp = 0;
   *maskp = 0;

   if(!tg)
   {
      if(!(tg = (void *)AllocMem(sizeof(*tg), MEMF_CLEAR)))
      {
         return(SDN_ERR);
      }
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
         hostaddr = (hostaddr * 10) + *data++ - '0';
      }
   }
   if (hostaddr == 0) hostaddr = 7656;

   tg->sock = socket(AF_INET, SOCK_STREAM, 0);
   if(tg->sock < 0)
   {
      BUG(("********ERROR: Create Socket - errno=%ld\n", errno))
      BUGR("No Socket!")
      FreeMem((void *)tg, sizeof(*tg));
      return(SDN_ERR) ;
   }

   tg->fhsmask = tg->sockmask = INETMASK(tg->sock);
   setsockopt(tg->sock,IPPROTO_TCP,TCP_NODELAY,&on,sizeof(on));

   tg->sin.sin_family       = AF_INET;
   tg->sin.sin_port         = htons(hostaddr);
   tg->sin.sin_addr.s_addr  = INADDR_ANY;
   if(bind(tg->sock, &tg->sin, sizeof(tg->sin)) < 0)
   {
      BUG(("********ERROR: Can't Bind Socket - errno=%ld\n", errno))
      BUGR("Can't Bind!")
      FreeMem((void *)tg, sizeof(*tg));
      return(SDN_ERR);
   }

   tg->timeout.tv_sec = 0;
   tg->timeout.tv_usec = 1;

   listen(tg->sock, MAXCONNECT);

   tg->fhscurmask = tg->fhsmask;
   sa.socks  = _socks;             /* array of socket ptrs              */
   sa.rd_set = (fd_set *)&tg->fhscurmask; /* sockets considered for read event */
   sa.wr_set = 0;                  /* sockets considered for write event*/
   sa.ex_set = 0;                  /* sockets considered for ex events  */
   sa.task   = FindTask(0L);       /* task to signal when selecting     */
   sa.sigbit = socket_sigio;       /* signal to send when selecting     */
   sa.numfds = 32;                 /* number of fds to consider         */
   sa.errno  = 0;                  /* return error value                */
   sa.rval   = 0;                  /* number of fds found ready         */

   SelectAsm(&sa);

   if (!sa.errno && !sa.rval)
   {
      /* Nothing really happened, just let them know */
      tg->fhscurmask = 0;
   }

   *maskp = 1L << socket_sigio;
   *drgp = (APTR)tg;

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
int SDNSerTerm(drglobal)
APTR drglobal;
{
   INETGLOBAL tg = (INETGLOBAL)drglobal;
   int i;

   if(tg != NULL)
   {
      for(i = 0; i < tg->fhscount; i++)
         close(tg->connect[i].fh);
      close(tg->sock);
      FreeMem((void *)tg, sizeof(*tg));
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
int SDNHanInit(drgp, maskp, data)
APTR *drgp;
ULONG *maskp;
char *data;
{
   int i;
   INETGLOBAL tg = (INETGLOBAL)*drgp;

   BUGP("SDNHanInit: Entry")
   *drgp = 0;
   *maskp = 0;

   if(!tg)
   {
      if(!(tg = (void *)AllocMem(sizeof(*tg), MEMF_CLEAR)))
      {
         return(SDN_ERR);
      }
   }

   for(i=0; i<MAXCONNECT; i++) tg->connect[i].usage = -1;

   *maskp = 1L << socket_sigio;
   *drgp = (APTR)tg;

   BUGP("SDNHant: Exit")
   
   return(SDN_OK);
}

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
int SDNHanTerm(drglobal)
APTR drglobal;
{
   INETGLOBAL tg = (INETGLOBAL)drglobal;

   if(tg != NULL)
   {
      FreeMem((void *)tg, sizeof(*tg));
   }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNInitNode                                                        */
/*                                                                             */
/* PURPOSE: Establish a new node connection                                    */
/*                                                                             */
/* SYNOPSIS:                                                                   */
/*    rc = SDNInitNode(drglobal, nnode);                                       */
/*                                                                             */
/*    drglobal   APTR             (in)     Driver-specific global data pointer */
/*                                                                             */
/*    nnode      struct NetNode * (in/out) On input, name of node is set.      */
/*                                         On output, driver sets the 'status' */
/*                                            and 'ioptr' fields.              */
/*                                                                             */
/*    rc         int              (ret)    SDN_ERR, SDN_OK                     */
/*                                                                             */
/* NOTES:                                                                      */
/*    Called by the handler to establish communications with a specific        */
/*    remote node.                                                             */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int SDNInitNode(APTR drglobal, APTR con)
{
   INETGLOBAL tg = (INETGLOBAL)drglobal;

   return(SDN_ERR);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/* ROUTINE: SDNTermNode                                                        */
/*                                                                             */
/* PURPOSE: Free Up all Driver resources associated with a Node                */
/*                                                                             */
/* SYNOPSIS:                                                                   */
/*    SDNTermNode(drglobal, nnode);                                            */
/*                                                                             */
/*    drglobal  APTR              (in)     Driver-specific global data pointer */
/*                                                                             */
/*                                                                             */
/*    nnode     struct NetNode *  (in)     NetNode to terminate                */
/*                                                                             */
/* NOTES:                                                                      */
/*    This routine is void.                                                    */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SDNTermNode(APTR drglobal, APTR con)
{
   INETGLOBAL tg = (INETGLOBAL)drglobal;
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
struct RPacket *SDNAllocRPacket(APTR drglobal, APTR con)
{
   INETGLOBAL tg = (INETGLOBAL)drglobal;
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
   }
   if(ip->con = (inetcon *)con) ip->con->usage++;
   if (ip->rp.DataP == NULL)
   {
      ip->rp.DataP = AllocMem(NETBUFSIZE, 0);
      if (ip->rp.DataP == NULL)
      {
         SDNFreeRPacket(drglobal, ip);
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

struct RPacket *SDNDupRPacket(APTR drglobal, struct RPacket *rp)
{
   struct INETRPacket *nrp;
   inetcon *con;

   if(con = ((struct INETRPacket *)rp)->con)
      con->usage++;

   if(nrp = (struct INETRPacket *)SDNAllocRPacket(drglobal, NULL))
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
void SDNFreeRPacket(APTR drglobal, struct RPacket *rp)
{
   INETGLOBAL tg = (INETGLOBAL)drglobal;
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
         if (ipkt->rp.DataP != NULL)
            FreeMem((void *)ipkt->rp.DataP, NETBUFSIZE);
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
