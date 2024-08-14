
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987, 1988 The Software Distillery.  All Rights  */
/* |. o.| || Reserved.  This program may not be distributed without the    */
/* | .  | || permission of the authors:                            BBS:    */
/* | o  | ||   John Toebes     Doug Walker    Dave Baker                   */
/* |  . |//                                                                */
/* ======                                                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* File Manipulation  */
/*  ActDelete ActRename ActSetProtection ActSetComment */

#include "handler.h"

/* Arg1: Lock   */
/* Arg2: Name   */
ACTFN(ActDelete)
{
   NETPTR nlock;
   struct FileLock *flock;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP;
   OBUG(("ActDelete\n"));

   flock = (struct FileLock *)pkt->dp_Arg1;

   /* Can't delete in the root of NET: */
   if(TESTROOT(flock, nlock))
   {
      if (ParseName(global, (char *)pkt->dp_Arg2, &nlock, global->work) ||
            !nlock)
      {
         /* Either he is trying to delete a node or he got the name wrong */
         pkt->dp_Res1 = DOS_FALSE;
         pkt->dp_Res2 = (nlock ? ERROR_OBJECT_IN_USE
                               : ERROR_WRITE_PROTECTED);
         HPQ(global, hpkt);
         return;
      }
      OBUGBSTR("RmtDelete: Sending to node ", nlock->NetNode->name);
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
      if(!(RP = AllocRPacket(nlock->NetNode, 0))) goto norpkt;
      RP->DataP = (char *)pkt->dp_Arg2;
   }
   OBUGBSTR("RmtDelete: remote node name =", RP->DataP);

   RP->Type = pkt->dp_Type;
   RP->Arg1 = (LONG)nlock;
   RP->DLen = BSTRLEN(RP->DataP)+1;
   if(RP->DLen == 1) RP->DLen = 0;
   RP->handle = hpkt;

   hpkt->hp_RP = RP;
   hpkt->hp_NPFlags = NP1;
   hpkt->hp_NNode = nlock->NetNode;
   hpkt->hp_Driver = nlock->NetNode->driver;

   RemotePacket(global, hpkt);
}

/* Arg1: FromLock       */
/* Arg2: FromName       */
/* Arg3: ToLock         */
/* Arg4: ToName         */

ACTFN(ActRename)
{
   NETPTR nlock, nlock2;
   struct FileLock *flock;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP;
   char *pname1, *pname2;
   int dlen;

   OBUG(("ActRename\n"));

   flock = (struct FileLock *)pkt->dp_Arg1;

   /* See if it is a local lock */
   if(TESTROOT(flock, nlock))
   {
      if(ParseName(global, (char *)pkt->dp_Arg2, &nlock, global->work) ||
         !nlock)
      {
         /* Trying to rename a node, or something that doesn't exist */
         pkt->dp_Res1 = DOS_FALSE;
         pkt->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
         HPQ(global, hpkt);
         return;
      }
      pname1 = global->work;
   } 
   else 
   {
      pname1 = (char *)pkt->dp_Arg2;
   }

   flock = (struct FileLock *)pkt->dp_Arg3;

   /* See if the second lock is a local lock */
   if(TESTROOT(flock, nlock2))
   {
      pname2 = global->work+FILENAMELEN;
      if(ParseName(global, (char *)pkt->dp_Arg4, &nlock2, pname2) || !nlock2)
      {
         /* See if they are trying to rename a node */
         if(pname1[0] == 1 && pname1[1] == ':') 
         {
            if(pname2[0] > RNAMELEN) pname2[0] = RNAMELEN;
            MBSTR(pname2, nlock->NetNode->name);
            pkt->dp_Res1 = DOS_TRUE;
            pkt->dp_Res2 = 0;
         } 
         else 
         {
            pkt->dp_Res1 = DOS_FALSE;
            pkt->dp_Res2 = (nlock2 ? ERROR_OBJECT_EXISTS
                                   : ERROR_RENAME_ACROSS_DEVICES);
         }
         HPQ(global, hpkt);
         return;
      }
   } 
   else 
   {
      pname2 = (char *)pkt->dp_Arg4;
   }

   /* Check for rename across devices */
   if(nlock->NetNode != nlock2->NetNode)
   {
      pkt->dp_Res1 = DOS_FALSE;
      pkt->dp_Res2 = ERROR_RENAME_ACROSS_DEVICES;
      HPQ(global, hpkt);
      return;
   }

   dlen = FILENAMELEN+BSTRLEN(pname2)+2;
   if(!(RP = AllocRPacket(nlock->NetNode, dlen)))
   {
      pkt->dp_Res1 = DOS_FALSE;
      pkt->dp_Res2 = ERROR_NO_FREE_STORE;
      HPQ(global, hpkt);
      return;
   }

   MBSTR(pname1, RP->DataP);
   MBSTR(pname2, RP->DataP+FILENAMELEN);

   RP->Type = pkt->dp_Type;
   RP->Arg1 = (LONG)nlock;
   RP->Arg3 = (LONG)nlock2;
   RP->DLen = dlen;
   RP->handle = hpkt;

   hpkt->hp_RP = RP;
   hpkt->hp_NPFlags = NP1|NP3;
   hpkt->hp_NNode = nlock->NetNode;
   hpkt->hp_Driver = nlock->NetNode->driver;

   RemotePacket(global, hpkt);
}

/* Arg1: Unused */
/* Arg2: Lock */
/* Arg3: Name */
/* Arg4: Mask of protection */

ACTFN(ActSetProtection)
{
   struct FileLock *flock;
   NETPTR nlock;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP;

   OBUG(("ActSetProtection\n"));

   OBUGBSTR("File to protect: ", pkt->dp_Arg3);

   flock = (struct FileLock *)pkt->dp_Arg2;

   if(TESTROOT(flock, nlock))
   {
      if(ParseName(global, (char *)pkt->dp_Arg3, &nlock, global->work) ||
          !nlock)
      {
         /* Local lock - protection not implemented yet */
         pkt->dp_Res1 = DOS_TRUE;
         HPQ(global, hpkt);
         return;
      }
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
      if(!(RP = AllocRPacket(nlock->NetNode, 0)))
         goto norpkt;
      RP->DataP = (char *)pkt->dp_Arg3;
   }


   RP->Type = pkt->dp_Type;
   RP->Arg2 = (LONG)nlock;
   RP->Arg4 = pkt->dp_Arg4;
   RP->DLen = BSTRLEN(RP->DataP)+1;
   if(RP->DLen == 1) RP->DLen = 0;
   RP->handle = hpkt;

   hpkt->hp_RP = RP;
   hpkt->hp_NPFlags = NP2;
   hpkt->hp_NNode = nlock->NetNode;
   hpkt->hp_Driver = nlock->NetNode->driver;

   RemotePacket(global, hpkt);
}

/* Arg1: Unused */
/* Arg2: Lock */
/* Arg3: Name */
/* Arg4: Comment */

ACTFN(ActSetComment)
{
   struct FileLock *flock;
   NETPTR nlock;
   int len;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP;

   OBUG(("ActSetComment\n"));

   flock = (struct FileLock *)pkt->dp_Arg2;

   OBUGBSTR("File to Comment: ", pkt->dp_Arg3);
   OBUGBSTR("New Comment Str: ", pkt->dp_Arg4);

   if(TESTROOT(flock, nlock))
   {
      if(ParseName(global, (char *)pkt->dp_Arg3, &nlock, global->work) ||
          !nlock)
      {
         /* Can't SetComment local nodes */
         pkt->dp_Res1 = DOS_FALSE;
         pkt->dp_Res2 = ERROR_NODE_DOWN;
         HPQ(global, hpkt);
         return;
      }
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
      if(!(RP = AllocRPacket(nlock->NetNode, 0))) goto norpkt;
      RP->DataP = (char *)pkt->dp_Arg3;
   }

   RP->Type = pkt->dp_Type;
   RP->Arg2 = (LONG)nlock;
   len = BSTRLEN(RP->DataP)+1;
   len = (((len+3)>>2)<<2);
   MBSTR(pkt->dp_Arg4, (RP->DataP+len));
   RP->DLen = len + *((char *)pkt->dp_Arg4) + 1;
   RP->handle = hpkt;

   hpkt->hp_RP = RP;
   hpkt->hp_NPFlags = NP2;
   hpkt->hp_NNode = nlock->NetNode;
   hpkt->hp_Driver = nlock->NetNode->driver;

   RemotePacket(global, hpkt);
}

/* Arg1: Unused */
/* Arg2: BPTR Lock */
/* Arg3: BSTR Name */
/* Arg4: APTR !!!!!!!!! DateStamp*/

ACTFN(ActSetFileDate)
{
   struct FileLock *flock;
   NETPTR nlock;
   int len;
   char *name;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP;

   OBUG(("ActSetFileDate\n"));
   OBUG(("Arg1=0x%08lx Arg2=0x%08lx Arg3=0x%08lx Arg4=%08lx\n",
      pkt->dp_Arg1, pkt->dp_Arg2, pkt->dp_Arg3, pkt->dp_Arg4))

   flock = (struct FileLock *)pkt->dp_Arg2;

   if(TESTROOT(flock, nlock))
   {
      if(ParseName(global, (char *)pkt->dp_Arg3, &nlock, global->work) ||
          !nlock)
      {
         /* Can't SetDate local nodes */
         pkt->dp_Res1 = DOS_FALSE;
         pkt->dp_Res2 = ERROR_NODE_DOWN;
         HPQ(global, hpkt);
         return;
      }
      name = global->work;
   }
   else
   {
      name = pkt->dp_Arg3;
   }

   len = sizeof(struct DateStamp) + BSTRLEN(name) + 1;
   if(!(RP = AllocRPacket(nlock->NetNode, len)))
   {
      pkt->dp_Res1 = DOS_FALSE;
      pkt->dp_Res2 = ERROR_NO_FREE_STORE;
      HPQ(global, hpkt);
      return;
   }
   RP->Type = pkt->dp_Type;
   RP->Arg2 = (LONG)nlock;
   RP->DLen = len;
   memcpy(RP->DataP, (char *)pkt->dp_Arg4, sizeof(struct DateStamp));
   MBSTR(name, RP->DataP+sizeof(struct DateStamp));
   RP->handle = hpkt;

   hpkt->hp_RP = RP;
   hpkt->hp_NPFlags = NP2;
   hpkt->hp_NNode = nlock->NetNode;
   hpkt->hp_Driver = nlock->NetNode->driver;

   RemotePacket(global, hpkt);
}
