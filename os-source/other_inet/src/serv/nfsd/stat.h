
/*
 * server statistics structure, one entry per NFS call 
 */
struct serverstats {
	u_long	nfsd_call;	/* calls to NFS server		*/
	u_long	nfsd_null;
	u_long	nfsd_getattr;
	u_long	nfsd_setattr;
	u_long	nfsd_root;
	u_long	nfsd_lookup;
	u_long	nfsd_readlink;
	u_long	nfsd_read;
	u_long	nfsd_writecache;
	u_long	nfsd_write;
	u_long	nfsd_create;
	u_long	nfsd_remove;
	u_long	nfsd_rename;
	u_long	nfsd_link;
	u_long	nfsd_symlink;
	u_long	nfsd_mkdir;
	u_long	nfsd_rmdir;
	u_long	nfsd_readdir;
	u_long	nfsd_statfs;
	u_long	nfsd_unknown;

	u_long	mntd_call;	/* calls to mount daemon	*/
	u_long	mntd_null;
	u_long	mntd_mnt;
	u_long	mntd_dump;
	u_long	mntd_umnt;
	u_long	mntd_umntall;
	u_long	mntd_export;
	u_long	mntd_exportall;
	u_long	mntd_unknown;

	u_long	rexx_call;	/* Rexx packet received		*/
};

extern struct serverstats stat;
