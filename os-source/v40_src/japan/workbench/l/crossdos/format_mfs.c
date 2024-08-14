/* Format.c **************************************************************
** Copyright 1991 CONSULTRON
*
*      DOS Formatting function (ACTION_FORMAT).
*
*************************************************************************/

#include "FS:FS.h"

extern struct ExecBase  *SysBase;
extern struct FS         *fsys;
extern int DOSerror;
extern struct IOStdReq      *diskreq;


/* Default parameters for floppy types */
struct FS_parms mfmt[] = {
{ 112, 1440, 0xF8, 3, 9, 2, 2 },  /* FMT_AUTO */
{ 112, 1440, 0xF9, 3, 9, 2, 2 },  /* FMT_MSDOS_QD9 */
{ 112,  720, 0xFD, 2, 9, 2, 2 },  /* FMT_MSDOS_D9  */
{  64,  360, 0xFC, 2, 9, 1, 2 },  /* FMT_MSDOS_S9  */
{ 112, 1440, 0xF9, 5, 9, 2, 2 },  /* FMT_ATARI_D9  */
{ 112,  720, 0xF8, 5, 9, 1, 2 },  /* FMT_ATARI_S9  */
{ 112*2,1440*2, 0xF0, 5, 18, 2, 2 },  /* FMT_MSDOS_QD18*/
};

static const struct FS_boot_blk M_bb =
{
0xEB,0x3C,0x90,
'C','D','P',' ',' ','3','.','3',
0x00, 0x02,
2,
0x01, 0x00,
2,
0x70, 0x00,
0xA0, 0x05,
0xF9,
0x03, 0x00,
0x09, 0x00,
0x02, 0x00,
0x00, 0x00,
};

static const UBYTE bootcode[] = { 0xFB, 0x31, 0xC0, 0x8E, 0xD0, 0x8E, 0xD8,
 0xBC, 0x00, 0x7C, 0xBE, 0x59, 0x7C, 0xB4, 0x0E, 0xBB, 0x07,
 0x00, 0xAC, 0x84, 0xC0, 0x74, 0xFE, 0xCD, 0x10, 0xEB, 0xF7,};
static const char bootstring[] = "CrossDOS non-bootable disk!";

/*************
*   FormatDisk()  --- Do a QUICK format of disk. If format = 0 then = FMT_AUTO
*
*   This format function ASSUMES that the user wishes to format with a
*   1 SECTOR = 1 BLOCK
*
*   Return DOSTRUE if successful or DOSFALSE if error
*************/
int FormatDisk(diskname, format)
UCHAR *diskname;     /* BADDR(BSTR) */
ULONG format;
{
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
    UBYTE temp0=0, temp1;
    UWORD volatile sectorsz=(fssm_env->de_SizeBlock)<<2;
    ULONG blocks_part, blks_disk, sectors_clust=2;
    volatile ULONG sectors_disk=0;

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
    if(Is_Disk_Out())
    {       /* no disk in drive */
        DOSerror = ERROR_NO_DISK;
        return(DOSFALSE);
    }

    if(DevCmd(TD_PROTSTATUS)
        || diskreq->io_Actual)
    {
        DOSerror = ERROR_DISK_WRITE_PROTECTED;
        return(DOSFALSE);
    }

/**********************************************************************
*   MSDOS FS specific boot block setting
**********************************************************************/
/* Calculate BLOCKS per disk */
    blocks_part = fssm_env->de_BlocksPerTrack * fssm_env->de_Surfaces
        * ((fssm_env->de_HighCyl - fssm_env->de_LowCyl)+1);
    blocks_sec = fssm_env->de_SectorPerBlock;   /* changed to BlocksPerSector for this file system */

DPRTF(KPrintF( "\nFormat: format=%lx",format));

    if(!GetBootBlock((APTR)data, fssm_env))  /* Check if a boot record can be found */
    {
        return(DOSFALSE);
    }

    switch(format)
    {
    case ID_MSDOS_DISK_HD:
        mfmt[FMT_AUTO].mp_sides = fs->f_surfaces;
        mfmt[FMT_AUTO].mp_sectors_track = fs->f_blockspertrack;
        sectors_clust = 8;
        goto Format_partition;

    case FMT_AUTO:
    case ID_DOS_DISK:
    case ID_MSDOS_DISK:
    case ID_MSDOS_DISK_DS:

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
    /* If BLOCKS per disk larger than 64K, increase blocks per sector until < 64K */
        blocks_sec = max(fssm_env->de_SectorPerBlock,(1+(blocks_part/0x10000)) );
        mfmt[fmt].mp_format_id = 0xF9;  /* set format ID to floppy-type */
        }
    /* Drop thru without break */
    case FMT_FORCE:
        mfmt[FMT_AUTO].mp_sides = fssm_env->de_Surfaces;
        mfmt[FMT_AUTO].mp_sectors_track = max(1,fssm_env->de_BlocksPerTrack / blocks_sec);

Format_partition:
        if((sectors_disk = blocks_part/blocks_sec) >= 0x10000) mfmt[FMT_AUTO].mp_sectors_disk = 0;
        else mfmt[FMT_AUTO].mp_sectors_disk = sectors_disk;

    /* Determine an efficient # of root directory entries. I believe that more than
    512 root directory entries are wasteful of disk space */
        if(blocks_part <= 720) mfmt[FMT_AUTO].mp_root_dir_ent = 64;
        else if(blocks_part < 2880) mfmt[FMT_AUTO].mp_root_dir_ent = 112;
        else if(blocks_part < 6000) mfmt[FMT_AUTO].mp_root_dir_ent = 112*2;
        else mfmt[FMT_AUTO].mp_root_dir_ent = 512;

            /* Calc sectors per cluster (non-adjusted) */
            /* For some strange reason, MSDOS (even through V5.0) does not like
            a sectors per cluster value that are not an even multiple of 2 */
           /* Make higher multiple of 2 */
        sectors_clust = max( max(fssm_env->de_PreAlloc,sectors_clust),
            (1+(sectors_disk/(FAT_BAD & fat_mask))) );

            /* If DosType = ID_MSDOS_DISK_HD, then f_Diskflags already set to
            proper value. If not ID_MSDOS_DISK_HD then autoadjust
            to minimum FAT size */
        if( fssm_env->de_DosType != ID_MSDOS_DISK_HD)
        {       /* If not = ID_MSDOS_DISK_HD */
            if((sectors_disk/sectors_clust) > (FAT_BAD & FAT12_MASK))
            {   /* # clusters > 12 bit FAT size.  Try 16 bit FAT size */
                fs->f_Diskflags |= FAT16_F;
            }
            else fs->f_Diskflags &= ~FAT16_F;   /* # clusters < 12 bit FAT size. */
        }

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
        sectors_clust = max(sectors_clust, (1+(sectors_disk/(FAT_BAD & fat_mask))));
        for(i=1<<1; (i <= 256) && (sectors_clust > i); i <<= 1);
        mfmt[FMT_AUTO].mp_sectors_clust = i;

            /* Calc # of Clusters */
        clusters = (sectors_disk)/mfmt[FMT_AUTO].mp_sectors_clust;

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
    CopyMem((UBYTE *)(&M_bb), data, sizeof(struct FS_boot_blk));
    CopyMem(&bootcode, &(data[62]), sizeof(bootcode)+sizeof(bootstring));

    fs_bb = (struct FS_boot_blk *)data;

/** Modify boot block parameters **/
    SWAPWORD(fs_bb->fbb_root_dir_ent,mfmt[fmt].mp_root_dir_ent);

    SWAPWORD(fs_bb->fbb_sectors_disk,mfmt[fmt].mp_sectors_disk);
    if(0 != mfmt[fmt].mp_sectors_disk) sectors_disk=0;
    SWAPLONG(fs_bb->fbb_sectors_disk_32,sectors_disk);

    fs_bb->fbb_format_id = mfmt[fmt].mp_format_id;

    (fs_bb->fbb_sectors_fat)[0] = mfmt[fmt].mp_sectors_fat;

    SWAPWORD(fs_bb->fbb_sectors_track,mfmt[fmt].mp_sectors_track);

    (fs_bb->fbb_sides)[0] = mfmt[fmt].mp_sides;

    SWAPWORD(fs_bb->fbb_bytes_sector,sectorsz);

    fs_bb->fbb_sectors_cluster = mfmt[fmt].mp_sectors_clust;

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

    return(DOSTRUE);
}
