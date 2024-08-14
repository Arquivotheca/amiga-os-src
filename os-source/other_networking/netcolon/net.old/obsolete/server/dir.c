/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987, 1988 The Software Distillery.  All Rights  */
/* |. o.| || Reserved.  This program may not be distributed without the    */
/* | .  | || permission of the authors:                            BBS:    */
/* | o  | ||   John Toebes     Doug Walker    Dave Baker                   */
/* |  . |//                                                                */
/* ======                                                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Directory Manipulation */
/*  RmtCreateDir RmtExamine RmtExNext RmtParent */
/*
**      $Filename: dir.c $ 
**      $Author: J_Toebes $
**      $Revision: 1.8 $
**      $Date: 90/11/28 01:54:04 $
**      $Log:	dir.c,v $
 * Revision 1.8  90/11/28  01:54:04  J_Toebes
 * Added support for icons.
 * 
 * Revision 1.7  90/11/23  15:07:32  J_Toebes
 * Added in session support of access control lists.
 * 
 * Revision 1.6  90/11/05  06:56:09  J_Toebes
 * Major rewrite to make server multi-threaded with support for multiple
 * devices.  At this point in time it is not debugged and the concept of
 * sessions is missing.
 * 
**/
#include "server.h"

ACTFN(RmtCreateDir)
{
   /* Arg1 - Lock */
   /* Arg2 - name */
   /* Arg3 (optional) Attributes */
   struct MsgPort *port;
   BUG(("RmtCreateDir\n"));
   BUGBSTR("Creating directory '%s'\n", spkt->sp_RP->DataP);

   if ((port = GetRDevice(global, spkt, spkt->sp_Pkt.dp_Arg1,
                                  spkt->sp_RP->DataP)) == NULL)
   {
      return;
   }
   spkt->sp_Pkt.dp_Arg2 = (LONG)MKBADDR(spkt->sp_RP->DataP);
   spkt->sp_Pkt.dp_Arg3 = spkt->sp_RP->Arg3;

   Dispatch(global, spkt, ACTION_RETLOCKNAME, port);
}

ACTFN(RmtNameLockRet)
{
   spkt->sp_RP->Arg1 = spkt->sp_Pkt.dp_Res1;
   spkt->sp_RP->Arg2 = spkt->sp_Pkt.dp_Res2;

   /* Now get the full pathname of this lock so we can reestablish it */
   if(spkt->sp_RP->Arg1 && spkt->sp_RP->Arg4)
   {
      BPTR lock;
      lock = spkt->sp_RP->Arg1;
      spkt->sp_RP->DLen = 1;
      chainlock(global, spkt, lock);
      NameNode(global, spkt, lock);
   }
   else
   {
      chainlock(global, spkt, spkt->sp_RP->Arg1);
      spkt->sp_RP->DLen = 0;
   }
}

ACTFN(RmtExamine)
{
   /* Arg1: Lock of object to examine */
   /* Arg2: FileInfoBlock to fill in */

   struct FileInfoBlock *fib;
   struct DEVLIST *dev;
   long key;
   int flag;

   BUG(("RmtExamine/RmtExNext - lock %lx type %ld\n", spkt->sp_RP->Arg1, spkt->sp_RP->Type));

   fib = (struct FileInfoBlock *)spkt->sp_RP->DataP;
   spkt->sp_Pkt.dp_Arg2 = (LONG)MKBADDR(fib);

   if (spkt->sp_RP->Arg1 == ROOTLOCK)
   {
      if (spkt->sp_RP->Type == ACTION_EXAMINE_OBJECT)
      {
         BUG(("Examine on root\n"));
         fib->fib_DiskKey = NULL;
         fib->fib_DirEntryType = fib->fib_EntryType = 2;
         strcpy(fib->fib_FileName, spkt->sp_Ses->name);
         fib->fib_Protection = FIBF_WRITE|FIBF_DELETE|FIBF_EXECUTE;
         fib->fib_Size = 0;
         fib->fib_NumBlocks = 0;
         fib->fib_Comment[0] = 0;
         spkt->sp_RP->Arg1 = DOSTRUE;
      }
      else
      {
         dev = spkt->sp_Ses->devices;
         BUG(("Exnext on root\n"));
         flag = 0;
         if (fib->fib_DiskKey)
         {
            flag = fib->fib_DiskKey & 1;
            key = fib->fib_DiskKey & ~1;
            while(dev != NULL && ((long)dev != key))
               dev = dev->next;
            if (dev && !flag) dev = dev->next;
         }
         if (dev == NULL)
         {
            spkt->sp_RP->Arg1 = DOSFALSE;
            spkt->sp_RP->Arg2 = ERROR_NO_MORE_ENTRIES;
            return;
         }
         /* If flag is set, it is time for us to look for the .info file */
         /* We will first attempt to find a file of the form:            */
         /* devs:networks/icons/<name>.info                              */
         /* If that fails we will then look for one of the form          */
         /* devs:networks/icons/__default__.info                         */
         /* Failing that we will ignore the .info file and go on to the  */
         /* next entry in the chain.  This will be slightly tricky to    */
         /* make this work.                                              */
         if (flag)
         {
            spkt->sp_Pkt.dp_Type = ACTION_LOCATE_OBJECT;
            spkt->sp_Pkt.dp_Arg1 = global->configlock;
            spkt->sp_Pkt.dp_Arg2 = MKBADDR(spkt->sp_RP->DataP);
            spkt->sp_Pkt.dp_Arg3 = SHARED_LOCK;
            spkt->sp_Pkt.dp_Arg4 = (long)dev;
            strcpy(spkt->sp_RP->DataP, "\x0Bicons/"); /* Account for icons/.info */
            strcpy(spkt->sp_RP->DataP+7, dev->name);
            spkt->sp_RP->DataP[0] += dev->len;
            strcpy(spkt->sp_RP->DataP+7+dev->len, ".info");
            spkt->sp_RP->DLen = sizeof(struct FileInfoBlock);

            Dispatch(global, spkt, ACTION_FINDINFO,
                     ((struct FileLock *)BADDR(spkt->sp_Pkt.dp_Arg1))->fl_Task);
         }
         else
         {
            fib->fib_DiskKey = (long)dev;
            if (global->configlock) fib->fib_DiskKey |= 1;
            fib->fib_DirEntryType = fib->fib_EntryType = dev->type;
            strcpy(fib->fib_FileName, &dev->len);
            fib->fib_Protection = dev->protect;
            fib->fib_Size = 0;
            fib->fib_NumBlocks = 0;
            fib->fib_Comment[0] = 0;
         }
         spkt->sp_RP->Arg1 = DOSTRUE;
         Permit();
      }
      return;
   }
    
   spkt->sp_RP->DLen = sizeof(struct FileInfoBlock);

   Dispatch(global, spkt, ACTION_RETURN,
            ((struct FileLock *)BADDR(spkt->sp_Pkt.dp_Arg1))->fl_Task);
}

ACTFN(RmtFindInfo)
{
   struct FileInfoBlock *fib;
   fib = (struct FileInfoBlock *)spkt->sp_RP->DataP;
   if (spkt->sp_Pkt.dp_Res1)
   {
      spkt->sp_Pkt.dp_Type = ACTION_EXAMINE_OBJECT;
      spkt->sp_Pkt.dp_Arg1 = spkt->sp_Pkt.dp_Res1;
      spkt->sp_Pkt.dp_Arg2 = (LONG)MKBADDR(fib);
      /* Lock succeeded */
      Dispatch(global, spkt, ACTION_RETINFO, 
                     ((struct FileLock *)BADDR(spkt->sp_Pkt.dp_Arg1))->fl_Task);
   }
   else if (strcmp(spkt->sp_RP->DataP, DEFICON))
   {
      strcpy(spkt->sp_RP->DataP, DEFICON);
      Dispatch(global, spkt, ACTION_FINDINFO,
                     ((struct FileLock *)BADDR(spkt->sp_Pkt.dp_Arg1))->fl_Task);
   }
   else
   {
      /* End of the road on this trial, default to no .info and go on to */
      /* the next entry                                                  */
      spkt->sp_RP->Arg1 = ROOTLOCK;
      spkt->sp_RP->Type = ACTION_EXAMINE_NEXT;
      fib->fib_DiskKey = spkt->sp_Pkt.dp_Arg4;
      RmtExamine(global, spkt);
   }
}

ACTFN(RmtDoneInfo)
{
   struct FileInfoBlock *fib;
   struct DEVLIST *dev;

   spkt->sp_RP->Arg1 = spkt->sp_Pkt.dp_Res1;
   spkt->sp_RP->Arg2 = spkt->sp_Pkt.dp_Res2;
   spkt->sp_RP->DLen = sizeof(struct FileInfoBlock);
   fib = (struct FileInfoBlock *)spkt->sp_RP->DataP;
   fib->fib_DiskKey = spkt->sp_Pkt.dp_Arg4;
   dev = (struct DEVLIST *)fib->fib_DiskKey;
   /* We also have to correct the name! */
   strcpy(fib->fib_FileName, &dev->len);
   strcpy(fib->fib_FileName+1+dev->len, ".info");
   fib->fib_FileName[0] += 5;
   spkt->sp_Pkt.dp_Type = ACTION_FREE_LOCK;
   Dispatch(global, spkt, ACTION_RWMORE3,
                  ((struct FileLock *)BADDR(spkt->sp_Pkt.dp_Arg1))->fl_Task);
}

ACTFN(RmtParent)
{
   /* Arg1 - Lock */

   BUG(("RmtParent\n"));

   if (spkt->sp_RP->Arg1 == ROOTLOCK)
   {
      spkt->sp_RP->Arg1 = 0;
      spkt->sp_RP->Arg2 = 0;
      return;
   }
   spkt->sp_Pkt.dp_Arg1 = spkt->sp_RP->Arg1;

   Dispatch(global, spkt, ACTION_RETPARENT,
           ((struct FileLock *)BADDR(spkt->sp_Pkt.dp_Arg1))->fl_Task);
}

ACTFN(RmtParentRet)
{      
   spkt->sp_RP->Arg1 = spkt->sp_Pkt.dp_Res1;
   spkt->sp_RP->Arg2 = spkt->sp_Pkt.dp_Res2;

   if (spkt->sp_RP->Arg1 == 0)
   {
      spkt->sp_RP->Arg1 = ROOTLOCK;
      if (spkt->sp_RP->Arg4)
      {
         spkt->sp_RP->DLen = 2;
         spkt->sp_RP->DataP[0] = 1;
         spkt->sp_RP->DataP[1] = ':';
      }
      return;
   }
   chainlock(global, spkt, spkt->sp_RP->Arg1);
   RmtNameLockRet(global, spkt);
}

