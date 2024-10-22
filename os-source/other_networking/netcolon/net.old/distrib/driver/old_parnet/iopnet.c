
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1988 The Software Distillery.  All Rights Reserved *
* |. o.| ||	     Written by Doug Walker				    *
* | .  | ||	     The Software Distillery				    *
* | o  | ||	     235 Trillingham Lane				    *
* |  . |//	     Cary, NC 27513					    *
* ======	     BBS:(919)-471-6436                                     *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "netcomm.h"
#include "proto.h"

#include <exec/interrupts.h>
#include <resources/misc.h>
#include <hardware/cia.h>
#include "netpnet.h"

int ReSync(global, ioptr)
GLOBAL global;
APTR ioptr;
{
   struct NPNGLOBAL *ng = global->n.d.ng;
   return(ng->Broke != 0);  /* Can't resync if user broke */
}

int PutRPacket(global, ioptr, RP)
NGLOBAL global;
APTR ioptr;
struct RPacket *RP;
{
   struct NPNGLOBAL *ng = global->n.d.ng;
   BUG(("PutRPacket: type %d, Args %lx %lx %lx %lx\n", RP->Type,
      RP->Arg1, RP->Arg2, RP->Arg3, RP->Arg4));

   if (SetSignal(0, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
      ng->Broke = 1;

   ng->Piow.io_Data   = (APTR)RP;
   ng->Piow.io_Length = RPSIZEN;
   if(RP->DLen) 
   {
      ng->Piow.io_Data2  = (APTR)RP->DataP;
      ng->Piow.io_Length2= RP->DLen;
   } 
   else 
   {
      ng->Piow.io_Data2 = NULL;
      ng->Piow.io_Length2= 0;
   }

   BUG(("PutRPacket: writing %d, %d to %lx. . .", RPSIZEN, 
      ng->Piow.io_Length2, ioptr));
   DoIO((IOR *)&ng->Piow);
   BUG(("done\n"))

   if (ng->Piow.io_Error) 
   {
      BUGR("Network write error");
      return(1);
   }

   if(global->n.infoport) 
   {
      struct Message *m;
      while (m = GetMsg(global->n.ntirec.m.mn_ReplyPort)) 
      {
         if (m == &global->n.ntirec.m)
            global->n.inuse_rec = 0;
         else
            global->n.inuse_trans = 0;
      }
      global->n.inf_trans += RPSIZEN + RP->DLen;

      if (!global->n.inuse_trans) 
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

int
GetRPacket(global, ioptr, RP)
NGLOBAL global;
APTR ioptr;
struct RPacket *RP;
{
   int len;
   IOParReq *io;
   struct NPNGLOBAL *ng = global->n.d.ng;

   BUG(("GetRPacket: reading %d from %lx. . .", RPSIZEN, ioptr));

   while ((io = (IOParReq *)GetMsg(ng->IoPort)) == NULL) 
   {
      if (Wait(SIGBREAKF_CTRL_C | (1 << ng->IoPort->mp_SigBit)) & SIGBREAKF_CTRL_C)
         return(1);
   }
   BUG(("done\n", io->io_Actual))

   /* SetPacketBuffer(RP, NETBUFSIZE); */

   len = io->io_Actual;
   if (len < RPSIZEN) 
   {
     SendIO((IOR *)io);
     BUGR("Network read error");
     return(1);
   }
   memcpy((char *)RP, (char *)io->io_Data, RPSIZEN);
   len -= RPSIZEN;

   RP->DataP[0] = 0;

   if (len)
   {
      BUG(("Copying %d additional bytes\n", len))
      memcpy(RP->DataP, (char *)io->io_Data + RPSIZEN, len);
   }

   SendIO((IOR *)io);

   if (len < RP->DLen) 
   {
     BUGR("Network read error 2");
     return(1);
   }

   if (global->n.infoport) 
   {
      struct Message *m;
      while (m = GetMsg(global->n.ntirec.m.mn_ReplyPort)) 
      {
         if (m == &global->n.ntirec.m)
            global->n.inuse_rec = 0;
         else
            global->n.inuse_trans = 0;
      }
      global->n.inf_rec += RPSIZEN + RP->DLen;

      if (!global->n.inuse_rec) 
      {
         BUG(("GetRPacket: Writing status info to port %lx: RECEIVE %ld\n",
               global->n.infoport, global->n.inf_rec));

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

int OpenPnet(NGLOBAL global, struct NPNGLOBAL *ng)
{
   /*
    *   Open parnet.device
    */

   ng->IoPort = CreatePort(NULL, 0);

   /*  communications port for datagrams        */
   ng->Pioc.io_Port = 1024 + global->n.fssm.fssm_Unit;
   ng->Pioc.io_Flags= PRO_DGRAM;
   ng->Pioc.io_Message.mn_ReplyPort = ng->IoPort;
   if (OpenDevice("parnet.device", 0, (IOR *)&ng->Pioc, 0))
      return(1);

   /*
    *   Set my address
    */

   ng->Pioc.io_Addr = ng->MyAddr;
   ng->Pioc.io_Command = PPD_SETADDR;
   DoIO((IOR *)&ng->Pioc);

   ng->Pioc.io_Addr = ng->DestAddr;
   ng->Pior[0] = ng->Pioc;
   ng->Pior[1] = ng->Pioc;
   ng->Piow = ng->Pioc;
   ng->Pior[0].io_Command = CMD_READ;
   ng->Pior[0].io_Data = (APTR)ng->RxBuf[0];
   ng->Pior[0].io_Length = sizeof(ng->RxBuf[0]);
   ng->Pior[1].io_Command = CMD_READ;
   ng->Pior[1].io_Data = (APTR)ng->RxBuf[1];
   ng->Pior[1].io_Length = sizeof(ng->RxBuf[0]);
   ng->Piow.io_Command = CMD_WRITE;

   SendIO((IOR *)&ng->Pior[0]);
   SendIO((IOR *)&ng->Pior[1]);
   ng->PiorIP[0] = 1;
   ng->PiorIP[1] = 1;

   return(0);
}

int ClosePnet(NGLOBAL global, struct NPNGLOBAL *ng)
{
    if (ng->PiorIP[0]) 
    {
        AbortIO((IOR *)(ng->Pior + 0));
        WaitIO((IOR *)(ng->Pior + 0));
        ng->PiorIP[0] = 0;
    }
    if (ng->PiorIP[1]) 
    {
        AbortIO((IOR *)(ng->Pior + 1));
        WaitIO((IOR *)(ng->Pior + 1));
        ng->PiorIP[1] = 0;
    }
    CloseDevice((IOR *)&ng->Pioc);
    DeletePort(ng->IoPort);
    FreeMem((void *)ng, sizeof(struct NPNGLOBAL));
    return(0);
}
