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
#include <socket.h>

#include "server.h"
#include "netinet.h"

int ReplyRPacket(global, ses, RP)
NGLOBAL global;
SESSION *ses;
struct RPacket *RP;
{
   BUG(("ReplyRPacket: Con: %ld type %d, Args %lx %lx %lx %lx\n", 
       ses->driver, RP->Type, 
       RP->Arg1, RP->Arg2, 
       RP->Arg3, RP->Arg4));

   BUG(("ReplyRPacket: writing %d to confd %ld. . .", RPSIZEN, ses->driver));
   RP->signature = SIGVALUE;
   if (writen(ses->driver, RP, RPSIZEN) != RPSIZEN)
   {
      /* Something is really bad here - I bet they closed the connection        */
      /* We need to return a failure.                                           */
      BUGR("NET write error")
      return(1);
   }

   BUG(("%ld written\n", RPSIZEN));

   if(RP->DLen)
   {
      BUG(("ReplyRPacket: writing %d to confd %ld. . .", RP->DLen, ses->driver));
      if (writen(ses->driver, RP->DataP, RP->DLen) != RP->DLen)
      {
        BUGR("NET write error");
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
         BUG(("ReplyRPacket: Writing status info to port %lx: TRANSMIT %ld\n", 
            global->n.infoport, global->n.inf_trans))

         global->n.ntitrans.nti_bytes = global->n.inf_trans;
         global->n.ntitrans.nti_direction = NTI_TRANSMIT;
         PutMsg(global->n.infoport, &global->n.ntitrans.m);

         global->n.inuse_trans = 1;
         global->n.inf_trans = 0;
      }
#if DEBUG
      else
         BUG(("ReplyRPacket: Skipping status write, packet outstanding\n"))
#endif
   }
   return(0);
}

SESSION * AcceptRPacket(global, RP)
NGLOBAL global;
struct RPacket *RP;
{
   INETGLOBAL tg = global->n.d.tg;
   int len, nfound, size;
   long i, j;

   for(;;)
   {     
      while(tg->fhscurmask == 0)
      {
         tg->fhscurmask = tg->fhsmask;
   
         /* First we wait for any outstanding activity */
         if ((nfound = my_select(&tg->fhscurmask)) == -1)
         {
            BUGR("Select Failed");
            return(1);
         }
         else if (nfound == 0)
         {
            tg->fhscurmask = 0;
         }
      }
   
      /* We have something that has happened on a connection               */
      /* Find out what it is.  We will give first attack to anyone that    */
      /* is establishing a new connection.                                 */
      if (tg->fhscurmask & tg->sockmask)
      {
         /* Reset the flag to indicate that we have processed it           */
         tg->fhscurmask &= ~tg->sockmask;
         /* We have a new connection */
         BUG(("Special event\n"));
         if (tg->fhscount >= (MAXCONNECT-1))
         {
            BUG(("Too many connections, ignoring it\n"));
         }
         else
         {
            len = sizeof(&tg->sin);
            tg->fhs[tg->fhscount] = accept(tg->sock, &tg->sin, &len);
            if (tg->fhs[tg->fhscount] == -1)
            {
               BUGR("Accept Error");
            }
            else
            {
               tg->fhsmask |= INETMASK(tg->fhs[tg->fhscount]);
               BUG(("New connection %ld on: port=%ld Addr=%08lx\n",
                     tg->fhscount, tg->sin.sin_port, tg->sin.sin_addr.s_addr));
               tg->ses[tg->fhscount] = NewSession(global);
               tg->ses[tg->fhscount]->driver = (void *)tg->fhs[tg->fhscount];
               tg->fhscount++;
            }
         }
      }
   
      /* No new connections, but someone came in with a request */
      for(i = 0; i < tg->fhscount; i++)
      {
         if (tg->fhscurmask & INETMASK(tg->fhs[i]))
         {
            tg->curcon = tg->fhs[i];

            /* This is the one that sent us a message */
            /* Reset the flag so that we know that we have processed it    */
            tg->fhscurmask &= ~INETMASK(tg->curcon);
            BUG(("AcceptRPacket: reading %d from Connection %ld. . .", RPSIZEN, i));

            RP->DataP[0] = '\0';
            size = readn(tg->curcon, RP, RPSIZEN);
            /* DO a sanity check on what we get.  If it is even slightly wrong  */
            /* we should immediately terminate the connection.  If it was a     */
            /* legitimate user, he will have to reestablish the connection.     */
            /* Note that this will cause any outstanding locks or file handles  */
            /* to be purged automatically.                                      */
            if ((size != RPSIZEN)       ||
                (RP->DLen > NETBUFSIZE) ||
                (RP->DLen < 0)          ||
                ((RP->DLen != 0) &&
                 (readn(tg->curcon, RP->DataP, RP->DLen) != RP->DLen)))
            {
               /* Something is wrong here, assume that the connection */
               /* is now closed and get rid of it.                    */
               BUGR("Connection Closed");
               tg->fhsmask ^= INETMASK(tg->curcon);
               close(tg->curcon);
               /* We need to return a packet indicating that the connection     */
               /* has now been broken                                           */
               RP->Type = ACTION_NETWORK_TERM;
               RP->Arg1 = (BPTR)tg->ses[i];
               RP->DLen = 0;
               BUG(("Done\ntype %d, Args %lx %lx %lx %lx\n", RP->Type, 
                    RP->Arg1, RP->Arg2, 
                    RP->Arg3, RP->Arg4));
               /* We also need to remove the dead connection from the table of  */
               /* open connections.                                             */
               tg->fhscount--;
               for(j = i; j < tg->fhscount; j++)
               {
                  tg->fhs[j] = tg->fhs[j+1];
                  tg->ses[j] = tg->ses[j+1];
               }
               return((SESSION *)RP->Arg1);
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
                  BUG(("AcceptRPacket: Writing status info to port %lx: RECEIVE %ld\n",
                     global->n.infoport, global->n.inf_rec))
         
                  global->n.ntirec.nti_bytes = global->n.inf_rec;
                  global->n.ntirec.nti_direction = NTI_RECEIVE;
                  PutMsg(global->n.infoport, &global->n.ntirec.m);
         
                  global->n.inuse_rec = 1;
                  global->n.inf_rec = 0;
               }
#if DEBUG
               else
                  BUG(("AcceptRPacket: Skipping status write, packet outstanding\n"))
#endif
            }
            return(tg->ses[i]);
         }
      }

      if (tg->fhscurmask)
      {
         BUGR("UNKNOWN HANDLE!");
         tg->fhscurmask = 0;
      }
   }
}


extern int socket_sigio;
extern void *_socks[FD_SETSIZE];
extern struct InetBase *InetBase;

int my_select(readfds)
	register fd_set	*readfds;
{
	fd_set rd_fd, save_rd;
	long event, selectmask;
	int returnval = 0;
	int selectbit;
	struct selectargs sa;
	long usermask = SIGBREAKF_CTRL_C;

	selectbit = AllocSignal(-1L);
	if(selectbit == -1){
		selectbit = socket_sigio;
	}
	selectmask = 1L << selectbit;

	sa.numfds = 32;
	sa.socks = _socks;
	sa.rd_set = readfds? &rd_fd : 0;
	sa.wr_set = sa.ex_set = 0;
	sa.task = FindTask(0L);
	sa.sigbit = selectbit;

	save_rd.fds_bits[0] = readfds->fds_bits[0];
	save_rd.fds_bits[1] = readfds->fds_bits[1];

	do {
		event = 0L;

		rd_fd = save_rd;

		sa.rval = sa.errno = 0;
		SelectAsm(&sa);
		if(sa.errno){
			errno = sa.errno;
			returnval = -1;
			break;
		}
		if(returnval = sa.rval){
			event = usermask & SetSignal(0L, 0L);
			break;
		}

		event = Wait(usermask | selectmask);
		if(event & usermask){
			errno = EINTR;
			returnval = -1;
			break;
		}

		if(event & selectmask){
			continue;
		}

	} while(1);

	readfds->fds_bits[0] = rd_fd.fds_bits[0];
	readfds->fds_bits[1] = rd_fd.fds_bits[1];

	if(selectbit != socket_sigio){
		FreeSignal(selectbit);
	}

	return returnval;
}
