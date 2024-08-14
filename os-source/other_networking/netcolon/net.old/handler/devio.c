/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1988 The Software Distillery.  All Rights Reserved *
* |. o.| ||          Written by Doug Walker                                 *
* | .  | ||          The Software Distillery                                *
* | o  | ||          235 Trillingham Lane                                   *
* |  . |//           Cary, NC 27513                                         *
* ======             BBS:(919)-471-6436                                     *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
**      $Filename:  $ 
**      $Author: J_Toebes $
**      $Revision: 1.17 $
**      $Date: 91/03/09 20:59:27 $
**      $Log:	devio.c,v $
 * Revision 1.17  91/03/09  20:59:27  J_Toebes
 * Correct code for aging out dead nodes
 * 
 * Revision 1.16  91/01/17  01:12:02  Doug
 * Handle rebooting
 * 
 * Revision 1.15  91/01/15  02:54:19  Doug
 * Put in code to kill outstanding hpackets when connection terminates
 * check for an actual hp_Pkt in a couple of places
 * 
**
**/

#include "handler.h"


static int ReLock(GLOBAL global, struct NetNode *nnode);
static int ReStartNode(GLOBAL global, HPACKET *hpkt);
ACTFN(ReLockNext);

static ACTFN(defact);

static ACTFN(defact)
{
   if(hpkt->hp_Pkt)
   {
      hpkt->hp_Pkt->dp_Res1 = hpkt->hp_RP->Arg1;
      hpkt->hp_Pkt->dp_Res2 = hpkt->hp_RP->Arg2;
   }
}

int
RemotePacket(GLOBAL global, HPACKET *hpkt)
{
  struct NetNode *nnode = hpkt->hp_NNode;
  struct RPacket *RP = hpkt->hp_RP;
  short npflags;
  LONG Arg1, Arg2, Arg3, Arg4;
  int badlock;
  int wasnil;
  struct Library *DriverBase = hpkt->hp_Driver->libbase;

   if(nnode->status != NODE_UP &&
         !(nnode->status == NODE_RELOCK && hpkt->hp_State == HPS_RELOCK))
   {
      ReStartNode(global, hpkt);
      BUG(("*** 0x%08lx node '%s' restarted\n", hpkt->hp_Pkt, nnode->name))
      return(1);
   }

   Arg1 = RP->Arg1;
   Arg2 = RP->Arg1;
   Arg3 = RP->Arg1;
   Arg4 = RP->Arg1;

/* Define macro to validate a remote lock/filehandle before sending it */
#define DORP(field, flag) if(npflags & (flag)){\
                             if(CheckNPTR(RP->field)) badlock=1;\
                             else RP->field = (LONG)(((NETPTR)RP->field)->RPtr);}

   if(npflags = hpkt->hp_NPFlags)
   {
      /* Save the resultant RPTRs in local vars in case any of them fail    */
      badlock=0;

      DORP(Arg1, NP1);
      DORP(Arg2, NP2);
      DORP(Arg3, NP3);
      DORP(Arg4, NP4);

      if(badlock)
      {
         hpkt->hp_State = HPS_ERR;

         /* Best guess at a reasonable error message */
         if(hpkt->hp_Pkt)
         {
            hpkt->hp_Pkt->dp_Res1 = DOS_FALSE;
            hpkt->hp_Pkt->dp_Res2 = ERROR_INVALID_LOCK;
         }

         HPQ(global, hpkt);
         BUG(("***  0x%08lx Bad lock, packet not sent\n", hpkt->hp_Pkt))
         return(1);
      }

      /* All of the args checked out OK, copy them in to the RP */
   }

   /* If there's no secondary function, the return codes from the */
   /* other side must be copied over directly                     */
   if(hpkt->hp_Func == NULL) hpkt->hp_Func = defact;

   BUG(("XMT 0x%08lx type %d args (%08lx,%08lx,%08lx,%08lx) node '%s'\n",
      hpkt->hp_Pkt, RP->Type, RP->Arg1, RP->Arg2, RP->Arg3, RP->Arg4, 
      nnode->name+1))

   wasnil = (RP->Type == ACTION_NIL);

   switch(SDNSend(hpkt->hp_Driver->drglobal, RP))
   {
      case SDN_OK:
         /* Basically, we do nothing.  When the remote side replies */
         /* to the packet, its driver will alert us.                */

         /* NOTE: WE SHOULD PUT THIS ON SOME KIND OF "OUTSTANDING PACKET" QUEUE */
         /*       SO IF THE CONNECTION IS BROKEN WE CAN CLEAN IT ALL UP         */
         hpkt->hp_RP = NULL;  /* SDNSend freed this */
         hpkt->hp_Age = (wasnil ? -1 : 0);    /* No age on this one yet... */
         hpkt->hp_WNext = global->qpkts;
         global->qpkts = hpkt;
         return(0);

      case SDN_PEND:
         hpkt->hp_State = HPS_PEND;  /* Waiting on I/O to complete */
         /* Leave the hp_RP - it's still not freed */
         return(0);

      case SDN_ERR:
         /* Any HPKTS sent but not replied to are now DEAD. */

         /* Restore arguments */
         RP->Arg1 = Arg1;
         RP->Arg2 = Arg2;
         RP->Arg3 = Arg3;
         RP->Arg4 = Arg4;

         KillHPackets(global, hpkt->hp_NNode, ERROR_OBJECT_NOT_FOUND); 

         if(hpkt->hp_State == HPS_RETRY)
         {
            /* Time to give up */
            hpkt->hp_State = HPS_ERR;
            HPQ(global, hpkt);
         }
         ReStartNode(global, hpkt);
         BUG(("***RemotePacket SDNSend failed\n"))
         break;
   }
   return(1);
}

static int ReStartNode(GLOBAL global, HPACKET *hpkt)
{
   struct NetNode *nnode = hpkt->hp_NNode;
   struct Library *DriverBase = hpkt->hp_Driver->libbase;
   int rc;

   BUG(("   ReStartNode '%s'\n", nnode->name))

   if(nnode->status != NODE_RELOCK)
   {
      rc = SDNInitNode(hpkt->hp_Driver->drglobal, nnode->name+1, &nnode->ioptr);

      if(rc == SDN_ERR || 
            (rc == SDN_DOWN && hpkt->hp_RP->Type != ACTION_NETWORK_INIT))
      {
         /* Error on write and we can't reestablish the connection */
         /* We have to give up and flush the packet */
         nnode->status = NODE_CRASHED;

         hpkt->hp_State = HPS_ERR;

         /* Best guess at a reasonable error message */
         if(hpkt->hp_Pkt)
         {
            hpkt->hp_Pkt->dp_Res1 = DOS_FALSE;
            hpkt->hp_Pkt->dp_Res2 = ERROR_NODE_DOWN;
         }
         HPQ(global, hpkt);
      }
      else
      {
         /* Allow this packet to ride in the hpc chain... eventually the */
         /* last lock will be reestablished and the whole chain will be  */
         /* processed.                                                   */
         hpkt->hp_WNext = nnode->hpc;
         nnode->hpc = hpkt;

         /* OK, we have managed to reinitialize - now reestablish shared locks  */
         nnode->RootLock.incarnation++;
         if(rc == SDN_OK) ReLock(global, nnode);
      }
   }
   return(0);
}

static int ReLock(GLOBAL global, struct NetNode *nnode)
{
   HPACKET *hpkt;
   struct RPacket *RP;
 
   if(nnode->status == NODE_RELOCK) return(1);

   nnode->status = NODE_RELOCK;  /* Prevent infinite loops */
 
   hpkt = GetHPacket(global);

   if(!(hpkt->hp_RP = RP = AllocRPacket(nnode, nnode->bufsize))) return(1);
   RP->Type = ACTION_LOCATE_OBJECT;
   RP->Arg1 = 0L;
   RP->Arg3 = SHARED_LOCK;
   RP->Arg4 = 0;
   RP->DLen = 0;
   RP->handle = hpkt;
   hpkt->hp_NNode = nnode;
   hpkt->hp_Driver = nnode->driver;
   hpkt->hp_Func = ReLockNext;

   hpkt->hp_NPFlags = 0;
   hpkt->hp_State = HPS_RELOCK;

   /* We could consider allocating bunches of HPACKETS and firing them all */
   /* off at once, but for now let's do it one at a time */

   HPQ(global, hpkt);

   return(0);
}

ACTFN(ReLockNext)
{
   struct RPacket *RP;
   struct NetNode *nnode = hpkt->hp_NNode;
   NETPTR nptr;
   struct FileLock *flock;
   HPACKET *thp;
   int cur, outcur, len, newstate;
   char *objname, *tmpchar, *outname;

   RP = hpkt->hp_RP;

   /* Process the results of the previous relock, if any */
   if(hpkt->hp_State == HPS_ERR)
      nnode->rlk = NULL;

   if(nnode->rlk)
   {
      flock = nnode->rlk;
      nptr = (NETPTR)flock->fl_Key;
      if(RP->Arg1 != NULL)
      {
         /* Success! */
         nptr->RPtr = (RNPTR)RP->Arg1;
      }
      else
      {
         nptr->incarnation--;
      }
      nnode->rlk = (struct FileLock *)flock->fl_Link;
   }
   else
      nnode->rlk = nnode->lkc;

   /* Send off the next relock, or clean up if none */
   if(nnode->rlk)
   {
      flock = nnode->rlk;
      nptr = (NETPTR)flock->fl_Key;

      nptr->incarnation = nptr->NetNode->RootLock.incarnation;

      /* We now must construct the name of the item from the 'objname' buffer  */
      /* the 'objname' buffer consists of the nodes of the name in REVERSE     */
      /* ORDER, each one followed by a 1-byte length (the length includes 1    */
      /* for the length itself).  Thus, FOO:BAR/BLETCH would show up as        */
      /* "BLETCH\7BAR\4FOO\4".  This allows the server to create the string    */
      /* easily, but still lets us parse it easily if/when we have to recreate */
      /* the original name.                                                    */

      outname = RP->DataP;
      objname = nptr->objname;
      cur = strlen(objname)-1;
      outcur=2;
      while(cur>0)
      {
         len = objname[cur] - 1; 
         tmpchar = objname + cur - len;
         memcpy(outname+outcur, tmpchar, len);
         outcur += len;
         cur -= (len+1);
         outname[outcur++] = '/';
      }
      outcur--;  /* Get rid of extra '/' */

      outname[outcur] = 0;

      RP->DLen = outcur;
      outname[0] = outcur-1;
      outname[1] = ':';

      RP->Type = ACTION_LOCATE_OBJECT;
      RP->Arg1 = 0;
      RP->Arg3 = SHARED_LOCK;
      RP->Arg4 = 0;
      RP->handle = hpkt;

      hpkt->hp_Func = ReLockNext;

      RemotePacket(global, hpkt);
   }
   else
   {
      /* Release all the queued-up HPACKETs */
      if(nnode->hpc)
      {
         newstate = (hpkt->hp_State == HPS_ERR ? HPS_ERR : HPS_RETRY);
         for(thp=nnode->hpc; thp->hp_WNext; thp=thp->hp_WNext)
            thp->hp_State = newstate;

         thp->hp_State = newstate;

         thp->hp_WNext = global->donepkts;
         global->donepkts = thp;
         nnode->hpc = NULL;
      }

      nnode->status = (hpkt->hp_State == HPS_ERR ? NODE_CRASHED : NODE_UP);

      hpkt->hp_Pkt = NULL;  /* Prevent retpkt from returning this */
   }
}


HPACKET *GetHPacket(GLOBAL global)
{
   HPACKET *hp;

   if(global->prmhpkt)
   {
      /* Free permanent hpackets are on a circularly linked list */
      /* global->prmhpkt points to one element, if there are any */

      hp = global->prmhpkt->hp_Next;
      if(hp == global->prmhpkt)  /* If the last one on the chain... */
         global->prmhpkt = NULL;
      else
         global->prmhpkt->hp_Next = hp->hp_Next;
      hp->hp_Next = NULL;        /* So we know it's a permanent one when freeing*/
   }
   else
   {
      /* Temporary hpackets are allocated and placed on a temporary list */
      /* They will be freed as soon as FreeHPacket is called on them */

      /* Have to allocate a temporary hpkt */
      if(!(hp = (HPACKET *)DosAllocMem(global, sizeof(HPACKET))))
         return(NULL);

      hp->hp_Next = global->tmphpkt;
      global->tmphpkt = hp;
   }

   hp->hp_State = HPS_NEW;

   return(hp);
}

void FreeHPacket(GLOBAL global, HPACKET *hp)
{
   HPACKET *thp;

   if(hp->hp_Next == NULL)
   {
      /* It's a permanent hpkt */
      /* Put it back on the circularly linked list */
      if(global->prmhpkt)
      {
         hp->hp_Next = global->prmhpkt->hp_Next->hp_Next;
         global->prmhpkt->hp_Next = hp;
      }
      else
      {
         global->prmhpkt = hp->hp_Next = hp;
      }
   }
   else
   {
      /* Temporary hpkt */
      /* Unlink it and free it */

      if(global->tmphpkt == hp)
         global->tmphpkt = hp->hp_Next;
      else
      {
         for(thp=global->tmphpkt; thp && thp->hp_Next != hp; thp = thp->hp_Next);
         if(!thp)
         {
            return;
         }
         thp->hp_Next = hp->hp_Next;
      }
      DosFreeMem((char *)hp);
   }
}

struct RPacket *AllocRPacket(struct NetNode *nnode, long size)
{
   struct Library *DriverBase = nnode->driver->libbase;
   struct RPacket *RP;
   RP = SDNAllocRPacket(nnode->driver->drglobal, nnode->ioptr, size);
#if DEBUG
   if(RP == NULL) 
   {
      BUG(("***************************************\n"))
      BUG(("************AllocRPacket failed!*******\n"))
      BUG(("***************************************\n"))
   }
#endif
   return(RP);
}
