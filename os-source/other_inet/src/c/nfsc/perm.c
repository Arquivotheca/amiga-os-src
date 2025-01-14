/* -----------------------------------------------------------------------
 * perm.c  (nfsc) SAS
 *
 * $Locker:  $
 *
 * $Id: perm.c,v 1.3 91/08/06 16:07:10 martin Exp $
 *
 * $Revision: 1.3 $
 *
 * $Header: inet2:src/c/nfsc/RCS/perm.c,v 1.3 91/08/06 16:07:10 martin Exp $
 *
 *------------------------------------------------------------------------
 */



/*
 * Utilities to convert to/from Unix style permissions.
 * To set the record straight:
 *
 *	FIBF_READ	- if present then the file is not readable
 *	FIBF_WRITE	- if present then the file is not writeable
 *	FIBF_EXECUTE	- if present then the file is not executable
 *	FIBF_DELETE	- not supported; Unix doesn't directly provide this
 *	FIBF_PURE	- if present then the file is consider "pure"
 *	FIBF_SCRIPT	- if present then the file is a script
 *	FIBF_ARCHIVE	- if present then this file will be archived
 *	FIBF_HIDDEN	- not supported; too bad, dudes.
 */

#include "fs.h"

#include <sys/types.h>
#include <nfs/perm.h>

/*
 * Map amiga permission word into something compatible with Unix.  umask
 * controls the Unix view of the file.
 */

int
perm_amiga_to_unix(amiga_perm)
{
	int	unix_perm;

	unix_perm = 0777 & ~getumask();
/*	unix_perm =  NFS_OWN_RDPERM | NFS_GID_RDPERM | NFS_PUB_RDPERM;
	unix_perm |= NFS_OWN_WRPERM | NFS_GID_WRPERM | NFS_PUB_WRPERM;
	unix_perm |= NFS_OWN_EXPERM | NFS_GID_EXPERM | NFS_PUB_EXPERM; */

	if(amiga_perm & FIBF_READ){
		unix_perm &= ~(NFS_OWN_RDPERM | NFS_GID_RDPERM | NFS_PUB_RDPERM);
	}

	if(amiga_perm & FIBF_WRITE){
		unix_perm &= ~(NFS_OWN_WRPERM | NFS_GID_WRPERM | NFS_PUB_WRPERM);
	}

	if(amiga_perm & FIBF_EXECUTE){
		unix_perm &= ~(NFS_OWN_EXPERM | NFS_GID_EXPERM | NFS_PUB_EXPERM);
	}

	return unix_perm;
}

int
perm_unix_to_amiga(uid, gid, unix_perm)
{
	int myuid, mygid = 0, amiga_perm;
	short i, num;
    gid_t gids[10];

	myuid = (int)getuid();

	amiga_perm = FIBF_EXECUTE|FIBF_READ|FIBF_WRITE|FIBF_DELETE;

	if(uid == myuid){
		if(unix_perm & NFS_OWN_RDPERM){
			amiga_perm &= ~FIBF_READ;
		}
		if(unix_perm & NFS_OWN_WRPERM){
			amiga_perm &= ~FIBF_WRITE;
		}
		if(unix_perm & NFS_OWN_EXPERM){
			amiga_perm &= ~FIBF_EXECUTE;
		}
	}

	/* search all gids */
	num = getgroups(10,gids);

	for(i=0;i<num;i++) {
		if (gids[i] == gid) {
			mygid++;
			break;
		}
	}

	if(mygid){
		if(unix_perm & NFS_GID_RDPERM){
			amiga_perm &= ~FIBF_READ;
		}
		if(unix_perm & NFS_GID_WRPERM){
			amiga_perm &= ~FIBF_WRITE;
		}
		if(unix_perm & NFS_GID_EXPERM){
			amiga_perm &= ~FIBF_EXECUTE;
		}
	}

	if(unix_perm & NFS_PUB_RDPERM){
		amiga_perm &= ~FIBF_READ;
	}
	if(unix_perm & NFS_PUB_WRPERM){
		amiga_perm &= ~FIBF_WRITE;
	}
	if(unix_perm & NFS_PUB_EXPERM){
		amiga_perm &= ~FIBF_EXECUTE;
	}



	/*
	 * In order to be considered executable the file must be readable
	 * AND have the owner execute permission bit set.
	 */
	if(amiga_perm & FIBF_READ){
		amiga_perm |= FIBF_EXECUTE;
	}

	/*
	 * Always turn off delete protection bit (this could be
	 * done better if the parent dir stats were available).
	 */
	amiga_perm &= ~FIBF_DELETE;

	return amiga_perm;
}
