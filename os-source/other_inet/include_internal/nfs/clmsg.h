/*
** ioctl's for NFS daemon
*/

#ifndef NFS_CLMSG_H
#define NFS_CLMSG_H

#ifndef SYS_PARAM_H
#include <sys/param.h>
#endif

#include <rpc/types.h>
#include <rpc/rpc.h>

#define NFSC_UMOUNT	0
#define NFSC_MOUNT	1
#define NFSC_GETSTAT	2
#define NFSC_REFRESH	4

struct nfs_req {
	struct Message msg;		/* standard exec message	*/
	short	opcode;			/* operation code		*/
	short	error;			/* inet layer problem		*/
	struct rpc_err rpc_error;	/* RPC layer problem		*/
	short	nfs_error;		/* NFS layer error		*/
};

struct nfs_umount_req {
	struct nfs_req nu_msg;		/* message header		*/
	char	nu_volume[MAXVOL];	/* amiga volume name or "all"	*/
};

struct amiga_stats {
	long	action_rw;
	long	mode_newfile;
	long	mode_oldfile;
	long	action_end;
	long	action_seek;
	long	action_read;
	long	action_write;
	long	action_info;
	long	action_disk_info;
	long	action_locate_object;
	long	action_rename_disk;
	long	action_free_lock;
	long	action_delete_object;
	long	action_copy_dir;
	long	action_set_protect;
	long	action_chmod;
	long	action_create_dir;
	long	action_examine_next;
	long	action_ex_next;
	long	action_examine_object;
	long	action_ex_object;
	long	action_parent;
	long	action_rename_object;
	long	action_morecache;
	long	action_flush;
	long	action_setdate;
	long	action_current_volume;
	long	action_unknown;
	long	action_future[16];
};

struct nfs_stats {
	long 	ns_nstat[25];		/* NFS RPC call statistics	*/
	long	ns_retries;		/* number of retries required	*/
};

struct nfs_stats_req {
	struct nfs_req	ns_msg;		/* boilerplate			*/
	char ns_volume[MAXVOL];		/* which volume or "all"	*/
	struct amiga_stats ns_astat;	/* amiga handler call statistics*/
	struct nfs_stats ns_nstat;	/* NFS statistics		*/
};

struct nfs_mount_req {
	struct nfs_req nm_msg;		/* standard message header	*/
	struct timeval nm_retry;	/* retry period			*/
	struct timeval nm_total;	/* timeout period		*/
	u_short	nm_rdsize;		/* MAX read packet size		*/
	u_short	nm_wrsize;		/* MAX write packet size	*/
	u_long	nm_cachesize;		/* amount of cache to use	*/
	u_short	nm_port;		/* NFS port # to use		*/
	char	nm_readonly;		/* Mount this volume readonly	*/
	u_char	nm_case;		/* don't do case conversions	*/
	char	nm_volume[MAXVOL];	/* amiga volume name		*/
	char 	nm_host[MAXHOSTNAMELEN];/* name of server		*/
	char	nm_path[NFS_MAXPATHLEN];/* path on server		*/
};

#define CLIENT_COMMAND_PORT "NFS-client-command"
#define SERVER_COMMAND_PORT "NFS-server-command"

#endif
