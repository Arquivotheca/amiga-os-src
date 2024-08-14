/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987, 1988 The Software Distillery.  All Rights  */
/* |. o.| || Reserved.  This program may not be distributed without the    */
/* | .  | || permission of the authors:                            BBS:    */
/* | o  | ||   John Toebes     Doug Walker    Dave Baker                   */
/* |  . |//                                                                */
/* ======                                                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Lock.c - lock manipulation */
/* RmtLock, RmtDupLock, RmtUnLock */
/*
**      $Filename: lock.c $ 
**      $Author: J_Toebes $
**      $Revision: 1.9 $
**      $Date: 90/11/28 01:51:44 $
**      $Log:	lock.c,v $
 * Revision 1.9  90/11/28  01:51:44  J_Toebes
 * Added support for icons.
 * 
 * Revision 1.8  90/11/18  23:26:17  J_Toebes
 * Corrected typo
 * 
 * Revision 1.7  90/11/18  22:52:27  J_Toebes
 * Fixed state machine for namelock to free the locks obtained when
 * traversing the file name.
 * 
 * Revision 1.6  90/11/15  08:17:47  J_Toebes
 * Correct multiple bugs associated with new version.  Sessions still are not supported.
 * 
 * Revision 1.5  90/11/05  06:56:13  J_Toebes
 * Major rewrite to make server multi-threaded with support for multiple
 * devices.  At this point in time it is not debugged and the concept of
 * sessions is missing.
 * 
*/
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

#include "server.h"

ACTFN(RmtLock)
{
   struct MsgPort *port;
   char *s, *t;
   int len;
   long lock;

   BUG(("RmtLock: lock %lx\n", spkt->sp_RP->Arg1));
   BUGBSTR("Locking filename = ", spkt->sp_RP->DataP);

   spkt->sp_Pkt.dp_Arg5 = 0;
   lock = spkt->sp_Pkt.dp_Arg1;

   if ((port = GetRDevice(global, spkt, lock, spkt->sp_RP->DataP)) == NULL)
   {
      /* We didn't find it in the list.  See if we are dealing with the */
      /* Root lock by chance and they are asking for a .info file       */
      if (lock != NULL && lock != ROOTLOCK)
         return;

      /* Make sure the name really ends with .info                      */
      t = spkt->sp_RP->DataP;
      len = t[0];
      if (len < 5 || memcmp(t+len-5, ".info", 5))
         return;

      /* Ok, let's just truncate the name and see if we can find it on the */
      /* List without the .info:                                           */
      t[0] -= 6;
      spkt->sp_Pkt.dp_Arg1 = lock;
      if ((port = GetRDevice(global, spkt, lock, t)) == NULL)
         return;

      /* Ok, there is a device and the name ended in .info.  Now all we need */
      /* to do is locate the .info file for them.  It will either be in      */
      /* devs:networks/icons/name.info or devs:networks/icons/__DEFAULT__.info */
      /* The name should now be in the form name: without the .info.         */
      len = t[0]-1; /* Ignore the ':' */
      s = t+len;
      strcpy(s+7, ".info");
      while(len-- >= 0)
      {
         s[6] = *s;
         s--;
      }
      memcpy(t+1, "icons/", 6);
      t[0] += 10;
      spkt->sp_Pkt.dp_Arg5 = (LONG)DEFICON;
      global->portlock = global->configlock;
      lock = spkt->sp_Pkt.dp_Arg1 = (LONG)global->portlock;
      port = ((struct FileLock *)BADDR(lock))->fl_Task;
   }
   if (lock == NULL || lock == ROOTLOCK)
   {
      BUG(("For Null/Root, using lock of %08lx\n", global->portlock));
      spkt->sp_Pkt.dp_Arg1 = global->portlock;
   }
   spkt->sp_Pkt.dp_Arg2 = (LONG)MKBADDR(spkt->sp_RP->DataP);

   if (port != (struct MsgPort *)-1)
   {
      spkt->sp_Pkt.dp_Arg4 = (long)port;
      Dispatch(global, spkt, ACTION_RETLOCK, port);
   }
   else
   {
      /* We have the NULL name - they want a lock on the true root */
      BUG(("Returning ROOTLOCK\n"));
      spkt->sp_RP->Arg1 = ROOTLOCK;
      if (spkt->sp_RP->Arg4)
      {
         spkt->sp_RP->DLen = spkt->sp_RP->DataP[0]+1;
      }
      return;
   }
}

ACTFN(RmtRetLock)
{
   spkt->sp_RP->Arg1 = spkt->sp_Pkt.dp_Res1;
   spkt->sp_RP->Arg2 = spkt->sp_Pkt.dp_Res2;

   if (spkt->sp_Pkt.dp_Res1 == 0 && spkt->sp_Pkt.dp_Arg5 != 0)
   {
      strcpy(spkt->sp_RP->DataP, (char *)spkt->sp_Pkt.dp_Arg5);
      spkt->sp_Pkt.dp_Arg5 = 0;
      Dispatch(global, spkt, ACTION_RETLOCK, (struct MsgPort *)spkt->sp_Pkt.dp_Arg4);
   }
   
   /* Now get the full pathname of this lock so we can reestablish it */
   else if(spkt->sp_RP->Arg1 && spkt->sp_RP->Arg4)
   {
      BPTR lock;
      lock = spkt->sp_RP->Arg1;
      spkt->sp_RP->DLen = 1;
      chainlock(global, spkt, lock);
      NameNode(global, spkt, lock);
   }
   else
   {
      spkt->sp_RP->DLen = 0;
   }
}

struct MsgPort *GetRDevice(global, spkt, lock, name)
GLOBAL global;
SPACKET *spkt;
BPTR lock;
char *name;
{
   int len;
   struct DEVLIST *dev;
   
   if (lock == NULL) lock = ROOTLOCK;

   BUGBSTR("GetRDevice: name is ", name);

   BUG(("GetRDevice: lock=0x%08lx\n", lock));

   if ((name[0] == 0) && (lock == ROOTLOCK))
   {
      BUG(("Returning -1\n"));
      return((struct MsgPort *)-1);
   }

   if ((lock != ROOTLOCK) &&
        ((name[0] == 0) || (name[1] != ':')))
   {
      BUG(("GetRDevice: Returning task 0x%08lx\n", 
         ((struct FileLock *)BADDR(lock))->fl_Task));
      return(((struct FileLock *)BADDR(lock))->fl_Task);
   }
   
   if (name[0] && name[1] == '/' && lock == ROOTLOCK)
   {
      /* Now what do they expect us to do here... */
      /* We are already at the root and they are saying to go up */
      /* a level.  My recommendation is to fail it with a not found */
      spkt->sp_RP->Arg1 = DOS_FALSE;
      spkt->sp_RP->Arg2 = ERROR_OBJECT_NOT_FOUND;
      return(NULL);
   }

      
   if (name[0] && name[1] == ':')
   {
      memcpy(name+1, name+2, name[0]-1);
      name[0]--;
      if (name[0] == 0)
      {
         BUG(("Returning -1\n"));
         return((struct MsgPort *)-1);
      }
   }
   
   /* Here we are being a bad boy by writing one byte past the end of           */
   /* the name some times.  We are making this a safe operation by always       */
   /* ensuring an extra pad byte in the buffer.                                 */
   for(len = 1; len <= name[0]; len++)
   {
      if (name[len] == '/')
         break;
   }

   /* Make sure we actually found it */
   if (len > name[0])
   {
      /* Sorry, make room for it */
      name[0]++;
   }
   name[name[0]+1] = 0;
   /* Walk through our validated access list to make sure that they can */
   /* actually talk to this device                                      */
   name[len] = 0;
   for(dev = spkt->sp_Ses->devices; dev != NULL; dev = dev->next)
   {
      if (!stricmp(dev->name, name+1)) break;
   }
   if (dev == NULL)
   {
      spkt->sp_RP->Arg1 = DOS_FALSE;
      spkt->sp_RP->Arg2 = ERROR_OBJECT_NOT_FOUND;
      return(NULL);
   }
   
   name[len] = ':';

   if (dev->type == ST_LINKDIR || dev->type == ST_LINKFILE)
   {
      global->portlock = dev->lock;
      return(((struct FileLock *)BADDR(dev->lock))->fl_Task);
   }
   
   global->dosport = DeviceProc(name+1);
   global->portlock = IoErr();
   if (global->dosport == NULL)
   {
      /* Now what do they expect us to do here... */
      /* We are already at the root and they are saying to go up */
      /* a level.  My recommendation is to fail it with a not found */
      spkt->sp_RP->Arg1 = DOS_FALSE;
      spkt->sp_RP->Arg2 = ERROR_OBJECT_NOT_FOUND;
      return(NULL);
   }
   return(global->dosport);
}

/*======================================================================*/
/* These routines perform the state machine for naming a node.          */
/*======================================================================*/
/* arg5 = 0                */
/* examine arg1            */
/* x = parent(Arg1)        */
/* if (arg5) unlock(Arg1)  */
/* if (x == NULL) return;  */
/* arg5 = arg1 = x;        */
/***************************/
void NameNode(global, spkt, lock)
GLOBAL global;
SPACKET *spkt;
BPTR lock;
{
   spkt->sp_Pkt.dp_Arg3 = (LONG)AllocMem(sizeof(struct FileInfoBlock), MEMF_PUBLIC);
   spkt->sp_RP->DLen = 0;

   if (spkt->sp_Pkt.dp_Arg3 == NULL)
   {
      return;
   }

   spkt->sp_Pkt.dp_Type = ACTION_EXAMINE_OBJECT;
   spkt->sp_Pkt.dp_Arg1 = lock;
   spkt->sp_Pkt.dp_Arg2 = MKBADDR(spkt->sp_Pkt.dp_Arg3);
   spkt->sp_Pkt.dp_Arg4 = (long)(((struct FileLock *)BADDR(lock))->fl_Task);
   spkt->sp_Pkt.dp_Arg5 = 0;

   Dispatch(global, spkt, ACTION_RETNAMELOCK, (struct MsgPort *)spkt->sp_Pkt.dp_Arg4);
}

ACTFN(RmtNameLock)
{
   struct FileInfoBlock *fib;
   int len;
   
   fib = (struct FileInfoBlock *)spkt->sp_Pkt.dp_Arg3;

   len = fib->fib_FileName[0]+1;

   if(spkt->sp_RP->DLen + len > global->n.NetBufSize)
   {
      spkt->sp_RP->DLen = 0;
      
      /* Force our way through as if a parent call failed in order to drive the  */
      /* normal cleanup mechanism                                                */
      spkt->sp_Pkt.dp_Res1 = 0;
      RmtNameUnlock(global, spkt);
      return;
   }

   memcpy(spkt->sp_RP->DataP+spkt->sp_RP->DLen, fib->fib_FileName, len);

   spkt->sp_RP->DLen += len;

   spkt->sp_Pkt.dp_Type = ACTION_PARENT;

   Dispatch(global, spkt, ACTION_RETNAMEEX, (struct MsgPort *)spkt->sp_Pkt.dp_Arg4);
}

ACTFN(RmtNameUnlock)
{
   spkt->sp_Pkt.dp_Arg6 = spkt->sp_Pkt.dp_Res1;
   
   if (spkt->sp_Pkt.dp_Arg5)
   {
      spkt->sp_Pkt.dp_Type = ACTION_FREE_LOCK;
      Dispatch(global, spkt, ACTION_RETNAMEUNLOCK,
                       (struct MsgPort *)spkt->sp_Pkt.dp_Arg4);
   }
   else
   {
      RmtNameExamine(global, spkt);
   }
}

ACTFN(RmtNameExamine)
{
   if (spkt->sp_Pkt.dp_Arg6 == 0)
   {
      /* No more parents, free our FIB and get out of here */
      FreeMem((APTR)spkt->sp_Pkt.dp_Arg3, sizeof(struct FileInfoBlock));
      return;
   }

   spkt->sp_Pkt.dp_Arg1 = spkt->sp_Pkt.dp_Arg5 = spkt->sp_Pkt.dp_Arg6;
   spkt->sp_Pkt.dp_Type = ACTION_EXAMINE_OBJECT;
   Dispatch(global, spkt, ACTION_RETNAMELOCK,
                    (struct MsgPort *)spkt->sp_Pkt.dp_Arg4);
}


ACTFN(RmtDupLock)
{
   BUG(("RmtDupLock\n"));

   if (spkt->sp_RP->Arg1 == ROOTLOCK)
   {
      if (spkt->sp_RP->Arg4)
      {
         spkt->sp_RP->DLen = 2;
         spkt->sp_RP->DataP[0] = 1;
         spkt->sp_RP->DataP[1] = ':';
      }
      return;
   }
   Dispatch(global, spkt, ACTION_RETLOCKNAME, 
            ((struct FileLock *)BADDR(spkt->sp_Pkt.dp_Arg1))->fl_Task);
}

ACTFN(RmtUnLock)
{
    BUG(("RmtUnLock\n"));
    spkt->sp_Pkt.dp_Arg1 = spkt->sp_RP->Arg1;

    spkt->sp_RP->DLen = 0;

    if (spkt->sp_RP->Arg1 != ROOTLOCK)
    {
       unchainlock(global, spkt, spkt->sp_Pkt.dp_Arg1);
       Dispatch(global, spkt, ACTION_RETURN,
                        ((struct FileLock *)BADDR(spkt->sp_Pkt.dp_Arg1))->fl_Task);
    }
}

void chainlock(global, spkt, lock)
GLOBAL global;
SPACKET *spkt;
BPTR lock;
{
   struct LCHAIN *lch;
   if (lock == NULL ||
       (lch = (struct LCHAIN *)AllocMem(sizeof(struct LCHAIN), 0)) == NULL)
   {
      return;
   }
   lch->data = (void *)lock;
   lch->next = spkt->sp_Ses->locks;
   spkt->sp_Ses->locks = lch;
}

void unchainlock(global, spkt, lock)
GLOBAL global;
SPACKET *spkt;
BPTR lock;
{
   struct LCHAIN *last, *lch;
   
   for(lch = spkt->sp_Ses->locks, last = NULL; lch != NULL; last=lch, lch=lch->next)
      if (lch->data == (void *)lock)
      {
         if (last == NULL)
            spkt->sp_Ses->locks = lch->next;
         else
            last->next = lch->next;
         FreeMem(lch, sizeof(struct LCHAIN));
         return;
      }
   
}
