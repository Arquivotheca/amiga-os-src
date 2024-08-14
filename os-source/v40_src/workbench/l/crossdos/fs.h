/*** FS.h ****************************************************************
** Copyright 1991 CONSULTRON
*
*       FileSystem header file
*
****************************************************************************/
               
#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/memory.h"
#include "exec/interrupts.h"
#include "exec/ports.h"
#include "exec/libraries.h"
#include "exec/io.h"
#include "exec/tasks.h"
#include "exec/execbase.h"
#include "exec/devices.h"
#include "devices/trackdisk.h"
#include "devices/timer.h"
#include "dos/dos.h"
#include "dos/dosextens.h"
#include "dos/filehandler.h"
#ifdef   MFS
#ifdef   AMB
#include "FS:Ambassador_rev.h"
#else
#include "FS:CrossDOSFileSystem_rev.h"
#endif
#include "internal/crossdos.h"
#endif
#ifdef   QFS
#include "QwikFileSystem_rev.h"
#endif

#ifndef NOPROTO
#include "clib/exec_protos.h"
#include "clib/dos_protos.h"
#include "clib/icon_protos.h"
#include "clib/intuition_protos.h"
#include "clib/utility_protos.h"    /* use the protos but not the pragmas */
#include "clib/alib_protos.h"       /* use the protos but not the pragmas */
#ifdef V33
#define LIB_VERS 33
#include "clib_ext/exec_protos.h"
#include "clib_ext/dos_protos.h"
#include "clib_ext/icon_protos.h"
#include "clib_ext/intuition_protos.h"
#include "clib_ext/utility_protos.h"

#undef  Fault
#define Fault Fault_FS_V36
BOOL Fault_FS_V36( long code, STRPTR header, STRPTR buffer, long len );
#endif

#include "pragmas/exec_sysbase_pragmas.h"
//#include "pragmas/exec_pragmas.h"
#include "pragmas/dos_pragmas.h"
#include "pragmas/icon_pragmas.h"
#include "pragmas/intuition_pragmas.h"
#endif

#ifndef LIB_VERS
#define LIB_VERS    37  /* set default to V2.04 AmigaDOS */
#endif

extern struct ExecBase  *SysBase;

#include "math.h"
#include "stdio.h"
#include "string.h"

#include "debug.h"

#ifdef DEBUG
#define DPRTF(x)  (x)
#else
#define DPRTF(x)
#endif

#define GETCLUSTER(cluster,dirent)  (GetBlock(ConvertCluster(cluster,dirent)))
#define PUTCLUSTER(cluster,dirent)  (PutBlock(ConvertCluster(cluster,dirent)))

#define PKT_WINDOW  (struct Window *)((fsys->f_pkt)? \
            (((struct Process *)fsys->f_pkt->dp_Port->mp_SigTask)->pr_WindowPtr): \
            (void *)0 )
#define FSSM        FileSysStartupMsg

#define MKBPTR(x)       ((BPTR)((long)(x) >> 2))

/* New DOS signal define */
#define SIGB_KILLPARENT  16
#define SIGF_KILLPARENT  1<<(SIGB_KILLPARENT)


/* Create function redefines if translate mode needed */
#define Trans_A_M(c,table)    (table? table->tbl_AtoM[(c)]: (c))
#define Trans_M_A(c,table)    (table? table->tbl_MtoA[(c)]: (c))

/* Table parameter the same as in the CrossDOSTrans structure in internal/crossdos.h */
struct trans_table
{
    UBYTE tbl_AtoM[256];     /* translate table Amiga to MSDOS 256 bytes */
    UBYTE tbl_MtoA[256];     /* translate table MSDOS to Amiga 256 bytes */
};

#define TRANSLATE_ASCII7    0
#define TRANSLATE_DEFAULT   TRANSLATE_ASCII7
#define TRANSLATE_TBL_SZ    512     /* size of translate table */

/* temp string buffer size */
#define TSTRG_BUFF_SZ 100
#define TSTRG1_BUFF_SZ 80

/* INTEL Byte swapping necessary */
/* It is assumed that x and y are UWORDs for SWAPWORD() and ULONGs for SWAPLONG() */
#define CBYTE(x,xn,y,yn)    ((UBYTE *)&(x))[xn] = ((UBYTE *)&(y))[yn]
#define SWAPWORD(x,y)   CBYTE(x,0,y,1); CBYTE(x,1,y,0);
#define SWAPLONG(x,y)   CBYTE(x,0,y,3); CBYTE(x,1,y,2); CBYTE(x,2,y,1); CBYTE(x,3,y,0);

#define TRK00           0

#define MD_NAME         "mfm.device"
#define MD_NUMSECS       9
#define MD_NUMHEADS      2
#define MD_SECTOR       TD_SECTOR
#define MSDOS_SEC_OFFSET 1      /* MSDOS sector  # offset */
#define TRACKRAWSIZE    ((250000/8)/5)*2+1500
     /* ((Bits per second [MFM] / bits per byte) / revs per sec )
        * MFM factor [1 MFM bit + 1 data bit] + slop for slow speed */
                        /* Maximum allowable buffer (32K) */
/*------ New MFM disk error codes */
#define MDERR_OutofTracks   36      /* Out of physical tracks */
#define MDERR_InvParam      37      /* Invalid parameter (MDCMD_SETPARMS) */
#define MDERR_IndexNotSync  38      /* Index mark not synced to index pulse */
#define MDERR_WrongTrack    39      /* Drive head on wrong track from where it should be. */

/*------ New Lock types for KS V36 */
#define READ_LOCK   -3
#define WRITE_LOCK  -4

  /* InfoData DiskType ID */
/*  Already defined in V37.4 headers
#define ID_MSDOS_DISK   (((ULONG)'M'<<24) | ((ULONG)'S'<<16) | ('D'<<8) | ('\0'))
*/
#define ID_MSDOS_DISK_HD  (((ULONG)'M'<<24) | ((ULONG)'S'<<16) | ('H'<<8) | ('\0'))
#define ID_MSDOS_DISK_DS  (((ULONG)'M'<<24) | ((ULONG)'D'<<16) | ('D'<<8) | ('\0'))

#define ID_QFS_DISK     (((ULONG)'Q'<<24) | ((ULONG)'F'<<16) | ('S'<<8) | ('\0'))

#define ID_BUSY_DISK    (((ULONG)'B'<<24) | ((ULONG)'U'<<16) | ('S'<<8) | ('Y'))

#define CHECK_DOSTYPE(x) ((x == ID_MSDOS_DISK) || (x == ID_MSDOS_DISK_HD) || (x == ID_MSDOS_DISK_DS))


/** These are changes to the DosEnvec struct names to assign new value names to
    little or never used vector variables */
#define de_BlocksPerSector  de_SectorPerBlock
#define de_BlocksPerCluster de_PreAlloc
#define de_DiskLowBlock     de_SecOrg
#define de_DiskHighBlock    de_Interleave


    /* New DOS error codes */
#define ERROR_UNKNOWN_DISK_FORMAT   254
#define ERROR_CODE_MAX  256

#define MOTOR_ON        1
#define MOTOR_OFF       0

#define MDB_SETPARMS 2
#define MDF_SETPARMS (1<<MDB_SETPARMS)
#define MDCMD_SETPARMS      29          /* command # after HD_SCSICMD */

struct FS_parms      /* FS disk parameters */
{
    UWORD   mp_root_dir_ent;        /* # root directory entries (e.g. 64 or 112) */
    ULONG   mp_sectors_part;        /* # sectors per disk/partition */
    UBYTE   mp_format_id;           /* disk format ID # (e.g. F8, F9 or FC-FF) */
    UBYTE   mp_sectors_fat;         /* # sectors per FAT (e.g. 1 or 2) */
    UWORD   mp_sectors_track;       /* # sectors per track (e.g. 8 or 9) */
    UBYTE   mp_sides;               /* # sides (e.g. 1 or 2) */
    UBYTE   mp_sectors_clust;       /* # sectors per cluster (e.g. 2) */
};

/** Format types **/
#define FMT_AUTO       0  /* FMT_AUTO */
#define FMT_MSDOS_QD9  1  /* FMT_MSDOS_QD9 */
#define FMT_MSDOS_D9   2  /* FMT_MSDOS_D9  */
#define FMT_MSDOS_S9   3  /* FMT_MSDOS_S9  */
#define FMT_ATARI_D9   4  /* FMT_ATARI_D9  */
#define FMT_ATARI_S9   5  /* FMT_ATARI_S9  */
#define FMT_MSDOS_QD18 6  /* FMT_MSDOS_QD18 */

#define FMT_MQD9  (ULONG)(0x4D514439)  /*("MQD9")   FMT_MSDOS_QD9 */
#define FMT_MD9   (ULONG)(0x4D443900)  /*("MD9")    FMT_MSDOS_D9  */
#define FMT_MS9   (ULONG)(0x4D533900)  /*("MS9")    FMT_MSDOS_S9  */
#define FMT_AD9   (ULONG)(0x41443900)  /*("AD9")    FMT_ATARI_D9  */
#define FMT_AS9   (ULONG)(0x41533900)  /*("AS9")    FMT_ATARI_S9  */
#define FMT_MQD18 (ULONG)(0x4D513138)  /*("MQ18")   FMT_MSDOS_QD18 */
#define FMT_FORCE (ULONG)(0x464F5243)  /*("FORC")   FMT_FORCE */

/*** Default boot block parameter settings ***/
#define MP_sectors_cluster  2       /* # sectors per cluster (e.g. 01 or 02 */
#define MP_blocks_sec       1       /* blocks per sector */
#define MP_bytes_block      512     /* bytes per block */
#define MP_res_sectors_beg  1       /* # reserved sectors at beginning */
#define MP_copies_fat       2       /* # copies of FAT */
#define MP_res_sectors      0       /* # reserved sectors (e.g. 0) */

#define VOLNAME_SIZE  11            /* MSDOS volume name size */

struct FS_boot_blk                 /* MSDOS Boot block */
{
    UBYTE   fbb_jmp_add[3];         /* boot jump address displacement */
    UBYTE   fbb_sys_id[8];          /* system ID not null terminated */
    UBYTE   fbb_bytes_sector[2];    /* # bytes per sector (e.g. 512) */
    UBYTE   fbb_sectors_cluster;    /* # sectors per cluster (e.g. 01 or 02 */
    UBYTE   fbb_res_sectors_beg[2]; /* # reserved sectors at beginning (e.g. 1) */
    UBYTE   fbb_copies_fat;         /* # copies of FAT (e.g. 2) */
    UBYTE   fbb_root_dir_ent[2];    /* # root directory entries (e.g. 64 or 112) */
    UBYTE   fbb_sectors_disk[2];    /* # sectors per disk */
    UBYTE   fbb_format_id;          /* disk format ID # (e.g. 0xF8, 0xF9 or 0xFC-0xFF) */
    UBYTE   fbb_sectors_fat[2];     /* # sectors per FAT (e.g. 1 or 2) */
    UBYTE   fbb_sectors_track[2];   /* # sectors per track (e.g. 8 or 9) */
    UBYTE   fbb_sides[2];           /* # sides (e.g. 1 or 2) */
    UBYTE   fbb_hidden_sectors[4];  /* # hidden (?) sectors (e.g. 0) */
    UBYTE   fbb_sectors_disk_32[4]; /* # sectors per disk when fbb_sectors_disk > 65536 (MSDOS 4+) */
    UBYTE   fbb_drive_num;          /* drive #. (currently only 80h or 0)(MSDOS 4+) */
    UBYTE   fbb_res0;               /* reserved (MSDOS 4+) */
    UBYTE   fbb_ext_boot_signature; /* extended boot signature (always 0x29) (MSDOS 4+) */
    UBYTE   fbb_serial_num[4];      /* volume serial # (MSDOS 4+) */
    UBYTE   fbb_volume_name[VOLNAME_SIZE];    /* volume name (MSDOS 4+) */
    UBYTE   fbb_FAT_type[8];        /* FAT type string (MSDOS 4+) */
    UBYTE   fbb_bootcode[1];        /* MSDOS partition boot code (MSDOS 4+) */
};


struct FS_HD_part_tbl                 /* MSDOS Hard disk partition table */
{
    UBYTE   fhdp_part_stat;         /* HD partition status */
    UBYTE   fhdp_start_head;        /* HD partition start head */
    UBYTE   fhdp_start_sec_cyl[2];  /* HD partition start cylinder and sector # */
    UBYTE   fhdp_part_type;         /* HD partition type */
    UBYTE   fhdp_end_head;          /* HD partition end head */
    UBYTE   fhdp_end_sec_cyl[2];    /* HD partition end cylinder and sector # */
    UBYTE   fhdp_start_sec[4];      /* HD partition start sector */
    UBYTE   fhdp_num_secs[4];       /* HD partition # of sectors */
};

struct FS_HD_blk        /* MSDOS HD partition block */
{
    UBYTE   fhdb_pad[0x1BE];        /* pad bytes to offset to begin of HD part table */
    struct  FS_HD_part_tbl fhdb_p_tbl[4];  /* HD partition table [4 entries] */
    UWORD   fhdb_signature;         /* HD partition block signature (=0x55AA) */

};

/*********** NOTE **********
Block # references in this structure refer to the ABSOLUTE block # to the
beginning of the disk.
*********** NOTE **********/
struct FS                    /* FS environment params */
{
    struct Task    *f_Task;         /* ptr to FileSystem task */
  /* Volume related params */
    struct FSSM    *f_fssm;         /* ptr to FileSysStartupMsg */
    struct DosList *f_VolumeNode;   /* ptr to current Volume Node */
  /* Packet related params */
    struct DosPacket *f_pkt;        /* ptr to current DOS packet being processed */
    BPTR    f_LockList;             /* BPTR to LockList for current volume */
    struct DeviceNode *f_DevNode;   /* ptr to current Device Node */

    BYTE    f_inhibit_cnt;          /* INHIBIT count */
    UBYTE   f_FS_special_flags;     /* File System (special) related flags */
    UBYTE   f_IOR_pend;             /* # IORequests pending return */
    UBYTE   f_FSflags;              /* File System related flags */
    UBYTE   f_Diskflags;            /* Disk related flags */

    UBYTE   f_NumSoftErrors;        /* # of soft errors on disk */

  /* Timer related params */
    BYTE    f_timer_cnt;            /* timer counter */
    BYTE    f_timer_start;          /* start timer */
    struct  TMRReq *f_timer;        /* Timer IORequest struct */

  /* Disk and block related params */
    UBYTE   f_sys_id[16];           /* system ID BSTR and null terminated (disk volume name) */
    struct DateStamp f_vol_date;    /* disk volume date */

    UWORD   f_bytes_block_sh;       /* # bytes per block shift value (e.g. 512 bytes = 9 bit shift) */
    ULONG   f_bytes_block;          /* # bytes per block (e.g. 512) */
    ULONG   f_maxblocks_transfer;   /* maximum # blocks per transfer; de_MaxTransfer/bytes_block */
    ULONG   f_mem_mask;             /* memory mask  for direct transfer for the device driver */
    ULONG   f_blocks_part;          /* # blocks per partition */

  /* FAT related params */
    UWORD   f_blocks_fat;           /* # blocks per FAT (e.g. 1 or 2) */
    UBYTE   f_blocks_sector;        /* # blocks per sector (e.g. 1) */
    UBYTE   f_copies_fat;           /* # copies of FAT (e.g. 2) */
    UBYTE   *f_fat_copy;            /* copy of FAT in memory */
    ULONG   f_fat_copy_sz;          /* copy of FAT in memory size */
    ULONG   f_ffFATent;             /* first free FAT entry (cluster) */
    ULONG   f_beg_fat1;             /* beginning block of first fat */
    ULONG   f_beg_fat2;             /* beginning block of second fat */

  /* Root Directory related params */
    UWORD   f_dirents_block;        /* # directory entries per block */
    UWORD   f_dirents_clust;        /* # directory entries per clust */
    UWORD   f_root_blocks;          /* # blocks of the root dir */
    UWORD   f_res01;                /* reserved */
    ULONG   f_root_dir_ent;         /* # of root dir entries */
    ULONG   f_beg_root_dir;         /* beginning block of root directory */

    UBYTE   *f_rootdir_cache;       /* root directory cache memory */
    ULONG   f_rootdir_cache_sz;     /* root directory cache memory size */

  /* Data cluster related params */
    UBYTE   f_blocks_clust;         /* # blocks per cluster (e.g. 2) */
    UBYTE   f_num_buffs;            /* # of cache buffers */
    UWORD   f_old_buff;             /* # of oldest cache buffer (for recycle) */
    struct Cache *f_cache;          /* ptr to cache struct */
    ULONG   f_cache_sz;             /* cache struct size */
    ULONG   f_beg_disk;             /* beginning block # of disk */
    ULONG   f_end_disk;             /* end block # of disk */
    ULONG   f_beg_part;             /* beginning block # of partition */
    ULONG   f_beg_data;             /* beginning block # of data */
    ULONG   f_used_blocks;          /* # of used blocks */
    UWORD   f_res10;                /* reserved */
    UWORD   f_end_clust;            /* end cluster # of data */

  /* Disk change interrupt related params */
    ULONG   f_dci_sig;              /* diskchange interrupt signal mask */
    struct  DCIReq *f_dci;          /* diskchange interrupt struct */

    struct CrossDOSLock *f_sema4;   /* ptr to sema4 node */
    struct CrossDOSHandler *f_sema4_hndlr; /* ptr to sema4 handler node */

    UBYTE   *f_scratch;             /* 'scratch' memory */
    ULONG   f_scratch_sz;           /* 'scratch' memory size */

    UWORD   f_surfaces;             /* # of logical surfaces of the disk (from BB) */
    UWORD   f_blockspertrack;       /* # of logical blocks per track of the disk (from BB) */

    ULONG   f_wp_passkey;           /* Write Protect pass key */
    ULONG   f_curr_part_tbl;        /* current partition table that points to the desired partition */
};


/* f_FS_special_flags -- File System (special) related flags */
#define MB_ADD_VOL      0               /* add volume node flag */
#define MB_REM_VOL      1               /* remove volume node flag */
#define MB_GG_DEVICE    6               /* device driver supports MD_GETGEOMETRY cmd */
#define MB_SP_DEVICE    7               /* device driver supports MD_SETPARMS cmd */

#define MF_ADD_VOL      (UBYTE)(1<<MB_ADD_VOL)
#define MF_REM_VOL      (UBYTE)(1<<MB_REM_VOL)
#define MF_GG_DEVICE    (UBYTE)(1<<MB_GG_DEVICE)
#define MF_SP_DEVICE    (UBYTE)(1<<MB_SP_DEVICE)

/* f_FSflags -- File System related flags */
#define MB_VALIDATED    0               /* DISK VALIDATED flag */
#define MB_NOTREADABLE  1               /* NOT a READABLE DISK in drive flag */
#define MB_NODISK       2               /* NO DISK in drive flag */
#define MB_FSDISK       3               /* File System DISK in drive flag */
#define MB_GLOB_TXTRANS 4               /* Global text translator flag */
#define MB_GLOB_TXFLTR  5               /* Global text filter (^M^Z) flag */
#define MB_WP_DEVICE    6               /* Write Protect device flag */

#define MF_VALIDATED    (UBYTE)(1<<MB_VALIDATED)
#define MF_NOTREADABLE  (UBYTE)(1<<MB_NOTREADABLE)
#define MF_NODISK       (UBYTE)(1<<MB_NODISK)
#define MF_FSDISK       (UBYTE)(1<<MB_FSDISK)
#define MF_GLOB_TXTRANS (UBYTE)(1<<MB_GLOB_TXTRANS)
#define MF_GLOB_TXFLTR  (UBYTE)(1<<MB_GLOB_TXFLTR)
#define MF_WP_DEVICE    (UBYTE)(1<<MB_WP_DEVICE)

/* f_Diskflags -- Disk related flags */
#define WRITE_PROT_B    0              /* disk write protected flag */
#define CREATE_ABOOT_B  1              /* create an ABOOT sector flag */
#define ABOOT_DISK_B    2              /* disk is an ABOOT type flag */
#define ROOTDIR_MOD_B   3              /* root directory modified flag */
#define CACHE_MOD_B     4              /* cache modified flag */
#define DISK_MOD_B      5              /* disk modified flag */
#define FAT_MOD_B       6              /* FAT Modified flag */
#define FAT16_B         7              /* 16 bit FAT flag */

#define WRITE_PROT_F    (UBYTE)(1<<WRITE_PROT_B)
#define CREATE_ABOOT_F  (UBYTE)(1<<CREATE_ABOOT_B)
#define ABOOT_DISK_F    (UBYTE)(1<<ABOOT_DISK_B)
#define ROOTDIR_MOD_F   (UBYTE)(1<<ROOTDIR_MOD_B)
#define CACHE_MOD_F     (UBYTE)(1<<CACHE_MOD_B)
#define DISK_MOD_F      (UBYTE)(1<<DISK_MOD_B)
#define FAT_MOD_F       (UBYTE)(1<<FAT_MOD_B)
#define FAT16_F         (UBYTE)(1<<FAT16_B)

#define MIN_CACHE_BUFFS 3
#define MAX_CACHE_BUFFS 255

struct Cache            /* Cache buffer pointer structure */
{
    ULONG   cache_block;            /* cluster # of cache */
    UBYTE   *cache_buff;            /* ptr to cache buffer */
};

#define C_BLOCK_MOD       (ULONG)(1<<31)    /* cluster modified flag */
#define C_BLOCK_UNUSED    ~(C_BLOCK_MOD)    /* cluster # to indicate used cluster in cache */
#define C_DBUFF_UNUSED    ~0                /* buff # to indicate used data buff in cache */


#define FNAME_SIZE  8                   /* MSDOS file name size */
#define FEXT_SIZE   3                   /* MSDOS file extension size */
#define FNAME_EXT_SIZE  (FNAME_SIZE+FEXT_SIZE)  /* MSDOS file name + extension size */
#define FNAME_TOTALSZ   FNAME_EXT_SIZE  /* MSDOS file name + extension size */

struct FS_dir_ent                    /* FS directory entry fields */
{
    UBYTE   fde_filename[FNAME_SIZE]; /* filename (or subdirectory) */
    UBYTE   fde_file_ext[FEXT_SIZE]; /* file extention */
    UBYTE   fde_protection;         /* file attributes */
    UBYTE   fde_res[10];            /* reserved for future use */
    UBYTE   fde_time[2];            /* file time */
    UBYTE   fde_date[2];            /* file date */
    UBYTE   fde_start_clust[2];     /* file starting cluster */
    UBYTE   fde_file_size[4];       /* file size (in bytes) */
};


/* filename specifiers */
#define FN_END          (UBYTE)'\0'     /* end of directory entries (DOS 2+) */
#define FN_ERASE        (UBYTE)'\xE5'   /* erased directory entry */
#define FN_DIR          (UBYTE)'.'      /* directory mark  = '.' */
#define FN_CURRDIR      (UBYTE)0x2E     /* Current sub-directory = '.' */
#define FN_PARENTDIR    (UWORD)0x2E2E   /* Parent directory '..'*/

/* file attributes defs */
#define ATTB_READONLY   (UBYTE)0
#define ATTB_HIDDEN     (UBYTE)1
#define ATTB_SYSTEM     (UBYTE)2
#define ATTB_VOL_LBL    (UBYTE)3
#define ATTB_SUBDIR     (UBYTE)4
#define ATTB_ARCHIVE    (UBYTE)5    /* Thi flag is opposite to the Amiga archive
                                        attribute. In MS-DOS, when this flag is set
                                        it means the file is TO BE archived. In
                                        Amiga, when set it means the file IS archived */

#define ATTF_READONLY   (UBYTE)(1<<ATTB_READONLY)
#define ATTF_HIDDEN     (UBYTE)(1<<ATTF_HIDDEN)
#define ATTF_SYSTEM     (UBYTE)(1<<ATTB_SYSTEM)
#define ATTF_VOL_LBL    (UBYTE)(1<<ATTB_VOL_LBL)
#define ATTF_SUBDIR     (UBYTE)(1<<ATTB_SUBDIR)
#define ATTF_ARCHIVE    (UBYTE)(1<<ATTB_ARCHIVE)

#define TO_BE_ARCHIVED_FLAG    ATTF_ARCHIVE
#define DELETE_FLAG     ATTF_READONLY

/* Time masks */
#define TIMEM_HOURS     (UWORD)0xF800          /* hours mask */
#define TIMEM_MINS      (UWORD)0x07E0          /* minutes mask */
#define TIMEM_2SECS     (UWORD)0x001F          /* 2 seconds mask */

/* Time bit positions */
#define TIMEB_HOURS     (UWORD)11              /* shift left to multiply */
#define TIMEB_MINS      (UWORD)5               /* shift left to multiply */
#define TIMEB_2SECS     (UWORD)1               /* shift right to divide */

/* Date masks */
#define DATEM_YEAR      (UWORD)0xFE00          /* Year mask (Year-1980) */
#define DATEM_MONTH     (UWORD)0x01E0          /* Month mask */
#define DATEM_DAY       (UWORD)0x001F          /* Day mask */

/* Date bit positions */
#define DATEB_YEAR      (UWORD)9              /* shift left to multiply */
#define DATEB_MONTH     (UWORD)5              /* shift left to multiply */
#define DATEB_DAY       (UWORD)0              /* shift left to multiply */

/* FAT defines */
#define FAT_FREE        (UWORD)0
#define FAT_BAD         (UWORD)0xFFF7
#define FAT_EOF_LOW     (UWORD)0xFFF8
#define FAT_EOF         (UWORD)0xFFFF
#define FAT12_MASK      (UWORD)0x0FFF
#define FAT16_MASK      (UWORD)0xFFFF


/************
    LOCKS
*************/

struct Lock_ext                 /* FS lock extension struct */
{
    LONG    le_curr_clust;          /* current cluster # of file */
    LONG    le_prev_clust;          /* previous cluster # of file from current cluster # */
    LONG    le_file_pos;            /* current position in file */
    ULONG   le_databuffnum;         /* Data buffer number in cache */
    UBYTE   le_filt_flags;          /* Filter Flag bits for file processing */
    UBYTE   le_res0[3];             /* reserved */
    struct trans_table *le_trans_tbl; /* Translation table pointer for file processing */
                                    /* This node structure used in a circular list */
};

struct KeyPtr                       /* substitute for fl_Key */
{
    LONG    kp_lock_count;          /* count of locks using this KeyPtr */
    LONG    kp_dir_clust;           /* cluster # of the directory of the entry */
    LONG    kp_dir_ent;             /* directory entry # */
    LONG    kp_file_size;           /* End of file (in bytes) */
    LONG    kp_start_clust;         /* starting cluster # of file */
    BSTR    *kp_file_name;          /* file name. Do not use this unless it is a dummy file */
    UBYTE   kp_flags;               /* Flag bits for file processing */
    UBYTE   kp_res0;                /* reserved */
};


/* Redefine struct Lock */
struct MLock                    /* FS lock struct with extensions */
{
    BPTR                ml_Link;        /* bcpl pointer to next lock */
    struct KeyPtr       *ml_KeyPtr;     /* disk Key Ptr struct */
    LONG                ml_Access;      /* exclusive or shared */
    struct MsgPort      *ml_Task;       /* handler task's port */
    BPTR                ml_Volume;      /* bptr to DLT_VOLUME DosList entry */
    struct  Lock_ext    ml_lock_ext;    /* lock extensions */
};


/* FileLock Cluster defines */
#define ROOTDIR_CLUST   0       /* starting root dir cluster number */
#define ILLEGAL_CLUST   1       /* illegal cluster number */
#define START_CLUST     2       /* starting data cluster number */

/* FileLock Filter Flags */
#define CTRLM_CTRLZ_B   0       /* bit # of ctrl M and ctrl Z filter flag */
#define XLAT1_B      1          /* bit # of translator (type 1) flag */
#define JLINK_B      7          /* bit # of Jlink boot record translator flag */

#define CTRLM_CTRLZ_F   (1<<CTRLM_CTRLZ_B)  /* ctrl M and ctrl Z filter flag */
#define XLAT1_F         (1<<XLAT1_B)        /* translator (type 1)  flag */
#define JLINK_F         (1<<JLINK_B)        /* bit # of Jlink boot record translator flag */

#define CTRLJ           ((char) 'J' - 0x40)
#define CTRLM           ((char) 'M' - 0x40)
#define CTRLZ           ((char) 'Z' - 0x40)
#define ASCII_7         0x7F


/* FileLock Flags */
#define FILE_MOD_B      0       /* bit # of file modified flag */
#define FILE_WRITE_PROT_B 1       /* bit # of file write protect flag */

#define FILE_MOD_F   (1<<FILE_MOD_B)
#define FILE_WRITE_PROT_F  (1<<FILE_WRITE_PROT_B)


/************
    FILEINFOBLOCKS
*************/

struct MFileInfoBlock   /* modified FileInfoBlock */
{
    LONG    mfib_DiskKey;
    LONG    mfib_DirEntryType;

    char    mfib_FileName[32];  /* only 16 chars are needed for MSDOS filenames */
    LONG    mfib_DiskCluster;   /* starting cluster of the root of the file or dir */
    LONG    mfib_DiskDirEntry;  /* directory entry  # */
    LONG    mfib_start_clust;   /* starting cluster of the subdirs or files */
    UBYTE   mfib_FileFlags;     /* dir or file flags */
    BYTE    mfib_DirEntStatus;  /* Directory entry status; like mfib_DirEntryType */

    char    mfib_Oname[FNAME_TOTALSZ];     /* Object name as found in a directory entry */
    UBYTE   mfib_pad1[62-(FNAME_TOTALSZ)]; /* pad */

    LONG    mfib_Protection;
    LONG    mfib_EntryType;
    LONG    mfib_Size;
    LONG    mfib_NumBlocks;
    struct  DateStamp  mfib_Date;
    char    mfib_Comment[80];
    char    mfib_Reserved[36];
};

struct MinFileInfoBlock   /* minimum modified FileInfoBlock */
{
    LONG    mmfib_DiskKey;
    LONG    mmfib_DirEntryType;

    char    mmfib_FileName[32];  /* only 16 chars are needed for MSDOS filenames */
    LONG    mmfib_DiskCluster;   /* starting cluster of the root of the file or dir */
    LONG    mmfib_DiskDirEntry;  /* directory entry  # */
    LONG    mmfib_start_clust;   /* starting cluster of the subdirs or files */
    UBYTE   mmfib_FileFlags;     /* dir or file flags */
    BYTE    mmfib_DirEntStatus;  /* Directory entry status; like mfib_DirEntryType */

    char    mmfib_Oname[FNAME_TOTALSZ];     /* Object name as found in a directory entry */
    UBYTE   mmfib_pad1[62-(FNAME_TOTALSZ)]; /* pad */

    LONG    mmfib_Protection;
    LONG    mmfib_EntryType;
    LONG    mmfib_Size;
};

/** Dir Entry Status Types **/
#define MLDE_DIR_ROOT    3          /* Root of Directory type (not necessarily root of disk) */
#define MLDE_FILE        -3         /* File type */
#define MLDE_DIR         2          /* Directory type */
#define MLDE_EMPTY       0          /* EMPTY entry type */
#define MLDE_END         -1         /* END entry type */
#define MLDE_VOL_LBL     -2         /* Volume Label entry type */


struct DCIReq             /* Diskchange Interrupt IORequest struct */
{
    struct IOStdReq     dci_req;
    struct Interrupt    dci_int;
};

struct TMRReq             /* Timer IORequest struct with DOSPacket I/F */
{
    struct timerequest  tmr_req;
    struct DosPacket    tmr_pkt;
};
#define DISK_TIME_OUT   2   /* Max. Disk time out value */

struct VolNode          /* Volume Node */
{
    struct DeviceList  vn_DeviceList;
    UBYTE vn_VolName[32];
};


#ifdef AMB
/**********************************************************************
*   Ambassador functions and defines
**********************************************************************/
#define ABOOT_NUMBLOCK  1
#endif

#include "FS:FS_protos.h"
