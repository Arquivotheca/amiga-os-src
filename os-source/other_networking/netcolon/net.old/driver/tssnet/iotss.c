
/*******************************************************************/
/*                                                                 */
/*    iotss.c -  TSSnet (TM) net access routines                   */
/*                                                                 */
/*                                                                 */
/*******************************************************************/

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


#include "TSSnetUInc.h"

VOID SetTimer (unsigned long, unsigned long, struct IOStdReq *) ;

extern struct IOStdReq *timermsg ;
extern struct MsgPort *timerport ;
extern ULONG timerbit ;

extern struct MsgPort *TSSReplyPort;
extern ULONG TSSMask;


#define NETCOMMON
#include "netdnet.h"
#include "netcomm.h"
#include "proto.h"

int PutRPacket(global, ioptr, RP)
NGLOBAL global;
APTR ioptr;
struct RPacket *RP;
{
   struct IOExtTSS *TSSreq ;
   int len;

   BUG(("PutRPacket: ioptr 0x%08lx type %d, Args %lx %lx %lx %lx\n", 
       ioptr, RP->Type, 
       RP->Arg1, RP->Arg2, 
       RP->Arg3, RP->Arg4));

TSSreq = (struct IOExtTSS *)ioptr ;

/* there is a better way to do this - later */

               while (!TSSreq->IOTSS.io_Error && TSSreq->User.WriteCnt > 3)
                 {
                  SetTimer (0L, 16666L, timermsg) ;
                  WaitIO (timermsg) ;
                  TSSreq->IOTSS.io_Command = TSCMD_QUERY ;
                  DoIO (TSSreq) ;
                 }

               if (TSSreq->IOTSS.io_Error == 0)
                 {
 BUG(("PutRPacket: writing %d to %lx. . .", RPSIZEN, ioptr));
                  TSSreq->IOTSS.io_Data = (APTR)RP ;
                  TSSreq->IOTSS.io_Command = CMD_WRITE ;
                  TSSreq->IOTSS.io_Length = RPSIZEN ;
                  DoIO (TSSreq) ;
                 }

               if (TSSreq->IOTSS.io_Error != 0)
                 {
                  BUG(("**********ERROR %d - status %d \n", 
                       TSSreq->IOTSS.io_Error, TSSreq->User.Status));
                  BUGR("Write error");
                  if(TSSreq->User.Status < 0 || 
                       TSSreq->IOTSS.io_Error == REM_DISCONNECT)
                     TermRDevice(global, 1);
                  return (1) ;
                 }

   BUG(("%d written\n", TSSreq->IOTSS.io_Actual));

   if(RP->DLen)
   {
               while (!TSSreq->IOTSS.io_Error && TSSreq->User.WriteCnt > 3)
                 {
                  SetTimer (0L, 16666L, timermsg) ;
                  WaitIO (timermsg) ;
                  TSSreq->IOTSS.io_Command = TSCMD_QUERY ;
                  DoIO (TSSreq) ;
                 }
               if (TSSreq->IOTSS.io_Error == 0)
                 {
   BUG(("PutRPacket: writing %d to %lx. . .", RPSIZEN, ioptr));
                  TSSreq->IOTSS.io_Data = (APTR)RP->DataP ;
                  TSSreq->IOTSS.io_Command = CMD_WRITE ;
                  TSSreq->IOTSS.io_Length = RP->DLen ;
                  DoIO (TSSreq) ;
                 }

               if (TSSreq->IOTSS.io_Error != 0)
                 {
                  BUG(("**********ERROR - status %d \n", TSSreq->User.Status));
                  BUGR("Write error 2");
                  if(TSSreq->User.Status < 0 ||
                       TSSreq->IOTSS.io_Error == REM_DISCONNECT)
                     TermRDevice(global, 1);
                  return (1) ;
                 }

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
   int len;
   struct IOExtTSS *TSSreq ;

   BUG(("GetRPacket: reading %d from %lx. . .", RPSIZEN, ioptr));



 TSSreq = (struct IOExtTSS *)ioptr ;

 TSSreq->IOTSS.io_Data = (APTR)RP ;
 TSSreq->IOTSS.io_Command = TSCMD_READM ;   /* read a message */
 TSSreq->IOTSS.io_Length = RPSIZEN ;

 SendIO(TSSreq);

 if(Wait(TSSMask) & SIGBREAKF_CTRL_C)
 {
   /* Must be a CTRL-C interrupt for the server */
   BUG(("**********NET SERVER INTERRUPTED\n"))
   BUGR("Net Server Interrupted")
   global->n.run = 0;
   return(1);
 }

if (TSSreq->IOTSS.io_Actual != RPSIZEN)
  {
   BUG(("**********ERROR - read %d instead\n", TSSreq->IOTSS.io_Actual));
   BUGR("Read error")
   return (1) ;   
  }

if (TSSreq->IOTSS.io_Error != 0)
  {
   BUG(("**********ERROR - status = %d\n", TSSreq->User.Status));
   BUGR("Read error")
   if(TSSreq->User.Status < 0 || TSSreq->IOTSS.io_Error == REM_DISCONNECT)
      TermRDevice(global, 1);
   return (1) ;   
  }





   BUG(("type %d, Args %lx %lx %lx %lx\n", RP->Type, 
       RP->Arg1, RP->Arg2, 
       RP->Arg3, RP->Arg4));

   if(RP->DLen > 0) 
   {
      BUG(("Reading %d more. . .", RP->DLen));
 TSSreq->IOTSS.io_Data = (APTR)RP->DataP ;
 TSSreq->IOTSS.io_Command = TSCMD_READM ;
 TSSreq->IOTSS.io_Length = RP->DLen ;
 DoIO (TSSreq) ;

if (TSSreq->IOTSS.io_Actual != RP->DLen)
  {
   BUG(("**********ERROR - read %d instead\n", TSSreq->IOTSS.io_Actual));
   BUGR("Read error")
   return (1) ;   
  }

if (TSSreq->IOTSS.io_Error != 0)
  {
   BUG(("**********ERROR - status = %d\n", TSSreq->User.Status));
   BUGR("Read error")
   if(TSSreq->User.Status < 0 || TSSreq->IOTSS.io_Error == REM_DISCONNECT)
      TermRDevice(global, 1);
   return (1) ;   
  }

   }
   else
      RP->DataP[0] = '\0';

   BUG(("Done\n"))

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

