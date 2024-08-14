/* -----------------------------------------------------------------------
 * main.c   nfsc  SAS
 *
 * $Locker:  $
 *
 * $Id: main.c,v 1.6 92/10/07 14:03:50 bj Exp $
 *
 * $Revision: 1.6 $
 *
 * $Header: Hog:Other/inet/src/c/nfsc/RCS/main.c,v 1.6 92/10/07 14:03:50 bj Exp $
 *
 *------------------------------------------------------------------------
 */

//#define MONITOR

/*
** Main handler body
*/

#include "fs.h"
#include <exec/ports.h>

#include <sys/param.h>
#include <sys/time.h>

#include <stdio.h>

#include "nfsc_rev.h"

static char *nfscvers = VERSTAG ;

struct mount_pt *nfs_mtlist;	/* list of mounted filesystems	*/
BYTE nfs_signal=-1;		/* exported to CreateMyPort()	*/
AUTH *cred;			/* credentials for this daemon 	*/
static struct MsgPort *mp;	/* My NFS command port (AREXX?) */
struct Library *SockBase ;
int MAXSOCKS = 25;    /* MAXSOCKS-1 is the max number of volumes that */
                      /* can be mounted by the client */


/* local functions */
void main(void);
static void cleanup(int code);
static void nfs_mpx(register struct mount_pt *mt, register struct DosPacket *dp);

struct Library *SockBase ;


/*
 * main loop -  setup command port, alloc DOS packet arrival int, and
 *		loop waiting for something to do.
 */
void main(void)
{
	register struct mount_pt *here;
	register long nfs, cmd, event;
	register struct Message *msg;

	if ((SockBase = OpenLibrary( "inet:libs/socket.library", 0L ))==NULL) {
		(void)requester(0L," OK ","%s","Opening of socket library failed.");
		exit(RETURN_FAIL);
	}

	/* initialize for MAXSOCKS sockets */
	setup_sockets( MAXSOCKS, &errno );

	mp = CreateMsgPort();
	if(!mp){
		cleanup(RETURN_FAIL);
	}
	mp->mp_Node.ln_Pri = 0L;
	mp->mp_Node.ln_Name = CLIENT_COMMAND_PORT;
	AddPort(mp);

	if((nfs_signal = AllocSignal(-1L)) == -1){
		cleanup(RETURN_FAIL);
	}
	if(!(cred = authunix_create_default())){
		(void)requester(0L, " OK ", "%s\n%s\n%s\n",
			"NFSc: can't create authentication",
			"credentials.  Check that hostname,",
			"user id, and group id are set");
		cleanup(RETURN_FAIL);
	}

	nfs = 1L << nfs_signal;	cmd = 1L << mp->mp_SigBit;
	while (1) {
		event = Wait(nfs | cmd | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D);
		if(event & nfs){
			for(here = nfs_mtlist; here; here = here->mt_next){
				if(here->mt_port->mp_MsgList.lh_Head->ln_Succ){
					while(msg = GetMsg(here->mt_port)){
						register struct	DosPacket *dp;

						dp = (struct DosPacket *)msg->mn_Node.ln_Name;
						msg->mn_ReplyPort = dp->dp_Port;
						nfs_mpx(here, dp);
						ReplyMsg(msg);
					}
				}
			}
		}

		if(event & cmd){
			while(msg = GetMsg(mp)){
				nfs_command((struct nfs_req *)msg);
			}
		}

		if(event & SIGBREAKF_CTRL_C){
			cleanup(RETURN_OK);
		}

		if(event & SIGBREAKF_CTRL_D){
			struct mount_pt *mt;
			int busy;

			busy = 0;
			for(mt = nfs_mtlist; mt; mt = mt->mt_next){
				busy += mt->mt_nlocks + mt->mt_nfiles;
			}

			if(!busy){
				cleanup(RETURN_OK);
			}
		}
	}
}

static void cleanup(int code)
{
	struct mount_pt *mt, *newmt;
	struct nfs_req *msg;

	if(mp){
		Forbid();
		while(msg = (struct nfs_req *)GetMsg(mp)){
			msg->error = ECONFIGPROBLEM;
			ReplyMsg((struct Message *)msg);
		}
		RemPort(mp);
		DeleteMsgPort(mp);
		Permit();
	}

	for(mt = nfs_mtlist; mt; mt = newmt){
		newmt = mt->mt_next;
		free_mt(mt, 0L);
	}

	if(cred){
		auth_destroy(cred);
	}

	if(nfs_signal > 0){
		FreeSignal(nfs_signal);
	}

	cleanup_sockets();
	CloseLibrary(SockBase);
	exit(code);
}


static void
nfs_mpx(mt, dp)
	register struct mount_pt *mt;
	register struct DosPacket *dp;
{
#ifdef MONITOR
	char buff[255];
	struct FileInfoBlock *fib;
#endif
	dp->dp_Res1 = dp->dp_Res2 = 0L;

	switch (dp->dp_Type) {
	case ACTION_READ:
		#ifdef MONITOR
		KPrintF("ACTION_READ  fh=%ld\n",dp->dp_Arg1);
		#endif
		mt->mt_astat.action_read++;
		nfs_rd(mt,dp);
		break;

	case ACTION_WRITE:
		#ifdef MONITOR
		KPrintF("ACTION_WRITE  fh=%ld\n",dp->dp_Arg1);
		#endif
		mt->mt_astat.action_write++;
		nfs_wr(mt,dp);
		break;

	case ACTION_LOCATE_OBJECT:
		#ifdef MONITOR
		bst_to_comm(dp->dp_Arg2,buff);
		KPrintF("ACTION_LOCATE_OBJECT  lock=%ld  name=%s\n",dp->dp_Arg1,buff);
		#endif
		mt->mt_astat.action_locate_object++;
		nfs_locate(mt, dp);
		#ifdef MONITOR
		if(dp->dp_Res1)
			KPrintF("\tLock=%ld\n",dp->dp_Res1);
		else
			KPrintF("\tFAILED\n");
		#endif
		break;

	case ACTION_EXAMINE_NEXT:
		#ifdef MONITOR
		KPrintF("ACTION_EXAMINE_NEXT  dir=%ld\n",dp->dp_Arg1);
		#endif
		mt->mt_astat.action_examine_next++;
		goto examine;

	case ACTION_EXAMINE_OBJECT:
		#ifdef MONITOR
		KPrintF("ACTION_EXAMINE_OBJECT   obj=%ld\n",dp->dp_Arg1);
		#endif
		mt->mt_astat.action_examine_object++;
		goto exnext;

	case ACTION_FINDUPDATE:
		#ifdef MONITOR
		bst_to_comm(dp->dp_Arg3,buff);
		KPrintF("ACTION_UPDATE  lock=%ld  name=%s\n",dp->dp_Arg2,buff);
		#endif
		mt->mt_astat.action_rw++;
		goto openit;

	case ACTION_FINDOUTPUT:
		#ifdef MONITOR
		bst_to_comm(dp->dp_Arg3,buff);
		KPrintF("ACTION_FINDOUTPUT  lock=%ld  name=%s\n",dp->dp_Arg2,buff);
		#endif
		mt->mt_astat.mode_newfile++;
		goto openit;

	case ACTION_FINDINPUT:
		#ifdef MONITOR
		bst_to_comm(dp->dp_Arg3,buff);
		KPrintF("ACTION_FINDINPUT  lock=%ld  name=%s\n",dp->dp_Arg2,buff);
		#endif
		mt->mt_astat.mode_oldfile++;
openit:		nfs_open(mt,dp);
		#ifdef MONITOR
		KPrintF("\tfh=%ld\n",btod(dp->dp_Arg1,struct FileHandle *)->fh_Arg1);
		#endif
		break;


	case ACTION_FREE_LOCK:
		#ifdef MONITOR
		KPrintF("ACTION_FREE_LOCK  lock=%ld\n",dp->dp_Arg1);
		#endif
		mt->mt_astat.action_free_lock++;
		nfs_freelock(mt, dp);
		break;

	case ACTION_DELETE_OBJECT:
		#ifdef MONITOR
		bst_to_comm(dp->dp_Arg2,buff);
		KPrintF("ACTION_DELETE_OBJECT  lock=%ld  name=%s\n",dp->dp_Arg1,buff);
		#endif
		mt->mt_astat.action_delete_object++;
		nfs_delete(mt, dp);
		break;

	case ACTION_COPY_DIR:
		mt->mt_astat.action_copy_dir++;
		nfs_copydir(mt, dp);
		#ifdef MONITOR
		KPrintF("ACTION_COPY_DIR   lock=%ld   lock=%ld\n",dp->dp_Arg1, dp->dp_Res1);
		#endif
		break;


	case ACTION_CREATE_DIR:
		#ifdef MONITOR
		bst_to_comm(dp->dp_Arg2,buff);
		KPrintF("ACTION_CREATE_DIR  lock=%ld  name=%s\n",dp->dp_Arg1,buff);
		#endif
		mt->mt_astat.action_create_dir++;
		nfs_createdir(mt, dp);
		#ifdef MONITOR
		if(dp->dp_Res1==DOSFALSE)
			KPrintF("CREATE_DIR failed\n");
		#endif
		break;

	case ACTION_INFO:
		#ifdef MONITOR
		KPrintF("ACTION_INFO  lock=%ld\n",dp->dp_Arg1);
		#endif
		mt->mt_astat.action_info++;
		goto info;

	case ACTION_SEEK:
		#ifdef MONITOR
		KPrintF("ACTION_SEEK  fh=%ld\n",dp->dp_Arg1);
		#endif
		mt->mt_astat.action_seek++;
		nfs_seek(mt,dp);
		break;

	case ACTION_EX_NEXT:
		#ifdef MONITOR
		KPrintF("ACTION_EX_NEXT  dir=%ld\n",dp->dp_Arg1);
		#endif
		mt->mt_astat.action_ex_next++;
examine:	nfs_examine_next(mt, dp);
		#ifdef MONITOR
		if (dp->dp_Res1==DOSTRUE) {
			fib = btod(dp->dp_Arg2,struct FileInfoBlock *);
			KPrintF("Name = %s  DirEntryType=%ld\n",fib->fib_FileName+1, fib->fib_DirEntryType);
		} else
			KPrintF("Examine failed\n");
		#endif
		break;


	case ACTION_EX_OBJECT:
		#ifdef MONITOR
		KPrintF("ACTION_EX_OBJECT   obj=%ld\n",dp->dp_Arg1);
		#endif
		mt->mt_astat.action_ex_object++;
exnext:		nfs_examine(mt, dp);
		#ifdef MONITOR
		if (dp->dp_Res1==DOSTRUE) {
			fib = btod(dp->dp_Arg2,struct FileInfoBlock *);
			KPrintF("Name = %s  DirEntryType=%ld\n",fib->fib_FileName+1, fib->fib_DirEntryType);
		} else
			KPrintF("Examine failed\n");
		#endif
		break;

	case ACTION_PARENT:
		#ifdef MONITOR
		KPrintF("ACTION_PARENT of lock %ld",dp->dp_Arg1);
		#endif
		mt->mt_astat.action_parent++;
		nfs_parentdir(mt, dp);
		#ifdef MONITOR
		KPrintF("  returned %ld\n",dp->dp_Res1);
		#endif
		break;

	case ACTION_CURRENT_VOLUME:
		#ifdef MONITOR
		KPrintF("ACTION_CURRENT_VOLUME\n");
		#endif
		mt->mt_astat.action_current_volume++;
		dp->dp_Res1 = dtob(mt->mt_vol);
		dp->dp_Res2 = 0;
		break;

	case ACTION_END:
		#ifdef MONITOR
		KPrintF("ACTION_END  fh=%ld\n",dp->dp_Arg1);
		#endif
		mt->mt_astat.action_end++;
		nfs_close(mt,dp);
		break;

	case ACTION_RENAME_OBJECT:
		#ifdef MONITOR
		KPrintF("ACTION_RENAME_OBJECT\n");
		#endif
		mt->mt_astat.action_rename_object++;
		nfs_rename(mt, dp);
		break;

	case ACTION_SET_PROTECT:
		#ifdef MONITOR
		KPrintF("ACTION_SET_PROTECT\n");
		#endif
		mt->mt_astat.action_set_protect++;
		goto prot;

	case ACTION_CHMOD:
		#ifdef MONITOR
		KPrintF("ACTION_CHMOD\n");
		#endif
		mt->mt_astat.action_chmod++;
prot:		nfs_setprotect(mt, dp);
		break;

	case ACTION_SET_DATE:
		#ifdef MONITOR
		KPrintF("ACTION_SETDATE\n");
		#endif
		mt->mt_astat.action_setdate++;
		nfs_setdate(mt, dp);
		break;


	case ACTION_IS_FILESYSTEM:
		#ifdef MONITOR
		KPrintF("ACTION_IS_FILESYSTEM\n");
		#endif
		dp->dp_Res1 = -1;
		dp->dp_Res2 = 0;
		break;

	case ACTION_FH_FROM_LOCK:
		#ifdef MONITOR
		KPrintF("ACTION_FH_FROM_LOCK lk=%ld\n",dp->dp_Arg1);
		#endif
		nfs_fh_from_lk(mt,dp);
		break;

	case ACTION_SAME_LOCK:
		#ifdef MONITOR
		KPrintF("ACTION_SAME_LOCK  %ld  %ld\n",dp->dp_Arg1,dp->dp_Arg2);
		#endif
		nfs_same_lock(mt,dp);
		#ifdef MONITOR
		KPrintF("\tresult=%ld\n",dp->dp_Res1);
		if(dp->dp_Res1==DOSFALSE)
			KPrintF("\tFAILED\n");
		#endif		
		break;

	case ACTION_COPY_DIR_FH:
		nfs_copydir(mt, dp);
		#ifdef MONITOR
		KPrintF("ACTION_COPY_DIR_FH   fh=%ld   lock=%ld\n",dp->dp_Arg1, dp->dp_Res1);
		#endif
		break;
	
	case ACTION_PARENT_FH:
		#ifdef MONITOR
		KPrintF("ACTION_PARENT_FH of fh %ld",dp->dp_Arg1);
		#endif
		mt->mt_astat.action_parent++;
		nfs_parentdir(mt, dp);
		#ifdef MONITOR
		KPrintF("  returned %ld\n",dp->dp_Res1);
		#endif
		break;

	case ACTION_EXAMINE_FH:
		#ifdef MONITOR
		KPrintF("ACTION_EXAMINE_FH  %ld\n",dp->dp_Arg1);
		#endif
		nfs_examine(mt, dp);
		break;

	case ACTION_SET_FILE_SIZE:
		#ifdef MONITOR
		KPrintF("ACTION_SET_FILE_SIZE  %ld to %ld (%ld)\n",
		dp->dp_Arg1,dp->dp_Arg2,dp->dp_Arg3);
		#endif
		nfs_setsize(mt, dp);
		break;

	case ACTION_DISK_INFO:
		#ifdef MONITOR
		KPrintF("ACTION_DISK_INFO\n");
		#endif
		mt->mt_astat.action_disk_info++;
info:		nfs_dkinfo(mt, dp);
		#ifdef MONITOR
		if(dp->dp_Res1==DOSFALSE)
			KPrintF("\tFAILED\n");
		#endif
		break;

	case ACTION_RENAME_DISK:
		#ifdef MONITOR
		KPrintF("ACTION_RENAME_DISK\n");
		#endif
		mt->mt_astat.action_rename_disk++;
		nfs_rename_dsk(mt, dp);
		break;

	case ACTION_READ_LINK:
		nfs_readlink(mt,dp);
		#ifdef MONITOR
		KPrintF("ACTION_READ_LINK   lock=%ld  name=%s\n",dp->dp_Arg1,dp->dp_Arg2);
		if(dp->dp_Res1 >=0)
			KPrintF("new path=%s\n",dp->dp_Arg3);
		else
			KPrintF("Failed. Code=%ld\n",dp->dp_Res2);
		#endif
		break;

	case ACTION_MAKE_LINK:
		nfs_makelink(mt,dp);
		#ifdef MONITOR
		bst_to_comm(dp->dp_Arg2,buff);
		KPrintF("ACTION_MAKE_LINK   lock=%ld name=%s\n",dp->dp_Arg1,buff);
                KPrintF("Mode:%ld   Target: %s\n",dp->dp_Arg4, dp->dp_Arg3);
		if(dp->dp_Res1 == DOSTRUE)
			KPrintF("Link succeeded\n");
		else
			KPrintF("Link failed code=%ld\n",dp->dp_Res2);
		#endif
		break;

	case ACTION_SET_COMMENT:
		#ifdef MONITOR
		KPrintF("ACTION_SET_COMMENT\n");
		#endif
		dp->dp_Res1 = DOSTRUE;
		dp->dp_Res2 = 0;
		break;

	case ACTION_FLUSH:
		#ifdef MONITOR
		KPrintF("ACTION_FLUSH\n");
		#endif
		mt->mt_astat.action_flush++;
		dp->dp_Res1 = DOS_TRUE;
		break;

	case ACTION_MORE_CACHE:
		#ifdef MONITOR
		KPrintF("ACTION_MORECACHE\n");
		#endif
		mt->mt_astat.action_morecache++;
		dp->dp_Res2 = ERROR_NOT_IMPLEMENTED;
		break;


	default:
		#ifdef MONITOR
		KPrintF("Unknown packet %ld\n",dp->dp_Type);
		#endif
		mt->mt_astat.action_unknown++;
		dp->dp_Res1 = 0;
		dp->dp_Res2 = ERROR_ACTION_NOT_KNOWN;
		break;
	}
}

