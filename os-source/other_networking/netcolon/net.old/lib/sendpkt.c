
/*
 *	SENDPKT.C
 *
 *	From: ConPackets.c -  C. Scheppner, A. Finkel, P. Lindsay  CBM
 *	DOS packet example
 *	Requires 1.2
 *
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>

LONG sendpkt(struct MsgPort *, LONG, LONG *, LONG);

LONG
sendpkt(pid,action,args,nargs)
struct MsgPort *pid;	/* process indentifier ... (handlers message port ) */
LONG action;		/* packet type ... (what you want handler to do )   */
LONG args[];		/* a pointer to a argument list */
LONG nargs;		/* number of arguments in list	*/
{
    struct MsgPort	  *replyport;
    struct StandardPacket *packet;

    LONG  count, *pargs, res1;

    replyport = (struct MsgPort *) CreatePort(NULL,0);
    if (!replyport)
	return(NULL);

    packet = (struct StandardPacket *)
    AllocMem((long)sizeof(struct StandardPacket),MEMF_PUBLIC|MEMF_CLEAR);
    if (!packet) {
	DeletePort(replyport);
	return(NULL);
    }

    packet->sp_Msg.mn_Node.ln_Name = (char *)&(packet->sp_Pkt);
    packet->sp_Pkt.dp_Link	   = &(packet->sp_Msg);
    packet->sp_Pkt.dp_Port	   = replyport;
    packet->sp_Pkt.dp_Type	   = action;

    /* copy the args into the packet */

    pargs = &(packet->sp_Pkt.dp_Arg1);       /* address of first argument */
    for (count = 0; count < nargs; count++)
	pargs[count] = args[count];

    PutMsg(pid,(struct Message *)packet); /* send packet */

    WaitPort(replyport);
    GetMsg(replyport);

    res1 = packet->sp_Pkt.dp_Res1;

    FreeMem((char *)packet,(long)sizeof(struct StandardPacket));
    DeletePort(replyport);

    return(res1);
}


