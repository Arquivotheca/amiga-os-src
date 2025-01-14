#ifndef	DEVICES_HARDBLOCKS_H
#define	DEVICES_HARDBLOCKS_H
/*
**	$Header: hardblocks.h,v 36.2 89/11/22 15:37:49 jesup Exp $
**
**	File System identifier blocks for hard disks
**
**	(C) Copyright 1988,1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

/*--------------------------------------------------------------------
 *
 *	This file describes blocks of data that exist on a hard disk
 *	to describe that disk.  They are not generically accessable to
 *	the user as they do not appear on any DOS drive.  The blocks
 *	are tagged with a unique identifier, checksummed, and linked
 *	together.  The root of these blocks is the RigidDiskBlock.
 *
 *	The RigidDiskBlock must exist on the disk within the first
 *	RDB_LOCATION_LIMIT blocks.  This inhibits the use of the zero
 *	cylinder in an AmigaDOS partition: although it is strictly
 *	possible to store the RigidDiskBlock data in the reserved
 *	area of a partition, this practice is discouraged since the
 *	reserved blocks of a partition are overwritten by "Format",
 *	"Install", "DiskCopy", etc.  The recommended disk layout,
 *	then, is to use the first cylinder(s) to store all the drive
 *	data specified by these blocks: i.e. partition descriptions,
 *	file system load images, drive bad block maps, spare blocks,
 *	etc.
 *
 *	Though only 512 byte blocks are currently supported by the
 *	file system, this proposal tries to be forward-looking by
 *	making the block size explicit, and by using only the first
 *	256 bytes for all blocks but the LoadSeg data.
 *
 *------------------------------------------------------------------*/

/*
 *  NOTE
 *	optional block addresses below contain $ffffffff to indicate
 *	a NULL address, as zero is a valid address
 */
struct RigidDiskBlock {
    ULONG   rdb_ID;		/* 4 character identifier */
    ULONG   rdb_SummedLongs;	/* size of this checksummed structure */
    LONG    rdb_ChkSum;		/* block checksum (longword sum to zero) */
    ULONG   rdb_HostID;		/* SCSI Target ID of host */
    ULONG   rdb_BlockBytes;	/* size of disk blocks */
    ULONG   rdb_Flags;		/* see below for defines */
    /* block list heads */
    ULONG   rdb_BadBlockList;	/* optional bad block list */
    ULONG   rdb_PartitionList;	/* optional first partition block */
    ULONG   rdb_FileSysHeaderList; /* optional file system header block */
    ULONG   rdb_DriveInit;	/* optional drive-specific init code */
				/* DriveInit(lun,rdb,ior): "C" stk & d0/a0/a1 */
    ULONG   rdb_Reserved1[6];	/* set to $ffffffff */
    /* physical drive characteristics */
    ULONG   rdb_Cylinders;	/* number of drive cylinders */
    ULONG   rdb_Sectors;	/* sectors per track */
    ULONG   rdb_Heads;		/* number of drive heads */
    ULONG   rdb_Interleave;	/* interleave */
    ULONG   rdb_Park;		/* landing zone cylinder */
    ULONG   rdb_Reserved2[3];
    ULONG   rdb_WritePreComp;	/* starting cylinder: write precompensation */
    ULONG   rdb_ReducedWrite;	/* starting cylinder: reduced write current */
    ULONG   rdb_StepRate;	/* drive step rate */
    ULONG   rdb_Reserved3[5];
    /* logical drive characteristics */
    ULONG   rdb_RDBBlocksLo;	/* low block of range reserved for hardblocks */
    ULONG   rdb_RDBBlocksHi;	/* high block of range for these hardblocks */
    ULONG   rdb_LoCylinder;	/* low cylinder of partitionable disk area */
    ULONG   rdb_HiCylinder;	/* high cylinder of partitionable data area */
    ULONG   rdb_CylBlocks;	/* number of blocks available per cylinder */
    ULONG   rdb_AutoParkSeconds; /* zero for no auto park */
    ULONG   rdb_Reserved4[2];
    /* drive identification */
    char    rdb_DiskVendor[8];
    char    rdb_DiskProduct[16];
    char    rdb_DiskRevision[4];
    char    rdb_ControllerVendor[8];
    char    rdb_ControllerProduct[16];
    char    rdb_ControllerRevision[4];
    ULONG   rdb_Reserved5[10];
};

#define	IDNAME_RIGIDDISK	0x5244534B	/* 'RDSK' */

#define	RDB_LOCATION_LIMIT	16

#define	RDBFB_LAST	0	/* no disks exist to be configured after */
#define	RDBFF_LAST	0x01L	/*   this one on this controller */
#define	RDBFB_LASTLUN	1	/* no LUNs exist to be configured greater */
#define	RDBFF_LASTLUN	0x02L	/*   than this one at this SCSI Target ID */
#define	RDBFB_LASTTID	2	/* no Target IDs exist to be configured */
#define	RDBFF_LASTTID	0x04L	/*   greater than this one on this SCSI bus */
#define	RDBFB_NORESELECT 3	/* don't bother trying to perform reselection */
#define	RDBFF_NORESELECT 0x08L	/*   when talking to this drive */
#define	RDBFB_DISKID	4	/* rdb_Disk... identification valid */
#define	RDBFF_DISKID	0x10L
#define	RDBFB_CTRLRID	5	/* rdb_Controller... identification valid */
#define	RDBFF_CTRLRID	0x20L

/*------------------------------------------------------------------*/
struct BadBlockEntry {
    ULONG   bbe_BadBlock;	/* block number of bad block */
    ULONG   bbe_GoodBlock;	/* block number of replacement block */
};

struct BadBlockBlock {
    ULONG   bbb_ID;		/* 4 character identifier */
    ULONG   bbb_SummedLongs;	/* size of this checksummed structure */
    LONG    bbb_ChkSum;		/* block checksum (longword sum to zero) */
    ULONG   bbb_HostID;		/* SCSI Target ID of host */
    ULONG   bbb_Next;		/* block number of the next BadBlockBlock */
    ULONG   bbb_Reserved;
    struct BadBlockEntry bbb_BlockPairs[61]; /* bad block entry pairs */
    /* note [61] assumes 512 byte blocks */
};

#define	IDNAME_BADBLOCK		0x42414442	/* 'BADB' */

/*------------------------------------------------------------------*/
struct PartitionBlock {
    ULONG   pb_ID;		/* 4 character identifier */
    ULONG   pb_SummedLongs;	/* size of this checksummed structure */
    LONG    pb_ChkSum;		/* block checksum (longword sum to zero) */
    ULONG   pb_HostID;		/* SCSI Target ID of host */
    ULONG   pb_Next;		/* block number of the next PartitionBlock */
    ULONG   pb_Flags;		/* see below for defines */
    ULONG   pb_Reserved1[2];
    ULONG   pb_DevFlags;	/* preferred flags for OpenDevice */
    UBYTE   pb_DriveName[32];	/* preferred DOS device name: BSTR form */
				/* (not used if this name is in use) */
    ULONG   pb_Reserved2[15];	/* filler to 32 longwords */
    ULONG   pb_Environment[17];	/* environment vector for this partition */
    ULONG   pb_EReserved[15];	/* reserved for future environment vector */
};

#define	IDNAME_PARTITION	0x50415254	/* 'PART' */

#define	PBFB_BOOTABLE	0	/* this partition is intended to be bootable */
#define	PBFF_BOOTABLE	1L	/*   (expected directories and files exist) */
#define	PBFB_NOMOUNT	1	/* do not mount this partition (e.g. manually */
#define	PBFF_NOMOUNT	2L	/*   mounted, but space reserved here) */

/*------------------------------------------------------------------*/
struct FileSysHeaderBlock {
    ULONG   fhb_ID;		/* 4 character identifier */
    ULONG   fhb_SummedLongs;	/* size of this checksummed structure */
    LONG    fhb_ChkSum;		/* block checksum (longword sum to zero) */
    ULONG   fhb_HostID;		/* SCSI Target ID of host */
    ULONG   fhb_Next;		/* block number of next FileSysHeaderBlock */
    ULONG   fhb_Flags;		/* see below for defines */
    ULONG   fhb_Reserved1[2];
    ULONG   fhb_DosType;	/* file system description: match this with */
				/* partition environment's DE_DOSTYPE entry */
    ULONG   fhb_Version;	/* release version of this code */
    ULONG   fhb_PatchFlags;	/* bits set for those of the following that */
				/*   need to be substituted into a standard */
				/*   device node for this file system: e.g. */
				/*   0x180 to substitute SegList & GlobalVec */
    ULONG   fhb_Type;		/* device node type: zero */
    ULONG   fhb_Task;		/* standard dos "task" field: zero */
    ULONG   fhb_Lock;		/* not used for devices: zero */
    ULONG   fhb_Handler;	/* filename to loadseg: zero placeholder */
    ULONG   fhb_StackSize;	/* stacksize to use when starting task */
    LONG    fhb_Priority;	/* task priority when starting task */
    LONG    fhb_Startup;	/* startup msg: zero placeholder */
    LONG    fhb_SegListBlocks;	/* first of linked list of LoadSegBlocks: */
				/*   note that this entry requires some */
				/*   processing before substitution */
    LONG    fhb_GlobalVec;	/* BCPL global vector when starting task */
    ULONG   fhb_Reserved2[23];	/* (those reserved by PatchFlags) */
    ULONG   fhb_Reserved3[21];
};

#define	IDNAME_FILESYSHEADER	0x46534844	/* 'FSHD' */

/*------------------------------------------------------------------*/
struct LoadSegBlock {
    ULONG   lsb_ID;		/* 4 character identifier */
    ULONG   lsb_SummedLongs;	/* size of this checksummed structure */
    LONG    lsb_ChkSum;		/* block checksum (longword sum to zero) */
    ULONG   lsb_HostID;		/* SCSI Target ID of host */
    ULONG   lsb_Next;		/* block number of the next LoadSegBlock */
    ULONG   lsb_LoadData[123];	/* data for "loadseg" */
    /* note [123] assumes 512 byte blocks */
};

#define	IDNAME_LOADSEG		0x4C534547	/* 'LSEG' */

#endif	/* DEVICES_HARDBLOCKS_H */

