/*
 * create hard link
 */

#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <sys/errno.h>
#include "nfsd.h"

nfsstat *
nfsproc_link_2(argp, rqstp)
	linkargs *argp;
	struct svc_req *rqstp;
{
	static nfsstat res;

	res = EOPNOTSUPP;	/* MMH connectathon 91 */
	return &res;
}

readlinkres *
nfsproc_readlink_2(argp, rqstp)
	nfs_fh	*argp;
	struct	svc_req *rqstp;
{
	static readlinkres res;

	res.status = EOPNOTSUPP;	/* MMH connectathon 91 */
	return &res;
}

nfsstat *
nfsproc_symlink_2(argp, rqstp)
	symlinkargs *argp;
	struct svc_req *rqstp;
{
	static nfsstat res;

	res = EOPNOTSUPP;	/* MMH connectathon 91 */
	return &res;
}

