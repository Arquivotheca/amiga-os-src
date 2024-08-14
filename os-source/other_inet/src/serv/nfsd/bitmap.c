/*
 * mapentry bitmap management
 */

#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <fcntl.h>

#include "nfsd.h"

char *freemap;		/* pointer to bitmap 		*/
static int mapsize;	/* number of longs in map	*/

/*
 * flush bitmap cache to disk file.  Should be done once every 30 seconds
 * or so.
 */
nfsstat
flush_bitmap(struct RexxMsg *rm)
{
	nfsstat error;
	mapentry l;
	map_id n;
	int bmap;

	if((error = read_map((map_id)LABEL_MAP, &l)) != NFS_OK){
		return error;
	}

       	bmap = 0;
	for(n = l.label.bitmap; n < (l.label.bsize+l.label.bitmap); n++){
		if((error = write_map(n, ((mapentry *)freemap) + bmap)) != NFS_OK){
			return error;
		}
		bmap++;
	}

	return NFS_OK;
}

/*
 * init_bitmap is called after the NFS mapping file is established in
 * order to read in the bitmap.  The bitmap will remain in ram and is
 * updated write-through cache style.
 */
nfsstat
init_bitmap()
{
	nfsstat error;
	mapentry l;
	map_id n;
	int bmap;

	if((error = read_map((map_id)LABEL_MAP, &l)) != NFS_OK){
		return error;
	}

	if(freemap == 0){
		mapsize = (l.label.bsize * MAPSIZE) / sizeof(freemap[0]);
		freemap = (char *)calloc(1, mapsize * sizeof(freemap[0]));
		if(freemap == 0){
			return NFSERR_NOSPC;
		}
	}

	bmap = 0;
	for(n = l.label.bitmap; n < (l.label.bsize+l.label.bitmap); n++){
		if((error = read_map(n, ((mapentry *)freemap) + bmap)) != NFS_OK){
			return error;
		}
		bmap++;
	}

	return NFS_OK;
}

/*
 * Given a number of mapentries, return the corresponding number of
 * bitmap blocks.
 */
int
num_bitmap(num_maps)
{
	int bits_per_map;

	bits_per_map = MAPSIZE*8;
	return (num_maps + bits_per_map - 1) / bits_per_map;
}

/*
 * get a free mapping entry; if none found, return map entry of the LABEL
 */
map_id
get_free_map()
{
	register int i, j;
	map_id n;

	for(i = 0; i < mapsize; i++){
		if(freemap[i]){
			n = i * (1 << SHIFT);
			for(j = 0; j < (1 << SHIFT); j++, n++){
				if(ISFREE(n)){
					MAKEUSED(n);
					return n;
				}
			}
		}
	}

	return LABEL_MAP;
}

/*
 * Free the bitmap
 */
void clean_bitmap()
{
	if(freemap){
		flush_bitmap((struct RexxMsg *)0);
		free(freemap);
		freemap = 0;
		mapsize = 0;
	}
}

