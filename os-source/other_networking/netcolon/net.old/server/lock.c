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
**      $Revision: 1.17 $
**      $Date: 91/03/11 11:15:05 $
**      $Log:	lock.c,v $
 * Revision 1.17  91/03/11  11:15:05  J_Toebes
 * Correct tracking of access privledges
 * 
 * Revision 1.16  91/01/21  23:49:11  J_Toebes
 * Correct overzealous advancing of name position.
 * 
 * Revision 1.15  91/01/15  02:20:55  J_Toebes
 * Fixed access to icons when DEVS: is excluded.
 * 
 * Revision 1.14  91/01/15  01:06:13  J_Toebes
 * Implement pseudo locks on handlers.
 * 
 * Revision 1.13  91/01/11  10:46:50  J_Toebes
 * Cleanup support for uplevel reference limits.  At the same time corrected
 * problems where cd net:john/assigned  would not return the asigned name
 * of the object
 * 
 * Revision 1.12  90/12/31  15:23:45  J_Toebes
 * Implement security restrictions for read/write access and prevention of
 * moving up too many levels.
 * Added code to redirect NODE.info requests to the devs: directory.
 * 
 * Revision 1.11  90/12/29  13:08:57  J_Toebes
 * Corrected logic error in namelock code.
 * 
 * Revision 1.10  90/11/29  01:59:34  J_Toebes
 * Added tracking of locks.
 * 
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
   ELOCK elock;
   ACINFO info;

   BUG(("RmtLock: lock %lx\n", spkt->sp_RP->Arg1));
   BUGBSTR("Locking filename = ", spkt->sp_RP->DataP);

   spkt->sp_Pkt.dp_Arg5 = 0;
   elock = (ELOCK)spkt->sp_Pkt.dp_Arg1;

   t = spkt->sp_RP->DataP;

   if(spkt->sp_RP->DLen == 0)
   {
      if(spkt->sp_RP->Arg2 != 0)
      {
         if (global->configlock == NULL)
         {
            spkt->sp_RP->Arg1 = DOSFALSE;
            spkt->sp_RP->Arg2 = ERROR_OBJECT_NOT_FOUND;
            return;
         }
         /* Special case - lock on .info file for node */
         strcpy(spkt->sp_RP->DataP, NODEINFONAME);
         spkt->sp_Pkt.dp_Arg1 = global->configlock;
         spkt->sp_Pkt.dp_Arg7 = 0;
         port = ((struct FileLock *)BADDR(global->configlock))->fl_Task;
         goto sendit;
      }
      else
        spkt->sp_RP->DataP[0] = 0;  /* Null name */
   }

   if ((port = GetRDevice(global, spkt, elock, t, ATTEMPT_ACCESS)) == NULL)
   {
      /* We didn't find it in the list.  See if we are dealing with the */
      /* Root lock by chance and they are asking for a .info file       */
      if (elock != NULL && elock != (ELOCK)ROOTLOCK)
         return;

      /* Make sure the name really ends with .info                      */
      len = t[0];
      if (len < 5 || memcmp(t+len-5, ".info", 5))
         return;

      /* Ok, let's just truncate the name and see if we can find it on the */
      /* List without the .info:                                           */
      t[0] -= 6;
      if ((port = GetRDevice(global, spkt, elock, t, ATTEMPT_ACCESS)) == NULL)
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
      spkt->sp_Pkt.dp_Arg1 = (LONG)global->portlock;
      port = ((struct FileLock *)BADDR(global->portlock))->fl_Task;
   }
   info.all = spkt->sp_Pkt.dp_Arg7;
   if (info.a.class & CLASS_PSEUDO)
   {
      /* This is a lock on a device that doesn't really support it */
      chainlock(global, spkt, HANDLOCK);
      spkt->sp_RP->DLen = spkt->sp_RP->DataP[0]+1;
      return;
   }

sendit:
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
      BUG(("RetLock: setting to reestablish\n"));
      lock = spkt->sp_RP->Arg1;
      spkt->sp_RP->DLen = 1;
      chainlock(global, spkt, lock);
      NameNode(global, spkt, lock);
   }
   else
   {
      BUG(("RetLock: Ignoring reestablish\n"));
      chainlock(global, spkt, spkt->sp_RP->Arg1);
      spkt->sp_RP->DLen = 0;
   }
}

/*********************************************************************************/
/*                                                                               */
/* This routine determines the process address, relative lock and security       */
/* information for any given file or device.  Upon completion the following      */
/* things will be determined:                                                    */
/*    spkt->sp_Pkt.dp_Arg1  -  Will hold the lock that the name is relative to   */
/*    spkt->sp_Pkt.dp_Arg7  -  Holds the depth and security flags for later use  */
/*                             by the chainlock routine.                         */
/*    name[]                -  Will contain the adjusted name                    */
/*    MsgPort (return)      -  Pointer to message port to talk to for device     */
/*                             There are two special return codes in addition    */
/*                             to the normal pointers.                           */
/*                               -1 indicates a special one that is handled      */
/*                                  internally by the server as it is a lock on  */
/*                                  pseudo-root maintained by the server.        */
/*                               0  Indicates a failure condition.  In this case */
/*                                  it will also set the fields:                 */
/*                                    spkt->sp_RP->Arg1  - DOSFALSE              */
/*                                    spkt->sp_RP->Arg2  - ERROR code for case   */
/*                                                                               */
/*********************************************************************************/
struct MsgPort *GetRDevice(global, spkt, elock, name, mode)
GLOBAL global;
SPACKET *spkt;
ELOCK elock;
char *name;
int mode;
{
   ACINFO info;
   int flag;
   int len, ipos;
   BPTR lock;
   struct MsgPort *port;
   struct DEVLIST *dev;
   char buf[100];
    
   if (elock == NULL) elock = (ELOCK)ROOTLOCK;
   else if (!testlock(global, spkt, elock)) return(NULL);

   if (name[0] && name[1] == ':')
   {
      name[0]--;
      memcpy(name+1, name+2, name[0]);
      if (elock != (ELOCK)ROOTLOCK)
      {
         if (elock->lock == NULL)
         {
nullelock:
            for(len = 1; len <= name[0]; len++)
               if (name[len] == '/')
                  break;

            /* Make sure we actually found it */
            if (len > name[0])
            {
               /* oops, we just want a colon for the name */
               name[0] = 1;
               name[2] = 0;
            }
            else
            {
               name[0] -= len-1;
               memcpy(name+1, name+len, name[0]);
            }
            name[1] = ':';
   
            /* Figure out which device it was we wanted to deal with */
            for(ipos = elock->info.a.devpos, dev = spkt->sp_Ses->devices;
                ipos > 0 && dev != NULL; dev = dev->next, ipos--);
            if (dev == NULL)
            {
               spkt->sp_RP->Arg1 = DOS_FALSE;
               spkt->sp_RP->Arg2 = ERROR_OBJECT_NOT_FOUND;
               return(NULL);
            }
            strcpy(buf, dev->name);
            buf[dev->len] = ':';
            buf[dev->len+1] = 0;
            port = global->dosport = DeviceProc(buf);
            spkt->sp_Pkt.dp_Arg7 = elock->info.all;
            return(port);
         }
      }
      elock = (ELOCK)ROOTLOCK;
   }

   /* Determine our base access level and security requirements */
   if (elock == (ELOCK)ROOTLOCK)
   {
      info.a.class = CLASS_READONLY;
      lock = ROOTLOCK;
   }
   else
   {
      /* Now test for one of those special locks that we had made before */
      if (elock->lock == NULL) goto nullelock;

      info.all  = elock->info.all;
      lock  = elock->lock;
      port  = ((struct FileLock *)BADDR(elock->lock))->fl_Task;
   }

   /* Now we have a valid name as well as an elock and a depth and security */
   /* class.  If the name is relative to the root, we have to look it up in */
   /* the table of entries                                                  */
   ipos = -1;
   flag = 1;

   for (len = 1; len <= name[0]; len++)
   {
      if (lock == ROOTLOCK)
      {
         /* Go through and find the first slash of the name                    */
         /* Here we are being a bad boy by writing one byte past the end of    */
         /* the name some times.  We are making this a safe operation by always*/
         /* ensuring an extra pad byte in the buffer.                          */
         while(len <= name[0])
         {
            if (name[len] == '/')
               break;
            len++;
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
         info.a.devpos = 0;
         for(dev = spkt->sp_Ses->devices; dev != NULL; dev = dev->next)
         {
            if (!stricmp(dev->name, name+1)) break;
            info.a.devpos++;
         }
         if (dev == NULL)
         {
            spkt->sp_RP->Arg1 = DOS_FALSE;
            spkt->sp_RP->Arg2 = ERROR_OBJECT_NOT_FOUND;
            return(NULL);
         }
         info.a.class = (dev->protect & FIBF_WRITE) ? CLASS_READONLY : CLASS_SHARED;
         name[len] = ':';
         info.a.depth = 0;
         flag = 1;

         if (dev->type == ST_LINKDIR || dev->type == ST_LINKFILE)
         {
            lock = dev->lock;
            port  = ((struct FileLock *)BADDR(lock))->fl_Task;
         }
         else
         {
            if (global->dosport != NULL)
            {
               /* Eliminate the DeviceProc to reduce the count on the device */
               global->dosport = NULL;
            }
            port = global->dosport = DeviceProc(name+1);
            lock = IoErr();
            if (dev->type == ST_FILE)
            {
               if (info.a.class == CLASS_READONLY && mode != ATTEMPT_ACCESS)
               {
                  spkt->sp_RP->Arg1 = DOS_FALSE;
                  spkt->sp_RP->Arg2 = ERROR_WRITE_PROTECTED;
                  return(NULL);
               }
               info.a.class |= CLASS_PSEUDO;
               spkt->sp_Pkt.dp_Arg7 = info.all;
               spkt->sp_Pkt.dp_Arg1 = lock;
               return(port);
            }
         }
         ipos = -1;
         continue;
      }

      /* Now name[len] is the next character to be checked                  */
      /* We want to go through the rest of the name and validate that they  */
      /* are not trying to move up too many levels or sneak a colon in      */

      if (name[len] == ':') /* uh, oh they shouldn't be doing this...    */
      {
         spkt->sp_RP->Arg1 = DOS_FALSE;
         spkt->sp_RP->Arg2 = ERROR_INVALID_COMPONENT_NAME;
         return(NULL);
      }
      /* When we hit a directory separator character we use the flag state       */
      /* Variable to track this event.  If we hit a slash and the previous unit  */
      /* was not a separator character, this is an indication of an uplevel      */
      /* reference.                                                              */

      if (name[len] == '/') /* Directory separator.  Is the next one uplevel?*/
      {
         if (flag)
         {
            info.a.depth--;
            if (info.a.depth < 0)
            {
               /* They attempted to pop back up to the root */
               /* Reshuffle the name and retry it.          */
               name[0] -= len;
               memcpy(name+1, name+len+1, name[0]);
               len = 0;
               ipos = -1;
               lock = ROOTLOCK;
               continue;
            }
         }
         else
         {
            flag = 1;
         }
      }
      else
      {
         if (flag) info.a.depth++;
         if (ipos == -1) ipos = len;
         flag = 0;
      }
   }

   /* Finally validate access to the object                                      */
   if (info.a.class == CLASS_READONLY && mode != ATTEMPT_ACCESS)
   {
      spkt->sp_RP->Arg1 = DOS_FALSE;
      spkt->sp_RP->Arg2 = ERROR_WRITE_PROTECTED;
      return(NULL);
   }

   /* See if we can optimize access by making .info file disappear.             */
   if ((ipos != -1) && ((len - ipos) == 6) && !stricmp(name+ipos, ".info"))
   {
      /* well, it is a .info file.  Just skip over it                           */
      if (mode == ATTEMPT_ACCESS)
         spkt->sp_RP->Arg2 = ERROR_OBJECT_NOT_FOUND;
      else
         spkt->sp_RP->Arg2 = ERROR_WRITE_PROTECTED;
      spkt->sp_RP->Arg1 = DOSFALSE;
      return(NULL);
   }

   /* Remember the depth and security access for subsequent attempts             */
   spkt->sp_Pkt.dp_Arg7 = info.all;
   spkt->sp_Pkt.dp_Arg1 = lock;

   /* Check to see if we have to return the special case root lock               */
   if (lock == ROOTLOCK)
   {
      return((struct MsgPort *)-1);
   }

   /* See if anywhere along the line we encountered a non-existant port.  It     */
   /* is unlikely to occur but should be checked for just in case                */
   if (port == NULL)
   {
      spkt->sp_RP->Arg1 = DOS_FALSE;
      spkt->sp_RP->Arg2 = ERROR_OBJECT_NOT_FOUND;
      return(NULL);
   }

   return(port);
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
      BUG(("Abandoning NameLock for lack of memory\n"));
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
      BUG(("Abandoning NameLock for size reason\n"));
      /* Force our way through as if a parent call failed in order to drive the  */
      /* normal cleanup mechanism                                                */
      spkt->sp_Pkt.dp_Res1 = 0;
      RmtNameUnlock(global, spkt);
      return;
   }

   memcpy(spkt->sp_RP->DataP+spkt->sp_RP->DLen, fib->fib_FileName+1, len-1);

   spkt->sp_RP->DLen += len;
   spkt->sp_RP->DataP[spkt->sp_RP->DLen-1] = len;

   spkt->sp_Pkt.dp_Type = ACTION_PARENT;

   Dispatch(global, spkt, ACTION_RETNAMEUNLOCK, (struct MsgPort *)spkt->sp_Pkt.dp_Arg4);
}

ACTFN(RmtNameUnlock)
{
   spkt->sp_Pkt.dp_Arg6 = spkt->sp_Pkt.dp_Res1;
   
   if (spkt->sp_Pkt.dp_Arg5)
   {
      spkt->sp_Pkt.dp_Type = ACTION_FREE_LOCK;
      Dispatch(global, spkt, ACTION_RETNAMEEX,
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
      BUG(("Abandoning NameLock - no more parents dlen=%08lx\n", spkt->sp_RP->DLen));
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
   ELOCK elock;
   BUG(("RmtDupLock\n"));

   elock = (ELOCK)spkt->sp_Pkt.dp_Arg1;

   switch(testlock(global, spkt, elock))
   {
      case LOCK_ROOT:
         spkt->sp_RP->DLen = 2;
         spkt->sp_RP->DataP[0] = 1;
         spkt->sp_RP->DataP[1] = ':';
         break;

      case LOCK_PSEUDO:
         spkt->sp_Pkt.dp_Arg7 = elock->info.all;
         spkt->sp_RP->DLen = 0;
         spkt->sp_RP->Arg1 = DOSTRUE;
         chainlock(global, spkt, HANDLOCK);
         break;

      case LOCK_NORMAL:
         spkt->sp_Pkt.dp_Arg7 = elock->info.all;
         spkt->sp_Pkt.dp_Arg1 = elock->lock;
         Dispatch(global, spkt, ACTION_RETLOCKNAME, 
                   ((struct FileLock *)BADDR(spkt->sp_Pkt.dp_Arg1))->fl_Task);
         break;
   }
}

ACTFN(RmtUnLock)
{
    ELOCK elock;
    int type;

    BUG(("RmtUnLock\n"));
    elock = (ELOCK) spkt->sp_RP->Arg1;

    spkt->sp_RP->DLen = 0;
    spkt->sp_RP->Arg1 = DOSTRUE;

    type = testlock(global, spkt, elock);
    if (type == LOCK_NORMAL || type == LOCK_PSEUDO)
    {
       spkt->sp_Pkt.dp_Arg1 = elock->lock;
       unchainlock(global, spkt, elock);
       if (type == LOCK_NORMAL)
       {
          Dispatch(global, spkt, ACTION_RETURN,
                           ((struct FileLock *)BADDR(spkt->sp_Pkt.dp_Arg1))->fl_Task);
       }
    }
}

void chainlock(global, spkt, lock)
GLOBAL global;
SPACKET *spkt;
BPTR lock;
{
   ELOCK lch;
   if (lock == NULL ||
       (lch = (ELOCK)AllocMem(sizeof(struct lchain), 0)) == NULL)
   {
      return;
   }
   if (lock == HANDLOCK)
      lch->lock = NULL;
   else
      lch->lock = lock;
   lch->next = spkt->sp_Ses->locks;
   lch->info.all = spkt->sp_Pkt.dp_Arg7;
   spkt->sp_Ses->locks = lch;
   spkt->sp_RP->Arg1 = (long)lch;
}

void unchainlock(global, spkt, elock)
GLOBAL global;
SPACKET *spkt;
ELOCK elock;
{
   ELOCK last, lch;
   
   for(lch = spkt->sp_Ses->locks, last = NULL; lch != NULL; last=lch, lch=lch->next)
      if (lch == elock)
      {
         if (last == NULL)
            spkt->sp_Ses->locks = lch->next;
         else
            last->next = lch->next;
         FreeMem(lch, sizeof(struct lchain));
         return;
      }
   
}

int testlock(global, spkt, elock)
GLOBAL global;
SPACKET *spkt;
ELOCK elock;
{
   ELOCK eptr;
 
   if (elock == (ELOCK)ROOTLOCK) return(LOCK_ROOT);
   /* Validate the elock on the lock list */
   for(eptr = spkt->sp_Ses->locks; eptr != NULL; eptr = eptr->next)
      if (eptr == elock) break;
   if (eptr == NULL)
   {
      spkt->sp_RP->Arg1 = DOS_FALSE;
      spkt->sp_RP->Arg2 = ERROR_INVALID_LOCK;
      return(LOCK_INVALID);
   }
   if (elock->lock == NULL) return(LOCK_PSEUDO);
   return(LOCK_NORMAL);
}
