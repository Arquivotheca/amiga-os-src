/* Timer.c - Timer support routines */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the author.                                           BBS:      */
/* | o  | ||   John Toebes    Dave Baker                     (919)-471-6436  */
/* |  . |//                                                                  */
/* ======                                                                    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
**      $Filename: timer.c $ 
**      $Author: J_Toebes $
**      $Revision: 1.4 $
**      $Date: 91/03/09 21:03:34 $
**      $Log:	timer.c,v $
 * Revision 1.4  91/03/09  21:03:34  J_Toebes
 * Add code for aging packets.
 * 
 * Revision 1.3  91/01/22  00:01:22  Doug
 * *** empty log message ***
 * 
 * Revision 1.2  91/01/17  01:10:35  Doug
 * Handle rebooting
 * 
 * Revision 1.1  91/01/15  02:56:45  Doug
 * Initial revision
 * 
**
**/

#include "handler.h"

int OpenTimer(global, port)
GLOBAL global;    
struct MsgPort *port;
{
   int error;

   /* assumes that a msg port has been allocated */   

   if ((global->timerpkt = (struct TimerPacket *)
                CreateExtIO(port, sizeof(struct TimerPacket)))== NULL)
     return(1);

   global->timerpkt->tm_req.tr_node.io_Message.mn_Node.ln_Name = 
                                         (char *)&(global->timerpkt->tm_pkt);
   global->timerpkt->tm_pkt.dp_Link =
                                 &(global->timerpkt->tm_req.tr_node.io_Message);
   global->timerpkt->tm_pkt.dp_Port = port;

   error = OpenDevice(TIMERNAME, UNIT_MICROHZ, 
                         (struct IORequest *)&(global->timerpkt->tm_req), 0);

   return(error);
}

void CloseTimer(global)
GLOBAL global;
{
   if (global->timerpkt != NULL)
   {
      CloseDevice((struct IORequest *)&(global->timerpkt->tm_req));
      DeleteExtIO((struct IORequest *)global->timerpkt);
      global->timerpkt = NULL;
   }
}

void PostTimerReq(global, secs)
GLOBAL global;
int secs;
{
   /* Fill in the timer packet values */
   /* that is the fields required for the timer device timerequest struct */
   /* and the necessary fields of the DosPacket struct                    */
   /* nothing like using 35 meg of store to accomplish a simple task      */
   /* oh well ! this is a 68K machine right ?                             */ 
   /* some of them get trampled on so fill them all */

   if (global->timerpkt != NULL)
      {
      global->timerpkt->tm_req.tr_node.io_Command = TR_ADDREQUEST;
      global->timerpkt->tm_req.tr_time.tv_secs = secs; 
      global->timerpkt->tm_req.tr_time.tv_micro = 0;

      global->timerpkt->tm_pkt.dp_Type = ACTION_TIMER;
      
      /* Async IO so we don't sleep here for the msg */
     
      SendIO((struct IORequest *)&global->timerpkt->tm_req);
      }
}

ACTFN(ActTimer)
{
   ULONG sigmask = 0L;
   HPACKET *thp, *nhp;
   struct RPacket *RP;

#define MAXAGENODES 10
   struct NetNode *nodes[MAXAGENODES];
   int curagenodes;
   int tmp;

   /* Timer requests come in every 2 seconds.  Every time one comes in, we   */
   /* check the HPackets for which we have no reply.  If no reply comes in   */
   /* after HPAGEOUT timer requests, we check the connection by sending an   */
   /* ACTION_NIL packet.  If the ACTION_NIL packet remains un-replied-to for */
   /* HPAGEOUT timer packets, we assume the connection is bad and kill it.   */

   thp = global->qpkts;
   while(thp)
   {
      curagenodes = 0;
      for(thp=global->qpkts; thp && curagenodes < MAXAGENODES;)
      {
         if(thp->hp_Age < 0)
         {
            /* This is an ACTION_NIL packet already sent to check the other node */
            if(--thp->hp_Age < -HPAGEOUT)
            {
               /* The node hasn't responded to our ACTION_NIL, either - kill it! */
               struct Library *DriverBase = thp->hp_NNode->driver->libbase;
               struct NetNode *killnode = thp->hp_NNode;
               BUG(("***NODE NOT RESPONDING '%s'\n", thp->hp_NNode->name+1))
               SDNTermNode(killnode->driver->drglobal, killnode->ioptr);
               killnode->status = NODE_CRASHED;
               killnode->ioptr = NULL;

               /* Advance 'thp' to the next outstanding packet from a different */
               /* node - after we call KillHPackets, all of this nodes' packets */
               /* will be removed from the list                                 */
               for(thp=thp->hp_WNext; 
                   thp && thp->hp_NNode == killnode; 
                   thp=thp->hp_WNext);

               /* If this node is already on the ageout list, remove it */
               for(tmp=0; tmp<curagenodes; tmp++)
               {
                  if(nodes[tmp] == killnode) 
                  {
                     nodes[tmp] = nodes[curagenodes-1];
                     --curagenodes;
                     break;
                  }
               }

               /* Return all outstanding packets for this node with an error */
               KillHPackets(global, killnode, ERROR_OBJECT_NOT_FOUND);
               continue;
            }
         }
         else if(++thp->hp_Age > HPAGEOUT)
         {
            BUG(("***NODE AGED OUT '%s'\n", thp->hp_NNode->name+1))
            for(tmp=0; tmp<curagenodes && nodes[tmp] != thp->hp_NNode; tmp++);
            if(tmp >= curagenodes)
               nodes[curagenodes++] = thp->hp_NNode;
            thp->hp_Age = 0;
         }

         thp = thp->hp_WNext;
      }

      /* Post writes to all aged-out nodes */
      for(tmp=0; tmp<curagenodes; tmp++)
      {
         if(!(nhp = GetHPacket(global)) ||
            !(RP = AllocRPacket(nodes[tmp], 0))) break;

         RP->Type = ACTION_NIL;
         RP->DLen = 0;

         RP->handle = nhp;

         nhp->hp_RP = RP;
         nhp->hp_NPFlags = 0;
         nhp->hp_NNode = nodes[tmp];
         nhp->hp_Driver = nodes[tmp]->driver;
         nhp->hp_Func = NULL;
         nhp->hp_Pkt = NULL;

         RemotePacket(global, nhp);
      }
   }
   
   PostTimerReq(global, HPTICK);
   hpkt->hp_Pkt = NULL;  /* So it doesn't get freed or returned */
   retpkt(global, hpkt);
}

