	IFND	DEVICES_HARDBLOCKS_I
DEVICES_HARDBLOCKS_I	SET	1
**
**	$Filename: devices/hardblocks.i $
**	$Release: 1.4 $
**	$Revision: 0.9 $
**	$Date: 88/06/20 12:58:03 $
**
**	File System identifier blocks for hard disks
**
**	(C) Copyright 1988 Kodiak Software
**

;---------------------------------------------------------------------
;
;	This file describes blocks of data that exist on a hard disk
;	to describe that disk.  They are not generically accessable to
;	the user as they do not appear on any DOS drive.  The blocks
;	are tagged with a unique identifier, checksummed, and linked
;	together.  The root of these blocks is the RigidDiskBlock.
;
;	The RigidDiskBlock must exist on the disk within the first
;	RDB_LOCATION_LIMIT blocks.  This inhibits the use of the zero
;	cylinder in an AmigaDOS partition: although it is strictly
;	possible to store the RigidDiskBlock data in the reserved
;	area of a partition, this practice is discouraged since the
;	reserved blocks of a partition are overwritten by "Format",
;	"Install", "DiskCopy", etc.  The recommended disk layout,
;	then, is to use the first cylinder(s) to store all the drive
;	data specified by these blocks: i.e. partition descriptions,
;	file system load images, drive bad block maps, spare blocks,
;	etc.
;
;	Though only 512 byte blocks are currently supported by the
;	file system, this proposal tries to be forward-looking by
;	making the block size explicit, and by using only the first
;	256 bytes for all blocks but the LoadSeg data.
;
;---------------------------------------------------------------------

;
;   NOTE
;	optional block addresses below contain $ffffffff to indicate
;	a NULL address
;
 STRUCTURE	RigidDiskBlock,0
    ULONG   rdb_ID		; 4 character identifier
    ULONG   rdb_SummedLongs	; size of this checksummed structure
    LONG    rdb_ChkSum		; block checksum (longword sum to zero)
    ULONG   rdb_HostID		; SCSI Target ID of host
    ULONG   rdb_BlockBytes	; size of disk blocks
    ULONG   rdb_Flags		; see below for defines
    ; block list heads
    ULONG   rdb_BadBlockList	; optional bad block list
    ULONG   rdb_PartitionList	; optional first partition block
    ULONG   rdb_FileSysHeaderList ; optional fule system header block
    ULONG   rdb_DriveInit	; optional drive-specific init code
    STRUCT  rdb_Reserved1,6*4	; set to $ffffffff
    ; physical drive characteristics
    ULONG   rdb_Cylinders	; number of drive cylinders
    ULONG   rdb_Sectors		; sectors per track
    ULONG   rdb_Heads		; number of drive heads
    ULONG   rdb_Interleave	; interleave
    ULONG   rdb_Park		; landing zone cylinder
    STRUCT  rdb_Reserved2,3*4
    ULONG   rdb_WritePreComp	; starting cylinder: write precompensation
    ULONG   rdb_ReducedWrite	; starting cylinder: reduced write current
    ULONG   rdb_StepRate	; drive step rate
    STRUCT  rdb_Reserved3,5*4
    ; logical drive characteristics
    ULONG   rdb_RDBBlocksLo	; i.e. the first partitionable block
    ULONG   rdb_RDBBlocksHi
    ULONG   rdb_LoCylinder	; low cylinder of partitionable disk area
    ULONG   rdb_HiCylinder	; high cylinder of partitionable data area
    ULONG   rdb_CylBlocks	; number of blocks available per cylinder
    ULONG   rdb_AutoParkSeconds
    ULONG   rdb_HighRDSKBlock
    STRUCT  rdb_Reserved4,1*4
    ; drive identification
    STRUCT  rdb_DiskVendor,8
    STRUCT  rdb_DiskProduct,16
    STRUCT  rdb_DiskRevision,4
    STRUCT  rdb_ControllerVendor,8
    STRUCT  rdb_ControllerProduct,16
    STRUCT  rdb_ControllerRevision,4
    STRUCT  rdb_Reserved5,10*4

    LABEL   RigidDiskBlock_SIZEOF

IDNAME_RIGIDDISK	EQU	(('R'<<24)!('D'<<16)!('S'<<8)!('K'))

RDB_LOCATION_LIMIT	EQU	10

    BITDEF  RDBF,LAST,0		; no disks exist to be configured after
				;   this one on this controller
    BITDEF  RDBF,LASTLUN,1	; no LUNs exist to be configured greater
				;   than this one at this SCSI Target ID
    BITDEF  RDBF,LASTTID,2	; no Target IDs exist to be configured
				;   greater than this one on this SCSI bus
    BITDEF  RDBF,DISKID,4	; rdb_Disk... identification valid
    BITDEF  RDBF,CTRLRID,5	; rdb_Controller... identification valid


;---------------------------------------------------------------------
 STRUCTURE	PartitionBlock,0
    ULONG   pb_ID		; 4 character identifier
    ULONG   pb_SummedLongs	; size of this checksummed structure
    LONG    pb_ChkSum		; block checksum (longword sum to zero)
    ULONG   pb_HostID		; SCSI Target ID of host
    ULONG   pb_Next		; block number of the next PartitionBlock
    ULONG   pb_Flags		; see below for defines
    STRUCT  pb_Reserved1,2*4
    ULONG   pb_DevFlags		; preferred flags for OpenDevice
    STRUCT  pb_DriveName,32	; preferred DOS device name: BSTR form
				; (not used if this name is in use)
    STRUCT  pb_Reserved2,15*4	; filler to 32 longwords
    STRUCT  pb_Environment,17*4	; environment vector for this partition
    STRUCT  pb_EReserved,15*4	; reserved for future environment vector
    LABEL   PartitionBlock_SIZEOF

IDNAME_PARTITION	EQU	(('P'<<24)!('A'<<16)!('R'<<8)!('T'))

    BITDEF  PBF,BOOTABLE,0	; this partition is bootable: (e.g. all the
				; expected directories and files exist)
    BITDEF  PBF,NOMOUNT,1	; if true, don't mount this partition

;---------------------------------------------------------------------
 STRUCTURE	FileSysHeaderBlock,0
    ULONG   fhb_ID		; 4 character identifier
    ULONG   fhb_SummedLongs	; size of this checksummed structure
    LONG    fhb_ChkSum		; block checksum (longword sum to zero)
    ULONG   fhb_HostID		; SCSI Target ID of host
    ULONG   fhb_Next		; block number of the next FileSysHeaderBlock
    ULONG   fhb_Flags		; see below for defines
    STRUCT  fhb_Reserved1,2*4
    ULONG   fhb_DosType		; file system description: match this with
				; partition environment's DE_DOSTYPE entry 
    ULONG   fhb_Version		; release version of this code
    ULONG   fhb_PatchFlags	; bits set for those of the following that
				;   need to be substituted into a standard
				;   device node for this file system: e.g.
				;   $180 to substitute SegList & GlobalVec
    ULONG   fhb_Type		; device node type: zero
    ULONG   fhb_Task		; standard dos "task" field: zero
    ULONG   fhb_Lock		; not used for devices: zero
    ULONG   fhb_Handler		; filename to loadseg: zero placeholder
    ULONG   fhb_StackSize	; stacksize to use when starting task
    LONG    fhb_Priority	; task priority when starting task
    LONG    fhb_Startup		; startup msg: zero placeholder
    LONG    fhb_SegListBlocks	; first of linked list of LoadSegBlocks:
				;   note that this entry requires some
				;   processing before substitution
    LONG    fhb_GlobalVec	; BCPL global vector when starting task
    STRUCT  fhb_Reserved2,23*4	; (those reserved by PatchFlags)
    STRUCT  fhb_Reserved3,21*4
    LABEL   FileSysHeader_SIZEOF

IDNAME_FILESYSHEADER	EQU	(('F'<<24)!('S'<<16)!('H'<<8)!('D'))

;---------------------------------------------------------------------
 STRUCTURE	LoadSegBlock,0
    ULONG   lsb_ID		; 4 character identifier
    ULONG   lsb_SummedLongs	; size of this checksummed structure
    LONG    lsb_ChkSum		; block checksum (longword sum to zero)
    ULONG   lsb_HostID		; SCSI Target ID of host
    ULONG   lsb_Next		; block number of the next FileSysBlock
    STRUCT  lsb_LoadData,123*4	; data for "loadseg"
    ; note 123 assumes 512 byte blocks
    ; there is no LoadSegBlock_SIZEOF: try rdb_BlockBytes

IDNAME_LOADSEG		EQU	(('L'<<24)!('S'<<16)!('E'<<8)!('G'))


;---------------------------------------------------------------------
 STRUCTURE	BadBlockBlock,0
    ULONG   bbb_ID		; 4 character identifier
    ULONG   bbb_SummedLongs	; size of this checksummed structure
    LONG    bbb_ChkSum		; block checksum (longword sum to zero)
    ULONG   bbb_HostID		; SCSI Target ID of host
    ULONG   bbb_Next		; block number of the next BadBlockBlock
    ULONG   bbb_Pad	        ; need even number of 8 byte entries
    STRUCT  bbb_BadData,122*4	; data for bad block mapping
    ; note 122 assumes 512 byte blocks
    ; there is no BadBlockBlock_SIZEOF: try rdb_BlockBytes

IDNAME_BADLIST		EQU	(('B'<<24)!('A'<<16)!('D'<<8)!('B'))

	ENDC
