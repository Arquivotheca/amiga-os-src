/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1988 The Software Distillery.  All Rights Reserved *
* |. o.| ||          Written by Doug Walker                                 *
* | .  | ||          The Software Distillery                                *
* | o  | ||          235 Trillingham Lane                                   *
* |  . |//           Cary, NC 27513                                         *
* ======             BBS:(919)-471-6436                                     *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define NETCOMMON
#include "netdnet.h"
#include "netcomm.h"
#include "proto.h"

int PutRPacket(global, ioptr, RP)
NGLOBAL global;
APTR ioptr;
struct RPacket *RP;
{
   int len;

   BUG(("PutRPacket: type %d, Args %lx %lx %lx %lx\n", RP->Type,
       RP->Arg1, RP->Arg2,
       RP->Arg3, RP->Arg4));


TOP:
   BUG(("PutRPacket: writing %d to %lx. . .", RPSIZEN, ioptr));
   if((len=DWrite((struct DChannel *)ioptr, (char *)RP, RPSIZEN)) != RPSIZEN)
   {
      BUG(("**********ERROR - wrote %d instead\n", len));
      BUGR("Write error");
      if(ReSync((GLOBAL)global, ioptr)) return(1);
      else goto TOP;
   }
   BUG(("%d written\n", len));

   if(RP->DLen)
   {
      BUG(("PutRPacket: writing %d to %lx. . .", RP->DLen, ioptr));
      if((len=DWrite((struct DChannel *)ioptr,
                     RP->DataP, RP->DLen)) != RP->DLen)
      {
         BUG(("**********ERROR - wrote %d instead\n", len));
         BUGR("Write error 2");
         if(ReSync((GLOBAL)global, ioptr)) return(1);
         else goto TOP;
      }
      BUG(("%d written\n", len));
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
APTR ioptr;
struct RPacket *RP;
{
   int len;

   BUG(("GetRPacket: reading %d from %lx. . .", RPSIZEN, ioptr));

TOP:
   if((len=DRead((struct DChannel *)ioptr, (char *)RP, RPSIZEN)) != RPSIZEN)
   {
      BUG(("**********ERROR - read %d instead\n", len));
      BUGR("Read error")
      if(ReSync((GLOBAL)global, ioptr)) return(1);
      else goto TOP;
   }

   BUG(("type %d, Args %lx %lx %lx %lx\n", RP->Type,
       RP->Arg1, RP->Arg2,
       RP->Arg3, RP->Arg4));

   if(RP->DLen > 0)
   {
      BUG(("Reading %d more. . .", RP->DLen));
      if((len=DRead((struct DChannel *)ioptr,
             RP->DataP, RP->DLen)) != RP->DLen)
      {
         BUG(("**********ERROR - read %d instead\n", len));
         BUGR("Read error 2")
         if(ReSync((GLOBAL)global, ioptr)) return(1);
         else goto TOP;
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

