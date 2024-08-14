/*
 * Utilities to access NFS map file.
 */

#define DEBUG 1
#include <dos/dos.h>
#include <clib/dos_protos.h>

#include <string.h>
#include <rpc/rpc.h>
#include <nfs/nfs.h>

#include "debug.h"
#include "nfsd.h"

extern BPTR debug_fh ;

/*
 * fh_delete - delete object referred to by cookie/name.
 */
nfsstat
fh_delete(ck, name)
	register cookie *ck;
	char *name;
{
	mapentry obj, parent, map;
	char path[NFS_MAXPATHLEN];
	map_id generation, id;
	nfsstat error;

	/*
	 * Get map for object which will be deleted.  If it is a directory
	 * and is non-empty then fail.  Get Parent directory
	 */
	if((error = get_map_name(ck, name, &obj)) != NFS_OK){
		return error;
	}
	if(obj.f_d.fattr.type == NFDIR && obj.f_d.child != 0){
		return NFSERR_NOTEMPTY;
	}
	if((error = get_map(obj.f_d.parent, &parent)) != NFS_OK){
		return error;
	}

	close_cache(&obj);

	/*
	 * Build path now since we will not have it after update
	 */
	if((error = fh_buildpath(ck, path, name)) != NFS_OK){
		return error;
	}

	/*
	 * update internal directory
	 */
	if(obj.f_d.fattr.fileid == parent.f_d.child){ /* easy street */
		parent.f_d.child = obj.f_d.next;
	} else {
		for(id = parent.f_d.child; id; id = map.f_d.next){
			if((error = get_map(id, &map)) != NFS_OK){
				return error;
			}

			if(map.f_d.next == obj.f_d.fattr.fileid){
				map.f_d.next = obj.f_d.next;
				if((error = put_map(&map)) != NFS_OK){
					return error;
				}
				break;
			}
		}
	}

	/*
	 * update parent directory
	 */
	parent.f_d.fattr.nlink--;
	NFStime(&parent.f_d.fattr.mtime);
	if((error = put_map(&parent)) != NFS_OK){
		return error;
	}
	MAKEFREE(obj.f_d.fattr.fileid);

	/*
	 * Clear the entry, bump generation counter
	 */
	generation = obj.f_d.generation;
	id = obj.f_d.fattr.fileid;
	bzero(&map, sizeof(map));
	map.f_d.generation = generation + 1;
	map.f_d.fattr.fileid = id;
	if((error = put_map(&map)) != NFS_OK){
		return error;
	}

	/*
	 * Finally, delete the on-disk object itself.  This can fail when
	 * the on-disk object has already been altered, eg during a rename
	 * call.
	 */
	if(!DeleteFile(path)){
		return io_to_nfserr();
	}

	return NFS_OK;
}

/*
 * Build new file object from cookie/name.
 */
nfsstat
fh_build(ck, name, what, newck)
	cookie 	*ck, *newck;
	char 	*name;
	ftype	what;
{
	BPTR fh, lk;
	char path[NFS_MAXPATHLEN];
	mapentry newmap, junk;
	nfsstat status;

	DB("fh.c:fh_build: name = ") ;
	DB(name) ;
	DB("\n") ;
	
	/*
	 * We don't support names longer than FD_NAME_SIZE
	 */
	if(strlen(name) > FD_NAME_SIZE){
		return NFSERR_NAMETOOLONG;
	}

	/*
	 * See if the object exists in our local directory first
	 */
	status = get_map_name(ck, name, &junk);
	switch(status){
	case NFS_OK:
		return NFSERR_EXIST;

	case NFSERR_NOENT:
		break;

	default:
		return status;
	}

	/*
	 * Build full pathname and try to open new file/directory.
	 */
	lk = 0;
	if((status = fh_buildpath(ck, path, name)) != NFS_OK){
		return status;
	}

	lk = 0;	fh = 0;
	if(what == NFDIR){
		if(lk = CreateDir(path)){
			nfs_unlock(lk);
		}
	} else if(what == NFREG){
		if(fh = nfs_open(path, MODE_NEWFILE)){
			nfs_close(fh);
		}
	}
	if(lk==0 && fh==0){
		return io_to_nfserr();
	}

	/*
	 * get new mapentry and initialize it for this object
	 */
	if((status = get_new_map(ck, &newmap)) != NFS_OK){
		return status;
	}
	strcpy(newmap.f_d.name, name);
	newmap.f_d.fattr.type = what;
	if(newmap.f_d.fattr.type == NFDIR){
		newmap.f_d.fattr.mode = NFSMODE_DIR | 0777; /* changed from 755 by MMH */
		newmap.f_d.fattr.size = 512;/* some NFS clients die otherwise */
		newmap.f_d.fattr.nlink = 2;
	} else {
		newmap.f_d.fattr.mode = NFSMODE_REG | 0666;
	}
	newmap.f_d.fattr.blocks = 1; /* used to be fib->fib_NumBlocks*/
	newmap.f_d.fattr.rdev = -1; /* ? */

	/*
	 * Build new cookie and flush new mapentry to disk
	 */
	if((status = put_map(&newmap)) != NFS_OK){
		MAKEFREE(newmap.f_d.fattr.fileid);
		return status;
	}

	bzero(newck, sizeof(*newck));
	newck->volume_id = newmap.f_d.fattr.fsid;
	newck->file_id = newmap.f_d.fattr.fileid;
	newck->file_gen = newmap.f_d.generation;
	status = NFS_OK;	/* already is, but to make it obvious */

	return status;
}

/*
 * Return full path (volume:a/b/c/....) that corresponds to cookie.
 */
nfsstat
fh_buildpath(ck, path, name)
	cookie 	*ck;
	char 	*path;
	char 	*name;
{
	char *cp;
	nfsstat error;
	mapentry map;
	int len;
	if((error = GET_MAP(ck, &map)) != NFS_OK){
		return error;
	}

	map.f_d.parent = ck->file_id;
	cp = path + NFS_MAXPATHLEN - 1;
	*path = *cp = '\0';

	do {
		if((error = get_map(map.f_d.parent, &map)) != NFS_OK){
			return error;
		}

		len = strlen(map.f_d.name);
		if((cp -= len) <= path){
			return NFSERR_NAMETOOLONG;
		}

		bcopy(map.f_d.name, cp, len);

		if(!IS_ROOT(map.f_d.parent)){
			*--cp = '/';
		}

	} while(map.f_d.parent != LABEL_MAP);

	if(cp != path){
		strcpy(path, cp);
	}

	if(name){
		cp = strrchr(path, ':');
		if(cp && cp[1] != 0){ /* if doesn't end in ':' add a '/' */
			strncat(path, "/", NFS_MAXPATHLEN);
		}

		strncat(path, name, NFS_MAXPATHLEN);

	}
	path[NFS_MAXPATHLEN - 1] = '\0';

	return NFS_OK;
}

delete_map(rm)
	struct RexxMsg *rm;
{
	mapentry map, parent;
	nfsstat error;
	cookie ck;

	if((error = get_map(atoi(rm->rm_Args[1]), &map)) != NFS_OK){
		return error;
	}
	if((error = get_map(map.f_d.parent, &parent)) != NFS_OK){
		return error;
	}

	bzero(&ck, sizeof(ck));
	ck.volume_id = parent.f_d.fattr.fsid;
	ck.file_id = parent.f_d.fattr.fileid;
	ck.file_gen = parent.f_d.generation;

	/*
	 * fh_delete will fail one way or another since object isn't on disk
	 */
	fh_delete(&ck, map.f_d.name);

	return NFS_OK;
}

nfsstat
rexx_get_path(rm)
	struct RexxMsg *rm;
{
	char path[NFS_MAXPATHLEN];
	mapentry map, l;
	nfsstat error;
	cookie ck;
	map_id n;

	if((error = get_map(LABEL_MAP, &l)) != NFS_OK){
		return error;
	}

	/*
	 * if n is LABEL, BITMAP, and unused maps - handle them here
	 */
	n = (map_id)atoi(rm->rm_Args[1]);
	if(n == LABEL_MAP){
		char *p = "LABEL";

		if(rm->rm_Action & RXFF_RESULT){
			rm->rm_Result2 = (long)CreateArgstring(p, strlen(p));
		}
		return NFS_OK;
	}
	if(n >= l.label.bitmap && n < (l.label.bitmap + l.label.bsize)){
		char *p = "BITMAP";

		if(rm->rm_Action & RXFF_RESULT){
			rm->rm_Result2 = (long)CreateArgstring(p, strlen(p));
		}
		return NFS_OK;
	}
	if(ISFREE(n)){
		char *p = "-unused-";

		if(rm->rm_Action & RXFF_RESULT){
			rm->rm_Result2 = (long)CreateArgstring(p, strlen(p));
		}
		return NFS_OK;
	}

	/*
	 * Read map to find out what we have
	 */
	if((error = get_map(n, &map)) != NFS_OK){
		return error;
	}
	if(IS_ROOT(n) && map.f_d.name[0]=='\0'){
		char *p = "EMPTY-EXPORT-SLOT";

		rm->rm_Result2 = (long)CreateArgstring(p, strlen(p));
		return NFS_OK;
	}
	bzero(&ck, sizeof(ck));
	ck.volume_id = map.f_d.fattr.fsid;
	ck.file_id = map.f_d.fattr.fileid;
	ck.file_gen = map.f_d.generation;
	if((error = fh_buildpath(&ck, path, (char *)0)) != NFS_OK){
		return error;
	}
	if(rm->rm_Action & RXFF_RESULT){
		rm->rm_Result2 = (long)CreateArgstring(path, strlen(path));
	}

	return NFS_OK;
}
