/*
 * rename object
 */

#include <exec/types.h>
#include <exec/memory.h>

#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <string.h>

#include "nfsd.h"

#define FROM_CK		&argp->from.dir
#define FROM_NAME 	argp->from.name
#define TO_CK		&argp->to.dir
#define TO_NAME		argp->to.name

nfsstat *
nfsproc_rename_2(argp, rqstp)
	renameargs *argp;
	struct svc_req *rqstp;
{
	char fpath[NFS_MAXPATHLEN], tpath[NFS_MAXPATHLEN];
	mapentry fm, tm, parent, m;
	static nfsstat res;
	map_id id;

	/*
	 * Check that destination name is of reasonable length
	 */
	if(strlen(TO_NAME) > FD_NAME_SIZE){
		res = NFSERR_NAMETOOLONG;
		return &res;
	}

	/*
	 * Check that to name doesn't already exist.  This must be right
	 * or timeouts will eat our newly moved object.  Also check that
	 * from name does exist.
	 */
	res = get_map_name(TO_CK, TO_NAME, &m);
	if(res != NFSERR_NOENT){
		return &res;
	}
	if((res = get_map_name(FROM_CK, FROM_NAME, &fm)) != NFS_OK){
		return &res;
	}

	/*
	 * Trim from map out of the destination directory.  close cache in
	 * case "from" is active.
	 */
	close_cache(&fm);
	if((res = get_map(fm.f_d.parent, &parent)) != NFS_OK){
		return &res;
	}
	if(parent.f_d.child == fm.f_d.fattr.fileid){
		parent.f_d.child = fm.f_d.next;
	} else {
		for(id = parent.f_d.child; id; id = m.f_d.next){
			if((res = get_map(id, &m)) != NFS_OK){
				return &res;
			}
			if(m.f_d.next == fm.f_d.fattr.fileid){
				m.f_d.next = fm.f_d.next;
				put_map(&m);
				break;
			}
		}
		/* probably should check for error here, in case
		 * we couldn't find the map in its owning dir.  As
		 * they say "can't happen"
		 */
	}
	NFStime(&parent.f_d.fattr.mtime);
	parent.f_d.fattr.nlink--;
	if((res = put_map(&parent)) != NFS_OK){
		return &res;
	}

	/*
	 * Build the from & to paths that will be used in rename
	 */
	if((res = fh_buildpath(FROM_CK, fpath, FROM_NAME)) != NFS_OK){
		return &res;
	}
	if((res = fh_buildpath(TO_CK, tpath, TO_NAME)) != NFS_OK){
		return &res;
	}

	/*
	 * Try renaming object on disk
	 */
	if(!nfs_rename(fpath, tpath)){
		res = io_to_nfserr();
		return &res;
	}

	/*
	 * enter "from" map into destination directory.  Note that "to" map
	 * is gotten here since it is possible both files live in the
	 * same directory.
	 */
	if((res = GET_MAP(TO_CK, &tm)) != NFS_OK){
		return &res;
	}
	fm.f_d.next = tm.f_d.child;
	tm.f_d.child = fm.f_d.fattr.fileid;
	fm.f_d.parent = tm.f_d.fattr.fileid;
	NFStime(&tm.f_d.fattr.mtime);
	tm.f_d.fattr.nlink++;
	strcpy(fm.f_d.name, TO_NAME);
	if((res = put_map(&fm))){
		return &res;
	}
	if((res = put_map(&tm))){
		return &res;
	}

	res = NFS_OK;
	return &res;
}
