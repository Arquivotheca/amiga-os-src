/***********************************************************************
****************                                        ****************
****************        -=   CD FILE SYSTEM   =-        ****************
****************                                        ****************
************************************************************************
***                                                                  ***
***   Written by Carl Sassenrath for Commodore International, Ltd.   ***
*** Copyright (C) 1991 Commodore Electronics Ltd. All rights reserved***
***                                                                  ***
***         Confidential and Proprietary System Source Code          ***
***                                                                  ***
***********************************************************************/

#include "main.h"

#define D(x)    ;       /* To enable, set to D(x)       x;      */
#define DD(x)   ;

IMPORT  BOOL    MatchBStr();
IMPORT  DIREC   *GetDirBlock();
IMPORT  LONG    DirCacheFull();

FORWARD DIREC   *FindKey();

/***********************************************************************
***
***  LockToDir
***
***     Get the directory record for a directory lock.
***
***********************************************************************/
DIREC *LockToDir(lock)
        CDLOCK *lock;
{
        PTE *p;
        ULONG pi;

        if (lock == 0 || lock->PathNum == 1)
                return GetDirBlock(RootDirLBN);

        pi = PathIndex[lock->PathNum]->ParentDirNum;
        return FindKey(PathIndex[pi]->Extent,lock->Key);
}

/***********************************************************************
***
***  ScanDirBlock
***
***     Scan a directory block for the specified file name.
***     Return the pointer to the directory entry of interest,
***     else return NULL if name wasn't found in this block.
***
***********************************************************************/
DIREC *ScanDirBlock(dp,limit,name,flag)
        REG DIREC *dp;
        REG DIREC *limit;
        UBYTE *name;    /* file name as BCPL string */
        BOOL flag;
{
        REG int match;
        char ident[MAX_BSTR_LEN];

        for (; limit > dp; dp = NEXTDIR(dp))
        {
                if (dp->LenDirRec == 0) break; /* end of block */

                if (flag) /* looking for name */
                {
                        MakeISOName(ident,&dp->LenFileIdent);
                        match = CmpISOStr(name,ident);
                        if (match == 0) return dp;   /* found the file */
                        if (FastSearch && match < 0) return NULL; /* no such file V22.7 */
                }
                else /* looking for key */
                {
                        if (dp->Extent == (ULONG)name) return dp;
                }
        }

        return (DIREC *)(-1);   /* end of block - CES 21.0*/
}

/***********************************************************************
***
***  FindFile
***
***     Scan a directory looking for the specified file name.
***     Return a pointer to the file's directory entry, or NULL
***     if the file name was not found.
***
***********************************************************************/
DIREC *FindFile(lbn,name)
        ULONG lbn;      /* directory block number */
        UBYTE *name;    /* file name as BCPL string */
{
        REG LONG size;
        REG DIREC *dp;
        DIREC *retval = NULL;
        APTR limit;
        int ignore = FALSE;
        char ident[MAX_BSTR_LEN];

        D(Debug0("In FindFile..."));
        MUST(dp = GetDirBlock(lbn));    /* V24.3 */
        limit = (APTR)((LONG)dp + (LONG)BlockSize);
        size = dp->LenData;
        dp = NEXTDIR(dp);       /* skip dir entry: self   */
        dp = NEXTDIR(dp);       /* skip dir entry: parent */

        MakeISOName(ident,name);
        Debug3("\tF%s\n",ident);        
        D(Debug0("FindFile: name=%s,lbn=%ld,ident=%s,size=%lx\n",name,lbn,ident,size));

        for (; size > 0; size -= BlockSize, lbn++)
        {
                /* if there are empty dir cache blocks, fill them with data */
                if (!ignore)
                {
                        dp = ScanDirBlock(dp,limit,ident,TRUE);
                        D(Debug0("      : size=%lx,lbn=%ld,dp=%lx\n",size,lbn,
                                 dp));
                        if (dp != (DIREC *)-1) 
                        {
                                /* dp is either NULL (didn't find it), or */
                                /* a block pointer.                       */
                                /* Any more space in cache?               */
                                retval = dp;
                                if (DirCacheFull())
                                        break;  /* no space */
                                ignore = TRUE;  /* just read blocks */
                        }
                }

                MUST(dp = GetDirBlock(lbn+1)); /* V24.3 */
                limit = (APTR) ((CPTR)dp + BlockSize);
        }

        return retval;
}

/***********************************************************************
***
***  FindKey
***
***     Scan a directory looking for the specified file key.
***     Return a pointer to the file's directory entry, or NULL
***     if the file name was not found.
***
***********************************************************************/
DIREC *FindKey(lbn,key)
        ULONG lbn;      /* directory block number */
        ULONG key;
{
        REG LONG size;
        REG DIREC *dp;
        DIREC *retval = NULL;
        int ignore = FALSE;
        APTR limit;

        MUST(dp = GetDirBlock(lbn));
        limit = (APTR)((LONG)dp + (LONG)BlockSize);
        size = dp->LenData;
        dp = NEXTDIR(dp);       /* skip dir entry: self   */
        dp = NEXTDIR(dp);       /* skip dir entry: parent */


        for (; size > 0; size -= BlockSize, lbn++)
        {
                /* if there are empty dir cache blocks, fill them with data */
                if (!ignore)
                {
                        dp = ScanDirBlock(dp,limit,key,FALSE);
                        D(Debug0("FindKey : size=%lx,lbn=%ld,dp=%lx,key=%lx\n",
                                 size,lbn,dp,key));
                        if (dp != (DIREC *)-1)
                        {
                                /* dp is either NULL (didn't find it), or */
                                /* a block pointer.                       */
                                /* Any more space in cache?               */
                                retval = dp;
                                if (DirCacheFull())
                                        break;  /* no space */
                                ignore = TRUE;  /* just read blocks */
                        }
                }
		/* if we're just prefetching dir blocks, we don't care about
		   errors */
		if (!ignore) {
	                MUST(dp = GetDirBlock(lbn+1));
		} else
			dp = GetDirBlock(lbn+1);

                limit = (APTR) ((CPTR)dp + BlockSize);
        }

        return retval;
}

/***********************************************************************
***
***  NextFile
***
***     Find the next file in a directory and return a pointer to
***     its entry or NULL if the file was not found.
***
***     This routine optimizes the search by saving the offset of
***     the last directory entry in the lock.  When the next file
***     is requested, the entry at this previous offset has its
***     name compared the name passed, and if correct, the next
***     entry is returned. 
***
***     We won't bother to check DirCacheFull() here, since ExNext
***     usually goes to the end anyways.
***********************************************************************/
DIREC *NextFile(lock,name)
        REG CDLOCK *lock;
        UBYTE *name;                    /* file name as BCPL string */
{
        REG LONG dirblk,dirlen,bias,limit;
        REG DIREC *np;
        CHAR ident1[MAX_BSTR_LEN];
        CHAR ident2[MAX_BSTR_LEN];

        MakeISOName(ident1,name);

        bias   = lock->DirOffset;       /* bytes from start of file */
        limit  = (bias + BlockSize) & ~BlockMask; /* bias limit in this block */
        dirlen = lock->ByteSize;        /* size of dir in bytes */
        dirblk = PathIndex[lock->PathNum]->Extent + (bias >> BlockShift);


        /* Get the correct directory block and offset within it: */
        D(Debug0("In NextFile, lbn $%lx...",dirblk));
        MUST(np = GetDirBlock(dirblk));
        D(Debug0("got block at $%lx\n",np));
        np = (DIREC *)((LONG)np + (bias & BlockMask));

        /* Verify Lock's cached entries (ByteSize & DirOffset): */
        MUST(np->LenDirRec);
        MakeISOName(ident2,&np->LenFileIdent);
        if (CmpISOStr(ident1,ident2) == 0) /* it is valid */
        {
                lock->DirOffset = 0;

                bias += np->LenDirRec;  /* advance bias to next entry */
                MUST(bias < dirlen);    /* check for end of dir */
                if (bias < limit)       /* not past end of block */
                {
                        np = NEXTDIR(np);  /* get next entry */
                        if (np->LenDirRec) /* it is real (typical) */
                        {
                                lock->DirOffset = bias;
                                return np;
                        }
                        /* Empty entry, skip to next block or... */
                        MUST(limit < dirlen) /* end of dir */
                }

                /* Dir wrapped into next block... */
	        D(Debug0("wrapping to lbn $%lx...",dirblk+1));
                MUST(np = GetDirBlock(++dirblk));
	        D(Debug0("wrapped\n"));

                lock->DirOffset = limit;
                return np;
        }

        /* The lock is not valid for this dir entry! Someone has */
        /* trashed it and is using it with some old FIB. Yuk.    */

        Debug2("\tNV\n");
        return 0;       
}

/***********************************************************************
***
***  Examine
***
***********************************************************************/
Examine(lock,fib)
        REG CDLOCK *lock;
        FIB *fib;
{
        REG DIREC *d;
        REG PTE *p;
        PTE *n = NULL;
        /*      char ident[MAX_BSTR_LEN];       V24.4 removed */

        if (!lock) {return SetVolFileInfo(fib);} /* CES 21.0 */

        p = PathIndex[lock->PathNum];

        /* Is it a DIR or a FILE?  Handle each. */
        if (IsDirLock(lock))    /* CES 20.5 copy name from PT */
        {
                d = LockToDir(lock);
                n = p;
        }
        else d = FindKey(p->Extent,lock->Key);

        if (!d)	return DOS_FALSE;

	return SetFileInfo(fib,d,n);
}

/***********************************************************************
***
***  ExamineNext
***
***********************************************************************/
ExamineNext(lock,fib)
        REG CDLOCK *lock;
        REG FIB *fib;
{
        REG DIREC *d,*dd;

	/* default error */
        PktRes2 = ERROR_NO_MORE_ENTRIES; 

        /* Is this the first ExamineNext done on this directory? */
        if (fib->fib_DiskKey == lock->Key)
        {
                MUST(d = GetDirBlock(lock->Key));
		dd = d;
                lock->ByteSize = d->LenData;    /* size of dir */               
                d = NEXTDIR(d);                 /* skip first entry */
                d = NEXTDIR(d);                 /* skip second entry */
                if (d->LenDirRec == 0) d = 0;   /* empty directory */
                else lock->DirOffset = (LONG)d - (LONG)dd; /* starting point */
        }
        else /* Next entry in the directory */
        {
                d = NextFile(lock,fib->fib_FileName);
        }

        if (!d) return DOS_FALSE;

        return SetFileInfo(fib,d,NULL);
}

/***********************************************************************
***
***  SetFileInfo
***
***     Get info about a file or a directory entry.
***
***********************************************************************/
SetFileInfo(fib,dr,n)
        REG FIB *fib;
        REG DIREC *dr;
        REG PTE *n;     /* PTE to get name when Examine of directory */
{
        fib->fib_DiskKey        = dr->Extent;
        fib->fib_DirEntryType   =
        fib->fib_EntryType      = (dr->FileFlags) ? 2 : -3; /* dir/file */
        fib->fib_FileName[0]    = 0;
        fib->fib_Protection     = 0;
        fib->fib_Comment[0]     = 0;
        fib->fib_Size           = dr->LenData;
        fib->fib_NumBlocks      = (dr->LenData+BlockMask)>>BlockShift;

        MakeDate(&fib->fib_Date,&dr->RecTime[0]);

        if (n)  /* directory examine? */
        {
                if (dr->Extent == RootDirLBN) /* root dir: fake a name */
                {
                        if (ThisVol)    /* V24.4 - just in case */
                                CopyMem(ThisVol->name,fib->fib_FileName,ThisVol->name[0]+1);
                        fib->fib_Protection = 0x9ff7; /* CES 21.0 - from FFS */
                }
                else
                {
                        fib->fib_FileName[0] = n->LenDirIdent;
                        CopyMem(n->DirIdent,&fib->fib_FileName[1],n->LenDirIdent);
                }
        }
        else MakeFileName(fib->fib_FileName,&dr->LenFileIdent);

        Debug3("\tI%s\n",&fib->fib_FileName[1]);

	return DOS_TRUE;
}

/***********************************************************************
***
***  SetVolFileInfo
***
***     When using Examine to look at a volume (which is legal so
***     it seems), this function is used instead of SetFileInfo().
***
***********************************************************************/
SetVolFileInfo(fib)
        REG FIB *fib;
{
        REG DIREC *dr;

        MUST(dr = GetDirBlock(RootDirLBN));

        fib->fib_DiskKey        = RootDirLBN;
        fib->fib_DirEntryType   =
        fib->fib_EntryType      = 2;
        fib->fib_FileName[0]    = 0;
        fib->fib_Protection     = 0x9ff7; /* CES 21.0 - from FFS */
        fib->fib_Comment[0]     = 0;
        fib->fib_Size           = dr->LenData;
        fib->fib_NumBlocks      = (dr->LenData+BlockMask)>>BlockShift;

        MakeDate(&fib->fib_Date,&dr->RecTime[0]);

        if (ThisVol)    /* V24.4 added - just in case */
                CopyMem(ThisVol->name,fib->fib_FileName,ThisVol->name[0]+1);

	return DOS_TRUE;
}
