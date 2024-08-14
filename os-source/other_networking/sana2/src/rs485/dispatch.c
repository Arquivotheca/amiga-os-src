/*
 * Command dispatch - BeginIO/AbortIO
 */

#ifndef RS485_H
#include "rs485.h"
#endif

/*
 * none of the following commands are implemented: Arcnet does not allow
 * the notion of a promiscuous listener thus station aliasing isn't possible.
 * Along the same lines, true multicasting isn't possible without using
 * the broadcast mechanism - which may break listeners who are not prepared
 * to participate properly.
 * Sana devices do not support most old CMD_xxx calls.
 */
#define S2_notimplemented	0
#define	S2_addmulticastaddress	S2_notimplemented
#define	S2_delmulticastaddress	S2_notimplemented
#define	S2_multicast		S2_notimplemented
#define	S2_getspecialstats	S2_notimplemented
#define CMD_reset		S2_notimplemented
#define CMD_invalid		S2_notimplemented
#define CMD_update		S2_notimplemented
#define CMD_clear		S2_notimplemented
#define CMD_stop		S2_notimplemented
#define CMD_start		S2_notimplemented
#define CMD_flush		S2_notimplemented

/*
 * All read/write packet routines go through common processing function
 */
#define S2_broadcast		CMD_rdwr
#define S2_readorphan		CMD_rdwr
#define CMD_read		CMD_rdwr
#define CMD_write		CMD_rdwr

/*
 * protocol stat commands are so similar, they are implemented in one
 * function call
 */
#define	S2_tracktype		S2_tracked
#define	S2_untracktype		S2_tracked
#define	S2_gettypestats		S2_tracked

/*
 * command dispatch table
 */
static int (*cmdtab[S2_END])(struct IOSana2Req *iob) = {
	/* Command name			 blocking?	queue?		Command	*/
	/* ------------			 ---------	------		-------	*/
	CMD_invalid,			/* -notimp-			0	*/
	CMD_reset,			/* -notimp-			1	*/
	CMD_read,			/* can block 	read_q  	2	*/
	CMD_write,			/* can block 	write_q		3	*/
	CMD_update,			/* -notimp-			4	*/
	CMD_clear,			/* -notimp-			5	*/
	CMD_stop,			/* -notimp-			6	*/
	CMD_start,			/* -notimp-			7	*/
	CMD_flush,			/* -notimp-			8	*/
	S2_devicequery,			/* immediate 			9	*/
	S2_getstationaddress,		/* can block 	misc_q		10	*/
	S2_configinterface,		/* immediate 		  	11	*/
	S2_addmulticastaddress,		/* -notimp-			8	*/
	S2_delmulticastaddress,		/* -notimp-			8	*/
	S2_addmulticastaddress,		/* -notimp-			14	*/
	S2_delmulticastaddress,		/* -notimp-			15	*/
	S2_multicast,			/* -notimp-			16	*/
	S2_broadcast,			/* can block 	write_q 	17	*/
	S2_tracktype,			/* immediate 			18	*/
	S2_untracktype,			/* immediate 			19	*/
	S2_gettypestats,		/* immediate 			20	*/
	S2_getspecialstats,		/* -notimp- 			21	*/
	S2_getglobalstats,		/* immediate 			22	*/
	S2_onevent,			/* can block 	misc_q		23	*/
	S2_readorphan,			/* can block 	read_q		24	*/
	S2_online,			/* immediate 			25	*/
	S2_offline			/* immediate 			26	*/
};

/*
 * BeginIO(iob, device) - start I/O on a particular unit.
 * a few brief sanity checks on message: check command code is in range,
 * check unit pointer: must exist on the rs485Units list.
 *
 * All commands are called from within a semaphore region; this saves a lot of
 * forbid/permit calls within the code.  The semaphore also protects the stateful
 * portions of the chip (eg autoinc address pointer) from inadvertent modification.
 */
ULONG __saveds __stdargs __asm
LIBBeginIO( register __a1 struct IOSana2Req *iob, register __a6 struct rs485Device *device )
{
struct rs485Unit *up;
int rtn;

	rtn = COMPLETE;
//	iob->ios2_WireError = iob->ios2_Req.io_Error = 0;
	iob->ios2_Req.io_Message.mn_Node.ln_Type = NT_MESSAGE;

	FORALL(&rs485Units, up, struct rs485Unit *){
		if(up == UNIT(iob)){
			break;
		}
	}
	if(up != UNIT(iob)){
		iob->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
		if(!UNIT(iob)){
			iob->ios2_WireError = S2WERR_NULL_POINTER;
		}
		goto reply;
	}

	if(iob->ios2_Req.io_Command>=S2_END || cmdtab[iob->ios2_Req.io_Command]==S2_notimplemented){
		iob->ios2_Req.io_Error = S2ERR_NOT_SUPPORTED;
		goto reply;
	}

	ObtainSemaphore(&up->sem);
	rtn = (*cmdtab[iob->ios2_Req.io_Command])(iob);
	ReleaseSemaphore(&up->sem);

reply:	if(rtn == COMPLETE){
		if(!(iob->ios2_Req.io_Flags & SANA2IOF_QUICK)){
			ReplyMsg(&iob->ios2_Req.io_Message);
		}
	}
	return 0;
}

/*
 * AbortIO(iob, device) - abort pending I/O on particular unit.  The driver is
 * designed such that if the i/o req is on a queue, it can be aborted without
 * further consideration.
 */
ULONG __saveds __stdargs __asm
LIBAbortIO( register __a1 struct IOSana2Req *iob, register __a6 struct rs485Device *device )
{
struct IOSana2Req *piob;
struct rs485Unit *up;
struct List *lp;
ULONG error;

	FORALL(&rs485Units, up, struct rs485Unit *){
		if(up == UNIT(iob)){
			break;
		}
	}
	if(up != UNIT(iob)){
		return 0; 	/* not clear what ios2_Req.io_Error to do here */
	}

	switch(iob->ios2_Req.io_Command){
	case CMD_READ:
	case S2_READORPHAN:
		lp = &up->read_q;
		break;

	case CMD_WRITE:
	case S2_BROADCAST:
		lp = &up->write_q;
		break;

	default:
		lp = &up->misc_q;
	}

	error = 0;
	ObtainSemaphore(&up->sem);
	FORALL(lp, piob, struct IOSana2Req *){
		if(iob == piob){
			iob->ios2_Req.io_Error = error = IOERR_ABORTED;
			Remove(&iob->ios2_Req.io_Message.mn_Node);
			ReplyMsg(&iob->ios2_Req.io_Message);
			break;
		}
	}
	ReleaseSemaphore(&up->sem);

	return error;
}

/*
 * Null routine just returns 0
 */
ULONG __saveds __stdargs
_LibNull()
{
	return 0L;
}

