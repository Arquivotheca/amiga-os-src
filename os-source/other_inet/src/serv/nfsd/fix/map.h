/*
 * Map management defines
 */

/*
 * Format of mapping file is:
 *	label block
 *	NUM_FS mapentries for root dirs of each FS
 *	freelist bitmap (MAPSIZE * label.bsize)
 *	map_id's
 */

#define NUM_FS		32		/* max # of filesystems exported*/
#define FD_NAME_SIZE 	43		/* max NFS name len supported	*/

typedef union {
	fattr	fattr;			/* directly from nfs_prot.h	*/
	long	bitmap[32];		/* free bitmap			*/
	struct {
		fattr	fattr;		/* directly from nfs_prot.h	*/
		unsigned long generation;/* object generation num 	*/
		map_id	next;		/* next object in directory	*/
		map_id	parent;		/* parent dir owning this obj	*/
		map_id 	child;		/* child, if dir		*/
		char 	name[FD_NAME_SIZE+1]; /* Amiga filename		*/
	} f_d;
	struct {
		fattr	fattr;		/* directly from nfs_prot.h	*/
		map_id	fs;		/* where filesystem maps start	*/
		long	numfs;		/* number of label blocks	*/
		map_id	bitmap;		/* where bitmap starts		*/
		long	bsize;		/* bitmap size in blocks	*/
	} label;
} mapentry;

#define MAPSIZE 	128
#define MAP_OFFSET(x) 	((x)<<7)
#define LABEL_MAP	0
#define BLOCKS(mp,size) (((mp)->f_d.fattr.blocksize+size-1)/(mp)->f_d.fattr.blocksize)

extern nfsstat get_new_map(), fh_build(), fh_delete(), get_map(), put_map();
extern nfsstat get_map_name(), fh_buildpath(), getattr(), setattr();
extern nfsstat GET_MAP(), read_map(), write_map(), io_to_nfserr();
extern nfsstat fh_build(), fh_buildpath(), fh_delete(), update_map();
extern nfsstat flush_bitmap(), get_fd(), get_lock();

extern map_id maxfs;
#define IS_ROOT(x) ((x) <= maxfs)

