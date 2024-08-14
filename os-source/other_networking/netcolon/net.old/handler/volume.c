/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987, 1988 The Software Distillery.  All Rights  */
/* |. o.| || Reserved.  This program may not be distributed without the    */
/* | .  | || permission of the authors:                            BBS:    */
/* | o  | ||   John Toebes     Doug Walker    Dave Baker                   */
/* |  . |//                                                                */
/* ======                                                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Volume Manipulation */
/* ActCurentVol  ActRenameDisk ActDiskInfo ActInfo ActDiskChange*/
/*
**      $Filename: volume.c $ 
**      $Author: Doug $
**      $Revision: 1.11 $
**      $Date: 91/01/06 20:51:32 $
**      $Log:	volume.c,v $
 * Revision 1.11  91/01/06  20:51:32  Doug
 * Revise debugging messages
 * 
 * Revision 1.10  90/12/31  00:58:04  Doug
 * Fix ActInfo
 * 
 * Revision 1.9  90/12/30  15:48:31  Doug
 * Delete ActNetKludge
 * 
**
**/

#include "handler.h"

static void GetInfo(GLOBAL, struct DosPacket *, struct InfoData *);

ACTFN(ActCurentVol)
{
   struct DosPacket *pkt = hpkt->hp_Pkt;
   OBUG(("ActCurentVol\n"));
   pkt->dp_Res1 = MKBADDR(global->volume);
   HPQ(global, hpkt);
}

ACTFN(ActRenameDisk)
{
   struct DosPacket *pkt = hpkt->hp_Pkt;
   OBUG(("ActRenameDisk\n"));

   DisMount(global);
   Mount(global, (char *)pkt->dp_Arg1);

   pkt->dp_Res1 = DOS_TRUE;
   HPQ(global, hpkt);
}

static void GetInfo(GLOBAL global, struct DosPacket *pkt, struct InfoData *info)
{
   OBUG(("GetInfo\n"));

   if (global->volume == NULL) 
   {
      info->id_DiskType = ID_NO_DISK_PRESENT;
      pkt->dp_Res1 = DOS_FALSE;
   } 
   else 
   {
      info->id_NumSoftErrors = global->n.ErrorCount;
      info->id_UnitNumber    = global->unitnum;
      info->id_DiskState     = ID_VALIDATED;
      info->id_DiskType      = ID_DOS_DISK;
      info->id_NumBlocks     = global->numnodes;
      info->id_NumBlocksUsed = global->upnodes;
      info->id_VolumeNode    = MKBADDR(global->volume);
      info->id_InUse         = 0;
      pkt->dp_Res1 = DOS_TRUE;
   }
   OBUG(("GetInfo: Exit\n"));
}

ACTFN(ActDiskInfo)
{
   struct DosPacket *pkt = hpkt->hp_Pkt;
   OBUG(("ActDiskInfo: Entry, hpkt 0x%08lx\n", hpkt));
   GetInfo(global, pkt, (struct InfoData *)pkt->dp_Arg1);
   HPQ(global, hpkt);
}

ACTFN(ActInfo)
{
   struct FileLock *flock;
   NETPTR nlock;
   struct RPacket *RP;
   struct DosPacket *pkt = hpkt->hp_Pkt;

   OBUG(("ActInfo: Entry, hpkt 0x%08lx\n", hpkt));

   flock = (struct FileLock *)pkt->dp_Arg1;

   if(TESTROOT(flock, nlock) || flock->fl_Volume != MKBADDR(global->volume))
   {
      GetInfo(global, pkt, (struct InfoData *)pkt->dp_Arg2);
      HPQ(global, hpkt);
   } 
   else 
   {
      /* It is a remote lock - query the remote fs */
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
      hpkt->hp_Func = ActInfo2;
      hpkt->hp_NPFlags = NP1;
      hpkt->hp_NNode = nlock->NetNode;
      hpkt->hp_Driver = nlock->NetNode->driver;

      RemotePacket(global, hpkt);
   }
}

ACTFN(ActInfo2)
{
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP = hpkt->hp_RP;
   struct InfoData *info;

   OBUG(("ActInfo2: Entry, hpkt 0x%08lx\n", hpkt))

   if(hpkt->hp_State != HPS_ERR)
   {
      if(RP->Arg1 == DOS_TRUE)
      {
         info = (struct InfoData *)pkt->dp_Arg2;
         MQ(RP->DataP, pkt->dp_Arg2, sizeof(struct InfoData));
         /* Override remote volume with ours */
         info->id_VolumeNode = MKBADDR(global->volume);
      }
      pkt->dp_Res1 = RP->Arg1;
      pkt->dp_Res2 = RP->Arg2;
   }
#if DEBUG
   else
   {
      BUG(("ActInfo2: Error writing\n"))
   }
#endif
}

ACTFN(ActDiskChange)
{
   OBUG(("ActDiskChange\n"));
   hpkt->hp_Pkt->dp_Res1 = ERROR_ACTION_NOT_KNOWN;
   HPQ(global, hpkt);
}


ACTFN(ActIsFS)
{
   OBUG(("ActIsFS\n"))
   hpkt->hp_Pkt->dp_Res1 = DOS_TRUE;
   HPQ(global, hpkt);
}

