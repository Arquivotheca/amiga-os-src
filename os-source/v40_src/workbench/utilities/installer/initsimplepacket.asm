**************************************************************************
* InitSimplePacket.asm - make a DOS packet which returns to your process *                             *
* Written March 1988 by Talin, in assembly March 1989 JP                 *
**************************************************************************

			section	initsimplepacket.asm,code

			include 'macros.i'
			include 'libraries/dosextens.i'

			DECLAREA InitSimplePacket

			move.l	4(sp),d0
			move.l	8(sp),a0
InitSimplePacket
			movem.l	d0/a0/a6,-(sp)
			suba.l	a1,a1
			CallEx	FindTask
			move.l	d0,a1
			lea		pr_MsgPort(a1),a1
			movem.l	(sp)+,d0/a0/a6
			GoSub	InitPacket
			rts

			end

#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <libraries/filehandler.h>
#include <macros.h>

InitSimplePacket (action, packet) ULONG action; struct StandardPacket *packet;
{	struct Process *FindTask(), *my_proc = FindTask(0);

	InitPacket(action, packet, &(my_proc->pr_MsgPort));
}

