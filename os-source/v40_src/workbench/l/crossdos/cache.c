/* cache.c **************************************************************
** Copyright 1991 CONSULTRON
*
*      cache buffer related functions.
*
*************************************************************************/

#include "FS:FS.h"

extern struct ExecBase  *SysBase;
extern struct FS     *fsys;
extern int DOSerror;


/********
*   AllocCache() -- Release old cache memory and allocate new cache memory
*       and clear block #s assigned to cache buffers.
********/
struct Cache *AllocCache(numbuffs, datasize)
register ULONG numbuffs;
register ULONG datasize;
{
    register struct DosEnvec *fssm_Env = (struct DosEnvec *)BADDR(fsys->f_fssm->fssm_Environ);
    UBYTE *buffs;
    ULONG buff_sz=0;
    WORD oldnumbuffs  = fsys->f_num_buffs;
    UWORD i;

/* only allow the # buffers to be within the cache limits */
    numbuffs = min(MAX_CACHE_BUFFS, max(numbuffs, MIN_CACHE_BUFFS));

    if(numbuffs == oldnumbuffs) return(fsys->f_cache);  /* Already the same cache same */
/**   Deallocate old cache buffer memory pointer struct and Cache buffer memory ****/
    if( fsys->f_cache )  FreeMem(fsys->f_cache,fsys->f_cache_sz);

/* loop until cache memory finally allocated.  Divide numbuffs by 2 after each loop */
    for(; numbuffs>=MIN_CACHE_BUFFS; numbuffs >>= 1)
    {
    /**   Allocate Cache buffer memory pointer struct and Cache buffer memory ****/
        if(buffs = AllocMem( buff_sz = numbuffs * (sizeof(struct Cache) + datasize),
            fssm_Env->de_BufMemType))
        {
            break;  /* Cache memory allocated, break out of for() loop */
        }
    }

    if(numbuffs<MIN_CACHE_BUFFS)
    {
        return(0);
    }

    fsys->f_cache = (struct Cache *)buffs;
    fsys->f_cache_sz = buff_sz;

    buffs = (UBYTE *)(fsys->f_cache) + (numbuffs * sizeof(struct Cache));

    /* Load cache buffer ptrs */
    for(i=0; i<numbuffs; i++)
    {
        fsys->f_cache[i].cache_buff = buffs+(i*datasize);
        fsys->f_cache[i].cache_block = C_BLOCK_UNUSED;
    }
    fsys->f_old_buff = 0;    /* point to # of oldest buffer in cache */
    fsys->f_num_buffs = numbuffs;    /* put # of buffers in cache struct */

    return(fsys->f_cache);
}


/********
*   More_Cache() -- Allocate more cache buffers
*
*  Return total number of buffers or =0 for error
********/
ULONG More_Cache(numbuffs, datasize)
ULONG numbuffs;
ULONG datasize;
{
    Flush_Buffers();    /* make sure no writes are pending in cache */

    if( !AllocCache((numbuffs)+(fsys->f_num_buffs), datasize)) return(0);

    return((ULONG)fsys->f_num_buffs);
}


/********
*   FindBlockInCache() -- Find the specified cluster cache and pass back buffer
*   pointer to cache or zero if not found.
*
*   Return buffer pointer to cache or 0 if error.
********/
UBYTE *FindBlockInCache(block)
ULONG block;
{
    register LONG i;
    register struct Cache *cache = fsys->f_cache;

/* Check if data buffer number already allocated for it */
        /** look for requested block in cache **/
    for(i=fsys->f_num_buffs-1;
        (i>=0) && ((~C_BLOCK_MOD & cache[i].cache_block) != block);
        i--);

    if(i < 0)
    {
        return(0);
    }

/*
DPRTF(KPrintF("\nFindBlockInCache: block = %ld",(~C_BLOCK_MOD & cache[i].cache_block)));
*/
    return(cache[i].cache_buff);
}


/********
*   GetBlock() -- Get block from cache or from disk (then write to cache)
*       and pass back buffer pointer.
*       Block #s inputted must be cluster aligned.
*       The entire cluster will be read into cache memory
*
*   Return buffer pointer to cache or 0 if error.
********/
UBYTE *GetBlock(block)
ULONG block;
{
    register struct FS *fs = fsys;
    register struct Cache *cache = fs->f_cache;
    int i;
    ULONG blocks_clust = fs->f_blocks_clust;
    UBYTE *mem;


    if(0 == (mem = FindBlockInCache(block)))  /* check if already in cache */
    {
            /** Cluster not found in cache.  Get cluster from disk ***/
        i = fs->f_old_buff;    /** point to next cache buffer to update **/

        mem = cache[i].cache_buff;

            /** If cluster modified, write to disk first ***/
        if(C_BLOCK_MOD & cache[i].cache_block)
            PutBlockMem(mem,(~C_BLOCK_MOD & cache[i].cache_block),blocks_clust);

        if( GetBlockMem(mem,block, blocks_clust))
        {
            return(0);
        }

        cache[i].cache_block = block;  /* store # of block in memory */
        if( (++(fs->f_old_buff)) >= fs->f_num_buffs) fs->f_old_buff = 0;
    }
        /** Return with buffer pointer ***/
    return(mem);
}


/********
*   ReadCluster() -- Read cluster from disk or cache and pass back buffer
*   pointer to cache.  WITH Cache Optimization.
*
*   This routine is called from the ReadData() or WriteData() routines to
*   optimize disk accesses within the cache so that data clusters for a specific
*   file are found at one specific place in the cache.  This helps to prevent
*   directory clusters in the cache from being reused by file data.
*
*   The entire cluster will be read into cache memory
*
*   Return buffer pointer to cache or 0 if error.
********/
UBYTE *ReadCluster(cluster,lock)
ULONG cluster;
struct MLock *lock;
{
    register struct FS *fs = fsys;
    register UWORD oldbuff;
    UBYTE *mem;

    oldbuff = fs->f_old_buff;    /* store old buffer number */

/* Check if data buffer number already allocated for it */
    if(lock->ml_lock_ext.le_databuffnum == C_DBUFF_UNUSED)
    /* No. Use oldest buffer number */
        lock->ml_lock_ext.le_databuffnum = fs->f_old_buff;

    /* Yes. Check if databuffnum is actually a memory pointer */
    else if((ULONG)(mem = (UBYTE *)lock->ml_lock_ext.le_databuffnum) > MAX_CACHE_BUFFS)
    {
        /* Right now this only handles data that is less the one cluster in size */
        return(mem);   /* point to the memory of the default buffer memory */
    }
    /* Yes. databuffnum is actually a buffer number */
    else
    {
        fs->f_old_buff = lock->ml_lock_ext.le_databuffnum;   /* put data buffer number of file */
    }
    mem = GETCLUSTER(cluster,0);      /* Get Cluster */

    fs->f_old_buff = oldbuff;    /* put back old buffer number */

    return(mem);
}


/********
*   PutBlock() -- Put block in cache and flag cache buffer modified.
*           If block not already in cache, do GetBlock() first.
*
*   A whole cluster is written into cache memory.
*
*   Return buffer pointer to cache or 0 if error.
********/
UBYTE *PutBlock(block)
ULONG block;
{
    register struct FS *fs = fsys;
    register struct Cache *cache = fs->f_cache;
    int i;
    ULONG blocks_clust = fs->f_blocks_clust;


        /** look for requested cluster in cache **/
    for(i=0;
        (i<fs->f_num_buffs) && ((~C_BLOCK_MOD & cache[i].cache_block) != block);
        i++);

    if(i == fs->f_num_buffs)
    {
            /** Cluster not found in cache.  Allocate cluster from disk ***/
        i = fs->f_old_buff;    /** point to next cache buffer to update **/

            /** If cluster modified, write to disk first ***/
        if(C_BLOCK_MOD & cache[i].cache_block)
            PutBlockMem(cache[i].cache_buff,
                (~C_BLOCK_MOD & cache[i].cache_block),
                blocks_clust);

        cache[i].cache_block = block;  /* store # of cluster in memory */

        if( (++(fs->f_old_buff)) >= fs->f_num_buffs) fs->f_old_buff = 0;
    }

    cache[i].cache_block |= C_BLOCK_MOD;    /* set cluster modified flag */
    fs->f_Diskflags |= CACHE_MOD_F;        /* set cache modified flag */

        /* Start timer counter */
      fsys->f_timer_start = DISK_TIME_OUT;   /* set timer count to disk time-out */

        /** Return pointer to cache buffer memory ***/
    return(cache[i].cache_buff);
}

/********
*   WriteCluster() -- Write cluster to disk or cache and pass back buffer
*       pointer to cache.  WITH Cache Optimization.
*
*   This routine is called from the WriteData() routines to optimize disk
*   accesses within the cache to so that data clusters for a specific file are
*   found at one specific place in the cache.  This helps to prevent directory
*   clusters in the cache from being reused by file data.
*
*   You must assure that the cluster is already in the cache.  If not,
*   do a ReadCluster() first.
*   A whole cluster is written from cache memory.
*
*   Return buffer pointer to cache or 0 if error.
********/
UBYTE *WriteCluster(cluster,lock)
ULONG cluster;
struct MLock *lock;
{
    register struct FS *fs = fsys;
    register UWORD oldbuff;
    UBYTE *mem;

    oldbuff = fs->f_old_buff;    /* store old buffer number */

/* Check if data buffer number already allocated for it */
    if(lock->ml_lock_ext.le_databuffnum == C_DBUFF_UNUSED)
    /* No. Use oldest buffer number */
        lock->ml_lock_ext.le_databuffnum = fs->f_old_buff;

    /* Yes. Use the buffer number allocated */
      else  fs->f_old_buff = lock->ml_lock_ext.le_databuffnum;   /* put data buffer number of file */

    mem = PUTCLUSTER(cluster,0);      /* Put Cluster of data */

    fs->f_old_buff = oldbuff;    /* put back old buffer number */

    return(mem);
}


/********
*   FlushCache() -- Flush all modified clusters from cache
*********/
void FlushCache()
{
    register struct FS *fs = fsys;
    register struct Cache *cache = fs->f_cache;
    int i;
    ULONG blocks_clust = fs->f_blocks_clust;
    ULONG block;


        /** cycle through cache looking for modified buffers **/
    for(i=0; i<fs->f_num_buffs; i++)
    {
            /** If cluster modified, write to disk ***/
        if(C_BLOCK_MOD & cache[i].cache_block)
        {
            block = (cache[i].cache_block &= ~C_BLOCK_MOD);
                    /* clear cluster modified flag */
            PutBlockMem(cache[i].cache_buff,block,blocks_clust);
        }
    }

        /** Return no error ***/
    return;
}


/********
*   StoreDirEnt() -- Store Directory Entry given the cache memory pointer to
*       the Directory Entry.
*
*   It is ASSUMED that the cluster # requested is IN the data cluster area.
*
*   Return ptr to Directory Entry cache memory or = 0 for error
********/
UBYTE *StoreDirEnt(dirent)
register struct FS_dir_ent *dirent;
{
    register UBYTE *mem = (UBYTE *)dirent;
    register struct FS *fs = fsys;
    register struct Cache *cache = fs->f_cache;
    int i;
    UBYTE *rootdir_mem;
    ULONG bytes_clust = fs->f_blocks_clust<<fs->f_bytes_block_sh;


/* check in root directory cache memory first */
    if( (mem >= (rootdir_mem = fs->f_rootdir_cache))
        && (mem < (rootdir_mem + fs->f_rootdir_cache_sz)))
    {
        fs->f_Diskflags |= ROOTDIR_MOD_F;
        fs->f_timer_start = DISK_TIME_OUT;   /* set timer count to disk time-out */
        return(mem);
    }
        /** look for requested cluster in cache based on the memory address **/
    for(i=0;
        (i < fs->f_num_buffs)
        && !(( (cache[i].cache_buff) <= mem )
        && (((cache[i].cache_buff)+(bytes_clust)) > mem) );
        i++);

    if(i < fs->f_num_buffs)
    {
        if(PutBlock(~C_BLOCK_MOD & cache[i].cache_block)) return(mem);
    }
    DOSerror = RETURN_ERROR;
    return(0);
}
