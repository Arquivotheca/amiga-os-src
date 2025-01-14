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
#include "netinet.h"

#ifdef CPR
char *dbgwind = "CON:0/0/640/160/NETIPC-SERVER/a";
#endif

#if 0
/*********** These are to define global symbols the inet.library needs *******/
/*           (but doesn't really use - we hope!                              */
struct Library *SysBase;
char *_ProgramName = "NETINET-SERVER";
char *_StackPtr;
long _base;
long _oserr;
long _OSERR;

/* These two write to location 0 to generate a memwatch/enforcer hit */
void XCEXIT(long l); void XCEXIT(long l) { long *x = 0; *x = 0x58434558; /* XCEX */}
long _ONBREAK;
#endif

int ReSync(global, ioptr, name)
GLOBAL global;
APTR *ioptr;
char *name;
{
   BUGP("ReSync: Called");
   return(1);
}

int InitRDevice(GLOBAL global)
{
   INETGLOBAL tg = global->n.d.tg;
   int on = 1;

   BUGP("InitRDevice: Entry")

   if(!tg)
   {
      if(!(tg = (void *)AllocMem(sizeof(*tg), MEMF_CLEAR)))
      {
         return(1);
      }
      global->n.d.tg = tg;
   }

   tg->sock = socket(AF_INET, SOCK_STREAM, 0);
   if(tg->sock < 0)
   {
      BUG(("********ERROR: Create Socket - errno=%ld\n", errno))
      BUGR("No Socket!")
      return(1) ;
   }
   tg->fhsmask = tg->sockmask = INETMASK(tg->sock);
   setsockopt(tg->sock,IPPROTO_TCP,TCP_NODELAY,&on,sizeof(on));
   tg->sin.sin_family       = AF_INET;
   tg->sin.sin_port         = htons(7656);
   tg->sin.sin_addr.s_addr  = INADDR_ANY;
   if(bind(tg->sock, &tg->sin, sizeof(tg->sin)) < 0)
   {
      BUG(("********ERROR: Can't Bind Socket - errno=%ld\n", errno))
      BUGR("Can't Bind!")
      return(1) ;
   }

   tg->timeout.tv_sec = 5;
   tg->timeout.tv_usec = 0;

   listen(tg->sock, MAXCONNECT);

   if(!global->RP.DataP)
   {
      SetPacketBuffer(&global->RP, NETBUFSIZE);
      global->n.NetBufSize = NETBUFSIZE;
   }
   
   if (tg->fhscurmask)
   {
      BUGR("Non-Zero!");
      tg->fhscurmask = 0;
   }

/*   global->n.devptr = (APTR)tg->IPCReq ;*/

   BUGP("InitRDevice: Exit")
   
   return(0);
}


int TermRDevice(global, status)
GLOBAL global;
int status;
{
   INETGLOBAL tg = global->n.d.tg;
   int i;

   if(tg)
   {
      for(i = 0; i < tg->fhscount; i++)
         close(tg->fhs[i]);
      close(tg->sock);
      FreeMem((void *)tg, sizeof(*tg));
      global->n.d.tg = NULL;
   }

   return(0);
}

void ActNetHello(global, pkt)
GLOBAL global;
struct DosPacket *pkt;
{
}
