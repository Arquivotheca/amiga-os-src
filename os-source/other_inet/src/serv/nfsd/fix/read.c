/*
 * Read from file
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

#include <rpc/rpc.h>
#include <nfs/nfs.h>

#include "nfsd.h"

extern char iobuf[NFS_MAXDATA];

readres *
nfsproc_read_2(argp, rqstp)
	readargs *argp;
	struct svc_req *rqstp;
{
	long fd, oldpos, fsize, nbytes;
	static readres res, oldres;
	mapentry map;

	res = oldres;
	if((res.status = get_fd(&argp->file, &fd)) != NFS_OK){
		return &res;
	}
	if((res.status = GET_MAP(&argp->file, &map)) != NFS_OK){
		return &res;
	}

	if(nfs_seek(fd, 0L, OFFSET_CURRENT) != argp->offset){
		oldpos = nfs_seek(fd, argp->offset, OFFSET_BEGINNING);
		if(oldpos == -1){
			/*
			 * Check if the seek failed because the file is
			 * smaller than argp->offset.  If so, return a zero
			 * datalen.  If not, then we have some kind of I/O
			 * error that killed the seek.
			 */
			fsize = nfs_seek(fd, 0, OFFSET_END);
			if(fsize == -1){
				res.status = NFSERR_IO;
				return &res;
			}
			if(fsize < argp->offset) {
				res.readres_u.reply.data.data_len = 0;
				res.readres_u.reply.attributes = map.f_d.fattr;
			} else {
				res.status = NFSERR_IO;
			}
			return &res;
		}
	}

	res.readres_u.reply.data.data_val = iobuf;
	nbytes = (argp->count > NFS_MAXDATA)? NFS_MAXDATA:argp->count;
	nbytes = nfs_rd(fd, iobuf, nbytes);
	if(nbytes < 0) {
		res.status = NFSERR_IO;
		return &res;
	}
	res.readres_u.reply.data.data_len = nbytes;

	/*
	 * Now update accessed time for file
	 */
	NFStime(&map.f_d.fattr.atime);
	res.readres_u.reply.attributes = map.f_d.fattr;
	update_map(&map);
	return &res;
}
