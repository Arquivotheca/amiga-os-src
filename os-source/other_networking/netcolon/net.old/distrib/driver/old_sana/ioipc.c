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
#include "proto.h"
#include "netipc.h"

static int GetIO(NGLOBAL global, struct IOStdIpc *IPCReq, int size);

int PutRPacket(global, ioptr, RP)
NGLOBAL global;
APTR ioptr;
struct RPacket *RP;
{
   IPCGLOBAL ig = global->n.d.ig;
   struct IOStdIpc *IPCReq ;

   if(ig->flags & NIPC_BADCON) 
   {
      BUG(("PutRPacket: NIPC_BADCON, quitting\n"))
      return(1);
   }

   BUG(("PutRPacket: ioptr 0x%08lx type %d, Args %lx %lx %lx %lx\n", 
       ioptr, RP->Type, 
       RP->Arg1, RP->Arg2, 
       RP->Arg3, RP->Arg4));

   IPCReq = (struct IOStdIpc *)ioptr ;

   if(!IPCReq) return(1);

   BUG(("PutRPacket: writing %d to %lx. . .", RPSIZEN, ioptr));
   IPCReq->IO.io_Data = (APTR)RP ;
   IPCReq->IO.io_Command = CMD_WRITE ;
   IPCReq->IO.io_Length = RPSIZEN ;
   DoIO((struct IoRequest *)IPCReq);

   if (IPCReq->IO.io_Error != 0)
   {
      BUG(("\n**********ERROR %d\n", IPCReq->IO.io_Error));
      BUGR("NET write error");
      ig->flags |= NIPC_BADCON;
      return(1) ;
   }

   BUG(("%d written\n", IPCReq->IO.io_Actual));

   if(RP->DLen)
   {
      BUG(("PutRPacket: writing %d to %lx. . .", RP->DLen, ioptr));
      IPCReq->IO.io_Data = (APTR)RP->DataP ;
      IPCReq->IO.io_Command = CMD_WRITE ;
      IPCReq->IO.io_Length = RP->DLen ;
      DoIO((struct IoRequest *)IPCReq) ;

      if (IPCReq->IO.io_Error != 0)
      {
        BUG(("\n**********ERROR - code %d \n", IPCReq->IO.io_Error));
        BUGR("NET write error");
        ig->flags |= NIPC_BADCON;
        return(1) ;
      }
      BUG(("%d written\n", IPCReq->IO.io_Actual));
   }

   if(global->n.infoport)
   {
      struct Message *m;
      while(m=GetMsg(global->n.ntirec.m.mn_ReplyPort))
      {
         if(m == &global->n.ntirec.m)
            global->n.inuse_rec = 0;
         else
            global->n.inuse_trans = 0;
      }
      global->n.inf_trans += RPSIZEN + RP->DLen;
         
      if(!global->n.inuse_trans)
      {
         BUG(("PutRPacket: Writing status info to port %lx: TRANSMIT %ld\n", 
            global->n.infoport, global->n.inf_trans))

         global->n.ntitrans.nti_bytes = global->n.inf_trans;
         global->n.ntitrans.nti_direction = NTI_TRANSMIT;
         PutMsg(global->n.infoport, &global->n.ntitrans.m);

         global->n.inuse_trans = 1;
         global->n.inf_trans = 0;
      }
#if DEBUG
      else
         BUG(("PutRPacket: Skipping status write, packet outstanding\n"))
#endif
   }

   return(0);
}

int GetRPacket(global, ioptr, RP)
NGLOBAL global;
APTR ioptr ;
struct RPacket *RP;
{
   struct IOStdIpc *IPCReq ;
   IPCGLOBAL ig = global->n.d.ig;

   if(ig->flags & NIPC_BADCON) 
   {
      BUG(("GetRPacket: NIPC_BADCON, quitting\n"))
      return(1);
   }

   BUG(("GetRPacket: reading %d from %lx. . .", RPSIZEN, ioptr));

   IPCReq = (struct IOStdIpc *)ioptr ;

   IPCReq->IO.io_Data = (APTR)RP ;

   if(GetIO(global, IPCReq, RPSIZEN)) return(1);

   BUG(("Done\ntype %d, Args %lx %lx %lx %lx\n", RP->Type, 
       RP->Arg1, RP->Arg2, 
       RP->Arg3, RP->Arg4));

   if(RP->DLen > 0) 
   {
      BUG(("Reading %d more. . .", RP->DLen));
      IPCReq->IO.io_Data = (APTR)RP->DataP;

      if(GetIO(global, IPCReq, RP->DLen)) return(1);

      BUG(("Done\n"))
   }
   else
      RP->DataP[0] = '\0';


   if(global->n.infoport)
   {
      struct Message *m;
      while(m=GetMsg(global->n.ntirec.m.mn_ReplyPort))
      {
         if(m == &global->n.ntirec.m)
            global->n.inuse_rec = 0;
         else
            global->n.inuse_trans = 0;
      }
      global->n.inf_rec += RPSIZEN + RP->DLen;
         
      if(!global->n.inuse_rec)
      {
         BUG(("GetRPacket: Writing status info to port %lx: RECEIVE %ld\n",
            global->n.infoport, global->n.inf_rec))

         global->n.ntirec.nti_bytes = global->n.inf_rec;
         global->n.ntirec.nti_direction = NTI_RECEIVE;
         PutMsg(global->n.infoport, &global->n.ntirec.m);

         global->n.inuse_rec = 1;
         global->n.inf_rec = 0;
      }
#if DEBUG
      else
         BUG(("GetRPacket: Skipping status write, packet outstanding\n"))
#endif
   }

   return(0);
}

static int GetIO(NGLOBAL global, struct IOStdIpc *IPCReq, int size)
{
   long mask;
   struct Message *msg;
   IPCGLOBAL ig = global->n.d.ig;

   /* io_Data is set by caller */
   IPCReq->IO.io_Command = CMD_READ;
   IPCReq->IO.io_Length = size;

   SetSignal(0, ig->IPCMask);

   SendIO(ig->IPCReq);

   mask = Wait(ig->IPCMask);
   msg = GetMsg(ig->IPCReplyPort);
   if(mask & SIGBREAKF_CTRL_C)
   {
      /* Must be a CTRL-C interrupt for the server */
      BUG(("**********NET SERVER INTERRUPTED\n"))
      BUGR("NET Interrupted")
      global->n.run = 0;
      AbortIO(IPCReq);
      WaitIO(IPCReq);
      GetMsg(ig->IPCReplyPort);
      return(1);
   }

   if (IPCReq->IO.io_Actual != size || IPCReq->IO.io_Error != 0)
   {
      BUG(("**********ERROR - read %d, error %d\n", 
         IPCReq->IO.io_Actual, IPCReq->IO.io_Error))
      BUGR("NET read error")
      ig->flags |= NIPC_BADCON;
      return(1) ;   
   }

   return(0);
}

int OpenIPC(NGLOBAL global)
{
   int errorcode ;
   char *device;
   long unit;
   char devname[256];
   IPCGLOBAL ig = global->n.d.ig;

   ig->IPCReplyPort = CreatePort (0L,0L) ;

   if(ig->IPCReplyPort == NULL)
   {
      BUG(("********ERROR: Can't Create IPC Reply Port!!\n"));
      return(1);
   }

   ig->IPCReq = (struct IOStdIpc *)
      AllocMem ((long)sizeof(*ig->IPCReq), MEMF_PUBLIC | MEMF_CLEAR) ;

   if(ig->IPCReq == NULL)
   {
      BUG(("********ERROR: Can't get mem for ipc req!!\n"));
      return(1);
   }

   ig->IPCReq->IO.io_Message.mn_ReplyPort = ig->IPCReplyPort ;

   if(device=(char *)global->n.fssm.fssm_Device)
   {
      memcpy(devname, device+1, device[0]);
      devname[device[0]] = '\0';
   }
   else
      strcpy(devname, DEFAULTDEV);

   unit = global->n.fssm.fssm_Unit;

OPENIT:
   if (errorcode=OpenDevice(devname, unit, (struct IoRequest *)ig->IPCReq, 0L))
   {
      BUG(("********ERROR: Can't open device %s unit %d!!\n", device, unit));
      BUGR("NET can't open device!")
      if(strcmp(devname, DEFAULTDEV))
      {
         strcpy(devname, DEFAULTDEV);
         goto OPENIT;
      }
      return(1);
   }

   ig->IPCBit = 1L << ig->IPCReplyPort->mp_SigBit ;
   ig->IPCMask = ig->IPCBit;

   ig->timerport = (struct MsgPort *)CreatePort(0L, 0L) ;
   ig->timermsg = (struct IOStdReq *)CreateStdIO(ig->timerport) ;
   OpenDevice(TIMERNAME, (LONG)UNIT_VBLANK, ig->timermsg, 0L) ;
   ig->timerbit = 1L << ig->timerport->mp_SigBit ;

   return(0);
}
