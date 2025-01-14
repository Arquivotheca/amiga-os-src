/*
 * Write to file
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <string.h>

#include <rpc/rpc.h>
#include <nfs/nfs.h>

#include "nfsd.h"

char iobuf[NFS_MAXDATA];

nfsstat
change_file_size(fh, tsize, mp)
	BPTR fh;
	unsigned long tsize;
	register mapentry *mp;
{
	unsigned long delta, cnt, fsize;

	/*
	 * Position to end of file and found out how big the file
	 * currently is.  Check that the NFS mapping directory's idea of
	 * size and the current file size actually match up.
	 */
	nfs_seek(fh, 0L, OFFSET_END);
	fsize = nfs_seek(fh, 0L, OFFSET_CURRENT);
	if(fsize == -1){
		return NFSERR_IO;
	}
	if(fsize != mp->f_d.fattr.size){ 	/* hmmm.  not good. */
		mp->f_d.fattr.size = fsize;
		mp->f_d.fattr.blocks = BLOCKS(mp, fsize);
	}

	/*
	 * Calculate the delta size change under consideration.  The Amiga
	 * cannot currently shrink files so delta < 0 is not an option.
	 */
	if(tsize >= mp->f_d.fattr.size){
		delta = tsize - mp->f_d.fattr.size;
	} else {
		if(!SetFileSize(fh,tsize,OFFSET_BEGINNING))
			return NFSERR_IO;
		else {
			mp->f_d.fattr.size = tsize;
			mp->f_d.fattr.blocks = BLOCKS(mp, mp->f_d.fattr.size);
			return NFS_OK;
		}
	}

	bzero(iobuf, sizeof(iobuf));
	while(delta != 0){
		cnt = min(delta, NFS_MAXDATA);
		if(nfs_wr(fh, iobuf, cnt) != cnt){
			return io_to_nfserr();
		}
		delta -= cnt;
		mp->f_d.fattr.size += cnt;
	}
	mp->f_d.fattr.blocks = BLOCKS(mp, mp->f_d.fattr.size);

	return NFS_OK;
}

attrstat *
nfsproc_write_2(argp, rqstp)
	writeargs *argp;
	struct svc_req *rqstp;
{
	long fd, nbytes, change, bsize;
	static attrstat res, zerores;
	mapentry map;

	/*
	 * get file descriptor for this file, fetch map so we can
	 * update it after the write
	 */
	res = zerores;
	if((res.status = get_fd((cookie *)&argp->file, &fd)) != NFS_OK){
		return &res;
	}
	if((res.status = GET_MAP((cookie *)&argp->file, &map)) != NFS_OK){
		return &res;
	}

	if(nfs_seek(fd, argp->offset, OFFSET_BEGINNING) == -1){
		res.status = change_file_size(fd, argp->offset, &map);
		if(res.status != NFS_OK){
			return &res;
		}

		if(nfs_seek(fd, argp->offset, OFFSET_BEGINNING) == -1){
			res.status = NFSERR_IO;
			return &res;
		}
	}

	/*
	 * File is now at position argp->offset, do the write
	 */
	nbytes = nfs_wr(fd, argp->data.data_val, argp->data.data_len);
	if (nbytes < 0) {
		res.status = io_to_nfserr();
		return &res;
	}

	/*
	 * Calculate how much, if any, the file changed in size.
	 * Estimate the number of blocks the change encompassed (we
	 * can't get real number without going to disk).
	 */
	change = argp->offset + nbytes - map.f_d.fattr.size;
	if(change < 0){
		change = 0;
	}
	bsize = map.f_d.fattr.blocksize;
	map.f_d.fattr.size += change;
	map.f_d.fattr.blocks = BLOCKS(&map, map.f_d.fattr.size);

	/*
	 * update file times.  Per the NFS RFC, writes to the
	 * file update mtime/atime.  If the file changes size
	 * then ctime is updated also.
	 */
	NFStime(&map.f_d.fattr.mtime);
	map.f_d.fattr.atime = map.f_d.fattr.mtime;
	if(change){
		map.f_d.fattr.atime = map.f_d.fattr.mtime;
	}
	if((res.status = update_map(&map))!= NFS_OK){
		return &res;
	}

	res.attrstat_u.attributes = map.f_d.fattr;
	return &res;
}
