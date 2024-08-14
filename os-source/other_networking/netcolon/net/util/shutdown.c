
/*
 *  SHUTDOWN.C
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <libraries/filehandler.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <proto/exec.h>
#include <proto/dos.h>

#undef GLOBAL
/* my version of BADDR() has no problems with casting */
#undef	BADDR
#define BADDR(x)        ((APTR)((long)x << 2))
#define MKBADDR(x)      ((BPTR)((long)x >> 2))

long sendpkt(struct MsgPort *, long, long*, long);

void main(int, char **);

void
main(argc, argv)
int argc;
char **argv;
{
struct MsgPort *proc;
long myargs[8];

if (argc != 2)
   {
   printf("Usage: %s <device>\n", argv[0]);
   return;
   }

myargs[0]=1;

if ((proc = (struct MsgPort *)DeviceProc(argv[1])) == NULL)
   {
   printf("Unable to get a device proc for %s\n", argv[1]);
   return;
   }

sendpkt(proc,ACTION_DIE,myargs,1);
}

LONG sendpkt(pid,action,args,nargs)
struct MsgPort *pid;  /* process indentifier ... (handlers message port ) */
LONG action,	      /* packet type ... (what you want handler to do )   */
     args[],	      /* a pointer to a argument list */
     nargs;	      /* number of arguments in list  */
   {
   struct MsgPort	 *replyport;
   struct StandardPacket *packet;

   LONG  count, *pargs, res1;

   replyport = (struct MsgPort *) CreatePort(NULL,0);
   if(!replyport) return(NULL);

   packet = (struct StandardPacket *)
      AllocMem((long)sizeof(struct StandardPacket),MEMF_PUBLIC|MEMF_CLEAR);
   if(!packet)
      {
      DeletePort(replyport);
      return(NULL);
      }

   packet->sp_Msg.mn_Node.ln_Name = (char *)&(packet->sp_Pkt);
   packet->sp_Pkt.dp_Link	  = &(packet->sp_Msg);
   packet->sp_Pkt.dp_Port	  = replyport;
   packet->sp_Pkt.dp_Type	  = action;

   /* copy the args into the packet */
   pargs = &(packet->sp_Pkt.dp_Arg1);       /* address of first argument */
   for(count=0;count < nargs;count++)
      pargs[count]=args[count];

   PutMsg(pid,(struct Message *)packet); /* send packet */

   WaitPort(replyport);
   GetMsg(replyport);

   res1 = packet->sp_Pkt.dp_Res1;

   FreeMem((char *)packet,(long)sizeof(struct StandardPacket));
   DeletePort(replyport);

   return(res1);
}
