/*
 * remove file
 */

#include <rpc/rpc.h>
#include <nfs/nfs.h>

#include "nfsd.h"

nfsstat *
nfsproc_remove_2(argp, rqstp)
	diropargs *argp;
	struct svc_req *rqstp;
{
	static nfsstat res;
	mapentry map;
	if((res = GET_MAP((cookie *)&argp->dir, &map)) != NFS_OK){
		return &res;
	}
	res = fh_delete((cookie *)&argp->dir, argp->name);
	return &res;
}

nfsstat *
nfsproc_rmdir_2(argp, rqstp)
	diropargs *argp;
	struct svc_req *rqstp;
{
	static nfsstat res;
	mapentry map;

	if((res = GET_MAP((cookie *)&argp->dir, &map)) != NFS_OK){
		return &res;
	}

	res = fh_delete((cookie *)&argp->dir, argp->name);
	return &res;
}

