#include <exec/types.h>
#include <exec/memory.h>
#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <dos/dos.h>
#include <string.h>

#include "nfsd.h"
#include "stat.h"

static BPTR mapfh;

static setvar(register struct RexxMsg *rm, char *basename, char *name, int value);


/*
 * read specified mapentry off disk.  No caching is done here.
 */
nfsstat
read_map(id, mapp)
	register map_id id;
	register mapentry *mapp;
{
	unsigned long fsize;
	if(mapfh == 0){
		return NFSERR_IO;
	}

	if(nfs_seek(mapfh, MAP_OFFSET(id), OFFSET_BEGINNING) == -1){
		nfs_seek(mapfh, 0L, OFFSET_END);
		fsize = nfs_seek(mapfh, 0L, OFFSET_CURRENT);
		return (fsize != -1 && fsize < MAP_OFFSET(id))? NFSERR_STALE:NFSERR_IO;
	}

	if(nfs_rd(mapfh, mapp, MAPSIZE) != MAPSIZE){
		return NFSERR_IO;
	}
	return NFS_OK;
}

/*
 * write the specified map to disk
 */
nfsstat
write_map(id, mapp)
	register map_id id;
	register mapentry *mapp;
{
	if(mapfh == 0){
		return NFSERR_IO;
	}

	if(nfs_seek(mapfh, MAP_OFFSET(id), OFFSET_BEGINNING) == -1){
		unsigned long fsize;

		nfs_seek(mapfh, 0L, OFFSET_END);
		fsize = nfs_seek(mapfh, 0L, OFFSET_CURRENT);
		return (fsize != -1 && fsize < MAP_OFFSET(id))? NFSERR_STALE:NFSERR_IO;
	}

	if(nfs_wr(mapfh, mapp, MAPSIZE) != MAPSIZE){
		return NFSERR_IO;
	}

	return NFS_OK;
}

/*
 * Build NFS directory file - called from Rexx.
 */
build_map_file(rm)
	struct RexxMsg *rm;
{
	extern char iobuf[NFS_MAXDATA];
	int m, mapentries;
	nfsstat error;
	mapentry l;
	map_id n;

	if((mapfh = nfs_open(rm->rm_Args[1], MODE_NEWFILE)) < 0){
		return io_to_nfserr();
	}
	mapentries = atoi(rm->rm_Args[2]);

	/*
	 * zero map file
	 */
	bzero(iobuf, sizeof(iobuf));
	for(n = 0; n < mapentries; n += m){
		m = min((NFS_MAXDATA/MAPSIZE), (mapentries - n));
		if(nfs_wr(mapfh, iobuf, m*MAPSIZE) < 0){
			return io_to_nfserr();
		}
	}

	/*
	 * Construct label
	 */
	bzero(&l, sizeof(l));
	l.label.fattr.type = NFNON;
	l.label.fattr.fileid = LABEL_MAP;
	l.label.fs = LABEL_MAP + 1;
	l.label.numfs = atoi(rm->rm_Args[3]);
	l.label.bitmap = l.label.fs + l.label.numfs;
	l.label.bsize = num_bitmap(mapentries);
	if((error = write_map(LABEL_MAP, &l)) != NFS_OK){
		return error;
	}

	/*
	 * Construct bitmap - mark label, filesystem, bitmap slots as used,
	 * rest of bitmap as free.
	 */
	if((error = init_bitmap()) != NFS_OK){
		return error;
	}

	MAKEUSED(LABEL_MAP);
	for(n = l.label.fs; n < (l.label.fs + l.label.numfs); n++){
		MAKEUSED(n);
	}
	for(n = l.label.bitmap; n < (l.label.bitmap + l.label.bsize); n++){
		MAKEUSED(n);
	}
	for(n = l.label.bitmap + l.label.bsize; n < mapentries; n++){
		MAKEFREE(n);
	}

	flush_bitmap((struct RexxMsg *)0);
	nfs_close(mapfh);
	mapfh = 0;
	return NFS_OK;
}

/*
 * Read in or build mapfile.
 */
nfsstat
init_map(rm)
	struct RexxMsg *rm;
{
	nfsstat error;
	char buf[16];
	mapentry l;
	long fsize;

	mapfh = nfs_open(rm->rm_Args[1], MODE_OLDFILE);
	if(mapfh == 0){
		return io_to_nfserr();
	}

	if((error = read_map(LABEL_MAP, &l)) != NFS_OK){
		return error;
	}

	nfs_seek(mapfh, 0L, OFFSET_END);
	fsize = nfs_seek(mapfh, 0L, OFFSET_CURRENT);

	sprintf(buf, "%ld", l.label.numfs);
	SetRexxVar(rm, "NFSd.numfs", buf, strlen(buf));

	sprintf(buf, "%ld", l.label.fs);
	SetRexxVar(rm, "NFSd.fs", buf, strlen(buf));

	sprintf(buf, "%ld", fsize/MAPSIZE);
	SetRexxVar(rm, "NFSd.nummap", buf, strlen(buf));

	if((error = init_bitmap()) != NFS_OK){
		return io_to_nfserr();
	}
	return NFS_OK;
}

nfsstat
query(rm)
	register struct RexxMsg *rm;
{
	nfsstat error;
	mapentry l;
	int fsize;

	if((error = read_map(LABEL_MAP, &l)) != NFS_OK){
		return error;
	}

	nfs_seek(mapfh, 0L, OFFSET_END);
	fsize = nfs_seek(mapfh, 0L, OFFSET_CURRENT);

	setvar(rm, rm->rm_Args[1], "map_fs", l.label.fs);
	setvar(rm, rm->rm_Args[1], "map_numfs", l.label.numfs);
	setvar(rm, rm->rm_Args[1], "map_bitmap", l.label.bitmap);
	setvar(rm, rm->rm_Args[1], "map_bsize", l.label.bsize);
	setvar(rm, rm->rm_Args[1], "map_nummap", fsize/MAPSIZE);

	setvar(rm, rm->rm_Args[1], "nfsd_call", stat.nfsd_call);
	setvar(rm, rm->rm_Args[1], "nfsd_null", stat.nfsd_null);
	setvar(rm, rm->rm_Args[1], "nfsd_getattr", stat.nfsd_getattr);
	setvar(rm, rm->rm_Args[1], "nfsd_setattr", stat.nfsd_setattr);
	setvar(rm, rm->rm_Args[1], "nfsd_root", stat.nfsd_root);
	setvar(rm, rm->rm_Args[1], "nfsd_lookup", stat.nfsd_lookup);
	setvar(rm, rm->rm_Args[1], "nfsd_readlink", stat.nfsd_readlink);
	setvar(rm, rm->rm_Args[1], "nfsd_read", stat.nfsd_read);
	setvar(rm, rm->rm_Args[1], "nfsd_writecache", stat.nfsd_writecache);
	setvar(rm, rm->rm_Args[1], "nfsd_write", stat.nfsd_write);
	setvar(rm, rm->rm_Args[1], "nfsd_create", stat.nfsd_create);
	setvar(rm, rm->rm_Args[1], "nfsd_remove", stat.nfsd_remove);
	setvar(rm, rm->rm_Args[1], "nfsd_rename", stat.nfsd_rename);
	setvar(rm, rm->rm_Args[1], "nfsd_link", stat.nfsd_link);
	setvar(rm, rm->rm_Args[1], "nfsd_symlink", stat.nfsd_symlink);
	setvar(rm, rm->rm_Args[1], "nfsd_mkdir", stat.nfsd_mkdir);
	setvar(rm, rm->rm_Args[1], "nfsd_rmdir", stat.nfsd_rmdir);
	setvar(rm, rm->rm_Args[1], "nfsd_readdir", stat.nfsd_readdir);
	setvar(rm, rm->rm_Args[1], "nfsd_statfs", stat.nfsd_statfs);
	setvar(rm, rm->rm_Args[1], "nfsd_unknown", stat.nfsd_unknown);

	setvar(rm, rm->rm_Args[1], "mntd_call", stat.mntd_call);
	setvar(rm, rm->rm_Args[1], "mntd_null", stat.mntd_null);
	setvar(rm, rm->rm_Args[1], "mntd_mnt", stat.mntd_mnt);
	setvar(rm, rm->rm_Args[1], "mntd_dump", stat.mntd_dump);
	setvar(rm, rm->rm_Args[1], "mntd_umnt", stat.mntd_umnt);
	setvar(rm, rm->rm_Args[1], "mntd_umntall", stat.mntd_umntall);
	setvar(rm, rm->rm_Args[1], "mntd_export", stat.mntd_export);
	setvar(rm, rm->rm_Args[1], "mntd_exportall", stat.mntd_exportall);
	setvar(rm, rm->rm_Args[1], "mntd_unkown", stat.mntd_unknown);

	setvar(rm, rm->rm_Args[1], "rexx_call", stat.rexx_call);

	return NFS_OK;
}

static
setvar(rm, basename, name, value)
	register struct RexxMsg *rm;
	char *basename, *name;
	int value;
{
	char buf[128], buf1[16];
	register char *p;

	if(strlen(basename) + strlen(name) + 2 > sizeof(buf)){
		return NFSERR_NOSPC;
	}

	strcpy(buf, basename);
	strcat(buf, ".");
	strcat(buf, name);
	for(p = buf; *p; p++){
		if(*p >= 'a' && *p <= 'z'){
			*p = *p - 'a' + 'A';
		}
	}

	if(CheckRexxMsg(rm)){
		sprintf(buf1, "%ld", value);
		SetRexxVar(rm, buf, buf1, strlen(buf1));
	}

	return NFS_OK;
}

void
clean_map()
{
	if(mapfh){
		nfs_close(mapfh);
		mapfh = 0;
	}
}
