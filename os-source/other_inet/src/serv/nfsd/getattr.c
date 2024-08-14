/*
 * get object attributes
 */
#include <rpc/types.h>
#include <nfs/nfs.h>

#include "nfsd.h"

nfsstat
getattr(cookie *ck, fattr *attr)
{
	nfsstat error;
	mapentry map;
	if((error = GET_MAP(ck, &map)) != NFS_OK){
		return error;
	}
	*attr = map.f_d.fattr;

	return NFS_OK;
}

attrstat *
nfsproc_getattr_2(argp, rqstp)
	nfs_fh *argp;
	struct svc_req *rqstp;
{
	static attrstat res, zerores;

	res = zerores;
	res.status = getattr(argp, &res.attrstat_u.attributes);
	return &res;
}

