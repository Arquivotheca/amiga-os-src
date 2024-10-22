/* misc.c ... Handler support routines */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/io.h>
#include <devices/serial.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

#include "handler.h"
 
/* returnpkt() ... returns packet to sender */
/* This is modeled just like a BCPL routine so its a little redundant */

VOID 
returnpkt(packet,res1,res2)

struct DosPacket *packet;
ULONG  res1,res2;
{
 struct Message *message;
 struct MsgPort *replyport;
 
 packet->dp_Res1 = res1;
 packet->dp_Res2 = res2; 
 replyport = packet->dp_Port;
 message = packet->dp_Link;
 packet->dp_Port = &((struct Process *) FindTask(0))->pr_MsgPort;
 message->mn_Node.ln_Name    = (char *) packet;
 message->mn_Node.ln_Succ    = NULL;
 message->mn_Node.ln_Pred    = NULL;
 
 PutMsg(replyport,message);

}


/* taskwait() ... Waits for a message to arrive at your port and *
 *   extracts the packet address which is returned to you.       */

struct DosPacket *taskwait()
{
 struct Process *process;
 struct MsgPort *port;
 struct Message *message;

 process = (struct Process *) FindTask(0);
 port = &process->pr_MsgPort;

 WaitPort(port); /* wait for packet */
 message = (struct Message *) GetMsg(port);

/* give them the pointer to the packet */
return((struct DosPacket *) message->mn_Node.ln_Name);
} 


/* GetProcess() will take a taskid (&pr_MsgPort) and return that process */

struct Process *GetProcess(taskid)

struct MsgPort *taskid;
{
  return((struct Process *) ((ULONG)taskid - (ULONG)sizeof(struct Task)));  
}
