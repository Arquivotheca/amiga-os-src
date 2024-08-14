/* -----------------------------------------------------------------------
 * PtoFH.c  (NFSC) SAS
 *
 * $Locker:  $
 *
 * $Id: ptofh.c,v 1.3 91/08/06 16:08:09 martin Exp $
 *
 * $Revision: 1.3 $
 *
 * $Header: inet2:src/c/nfsc/RCS/ptofh.c,v 1.3 91/08/06 16:08:09 martin Exp $
 *
 *------------------------------------------------------------------------
 */



/*
 * Path to fhandle translator.  Parses Amiga path relative to a lock,
 * returns filehandle for object pointed to by path.
 */

#include "fs.h"
#include <sys/types.h>
#include <sys/time.h>

#include <string.h>

/* local functions */
static int searchdir(struct mount_pt *mt, register char *name, nfs_fh *dirp);

#define MAX_BSTR_SIZE	255

int
ptofh(mt, dp, fd, path, nfd, name)
	struct mount_pt *mt;
	register struct DosPacket *dp;
	struct fdata *fd;
	u_char *path;
	char *name;
	register struct fdata *nfd;
{
	u_char pathbuf[MAX_BSTR_SIZE+1];
	register u_char *c, *p, *last;
	struct diropargs di;
	struct diropres dor;
	enum clnt_stat err;
	int len;
	short readlink = 0;



	/* ACTION_READ_LINK passes in a CSTR instead of a BSTR */
	/* set readlink so we know to interpret softlinks */
	if(dp->dp_Type==ACTION_READ_LINK) {
		readlink=1;
		len = strlen(path);
		if(len > MAX_BSTR_SIZE) {
			dp->dp_Res2 = ERROR_LINE_TOO_LONG;
			return NOTFOUND;
		}

	} else
		len = *path++;


	bcopy(path, pathbuf, len);
	pathbuf[len] = 0;

	/*
	 * everything up to the first colon is considered a volume name
	 */
	for(c = pathbuf; *c; c++){
		if(*c == ':'){
			break;
		}
	}
	c = (*c != 0) ? c + 1 : pathbuf;

	*nfd = *fd;
	nfd->fd_head = nfd->fd_next = (struct entry *)NULL;

	if(*c == 0){
		dp->dp_Res2 = ERROR_OBJECT_EXISTS;
		return ALLFOUND;
	}

	/*
	 * now iterate for each path component
  	 */
	do {
		/*
		 * get next component into name
		 */
		last = c;
		p = (u_char *)name;
		while(*c != '/'){
			*p++ = *c++;
			if(*c == 0){
				break;
			}
		}
		*p = 0;
#ifdef MONITOR
KPrintF("looking for %s\n",name);
#endif
		/*
		 * check to see that current args describe directory
		 */
		if(nfd->fd_type != FD_DIR){
			if(nfd->fd_type == FD_LINK)
				dp->dp_Res2 = ERROR_IS_SOFT_LINK;
			else
				dp->dp_Res2 = ERROR_OBJECT_WRONG_TYPE;
			return NOTFOUND;
		}
		/*
		 * set up diropargs to lookup relative to cwd
  		 */
		di.dir = nfd->fd_dir;
		if(name[0] == 0 || !strcmp(name,"..")){
			di.name = "..";
			if(nfd->fd_dir_fid == mt->mt_lock.fd_dir_fid){
				dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
				return NOTFOUND;
			}
		} else {
			di.name = name;
		}

		/*
		 * lookup pathname component on server.  Try verbatim
		 * form first, if not found, then do case insensitive search
		 * of directory.  If multiple matches are found then
	 	 * file requester is put up.
		 */
		err = (int)nfs_call(mt, NFSPROC_LOOKUP,	xdr_diropargs, &di,
							xdr_diropres, &dor);

		if(!mt->mt_case) {
			if(err==0 && dor.status != NFS_OK){
				if(searchdir(mt, name, &di.dir)){
					err = (int)nfs_call(mt, NFSPROC_LOOKUP,
								xdr_diropargs, &di,
								xdr_diropres, &dor);
				}
			}
		}
		if(err != RPC_SUCCESS || dor.status != 0){
#ifdef MONITOR
KPrintF("Lookup failed\n");
#endif
			dp->dp_Res2 = fh_status(err, dor.status);
			return (*c == 0) ? LASTNOT:NOTFOUND;
		}

#define ATTR(ff) ((ff).diropres_u.diropres.attributes)
		if((ATTR(dor).mode&NFSMODE_DIR) == NFSMODE_DIR){
			nfd->fd_type = FD_DIR;
			nfd->fd_dir = dor.diropres_u.diropres.file;
			nfd->fd_dir_fid = ATTR(dor).fileid;
		} else
			if ((ATTR(dor).mode&NFSMODE_LNK) == NFSMODE_LNK) {
			struct readlinkres rl;
			char buffer[NFS_MAXPATHLEN];
			int nlen;
#ifdef MONITOR
                        KPrintF("Looks like a link\n");
#endif
			if (readlink) {
				rl.readlinkres_u.data = buffer;
				err = (int)nfs_call(mt, NFSPROC_READLINK,xdr_nfs_fh,
					&dor.diropres_u.diropres.file, xdr_readlinkres, &rl);
#ifdef MONITOR
				KPrintF("err=%ld\n",err);
#endif
				if (err != RPC_SUCCESS || rl.status != 0) {
					dp->dp_Res2 = fh_status(err,rl.status);
#ifdef MONITOR
					KPrintF("link %s not found\n",name);
#endif
					return (NOTFOUND);
				}
#ifdef MONITOR
				KPrintF("Found a link from %s to %s\n",name,rl.readlinkres_u.data);
#endif
				len = strlen(buffer);
				if(strchr(buffer,':')) {
					if(len+1>dp->dp_Arg4) {
						dp->dp_Res1 = -2;
						dp->dp_Res2 = ERROR_OBJECT_TOO_LARGE;
						return (NOTFOUND);
					}
					strncpy(dp->dp_Arg3,buffer,len+1);
					dp->dp_Res1 = len;
				} else {
					nlen = last-pathbuf;
					if(len+nlen+1>dp->dp_Arg4) {
						dp->dp_Res1 = -2;
						dp->dp_Res2 = ERROR_OBJECT_TOO_LARGE;
						return (NOTFOUND);
					}
					strncpy(dp->dp_Arg3,pathbuf,nlen);
					strcpy((char *)dp->dp_Arg3+nlen,buffer);
					dp->dp_Res1 = nlen+len;
				}
				dp->dp_Res2 = 0;
				return (ALLFOUND);
			} else {
				nfd->fd_type = FD_LINK;
				nfd->fd_file = dor.diropres_u.diropres.file;
				nfd->fd_file_fid = ATTR(dor).fileid;
			}

		} else {
			nfd->fd_type = FD_FILE;
			nfd->fd_file = dor.diropres_u.diropres.file;
			nfd->fd_file_fid = ATTR(dor).fileid;
		}
#undef ATTR
		/*
		 *  x/y/.. where y is file fail
		 */
		if(*c == '/'){
			if(nfd->fd_type == FD_FILE){
				dp->dp_Res1 = 0;
				dp->dp_Res2 = ERROR_OBJECT_WRONG_TYPE;
				return (NOTFOUND);
			} else if(nfd->fd_type == FD_LINK) {
				dp->dp_Res1 = 0;
				dp->dp_Res2 = ERROR_IS_SOFT_LINK;
				return (NOTFOUND);
			}
			c++;
		}
	} while (*c != 0);

	dp->dp_Res2 = ERROR_OBJECT_EXISTS;

	return ALLFOUND;
}

/*
 * Do case insensitive search of directory for name.  Return match in
 * name.  If more than one match is found then (to prevent inadvertent
 * modification of the wrong file) we fail.
 */
static int
searchdir(mt, name, dirp)
	struct mount_pt *mt;
	register char *name;
	nfs_fh *dirp;
{
	extern bool_t xdr_my_readdirres(), xdr_my_entry_list();
	struct readdirargs ra;
	struct readdirres rr;
	register entry *e;
	int err, found;

	ra.count = mt->mt_rdsize;
	ra.dir = *dirp;
	bzero(&ra.cookie, sizeof(ra.cookie));
	found = 0;

	do {
		bzero(&rr, sizeof(rr));
		err = (int)nfs_call(mt, NFSPROC_READDIR,
						xdr_readdirargs, &ra,
						xdr_my_readdirres, &rr);
		if(err != 0 || rr.status != NFS_OK){
			break;
		}

		for(e = rr.readdirres_u.reply.entries; e; e = e->nextentry){
			if(stricmp(e->name,name) == 0){
				strcpy(name, e->name);
				found++;
			}
			if(!e->nextentry){
				ra.cookie = e->cookie;
			}
		}

		if(rr.readdirres_u.reply.entries){
			xdr_free(xdr_my_entry_list, &rr.readdirres_u.reply.entries);
			rr.readdirres_u.reply.entries = 0;
		}

	} while(!rr.readdirres_u.reply.eof);

	if(rr.readdirres_u.reply.entries){
		xdr_free(xdr_my_entry_list, &rr.readdirres_u.reply.entries);
		rr.readdirres_u.reply.entries = 0;
	}

	return (found == 1) ? 1:0;
}

/*
 * Map rpc/nfs status code to Amiga status code.
 */
int
fh_status(rpc_error, nfs_error)
	enum clnt_stat rpc_error;
	int nfs_error;
{
	char *errmsg, *clnt_sperrno();

	if(rpc_error != RPC_SUCCESS){
		errmsg = clnt_sperrno(rpc_error);
		if(errmsg){
			(void)requester(0L, " OK ", "NFS Client Problem\n%s\n", errmsg);
		}
		return 4000 + rpc_error;
	}

	switch(nfs_error){
	case NFS_OK:
		return 0;

	case NFSERR_PERM:
	case NFSERR_ACCES:
		return ERROR_WRITE_PROTECTED;

	case NFSERR_NOENT:
		return ERROR_OBJECT_NOT_FOUND;

	case NFSERR_NAMETOOLONG:
		return ERROR_INVALID_COMPONENT_NAME;

	case NFSERR_FBIG:
	case NFSERR_DQUOT:
	case NFSERR_NOSPC:
		return ERROR_DISK_FULL;

	case NFSERR_EXIST:
		return ERROR_OBJECT_EXISTS;

	case NFSERR_NOTDIR:
	case NFSERR_ISDIR:
		return ERROR_OBJECT_WRONG_TYPE;

	case NFSERR_ROFS:
		return ERROR_DISK_WRITE_PROTECTED;

	case NFSERR_NOTEMPTY:
		return ERROR_DIRECTORY_NOT_EMPTY;

	case NFSERR_NXIO:
	case NFSERR_NODEV:
		return ERROR_DEVICE_NOT_MOUNTED;

	case NFSERR_STALE:
		return ERROR_INVALID_LOCK;

	case NFSERR_WFLUSH:
	case NFSERR_IO:
	default:
		return 5000+nfs_error;
	}
}
