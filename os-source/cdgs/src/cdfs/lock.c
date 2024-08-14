/***********************************************************************
****************                                        ****************
****************        -=   CD FILE SYSTEM   =-        ****************
****************                                        ****************
************************************************************************
***                                                                  ***
***   Written by Carl Sassenrath for Commodore International, Ltd.   ***
*** Copyright (C) 1991 Commodore Electronics Ltd. All rights reserved***                                                                  ***
***                                                                  ***
***         Confidential and Proprietary System Source Code          ***
***                                                                  ***
***********************************************************************/

#include "main.h"

FORWARD APTR *AllocElem();

#define D(x)    ;

/***********************************************************************
***
***  MakeLock
***
*** Make a new lock for the currently mounted volume, and link
*** it to the volume.  Check to see if this lock already exists
*** and whether it has exclusive access (in which case this
*** function fails and returns an error.)
***
***********************************************************************/
CDLOCK *MakeLock(path,key,mode)
    /*
    ** Create a new Lock from ....
    */
    ULONG path;
    ULONG key;
    ULONG mode;
{
    REG CDLOCK *lock;

    /* check if volume mounted */

    /* check if lock exists and is exclusive */

    lock = (CDLOCK *)AllocElem(LockPool);   /* CES 20.7 */
    if (!lock) {PktRes2 = ERROR_NO_FREE_STORE; return 0;}

    lock->Link = NULL;
    lock->Key = key;
    lock->Access = mode;
    lock->Task = FSPort;
    lock->Volume = ATOB(ThisVol);
    lock->PathNum = path;
    lock->ByteSize = 0;

    Forbid();
        lock->Link = ThisVol->dev.dl_LockList;
        ThisVol->dev.dl_LockList = ATOB(lock);
    Permit();

    return lock;
}

/***********************************************************************
***
***  FreeLock
***
***********************************************************************/
FreeLock(lock)
    REG CDLOCK *lock;
{
    REG CDLOCK *lp;
    REG VOLDEV *vol;

    if (!lock) return DOS_TRUE; /* root */

    vol = (VOLDEV *)BTOA(lock->Volume);
    if (!vol)
    {
        PktRes2 = ERROR_INVALID_LOCK;
        return DOS_FALSE;
    }

    lp = (CDLOCK *)BTOA(vol->dev.dl_LockList);
    if (lp == lock) /* at head */
    {
        if ((vol->dev.dl_LockList = lock->Link) == NULL) /* last lock */
        {
            if (vol != ThisVol) /* vol not current */
            {
                DeMount(vol);
            }
        }
    }
    else    /* in list */
    {
        while (lp)
        {
            if (lp->Link == ATOB(lock)) break;
            lp = (CDLOCK *)BTOA(lp->Link);
        }

        if (!lp)
        {
            PktRes2 = ERROR_INVALID_LOCK;
            return DOS_FALSE;
        }

        lp->Link = lock->Link;
    }

    lock->Volume = 0;  /* just in case someone still attempts it */
/*  FreeVector(lock);   */
    FreeElem(LockPool,lock);    /* CES 20.7 */

    return DOS_TRUE;
}

/***********************************************************************
***
***  SourLock
***
*** Check for a bad lock.
*** For the purposes of this function, a NULL lock is not valid.
***
***********************************************************************/
SourLock(lock)
    REG CDLOCK *lock;
{
    VOLDEV *vol;

    if (!lock)
    {
        PktRes2 = ERROR_INVALID_LOCK;
        return DOS_TRUE;
    }

    if (!ThisVol) 
    {
        PktRes2 = ERROR_NO_DISK;
        return DOS_TRUE;        /* no volume mounted */
    }

    if (lock->Volume != ATOB(ThisVol))
    {
        vol = (VOLDEV *)BTOA(lock->Volume);
        PktRes2 = ((vol->dev.dl_Type == DLT_VOLUME) ? 
                ERROR_DEVICE_NOT_MOUNTED :
                ERROR_INVALID_LOCK);
        return DOS_TRUE;
    }

    return DOS_FALSE;
}

/***********************************************************************
***
***  LockFile
***
*** Given a path string, find the associated file or directory
*** and return a lock to it.  This is the main support function
*** for Locating and Opening files.
***
***********************************************************************/
CDLOCK *LockFile(baselock,pathstr,mode)
    CDLOCK *baselock;
    UCHAR *pathstr;
    ULONG mode;
{
    CDLOCK *newlock;
    ULONG basepath;
    ULONG path;
    char name[MAX_BSTR_LEN];
    DIREC *d,*FindFile();

    D(Debug0("\tL%lx.%s\n",baselock,&pathstr[1]));

    if (!ThisVol)   /* CES 20.7 */
    {
        PktRes2 = ERROR_NO_DISK;
        return DOS_FALSE;       /* no volume mounted */
    }

    if (baselock && SourLock(baselock)) return NULL;

    path = (baselock) ? baselock->PathNum : 1;

    /* Scan the path table for the directory part of the path: */
    path = ScanPath(path,pathstr,name);
    D(Debug0("Scanpath(%ld,...,%s) returned %ld\n",
        (baselock) ? baselock->PathNum : 1,path,name));
    if (!path) {PktRes2 = ERROR_OBJECT_NOT_FOUND;return NULL;}

    if (name[0] != 0) /* a file! */
    {
        d = FindFile(PathIndex[path]->Extent,name);
        if (!d) {PktRes2 = ERROR_OBJECT_NOT_FOUND;return NULL;}
        MUST(newlock = MakeLock(path,d->Extent,mode)); /* need d too */
        newlock->ByteSize = d->LenData;
    }
    else /* a dir! */
    {
        newlock = MakeLock(path,PathIndex[path]->Extent,mode);
    }

    return newlock;
}

/***********************************************************************
***
***  IsDirLock
***
***********************************************************************/
IsDirLock(lock)
    CDLOCK *lock;
{
    if (lock == 0 || PathIndex[lock->PathNum]->Extent == lock->Key)
        return TRUE;

    return FALSE;
}

/***********************************************************************
***
***  CopyLock
***
*** Make a copy of a lock and all of its special CDFS fields.
*** Works for locks with volumes that are not currently mounted.
***
***********************************************************************/
CDLOCK *CopyLock(l)
    REG CDLOCK *l;
{
    REG CDLOCK *lp;
    REG CDLOCK *lock;
    REG VOLDEV *vol;

    if (!l) /* CES 21.0 - null lock */
    {
        if (!ThisVol)
        {
            PktRes2 = ERROR_NO_DISK;
            return DOS_FALSE;       /* no volume mounted */
        }
    
        lock = MakeLock(1,RootDirLBN,SHARED_LOCK);
        vol = ThisVol;
    }
    else
    {
        /* !!! Check for execlusive lock? */

        lock = (CDLOCK *)AllocElem(LockPool);   /* CES 20.7 */
        if (!lock) {PktRes2 = ERROR_NO_FREE_STORE; return DOS_FALSE;}

        *lock = *l; /* struct copy */
        vol = (VOLDEV *)BTOA(l->Volume);
    }

    Forbid();
        lock->Link = vol->dev.dl_LockList;
        vol->dev.dl_LockList = ATOB(lock);
    Permit();

    return lock;
}

/***********************************************************************
***
***  MakePool
***
***********************************************************************/
POOL *MakePool(number,size)
    ULONG number;
    ULONG size;
{
    REG POOL *p;
    REG APTR *l;
    REG ULONG i;

    i = sizeof(POOL) + Mult(size,number);

    p = (POOL *)MakeVector(i);

    p->head = l = (APTR *)(p+1);    /* entries start here */
    p->size = size;
    p->limit = (APTR *)((ULONG)p + i);

    /* Link entries together on list */
    for (i = 0; i < number-1; i++)
    {
        *l = (APTR)((ULONG)l + size);
        l = (APTR *)(*l);
    }

    *l = NULL;
    p->tail = l;

    return p;
}

/***********************************************************************
***
***  AllocElem
***
***********************************************************************/
APTR *AllocElem(p)
    REG POOL *p;
{
    REG APTR *n;

    n = p->head;
    if (!n) return MakeVector(p->size);  /* pool is empty, use mem */

    p->head = (APTR *)(*n);
    if (!p->head) p->tail = NULL;

    ClearMem(n,p->size);
    return n;
}

/***********************************************************************
***
***  FreeElem
***
***********************************************************************/
FreeElem(p,n)
    REG POOL *p;
    REG APTR *n;
{
    REG APTR *m;

    if (n > (APTR *)p && n < (APTR *)p->limit)
    {
        m = p->tail;
        if (!m) p->head = n;
        else *m = (APTR)n;
        p->tail = n;
        *n = NULL;
    }
    else FreeVector(n); /* return it to mem */
}
