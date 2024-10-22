/* fileIO.c **************************************************************
** Copyright 1991 CONSULTRON
*
*      DOS file IO related functions.
*
*************************************************************************/

#include "FS:FS.h"

extern struct Library *JCCBase;		/*  wc  */

//extern VOID __stdargs kprintf();	/* wc.  debug */


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

    if( newmfib && (mfib->mfib_Protection & FIBF_WRITE))
    {
        fileflags |= FILE_WRITE_PROT_F;
    }
    if( !newmfib )
    {
        return(DOSFALSE);
    }

    if(lock == 0) if(!(lock = MakeLock(dirlock, newmfib, lockmode)))
    {
        return(DOSFALSE);
    }

    lock->ml_lock_ext.le_flags          = fileflags;

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
    lock->ml_lock_ext.le_curr_clust     = lock->ml_lock_ext.le_start_clust;
    lock->ml_lock_ext.le_databuffnum    = C_DBUFF_UNUSED;

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
    LONG curr_cluster,cluster,prevcluster;
    register ULONG new_eof,old_eof;
    register LONG i;
    register struct Lock_ext *nextlock_ext,*lock_ext;
    ULONG filepos, maxpos=0;
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

    old_eof = lock_ext->le_file_size;
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
/* Find maximum file position of all lock extensions of the same object */
/* But do not count the current position of "lock" */
        nextlock_ext = (struct Lock_ext *)(lock_ext->le_samelock.mln_Succ);
        do
        {
            maxpos = max(maxpos,nextlock_ext->le_file_pos);
            nextlock_ext = (struct Lock_ext *)(nextlock_ext->le_samelock.mln_Succ);
        } while(nextlock_ext != lock_ext);

    /* The new EOF cannot be smaller than the largest current file position */
    /* except the current position of "lock" */
        new_eof = max(maxpos,new_eof);

    /* Set the new EOF and free unused clusters */
        cluster = lock_ext->le_start_clust;
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
        nextlock_ext = lock_ext;
        do
        {
            nextlock_ext->le_file_size = new_eof;
            nextlock_ext = (struct Lock_ext *)(nextlock_ext->le_samelock.mln_Succ);
        } while(nextlock_ext != lock_ext);

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
        lock_ext->le_flags |= FILE_MOD_F;
    }

    lock_ext->le_file_pos = filepos;
    lock_ext->le_curr_clust = curr_cluster;

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
    register struct MFileInfoBlock *mfib = (struct MFileInfoBlock *)fsys->f_scratch;
    register struct FS_dir_ent *dirent;
    UCHAR ctrlz=CTRLZ;
    ULONG volatile filesize;

    if(!lock)
    {
        DOSerror = ERROR_INVALID_LOCK;
        return(DOSFALSE);
    }

    if(lock && (lock->ml_lock_ext.le_flags & FILE_MOD_F))
    {
        if(!CompareVolNode_Write(lock)) return(DOSFALSE);

                /* If Ctrl M or Ctrl Z filter flag set put */
        if( lock->ml_lock_ext.le_filt_flags & CTRLM_CTRLZ_F)
        {
            SeekFilePos(lock, 0, OFFSET_END);     /* go to end of file */
            WriteData(lock, &(ctrlz), 1);     /* write CTRLZ at end of file */
        }
          mfib->mfib_DiskCluster   = (lock->ml_lock_ext.le_dir_clust);
          mfib->mfib_DiskDirEntry  = (lock->ml_lock_ext.le_dir_ent)-1; /* point to previous entry */
          mfib->mfib_DirEntStatus  = MLDE_DIR;

/*### Update date of parent directory of file ###*/
        SetDate_ParentDir(mfib);

        if(!(dirent = GetNextDirEnt(mfib)))
        {
            return(DOSFALSE);
        }

        filesize = lock->ml_lock_ext.le_file_size;
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
    register struct FS *fs = fsys;
    register ULONG blocks_clust = fs->f_blocks_clust;
    register ULONG bytes_clust = blocks_clust<<fs->f_bytes_block_sh;
    register LONG datasize, pos_mod;
    register LONG i, j, k, actualsize=0;
    LONG maxtransferpos;
    register LONG filepos;
    LONG cluster, prevcluster, nextcluster, tempcluster;
    UBYTE *data, *databgn, *tempdata;
    struct trans_table *t_tbl;
    UBYTE *tbl;

    struct JConversionCodeSet *jcc;	/*  wc  */
    struct JConversionCodeSet *jccID;	/*  wc  */

    UBYTE *tmpbuf;   /* wc */

    int WrtCount;   /* wc */
    UBYTE tmp;      /****test***/
    LONG whatcode = CTYPE_UNKNOWN;

    BOOL ReadHalf;
    UBYTE OneCharIn[2], OneCharOut[2];	/* wc */
    UBYTE tempbyte;

    BYTE error=0;

    DOSerror = 0;


//kprintf ("call ReadData, size = %lx\n", size);

    if(!lock)
    {
        DOSerror = ERROR_INVALID_LOCK;
        return(FILE_ERROR);
    }

    if(!CompareVolNode(lock)) return(FILE_ERROR);

    maxtransferpos = min(lock->ml_lock_ext.le_file_size, (lock->ml_lock_ext.le_file_pos) + size);
    filepos = lock->ml_lock_ext.le_file_pos;
//kprintf ("filepos on entry = %lx\n", filepos);
    cluster = lock->ml_lock_ext.le_curr_clust;
    prevcluster = lock->ml_lock_ext.le_prev_clust;

    jcc = lock->ml_lock_ext.le_jcc_handle1;  		/* wc */
    jccID = lock->ml_lock_ext.le_jcc_handle2;           /* wc */
    ReadHalf = lock->ml_lock_ext.le_ReadHalf;           /* wc */
    tempbyte = lock->ml_lock_ext.le_TempByte;           /* wc */


/* Already at End-of-File */
    if( filepos >= lock->ml_lock_ext.le_file_size) return(FILE_EOF);

    while( (filepos < maxtransferpos)
        && (cluster < FAT_BAD)
        && !(error) )
    {
        pos_mod = filepos%bytes_clust;
//kprintf ("pos_mod = %lx mod %lx = %lx\n", filepos, bytes_clust, pos_mod);

    /****** special SPEEDUP particularly for large files */
        if( (0 == pos_mod)
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
//kprintf("datasize = min (%lx,  %lx) = %lx\n", bytes_clust - pos_mod, maxtransferpos - filepos, datasize);
            if( !(data = ReadCluster(cluster, lock)))
            {
                error = (BYTE)FILE_ERROR;
                break;  /* break out of while() */
            }

        /* calc begin displacement of data in current Block */

            databgn = &data[pos_mod];

	    /* clear internal buffer for identifying data code set.   wc  */

//	    if (ReadHalf)
//		{
//		databgn++;
//		datasize--;
//		}

	    if (jccID && jcc)
		{
		SetJConversionAttrs(jccID, JCC_FirstByte, 0x00, JCC_ESC, 0, JCC_HanKata, 0, TAG_DONE);
		if (ReadHalf)
		    whatcode = IdentifyCodeSet(jccID, databgn+1, datasize-1);
		else
		    whatcode = IdentifyCodeSet(jccID, databgn, datasize);

		}

//kprintf ("code = %lx\n", whatcode);

	    /* Ctrl M or Ctrl Z Filter flag set */

	    /* If Filter flag is set, go through text filter/conversion  */
	    /* only for text files.  wc  */

            if( (lock->ml_lock_ext.le_filt_flags & CTRLM_CTRLZ_F) &&
		((whatcode == CTYPE_SJIS) || (whatcode == CTYPE_ASCII)) )

		{

		/* allocate memory for conversion.  wc */
		tmpbuf = AllocVec (datasize+16L, MEMF_CLEAR);
		if (tmpbuf)
		    {

//kprintf ("filepos = %lx\n", filepos);

		    /* clear internal conversion buffer if this is the beginning of a file */
		    if (filepos == 0)
			SetJConversionAttrs(jcc, JCC_FirstByte, 0x00, JCC_ESC, 0, JCC_HanKata, 0, TAG_DONE);

		    /* first filter out Ctrl-M */
		    k = 0;
		    for (i=0; i<datasize; i++)
			{
	                if(databgn[i] == CTRLM);	/* if CTRL_M do not transfer */
                        else if (databgn[i] == CTRLZ)	/* if CTRL_Z then EOF */
	                    {
                            filepos = lock->ml_lock_ext.le_file_size-i;
			    break;
			    }
                        else tmpbuf[k++] = databgn[i];	/* put into temp buffer for conversion */

			}
		    filepos += i;			/* update input file position */


		    /* the conversion routine takes a null terminated string */
		    /* as input, so we have to attach null at the end of input stream  wc */

//		    tmp = tmpbuf[k];			/* save data at buffer end.  wc */
//		    tmpbuf[k] = 0;

		    /* Convert from SJIS to EUC and store converted data.  wc */

		    j = k;				/* save buffer length */

		    if (ReadHalf)
			{
			buffer[actualsize++] = tempbyte;
			k--;
			ReadHalf = FALSE;
//kprintf ("ReadHalf clear,  next char is: %lx, %lx\n", tmpbuf[1], tmpbuf[2] );
			WrtCount = JStringConvert (jcc, &tmpbuf[1], &buffer[actualsize], CTYPE_SJIS, CTYPE_EUC, k, JCT_UDHanKata, 0x25, TAG_DONE );
			}
		    else
			WrtCount = JStringConvert (jcc, tmpbuf, &buffer[actualsize], CTYPE_SJIS, CTYPE_EUC, k, JCT_UDHanKata, 0x25, TAG_DONE );
		    actualsize += WrtCount;		/* update actrualsize.  wc */
//kprintf ("datasize = %lx    k = %lx     ", datasize, k);
//kprintf ("WrtCount = %lx\n",WrtCount);

//		    tmpbuf[k] = tmp;			/* restore data at buffer end. wc */


		    if (WrtCount < k)
			{
			OneCharIn[0] = tmpbuf[j-1];
			if ((filepos%bytes_clust) == 0 )
			    {
			    tempcluster = readFATentry(fs, cluster);
//kprintf("cluster boundary, filepos = %lx cluster = %lx tempcluster = %lx\n", filepos, cluster, tempcluster);
                            tempdata = ReadCluster(tempcluster, lock);
			    OneCharIn[1] = tempdata[0];
			    }
			else
			    {
//kprintf ("mid cluster\n");
			    OneCharIn[1] = databgn[i];
			    }
			WrtCount = JStringConvert (jcc, OneCharIn, OneCharOut, CTYPE_SJIS, CTYPE_EUC, 2, JCT_UDHanKata, 0x25, TAG_DONE);
			buffer[actualsize++] = OneCharOut[0];
			tempbyte = OneCharOut[1];
			ReadHalf = TRUE;
//kprintf ("boundary char = %lx, %lx,  ReadHalf set\n", OneCharIn[0], OneCharIn[1]);

			}


		    /* Free memory allocated.  wc */
		    FreeVec (tmpbuf);

		    }

		/* don't do filter if there is trouble allocating memory.  wc */
		else
		    {
		    CopyMem(databgn, &buffer[actualsize], datasize );
		    actualsize += datasize;
		    filepos += datasize;
		    }


/***************************/
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
//kprintf ("filepos = %lx on exit\n", filepos);
	lock->ml_lock_ext.le_ReadHalf = ReadHalf;		/* wc */
	lock->ml_lock_ext.le_TempByte = tempbyte;               /* wc */
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
//kprintf ("actualsize = %lx\n", actualsize);
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
    register struct FS *fs = fsys;
    register int bytes_clust = fs->f_blocks_clust<<fs->f_bytes_block_sh;
    register int datasize, pos_mod;
    register LONG i, k, actualsize=0;
    LONG cluster, prevcluster;
    register LONG filepos;
    UBYTE *data, *databgn;
    struct Lock_ext *nextlock_ext,*lock_ext;
    UBYTE addctrlj = 0, c;
    struct trans_table *t_tbl;
    UBYTE *tbl;

    struct JConversionCodeSet *jcc;	/*  wc  */
    struct JConversionCodeSet *jccID;	/*  wc  */

    UBYTE *tmpbuf = NULL;		/* wc */
    int WrtCount, NumBytesProc;		/* wc */
    UBYTE tmp, tmp1;			/* wc */
    LONG whatcode = CTYPE_UNKNOWN;	/* wc */

    UBYTE OneJISin[2], OneJISout[2];	/* wc */
    BOOL WrtHalf, WrtHalf2 = FALSE;	/* wc */

    DOSerror = 0;



DPRTF(KPrintF("\n\tW: block = %ld  size = %ld",lock->ml_lock_ext.le_curr_clust>>fs->f_bytes_block_sh, size));
//kprintf ("size = %lx\n", size);
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
    cluster = lock_ext->le_curr_clust;
    prevcluster = lock_ext->le_prev_clust;

    lock_ext->le_flags |= FILE_MOD_F;

    jcc = lock_ext->le_jcc_handle1;	/* wc */
    jccID = lock_ext->le_jcc_handle2;	/* wc */
    WrtHalf = lock_ext->le_WrtHalf;	/* wc */
//    HalfJIS2 = lock_ext->le_HalfJIS2;   /* wc */


    /* allocating memory for conversion buffer if filter flag is set.  wc */

    if( lock_ext->le_filt_flags & CTRLM_CTRLZ_F)
	tmpbuf = AllocVec (bytes_clust+16L, MEMF_CLEAR);


    while( ((datasize = size - actualsize) > 0)
        && ((cluster < FAT_BAD) || (cluster >= FAT_EOF_LOW)))
    {
        pos_mod = filepos%bytes_clust;

//kprintf ("cluster = %lx\n", cluster);

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

            /* clear internal buffer for identify code set.  wc */

	    if (jccID && jcc)
		{
		SetJConversionAttrs(jccID, JCC_FirstByte, 0x00, JCC_ESC, 0, JCC_HanKata, 0, TAG_DONE);

		/* if WrtHalf or WrtHalf2 if set, we are in the middle of conversion */
		/* the code set must be type EUC.  */
		/* otherwise try identify code set.  wc */

		if (WrtHalf || WrtHalf2)
		    whatcode = CTYPE_EUC;
		else
		    {
		    whatcode = IdentifyCodeSet(jccID, &buffer[actualsize], datasize);

//kprintf("codeset: %lx at file pos %lx, size = %lx\n" , whatcode, filepos, datasize);
		    }
		}

	    /* go through filter/conversion only for text files.  wc */

            if( (lock_ext->le_filt_flags & CTRLM_CTRLZ_F) &&
		( (whatcode == CTYPE_EUC) || (whatcode == CTYPE_ASCII)) )
            {


	    /* if this is the beginning of a file, clear convertion buffer */

		if (filepos == 0)
		    SetJConversionAttrs(jcc, JCC_FirstByte, 0x00, JCC_ESC, 0, JCC_HanKata, 0, TAG_DONE);

		if (tmpbuf)
                    {

		    /* calculate number of bytes to process.		 */
		    /* if WrtHalf set, 1 more byte left from last write */
		    /* if WrtHalf2 set, 1 less byte to write		 */
		    /* otherwise write whole cluster unless there is less data.  wc */

		    if (WrtHalf)
			NumBytesProc = pos_mod + 1;
		    else if (WrtHalf2)
			NumBytesProc = bytes_clust - 1;
		    else
			NumBytesProc = bytes_clust;

		    for (i=pos_mod;
                    (actualsize < size) && (i < NumBytesProc); /* wc */
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
                 	else
			    addctrlj = 0;

//wc                   data[i] = c;    /* no filter byte */
	    	        tmpbuf[i-pos_mod] = c;	/* wc */
			}

//wc               filepos += (datasize = (i-pos_mod));

/* wc ***************************/


		    /* make sure the data is null terminated.  wc */

		    tmp = tmpbuf[i-pos_mod];
		    tmpbuf[i-pos_mod] = 0;
		    k = (i-pos_mod);		/* save the size of data.  wc */

		    /* if WrtHalf set, 1 byte left from previous write. */
		    /* get it from conversion lib internal buffer, combine */
		    /* it with the first byte of current data block, and  */
		    /* convert these two bytes.  the result is stored in  */
		    /* temp location OneJISout[0..1].  OneJISout[0] is then */
		    /* copied to the write buffer which is to be written to */
		    /* the same cluster of the previous write.   wc */

		    if (WrtHalf)
			{
			WrtCount = GetJConversionAttrs (jcc, JCC_FirstByte, &OneJISin, TAG_DONE);
			OneJISin[1] = tmpbuf[0];
			WrtCount = JStringConvert (jcc, OneJISin, OneJISout, CTYPE_EUC, CTYPE_SJIS, 2, TAG_DONE);
			SetJConversionAttrs(jcc, JCC_FirstByte, 0x00, JCC_ESC, 0, JCC_HanKata, 0, TAG_DONE);

			data[pos_mod] = OneJISout[0];
			filepos++;		/* updata file position. wc */
//			datasize = 1;
			WrtHalf = FALSE;	/* clear HalfJIS1 flag. wc */
			WrtHalf2 = TRUE;        /* set HalfJIS2 flag.   wc */
			}

		    /* if WrtHalf2 set, the first byte in this cluster  */
		    /* has already been converted; copy it from OneJISout[1]. */
		    /* Then convert rest of the data.  wc */

		    else if (WrtHalf2)
			{
		        data[pos_mod] = OneJISout[1];	/* copy the first one.  wc */
			databgn++;			/* update write buffer ptr. wc */
//			datasize = 1;
			filepos++;			/* update file pos.  wc */
			WrtHalf2 = FALSE;               /* clear WrtHalf2 flag.  wc */
			WrtCount = JStringConvert (jcc, tmpbuf, databgn, CTYPE_EUC, CTYPE_SJIS, -1, TAG_DONE );

			filepos += (datasize = WrtCount);
			}

		    /* No boundary problem occured in last write, just */
		    /* strait conversion.  wc */

		    else
			{
 		        WrtCount = JStringConvert (jcc, tmpbuf, databgn, CTYPE_EUC, CTYPE_SJIS, -1, TAG_DONE );
			filepos += (datasize = WrtCount);
			}

		    tmpbuf[k] = tmp;
//kprintf ("WrtCount = %lx\n", WrtCount);
		    /* if data size got smaller after conversion, the last */
		    /* byte in the cluster must be a first byte EUC code.  */
		    /* set WrtHalf flag to indicate this.  wc */

		    if (WrtCount < (i-pos_mod))
			{
			WrtHalf = TRUE;
			}

		    }

		/* no conversion is taken place if there is problem alloc buffer.  wc */

		else
		    {
		    CopyMem(&buffer[actualsize], databgn, datasize );
		    actualsize += datasize;
		    filepos += datasize;
		    }
/*******************************/

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

    if (tmpbuf)          		/* wc */
	FreeVec (tmpbuf);


/* If actual size of write is less than the requested transfer size, do not
update current cluster and file position so that the user can retry this
operation at the same file location.  */
    if(actualsize == size)
    {
        lock_ext->le_curr_clust = cluster;
        lock_ext->le_prev_clust = prevcluster;
        lock_ext->le_file_pos = filepos;
    }

    lock_ext->le_WrtHalf = WrtHalf;	/* wc */
//    lock_ext->le_HalfJIS2 = HalfJIS2;	/* wc */

/* If the current file position is greater than the file size, change the file
size for this lock and locks on the same object */

    if(filepos > lock_ext->le_file_size)
    {
        nextlock_ext = lock_ext;
        do
        {
            nextlock_ext->le_file_size = filepos;
            nextlock_ext = (struct Lock_ext *)(nextlock_ext->le_samelock.mln_Succ);
        } while(nextlock_ext != lock_ext);
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
    LONG filepos, oldpos;
    int i;
    LONG cluster, prevcluster, endpos;

    DOSerror = 0;

    if(!lock)
    {
        DOSerror = ERROR_INVALID_LOCK;
        return(FILE_ERROR);
    }

    endpos= lock->ml_lock_ext.le_file_size;


    oldpos = lock->ml_lock_ext.le_file_pos;
    cluster = lock->ml_lock_ext.le_start_clust;

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
