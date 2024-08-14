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
**      $Revision: 1.16 $
**      $Date: 91/03/11 11:16:41 $
**      $Log:	io.c,v $
 * Revision 1.16  91/03/11  11:16:41  J_Toebes
 * Add code to limit the number of active open file handles
 * 
 * Revision 1.15  91/01/21  23:49:47  J_Toebes
 * Correct opening of pseudo-root files.
 * 
 * Revision 1.14  91/01/15  02:21:16  J_Toebes
 * Fixed access to icons when DEVS: is excluded.
 * 
 * Revision 1.13  91/01/11  10:46:44  J_Toebes
 * Cleanup support for uplevel reference limits.  At the same time corrected
 * problems where cd net:john/assigned  would not return the asigned name
 * of the object
 * 
 * Revision 1.12  90/12/31  15:24:36  J_Toebes
 * Implement security restrictions for read/write access and prevention of
 * moving up too many levels.
 * 
 * Revision 1.11  90/12/30  15:39:30  J_Toebes
 * Add passing back interactive information.
 * 
 * Revision 1.10  90/12/29  13:45:37  J_Toebes
 * Correct syntax error.
 * 
 * Revision 1.9  90/12/01  21:08:59  J_Toebes
 * Corrected problems with returns for ACTION_READ and ACTION_WRITE.  Under
 * ACTION_WRITE it would send back a copy of the original data (performance
 * hit) and under ACTION_READ it would set DLEN to -1 if there was an error
 * on a read.
 * 
 * Revision 1.8  90/11/29  01:57:58  J_Toebes
 * Eliminate DosAllocMem, DosFreeMem calls.
 * 
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

   /* Make sure that they have access to the file on the device */
   if (fh->info.a.class == CLASS_READONLY && spkt->sp_RP->Type == ACTION_WRITE)
   {
      spkt->sp_RP->Arg1 = -1;
      spkt->sp_RP->Arg2 = ERROR_WRITE_PROTECTED;
      return;
   }
   spkt->sp_Pkt.dp_Arg1 = fh->fh.fh_Arg1;
   spkt->sp_Pkt.dp_Arg2 = (LONG)spkt->sp_RP->DataP;

   Dispatch(global, spkt, ACTION_RETRW, fh->fh.fh_Type);
}

ACTFN(RmtRWRet)
{
   spkt->sp_RP->Arg1 = spkt->sp_Pkt.dp_Res1;
   spkt->sp_RP->Arg2 = spkt->sp_Pkt.dp_Res2;
   spkt->sp_RP->DLen = 0;
   if (spkt->sp_RP->Type == ACTION_READ &&
       spkt->sp_RP->Arg1 > 0)
   {
      spkt->sp_RP->DLen = spkt->sp_RP->Arg1;
   }
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
   if (fh->info.a.class == CLASS_READONLY && spkt->sp_RP->Type == ACTION_WRITE_MORE)
   {
      spkt->sp_RP->Arg1 = -1;
      spkt->sp_RP->Arg2 = ERROR_WRITE_PROTECTED;
      return;
   }
   
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
   ELOCK elock;
   int intent;

   BUG(("RmtFindwrite, lock %lx\n", spkt->sp_RP->Arg2));
   BUGBSTR("Filename = ", spkt->sp_RP->DataP);

   if (!global->opencount)
   {
      spkt->sp_Pkt.dp_Arg5 = (LONG)global->openpend;
      global->openpend = spkt;
      global->n.reply = 0;
      return;
   }
   
   global->opencount--;
   spkt->sp_Pkt.dp_Arg5 = 0;
   t = spkt->sp_RP->DataP;
   intent = spkt->sp_RP->Type == ACTION_FIND_INPUT ? ATTEMPT_ACCESS : ATTEMPT_WRITE;
   elock = (ELOCK)spkt->sp_Pkt.dp_Arg2;

   if(spkt->sp_RP->DLen == 0)
   {
      if(spkt->sp_RP->Arg3 != 0)
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

   port = GetRDevice(global, spkt, elock, t, intent);
   if (port == (struct MsgPort *)-1)
   {
      spkt->sp_RP->Arg1 = DOSFALSE;
      spkt->sp_RP->Arg2 = ERROR_OBJECT_NOT_FOUND;
      return;
   }

   if (port == NULL) 
   {
      /* We didn't find it in the list.  See if we are dealing with the */
      /* Root lock by chance and they are asking for a .info file       */
      if (elock != NULL && elock != (ELOCK)ROOTLOCK)
         return;

      /* Make sure the name really ends with .info                      */
      t = spkt->sp_RP->DataP;
      len = t[0];
      if (len < 5 || memcmp(t+len-5, ".info", 5))
         return;

      /* Ok, let's just truncate the name and see if we can find it on the */
      /* List without the .info:                                           */
      t[0] -= 6;
      if ((port = GetRDevice(global, spkt, elock, t, intent)) == NULL)
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
      spkt->sp_Pkt.dp_Arg1 = (LONG)global->configlock; /* It gets moved below */
      port = ((struct FileLock *)BADDR(global->configlock))->fl_Task;
   }

sendit:
   spkt->sp_Pkt.dp_Arg2 = spkt->sp_Pkt.dp_Arg1; 
   fh = (struct FCHAIN *)AllocMem(sizeof(struct FCHAIN), MEMF_PUBLIC|MEMF_CLEAR);
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
         FreeMem((char *)fh, sizeof(struct FCHAIN));
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
      spkt->sp_RP->Arg4 = (LONG)fh->fh.fh_Port;
      /* Add the file handle to the tracked file handles for this session */
      fh->next = spkt->sp_Ses->fhs;
      spkt->sp_Ses->fhs = fh;
      fh->fh.fh_Type = port;
      fh->info.all = spkt->sp_Pkt.dp_Arg7;
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

   global->opencount++;

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
   FreeMem((char *)fh, sizeof(struct FCHAIN));

   spkt->sp_RP->DLen = 0;
   spkt->sp_RP->Arg1 = spkt->sp_Pkt.dp_Res1;
   spkt->sp_RP->Arg2 = spkt->sp_Pkt.dp_Res2;
}
