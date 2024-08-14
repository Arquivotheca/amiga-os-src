/* Copyright (C) 1990 by Commodore-Amiga */
#include <sys/time.h>
#include <config.h>
#include <exec/types.h>
#include <exec/lists.h>
#include <devices/timer.h>

#ifdef LATTICE
#include <proto/exec.h>
#endif

gettimeofday(tp, tzp)
	struct timeval *tp;
	struct timezone	*tzp;
{
	struct MsgPort *CreatePort();
	static struct timerequest t;	/* made static MMH */
	static struct timezone tz;
	struct config *cf;

	if (OpenDevice(TIMERNAME, UNIT_VBLANK, &t, 0L)) {
		return -1;
	}

	t.tr_node.io_Message.mn_ReplyPort = CreatePort(0L, 0L);
	t.tr_node.io_Command = TR_GETSYSTIME;
	DoIO((struct IORequest *)&t);
	DeletePort(t.tr_node.io_Message.mn_ReplyPort);
	CloseDevice(&t);
	t.tr_time.tv_secs += 252460800;	/* correct to 1 Jan 70 */
	*tp =  t.tr_time;

	GETCONFIG(cf);
	if (!cf) return -1;
	tz.tz_minuteswest = cf->tz_offset;
	tz.tz_dsttime = 0;	/* ignore daylight savings time for now */
	t.tr_time.tv_secs += cf->tz_offset*60;	/* correct for timezone */

	return(0);
}

