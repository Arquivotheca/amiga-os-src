/* Cluster.c **************************************************************
** Copyright 1991 CONSULTRON
*
*      Cluster and FAT related functions.
*
*************************************************************************/

#include "FS:FS.h"

extern struct ExecBase  *SysBase;
extern struct FS     *fsys;
extern int  DOSerror;

/*******
* usedblocks() -- Determine # of used blocks for entire disk from the FAT
*******/
ULONG usedblocks()
{
    register LONG endclust, usedclust=0, clust;

    endclust = fsys->f_end_clust;
/* cycle through the clusters */
    for(clust=START_CLUST; clust<=endclust; clust++)
    {
        if( readFATentry(fsys,clust) ) usedclust++;
    }

    return((fsys->f_beg_data - fsys->f_beg_part) + (usedclust * fsys->f_blocks_clust) );
}


/*******
* usedfileblocks() -- Determine # of used blocks for file or dir from FAT
*****/
ULONG usedfileblocks(clust)
LONG clust;
{
    LONG usedclust=1, cluster = clust, oldclust=0;

    while( (FAT_BAD > (cluster = readFATentry(fsys,cluster)))
        && (cluster != FAT_FREE) )
    {
        if(oldclust == cluster) return(0);       /* something went wrong! */
        if(usedclust++ == 10) oldclust = cluster;   /* check if FAT recursive */
    }

    return( (ULONG)(usedclust * fsys->f_blocks_clust) );
}


/*******
* AllocCluster() -- Allocate next Free FAT entry from FAT and link the
*   new entry into the end of the FAT entry chain supplied.
*   If the entry supplied >= FAT_EOF_LOW then do not link.
*
*   Return cluster # of next free FAT entry. If '= 0' then no free entries
*   availiable (Disk Full).
*******/
LONG AllocCluster(cluster)
LONG cluster;
{
    LONG endclust, freeclust, nxtclust;

    if(cluster<=ROOTDIR_CLUST)
    {   /* In root directory, cannot allocate any more clusters in this directory */
        DOSerror = ERROR_DISK_FULL;
        return(0);    /* no free entries availiable in root directory */
    }
    endclust = fsys->f_end_clust;
/* find next free cluster.  Start with the ffFATent (first free FAT entry) value. */
    for(freeclust=fsys->f_ffFATent; freeclust<=endclust; freeclust++)
    {
        if( readFATentry(fsys,freeclust) == FAT_FREE )
        {
            fsys->f_ffFATent = freeclust;
            break;      /* found a free cluster */
        }
    }
    if(freeclust>=endclust)
    {   /* found the free cluster # >= last cluster number in the disk/partition. */
        DOSerror = ERROR_DISK_FULL;
        return(0);    /* no free entries availiable */
    }

    if(cluster < FAT_EOF_LOW)
    {       /* The new cluster to be linked to the cluster linked list */
    /* Follow the linked cluster list until the end (FAT_EOF) is found */
        while((FAT_BAD > (nxtclust = readFATentry(fsys,cluster)))
            && (nxtclust != FAT_FREE) )
        {
            cluster = nxtclust;
        }
        if(nxtclust < FAT_EOF_LOW)
        {
            DOSerror = 20;
            return(0);              /* something is wrong */
        }
        writeFATentry(fsys,cluster,freeclust);  /* link the new cluster on the end */
    }

    /* Actually allocate new cluster */
    writeFATentry(fsys,freeclust,FAT_EOF);  /* make the new cluster the FAT_EOF */
    fsys->f_used_blocks += fsys->f_blocks_clust;    /* inc the # of usedblocks */

    return(freeclust);
}


/*******
* FreeClusters() -- Free all clusters in FAT entry chain supplied.
*
*   Return number of clusters freed or = 0 if error occurred.
*******/
LONG FreeClusters(cluster)
LONG cluster;
{
    LONG nxtclust;
    UWORD numclusts=0;

    if(cluster<=ROOTDIR_CLUST)
    {
        DOSerror = ERROR_DIRECTORY_NOT_EMPTY;
        return(0);                  /* cannot free Root directory clusters */
    }
    while( (cluster < FAT_BAD) && (cluster != FAT_FREE) )
    {
        if(cluster < (fsys->f_ffFATent))
        {
            fsys->f_ffFATent = cluster;
        }

        nxtclust = readFATentry(fsys,cluster);
/* Actually deallocate cluster */
        writeFATentry(fsys,cluster,FAT_FREE);
        cluster = nxtclust;
        numclusts++;
    }

    if(nxtclust < FAT_EOF_LOW)
    {
        DOSerror = RETURN_ERROR;
    }

    fsys->f_used_blocks -= (numclusts * fsys->f_blocks_clust); /* dec # of blocks used */
    return((LONG)numclusts);
}


/**************
* ConvertCluster() -- Convert Cluster and directory entry #s to block #.
*   Block # is ALWAYS cluster aligned.
*   Return # or -1 if error.
*******/
ULONG ConvertCluster(cluster,dirent)
LONG cluster;
WORD dirent;
{
    int i;
    ULONG block;
    LONG clust;
    LONG clust_mult = dirent/fsys->f_dirents_clust;

    if( 0 <= (clust = ROOTDIR_CLUST - cluster) )
    {           /** Cluster of root dir (cluster <= 0) **/
DPRTF(KPrintF("\nConvertCluster: cluster=%ld  clust=%ld",cluster,clust));
        if(fsys->f_beg_data <= (block = ((clust+clust_mult) * fsys->f_blocks_clust) + fsys->f_beg_root_dir))
            return(-1L);
    }
    else
    {           /* Cluster of data area of disk/partition. (cluster > 0) */
        for(i=clust_mult; i>0; i--)
        {   /* cluster in data portion of disk/partition.*/
            /* Search through FAT entry list for the cluster # specified by the
                directory entry relative to the cluster # supplied */
            if( (FAT_BAD <= (cluster = readFATentry(fsys,cluster)))
                    || (cluster == FAT_FREE)
                    || (cluster > fsys->f_end_clust) )  /* check if it exceeds the end cluster */
                return(-1L);
        }
        block = ((cluster-START_CLUST) * fsys->f_blocks_clust) + fsys->f_beg_data;
    }
    return(block);
}
