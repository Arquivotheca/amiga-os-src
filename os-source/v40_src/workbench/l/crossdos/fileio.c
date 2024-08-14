/* fileIO.c **************************************************************
** Copyright 1991 CONSULTRON
*
*      DOS file IO related functions.
*
*************************************************************************/

#include "FS:FS.h"

extern struct FS     *fsys;
extern int  DOSerror;
extern struct IOStdReq  *diskreq;
extern struct trans_table *trans_tbl;
extern struct ExecBase  *SysBase;

extern UCHAR *info;
#define STRLEN_INFO 5


/*************
* OpenFile()  --- Open file.
*   Return 0 if cannot open.
*************/
int OpenFile(filehandle, dirlock, name, mode)
register struct FileHandle *filehandle;
register struct MLock *dirlock;
UCHAR *name;    /* BADDR(BSTR) */
int mode;
{
F();
    register struct MFileInfoBlock *mfib = (struct MFileInfoBlock *)fsys->f_scratch;
    register struct MFileInfoBlock *newmfib;
    register struct MLock *lock=0;
    int lockmode;
    int strln;
    UBYTE fileflags=0;


    {
            /* Find if file exists.  newmfib = mfib if found or =0 if not found */
        newmfib = FindObject(dirlock, name, mfib);

        if( DOSerror == ERROR_INVALID_COMPONENT_NAME )    /* bad filespec */
        {
            return(DOSFALSE);
        }

        if( newmfib && (mfib->mfib_DirEntStatus != MLDE_FILE))    /* if not a file, return with error */
        {
            DOSerror = ERROR_OBJECT_WRONG_TYPE;
            return(DOSFALSE);
        }

        switch(mode)
        {
            case ACTION_FINDINPUT:
                lockmode = SHARED_LOCK;
                break;
            case ACTION_FINDUPDATE:
                lockmode = SHARED_LOCK;
                if(!newmfib) goto Open_Create_New_File;
                break;
            case ACTION_FINDOUTPUT:
                lockmode = EXCLUSIVE_LOCK;
                if( newmfib )
                {
                    if( !DeleteObject(dirlock, name))
                    {
                        return(DOSFALSE);
                    }
                }
Open_Create_New_File:
                if( newmfib = CreateObject(dirlock, name, mfib))
                {
                    fileflags |= FILE_MOD_F;
                }
                break;
            case 0:                     /* if mode = 0 then this function was called by ACTION_FH_FROM_LOCK */
                lock = dirlock;         /* set the lock to the lock passed in dirlock */
                break;
            default:    return(DOSFALSE);
        }
    }

    if( !newmfib )
    {
#ifdef MFS
        if(newmfib = GetDummyObject(name, mfib));
        else
#endif
        {
            return(DOSFALSE);
        }
    }

    if(lock == 0) if(!(lock = MakeLock(dirlock, newmfib, lockmode)))
    {
        return(DOSFALSE);
    }

    lock->ml_KeyPtr->kp_flags          |= fileflags;

/*### set global text filter if FS flag set and != icon file ".info" ###*/
/* Using the utility.library equivalent to strnicmp() because with localization
it should handle case for international characters properly */
    if( (fsys->f_FSflags & (MF_GLOB_TXFLTR | MF_GLOB_TXTRANS))
           /* make sure the string length is not longer than the string to be searched */
        && ((0 > (strln = name[0]-STRLEN_INFO))
        || (0 != strnicmp(info,&name[strln+1],STRLEN_INFO))) )
    {
        if(fsys->f_FSflags & MF_GLOB_TXFLTR)
        {
DPRTF(KPrintF( "\nOpenFile: Text Filter ON"));
            lock->ml_lock_ext.le_filt_flags |= CTRLM_CTRLZ_F;
        }
        if(fsys->f_FSflags & MF_GLOB_TXTRANS)
        {
DPRTF(KPrintF( "\nOpenFile: Text Tranlator ON"));
            lock->ml_lock_ext.le_filt_flags |= XLAT1_F;
            lock->ml_lock_ext.le_trans_tbl = trans_tbl; /* set the translation table if used */
        }
    }
    lock->ml_lock_ext.le_prev_clust     =
    lock->ml_lock_ext.le_curr_clust     = lock->ml_KeyPtr->kp_start_clust;
    if(ILLEGAL_CLUST == lock->ml_KeyPtr->kp_dir_clust) lock->ml_lock_ext.le_databuffnum = lock->ml_KeyPtr->kp_dir_ent;
    else lock->ml_lock_ext.le_databuffnum = C_DBUFF_UNUSED;

  /* the following statement is VERY NECESSARY but not documented very clearly */
    filehandle->fh_Arg1 = (LONG)lock;     /* pass back FileLock */

    return(DOSTRUE);
}


#define SEEK_ERROR  -1L
#define FILE_ERROR  (ULONG)(-1L)
/**********************************************************************
*   Set_File_Size() -- Set file to the specified file position.  Zero new data.
*
*   return = -1 if error.
*          = new file size if successful.
**********************************************************************/
LONG Set_File_Size(lock,offset,mode)
register struct MLock *lock;
LONG offset;
LONG mode;
{
F();
    LONG curr_cluster,cluster,prevcluster;
    register ULONG new_eof,old_eof;
    register LONG i;
    register struct Lock_ext *lock_ext;
    ULONG filepos;
    register UBYTE *data;
    ULONG datasz;

    if(!lock)
    {
        DOSerror = ERROR_INVALID_LOCK;
        return(DOSFALSE);
    }

    if(!CompareVolNode_Write(lock)) return(DOSFALSE);

    DOSerror = ERROR_SEEK_ERROR;

    lock_ext = &(lock->ml_lock_ext);

    old_eof = lock->ml_KeyPtr->kp_file_size;
    curr_cluster = lock_ext->le_curr_clust;
    filepos = lock_ext->le_file_pos;

    switch(mode)
    {
        case OFFSET_BEGINNING:
            new_eof = offset;
            break;
        case OFFSET_CURRENT:
            new_eof = offset + filepos;
            break;
        case OFFSET_END:
            new_eof = offset + old_eof;
            break;
        default:
            DOSerror = ERROR_SEEK_ERROR;
            break;
    }

    if(old_eof > new_eof)
    {           /* Shorten file size if possible */
/**************************************************************************
For purposes of design simplification, I allow any handle owner to shorten the file
to any size even if there are handle owners of this file with current positions
larger than the new size. I can't imagine the need to shorten the file with other
handles on it at the same time.

If in the future we need to support the limiting of the shorten of the files
dependant on the other handles, we can add it in.
**************************************************************************/
    /* Set the new EOF and free unused clusters */
        cluster = lock->ml_KeyPtr->kp_start_clust;
        for(i = (new_eof)/(fsys->f_blocks_clust<<fsys->f_bytes_block_sh); i > 0; i--)
        {       /* fast-forward to current cluster */
            cluster = readFATentry(fsys,prevcluster = cluster);
            if( (cluster == FAT_BAD ) || (cluster == FAT_FREE) )
            {       /* Illegal FAT link */
                return(SEEK_ERROR);
            }
            else if( cluster >= FAT_EOF_LOW )
            {
                lock_ext->le_prev_clust = prevcluster;
            }
        }
        FreeClusters(readFATentry(fsys,prevcluster));   /* Free cluster no longer used */
        writeFATentry(fsys,prevcluster,FAT_EOF);        /* Make last cluster the EOF */

    /* Change the file size for this lock and locks on the same object */
        lock->ml_KeyPtr->kp_file_size = new_eof;

    /* If current file position of "lock" is greater than new file size, set
    current file position of "lock" to EOF */
        if(lock_ext->le_file_pos > new_eof)
        {
            filepos = new_eof;
            curr_cluster = cluster;
        }
    }
    else
    {           /* Lengthen file size if possible */
        if(SEEK_ERROR == SeekFilePos(lock,0,OFFSET_END))    /* seek to end of file */
        {
            return(SEEK_ERROR);
        }
        if((cluster = lock_ext->le_curr_clust) >= FAT_EOF_LOW) cluster = lock_ext->le_prev_clust;

        data = (UBYTE *)fsys->f_scratch;
    /** Zero all data in buffer **/
        datasz = fsys->f_scratch_sz/sizeof(ULONG);
        for(i=0; i<datasz; i++) ((ULONG *)data)[i] = 0;
        datasz = fsys->f_scratch_sz;
        for(i=(new_eof-old_eof)/(datasz); i>=0; i--)
        {
            if(i==0) datasz=(new_eof-old_eof)%(datasz);
            if(FILE_ERROR == WriteData(lock,data,datasz))
            {
                FreeClusters(readFATentry(fsys,cluster));   /* Free clusters allocated */
                writeFATentry(fsys,cluster,FAT_EOF);        /* Make last cluster the EOF */
                new_eof = SEEK_ERROR;      /* indicate seek error */
                break;      /* break out of for() loop */
            }
        }
    }

/* only set file modified bit if the EOF changed */
    if( (old_eof != new_eof) && (new_eof != SEEK_ERROR) )
    {
        lock->ml_KeyPtr->kp_flags |= FILE_MOD_F;
    }

    lock_ext->le_file_pos = filepos;
    lock_ext->le_curr_clust = curr_cluster;

    DOSerror = 0;
    return((LONG)new_eof);

}


#define JAN_01_80  0x1B00

/*************
* CloseFile()  --- Close file.
*   Return DOSFALSE if cannot close.
*************/
int CloseFile(lock)
register struct MLock *lock;
{
F();
    register struct MFileInfoBlock *mfib = (struct MFileInfoBlock *)fsys->f_scratch;
    register struct FS_dir_ent *dirent;
    UCHAR ctrlz=CTRLZ;
    ULONG volatile filesize;

    if(!lock)
    {
        DOSerror = ERROR_INVALID_LOCK;
        return(DOSFALSE);
    }

    if(lock && (lock->ml_KeyPtr->kp_flags & FILE_MOD_F))
    {
        if(!CompareVolNode_Write(lock)) return(DOSFALSE);

                /* If Ctrl M or Ctrl Z filter flag set put */
        if( lock->ml_lock_ext.le_filt_flags & CTRLM_CTRLZ_F)
        {
            SeekFilePos(lock, 0, OFFSET_END);     /* go to end of file */
            WriteData(lock, &(ctrlz), 1);     /* write CTRLZ at end of file */
        }
          mfib->mfib_DiskCluster   = (lock->ml_KeyPtr->kp_dir_clust);
          mfib->mfib_DiskDirEntry  = (lock->ml_KeyPtr->kp_dir_ent)-1; /* point to previous entry */
          mfib->mfib_DirEntStatus  = MLDE_DIR;

/*### Update date of parent directory of file ###*/
        SetDate_ParentDir(mfib);

        if(!(dirent = GetNextDirEnt(mfib)))
        {
            return(DOSFALSE);
        }

        filesize = lock->ml_KeyPtr->kp_file_size;
          SWAPLONG(dirent->fde_file_size,filesize);

            /*** set "rwed" file attributes ****/
        dirent->fde_protection = 0;

            /**** Set Current Date and Time ****/
        *((ULONG *)(dirent->fde_time)) = SetCurrentTime(); /* copy creation time */

        if( !StoreDirEnt(dirent))
        {
            return(DOSFALSE);
        }
    }

    return(FreeLock(lock));
}



#define FILE_ERROR  (ULONG)(-1L)
#define FILE_EOF    0
/***************
*   ReadData() -- Get file data requested and move into buffer.
*
*   return = 0 for end of file
*          = size of transfer
*          = -1 (DOSTRUE) for error in reading.
********/
LONG ReadData(lock, buffer, size)
register struct MLock *lock;
register UBYTE *buffer;
LONG size;
{
F();
    register struct FS *fs = fsys;
    register ULONG blocks_clust = fs->f_blocks_clust;
    register ULONG bytes_clust = blocks_clust<<fs->f_bytes_block_sh;
    register LONG datasize, pos_mod;
    register LONG i, j, actualsize=0;
    LONG maxtransferpos;
    register LONG filepos;
    LONG cluster, prevcluster, nextcluster;
    UBYTE *data, *databgn;
    struct trans_table *t_tbl;
    UBYTE *tbl;

    BYTE error=0;

    DOSerror = 0;

    if(!lock)
    {
        DOSerror = ERROR_INVALID_LOCK;
        return(FILE_ERROR);
    }

    if(!CompareVolNode(lock)) return(FILE_ERROR);

    maxtransferpos = min(lock->ml_KeyPtr->kp_file_size, (lock->ml_lock_ext.le_file_pos) + size);
    filepos = lock->ml_lock_ext.le_file_pos;
    cluster = lock->ml_lock_ext.le_curr_clust;
    prevcluster = lock->ml_lock_ext.le_prev_clust;

/* Already at End-of-File */
    if( filepos >= lock->ml_KeyPtr->kp_file_size) return(FILE_EOF);

    while( (filepos < maxtransferpos)
        && (cluster < FAT_BAD)
        && !(error) )
    {
        pos_mod = filepos%bytes_clust;

    /****** special SPEEDUP particularly for large files */
        if( (0 == pos_mod)
            /* Added to insure dummy file doe not try to get read from disk */
            && (ILLEGAL_CLUST != cluster)     /* not a dummy file */
                /* Check if you can transfer direct to buffer. */
            && !(lock->ml_lock_ext.le_filt_flags & CTRLM_CTRLZ_F)   /* not filtering */
                 /* and mask fits memory */
            && ((ULONG)(&buffer[actualsize]) == (fsys->f_mem_mask & (ULONG)(&buffer[actualsize])))
            && ((datasize = min(size-actualsize, maxtransferpos-filepos)) >= bytes_clust))
        {
        /* Calculate maximum cluster size to transfer directly at one time */
            for(j=1, i=datasize/bytes_clust, nextcluster=prevcluster=cluster;
                (i>1);
                i--, j++)
            {
                if((nextcluster=readFATentry(fsys,nextcluster)) != cluster+j) break;    /* check if contiguous clusters */
                if(0 != (FindBlockInCache(ConvertCluster(nextcluster,0)))) break;     /* check if already in cache */
            }
D();
            error = GetBlockMem(&buffer[actualsize],ConvertCluster(cluster,0), blocks_clust*j);
            
        /* adjust file and buffer pointers to actual bytes read */
            datasize = diskreq->io_Actual;
            cluster += (datasize/bytes_clust) - 1;
            actualsize += datasize;
            filepos += datasize;
        }
         else 
        {
        /* calc maximum number of bytes to be transferred in current cluster */
            datasize = min(bytes_clust - pos_mod, maxtransferpos - filepos);

            if( !(data = ReadCluster(cluster, lock)))
            {
                error = (BYTE)FILE_ERROR;
                break;  /* break out of while() */
            }

        /* calc begin displacement of data in current Block */
            databgn = &data[pos_mod];

        /* Ctrl M or Ctrl Z Filter flag set */
            if( lock->ml_lock_ext.le_filt_flags & CTRLM_CTRLZ_F)
            {
                for(i=0; i < datasize; i++)
                {
                    if(databgn[i] == CTRLM);    /* if CTRL_M do not transfer */
                     else if (databgn[i] == CTRLZ)  /* if CTRL_Z then EOF */
                    {
                        filepos = lock->ml_KeyPtr->kp_file_size-i;
                        break;
                    }
                     else buffer[actualsize++] = databgn[i];    /* no filter byte */
                }
                filepos += i;
            }
        /* NO Filter -- regular transfer */
             else
            {
                CopyMem(databgn, &buffer[actualsize], datasize );
                actualsize += datasize;
                filepos += datasize;
            }
        }
    /* go find next cluster if file position pointing to next cluster */
        if( (filepos) && (0 == (pos_mod=filepos%bytes_clust)) && (datasize) )
        {
            cluster = readFATentry(fs,prevcluster = cluster);
            if( (cluster == FAT_BAD ) || (cluster == FAT_FREE) )
            {       /* Illegal FAT link */
                error = (BYTE)FILE_ERROR;
            }
        }
        lock->ml_lock_ext.le_curr_clust = cluster;
        lock->ml_lock_ext.le_prev_clust = prevcluster;
        lock->ml_lock_ext.le_file_pos = filepos;
    }

    if( (lock->ml_lock_ext.le_filt_flags & XLAT1_F))
    {                           /* XLAT1 filter or International translator set */
        if(t_tbl = lock->ml_lock_ext.le_trans_tbl)
        {
            tbl = t_tbl->tbl_MtoA;      /* Translate MSDOS to Amiga */
            for(i=0; i < actualsize; i++)
            {
               buffer[i] = tbl[buffer[i]];
            }
        }
         else
        {   /* No translate table. Use high-bit strip */
            for(i=0; i < actualsize; i++)
            {
                buffer[i] &= ASCII_7;
            }
        }
    }
    return((LONG)actualsize);
}


/***************
*   WriteData() -- Move file data from buffer to block cache buffer.
*
*   return = 0 for end of file
*          = size of transfer
*          = -1 (DOSTRUE) for error in writing.
********/
LONG WriteData(lock, buffer, size)
register struct MLock *lock;
register UBYTE *buffer;
LONG size;
{
F();
    register struct FS *fs = fsys;
    register int bytes_clust = fs->f_blocks_clust<<fs->f_bytes_block_sh;
    register int datasize, pos_mod;
    register LONG i, actualsize=0;
    LONG cluster, prevcluster;
    register LONG filepos;
    UBYTE *data, *databgn;
    struct Lock_ext *lock_ext;
    UBYTE addctrlj = 0, c;
    struct trans_table *t_tbl;
    UBYTE *tbl;


DPRTF(KPrintF("\n\tW: block = %ld  size = %ld",lock->ml_lock_ext.le_curr_clust>>fs->f_bytes_block_sh, size));
    if(size <= 0) return(0);
    if(!lock)
    {
        DOSerror = ERROR_INVALID_LOCK;
        return(FILE_ERROR);
    }


    if(!CompareVolNode_Write(lock)) return(FILE_ERROR);
    if(DOSerror == ERROR_WRITE_PROTECTED) return(FILE_ERROR);

    lock_ext = &(lock->ml_lock_ext);
    filepos= lock_ext->le_file_pos;
    /* If a dummy file, then protect it from being written to */
    if(ILLEGAL_CLUST == (cluster = lock_ext->le_curr_clust))
    {
        DOSerror = ERROR_WRITE_PROTECTED;
        return(FILE_ERROR);
    }
    prevcluster = lock_ext->le_prev_clust;

    lock->ml_KeyPtr->kp_flags  |= FILE_MOD_F;

    while( ((datasize = size - actualsize) > 0)
        && ((cluster < FAT_BAD) || (cluster >= FAT_EOF_LOW)))
    {
        pos_mod = filepos%bytes_clust;

    /* calc maximum number of bytes to be transfered in current cluster */
        datasize = min(bytes_clust - pos_mod, size-actualsize);

    /* go allocate next cluster if at the end of file */
        if((datasize) && (cluster >= FAT_EOF_LOW))
        {
            if( !(cluster = AllocCluster(prevcluster)))
            {
                DOSerror = ERROR_DISK_FULL;
                return(FILE_ERROR);
            }
              else if(FAT_BAD <= cluster)
            {
                break;  /* break out of while() */
            }
        }

/*###########################################################################
    INSERT Direct transfer speedup routine here.
###########################################################################*/
        {
            if(datasize != bytes_clust)  /* only partial cluster to be written */
            {               /* Get data from cluster(s) */
                if( !(data = ReadCluster(cluster, lock)))
                {
                    break;  /* break out of while() */
                }
            }
            if( !(data = WriteCluster(cluster, lock)))
            {
                break;  /* break out of while() */
            }

        /* calc begin displacement of data in current Block */
            databgn = &data[pos_mod];

        /* Ctrl M or Ctrl Z filter flag set */
            if( lock_ext->le_filt_flags & CTRLM_CTRLZ_F)
            {
                for(i=pos_mod;
                    (actualsize < size) && (i < bytes_clust);
                    i++)
                {
                /* if CTRL_J add CTRL_M before */
                    c = buffer[actualsize++];
                    if(!addctrlj)
                    {
                        switch( c )
                        {
                            case CTRLJ:
                                c = CTRLM;  /* insert CTRL M before CTRL J */
                                actualsize--;
                            case CTRLM:
                                addctrlj = 1;
                            default:
                                break;
                        }
                    }
                      else addctrlj = 0;
                    data[i] = c;    /* no filter byte */
                }
                filepos += (datasize = (i-pos_mod));
            }
        /* NO Filter -- regular transfer */
              else
            {
                CopyMem(&buffer[actualsize], databgn, datasize );
                actualsize += datasize;
                filepos += datasize;
            }

            if( (lock_ext->le_filt_flags & XLAT1_F))
            {                           /* International translator set */
                if(t_tbl = lock->ml_lock_ext.le_trans_tbl)
                {
                    tbl = t_tbl->tbl_AtoM;      /* Translate Amiga to MSDOS */
                    for(i=0; i < datasize; i++)
                    {
                        databgn[i] = tbl[databgn[i]];
                    }
                }
            }
        }
    /* go find next cluster if file position pointing to next cluster */
        if( (filepos) && (0 == (pos_mod=filepos%bytes_clust)) && (datasize) )
        {
            cluster = readFATentry(fsys,prevcluster = cluster);
            lock_ext->le_prev_clust = prevcluster;
        }
    }

/* If actual size of write is less than the requested transfer size, do not
update current cluster and file position so that the user can retry this
operation at the same file location.  */
    if(actualsize == size)
    {
        lock_ext->le_curr_clust = cluster;
        lock_ext->le_prev_clust = prevcluster;
        lock_ext->le_file_pos = filepos;
    }

/* If the current file position is greater than the file size, change the file
size for this lock and locks on the same object */
    if(filepos > lock->ml_KeyPtr->kp_file_size)
    {
        lock->ml_KeyPtr->kp_file_size = filepos;
    }
    return((LONG)actualsize);
}


#define SEEK_ERROR  -1L
/********
*   SeekFilePos() -- Seek to the specified file position and return old position.
*
*   mode = -1 = OFFSET_BEGINNING -- relative to the beginning of file
*        =  0 = OFFSET_CURRENT   -- relative to the current position of file
*        =  1 = OFFSET_END       -- relative to the end of file
*
*   return = old position
*          = -1 (SEEK_ERROR) for error in seeking.
********/
LONG SeekFilePos(lock, offset, mode)
register struct MLock *lock;
LONG offset;
LONG mode;
{
F();
    LONG filepos, oldpos;
    int i;
    LONG cluster, prevcluster, endpos;


    if(!lock)
    {
        DOSerror = ERROR_INVALID_LOCK;
        return(FILE_ERROR);
    }

    endpos= lock->ml_KeyPtr->kp_file_size;


    oldpos = lock->ml_lock_ext.le_file_pos;
    cluster = lock->ml_KeyPtr->kp_start_clust;

    if(!CompareVolNode(lock)) return(FILE_ERROR);

    switch(mode)
    {
        case OFFSET_BEGINNING:
            filepos = offset;
            break;
        case OFFSET_CURRENT:
            filepos = offset + lock->ml_lock_ext.le_file_pos;
            break;
        case OFFSET_END:
            filepos = offset + endpos;
            break;
        default:
            DOSerror = ERROR_SEEK_ERROR;
            return(SEEK_ERROR);
            break;
    }
    if((filepos>endpos) || (filepos<0))
    {
            DOSerror = ERROR_SEEK_ERROR;
            return(SEEK_ERROR);
    }

/* Find new current cluster */
/* Since this is a single-linked list, I must start with the starting cluster
 to find the current cluster of the seek position */
    for(i = (filepos)/(fsys->f_blocks_clust<<fsys->f_bytes_block_sh); i > 0; i--)
    {       /* fast-forward to current cluster */
        cluster = readFATentry(fsys,prevcluster = cluster);
        if( (cluster == FAT_BAD) || (cluster == FAT_FREE) )
        {       /* Illegal FAT link */
            DOSerror = ERROR_SEEK_ERROR;
            return(SEEK_ERROR);
        }
        else if( cluster >= FAT_EOF_LOW )
        {
            lock->ml_lock_ext.le_prev_clust = prevcluster;
        }
    }
    lock->ml_lock_ext.le_file_pos = filepos;
    lock->ml_lock_ext.le_curr_clust = cluster;

    return(oldpos);
}


/*##############################################################################
###############   Begin WriteData() Direct transfer Speedup Routine  ###########
####    This routine is not used because it actually slows down writing
####    data to the disk.  Go figure!  Current tests results:
####
####    (Copying >700K worth of files to the disk with 65K minimum buffer size)
####    to floppy -> 70 seconds slower.
####    to hard disk -> 2 seconds slower.
####
####    This routine is here for those who would like to try it out.  If you can
####    make it faster than the buffering technique used above.  Be my guest.

    LONG nextcluster;


    /****** special SPEEDUP particularly for large files */
        if( (0 == pos_mod)
                /* Check if you can transfer direct to buffer. */
            && !(lock->ml_lock_ext.le_filt_flags & CTRLM_CTRLZ_F)   /* not filtering */
                 /* and mask fits memory */
            && ((ULONG)(&buffer[actualsize]) == (fsys->f_mem_mask & (ULONG)(&buffer[actualsize])))
            && ((datasize = size-actualsize) >= bytes_clust))
        {
        /* Calculate maximum cluster size to transfer directly at one time */
DPRTF(KPrintF("\n\tdatasize=%ld  ",datasize));
        /* First, try to find all consecutive clusters */
            for(j=1,i=datasize/bytes_clust, nextcluster=cluster;
                (i>1);
                i--,j++)
            {
DPRTF(KPrintF("\n\t1: i=%ld  j=%ld  nextcluster=%ld  cluster=%ld",i,j,nextcluster,cluster));
                if((nextcluster=readFATentry(fsys,nextcluster)) != cluster+j) break;    /* check if contiguous clusters */
                if(0 != (FindBlockInCache(ConvertCluster(nextcluster,0)))) break;     /* check if already in cache */
            }
        /* Next, try to allocate consecutive clusters if nextcluster>=FAT_EOF_LOW from previous routine */
            if(nextcluster >= FAT_EOF_LOW)
            {
                for(nextcluster=prevcluster;
                    (i>1) && ((nextcluster = AllocCluster(nextcluster)) == cluster+j);
                    i--,j++)
                {
DPRTF(KPrintF("\n\t2: i=%ld  j=%ld  nextcluster=%ld  cluster=%ld",i,j,nextcluster,cluster));
                }
            }

/** Check to see if one too many clusters allocated if the last cluster could
not be consecutive. Deallocate if necessary */
                if( nextcluster == 0 )
                {
                    DOSerror = ERROR_DISK_FULL;
                    return(FILE_ERROR);
                }
                  else if(FAT_BAD <= nextcluster)
                {
                    break;  /* break out of while() */
                }
            if(PutBlockMem(&buffer[actualsize],ConvertCluster(cluster,0), blocks_clust*j))
            {
                break;  /* break out of while() loop */
            }

            datasize = diskreq->io_Actual;
            cluster += (datasize/bytes_clust) - 1;
DPRTF(KPrintF( "\nWriteData: j=%ld  datasize=%ld  size-actualsize=%ld",j,datasize,size-actualsize));
            actualsize += datasize;
            filepos += datasize;

        }
        else
###############   End WriteData() Speedup Routine  ############################
##############################################################################*/
