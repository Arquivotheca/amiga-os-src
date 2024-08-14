
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987, 1988 The Software Distillery.  All Rights  */
/* |. o.| || Reserved.  This program may not be distributed without the    */
/* | .  | || permission of the authors:                            BBS:    */
/* | o  | ||   John Toebes     Doug Walker    Dave Baker                   */
/* |  . |//                                                                */
/* ======                                                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Process Control */

/* ActDie ActInhibit ActFlush ActTimer */

#include "handler.h"

ACTFN(ActDie)
{
#if 1
   hpkt->hp_Pkt->dp_Res1 = DOS_TRUE;
   hpkt->hp_Pkt->dp_Res2 = 0;
   global->n.run = 0;
   HPQ(global, hpkt);
#else
   struct NetNode *netnode, *tmpnode;
   OBUG(("ActDie\n"));
   global->n.run = 0;
   global->RP.Type = pkt->dp_Type;

   for(netnode=global->netchain.next; netnode; netnode=tmpnode) 
   {
      OBUGBSTR("Shutting down node ", netnode->name);
      RemotePacket(global, &netnode->RootLock, &global->RP);
      TermNode(global, netnode);
      tmpnode = netnode->next;
      DosFreeMem((void *)netnode);
   }
#endif
}

ACTFN(ActInhibit)
{
   struct DosPacket *pkt = hpkt->hp_Pkt;

   OBUG(("ActInhibit: %ld\n", pkt->dp_Arg1));
   pkt->dp_Res1 = DOS_TRUE;
   if (pkt->dp_Arg1 == 0) 
   {
      /* Since the DISKCHANGE command uses ACTION_INHIBIT instead of */
      /* ACTION_DISK_CHANGE, do a ACTION_DISK_CHANGE just in case    */
      ActDiskChange(global, hpkt);
   }
}

ACTFN(ActFlush)
{
   hpkt->hp_Pkt->dp_Res1 = DOS_TRUE;
   HPQ(global, hpkt);
   OBUG(("ActFlush\n"));
}

#if 0
void
ActTimer(global, pkt)
GLOBAL global;
struct DosPacket *pkt;        /* a pointer to the dos packet sent */
{
    OBUG(("ActTimer\n"));

    if (global->run == -1)
        global->run = 0;
    else
        PostTimerReq(global);

    /* Prevent them from replying to the message */
    global->reply = 0;
}
#endif


