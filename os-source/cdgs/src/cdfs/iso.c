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

IMPORT  void    *ReadBlock();

#define D(x)    ;
#define DD(x)   ;

 
/***********************************************************************
***
***  FindVol
***
***********************************************************************/
VOLDEV *FindVol(name,date)
    char    *name;
    struct  DateStamp *date;
{
    REG VOLDEV *vol;

    /* Check if volume node already present */
    Forbid();
        vol  = (VOLDEV *) BTOA(DOSInfo->di_DevInfo);
        for (; vol; vol = (VOLDEV *)BTOA(vol->dev.dl_Next))
        {
/*          Debug2("Check volume: %s\n",BTOA(vol->dev.dl_Name));    */
            if (vol->dev.dl_Type != DLT_VOLUME) continue;
            if (!MatchBStr(name,BTOA(vol->dev.dl_Name))) continue;
            if (!CompDate(&vol->dev.dl_VolumeDate,date)) continue;
            break;
        }
    Permit();

/*  if (vol) Debug2("vol found %lx %s\n",vol,&name[1]);*/
/*  else Debug2("new vol\n");*/
    return vol;
}

/***********************************************************************
***
***  IsValidVolDesc
***
***********************************************************************/
IsValidVolDesc(p)
    struct PrimVolDescriptor *p;
{
    if (!MatchStr(p->StdIdent,STD_IDENT,sizeof(p->StdIdent)))
        return FALSE;

    if (p->VolDescVers != 1) return FALSE;

    return TRUE;
}

/***********************************************************************
***
***  FindPVD
***
*** Find a valid PVD, picking a valid Amiga PVD first over just a
*** random PVD.  The volume descriptors are read into a large
*** temporary buffer to be scanned.  If no Amiga PVD can be found
*** then the first valid PVD is used.
***
***********************************************************************/
PVD *FindPVD()
{
    LONG first = 0;
    LONG size;
    REG LONG lsn,b,i;
    REG PVD *p;
    REG UBYTE *temp,*block;

    /* Get a temporary buffer to search for PVD: */
    size = 16;
    if (!(block = MakeVector(SECT_SIZE)))
        return NULL;

    temp = MakeVector(SECT_SIZE<<4);
    if (!temp) /* Try again, use smaller size */
    {
        size = 2;
        temp = MakeVector(SECT_SIZE<<1);
        if (!temp) {
            FreeVector(block);
            return NULL;
        }
    }

    /* Search for PVD in volume descriptor area: */
    for (lsn = 16; lsn < 256; lsn += size)
    {
        Debug3("\tp%lx.%lx\n",lsn,size);
        D(Debug0("PVD Srch: %lx.%lx\n",lsn,size));

        if (ReadCD(temp,lsn,size)) break; /* had an error */

        /* Examine each sector: */
        for (i = 0, b = 0; i < size; i++, b+=SECT_SIZE)
        {
            p = (PVD *)(&temp[b]);

            if (!IsValidVolDesc(p)) continue;

            switch (p->VolDescType)
            {
            case BOOT_VOL_DESC:
                if (BootRecLSN) break;
                if (MatchStr(((BVD*)p)->BootSysIdent,SYS_IDENT,sizeof(((BVD*)p)->BootSysIdent)))
                {
                    BootRecLSN = lsn + i;
                }
                break;

            case PRIM_VOL_DESC:
                /* Save first, in case no valid Amiga PVD */
                if (!first) {
                    first = lsn;
                    CopyMem(p,block,SECT_SIZE);
                }

                /* Is it an Amiga PVD? */
                if (MatchStr(p->SysIdent,SYS_IDENT,sizeof(p->SysIdent)) ||
                    MatchStr(p->SysIdent,SYS_IDENT2,sizeof(p->SysIdent)))
                {
                    BootPVDLSN = lsn + i;  /* Only set for Amiga */
                    /* if we didn't just copy it, do now */
                    if (first != lsn)
                        CopyMem(p,block,SECT_SIZE);
                    FreeVector(temp);
                    return (PVD *)block;
                }
                break;

            case TERM_VOL_DESC:
                lsn = 256;  /* stop the loop */
                i = size;   /* stop the loop */
                break;

            default:
                break;
            }   
        }
    }

    FreeVector(temp);

    Debug1("\tNC\n");
    if (first)
        return (PVD *)block;

    Debug1("\tNI\n");
    FreeVector(block);
    return NULL;
}


/***********************************************************************
***
***  MakePathTable
***
***********************************************************************/
MakePathTable(pvd)
    REG PVD *pvd;
{
    UBYTE *p;
    ULONG size;

    PathTableSize = pvd->PathTableSize;
    PathTable = MakeVector(PathTableSize+16);

    D(Debug0("Path/Make PathTable.  size=%ld\n",PathTableSize));

    /* copy path table 1 block at a time - a bit ugly, but this stops us from
       having to do non-blocksize IO's */
    for (size = 0;
	 size < PathTableSize;
	 size += BlockSize)
    {
	    if (!(p = ReadBlock(pvd->MPathTable[0] + (size>>BlockShift),-1,
				FALSE,FALSE)))
	    {
        	FreeVector(PathTable);
	        return FALSE;
	    }
	    /* make sure we don't copy past the end */
	    CopyMem(p,((char *)PathTable)+size,
		    MIN(PathTableSize-size,BlockSize));
    }

    MaxPaths = CountPaths();
    PathIndex = MakeVector((MaxPaths+2)<<2);
    IndexPaths();

    return TRUE;
}

/***********************************************************************
***
***  SetOptions
***
*** Scan the PVD looking for file system options.
***
*** The primary volume descriptor must still be in scratch memory
*** for this function to work.
***
***********************************************************************/
#define OC(a,b) (((a)<<8)+(b))
SetOptions(pvd)
    PVD *pvd;
{
    UWORD *op;
    UWORD code,size;

    op = (UWORD *)&pvd->ApplicationUse[1];  /* 650 bytes */

    while ((code = *op++))
    {
        size = *op++;

        Debug2("\tZ%lx.%lx\n",code,size);

        switch(code)
        {
        case OC('B','F'):   /* boot file name */
            CopyStr(BootFile,op,32);
            break;

        case OC('C','D'):   /* dir cache size in blocks */
            DirCacheSize = *op;
            break;

        case OC('C','R'):   /* read cache size in blocks    */
            CacheSize = *op;
            break;

        case OC('D','R'):   /* direct file reading  */
            DirectRead = TRUE; /* V23.3 */
            break;

        case OC('F','S'):   /* fast directory searches */
            FastSearch = TRUE;
            break;

        case OC('N','P'):   /* No prefs flag */
            NoPrefs = TRUE;
            break;

        case OC('P','L'):   /* lock pool */
            LockPoolSize = *op;
            break;

        case OC('P','F'):   /* file pool */
            FilePoolSize = *op;
            break;

        case OC('R','C'):   /* read retry count */
            Retry = *op;
            break;

        case OC('T','M'):   /* trademark info   */
            TMInfo = *((struct TMStruct *)op);
            break;

        case OC('D','S'):   /* Double-speed OK (data disk) */
            DoubleSpeedOK = TRUE;
            break;

        default:
            break;
        }

        op = (UWORD *)((LONG)op + (LONG)size);
    }
}

/***********************************************************************
***
***  Mount
***
***********************************************************************/
Mount(boot)
    BOOL boot;      /* TRUE when booting from this */
{
    REG VOLDEV *vol;
    REG PVD *pvd;
    char    date[8];
    struct  DateStamp stamp;
    char    name[MAX_BSTR_LEN];

    D(Debug0("Mount : boot=%ld\n",boot));

    /* Can we find a valid PVD on the disk? */
    if (!PVDBuffer)
    {
        /* it hasn't been read before */
        if (!(pvd = FindPVD())) return NULL;
        if (boot) SetOptions(pvd);
    } else {
        pvd = PVDBuffer;
        PVDBuffer = NULL;   /* we're going to free it */
    }

    /* Convert volume related info: */
    DStrToBStr(name,pvd->VolIdent,MAX_BSTR_LEN-2);
    ConvertDate(date,&pvd->VolCreateTime);
    MakeDate(&stamp,&date[0]);

    /* Find existing DOS Volume */
    vol = FindVol(name,&stamp);

    /* If volume doesn't exist, create it: */
    if (!vol)
    {
        vol = MakeVector(sizeof(*vol));
        if (!vol) {
            FreeVector(pvd);
            return NULL;
        }

        MaxBlocks = vol->MaxBlocks = pvd->VolSize;
        BlockSize = vol->BlockSize = pvd->BlockSize;
        BlockMask = BlockSize - 1;
        SetBlockShift();

        if (!MakePathTable(pvd)) 
        {
            FreeVector(pvd);
            FreeVector(vol);
            return NULL;
        }

        vol->dev.dl_Type = DLT_VOLUME;
        vol->dev.dl_DiskType = ID_DOS_DISK;
        vol->dev.dl_Name = (BSTR)ATOB(vol->name);
        vol->dev.dl_Lock = vol->dev.dl_LockList = NULL;

        RootDirLBN = vol->RootDirLBN = PathTable->Extent;
        vol->PathTableSize = PathTableSize;
        vol->PathTable = PathTable;
        vol->MaxPaths = MaxPaths;
        vol->PathIndex = PathIndex;

        CopyMem(name,vol->name,name[0]+2); /* include len & term */
        vol->dev.dl_VolumeDate = stamp;  /* struct copy */

        Forbid();
            vol->dev.dl_Next = DOSInfo->di_DevInfo;
            DOSInfo->di_DevInfo = ATOB(vol);
        Permit();
    }
    else /* remount old vol */
    {
        MaxBlocks = vol->MaxBlocks;
        BlockSize = vol->BlockSize;
        BlockMask = BlockSize - 1;
        SetBlockShift();
        RootDirLBN = vol->RootDirLBN;
        PathTableSize = vol->PathTableSize;
        PathTable = vol->PathTable;
        MaxPaths = vol->MaxPaths;
        PathIndex = vol->PathIndex;
        ReadErrs = vol->ReadErrs;
    }

/*  Debug1("MB=%ld RD=%ld PTS=%ld PT=%lx\n",*/
/*      MaxBlocks, RootDirLBN, PathTableSize, PathTable);*/

    ThisVol = vol;
    vol->dev.dl_Task = FSPort;

    /* done with pvd, free it */
    FreeVector(pvd);

    Debug1("\tM%s\n", &vol->name[1]);
    return TRUE;
}


/***********************************************************************
***
***  UnMount
***
*** Unmount a CD volume.  If there are no locks on it, deallocate
*** its resources, otherwize wait for all locks to be freed.
***
***********************************************************************/
UnMount()
{
    D(Debug0("Unmount $%lx\n",ThisVol));
    if (ThisVol)
    {
	if (!ThisVol->dev.dl_LockList) DeMount(ThisVol);
	ThisVol->ReadErrs = ReadErrs;
    }
    FlushCache();

    ThisVol = NULL;
}

/***********************************************************************
***
***  DeMount
***
*** Unlink a volume from the system tables and free its resources.
***
***********************************************************************/
DeMount(vol)
    REG VOLDEV *vol;
{
    REG VOLDEV *v;

    /* Find the vol in dos device list (critical section): */
    Forbid();
    v = (VOLDEV *) BTOA(DOSInfo->di_DevInfo);
    if (v == vol)   /* at head of list */
    {
        DOSInfo->di_DevInfo = v->dev.dl_Next;
    }
    else
    {
        for (; v; v = (VOLDEV *)BTOA(v->dev.dl_Next))
        {
            if (vol == (VOLDEV *)BTOA(v->dev.dl_Next)) break;
        }

        if (v) v->dev.dl_Next = vol->dev.dl_Next;
    }
    Permit();

    FreeVector(vol->PathIndex);
    FreeVector(vol->PathTable);
    FreeVector(vol);
}

/***********************************************************************
***
***  SetVolInfo
***
***********************************************************************/
SetVolInfo(vi)
    REG struct InfoData *vi;
{
    if (ThisVol)
    {
        vi->id_NumSoftErrors    = ReadErrs;
        vi->id_UnitNumber   = UnitNumber;
        vi->id_DiskState    = ID_WRITE_PROTECTED;
        vi->id_NumBlocks    = 
        vi->id_NumBlocksUsed    = MaxBlocks;
        vi->id_BytesPerBlock    = BlockSize;
        vi->id_DiskType     = ThisVol->dev.dl_DiskType;
        vi->id_VolumeNode   = ATOB(ThisVol);
        vi->id_InUse        = (ThisVol->dev.dl_LockList?DOS_TRUE:DOS_FALSE);
    }
    else vi->id_DiskType = ID_NO_DISK_PRESENT; /* was DiskState, CES */
}

/***********************************************************************
***
***  SetBlockShift
***
***********************************************************************/
SetBlockShift()
{
    switch (BlockSize)
    {
    case 2048: BlockShift = 11; return;
    case 1024: BlockShift = 10; return;
    case  512: BlockShift =  9; return;
    default:   BlockShift = 11;
    }
}


/***********************************************************************
***
***  CallBoot
***
*** Call boot code, saving registers first.  Pass the second
*** argument in A1 (an open CD I/O Request).
***
***********************************************************************/
#asm
    public  _CallBoot
_CallBoot:
        movem.l 4(sp),a0/a1
        movem.l d2-d7/a2-a6,-(sp)
        jsr (a0)        ; a1 for Arg
        movem.l (sp)+,d2-d7/a2-a6
        rts
#endasm
