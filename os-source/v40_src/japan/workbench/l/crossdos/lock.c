/* Lock.c **************************************************************
** Copyright 1991 CONSULTRON
*
*      Lock related functions.
*
*************************************************************************/

#include "FS:FS.h"

extern struct ExecBase  *SysBase;
extern struct FS  *fsys;
extern int DOSerror;
extern struct IOStdReq      *diskreq;
extern struct MsgPort       *StatusPort;

//extern VOID __stdargs kprintf();	/* wc.  debug */

extern struct Library *JCCBase ;			/*  wc */

static void Add2LockList(struct MLock *lock, struct MLock *newlock );

/*************
* MakeLock()  --- Allocate lock memory and fill minimum values.
*************/
struct MLock *MakeLock(lock, mfib, mode)
struct MLock *lock;
register struct MFileInfoBlock *mfib;
LONG mode;
{
    register struct MLock *newlock;
    register struct MLock *samelock=0;

//kprintf ("call MakeLock\n");

    if(fsys->f_LockList)
    {
        samelock=(struct MLock *)&(fsys->f_LockList);
            /* search for other locks */
        while( samelock = (struct MLock *)BADDR(samelock->ml_lock.fl_Link))
        {       /* check if Lock already availiable. If so return with NO LOCK */
            if( (samelock->ml_lock_ext.le_dir_ent == mfib->mfib_DiskDirEntry)
                && (samelock->ml_lock_ext.le_dir_clust == mfib->mfib_DiskCluster))
            {
                if( (mode == EXCLUSIVE_LOCK)
                    || (samelock->ml_lock.fl_Access == EXCLUSIVE_LOCK))
                {
                    DOSerror = ERROR_OBJECT_IN_USE;   /* found another Lock return NO LOCK */
                    return(DOSFALSE);
                }
                  else break;   /* break out of while loop */
            }
        }
    }

    if(samelock)
    {
        newlock = CopyLock(samelock);

//kprintf ("copy lock %lx   size = %lx\n", newlock, sizeof(struct MLock));

            /* do not inherit filter flags from other locks */
        newlock->ml_lock_ext.le_filt_flags     = mfib->mfib_FileFlags;
	newlock->ml_lock_ext.le_ReadHalf = FALSE;	/* wc */
	newlock->ml_lock_ext.le_WrtHalf = FALSE;	/* wc */
//	newlock->ml_lock_ext.le_HalfJIS2 = FALSE;	/* wc */
	newlock->ml_lock_ext.le_jcc_handle1 = AllocConversionHandle(TAG_DONE);  /* wc */
	newlock->ml_lock_ext.le_jcc_handle2 = AllocConversionHandle(TAG_DONE);  /* wc */

//kprintf ("alloc conversion handle %lx\n", newlock->ml_lock_ext.le_jcc_handle1);
//kprintf ("alloc conversion handle %lx\n", newlock->ml_lock_ext.le_jcc_handle2);

        return(newlock);
    }

/*  else   No lock found on the same object */
    if( !(newlock = (struct MLock *)AllocMem(sizeof(struct MLock),MEMF_PUBLIC|MEMF_CLEAR)))
    {
        DOSerror = ERROR_NO_FREE_STORE;
        return(DOSFALSE);
    }
//kprintf ("make lock %lx   size = %lx\n", newlock, sizeof(struct MLock));

      newlock->ml_lock.fl_Key               = mfib->mfib_DiskKey;
      ((UBYTE *)&(newlock->ml_lock.fl_Key))[0]  =
        (UBYTE)((mfib->mfib_DiskDirEntry)%(fsys->f_dirents_block));

      newlock->ml_lock.fl_Volume            = MKBPTR(fsys->f_VolumeNode);
      newlock->ml_lock.fl_Task              = StatusPort;
      newlock->ml_lock_ext.le_dir_clust     = mfib->mfib_DiskCluster;
      newlock->ml_lock_ext.le_dir_ent       = mfib->mfib_DiskDirEntry;
      newlock->ml_lock_ext.le_start_clust   = mfib->mfib_start_clust;
      newlock->ml_lock_ext.le_filt_flags    = mfib->mfib_FileFlags;
      newlock->ml_lock_ext.le_file_size     = mfib->mfib_Size;
      newlock->ml_lock_ext.le_file_pos      = 0;

      newlock->ml_lock_ext.le_ReadHalf = FALSE;		/* wc */
      newlock->ml_lock_ext.le_WrtHalf = FALSE;		/* wc */
//      newlock->ml_lock_ext.le_HalfJIS2 = FALSE;	/* wc */

      newlock->ml_lock_ext.le_jcc_handle1 = AllocConversionHandle(TAG_DONE);  /* wc */
      newlock->ml_lock_ext.le_jcc_handle2 = AllocConversionHandle(TAG_DONE);  /* wc */

//kprintf ("alloc conversion handle %lx\n", newlock->ml_lock_ext.le_jcc_handle1);
//kprintf ("alloc conversion handle %lx\n", newlock->ml_lock_ext.le_jcc_handle2);

    if( mode != EXCLUSIVE_LOCK) mode        = SHARED_LOCK;
      newlock->ml_lock.fl_Access            = mode;
    Add2LockList(lock, newlock);

      newlock->ml_lock_ext.le_samelock.mln_Succ =
      newlock->ml_lock_ext.le_samelock.mln_Pred =
        (struct MinNode *)&(newlock->ml_lock_ext);

    return( newlock );
}


/*************
* CompareVolNode()  --- Make sure Volume node of lock == current Volume node.
*************/
int CompareVolNode(lock)
register struct MLock *lock;
{
    if(lock)
    {
        if(fsys->f_VolumeNode != (struct DosList *)BADDR(lock->ml_lock.fl_Volume))
        {
            DOSerror = ERROR_DEVICE_NOT_MOUNTED;
            return(DOSFALSE);
        }
    }
      else if((fsys->f_FSflags & MF_NODISK))
    {
        DOSerror = ERROR_NO_DISK;
        return(DOSFALSE);
    }
      else if(!(fsys->f_FSflags & MF_FSDISK))
    {
        DOSerror = ERROR_NOT_A_DOS_DISK;
        return(DOSFALSE);
    }
    return(DOSTRUE);
}


/*************
* CompareVolNode_Write()  --- Make sure Volume node of lock ==
*       current Volume node and not write protected.
*************/
int CompareVolNode_Write(lock)
register struct MLock *lock;
{
    int bool = DOSTRUE;

    if((bool = CompareVolNode(lock)) == DOSTRUE)
    {
        if(fsys->f_Diskflags & WRITE_PROT_F)
        {
            DOSerror = ERROR_DISK_WRITE_PROTECTED;
            bool=DOSFALSE;
        }
         else if(lock && (lock->ml_lock_ext.le_flags & FILE_WRITE_PROT_F))
        {
            DOSerror = ERROR_WRITE_PROTECTED;   /* Set the DOS error but
                                            return no error from function */
        }
    }
    return(bool);
}


/**********************************************************************
*   Find_File_Mod()  --- Find if any file is modified given the locklist.
*   If file found to be modified - return(DOSTRUE)
*   If file not found to be modified - return(DOSFALSE)
**********************************************************************/
int Find_File_Mod(locklist)
register BPTR locklist;
{
    register struct MLock *lk;
    register BPTR lk_next = locklist;

    while (lk = (struct MLock *) BADDR(lk_next))
    {
        lk_next = lk->ml_lock.fl_Link;
        if(lk->ml_lock_ext.le_flags & FILE_MOD_F)
        {   /* Found modified file */
            return(DOSTRUE);
        }
    }
    return(DOSFALSE);
}


/*************
* GetLock()  --- Find specified directory or file and allocate lock.
*************/
struct MLock *GetLock(lock, name, mode)
register struct MLock *lock;
UCHAR *name;    /* BADDR(BSTR) */
LONG mode;
{
    register struct MFileInfoBlock *mfib = (struct MFileInfoBlock *)fsys->f_scratch;
    register struct MLock *newlock = DOSFALSE;

    if( FindObject(lock, name, mfib) )
    {
        newlock = MakeLock(lock, mfib, mode);
    }
    return(newlock);
}


/**********************************************************************
*   Add2LockList() -- Add the new lock to the current lock list.
*
*   The current lock list is determined by the following criteria in order
*   of priorty:
*   - The dol_LockList of the volume node of the associated lock passed.
*   - The f_LockList of the struct FS.
**********************************************************************/
static void Add2LockList(struct MLock *lock, struct MLock *newlock )
{
    ULONG *locklist;
    register struct DosList *volnode;

    if(     lock
        &&  (volnode = (struct DosList *)BADDR(lock->ml_lock.fl_Volume))
        &&  (volnode->dol_Type == DLT_VOLUME)
        &&  *(locklist = &(volnode->dol_misc.dol_volume.dol_LockList)));
    else locklist = &(fsys->f_LockList);
DPRTF(KPrintF( "\nAdd2LockList: locklist=%lx",locklist));
    newlock->ml_lock.fl_Link    = *locklist;
    *locklist                   = MKBPTR(newlock);
}


/*************
* CopyLock()  --- Duplicate and link specified lock.
*************/
struct MLock *CopyLock(lock)
register struct MLock *lock;
{
    struct MLock *newlock=DOSFALSE;

    DOSerror = ERROR_OBJECT_IN_USE;   /* if not a shared Lock return NO LOCK */

    if(!lock)       /* lock = 0; get lock on root of filesystem */
    {
        if(!(newlock = GetLock(0,"",SHARED_LOCK)))
        {
            return(DOSFALSE);
        }
    }
    else if(lock->ml_lock.fl_Access == SHARED_LOCK)
    {
        if( !(newlock = (struct MLock *)AllocMem(sizeof(struct MLock),MEMF_PUBLIC)))
        {
            DOSerror = ERROR_NO_FREE_STORE;
            return(DOSFALSE);
        }
        CopyMem( (UBYTE *)lock, (UBYTE *)newlock, sizeof(struct MLock));
        newlock->ml_lock_ext.le_file_pos    = 0;
        Add2LockList(lock, newlock);
    }
/* Link locks of the same object together in a circular list */
/*### A circular list does not have a list header.  It is basically a double-linked list
    with all the nodes in the list linked so that proceeding in the list forwards or
    backwards always brings you back to the node you started with.  To keep you from
    going in circles forever during list checks, keep a copy of the original node you
    started with and compare it to the node being currently checked.
###*/
/*### The following lines of code are equivalent to Insert() for this use of a circular list
    if(newlock)
    {
        newlock->ml_lock_ext.le_samelock.mln_Succ = (struct MinNode *)&(lock->ml_lock_ext.le_samelock.mln_Succ);
        (newlock->ml_lock_ext.le_samelock.mln_Pred = lock->ml_lock_ext.le_samelock.mln_Pred)->mln_Succ =
            (struct MinNode *)&(newlock->ml_lock_ext.le_samelock.mln_Succ);
        lock->ml_lock_ext.le_samelock.mln_Pred = (struct MinNode *)&(newlock->ml_lock_ext.le_samelock.mln_Succ);
    }
###*/
    if(newlock) Insert(0,(struct Node *)&(newlock->ml_lock_ext),(struct Node *)&(lock->ml_lock_ext));

    return( newlock );
}


/*************
* FreeLock()  --- Unlink and deallocate lock
*************/
int FreeLock(lock)
register struct MLock *lock;
{
    register struct MLock *lk_prev=0;
    register struct MLock *lk;
    BPTR lk_next;
    register struct DosList *vol;

//kprintf ("call FreeLock\n");

    if(!lock)
    {
        return(DOSTRUE);
    }

    if(CompareVolNode(lock))
    {       /* if lock from current volume -- use f_LockList */
        vol = fsys->f_VolumeNode;
        lk_prev = (struct MLock *)&(fsys->f_LockList);
    }
     else
    {       /* Lock not from current volume.  Free only if dol_Type is DLT_VOLUME */
        vol = (struct DosList *)BADDR(lock->ml_lock.fl_Volume);
        if(vol->dol_Type == DLT_VOLUME) lk_prev = (struct MLock *)&(vol->dol_misc.dol_volume.dol_LockList);
    }

    if(lk_prev)
    {
        lk = (struct MLock *) BADDR(lk_prev->ml_lock.fl_Link);

        while (lk)
        {
            lk_next = lk->ml_lock.fl_Link;
            if(lk == lock)
            {
		/* free jstring conversion handles.  wc */
		if (lock->ml_lock_ext.le_jcc_handle1)
		    {
		    FreeConversionHandle (lock->ml_lock_ext.le_jcc_handle1);
//kprintf ("free conversion handle %lx\n", lock->ml_lock_ext.le_jcc_handle1);
		    lock->ml_lock_ext.le_jcc_handle1 = NULL;
		    }
		if (lock->ml_lock_ext.le_jcc_handle2)
		    {
		    FreeConversionHandle (lock->ml_lock_ext.le_jcc_handle2);
//kprintf ("free conversion handle %lx\n", lock->ml_lock_ext.le_jcc_handle2);
		    lock->ml_lock_ext.le_jcc_handle2 = NULL;
		    }

                lk_prev->ml_lock.fl_Link = lk_next;   /* unlink from single-link list */

        /* unlink lock from lock list of same object */
                Remove((struct Node *)&(lock->ml_lock_ext));
        /****/
                FreeMem((UBYTE *)lock, sizeof(struct MLock));
//kprintf ("free lock %lx   size = %lx\n", lock, sizeof(struct MLock));

                if( (fsys->f_VolumeNode != vol) && !(vol->dol_misc.dol_volume.dol_LockList))
                {   /* Volume Node should be removed -- Only if:
                    - Not the current Volume and
                    - No outstanding locks */
                    DeleteVolNode(vol);
                }

                return(DOSTRUE);
            }
            lk_prev = lk;
            lk = (struct MLock *) BADDR(lk_next);
        }
    }

    DOSerror = ERROR_INVALID_LOCK;
    return(DOSFALSE);
}


/********************
* Parent()  --- Find Parent directory of the specified lock.
*   Return new lock to parent or 0 if already at root dir.
*************/
struct MLock *Parent(lock)
register struct MLock *lock;
{
    register struct MFileInfoBlock *mfib = (struct MFileInfoBlock *)fsys->f_scratch;
    register struct MLock *parentlock=0;

    if(!CompareVolNode(lock)) return(DOSFALSE);

    if( !lock )
    {                 /* lock = 0; already at root of filesystem */
        DOSerror =0;
        return(DOSFALSE);
    }

    mfib->mfib_DiskCluster   = lock->ml_lock_ext.le_dir_clust;
    mfib->mfib_DiskDirEntry  = lock->ml_lock_ext.le_dir_ent;
    mfib->mfib_DirEntStatus  = MLDE_DIR;

    if(FindParent(mfib) )
    {
        parentlock = MakeLock(lock, mfib, SHARED_LOCK);
    }

    return(parentlock);
}


/*************
* Change_Mode()  --- Change the mode of a FileHandle or FileLock
*               Used for ACTION_CHANGE_MODE
*************/
LONG Change_Mode(type, obj, mode)
LONG type;
LONG obj;
LONG mode;
{
    struct MLock *lock;

    if(type == CHANGE_FH)
    {
        lock = (struct MLock *)((struct FileHandle *)obj)->fh_Arg1;
    }
    else    /* type = CHANGE_LOCK */
    {
        lock = (struct MLock *)((struct FileHandle *)obj);
    }
    if(!CompareVolNode(lock)) return(DOSFALSE);

/* At this time only SHARED_LOCK and EXCLUSIVE_LOCK modes are supported */
/* You can always change an EXCLUSIVE_LOCK to a SHARED_LOCK but not visa-versa */
    switch(mode)
    {
    case EXCLUSIVE_LOCK:
        /* Check if any other locks of the same object.  If the circular list points to itself then there is no other of the same lock */
        if(&(lock->ml_lock_ext) != (struct Lock_ext *)(lock->ml_lock_ext.le_samelock.mln_Succ))
        {
            DOSerror = ERROR_OBJECT_IN_USE;
            return(DOSFALSE);   /* return fail because another lock on the same object exists */
        }
        break;

    case SHARED_LOCK:
    default:        /* default to shared lock */
        break;
    }
    lock->ml_lock.fl_Access = mode;

    return(DOSTRUE);
}

