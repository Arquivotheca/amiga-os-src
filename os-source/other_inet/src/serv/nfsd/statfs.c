/*
 * get FS statistics
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>

#include <rpc/types.h>
#include <nfs/nfs.h>

#include "nfsd.h"

statfsres *
nfsproc_statfs_2(argp, rqstp)
	nfs_fh *argp;
	struct svc_req *rqstp;
{
	static statfsres res, zerores;
	BPTR lk;
	struct InfoData	*id;
	res = zerores;
	if((res.status = get_lock(argp, &lk)) != NFS_OK){
		return &res;
	}

	id = (struct InfoData *)AllocMem(sizeof(*id), MEMF_CLEAR|MEMF_PUBLIC);
	if(id == (struct InfoData *)NULL){
		res.status = NFSERR_NOSPC;
		return &res;
	}

	if(nfs_info(lk, id)){
		res.statfsres_u.reply.tsize = NFS_MAXDATA;
		res.statfsres_u.reply.bsize = id->id_BytesPerBlock;
		res.statfsres_u.reply.blocks = id->id_NumBlocks;
		res.statfsres_u.reply.bfree =
		res.statfsres_u.reply.bavail=
				id->id_NumBlocks - id->id_NumBlocksUsed;
		res.status = NFS_OK;
	} else {
		res.status = io_to_nfserr();
	}

	FreeMem(id, sizeof(*id));

	return &res;
}
