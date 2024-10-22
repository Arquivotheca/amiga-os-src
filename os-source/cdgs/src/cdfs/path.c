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

FORWARD struct  PathTableEntry *CalcPathEntry();
FORWARD struct  PathTableEntry *CalcPathIndex();
FORWARD struct  PathTableEntry *FindPathKey();

#asm
    include 'stddefs.i'

    public  _PathTable
    public  _PathIndex
    public  _PathTableSize

** Path Table Entry:
PTE_DIRIDENTLEN     equ 0
PTE_DIRXARLEN       equ 1
PTE_EXTENT      equ 2
PTE_PARENTDIRNUM    equ 6
PTE_DIRIDENT        equ 8

** Path Pair
PP_PINDEX       equ 0
PP_PENTRY       equ 4

NEXTPATH    MACRO
        move.b  (\1),\2
        addq    #1,\2
        and.w   #$00FE,\2
        lea PTE_DIRIDENT(\1,\2.w),\1
        ENDM

    public  _CountPaths
_CountPaths:
        clear   d0
        move.l  _PathTable,a0
        move.l  a0,a1
        add.l   _PathTableSize,a1
        bra.s   20$

10$:        NEXTPATH a0,d1
        addq.l  #1,d0
20$:        cmp.l   a0,a1       end of path table?
        bhi.s   10$

        rts

    public  _IndexPaths
_IndexPaths:
        move.l  _PathIndex,a1
        move.l  _PathTable,a0
        move.l  a0,d1
        add.l   _PathTableSize,d1
        clr.l   (a1)+       clear index 0
        bra.s   20$

10$:        move.l  a0,(a1)+
        NEXTPATH a0,d0
20$:        cmp.l   a0,d1       end of path table?
        bhi.s   10$

        clr.l   (a1)+       clear index 0
        rts


** ULONG CalcPathEntry(pp)
**
**  Find a path entry from its path number.  Means
**  counting entries until the right one is found.
**  Return offset, NULL on error.
**  Updates PP.
**  Path table is ONE based array (not zero based).
    public  _CalcPathEntry
_CalcPathEntry:
        move.l  4(sp),a0    path pair
        move.l  (a0),d1     path index
        subq.l  #1,d1       make zero based
        move.l  _PathTable,a0
        move.l  a0,a1
        add.l   _PathTableSize,a1
        bra.s   20$

10$     NEXTPATH a0,d0
20$     cmp.l   a0,a1       end of path table?
        bls.s   40$
        dbf d1,10$      decrement counter

        move.l  a0,d0       return entry pointer
        move.l  4(sp),a0
        move.l  d0,4(a0)
30$     rts

40$     moveq   #0,d0       return error: not found
        bra.s   30$


** ULONG CalcPathIndex(pp)
**
**  Find a path Index from its path Entry.  Means
**  counting entries until the right one is found.
**  Return offset, NULL on error.
**  Updates PP.
**  Path table is ONE based array (not zero based).
**  Does not check for invalid entry address.
    public  _CalcPathIndex
_CalcPathIndex:
        move.l  4(sp),a0    path pair
        move.l  4(a0),a1    path entry
        moveq   #1,d1
        move.l  _PathTable,a0
        bra.s   20$

10$     NEXTPATH a0,d0
20$     cmp.l   a0,a1       end of path table?
        bls.s   40$
        addq.l  #1,d1
        bra.s   10$

40$     move.l  4(sp),a0
        move.l  d1,(a0)
        move.l  d1,d0
        rts


** ULONG FindPathKey(key,pp)
**
**  Scan the path table looking for an entry with the
**  specified key (extent LBN).
**
    public  _FindPathKey
_FindPathKey:
        move.l  d2,-(sp)
        move.l  8(sp),d1    dir record number
        moveq.l #0,d2
        move.l  _PathTable,a0
        move.l  a0,a1
        add.l   _PathTableSize,a1
        bra.s   20$

10$     NEXTPATH a0,d0
20$     cmp.l   a0,a1       end of path table?
        bls.s   40$
        addq.l  #1,d2
        cmp.l   PTE_EXTENT(a0),d1
        bne.s   10$

        move.l  a0,d0       return entry pointer
        move.l  12(sp),a0
        move.l  d2,(a0)
        move.l  d0,4(a0)
30$     move.l  (sp)+,d2
        rts

40$     moveq   #0,d0       return error: not found
        bra.s   30$

** ULONG FindPathName(pi,fileName,len)
**
**  Starting with the current path directory entry,
**  scan for a dir name.  If found, return the offset
**  of the entry in the path table.  If not found,
**  return NULL.
**  Does not shift case.... must already be correct.
**
    public  _FindPathName
_FindPathName:
        movem.l d2/a2-a4,-(sp)
        move.l  20(sp),d2   parent path index
        move.l  _PathIndex,a0
        move.l  d2,d0
        lsl.l   #2,d0
        move.l  0(a0,d0),a0 parent path entry
        move.l  24(sp),a2   file name
        move.l  _PathTable,a1
        add.l   _PathTableSize,a1


    ;------ Prepare a 32 bit word containing parent dir num
    ;   and first two chars of name.  Use this for quick
    ;   compare to home in on the target entry.
    ;   (This works because a single char entry must be
    ;   padded with a null byte, which is the C terminator
        move.l  d2,d1
        swap    d1
        move.b  (a2)+,d1
        lsl.w   #8,d1
        move.b  (a2)+,d1

10$     NEXTPATH a0,d0
        addq.l  #1,d2
20$     cmp.l   a0,a1       end of path table?
        bls.s   40$
        cmp.l   PTE_PARENTDIRNUM(a0),d1
        bhi.s   10$
        blo.s   40$

    ;------ Now compare the rest of the characters.   !!!!! check!!!
        moveq   #0,d0
        move.b  PTE_DIRIDENTLEN(a0),d0
        cmp.l   28(sp),d0   ; CES 091390
        bne.s   10$     ; CES 091390
        subq.l  #3,d0
        blt.s   50$
        lea PTE_DIRIDENT+2(a0),a3
        move.l  a2,a4
44$     cmp.b   (a4)+,(a3)+
        dbne    d0,44$
        bne.s   10$
        tst.b   (a4)        check for end of string
        bne.s   10$

    ;------ got it!
50$     move.l  d2,d0
60$     movem.l (sp)+,d2/a2-a4
        rts
40$     moveq   #0,d0
        bra.s   60$

#endasm

/***********************************************************************
***
***  ScanPath
***
*** Walk down a directory path relative to a lock
*** and return the path index of the final directory
*** along with the actual file name.
***
*** If the lock is zero the scan starts from the root.
*** If the path is simply a lone ':', the root is
*** returned.  If a "dev:" is specified, it is ignored
*** (because DOS has already dealt with it).
***
*** The path is scanned for slashes to delimit each
*** directory.  If a path string contains multiple
*** adjacent slashes (//) the scan backs up the correct
*** number of directories (parents).  This also happens
*** if the path begins with a slash.
***
*** An actual file name need not be specified.  That is,
*** a path may end with a slash or a dir name.
*** In such a case the filename returned is empty.
***
*** The CD ROM Path Tables are used extensively for
*** this function.  We don't even begin to look at
*** directory blocks for this code.
***
***********************************************************************/
ScanPath(path,pathStr,fileName)
    REG ULONG   path;
    UCHAR   *pathStr;
    REG UCHAR   *fileName; /* BSTR returned for files */
    /* NOTE: all args must be provided (buffers too!); */
{
    ULONG newpath;
    REG int len;
    REG int i;
    REG UCHAR *p;
    REG UCHAR *s;

    Debug3("\tP%lx.%s\n",path,&pathStr[1]);

    p = pathStr;
    len = (int)(*p++);
    fileName[0] = 0;    /* if stays zero, then directory */

    /* Check for lone ':', return root key
    if (len == 1 && *p == ':') return 1;  /* root path num */

    /* Scan, remove, and ignore any "dev:" stuff */
    for (i = len, p += len-1; i > 0; i--, p--)
    {
        if (*p == ':') {len -= i; break;}
    }
    p++;

    /* For each dir entry, scan down the Path Table */
    for (; len > 0; p++, len--)
    {
        /* Save name (ignore > 30 chars) */
        s = fileName+1;
        for (i = 0; len > 0 && *p != '/'; len--, p++)
        {
            if (i < MAX_NAME_LEN)
            {
                *s++ = CharSet[*p];
                i++;
            }
        }
        *s = '\0';
        fileName[0] = i;
/*      Debug4("ScanPath: filename='%s' len=%ld\n",fileName+1,i);*/

        if (i == 0 && *p == '/') /* get parent */
        {
            if (path == 1)  /* past top, so bad path */
                { fileName[0] = 0; return 0; }
            newpath = PathIndex[path]->ParentDirNum;
        }
        else    /* Determine what the name is */
        {
            /* If in path table, must be a dir, else... */
            newpath = FindPathName(path,fileName+1,*fileName);
/*          Debug4("Look on path: %ld, Newpath = %ld\n",path,newpath);*/
            if (!newpath) /* was not found, must be file (or bad)*/
            {
                if (len <= 0) return path; /* a file */
/*              Debug4("Not found");*/
                fileName[0] = 0;
                return 0;   /* bad path */
            }
        }

        path = newpath;
    }

    fileName[0] = 0;
    return path; /* a dir */
}


