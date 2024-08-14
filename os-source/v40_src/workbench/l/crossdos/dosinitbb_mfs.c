/*** DOSinitBB_MFS.c  ************************************************************
** Copyright 1991 CONSULTRON
*
*       DOS initialization Block Block subroutines
*
****************************************************************************/

#include "FS:FS.h"
#include "FS:Fault_txt.h"

/******
* STATIC data references
*******/
extern struct ExecBase  *SysBase;
extern struct DosLibrary *DOSBase;
extern struct IntuitionBase *IntuitionBase;
extern struct IOStdReq *diskreq;
extern struct FS *fsys;
extern char DeviceName[];

static void FATC_Req(void);

/* Partition types */
#define NO_PT       0   /* no partition */
#define PRI_12_PT   1   /* primary DOS partition (12 bit FAT) */
#define PRI_16_PT   4   /* primary DOS partition (16 bit FAT) */
#define EXT_PT      5   /* extended DOS partition */
#define PRI_PT_4    6   /* primary DOS partition (MSDOS 4.0) (Assumed) */

#define PRI_PART_ENT    4   /* Primary partition table entries */


/**********************************************************************
*   GetBootBlock() -- Finds MSDOS Boot Block of the Partition
*       (No validation of the Boot Record occurs in this routine.)
*
*   If DOSType = "ID_MSDOS_DISK_HD", expect a valid FDISK-type
*   hard disk partition table.  It will then parse the partition
*   table to find the desired partition.  It handles MSDOS 3.0
*   primary and extended partitions.  New types of MSDOS partitions
*   4.0 and beyond can be supported if the veil of secrecy is removed
*   from this generally proprietry information.
*
*   To find and point to the first partition, the device name must end
*   in 'C'.  The second partition must end in 'D', etc.
*
*   Once the partition is found, the DosEnvec of the device is modified
*   to point to the boot record block of the partition.
*
*   If DOSType != "ID_MSDOS_DISK_HD", expect a valid boot record block
*   (such as with floppies).
*
*   The modification of the DosEnvec of the device is defined in the
*   NOTE located at the bottom.
**********************************************************************/
UBYTE *GetBootBlock(data, fssm_Env)
register UBYTE *data;
register struct DosEnvec *fssm_Env;
{
F();
    register struct FS           *fs = fsys;
    register struct FS_boot_blk  *fs_bb;
    register struct FS_HD_blk  *fs_HDb;
    ULONG lowblock;
    ULONG volatile startsec=0, numsecs=0, ext_part_sec=0;
    BYTE drv_id;
    UBYTE index, pri_index=0, part_type;

/****
    Get FS Boot Block or hard disk partition table
****/
    fs->f_curr_part_tbl = fs->f_beg_part = lowblock = fssm_Env->de_DiskLowBlock;
DPRTF(KPrintF( "\nGetBootBlock: fssm_Env=%lx  lowblock=%ld",fssm_Env, lowblock));

    if(GetBlockMem((APTR)data, lowblock, 1))
    {
        fs->f_FSflags |= MF_NOTREADABLE;      /* set flag -- NOT READABLE DISK disk */
        return(0);
    }
    fs->f_FSflags &= ~MF_NOTREADABLE;      /* clear flag -- NOT READABLE DISK disk */

    fs_bb = (struct FS_boot_blk *)data;

/****
    Check to see if the block specified is the HD partition block
        if de_DosType = MSH'\0'
         ASSUME that ONE SECTOR = ONE BLOCK
****/
    if( fssm_Env->de_DosType == ID_MSDOS_DISK_HD)
    {
    /* Find out which partition to parse.  Based on the last char of the device name */
    /* 'C' = first partition. 'D' = second partition. etc */
        drv_id = strlen(DeviceName)-1;
        drv_id = DeviceName[drv_id]-'C';

        if( (drv_id < 0) || (drv_id > 24) )  /* check if last character in device name NOT between 'C' and 'Z' */
        {   /* Outside the partition limit */
            return(0);
        }

    /* Do a simple check to see if this is a HD partition block.
        Check for 0x55AA in the last word of the block */
    /* This code is implemented to provide a "kludge" for some Amiga vendors
        creating MS-DOS products that do not use the beginning of the Amiga
        partition to begin the MS-DOS partition.  Such a vendor is Vortex with
        their AtOnce product. */

        for(index=max(100,fssm_Env->de_Surfaces * fssm_Env->de_BlocksPerTrack);
                index && ( ((struct FS_HD_blk *)data)->fhdb_signature != (UWORD)0x55AA);
                index--)
        {   /* Not found! */
DPRTF(KPrintF( "\nGetBootBlock: index=%ld",index));
            if(GetBlockMem((APTR)data, ++lowblock, 1))
            {   /* error in reading block! */
                return(0);
            }
        }
        if(index == 0)
        {   /* Not found! */
            return(0);
        }

        fssm_Env->de_DiskLowBlock = lowblock; /* This is to speed up future searches */
    /********** End kludge *******/

        fs_HDb = (struct FS_HD_blk *)data;

        for(index=0; (drv_id >= 0) && (pri_index < PRI_PART_ENT); drv_id--)
        {
        /* Do a simple check if this is a HD partition block.
            Check for 0x55AA in the last word of the block */
            if( ((struct FS_HD_blk *)data)->fhdb_signature != (UWORD)0x55AA)
            {   /* Not found! */
                return(0);
            }

            part_type = fs_HDb->fhdb_p_tbl[index].fhdp_part_type;
DPRTF(KPrintF( "\nGetBootBlock: part_type=%ld",part_type));

        /* Partition type parsing */
            switch(part_type)
            {
            case EXT_PT:    /* extended DOS partition */
                  SWAPLONG(startsec,fs_HDb->fhdb_p_tbl[index].fhdp_start_sec);  /* get start sector */
                if(!ext_part_sec)
                {
                    ext_part_sec += startsec;
                    startsec = 0;
                }

                drv_id++;   /* We haven't found it yet. Look again */
DPRTF(KPrintF( "\nGetBootBlock: lowblock=%ld  ext_part_sec=%ld  startsec=%ld",lowblock,ext_part_sec,startsec));
                if(GetBlockMem((APTR)data, lowblock+ext_part_sec+startsec, 1))
                {
                    return(0);
                }
                break;

            case NO_PT:     /* Not a true partition type -- Skip this */
                drv_id++;
                ext_part_sec = 0;
                break;

            case PRI_16_PT:     /* Primary DOS partition (16 bit) */
            case PRI_PT_4:      /* Primary DOS partition (MSDOS 4) */
                fs->f_Diskflags |= FAT16_F;
                break;
            case PRI_12_PT:     /* Primary DOS partition (12 bit) */
                fs->f_Diskflags &= ~FAT16_F;    /* set default FAT size to 12 bit */
                break;
            default:            /* Unknown partition type -- Skip this */
                drv_id++;
                break;
            }

            if(ext_part_sec)
            {
                if(drv_id >= 2)
                {
                    index = 1;
                    drv_id--;
                }
                 else index = 0;
DPRTF(KPrintF("\ndrv_id=%ld  index=%ld",drv_id, index));
            }
             else if(drv_id)
            {
                index = (++pri_index);
            }
        }
        if(pri_index >= PRI_PART_ENT) return(0);   /* exit if beyond the partition table array size */

DPRTF(KPrintF( "\nGetBootBlock: lowblock=%ld  ext_part_sec=%ld  startsec=%ld",lowblock,ext_part_sec,startsec));
    /* Find the location of the current partition table and point to the boot record block */
        fs->f_curr_part_tbl = (lowblock += startsec + ext_part_sec);
          SWAPLONG(startsec,fs_HDb->fhdb_p_tbl[index].fhdp_start_sec);  /* get start sector */
          SWAPLONG(numsecs,fs_HDb->fhdb_p_tbl[index].fhdp_num_secs);  /* get num sectors per disk */

        fs->f_surfaces = fs_HDb->fhdb_p_tbl[index].fhdp_end_head + 1;
        fs->f_blockspertrack = (fs_HDb->fhdb_p_tbl[index].fhdp_end_sec_cyl[0]) & 0x3F;

DPRTF(KPrintF( "\nGetBootBlock: lowblock=%ld  ext_part_sec=%ld  startsec=%ld",lowblock,ext_part_sec,startsec));
        if(GetBlockMem((APTR)data, lowblock += startsec, 1))
        {
            return(0);
        }

        fssm_Env->de_Surfaces = 1;
        fssm_Env->de_BlocksPerTrack = 1;
        fssm_Env->de_LowCyl  = lowblock;
        fssm_Env->de_HighCyl = numsecs + lowblock - 1;
        fs->f_end_disk = fssm_Env->de_DiskHighBlock = max(fssm_Env->de_DiskHighBlock,
            fssm_Env->de_HighCyl);

        fs_bb = (struct FS_boot_blk *)data;
    }

    fs->f_beg_part = lowblock;             /* set beginning block of partition */
    return(data);
}


/**********************************************************************
*   InitDisk()  --- Initialize disk parameters in FS struct based on boot record block
**********************************************************************/
int InitDisk()
{
F();
    register struct FS           *fs = fsys;
    register struct FS_boot_blk  *fs_bb;
    register struct DosEnvec *fssm_Env = (struct DosEnvec *)BADDR(fs->f_fssm->fssm_Environ);
    UBYTE *data=fs->f_scratch, i;
    ULONG lowblock, tempblock;
    ULONG volatile templong;
    UWORD volatile tempword;

    if(fs->f_FSflags & MF_NODISK) return(-1L);     /* check if NO DISK in drive */

/****
    Get FS Boot Block
****/
    if(0 == (fs_bb = (struct FS_boot_blk *)GetBootBlock((APTR)data, fssm_Env))
        /**** Check for valid FS boot record block (Format ID must be >= 0xF0) ****/
        || ( (0xF0 > fs_bb->fbb_format_id) || (fs_bb->fbb_copies_fat > 2)) )
    {
        RemDisk();
        return(-1L);
    }

    lowblock = fs->f_beg_part;             /* set beginning block of partition */

/****
    Fill in bytes per block param
****/
      SWAPWORD(tempword,fs_bb->fbb_bytes_sector);
        /* de_SectorPerBlock redefined to BlocksPerSector */
    fssm_Env->de_SectorPerBlock =
      (fs->f_blocks_sector = tempword>>fs->f_bytes_block_sh);

/****
    Fill FAT params
****/
    fs->f_copies_fat = fs_bb->fbb_copies_fat;

      SWAPWORD(tempword,fs_bb->fbb_sectors_fat);
    fs->f_blocks_fat = tempword * fs->f_blocks_sector;
      SWAPWORD(tempword,fs_bb->fbb_res_sectors_beg);
    fs->f_beg_fat1 = (tempword * fs->f_blocks_sector) + lowblock;
    fs->f_beg_fat2 = fs->f_beg_fat1 + fs->f_blocks_fat;

/****
    Fill Volume params
****/

    fs->f_sys_id[sizeof(fs_bb->fbb_sys_id)+1] = '\0';
    for(i=0; i<sizeof(fs_bb->fbb_sys_id); i++)
    {
        if('\0' == (fs->f_sys_id[1+i] = fs_bb->fbb_sys_id[i])) break;
    }
    fs->f_sys_id[0] = i;

/* Try to convert volume serial # into a somewhat unique DateStamp if the the
    disk doesn't have a volume label entry in the root dir */

    ConvertFileDate(fs_bb->fbb_serial_num, &fs->f_vol_date);

/* Test if sectors per disk = 0.  This is a undesirable condition but it can
occur in some HD formatting programs on the PC.  Such a program found to do
that is "SpeedStore".  This usually occurs because the "sectors_disk" value is
only 64K in maximum sectors.  Partitions over 32M may tend to exceed this limit.
MSDOS 5.0 documentation seems to indicate another "sectors_disk" in an extended
version of the boot block. This value can accomodate 4G in sectors instead of 
64K.

We perform three checks to determine the sectors per disk.
1) from fbb_sectors_disk
2) from fbb_sectors_disk_32
3) from the physical geometry given in the Dos environment vector.

MSDOS incompatibility strikes AGAIN!
*/

    /* Calculate based on the fbb_sectors_disk */
      SWAPWORD(tempword,fs_bb->fbb_sectors_disk);
    templong = tempword;
    if(templong == 0)
    {
        /* Calculate based on the fbb_sectors_disk_32 (> 32M) */
          SWAPLONG(templong,fs_bb->fbb_sectors_disk_32);
        if(templong == 0)
        {
            /* Last chance! Calculate based on the geometry given in the Dos environment vector */
            templong = ((fssm_Env->de_HighCyl+1)
                *(fssm_Env->de_BlocksPerTrack)
                *(fssm_Env->de_Surfaces))
                -lowblock;
        }
    }
    /*** Fix bug for partitions > 32M ***/
    fs->f_blocks_part = templong * fs->f_blocks_sector;

/****
    Fill Directory params
****/
      SWAPWORD(tempword,fs_bb->fbb_root_dir_ent);
    fs->f_root_dir_ent = tempword;
    fs->f_dirents_block = ( fs->f_bytes_block / sizeof(struct FS_dir_ent));
    fs->f_blocks_clust = fs_bb->fbb_sectors_cluster  * fs->f_blocks_sector;
    fs->f_dirents_clust = fs->f_dirents_block * fs->f_blocks_clust;
    fs->f_root_blocks = fs->f_root_dir_ent/fs->f_dirents_block;
    fs->f_beg_root_dir = fs->f_beg_fat1
        + (fs->f_blocks_fat)*fs_bb->fbb_copies_fat;

/****
    Fill Data params
****/
    fs->f_beg_data = fs->f_beg_root_dir + fs->f_root_blocks;
    fs->f_end_clust = ((fs->f_blocks_part - (fs->f_beg_data - lowblock))/fs->f_blocks_clust)
        + START_CLUST;
    fs->f_ffFATent = START_CLUST;

/****
    Allocate FAT memory 
****/
    if( fs->f_fat_copy )  FreeMem(fs->f_fat_copy, fs->f_fat_copy_sz);

    if( !(fs->f_fat_copy = (UBYTE *)AllocMem(
        (fs->f_fat_copy_sz = fs->f_blocks_fat<<fs->f_bytes_block_sh),fssm_Env->de_BufMemType)))
    {
        fs->f_fat_copy_sz = 0;
        return(-1L);
    }

/****
    Allocate Root Directory cache memory 
****/
    if( fs->f_rootdir_cache)  FreeMem(fs->f_rootdir_cache, fs->f_rootdir_cache_sz);

    if( !(fs->f_rootdir_cache = (UBYTE *)AllocMem(
        (fs->f_rootdir_cache_sz = fs->f_root_blocks<<fs->f_bytes_block_sh),fssm_Env->de_BufMemType)))
    {
        fs->f_rootdir_cache_sz = 0;
        return(-1L);
    }

/****
    Get cache buffers
****/
    if( !AllocCache(fssm_Env->de_NumBuffers,(fs->f_blocks_clust<<fs->f_bytes_block_sh)))
    {
        return(-1L);
    }

        /* f_surfaces and f_blockspertrack may end up being different from
        ** de_Surfaces and de_BlockPerTrack because:
        ** The first reflects the logical surfaces from the MSDOS disk image
        ** perspective. While the second is from the AmigaDOS disk perspective. */
      SWAPWORD(tempword,fs_bb->fbb_sides);
    fs->f_surfaces = tempword;
      SWAPWORD(tempword,fs_bb->fbb_sectors_track);
    fs->f_blockspertrack = tempword * fs->f_blocks_sector;

    if( fssm_Env->de_DosType != ID_MSDOS_DISK_HD)
    {
        fssm_Env->de_Surfaces = fs->f_surfaces;            
        fssm_Env->de_BlocksPerTrack = fs->f_blockspertrack;
        fssm_Env->de_LowCyl  =
            ((lowblock)/(fssm_Env->de_BlocksPerTrack))/(fssm_Env->de_Surfaces);
        fssm_Env->de_HighCyl =
            ((((fs->f_blocks_part)
            /(fssm_Env->de_BlocksPerTrack))
            /(fssm_Env->de_Surfaces))
            -1)
            +fssm_Env->de_LowCyl;
    /* recalculate the last block # of the MS-DOS "disk" and store it in de_DiskHighBlock. */
        fs->f_end_disk = fssm_Env->de_DiskHighBlock = ((fssm_Env->de_HighCyl+1)
                        * fssm_Env->de_Surfaces
                        * fssm_Env->de_BlocksPerTrack)-1;
    }
/****
    Initialize disk physical params (if supported by the device code)
****/
    if(fs->f_FS_special_flags & MF_SP_DEVICE)
    {
          tempblock = fssm_Env->de_SecOrg;    /* Initialize sector origin. ie Sec #s on track begin with */
          fssm_Env->de_SecOrg= 1;    /* Initialize sector origin. ie Sec #s on track begin with */
          diskreq->io_Data = (APTR) fssm_Env;
          diskreq->io_Command = MDCMD_SETPARMS;
        DoIO((struct IORequest *)diskreq);

          fssm_Env->de_SecOrg = tempblock;    /* Initialize sector origin. ie Sec #s on track begin with */
    }

/****
    Find if Volume uses 16 bit FAT entries
****/
    if( !(fs->f_Diskflags & FAT16_F) && (fs->f_end_clust >= (FAT_BAD & FAT12_MASK)) )
    {
        fs->f_Diskflags |= FAT16_F;
    }

/********
    Get FAT from disk
********/
    if(GetBlockMem((APTR)fs->f_fat_copy, fs->f_beg_fat1,fs->f_blocks_fat)
        || (diskreq->io_Error))     /* check for any errors that were ignored */
    {                   /* if error in reading FAT get second FAT */
        if(GetBlockMem((APTR)fs->f_fat_copy, fs->f_beg_fat2,fs->f_blocks_fat)
            || (diskreq->io_Error))     /* check for any errors that were ignored */
        {
            FATC_Req();
        }
    }

/********
    Get Root Directory from disk
********/
    if(GetBlockMem((APTR)fs->f_rootdir_cache, fs->f_beg_root_dir,
        fs->f_root_blocks))
    {                   /* error in reading root dir */
        return(-1L);
    }

    return(0);
}


static const char *FATC_ErrorMsg1 = "Results uncertain.";
static const char *FATC_ErrorMsg2 = "Proceed at your own risk.";
static const char *disk_cont = "Continue";

extern struct EasyStruct ERS;
extern char *disk_warn;

extern char *FixStringBuff();


/******************
* FATC_Req() -- Make a simple requestor for "File Allocation Table Corrupt!"
******************/
static void FATC_Req(void)
{
F();
    char tempbuff[TSTRG_BUFF_SZ];

    ERS.es_TextFormat   = &tempbuff[1];
    ERS.es_Title        = disk_warn;

    Motor_Off();
    Fault(VALIDATION_ERROR,"",tempbuff,TSTRG_BUFF_SZ);          /* Fault() = Error validating %b */
    strcat(tempbuff,"\n %s");                       /* "FAT" */
    Fault(IN_DEVICE,tempbuff,tempbuff,TSTRG_BUFF_SZ);    /* Fault() = in device %s%s */
    strcat(tempbuff,"\n %s");                       /* "ErrorMsg1" */
    strcat(tempbuff,"\n %s");                       /* "ErrorMsg2" */
    FixStringBuff(&tempbuff[1]);
    EasyRequest(PKT_WINDOW, &ERS, 0, 0, "File Allocation Table", DeviceName, "",
        FATC_ErrorMsg1, FATC_ErrorMsg2, disk_cont);
    fsys->f_NumSoftErrors++;
}


/**********************************************************************
*   NOTE
*
*   The DosEnvec is modified with the information obtained by the
*   partition table and the boot record block.  This is done to
*   automatically adjust for different formatted media in the same drive.
*   This method is also used by AmigaDOS file systems subsequent to 2.0.
*
*   The following are changes the definitions of some of the DosEnvec
*   elements:
*
*   de_Surfaces -- actual # of surfaces or
*       '= 1' if DOSType = "ID_MSDOS_HD"
*   de_BlocksPerTrack -- actual # of blocks per track or
*       '= 1' if DOSType = "ID_MSDOS_HD"
*   de_LowCyl   -- actual low cylinder # or
*       actual absolute block # of boot record block of the partition
*       if DOSType = "ID_MSDOS_HD" and partition is valid.
*   de_HighCyl  -- actual high cylinder # or
*       actual absolute block # of the last block of the partition
*       if DOSType = "ID_MSDOS_HD" and partition is valid.
*   de_DiskLowBlock (orig. de_SecOrg) -- = 0 if file system not loaded
*       for the device or absolute block # of the hard disk partition table.
*   de_BlocksPerCluster (orig. de_PreAlloc) -- desired minimum blocks per
*       cluster value when formatting.
*   de_BlocksPerSector (orig. de_SectorsPerBlock) -- # of blocks per sector
*       value when formatting.
*   de_DiskHighBlock (orig. de_Interleave) -- block # of the last block in
*       the entire "disk". This used to help determine the total # of blocks
*       in the "disk" for programs such as "IBeM".
**********************************************************************/
