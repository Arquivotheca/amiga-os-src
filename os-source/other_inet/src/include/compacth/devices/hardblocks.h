��DEVICES_HARDBLOCKS_H�DEVICES_HARDBLOCKS_H
�RigidDiskBlock{
�rdb_ID;
�rdb_SummedLongs;
�rdb_ChkSum;
�rdb_HostID;
�rdb_BlockBytes;
�rdb_Flags;
�rdb_BadBlockList;
�rdb_PartitionList;
�rdb_FileSysHeaderList;
�rdb_DriveInit;
�rdb_Reserved1[6];
�rdb_Cylinders;
�rdb_Sectors;
�rdb_Heads;
�rdb_Interleave;
�rdb_Park;
�rdb_Reserved2[3];
�rdb_WritePreComp;
�rdb_ReducedWrite;
�rdb_StepRate;
�rdb_Reserved3[5];
�rdb_RDBBlocksLo;
�rdb_RDBBlocksHi;
�rdb_LoCylinder;
�rdb_HiCylinder;
�rdb_CylBlocks;
�rdb_AutoParkSeconds;
�rdb_Reserved4[2];
�rdb_DiskVendor[8];
�rdb_DiskProduct[16];
�rdb_DiskRevision[4];
�rdb_ControllerVendor[8];
�rdb_ControllerProduct[16];
�rdb_ControllerRevision[4];
�rdb_Reserved5[10];
};�IDNAME_RIGIDDISK (((�)'R'<<24)|((�)'D'<<16)|('S'<<8)|('K'))�RDB_LOCATION_LIMIT 16�RDBFB_LAST 0�RDBFF_LAST 1L�RDBFB_LASTLUN 1�RDBFF_LASTLUN 2L�RDBFB_LASTTID 2�RDBFF_LASTTID 4L�RDBFB_NORESELECT 3�RDBFF_NORESELECT 8L�RDBFB_DISKID 4�RDBFF_DISKID 16L�RDBFB_CTRLRID 5�RDBFF_CTRLRID 32L
�BadBlockEntry{
�bbe_BadBlock;
�bbe_GoodBlock;
};
�BadBlockBlock{
�bbb_ID;
�bbb_SummedLongs;
�bbb_ChkSum;
�bbb_HostID;
�bbb_Next;
�bbb_Reserved;
�BadBlockEntry bbb_BlockPairs[61];
};�IDNAME_BADBLOCK (((�)'B'<<24)|((�)'A'<<16)|('D'<<8)|('B'))
�PartitionBlock{
�pb_ID;
�pb_SummedLongs;
�pb_ChkSum;
�pb_HostID;
�pb_Next;
�pb_Flags;
�pb_Reserved1[2];
�pb_DevFlags;
�pb_DriveName[32];
�pb_Reserved2[15];
�pb_Environment[17];
�pb_EReserved[15];
};�IDNAME_PARTITION (((�)'P'<<24)|((�)'A'<<16)|('R'<<8)|('T'))�PBFB_BOOTABLE 0�PBFF_BOOTABLE 1L�PBFB_NOMOUNT 1�PBFF_NOMOUNT 2L
�FileSysHeaderBlock{
�fhb_ID;
�fhb_SummedLongs;
�fhb_ChkSum;
�fhb_HostID;
�fhb_Next;
�fhb_Flags;
�fhb_Reserved1[2];
�fhb_DosType;
�fhb_Version;
�fhb_PatchFlags;
�fhb_Type;
�fhb_Task;
�fhb_Lock;
�fhb_Handler;
�fhb_StackSize;
�fhb_Priority;
�fhb_Startup;
�fhb_SegListBlocks;
�fhb_GlobalVec;
�fhb_Reserved2[23];
�fhb_Reserved3[21];
};�IDNAME_FILESYSHEADER (((�)'F'<<24)|((�)'S'<<16)|('H'<<8)|('D'))
�LoadSegBlock{
�lsb_ID;
�lsb_SummedLongs;
�lsb_ChkSum;
�lsb_HostID;
�lsb_Next;
�lsb_LoadData[123];
};�IDNAME_LOADSEG (((�)'L'<<24)|((�)'S'<<16)|('E'<<8)|('G'))�