��LIBRARIES_ROMBOOT_BASE_H�LIBRARIES_ROMBOOT_BASE_H�EXEC_TYPES_H�<exec/types.h>����<exec/nodes.h>����<exec/lists.h>��EXEC_LIBRARIES_H�<exec/libraries.h>��EXEC_EXECBASE_H�<exec/execbase.h>��EXEC_EXECNAME_H�<exec/execname.h>�
�RomBootBase
{
�Library LibNode;
�ExecBase*ExecBase;
��BootList;
�Reserved[4];
};
�BootNode
{
��bn_Node;
�bn_Flags;
CPTR bn_DeviceNode;
};�ROMBOOT_NAME "romboot.library"�