��LIBRARIES_CONFIGVARS_H�LIBRARIES_CONFIGVARS_H�EXEC_TYPES_H�"exec/types.h"����"exec/nodes.h"��LIBRARIES_CONFIGREGS_H�"libraries/configregs.h"�
�ConfigDev{
��cd_Node;
�cd_Flags;
�cd_Pad;
�ExpansionRom cd_Rom;
�cd_BoardAddr;
�cd_BoardSize;
�cd_SlotAddr;
�cd_SlotSize;
�cd_Driver;
�ConfigDev*cd_NextCD;
�cd_Unused[4];
};�CDB_SHUTUP 0�CDB_CONFIGME 1�CDF_SHUTUP 1�CDF_CONFIGME 2
�CurrentBinding{
�ConfigDev*cb_ConfigDev;
�*cb_FileName;
�*cb_ProductString;
�**cb_ToolTypes;
};
�ConfigDev*AllocConfigDev(),*FindConfigDev();�