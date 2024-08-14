/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987, 1988 The Software Distillery.  All Rights  */
/* |. o.| || Reserved.  This program may not be distributed without the    */
/* | .  | || permission of the authors:                            BBS:    */
/* | o  | ||   John Toebes     Doug Walker    Dave Baker                   */
/* |  . |//                                                                */
/* ======                                                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
**      $Filename:  $ 
**      $Author: Doug $
**      $Revision: 1.20 $
**      $Date: 91/01/17 01:12:11 $
**      $Log:	lock.c,v $
 * Revision 1.20  91/01/17  01:12:11  Doug
 * Handle rebooting
 * 
 * Revision 1.19  91/01/15  02:55:26  Doug
 * Remove return-code-screening code - unnecessary since Mike fixed WB
 * 
**
**/

/* Lock.c - lock manipulation */
/* ActLock, ActDupLock, ActUnLock */

/*-------------------------------------------------------------------------*/
/* Structure of a Lock:                                                    */
/*   struct FileLock {                                                     */
/*      BPTR fl_Link;             Next lock in the chain of device locks   */
/*      LONG fl_Key;              Block number of directory or file header */
/*      LONG fl_Access;           Shared Read (-2) or Exclusive Write (-1) */
/*      struct MsgPort * fl_Task; Handler process for Lock (Us)            */
/*      BPTR fl_Volume;           Node in DevInfo structure for Lock       */
/*      };                                                                 */
/*-------------------------------------------------------------------------*/

#include "handler.h"


/* create a lock to be returned */
struct FileLock *
CreateLock(GLOBAL global, NETPTR nlock, 
           RNPTR RLock, LONG Access, struct RPacket *RP)
{
    struct FileLock *flock;
    NETPTR newnlock;
    int len;

    OBUG(("CreateLock: Entry, nlock %08lx NetNode %08lx RLock %08lx Access %d\n",
          nlock, nlock->NetNode, RLock, Access));

    if(Access == SHARED_LOCK && (global->flags & NGF_RELOCK) && RP && RP->DLen)
    {
      /* The name of the object is in DataP */
      len = RP->DLen+1;
    }
    else
      len = 0;

    flock = (struct FileLock *)DosAllocMem(global, sizeof(struct FileLock));
    if(flock == NULL) return(NULL);

    newnlock = (NETPTR)DosAllocMem(global, sizeof(struct NetPtr) + len);

    if(newnlock == NULL) return(NULL);

    OBUG(("CreateLock: Allocated lock %08lx nlock %08lx\n", flock, newnlock));

    flock->fl_Key = (LONG)newnlock;
    flock->fl_Access = Access;
    flock->fl_Task = global->n.port;
    flock->fl_Volume = MKBADDR(global->volume);
    newnlock->NetNode = nlock->NetNode;
    newnlock->RPtr    = RLock;
    newnlock->incarnation = newnlock->NetNode->RootLock.incarnation;

    /* If we are to reestablish this lock, link it into the chain */
    if(len)
    {
       newnlock->objname = (char *)(newnlock+1);
       memcpy(newnlock->objname, RP->DataP, len-1);
       newnlock->objname[len-1] = 0;  /* NULL terminator */
       flock->fl_Link = (BPTR)nlock->NetNode->lkc;
       nlock->NetNode->lkc = flock;
    }

    OBUG(("CreateLock: Exit\n"));

    return(flock);
}

void
FreeLock(global, flock)
GLOBAL global;
struct FileLock *flock;
{
   struct FileLock *current, *next;
   NETPTR nlock;

   OBUG(("FreeLock: Entry, lock %08lx\n", flock));

   if (!flock)
      return;

   nlock = (NETPTR)flock->fl_Key;

   if(nlock->objname)
   {
      /* This one was to be reestablished, so it's linked into the list */
      if((current=nlock->NetNode->lkc) == flock) 
      {
         nlock->NetNode->lkc = (struct FileLock *)flock->fl_Link;
         OBUG(("FreeLock: Head lock freed, new head = %lx\n", 
              nlock->NetNode->lkc));
      } 
      else 
      {
         for(next=NULL;
             current && (next=(struct FileLock *)current->fl_Link) != flock;
             current = next);

         if (next) 
         {
            current->fl_Link = next->fl_Link;
            OBUG(("Lock found and removed from chain\n"));
         } 
      }
   }

   if (nlock != (NETPTR)nlock->NetNode)
      DosFreeMem((char *)nlock);  /* Free the net lock   */

   DosFreeMem((char *)flock);     /* Free the local lock */

   OBUG(("FreeLock: Exit\n"));
}

/* Arg1: Lock */
/* Arg2: Name */
/* Arg3: Mode: ACCESS_READ, ACCESS_WRITE */

ACTFN(ActLock)
{
   NETPTR nlock;
   struct FileLock *flock;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP;

   OBUG(("ActLock: Entry, hpkt %08lx\n", hpkt));
   OBUGBSTR("File to lock: ", pkt->dp_Arg2);

   flock = (struct FileLock *)pkt->dp_Arg1;
   /**********************************************************************/
   /* If the given lock is on the local root, call the ParseName routine */
   /* to determine if we need to call the remote nodes.  If ParseName    */
   /* returns 1, we have a name of the form "NET:" and we should return  */
   /* a lock on the root of NET:.  If it returns 0, we have a name of    */
   /* the form "NET:FOO" or "NET:FOO/BAR", etc and we need to call the   */
   /* remote node specified in the pseudo-lock returned.                 */
   /**********************************************************************/
   if(TESTROOT(flock, nlock))
   {
      /* ParseName returns 1 if we are locking the root.  It fills in */
      /* nlock with a pointer to the pseudolock for the node.  The    */
      /* pseudolock is kept in the NetNode struct.                    */
      /* Any leftover parts of the name are left in RP.Data as a      */
      /* BCPL string.                                                 */

      if(ParseName(global, (char *)pkt->dp_Arg2, &nlock, global->work))
      {
         OBUG(("ActLock: Returning lock on root\n"));
         /* Return lock on root */
         /* ParseName has put the pseudolock for the root into nlock */
         flock = CreateLock(global, nlock, (RNPTR)1, pkt->dp_Arg3, NULL);
         nlock = (NETPTR)flock->fl_Key;

         pkt->dp_Res1 = MKBADDR(flock);
         HPQ(global, hpkt);
         return;
      } 
      else if (!nlock) 
      {
         OBUG(("ActLock: Couldn't find node\n"));
errcase:
         pkt->dp_Res1 = NULL;
         pkt->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
         HPQ(global, hpkt);
         return;
      }
      OBUG(("ActLock: got nlock, falling through\n"));
      if(!(RP = AllocRPacket(nlock->NetNode, BSTRLEN(global->work)+1)))
      {
         norpkt:
         pkt->dp_Res1 = DOS_FALSE;
         pkt->dp_Res2 = ERROR_NO_FREE_STORE;
         HPQ(global, hpkt);
         return;
      }
      MBSTR(global->work, RP->DataP);
   } 
   else 
   {
      OBUGBSTR("ActLock: Relative open, node ", nlock->NetNode->name);
      /* We are doing a relative open - put filename in Data */
      if(!(RP = AllocRPacket(nlock->NetNode, 0))) goto norpkt;
      RP->DataP = (char *)pkt->dp_Arg2;
   }

   RP->Type = pkt->dp_Type;
   RP->Arg1 = (LONG)nlock;
   if(!memcmp(RP->DataP, "\2::", 3))
   {
      /* Special lock on nodename.info */
      RP->Arg2 = 1;
      RP->DLen = 0;
   }
   else
   {
      RP->Arg2 = 0;
      RP->DLen = BSTRLEN(RP->DataP)+1;
      if (RP->DLen == 1)
         RP->DLen = 0;
   }
   RP->Arg3 = pkt->dp_Arg3;
   RP->Arg4 = (pkt->dp_Arg3 == SHARED_LOCK && (global->flags & NGF_RELOCK));
   RP->handle = hpkt;

   hpkt->hp_RP = RP;
   hpkt->hp_Func = ActLock2;
   hpkt->hp_NNode = nlock->NetNode;
   hpkt->hp_Driver = nlock->NetNode->driver;

   hpkt->hp_NPFlags = NP1;
   pkt->dp_Res1 = (LONG)nlock;

   RemotePacket(global, hpkt);
}

ACTFN(ActLock2)
{
   struct FileLock *newflock;
   NETPTR nlock;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP = hpkt->hp_RP;

   OBUG(("ActLock2: Entry, hpkt %08lx\n", hpkt))

   if(hpkt->hp_State != HPS_ERR)
   {
      OBUG(("ActLock2: Returned RNPTR is %lx\n", RP->Arg1));
      pkt->dp_Res2 = RP->Arg2;
      if(RP->Arg1)
      {
         nlock = (NETPTR)pkt->dp_Res1;
         newflock = CreateLock(global, nlock, (RNPTR)RP->Arg1, pkt->dp_Arg3, RP);
         pkt->dp_Res1 = MKBADDR(newflock);
      }
      else
      {
         pkt->dp_Res1 = NULL;
#if 0
/* No longer necessary, thanks to Mike Sinz */
         if(pkt->dp_Res2 == ERROR_DISK_NOT_VALIDATED ||
            pkt->dp_Res2 == ERROR_DEVICE_NOT_MOUNTED ||
            pkt->dp_Res2 == ERROR_NOT_A_DOS_DISK     ||
            pkt->dp_Res2 == ERROR_NO_DISK)
         {
            pkt->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
         }
#endif
      }
   }
#if DEBUG
   else
      BUG(("ActLock2: ERROR RETURN\n"))
      
#endif
}

ACTFN(ActDupLock)
{
   NETPTR nlock;
   struct FileLock *flock;
   struct RPacket *RP;
   struct DosPacket *pkt = hpkt->hp_Pkt;

   OBUG(("ActDupLock: Entry, hpkt %08lx\n", hpkt));

   flock = (struct FileLock *)pkt->dp_Arg1;

   /* If this is a lock on the local root, it is a special case */
   if(TESTROOT(flock, nlock))
   {
      if(flock)
         flock = CreateLock(global, nlock, (RNPTR)1, flock->fl_Access, NULL);
      pkt->dp_Res1 = MKBADDR(flock);
      pkt->dp_Res2 = 0;
      HPQ(global, hpkt);
      OBUG(("ActDupLock: Lock on root\n"))
      return;
   }

   if(!(RP = AllocRPacket(nlock->NetNode, 0)))
   {
      pkt->dp_Res1 = DOS_FALSE;
      pkt->dp_Res2 = ERROR_NO_FREE_STORE;
      HPQ(global, hpkt);
      return;
   }

   RP->Type = pkt->dp_Type;
   RP->Arg1 = (LONG)nlock;
   RP->Arg4 = (global->flags & NGF_RELOCK);
   RP->DLen = 0;
   RP->handle = hpkt;

   hpkt->hp_RP = RP;
   hpkt->hp_Func = ActDupLock2;
   hpkt->hp_NPFlags = NP1;
   hpkt->hp_NNode = nlock->NetNode;
   hpkt->hp_Driver = nlock->NetNode->driver;

   pkt->dp_Res1 = (LONG)nlock;

   RemotePacket(global, hpkt);
}

ACTFN(ActDupLock2)
{
   NETPTR nlock;
   struct FileLock *flock;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP = hpkt->hp_RP;

   OBUG(("ActDupLock2: Entry, hpkt %08lx\n", hpkt))

   if(hpkt->hp_State != HPS_ERR)
   {
      nlock = (NETPTR)pkt->dp_Res1;
      flock = (struct FileLock *)pkt->dp_Arg1;
      pkt->dp_Res1 = MKBADDR(CreateLock(global, nlock, (RNPTR)RP->Arg1, 
                                        flock->fl_Access, RP));
      pkt->dp_Res2 = RP->Arg2;
   }
}

ACTFN(ActUnLock)
{
   NETPTR nlock;
   struct FileLock *flock;
   struct RPacket *RP;
   struct DosPacket *pkt = hpkt->hp_Pkt;

   OBUG(("ActUnLock: Entry, hpkt %08lx\n", hpkt));

   flock = (struct FileLock *)pkt->dp_Arg1;
   /* If this lock exists on a remote node, delete it */
   if(!TESTROOT(flock, nlock))
   {
      if(!(RP = AllocRPacket(nlock->NetNode, 0)))
      {
         pkt->dp_Res1 = DOS_FALSE;
         pkt->dp_Res2 = ERROR_NO_FREE_STORE;
         HPQ(global, hpkt);
         return;
      }
      RP->Type = pkt->dp_Type;
      RP->Arg1 = (LONG)nlock;
      RP->DLen = 0;
      RP->handle = hpkt;
      hpkt->hp_RP = RP;
      hpkt->hp_NPFlags = NP1;
      hpkt->hp_NNode = nlock->NetNode;
      hpkt->hp_Driver = nlock->NetNode->driver;
      hpkt->hp_Func = ActUnLock2;
      hpkt->hp_Pkt->dp_Res1 = (LONG)flock;
      RemotePacket(global, hpkt);
   } 
   else 
   {
      /* No remote lock, just local on the root */
      pkt->dp_Res1 = DOS_TRUE;
      HPQ(global, hpkt);
      if(flock) FreeLock(global, flock);
   }
}

ACTFN(ActUnLock2)
{
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP = hpkt->hp_RP;

   OBUG(("ActUnLock2: Entry, hpkt %08lx\n", hpkt))

   if(hpkt->hp_State != HPS_ERR)
   {
      if(pkt->dp_Res1) FreeLock(global, (struct FileLock *)pkt->dp_Res1);

      pkt->dp_Res1 = RP->Arg1;
      pkt->dp_Res2 = RP->Arg2;
   }
}

int
ParseName(global, name, nlock, outname)
GLOBAL global;
char *name;
NETPTR *nlock;
char *outname;
{
   int len, cur;
   struct NetNode *node;
   char bname[RNAMELEN+2];

   OBUGBSTR("ParseName: Parsing name ", name);
   /* Check for 2.0 special .whatever files  - ".icon" ".backdrop" */
   if(!memcmp(name+1, ".icon", 5) || !memcmp(name+1, ".backdrop", 9))
   {
      OBUG(("ParseName: Special 2.0 .filename\n"))
      *nlock = NULL;
      return(0);
   }

   for (len=*name++, cur=0; len && (*name != '/'); len--, name++) 
   {
      if (*name==':')
         cur=0;
      else
         outname[cur++] = *name;
   }
   outname[cur] = '\0';
   if (len > 0 && *name == '/') 
   {
      name++;
      len--;
   }

   if (len <= 1 && cur == 0) 
   {
      /* It is the root of NET: */
      *nlock = &global->netchain.RootLock;
      return(1);
   }
OBUG(("ParseName: Trying to find node %s\n", outname))
   /* We have a network node name - find it */
   node = FindNode(global, outname);


   /* OK, check to see if it is a .info file for a node */
   if (!node && cur >= 6 && !stricmp(outname+cur-5, ".info"))
   {
OBUG(("ParseName: Checking for %s.info\n", outname))
      outname[cur-5] = '\0';
      node = FindNode(global, outname);

      name = ":";
      len = 1;
   }

   if(!node && stricmp(outname, "disk") &&
         stricmp(outname, ".info"))  /* DISK.INFO, .INFO - special cases */
   {
      /* No node found - mount it */
      strcpy(bname+1, outname);
      bname[0] = strlen(outname);
      OBUGBSTR("ParseName: No node found, mounting ", bname);
      if(node = AddNode(global, bname))
         strcpy(node->devname, outname);
   } 
   if(node)
   {
      /* Return template lock */
      *nlock = &node->RootLock;
      OBUGBSTR("ParseName: Resolved node is ", node->name);
   }
   else
      *nlock = NULL;

   /* Move the rest of the name into outname as a BSTR */
   outname[0] = (char)len+1;
   outname[1] = ':';
   if(len) MQ(name, outname+2, len);
   outname[len+2] = '\0';
   OBUG(("Remote filename: len %d text '%s'\n", (int)*outname, outname+1));
   return(0);
}

struct NetNode *
   FindNode(global, name)
GLOBAL global;
char *name;
{
    struct NetNode *node;
    for (node=global->netchain.next; node; node=node->next) 
    {
        if (!stricmp(name, node->name+1))
            break;
    }
    OBUG(("FindNode: name %s : %s\n", name, node ? "SUCCESS" : "FAILURE"))
    return(node);
}
