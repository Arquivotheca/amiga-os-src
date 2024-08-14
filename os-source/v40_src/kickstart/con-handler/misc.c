/* misc.c ... Miscellaneous support routines */

#include "con-handler.h"
#include "devices/timer.h"

/* AmigaDOS uses task signal bit 8 for message signaling */
#define DOS_SIGNAL              8
#define DOS_MASK                ( 1 << DOS_SIGNAL )



/* append() will add a packet to a singlely linked list of packets
 * it will use the dp_Link for next pointer and save the
 * previous value of dp_Link in the dp_Res1 field
 * Arguments:
 *  plist - is the address of a pointer to a DosPacket. Initially
 *          set to zero
 *  pkt   - the packet to add to the list.
 *
 * Example:
 *  struct DosPacket *pkts=0,*pkt;
 *
 *  pkt = taskwait();
 *  append(&pkts,pkt);
 *
 */

VOID __regargs append(plist,pkt)
struct DosPacket **plist;
struct DosPacket *pkt;
{
 register struct DosPacket *cpkt;

 if(!(cpkt = *plist))*plist = pkt;
 else {
   for(;cpkt->dp_Link;cpkt=(struct DosPacket *)cpkt->dp_Link);
   cpkt->dp_Link=(struct Message *)pkt;
 }
 pkt->dp_Res1 = (long) pkt->dp_Link;
 pkt->dp_Link = NULL;
}


VOID __regargs dSendIO(iob,packet,command,buffer,length)
struct IOStdReq *iob;
struct DosPacket *packet;
LONG command;
APTR buffer;
LONG length;
{
   iob->io_Command = command;
   iob->io_Length = length;
   /* special for timer , because timer uses a short IOR */
   iob->io_Data = (APTR)buffer;
   if(buffer)iob->io_Offset = 0;

   iob->io_Message.mn_Node.ln_Name = (UBYTE *)packet;
   packet->dp_Link = (struct Message *)iob;
   SendIO(iob);
}


void __regargs StartPaste(gv)
struct Global *gv;
{
    struct MsgPort *pasteport, *cutport;
    struct cmdMsg cm={0};

    gv->pcount=0;
    pasteport=CreateMsgPort();
    cm.cm_Msg.mn_Node.ln_Type=0;
    cm.cm_Msg.mn_Length=(long)sizeof(struct cmdMsg);
    cm.cm_Msg.mn_ReplyPort=pasteport;
    cm.cm_type=TYPE_READ;

    Forbid();
    if(cutport=FindPort("ConClip.rendezvous")) {
	PutMsg(cutport,(struct Message *)&cm);
	Permit();
	WaitPort(pasteport);
	if(!cm.cm_error && cm.cm_type==TYPE_REPLY) {
	    struct MinNode *node;
	    /* pull paste node off the conclip list and add them to ours */
	    node=RemHead((struct List *)&cm.cm_Chunks);
	    while (node) {
		AddTail((struct List *)&gv->paste,(struct Node *)node);
		node=RemHead((struct List *)&cm.cm_Chunks);	    
	    }
	    /* start the paste by connecting the first item to the port */
	    gv->node=(struct MinNode *)RemHead((struct List *)&gv->paste);
	}
    }
    else Permit();
    DeleteMsgPort(pasteport);
}

void __regargs KillPaste(gv)
struct Global *gv;
{
    while(gv->node) {
	if( ((struct CHRSChunk *)gv->node)->freeme)
		FreeVec(((struct CHRSChunk *)gv->node)->freeme);
	FreeVec(gv->node);
	gv->node=(struct MinNode *)RemHead((struct List *)&gv->paste);
    }
    gv->pcount=0;
}

UBYTE __regargs GetPaste(gv,flag)
struct Global *gv;
int flag; /* indicates whether to get just one line */
{
    UBYTE ch;

    ch=((struct CHRSChunk *)gv->node)->text[gv->pcount++];
    if(gv->pcount>((struct CHRSChunk *)gv->node)->size) {
	if( ((struct CHRSChunk *)gv->node)->freeme)
		FreeVec(((struct CHRSChunk *)gv->node)->freeme);
	FreeVec(gv->node);
	/* normally, keep going */
	if(!flag)gv->node=(struct MinNode *)RemHead((struct List *)&gv->paste);
	else gv->node = 0;
	gv->pcount=0;
    }
    return(ch);
}

int __regargs PutPaste(gv,action,string,len)
struct Global *gv;
int action;
UBYTE *string;
LONG len;
{
    struct CHRSChunk *chrs;

    if(!len)return(-1);	/* nothing to do */
    chrs=(struct CHRSChunk *)AllocVec(len+1+sizeof(struct CHRSChunk),
		MEMF_PUBLIC|MEMF_CLEAR);
    if(chrs) {
	memcpy(chrs->data,string,len);
	chrs->size = len;
	chrs->text=&chrs->data[0];
	chrs->freeme=0;
	if(action == ACTION_QUEUE) {
	    AddTail((struct List *)&gv->paste,(struct Node *)&chrs->mn);
	}
	else {
	    AddHead((struct List *)&gv->paste,(struct Node *)&chrs->mn);

	}
	/* start things off unless there's a paste in progress */
	if((action == ACTION_FORCE) && (!gv->node)) {
	    gv->node=(struct MinNode *)RemHead((struct List *)&gv->paste);
	    gv->pcount=0;
	}
	return(len);
    }
    return(-1);
}
