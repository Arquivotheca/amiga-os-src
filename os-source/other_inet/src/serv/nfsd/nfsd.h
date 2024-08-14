#ifndef __TYPES_RPC_HEADER__
#include <rpc/types.h>
#endif
#include <dos/dos.h>
#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <rpc/svc.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include "rexx.h"
#include "cookie.h"
#include "map.h"
#include "bitmap.h"
#include "mount.h"

/* bitmap.c */
nfsstat flush_bitmap(struct RexxMsg *rm);
nfsstat init_bitmap(void);
int num_bitmap(int num_maps);
map_id get_free_map(void);
void clean_bitmap(void);

/* build_vol.c */
nfsstat build_map(fs_id fsid, long lk, map_id parent);
nfsstat build_volume(struct RexxMsg *rm);

/* cache.c */
nfsstat get_root_dir(char *volume, cookie *ck);
nfsstat get_map_name(cookie *ck, register char *name, register mapentry *mapp);
nfsstat GET_MAP(register cookie *ck, register mapentry *mapp);
nfsstat get_map(register map_id id, register mapentry *mapp);
nfsstat update_map(register mapentry *mapp);
nfsstat put_map(register mapentry *mapp);
nfsstat get_new_map(register cookie *ck, mapentry *child);
nfsstat get_lock(register cookie *ck, BPTR *lkp);
nfsstat get_fd(register cookie *ck, BPTR *fhp);
int init_cache(struct RexxMsg *rm);
void close_cache(register mapentry *mapp);
nfsstat flush_open(struct RexxMsg *rm);
void clean_cache(void);

/* create.c */
diropres * nfsproc_create_2(createargs *argp, struct svc_req *rqstp);
diropres * nfsproc_mkdir_2(createargs *argp, struct svc_req *rqstp);

/* fh.c */
nfsstat fh_delete(register cookie *ck, char *name);
nfsstat fh_build(cookie *ck, char *name, ftype what, cookie *newck);
nfsstat fh_buildpath(cookie *ck, char path[NFS_MAXPATHLEN], char *name);
int delete_map(struct RexxMsg *rm);
nfsstat rexx_get_path(struct RexxMsg *rm);

/* getattr.c */
nfsstat getattr(cookie *ck, fattr *attr);
attrstat *nfsproc_getattr_2(nfs_fh *argp, struct svc_req *rqstp);

/* link.c */
nfsstat *nfsproc_link_2(linkargs *argp, struct svc_req *rqstp);
readlinkres *nfsproc_readlink_2(nfs_fh *argp, struct svc_req *rqstp);
nfsstat *nfsproc_symlink_2(symlinkargs *argp, struct svc_req *rqstp);


/* lookup.c */
diropres *nfsproc_lookup_2(diropargs *argp, struct svc_req *rqstp);

/* main.c */
void clean_up(struct RexxMsg *rm);
void cleanup(int code);

/* map.c */
nfsstat read_map(register map_id id, register mapentry *mapp);
nfsstat write_map(register map_id id, register mapentry *mapp);
int build_map_file(struct RexxMsg *rm);
nfsstat init_map(struct RexxMsg *rm);
nfsstat query(register struct RexxMsg *rm);
void clean_map(void);

/* mntd.c */
void *mountproc_null_1(void *, struct svc_req *);
fhstatus *mountproc_mnt_1(dirpath *, struct svc_req *);
mountlist *mountproc_dump_1(void *, struct svc_req *);
void *mountproc_umnt_1(dirpath *, struct svc_req *);
void *mountproc_umntall_1(void *, struct svc_req *);
exports *mountproc_export_1(void *, struct svc_req *);
exports *mountproc_exportall_1(void *, struct svc_req *);

/* mount_svc.c */
void mountprog_1(struct svc_req *, SVCXPRT *transp);

/* nfs_io.c */
BPTR nfs_lock(char *name, LONG how);
BPTR nfs_open(char *name, LONG how);
BPTR nfs_createdir(char *path);
long nfs_wr(BPTR fh, void *buf, long len);
long nfs_rd(BPTR fh, void *buf, long len);
long nfs_seek(BPTR fh, long pos, long how);
BOOL nfs_examine(BPTR lk, struct FileInfoBlock *fib);
BOOL nfs_exnext(BPTR lk, struct FileInfoBlock *fib);
void nfs_unlock(BPTR lk);
BOOL nfs_close(BPTR fh);
BOOL nfs_rename(char *from, char *to);
BOOL nfs_info(BPTR lk, struct InfoData *id);
void init_io(void);

/* nfs_svc.c */
void nfs_program_2(struct svc_req *, SVCXPRT *);
void *nfsproc_null_2(void *, struct svc_req *);
void *nfsproc_writecache_2(void *, struct svc_req *);
void *nfsproc_root_2(void *, struct svc_req *);

/* nfserr.c */
int nfserrtxt(struct RexxMsg *rm);
nfsstat io_to_nfserr(void);
void seterror(struct RexxMsg *rm, nfsstat code);

/* nfstime.c */
nfsstat init_time(void);
void clean_time(void);
nfsstat NFStime(nfstime *tp);
void nfs_to_amigatime(nfstime *ns, struct DateStamp *ds);
void amiga_to_nfstime(struct DateStamp *ds, nfstime *ns);

/* portmap.c */
void abort(void);
int init_pmap(void);
void clean_pmap(void);
void reg_service(struct svc_req *rqstp, SVCXPRT *xprt);

/* read.c */
readres *nfsproc_read_2(readargs *argp, struct svc_req *rqstp);

/* readdir.c */
readdirres *nfsproc_readdir_2(readdirargs *argp, struct svc_req *rqstp);

/* readlink.c */
readlinkres *nfsproc_readlink_2(nfs_fh *argp, struct svc_req *rqstp);

/* remove.c */
nfsstat *nfsproc_remove_2(diropargs *argp, struct svc_req *rqstp);

/* rename.c */
nfsstat *nfsproc_rename_2(renameargs *argp, struct svc_req *rqstp);

/* rmdir.c */
nfsstat *nfsproc_rmdir_2(diropargs *argp, struct svc_req *rqstp);

/* setattr.c */
nfsstat setattr(cookie *ck, sattr *att, fattr *fatt);
attrstat *nfsproc_setattr_2(sattrargs *argp, struct svc_req *rqstp);

/* statfs.c */
statfsres *nfsproc_statfs_2(nfs_fh *argp, struct svc_req *rqstp);

/* svc_run.c */
void svc_run(void);
void init_svc(void);
void clean_svc(void);

/* symlink.c */
nfsstat *nfsproc_symlink_2(symlinkargs *argp, struct svc_req *rqstp);

/* write.c */
nfsstat change_file_size(BPTR fh, unsigned long tsize, register mapentry *mp);
attrstat *nfsproc_write_2(writeargs *argp, struct svc_req *rqstp);


