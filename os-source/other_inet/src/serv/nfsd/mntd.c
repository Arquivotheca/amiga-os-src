/*
 * mntd main code
 */

#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>

#include "mount.h"
#include "nfsd.h"

void *
mountproc_null_1(argp, rqstp)
	void *argp;
	struct svc_req *rqstp;
{
	static char res;

	bzero(&res, sizeof(res));
	return ((void *)&res);
}

fhstatus *
mountproc_mnt_1(argp, rqstp)
	dirpath *argp;
	struct svc_req *rqstp;
{
	struct authunix_parms *ucp;
	static fhstatus res, zero;
	char buf[256];
	int result;
	cookie *ck;

	res = zero;
	res.fhs_status = NFSERR_ACCES;
	if (rqstp->rq_cred.oa_flavor != AUTH_UNIX) {
		return &res;
	}

/*
	ucp = (struct authunix_parms *) rqstp->rq_clntcred;
	KPrintF("exportproc %s %s %ld %ld",
				*argp,
				ucp->aup_machname? ucp->aup_machname:"-",
				ucp->aup_uid,
				ucp->aup_gid);
*/
	ck = (cookie *)&res.fhstatus_u.fhs_fhandle;
	if((res.fhs_status = get_root_dir(*argp, ck)) != NFS_OK){
		return &res;
	}

	res.fhs_status = NFS_OK;
	return &res;
}

mountlist *
mountproc_dump_1(argp, rqstp)
	void *argp;
	struct svc_req *rqstp;
{
	static mountlist res;
	return &res;
}


void *
mountproc_umnt_1(argp, rqstp)
	dirpath *argp;
	struct svc_req *rqstp;
{
	static char res;
	return ((void *)&res);
}

void *
mountproc_umntall_1(argp, rqstp)
	void *argp;
	struct svc_req *rqstp;
{
	static char res;
	return ((void *)&res);
}

exports *
mountproc_export_1(argp, rqstp)
	void *argp;
	struct svc_req *rqstp;
{
	extern char iobuf[NFS_MAXDATA];
	static exports p;
	mapentry map, l;
	exports op;
	map_id n;
	int len;

	if(get_map(LABEL_MAP, &l) != NFS_OK){
		return (exports *)NULL;
	}

	bzero(iobuf, sizeof(iobuf));
	p = (exports)iobuf; op = 0;
	for(n = l.label.fs; n < (l.label.fs + l.label.numfs); n++){
		if(get_map(n, &map) != NFS_OK){
			break;
		}
		if((len = strlen(map.f_d.name) + 1) == 1){
			continue;
		}

		if(len & 1){
			len++;	/* to maintain alignment */
		}
		if((((char *)p) + len) >= (iobuf + NFS_MAXDATA)){
			break;
		}

		p->ex_dir = (char *)(p + 1);
		strcpy(p->ex_dir, map.f_d.name);
		p->ex_groups = 0;
		p->ex_next = 0;
		if(op){
			op->ex_next = p;
		}
		op = p;
		p = (exports)(p->ex_dir + len);
	}

	p = op? (exports)iobuf : 0;
	return &p;
}

exports *
mountproc_exportall_1(argp, rqstp)
	void *argp;
	struct svc_req *rqstp;
{
	return (mountproc_export_1(argp, rqstp));
}
