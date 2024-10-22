/* Copyright (C) 1986,1987 by Manx Software Systems, Inc. */

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#ifdef LATTICE
#include <proto/exec.h>
#endif

long
dos_packet(port, type, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
struct MsgPort *port;
long type, arg1, arg2, arg3, arg4, arg5, arg6, arg7;
{
	register struct StandardPacket *sp = NULL ;
	register struct MsgPort *rp, *CreatePort();
	struct Task *FindTask();
	struct Process *pr;
	void *AllocMem();
	long ret;

	if ((rp = CreatePort(0L, 0L)) == 0)
		return(0);
	if ((sp = AllocMem((long)sizeof(*sp), MEMF_PUBLIC|MEMF_CLEAR)) == 0) {
		DeletePort(rp);
		return(0);
	}
	sp->sp_Msg.mn_Node.ln_Name = (char *)&sp->sp_Pkt;
	sp->sp_Pkt.dp_Link = &sp->sp_Msg;
	sp->sp_Pkt.dp_Port = rp;
	sp->sp_Pkt.dp_Type = type;
	sp->sp_Pkt.dp_Arg1 = arg1;
	sp->sp_Pkt.dp_Arg2 = arg2;
	sp->sp_Pkt.dp_Arg3 = arg3;
	sp->sp_Pkt.dp_Arg4 = arg4;
	sp->sp_Pkt.dp_Arg5 = arg5;
	sp->sp_Pkt.dp_Arg6 = arg6;
	sp->sp_Pkt.dp_Arg7 = arg7;
	PutMsg(port, &sp->sp_Msg);
	WaitPort(rp);
	GetMsg(rp);
	ret = sp->sp_Pkt.dp_Res1;

	pr = (struct Process *)FindTask(0L);
	pr->pr_Result2 = sp->sp_Pkt.dp_Res2;

	FreeMem(sp, (long)sizeof(*sp));
	DeletePort(rp);
	return(ret);
}

