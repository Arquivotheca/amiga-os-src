#include <rpc/rpc.h>
#include <stdio.h>
#include <string.h>
#include "mount.h"
#include "stat.h"
#include "nfsd.h"

void
mountprog_1(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	union {
		dirpath mountproc_mnt_1_arg;
		dirpath mountproc_umnt_1_arg;
	} argument;
	char *result;
	bool_t (*xdr_argument)(), (*xdr_result)();
	char *(*local)();

	stat.mntd_call++;
	switch (rqstp->rq_proc) {
	case MOUNTPROC_NULL:
		stat.mntd_null++;
		xdr_argument = xdr_void;
		xdr_result = xdr_void;
		local = (char *(*)()) mountproc_null_1;
		break;

	case MOUNTPROC_MNT:
		stat.mntd_mnt++;
		xdr_argument = xdr_dirpath;
		xdr_result = xdr_fhstatus;
		local = (char *(*)()) mountproc_mnt_1;
		break;

	case MOUNTPROC_DUMP:
		stat.mntd_dump++;
		xdr_argument = xdr_void;
		xdr_result = xdr_mountlist;
		local = (char *(*)()) mountproc_dump_1;
		break;

	case MOUNTPROC_UMNT:
		stat.mntd_umnt++;
		xdr_argument = xdr_dirpath;
		xdr_result = xdr_void;
		local = (char *(*)()) mountproc_umnt_1;
		break;

	case MOUNTPROC_UMNTALL:
		stat.mntd_umntall++;
		xdr_argument = xdr_void;
		xdr_result = xdr_void;
		local = (char *(*)()) mountproc_umntall_1;
		break;

	case MOUNTPROC_EXPORT:
		stat.mntd_export++;
		xdr_argument = xdr_void;
		xdr_result = xdr_exports;
		local = (char *(*)()) mountproc_export_1;
		break;

	case MOUNTPROC_EXPORTALL:
		stat.mntd_exportall++;
		xdr_argument = xdr_void;
		xdr_result = xdr_exports;
		local = (char *(*)()) mountproc_exportall_1;
		break;

	default:
		stat.mntd_unknown++;
		svcerr_noproc(transp);
		return;
	}
	bzero(&argument, sizeof(argument));
	if (!svc_getargs(transp, xdr_argument, &argument)) {
		svcerr_decode(transp);
		return;
	}
	result = (*local)(&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, xdr_result, result)) {
		svcerr_systemerr(transp);
	}
	if (!svc_freeargs(transp, xdr_argument, &argument)) {
		fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
}

