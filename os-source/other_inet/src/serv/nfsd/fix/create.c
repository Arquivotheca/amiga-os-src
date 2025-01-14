/*
 * create directory
 */

#include<exec/types.h>
#include<dos/dos.h>
#include <rpc/rpc.h>
#include "nfsd.h"

#define DEBUG 1 /*************** !!!!!!!!!!! ***********/
#include "debug.h"


extern BPTR debug_fh ;

static void pick_uid_gid(struct svc_req *rqstp, sattr *attr);

static diropres *create(ftype type, createargs *argp, struct svc_req *rqstp);

static void
pick_uid_gid(rqstp, attr)
	struct svc_req *rqstp;
	sattr *attr;
{
	struct authunix_parms *unix_cred;
	DB("create:pick_uid_gid\n") ;
	if (rqstp->rq_cred.oa_flavor == AUTH_UNIX) {
		unix_cred = (struct authunix_parms *) rqstp->rq_clntcred;
		attr->uid = unix_cred->aup_uid;
		attr->gid = unix_cred->aup_gid;
	} else {
		attr->gid = attr->uid = -2;
	}
}

/*
 * Create object.
 */
static diropres *
create(type, argp, rqstp)
	ftype type;
	createargs *argp;
	struct svc_req *rqstp;
{
	static diropres res, zerores;
	cookie *ck;

	res = zerores;
	ck = (cookie *)&res.diropres_u.diropres.file;
	res.status = fh_build(&argp->where.dir, argp->where.name, type, ck);
	
	DB("create.c :create() : where.dir = ") ;
	DB(&argp->where.dir) ;
	DB("\nwhere.name = ") ;
	DB(&argp->where.name) ;
	DB("\n")

	if(res.status != NFS_OK){
		return &res;
	}

	pick_uid_gid(rqstp, &argp->attributes);
	res.status = setattr(ck, &argp->attributes,
			&res.diropres_u.diropres.attributes);
	return &res;
}

/*
 * create a file - this function can implicitly delete an object if
 *		   it exists already, though the protocol spec doesn't
 *		   mention this fact ;-).  We'll that's whatcha get when
 *		   a committee of one designs and implements a protocol.
 */
diropres *
nfsproc_create_2(argp, rqstp)
	createargs *argp;
	struct svc_req *rqstp;
{
	static diropres res, zerores;
	DB("create:nfsproc_create_2\n") ;
	res = zerores;
	res.status = fh_delete(&argp->where.dir, argp->where.name);
	if(res.status == NFSERR_NOENT || res.status == NFS_OK){
		return create(NFREG, argp, rqstp);
	}
	return &res;
}

diropres *
nfsproc_mkdir_2(argp, rqstp)
	createargs *argp;
	struct svc_req *rqstp;
{
	DB("create:nfsproc_mkdir_2\n") ;
	return create(NFDIR, argp, rqstp);
}

