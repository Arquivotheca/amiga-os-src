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

IMPORT  CDLOCK *LockFile(),*CopyLock(),*MakeLock();

ULONG A_CurrentVol()    /* TYPE 7, ARGS: LOCK */
    /*
    ** Return a BPTR to the current volume's DOS Dev List pointer.
    ** If a lock is given, return the Dev List for that lock.
    */
{
    REG CDLOCK *lock = (CDLOCK *)PktArg1;

    PktRes2 = UnitNumber;

    if (!lock) return ATOB(ThisVol);
    else return lock->Volume;
}

ULONG A_LocateObj() /* TYPE 8, ARGS: LOCK, PATH, MODE */
    /*
    ** Locate and lock a disk object: either a directory or a file.
    ** The object is located relative to a current lock (or null)
    ** for the current volume.  The new file lock may be shared or
    ** exclusive.
    **
    ** The memory used for a lock is controlled entirely by the
    ** handler.  The CD file system returns locks that contain
    ** additional fields that are not described by DOS structures.
    ** These fields hole the path pair for accessing into the
    ** volume's directory path table. Unlike most AmigaDOS
    ** handlers, the KEY is not enough to track a particular
    ** file, the path pair is also required.
    **
    ** This action code was implemented to locate an object with
    ** zero or one disk seeks.  A directory object can be found
    ** with no disk seeks, a file will require one seek.
    **
    */
{
    REG CDLOCK *lock;

    lock = LockFile(PktArg1,PktArg2,PktArg3);

    return ATOB(lock);
}


ULONG A_FreeLock()  /* TYPE 15, ARGS: LOCK */
    /*
    ** Free a lock.  Release its memory back to the system.
    ** A root lock of zero works, but does not free anything.
    **
    ** The lock must be found in the volume's lock list and
    ** unlinked before it can be freed.
    */
{
    return FreeLock(PktArg1);
}

ULONG A_MoreCache() /* TYPE 18, ARGS: DIFFERENCE */
    /*
    ** Add or remove cache buffers to the file system.  A postive
    ** integer arg adds, a negative removes, and zero remains
    ** unchanged.  The number of cache blocks used is returned
    ** in all cases.
    **
    ** The number of blocks used may have a great impact on the
    ** performance of the CDFS due to the slow seek time of a CD.
    ** The CDFS uses information in the Primary Volume Desc. to
    ** indicate the initial number of blocks, so a developer can
    ** set the system up has needed from the start.
    */
{
    return MoreCache(PktArg1);
}

ULONG A_CopyDir()   /* TYPE 19, ARGS: LOCK */ 
    /*
    ** Copy a lock.  The new lock will represent the same
    ** object.  The new lock is freed in the same fashion
    ** as any other lock.
    **
    ** If a zero lock is passed in, the resulting lock
    ** will be to the volume's root directory.
    */
{
    REG CDLOCK *lock;

    lock = CopyLock(PktArg1);

    return ATOB(lock);
}

ULONG A_ExamineObj()    /* TYPE 23, ARGS: LOCK, FIB */
    /*
    ** Get information about a locked object.  This action fills
    ** in a file info block with all the relevant detail.  To do
    ** so, the directory which references the object is accessed,
    ** possibly causing a disk read to occur.
    **
    */
{
    if (PktArg1 && SourLock(PktArg1)) return DOS_FALSE;

    return Examine(PktArg1,PktArg2);
}

ULONG A_ExamineNext()   /* TYPE 24, ARGS: LOCK, FIB */
    /*
    ** Examine the next object in a directory.  This next object
    ** is found relative to the lock argument and the object
    ** referenced by the DiskKey and Name fields of the file
    ** info block (FIB) passed to this routine. 
    **
    ** Because CD is read only, we need not worry about someone
    ** deleting a file between examines.
    */
{
    if (SourLock(PktArg1) || !ExamineNext(PktArg1,PktArg2)) /* CES 21.0 */
        return DOS_FALSE;

    return DOS_TRUE;
}

ULONG A_DiskInfo()  /* TYPE 25, ARGS: INFO */
    /*
    ** Return disk volume information.
    */
{
    SetVolInfo(PktArg1);

    return DOS_TRUE;
}

ULONG A_Info()      /* TYPE 26, ARGS: LOCK INFO */
    /*
    ** This is similar to the DiskInfo action but checks
    ** the lock first to see if the volume is present
    ** and valid.  If not, an error is returned.
    ** A NULL lock is valid.
    */
{
    CDLOCK *lock = (CDLOCK *)PktArg1;

    if (lock && SourLock(lock)) return DOS_FALSE;
    SetVolInfo(PktArg2);

    return DOS_TRUE;
}

ULONG A_Flush()     /* TYPE 27, NOARGS */
    /*
    ** Does nothing for CD ROM.  (It is read only.).
    */
{
    return DOS_TRUE;
}

ULONG A_Parent()    /* TYPE 29, ARGS: LOCK */
    /*
    ** Return a lock on a parent directory.  This works for
    ** both locks on files and directories.
    **
    ** This function depends entirely on the Path Pair fields
    ** of the CD Lock structure (an extension to standard DOS
    ** lock).
    **
    */
{
    REG CDLOCK *lock = (CDLOCK *) PktArg1;
    REG LONG i;

    if (!lock || SourLock(lock)) return DOS_FALSE;

    if (IsDirLock(lock))   /* A directory */
    {
        if (lock->PathNum <= 1) return NULL;  /* no parent */

        i = PathIndex[lock->PathNum]->ParentDirNum;
    }
    else
    {
        i = lock->PathNum;
    }

    lock = MakeLock(i,PathIndex[i]->Extent,SHARED_LOCK);
    return ATOB(lock);
}

ULONG A_Inhibit()   /* TYPE 31, ARGS: FLAG   V34.2 Added */
    /*
    ** Stop normal file system operation.  Basically, turn off
    ** access to the disk.  Since it is read only, we don't need
    ** to worry about write access or flushing buffers.
    **
    */
{
    if (PktArg1)
    {
        if (InhibitCount++ == 0) UnMount();
    }
    else
    {
        if (--InhibitCount <= 0)
        {
            InhibitCount = 0;
            ReMount();
        }
    }

    return DOS_TRUE;
}


ULONG A_Read()      /* TYPE 'R', ARGS: CDFH BUF LEN */
    /*
    ** Read data from a file into a buffer.  Return the number of
    ** bytes read, or zero for EOF, and -1 for an error.
    **
    ** To identify the file, a CD file handle is passed back
    ** from the file open operation.  This handle is allocated
    ** by us and contains the file state information, including
    ** its current read position.
    */
{
    return ReadFile(PktArg1,PktArg2,PktArg3);
}

ULONG A_OpenFile()  /* TYPE 1005,1004,1006, ARGS: FHDL LOCK NAME */
    /*
    ** Open a file for reading.  It fills in a user's file handle
    ** structure and creates its own CD file handle for file
    ** management.
    **
    ** The file is found in a fashion identical to A_Locate (above).
    */
{
    return OpenFile(PktArg1,PktArg2,PktArg3);
}

ULONG A_End()       /* TYPE 1007, ARGS: CDFH */
    /*
    ** Close a file.  Free memory used for internal file
    ** handle and release lock used for this.
    */
{
    return CloseFile(PktArg1);
}

ULONG A_Seek()      /* TYPE 1008, ARGS: CDFH POS MODE */
    /*
    ** Change the read position in a file.  The seek is
    ** performed depending on the setting of the mode arg
    ** from the beginning, current, or end positions.
    ** Seeking past the end of file sets the position to
    ** the end of the file.
    */
{
    return SeekFile(PktArg1,PktArg2,PktArg3);
}

ULONG A_WriteProtect()  /* TYPE 1023, ARGS: PROT PASS */
    /*
    ** Used to write protect volume partitions.  Since the
    ** CD is always write protected, we will just ignore this
    ** action and return TRUE;
    */
{
    return DOS_TRUE;
}

ULONG A_FHFromLock()    /* TYPE 1026, ARGS: FH, lock */
    /*
    ** Open a file from a given lock.  The lock is given up if this
    ** succeeds.  See A_OpenFile().
    */
{
	return OpenFileFromLock(PktArg1,PktArg2);
}

ULONG A_IsFilesystem()    /* TYPE 1027, ARGS: */
    /*
    ** Return TRUE.
    */
{
	return DOSTRUE;
}

ULONG A_DupLockFH()   /* TYPE 1030, ARGS: FH_arg1 */ 
    /*
    ** Copy a lock from a filehandle.  The new lock will represent the same
    ** object.  The new lock is freed in the same fashion
    ** as any other lock.
    **
    */
{
    REG CDLOCK *lock;
    REG FCNTRL *fc = (FCNTRL *) PktArg1;

    lock = CopyLock(fc->Lock);

    return ATOB(lock);
}

ULONG A_ParentOfFH()    /* TYPE 1031, ARGS: FH_arg1 */
    /*
    ** Return a lock on a parent directory of an open file.
    **
    ** This function depends entirely on the Path Pair fields
    ** of the CD Lock structure (an extension to standard DOS
    ** lock).
    **
    */
{
    REG CDLOCK *lock = (CDLOCK *) ((FCNTRL *) PktArg1)->Lock;
    REG LONG i;

    if (!lock || SourLock(lock)) return DOS_FALSE;

    /* it can't be a directory */
    i = lock->PathNum;

    lock = MakeLock(i,PathIndex[i]->Extent,SHARED_LOCK);
    return ATOB(lock);
}

ULONG A_ExamineFH()    /* TYPE 1034, ARGS: FH_arg1, FIB */
    /*
    ** Get information about a file.  This action fills
    ** in a fileinfoblock with all the relevant detail.  To do
    ** so, the directory which references the object is accessed,
    ** possibly causing a disk read to occur.
    **
    */
{
    CDLOCK *lock = ((FCNTRL *) PktArg1)->Lock;

    if (lock && SourLock(lock)) return DOS_FALSE;

    return Examine(lock,PktArg2);
}

ULONG A_DirectRead()    /* TYPE 1900, ARGS: CDFH FLAG   V24.1*/
    /*
    ** Turn on/off direct file read capability.
    */
{
    REG FCNTRL *fc = (FCNTRL *)PktArg1;

    if (PktArg2)
        fc->Flags |= FC_DIRECTREAD;
    else
        fc->Flags &= ~FC_DIRECTREAD;

    return DOS_TRUE;
}


ULONG BadWrite(s)
    char *s;
{
    Debug2("\tW%s\n",s);
    PktRes2 = ERROR_DISK_WRITE_PROTECTED;
    return DOS_FALSE;
}


ULONG A_FileDelError()
{
    char *s = (char *)PktArg2;

    return BadWrite(&s[1]);
}


ULONG NoWrite()
    /*
    ** This is the response to any packet Action that would
    ** cause a write to the disk.
    */
{
    return BadWrite("?");
}


ULONG BadPkt()
    /*
    ** There are a few packets that we cannot respond to.
    ** So, return an error and hope for the best.
    */
{
    PktRes2 = ERROR_ACTION_NOT_KNOWN;
    return DOS_FALSE;
}

