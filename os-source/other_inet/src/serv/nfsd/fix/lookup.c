/* -----------------------------------------------------------------------
 * lookup.c   NFSD
 *
 * $Locker:$
 *
 * $Id:$
 *
 * $Revision:$
 *
 * $Log:$
 *
 * $Header:$
 *
 *------------------------------------------------------------------------
 */

#define DEBUG 1

#include<exec/types.h>
#include<dos/dos.h>


#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <string.h>
#include "nfsd.h"

#include "debug.h"


/*
 * Lookup: check that directory generation number is still ok, then
 *	   lookup name in dir and return new filehandle.
 */
diropres *
nfsproc_lookup_2(argp, rqstp)
	diropargs *argp;
	struct svc_req *rqstp;
{
	static diropres res, zerores;
	register cookie *ck, *newck;
	mapentry map;
	res = zerores;
	ck = (cookie *)&argp->dir;

	DB("lookup:nfsproc_lookup\n") ;
	DB("  argp->filename = ") ;
	DB(argp->name) ;
	DB("\n") ;

	DB("  argp->dir = ") ;
	DB(argp->dir.data) ;
	DB("\n") ;

	if((res.status = GET_MAP(ck, &map)) != NFS_OK){
		return &res;
	}
	if((res.status = get_map_name(ck, argp->name, &map)) != NFS_OK){
		return &res;
	}

	res.diropres_u.diropres.attributes = map.f_d.fattr;

	newck = (cookie *)&res.diropres_u.diropres.file;
	bzero(newck, sizeof(*newck));
	newck->file_gen = map.f_d.generation;
	newck->file_id = map.f_d.fattr.fileid;
	newck->volume_id = map.f_d.fattr.fsid;

	res.status = NFS_OK;
	return &res;
}

