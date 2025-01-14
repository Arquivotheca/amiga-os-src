/*
 * Readdir call.  Probably the most complex of any NFS call.
 */

#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <string.h>
#include "nfsd.h"

#define NULLENTRY (struct entry *)NULL

extern char iobuf[NFS_MAXDATA];
static char *cur, *end = iobuf + NFS_MAXDATA;

static struct entry *make_entry(ulong fileid, char *name, int namelen, nfscookie cky);

static struct entry *
make_entry(fileid, name, namelen, cky)
	ulong fileid;
	char *name;
	int namelen;
	nfscookie cky;
{
	register struct entry *e;

	if( ((long)cur) & 1){ /* if odd, roll up one */
		cur++;
	}

	e = (entry *)cur;
	if((cur += (sizeof(entry) + namelen + 1)) > end){
		return NULLENTRY;
	}
	e->name = (char *)(e + 1);
	strcpy(e->name, name);
	e->fileid = fileid;
	e->cookie = cky;

	return e;
}

readdirres *
nfsproc_readdir_2(argp, rqstp)
	readdirargs *argp;
	struct svc_req *rqstp;
{
	static readdirres res;
	register cookie *ck;
	mapentry map, tmap;
	register entry **e;
	map_id file_id;
	int res_size;
	int namelen;

	cur = iobuf;
	ck = (cookie *)&argp->dir;
	bcopy(&argp->cookie, &file_id, sizeof(file_id));
	if(argp->count > NFS_MAXDATA){
		argp->count = NFS_MAXDATA;
	}

	e = &res.readdirres_u.reply.entries;
	bzero(&res,sizeof(res));
	res.status = NFS_OK;

	/*
	 * Sanity check parent directory: must be present and readable.
	 */
	if((res.status = GET_MAP(ck, &map)) != NFS_OK){
		return &res;
	}

	if(map.f_d.fattr.type != NFDIR){
		res.status = NFSERR_NOTDIR;
		return &res;
	}

	/*
	 * Head of directory is defined as cookie == 0.  If cookie is
	 * non zero, sanity check incoming id to see that its parent
	 * directory is the one we expect and that the map does in fact
	 * exist for the cookie.
	 */
	res_size = 0;
	if(file_id == 0){

		file_id = map.f_d.child;

		*e = make_entry(map.f_d.fattr.fileid, ".", 1, map.f_d.child);
		if(*e == NULLENTRY){
			res.status = NFSERR_IO;
			return &res;
		}
		e = &(*e)->nextentry;

		if(map.f_d.parent == LABEL_MAP){ /* at root  ".." = "." map */
			map.f_d.parent = map.f_d.fattr.fileid;
			*e = make_entry(map.f_d.fattr.fileid, "..", 2,
							map.f_d.child);
		} else {
			res.status = get_map(map.f_d.parent, &map);
			if(res.status != NFS_OK){
				return &res;
			}
			*e = make_entry(map.f_d.fattr.fileid, "..", 2,
							map.f_d.child);
		}
		if(*e == NULLENTRY){
			return &res;
		}
		e = &(*e)->nextentry;

		res_size += 2*(sizeof(entry) + 16) + 4;

	} else {
		if(file_id == -1){
			res.readdirres_u.reply.eof = 1;
			return &res;
		}

		if((res.status = get_map(file_id, &map)) != NFS_OK){
			return &res;
		}

		if(map.f_d.parent != ck->file_id){
			/*
			 * Directory read has gotten out of sync; try to
			 * recover by restarting scan at f_d.child.  This
			 * can happen when the user does rm on an object
			 * within a directory that is being readdir'ed. Caching
			 * of directories in NFS clients ordinarily obscures
			 * this subtle NFS server problem.
			 */
			if((res.status = GET_MAP(ck, &map)) != NFS_OK){
				return &res;
			}
			file_id = map.f_d.child;
		}
	}

	do {
		/*
		 * If directory is empty then just return
		 */
		if(file_id == 0){
			*e = NULLENTRY;
			res.readdirres_u.reply.eof = 1;
			res.readdirres_u.reply.entries=NULLENTRY; 
			return &res;
		}

		if((res.status = get_map(file_id, &map)) != NFS_OK){
			return &res;
		}

		namelen = strlen(map.f_d.name);
		res_size += namelen + sizeof(entry) + 16; /* XXX why 16? */
		if(res_size > argp->count){
			break;
		}

		if((file_id = map.f_d.next) == 0){
			res.readdirres_u.reply.eof = 1;
			file_id = -1;
		}

		*e = make_entry(map.f_d.fattr.fileid, map.f_d.name,
						namelen, file_id);
		if(*e == NULLENTRY){
			break;
		}
		e = &(*e)->nextentry;
	} while(!res.readdirres_u.reply.eof);

	*e = NULLENTRY;

	return &res;
}
