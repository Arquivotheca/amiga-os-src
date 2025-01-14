/*
 * mapentry cache
 */

#define DEBUG 1

#include <exec/types.h>
#include <exec/memory.h>
#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <fcntl.h>
#include <dos/dos.h>
#include <string.h>
#include "nfsd.h"
#include "debug.h"

struct cacheentry {
	mapentry map;			/* copy of on-disk map entry	*/
	struct cacheentry *next;	/* link for cache		*/
	BPTR fl;			/* if Lock()'ed, pointer to lock*/
	BPTR fh;			/* if Open()'ed, pointer to fh	*/
	char locked;			/* entry in use, don't flush	*/
	char dirty;			/* entry is dirty		*/
};
typedef struct cacheentry cacheentry;

#define FFSID 		f_d.fattr.fsid
#define FID 		f_d.fattr.fileid
#define ID 		map.FID
#define FSID 		map.FFSID

#define HTSIZE		64
#define GETFREE(ca) 	{(ca) = cafree; if(ca){cafree=(ca)->next;}else{ca=getfree();}}
#define FREE(ca) 	{(ca)->next = cafree; cafree = (ca);}
#define HASH(id)	((id) & (HTSIZE - 1))
#define ENTER(ca)	enter(ca)
#define DIRTY(ca)	(ca)->dirty++

map_id maxfs;
static cacheentry *cafree, *cache, *htable[HTSIZE]; /* cache variables	*/

/*
 * Get free entry in cache.  LRU things as needed to get free entry
 */
static
cacheentry *getfree()
{
	register cacheentry *ca, *nxt;
	int idx, cnt, start;
	nfstime t;

	NFStime(&t);
	start = idx = t.seconds & (HTSIZE - 1);
	cnt = 0;
	do {
		for(ca = htable[idx]; ca; ca = nxt){
			nxt = ca->next;
			if(ca->dirty){
				write_map(ca->map.f_d.fattr.fileid, &ca->map);
			}
			if(ca->fl){
				nfs_unlock(ca->fl);
			}
			if(ca->fh){
				nfs_close(ca->fh);
			}
			bzero(ca, sizeof(*ca));
			ca->next = cafree;
			cafree = ca;
			cnt++;
		}
		htable[idx] = (cacheentry *)0;
		idx = (idx + 1) & (HTSIZE - 1);
	} while(idx != start && cnt < 50);

	if(ca = cafree){
		cafree = ca->next;
	}

	return ca;
}

static void enter(register cacheentry *ca)
{
	unsigned short hashent;

	hashent = HASH(ca->ID);
	ca->next = htable[hashent];
	htable[hashent] = ca;
}

/*
 * get root directory of specified volume
 */
nfsstat
get_root_dir(volume, ck)
	char	*volume;
	cookie	*ck;
{
	nfsstat error;
	mapentry r, l;
	map_id n;

	bzero(ck, sizeof(*ck));
	if((error = get_map(LABEL_MAP, &l)) != NFS_OK){
		return error;
	}

	for(n = l.label.fs; n < (l.label.numfs + l.label.fs); n++){
		if((error = get_map(n, &r)) != NFS_OK){
			return error;
		}

		if(stricmp(r.f_d.name, volume) == 0){
			ck->volume_id = r.f_d.fattr.fsid;
			ck->file_id = r.f_d.fattr.fileid;
			ck->file_gen = r.f_d.generation;
			return NFS_OK;
		}
	}

	return NFSERR_NOENT;
}

/*
 * Lookup name in given directory
 */
nfsstat
get_map_name(ck, name, mapp)
	cookie *ck;
	register char *name;
	register mapentry *mapp;
{
	register map_id i;
	nfsstat error;

	DB("cache:get_map_name\n") ;
	DB("  name = ") ;
	DB(name) ;
	DB("\n") ;
	
	if((error = GET_MAP(ck, mapp)) != NFS_OK){
		return error;
	}
	if(mapp->f_d.fattr.type != NFDIR){
		return NFSERR_NOTDIR;
	}

	/*
	 * Fake "", ".", and ".." using links stored in map
	 */
	if(name[0] == 0 || (name[0]=='.' && name[1] == '\0')){
		return NFS_OK;
	}
	if(name[0] == '.' && name[1] == '.' && name[2] == 0){
		if(mapp->f_d.parent != LABEL_MAP){
			if((error = get_map(mapp->f_d.parent, mapp)) != NFS_OK){
				return error;
			}
		}
		return NFS_OK;
	}

	for(i = mapp->f_d.child; i != 0; i = mapp->f_d.next){
		if((error = get_map(i, mapp)) != NFS_OK){
			return error;
		}

		if(ck->volume_id != mapp->FFSID){
			return NFSERR_STALE;
		}

		if(stricmp(name, mapp->f_d.name) == 0){
			break;
		}
	}

	return (i==0) ? NFSERR_NOENT:NFS_OK;
}

/*
 * GET_MAP - get mapentry, check fsid, generation number
 */
nfsstat
GET_MAP(ck, mapp)
	register cookie	*ck;
	register mapentry *mapp;
{
	nfsstat error;

	if((error = get_map(ck->file_id, mapp)) != NFS_OK){
		return error;
	}

	if(ck->volume_id!=mapp->FFSID || ck->file_gen!=mapp->f_d.generation){
		return NFSERR_STALE;
	}

	return NFS_OK;
}

/*
 * Get map specified by id.  This procedure does not check generation number.
 */
nfsstat
get_map(id, mapp)
	register map_id id;
	register mapentry *mapp;
{
	register cacheentry *ca;
	nfsstat error;

	for(ca = htable[HASH(id)]; ca; ca = ca->next){
		if(ca->map.fattr.fileid == id){
			break;
		}
	}

	error = NFS_OK;
	if(!ca){
		GETFREE(ca);
		if(!ca){
			return NFSERR_NOSPC;
		}

		error = read_map(id, &ca->map);
		/*
		 * Some blocks may not have their fileid field setup right,
		 * so set it here before it infects the cache.
		 */
		ca->map.f_d.fattr.fileid = id;
		if(error == NFS_OK){
			ENTER(ca);
		} else {
			FREE(ca);
			ca = 0;
		}
	}

	if(ca){
		*mapp = ca->map;
	}

	return error;
}

/*
 * update_map - force update of cache copy of map or disk copy if none in cache
 */
nfsstat
update_map(mapp)
	register mapentry *mapp;
{
	register cacheentry *ca;

	DB("cache:update_map\n") ;
	for(ca = htable[HASH(mapp->fattr.fileid)]; ca; ca = ca->next){
		if(ca->map.fattr.fileid == mapp->fattr.fileid){
			break;
		}
	}

	/*
	 * Update disk version if none in cache
	 */
	if(!ca){
		return write_map(mapp->fattr.fileid, mapp);
	}

	DIRTY(ca);
	ca->map = *mapp;
	return NFS_OK;
}

/*
 * update the in ram and on disk copies of the specified mapentry
 */
nfsstat
put_map(mapp)
	register mapentry *mapp;
{
	register cacheentry *ca;

	DB("cache:putmap\n") ;
	for(ca = htable[HASH(mapp->fattr.fileid)]; ca; ca = ca->next){
		if(ca->map.fattr.fileid == mapp->fattr.fileid){
			break;
		}
	}

	/*
	 * Update in core version, if we have one, else do the one on disk
	 */
	if(ca){
		ca->map = *mapp;
	}

	return write_map(mapp->fattr.fileid, mapp);
}

/*
 * Get a new map entry from the specified filesystem.  The mapentry is
 * returned with the following fields initialized:
 *
 *	fattr.nlink = 1
 *	fattr.fsid = fsid
 * 	fattr.atime = fattr.ctime = fattr.mtime = current time
 *	mapid = id of new map
 *	parent = id of parent
 *
 * Note that <fsid, id> must refer to a directory that the new mapentry
 * will be created in.  This proc can fail for variety of reasons, eg
 * no space on device, readonly filesystem, <fsid, id> not directory,
 * etc.
 */
nfsstat
get_new_map(ck, child)
	register cookie *ck;
	mapentry *child;
{
	mapentry parent;
	nfsstat err;

	if((err = GET_MAP(ck, &parent)) != NFS_OK){
		return err;
	}
	if(parent.f_d.fattr.type != NFDIR){
		return NFSERR_NOTDIR;
	}

	bzero(child, sizeof(*child));
	child->f_d.fattr.fileid = get_free_map();
	if(child->f_d.fattr.fileid == LABEL_MAP){
		return NFSERR_NOSPC;
	}
	child->f_d.fattr.fsid = parent.f_d.fattr.fsid;
	child->f_d.fattr.nlink = 1;
	NFStime(&child->f_d.fattr.ctime);
	child->f_d.generation = child->f_d.fattr.ctime.seconds;
	child->f_d.fattr.atime = child->f_d.fattr.ctime;
	child->f_d.fattr.mtime = child->f_d.fattr.ctime;
	child->f_d.parent = parent.f_d.fattr.fileid;
	child->f_d.next = parent.f_d.child;
	child->f_d.fattr.blocksize = parent.f_d.fattr.blocksize;

	parent.f_d.child = child->f_d.fattr.fileid;
	parent.f_d.fattr.nlink++;

	if((err = put_map(child)) != NFS_OK){
		MAKEFREE(child->f_d.fattr.fileid);
		return err;
	}
	NFStime(&parent.f_d.fattr.ctime);
	if((err = put_map(&parent)) != NFS_OK){
		MAKEFREE(child->f_d.fattr.fileid);
		return err;
	}

	return NFS_OK;
}

/*
 * Return read access FileLock corresponding to cookie
 */
nfsstat
get_lock(ck, lkp)
	register cookie	*ck;
	BPTR *lkp;
{
	char path[NFS_MAXPATHLEN];
	register cacheentry *ca;
	nfsstat error;
	mapentry map;

	*lkp = 0;
	do { /* while not in cache */
		for(ca = htable[HASH(ck->file_id)]; ca; ca = ca->next){
			if(ca->ID == ck->file_id){
				break;
			}
		}
		if(!ca){
		 	if((error = GET_MAP(ck, &map)) != NFS_OK){
				return error;
			}
		}
	} while(!ca);

	/*
	 * fh_buildpath can cause cache flush, so lock *ca first before
	 * making path
	 */
	error = NFS_OK;
	if(ca->fl == 0){
		ca->locked = 1;
		if((error = fh_buildpath(ck, path, 0L)) != NFS_OK){
			ca->locked = 0;
			return error;
		}
		ca->locked = 0;
		ca->fl = nfs_lock(path, MODE_OLDFILE);
		if(!ca->fl){
			error = io_to_nfserr();
		}
	}

	*lkp = ca->fl;
	return error;
}

/*
 * Return FileHandle corresponding to cookie
 */
nfsstat
get_fd(ck, fhp)
	register cookie	*ck;
	BPTR *fhp;
{
	char path[NFS_MAXPATHLEN];
	register cacheentry *ca;
	nfsstat error;
	mapentry map;

	*fhp = 0;
	do { /* while not in cache */
		for(ca = htable[HASH(ck->file_id)]; ca; ca = ca->next){
			if(ca->ID == ck->file_id && ca->FSID == ck->volume_id){
				break;
			}
		}
		if(!ca && (error = GET_MAP(ck, &map)) != NFS_OK){
			return error;
		}
	} while(!ca);

	/*
	 * fh_buildpath can cause cache flush, so lock *ca first before
	 * making path
	 */
	error = NFS_OK;
	if(!ca->fh){
		ca->locked = 1;
		if((error = fh_buildpath(ck, path, 0L)) != NFS_OK){
			ca->locked = 0;
			return error;
		}
		ca->locked = 0;
		ca->fh = nfs_open(path, MODE_OLDFILE);
		if(!ca->fh){
			error = io_to_nfserr();
		}
	}

	*fhp = ca->fh;
	return error;
}

/*
 * Allocate and initialize cache.
 */
init_cache(rm)
	struct RexxMsg *rm;
{
	int cache_size, num_entries, cacheentries;
	register cacheentry *ca;
	nfsstat error;
	mapentry lab;

	cacheentries = atoi(rm->rm_Args[1]);
	if(cacheentries < 0){
		cacheentries = 64;
	}

	cache_size = cacheentries*sizeof(cacheentry);
	for(; cache_size > 0; cache_size -= 16*sizeof(cacheentry)){
		cafree = cache = (cacheentry *)calloc(cache_size, 1);
		if(cache){
			num_entries = cache_size / sizeof(cacheentry);
			for(ca = cache; ca < (cache + num_entries - 1); ca++){
				ca->next = ca + 1;
			}
			break;
		}
	}

	if((error = get_map(LABEL_MAP, &lab)) != NFS_OK){
		return error;
	}

	maxfs = lab.label.fs + lab.label.numfs - 1;

	return NFS_OK;
}

/*
 * Close file handle/lock/dirty page caches for a particular map.  This
 * is usually done as a precursor to an operation that updates the on
 * disk heirarchy, eg delete file/dir, makedir, etc.
 */
void
close_cache(mapp)
	register mapentry *mapp;
{
	register cacheentry *ca;

	for(ca = htable[HASH(mapp->f_d.fattr.fileid)]; ca; ca = ca->next){
		if(ca->map.f_d.fattr.fileid == mapp->f_d.fattr.fileid){
			break;
		}
	}
	if(ca){
		if(ca->dirty){
			write_map(ca->map.f_d.fattr.fileid, &ca->map);
			ca->dirty = 0;
		}
		if(ca->fl){
			nfs_unlock(ca->fl);
			ca->fl = 0;
		}
		if(ca->fh){
			nfs_close(ca->fh);
			ca->fh = 0;
		}
	}
}

/*
 * flush_open - run through cache and unlock/close objects.  If the machine
 * is used as more than an NFS server, this should be done frequently so
 * that the local user isn't locked out of objects for too long.
 */
nfsstat
flush_open(rm)
	struct RexxMsg *rm;
{
	register cacheentry *ca;
	register int i;

	for(i = 0; i < HTSIZE; i++){
		for(ca = htable[i]; ca; ca = ca->next){
			if(ca->dirty){
				write_map(ca->map.f_d.fattr.fileid, &ca->map);
				ca->dirty = 0;
			}
			if(ca->fl){
				nfs_unlock(ca->fl);
				ca->fl = 0;
			}
			if(ca->fh){
				nfs_close(ca->fh);
				ca->fh = 0;
			}
		}
	}

	return NFS_OK;
}

/*
 * close any open files/locks in cache, deallocate cache memory
 */
void
clean_cache()
{
	int i;

	if(cache){
		flush_open((struct RexxMsg *)0);
		free(cache);
		cache = 0;
		for(i = 0; i < HTSIZE; i++){
			htable[i] = 0;
		}
		cafree = cache = 0;
	}
}

