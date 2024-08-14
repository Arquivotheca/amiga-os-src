/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987, 1988 The Software Distillery.  All Rights  */
/* |. o.| || Reserved.  This program may not be distributed without the    */
/* | .  | || permission of the authors:                            BBS:    */
/* | o  | ||   John Toebes     Doug Walker    Dave Baker                   */
/* |  . |//                                                                */
/* ======                                                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* File Access:         */
/* RmtRead RmtWrite RmtSeek RmtWaitForChar    */
/* RmtFindwrite RmtFindin RmtFindout RmtEnd   */
/*
**      $Filename: io.c $ 
**      $Author: J_Toebes $
**      $Revision: 1.7 $
**      $Date: 90/11/28 01:52:19 $
**      $Log:	io.c,v $
 * Revision 1.7  90/11/28  01:52:19  J_Toebes
 * Added support for icons, added session tracking back in.
 * 
 * Revision 1.5  90/11/15  08:17:42  J_Toebes
 * Correct multiple bugs associated with new version.  Sessions still are not supported.
 * 
 * Revision 1.4  90/11/05  06:56:19  J_Toebes
 * Major rewrite to make server multi-threaded with support for multiple
 * devices.  At this point in time it is not debugged and the concept of
 * sessions is missing.
 * 
*/

#include "server.h"


/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                 RmtRW( global, pkt )                                    */
/*                                                                         */
/*------------------------------------------------------------------------*/

ACTFN(RmtRW)
{
   /* Arg1: APTR EFileHandle */
   /* Arg2: APTR Buffer      */
   /* Arg3: Length           */
   struct FCHAIN *fh;

   BUG(("RmtRW of %d bytes from fh 0x%08lx\n", spkt->sp_RP->Arg3, spkt->sp_RP->Arg1));

   fh = (struct FCHAIN *)spkt->sp_RP->Arg1;
   spkt->sp_Pkt.dp_Arg1 = fh->fh.fh_Arg1;
   spkt->sp_Pkt.dp_Arg2 = (LONG)spkt->sp_RP->DataP;

   Dispatch(global, spkt, ACTION_RETRW, fh->fh.fh_Type);
}

ACTFN(RmtRWRet)
{
   spkt->sp_RP->DLen =
   spkt->sp_RP->Arg1 = spkt->sp_Pkt.dp_Res1;
   spkt->sp_RP->Arg2 = spkt->sp_Pkt.dp_Res2;
}

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                 RmtReadMore( global, pkt )                              */
/*                                                                         */
/*-------------------------------------------------------------------------*/

ACTFN(RmtRWMore)
{
   /* Arg1: APTR EFileHandle    */
   /* Arg2: APTR Buffer         */
   /* Arg3: Length              */
   /* Arg4: Seek Pos Indicator  */
   struct FCHAIN *fh;

   BUG(("RmtRWMore of %d bytes\n", spkt->sp_RP->Arg3));

   fh = (struct FCHAIN *)spkt->sp_RP->Arg1;
   
   spkt->sp_Pkt.dp_Arg1 = fh->fh.fh_Arg1;

   /* IF they asked us to save the current seek position, ask the file system   */
   /* where we are in the file and return it as Arg4.  Remember that we         */
   /* will use this as the seek back position in case of error.                 */
   if (spkt->sp_Pkt.dp_Arg4 == -1)
   {
      spkt->sp_Pkt.dp_Type = ACTION_SEEK;
      spkt->sp_Pkt.dp_Arg2 = 0L;
      spkt->sp_Pkt.dp_Arg3 = OFFSET_CURRENT;
      Dispatch(global, spkt, ACTION_RWMORE1, fh->fh.fh_Type);
   }
   else
   {
      spkt->sp_Pkt.dp_Res1 = spkt->sp_RP->Arg4; /* Fall through main case */
      RmtRWMore1(global, spkt);
   }
}

ACTFN(RmtRWMore1)
{
   struct FCHAIN *fh;

   fh = (struct FCHAIN *)spkt->sp_RP->Arg1;
 
   /* Was there any sort of error during the seek operation? */
   if (spkt->sp_Pkt.dp_Res1 < 0)
   {
      BUG(("RmtRead: Seek failed, code %d\n", spkt->sp_Pkt.dp_Res2));
      spkt->sp_RP->Arg1 = spkt->sp_Pkt.dp_Res1;
      spkt->sp_RP->Arg2 = spkt->sp_Pkt.dp_Res2;
      spkt->sp_RP->DLen = 0;
      return;
   }

   /* Remember the original Seek position to return to them     */
   spkt->sp_Pkt.dp_Arg4 = spkt->sp_Pkt.dp_Res1;

   /* Do the I/O operation to get the data moved                */
   spkt->sp_Pkt.dp_Type = spkt->sp_RP->Type-1;
   spkt->sp_Pkt.dp_Arg1 = fh->fh.fh_Arg1;
   spkt->sp_Pkt.dp_Arg2 = (LONG)spkt->sp_RP->DataP;
   spkt->sp_Pkt.dp_Arg3 = spkt->sp_RP->Arg3;

   BUG(("RmtRWMore: Amount to r/w is %d\n", spkt->sp_Pkt.dp_Arg3));
   Dispatch(global, spkt, ACTION_RWMORE2, fh->fh.fh_Type);
}

ACTFN(RmtRWMore2)
{
   struct FCHAIN *fh;

   fh = (struct FCHAIN *)spkt->sp_RP->Arg1;
 
   /* If we encountered an error, we need to seek back to the original  */
   /* file position.                                                    */
   spkt->sp_RP->DLen = spkt->sp_RP->Arg1 = spkt->sp_Pkt.dp_Res1;
   spkt->sp_RP->Arg2 = spkt->sp_Pkt.dp_Res2;

   if ((spkt->sp_RP->DLen < 0) &&
       (spkt->sp_RP->Arg4 != -1)) /* We seeked to begin with */
   {
      spkt->sp_RP->DLen = 0;
      /* Seek back to the original pos */
      spkt->sp_Pkt.dp_Type = ACTION_SEEK;
      spkt->sp_Pkt.dp_Arg2 = spkt->sp_Pkt.dp_Arg4;
      spkt->sp_Pkt.dp_Arg3 = OFFSET_BEGINNING;
      Dispatch(global, spkt, ACTION_RWMORE3, fh->fh.fh_Type);
   }
   else
   {
      spkt->sp_RP->Arg4 = spkt->sp_Pkt.dp_Arg4;
   }
}

ACTFN(RmtRWMore3) {}

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                       RmtSeek( global, pkt )                            */
/*                                                                         */
/*-------------------------------------------------------------------------*/

ACTFN(RmtSeek)
{
   /* Arg1: APTR EFileHandle */
   /* Arg2: Position */
   /* Arg3: Mode */
   struct FCHAIN *fh;

   fh = (struct FCHAIN *)spkt->sp_RP->Arg1;

   BUG(("RmtSeek\n"));

   spkt->sp_Pkt.dp_Arg1 = fh->fh.fh_Arg1;
   spkt->sp_RP->DLen = 0;

   Dispatch(global, spkt, ACTION_RETURN, fh->fh.fh_Type);
}

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                    RmtFindwrite( global, pkt )                          */
/*                                                                         */
/*-------------------------------------------------------------------------*/

ACTFN(RmtOpen)
{
   /* Arg1: FileHandle to fill in */
   /* Arg2: Lock for file relative to */
   /* Arg3: Name of file */

   struct FCHAIN *fh;
   struct MsgPort *port;
   char *s, *t;
   int len;
   long lock;

   BUG(("RmtFindwrite, lock %lx\n", spkt->sp_RP->Arg2));
   BUGBSTR("Filename = ", spkt->sp_RP->DataP);

   spkt->sp_Pkt.dp_Arg5 = 0;
   lock = spkt->sp_Pkt.dp_Arg2;

   if ((port = GetRDevice(global, spkt, lock,
                                  spkt->sp_RP->DataP)) == NULL)
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
      spkt->sp_Pkt.dp_Arg2 = lock;
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
      spkt->sp_Pkt.dp_Arg2 = (LONG)global->configlock;
      port = ((struct FileLock *)BADDR(global->configlock))->fl_Task;
   }

   fh = (struct FCHAIN *)DosAllocMem(global, sizeof(struct FCHAIN));
   if (fh == NULL)
   {
      spkt->sp_RP->Arg1 = DOS_FALSE;
      spkt->sp_RP->Arg2 = ERROR_NO_FREE_STORE;
      return;
   }
   BUG(("Allocated FileHandle = %lx\n", fh));

   spkt->sp_Pkt.dp_Arg1 = (LONG)MKBADDR(fh);
   if (spkt->sp_Pkt.dp_Arg2 == NULL || spkt->sp_Pkt.dp_Arg2 == ROOTLOCK)
   {
      BUG(("For Null/Root, using lock of %08lx\n", global->portlock));
      spkt->sp_Pkt.dp_Arg2 = global->portlock;
   }
   spkt->sp_Pkt.dp_Arg3 = (LONG)MKBADDR(spkt->sp_RP->DataP);
   spkt->sp_Pkt.dp_Arg4 = (LONG)port;

   Dispatch(global, spkt, ACTION_RETOPEN, port);
}

ACTFN(RmtOpenRet)
{
   struct FCHAIN *fh;
   struct MsgPort *port;

   fh = (struct FCHAIN *)BADDR(spkt->sp_Pkt.dp_Arg1);
   port = (struct MsgPort *)spkt->sp_Pkt.dp_Arg4;

   spkt->sp_RP->Arg1 = spkt->sp_Pkt.dp_Res1;
   spkt->sp_RP->Arg2 = spkt->sp_Pkt.dp_Res2;

   /* If the open was successful, return the allocated filehandle as Arg3 */
   if (spkt->sp_Pkt.dp_Res1 == DOS_FALSE)
   {
      if (spkt->sp_Pkt.dp_Arg5 == 0)
      {
         DosFreeMem((char *)fh);
      }
      else
      {
         strcpy(spkt->sp_RP->DataP, (char *)spkt->sp_Pkt.dp_Arg5);
         spkt->sp_Pkt.dp_Arg5 = 0;
         Dispatch(global, spkt, ACTION_RETOPEN, port);
      }
   }
   else
   {
      BUG(("RmtFindWrite: new fh is 0x%08lx, port 0x%08lx\n", fh, port));
      spkt->sp_RP->Arg3 = (LONG)fh;
      /* Add the file handle to the tracked file handles for this session */
      fh->next = spkt->sp_Ses->fhs;
      spkt->sp_Ses->fhs = fh;
      fh->fh.fh_Type = port;
   }
   spkt->sp_RP->DLen = 0;
}

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                       RmtEnd( global, pkt )                             */
/*                                                                         */
/*-------------------------------------------------------------------------*/

ACTFN(RmtEnd)
{
   struct FileHandle *fh;

   BUG(("RmtEnd, freeing %lx\n", spkt->sp_RP->Arg1));

   spkt->sp_Pkt.dp_Arg1 = (fh = (struct FileHandle *)spkt->sp_RP->Arg1)->fh_Arg1;

   Dispatch(global, spkt, ACTION_RETEND, fh->fh_Type);
}

ACTFN(RmtEndRet)
{
   struct FileHandle *fh;
   struct FCHAIN *fc;

   fh = (struct FileHandle *)spkt->sp_RP->Arg1;

   /* Remove the file handle from the active list */
   if (spkt->sp_Ses->fhs == (struct FCHAIN *)fh)
   {
      spkt->sp_Ses->fhs = spkt->sp_Ses->fhs->next;
   }
   else
   {
      for(fc = spkt->sp_Ses->fhs; fc != NULL; fc = fc->next)
      {
         if (fc->next == (struct FCHAIN *)fh)
         {
            fc->next = fc->next->next;
            break;
         }
      }
   }
   DosFreeMem((char *)fh);

   spkt->sp_RP->DLen = 0;
   spkt->sp_RP->Arg1 = spkt->sp_Pkt.dp_Res1;
   spkt->sp_RP->Arg2 = spkt->sp_Pkt.dp_Res2;
}
