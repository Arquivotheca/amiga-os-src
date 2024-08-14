/* -----------------------------------------------------------------------
 * mount.c  nfsc  SAS
 *
 * $Locker:  $
 *
 * $Id: mount.c,v 1.4 92/12/04 14:15:36 bj Exp $
 *
 * $Revision: 1.4 $
 *
 * $Header: Hog:Other/inet/src/c/nfsc/RCS/mount.c,v 1.4 92/12/04 14:15:36 bj Exp $
 *
 *------------------------------------------------------------------------
 */



/*
 * Mount/Unmount handling
 */
 
typedef long BPTR ;

#include "fs.h"
#include <exec/ports.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <rpcsvc/mount.h>

#include <string.h>

extern struct mount_pt *nfs_mtlist;
extern struct rpc_createerr rpc_createerr;
extern AUTH *cred;

static struct timeval dummy = {5, 0};	/* client call uses defaults */

/* local functions */
static void add_astat(struct amiga_stats *to, struct amiga_stats *from);
static void add_nstat(register struct nfs_stats *to, register struct nfs_stats *from);
static void nfs_stats(struct nfs_stats_req *req);
static void nfs_refresh(struct nfs_req *req);
static void nfs_mount(struct nfs_mount_req *req);
static void nfs_umount(struct nfs_umount_req *req);
static int set_attr(struct mount_pt *mt,struct nfs_req *req);

#include <libraries/dosextens.h>

void
nfs_command(req)
	struct nfs_req *req;
{
	req->error = 0;
	req->nfs_error = NFS_OK;
	bzero(&req->rpc_error, sizeof(req->rpc_error));

	switch(req->opcode){
	case NFSC_UMOUNT:
		nfs_umount((struct nfs_umount_req *)req);
		break;

	case NFSC_MOUNT:
		nfs_mount((struct nfs_mount_req *)req);
		break;

	case NFSC_GETSTAT:
		nfs_stats((struct nfs_stats_req *)req);
		break;

	case NFSC_REFRESH:
		nfs_refresh(req);
		break;

	default:
		req->error = EINVAL;
	}

	ReplyMsg((struct Message *)req);
}

static void
add_astat(to, from)
	struct amiga_stats *to, *from;
{
	to->action_rw += from->action_rw;
	to->mode_newfile += from->mode_newfile;
	to->mode_oldfile += from->mode_oldfile;
	to->action_end += from->action_end;
	to->action_seek += from->action_seek;
	to->action_read += from->action_read;
	to->action_write += from->action_write;
	to->action_info += from->action_info;
	to->action_disk_info += from->action_disk_info;
	to->action_locate_object += from->action_locate_object;
	to->action_rename_disk += from->action_rename_disk;
	to->action_free_lock += from->action_free_lock;
	to->action_copy_dir += from->action_copy_dir;
	to->action_set_protect += from->action_set_protect;
	to->action_chmod += from->action_chmod;
	to->action_create_dir += from->action_create_dir;
	to->action_examine_next += from->action_examine_next;
	to->action_ex_next += from->action_ex_next;
	to->action_examine_object += from->action_examine_object;
	to->action_ex_object += from->action_ex_object;
	to->action_parent += from->action_parent;
	to->action_rename_object += from->action_rename_object;
	to->action_morecache += from->action_morecache;
	to->action_flush += from->action_flush;
	to->action_setdate += from->action_setdate;
	to->action_current_volume += from->action_current_volume;
	to->action_delete_object += from->action_delete_object;
	to->action_unknown += from->action_unknown;
}

static void
add_nstat(to, from)
	register struct nfs_stats *to, *from;
{
	register int i;

	for(i = 0; i < sizeof(to->ns_nstat)/sizeof(to->ns_nstat[0]); i++){
		to->ns_nstat[i] += from->ns_nstat[i];
	}
}

static void
nfs_stats(req)
	struct nfs_stats_req *req;
{
	struct mount_pt *mt, *voltomt();

	if(stricmp("all", req->ns_volume) == 0){
		bzero(&req->ns_nstat, sizeof(req->ns_nstat));
		bzero(&req->ns_astat, sizeof(req->ns_astat));
		for(mt = nfs_mtlist; mt; mt = mt->mt_next){
			add_nstat(&req->ns_nstat, &mt->mt_nstat);
			add_astat(&req->ns_astat, &mt->mt_astat);
		}
	} else {
		mt = voltomt(req->ns_volume);
		if(!mt){
			req->ns_msg.error = ENOENT;
			return ;
		}
		req->ns_nstat = mt->mt_nstat;
		req->ns_astat = mt->mt_astat;
	}
}

static void
nfs_refresh(struct nfs_req *req)
{
	struct mount_pt *mt;

	auth_destroy(cred);
	cred = authunix_create_default();
	if(!cred){
		return;
	}

	for(mt = nfs_mtlist; mt; mt = mt->mt_next){
		if(mt->mt_host){
			mt->mt_host->cl_auth = cred;
		}
	}
}

// #define DEBUG 1 /**************** DEBUG ***********************/

static void
nfs_mount(req)
	struct nfs_mount_req *req;
{
	struct mount_pt *mt, *insert_vname();
	struct sockaddr_in server;
	enum clnt_stat stat;
	char *path;
	fhstatus df;
	CLIENT *cl;
	int s;
#ifdef DEBUG   /****************** DEBUG *********************/
	BPTR debugfh ;
#endif	

	if((req->nm_msg.error = validvol(req->nm_volume)) != 0){
		return ;
	}
	cl = clnt_create(req->nm_host, MOUNTPROG, MOUNTVERS, "udp");
	if(!cl){
		if(rpc_createerr.cf_error.re_status == RPC_SYSTEMERROR){
			req->nm_msg.error = rpc_createerr.cf_error.re_errno;
		}
		req->nm_msg.rpc_error.re_status = rpc_createerr.cf_stat;
		return ;
	}

	cl->cl_auth = cred;

	path = req->nm_path;
	clnt_control(cl, CLSET_TIMEOUT, &req->nm_total);
	clnt_control(cl, CLSET_RETRY_TIMEOUT, &req->nm_retry);

	stat = clnt_call(cl, MOUNTPROC_MNT, xdr_nfspath, &path,
					    xdr_fhstatus, &df,
					    dummy);
	if(stat != RPC_SUCCESS || df.fhs_status != NFS_OK) {
		clnt_geterr(cl, &req->nm_msg.rpc_error);
		req->nm_msg.nfs_error = df.fhs_status;
		return;
	}

	clnt_control(cl, CLGET_SERVER_ADDR, &server);
	server.sin_port = htons(req->nm_port);
	clnt_destroy(cl);

	mt = insert_vname(req->nm_volume);
	if(!mt){
		req->nm_msg.error = ENOMEM;
		return ;
	}

	mt->mt_lock.fd_lock.fl_Key = 0;
	mt->mt_lock.fd_lock.fl_Volume = dtob(mt->mt_vol);
	mt->mt_lock.fd_lock.fl_Task = mt->mt_port;
	mt->mt_lock.fd_magic = FD_MAGIC;
	mt->mt_lock.fd_pos = 0;
	mt->mt_lock.fd_file_fid = 0;
	mt->mt_lock.fd_dir_fid = 0;
	bzero(&mt->mt_lock.fd_dirpos, NFS_COOKIESIZE);
	bcopy(&df.fhstatus_u.fhs_fhandle, &mt->mt_lock.fd_dir, NFS_FHSIZE);
	mt->mt_lock.fd_head = mt->mt_lock.fd_next = (struct entry *)NULL;
	mt->mt_lock.fd_type = FD_DIR;
	mt->mt_lock.fd_flags = 0;
	mt->mt_lock.fd_cache = 0;
	mt->mt_rdsize = req->nm_rdsize;
	mt->mt_wrsize = req->nm_wrsize;
	mt->mt_nfiles = mt->mt_nlocks = 0;
	mt->mt_case = req->nm_case;
	strcpy(mt->mt_path, req->nm_path);
	s = RPC_ANYSOCK;
	mt->mt_host = clntudp_create(&server, NFS_PROGRAM, NFS_VERSION,
					dummy, &s);
	if(!mt->mt_host){
		if(rpc_createerr.cf_error.re_status == RPC_SYSTEMERROR){
			req->nm_msg.error = rpc_createerr.cf_error.re_errno;
		}
		req->nm_msg.rpc_error.re_status = rpc_createerr.cf_stat;
		return ;
	}

	clnt_control(mt->mt_host, CLSET_TIMEOUT, &req->nm_total);
	clnt_control(mt->mt_host, CLSET_RETRY_TIMEOUT, &req->nm_retry);
    	mt->mt_host->cl_auth = cred;

#ifdef DEBUG
	debugfh = Open("CON:0/0/600/70/NFSC Debug/AUTO/WAIT/CLOSE", MODE_NEWFILE) ;
	if(debugfh)
	{
		VFPrintf( debugfh, "timeout = %ld\n", &req->nm_total) ;
		VFPrintf( debugfh, "retry   = %ld\n", &req->nm_retry) ;
		Close(debugfh) ;
	}
#endif

	if(!set_attr(mt, (struct nfs_req *)req)){
		clnt_destroy(mt->mt_host);
		deletevol(mt->mt_vol);
		free(mt);
		return ;
	}

	mt->mt_next = nfs_mtlist;
	nfs_mtlist = mt;
	return ;
}

/*
 * nfs_umount - "unmount" filesystem associated with given volume name.  Only
 *		processing performed here is local, eg removing volume from
 *		vol list, deallocating memory, etc.  The caller must notify
 *		the server.
 */
static void
nfs_umount(req)
	struct nfs_umount_req *req;
{
	register struct mount_pt *mt, *nextmt;
	struct mount_pt *voltomt();

	if(stricmp(req->nu_volume, "all") == 0){
		for(mt = nfs_mtlist; mt; mt = nextmt){
			nextmt = mt->mt_next;
			if(mt->mt_nfiles || mt->mt_nlocks){
				req->nu_msg.error = EBUSY;
			} else {
				free_mt(mt, req);
			}
		}
		return ;
	}

	mt = voltomt(req->nu_volume);
	if(!mt){
		req->nu_msg.error = ENOENT;
		return ;
	}

	if(mt->mt_nfiles || mt->mt_nlocks){
		req->nu_msg.error = EBUSY;
		return ;
	}

	free_mt(mt, req);

	return ;
}

/*
 * set_attr - get attributes for root filehandle, stat filehandle to get
 *	      max read/write transfer sizes server prefers.
 */
static int
set_attr(mt, req)
	struct mount_pt	*mt;
	struct nfs_req *req;
{
	struct statfsres fs;
	enum clnt_stat stat;
	struct attrstat fa;

	fa.status = NFS_OK;
#define OLD
#ifdef OLD
	stat = clnt_call(mt->mt_host, NFSPROC_GETATTR,
				xdr_nfs_fh, &mt->mt_lock.fd_dir,
				xdr_attrstat, &fa, dummy);
	if(stat != RPC_SUCCESS || fa.status != NFS_OK){
		clnt_geterr(mt->mt_host, &req->rpc_error);
		req->nfs_error = fa.status;
		return 0;
	}
#else
{
		struct diropargs di;
		struct diropres dor;

		di.dir = mt->mt_lock.fd_dir;
		di.name = "..";
		stat = clnt_call(mt->mt_host, NFSPROC_LOOKUP,
					xdr_diropargs, &di,
					xdr_diropres, &dor, dummy);
		if(stat != RPC_SUCCESS || dor.status != NFS_OK){
			clnt_geterr(mt->mt_host, &req->rpc_error);
			req->nfs_error = dor.status;
			return 0;
		}
		fa.attrstat_u.attributes = dor.diropres_u.diropres.attributes;
}
#endif
	if(fa.attrstat_u.attributes.type != NFDIR){
		req->nfs_error = NFSERR_NOTDIR;
		return 0;
	}

	mt->mt_lock.fd_dir_fid = fa.attrstat_u.attributes.fileid;

	fs.status = NFS_OK;
	stat = clnt_call(mt->mt_host, NFSPROC_STATFS,
					xdr_nfs_fh, &mt->mt_lock.fd_dir,
					xdr_statfsres, &fs,
					dummy);
	if(stat != RPC_SUCCESS || fs.status != NFS_OK) {
		clnt_geterr(mt->mt_host, &req->rpc_error);
		req->nfs_error = fs.status;
		return 0;
	}

#define TSIZE fs.statfsres_u.reply.tsize
	if(TSIZE < 0 || TSIZE > NFS_MAXDATA){
		TSIZE = 1024;
	}

	if (mt->mt_rdsize <= 0 || mt->mt_rdsize > TSIZE){
		mt->mt_rdsize = TSIZE;
	}
	if (mt->mt_wrsize <= 0 || mt->mt_wrsize > TSIZE){
		mt->mt_wrsize = TSIZE;
	}
#undef TSIZE

	return 1;
}

/*
 * Cleanup & free mount_pt structure.
 */
int
free_mt(mt, req)
	register struct mount_pt *mt;
	struct nfs_umount_req *req;
{
	register struct	DosPacket *dp;
	register struct mount_pt **p;
	struct Message *msg;
	struct sockaddr_in server;
	enum clnt_stat stat;
	struct timeval to;
	CLIENT *cl;
	char *path;
	int s;

	for(p = &nfs_mtlist; *p; p = &(*p)->mt_next){
		if(*p == mt){
			break;
		}
	}
	if(!*p){
		return (0);
	}

	/*
	 * Pull volume off both dos and local lists.
	 */
	Forbid();
	*p = mt->mt_next;
	Permit();
	deletevol(mt->mt_vol);

	/*
	 * Try to unmount volume at server.  If this fails we go ahead and
	 * remove the local volume information anyways.
	 */
	if(mt->mt_host){
		clnt_control(mt->mt_host, CLGET_SERVER_ADDR, &server);
		s = RPC_ANYSOCK;
		server.sin_port = 0;
		cl = clntudp_create(&server, MOUNTPROG, MOUNTVERS, dummy, &s);
		if(!cl){
			if(req){
				if(rpc_createerr.cf_error.re_status == RPC_SYSTEMERROR){
					req->nu_msg.error = rpc_createerr.cf_error.re_errno;
				}
				req->nu_msg.rpc_error.re_status = rpc_createerr.cf_stat;
			}
		} else {
			clnt_control(mt->mt_host, CLGET_TIMEOUT, &to);
			clnt_control(cl, CLSET_TIMEOUT, &to);
			clnt_control(mt->mt_host, CLGET_RETRY_TIMEOUT, &to);
			clnt_control(cl, CLSET_RETRY_TIMEOUT, &to);
			path = mt->mt_path;
			cl->cl_auth = cred;

			stat = clnt_call(cl, MOUNTPROC_UMNT,
							xdr_nfspath, &path,
							xdr_void, 0L, dummy);
			if(stat != RPC_SUCCESS && req){
				clnt_geterr(cl, &req->nu_msg.rpc_error);
			}

			clnt_destroy(cl);
		}
		clnt_destroy(mt->mt_host);
	}

	/*
	 * Return any DOS packets hanging around.  Ordinarily this should
	 * not be necessary.
	 */
	Forbid();
	while(msg = GetMsg(mt->mt_port)){
		dp = (struct DosPacket *)msg->mn_Node.ln_Name;
		dp->dp_Res1 = 0L;
		dp->dp_Res2 = ERROR_DEVICE_NOT_MOUNTED;
		ReplyMsg(msg);
	}
	DeleteMyPort(mt->mt_lock.fd_lock.fl_Task);
	Permit();

	FreeMem(mt,sizeof(*mt));
	mt = (struct mount_pt *)0;

	return (1);
}
