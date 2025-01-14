
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1988 The Software Distillery.  All Rights Reserved *
* |. o.| ||	     Written by Doug Walker				    *
* | .  | ||	     The Software Distillery				    *
* | o  | ||	     235 Trillingham Lane				    *
* |  . |//	     Cary, NC 27513					    *
* ======	     BBS:(919)-471-6436                                     *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "netdnet.h"

#ifdef ISHANDLER

short DontClose = 0;

#include "/handler/handler.h"
#ifdef CPR
char *dbgwind = "CON:0/0/640/160/NETDNET-HANDLER/a";
#endif

#else

#include "/server/server.h"
#ifdef CPR
char *dbgwind = "CON:0/0/640/160/NETDNET-SERVER/a";
#endif

#endif

int ReSync(global, ioptr)
GLOBAL global;
APTR ioptr;
{
#ifdef ISHANDLER
    DontClose = 1;
    TermRDevice(global, 0);
    DontClose = 0;
    return(InitRDevice(global));
#else
    return(1);
#endif
}


int
InitRDevice(global)
GLOBAL global;
{
#ifdef ISHANDLER
   struct MsgPort *chan;
   struct NetNode *tmpnode;
   struct RPacket RP;
   static doinit = 1;

   if(!doinit)
   {
      BUG(("InitRDevice: Init loop, exiting\n"))
      return(1);
   }

   doinit = 0;

   BUG(("InitRDevice: Initializing for DNET operation on unit %d\n",
      global->unitnum));

   BUGP("InitRDevice: Entry")

   if(!(chan=DOpen(NULL, (uword)(PORT_FHANDLER), 20, 15)))
   {
      BUG(("********ERROR: Can't DOpen!!\n"));
      BUGR("Can't open channel!")
   }
   if(chan) DQueue((struct DChannel *)chan, 32);/* Is this really necessary? */
   BUG(("DNET Channel %lx open\n", chan));

   if(chan)
   {
      for(tmpnode=global->netchain.next; tmpnode; tmpnode=tmpnode->next)
      {
         BUG(("InitRDevice: Trying to resync '%s'\n", tmpnode->devname))
         NetRestart(global, &tmpnode->RootLock);
      }
   }

   doinit = 1;

   BUGP("InitRDevice: Exit")

   return(chan != NULL);

#else
   BUGP("InitRDevice: Entry")

   global->n.d.LisPort = DListen(PORT_FHANDLER);

   WaitPort(global->n.port);
   ReplyMsg(GetMsg(global->n.port));  /* Tell DNET we are here */

   if(!global->n.d.LisPort)
   {
      BUG(("InitRDevice: Can't init, LisPort %lx\n", global->n.d.LisPort));
      BUGR("Null LisPort");
      return(1);
   }

   /* Wait for a DNET request */
   Wait(1<<global->n.d.LisPort->mp_SigBit);

   if(!(global->n.devptr = (APTR)DAccept(global->n.d.LisPort)))
   {
      BUG(("InitRDevice: Can't DAccept\n"))
      BUGR("No DAccept")
   }

   BUGP("InitRDevice: Exit")

   return(0);
#endif
}

int TermRDevice(global, severity)
GLOBAL global;
int severity;
{
#ifdef ISHANDLER
   struct NetNode *netnode;

   for(netnode=global->netchain.next; netnode; netnode=netnode->next)
   {
      if(netnode->ioptr)
      {
	 netnode->status = NODE_DEAD;
	 if(!DontClose) DClose((struct DChannel *)netnode->ioptr);
	 netnode->ioptr = NULL;
      }
   }
#else
   if(global->n.d.LisPort)
   {
      DNAAccept(global->n.d.LisPort);
      DUnListen(global->n.d.LisPort);
   }
   DeletePort(global->n.devport);
#endif
   return(0);
}

