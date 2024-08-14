/*
 * set attributes
 */

#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include "nfsd.h"

#define	  DONT_CARE	-1

static void update_date(cookie *ck, nfstime ntime);
static nfsstat change_size(cookie *ck, mapentry *mp, sattr *atp);

/*
 * update time on disk
 */
static void update_date(ck, ntime)
	cookie *ck;
	nfstime	ntime;
{
	struct DateStamp ds;

	nfs_to_amigatime(&ntime, &ds);
	/* SetDate(XXX, &ds);   No binding ??????? */
}

static nfsstat
change_size(ck, mp, atp)
	cookie *ck;
	mapentry *mp;
	sattr *atp;
{
	extern nfsstat change_file_size();
	nfsstat error;
	long fh;

	if((error = get_fd(ck, &fh)) != NFS_OK){
		return error;
	}
	return change_file_size(fh, atp->size, mp);
}

nfsstat
setattr(ck, att, fatt)
	cookie 	*ck;
	sattr 	*att;
	fattr 	*fatt;
{
	nfsstat error;
	mapentry map;
 
	if((error = GET_MAP(ck, &map)) != NFS_OK){
		return error;
	}

    	if(att->size != DONT_CARE){
		switch(map.f_d.fattr.type){
		case NFREG:
			if((error = change_size(ck, &map, att)) != NFS_OK){
				return error;
			}
			break;

		case NFDIR:		/* let this minor sin pass */
			map.f_d.fattr.size = att->size;
			break;

		default:
			return NFSERR_IO;
		}
	}

	if(att->uid != DONT_CARE){
		map.f_d.fattr.uid = att->uid;
	}

	if(att->gid != DONT_CARE){
		map.f_d.fattr.gid = att->gid;
	}

	if(att->mode != 0177777 && att->mode != DONT_CARE){

		/*
		 * Change permission bits first
		 */
		map.f_d.fattr.mode &= NFSMODE_FMT;
		map.f_d.fattr.mode |= att->mode & ~NFSMODE_FMT;

		/*
		 * Check mode bits now
		 */
		switch(att->mode & NFSMODE_FMT){
		case 0:
			/* no change in mode bits */
			break;

		case NFSMODE_DIR:
		case NFSMODE_REG:
			if((map.f_d.fattr.mode&NFSMODE_FMT) != (att->mode&NFSMODE_FMT)){
				/*
				 * change of object type.  Does this happen?
				 */
				return NFSERR_IO;
			}
			break;

		case NFSMODE_BLK:
		case NFSMODE_CHR:
		case NFSMODE_LNK:
		case NFSMODE_SOCK:
		case NFSMODE_FIFO:
		default:
			return NFSERR_IO;
		}
	}

	if(att->atime.seconds != DONT_CARE || att->atime.useconds != DONT_CARE){
		map.f_d.fattr.atime = att->atime;
	}

	if(att->mtime.seconds != DONT_CARE || att->mtime.useconds != DONT_CARE){
		map.f_d.fattr.mtime = att->mtime;
		update_date(ck, att->mtime);
	}

	*fatt = map.f_d.fattr;
	if((error = put_map(&map)) != NFS_OK){
		return error;
	}

	return NFS_OK;
}

attrstat *
nfsproc_setattr_2(argp, rqstp)
	sattrargs *argp;
	struct svc_req *rqstp;
{
	static attrstat res, zerores;

	res = zerores;
	res.status = setattr(&argp->file, &argp->attributes, &res.attrstat_u.attributes);
	return &res;
}
