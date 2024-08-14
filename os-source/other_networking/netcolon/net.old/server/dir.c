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
**      $Revision: 1.13 $
**      $Date: 91/01/15 01:06:21 $
**      $Log:	dir.c,v $
 * Revision 1.13  91/01/15  01:06:21  J_Toebes
 * Implement pseudo locks on handlers.
 * 
 * Revision 1.12  91/01/11  10:46:57  J_Toebes
 * Cleanup support for uplevel reference limits.  At the same time corrected
 * problems where cd net:john/assigned  would not return the asigned name
 * of the object
 * 
 * Revision 1.11  91/01/11  00:00:39  J_Toebes
 * Added code to compress the FIB portion of the return on an ACT_EXAMINE/EXNEXT
 * 
 * Revision 1.10  90/12/31  15:27:48  J_Toebes
 * Implement security restrictions for read/write access and prevention of
 * moving up too many levels.
 * Elmininated bogus Permit() call.
 * Changed LCHAIN structure to an ELOCK.
 * 
 * Revision 1.9  90/11/29  01:59:59  J_Toebes
 * Added chaining of locks for tracking.
 * 
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

   if ((port = GetRDevice(global, spkt, (ELOCK)spkt->sp_Pkt.dp_Arg1,
                                  spkt->sp_RP->DataP, ATTEMPT_WRITE)) == NULL)
   {
      return;
   }
   spkt->sp_Pkt.dp_Arg2 = (LONG)MKBADDR(spkt->sp_RP->DataP);
   spkt->sp_Pkt.dp_Arg3 = spkt->sp_RP->Arg3;

   Dispatch(global, spkt, ACTION_RETLOCKNAME, port);
}

ACTFN(RmtNameLockRet)
{
   BPTR lock;
   lock = spkt->sp_Pkt.dp_Res1;
   spkt->sp_RP->Arg2 = spkt->sp_Pkt.dp_Res2;

   /* Now get the full pathname of this lock so we can reestablish it */
   if(lock && spkt->sp_RP->Arg4)
   {
      spkt->sp_RP->DLen = 1;
      chainlock(global, spkt, lock);
      NameNode(global, spkt, lock);
   }
   else
   {
      chainlock(global, spkt, lock);
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
   ELOCK elock;

   BUG(("RmtExamine/RmtExNext - lock %lx type %ld\n", spkt->sp_RP->Arg1, spkt->sp_RP->Type));

   fib = (struct FileInfoBlock *)spkt->sp_RP->DataP;
   spkt->sp_Pkt.dp_Arg2 = (LONG)MKBADDR(fib);
   spkt->sp_Pkt.dp_Arg7 = 0;

   if (spkt->sp_RP->Type == ACTION_EXAMINE_OBJECT ||
       (spkt->sp_RP->DLen < sizeof(*fib)))
      memset(fib->fib_Reserved, 0, sizeof(fib->fib_Reserved));

   elock = (ELOCK)spkt->sp_Pkt.dp_Arg1;
   switch(testlock(global, spkt, elock))
   {
      case LOCK_ROOT:
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
            break;
         }
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
            break;
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
         break;

      case LOCK_PSEUDO:
         if (spkt->sp_RP->Type == ACTION_EXAMINE_NEXT)
         {
            spkt->sp_RP->Arg1 = DOSFALSE;
            spkt->sp_RP->Arg2 = ERROR_NO_MORE_ENTRIES;
            break;
         }
         /* Figure out which device it was we wanted to deal with */
         for(flag = elock->info.a.devpos, dev = spkt->sp_Ses->devices;
             flag > 0 && dev != NULL; dev = dev->next, flag--);
         if (dev == NULL)
         {
            spkt->sp_RP->Arg1 = DOS_FALSE;
            spkt->sp_RP->Arg2 = ERROR_OBJECT_NOT_FOUND;
            break;
         }
      
         fib->fib_DiskKey = (long)dev;
         fib->fib_DirEntryType = fib->fib_EntryType = dev->type;
         strcpy(fib->fib_FileName, &dev->len);
         fib->fib_Protection = dev->protect;
         fib->fib_Size = 0;
         fib->fib_NumBlocks = 0;
         fib->fib_Comment[0] = 0;
         break;

      case LOCK_NORMAL:
         if (spkt->sp_RP->Type == ACTION_EXAMINE_OBJECT &&
             elock->info.a.depth == 0)
         {
            spkt->sp_Pkt.dp_Arg7 = elock->info.all+1;
         }
      
         spkt->sp_Pkt.dp_Arg1 = elock->lock;
         spkt->sp_RP->DLen = sizeof(struct FileInfoBlock);
      
         Dispatch(global, spkt, ACTION_RETFIB,
                               ((struct FileLock *)BADDR(elock->lock))->fl_Task);
         break;
   }
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
   Dispatch(global, spkt, ACTION_RETFIB,
                  ((struct FileLock *)BADDR(spkt->sp_Pkt.dp_Arg1))->fl_Task);
}

ACTFN(RmtRetFIB)
{
   struct FileInfoBlock *fib;
   ACINFO info;
   struct DEVLIST *dev;

   fib = (struct FileInfoBlock *)spkt->sp_RP->DataP;

   if ((info.all = spkt->sp_Pkt.dp_Arg7) != 0)
   {
      for(dev = spkt->sp_Ses->devices; info.a.devpos > 0 && dev != NULL;
                       dev = dev->next, info.a.devpos--);
      if (dev == NULL)
      {
         spkt->sp_RP->Arg1 = DOSFALSE;
         spkt->sp_RP->Arg2 = ERROR_INVALID_LOCK;
      }
      fib->fib_DirEntryType = fib->fib_EntryType = dev->type;
      strcpy(fib->fib_FileName, &dev->len);
      fib->fib_Protection = dev->protect;
      fib->fib_Size = 0;
      fib->fib_NumBlocks = 0;
      fib->fib_Comment[0] = 0;
   }

   if (fib->fib_Comment[0] == 0)
#if 0
   /* We really don't need this check because it can't really occur. The
      reserved field is reserved because the original BCPL DOS commands did
      not allocate more than 80 bytes for the comment field that used to extend
      into what is now the reserved field.  However we leave the code here 
      if someone wants to be paranoid about it.                                 */
 &&
       !memcmp(fib->fib_Reserved,"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                 "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 36))
#endif
   {
      spkt->sp_RP->DLen -= (sizeof(fib->fib_Comment)+sizeof(fib->fib_Reserved))-1;
   }

   if (spkt->sp_Pkt.dp_Type != ACTION_FREE_LOCK)
   {
      spkt->sp_RP->Arg1 = spkt->sp_Pkt.dp_Res1;
      spkt->sp_RP->Arg2 = spkt->sp_Pkt.dp_Res2;
   }
}

ACTFN(RmtParent)
{
   /* Arg1 - Lock */
   ELOCK elock;

   BUG(("RmtParent\n"));

   elock = (ELOCK)spkt->sp_Pkt.dp_Arg1;

   switch(testlock(global, spkt, elock))
   {
      case LOCK_ROOT:
         spkt->sp_RP->Arg1 = 0;
         spkt->sp_RP->Arg2 = 0;
         break;

      case LOCK_PSEUDO:
      case LOCK_NORMAL:
         if (elock->info.a.depth <= 0)
         {
            spkt->sp_Pkt.dp_Res1 = 0;
            RmtParentRet(global, spkt);
            break;
         }
         spkt->sp_Pkt.dp_Arg1 = elock->lock;
         spkt->sp_Pkt.dp_Arg7 = elock->info.all-1;  /* Reduce the depth */
      
         Dispatch(global, spkt, ACTION_RETPARENT,
                 ((struct FileLock *)BADDR(spkt->sp_Pkt.dp_Arg1))->fl_Task);
         break;
   }
}

ACTFN(RmtParentRet)
{      
   spkt->sp_RP->Arg2 = spkt->sp_Pkt.dp_Res2;

   if (spkt->sp_Pkt.dp_Res1 == 0)
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
