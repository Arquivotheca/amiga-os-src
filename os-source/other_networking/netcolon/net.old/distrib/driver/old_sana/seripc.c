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
#include "server.h"
#include "netipc.h"

#ifdef CPR
char *dbgwind = "CON:0/0/640/160/NETIPC-SERVER/a";
#endif

#if 1

int ReSync(global, ioptr)
GLOBAL global;
APTR ioptr;
{
   TermRDevice(global, 1);
   return(InitRDevice(global));
}

#else

int ReSync(global, ioptr)
GLOBAL global;
APTR ioptr;
{
   return(1);
}

#endif

int InitRDevice(GLOBAL global)
{
   struct sockaddr_un foo;
   IPCGLOBAL ig = global->n.d.ig;

   BUGP("InitRDevice: Entry")

   if(!ig)
   {
      if(!(ig = (void *)AllocMem(sizeof(*ig), MEMF_CLEAR)))
         return(1);
      global->n.d.ig = ig;
   }
   else
      ig->flags &= ~NIPC_BADCON;

   if(!global->RP.DataP)
   {
      SetPacketBuffer(&global->RP, NETBUFSIZE);
      global->n.NetBufSize = NETBUFSIZE;
   }

   if(!ig->IPCReq)
   {
      if(ig->errorcode = OpenIPC(global))
      {
         BUG(("********ERROR: Can't open ipc!!\n"));
         BUGR("Can't open channel!")
         return(1);
      }
      ig->IPCMask |= SIGBREAKF_CTRL_C;
   }

   foo.sun_family = AF_UNIX;
   strcpy(foo.sun_path, NETNAME);

   ig->IPCReq->IO.io_Command = IPCMD_DCLOBJECT;
   ig->IPCReq->IO.io_Data = (APTR)&foo;
   ig->IPCReq->IO.io_Length = strlen(NETNAME)+2;

   BUG(("InitRDevice: Declaring object..."))
   DoIO((struct IoRequest *)ig->IPCReq);
   BUG(("Done\n"))

   if(ig->IPCReq->IO.io_Error != 0)
   {
      BUG(("********ERROR: Can't declare obj, io_Error %d\n", 
           ig->IPCReq->IO.io_Error))
      BUGR("Can't connect!")
      return(1) ;
   }

   ig->IPCReq->IO.io_Command = IPCMD_ACCEPT;

   BUG(("InitRDevice: Accepting connection..."))
   DoIO((struct IoRequest *)ig->IPCReq);
   BUG(("Done\n"))

   if(ig->IPCReq->IO.io_Error != 0)
   {
      BUG(("********ERROR: Can't accept, io_Error %d\n", ig->IPCReq->IO.io_Error))
      BUGR("No connection!")
      return(1) ;
   }

   global->n.devptr = (APTR)ig->IPCReq ;

   BUGP("InitRDevice: Exit")

   return(0);
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
   struct IOStdIpc *IPCReq ;
   IPCGLOBAL ig = global->n.d.ig;

   IPCReq = (struct IOStdIpc *)global->n.devptr ;

   if(ig)
   {
      if(!(ig->flags & NIPC_BADCON))
      {
         IPCReq->IO.io_Command = IPCMD_DISCONNECT ;
         DoIO (IPCReq) ;
      }

      CloseDevice(ig->timermsg) ;
      DeleteStdIO(ig->timermsg) ;
      DeletePort(ig->timerport) ;  
   }
   CloseDevice(IPCReq) ;
   FreeMem(IPCReq, (long)sizeof(*IPCReq)) ;
   DeletePort(ig->IPCReplyPort) ;

   DeletePort(global->n.devport);

   FreeMem((void *)ig, sizeof(*ig));
   global->n.d.ig = NULL;

   return(0);
}

void ActNetHello(global, pkt)
GLOBAL global;
struct DosPacket *pkt;
{
}
