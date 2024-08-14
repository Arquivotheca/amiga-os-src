/* -----------------------------------------------------------------------
 * examine.c  nfsc  SAS
 *
 * $Locker:  $
 *
 * $Id: examine.c,v 1.5 92/10/07 15:14:12 bj Exp $
 *
 * $Revision: 1.5 $
 *
 * $Header: AS225:src/c/nfsc/RCS/examine.c,v 1.5 92/10/07 15:14:12 bj Exp $
 *
 *------------------------------------------------------------------------
 */



/*
 * nfs_examine()
 * nfs_examinenext()
 */

#include "fs.h"
#include <string.h>

#define ZEROCOOKIE(c) bzero(&(c), NFS_COOKIESIZE);

/* local functions */
static entry *get_entry_list(struct mount_pt *mt, register struct DosPacket *dp,
	nfs_fh *dirp, nfscookie pos, bool_t *eofp);
static void makefib(struct mount_pt *mt,struct DosPacket *dp,
	register struct FileInfoBlock *fib, struct entry *rl, 
	register nfs_fh *fh);
static void makerootfib(
	struct mount_pt *mt,
	struct DosPacket *dp,
	register struct FileInfoBlock *fib,
	fattr *attr);

/*
 * Return list of directory entries for given directory.
 */
static entry *
get_entry_list(mt, dp, dirp, pos, eofp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
	nfs_fh *dirp;
	nfscookie pos;
	bool_t *eofp;
{
	struct readdirargs ra;
	struct readdirres rr;
	enum clnt_stat err;

	*eofp = TRUE;
	ra.count = mt->mt_rdsize;
	ra.dir = *dirp;
	ra.cookie = pos;

	bzero(&rr, sizeof(rr));
	err = nfs_call(mt, NFSPROC_READDIR, 	xdr_readdirargs, &ra,
						xdr_my_readdirres, &rr);
	if(err != RPC_SUCCESS || rr.status != NFS_OK){
		dp->dp_Res2 = fh_status(err, rr.status);
		return (struct entry *)NULL;
	}

	*eofp = rr.readdirres_u.reply.eof;
	return rr.readdirres_u.reply.entries;
}

/*
**	nfs_examine(mount_pt *mt, struct DosPacket *dp, int type)
**
**	ACTION_EXAMINE_OBJECT
**
**	dp->dp_Arg1	LOCK	Lock of object to examine
**	dp->dp_Arg2	BPTR	FileInfoBlock to fill in
**
**	dp->dp_Res1	BOOL	Success/Failure (DOSTRUE/DOSFALSE)
**	dp->dp_Res2	CODE	Failure code if Res1 is DOSFALSE
**
**	ACTION_EXAMINE_FH
**
**	dp->dp_Arg1	BPTR	FileHandle on open file
**
**  Examine searchs the parent directory of the object described by the given
**  lock.  For files, the parent directory is kept in the lock.  When we
**  examine a directory we always lookup ".." as the search dir - this method
**  is more robust in the event the directory tree has been mv'ed.  An
**  Examine() on a file can fail if the file was moved after it was created
**  and nfs.handlers idea of the parent directory is no longer correct.  In
**  such a situation NFSERR_STALE is returned.
*/

void
nfs_examine(mt, dp)
	struct mount_pt  *mt;
	struct DosPacket *dp;
{
	register struct entry *rl;
	register struct fdata *fd;
	struct FileInfoBlock *fib;
	char name[NFS_MAXNAMLEN];
	struct entry *head, trl;
	struct diropres dor;
	register long fid;
	bool_t found = FALSE;
	bool_t eof;
	nfscookie pos;
	nfs_fh dir;

	if( dp->dp_Type == ACTION_EXAMINE_FH ) { /* FileHandle */
		fd = (struct fdata *)dp->dp_Arg1;
		if(fd==0 || (((u_long)fd)&1) || fd->fd_magic != FD_MAGIC){
			dp->dp_Res2 = ERROR_INVALID_LOCK;
			return;
		}
	} else  /* Lock */
		CHECKLOCK(fd, dp);

	if (fd->fd_type == FD_LINK) {
		dp->dp_Res2 = ERROR_IS_SOFT_LINK;
		return;
	}

	fib = btod(dp->dp_Arg2, struct FileInfoBlock *);
	/*
	 * Examine resets any directory scans in process
	 */
	ZEROCOOKIE(fd->fd_dirpos);
	if(fd->fd_head){
		xdr_free(xdr_my_entry_list, &fd->fd_head);
		fd->fd_head = fd->fd_next = (struct entry *)NULL;
	}

	if(fd->fd_type == FD_DIR){
		if(fd->fd_dir_fid == mt->mt_lock.fd_dir_fid){ /* root */
			struct attrstat fa;
			enum clnt_stat err;

			err = nfs_call(mt, NFSPROC_GETATTR,
						xdr_nfs_fh, &mt->mt_lock.fd_dir,
						xdr_attrstat, &fa);
			if(err != RPC_SUCCESS || fa.status != NFS_OK){
				dp->dp_Res2 = fh_status(err, fa.status);

				return ;
			}

			makerootfib(mt, dp, fib, &fa.attrstat_u.attributes);
			return;
		} if(!get_parent(mt, dp, &fd->fd_dir, &dor)){

			return;
		}


		fid = fd->fd_dir_fid;
		dir = dor.diropres_u.diropres.file;
	} else {
		fid = fd->fd_file_fid;
		dir = fd->fd_dir;
	}

	/*
	 * Search owning directory for object with fileid that matches
	 * the one we're looking for.  This can fail under certain
	 * conditions, eg file movement after creation, not having
	 * read permission on the searched directory, etc.
	 */
	pos = fd->fd_dirpos;
	do {
		head = get_entry_list(mt, dp, &dir, pos, &eof);

		for(rl = head; rl != (struct entry *)NULL; rl = rl->nextentry){

			if(fid == rl->fileid){
				strcpy(name, rl->name);
				trl = *rl; trl.name = name;
				found = TRUE;
				break;
			}
			pos = rl->cookie;
		}


		if(head){
			xdr_free(xdr_my_entry_list, &head);
		}
	} while (!found && !eof);


	if(found){
		makefib(mt, dp, fib, &trl, &dir);
	} else {
		dp->dp_Res2 = (fd->fd_type == FD_FILE) ? NFSERR_STALE:ERROR_OBJECT_NOT_FOUND;
	}

}

void
nfs_examine_next(mt, dp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
{
	struct FileInfoBlock *fib;
	register struct entry *rl;
	register struct fdata *fd;
	nfscookie pos;
	bool_t eof;
	int got_one;

	CHECKDIR(fd, dp);
	got_one = 0;
	pos = fd->fd_dirpos;

	do {
		if(rl = fd->fd_next){
			fd->fd_next = rl->nextentry;
		} else {
			if(fd->fd_head){
				xdr_free(xdr_my_entry_list, &fd->fd_head);
				fd->fd_head = fd->fd_next = (struct entry *)NULL;
			}

			rl = get_entry_list(mt, dp, &fd->fd_dir, pos, &eof);
			if(rl == NULL && eof){ /* test for end of directory */
				break;
			}

			fd->fd_head = rl;
			fd->fd_next = rl ? rl->nextentry : (struct entry *)NULL;
		}

		if(rl == (struct entry *)NULL){
			break;
		}

		if(rl->name[0] == '.'){
			if(rl->name[1] == 0){
				pos = rl->cookie;
				continue;
			}

			if(rl->name[1] == '.' && rl->name[2] == 0){
				pos = rl->cookie;
				continue;
			}
		}

		got_one++;

	} while(!got_one);

	fib = btod(dp->dp_Arg2, struct FileInfoBlock *);
	if(got_one){
		makefib(mt, dp, fib, rl, &fd->fd_dir);
		fd->fd_dirpos = rl->cookie;
	} else {
		fib->fib_FileName[0] = fib->fib_Comment[0] = 0;
		dp->dp_Res2 = ERROR_NO_MORE_ENTRIES;
	}

}

/*
 * Fill in FileInfoBlock to reflect the volume name.  This is used
 * when an application does an Examine() operation on the volume node.
 */
static void makerootfib(mt, dp, fib, attr)
	struct mount_pt *mt;
	struct DosPacket *dp;
	register struct FileInfoBlock *fib;
	fattr *attr;
{
	unsigned char *cp;

	cp = btod(mt->mt_vol->dol_Name, unsigned char *);
	bcopy(cp, fib->fib_FileName, cp[0]+1);
	fib->fib_FileName[cp[0]+1] = 0;
	fib->fib_Comment[0] = 0;
	fib->fib_DirEntryType = fib->fib_EntryType = 2;
	fib->fib_DiskKey = attr->fileid;
	fib->fib_Size = fib->fib_NumBlocks = 0;

	if(dp->dp_Type == ACTION_EX_OBJECT){
		*((struct fattr *)dp->dp_Arg3) = *attr;
	}

	dp->dp_Res1 = DOS_TRUE;
}

/*
 * Fill in <fib> using information in <rl>.  Must do lookup in <fh> to find
 * file size and date information.  Unix dates are relative to Jan 1st, 1970,
 * while AmigaDOS dates are relative to Jan 1st, 1978; must translate by
 * difference in seconds.  Date returned for file is modification time.
 */
#define MIN_PER_DAY	(24*60)
#define SEC_PER_DAY	(MIN_PER_DAY*60)
#define UNIX_DELTAT	(((1978-1970)*365 + 2 /* 2 leap years */)*SEC_PER_DAY)


static void makefib(mt, dp, fib, rl, fh)
	struct mount_pt *mt;
	struct DosPacket *dp;
	register struct FileInfoBlock *fib;
	struct entry *rl;
	register nfs_fh *fh;
{
	register u_long amiga_secs, prot;
	int len, tz_min_off;
	struct diropargs di;
	struct diropres dor;
	enum clnt_stat err;
	long mode;

	len = min(strlen(rl->name),106);
	strncpy(&fib->fib_FileName[1], rl->name,len);
	fib->fib_FileName[len+1] = 0;
	fib->fib_FileName[0] = len;
	fib->fib_Comment[0] = 0;
	fib->fib_DiskKey = rl->fileid;
	di.name = rl->name; di.dir = *fh;
//KPrintF("makefib: lookup %s\n",di.name);
	err = nfs_call(mt, NFSPROC_LOOKUP, 	xdr_diropargs, &di,
						xdr_diropres, &dor);
	if(err != RPC_SUCCESS || dor.status != NFS_OK){
//KPrintF("err=%ld status=%ld\n",err,dor.status);
		dp->dp_Res2 = fh_status(err, dor.status);
		return;
	}
	if(dp->dp_Type==ACTION_EX_OBJECT || dp->dp_Type==ACTION_EX_NEXT){
		*((struct fattr *)dp->dp_Arg3) = dor.diropres_u.diropres.attributes;
	}

	mode = dor.diropres_u.diropres.attributes.mode;
//KPrintF("mode = x%lx\n",mode);
	if ((mode & NFSMODE_LNK) == NFSMODE_LNK ) fib->fib_DirEntryType = ST_SOFTLINK;
		else if ((mode & NFSMODE_REG) == NFSMODE_REG ) fib->fib_DirEntryType = ST_FILE;
		else if ((mode & NFSMODE_DIR) == NFSMODE_DIR ) fib->fib_DirEntryType = ST_USERDIR;
//KPrintF("fib_DirEntryType=%ld\n",fib->fib_DirEntryType);
	fib->fib_EntryType = fib->fib_DirEntryType;
	if (fib->fib_DirEntryType < 0){
		fib->fib_Size = dor.diropres_u.diropres.attributes.size;
		fib->fib_NumBlocks = dor.diropres_u.diropres.attributes.blocks;
	} else {
		fib->fib_Size = fib->fib_NumBlocks = 0;
	}

	tz_min_off = get_tz();
	amiga_secs = (dor.diropres_u.diropres.attributes.mtime.seconds -
				tz_min_off * 60) - UNIX_DELTAT;
	fib->fib_Date.ds_Days = amiga_secs / SEC_PER_DAY;
	fib->fib_Date.ds_Minute = (amiga_secs % SEC_PER_DAY)/60;
	fib->fib_Date.ds_Tick = (amiga_secs % 60)*TICKS_PER_SECOND;

#define UID dor.diropres_u.diropres.attributes.uid
#define GID dor.diropres_u.diropres.attributes.gid
#define MODE dor.diropres_u.diropres.attributes.mode
	prot = perm_unix_to_amiga(UID, GID, MODE);
#undef UID
#undef GID
#undef MODE

	fib->fib_Protection = prot;

	dp->dp_Res1 = DOS_TRUE;	dp->dp_Res2 = 0;
}

/*
 * SetDate(Arg1:Unused, Arg2: Lock, Arg3: Name, Arg4 :(struct DateStamp *))
 * 	Update last modified (mtime) on UNIX object
 */

void
nfs_setdate(mt,dp)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
{
	char name[NFS_MAXNAMLEN];
	int status, tz_min_off;
	struct fdata *fd, tfd;
	struct DateStamp *dst;
	struct sattrargs saa;
	struct attrstat fa;
	enum clnt_stat err;
	long amiga_secs;

	fd = dp->dp_Arg2==0 ? &mt->mt_lock : btod(dp->dp_Arg2, struct fdata *);
	if (fd->fd_magic != FD_MAGIC || fd->fd_type != FD_DIR) {
		dp->dp_Res2 = ERROR_INVALID_LOCK;
		return;
	}

	status = ptofh(mt, dp, fd, btod(dp->dp_Arg3, char *), &tfd, name);
	if (status != ALLFOUND){
		return;
	}

	if(tfd.fd_type == FD_FILE){
		saa.file = tfd.fd_file;
	} else {
		saa.file = tfd.fd_dir;
	}

	saa.attributes.uid = -1;
	saa.attributes.gid = -1;
	saa.attributes.size = -1;
	saa.attributes.atime.seconds = -1;
	saa.attributes.atime.useconds = -1;
	saa.attributes.mode = -1;

	tz_min_off = get_tz();
	dst = (struct DateStamp *) dp->dp_Arg4;

	amiga_secs = dst->ds_Days * SEC_PER_DAY	+ dst->ds_Minute * 60
				+ dst->ds_Tick / TICKS_PER_SECOND;
	saa.attributes.mtime.seconds = amiga_secs + UNIX_DELTAT + tz_min_off*60;
	saa.attributes.mtime.useconds = -1;

	err = nfs_call(mt, NFSPROC_SETATTR, 	xdr_sattrargs, &saa,
						 xdr_attrstat, &fa);
	if(err != RPC_SUCCESS || fa.status != NFS_OK){
		dp->dp_Res2 = fh_status(err, fa.status);
	} else {
		dp->dp_Res1 = DOS_TRUE;
		dp->dp_Res2 = 0;
	}
}
