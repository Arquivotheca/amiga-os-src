/* DOS.c **************************************************************
** Copyright 1991 CONSULTRON
*
*      DOS related functions.
*
*************************************************************************/

#include "FS:FS.h"

extern struct FS         *fsys;
extern int DOSerror;
extern struct IOStdReq      *diskreq;
extern struct ExecBase  *SysBase;

static int SetDate_Object(register struct MFileInfoBlock *mfib, ULONG date);
static struct MFileInfoBlock *FindObjDir(register struct MLock *lock, 
                                        UCHAR *name,     /* BADDR(BSTR) */
                                        register struct MFileInfoBlock *mfib);


/*************
* SetDate_Object()  --- Set specified object to the specified date.
*   Return DOSFALSE if cannot perform.
*************/
static int SetDate_Object(register struct MFileInfoBlock *mfib, ULONG date)
{
    register struct FS_dir_ent *dirent;
    int retbool = DOSFALSE;

    if((mfib->mfib_DiskCluster == ROOTDIR_CLUST)
        && (mfib->mfib_DiskDirEntry == -1)) return(DOSTRUE);   /* point to ROOT dir already */

      mfib->mfib_DiskDirEntry  -= 1;   /* point to previous entry */
      mfib->mfib_DirEntStatus  = MLDE_DIR;
    if(dirent = GetNextDirEnt(mfib))
    {
        *(ULONG *)(dirent->fde_time) = date;

    /* clear archive bit */
        dirent->fde_protection |= TO_BE_ARCHIVED_FLAG;
 
        if( StoreDirEnt(dirent))
        {
            retbool = DOSTRUE;
        }
    }
    return(retbool);
}


/*************
* SetDate()  --- Set specified Object to the specified date.
*   Return DOSFALSE if cannot perform.
*************/
int SetDate(dirlock, name, date)
register struct MLock *dirlock;
UCHAR *name;    /* BADDR(BSTR) */
struct DateStamp *date;
{
    register struct MFileInfoBlock *mfib = (struct MFileInfoBlock *)fsys->f_scratch;
    register struct MLock *lock;
    int retbool = DOSFALSE;

    if(!CompareVolNode_Write(dirlock)) return(DOSFALSE);

    if( (lock = GetLock(dirlock, name, SHARED_LOCK)))
    {
          mfib->mfib_DiskCluster   = lock->ml_lock_ext.le_dir_clust;   /* point to disk cluster */
          mfib->mfib_DiskDirEntry  = lock->ml_lock_ext.le_dir_ent;   /* point to previous entry */
        if(SetDate_Object(mfib,ConvertFromDateStamp(date))) retbool = DOSTRUE;
    }
    FreeLock(lock);
    return(retbool);
}

/*************
* SetProtect()  --- Set specified Object to the specified protection attributes.
*   Return DOSFALSE if cannot perform.
*************/
int SetProtect(dirlock, name, attributes)
register struct MLock *dirlock;
UCHAR *name;    /* BADDR(BSTR) */
ULONG attributes;
{
    register struct MFileInfoBlock *mfib = (struct MFileInfoBlock *)fsys->f_scratch;
    register struct MLock *lock;
    register struct FS_dir_ent *dirent;
    int retbool = DOSFALSE;

    if(!CompareVolNode_Write(dirlock)) return(DOSFALSE);

    if( (lock = GetLock(dirlock, name, EXCLUSIVE_LOCK)))
    {
          mfib->mfib_DiskDirEntry  = (lock->ml_lock_ext.le_dir_ent)-1;   /* point to previous entry */
          mfib->mfib_DirEntStatus  = MLDE_DIR;
        dirent = GetNextDirEnt(mfib);

        if(attributes & (FIBF_WRITE|FIBF_DELETE)) dirent->fde_protection |= ATTF_READONLY;
          else dirent->fde_protection &= ~ATTF_READONLY;
        if(attributes & FIBF_ARCHIVE) dirent->fde_protection &= ~TO_BE_ARCHIVED_FLAG;
          else dirent->fde_protection |= TO_BE_ARCHIVED_FLAG;

        if( StoreDirEnt(dirent))
        {
            retbool = DOSTRUE;
        }
    }

    FreeLock(lock);
    return(retbool);
}

/*************
* SetDate_ParentDir()  --- Set the parent directory date.
*   mfib modified to point to parent directory if successful.
*************/
void SetDate_ParentDir(mfib)
register struct MFileInfoBlock *mfib;
{
    register struct MFileInfoBlock prntmfib;

/* find the parent directory of specified mfib */
    CopyMem((UBYTE *) mfib, (UBYTE *)&prntmfib, sizeof(struct MinFileInfoBlock));

    if( !FindParent(&prntmfib)) return;

    SetDate_Object(&prntmfib,SetCurrentTime());
}


/*************
* AllocDirEnt()  --- Allocate next availiable Directory entry 
*       from WITHIN the specified directory.
*
*   Return memory pointer to directory entry or 0 if error
*   mfib is updated with the current dirent specs
*************/
struct FS_dir_ent *AllocDirEnt(mfib)
register struct MFileInfoBlock *mfib;
{
    register struct FS_dir_ent   *dirent;
    LONG cluster=mfib->mfib_DiskCluster;
    ULONG time;
    UWORD i;

    if( (mfib->mfib_DirEntStatus < MLDE_DIR) ) return(0);  /* not a directory */

/* Set directory (which is the parent) to current date */
    SetDate_Object(mfib,(time = SetCurrentTime()));

/* Look for unused entry inside directory */
    mfib->mfib_DirEntStatus = MLDE_DIR_ROOT;   /* go WITHIN the directory */

         /* search next dir entry availiable.  Look for ERASED or END entries */
    while((dirent = GetNextDirEnt(mfib)) 
        && ((dirent->fde_filename[0] != FN_ERASE)
        && ( dirent->fde_filename[0] != FN_END) ) );

    if( !dirent )       /* run out of unused directory entries */
    {
    /* Try to allocate another cluster full of dir ents */
        if(cluster = AllocCluster(mfib->mfib_DiskCluster) )
        {
            mfib->mfib_DiskDirEntry += 1;   /* point to new (next) dir entry */
            if(dirent = (struct FS_dir_ent *)PUTCLUSTER(cluster,0) )
                    /* 0 = no more clusters availible (either ROOT DIR or DISK FULL) */
            {       /* put FN_END in directory entries */
                for(i=0; i<(fsys->f_dirents_clust); i++) dirent[i].fde_filename[0] = '\0';
            }
        }
    }

    if(dirent)
    {
        memset((UBYTE *)dirent,'\0',sizeof(struct FS_dir_ent));
        dirent->fde_filename[0] = FN_ERASE;     /* make sure entry is 'erased'
                                                just in case  later operation fails */
    /**** Set Current Date and Time on Directory entry ****/
        *((ULONG *)(dirent->fde_time)) = time; /* copy creation time */
    }

    return((struct FS_dir_ent *)StoreDirEnt(dirent)); 
}


/********
*   PutDirEnt() -- Put copy of Directory Entry given into Directory Entry
*       cache memory.
*
*       Cache management required.
*   It is ASSUMED that the block # requested is IN the data block area.
*
*   Return ptr to Directory Entry cache memory or = 0 for error
********/
UBYTE *PutDirEnt(dirent, cluster, direntnum)
register struct FS_dir_ent *dirent;
LONG cluster;
WORD direntnum;
{
    register struct MinFileInfoBlock mmfib;
    register struct FS_dir_ent    *destdirent;

        /** look for requested block in cache **/
      mmfib.mmfib_DiskCluster   = (cluster);     /* point to disk clust */
      mmfib.mmfib_DiskDirEntry  = (direntnum)-1; /* point to previous entry */
      mmfib.mmfib_DirEntStatus  = MLDE_DIR;
    if(destdirent = GetNextDirEnt((struct MFileInfoBlock *)&mmfib))  /* check if directory entry available */
    {
        CopyMem((UCHAR *)dirent, (UCHAR *)(destdirent), sizeof(struct FS_dir_ent));
    }

    return(StoreDirEnt(destdirent));
}


/********
*   FindObjDir() -- Find the directory node of the specified filespec
*
*   This routine does NOT assume that the object is found or availiable.
*   Whereas, the FindParent() routine assumes the lock to the object is valid.
*
*   Return with mfib or = 0 for error.  The mfib struct will be updated regardless.
********/
static struct MFileInfoBlock *FindObjDir(register struct MLock *lock, 
                                        UCHAR *name,     /* BADDR(BSTR) */
                                        register struct MFileInfoBlock *mfib)
{
    int i;
    UBYTE   strsize = name[0];


        /* adjust BSTR to point to parent of non-existant object */

    for(i=strsize, name[0]=0; i>0 ; i--)
    {
        switch(name[i])
        {
            case '/' :
            case ':' :
                name[0] = i;
                i=0;        /* this will break out of the for() */
                break;
            default:
                break;
        }
    }
        /* Find parent of filespec */
    if( ! FindObject(lock, name, mfib)
        || (mfib->mfib_DirEntStatus < MLDE_DIR) )
    {       /* directory path not found */
        DOSerror = ERROR_DIR_NOT_FOUND;
        mfib = 0;
    }

    name[0] = strsize;
    return(mfib);
}


/*************
* RenameObject()  --- Rename specified file or directory.
*
* Because TWO or more directories may be changed at a time, the directory
* entries may NOT be allowed to be changed directly into the memory for
* fear of the cache management reusing it.  Therefore PutDirEnt() must be used.
*
* Return DOSFALSE if not successful.
*************/
int RenameObject(fromlock, fromname, tolock, toname)
register struct MLock *fromlock, *tolock;
register UCHAR *fromname, *toname;    /* BADDR(BSTR) */
{
    register struct MFileInfoBlock *mfib = (struct MFileInfoBlock *)fsys->f_scratch;
    register struct MLock *newlock=0, *chklock;
    register struct MFileInfoBlock *tomfib;
    struct MinFileInfoBlock mmfib;
    int retbool = DOSFALSE;
    LONG fromclust, toclust;
    WORD fromdirent, todirent;
    UWORD volatile tempword;
    struct FS_dir_ent  dirent;
    register struct FS_dir_ent *tmpdirent;
    UBYTE setdate_pd=TRUE, copy_dirent=TRUE;


    if(!CompareVolNode_Write(fromlock)) return(DOSFALSE);

/* get shared lock for object to be renamed
IMPORTANT -- GetLock() is also used to test for exclusive lock */
/* Abort rename if newlock == Disk name */
    if( (newlock = GetLock(fromlock, fromname, SHARED_LOCK))
        && !((newlock->ml_lock_ext.le_dir_clust == ROOTDIR_CLUST) && (newlock->ml_lock_ext.le_dir_ent == -1)))
    {
    /* copy old dirent into memory and remember where it is currently stored */ 
          mfib->mfib_DiskCluster    = (fromclust  = newlock->ml_lock_ext.le_dir_clust);
          mfib->mfib_DiskDirEntry   = (fromdirent = newlock->ml_lock_ext.le_dir_ent)-1;
          mfib->mfib_DirEntStatus   = MLDE_DIR;
        CopyMem( (UCHAR *)GetNextDirEnt(mfib), (UCHAR *)&dirent,
            sizeof(struct FS_dir_ent));

    /* find if new object name already exists */
        if( tomfib = FindObject(tolock, toname, mfib))
        {
            DOSerror = ERROR_OBJECT_EXISTS;
        }

    /* It is assumed that the variable "mfib->mfib_Oname" contains
    the object name last searched.
    Also,  "mfib"  that  is used contains cluster and direntry
    pointers to the last directory searched */
        CopyMem( mfib->mfib_Oname, dirent.fde_filename, FNAME_TOTALSZ);

        if( tomfib )
        {
            if((tomfib->mfib_DiskCluster == fromclust)
                && (tomfib->mfib_DiskDirEntry == fromdirent))
            {   /* SAME PARENTDIR , SAME NAME */
                setdate_pd = FALSE;
                copy_dirent= FALSE;
                retbool = DOSTRUE;
            }
            /* else -- Object exists. Cannot do rename */
        }
         else
        {    /* Object does not exist. Find parent dir of to object */
            if( ! (tomfib = FindObjDir(tolock, toname, mfib)) )
            {       /* parent directory path not found for object */
                DOSerror = ERROR_OBJECT_NOT_FOUND;
            }
             else
            {       /* parent directory path found for object */
                if( ((tomfib->mfib_start_clust) == fromclust) )
                {       /* SAME PARENTDIR , DIFF NAME */
                    copy_dirent= FALSE;
                    retbool = DOSTRUE;
                }
                 else if(mfib->mfib_DirEntStatus & MLDE_DIR)
                {       /* From Object is a Directory */
                    CopyMem( tomfib, &mmfib, sizeof(struct MinFileInfoBlock));

                /* Find if To filespec a subdir of From Object */
                    do
                    {
                        if((mmfib.mmfib_DiskCluster == fromclust)
                            && (mmfib.mmfib_DiskDirEntry == fromdirent))
                        {
                            DOSerror = ERROR_OBJECT_IN_USE;
                            break;  /* break out of 'do' */
                        }
                    } while(FindParent((struct MFileInfoBlock *)&mmfib));
                }
                /* else object a file */

                if(DOSerror != ERROR_OBJECT_IN_USE)
                {   /* To filespec NOT a subdir of From Object */
                    if( AllocDirEnt(tomfib) )
                    {   /* allocated new directory entry */
                        retbool = DOSTRUE;
                    }
                     else
                    {   /* could not allocate new directory entry */
                        DOSerror = ERROR_DISK_FULL;
                    }
                }
            }
        }

        if(retbool==DOSTRUE)
        {       /* continue, no error occurred yet */
            if(copy_dirent == TRUE)
            {       /* Renaming across directories */
                /* save new dirent position */
                  toclust   = tomfib->mfib_DiskCluster;
                  todirent  = tomfib->mfib_DiskDirEntry;

                if(mfib->mfib_DirEntStatus & MLDE_DIR)
                {
                /* move a directory.  Update directory parent pointer */
                      mfib->mfib_DiskCluster    = newlock->ml_lock_ext.le_start_clust;
                      mfib->mfib_DiskDirEntry   = -1;
                      mfib->mfib_DirEntStatus   = MLDE_DIR;
                    while((tmpdirent = GetNextDirEnt(mfib))
                        && !(((UWORD *)(tmpdirent->fde_filename))[0] == FN_PARENTDIR));
                    if(tmpdirent)
                    {
                        tempword = (UWORD)toclust;
                        SWAPWORD(tmpdirent->fde_start_clust,tempword);
                        if( !StoreDirEnt(tmpdirent))
                        {
                            retbool = DOSFALSE;
                        }
                    }
                }
                if(retbool == DOSTRUE)
                {       /* Copy dir ent into destination dir ent */
                    if( !PutDirEnt(&dirent,toclust,todirent))
                    {
                        retbool = DOSFALSE;
                    }
                }
                if(retbool == DOSTRUE)
                {       /* Erase old FROM dir ent */
                      mfib->mfib_DiskCluster    = fromclust;    /* point to disk clust */
                      mfib->mfib_DiskDirEntry   = fromdirent-1; /* point to previous entry */
                      mfib->mfib_DirEntStatus   = MLDE_DIR;
                    if(tmpdirent = GetNextDirEnt(mfib))
                    {
                        tmpdirent->fde_filename[0] = FN_ERASE;
                        if( !StoreDirEnt(tmpdirent))
                        {
                            retbool = DOSFALSE;
                        }
                        SetDate_ParentDir(mfib);
                    }
                    else retbool = DOSFALSE;
                }
                chklock = newlock;
                    /* fix ALL shared locks */
                while( chklock = (struct MLock *)BADDR(chklock->ml_lock.fl_Link))
                {
                    if( (chklock->ml_lock_ext.le_dir_ent == fromdirent)
                        && (chklock->ml_lock_ext.le_dir_clust == fromclust))
                    {
                        chklock->ml_lock_ext.le_dir_ent = todirent;
                        chklock->ml_lock_ext.le_dir_clust = toclust;
                        chklock->ml_lock.fl_Key = ConvertCluster(toclust,todirent);
                        ((UBYTE *)&(chklock->ml_lock.fl_Key))[0]  = 
                            (UBYTE)((todirent)%(fsys->f_dirents_block));
                    }
                }
            }
             else
            {       /* Renaming within the same directory */
                  mfib->mfib_DiskCluster    = fromclust;    /* point to disk clust */
                  mfib->mfib_DiskDirEntry   = fromdirent-1; /* point to previous entry */
                  mfib->mfib_DirEntStatus   = MLDE_DIR;
                if(tmpdirent = GetNextDirEnt(mfib))
                {
                    CopyMem(dirent.fde_filename,tmpdirent->fde_filename, FNAME_TOTALSZ);
                    if( StoreDirEnt(tmpdirent))
                    {
                        if(setdate_pd == TRUE)
                        {
                        /* Set parent directory to current date */
                            SetDate_ParentDir(tomfib);
                        }
                    }
                    else retbool = DOSFALSE;
                }
                 else retbool = DOSFALSE;
            }
        }
    }

     else
    {
        DOSerror = ERROR_OBJECT_NOT_FOUND;
    }

    FreeLock(newlock);
    return(retbool);
}


/*************
* CreateObject()  --- Create a directory or file Entry.
*
*   Return filled mfib or 0 if error
*************/
struct MFileInfoBlock *CreateObject(lock, name, mfib)
register struct MLock *lock;
UCHAR *name;     /* BADDR(BSTR) */
register struct MFileInfoBlock *mfib;
{
    register struct MFileInfoBlock *newmfib=0;
    register struct FS_dir_ent   *dirent;
    UCHAR ObjName[FNAME_TOTALSZ];
    UWORD volatile firstclust;
    UBYTE fileflags;
    

    if(!CompareVolNode_Write(lock)) return(DOSFALSE);

    /* find if new object name already exists */
    if( FindObject(lock, name, mfib) )
    {
        DOSerror = ERROR_OBJECT_EXISTS;
        return(DOSFALSE);
    }

        /* It is assumed that if the object is not found, that the
        variable "mfib->mfib_Oname" contains the filename object that
        was not found.
        Also,  "mfib"  that  is used contains cluster and direntry
        pointers to the last directory searched */
      else if(DOSerror == ERROR_OBJECT_NOT_FOUND)
    {
       CopyMem( mfib->mfib_Oname, ObjName, FNAME_TOTALSZ);
        fileflags = mfib->mfib_FileFlags;

            /* Find parent of filespec */
        if( ! FindObjDir(lock, name, mfib) )
        {       /* directory path not found */
            return(DOSFALSE);
        }

                /* directory path found */
            /* allocate sub directory cluster or beginning of data */
        if( !( dirent = AllocDirEnt(mfib))   /* could not allocate directory entry */
            || !(firstclust = AllocCluster(FAT_EOF)) )  /* or could not allocate
                                                    first cluster of file or directory */
        {
            return(DOSFALSE);
        }

        newmfib = mfib;
        mfib->mfib_Protection = dirent->fde_protection = 0;

        mfib->mfib_start_clust = firstclust;
          SWAPWORD(dirent->fde_start_clust,firstclust);
        mfib->mfib_Size = *((ULONG *)(dirent->fde_file_size)) = 0;

            /* store directory name */
        CopyMem( ObjName, dirent->fde_filename, FNAME_TOTALSZ);

        if( !StoreDirEnt(dirent))
        {
            return(DOSFALSE);
        }

        mfib->mfib_FileFlags = fileflags;
    }

    return(newmfib);
}


/*************
* CreateDirectory()  --- Create a Directory Entry.
*
*   Return lock or 0 if error
*************/
struct MLock *CreateDirectory(lock, name)
register struct MLock *lock;
UCHAR *name;     /* BADDR(BSTR) */
{
    register struct MFileInfoBlock *mfib = (struct MFileInfoBlock *)fsys->f_scratch;
    register struct MLock *dirlock=DOSFALSE;
    UWORD volatile dirclust, subdirclust;
    WORD i;
    register struct FS_dir_ent   *dirent;
    ULONG time;

    /* Try to create new object (directory) */
    if( !CreateObject(lock, name, mfib) )
    {
        return(DOSFALSE);
    }

               /* save new dirent position */
      dirclust = mfib->mfib_DiskCluster;
      mfib->mfib_DiskDirEntry  += -1;      /* point to previous entry */
      mfib->mfib_DirEntStatus  = MLDE_DIR;
    if(!(dirent = GetNextDirEnt(mfib)))
    {
        return(DOSFALSE);
    }

      dirent->fde_protection = ATTF_SUBDIR;

    if( !StoreDirEnt(dirent))
    {
        return(DOSFALSE);
    }

      SWAPWORD(subdirclust,dirent->fde_start_clust);

      time = *((ULONG *)(dirent->fde_time));


        /* Fill-in subdir params */
    if( !(dirent = (struct FS_dir_ent *)PUTCLUSTER(subdirclust,0) ))
    {
        return(DOSFALSE);
    }
/* Put FILE NAME END in remaining directory entries */
    for(i=0; i<(fsys->f_dirents_clust); i++) (dirent[i].fde_filename)[0] = FN_END;

/* Clear DIR and PARENT entries */
    memset((UBYTE *)dirent,'\0',2*sizeof(struct FS_dir_ent));

    memset((UBYTE *)&(dirent[0]), ' ', FNAME_TOTALSZ); /* set to all spaces */
    memset((UBYTE *)&(dirent[1]), ' ', FNAME_TOTALSZ); /* set to all spaces */

        /* Fill in current directory params */
    (dirent[0].fde_filename)[0] = FN_DIR;
    dirent[0].fde_protection = ATTF_SUBDIR;
    SWAPWORD(dirent[0].fde_start_clust,subdirclust);
    *((ULONG *)&(dirent[0].fde_time)) = time; /* copy creation time */

        /* Fill in parent directory params */
    (dirent[1].fde_filename)[0] = FN_DIR;
    (dirent[1].fde_filename)[1] = FN_DIR;
    dirent[1].fde_protection = ATTF_SUBDIR;
    SWAPWORD(dirent[1].fde_start_clust,dirclust);
    *((ULONG *)&(dirent[1].fde_time)) = time; /* copy creation time */

    if( !StoreDirEnt(dirent))
    {
        return(DOSFALSE);
    }

    dirlock = MakeLock(lock, mfib, SHARED_LOCK);

    return(dirlock);
}


/*************
* DeleteObject()  --- Delete the specified Object
*
*   Return DOSTRUE if successful or DOSFALSE if error
*************/
int DeleteObject(lock, name)
register struct MLock *lock;
UCHAR *name;     /* BADDR(BSTR) */
{
    register struct MFileInfoBlock *mfib = (struct MFileInfoBlock *)fsys->f_scratch;
    register struct MLock *dirlock=0;
    int retbool = DOSFALSE;
    register struct FS_dir_ent   *dirent;
    UWORD volatile cluster;
    UWORD volatile tempword;
    WORD direntnum;

    if(!CompareVolNode_Write(lock)) return(DOSFALSE);

        /* get exclusive lock for object to be deleted
         IMPORTANT -- GetLock() is also used to test for exclusive lock */
    if( (dirlock = GetLock(lock, name, EXCLUSIVE_LOCK) )
        && (dirlock->ml_lock_ext.le_dir_ent > -1) )
    {
                /* get dirent for dirlock */
          mfib->mfib_DiskCluster   = (dirlock->ml_lock_ext.le_dir_clust);   /* point to disk clust */
          mfib->mfib_DiskDirEntry  = (dirlock->ml_lock_ext.le_dir_ent)-1;   /* point to previous entry */
          mfib->mfib_DirEntStatus  = MLDE_DIR;
        dirent = GetNextDirEnt(mfib);

        if(mfib->mfib_Protection & FIBF_DELETE)
        {       /* Object cannot be deleted */
            DOSerror = ERROR_DELETE_PROTECTED;
            mfib = 0;
        }

            /**** Check if Directory.  If so, make sure directory is empty ****/
        else if(mfib->mfib_DirEntStatus == MLDE_DIR)
        {       /* Object is a directory */
            direntnum = mfib->mfib_DiskDirEntry;
            cluster = mfib->mfib_DiskCluster;

                SWAPWORD(tempword,dirent->fde_start_clust);
              mfib->mfib_DiskCluster = tempword;
              mfib->mfib_DiskDirEntry  = -1;
              mfib->mfib_DirEntStatus  = MLDE_DIR;
            while((dirent = GetNextDirEnt(mfib)) 
                && ( (dirent->fde_filename[0] == FN_ERASE)
                || (dirent->fde_filename[0] == FN_END)
                || (dirent->fde_filename[0] == FN_DIR) ) );

            if(dirent)
            {
                DOSerror = ERROR_DIRECTORY_NOT_EMPTY;
                mfib = 0;
            }            
              else
            {
                  mfib->mfib_DiskCluster   = cluster;       /* point to disk clust */
                  mfib->mfib_DiskDirEntry  = direntnum-1;   /* point to previous entry */
                  mfib->mfib_DirEntStatus  = MLDE_DIR;
                dirent = GetNextDirEnt(mfib);
            }
        }

        if(mfib)
        {
                /**** Deallocating the object clusters ****/
            SWAPWORD(cluster,dirent->fde_start_clust);
            FreeClusters(cluster);

                /* store ERASE in name of dirent position */
              (dirent->fde_filename)[0] = FN_ERASE;

            if( StoreDirEnt(dirent))
            {
                retbool = DOSTRUE;
            }
/* update parent dirs when file in dir modified */
        /* Set parent directory to current date */
            SetDate_ParentDir(mfib);
        }

    FreeLock(dirlock);
    }

    return(retbool);
}
