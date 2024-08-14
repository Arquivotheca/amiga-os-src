/*-----------------------------------------------------------------------
 * lk.c  nfsc  SAS
 *
 * $Locker:  $
 *
 * $Id: lk.c,v 1.4 92/10/07 15:07:42 bj Exp $
 *
 * $Revision: 1.4 $
 *
 * $Header: AS225:src/c/nfsc/RCS/lk.c,v 1.4 92/10/07 15:07:42 bj Exp $
 *
 *------------------------------------------------------------------------
 */



/*
 * Routines which take lock(s) as arguments.
 *
 * nfs_parentdir()
 * nfs_delete()
 * nfs_createdir()
 * nfs_locate()
 * nfs_copydir()
 * nfs_freelock()
 * nfs_setprotect()
 * nfs_setdate()
 * nfs_rename()
 * nfs_readlink()
 * nfs_makelink()
 */

#include "fs.h"
#include <string.h>

#define ZEROCOOKIE(c) bzero(&(c), NFS_COOKIESIZE);

/*
 * Return parent dir & attributes
*/
bool_t
get_parent(mt, dp, dirp, dorp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
	nfs_fh *dirp;
	struct diropres *dorp;
{
	struct diropargs di;
	enum clnt_stat err;

	di.name = "..";
	di.dir = *dirp;
	err = nfs_call(mt, NFSPROC_LOOKUP,xdr_diropargs, &di, xdr_diropres, dorp);
	if(err != RPC_SUCCESS || dorp->status != NFS_OK){
		dp->dp_Res2 = fh_status(err, dorp->status);
		return FALSE;
	}

	return TRUE;
}

/*
 * ParentDir is a little bit tricky at the boundary condition where the
 * application has asked for ParentDir of an object in the root directory.
 * For example ParentDir on a lock of usr:tmp is supposed to return a lock
 * on the volume, and ParentDir on a lock of usr: is supposed to return 0.
 */

/*
**	nfs_parentdir (mount_pt mt, struct DosPacket *dp)
**
**	ACTION_PARENT
**
**	dp->dp_Arg1	BPTR	Lock on object to get the parent of
**
**	dp->dp_Res1	BPTR	Parent lock
**	dp->dp_Res2	CODE	Failure code if Res1 == 0
**
**	ACTION_PARENT_FH
**	
**	dp->dp_Arg1	BPTR	FileHandle
*/

void
nfs_parentdir(mt, dp)
	struct mount_pt  *mt;
	struct DosPacket *dp;
{
	register struct fdata *fd, *nfd;
	struct diropres dor;

	if( dp->dp_Type == ACTION_PARENT_FH ) { /* FileHandle */
		fd = (struct fdata *)dp->dp_Arg1;
		if(fd==0 || (((u_long)fd)&1) || fd->fd_magic != FD_MAGIC){
			dp->dp_Res2 = ERROR_INVALID_LOCK;
			return;
		}
	} else  /* Lock */
		CHECKLOCK(fd, dp);

	/*
	 * Return 0 if ParentDir on mount point
	 */
	if(fd->fd_type == FD_DIR && fd->fd_dir_fid == mt->mt_lock.fd_dir_fid){
		dp->dp_Res2 = dp->dp_Res1 = 0;
		return;
	}

	nfd = (struct fdata *)AllocMem(sizeof(*nfd), MEMF_CLEAR|MEMF_PUBLIC);
	if(nfd == (struct fdata *)0){
		dp->dp_Res2 = ERROR_NO_FREE_STORE;
		return;
	}

	*nfd = *fd;
	nfd->fd_head = nfd->fd_next = (struct entry *)NULL;

	if(nfd->fd_type == FD_DIR){
		if(!get_parent(mt, dp, &nfd->fd_dir, &dor)){
			return;
		}
		nfd->fd_dir = dor.diropres_u.diropres.file;
		nfd->fd_dir_fid = dor.diropres_u.diropres.attributes.fileid;
	} else {
		nfd->fd_type = FD_DIR; 	/* Already had parent in fd_dir */
	}

	nfd->fd_lock.fl_Key = nfd->fd_dir_fid;
	dp->dp_Res1 = dtob(nfd);
	mt->mt_nlocks++;
}


/*
**	nfs_delete (mount_pt mt, struct DosPacket *dp)
**
**	dp->dp_Arg1	BPTR	Lock that Arg2 is relative to
**	dp->dp_Arg2	BSTR	Name of object to delete (relative to Arg1)
**
**	dp->dp_Res1	BOOL	Success/Failure (DOSTRUE/DOSFALSE)
**	dp->dp_Res2	CODE	Failure code if Res1 is DOSFALSE
**
*/


void nfs_delete(mt, dp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
{
	register struct fdata *fd;
	char name[NFS_MAXNAMLEN];
	int opc, status, cnt;
	struct diropargs di;
	struct diropres dor;
	enum clnt_stat err;
	bool_t nfs_status;
	struct fdata nfd;

	CHECKLOCK(fd, dp);

	status = ptofh(mt, dp, fd, btod(dp->dp_Arg2, char *), &nfd, name);
	if(status != ALLFOUND){
		return;
	}

	cnt = *btod(dp->dp_Arg2, unsigned char *);
	if (cnt == 0) {
		dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
		return;
	}

	di.name = name;
	if(nfd.fd_type == FD_DIR){
		opc = NFSPROC_RMDIR;
		if(nfd.fd_dir_fid != mt->mt_lock.fd_dir_fid){ /* root */
			if(!get_parent(mt, dp, &nfd.fd_dir, &dor)){
				return;
			}
		}
		di.dir = dor.diropres_u.diropres.file;
	} else {
		opc = NFSPROC_REMOVE;
		di.dir = nfd.fd_dir;
	}

	err = nfs_call(mt, opc, xdr_diropargs, &di, xdr_enum, &nfs_status);
	if(err != RPC_SUCCESS || nfs_status != NFS_OK){
		dp->dp_Res2 = nfs_status==NFSERR_PERM ?
				ERROR_DELETE_PROTECTED:fh_status(err, nfs_status);
		return;
	}

	dp->dp_Res1 = DOS_TRUE;
	dp->dp_Res2 = 0;
}

/*
**	nfs_createdir(mount_pt mt, struct DosPacket *dp)
**
**	dp->dp_Arg1	BPTR	Lock that Arg2 is relative to
**	dp->dp_Arg2	BSTR	Name of new directory (relative to Arg1)
**
**	dp->dp_Res1	BOOL	Success/Failure (DOSTRUE/DOSFALSE)
**	dp->dp_Res2	CODE	Failure code if Res1 is DOSFALSE
**
*/


void
nfs_createdir(mt, dp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
{
	register struct fdata *nfd, *fd;
	char name[NFS_MAXNAMLEN];
	struct createargs ca;
	struct diropres dor;
	enum clnt_stat err;
	int status;
	long mask;

	CHECKLOCK(fd, dp);

	nfd = (struct fdata *)AllocMem(sizeof(*nfd), MEMF_CLEAR|MEMF_PUBLIC);
	if(nfd == (struct fdata *)0){
		dp->dp_Res2 = ERROR_NO_FREE_STORE;
		return;
	}

	status = ptofh(mt, dp, fd, btod(dp->dp_Arg2, char *), nfd, name);
	if(status != LASTNOT){
		FreeMem(nfd, sizeof(*nfd));
		return; 	/* Error code already set by ptofh */
	}

	if(name != (char *)NULL && nfd->fd_type == FD_DIR){
		ca.where.name = name;
		ca.where.dir = nfd->fd_dir;

		mask = getumask();

		ca.attributes.mode = (~mask) & 0777;
		ca.attributes.mode |= NFSMODE_DIR;

		ca.attributes.uid = getuid();
		ca.attributes.gid = getgid();
		ca.attributes.size = 512;
		ca.attributes.atime.seconds = -1;
		ca.attributes.atime.useconds = -1;
		ca.attributes.mtime.seconds = -1;
		ca.attributes.mtime.useconds = -1;
		err = nfs_call(mt, NFSPROC_MKDIR, xdr_createargs, &ca,
						  xdr_diropres, &dor);
		if(err != RPC_SUCCESS || dor.status != NFS_OK){
			FreeMem(nfd, sizeof(*nfd));
			dp->dp_Res2 = fh_status(err, dor.status);
			return;
		}


		nfd->fd_lock = fd->fd_lock;
		nfd->fd_dir = dor.diropres_u.diropres.file;
		nfd->fd_dir_fid = dor.diropres_u.diropres.attributes.fileid;
		nfd->fd_type = FD_DIR;
		dp->dp_Res1 = dtob(nfd);
		dp->dp_Res2 = 0;
		mt->mt_nlocks++;

	} else {
		dp->dp_Res2 = ERROR_INVALID_COMPONENT_NAME;
	}
}

/*
**	nfs_locate (mount_pt mt, struct DosPacket *dp)
**
**	dp->dp_Arg1	BPTR	Lock on directory that Arg2 is relative to
**	dp->dp_Arg2	BSTR	Name or path of object to lock (relative to Arg1)
**	dp->dp_Arg3	LONG	Mode: ACCESS_READ/SHARED_LOCK, ACCESS_WRITE/EXCLUSIVE_LOCK
**
**	dp->dp_Res1	BPTR	Lock on requested object or 0 if failure
**	dp->dp_Res2	CODE	Failure code if Res1 == 0
**
*/

void
nfs_locate(mt, dp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
{
	register struct fdata *nfd, *fd;
	char name[NFS_MAXNAMLEN];
	int status;

	CHECKLOCK(fd, dp);

	nfd = (struct fdata *)AllocMem(sizeof(*nfd), MEMF_CLEAR|MEMF_PUBLIC);
	if(nfd == (struct fdata *)0){
		dp->dp_Res2 = ERROR_NO_FREE_STORE;
		return;
	}

	status = ptofh(mt, dp, fd, btod(dp->dp_Arg2, char *), nfd, name);
	if(nfd->fd_type==FD_LINK) {
		dp->dp_Res2 = ERROR_IS_SOFT_LINK;
		goto bad_locate;
	}

	if(status != ALLFOUND){
bad_locate:	FreeMem(nfd, sizeof(*nfd));
		return;
	}

	nfd->fd_lock = mt->mt_lock.fd_lock;
	nfd->fd_lock.fl_Key = nfd->fd_dir_fid;
	nfd->fd_magic = FD_MAGIC;
	mt->mt_nlocks++;

	dp->dp_Res1 = dtob(nfd);
	dp->dp_Res2 = 0;
}

/*
**	nfs_copydir (mount_pt mt, struct DosPacket *dp)
**
**	dp->dp_Arg1	BPTR	Lock (or FileHandle) to duplicate
**
**	dp->dp_Res1	BPTR	Duplicated lock or 0 if failure
**	dp->dp_Res2	CODE	Failure code if Res1 == 0
**
**	This function makes a duplicate lock from a lock or filehandle
*/

void
nfs_copydir(mt, dp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
{
	register struct fdata *fd, *nfd;

	if(dp->dp_Arg1 == 0){	/* CopyDir of root = root */
		return;
	}

	if( dp->dp_Type == ACTION_COPY_DIR_FH ) { /* FileHandle */
		fd = (struct fdata *)dp->dp_Arg1;
		if(fd==0 || (((u_long)fd)&1) || fd->fd_magic != FD_MAGIC){
			dp->dp_Res2 = ERROR_INVALID_LOCK;
			return;
		}
	} else  /* Lock */
		CHECKLOCK(fd, dp);

	nfd = (struct fdata *)AllocMem(sizeof(*nfd), MEMF_CLEAR|MEMF_PUBLIC);
	if(nfd == (struct fdata *)0){
		dp->dp_Res2 = ERROR_NO_FREE_STORE;
		return;
	}

	*nfd = *fd;
	nfd->fd_head = nfd->fd_next = (struct entry *)NULL;
	mt->mt_nlocks++;
	dp->dp_Res1 = dtob(nfd);
}

/*
**	nfs_freelock (mount_pt mt, struct DosPacket *dp)
**
**	dp->dp_Arg1	BPTR	Lock to free
**
**	dp->dp_Res1	BOOL	DOS_TRUE
**
*/

void
nfs_freelock(mt, dp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
{
	register struct fdata *fd;

	dp->dp_Res1 = DOS_TRUE;
	if(dp->dp_Arg1 == 0){
		return;
	}

	CHECKLOCK(fd, dp);

	mt->mt_nlocks--;
	fd->fd_magic = 0;	/* firewall */
	if(fd->fd_head){
		xdr_free(xdr_my_entry_list, &fd->fd_head);
		fd->fd_head = fd->fd_next = (struct entry *)NULL;
	}

	FreeMem(fd, sizeof(*fd));
}

/*
**	nfs_setprotect (mount_pt mt, struct DosPacket *dp)
**
**	dp->dp_Arg1	unused
**	dp->dp_Arg2	BPTR	Lock to which Arg3 is relative
**	dp->dp_Arg3	BSTR	Name of object (relative to Arg2)
**	dp->dp_Arg4	LONG	Mask of new protection bits
**
**	dp->dp_Res1	BOOL	Success/Failure (DOSTRUE/DOSFALSE)
**	dp->dp_Res2	CODE	Failure code if Res1 is DOSFALSE
**
*/

void
nfs_setprotect(mt, dp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
{
	register struct fdata *fd, tfd;
	char name[NFS_MAXNAMLEN];
	struct sattrargs saa;
	enum clnt_stat err;
	struct attrstat fa;
	int status;

	fd = dp->dp_Arg2==0 ? &mt->mt_lock: btod(dp->dp_Arg2, struct fdata *);
	if(fd->fd_magic != FD_MAGIC){
		dp->dp_Res2 = ERROR_INVALID_LOCK;
		return;
	}

	if(fd->fd_type != FD_DIR){
		dp->dp_Res2 = ERROR_OBJECT_WRONG_TYPE;
		return;
	}

	status = ptofh(mt, dp, fd, btod(dp->dp_Arg3, char *), &tfd, name);
	if(status != ALLFOUND){
		return;
	}

	if(tfd.fd_type == FD_FILE){
		saa.file = tfd.fd_file;
	} else {
		saa.file = tfd.fd_dir;
	}

	err = nfs_call(mt, NFSPROC_GETATTR,xdr_nfs_fh, &saa.file,
						xdr_attrstat, &fa);
	if(err != RPC_SUCCESS || fa.status != NFS_OK){
		dp->dp_Res2 = fh_status(err, fa.status);
		return;
	}

	saa.attributes.uid = -1;
	saa.attributes.gid = -1;
	saa.attributes.size = -1;
	saa.attributes.atime.seconds = -1;
	saa.attributes.atime.useconds = -1;
	saa.attributes.mtime.seconds = -1;
	saa.attributes.mtime.useconds = -1;

	if(dp->dp_Type == ACTION_SET_PROTECT){
		saa.attributes.mode = perm_amiga_to_unix(dp->dp_Arg4);
		saa.attributes.mode |= fa.attrstat_u.attributes.mode & ~07777;
	} else {
		saa.attributes.mode = (fa.attrstat_u.attributes.mode & ~07777)| (dp->dp_Arg4 & 07777);
	}

	err = nfs_call(mt, NFSPROC_SETATTR,	xdr_sattrargs, &saa,
						xdr_attrstat, &fa);
	if(err != RPC_SUCCESS || fa.status != NFS_OK){
		dp->dp_Res2 = fh_status(err, fa.status);
	} else {
		dp->dp_Res1 = DOS_TRUE; dp->dp_Res2 = 0;
	}
}

/*
**	nfs_rename (mount_pt mt, struct DosPacket *dp)
**
**	dp->dp_Arg1	BPTR	Lock that Arg2 is relative to
**	dp->dp_Arg2	BSTR	Name of object to rename (relative to Arg1)
**	dp->dp_Arg3	BPTR	Lock on target directory
**	dp->dp_Arg4	BSTR	Requested new name for object
**
**	dp->dp_Res1	BOOL	Success/Failure (DOSTRUE/DOSFALSE)
**	dp->dp_Res2	CODE	Failure code if Res1 is DOSFALSE
**
*/

void
nfs_rename(mt, dp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
{
	char tname[NFS_MAXNAMLEN], fname[NFS_MAXNAMLEN];
	struct fdata *ffd, ffdb, *tfd, tfdb;
	int status, nfs_status;
	struct renameargs rna;
	struct diropres dor;

	CHECKLOCK(ffd, dp);

	tfd = dp->dp_Arg3==0 ? &mt->mt_lock: btod(dp->dp_Arg3, struct fdata *);
	if(tfd->fd_magic!=FD_MAGIC){
		dp->dp_Res2 = ERROR_INVALID_LOCK;
		return;
	}

   	status = ptofh(mt, dp, ffd, btod(dp->dp_Arg2, char *), &ffdb, fname);
	if(status != ALLFOUND){
		return;
	}

	status = ptofh(mt, dp, tfd, btod(dp->dp_Arg4, char *), &tfdb, tname);
	if(status != LASTNOT){
		return;
	}

	if(tfdb.fd_type != FD_DIR){
		dp->dp_Res2 = ERROR_OBJECT_WRONG_TYPE;
		return;
	}

	if(ffdb.fd_type == FD_DIR){	/* must lookup parent of from dir */
		if(!get_parent(mt, dp, &ffdb.fd_dir, &dor)){
			return;
		}
		rna.from.dir = dor.diropres_u.diropres.file;
	} else {
		rna.from.dir = ffdb.fd_dir;
	}

	rna.from.name = fname;
	rna.to.name = tname;
	rna.to.dir = tfdb.fd_dir;
	status = nfs_call(mt, NFSPROC_RENAME, 	xdr_renameargs, &rna,
					      	xdr_enum, &nfs_status);
	if(status != RPC_SUCCESS || nfs_status != NFS_OK){
		dp->dp_Res2 = fh_status(status, nfs_status);
	} else {
		dp->dp_Res1 = DOS_TRUE;
		dp->dp_Res2 = 0;
	}
}


/*
**	nfs_readlink(mount_pt mt, struct DosPacket *dp)
**
**	dp->dp_Arg1	BPTR	Lock on directory that Arg2 is relative to
**	dp->dp_Arg2	CPTR	Path and name of link relative to Arg1
**	dp->dp_Arg3	APTR	Buffer for new path string
**	dp->dp_Arg4	LONG	Size of buffer in bytes
**
**	dp->dp_Res1	LONG	Actual length of returned string
**				-2 not enough space in buffer
**				-1 other error
**	dp->dp_Res2	CODE	Failure code
**
*/


void
nfs_readlink(mt, dp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
{
	register struct fdata nfd, *fd;
	char name[NFS_MAXNAMLEN];

	CHECKLOCK(fd, dp);


	(void)ptofh(mt, dp, fd, dp->dp_Arg2, &nfd, name);
}

/*
**	nfs_makelink(mount_pt *mt, struct DosPacket *dp)
**
**	dp->dp_Arg1	BPTR	Lock on directory that Arg2 is relative to
**	dp->dp_Arg2	BSTR	Name of link to be created (relative to Arg1)
**	dp->dp_Arg3	CSTR	Target name
**	dp->dp_Arg4	LONG	Mode of link (must be LINK_SOFT)
**
**	dp->dp_Res1	BOOL	Success/Failure (DOSTRUE/DOSFALSE)
**	dp->dp_Res2	CODE	Failure code if Res1 is DOSFALSE
**
*/


void
nfs_makelink(mt, dp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
{
	register struct fdata *fd, nfd;
	char name[NFS_MAXNAMLEN];
	struct symlinkargs sla;
	enum clnt_stat err;
	int status;
        bool_t nfs_status;

	/* if it's not a soft link then go away */
	if(dp->dp_Arg4 != LINK_SOFT) {
		dp->dp_Res2 = ERROR_NOT_IMPLEMENTED;
		return;
	}

	CHECKLOCK(fd,dp);

	/* check to see if Arg1 was really a directory lock */
	if(fd->fd_type != FD_DIR){
		dp->dp_Res2 = ERROR_OBJECT_WRONG_TYPE;
		return;
	}

	status = ptofh(mt, dp, fd, btod(dp->dp_Arg2, char *), &nfd, name);
	if(status != LASTNOT){
// KPrintF("expected LASTNOT\n");
	/* set error codes here */
		return;
	}

	sla.from.dir = nfd.fd_dir;
	sla.from.name = name;
        sla.to = dp->dp_Arg3;
// KPrintF("Creating a link from %s to %s\n",name,sla.to);

	/* attributes are ignored by Unix servers */

	sla.attributes.mode = (~getumask()) & 0777;
	sla.attributes.mode |= NFSMODE_LNK;
	sla.attributes.uid = geteuid();
	sla.attributes.gid = getegid();

	err = nfs_call(mt, NFSPROC_SYMLINK, xdr_symlinkargs, &sla, xdr_enum, &nfs_status);

	if(err != RPC_SUCCESS || nfs_status != NFS_OK){
		dp->dp_Res2 = fh_status(err, nfs_status);
		return;
	}
	dp->dp_Res1 = DOSTRUE;

}


/*
**	nfs_fh_from_lk(mount_pt *mt, struct DosPacket *dp)
**
**	dp->dp_Arg1	BPTR	BPTR to FileHandle
**	dp->dp_Arg2	BPTR	Lock on file to open
**
**	dp->dp_Res1	BOOL	Success/Failure (DOSTRUE/DOSFALSE)
**	dp->dp_Res2	CODE	Failure code if Res1 is DOSFALSE
**
*/


void
nfs_fh_from_lk(mt, dp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
{
	register struct fdata *fd;
	struct FileHandle *fh;

	/* if lock is NULL, then return error */
	if( dp->dp_Arg2 == 0 ) {
		dp->dp_Res2 = ERROR_OBJECT_WRONG_TYPE;
		return;
	}

	fd = btod(dp->dp_Arg2, struct fdata *);

	/* check for valid lock */
	if( fd->fd_magic != FD_MAGIC ) {
		dp->dp_Res2 = ERROR_INVALID_LOCK;
		return;
	}

	/* can't return a FileHandle if our lock isn't a file */
	if(fd->fd_type != FD_FILE){
		dp->dp_Res2 = ERROR_OBJECT_WRONG_TYPE;
		return;
	}

	/* looks like a real lock, so convert it to a filehandle */
	/* this is easy for us because they are really the same structure */
	fd->fd_pos = 0;
	fh = btod(dp->dp_Arg1, struct FileHandle *);
	fh->fh_Arg1 = (LONG)fd;
	dp->dp_Res1 = DOSTRUE;
}


/*
**	nfs_same_lock(mount_pt *mt, struct DosPacket *dp)
**
**	dp->dp_Arg1	BPTR	Lock 1
**	dp->dp_Arg2	BPTR	Lock 2
**
**	dp->dp_Res1	BOOL	DOSTRUE if same, DOSFALSE if different
**	dp->dp_Res2	CODE	Failure code if Res1 is LOCK_DIFFERENT(-1)
**
*/


void
nfs_same_lock(mt, dp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
{
	register struct fdata *fd1, *fd2;
	struct nfs_fh *fh1, *fh2;

	fd1 = dp->dp_Arg1==0 ? &mt->mt_lock : btod(dp->dp_Arg1,struct fdata *);
	fd2 = dp->dp_Arg2==0 ? &mt->mt_lock : btod(dp->dp_Arg2,struct fdata *);
	if( fd1->fd_magic != FD_MAGIC || fd2->fd_magic != FD_MAGIC ) {
		dp->dp_Res1 = DOSFALSE;
		return;
	}

	if( fd1->fd_type == FD_FILE )
		fh1 = &fd1->fd_file;
	else
		fh1 = &fd1->fd_dir;

	if( fd2->fd_type == FD_FILE )
		fh2 = &fd2->fd_file;
	else
		fh2 = &fd2->fd_dir;

	if( memcmp(fh1, fh2, sizeof(nfs_fh)) ) {
		dp->dp_Res1 = DOSFALSE;
		return;
	} else 
		dp->dp_Res1 = DOSTRUE;

}


enum clnt_stat
nfs_call(mt, proc, inproc, in, outproc, out)
	struct mount_pt *mt;
{
	(mt)->mt_nstat.ns_nstat[proc]++;
	return clnt_call((mt)->mt_host, proc, inproc, in, outproc, out, 0L);
}
