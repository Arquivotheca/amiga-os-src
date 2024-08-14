#include <exec/types.h>
#include <devices/timer.h>
#include <dos/dos.h>

#include <rpc/rpc.h>
#include <nfs/nfs.h>

#include "nfsd.h"
#include <clib/timer_protos.h>

static struct timerequest t;
static int off_min;

nfsstat init_time()
{

	t.tr_node.io_Message.mn_ReplyPort = CreateMsgPort();
	if(t.tr_node.io_Message.mn_ReplyPort == 0){
		return NFSERR_NOSPC;
	}

	if (OpenDevice(TIMERNAME, UNIT_VBLANK, &t, 0L)) {
		return NFSERR_NOENT;
	}

	off_min = get_tz();

	return NFS_OK;
}

void clean_time()
{
	if(t.tr_node.io_Message.mn_ReplyPort){
		CloseDevice(&t);
		DeleteMsgPort(t.tr_node.io_Message.mn_ReplyPort);
		t.tr_node.io_Message.mn_ReplyPort = 0;
	}
}

/* Unix dates are relative to Jan 1st, 1970,
 * while AmigaDOS dates are relative to Jan 1st, 1978; must translate by
 * difference in seconds.  Date returned for file is modification time.
 */
#define MIN_PER_DAY	(24*60)
#define SEC_PER_DAY	(MIN_PER_DAY*60)
#define UNIX_DELTAT	(((1978-1970)*365 + 2 /* 2 leap years */)*SEC_PER_DAY)

nfsstat NFStime(nfstime *tp)
{
	if(t.tr_node.io_Message.mn_ReplyPort == 0){
		return NFSERR_NOSPC;
	}

	t.tr_node.io_Command = TR_GETSYSTIME;
	DoIO(&t);
	tp->seconds = t.tr_time.tv_secs + 60*off_min + UNIX_DELTAT;
	tp->useconds = t.tr_time.tv_micro;

	return NFS_OK;
}

void nfs_to_amigatime(nfstime *ns, struct DateStamp *ds)
{
	register long amigasec, amigamin;

	amigasec = ns->seconds - UNIX_DELTAT - 60*off_min;
	amigamin = amigasec/60;

	ds->ds_Days = amigamin / 24*60;
	ds->ds_Minute = amigamin % 60;
	ds->ds_Tick = (amigasec % 60) * TICKS_PER_SECOND;
}

void amiga_to_nfstime(struct DateStamp *ds, nfstime *ns)
{
	ns->seconds = (ds->ds_Days*24*60 + ds->ds_Minute)*60 + UNIX_DELTAT;
	ns->seconds += ds->ds_Tick/TICKS_PER_SECOND + 60*off_min;
	ns->useconds = 0;
}
