��RESOURCES_DISK_H�RESOURCES_DISK_H�EXEC_TYPES_H�"exec/types.h"����"exec/lists.h"����"exec/ports.h"��EXEC_INTERRUPTS_H�"exec/interrupts.h"��EXEC_LIBRARIES_H�"exec/libraries.h"�
�DiscResourceUnit{
��dru_Message;
�Interrupt dru_DiscBlock;
�Interrupt dru_DiscSync;
�Interrupt dru_Index;
};
�DiscResource{
�Library dr_Library;
�DiscResourceUnit*dr_Current;
�dr_Flags;
�dr_pad;
�Library*dr_SysLib;
�Library*dr_CiaResource;
�dr_UnitID[4];
��dr_Waiting;
�Interrupt dr_DiscBlock;
�Interrupt dr_DiscSync;
�Interrupt dr_Index;
};�DRB_ALLOC0 0�DRB_ALLOC1 1�DRB_ALLOC2 2�DRB_ALLOC3 3�DRB_ACTIVE 7�DRF_ALLOC0 (1<<0)�DRF_ALLOC1 (1<<1)�DRF_ALLOC2 (1<<2)�DRF_ALLOC3 (1<<3)�DRF_ACTIVE (1<<7)�DSKDMAOFF 0x4000�DISKNAME "disk.resource"�DR_ALLOCUNIT (LIB_BASE-0*LIB_VECTSIZE)�DR_FREEUNIT (LIB_BASE-1*LIB_VECTSIZE)�DR_GETUNIT (LIB_BASE-2*LIB_VECTSIZE)�DR_GIVEUNIT (LIB_BASE-3*LIB_VECTSIZE)�DR_GETUNITID (LIB_BASE-4*LIB_VECTSIZE)�DR_LASTCOMM (DR_GIVEUNIT)�DRT_AMIGA (0x00000000)�DRT_37422D2S (0x55555555)�DRT_EMPTY (0xFFFFFFFF)�