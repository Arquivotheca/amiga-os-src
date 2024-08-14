/*
 * procedures used to build the NFS mapping file.
 */

#include <rpc/rpc.h>
#include <nfs/nfs.h>
// #include <rpcsvc/mount.h>
#include <stdio.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <string.h>


#include "nfsd.h"

#define IS_DIR(x) ((x) > 0)
#define BLOCKSIZE 512

static u_int perm_amiga_to_unix(LONG);

struct maplist {
	struct maplist *next;
	mapentry map;
};

static struct FileInfoBlock *fib;

nfsstat
build_map(fsid, lk, parent)
	fs_id	fsid;
	long	lk;
	map_id	parent;
{
	BPTR tlk, oldlk;
	struct maplist *head, **next, *p, *np;
	nfsstat error;
	mapentry map;
	map_id child;
	int nlink;

	if(nfs_examine(lk, fib) == DOSFALSE){
		return NFSERR_IO;
	}

	if(!IS_DIR(fib->fib_EntryType)){
		return NFSERR_NOTDIR;
	}

	oldlk = CurrentDir(lk); next = &head; *next = 0;

	while(nfs_exnext(lk, fib)){
		*next = (struct maplist *)calloc(1, sizeof(*p));
		if(!(p = *next)){
			return NFSERR_NOSPC;
		}

		p->map.f_d.fattr.fileid = get_free_map();
		if(p->map.f_d.fattr.fileid == LABEL_MAP){
			for(p = head; p; p = np){
				np = p->next;
				free(p);
			}
			return NFSERR_NOSPC; /* ran out of free maps */
		}

		if(IS_DIR(fib->fib_EntryType)){
			p->map.f_d.fattr.type = NFDIR;
			p->map.f_d.fattr.mode = NFSMODE_DIR | 0777; /* changed from 755 MMH */
			p->map.f_d.fattr.nlink = 2;
			p->map.f_d.fattr.size = 512;
		} else {
			p->map.f_d.fattr.type = NFREG;
			p->map.f_d.fattr.mode = NFSMODE_REG | perm_amiga_to_unix(fib->fib_Protection);
			p->map.f_d.fattr.nlink = 1;
			p->map.f_d.fattr.size = fib->fib_Size;
		}
		p->map.f_d.fattr.blocksize = BLOCKSIZE;
		p->map.f_d.fattr.blocks = BLOCKS(&p->map,p->map.f_d.fattr.size);
		p->map.f_d.fattr.uid = -2;
		p->map.f_d.fattr.gid = -2;
		amiga_to_nfstime(&fib->fib_Date, &p->map.f_d.fattr.ctime);
		p->map.f_d.fattr.atime = p->map.f_d.fattr.ctime;
		p->map.f_d.fattr.mtime = p->map.f_d.fattr.ctime;
		p->map.f_d.parent = parent;
		strcpy(p->map.f_d.name, fib->fib_FileName);
		p->map.f_d.generation = p->map.f_d.fattr.ctime.seconds + parent;
		p->map.f_d.fattr.fsid = fsid;
		p->next = 0;  /* already is, but to be explicit */
		next = &p->next;
	}

	nlink = 0; child = 0;
	for(p = head; p; p = p->next){
		if(child == 0){
			child = p->map.f_d.fattr.fileid;
		}
		if(p->next){
			p->map.f_d.next = (p->next)->map.f_d.fattr.fileid;
		}
		put_map(&p->map);
		nlink++;
	}

	for(p = head; p; p = np){
		if(p->map.f_d.fattr.type == NFDIR){
			if(tlk = nfs_lock(p->map.f_d.name, ACCESS_READ)){

				error = build_map(fsid, tlk, p->map.f_d.fattr.fileid);
				nfs_unlock(tlk);
				if(error != NFS_OK){
					return error;
				}

				error = get_map(p->map.f_d.fattr.fileid, &map);
				if(error != NFS_OK){
					return error;
				}
			} else {
				return io_to_nfserr();
			}
		}
		np = p->next;
		free(p);
	}

	CurrentDir(oldlk);

	/*
	 * patch child/nlink in parent dir
	 */
	if((error = get_map(parent, &map)) == NFS_OK){
		map.f_d.child = child;
		map.f_d.fattr.nlink += nlink;
		put_map(&map);
	}

	return error;
}

/*
 * Build the NFS directory for volume <entry>
 */

nfsstat build_volume(struct RexxMsg *rm)
{
	BPTR lk;
	nfsstat error;
	mapentry r, l;
	map_id vol;

	if((error = get_map(LABEL_MAP, &l)) != NFS_OK){
		return error;
	}
	for(vol = (l.label.fs + l.label.numfs)-1; vol >= l.label.fs; vol--){
		if((error = get_map(vol, &r)) != NFS_OK){
			return error;
		}
		if(r.f_d.name[0] == 0){
			break;
		}
	}

	if(vol < l.label.fs ){
		return error;
	}

	/*
	 * Build root directory for this volume
	 */
	bzero(&r, sizeof(r));
	r.f_d.fattr.type = NFDIR;
	r.f_d.fattr.mode = NFSMODE_DIR | 0777; /* changed from 755 by MMH */
	r.f_d.fattr.nlink = 2;
	r.f_d.fattr.uid = r.f_d.fattr.gid = -2;
	r.f_d.fattr.size = BLOCKSIZE;
	r.f_d.fattr.blocksize = BLOCKSIZE;
	r.f_d.fattr.rdev = -1;
	r.f_d.fattr.blocks = BLOCKS(&r, r.f_d.fattr.size);
	r.f_d.fattr.fileid = vol;
	NFStime(&r.f_d.fattr.ctime);
	r.f_d.fattr.fsid = r.f_d.fattr.ctime.seconds;
	r.f_d.fattr.atime = r.f_d.fattr.ctime;
	r.f_d.fattr.mtime = r.f_d.fattr.ctime;
	r.f_d.generation = r.f_d.fattr.fsid;
	strcpy(r.f_d.name, rm->rm_Args[1]);
	if((error = put_map(&r)) != NFS_OK){
		return error;
	}

	/*
	 * recurse through directories building map file
	 */
	fib = (struct FileInfoBlock *)AllocMem(sizeof(*fib), MEMF_PUBLIC);
	if(fib == (struct FileInfoBlock *)NULL){
		return NFSERR_NOSPC;
	}

	lk = nfs_lock(rm->rm_Args[1], ACCESS_READ);
	error = build_map(r.f_d.fattr.fsid, lk, vol);
	nfs_unlock(lk);
	if(error != NFS_OK){
		FreeMem(fib, sizeof(*fib)); fib = 0;
		return error;
	}

	FreeMem(fib, sizeof(*fib));
	fib = 0;

	return NFS_OK;
}

#include <nfs/perm.h>

static u_int perm_amiga_to_unix(LONG amiga_perm)
{
	u_int	unix_perm=0777;

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

