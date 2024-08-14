/*
 *      Sendpacket.c 
 *
 *  An invaluable addition to your Amiga.lib file. This code sends a packet
 * the given message port. This makes working around DOS lots easier.
 * 
 * Note, I didn't write this, those wonderful folks at CBM did. I do suggest
 * however that you may wish to add it to Amiga.Lib, to do so, compile it
 * and say 'oml lib:amiga.lib -r sendpacket.o' 
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
 
/*
 * Function - SendPacket written by Phil Lindsay, Carolyn Scheppner, and
 * Andy Finkel. This function will send a packet of the given type to the
 * Message Port supplied.
 */
 
long
SendPacket(pid,action,args,nargs)
 
struct MsgPort *pid;  /* process indentifier ... (handlers message port ) */
long action,          /* packet type ... (what you want handler to do )   */
     args[],          /* a pointer to a argument list */
     nargs;           /* number of arguments in list  */
{
  struct MsgPort        *replyport;
  struct StandardPacket *packet;
 
  long  count, *pargs, res1;
 
  replyport = (struct MsgPort *) CreatePort(NULL,0);
  if(!replyport) return(0);
 
  /* Allocate space for a packet, make it public and clear it */
  packet = (struct StandardPacket *) 
    AllocMem((long)sizeof(struct StandardPacket),MEMF_PUBLIC|MEMF_CLEAR);
  if(!packet) {
    DeletePort(replyport);
    return(0);
  }
 
  packet->sp_Msg.mn_Node.ln_Name = (char *)&(packet->sp_Pkt);
  packet->sp_Pkt.dp_Link         = &(packet->sp_Msg);
  packet->sp_Pkt.dp_Port         = replyport;
  packet->sp_Pkt.dp_Type         = action;
 
  /* copy the args into the packet */
  pargs = &(packet->sp_Pkt.dp_Arg1);       /* address of first argument */
  for(count=0;count < nargs;count++) 
    pargs[count]=args[count];
 
  PutMsg(pid,packet); /* send packet */

  WaitPort(replyport);
  GetMsg(replyport); 
 
  res1 = packet->sp_Pkt.dp_Res1;
 
  FreeMem(packet,(long)sizeof(struct StandardPacket));
  DeletePort(replyport); 

  return(res1);
}
