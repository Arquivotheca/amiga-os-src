/*
 *  DNETLIB.C
 *
 *  DNET (c)Copyright 1988, Matthew Dillon, All Rights Reserved.
 *
 *  Library Interface for DNET.
 */

#define NOEXT
#define NETCOMMON
#include "dnet.h"
#include "netcomm.h"
#include "proto.h"

#include <stdio.h>

static struct DChannel *MakeChannel U_ARGS((IOR *, char *));
static void DeleteChannel U_ARGS((struct DChannel *));
static void FixSignal	  U_ARGS((struct DChannel *));

#define NAMELEN sizeof("DNET.PORT.XXXXX")
#define NAMEPAT "DNET.PORT.%ld"

PORT *DListen(portnum)
uword portnum;
{
    PORT *port;
    char *ptr;

    port = NULL;
    ptr = AllocMem(NAMELEN, MEMF_PUBLIC);   /*  memory the the name     */
    sprintf(ptr, NAMEPAT, portnum);
    Forbid();                               /*  task-atomic operation   */
    if (FindPort(ptr) || !(port = CreatePort(ptr,0)))
       FreeMem(ptr, NAMELEN);
    Permit();
    return(port);
}

void
DUnListen(lisport)
PORT *lisport;
{
   register char *ptr;
   ptr = lisport->mp_Node.ln_Name;

   if (lisport)
   {
      Forbid();                       /*  task-atomic operation       */
      while (DNAAccept(lisport));     /*  remove all pending requests */
      DeletePort(lisport);            /*  gone!                       */
      Permit();
      FreeMem(ptr, NAMELEN);
   }
}

/*
 *  DAccept()
 *
 *  Note:   This call will work even if called by a task which does not
 *	    own the listen port.
 */

PORT *
DAccept(lisport)
PORT *lisport;
{
    register IOR *ior;
    register struct DChannel * chan;

BUG(("DAccept: Entry, port %lx\n", lisport))

    chan = NULL;
    while (!chan && (ior = (IOR *)GetMsg(lisport))) {
       switch(ior->io_Command) {
	  case DNCMD_SOPEN:
	     BUG(("DAccept: SOPEN command\n"))
	     chan = MakeChannel(ior, NULL);
	     break;
	  default:
	     BUG(("DAccept: Unrecognized command '%d'\n", ior->io_Command))
	     ior->io_Error = 1;
	     break;
	}
	BUG(("DAccept: Replying\n"))
	ReplyMsg(&ior->io_Message);
    }
    BUG(("DAccept: After while loop\n"))
    if (lisport->mp_MsgList.lh_Head != (NODE *)&lisport->mp_MsgList.lh_Tail)
       SetSignal(1 << lisport->mp_SigBit, 1 << lisport->mp_SigBit);

    return(chan ? &chan->port : NULL);
}

/*
 *  Refuse a connection
 */

DNAAccept(lisport)
PORT *lisport;
{
    IOR *ior;

    if (ior = (IOR *)GetMsg(lisport)) {
	ior->io_Error = 1;
	ReplyMsg(&ior->io_Message);
    }
    if (lisport->mp_MsgList.lh_Head != (NODE *)&lisport->mp_MsgList.lh_Tail)
       SetSignal(1 << lisport->mp_SigBit, 1 << lisport->mp_SigBit);
    return(ior != NULL);
}

void DPri(chan, pri)
struct DChannel * chan;
int pri;
{
}


PORT *
DOpen(host, portnum, txpri, rxpri)
char *host;
char txpri, rxpri;
uword portnum;
{
    IOR ior;
    struct DChannel *chan;

    if (!host)
       host = "0";
    chan = MakeChannel(&ior, host);
    if (rxpri > 126)
	rxpri = 126;
    if (rxpri < -127)
	rxpri = -127;
    if (txpri > 126)
	txpri = 126;
    if (txpri < -127)
	txpri = -127;
    if (chan->dnetport) {
	ior.io_Command = DNCMD_OPEN;
	ior.io_Unit = (void *)portnum;
	ior.io_Offset = (long)chan;
	ior.io_Message.mn_ReplyPort = &chan->port;
	ior.io_Message.mn_Node.ln_Pri = txpri;
	ior.io_Message.mn_Node.ln_Name= (char *)rxpri;

	PutMsg(chan->dnetport, &ior.io_Message);
	WaitMsg(&ior);
	if (ior.io_Error == 0) {
	   chan->chan = (long)ior.io_Unit;
	   FixSignal(chan);
	   return(&chan->port);
       }
    }
    DeleteChannel(chan);
    return(NULL);
}


DNRead(chan, buf, bytes)
struct DChannel * chan;
char *buf;
int bytes;
{
    register IOR *ior;
    int len;
    long n;

    len = 0;
    if (chan->eof)
       return(-1);
    while (bytes && ((ior = (IOR *)
	       RemHead((struct List *)&chan->rdylist)) ||
	       (ior = (IOR *)GetMsg(&chan->port)))) {
       if (ior->io_Message.mn_Node.ln_Type == NT_REPLYMSG)
       {
	  if (!chan->queued)
	  {
	     BUG(("DNRead: Software Error"));
	  }
	  else
	     --chan->queued;
	  if (ior->io_Length)
	     FreeMem((char *)ior->io_Data, (long)ior->io_Length);
	 FreeMem((char *)ior, (long)sizeof(IOR));
	 continue;
       }
      switch(ior->io_Command)
      {
	 case DNCMD_CLOSE:
	 case DNCMD_EOF:
	    chan->eof = 1;
	    ReplyMsg(&ior->io_Message);
	    break;

	 case DNCMD_WRITE:
	    n = ior->io_Length - ior->io_Actual;
	    if (n <= bytes)
	    {
	       memcpy(buf, ((char *)ior->io_Data) + ior->io_Actual, n);
	       bytes -= n;
	       len += n;
	       buf += n;
	       ReplyMsg(&ior->io_Message);
	    }
	    else
	    {
	       memcpy(buf, (char *)ior->io_Data + ior->io_Actual, bytes);
	       len += bytes;
	       ior->io_Actual += bytes;
	       bytes = 0;
	       Forbid();   /*  DNET device is a task, no need to Disable() */
	       ior->io_Message.mn_Node.ln_Type = NT_MESSAGE;
	       AddHead(&chan->port.mp_MsgList, (struct Node *)ior);
	       Permit();
	    }
	    break;
	 default:
	    ior->io_Error = 1;
	    ReplyMsg(&ior->io_Message);
      }
    }
    FixSignal(chan);
    if (chan->eof)
       SetSignal(1 << chan->port.mp_SigBit, 1 << chan->port.mp_SigBit);
    return(len);
}

int DRead(chan, buf, bytes)
char *buf;
struct DChannel * chan;
int bytes;
{
   long len;
   long n;

   len = 0;
   if (chan->eof)
   {
      BUG(("****DNET EOF!!!\n"));
      return(-1);
   }

   while (bytes)
   {
      WaitPort(&chan->port);
      n = DNRead(chan, buf, bytes);
      len += n;
      if (n < 0) break;
      buf += n;
      bytes -= n;
      if (chan->eof) break;
   }
   return(len);
}

void DQueue(chan, n)
struct DChannel * chan;
int n;
{
    chan->qlen = n;
}

DWrite(chan, buf, bytes)
struct DChannel * chan;
char *buf;
int bytes;
{
    IOR tmpior;
    IOR *ior;
    int error;

   error = bytes;
   if (chan->qlen)
   {
      if (WaitQueue(chan, NULL) >= 0)
      {
	 ior = (IOR *)AllocMem(sizeof(IOR), MEMF_CLEAR|MEMF_PUBLIC);
	 ior->io_Command = DNCMD_WRITE;
	 ior->io_Unit = (void *)chan->chan;
	 ior->io_Offset = (long)chan;
	 ior->io_Message.mn_ReplyPort = &chan->port;
	 ior->io_Data = (APTR)AllocMem(bytes, MEMF_PUBLIC);
	 ior->io_Length = bytes;
	 memcpy((char *)ior->io_Data, buf, (int)bytes);
	 PutMsg(chan->dnetport, &ior->io_Message);
	 ++chan->queued;
      }
      else
      {
	 error = -1;
      }
   }
   else
   {
      tmpior.io_Command = DNCMD_WRITE;
      tmpior.io_Unit = (void *)chan->chan;
      tmpior.io_Offset = (long)chan;
      tmpior.io_Message.mn_ReplyPort = &chan->port;
      tmpior.io_Data = (APTR)buf;
      tmpior.io_Length = bytes;
      PutMsg(chan->dnetport, &tmpior.io_Message);
      WaitMsg(&tmpior);
      if (tmpior.io_Error)
      {
	 error = -1;
	 BUG(("*****DWrite: io_Error %d\n", tmpior.io_Error));
	 BUGGETC
      }
   }
   FixSignal(chan);
   return(error);
}

void DEof(chan)
struct DChannel * chan;
{
    IOR ior;

    ior.io_Command = DNCMD_EOF;
    ior.io_Unit = (void *)chan->chan;
    ior.io_Offset = (long)chan;
    ior.io_Message.mn_ReplyPort = &chan->port;
    PutMsg(chan->dnetport, &ior.io_Message);
    WaitMsg(&ior);
    FixSignal(chan);
}

void DIoctl(chan, cmd, val, aux)
struct DChannel * chan;
ubyte cmd;
uword val;
ubyte aux;
{
    IOR ior;

    ior.io_Command = DNCMD_IOCTL;
    ior.io_Unit = (void *)chan->chan;
    ior.io_Offset = (long)chan;
    ior.io_Message.mn_ReplyPort = &chan->port;
    ior.io_Data = (APTR)(long)((val<<16)|(aux<<8)|cmd);
    PutMsg(chan->dnetport, &ior.io_Message);
    WaitMsg(&ior);
    FixSignal(chan);
}

int DQuit(host)
char *host;
{
    IOR ior;
    char buf[sizeof(DNETPORTNAME)+32];
    PORT *replyport;
    PORT *dnetport;

    if (!host)
	host = "0";
    sprintf(buf, "%s%s", DNETPORTNAME, host);
    if (dnetport = FindPort(buf)) {
	replyport = CreatePort(NULL, 0);
	ior.io_Command = DNCMD_QUIT;
	ior.io_Unit = 0;
	ior.io_Offset = 0;
	ior.io_Message.mn_ReplyPort = replyport;
	PutMsg(dnetport, &ior.io_Message);
	WaitMsg(&ior);
	DeletePort(replyport);
    }
    return(dnetport != NULL);
}


void DClose(chan)
struct DChannel * chan;
{
    IOR ior;
    IOR *io;

BUG(("DClose: Enter\n"))

    ior.io_Command = DNCMD_CLOSE;
    ior.io_Unit = (void *)chan->chan;
    ior.io_Offset = (long)chan;
    ior.io_Message.mn_ReplyPort = &chan->port;
    PutMsg(chan->dnetport, &ior.io_Message);
    ++chan->queued;
    chan->qlen = 0;
    WaitQueue(chan, &ior);
    while ((io = (IOR *)RemHead((struct List *)&chan->rdylist)) ||
	   (io = (IOR *)GetMsg(&chan->port))) {
	io->io_Error = 1;
	ReplyMsg(&io->io_Message);
    }
    DeleteChannel(chan);
}

void WaitMsg(ior)
IOR *ior;
{
    while (ior->io_Message.mn_Node.ln_Type != NT_REPLYMSG)
	Wait(1 << ior->io_Message.mn_ReplyPort->mp_SigBit);
    Forbid();
    Remove((struct Node *)ior);
    Permit();
}

int WaitQueue(chan, skipior)
struct DChannel * chan;
IOR *skipior;
{
    register IOR *io;
    short error;

   error = 0;
    while (chan->queued > chan->qlen) {     /*  until done  */
	WaitPort(&chan->port);   /*  something   */
	io = (IOR *)GetMsg(&chan->port);
	if (io->io_Message.mn_Node.ln_Type == NT_REPLYMSG) {
	    if (error == 0)
		error = io->io_Error;
	    if (io != skipior) {
		if (io->io_Length)
		    FreeMem((char *)io->io_Data, io->io_Length);
		FreeMem((char *)io, sizeof(IOR));
	    }
	    --chan->queued;
	} else {
	    AddTail(&chan->rdylist, (struct Node *)io);
	}
    }
    return((int)error);
}

static struct DChannel *MakeChannel(ior, host)
register IOR *ior;
char *host;
{
    struct DChannel * chan;
    char buf[sizeof(DNETPORTNAME)+32];

BUG(("MakeChannel: Entry\n"))

    chan = (struct DChannel *)AllocMem(sizeof(struct DChannel),
				       MEMF_PUBLIC|MEMF_CLEAR);

    /*	Name, Pri */
    chan->port.mp_Node.ln_Type = NT_MSGPORT;
    chan->port.mp_SigBit = AllocSignal(-1);
    chan->port.mp_SigTask = FindTask(NULL);
    NewList(&chan->port.mp_MsgList);
    NewList(&chan->rdylist);
    chan->chan = (long)ior->io_Unit;
    ior->io_Offset = (long)chan;
    if (host) {
       sprintf(buf, "%s%s", DNETPORTNAME, host);
	ior->io_Message.mn_ReplyPort = FindPort(buf);
    }
    chan->dnetport = ior->io_Message.mn_ReplyPort;
    return(chan);
}

static void
DeleteChannel(chan)
struct DChannel * chan;
{
    FreeSignal(chan->port.mp_SigBit);
    FreeMem((char *)chan, (long)sizeof(struct DChannel));
}

static void
FixSignal(chan)
register struct DChannel * chan;
{
    if (chan->port.mp_MsgList.lh_Head !=
	     (NODE *)&chan->port.mp_MsgList.lh_Tail ||
       chan->rdylist.lh_Head != (NODE *)&chan->rdylist.lh_Tail)
	SetSignal(1 << chan->port.mp_SigBit, 1 << chan->port.mp_SigBit);
}

