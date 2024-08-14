***********************************************************************
* InitPacket.asm - initialize a DOS packet                            *
* Written March 1988 by Talin, in assembly March 1989 JP              *
***********************************************************************

* InitPacket(action,packet,port) (d0,a0,a1)

			section	code

			include 'macros.i'
			include 'exec/lists.i'
			include 'exec/ports.i'
			include 'libraries/dosextens.i'

			DECLAREA InitPacket

			move.l	4(sp),d0
			move.l	8(sp),a0
			move.l	12(sp),a1
InitPacket
			move.l	a2,-(sp)
			move.b	#NT_MESSAGE,sp_Msg+LN_TYPE(a0)
			lea		sp_Pkt(a0),a2
			move.l	a2,sp_Msg+LN_NAME(a0)
			move.l	a1,sp_Msg+MN_REPLYPORT(a0)
			move.l	a1,sp_Pkt+dp_Port(a0)
			lea		sp_Msg(a0),a2
			move.l	a2,sp_Pkt+dp_Link(a0)
			move.l	d0,sp_Pkt+dp_Type(a0)
			move.l	(sp)+,a2
			rts

			end

#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <libraries/filehandler.h>
#include <macros.h>

InitPacket (action, packet, port)
	ULONG action; struct StandardPacket *packet; struct MsgPort *port;
{
	packet->sp_Msg.mn_Node.ln_Type	= NT_MESSAGE;
	packet->sp_Msg.mn_Node.ln_Name 	= (char *) &packet->sp_Pkt;
	packet->sp_Msg.mn_ReplyPort		= port;
	packet->sp_Pkt.dp_Port			= port;
	packet->sp_Pkt.dp_Link			= &packet->sp_Msg;
	packet->sp_Pkt.dp_Type			= action;
}
