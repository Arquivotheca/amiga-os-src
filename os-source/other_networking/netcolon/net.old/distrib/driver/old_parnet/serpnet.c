
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1988 The Software Distillery.  All Rights Reserved *
* |. o.| ||	     Written by Doug Walker				    *
* | .  | ||	     The Software Distillery				    *
* | o  | ||	     235 Trillingham Lane				    *
* |  . |//	     Cary, NC 27513					    *
* ======	     BBS:(919)-471-6436                                     *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "netcomm.h"
#include "server.h"

#include <exec/interrupts.h>
#include <resources/misc.h>
#include <hardware/cia.h>
#include "netpnet.h"

#if DEBUG
char *dbgwind = "CON:0/0/640/160/NETPNET-SERVER/a";
#endif /* DEBUG */

InitRDevice(GLOBAL global)
{
   struct NPNGLOBAL *ng;

   if(ng=global->n.d.ng)  return(0);  /* Already initialized */

   if(!(ng = AllocMem(sizeof(struct NPNGLOBAL), MEMF_CLEAR)))
   {
      return(1);
   }

   ng->MyAddr = global->n.fssm.fssm_Unit;
   ng->DestAddr = 127 + ng->MyAddr;

   if(!global->RP.DataP)
   {
      global->n.NetBufSize = NETBUFSIZE;
      SetPacketBuffer(&global->RP, NETBUFSIZE);
   }

   BUG(("InitRDevice: DestAddr=%ld, MyAddr= %ld\n", ng->DestAddr, ng->MyAddr))

   global->n.d.ng = ng;

   return(OpenPnet(global, ng));
}

int TermRDevice(GLOBAL global, int force)
{
   struct NPNGLOBAL *ng = global->n.d.ng;

   global->n.d.ng = NULL;

   return(ClosePnet(global, ng));
}

