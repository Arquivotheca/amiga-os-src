/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1988 The Software Distillery.  All Rights Reserved *
* |. o.| ||          Written by Doug Walker                                 *
* | .  | ||          The Software Distillery                                *
* | o  | ||          235 Trillingham Lane                                   *
* |  . |//           Cary, NC 27513                                         *
* ======             BBS:(919)-471-6436                                     *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "netcomm.h"
#include "handler.h"
#include "netinet.h"

#include <exec/interrupts.h>
#include <resources/misc.h>
#include <stdio.h>
#include <fcntl.h>
#include <ios1.h>
#include <string.h>
#include <stdlib.h>

#if DEBUG
char *dbgwind = "CON:0/0/640/160/NETINET-HANDLER/a";
#endif /* DEBUG */

#if 0
/*********** These are to define global symbols the inet.library needs *******/
/*           (but doesn't really use - we hope!                              */
struct Library *SysBase;
char *_ProgramName = "NETINET-HANDLER";
char *_StackPtr;
long _base;
long _oserr;
long _OSERR;

/* These two write to location 0 to generate a memwatch/enforcer hit */
void XCEXIT(long l); void XCEXIT(long l) { long *x = 0; *x = 0x58434558; /* XCEX */}
long _ONBREAK;

#endif

/*****************************************************************************/

InitRDevice(global)
GLOBAL global;
{
   INETGLOBAL tg;

   if(global->n.d.tg)  return(0);  /* Already initialized */

   if(!(global->n.d.tg = AllocMem(sizeof(*tg), MEMF_CLEAR)))
   {
      return(1);
   }

   if(!global->RP.DataP)
   {
      global->n.NetBufSize = NETBUFSIZE;
      SetPacketBuffer(&global->RP, NETBUFSIZE);
   }

   BUGP("InitRDevice: Exit")

   return(0);
}

int TermRDevice(GLOBAL global, int force)
{
   struct NetNode *netnode, *tmpnode;

   for(netnode=global->netchain.next; netnode; netnode=tmpnode) 
   {
      TermNode(global, netnode);
      tmpnode = netnode->next;
      DosFreeMem((void *)netnode);
   }

   return(0);
}

int InitNode(GLOBAL global, struct NetNode *nnode)
{
   int s;
   struct sockaddr_in sin;
   struct hostent *he;
   int on = 1;
   
   s = socket(AF_INET, SOCK_STREAM, 0);
   if(s < 0)
   {
      BUG(("********ERROR: Create Socket - errno=%ld\n", errno))
      BUGR("No Socket!")
      nnode->status = NODE_DEAD;
      return(1) ;
   }
   sin.sin_family       = AF_INET;
   sin.sin_port         = htons(7656);
   if(he = gethostbyname(nnode->name+1))
   {
      sin.sin_addr.s_addr = *(long *)he->h_addr_list[0];
      BUG(("Setting address to %08lx\n", sin.sin_addr.s_addr));
   }
   else if((sin.sin_addr.s_addr  = inet_addr(nnode->name+1)) == INADDR_NONE)
   {
      BUG(("No such address '%s'\n", nnode->name+1))
      nnode->status = NODE_DEAD;
      return(1);
   }

   if(connect(s, &sin, sizeof(sin)) < 0)
   {
      BUG(("********ERROR: Can't Connect Socket - errno=%ld\n", errno))
      BUGR("Can't Connect!")
      nnode->status = NODE_DEAD;
      return(1) ;
   }
   if (setsockopt(s,IPPROTO_TCP,TCP_NODELAY,&on,sizeof(on)) < 0)
      BUGR("CAN'T SET OPTIONS");

   nnode->ioptr = (APTR)s;
   BUG(("Connection is now %ld for %08lx '%s' errno=%d\n", 
      nnode->ioptr, nnode, nnode->name+1, errno));

   nnode->status = NODE_UP;
   return(0);
}

int TermNode(GLOBAL global, struct NetNode *nnode)
{
   if(nnode->status == NODE_UP)
   {
      close((int)nnode->ioptr);
      nnode->status = NODE_DEAD;
   }
   return(0);
}

int PostRPacket(GLOBAL global, struct NetNode *nnode, struct RPacket *RP)
{
   int s;
   global->lastnode = nnode;

   BUG(("PostRPacket: Con: %ld type %d, Args %lx %lx %lx %lx\n", 
       nnode->ioptr, RP->Type, 
       RP->Arg1, RP->Arg2, 
       RP->Arg3, RP->Arg4));

   if(nnode->status != NODE_UP && InitNode(global, nnode))
      return(1);

   s = (int)nnode->ioptr;

   BUG(("PostRPacket: s = %ld for %08lx\n", s, nnode));

   RP->signature = SIGVALUE;
   BUG(("PostRPacket: writing %d to confd %ld. . .", RPSIZEN, s));

   if (writen(s, RP, RPSIZEN) != RPSIZEN)
   {
      /* Something is really bad here - I bet they closed the connection        */
      /* We need to return a failure.                                           */
      BUGR("NET write error")
      nnode->status = NODE_CRASHED;
      close(s);
      return(1);
   }

   BUG(("%ld written\n", RPSIZEN));

   if(RP->DLen)
   {
      BUG(("PostRPacket: writing %d to confd %ld. . .", RP->DLen, s));
      if (writen(s, RP->DataP, RP->DLen) != RP->DLen)
      {
        BUGR("NET write error");
        nnode->status = NODE_CRASHED;
        close(s);
        return(1) ;
      }
      BUG(("%d written\n", RP->DLen));
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
         BUG(("PostRPacket: Writing status info to port %lx: TRANSMIT %ld\n", 
            global->n.infoport, global->n.inf_trans))

         global->n.ntitrans.nti_bytes = global->n.inf_trans;
         global->n.ntitrans.nti_direction = NTI_TRANSMIT;
         PutMsg(global->n.infoport, &global->n.ntitrans.m);

         global->n.inuse_trans = 1;
         global->n.inf_trans = 0;
      }
#if DEBUG
      else
         BUG(("PostRPacket: Skipping status write, packet outstanding\n"))
#endif
   }
   return(0);
}

int PluckRPacket(GLOBAL global, struct RPacket *RP)
{
   int s;
   int size;

   if(global->lastnode->status != NODE_UP && InitNode(global, global->lastnode))
      return(1);

   s = (int)global->lastnode->ioptr;

   BUG(("PluckRPacket: reading %d from Connection %ld. . .", RPSIZEN, s));

   RP->DataP[0] = '\0';
   size = readn(s, RP, RPSIZEN);
   if (RP->signature != SIGVALUE)
   {
      BUGR("Pluck Signature Failure");
      BUG(("PluckRPacket: Signature Failure\n"));
      global->lastnode->status = NODE_CRASHED;
      global->lastnode->incarnation++;
      close(s);
      return(1);
   }

   BUG(("PluckRPacket - read %d into %08lx\n", RP->DLen, RP->DataP));   
   /* DO a sanity check on what we get.  If it is even slightly wrong  */
   /* we should immediately terminate the connection.  If it was a     */
   /* legitimate user, he will have to reestablish the connection.     */
   /* Note that this will cause any outstanding locks or file handles  */
   /* to be purged automatically.                                      */
   if ((size != RPSIZEN)       ||
       (RP->DLen > NETBUFSIZE) ||
       (RP->DLen < 0)          ||
       ((RP->DLen != 0) &&
        (readn(s, RP->DataP, RP->DLen) != RP->DLen)))
   {
      /* Something is wrong here, assume that the connection */
      /* is now closed and get rid of it.                    */
      BUGR("Connection Closed");
      global->lastnode->status = NODE_CRASHED;
      global->lastnode->incarnation++;
      close(s);
      return(1);
   }

   BUG(("Done\ntype %d, Args %lx %lx %lx %lx\n", RP->Type, 
           RP->Arg1, RP->Arg2, 
           RP->Arg3, RP->Arg4));

   /* See if we have a monitor process to inform of the event          */
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
         BUG(("PluckRPacket: Writing status info to port %lx: RECEIVE %ld\n",
            global->n.infoport, global->n.inf_rec))

         global->n.ntirec.nti_bytes = global->n.inf_rec;
         global->n.ntirec.nti_direction = NTI_RECEIVE;
         PutMsg(global->n.infoport, &global->n.ntirec.m);

         global->n.inuse_rec = 1;
         global->n.inf_rec = 0;
      }
#if DEBUG
      else
         BUG(("PluckRPacket: Skipping status write, packet outstanding\n"))
#endif
   }
   return(0);
}
