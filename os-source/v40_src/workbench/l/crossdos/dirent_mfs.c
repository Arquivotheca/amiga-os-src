/* DirEnt_MFS.c **************************************************************
** Copyright 1991 CONSULTRON
*
*      MSDOS Directory Entry functions.
*
*************************************************************************/
#include "FS:FS.h"

extern struct ExecBase  *SysBase;
extern struct DOSBase *DOSBase;
extern struct FS      *fsys;
extern int DOSerror;
extern struct trans_table *trans_tbl;


/***************
*   GetNextDirEnt() -- Get next directory entry.
*
*   This routine is expected to set
*   DiskCluster, DiskKey, DirEntStatus, DiskDirEntry, Starting cluster
*   and Protection of the mfib structure.
*
*     Return memory ptr to entry or 0 if error.
********/
struct FS_dir_ent *GetNextDirEnt(mfib)
struct MFileInfoBlock *mfib;
{
    UWORD volatile cluster    = mfib->mfib_DiskCluster;
    WORD dirent     = (WORD)mfib->mfib_DiskDirEntry;
    WORD clustent;
    BYTE entrytype  = mfib->mfib_DirEntStatus;
    register struct FS *fs = fsys;
    struct FS_dir_ent *mem;
    ULONG block;

        /** Return if already pointing to end dir entry */
    if((entrytype == MLDE_END))
    {
        return(DOSFALSE);
    }

        /** Get first cluster and point to first dir entry if ROOT of dir */
    if( ( entrytype == (MLDE_DIR_ROOT) ) && (dirent > -1) )   /* NOT root of root dir */
    {       /** root of a dir. Get first cluster and dir entry **/
        mfib->mfib_DirEntStatus = MLDE_DIR;  /* clear ROOT flag */
        mfib->mfib_DiskDirEntry -= 1;  /* point ot prev dir ent */
        if( !(mem = (struct FS_dir_ent *)GetNextDirEnt(mfib)))
        {
            return(DOSFALSE);
        }
          SWAPWORD(cluster,mem->fde_start_clust);
        if( FAT_EOF_LOW <= ( mfib->mfib_DiskCluster = cluster)) 
        {                       /* pre-mature end of dir entry */
            mfib->mfib_DirEntStatus = MLDE_END;
            return(DOSFALSE);
        }
        dirent = -1;
        if( mfib->mfib_DirEntStatus < 0 )  /* ILLEGAL. Should not be performed */
        {
            return((struct FS_dir_ent *)GETCLUSTER(mfib->mfib_DiskKey = cluster,0));
        }

    }

    dirent++;   /* go to next entry */

        /** check for bounds of directory entry **/
    if( ROOTDIR_CLUST >= cluster )
    {       /** Cluster of root dir **/
        if( dirent >= fs->f_root_dir_ent)
        {       /* at the end of the root directory entries */
            mfib->mfib_DirEntStatus = MLDE_END;
            return(DOSFALSE);
        }
    }

    clustent = dirent%(WORD)fs->f_dirents_clust;
    if( -1L == (block = ConvertCluster(cluster, dirent)))
    {
        mfib->mfib_DirEntStatus = MLDE_END;
        return(DOSFALSE);
    }

    mfib->mfib_DiskCluster  = cluster;
    mfib->mfib_DiskKey      = block + (clustent/fs->f_dirents_block) - fs->f_beg_part;
    mfib->mfib_DiskDirEntry = dirent;

    if( ROOTDIR_CLUST >= cluster )
    {       /** Cluster of root dir **/
        mem = (struct FS_dir_ent *)(fs->f_rootdir_cache);
        clustent = dirent;
    }
/* Get cluster */
     else if( !(mem = (struct FS_dir_ent *)GetBlock(block)))
    {       /* Get block from disk or cache */
        return(DOSFALSE);
    }

    mfib->mfib_Size = 0;

    if(mem[clustent].fde_filename[0] == FN_ERASE)   /* Empty dir entry */
    {
        mfib->mfib_DirEntStatus = MLDE_EMPTY;
    }
      else if(mem[clustent].fde_filename[0] == FN_END )   /* End dir entry */
    {
        mfib->mfib_DirEntStatus = MLDE_END;
    }
      else if(mem[clustent].fde_protection & ATTF_VOL_LBL )     /* volume label entry */
    {
        mfib->mfib_DirEntStatus = MLDE_VOL_LBL;
    }
      else if(mem[clustent].fde_protection & ATTF_SUBDIR )  /* subdirectory entry */
    {
        mfib->mfib_DirEntStatus = MLDE_DIR;
    }
      else
    {       /* file entry */
        mfib->mfib_DirEntStatus = MLDE_FILE;
            /* set size of file (in bytes). */
        SWAPLONG(mfib->mfib_Size,mem[clustent].fde_file_size);
    }
        /* set starting cluster */
      SWAPWORD(cluster,mem[clustent].fde_start_clust);
    mfib->mfib_start_clust = (UWORD)cluster;

        /* set file attributes (protection) */
    mfib->mfib_Protection     = FIBF_ARCHIVE;
    if(mem[clustent].fde_protection & ATTF_READONLY) mfib->mfib_Protection |= (FIBF_WRITE|FIBF_DELETE);
    if(mem[clustent].fde_protection & TO_BE_ARCHIVED_FLAG ) mfib->mfib_Protection &= ~FIBF_ARCHIVE;

/*
DPRTF(KPrintF( "\nGetNextDirEnt: entrytype = %ld  dirent = %ld",mfib->mfib_EntryType, dirent));
*/
    return(&(mem[clustent]));            /* pass back pointer to next dir ent */
}


/********************
* FindObject()  --- Find specified object (file or directory).
*   Return mfib to object or 0 if not found.
*
*  Three external variables will be filled with information when this routine
*  returns.
*   "Mname" is the MSDOS formatted name of the last object checked in the filespec
*   "DOSerror" is anticipated DOS error of the last object checked in the filespec
*  The "mfib" will be filled with information of the last object checked in the
*   filespec when this routine returns.
*************/
struct MFileInfoBlock *FindObject(lock, name, mfib)
struct MLock *lock;
UCHAR *name;    /* BADDR(BSTR) */
struct MFileInfoBlock *mfib;
{
    register struct FS_dir_ent *dirent;
    BYTE ext=0, i=0, drivespec=0;
    WORD j;
    register UCHAR *foundbrk=&name[1], *firstbrk=foundbrk;
    register UCHAR *Mname = mfib->mfib_Oname;
    UCHAR c;


    DOSerror = ERROR_OBJECT_NOT_FOUND;    /* could not find specified object */
    mfib->mfib_FileFlags = 0;

    if( !ExamineObject(lock, mfib)) return(DOSFALSE);   /* could not find Object */ 
    memset(Mname, ' ', FNAME_EXT_SIZE);         /* set Mname to all ' 's */

    for(j=name[0]; (j>=0) && mfib; j--)
    {
        switch( j?(c=*foundbrk):'\0')
        {
        case '.' :
            if(i==0 )  /* string empty */
            {
                memset(Mname, ' ', FNAME_SIZE);     /* set Mname to all ' 's */
            }

            i = (ext = FNAME_SIZE);
            memset(&Mname[FNAME_SIZE], ' ', FEXT_SIZE); /* set Mname ext to all ' 's */
            break;

        case '/' :
            DOSerror = ERROR_DIR_NOT_FOUND; /* could not find directory */

            drivespec = 1;
            if( (i==0) && (foundbrk==firstbrk) )   /* string empty */
            {
                firstbrk = foundbrk+1;
                if( !FindParent(mfib) )
                {
                    DOSerror = ERROR_OBJECT_WRONG_TYPE;
                    mfib = DOSFALSE;
                }
                    /* could not find parent directory */
                break;
            }

        nameend:
        case '\0':
            drivespec = 1;
            if(i==0)
            {
                if(foundbrk==firstbrk)  /* string empty */
                {
                    break;
                }
                  else    /* invalid name */
                {
                    DOSerror = ERROR_INVALID_COMPONENT_NAME;    /* bad filespec */
                    mfib = DOSFALSE;
                }
            }

              else
            {
                mfib->mfib_DirEntStatus  = MLDE_DIR_ROOT;
                while( (dirent = GetNextDirEnt(mfib))
/* in Lattice C V5.04 the strnicmp() does not take in account the extended
chars for case ignoring */
/* Using the utility.library equivalent to strnicmp() becasue with localization
it should handle case for international characters properly */
                    && ( (strnicmp(Mname, dirent->fde_filename, FNAME_EXT_SIZE))
                    || ((dirent->fde_filename[0] == FN_ERASE)
                    || ( dirent->fde_filename[0] == FN_DIR)
                    || ( dirent->fde_protection & ATTF_VOL_LBL) ) ) );
                if((!dirent)                /* No match found */
                    || ((DOSerror == ERROR_DIR_NOT_FOUND) 
                    && !(mfib->mfib_DirEntStatus == MLDE_DIR))) mfib=DOSFALSE;
            }

            if(mfib)
        newname:
            {
                i = (ext = 0);
                firstbrk = foundbrk+1;
            }
            break;

        case ':' :
            if(drivespec)
            {
                DOSerror = ERROR_INVALID_COMPONENT_NAME;    /* bad filespec */
                mfib = DOSFALSE;
            }
              else
            {
                drivespec = 1;
                goto newname;
            }
            break;

        case ']' :      /* Ctrl M and Ctrl Z filter flag */
            mfib->mfib_FileFlags |= CTRLM_CTRLZ_F;
            break;

        case '[' :      /* translator (type 1) flag */
            mfib->mfib_FileFlags |= XLAT1_F;
            break;

        default:
            if(i==0 )  /* string empty */
            {
                memset(Mname, ' ', FNAME_EXT_SIZE);     /* set Mname to all ' 's */
            }

            if( (!ext && (i < FNAME_SIZE)) || ( ext && (i < FNAME_EXT_SIZE)))
            {
                if( c < (UCHAR)' ' )           /* check if a control char */
                {
                    DOSerror = ERROR_INVALID_COMPONENT_NAME;    /* bad filespec */
                    mfib = DOSFALSE;
                }
                  else
                {
/* This one is kind of strange.  In order to use the 0xE5 char for international chars
   the 0x05 char is used because the 0xE5 char is used to describe FileName Erased (FN_ERASE) 
   in only the first char location */
                    if((i==0) && (c == FN_ERASE)) c = 0x05;     /* change 0x05 to 0xE5 for first char only */
                    DOSerror = ERROR_OBJECT_NOT_FOUND;    /* could not find specified object */
                    Mname[i++] = Trans_A_M(toupper(c),trans_tbl);
                }
            }
            break;
        }
        foundbrk++;
    }
    if(mfib) DOSerror = 0;
    return(mfib);
}


#ifdef MFS
static UCHAR *disk_info = "\011disk.info";

/**************************************************************************
struct MFileInfoBlock *GetDummyObject(UCHAR *name, struct MFileInfoBlock *mfib);
**************************************************************************/
struct MFileInfoBlock *GetDummyObject(UCHAR *name, struct MFileInfoBlock *mfib)
{
    UBYTE strln;
F();

/*** This part checks for "disk.info" and creates a dummy object mfib if needed */
    if(mfib->mfib_DiskCluster <= ROOTDIR_CLUST)    /* Make sure we're in the root directory */
    {
        if(0 > (strln = name[0]-disk_info[0])) name = 0; /* Do not create dummy object. name too short */
        else if(0 < strln)                         /* name size greater than dummy object name. */
        {
            switch(name[strln])
            {   /* Search for directory break character for the previous character. */
            case '/' :
            case ':' :
                name += strln;   /* previous character is directory break character */
                break;
            default:
                name = 0;   /* previous character is not a directory break character */
                break;
            }
        }
            
        if( (name)
            && (0 == strnicmp(name+1,disk_info+1,(UBYTE)disk_info[0])))
        {
            extern UBYTE *disk_info_icon;
            extern ULONG disk_info_icon_length;

            mfib->mfib_Protection       = 0;    /* prevent anyone from modifying this data */
            mfib->mfib_start_clust      = ILLEGAL_CLUST;
            mfib->mfib_DiskCluster      = ILLEGAL_CLUST;
            mfib->mfib_DiskDirEntry     = (LONG) &disk_info_icon;
            mfib->mfib_Size             = disk_info_icon_length;
            *(BSTR *)mfib->mfib_FileName  = disk_info;
            DOSerror                    = 0;
            return(mfib);
        }
    }
    DOSerror = ERROR_OBJECT_NOT_FOUND;
    return(0);
}
#endif


/********************
* ExamineObject()  --- Fill in FileInfoBlock pointed to by specified lock.
*************/
int ExamineObject(lock, mfib)
register struct MLock *lock;
register struct MFileInfoBlock *mfib;
{
    UCHAR *bstr;

    if( !mfib) return(DOSFALSE);

        /* fill in FileInfoBlock */
    if(!lock || (lock && (lock->ml_KeyPtr->kp_dir_clust==ROOTDIR_CLUST) && (lock->ml_KeyPtr->kp_dir_ent==-1)))
    {           /* Root Directory ONLY! */
        if(!CompareVolNode(lock)) return(DOSFALSE);

        if(fsys->f_VolumeNode)
        {       /* copy volume label to FileName entry (only if there is a volume node) */
            CopyMem(bstr =(UCHAR *)BADDR(fsys->f_VolumeNode->dol_Name),
                mfib->mfib_FileName, bstr[0]+1);
        }
         else mfib->mfib_FileName[0] = 0;

        mfib->mfib_DiskCluster    =
            (mfib->mfib_start_clust  = ROOTDIR_CLUST);
        mfib->mfib_DiskDirEntry   = -1;
        mfib->mfib_DiskKey        = 0;
        mfib->mfib_Protection     = 0;
        mfib->mfib_EntryType      = (mfib->mfib_DirEntryType = MLDE_DIR);
        mfib->mfib_DirEntStatus   = MLDE_DIR_ROOT;
        mfib->mfib_Size           = 0;
        mfib->mfib_NumBlocks      = fsys->f_root_blocks;

    /* set volume date and time. */
        mfib->mfib_Date.ds_Days   = 15330;
        mfib->mfib_Date.ds_Minute = 0;
        mfib->mfib_Date.ds_Tick   = 0;
        mfib->mfib_Comment[0]     = '\0';
    }
    else if(ILLEGAL_CLUST == lock->ml_KeyPtr->kp_dir_clust)
    {
        if(fsys->f_VolumeNode)
        {       /* copy volume label to FileName entry (only if there is a volume node) */
            CopyMem(bstr =(UCHAR *)lock->ml_KeyPtr->kp_file_name, mfib->mfib_FileName, bstr[0]+1);
        }
        mfib->mfib_DiskKey        = 1;
        mfib->mfib_Protection     = 0;
        mfib->mfib_EntryType      = (mfib->mfib_DirEntryType = MLDE_FILE);
        mfib->mfib_Size           = lock->ml_KeyPtr->kp_file_size;
        mfib->mfib_NumBlocks      = (lock->ml_KeyPtr->kp_file_size/fsys->f_bytes_block)
                                    + (lock->ml_KeyPtr->kp_file_size%fsys->f_bytes_block ? 1 : 0);

    /* set volume date and time. */
        DateStamp(&(mfib->mfib_Date));
        mfib->mfib_Comment[0]     = '\0';
    }
      else
    {           /* All other directories and files */
        mfib->mfib_DiskCluster = lock->ml_KeyPtr->kp_dir_clust;
        mfib->mfib_DiskDirEntry = (lock->ml_KeyPtr->kp_dir_ent)-1;
        mfib->mfib_DirEntStatus  = MLDE_DIR;

        if( ! ExamineNext(lock, mfib)) return(DOSFALSE);

        if(mfib->mfib_DirEntStatus == MLDE_DIR) mfib->mfib_DirEntStatus = MLDE_DIR_ROOT;

/*
DPRTF(KPrintF( "\nmfib_DiskCluster = %ld  mfib_DiskDirEntry = %ld  mfib_DirEntStatus = %ld",
    mfib->mfib_DiskCluster,mfib->mfib_DiskDirEntry,mfib->mfib_DirEntStatus));
*/
    }

    return(DOSTRUE);
}


/********************
* ExamineNext() --- Fill in FileInfoBlock pointed to by specified lock with next
*       directory entry.
*************/
int ExamineNext(lock, mfib)
register struct MLock *lock;
register struct MFileInfoBlock *mfib;
{
    register struct FS_dir_ent    *dirent;
    register BYTE entrytype;
    UWORD volatile cluster;

    if( !mfib) return(DOSFALSE);

    if(!CompareVolNode(lock)) return(DOSFALSE);

        /* next dir entry availiable.  Skip erased entries */
    while((dirent = GetNextDirEnt(mfib)) 
        && ( (dirent->fde_filename[0] == FN_ERASE)
        || (dirent->fde_filename[0] == FN_END)
        || (dirent->fde_filename[0] == FN_DIR)
        || (dirent->fde_protection & ATTF_VOL_LBL) ) );

    if(dirent)
    {       /*** It is assumed the GetNextDirEnt() will set
            DiskCluster, DiskKey, DirEntStatus, DiskDirEntry, Starting cluster
            and Protection to the correct values ***/

            /* set filename */
        ConvertFileName(dirent->fde_filename, mfib->mfib_FileName);

            /* set number of blocks in file. */
          SWAPWORD(cluster,dirent->fde_start_clust);
        mfib->mfib_NumBlocks = usedfileblocks(cluster);

            /* set file date and time. */
        ConvertFileDate( dirent->fde_time, &(mfib->mfib_Date));

        mfib->mfib_Comment[0]     = '\0';

        if(mfib->mfib_DirEntStatus == MLDE_DIR) entrytype = MLDE_DIR;
         else entrytype = MLDE_FILE;    /* default to file status */

        mfib->mfib_EntryType = (mfib->mfib_DirEntryType = entrytype);

        return(DOSTRUE);
    }
    DOSerror = ERROR_NO_MORE_ENTRIES;
    return(DOSFALSE);
}


/********************
* FindVolEntry()  --- Find the volume entry in the root directory.
*   Return memory to dir entry or 0 if not found.
*************/
struct FS_dir_ent *FindVolEntry()
{
    register struct FS_dir_ent *dirent;
    struct MFileInfoBlock   *mfib = (struct MFileInfoBlock *)fsys->f_scratch;

        /* Look in Root Directory ONLY! */
    mfib->mfib_DiskCluster     = ROOTDIR_CLUST;
    mfib->mfib_DiskDirEntry    = -1;
    mfib->mfib_DirEntStatus    = MLDE_DIR_ROOT;

        /* search for Volume entry or End of directory which ever comes first */
    while( (dirent = (struct FS_dir_ent *)GetNextDirEnt(mfib))
        && !((dirent->fde_protection) & ATTF_VOL_LBL) );

    if( !(dirent)
        || (dirent->fde_filename[0] == FN_END) 
        || (dirent->fde_filename[0] == FN_ERASE) ) dirent = 0;

    return(dirent);
}


/********************
* FindParent()  --- Find Parent directory of the specified mfib.  
*   Return mfib or 0 if not found.
*************/
int FindParent(mfib)
register struct MFileInfoBlock *mfib;
{
    register struct FS_dir_ent *dirent;
    UWORD cluster = mfib->mfib_DiskCluster; 
    UWORD volatile parentcluster, clust;
    int ent = -1, found = 0;

    if( cluster == ROOTDIR_CLUST)
    {
        if(mfib->mfib_DiskDirEntry == -1)
        {   /* Already at Root Directory */
            found = 0;
        }
          else
        {   /* File or directory within Root Directory */
            found = 1;
        }
    }
      else if(cluster >= START_CLUST)
    {           /* All other directories and files not in Root Dir */
        mfib->mfib_DiskDirEntry    = -1;
        mfib->mfib_DirEntStatus    = MLDE_DIR;

        while( (!found) && (dirent = GetNextDirEnt(mfib)))
        {       /* Look for parent directory filename ("..") */
            if( (*((UWORD *)&(dirent->fde_filename)) == FN_PARENTDIR)
                && (dirent->fde_protection & ATTF_SUBDIR) )
            {
                    SWAPWORD(parentcluster,dirent->fde_start_clust);
                  mfib->mfib_DiskCluster = parentcluster;
                  mfib->mfib_DiskDirEntry    = -1;
                  mfib->mfib_DirEntStatus    = MLDE_DIR;
                for(ent=0; (!found) && (dirent = GetNextDirEnt(mfib)); ent++)
                {
                      SWAPWORD(clust,dirent->fde_start_clust);
                    if (cluster == clust)
                    {
                        found = 1;
                        ent--;      /* re-adjust entry for loop inc */
                    }
                }
            }
        }
        if(!found) DOSerror = ERROR_DIR_NOT_FOUND; /* set default error if parent not found */
    }
     else   /* who knows what cluster this is? */
    {
        DOSerror = ERROR_OBJECT_WRONG_TYPE;
        found = 0;
    }
    mfib->mfib_DiskDirEntry     = ent;
    
    return(found);
}
