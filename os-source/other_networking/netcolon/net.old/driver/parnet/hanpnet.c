/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1988 The Software Distillery.  All Rights Reserved *
* |. o.| ||          Written by Doug Walker                                 *
* | .  | ||          The Software Distillery                                *
* | o  | ||          235 Trillingham Lane                                   *
* |  . |//           Cary, NC 27513                                         *
* ======             BBS:(919)-471-6436                                     *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "netcomm.h"
#include "handler.h"
#include "netpnet.h"

#include <exec/interrupts.h>
#include <resources/misc.h>
#include <hardware/cia.h>

#if DEBUG
char *dbgwind = "CON:0/0/640/160/NETPNET-HANDLER/a";
#endif /* DEBUG */

InitRDevice(global)
GLOBAL global;
{
   struct NPNGLOBAL *ng;
   struct NetNode *tmpnode;
   int rc;

   if(ng=global->n.d.ng)  return(0);  /* Already initialized */

   if(!(ng = AllocMem(sizeof(struct NPNGLOBAL), MEMF_CLEAR)))
   {
      return(1);
   }

   ng->DestAddr = global->n.fssm.fssm_Unit;
   ng->MyAddr = 127 + ng->DestAddr;

   if(!global->RP.DataP)
   {
      global->n.NetBufSize = NETBUFSIZE;
      SetPacketBuffer(&global->RP, NETBUFSIZE);
   }

   BUG(("InitRDevice: DestAddr=%ld, MyAddr= %ld\n", ng->DestAddr, ng->MyAddr))

   global->n.d.ng = ng;

   if(!(rc = OpenPnet(global, ng)))
   {
      for (tmpnode=global->netchain.next; tmpnode; tmpnode=tmpnode->next) 
      {
         BUG(("InitRDevice: Trying to resync '%s'\n", tmpnode->devname));
         NetRestart(global, &tmpnode->RootLock);
      }
   }

   BUGP("InitRDevice: Exit")

   return(rc);
}

int TermRDevice(GLOBAL global, int force)
{
   struct NPNGLOBAL *ng = global->n.d.ng;
   struct NetNode *netnode;

   for (netnode = global->netchain.next; netnode; netnode=netnode->next) 
   {
      if (netnode->ioptr) 
      {
         netnode->status = NODE_DEAD;
         /* close io */
         netnode->ioptr = NULL;
      }
   }

   global->n.d.ng = NULL;
   return(ClosePnet(global, ng));

}
