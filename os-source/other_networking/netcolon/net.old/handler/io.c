/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987, 1988 The Software Distillery.  All Rights  */
/* |. o.| || Reserved.  This program may not be distributed without the    */
/* | .  | || permission of the authors:                            BBS:    */
/* | o  | ||   John Toebes     Doug Walker    Dave Baker                   */
/* |  . |//                                                                */
/* ======                                                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* File Access:            */
/* ActRead ActWrite ActSeek ActWaitForChar    */
/* ActFindwrite ActFindin ActFindout ActEnd   */

#include "handler.h"

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                 ActRead( global, pkt )                                  */
/*                                                                         */
/*-------------------------------------------------------------------------*/

/* Arg1: APTR EFileHandle */
/* Arg2: APTR Buffer      */
/* Arg3: Length           */
ACTFN(ActRead)
{
   NETPTR nfile;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP;

   nfile = (NETPTR)pkt->dp_Arg1;

   if(pkt->dp_Arg3 == 0)
   {
      pkt->dp_Res1 =
      pkt->dp_Res2 = 0;
      HPQ(global, hpkt);
      return;
   }

   if(!(RP = AllocRPacket(nfile->NetNode, 0)))
   {
      pkt->dp_Res1 = -1;
      pkt->dp_Res2 = ERROR_NO_FREE_STORE;
      HPQ(global, hpkt);
      return;
   }

   /* I keep a running total of the number of bytes read in dp_Res1 */
   pkt->dp_Res1 = 0;
   pkt->dp_Res2 = (LONG)nfile;

   RP->Arg2 =
   RP->Type = (pkt->dp_Arg3 > global->n.NetBufSize 
                            ? ACTION_READ_MORE : ACTION_READ);
   RP->Arg4 = -1;  /* To let server know this is the first packet */
   RP->handle = hpkt;

   hpkt->hp_RP = RP;
   hpkt->hp_NPFlags = NP1;
   hpkt->hp_Func = ActRead2;
   hpkt->hp_NNode = nfile->NetNode;
   hpkt->hp_Driver = nfile->NetNode->driver;

   ActRead2(global, hpkt);
}

ACTFN(ActRead2)
{
   int amtread;
   struct RPacket *RP = hpkt->hp_RP;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   int eof = 0;

   if(RP->Arg4 != -1)
   {
      /* Not the first time in here, process the results of the last time */
      if(hpkt->hp_State == HPS_ERR || (amtread=RP->Arg1) < 0) 
      {
         if(hpkt->hp_State == HPS_ERR)
         {
            pkt->dp_Res1 = -1;  /* RemotePacket would have set it to 0 */
         }
         else
         {
            pkt->dp_Res1 = RP->Arg1;
            pkt->dp_Res2 = RP->Arg2;
         }
         /* hp_Func should be NULL, so the packet will get returned this time */
         return;
      }

      if(amtread > 0) memcpy((char *)pkt->dp_Arg2+pkt->dp_Res1, RP->DataP, amtread);
      pkt->dp_Res1 += amtread;

      /* Successful read for less than specified amount is EOF */
      if(amtread < RP->Arg3 || RP->Arg3 == 0) eof = 1;
   }
   else if(RP->Type == ACTION_READ)
   {
      /* Not READ_MORE, so the other side won't change Arg4 */
      /* We'd better do it so we know next time to do the above stuff */
      RP->Arg4 = -2;
   }

   if(pkt->dp_Res1 < pkt->dp_Arg3 && !eof)
   {
      /* Still some left to read - do it */
      /* Arg1 must be reinitialized each time since it will be trashed */
      /* with the dp_Res1 field by the remote side                     */
      RP->Arg1 = pkt->dp_Res2;
      RP->Arg3 = min(pkt->dp_Arg3-pkt->dp_Res1, global->n.NetBufSize);
      RP->DLen = 0;
      hpkt->hp_Func = ActRead2;

      RemotePacket(global, hpkt);
   }
   else
   {
      /* successful read                    */
      /* dp_Res1 is already set             */
      pkt->dp_Res2 = RP->Arg2;
      return;
   }

}

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                      ActWrite( global, pkt )                            */
/*                                                                         */
/*-------------------------------------------------------------------------*/

/* This should probably be changed to look for a confirmation after */
/* each write in order to notice errors faster - djw                */

/* Arg1: APTR EFileHandle */
/* Arg2: APTR Buffer */
/* Arg3: Length */

ACTFN(ActWrite)
{
   NETPTR nfile;
   struct RPacket *RP;
   struct DosPacket *pkt = hpkt->hp_Pkt;

   nfile = (NETPTR)pkt->dp_Arg1;

   if(pkt->dp_Arg3 == 0)
   {
      pkt->dp_Res1 =
      pkt->dp_Res2 = 0;
      HPQ(global, hpkt);
      return;
   }

   if(!(RP = AllocRPacket(nfile->NetNode, 0)))
   {
      pkt->dp_Res1 = -1;
      pkt->dp_Res2 = ERROR_NO_FREE_STORE;
      HPQ(global, hpkt);
      return;
   }

   pkt->dp_Res1 = 0L;
   pkt->dp_Res2 = (LONG)nfile;

   RP->Type = (pkt->dp_Arg3 > global->n.NetBufSize ? 
                              ACTION_WRITE_MORE : ACTION_WRITE);
   RP->Arg4 = -1;
   RP->handle = hpkt;

   hpkt->hp_RP = RP;
   hpkt->hp_Func = ActWrite2;
   hpkt->hp_NPFlags = NP1;
   hpkt->hp_NNode = nfile->NetNode;
   hpkt->hp_Driver = nfile->NetNode->driver;


   ActWrite2(global, hpkt);
}

ACTFN(ActWrite2)
{
   struct RPacket *RP = hpkt->hp_RP;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   int amtwrite;

   if(RP->Arg4 != -1)
   {
      /* Not the first time in here, process the results of the last time */
      if(hpkt->hp_State == HPS_ERR || (amtwrite=RP->Arg1) <= 0) 
      {
         if(hpkt->hp_State == HPS_ERR)
         {
            pkt->dp_Res1 = -1;  /* RemotePacket would have set it to 0 */
         }
         else
         {
            pkt->dp_Res1 = RP->Arg1;
            pkt->dp_Res2 = RP->Arg2;
         }
         return;
      }

      pkt->dp_Res1 += amtwrite;
   }
   else if(RP->Type == ACTION_WRITE)
   {
      /* Not WRITE_MORE, so the other side won't change Arg4          */
      /* We'd better do it so we know next time to do the above stuff */
      RP->Arg4 = -2;
   }


   if(pkt->dp_Res1 < pkt->dp_Arg3 && pkt->dp_Arg3 > 0)
   {
      RP->Arg1 = pkt->dp_Res2;  /* Must do this every time */
      RP->DLen = RP->Arg3 =
           min(pkt->dp_Arg3-pkt->dp_Res1, global->n.NetBufSize);
      RP->DataP = ((char *)pkt->dp_Arg2)+pkt->dp_Res1;
      hpkt->hp_Func = ActWrite2;

      RemotePacket(global, hpkt);
   }
   else
   {
      /* Write complete */
      pkt->dp_Res2 = RP->Arg2;
   }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                       ActSeek( global, pkt )                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/* Arg1: APTR EFileHandle */
/* Arg2: Position */
/* Arg3: Mode */

ACTFN(ActSeek)
{
    NETPTR nfile;
    struct RPacket *RP;
    struct DosPacket *pkt = hpkt->hp_Pkt;

    nfile = (NETPTR)pkt->dp_Arg1;

    if(!(RP = AllocRPacket(nfile->NetNode, 0)))
    {
      pkt->dp_Res1 = -1;
      pkt->dp_Res2 = ERROR_NO_FREE_STORE;
      HPQ(global, hpkt);
      return;
   }

    RP->Type = pkt->dp_Type;
    RP->Arg1 = (LONG)nfile;
    RP->Arg2 = pkt->dp_Arg2;
    RP->Arg3 = pkt->dp_Arg3;
    RP->DLen = 0;
    RP->handle = hpkt;

    hpkt->hp_RP = RP;
    hpkt->hp_NPFlags = NP1;
    hpkt->hp_NNode = nfile->NetNode;
    hpkt->hp_Driver = nfile->NetNode->driver;

    RemotePacket(global, hpkt);
}

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                    ActFindwrite( global, pkt )                          */
/*                                                                         */
/*-------------------------------------------------------------------------*/

/* ARG1: FileHandle to fill in */
/* ARG2: Lock for file relative to */
/* Arg3: Name of file */

ACTFN(ActFindwrite)
{
   struct FileHandle *fh;
   NETPTR nlock;
   struct FileLock *flock;
   struct RPacket *RP;
   struct DosPacket *pkt = hpkt->hp_Pkt;

   /* Code 1004 -
    *   If file does not exist, open should fail.
    *   If file does exist, open with an exclusive lock
    */

   fh = (struct FileHandle *)pkt->dp_Arg1;

   flock = (struct FileLock *)pkt->dp_Arg2;
   if(TESTROOT(flock, nlock))
   {
      if (ParseName(global, (char *)pkt->dp_Arg3, &nlock, global->work) ||
          !nlock)
      {
         /* Can't open files in NET: itself */
         pkt->dp_Res1 = NULL;
         pkt->dp_Res2 = (nlock   ? ERROR_OBJECT_NOT_FOUND
                                 : ERROR_OBJECT_WRONG_TYPE);
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
   if(!memcmp(RP->DataP, "\2::", 3))
   {
      /* Special name meaning nodename.info file on remote side */
      RP->Arg3 = 1;
      RP->DLen = 0;
   }
   else
   {
      RP->Arg3 = 0;
      RP->DLen = BSTRLEN(RP->DataP)+1;
      if (RP->DLen == 1)
          RP->DLen = 0;
   }
   RP->handle = hpkt;

   hpkt->hp_RP = RP;
   hpkt->hp_NPFlags = NP2;
   hpkt->hp_Func = ActFindwrite2;
   hpkt->hp_NNode = nlock->NetNode;
   hpkt->hp_Driver = nlock->NetNode->driver;

   pkt->dp_Res1 = (LONG)fh;
   pkt->dp_Res2 = (LONG)nlock;

   RemotePacket(global, hpkt);
}

ACTFN(ActFindwrite2)
{
   struct FileHandle *fh;
   NETPTR nfile;
   NETPTR nlock;
   struct RPacket *RP = hpkt->hp_RP;
   struct DosPacket *pkt = hpkt->hp_Pkt;

   if(hpkt->hp_State != HPS_ERR)
   {
      if(RP->Arg1 != DOS_FALSE)
      {
         fh = (struct FileHandle *)pkt->dp_Res1;
         nlock = (NETPTR)pkt->dp_Res2;

         if(RP->Arg1 != DOS_FALSE)
         {
            if (!(nfile=(NETPTR)DosAllocMem(global, (long)sizeof(struct NetPtr)))) 
            {
               BUG(("******NO MEMORY FOR LOCAL LOCK"));
               pkt->dp_Res1 = DOS_FALSE;
               pkt->dp_Res2 = ERROR_NO_FREE_STORE;
               return;
            }

            nfile->NetNode     = nlock->NetNode;
            /* Remote filehandle passed back in RP.Arg3 */
            nfile->RPtr        = (RNPTR)RP->Arg3;
            nfile->incarnation = nfile->NetNode->RootLock.incarnation;

            /* RP.Arg4 tells us if it's an interactive filehandle */

            fh->fh_Arg1 = (LONG)nfile;
         }
      }
      pkt->dp_Res1 = RP->Arg1;
      pkt->dp_Res2 = RP->Arg2;
   }
#if DEBUG
   else
   {
      BUG(("ActFind2: Write error\n"))
   }
#endif
}

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                       ActEnd( global, pkt )                             */
/*-------------------------------------------------------------------------*/

ACTFN(ActEnd)
{
   NETPTR nfile;
   struct RPacket *RP;

   nfile = (NETPTR)hpkt->hp_Pkt->dp_Arg1;

   if(!(RP = AllocRPacket(nfile->NetNode, 0)))
   {
      hpkt->hp_Pkt->dp_Res1 = DOS_FALSE;
      hpkt->hp_Pkt->dp_Res2 = ERROR_NO_FREE_STORE;
      HPQ(global, hpkt);
      return;
   }

   RP->Type = hpkt->hp_Pkt->dp_Type;
   RP->Arg1 = (LONG)nfile;
   RP->DLen = 0;
   RP->handle = hpkt;

   hpkt->hp_RP = RP;
   hpkt->hp_NNode = nfile->NetNode;
   hpkt->hp_Driver = nfile->NetNode->driver;
   hpkt->hp_NPFlags = NP1;

   hpkt->hp_Func = ActEnd2;
   hpkt->hp_Pkt->dp_Res1 = (LONG)nfile;

   RemotePacket(global, hpkt);
}

ACTFN(ActEnd2)
{
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP = hpkt->hp_RP;

   if(hpkt->hp_State != HPS_ERR)
   {
      if(pkt->dp_Res1) DosFreeMem((char *)pkt->dp_Res1);

      pkt->dp_Res1 = RP->Arg1;
      pkt->dp_Res2 = RP->Arg2;
   }
}


