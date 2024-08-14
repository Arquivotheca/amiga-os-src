/* Format.c **************************************************************
** Copyright 1991 CONSULTRON
*
*      DOS Formatting function (ACTION_FORMAT).
*
*************************************************************************/

#include "FS:FS.h"

extern struct ExecBase  *SysBase;
extern struct DOSBase *DOSBase;
extern struct FS         *fsys;
extern int DOSerror;
extern struct IOStdReq      *diskreq;

#define DEFAULT_MAX_CLUSTER_SIZE   8

/* Default parameters for floppy types */
struct FS_parms mfmt[] = {
{ 112, 1440, 0xF9, 3, 9, 2, 2 },  /* FMT_AUTO */
{ 112, 1440, 0xF9, 3, 9, 2, 2 },  /* FMT_MSDOS_QD9 */
{ 112,  720, 0xFD, 2, 9, 2, 2 },  /* FMT_MSDOS_D9  */
{  64,  360, 0xFC, 2, 9, 1, 2 },  /* FMT_MSDOS_S9  */
{ 112, 1440, 0xF9, 5, 9, 2, 2 },  /* FMT_ATARI_D9  */
{ 112,  720, 0xF8, 5, 9, 1, 2 },  /* FMT_ATARI_S9  */
{ 112*2,1440*2, 0xF0, 5, 18, 2, 2 },  /* FMT_MSDOS_QD18*/
};

static const UBYTE bootcode[] = { 
0xEB,0x3C,0x90,                         /* boot jump address displacement */
'C','D','P',' ',' ','5','.','0',        /* system ID not null terminated */
0x00, 0x02,                             /* # bytes per sector (e.g. 512) */
2,                                      /* # sectors per cluster (e.g. 01 or 02 */
0x01, 0x00,                             /* # reserved sectors at beginning (e.g. 1) */
2,                                      /* # copies of FAT (e.g. 2) */
0x70, 0x00,                             /* # root directory entries (e.g. 64 or 112) */
0xA0, 0x05,                             /* # sectors per disk (16 bit) */
0xF9,                                   /* disk format ID # (e.g. F8, F9 or FC-FF) */
0x03, 0x00,                             /* # sectors per FAT (e.g. 1 or 2) */
0x09, 0x00,                             /* # sectors per track (e.g. 8 or 9) */
0x02, 0x00,                             /* # sides (e.g. 1 or 2) */
0x00, 0x00, 0x00, 0x00,                 /* # hidden (?) sectors (e.g. 0) */
0x00, 0x00, 0x00, 0x00,                 /* # sectors per disk when fbb_sectors_disk > 65536 (MSDOS 4+) */
0x00, 0x00, 0x29, 0xBC, 0x07, 0x4D, 0x37,  /* unknown (MSDOS 4+) & volume serial # (MSDOS 4+) */
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,   /* volume name (MSDOS 4+) */
'F',  'A',  'T',  '1',  '2',  0x20, 0x20, 0x20,     /* FAT type string (MSDOS 4+) */
0xFB, 0x31, 0xC0, 0x8E, 0xD0, 0x8E, 0xD8,           /* CrossDOS boot code */
0xBC, 0x00, 0x7C, 0xBE, 0x59, 0x7C, 0xB4, 0x0E, 0xBB, 0x07,
0x00, 0xAC, 0x84, 0xC0, 0x74, 0xFE, 0xCD, 0x10, 0xEB, 0xF7,
};
static const char bootstring[] = "CrossDOS(TM) non-bootable disk!";

/*************
*   FormatDisk()  --- Do a QUICK format of disk. If format = 0 then = FMT_AUTO
*
*   This format function ASSUMES that the user wishes to format with a
*   1 SECTOR = 1 BLOCK
*
*   Return DOSTRUE if successful or DOSFALSE if error
*************/
int FormatDisk(UCHAR *diskname, ULONG format)
/* diskname = BADDR(BSTR) */
{
F();
    register struct FS           *fs = fsys;
    register struct FS_boot_blk  *fs_bb;
    register struct FS_dir_ent   *dirent;

    register LONG endlw, i, SizeBlock;
    register struct DosEnvec *fssm_env = (struct DosEnvec *)BADDR(fs->f_fssm->fssm_Environ);
    ULONG clusters, lowblock;
    register UBYTE *data=fs->f_scratch;
    ULONG datasz;
    int  error;
    UBYTE fmt=FMT_AUTO;
    UWORD blocks_sec = MP_blocks_sec, fat_mask=FAT12_MASK;
    UWORD rootdir_ent=512;  /* Default for hard drives */
    volatile UWORD temp0=0;
    volatile ULONG temp1;
    volatile UWORD sectorsz=((fssm_env->de_SizeBlock)<<2);
    ULONG blocks_part, blks_disk, sectors_clust=2;

    DOSerror = ERROR_DISK_NOT_VALIDATED;

/** Check if proper name requirements */
    if( (diskname == 0) || (diskname[0] == 0) )
    {
        DOSerror = ERROR_INVALID_COMPONENT_NAME;
        return(DOSFALSE);
    }

/****
    Check if disk installed and if write protected
****/
    if(Is_Disk_Out(FALSE))
    {       /* no disk in drive */
    /*  DOSerror = ERROR_NO_DISK already set by Is_Disk_Out() */
        return(DOSFALSE);
    }

    /*  if DOSerror = ERROR_DISK_WRITE_PROTECTED it was set by Is_Disk_Out() */
    if(DOSerror == ERROR_DISK_WRITE_PROTECTED)
    {
        return(DOSFALSE);
    }

/**********************************************************************
*   MSDOS FS specific boot block setting
**********************************************************************/
/* Calculate BLOCKS per disk */
    blocks_part = fssm_env->de_BlocksPerTrack * fssm_env->de_Surfaces
        * ((fssm_env->de_HighCyl - fssm_env->de_LowCyl)+1);

DPRTF(KPrintF( "\n%s: %s=%lx  %s=%ld  %s=%lx",__FUNC__,s(format),s(blocks_part),s(fssm_env)));

    if(!GetBootBlock((APTR)data, fssm_env))  /* Check if a boot record can be found */
    {
        return(DOSFALSE);
    }

    switch(format)
    {
    case ID_MSDOS_DISK_HD:
        mfmt[FMT_AUTO].mp_sides = fs->f_surfaces;
        mfmt[FMT_AUTO].mp_sectors_track = fs->f_blockspertrack;
        mfmt[FMT_AUTO].mp_format_id = 0xF8;  /* set format ID to hard drive-type */
        sectors_clust = DEFAULT_MAX_CLUSTER_SIZE;
        goto Format_partition;

    case ID_MSDOS_DISK:
    case ID_MSDOS_DISK_DS:
    case FMT_AUTO:
    case ID_DOS_DISK:

/* Use Drive Geometry if:
  1) de_DosType = ID_MSDOS_DISK or ID_MSDOS_DISK_DS    and
  2) the boot block is to be located at sector 0    and
  3) TD_GETGEOMETRY command supported otherwise use TD_GETDRIVETYPE if supported.
*/
        if(fssm_env->de_LowCyl == 0)
        {
        /****
            Initialize disk physical params to default (if supported by the device code)
        ****/
            if(blks_disk = Get_Default_Drive_Geometry()) blocks_part = blks_disk;
                /* use drive type */
            else if( !DevCmd(TD_GETDRIVETYPE))
            {
                if(diskreq->io_Actual == DRIVE3_5) fmt = FMT_MSDOS_QD9;
                else if(diskreq->io_Actual == DRIVE5_25)
                {
                    if(fssm_env->de_Surfaces == 1) fmt = FMT_MSDOS_S9;  /* if single-sided */
                     else fmt = FMT_MSDOS_D9;     /* else double-sided */
                }
                 else if(diskreq->io_Actual == DRIVE3_5_150RPM) fmt = FMT_MSDOS_QD18;
                if(fmt != FMT_AUTO) break;  /* if fmt != FMT_AUTO break out of switch() */
            }
            mfmt[fmt].mp_format_id = 0xF9;  /* set format ID to floppy-type */
        }
        blocks_sec = fssm_env->de_SectorPerBlock;   /* changed to BlocksPerSector for this file system */
    /* Drop thru without break */
    case FMT_FORCE:
        mfmt[FMT_AUTO].mp_sides = fssm_env->de_Surfaces;
        mfmt[FMT_AUTO].mp_sectors_track = max(1,fssm_env->de_BlocksPerTrack / blocks_sec);
DPRTF(KPrintF( "\n%s: %s=%ld  %s=%ld",
    __FUNC__,s(mfmt[FMT_AUTO].mp_sides),s(mfmt[FMT_AUTO].mp_sectors_track)));

Format_partition:
        sectorsz = sectorsz * blocks_sec;

            /* Calc sectors per cluster (non-adjusted) */
            /* Set the temp max sectors per cluster to DEFAULT_MAX_CLUSTER_SIZE */
        sectors_clust = max(fssm_env->de_PreAlloc,sectors_clust);

        mfmt[fmt].mp_sectors_part = blocks_part/blocks_sec;

        if( fssm_env->de_DosType != ID_MSDOS_DISK_HD)
        {       /* If not = ID_MSDOS_DISK_HD */
            /* It appears that MSDOS does not like # of sectors over 64K on a floppy */
            if(mfmt[fmt].mp_sectors_part >= 0x10000)
            {
                DOSerror = ERROR_UNKNOWN_DISK_FORMAT;
                return(DOSFALSE);
            }

            fs->f_Diskflags &= ~FAT16_F;   /* # clusters < 12 bit FAT size. */
/**************************************************************************
This is older code that would try to promote a floppy disk format to use
a 16 bit FAT if it could not be reasonably fit in a 12 bit FAT.
Do to current limitations in CrossPC WRT incorrectly accessing floppy media with
16 bit FAT, I chose, at this time, to always use a 12 bit FAT for floppies.
This may change in the future. If so, I will use this code again.
**************************************************************************
            /* If DosType = ID_MSDOS_DISK_HD, then f_Diskflags already set to
            proper value. If not ID_MSDOS_DISK_HD then autoadjust
            to minimum FAT size */

            if((mfmt[fmt].mp_sectors_part/sectors_clust) > (FAT_BAD & FAT12_MASK))
            {   /* # clusters > 12 bit FAT size.  Try 16 bit FAT size */
                fs->f_Diskflags |= FAT16_F;
            }
            else fs->f_Diskflags &= ~FAT16_F;   /* # clusters < 12 bit FAT size. */
**************************************************************************/

        /* Determine an efficient # of root directory entries. I believe that more than
        512 root directory entries are wasteful of disk space */
            if(blocks_part <= 720) rootdir_ent = 64;
            else if(blocks_part < 2880) rootdir_ent = 112;
            else if(blocks_part < 6000) rootdir_ent = 112*2;
            else rootdir_ent = 240; /* For some reason, MSDOS does not like # of
                                    root dir entries > than 256 for floppies */
        }

    /* Load in the maximum of either the default root dir entries
       or user-specified value in de_Control */
        if(fssm_env->de_TableSize >= DE_CONTROL)
            rootdir_ent = max(rootdir_ent, fssm_env->de_Control);

    /* Normalize to the actual # of dir entries per block */
        mfmt[FMT_AUTO].mp_root_dir_ent = rootdir_ent & ~((sectorsz / sizeof(struct FS_dir_ent))-1);

        if(fs->f_Diskflags & FAT16_F)
        {   /* 16 bit FAT table entry size */
            temp0 = 2;
            temp1 = 1;
            fat_mask = FAT16_MASK;
        }
        else
        {   /* 12 bit FAT table entry size */
            temp0 = 12;
            temp1 = 8;
            fat_mask = FAT12_MASK;
        }

            /* Calc sectors per cluster (adjusted) */
            /* For some strange reason, MSDOS (even through V5.0) does not like
            a sectors per cluster value that are not an even multiple of 2 */
           /* Make higher multiple of 2 */
        sectors_clust = max(sectors_clust, (1+(mfmt[FMT_AUTO].mp_sectors_part/(FAT_BAD & fat_mask))));
        for(i=1<<1; (i <= 256) && (sectors_clust > i); i <<= 1);
        mfmt[FMT_AUTO].mp_sectors_clust = i;

            /* Calc # of Clusters */
        clusters = (mfmt[FMT_AUTO].mp_sectors_part)/mfmt[FMT_AUTO].mp_sectors_clust;

        mfmt[FMT_AUTO].mp_sectors_fat = ((clusters * temp0)/(sectorsz * temp1))+1;
        break;

    case FMT_MQD18:
        if(temp0 == DRIVE5_25) return(DOSFALSE); 
        fmt = FMT_MSDOS_QD18;
        break;
    case FMT_MQD9:
        if(temp0 == DRIVE5_25) return(DOSFALSE); 
        fmt = FMT_MSDOS_QD9;
        break;
    case FMT_MD9 :
        fmt = FMT_MSDOS_D9; 
        break;
    case FMT_MS9 :
        fmt = FMT_MSDOS_S9; 
        break;
    case FMT_AD9 :
        if(temp0 == DRIVE5_25) return(DOSFALSE); 
        fmt = FMT_ATARI_D9; 
        break;
    case FMT_AS9 :
        if(temp0 == DRIVE5_25) return(DOSFALSE); 
        fmt = FMT_ATARI_S9; 
        break;
    default:
        DOSerror = ERROR_UNKNOWN_DISK_FORMAT;
        return(DOSFALSE);
        break;
    }

/** Get boot block **/
    if( !(data = AllocMem( datasz = sectorsz, (fssm_env->de_BufMemType)|MEMF_CLEAR )))
    {
        DOSerror = ERROR_NO_FREE_STORE;
        return(DOSFALSE);
    }

/** Copy default boot block into block **/
    CopyMem(&bootcode, data, sizeof(bootcode)+sizeof(bootstring));

    fs_bb = (struct FS_boot_blk *)data;

/** Modify boot block parameters **/
    SWAPWORD(fs_bb->fbb_root_dir_ent,mfmt[fmt].mp_root_dir_ent);

/* Alter the FAT type if 16 bit FAT only */
    if(fs->f_Diskflags & FAT16_F)
    {   /* 16 bit FAT table entry size */
        fs_bb->fbb_FAT_type[4] = '6';       /* change the FAT type string to "FAT16" */
    }
/* Add volume name to boot block */
    CopyMem(&diskname[1], fs_bb->fbb_volume_name, min(diskname[0], VOLNAME_SIZE));

    if(mfmt[fmt].mp_sectors_part >= 0x10000)
    {   /* Can't fit in 16 bit sectors_part field, use 32 bit field */
        temp0 = 0;
        temp1 = mfmt[fmt].mp_sectors_part;
    }
    else
    {   /* Can fit in 16 bit sectors_part field */
        temp0 = mfmt[fmt].mp_sectors_part;
        temp1 = 0;
    }
    SWAPWORD(fs_bb->fbb_sectors_disk,temp0);
    SWAPLONG(fs_bb->fbb_sectors_disk_32,temp1);

    fs_bb->fbb_format_id = mfmt[fmt].mp_format_id;

    (fs_bb->fbb_sectors_fat)[0] = mfmt[fmt].mp_sectors_fat;

    SWAPWORD(fs_bb->fbb_sectors_track,mfmt[fmt].mp_sectors_track);

    (fs_bb->fbb_sides)[0] = mfmt[fmt].mp_sides;

    SWAPWORD(fs_bb->fbb_bytes_sector,sectorsz);

    fs_bb->fbb_sectors_cluster = mfmt[fmt].mp_sectors_clust;

/* Set the hidden sectors value by subtracting the current partition table
  block from the beginning of the partition block */
    temp1 = fs->f_beg_part - fs->f_curr_part_tbl;
    SWAPLONG(fs_bb->fbb_hidden_sectors,temp1);

    *((UWORD *)&(data[sectorsz-sizeof(UWORD)])) = (UWORD)0x55AA;

    fs->f_bytes_block = (fssm_env->de_SizeBlock)<<2; /* set default block size */
    for(i=1,SizeBlock=fssm_env->de_SizeBlock; SizeBlock!=0; i++, SizeBlock>>=1);
    fs->f_bytes_block_sh = i; /* set block size shift value */

    error = PutBlockMem((APTR)data, lowblock = fs->f_beg_part, MP_res_sectors_beg * blocks_sec);

    FreeMem(data, datasz);

    if(error) return(error);

/** Initialize disk **/
    fs->f_FSflags |= MF_FSDISK;   /* set FS DISK flag */
    if(fs->f_VolumeNode)
    {   /* If there was a current Volume before the format; Delete it NOW */
        DeleteVolNode(fs->f_VolumeNode);
        fs->f_VolumeNode = 0;
        fs->f_LockList = 0;  /* If there were outstanding locks before
                                TOUGH.  If it breaks, it is your fault */
    }

    if(InitDisk() != 0)
    {
        DOSerror = ERROR_DISK_NOT_VALIDATED;
        return(DOSFALSE);
    }

/** Get root directory block **/
/* clear (=0) first byte of all filenames in root directory entries */
    dirent = (struct FS_dir_ent *)fs->f_rootdir_cache;
    for(i=0; i<fs->f_root_dir_ent; i++,dirent++)
    {
        (dirent->fde_filename)[0] = '\0';
    }

    dirent = (struct FS_dir_ent *)fs->f_rootdir_cache;

    fs->f_NumSoftErrors = 0;   /* set # of soft errors to 0 */

/** Free all cluster entries in FAT table **/
    endlw = fs->f_fat_copy_sz/sizeof(ULONG);
    for(i=0; i<endlw; i++) ((ULONG *)fs->f_fat_copy)[i] = 0;

/** Write Format ID in FAT table **/
    if(fs->f_Diskflags & FAT16_F) i = 0xFFFFFF;
      else i = 0xFFFF00;

    *((ULONG *)&(fs->f_fat_copy[0])) = (ULONG)(mfmt[fmt].mp_format_id)<<24 | i;

    fs->f_Diskflags |= FAT_MOD_F;   /* set FAT Modified flag */

    fs->f_used_blocks = usedblocks();

/**** Set Volume Label attribute in directory entry ****/
    RenameDisk(diskname,DOSTRUE);

    fs->f_Diskflags |= ROOTDIR_MOD_F;   /* set Root Directory Modified flag */

    MakeVolNode();

    Flush_Buffers();        /* Flush modified FAT and root dir */

    Motor_Off();

    DOSerror=0;     /* no error */
    return(DOSTRUE);
}
