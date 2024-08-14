/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1990 The Software Distillery.                    *
* |. o.| ||          All Rights Reserved                                  *
* | .  | ||          Written by Doug Walker                               *
* | o  | ||          The Software Distillery                              *
* |  . |//           235 Trillingham Lane                                 *
* ======             Cary, NC 27513                                       *
*                    BBS:(919)-471-6436                                   *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <exec/types.h>
#include <exec/errors.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <graphics/gfxbase.h>
#include <graphics/regions.h>
#include <graphics/gels.h>
#include <devices/serial.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

#include "netcomm.h"
#include "handler.h"
#include "netipc.h"

#ifdef CPR
char *dbgwind = "CON:0/0/640/160/NETIPC-HANDLER/a";
#endif

#if 1

int ReSync(global, ioptr)
GLOBAL global;
APTR ioptr;
{
   struct NetNode *tmpnode;
   IPCGLOBAL ig = global->n.d.ig;

   BUG(("ReSync: ioptr 0x%08lx\n", ioptr));

   InitRDevice(global);

   BUG(("ReSync: Setting all ioptrs to 0x%08lx\n", ig->IPCReq));

   for(tmpnode=global->netchain.next; tmpnode; tmpnode=tmpnode->next)
      tmpnode->ioptr = (APTR)ig->IPCReq;

   BUG(("ReSync: Exit\n"));

   return((ig->IPCReq) != NULL);
}

#else

int ReSync(global, ioptr)
GLOBAL global;
APTR ioptr;
{
   return(1);
}

#endif

int InitRDevice(global)
GLOBAL global;
{
   IPCGLOBAL ig = global->n.d.ig;

   BUG(("InitRDevice: Initializing for IPC operation on unit %d\n",
        global->unitnum));

   BUGP("InitRDevice: Entry")

   if(!global->RP.DataP) 
   {
      SetPacketBuffer(&global->RP, NETBUFSIZE);
      global->n.NetBufSize = NETBUFSIZE;
   }

   if(!ig)
   {
      if(!(ig = (void *)AllocMem(sizeof(*ig), MEMF_CLEAR)))
         return(0);
      global->n.d.ig = ig;
   }
   else
      ig->flags &= ~NIPC_BADCON;

   if(!ig->IPCReq)
   {
      if(ig->errorcode = OpenIPC(global))
      {
         BUG(("********ERROR: Can't open ipc!!\n"));
         BUGR("Can't open ipc!")
         return (0) ;
      }

      ConnectIPC(global);
   }

   BUG(("InitRDevice: Exit, IPCReq %lx\n", ig->IPCReq));

   BUGP("InitRDevice: Exit")

   return((ig->IPCReq) != NULL);
}


int ConnectIPC(global)
GLOBAL global;
{
   int done;
   struct Message *msg;
   struct sockaddr_un foo;
   IPCGLOBAL ig = global->n.d.ig;

   foo.sun_family = AF_UNIX;
   strcpy(foo.sun_path, NETNAME);

   ig->IPCReq->IO.io_Command = IPCMD_CONNECT;
   ig->IPCReq->IO.io_Data = (APTR)&foo;
   ig->IPCReq->IO.io_Length = strlen(NETNAME)+2;

	SendIO(ig->IPCReq) ;

   SetTimer(30L, 0L, ig->timermsg) ; /* time it out after 30 sec */

   done = FALSE ;
   ig->errorcode = 0 ;
   while (!done)
   {
      ig->wakeupbits = Wait (ig->timerbit | ig->IPCBit) ;
      if(GetMsg(ig->IPCReplyPort))
      {
         done = TRUE ;
         ig->errorcode = ig->IPCReq->IO.io_Error;
         BUG(("ConnectIPC: Back from SendIO, io_Error %d\n", ig->errorcode))
         AbortIO(ig->timermsg) ;
         WaitIO(ig->timermsg) ;
         GetMsg(ig->timerport) ;
         SetSignal(0L, ig->timerbit) ;
      }
      else
      {
         BUG(("ConnectIPC: Connection timed out\n"))
         msg = GetMsg(ig->timerport) ;
         if (msg != (struct Message *)0L) 
         {
            done = TRUE ;
            ig->errorcode = 1 ;
            AbortIO(ig->IPCReq) ;
            WaitIO(ig->IPCReq) ;
            GetMsg(ig->IPCReplyPort) ;
         }
      }
   }

   return(ig->errorcode);

}

VOID SetTimer(sec, micro, timermsg)
ULONG sec, micro;
struct IOStdReq *timermsg;
/* This routine simply sets the timer to interrupt us after secs.micros */
{
    timermsg->io_Command = TR_ADDREQUEST;    /* add a new timer request */
    timermsg->io_Actual = sec;    /* seconds */
    timermsg->io_Length = micro;    /* microseconds */
    SendIO(timermsg);	/* post a request to the timer */
}


int TermRDevice(global, status)
GLOBAL global;
int status;
{
   struct NetNode *netnode;
   IPCGLOBAL ig = global->n.d.ig;

   BUG(("TermRDevice: Status %d\n", status));

   for(netnode=global->netchain.next; netnode; netnode=netnode->next)
   {
      netnode->status = NODE_DEAD;
      netnode->incarnation++;
      netnode->RootLock.incarnation = netnode->incarnation;
      netnode->ioptr = NULL;
   }

   if(ig)
   {
      if(ig->IPCReq) 
      {
         if(!(ig->flags & NIPC_BADCON))
         {
            ig->IPCReq->IO.io_Command = IPCMD_DISCONNECT ;
            DoIO((struct IoRequest *)ig->IPCReq) ;
         }
         CloseDevice(ig->timermsg);
         DeleteStdIO(ig->timermsg);
         DeletePort(ig->timerport);
         CloseDevice(ig->IPCReq);
         FreeMem(ig->IPCReq, (long)sizeof(*ig->IPCReq));
         DeletePort(ig->IPCReplyPort);
         ig->IPCReq = NULL;
      }
      FreeMem((void *)ig, sizeof(*ig));
      global->n.d.ig = NULL;
   }

   BUG(("TermRDevice: Exit\n"));

   return(0);
}
