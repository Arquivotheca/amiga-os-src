/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987, 1988 The Software Distillery.  All Rights  */
/* |. o.| || Reserved.  This program may not be distributed without the    */
/* | .  | || permission of the authors:                            BBS:    */
/* | o  | ||   John Toebes     Doug Walker    Dave Baker                   */
/* |  . |//                                                                */
/* ======                                                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Directory Manipulation */
/*  ActCreateDir ActExamine ActExNext ActParent */

#include "handler.h"

/* Arg1 - Lock */
/* Arg2 - name */
/* Arg3 (optional) Attributes */

ACTFN(ActCreateDir)
{
   struct FileLock *flock;
   NETPTR nlock;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP;

   OBUG(("ActCreateDir\n"));

   flock = (struct FileLock *)pkt->dp_Arg1;

   if(TESTROOT(flock, nlock))
   {
      if(ParseName(global, (char *)pkt->dp_Arg2, &nlock, global->work) 
                     || !nlock)
      {
         /* He is trying to create a directory in NET:, or node doesn't exist */
         OBUG(("ActCreateDir: Can't create dir in NET:, giving up\n"))
         pkt->dp_Res1 = NULL;
         pkt->dp_Res2 = ERROR_WRITE_PROTECTED;
         HPQ(global, hpkt);
         return;
      }
      /* Fall through, nlock is correct */
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

   RP->Type = pkt->dp_Type;
   RP->Arg1 = (LONG)nlock;
   RP->Arg3 = pkt->dp_Arg3;
   RP->Arg4 = 0;  /* So we don't get the full name */
   RP->DLen = BSTRLEN(RP->DataP)+1;
   RP->handle = hpkt;
   if (RP->DLen == 1)
        RP->DLen = 0;

   hpkt->hp_RP = RP;
   hpkt->hp_Func= ActCreateDir2;
   hpkt->hp_NPFlags = NP1;
   hpkt->hp_NNode = nlock->NetNode;
   hpkt->hp_Driver = nlock->NetNode->driver;

   pkt->dp_Res1 = (LONG)nlock;

   RemotePacket(global, hpkt);
}

ACTFN(ActCreateDir2)
{
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP = hpkt->hp_RP;

   if(hpkt->hp_State != HPS_ERR)
   {
      if(RP->Arg1 == DOS_FALSE)
      {
         pkt->dp_Res1 = DOS_FALSE;
      }
      else
      {
         /* dp_Res1 now holds the old nlock - soon to hold the local lock */

         pkt->dp_Res1 = MKBADDR(CreateLock(global, (NETPTR)pkt->dp_Res1,
                                  (RNPTR)RP->Arg1, ACCESS_READ, RP));
      }
      pkt->dp_Res2 = RP->Arg2;
   }
}

/*-------------------------------------------------------------------------*/
/* Structure of the FileInfoBlock                                          */
/* struct FileInfoBlock {                                                  */
/*    LONG fib_DiskKey;           Location on disk of entry                */
/*    LONG fib_DirEntryType;      <0 plain file, >0 a directory            */
/*    char fib_FileName[108];     Null terminated name of file             */
/*    LONG fib_Protection;        Protection bits for the file             */
/*    LONG fib_EntryType;         Internal for Dos use                     */
/*    LONG fib_Size;              Length of file in bytes                  */
/*    LONG fib_NumBlocks;         Length of file in blocks                 */
/*    struct DateStamp fib_Date;  File creation date                       */
/*    char fib_Comment[116];      Comment associated with file             */
/*    };                                                                   */
/*-------------------------------------------------------------------------*/

static char status[] = "\x20NET: by The Software Distillery.";

void ExDotInfo U_ARGS((GLOBAL, struct FileInfoBlock *));

void
ExDotInfo(global, info)
GLOBAL global;
struct FileInfoBlock *info;
{
    OBUGBSTR("ExDotInfo for ", info->fib_FileName);
    strcpy(info->fib_FileName+(info->fib_FileName[0]+1), ".info");
    info->fib_FileName[0] += 5;
    info->fib_EntryType =
    info->fib_DirEntryType = -3;
    info->fib_Protection = 0L;
    info->fib_Size = 974;
    info->fib_NumBlocks = 2;
    info->fib_Comment[0] = '\0';
}

static void ExRoot U_ARGS((GLOBAL, struct FileInfoBlock *));

static void
ExRoot(global, info)
GLOBAL global;
struct FileInfoBlock *info;
{
    OBUG(("ExRoot\n"))   
    /* Examine on the root of NET: */
    info->fib_DiskKey = (LONG)&global->netchain;
    info->fib_DirEntryType = info->fib_EntryType = 2;
    MBSTR(global->netchain.name, info->fib_FileName);
    info->fib_Protection = 0x5; /* ----r-e- */
    info->fib_NumBlocks = 1;
    info->fib_Size = 0;
    info->fib_Date.ds_Days = 3000;  /* Pick a number */
    info->fib_Date.ds_Minute =
    info->fib_Date.ds_Tick = 0L;
    strcpy(info->fib_Comment, status);
}

static void ExRootNext U_ARGS((GLOBAL, struct FileInfoBlock *, struct DosPacket *));

static void
ExRootNext(global, info, pkt)
GLOBAL global;
struct FileInfoBlock *info;
struct DosPacket *pkt;
{
    struct NetNode *node;

    /* Examine next in the root of NET: */
    OBUG(("ExRootNext\n"))

    node = (struct NetNode *)info->fib_DiskKey;

    /* If the node name is the same length as the filename, we haven't */
    /* returned the exnext of the .info file yet;  do so               */
    if (node != &global->netchain &&
        info->fib_FileName[0] > 0 &&
        node->name[0] == info->fib_FileName[0])
    {
        ExDotInfo(global, info);
        return;
    }

#if 0
    for (node=node->next; node && node->status != NODE_UP; node=node->next);
#else
    node = node->next;
#endif

    if (!node) 
    {
        pkt->dp_Res1 = DOS_FALSE;
        pkt->dp_Res2 = ERROR_NO_MORE_ENTRIES;
        OBUG(("Returning ERROR_NO_MORE_ENTRIES\n"));
        return;
    }

    info->fib_DiskKey = (LONG)node;
    info->fib_DirEntryType = info->fib_EntryType = 2;
    MBSTR(node->name, info->fib_FileName);
    OBUGBSTR("ExRootNext: Filename is ", info->fib_FileName);
    OBUG(("ExRootNext: Node status is %d\n", node->status))
    info->fib_Protection = 0L;
    info->fib_NumBlocks = 1;
    info->fib_Size = 0;
    info->fib_Date.ds_Days = 3000;
    info->fib_Date.ds_Minute =
    info->fib_Date.ds_Tick = 0L;
    info->fib_Comment[0] = '\0';
}

/* Arg1: Lock of object to examine */
/* Arg2: FileInfoBlock to fill in */

ACTFN(ActExamine)
{
   NETPTR nlock;
   struct FileLock *flock;
   struct FileInfoBlock *info;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP;

   OBUG(("ActExamine/ActExNext: Entry, hpkt 0x%08lx\n", hpkt));

   flock = (struct FileLock *)pkt->dp_Arg1;
   info = (struct FileInfoBlock *)pkt->dp_Arg2;

   if(TESTROOT(flock, nlock))
   {
      pkt->dp_Res1 = DOS_TRUE;
      if(pkt->dp_Type == ACTION_EXAMINE_OBJECT)
         ExRoot(global, info);
      else
         ExRootNext(global, info, pkt);
      HPQ(global, hpkt);
      OBUG(("     ActExamine: HPQ done, root object \n"))
      return;
   }

   if(!(RP = AllocRPacket(nlock->NetNode, 0)))
   {
      pkt->dp_Res1 = DOS_FALSE;
      pkt->dp_Res2 = ERROR_NO_FREE_STORE;
      HPQ(global, hpkt);
      OBUG(("     ActExamine: HPQ done, no RPacket\n"))
      return;
   }
   RP->DataP = (char *)info;
   RP->DLen = sizeof(struct FileInfoBlock) - sizeof(info->fib_Reserved);
   if(info->fib_Comment[0] == 0) RP->DLen -= sizeof(info->fib_Comment);

   OBUG(("ActEx: Lock %lx on node %s\n",nlock->RPtr,nlock->NetNode->name+1));
   RP->Type = pkt->dp_Type;
   RP->Arg1 = (LONG)nlock;
   RP->handle = hpkt;

   hpkt->hp_RP = RP;
   hpkt->hp_Func = ActExamine2;
   hpkt->hp_NPFlags = NP1;
   hpkt->hp_NNode = nlock->NetNode;
   hpkt->hp_Driver = nlock->NetNode->driver;

   RemotePacket(global, hpkt);
}

ACTFN(ActExamine2)
{
   struct FileInfoBlock *info;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP = hpkt->hp_RP;

   OBUG(("ActExamine2: Entry, hpkt 0x%08lx\n", hpkt))

   if(hpkt->hp_State != HPS_ERR)
   {
      OBUG(("ActExamine2: RC %d %d\n", RP->Arg1, RP->Arg2));
      if(RP->Arg1 != DOS_FALSE)
      {
         info = (struct FileInfoBlock *)pkt->dp_Arg2;
         MQ(RP->DataP, info, RP->DLen);
         OBUGBSTR("   ActExamine2: Returned filename ", info->fib_FileName);
      }
      pkt->dp_Res1 = RP->Arg1;
      pkt->dp_Res2 = RP->Arg2;
   }
#if DEBUG
   else
   {
      BUG(("ActExamine2: Error during send\n"))
   }
#endif

}



/* Arg1 - Lock */

ACTFN(ActParent)
{
   struct FileLock *flock;
   NETPTR nlock;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP;

   OBUG(("ActParent: Entry, hpkt %08lx\n", hpkt));

   flock = (struct FileLock *)pkt->dp_Arg1;

   /* If this is the root of NET:, return NULL */
   if(TESTROOT(flock, nlock))
   {
      pkt->dp_Res1 = NULL;
      pkt->dp_Res2 = 0;
      HPQ(global, hpkt);
      OBUG(("ActParent: parent of root, returning NULL\n"))
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
   hpkt->hp_NPFlags = NP1;
   hpkt->hp_Func = ActParent2;
   hpkt->hp_NNode = nlock->NetNode;
   hpkt->hp_Driver = nlock->NetNode->driver;
   pkt->dp_Res1 = (LONG)nlock;

   RemotePacket(global, hpkt);
}

ACTFN(ActParent2)
{
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP = hpkt->hp_RP;
   NETPTR nlock;

   OBUG(("ActParent2: Entry, hpkt 0x%08lx\n", hpkt))

   if(hpkt->hp_State != HPS_ERR)
   {
      nlock = (NETPTR)pkt->dp_Res1;
      pkt->dp_Res2 = RP->Arg2;

      if(RP->Arg1 == NULL)
      {
         pkt->dp_Res1 = NULL;
         if(pkt->dp_Res2) return;  /* Error on other side */

         /* Null lock from other side, return a lock on our root */
         nlock = &global->netchain.RootLock;
         RP->Arg1 = 1;
      }

      pkt->dp_Res1 = MKBADDR(CreateLock(global, nlock, (RNPTR)RP->Arg1, 
                             ACCESS_READ, RP));
   }
#if DEBUG
   else
   {
      BUG(("ActParent2: Error during send\n"))
   }
#endif
}

/* Arg1: Lock */
/* Arg2: Name relative to Arg1 */
/* Arg3: lock on target object(hard links) or name (soft links) */
/* Arg4: LINK_SOFT or LINK_HARD */
ACTFN(ActMakeLink)
{
  NETPTR nlock, nlock2;
  struct FileLock *flock;
  struct DosPacket *pkt = hpkt->hp_Pkt;
  struct RPacket *RP;
  int len;
  LONG arg3;

  OBUG(("ActMakeLink: Entry, hpkt 0x%08lx\n", hpkt))
  OBUGBSTR("Link to create: ", pkt->dp_Arg2);
  
  flock = (struct FileLock *)pkt->dp_Arg1;
  if(TESTROOT(flock, nlock))
  {
     if(ParseName(global, (char *)pkt->dp_Arg2, &nlock, global->work))
     {
        OBUG(("ActMakeLink: Can't create link in root\n"))
        pkt->dp_Res2 = ERROR_WRITE_PROTECTED;
        goto errcase;
     }
     else if(!nlock)
     {
        OBUG(("ActMakeLink: Can't find node\n"))
        pkt->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
errcase:
        pkt->dp_Res1 = DOS_FALSE;
        HPQ(global, hpkt);
        return;
     }
  }
  else
  {
     OBUGBSTR("ActLock: Relative to node ", nlock->NetNode->name);
     MBSTR(pkt->dp_Arg2, global->work);
  }

  if(pkt->dp_Arg4 == LINK_HARD)
  {
     /* Arg3 is a lock on the object to be linked */
     flock = (struct FileLock *)pkt->dp_Arg3;
     if(TESTROOT(flock, nlock2))
     {
        if(ParseName(global, "", &nlock2, global->work+global->work[0]+1)
               || !nlock2)
        {
           pkt->dp_Res2 = ERROR_RENAME_ACROSS_DEVICES;
           goto errcase;
        }
     }
     if(nlock2->NetNode != nlock->NetNode)
     {
        pkt->dp_Res2 = ERROR_RENAME_ACROSS_DEVICES;
        goto errcase;
     }
     len = global->work[0] + 1;
     arg3 = (LONG)nlock2;
  }
  else
  {
     /* Arg3 is the name of the object to be linked to */
     MBSTR(pkt->dp_Arg3, global->work+BSTRLEN(global->work)+1);
     len = global->work[0] + BSTRLEN(pkt->dp_Arg3)+2;
     arg3 = 0;
  }

  if(!(RP = AllocRPacket(nlock->NetNode, nlock->NetNode->bufsize)))
  {
     norpkt:
     pkt->dp_Res1 = DOS_FALSE;
     pkt->dp_Res2 = ERROR_NO_FREE_STORE;
     HPQ(global, hpkt);
     return;
  }
  memcpy(RP->DataP, global->work, len);
  RP->DLen = len;

  RP->Type = pkt->dp_Type;
  RP->Arg1 = (LONG)nlock;
  RP->Arg3 = arg3;
  RP->Arg4 = pkt->dp_Arg4;

  hpkt->hp_RP = RP;
  hpkt->hp_NPFlags = NP1|NP3;
  hpkt->hp_NNode = nlock->NetNode;
  hpkt->hp_Driver = nlock->NetNode->driver;

  RemotePacket(global, hpkt);
}

/* Arg1: Lock */
/* Arg2: Path and name of link (relative to arg1) */
/* Arg3: (output) buffer for new path string */
/* Arg4: size of buffer in bytes */
ACTFN(ActReadLink)
{
  NETPTR nlock;
  struct FileLock *flock;
  struct DosPacket *pkt = hpkt->hp_Pkt;
  struct RPacket *RP;
  struct Library *DriverBase;
  int len;

/* READLINK WILL NOT WORK because the RPacket here is allocated BEFORE we */
/* know what NetNode to use.  Fix This. */
  BUGR("ActReadLink Ain't Gonna Work");

return;

#if 0

  OBUG(("ActReadLink: Entry, hpkt 0x%08lx name '%s'\n", hpkt, pkt->dp_Arg2))

  if(!(RP = AllocRPacket(nlock->NetNode, nlock->NetNode->bufsize)))
  {
     pkt->dp_Res1 = DOS_FALSE;
     pkt->dp_Res2 = ERROR_NO_FREE_STORE;
     HPQ(global, hpkt);
     return;
  }
  
  if((len=strlen((char *)pkt->dp_Arg2)) > 256)
  {
     OBUG(("ActReadLink: NAME TOO LONG!\n"))
     BUGR("Name too long in ReadLink");
     pkt->dp_Res2 = ERROR_LINE_TOO_LONG;
     goto errcase;
  }
  global->work[0] = len;
  memcpy(global->work+1, (char *)pkt->dp_Arg2, len);

  flock = (struct FileLock *)pkt->dp_Arg1;
  if(TESTROOT(flock, nlock))
  {
     if(ParseName(global, global->work, &nlock, RP->DataP))
     {
        OBUG(("ActReadLink: No links in root\n"))
        pkt->dp_Res2 = ERROR_WRITE_PROTECTED;
        goto errcase;
     }
     else if(!nlock)
     {
        OBUG(("ActReadLink: Can't find node\n"))
        pkt->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
        goto errcase;
     }
     RP->DLen = BSTRLEN(RP->DataP);
  }
  else
  {
     OBUGBSTR("ActReadLink: Relative to node ", nlock->NetNode->name);
     memcpy(RP->DataP, global->work, len);
     RP->DLen = len+1;
  }

  RP->Type = pkt->dp_Type;
  RP->Arg1 = (LONG)nlock;
  RP->Arg4 = pkt->dp_Arg4;

  hpkt->hp_RP = RP;
  hpkt->hp_NPFlags = NP1;
  hpkt->hp_NNode = nlock->NetNode;
  hpkt->hp_Driver = nlock->NetNode->driver;
  hpkt->hp_Func = ActReadLink2;

  RemotePacket(global, hpkt);

  return;

errcase:
  pkt->dp_Res1 = DOS_FALSE;
  HPQ(global, hpkt);
  DriverBase = hpkt->hp_Driver->libbase;
  SDNFreeRPacket(hpkt->hp_Driver->drglobal, RP);
  return;
#endif
}

ACTFN(ActReadLink2)
{
  struct DosPacket *pkt = hpkt->hp_Pkt;
  struct RPacket *RP = hpkt->hp_RP;
  
  if(hpkt->hp_State == HPS_ERR)
  {
     pkt->dp_Res1 = -1;
  }
  else
  {
     pkt->dp_Res1 = RP->Arg1;
     pkt->dp_Res2 = RP->Arg2;
     if(RP->DLen > 0)
        memcpy((char *)pkt->dp_Arg3, RP->DataP, RP->DLen);
  }
}
