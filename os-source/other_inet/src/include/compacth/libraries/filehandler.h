��LIBRARIES_FILEHANDLER_H�LIBRARIES_FILEHANDLER_H�EXEC_TYPES_H�"exec/types.h"����"exec/ports.h"��LIBRARIES_DOS_H�"libraries/dos.h"�
�DosEnvec{
�de_TableSize;
�de_SizeBlock;
�de_SecOrg;
�de_Surfaces;
�de_SectorPerBlock;
�de_BlocksPerTrack;
�de_Reserved;
�de_PreAlloc;
�de_Interleave;
�de_LowCyl;
�de_HighCyl;
�de_NumBuffers;
�de_BufMemType;
�de_MaxTransfer;
�de_Mask;
�de_BootPri;
�de_DosType;
};�DE_TABLESIZE 0�DE_SIZEBLOCK 1�DE_SECORG 2�DE_NUMHEADS 3�DE_SECSPERBLK 4�DE_BLKSPERTRACK 5�DE_RESERVEDBLKS 6�DE_PREFAC 7�DE_INTERLEAVE 8�DE_LOWCYL 9�DE_UPPERCYL 10�DE_NUMBUFFERS 11�DE_MEMBUFTYPE 12�DE_BUFMEMTYPE 12�DE_MAXTRANSFER 13�DE_MASK 14�DE_BOOTPRI 15�DE_DOSTYPE 16
�FileSysStartupMsg{
�fssm_Unit;
BSTR fssm_Device;
�fssm_Environ;
�fssm_Flags;
};
�DeviceNode{
�dn_Next;
�dn_Type;
��*dn_Task;
�dn_Lock;
BSTR dn_Handler;
�dn_StackSize;
�dn_Priority;
�dn_Startup;
�dn_SegList;
�dn_GlobalVec;
BSTR dn_Name;
};�