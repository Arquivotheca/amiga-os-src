************************************************************************
* SendPacket.c - send a DOS packet to a speicified port                *
* Written March 1988 by Talin                                          *
************************************************************************

*	Note that sp_Msg is at offset 0

			section	sendpacket.asm,code

			include	'lat_macros.i'
			include 'libraries/dosextens.i'

			DECLAREA SendPacket

			move.l	4(sp),a0
			move.l	8(sp),a1
SendPacket
			movem.l	a2/a6,-(sp)
			move.l	a1,-(sp)
			move.l	MN_REPLYPORT(a1),a2
			move.l	4,a6
			Call	PutMsg
			move.l	(sp),a1
			move.l	a2,a0
			Call	WaitPort
			move.l	(sp)+,a1
			Call	Remove
			movem.l	(sp)+,a2/a6
			rts

			end

#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <libraries/filehandler.h>
#include <macros.h>

SendPacket (port, packet) struct MsgPort *port; struct StandardPacket *packet;
{	PutMsg(port, &packet->sp_Msg);
	WaitPort(packet->sp_Msg.mn_ReplyPort);
	Remove(&packet->sp_Msg);
}
