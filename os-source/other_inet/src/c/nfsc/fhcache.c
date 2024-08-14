/* -----------------------------------------------------------------------
 * fhcache.c  nfsc  SAS
 *
 * $Locker:  $
 *
 * $Id: fhcache.c,v 1.4 92/10/07 15:21:34 bj Exp $
 *
 * $Revision: 1.4 $
 *
 * $Header: AS225:src/c/nfsc/RCS/fhcache.c,v 1.4 92/10/07 15:21:34 bj Exp $
 *
 *------------------------------------------------------------------------
 */



/*
 * Routines which take an Amiga filehandle as an argument.
 *
 * nfs_open()
 * nfs_close()
 * nfs_rd()
 * nfs_wr()
 * nfs_seek()
 * nfs_dkinfo()
 * nfs_setsize()
 */

#include "fs.h"
#include "fhcache.h"
#include <string.h>

/* local functions */
static struct fh_cache *get_cache_blk(struct mount_pt *mt);
static void free_cache_blk(struct mount_pt *mt, struct fh_cache *ca);
static void real_read(struct mount_pt *mt, struct fdata *fd, u_long offset,
	char *bufptr, u_long len, u_long *bytes_read, long *failcode);
static void ca_read(struct mount_pt *mt, struct fdata *fd, u_long offset,
	u_char *bufptr, long len, long *bytes_read);

static struct fh_cache *
get_cache_blk(mt)
	struct mount_pt *mt;
{
	return (struct fh_cache *)calloc(1, sizeof(struct fh_cache)+mt->mt_rdsize);
}

static void
free_cache_blk(mt, ca)
	struct mount_pt *mt;
	struct fh_cache *ca;
{
	free(ca);
}

/*
 * Open(FileHandle: BPTR; Lock: BPTR; Name: BSTR): Boolean;
 *
 * Allocate and initialize fdata structure, fill in Amiga FileHandle
 * structure.  Functional table:
 *
 *			File exists	File not exist
 *			-----------	--------------
 *	MODE_OLDFILE:	Open it.	return 0.		(read only)
 *	MODE_NEWFILE:	Truncate to 0.	create length 0 file.	(r/w, trunc)
 *	1004:		Open it.	create length 0 file.	(r/w, no trunc)
 *
 */
void nfs_open(mt, dp)
	struct mount_pt *mt;
	register struct DosPacket *dp;
{
	struct fdata *ofd, *fd, nfd;
	char name[NFS_MAXNAMLEN];
	struct FileHandle *fh;
	struct diropres dor;
	enum clnt_stat err;
	int status;
	long mask;

	ofd = dp->dp_Arg2==0 ? &mt->mt_lock:btod(dp->dp_Arg2, struct fdata *);
	status = ptofh(mt, dp, ofd, btod(dp->dp_Arg3, char *), &nfd, name);

	if(status == ALLFOUND && nfd.fd_type != FD_FILE){
		if(nfd.fd_type==FD_LINK)
			dp->dp_Res2 = ERROR_IS_SOFT_LINK;
		else
			dp->dp_Res2 = ERROR_OBJECT_WRONG_TYPE;
		return;
	}

	if(dp->dp_Type==MODE_NEWFILE || dp->dp_Type==MODE_READWRITE){

		if (status == ALLFOUND && dp->dp_Type == MODE_READWRITE){
			dp->dp_Res2 = 0;
		} else if(status == ALLFOUND || status == LASTNOT){
			struct createargs ca;

			mask = getumask();
			ca.attributes.mode = (~mask) & 0777;
			ca.attributes.mode |= NFSMODE_REG; /* regular file */
			ca.where.name = name;
			ca.where.dir = nfd.fd_dir;
			ca.attributes.uid = geteuid();
			ca.attributes.gid = getegid();
			ca.attributes.size = 0;
			ca.attributes.atime.seconds =
			ca.attributes.atime.useconds =
			ca.attributes.mtime.seconds =
			ca.attributes.mtime.useconds = -1;

			err = nfs_call(mt, NFSPROC_CREATE,
						xdr_createargs, &ca,
						xdr_diropres, &dor);
			if(err != RPC_SUCCESS || dor.status != NFS_OK){
				dp->dp_Res2 = fh_status(err, dor.status);
				return;
			}

			nfd.fd_file = dor.diropres_u.diropres.file;
			nfd.fd_type = FD_FILE;
		} else {
			return;
		}
	} else
		if(status != ALLFOUND) return;

	/*
	 * Now that we're sure that we have opened the file, open
	 * an internal file descriptor for the file.
	 */
	fd = (struct fdata *)AllocMem(sizeof(*fd), MEMF_CLEAR|MEMF_PUBLIC);
	if(fd == (struct fdata *)0){
		dp->dp_Res2 = ERROR_NO_FREE_STORE;
		return;
	}
	*fd = nfd;
	fd->fd_magic = FD_MAGIC;
	fd->fd_flags |= FD_FHANDLE;
	fd->fd_pos = 0;
	fh = btod(dp->dp_Arg1, struct FileHandle *);
	fh->fh_Arg1 = (u_long)fd;
	dp->dp_Res1 = DOS_TRUE;
	dp->dp_Res2 = 0;
	mt->mt_nfiles++;
}

/*
 * Close(FileHandle_Arg1: BPTR): Boolean;
 *
 * Deallocate fdata structure.
 */
void
nfs_close(mt, dp)
	struct mount_pt *mt;
	register struct DosPacket *dp;
{
	struct fdata *fd;

	fd = (struct fdata *)dp->dp_Arg1;
	if(fd==0 || (((u_long)fd)&1) || fd->fd_magic != FD_MAGIC){
		dp->dp_Res2 = ERROR_INVALID_LOCK;
		return ;
	}

	if (fd->fd_cache != (struct fh_cache *) NULL) {
		free_cache_blk(mt, fd->fd_cache);
		fd->fd_cache = (struct fh_cache *) NULL;
	}
	fd->fd_magic = 0;
	FreeMem(fd, sizeof(*fd));
	dp->dp_Res1 = DOS_TRUE;
	mt->mt_nfiles-- ;
}

/*
 * Read(FileHandle_Arg1: BPTR; Buffer: caddr_t; Length: LONG): LONG;
 *
 * Do a read on file handle.
 */
static void
real_read(mt, fd, offset, bufptr, len, bytes_read, failcode)
	struct mount_pt *mt;
	struct fdata *fd;
	char *bufptr;
	unsigned long offset, len, *bytes_read;
	long *failcode;
{
	register long total, udata, cdata;
	struct readargs ra;
	enum clnt_stat err;
	struct readres rr;

	*bytes_read = *failcode = 0;
	rr.readres_u.reply.data.data_val = bufptr;
	rr.readres_u.reply.data.data_len = len;
	ra.file = fd->fd_file; 	ra.totalcount = 0; ra.offset = offset;

	for(total = 0; total < len;){
		udata = ra.count = MIN(len - total, mt->mt_rdsize);
		err = nfs_call(mt, NFSPROC_READ,
						xdr_readargs, &ra,
						xdr_readres, &rr);
		if(err != RPC_SUCCESS || rr.status != NFS_OK){
			*failcode = fh_status(err, rr.status);
			return;
		}
		if(rr.readres_u.reply.data.data_len < udata){
			udata = rr.readres_u.reply.data.data_len;
			cdata = 0;
		} else {
			cdata = rr.readres_u.reply.data.data_len - udata;
		}

		total += udata;
		if(rr.readres_u.reply.data.data_len < ra.count){
			break;
		}
		rr.readres_u.reply.data.data_val += udata;
		ra.offset += udata;
	}
	*bytes_read = total;
}

static void
ca_read(mt, fd, offset, bufptr, len, bytes_read)
	struct mount_pt *mt;
	struct fdata *fd;
	unsigned long offset;
	unsigned char *bufptr;
	long len, *bytes_read;
{
	long ubegin, uend, cbegin, cend, cnb, coff;
	struct fh_cache	*ca;

	*bytes_read = 0;
	if (len <= 0){
		return;
	}
	if(!fd->fd_cache){
		if(len > mt->mt_rdsize >> 1){
			return;
		}
		ca = get_cache_blk(mt);
		if(!ca){
			return;
		}
		fd->fd_cache = ca;
		ca->ca_valid = FALSE;
		ca->ca_offset = 0;
		ca->ca_nbytes = 0;
		return;
	} else {
		ca = fd->fd_cache;
	}

	ubegin = offset;
	uend = ubegin + len - 1;
	cbegin = ca->ca_offset;
	cend = ca->ca_offset + ca->ca_nbytes - 1;

	/*
	 * cache first byte <= user first byte <= cache end
	 * user last byte <= cache last byte OR
	 * user last byte > cache end due to runt read
	 */
	if (ca->ca_valid && ubegin >= cbegin && ubegin <= cend
	   	&& (uend <= cend || ca->ca_nbytes < mt->mt_rdsize)
	   ){
		ca->ca_hits += 1;
		coff = ubegin - cbegin;

		cnb = MIN(len, cend - ubegin + 1);

		bcopy( &ca->ca_data[coff], bufptr, cnb);

		*bytes_read = cnb;
		return;
	}
}

void
nfs_rd(mt, dp)
	struct mount_pt *mt;
	register struct DosPacket *dp;
{
	long length, fromcache, failcode, bytes_read;
	struct fh_cache *ca;
	unsigned char *uptr;
	struct fdata *fd;

	fd = (struct fdata *)dp->dp_Arg1;
	dp->dp_Res1 = -1;
	if(fd==0 || (((u_long)fd)&1) || fd->fd_magic != FD_MAGIC){
		dp->dp_Res2 = ERROR_INVALID_LOCK;
		return;
	}
	if(fd->fd_type != FD_FILE){
		dp->dp_Res2 = ERROR_OBJECT_WRONG_TYPE;
		return;
	}

	uptr = (unsigned char *)dp->dp_Arg2;
	if(!(length = dp->dp_Arg3)){
		dp->dp_Res1 = 0;
		return;
	}
	if(fd->fd_cache){
		ca = fd->fd_cache;
		ca->ca_nreads += 1;
	}

	/*
	 * if hit on cache, just return bytes
	 */
	ca_read(mt, fd, fd->fd_pos, uptr, length, &bytes_read);
	if (bytes_read) {
		dp->dp_Res1 = bytes_read;
		fd->fd_pos += bytes_read;
		return;
	}

	/*
	 * reaching this point implies a total miss on cache
	 * if cache exists and read length is small fill the cache
	 * otherwise read data directly into users buffer
	 */
	if (length <= mt->mt_rdsize >> 1 && fd->fd_cache != (struct fh_cache *)NULL){
		fromcache = TRUE;
		ca = fd->fd_cache;
		real_read(mt, fd, fd->fd_pos, ca->ca_data, mt->mt_rdsize,
						&bytes_read, &failcode);
		if (bytes_read) {
			ca->ca_offset = fd->fd_pos;	/* update */
			ca->ca_valid = TRUE;		/* cache */
			ca->ca_nbytes = bytes_read;	/* status */
		}
	} else {
		fromcache = FALSE;
		real_read (mt, fd, fd->fd_pos, uptr, length, &bytes_read, &failcode);
	}

	if (failcode) {
		dp->dp_Res2 = failcode;
		return;
	}

	if (fromcache){
		ca_read(mt, fd, fd->fd_pos, uptr, length, &bytes_read);
	}
	fd->fd_pos += bytes_read;
	dp->dp_Res1 = bytes_read;
}

/*
 * Write(FileHandle_Arg1: BPTR; Buffer: caddr_t; Length: LONG): LONG;
 *
 * Do a write on file handle.
 */
void
nfs_wr(mt, dp)
	struct mount_pt *mt;
	register struct DosPacket *dp;
{
	register struct fdata *fd;
	register u_long	cnt, len;
	struct writeargs wa;
	struct attrstat wr;
	enum clnt_stat err;

	fd = (struct fdata *)dp->dp_Arg1;
	dp->dp_Res1 = -1;
	if(fd==0 || (((u_long)fd)&1) || fd->fd_magic != FD_MAGIC){
		dp->dp_Res2 = ERROR_INVALID_LOCK;
		return;
	}
	if(fd->fd_type != FD_FILE){
		dp->dp_Res2 = ERROR_OBJECT_WRONG_TYPE;
		return;
	}

	/*
	 * invalidate read cache on writes
	 */
	if (fd->fd_cache != (struct fh_cache *)NULL ) {
		struct fh_cache *ca;

		ca = fd->fd_cache;
		ca->ca_valid = FALSE;
	}

	wa.file = fd->fd_file;
	wa.offset = fd->fd_pos;
	wa.beginoffset = wa.totalcount = 0;
	wa.data.data_val = (char *)dp->dp_Arg2;
	len = dp->dp_Arg3;
	for(cnt = 0; cnt < len; cnt += wa.data.data_len){
		wa.data.data_len = MIN(len - cnt, mt->mt_wrsize);
		err = nfs_call(mt, NFSPROC_WRITE,
						xdr_writeargs, &wa,
						xdr_attrstat, &wr);
		if(err != RPC_SUCCESS || wr.status!= NFS_OK){
			dp->dp_Res2 = fh_status(err, wr.status);
			return;
		}
		wa.data.data_val += wa.data.data_len;
		wa.offset += wa.data.data_len;
	}
	fd->fd_pos += len;
	dp->dp_Res1 = len;
}

/*
 * Seek(FileHandle_Arg1: BPTR;  Position: LONG;  Mode: LONG): LONG;
 *
 * do a seek on filehandle.  Does a getattr to get most recent copy of
 * fattr, then updates position after checking to ensure the user didn't
 * try to seek past EOF.  Does some sanity checking: struct fdata MAGIC number,
 * object is regular file, and BOF/EOF check.
 */
void
nfs_seek(mt, dp)
	struct mount_pt *mt;
	register struct DosPacket *dp;
{
	register long oldpos, newpos;
	register struct fdata *fd;
	struct attrstat fa;
	enum clnt_stat err;

	fd = (struct fdata *)dp->dp_Arg1;
	dp->dp_Res1 = -1;
	if(fd==0 || (((u_long)fd)&1) || fd->fd_magic != FD_MAGIC){
		dp->dp_Res2 = ERROR_INVALID_LOCK;
		return;
	}
	oldpos = fd->fd_pos;
	err = nfs_call(mt, NFSPROC_GETATTR,  	xdr_nfs_fh,  &fd->fd_file,
					     	xdr_attrstat, &fa);
	if(err != RPC_SUCCESS || fa.status != NFS_OK){
		dp->dp_Res2 = fh_status(err, fa.status);
		return;
	}
	if((fa.attrstat_u.attributes.mode & NFSMODE_REG) != NFSMODE_REG){
		dp->dp_Res2 = ERROR_OBJECT_WRONG_TYPE;
		return;
	}
	newpos = dp->dp_Arg2;
	switch (dp->dp_Arg3){
	case OFFSET_CURRENT:
		newpos += oldpos;
		break;

	case OFFSET_BEGINNING:
		break;

	case OFFSET_END:
		newpos += fa.attrstat_u.attributes.size;
		break;

	default:
		dp->dp_Res2 = ERROR_ACTION_NOT_KNOWN;
		return;
	}
	if(newpos > fa.attrstat_u.attributes.size || newpos < 0){
		dp->dp_Res2 = ERROR_SEEK_ERROR;
		return;
	}
	dp->dp_Res1 = oldpos;
	dp->dp_Res2 = 0;
	fd->fd_pos = newpos;
}

/*
 * Get remote "disk"/volume info.
 */
void
nfs_dkinfo(mt, dp)
	register struct mount_pt *mt;
	register struct DosPacket *dp;
{
	register struct InfoData *id;
	register struct fdata *fd;
	struct statfsres fs;
	register long mult;
	enum clnt_stat err;
	nfs_fh *obj;

	if(dp->dp_Type == ACTION_DISK_INFO){
		obj = &mt->mt_lock.fd_dir;
		id = btod(dp->dp_Arg1, struct InfoData *);
	} else {
		fd = dp->dp_Arg1==0 ? &mt->mt_lock :
					btod(dp->dp_Arg1, struct fdata *);
		obj = &fd->fd_dir;
		id = btod(dp->dp_Arg2, struct InfoData *);
	}
	err = nfs_call(mt, NFSPROC_STATFS, 	xdr_nfs_fh, obj,
						xdr_statfsres, &fs);
	if(err != RPC_SUCCESS || fs.status != NFS_OK){
		dp->dp_Res1 = FALSE;
		dp->dp_Res2 = fh_status(err, fs.status);
		return;
	}
	id->id_NumSoftErrors = 0;
	id->id_UnitNumber = 0;
	id->id_DiskState = ID_VALIDATED;
	id->id_DiskType = ID_DOS_DISK;
	id->id_VolumeNode = dtob(mt->mt_vol);
	id->id_InUse = 0;
	mult = fs.statfsres_u.reply.bsize/512;
	if (mult == 0){
		mult = 1;
	}
	id->id_BytesPerBlock = 512;
	id->id_NumBlocks = fs.statfsres_u.reply.blocks*mult;
	id->id_NumBlocksUsed =
	     (fs.statfsres_u.reply.blocks - fs.statfsres_u.reply.bavail)*mult;

	dp->dp_Res1 = DOS_TRUE;
	dp->dp_Res2 = 0;
}


/*
**	nfs_setsize (mount_pt mt, struct DosPacket *dp)
**
**	dp->dp_Arg1	BPTR	FileHandle to modify
**	dp->dp_Arg2	LONG	new end of file location based on mode
**	dp->dp_Arg3	LONG	mode
**
**	dp->dp_Res1	LONG	new size on success, -1 on failure
**	dp->dp_Res2	CODE	Failure code if Res1 == -1
**
**	This function set expands or truncates a file.
*/

void
nfs_setsize(mt, dp, type)
	struct mount_pt  *mt;
	register struct DosPacket *dp;
{
	register struct fdata *fd;
        struct sattrargs saa;
	struct attrstat fa;
	enum clnt_stat err;
	u_long newsize;

	/* on error, return -1 */
	dp->dp_Res1 = -1;

	fd = (struct fdata *)dp->dp_Arg1;
	if(fd==0 || (((u_long)fd)&1) || fd->fd_magic != FD_MAGIC){
		dp->dp_Res2 = ERROR_INVALID_LOCK;
		return;
	}

	if(fd->fd_type!=FD_FILE) {		
		/* couldn't happen? */
		dp->dp_Res2 = ERROR_OBJECT_WRONG_TYPE;
		return;
	}

	switch (dp->dp_Arg3) {
	case OFFSET_CURRENT:
		newsize = dp->dp_Arg2 + fd->fd_pos;
		break;
	case OFFSET_BEGINNING:
		newsize = dp->dp_Arg2;
		break;
	case OFFSET_END:
		/* first we need to get the current size */
		err = nfs_call(mt, NFSPROC_GETATTR, xdr_nfs_fh,
			&fd->fd_file, xdr_attrstat, &fa);
		if(err != RPC_SUCCESS || fa.status != NFS_OK) {
			dp->dp_Res2 = fh_status(err,fa.status);
			return;
		}
		newsize = dp->dp_Arg2 + fa.attrstat_u.attributes.size;
		break;
	default:
		dp->dp_Res2 = ERROR_ACTION_NOT_KNOWN;
		return;
	}

	/* Fill in attributes struct with new size */
	/* Fields which we don't want changed get set to -1 */
	saa.file = fd->fd_file;		/* nfs filehandle */
	saa.attributes.mode = -1;	
	saa.attributes.uid = -1;	
	saa.attributes.gid = -1;	
	saa.attributes.size = newsize;	
	saa.attributes.atime.seconds = -1;	
	saa.attributes.atime.useconds = -1;	
	saa.attributes.mtime.seconds = -1;	
	saa.attributes.mtime.useconds = -1;	

	err = nfs_call(mt, NFSPROC_SETATTR, xdr_sattrargs, &saa,
		xdr_attrstat, &fa);
	if(err != RPC_SUCCESS || fa.status != NFS_OK) 
		dp->dp_Res2 = fh_status(err,fa.status);
	else
		dp->dp_Res1 = newsize;
	
}	
	





